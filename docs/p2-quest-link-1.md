# P2-AC — Quest-file LP64 blocker map + safe-fix prototype

Phase-2 task AC. Single-TU deep map of `src/pso/quest/QuestOpcodeHandlers.cpp`
(`pso-decomp @ 6b38b5383`, READ-ONLY, zero builds — clang `-fsyntax-only` probes in
`$TMPDIR/p2ac_probe/` only). Companion data: [`data/quest-blockers.tsv`](../data/quest-blockers.tsv)
(487 rows, one per distinct error site, columns `error|first-line|class|count|fix-vehicle`).
Method seeds: [p2-dialect-triage](p2-dialect-triage.md) (T),
[p2-link-deps](p2-link-deps.md) (U), [p2-lp64-tranche1](p2-lp64-tranche1.md) (R).
Resumed after the 2026-11-26 storm; the 487-site baseline inventory is the predecessor
lane's (deleg_8ae9ba57) uncapped probe, integrity re-verified row-for-row this lane.

## 1. Baseline blocker map (uncapped, compat+committed-waiver headers)

`clang++ -fsyntax-only -std=c++17 -ferror-limit=0` with the P2-R flag set
(`-include compat/pso_metal_compat.h -include build/waivers/pso_assert_waivers.h`):
**exit 1, 487 error sites, 487 diag lines** (every site carries exactly one diag).

| dimension | split |
|---|---|
| class | layout-assert 486 · other 1 (`conflicting types for 'read_byte_advance__6TLobbyFv'` — sigdrift vs the `pf2_p0` namespace decl at QuestOpcodeHandlers.cpp:214 vs def :484) |
| file locus | 486/487 errors land in **included headers**, not the TU body — top dirs: `player/` 53, `boss/dragon/` 43, `quest/challenge/` 24, `system/` 20, `map/` 18, `ui/window/` 17, `network/protocol/` 16 |
| sub-idiom | 43 sites are `DRAGON_OFFSET(member,0x…)` macro-generated asserts in `TBoss1Dragon.h:268` (all one type, `TBoss1Dragon`) |

## 2. Safe interventions (measured before → after)

Only two intervention classes are safe per lane law: waiver-stitch restricted to types
marked RUNTIME-WAIVE in the verdict files, and T's dialect shadows. LOCKED/UNRESOLVED
rows are findings, not fixes; deny-list roots (`DENY_FORMAT_ROOTS`) stay KEEP regardless.

| # | intervention | before | after | exit | note |
|---|---|---:|---:|---|---|
| I1 | ad-hoc waiver header regenerated over `merged-verdicts` (base `assert-verdicts.tsv` verdict-col(3) overlaid by `verdict-tranche1-3.tsv`: 734 RW types, 0 conflicts; gen_waivers waived **1435/3317** asserts) | 487 | **272** | 1 | −215 sites = every site whose type is tranche-RW. Re-measured this lane, byte-identical error set to the predecessor's merged probe. |
| I2 | `compat/dialect` shadow set (`-I compat/dialect/pshim -include pso_dialect_compat.h`) on top of I1 | 272 | **272** | 1 | **zero delta** — this TU carries no SDK-dialect/intrinsic blockers (all residual is LP64 layout); the quest file needs no shadow, and the shadow costs it nothing. |

No third safe intervention exists. **Projected residual with all safe fixes applied:
272 sites / 106 distinct types, exit 1.** Nowhere near 3 fixes of PASS, so the
task-step-(3) full scratch assembly was skipped by rule; parse verdicts above are the
hard exit codes.

## 3. Residual composition (the 272)

Effective verdict per site (merged-verdict truth via `gen_waivers.read_verdicts` + deny list):

| effective verdict | sites | vehicle | pipeline destination |
|---|---:|---|---|
| UNRESOLVED | 138 | verdicts-needed | verdict lane (AD, `data/verdict-resolutions-1.tsv`) |
| UNREVIEWED | 104 | verdicts-needed | verdict lane (unreviewed base rows; tranches 4+ or targeted review) |
| DENY/LOCKED (deny-format-roots) | 14 | source-rewrite | decomp-side: assert re-spell or ILP32-only gate; NOT waivable |
| LOCKED-FORMAT | 14 | source-rewrite | decomp-side, same |
| FORMAT-KEEP | 1 | source-rewrite | decomp-side |
| source-conflict (sigdrift) | 1 | source-rewrite | decomp-side: reconcile `read_byte_advance__6TLobbyFv` def vs namespace redecl |
| ~~RUNTIME-WAIVE~~ | ~~215~~ | ~~waiver-list~~ | cleared by I1 |

