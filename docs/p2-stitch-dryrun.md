# P2-AO — Waiver stitch dry run (OUT OF PLACE, zero repo mutation, zero make)

Date: 2026-11-26 · Decomp pinned `6b38b5383` (read-only) · Working copy: `$TMPDIR/stitch_dry/gen_waivers.py`
Committed `build/gen_waivers.py` and `build/waivers/**` were NOT touched; nothing in `data/` was written.
Probe class: direct-clang LP64-native (`-target arm64-apple-macos11 -fsyntax-only -ferror-limit=0`),
`-include compat/pso_metal_compat.h -include <generated header>`, run_p2t_uncapped/flip_test flag set, -P10.

## 1. STITCH-STEP-0 — enumeration audit: **PASS (after fix); generator bug CONFIRMED**

`scan_tree` (repo version) enumerates 3317 assert names; it **misses all 21 NET_DISPATCH_OFFSET
`##`-paste names** (`NetProtocolHandlerEntry_*_offset` … `NetworkDispatchState_pad_F4_offset`) declared in
`include/pso/network/NetworkDispatch.h:63`. Verified absent from the committed header (grep count 0).
Root cause is two-fold: `HELP_RE` allows at most ONE `##` paste (`(?:\s*##\s*\w+)?`), and the registration
loop requires the paste RHS to be a macro param (`_##` has literal `_` RHS → `continue`).
Consequence measured: under the committed header, the 21 names have **no `PSO_LA_FLAG_*` at all**; in C++
TUs the dispatch expands to an undeclared identifier (hard compile death — fail-closed as designed), and in
-w C11 probes (this corpus's 16 includers are `.c`) the token degrades to a *suppressed implicit-function
warning* — the assert silently never checks, violating the header's own "unknown asserts can NEVER be
silently swallowed" contract. Either way the unit is lost, exactly as T flagged (16 files incl. NetworkDispatch.h,
all 16 in the known-FAIL set).
Fixed copy: 3333 names = 3317 − 5 phantom (`ChatOffset_field`-class names the OLD code minted from the
`#define` line itself; all committed as 0, harmless) + 21 NET names; `unresolved == 0`; after-fix sweep shows
**zero** still-fail files carrying the `PSO_LA_ACT_PSO_LA_FLAG` signature.

## 2. Flip audit (stitched header vs committed 99-waive baseline)

Chain: `--tsv assert-verdicts.tsv verdict-tranche1.tsv verdict-tranche2.tsv verdict-tranche3.tsv
verdict-resolutions-1.tsv verdict-resolutions-2.tsv verdict-resolutions-3.tsv` (resolutions LAST = override;
env-first honored). Base-parity run (`--tsv` first file only) reproduces committed baseline: **waived=99,
types=31, 0 flips of any kind** — before/after identical on all 3317 common names.

Stitched: `ACTIVE — waived 1714/3333 asserts over 842 types` (receipt rows per source: 1770+200+430+271+60+84+7).

| assertion | result |
|---|---|
| 0→1 flips | **1615** (810 new RW types; all corpus-flagged) |
| 1→0 flips | **0** |
| flips on extended deny-list (6 new + 13 old names; 51 asserts) | **0** |
| flips on lane-j held set (5 NJS_* + EnemySetRecord + FloorObjectSetRow; 26 asserts) | **0** |
| added names (21 NET, all value 0) / removed phantoms (5, all value 0) | benign |

Flip asserts by first RW source: tranche1 197 · tranche2 751 · tranche3 386 · resolutions-1 102 · resolutions-2 179.
No deny-leak WARNING fired. Only WARN: `conflicting verdict rows for NJS_POINT3COL — kept (fail-closed)`.

