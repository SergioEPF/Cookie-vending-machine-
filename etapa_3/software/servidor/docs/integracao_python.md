# Uso do sistema — ESP32, IHM e Python

## Rede e telas

Conecte computador e celular ao Wi-Fi `COOKIE_EXPRESS`, senha `cookie123`.
Mantenha o computador ligado, sem suspensão, executando o Python:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/start.ps1
```

| Acesso | Endereço |
| --- | --- |
| IHM de clientes, servida pelo PC | http://192.168.4.2:8080/ |
| Backoffice administrativo, servido pelo PC | http://192.168.4.2:8080/backoffice |
| Saúde do Python neste computador | http://127.0.0.1:8080/health |

O Python serve os HTMLs diretamente da pasta `ihm`; basta recarregar a página
após uma edição, sem gravar novamente a placa. O IP `192.168.4.2` é o endereço
atual deste computador no Wi-Fi do ESP; se mudar, consulte `ipconfig` e substitua
nos links. No próprio PC, pode usar `http://127.0.0.1:8080/` e `/backoffice`.

A API das páginas é sempre `http://192.168.4.1`, mesmo abrindo os HTMLs pelo PC.
O ESP permite CORS com Authorization para essas chamadas. Não há chave privada
do servidor nos HTMLs. O Python publica somente `/`, `/index.html`, `/backoffice`
e `/backoffice.html`, sem listagem de pastas ou arquivos de dados.

O firmware simplificado fornece somente a API; `http://192.168.4.1/` retorna
o status em JSON. As páginas ficam exclusivamente no PC. Não é necessário
regravar a placa para atualizar os HTMLs.

O ESP envia um desafio aleatório por UDP para `192.168.4.255:8081`. O Python
responde com a porta HTTP e HMAC-SHA256. A placa valida com a chave compartilhada
e usa o IP de origem. Assim, o computador não precisa de IP fixo. Após falha de
transporte, o ESP refaz a descoberta. O AP admite até quatro clientes.

Em PowerShell **como administrador**, quando necessário:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/allow_firewall.ps1
```

Cria regras de entrada TCP 8080 e UDP 8081 limitadas à rede `192.168.4.0/24`.
Se usar outra porta HTTP, ajuste a regra TCP correspondente.

## Contas e acesso administrativo

No primeiro início da versão integrada, o Python cria o usuário `admin` com senha
aleatória em `servidor/data/admin-inicial.txt`. Não usa `admin/admin`.
Cadastros da IHM recebem papel `cliente`. O ESP consulta a sessão no Python antes
de cada compra e de cada ação administrativa. O comprador vem da sessão, sem
confiar no nome enviado no corpo do pedido. O HTML não pode escolher o papel.

Contas antigas da NVS não foram migradas: cadastre novamente pela IHM. Nome e
sigla passam a ficar no SQLite e funcionam em qualquer aparelho. O cache local da
IHM guarda somente nomes, siglas e última consulta de estoque; não aprova login,
nem autoriza compras e não contém senha ou token. O botão de exportação foi removido.
O armazenamento de páginas abertas por arquivo depende do navegador; prefira HTTP.

Senhas usam PBKDF2-SHA256, sal individual e 600.000 iterações. Tokens expiram em
12 horas, ficam como hash no banco e apenas em memória nas páginas. Sair solicita
revogação ao servidor. O banco anterior recebe automaticamente a coluna `role`;
contas existentes continuam clientes. Se já houver um cliente chamado `admin`,
o servidor avisa sobre o conflito e exige definir o administrador manualmente.

O backoffice consulta o estoque físico do ESP, permite somar reposição ou definir
quantidade e exibe as últimas 30 vendas completas com cookies por sabor. Mostra
contagens dessa lista, não um total histórico geral. Atualiza a cada 15 segundos,
com botão de atualização manual. Alterações são recusadas durante pedidos.

## Estoque, vendas e falhas

O estoque físico fica no ESP/NVS. A cópia SQLite é publicada a cada cinco segundos;
não autoriza compra nem é abatida automaticamente pelo servidor ao registrar venda.
A IHM consulta estoque após login, antes de comprar e a cada 30 segundos durante
a seleção. Sem consulta válida ou com banco indisponível, compras ficam bloqueadas.

Depois da confirmação de todas as entregas, a placa guarda uma venda pendente em
NVS com identificador aleatório. Reenvia até o Python confirmar sucesso. O SQLite
não duplica eventos idênticos; o mesmo ID com dados diferentes gera 409. Novos
pedidos aguardam o envio da venda anterior. Reiniciar com uma venda pendente
preserva o reenvio; não há recuperação completa de um pedido interrompido durante
o acionamento ou de entregas parciais após falta de energia. O histórico não
registra valores de dinheiro nem pedidos cancelados/parciais. O horário exibido
é o momento do registro no computador.

Sem Python, novos logins, compras e ações administrativas são recusados. O pedido
já aceito continua sob controle local; a consulta de andamento exige o token do
comprador desse pedido, sem consultar o banco a cada atualização.

Os seis motores usam GPIO18, 19, 21, 22, 23 e 25, respectivamente, com pulso
de três segundos por unidade e apenas um acionamento por vez.
A barreira IR confirma a entrega: TSSP58038 no GPIO27 e TSAL6200 no GPIO26,
com portadora de 38 kHz. O modo atual usa IR fisico (MODO_TESTE_SERIAL=0). BOOT permite confirmar manualmente apos a parada (BOOT_CONFIRMA_TESTE=1). D exige modo serial ativo; R cancela e S diagnostica. Confirmacoes manuais alteram estoque e podem registrar vendas reais.
Veja [ligação, limites e testes](sensor_ir.md). O backoffice não possui comando para acionar motores.
O GPIO apenas comanda um circuito de potência adequado, nunca o motor diretamente.

O fim de curso no GPIO32 fecha com GND ao acionar. Ele para todos os motores,
interrompe pedidos e bloqueia compras com aviso de manutenção na IHM. O bloqueio
é persistido; no firmware atual, fechar a porta libera automaticamente a
manutenção se não houver falha de persistência, sem retomar o pedido interrompido.
A rota administrativa também permite solicitar liberação com a chave solta. Veja
[pinagem, comportamento e verificação física](placa_motores_manutencao.md).

## Compilar e gravar

Com o servidor iniciado ao menos uma vez para gerar a chave:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/build_flash.ps1 -Port COM4
```

