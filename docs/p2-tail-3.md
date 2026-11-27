# P2-TL3 — asm-ten batch-B port specs, _GXColor site census, eth.c `__lhbrx` wall entry

Phase-3 lane TL3 (card `t_3a77438e`), fleet wave-4 2026-11-27. ZERO builds;
official corpus untouched; scratch `clang -fsyntax-only` probes only
(`cache/scratch/tl3/`, tlp-era flag shape cloned verbatim from
`cache/scratch/tlp/play3_verify.py`, applied to the ON-DISK post-apply-8 pin).
Waiver pin re-hashed this turn: `build/waivers/pso_assert_waivers.h` =
`d4382fcd0659357f…` == pin. Decomp `@6b38b5383` READ-ONLY; writes = this doc
+ scratch dir only. Predecessors: `p2-tail-2.md` §2–§5, `p2-stitch-apply8.md`,
`p2-mwccasm-1.md` (class-doc precedent), `p2-njs-repin-1.md` §4.2 (wall).

## 1. Apply-8 verification — the §2 va_info shim spec is DROPPED (moot)

Card mission (2) resolved at intake, per law "verify before authoring":

- `p2-stitch-apply8.md` landed ledger: FAIL 517→513, FIXED exactly
  `CriSvm.cpp`, `LscLog.cpp`, `NetworkConnect.cpp`, `Gcci.cpp`, BROKE ∅.
- **Own same-turn measurement** (`va_pair_check.py` → `va_pair_receipt.json`):
  UNMODIFIED `src/pso/audio/CriSvm.cpp` and `src/pso/audio/LscLog.cpp` compile
  **rc=0, 0 errors** at the post-apply-8 LP64 shape.
- Corpus grep: **zero** `__builtin_va_info` references in `src/**`+`include/**`
  (non-musyx); the only `va_*` users are ordinary `va_list/va_start/vsprintf`
  spellings (CriSvm:188-190, LscLog:15-17, DebugFont:124) fed by the pin's
  guard-occupying `_STDARG_H_` + `__builtin_va_*` supply
  (`pin_dolphin_types.h:94-97`, read on disk this turn).

No fixed-arity shim, no era-macro carry, no scratch shim probes are needed.
`p2-tail-2.md` §5 item 5 closes via apply-8's supply. Nothing in this doc
proposes `compat/**` changes.

## 2. Play 3 batch B — era-.s-traced port specs (asm-ten, remainder 4)

**"6 of 10" hypothesis, verified at intake:** batch-A = SIX spec entries
(tail-2 §4.1–§4.6) covering SEVEN functions over SIX TUs (§4.1 specced the
sin+cos pair as one entry; `play3_receipts.json` shows `replaced: 2` there).
The asm-ten leaf census = the ten `orig_asm/*.inc` leaves inside those six
TUs. Remainder 4, each named by tail-2 as batch-B/queued: `matrix_identity`
(§4.2 note), `transform_point_by_current_matrix` (§4.3 note),
`convert_color_u8_to_f32` (§4.6 note), `LaunchDol` (line 192). Receipts:
`cache/scratch/tl3/tl3_verify.py` + `tl3_receipts.json` + patched copies in
`cache/scratch/tl3/`. Controls: batch-A ports are PROPOSALS — decomp still
carries all ten asm bodies, so my patched copies keep the batch-A sibling asm
as residual diagnostics (never the ported function).

P2-V mirror law (restated): the era `.s` proves only what the era executed;
the port decisions cite it as semantic ground truth while the TU keeps its
unported state. Trace addresses are `.s` comments.

**B.1 `matrix_identity` — src/pso/math/MatrixVector.cpp:47** (era 803D8CA8)
Trace: `lfs f1, Unit01+0x4` (=1.0f — the TU ALREADY ships `static f32 Unit01[]`
at :45 with a comment naming THIS function as its reader); `ps_sub f0,f1,f1`
→ zero pair; `ps_merge01` builds the unit pair; five `psq_st` + two `stfs`
write, at 0x10 row stride over `MtxP m` (r3): row0 {1,0,0,0}@0x00–0x0C, row1
{0,1,0,0}@0x10–0x1C, row2 {0,0,1,0}@0x20–0x2C. Meaning: plain 3×4 identity,
data-dependent on Unit01 for its 1.0f (no arithmetic).
Port (C-legal, keeps the Unit01 reference so no dead-static drift):
`for (i<3) for (j<4) m[i][j] = (i==j) ? Unit01[1] : 0.0f;`
Fidelity: exact IEEE constant stores — bit-faithful, zero caveats.
**Receipt: zero `matrix_identity` diagnostics; 5 residual = NJS wall ×2 +
batch-A-queued `vector_length` asm ×3. TU stays §4.2-walled (NC/NA seat).**

