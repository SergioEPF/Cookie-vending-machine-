# IHM e backoffice — versão 4.0

O servidor Python neste PC serve index.html em `/` (ou `/index.html`) e backoffice.html em `/backoffice` (ou `/backoffice.html`). Na rede COOKIE_EXPRESS, use `http://192.168.4.2:8080/` e `http://192.168.4.2:8080/backoffice`, substituindo o IP se o computador receber outro endereço. No próprio PC, também funciona `http://127.0.0.1:8080/`.

**Teste local sem ESP:** na raiz do projeto, execute
`powershell -NoProfile -ExecutionPolicy Bypass -File servidor/testes/demonstracao/iniciar.ps1`
e abra http://127.0.0.1:8082/. Entre com `teste` / `teste123`.
O backoffice de teste usa `admin` / `admin123`. Estoque e entregas são simulados,
com dados separados e descartados ao encerrar. Veja [instruções do simulador](../README.md).

No servidor normal, as duas páginas usam explicitamente a API do ESP em `http://192.168.4.1`; o controlador encaminha contas e histórico ao Python. O IP do servidor de páginas não é usado como API da máquina. A IHM envia a sigla ao banco no cadastro e pode entrar por nome ou sigla em qualquer aparelho. Alterar o HTML passa a valer ao recarregar a página servida pelo PC, sem gravar a placa.

No firmware simplificado, o ESP fornece somente a API. A raiz `http://192.168.4.1/` retorna o estado da máquina em JSON. Os HTMLs ficam exclusivamente no PC, sem cópias incorporadas ao binário da placa.

O backoffice exige papel administrativo, consulta estoque físico, permite reposição/ajuste e exibe as últimas 30 vendas entregues. Tokens permanecem apenas em memória. O cache JSON da IHM contém nomes/siglas e última consulta de estoque; não autentica nem autoriza compras.

Veja [uso, contas, gravação, rede, API e testes](../docs/integracao_python.md).

A IHM permite selecionar os seis sabores e exibe manutenção quando o fim de
curso do GPIO32 é acionado. Compras ficam bloqueadas até liberação pelo
responsável no backoffice, com a chave solta. A exportação de JSON foi removida.
Veja [mapa dos motores e manutenção](../docs/placa_motores_manutencao.md).
