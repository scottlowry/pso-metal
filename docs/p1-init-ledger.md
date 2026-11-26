# P1-P — Init-Action Ledger: `init_game_systems` → first logo frame → title

Integration point for the boot spine. Every discrete action in
`init_game_systems()` (`src/pso/ui/keyboard/SystemBootInit.cpp:249-338`) and the
main-phase dispatch (`:340-395`) down to the title screen gets one row:
**verdict**, **dependency**, **blocking-for-logo?**, and **intersection** with the
three Phase-1 mechanism docs.

Verified against `pso-decomp @ 6b38b53837` by reading the source directly (not the
boot-spine summary). Line citations below are from that tree, all relative to
`/Users/scott/Hermes/pso-decomp/pso-decomp`.

## 0. Verdict vocabulary and intersection codes

| Verdict | Meaning |
|---|---|
| **KEEP-AS-IS** | Pure decompiled logic; compiles/runs once ABI waivers (§abi.md/assert-policy) + data model (§p1-datamodel) are in place, swap handled where marked. |
| **STUB** | SDK call the port may no-op for boot-to-logo; replaced with a real shim in a later slice. |
| **HOST-IMPLEMENT** | Needs real native backing; subsystem named from `docs/host-surface.md` §2 rows. |
| **DEFER** | Not needed for a minimum-viable logo frame; keep the symbol as a link stub, do the work later. |

Intersections (per row):
- **A** = size-assert waiver path: a `LAYOUT_ASSERT` this action's types carry is in the 103-name waiver list or must be re-derived (MWCC vptr@-end Cause A / 12-byte PMF Cause B, `abi.md` §0/§2; mechanism `assert-policy.md` §4, `-DPSO_PORT_LAYOUT_WAIVER`).
- **S** = byteswap: BE data crosses at this action or its load path; swap goes at the `compat/pso_byteswap.h` chokepoints defined in `p1-format-net-disc.md` §5 (disc = swap-on-load at `open_gsl_archive`/GVM readers), **never** scattered in render code.
- **D** = data-model: `uaddr`/`saddr` or laundered-pointer sites; per `p1-datamodel.md` §3, `uaddr`→`uintptr_t` for runtime-only arithmetic, format-struct u32 handles stay 32-bit.

**Blocking?** = strictly *is this action required to put the first logo pixels on
screen?* "yes*" = required unless we make the labeled assumption in its note.

## 1. `init_game_systems` ledger (64 action rows over 73 call statements, source order)

boot-spine.md counts 48 actions by coarsely grouping the arena/stage blocks; the
source has 73 call statements (SystemBootInit.cpp:256-337). This ledger resolves to
64 rows: consecutive xmalloc carve-outs and cfg field-fills are folded, nothing else.

### S1 — Pre-OS hardware flags (6)

| # | Action (verified file:line) | Verdict | Depends on | Block? | IX | Notes |
|---|---|---|---|---|---|---|
| 1 | `PADSetSpec(5)` :256 | STUB | — | no | — | Sets `__PADSpec` before OSInit; SI-only semantics. Port: no-op symbol. |
| 2 | `OSInit()` :257 → `libs/dolphin/src/os/OS.c:111` | HOST-IMPLEMENT | — | **yes** | D | Subsystems: **Memory/heap** (arena from mmap), **Clock/timer** (GC timebase ticks — keep TB=79.824327 MHz semantics), **Debug/report**. Drop exception vectors, MPU, MemTracker, MetroTRK, `__OSInitSram`, `*0x800000F8` BootInfo reads (9 live sites, p1-datamodel conversion queue). |
| 3 | `DVDInit()` :258 | HOST-IMPLEMENT | 2 | **yes** | — | **Filesystem (DVD)** → bundle/dir provider; async command-block contract must still report *ready* — `InquiryCallback`→`__OSDeviceCode` feeds error paths. |
| 4 | `VIInit()` :259 | HOST-IMPLEMENT | 2 | **yes** | — | **Video / display**: CAMetalLayer + retrace counter. |
| 5 | `PADInit()` :260 | STUB | 2 | no | — | **Input (PAD/SI)** later; zeroed `PADStatus` suffices for logo. |
| 6 | `reset_projection_state()` :261 | KEEP-AS-IS | — | no | — | Pure state init. |

