"""Copia a chave local para um header privado, sem exibi-la no terminal."""
from pathlib import Path
import json

root = Path(__file__).resolve().parent.parent
key = (root / 'servidor/data/esp.key').read_text(encoding='ascii').strip()
if len(key) < 32:
    raise SystemExit('Inicie primeiro o servidor para gerar a chave.')
header = root / 'main/backend_secret.h'
content = '#pragma once\n#define BACKEND_KEY ' + json.dumps(key) + '\n'
if not header.exists() or header.read_text(encoding='ascii') != content:
    header.write_text(content, encoding='ascii')
print('Chave local configurada em main/backend_secret.h (nao incluir em backups publicos).')
