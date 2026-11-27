# P2-D4 — era OSAlloc + OSTime declaration supply for the rebind facade world

Lane D4 (card `t_075ab4e1`), fleet wave-2 2026-11-27. Decomp `pso-decomp
@6b38b5383` READ-ONLY; zero repo builds — every number is a scratch
`clang(-++) -fsyntax-only -ferror-limit=0` probe replicating the official
`build/Makefile` parse-lp64 token order verbatim (`-target
arm64-apple-macos12.0`; corpus receipts `cache/scratch/d4/`, `cache/scratch/d4b/`).
Waiver header intake sha `495cc91a…` (pin contents `495cc91a5b9694c4…`),
re-hashed post-sweep — still the 495cc91a era; zero waiver edits; official
`build/parse/summary.txt` verified `target=arm64-apple-macos12.0` (350 PASS)
before trusting its err/ files.

## 1. The two classes, mechanism-proved

**(a) GcrnaStream collateral — era OSAlloc.h macro tail is not in the facade
world.** `OSAlloc`/`OSFree` are not functions: era
`libs/dolphin/include/dolphin/os/OSAlloc.h:25-26` defines them as macros over
`OSAllocFromHeap(__OSCurrHeap, size)`. Under the rebind, `<dolphin/os/OSAlloc.h>`
(GcrnaStream.cpp:6; era os.h:5) resolves to the host-lane shadow
`host/bringup/include/dolphin/os/OSAlloc.h`, which ships **only**
`pso_bringup.h` — the substrate declares the six heap functions
(pso_bringup.h:65-72) but carries no macros → `GcrnaStream.cpp:415:44:
use of undeclared identifier 'OSAlloc'`, the sole collateral of
docs/p2-collision-debt.md §3's ledger (§4.8 class 8, "the only rebind-caused
failure left"). Six era declarations (OSAlloc.h:13,18,19,21,22,23) also have
no substrate twin.

**(b) OSTime.h:15 era-vs-facade conflict.** Era os.h:5 pulls the bringup
OSAlloc.h shadow → `pso_bringup.h:60 u64 OSGetTime(void)`; era os.h:22 then
pulls the **era** OSTime.h (no bringup shadow exists) whose `:15
OSTime OSGetTime(void)` with `:12 typedef s64 OSTime` collides →
`conflicting types for 'OSGetTime'`. Official LP64 err/ census: **12 TUs
carry the collision** (9 with it as FIRST error), incl. the card-named `.c`
set `mdm.c / gjLight.c / nuReLoadTexture.c`; `gjSystem.c` hits the §4.2
`NinjaTypes.h:162` assert first and carries OSTime.h:15 as a secondary
error (docs/p2-mwccasm-1.md §4 lists it as OSTime-first only under the
`-Icompat/njs`-sprayed sweep shape, which the law forbids for `.c` — noted
honestly).

## 2. Supply (compat/facade/**, INERT — `-I`-path-only, never `-include`)

    compat/facade/dolphin/os/OSAlloc.h   bringup-shadow pattern: re-exports
                                         pso_bringup.h + era-verbatim orphan
                                         decls (6, line-cited) + the
                                         OSAlloc/OSFree macro pair (era :25-26)
    compat/facade/dolphin/os/OSTime.h    era decl set: OSTime/OSTick typedefs
                                         (:12/:13 verbatim), __OSGetSystemTime,
                                         OSCalendarTime (:19-31 verbatim),
                                         OSCalendarTimeToTicks/OSTicksToCalendar-
                                         Time (:33/:34), __OSSystemTime,
                                         OSDiffTick, constants (:36-47); occupies
                                         era guard OS_TIME_H. SELF-CONTAINED:
                                         includes "types.h" (era :3 edge), NOT
                                         pso_bringup.h — measured necessity, §4.

Declarations only, zero definitions, zero reinterpretation of host structs
(implementations stay `osalloc_era.c`/`osinit.c`, host seat). Signatures are
era-line-cited in comments; the ONE deliberate deviation: OSTime.h facade
re-declares `OSGetTime`/`OSGetTick` with the substrate signatures
(`u64`/`u32`, pso_bringup.h:60-61) so that either inclusion order is a legal
re-declaration — era `OSTime`-typed `OSGetTime` is the exact collision being
ended. `OSTime` itself keeps era `s64` (int64_t on LP64, layout-identical).

## 3. Measurement (same-turn d4b sweep; O = official shape, OF-A = facade
spliced AFTER host/bringup/include (card's literal position), OF-B = facade
BEFORE bringup (measured ask))

| TU (official first-fail) | O | OF-A | OF-B |
|---|---|---|---|
| `src/pso/audio/GcrnaStream.cpp` | OSAlloc undeclared :415 (1 err) | **unchanged — dead** | **CLEAN (0 err)** |
| `src/pso/network/mdm.c` | OSTime.h:15 (20 err) | collision gone → hw_regs.h:4 (19) | same (19) |
| `src/pso/render/gjLight.c` | OSTime.h:15 (23) | → hw_regs.h:4 (22) | same (22) |
| `src/pso/render/nuReLoadTexture.c` | OSTime.h:15 (23) | → hw_regs.h:4 (22) | same (22) |
| `src/pso/render/gjSystem.c` | §4.2 assert first-fail (23) | §4.2 unchanged (22; OSTime secondary gone) | same (22) |
| `src/pso/network/NetworkConnect.cpp` | OSTime.h:15 (23) | → hw_regs.h:4 (22) | same (22) |
| `src/pso/network/NetworkInfoLoader.cpp` | OSTime.h:15 (4) | → repin dvd.h:36/51 DVD-struct redef, PRE-EXISTING at O (3) | same (3) |
| `src/pso/network/eth.c` | hw_regs.h:4 first-fail (22) | hw_regs (21; OSTime gone) | same (21) |
| `src/pso/render/ChunkModelDraw.cpp` | §4.2 assert (37) | §4.2 (36) | same (36) |
| `src/pso/render/GinjaDrawUtil.cpp` | OSTime.h:15 (27) | → hw_regs.h:4 (26) | same (26) |
| `src/pso/system/movie/Gcci.cpp` | OSTime.h:15 (23) | → hw_regs.h:4 (22) | same (22) |
| `src/pso/system/save/McFsGc.cpp` | OSTime.h:15 (22) | → hw_regs.h:4 (21) | same (21) |
| `src/pso/ui/keyboard/SystemBootInit.cpp` | OSTime.h:15 (25) | → hw_regs.h:4 (24) | same (24) |

