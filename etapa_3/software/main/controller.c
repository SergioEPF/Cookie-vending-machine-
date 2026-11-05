// Coordena pedidos e API; sensor, entrada de teste e NVS ficam em módulos próprios.
// O mutex protege todo estado abaixo. Nunca é mantido durante HTTP.
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "controller.h"
#include "maquina_config.h"
#include "sensor_ir.h"
#include "modo_teste.h"
#include "persistencia.h"
#include "acionamento.h"

static const char *TAG = "COOKIE";
static const char *const sabores[NUM_SABORES] = {"Chocolate", "Chocolate Branco", "Red Velvet", "Baunilha", "M&M's", "Integral"};
static persistencia_t dados;
static bool ir_passagem;

static bool pedido_ativo = false;
static uint32_t pedido_id = 0;
static int64_t instante_pedido_ms = 0;
static char usuario_pedido[128] = "";
typedef enum { OCIOSO, RECEBIDO, LIBERANDO, ENTREGUE, ERRO, EMITINDO, INTERVALO_REPETICAO } estado_t;
static estado_t estado=OCIOSO;
static const char *const estados[]={"ocioso","recebido","liberando","entregue","erro","recebido","recebido"};
static char mensagem_sensor[64] = "";
static int restante_pedido[NUM_SABORES], quantidade_entregue[NUM_SABORES];
static int total_cookies = 0, cookies_entregues = 0, slot_atual = -1;

static bool motor_ativo = false;
static int64_t motor_inicio_ms = 0;
static int64_t motor_parada_ms = 0;
static int64_t emissor_inicio_ms = 0;
static unsigned tentativas_cookie=0;
static SemaphoreHandle_t mutex;

static bool banco_online = false;
static int64_t banco_ultimo_ok = 0;

static char evento_pedido[33] = "";
static char token_pedido[96] = "";

static void aleatorio_hex(char *out, size_t bytes) {
    const char *digits="0123456789abcdef";
    for (size_t i=0;i<bytes;i++) {
        uint8_t b=(uint8_t)esp_random(); out[2*i]=digits[b>>4]; out[2*i+1]=digits[b&15];
    }
    out[2*bytes]=0;
}

static int64_t agora_ms(void) { return esp_timer_get_time()/1000; }

static void lock(void) { xSemaphoreTake(mutex, portMAX_DELAY); }

static void unlock(void) { xSemaphoreGive(mutex); }

// A API usa q1..q6; os vetores internos usam índices 0..5.
static void chave_slot(char *dst, size_t n, const char *prefixo, int i) { snprintf(dst,n,"%s%d",prefixo,i+1); }

// Chamado sob mutex, antes de qualquer ação de motor ou confirmação.
static void verificar_manutencao(void) {
    if(!fim_curso_acionado()) {
        // Porta fechada libera compras, sem retomar o pedido interrompido.
        // Falha NVS mantém o bloqueio; não repetir gravações a cada tick.
        if(dados.manutencao && !dados.falha_persistencia &&
           persistencia_manutencao(&dados,false))
            ESP_LOGI(TAG,"Porta fechada: manutencao liberada automaticamente");
        return;
    }
    // Na entrada em manutenção, garante todas as saídas em LOW. Nos ticks
    // seguintes elas já estão desligadas e nenhum acionamento é permitido.
    // Evita seis escritas GPIO por tick enquanto a porta permanece aberta.
    if(motor_ativo || !dados.manutencao) acionamento_parar();
    motor_ativo=false;
    if(pedido_ativo) {
        pedido_ativo=false; estado=ERRO;
        sensor_ir_encerrar_observacao();
        ir_passagem=false;
        snprintf(mensagem_sensor,sizeof(mensagem_sensor),"Entrega interrompida: maquina em manutencao");
    }
    if(!dados.manutencao) {
        persistencia_manutencao(&dados,true);
        ESP_LOGW(TAG,"Fim de curso acionado: motores parados, manutencao bloqueada");
    }
}

// Iniciar pulso não reduz estoque; fim de curso tem prioridade sobre D/BOOT e IR.
static bool aguardando_confirmacao(void) {
    return !dados.manutencao && !fim_curso_acionado() &&
        pedido_ativo && slot_atual>=0 && !motor_ativo && estado==LIBERANDO;
}

