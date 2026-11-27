# P2-AJ — Lane-J handle gate: the 7 `lane-j-handle` types settled (pre-stitch)

Phase-2 lane AJ. Settles the 7 types flagged `action-needed=lane-j-handle` in
`data/corpus-corrections.tsv` (rows 2, 3, 182, 183, 376, 484, 498) into
`data/verdict-resolutions-3.tsv` — one positive resolution row per type, which
overrides the tranche UNRESOLVED rows at stitch (same consume-last rule AD
established in `p2-verdict-resolve-1.md` §2). Decomp `6b38b5383` READ-ONLY,
zero builds. **All `file:line` cites below resolve against the pso-decomp tree
at `6b38b5383`** (convention inherited from resolutions-1/2).

Result: **6 LOCKED-FORMAT / 1 RUNTIME-WAIVE**. Five rows CONFIRM AD's existing
LOCKEDs (resolutions-1:16,17,52,60,61) with direct evidence replacing two
FAMILY-PRECEDENT MED anchors; one row FLIPS AD (NJS_POINT3COL); the two
tranche-1 NJS rows get their first positive verdict.

## The decisive shared finding: `read_ninja_chunk` + POF0

`src/pso/unk/TLobby_801E2F94.cpp:334-430` is the era's Ninja image loader. It
switches on FourCC chunk tags — `NJIN`, **NJCM/GJCM** (:355), NJBM,
**NJTL/GJTL** (:362), NJLI, NJCA, **NDMD/GMDM** (:372), NLIM, NCAM, NSSM —
`TQuestVmDownLoadServer::alloc(size)`s a buffer, copies the chunk **verbatim**
(`move_data(buf, p, size)`, :400), then walks the trailing **`POF0`
pointer-offset table** (:413) patching the 4-byte pointer slots inside the
copied bytes. So every struct cast onto such a buffer must keep the era
byte-layout: pointer slots stay 32-bit PPC handles patched against the copy
base — exactly p1-datamodel §3 rule-1's u32-handle + base class. The
`BmlResource.cpp` walk (`allocate_bml_chunk` over `lobby_download_buffer`,
:420-485, quest-bin/DVD bytes) applies the same in-place-cast discipline.
A WAIVE on any of these types tells gen_waivers the host layout is
authoritative; an 8-byte `model`/`child`/`filename` read against POF0-patched
image bytes yields garbage pointers → silently corrupted rendered assets.
This is why the NJS_* rows stay LOCKED while lane J's handle conversion
(§"what J must cover") remains the runtime fix — matching AD's framing at
`p2-verdict-resolve-1.md` §5.2 ("rows stay LOCKED meanwhile").

## Per-type evidence

