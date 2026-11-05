"""Contrato HTTP do simulador: fluxo real das telas, sem hardware nem banco real."""
import json
import tempfile
import sys
import threading
import unittest
from pathlib import Path
from urllib.error import HTTPError
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT))
from servidor.testes.demonstracao.demo import create_demo_server


class DemoTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.now = 100.
        self.server = create_demo_server(0, Path(self.tmp.name) / 'demo.sqlite3', lambda: self.now)
        self.thread = threading.Thread(target=self.server.serve_forever)
        self.thread.start()
        self.base = 'http://127.0.0.1:' + str(self.server.server_port)
        self.client = self.login('teste', 'teste123')
        self.admin = self.login('admin', 'admin123')

    def tearDown(self):
        self.server.shutdown()
        self.thread.join()
        self.server.server_close()
        self.tmp.cleanup()

    def call(self, path, method='GET', body=None, token=''):
        request = Request(self.base + path, method=method,
                          data=None if body is None else json.dumps(body).encode(), headers={
                              'Content-Type': 'application/json', 'Authorization': 'Bearer '+token})
        try:
            with urlopen(request, timeout=10) as response:
                return response.status, json.load(response)
        except HTTPError as error:
            return error.code, json.load(error)

    def login(self, name, password):
        code, data = self.call('/api/auth/login', 'POST', dict(nome=name, senha=password))
        self.assertEqual(code, 200)
        return data['token']

    def order(self, **quantities):
        return self.call('/api/pedido', 'POST', dict(
            total_cookies=sum(quantities.values()), **quantities), self.client)

    def test_pages_are_local_and_private_files_inaccessible(self):
        self.assertTrue(self.call('/health')[1]['demo'])
        for path in ('/', '/backoffice'):
            with urlopen(self.base+path, timeout=5) as response:
                page = response.read().decode('utf-8')
            self.assertIn('const API_BASE=location.origin;', page)
            self.assertIn('DEMONSTRAÇÃO LOCAL', page)
            self.assertNotIn("const API_BASE='http://192.168.4.1';", page)
        with urlopen(self.base+'/simulador', timeout=5) as response:
            self.assertIn('Acionar chave', response.read().decode('utf-8'))
        for path in ('/data/admin-inicial.txt', '/data/esp.key', '/demo.py'):
            self.assertEqual(self.call(path)[0], 404)

    def test_full_delivery_stock_and_single_sale(self):
        self.assertEqual(self.order(q1=1, q6=2)[0], 200)
        self.assertEqual(self.order(q2=1)[0], 409)
        self.now += 3
        status = self.call('/api/pedido/status', token=self.client)[1]
        self.assertEqual((status['status'], status['cookies_entregues']), ('liberando', 1))
        self.now += 6
        for _ in range(2):
            status = self.call('/api/pedido/status', token=self.client)[1]
            self.assertEqual((status['status'], status['sensor'], status['cookies_entregues']),
                             ('entregue', 'ok cookie entregue', 3))
        stock = self.call('/api/estoque')[1]['itens']
        self.assertEqual([i['quantidade'] for i in stock], [9, 10, 10, 10, 10, 8])
        sales = self.call('/api/vendas', token=self.admin)[1]['vendas']
        self.assertEqual(len(sales), 1)
        self.assertEqual((sales[0]['usuario'], sales[0]['q6']), ('teste', 2))

    def test_maintenance_interrupts_and_requires_admin_after_release(self):
        self.order(q1=1, q6=2)
        self.now += 3
        self.assertEqual(self.call('/api/demo/fim-curso', 'POST', dict(acionado=True))[0], 200)
        self.now += 30
        order = self.call('/api/pedido/status', token=self.client)[1]
        self.assertEqual((order['status'], order['cookies_entregues']), ('erro', 1))
        self.assertEqual(self.order(q1=1)[0], 409)
        self.assertEqual(self.call('/api/manutencao/liberar', 'POST', {}, self.admin)[0], 409)
        self.call('/api/demo/fim-curso', 'POST', dict(acionado=False))
        self.assertTrue(self.call('/api/status')[1]['manutencao'])
        self.assertEqual(self.call('/api/manutencao/liberar', 'POST', {}, self.client)[0], 403)
        self.assertEqual(self.call('/api/manutencao/liberar', 'POST', {}, self.admin)[0], 200)
        self.assertFalse(self.call('/api/status')[1]['manutencao'])
        self.assertEqual(self.call('/api/vendas', token=self.admin)[1]['vendas'], [])
        self.assertEqual(self.call('/api/estoque')[1]['itens'][0]['quantidade'], 9)
        self.assertEqual(self.order(q6=1)[0], 200)

    def test_auth_validation_stock_and_order_ownership(self):
        self.assertEqual(self.call('/api/auth/login', 'POST', dict(nome='teste', senha='errada'))[0], 401)
        self.assertEqual(self.call('/api/pedido', 'POST', dict(total_cookies=1, q1=1))[0], 401)
        self.assertEqual(self.order(q1=4)[0], 400)
        self.assertEqual(self.order(q1=True)[0], 400)
        self.assertEqual(self.call('/api/estoque/ajuste', 'POST', dict(slot=6, quantidade=0), self.client)[0], 403)
        self.assertEqual(self.call('/api/estoque/ajuste', 'POST', dict(slot=6, quantidade=0), self.admin)[0], 200)
        self.assertEqual(self.order(q6=1)[0], 409)
        self.assertEqual(self.call('/api/estoque/reposicao', 'POST', dict(slot=6, quantidade=2), self.admin)[0], 200)
        self.assertEqual(self.order(q6=2)[0], 200)
        self.assertEqual(self.call('/api/pedido/status', token=self.admin)[0], 401)
        self.assertEqual(self.call('/api/estoque/ajuste', 'POST', dict(slot=1, quantidade=5), self.admin)[0], 409)
        self.assertEqual(self.call('/api/estoque', 'PUT', dict(itens=[]), self.admin)[0], 404)


if __name__ == '__main__':
    unittest.main()
