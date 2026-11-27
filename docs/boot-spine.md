# PSO Metal — Boot Spine (Phase 0, task D)

Decomp reference: `/Users/scott/Hermes/pso-decomp/pso-decomp` (GC PSO EP&I, disc build GPOE8P).
All paths below are relative to that tree. Verified by reading the actual functions, not inferred.

## 0. Real entry sequence (crt0 → main)

| Step | Symbol | Location | Notes |
|---|---|---|---|
| A1 | `__start` | `.init:0x8000C040` (config/GPOE8P/psov3/symbols.txt:2); asm body `orig_asm/os/__start/__start.inc`, wrapped by `__declspec(weak) asm void __start()` at libs/dolphin/src/os/__start.c:17 | Nintendo Dolphin CRT0: sets up GPR/FPR, MSR (machine-state words are pure PPC), FP mode, then calls `__init_registers` → `__check_pad3` → `__init_data` → `main`. |
| A2 | `__init_registers` | libs/dolphin/src/os/__start.c:21 (asm .inc) | zeroes regs, sets stack. |
| A3 | `__check_pad3` | libs/dolphin/src/os/__start.c:10 | if the 4th controller port read-back at `0x800030E4` is all-1s → `OSResetSystem` (hold-B reset combo). SI-poll specific; port replaces with a key combo or drops it. |
| A4 | `__init_data` | libs/dolphin/src/os/__start.c:36 | walks `_rom_copy_info` / `_bss_init_info`: memcpy .data from disc-image ROM offset into RAM + `__flush_cache` per section; memset .bss. Port: dyld does this for us; the flush_cache half disappears on ARM. |
| A5 | `main` | src/pso/ui/keyboard/SystemBootInit.cpp:365, `.text:0x8000FD54` | prototype `int main(int argc, char** argv)` at libs/dolphin/src/os/__os.h:133 (game defines it as `void main(void)`). Body: `init_game_systems()`, then the phase-dispatcher loop (§3), then `shutdown_and_reset_system()` (SystemBootInit.cpp:124). |

## 1. Ordered init ladder — `init_game_systems()` (SystemBootInit.cpp:249, `.text:0x8000FAE4`)

Execution order as read from source (line = SystemBootInit.cpp unless stated). 48 discrete init actions, grouped into 12 stages below.