**B.2 `transform_point_by_current_matrix` — src/pso/unk/text_803D96A8.cpp:92**
(era 803D9884)
Trace: `lis/addi/lwz` deref of global `current_matrix` (RenderSbssGlobals.h:11
`extern GC_Mtx *current_matrix;` — matches the sibling bodies' `*current_matrix`
spelling in this TU); `psq_l f6=src{0,1}`, `psq_l f7=src{2,3}` (the point is a
4-float record — the 4th lane at src+0xC is consumed by column 3); per row i:
`psq_l` row pairs {m_i0,m_i1}, {m_i2,m_i3} at 0x10 stride, `ps_mul {m_i0,m_i1}
*{x,y}`, `ps_madd {m_i2,m_i3}*{z,w}+`, `ps_sum0` (horizontal add, both lanes =
sum), `psq_st` → dst->x/y/z at 0/4/8. Meaning: full homogeneous
`dst = current_matrix · src` with the row-sum order `(m_i2·z + m_i0·x) +
(m_i3·w + m_i1·y)`.
Port (trace order, fused — `ps_madd` IS an fma, so `__builtin_fmaf` is
BIT-EXACT against the era, unlike the §4.2/4.3 sqrt-estimate notes):
```c
void transform_point_by_current_matrix(const f32 *src, Vec *dst) {
    MtxP m = current_matrix;
    f32 x = src[0], y = src[1], z = src[2], w = src[3];
    f32 o[3];
    for (s32 i = 0; i < 3; i++) {
        f32 lo = __builtin_fmaf(m[i][2], z, m[i][0] * x);
        f32 hi = __builtin_fmaf(m[i][3], w, m[i][1] * y);
        o[i] = lo + hi;
    }
    dst->x = o[0]; dst->y = o[1]; dst->z = o[2];
}
```
**Receipt: zero ported-fn diagnostics; 5 residual = wall + batch-A-queued
`normalize_vector` asm. Six sibling asm blocks in this TU (rotate/push/scale/
post-translate…) are OUTSIDE the asm-ten census — future-wave pool.**

