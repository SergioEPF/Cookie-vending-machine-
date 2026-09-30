# Etapa 2

A etapa 2 tem como objetivo detalhar a construção da máquina de venda de cookies, considerando a estrutura, o armazenamento, a liberação dos produtos e os componentes eletrônicos. As atividades incluem verificar o acesso para manutenção e reposição, dimensionar a caixa, desenvolver os desenhos em CAD, projetar os mecanismos de armazenamento e dispensação, selecionar motores e sensores, definir o controlador e planejar o sistema de pagamento. Nesta etapa, as escolhas iniciais serão ajustadas conforme as dimensões dos cookies embalados e os resultados dos testes com o mecanismo.

## Desenvolvimento

### Estrutura, dimensionamento e manutenção

A estrutura proposta utiliza MDF na base, nas laterais e na parte traseira, com um visor de acrílico na frente para permitir a visualização dos produtos. A reposição será feita por uma tampa superior, que dará acesso aos seis canais de armazenamento. Cada canal terá uma espiral e um motor independente.

<p align="center">
  <img width="500" height="400" alt="CAD completo da máquina" src="https://github.com/user-attachments/assets/c5c8cf37-9741-46da-9d0a-7d2933c9acdf" />
  <br>
  <em>Figura 2 — Estrutura completa.</em>
</p>

O dimensionamento deverá considerar a largura e a altura da embalagem, o comprimento das espirais, a quantidade de cookies por canal e o espaço ocupado pelos motores. Também será reservado espaço para a passagem do produto até a bandeja de retirada, evitando pontos onde a embalagem possa ficar presa.

<p align="center">
  <br>
  <em>Figura 2 — Colocar uma imagem contendo todas as dimensoes.</em>
</p>

As superfícies de apoio e a bandeja de retirada serão lisas e removíveis para facilitar a limpeza. O compartimento eletrônico ficará separado da área dos produtos e terá acesso para manutenção. Os suportes dos motores e das espirais deverão permitir a desmontagem individual de cada conjunto.

Para iluminação será utilizado uma fita de led controlada pelo microcontrolador. 


## Armazenamento e dispensação

Os cookies serão armazenados em embalagens individuais fechadas, posicionados entre as espiras das molas de aço mola. Divisórias separarão os canais e ajudarão a manter os produtos alinhados durante o avanço.

A liberação ocorrerá pela rotação da espiral correspondente ao produto selecionado. O movimento deslocará as embalagens em direção à saída, permitindo que o primeiro cookie caia na bandeja de retirada. O espaçamento das espiras e o avanço por venda deverão ser ajustados para liberar apenas uma unidade, sem comprimir os produtos.

Serão utilizados seis motores de passo, um por espiral. A escolha permite comandar o avanço por uma quantidade definida de passos. Como referência inicial, foi pesquisado o motor bipolar NEMA 17 Pololu 2267, com passo de 1,8° e corrente nominal de 1,7 A por fase [1]. O modelo definitivo dependerá do esforço necessário para movimentar o canal carregado.

Cada motor terá um driver próprio. O DRV8825 está em avaliação por oferecer controle por sinais STEP/DIR e limitação ajustável de corrente [2]. A seleção deverá considerar a corrente do motor e a dissipação de calor. Os seis drivers compartilharão uma fonte, com distribuição em paralelo. A potência da fonte será definida considerando o acionamento e a eventual manutenção dos motores energizados em repouso.

## Motores, Atuadores e Sensores 

Para o sistema de entrega da máquina, foram utilizados motores de passo acoplados às molas responsáveis pelo armazenamento e liberação dos cookies. A escolha desse tipo de motor foi feita principalmente pela possibilidade de controlar de forma precisa o deslocamento angular do eixo, permitindo realizar uma volta completa da mola sempre que uma venda for efetuada.

Diferentemente de um motor DC convencional, o motor de passo permite controlar diretamente a quantidade de passos realizados, dispensando a necessidade de um sensor específico para verificar a posição final da mola. Dessa forma, o sistema pode comandar uma rotação de aproximadamente 360° a cada acionamento e utilizar os sensores apenas para validar se o produto foi efetivamente entregue.

### Drivers dos motores

Durante os testes iniciais foi utilizado um shield para Arduino para realizar o acionamento dos motores de passo. Para a versão final do projeto, entretanto, optou-se pela utilização de drivers individuais, devido a escolha do microtrolador utilizado na máquina.

O driver selecionado para o projeto foi o DRV8825. Esse componente utiliza sinais do tipo STEP e DIR, o que torna seu controle simples.

Como a máquina possui seis mecanismos de entrega, o sistema utilizará um driver para cada motor, totalizando seis drivers. Essa configuração também permite que cada conjunto motor/mola seja controlado individualmente.

### Sensores de detecção de produto

Para confirmar que o cookie foi realmente liberado após o acionamento do motor, será utilizado um sistema de barreira infravermelha.

O conjunto selecionado é formado pelo emissor infravermelho TSAL6200 e pelo receptor TSSP58038.

O TSAL6200 é responsável por emitir luz infravermelha, enquanto o TSSP58038 detecta esse sinal. Os dois componentes são posicionados em lados opostos do caminho de queda do produto, formando uma barreira óptica.

Quando não existe nenhum objeto entre os componentes, o receptor detecta normalmente o sinal infravermelho. Durante a queda de um cookie, essa comunicação é momentaneamente interrompida, permitindo que o microcontrolador identifique que houve passagem de um produto.

No projeto serão utilizados três conjuntos de sensores, compostos por:

3 emissores TSAL6200;

2 receptores TSSP58038.

Os sensores não serão responsáveis por controlar a posição dos motores. O movimento continuará sendo definido pela quantidade de passos enviada ao driver, enquanto os sensores funcionarão como uma confirmação independente de que a entrega ocorreu corretamente.

## Sistema de pagamento

Para o funcionamento da vending machine, optou-se por não utilizar um sistema de pagamento integrado diretamente à máquina. Em vez disso, será utilizado um cadastro de usuários, permitindo identificar quem realizou cada compra e registrar os produtos retirados.

Essa abordagem foi escolhida com o objetivo de simplificar o desenvolvimento do protótipo, evitando a necessidade de integração com meios de pagamento, instituições financeiras ou leitores específicos. Ao mesmo tempo, o sistema continua permitindo o controle das vendas e a identificação do responsável por cada retirada.

### Cadastro do usuário

Antes de realizar uma compra, o usuário deverá se identificar na interface da máquina. Caso ainda não possua cadastro, poderá realizar um cadastro simples diretamente pela IHM.

O cadastro deverá conter apenas as informações necessárias para identificar posteriormente o usuário e associar as compras realizadas a ele.

Após o cadastro, o usuário poderá acessar a máquina utilizando suas credenciais e selecionar normalmente o produto desejado.

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


