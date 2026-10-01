# Hardware

## Motores, atuadores e sensores 

Inicialmente, foi testado um motor DC genérico com redução. Durante os testes, o motor não apresentou funcionamento adequado quando havia apenas um cookie no mecanismo. o torque fornecido pelo motor não foi suficiente para realizar o movimento completo da mola, inviabilizando sua utilização no projeto.

Em seguida, foi testado um motor de passo NEMA 17, acionado por meio de um driver conectado a um shield para Arduino. Esse motor apresentou torque suficiente para movimentar a mola mesmo quando havia mais de um cookie armazenado, atendendo aos requisitos mecânicos do sistema. Apesar do bom desempenho, seu custo mais elevado e a necessidade de utilização de um driver específico motivaram a busca por uma alternativa mais simples e econômica.

No terceiro teste, foi utilizado um motor DC com redução desenvolvido para aplicações em máquinas de venda automática. Esse motor apresentou torque suficiente para movimentar o mecanismo mesmo com vários produtos armazenados e demonstrou funcionamento satisfatório durante os testes. Além disso, possui menor custo e um sistema de acionamento mais simples quando comparado ao motor de passo.

Dessa forma, o motor DC com redução para máquinas de venda automática foi escolhido para a versão final do projeto.


### Acionamento dos motores

Como os motores selecionados são motores DC, não será necessária a utilização de drivers específicos para motores de passo. O acionamento será realizado por meio de um circuito de potência simples utilizando um MOSFET e resistores, conforme apresentado na Figura X.

A máquina possui seis mecanismos independentes de entrega, sendo cada mecanismo responsável por um sabor diferente de cookie. Para selecionar qual motor deverá ser acionado, serão utilizados seis relés, permitindo que o microcontrolador controle individualmente cada mecanismo de entrega.

Esse sistema reduz a quantidade de componentes necessários para o acionamento dos motores e simplifica o controle realizado pelo microcontrolador, mantendo cada mecanismo de entrega independente dos demais.

### Sensores de detecção de produto

Para confirmar que o cookie foi realmente liberado após o acionamento do motor, será utilizado um sistema de barreira infravermelha.

O conjunto selecionado é formado pelo emissor infravermelho TSAL6200 e pelo receptor TSSP58038.

O TSAL6200 é responsável por emitir luz infravermelha, enquanto o TSSP58038 detecta esse sinal. Os dois componentes são posicionados em lados opostos do caminho de queda do produto, formando uma barreira óptica.

Quando não existe nenhum objeto entre os componentes, o receptor detecta normalmente o sinal infravermelho. Durante a queda de um cookie, essa comunicação é momentaneamente interrompida, permitindo que o microcontrolador identifique que houve passagem de um produto.

No projeto serão utilizados três 3 emissores e 2 receptores. Cada conjunto será responsável por monitorar a passagem dos produtos e confirmar que a entrega ocorreu corretamente.

Os sensores não serão responsáveis pelo controle direto dos motores. Sua função será atuar como uma confirmação independente da liberação do produto após o acionamento do mecanismo de entrega.

## Microcontrolador 

O ESP32 foi escolhido por atender aos principais requisitos da vending machine, oferecendo Wi-Fi integrado, quantidade suficiente de GPIOs e baixo custo. Em comparação, o NXP RW610 também possui conectividade sem fio e bom desempenho, porém apresenta maior custo e menor disponibilidade no mercado. Já a STM32F411 Black Pill possui baixo consumo e boa quantidade de GPIOs, mas não possui Wi-Fi integrado, exigindo um módulo adicional para a comunicação com a IHM. Como o baixo consumo de energia não é um requisito crítico para o projeto, já que a máquina será alimentada continuamente pela rede elétrica, essa vantagem da Black Pill e de outras soluções de baixo consumo tem menor impacto na escolha. Dessa forma, o ESP32 apresentou a melhor relação entre recursos, simplicidade de implementação e custo para o projeto.

## Testes

Os testes realizados tiveram como objetivo validar o funcionamento dos principais componentes do sistema, especialmente motores, drivers e sensores. Durante essa etapa, foram avaliados o acionamento dos motores, o comportamento das molas e a detecção da passagem dos cookies. Esses testes permitiram identificar ajustes necessários antes da integração definitiva dos componentes na vending machine.

### Teste Motor DC 


### Teste Motor de passo 

### Teste Motor para máquina de vendas

## Referências (links/datasheets/livros)


- [nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.2/nrf/getting_started/modifying.html#configure-application>)