// Cada tentativa mantém o mesmo slot e não altera estoque nem quantidade entregue.
static void iniciar_tentativa_cookie(void) {
    verificar_manutencao();
    if(dados.manutencao) return;
    sensor_ir_preparar_emissao();
    ir_passagem=false;
    emissor_inicio_ms=agora_ms();
    estado=EMITINDO;
    snprintf(mensagem_sensor,sizeof(mensagem_sensor),"Emissor ligado; motor em 1 segundo");
}

static void ligar_motor_tentativa(void) {
    verificar_manutencao();
    if(dados.manutencao) return;
    if(!MODO_TESTE_SERIAL) sensor_ir_iniciar_observacao();
    if(!acionamento_ligar(slot_atual)) {
        sensor_ir_encerrar_observacao();
        verificar_manutencao();
        return;
    }
    ir_passagem=false;
    tentativas_cookie++;
    snprintf(mensagem_sensor,sizeof(mensagem_sensor),"Tentativa %u/%u: aguardando cookie",
             tentativas_cookie,(unsigned)(IR_REPETICOES_MAX+1));
    estado=LIBERANDO;
    motor_parada_ms=0;
    motor_inicio_ms=agora_ms();motor_ativo=true;
    ESP_LOGI(TAG,"Slot %d: tentativa %u/%u; emissor antecipado 1 s",
             slot_atual+1,tentativas_cookie,(unsigned)(IR_REPETICOES_MAX+1));
}

// Operações internas de pedido exigem o mutex adquirido.
static void preparar_proximo_cookie(void) {
    verificar_manutencao();
    if(dados.manutencao) return;
    slot_atual=-1;
    for(int i=0;i<NUM_SABORES;i++) if(restante_pedido[i]>0) {slot_atual=i; break;}
    if(slot_atual<0) return;
    tentativas_cookie=0;
    iniciar_tentativa_cookie();
}

// Baixa uma unidade após confirmação; a última agenda a venda durável.
static void confirmar_entrega(void) {
    verificar_manutencao();
    if(!aguardando_confirmacao()) {
        ESP_LOGW(TAG,"Confirmacao ignorada (sem cookie aguardando)"); return;
    }
    if(dados.estoque[slot_atual]<=0 || restante_pedido[slot_atual]<=0) {
        sensor_ir_encerrar_observacao();
        estado=ERRO;
        snprintf(mensagem_sensor,sizeof(mensagem_sensor),"estoque inconsistente"); return;
    }

    sensor_ir_encerrar_observacao();
    quantidade_entregue[slot_atual]++; restante_pedido[slot_atual]--;
    cookies_entregues++; dados.estoque[slot_atual]--; persistencia_salvar_estoque(&dados,slot_atual);
    snprintf(mensagem_sensor,sizeof(mensagem_sensor),"ok cookie entregue");
    ESP_LOGI(TAG,"Cookie confirmado: slot %d; %d/%d",slot_atual+1,cookies_entregues,total_cookies);
    if(cookies_entregues>=total_cookies) {
        estado=ENTREGUE; pedido_ativo=false;
        persistencia_salvar_venda(&dados,pedido_id,usuario_pedido,quantidade_entregue,total_cookies,evento_pedido); ESP_LOGI(TAG,"Pedido concluido e ultima venda salva");
    } else {

        estado=RECEBIDO;
        slot_atual=-1; instante_pedido_ms=agora_ms();
    }
}

// Cancelar não estorna entregas já confirmadas nem registra venda parcial.
static void resetar_pedido(void) {
    acionamento_parar(); motor_ativo=false;
    sensor_ir_encerrar_observacao();
    ir_passagem=false;
    tentativas_cookie=0; motor_parada_ms=0;
    pedido_ativo=false; estado=OCIOSO;
    mensagem_sensor[0]='\0'; total_cookies=0; cookies_entregues=0; slot_atual=-1;
    memset(restante_pedido,0,sizeof(restante_pedido));
    ESP_LOGW(TAG,"Pedido resetado e GPIO desligado");
}

