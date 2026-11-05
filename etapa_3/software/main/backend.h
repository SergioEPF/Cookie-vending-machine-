/* Interface do cliente HTTP que conversa com o Python.
 * Depende de cJSON para árvores JSON e do enum de métodos HTTP do ESP-IDF.
 * Os tipos incluídos aqui também são usados pelo controller.h.
 */
#pragma once
#include "cJSON.h"
#include "esp_http_client.h"

// Cria o mutex de transporte; executar uma vez antes das tarefas concorrentes.
void backend_init(void);
// Retorna status HTTP; 503 em falha de transporte/descoberta.
// body/authorization podem ser NULL. A função não assume propriedade dos argumentos.
// Escreve uma árvore alocada em *response; destruir com cJSON_Delete após utilizar.
// Descoberta e chamadas são serializadas, com limites de tamanho e timeouts.
int backend_request(esp_http_client_method_t method, const char *path,
                    const char *body, const char *authorization, cJSON **response);
