# Etapa 3

A etapa 3 do projeto apresenta a construção da estrutura física e o desenvolvimento do firmware responsável pela lógica de venda. Também contempla a integração do sistema de pagamento escolhido, a instalação dos motores e sensores e a programação do controle do mecanismo. Além disso, inclui o projeto do painel de interface com o usuário, que permite a seleção de produtos e o acompanhamento do processo de compra.


## 1. Estrutura

Foi desenvolvido um gabinete para acomodar os seis compartimentos de armazenamento e distribuição dos cookies. A estrutura possui divisórias individuais, espirais para movimentação dos produtos e áreas destinadas à interface e aos componentes eletrônicos.


<!-- Tabela com as três etapas de construção da máquina -->
<table>
  <!-- Linha das imagens -->
  <tr>
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/d2465485-4246-44cf-99db-96fa1ec1eee2"
           width="350" alt="Estrutura em MDF cru">
    </td>
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/3205a4b2-eb67-4070-9166-fadc959a2a99"
           width="350" alt="Estrutura pintada de branco">
    </td>
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/9852b0da-3e48-4849-b797-1bd041af14e2"
           width="350" alt="Estrutura final da máquina de cookies">
    </td>
  </tr>
  <!-- Linha das legendas -->
  <tr>
    <td align="center">
      <em>Figura 1 - Estrutura em MDF cru após montagem.</em>
    </td>
    <td align="center">
      <em>Figura 2 - Estrutura após primeira pintura.</em>
    </td>
    <td align="center">
      <em>Figura 3 - Estrutura com pintura final, seis compartimentos e espirais instalados.</em>
    </td>
  </tr>
</table>

    

**Testes realizados:** estabilidade da estrutura, alinhamento das espirais, encaixe dos produtos e funcionamento mecânico da distribuição.



## 2. Firmware da lógica de venda funcional

O firmware foi desenvolvido em C, utilizando ESP-IDF e FreeRTOS. O sistema funciona por meio de uma máquina de estados, responsável por organizar as etapas de venda e entrega dos cookies. Os pedidos são recebidos por uma API HTTP em formato JSON, com autenticação por token Bearer, validado pelo servidor Python.

Cada pedido pode conter de um a três cookies, distribuídos entre os seis compartimentos disponíveis. Antes de iniciar a venda, o sistema verifica o estoque, as condições de funcionamento da máquina e a existência de vendas anteriores que ainda não foram sincronizadas com o servidor.

O controle dos motores e a comunicação com o servidor são realizados em tarefas separadas, utilizando mutex para evitar conflitos no acesso às informações compartilhadas. O estoque é atualizado individualmente, conforme a passagem de cada cookie é confirmada pelo sensor infravermelho.

Após a entrega, os dados da venda são armazenados na memória flash do ESP32, utilizando NVS, e enviados ao backend para registro no banco de dados SQLite. Caso ocorra alguma falha de comunicação, o registro permanece armazenado para uma nova tentativa de envio. Enquanto essa sincronização não for concluída, novos pedidos ficam bloqueados.

## 3. Integrar o sistema de pagamento escolhido

O sistema conta com uma interface acessada pelo celular utilizado como IHM da máquina de cookies. Nela, o usuário podee realizar seu cadastro, entrar em uma conta existente e acessar as funcionalidades de compra.

O cadastro é feito por meio de um formulário com nome de usuário e senha. Essas informações serão enviadas ao servidor Python, responsável por armazenar os dados no banco SQLite e realizar a autenticação. 

Após realizar o login, o usuário terá acesso à tela de seleção dos seis sabores de cookies disponíveis. Será possível adicionar até três unidades ao carrinho, independentemente da combinação de sabores escolhida. Antes de confirmar o pedido, a interface consultará o estoque disponível no ESP32 para verificar se os produtos selecionados podem ser entregues.

A comunicação entre a IHM e o ESP32 será realizada por requisições HTTP, utilizando tokens de autenticação para identificar e validar as solicitações. Dessa forma, cada compra poderá ser associada ao usuário responsável pelo pedido.

Além da interface do cliente, o sistema contará com uma página de administração (backoffice), com acesso separado por login. Nela, será possível acompanhar o estoque dos seis compartimentos, consultar o histórico de vendas e realizar atualizações de estoque após a reposição dos produtos.

### Funções lógica de venda 