// Um passo sob mutex. A janela de cada tentativa começa junto com o motor.
static void controle_passo(int64_t t) {
    verificar_manutencao();
    sensor_ir_atualizar();
    if(pedido_ativo && estado==EMITINDO && t-emissor_inicio_ms>=IR_ANTECEDENCIA_MS)
        ligar_motor_tentativa();
    if(motor_ativo && t-motor_inicio_ms>=TEMPO_MOTOR_MS) {
        acionamento_parar(); motor_ativo=false;
        motor_parada_ms=t;
        ESP_LOGI(TAG,"Motor %d DESLIGADO; aguardando %s",slot_atual+1,MODO_TESTE_SERIAL ? "D ou BOOT para simular entrega" : "passagem no sensor IR");
    }

    // Evento retido pela ISR: nao perde um ciclo completo entre dois ticks.
    if(!MODO_TESTE_SERIAL && pedido_ativo && estado==LIBERANDO)
        ir_passagem=ir_passagem || sensor_ir_passagem_detectada();
    unsigned eventos=modo_teste_ler(t,aguardando_confirmacao());
    if(eventos & TESTE_CANCELAR) resetar_pedido();
    else if((!MODO_TESTE_SERIAL && ir_passagem && aguardando_confirmacao()) ||
            (eventos & TESTE_CONFIRMAR))
        confirmar_entrega();
    if(!MODO_TESTE_SERIAL && pedido_ativo && estado==LIBERANDO && !motor_ativo &&
       t-motor_parada_ms>=IR_INTERVALO_REPETICAO_MS && tentativas_cookie<IR_REPETICOES_MAX+1) {
        sensor_ir_encerrar_observacao();
        estado=INTERVALO_REPETICAO;
        instante_pedido_ms=t;
        ESP_LOGI(TAG,"Sem confirmacao; repeticao com intervalo de 2 s, sem erro de IR");
    }
    // O intervalo total inclui o segundo final de antecedencia do emissor.
    if(pedido_ativo && t-instante_pedido_ms>=INTERVALO_COOKIE_MS-IR_ANTECEDENCIA_MS) {
        if(estado==RECEBIDO) preparar_proximo_cookie();
        else if(estado==INTERVALO_REPETICAO) iniciar_tentativa_cookie();
    }
    if(eventos & TESTE_DIAGNOSTICO) {
        sensor_ir_diagnostico();
        modo_teste_diagnostico();
        ESP_LOGI(TAG,"pedido=%d; motor=%d; estado=%s",pedido_ativo,motor_ativo,estados[estado]);
    }
}

// Laço sem HTTP: sensor, temporização do motor e entradas de teste.
static void tarefa_controle(void *arg) {
    while(true) {
        lock();
        controle_passo(agora_ms());
        unlock();
        vTaskDelay(1);
    }
}

static cJSON *erro(const char *msg) { cJSON *j=cJSON_CreateObject(); cJSON_AddBoolToObject(j,"ok",false); cJSON_AddStringToObject(j,"erro",msg); return j; }

static cJSON *ok_obj(void) {cJSON *j=cJSON_CreateObject();cJSON_AddBoolToObject(j,"ok",true);return j;}

static int get_int(cJSON *j,const char *key,int default_value) {
    cJSON *p=cJSON_GetObjectItemCaseSensitive(j,key);
    return cJSON_IsNumber(p) && p->valuedouble==(double)p->valueint ? p->valueint:default_value;
}

// Nunca manter o mutex de estado durante HTTP; disponibilidade expira em 20 s.
int controller_request(esp_http_client_method_t method,const char *path,
                            const char *body,const char *auth,cJSON **out) {
    int code=backend_request(method,path,body,auth,out);
    lock();
    banco_online=(code!=503);
    if(banco_online) banco_ultimo_ok=agora_ms();
    unlock();
    return code;
}

// Contrato da API preservado; JSON retornado pertence ao chamador.
cJSON *controller_status(void) {
    cJSON *j=cJSON_CreateObject(); lock();
    verificar_manutencao();
    cJSON_AddBoolToObject(j,"online",true);
    cJSON_AddStringToObject(j,"versao","4.0-python");
    cJSON_AddBoolToObject(j,"banco_online",banco_online && agora_ms()-banco_ultimo_ok<20000);
    cJSON_AddBoolToObject(j,"venda_pendente",dados.venda_pendente[0]!=0);
    cJSON_AddBoolToObject(j,"falha_persistencia",dados.falha_persistencia);
    cJSON_AddBoolToObject(j,"pedido_ativo",pedido_ativo);
    cJSON_AddStringToObject(j,"status_pedido",estados[estado]);
    cJSON_AddBoolToObject(j,"manutencao",dados.manutencao);
    cJSON_AddBoolToObject(j,"fim_curso_acionado",fim_curso_acionado());
    cJSON_AddNumberToObject(j,"fim_curso_gpio",FIM_CURSO_GPIO);
    cJSON_AddStringToObject(j,"mensagem_manutencao",dados.manutencao ? "Maquina em manutencao. Procure o responsavel." : "");
    sensor_ir_status(j);
    cJSON_AddBoolToObject(j,"modo_teste_serial",MODO_TESTE_SERIAL);
    cJSON_AddBoolToObject(j,"botao_teste_ativo",MODO_TESTE_SERIAL || BOOT_CONFIRMA_TESTE);
    if(MODO_TESTE_SERIAL || BOOT_CONFIRMA_TESTE)
        cJSON_AddNumberToObject(j,"botao_teste_gpio",BOTAO_TESTE_GPIO);
    unlock(); return j;
}

