/* P2-L7 boot ladder rung 5 PLAY-1 + PLAY-2 — the logo main-phase machine runs
 * its five promoted legs AND the main_task flag spine as ERA BODIES
 * (contract docs/p2-rung5-seams-1.md Play 1 + Play 2; spec
 * docs/p2-ladder7-play23.md §1; seam ledger rows L1/R5/R6/R7/R8 + L2/R3/R4).
 * The run_frame carrier, the five promoted legs, the flag spine (L2
 * TObject.cpp:283-290 / R3 :509-525 incl. the :510 macro re-pin / R4 :644-652)
 * and the three era-empty REAL bodies arrive VERBATIM from the read-only
 * decomp via gen_ladder7_runframe.py; everything still behind lane J / Play 3
 * stays a counted seam (L7-2) and everything the era-true zero state can never
 * select is an LB-3 fail-loud stand-in (L7-3).
 *
 * GREEN MEANS (RS Play-2 What-green, verified by REAL reads): render_pass
 * sequence == {0,1,0,1,0(,0)} and MAIN_TASK_RENDERED set by the REAL body at
 * :524, consumed + cleared by the REAL overlay guard at :645/:651 — the
 * overlay leg's guard flipped from stub-honesty to flag-honesty. Probes
 * L7-P5..P9 (macro fire, L2/R3/R4 spines, guard honesty, full chain) and
 * invariants L7-I2a..e read the era stores through the same globals the
 * sliced bodies write. PLAY-1 asserts keep their labels and texts except: the
 * L7-R3a/R3b/R3d expected vector is re-targeted per spec §1.5 (the three
 * counter seams left ladder7_seam.cpp — real slices count via the walk
 * extern + sub-pass roster), and the two post-probe restoration lines are
 * relabeled I2f/I2g to hand the I2a..e namespace to the spec's flag-spine set
 * (assertions unchanged).
 */
#include "ladder7_run_prelude.h"
#include <stdio.h>
#include <string.h>

extern "C" void DVDInit(void);   /* pso_bringup.h:107 shape; fake_disc regime */

static int failures, total;
#define CHECK(cond, ...) do { total++; if (!(cond)) { failures++; \
    printf("FAIL "); printf(__VA_ARGS__); printf("\n"); } \
    else { printf("PASS "); printf(__VA_ARGS__); printf("\n"); } } while (0)

static const int kFrameOrder[] = {
    LC7_BEGIN_GX, LC7_ACT_CAM_MODE, LC7_SET_POSE, LC7_CTRL_STUFF, LC7_KBD_FRAME,
    LC7_WALK_MAIN,                                        /* run_task :287 (REAL slice) */
    LC7_GEOM, LC7_EFF, LC7_PTEX, LC7_OBJ, LC7_UI,         /* render :513..:522 (REAL slice) */
    LC7_CLIP, LC7_INFO,                                   /* overlay :646/:649 (REAL slice) */
    LC7_READY, LC7_RENDER_TICK,
};
#define FRAME_EVENTS ((int)(sizeof kFrameOrder / sizeof kFrameOrder[0]))

