# Barreira infravermelha de entrega

**Modo físico ativo:** `MODO_TESTE_SERIAL=0` em `main/maquina_config.h`.
O infravermelho não bloqueia pedidos por falta de feixe e confirma a entrega
pela sequência livre -> interrompido -> livre. BOOT também confirma uma unidade
para teste após o motor parar (`BOOT_CONFIRMA_TESTE=1`), sem desativar o IR.
Sem passagem confirmada, espera 3 segundos após o motor parar e repete
até duas vezes a tentativa da mesma unidade (três acionamentos no total).
Depois disso continua aguardando IR ou BOOT, sem erro nem novas repetições.
O fim de curso no GPIO32 continua parando motores e bloqueando a máquina para
manutenção. O comando D permanece desativado: `MODO_TESTE_SERIAL=1` seleciona
o modo exclusivamente manual, que desativa confirmação e timeout do IR.
A API expõe `modo_teste_serial`, `botao_teste_ativo` e `sensor_pronto`.

`sensor_ir.c` gera e lê o IR; `modo_teste.c` trata BOOT/UART; `controller.c`
decide quando confirmar o pedido. Pinos e tempos ficam em `maquina_config.h`.

O botão BOOT da placa (GPIO0 ligado ao GND ao apertar) confirma a entrega para
teste. Aguarde o motor parar e aperte e solte BOOT uma vez por unidade,
inclusive após as repetições. Há filtro de 30 ms e
é necessário soltar antes do próximo aperto. Apertos iniciados durante o pulso
são ignorados. EN/RESET reinicia a placa e não confirma entrega. Não segure
BOOT ao ligar/reiniciar, pois GPIO0 seleciona o modo de gravação no ESP32.
É necessário gravar este firmware via USB uma vez para usar o botão na placa.
O serial imprime BOOT PRESSIONADO/SOLTO mesmo sem pedido, com o motivo de um
aperto ignorado. S mostra também GPIO0, modo de teste, estado do pedido e motor.
Sem pedido, apertar BOOT não deve acionar o motor nem mudar estoque.

**Configuração atual:** polaridade original restaurada (`sensor_logica_invertida=false`).
A recepção é um pulso LOW na janela de emissão; HIGH entre rajadas é normal e
não representa, sozinho, obstrução. Esse ajuste não confirma a causa anterior.

Firmware: receptor TSSP58038 no GPIO27; comando do emissor TSAL6200 no GPIO26.
Configuração: 100 ms de portadora de 38 kHz e duty de 50%, seguidos por
100 ms desligado (período de 200 ms), em todos os cookies e repetições.
O LED fica desligado entre entregas. Cada unidade tem um intervalo mínimo
de 2 segundos: 1 segundo de espera e 1 segundo com o LED já pulsando antes
do motor. A captura de passagem só é armada ao iniciar o motor; o preparo
não confirma entrega. O mesmo intervalo se aplica ao início de novos pedidos.
Confirmar, cancelar ou abrir a porta encerra a observação e desliga o LED.
Não há bloqueios por falta de feixe ou timeout de IR. Há filtro de duração
para rejeitar oscilações curtas, configurado em `main/sensor_ir_eventos.h`.
Referência: https://www.vishay.com/docs/49009/pl0438-infrared_sensors.pdf (página 2).

