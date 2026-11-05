window.setInterval=()=>1;
window.mock={role:'admin',calls:[],failSales:false,quantity:10,maintenance:false,limitActive:false};
window.fetch=async(url,options={})=>{
 const path=new URL(url).pathname,body=options.body?JSON.parse(options.body):null;
 mock.calls.push({path,body,authorization:options.headers?.Authorization});
 let data={},status=200;
 if(path==='/api/status')data={online:true,banco_online:true,manutencao:mock.maintenance,fim_curso_acionado:mock.limitActive};
 else if(path==='/api/manutencao/liberar'){
  if(options.headers?.Authorization!=='Bearer test-admin-token'){status=403;data={erro:'Administrador necessario'};}
  else if(mock.limitActive){status=409;data={erro:'Libere a chave de fim de curso'};}
  else {mock.maintenance=false;data={ok:true};}
 }
 else if(path==='/api/auth/login')data={token:'test-admin-token',usuario:{nome:'Responsavel',role:mock.role}};
 else if(path==='/api/auth/logout')data={ok:true};
 else if(path==='/api/estoque')data={itens:Array.from({length:6},(_,i)=>({slot:i+1,sabor:'Sabor '+(i+1),quantidade:i===0?mock.quantity:10,capacidade:20}))};
 else if(path.startsWith('/api/estoque/')){mock.quantity=body.quantidade;data={ok:true};}
 else if(path==='/api/vendas'){
  if(mock.failSales){status=503;data={erro:'Banco indisponivel'}}
  else data={vendas:[{evento_id:'evento-1',usuario:'<img src=x onerror=alert(1)>',registrado_em:1700000000,total:2,q1:2,q2:0,q3:0,q4:0,q5:0,q6:0}]};
 }
 return new Response(JSON.stringify(data),{status,headers:{'Content-Type':'application/json'}});
};
