# P1-I — Layout-assert policy: LP64 verdicts + the waiver mechanism

Tool: `tools/assert_policy.py` (stdlib-only). Regenerate everything:

```sh
python3 tools/assert_policy.py                    # full LP64 probe (~2.5 min, 20 jobs)
python3 tools/assert_policy.py --no-probe         # reuse cached probe log
python3 tools/assert_policy.py --emit-waiver-list # write the selective waiver list
```

Artifacts: `data/assert-verdicts.tsv` (committed, per-type) and
`cache/assert-probe/` (untracked: clang logs, `diag.json`, `stats.json`,
`pso_layout_waiver_list.h`).

## 1. Census (decomp @ master, `include/` + `src/`)

| quantity | count |
|---|---|
| `LAYOUT_ASSERT` call sites (incl. 23 via helper macros `CHAT_OFFSET`/`MOJA_OFFSET`/`FIREWORK_OFFSET`/`TRADE_MESSAGE_OFFSET`) | **3277** |
| … in `include/` + `src/` files | 1376 files |
| kind = `sizeof(T)==N` | 1734 |
| kind = `offsetof(T,f)==N` | 1530 |
| kind = other constant-equality (`GX_TF_I4==0`, `sizeof(PMF)==0xC`, …) | 13 |
| distinct assert names | 3272 (5 names repeat across headers — the waiver list is name-keyed, so a repeat waives/keeps both; re-run flags new names) |

(The prior "3256" figure in `docs/abi.md` predates #411 landing; grep of
`LAYOUT_ASSERT(` now yields 3258 raw lines + 23 helper expansions − 4 helper
`#define` lines = 3277 real sites.)

## 2. Which asserts fail under the real Phase-1 runtime ABI

**Method used: full per-file/per-TU compile probe under LP64 — not the
`docs/abi.md` intersection.** abi.md measured `arm64_32-apple-watchos`
(ILP32); Phase 1 runs Apple arm64 **LP64**, where 8-byte pointers/`long`
shift nearly every struct containing one, so the abi.md ~20-type list is the
wrong ground truth here. The tool compiles, with
`-fsyntax-only -ferror-limit=0 -w -fms-extensions -ffreestanding
-Wno-c++11-narrowing -include compat/pso_metal_compat.h` (i.e. Phase-0 §7
flags minus `-target arm64_32…`), all 1376 assert-bearing files **plus** every
`src/` TU (3010 units) and attributes clang diagnostics to exact assert lines.
A failed condition always surfaces as a *negative array size* error at the
macro-invocation line (`'X' declared as an array with a negative size` for
sizeof sites; bare `array size is negative` for offsetof sites — both counted;
anything else at that line = INCONCLUSIVE). Since clang recovers from typedef
errors and keeps parsing, absence of a diagnostic at an assert line in a unit
whose only complaints there are negative sizes is a true pass ("healthy file").

| status (LP64) | asserts |
|---|---|
| **FAIL** (condition evaluated false — assert fires) | **3022** (92.2%) |
| PASS | 185 |
| INCONCLUSIVE | 0 |
| UNPROBED (file never parsed cleanly: 9 headers, 70 sites — LP64 type errors *before* the assert lines; all header-adjacent to derailed contexts) | 70 |

Types: **1770** layout types asserted → **1647 types have ≥1 failing assert**,
123 types NOT-AT-FAULT. Sanity anchors: abi.md Cause-A types stay failing here
(`TObject` ✓); the MWCC 12-byte PMF records (`PlayerWindowCommand`/
`PlayerWindowQuery`, `WindowCtrlCallback.h:9-10`) FAIL as expected under LP64;
and pointer-free scalar types (`AdxEncHistory`, `AdxfLogHeader`,
`ChunkStripGrid::Vertex`) PASS — LP64 damage is dominated by pointer/`long`
width, matching abi.md's rules #1/#5/#6.

## 3. FORMAT-verdict merge (sibling lanes G/H)

Each failing type is looked up (word-boundary) in the lane docs as they land:
`docs/p1-format-saves.md` (G) and `docs/p1-format-net-disc.md` (H). A line
naming the type with `FORMAT-KEEP` → **L-P**; `RUNTIME-WAIVE` → **RW**; both
→ CONFLICT (manual review); neither/absent → **UNREVIEWED**. Non-failing
types default to NOT-AT-FAULT.

Snapshot as of this commit (net-disc present and *still being written*; saves
absent — re-run the tool after both land, no re-probe needed:
`--no-probe`; verdicts are merged at run time):

| bucket (failing types) | count |
|---|---|
| L-P (FORMAT-KEEP) | 9 — e.g. `GslArchiveEntry`, `CardSaveHeader`, `CharacterSaveRecord`, `NetworkInfoConfig`, `IPv4Header`, `TcpHdr`, `GuildCardEntry` |
| RW (RUNTIME-WAIVE) | 34 — incl. `TcpTcb`, `PppFsm`, `TcpSackSeg`, `TObject`-family save overlays |
| CONFLICT (needs manual review) | 3 — `TItemData`, `TPlyCharData`, one more |
| N/A-CONSTANT (non-type asserts) | 8 |
| **UNREVIEWED** | **1593** |
| (types with no failing assert) | 123 NOT-AT-FAULT |

