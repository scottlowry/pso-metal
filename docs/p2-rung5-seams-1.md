# P2-RS — Rung-5 seam ledger: `TLogo::run_frame` legs 3–9 + `render_tick` + the pre-render legs

Lane RS, 2026-11-27 (card `t_ba7991f3`). ZERO builds, ZERO writes outside this doc.
READ-ONLY inputs: decomp `pso-decomp @ 6b38b5383` (rev verified at intake),
`docs/p2-rung4-contract.md` (AP), `docs/p2-v-wave.md` (BA V1–V4), `docs/p2-v-wave2.md`
(BB V5), `docs/p2-gx-tex-consumer.md` §5, machinery reads: `host/gx/dl/
{gen_ladder6_logo.py,ladder6_gx_prelude.h,ladder6_logo_prelude.h,gx_logo_driver.cpp}`,
`host/bringup/include/ladder6_tex_prelude.h`, `host/bringup/{bringup.mk,ladder6_test.cpp,
ladder5_seam.cpp}`.

**Vigilance clause (era statement).** Waiver-header pin file
`host/bringup/waiver-header.sha256` read at intake: content
`495cc91a5b9694c4c0e4706e453821dcf62c509bc4e610ac9c18673f3c5d061b` (file sha256
`02edff82c6cfd3be…`). BA/BB receipts certify against the OLD header `c6b514035f39b633…`;
the apply-6 wave re-pinned mid-window (metal commit `b59e8b1`). **All parse-state claims
below are the NEW post-apply-6 era**: official corpus
`build/parse/summary.txt` = `TOTAL=1680 FAIL=1330 PASS=350 (target=arm64-apple-macos12.0)`
(target ≠ watchos → battery complete, not mid-run), re-read same turn. Pass/fail per TU
measured by `build/parse/err/<TU>.err` byte-size (0 = pass) — never inferred from
asm-mirror or build-tree presence (era-asm-mirror landmine respected: `.s` citations are
AP-published anchors only).

## 0. What rung-5 is

AP rung-4 stopped at `begin_gx_frame` (`gjSystem.c:41`, wall `TLogo.s 0x8013F5F4`).
BA V4 (`docs/p2-v-wave.md` V4, LOGO-2/3 deviations) now **executes** `request_main_phase`
+ `TLogo::run_frame` (TLogo.cpp:94 verbatim) + `begin_gx_frame` (real), with the **13
legs behind the wall counted, not crossed** (`gx_logo_driver.cpp:84-101` — every leg is
`l6_note(...)` + a constant). This ledger tables exactly those 13 legs (L6Call enum minus
`LC_BEGIN_GX`): era home file:line, reads/writes, crossing class, and the MINIMUM next
rung (one spelled play, ≤2 builds) per seam — ranked cheapest-crossing-first.

Class legend (rung-4 grammar, RS refinement of (b)/(c) for seams):
**(a)** verbatim-slice now, zero externs · **(b)** verbatim-slice + prelude externs only ·
**(c)** class-(d)-seam crossing: slice the body but at least one callee is an
allocator/frame-pump/pixel seam (LB-1/LB-2 pattern) · **(d)** blocked — named residual
lane must land first.

## 1. The 13-leg seam table (era order, `TLogo.cpp:98-114` = AP rows 3–9 + pre-legs)

