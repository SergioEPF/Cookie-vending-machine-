"""API local Cookie Express. Somente biblioteca padrao Python 3.10+."""
import argparse
import hashlib
import hmac
import json
import logging
import secrets
import sqlite3
import time
import socket
import threading
from contextlib import contextmanager
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit

ROOT = Path(__file__).resolve().parent
PUBLIC_PAGES = {
    '/': ROOT / 'ihm/index.html',
    '/index.html': ROOT / 'ihm/index.html',
    '/backoffice': ROOT / 'ihm/backoffice.html',
    '/backoffice.html': ROOT / 'ihm/backoffice.html',
    # Guia público com fontes comentados; não expõe diretórios ou dados locais.
    '/guia': ROOT / 'docs/guia_projeto.html',
    '/guia.html': ROOT / 'docs/guia_projeto.html',
}
FLAVORS = ['Chocolate', 'Chocolate Branco', 'Red Velvet', 'Baunilha', "M&M's", 'Integral']


class ApiError(Exception):
    def __init__(self, status, message):
        self.status, self.message = status, message


def text(data, key, limit=63, minimum=1):
    value = data.get(key)
    if not isinstance(value, str) or not minimum <= len(value.strip()) <= limit:
        raise ApiError(400, 'Campo invalido: ' + key)
    return value.strip()


def integer(value, low, high):
    if type(value) is not int or not low <= value <= high:
        raise ApiError(400, 'Numero inteiro fora dos limites')
    return value


