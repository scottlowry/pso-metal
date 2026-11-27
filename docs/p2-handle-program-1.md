# P2-HP — uaddr-handle program CONTRACT: guest windows, stamping seam, first runtime golden

Phase-2 fleet wave-3 lane HP, 2026-11-27 (card `t_a5fc8c9c`). **ZERO builds,
ZERO code landed**: decomp `pso-decomp @6b38b5383` READ-ONLY, no repo make,
scratch `clang -fsyntax-only` probes only (`cache/scratch/hp/`). Waiver pin
`host/bringup/waiver-header.sha256` at intake era `d4382fcd0659357f`
(apply-7); this lane ran no build, so the pin stands untested-and-untouched —
a later builder lane re-hashes pre/post its builds per D8 and NEVER bumps it.
Writes: this doc + `compat/njs/handle/**` (new dir, inert) — nothing else.
Inputs honored: apply4 §Meaning ("Parse != runtime correctness"), NA §4
(260 grep-proof RUNTIME-GATED rows), AR §4.1/§4.3/§5 (loader sites, slot-space
law, C-silent-truncation ban), AV §3/§4 S4 (stamp-before-GX rule), p1-datamodel
§3 (u32 handles + base register, BufferRef composite), p2-v-wave (g_sinbd
arena-resident placement: sinbd @+0x32000, pool @+0x34000/+0x38000, cache heap
0x40000 — "the h2g/g2h round-trip only closes inside the guest window"), NC
`docs/p2-njs-wave-1.md` §2 (LobbyPoolEntry = AramBlock * / ARAM window,
LobbyFloorRows = heap window slots — parse-shape only until a loader binds).

## 1. What this program is

The apply-7 stack makes 1,163 corpus units **parse** with honest layouts, and
the AR shadow makes NJS slots 4 bytes on LP64. That is typing, not behavior:
NA's 260 rows coerce host pointers into/out of u32 slot space at runtime, and
every crossing is only *correct* if the stored value and the resolver agree on
**which guest window's base** the slot is relative to. Single-base today
(`pso_njs_g2h` = `pso_arena_base + (slot − 0x80000000)`, AR shadow:43-53)
covers main-RAM payloads and mis-converts everything else — AR §5 named the
failure modes (ARAM content, statics outside the arena, stack descriptors).
This contract (a) enumerates the windows the 260 rows actually touch, (b) fixes
the seam API where loaders bind bases and stamp slots, (c) assigns every row a
window + stamping loader, and (d) names the FIRST runtime golden a builder
lane runs before any of the 260 rows may be adopted.

## 2. The distinct guest-window bases (enumeration)

The port arena is ONE 26 MB mmap: `[0,24MB)` = main RAM (guest
`0x80000000..0x81800000`), `[24MB,26MB)` = ARAM (crt0.c; `pso_arena_base`,
`pso_aram_base`). So the distinct **base registers** are exactly TWO —
`pso_arena_base` (main-RAM windows) and `pso_aram_base` (ARAM window) — and
what the seam needs on top is a **window registry** (guest range → host base)
so conversions can classify, range-check, and fail loud. Main-RAM windows all
share the base and differ by span/ownership:

| window class | guest space | host base register | era homes (what fills slots of this class) | stamping loader | gated rows |
|---|---|---|---|---|---|
| `PSO_W_POOL` | main RAM 0x80xxxxxx | `pso_arena_base` | runtime objects: THeap `obj_heap`/`alt_heap` (THeap.cpp, era operator-new path), TLobby node allocator, BmlResource runtime models/motions, entity tables | **L-POOL** (object/node allocator seam) | 127 |
| `PSO_W_IMAGE` | main RAM 0x80xxxxxx | `pso_arena_base` | image statics: POF0-patched NJS trees/tables (read_ninja_chunk `TLobby_801E2F94.cpp:334-430`, patcher :413+), archive copies, `.bss` globals; g_sinbd placement regime @+0x32000 | **L-IMAGE** (chunk/archive loaders + data placement) | 65 |
| `PSO_W_HEAP` | main RAM 0x80xxxxxx | `pso_arena_base` | `sys_heap` = `OSCreateHeap` over `cfg.heap+0x518000..hi` (SystemBootInit.cpp:275 → memory.cpp:27); xmalloc/mem_alloc, boot global buffers (`resource_decompress_buffer` :283, `player_matrix_stack_buffer` :288), `TQuestVmDownLoadServer::alloc` = xmalloc (TQuestListVM_8007EAF8.cpp:48), archive/quest-download copies | **L-SYSHEAP** (memory.cpp / allocator return seam) | 23 |
| `PSO_W_ARAM` | ARAM 2 MB, **ARAlloc-offset space** (small ints, NOT 0x80-tagged) | `pso_aram_base` | `floor_set_archive_pool = new AramPool(item_drop_memory_pool_ar_addr=ARAlloc(0x100000), …)` + `floor_saved_row_heap = new THeap(0xA000,0x20)` (TSetManager.cpp:1529/:1534/:1561 — NC LobbyPoolEntry/LobbyFloorRows regime), `quest_download_aram_base` (QuestDownloadAramFile.cpp:146), `ad_warning_aram_base = ARAlloc(0x80000)` (TAdWarningBaseGC.cpp:101), askdic | **L-ARAM** (AramPool/ARAlloc + aram_dma_setup) and **L-FLOOR** (TSetManager floor init — registers BOTH the ARAM pool span and the row-heap main-RAM span) | 5 |
| `PSO_W_CACHE` | main RAM 0x80xxxxxx | `pso_arena_base` | locked-cache heap (`OSInitAlloc` span, `locked_cache_heap_handle`): GX texel/surface buffers, DVD read destinations; AU-4 carve regime; **AV §1.4-S4: the +0x1C pixels slot MUST be `pso_njs_h2g`-stamped before GX sees it** | **L-CACHE** (alloc_buffer_from_heap / GX supply seam) | 2 |
| `PSO_W_RESERVE` | main RAM 0x80xxxxxx | `pso_arena_base` | boot reserve carve `lo..fifo+0x40000`: display/draw framebuffers + GX FIFO (SystemBootInit.cpp:264-271, `cfg.fifo_size=0x40000`) | **L-RESERVE** (init_game_systems carve) | 1 |

