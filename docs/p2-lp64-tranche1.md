# P2-R — LP64 residual measurement + verdict tranche 1

Phase-2 task R. Re-scores the assert/waiver picture for the **real link target**:
Apple clang LP64 (`-target arm64-apple-macos11`). ILP32 (`arm64_32-*`) remains a
parse-probe triple only (phase0-verdict, p1-datamodel §0); the 99-waiver list
(`build/waivers/pso_assert_waivers.h`, 31 RUNTIME-WAIVE types) was fitted to ILP32
residuals and does **not** address the pointer-width class. All numbers below were
**measured 2026-11-26** by direct clang invocation (no `make`, no writes to `build/`):
`/Users/scott/.hermes/profiles/pso-decomp/cache/scratch/clangprobe/run_p2r.sh` over
`pso-decomp @ 6b38b5383` (1680 units from `find src \( -name '*.cpp' -o -name '*.c' \)`),
exact flag set = `probes/tree_compile_probe.sh` minus `-target arm64_32`
(+ `-ffreestanding -Wno-c++11-narrowing`), force-including `compat/pso_metal_compat.h`
then `build/waivers/pso_assert_waivers.h`, decomp `-Iinclude -Iinclude/clib
-Ilibs/dolphin/include -Ilibs/dolphin/include/dolphin -Isrc`.

## 1. Headline: LP64 parses 70/1680, not 1416

| Probe (both headers active) | PASS | FAIL |
|---|---:|---:|
| `arm64_32-apple-watchos7.0` (re-measured — reproduces `p1-buildsystem.md` gate) | **1416** | 264 |
| `arm64-apple-macos11` (LP64 native, **today**) | **70** | 1610 |

* **Gap = 1346 files.** Set logic: every ILP32 failure also fails at LP64
  (`comm` cross-check: 0 ILP32-only failures) — LP64 is a strict superset class.
