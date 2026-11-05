# Executar em PowerShell como administrador. Acesso limitado a rede do ESP32.
$ErrorActionPreference = 'Stop'
$rules = @(
    @{ Name = 'CookieExpress-HTTP-ESP'; Protocol = 'TCP'; Port = 8080 },
    @{ Name = 'CookieExpress-Discovery-ESP'; Protocol = 'UDP'; Port = 8081 }
)
foreach ($rule in $rules) {
    if (-not (Get-NetFirewallRule -Name $rule.Name -ErrorAction SilentlyContinue)) {
        New-NetFirewallRule -Name $rule.Name -DisplayName $rule.Name -Direction Inbound -Action Allow -Protocol $rule.Protocol -LocalPort $rule.Port -RemoteAddress '192.168.4.0/24' -Profile Any | Out-Null
    }
}
Write-Output 'Liberadas TCP 8080 e UDP 8081 somente para 192.168.4.0/24.'
