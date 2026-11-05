/* backend.c — transporte ESP -> Python.
 * Descobre o IPv4 do PC por UDP com desafio HMAC e usa HTTP para a API do banco.
 * Uma trava exclusiva serializa as requisições e protege o endereço em cache.
 * Esta trava é diferente do mutex do controller: espera de rede não segura
 * diretamente o estado físico do motor.
 * backend_secret.h é gerado localmente; não copiar seu conteúdo para documentação.
 */
// lwIP fornece sockets; mbedTLS calcula HMAC-SHA256; FreeRTOS fornece o mutex.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "lwip/sockets.h"
#include "esp_random.h"
#include "mbedtls/md.h"
#include "backend.h"
#include "backend_secret.h"

// gate: uma requisição por vez; base: URL descoberta (por exemplo http://IP:8080).
// O buffer estático inicia zerado; base vazia solicita nova descoberta.
static SemaphoreHandle_t gate;
static char base[80];

// Prepara o mutex antes de API e tarefa_banco começarem a usar este módulo.
void backend_init(void) {
    gate = xSemaphoreCreateMutex();
    configASSERT(gate);
}

/* Busca o Python na sub-rede do AP sem fixar o IP do PC.
 * Envia CE1:<nonce> para 192.168.4.255:8081. O Python responde
 * CE1:<mesmo_nonce>:<porta_http>:<hmac_sha256(nonce:porta)>.
 * A assinatura usa BACKEND_KEY compartilhada. O desafio aleatório dificulta
 * reaproveitar uma resposta antiga; isto autentica a descoberta, não cifra HTTP.
 * Chamador já segura gate. Fecha o socket ao terminar e só grava base se validar.
 */
static bool discover(void) {
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) return false;
    // Broadcast precisa ser habilitado. A recepção tem timeout de 1 segundo.
    // htons converte a porta para ordem de bytes de rede; inet_addr converte IPv4.
    int enabled = 1;
    setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled));
    struct timeval timeout = {.tv_sec=1, .tv_usec=0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    struct sockaddr_in destination = {.sin_family=AF_INET, .sin_port=htons(8081)};
    destination.sin_addr.s_addr = inet_addr("192.168.4.255");
    // Quatro números de 32 bits formam 32 caracteres hexadecimais + NUL.
    char nonce[33], probe[40], reply[160];
    for (int i=0;i<4;i++) snprintf(nonce+8*i, 9, "%08lx", (unsigned long)esp_random());
    snprintf(probe, sizeof(probe), "CE1:%s", nonce);
    sendto(fd, probe, strlen(probe), 0, (struct sockaddr *)&destination, sizeof(destination));
    struct sockaddr_in source;
    socklen_t source_size = sizeof(source);
    bool found = false;
    // Ignora respostas invalidas; ha ate quatro clientes no AP.
    for (int attempt=0;attempt<4;attempt++) {
        int n = recvfrom(fd, reply, sizeof(reply)-1, 0, (struct sockaddr *)&source, &source_size);
        if (n<=0) break;
        reply[n]=0;
        char echoed[33], signature[65], extra;
        unsigned port;
        // Campos limitados pelos tamanhos dos buffers; %c detecta dados extras.
        // Só aceita nonce correto, assinatura de 64 hex e porta válida 1..65535.
        if (sscanf(reply,"CE1:%32[0-9a-f]:%u:%64[0-9a-f]%c",echoed,&port,signature,&extra)!=3 ||
            strcmp(echoed,nonce)!=0 || strlen(signature)!=64 || port<1 || port>65535) continue;
        char message[48], expected[65]; unsigned char digest[32];
        snprintf(message,sizeof(message),"%s:%u",nonce,port);
        // Recalcula a assinatura local: nunca enviar a chave no pacote de descoberta.
        mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
                        (const unsigned char *)BACKEND_KEY,strlen(BACKEND_KEY),
                        (const unsigned char *)message,strlen(message),digest);
        for (int i=0;i<32;i++) snprintf(expected+2*i,3,"%02x",digest[i]);
        // Compara todos os 64 caracteres, acumulando diferenças em vez de sair
        // no primeiro caractere desigual. Qualquer diferença invalida a resposta.
        unsigned difference=0;
        for (int i=0;i<64;i++) difference |= (unsigned char)signature[i] ^ (unsigned char)expected[i];
        if (difference) continue;
        // O IP vem do remetente UDP; a porta HTTP vem do anúncio autenticado.
        // Não assumir que o PC sempre recebe 192.168.4.2 do DHCP.
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET,&source.sin_addr,ip,sizeof(ip));
        snprintf(base,sizeof(base),"http://%s:%u",ip,port);
        found=true; break;
    }
    close(fd);
    return found;
}

