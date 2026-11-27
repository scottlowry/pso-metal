/* P2-L7 rung-5 PLAY-1 — logo main-phase carrier prelude (ladder7).
 *
 * Prepended to the GENERATED slice TU host/bringup/gen_ladder7_runframe.py ->
 * build/bringup/gen/ladder7_runframe_glue.cpp. This rung EXECUTES the five
 * legs docs/p2-rung5-seams-1.md Play 1 names, verbatim from the era homes:
 *   TMainTask::begin_frame            (task/TObject.cpp:640, empty era body, class (a))
 *   poll_reset_request                (system/ControllerInput.cpp:258, class (b))
 *   process_async_file_reads          (system/AsyncFileRead.cpp:198 — home TU is
 *                                      parse-PASS @apply-7, RS row-5 promotion)
 *   update_sound_request_slots        (audio/SoundRequests.cpp:340, (b) guard leg,
 *                                      sound_requests_enabled==0 era-dark)
 *   TQuestVmDownLoadServer::is_ready  (unk/TLobby_800F8AA8.cpp:1371, class (c) —
 *                                      its wait_ready_loop leg rides the LB-2
 *                                      pump-count seam, RS §3: reuse, no new
 *                                      seam mechanism)
 * + the run_frame carrier itself (TLogo.cpp:94) and request_main_phase
 * (SystemBootInit.cpp) — same anchors gen_ladder6_logo.py pins on the gx side.
 *
 * DEVIATION L7-1 (carrier decls): TLogo/TMainTask/TQuestVmDownLoadServer/
 * TLobby/TCameraCtrl arrive as MINIMAL host carriers (D4 keep-the-era-graph
 * off rule; ladder6_tex_prelude LB-1 + ladder6_logo_prelude bare-carrier
 * precedent). The sliced bodies touch ONLY the declared members — verified
 * against the era text this rung: run_frame touches no TLogo member;
 * begin_frame is empty; is_ready is a static with no member state.
 * DEVIATION L7-2 (counter legs stay): the pre-begin_gx legs P1–P4 (lane J)
 * and main_task run_task/render/render_screen_overlay (Play 2 material) are
 * COUNTED seams exactly as LOGO-2 shaped them — rung-5 edits only the seam
 * bodies BEHIND the five promoted legs, never the slices (§3 cross-check).
 * begin_gx_frame/render_tick stay counters on the bringup side — the REAL
 * begin_gx_frame slice + era-present render_tick backings live in the gx
 * tree (LOGO-1/LOGO-3) and are not re-minted here.
 * DEVIATION L7-3 (LB-3 roster): reset_system (card-named), check_reset_button,
 * switch_to_fallback_main_phase, pop_and_close, notify_and_close,
 * destroy_instance, tmp_buf_release, move_words, copy_u16_array,
 * load_gvm_texture_archive = fail-loud stand-ins for legs the era-true zero
 * state can never select. ADXF_Open/ReadNw/GetStat, tmp_buf_use_req (LB-2
 * stand-in per RS R5), get_sound_voice_state, update_music_fade,
 * get_player_by_client_id, LoadBmlExec, VecCopy = COUNTED stand-ins returning
 * safe era-defaults (RS "counters for pump/ADXF/voice-state stay 0").
 */
#ifndef LADDER7_RUN_PRELUDE_H
#define LADDER7_RUN_PRELUDE_H

#include "types.h"
#include <stddef.h>

/* Vec: era dolphin/mtx/GeoTypes.h math type; same guard shape as the gx logo
 * prelude (3xf32; the run_frame body only builds {0,0,0}/{0,0,-100}). */
#ifndef DOLPHIN_MTX_GEOTYPES_H
struct Vec { f32 x, y, z; };
#endif

/* ---- era headers: narrow, assert-safe graphs (RS R5: homes of the
 * promoted legs are host-definable). AsyncFileRead.cpp + its header are in
 * the official post-apply-7 PASS set; the AsyncFileReadRequest* LAYOUT_ASSERT
 * names carry waiver flags in the force-included generated list. ---- */