Stamped total = **223**. The remaining **37** rows carry values that are NOT
arena-window addresses — they get a class, an explicit NO-STAMP verdict, and a
per-site rewrite obligation (they are why "adopt the shadow's helpers at every
site" would be a runtime bug, not a fix):

| class | rows | why it is not a window | obligation |
|---|---|---|---|
| `W-DERIVED` | 21 | host-stack/local-derived addresses laundered into slots (`(saddr)&item`, `pts`, `position`, the `encoded_amount ^ (saddr)&encoded_amount` key) — stack is outside every guest window (AR §5 "stack descriptors mis-convert") | per-site: hoist the buffer to a stamped allocator object, pass by value, or keep the whole site compare-only; NEVER resolve base+slot |
| `W-INT` | 12 | immediates/scalars in pointer dress (`(saddr)((void*)0xD)` states, `4194304` flags, `RADIANS_TO_ANGLE*atan2` angles, `get_parts_entry((saddr)i)`) — resolving these would fabricate ARAM/main-RAM pointers (offset-space collision!) | slot-space compare law; converter must reject unregistered small ints loudly |
| `W-CODE` | 2 | function pointers in slots (`set_si_polling_callback((uaddr)…append_next_char_global)`, the `impact_callback` indirect-call cast) — era `.text` sits in guest range but port code is host-side | port callback table / trampoline registry; never base+slot |
| `W-HARNESS` | 2 | `__sinit_*` static-ctor ref artifacts (decomp harness, not guest memory) | strip at adoption; no runtime meaning |

Python check of the split: W-POOL 127 + W-IMAGE 65 + W-HEAP 23 + W-ARAM 5 + W-CACHE 2 + W-RESERVE 1 + W-DERIVED 21 + W-INT 12 + W-CODE 2 + W-HARNESS 2 = 223 stamped + 37 non-window = 260 (classifier + row extraction:
`cache/scratch/hp/{classify.py,rows.json}`, first-cited NA evidence per row,
260 rows = 260 files, matches NA §4 line-for-line).

Logo-trio corroboration: V-wave V2/V5b already RUNS the stamped regime the
contract generalizes — `g_sinbd` `.data` block placed arena-resident
(@+0x32000, pools @+0x34000/+0x38000, cache heap 0x40000) because "the
h2g/g2h round-trip only closes inside the guest window," and L6-G10/G8-G9
prove stamp-at-genesis through the era chain. This program is that regime,
named, per-window, with all 260 rows assigned.

## 3. The stamping seam API

Paste-ready sketch lands inert at `compat/njs/handle/pso_handle_windows.h`
(probe: `-fsyntax-only` clean under C11 and C++17, zero warnings; zero repo
references — `grep njs/handle` resolves to this doc + the sketch only).

### 3.1 Binding — the loader owns the base, once

`pso_handle_window_bind(win, guest_begin, guest_end, host_begin)` registers a
window span at the moment the loader materializes it. Each loader and its bind
site (builder lane implements; era addresses stay the contract):

| loader | binds when | windows |
|---|---|---|
| L-RESERVE | `init_game_systems` carve (`cfg.display_framebuffer … fifo_buffer + 0x40000`, SystemBootInit.cpp:251-271) | `PSO_W_RESERVE` |
| L-SYSHEAP | `init_system_heap_and_global_allocator` (memory.cpp:26-29; `OSCreateHeap(start, hi)`) | `PSO_W_HEAP` |
| L-CACHE | `OSInitAlloc(cfg.heap, …)` + every `alloc_buffer_from_heap` return that feeds GX headers (AV §1.4-S4) | `PSO_W_CACHE` |
| L-IMAGE | each `read_ninja_chunk`/archive copy completes at its arena destination (TLobby_801E2F94.cpp:334-430; g_sinbd placement) | `PSO_W_IMAGE` |
| L-POOL | first THeap `obj_heap`/`alt_heap` span + TLobby node-pool init | `PSO_W_POOL` |
| L-FLOOR | `TSetManager` floor init (:1527-1534, :1561): ARAM pool span AND row-heap span | `PSO_W_ARAM` + row-heap `PSO_W_HEAP`-class record |
| L-ARAM | `aram_dma_setup`/`ARAlloc` shim records each base (QuestDownloadAramFile.cpp:146, TAdWarningBaseGC.cpp:101) | `PSO_W_ARAM` |

Registry shape: multi-span per class is the builder's option (span counts are
boot-bounded); conflicting re-binds fail loud. crt0 (`pso_bringup_install`)
remains the ONLY owner of the two base registers — windows never mmap.

### 3.2 Crossing — `NJS_HANDLE(T, slot) = base + slot`, rules kept

* **NULL law (untouched):** `slot == 0 ↔ NULL`. `g2h(0) == NULL`,
  `h2g(NULL) == 0`. Era sentinel semantics (92 `(uaddr)x == 0` null tests, J
  §1b) stay honest — offset 0 is NOT a valid object address in any window, so
  no collision with the sentinel.
* **Stamp-at-genesis:** every slot WRITE happens once, at the site that
  materialized the pointed-to object, via the window-tagged converter
  (`pso_njs_handle_h2g_main` / `_h2g_aram`; the AV S4 pixels-slot is the
  named precedent). Read sites ONLY convert (`NJS_HANDLE`), never re-derive a
  guest value from a host pointer they did not stamp.
* **Runtime `NJS_HANDLE(T,slot)`:** same macro name the shadow ships —
  registry-backed `pso_njs_handle_g2h` replaces the single-base body when HG-1
  (§5) passes; site code never changes twice. Resolution order: `0`→NULL;
  `slot ≥ 0x80000000` → covering main-RAM record → `host_begin + (slot −
  guest_begin)`; ARAM-offset → ARAM records; **anything else: FAIL LOUD** —
  that reject-path is what keeps the 12 W-INT rows from becoming fabricated
  pointers.
* **Slot-space comparison law:** validity/range checks stay on the u32
  (`pso_handle_slot_in_window`, TChCrLoadReq AR §4.1 precedent); distances
  `(uaddr)a − (uaddr)b` stay in slot space (J §1c: values ≤24 MB, pointer
  width never needed).
