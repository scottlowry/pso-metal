# P2-WT — waiver-tail 1: duplicate waived-name collisions, mechanism fix (proposal)

Lane WT (zero-build), wave-2 2026-11-27, card `t_889babbb`.
Decomp @ `6b38b5383` (read-only). Official corpus: `build/parse/` summary
`TOTAL=1680 FAIL=1330 PASS=350 (target=arm64-apple-macos12.0)` (macos era, not
mid-run; re-read same-turn as every number below).

**Header-era vigilance (D8):** waiver header sha256 at intake AND after every
gate run below = `495cc91a5b9694c4c0e4706e453821dcf62c509bc4e610ac9c18673f3c5d061b`
(== `host/bringup/waiver-header.sha256`). **All numbers in this doc certify the
495cc91a era.** The generator patch is submitted-not-applied; when the
integrator lands it with the next regen the header re-pins and this tail
closes.

## 1.Collision set (complete, corpus-wide)

Method: (a) grep `redefinition` over all 1,680 official `build/parse/err/*.err`
— with `-ferror-limit=0` every reached collision materializes; (b) static
include-closure sweep (`$TMP/wt/sweep.py`: flags==1 names × per-file
LAYOUT_ASSERT-site map × per-TU quoted+angle include closure under official
LP64 `-I` resolution incl. compat/net|repin|musyx|njs shadows) to catch pairs
masked behind earlier fatal errors; (c) full-corpus scratch re-sweep under both
headers (GONE/NEW error-line multisets). (a)∩(b)∩(c) agree:

| waived assert name | colliding TU | emission site 1 | emission site 2 | materialized? |
|---|---|---|---|---|
| `TChallenge_size` | `src/pso/quest/challenge/ChallengeMode.cpp` | `include/pso/quest/challenge/TChallenge.h:44` | `ChallengeMode.cpp:593` (body) | yes — `:593:1: error: redefinition of 'pso_la_waived_TChallenge_size'` |
| `ChallengeEventHeaderSize` | `src/pso/unk/TLobby_80209524.cpp` | `include/pso/map/ChallengeEnemySet.h:59` | `TLobby_80209524.cpp:342` (body) | yes |
| `ChallengeRandomEventSize` | same TU | `include/pso/map/ChallengeEnemySet.h:60` | `TLobby_80209524.cpp:343` (body) | yes |
| `ChallengeFixedEventSize` | same TU | `include/pso/map/ChallengeEnemySet.h:61` | `TLobby_80209524.cpp:344` (body) | yes |

**Total: 4 collisions / 2 TUs. No latent (masked) pairs exist**: the static
closure sweep reaches-2-sites in exactly these 4 (name × TU) pairs and nothing
else; among flag==1 names in the whole decomp tree exactly these 4 have ≥2
sites. The 23 further multi-site names the raw sweep first showed were
dir-walk double-count artifacts (compat subtree scanned twice) plus pairs that
never co-resolve in any single TU; after `(resolved-path, line)` dedup the set
closed to the 4 above.

Not waiver-lineage: 12 other err files carry `redefinition of 'DVDCommandBlock'`
(`compat/repin/dolphin/dvd.h:36`) — the repin/era struct-tag class, P2-collision
debt ledger's, out of this tail's scope and untouched by this patch (verified
byte-identical in §4).

## 2.Root cause

