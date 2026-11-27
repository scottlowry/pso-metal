# P2-AV — GX texture-consumer contract: AN pack → PAL8 binding → pixel

Lane AV (planning contract, read-only + this doc), 2026-11-27. Consumes: AN
`docs/p2-tex-supply.md` (commit `c140181`), AS `docs/p2-an4-triage.md` §4,
AR `docs/p2-njs-repin-1.md` (commit `36abf57`), AP `docs/p2-rung4-contract.md`
(GX boundary `begin_gx_frame` = `src/pso/render/gjSystem.c:41`, AP row 32),
AQ display path (commit `ce7090d`), AT/stitch facts (`b85a254`, resolutions-5
`f7655fc`). Decomp refs are `pso-decomp @ 6b38b5383`-relative; metal refs are
marked (metal). This doc writes NO code; it is the executable spec for the
post-AU consumer lane. AU (`docs/p2-tex-upload.md`) is IN FLIGHT at authoring
time — everything AU settles is an explicit **[AU-FILL]** slot below (§1.4, §6).

## 0. Standing robustness clauses

- **R1 — no load-bearing waiver constants.** The current header is
  `build/waivers/pso_assert_waivers.h` "2179 waived LAYOUT_ASSERT(s) over 934
  RUNTIME-WAIVE type(s), 1154 keep-flags" (header:2-3; 2179+1154=3333). D8
  (metal `host/bringup/bringup.mk:56-65`) **verifies** the integrator-pinned
  sha in `host/bringup/waiver-header.sha256` (`c1a64d98…`, file:1); it never
  regenerates. AT resolutions-5 (566/566 tail) means a battery-2 regen is
  pending: the sha and counts WILL move. No consumer/host code may branch on
  2179/934/3333 or any header hash — the sha file is the only era pin.
- **R2 — MSL scalar-read ban** (from AQ): see §2.5. Hard MUST-AVOID, not style.
- **R3 — slices stay era-verbatim** (AN §3/AN-2); psoBe/psoProt traffic only
  through `compat/pso_byteswap.h` wrappers (metal `host/bringup/ladder4_supply.c:16-27`).

## 1. INPUT contract — what the consumer binds

### 1.1 The pack (era-authoritative, AN §6 = docs/p2-tex-supply.md:120-124)

```
GinjaTextureHeader-shape { format=9, width=32, height=32, size=1024,
                           flags=0x8000 /* colour-indexed */,
                           pixels=<img+0x1C, byte-true PAL8 texels> }
id=0x999   tlut=0
```
Golden integrity pins (metal): texels fnv1a64 `0xa92d59be5839fce5`, image fnv1a64
`0x93b9be4e18606d07` (docs/p2-tex-supply.md:109-114; fixture
`host/bringup/fake_disc/sinbd_tex.gvr`, 0x41C B, :12). Header field offsets are
era-store-pinned: format@+0x00, width@+0x0C, height@+0x10, size@+0x14
(`src/pso/render/GinjaTextureHeader.cpp:65-67,82`; metal mirror comment
`host/bringup/ladder4_test.cpp:90`).

### 1.2 The consumer door (4-fn era chain, AS §4 = docs/p2-an4-triage.md:92-97)

| era addr | fn | metal job |
|---|---|---|
| 0x803DA12C | `init_texture_header_and_alloc(void*)` 1-arg wrapper (def `src/pso/unk/text_803DA12C.cpp:9`) | **[AU-FILL S1]** upload entry — AU owns spelling |
| 0x803DB0AC | `init_ginja_texture_header(hdr,w,h,fmt)` (`GinjaTextureHeader.cpp:62-84`) | 4-arg field-fill + flags (0x200→0x8000 :75-77) |
| 0x803DB15C | `get_texture_buffer_size` (`:86-94`) → `GXGetTexBufferSize` (decomp `libs/dolphin/src/gx/GXTexture.c:98`) | **[AU-FILL S3]** host `GXGetTexBufferSize` symbol |
| 0x803DB1B4 | `alloc_buffer_from_heap` (`:96-105`, `OSAllocFromHeap(locked_cache_heap_handle)`) | **[AU-FILL S2]** host allocator seam |

