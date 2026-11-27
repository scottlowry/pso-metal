# P2-AF — Link-hygiene gate (reusable `tools/link_hygiene.sh`)

Phase-2 lane AF. Turns the P2-AA one-time audit ([`docs/p2-musyx-linknames.md`](p2-musyx-linknames.md) §5
"nm-gate idea") into a permanent build gate. Script: [`tools/link_hygiene.sh`](../tools/link_hygiene.sh);
golden set: [`data/musyx-linknames.tsv`](../data/musyx-linknames.tsv) (27 rows = 24 func crossers +
3 `synth_dbtab.c` data tables). Everything below was measured 2026-11-26 against
`pso-decomp @ 6b38b5383` **read-only**; all objects live in `${TMPDIR}/p2af_selftest` (scratch).

## 1. Usage

```
tools/link_hygiene.sh <obj-dir> [--musyx] [--tsv <file>]
```

`<obj-dir>` is scanned recursively for `*.o`; symbols come from `nm -g --defined-only`
(same convention as the P2-U census). Absolute-path safe: the TSV resolves from the
script's own repo location, never from CWD; relative `<obj-dir>` and relative script
invocation both verified.

| Assertion | Default mode (`golden-scope`, mixed first-link battery) | `--musyx` mode (pure musyx archive dir) | Gatekeeping |
|---|---|---|---|
| (a) `__Z…` C++-mangled defined globals | `c++filt`-demangles every `__Z` name; fails only if the demangled identifier **is a golden name** (pso C++ `__Z` are legal) | **any** `__Z` defined global fails (AA §5 hard rule) | FAIL + exit 1 |
| (b) golden names defined | `func` rows → exact `_name` with nm type `T`; `data` rows → exact `_name` with type `D/B/S/R`; a mangled twin counts as ABSENT | same | FAIL + exit 1 |
| (c) duplicate `T` definitions | names with type `T` in ≥2 objects, per-name `xN` + object list | same | **WARNING only** (non-fatal) |

Exit codes: `0` = (a)+(b) pass (dupes may still warn) · `1` = assertion failure ·
`2` = usage/env error (missing dir, missing TSV, `nm` absent). Final line:

```
SUMMARY objs=N mangled=V/TZ golden=F/24+D/3 dupT=X verdict=PASS|FAIL[a][b]
```

## 2. Two-mode self-test (real musyx sources → scratch `.o`)

Battery files (the exact TU set covering all 24 golden funcs + the 3 tables):
`snd_synthapi.c`(8) `stream.c`(8) `s_data.c`(2) `Chorus/chorus_fx.c`(2) `StdReverb/reverb_fx.c`(2)
`snd_init.c`(1) `hardware.c`(1) + `synth_dbtab.c`(data×3). Flags = T's exact flip set from
`compat/dialect/flip_test.sh` (`COMMON` = P2-R probe flags incl. `pso_metal_compat.h` +
assert waivers; `DIALECT` = `-I compat/dialect/pshim -include compat/dialect/pso_dialect_compat.h`).

**Compile-mode survey (`-c`, results per file):**

| TU | `clang++ -std=c++17 -x c++ $DIALECT $COMMON` (T pattern) | `clang -std=c11 -x c $DIALECT $COMMON` (pure C) |
|---|---|---|
| snd_synthapi / stream / s_data / snd_init / hardware | PASS | **FAIL** — `error: expected identifier or '('` at the bare `extern "C" {` (:4/:4/:37/:3/:5) |
| chorus_fx | **FAIL** — `fatal error: 'orig_asm/.../do_src1.inc' file not found` (:114; see §4) | FAIL (same bare-EC error) |
| reverb_fx | PASS | **FAIL** — bare EC + `include/pso/audio/SoundReverb.h:11` tag-struct error in C mode |
| synth_dbtab | PASS | **PASS** (P3 re-verified; the only runtime `.c` with no `extern "C"` token — `grep -L` confirms) |

So the gate ran on four batteries (both modes), stand-ins = two plain-C scratch defs of
the 2 chorus names (pure scratch fixtures, §4):

