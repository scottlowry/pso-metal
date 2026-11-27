#!/usr/bin/env python3
"""P2-HG2 static-tree census generator (builder-lane-owned; docs/p2-static-init-1.md
§3 — "emitted/checked by a build-time census generator").

Reads the FIVE era TUs in the decomp checkout READ-ONLY (never writes them) and
emits $(GEN)/hg2_static_tree_blob.cpp defining the 82x32 payload blob the
W-STATIC-TREE window stages at arena+0x80000, plus the per-cell expected
name/slot/length/site vectors the HG-2 golden compares against. NOTHING is
hand-typed: the 81 census strings are regex-matched at their exact
docs/p2-static-init-1.md §2 file:line rows (81/81 required, loud fail on any
drift), and the rider row (NinjaUtil.cpp:7 texlist_null_name[0x20]) is verified
to be the era zero-initialized char array it is documented as.

Slot law (STA1 §1/§4): slot VALUES are pure contract arithmetic
(0x80080000u + 32u*k) — this file never emits, and the header never allows, a
host-address term. Exit non-zero + no output on any census drift.
"""
import argparse, hashlib, os, re, sys
from typing import NoReturn

GUEST_BEGIN = 0x80080000
CELL = 32
CELLS = 82  # 81 census sites + rider cell 81 (texlist_null_name)

# (kind, relpath, line_or_decl_rx, count, entry/full-line rx, class) per STA1 §2.
# Arrays self-anchor on their declaration line (the live file sits one line off
# the STA1 doc citation for Volopt — strings/order are what the census pins).
CENSUS = [
    ("array", "src/pso/render/CameraCapture.cpp",
     re.compile(r'^static NJS_TEXNAME effect_textures\[54\] = \{$'), 54,
     re.compile(r'^\s*\{"(?P<n>[A-Za-z0-9_]+)", 0, NULL\},$'), "A"),
    ("array", "src/pso/boss/volopt/TBoss3VoloptMonitor.cpp",
     re.compile(r'^static NJS_TEXNAME volopt_monitor_texture_entries\[25\] = \{$'), 25,
     re.compile(r'^\s*\{ "(?P<n>[A-Za-z0-9_]+)" \},$'), "B"),
    ("single", "src/pso/ui/frontend/TAdMcSelectCursorGC.cpp", 20, 1,
     re.compile(r'^NJS_TEXNAME mc_select_cursor_texname = \{ "(?P<n>[A-Za-z0-9_]+)", 0, NULL \};$'), "C"),
    ("single", "src/pso/ui/frontend/TAdMcSelectMcGC.cpp", 14, 1,
     re.compile(r'^NJS_TEXNAME mc_select_memorycard_texname = \{ "(?P<n>[A-Za-z0-9_]+)", 0, NULL \};$'), "C"),
]
RIDER = ("src/pso/render/NinjaUtil.cpp", 7,
         re.compile(r'^static char texlist_null_name\[0x20\];$'))
# Same-turn cross-check anchors vs docs/p2-static-init-1.md §2 (first/last per
# block + the two singles): catches whole-table-order drift, not just shape.
STA1_ANCHORS = {
    0:  ("CameraCapture.cpp", "f008_white_npr01"),
    53: ("CameraCapture.cpp", "u064_glasshahen_npr01"),
    54: ("TBoss3VoloptMonitor.cpp", "ki_u256_vo_mo_moji001"),
    78: ("TBoss3VoloptMonitor.cpp", "ki_u064_vo_mo"),
    79: ("TAdMcSelectCursorGC.cpp", "f256_memorycard_select"),
    80: ("TAdMcSelectMcGC.cpp", "f256_memorycard"),
}