* **4-byte slots, translate never widen** (AR §4.1 adopted policy): POF0
  patcher keeps writing 4-byte guest absolutes; read_ninja_chunk copy :400
  unchanged.
* **C-TU ban stands:** the seam is C++-typed; no `.c` TU ever gets
  `-Icompat/njs*` without unsuppressed `-Wint-conversion` (AR §5), and
  adoption stays per-TU allow-list, integrator-owned.

### 3.3 Out-of-window classes

`W-DERIVED`/`W-INT`/`W-CODE`/`W-HARNESS` rows receive NO stamping loader: the
§4 obligation column is their rewrite/compare adjudication queue. Admitting
them to adoption without per-site edits is exactly the "silent runtime risk"
NA §4 tail forbids.

## 4. Per-row obligations (row → window → stamping loader)

Generated from NA §4's table verbatim (`cache/scratch/hp/classify.py`,
python-validated 260/260, one row per file, first-cited evidence). Rows in
window order. Final adjudication per site stays with adoption (this table is
the named blast radius + assignment); truncated evidence (`…`) rows are marked
in their obligation note.

| row (source file : first-cited site) | window | stamping loader | obligation |
|---|---|---|---|
| src/pso/audio/SoundControlState.cpp (ptr->saddr-cast @641) | W-POOL | L-POOL | runtime actor address; pool allocator stamps |
| src/pso/boss/TBoss3VoloptMissile.cpp (saddr-uaddr-coerce @150) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/boss/darkfalz/TOBoss4.cpp (slotint->ptr @74) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/boss/galgryphon/Boss5GryphonBehavior.cpp (saddr-uaddr-coerce @814) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/boss/goldragon/TBoss8DragonEffBreath.cpp (saddr-uaddr-coerce @31) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/boss/olgaflow/TBoss6Rock.cpp (saddr-uaddr-coerce @160) | W-POOL | L-POOL | BmlResource runtime model/texlist (mem_alloc pool side) |
| src/pso/enemy/epsilon/TObjEneEpsilonBody.cpp (saddr-uaddr-coerce @389) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/enemy/epsilon/TObjEneEpsilonGuard.cpp (saddr-uaddr-coerce @521) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/enemy/flower/TObjEneRe2Flower_Bomb.cpp (int->ptr-rcast @57) | W-POOL | L-POOL | battle-param slot table (pool window) |
| src/pso/enemy/gibbles/TObjEneGibbles.cpp (saddr-uaddr-coerce @464) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/enemy/gyaranzo/TObjEneGyaranzo.cpp (saddr-uaddr-coerce @823) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/enemy/gyaranzo/TObjGyaranzoMissile.cpp (saddr-uaddr-coerce @59) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/enemy/gyaranzo/TObjGyaranzoMissile_800D40D8.cpp (saddr-uaddr-coerce @184) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/enemy/mericarol/TObjEneBm9Mericarol_Bomb.cpp (int->ptr-rcast @46) | W-POOL | L-POOL | battle-param slot table (pool window) |
| src/pso/enemy/moja/TObjEneMoja.cpp (saddr-uaddr-coerce @578) | W-POOL | L-POOL | address of member field of pooled object; base+slot closes |
| src/pso/enemy/morfos/TObjEneMorfosGroundShot.cpp (saddr-uaddr-coerce @80) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/enemy/nanodrago/TObjNanoDragoSingleBeam.cpp (saddr-uaddr-coerce @128) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/enemy/recon/TObjEneRecobox.cpp (saddr-uaddr-coerce @174) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/enemy/recon/TObjEneRecon.cpp (saddr-uaddr-coerce @700) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/enemy/recon/TObjEneReconBomb.cpp (saddr-uaddr-coerce @69) | W-POOL | L-POOL | BmlResource runtime model/texlist (mem_alloc pool side) |
| src/pso/enemy/slime/TObjEneSlimeOne.cpp (int->ptr-rcast @228) | W-POOL | L-POOL | BmlResource runtime model/texlist (mem_alloc pool side) |
| src/pso/item/TItemData.cpp (saddr-uaddr-coerce @780) | W-POOL | L-POOL | runtime player entity; pool allocator stamps |
| src/pso/item/TItemEquipBase.cpp (saddr-uaddr-coerce @299) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/item/TItemFollowStarLight.cpp (saddr-uaddr-coerce @21) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/item/TItemLockCountCtrl_80119238.cpp (saddr-uaddr-coerce @37) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/item/TItemTimerEvent.cpp (saddr-uaddr-coerce @63) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/item/TItemToolTechniqueDisk.cpp (saddr-uaddr-coerce @9) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/item/shop/TShopGenerator.cpp (saddr-uaddr-coerce @142) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/map/TSetEvtDestroy.cpp (saddr-uaddr-coerce @39) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/npc/TNpcPlayer_800D87A8.cpp (int->ptr-rcast @8) | W-POOL | L-POOL | battle-param table entry (truncated evidence); pool stamps |
| src/pso/object/TObjControl.cpp (ptr->saddr-cast @234) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/object/TObjHMS_EZRope_Player.cpp (saddr-uaddr-coerce @48) | W-POOL | L-POOL | address of member field of pooled object; base+slot closes |
| src/pso/object/TObjLuker.cpp (saddr-uaddr-coerce @63) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/object/base/TAttackCollider.cpp (int->ptr-rcast @78) | W-POOL | L-POOL | battle-param slot table (pool window) |
| src/pso/object/base/TObjCollidable.cpp (ptr->saddr-cast @875) | W-POOL | L-POOL | battle-param slot table (pool window) |
| src/pso/object/collision/TObjFogCollision.cpp (saddr-uaddr-coerce @332) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/object/container/TContainerAncient01.cpp (saddr-uaddr-coerce @294) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/object/ending/TObjOnlineEndingMain.cpp (ptr->saddr-cast @318) | W-POOL | L-POOL | member field of pooled account entry; base+slot closes |
| src/pso/object/switch/TOSwitchAttack.cpp (saddr-uaddr-coerce @112) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/object/trap/TOMine.cpp (saddr-uaddr-coerce @271) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/object/warp/TObjAreaWarp_80307B70.cpp (saddr-uaddr-coerce @13) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/object/warp/TObjCityMainWarp.cpp (saddr-uaddr-coerce @153) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/object/warp/TObjCityMainWarpChallenge.cpp (saddr-uaddr-coerce @79) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/object/warp/TObjLaboMapWarp.cpp (int->ptr-rcast @617) | W-POOL | L-POOL | battle-param slot table (pool window) |
| src/pso/player/TObjEndingPlayer.cpp (saddr-uaddr-coerce @63) | W-POOL | L-POOL | battle-param slot table (pool window) |
| src/pso/player/TObjPlayerCreate.cpp (saddr-uaddr-coerce @515) | W-POOL | L-POOL | address of member field of pooled object; base+slot closes |
| src/pso/player/TObjPlayer_801AF2F0.cpp (int->ptr-rcast @435) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/player/TPlayerBattleCtrl4P.cpp (saddr-uaddr-coerce @126) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/player/TPlyItem.cpp (saddr-uaddr-coerce @719) | W-POOL | L-POOL | transparent record pointer (memset return); window follows record |
| src/pso/quest/TQuestThread.cpp (saddr-uaddr-coerce @488) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/quest/TQuestVmDownLoadServer.cpp (saddr-uaddr-coerce @593) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/quest/challenge/TChallenge4P.cpp (saddr-uaddr-coerce @411) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/system/GbaDownloadRuntime.cpp (slotint->ptr @37) | W-POOL | L-POOL | int-declared carrier of runtime TObject; pool stamps |
| src/pso/system/save/TSaveChCr.cpp (saddr-uaddr-coerce @155) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/technique/TTecAnti.cpp (saddr-uaddr-coerce @107) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/technique/TTecBarta.cpp (int->ptr-rcast @117) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/technique/TTecDeband.cpp (saddr-uaddr-coerce @80) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/technique/TTecGiZonde.cpp (saddr-uaddr-coerce @114) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/technique/TTecPlayerInfluence.cpp (saddr-uaddr-coerce @116) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/technique/TTecRaBarta.cpp (saddr-uaddr-coerce @191) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/technique/TTecRaFoie.cpp (saddr-uaddr-coerce @120) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/technique/TTecReverser.cpp (saddr-uaddr-coerce @66) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/technique/TTecZonde.cpp (saddr-uaddr-coerce @177) | W-POOL | L-POOL | runtime target entity; pool allocator stamps |
| src/pso/ui/chat/TWindowChat.cpp (saddr-uaddr-coerce @94) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/chat/TWindowChatSymbol.cpp (saddr-uaddr-coerce @257) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/chat/TWindowSubSymbolChat.cpp (saddr-uaddr-coerce @182) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/chat/TWindowSubSymbolChatHelp.cpp (saddr-uaddr-coerce @214) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/chat/TWindowSubSymbolChat_802ACF5C.cpp (saddr-uaddr-coerce @9) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/ui/city/TCheckRoomSequence.cpp (saddr-uaddr-coerce @550) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/city/TShopSequence.cpp (saddr-uaddr-coerce @49) | W-POOL | L-POOL | freshly created window object; pool stamps on create |
| src/pso/ui/frontend/TAdChrSelPlyPlateGC.cpp (saddr-uaddr-coerce @117) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/ui/frontend/TAdDialingGC.cpp (saddr-uaddr-coerce @424) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/frontend/TAdEnterPPPPassGC.cpp (saddr-uaddr-coerce @130) | W-POOL | L-POOL | parent this-pointer argument; pool stamps |
| src/pso/ui/frontend/TAdEnterSerialAccessGC.cpp (saddr-uaddr-coerce @136) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/frontend/TAdMultiPlyChrSelectGC.cpp (int->ptr-rcast @177) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/frontend/TAdMultiSelGameModeGC.cpp (saddr-uaddr-coerce @119) | W-POOL | L-POOL | runtime result object; pool stamps |
| src/pso/ui/frontend/TAdSinglePlyChrSelectGC_801FBF54.cpp (ptr->saddr-cast @1890) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/ui/frontend/TAdSystemFileCheckGC.cpp (saddr-uaddr-coerce @368) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/frontend/TAdTimeSetGC.cpp (saddr-uaddr-coerce @832) | W-POOL | L-POOL | created menu window object; pool stamps on create |
| src/pso/ui/frontend/TAdYesNoBaseGC.cpp (saddr-uaddr-coerce @25) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/frontend/TTitleBackground.cpp (saddr-uaddr-coerce @131) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/window/ItemListRow.cpp (saddr-uaddr-coerce @512) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/ui/window/TObjResultWin.cpp (saddr-uaddr-coerce @139) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/window/TWindowActionPalette.cpp (saddr-uaddr-coerce @158) | W-POOL | L-POOL | runtime help window; pool stamps |
| src/pso/ui/window/TWindowChoiceSetting_80249078.cpp (ptr->saddr-cast @32) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/window/TWindowCtrl.cpp (saddr-uaddr-coerce @439) | W-POOL | L-POOL | address of member field of pooled object; base+slot closes |
| src/pso/ui/window/TWindowCtrlMain_800FC91C.cpp (saddr-uaddr-coerce @25) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/ui/window/TWindowEffectParty.cpp (saddr-uaddr-coerce @42) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/window/TWindowFukidasi.cpp (saddr-uaddr-coerce @91) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/window/TWindowGetComCard.cpp (saddr-uaddr-coerce @92) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/ui/window/TWindowHelpItem.cpp (saddr-uaddr-coerce @176) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/window/TWindowKeyBoardHard.cpp (ptr->saddr-cast @194) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/window/TWindowMenuIconLineLists.cpp (int->ptr-rcast @143) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/ui/window/TWindowMessageYesNo.cpp (int->ptr-rcast @108) | W-POOL | L-POOL | created menu window; pool stamps on create |
| src/pso/ui/window/TWindowOptionCtrl.cpp (saddr-uaddr-coerce @71) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/ui/window/TWindowShortListCtrl.cpp (saddr-uaddr-coerce @272) | W-POOL | L-POOL | runtime tech window object; pool stamps |
| src/pso/ui/window/TWindowSubComCard.cpp (saddr-uaddr-coerce @456) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/window/TWindowSubComCard_8007341C.cpp (saddr-uaddr-coerce @165) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/window/TWindowSubComCard_8007A810.cpp (ptr->saddr-cast @31) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/ui/window/TWindowSubComCard_8007BD24.cpp (int->ptr-rcast @12) | W-POOL | L-POOL | runtime card entry (truncated get_card_entry call); pool stamps |
| src/pso/ui/window/TWindowSubCustomEquip.cpp (saddr-uaddr-coerce @273) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/ui/window/TWindowSubEquip.cpp (saddr-uaddr-coerce @174) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/ui/window/TWindowSubItem.cpp (saddr-uaddr-coerce @505) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/ui/window/TWindowSubMag.cpp (saddr-uaddr-coerce @191) | W-POOL | L-POOL | runtime help window; pool stamps |
| src/pso/ui/window/TWindowSubQuestBoard.cpp (saddr-uaddr-coerce @184) | W-POOL | L-POOL | created titled menu window; pool stamps on create |
| src/pso/ui/window/TWindowSubTradeMain.cpp (saddr-uaddr-coerce @150) | W-POOL | L-POOL | address of member field of pooled object; base+slot closes |
| src/pso/ui/window/TWindowTimer.cpp (saddr-uaddr-coerce @72) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/unk/text_80076BC4.cpp (saddr-uaddr-coerce @62) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/unk/text_8018E258.cpp (saddr-uaddr-coerce @16) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/unk/text_80278460.cpp (saddr-uaddr-coerce @24) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/weapon/TItemAttackCol.cpp (saddr-uaddr-coerce @272) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/weapon/TItemFollowEffect.cpp (saddr-uaddr-coerce @20) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/weapon/TItemWeapon.cpp (saddr-uaddr-coerce @127) | W-POOL | L-POOL | this-flag carrier (truncated (uaddr) cast); pool stamps |
| src/pso/weapon/TItemWeaponBaranzLauncher.cpp (saddr-uaddr-coerce @17) | W-POOL | L-POOL | runtime owner player; pool stamps |
| src/pso/weapon/TItemWeaponBranchOfPakupaku.cpp (saddr-uaddr-coerce @10) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/weapon/TItemWeaponBroom.cpp (saddr-uaddr-coerce @10) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/weapon/TItemWeaponFlowerBouquet.cpp (saddr-uaddr-coerce @11) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/weapon/TItemWeaponInfernoBazooka.cpp (saddr-uaddr-coerce @67) | W-POOL | L-POOL | address of member field of pooled object; base+slot closes |
| src/pso/weapon/TItemWeaponNeisClaw.cpp (saddr-uaddr-coerce @131) | W-POOL | L-POOL | runtime entity from object pool; pool allocator stamps |
| src/pso/weapon/TItemWeaponRocketPunch.cpp (saddr-uaddr-coerce @51) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/weapon/TItemWeaponSRank.cpp (saddr-uaddr-coerce @144) | W-POOL | L-POOL | runtime target entity; pool stamps (stack `body` arg is second-site, review at adoption) |
| src/pso/weapon/TItemWeaponSRankBazooka.cpp (saddr-uaddr-coerce @38) | W-POOL | L-POOL | address of member field of pooled object; base+slot closes |
| src/pso/weapon/TItemWeaponWand.cpp (saddr-uaddr-coerce @10) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/weapon/TItemWeaponYasminkov2000H.cpp (saddr-uaddr-coerce @65) | W-POOL | L-POOL | address of member field of pooled object; base+slot closes |
| src/pso/weapon/TItemWeaponYasminkov3000R.cpp (saddr-uaddr-coerce @31) | W-POOL | L-POOL | address of member field of pooled object; base+slot closes |
| src/pso/weapon/TItemWeaponYasminkov7000V.cpp (saddr-uaddr-coerce @51) | W-POOL | L-POOL | runtime object address (TObject pool); pool allocator stamps |
| src/pso/weapon/TItemWeaponYasminkov9000M.cpp (saddr-uaddr-coerce @75) | W-POOL | L-POOL | address of member field of pooled object; base+slot closes |
| src/pso/audio/MusicControl.cpp (int->ptr-rcast @564) | W-IMAGE | L-IMAGE | era static label address in slot; POF0/image-space payload |
| src/pso/boss/darkfalz/TObjDFDemo2.cpp (int->ptr-rcast @128) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/boss/darkfalz/TObjDFDemo4.cpp (int->ptr-rcast @184) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/boss/darkfalz/TObjDFDemoF.cpp (int->ptr-rcast @97) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/boss/volopt/TBoss3Volopt_800D8ACC.cpp (saddr-uaddr-coerce @145) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/boss/volopt/TObjVoloptPillar.cpp (int->ptr-rcast @70) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/boss/volopt/TObjVoloptShutter.cpp (int->ptr-rcast @62) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/effect/particle/TParticle.cpp (int->ptr-rcast @253) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/effect/particle/TPlyFollowParticleCtrl.cpp (saddr-uaddr-coerce @112) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/enemy/dubchik/TObjEneDubchik.cpp (saddr-uaddr-coerce @351) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/enemy/nanodrago/TObjEneNanoDrago.cpp (saddr-uaddr-coerce @564) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/item/TItemData_8010EDA8.cpp (saddr-uaddr-coerce @68) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/item/TItemTexLoad.cpp (saddr-uaddr-coerce @119) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/item/item_create.cpp (saddr-uaddr-coerce @845) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/item/shop/ShopWeaponStock.cpp (ptr->saddr-cast @551) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/map/area/AreaCity.cpp (saddr-uaddr-coerce @72) | W-IMAGE | L-IMAGE | shared texlist static (tl_07); image placement stamps |
| src/pso/npc/TObjNpcAHM01.cpp (ptr->saddr-cast @39) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/npc/TObjNpcARM00.cpp (ptr->saddr-cast @39) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/npc/TObjNpcARW00.cpp (ptr->saddr-cast @35) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/npc/TObjNpcHFW00.cpp (ptr->saddr-cast @35) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/npc/TObjNpcHFW01.cpp (ptr->saddr-cast @35) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/npc/TObjNpcHHM00.cpp (ptr->saddr-cast @31) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/npc/TObjNpcHRM00.cpp (ptr->saddr-cast @31) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/npc/TObjNpcHRM01.cpp (ptr->saddr-cast @31) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/npc/TObjNpcNFM00.cpp (ptr->saddr-cast @31) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/npc/TObjNpcNFM01.cpp (ptr->saddr-cast @31) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/npc/TObjNpcNFW00.cpp (ptr->saddr-cast @31) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/npc/TObjNpcNFW01.cpp (ptr->saddr-cast @31) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/npc/TObjNpcNHW00.cpp (ptr->saddr-cast @31) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/npc/TObjNpcNHW01.cpp (ptr->saddr-cast @31) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/object/base/TObjChara.cpp (saddr-uaddr-coerce @1426) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/object/collision/TOSymbolchatColli.cpp (saddr-uaddr-coerce @96) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/object/fence/TOBarrierEnergy01.cpp (saddr-uaddr-coerce @90) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/object/fence/TOFenceAncient01.cpp (saddr-uaddr-coerce @48) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/object/fence/TOFenceAncient02.cpp (saddr-uaddr-coerce @48) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/object/fence/TOFenceAncient03.cpp (saddr-uaddr-coerce @59) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/object/fence/TOFenceAncient04.cpp (saddr-uaddr-coerce @57) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/object/key/TOKeyGenericSw.cpp (saddr-uaddr-coerce @109) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/object/switch/TOChatSensor.cpp (saddr-uaddr-coerce @89) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/object/warp/TObjMapWarpAncient.cpp (saddr-uaddr-coerce @70) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/player/TObjPlayer_801D8ADC.cpp (saddr-uaddr-coerce @294) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/quest/TQuestListOffline.cpp (ptr->saddr-cast @74) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/quest/TQuestListOnline.cpp (ptr->saddr-cast @72) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/chat/TSymbolChatCapture.cpp (slotint->ptr @36) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/chat/TWindowSubShortMail.cpp (saddr-uaddr-coerce @479) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/city/THunterGuildSeqOnlineTask_802156A0.cpp (saddr-uaddr-coerce @204) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/frontend/TAdChrSelPlayerGC.cpp (ptr->saddr-cast @146) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/frontend/TAdEnterBaseGC.cpp (saddr-uaddr-coerce @319) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/frontend/TAdGameStartBackGC.cpp (int->ptr-rcast @18) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/frontend/TAdGameStartShadowGC.cpp (int->ptr-rcast @17) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/frontend/TAdGameStartShineGC.cpp (int->ptr-rcast @21) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/frontend/TAdGameStartSubGC.cpp (int->ptr-rcast @24) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/frontend/TAdMultiPlyChrSelectGC_802C6F88.cpp (saddr-uaddr-coerce @23) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/frontend/TAdSelectTxtSubGC.cpp (int->ptr-rcast @20) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/hud/TRaderMapCore.cpp (slotint->ptr @255) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/window/TWindowDD_803430BC.cpp (saddr-uaddr-coerce @260) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/window/TWindowKeyBoardHard_801A529C.cpp (slotint->ptr @68) | W-IMAGE | L-IMAGE | static .bss network buffer; image placement stamps |
| src/pso/ui/window/TWindowSnapShot.cpp (saddr-uaddr-coerce @70) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/ui/window/TWindowSubComCardList.cpp (saddr-uaddr-coerce @77) | W-IMAGE | L-IMAGE | message string from text table; image placement stamps |
| src/pso/ui/window/TWindowSubEquipKind_80278300.cpp (saddr-uaddr-coerce @53) | W-IMAGE | L-IMAGE | message string from text table; image placement stamps |
| src/pso/ui/window/WindowLayoutEntries.cpp (ptr->saddr-cast @68) | W-IMAGE | L-IMAGE | layout entries table static; image placement stamps |
| src/pso/unk/TLobby_8001D6A0.cpp (saddr-uaddr-coerce @90) | W-IMAGE | L-IMAGE | bank-buffer slot + offsetof arithmetic in slot space; base+slot closes once image is arena-placed |
| src/pso/unk/TLobby_800F8AA8.cpp (saddr-uaddr-coerce @838) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/weapon/TItemWeaponPlantainHugeFan.cpp (saddr-uaddr-coerce @13) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/weapon/TItemWeaponSRankPunch.cpp (saddr-uaddr-coerce @48) | W-IMAGE | L-IMAGE | image-static table/texture address in slot; POF0/g_sinbd placement stamps |
| src/pso/boss/galgryphon/TBoss5GryphonLightningWing.cpp (saddr-uaddr-coerce @109) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/boss/goldragon/TBoss8DragonLeaderPart.cpp (saddr-uaddr-coerce @225) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/boss/goldragon/TObjBoss8DemoRoom.cpp (saddr-uaddr-coerce @61) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/enemy/illgill/TObjEneIllGill.cpp (saddr-uaddr-coerce @1127) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/npc/TObjNpcBase.cpp (saddr-uaddr-coerce @493) | W-HEAP | L-SYSHEAP | archive copy on heap; stamp at loader copy |
| src/pso/object/collision/TOAttackableCol.cpp (saddr-uaddr-coerce @271) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/object/door/TODoorJungleMain.cpp (saddr-uaddr-coerce @233) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/object/switch/TOSwitchTimer.cpp (saddr-uaddr-coerce @156) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/player/TPlayerFootStamp.cpp (saddr-uaddr-coerce @70) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/quest/challenge/TChallenge4PItemSelect.cpp (slotint->ptr @254) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/ui/TCharSaveSequence.cpp (saddr-uaddr-coerce @233) | W-HEAP | L-SYSHEAP | runtime OSAlloc/mem_alloc buffer; stamp at allocator return |
| src/pso/ui/charcreate/TCMCharacter.cpp (saddr-uaddr-coerce @253) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/ui/font/TFontSmallTask.cpp (saddr-uaddr-coerce @171) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/ui/frontend/TAdEnterPasswordGC.cpp (saddr-uaddr-coerce @68) | W-HEAP | L-SYSHEAP | realloc_buffer result on sys_heap; stamp at allocator return |
| src/pso/ui/frontend/TAdPlyPltAnimCtrlGC.cpp (saddr-uaddr-coerce @27) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/ui/frontend/TTextureWave.cpp (saddr-uaddr-coerce @41) | W-HEAP | L-SYSHEAP | runtime OSAlloc/mem_alloc buffer; stamp at allocator return |
| src/pso/ui/hud/TMainCursor.cpp (saddr-uaddr-coerce @227) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/ui/window/TCustomizeMenuCtrl.cpp (saddr-uaddr-coerce @45) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/ui/window/TWindowMainMenu.cpp (saddr-uaddr-coerce @238) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/ui/window/TWindowQuantity.cpp (saddr-uaddr-coerce @75) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/unk/text_80103674.cpp (saddr-uaddr-coerce @12) | W-HEAP | L-SYSHEAP | runtime OSAlloc/mem_alloc buffer; stamp at allocator return |
| src/pso/unk/text_801A2710.cpp (saddr-uaddr-coerce @37) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/unk/text_801A2B38.cpp (saddr-uaddr-coerce @39) | W-HEAP | L-SYSHEAP | era operator new -> sys_heap window; stamp at allocator return |
| src/pso/item/TObjItemLight.cpp (slotint->ptr @51) | W-ARAM | L-FLOOR | floor-set row data: ARAM pool resident (NC LobbyPoolEntry note) |
| src/pso/object/collision/TObjInfoCol.cpp (slotint->ptr @48) | W-ARAM | L-FLOOR | floor-set row data: ARAM pool resident (NC LobbyPoolEntry note) |
| src/pso/object/collision/TObjTradeCollision.cpp (slotint->ptr @39) | W-ARAM | L-FLOOR | floor-set row data: ARAM pool resident (NC LobbyPoolEntry note) |
| src/pso/quest/QuestDownloadAramFile.cpp (ptr->saddr-cast @109) | W-ARAM | L-ARAM | aux-RAM 2MB window; ARAlloc-offset space -> pso_aram_base |
| src/pso/unk/TLobby_801F3A24.cpp (saddr-uaddr-coerce @217) | W-ARAM | L-FLOOR | set-row bytes `(record + 1)`: floor-set archive copy in ARAM pool (NC LobbyPoolEntry regime) |
| src/pso/system/BmlResource.cpp (ptr->saddr-cast @175) | W-CACHE | L-CACHE | DVD read destination / file-handle params (locked-cache regime) |
| src/pso/system/TGStream.cpp (saddr-uaddr-coerce @186) | W-CACHE | L-CACHE | DVD read destination / file-handle params (locked-cache regime) |
| src/pso/system/movie/TMovie.cpp (saddr-uaddr-coerce @300) | W-RESERVE | L-RESERVE | boot-carved display/draw framebuffers (arena lo carve) |
| src/pso/audio/SoundRequests.cpp (saddr-uaddr-coerce @290) | W-DERIVED | none | stack NJS_ARPOINT position laundered; hoist to stamped buffer or pass by value |
| src/pso/boss/darkfalz/TBoss4DarkFalz.cpp (int->ptr-rcast @76) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/boss/olgaflow/TBoss6Mag.cpp (saddr-uaddr-coerce @665) | W-DERIVED | none | stack point array (pts) laundered; hoist to stamped buffer |
| src/pso/effect/item/TItemEffLandHit.cpp (saddr-uaddr-coerce @47) | W-DERIVED | none | stack ray endpoints (&start,&end) into call_impact_callback (callee name is NOT the coerced value); hoist to stamped buffer |
| src/pso/enemy/slime/TObjEneSlimeTailAttack.cpp (saddr-uaddr-coerce @405) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/item/TItemIdCounters.cpp (saddr-uaddr-coerce @94) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/item/TItemObject.cpp (saddr-uaddr-coerce @59) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/item/item_pmt.cpp (saddr-uaddr-coerce @48) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/network/GuildCardRecord.cpp (int->ptr-rcast @253) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/object/cave/TORockCave.cpp (int->ptr-rcast @143) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/player/TObjPlayer.cpp (saddr-uaddr-coerce @542) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/system/scene/OnlineLobbyScene.cpp (int->ptr-rcast @266) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/ui/charcreate/TObjCMName.cpp (int->ptr-rcast @1833) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/ui/city/TShopItemListWindow.cpp (int->ptr-rcast @193) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/ui/frontend/TAdChrMoveEffectGC.cpp (int->ptr-rcast @54) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/ui/frontend/TAdFadeGC.cpp (int->ptr-rcast @84) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/ui/window/TWindowEffectLight.cpp (int->ptr-rcast @42) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/ui/window/TWindowKeyBoardBase.cpp (saddr-uaddr-coerce @299) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/ui/window/TWindowLockOn.cpp (int->ptr-rcast @163) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/ui/window/TWindowSubLog.cpp (saddr-uaddr-coerce @31) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/ui/window/TWindowSubStatusDisp_8036D774.cpp (int->ptr-rcast @49) | W-DERIVED | none | stack/local-derived address; per-site hoist to stamped allocator |
| src/pso/enemy/deldepth/TObjEneDelDepth.cpp (saddr-uaddr-coerce @520) | W-INT | none | immediate flag in slot dress; slot-space compare only, never convert |
| src/pso/enemy/panarms/TObjEnePanarms.cpp (saddr-uaddr-coerce @1028) | W-INT | none | immediate flag in slot dress; slot-space compare only, never convert |
| src/pso/enemy/sorcerer/TObjEneRe4Sorcerer.cpp (saddr-uaddr-coerce @615) | W-INT | none | scalar smuggled in pointer dress; slot-space compare only |
| src/pso/player/PlayerTaskEffects.cpp (saddr-uaddr-coerce @749) | W-INT | none | immediate flag in slot dress; slot-space compare only, never convert |
| src/pso/player/TObjPlayerActions.cpp (saddr-uaddr-coerce @827) | W-INT | none | immediate flag in slot dress; slot-space compare only, never convert |
| src/pso/player/TObjPlayer_802C422C.cpp (saddr-uaddr-coerce @152) | W-INT | none | immediate flag in slot dress; slot-space compare only, never convert |
| src/pso/ui/chat/TWindowSubSymbolChatPartsSet.cpp (saddr-uaddr-coerce @59) | W-INT | none | scalar smuggled in pointer dress; slot-space compare only |
| src/pso/ui/city/THunterGuildWarning.cpp (slotint->ptr @141) | W-INT | none | scalar smuggled in pointer dress; slot-space compare only |
| src/pso/ui/frontend/TAdTrialAgreementGC.cpp (saddr-uaddr-coerce @39) | W-INT | none | immediate flag in slot dress; slot-space compare only, never convert |
| src/pso/ui/window/TWindowSubStatusDisp_800FBD74.cpp (saddr-uaddr-coerce @64) | W-INT | none | scalar smuggled in pointer dress; slot-space compare only |
| src/pso/unk/TLobby_80207644.cpp (saddr-uaddr-coerce @408) | W-INT | none | scalar smuggled in pointer dress; slot-space compare only |
| src/pso/unk/TLobby_80207DA8.cpp (saddr-uaddr-coerce @91) | W-INT | none | immediate flag in slot dress; slot-space compare only, never convert |
| src/pso/ui/keyboard/KeyboardKeyState.cpp (saddr-uaddr-coerce @304) | W-CODE | none | function pointer in slot; port callback table, never base+slot |
| src/pso/weapon/WeaponAttackActions.cpp (saddr-uaddr-coerce @186) | W-CODE | none | function pointer in slot; port callback table, never base+slot |
| src/pso/enemy/gibon/TObjEneBm5GibonU.cpp (slotint->ptr @2468) | W-HARNESS | none | decomp harness static-ctor artifact; strip at adoption |
| src/pso/enemy/wolf/TObjEneBm5Wolf.cpp (slotint->ptr @135) | W-HARNESS | none | decomp harness static-ctor artifact; strip at adoption |

