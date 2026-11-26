#!/usr/bin/env python3
"""pso-metal Phase-0 task F: big-endian hazard scanner.

Heuristically flags big-endian-dependent patterns in the pso-decomp source
tree (src/ + include/) so the Metal/arm64 port can prioritize byte-order work
by data instead of guesses. Stdlib only; reproducible from the repo root:

    python3 tools/be_scan.py [--root DECOMP_ROOT] [--out data/be-hazard-index.csv]

Categories (per-file hit counts):
  cat1  C bitfields (`name : width;`) -- MSB/LSB packing differs PPC vs arm64.
  cat2  byte assembly/splitting -- `(u8*)`/`(char*)` casts and `<< 8|16|24` /
        `>> 8|16|24` shifts used to stitch words from bytes.
  cat3  explicit byte-swap machinery already present (byte_swap/bswap/swpb/
        ntohl/htonl/lwbrx-style names) -- endianness ALREADY handled here;
        lowers the file's risk score.
  cat4  multi-byte member reads through byte-offset pointer math
        (`*(s32 *)((s8*)e + 0x20)`, `buf[i] >> 16`, byte-cast + offset adds) --
        layout-dependent reinterpretation.
  cat5  unions whose members have differing integer/float widths --
        type-punning over multi-byte scalars; byte order decides what you see.

Risk formula (stated, integer, floor 0):
    risk_score = 4*cat1 + 2*cat2 + 3*cat4 + 5*cat5 - 2*cat3

Weight rationale: cat5 is the most dangerous (silently wrong reads), cat4 is
precise evidence of offset reinterpretation, cat1 is the classic PPC-vs-ARM
bitfield packing bug, cat2 is a noisy candidate pool (some hits are fixed-point
math, not endianness), cat3 earns credit for endianness awareness.

Only files with >= 1 hazard hit are written to the CSV. Comments are stripped
before matching so commented-out decomp dumps don't inflate counts.
"""

import argparse
import csv
import os
import re
import sys
from collections import Counter

# ---------------------------------------------------------------- discovery

EXTS = {".c", ".h", ".cc", ".cpp", ".cxx", ".hpp", ".hh"}
SKIP_DIRS = {".git", "build", "obj", "third_party", "orig", "cache"}

ROOT_FALLBACKS = [
    os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                 "pso-decomp", "pso-decomp"),  # sibling checkout
    "/Users/scott/Hermes/pso-decomp/pso-decomp",
]

# ---------------------------------------------------------------- regexes

# cat1: bitfield decls. Require an identifier before the colon, an integer
# literal after, and forbid ternary/`::`/`=` on the line to dodge labels and
# `a ? b : 0;` idioms (a very common decompiler artifact).
RE_BITFIELD = re.compile(
    r"^[ \t]*(?:(?:unsigned|signed|const)\s+)*[A-Za-z_]\w*(?:\s*\*+)?\s+"
    r"[A-Za-z_]\w*\s*:\s*(?:\d+|0[xX][0-9a-fA-F]+)\s*;"
)

# cat2: byte-cast and wide-shift occurrences (counted per occurrence).
RE_BYTE_CAST = re.compile(r"\(\s*(?:u8|s8|uint8_t|int8_t|char)\s*\*\s*\)")
RE_SHIFT = re.compile(r"(?:<<|>>)\s*(?:8|16|24|0[xX]08|0[xX]10|0[xX]18)\b")

# cat3: already endianness-aware. Byteswap-specific terms only -- bare "swap"
# is slot/item swapping in PSO UI code and would be pure noise.
RE_SWAP = re.compile(
    r"\b(?:byte_?swap|byteswap|bswap(?:_\d+)?|swpb|swplw|lw?brx|st?h?brx|"
    r"lwbrx|lbzux.*br|ntoh[ls]|ntoh[ls]|hto[nl]|swap_bytes?)\b",
    re.IGNORECASE,
)