O receptor deve receber 3,3 V e compartilhar GND com o ESP e o circuito emissor.
Conecte OUT ao GPIO27. Confira a pinagem OUT/GND/VS no desenho do fabricante,
sem deduzir a ordem pela foto. Use desacoplamento próximo da alimentação.
O GPIO26 comanda o transistor NPN; não alimenta diretamente o LED.
O resistor da base do 2N2222 não substitui um resistor em série com o TSAL6200:
a corrente do LED precisa ser limitada conforme a tensão de alimentação e seu
datasheet (https://www.vishay.com/docs/81010/tsal6200.pdf).

Ligação informada neste protótipo: receptor em 3,3 V; emissor alimentado em
5 V, com resistor de 1 kΩ em série com o LED e acionamento por 2N2222 com
resistor de 1 kΩ na base. O resistor do LED limita a corrente a poucos mA;
o alcance precisa ser validado com o alinhamento e a distância reais.

## Funcionamento

- Interrupções nas duas bordas registram LOW (livre) -> HIGH (bloqueado)
  -> LOW (livre), durante emissão ativa.
- Alinhe emissor e receptor frente a frente, atravessando o caminho do produto.
- A passagem fica retida até o controlador consumi-la, mesmo entre dois ticks.
  O primeiro 1 ms ao ligar é ignorado para acomodação inicial. Depois, exige
  LOW livre estável por 2 ms, HIGH bloqueado por 2 ms e retorno LOW por 2 ms.
  Cada borda valida a duração do nível anterior; a tarefa também valida níveis
  mantidos. Assim, uma passagem entre ticks pode ficar retida pela ISR.
  Pulsos curtos não são somados. Desligar o emissor descarta sequências ainda
  incompletas e a próxima rajada exige uma nova referência livre. Uma passagem
  já validada permanece retida. Esses 2 ms são valores iniciais de teste:
  ruído mais longo ainda pode confirmar e bloqueios reais menores podem se perder.
  A pausa de 100 ms também continua sendo uma janela sem detecção.
- Não há verificação de feixe livre para aceitar o pedido ou iniciar motores.
- A observação começa junto com o motor, exigindo livre -> interrompido -> livre.
  Sem confirmação, após 3 segundos da parada agenda uma repetição, com o mesmo
  intervalo mínimo de 2 segundos e antecedência do LED de 1 segundo.
- Uma passagem durante os 3 segundos do motor fica retida para confirmação
  ao final do pulso. O pulso permanece de 3 segundos e não termina pelo sensor.
- Cada tentativa limpa a passagem anterior e exige um novo ciclo completo.
  O segundo e o terceiro cookie usam exatamente a mesma captura do primeiro.
- Confirma uma unidade por pulso e só então baixa o estoque.
- Sem ciclo completo, espera 3 segundos após cada pulso terminar e tenta
  novamente no mesmo compartimento, até duas repetições. Cada tentativa tem
  pulso de 3 segundos. O fim de curso continua impedindo motor com porta aberta.
- O limite é reiniciado para cada unidade: primeira tentativa e duas repetições,
  mesmo que o pedido já esteja parcialmente entregue.
- Após três tentativas sem confirmação, permanece aguardando IR ou BOOT,
  sem erro de IR e sem novas repetições. R cancela. O pedido continua em andamento
  até confirmação ou cancelamento; não são aceitos pedidos simultâneos.
  Nenhuma tentativa sem confirmação reduz o estoque.
- Uma passagem durante a espera confirma a unidade e impede a repetição.
  BOOT também confirma nessa espera; se IR e BOOT coincidirem, baixa só uma unidade.
  O contador de tentativas reinicia para cada cookie. Se o IR perder uma queda
  real, a repetição pode liberar um produto extra; valide a detecção física.
- Cancelamento/timeout de pedido parcialmente entregue não registra venda parcial;
  a limitação anterior de recuperação após falta de energia permanece.

## Diagnóstico sem compra

Depois de gravar, abra COM4 a 115200 baud. S mostra nível instantâneo, estado
do feixe, idade da última resposta (-1 se nenhuma), `emissor_ativo`, passagem
retida e total de passagens. No osciloscópio, GPIO26 deve ter
100 ms de emissão/100 ms de pausa desde o segundo de antecedência até a confirmação.
Verifique a temporização real.
As transições imprimem LIVRE ou BLOQUEADO/SEM FEIXE. Interrompa e libere o
feixe manualmente sem pedido: isso não deve acionar o motor nem alterar estoque.
D confirma entrega apenas com `MODO_TESTE_SERIAL=1` e motor parado.

S também mostra contadores TX (ativações do emissor) e RX (ativações com resposta).
Durante emissão, com feixe alinhado, RX deve acompanhar TX; bloqueado, RX deve parar.
Em repouso o emissor fica desligado, portanto TX/RX não avançam.
TX conta as rajadas; o total de passagens é limitado a uma por tentativa.
S também mostra `rejeitados` (bloqueios/retornos curtos descartados durante a
observação), `ultimo_bloqueio` em microssegundos e os três limiares do filtro.
Os contadores não medem todo o ruído analógico; contam eventos digitais observados.

GET /api/status inclui sensor_gpio, emissor_gpio, sensor_feixe_livre,
sensor_pronto, sensor_ultima_resposta_ms, emissor_periodo_ms,
emissor_continuo_ativo e sensor_passagens. Também inclui sensor_pulsos_rejeitados,
sensor_ultimo_bloqueio_us e sensor_filtro_livre_us/bloqueio_us/retorno_us.
Os tempos na API descrevem o ciclo de 100/100 ms durante a entrega;
`emissor_continuo_ativo=false` e `emissor_ativo` indica o ciclo ligado.
Esses campos não verificam eletricamente a corrente do emissor
nem distinguem obstrução, desalinhamento e receptor desconectado.

O filtro de firmware não limpa eletricamente o sinal. Para investigar ruído,
compare OUT e alimentação no osciloscópio durante partida/parada do motor,
com referência no GND do receptor. Confira desacoplamento junto ao receptor,
ligações de GND e passagem dos fios longe dos cabos dos motores. O circuito
de referência Vishay acima (página 2) mostra filtro de alimentação de 100 Ω
em série com VS e 0,47 µF entre VS e GND junto ao receptor; não na saída OUT.
O ajuste precisa ser conferido na montagem real, sem presumir a origem do ruído.

Após conferir a ligação, faça um pedido de uma unidade e passe o produto uma
única vez pelo feixe durante a liberação. Confira confirmação automática e
redução de uma unidade. Teste também ausência de passagem, feixe já bloqueado
e duas unidades, observando o requisito de novo ciclo para cada pulso.
