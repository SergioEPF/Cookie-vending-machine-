param([switch]$Restore, [ValidateSet('EnableUsbSS','bFwCtrlLPS')][string]$Property = 'EnableUsbSS')
$ErrorActionPreference = 'Stop'
$runtimePath = Join-Path (Split-Path $PSScriptRoot -Parent) '.runtime\diagnosticos'
New-Item -ItemType Directory -Path $runtimePath -Force | Out-Null
$logPath = Join-Path $runtimePath 'wifi-power.log'
$backupName = if ($Property -eq 'EnableUsbSS') { 'wifi-power-before.json' } else { 'wifi-power-bFwCtrlLPS-before.json' }
$backupPath = Join-Path $runtimePath $backupName
try {
    $adapter = Get-NetAdapter -Name 'Wi-Fi 3'
    if ($adapter.InterfaceDescription -notmatch 'Realtek RTL8188CU') { throw 'Adaptador diferente do Realtek investigado.' }
    $setting = Get-NetAdapterAdvancedProperty -Name $adapter.Name -AllProperties -RegistryKeyword $Property
    if ($Restore) {
        $backup = Get-Content -LiteralPath $backupPath -Raw | ConvertFrom-Json
        if ($backup.InterfaceGuid -ne $adapter.InterfaceGuid.ToString()) { throw 'Backup pertence a outro adaptador.' }
        $value = [string]$backup.Value
    } else {
        if (-not (Test-Path -LiteralPath $backupPath)) {
            @{ InterfaceGuid=$adapter.InterfaceGuid.ToString(); Value=[string]$setting.RegistryValue[0] } | ConvertTo-Json | Set-Content -LiteralPath $backupPath -Encoding UTF8
        }
        $value = '0'
    }
    Set-NetAdapterAdvancedProperty -Name $adapter.Name -AllProperties -RegistryKeyword $Property -RegistryValue $value -NoRestart
    Restart-NetAdapter -Name $adapter.Name -Confirm:$false
    Get-NetAdapterAdvancedProperty -Name $adapter.Name -AllProperties -RegistryKeyword $Property | Select-Object Name,RegistryKeyword,RegistryValue | Format-List | Out-String | Set-Content -LiteralPath $logPath -Encoding UTF8
    exit 0
} catch {
    $_ | Out-String | Set-Content -LiteralPath $logPath -Encoding UTF8
    exit 1
}
