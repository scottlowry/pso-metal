# P2-AZ — quoted-include collision debt: anatomy, fix, measurement

Phase-3 lane AZ, 2026-11-27. Decomp `pso-decomp @6b38b5383` READ-ONLY; zero
make/ninja; every number is scratch `clang -fsyntax-only -ferror-limit=0` over
the battery-3 corpus on disk (`$TMPDIR/b2_lp64/`: 1,680 units, H0 = 162
clean / 1,518 fail — the "162 PASS" of `docs/p2-stitch-apply3.md`). Inputs:
AW `docs/p2-repin-saves-1.md` (`befe299`, 18-breakage anatomy, +72/−18,
"1,109 quoted-SDK-sibling cases remain"), `docs/p2-stitch-apply3.md` (current
pin: apply-3, header sha `c6b514035f39b633…`). Writes: `compat/collision/**`
(new, inert until flags land) + this doc only.

## 1. Binding-edge anatomy (the fact AW's numbers implied)

The era SDK header `libs/dolphin/include/dolphin/types.h` uses its OWN guard
`_DOLPHIN_TYPES_H_` and re-typedefs s32/u32/uaddr/saddr + `typedef unsigned
int uintptr_t` in era spellings. Under the port rebind
(`-Ihost/bringup/include` first + `-DPSO_PORT_LAYOUT_WAIVER`; its `types.h`
occupies `TYPES_H` with fixed-width/pointer-width spellings) the era file is
*also* processed → a 5-error `typedef redefinition with different types`
cluster (era `types.h:8,9,17,18,32`). Live: **1,136/1,680 TUs carry the
cluster at rebind-only** — AW's number, reproduced exactly.

Two resolution edges reach the era file, and they bind DIFFERENT mechanisms
(`-H` receipts in `compat/collision/receipts/`):