cJSON *controller_inventory(void) {
    cJSON *j=cJSON_CreateObject(), *a=cJSON_AddArrayToObject(j,"itens"); lock();
    for(int i=0;i<NUM_SABORES;i++) {
        cJSON *it=cJSON_CreateObject(); cJSON_AddNumberToObject(it,"slot",i+1);
        cJSON_AddStringToObject(it,"sabor",sabores[i]);
        cJSON_AddNumberToObject(it,"quantidade",dados.estoque[i]);
        cJSON_AddNumberToObject(it,"capacidade",CAPACIDADE_SLOT);cJSON_AddItemToArray(a,it);
    }
    unlock();return j;
}

// Comprador/token vêm da sessão validada; aceitar apenas agenda o motor.
cJSON *controller_place_order(cJSON *body, const char *comprador, const char *token, int *status) {
    int qtd[NUM_SABORES]={0},soma=0;char key[12];
    for(int i=0;i<NUM_SABORES;i++) {
        chave_slot(key,sizeof(key),"q",i);
        int v=get_int(body,key,0);
        if(v<0 || v>PEDIDO_MAX) { *status=400; return erro("Quantidade invalida"); }
        qtd[i]=v; soma+=v;
    }

    int total=get_int(body,"total_cookies",-1);
    if(soma<1 || soma>PEDIDO_MAX || soma!=total) {
        *status = 400; return erro("Pedido deve possuir entre 1 e 3 cookies");
    }

    lock();
    verificar_manutencao();
    if(dados.manutencao) {
        unlock();*status=409;
        cJSON *j=erro("Maquina em manutencao. Procure o responsavel.");
        cJSON_AddBoolToObject(j,"manutencao",true);
        return j;
    }
    if(pedido_ativo) { unlock();*status = 409; return erro("Existe um pedido em andamento"); }
    if(dados.venda_pendente[0] || dados.falha_persistencia) {
        unlock();*status = 503; return erro("Aguardando gravacao da venda anterior no banco");
    }
    for(int i=0;i<NUM_SABORES;i++) if(qtd[i]>dados.estoque[i]) {
        cJSON *e=erro("Estoque insuficiente"); cJSON_AddNumberToObject(e,"slot",i+1);
        unlock();*status = 409; return e;
    }

    pedido_id=persistencia_proximo_id();
    snprintf(usuario_pedido,sizeof(usuario_pedido),"%s",comprador);
    snprintf(token_pedido,sizeof(token_pedido),"%s",token);
    aleatorio_hex(evento_pedido,16);
    for(int i=0;i<NUM_SABORES;i++) {
        restante_pedido[i]=qtd[i];quantidade_entregue[i]=0;
    }
    total_cookies=soma;cookies_entregues=0;pedido_ativo=true;
    estado=RECEBIDO; mensagem_sensor[0]='\0';
    slot_atual=-1; instante_pedido_ms=agora_ms();
    cJSON *j=ok_obj(); cJSON_AddNumberToObject(j,"pedido_id",pedido_id);
    cJSON_AddStringToObject(j,"status","recebido");
    ESP_LOGI(TAG,"Pedido #%" PRIu32 " recebido: %d cookies, usuario %s",pedido_id,soma,usuario_pedido);
    unlock();*status = 200; return j;
}

