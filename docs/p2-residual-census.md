# P2-AS2 — Post-waiver residual census (battery-2, uncapped LP64)

Phase-2 lane AS2, 2026-11-27. Data-only: zero builds, zero `make/ninja`, decomp and
`build/`+`host/` READ-ONLY. Input = `$TMPDIR/b2_lp64/err/*.err` — battery-2 full-uncapped
(`-ferror-limit=0`, committed 2806-waive era header `751f077b` + `compat/pso_metal_compat.h`,
flags = `cache/scratch/probe_one.sh`). Companion data: [`data/residual-census.tsv`](../data/residual-census.tsv)
(1,519 file rows + SUMMARY rows). Method seeds: [p2-dialect-triage](p2-dialect-triage.md) (T,
class grammar), [p2-njs-repin-1](p2-njs-repin-1.md) (AR shadow), [p2-rung4-contract](p2-rung4-contract.md)
(AP `(d)` classes), [p2-quest-link-1](p2-quest-link-1.md) (AC), [p2-stitch-apply2](p2-stitch-apply2.md).

## 0. Input integrity (verified before parsing)

- `err/` holds **1,680** `.err` files. Exactly **1,519 are non-empty** and map 1:1 to
  `fail.txt` (0 missing); the **161 empty** ones are the now-passing units (`pass.txt` = 161,
  `all.txt` = 1680) — battery remnants, not data. Census population = the 1,519.
- 16 of the 1,519 are **not compile failures**: clang exits on `no such file or directory`
  (16 `src/pso/unk/TLobby_*` / `text_*` units absent from decomp `6b38b5383`+worktree —
  stale corpus list). Bucketed `other:missing-unit`, owner UPSTREAM (corpus re-pin, one-line
  relist), NOT a parse wall. Real parse wall = 1,503 TUs, 49,644 error diagnostics.

## 1. Classifier (all-classes-per-file, never first-error-only)

Per file: extract every `path:line: error:` diag (+`fatal error:`), grab the shown source
line, then class:

| class | trigger rule |
|---|---|
| layout buckets | `'Name' declared as an array with a negative size` / `array size is negative` → assert name + type from `LAYOUT_ASSERT(Name, sizeof/offsetof(TYPE…`) (P2-R `p2rLP64perttype.json` + name-prefix + type-universe fallback; **0 unmapped after fallbacks**) |
| `allow-list-pending` | fired assert's TYPE ∈ AR shadow family (NJS_OBJECT, NJS_MKEY_F/P, NJS_TEXLIST/NAME, NJS_MOTION, NJS_MDATA2, NJS_CNK_MODEL, GinjaMaterialState) |
| `layout-LOCKED-cogate` | fired assert's TYPE ∈ LOCKED = base FORMAT-KEEP ∪ resolutions-1..5 `LOCKED-FORMAT` (69) ∪ `DENY_FORMAT_ROOTS` (gen_waivers, verbatim) |
| `verdict-open` | fired assert's TYPE still UNREVIEWED/absent after resolutions (75 distinct types, 21,236 sites) |
| `other:waiver-label-gap` | TYPE labeled `RW (RUNTIME-WAIVE)` in BASE tsv (not the exact string gen_waivers matches) — keep-flagged by design; 23 sites/8 names incl. TBoss6Type1, TcpTcb |
| `dialect-SDK` | any other parse-syntax error (expected-…/unqualified-id/needs-explicit-size) — `libs/dolphin/**` dominant, plus decomp-header AT_ADDRESS sites |
| `mussyx-externC` | musyx-header/src path errors + `must use 'struct' tag` collisions (T §1: one bundle) |
| `intrinsic` | undeclared `__*`/`acos|asin|atan|fmaf`, no-matching-function `__dcbt`/`SI*PollingHandler`/MWCC-ABI |
| `orig_asm` | any msg containing `asm` (`expected '(' after 'asm'`, `… after top-level asm block`) |
| `include-graph` | `fatal error: '…' file not found` |
| `sig-drift` | conflicting types / language-linkage / redefinition / ambiguous |
| `quest-source-rewrite` | `layout-LOCKED-cogate`-owned file under `src/pso/quest/` (AC §3/§5 decomp-side re-spell ask) |

Ownership = **last-key priority** (uncapped rosters permit it; first-visible is still
carried in `first-error:line`): UPSTREAM structural > DIALECT-QUEUE > LOCKED re-pin >
verdict queue > ALLOW-LIST. A file with a dialect blocker AND layout locks belongs to the
dialect queue only because the parse error gates everything behind it… but green-arrival
needs ALL its buckets, hence buckets column lists everything.

