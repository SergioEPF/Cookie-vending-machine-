# Etapa 3

A etapa 3 do projeto apresenta a construção da estrutura física e o desenvolvimento do firmware responsável pela lógica de venda. Também contempla a integração do sistema de pagamento escolhido, a instalação dos motores e sensores e a programação do controle do mecanismo. Além disso, inclui o projeto do painel de interface com o usuário, que permite a seleção de produtos e o acompanhamento do processo de compra.


## 1. Estrutura

Foi desenvolvido um gabinete para acomodar os seis compartimentos de armazenamento e distribuição dos cookies. A estrutura possui divisórias individuais, espirais para movimentação dos produtos e áreas destinadas à interface e aos componentes eletrônicos. 

A estrutura foi projetada no Onshape, uma plataforma de modelagem 3D. A **Figura 1** e a **Figura 2** mostram o modelo com os formatos e os encaixes das peças de MDF. As peças foram cortadas em uma máquina automatizada de corte a laser, conforme mostra a **Figura 3**.

<!-- Tabela com as três etapas de construção da máquina -->
<table>
  <!-- Linha das imagens -->
  <tr>
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/23c1094d-9c4d-4278-a813-2aa9df7ad096"
           width="350" alt="Vista frontal da estrutura">
    </td>
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/4abdc7e9-effe-4eaf-a132-79334c30bd56"
           width="350" alt="Vista posterior da estrutura">
    </td>
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/77d941f4-d249-4eea-b919-4d8a56a55b9d" 
           width="350" alt="Máquina de corte">
  </tr>
  <!-- Linha das legendas -->
  <tr>
    <td align="center">
      <em>Figura 1 - Vista frontal da estrutura.</em>
    </td>
    <td align="center">
      <em>Figura 2 - Vista posterior da estrutura.</em>
    </td>
    <td align="center">
      <em>Figura 3 - Máquina de corte a laser.</em>
    </td>
  </tr>
</table>


A **Figura 4** apresenta as etapas de corte das peças em MDF e a montagem inicial da estrutura. Na **Figura 5**, observa-se a aplicação da primeira demão de tinta branca, utilizada como preparação para o acabamento. Por fim, a **Figura 6** apresenta o gabinete montado e com a pintura finalizada, incluindo a instalação das espirais responsáveis pela movimentação e distribuição dos cookies.

**Testes realizados:** Foram realizados testes para verificar a estabilidade da estrutura, o alinhamento das espirais, o encaixe adequado dos produtos nos compartimentos e o funcionamento mecânico do sistema de distribuição, visando garantir a correta movimentação e liberação dos cookies.

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
      <em>Figura 4 - Estrutura em MDF cru após montagem.</em>
    </td>
    <td align="center">
      <em>Figura 5 - Estrutura após primeira pintura.</em>
    </td>
    <td align="center">
      <em>Figura 6 - Estrutura com pintura final, seis compartimentos e espirais instalados.</em>
    </td>
  </tr>
</table>



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

A Figura 7 reúne em um diagrama as conexões entre o navegador, o ESP32, o servidor Python e o banco de dados SQLite.

<p align="center">
  <img
    src="https://github.com/user-attachments/assets/4055f01c-def4-4ad1-9195-b1eb9e94ea70"
    alt="fluxo-rede"
    width="400"
    height="600"
  />
  <br>
  <em>Figura 7 - Fluxograma da rede .</em>
</p>


## 4. Instalar motores e sensores

Foi realizada a montagem física de seis conjuntos de motores responsáveis pelo acionamento individual das espirais de distribuição dos cookies, conforme apresentado na **Figura 8**. Além disso, foram instalados sensores para garantir a segurança da operação e monitorar a entrega dos produtos.

Na porta de reposição, foi instalada uma chave de fim de curso, apresentada na **Figura 9**, responsável por identificar sua abertura. Quando a porta está aberta, o sistema deve bloquear o acionamento dos motores e impedir que o usuário solicite novos cookies, garantindo maior segurança durante as atividades de reposição dos produtos.

Também foram instalados um emissor e um receptor infravermelho, posicionados na saída de distribuição dos cookies. A **Figura 10** apresenta o emissor, enquanto a **Figura 11** mostra o receptor. Esses sensores formam uma barreira óptica que permite detectar a passagem do produto durante a queda, possibilitando ao sistema identificar a entrega do cookie.


