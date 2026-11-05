#pragma once
#include <stdbool.h>
#include <stdint.h>

enum { TESTE_CONFIRMAR=1, TESTE_CANCELAR=2, TESTE_DIAGNOSTICO=4 };
void modo_teste_init(void);
// Um evento por aperto; não guarda confirmações iniciadas durante o motor.
unsigned modo_teste_ler(int64_t agora_ms, bool pode_confirmar);
void modo_teste_diagnostico(void);
