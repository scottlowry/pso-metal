# P2-STITCH APPLY — receipt (integrator, 2026-11-26 22:4x)

The multi-source waiver stitch (plan: cache/scratch/gen_waivers_stitch.py; dry-run:
`docs/p2-stitch-dryrun.md` by lane AO, verdict GO) applied to the live tree.

## What changed
- `build/gen_waivers.py` (already in `efd8a8c`): AO §5 fixes — multi-paste HELP_RE
  enumeration (+21 NET_DISPATCH names, −5 phantoms: 3317→3333), 8-source multi-TSV
  with resolutions-glob-later-wins precedence, deny-root set extended (29 roots incl
  AJ/AD loaded-image positives), override receipts on stderr.
- `build/waivers/pso_assert_waivers.h` + `waiver-status.txt`: regenerated ACTIVE —
  **waived 2179/3333 asserts over 934 types**; sources: assert-verdicts=1770,
  t1=200, t2=430, t3=271, res1=60, res2=84, res3=7, res4=101; override printed:
  NJS_POINT3COL(keeper@4->rw@6) per AJ census (deny-roots immune).
- `host/bringup/bringup.mk` + `waiver-header.sha256`: D8 guard migrated from literal
  `STATE: 99 waived` to integrator-approved sha pin (`c1a64d9817c8b20c…`).

## Battery results (all executed, all read back)
| Gate | Result |
|---|---|
| gen_waivers rc | 0 |
| `make parse` (arm64_32) | **PASS=1576/1680** (was 1416; +160 from NET_DISPATCH honest enumeration) |
| `tools/parse_gate.py` | rc 0 — baseline 669 floor, +907 |
| bringup suites, new era | Q 16/16 · Y 13/13 · AE 26/26 · AN 27/27 — run.sh exit 0 |
| dl 7-pt textured gate, new era | frames=30 samples_ok=210 verify_failures=0 exit 0 (AQ's stitch-era re-cert clause satisfied) |
| LP64 full-tree sweep (1,680, -P10, argv-safe driver) | **PASS=150 FAIL=1530**; +80 flips vs pre-stitch 70; all 70 prior passes preserved (zero regression); sample-checked (zero-byte err logs) |

## Honest anchors
- 150/1,680 is the MATERIALIZED truth vs AO's extrapolation 150 (exact hit).
- Population truth per AK: 1,602 failing types / 2,951 sites; blind tail (566 types,
  1–2-site) in adjudication; boot-spine still blocked on NJS re-pin adoption
  (per-TU C++ allow-list, AR policy) — the stitch was never the title-screen gate.
- Sweep v1's phantom 0/0 (xargs argv overflow, wrapper rc 0) — pitfall banked;
  v2 driver `probe_one.sh` is the canonical pattern.
