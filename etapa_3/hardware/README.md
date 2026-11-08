
# 5.1 Desenvolvimento e montagem da placa eletrônica

Inicialmente, foi realizada a seleção dos componentes eletrônicos necessários para o funcionamento da máquina de cookies, considerando os requisitos de alimentação, acionamento dos motores e monitoramento dos sensores. Entre os componentes selecionados estão o microcontrolador ESP32, transistores, resistores, sensores infravermelhos, conversores de tensão e conectores. Os principais componentes utilizados, juntamente com suas especificações e respectivas funções, são apresentados na **Tabela 1 – Componentes eletrônicos utilizados e suas funções**.




<!-- Tabela 1 - Imagem centralizada com legenda -->
<p align="center">
  <!-- Imagem da tabela de componentes eletrônicos -->
  <img
    src="https://github.com/user-attachments/assets/c2a363e8-4386-4d4e-8ff6-f97a2236546d"
    width="900"
    alt="Tabela de componentes eletrônicos"
  >

  <!-- Legenda da tabela -->
  <br>
  <em>Tabela 1 - Componentes eletrônicos utilizados e suas funções.</em>
</p>



Após a definição dos componentes, iniciou-se o desenvolvimento do projeto eletrônico no software Altium Designer. Primeiramente, foi elaborado o esquemático elétrico, apresentado na **Figura 1**, no qual foram estabelecidas as conexões entre os componentes. Em seguida, foi desenvolvido o layout da placa de circuito impresso (PCB), apresentado na **Figura 2**, incluindo o posicionamento dos componentes, o roteamento das trilhas e a organização das conexões elétricas.


<!-- Figura 1 - Esquemático elétrico -->
<p align="center">
  <img
    src="https://github.com/user-attachments/assets/b4c3670d-6670-46a2-937a-53e1630f94ea"
    width="700"
    alt="Esquemático elétrico"
  >
  <br>
  <em>Figura 1 - Esquemático elétrico desenvolvido no Altium Designer.</em>
</p>

<!-- Figura 2 - Layout da placa eletrônica -->
<p align="center">
  <img
    src="https://github.com/user-attachments/assets/b00feebc-5ee0-4bf5-8a90-9f8e58ff877b"
    width="500"
    alt="Layout da placa eletrônica"
  >
  <br>
  <em>Figura 2 - Layout da placa eletrônica desenvolvido no Altium Designer.</em>
</p>





Com a finalização do projeto, foi realizada a fabricação da placa de circuito impresso. A **Figura 3** apresenta a PCB fabricada, ainda sem os componentes eletrônicos. Posteriormente, foi realizada a montagem e a soldagem dos componentes, resultando na placa montada, conforme apresentado nas **Figuras 4 e 5**.

Com a conclusão dessas etapas, a placa ficou preparada para a integração com os motores, sensores e demais dispositivos eletrônicos da máquina de cookies.

Observação: A lista considera os componentes do projeto e das montagens mostradas nas figuras. Nem todos estão necessariamente soldados diretamente à PCB; alguns, como motores e sensores, são conectados externamente.


<!-- Tabela com as três imagens da placa eletrônica lado a lado -->
<table align="center">
  <tr>
    <!-- Figura 3 - Placa não montada -->
    <td align="center" width="33%">
      <img
        src="https://github.com/user-attachments/assets/c8339860-ae59-4b57-97a1-9c2be5efcc6d"
        width="300"
        alt="Placa eletrônica não montada"
      >
      <br>
      <em>Figura 3 - Placa não montada.</em>
    </td>
    <!-- Figura 4 - Placa montada -->
    <td align="center" width="33%">
      <img
        src="https://github.com/user-attachments/assets/d3dfc36b-71f0-43ce-8ec3-400846454987"
        width="300"
        alt="Placa eletrônica montada"
      >
      <br>
      <em>Figura 4 - Placa montada.</em>
    </td>
    <!-- Figura 5 - Placa montada (outra vista) -->
    <td align="center" width="33%">
      <img
        src="https://github.com/user-attachments/assets/84ff2ed7-ee79-4e1e-8132-a9e9f5463654"
        width="300"
        alt="Placa eletrônica montada - outra vista"
      >
      <br>
      <em>Figura 5 - Placa montada (outra vista).</em>
    </td>
  </tr>
</table>

### Problemas enfrentados nessa etapa

Durante a fabricação e montagem da placa de circuito impresso, foram identificados alguns problemas que exigiram ajustes para garantir o funcionamento do circuito.

O primeiro problema estava relacionado ao **footprint do módulo conversor DC-DC (buck LM2596)**, cuja incompatibilidade com a disposição dos terminais do módulo utilizado foi identificada **somente após o processo de corrosão da placa**. Como a PCB já havia sido fabricada, optou-se por realizar a **soldagem do módulo na parte inferior da placa**, permitindo corrigir a disposição das conexões sem a necessidade de fabricar uma nova PCB.

Além disso, foi identificado um **problema de conexão na malha de GND (terra)**, que comprometeu a continuidade elétrica entre determinados pontos do circuito. Para solucionar essa falha, foi realizado um pequeno *jumper* com fio, restabelecendo a conexão elétrica necessária.

Após essas correções, foi possível dar continuidade à montagem e à integração dos componentes eletrônicos da máquina de cookies.
