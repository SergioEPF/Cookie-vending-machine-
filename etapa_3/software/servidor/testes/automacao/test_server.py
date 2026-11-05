import json
import tempfile
import sys
import threading
import socket
import hmac
import hashlib
import queue
import unittest
from pathlib import Path
from urllib.request import Request, urlopen
from urllib.error import HTTPError
ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT))
from servidor.server import create_server, Database, discovery_loop


class ServerTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.db = Path(self.tmp.name) / 'test.sqlite3'
        self.server = create_server('127.0.0.1', 0, self.db, 'test-key')
        self.credentials = Path(self.tmp.name) / 'admin.txt'
        self.server.database.bootstrap_admin(self.credentials)
        self.thread = threading.Thread(target=self.server.serve_forever)
        self.thread.start()
        self.base = 'http://127.0.0.1:' + str(self.server.server_port)

    def tearDown(self):
        self.server.shutdown()
        self.thread.join()
        self.server.server_close()
        self.tmp.cleanup()

    def call(self, path, method='GET', body=None, key='test-key', token=''):
        raw = None if body is None else json.dumps(body).encode()
        request = Request(self.base + path, data=raw, method=method, headers={
            'Content-Type': 'application/json', 'X-ESP-Key': key,
            'Authorization': 'Bearer ' + token})
        try:
            with urlopen(request, timeout=10) as response:
                return response.status, json.load(response)
        except HTTPError as error:
            return error.code, json.load(error)

    def test_auth_persistence_logout(self):
        self.assertEqual(self.call('/health', key='')[0], 200)
        self.assertEqual(self.call('/api/estoque', key='wrong')[0], 401)
        self.assertEqual(self.call('/api/auth/register', 'POST', dict(nome='á'*32, senha='senha123'))[0], 400)
        user = dict(nome='Maria', sigla='MS', senha='teste123')
        status, data = self.call('/api/auth/register', 'POST', user)
        self.assertEqual(status, 200)
        self.assertEqual(self.call('/api/auth/register', 'POST', user)[0], 409)
        self.assertEqual(self.call('/api/auth/login', 'POST', dict(nome='MS', senha='errada'))[0], 401)
        status, data = self.call('/api/auth/login', 'POST', dict(nome='MS', senha='teste123'))
        self.assertEqual(status, 200)
        token = data['token']
        self.server.database = Database(self.db)
        self.assertEqual(self.call('/api/auth/me', token=token)[1]['nome'], 'Maria')
        self.assertEqual(self.call('/api/auth/logout', 'POST', {}, token=token)[0], 200)
        self.assertEqual(self.call('/api/auth/me', token=token)[0], 401)

    def test_public_html_does_not_expose_private_files(self):
        for path, title in (('/', 'Docce Luna'), ('/index.html', 'Docce Luna'),
                            ('/backoffice', 'Backoffice'), ('/backoffice.html', 'Backoffice')):
            with urlopen(self.base + path, timeout=5) as response:
                self.assertEqual(response.status, 200)
                self.assertEqual(response.headers.get_content_type(), 'text/html')
                self.assertEqual(response.headers.get_content_charset(), 'utf-8')
                self.assertEqual(response.headers['Cache-Control'], 'no-store')
                html = response.read().decode('utf-8')
                self.assertIn(title, html)
                self.assertIn("const API_BASE='http://192.168.4.1';", html)
        for path in ('/guia', '/guia.html'):
            with urlopen(self.base + path, timeout=5) as response:
                self.assertEqual(response.status, 200)
                self.assertEqual(response.headers.get_content_type(), 'text/html')
                self.assertEqual(response.headers.get_content_charset(), 'utf-8')
                self.assertEqual(response.headers['Cache-Control'], 'no-store')
                html = response.read().decode('utf-8')
                self.assertIn('Guia do projeto', html)
                self.assertIn('SOURCE:main/controller.c:START', html)
                self.assertNotIn('#define BACKEND_KEY', html)
        for path in ('/data/esp.key', '/data/admin-inicial.txt', '/server.py',
                     '/ihm/../servidor/data/esp.key', '/%2e%2e/servidor/data/esp.key'):
            self.assertEqual(self.call(path, key='')[0], 401)
            self.assertEqual(self.call(path)[0], 404)

    def test_inventory_atomic_and_sales_idempotent(self):
        items = [dict(slot=i, quantidade=10, capacidade=20) for i in range(1,7)]
        self.assertEqual(self.call('/api/estoque', 'PUT', dict(itens=items))[0], 200)
        items[-1]['slot'] = 1
        self.assertEqual(self.call('/api/estoque', 'PUT', dict(itens=items))[0], 400)
        self.assertEqual(len(self.call('/api/estoque')[1]['itens']), 6)
        sale = dict(evento_id='esp01:uuid-persistente', usuario='Maria', status='entregue', total=1,
                    q1=1, q2=0, q3=0, q4=0, q5=0, q6=0)
        self.assertEqual(self.call('/api/vendas', 'POST', sale)[0], 200)
        self.assertTrue(self.call('/api/vendas', 'POST', sale)[1]['duplicada'])
        sale['usuario'] = 'Outra'
        self.assertEqual(self.call('/api/vendas', 'POST', sale)[0], 409)
        self.assertEqual(self.call('/api/vendas')[0], 401)
        password = self.credentials.read_text().split('Senha: ')[1].strip()
        _, admin = self.call('/api/auth/login', 'POST', dict(nome='admin', senha=password))
        self.assertEqual(admin['usuario']['role'], 'admin')
        self.assertEqual(len(self.call('/api/vendas', token=admin['token'])[1]['vendas']), 1)
        _, client = self.call('/api/auth/register', 'POST', dict(nome='Cliente', sigla='CL', senha='senha123', role='admin'))
        self.assertEqual(client['usuario']['role'], 'cliente')
        self.assertEqual(self.call('/api/vendas', token=client['token'])[0], 403)
        self.assertEqual(self.call('/api/auth/register', 'POST', dict(nome='CL', senha='senha123'))[0], 409)
        self.assertEqual(self.call('/api/auth/register', 'POST', [1])[0], 400)

    def test_discovery_signed_response(self):
        stop = threading.Event()
        ready = queue.Queue()
        worker = threading.Thread(target=discovery_loop, args=(self.server.server_port, 'test-key', stop, ready, 0))
        worker.start()
        try:
            discovery_port = ready.get(timeout=3)
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
                sock.settimeout(2)
                nonce = '0123456789abcdef' * 2
                sock.sendto(('CE1:'+nonce).encode(), ('127.0.0.1',discovery_port))
                response, _ = sock.recvfrom(256)
                payload = f'{nonce}:{self.server.server_port}'
                signature = hmac.new(b'test-key', payload.encode(), hashlib.sha256).hexdigest()
                self.assertEqual(response.decode(), f'CE1:{payload}:{signature}')
        finally:
            stop.set()
            worker.join(timeout=3)


if __name__ == '__main__':
    unittest.main()