class Database:
    def __init__(self, path):
        self.path = str(path)
        Path(path).parent.mkdir(parents=True, exist_ok=True)
        with self.connect() as db:
            db.executescript('''
                PRAGMA journal_mode=WAL;
                CREATE TABLE IF NOT EXISTS users (
                    nome TEXT PRIMARY KEY, sigla TEXT NOT NULL, salt TEXT NOT NULL,
                    hash TEXT NOT NULL, created_at INTEGER NOT NULL);
                CREATE UNIQUE INDEX IF NOT EXISTS unique_sigla ON users(sigla) WHERE sigla != '';
                CREATE TABLE IF NOT EXISTS sessions (
                    token_hash TEXT PRIMARY KEY, nome TEXT NOT NULL REFERENCES users(nome),
                    expires_at INTEGER NOT NULL);
                CREATE TABLE IF NOT EXISTS inventory (
                    slot INTEGER PRIMARY KEY, sabor TEXT NOT NULL, quantidade INTEGER NOT NULL,
                    capacidade INTEGER NOT NULL, updated_at INTEGER NOT NULL);
                CREATE TABLE IF NOT EXISTS sales (
                    event_id TEXT PRIMARY KEY, payload TEXT NOT NULL, created_at INTEGER NOT NULL);
            ''')
            if 'role' not in {r[1] for r in db.execute('PRAGMA table_info(users)')}:
                db.execute("ALTER TABLE users ADD COLUMN role TEXT NOT NULL DEFAULT 'cliente'")

    def bootstrap_admin(self, credential_file):
        with self.connect() as db:
            if db.execute("SELECT 1 FROM users WHERE role='admin'").fetchone():
                return
            if db.execute("SELECT 1 FROM users WHERE nome='admin'").fetchone():
                raise RuntimeError('Conta admin ja existe como cliente; defina o administrador manualmente no banco')
            password = secrets.token_urlsafe(12)
            salt = secrets.token_hex(16)
            digest = hashlib.pbkdf2_hmac('sha256', password.encode(), bytes.fromhex(salt), 600000).hex()
            Path(credential_file).write_text('Usuario: admin\nSenha: ' + password + '\n', encoding='utf-8')
            db.execute('INSERT INTO users (nome,sigla,salt,hash,created_at,role) VALUES (?,?,?,?,?,?)',
                       ('admin', '', salt, digest, int(time.time()), 'admin'))

    @contextmanager
    def connect(self):
        db = sqlite3.connect(self.path, timeout=10)
        try:
            db.execute('PRAGMA foreign_keys=ON')
            with db:
                yield db
        finally:
            db.close()

    def dispatch(self, method, path, body, authorization):
        now = int(time.time())
        with self.connect() as db:
            if method == 'POST' and path in ('/api/auth/register', '/api/auth/login'):
                name = text(body, 'nome')
                if len(name.encode('utf-8')) > 63:
                    raise ApiError(400, 'Nome ou sigla deve ocupar ate 63 bytes UTF-8')
                password = body.get('senha')
                if not isinstance(password, str) or not 1 <= len(password) <= 128:
                    raise ApiError(400, 'Senha deve ter entre 1 e 128 caracteres')
                if path.endswith('register'):
                    alias = body.get('sigla', '')
                    if not isinstance(alias, str) or len(alias) > 32:
                        raise ApiError(400, 'Sigla invalida')
                    salt = secrets.token_hex(16)
                    digest = hashlib.pbkdf2_hmac('sha256', password.encode(), bytes.fromhex(salt), 600000).hex()
                    try:
                        if db.execute('SELECT 1 FROM users WHERE (nome=? OR sigla=?) OR (? != ? AND (nome=? OR sigla=?))',
                                      (name, name, alias.strip(), '', alias.strip(), alias.strip())).fetchone():
                            raise ApiError(409, 'Nome ou sigla ja cadastrado')
                        db.execute('INSERT INTO users (nome,sigla,salt,hash,created_at) VALUES (?,?,?,?,?)', (name, alias.strip(), salt, digest, now))
                    except sqlite3.IntegrityError:
                        raise ApiError(409, 'Nome ou sigla ja cadastrado')
                else:
                    row = db.execute('SELECT nome,salt,hash FROM users WHERE nome=? OR (sigla=? AND sigla != ?)', (name, name, '')).fetchall()
                    if len(row) != 1:
                        raise ApiError(401, 'Credenciais invalidas')
                    name, salt, expected = row[0]
                    digest = hashlib.pbkdf2_hmac('sha256', password.encode(), bytes.fromhex(salt), 600000).hex()
                    if not hmac.compare_digest(digest, expected):
                        raise ApiError(401, 'Credenciais invalidas')
                token = secrets.token_hex(32)
                db.execute('DELETE FROM sessions WHERE expires_at<=?', (now,))
                db.execute('INSERT INTO sessions VALUES (?,?,?)', (hashlib.sha256(token.encode()).hexdigest(), name, now + 43200))
                role, alias = db.execute('SELECT role,sigla FROM users WHERE nome=?', (name,)).fetchone()
                return 200, {'ok': True, 'nome': name, 'usuario': {'nome': name, 'sigla': alias, 'role': role}, 'token': token, 'expires_at': now + 43200}
            if path in ('/api/auth/me', '/api/auth/logout') and method == ('GET' if path.endswith('me') else 'POST'):
                token = authorization.removeprefix('Bearer ')
                digest = hashlib.sha256(token.encode()).hexdigest()
                row = db.execute('SELECT s.nome,u.role,u.sigla FROM sessions s JOIN users u ON u.nome=s.nome WHERE token_hash=? AND expires_at>?', (digest, now)).fetchone()
                if row is None:
                    raise ApiError(401, 'Sessao invalida ou expirada')
                if path.endswith('logout'):
                    db.execute('DELETE FROM sessions WHERE token_hash=?', (digest,))
                return 200, {'ok': True, 'nome': row[0], 'usuario': {'nome': row[0], 'role': row[1], 'sigla': row[2]}}
            if path == '/api/estoque' and method == 'PUT':
                items = body.get('itens')
                if not isinstance(items, list) or len(items) != 6:
                    raise ApiError(400, 'Envie os seis slots em itens')
                rows = []
                for item in items:
                    if not isinstance(item, dict):
                        raise ApiError(400, 'Item invalido')
                    slot = integer(item.get('slot'), 1, 6)
                    capacity = integer(item.get('capacidade'), 1, 10000)
                    quantity = integer(item.get('quantidade'), 0, capacity)
                    rows.append((slot, FLAVORS[slot-1], quantity, capacity, now))
                if len({row[0] for row in rows}) != 6:
                    raise ApiError(400, 'Slots repetidos')
                db.executemany('INSERT OR REPLACE INTO inventory VALUES (?,?,?,?,?)', rows)
                return 200, {'ok': True}
            if path == '/api/estoque' and method == 'GET':
                rows = db.execute('SELECT slot,sabor,quantidade,capacidade,updated_at FROM inventory ORDER BY slot').fetchall()
                return 200, {'itens': [dict(zip(('slot','sabor','quantidade','capacidade','updated_at'), row)) for row in rows], 'origem': 'ultima_copia_esp32'}
            if path == '/api/vendas' and method == 'POST':
                event_id = text(body, 'evento_id', 128)
                name = text(body, 'usuario')
                quantities = [integer(body.get('q'+str(i)), 0, 3) for i in range(1, 7)]
                total = integer(body.get('total'), 1, 3)
                if sum(quantities) != total or body.get('status') != 'entregue':
                    raise ApiError(400, 'Registre somente vendas entregues com total consistente')
                payload = json.dumps(dict(evento_id=event_id, usuario=name, total=total, status='entregue', **{'q'+str(i+1): q for i,q in enumerate(quantities)}), sort_keys=True, ensure_ascii=False)
                existing = db.execute('SELECT payload FROM sales WHERE event_id=?', (event_id,)).fetchone()
                if existing and existing[0] != payload:
                    raise ApiError(409, 'evento_id ja usado com outros dados')
                db.execute('INSERT OR IGNORE INTO sales VALUES (?,?,?)', (event_id, payload, now))
                return 200, {'ok': True, 'duplicada': existing is not None}
            if path == '/api/vendas' and method == 'GET':
                digest = hashlib.sha256(authorization.removeprefix('Bearer ').encode()).hexdigest()
                role = db.execute('SELECT u.role FROM sessions s JOIN users u ON u.nome=s.nome WHERE token_hash=? AND expires_at>?', (digest, now)).fetchone()
                if role is None:
                    raise ApiError(401, 'Sessao invalida ou expirada')
                if role[0] != 'admin':
                    raise ApiError(403, 'Acesso administrativo necessario')
                rows = db.execute('SELECT payload,created_at FROM sales ORDER BY created_at DESC,event_id DESC LIMIT 30').fetchall()
                return 200, {'vendas': [dict(json.loads(payload), registrado_em=created) for payload,created in rows], 'limite': 30}
        raise ApiError(404, 'Rota nao encontrada')


