# Motores e manutenção por fim de curso

O mapa segue o ESP32-DEVKIT-V1 do esquema fornecido. Os números são GPIOs,
não a posição física nos conectores. Pinos e tempos ficam em `main/maquina_config.h`;
as saídas e a entrada da chave são tratadas por `main/acionamento.c`.

| Compartimento | Sabor | GPIO do motor |
| --- | --- | --- |
| 1 | Chocolate | 18 |
| 2 | Chocolate Branco | 19 |
| 3 | Red Velvet | 21 |
| 4 | Baunilha | 22 |
| 5 | M&M's | 23 |
| 6 | Integral | 25 |

Os seis GPIOs comandam os circuitos de potência existentes na placa. Todos
iniciam em LOW. Só um motor é ligado por vez, com pulso de 3 segundos para cada
unidade. Entre unidades há intervalo mínimo de 2 segundos, incluindo 1 segundo
com o LED IR pulsando antes do motor. Pedidos continuam limitados a três
cookies, podendo combinar os seis compartimentos.

O IR permanece no GPIO26 (38 kHz com 100 ms ligado/100 ms desligado) e
GPIO27 (receptor). O modo físico está ativo: a passagem no feixe confirma
a entrega. O fim de curso tem prioridade e não pode ser ignorado por D, BOOT ou R.

## Chave no GPIO32

A lógica foi invertida novamente para a montagem da porta: aberta conecta GPIO32 ao GND.
O firmware habilita pull-up interno em 3,3 V; LOW = porta aberta (bloqueio),
HIGH = porta fechada. Com essa polaridade, um fio desconectado indica HIGH;
a leitura não distingue fio desconectado de porta fechada.
Não há filtro temporal na chave: uma leitura ativa já solicita o bloqueio.

Ao detectar acionamento, o controlador desliga os seis motores, interrompe o
pedido ativo sem confirmar unidades extras e ativa manutenção. A manutenção
é salva na chave NVS `maint`. Com a porta fechada, o controlador remove esse
bloqueio automaticamente, inclusive após reiniciar, sem retomar pedidos interrompidos.
Uma falha ao gravar mantém o bloqueio em RAM e sinaliza falha de persistência.

A leitura ocorre no laço de controle (um tick, nominalmente 10 ms), e também
antes de iniciar motores ou confirmar entrega. Esse tempo depende do
escalonamento e do mutex: o firmware não substitui um intertravamento elétrico.

Pedidos interrompidos não são registrados como vendas completas. Unidades já
confirmadas permanecem abatidas; produtos que caíram sem confirmação exigem
conferência manual de estoque. Não foi implementado estorno ou venda parcial.

## IHM e liberação automática

A IHM exibe uma tela de manutenção mesmo antes do login, bloqueia seleção e
envio de compras e informa contagem parcial quando o pedido foi interrompido.
O aviso acompanha a consulta periódica de status (5 segundos) e as respostas
de pedido; sem conexão, a página não recebe imediatamente mudanças da placa.
O botão de exportar JSON foi removido. O cache local continua apenas como apoio;
não autentica nem autoriza compras.

Para retirar o bloqueio:

1. Verifique a máquina e confira a entrega/estoque de pedidos interrompidos.
2. Feche a porta: GPIO32 deve ficar em HIGH.
3. Aguarde a atualização da IHM; a liberação não exige confirmação no backoffice.

A API `POST /api/manutencao/liberar`, com corpo `{}`, exige sessão admin e
recusa a operação enquanto GPIO32 estiver ativo. Ela não liga nenhum motor
nem retoma pedidos interrompidos. As demais condições de compra continuam
valendo (estoque, sessão, servidor disponível e ausência de falha de gravação).

`GET /api/status` informa `manutencao`, `fim_curso_acionado`, `fim_curso_gpio`
e `mensagem_manutencao`. O status do pedido também informa `manutencao`.

## Verificação física após gravar

Primeiro abra a porta sem pedido e confirme o aviso. Feche-a e verifique
a liberação automática. Depois teste uma unidade de cada compartimento,
conferindo o motor correspondente e a confirmação pelo infravermelho.
Por fim, acione a chave durante um pedido e verifique parada, bloqueio da
confirmação e aviso na tela. A validação automatizada usa respostas simuladas;
não comprova a ligação elétrica nem aciona motores reais.
