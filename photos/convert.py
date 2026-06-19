#!/usr/bin/env python3
"""
Convert photos to 200x200 1-bit raw bitmaps for ESP32 LittleFS photo viewer.
Drop any JPG/PNG into photos/ and run this script.
Output .bin files go into firmware/data/ (the LittleFS data folder).
"""
from PIL import Image
from pathlib import Path
import sys

INPUT_DIR = Path(__file__).parent / "input"
OUTPUT_DIR = Path(__file__).parent.parent / "firmware" / "data"
OUTPUT_DIR.mkdir(exist_ok=True)

EXTENSIONS = {".jpg", ".jpeg", ".png", ".bmp", ".webp"}

sources = [f for f in INPUT_DIR.iterdir() if f.suffix.lower() in EXTENSIONS]
if not sources:
    print("No images found in photos/input/. Drop JPG/PNG files there and re-run.")
    sys.exit(0)

for i, src in enumerate(sorted(sources)):
    out = OUTPUT_DIR / f"p{i:02d}.bin"
    img = Image.open(src)
    w, h = img.size
    crop = min(w, h)
    img = img.crop(((w - crop) // 2, (h - crop) // 2,
                    (w + crop) // 2, (h + crop) // 2))
    img = img.resize((200, 200), Image.LANCZOS)
    img = img.convert("1", dither=Image.FLOYDSTEINBERG)

    pixels = list(img.getdata())
    raw = bytearray(5000)
    for y in range(200):
        for bx in range(25):
            byte = 0
            for bit in range(8):
                x = bx * 8 + bit
                if x < 200 and pixels[y * 200 + x] != 0:
                    byte |= (1 << (7 - bit))
            raw[y * 25 + bx] = byte

    out.write_bytes(raw)
    print(f"  {src.name} → {out.name} ({len(raw)} bytes)")

print(f"\nDone. {len(sources)} photo(s) in firmware/data/")
print("Run upload.sh to push to device.")
