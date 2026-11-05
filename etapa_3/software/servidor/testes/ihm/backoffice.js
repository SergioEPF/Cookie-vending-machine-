(async()=>{
 const results=[];
 const assert=(condition,message)=>{if(!condition)throw Error(message)};
 const wait=async predicate=>{for(let i=0;i<100;i++){if(predicate())return;await new Promise(r=>setTimeout(r,10))}throw Error('Tempo excedido')};
 const test=async(name,fn)=>{try{await fn();results.push({name,ok:true})}catch(error){results.push({name,ok:false,error:error.message})}};
 const login=()=>{$('name').value='Responsavel';$('password').value='senha123';$('login-form').dispatchEvent(new Event('submit',{bubbles:true,cancelable:true}))};
 await test('Cliente comum nao abre o painel e sua sessao e revogada',async()=>{
  mock.role='cliente';login();await wait(()=>!busy);
  assert(!active&&$('dashboard').hidden,'Cliente entrou no painel');
  assert(mock.calls.some(call=>call.path==='/api/auth/logout'),'Sessao recusada nao foi revogada');
 });
 await test('Administrador consulta estoque e vendas pelo ESP com token',async()=>{
  mock.role='admin';mock.calls=[];login();await wait(()=>active&&!busy);
  assert(!$('dashboard').hidden&&$('login').hidden,'Painel nao abriu');
  assert($('stock-body').rows.length===6,'Estoque incompleto');
  assert(mock.calls.find(call=>call.path==='/api/vendas').authorization==='Bearer test-admin-token','Vendas sem token');
 });
 await test('Historico mostra unidades e trata nome como texto',async()=>{
  assert($('sales-body').rows.length===1,'Venda ausente');
  assert(!$('sales-body').querySelector('img'),'Nome executou HTML');
  assert($('sales-summary').textContent.includes('2 cookies'),'Resumo incorreto');
 });
 await test('Alteracao envia slot e quantidade e relê o ESP',async()=>{
  mock.calls=[];$('qty-1').value='7';
  document.querySelector('[data-slot="1"][data-operation="ajuste"]').click();
  await wait(()=>!busy&&mock.calls.some(call=>call.path==='/api/vendas'));
  const call=mock.calls.find(call=>call.path==='/api/estoque/ajuste');
  assert(call.body.slot===1&&call.body.quantidade===7,'Alteracao incorreta');
  assert($('stock-body').rows[0].cells[1].textContent==='7','Nao releu estoque');
 });
 await test('Falha do banco avisa que historico pode estar antigo',async()=>{
  mock.failSales=true;await refresh();assert($('message').textContent.includes('desatualizados'),'Falha ocultada');mock.failSales=false;
 });
 await test('Fim de curso ativo exibe manutencao e impede liberacao',async()=>{
  mock.maintenance=true;mock.limitActive=true;mock.calls=[];await status();
  assert(!$('maintenance-panel').hidden,'Painel de manutencao ausente');
  assert($('maintenance-reset').disabled,'Liberacao permitida com chave ativa');
  $('maintenance-reset').click();
  assert(!mock.calls.some(c=>c.path==='/api/manutencao/liberar'),'Enviou liberacao com chave ativa');
 });
 await test('Soltar chave nao libera compras automaticamente',async()=>{
  mock.limitActive=false;await status();
  assert(maintenance&&!$('maintenance-panel').hidden,'Soltar chave removeu bloqueio');
  assert(!$('maintenance-reset').disabled,'Responsavel nao pode confirmar manutencao');
 });
 await test('Responsavel confirma manutencao com token administrativo',async()=>{
  mock.calls=[];$('maintenance-reset').click();await wait(()=>!busy&&!maintenance);
  const call=mock.calls.find(c=>c.path==='/api/manutencao/liberar');
  assert(call?.authorization==='Bearer test-admin-token','Liberacao sem sessao administrativa');
  assert($('maintenance-panel').hidden&&$('message').classList.contains('success'),'Liberacao nao atualizou o painel');
 });
 await test('Sair limpa token e dados privados da pagina',async()=>{
  $('logout').click();await wait(()=>!busy&&!active);
  assert(!token&&$('dashboard').hidden&&$('sales-body').textContent==='','Sessao ou historico permaneceu');
 });
 const pre=document.createElement('pre');pre.id='test-results';pre.textContent=JSON.stringify(results);document.body.appendChild(pre);
})().catch(error=>{const pre=document.createElement('pre');pre.id='test-results';pre.textContent=JSON.stringify([{name:'Execucao',ok:false,error:error.message}]);document.body.appendChild(pre)});
