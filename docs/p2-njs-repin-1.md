# P2-AR — NJS handle re-pin prototype: measure, don't adopt (evidence pack)

Phase-2 lane AR, 2026-11-26. Decomp `pso-decomp @ 6b38b5383` READ-ONLY;
**zero make/ninja** — every number below is a scratch `clang -fsyntax-only`
probe. Writes: `compat/njs/**` (new dir, inert) + this doc only.
Inputs: `docs/p2-rung4-contract.md` (AP: target TU + 24 (d)-rows),
`docs/p1-datamodel.md` (J: rule-1 u32-handle+base), `docs/p2-njs-confirm.md`
(AJ: 7 slots, 5 obligations, LOCKED pair), `NinjaTypes.h:162-163` era asserts.

## 1. Slot inventory (from NinjaTypes.h + AJ obligation 1)

| struct | field | era off | era sz | LP64-H1 off/impact | re-typed? |
|---|---|---|---|---|---|
| NJS_OBJECT | evalflags | 0x00 | 4 | ok | no |
| NJS_OBJECT | **model** | 0x04 | 4 | `void*`→@0x08/8 → every field shifts | **u32** |
| NJS_OBJECT | pos[3] | 0x08 | 12 | @0x10 (collateral) | no |
| NJS_OBJECT | ang[3] | 0x14 | 12 | @0x1C | no |
| NJS_OBJECT | scl[3] | 0x20 | 12 | @0x28 | no |
| NJS_OBJECT | **child** | 0x2C | 4 | `NJS_OBJECT*`→@0x34 — **assert fires** | **u32** |
| NJS_OBJECT | **sibling** | 0x30 | 4 | @0x3C | **u32** |
| NJS_OBJECT | quat_w | 0x34 | 4 | @0x44; size 0x38→0x48 — **assert fires** | no |
| NJS_MKEY_F | keyframe,key[3] | 0x00-0x0C | 0x10 | **0x10 PASS under H1** (no ptr; H0 fired only because raw `u32`=`unsigned long`=8B) | no |
| NJS_TEXLIST | **textures** | 0x00 | 4 | 8B, no era assert → SILENT era-width break | **u32** |
| NJS_MOTION | **mdata** | 0x00 | 4 | 0xC→0x10, no assert → silent | **u32** |
| NJS_MDATA2 | **p[2]** | 0x00/0x04 | 8 | 16B, no assert → silent | **u32[2]** |
| NJS_MKEY_P | **key** | 0x04 | 4 | 8B, no assert → silent | **u32** |
| NJS_CNK_MODEL | **vertex_list,polygon_list** | 0x00/0x04 | 8 | 0x18→0x28 silent | **u32×2** |
| NJS_TEXNAME | **filename / texaddr/texture** | 0x00/0x08 | 0xC-rec | GinjaMaterialState.h (uaddr=uintptr_t→0x18) silent | **u32** (2nd shadow) |
| NJS_SPRITE / NJS_MOTION_LINK / NJS_POINT3COL / NJS_POINT2COL | ptr fields | — | — | runtime-only (no chunk tag in `read_ninja_chunk:350-392`; POINT3COL = AJ RUNTIME-WAIVE) | left host ptrs |

Era-offset truth column verified by executing the shadow's asserts (§3), not
by hand.

## 2. The shadow (compat/njs/, inert)

`compat/njs/pso/system/NinjaTypes.h` + `compat/njs/pso/render/GinjaMaterialState.h`
= era headers copied verbatim; **only field TYPES change** (names/order/offsets
identical), plus conversion helpers and `PSO_NJS_BYTE_TRUTH` pins (private
macro — doesn't disturb the waiver's fail-closed `PSO_LA_FLAG_<name>` dispatch;
the three era asserts keep their `LAYOUT_ASSERT` names so the shipped KEEP
flags now gate **passing** checks). Helpers mirror the bringup crt0 lineage:
`pso_njs_g2h(h)=pso_arena_base+(h-0x80000000)`, `pso_njs_h2g`, `NJS_HANDLE(T,slot)`;
slot==0 ↔ NULL preserved. Requires the port types.h rebind
(`host/bringup/include/types.h`, J rule-1) — with the era `unsigned long u32`
nothing is era-shaped anywhere.

## 3. Measurement (H0 raw / H1 port-rebind / H2 = H1 + `-Icompat/njs` first)

