/* API HTTP pública da placa. Implementação em api.c.
 * Este cabeçalho expõe somente a inicialização; detalhes das rotas ficam internos.
 */
#pragma once

// Cria o servidor na porta 80 e registra GET, POST e OPTIONS; chamar após o Wi-Fi.
void api_start(void);