Note: net-disc §"Assert-crossing types: 0" was scoped to the 43 ILP32-failing
lines; under LP64 six FORMAT-KEEP types *do* have failing asserts — which is
exactly what the gate below is for.

## 4. Proposed waiver mechanism (no decomp edits)

Decomp `include/types.h:69` defines `LAYOUT_ASSERT` **unguarded**, with the
contract *"A port, whose layouts differ, defines it empty"*. Because the
`#define` is unguarded, `-D LAYOUT_ASSERT=…` loses (types.h re-defines it,
last definition wins). The mechanism is a **port-only header that shadows
`types.h` on the include path** (all 3553 decomp includes are quoted
`"types.h"`, and the only `types.h` sits at `include/` root, so no quoted
search ever beats an earlier `-I`):

`compat/waive/types.h` (proposed; prototype tested against the decomp tree):

```c
#ifndef PSO_METAL_WAIVE_TYPES_H
#define PSO_METAL_WAIVE_TYPES_H
#include_next <types.h>              /* real decomp types.h: constants + active macro */
#ifdef PSO_PORT_LAYOUT_WAIVER
#undef LAYOUT_ASSERT
#include "pso_layout_waiver_list.h"  /* generated: #define PSO_LA_<name> 0|1 */
#define LAYOUT_ASSERT(name, cond) \
    typedef char name[(PSO_LA_##name || (cond)) ? 1 : -1]
#endif
#endif
```

Build flags: `-I compat/waive` **before** `-I <decomp>/include`, plus
`-DPSO_PORT_LAYOUT_WAIVER`. Measured on `AdxEncoder.h`: control = 6 negative
sizes; with `AdxEncoder_size` waived and `AdxEncChannel_size` kept = 5, the
exact named waiver — selective at *assert-name* granularity, no decomp edit,
no `#pragma`, no parallel tree.

**Gate: FORMAT-KEEP types are never waived — enforced three ways.**

1. *Generated list:* `--emit-waiver-list` writes `PSO_LA_<name> 1` **only**
   for asserts whose type's verdict is `RW (RUNTIME-WAIVE)`. L-P, UNREVIEWED,
   CONFLICT and N/A-CONSTANT get `0` → their asserts stay live; a waived type
   keeps its `typedef char` (still declared, just always size 1).
2. *Forcing function:* any **new** `LAYOUT_ASSERT` has no `PSO_LA_*` entry →
   `use of undeclared identifier` in the waived build (verified on `TLogo.h`).
   The waiver list cannot drift silently; CI re-runs the tool and diffs
   `data/assert-verdicts.tsv` (new FAILs outside the baseline, or baseline
   FAILs that vanished, are both red until reviewed).
3. *Audit lane:* the default `python3 tools/assert_policy.py` build has **no
   waiver** — the same probe that produced this table runs in CI against the
   live decomp, so LP64 layout regressions on L-P types remain compile-visible
   even while the port build waives RW ones.

Unwaived-but-failing is the *default state*: with 1593 UNREVIEWED failing
types, the port build stays asserted-red until the G/H lanes' verdicts cover
them (waived asserts in the generated list right now: 103 names over the 34
RW types, 0 for L-P/UNREVIEWED). The UNREVIEWED mass is itself the P1-J
worklist — LP64 pointer widening hits nearly every pointer-bearing type, so
either the port adopts an ILP32-ish target for data (abi.md's measured 3213
pass) or the review pipeline must disposition ~1.6k types; the mechanism above
works unchanged for either endgame.

## 5. Caveats

* Helper-macro sites (`CHAT_OFFSET(...)` etc.) are enumerated by name
  expansion (`MojaOffset_<field>`); their diagnostics land on the *usage* line.
* 13 `other`-kind sites are non-simple expressions (array-subscript sizeof,
  summed `offsetof` arithmetic, `DNS_*`/`GX_TF_I4` constant equalities) — they
  are probed like every other site but carry no single layout type, so their
  pseudo-type gets verdict `N/A-CONSTANT` (8 of them FAIL under LP64). The
  12-byte PMF records are *not* in this bucket — they are ordinary
  `sizeof(...)==0xC` sites on `PlayerWindowCommand`/`PlayerWindowQuery` and
  ride the normal review path.
* `UNPROBED` ≠ passing: the 9 derailed headers (`TBoss1Dragon.h`,
  `OlgaFlowActions.h`, …) must compile under LP64 before their 70 asserts can
  be classified; treat as gated-live.
* Verdict merge is line-scoped text matching over WIP lane docs — it is a
  first-pass classifier, and CONFLICT/manual review wins over silence.