## 5. FIRST runtime golden — HG-1 (bind real base, round-trip slot, assert byte-address)

Buildable by a later builder lane at `host/bringup` (ladder7-test regime,
insert-only `run.sh` line; THIS lane ran nothing). Closes the loop apply4 §
Meaning demands: proves the seam before ANY of the 260 rows is adopted.

* **HG-1.0 Bind:** `pso_bringup_install()` → assert `pso_aram_base ==
  pso_arena_base + 0x1800000`. Bind via the API: `L-IMAGE`
  `[arena+0x32000, arena+0x34000)` ↔ guest `[0x80032000, 0x80034000)` (the
  V5b g_sinbd placement), `L-POOL` `[arena+0x34000, arena+0x38000)` ↔ guest
  `[0x80034000, 0x80038000)`, `L-ARAM` `[0x0, 0x200000)` ↔ `pso_aram_base`
  (ARAlloc-offset space).
* **HG-1.1 Byte-address assert (the round-trip):** fabricate a pooled record
  `R` at `arena+0x34040`; `s = pso_njs_handle_h2g_main((u8*)&R)` →
  **assert `s == 0x80034040` exactly** (not just `g2h(h2g(p))==p` — the
  *value* must be the era-true guest byte address); `NJS_HANDLE(T, s) == &R`
  (pointer identity); `h2g(g2h(s)) == s`; field-of-record:
  `g2h(s + offsetof(T, f)) == (u8*)&R + offsetof(T, f)`.