**B.3 `convert_color_u8_to_f32` — src/pso/animation/MotionContext.cpp:392**
(era 803D8008)
Trace: `mtspr GQR1, 0x08040804` arms the byte-type (u8) load conversion —
mirror of §4.6's f32→u8 direction; then THREE load/store lanes and ZERO
arithmetic instructions: pair lane `psq_l +0x0 → psq_st dst+0x4/+0x8` (r→r,
g→g), single lanes `+0x3 → stfs dst+0x0` (a→a) and `+0x2 → stfs dst+0xC`
(b→b) against `_GXColor{u8 r,g,b,a}`×`NJS_ARGB{f32 a,r,g,b}` (offsets decoded
from the store addresses, name-preserving exactly like §4.6). Pure widen:
u8→f32 is exact under the GQR converter — ZERO fidelity caveats (no saturation
concern in this direction; the mirror's [0,255] UB note does NOT apply).
Port (batch-A §4.6 shape, one include):
```c
#include <dolphin/gx/GXStruct.h>   // after the TU's first include — §3
void convert_color_u8_to_f32(const struct _GXColor *src, NJS_ARGB *dst) {
    dst->a = (f32)src->a; dst->r = (f32)src->r;
    dst->g = (f32)src->g; dst->b = (f32)src->b;
}
```
**Receipt: zero ported-fn diagnostics AND zero incomplete-type diags (the
§4.6 mc1 catch, now proven closed by the supply line); 5 residual = wall +
batch-A-queued `convert_color_f32_to_u8` asm.**

**B.4 `LaunchDol` — src/pso/unk/text_8042E8C4.cpp:27 (era 8042E8C4) —
VERDICT: NOT PORTABLE; route to the MX host-facade queue.**
This is the batch-A forecast (line 192), now era-traced in full. Body =
reset-payload relocator: frame + `stmw`; `memmove(0x80003100, context,
(context->[0x108]+0x132) & ~31)` + `DCStoreRange`; `OSResetSystem(2,0,0)`;
`memset`/`DCStoreRange` on the `[0xd8]/[0xdc]` region; a 7-entry
(delta@+0x00, dst@+0x48, len@+0x90) `memmove+DCStore+ICInvalidate` table and
an 11-entry (delta@+0x1c, dst@+0x64, len@+0xac) move/store table, base =
`context->[0x104]`; then the epilogue-free computed tail jump
`lwz r0,0xE0(r26); isync; mtlr r0; blr` into the reloaded image. The TU's own
comment (probed GC/1.2.5…3.0a5.2) documents that no MWCC emits that tail from
C; independently, clang CANNOT parse `asm void` typed definitions at all
(control this turn: first error `expected '(' after 'asm'` at :27, then
`register` storage-class error, then `LaunchDol.inc' file not found` — 4 diags,
so even landing the generated `.inc` would not flip this TU). Absolute-address
writes (0x80003100), cache-management calls, and a jump into loaded code have
no host semantics. **Proposal (INERT): batch it with the musyx §4.4 TU-body
residues (chorus_fx/reverb) into the host-facade queue — p2-mwccasm-1 class
treatment, not a decomp edit; zero green available from TL here until that
program seats it.**

## 3. `_GXColor` incomplete-type site census (batch-A catch, mission 3)

Exhaustive grep over decomp `src/**`+`include/**` (python pass, this turn):
**exactly 5 named sites, all in the MotionContext pair.** No other TU names
`_GXColor` at all.

| site | use | INERT disposition | evidence |
|---|---|---|---|
| MotionContext.cpp:388 (`f32_to_u8` dst) | port names members | **SUPPLY**: `#include <dolphin/gx/GXStruct.h>` after first include (already in batch-A §4.6 spec) | era `animation/MotionContext.s` 803D7FE0–803D8004 psq lanes read/write byte offsets 0–3 of the struct |
| MotionContext.cpp:392 (`u8_to_f32` src) | port names members | **SUPPLY**: same single include line covers both | era `.s` 803D8008–803D800C lanes (B.3 above); receipt P3: with the include, 0 incomplete-type diags (mc1 control without it: 4) |
| MotionContext.h:11 `struct _GXColor;` | forward decl | **KEEP fwd-decl** | 20 TUs include this header (grep count, same turn); era evidence shows NO pso TU outside MotionContext touches the layout; the definition owner is the dolphin SDK (`libs/dolphin/include/dolphin/gx/GXStruct.h:28-33`, already on the corpus `-I` chain — the P3 probe resolves it with zero compat work) |
| MotionContext.h:109/:110 (the two decls) | pointer-only params | **KEEP fwd-decl** — pointers to incomplete types are legal; no member access exists at decl sites | C++ rule; batch-A §4.6 same reasoning for the asm-era spelling |

Net: supply-vs-fwd-decl resolves to ONE supply line inside the MotionContext
.cpp port (both members sites share it) + zero header changes. Moving
`GXStruct.h` into MotionContext.h would drag the full `dolphin/gx` chain into
20 TUs for no parsing benefit — rejected on the .s ownership evidence above.

## 4. eth.c `__lhbrx` — post-apply-8 first wall, batch entry (mission 4)

apply-8 named it (`p2-stitch-apply8.md`: "behind it eth.c → `__lhbrx`");
control probe this turn confirms it is eth.c's **SOLE first error**:
`ctl_eth.c:176:15: error: call to undeclared function '__lhbrx'` (n_err = 1 —
no wall behind it). Single corpus site: `bba_swap16()` in
`src/pso/network/eth.c:174-177` — MWCC intrinsic, not header-supplied (zero
decls anywhere in src/include), hence not a §2/compat supply candidate; it is
the MWCC-intrinsic dialect class of p2-mwccasm-1 §1 but with exactly one
site, so the minimal legal spelling is the per-TU one-liner (same class as the
tail-2 §3 network decl-aligns, which this lane keeps re-verified for landing
together).

Era trace (`network/eth.s`, 5 inline sites: 80429848→80429850, 8042A390,
8042A3F8, 8042A918, 8042A980): `sth rX, slot(r1); addi r0, r1, slot;
lhbrx rD, r0, r0; sth rD, …` — store-halfword then reload **byte-reversed**:
a pure register u16 byte swap on a local, no MMIO, no endianness assumption
about the caller's data. That is exactly `bba_swap16`'s job: the BBA returns
LE-ordered bytes in a BE u16; era swapped them at compile-lowered store/
reload.

Spec (batch-B entry B.5, paste-ready one-liner):
```diff
--- a/src/pso/network/eth.c                                                  (line 176)
-    swapped = __lhbrx(&val, 0);
+    swapped = (u16)((val >> 8) | (val << 8));  /* era eth.s lhbrx-after-sths = store/reload byte swap; portable u16 rotate */
```
**Receipt: patched copy rc=0, 0 errors — eth.c FLIPS PASS (the wave-4 tail's
first named green from this lane's new work).**

## 5. Wave ledger

| disposition | TUs |
|---|---|
| shim spec dropped (apply-8 verified) | CriSvm, LscLog (already counted in the 1,163→1,167) |
| NEW flip spec: eth.c (B.5, 1 line) | 1 |
| ports specced, zero ported-fn diags, wall-behind (join §4.2 stage rows) | MatrixVector, text_803D96A8, MotionContext |
| verdict resolved → MX host-facade queue | text_8042E8C4 `LaunchDol` |
| prior-batch flips still queued for landing (receipts unchanged) | SinCosTable, text_803D9650, 4× network .c (tail-2 §3) |

Ranking for next wave: (1) land batch-A+B ports + tail-2 §3 aligns + B.5 in
one integrator battery (7 flips measurable today from landed receipts:
SinCosTable, text_803D9650 (batch-A), eth.c (B.5), NetApi, NetDns, PppBuffer,
PppFsm (tail-2 §3)); (2) §4.2
stage-4 rows unlock the B.1–B.3 arrivals; (3) LaunchDol/facade seat;
(4) remaining pool leaves outside the asm-ten (DebugFont, SceneFog,
GinjaDrawUtil, ChunkModelDraw, TAdTrial siblings — future census, not this
doc's scope).

Repro: `cache/scratch/tl3/{tl3_verify.py,va_pair_check.py,tl3_receipts.json,
va_pair_receipt.json,ctl_*,patched_*,*.cpp}`. Inertness: zero repo builds,
zero decomp writes, official corpus untouched, pin `d4382fcd…` unmoved
(re-hashed pre and post, identical).