| Battery | objects | gate | verdict |
|---|---|---|---|
| `bat_cpp` — **mode 1 (T pattern)**: 7 real `.o` all `clang++ -x c++` (dbtab relying on the ABI property) + 2 chorus stand-ins | 8 | `--musyx` | **PASS exit 0** |
| `bat_mix` — **mode 2 (-x c guarded)**: same, but `synth_dbtab.c` compiled `clang -x c` (P3 per-TU exception; the EC-block files cannot leave C++ mode) | 8 | `--musyx` | **PASS exit 0** |
| `objs_cpp` — real-only (no stand-ins) | 7 | `--musyx` | FAIL[b] exit 1 (22/24 + 3/3; the 2 chorus names listed absent) |
| `merged` — `bat_cpp`+`bat_mix` in one dir (every symbol twice) | 16 | `--musyx` | PASS exit 0 **with 85 `WARN c` dup names** |

`bat_cpp` / `bat_mix` verbatim:

```
== link_hygiene musyx-strict: …/p2af_selftest/bat_cpp        (bat_mix identical except path)
   objs=8 defined_globals=97 tsv=…/data/musyx-linknames.tsv
-- (a) C++-mangled defined globals: 0 violation(s) (of 0 distinct __Z)
-- (b) golden names: func 24/24 as T, data 3/3 as D/B/S/R
-- (c) duplicate T definitions: 0 symbol name(s) [WARNING only]
SUMMARY objs=8 mangled=0/0 golden=24/24+3/3 dupT=0 verdict=PASS
```

## 3. AA's "Itanium accident" claim — **CONFIRMED empirically**

P2-AA §2/§3 asserted the 3 unguarded `synth_dbtab.c` tables link in C++ mode "only by
the data-mangling accident" (Itanium never mangles file-scope variables). Compiled
`synth_dbtab.c` (no `extern "C"` anywhere, includes only `types.h`) with plain
`clang++ -x c++` — real `nm -g --defined-only` output:

```
0000000000000000 D _dspAttenuationTab
0000000000000184 D _dspScale2IndexTab
0000000000000584 D _dspDLSVolTab
```

Unmangled despite zero EC → claim CONFIRMED. The same three names appear with identical
types when the TU is compiled `-x c` (guarded P3 route; base addresses shift because the
DIALECT force-include additionally leaks `_sqrt`/`_sqrtf` T defs into that variant — kept
out of `bat_mix` by compiling the guarded dbtab TU with `$COMMON` only). `grep '^__Z'`
across **all** `bat_cpp` objects returns zero lines, so gate (a) holds in both modes.
The gate now *catches* the accident ending: if anyone adds a function to that TU or
wraps it in a namespace, (a) fires on the new `__Z` name — and if a golden function ever
escapes its `extern "C"` block, (a)+(b) fire together even in default mode:

```
$ tools/link_hygiene.sh …/bad2          # scratch escaper.cpp: int sndInit(u,u,u,u) without EC, snd_init.o removed
-- (a) …: 1 violation(s) (of 2 distinct __Z)
   FAIL a: __Z7sndInitjjjj  [escaper.o ]  dem: sndInit(unsigned int, …)
-- (b) golden names: func 23/24 as T, data 3/3 as D/B/S/R
   FAIL b: sndInit (func): absent
SUMMARY … verdict=FAIL[a+b]             # exit 1; the non-golden __Z19musyxExtraMangledFni ignored in default mode,
                                        # and caught (2/2) in --musyx mode
```

## 4. New finding for the T-flip landing lane: `chorus_fx.c` has no host-compilable path

`src/musyx/runtime/Chorus/chorus_fx.c:114/118` `#include` `orig_asm/musyx/runtime/Chorus/
chorus_fx/do_src{1,2}.inc` — present only under `build/GPOE8P/include/` (add `-I` and the
error moves to the content: `nofralloc` / `opword 0x9421FFC0` = MWCC inline-asm blobs the
host clang cannot parse at all). Consequence: **2 of 24 golden names**
(`sndAuxCallbackChorus`, `sndAuxCallbackPrepareChorus`) have no host object until chorus_fx
is port-side handled (vendored asm translated, or the callbacks defined in a port EC-shim
TU à la P6, or the `.inc` dirs land in the flip include path with a dialect guard). The
flip-test pool count (26/26 at P2-T time) evidently never `-c`'d this TU. Until it lands,
the honest real-only battery is 22/24 — gate exit 1, exactly as designed.

