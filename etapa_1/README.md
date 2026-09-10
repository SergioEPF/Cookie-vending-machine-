# Etapa 1

A etapa 1 foi dedicada à definição dos principais conceitos de hardware, software e mecânica da nossa cookie vending machine e de como essas três partes irão funcionar em conjunto. A proposta é desenvolver uma máquina automatizada que ofereça seis sabores de cookies, organizados em molas acopladas a motores de passo, com sensores para confirmar a entrega do produto e identificar a abertura da porta de manutenção. A interação com o usuário será feita por uma IHM, enquanto o microcontrolador será responsável por integrar esses elementos e controlar o funcionamento da máquina.

## Desenvolvimento

A ideia do projeto surgiu da proposta de desenvolver uma máquina de venda automática de pequeno porte, voltada a microempreendedores que produzem seus próprios alimentos e buscam uma alternativa para comercializá-los. Nesse contexto, escolhemos os cookies como produto para o desenvolvimento do protótipo.

### Conceito geral do funcionamento

A Figura 1 apresenta o diagrama de blocos do funcionamento geral da máquina. O usuário inicia a interação pela IHM, que se comunica por Wi-Fi com o microcontrolador. Este recebe o sabor selecionado, verifica as condições para a liberação e aciona o motor correspondente. A entrega do cookie é então verificada por meio do sensor ultrassônico.

<p align="center">
 
  <img width="500" height="500" alt="image" src="https://github.com/user-attachments/assets/fb7adfe5-52b8-4086-826e-4f593f78a32e" />


  <br>
  <em>Figura 1 — Diagrama de blocos da máquina.</em>
</p>

### Conceito geral do software 

A Figura 2 apresenta o fluxograma do software a ser implementado nas próximas etapas. Para realizar uma compra, o usuário deverá estar cadastrado e fazer login com sua senha. Após a seleção do sabor, o sistema verificará a disponibilidade em estoque e, caso haja produto disponível, acionará o motor correspondente. A entrega será confirmada pelo sensor. Caso o cookie não seja detectado, será necessário tratar a falha, definindo um limite de tentativas.

<p align="center">
  <img width="300" height="500" alt="Diagrama sem nome drawio" src="https://github.com/user-attachments/assets/1781f7c5-0e7b-4030-a75d-fe9271dd0d97" />
  <br>
  <em>Figura 2 — Fluxograma de funcionamento.</em>
</p>

### Armazenamento e liberação dos cookies

A alternativa escolhida para o desenvolvimento do protótipo é o sistema de espirais acionadas individualmente, aproveitando as espirais de aço mola já produzidas e permitindo a divisão dos produtos em canais independentes. Os cookies serão armazenados em embalagens individuais fechadas, posicionadas entre as espiras, com os canais separados por divisórias. A reposição será realizada por uma tampa localizada na parte superior da caixa, facilitando o acesso ao compartimento de armazenamento.

A estrutura da caixa será fabricada em MDF, com superfícies de apoio e bandeja de retirada lisas e removíveis, facilitando a limpeza e a manutenção. O compartimento eletrônico ficará isolado da área de armazenamento e do caminho de queda dos cookies.

<p align="center">
<img width="1200" height="400" alt="image" src="https://github.com/user-attachments/assets/c3e7c261-d5ff-4b7a-a3ab-4a08e82c8982" />
<br>
<em>Figura 3 — Molas fabricadas.</em>
</p>

<p align="center">
<img width="1200" height="400" alt="image" src="https://github.com/user-attachments/assets/5907f187-6b4f-49b5-9cb4-5f08e688e732" />
<br>
<em>Figura 4 — Protótipo Inicial da Máquina.</em>
</p>

### Interface com o usuário 

A interação com o usuário será realizada por meio de um celular utilizado como Interface Homem-Máquina (IHM), conectado ao microcontrolador por Wi-Fi. As telas foram organizadas para orientar cada etapa da compra: identificação por login e senha, seleção do sabor, conferência do pedido e acompanhamento da liberação do cookie. Ao final, a interface informará que o produto está disponível para retirada. 
As imagens abaixo apresentam exemplos das principais telas da interface. O fluxo completo pode ser consultado no [protótipo interativo no Figma](https://www.figma.com/proto/RLSzlbX6hY4JLrHZYsqM1v/Cookie-Vending-Machine---IHM-Mockup?node-id=2-19&viewport=-164%2C-284%2C0.24&t=vtPGIQQFpD4Fdbzo-1&scaling=scale-down&content-scaling=fixed&starting-point-node-id=2%3A19&page-id=0%3A1).

<table align="center">
  <tr>
    <td align="center">
      <img src="assets/login.jpeg"
           alt="Tela de login"
           width="220">
      <br>
      <em>Figura 5 — Tela de login.</em>
    </td>
    <td align="center">
      <img src="assets/escolha_cookie.jpeg"
           alt="Tela de escolha de sabor"
           width="210">
      <br>
      <em>Figura 6 — Escolha de sabor.</em>
    </td>
    <td align="center">
      <img src="assets/tela_final.jpeg"
           alt="Tela de compra concluída"
           width="220">
      <br>
      <em>Figura 7 — Compra concluída.</em>
    </td>
  </tr>
</table>

### Sensores 

A máquina contará com uma barreira infravermelha para confirmar a passagem do cookie, um sensor Hall com ímã para verificar a posição de cada espiral e um sensor magnético na tampa superior para bloquear os motores durante a reposição.


## Referências (links/datasheets/livros)


- [nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.2/nrf/getting_started/modifying.html#configure-application>)


