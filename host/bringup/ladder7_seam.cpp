/* P2-L7 rung-5 PLAY-1 + PLAY-2 — host zero-def globals + seams behind the
 * promoted legs (contract docs/p2-rung5-seams-1.md Play 1; Play-2 spec
 * docs/p2-ladder7-play23.md §1.3; deviation roster L7-1..L7-3 is spelled out
 * in include/ladder7_run_prelude.h).
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
    "controller_stuff", "update_keyboard_input_frame",
    "process_async_file_reads", "update_sound_request_slots", "poll_reset_request",
    "TQuestVmDownLoadServer::is_ready", "render_tick",
    "run_tasks:main", "run_tasks:kbd", "run_tasks:eor",
    "render_geometry", "render_effects", "render_particle_effects",
    "render_objects", "render_ui", "render_clipout_and_fade",
    "render_info_icon_overlay",
    "rt1:kbd_run_tasks", "rt2:begin_effect_render", "rt3:zmode_always",
    "rt4:restore_full_screen_view", "rt5:set_cull_none", "rt6:debug_text",
    "rt7:zmode_lequal", "rt8:viewport_640x480", "rt9:set_cull_none2",
    "rt10:end_effect_render", "rt11:copy_efb", "rt12:present_once",
    "rt13:eor_run_tasks",
};
int g_call_seq[512];
int g_call_n;
int g_begin_gx_calls, g_ctrl_calls, g_kbd_calls;
int g_render_tick_calls;                  /* PLAY-3: incremented at render_tick's FIRST statement (leg 1 :492 epoch, era-faithful tick count) */
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

/* ---- PLAY-2 flag-spine globals (era homes verified against the pinned tree
 * this lane; spec docs/p2-ladder7-play23.md §1.3) ---- */
u32 render_pass = 1;      /* .data — era INIT is 1 (TObject.cpp:25). ZERO-INIT IS DRIFT (§3 D7) */
u32 update_flags;         /* TObject.cpp:53 (.bss zero-def) */
u32 old_update_flags;     /* :54 */
u32 render_flags;         /* :55 */
u32 old_render_flags;     /* :56 */
TObject end_of_render_root;      /* TObject.cpp:35 — PLAY-3 root, def here once (m_down=NULL, era-empty walk) */
TObject keyboard_task_object;    /* TObject.cpp:37 — same */

/* ---- PLAY-2 seam state (probes P5-P9 + invariants I2a..e REAL-read these) ---- */
int g_walk_calls;
int g_walk_cnt[3];
int g_walk_root;
u32 g_walk_flags;
int g_geom, g_eff, g_ptex, g_obj, g_ui, g_clip, g_info;
u8 g_pass_trace[16];
int g_pass_n;
u32 g_flags_at_subpass;

/* ---- PLAY-3 era zero-defs (spec §2.3; homes verified against the pinned tree
 * this lane) + seam state (probes P10-P12 + invariants I3a..d REAL-read) ---- */
s32 lbl_805D3584 = 1;            /* .data INIT 1 — TLobby_801E2F94.cpp:108 (spec §3 D7) */
s8  scene_fog_enable_flag;       /* .bss zero — SceneFog.cpp:13 */
void *video_current_framebuffer; /* .bss zero — gjSystem.c:23 */
int g_fog_reads, g_clearshadow_calls, g_soundframe_calls, g_rfs_calls;
int g_dbgtext_calls; u32 g_dbgtext_arg;
int g_zmode_calls; int g_zmode_trace[3][3];
int g_cull_calls, g_cull_mode;
int g_vp_calls; float g_vp_l, g_vp_t, g_vp_w, g_vp_h, g_vp_n, g_vp_f;
int g_colorupdate_calls;
void *g_copydisp_fb; int g_copydisp_clear, g_copydisp_calls;
int g_drawdone_calls, g_present_calls;
/* per-render_tick capture epochs — render_tick's FIRST statement (leg 1,
 * :492 -> TObjPlayer::clear_shadow_pass_request, the leg's first inner extern)
 * opens each tick, so the in-tick call ordinal maps legs 3/7/11 (zmode),
 * 5/9 (cull) and 2/10 (apply-fog) to their LC7_RT* notes in era order. */