| Função | Descrição |
|---|---|
| `preparar_proximo_cookie()` | Identifica o próximo compartimento com unidades pendentes e prepara a sequência de entrega. |
| `aguardando_confirmacao()` | Verifica se existe uma unidade aguardando confirmação, com motor desligado e máquina fora de manutenção. |
| `confirmar_entrega()` | Confirma a entrega do cookie, atualiza o estoque e prepara a próxima unidade ou finaliza o pedido. |
| `resetar_pedido()` | Cancela o pedido em andamento, desliga os motores e preserva as baixas de estoque dos cookies já entregues. |


### Fluxo da rede implementada 

<p align="center">
  <img
    src="https://github.com/user-attachments/assets/4055f01c-def4-4ad1-9195-b1eb9e94ea70"
    alt="fluxo-rede"
    width="400"
    height="600"
  />
</p>


## 4. Instalar motores e sensores

Foi realizada a montagem física de seis conjuntos de motores responsáveis pelo acionamento individual das espirais de distribuição dos cookies. Além disso, foram instalados sensores para garantir a segurança da operação e monitorar a entrega dos produtos.

Na **porta de manutenção**, foi instalada uma **chave de fim de curso**, responsável por identificar sua abertura. Quando a porta está aberta, o sistema deve bloquear o acionamento dos motores e impedir que o usuário solicite novos cookies, garantindo maior segurança durante as atividades de manutenção.

Também foram instalados um **emissor e um receptor infravermelho**, posicionados na saída de distribuição dos cookies. Esses sensores formam uma barreira óptica que permite detectar a passagem do produto durante a queda, possibilitando ao sistema identificar a entrega do cookie.

<p align="center">
  <img src="https://github.com/user-attachments/assets/9159dc15-d98a-4ba7-9ad3-1e0cdb0594c7"
       width="700" alt="Figura 4">
  <br>
  <em>Figura 4 - Motores instalados.</em>
</p>


<!-- Tabela com as três imagens lado a lado -->
<table>
  <!-- Linha das imagens -->
  <tr>
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/ce77f973-a7b2-4e10-9d93-659b24ec106d"
           width="350" alt="Imagem 1">
    </td>
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/98102b71-5bbc-4ad0-bb49-7e5db96fa568"
           width="350" alt="Imagem 2">
    </td>
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/70f1376d-9366-442d-8c47-9f810b666a84"
           width="350" alt="Imagem 3">
    </td>
  </tr>

  <!-- Linha das legendas -->
  <tr>
    <td align="center">
      <em>Figura 5 - Chave de fim de curso.</em>
    </td>
    <td align="center">
      <em>Figura 6 - Emissor.</em>
    </td>
    <td align="center">
      <em>Figura 7 - Receptor.</em>
    </td>
  </tr>
</table>

## 5. Firmware do controle do mecanismo
O controle dos seis motores é realizado pelo ESP32, utilizando os GPIOs 18, 19, 21, 22, 23 e 25, conectados ao circuito de acionamento. Para evitar que dois motores sejam acionados ao mesmo tempo, o firmware desativa todas as saídas antes de ligar o motor correspondente ao compartimento escolhido.

Cada acionamento tem duração de três segundos, controlada pela função esp_timer_get_time(). Essa abordagem permite acompanhar o tempo de funcionamento do motor sem precisar interromper a execução da tarefa durante todo o período.

Para identificar a entrega dos cookies, é utilizado um sistema de detecção infravermelha, composto por transmissores e um receptor. O sinal dos transmissores é gerado pelo GPIO26, utilizando o periférico LEDC com frequência de 38 kHz. Já o receptor, conectado ao GPIO27, é monitorado por interrupções e pela tarefa responsável pelo controle do mecanismo.
Quando a passagem de um cookie é identificada, essa informação fica registrada até o desligamento do motor, momento em que a entrega é confirmada. Caso o sensor não identifique a passagem, o sistema pode realizar até três tentativas de acionamento para a mesma unidade. Se nenhuma tentativa for confirmada, o pedido permanece aguardando confirmação, evitando registrar uma entrega que não foi identificada.

O mecanismo também possui uma chave de fim de curso conectada ao GPIO32, utilizada para identificar a abertura da máquina durante procedimentos de manutenção ou reposição. Quando acionada, o ESP32 desliga os motores, interrompe o pedido em andamento e coloca o sistema em modo de manutenção, impedindo novas vendas até que a condição seja normalizada.