#include "pso/system/AsyncFileRead.h"   /* request struct + 8 externs + ADXF-adjacent decls */
#include "pso/system/Adxf.h"            /* ADXF_Open/ReadNw/GetStat (extern "C")             */
#include "pso/audio/SoundRequests.h"    /* SetSoundPitchRequestSndEnt + stop_sound_request   */
#include "pso/network/client_id.h"      /* local_client_id (era s32)                          */

/* async_file_read_texture_buffer has no extern in the era header (def site
 * AsyncFileRead.cpp:197) — declared here so the header stays untouched. */
extern u8 *async_file_read_texture_buffer;

/* DVD shim already binds this (host/bringup/dvd_shim.c; same decl as
 * pso_bringup.h:109). fake_disc regime -> DVDCheckDisk()==TRUE, so the era
 * poll leg short-circuits before check_reset_button. */
extern "C" BOOL DVDCheckDisk(void);

/* ---- externs the sliced bodies reference with no safe era header home ---- */
extern SetSoundPitchRequestSndEnt sound_pitch_requests[];  /* era def SoundRequests.cpp:200 [40] */
extern s8 lbl_805D6279;                                     /* SoundRequests.cpp:82               */
extern f32 sound_listener_position[4];                      /* SoundRequests.cpp:202              */
struct TCameraCtrl { Vec pos; };                            /* carrier: body reads ->pos only     */
extern TCameraCtrl *camera_ctrl_instance;                   /* era decl TCameraCtrl.h:209         */
s32 get_sound_voice_state(s32 voice);                       /* SoundControlState.h:86             */
s32 check_sound_pitch_request_range(s32 slot, s32);         /* SoundRequests.cpp:185              */
void update_music_fade(void);                               /* MusicControl.h:22                  */
struct TObjPlayer;
TObjPlayer *get_player_by_client_id(s32 client);            /* PlayerSlots.h:12                   */
Vec *VecCopy(Vec *dst, const Vec *src);                     /* system/vector.h:7                  */
void move_words(void *destination, const void *source, s32 count);  /* system/memory.h:14         */
void move_data(void *dst, const void *src, s32 n);          /* move_data_glue.o (era def bound)   */
void copy_u16_array(u16 *dst, u16 *src, s32 n);             /* TLobby_803DE778.h:7                */
u8 *tmp_buf_use_req(const char *name);                      /* BmlResource.h:92                   */
s32 tmp_buf_release(const char *name);                      /* BmlResource.h:90                   */
s32 load_gvm_texture_archive(void *archive, NJS_TEXLIST *list);  /* GvmTextureArchive.h:9         */
s32 LoadBmlExec(BmlResource *bml, const char *name);        /* network/TLobby.h:43                */
void reset_system(void);                                    /* pso_date.h:14                      */
s32 check_reset_button();                                   /* ControllerInput.cpp:57             */
void switch_to_fallback_main_phase(void);                   /* run_frame trap leg                 */

/* era tail callee of process_async_file_reads — DEFINED VERBATIM in the glue
 * below (exact line TGStream.cpp:203, empty era body); declared here because
 * the async slice precedes it in emission order. */
void async_file_read_stub(void);

/* ---- zero-def host globals (era .sbss regime: definitions in
 * ladder7_seam.cpp, storage untouched; the INVARIANT reads below are REAL
 * reads of the same objects the sliced bodies read/write). ---- */
extern s32 sound_requests_enabled;        /* SoundRequests.cpp:51            */
extern u8 controller_reset_requested;     /* ControllerInput.cpp:24          */
extern s32 memcard_access_active;         /* SystemBootInit.cpp:87           */
extern s32 quest_download_in_progress;    /* TQuestVmDownLoadServer.cpp:50   */
extern s32 quest_download_server_ready;   /* TLobby_800F8AA8.cpp:91          */
extern u8 lobby_wait_ready_flag;          /* TLobby_800F8AA8.cpp:93          */