static int g_tick_zmode, g_tick_cull, g_tick_fog;

/* ---- counted seams around the promoted five (L7-2, LOGO-2 shape) ---- */
void begin_gx_frame(void) { l7_note(LC7_BEGIN_GX); g_begin_gx_calls++; }   /* real slice: gx tree LOGO-1 */
void activate_camera_mode(s32 mode) { g_cam_mode = mode; l7_note(LC7_ACT_CAM_MODE); }
void set_active_camera_pose(Vec *a, Vec *b) { g_pose_a = *a; g_pose_b = *b; l7_note(LC7_SET_POSE); }
void controller_stuff(void) { l7_note(LC7_CTRL_STUFF); g_ctrl_calls++; }
void update_keyboard_input_frame(void) { l7_note(LC7_KBD_FRAME); g_kbd_calls++; }
/* PLAY-3: the render_tick counter body LEAVES — the seam is a REAL verbatim
 * slice now (TLobby_801E2F94.cpp:491-505, glue); its legs count/capture below. */

/* ---- counted tree-walk stand-in (LB-2 roster joins: RS §3 — reuse, no new
 * mechanism; era body TObject::run_tasks @TObject.cpp:116 walks child
 * run_task/render bodies, all lane-J homes -> the counter is the
 * rung-5-faithful backing). PLAY-2's only caller is main_task.run_task
 * (:286), so the LIVE flag capture reads main_task.task_flags — the spec's
 * bare `task_flags` would name a TMainTask member from a TObject method
 * (spell-out deviation, same observable). ---- */
void TObject::run_tasks() {
    g_walk_calls++;
    g_walk_root = (this == (TObject *)&main_task)  ? WALK_MAIN
              : (this == &keyboard_task_object)    ? WALK_KBD : WALK_EOR;
    g_walk_flags = main_task.task_flags;            /* capture the caller's LIVE flags */
    g_walk_cnt[g_walk_root]++;                      /* PLAY-3 I3c per-root tally */
    l7_note(LC7_WALK_MAIN + (g_walk_root - WALK_MAIN));
    if (g_walk_root == WALK_EOR) l7_note(LC7_RT13); /* leg 13 first-inner extern (spec §2.3 note rule) */
}
/* ---- counted pixel sub-passes (RS Play 2: the 8 stay counters; pixels keep
 * riding LOGO-3's single present). Each records the LIVE render_pass/
 * task_flags; homes: geometry :399, effects :432, particle_effects :449,
 * objects :477, ui :460, clipout_and_fade :292 (body :292-313, pixel-chain
 * callee set — count, do NOT slice); render_info_icon_overlay is a FREE
 * function @ TLobby_803382E4.cpp:145 (lane-J `(d)` home; RS R4 row). ---- */
static void l7_subpass(int ev, int *ctr) { l7_note(ev); (*ctr)++;
    g_pass_trace[g_pass_n < 16 ? g_pass_n++ : 15] = (u8)render_pass;
    g_flags_at_subpass = main_task.task_flags; }
void TMainTask::render_geometry()        { l7_subpass(LC7_GEOM, &g_geom); }
void TMainTask::render_effects()         { l7_subpass(LC7_EFF,  &g_eff);  }
void TMainTask::render_particle_effects(){ l7_subpass(LC7_PTEX, &g_ptex); }
void TMainTask::render_objects()         { l7_subpass(LC7_OBJ,  &g_obj);  }
void TMainTask::render_ui()              { l7_subpass(LC7_UI,   &g_ui);   }
void TMainTask::render_clipout_and_fade(){ l7_subpass(LC7_CLIP, &g_clip); }
void render_info_icon_overlay(void)      { l7_subpass(LC7_INFO, &g_info); }

