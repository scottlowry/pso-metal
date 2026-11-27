# P2-TL — §4.9 long-tail census on the post-apply-6 official LP64 corpus

Phase-3 lane TL (card `t_af1ef22d`), 2026-11-27. ZERO builds: every number is a
python pass over the committed official corpus `build/parse/{fail.txt,err/}`
(uncapped `-ferror-limit=0` logs). Decomp `@6b38b5383` READ-ONLY; writes = this
doc only. Per-TU receipts: `cache/scratch/tl/tl_census.json`.

## 0. Input integrity + era certification (vigilance clause)

- `build/parse/summary.txt`: `TOTAL=1680 FAIL=1330 PASS=350 (target=arm64-apple-macos12.0)`
  — target string says **macos** (not watchos), so the battery is not mid-run; the
  350/1330 split matches the apply-6 expected shape. All 1,330 fail rows map 1:1
  to a non-empty `err/` log; every log yields a first `error:` diag.
- Waiver-header era: `build/waivers/pso_assert_waivers.h` sha256
  `495cc91a5b96…` == pin `host/bringup/waiver-header.sha256` content
  (re-pinned by apply-6 `b59e8b1`). **These numbers certify the NEW (post-apply-6)
  era** — MA's §3 table (1,259/1,301 H3 sweeps, header `c6b51403`) is the OLD era;
  the tail shape below differs from MA §1 accordingly.

## 1. First-error bucket table (sums to 1,330 exactly; python)

The apply-6 wave (PsoCommandHandler RW, compat/net, compat/repin, musyx-as-C++)
collapsed the battery-2 multi-assert wall: of the 1,302 negative-array
first-errors, 1,301 fire at exactly TWO era-header sites:
`include/pso/system/NinjaTypes.h:162` (`NJS_OBJECT_size`) and
`include/pso/map/FloorPopulation.h:56/60` (`FloorObjectSetRow_size` /
`EnemySetRecordSize`). Named-class families + TL tail:

| first-error token family | n | class / disposition |
|---|---|---|
| `NJS_OBJECT_size` assert, NinjaTypes.h:162-163 | **1,290** | §4.2 stage-1/2 NJS allow-list wall — EXCLUDED (NC/NA seats) |
| `FloorObjectSetRow_size`/FloorPop row asserts | **12** | §4.6 — EXCLUDED (NC/NA) |
| `conflicting types for 'OSGetTime'` @ era `libs/dolphin/include/dolphin/os/OSTime.h:15` (s64-vs-u64 vs rebind) | **9** | D4 facade family — EXCLUDED/CROSS-REF (supply `compat/facade/dolphin/os/OSTime.h` EXISTS, unrouted; brief said "OSTime four", live = 9 first-error + 3 more behind walls = 12 touched) |
| `use of undeclared identifier 'OSAlloc'` (GcrnaStream.cpp:415) | **1** | D4 §4.8 — EXCLUDED/CROSS-REF |
| musyx TU-body `asm void` + missing `orig_asm/*.inc` (chorus_fx.c:113, reverb.c:86) | **2** | §4.4 — EXCLUDED (MX named WAIVE/facade queue) |
| **TL asm blocks** — `expected '(' after 'asm'` TU-body | **3** | UNATTRIBUTED §3 |
| **TL covered-unrouted** — `expected ';' after top level declarator` @ `hw_regs.h:4` raw-colon `u16 __VIRegs[59] : 0xCC002000` | **2** | UNATTRIBUTED §3 (fix = route existing P2-T supply, zero source edits) |
| **TL decl-conflicts** — `conflicting types` static def vs era header decl (4 network .c) | **4** | UNATTRIBUTED §3 — CROSS-REF NP (`p2-pppframe-1.md` §2 R-classes pre-listed exactly these: "Q+R (1: NetApi.c)" + "R (3)") |
| **TL subscript-on-int / u32-handle** (subscripted value / member ref on u32 / u32=char*) | **3** | UNATTRIBUTED §3 (NJS-handle dialect in TU BODY — outside §4.2 header-wall; MA `trio2_*` probes already measured these 3 TUs) |
| **TL intrinsic** — `__builtin_va_info` (MWCC va_list internals) | **2** | UNATTRIBUTED §3 (DebugFont.cpp carries the 3rd, hidden behind D4 wall) |
| **TL TU-body era LAYOUT_ASSERT** — `array size is negative` in TU (TProtocol.cpp:20-25, 6 diags, PMF callback offsets e.g. `TradeResultCallbackOffset == 0xBC9C`) | **1** | UNATTRIBUTED §3 verdict-ask |
| **TL remainder** — `no matching function for call to 'SIRegisterPollingHandler'` (MWCC-ABI, SiPolling.cpp:29) | **1** | UNATTRIBUTED §3 verdict-ask |
| **TOTAL** | **1,330** | unattributed after exclusions = **19** |

