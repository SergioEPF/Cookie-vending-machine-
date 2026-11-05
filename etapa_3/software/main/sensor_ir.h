#pragma once
#include <stdbool.h>
#include "cJSON.h"

void sensor_ir_init(void);
// Após init, chamadas feitas sob o mutex do controlador; ISR usa trava própria.
void sensor_ir_atualizar(void);
bool sensor_ir_livre(void);
bool sensor_ir_pronto(void);
// Inicia o ciclo 100/100 ms sem contar passagem durante a antecedencia do LED.
void sensor_ir_preparar_emissao(void);
// Arma a captura no inicio do motor, preservando o ciclo ja iniciado.
void sensor_ir_iniciar_observacao(void);
void sensor_ir_encerrar_observacao(void);
bool sensor_ir_passagem_detectada(void);
void sensor_ir_status(cJSON *json);
void sensor_ir_diagnostico(void);
