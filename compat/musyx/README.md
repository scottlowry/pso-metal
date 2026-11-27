# compat/musyx — P2-MX musyx C-dialect shadow headers (INERT SUPPLY)

Shadow directory for the musyx C-dialect residual class
(`docs/p2-collision-debt.md` §4.4 item 4; survey + receipts:
`docs/p2-musyx-1.md`). Nothing here is active unless `-I compat/musyx`
is placed **ahead of** `-I include` / `-I src` on the harness line
(paste-ready ask lives in `docs/p2-musyx-1.md`; integrator owns flag
lines).

Contents (faithful copies of the decomp originals, era-truth edits only):

| shadow | original | edits |
|---|---|---|
| `musyx/musyx.h` | `include/musyx/musyx.h` | wrap bare `extern "C" {…}` (orig :161–170) in `#ifdef __cplusplus`; decls stay unconditional |
| `pso/audio/SoundReverb.h` | `include/pso/audio/SoundReverb.h` | elaborate `struct SoundReverbDelay`/`struct SoundReverb` (orig :11,12,29,30); guarded linkage + named-param C form of `ReverbHICreate` (orig :32–33) |
| `musyx/runtime/musyx_internal.h` | `src/musyx/runtime/musyx_internal.h` | elaborate `struct SoundReverb *rv` (orig :13); neutralize `AT_ADDRESS` (colon-declarator pin from `include/types.h:61`) immediately after `#include "types.h"` — same precedent as `compat/dialect/pso_dialect_compat.h:26` (P2-T), parse-world only |

Rules honored: `-I`-path only (never `-include`), no `compat/njs`
reinterpretation on `.c` TUs, zero layout change (scalar mirrors only),
pin order untouched.

**Ceiling (measured, docs/p2-musyx-1.md §3):** the shadows clear every
*header-side* error, but 27 runtime `.c` bodies carry their own bare
`extern "C" {` which no header can fix — the class-kill for those TUs is
driver routing (`clang++ -x c++`, the P2-T flip link-name-audited in
`docs/p2-musyx-linknames.md`), not this supply. Keep the shadows for
C-side consumers that include `musyx.h`/`SoundReverb.h` without the flip
and for the two `asm void`+missing-`orig_asm/*.inc` WAIVE candidates.
