// API publica do ESP. O PC fornece os HTMLs e o Python autentica as contas.
/* api.c — servidor HTTP da placa: JSON, autenticação e despacho de rotas.
 * O navegador baixa HTML no PC e chama http://192.168.4.1 para operar a máquina.
 * Contas e histórico são encaminhados ao Python; motor/estoque ficam no controller.
 * Nenhuma rota deste arquivo serve o conteúdo HTML da IHM ou do backoffice.
 */
// SDK HTTP: requisições/respostas; Wi-Fi: canal e quantidade de clientes no AP.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "controller.h"
#include "api.h"

// Limite do corpo recebido pela placa em bytes, antes da alocação do buffer.
#define BODY_MAX 2048

// Cria uma árvore JSON de erro. reply assume sua propriedade e a destrói.
static cJSON *error(const char *message) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddBoolToObject(json, "ok", false);
    cJSON_AddStringToObject(json, "erro", message);
    return json;
}

/* Libera as chamadas feitas pelo HTML do PC a uma origem diferente (ESP).
 * OPTIONS é o preflight do navegador. CORS não autentica usuários; Bearer e
 * permissões são verificados pelas rotas. Não substitui HTTPS nem protege a rede.
 */
static void cors(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type, Authorization");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Private-Network", "true");
}

/* Envia JSON e consome o objeto recebido, inclusive na falha de serialização.
 * cJSON_PrintUnformatted cria uma string separada; ela é liberada após o envio.
 * no-store evita guardar respostas de sessão/estoque em cache HTTP.
 * O código numérico é o status real; a frase auxiliar é OK ou Error.
 */
static esp_err_t reply(httpd_req_t *req, int code, cJSON *json) {
    char *body = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    cors(req);
    if (!body) return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Sem memoria");
    char status[32];
    snprintf(status, sizeof(status), "%d %s", code, code == 200 ? "OK" : "Error");
    httpd_resp_set_status(req, status);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    esp_err_t result = httpd_resp_sendstr(req, body);
    free(body);
    return result;
}

/* Lê o corpo completo (1..2048 bytes) e exige um objeto JSON.
 * Uma recepção pode vir fragmentada: o laço acumula até content_len.
 * calloc reserva um byte extra já zerado para terminar a string C.
 * Em erro, a função já responde HTTP e retorna NULL; não responder duas vezes.
 * Em sucesso, o chamador destrói a árvore com cJSON_Delete.
 */
static cJSON *read_body(httpd_req_t *req) {
    if (req->content_len <= 0 || req->content_len > BODY_MAX) {
        reply(req, 400, error("Corpo JSON ausente ou maior que 2048 bytes"));
        return NULL;
    }
    char *raw = calloc(req->content_len + 1, 1);
    if (!raw) {
        reply(req, 503, error("Sem memoria"));
        return NULL;
    }
    int received = 0;
    while (received < req->content_len) {
        int size = httpd_req_recv(req, raw + received, req->content_len - received);
        if (size <= 0) {
            free(raw);
            reply(req, 400, error("Corpo JSON incompleto"));
            return NULL;
        }
        received += size;
    }
    cJSON *json = cJSON_Parse(raw);
    free(raw);
    if (!cJSON_IsObject(json)) {
        cJSON_Delete(json);
        reply(req, 400, error("Objeto JSON invalido"));
        return NULL;
    }
    return json;
}

// Copia o cabeçalho completo "Bearer <token>" para um buffer de 96 bytes.
// Inicializa vazio: cabeçalho ausente/inadequado não deve reutilizar memória antiga.
static void authorization(httpd_req_t *req, char token[96]) {
    token[0] = 0;
    httpd_req_get_hdr_value_str(req, "Authorization", token, 96);
}

// Retorna ponteiro emprestado de um campo string; não liberar separadamente.
// O ponteiro só continua válido enquanto o JSON pai existir.
static const char *string(cJSON *json, const char *key) {
    cJSON *value = cJSON_GetObjectItemCaseSensitive(json, key);
    return cJSON_IsString(value) ? value->valuestring : NULL;
}