int main() {
    setvbuf(stdout, 0, _IOLBF, 0);
    DVDInit();   /* fake_disc present -> DVDCheckDisk TRUE (rung4 U6 regime) */

    /* ============ I0: era-zero starting state (REAL global reads) ============ */
    CHECK(async_file_read_state == 0 && async_file_read_current_request == NULL
          && lobby_download_state_ptr == NULL,
          "L7-I0a async-read globals zero (AsyncFileRead.cpp:189-197 host zero-def; RS row-5 promoted TU)");
    CHECK(controller_reset_requested == 0 && memcard_access_active == 0,
          "L7-I0b controller_reset_requested==0 && memcard_access_active==0 (ControllerInput.cpp:24 / SystemBootInit.cpp:87 zero-def)");
    CHECK(sound_requests_enabled == 0,
          "L7-I0c sound_requests_enabled==0 — audio-dark (SoundRequests.cpp:51; boot-spine §4 + ledger row 35)");
    CHECK(quest_download_in_progress == 0 && quest_download_server_ready == 0
          && lobby_wait_ready_flag == 0,
          "L7-I0d quest/download globals zero (TQuestVmDownLoadServer.cpp:50, TLobby_800F8AA8.cpp:91/:93)");
    CHECK(main_phase_state.cur == 0 && main_phase_state.prev == 0
          && main_phase_state.next == 0 && main_phase_state.result == 0,
          "L7-I0e main_phase_state untouched pre-run (trap legs must not fire)");
    CHECK(render_pass == 1,
          "L7-I0f render_pass == 1 host-def era INIT (TObject.cpp:25 .data — spec §3 D7: zero-init would drift the first-frame trace)");
    CHECK(update_flags == 0 && old_update_flags == 0 && render_flags == 0 && old_render_flags == 0,
          "L7-I0g .bss flag quartet update/old_update/render/old_render all zero pre-run (TObject.cpp:53-56 zero-defs; RS 'all zero = era default' holds for the quartet)");

    /* ============ THE RUNG: era run_frame x3 ============ */
    TLogo logo;
    for (int f = 0; f < 3; f++) logo.run_frame();   /* TLogo.cpp:94 verbatim */

    /* ============ L7-R3: call order == era text order ============ */
    CHECK(g_call_n == 3 * FRAME_EVENTS,
          "L7-R3a order log holds 3 frames x %d events (PLAY-2: run/render/overlay counter slots left the ladder — the flag spine counts through WALK_MAIN :287 + GEOM/EFF/PTEX/OBJ/UI :513-522 + CLIP/INFO :646/:649; MT_BEGIN/ASYNC/SOUND/POLL slots stay era-occupied, no stub notes; LC7_READY fires INSIDE the era is_ready chain at wait_ready_loop)", FRAME_EVENTS);
    int order_ok = g_call_n == 3 * FRAME_EVENTS;
    for (int i = 0; order_ok && i < g_call_n; i++)
        order_ok = g_call_seq[i] == kFrameOrder[i % FRAME_EVENTS];
    CHECK(order_ok, "L7-R3b sequence == TLogo.cpp:95-114 text order: %s (LC7_READY bracket proves the ASYNC+SOUND+POLL slots were traversed before it)",
          order_ok ? "begin_gx,cam_mode,pose,ctrl,kbd,walk,geom,eff,ptex,obj,ui,clip,info,ready,render_tick x3" : "MISMATCH");
    CHECK(g_cam_mode == 9 && g_pose_a.x == 0.0f && g_pose_a.y == 0.0f && g_pose_a.z == 0.0f
          && g_pose_b.x == 0.0f && g_pose_b.y == 0.0f && g_pose_b.z == -100.0f,
          "L7-R3c camera mode 9 + pose {0,0,0}/{0,0,-100} era literals captured through the real run_frame body");
    CHECK(g_begin_gx_calls == 3 && g_ctrl_calls == 3 && g_kbd_calls == 3
          && g_render_tick_calls == 3 && g_walk_calls == 3
          && g_geom == 3 && g_eff == 3 && g_ptex == 3 && g_obj == 3 && g_ui == 3
          && g_clip == 3 && g_info == 3 && main_task.task_flags == 0,
          "L7-R3d every counted leg fired exactly 3x — PLAY-2 spine counted via the shared walk extern + the 7 in-scope sub-pass externs (run/render/overlay legs now REAL, task_flags==0 at every frame boundary (:651 real clear); render_tick still Play-3 territory)");

    /* ============ I1: invariants proven after real execution ============ */
    CHECK(async_file_read_state == 0 && async_file_read_current_request == NULL
          && lobby_download_state_ptr == NULL,
          "L7-I1a process_async_file_reads (ERA BODY, AsyncFileRead.cpp:198) held state 0: no request -> case 0 fall-through, era tail async_file_read_stub() (TGStream.cpp:203 verbatim) ran 3x harmlessly");
    CHECK(controller_reset_requested == 0 && memcard_access_active == 0,
          "L7-I1b poll_reset_request (ERA BODY, ControllerInput.cpp:258) held no-request (ledger U6): disc present short-circuits check_reset_button, ternary reads memcard/reset globals direct");
    CHECK(sound_requests_enabled == 0 && g_voice_calls == 0 && g_fade_calls == 0
          && g_gpbci_calls == 0,
          "L7-I1c update_sound_request_slots (ERA BODY, SoundRequests.cpp:340) guard held: enabled==0 skips the whole slot loop — voice-state/music-fade/player-lookup counters 0");
    CHECK(quest_download_server_ready == 0 && quest_download_in_progress == 0
          && g_ready_pump_calls == 3,
          "L7-I1d is_ready (ERA BODY, TLobby_800F8AA8.cpp:1371) returned 0 x3 — pump seam entered 3x, era disk-present path pumps ZERO render_ticks (render_tick stayed at the 3 frame-tail calls)");
    CHECK(g_adxf_open_calls == 0 && g_adxf_readnw_calls == 0 && g_adxf_getstat_calls == 0
          && g_tmpbuf_use_calls == 0 && g_loadbml_calls == 0,
          "L7-I1e ADXF/tmp_buf/LoadBmlExec counters 0 on the untouched legs (RS What-green: 'counters for pump/ADXF/voice-state stay 0')");
    CHECK(main_phase_state.next == 0 && main_phase_state.result == 0,
          "L7-I1f request_main_phase(0) NEVER fired — never-ready trap guard held through REAL leg bodies (rung4 §5); era slice linked+executed, trap leg provably not taken");

    /* ============ FLIP PROBES: the era bodies EXECUTE (real reads) ============ */
    /* P1 poll: both ternary legs + the memcard gate */
    controller_reset_requested = 3; memcard_access_active = 1;
    CHECK(poll_reset_request() == 0,
          "L7-P1a poll_reset_request ERA TERNARY leg A: memcard busy -> 0 despite controller_reset_requested==3");
    memcard_access_active = 0;
    CHECK(poll_reset_request() == 3,
          "L7-P1b ERA ternary leg B: returns the REAL controller_reset_requested (3) — reads, not a constant");
    controller_reset_requested = 0;
    CHECK(poll_reset_request() == 0, "L7-P1c era-zero restored -> 0 (DVDCheckDisk TRUE short-circuited check_reset_button — its LB-3 fire would have aborted)");

    /* P2 async case 0: era writes 0x100 into the request and calls ADXF_Open with the ERA name arg */
    AsyncFileReadRequest probe; memset(&probe, 0, sizeof probe);
    strcpy(probe.name, "L7PROBE"); probe.count = 1;
    async_file_read_current_request = &probe;
    process_async_file_reads();
    CHECK(probe.flags == 0x100 && g_adxf_open_calls == 1 && g_adxf_last_name == probe.name
          && lobby_download_state_ptr == NULL && async_file_read_state == 0,
          "L7-P2a ERA case-0 body: request->flags |= 0x100 stamped by the era store, ADXF_Open(name) called with the request's own name buffer, h==0 -> state stays 0 (zero-pixel read, RS R5)");
    /* P2b async state 4: era dispatches LoadBmlExec(mode,name) — arg capture proves member reads */
    probe.mode = (BmlResource *)0x1234;
    async_file_read_state = 4;
    process_async_file_reads();
    CHECK(g_loadbml_calls == 1 && g_loadbml_last_name == probe.name
          && g_loadbml_last_bml == probe.mode && async_file_read_state == 4,
          "L7-P2b ERA state-4 leg: LoadBmlExec(->mode,->name) called with the request's own fields; counter returns 0 -> era correctly keeps state 4 (no pop)");
    async_file_read_current_request = NULL; async_file_read_state = 0;
    process_async_file_reads();
    CHECK(g_adxf_open_calls == 1 && g_loadbml_calls == 1,
          "L7-P2c era-zero again: case 0 with NULL request changes nothing (ADXF/LoadBml counters frozen at probe values)");

    /* P3 sound: enabled==1 enters the era body — listener zeroing + flags write are ERA stores */
    sound_listener_position[0] = 1.0f; sound_listener_position[1] = 2.0f;
    sound_listener_position[2] = 3.0f; sound_pitch_requests[0].flags = 0x400;
    local_client_id = 7;
    sound_requests_enabled = 1;
    update_sound_request_slots();
    CHECK(g_gpbci_calls == 1 && g_gpbci_last_client == 7
          && sound_listener_position[0] == 0.0f && sound_listener_position[1] == 0.0f
          && sound_listener_position[2] == 0.0f && g_voice_calls == 0x28
          && sound_pitch_requests[0].flags == 0 && g_fade_calls == 1,
          "L7-P3 ERA guard opens on the flip: get_player_by_client_id(local_client_id=7) via the era call, camera-NULL else-leg zeroed sound_listener_position (era stores), 40-slot voice loop ran (counters, no fail-loud), sound_pitch_requests[0].flags &= ~0x400 written by the era body, update_music_fade reached");
    sound_requests_enabled = 0; local_client_id = 0;

    /* P4 is_ready: all three era return legs + pump always entered */
    quest_download_server_ready = 1;
    CHECK(TQuestVmDownLoadServer::is_ready() == 1 && g_ready_pump_calls == 4,
          "L7-P4a ERA ready leg: server_ready==1 -> returns 1 (pump entered 4th time — first statement of the era body)");
    quest_download_in_progress = 1;
    CHECK(TQuestVmDownLoadServer::is_ready() == 0, "L7-P4b ERA in-progress gate: returns 0 despite ready==1");
    quest_download_in_progress = 0; quest_download_server_ready = 0;
    memcard_access_active = 1; quest_download_server_ready = 1;
    CHECK(TQuestVmDownLoadServer::is_ready() == 0, "L7-P4c ERA memcard gate first: memcard busy masks ready==1");
    memcard_access_active = 0; quest_download_server_ready = 0;
    CHECK(TQuestVmDownLoadServer::is_ready() == 0, "L7-P4d era-zero restored -> never-ready (INVARIANT leg)");

    /* ============ I2f/I2g: post-probe frames — restoration is era-clean ============
     * (PLAY-1 receipts spelled these L7-I2a/L7-I2b; relabeled to hand the
     * I2a..e namespace to the spec §1.5 flag-spine invariant set below.
     * Assertions untouched.) */
    int seq_before = g_call_n, ready_before = g_ready_pump_calls;
    logo.run_frame();
    CHECK(g_call_n == seq_before + FRAME_EVENTS && g_ready_pump_calls == ready_before + 1,
          "L7-I2f 4th frame after all probes: order log + pump continue at the same cadence");
    CHECK(async_file_read_state == 0 && controller_reset_requested == 0
          && sound_requests_enabled == 0 && quest_download_server_ready == 0
          && main_phase_state.next == 0 && main_phase_state.result == 0,
          "L7-I2g ALL four RS invariants re-read zero after the flips — the frame machine is back on the no-request/never-ready/audio-dark/async-idle spine, no phase fired");

    /* ============ PLAY-2 flag-spine flip probes (spec §1.5 P5-P9) ============ */
    /* P5 macro fire: REAL PAUSE_OR_RESUME_CHILD through the REAL carrier methods,
       two TMainTask dummies linked era-style under main_task.m_down. */
    TMainTask n0, n1; memset(&n0, 0, sizeof n0); memset(&n1, 0, sizeof n1);
    main_task.m_down = &n0; n0.m_next = &n1; n1.m_next = NULL;
    n0.m_flags = 0; n1.m_flags = PAUSE_UPDATE;        /* n1 'was paused': else leg must clear */
    update_flags = 0x1;                               /* old stays 0 — gate opens */
    main_task.run_task();                             /* REAL L2 slice :283 */
    CHECK((n0.m_flags & PAUSE_UPDATE) != 0 && (n1.m_flags & PAUSE_UPDATE) == 0
          && old_update_flags == update_flags,
          "L7-P5a macro fires for real: bit 1 -> n0.pause_update() (PAUSE_UPDATE set), else leg -> n1.resume_update() (bit cleared); macro tail store old_update_flags==update_flags (:510-era grammar, spec §3 D1)");
    n0.m_flags = 0; n1.m_flags = PAUSE_UPDATE;        /* reset fire states */
    main_task.run_task();                             /* flags == old — gate must stay shut */
    CHECK(n0.m_flags == 0 && n1.m_flags == PAUSE_UPDATE,
          "L7-P5b second call, flags unchanged -> NO method fires (gate flags != old_flags is the REAL load pair, not a stub)");
    update_flags = 0; old_update_flags = 0;           /* era-zero restore */
    main_task.m_down = NULL; main_task.task_flags = 0;
    render_pass = 1;

    /* P6 L2 spine: run_task drives flags + render_pass + the walk for real */
    int walk_before = g_walk_calls;
    main_task.run_task();
    CHECK(g_walk_calls == walk_before + 1 && (g_walk_flags & MAIN_TASK_UPDATING) != 0
          && g_walk_root == WALK_MAIN && main_task.task_flags == MAIN_TASK_UPDATED
          && render_pass == 0,
          "L7-P6 L2 spine REAL (:283-290): walk extern saw task_flags & MAIN_TASK_UPDATING LIVE at :287, root==MAIN; after: task_flags==MAIN_TASK_UPDATED (0x080), UPDATING cleared, render_pass==0 (:285)");

    /* P7 R3 spine: the render_pass alternation is the REAL body's (:512-:520) */
    g_pass_n = 0; main_task.task_flags = MAIN_TASK_UPDATED;
    main_task.render();
    CHECK(g_pass_n == 5 && g_pass_trace[0] == 0 && g_pass_trace[1] == 1
          && g_pass_trace[2] == 0 && g_pass_trace[3] == 1 && g_pass_trace[4] == 0,
          "L7-P7a render REAL (:509-525): g_pass_trace == [0,1,0,1,0] at geom/eff/ptex/obj/ui entries — every render_pass store between :512-:520 executed (RS What-green sequence, era .data init 1 untouched by the spine)");
    CHECK(main_task.task_flags == (u32)(MAIN_TASK_UPDATED | MAIN_TASK_RENDERED)
          && g_flags_at_subpass == (u32)(MAIN_TASK_UPDATED | MAIN_TASK_RENDERING),
          "L7-P7b RENDERING live across the sub-pass window (:511..:522 — ui entry saw UPDATED|RENDERING, not yet RENDERED), cleared on exit (:523), MAIN_TASK_RENDERED set by the REAL :524 store");
    g_pass_n = 0;

    /* P8 R4 guard honesty: the :645 load is real in BOTH directions */
    int clip_before = g_clip, info_before = g_info;
    main_task.task_flags = 0;
    main_task.render_screen_overlay();
    CHECK(g_clip == clip_before && g_info == info_before,
          "L7-P8a task_flags==0 -> overlay fires NOTHING: guard :645 false through the REAL load (stub-honesty is dead)");
    main_task.task_flags = MAIN_TASK_UPDATED | MAIN_TASK_RENDERED;
    main_task.render_screen_overlay();
    CHECK(g_clip == clip_before + 1 && g_info == info_before + 1 && main_task.task_flags == 0,
          "L7-P8b UPDATED|RENDERED -> clip+info fire once each; debug_render/debug_render_overlay/render_after_scene execute their REAL era-empty bodies (unobservable by design, L7 §3 argument); task_flags==0 after (:651 real clear)");

    /* P9 full-frame chain through the run_frame carrier from era-zero */
    main_task.task_flags = 0; render_pass = 1; g_pass_n = 0;
    seq_before = g_call_n; walk_before = g_walk_calls;
    clip_before = g_clip; info_before = g_info;
    logo.run_frame();
    int spine_ok = g_call_n == seq_before + FRAME_EVENTS;
    static const int kSpine[9] = { LC7_WALK_MAIN, LC7_GEOM, LC7_EFF, LC7_PTEX, LC7_OBJ,
                                   LC7_UI, LC7_CLIP, LC7_INFO, LC7_READY };
    for (int i = 0; spine_ok && i < 9; i++)
        spine_ok = g_call_seq[seq_before + 5 + i] == kSpine[i];
    CHECK((g_flags_at_subpass & MAIN_TASK_RENDERED) != 0 && main_task.task_flags == 0
          && spine_ok && g_walk_calls == walk_before + 1
          && g_clip == clip_before + 1 && g_info == info_before + 1,
          "L7-P9 run_frame from era-zero: flag-honesty end-to-end — the g_info extern observed task_flags & MAIN_TASK_RENDERED != 0 (guard opened because of :524's REAL store), per-frame sub-pass order == :513-522 text order (walk,geom,eff,ptex,obj,ui,clip,info before READY), end-of-frame task_flags==0");

    /* ============ PLAY-2 read-back invariants (spec §1.5 L7-I2a..e) ============ */
    CHECK(g_pass_n == 7 && g_pass_trace[0] == 0 && g_pass_trace[1] == 1
          && g_pass_trace[2] == 0 && g_pass_trace[3] == 1 && g_pass_trace[4] == 0
          && g_pass_trace[5] == 0 && g_pass_trace[6] == 0,
          "L7-I2a per-frame render_pass trace == {0}(:285) ∪ <0,1,0,1,0>(:512-:520) ∪ <0,0>(overlay clip/info ride the trailing 0 — the 'next frame :285' leg), read through the full P9 frame");
    main_task.task_flags = 0x800;                     /* bit OUTSIDE main_task_flags (~0x3FF) */
    logo.run_frame();
    CHECK(main_task.task_flags == 0x800,
          "L7-I2b flag bits outside the enum universe never move: 0x800 seeded, full frame run — RENDERED is set ONLY by :524, consumed ONLY at :645, cleared ONLY by :651, and no real store touches ~0x3FF");
    main_task.task_flags = 0;
    CHECK(g_walk_calls == walk_before + 2 && g_walk_root == WALK_MAIN,
          "L7-I2c g_walk_calls == 1 per run_frame (2 since P9: P9 + this I2b frame) and root == MAIN — until PLAY-3 adds the two render_tick roots");
    CHECK(update_flags == 0 && old_update_flags == 0 && render_flags == 0 && old_render_flags == 0,
          "L7-I2d .bss flag quartet back to zero after P5-style probes (macro tail stores old=flags; probes restored flags=0 — era-default pairs held, RS 'all zero on host = era default' for the quartet)");
    CHECK(async_file_read_state == 0 && controller_reset_requested == 0
          && sound_requests_enabled == 0 && quest_download_server_ready == 0
          && main_phase_state.next == 0 && main_phase_state.result == 0
          && g_adxf_open_calls == 1 && g_loadbml_calls == 1 && g_voice_calls == 0x28,
          "L7-I2e ALL PLAY-1 invariants re-read zero after every PLAY-2 frame — the flag spine changes nothing behind the promoted five (ADXF/LoadBml/voice counters frozen at their P2/P3 probe values across 3 more frames)");

    printf("\n== P2-L7 ladder7 (rung-5 PLAY-1 + PLAY-2) ==\n");
    printf("  five promoted legs + the main_task flag spine EXECUTE era bodies verbatim (begin_frame/poll/async/sound/is_ready + run_task :283 / render :509 / overlay :644 + render_after_scene :505 / debug_render_overlay :533 / TObject::debug_render :174 era-empty REAL)\n");
    printf("  L7-R3 order == TLogo.cpp:95-114 (15-event PLAY-2 spine); render_pass {0}+<0,1,0,1,0> + RENDERED set@:524/consume@:645/clear@:651 by REAL stores (spec P5-P9 + I2a..e)\n");
    printf("== P2-L7 ladder7: %d/%d PASS ==\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
