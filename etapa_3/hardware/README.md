
# 5.1 Desenvolvimento e montagem da placa eletrônica

Foi desenvolvida uma placa de circuito impresso (PCB) para integrar os componentes eletrônicos responsáveis pelo controle dos mecanismos de distribuição da máquina de cookies.

O projeto foi realizado no Altium Designer, iniciando pela elaboração do esquemático elétrico, apresentado na **Figura 12**, no qual foram definidas as conexões entre os componentes eletrônicos. Em seguida, foi desenvolvido o layout da PCB, apresentado na **Figura 13**, incluindo o posicionamento dos componentes, o roteamento das trilhas e a organização das conexões elétricas necessárias para o funcionamento do sistema.

Após a finalização do projeto, foi realizada a fabricação da placa de circuito impresso. A **Figura 14** apresenta a placa fabricada, ainda sem a montagem dos componentes eletrônicos. Posteriormente, foi realizada a soldagem dos componentes, resultando na placa montada, conforme apresentado na **Figura 15** e na **Figura 16**.

Com a conclusão dessa etapa, a placa ficou preparada para a integração com os motores, sensores e demais dispositivos eletrônicos da máquina de cookies.
A placa eletrônica da máquina de cookies foi desenvolvida para integrar os componentes responsáveis pela alimentação, controle dos motores e monitoramento dos sensores. Os principais componentes utilizados e suas respectivas funções são apresentados a seguir:

1. ESP32: Microcontrolador responsável pelo controle geral da máquina, realizando a leitura dos sensores, o processamento dos comandos e o acionamento dos motores responsáveis pela distribuição dos cookies.

2. Motores CC 12 V: Responsáveis pelo movimento das espirais dos seis compartimentos, permitindo a liberação dos cookies conforme a seleção realizada pelo usuário.

3. Transistores TIP122: Utilizados como dispositivos de chaveamento para acionar individualmente os motores de corrente contínua, permitindo que o ESP32 controle cargas alimentadas em 12 V.

4. Resistores de 560 Ω: Conectados às bases dos transistores TIP122, possuem a função de limitar a corrente de acionamento fornecida pelas saídas do ESP32.

5. Módulo conversor DC-DC LM2596: Responsável por reduzir a tensão de alimentação de 12 V para 5 V, permitindo alimentar o ESP32 e outros componentes eletrônicos que necessitam dessa tensão.

6. Módulo de carregamento rápido USB (QC 3.0): Utilizado para alimentar e carregar o celular integrado à máquina, convertendo a alimentação de 12 V em uma saída USB compatível.

7. Sensor infravermelho (emissor e receptor): Instalado na região de saída dos cookies, permite detectar a passagem do produto após o acionamento do motor, auxiliando na confirmação da distribuição.

8. Chave de fim de curso: Instalada na porta de manutenção, permite identificar sua abertura e impedir o acionamento dos motores enquanto a porta estiver aberta, aumentando a segurança durante a operação.

9. Bornes de conexão: Utilizados para realizar as conexões elétricas entre a PCB e os dispositivos externos, como motores, sensores e alimentação, facilitando a montagem e a manutenção.

10. Fonte de alimentação chaveada 12 V: Responsável por fornecer energia elétrica aos motores e aos circuitos da máquina, com conversão de tensão para os dispositivos que operam em 5 V.

11. Conectores e pinos de conexão: Permitem a instalação e a interligação dos módulos eletrônicos à PCB, facilitando a substituição de componentes e a manutenção do sistema.

12. Placa de circuito impresso (PCB): Responsável por integrar eletricamente os componentes por meio de trilhas de cobre, substituindo conexões individuais por uma estrutura organizada e permanente.

Observação: A lista considera os componentes do projeto e das montagens mostradas nas figuras. Nem todos estão necessariamente soldados diretamente à PCB; alguns, como motores e sensores, são conectados externamente.

Para o seu relatório, essa seção pode ser colocada logo após a 5.1 Desenvolvimento e montagem da placa eletrônica, com o título 5.2 Componentes eletrônicos e suas funções.

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
