#pragma once
#include <stdbool.h>

void acionamento_init(void);
void acionamento_parar(void);
// Slot interno 0..5; somente um motor por vez. Recusa se a chave estiver ativa.
bool acionamento_ligar(int slot);
bool fim_curso_acionado(void);
