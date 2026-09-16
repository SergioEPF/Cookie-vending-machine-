# Etapa 2

A etapa 2 tem como objetivo detalhar a construção da máquina de venda de cookies, considerando a estrutura, o armazenamento, a liberação dos produtos e os componentes eletrônicos. As atividades incluem verificar o acesso para manutenção e reposição, dimensionar a caixa, desenvolver os desenhos em CAD, projetar os mecanismos de armazenamento e dispensação, selecionar motores e sensores, definir o controlador e planejar o sistema de pagamento. Nesta etapa, as escolhas iniciais serão ajustadas conforme as dimensões dos cookies embalados e os resultados dos testes com o mecanismo.

## Desenvolvimento

### Estrutura, dimensionamento e manutenção

A estrutura proposta utiliza MDF na base, nas laterais e na parte traseira, com um visor de acrílico na frente para permitir a visualização dos produtos. A reposição será feita por uma tampa superior, que dará acesso aos seis canais de armazenamento. Cada canal terá uma espiral e um motor independente.

O dimensionamento deverá considerar a largura e a altura da embalagem, o comprimento das espirais, a quantidade de cookies por canal e o espaço ocupado pelos motores. Também será reservado espaço para a passagem do produto até a bandeja de retirada, evitando pontos onde a embalagem possa ficar presa.

As superfícies de apoio e a bandeja de retirada serão lisas e removíveis para facilitar a limpeza. O compartimento eletrônico ficará separado da área dos produtos e terá acesso para manutenção. Os suportes dos motores e das espirais deverão permitir a desmontagem individual de cada conjunto.

O desenho em CAD deverá mostrar a posição dos seis canais, a fixação dos motores, a tampa de reposição, o visor e o caminho de queda. As medidas finais e as espessuras dos materiais ainda serão definidas. Após a atualização do modelo, serão adicionadas vistas gerais e detalhes de montagem para acompanhar esta descrição.

### Armazenamento e dispensação

Os cookies serão armazenados em embalagens individuais fechadas, posicionados entre as espiras das molas de aço mola. Divisórias separarão os canais e ajudarão a manter os produtos alinhados durante o avanço.

A liberação ocorrerá pela rotação da espiral correspondente ao produto selecionado. O movimento deslocará as embalagens em direção à saída, permitindo que o primeiro cookie caia na bandeja de retirada. O espaçamento das espiras e o avanço por venda deverão ser ajustados para liberar apenas uma unidade, sem comprimir os produtos.

Serão utilizados seis motores de passo, um por espiral. A escolha permite comandar o avanço por uma quantidade definida de passos. Como referência inicial, foi pesquisado o motor bipolar NEMA 17 Pololu 2267, com passo de 1,8° e corrente nominal de 1,7 A por fase [1]. O modelo definitivo dependerá do esforço necessário para movimentar o canal carregado.

Cada motor terá um driver próprio. O DRV8825 está em avaliação por oferecer controle por sinais STEP/DIR e limitação ajustável de corrente [2]. A seleção deverá considerar a corrente do motor e a dissipação de calor. Os seis drivers compartilharão uma fonte, com distribuição em paralelo. A potência da fonte será definida considerando o acionamento e a eventual manutenção dos motores energizados em repouso.

### Controlador, interface e sensores

O controlador escolhido é o ESP32, responsável por receber os comandos da interface, ler os sensores e controlar os drivers. Sua comunicação Wi-Fi permite integrar uma interface acessada pelo celular [3]. O Redmi M2006C3LG será utilizado como IHM, exibindo os produtos disponíveis, as instruções de pagamento e o resultado da liberação.

Um sensor de fim de curso será instalado na tampa superior. Sua função será identificar a abertura para reposição e impedir novos acionamentos enquanto a tampa estiver aberta. A abertura durante um ciclo deverá interromper o movimento e sinalizar a ocorrência.

Para detectar a queda, está prevista uma barreira fotoelétrica infravermelha com emissor e receptor em lados opostos da passagem. O Adafruit 2168 foi pesquisado como referência por apresentar alcance nominal aproximado de 50 cm e resposta inferior a 2 ms [4].

A instalação deverá garantir que o cookie atravesse o feixe. A necessidade de mais de uma barreira dependerá da geometria da saída. A contagem de passos do motor não será considerada confirmação de entrega: essa confirmação dependerá da detecção da passagem do produto.

### Sistema de pagamento

A proposta é utilizar pagamento por Pix. A interface apresentará a cobrança, e um servidor verificará sua aprovação antes de autorizar a liberação. O serviço de pagamento e sua integração ainda serão definidos.

Cada autorização deverá estar associada a um pedido específico, evitando liberações repetidas por atualização da página ou reenvio de comandos. Caso o pagamento seja aprovado e a queda não seja detectada, o sistema deverá registrar a falha e informar o usuário, sem executar outra liberação automaticamente.

## Testes

Até o momento, este registro contempla o planejamento dos testes, sem resultados experimentais documentados. A primeira montagem de validação deverá utilizar um canal, uma espiral, um motor e o sensor de queda.

Serão verificadas a movimentação com diferentes quantidades de cookies, a liberação de uma unidade por comando e a ocorrência de travamentos. O sensor será testado com a embalagem definitiva e diferentes trajetórias de queda.

Também serão verificados o bloqueio com a tampa aberta, a interrupção do movimento, a comunicação entre celular e ESP32 e o tratamento de comandos duplicados. Fotografias da montagem e uma tabela com condições, resultados e ajustes serão acrescentadas após os ensaios.

## Referências

* [1] [Pololu — Motor de passo bipolar NEMA 17, modelo 2267](https://www.pololu.com/product/2267).
* [2] [Pololu — Driver de motor de passo DRV8825](https://www.pololu.com/product/2133).
* [3] [Espressif — Datasheet da série ESP32](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf).
* [4] [Adafruit — Sensor infravermelho de barreira 2168](https://www.adafruit.com/product/2168).



O QUE PRECISA NA ETAPA:
# Etapa 1

**(MÍNIMO DE 600 E MÁXIMO DE 1000 PALAVRAS no total do arquivo md.)**

A etapa 1 ...

**(Adicionar aqui UM parágrafo com visão geral da etapa. Resumo dos itens da planilha.)**

**(Não adicione código em nenhum arquivo md. )**

## Desenvolvimento

Apresentar o desenvolvimento da etapa contendo detalhes de implementação (se houver) de hardware e software. Use fotos, diagramas, tabelas etc. Adicionar pesqusisas realizadas. Relacionar as fotos, diagramas, etc no texto. Todas as referências devem citadas no texto. 

## Testes

Descrição dos testes/validações realizadas. Use fotos, diagramas, tabelas, etc.

## Referências (links/datasheets/livros)


- [nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.2/nrf/getting_started/modifying.html#configure-application>)


