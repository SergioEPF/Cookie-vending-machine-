// Entrada de teste: BOOT com filtro de 30 ms e comandos D/R/S da UART.
#include "driver/uart.h"
#include "esp_log.h"
#include "maquina_config.h"
#include "modo_teste.h"

static int estavel, candidato;
static int64_t desde;
static bool armado, elegivel;

void modo_teste_init(void) {
    if(MODO_TESTE_SERIAL || BOOT_CONFIRMA_TESTE) {
        gpio_config_t cfg={.pin_bit_mask=1ULL<<BOTAO_TESTE_GPIO,
            .mode=GPIO_MODE_INPUT,.pull_up_en=GPIO_PULLUP_ENABLE};
        ESP_ERROR_CHECK(gpio_config(&cfg));
    }
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0,1024,0,0,NULL,0));
}

unsigned modo_teste_ler(int64_t t, bool pode_confirmar) {
    unsigned eventos=0;
    if(MODO_TESTE_SERIAL || BOOT_CONFIRMA_TESTE) {
        int nivel=gpio_get_level(BOTAO_TESTE_GPIO);
        if(nivel!=candidato) {
            candidato=nivel; desde=t;
            elegivel=nivel==0 && pode_confirmar;
        }
        if(nivel!=estavel && t-desde>=BOTAO_DEBOUNCE_MS) {
            estavel=nivel;
            if(nivel==1) {
                armado=true;
                ESP_LOGI("COOKIE","BOOT GPIO0 SOLTO");
            } else {
                ESP_LOGI("COOKIE","BOOT GPIO0 PRESSIONADO");
                if(armado && elegivel) eventos|=TESTE_CONFIRMAR;
                else ESP_LOGW("COOKIE","BOOT ignorado: aguarde pedido em liberacao e motor parado");
                // Só uma soltura estável arma novamente; segurar não repete.
                armado=false;
            }
        }
    }
    uint8_t comando;
    if(uart_read_bytes(UART_NUM_0,&comando,1,0)>0) {
        switch(comando) {
            case 'd': case 'D':
                if(MODO_TESTE_SERIAL) eventos|=TESTE_CONFIRMAR;
                else ESP_LOGW("COOKIE","D desativado: entrega exige sensor IR");
                break;
            case 'r': case 'R': eventos|=TESTE_CANCELAR; break;
            case 's': case 'S': eventos|=TESTE_DIAGNOSTICO; break;
        }
    }
    return eventos;
}

void modo_teste_diagnostico(void) {
    ESP_LOGI("COOKIE","TESTE=%d; BOOT_TESTE=%d; BOOT GPIO0=%d; armado=%d",
        MODO_TESTE_SERIAL,BOOT_CONFIRMA_TESTE,gpio_get_level(BOTAO_TESTE_GPIO),armado);
}