### 1.3 The named consumer chain (AN §6:126-136, committed era bodies)

`load_texture_entry(list,index)` switches on `textures[index].attr & 0x5C000000`;
attr `0x40000000` = `NJD_TEXATTR_TYPE_MEMORY` (decomp `include/pso/system/NinjaTypes.h:138`)
→ `loader=1` → `texture_loaders[1] = load_texture_from_memory`
(`src/pso/unk/text_803DD5E0.cpp:26-27,46-63`) → whose memory-input leg
`:139-144` reads `GinjaTextureInput{width,height,format,data}` **directly** —
that is exactly the door the AN pack fits → `bind_texlist_texture`
(`src/pso/unk/text_803DA798.cpp:191`) → `create_texture_state`
(`text_803DA798.cpp:132-158`: `init_texture_from_gvr_desc` :141, **tlut kept iff
`is_palette_texture_format(format)` :148-152** — predicate `case 8/9/10` in
`src/pso/ui/frontend/TAdSinglePlyChrSelectGC_803DA038.cpp:26-34`, so format 9
⇒ tlut_name=0 retained, NOT −1) → `reload_texture_slot_pixels(slot,chunk)`
(`text_803DA798.cpp:168-179`) → `copy_texture_pixels`→
`copy_texture_data_and_flush(hdr->pixels,src,hdr->size)` (`:82-85`).
Caller shape: `load_sin_board_image` (`src/pso/object/TObjSinBoard.cpp:111-121`):
`set_texture_name(&name,&surface,0,0x40000000)` (:119) then load.

### 1.4 [AU-FILL] slots — FILLED by integrator 2026-11-27 from AU's committed §6 (`docs/p2-tex-upload.md`, commit 7d076d5)

- **S1** RESOLVED: consumers take `GinjaTextureHeader*` (port mirror; era pixels
  slot **+0x1C** holds a heap texel buffer of exactly `header->size` bytes,
  internally allocated). Handoff shape: `reload_texture_slot_pixels(slot,
  header->pixels)` / `create_texture_state` (text_803DA798.cpp:132-158).
- **S2** RESOLVED: allocator = era `alloc_buffer_from_heap` → `OSAllocFromHeap`
  on the port's arena-backed OSAlloc (AU E4a: cell {size 1056, requested 1024,
  hd_idx} pinned vs 0x80370BEC).
- **S3** RESOLVED as GX-lane work: `GXGetTexBufferSize` exists verbatim in the
  bringup slices; consumer-side use is via `GXInitTexObj*` in the AR allow-list
  stage (touchpoint quoted in AU §6; do NOT widen GXInitTexObj params).
- **S4** RESOLVED: AU's +0x1C slot currently holds the HOST u64 pointer — the
  §3 adapter MUST stamp it through the guest-address shim (`pso_njs_h2g`) before
  GX sees it (AU: "must be stamped ... before GX sees it"); once the texlist
  binds, `NJS_TEXNAME.texattr` and table pointers travel as u32 handles only.

## 2. BINDING map — format9 PAL8 → MTLTexture (PAL8 label DISPUTED for fmt-9 records — issue #8: is_palette_texture_format{8,9,10} ⇒ CI14 class; the 1 B/texel SIZE class below is era-table-proven and unaffected)

