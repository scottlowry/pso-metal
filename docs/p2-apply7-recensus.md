# P2-RC apply-7 integrity RE-CENSUS (independent audit sweep; zero repo builds)

Fleet wave-3 lane RC, card t_a59c215e, 2026-11-27. Decomp `pso-decomp @6b38b5383`
READ-ONLY (rev re-verified post-sweep, zero files touched). Zero `make`/`ninja`/
`gen_waivers` — four FULL scratch sweeps `-P6` (fresh runner `cache/scratch/rc/rc_probe.sh`
+ `rc_sweep.sh`, flags transcribed from `build/Makefile` apply-7 @9c5aba7, never from
`build/parse`). Waiver pin `d4382fcd0659357f…` byte-verified at intake AND after the
sweeps, never re-hashed up. Corpus = battery-3 `all.txt` (1,680; set-identical to
`build/units.txt`, diff 0). Official `build/parse/` was mid watchos battery
(FAIL=75, target=arm64_32-apple-watchos7.0) all during the lane — per shared law the
scratch sweep replaces it as the RC ledger.

**VERDICT: no real regression, no AR §5 breach.** PASS = 1,163, collateral vs H0 = ∅,
LOST = ∅, gained = exactly 813, all 808 rows attribute fully. Three accounting drifts
below (never smoothed).

## 0. Runner sanity + shapes

Every shape dir recorded exactly (pass+fail)=1,680, disjoint, union=corpus (a first
driver invocation had xargs arg-order swapped — `bad shape <unit>` ×4, all four shapes
zero-work; caught by the gate, driver fixed with `-I RCUNIT`, dirs wiped, re-swept:
`sweep2.log`, 0 bad-shape lines. Nothing from run 1 was read into any number here).

| shape | = | flags | PASS | FAIL |
|---|---|---|---|---|
| H0 | true-LP64 base | CORE (`-include` compat, waivers) + SDK -I, `-target arm64-apple-macos12.0 -ferror-limit=0`, no rebind/pin/njs; musyx as plain C | **167** | 1,513 |
| A6E | apply-6 reconstruct | H0 + `-DPSO_PORT_LAYOUT_WAIVER` + pin (3rd `-include`) + `-I` net repin bringup; `-Icompat/njs` FIRST on stage-1+2 (5 rows) only; musyx `clang++ -x c++ -Icompat/musyx` | **350** | 1,330 |
| A7 | official apply-7 | A6E + `-Icompat/facade -Icompat/dialect/pshim` (before bringup) + `-Icompat/njs` FIRST on all 813 allow-listed `.cpp` rows | **1,163** | **517** |
| A7N | attribution base | A7 verbatim with ZERO `-Icompat/njs` anywhere | 192 | 1,488 |

A6E reproduces the apply-6 official 350/1,330 exactly (WT waiver delta LP64-inert).
A7 reproduces the claimed 1,163/517 exactly. `.c` units NEVER receive `-Icompat/njs`
in any shape (Makefile case-guard mirrored).

## 1. Core claims, by set-diff (python, `rc_analyze.py`)

- **PASS 1,163** CONFIRMED independently. **collateral (H0-clean → A7-fail) = ∅** —
  both against my H0@macos12.0 (167) and the saved apply-6-era `pre_adopt/h0/pass.txt`
  (162). The GcrnaStream era-collateral is dead: it is H0-clean AND A7-pass, err 0 B.
- **LOST (A6E-pass → A7-fail) = ∅** — zero corpus-wide regressions apply-6→apply-7. CONFIRMED.
- **GAINED (A6E-fail → A7-pass) = 813 exactly = 808 stage-3 ∪ five named**:
  `mdm.c, NetworkInfoLoader.cpp, McFsGc.cpp, GcrnaStream.cpp, KBDXfer.c` (full paths
  `src/pso/network/mdm.c`, `src/pso/system/save/McFsGc.cpp`) — matches the stitch §46
  bundle row one-for-one. stage3∩gained = 808/808.
- H0 era-drift (not a regression): my H0 = 167 = 162 + 5 network `.c` rows
  (`NetAuthSecrets.c NetPppoe.c NetworkMessageTransport.c NetworkPacketRuntime.c
  PppVjCompress.c`) that the waiver-header growth (2,943-era → d4382fcd) now admits at
  plain LP64. All 167 still pass at A7.

## 2. 808-row ADOPT-NOW attribution

Predicate per row (all four, python-verified over all 808): **allow-list row present ∧
FLIP (fail at A7N ∧ pass at A7, i.e. the `-Icompat/njs` flag is load-bearing for the
row) ∧ `.cpp` ∧ A7 err file 0 bytes.**