### S2 — Arena carve-out (4)

| # | Action | Verdict | Depends on | Block? | IX | Notes |
|---|---|---|---|---|---|---|
| 7 | `OSGetArenaLo/Hi()` :262-263 | HOST-IMPLEMENT | 2 | **yes** | D | `uaddr` casts — bindings per p1-datamodel rule 2. |
| 8 | cfg fill :264-271 incl. `calc_framebuffer_size_bytes` (gjSystem.c:199) | KEEP-AS-IS | 7 | **yes** | D | 32B-align arithmetic on arena base; `GXNtsc480IntDf` mode struct is referenced only — host never reads GX regs, so its BE layout is inert here. |
| 9 | `OSInitAlloc(...,2)` :272 + heap-start math :273-274 (OSAlloc.c:360) | KEEP-AS-IS | 8 | **yes** | — | Arithmetic is the game's; the allocator behind it is host (**Memory/heap**, S–M). |
| 10 | `init_system_heap_and_global_allocator` :275 → src/pso/system/memory.cpp:26 | KEEP-AS-IS | 9 | **yes** | A | `OSCreateHeap`+global_allocator; game-heap struct asserts may land on the waiver list — verify with `tools/assert_policy.py`. |

### S3 — Video bring-up (2)

| # | Action | Verdict | Depends on | Block? | IX | Notes |
|---|---|---|---|---|---|---|
| 11 | `init_video_and_gx(&cfg)` :276 → gjSystem.c:33 | HOST-IMPLEMENT | 4,10 | **yes** | S | **Graphics pipe** (XL) + **Video/display**: `set_present_interval`(:111), `init_locked_cache_heap`(:92 — LCEnable → drop, heap shim only), `VIConfigure`, `init_gx_display`(:124), `show_first_framebuffer`(:152). GX FIFO byte streams are **not** ported — Metal semantics re-implementation; only the frame math port's as-is. |
| 12 | `OSSetArenaLo(hi)` :277 | HOST-IMPLEMENT | 11 | **yes** | — | One-liner seal against the arena shim. |

### S4 — Render scratch + input wiring (15)

| # | Action | Verdict | Depends on | Block? | IX | Notes |
|---|---|---|---|---|---|---|
| 13 | xmalloc texture_state/surface buffers :278-279 | KEEP-AS-IS | 10 | **yes** | — | 0x4560 + 0x73A0. |
| 14 | `init_texture_tables` :280 | KEEP-AS-IS | 13 | **yes** | A | `GinjaTextureState`/`GinjaSurfaceState` assert scan. |
| 15 | `init_light_globals(0,0)` :281 | KEEP-AS-IS | 10 | no | — | |
| 16 | `set_model_render_flags(0x100000)` :282 | KEEP-AS-IS | — | no | — | |
| 17 | xmalloc decompress/motion/matrix bufs :283-285 | KEEP-AS-IS | 10 | **yes** | — | 0x80000 `resource_decompress_buffer` is blocking (PRS chunks); 0x6000/0x14A0 not. |
| 18 | `init_chunk_draw` :286 | KEEP-AS-IS | 17 | **yes** | S, D | Chunk/DLT payloads are BE → swap-on-load at the disc chokepoint (p1-format-net-disc §5 rule 3), not here; `(saddr)` cast. |
| 19 | `set_frame_update_callback(poll_input_devices_frame)` :287 | KEEP-AS-IS | 5 | no | — | Install is pure; the callback body bottoms at PAD (stubbed). |
| 20 | matrix stack xmalloc+`init_matrix_stack`+`set_matrix_identity` :288-290 | KEEP-AS-IS | 10 | **yes** | — | Needs PSMTX/C_MTX **definitions** to land in the port build (host-surface §4 open item: ~21 pure-math helpers, trivial either way). |
| 21 | `init_pad_slots_and_read(0,0,0)` :291 → **render/ChunkMaterialState.cpp:33** | STUB | 13 | no | — | NOT a PAD call — chunk-material "pad" slots (boot-spine groups it under pads; misgrouped). Safe to keep-as-is too once chunk init exists; STUB until that compiles. |
| 22 | `TObjBoss2Tunnel::enable_and_trigger()` :292 | DEFER | 20 | no | A | Boss debug trigger; TObject-derived — waiver class. |
| 23 | `init_pad_rumble()` :293 → system/PadRumble.cpp:288 | DEFER | 5 | no | — | **Input**; `PADControlMotor` on host = haptics later. |
| 24 | `init_pad_state()` :294 → system/ControllerInput.cpp:546 | KEEP-AS-IS | 5 | no | S | Game-side state only; when PAD feed goes real, `PADStatus` bit-packing (host-surface Input row) needs care. |
| 25 | `set_shape_vertex_buffer((u32*)motion_blend_buffer)` :295 | KEEP-AS-IS | 17 | no | D | `(u32*)` pointer cast — data-model review queue. |
| 26 | debug-text xmalloc + `init_debug_text_buffer`/`color`/`scale` :296-299 | KEEP-AS-IS | 10 | no | — | Diagnostics; keep (cheap, useful during bring-up). |
| 27 | `init_gx_lighting_defaults()` :300 (bodies :~210-246) | HOST-IMPLEMENT | 11 | **yes** | S, A | `GXInitLightPos/Color/LoadLightObjImm` + `GXSetCopyClear` + double `GXCopyDisp` + `gx_default_light_obj` (:79) is a raw 32-B **BE bit-packed HW struct** → bake into pipeline constants, don't overlay. Its final `GXCopyDisp` is what puts the first black framebuffer up — semantically blocking. |

