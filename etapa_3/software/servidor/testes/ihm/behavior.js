// Executado no HTML real, com fetch simulado e sem intervalos recorrentes.
(async()=>{
 const results=[];
 const assert=(condition,message)=>{if(!condition)throw Error(message);};
 const tick=()=>new Promise(resolve=>setTimeout(resolve,0));
 const waitFor=async(predicate)=>{for(let i=0;i<100;i++){if(predicate())return;await new Promise(resolve=>setTimeout(resolve,10));}throw Error('A operação assíncrona não terminou');};
 const test=async(name,run)=>{try{await run();results.push({name,ok:true});}catch(e){results.push({name,ok:false,error:e.message});}};
 const stock=(n=10)=>({itens:Array.from({length:6},(_,i)=>({slot:i+1,quantidade:n,capacidade:20}))});
 const input=(id,value)=>{document.getElementById(id).value=value;};
 const loginForm=(name='Ana Silva',password='senha123')=>{
  state.screen='login';state.busy=false;render();input('identity',name);input('password',password);
 };
 await tick();

 await test('Cache de disco recente não autoriza compras',async()=>{
  localDB.inventory={quantities:[10,10,10,10,10,10],syncedAt:Date.now()};saveDatabase();
  state.inventory=[...localDB.inventory.quantities];state.inventoryVerifiedAt=null;state.online=true;
  assert(!stockReady(),'Cache de disco liberou compra');
  state.cart=[];addSelection(0);assert(total()===0,'Permitiu seleção sem consulta');
 });
 await test('Estoque ordenado pelo slot e persistido como JSON',async()=>{
  testServer.stock={itens:stock().itens.reverse()};testServer.stock.itens[5].quantidade=7;
  assert(await syncInventory(),'Não sincronizou');
  assert(state.inventory[0]===7&&stockReady(),'Ordem dos slots ou validade incorreta');
  assert(readDatabase().inventory.quantities[0]===7,'Consulta não persistiu');
 });
 await test('Estoque malformado bloqueia compra e preserva última cópia',async()=>{
  const saved=JSON.stringify(localDB.inventory);
  for(const malformed of [stock().itens.slice(1),[{slot:1,quantidade:1},...stock().itens.slice(0,5)],stock().itens.map((x,i)=>({...x,quantidade:i===0?null:1})),stock().itens.map((x,i)=>({...x,quantidade:i===0?-1:1}))]){
   testServer.stock={itens:malformed};assert(!await syncInventory(),'Aceitou estoque inválido');
   assert(!stockReady(),'Estoque inválido liberou compra');
  }
  assert(JSON.stringify(localDB.inventory)===saved,'Sobrescreveu última cópia válida');
 });
 await test('Falha de rede mantém cache e bloqueia confirmação',async()=>{
  testServer.failStock=true;assert(!await syncInventory(),'Rede com falha aceita');
  state.screen='confirm';render();
  assert(document.querySelector('[data-action="place-order"]').disabled,'Confirmação habilitada sem sincronização');
  assert(state.stockError.includes('apenas para consulta'),'Não indicou cache');testServer.failStock=false;
 });
 await test('Sigla enviada ao servidor sem depender do cache local; senha errada rejeitada',async()=>{
  rememberProfile('Ana Silva','AS');testServer.loginStatus=401;testServer.calls=[];
  loginForm('AS','errada');await authenticate('login');
  assert(state.screen==='login'&&!state.token,'Login local aprovou senha inválida');
  const call=testServer.calls.find(x=>x.path===API.login);
  assert(call.body.nome==='AS'&&call.body.senha==='errada','Não encaminhou sigla/senha para aprovação');
  testServer.loginStatus=200;
 });
 await test('Nome desconhecido localmente pode entrar pelo ESP',async()=>{
  testServer.stock=stock();loginForm('Bruno Lima');await authenticate('login');
  assert(state.screen==='select'&&state.user.name==='Bruno Lima','Bloqueou conta ausente no celular');
  assert(readDatabase().profiles.some(p=>p.name==='Bruno Lima'),'Perfil aprovado não persistiu');
 });
 await test('Cadastro recusado não cria perfil local; aprovado guarda sigla',async()=>{
  const fillRegister=()=>{state.screen='register';render();input('name','Carla Souza');input('registration','CS');input('password','senha123');input('confirm','senha123');};
  testServer.registerStatus=409;fillRegister();await authenticate('register');
  assert(!localDB.profiles.some(p=>p.name==='Carla Souza'),'Guardou cadastro recusado');
  testServer.registerStatus=200;fillRegister();await authenticate('register');
  assert(findLocalProfile('cs')?.name==='Carla Souza','Não guardou sigla aprovada');
  assert(testServer.calls.filter(x=>x.path===API.register).at(-1).body.sigla==='CS','Não enviou sigla para o banco Python');
 });
 await test('Senha e token ausentes do JSON; releitura preserva sigla',async()=>{
  const raw=localStorage.getItem(LOCAL_DB_KEY);
  assert(!raw.includes('senha123')&&!raw.includes('token-de-teste'),'Persistiu segredo');
  assert(readDatabase().profiles.some(p=>p.registration==='CS'),'Perdeu sigla na releitura');
 });
 await test('JSON corrompido e armazenamento bloqueado não derrubam a IHM',async()=>{
  localStorage.setItem(LOCAL_DB_KEY,'{inválido');assert(readDatabase().profiles.length===0,'Não recuperou JSON inválido');
  const nativeSet=Storage.prototype.setItem;
  try{Storage.prototype.setItem=()=>{throw Error('Quota');};rememberProfile('Teste Local','TL');assert(storageWarning,'Não avisou falha de escrita');}
  finally{Storage.prototype.setItem=nativeSet;saveDatabase();}
 });
 await test('Sessão recusada impede envio de pedido',async()=>{
  state.screen='confirm';state.online=true;state.cart=[{id:0,qty:1}];testServer.meStatus=401;testServer.calls=[];
  await placeOrder();assert(!testServer.calls.some(x=>x.path===API.createOrder),'Enviou pedido sem sessão aprovada');
  testServer.meStatus=200;
 });
 await test('Banco indisponível bloqueia seleção mesmo com estoque consultado',async()=>{
  state.online=true;state.bankReady=false;state.inventoryVerifiedAt=performance.now();state.inventory=[10,10,10,10,10,10];
  assert(!stockReady(),'Banco offline liberou compra');
  state.bankReady=true;
 });
 await test('Nova consulta antes da compra rejeita estoque que acabou',async()=>{
  state.screen='confirm';state.online=true;testServer.stock=stock(0);testServer.calls=[];
  await placeOrder();assert(!testServer.calls.some(x=>x.path===API.createOrder),'Enviou produto esgotado');
  assert(state.error.includes('estoque mudou'),'Não explicou alteração do estoque');
 });
 await test('Pedido válido consulta sessão e estoque antes de enviar',async()=>{
  state.screen='confirm';state.online=true;state.cart=[{id:0,qty:3}];testServer.stock=stock();testServer.calls=[];
  await placeOrder();await tick();
  const paths=testServer.calls.map(x=>x.path);
  assert(paths.indexOf(API.me)<paths.indexOf(API.inventory)&&paths.indexOf(API.inventory)<paths.indexOf(API.createOrder),'Ordem de validação incorreta');
  assert(testServer.calls.find(x=>x.path===API.createOrder).body.q1===3,'Pedido alterado');
  assert(state.screen==='progress','Não iniciou acompanhamento');
  assert(document.querySelector('.progress-value').textContent==='0%','Exibiu progresso sem entrega');
 });
 await test('Sucesso exige contagem completa, atualiza JSON após entrega',async()=>{
  testServer.orderStatus={pedido_id:123,status:'entregue',sensor:'ok cookie entregue',cookies_entregues:1,total_cookies:3};
  startPolling();await waitFor(()=>state.order.delivered===1);assert(state.screen==='progress','Uma entrega finalizou pedido de três');
  testServer.orderStatus={...testServer.orderStatus,cookies_entregues:3};testServer.stock=stock(7);
  startPolling();await waitFor(()=>state.screen==='success'&&inventoryRequest===null);assert(state.screen==='success','Não finalizou entrega confirmada');
  assert(readDatabase().inventory.quantities[0]===7,'Não atualizou cache após entrega');
 });
 await test('Sexto motor aceito sem limitar compra ao slot 1',async()=>{
  stopPolling();state.screen='confirm';state.online=true;state.bankReady=true;state.maintenance=false;
  state.cart=[{id:5,qty:2}];state.token='token-de-teste';state.user={name:'Teste',registration:'T'};
  testServer.stock=stock();testServer.calls=[];
  await placeOrder();await tick();
  const call=testServer.calls.find(x=>x.path===API.createOrder);
  assert(call?.body.q6===2&&call.body.q1===0,'Slot 6 nao chegou ao pedido');
  assert(state.screen==='progress','Motor 6 foi bloqueado pela IHM');stopPolling();
 });
 await test('Exportacao JSON removida de todas as telas',async()=>{
  for(const screen of ['login','register','select','confirm','progress','success']){
   state.screen=screen;render();
   assert(!document.querySelector('[data-action="export-data"],[download]'),'Exportacao ainda visivel em '+screen);
  }
  assert(typeof exportDatabase==='undefined','Funcao de download permaneceu');
 });
 await test('Manutencao aparece antes do login e bloqueia selecao',async()=>{
  state.screen='login';testServer.maintenance=true;testServer.limitActive=true;
  await ping();
  assert(document.getElementById('maintenance-title'),'Aviso ausente antes do login');
  assert(!document.getElementById('loginForm'),'Formulario ocultou a manutencao');
  assert(connectionLabel()==='Em manutenção'&&!stockReady(),'Estado nao bloqueou compras');
 });
 await test('Nenhum pedido enviado enquanto ha manutencao',async()=>{
  testServer.calls=[];state.cart=[{id:2,qty:1}];const before=JSON.stringify(state.cart);
  addSelection(4);await placeOrder();
  assert(JSON.stringify(state.cart)===before,'Selecao alterada durante bloqueio');
  assert(!testServer.calls.some(x=>x.path===API.createOrder),'Pedido enviado durante bloqueio');
 });
 await test('Acionamento entre prechecagem e envio mostra manutencao no 409',async()=>{
  testServer.maintenance=false;testServer.limitActive=false;await ping();
  state.screen='confirm';state.cart=[{id:1,qty:1}];testServer.maintenanceOnOrder=true;
  await placeOrder();testServer.maintenanceOnOrder=false;
  assert(state.maintenance&&document.getElementById('maintenance-title'),'Erro 409 nao mostrou manutencao');
  assert(state.screen!=='success','Erro foi tratado como entrega');
 });
 await test('Manutencao durante entrega preserva contagem parcial e impede sucesso',async()=>{
  applyMachineStatus({manutencao:false},++requestGeneration);
  state.screen='progress';state.order={id:123,status:'liberando',delivered:0,requested:2,confirmed_by_sensor:false};
  testServer.orderStatus={pedido_id:123,status:'erro',sensor:'manutencao',cookies_entregues:1,total_cookies:2,manutencao:true};
  startPolling();await waitFor(()=>state.maintenance&&state.order.delivered===1);
  assert(state.screen!=='success'&&!state.order.confirmed_by_sensor,'Entrega parcial virou sucesso');
  assert(root.textContent.includes('1 de 2 unidades confirmadas'),'Contagem parcial ausente');
  assert(state.poll===null,'Polling continuou apos interrupcao');
 });
 await test('Resposta antiga nao remove manutencao mais recente',async()=>{
  const newer=++requestGeneration;applyMachineStatus({manutencao:true},newer);
  applyMachineStatus({manutencao:false},newer-1);
  assert(state.maintenance,'Resposta antiga removeu bloqueio');
  applyMachineStatus({manutencao:false},++requestGeneration);state.screen='select';render();
  assert(!document.getElementById('maintenance-title'),'Liberacao nao saiu da manutencao');
 });
 await test('Pedido interrompido permite voltar aos sabores apos liberacao',async()=>{
  state.screen='progress';state.order={id:123,status:'erro',delivered:1,requested:2,confirmed_by_sensor:false};
  state.maintenance=false;testServer.maintenance=false;testServer.calls=[];render();
  const button=document.querySelector('[data-action="dismiss-order"]');
  assert(button,'Saida do pedido interrompido ausente');button.click();
  await waitFor(()=>state.screen==='select'&&inventoryRequest===null);
  assert(state.order===null&&state.cart.length===0,'Pedido anterior permaneceu selecionado');
  assert(!testServer.calls.some(x=>x.path===API.createOrder),'Voltar aos sabores iniciou compra');
 });
 const pre=document.createElement('pre');pre.id='test-results';pre.textContent=JSON.stringify(results);document.body.appendChild(pre);
 document.title=results.every(x=>x.ok)?'TESTS PASSED':'TESTS FAILED';
})().catch(e=>{document.title='TESTS FAILED';const pre=document.createElement('pre');pre.id='test-results';pre.textContent=JSON.stringify([{name:'Execução',ok:false,error:e.message}]);document.body.appendChild(pre);});