- **edge A — angle** `#include <dolphin/types.h>` (19 SDK-internal files,
  gx.h/dvd.h/card.h/gx/*): binds **-I order**. This is the only form AW's
  `compat/repin/dolphin/types.h` shadow can hijack (its +2 repair, angle
  "27 TUs").
- **edge B — quoted sibling** `#include "types.h"` from inside
  `libs/dolphin/include/dolphin/` (28 SDK files — mtx.h:4, os.h, db.h,
  pad.h…): binds **the including file's directory FIRST**, then -I. Receipt:
  with `-Icompat/repin -Icompat/njs -Ihost/bringup/include` all ahead of the
  base list, `mtx.h` still resolves to the era file and fires the cluster.
  TProtocol.cpp is the same edge at chain depth 6
  (protocol_types.h → mtx.h → types.h). **No -I ordering and no shadow
  outside the (read-only) SDK dir can intercept edge B** — that is exactly
  why 1,109 cases stayed after AW's angle fix (my H2FN live reproduction:
  1,109 cluster-carrying TUs, on the nose).

`include/pso/dolphin/types.h`-style per-dir shadows are useless against edge
B (the includers live under `libs/dolphin/include/dolphin/`, which is
read-only) and dangerous-as-blanket elsewhere — the per-shape shadow matrix
Option (b) suggested is **structurally impossible here; none needed**,
because the right primitive is resolution-independent: the guard macro
itself (AW §5.3 already named "SDK pre-include pin or guard macro" as the
landing gate).

## 2. Option matrix (measured on battery-3, full corpus)

| option | mechanism | cluster TUs left | notes / blast radius |
|---|---|---|---|
| (a) `-I` ordering alone | rebind-first + shadow dirs | **1,109** | edge B immune (receipt); AW H2F: +2 repairs only |
| (b) quoted-name shadow at hijack-correct dir | file placement | n/a | impossible for edge B (needs write INTO decomp SDK dir); dead |
| (c) wrapper-header rename discipline | naming policy | 1,109 | n/a — zero repo TUs include `dolphin/types.h` directly (grepped: 0 angle + 0 quote); nothing to rename |
| (d) bare `-D_DOLPHIN_TYPES_H_` | guard macro on cmd line | 0 | 257 clean @rebind-only, 1 collateral=0-new — but opaque, carries NO era tail, fails confusingly if a future TU needs `Ptr`/`BOOL` |
| **(e) `compat/collision/pin_dolphin_types.h` force-include — CHOSEN** | guard occupied at TU start, tail carried verbatim (minus libc legs) | **0** | 257 clean @rebind-only, **0 new collateral** on full stack; self-documenting; binds BOTH edges by construction |

(e) v1 shipped WITH the era `#include "libc/*.h"` legs → exactly 1 new
collateral (`TObjEneMe1CanadinLeader.cpp`: force-loaded era math.h poisons
the host math path, `ceil` vanishes — guard-name squat, measured 256/
breakage=2). v2 (shipped) DROPS the legs: battery-3 proves zero corpus
consumers need the tail through this edge (bare `-D` = 0 collateral; the
`TRUE/FALSE/BOOL/NULL/ATTRIBUTE_ALIGN` names duplicate what the rebind
mirror of `include/types.h:31-39,62` already provides — carried anyway,
identical spellings, lossless-suppression posture for out-of-corpus TUs).
257 = bare-`-D` parity **with** the tail. C-TUs: `.c` pass count identical
H2FN↔H3 (2↔2) — pin is C-safe.

## 3. Measurement table (battery-3, 1,680 units; flips = H0-fail→clean, breakage = H0-clean→fail)

| shape | flags delta over committed battery | clean | flips | breakage | cluster TUs |
|---|---|---|---|---|---|
| H0 (on disk) | — | 162 | — | — | 0 |
| H1 rebind-only | +`-DPSO_PORT_LAYOUT_WAIVER -Ihost/bringup/include` | 217 | 74 | 19 | 1,136 |
| **H1C = H1 + pin** | | **257** | **96** | **1** | **0** |
| H1D = H1 + bare -D | | 257 | 96 | 1 | 0 |
| H2FN = AW full stack (live) | +`-Icompat/repin -Icompat/njs` | 427 | 282 | 17 | 1,109 |
| **H3 = H2FN + pin** | | **1,258** | **1,097** | **1** | **0** |

- AW's "+72/−18" class, scaled to battery-3: my live H1 = +74/−19 (the 19th
  is `TAdGuideButtonGC.cpp`, which apply-3 had flipped clean — it re-breaks
  at H1 via the cluster). Pin makes rebind-only **+96/−1** (net +95 vs +55).
- Did the collision fix close AW's −16? **Yes, fully**: all 16
  stack-breakage files were edge-B cluster (repaired, 0 remain); the single
  survivor `GcrnaStream.cpp:415 undeclared OSAlloc` is the D4 facade
  coverage class (§4.8), present at H1, not collision, and outside
  `compat/**` (host lane owns `host/bringup/include/dolphin/os/OSAlloc.h`).
- Full stack: 427 → **1,258 clean** (+815 flips + 16 repairs). AW's
  documented 379 ceiling is obsoleted — 379/427 were battery-2/battery-3
  numbers taken while the cluster blocked everything downstream; the
  collision class was the dominant single blocker of the whole rebind
  program (831/1,109 edge-B files become fully clean at stack — flips 815 +
  breakage repairs 16; the other 278 carry a §4 class on top).
- Collateral (files NEWLY failing due to the pin, per-file pairwise):
  **∅** at both H1→H1C and H2FN→H3.

## 4. Residual classes NOT fixed by the pin (H3-fail breadth, 422 files)

1. `packet_classes.h` KEEP-assert cluster — 57 TUs (`PsoCommandHandler_size`)
2. NJS-handle dialect (u32↔pointer) — ~112 TUs (AR/AS2 queue)
3. `NetworkRuntime.h` `PppFrame_size_check` — 50 TUs (net C pool)
4. musyx C dialect — 26 TUs (`src/musyx/runtime/*.c`)
5. MWCC asm/decl dialect (`GXVert.h`/`OSExec.h`/`dvd.h` "expected ';'") — ~38
6. `FloorPopulation.h` row-asserts — 18 TUs (AR §4.3 gap)
7. `TBoss6Type1.h` negative arrays — 14 TUs
8. D4 `OSAlloc` facade coverage — 1 TU (`GcrnaStream.cpp`; the only
   rebind-caused failure left; host lane adds era-OSAlloc decl set to the
   facade or stubs the call)
9. long tail (asm blocks, subscript-on-int, char-create AFS asserts…) — ~68

2, 4, 5 are queue items already sequenced "behind collision debt" by
`p2-stitch-apply3` §State — they are now the binding constraint.

## 5. Recipe (paste-ready, integrator-owned flag lines)

One file, no placements needed anywhere else:
`compat/collision/pin_dolphin_types.h`.

Rebind-only shape (staged per-TU adoption rows keep working unchanged):

```
-DPSO_PORT_LAYOUT_WAIVER
-include compat/pso_metal_compat.h -include build/waivers/pso_assert_waivers.h \
-include compat/collision/pin_dolphin_types.h
-I host/bringup/include \
-I include -I include/clib -I libs/dolphin/include -I libs/dolphin/include/dolphin -I src
```

Full stack (adds AW's repin+njs shadow dirs; pin SUBSUMES
`compat/repin/dolphin/types.h` — guard idempotent, keep the shadow for
audit, measured 0 interaction):

```
-DPSO_PORT_LAYOUT_WAIVER
-include compat/pso_metal_compat.h -include build/waivers/pso_assert_waivers.h \
-include compat/collision/pin_dolphin_types.h
-I compat/repin -I compat/njs -I host/bringup/include \
-I include -I include/clib -I libs/dolphin/include -I libs/dolphin/include/dolphin -I src
```

The pin MUST come after `pso_metal_compat.h`/waivers in the -include list as
measured; it fails closed if `PSO_PORT_LAYOUT_WAIVER` is absent (the
delegated `<types.h>` hits the port header's own negative-array assert) and
is a documented silent no-op without the rebind -I.

## 6. Gate proposal (what to re-pin after apply)

1. **Cluster-zero gate** (cheap, every battery run): `grep -l 'typedef
   redefinition with different types'` over sweep err corpus must be 0.
2. **Re-record the battery**: LP64 "162 PASS" and apply-3 §State
   "215/379" assertions become 257 (rebind-only) / **1,258** (full stack) —
   this is the intended, documented jump, not a regression; re-pin whatever
   count assertions cite 162/427/379.
3. **Collateral ledger**: H0-clean→shape-fail set must equal
   {`GcrnaStream.cpp`} until the D4 facade class closes; any other name =
   new collision behavior, block.
4. Re-run D8 guard suites + `parse_gate` under the stack shape (0 waiver
   edits were needed — the pin changes parse admits, not verdicts; the 2,943
   header is untouched by this lane).

## Repro

`cache/scratch/az_probe.sh` (shape args H1/H1C/H1D/H2FN/H3),
`az_sweep.sh` (xargs -P20 over battery-3 `all.txt`), `az_analyze.py`
(pairwise diffs, cluster census), `az_edge{A,B}.cpp` (receipt TUs); sweep
outputs + `matrix.json` under `cache/scratch/az/`. Repo grep (proved at
commit): `compat/collision` appears in no build/host/data file — this dir +
this doc only; zero decomp/host/build writes.
