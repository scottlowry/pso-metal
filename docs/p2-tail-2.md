# P2-TL2 — post-apply-7 tail re-census, network decl-aligns, asm-ten port specs

Phase-3 lane TLP (card `t_bf0a9079`), fleet wave-3 2026-11-27. ZERO builds:
the official `build/parse/` corpus was **mid-battery** at lane intake
(`summary.txt` = `TOTAL=1680 FAIL=75 PASS=1605 (target=arm64_32-apple-watchos7.0)`
— the ILP32 leg, integrator busy), so per card law the LP64 census was
**re-measured by a fresh scratch sweep** (`cache/scratch/tlp/sweep_lp64.sh`,
`build/Makefile@9c5aba7` `parse-lp64` token order replicated verbatim, `-P6`).
Decomp `@6b38b5383` READ-ONLY; writes = this doc only.

## 0. Era certification (vigilance clause)

- Sweep result: `TOTAL=1680 FAIL=517 PASS=1163 (target=arm64-apple-macos12.0)`
  — byte-shape match to the apply-7 official expectation (1,163/517); the
  scratch corpus is usable as the census input.
- Waiver era: `build/waivers/pso_assert_waivers.h` sha256
  `d4382fcd0659357f…` == pin `host/bringup/waiver-header.sha256` (apply-7
  regen). **All numbers below certify the d4382fcd era.** Wave-2's TL table
  (`p2-tail-1.md` §1, `495cc91a` era) is superseded.
- Wall-family spot checks over the sweep's 517 `err/` logs (python):
  `OSTime.h:15` collisions = **0** (was 12 touched) — facade ROUTED and works;
  `hw_regs.h:4` raw-colon = **0** (was 2 first + 10 wall-behind) — pshim
  ROUTED and works; `pso_la_waived` redefinitions = **0** — the WT typedef-NOP
  mechanism change (`PSO_LA_NOP → typedef int`) absorbs both seats;
  `orig_asm/*.inc` misses = 12 logs (10 TL asm + 2 MX — the wave-2 forecast).

## 1. Re-bucketed first-error table (sums to 517 exactly; python, `first_errors.json`)