/* ---- PLAY-3 render_tick leg backings (spec §2.2 roster; era calls reach these
 * from the VERBATIM slices in the glue — render_tick :491-505,
 * TKeyBoardControl::run_tasks :100-107, run_end_of_render_tasks :113-115,
 * begin/end_effect_render EffectRender.cpp:10/:12, set_cull_none :39,
 * copy_efb :52-57, present_frame :59-61). ---- */
/* leg 1 (:492) — first-inner counted extern; ALSO opens the tick capture epoch. */
void TObjPlayer::clear_shadow_pass_request() {
    l7_note(LC7_RT1); g_clearshadow_calls++; g_render_tick_calls++;   /* :492 is render_tick's first statement — tick count rides the epoch (receipt note) */
    g_tick_zmode = g_tick_cull = g_tick_fog = 0;
}
void update_global_sound_frame(void) { g_soundframe_calls++; }        /* leg 1 — counted (musyx chain, SoundPlayback.cpp:232) */
/* leg 10 inner (#2) / leg 2 inner (#1): (d) pixel chain — counted, reads the REAL
 * fog flag the enable/disable slices just stored (P10/I3d). RT2/RT10 notes. */
void apply_scene_fog(void) { int i = g_tick_fog++; g_fog_reads++;
    l7_note(i == 0 ? LC7_RT2 : LC7_RT10); }
void TCameraCtrl::restore_full_screen_view() { l7_note(LC7_RT4); g_rfs_calls++; }  /* leg 4 (d) — era TCameraCtrl.cpp:1148-1156 statics, lane J; camera_ctrl_instance era NULL (L7-1) */
void render_debug_text(u32 enabled) { l7_note(LC7_RT6); g_dbgtext_calls++; g_dbgtext_arg = enabled; }  /* leg 6 (d) — era DebugFont.cpp:140 text chain; arg = lbl_805D3584 (era INIT 1, §3 D7) */
/* GX capture seams (legs 3,5,7,8,9,11-inner): args logged, no state, no hardware.
 * In-tick ordinals map to era text positions: zmode #1=:494 #2=:498 #3=copy_efb :53;
 * cull #1=:496 #2=:500. */
void GXSetZMode(s32 test, s32 comp, s32 mask) {
    int i = g_tick_zmode++; g_zmode_calls++;
    if (i < 3) { g_zmode_trace[i][0] = test; g_zmode_trace[i][1] = comp; g_zmode_trace[i][2] = mask; }
    l7_note(i == 0 ? LC7_RT3 : (i == 1 ? LC7_RT7 : LC7_RT11));
}
void GXSetCullMode(s32 mode) { int i = g_tick_cull++; g_cull_calls++; g_cull_mode = mode;
    l7_note(i == 0 ? LC7_RT5 : LC7_RT9); }
void GXSetViewport(float left, float top, float width, float height, float near, float far) {
    l7_note(LC7_RT8); g_vp_calls++;
    g_vp_l = left; g_vp_t = top; g_vp_w = width; g_vp_h = height; g_vp_n = near; g_vp_f = far;
}
void GXSetColorUpdate(s32 on) { (void)on; g_colorupdate_calls++; }
void GXCopyDisp(void *fb, s32 clear) { g_copydisp_calls++; g_copydisp_fb = fb; g_copydisp_clear = clear; }
void GXDrawDone(void) { g_drawdone_calls++; }
/* leg 12 SINGLE-present seam (:503 -> present_frame_and_wait_vsync): one note +
 * one count per render_tick — a second present must never appear (RS §3, I3b).
 * The gx tree rides the EXISTING LOGO-3 backing (host/gx/dl — sibling seat);
 * bringup never re-mints it (spec §2.2 leg 12). */
void present_frame_and_wait_vsync(void) { l7_note(LC7_RT12); g_present_calls++; }

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
/* PLAY-3 LB-3: the :103 guard inside the REAL TKeyBoardControl::run_tasks slice
 * (SystemBootInit.cpp:103) is era-unreachable with sound_requests_enabled==0;
 * any fire is a contract violation (P12 proves the guard load is real). */
void update_music_sequencer(void) { l7_fatal("update_music_sequencer"); }
