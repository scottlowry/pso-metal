/* P2-L7 rung-5 PLAY-1 — host zero-def globals + seams behind the five
 * promoted legs (contract docs/p2-rung5-seams-1.md Play 1; deviation roster
 * L7-1..L7-3 is spelled out in include/ladder7_run_prelude.h).
 *
 * Every era .sbss/.data global the sliced bodies touch is DEFINED HERE with
 * era-true zero storage (no initializer = .bss, exactly the era regime); the
 * harness's INVARIANT checks are real reads of these objects, and the flip
 * probes write them the way era code would, so the era legs' own reads/writes
 * are what the CHECKs observe.
 */
#include "ladder7_run_prelude.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- era .sbss globals (zero-def; homes cited per prelude) ---- */
s32 sound_requests_enabled;              /* SoundRequests.cpp:51 (audio-dark per boot-spine §4 + ledger row 35) */
s8  lbl_805D6279;                        /* SoundRequests.cpp:82 */
f32 sound_listener_position[4];          /* SoundRequests.cpp:202 */
SetSoundPitchRequestSndEnt sound_pitch_requests[40]; /* SoundRequests.cpp:200 */
s32 local_client_id;                     /* client_id.h (era net off on host) */
TCameraCtrl *camera_ctrl_instance;       /* TCameraCtrl.h:209 — era NULL pre-camera-spawn */

AsyncFileReadRequest *async_file_read_current_request;   /* AsyncFileRead.cpp:189-197 block — */
AsyncFileReadRequest *lobby_stack_entry_list_tail;       /* all nine era defs, host zero-def  */
s32 lobby_list_entry_count;
s32 async_file_read_state;
u8 *async_file_read_sector_buffer;
AdxfHandle *lobby_download_state_ptr;
s32 lobby_list_next_id;
THeap *game_heap;
u8 *async_file_read_texture_buffer;

u8 controller_reset_requested;           /* ControllerInput.cpp:24 */
s32 memcard_access_active;               /* SystemBootInit.cpp:87 */
s32 quest_download_in_progress;          /* TQuestVmDownLoadServer.cpp:50 */
s32 quest_download_server_ready;         /* TLobby_800F8AA8.cpp:91 */
u8 lobby_wait_ready_flag;                /* TLobby_800F8AA8.cpp:93 */

MainPhaseState main_phase_state;         /* MainPhaseState.cpp era global (C6/C7 regime) */
TMainTask main_task;                     /* era TObject.cpp home global */

/* ---- seam state (L7-R3 order log + counted legs) ---- */
const char *const l7_call_names[] = {
    "", "begin_gx_frame", "activate_camera_mode", "set_active_camera_pose",
    "controller_stuff", "update_keyboard_input_frame", "main_task.run_task",
    "main_task.render", "main_task.render_screen_overlay",
    "process_async_file_reads", "update_sound_request_slots", "poll_reset_request",
    "TQuestVmDownLoadServer::is_ready", "render_tick",
};
int g_call_seq[64];
int g_call_n;
int g_begin_gx_calls, g_ctrl_calls, g_kbd_calls;
int g_mt_run_calls, g_mt_render_calls, g_mt_overlay_calls, g_render_tick_calls;
int g_cam_mode;
Vec g_pose_a, g_pose_b;
int g_adxf_open_calls, g_adxf_readnw_calls, g_adxf_getstat_calls;
const void *g_adxf_last_name;
int g_tmpbuf_use_calls;
int g_voice_calls, g_fade_calls, g_gpbci_calls;
s32 g_gpbci_last_client;
int g_loadbml_calls;
BmlResource *g_loadbml_last_bml;
const char *g_loadbml_last_name;
int g_ready_pump_calls;

/* ---- counted seams around the promoted five (L7-2, LOGO-2 shape) ---- */
void begin_gx_frame(void) { l7_note(LC7_BEGIN_GX); g_begin_gx_calls++; }   /* real slice: gx tree LOGO-1 */
void activate_camera_mode(s32 mode) { g_cam_mode = mode; l7_note(LC7_ACT_CAM_MODE); }
void set_active_camera_pose(Vec *a, Vec *b) { g_pose_a = *a; g_pose_b = *b; l7_note(LC7_SET_POSE); }
void controller_stuff(void) { l7_note(LC7_CTRL_STUFF); g_ctrl_calls++; }
void update_keyboard_input_frame(void) { l7_note(LC7_KBD_FRAME); g_kbd_calls++; }
void TMainTask::run_task() { l7_note(LC7_MT_RUN); g_mt_run_calls++; }              /* Play 2 */
void TMainTask::render() { l7_note(LC7_MT_RENDER); g_mt_render_calls++; }          /* Play 2 */
void TMainTask::render_screen_overlay() { l7_note(LC7_MT_OVERLAY); g_mt_overlay_calls++; } /* Play 2 */
void render_tick(void) { l7_note(LC7_RENDER_TICK); g_render_tick_calls++; }        /* Play 3 */

