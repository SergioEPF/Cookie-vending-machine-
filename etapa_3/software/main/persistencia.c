// NVS: estoque, sequência, última venda e fila de reenvio ao Python.
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs.h"
#include "persistencia.h"
static const char *TAG="COOKIE";
static int64_t agora_ms(void) { return esp_timer_get_time()/1000; }
static void chave_slot(char *dst, size_t n, const char *prefixo, int i) {
    snprintf(dst,n,"%s%d",prefixo,i+1);
}
void persistencia_salvar_estoque(persistencia_t *dados, int i) {
    // Contrato legado: falhas de estoque/seq não são propagadas à resposta HTTP.
    nvs_handle_t h;
    if (nvs_open("cookievm", NVS_READWRITE, &h) != ESP_OK) return;
    char k[16]; chave_slot(k,sizeof(k),"s",i);
    nvs_set_i32(h,k,dados->estoque[i]); nvs_commit(h); nvs_close(h);
}

// As chaves NVS são preservadas; não recupera pedido interrompido.
void persistencia_carregar(persistencia_t *dados) {
    nvs_handle_t h;
    if (nvs_open("cookievm", NVS_READWRITE, &h) != ESP_OK) {
        dados->falha_persistencia=true; ESP_LOGE(TAG,"Falha NVS"); return;
    }

    for(int i=0;i<NUM_SABORES;i++) {
        char k[16]; int32_t v=ESTOQUE_INICIAL; chave_slot(k,sizeof(k),"s",i);
        nvs_get_i32(h,k,&v); dados->estoque[i]=(v>=0 && v<=CAPACIDADE_SLOT)?v:ESTOQUE_INICIAL;
        chave_slot(k,sizeof(k),"saleq",i); v=0; nvs_get_i32(h,k,&v); dados->ultima_venda_qtd[i]=v;
    }

    nvs_get_u32(h,"saleid",&dados->ultima_venda_id);
    nvs_get_i32(h,"saletotal",(int32_t*)&dados->ultima_venda_total);
    size_t len=sizeof(dados->ultima_venda_usuario);
    if(nvs_get_str(h,"saleuser",dados->ultima_venda_usuario,&len)!=ESP_OK) dados->ultima_venda_usuario[0]='\0';
    len=sizeof(dados->venda_pendente);
    if(nvs_get_str(h,"pending",dados->venda_pendente,&len)!=ESP_OK) dados->venda_pendente[0]=0;
    uint8_t manutencao=0;
    esp_err_t e=nvs_get_u8(h,"maint",&manutencao);
    dados->manutencao=manutencao!=0;
    if(e!=ESP_OK && e!=ESP_ERR_NVS_NOT_FOUND) dados->falha_persistencia=true;
    nvs_close(h);
}

bool persistencia_manutencao(persistencia_t *dados, bool ativa) {
    if(ativa) dados->manutencao=true; // Bloqueia em RAM mesmo se a flash falhar.
    nvs_handle_t h;
    esp_err_t e=nvs_open("cookievm",NVS_READWRITE,&h);
    if(e==ESP_OK) {
        e=nvs_set_u8(h,"maint",ativa);
        if(e==ESP_OK) e=nvs_commit(h);
        nvs_close(h);
    }
    if(e!=ESP_OK) dados->falha_persistencia=true;
    else dados->manutencao=ativa;
    return e==ESP_OK;
}

// Publica a venda pendente antes de aceitar outro pedido; falha bloqueia compras.
void persistencia_salvar_venda(persistencia_t *dados, uint32_t pedido_id,
    const char *usuario_pedido, const int *quantidade_entregue, int total_cookies,
    const char *evento_pedido) {
    nvs_handle_t h;
    if(nvs_open("cookievm",NVS_READWRITE,&h)!=ESP_OK) {dados->falha_persistencia=true;return;}
    dados->ultima_venda_id=pedido_id;
    snprintf(dados->ultima_venda_usuario,sizeof(dados->ultima_venda_usuario),"%s",usuario_pedido);
    dados->ultima_venda_total=total_cookies;
    nvs_set_u32(h,"saleid",dados->ultima_venda_id);
    nvs_set_str(h,"saleuser",dados->ultima_venda_usuario);
    nvs_set_i32(h,"saletotal",dados->ultima_venda_total);
    for(int i=0;i<NUM_SABORES;i++) {
        char k[16]; chave_slot(k,sizeof(k),"saleq",i);
        dados->ultima_venda_qtd[i]=quantidade_entregue[i]; nvs_set_i32(h,k,dados->ultima_venda_qtd[i]);
    }

    cJSON *sale=cJSON_CreateObject();
    cJSON_AddStringToObject(sale,"evento_id",evento_pedido);
    cJSON_AddStringToObject(sale,"usuario",usuario_pedido);
    cJSON_AddStringToObject(sale,"status","entregue");
    cJSON_AddNumberToObject(sale,"total",total_cookies);
    for(int i=0;i<NUM_SABORES;i++) {
        char k[12];chave_slot(k,sizeof(k),"q",i);
        cJSON_AddNumberToObject(sale,k,quantidade_entregue[i]);
    }

    esp_err_t saved=ESP_FAIL;
    if(cJSON_PrintPreallocated(sale,dados->venda_pendente,sizeof(dados->venda_pendente),false)) {
        saved=nvs_set_str(h,"pending",dados->venda_pendente);
        if(saved==ESP_OK) saved=nvs_commit(h);
    } else {
        dados->venda_pendente[0]=0; // Nunca enviar JSON parcial após erro de memória.
    }
    cJSON_Delete(sale); nvs_close(h);
    if(saved!=ESP_OK) dados->falha_persistencia=true;
}

uint32_t persistencia_proximo_id(void) {
    nvs_handle_t h; uint32_t id=0;
    if(nvs_open("cookievm",NVS_READWRITE,&h)!=ESP_OK) return (uint32_t)agora_ms();
    nvs_get_u32(h,"seq",&id); ++id;
    nvs_set_u32(h,"seq",id); nvs_commit(h); nvs_close(h); return id;
}

// Só retira da RAM depois de confirmar a remoção da fila na flash.
void persistencia_limpar_pendente(persistencia_t *dados) {
    nvs_handle_t h;
    if(nvs_open("cookievm",NVS_READWRITE,&h)!=ESP_OK) return;
    esp_err_t e=nvs_erase_key(h,"pending");
    if(e==ESP_OK || e==ESP_ERR_NVS_NOT_FOUND) e=nvs_commit(h);
    nvs_close(h);
    if(e==ESP_OK) dados->venda_pendente[0]=0;
}

cJSON *persistencia_ultima_venda(const persistencia_t *dados) {
    cJSON *j=cJSON_CreateObject();
    cJSON_AddBoolToObject(j,"existe",dados->ultima_venda_id!=0);
    if(!dados->ultima_venda_id) return j;
    cJSON_AddNumberToObject(j,"id",dados->ultima_venda_id);
    cJSON_AddStringToObject(j,"usuario",dados->ultima_venda_usuario);
    cJSON_AddNumberToObject(j,"total",dados->ultima_venda_total);
    for(int i=0;i<NUM_SABORES;i++) {
        char k[12]; chave_slot(k,sizeof(k),"q",i);
        cJSON_AddNumberToObject(j,k,dados->ultima_venda_qtd[i]);
    }
    return j;
}
