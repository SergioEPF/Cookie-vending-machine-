param([string]$BrowserPath, [string]$Page = 'index.html', [string]$Behavior = 'behavior.js', [string]$MockPath)
$ErrorActionPreference = 'Stop'
$testRoot = [System.IO.Path]::GetFullPath($PSScriptRoot)
$projectRoot = Split-Path (Split-Path $testRoot -Parent) -Parent
$sourcePath = Join-Path (Join-Path $projectRoot 'ihm') $Page
if (-not $BrowserPath) {
    $BrowserPath = @(
        'C:\Program Files\Google\Chrome\Application\chrome.exe'
        'C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe'
    ) | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
}
if (-not $BrowserPath) { throw 'Informe -BrowserPath com o caminho de Chrome ou Edge.' }
$runRoot = Join-Path $testRoot ('.run-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $runRoot | Out-Null
$utf8 = New-Object System.Text.UTF8Encoding($false)
try {
    $html = [System.IO.File]::ReadAllText($sourcePath)
    $mock = @'
window.setInterval=()=>1;
window.testServer={calls:[],loginStatus:200,registerStatus:200,meStatus:200,failStock:false,maintenance:false,limitActive:false,maintenanceOnOrder:false,stock:{itens:Array.from({length:6},(_,i)=>({slot:i+1,quantidade:10}))},orderStatus:{pedido_id:123,status:'liberando',sensor:'',cookies_entregues:0,total_cookies:3}};
window.fetch=async(url,options={})=>{
 const path=new URL(url).pathname,body=options.body?JSON.parse(options.body):null;
 testServer.calls.push({path,body});
 let data={},status=200;
 if(path==='/api/status')data={online:true,banco_online:true,venda_pendente:false,falha_persistencia:false,manutencao:testServer.maintenance,fim_curso_acionado:testServer.limitActive};
 else if(path==='/api/estoque'){if(testServer.failStock)throw Error('Rede indisponível');data=testServer.stock;}
 else if(path==='/api/auth/login'||path==='/api/auth/register'){
  status=path.endsWith('login')?testServer.loginStatus:testServer.registerStatus;
  data=status===200?{token:'token-de-teste',usuario:{nome:body.nome,sigla:body.sigla||'',role:'cliente'}}:{erro:'Credenciais ou cadastro recusados'};
 }
 else if(path==='/api/auth/me'){status=testServer.meStatus;data=status===200?{usuario:{nome:state.user.name}}:{erro:'Sessão expirada'};}
 else if(path==='/api/pedido'){
  if(testServer.maintenance||testServer.maintenanceOnOrder){status=409;data={erro:'Maquina em manutencao',manutencao:true};}
  else data={pedido_id:123,status:'recebido'};
 }
 else if(path==='/api/pedido/status')data=testServer.orderStatus;
 return new Response(JSON.stringify(data),{status,headers:{'Content-Type':'application/json'}});
};
// Ambiente isolado: não usa o banco de uma execução anterior do teste.
localStorage.clear();
'@
    if ($MockPath) { $mock = [System.IO.File]::ReadAllText((Join-Path $testRoot $MockPath)) }
    $tests = [System.IO.File]::ReadAllText((Join-Path $testRoot $Behavior))
    $html = $html.Replace('<script>', "<script>`n" + $mock + "`n").Replace('</script>', "`n" + $tests + "`n</script>")
    $fixture = Join-Path $runRoot 'fixture.html'
    [System.IO.File]::WriteAllText($fixture, $html, $utf8)
    $profile = Join-Path $runRoot 'profile'
    $stderr = Join-Path $runRoot 'browser.log'
    $uri = ([System.Uri]$fixture).AbsoluteUri
    $stdout = Join-Path $runRoot 'dom.html'
    $browserArgs = @('--headless', '--disable-gpu', '--no-first-run', '--no-default-browser-check', '--disable-background-networking', ('--user-data-dir="' + $profile + '"'), '--dump-dom', '--virtual-time-budget=10000', ('"' + $uri + '"'))
    $process = Start-Process -FilePath $BrowserPath -ArgumentList $browserArgs -WindowStyle Hidden -PassThru -Wait -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    if ($process.ExitCode -ne 0) { throw "O navegador encerrou com código $($process.ExitCode). Log: $stderr" }
    $dom = [System.IO.File]::ReadAllText($stdout)
    $match = [regex]::Match($dom, '<pre id="test-results">(.*?)</pre>', 'Singleline')
    if (-not $match.Success) { throw "O navegador não retornou os testes. Log: $stderr" }
    $results = [System.Net.WebUtility]::HtmlDecode($match.Groups[1].Value) | ConvertFrom-Json
    $results | Format-Table name,ok,error -AutoSize -Wrap
    if ($results | Where-Object { -not $_.ok }) { throw 'Há verificações com falha.' }
    Write-Output ("PASS: {0} verificações no JavaScript real da IHM." -f @($results).Count)
}
finally {
    # Confere o destino resolvido antes de apagar recursivamente os dados do teste.
    $resolvedRun = [System.IO.Path]::GetFullPath($runRoot)
    if ($resolvedRun.StartsWith($testRoot + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase) -and (Split-Path $resolvedRun -Leaf).StartsWith('.run-')) {
        Remove-Item -LiteralPath $resolvedRun -Recurse -Force -ErrorAction SilentlyContinue
    }
}