1. **bpp truth**: `GXGetTexBufferSize` groups format `case 0x9` with `GX_TF_I8`
   in the 1-byte/texel class (decomp `libs/dolphin/src/gx/GXTexture.c:63-76`;
   enum base `libs/dolphin/include/dolphin/gx/GXEnum.h:143-151`). ⇒ PAL8 (label DISPUTED — issue #8) =
   1 B/texel, `size = w*h = 1024` — AN-4's "re-derive from real header fn" is
   now satisfied with evidence, no assumption left.
2. **Rung-5 binding (CHOSEN): CPU CLUT-expand → RGBA8**, feeding the proven AQ
   display entry `gxh_dl_set_texture(w,h,rgba8)` (metal `host/gx/dl/gx_dl.h:81`,
   impl `host/gx/dl/gx_dl_metal.m:159-177`):
   - pixelFormat `MTLPixelFormatRGBA8Unorm`, `mipmapped:NO` (:161-163)
   - `usage = MTLTextureUsageShaderRead` (:164); `newTextureWithDescriptor` (:165)
   - `replaceRegion … bytesPerRow = w*4` (:166-167)
   - **storageMode: leave the descriptor default** (Shared on Apple Silicon) —
     AQ never set it and the `getBytes` roundtrip verify (:168-175) only holds
     under that regime; do NOT switch to Managed without re-proving the roundtrip.
   - palette source: fixture `tlut=0` and no CLUT chunk exists yet
     (`init_and_load_tlut` era home `GinjaTextureHeader.cpp:56-60` stays unported
     this rung) → use a **pinned stand-in palette** (AQ's 16-distinct-color
     diagnostic or a gray ramp) golden-checked, labelled DEVIATION AV-1.
3. **Sample state**: `minFilter=magFilter=Nearest`, `s/t=ClampToEdge`
   (:150-151) exists only for the FSHOP=3 `sample()` variant; the shipped fetch
   is `read()` so the sampler stays out of the failure surface (:48-49).
4. **Deferred alternative** (era-faithful, post-tlut-chunk): keep indices as
   `MTLPixelFormatR8Unorm` `bytesPerRow=32` + palette texture + in-shader LUT.
   NOT rung 5 — nothing before §3's real CLUT supply exists.
5. **MUST-AVOID (R2)**: the scalar-coordinate overload
   `texture2d<float>::read(size_t x, size_t y)` is **miscompiled by the runtime
   MSL compiler on this machine** — dynamic coords return the diagonal
   `texel(cx,cx)` at cy=0 and `(0,0,0,0)` for every cy>0; even a literal
   `read(2u,2u)` on a verified texel reads zero (metal
   `host/gx/dl/gx_dl_metal.m:50-56`; `docs/p2-gx-dl2.md:22-38`). Every consumer
   fragment MUST use the vector form `tex.read(uint2(cx,cy))` with shader-side
   uv→texel floor (`:63-65`, shipped FSHOP=6 body `:91-94`). New read() sites
   are reviewed against this rule; "simplify back to scalar" is a forbidden edit.

## 3. The 6 NJS_HANDLE sites (AR §4) — adapter + logo-path verdict

Shadow pair = `compat/njs/pso/system/NinjaTypes.h` +
`compat/njs/pso/render/GinjaMaterialState.h`; helpers `pso_njs_g2h` /
`pso_njs_h2g` / `NJS_HANDLE(T,slot)` at `compat/njs/pso/system/NinjaTypes.h:43-53`
(era access idiom `NJS_HANDLE(NJS_TEXNAME, tl->textures)` :73; conversion list
:243-245). Slot==0 ↔ NULL preserved (docs/p2-njs-repin-1.md:43-44).

| # | site (decomp) | current code path | minimal adapter | logo? |
|---|---|---|---|---|
| 1 | `GinjaTextureTable.cpp:18-28` `set_texture_name` | `entry->filename = reinterpret_cast<char*>(input)` (:20/:24) + attr/global_index writes | store `pso_njs_h2g(input)` into the u32 slot (both attr legs) | **ON** — TObjSinBoard.cpp:119 caller |
| 2 | `psoLoadTexture.cpp:89` | `texlist->textures->filename` (double deref) | `NJS_HANDLE(NJS_TEXNAME, texlist->textures)` then `pso_njs_g2h` of `filename` for `%s` | **ON** (AR §4.4 GVM/TLS logo path) |
| 2b | `psoLoadTexture.cpp:97-101, 142-153` `LoadPvmFileTls` | `res->textures=(NJS_TEXNAME*)(res+1)` (:145), `ent->filename=str` (:149), saddr free compares (:97-101) | writes = `pso_njs_h2g()`; frees/saddr = slot compares, zero host ptr into TEXNAME | **ON** |
| 3 | `GinjaMaterial.cpp:367, 388-389, 421` | `current_render_context->textures->textures[texture].texture->surface…` subscripts | `NJS_HANDLE` over CNK/TEXLIST slots (measured +3 errors, AR:85-86); DRAW_TPL legs `:478/:495` `GXCallDisplayList` = opaque BE32 words, **no change** (AR §4.5) | **ON** (draw side of the textured quad) |
| 4 | `BmlResource.cpp:427-428, 441, 453-463, 478, 481` | `*slot=(NJS_TEXLIST*)data`, `(NJS_OBJECT*)data`, `static_cast<NJS_OBJECT*/NJS_TEXLIST*>(data)` | writes = `pso_njs_h2g(data)`; readers wrap `NJS_HANDLE` | OFF — defer (BML model loads) |
| 5 | `TCMCharacter.cpp:439,441,446,449` (AR wrote "TCAMCharacter" — real path `src/pso/ui/charcreate/TCMCharacter.cpp`) | `(NJS_OBJECT*)char_creation_data_block[..]` walk casts | `NJS_HANDLE(NJS_OBJECT, slot)` at walk entry | OFF — defer (char create) |
| 6 | `TChCrLoadReq.cpp:30-42` | PPC-window range checks on `object->child/sibling/model` (:38-40), static bounds :34-37 | compare in **slot space** (guest-window bounds, no convert) — AR §4.1 | OFF — defer |

**NJS_HANDLE on-logo = 3 sites** (#1 #2/#2b #3). FloorPopulation: NOT a consumer
edit here — `EnemySetRecord.enemy`/`FloorObjectSetRow.object`
(`TLobby_80209524.cpp:674/714/1063`, `TSetManager.cpp:192/511/590/876/1354/1504`,
AR §4.3) need a **3rd shadow** (`compat/njs/.../FloorPopulation.h`, same
mechanic): named follow-on, owned by the loader lane, DEFERRED out of rung 5.
read_ninja_chunk copy `TLobby_801E2F94.cpp:334-430` (:400) + POF0 patcher (:413+)
need **no change** — slots stay 4B, translate never widen (AR:92-94, adopted policy).

## 4. ALLOW-LIST mechanics — integrator build/ edit request (ZERO lane action)

AR §6 policy adopted: per-TU C++ opt-in, `-Icompat/njs` **earliest-I** (before
`SDK_INCS`, so `#include "pso/system/NinjaTypes.h"` resolves to the shadow);
NEVER on `.c` TUs (C launders ptr↔u32 silently under `-w`, AR §5). Requested
edit to metal `build/Makefile` (anchor points: `SDK_INCS` :34-38, `COMMON`
:43-44, `PARSE_RECIPE` :71-83):

```make
# P2-AV: per-TU C++ NJS shadow opt-in (docs/p2-njs-repin-1.md §6,
# docs/p2-gx-tex-consumer.md §4). C++ TUs ONLY — list is enforced by suffix.
NJS_ALLOWLIST := $(BUILD_DIR)/njs_allowlist.txt
```
inside `PARSE_RECIPE`'s `bash -c` body, before the `case`:
```make
  extra=""; case "$$f" in *.cpp) grep -qxF "$$f" $(NJS_ALLOWLIST) && extra="-I$(REPO)/compat/njs" ;; esac; \
```
and `$$extra` spliced **first** on the clang++ line: `clang++ $$extra $$CXXFLAGS_PARSE "$$abs"`
(the `*.c)` branch stays untouched — .c units never receive the flag).

New integrator file `build/njs_allowlist.txt`:
```
# stage 1 (rung-5 logo trio; lands with §3 #1-#3 edits):
src/pso/render/psoLoadTexture.cpp
src/pso/render/GinjaTextureTable.cpp
src/pso/render/GinjaMaterial.cpp
# stage 2 (deferred, §3 #4-#6):
src/pso/system/BmlResource.cpp
src/pso/ui/charcreate/TCMCharacter.cpp
src/pso/ui/charcreate/TChCrLoadReq.cpp
```
Integrator acceptance gate: unlisted TUs keep byte-identical parse output vs
the prior corpus run (zero-regression, AR controls precedent :67-70); listed
TUs' error delta equals exactly the AR §3 loud clusters (+3 psoLoadTexture,
+3 GinjaMaterial, +6 GinjaTextureTable-neighborhood) until the §3 adapter edits
land, then 0.

## 5. Ordered rung-5 task list (post-AU lane)

| # | goal | files it may touch | est | THE gate |
|---|---|---|---|---|
| V1 | Resolve S1–S4 against landed `docs/p2-tex-upload.md`; intake test binds pack BY OFFSET | host/bringup/ladder5_test.cpp (extend), this doc's slot table | S (≤2h) | pack golden {9,32,32,1024,0x8000,id0x999,tlut0} + texels fnv `0xa92d59be5839fce5` @ offsets 0x00/0x0C/0x10/0x14 |
| V2 | Call-through: AU upload entry fed by pack writes the header + pixels | host/bringup/ladder5* | S (≤2h) | wrapper ret==0, hdr.size==1024 via REAL `get_texture_buffer_size→GXGetTexBufferSize`, pixels fnv unchanged |
| V3 | Consumer-chain port: `load_texture_entry→load_texture_from_memory(:139-144 leg)→bind_texlist_texture→create_texture_state→reload_texture_slot_pixels` as guarded verbatim slices + 1-entry slot pool | host/bringup/ladder6_* (new), bringup.mk, run.sh | **HARD** (4-6h) | slot0 = {ref_count 1, surface->id 0x999, tlut_name **kept 0** (not −1) per :148-152}; run.sh BRINGUP GREEN, prior ladder lines insert-only |
| V4 | **LOGO VISIBLE**: CLUT-expand pack→RGBA8 (pinned stand-in palette, deviation AV-1) → `gxh_dl_set_texture` → sin-board rect through the AQ 7-pt driver, fragment = uint2 read only | host/gx/dl/** | M (2-4h) | `run_dl.sh` 7-pt gate samples_ok exit 0 ×2 same-header era; `GX_DL_TEXP` roundtrip 16/16 |
| V5 | On-logo adapters §3 #1-#3 + submit (not apply) §4 request | decomp-side edits go in the ported slices until TU parse lands; metal host/bringup only | M (2-3h) | scratch `-fsyntax-only` H2 probes: enumerated-fix-only residuals; integrator applies stage-1 list then parse parity |
| V6 | Deferred adapters §3 #4-#6 | same shape as V5 | M (2-3h) | per-TU probe parity; NOT on V4's critical path |
| V7 | 3rd shadow FloorPopulation (AR §4.3) | compat/njs/** (loader lane owns) | S | era LAYOUT_ASSERTs pass under 3rd shadow + new byte-truth pins |

**V4 is the task that puts logo art on screen end-to-end.** V1→V4 is the critical
path; V5-V7 compound honesty but do not gate it. Rung boundary (AP §2): consumer
sits strictly after `begin_gx_frame` (`gjSystem.c:41`) — every V-task runs inside
an opened GX frame via the dl lane, never before it.

## 6. Blocking unknown awaiting AU (one, stated loudly)

**The upload entry's exported shape and pixel-storage ownership (S1+S4):**
whether AU exposes the era wrapper spelling `init_texture_header_and_alloc(void*)`
(1-arg, 0x803DA12C) or a host-named seam, and whether `hdr.pixels` comes back as
arena-guest u32 or host pointer. Everything in V2–V4 waits on this one fact; the
offset-binding rule (§1.4 S4) is designed so no other slot blocks the plan.
