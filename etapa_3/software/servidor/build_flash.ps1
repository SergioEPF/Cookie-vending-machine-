param([string]$Port = 'COM4', [switch]$BuildOnly)
$ErrorActionPreference = 'Stop'
$env:IDF_PATH = 'C:\esp\v5.5.6\esp-idf'
$env:IDF_TOOLS_PATH = 'C:\Espressif\tools'
$env:IDF_PYTHON_ENV_PATH = 'C:\Espressif\tools\python\v5.5.6\venv'
$env:IDF_COMPONENT_LOCAL_STORAGE_URL = 'file://C:\Espressif\tools'
$env:ESP_ROM_ELF_DIR = 'C:\Espressif\tools\esp-rom-elfs\20260528'
$env:PATH = 'C:\Espressif\tools\python\v5.5.6\venv\Scripts;C:\Espressif\tools\cmake\3.30.2\bin;C:\Espressif\tools\ninja\1.12.1;C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20260121\xtensa-esp-elf\bin;C:\Espressif\tools\esp32ulp-elf\2.38_20240113\esp32ulp-elf\bin;' + $env:PATH
$pythonExe = Join-Path $env:IDF_PYTHON_ENV_PATH 'Scripts\python.exe'
& $pythonExe (Join-Path $PSScriptRoot 'configure_firmware.py')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$idfScript = Join-Path $env:IDF_PATH 'tools\idf.py'
& $pythonExe $idfScript build
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
if (-not $BuildOnly) {
    & $pythonExe $idfScript -p $Port flash
    exit $LASTEXITCODE
}
