# compat/net — P2-NP era-truth layout shadows (net pool)

**STATE: INERT.** No build file, host file, or data file references this
directory (verified by repo grep at commit time; see docs/p2-pppframe-1.md
Repro). Adoption is the integrator's paste-ready `-I` line below — this dir
ships zero flags of its own and force-includes nothing.

## Why a shadow dir (not a njs-style per-TU reinterpretation)
The PppFrame residual class (docs/p2-collision-debt.md §4.3 item 3) is 50 TUs,
46 of them **.c**. The `compat/njs` handle reinterpretation (u32↔pointer
shadow) is BANNED on `.c` TUs — AR §5 silent-truncation; it is a C++-only,
opt-in, listed-TU mechanism. On `.c` there is **no C-legal way** to make
`sizeof(struct PppFrame { u8 *data; u16 len; }) == 8` under LP64: a real host
pointer is 8B. Measured before/after receipts: `cache/scratch/np/` (probes
N3R = fail, shadow_bad/u32-field variant = new deref type-errors, S = clean).

## Contents
- `pso/network/NetworkRuntime.h` — verbatim copy of the era
  `include/pso/network/NetworkRuntime.h` (124 lines, diff = 1 line + header
  comment) with `NP-EDIT`: the `PppFrame_size_check` assert is rewritten as an
  **era-layout MODEL check** (`sizeof(struct PppFrameEraModel { u32 data; u16
  len; }) == 8`). Host struct keeps its real `u8 *data`; every deref in the
  corpus compiles as written. The assert NAME and the era invariant stay
  compile-visible (stronger than waiving the name: a waived `PSO_LA_FLAG`
  collapses the typedef to a `static int`, erasing the check — assert-policy.md
  §4). All other asserts in the file (`Ipv6cp*`, `NetThreadState_size_check`)
  are already waived in the pin'd header and their scalar ones pass under the
  rebind honestly — carried verbatim.

Quoted-edge note: all 50 members reach the header via the quoted spelling
`#include "pso/network/NetworkRuntime.h"`; no header inside
`include/pso/network/` includes it by same-dir sibling spelling (grep
receipt: zero `#include "NetworkRuntime.h"`), so `-I` ordering DOES hijack
every edge — unlike the AZ edge-B (dolphin) case, no pin is needed here.

## Integrator flag-ask (paste-ready, one line, order-sensitive)
Add BEFORE `-I include` on the C-legal stack shape:

```
-I $REPO/compat/net
```

Full legal-stack line set (existing three pinned force-includes unchanged):

```
-DPSO_PORT_LAYOUT_WAIVER
-include compat/pso_metal_compat.h -include build/waivers/pso_assert_waivers.h \
-include compat/collision/pin_dolphin_types.h
-I compat/net -I compat/repin [-I compat/njs (C++ TUs only)] -I host/bringup/include \
-I include -I include/clib -I libs/dolphin/include -I libs/dolphin/include/dolphin -I src
```

Alternative (equivalent admission, weaker posture): flip
`PSO_LA_FLAG_PppFrame_size_check` 0→1 in the generated waiver header
(`data/assert-verdicts.tsv` re-verdict of `PppFrame` as RUNTIME-WAIVE +
`gen_waivers`; integrator-owned — NOT run by this lane). Both shapes measured
identical on the battery: see docs/p2-pppframe-1.md probe table.
