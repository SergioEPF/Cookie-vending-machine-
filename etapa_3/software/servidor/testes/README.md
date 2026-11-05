# Testes, demonstração e diagnósticos

Os testes ficam em `servidor/testes/`. O firmware fica em `main/`, e o backend
e as telas em `servidor/`. Referências antigas foram preservadas em
`main/historico/` e `servidor/historico/`.

| Pasta | Finalidade | Usa hardware / dados reais? |
| --- | --- | --- |
| `demonstracao/` | Simulador local e comparação histórica das telas | Não; banco temporário |
| `automacao/` | Testes HTTP, autenticação, SQLite e descoberta UDP | Não; portas e bancos temporários |
| `servidor/ihm/` | Cenários JavaScript da IHM e backoffice, com API simulada | Não; navegador com perfil temporário |
| `diagnosticos/` | Serial, integração real e experimentos do adaptador Wi-Fi | Sim; executar apenas conforme a necessidade |
| `firmware/` | D/R/S pela serial e botão BOOT do firmware | Sim; D/BOOT pode confirmar entrega e baixar estoque |
| `.runtime/` | Logs, PID do simulador e backups de diagnóstico | Local; ignorada pelo Git |

Execute os comandos abaixo **na raiz do projeto**.

## Demonstração sem ESP

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/testes/demonstracao/iniciar.ps1
```

Abra <http://127.0.0.1:8082/>. Cliente: `teste` / `teste123`.
Backoffice: <http://127.0.0.1:8082/backoffice>, com `admin` / `admin123`.
São contas exclusivas do simulador. Também é possível cadastrar clientes de teste.

O simulador serve os mesmos HTMLs de `servidor/ihm/`, alterando a API somente na cópia
enviada ao navegador. Cada cookie é entregue automaticamente após 3 segundos;
estoque e vendas aparecem no backoffice simulado. `/simulador` permite acionar
e soltar o fim de curso. Para liberar compras, solte a chave virtual e confirme
a manutenção no backoffice.

Tudo roda no PC, sem conexão com ESP nem com o banco real. O processo escuta
somente em `127.0.0.1`. Dados são descartados ao encerrar e recriados no próximo
processo. Ctrl+C encerra a execução em primeiro plano. `-Background` inicia
oculto e `-Background -Restart` reinicia com os dados iniciais.
PID e logs ficam em `servidor/testes/.runtime/demo/`.

`demonstracao/comparativo_ihm_esp32.html` é uma referência histórica visual,
independente do simulador e da versão operacional das telas.

## Verificações automáticas

```powershell
& 'C:/Espressif/tools/python/python.exe' -m unittest discover -s servidor/testes/automacao -v
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/testes/ihm/run.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/testes/ihm/run.ps1 -Page backoffice.html -Behavior backoffice.js -MockPath backoffice-mock.js
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/testes/firmware/test_sensor_ir.ps1
```

O teste de IR avalia 17 cenários da captura com
`static_assert` C++14 no compilador ESP-IDF, sem executar código na placa:
passagem entre ticks, três cookies, confirmação tardia sem timeout,
desligamento intencional, referência inicial, contagem única, acomodação,
cancelamento e ausência de confirmação durante o preparo do LED. Inclui limites
de 2 ms, picos de 100/500 µs, ruído repetido sem acumulação, retorno e referência
curtos, descarte de sequência incompleta na pausa e retenção da passagem válida.
Não testa a instalação física nem a pontualidade das callbacks na placa.

Python 3.10+ usa apenas biblioteca padrão. Os testes das telas exigem Chrome
ou Edge; use `-BrowserPath` se necessário. O executor cria e remove somente
seu diretório `.run-*` verificado. Esses testes não comprovam funcionamento
elétrico dos sensores, motores e fim de curso.

## Diagnóstico com hardware

```powershell
# Consulta a integração, cria/revoga uma sessão admin; não faz pedidos.
& 'C:/Espressif/tools/python/python.exe' servidor/testes/diagnosticos/verify_connection.py

# Leitura serial; requer pyserial. --reset reinicia a placa se solicitado.
& 'C:/Espressif/tools/python/v5.5.6/venv/Scripts/python.exe' servidor/testes/diagnosticos/check_device.py --port COM4 --seconds 12
```

Os scripts `restart_wifi.ps1`, `test_wifi_power.ps1` e `test_wifi_legacy.ps1`
reiniciam/alteram o adaptador Realtek investigado. Não integram os testes
automáticos. Consulte [o diagnóstico do Wi-Fi](../docs/diagnostico_wifi.md)
antes de usá-los. Backups existentes foram preservados em
`servidor/testes/.runtime/diagnosticos/` nesta instalação; não são publicados no Git.

## Comandos de teste no firmware

`firmware/modo_teste.c` e `.h` continuam compilados pelo componente `main`.
`MODO_TESTE_SERIAL` em `main/maquina_config.h` seleciona a confirmação:
atualmente `0`, com confirmação pelo IR físico. `BOOT_CONFIRMA_TESTE=1` mantém
BOOT como confirmação manual após o motor parar; D exige `MODO_TESTE_SERIAL=1`.
R cancela e S diagnostica o IR.
Esses comandos não são uma simulação isolada: afetam o estoque da placa e
podem produzir vendas reais no banco ao concluir um pedido. Não ignoram o
bloqueio de manutenção. Mover o módulo de pasta não alterou seu comportamento.
