# Servidor Python local — versão integrada

API HTTP com SQLite e biblioteca padrão Python 3.10+, mais hospedagem dos dois HTMLs em `/` e `/backoffice`. Contas, sessões com papel de cliente/administrador, cópia do estoque e histórico de vendas. O ESP32 acessa a API de dados com chave privada; navegadores carregam as páginas deste PC e chamam a API da placa em `http://192.168.4.1`.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/start.ps1
```

Verifique http://127.0.0.1:8080/health. O computador deve estar conectado ao Wi-Fi COOKIE_EXPRESS. A descoberta automática usa UDP 8081; HTTP usa TCP 8080. Conta admin e senha aleatória inicial em data/admin-inicial.txt. Dados persistentes em data/cookie_express.sqlite3; chave em data/esp.key.

Veja [configuração completa, Firewall, firmware, API e testes](docs/integracao_python.md).

Para testar **sem ESP**, execute na raiz do projeto:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File servidor/testes/demonstracao/iniciar.ps1
```

Abra http://127.0.0.1:8082/. Cliente: `teste` / `teste123`.
Backoffice: http://127.0.0.1:8082/backoffice, com `admin` / `admin123`.
Você também pode cadastrar um cliente de teste. Cada cookie é entregue
automaticamente após 3 segundos. Em `/simulador`, acione/solte a chave de fim de
curso e depois libere a manutenção no backoffice.

O simulador atende apenas neste PC, não procura o ESP e usa banco temporário
separado: estoque, usuários e vendas de demonstração são reiniciados ao abrir
um novo processo. Use Ctrl+C para encerrar, ou `-Background` para iniciar oculto
e `-Background -Restart` para reiniciar. O servidor real continua em 8080.
As senhas acima servem somente para a demonstração.
Os fontes do simulador estão em `servidor/testes/demonstracao/`; PID e logs ficam em
`servidor/testes/.runtime/demo/`. Os testes automáticos também foram separados do
serviço: veja [todos os comandos e diagnósticos](testes/README.md).

Telas na rede do ESP: `http://192.168.4.2:8080/` e `http://192.168.4.2:8080/backoffice` (IP atual do PC; confira com `ipconfig` se mudar). Somente os dois HTMLs são públicos; o servidor não oferece listagem de pastas nem acesso HTTP aos arquivos de dados e credenciais.
