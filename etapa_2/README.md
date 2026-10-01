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

## Motores, Atuadores e Sensores

Para o sistema de entrega da máquina, foram realizados testes com diferentes tipos de motores, buscando uma solução que apresentasse torque suficiente para movimentar as molas responsáveis pelo armazenamento e liberação dos cookies, além de possuir baixo custo e simplicidade de acionamento.

Mais informações sobre motor, atuadores e sensores podem ser encontradas em [Hardware](./hardware/README.md)


## Escolha do microcontrolador 

O microcontrolador escolhido para o projeto foi o ESP32, principalmente por possuir Wi-Fi integrado, boa quantidade de GPIOs e baixo custo.
Além disso, ele oferece desempenho suficiente para controlar os motores, sensores e a comunicação com a IHM.
Sua ampla documentação e disponibilidade também facilitam o desenvolvimento e a implementação do sistema.

Mais informações sobre a escolha do microcontrolador podem ser encontradas em [Hardware](./hardware/README.md)

## Cadastro do usuário

Antes de realizar uma compra, o usuário deverá se identificar pela IHM da máquina. Caso ainda não possua cadastro, poderá realizar um cadastro simples utilizando a própria interface.

Após a identificação, o usuário poderá visualizar os sabores disponíveis e selecionar o cookie desejado.

### Funcionamento do sistema

Após a seleção do produto, a IHM registrará a solicitação e enviará o comando ao microcontrolador. O microcontrolador será responsável por acionar o motor correspondente ao sabor escolhido.

Depois da liberação do produto, o sensor de barreira verificará a passagem do cookie. Caso a entrega seja confirmada, a compra será registrada no histórico do usuário juntamente com informações como produto selecionado, data e horário.

Caso o sensor não identifique a passagem do produto após o acionamento do motor, a operação poderá ser registrada como uma falha de entrega, evitando que uma compra seja contabilizada sem que o usuário tenha recebido o produto.

### Cobrança e histórico

Como o pagamento não será realizado diretamente na máquina, as compras registradas poderão ser utilizadas posteriormente para realizar a cobrança de cada usuário.

O sistema permitirá consultar o histórico de consumo e o valor acumulado das compras realizadas em uma área administrativa.


Mais informações sobre o aplicativo da IHM e página web do backoffice podem ser encontradas em [Software](./software/README.md)




## Referências

* [1] [Pololu — Motor de passo bipolar NEMA 17, modelo 2267](https://www.pololu.com/product/2267).
* [2] [Pololu — Driver de motor de passo DRV8825](https://www.pololu.com/product/2133).
* [3] [Espressif — Datasheet da série ESP32](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf).
* [4] [Adafruit — Sensor infravermelho de barreira 2168](https://www.adafruit.com/product/2168).



