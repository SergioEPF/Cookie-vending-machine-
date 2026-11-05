# Cookie Express — ESP-IDF 5.5.6 + Python

## Organização do repositório

| Diretório | Conteúdo |
| --- | --- |
| `main/` | Firmware ESP32: API, controle, motores, IR e persistência |
| `servidor/` | Backend Python, telas, documentação, testes e scripts |

Há apenas duas pastas versionadas na raiz: `main/` e `servidor/`.
As telas ficam em `servidor/ihm/`, o guia em `servidor/docs/` e os testes
em `servidor/testes/`. Os comandos UART/BOOT compilados ficam em
`main/modo_teste.c`. Referências antigas ficam em `main/historico/` e
`servidor/historico/`, fora da compilação.

`CMakeLists.txt` e `sdkconfig.defaults` permanecem na raiz para o ESP-IDF,
junto do README e das regras do Git. `build/` e pastas locais do editor
podem existir no computador, mas não fazem parte do repositório publicado.

Veja o [resumo técnico e o estado da integração de pagamento](servidor/docs/arquitetura.md).
PDFs exportados, credenciais, bancos locais e artefatos de build são ignorados
pelo Git. Os documentos editáveis permanecem em `servidor/docs/`.

## Execução e desenvolvimento

Os arquivos antigos do repositório foram preservados em `main/historico/ESP.txt` e
`servidor/historico/IHM.txt`. O projeto atual está nas pastas `main` e `servidor`.

O ESP32 fornece o Wi-Fi e a API da máquina. O servidor Python neste PC serve a IHM e o backoffice e guarda contas e vendas em SQLite; o ESP mantém estoque físico, motores e confirmação de entrega.

Os testes e demonstrações estão separados em [`servidor/testes/`](servidor/testes/README.md):
simulador de PC, testes automáticos, cenários da IHM, diagnósticos de hardware e
comandos de teste do firmware. Dados reais permanecem em `servidor/data/`,
enquanto logs e backups dos testes ficam em `servidor/testes/.runtime/`, fora do Git.

Para usar **sem ESP**, inicie o simulador:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/testes/demonstracao/iniciar.ps1
```

Abra <http://127.0.0.1:8082/> com `teste` / `teste123`. Backoffice de demonstração:
`admin` / `admin123`. Cada cookie é entregue automaticamente após 3 segundos,
com banco temporário separado. Reiniciar o processo recria os dados iniciais.
Veja os [diagramas de comunicação](servidor/docs/comunicacao.md) dos dois ambientes.

Conecte computador e celular ao Wi-Fi COOKIE_EXPRESS (senha cookie123) e mantenha o Python executando:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/start.ps1
```

- IHM no próprio PC: http://127.0.0.1:8080/
- Backoffice no próprio PC: http://127.0.0.1:8080/backoffice
- Saúde do Python local: http://127.0.0.1:8080/health

Para usar sempre o mesmo endereço no celular, execute como administrador
`powershell -NoProfile -ExecutionPolicy Bypass -File servidor/configure_wifi_ip.ps1`.
O script fixa o adaptador Wi-Fi deste PC em `192.168.4.200`, fora da faixa DHCP
padrão do ESP. Conecte PC e celular ao Wi-Fi COOKIE_EXPRESS e abra
`http://192.168.4.200:8080/`. O PC e o servidor precisam permanecer ligados.
Se o adaptador tiver outro nome, informe `-InterfaceAlias 'Nome do adaptador'`.
Antes de usar esse adaptador em outra rede, execute o mesmo script com
`-Automatic` para restaurar IP e DNS automáticos. Sem IP fixo, consulte
o IPv4 do PC com `ipconfig` e use `http://IP_DO_PC:8080/`.
No servidor normal, as páginas usam a API do ESP em
`http://192.168.4.1`. A porta 8082 pertence exclusivamente à demonstração local.

Administrador inicial: admin, com senha aleatória em servidor/data/admin-inicial.txt. Contas antigas da NVS não são migradas; clientes se cadastram pela IHM.

Para compilar e gravar na COM4:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/build_flash.ps1 -Port COM4
```

Veja [instruções completas, API, rede e testes](servidor/docs/integracao_python.md).

Para estudar cada arquivo e o firmware comentado, abra o
[guia completo do projeto](servidor/docs/guia_projeto.html). Com o servidor ativo, ele também
está em `http://IP_DO_PC:8080/guia` (neste PC: <http://127.0.0.1:8080/guia>).
O guia funciona offline e permite buscar arquivos, explorar o fluxo de compra e
imprimir em PDF. Depois de editar os fontes, atualize seus blocos de código com
`& 'C:/Espressif/tools/python/python.exe' servidor/docs/atualizar_guia.py`; revise o texto
manualmente se houver alteração de comportamento. O atualizador também
incorpora os dois diagramas SVG de `servidor/docs/diagramas/` no HTML offline.

O `main/main.c` contém apenas inicialização. O controle da máquina está em
`main/controller.c`, com IR em `main/sensor_ir.c`, BOOT/UART em `main/modo_teste.c`
e NVS em `main/persistencia.c`. Pinos, tempos e modo de teste ficam em
`main/maquina_config.h`. As rotas HTTP estão em `main/api.c` e o acesso ao Python
em `main/backend.c`. O ESP fornece a API; os HTMLs são servidos exclusivamente pelo PC.

Os seis motores usam GPIO18, 19, 21, 22, 23 e 25, com pulso de três segundos por unidade. O fim de curso no GPIO32 indica porta aberta em LOW: para os motores, interrompe o pedido e bloqueia compras. Ao fechar a porta (GPIO32 em HIGH), a manutenção é liberada automaticamente, sem retomar pedidos interrompidos. Veja [pinagem e manutenção](servidor/docs/placa_motores_manutencao.md).

O IR usa 100 ms ligado/100 ms desligado. Cada cookie, repetição ou novo pedido aguarda pelo menos 2 segundos para acionar o motor, com o LED já pulsando no último segundo. Sem confirmação, faz até duas repetições e depois aguarda IR ou BOOT sem erro por falta de feixe ou timeout do IR. O fim de curso permanece ativo. S mostra diagnóstico e R cancela. Veja [ligação e modos do sensor](servidor/docs/sensor_ir.md). A recuperação completa de pedidos interrompidos permanece pendente.
