#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "cJSON.h"
#include "maquina_config.h"

// Estado durável; o controlador é o dono e protege todo acesso com seu mutex.
typedef struct {
    int estoque[NUM_SABORES];
    uint32_t ultima_venda_id;
    char ultima_venda_usuario[128];
    int ultima_venda_qtd[NUM_SABORES], ultima_venda_total;
    char venda_pendente[1024];
    bool falha_persistencia;
    bool manutencao;
} persistencia_t;

void persistencia_carregar(persistencia_t *dados);
void persistencia_salvar_estoque(persistencia_t *dados, int slot);
uint32_t persistencia_proximo_id(void);
void persistencia_salvar_venda(persistencia_t *dados, uint32_t id,
    const char *usuario, const int *quantidades, int total, const char *evento);
void persistencia_limpar_pendente(persistencia_t *dados);
// A manutenção permanece após reiniciar. Falha ao salvar mantém compras bloqueadas.
bool persistencia_manutencao(persistencia_t *dados, bool ativa);
// JSON novo, a liberar com cJSON_Delete. Mantém a rota legada da última venda.
cJSON *persistencia_ultima_venda(const persistencia_t *dados);
