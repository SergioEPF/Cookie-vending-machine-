param([string]$AdapterName = 'Wi-Fi 3')
$ErrorActionPreference = 'Stop'
$runtimePath = Join-Path (Split-Path $PSScriptRoot -Parent) '.runtime\diagnosticos'
New-Item -ItemType Directory -Path $runtimePath -Force | Out-Null
$logPath = Join-Path $runtimePath 'wifi-restart.log'
try {
    $adapter = Get-NetAdapter -Name $AdapterName
    if ($adapter.InterfaceDescription -notmatch 'Realtek RTL8188CU') {
        throw 'O adaptador selecionado nao e o Realtek investigado; nenhuma alteracao foi feita.'
    }
    Restart-NetAdapter -Name $AdapterName -Confirm:$false
    Get-NetAdapter -Name $AdapterName | Select-Object Name,Status,InterfaceDescription | Format-List | Out-String | Set-Content -LiteralPath $logPath -Encoding UTF8
    exit 0
} catch {
    $_ | Out-String | Set-Content -LiteralPath $logPath -Encoding UTF8
    exit 1
}