/* Verifica o Bearer no Python por /api/auth/me e, se exigido, role=admin.
 * name é opcional; quando presente, o chamador reserva pelo menos 64 bytes.
 * O limite em BYTES evita ultrapassar esse destino com nomes UTF-8 acentuados.
 * Não confia no nome nem no papel enviados pelo navegador no corpo do pedido.
 * Destrói a resposta do Python depois de copiar os dados necessários.
 */
static int authorize(httpd_req_t *req, bool admin, char *name) {
    char token[96];
    authorization(req, token);
    if (strncmp(token, "Bearer ", 7) != 0 || !token[7]) return 401;
    cJSON *json = NULL;
    int code = controller_request(HTTP_METHOD_GET, "/api/auth/me", NULL, token, &json);
    if (code == 200) {
        cJSON *user = cJSON_GetObjectItemCaseSensitive(json, "usuario");
        const char *user_name = string(user, "nome"), *role = string(user, "role");
        if (!user_name || strlen(user_name) > 63) code = 503;
        else if (admin && (!role || strcmp(role, "admin") != 0)) code = 403;
        else if (name) strcpy(name, user_name);
    }
    cJSON_Delete(json);
    return code;
}

/* Ponte para login/cadastro/sessão e consulta de vendas.
 * Preserva método, caminho, Authorization, status e JSON da resposta do Python.
 * A chave privada de máquina é acrescentada somente pelo backend do ESP.
 * Para /api/vendas GET, o próprio Python exige admin. Login/cadastro não exigem
 * sessão anterior; suas validações também pertencem ao Python.
 */
static esp_err_t proxy(httpd_req_t *req) {
    char token[96];
    authorization(req, token);
    char *raw = NULL;
    if (req->method == HTTP_POST) {
        cJSON *body = read_body(req);
        if (!body) return ESP_OK; // read_body ja respondeu ao erro.
        raw = cJSON_PrintUnformatted(body);
        cJSON_Delete(body);
        if (!raw) return reply(req, 503, error("Sem memoria"));
    }
    cJSON *json = NULL;
    int code = controller_request(req->method == HTTP_POST ? HTTP_METHOD_POST : HTTP_METHOD_GET,
                                  req->uri, raw, token, &json);
    free(raw);
    if (!json) json = error("Banco indisponivel. Conecte o PC ao Wi-Fi do ESP e inicie o Python.");
    return reply(req, code, json);
}

// Acrescenta canal Wi-Fi e número de estações ao snapshot operacional do controller.
static esp_err_t status(httpd_req_t *req) {
    cJSON *json = controller_status();
    uint8_t channel = 0;
    wifi_second_chan_t secondary;
    if (esp_wifi_get_channel(&channel, &secondary) == ESP_OK)
        cJSON_AddNumberToObject(json, "wifi_canal", channel);
    wifi_sta_list_t clients = {0};
    esp_wifi_ap_get_sta_list(&clients);
    cJSON_AddNumberToObject(json, "clientes_conectados", clients.num);
    return reply(req, 200, json);
}

// Consulta pública do estoque físico; o JSON criado é consumido por reply.
static esp_err_t inventory(httpd_req_t *req) {
    return reply(req, 200, controller_inventory());
}

// Polling do pedido: o controller compara a sessão com a que aceitou o pedido.
static esp_err_t order_status(httpd_req_t *req) {
    char token[96];
    authorization(req, token);
    int code;
    cJSON *json = controller_order_status(token, &code);
    return reply(req, code, json);
}

// A última venda inclui usuário; esta rota exige sessão de administrador.
static esp_err_t last_sale(httpd_req_t *req) {
    int code = authorize(req, true, NULL);
    if (code != 200) return reply(req, code, error("Acesso administrativo necessario"));
    return reply(req, 200, controller_last_sale());
}

// Retirada do bloqueio exige administrador e chave fisicamente liberada.
static esp_err_t maintenance_clear(httpd_req_t *req) {
    int code=authorize(req,true,NULL);
    if(code!=200) return reply(req,code,error("Acesso administrativo necessario"));
    cJSON *body=read_body(req);
    if(!body) return ESP_OK;
    cJSON_Delete(body);
    cJSON *json=controller_clear_maintenance(&code);
    return reply(req,code,json);
}

