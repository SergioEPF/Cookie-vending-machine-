# Executar como administrador. Configura somente o adaptador Wi-Fi do ESP.
param([switch]$Automatic, [string]$InterfaceAlias = 'Wi-Fi')
$ErrorActionPreference = 'Stop'
# O nome pode ser informado em outros computadores, sem um GUID pessoal fixo.
$adapter = Get-NetAdapter -Name $InterfaceAlias -ErrorAction Stop
$interfaceName = $adapter.Name
if ($Automatic) {
    & netsh interface ipv4 set address "name=$interfaceName" source=dhcp
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao restaurar DHCP.' }
    & netsh interface ipv4 set dnsservers "name=$interfaceName" source=dhcp
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao restaurar DNS automatico.' }
    Write-Output 'IP e DNS automaticos restaurados.'
} else {
    # DHCP padrao do ESP: .2 a .101. .200 fica fora dessa faixa.
    # Sem gateway: esta rede serve a maquina; a internet usa a outra conexao.
    & netsh interface ipv4 set address "name=$interfaceName" source=static address=192.168.4.200 mask=255.255.255.0 gateway=none store=persistent
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao configurar IP fixo.' }
    Write-Output 'IP fixo configurado: http://192.168.4.200:8080/'
}