### NJS_OBJECT — LOCKED-FORMAT (CONFIRM tranche-1 direction; HIGH)
Declaring struct `include/pso/system/NinjaTypes.h:151-164` (`u32 evalflags;
void *model; f32 pos/ang/scl; NJS_OBJECT *child,*sibling; f32 quat_w;`,
LAYOUT_ASSERT 0x38, child@0x2C). Direct overlay casts of loaded byte images:
`src/pso/system/BmlResource.cpp:441` `*self->model = (NJS_OBJECT *)data;` and
`:478` (NJCM tag branch, quest-bin buffer). Era image pointers confirmed by
the PPC-window validation `TChCrLoadReq.cpp:34-35`
(`(NJS_OBJECT *)0x0 … (NJS_OBJECT *)0x200000`) and tree-walk casts at
`TCMCharacter.cpp:441,449`. Runtime trees are flat *copies* of the image
(`copy_model_node`, `TBoss1Dragon.cpp:391`; capture arenas
`TBoss1Dragon.cpp:538` / `TBoss8Dragon.cpp:390`, `nodes*0x50` stride — the
game's own fixed copy-stride, also format-adjacent) — the overlay is
canonical, so no (c)-split: LOCKED, waiver forbidden.

### NJS_MKEY_F — LOCKED-FORMAT (CONFIRM; upgrades AD's FAMILY-PRECEDENT to direct evidence, HIGH)
Struct `NinjaTypes.h:235-239` (`u32 keyframe; f32 key[3];`, assert 0x10) has
**no pointer member** — its era layout is LP64-invariant, so the lock is
zero-cost and only guards against future field drift. It is nevertheless
image-resident: NDMD/GMDM chunks (:372) are copied verbatim →
`AnimationMotion*` (BmlResource.cpp:453-463) → `NJS_MOTION::mdata` →
`NJS_MDATA2::p[0]` (`void*` into the same image) →
`src/pso/boss/dragon/TBoss1Dragon.cpp:574,:595`
`nodes = static_cast<NJS_MKEY_F *>(data[i].p[0]);` and the game then
**mutates the loaded keys in place** (`nodes[25].key[1] -= 0.2f;` …), which
only makes sense over image bytes, not a host-built copy.

### NJS_POINT3COL — RUNTIME-WAIVE (⚠ FLIPS AD resolutions-1:52 FAMILY-PRECEDENT MED; HIGH)
Struct `NinjaTypes.h:64-69` (`NJS_POINT3 *p; u32 *col; NJS_TEX *tex; u32
num;`), asserts `include/pso/render/TriangleFan3D.h:10-12`. Exhaustive census:
the type occurs in **6 TUs / 13 sites total**, all category (b):
in-code static descriptors initialized with *compiled* array addresses
(`TAdRicoRingCtrl.cpp:48`, `UISprite.cpp:373`, `TObjRain.cpp:27`), stack-local
descriptors filled field-wise per frame (`TBoss6Zanzo.cpp:65-95`,
`TObjSinBoard.cpp:130`), and the GX consumer `draw_tri_fan_3d`
(`TLobby_801E2F94.cpp:292-322`) which walks it into `GXBegin`/`POS3F32`
writes. **Zero** overlay casts, zero `memcpy` from buffers, and
`read_ninja_chunk` has **no POINT3COL-bearing chunk tag** (:350-392). The
era's disc-resident colored-vertex groups arrive instead as DRAW_TPL GX
command streams replayed via `GXCallDisplayList`
(`src/pso/render/GinjaMaterial.cpp:478`) — byte wires that never
materialize an `NJS_POINT3COL` struct (host-side wire by the Z-lane, so a
host struct can't leak into it). AD's lock cited "Point3COL in the loaded
triangle streams" — that claim has **no supporting site**; the family
precedent fails because the family's other members are precisely the
POF0-patched pointer structs this one is not. The 0x10 assert fails on LP64
only because the descriptor legitimately holds *host* pointers to host
vertex arrays → waiving it cannot corrupt any image.
**What would flip this back**: any `(NJS_POINT3COL *)` /
`memcpy(...,NJS_POINT3COL)` of DVD/archive/quest-buffer bytes, a new chunk
tag whose records are walked as POINT3COLs, or GVR display-list-side evidence
that scene files embed stride-0x10 point/col/tex/num records.

### NJS_TEXNAME — LOCKED-FORMAT (CONFIRM; upgrades resolutions-1:60 MED→HIGH)
Struct `include/pso/render/GinjaMaterialState.h:37-45` (`char *filename; u32
attr; union { uaddr texaddr; GinjaTextureState*; u32 global_index; }`) —
pointer-bearing; the decomp has already laundered the image slot to `uaddr`
(lane-J type). Inside an NJTL image the `textures` pointer (POF0-patched)
addresses the texname records, so the layout is on the loaded-image path at
`BmlResource.cpp:427` (cast of chunk bytes to NJS_TEXLIST, records walked
thereafter; `psoLoadTexture.cpp:89` dereferences `texlist->textures->filename`
on exactly such lists). The GVM runtime builder `LoadPvmFileTls`
(`psoLoadTexture.cpp:142-153`) allocates `count*0x2C+8` and fills the **same
struct** — runtime list and image list are one layout, which is precisely why
the type cannot be waived. `set_texture_name`
(`src/pso/render/GinjaTextureTable.cpp:18`) writes entries field-wise at
runtime (J's conversion must keep `filename`/`texaddr` u32).

### NJS_TEXLIST — LOCKED-FORMAT (CONFIRM; upgrades resolutions-1:61 MED→HIGH)
Struct `NinjaTypes.h:13-16` (`NJS_TEXNAME *textures; u32 nbTexture;`). Direct
overlay of loaded chunks: `BmlResource.cpp:427` `*slot = (NJS_TEXLIST
*)data;` (NJTL) and `:481`; the POF0 table is what turns the image's
4-byte `textures` slot into a live pointer post-copy. The game itself stores
texlist references as 32-bit handles — `TTextureWave.cpp:47`
(`texture_list` is an `saddr` field: `psoLoadTexture(*(NJS_TEXLIST **)&this->texture_list)`), `psoLoadTexture.cpp:97-101`
(`free_if_valid(saddr texlist)` → `(NJS_TEXLIST *)texlist`) — textbook rule-1
handles: the slot width is format-pinned. Runtime-built variant
(`psoLoadTexture.cpp:142`) shares the layout, same argument as TEXNAME.

### EnemySetRecord — LOCKED-FORMAT (CONFIRM AD resolutions-1:16 HIGH; §2.2-overreach path cleared)
Direct overlay on the floor set image, anchor verified fresh at
`src/pso/unk/TLobby_80207DA8.cpp:172` (`EnemySetRecord *row =
(EnemySetRecord *)row_ptr;`) plus `TLobby_80209524.cpp:1063`
(`init_quest_floor_enemies(floor, (EnemySetRecord *)source, size)`). Runtime
row buffers are sized *from the image*: `TSetManager.cpp:1504`
`current_floor_enemy_set_rows = (EnemySetRecord *)TQuestVmDownLoadServer::alloc(bytes)`
with 0x5A8-row tables memset by `sizeof` (:1354); `TSetManager.cpp:511,590`
cast the stored `set_entry` back over image offsets. Struct
`include/pso/map/FloorPopulation.h:26-30` has `s16 type; s16 flags;
EnemyPlacement placement; TObjEnemy *enemy;` — the `enemy` slot is assigned
post-load by `assign_display_objects` (`TSetManager.cpp:876`), so the slot is
a handle; J's conversion must cover it.

### FloorObjectSetRow — LOCKED-FORMAT (CONFIRM AD resolutions-1:17 HIGH)
Strongest chain in the set: `src/pso/unk/TLobby_80209524.cpp:674`
`read_disc_file(name.s, map_set_file_buffer)` (raw DVD read) → `:714`
`assign_object_ids((FloorObjectSetRow *)map_set_file_buffer, n, floor)` →
stride walk `byte_size/68U` over the same buffer (resolve-1 §5.2 anchors at
TLobby_801F3A24.cpp:284+). Quest-download variant passes image-typed rows:
`TSetManager.cpp:192 load_floor_enemy_group_entries(s32, FloorObjectSetRow
*src, u32 size)` from `TLobby_80209524.cpp:1072`. Struct
`FloorPopulation.h:19-23`, `object` slot = post-load handle (rule-1, J).

## Image-copy obligations for the assets lane (5)

The one RUNTIME-WAIVE removes nothing from this list; these bind whoever
copies/patches loaded images — **logo textures especially** (boot-spine.md:68:
`load_streamed_prs_texture("logo_e",…)` from TAdSEGAlogo; the GVM/PRS BE path,
net-disc D2–D4):

1. **BML/NJ\* chunk copies stay byte-true**: raw copy (TLobby_801E2F94.cpp:400)
   of NJCM/NJTL/NDMD chunks into a retained arena, POF0 table applied against
   the copy base; the 4-byte slots (`NJS_OBJECT.model/child/sibling`,
   `NJS_TEXLIST.textures`, `NJS_TEXNAME.filename/texaddr`, `NJS_MOTION.mdata`,
   `NJS_MDATA2.p[]`) are **never re-spelled as host pointers**.
2. **POF0 semantics preserved** (TLobby_801E2F94.cpp:413+): u8 delta-stream
   walk of pointer offsets — loaders must keep post-copy patching even when
   the host keeps images in 64-bit-addressed RAM (translate, don't widen).
3. **Floor-set rows keep the era stride**: `read_disc_file` →
   `map_set_file_buffer` → 68-byte `FloorObjectSetRow` walk (:674/:714) and
   the enemy-set mirror; any pre-stitch repack of `.set` blocks must copy at
   `sizeof(era-row)`, not `sizeof(host-row)`.
4. **GVM/TLS logo path** (`LoadPvmFileTls`, psoLoadTexture.cpp:110-175):
   header-driven `count*0x2C` record stride + 0x20 name slots must be read
   with era field widths before populating the runtime NJS_TEXNAME arena;
   PRS-decompressed texel chunks keep BE texel order / palette banks
   (`GinjaMaterialState.h` `tlut_name`, `uaddr texaddr` slot).
5. **Z-lineage display-list work must respect**: DRAW_TPL lists are opaque
   **BE32 GP command wires** executed via `GXCallDisplayList`
   (`GinjaMaterial.cpp:478,:495`; host gate
   `host/gx/dl/gx_dl_driver.c` PATH B, `GXDisplayList.c:74` wire format) —
   build/capture must keep byte-order and command-word encodings verbatim;
   colored-vertex payloads live **only** in that wire and in GX VTXFMT
   attributes — they must NOT be re-interpreted as `NJS_POINT3COL` structs
   (that type is host-runtime-only per the flip above), and material/texlist
   handles crossing the wire (`GinjaSceneState.textures`,
   GinjaMaterialState.h:57) stay u32 per obligation 1.

**Lane-J handle-conversion coverage required** (LOCKED ≠ no work): the
dereference sites of the seven pointer slots named in obligation 1 plus
`EnemySetRecord.enemy` and `FloorObjectSetRow.object` — u32 slots stay 4 bytes
in the pinned structs; base+handle translation happens at access, per
p1-datamodel §3 rule-1 (J's arena proof passed round-trip incl. composite
handles, datamodel §:127-137).

## Stitch-gate ledger

| type | prior state | AJ verdict | vs AD |
|---|---|---|---|
| NJS_OBJECT | tranche-1 UNRESOLVED | LOCKED-FORMAT | settles (direction prefigured §5.2) |
| NJS_MKEY_F | tranche-1 UNRESOLVED | LOCKED-FORMAT | settles |
| NJS_POINT3COL | resolutions-1:52 LOCKED MED | **RUNTIME-WAIVE** | **FLIP** (reasoned above) |
| NJS_TEXNAME | resolutions-1:60 LOCKED MED | LOCKED-FORMAT | CONFIRM (MED→HIGH) |
| NJS_TEXLIST | resolutions-1:61 LOCKED MED | LOCKED-FORMAT | CONFIRM (MED→HIGH) |
| EnemySetRecord | resolutions-1:16 LOCKED HIGH | LOCKED-FORMAT | CONFIRM |
| FloorObjectSetRow | resolutions-1:17 LOCKED HIGH | LOCKED-FORMAT | CONFIRM |

All 7 lane-j-handle rows now carry a positive verdict; no type remains
without a resolution, and the single waiver is proven image-free by an
exhaustive 13-site census.

STITCH-GATE: clear
