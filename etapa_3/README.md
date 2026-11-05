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
      <em>Figura 2 - Estrutura após pintura e acabamento.</em>
    </td>
    <td align="center">
      <em>Figura 3 - Estrutura com os seis compartimentos e espirais instalados.</em>
    </td>
  </tr>
</table>

    

**Testes realizados:** estabilidade da estrutura, alinhamento das espirais, encaixe dos produtos e funcionamento mecânico da distribuição.



## 2. Firmware da lógica de venda funcional

Desenvolver a lógica responsável por gerenciar a seleção dos produtos, verificar sua disponibilidade, solicitar o pagamento e autorizar a distribuição após a confirmação da transação.

**Testes previstos:** seleção correta, identificação do canal, bloqueio de produtos indisponíveis, recusa de pagamentos e prevenção de entregas duplicadas.

**Situação:** requisitos e testes definidos; implementação funcional ainda precisa ser documentada.

## 3. Integrar o sistema de pagamento escolhido

Integrar o método de pagamento à lógica de venda, permitindo que a distribuição seja autorizada somente após a confirmação de uma transação válida.

**Testes previstos:** confirmação de pagamento, pagamentos recusados, cancelamentos, falhas de comunicação e prevenção de acionamentos indevidos.

**Situação:** solução de pagamento e integração ainda precisam ser registradas.

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

Desenvolver o firmware responsável por comandar os motores, interpretar os sinais dos sensores e finalizar cada ciclo de distribuição.

O projeto também contempla o desenvolvimento de uma placa eletrônica de controle, com layout e face de cobre registrados fotograficamente.

**Testes previstos:** acionamento correto de cada motor, funcionamento dos sensores, tempo máximo de acionamento, tratamento de falhas e verificação elétrica da placa.

**Situação:** desenvolvimento da placa documentado por imagens; firmware e validações elétricas ainda precisam de evidências.

### 5.1 Desenvolvimento e montagem da placa eletrônica

Foi desenvolvida uma placa de circuito impresso (PCB) para integrar os componentes eletrônicos responsáveis pelo controle dos mecanismos de distribuição da máquina de cookies.

O projeto foi realizado no **Altium Designer**, incluindo o posicionamento dos componentes, o roteamento das trilhas e a definição das conexões elétricas necessárias para o funcionamento do sistema.

Após a etapa de desenvolvimento do layout, a placa foi fabricada e os componentes eletrônicos foram soldados, permitindo avançar para a integração com os motores, sensores e demais dispositivos da máquina.

**Testes previstos:** verificação de continuidade elétrica, identificação de possíveis curtos-circuitos, medição das tensões de alimentação, testes dos acionamentos e validação das conexões com os sensores.

**Situação:** layout desenvolvido e placa montada com os componentes soldados. Resultados dos testes elétricos e funcionais a serem documentados.


## 6. Projetar painel/interface com o usuário

Desenvolver a interface responsável pela interação entre o consumidor e a máquina, apresentando produtos disponíveis, preços, informações de pagamento e andamento da distribuição.

**Testes previstos:** navegação, seleção dos produtos, exibição dos valores, atualização do estado do pagamento e mensagens de confirmação ou erro.

**Situação:** espaço físico para interface identificado na estrutura; implementação e testes da interface pendentes de documentação.

## Conclusão

O projeto já apresenta elementos físicos de desenvolvimento, incluindo gabinete, mecanismos de espiral, montagem dos motores e fabricação da placa eletrônica.

As próximas etapas de documentação envolvem registrar a implementação dos sistemas de controle e pagamento, executar os ensaios funcionais e anexar evidências que comprovem o atendimento aos requisitos de cada subentrega.


<img width="400" height="400" alt="image" src="https://github.com/user-attachments/assets/b00feebc-5ee0-4bf5-8a90-9f8e58ff877b" />
<img width="700" height="400" alt="image" src="https://github.com/user-attachments/assets/b1df0e02-4995-4d15-be0a-7b2cddd3a8dc" />

<img width="400" height="400" alt="image" src="https://github.com/user-attachments/assets/c8339860-ae59-4b57-97a1-9c2be5efcc6d" />







## Testes

Descrição dos testes/validações realizadas. Use fotos, diagramas, tabelas, etc.

## Referências (links/datasheets/livros)


- [nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.2/nrf/getting_started/modifying.html#configure-application>)