<!-- Figura 4 - Motores instalados -->
<p align="center">
  <img src="https://github.com/user-attachments/assets/9159dc15-d98a-4ba7-9ad3-1e0cdb0594c7"
       width="700" alt="Figura 4">
  <br>
  <em>Figura 8 - Motores instalados.</em>
</p>

<!-- Tabela com as Figuras 5, 6 e 7 lado a lado -->
<table>
  <!-- Linha das imagens -->
  <tr>
    <!-- Figura 5 - Chave de fim de curso -->
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/ce77f973-a7b2-4e10-9d93-659b24ec106d"
           width="350" alt="Chave de fim de curso">
    </td>
    <!-- Figura 6 - Emissor infravermelho (nova imagem) -->
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/4aa7376b-5070-4a1c-a0ba-90880c89cf18"
           width="350" alt="Emissor infravermelho">
    </td>
    <!-- Figura 7 - Receptor infravermelho (nova imagem) -->
    <td align="center" width="33%">
      <img src="https://github.com/user-attachments/assets/4c751f13-4afc-46b8-9e10-e3681442457b"
           width="350" alt="Receptor infravermelho">
    </td>
  </tr>
  
  <!-- Linha das legendas -->
  <tr>
    <td align="center">
      <em>Figura 9 - Chave de fim de curso.</em>
    </td>
    <td align="center">
      <em>Figura 10 - Emissor infravermelho.</em>
    </td>
    <td align="center">
      <em>Figura 11 - Receptor infravermelho.</em>
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

O projeto foi realizado no Altium Designer, iniciando pela elaboração do esquemático elétrico, apresentado na **Figura 12**, no qual foram definidas as conexões entre os componentes eletrônicos. Em seguida, foi desenvolvido o layout da PCB, apresentado na **Figura 13**, incluindo o posicionamento dos componentes, o roteamento das trilhas e a organização das conexões elétricas necessárias para o funcionamento do sistema.

Após a finalização do projeto, foi realizada a fabricação da placa de circuito impresso. A **Figura 14** apresenta a placa fabricada, ainda sem a montagem dos componentes eletrônicos. Posteriormente, foi realizada a soldagem dos componentes, resultando na placa montada, conforme apresentado na **Figura 15** e na **Figura 16**.

Com a conclusão dessa etapa, a placa ficou preparada para a integração com os motores, sensores e demais dispositivos eletrônicos da máquina de cookies.

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
      <em>Figura 12 - Esquemático da placa.</em>
    </td>
    <td align="center">
      <em>Figura 13 - Layout da placa.</em>
    </td>
    <td align="center">
      <em>Figura 14 - Placa não montada.</em>
    </td>
  </tr>
</table>


<!-- Figuras 8 e 9 lado a lado -->
<table align="center">
  <tr>
    <td align="center" width="50%">
      <img
        src="https://github.com/user-attachments/assets/d3dfc36b-71f0-43ce-8ec3-400846454987"
        width="350"
        alt="Esquemático elétrico da PCB">
      <br>
      <em>Figura 15 - Esquemático elétrico da PCB.</em>
    </td>
    <td align="center" width="50%">
      <img
        src="https://github.com/user-attachments/assets/84ff2ed7-ee79-4e1e-8132-a9e9f5463654"
        width="350"
        alt="Layout da placa de circuito impresso">
      <br>
      <em>Figura 16 - Layout da PCB no Altium Designer.</em>
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
  <em>Figura 17 - Demonstração do funcionamento do sistema de pedidos.</em>
</p>


## Conclusão

O projeto já apresenta elementos físicos de desenvolvimento, incluindo gabinete, mecanismos de espiral, montagem dos motores e fabricação da placa eletrônica.

As próximas etapas de documentação envolvem registrar a implementação dos sistemas de controle e pagamento, executar os ensaios funcionais e anexar evidências que comprovem o atendimento aos requisitos de cada subentrega.


## Testes

Descrição dos testes/validações realizadas. Use fotos, diagramas, tabelas, etc.

## Referências (links/datasheets/livros)


- [nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.2/nrf/getting_started/modifying.html#configure-application>)