* **Assert traffic at LP64: 1467 unique `LAYOUT_ASSERT` sites** (deduped error
  file:line; **a floor** — 1316 TU runs printed `too many errors emitted, stopping
  now [-ferror-limit=]`, clang's default 20-error cap truncating deeper sites),
  ≈26,966 negative-size error occurrences across TUs. Of these, **1444 sites are
  LP64-only** (pass at arm64_32): the pure pointer-widening (4→8 B) class — the
  ILP32 waiver list covers **0** of them (and its 99 waived names produce no LP64
  error traffic at all, being NOP'd). ILP32-residual assert sites under the current
  waiver list: 45 — consistent with 1444 + 45 ≈ 1467.
* Failing-file decomposition at LP64: **1544 files fail ONLY from layout asserts**,
  29 fail asserts + non-layout errors, 37 fail on non-layout errors only.

### Top failure buckets (LP64 native probe), 3 representative `error:line` each

| Bucket | Occurrences | Representatives |
|---|---:|---|
| LAYOUT-ASSERT negative size (named + C-form) | 26,966 | `include/pso/object/base/ObjectSetData.h:23` `'ObjectSetDataSize' declared as an array with a negative size`; `include/pso/system/NinjaTypes.h:162` `'NJS_OBJECT_size' …`; `include/pso/system/NinjaTypes.h:163` `error: array size is negative` |
| 20-error cap truncation (`too many errors emitted`) | 1,316 files | hides additional assert sites — every count above is a lower bound |
| Dolphin-SDK C-dialect header parse (`expected identifier`, `expected ';' after top level declarator`, `expected ')'`, `expected unqualified-id`) | 275 combined | `libs/dolphin/include/dolphin/os/OSExec.h:25` ×(4 variants); `libs/dolphin/include/dolphin/os/OSLink.h:27` |
| musyx struct-tag collision | 30 | `src/musyx/runtime/musyx_internal.h:13` `must use 'struct' tag to refer to type 'SoundReverb'` |
| missing includes | 8 | `-fmax-errors` header-not-found errors across musyx/dolphin units |
| PPC `asm` blocks | 12 | `src/pso/unk/text_803D9650.cpp:6`, `src/pso/math/SinCosTable.cpp:16` `expected '(' after 'asm'` |
| PPC intrinsic calls | 5 | `src/pso/system/movie/MovieFramePack.cpp:62` `no matching function for call to '__dcbt'`; `src/pso/system/SiPolling.cpp:29` `SIRegisterPollingHandler` |

## 2. Verdict tranche 1 — `data/verdict-tranche1.tsv` (200 types)

Selection: all `UNREVIEWED` rows of `data/assert-verdicts.tsv` with nonzero
include-closure leverage, ranked by **how many of the 1680 units transitively
include the declaring header** (full `#include` closure per unit over the decomp
search paths — the honest version of "git grep count of that header", since the
failures ride the `pso.h` umbrella chain; top of range: `NinjaTypes.h` 1311,
`object/base/*.h` ~1050, `PhotonBlastGauge.h` 895).

| Verdict | Count | Meaning |
|---|---:|---|
| RUNTIME-WAIVE | **80** | pointer-bearing/derived runtime objects whose asserts fail **only** at LP64 (78) + 2 named RUNTIME-WAIVE in the G/H docs; waiving is safe — bytes never leave the process |
| LOCKED-FORMAT | **4** | named in the G/H FORMAT tables → **refused FORMAT-KEEP→waive**: `ChallengeRecord` (7 LP64-fail sites!), `BattleRecord` (1), `ChatShortcut`, `TRegister` |
| UNRESOLVED | **116** | format-adjacent dirs (`network/`, `system/save|movie|Gsl*|Rlc*|Bml*|Aram|TGStream|SubtractiveRng`, `quest/`, `map/`) + `NJS_*` overlay class + zero-measured-traffic types; each row names the settling evidence |

**LP64-aware findings worth escalating:**
* `ChallengeRecord`/`BattleRecord` are **by-value members of `CharacterSaveRecord`**
  (save record, `p1-format-saves.md` §2) yet fail 8 LP64 sites — they contain a
  pointer-width-shifting member. Their PPC card bytes are pinned by these asserts;
  the fix is re-pin/handle-conversion, never a waiver. (Saves lane: the
  "0 of 46 failing asserts touch the save format" claim holds for *ILP32*; at LP64
  this tranche finds counterexamples — the claim must be re-audited for link target.)
* `NJS_OBJECT`/`NJS_MKEY_F` (leverage 1311): net-disc §2.2 says Ginja/Ninja runtime
  structs are runtime, but their `T*` sibling/child links are exactly abi.md §2's
  "cross-module pointer table loaded from data" — p1-datamodel §3 rule-1 (u32
  handles + base register) is the settlement, flagged UNRESOLVED → lane J.

**Tranche vs current waiver list delta:** the tranche enables **161 additional
LP64-failing assert-site waivers** (vs 99 ILP32-fitted names that waive *nothing*
LP64-visible), and simulation over the probe `.err` corpus shows that would move
LP64 parse from **70 → 138 files (+68)** with zero files hidden behind the error
cap. Waiving *every* measured LP64-failing type would reach **1544/1680 (92%)** —
the ceiling of what assert policy alone can do. Tranche 1 covers 80 of 829
LP64-failing types because ranking by header-include leverage front-loads the
umbrella-reachable base classes; expect tranche 2+ to be cheaper per file.

## 3. What still blocks a linked LP64 object set (honest list)

1. **749 untranced LP64-failing types** (~1306 more assert sites): tranche 2/3 over
   `enemy/`, `boss/`, `ui/` per-file headers; same mechanical rule applies, but each
   needs the same FORMAT-table refusal check.
2. **66 files never fixed by any waiver** — non-layout classes: dolphin-header C
   dialect parse errors (OSExec/OSLink `extern`+`asm` idioms), musyx
   `SoundReverb` tag collisions, PPC `asm` blocks, `__dcbt`/`SIRegisterPolling`
   intrinsics, 8 missing includes. Compat-header shims/host stubs (abi.md §6
   reserves the toolchain here), not assert waivers.
3. **LOCKED-FORMAT re-pinning**: `ChallengeRecord`, `BattleRecord` (measured LP64
   failures), plus `NJS_*` handle conversion (lane J) — must land with corrected
   host constants, asserts kept live.
4. **The value-truncation class size asserts cannot see**: 26 `(u32)this`
   launderings, 9 `*(u32*)0x800000F8` BootInfo loads, `disp[0xD0]`/`inventory[0x34C]`
   opaque blobs (p1-datamodel §1/§3, p1-format-net-disc §6) — runtime corruption
   gates, not compile errors.
5. **Endianness runtime class** (abi.md §3, net-disc §0 regime B + D1 unswapped
   `GslArchiveEntry`): every struct that parses there must pass the be-index CI
   gates before it is trusted on LE LP64.
6. **Link-phase reality**: the whole Dolphin SDK + PPC startup path is stub-replaced
   by host bringup (lane Q owns `host/bringup/`), libc/libmusyx sources for LP64,
   and `-ferror-limit=0` full-enumeration re-probe to turn every floor-count in
   this document into an exact one.

Repro: `bash cache/scratch/clangprobe/run_p2r.sh` (scratch); census helpers
`census.py`, `leverage.py`, `build_tranche2.py` alongside it. TSV schema:
`type verdict asserts lp64-fail-sites lp64-tu-traffic header-include-leverage declaring-header evidence`.