// Consulta vinculada ao token do pedido; não revalida sessão no Python.
cJSON *controller_order_status(const char *token, int *status) {
    lock();bool allowed=token_pedido[0] && strcmp(token,token_pedido)==0;unlock();
    if(!allowed) { *status = 401; return erro("Pedido pertence a outra sessao"); }
    cJSON *j=cJSON_CreateObject();lock();
    verificar_manutencao();
    cJSON_AddNumberToObject(j,"pedido_id",pedido_id);
    cJSON_AddStringToObject(j,"status",estados[estado]);
    cJSON_AddStringToObject(j,"sensor",mensagem_sensor);
    cJSON_AddNumberToObject(j,"cookies_entregues",cookies_entregues);
    cJSON_AddNumberToObject(j,"total_cookies",total_cookies);
    cJSON_AddBoolToObject(j,"manutencao",dados.manutencao);
    unlock();*status = 200; return j;
}

cJSON *controller_clear_maintenance(int *status) {
    lock();
    verificar_manutencao();
    if(fim_curso_acionado()) {
        unlock();*status=409;return erro("Libere a chave de fim de curso antes de retirar a manutencao");
    }
    bool ok=!dados.manutencao || persistencia_manutencao(&dados,false);
    unlock();*status=ok ? 200 : 503;
    return ok ? ok_obj() : erro("Nao foi possivel salvar a liberacao da manutencao");
}

cJSON *controller_last_sale(void) {
    lock(); cJSON *j=persistencia_ultima_venda(&dados); unlock(); return j;
}

// A API valida papel admin. Ajustes são recusados com pedido ativo.
cJSON *controller_adjust_stock(cJSON *body, bool reposicao, int *status) {
    int slot=get_int(body,"slot",-1),quantidade=get_int(body,"quantidade",-1);

    if(slot<1||slot>NUM_SABORES||quantidade<(reposicao?1:0)) {
        *status = 400; return erro("Dados invalidos");
    }
    lock();
    if(pedido_ativo) {unlock();*status = 409; return erro("Nao alterar estoque com pedido ativo");}
    int i=slot-1;
    int novo=reposicao ? (quantidade>CAPACIDADE_SLOT-dados.estoque[i] ? CAPACIDADE_SLOT : dados.estoque[i]+quantidade) : quantidade;
    if(novo>CAPACIDADE_SLOT) novo=CAPACIDADE_SLOT;
    dados.estoque[i]=novo; persistencia_salvar_estoque(&dados,i);
    cJSON *j=ok_obj();cJSON_AddNumberToObject(j,"slot",slot);
    cJSON_AddNumberToObject(j,"quantidade",dados.estoque[i]);
    unlock();*status = 200; return j;
}

// Copia sob mutex e envia fora dele. Falhas mantêm a fila para reenvio.
static void tarefa_banco(void *arg) {
    while(true) {
        char pending[sizeof(dados.venda_pendente)];
        lock();snprintf(pending,sizeof(pending),"%s",dados.venda_pendente);unlock();
        cJSON *response=NULL;
        if(pending[0]) {
            int code=controller_request(HTTP_METHOD_POST,"/api/vendas",pending,NULL,&response);

            if(code==200) {
                lock();
                persistencia_limpar_pendente(&dados);
                unlock();
            }
            cJSON_Delete(response);response=NULL;
        }

        // Reutiliza o buffer da fila; seis slots cabem sem alocar outra string.
        cJSON *snapshot=controller_inventory();
        bool pronto=cJSON_PrintPreallocated(snapshot,pending,sizeof(pending),false);
        cJSON_Delete(snapshot);
        if(pronto) controller_request(HTTP_METHOD_PUT,"/api/estoque",pending,NULL,&response);
        cJSON_Delete(response);
        vTaskDelay(pdMS_TO_TICKS(SINCRONIZACAO_MS));
    }
}

// Inicialização única, após NVS e antes da API; todos os motores começam desligados.
void controller_init(void) {
    mutex=xSemaphoreCreateMutex();
    configASSERT(mutex);
    acionamento_init();
    modo_teste_init();
    sensor_ir_init();
    persistencia_carregar(&dados);
    verificar_manutencao();
}

// Pilhas em bytes: controle prioritário; rede em tarefa separada.
void controller_start(void) {
    configASSERT(xTaskCreate(tarefa_controle,"motor_serial",4096,NULL,5,NULL)==pdPASS);
    configASSERT(xTaskCreate(tarefa_banco,"banco_python",8192,NULL,3,NULL)==pdPASS);
    ESP_LOGW(TAG,"Modo teste=%d: D/BOOT confirma apos motor parar; S diagnostica; R cancela",MODO_TESTE_SERIAL);
}