# cat4 (per-line, one hit max per line):
#   a) wide-type pointer deref cast:  *(s32 *)(...   *(u32)*p
RE_WIDE_DEREF = re.compile(r"\*\s*\(\s*[us](?:16|32|64)\s*\*\s*\)\s*[\w(&*]")
#   b) array byte-offset read combined with a split shift:  para[0] >> 16
RE_INDEX_SHIFT = re.compile(r"\[[^\]]+\]\s*(?:>>|<<)\s*\d")
RE_SHIFT_INDEX = re.compile(r"(?:>>|<<)\s*\d.*\[[^\]]+\]")
#   c) byte-cast pointer arithmetic with an offset add:  (u8*)this + i*sizeof
RE_BYTEPTR_MATH = re.compile(
    r"\(\s*(?:u8|s8|char)\s*\*\s*\)\s*[\w.>-]+[^;]*\+|\+\s*(?:offsetof|\d+\s*\*\s*sizeof)"
)

# cat5: union header and member types.
UNION_WIDTH = {
    "char": 1, "s8": 1, "u8": 1, "uint8_t": 1, "int8_t": 1, "signedchar": 1,
    "short": 2, "u16": 2, "s16": 2, "uint16_t": 2, "int16_t": 2,
    "int": 4, "u32": 4, "s32": 4, "uint32_t": 4, "int32_t": 4, "long": 4,
    "float": 4, "f32": 4,
    "double": 8, "f64": 8, "u64": 8, "s64": 8, "uint64_t": 8, "int64_t": 8,
    "longlong": 8,
}
RE_UNION_OPEN = re.compile(r"\bunion\s+(?:[A-Za-z_]\w*\s*)?\{")
RE_MEMBER_TYPE = re.compile(
    r"^[ \t]*(?:(?:unsigned|signed|const|volatile|struct|enum)\s+)*"
    r"([A-Za-z_]\w*)"          # base type token
)

# ---------------------------------------------------------------- helpers

def strip_comments(text):
    """Blank out // and /* */ comment contents, preserving line structure."""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            seg = text[i:j]
            out.append("".join(ch if ch == "\n" else " " for ch in seg))
            i = j
        elif c in "\"'":  # skip string/char literal
            quote, j = c, i + 1
            while j < n:
                if text[j] == "\\":
                    j += 2
                    continue
                if text[j] == quote:
                    j += 1
                    break
                j += 1
            out.append(text[i:j])
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def scan_union(file_lines, start_idx, start_col):
    """From a `union ... {` opening brace, collect distinct member widths.
    Returns (line_number_1based, hazard: bool)."""
    depth = 0
    widths = set()
    for idx in range(start_idx, len(file_lines)):
        line = file_lines[idx][start_col:] if idx == start_idx else file_lines[idx]
        for ch in line:
            if ch == "{":
                depth += 1
                if depth == 1:
                    continue
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return idx + 1, len(widths) > 1
            elif depth >= 1:
                pass
        if depth >= 1:
            m = RE_MEMBER_TYPE.match(line if idx != start_idx else
                                    re.sub(r"^.*?\{", "", line, count=1))
            if m and not line.lstrip().startswith("//"):
                tok = m.group(1)
                # ignore pointers/references: wide-pointer members are fine
                rest = line[m.end():]
                if "*" in rest.split(";")[0][:3] or rest.lstrip().startswith("*"):
                    continue
                if tok in UNION_WIDTH:
                    widths.add(UNION_WIDTH[tok])
    return start_idx + 1, False


