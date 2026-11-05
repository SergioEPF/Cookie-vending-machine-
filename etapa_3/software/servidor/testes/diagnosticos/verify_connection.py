"""Verifica ESP -> Python e paginas, sem pedidos, ajustes de estoque ou motor."""
import argparse
import json
import time
from pathlib import Path
from urllib.request import build_opener, ProxyHandler, Request

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--host', default='http://192.168.4.1')
parser.add_argument('--pages-host', default='http://127.0.0.1:8080')
parser.add_argument('--samples', type=int, default=1)
parser.add_argument('--interval', type=int, default=20)
args = parser.parse_args()
opener = build_opener(ProxyHandler({}))


def request(path, body=None, token=None, html=False, host=None):
    headers = {'Content-Type': 'application/json'}
    if token:
        headers['Authorization'] = 'Bearer ' + token
    payload = None if body is None else json.dumps(body).encode()
    with opener.open(Request((host or args.host).rstrip('/') + path, data=payload, headers=headers), timeout=10) as response:
        raw = response.read().decode('utf-8')
        return raw if html else json.loads(raw)


for index in range(max(1, args.samples)):
    status = request('/api/status')
    assert status.get('online') is True and status.get('banco_online') is True, 'ESP ou Python indisponivel'
    print(json.dumps({'amostra': index + 1, 'hora': time.strftime('%H:%M:%S'), 'status': status}), flush=True)
    if index + 1 < args.samples:
        time.sleep(max(1, args.interval))

assert 'Docce Luna' in request('/', html=True, host=args.pages_host), 'IHM nao foi servida pelo PC'
assert 'Backoffice' in request('/backoffice', html=True, host=args.pages_host), 'Backoffice nao foi servido pelo PC'
inventory = request('/api/estoque')
assert len(inventory['itens']) == 6, 'Estoque sem seis slots'
credentials = (Path(__file__).resolve().parents[3] / 'servidor/data/admin-inicial.txt').read_text(encoding='utf-8')
password = credentials.split('Senha: ', 1)[1].strip()
session = request('/api/auth/login', {'nome': 'admin', 'senha': password})
token = session['token']
try:
    assert session['usuario']['role'] == 'admin'
    assert request('/api/auth/me', token=token)['usuario']['role'] == 'admin'
    sales = request('/api/vendas', token=token)
    assert isinstance(sales['vendas'], list)
    print(json.dumps({'ihm': 'ok', 'backoffice': 'ok', 'slots': len(inventory['itens']),
                      'login_admin_via_esp': 'ok', 'consulta_vendas_via_esp': 'ok',
                      'vendas_na_lista': len(sales['vendas'])}), flush=True)
finally:
    request('/api/auth/logout', {}, token=token)
    print('Sessao de verificacao encerrada. Nenhum motor ou estoque foi alterado.', flush=True)