| # | Call (file:line) | Needs | PPC/BE hazard |
|---|---|---|---|
| S1 | `PADSetSpec(5)` (:256) | sets `__PADSpec` **before** OSInit so the BI2 debug branch doesn't overwrite it | raw SI register semantics |
| S2 | `OSInit()` (:257) → libs/dolphin/src/os/OS.c:111 | BootInfo block at cached physical 0 (`OSPhysicalToCached(0)`), `__ArenaLo/__ArenaHi` linker symbols, exception vectors at 0x100..0x1700 | **Massive PPC surface**: `OSDisableInterrupts`, `PPCDisableSpeculation`/`PPCSetFpNonIEEEMode` (MFR/MCR of HID0/FPSCR, OS.c:122-123,159), `__OSInitMemoryProtection` (bat/MPU), MemTracker/`ClearArena`, MetroTRK. Port substitutes: own boot-info, own arena from malloc'd region, no cache flushes needed the same way. |
| S3 | `DVDInit()` (:258) | disc drive reset + async `DVDInquiryAsync` (also called inside OS.c:206) | On the port: replaces with the bundle/dir resource-file provider; OS.c's `InquiryCallback` sets `__OSDeviceCode` used by error paths. |
| S4 | `VIInit()` (:259), `PADInit()` (:260) | SI/EXI hardware | VI → Metal/CAMetalLayer display; PAD → Game controller input. |
| S5 | `reset_projection_state()` (:261) | nothing | — |
| S6 | Arena carve-out (:262-275): `OSGetArenaLo/Hi`; `cfg.render_mode=&GXNtsc480IntDf` (:265, hard NTSC — `get_video_standard()` at :120 always returns 0); `calc_framebuffer_size_bytes` (gjSystem.c:199); two framebuffers 32B-aligned; `fifo_size=0x40000`; `OSInitAlloc(fifo_end, hi, 2)` (libs/dolphin/src/os/OSAlloc.c:360); heap size `0x518000`; `init_system_heap_and_global_allocator` (src/pso/system/memory.cpp:26 → `OSCreateHeap` + `global_allocator`) | OSInit done (arena set) | All pointers are 32-bit guest addresses; ILP32 keeps arithmetic valid but the heap is fed from the boot arena — port must expose a contiguous 24 MB-ish arena. |
| S7 | `init_video_and_gx(&cfg)` (:276) → src/pso/render/gjSystem.c:33: `set_present_interval` (:111, interval=2 retraces, VIGetRetraceCount), `init_locked_cache_heap` (:92 — `LCEnable()` + `OSCreateHeap` for the locked-cache heap), `VIConfigure(GXNtsc480IntDf)`, `init_gx_display` (:124 — `GXInit` FIFO, viewport/scissor/disp-copy src/dst/yscale, AA filter, `GXSetPixelFmt(GX_PF_RGB8_Z24)`, 0x40 TLUT regions), `show_first_framebuffer` (:152); then `OSSetArenaLo(hi)` (:277) seals the static arena | S6 | LCEnable = PPC l2-lock lines (GX xfmem/vtx-cdt sim). GX FIFO bytes are BE GPU display lists → **re-implement GX semantics on Metal, do not port the byte stream**. |
| S8 | Ginja/render scratch (:278-300): xmalloc texture_state/surface buffers, `init_texture_tables`, `init_light_globals`, `set_model_render_flags(0x100000)`, 0x80000 `resource_decompress_buffer` + `init_chunk_draw`, `set_frame_update_callback(poll_input_devices_frame)` (:287), `init_matrix_stack`, `init_pad_slots_and_read`, `TObjBoss2Tunnel::enable_and_trigger`, `init_pad_rumble`, `init_pad_state`, `set_shape_vertex_buffer`, debug-text buffer, `init_gx_lighting_defaults` (:231 — GXInitLightPos/Color/LoadLightObjImm + `GXSetCopyClear` + double `GXCopyDisp` + `GXSetPixelFmt(RGBA6)` later at :325) | S7, heaps | `GXLightObj` is a raw 32-byte HW-format struct (gx_default_light_obj, :79) — BE bit-packing. Chunk/DLT texture data on disc is BE. |
| S9 | Phase/actor system (:301-306): `install_depth_alpha_state`, `TKeyBoardControl::alloc_buffer`, `reset_main_phase_state()` (:224 — **cur=1**), `hardware_keyboard_state.reset_input`, `init_buffers` (TObjPlayer_8023E480.cpp:115), `main_task.init_main_task()` (src/pso/system/task/TObject.cpp:253) | S8 | port-free logic |
| S10 | Audio (:307-315): `TLobby__sub_8007EB70`, `init_adx_audio_system` (src/pso/unk/TLobby_80226C3C.cpp:574: `adxt_init`, `setup_adx_file_systems` (src/pso/audio/AdxManager.cpp:39 — cvFs devices "MFS"/"GCD", default GCD), `init_runtime_threads` (OSThread!), `ARInit/ARQInit` (ARAM), `AIInit`, `VISetPre/PostRetraceCallback`, `ARAlloc(0xC00000)`, `sndInit(48,48,48,4,1,...)` (src/musyx/runtime/snd_init.c:24), 2×stereo + 1 mono adxt stream players), `init_download_state`, `load_menu_font` (src/pso/ui/font/TNECFontEORFunction.cpp:528 — `load_disc_file("S18all04.fon")`), `TLobby::load_now_loading_texture`, `reset_sound_request_state`, `TLobby::wait_for_sound_init()` (:315 — **spins until the audio threads publish**; stub it to skip audio) | S3/S7/S8; DSP/ARAM; criware ADX + Musyx | ARAM 24 MB window + DSP thread model = full replacement. ADX sample format itself is endian-agnostic; .gsl/.afs partition tables are BE. |
| S11 | Resource/memcard/gameplay preload (:316-336): `load_info_icon`, `alloc_item_drop_memory_pool`, `alloc_player_motion_aram`, `TAdWarningBaseGC::init_heap`, `TLobby::load_ask_dictionary`, `request_chara_making_archives`, `load_effect_resources`, `init_particle_sprite_atlas`, `TParticleCtrl::init_particle_buffer`, `init_disc_error_messages`, `init_keyboard_buffer`, `TAdPlayerLoadGC::load_banner_files`, `TLobby::load_player_data`, `init_account_entries`, `load_ha_u032_kage`, `fn_800FFE10`, `TLobby::reset_story_select`, `gba_init` (GBA link via EXI), `load_ply_level_tbl` | S10 + file layer | GBA download path (EXI channel) is dead weight for the port; memcard = DVD→no-op is fine for title. |
| S12 | Integrity gate (:336): `verify_game_file_integrity` (src/pso/unk/text_802A1020.cpp ~:50: sizes + MSB-first CRC32 of 11 disc files → `game_files_modified_flag`) | file layer | CRC loop is `crc ^= buf[j] << 24` (bit-serial, endian-safe) but **the table hardcodes the shipped disc's CRCs** — port must skip or re-baseline. |

