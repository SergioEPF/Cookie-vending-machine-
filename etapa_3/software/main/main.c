// Inicializacao do ESP32. As telas HTML sao servidas pelo PC.
/* main.c — ponto de entrada do firmware (ESP-IDF).
 * Este arquivo apenas organiza a partida. A API fica em api.c; estoque,
 * motor e tarefas em controller.c; a comunicação com o PC em backend.c.
 * Os HTMLs não são embutidos na flash: são fornecidos pelo servidor Python.
 * ESP_ERROR_CHECK interrompe a execução se uma inicialização essencial falhar.
 */
// Serviços do SDK: rádio Wi-Fi, eventos, rede TCP/IP, logs e flash NVS.
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "controller.h"
#include "api.h"

// Nome, senha WPA2 e canal da rede criada pelo ESP (modo Access Point).
// Estes valores são configuração do protótipo, separados da chave do Python.
#define WIFI_SSID "COOKIE_EXPRESS"
#define WIFI_PASS "cookie123"
#define WIFI_CHANNEL 11

/* Inicializa a pilha de rede e cria o ponto de acesso.
 * O ESP é o AP; PC e celular entram nessa mesma rede. A interface AP padrão
 * do ESP-IDF fornece DHCP aos clientes e usa 192.168.4.1 no projeto atual.
 * Esta função não conecta o ESP a um roteador e não fornece acesso à internet.
 */
static void iniciar_wifi(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    // WIFI_INIT_CONFIG_DEFAULT preenche parâmetros internos exigidos pelo SDK.
    // O inicializador .ap configura somente a interface de ponto de acesso.
    wifi_init_config_t cfg=WIFI_INIT_CONFIG_DEFAULT();ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t ap={.ap={
        .ssid=WIFI_SSID, .password=WIFI_PASS, .ssid_len=sizeof(WIFI_SSID)-1,
        .channel=WIFI_CHANNEL, .authmode=WIFI_AUTH_WPA2_PSK, .max_connection=4
    }};
    // Até quatro estações simultâneas; WPA2 exige a senha definida acima.
    // HT20 fixa a largura em 20 MHz para a configuração de compatibilidade local.
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP,&ap));
    // Trafego HTTP local e pequeno; HT20 evita depender de coexistencia HT20/40.
    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP,WIFI_BW_HT20));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI("COOKIE","Wi-Fi %s: http://192.168.4.1, canal %d, 20 MHz", WIFI_SSID,WIFI_CHANNEL);
}

/* Entrada chamada pelo ESP-IDF depois da partida do sistema.
 * A ordem importa: NVS antes de carregar estoque; mutexes antes das tarefas;
 * Wi-Fi antes de expor a API e iniciar a sincronização periódica com o PC.
 * As tarefas criadas continuam executando depois que app_main retorna.
 */
void app_main(void) {
    esp_err_t result=nvs_flash_init();
    if(result==ESP_ERR_NVS_NO_FREE_PAGES || result==ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Recuperação prevista pelo SDK para NVS sem páginas livres/incompatível.
        // ATENÇÃO: apaga toda a partição NVS, incluindo estoque e venda pendente.
        // Não equivale a recuperar os dados antigos a partir do banco Python.
        ESP_ERROR_CHECK(nvs_flash_erase());
        result=nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);
    // Prepara exclusão mútua do cliente HTTP e o controle físico ainda desligado.
    backend_init();
    controller_init();
    iniciar_wifi();
    // HTTP atende as telas do PC; duas tarefas cuidam de motor/serial e banco.
    api_start();
    controller_start();
}
