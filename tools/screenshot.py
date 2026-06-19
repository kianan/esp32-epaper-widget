#!/usr/bin/env python3
"""
Capture a screenshot from the ESP32 ePaper display.

Usage:
    python3 screenshot.py                    # auto-detect port
    python3 screenshot.py /dev/cu.usbmodem* # specify port
"""
import sys, serial, serial.tools.list_ports
from PIL import Image
from pathlib import Path
from datetime import datetime

PORT  = sys.argv[1] if len(sys.argv) > 1 else None
BAUD  = 115200
W, H  = 200, 200

def find_port():
    ports = [p.device for p in serial.tools.list_ports.comports()
             if 'usbmodem' in p.device or 'usbserial' in p.device or 'SLAB' in p.device]
    if not ports:
        raise RuntimeError("No ESP32 port found. Connect device or pass port as argument.")
    return ports[0]

port = PORT or find_port()
print(f"Connecting to {port}...")

with serial.Serial(port, BAUD, timeout=5) as ser:
    ser.write(b"SCREENSHOT\n")
    ser.flush()
    print("Waiting for dump...")

    # Wait for dump start
    while True:
        line = ser.readline().decode(errors='replace').strip()
        if line == 'EPDDUMP_START':
            print("Receiving buffer...")
            break
        if not line:
            raise RuntimeError("Timed out waiting for EPDDUMP_START. Is the firmware flashed?")

    # Read hex rows until end marker
    hex_data = ''
    while True:
        line = ser.readline().decode(errors='replace').strip()
        if line == 'EPDDUMP_END':
            break
        if not line:
            raise RuntimeError("Timed out mid-dump.")
        hex_data += line

raw = bytes.fromhex(hex_data)
assert len(raw) == 5000, f"Expected 5000 bytes, got {len(raw)}"

img = Image.new('RGB', (W, H), 'white')
pixels = img.load()
for y in range(H):
    for bx in range(25):
        byte = raw[y * 25 + bx]
        for bit in range(8):
            x = bx * 8 + bit
            white = (byte >> (7 - bit)) & 1
            pixels[x, y] = (255, 255, 255) if white else (0, 0, 0)

img = img.resize((W * 3, H * 3), Image.NEAREST)

out_dir = Path(__file__).parent / "screenshots"
out_dir.mkdir(exist_ok=True)
out = out_dir / f"screenshot_{datetime.now().strftime('%Y%m%d_%H%M%S')}.png"
img.save(out)
print(f"Saved: {out}")