class Handler(BaseHTTPRequestHandler):
    server_version = 'CookieExpress/1'

    def setup(self):
        super().setup()
        self.connection.settimeout(10)

    def respond(self, status, data):
        raw = json.dumps(data, ensure_ascii=False).encode('utf-8')
        self.respond_bytes(status, raw, 'application/json; charset=utf-8')

    def respond_bytes(self, status, raw, content_type):
        self.send_response(status)
        self.send_header('Content-Type', content_type)
        self.send_header('Content-Length', str(len(raw)))
        self.send_header('Cache-Control', 'no-store')
        self.send_header('X-Content-Type-Options', 'nosniff')
        self.end_headers()
        self.wfile.write(raw)

    def handle_api(self):
        try:
            path = urlsplit(self.path).path
            if self.command == 'GET' and path in PUBLIC_PAGES:
                return self.respond_bytes(200, PUBLIC_PAGES[path].read_bytes(), 'text/html; charset=utf-8')
            if self.command == 'GET' and path == '/health':
                with self.server.database.connect() as db:
                    db.execute('SELECT 1')
                return self.respond(200, {'ok': True, 'servico': 'cookie-express-db', 'schema': 1})
            key = self.headers.get('X-ESP-Key', '')
            if not hmac.compare_digest(key.encode(), self.server.machine_key.encode()):
                raise ApiError(401, 'Chave do ESP32 invalida')
            body = {}
            if self.command in ('POST', 'PUT'):
                if self.headers.get('Transfer-Encoding'):
                    raise ApiError(400, 'Use Content-Length, sem Transfer-Encoding')
                try:
                    length = int(self.headers.get('Content-Length', '-1'))
                except ValueError:
                    raise ApiError(400, 'Content-Length invalido')
                if not 0 <= length <= 16384:
                    raise ApiError(413, 'Corpo ausente ou maior que 16 KiB')
                if self.headers.get_content_type() != 'application/json':
                    raise ApiError(415, 'Use application/json')
                raw = self.rfile.read(length)
                try:
                    body = json.loads(raw)
                except (ValueError, UnicodeDecodeError):
                    raise ApiError(400, 'JSON invalido')
                if not isinstance(body, dict):
                    raise ApiError(400, 'Envie um objeto JSON')
            status, data = self.server.database.dispatch(self.command, path, body, self.headers.get('Authorization', ''))
            self.respond(status, data)
        except ApiError as error:
            self.respond(error.status, {'ok': False, 'erro': error.message})
        except (TimeoutError, ConnectionError):
            self.close_connection = True
        except Exception:
            logging.exception('Falha interna na API')
            self.respond(500, {'ok': False, 'erro': 'Falha interna no servidor'})

    do_GET = do_POST = do_PUT = handle_api


