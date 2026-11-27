# compat/collision — quoted-include collision pin (P2-AZ)

Status: authored, INERT — nothing references this dir until the integrator
adds the `-include compat/collision/pin_dolphin_types.h` flag line (recipe +
full evidence: `docs/p2-collision-debt.md`).

- `pin_dolphin_types.h` — force-include pin occupying the era SDK guard
  `_DOLPHIN_TYPES_H_` (decomp `libs/dolphin/include/dolphin/types.h`) before
  either include edge can fire, delegating scalars to the port rebind
  (`<types.h>` → `host/bringup/include/types.h`) and carrying the era tail
  minus libc legs. Kills the 5-error typedef cluster tree-wide (1,136 TUs at
  rebind-only; measured 0 collateral, battery-3).
- `receipts/` — `clang -H` proof that the angle edge binds -I order but the
  quoted-sibling edge binds the including file's directory (why shadows/
  ordering alone can never fix the 1,109 and a pin can).

Rule: this is the ONLY permitted mechanism class for SDK-header typedef
collisions — no blanket per-dir shadows (read-only tree forbids the
hijack-correct placement anyway), no era `-D` without a carried tail.