## 2. Owning-bucket histogram (sums to 1,519)

```
1127  layout-LOCKED-cogate      (co-gate, incl. NJS + verdict-open behind it)
 173  allow-list-pending        (NJS-family asserts are the ONLY blockers)
  86  verdict-open              (UNREVIEWED-type keeps only)
  41  dialect-SDK
  24  mussyx-externC
  17  quest-source-rewrite      (quest-lane LOCKED owners)
  16  other:missing-unit
  12  sig-drift
  10  intrinsic
   9  orig_asm
   2  include-graph
   2  other:waiver-label-gap
--------
1127+173+86+41+24+17+16+12+10+9+2+2 = 1519  ✓
```

Any-bucket presence (co-membership): allow-list-pending **1,304** · layout-LOCKED-cogate
**1,183** · verdict-open **1,107** · dialect-SDK 45 · mussyx-externC 27 · waiver-label-gap
23 · missing-unit 16 · intrinsic 13 · sig-drift 12 · include-graph 11 · orig_asm 9.
Owner programs: SOURCE-REWRITE-ASK 1,144 · ALLOW-LIST 173 · UNCLASSIFIED 88 ·
DIALECT-QUEUE 87 · UPSTREAM 27.

**Reading:** the NJS pair is the widest trigger in the corpus (1,304/1,519 = 86% carry it —
AR's "~1000 TUs" inherited, now exact), but it almost never stands alone (173 only). The
real wall is the LOCKED cluster: 1,127 files need a structural re-pin that neither queue
nor allow-list can supply.

## 3. Top files per big bucket (blocking construct = fired-type × sites, heaviest first)

layout-LOCKED-cogate owners: `TTcpSocket.cpp` (100 err: DnsQuery×9, ChallengeRecord×7),
`item_create.cpp` (87: TItemWeapon×18), `TWindowSubComCard.cpp` (82: BmlResource×8),
`TObjGrass.cpp`, `TObjEneIllGill.cpp`, `TUdpSocket.cpp`, `TObjCollidable.cpp`,
`TODoorCave01.cpp` (EnemySetRecord×7), `TOContainerAncientEnemy.cpp`, `TLobby_8001D6A0.cpp`.

allow-list-pending owners: all NJS size/child/sibling + NinjaFloatKeySize clusters
(`GinjaTextureEntryCopy.cpp` 4, `TLoadingCur.cpp` 4, then 3-error NJS pair files).
172/172 are `.cpp` — no C-TU in the pool (the §4 C-ban costs nothing here).

verdict-open owners: `NetBufferPool.c`/`NetDispatcher.c` (35 each: UdpDatagram, Ipv6cp*),
`TORockJungleL01.cpp` (35: BmlResource×8), `CsnQuery.cpp` (28), `TOBoss4Rock.cpp`,
`TObjBoss1MokoHill.cpp`, `TObjNpcNgcBase.cpp`, `TBoss8Dragon_8028A8F8.cpp`.

dialect-SDK owners: `NetworkConnect.cpp` (161, `dolphin/os/OSExec.h:25` AT_ADDRESS first),
`SystemBootInit.cpp` (132), `ChunkModelDraw.cpp` (115), `eth.c` (95, `hw_regs.h` raw pins),
`gjLight.c`/`gjSystem.c`/`nuReLoadTexture.c` (85, `OSExec.h:25`), `Gcci.cpp`,
`TAdSinglePlyChrSelectGC_801FBF54.cpp`, `McFsGc.cpp`.

mussyx-externC owners: 24 × `src/musyx/runtime/*.c` (`musyx.h:161` bare `extern "C"`,
3–4 errors each) + `dsp_import.c` (source-side, 1 error).

## 4. Closest to green (≤3 remaining errors): 220 files — rung-5-adjacent cheap wins

Owner split: 171 allow-list-pending (2–3 NJS asserts) · 23 mussyx-externC (1–3) ·
16 layout-LOCKED-cogate (1–3) · 8 dialect-SDK · 1 verdict-open · 1 orig_asm.
The **9 one-error files** are the cheapest structural asks in the corpus:

| file | single blocking assert site |
|---|---|
| `src/musyx/runtime/dsp_import.c` | own source `:4` bare `extern "C"` (2-line vendor-rule fix) |
| `NetworkInfoRequests.cpp` | `NetworkInfoLoader.h:33` NetworkInfoCardFile |
| `TAdPlayerLoadGC_80076BBC.cpp`, `TAdSinglePlyChrSelectGC_8001C5E8/8001ED80/801D5DAC/802E3AE4.cpp`, `text_801A5778.cpp` | `TAdSinglePlyChrSelectGC.h:49` SystemSaveData ×1 (six files, ONE deny-root) |
| `TWindowSubComCard_80073DB8.cpp` | `QuestScriptFormat.h:22` QuestBinHeader |

## 5. Expected materialization if allow-list + dialect queue land (the honest number)

Census predicate (ONLY buckets from those two programs): **221 files** = 172 allow-list-only
+ 36 dialect-only + 13 allow∩dialect. Measured, scratch probes, this lane:

- T dialect recipe (`compat/dialect/pshim` + `pso_dialect_compat.h`, musyx `.c` in `-x c++`):
  **36/36 dialect-only files flip FAIL→PASS.**
- AR allow-list spec (`-DPSO_PORT_LAYOUT_WAIVER -Ihost/bringup/include` + `-Icompat/njs`
  earliest, committed force-includes): **114/172 allow-only flip**; the 58 stragglers break on
  **port-rebind vs `libs/dolphin/include/dolphin/types.h` typedef redefinitions** (46) or
  rebind-surfaced C++ errors (12) — the landing lane owes a rebind/ordering fix (AR's 38-TU
  control set never touched that path).
- 13 mixed allow∩dialect with BOTH shadows: 0/13 flip (each still hits rebind/lock residue).

→ **Projected LP64 materialization: 161 → ≈311 (+150 measured)**, i.e. these two programs
buy ~10% of the 1,519. The remaining ~1,369 are gated by LOCKED re-pins (SOURCE-REWRITE-ASK
1,144 incl. the NJS-vs-§4 staged-list gap: the committed spec admits 6 TUs, not 173 —
widening it is an AR/AV decision, and every allow-list flip also needs the rebind fix).
**Wall is dominated by structural re-pin, not queue** — but see §6.2: it is not *purely*
structural either.

## 6. Surprises / corrections to inherited claims

1. **T's 16 waiver-harness tripwires are gone** — the apply-2 regen emitted flags for the
   `NET_DISPATCH_OFFSET` ##-names; the class retired itself.
2. **The verdict queue is NOT finished:** 1,107 files (73%) still fire ≥1 UNREVIEWED-type
   keep; 75 types unadjudicated; top blockers `ChallengeRecord` (903 files — and AC §3
   labels it LOCKED-FORMAT while base+resolutions say UNREVIEWED: membership contradiction,
   verdict lane must settle it), `BmlResource/Motion/ModelEntry` (413 each — AP's `verdict-open`
   BmlModelEntry flag generalizes), `CsnHit/CsnContact` (253). Adjudicating these ~10 types
   doesn't green files (LOCKED co-gates behind them) but de-risks every re-pin estimate.
3. **16 corpus-list ghosts** (missing units) inflate the wall; arm64_32 `make parse` counts
   them as errors too — reconcile the unit list with the tree.
4. **23 sites are a label-format artifact** (`RW (RUNTIME-WAIVE)` in base ≠ exact
   `RUNTIME-WAIVE`): either normalize `assert-verdicts.tsv` or widen gen_waivers matching —
   then 2 owning files (TBoss6Type1.cpp, NetTcpState consumers) move buckets.
5. **AR's 17-chain cross-check holds in the battery era:** 31/32 in-fail AR cpp TUs carry
   allow-list-pending; NJS pair still fires for every non-shadowed includer.
6. `ItemDataSize`→TItemData is the co-equal giant, not a minor cluster: TItemData keeps fire
   in **1,106** LOCKED-present files (vs ChallengeRecord 903 verdict-open, SystemSaveData 236)
   — any re-pin pricing must count both roots.

## 7. Repro / method caveats

Scratch-only scripts: `cache/scratch/as2_census.py` (pass-1 classifier → `as2_rows.json`),
`as2_rollup.py` (ownership/projections → `as2_rollup.json`), `as2_h2_probe.sh`,
`as2_dialect_probe.sh`, `as2_joint_probe.sh` (+ `as2_h2/`, `as2_dialect/` logs). Caveats:
assert→type attribution is name/cond-based (uncapped sites, exact macro text — no cap
hiding per T's lesson); `dialect-SDK` lumps SDK-header and decomp-header AT_ADDRESS sites
per T §2 (same fix family); cascade artifacts are possible inside a class (a broken vendor
declaration can produce downstream layout hits) — bucket sets, not counts, are the unit of
trust. Waived-assert re-materialization (if re-pin changes widths) is out of scope — this is
a census of the committed era `751f077b`, re-record only.
