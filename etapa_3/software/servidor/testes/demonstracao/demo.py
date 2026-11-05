"""Simulador local da maquina, sem ESP e com banco descartavel a cada execucao."""
import argparse
import secrets
import sys
import tempfile
import threading
import time
from http.server import ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT))
from servidor.server import ApiError, Database, Handler, integer


BANNER = '''<aside style="padding:12px 16px;background:#fff0cc;color:#513414;
font:14px/1.6 Arial,sans-serif;text-align:center;border-bottom:1px solid #d3a052">
<strong>DEMONSTRAÇÃO LOCAL · sem ESP</strong><br>
Cliente: <b>teste</b> / <b>teste123</b> · Administrador: <b>admin</b> / <b>admin123</b><br>
<a href="/">IHM</a> · <a href="/backoffice" target="_blank" rel="noopener">Backoffice</a> ·
<a href="/simulador" target="_blank" rel="noopener">Simular fim de curso</a>
<br>Pedidos e entregas são simulados. Os dados são apagados ao encerrar o simulador.
</aside>'''


class Simulator:
    """Replica apenas a API usada pelas telas; nunca acessa a placa nem o banco real."""

    def __init__(self, path, clock=time.monotonic):
        self.database = Database(path)
        self.clock = clock
        self.lock = threading.Lock()
        self.order = None
        self.next_id = 1
        self.maintenance = self.limit_active = False
        for name, alias, password in [('teste', 'TS', 'teste123'), ('admin', 'ADM', 'admin123')]:
            self.database.dispatch('POST', '/api/auth/register',
                                   dict(nome=name, sigla=alias, senha=password), '')
        with self.database.connect() as db:
            db.execute("UPDATE users SET role='admin' WHERE nome='admin'")
            db.execute('DELETE FROM sessions')
        self.database.dispatch('PUT', '/api/estoque', dict(itens=[
            dict(slot=i, quantidade=10, capacidade=20) for i in range(1, 7)]), '')

    def connect(self):
        return self.database.connect()

    def inventory(self):
        return self.database.dispatch('GET', '/api/estoque', {}, '')[1]

    def user(self, authorization, admin=False):
        user = self.database.dispatch('GET', '/api/auth/me', {}, authorization)[1]['usuario']
        if admin and user['role'] != 'admin':
            raise ApiError(403, 'Acesso administrativo necessario')
        return user

    def advance(self):
        # Uma unidade a cada 3 s. A consulta recupera o progresso mesmo em outra aba.
        order = self.order
        if not order or order['status'] not in ('recebido', 'liberando'):
            return
        elapsed = max(0, self.clock() - order['started'])
        delivered = min(order['total_cookies'], int(elapsed / 3))
        sequence = order['sequence']
        items = self.inventory()['itens']
        for slot in sequence[order['cookies_entregues']:delivered]:
            items[slot - 1]['quantidade'] -= 1
        self.database.dispatch('PUT', '/api/estoque', dict(itens=items), '')
        order['cookies_entregues'] = delivered
        order['status'] = 'recebido' if elapsed < .5 else 'liberando'
        if delivered == order['total_cookies']:
            sale = dict(evento_id=order['event'], usuario=order['usuario'],
                        total=delivered, status='entregue', **order['quantities'])
            self.database.dispatch('POST', '/api/vendas', sale, '')
            order.update(status='entregue', sensor='ok cookie entregue')

    def dispatch(self, method, path, body, authorization):
        with self.lock:
            self.advance()
            if method == 'GET' and path == '/api/status':
                return 200, dict(online=True, banco_online=True, demo=True,
                                 venda_pendente=False, falha_persistencia=False,
                                 manutencao=self.maintenance, fim_curso_acionado=self.limit_active,
                                 fim_curso_gpio=32)
            if method == 'POST' and path == '/api/demo/fim-curso':
                active = body.get('acionado')
                if type(active) is not bool:
                    raise ApiError(400, 'Envie acionado como booleano')
                self.limit_active = active
                if active:
                    self.maintenance = True
                    if self.order and self.order['status'] in ('recebido', 'liberando'):
                        self.order.update(status='erro', sensor='Entrega interrompida: manutencao')
                return 200, dict(ok=True, manutencao=self.maintenance, fim_curso_acionado=active)
            if (method, path) in {
                ('POST', '/api/auth/register'), ('POST', '/api/auth/login'),
                ('GET', '/api/auth/me'), ('POST', '/api/auth/logout'), ('GET', '/api/vendas')
            }:
                return self.database.dispatch(method, path, body, authorization)
            if method == 'GET' and path == '/api/estoque':
                return 200, dict(self.inventory(), origem='simulador_local')
            if method == 'GET' and path == '/api/pedido/status':
                if not self.order or self.order['token'] != authorization:
                    raise ApiError(401, 'Pedido pertence a outra sessao')
                return 200, {key: self.order[key] for key in (
                    'pedido_id', 'status', 'sensor', 'cookies_entregues', 'total_cookies')
                } | dict(manutencao=self.maintenance)
            if method == 'POST' and path == '/api/pedido':
                user = self.user(authorization)
                if self.maintenance:
                    return 409, dict(erro='Maquina em manutencao', manutencao=True)
                if self.order and self.order['status'] in ('recebido', 'liberando'):
                    raise ApiError(409, 'Existe um pedido em andamento')
                quantities = {'q'+str(i): integer(body.get('q'+str(i), 0), 0, 3) for i in range(1, 7)}
                total = integer(body.get('total_cookies'), 1, 3)
                if sum(quantities.values()) != total:
                    raise ApiError(400, 'Total inconsistente')
                for item in self.inventory()['itens']:
                    if quantities['q'+str(item['slot'])] > item['quantidade']:
                        raise ApiError(409, 'Estoque insuficiente')
                self.order = dict(pedido_id=self.next_id, status='recebido', sensor='',
                                  cookies_entregues=0, total_cookies=total, usuario=user['nome'],
                                  token=authorization, started=self.clock(), quantities=quantities,
                                  sequence=[i for i in range(1, 7) for _ in range(quantities['q'+str(i)])],
                                  event='demo:'+secrets.token_hex(16))
                self.next_id += 1
                return 200, dict(pedido_id=self.order['pedido_id'], status='recebido')
            if method == 'POST' and path == '/api/manutencao/liberar':
                self.user(authorization, admin=True)
                if self.limit_active:
                    raise ApiError(409, 'Libere a chave de fim de curso antes de retirar a manutencao')
                self.maintenance = False
                return 200, dict(ok=True)
            if method == 'POST' and path in ('/api/estoque/reposicao', '/api/estoque/ajuste'):
                self.user(authorization, admin=True)
                if self.order and self.order['status'] in ('recebido', 'liberando'):
                    raise ApiError(409, 'Nao alterar estoque com pedido ativo')
                slot = integer(body.get('slot'), 1, 6)
                quantity = integer(body.get('quantidade'), 1 if path.endswith('reposicao') else 0, 10000)
                items = self.inventory()['itens']
                item = items[slot - 1]
                item['quantidade'] = min(item['capacidade'], quantity + (
                    item['quantidade'] if path.endswith('reposicao') else 0))
                self.database.dispatch('PUT', '/api/estoque', dict(itens=items), '')
                return 200, dict(ok=True, slot=slot, quantidade=item['quantidade'])
            raise ApiError(404, 'Rota nao encontrada no simulador')