File layer used by all of S10-S12: `load_disc_file` (src/pso/unk/TLobby_800F8AA8.cpp:1382) → `TGStream::load_file` (src/pso/system/TGStream.cpp:120): GSL-archive lookup first (`find_gsl_entry` :206; `open_gsl_archive` :238 uses `ADXF_Open/ADXF_ReadNw`), else raw ADXF/DVD sector reads (2048-aligned, `TQuestVmDownLoadServer::alloc` buffer). `Adxf.cpp` (src/pso/audio) and `CvFs.cpp` (src/pso/system) are **already decompiled game-side code** that only bottom out at DVD + OS thread calls.

## 2. Port-free vs SDK-surface

**Port-free (pure decompiled logic; compiles once types/headers settle, endianness aside):**
`main` + phase loop, `init_game_systems` call ordering, MainPhaseState/table, TLogo state machine, game-mode scene machine (`TAdMultiChgGameModeGC_8023E7A8.cpp`), TTitleScene, memory.cpp allocator wiring, gjSystem frame math (minus GX calls), TGStream/Adxf/CvFs (minus DVD/thread calls), musyx src/musyx, AdxtInit, load_menu_font glyph logic, the whole of `src/pso/ui/frontend`.

**SDK the port must provide (dolphin + middleware surface on the boot path):**
1. **GX → Metal**: GXInit/FIFO, disp-copy, viewport/scissor/pixelfmt, light objects, TLUT regions, GXDrawDone — semantics layer, ~everything in init_gx_display + begin_gx_frame/copy_efb_to_framebuffer (gjSystem.c:40-90) + render_tick (src/pso/unk/TLobby_801E2F94.cpp:491: Z-mode, viewport 640×480, `copy_efb_to_framebuffer`, `present_frame` with retrace pacing).
2. **VI/OS timing**: VIConfigure, retrace count + WaitForRetrace (present_interval=2 → 30 Hz update gate), OSTime/OSGetSystemTime, OSResetSystem (used by shutdown_and_reset_system:151-154), OSCreateHeap/OSInitAlloc (could shim over a real allocator).
3. **DVD/AFS → bundle FS**: DVDInit/DVDOpen/DVDReadAsync+sync callbacks, DVDInquiry; Adxf bottoms out here.
4. **PAD/SI**: PADInit/PADRead/padSpec, the SI keyboard (TKeyBoardControl path is on the boot spine!), EXI/GBA link (stub), SRAM (OSInit's `__OSInitSram`).
5. **DSP/ARAM + CRI audio**: ARInit/ARAlloc, AIInit, VI retrace audio callbacks, sndInit's DSP thread (src/musyx), and the `game->aram` motion data (`alloc_player_motion_aram`).
6. **CRT0**: replaced wholesale by host startup (§0 A1-A4).

## 3. Phase dispatcher → title screen

- Loop: `main` SystemBootInit.cpp:370-393. `main_phase_state.cur` (s32, init to **1** by `reset_main_phase_state` :224) indexes `main_phase_table[]` (:340, `.data:0x804805E0`, 22 `{init, update, exit}` entries, type in include/pso/system/MainPhaseTable.h). Runs `init`, then `update` until `result != 0`, then `exit`; result 1/2/3 → `cur = next`. `cur == 0` exits loop → `shutdown_and_reset_system`.
- **Phase 1 = TLogo**: `{TLogo::init_logo_scene, TLogo::run_frame, TLogo::exit_logo_scene}` (:342; bodies src/pso/ui/TLogo.cpp:66/:94/:117). Logo scene runs the SEGA→Sonic→ADX chain in `TLogo::run_task` (TLogo.cpp:135) → state 7 (:193) `request_main_phase(0xA)`.
- **Phase 10 (0xA) is where the real title screen first appears** (:351, `TAdMultiChgGameModeGC::init_trial_agreement_phase`): init_game_mode_select_scene (TAdMultiChgGameModeGC_8023E7A8.cpp:452) + open_trial_agreement_scene (:214, mode 0x17); the frame machine run_game_mode_select_frame (:494) swaps the scene object via create_mode_scene (:318). Chain inside phase 10: TAdTrialAgreementGC → on_agree (TAdTrialAgreementGC.cpp:40) → open_time_set_scene (mode 8, TAdTimeSetGC) → `TAdTimeSetGC::apply_settings` (TAdTimeSetGC.cpp:885-899) → `open_title_scene` (:313, mode 0) → create_mode_scene case 0 (:331) → **`new TTitleScene(tl_01)`**.
- **Named title *phase* = entry 2**: `{TAdMultiChgGameModeGC::init_title_phase, run_game_mode_select_frame, TLobby::release_scene_resources}` (SystemBootInit.cpp:343) — same scene machine, opened directly (e.g. after ending: set_queued_main_phase(2)/request_main_phase(2) in EndingSequence.cpp:41/98/101).
- Title behavior: TTitleScene::run_task (TTitleScene.cpp:289) bounces back to time-set if no memcard (GC clock lives in the memory card save!), dispatches menu choices, and after `attract_timeout` launches the opening movie: `set_fallback_main_phase(2); request_main_phase(3)` (movie phase 3, SystemBootInit.cpp:344). Return paths: TTitleScene::open_keyboard → phase 19; `enter_game_mode_change` → dialing select, etc.

## 4. Minimum viable boot cut (black window + logo, fastest)

Smallest ladder that should show the SEGA logo animation [2026-11-27 ledger note: PLAN, not achieved capability — rung-6 runs the FIXTURE path; genuine-record logo render DEFECT root-caused flat-vs-TI-8x4 (issue #8; docs/p2-oracle-1.md)]:

1. Host crt0 → allocate one arena, zero bss, `main()` equivalent.
2. Mini-OSInit: heap create, timers, (no exceptions/threads/SI).
3. FS shim: `open/size/read` on an unpacked-disc dir + stub `DVDInit`/inquiry; keep Adxf/TGStream code if bundle stays AFS-shaped, else short-circuit `find_gsl_entry`.
4. Heaps: replicate S6 exactly (OSCreateHeap×2 + global_allocator + `xmalloc`); sizes 0x518000 game heap, 0x40000 FIFO.
5. Video: init_video_and_gx against a Metal backend: 640×480 double fb (NTSC480 mode struct only — hardcode, `get_video_standard()==0`), retrace-paced present (present_interval=2), plus init_gx_lighting_defaults → just clear-black + copy.
6. Input stub: `set_frame_update_callback(poll_input_devices_frame)` with read_pads→host controller, keyboard + lobby reset→no-ops.
7. Actor/render core: main_task.init_main_task, init_matrix_stack, init_texture_tables, init_chunk_draw + Ginja texture loader (PRS-compressed BE chunks — `load_streamed_prs_texture("logo_e",…)` TAdSEGAlogo.cpp:103-105), TParticleCtrl::create_quad_sprites, render_tick.
8. Phase loop + `reset_main_phase_state` + table entries 1 and 10 only (others NULL → instant exit avoids infinite loop? no: keep full table, just don't reach others).
9. TLogo scene with `is_area_transfer_pending()==0` path; run its load table (needs TCameraCtrl, TWindowCtrlMain, logo textures).

**Deliberately skipped:** all of S10 audio (`TLobby::wait_for_sound_init` → stub return immediately; adxt/musyx/ARAM absent → `apply_bgm_col_ctrl("advr.adx")` in create_mode_scene and `start_music_crossfade` become no-ops), S11 GBA/memcard/network/lobby loads, S12 integrity gate (or set `game_files_modified_flag=0`), `init_disc_error_messages`, TCharaMaking/char-select. Font (`load_menu_font`) is needed for readable title text but **not** for the logo — add it second.

Reaching a *text-bearing* title = above + load_menu_font (S10's only cheap-to-port member, needs the GX texture-upload for the glyph pages) + phase-10 scene machine + TTitleScene/TTitleMenu assets.

## 5. Unknowns

- **U1** `__start` asm internals live in `orig_asm/os/__start/__start.inc` (not re-read line-by-line; standard NTTPLP layout assumed).
- **U2** Exact disc path/where `logo_e`, `sega_logo_eNN`, `gc_adxNN`, `S18all04.fon`, `.gsl` archives physically live (root vs which afs partition) — needs orig/ disc image inspection, out of scope here.
- **U3** `TAdVIConfigGC` branch (TLogo.cpp:140: runs VI-config prompt first when `logo_clear_flag` set — dev-flag path; which shipped flag value EP&I uses is unread).
- **U4** Whether real production boot passes through phase 2 `init_title_phase` at all on first boot (evidence says no: first title is inside phase 10; phase 2 exists for post-ending return) — low risk either way.
- **U5** `TQuestVmDownLoadServer::alloc` (backing arena for all load_disc_file buffers) — allocation site unread; presumably xmalloc from game heap.
- **U6** `poll_reset_request`/reset-switch path (OS.c:151 `__OSResetSWInterruptHandler`) has no native analog — decide on a hotkey.
- **U7** GBA-link `gcrna_bridge_init_buffers` declared with 3 args in TLobby_80226C3C.cpp:287 but AdxUiBridge.cpp:16 shows 0 — decomp drift; harmless for boot, note for the audio lane.