O script configura a instalação ESP-IDF 5.5.6 deste computador, copia `esp.key`
para `main/backend_secret.h` e compila/grava o firmware. Use
`-BuildOnly` para apenas compilar. Feche monitores seriais antes de gravar.
Não executa `erase-flash`. Ao trocar `esp.key`, compile e grave novamente.

## API

A API pública continua no ESP32. `/api/auth/register`, `/api/auth/login`,
`/api/auth/me` e `/api/auth/logout` são encaminhadas ao Python. Cadastro/login
retornam `token`, `nome`, `usuario:{nome,sigla,role}` e `expires_at`. O cadastro
recebe `{nome,sigla,senha}`; login recebe `{nome,senha}`, aceitando sigla em nome.

`GET /api/estoque` consulta a placa. `POST /api/pedido` exige sessão e usa as
quantidades `q1` a `q6`, até três cookies. `GET /api/pedido/status` exige a sessão
do comprador atual. `GET /api/vendas`, `GET /api/venda/ultima`,
`POST /api/estoque/reposicao` e `POST /api/estoque/ajuste` exigem administrador.
`GET /api/status` (também disponível em `/`) é público e informa versão, banco disponível, venda pendente e
falha de persistência. As rotas da placa permitem CORS para os HTMLs locais.

O status também informa manutenção e fim de curso. `POST /api/manutencao/liberar`
com `{}` exige administrador e recusa liberação com a chave ainda acionada.

No Python, toda API exige `X-ESP-Key`, exceto `GET /health`. Não exponha essa chave
ao HTML. O ESP acrescenta o cabeçalho internamente. JSON exige Content-Length e
Content-Type application/json, com limite de 16 KiB. Python não permite CORS.

| API interna Python | Finalidade |
| --- | --- |
| POST /api/auth/register, POST /api/auth/login | Contas e sessões |
| GET /api/auth/me, POST /api/auth/logout | Exigem também Authorization: Bearer TOKEN |
| PUT /api/estoque | Snapshot com seis slots em itens: slot, quantidade, capacidade |
| GET /api/estoque | Última cópia recebida da placa |
| POST /api/vendas | Receber venda confirmada pela placa |
| GET /api/vendas | Últimas 30 vendas, exige sessão admin |

Exemplo de venda enviada pelo firmware:

```json
{"evento_id":"identificador-unico-persistente","usuario":"Maria","status":"entregue","total":1,"q1":1,"q2":0,"q3":0,"q4":0,"q5":0,"q6":0}
```

## Dados privados e testes

`servidor/data` contém SQLite, chave e credencial inicial, ignorados no controle de
versão. Para backup simples, pare o servidor e copie essa pasta inteira. Não
compartilhe o header privado nem o firmware que incorpora a chave. HTTP não cifra
as credenciais: este é um protótipo em rede local confiável.