| # | leg (TLogo.cpp call) | era home (file:line) | reads / writes (measured this turn) | class · residual lane | min next rung (spelled play, builds) |
|---|---|---|---|---|---|
| P1 | `activate_camera_mode(9)` :98 | `camera/TCameraCtrl.cpp:722` | camera-mode table, active-ctrl singleton | **(d)** — lane **J NJS re-pin** (first blocker `NJS_OBJECT_size` @ `NinjaTypes.h:162`, measured err 2156B) | keep LOGO-2 capture seam (mode==9 asserted); cross only after lane J |
| P2 | `set_active_camera_pose(&a,&b)` :99 | `TCameraCtrl.cpp:796` | Vec {0,0,0}/{0,0,-100} args, ctrl pose | **(d)** — lane J (same TU) | seam already pins era literals (`g_pose_a/b`); cross after lane J |
| P3 | `controller_stuff()` :100 | `system/ControllerInput.cpp:278+` | `ControllerInputState`, PadSlot, PAD read | **(d)** — lane J (measured first blocker `NJS_OBJECT_size`, err 2374B; rung4's `ControllerInputContext/State` RW-waives hold but don't cover the NJS include chain) + OS-input lane (dolphin PAD) | counter seam stands; cross after lane J |
| P4 | `update_keyboard_input_frame()` :101 | `ControllerInput.cpp:268-274` | `hardware_keyboard_state`, `is_split_screen`, key counters | **(d)** — lane J (same TU include chain) | counter seam stands |
| L1 | `main_task.begin_frame()` :102 | `system/task/TObject.cpp:640-642` | **empty body** (verified verbatim) | **(a)** | add to slice generator (one anchored `slice_fn`, `TLogo{run_frame}` bare-carrier precedent, logo prelude :50-54) — 1 build |
| L2 | `main_task.run_task()` :103 | `TObject.cpp:283-290` | W `task_flags` UPDATING→UPDATED, `render_pass=0`; R/C `PAUSE_OR_RESUME_CHILD` macro, `TObject::run_tasks()` (:116 tree walk) | **(c)** frame-pump seam (tree walk = LB-2 backing; empty-by-state-7 logo tree makes a counter faithful for tl_02–tl_particle* except `tl_window`, see §3) | flag-spine slice (Play 2) — 1–2 builds |
| R3 | `main_task.render()` :104 — **AP row 3** | `TObject.cpp:509-525` | W `task_flags` RENDERING, RENDERED-on-exit (:524); W `render_pass` seq {0,1,0,1,0}; C `render_geometry` :399 / `render_effects` :432 / `render_particle_effects` :449 / `render_objects` :477 / `render_after_scene` :505 (**empty**) / `render_ui` :460 | **(c)** — 5 of 6 sub-passes are draw-list pixel producers over the 18 `tl_*` globals; owner TU **(d)** lane J (err 2370B, `NJS_OBJECT_size`) | flag-spine slice: verbatim body + 6 sub-pass externs (counters); `render_after_scene` slices real — Play 2, 1–2 builds |
| R4 | `main_task.render_screen_overlay()` :105 — **AP row 4** | `TObject.cpp:644-652` | guard R `task_flags & MAIN_TASK_RENDERED`; W clears UPDATED\|RENDERED (:651); C `render_clipout_and_fade` :292 (pixel), `TObject::debug_render` :174, `debug_render_overlay` :533 (**empty**), `render_info_icon_overlay` (`unk/TLobby_803382E4.cpp:145`, err 2366B → (d) lane J) | **(c)** — same owner-TU block; guard opens only once R3 sets the real flag | rides Play 2 (same build); empty `debug_render_overlay` verbatim-sliced |
| R5 | `process_async_file_reads()` :106 — **AP row 5** | `system/AsyncFileRead.cpp:198-~260` | R/W `async_file_read_state` (:192); R `async_file_read_current_request` (:189, NULL through logo per D10); legs fire `ADXF_Open/ReadNw` (Y shim regime, `dvd_shim.o` already links), `tmp_buf_use_req("FSVR_READPVM")`, `pop_and_close` | **(b)** — **PROMOTION: AP's row-5 (d) flag is superseded.** `AsyncFileRead.cpp` is in the post-apply-6 PASS set (err file 0 bytes; absent from fail.txt 1330). Its 9 globals (:189-197) are plain ptr/int host-definables; `AsyncFileReadRequest`/`THeap` stay opaque-forward decls | verbatim-slice full body + 9 globals zero-def'd + `tmp_buf_use_req` LB-2 stand-in + ADXF counters — Play 1, 1 build |
| R6 | `update_sound_request_slots()` :107 — **AP row 6** | `audio/SoundRequests.cpp:340-~420` | guard R `sound_requests_enabled` (:51, era bss-zero → 0 = audio-dark per boot-spine §4 + ledger row 35); body R `local_client_id`/`get_player_by_client_id`, `camera_ctrl_instance`, `sound_pitch_requests[0x28]`; C `get_sound_voice_state` (musyx), `stop_sound_request`, `check_sound_pitch_request_range` | **(b)** guard leg (body never enters when enabled==0 — verbatim slice keeps ALL callee decls unbound except the flag). Full body = **(d)**: measured first blocker `NJS_OBJECT_size` (err 2366B) behind AP's `SystemSaveData` residual; lanes J + musyx (P2-M) | verbatim-slice body + `sound_requests_enabled` zero-def — Play 1, 1 build |
| R7 | `poll_reset_request()` :108 — **AP row 7** | `ControllerInput.cpp:258-263` | R `memcard_access_active`, `controller_reset_requested` (:24); C `DVDCheckDisk` (**already bound** — `dvd_shim.o` in ladder6 link line, bringup.mk:371), `check_reset_button`, `reset_system` (fail-loud stand-in, LB-3 pattern) | **(b)** — 6-line body, 2 trivial globals; TU (d) irrelevant to the slice (rung4 §4 fallback) | verbatim-slice + fail-loud `reset_system` — Play 1, 1 build |
| R8 | `TQuestVmDownLoadServer::is_ready()` :111 — **AP row 8** | `unk/TLobby_800F8AA8.cpp:1371-1380` | C `TLobby::wait_ready_loop` (:981-993 — UNCONDITIONAL frame pump: loops `render_tick()` until `run_disk_error_check()==1`; on disk-OK it returns on the first iteration WITHOUT pumping — verified against the body); R `memcard_access_active`, `quest_download_in_progress` (`quest/TQuestVmDownLoadServer.cpp:50`), `quest_download_server_ready` (`TLobby_800F8AA8.cpp:91`) — all zero on host = era default | **(c)** frame-pump seam — LB-2 machinery already exists (`pump_frames`, `TLobby::run_frame` counters, ladder6_test.cpp:71-74): reuse, don't re-mint. Owner TU (d) lane J (err 2140B) | verbatim-slice `is_ready` + `wait_ready_loop` pump-count extern + 3 globals zero-def; `request_main_phase(0)` stays bound (trap leg can never fire with ready==0) — Play 1, 1 build |
| R9 | `render_tick()` :114 — **AP row 9** | `unk/TLobby_801E2F94.cpp:491-506` | 13 sub-legs: `TKeyBoardControl::run_tasks` (home `TKeyBoardControl_80290CB0.cpp` — STILL failing post-apply-6, err 2209B; AP's "flips under stitch alone" predates the corpus and is NOT observed in the 350), `TParticleCtrl::begin/end_effect_render` (`effect/particle/TParticle.cpp` err 2162B), `GXSetZMode`×2 + `GXSetViewport(640×480)` + `set_cull_none`×2 (host/gx seam shape exists, `ladder6_gx_prelude.h:32-38`), `render_debug_text`, `copy_efb_to_framebuffer` + `present_frame` (Z dl lane — LOGO-3 host backing already presents at this exact era position, `gx_logo_driver.cpp:182-198`), `run_end_of_render_tasks` | **(c)** decomposition now / **(d)** era-body-complete (owner TU: measured first blocker `NJS_OBJECT_size`, err 2154B; + dialect `GXVert.h:25` behind the cap — AP §1) — lane J | Play 3: wrap the existing LOGO-3 present in an era-ordered 13-sub-leg roster (GX legs → real seam calls; (d)-home legs → LB-3 fail-loud), 2 builds |

Rollup (13 legs): **(a) 1 · (b) 3 · (c) 5 · (d) 4**. The (c)s cross THIS rung with
existing machinery; the (d)s are all lane-J-gated at the first blocker — measured, not
inherited.

## 2. Cheapest-crossing-first ranking + the three named plays

1. **Play 1 — "zero-pixel seam slice" (1 build).** Ladder7 slice TU (new
   `gen_ladder7_runframe.py` off the proven `slice_fn` anchor/guard machinery of
   `gen_ladder6_logo.py`, bringup.mk insert-only rule): verbatim bodies of
   `begin_frame`(L1), `poll_reset_request`(R7), `process_async_file_reads`(R5),
   `update_sound_request_slots`(R6), `is_ready`(R8) + host-def'd trivial globals +
   LB-2/LB-3 stand-ins. Kills five `return 0`/no-op counter stubs.
   **What-green:** run_frame legs EXECUTE era bodies (call-order log L7-R3 unchanged);
   invariants now proven by real reads: `controller_reset_requested==0` ∧
   `async_file_read_state==0` ∧ `sound_requests_enabled==0` ∧
   `quest_download_server_ready==0` → poll==0, no fallback fire, no
   `request_main_phase(0)`; counters for pump/ADXF/voice-state stay 0.
2. **Play 2 — "main_task flag spine" (1–2 builds).** Slice L2+R3+R4 verbatim; the 8
   sub-passes (`run_tasks`, `render_geometry`, `render_effects`,
   `render_particle_effects`, `render_objects`, `render_ui`, `render_clipout_and_fade`,
   `render_info_icon_overlay`) become counted externs; `render_after_scene` and
   `debug_render_overlay` slice real (both empty, `TObject.cpp:505/533`).
   **What-green:** `render_pass` sequence == {0,1,0,1,0(,0)} and `MAIN_TASK_RENDERED`
   set by the REAL body at :524 and consumed+cleared by the REAL overlay guard at
   :645/:651 — the overlay leg's guard flips from stub-honesty to flag-honesty.
3. **Play 3 — "render_tick leg decomposition" (2 builds).** 13-leg roster per §1 R9
   inside the existing LOGO-3 driver seam. **What-green:** sub-leg order ==
   `TLobby_801E2F94.cpp:491-506` text order; `GXSetViewport(0,0,640,480,0,1)` recorded
   twice per frame (begin_gx_frame + render_tick); L7-P1/P2/P3 pixel gates
   (run_window.sh) byte-identical under the new shape — present stays single.

Then lane J (below) converts P1–P4 + R6-full + R9-complete from (d) to slice-able.

## 3. Cross-check vs existing seams — NOTHING here re-ports

| existing | state | rung-5 relation |
|---|---|---|
| `begin_gx_frame` (AP row 32) | REAL verbatim, executing (V4/LOGO-1, `ladder6_gx_glue.c`) | out of ledger — the wall itself |
| `request_main_phase` / `TLogo::run_frame` | REAL verbatim slices (ladder3 + V4 glue) | rung-5 edits only the **seam bodies the slice calls**, never the slice |
| LB-1 alloc seam → AU-4 cache heap (`ladder6_tex_prelude.h:51`, `ladder5_seam.cpp:28`) | landed | reused for any Play-1 heap def; AU-4 NOT re-ported |
| LB-2 pump seams `pump_frames`/`TLobby::run_frame`/`debug_print` (`ladder6_test.cpp:71-74`) | landed | R8 `wait_ready_loop` joins THIS roster — no new seam mechanism |
| LB-3 fail-loud loader stand-ins | landed | pattern reused for `reset_system` + Play-3 (d)-home legs |
| LB-5..LB-9 (DCFlushRange / linkage puns / `texture->p+8` / typed forwarder / psoBe loads) | landed | texture-chain-local; untouched by rung-5 |
| **LB-4** | **declared nowhere in the tree** (grep-verified, docs+code+gen) | numbering gap in the V-wave doc's "LB-1..LB-9" range, NOT a lost port — flagged for BA/BB re-ack |
| LOGO-3 `render_tick` host present | landed (`gx_logo_driver.cpp:182-198`) | Play 3 WRAPS it; a second `present_frame` must never appear |
| AU-4 heap, era CLUT supply (BB S1), `load_sin_board_image` chain (V2) | landed | supply side — orthogonal to this ledger |

Faithfulness note for Play 2: at state 7 the TLogo ctor-spawned ad children are all
`queue_destruction`'d (`TLogo.cpp:169/177/187`), but `new TWindowCtrlMain` (:91) lives
on `tl_window` — window draw-list nodes may be real by design, which is exactly why the
6 pixel sub-passes stay counters and pixels keep riding LOGO-3's single present.
Actor-driven sinbd QUAD (`TObjSinBoard::render`, BA blocker-2) is AP §4 target-TU
territory — not a run_frame leg, not double-counted here.

## 4. One-line ASKs (per blocker)

- **ASK lane J (NJS overlay re-pin, AP §4 step 1, `NinjaTypes.h:162-163`):** first
  blocker for ALL 8 remaining (d)/(c)-complete homes (measured this turn) — one header,
  ten-blocker-family key; ETA?
- **ASK integrator:** apply BA's submitted-not-applied `build/njs_allowlist.txt` +
  PARSE_RECIPE (AV §4 block) so Play-1/2 slices can compound into parses, not just runs.
- **ASK musyx lane (P2-M):** voice-state accessor names for the `R6` full-body leg
  (`get_sound_voice_state`) — not needed for the guard-leg slice.
- **ASK save-blob lane (p1-format-saves):** confirm `SystemSaveData`/`TItemData` status
  behind apply-6 (AP's row-6/7 residuals vs this turn's NJS first-blocker measurement).
- **ASK BA (receipt hygiene):** ack the LB-4 numbering gap in `p2-v-wave.md` §V1 range.

## 5. Method / honesty notes

- All line numbers re-opened in the decomp this turn (bodies quoted shapes, not
  inherited from AP's table); AP row refs kept for cross-doc navigation.
- Parse states are THIS-TURN `build/parse/err` size reads on the official post-apply-6
  corpus (macos target → not mid-run). No compiler invoked by this lane.
- Row-5 promotion (AP (d) → parse-PASS) is the single classification change vs
  `p2-rung4-contract.md`; nothing else on the 13 legs flipped.
- Cap caveat stands (AP §6): (d) lists can only gain, never lose, blockers under a cap
  lift; err logs here are head-blockers, not full rosters.
