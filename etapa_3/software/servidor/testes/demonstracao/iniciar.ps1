param(
    [int]$Port = 8082,
    [string]$Python = 'C:\Espressif\tools\python\python.exe',
    [switch]$Background,
    [switch]$Restart
)
$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $Python)) { throw 'Informe um Python 3.10+ valido com -Python CAMINHO.' }
$demoScript = Join-Path $PSScriptRoot 'demo.py'
$runtimePath = Join-Path (Split-Path $PSScriptRoot -Parent) '.runtime\demo'
$pidPath = Join-Path $runtimePath 'server.pid'
if ($Restart) {
    if (-not $Background) { throw 'Use -Restart junto com -Background.' }
    if (Test-Path -LiteralPath $pidPath) {
        $previousPid = [int](Get-Content -LiteralPath $pidPath)
        $previousProcess = Get-CimInstance Win32_Process -Filter "ProcessId = $previousPid"
        if ($previousProcess) {
            $normalizedCommand = $previousProcess.CommandLine -replace '/', '\'
            if ($previousProcess.ExecutablePath -ne $Python -or $normalizedCommand.IndexOf($demoScript, [StringComparison]::OrdinalIgnoreCase) -lt 0) {
                throw 'PID salvo nao corresponde ao simulador; nenhum processo foi encerrado.'
            }
            $demoPreviousHandle = [System.Diagnostics.Process]::GetProcessById($previousPid)
            $demoPreviousHandle.Kill()
            if (-not $demoPreviousHandle.WaitForExit(5000)) { throw 'O simulador anterior nao encerrou.' }
        }
    }
}
if ($Background) {
    New-Item -ItemType Directory -Path $runtimePath -Force | Out-Null
    $demoArguments = @(('"' + $demoScript + '"'), '--port', $Port)
    $demoProcess = Start-Process -FilePath $Python -ArgumentList $demoArguments -WorkingDirectory $PSScriptRoot -WindowStyle Hidden -RedirectStandardOutput (Join-Path $runtimePath 'stdout.log') -RedirectStandardError (Join-Path $runtimePath 'stderr.log') -PassThru
    $demoProcess.Id | Set-Content -LiteralPath $pidPath
    Write-Output ('Simulador iniciado. PID: ' + $demoProcess.Id + ' | http://127.0.0.1:' + $Port + '/')
    exit 0
}
& $Python $demoScript --port $Port
exit $LASTEXITCODE