### S5 — Phase/actor core (7)

| # | Action | Verdict | Depends on | Block? | IX | Notes |
|---|---|---|---|---|---|---|
| 28 | `install_depth_alpha_state()` :301 → render/render_util.cpp:89 | HOST-IMPLEMENT | 11 | **yes*** | — | Depth/alpha GX state → `MTLDepthStencilState`/blend. yes* = ASSUMPTION logo quads alpha-blend; if not, downgrades to non-blocking. |
| 29 | `TKeyBoardControl::alloc_buffer()` :302 | STUB | — | no | A | SI keyboard path; `KeyboardDeviceDriver.h:41` f64@0x28 measured *pass* (abi.md §1#2/#3) — latent, no waiver needed, verify. |
| 30 | `reset_main_phase_state()` :303 → :224 (`cur=1`) | KEEP-AS-IS | — | **yes** | — | The logo seed: dispatcher starts at phase 1. |
| 31 | `hardware_keyboard_state.reset_input()` :304 | STUB | 29 | no | — | |
| 32 | `init_buffers()` :305 → TObjPlayer_8023E480.cpp:115 | KEEP-AS-IS | 10 | no | — | Player buffers; logo never draws a player but it's pure logic — keep. |
| 33 | `main_task.init_main_task()` :306 → system/task/TObject.cpp:253 | KEEP-AS-IS | 20,30 | **yes** | A | TObject ancestor = Cause-A waiver traffic (vptr@0 shift fires in 823 TUs per abi.md §1). Logo frames run through `main_task.begin_frame/run_task/render`. |
| 34 | `init_quest_vm_stub()` :307 → TQuestListVM_8007EAF8.cpp:32 (**verified empty**) | KEEP-AS-IS | — | **yes** | — | Already a no-op. Trap: `TLogo::run_frame` (TLogo.cpp:111-113) exits to `request_main_phase(0)` → shutdown if `TQuestVmDownLoadServer::is_ready()` ever becomes true. Keep it never-ready in the stub slice. |

### S6 — Audio + shared assets (8)

| # | Action | Verdict | Depends on | Block? | IX | Notes |
|---|---|---|---|---|---|---|
| 35 | `init_adx_audio_system()` :308 → TLobby_80226C3C.cpp:574 | STUB | 2 | no | — | Whole chain (`adxt_init`, cvFs MFS/GCD, OSThread, `ARInit/ARAlloc/AIInit`, `sndInit`) = **Audio (AR/AI/DSP)** + **Threads/sync** subsystems. Logo runs mute (boot-spine §4). |
| 36 | `init_download_state()` :309 → AsyncFileRead.cpp:289 | KEEP-AS-IS | 10 | **yes** | — | `TLogo::run_frame` calls `process_async_file_reads()` every frame (:107) — queue must exist. |
| 37 | `load_menu_font()` :310 → TNECFontEORFunction.cpp:528 (`S18all04.fon`) | KEEP-AS-IS | 3,11 | no(logo)/**yes(title)** | S | Glyph logic port's as-is; load needs DVD shim + GX texture-upload semantics; font pages cross the disc-swap chokepoint. |
| 38 | `TLobby::load_now_loading_texture()` :311 | DEFER | 3 | no | — | "NOW LOADING" overlay; not in the logo path. |
| 39 | `chara_making_init_stub()` :312 → TCharaMaking.cpp:584 (**verified empty**) | KEEP-AS-IS | — | no | — | |
| 40 | `TAdSinglePlyChrSelectGC::init_char_select_globals()` :313 | KEEP-AS-IS | — | no | A | Pure globals; TObject-derived members → waiver scan. |
| 41 | `reset_sound_request_state()` :314 → audio/SoundRequests.cpp:395 | KEEP-AS-IS | 35 | **yes** | — | `run_frame` calls `update_sound_request_slots()` (:110) every frame — slots must be zeroed even with audio stubbed. |
| 42 | `TLobby::wait_for_sound_init()` :315 | STUB | 35 | **yes** | — | Spins until audio threads publish; **must** stub to immediate return or boot hangs here. |

### S7 — Resource / memcard / gameplay preload + integrity gate (22)

All of this block is after-logo scope; keep symbols compiling as no-op stubs.

| # | Action | Verdict | Depends on | Block? | IX | Notes |
|---|---|---|---|---|---|---|
| 43 | `load_info_icon()` :316 → TLobby_803382E4.cpp:235 (`LoadPvmFileTls("InfoIcon.gvm")`) | DEFER | 3 | no | S | Returns `(saddr)` handle — D review when activated. |
| 44 | `alloc_item_drop_memory_pool()` :317 | DEFER | 10 | no | — | |
| 45 | `alloc_player_motion_aram()` :318 | DEFER | 35 | no | D | ARAM = guest address space (p1-datamodel: hw_dspctrl-class, audio-glue lane). |
| 46 | `TAdWarningBaseGC::init_heap()` :319 | DEFER | 10 | no | — | Warning scenes only. |
| 47 | `TLobby::load_ask_dictionary()` :320 | DEFER | 3 | no | — | |
| 48 | `request_chara_making_archives()` :321 | DEFER | 3 | no | — | |
| 49 | `load_effect_resources()` :322 | DEFER | 3 | no | S | yes*-risk: if logo sprite FX turn out to need it, promote; ASSUMPTION: logo is plain textured quads. |
| 50 | `init_particle_sprite_atlas()` :323 | DEFER | 49 | no | S | Same risk label as 49. |
| 51 | `TParticleCtrl::init_particle_buffer()` :324 | KEEP-AS-IS | 10 | **yes** | — | `init_logo_scene` calls `TParticleCtrl::create_quad_sprites()` (TLogo.cpp:90) — buffer must exist or crash. |
| 52 | `GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR)` :325 | HOST-IMPLEMENT | 11 | no | — | Trivial one-liner mapped inside the Graphics-pipe shim (EFB format choice). |
| 53 | `init_disc_error_messages()` :326 → SwitcherLaunch.cpp:46 | DEFER | 3 | no | — | |
| 54 | `init_keyboard_buffer()` :327 → NetworkConnect.cpp:68 | DEFER | — | no | A,D | Touches `NetBufferRef` family — u32 pool handles must NOT widen (assert enforces, p1-datamodel §3). |
| 55 | `TAdPlayerLoadGC::load_banner_files()` :328 | DEFER | 3 | no | S | |
| 56 | `TLobby::load_player_data()` :329 | DEFER | 3 | no | — | **Memory card** subsystem later. |
| 57 | `init_account_entries()` :330 → NetworkConnect.cpp:837 | DEFER | 54 | no | — | |
| 58 | `load_ha_u032_kage()` :331 → BlobShadow.cpp:60 (`ha_u032_kage.gvm`) | DEFER | 3 | no | S | Blob shadows = in-game, not title. |
| 59 | `system_init_stub()` :332 → text_800FFE04.cpp:6 (**verified empty**) | KEEP-AS-IS | — | no | — | |
| 60 | `TLobby::reset_story_select()` :333 | KEEP-AS-IS | — | no | — | Pure state reset. |
| 61 | `gba_init()` :334 | STUB | 2 | no | — | host-surface GBA row: stub "no GBA attached", S-difficulty. |
| 62 | `load_ply_level_tbl()` :335 → TObjPlayer_801D8ADC.cpp:296 (`PlyLevelTbl.cpt`) | DEFER | 3 | no | S | `.cpt` = encrypted archive; cipher **word-arithmetic is the live BE hazard** (p1-format-saves) — solve in the save/cipher lane, not here. |
| 63 | `verify_game_file_integrity()` :336 → text_802A1020.cpp:~50 | STUB | 3 | no | — | CRC loop itself is endian-safe; the table hardcodes the shipped disc → re-baseline or skip. Set `game_files_modified_flag=0` (text_802A1020.cpp:30). |
| 64 | `TLobby::handle_flag2_request()` :337 | STUB | 63 | no | — | ASSUMPTION: flag2 request has no producer when integrity is stubbed; verify when wiring phase 10. |

## 2. Dispatch ledger — `main` → logo → title

| # | Action (verified file:line) | Verdict | Depends on | Block? | IX | Notes |
|---|---|---|---|---|---|---|
| D1 | `main()` loop SystemBootInit.cpp:365-395 (init→update-until-result→exit; result 1/2/3 → `cur=next`; `cur==0` → shutdown :394) | KEEP-AS-IS | 30 | **yes** | — | Dispatcher is pure logic. |
| D2 | `main_phase_table[]` :340-363 (22 entries; 0,17,20 NULL — guarded by the loop) | KEEP-AS-IS | — | **yes** | — | `MainPhaseEntry` = plain fn pointers, no PMFs → Cause-B waivers not implicated here. Keep the FULL table (boot-spine §4.8). |
| D3 | Phase 1 init `TLogo::init_logo_scene` (table :342; TLogo.cpp:66): copy-clear color, lights, `new TCameraCtrl`, pose, `set_fallback_main_phase(1)`, `run_load_callback_table(logo_load_callback_table)`, `create_quad_sprites`, `new TLogo`, `new TWindowCtrlMain` | KEEP-AS-IS | 14,20,28,30,33,51 | **yes** | A | Requires `is_area_transfer_pending()==0` path (else TLobby keyboard-callback detour — stub side-effects). Load table pulls logo textures (GVM/PRS → row 18's swap chokepoints). |
| D4 | Phase 1 frame `TLogo::run_frame` TLogo.cpp:94: `begin_gx_frame`, camera activate, `main_task.*`, `process_async_file_reads`, `update_sound_request_slots`, `poll_reset_request` (U6 — map to hotkey or never-true), `render_tick` | KEEP-AS-IS + HOST-IMPLEMENT pieces | 19,36,41 | **yes** | — | `render_tick` (TLobby_801E2F94.cpp:491: Z-mode, 640×480 viewport, `copy_efb_to_framebuffer`, present pacing) = **Graphics pipe + Video/display**. Trap: `TQuestVmDownLoadServer::is_ready()` → phase 0 → shutdown (see row 34). `poll_reset_request` (OS.c reset-SW, U6) must read no-request. |
| D5 | `TLogo::run_task` state chain TLogo.cpp:135-197: 0→(TAdVIConfigGC if `logo_clear_flag`; skip) /→`TAdSEGAlogo`; 4→5 `TAdSoniclogo`; 5→6 `TAdADXlogo`; 6→7; ASSUMPTIONS: `check_progressive_ready()`→ready, flag clear (boot-spine U3 unread), state-2 `has_network_adapter(1)`≠0 so modem check is skipped | KEEP-AS-IS | 17,18,20 | **yes** | A | `TAdSEGAlogo` loads `logo_e` via `load_streamed_prs_texture` (TAdSEGAlogo.cpp:103-105) — first real asset through the PRS+swap path. Logo SFX calls (`start_music_crossfade`/`apply_bgm_col_ctrl`) no-op under row 35's stub. |
| D6 | `request_main_phase(0xA)` TLogo.cpp:193-194 + `TLogo::exit_logo_scene` :117 (`delete_layer_children`, reverse-callback teardown, extra `render_tick`) | KEEP-AS-IS | D5 | **yes** | — | This completes "logo done". |
| D7 | Phase 10 init `TAdMultiChgGameModeGC::init_trial_agreement_phase` (table :351; TAdMultiChgGameModeGC_8023E7A8.cpp:452 + `open_trial_agreement_scene` :214, mode 0x17) | KEEP-AS-IS | D6,37 | yes(title) | — | First real title-adjacent screen (trial agreement). `create_mode_scene` BGM calls stubbed. |
| D8 | Scene machine `run_game_mode_select_frame` :494 + `create_mode_scene` :318 | KEEP-AS-IS | D7 | yes(title) | — | |
| D9 | Agree chain: `TAdTrialAgreementGC::on_agree` (:40) → `open_time_set_scene` (mode 8) → `TAdTimeSetGC::apply_settings` (TAdTimeSetGC.cpp:885-899) → `open_title_scene` (:313) → case 0 (:331) `new TTitleScene(tl_01)` | KEEP-AS-IS | D8, Input, 37 | yes(title) | — | Needs **Input (PAD/SI)** real (A to agree/advance) and a clock default: GC clock lives in the memory card (boot-spine §3) — ASSUMPTION: time-set with SRAM/CARD stubbed falls back to a sane default date. |
| D10 | `TTitleScene::run_task` TTitleScene.cpp:289 (attract loop, menu, opening-movie handoff to phase 3) | KEEP-AS-IS | D9 | yes(title) | — | Title = the deliverable of "logo-on-screen + a beat". |
| D11 | `shutdown_and_reset_system()` :124 (via loop exit `cur==0`) | STUB | — | no | — | `OSResetSystem`×4 → host exit/restart hook. |

## 3. Tally (from the tables above)

- Rows ledgered: **64** init actions + **11** dispatch rows = **75** (counts machine-verified against the tables above).
- Verdicts (all 75 rows): KEEP-AS-IS **37** (incl. D4, whose mixed "KEEP-AS-IS + HOST-IMPLEMENT pieces" is counted by its primary) · STUB **11** · HOST-IMPLEMENT **9** · DEFER **18**.
- Blocking for the FIRST logo frame: init rows **2,3,4,7,8,9,10,11,12,13,14,17,18,20,27,28,30,33,34,36,41,42,51** (23, one of them "yes*"=row 28) + dispatch **D1,D2,D3,D4,D5,D6** = **29 of 75 rows**. Title-only blocking adds row 37 + D7,D8,D9,D10 (5 more).
- Everything not blocking is either a verified-empty stub (rows 34, 39, 59), a no-op STUB, or a DEFER link-stub.

## 4. MINIMUM-VIABLE BOOT SUBSET — ordered build checklist

Build order (not source order). Goal per step is cumulative; step 20 is the logo frame.

1. **Build flags first** (unblocks every row): `-DPSO_PORT_LAYOUT_WAIVER` + generated `pso_layout_waiver_list.h` (assert-policy §4); `uaddr/saddr = uintptr_t`; fixed-width `<stdint>` rebind for u32/u16 (p1-datamodel §3 rules 1–2). Nothing else compiles cleanly without these.
2. Host crt0: one ~26 MB `mmap` arena + ARAM block (doubles as the `0x800000F8` BootInfo replacement anchor); zero bss; call `main()`-equivalent. *(implies rows 1–2's substrate)*
3. Mini-`OSInit`: arena lo/hi, GC-timebase clock, heap create, OSReport→os_log. No threads/interrupts/MPU. (row 2)
4. DVD shim: `DVDInit`→bundle handle; open/read→pread; command-block status words report present/ready. Keep `Adxf/CvFs/TGStream` bodies over it. (row 3)
5. Metal window: CAMetalLayer, retrace counter via CVDisplayLink; `VIInit`/retrace API. (row 4)
6. Arena reads + cfg arithmetic verbatim; framebuffers become host MTLTextures, "fifo" = no-op ring. (rows 7–9)
7. `init_system_heap_and_global_allocator` over the heap shim — `OSCreateHeap` semantics, 0x518000 game heap. (row 10)
8. **GX→Metal minimum pipeline slice** ("logo pipeline"): GXInit→device/queue; VIConfigure(GXNtsc480IntDf)→hardcoded 640×480 progressive (get_video_standard()==0 is compile-time-true per gjSystem; ASSUMPTION from boot-spine :120); viewport/scissor; `GXSetPixelFmt` RGBA6_Z24→format choice; `GXCopyDisp`→EFB-to-drawable blit; `show_first_framebuffer`→black clear + present. (rows 11, 12, 27, 52)
9. Present pacing: `present_interval=2` retrace gate (30 Hz), `render_tick` frame tail. (D4's host piece)
10. Texture tables + chunk-draw init; **GVM/GSL swap-on-load** at `open_gsl_archive`/GVM readers (pso_byteswap.h chokepoints) — verify with a golden first texture. (rows 13–18)
11. Matrix stack + PSMTX definitions vendored into the port build. (row 20)
12. Input wiring installed, feed zeroed: rows 1, 5, 19, 21, 24 + `poll_input_devices_frame` stub.
13. Phase seed + dispatcher + full 22-entry table. (rows 30; D1, D2)
14. Actor core: `main_task.init_main_task`, `init_buffers`. (rows 32–33)
15. Per-frame deps: `init_download_state`, `reset_sound_request_state`, quest-VM stub staying not-ready, `poll_reset_request` never-true. (rows 34, 36, 41 + D4 traps)
16. Audio dark: `init_adx_audio_system` no-op, `wait_for_sound_init` return-immediately. (rows 35, 42)
17. Particle buffer + `create_quad_sprites` (atlas/deferred until visual gap). (row 51)
18. Depth/alpha state. (row 28)
19. TLogo scene assets unpacked: `logo_load_callback_table` + `logo_e` PRS texture; force `check_progressive_ready()`=ready, `logo_clear_flag`=0, `is_area_transfer_pending()`=0, `has_network_adapter(1)`=present. (rows 17–18; D3, D5 assumptions)
20. **▶ FIRST LOGO FRAME.** SEGA→Sonic→ADX chain plays to `request_main_phase(0xA)` (D5/D6) — logo *done*; each logo's own assets may stream via step 10's loader.
21. Title extension: `load_menu_font` (row 37), phase-10 machine (D7–D9), `TTitleScene` (D10).
22. Real input (GameController/PADStatus shim) so trial-agreement/time-set/title menus advance (D9); memcard/SRAM stubs defaulting the clock (D9 assumption).
23. Post-title backlog = every DEFER row (S7 block, chara select, warnings, banners, `.cpt` cipher, integrity re-baseline).

## 5. Critical path: host subsystems that must be REAL per subset stage

| Stage (checklist steps) | Subsystem(s) from host-surface.md | Why it's on the path |
|---|---|---|
| 2–3 (process lives) | Memory/heap (S–M), Clock/timer (S), Debug/report (S) | Arena, heaps, tick pacing under everything. |
| 4 (assets reachable) | Filesystem (DVD) (M) | Every KEEP-AS-IS loader body bottoms out here. |
| 5, 8–9 (first pixels) | Video/display (M), **Graphics pipe (XL)** | Clear+present needs the minimal GX-semantics slice; the pipe is the long pole for the *whole* subset. |
| 10–11, 17–20 (logo anim) | Graphics pipe (textured quads, TEV-lite), Math PSMTX (S), Cache/CPU ops (M–L — mostly no-ops; matters only where buffers cross to Metal) | PRS/GVM decode + per-frame draws + GXDrawDone semantics. |
| 21–22 (title) | Input (PAD/SI) (M), Memory card + Flash/EEPROM (M, stub-grade), Graphics pipe font/glyph upload | Menu advance + clock + readable text. |
| (dark, by design) | Audio (L), Network EXI (L), GBA link (S-stub), DSP ucode wildcard | Stubbed at rows 35/42/61; none on the logo path. |

**Biggest single blocker = the Graphics pipe (GX→Metal) semantics slice.** Blocking
rows 11, 27, 28, 52 plus D4's `render_tick` and D5/D3's texture-upload path all
terminate in it; steps 8–9 are the only XL item in the subset, and nothing on screen
exists before it — every other blocking row is S/M difficulty or pure logic.

## 6. Waiver / swap / data-model intersection roll-up

- **A (waiver)** rows: 10, 14, 22, 29, 33, 40, 54, D3, D5 — all Cause-A (TObject-vptr)
  or struct-assert traffic; D2 checked clean of Cause-B (no PMFs in `MainPhaseEntry`).
  Action: run `tools/assert_policy.py` against the port translation unit graph once,
  not per row.
- **S (byteswap)** rows: 11, 18, 27, 37, 43, 49, 50, 55, 58, 62 + D3/D5 asset pulls.
  Rule from p1-format-net-disc §5: **swap-on-load at the disc chokepoint only**
  (`open_gsl_archive`, GVM readers, BootInfo glue); GX light obj (:79 gjSystem) is
  *translated* to pipeline constants, never overlaid; `.cpt` cipher defers to the
  save-lane fix.
- **D (data-model)** rows: 2, 7, 8, 18, 25, 43, 45, 54 — the `(uaddr)`/`(saddr)`
  census maps onto exactly two rules: widen at runtime sites (rule 2), keep u32
  handles at format sites (rule 1, NetBufferRef at 54 is the poster child); the 9
  absolute `*(u32*)0x800000F8` loads die with step 2's glue.

## 7. Deltas vs boot-spine.md (found by reading source; doc was written from the same tree, this lane just re-verified)

1. The 48-action count under-decomposes the source's **73 call statements** (64 rows here) — boot-spine folded S6/S7 field fills.
2. boot-spine S9/S10/S11 text **misses six statements** present at HEAD: `init_quest_vm_stub` :307, `chara_making_init_stub` :312, `init_char_select_globals` :313, inline `GXSetPixelFmt` :325, `system_init_stub` :332, `handle_flag2_request` :337 (the tail of S12). None change the min-viable cut — three are verified-empty stubs.
3. `init_pad_slots_and_read` is **not** a PAD call — defined in `render/ChunkMaterialState.cpp:33` (chunk "pad" slots). Row 21.
4. `reset_sound_request_state` is in `audio/SoundRequests.cpp:395` and IS per-frame-live via `TLogo::run_frame:110` — boot-spine's audio "stub it all" note would hang/UB-crash logo frames if it swallowed this row too. It's KEEP-AS-IS-blocking (row 41).

## 8. Assumptions register (all inline-labeled above)

A1 row 28/D3: logo uses alpha-depth state (conservative: keep it).
A2 row 34/D4: quest-VM download server never becomes ready during logo.
A3 D5: shipped `logo_clear_flag`=0 and progressive-ready true (boot-spine U3 unread).
A4 D5: `has_network_adapter(1)` nonzero path skips modem check.
A5 D9: time-set survives missing memcard with a default clock.
A6 rows 49/50: no particle atlas on the logo chain.
A7 row 64: flag2 request has no producer when integrity is stubbed.
A8 checklist 8: `get_video_standard()` constant-0 (boot-spine :120 claim, not re-verified here).