H1/H2 COMMON = P2-R flags + `-DPSO_PORT_LAYOUT_WAIVER -Ihost/bringup/include`
+ committed `-include compat/pso_metal_compat.h build/waivers/pso_assert_waivers.h`,
`-ferror-limit=0` (the 20-cap hid deltas; uncapped rerun). Corpus:
`cache/scratch/clangprobe/ar_repin/{H1,H2}/` (38 TUs).

**Target `TAdMultiChgGameModeGC_8023E7A8.cpp`** (AP §4):
H0 = fail (NJS pair + NinjaFloatKeySize + ItemDataSize + RW, capped 20).
H1 = 129 errors → **H2 = 127: `NJS_OBJECT_size` & `NJS_OBJECT_child_offset`
flip FAIL→PASS, ZERO new errors** (the TU touches no re-typed field as a C
pointer). `NinjaFloatKeySize` already passed at H1 (rebind effect, not shadow).
Residual: 108 assert sites on types already judged RUNTIME-WAIVE (the stitch
battery clears — the committed waiver file predates AL) + 15
unreviewed/blind-census names; `ItemDataSize` itself does **not** fire under
H1/H2 (the AP-2 "TItemData re-pin" need is not visible as this assert at the
port rebind — residual TItemData-family names are in the unreviewed bucket).
→ **Target verdict: NJS lock measurably removed by a header-only change;
TU still parse-blocked by the stitch queue + unreviewed types, not by NJS.**

**Controls:** `Vec2Constants.cpp` 2→0 **PASS**, `TWindowLockOn_8026CB80.cpp`
2→0 **PASS** (opaque NJS users: flipped, nothing broken);
AP's RW-only control `TKeyBoardControl_80290CB0.cpp` 25→23, NJS pair flipped,
0 new errors; `memory.cpp` 0/0 unaffected. **Zero regressions on controls.**

**Chain-24** (AP (d)-rows 5–33 → 19 unique owner TUs, +row-20/deep-chain 5):
17/19 unique TUs carry the NJS pair and **all 17 flip FAIL→PASS**. **0/19
become parse-clean** (none is NJS-only-blocked): residuals per class —
RW-asserts-pending-stitch everywhere (20–108 sites/TU), unreviewed/AK-blind
types (GinjaView 42, TWindowCtrlMain 67, SystemBootInit 62), verdict-open
`BmlModelEntry` (TCameraCtrl 9 sites), SystemBootInit keeps its **dialect**
cluster (30×`expected ')'` AT_ADDRESS legs), SinCosTable keeps its **asm**
block (2 errors, NJS never fired), AsyncFileRead/ControllerInput carry no NJS
assert at H1. Nearest-to-clean: `TObject.cpp` 11→6 (3 harness typedef-redefs,
1 RW, 1 unreviewed, `nm`); `ParticleDefinition.cpp` 19→17.
Row-20 hooks add the loud finding: `TAdSEGAlogo.cpp` +25 new,
`TTitleScene.cpp` +22, `TAdTitleGC.cpp` +6, `TObjBoss2Tunnel.cpp` +10,
`TChCrLoadReq.cpp` +8 (PPC-window comparisons), `TBoss1Dragon.cpp` +11,
`TechniqueRingwaveList.cpp` 2→10, `text_80327844.cpp` 2→3, `psoLoadTexture.cpp`
+3, `GinjaMaterial.cpp` +3 — **11 TUs of the 38 probed gain new C++ errors**
(static NJS-tree initializers + walk casts). **Fully H2-clean across the probe
set: Vec2Constants, TWindowLockOn, memory.cpp.**

## 4. AJ's 5 obligations → concrete loader/consumer sites

1. **BML/NJ* copies byte-true**: `read_ninja_chunk` `TLobby_801E2F94.cpp:334-430`
   — copy :400 unchanged; POF0 patcher :413+ keeps writing **4-byte** slots
   (value = guest-abs = copy-base-guest-addr + delta — translate, never widen).
   Consumers needing `NJS_HANDLE`/`pso_njs_h2g`: `BmlResource.cpp:427/441/453-463/478/481`
   (`(NJS_OBJECT*)data` → `slot = pso_njs_h2g(data)`), `TCAMCharacter.cpp:441,449`
   tree walks, `copy_model_node` `TBoss1Dragon.cpp:391` (memcpy 0x38 keeps slots —
   OK as-is), `TChCrLoadReq.cpp:34-35` window test → range-check on slot space.