* **HG-1.2 NULL rule:** `g2h(0) == NULL`, `h2g(NULL) == 0`.
* **HG-1.3 ARAM leg:** stamp quest-download-style: host
  `p = pso_aram_base + 0x100000`, `pso_njs_handle_h2g_aram(p) == 0x100000`,
  `g2h` of that slot (via ARAM dispatch) `== p` byte-exact; unbind via
  L-FLOOR-shaped `window_unbind` → resolver must fail loud.
* **HG-1.4 Reject leg (the W-INT guard):** `g2h(0xD)` must fail loud in the
  debug window dispatch — proving flags-in-slot-dress can never resolve to a
  fabricated pointer. Golden is only green when BOTH sides (round-trip +
  reject) hold; deterministic across two runs.
* **Gate:** exit 0, prior ladder lines verbatim green, waiver sha re-hashed
  pre/post equal (no regen — integrator owns `make parse`).

## 6. Sequencing + boundaries

1. Builder lane lands the registry + HG-1 (host/bringup owns; compat/njs/handle
   is the pasted shape).
2. Same or next lane swaps the AR shadow's helper body to `pso_njs_handle_g2h`
   (macro name stable); re-runs run.sh suites (bringup GREEN — 200/200 today per docs/STATUS.md §1, + run_window +
   run_dl + run_vi regimes stay green — V-wave acceptance baselines).
