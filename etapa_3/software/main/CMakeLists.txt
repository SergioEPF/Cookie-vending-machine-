# Registra os módulos do firmware; a IHM é servida pelo Python no PC.
# Os cabeçalhos .h são incluídos pelos fontes; não são programas separados.
idf_component_register(
    SRCS "modo_teste.c" "main.c" "api.c" "controller.c" "backend.c"
         "sensor_ir.c" "persistencia.c" "acionamento.c"
    INCLUDE_DIRS "." # Todos os módulos compilados do firmware ficam em main/.
    # Dependências do SDK: rede/API, relógio, NVS, GPIO/UART, JSON e HMAC.
    REQUIRES esp_wifi esp_event esp_netif esp_http_server esp_http_client
             esp_timer nvs_flash driver esp_driver_gpio esp_driver_uart esp_driver_ledc json mbedtls
)
# Não há EMBED_FILES: IHM e backoffice são servidos pelo PC, não pela flash do ESP.
