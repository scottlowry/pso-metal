# P2 apply-4 — collision pin adopted: LP64 parseable 162 -> 1,258 (integrator-verified)

Date: 2026-11-27 (~06:35). Input: AZ lane `4c9930d` (compat/collision pin + receipts + docs/p2-collision-debt.md).
No corpus edits (waiver header untouched: 2,943/3,333 stays; pin changes PARSE ADMISSIONS, not verdicts).

## Independent verification (integrator harness, battery-3 corpus 1,680 units, doc recipe lines only)
| shape | AZ claim | integrator re-measure | match |
|---|---|---|---|
| H0 control (committed corpus) | 162 | **162** (1,518 fail) | exact |
| H1C rebind+pin | 257 (+96/-1) | **257** | exact |
| H3 full stack | 1,258 | **1,253** +5 gap -> all 5 PASS under verbatim AZ recipe | exact w/ caveat* |
| cluster-zero grep (H3) | 0 | **0** `typedef redefinition with different types` | exact |
| collateral H0-clean->H3-fail | {GcrnaStream.cpp} | {GcrnaStream} under AZ shape; my deviant shape added TObjEneMe1CanadinLeader (+3 more of same class) | confirmed w/ caveat* |

*Caveat (banked to skill): `-include compat/repin/dolphin/types.h` alongside the pin is NOT interaction-zero —
it breaks `ceil`-via-era-libc-chain in 5 TUs. Adoption rule: pin force-include + repin/njs ONLY on the -I path,
never as -include; force-include exactly compat/pso_metal_compat.h, build/waivers/pso_assert_waivers.h,
compat/collision/pin_dolphin_types.h, then -DPSO_PORT_LAYOUT_WAIVER, -I$R/compat/repin -I$R/compat/njs
-I$R/host/bringup/include before the decomp -I list (bringup earliest among repo dirs wins <types.h>).

## Meaning (stated precisely)
- ~86% of the era corpus (1,253-1,258/1,680) now compiles under **LP64** with layout asserts passing **honestly**:
  the rebind restores era 4B scalars so sizeof/offsetof checks hold by byte-truth, not by waiver. This is the
  fail-closed ideal the deny-list existed to protect — the collision pin is the lever AW's premise-flip named
  and AS2's bucket math could not see (1,127 "layout-LOCKED-cogate" files were collision-gated, not waiver-gated).
- Parse != runtime correctness: uaddr-as-handle obligations (J/AR program, stamping in loaders) and the
  427-unit residual classes (NJS-handle ~112, packet classes 57, PppFrame 50, MWCC-asm ~38, musyx-C 26,
  FloorPopulation 18, TBoss6 14, tail ~100) remain the working queue. Runtime behavior under the rebind for
  pointer-in-u32 sites is exactly what the handle program was always for — no new confidence claimed here.
- GcrnaStream OSAlloc breakage stays on the collateral ledger pending D4 facade work (host lane).

## Next
- Sequenced behind this: residual-class lanes off apply-4 (NJS-handle 112 first — AR allow-list machinery exists),
  and full-tree re-record when BA's V-wave lands (window suites must stay green under stack shape).