| first-error family | n | class / disposition |
|---|---|---|
| `NJS_OBJECT_size` assert, NinjaTypes.h:162 | **485** | §4.2 stage-3 residual (808-row allowlist absorbed ~805) — EXCLUDED (NC/NA) |
| `FloorObjectSetRow_size` assert, FloorPopulation.h:56 | **9** | §4.6 — EXCLUDED (NC/NA) |
| musyx TU-body `asm void` residues (chorus_fx.c, reverb.c) | **2** | §4.4 MX queue — EXCLUDED |
| **`unknown type name 'va_list'` @ era `os.h:40`** (`OSVReport`) | **7** | NEW first-error family (this lane's promotion find — §2) |
| TL asm blocks — `expected '(' after 'asm'` (SinCosTable.cpp:16, text_803D9650.cpp:6, text_8042E8C4.cpp:27) | **3** | UNATTRIBUTED §4 (Play-3 specs here; 2 of 3 flip per receipts) |
| TL decl-conflicts — 4 network `.c` | **4** | UNATTRIBUTED §3 — **fix specs §3, 4/4 flip clean** |
| TL u32-handle body dialect (GinjaMaterial.cpp:367, GinjaTextureTable.cpp:20, psoLoadTexture.cpp:89) | **3** | UNATTRIBUTED; none is on the 808-row allowlist — hand to njs stage-4 seat |
| TL intrinsic — `__builtin_va_info` (CriSvm.cpp:190, LscLog.cpp:16) | **2** | UNATTRIBUTED (MWCC `va_list` internals, era `libc/stdarg.h:17-20`) |
| TL TU-body LAYOUT_ASSERT (TProtocol.cpp:20, 6 PMF-offset diags) | **1** | verdict ask (embed-pin precedent unchanged) |
| TL MWCC-ABI remainder (SiPolling.cpp:29) | **1** | verdict ask (Cause-B precedent unchanged) |
| **TOTAL** | **517** | unattributed tail after exclusions = **21** (wave-2: 19) |

## 2. Promotion ledger — what apply-7 advanced, and the new wall behind it

Wave-2's 19 were the pool; the routed facade/pshim killed their first-walls and
the 808-row stage-3 allowlist moved the big wall families. Per-TU disposition
(scratch sweep, same-turn reads):

- **→ PASS (fully green now):** `KBDXfer.c` (hw_regs→pshim, nothing behind),
  `mdm.c` (OSTime→hw_regs→pshim, nothing behind), `McFsGc.cpp` (OSTime cleared).
- **→ NEW first-wall `os.h:40 va_list` (7):** `eth.c`, `gjLight.c`,
  `nuReLoadTexture.c`, `Gcci.cpp`, `SystemBootInit.cpp`, `NetworkConnect.cpp`,
  `GinjaDrawUtil.cpp`. Two more carry it as SECONDARY behind §4.2
  (`gjSystem.c`, `ChunkModelDraw.cpp`) — census says the family grows to ~9+
  when the wall program lands.
  **Mechanism (read, not guessed):** era `dolphin/types.h:61` is the corpus's
  `va_list` supplier (`→ libc/stdarg.h → PowerPC_EABI_Support` `__va_list[1]`
  struct); the collision pin OCCUPIES that guard and deliberately carries only
  the era tail minus the libc legs ("suppression stays lossless" —
  `pin_dolphin_types.h:38-43` — true for everything except `va_list`, which
  `os.h:40 OSVReport` needs and era `os.h` never includes stdarg for). The pin
  law (integrator-owned `compat/**`) makes this a supply ask, NOT a TL edit:
  minimal fix = carry `typedef struct {…} __va_list[1]; typedef __va_list va_list;`
  (PowerPC_EABI_Support `libc/stdarg.h:7-14` verbatim, guard-neutral since the
  real `_STDARG_H_` stays unoccupied) into the pin, or ship
  `compat/facade/dolphin/libc/stdarg.h`. The same era struct is what the
  `__builtin_va_info` pair (CriSvm, LscLog) reaches for — ONE supply plausibly
  addresses 7+2+2 = 11 tail TUs.
- **NJS wall absorption:** 1,290 (wave-2 `495cc91a` era) → 485: the stage-3
  rows flipped ~805; the four Play-3 wall-TUs (MatrixVector, MatrixTransforms,
  text_803D96A8, MotionContext) stayed walled (not allowlisted — §4).
- WT/dvd collateral families re-checked: zero first-errors, zero diagnostics
  anywhere. apply-7's routing bundle delivered its certified expectation.

## 3. Play 2 — the four network `.c` decl-conflicts: paste-ready PROPOSALS (decomp-relative, one line each)

Root cause (all four, verified): a function-local prototype inside the `.c`
TU-body disagrees with the file's own canonical decl (era header or file-scope
decl matching the cross-TU **definition**). Alignment direction = toward the
DEFINITION/era-header everywhere. Receipts:
`cache/scratch/tlp/play2_verify.py` + `play2_receipts.json` — unpatched copies
in the identical scratch shape reproduce the official first-error at the decl
site (control rc=1 ×4), patched copies give **rc=0, 0 errors: 4 of 4 flip
clean**. Diffs below are PROPOSALS; decomp was not touched.

```diff
--- a/src/pso/network/NetApi.c                                          (line 839)
-    s16 validate_tcp_handle(s16 handle);
+    s16 validate_tcp_handle(int handle);      /* align to NetTcpState.h:200 (K&R def NetTcp.c:2494 is s16→int-promoted, header is the canonical form) */
```
```diff
--- a/src/pso/network/NetDns.c                                     (lines 795 + 824)
-    s32 dns_encode_name(u8 *dst, char *src, s16 len);
+    s32 dns_encode_name(u8 *dst, char *src, s32 len);  /* align to file-scope :56 + definition :850 (s32 max); call sites pass s16 len — widening, legal */
```
```diff
--- a/src/pso/network/PppBuffer.c                                         (line 27)
-    void NetMemcpy(void*, const void*, s16);
+    void NetMemcpy(void*, const void*, int);  /* align to definition NetworkThreadRuntime.c:244 + compat/net NetworkRuntime.h:121 era-model shadow */
```
```diff
--- a/src/pso/network/PppFsm.c                                           (line 536)
-    u8 *write_network_u16_be(u8 *p, u32 v); /* declared here with other parameter types; the definition is u8 *(u8 *, u16) */
+    u8 *write_network_u16_be(u8 *p, u16 v); /* align to definition NetByteOrder.c:35 / Ppp.h:53 (decomp's own comment names u16 as truth) */
```