Top residual types (sites): TBoss1Dragon 43 (UNREVIEWED), TObjPlayer 30 (UNRESOLVED),
TItemMag 8, ChallengeRecord 7 (LOCKED-FORMAT), CharacterSaveRecord 7 (DENY),
EnemySetRecord 7, TBoss4DarkFalz 6, TWindowSubStatusDisp 6, TChallenge4PResult 6,
TChallenge4PItemSelect 6 — full per-site table in `data/quest-blockers.tsv`.

**Not lockable by its own waives:** this file is *not* waiver-its-way-able. The safe
ceiling is 215/487 (44%); the remaining 272 need verdict decisions (242) and decomp
source edits (30, of which 29 are locked-format asserts — the same class the phase-0
doctrine says must be re-spelled, not waived).

## 4. gen_waivers stitch — landing I1 for real

I1 was prototyped against a scratch-generated header. Pipeline destination is
**gen_waivers stitch**, i.e. integrator-side verdict merge, then regeneration:

1. Overlay tranche verdicts into `data/assert-verdicts.tsv` **verdict column index 3**
   (the predecessor's first merge wrote index 1 — wrong column, silently no-op; assert
   the base row count and that RW-label counts change before trusting any regen).
2. Re-run `build/gen_waivers.py` → expect `ACTIVE — waived 1435/3317`, 734 RW types,
   **0 conflicts**; deny roots stay KEEP by the generator's own guard (verified).
3. The committed `build/waivers/pso_assert_waivers.h` currently fits ILP32 (31 RW
   types, 99 names) — this stitch is exactly the tranche→LP64 propagation R queued.

## 5. Next-lane recipe (paste-ready)

```bash
SC=${TMPDIR:-/tmp}/p2ac_probe; D=/Users/scott/Hermes/pso-decomp/pso-decomp
R=/Users/scott/Hermes/pso-metal; F=src/pso/quest/QuestOpcodeHandlers.cpp
COMMON="-w -fms-extensions -ffreestanding -Wno-c++11-narrowing -ferror-limit=0 \
  -include $R/compat/pso_metal_compat.h \
  -include $R/build/waivers/pso_assert_waivers.h \
  -Iinclude -Iinclude/clib -Ilibs/dolphin/include -Ilibs/dolphin/include/dolphin -Isrc"
cd "$D"
clang++ -fsyntax-only -std=c++17 $COMMON "$F"           # expect exit 1, 487 errors
# after §4 stitch lands (regenerated committed header):
clang++ -fsyntax-only -std=c++17 $COMMON "$F"           # expect exit 1, 272 errors
```

Verdict-pick order for the AD lane (sites cleared per RW verdict, all currently
UNRESOLVED/UNREVIEWED — top 10 adjudicable types clear 119/272): TBoss1Dragon 43,
TObjPlayer 30, TItemMag 8, EnemySetRecord 7, TBoss4DarkFalz 6, TWindowSubStatusDisp 6,
TChallenge4PResult 6, TChallenge4PItemSelect 6, BmlResourceDescriptor 5,
FloorObjectSetRow 4. TBoss1Dragon is pure `DRAGON_OFFSET` macro asserts (`offsetof ==
literal`) — one verdict over the whole class collapses 43 sites.
Decomp-side worklist (30 sites, source lane, not this fleet): ChallengeRecord.h :43-49,
CharacterSaveRecord/SymbolChatPreset/TItemData/SystemSaveData/BankItemRecord deny rows,
TItemData.h :151-153, PlayerAttackParams.h fmt, plus the `read_byte_advance__6TLobbyFv`
signature reconcile.

## 6. Link value (cross-check vs `data/link-deps.tsv`)

This TU is *the* Phase-2 link chokepoint: row-join on the `source-objects` column gives
**520 of 570 link-deps rows attributed to quest objects** (518 →
`src_pso_quest_QuestOpcodeTables.o`, 2 → `src_pso_quest_TQuestThreadBoard.o`), all
`game-internal-unresolved`; definition-attribution per U is **519 of the 560** defined
in `QuestOpcodeHandlers.cpp` itself. The TQuestThreadBoard-attributed pair
(`_quest_thread_board`, `__ZN7TObject17queue_destructionEv`) grep-checks as NOT defined
in any `src/pso/quest/*.cpp` (ref sites live in `ui/city/`) — that is U's 520-vs-519
delta, resolve in the verdict/source lane, not here. Spot-
verified this lane: `qst_getargs_*` and `is_message_source_exhausted` defs sit in the TU.

Unlocked-name math: safe fixes alone (I1+I2) leave the TU at exit 1 → **0 of the 519
names unlocked**; the lane does not move link even once. The 91% gap pool stays gated
behind §3: verdicts on 106 residual types (242 sites) + 30 decomp source edits. Every
RW verdict in §5's order directly prices against these names — e.g. TBoss1Dragon alone
removes 43/272 parse sites standing between the TU and emitting all 519 defs.
