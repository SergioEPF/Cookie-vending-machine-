// 100 ms de emissao / 100 ms de pausa. Preparo sem captura antes do motor.
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "maquina_config.h"
#include "sensor_ir.h"
#include "sensor_ir_eventos.h"

static portMUX_TYPE ir_mux = portMUX_INITIALIZER_UNLOCKED;
static SemaphoreHandle_t emissao_gate;
static ir_eventos_t ir;
static bool ir_preparado,ir_respondeu;
static int64_t ir_fim_previsto_us;
static int ir_nivel=1;
static int64_t ir_ultima_recepcao_us;
static uint32_t ir_emitidas, ir_recebidas;
static esp_timer_handle_t ir_timer_periodo, ir_timer_fim;

// Chamador segura ir_mux (ISR ou tarefa). LOW significa recepcao de 38 kHz.
static void amostrar(int nivel, int64_t agora) {
    if(!ir.emitindo || agora<ir.valido_apos_us) return;
    if(nivel==0) {
        ir_ultima_recepcao_us=agora;
        if(!ir_respondeu) {ir_respondeu=true;ir_recebidas++;}
    }
    ir_eventos_amostrar(&ir,nivel,agora);
}

static void ir_recepcao(void *arg) {
    portENTER_CRITICAL_ISR(&ir_mux);
    amostrar(gpio_get_level(IR_RX_GPIO),esp_timer_get_time());
    portEXIT_CRITICAL_ISR(&ir_mux);
}

static void duty(unsigned valor) {
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0,valor));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE,LEDC_CHANNEL_0));
}

// emissao_gate serializa LEDC e timers. Nunca tratar o desligamento como cookie.
static void desligar(void) {
    portENTER_CRITICAL(&ir_mux);
    // Fecha um retorno ja estavel antes da pausa, sem esperar o proximo tick.
    amostrar(gpio_get_level(IR_RX_GPIO),esp_timer_get_time());
    ir_eventos_emitir(&ir,false,0);
    portEXIT_CRITICAL(&ir_mux);
    duty(0);
}

static void ligar(void) {
    desligar();
    portENTER_CRITICAL(&ir_mux);
    ir_eventos_emitir(&ir,true,esp_timer_get_time());
    ir_respondeu=false;
    ir_emitidas++;
    portEXIT_CRITICAL(&ir_mux);
    duty(512);
    ir_fim_previsto_us=esp_timer_get_time()+IR_RAJADA_US;
}

static void parar_timer(esp_timer_handle_t timer) {
    esp_err_t e=esp_timer_stop(timer);
    if(e!=ESP_ERR_INVALID_STATE) ESP_ERROR_CHECK(e);
}

static void ir_finalizar_rajada(void *arg) {
    xSemaphoreTake(emissao_gate,portMAX_DELAY);
    if(ir_preparado && esp_timer_get_time()>=ir_fim_previsto_us) desligar();
    xSemaphoreGive(emissao_gate);
}

static void ir_iniciar_rajada(void *arg) {
    xSemaphoreTake(emissao_gate,portMAX_DELAY);
    if(ir_preparado) {
        parar_timer(ir_timer_fim);
        ligar();
        ESP_ERROR_CHECK(esp_timer_start_once(ir_timer_fim,IR_RAJADA_US));
    }
    xSemaphoreGive(emissao_gate);
}

// Chamador segura emissao_gate. Reinicia a fase do ciclo sem capturar cookie.
static void preparar_emissao(void) {
    parar_timer(ir_timer_periodo);
    parar_timer(ir_timer_fim);
    desligar();
    portENTER_CRITICAL(&ir_mux);
    ir_preparado=true;
    ir.observando=false;
    ir.passagem=false;
    portEXIT_CRITICAL(&ir_mux);
    ligar();
    ESP_ERROR_CHECK(esp_timer_start_once(ir_timer_fim,IR_RAJADA_US));
    ESP_ERROR_CHECK(esp_timer_start_periodic(ir_timer_periodo,IR_PERIODO_US));
}

void sensor_ir_preparar_emissao(void) {
    xSemaphoreTake(emissao_gate,portMAX_DELAY);
    preparar_emissao();
    xSemaphoreGive(emissao_gate);
}

void sensor_ir_iniciar_observacao(void) {
    xSemaphoreTake(emissao_gate,portMAX_DELAY);
    if(!ir_preparado) preparar_emissao();
    portENTER_CRITICAL(&ir_mux);
    ir_eventos_observar(&ir); // O segundo de antecedencia nao confirma entrega.
    portEXIT_CRITICAL(&ir_mux);
    xSemaphoreGive(emissao_gate);
}

void sensor_ir_encerrar_observacao(void) {
    xSemaphoreTake(emissao_gate,portMAX_DELAY);
    parar_timer(ir_timer_periodo);
    parar_timer(ir_timer_fim);
    desligar();
    portENTER_CRITICAL(&ir_mux);
    ir.observando=false;
    ir_preparado=false;
    portEXIT_CRITICAL(&ir_mux);
    xSemaphoreGive(emissao_gate);
}

bool sensor_ir_passagem_detectada(void) {
    portENTER_CRITICAL(&ir_mux);
    bool passagem=ir.passagem;
    portEXIT_CRITICAL(&ir_mux);
    return passagem;
}

void sensor_ir_atualizar(void) {
    // Complementa a ISR: reconhece LOW mantido apos a acomodacao inicial.
    portENTER_CRITICAL(&ir_mux);
    amostrar(gpio_get_level(IR_RX_GPIO),esp_timer_get_time());
    int nivel=!ir.livre;
    portEXIT_CRITICAL(&ir_mux);
    if(nivel!=ir_nivel) {
        ir_nivel=nivel;
        ESP_LOGI("COOKIE","IR GPIO27: %s",nivel ? "BLOQUEADO/SEM FEIXE" : "LIVRE");
    }
}