def scan_file(path):
    """Return (cat1..cat5 counts, total, worst_lines list) for one file."""
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            raw = f.read()
    except OSError:
        return None
    text = strip_comments(raw)
    lines = text.splitlines()
    cats = Counter()
    per_line = Counter()

    for lineno, line in enumerate(lines, 1):
        stripped = line.strip()
        if not stripped:
            continue
        hits = 0
        # cat1 bitfields
        if RE_BITFIELD.match(line) and "?" not in line and "=" not in line \
                and "::" not in line:
            n = 1
            cats[1] += n
            hits += n
            per_line[lineno] += n
        # cat2 casts + split shifts (per occurrence)
        n = len(RE_BYTE_CAST.findall(line)) + len(RE_SHIFT.findall(line))
        if n:
            cats[2] += n
            hits += n
            per_line[lineno] += n
        # cat3 byteswap machinery
        n = len(RE_SWAP.findall(line))
        if n:
            cats[3] += n
            hits += n
            per_line[lineno] += n
        # cat4 offset reinterpretation (one hit per line)
        if RE_WIDE_DEREF.search(line) or RE_INDEX_SHIFT.search(line) \
                or RE_SHIFT_INDEX.search(line) or RE_BYTEPTR_MATH.search(line):
            cats[4] += 1
            hits += 1
            per_line[lineno] += 1
        # cat5 union starts
        m = RE_UNION_OPEN.search(line)
        if m:
            ln, hazard = scan_union(lines, lineno - 1, m.end() - 1)
            if hazard:
                cats[5] += 1
                hits += 1
                per_line[ln] += 1

    total = sum(cats.values())
    worst = [ln for ln, _ in per_line.most_common(3)]
    return cats, total, worst


def risk_score(cats):
    return max(0, 4 * cats[1] + 2 * cats[2] + 3 * cats[4] + 5 * cats[5]
               - 2 * cats[3])


def main():
    repo = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    ap = argparse.ArgumentParser(description="BE hazard scanner (pso-metal Phase 0 task F)")
    ap.add_argument("--root", help="decomp repo root (contains src/ and include/)")
    ap.add_argument("--out", default=os.path.join(repo, "data", "be-hazard-index.csv"))
    args = ap.parse_args()

    root = args.root or os.environ.get("PSO_DECOMP_ROOT")
    if not root:
        for cand in ROOT_FALLBACKS:
            if os.path.isdir(os.path.join(cand, "src")):
                root = cand
                break
    if not root or not os.path.isdir(os.path.join(root, "src")):
        sys.exit("error: could not locate decomp root (has src/); pass --root")

    scan_dirs = [os.path.join(root, d) for d in ("src", "include")
                 if os.path.isdir(os.path.join(root, d))]

    rows = []
    scanned = 0
    for d in scan_dirs:
        for dirpath, dirnames, filenames in os.walk(d):
            dirnames[:] = sorted(x for x in dirnames if x not in SKIP_DIRS)
            for name in sorted(filenames):
                if os.path.splitext(name)[1].lower() not in EXTS:
                    continue
                path = os.path.join(dirpath, name)
                scanned += 1
                res = scan_file(path)
                if res is None:
                    continue
                cats, total, worst = res
                if total == 0:
                    continue
                rel = os.path.relpath(path, root)
                rs = risk_score(cats)
                rows.append({
                    "file": rel,
                    "cat1": cats[1], "cat2": cats[2], "cat3": cats[3],
                    "cat4": cats[4], "cat5": cats[5],
                    "total": total, "risk_score": rs,
                    "worst_lines": ";".join(str(w) for w in worst),
                })

    rows.sort(key=lambda r: (-r["risk_score"], -r["total"], r["file"]))

    os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
    with open(args.out, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=["file", "cat1", "cat2", "cat3", "cat4",
                                          "cat5", "total", "risk_score", "worst_lines"])
        w.writeheader()
        w.writerows(rows)

    # summary to stdout
    cat_files = Counter()
    cat_hits = Counter()
    for r in rows:
        for c in range(1, 6):
            if r[f"cat{c}"] > 0:
                cat_files[c] += 1
            cat_hits[c] += r[f"cat{c}"]
    print(f"root:        {root}")
    print(f"scanned:     {scanned} files ({len(rows)} carry >=1 hazard)")
    names = {1: "bitfields", 2: "byte-casts/shifts", 3: "byteswap-present",
             4: "offset-derefs", 5: "mixed-width unions"}
    for c in range(1, 6):
        print(f"  cat{c} {names[c]:<19} files={cat_files[c]:>4}  hits={cat_hits[c]:>6}")
    print(f"out:         {args.out}")
    print("top 5 by risk:")
    for r in rows[:5]:
        print(f"  {r['risk_score']:>4}  {r['file']}  "
              f"(c1={r['cat1']} c2={r['cat2']} c3={r['cat3']} c4={r['cat4']} c5={r['cat5']})")


if __name__ == "__main__":
    main()
