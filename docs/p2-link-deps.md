# P2-U — Link-phase dependency census (first-link backing surface)

Measured 2026-11-26 by direct `clang -c` + `nm` over the LP64-parse-pass set of
`pso-decomp @ 6b38b5383` (read-only). Objects built into a scratch dir only —
nothing under `build/` was written. Companion data: [`data/link-deps.tsv`](../data/link-deps.tsv)
(one row per distinct gap: `symbol | source-objects | class | N-scaffold-entry-or-none | traffic-sites | evidence`).

## Headline

| Quantity | Count |
|---|---:|
| LP64-parse-pass files (matches P2-R) | **70** / 1680 |
| Compilable to `.o` at `-target arm64-apple-macos11` | **70** (0 lost between parse and codegen) |
| Raw distinct undefined symbols (`nm -u` union, de-duped) | **603** |
| …defined inside the 70-object set (subset-internal refs) | 10 |
| …backed by macOS libc/libc++/libc++abi (excluded per task) | 23 |
| **Distinct symbols a first link must have backed** | **570** |
| — `in-N-scaffold` (host platform API, declared in `host/*.h`) | **9** |
| — `in-compat-shim` (`compat/*.h` emitted symbols) | **0** |
| — `game-internal-unresolved` (defined in LP64-failing TUs) | **560** |
| — `truly-missing` (**NO backing anywhere in the current link closure**) | **1** — `___upper_map`, see below |

**How many DISTINCT host symbols a first link would need: 9.** All 9 are already in
N's 274-entry scaffold (`host/README.md`); none of the other 265 are touched by this
subset. The compat shim contributes **zero** link symbols — it is `static inline` +
macros by construction, which is exactly why no `__fabs`/`__cntlzw`/`__memcpy`
references leak into the union. The first link's real blocker is not the host surface
but the LP64 assert/waiver pipeline: **560 of 570 gaps** are ordinary game symbols
whose defining TUs currently fail to parse at LP64.

Near-miss, worth a lane decision: `___upper_map` — referenced from
`src/pso/unk/text_803D9FF0.cpp` (a PASS file) via `include/clib/ctype.h`; defined only
at `libs/PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/ctype.c:46`, i.e. inside the
vendored MSL C library, which is **outside** the 1680-unit `src/` probe set and inside
the era `PowerPC_EABI_Support` SDK extract (class: era-SDK gap vs scaffold). The link
recipe (p2-lp64-tranche1 §3.6 "libc/libmusyx sources for LP64") must either adopt MSL
`ctype.c` into the link unit set or shim the `__upper_map`/`__lower_map` tables
(TRIVIAL, 2×256 B). It is the only symbol in the union that fixing all 1610 files
would **not** resolve.

## 9 host symbols needed (all scaffold-covered)

Tree-wide site counts from `docs/host-surface.md` §3 / the `PSOSH_TAG` site args;
`occ₇₀` = identifier occurrences across the 70 pass TUs.