**808/808 hold. Zero exceptions: zero rows flip without a present allow-list line, zero
`.c` rows (allow-list = 813 rows, 813 `.cpp`, 0 `.c`), zero 808-row fails at A7, zero
non-0-byte receipts.** The 3 allow-listed failing rows are stage-1, not stage-3: the
NJS `subscript-on-int` trio `GinjaMaterial/GinjaTextureTable/psoLoadTexture.cpp`
(documented queue rows, TL §3 + NA §3). Allow-list ∩ gated = {BmlResource,
TCMCharacter} — live, parse-clean at A7, ledger-TAGGED gated, exactly as stitch §1
claims; no rollback claim touched.

## 3. Residual recount vs docs' claims (promise-vs-real, unsmoothed)

FAIL(A7) = 517. Class ledgers: NA `p2-njs-adopt-1.md` §3 (169) / §4 (260) / §5 (83),
MX named 2, TL `p2-tail-1.md` "unattributed after exclusions = 19".

| class | promised | real @ A7 | drift / detail |
|---|---|---|---|
| NJS adapter-queue | 169 | **169** | all still fail — 166 non-allow-listed (no `-Icompat/njs`), 3 are the live stage-1 trio (`GinjaMaterial/GinjaTextureTable/psoLoadTexture.cpp`) failing body-level dialect EVEN WITH njs (NA §3 known; the trio ∈ queue∩allow-list) |
| runtime-gated | 260 | **258** | −2: `BmlResource.cpp`, `TCMCharacter.cpp` are allow-listed live → pass (documented tag regime) |
| other-class (not NJS) | 83 | **80** | −3: `GcrnaStream.cpp`, `NetworkInfoLoader.cpp`, `McFsGc.cpp` repaired by the facade/pshim bundle (they are 3 of the named +5) |
| musyx | 2 | **2** | named stable: `Chorus/chorus_fx.c`, `StdReverb/reverb.c` |
| tail | 19 | **18** | TL doc is INTERNALLY INCONSISTENT: §1 buckets sum 16, §3 table lists 16 rows, headline says 19 (−3 = `gjLight.c`, `nuReLoadTexture.c` untabled + one count error; `gjSystem.c` is named in §4 blocked-rows). Of the 16 table rows: `KBDXfer.c` repaired (pshim), trio 3 counted in queue above, 7 counted in other above → 5 net `.c` tail rows here (`eth.c NetApi.c NetDns.c PppBuffer.c PppFsm.c`). `eth.c` first-error is now `va_list` at era `os.h:40` (hw_regs wall fixed, NEXT wall fires). |
| unattributed anywhere | 0 | **3** | `gjLight.c` + `nuReLoadTexture.c`: `unknown type name 'va_list'` @ era `os.h:40`; `gjSystem.c`: `NJS_OBJECT_size` negative-array wall. ALL THREE failed at H0 and A6E too — pre-existing, not apply-7 collateral; they exist only as a §3-class ledger gap. |

Partition check: 169+258+80+2+5+3 = **517 = FAIL total** (disjoint partition, python).
The naive 533 = sum-of-headline-claims over-counts via queue∩TL=3, other∩TL=7, the 2
gated-live rows, the 3 bundle-repaired rows, and TL's own 19-vs-16 arithmetic — no
row-count regression anywhere.

## 4. Anomalies (ledger hygiene only; zero verdict anomalies)

1. TL §3/§1 internal count inconsistency 19 vs 16 (recount here: 16 table rows, 15
   failing at A7; the untabled 3-row gap = the `.c` trio `gjLight.c`/`gjSystem.c`/
   `nuReLoadTexture.c` — `gjSystem.c` named in §4 blocked-rows, the other two named
   nowhere in TL — all three fail at A7 → tail-real **18**).
2. NA §5 other-83 stale by 3 post-bundle (repaired rows still in the list).
3. H0 era: 162 (battery-3) vs 167 (current waiver era) — re-pin the H0 ledger, or the
   next lane's collateral math silently shifts by 5.

## Repro

`cache/scratch/rc/` — `rc_probe.sh` (shapes H0/A6E/A7/A7N), `rc_sweep.sh` (`-P6`,
1,680-gate), `rc_analyze.py` (all set-math), `lists.json` (808/169/260/83 extracted
from repo docs by python), `sweep.log` (broken run 1, kept), `sweep2.log` +
`{H0,A6E,A7,A7N}/{pass,fail,err}/`. Waiver pin read-only; decomp untouched; repo
writes = this doc only.


---
**ERRATUM (integrator, apply-9, 2026-11-27 — RC2 docs/p2-rc2-recensus.md §4):**
the certified A7N column was zero-REBIND, not zero-njs (rc_probe.sh case gap:
RB set only for A7|A6E). A7N=192 = H0 167 + 25 musyx rows. Wave-3 §2's FLIP
predicate therefore proved the rebind regime load-bearing, NOT the -Icompat/njs
flag in isolation. The 808/808 ADOPT-NOW conclusion itself STANDS — re-proven
by RC2 against a corrected A8N shape. Ledger math from this doc onward keys
the corrected H0=167/A8N definitions (fleet/receipts/apply7-rc/H0-CONTROL.md).