class DemoHandler(Handler):
    def respond_bytes(self, status, raw, content_type):
        if content_type.startswith('text/html'):
            page = raw.decode('utf-8')
            marker = "const API_BASE='http://192.168.4.1';"
            if marker in page:
                # Somente a copia servida pelo simulador usa a API local.
                page = page.replace(marker, 'const API_BASE=location.origin;', 1)
                page = page.replace('<body>', '<body>'+BANNER, 1)
                raw = page.encode('utf-8')
        super().respond_bytes(status, raw, content_type)

    def handle_demo(self):
        path = urlsplit(self.path).path
        if self.command == 'GET' and path == '/health':
            return self.respond(200, dict(ok=True, demo=True))
        if self.command == 'GET' and path == '/simulador':
            page = Path(__file__).with_name('simulador.html').read_bytes()
            return self.respond_bytes(200, page, 'text/html; charset=utf-8')
        super().handle_api()

    do_GET = do_POST = do_PUT = handle_demo


def create_demo_server(port, path, clock=time.monotonic):
    simulator = Simulator(path, clock)
    server = ThreadingHTTPServer(('127.0.0.1', port), DemoHandler)
    server.database = simulator
    server.machine_key = ''  # API de demonstracao, disponivel apenas neste PC.
    server.daemon_threads = True
    return server


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', type=int, default=8082)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='cookie-express-demo-') as data:
        server = create_demo_server(args.port, Path(data) / 'demo.sqlite3')
        print(f'IHM de demonstracao: http://127.0.0.1:{server.server_port}/', flush=True)
        print('Cliente: teste / teste123 | Administrador: admin / admin123', flush=True)
        print('Sem ESP. Dados descartados ao encerrar. Ctrl+C para parar.', flush=True)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass
        finally:
            server.server_close()


if __name__ == '__main__':
    main()
