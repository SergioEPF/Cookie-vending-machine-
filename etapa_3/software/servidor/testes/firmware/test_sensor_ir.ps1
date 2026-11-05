param([string]$Compiler = 'C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20260121\xtensa-esp-elf\bin\xtensa-esp32-elf-g++.exe')
$ErrorActionPreference = 'Stop'
$root = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
& $Compiler -std=c++14 -fsyntax-only -Wall -Wextra -Werror -I (Join-Path $root 'main') (Join-Path $PSScriptRoot 'test_sensor_ir_eventos.cpp')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Output '17 cenarios da captura e filtro IR verificados com static_assert; nenhum hardware acionado.'