Collision cleared **12/12** TUs under both splice orders (python err-set pass
over official `build/parse/err`: 12 carry `OSTime.h:15 … OSGetTime`). Next
residuals, honestly enumerated: 9 × era `hw_regs.h:4` raw-colon pin (§4.9
tail; P2-T candidate supply `compat/dialect/pshim/dolphin/hw_regs.h` exists,
own `-I` ask — NOT D4 remit); `gjSystem.c`/`ChunkModelDraw.cpp` stay §4.2;
`NetworkInfoLoader.cpp` pre-existing repin-dvd.h struct redef; `eth.c` was
hw_regs-first already. The 4 `.c` victims are **not** fully clean after this
supply — the OSTime collision is the class D4 closes; the wall behind it is
class §4.9/§4.2.

**Clean-set collateral gate (the ask's risk case):** all 350 official-PASS
units re-run under OF-B → 325 pass + 25 "fail" that are exactly the
`src/musyx/runtime/*.c` files compiled in plain-C instead of their official
`MUSYX_CXXFLAGS` shape (runner artifact). Those 26 musyx units re-run in the
faithful shape + facade (OMF): **26/26 PASS**. Net true collateral **0**;
GcrnaStream flips +1. Ledger stays: `{GcrnaStream}` → closed by this supply.

## 4. Why OF-B (facade BEFORE bringup) is the ask, and why OSTime.h is self-contained

- OF-A (card's literal "splice after bringup/include") provably cannot fix
  class (a): `host/bringup/include/dolphin/os/OSAlloc.h` already exists and
  wins the lookup; a facade shadow of the same path is dead code there. OF-A
  does fix class (b) only.
- OF-B is safe for (a) because the facade OSAlloc.h re-exports
  `pso_bringup.h` first (superset of the bringup shadow's entire content),
  then adds the era tail — measured clean across the whole 350-unit pass set.
- v1 of the facade OSTime.h DID re-export `pso_bringup.h` (bringup pattern);
  the first OF-B clean-set sweep caught real collateral: official-clean
  `src/pso/system/save/McFsGcRequests.cpp` broke — its chain reached OSTime.h
  WITHOUT the substrate, and the re-export injected the substrate's
  `DVDCommandBlock/DVDFileInfo` structs, redefining `compat/repin/dvd.h:36/51`
  (receipt `cache/scratch/d4/OF-B/err/src_pso_system_save_McFsGcRequests.cpp.err`,
  kept). v2 (self-contained, §2) removes it: McFsGcRequests passes again at
  OF-B, collision results unchanged. Lesson for the facade family: re-export
  the substrate ONLY where the shadowed path IS already a substrate edge
  (bringup OSAlloc.h); OSTime.h is not.
- Static asserts: none added, none needed — no new struct with layout risk.
  `OSCalendarTime` is the era 10×int verbatim (40 B, OSTime.h:19-31); era
  host-side layout pins live in the waiver/pin system, untouched.

## 5. Paste-ready `-I` ask (integrator lands; zero -I lines added by D4)

Rebind-only adoption rows — insert `-I$REPO/compat/facade` BEFORE
`-I host/bringup/include` (MA §5 rebind-only block otherwise unchanged):

```
-DPSO_PORT_LAYOUT_WAIVER
-include compat/pso_metal_compat.h -include build/waivers/pso_assert_waivers.h -include compat/collision/pin_dolphin_types.h
-I compat/net -I compat/repin -I compat/facade -I host/bringup/include \
-I include -I include/clib -I libs/dolphin/include -I libs/dolphin/include/dolphin -I src
```

Full-stack (AZ §5 / MA §5): same splice position — `-I compat/facade`
between `-I compat/repin` and `-I host/bringup/include` (`compat/njs` stays
per-TU C++-allowlist; none of the 13 D4 probe units is on the allowlist, so
the row above is exactly what they ran). Musyx rows: splice identically into
`MUSYX_CXXFLAGS` (measured 26/26 at OMF).

## 6. Gate + repro

1. Collision gate: `grep -l "OSTime.h:15.*conflicting types for 'OSGetTime'"`
   over the adopted err corpus must be 0 (D4 sweep: 0/12 remain).
2. Collateral gate: H0/official-clean → shape-fail delta must be ∅ under
   OF-B (measured ∅ after the musyx-shape correction; pre-fix v1 caught
   McFsGcRequests — §4).
3. Repro: `cache/scratch/d4b/d4_probe.sh` (shapes O/OF-A/OF-B/OMF, fresh full
   file), `victims.txt` (13), `official_pass.txt` (350), per-file err/ +
   pass/fail lists, `d4b_counts.json` (pairwise tally); stale v1 receipts kept
   under `cache/scratch/d4/`.
4. Inertness: `grep -rn 'compat/facade' build/ host/ data/` = 0 hits (this dir
   + this doc only). Zero host/ build/ data/ writes; no waiver regen.
