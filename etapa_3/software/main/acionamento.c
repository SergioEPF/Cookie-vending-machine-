// Saídas dos seis motores e entrada do fim de curso, conforme o esquema da placa.
#include "esp_log.h"
#include "maquina_config.h"
#include "acionamento.h"

static const gpio_num_t motores[]=MOTORES_GPIO;
_Static_assert(sizeof(motores)/sizeof(motores[0])==NUM_SABORES,"Mapa de motores incompleto");

bool fim_curso_acionado(void) {
    // Pull-up mantém HIGH com a chave aberta; contato para GND indica porta aberta.
    return gpio_get_level(FIM_CURSO_GPIO)==FIM_CURSO_ATIVO;
}

void acionamento_parar(void) {
    // Também usado na partida e no cancelamento: desliga mesmo sem slot conhecido.
    for(int i=0;i<NUM_SABORES;i++) gpio_set_level(motores[i],0);
}

bool acionamento_ligar(int slot) {
    // Exclusividade: desliga qualquer saída anterior antes de validar/ligar outra.
    // HIGH é o comando do driver de potência; o GPIO não alimenta o motor.
    acionamento_parar();
    if(slot<0 || slot>=NUM_SABORES || fim_curso_acionado()) return false;
    gpio_set_level(motores[slot],1);
    ESP_LOGI("COOKIE","Motor %d: GPIO%d LIGADO",slot+1,motores[slot]);
    return true;
}

void acionamento_init(void) {
    // A máscara reúne os seis GPIOs em uma única configuração do driver.
    uint64_t mask=0;
    for(int i=0;i<NUM_SABORES;i++) mask|=1ULL<<motores[i];
    gpio_config_t saidas={.pin_bit_mask=mask,.mode=GPIO_MODE_OUTPUT};
    ESP_ERROR_CHECK(gpio_config(&saidas));
    acionamento_parar();
    gpio_config_t chave={.pin_bit_mask=1ULL<<FIM_CURSO_GPIO,.mode=GPIO_MODE_INPUT,
        .pull_up_en=GPIO_PULLUP_ENABLE};
    ESP_ERROR_CHECK(gpio_config(&chave));
}
