# Cookie Machine
Software de venda de cookies: o ESP32 controla a máquina e o servidor Python oferece telas, contas e histórico de vendas.
Com Python 3.10+ instalado, conecte o PC ao Wi-Fi `COOKIE_EXPRESS` (senha `cookie123`).
Na raiz do projeto, abra o PowerShell e execute: `powershell -NoProfile -ExecutionPolicy Bypass -File servidor/start.ps1`.
Acesse http://127.0.0.1:8080/ (loja) ou http://127.0.0.1:8080/backoffice (administração); mantenha o terminal aberto.

