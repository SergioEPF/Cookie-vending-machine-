# Diagnóstico do Wi-Fi — 30/10/2026

## Evidências

- O celular acessou `/api/status` e confirmou `versao: 4.0-python`, identificando
  a placa e o firmware atuais.
- O Windows usava o Realtek RTL8188CU USB, ID `VID_0BDA&PID_8176`, com driver
  Microsoft/Realtek `1027.4.630.2015`, datado de 04/03/2016.
- Os eventos WLAN 8002 informaram “A rede específica não está disponível”, antes
  da conclusão da associação. Não havia filtros de SSID; o perfil WPA2/CCMP era
  compatível com o ESP. O driver declarava suporte a 802.11b/g/n.
- O adaptador chegou a retornar zero redes após uma nova varredura. Reiniciá-lo
  por software, sozinho, não resolveu.
- O rádio do ESP estava em largura de 40 MHz, com protocolos b/g/n e potência
  máxima reportada de 19,5 dBm. O teste isolado no canal 11 não resolveu a detecção.
- Após o usuário trocar o adaptador de porta USB e o firmware passar a usar 20 MHz,
  a rede COOKIE_EXPRESS apareceu com sinal de 100%. Outras redes também voltaram
  a aparecer. O Windows conectou e recebeu `192.168.4.2`.
- O Python recebeu snapshots do ESP (`192.168.4.1`) com HTTP 200. Depois houve
  instabilidade: o Windows permanecia associado, mas consultas deixaram de responder.
- O teste com suspensão seletiva USB desativada (`EnableUsbSS: 1 → 0`) teve
  respostas iniciais, mas falhou durante a observação prolongada. A propriedade
  foi restaurada para 1; esse teste sozinho não resolveu a instabilidade.
- O teste separado com economia de energia do rádio desativada
  (`bFwCtrlLPS: 1 → 0`) respondeu a quatro consultas durante cerca de um minuto,
  mas também apresentou timeout. A propriedade foi restaurada para 1. Conexões
  HTTP vinculadas explicitamente ao IP Wi-Fi também falharam, enquanto o Python
  no computador continuava respondendo em localhost.
- Durante a falha do computador, o usuário confirmou que o celular continuava
  abrindo o status do ESP. Isso localiza a interrupção no caminho do PC/Realtek,
  sem evidência de uma trava geral da placa ou do servidor HTTP.

- Com as duas propriedades de energia restauradas, o teste alterou somente o
  `WirelessMode` do Realtek de 8 (b/g/n) para 4 (b/g). O Windows negociou 802.11g,
  e cinco consultas ao status durante 80 segundos tiveram sucesso. Também passaram
  a leitura das duas páginas, leitura dos seis slots, login admin, consulta de
  sessão, consulta de vendas e logout, todos pelo ESP32.
- Uma leitura adicional logo após o teste funcional apresentou novo timeout em
  b/g. Portanto, o teste não confirmou estabilidade nem uma causa específica do
  modo N. O modo b/g/n original foi restaurado. Os resultados funcionais mostram
  que a integração opera quando há tráfego, mas não resolvem a falha intermitente.

Esses resultados localizam a instabilidade no caminho PC/Realtek, mas não distinguem
falha física, USB, driver ou uma incompatibilidade nessa combinação de equipamentos.
É necessário comparar com outro adaptador/computador para separar essas hipóteses.
A recuperação da detecção inicial também envolveu troca de porta USB e ajuste
para 20 MHz, cujos efeitos individuais não foram isolados. A idade do driver,
sozinha, não comprova a causa.

## Configuração mantida

- ESP: SSID COOKIE_EXPRESS, WPA2, senha anterior, canal 11 e largura HT20.
- Realtek: modo b/g/n original restaurado (WirelessMode 8).
- `main/main.c` registra canal, protocolos, largura e potência na inicialização;
  `/api/status` também informa `wifi_canal`.
- As duas opções de energia testadas foram restauradas aos valores originais e
  têm backup local. Nenhum driver
  foi instalado ou removido. Ethernet e planos globais de energia não foram
  alterados.
- O monitor serial é encerrado ao fim da leitura; nenhum motor foi acionado.
- O teste não alterou estoque nem registrou vendas. O histórico estava vazio.

A API de configuração de largura é documentada pela
[Espressif para o ESP-IDF 5.5.6](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32/api-guides/wifi.html).

## Repetir a verificação ou restaurar opções de energia

Verificação somente de leitura, exceto criação e revogação de uma sessão de teste:

```powershell
& C:\Espressif\tools\python\python.exe servidor/testes/diagnosticos/verify_connection.py --samples 5 --interval 20
```

Consulta o status por cerca de 80 segundos, lê IHM, backoffice e seis slots e
valida login administrativo e histórico pelo ESP. Não registra pedidos, vendas,
nem altera estoque. Usa a senha inicial local; se ela tiver sido trocada no banco,
atualize o procedimento de login antes de repetir a verificação.

O backup da propriedade do adaptador está em
`servidor/testes/.runtime/diagnosticos/wifi-power-before.json`. Para restaurá-la, em PowerShell **como
administrador**:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/testes/diagnosticos/test_wifi_power.ps1 -Restore
```

Esse comando reinicia apenas o Realtek investigado. Os scripts verificam o modelo
do adaptador e, na restauração, seu GUID, para evitar alterar outro dispositivo.
Para restaurar a economia do rádio, se ela estiver alterada, use
`-Property bFwCtrlLPS -Restore`. O backup correspondente é
`servidor/testes/.runtime/diagnosticos/wifi-power-bFwCtrlLPS-before.json`.
O teste do modo de rádio usa `servidor/testes/diagnosticos/test_wifi_legacy.ps1`: valor 4 representa
802.11b/g, conforme o INF instalado; o original 8 representa b/g/n. Para restaurar,
execute esse script como administrador com `-Restore`. O backup fica em
`servidor/testes/.runtime/diagnosticos/wifi-mode-before.json`.
