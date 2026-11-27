#!/usr/bin/env python3
"""P2-PREVIEW ppm2png — pure-stdlib PPM (P6/P3) -> PNG converter.

Converts the drivers' readback dump-hook PPMs (gxh_dl_dump output, e.g.
/tmp/dl_dump.ppm, /tmp/logo_dump.ppm) to viewable PNGs without touching any
third-party image lib. PNG is emitted from scratch: zlib-deflated scanlines
with filter-type-0 bytes, IHDR/IDAT/IEND chunks with CRC32s (zlib.crc32).

usage: ppm2png.py in.ppm out.png
Exit codes: 0 ok, 2 unreadable/bad input, 3 write failure.
"""
import struct
import sys
import zlib


def read_pnm(path):
    """Return (width, height, maxval, rows_of_rgb) for P6 or P3."""
    with open(path, "rb") as f:
        data = f.read()
    pos = 0

    def token():
        nonlocal pos
        while pos < len(data):
            c = data[pos:pos + 1]
            if c.isspace():
                pos += 1
            elif c == b"#":                      # comment to end of line
                nl = data.find(b"\n", pos)
                pos = len(data) if nl < 0 else nl + 1
            else:
                break
        start = pos
        while pos < len(data) and not data[pos:pos + 1].isspace():
            pos += 1
        return data[start:pos]

    magic = token()
    if magic not in (b"P6", b"P3"):
        raise ValueError("not a P6/P3 pixmap: %r" % magic)
    w, h, mv = int(token()), int(token()), int(token())
    if w <= 0 or h <= 0 or mv <= 0 or mv > 65535:
        raise ValueError("bad header %d %d %d" % (w, h, mv))
    if magic == b"P6":
        pos += 1                                 # single whitespace after maxval
        body = data[pos:]
        stride = 2 if mv > 255 else 1
        need = w * h * 3 * stride
        if len(body) < need:
            raise ValueError("truncated raster: %d < %d" % (len(body), need))
        px = [body[i * 3 * stride:(i + 1) * 3 * stride] for i in range(w * h)]
    else:
        nums = [int(t) for t in iter(lambda: token(), b"")][: w * h * 3]
        if len(nums) < w * h * 3:
            raise ValueError("truncated ASCII raster")
        px = [struct.pack("BBB", *(min(255, n) for n in nums[i:i + 3]))
              for i in range(0, w * h * 3, 3)]
    if mv != 255:                                # scale to 8-bit
        px = [bytes((c * 255 + mv // 2) // mv for c in p) for p in px]
    return w, h, px


def write_png(path, w, h, px):
    rows = b"".join(b"\x00" + b"".join(px[r * w:(r + 1) * w]) for r in range(h))

    def chunk(tag, payload):
        return (struct.pack(">I", len(payload)) + tag + payload
                + struct.pack(">I", zlib.crc32(tag + payload) & 0xFFFFFFFF))

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)))
        f.write(chunk(b"IDAT", zlib.compress(rows, 9)))
        f.write(chunk(b"IEND", b""))


def main(argv):
    if len(argv) != 3:
        sys.stderr.write("usage: ppm2png.py in.ppm out.png\n")
        return 2
    try:
        w, h, px = read_pnm(argv[1])
    except (OSError, ValueError) as e:
        sys.stderr.write("ppm2png: %s: %s\n" % (argv[1], e))
        return 2
    try:
        write_png(argv[2], w, h, px)
    except OSError as e:
        sys.stderr.write("ppm2png: %s: %s\n" % (argv[2], e))
        return 3
    print("ppm2png: %dx%d -> %s" % (w, h, argv[2]))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
