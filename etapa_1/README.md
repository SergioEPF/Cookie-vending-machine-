# Etapa 1

A etapa 1 foi dedicada à definição dos principais conceitos de hardware, software e mecânica da nossa cookie vending machine e de como essas três partes irão funcionar em conjunto. A proposta é desenvolver uma máquina automatizada que ofereça seis sabores de cookies, organizados em molas acopladas a motores de passo, com sensores para confirmar a entrega do produto e identificar a abertura da porta de manutenção. A interação com o usuário será feita por uma IHM, enquanto o microcontrolador será responsável por integrar esses elementos e controlar o funcionamento da máquina.

## Desenvolvimento

A ideia do projeto surgiu da proposta de desenvolver uma máquina de venda automática de pequeno porte, voltada a microempreendedores que produzem seus próprios alimentos e buscam uma alternativa para comercializá-los. Nesse contexto, escolhemos os cookies como produto para o desenvolvimento do protótipo.

###Conceito geral do funcionamento

A Figura 1 apresenta o diagrama de blocos do funcionamento geral da máquina. O usuário inicia a interação pela IHM, que se comunica por Wi-Fi com o microcontrolador. Este recebe o sabor selecionado, verifica as condições para a liberação e aciona o motor correspondente. A entrega do cookie é então verificada por meio do sensor ultrassônico.

![Figura 1 - Diagrama de blocos da máquina](etapa_1/assets/diagrama_blocos_cookie_vending.png)
*Figura 1 — Diagrama de blocos da máquina.*




## Testes

Descrição dos testes/validações realizadas. Use fotos, diagramas, tabelas, etc.

## Referências (links/datasheets/livros)


- [nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.2/nrf/getting_started/modifying.html#configure-application>)