def fail(msg) -> "NoReturn":  # noqa: F821 — exits, never returns
    sys.stderr.write("HG2-CENSUS GUARD: %s\n" % msg)
    sys.exit(1)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--decomp", default=os.environ.get(
        "DECOMP_ROOT", os.path.expanduser("~/Hermes/pso-decomp/pso-decomp")))
    ap.add_argument("--out", required=True)
    a = ap.parse_args()

    def read_lines(rel):
        p = os.path.join(a.decomp, rel)
        if not os.path.isfile(p):
            fail("missing era source %s" % rel)
        with open(p, "r", encoding="utf-8") as f:
            return f.read().splitlines()

    rows = []  # (index, relpath, line1, name, cls)
    for kind, rel, anchor, count, rx, cls in CENSUS:
        lines = read_lines(rel)
        if kind == "array":
            hits = [i for i, L in enumerate(lines) if anchor.match(L)]
            if len(hits) != 1:
                fail("%s: array declaration matched %d times (want exactly 1)"
                     % (rel, len(hits)))
            line1 = hits[0] + 2          # first entry line (1-based)
        else:
            line1 = anchor
        for i in range(count):
            ln = line1 + i
            if ln - 1 >= len(lines):
                fail("%s:%d past EOF" % (rel, ln))
            m = rx.match(lines[ln - 1])
            if not m:
                fail("%s:%d does not match census shape %r: %r"
                     % (rel, ln, rx.pattern, lines[ln - 1]))
            rows.append((len(rows), rel, ln, m.group("n"), cls))
    idx = len(rows)
    if idx != 81:
        fail("census produced %d rows, expected 81" % idx)

    names = [r[3] for r in rows]
    if len(set(names)) != 81:
        fail("census names not all distinct (STA1 §2 set math)")
    for k, (rel, n) in STA1_ANCHORS.items():
        if rows[k][3] != n or not rows[k][1].endswith(rel):
            fail("index %d drifted from STA1 §2 anchor %s %r (got %r)"
                 % (k, rel, n, rows[k][1:4]))
    if max(len(n) + 1 for n in names) > CELL:
        fail("payload exceeds %d-byte cell" % CELL)

    rlines = read_lines(RIDER[0])
    if not RIDER[2].match(rlines[RIDER[1] - 1]):
        fail("rider %s:%d is not the era 'static char texlist_null_name[0x20];'"
             % (RIDER[0], RIDER[1]))

    # byte truth + shas (receipts; era files READ-ONLY)
    shas = {}
    for rel in [c[1] for c in CENSUS] + [RIDER[0]]:
        with open(os.path.join(a.decomp, rel), "rb") as f:
            shas[rel] = hashlib.sha256(f.read()).hexdigest()

    o = []
    o.append("/* GENERATED by host/bringup/gen_hg2_static_tree.py — DO NOT HAND-EDIT.")
    o.append(" * P2-HG2 W-STATIC-TREE payload blob (82 cells x 32 B: rows 0..80 = the")
    o.append(" * STA1 §2 81-site census NUL-padded; row 81 = texlist_null_name cell,")
    o.append(" * 32 zero bytes, era NinjaUtil.cpp:7 'static char texlist_null_name[0x20]').")
    for rel in [c[1] for c in CENSUS] + [RIDER[0]]:
        o.append(" * era sha256 %s = %s" % (rel, shas[rel]))
    o.append(" */")
    o.append('#include "hg2_static_tree.h"')
    o.append("")
    o.append("const u8 pso_st_blob[PSO_ST_CELLS][PSO_ST_CELL] = {")
    for r in rows:
        b = r[3].encode("ascii") + b"\x00"
        b += b"\x00" * (CELL - len(b))
        o.append("    { %s }, /* %2d %s:%d \"%s\" */" % (
            ", ".join("0x%02X" % x for x in b), r[0],
            os.path.basename(r[1]), r[2], r[3]))
    o.append("    { %s }, /* 81 rider: texlist_null_name (NinjaUtil.cpp:7) */"
             % ", ".join(["0x00"] * CELL))
    o.append("};")
    o.append("")
    o.append("const char *const pso_st_name[PSO_ST_CELLS] = {")
    for r in rows:
        o.append('    "%s", /* %2d %s */' % (r[3], r[0], r[4]))
    o.append("    0,      /* 81 rider cell — content law = 32 zero bytes        */")
    o.append("};")
    o.append("")
    o.append("const u32 pso_st_slot[PSO_ST_CELLS] = {")
    for k in range(CELLS):
        o.append("    0x%08Xu, /* %2d */" % (GUEST_BEGIN + CELL * k, k))
    o.append("};")
    o.append("")
    o.append("const u32 pso_st_cell_len[PSO_ST_CELLS] = {")
    for r in rows:
        o.append("    %2du,   /* %2d strlen+1 */" % (len(r[3]) + 1, r[0]))
    o.append("    0u,      /* 81 rider */")
    o.append("};")
    o.append("")
    o.append("const char *const pso_st_site[PSO_ST_CELLS] = {")
    for r in rows:
        o.append('    "%s:%d", /* %2d */' % (os.path.basename(r[1]), r[2], r[0]))
    o.append('    "NinjaUtil.cpp:7", /* 81 rider */')
    o.append("};")
    o.append("")
    o.append("static_assert(PSO_ST_CELLS == 82u && PSO_ST_CELL == 32u"
             " && sizeof(pso_st_blob) == 82u * 32u && 82u * 32u <= 0x1000u,"
             " \"STA1 §2/§3: padded blob 2624 B must fit the 4 KB window\");")
    o.append("")

    tmp = a.out + ".tmp"
    with open(tmp, "w", encoding="utf-8") as f:
        f.write("\n".join(o))
    os.replace(tmp, a.out)
    print("HG2-CENSUS: 81/81 sites + rider matched at %s (max payload %d B, all distinct)"
          % (a.decomp, max(len(n) + 1 for n in names)))


if __name__ == "__main__":
    main()