**Spec collision for the integrator (documented, not resolved here):** plan EDIT-2 as written collects keepers
globally, so NJS_POINT3COL (LOCKED-FORMAT in resolutions-1:52, RUNTIME-WAIVE in AJ's decisive resolutions-3:4)
becomes a CONFLICT → kept. That satisfies "zero flips on held set" but contradicts "resolutions LAST override"
+ AJ's closed-gate intent (p2-njs-confirm.md). Either reading changes exactly ONE type's flags (NJS_POINT3COL,
3 asserts — immaterial to the realized count); nothing else. If last-wins-over-earlier-lock is desired: gate keeper
membership on "keeper appears in a file **no earlier than** the RW claim" and exempt res-3 explicitly.

## 3. Materialize (real header, not simulation)

- Full known-FAIL corpus (1610, superset of the required ≥400 sample): **80 FAIL→PASS flips**.
- Stratified sample (every 4th line, n=403) run first: 20 flips → extrapolation 20/403·1610 = **80** — sample and
  full agree exactly.
- Regression: all **70/70 known-PASS stay PASS** (≥60 required).
- **Realized LP64 = 150/1680** (was 70).
- Per-cause split of the 1530 still-fails (uncapped, new header): 1473 multi-cause (keep-class assert + other
  errors), 18 layout-assert-only (blocked purely by keep-class asserts — AL queue pickup), 39 pure
  dialect/intrinsic (no layout traffic at all; ≈ flip2-class, musyx-heavy), 0 missing-flag deaths.

## 4. Reconciliation vs X's 229 sim — numbers first

X anchor = 70→229 (sim123 newly-clean=136 at +206, T4 queue +23). Realized +80 ≈ half. Three-way attribution:

- **(b) cap-hidden BLIND tail — the dominant term (cited, per AK).** S/X's `sim()` scored files from the CAPPED
  20-error logs; AK's landed audit (`docs/p2-census-audit.md`, `9cc6e20`, `data/census-audit.tsv`) shows the true
  uncapped population is 1602 types / 2951 sites vs the 829 / 1467 the corpus adjudicates — **667 BLIND types /
  1130 sites, 667/667 cap-hidden**, files whose sim-visible errors were all RW still carry keep-class asserts
  once uncapped. Spot examples from this dry run's still-fail sample: `src/pso/audio/AdxtStart.cpp`
  (failing `AdxtPlayer_size`, type BLIND), `src/pso/audio/SoundRequestVoice.cpp` (BLIND `GuildCardSlot` +
  `ChallengeRecord`), `src/pso/boss/TBoss3VoloptCageShot.cpp` (BLIND `TObjVoloptCage`). The 18 assert-only
  still-fails are the immediately-next tranche of that tail.
- **(a) step-0 enumeration — confirmed generator bug, small realized impact.** The 21 NET names fail their
  offset asserts only if unprefixed past-LP64 layouts shift; with the fix they enumerate as honest KEEP flags
  and NET_DISPATCH's own 21 offsets PASS at LP64 (packed wire structs) — so none of the 16 net files flipped
  (all still die on the BLIND net/wire families AK sized, e.g. the NetIpv6Route/Ppp/Bootp/Dhcp 72-probable set).
  The bug's real cost is the silent-skip contract hole in C and C++ TU death, not the +80/+56 delta.
- **(c) chain content vs sim inputs.** (i) X's 229 includes the T4 queue (371 sites, `p2x_tranche4.json`) — there
  is **no `data/verdict-tranche4.tsv`**; the stitch chain's honest ceiling is the 206 anchor, not 229.
  (ii) resolutions-1/2 re-locked ~36 types tranche had called RW and EDIT-1 deny + NJS_POINT3COL conflict remove
  waivers the sim counted. (iii) 39 files need the dialect/intrinsic lanes, no waiver can help them.

Honest anchor going forward: **150/1680 now, realized == what the corpus can materialize; AL's BLIND queue
(101 priority types / 507 sites) closes the rest** — 70+~80 is the correct figure to expect today, not 229.

## 5. Generator fixes for the integrator (apply to repo `build/gen_waivers.py` verbatim from the copy)

1. `HELP_RE`: `([A-Za-z_]\w*(?:\s*##\s*\w+)?)` → `(?:\s*##\s*\w+)*` (multi-paste templates).
2. Registration: replace single-`paste` search with `segs = name_tmpl.split("##")`; register when ANY segment
   is a macro param (`helpers[macro] = (segs, plist, typ)`); drop the `paste.group(2) not in plist` bail.
3. Call-site expansion: join segments, substituting param segments from call args; resolve generic TYPE from
   the call arg when the TYPE token is itself a param (`NET_DISPATCH_OFFSET(type,…) → type=args[0]`);
   **skip matches whose line starts `#define`** (kills the 5 phantom-name class).
4. `read_verdicts`: accept list; verdict column `verdict` **or `new-verdict`** (resolutions files lack
   `verdict` — without this the override layer would be fail-closed-skipped with an ERROR line);
   two-pass keepers {FORMAT-KEEP, LOCKED-FORMAT}; return None only if ALL sources absent.
5. `main`: `--tsv nargs="*"`, default [env?, assert-verdicts, t1, t2, t3, sorted(resolutions-*) LAST];
   print per-source row counts on the ACTIVE line.
6. Decide the NJS_POINT3COL keeper-vs-last-wins policy (§2) before regenerating the real header.

## 6. Verdict

Not GENUINELY UNSAFE anywhere: zero deny-list and zero held-set flips, zero 1→0 regressions, zero
PASS-file regressions, zero missing-flag deaths; realized 150/1680 is strictly ⊇ today's 70 and matches
"materialized + known-unadjudicated tail (AK/AL)" rather than the stale capped 229. Remaining pre-apply
conditions are the NOT-run battery (this dry run was forbidden from `make` while lane Z builds): steps 2–3 of
the plan (`make parse` arm64_32 ≥1416 non-regression, `tools/parse_gate.py` floor 669) must be executed by the
integrator at apply time, plus the §2 NJS_POINT3COL policy call.

**GO — for applying the real stitch** (with the §5 fixes included and the battery run at apply time).
