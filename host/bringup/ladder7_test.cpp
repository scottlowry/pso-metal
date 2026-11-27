/* P2-L7 boot ladder rung 5 PLAY-1 — the logo main-phase machine runs its five
 * promoted legs as ERA BODIES (contract docs/p2-rung5-seams-1.md Play 1;
 * seam ledger rows L1/R5/R6/R7/R8). The run_frame carrier + the five legs
 * arrive VERBATIM from the read-only decomp via gen_ladder7_runframe.py;
 * everything still behind lane J / Play 2 / Play 3 stays a counted seam
 * (L7-2) and everything the era-true zero state can never select is an
 * LB-3 fail-loud stand-in (L7-3).
 *
 * GREEN MEANS (RS "What-green" row, verified here by REAL reads — not stub
 * honesty): the era bodies of begin_frame/poll_reset_request/
 * process_async_file_reads/update_sound_request_slots/is_ready executed,
 * controller_reset_requested==0 AND async_file_read_state==0 AND
 * sound_requests_enabled==0 AND quest_download_server_ready==0 held poll==0
 * and never-ready, no fallback fire, no request_main_phase(0) (trap legs),
 * pump/ADXF/voice-state counters stay 0 on the untouched legs, and the
 * L7-R3 order == TLogo.cpp:95-114 text order. The flip probes (L7-P1..P4)
 * write the era globals the way era code would and read the era bodies' own
 * stores/branches back — the legs provably EXECUTE, not merely link.
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
    LC7_MT_RUN, LC7_MT_RENDER, LC7_MT_OVERLAY, LC7_READY, LC7_RENDER_TICK,
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

    /* ============ THE RUNG: era run_frame x3 ============ */
    TLogo logo;
    for (int f = 0; f < 3; f++) logo.run_frame();   /* TLogo.cpp:94 verbatim */

    /* ============ L7-R3: call order == era text order ============ */
    CHECK(g_call_n == 3 * FRAME_EVENTS,
          "L7-R3a order log holds 3 frames x %d events (MT_BEGIN/ASYNC/SOUND/POLL slots era-occupied, no stub notes; LC7_READY fires INSIDE the era is_ready chain at wait_ready_loop)", FRAME_EVENTS);
    int order_ok = g_call_n == 3 * FRAME_EVENTS;
    for (int i = 0; order_ok && i < g_call_n; i++)
        order_ok = g_call_seq[i] == kFrameOrder[i % FRAME_EVENTS];
    CHECK(order_ok, "L7-R3b sequence == TLogo.cpp:95-114 text order: %s (LC7_READY bracket proves the ASYNC+SOUND+POLL slots were traversed before it)",
          order_ok ? "begin_gx,cam_mode,pose,ctrl,kbd,run,render,overlay,ready,render_tick x3" : "MISMATCH");
    CHECK(g_cam_mode == 9 && g_pose_a.x == 0.0f && g_pose_a.y == 0.0f && g_pose_a.z == 0.0f
          && g_pose_b.x == 0.0f && g_pose_b.y == 0.0f && g_pose_b.z == -100.0f,
          "L7-R3c camera mode 9 + pose {0,0,0}/{0,0,-100} era literals captured through the real run_frame body");
    CHECK(g_begin_gx_calls == 3 && g_ctrl_calls == 3 && g_kbd_calls == 3
          && g_mt_run_calls == 3 && g_mt_render_calls == 3 && g_mt_overlay_calls == 3
          && g_render_tick_calls == 3,
          "L7-R3d every counted leg (LOGO-2 seams, Play 2/3 territory) fired exactly 3x — no leg skipped or doubled");

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

    /* ============ I2: post-probe frames — restoration is era-clean ============ */
    int seq_before = g_call_n, ready_before = g_ready_pump_calls;
    logo.run_frame();
    CHECK(g_call_n == seq_before + FRAME_EVENTS && g_ready_pump_calls == ready_before + 1,
          "L7-I2a 4th frame after all probes: order log + pump continue at the same cadence");
    CHECK(async_file_read_state == 0 && controller_reset_requested == 0
          && sound_requests_enabled == 0 && quest_download_server_ready == 0
          && main_phase_state.next == 0 && main_phase_state.result == 0,
          "L7-I2b ALL four RS invariants re-read zero after the flips — the frame machine is back on the no-request/never-ready/audio-dark/async-idle spine, no phase fired");

    printf("\n== P2-L7 ladder7 (rung-5 PLAY-1) ==\n");
    printf("  five promoted legs EXECUTE era bodies verbatim (TObject.cpp:640 / ControllerInput.cpp:258 / AsyncFileRead.cpp:198 / SoundRequests.cpp:340 / TLobby_800F8AA8.cpp:1371)\n");
    printf("  L7-R3 order == TLogo.cpp:95-114; invariants by REAL reads; flip probes proved the bodies, not just the links\n");
    printf("== P2-L7 ladder7: %d/%d PASS ==\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