3. Loaders land binds per §3.1 table; then RUNTIME-GATED rows become
   adoptable per window as their loader stamps + site adapter lands
   (Suggested order: POOL+IMAGE first — 192 rows, one base, era-proven regime;
   HEAP/CACHE next (25); ARAM/FLOOR last (5, needs L-FLOOR/L-ARAM binds)).
   W-DERIVED/INT/CODE/HARNESS (37) never adopt by stamping — per-site edits.
4. Allow-list + corpus moves stay integrator-owned (`build/njs_allowlist.txt`,
   `PARSE_RECIPE`); this doc supplies no build edits; lane wrote only
   `docs/p2-handle-program-1.md` + `compat/njs/handle/**`.

## Repro

`cache/scratch/hp/{classify.py,rows.json,assemble_doc.py,hp_probe.sh}` (python
stdlib + clang -fsyntax-only, `-P6` not needed at this scale; zero repo
builds). Row counts are python set-math over the generated table, re-verified
at write time: 260 rows, 260 unique files, window split W-POOL 127 + W-IMAGE 65 + W-HEAP 23 + W-ARAM 5 + W-CACHE 2 + W-RESERVE 1 + W-DERIVED 21 + W-INT 12 + W-CODE 2 + W-HARNESS 2 = 223 stamped + 37 non-window = 260.