`PSO_LA_NOP(nm, cond)` expands to `static int pso_la_waived_##nm = 0` — a
TU-local *variable definition*. When one TU reaches the same waived assert NAME
twice (here: a decomp-owned header emits the assert, and the .cpp body re-emits
the identical `LAYOUT_ASSERT(name, sizeof(...) == N)` line), the preprocessor
produces the same file-scope variable twice → redefinition error. The KEEP arm
never hits this because `typedef char nm[1]` ×2 is an *identical typedef
redefinition*, legal in both `-std=c11` (6.7p3) and `-std=c++17` — the waiver
arm simply picked the one spelling (`static int … = 0`) that is not
repeat-safe. Note the shape is header+body, not two headers (card said "two
headers" — mechanism is identical: two emissions, one TU, one waived name).

## 3.Mechanism fix (proposal — gen_waivers only, paste-ready)

Make the NOP arm the same repeat-safe spelling the KEEP arm already relies on:
an identical typedef. Deterministic name, zero cost, consumes the `;`, legal in
both dialects the battery uses (`-std=c11`, `-std=c++17`), and `-w` keeps
lint-stable.

```diff
--- a/build/gen_waivers.py
+++ b/build/gen_waivers.py
@@ -299,8 +299,11 @@
 #undef LAYOUT_ASSERT
 
 #define PSO_LA_KEEP(nm, cond) typedef char nm[(cond) ? 1 : -1]
-/* Waived case: a harmless zero-cost TU-local definition; consumes the ';'. */
-#define PSO_LA_NOP(nm, cond)  static int pso_la_waived_##nm = 0
+/* Waived case: TU-safe zero-cost typedef; consumes the ';'. P2-WT: an
+ * identical typedef redefinition is legal in -std=c11 AND -std=c++17, so a
+ * TU that reaches the same waived assert NAME through two emission sites
+ * (two headers, or header + body) no longer gets a variable redefinition. */
+#define PSO_LA_NOP(nm, cond)  typedef int pso_la_waived_##nm
 
 /* Dispatch: PSO_LA_FLAG_<nm> must be generated for every assert name.
  * If it is missing, PSO_LA_ACT_PSO_LA_FLAG_<nm> is not a macro and the unit
```

Rejected alternatives: `#ifndef`-style guard around NOP is impossible (a macro
expansion cannot emit a directive / test a variable); `__COUNTER__`-suffixed
variables do silence the collision but bake non-deterministic names into every
TU and keep a dead definition — the typedef makes the *same* emission repeat
*legally*, which is strictly the smaller change and mirrors KEEP.

## 4.Proof receipts (all scratch; `$TMP/wt/` = `/Users/scott/.hermes/profiles/pso-decomp/cache/scratch/wt/`)

1. **Generator reproducibility:** `python3 build/gen_waivers.py --decomp … --out $TMP/wt/base`
   → `base/pso_assert_waivers.h` byte-identical to live header (sha `495cc91a…`).
2. **Patched regen:** scratch copy under pseudo-root (`$TMP/wt/proot/build/gen_waivers.py`,
   `data/` symlinked read-only) → `$TMP/wt/patched/pso_assert_waivers.h`
   (sha `d4382fcd0659357f283e675726c1076338e909550c067513459280025e9f0eee`).
   vs live: single hunk, mechanism lines ONLY; `#define PSO_LA_FLAG_*` lines:
   3333 vs 3333, empty diff (`FLAG-LINES-IDENTICAL`), waived=2946/3333
   over 1549 types both sides.
3. **Harness fidelity:** full-corpus re-sweep with the LIVE header under
   official LP64 flags (`$TMP/wt/sweep_patched.sh`, -P6, 1,680/1,680 files):
   error-line signature identical to official `build/parse/err/` for all
   1,680 units (0 mismatches).
4. **Collisions vanish / none new:** full-corpus sweep with PATCHED header
   (`$TMP/wt/err_full_patched/`): `pso_la_waived` redefinition error lines
   4 → 0; corpus-wide error-multiset diff vs official = exactly 4 GONE
   (the 4 table lines in 2 TUs), **0 NEW**; official-PASS set (350) errors:
   0; FAIL total stays 1330. Residual red in the two collision TUs is the
   separately-named classes: `NJS_OBJECT_size` negative-array (njs lane) and
   `FloorObjectSetRow_size`/`EnemySetRecordSize` (floor-population lane).
5. **Era check re-run post-gates:** live header sha still `495cc91a…` at
   sweep completion (08:51) — whole doc is 495cc91a-era certified.

## 5.Landing note (integrator)

Apply §3 to `build/gen_waivers.py` TEMPLATE; next `make waivers`/regen emits
the typedef mechanism (header sha leaves `495cc91a` — D8 re-pin at the next
apply wave). No source-side edits are needed in `ChallengeMode.cpp` or
`TLobby_80209524.cpp` for the waiver tail (their duplicate assert lines become
legal repeat-typedefs); their remaining errors stay on the njs/floor lanes.
Expected post-landing delta on the official battery: PASS ≥ 350 (350 cannot
regress — receipts §4.4), minus-4 redefinition lines in exactly those two err
files, `DVDCommandBlock` class unchanged.

Artifacts (scratch, uncommitted): `wt/sweep.py`, `wt/gen_waivers_patched.py`,
`wt/proot/`, `wt/sweep_patched.sh`, `wt/err_full_patched/`, `wt/err_full_live/`,
`wt/analyze.py`, `wt/static_collisions.json`, `wt/base/`, `wt/patched/`.
