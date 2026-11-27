#!/usr/bin/env python3
"""P2-AN synthetic GVR texture-archive fixture generator (host/bringup lane).

Emits two fake_disc files, byte-for-byte deterministically:

  sinbd_tex.gvr  0x41C bytes — a GVR image as load_sin_board_image receives it
               (era path: qst_op_call_image_data :801EB5EC -> decoded bytes ->
               load_texture_from_memory -> find_gvrt_texture_chunk).
               Byte regimes (docs/p2-tex-supply.md, per compat/README-byteswap.md
               §2 + docs/p1-format-net-disc.md D2/D3/D4):
                 GBIX magic  'GBIX' ASCII  -> psoProt  (era swaps it: 0x803D9CE4)
                 GBIX len    u32 LE=4      -> psoProt  (era swaps it: 0x803D9D34)
                 GBIX id     u32 BE=0x999  -> psoBe    (era raw lwz, UNSWAPPED:
                                                        0x803D9D40 `lwz r0,0(r29)`)
                 GVRT magic  'GVRT' ASCII  -> psoProt  (same seam as GBIX magic)
                 GVRT size   u32 LE=0x408  -> psoProt  (GVM chunk walk consumes
                                                        `chunk->size - 8` at
                                                        text_803DD5E0.cpp:442)
                 GVRT desc   format/width/height BE   -> psoBe (era move_data
                                                        copies 8 raw bytes:
                                                        0x803D9D0C-:9D18, no swap)
                 texels      1024 B, byte-true        -> D4 payload, never widen

  gsl_tab2.dat   4 x GslArchiveEntry (0x30 stride) — AE's 3 rows from
               gsl_tab.dat VERBATIM (entries 0..2) + the GVR row. Multi-byte
               fields carry the same LE-image working values AE's ladder3
               overlays host-order (bringup rung regime, DA-4 host fill); the
               D1 true-disc psoBe pass is documented, not re-litigated here.

DO NOT EDIT THE OUTPUT BY HAND; run:  python3 gen_gvr_fixture.py
"""
import struct, hashlib, os, pathlib

HERE = pathlib.Path(__file__).resolve().parent
PX_W, PX_H = 32, 32                      # desc.width/height working values
FMT_PAL8 = 0x09                          # is_palette_texture_format 8/9/10
GLOBAL_ID = 0x00000999                   # GBIX surface id working value
N_PX = PX_W * PX_H                       # PAL8: 1 byte per texel (assumption AN-4)
GVRT_SIZE = N_PX + 8                     # `size - 8` == texel bytes (:442 rule)

def texel(x: int, y: int) -> int:
    return ((x * 3 + y * 5) ^ ((x ^ y) << 2)) & 0xFF

def fnv1a64(b: bytes) -> int:
    h = 0xCBF29CE484222325
    for c in b:
        h = ((h ^ c) * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return h

px = bytes(texel(x % PX_W, x // PX_W) for x in range(N_PX))
gvr = b"".join([
    b"GBIX",                    # +0x00 magic, psoProt image (DC-legacy ASCII)
    struct.pack("<I", 4),       # +0x04 len,   psoProt (skip = 12B chunk total)
    struct.pack(">I", GLOBAL_ID),   # +0x08 id, psoBe (era raw load)
    b"GVRT",                    # +0x0C magic, psoProt
    struct.pack("<I", GVRT_SIZE),   # +0x10 size, psoProt
    struct.pack(">I", FMT_PAL8),    # +0x14 desc.format, psoBe
    struct.pack(">H", PX_W),        # +0x18 desc.width,  psoBe
    struct.pack(">H", PX_H),        # +0x1A desc.height, psoBe
    px,                         # +0x1C texels, byte-true (D4)
])
assert len(gvr) == 0x1C + N_PX == 0x41C, len(gvr)

# ---- AE's three existing rows verbatim (gsl_tab.dat), + GVR row -----------
tab_src = HERE / "gsl_tab.dat"
ae_rows = tab_src.read_bytes() if tab_src.exists() else b""
assert len(ae_rows) == 3 * 0x30, "gsl_tab.dat must be 3 entries"
def entry(name: str, off: int, size: int, dec: int) -> bytes:
    return name.encode().ljust(0x20, b"\0") + struct.pack("<III", off, size, dec) + b"\0" * 4
gvr_row = entry("sinbd_tex.gvr", 0, len(gvr), len(gvr))   # GVR stored raw (PRS chain proven by AE)
tab = ae_rows + gvr_row

(HERE / "sinbd_tex.gvr").write_bytes(gvr)
(HERE / "gsl_tab2.dat").write_bytes(tab)
print(f"sinbd_tex.gvr: {len(gvr)} bytes  fnv1a64={fnv1a64(gvr):016x}  texels-fnv={fnv1a64(px):016x}")
print(f"gsl_tab2.dat:  {len(tab)} bytes ({len(tab)//0x30} entries) fnv1a64={fnv1a64(tab):016x}")