Zero first-errors in: PK families (`PsoCommandHandler` fires in 0 logs — apply-6
RW absorbed §4.1's cluster), PppFrame (`compat/net` absorbed §4.3's 50),
`pso_la_waived` redefinitions (see §5 CROSS-REF WT — 2 TUs, all behind walls),
"char-create AFS asserts" (§4.9 row-9 family name: **no live member** — the 19
`src/pso/ui/charcreate/*` fail TUs all first-error on the §4.2 NJS wall; the AFS
assert names fire nowhere in the corpus), and TBoss6Type1 negative arrays (gone —
no such site fires).

## 2. MA-tip reconciliation ("28 compat/dialect-covered-but-unrouted")

MA §1's covered-but-unrouted family at the old era = 27 musyx bare-`extern "C"`
+ 1 hw_regs raw-colon = **28**. Live: apply-6 routed `src/musyx/runtime/*` →
clang++ `-x c++` + `-Icompat/musyx`, and all 26 fail-listed musyx TUs except the
2 named MX residues now PASS. The family's surviving member is exactly the
hw_regs line: P2-T supply `compat/dialect/pshim/dolphin/hw_regs.h` EXISTS on
disk, is on NO `-I` line (`grep -rn pshim build/` = 0 flag hits) — 2 first-error
TUs (eth.c, KBDXfer.c) + 10 more touch it behind the NJS/D4 walls (12 total).

## 3. Per-file era site + minimal fix family — the 19 unattributed TUs

| TU | first-error site (era text) | fix family |
|---|---|---|
| src/pso/math/SinCosTable.cpp | :16 `asm f32 sin_table_lookup(s32) {` + fatal `'orig_asm/pso/math/SinCosTable/sin_table_lookup.inc' file not found` | per-TU portable port (leaf lookup) |
| src/pso/unk/text_803D9650.cpp | :6 `asm void matrix_apply_translation(MtxP,x,y,z)` + missing `.inc` | per-TU portable port |
| src/pso/unk/text_8042E8C4.cpp | :27 `asm … LaunchDol` + missing `.inc` | per-TU portable port / verdict ask |
| src/pso/network/eth.c | era `hw_regs.h:4` raw-colon pin | **-I routing** (`compat/dialect/pshim`) |
| src/pso/ui/keyboard/KBDXfer.c | era `hw_regs.h:4` | **-I routing** (same) |
| src/pso/network/NetApi.c | :839 `s16 validate_tcp_handle(s16)` static def vs era decl | per-TU 1-line port (NP R-family) |
| src/pso/network/NetDns.c | :795 `s32 dns_encode_name(u8*,char*,s16)` | per-TU 1-line port |
| src/pso/network/PppBuffer.c | :27 `void NetMemcpy(void*,const void*,s16)` | per-TU 1-line port |
| src/pso/network/PppFsm.c | :536 `u8 *write_network_u16_be(u8*,u32)` | per-TU 1-line port |
| src/pso/render/GinjaMaterial.cpp | :367 `textures->textures[texture].texture->…` u32-handle subscript | per-TU `-Icompat/njs` allow-list row (C++ TUs only — law) |
| src/pso/render/GinjaTextureTable.cpp | :20 `u32 = char*` handle assign | same allow-list row |
| src/pso/render/psoLoadTexture.cpp | :89 member ref on `u32` handle | same allow-list row |
| src/pso/audio/CriSvm.cpp | :190 `__builtin_va_info` | per-TU fixed-arity vararg shim / verdict ask |
| src/pso/audio/LscLog.cpp | :16 `__builtin_va_info` | same |
| src/pso/network/TProtocol.cpp | :20-25 six TU-body `LAYOUT_ASSERT(TradeResultCallbackOffset, offsetof(…) == 0xBC9C)` PMF-callback offsets | **verdict ask** (TProtocol = LOCKED embed-pin precedent, `p2-tail-verdicts.md` §3; era-asm mirror NOT the issue) |
| src/pso/system/SiPolling.cpp | :29 `SIRegisterPollingHandler(((call_si_polling_callback)))` MWCC-ABI fn-ptr arg | **verdict ask** (MWCC-ABI Cause-B precedent) |

Hidden behind the §4.2/§4.6/D4 walls (recensus when those land — do NOT count
now): asm-blocks **+7 more TUs** (MotionContext, MatrixTransforms, MatrixVector,
text_803D96A8, DebugFont, SceneFog, GinjaDrawUtil) — 10 TL-owned asm TUs total
(+2 MX = 12 corpus `orig_asm/*.inc` misses); va-info +1 (DebugFont);
hw_regs +10; OSTime conflict +3; EnemySetRecordSize +21 (all §4.6, NC CROSS-REF).

## 4. NEW FINDING — `compat/repin/dolphin/dvd.h` double-processing regression

