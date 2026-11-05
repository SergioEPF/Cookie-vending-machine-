"""Le inicializacao sem pedidos ou comandos de motor. Requer Python do ESP-IDF."""
import argparse
import time
from pathlib import Path
import serial

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--port', default='COM4')
parser.add_argument('--seconds', type=int, default=12)
parser.add_argument('--reset', action='store_true')
args = parser.parse_args()
device = serial.Serial(port=None, baudrate=115200, timeout=0.3)
device.port = args.port
device.dtr = False
device.rts = False
device.open()
with device as port:
    if args.reset:
        port.dtr = False
        port.rts = True
        time.sleep(0.15)
        port.rts = False
    end = time.monotonic() + args.seconds
    output = bytearray()
    while time.monotonic() < end:
        output.extend(port.read(port.in_waiting or 1))
    text = output.decode('utf-8', errors='replace')
    log = Path(__file__).resolve().parents[1] / '.runtime/diagnosticos/esp-boot.log'
    log.parent.mkdir(parents=True, exist_ok=True)
    log.write_text(text, encoding='utf-8')
    print(text)