/* Executa uma chamada JSON autenticada na API do Python.
 * method/path/body são emprestados. authorization é opcional e pertence ao usuário;
 * X-ESP-Key identifica a placa. O navegador nunca precisa conhecer BACKEND_KEY.
 * Inicializa *response=NULL. Em resposta JSON válida, o chamador recebe a árvore
 * e deve destruí-la. Retorna o status recebido, ou 503 se transporte/JSON falhar.
 * Aguarda gate por até 10 s; o cliente tem timeout de E/S de 5 s. Esses valores
 * são etapas, não um limite global único para toda a operação.
 */
int backend_request(esp_http_client_method_t method, const char *path,
                    const char *body, const char *authorization, cJSON **response) {
    *response=NULL;
    if (xSemaphoreTake(gate,pdMS_TO_TICKS(10000))!=pdTRUE) return 503;
    int code=503;
    if (!base[0] && !discover()) goto done;
    char url[160]; snprintf(url,sizeof(url),"%s%s",base,path);
    // Redirecionamento automático é desativado para não reenviar os cabeçalhos
    // de autenticação a outro destino. Endereço e resposta têm limites explícitos.
    esp_http_client_config_t cfg={.url=url,.timeout_ms=5000,.disable_auto_redirect=true,.buffer_size=1024};
    esp_http_client_handle_t client=esp_http_client_init(&cfg);
    if (!client) goto done;
    esp_http_client_set_method(client,method);
    esp_http_client_set_header(client,"X-ESP-Key",BACKEND_KEY);
    esp_http_client_set_header(client,"Content-Type","application/json");
    if (authorization && authorization[0]) esp_http_client_set_header(client,"Authorization",authorization);
    int length=body ? (int)strlen(body):0;
    if (esp_http_client_open(client,length)!=ESP_OK) goto cleanup;
    // Escrita HTTP pode ser parcial; só prossegue após enviar todo o corpo.
    for (int sent=0;sent<length;) {
        int n=esp_http_client_write(client,body+sent,length-sent);
        if(n<=0) goto cleanup;
        sent+=n;
    }
    if(esp_http_client_fetch_headers(client)<0) goto cleanup;
    // Até 24575 bytes de resposta útil: atingir 24576 é tratado como grande demais.
    // O byte adicional permite NUL sem ultrapassar o buffer. Ler não garante
    // mensagem completa; o teste do SDK abaixo também precisa confirmar o término.
    char *raw=malloc(24577);
    if(!raw) goto cleanup;
    int used=0;
    while(used<24576) {
        int n=esp_http_client_read(client,raw+used,24576-used);
        if(n<0) {free(raw);goto cleanup;}
        if(n==0) break;
        used+=n;
    }
    raw[used]=0;
    if(used<24576 && esp_http_client_is_complete_data_received(client)) *response=cJSON_Parse(raw);
    free(raw);
    if(*response) code=esp_http_client_get_status_code(client);
// Saída comum após abrir o handle: cleanup fecha/libera o cliente HTTP.
// Falha 503 invalida o IP em cache; a próxima chamada tenta descobrir de novo.
cleanup:
    esp_http_client_cleanup(client);
    if(code==503) base[0]=0; // Redescobre IP depois de falha ou troca de rede.
// Todo caminho que adquiriu gate passa por esta liberação, inclusive erros.
done:
    xSemaphoreGive(gate);
    return code;
}