2. **POF0 semantics**: same loader; nothing else walks POF0.
3. **Floor-set rows**: `TLobby_80209524.cpp:674/714/1063`, `TSetManager.cpp:192/511/590/876/1354/1504`
   — `EnemySetRecord.enemy`/`FloorObjectSetRow.object` are **FloorPopulation.h**,
   outside this prototype's 2 headers: named gap (3rd shadow, same mechanic).
4. **GVM/TLS logo path**: `psoLoadTexture.cpp:89` (`texlist->textures->filename`
   → double `NJS_HANDLE`), `:142-153` `LoadPvmFileTls` fills runtime TEXNAME
   (`filename/texaddr` write via `pso_njs_h2g`), `:97-101` saddr frees → slot
   compares; `GinjaTextureTable.cpp:18` `set_texture_name` field writes via h2g.
5. **Z-lineage wires**: `GinjaMaterial.cpp:478/495` DRAW_TPL untouched (opaque
   BE32 words); `:367/388/421` chunk-stream subscripts → `NJS_HANDLE` over
   CNK_MODEL slots (measured above). `GinjaSceneState.textures` stays host-ptr
   (runtime state, not image-resident).

## 5. Risk table (why "measure ≠ adopt")

- **Interpretation blast radius**: 357 direct-includer files (238 src),
  1015/976 TU traffic (tranche-1). Deref-site ceilings (name-generic, upper
  bounds): `->model` 1297 sites/306f, `->child` 400/139, `->sibling` 213/98,
  `->textures` 138/41, `->polygon_list` 77/27, `->filename` 18/11,
  `->mdata` 5/4, `->vertex_list` 15/6, `->texaddr` 10/6 (many sites are
  same-named fields of OTHER structs; ground truth = the compile census above).
- **C++ fails loud** (every bad conversion is a hard error — 11/38 sample TUs
  surfaced), **C fails SILENT** under `-w`: `gjSystem.c`-type era `.c` TUs
  would launder ptr↔u32 with truncation. Never enable compat/njs on a `.c` TU
  without `-Wint-conversion` unsuppressed.
- Worst cases today: static NJS trees with embedded pointers
  (TAdSEGAlogo:23-25, TTitleScene:58-60, TechniqueRingwaveList:6 — initializer
  lists are the +25/+22/+10 clusters), runtime-walk casts (BmlResource,
  TBoss1Dragon:382/384/407), PPC-window validation (TChCrLoadReq:38-39).
- Slot-space correctness assumption: all re-typed pointers must be
  arena-window-resident (guest `0x80xxxxxx` or `TQuestVmDownLoadServer::alloc`
  copies). Out-of-arena pointers (statics in ROM image space, stack
  descriptors) would mis-convert — per-site review, not free.
- The 4 era-struct SILENT sites (TEXLIST/MOTION/MDATA2/CNK_MODEL had **no**
  era assert): without the shadow they compile wrong at LP64 and corrupt
  images at runtime; with it they're pinned. Shadow strictly increases honesty.

## 6. Recommendation (integrator + Scott)

**Adopt per-TU opt-in**: add `-Icompat/njs` to the parse allow-list ONLY for
C++ TUs, staged with the stitch battery — the 17/19 chain flips cost zero
rewrite where fields are opaque (target included), and every consumer TU that
breaks breaks loudly with an enumerated fix (`NJS_HANDLE` at the §4 sites).
Do **not** go tree-wide yet: ~1000 TUs inherit the reinterpretation, the C-TU
silent-truncation class is unaudited, and the FloorPopulation gap (§4.3) is
open. Defer runtime enablement (arena-base binding, POF0 patcher wiring) to
the loader lane — this header changes *typing*, not behavior. If title-phase
pressure grows, the verbatim-slice fallback (AP §4) still bypasses parsing
entirely; the shadow's real payoff is making rung-4 TUs *parse* honestly.

## Repro

`cache/scratch/clangprobe/ar_repin_probe.sh` + `ar_analyze2.py` + `ar_chain.py`
(TMPDIR scratch; H0 = committed `lp64native/err/` corpus). Decisions: none
recorded in build/, host/, data/ — grep `compat/njs` in the repo resolves to
this dir + this doc only (proved at commit time).
