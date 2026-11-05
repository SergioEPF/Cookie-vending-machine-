"""Atualiza fontes e diagramas do guia usando listas explicitas de arquivos publicos.

Execute na raiz: & 'C:/Espressif/tools/python/python.exe' servidor/docs/atualizar_guia.py
Não percorre diretórios nem lê servidor/data ou backend_secret.h.
"""
from html import escape
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
GUIDE = ROOT / 'servidor/docs/guia_projeto.html'
SOURCES = (
    'main/main.c', 'main/controller.c', 'main/controller.h',
    'main/maquina_config.h', 'main/sensor_ir.c', 'main/sensor_ir.h',
    'main/sensor_ir_eventos.h',
    'main/modo_teste.c', 'main/modo_teste.h',
    'main/persistencia.c', 'main/persistencia.h',
    'main/acionamento.c', 'main/acionamento.h',
    'main/api.c', 'main/api.h', 'main/backend.c', 'main/backend.h',
    'CMakeLists.txt', 'main/CMakeLists.txt', 'sdkconfig.defaults',
)
DIAGRAMS = ('servidor/docs/diagramas/comunicacao-esp.svg', 'servidor/docs/diagramas/comunicacao-local.svg')


def update():
    """Substitui somente conteúdo entre marcadores conhecidos; mantém o texto do guia."""
    page = GUIDE.read_text(encoding='utf-8')
    for name in SOURCES:
        start, end = f'<!-- SOURCE:{name}:START -->', f'<!-- SOURCE:{name}:END -->'
        if page.count(start) != 1 or page.count(end) != 1:
            raise ValueError('Marcador ausente ou repetido: ' + name)
        source = (ROOT / name).read_text(encoding='utf-8')
        block = start + '\n<pre class="source"><code>' + escape(source) + '</code></pre>\n' + end
        page = re.sub(re.escape(start) + r'.*?' + re.escape(end), lambda _: block, page, flags=re.S)
    for name in DIAGRAMS:
        start, end = f'<!-- DIAGRAM:{name}:START -->', f'<!-- DIAGRAM:{name}:END -->'
        if page.count(start) != 1 or page.count(end) != 1:
            raise ValueError('Marcador de diagrama ausente ou repetido: ' + name)
        svg = (ROOT / name).read_text(encoding='utf-8')
        block = start + '\n' + svg + '\n' + end
        page = re.sub(re.escape(start) + r'.*?' + re.escape(end), lambda _: block, page, flags=re.S)
    GUIDE.write_text(page, encoding='utf-8')
    print(f'Guia atualizado: {len(SOURCES)} fontes e {len(DIAGRAMS)} diagramas (sem chaves privadas).')


if __name__ == '__main__':
    update()