Net effect when landed with the §2 va supply: 4 flips now, no further wall
behind them (each control log shows EXACTLY 1 error). PppFsm.c's second-order
detail: its local decl is the OUTLIER (era definition takes u16; the u32
spelling would silently truncate at call sites — the alignment is a
correctness fix, not just a parse fix).

## 4. Play 3 — asm-ten port specs (batch A: six small math/unk leaves)

P2-V mirror law, stated up front: every leaf below HAS an era `.s` in
`build/GPOE8P/asm/pso/**` — that proves ONLY what the era machine executed.
The decomp bodies (`asm void/f32 …{#include "orig_asm/…inc"}`) are UNPORTED
either way; port decisions below cite the era trace as SEMANTIC ground truth
and the TU body as state. Trace addresses are the era `.s` comments.
Receipts: `cache/scratch/tlp/play3_verify.py` + `play3_receipts.json` +
`play3/MotionContext.ported.cpp` (+ `play3_receipt_mc2.json`). Two leaves flip
their WHOLE TU green (§0 wall-free files); the other four sit behind §4.2 —
their ported bodies provably add ZERO diagnostics (residual errors = the wall
+ untouched sibling asm blocks, none naming a ported function).

**4.1 `sin_table_lookup` + `cos_table_lookup` — src/pso/math/SinCosTable.cpp:16,20** (era 803D64C0 / 803D64DC)
Trace: `lis/addi/lwz` deref of `extern SinCosPair *sin_cos_table`
(AngleLookup.h:17); index via raw `0x54671B78` = `clrlslwi r7,r3,16,3` ≡
`rlwinm r7,r3,3,13,28` (bit-decoded) = `(angle & 0xFFFF) * sizeof(SinCosPair=8)`
— full 65536-pair turn, zero quadrant math; `lfs f1` at +0 (sin) / +4 (cos).
Port (C-legal): `return sin_cos_table[(u16)angle].sin;` / `.cos;`.
**Receipt: patched TU rc=0 — FLIPS PASS.**

**4.2 `vector_length` — src/pso/math/MatrixVector.cpp:18** (era 803D8BD8)
Trace: `lfs x,y,z; fmuls x*x; fmadds y*y,+; fmadds z*z,+` = `x²+(y²+z²)` in fma
chain → `frsqrte; fres` = MWCC's sqrt LOWERING (`fres(frsqrte(s)) ≈ √s`, 12-bit
est.). Port: `f32 s = __builtin_fmaf(z,z,__builtin_fmaf(y,y,x*x)); return
__builtin_sqrtf(s);`. Fidelity note: host `sqrtf` is correctly-rounded where the
era was a 12-bit estimate — document as a deliberate precision improvement, no
consumer needs the defect. **Receipt: zero ported-fn diagnostics; TU stays §4.2
(first-error NinjaTypes.h:162; sibling `matrix_identity` asm untouched, out of
batch A).**

**4.3 `normalize_vector` — src/pso/unk/text_803D96A8.cpp:66** (era 803D97B4)
Trace: same fma sum; `frsqrte f0` used DIRECTLY to scale the 3 lanes
(`stfs` back into v) but the RETURN value is `fres(f0)` ≈ √s. So era returns
the length while scaling by the *unrefined* rsqrt estimate. Port: scale by
`1.0f/__builtin_sqrtf(s)`, `return __builtin_sqrtf(s);` — note both era
approximations land on the precise path; bit-fidelity ask if a wave ever needs
it: `vrsqrt` intrinsic (arm64 has no drop-in; not justified — consumers are
camera/geometry). **Receipt: zero ported-fn diagnostics; §4.2 wall + sibling
`transform_point_by_current_matrix` asm remain.**