| Symbol | Host header | Effort tag (N) | Tree-wide sites | occ₇₀ | Referenced from |
|---|---|---|---:|---:|---|
| `OSRestoreInterrupts` | psosh_os.h | MODERATE (featured #4) | 33 | 1 | MiddlewareInterruptLock.cpp |
| `OSDisableInterrupts` | psosh_os.h | MODERATE (featured #10) | 28 | 1 | MiddlewareInterruptLock.cpp |
| `GXBegin` | psosh_gx.h | BLOCKING-GPU (featured #12) | 23 | 2 | boss/derolle/ProjectedColorDraw.cpp |
| `GXDrawDone` | psosh_gx.h | BLOCKING-GPU (tag, 4 sites) | 4 | 2 | system/TLanguageFontPalette.cpp |
| `DCStoreRange` | psosh_os.h | TRIVIAL (tag, 6 sites) | 6 | 1 | system/TLanguageFontPalette.cpp |
| `OSAllocFromHeap` / `OSFreeToHeap` / `OSCreateHeap` | psosh_os.h | MODERATE (heap trio) | – | 3/2/1 | system/memory.cpp |
| `DVDChangeDir` | psosh_dvd.h | MODERATE (tag, 2 sites) | 2 | 2 | unk/text_803DEE48.cpp |

Reads as p1-init-ledger §5's critical path in miniature: interrupt-gate + heap spine
(stages 2–3) and the first GX slice (stages 5/8–9) are already what the earliest
parsable TUs ask for; DVD shows up before PAD/CARD/Audio/EXI, consistent with the
recommended Phase-2 build order (host/README.md §"Recommended Phase-2 build order").

## Top 20 gaps by call-site traffic (within the 70-TU subset)

Traffic = identifier occurrences across the pass sources (TSV `traffic-sites`); class
abbrev: GI = game-internal-unresolved.

| # | Symbol | occ₇₀ | #referencing objs | Class |
|--:|---|---:|--:|---|
| 1 | `qst_getargs_void_reset(long)` | 146 | 1 | GI |
| 2 | `qst_getargs_u8(long)` | 133 | 1 | GI |
| 3 | `qst_getargs_void(long)` | 96 | 1 | GI |
| 4 | `qst_getargs_u8_u8(long)` | 63 | 1 | GI |
| 5 | `qst_getargs_u8_s32(long)` | 17 | 1 | GI |
| 6 | `qst_getargs_u16_2(long)` | 16 | 1 | GI |
| 7 | `qst_getargs_u8_u8_u16(long)` | 11 | 1 | GI |
| 8–10 | `TObject::TObject(TObject*)` / `TObject::~TObject()` / `typeinfo for TObject` | 11 ea | 2 | GI |
| 11 | `qst_getargs_u8_s32_u16(long)` | 10 | 1 | GI |
| 12 | `TObject::render()` | 7 | 2 | GI |
| 13 | `bswap_32(unsigned long*)` | 4 | 1 | GI |
| 14 | `char_creation_tag_info_selected_index` | 4 | 2 | GI |
| 15 | **`OSAllocFromHeap`** | 3 | 1 | **in-N-scaffold** |
| 16 | `calc_box_bounds(Vec*,Vec*,Vec*)` | 3 | 2 | GI |
| 17–18 | `qst_getargs_s32` / `qst_getargs_u16` | 3 ea | 1 | GI |
| 19 | `TObject::operator new(unsigned long)` | 3 | 1 | GI |
| 20 | `chat_last_line_buffer` | 3 | 1 | GI |

Two structural facts drop out: (1) **one TU dominates** — `src/pso/quest/QuestOpcodeHandlers.cpp`
alone defines **519 of the 560** game-internal gaps (the quest-VM opcode/getargs chain
referenced by `QuestOpcodeTables.cpp`, a pass file); fixing it first removes 91% of the
whole union. `src/pso/system/task/TObject.cpp` (base task class: ctors/dtor/render/typeinfo/
operator new) is #2 with 4+; (2) **zero host symbols** crack the top 14 — the host-side
demand of today's parseable subset is trivially small, reinforcing that P2's leverage is
the LP64 assert-waiver tranches (R's 70→138 simulation), not host implementation.

## Method + caveats

1. Pass set: R's scratch `lp64native` logs (`comm -23 units fail`), independently re-derived — 70.
2. Objects: same force-includes (`compat/pso_metal_compat.h`, `build/waivers/pso_assert_waivers.h`) and
   include paths as P2-R, `-c` instead of `-fsyntax-only`; 70/70 compile.
3. Union: `nm -u` per object, sorted-unique; symbols **defined within the object set** are removed
   (they are not link gaps for a first link of exactly these objects — 10 such, e.g. the
   `FlagWord`/`THeap`/`xmalloc` pair refs).
4. libc/libsystem exclusion: membership against `xcrun --show-sdk-path` `libSystem/libc++/libc++abi`
   `.tbd` export lists (23: `_memcpy`, `_memset`, `_ceil`, `_floor`, `__Znwm/__ZdlPv`, unwinder,
   stack-guard, `__cxa_*`, `__class_type_info` vtables …).
5. Classification (order = precedence): scaffold → compat → source-definition check → era cross-check.
   * **Scaffold**: exact name match against 296 tokens extracted from `host/*.h` (`PSOSH_TAG(` arg-1,
     `#define`s, top-level prototype declarators). Word-prose matching was explicitly rejected —
     it false-positives on generic method names (`set`, `render`, `create`).
   * **Game-internal**: demangle (c++filt) → entity identifier → definition-shaped line in
     `src|include|libs` @ HEAD **outside the 70 pass files** (MWCC ctor/dtor `__ct__<len><Class>`
     grammar handled; namespace-implicit methods matched as `Class::method`). 556/560 carry a
     verbatim definition line (evidence column); 4 are era(game-obj)-defined renames whose C line
     did not match the def patterns (`_tl_05`, `_random_angle_range`, …) — still game-internal.
   * **Truly-missing**: nothing above matched ⇒ no backing inside the current link closure. Result 1 (`___upper_map` — MSL vendored source is outside both the host surface and the 1680-unit set).
   Caveats: the def-line test is a source-shape heuristic (a call-expression line with a
   trailing `,`/`(` continuation could false-hit; all 12 sampled audit lines were genuine
   definitions); era cross-checks were split so PowerPC_EABI_Support/dolphin extracts count as
   SDK-side (host gap signal), not game-internal; 23-item tbd grep may over-match exotic
   substrings (all 23 verified as real libc/libc++ symbols).

## Reproduction (exact)

The census is `docs`-embedded end to end: the bash below regenerates the object set and the
filtered gap list; the embedded Python regenerates `link-deps.tsv` from `classified.json`.
Both read `pso-decomp` strictly read-only (`git show` + era `build/GPOE8P/obj` nm) and write
only to a scratch dir.

```bash
#!/bin/bash
# P2-U repro: LP64 link-phase dependency census. Read-only vs pso-decomp; all output in scratch.
set -e
DECOMP=/Users/scott/Hermes/pso-decomp/pso-decomp          # must be @ 6b38b5383
REPO=/Users/scott/Hermes/pso-metal
SC=$(mktemp -d /tmp/p2u.XXXXXX); OBJ=$SC/objs; mkdir -p $OBJ
COMPAT=$REPO/compat/pso_metal_compat.h
WAIV=$REPO/build/waivers/pso_assert_waivers.h
COMMON="-w -fms-extensions -ffreestanding -Wno-c++11-narrowing -include $COMPAT -include $WAIV -Iinclude -Iinclude/clib -Ilibs/dolphin/include -Ilibs/dolphin/include/dolphin -Isrc"
cd "$DECOMP"
# 1) LP64-parse-pass list (identical probe to P2-R run_p2r.sh, -fsyntax-only)
find src \( -name '*.cpp' -o -name '*.c' \) | sort > $SC/units.txt
probe(){ f=$1; case $f in *.cpp) clang++ -fsyntax-only -std=c++17 $COMMON -target arm64-apple-macos11 "$f" > $SC/$(basename $f).err 2>&1;;
                 *.c) clang -fsyntax-only -std=c11 $COMMON -target arm64-apple-macos11 "$f" > $SC/$(basename $f).err 2>&1;; esac
  [ $? -ne 0 ] && echo "$f"; return 0; }
export -f probe; export COMMON
xargs -P20 -I@ bash -c 'probe "@"' < $SC/units.txt | sort > $SC/fail.txt
comm -23 $SC/units.txt $SC/fail.txt > $SC/pass.txt          # = 70 files @ 6b38b5383
# 2) compile the pass set to .o in scratch (NOT build/)
compile(){ f=$1; safe=${f//\//_}; base=${safe%.*}
  case $f in *.cpp) clang++ -c -std=c++17 $COMMON -target arm64-apple-macos11 -o "$OBJ/$base.o" "$f";;
               *.c) clang   -c -std=c11   $COMMON -target arm64-apple-macos11 -o "$OBJ/$base.o" "$f";; esac; }
export -f compile; export OBJ
xargs -P20 -I@ bash -c 'compile "@"' < $SC/pass.txt
ls $OBJ/*.o | wc -l                                        # = 70
# 3) UNION of undefined (nm -u), minus symbols defined inside the object set, minus libc/libsystem exports
for o in $OBJ/*.o; do nm -u "$o" | awk '{print $NF}'; done | sort -u > $SC/undef_union.txt   # 603
for o in $OBJ/*.o; do nm -g --defined-only "$o" | awk '$2 ~ /^[TDBusgR]$/ {print $3}'; done | sort -u > $SC/defined_set.txt
comm -23 $SC/undef_union.txt $SC/defined_set.txt > $SC/gaps_pre_sys.txt                        # 593
SDK=$(xcrun --show-sdk-path)
for t in libSystem.tbd libc++.tbd libc++abi.tbd; do grep -oE '_[A-Za-z0-9_.$]+' $SDK/usr/lib/$t; done | sort -u > $SC/sys_exports.txt
comm -23 $SC/gaps_pre_sys.txt $SC/sys_exports.txt > $SC/gaps.txt                               # 570 census rows
comm -12 $SC/gaps_pre_sys.txt $SC/sys_exports.txt | wc -l                                      # 23 system-backed
# 4) classify (system handled above; here: in-N-scaffold / in-compat-shim / game-internal-unresolved / truly-missing)
python3 - <<'PYCLASS'
#!/usr/bin/env python3
"""P2-U link-phase dependency census.
Inputs: 70 LP64-pass .o in OBJ dir; decomp @HEAD read-only via git show; era PPC objs via llvm-nm.
Output: classified.json + TSV rows."""
import subprocess, os, re, json, sys

SC  = "/Users/scott/.hermes/profiles/pso-decomp/cache/scratch/clangprobe"
OBJ = f"{SC}/p2u_objs"
DECOMP = "/Users/scott/Hermes/pso-decomp/pso-decomp"
REPO = "/Users/scott/Hermes/pso-metal"
NM = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/llvm-nm"
PASS = set(l.strip() for l in open(f"{SC}/lp64_pass.txt") if l.strip())

# ---------- 1. undefined pairs (Mach-O nm) ----------
pairs = {}
for f in sorted(os.listdir(OBJ)):
    if not f.endswith(".o"): continue
    out = subprocess.run(["nm","-u",os.path.join(OBJ,f)],capture_output=True,text=True).stdout
    for ln in out.splitlines():
        s = ln.strip()
        if s: pairs.setdefault(s, set()).add(f[:-2])
defined_in_set = set()
for f in sorted(os.listdir(OBJ)):
    if not f.endswith(".o"): continue
    out = subprocess.run(["nm","-g","--defined-only",os.path.join(OBJ,f)],capture_output=True,text=True).stdout
    for ln in out.splitlines():
        parts = ln.split()
        if len(parts)>=3 and parts[1] in "TDBusgR": defined_in_set.add(parts[2])
print("distinct undefined:", len(pairs), "| defined-in-set:", len(defined_in_set))
internal = {s: f for s,f in pairs.items() if s in defined_in_set}
pairs = {s: f for s,f in pairs.items() if s not in defined_in_set}
print("undefined NOT defined by any of the 70 objs (true link gaps):", len(pairs),
      "| subset-internal:", len(internal))
json.dump({"internal":[s for s in internal]}, open(f"{OBJ}/subset_internal.json","w"))

# ---------- 2. system exports ----------
sdk = subprocess.run(["xcrun","--show-sdk-path"],capture_output=True,text=True).stdout.strip()
sys_syms = set()
for t in ["libSystem.tbd","libc++.tbd","libc++abi.tbd"]:
    fp = os.path.join(sdk,"usr/lib",t)
    if os.path.exists(fp):
        sys_syms.update(re.findall(r"_[A-Za-z0-9_.$]+", open(fp,errors="replace").read()))

# ---------- 3. host scaffold + compat header texts ----------
hdr_txt = {h: open(f"{REPO}/host/{h}",errors="replace").read()
           for h in os.listdir(f"{REPO}/host") if h.endswith(".h")}
scaffold_names = {}
for h, t in hdr_txt.items():
    names = set(re.findall(r"PSOSH_TAG\(\s*([A-Za-z_]\w*)", t))
    names |= set(re.findall(r"^\s*#\s*define\s+([A-Z][A-Za-z0-9_]*)\b", t, re.M))
    # featured prototypes: line-initial `RetType name(args` (not typedef/struct/enum/#define/comment)
    for ln in t.splitlines():
        s = ln.strip()
        if not s or s.startswith(("//","*","#","typedef","struct","enum","union")): continue
        m = re.match(r"^[A-Za-z_][\w\s\*&<>,]*?([A-Za-z_]\w*)\s*\(", s)
        if m and m.group(1) not in ("if","for","while","sizeof","switch","return"):
            names.add(m.group(1))
    for n in names: scaffold_names.setdefault(n, h)
print("scaffold name set:", len(scaffold_names))
compat_txt = {}
for f, t in [(f, open(f"{REPO}/compat/{f}",errors="replace").read())
             for f in os.listdir(f"{REPO}/compat") if f.endswith(".h")]:
    compat_txt[f] = t
compat_names = {}
for f, t in compat_txt.items():
    for m in re.finditer(r"^\s*(?:static\s+inline|inline)\s+[\w\s\*&]+?\b([A-Za-z_]\w*)\s*\(", t, re.M):
        compat_names.setdefault(m.group(1), f)
    for m in re.finditer(r"#\s*define\s+([A-Za-z_]\w*)", t):
        compat_names.setdefault(m.group(1), f)

# ---------- 4. decomp corpus at HEAD ----------
rev = subprocess.run(["git","-C",DECOMP,"rev-parse","HEAD"],capture_output=True,text=True).stdout.strip()
files = subprocess.run(["git","-C",DECOMP,"ls-tree","-r","--name-only",rev],capture_output=True,text=True).stdout.splitlines()
keep = [f for f in files if f.startswith(("src/","include/","libs/")) and f.endswith((".c",".cpp",".h"))]
inp = ("\n".join(f"{rev}:{f}" for f in keep)+"\n").encode()
out = subprocess.run(["git","-C",DECOMP,"cat-file","--batch"], input=inp, capture_output=True).stdout
corpus = {}
pos = 0
for rel in keep:
    nl = out.index(b"\n", pos)
    hdr = out[pos:nl].split()
    pos = nl+1
    if len(hdr)==3 and hdr[1]==b"blob":
        size = int(hdr[2])
        corpus[rel] = out[pos:pos+size].decode("utf-8","replace")
        pos += size+1          # skip trailing newline
    else:
        corpus[rel] = ""
print("corpus files:", len(corpus), "at", rev[:10])

# ---------- 5. era defined symbols (game-internal universe, all 9663 PPC objs) ----------
all_obj_dirs = {"POWERPC": ("PowerPC_EABI_Support","TRK_MINNOW_DOLPHIN","ai","amcstubs","ar","base","card","db","dsp","dvd","exi","gba","gx","mtx","odemuexi2","odenotstub","os","pad","si","vi")}
era_objs = sorted(os.path.join(dp,f) for dp,_,fs in os.walk(f"{DECOMP}/build/GPOE8P/obj") for f in fs if f.endswith(".o"))
era, era_sdk = set(), set()
def is_game(pth):
    rel = pth.split("/obj/")[1]
    return rel.startswith(("pso/","musyx/")) or rel.startswith("auto_")
for i2 in range(0,len(era_objs),400):
    batch = era_objs[i2:i2+400]
    p = subprocess.run([NM,"--defined-only","-g",*batch],capture_output=True,text=True)
    cur = None
    for ln in p.stdout.splitlines():
        if ln.endswith(".o:"):
            cur = is_game("/".join(ln[:-1].split("/"))) if "/" in ln else None
            tgt = era if cur else era_sdk
            continue
        parts = ln.split()
        tgt = era if (cur is None or cur) else era_sdk
        # cur None => nm didn't print a header (single file); decide per batch:
        if len(parts)>=3 and re.fullmatch(r"[A-Za-z]",parts[-2]): tgt.add(parts[-1])
        elif len(parts)==2 and re.fullmatch(r"[A-Za-z]",parts[0]): tgt.add(parts[1])
era_list = sorted(era)
print("era GAME defined:", len(era), "| era SDK defined:", len(era_sdk))

# ---------- 6. entity extraction (demangle) ----------
def entity(sym):
    """-> dict(ident, qual, kind, dem)"""
    if sym.startswith("__Z") or (sym.startswith("_Z") and not sym.startswith("_ZTV")==False or False) :
        pass
    if re.match(r"^__?Z(?:N|L?\d|St|TV|TI|TS|TV|GV|dl|nw|naj|da|ls|rs|eq|ne|lt|gt|le|ge|pl|mi|cl|ao|ad|rm|co)", sym):
        d = subprocess.run(["c++filt",sym],capture_output=True,text=True).stdout.strip()
        mt = re.match(r"^(?:vtable|typeinfo(?:\[\d+\])?|construction vtable) for (.+)$", d)
        if mt:
            cls = re.sub(r"<.*>","",mt.group(1).strip())
            inner = cls.split("::")[-1]
            return dict(dem=d, ident=inner, qual=inner, kind="vtable")
        base = re.sub(r"\(.*\)","",d)
        base = re.sub(r"<.*>","",base)
        parts = [x.strip() for x in base.split("::")]
        if len(parts)>=2 and parts[-1].startswith("operator"):
            op = parts[-1].split()[-1]
            optab = {"new":"nw","delete":"dl","new[]":"naj","delete[]":"daa"}
            return dict(dem=d, ident=optab.get(op,"operator"), qual=parts[-2], kind="operator")
        ident = parts[-1].split()[-1] if parts else sym
        ident = ident.lstrip("~") if ident.startswith("~") else ident
        qual = parts[-2] if len(parts)>=2 else None
        kind = "method" if (qual and qual[0].isupper()) else "func"
        return dict(dem=d, ident=ident, qual=qual, kind=kind)
    return dict(dem=None, ident=sym[1:] if sym.startswith("_") else sym, qual=None, kind="plain")

ent = {s: entity(s) for s in pairs}

# ---------- 7. token index over corpus ----------
tokre = re.compile(r"[A-Za-z_]\w*")
all_names = set(e["ident"] for e in ent.values())
index = {n: [] for n in all_names}
cls_index = {}   # class-name -> files with Class:: lines
for rel, txt in corpus.items():
    for ln in txt.splitlines():
        st = ln.strip()
        for tok in tokre.findall(ln):
            if tok in index: index[tok].append((rel, st))
print("index built")

def era_hit(e):
    """any era MWCC/plain name backing this entity?"""
    ident, qual, kind = e["ident"], e["qual"], e["kind"]
    if kind == "vtable":
        pat = re.compile(rf"(^|[^A-Za-z0-9]){len(qual)}{re.escape(qual)}($|[^0-9A-Za-z])")
        return any(pat.search(n) for n in era_list)
    if ident in era: return True
    pref = ident + "__"   # MWCC: name__<mangling>
    if kind == "method":
        ctok = str(len(qual)) + qual if qual else None
        if any((n.startswith(pref) or ("__"+pref) in n) and (not ctok or ctok in n) for n in era_list):
            return True
        # MWCC ctor/dtor grammar: __ct__<enc><Class> / __dt__<enc><Class>
        if ctok: return any(re.match(r"^(?:__ct__|__dt__|_Q2)(?:\w*)"+re.escape(ctok), n) or re.match(r"^__(?:ct|dt)__\d*", n) and ctok in n for n in era_list)
    if kind == "operator":
        ctok = str(len(qual)) + qual
        return any(n.startswith("__"+e["ident"]+"__") and ctok in n for n in era_list)
    return any(n.startswith(pref) or ("__"+pref) in n for n in era_list)

def def_evidence(e):
    """source lines outside PASS that define the entity."""
    ident, qual, kind = e["ident"], e["qual"], e["kind"]
    if kind == "vtable":
        pat = re.compile(r"\b"+re.escape(qual)+r"::")
        for rel, st in index[ident]:
            if rel in PASS or st.startswith(("//","*","/*")): continue
            if pat.search(st): return (rel, st[:100])
        return None
    if kind in ("method","operator"):
        idn = ident if kind=="method" else {"nw":"operator\s+new","dl":"operator\s+delete",
               "naj":"operator\s+new\[\]","daa":"operator\s+delete\[\]"}.get(ident,"operator")
        pat = re.compile(r"\b"+re.escape(qual)+r"\s*(?:<[^>]*>)?::\s*~?\s*"+idn+r"\s*(\(|\{|;)")
        for rel, st in index.get(qual if kind=="operator" else ident, index.get(ident,[])):
            if rel in PASS or st.startswith(("//","*","/*")): continue
            if pat.search(st): return (rel, st[:100])
        return None
    # plain/func: generic def heuristic
    idre = re.compile(r"(?<![\w:])"+re.escape(ident)+r"\s*(\(|=)")
    for rel, st in index[ident]:
        if rel in PASS or st.startswith(("//","*","/*","extern","class","struct","typedef","return","case")): continue
        if st.startswith("virtual") or st.startswith("using"): continue
        if idre.search(st):
            if "(" in st and st.rstrip().endswith(";"): continue   # prototype
            if st.rstrip().endswith(","): continue
            return (rel, st[:100])
    return None

# ---------- 8. classify ----------
rows = []
cache = {}
for sym, fset in sorted(pairs.items()):
    e = ent[sym]; ident = e["ident"]
    if sym in sys_syms:
        rows.append(dict(sym=sym, dem=e["dem"], cls="system-libc", back="-", objs=sorted(fset), ev="SDK export")); continue
    if ident in scaffold_names:
        rows.append(dict(sym=sym, dem=e["dem"], cls="in-N-scaffold", back=scaffold_names[ident], objs=sorted(fset), ev="scaffold entry")); continue
    if ident in compat_names:
        rows.append(dict(sym=sym, dem=e["dem"], cls="in-compat-shim", back=compat_names[ident], objs=sorted(fset), ev="compat header")); continue
    k = (e["kind"], ident, e["qual"])
    if k not in cache: cache[k] = (def_evidence(e), era_hit(e))
    ev, eh = cache[k]
    if ev:
        rows.append(dict(sym=sym, dem=e["dem"], cls="game-internal-unresolved", back="none", objs=sorted(fset),
                         ev="def:"+ev[0], defline=ev[1]))
    elif eh:
        rows.append(dict(sym=sym, dem=e["dem"], cls="game-internal-unresolved", back="none", objs=sorted(fset), ev="era(game-obj) hit, no C-def line matched"))
    elif sym[1:] in era_sdk or ident in era_sdk:
        rows.append(dict(sym=sym, dem=e["dem"], cls="truly-missing", back="none", objs=sorted(fset),
                         ev="era-SDK-defined only (host scaffold gap)"))
    else:
        men = len(index[ident]) if ident in index else 0
        rows.append(dict(sym=sym, dem=e["dem"], cls="truly-missing", back="none", objs=sorted(fset),
                         ev=(f"mentions:{men} no-def no-era" if men else "absent")))

from collections import Counter
print(json.dumps(dict(Counter(r["cls"] for r in rows)), indent=1))
json.dump(rows, open(f"{OBJ}/classified.json","w"), indent=0)

tm = [r for r in rows if r["cls"]=="truly-missing"]
print("\n== TRULY-MISSING top by #referencing objects ==")
for r in sorted(tm, key=lambda r: -len(r["objs"]))[:25]:
    print(f"{len(r['objs']):>3} {r['sym'][:58]:<58} {r['dem'] or ''}"[:150], "|", r["ev"])
gi = [r for r in rows if r["cls"]=="game-internal-unresolved"]
sc = [r for r in rows if r["cls"]=="in-N-scaffold"]
cp = [r for r in rows if r["cls"]=="in-compat-shim"]
sy = [r for r in rows if r["cls"]=="system-libc"]
print(f"\nSUMMARY gaps={len(rows)}: scaffold={len(sc)} compat={len(cp)} game-internal={len(gi)} truly-missing={len(tm)} system={len(sy)}")


PYCLASS
echo "repro done: $SC (link-deps.tsv, classified.json)"
```

```python
# p2u_classify.py — the classifier actually run (python3, stdlib only)
#!/usr/bin/env python3
"""P2-U link-phase dependency census.
Inputs: 70 LP64-pass .o in OBJ dir; decomp @HEAD read-only via git show; era PPC objs via llvm-nm.
Output: classified.json + TSV rows."""
import subprocess, os, re, json, sys

SC  = "/Users/scott/.hermes/profiles/pso-decomp/cache/scratch/clangprobe"
OBJ = f"{SC}/p2u_objs"
DECOMP = "/Users/scott/Hermes/pso-decomp/pso-decomp"
REPO = "/Users/scott/Hermes/pso-metal"
NM = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/llvm-nm"
PASS = set(l.strip() for l in open(f"{SC}/lp64_pass.txt") if l.strip())

# ---------- 1. undefined pairs (Mach-O nm) ----------
pairs = {}
for f in sorted(os.listdir(OBJ)):
    if not f.endswith(".o"): continue
    out = subprocess.run(["nm","-u",os.path.join(OBJ,f)],capture_output=True,text=True).stdout
    for ln in out.splitlines():
        s = ln.strip()
        if s: pairs.setdefault(s, set()).add(f[:-2])
defined_in_set = set()
for f in sorted(os.listdir(OBJ)):
    if not f.endswith(".o"): continue
    out = subprocess.run(["nm","-g","--defined-only",os.path.join(OBJ,f)],capture_output=True,text=True).stdout
    for ln in out.splitlines():
        parts = ln.split()
        if len(parts)>=3 and parts[1] in "TDBusgR": defined_in_set.add(parts[2])
print("distinct undefined:", len(pairs), "| defined-in-set:", len(defined_in_set))
internal = {s: f for s,f in pairs.items() if s in defined_in_set}
pairs = {s: f for s,f in pairs.items() if s not in defined_in_set}
print("undefined NOT defined by any of the 70 objs (true link gaps):", len(pairs),
      "| subset-internal:", len(internal))
json.dump({"internal":[s for s in internal]}, open(f"{OBJ}/subset_internal.json","w"))

# ---------- 2. system exports ----------
sdk = subprocess.run(["xcrun","--show-sdk-path"],capture_output=True,text=True).stdout.strip()
sys_syms = set()
for t in ["libSystem.tbd","libc++.tbd","libc++abi.tbd"]:
    fp = os.path.join(sdk,"usr/lib",t)
    if os.path.exists(fp):
        sys_syms.update(re.findall(r"_[A-Za-z0-9_.$]+", open(fp,errors="replace").read()))

# ---------- 3. host scaffold + compat header texts ----------
hdr_txt = {h: open(f"{REPO}/host/{h}",errors="replace").read()
           for h in os.listdir(f"{REPO}/host") if h.endswith(".h")}
scaffold_names = {}
for h, t in hdr_txt.items():
    names = set(re.findall(r"PSOSH_TAG\(\s*([A-Za-z_]\w*)", t))
    names |= set(re.findall(r"^\s*#\s*define\s+([A-Z][A-Za-z0-9_]*)\b", t, re.M))
    # featured prototypes: line-initial `RetType name(args` (not typedef/struct/enum/#define/comment)
    for ln in t.splitlines():
        s = ln.strip()
        if not s or s.startswith(("//","*","#","typedef","struct","enum","union")): continue
        m = re.match(r"^[A-Za-z_][\w\s\*&<>,]*?([A-Za-z_]\w*)\s*\(", s)
        if m and m.group(1) not in ("if","for","while","sizeof","switch","return"):
            names.add(m.group(1))
    for n in names: scaffold_names.setdefault(n, h)
print("scaffold name set:", len(scaffold_names))
compat_txt = {}
for f, t in [(f, open(f"{REPO}/compat/{f}",errors="replace").read())
             for f in os.listdir(f"{REPO}/compat") if f.endswith(".h")]:
    compat_txt[f] = t
compat_names = {}
for f, t in compat_txt.items():
    for m in re.finditer(r"^\s*(?:static\s+inline|inline)\s+[\w\s\*&]+?\b([A-Za-z_]\w*)\s*\(", t, re.M):
        compat_names.setdefault(m.group(1), f)
    for m in re.finditer(r"#\s*define\s+([A-Za-z_]\w*)", t):
        compat_names.setdefault(m.group(1), f)

# ---------- 4. decomp corpus at HEAD ----------
rev = subprocess.run(["git","-C",DECOMP,"rev-parse","HEAD"],capture_output=True,text=True).stdout.strip()
files = subprocess.run(["git","-C",DECOMP,"ls-tree","-r","--name-only",rev],capture_output=True,text=True).stdout.splitlines()
keep = [f for f in files if f.startswith(("src/","include/","libs/")) and f.endswith((".c",".cpp",".h"))]
inp = ("\n".join(f"{rev}:{f}" for f in keep)+"\n").encode()
out = subprocess.run(["git","-C",DECOMP,"cat-file","--batch"], input=inp, capture_output=True).stdout
corpus = {}
pos = 0
for rel in keep:
    nl = out.index(b"\n", pos)
    hdr = out[pos:nl].split()
    pos = nl+1
    if len(hdr)==3 and hdr[1]==b"blob":
        size = int(hdr[2])
        corpus[rel] = out[pos:pos+size].decode("utf-8","replace")
        pos += size+1          # skip trailing newline
    else:
        corpus[rel] = ""
print("corpus files:", len(corpus), "at", rev[:10])

# ---------- 5. era defined symbols (game-internal universe, all 9663 PPC objs) ----------
all_obj_dirs = {"POWERPC": ("PowerPC_EABI_Support","TRK_MINNOW_DOLPHIN","ai","amcstubs","ar","base","card","db","dsp","dvd","exi","gba","gx","mtx","odemuexi2","odenotstub","os","pad","si","vi")}
era_objs = sorted(os.path.join(dp,f) for dp,_,fs in os.walk(f"{DECOMP}/build/GPOE8P/obj") for f in fs if f.endswith(".o"))
era, era_sdk = set(), set()
def is_game(pth):
    rel = pth.split("/obj/")[1]
    return rel.startswith(("pso/","musyx/")) or rel.startswith("auto_")
for i2 in range(0,len(era_objs),400):
    batch = era_objs[i2:i2+400]
    p = subprocess.run([NM,"--defined-only","-g",*batch],capture_output=True,text=True)
    cur = None
    for ln in p.stdout.splitlines():
        if ln.endswith(".o:"):
            cur = is_game("/".join(ln[:-1].split("/"))) if "/" in ln else None
            tgt = era if cur else era_sdk
            continue
        parts = ln.split()
        tgt = era if (cur is None or cur) else era_sdk
        # cur None => nm didn't print a header (single file); decide per batch:
        if len(parts)>=3 and re.fullmatch(r"[A-Za-z]",parts[-2]): tgt.add(parts[-1])
        elif len(parts)==2 and re.fullmatch(r"[A-Za-z]",parts[0]): tgt.add(parts[1])
era_list = sorted(era)
print("era GAME defined:", len(era), "| era SDK defined:", len(era_sdk))

# ---------- 6. entity extraction (demangle) ----------
def entity(sym):
    """-> dict(ident, qual, kind, dem)"""
    if sym.startswith("__Z") or (sym.startswith("_Z") and not sym.startswith("_ZTV")==False or False) :
        pass
    if re.match(r"^__?Z(?:N|L?\d|St|TV|TI|TS|TV|GV|dl|nw|naj|da|ls|rs|eq|ne|lt|gt|le|ge|pl|mi|cl|ao|ad|rm|co)", sym):
        d = subprocess.run(["c++filt",sym],capture_output=True,text=True).stdout.strip()
        mt = re.match(r"^(?:vtable|typeinfo(?:\[\d+\])?|construction vtable) for (.+)$", d)
        if mt:
            cls = re.sub(r"<.*>","",mt.group(1).strip())
            inner = cls.split("::")[-1]
            return dict(dem=d, ident=inner, qual=inner, kind="vtable")
        base = re.sub(r"\(.*\)","",d)
        base = re.sub(r"<.*>","",base)
        parts = [x.strip() for x in base.split("::")]
        if len(parts)>=2 and parts[-1].startswith("operator"):
            op = parts[-1].split()[-1]
            optab = {"new":"nw","delete":"dl","new[]":"naj","delete[]":"daa"}
            return dict(dem=d, ident=optab.get(op,"operator"), qual=parts[-2], kind="operator")
        ident = parts[-1].split()[-1] if parts else sym
        ident = ident.lstrip("~") if ident.startswith("~") else ident
        qual = parts[-2] if len(parts)>=2 else None
        kind = "method" if (qual and qual[0].isupper()) else "func"
        return dict(dem=d, ident=ident, qual=qual, kind=kind)
    return dict(dem=None, ident=sym[1:] if sym.startswith("_") else sym, qual=None, kind="plain")

ent = {s: entity(s) for s in pairs}

# ---------- 7. token index over corpus ----------
tokre = re.compile(r"[A-Za-z_]\w*")
all_names = set(e["ident"] for e in ent.values())
index = {n: [] for n in all_names}
cls_index = {}   # class-name -> files with Class:: lines
for rel, txt in corpus.items():
    for ln in txt.splitlines():
        st = ln.strip()
        for tok in tokre.findall(ln):
            if tok in index: index[tok].append((rel, st))
print("index built")

def era_hit(e):
    """any era MWCC/plain name backing this entity?"""
    ident, qual, kind = e["ident"], e["qual"], e["kind"]
    if kind == "vtable":
        pat = re.compile(rf"(^|[^A-Za-z0-9]){len(qual)}{re.escape(qual)}($|[^0-9A-Za-z])")
        return any(pat.search(n) for n in era_list)
    if ident in era: return True
    pref = ident + "__"   # MWCC: name__<mangling>
    if kind == "method":
        ctok = str(len(qual)) + qual if qual else None
        if any((n.startswith(pref) or ("__"+pref) in n) and (not ctok or ctok in n) for n in era_list):
            return True
        # MWCC ctor/dtor grammar: __ct__<enc><Class> / __dt__<enc><Class>
        if ctok: return any(re.match(r"^(?:__ct__|__dt__|_Q2)(?:\w*)"+re.escape(ctok), n) or re.match(r"^__(?:ct|dt)__\d*", n) and ctok in n for n in era_list)
    if kind == "operator":
        ctok = str(len(qual)) + qual
        return any(n.startswith("__"+e["ident"]+"__") and ctok in n for n in era_list)
    return any(n.startswith(pref) or ("__"+pref) in n for n in era_list)

def def_evidence(e):
    """source lines outside PASS that define the entity."""
    ident, qual, kind = e["ident"], e["qual"], e["kind"]
    if kind == "vtable":
        pat = re.compile(r"\b"+re.escape(qual)+r"::")
        for rel, st in index[ident]:
            if rel in PASS or st.startswith(("//","*","/*")): continue
            if pat.search(st): return (rel, st[:100])
        return None
    if kind in ("method","operator"):
        idn = ident if kind=="method" else {"nw":"operator\s+new","dl":"operator\s+delete",
               "naj":"operator\s+new\[\]","daa":"operator\s+delete\[\]"}.get(ident,"operator")
        pat = re.compile(r"\b"+re.escape(qual)+r"\s*(?:<[^>]*>)?::\s*~?\s*"+idn+r"\s*(\(|\{|;)")
        for rel, st in index.get(qual if kind=="operator" else ident, index.get(ident,[])):
            if rel in PASS or st.startswith(("//","*","/*")): continue
            if pat.search(st): return (rel, st[:100])
        return None
    # plain/func: generic def heuristic
    idre = re.compile(r"(?<![\w:])"+re.escape(ident)+r"\s*(\(|=)")
    for rel, st in index[ident]:
        if rel in PASS or st.startswith(("//","*","/*","extern","class","struct","typedef","return","case")): continue
        if st.startswith("virtual") or st.startswith("using"): continue
        if idre.search(st):
            if "(" in st and st.rstrip().endswith(";"): continue   # prototype
            if st.rstrip().endswith(","): continue
            return (rel, st[:100])
    return None

# ---------- 8. classify ----------
rows = []
cache = {}
for sym, fset in sorted(pairs.items()):
    e = ent[sym]; ident = e["ident"]
    if sym in sys_syms:
        rows.append(dict(sym=sym, dem=e["dem"], cls="system-libc", back="-", objs=sorted(fset), ev="SDK export")); continue
    if ident in scaffold_names:
        rows.append(dict(sym=sym, dem=e["dem"], cls="in-N-scaffold", back=scaffold_names[ident], objs=sorted(fset), ev="scaffold entry")); continue
    if ident in compat_names:
        rows.append(dict(sym=sym, dem=e["dem"], cls="in-compat-shim", back=compat_names[ident], objs=sorted(fset), ev="compat header")); continue
    k = (e["kind"], ident, e["qual"])
    if k not in cache: cache[k] = (def_evidence(e), era_hit(e))
    ev, eh = cache[k]
    if ev:
        rows.append(dict(sym=sym, dem=e["dem"], cls="game-internal-unresolved", back="none", objs=sorted(fset),
                         ev="def:"+ev[0], defline=ev[1]))
    elif eh:
        rows.append(dict(sym=sym, dem=e["dem"], cls="game-internal-unresolved", back="none", objs=sorted(fset), ev="era(game-obj) hit, no C-def line matched"))
    elif sym[1:] in era_sdk or ident in era_sdk:
        rows.append(dict(sym=sym, dem=e["dem"], cls="truly-missing", back="none", objs=sorted(fset),
                         ev="era-SDK-defined only (host scaffold gap)"))
    else:
        men = len(index[ident]) if ident in index else 0
        rows.append(dict(sym=sym, dem=e["dem"], cls="truly-missing", back="none", objs=sorted(fset),
                         ev=(f"mentions:{men} no-def no-era" if men else "absent")))

from collections import Counter
print(json.dumps(dict(Counter(r["cls"] for r in rows)), indent=1))
json.dump(rows, open(f"{OBJ}/classified.json","w"), indent=0)

tm = [r for r in rows if r["cls"]=="truly-missing"]
print("\n== TRULY-MISSING top by #referencing objects ==")
for r in sorted(tm, key=lambda r: -len(r["objs"]))[:25]:
    print(f"{len(r['objs']):>3} {r['sym'][:58]:<58} {r['dem'] or ''}"[:150], "|", r["ev"])
gi = [r for r in rows if r["cls"]=="game-internal-unresolved"]
sc = [r for r in rows if r["cls"]=="in-N-scaffold"]
cp = [r for r in rows if r["cls"]=="in-compat-shim"]
sy = [r for r in rows if r["cls"]=="system-libc"]
print(f"\nSUMMARY gaps={len(rows)}: scaffold={len(sc)} compat={len(cp)} game-internal={len(gi)} truly-missing={len(tm)} system={len(sy)}")


```