/* Entrada comum das mutações locais: pedido de cliente ou estoque de admin.
 * Autoriza ANTES de aceitar o corpo e passar ao controller.
 * O controller apenas empresta body, portanto este handler deve destruí-lo.
 * Ajustar estoque não aciona motor; fazer pedido agenda a tarefa de controle.
 */
static esp_err_t change(httpd_req_t *req) {
    bool order = strcmp(req->uri, "/api/pedido") == 0;
    char buyer[64], token[96];
    int code = authorize(req, !order, buyer);
    if (code != 200) return reply(req, code, error("Sessao invalida, acesso negado ou banco indisponivel"));
    cJSON *body = read_body(req);
    if (!body) return ESP_OK;
    authorization(req, token);
    cJSON *json = order ? controller_place_order(body, buyer, token, &code) :
        controller_adjust_stock(body, strcmp(req->uri, "/api/estoque/reposicao") == 0, &code);
    cJSON_Delete(body);
    return reply(req, code, json);
}

/* Roteador: um handler curinga para cada método, com tabela de caminhos exatos.
 * OPTIONS responde 204 para o navegador; os demais consultam método + URI.
 * Os caminhos atuais não aceitam sufixos ou query strings arbitrárias, pois
 * a comparação usa strcmp no req->uri. Recurso desconhecido devolve JSON 404.
 */
static esp_err_t route(httpd_req_t *req) {
    // Um unico preflight atende os HTMLs hospedados no PC.
    if (req->method == HTTP_OPTIONS) {
        cors(req);
        httpd_resp_set_status(req, "204 No Content");
        return httpd_resp_send(req, NULL, 0);
    }
    // Cada entrada associa caminho e método a uma função do tipo handler HTTP.
    static const struct {
        const char *path;
        httpd_method_t method;
        esp_err_t (*handler)(httpd_req_t *);
    } routes[] = {
        {"/", HTTP_GET, status},
        {"/api/status", HTTP_GET, status},
        {"/api/estoque", HTTP_GET, inventory},
        {"/api/pedido/status", HTTP_GET, order_status},
        {"/api/venda/ultima", HTTP_GET, last_sale},
        {"/api/vendas", HTTP_GET, proxy}, // O Python exige sessao admin nesta rota.
        {"/api/auth/register", HTTP_POST, proxy},
        {"/api/auth/login", HTTP_POST, proxy},
        {"/api/auth/me", HTTP_GET, proxy},
        {"/api/auth/logout", HTTP_POST, proxy},
        {"/api/pedido", HTTP_POST, change},
        {"/api/estoque/reposicao", HTTP_POST, change},
        {"/api/estoque/ajuste", HTTP_POST, change},
        {"/api/manutencao/liberar", HTTP_POST, maintenance_clear},
    };
    for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++)
        if (req->method == routes[i].method && strcmp(req->uri, routes[i].path) == 0)
            return routes[i].handler(req);
    return reply(req, 404, error("Rota nao encontrada. Abra a IHM e o backoffice no servidor do PC."));
}

/* Sobe o servidor do ESP na porta HTTP padrão 80.
 * Há três registros curinga (GET/POST/OPTIONS), não um registro SDK por endpoint.
 * A tabela em route faz o despacho final. A pilha HTTP tem 8192 bytes; envio tem
 * timeout de 10 s. ESP_ERROR_CHECK interrompe a partida se o servidor falhar.
 */
void api_start(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_uri_handlers = 3;
    config.stack_size = 8192;
    config.send_wait_timeout = 10;
    ESP_ERROR_CHECK(httpd_start(&server, &config));
    httpd_uri_t uri = {.uri = "/*", .handler = route};
    const httpd_method_t methods[] = {HTTP_GET, HTTP_POST, HTTP_OPTIONS};
    for (size_t i = 0; i < sizeof(methods) / sizeof(methods[0]); i++) {
        uri.method = methods[i];
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &uri));
    }
}