/* ---- carriers for the sliced members (L7-1) ---- */
struct TQuestVmDownLoadServer { static s32 is_ready(); };
struct TLobby { static void wait_ready_loop(); };  /* LB-2 pump-count seam (R8) */
struct TMainTask {
    void begin_frame();                 /* REAL: verbatim TObject.cpp:640    */
    void run_task();                    /* counter seam (Play 2 material)    */
    void render();                      /* counter seam (Play 2 material)    */
    void render_screen_overlay();       /* counter seam (Play 2 material)    */
};
extern TMainTask main_task;             /* era global (TObject.cpp home)     */
struct TLogo { void run_frame(); };     /* bare carrier — gx precedent       */

/* request_main_phase: REAL verbatim slice (SystemBootInit.cpp:219, .s C6/C7
 * pins — same anchor gen_ladder6_logo.py uses). Trap leg can never fire with
 * is_ready()==0 (rung4 §5 invariant); storage is the era zero global. */
void request_main_phase(s32 phase);
struct MainPhaseState { s32 cur, prev, next, result; };
extern MainPhaseState main_phase_state;

/* ---- counted legs around the promoted five (LOGO-2 shape; the L7-R3 order
 * invariant compares this sequence against TLogo.cpp:95-114 text order).
 * Slots with NO event by design: LC7_MT_BEGIN (empty era body), LC7_ASYNC,
 * LC7_SOUND, LC7_POLL (verbatim slices cannot be logged from inside) — each
 * is bracketed at its era text position by the events before/after it, and
 * execution is proven by flip-probe real reads (receipts L7-P*). LC7_READY
 * fires INSIDE the era body chain at wait_ready_loop (LB-2 seam). ---- */
enum L7Call {
    LC7_BEGIN_GX = 1, LC7_ACT_CAM_MODE, LC7_SET_POSE, LC7_CTRL_STUFF, LC7_KBD_FRAME,
    LC7_MT_RUN, LC7_MT_RENDER, LC7_MT_OVERLAY, LC7_ASYNC, LC7_SOUND,
    LC7_POLL, LC7_READY, LC7_RENDER_TICK, LC7_COUNT
};
extern const char *const l7_call_names[];
extern int g_call_seq[64];
extern int g_call_n;
inline void l7_note(int c) { if (g_call_n < 64) g_call_seq[g_call_n++] = c; }

/* counted seam decls (bodies: ladder7_seam.cpp) */
void begin_gx_frame(void);              /* bringup-side count; REAL slice lives gx-side (L7-2) */
void activate_camera_mode(s32 mode);
void set_active_camera_pose(Vec *a, Vec *b);
void controller_stuff(void);
void update_keyboard_input_frame(void);
void render_tick(void);
/* promoted-era legs: defined BY THE SLICE in the glue TU; decls match era homes */
void process_async_file_reads(void);    /* AsyncFileRead.cpp:198  (header also decls) */
void update_sound_request_slots(void);
s32 poll_reset_request(void);

/* ---- seam counters (L7-3; harness reads for the proven-invariant set) ---- */
extern int g_begin_gx_calls, g_ctrl_calls, g_kbd_calls;
extern int g_mt_run_calls, g_mt_render_calls, g_mt_overlay_calls, g_render_tick_calls;
extern int g_cam_mode;
extern Vec g_pose_a, g_pose_b;          /* captured set_active_camera_pose args */
extern int g_adxf_open_calls, g_adxf_readnw_calls, g_adxf_getstat_calls;
extern const void *g_adxf_last_name;    /* ADXF_Open arg capture (probe read-back) */
extern int g_tmpbuf_use_calls;
extern int g_voice_calls, g_fade_calls, g_gpbci_calls;
extern s32 g_gpbci_last_client;
extern int g_loadbml_calls;
extern BmlResource *g_loadbml_last_bml;
extern const char *g_loadbml_last_name;
extern int g_ready_pump_calls;          /* LB-2 wait_ready_loop pump count */

#endif /* LADDER7_RUN_PRELUDE_H */