12 fail TUs log `redefinition of 'DVDCommandBlock'/'DVDFileInfo'` +
`conflicting types for 'DVDGetCommandBlockStatus'` **from
compat/repin/dolphin/dvd.h** (era copy processed alongside MA's shadow via the
`<dolphin/dvd.h>` edge). This contradicts MA §2's "zero quoted-sibling includes
of these four headers … double-processing is impossible" claim for dvd.h
(os.h:88/card.h:6 carry quoted/angle edges). None is first-error today (walls
gate them) — every one becomes the NEXT wall the moment D4/NJS routing lands.
Minimal fix = one generator line: make the repin dvd.h shadow occupy the era
guard so the era copy skips (the pin's own trick), or guard just the re-emitted
structs. Blocked lines: the 10 D4-family TUs + eth.c, KBDXfer.c, gjSystem.c,
ChunkModelDraw.cpp (NinjaTypes-walled). **Integrator should NOT `-Icompat/facade`
in a wave that omits this fix.**

## 5. CROSS-REF rows (no double-count)

- **WT seat (waiver-dup redefinitions):** 2 TUs fire `redefinition of
  'pso_la_waived_*'` in-TU — `src/pso/quest/challenge/ChallengeMode.cpp`
  (`pso_la_waived_TChallenge_size`), `src/pso/unk/TLobby_80209524.cpp`
  (`…ChallengeEventHeaderSize`, `…ChallengeRandomEventSize`). Both first-error
  on the §4.2/§4.6 walls, so they are NOT in TL's 19; WT owns the rows.
- **D4 seat:** owns the 10 facade first-errors; TL measured the ask as
  flag-level (`-Icompat/facade` BEFORE the SDK -I list, never `-include`,
  collision law) — see Play 1.
- **MA seat:** owns §4 fix + its 38 TUs (none first-error for MA reasons now).
- **Era-asm mirror (P2-V law):** for the 10 asm TUs, `build/GPOE8P/asm` files in
  the decomp tree say NOTHING about port state — port decisions must cite the TU
  body + verdict, never the mirror.

## 6. Ranking + top-3 plays for the next build wave (≤2 builds each)

Ranked by (TUs × triviality): (1) routing bundle [14 TUs, flag/1-header level];
(2) njs stage-2 allow-list + net .c 1-line ports [7 TUs, receipts exist];
(3) decl-family misc [3]; (4) va-info [2-3]; (5) verdict asks [2];
(6) asm ports [10 TUs, real bodies — heaviest].

**Play 1 (one build, routing-only):** integrator splices into `LP64_REBIND`
(after `-Icompat/repin`, before SDK): `-I$(REPO)/compat/facade` +
`-I$(REPO)/compat/dialect/pshim`, PAIRED with the one-line `compat/repin/
dolphin/dvd.h` guard fix (§4). Certified expectation: 14 TUs shed their tail
first-errors (10 D4-family, 2 hw_regs, 2 wall-behind); expect a fresh
first-error split to §4.2 wall on gjSystem/ChunkModelDraw (hand to NC ledger).
Zero era-TU edits. Supply status: `compat/dialect/pshim/**` is committed and
inert; `compat/facade/**` was still UNTRACKED at TL commit time (D4 seat
mid-window) — Play 1 requires D4's ask landed (or the integrator landing it per
D4's `-I` ask block) or the facade half of the bundle has no supply to route.

**Play 2 (one build, 7 TUs):** the four network `.c` decl-conflicts as one-line
static-def/prototypes alignment (NP listed them; `clang -fsyntax-only`
scratch-verified pre-land) + three `-Icompat/njs` per-TU allow-list rows for
GinjaMaterial/GinjaTextureTable/psoLoadTexture (all .cpp — law OK; MA trio2
receipts already measured SHADOWFIRST clean legs; apply-6 kept
TChCrLoadReq commented — check its row while in the file).

**Play 3 (two builds, the asm ten):** batch A (build 1): six small math/unk leaf
bodies — `sin_table_lookup`, `vector_length`, `normalize_vector`,
`matrix_apply_translation`, `subtract_vector`, `convert_color_f32_to_u8` — as
portable C++ replacements for `asm void/f32 f(){#include "*.inc"}`, each
probe-verified fresh-file `-fsyntax-only`; batch B (build 2): DebugFont
`vec3_add_to`, SceneFog/GinjaDrawUtil `*_current_matrix_ps` pair, text_8042E8C4
`LaunchDol` (verdict ask: PPC branch thunk — may join MX's host-facade queue
instead). Expected ≤ +10 unattributed-to-green, minus whatever the NJS wall
exposes as new firsts (MotionContext et al. will re-present their wall first —
their green arrival needs the §4.2 program too; say so in the wave ledger).

TL numbers certify the `495cc91a` era; re-pin §1 when the official post-Play-1
battery lands.