/* LB-2 pump-count seam (RS §3: wait_ready_loop JOINS this roster, no new
 * mechanism). Era-true disk-present path: run_disk_error_check()==1 returns
 * on the FIRST test with ZERO render_tick pumps (RS R8 body verification) —
 * the fake_disc DVD shim reports present, so counting the entry and not
 * pumping is the era-faithful leg. lobby_wait_ready_flag stays 0 exactly as
 * that path leaves it. The era-true is_ready body calls THIS on its first
 * statement, so l7_note here marks the R8 slot at the era text position —
 * strictly stronger than the rung-4 stub note (it logs the inner call, not
 * the leg). The other four promoted legs leave no order-log event by design
 * (a verbatim slice cannot be logged from inside); their execution is proven
 * by the flip-probe real reads below (flags 0x100 stamp, listener zeroing,
 * poll ternary, empty begin_frame slot bracketed at text position). */
void TLobby::wait_ready_loop() { l7_note(LC7_READY); g_ready_pump_calls++; }

/* ---- counted stand-ins (L7-3 first half; RS: "counters for pump/ADXF/
 * voice-state stay 0") — era-default returns, args captured for probes ---- */
extern "C" AdxfHandle *ADXF_Open(const void *name, s32 mode) {
    (void)mode; g_adxf_open_calls++; g_adxf_last_name = name; return 0;   /* handle pool empty on host */
}
extern "C" s32 ADXF_ReadNw(AdxfHandle *file, s32 sectors, void *destination) {
    (void)file; (void)sectors; (void)destination; g_adxf_readnw_calls++; return 0;
}
extern "C" s32 ADXF_GetStat(AdxfHandle *file) { (void)file; g_adxf_getstat_calls++; return 0; }
u8 *tmp_buf_use_req(const char *name) { (void)name; g_tmpbuf_use_calls++; return 0; } /* LB-2 stand-in (RS R5) */
s32 get_sound_voice_state(s32 voice) { (void)voice; g_voice_calls++; return 0; }      /* no voice ever active (audio-dark) */
void update_music_fade(void) { g_fade_calls++; }
TObjPlayer *get_player_by_client_id(s32 client) { g_gpbci_calls++; g_gpbci_last_client = client; return 0; }
s32 LoadBmlExec(BmlResource *bml, const char *name) {
    g_loadbml_calls++; g_loadbml_last_bml = bml; g_loadbml_last_name = name; return 0; /* exec-not-done leg */
}
Vec *VecCopy(Vec *dst, const Vec *src) { *dst = *src; return dst; }        /* era vector.c: pure copy */

/* ---- LB-3 fail-loud stand-ins (L7-3 second half): the era-true zero state
 * can never select these legs; any fire is a contract violation (rung4 §5
 * never-ready/no-request traps). reset_system is the card-named pin. ---- */
static void l7_fatal(const char *what) {
    fprintf(stderr, "L7 FATAL: %s called — era-zero invariant broken (LB-3)\n", what);
    exit(9);
}
void reset_system(void) { l7_fatal("reset_system"); }
s32 check_reset_button() { l7_fatal("check_reset_button"); return 0; }    /* unreachable: DVDCheckDisk()==TRUE short-circuits */
void switch_to_fallback_main_phase(void) { l7_fatal("switch_to_fallback_main_phase"); }
void pop_and_close() { l7_fatal("pop_and_close"); }
void notify_and_close() { l7_fatal("notify_and_close"); }
void destroy_instance() { l7_fatal("destroy_instance"); }
s32 tmp_buf_release(const char *name) { (void)name; l7_fatal("tmp_buf_release"); return 0; }
void move_words(void *destination, const void *source, s32 count) {
    (void)destination; (void)source; (void)count; l7_fatal("move_words");
}
void copy_u16_array(u16 *dst, u16 *src, s32 n) {
    (void)dst; (void)src; (void)n; l7_fatal("copy_u16_array");
}
s32 load_gvm_texture_archive(void *archive, NJS_TEXLIST *list) {
    (void)archive; (void)list; l7_fatal("load_gvm_texture_archive"); return 0;
}
s32 check_sound_pitch_request_range(s32 slot, s32 flag) {
    (void)slot; (void)flag; l7_fatal("check_sound_pitch_request_range"); return 0;
}
void stop_sound_request(s32 idx) { (void)idx; l7_fatal("stop_sound_request"); }