O script start usa `C:\Espressif\tools\python\python.exe` porque o Python do PATH
aponta para um ambiente virtual incompleto. Aceita `-Python CAMINHO`. Não instala
serviço do Windows nem inicialização automática; em primeiro plano, Ctrl+C encerra.
Use `-Background` para iniciar oculto, com PID e logs em `servidor/data`;
`-Background -Restart` reinicia apenas o processo identificado como este servidor.

```powershell
& C:\Espressif\tools\python\python.exe -m unittest discover -s servidor/testes/automacao -v
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/testes/ihm/run.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/testes/ihm/run.ps1 -Page backoffice.html -Behavior backoffice.js -MockPath backoffice-mock.js
```

Testes usam banco temporário, HTTP/UDP local e JavaScript real no Chrome/Edge com
respostas simuladas. Não acionam motores. Para teste físico sem movimento, abra
as duas páginas, consulte status/estoque e entre no backoffice para ler as vendas.

## Organização do firmware simplificado

- `main/main.c`: inicializa NVS, Wi-Fi e os módulos.
- `main/controller.c`: regras dos pedidos, motor, dados da API e sincronização.
- `main/sensor_ir.c`: emissão, recepção e diagnóstico da barreira IR.
- `main/modo_teste.c`: comandos D/R/S e botão BOOT com filtro contra repiques.
- `main/persistencia.c`: estoque, última venda e pendência na NVS.
- `main/maquina_config.h`: pinos, tempos, capacidade e modo de teste.
- `main/acionamento.c`: seis motores e entrada do fim de curso.
- `main/api.c`: rotas HTTP, CORS e validação de sessão pelo Python.
- `main/backend.c`: descoberta do PC e transporte HTTP autenticado.

Motor e serial compartilham uma tarefa; outra tarefa sincroniza o banco a cada
cinco segundos. Nenhuma chamada HTTP é feita enquanto o mutex do motor está
adquirido. As chaves existentes da NVS e as rotas `/api/*` foram preservadas.
O histórico completo continua no Python; a última venda local mantém a API antiga.

## Estado atual dos fontes — 03/11/2026

Testes e demonstração estão em `servidor/testes/`, incluindo o módulo de comandos de
teste do firmware. A operação normal continua em `main/`, `servidor/` e `servidor/ihm/`.
Veja [estrutura e comandos](../testes/README.md),
[diagramas da comunicação real e local](comunicacao.md) e
[guia HTML atualizado](guia_projeto.html#comunicacao).

O simulador sem ESP atende `127.0.0.1:8082` com banco temporário separado,
entrega automática, seis sabores e manutenção virtual. Inicie com
`powershell -NoProfile -ExecutionPolicy Bypass -File servidor/testes/demonstracao/iniciar.ps1`.
O servidor real em 8080 mantém a API privada para a placa e seus dados persistentes.

O código atual inclui seis motores, fim de curso e manutenção persistente.
A compilação e os testes isolados não comprovam o funcionamento elétrico;
as alterações recentes ainda precisam ser gravadas e verificadas na placa.
O registro abaixo descreve uma versão anterior, não a validação física dos
fontes atuais.

## Registro histórico de validação — 30/10/2026

Firmware compilado para ESP32 e gravado na COM4, hashes verificados e NVS
preservada. Leitura serial confirmou inicialização do Wi-Fi AP e do HTTP, sem
comandos de motor. Passaram 14 verificações da IHM, seis do backoffice e três
cenários da API Python (incluindo permissões e descoberta assinada).

Na validação inicial, o Realtek RTL8188CU não encontrou a rede. Em uma rodada
posterior, a troca da porta USB, o ajuste do ESP para canal 11/HT20 e o teste do
adaptador em b/g recuperaram a detecção e permitiram testes funcionais. Os modos
b/g/n e b/g apresentaram timeouts em observações mais longas; o modo do adaptador
e as opções de energia foram restaurados aos valores originais.

Em b/g, passaram cinco consultas durante 80 segundos e o teste físico completo:
páginas IHM/backoffice, seis slots, login administrativo, sessão, vendas e logout
por meio do ESP32. O Python recebeu estoque da placa com HTTP 200. O computador
recebeu `192.168.4.2`, mas a descoberta do servidor não depende desse IP fixo.
Uma leitura adicional voltou a falhar após o teste de 80 segundos. O celular
continuou acessando a placa durante a falha do PC. A estabilidade da conexão do
Realtek permanece pendente de teste comparativo com outro adaptador/computador.
Veja [evidências, configuração mantida e restauração](diagnostico_wifi.md).