### Funções de Controle e acionamento dos motores

| Função | Descrição |
|---|---|
| `acionamento_init()` | Configura os GPIOs dos seis motores como saídas, desliga todos os motores e configura a chave de fim de curso. |
| `acionamento_ligar(slot)` | Verifica o compartimento selecionado e o estado da porta, garantindo que apenas o motor escolhido seja acionado. |
| `acionamento_parar()` | Desliga todos os motores, colocando seus GPIOs em nível baixo. |
| `fim_curso_acionado()` | Verifica o estado da chave de fim de curso para identificar a abertura da porta. |
| `verificar_manutencao()` | Desliga os motores e coloca a máquina em manutenção quando a porta é aberta. |
| `iniciar_tentativa_cookie()` | Prepara o sensor infravermelho e inicia a espera de um segundo antes do acionamento do motor. |
| `ligar_motor_tentativa()` | Liga o motor selecionado, inicia o monitoramento do sensor e registra o instante e o número da tentativa. |
| `controle_passo()` | Executa o ciclo de controle, acompanhando o sensor, o tempo de acionamento dos motores e as condições de segurança. |
| `tarefa_controle()` | Executa continuamente os ciclos de controle, utilizando mutex para proteger os dados compartilhados. |


### 5.1 Desenvolvimento e montagem da placa eletrônica

Foi desenvolvida uma placa de circuito impresso (PCB) para integrar os componentes eletrônicos responsáveis pelo controle dos mecanismos de distribuição da máquina de cookies.

O projeto foi realizado no **Altium Designer**, incluindo o posicionamento dos componentes, o roteamento das trilhas e a definição das conexões elétricas necessárias para o funcionamento do sistema.

Após a etapa de desenvolvimento do layout, a placa foi fabricada e os componentes eletrônicos foram soldados, permitindo avançar para a integração com os motores, sensores e demais dispositivos da máquina.


<!-- Tabela com as três imagens da placa eletrônica -->
<table>
  <!-- Linha das imagens -->
  <tr>
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/b1df0e02-4995-4d15-be0a-7b2cddd3a8dc"
           width="350" alt="Imagem 1 da placa eletrônica">
    </td>
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/b00feebc-5ee0-4bf5-8a90-9f8e58ff877b"
           width="350" alt="Imagem 2 da placa eletrônica">
    </td>
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/c8339860-ae59-4b57-97a1-9c2be5efcc6d"
           width="350" alt="Imagem 3 da placa eletrônica">
    </td>
  </tr>

  <!-- Linha das legendas -->
  <tr>
    <td align="center">
      <em>Figura 8 - Esquemático da placa.</em>
    </td>
    <td align="center">
      <em>Figura 9 - Layout da placa.</em>
    </td>
    <td align="center">
      <em>Figura 10 - Placa não montada.</em>
    </td>
  </tr>
</table>


## 6. Projetar painel/interface com o usuário

Desenvolver a interface responsável pela interação entre o consumidor e a máquina, apresentando produtos disponíveis, preços, informações de pagamento e andamento da distribuição.

**Testes previstos:** navegação, seleção dos produtos, exibição dos valores, atualização do estado do pagamento e mensagens de confirmação ou erro.

**Situação:** espaço físico para interface identificado na estrutura; implementação e testes da interface pendentes de documentação.



<!-- GIF centralizado com reprodução automática -->
<p align="center">
  <img
    src="https://github.com/user-attachments/assets/30db38d0-c775-4de2-8177-c9ee528174d4"
    width="320"
    alt="Demonstração do sistema de pedidos"
  />
  <br>
  <em>Figura 11 - Demonstração do funcionamento do sistema de pedidos.</em>
</p>



## Conclusão

O projeto já apresenta elementos físicos de desenvolvimento, incluindo gabinete, mecanismos de espiral, montagem dos motores e fabricação da placa eletrônica.

As próximas etapas de documentação envolvem registrar a implementação dos sistemas de controle e pagamento, executar os ensaios funcionais e anexar evidências que comprovem o atendimento aos requisitos de cada subentrega.







## Testes

Descrição dos testes/validações realizadas. Use fotos, diagramas, tabelas, etc.

## Referências (links/datasheets/livros)


- [nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.2/nrf/getting_started/modifying.html#configure-application>)


