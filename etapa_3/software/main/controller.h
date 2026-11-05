// API do controlador. JSON retornado pertence ao chamador (cJSON_Delete).
// Argumentos são emprestados; os dados necessários ao pedido são copiados.
#pragma once

#include <stdbool.h>
#include "backend.h"

// Cria mutex/driver, desliga GPIO e carrega NVS; chamar antes de expor a API.
void controller_init(void);
// Inicia tarefas de motor/serial e sincronização; chamar depois da rede.
void controller_start(void);

// JSON retornado pertence ao chamador. As funcoes protegem o estado com mutex.
// Snapshot de flags e versão do ESP; não devolve senha ou token.
cJSON *controller_status(void);
// Seis slots: estoque atual da RAM, sabores e capacidades.
cJSON *controller_inventory(void);
// Última venda local; autorização admin é obrigação do chamador da API.
cJSON *controller_last_sale(void);
// Consulta vinculada ao token do pedido; escreve o status HTTP em *status.
cJSON *controller_order_status(const char *token, int *status);
// Valida 1..3 unidades e agenda o motor; buyer vem da sessão autenticada.
cJSON *controller_place_order(cJSON *body, const char *buyer, const char *token, int *status);
// replenish=true soma, false define estoque; acesso admin validado pela API.
cJSON *controller_adjust_stock(cJSON *body, bool replenish, int *status);
// Somente a API administrativa pode chamar; exige chave de fim de curso liberada.
cJSON *controller_clear_maintenance(int *status);

// Transporte para o Python, atualizando a disponibilidade mostrada na IHM.
// Não segura o mutex físico durante HTTP. *response, quando não NULL, é do chamador.
// Disponibilidade é estimada a partir das respostas recentes do transporte.
int controller_request(esp_http_client_method_t method, const char *path,
                       const char *body, const char *authorization, cJSON **response);