## 5. Paste-ready integration (first-link battery + musyx archive rule)

```bash
# in the build script, AFTER the musyx .o rule and after the first full link-object pass:
"$REPO/tools/link_hygiene.sh" "$OBJ_ROOT/musyx" --musyx \
  || { echo "FATAL: musyx link-hygiene gate failed (docs/p2-link-hygiene.md)"; exit 1; }
"$REPO/tools/link_hygiene.sh" "$OBJ_ROOT"        # mixed first-link battery: golden-scope
```

Gate (c) is the canary for double-vending (e.g. a golden symbol defined both by a musyx
`.o` and a pso reconvention stub — the `gx_shims.c:156` T's P2-AB collision precedent);
warnings don't fail the build but every dup on a golden name deserves an owner.

## Repro (scratch-only, decomp read-only)

> Status note: every command below is the exact command behind §2/§3 (individually run
> and captured). The block was NOT re-executed fresh end-to-end as one script — the
> lane hit its 30-min cap at that final re-run. Treat as line-for-line assembled from
> real runs, pending one clean `bash` pass by the integrator.

```bash
#!/bin/bash   # P2-AF self-test — regenerates §2/§3 artifacts
set -u
DECOMP=/Users/scott/Hermes/pso-decomp/pso-decomp   # @ 6b38b5383
REPO=/Users/scott/Hermes/pso-metal
SC=$(mktemp -d "${TMPDIR:-/tmp}/p2af_selftest.XXXXXX"); mkdir -p "$SC/bat_cpp" "$SC/bat_mix"
COMMON="-w -fms-extensions -ffreestanding -Wno-c++11-narrowing -ferror-limit=0 -include $REPO/compat/pso_metal_compat.h -include $REPO/build/waivers/pso_assert_waivers.h -Iinclude -Iinclude/clib -Ilibs/dolphin/include -Ilibs/dolphin/include/dolphin -Isrc"
DIALECT="-I$REPO/compat/dialect/pshim -include $REPO/compat/dialect/pso_dialect_compat.h"
cd "$DECOMP"
for f in runtime/snd_synthapi.c runtime/stream.c runtime/s_data.c runtime/snd_init.c runtime/hardware.c runtime/StdReverb/reverb_fx.c runtime/synth_dbtab.c; do
  b=$(basename "$f" .c)
  clang++ -c -std=c++17 -x c++ $DIALECT $COMMON -target arm64-apple-macos11 -o "$SC/bat_cpp/$b.o" "src/musyx/$f"
  clang++ -c -std=c++17 -x c++ $DIALECT $COMMON -target arm64-apple-macos11 -o "$SC/bat_mix/$b.o" "src/musyx/$f"
done
clang -c -std=c11 -x c $COMMON -target arm64-apple-macos11 -o "$SC/bat_mix/synth_dbtab.o" src/musyx/runtime/synth_dbtab.c
printf 'void sndAuxCallbackChorus(void *p){(void)p;}\nvoid sndAuxCallbackPrepareChorus(void *p){(void)p;}\n' > "$SC/chorus_standins.c"
clang -c -std=c11 -x c -o "$SC/bat_cpp/chorus_standins.o" "$SC/chorus_standins.c"
clang -c -std=c11 -x c -o "$SC/bat_mix/chorus_standins.o" "$SC/chorus_standins.c"
"$REPO/tools/link_hygiene.sh" "$SC/bat_cpp" --musyx && echo "MODE-CPP GATE OK"
"$REPO/tools/link_hygiene.sh" "$SC/bat_mix" --musyx && echo "MODE-GUARDED-C GATE OK"
nm -g --defined-only "$SC/bat_cpp/synth_dbtab.o" | sort    # 3 unmangled _dsp* D lines = accident confirmed
```