bool sensor_ir_livre(void) {return ir_nivel==0;}
bool sensor_ir_pronto(void) {return sensor_ir_livre();}

void sensor_ir_diagnostico(void) {
    portENTER_CRITICAL(&ir_mux);
    uint32_t tx=ir_emitidas,rx=ir_recebidas,passagens=ir.passagens;
    uint32_t rejeitados=ir.pulsos_rejeitados;
    int64_t bloqueio=ir.ultimo_bloqueio_us;
    bool preparado=ir_preparado,passagem=ir.passagem;
    int64_t idade=ir_ultima_recepcao_us ? (esp_timer_get_time()-ir_ultima_recepcao_us)/1000 : -1;
    portEXIT_CRITICAL(&ir_mux);
    ESP_LOGI("COOKIE","IR GPIO27=%d; %s; ultima=%" PRId64 " ms; TX=%" PRIu32 " RX=%" PRIu32
             "; emissor_ativo=%d passagem=%d total=%" PRIu32 "; ciclo %d/%d ms"
             "; rejeitados=%" PRIu32 " ultimo_bloqueio=%" PRId64 " us; filtro=%d/%d/%d us",
             gpio_get_level(IR_RX_GPIO),ir_nivel ? "sem feixe" : "livre",idade,tx,rx,
             preparado,passagem,passagens,IR_RAJADA_US/1000,(IR_PERIODO_US-IR_RAJADA_US)/1000,
             rejeitados,bloqueio,IR_FILTRO_LIVRE_US,IR_FILTRO_BLOQUEIO_US,IR_FILTRO_RETORNO_US);
}

void sensor_ir_status(cJSON *j) {
    portENTER_CRITICAL(&ir_mux);
    int64_t idade=ir_ultima_recepcao_us ? (esp_timer_get_time()-ir_ultima_recepcao_us)/1000 : -1;
    bool preparado=ir_preparado;
    uint32_t passagens=ir.passagens;
    uint32_t rejeitados=ir.pulsos_rejeitados;
    int64_t bloqueio=ir.ultimo_bloqueio_us;
    portEXIT_CRITICAL(&ir_mux);
    cJSON_AddNumberToObject(j,"sensor_gpio",IR_RX_GPIO);
    cJSON_AddNumberToObject(j,"emissor_gpio",IR_TX_GPIO);
    cJSON_AddBoolToObject(j,"sensor_logica_invertida",false);
    cJSON_AddNumberToObject(j,"sensor_ultima_resposta_ms",idade);
    cJSON_AddNumberToObject(j,"emissor_periodo_ms",IR_PERIODO_US/1000);
    cJSON_AddNumberToObject(j,"emissor_ligado_ms",IR_RAJADA_US/1000);
    cJSON_AddNumberToObject(j,"emissor_desligado_ms",(IR_PERIODO_US-IR_RAJADA_US)/1000);
    cJSON_AddBoolToObject(j,"emissor_continuo_ativo",false);
    cJSON_AddBoolToObject(j,"emissor_ativo",preparado);
    cJSON_AddNumberToObject(j,"sensor_passagens",passagens);
    cJSON_AddNumberToObject(j,"sensor_pulsos_rejeitados",rejeitados);
    cJSON_AddNumberToObject(j,"sensor_ultimo_bloqueio_us",bloqueio);
    cJSON_AddNumberToObject(j,"sensor_filtro_livre_us",IR_FILTRO_LIVRE_US);
    cJSON_AddNumberToObject(j,"sensor_filtro_bloqueio_us",IR_FILTRO_BLOQUEIO_US);
    cJSON_AddNumberToObject(j,"sensor_filtro_retorno_us",IR_FILTRO_RETORNO_US);
    cJSON_AddBoolToObject(j,"sensor_feixe_livre",sensor_ir_livre());
    cJSON_AddBoolToObject(j,"sensor_pronto",sensor_ir_pronto());
}

void sensor_ir_init(void) {
    emissao_gate=xSemaphoreCreateMutex();
    configASSERT(emissao_gate);
    gpio_config_t rx={.pin_bit_mask=1ULL<<IR_RX_GPIO,.mode=GPIO_MODE_INPUT,
        .pull_up_en=GPIO_PULLUP_ENABLE,.intr_type=GPIO_INTR_ANYEDGE};
    ESP_ERROR_CHECK(gpio_config(&rx));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(IR_RX_GPIO,ir_recepcao,NULL));
    ledc_timer_config_t timer={.speed_mode=LEDC_LOW_SPEED_MODE,
        .duty_resolution=LEDC_TIMER_10_BIT,.timer_num=LEDC_TIMER_0,
        .freq_hz=IR_FREQUENCIA_HZ,.clk_cfg=LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&timer));
    ledc_channel_config_t tx={.gpio_num=IR_TX_GPIO,.speed_mode=LEDC_LOW_SPEED_MODE,
        .channel=LEDC_CHANNEL_0,.timer_sel=LEDC_TIMER_0,.duty=0,.hpoint=0};
    ESP_ERROR_CHECK(ledc_channel_config(&tx));
    esp_timer_create_args_t fim={.callback=ir_finalizar_rajada,.name="ir_fim"};
    esp_timer_create_args_t periodo={.callback=ir_iniciar_rajada,.name="ir_periodo",
        .skip_unhandled_events=true};
    ESP_ERROR_CHECK(esp_timer_create(&fim,&ir_timer_fim));
    ESP_ERROR_CHECK(esp_timer_create(&periodo,&ir_timer_periodo));
}