**4.4 `matrix_apply_translation` — src/pso/unk/text_803D9650.cpp:6** (era 803D9650)
Trace: `lfs f0, Unit01+4` (=1.0f, the TU ALREADY ships `static f32 Unit01[]`);
ps build `{x,y}`/`{z,1.0}` lane pairs via `ps_merge00`; per row i: 3×`psq_l`
(0x00/0x08/0x10 stride = rows of Mtx[3][4]), `ps_mul {mᵢ₀,mᵢ₁}*{x,y}`,
`ps_madd {mᵢ₂,mᵢ₃}*{z,1.0}+`, `ps_sum0`, `psq_st` → `m[i][3]`. Meaning:
`m[i][3] = (mᵢ₂·z + mᵢ₀·x) + (mᵢ₃·1 + mᵢ₁·y)` — translation composition into
the 4th column. Port (keeps the Unit01 data-dependency + trace add order):
`for (i<3){ f32 lo=__builtin_fmaf(m[i][2],z,m[i][0]*x); f32
hi=__builtin_fmaf(m[i][3],Unit01[1],m[i][1]*y); m[i][3]=lo+hi; }`.
**Receipt: patched TU rc=0 — FLIPS PASS** (whole 8-line file; Unit01 stays
referenced → no dead-static drift).

**4.5 `subtract_vector` — src/pso/math/MatrixTransforms.cpp:45** (era 803D9370)
Trace: `psq_l {v0,v1}/{sub0,sub1}` + word-1 pairs for z, two `ps_sub`,
`psq_st` back. Plain componentwise f32 subtract. Port: `v->x-=sub->x;
v->y-=sub->y; v->z-=sub->z;` — no fidelity caveats (IEEE single ops).
**Receipt: zero ported-fn diagnostics; §4.2 wall is the only residual (2 diags).**

**4.6 `convert_color_f32_to_u8` — src/pso/animation/MotionContext.cpp:388** (era 803D7FE0)
Trace: `mtspr GQR1, 0x00804804` arms f32→u8 SIMD store conversion; three
`psq_st` lanes map src `NJS_ARGB{f32 a,r,g,b}` (NinjaTypes.h:76-81) → dst
`_GXColor{u8 r,g,b,a}` (GXStruct.h:28-33): `a→+3` (word-1 single),
`{r,g}→+0,+1` (pair), `b→+2` — exact name-preserving copy, ZERO scaling
(GQR1 scale fields decode 0/1: plain truncating cvt + u8 saturation at store).
Port: `dst->a=(u8)src->a; dst->r=(u8)src->r; dst->g=(u8)src->g; dst->b=(u8)src->b;`
**plus one required supply line: `#include <dolphin/gx/GXStruct.h>`** after the
TU's first include — `_GXColor` is only forward-declared in that TU
(MotionContext.h:11) because the asm never named members; without it the port
draws `member access into incomplete type` (caught by probe, receipt mc1; the
fixed spelling draws ZERO ported-fn diagnostics in mc2). Deviation note:
`(u8)` cast is UB outside [0,255] where the era SATURATED; callers pass
0..255-range colors (GinjaMaterial.cpp:203) — if a wave wants range-proofing,
spell each lane `dst->c = (u8)(src->c < 0 ? 0 : (src->c > 255 ? 255 : src->c));`.
Sibling `convert_color_u8_to_f32` (:392) is NOT batch A — it stays queued.

**Wave ledger honesty (p2-tail-1 Play-3 expectation restated):** the two
flips above are real green arrivals; the four wall-TUs need the §4.2 stage-4
allowlist rows too — asm-ports alone move them 0/+0 until then. text_8042E8C4
`LaunchDol` remains the PPC-branch-thunk verdict ask (batch B, not specced here).

## 5. Ranking for the next wave (unchanged families marked)

1. va_list supply (§2 ask, 7 firsts + grows with §4.2; one header line)
2. §4.2/§4.6 walls (NC/NA seats — 494 of 517)
3. njs stage-4 rows for the 3 u32-handle TUs
4. asm batch A landings (receipts above: 2 flips now, 4 queued behind the wall)
5. va_info fixed-arity shim (2), verdict asks (2: TProtocol, SiPolling)

Repro: `cache/scratch/tlp/{sweep_lp64.sh,bucket.py,play2_verify.py,play3_verify.py}`,
`first_errors.json`, `play2_receipts.json`, `play3_receipts.json`, `patch/`,
`play3/`. Inertness: zero repo builds, zero non-doc writes; official corpus
untouched (integrator mid-battery at intake).
