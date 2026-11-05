// Configuração física e tempos do protótipo. Altere aqui e recompile.
#pragma once
#include "driver/gpio.h"
#define NUM_SABORES 6
#define MOTORES_GPIO {GPIO_NUM_18, GPIO_NUM_19, GPIO_NUM_21, GPIO_NUM_22, GPIO_NUM_23, GPIO_NUM_25}
#define FIM_CURSO_GPIO GPIO_NUM_32
#define FIM_CURSO_ATIVO 0 // Porta aberta: GPIO32 em LOW; porta fechada: HIGH (pull-up).
#define TEMPO_MOTOR_MS 3000
#define PEDIDO_MAX 3
#define IR_RX_GPIO GPIO_NUM_27
#define IR_TX_GPIO GPIO_NUM_26
#define IR_FREQUENCIA_HZ 38000
#define IR_INTERVALO_REPETICAO_MS 3000 // Espera antes das duas repeticoes; nao gera erro de IR.
#define IR_REPETICOES_MAX 2 // Duas repeticoes alem da primeira tentativa por unidade.
#define IR_PERIODO_US 200000 // 100 ms ligado + 100 ms desligado em todos os cookies.
#define IR_RAJADA_US 100000
#define IR_ANTECEDENCIA_MS 1000 // Inicia emissao 1 segundo antes do motor.
#define MODO_TESTE_SERIAL 0 // Entrega confirmada pelo sensor infravermelho fisico.
#define BOOT_CONFIRMA_TESTE 1 // BOOT confirma apos o motor parar, mantendo IR ativo.
#define BOTAO_TESTE_GPIO GPIO_NUM_0 // BOOT, ativo em LOW; EN/RESET nao serve.
#define BOTAO_DEBOUNCE_MS 30
#define CAPACIDADE_SLOT 20
#define ESTOQUE_INICIAL 10
#define INTERVALO_COOKIE_MS 2000 // Inclui 1 segundo final com emissor ja ligado.
#define SINCRONIZACAO_MS 5000

// Rejeita configurações incoerentes durante a compilação, antes de gravar a placa.
_Static_assert(IR_RAJADA_US > 1000 && IR_RAJADA_US < IR_PERIODO_US,
               "A rajada IR deve incluir acomodacao e uma pausa entre emissoes");
_Static_assert(INTERVALO_COOKIE_MS >= IR_ANTECEDENCIA_MS,
               "O intervalo deve incluir a antecedencia do emissor");
_Static_assert(ESTOQUE_INICIAL >= 0 && ESTOQUE_INICIAL <= CAPACIDADE_SLOT,
               "Estoque inicial fora da capacidade do compartimento");
