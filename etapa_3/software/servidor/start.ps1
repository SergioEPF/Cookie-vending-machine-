param(
    [string]$BindAddress = '0.0.0.0',
    [int]$Port = 8080,
    [string]$Python = 'C:\Espressif\tools\python\python.exe',
    [switch]$Background,
    [switch]$Restart
)
$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $Python)) {
    throw 'Informe um Python 3.10+ valido com -Python CAMINHO.'
}
$serverScript = Join-Path $PSScriptRoot 'server.py'
$dataPath = Join-Path $PSScriptRoot 'data'
$pidPath = Join-Path $dataPath 'server.pid'
if ($Restart) {
    if (-not $Background) { throw 'Use -Restart junto com -Background.' }
    if (Test-Path -LiteralPath $pidPath) {
        $previousPid = [int](Get-Content -LiteralPath $pidPath)
        $previousProcess = Get-CimInstance Win32_Process -Filter "ProcessId = $previousPid"
        if ($previousProcess) {
            $normalizedCommand = $previousProcess.CommandLine -replace '/', '\'
            if ($previousProcess.ExecutablePath -ne $Python -or $normalizedCommand.IndexOf($serverScript, [StringComparison]::OrdinalIgnoreCase) -lt 0) {
                throw 'PID salvo nao corresponde a este servidor; nenhum processo foi encerrado.'
            }
            Stop-Process -Id $previousPid
        }
    }
}
if ($Background) {
    New-Item -ItemType Directory -Path $dataPath -Force | Out-Null
    $serverArguments = @(('"' + $serverScript + '"'), '--host', $BindAddress, '--port', $Port)
    $serverProcess = Start-Process -FilePath $Python -ArgumentList $serverArguments -WorkingDirectory $PSScriptRoot -WindowStyle Hidden -RedirectStandardOutput (Join-Path $dataPath 'stdout.log') -RedirectStandardError (Join-Path $dataPath 'stderr.log') -PassThru
    $serverProcess.Id | Set-Content -LiteralPath $pidPath
    Write-Output ('Servidor iniciado em segundo plano. PID: ' + $serverProcess.Id)
    exit 0
}
& $Python $serverScript --host $BindAddress --port $Port
exit $LASTEXITCODE
