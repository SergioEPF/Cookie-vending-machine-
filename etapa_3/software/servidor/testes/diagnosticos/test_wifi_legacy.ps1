param([switch]$Restore)
$ErrorActionPreference = 'Stop'
$runtimePath = Join-Path (Split-Path $PSScriptRoot -Parent) '.runtime\diagnosticos'
New-Item -ItemType Directory -Path $runtimePath -Force | Out-Null
$backupPath = Join-Path $runtimePath 'wifi-mode-before.json'
$logPath = Join-Path $runtimePath 'wifi-mode.log'
try {
    $adapter = Get-NetAdapter -Name 'Wi-Fi 3'
    if ($adapter.InterfaceDescription -notmatch 'Realtek RTL8188CU') { throw 'Adaptador diferente do Realtek investigado.' }
    $setting = Get-NetAdapterAdvancedProperty -Name $adapter.Name -RegistryKeyword 'WirelessMode'
    if ($Restore) {
        $backup = Get-Content -LiteralPath $backupPath -Raw | ConvertFrom-Json
        if ($backup.InterfaceGuid -ne $adapter.InterfaceGuid.ToString()) { throw 'Backup pertence a outro adaptador.' }
        $value = [string]$backup.Value
    } else {
        if (-not (Test-Path -LiteralPath $backupPath)) {
            @{ InterfaceGuid=$adapter.InterfaceGuid.ToString(); Value=[string]$setting.RegistryValue[0] } | ConvertTo-Json | Set-Content -LiteralPath $backupPath -Encoding UTF8
        }
        # Valor 4 = IEEE 802.11b/g, conforme enum do INF instalado.
        $value = '4'
    }
    Set-NetAdapterAdvancedProperty -Name $adapter.Name -RegistryKeyword 'WirelessMode' -RegistryValue $value -NoRestart
    Restart-NetAdapter -Name $adapter.Name -Confirm:$false
    Get-NetAdapterAdvancedProperty -Name $adapter.Name -RegistryKeyword 'WirelessMode' | Select-Object Name,DisplayName,DisplayValue,RegistryValue | Format-List | Out-String | Set-Content -LiteralPath $logPath -Encoding UTF8
    exit 0
} catch {
    $_ | Out-String | Set-Content -LiteralPath $logPath -Encoding UTF8
    exit 1
}