def create_server(host, port, database, machine_key):
    server = ThreadingHTTPServer((host, port), Handler)
    server.database = Database(database)
    server.machine_key = machine_key
    server.daemon_threads = True
    return server


def discovery_loop(port, machine_key, stop, ready=None, discovery_port=8081):
    """O ESP descobre o IP do PC sem depender do endereco atribuido pelo DHCP."""
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.bind(('0.0.0.0', discovery_port))
        sock.settimeout(1)
        if ready is not None:
            ready.put(sock.getsockname()[1])
        while not stop.is_set():
            try:
                raw, address = sock.recvfrom(128)
                request = raw.decode('ascii')
                if not request.startswith('CE1:'):
                    continue
                nonce = request[4:]
                if len(nonce) != 32 or any(c not in '0123456789abcdef' for c in nonce):
                    continue
                message = f'{nonce}:{port}'
                signature = hmac.new(machine_key.encode(), message.encode(), hashlib.sha256).hexdigest()
                sock.sendto(f'CE1:{message}:{signature}'.encode(), address)
            except socket.timeout:
                pass
            except (UnicodeError, OSError):
                logging.exception('Falha na descoberta do ESP')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host', default='0.0.0.0')
    parser.add_argument('--port', type=int, default=8080)
    parser.add_argument('--data-dir', type=Path, default=ROOT / 'data')
    args = parser.parse_args()
    args.data_dir.mkdir(parents=True, exist_ok=True)
    key_file = args.data_dir / 'esp.key'
    try:
        with key_file.open('x', encoding='ascii') as file:
            file.write(secrets.token_hex(32))
    except FileExistsError:
        pass
    key = key_file.read_text(encoding='ascii').strip()
    if len(key) < 32:
        parser.error('Chave em esp.key deve ter pelo menos 32 caracteres')
    server = create_server(args.host, args.port, args.data_dir / 'cookie_express.sqlite3', key)
    server.database.bootstrap_admin(args.data_dir / 'admin-inicial.txt')
    stop = threading.Event()
    discovery = threading.Thread(target=discovery_loop, args=(args.port, key, stop), daemon=True)
    discovery.start()
    print(f'Cookie Express: http://{args.host}:{args.port}/health', flush=True)
    print(f'IHM neste PC: http://127.0.0.1:{args.port}/ | Backoffice: http://127.0.0.1:{args.port}/backoffice', flush=True)
    print('Em outros aparelhos, use o IPv4 deste PC na rede COOKIE_EXPRESS no lugar de 127.0.0.1.', flush=True)
    print(f'Banco: {args.data_dir.resolve()}; chave do ESP32: {key_file.resolve()}', flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        stop.set()
        server.server_close()


if __name__ == '__main__':
    main()
