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
 * DEVIATION L7-2 (counter legs stay): the pre-begin_gx legs P1–P4 (lane J) are
 * COUNTED seams exactly as LOGO-2 shaped them; PLAY-2 retired the main_task
 * run_task/render/render_screen_overlay counters — those seams are REAL verbatim
 * slices now (spec §1.1) and count through the shared walk extern + the
 * 8-sub-pass roster (spec §1.3). PLAY-3 retires the render_tick counter too:
 * R9 :491-505 is a REAL verbatim slice whose 13 legs run the spec §2.2 roster.
 * rung-5 edits only the seam bodies BEHIND the sliced legs, never the slices
 * (§3 cross-check).
 * begin_gx_frame stays a counter on the bringup side — the REAL
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
/* PLAY-3: carrier gains the render_tick leg-4 static (TObjPlayer/TKeyBoardControl/
 * TParticleCtrl static members verified against era headers this lane:
 * TObjPlayer.h:1105, TKeyBoardControl.h:36, TParticleCtrl.h:56-57, TCameraCtrl.h:151). */
struct TCameraCtrl { Vec pos; static void restore_full_screen_view(); };
extern TCameraCtrl *camera_ctrl_instance;                   /* era decl TCameraCtrl.h:209         */
s32 get_sound_voice_state(s32 voice);                       /* SoundControlState.h:86             */
s32 check_sound_pitch_request_range(s32 slot, s32);         /* SoundRequests.cpp:185              */
void update_music_fade(void);                               /* MusicControl.h:22                  */
struct TObjPlayer { static void clear_shadow_pass_request(); };  /* TObjPlayer.h:1105 static — COUNTED (leg 1 first-inner, era body CameraCapture.cpp:384 static store, do NOT drag the graph) */
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

/* ---- PLAY-2 (spec docs/p2-ladder7-play23.md §1.2): TMainTask.h:17–38 verbatim
 * macro trio + macros.h:89–90 iterator. NOT `#include "pso/system/task/TMainTask.h"`
 * — that header drags the TL_OBJECTS externs + the full TObject include graph
 * (D4/L7-1 keep-the-graph-off rule). Era sha256[:12]: TMainTask.h 2865c241009f,
 * macros.h anchors re-opened this lane. ---- */
#define FOREACH_NODE_NODECL_MULTI_ITER(type, first, varname, ...) \
    for (varname = (type *)(first); varname != NULL; varname = (type *)(varname->m_next), __VA_ARGS__)
#define X_OR_Y_CHILD(flags, old_flags, one_prefix, zero_prefix, suffix) \
    if (flags != old_flags) { \
        TMainTask *child; \
        u32 bit = 1; \
        FOREACH_NODE_NODECL_MULTI_ITER(TMainTask, main_task.m_down, child, bit <<= 1) { \
            if (flags & bit) { \
                child->one_prefix##_##suffix(); \
            } else { \
                child->zero_prefix##_##suffix(); \
            } \
        } \
        old_flags = flags; \
    }
#define PAUSE_OR_RESUME_CHILD(flags, old_flags) \
    X_OR_Y_CHILD(flags, old_flags, pause, resume, update)
#define ALLOW_OR_DISALLOW_CHILD(flags, old_flags, flag_name) \
    X_OR_Y_CHILD(flags, old_flags, disallow, allow, flag_name)

/* main_task_flags TMainTask.h:80–89 verbatim (UPDATING 0x001 :81, RENDERING 0x002
   :82, UPDATED 0x080 :86, RENDERED 0x100 :87); object_flags bits TMainTask.h
   sibling TObject.h:29–36 (PAUSE_UPDATE 0x0008 :32, DISALLOW_RENDER 0x0010 :34)
   — the two the macro carrier methods touch. */
enum main_task_flags {
    MAIN_TASK_UPDATING          = 0x001, // inside run_task / run_tl_camera_tasks
    MAIN_TASK_RENDERING         = 0x002, // inside render / render_clipout_and_fade
    MAIN_TASK_SCENE_PASSES      = 0x010, // inside render_scene_passes
    MAIN_TASK_OBJECT_LAYERS     = 0x020, // inside render_object_layers
    MAIN_TASK_LAYER_02          = 0x040, // inside render_task_layer_02
    MAIN_TASK_UPDATED           = 0x080, // run_task finished this frame
    MAIN_TASK_RENDERED          = 0x100, // render finished this frame
    MAIN_TASK_DEFERRED_OBJECTS  = 0x200  // inside render_objects
};
enum { PAUSE_UPDATE = 0x0008, DISALLOW_RENDER = 0x0010 };

/* carriers: method members verbatim from the era decls — TObject.h:119–122
 * inline bodies; TMainTask class shape per spec §1.2 (members the sliced
 * bodies touch, verified against the era text this lane). */
struct TObject {
    u32 m_flags; TObject *m_prev, *m_down, *m_next;
    void run_tasks();                       /* COUNTED extern (spec §1.3) — era walk :116 */
    void debug_render();                    /* REAL era-empty slice (TObject.cpp:174) */
    void allow_rendering()    { m_flags &= ~DISALLOW_RENDER; }
    void disallow_rendering() { m_flags |=  DISALLOW_RENDER; }
    void resume_update()      { m_flags &= ~PAUSE_UPDATE; }
    void pause_update()       { m_flags |=  PAUSE_UPDATE; }
};
struct TMainTask : TObject {
    u32 task_flags;                         /* era member; main_task def TObject.cpp:33 */
    void begin_frame();                     /* REAL slice — PLAY-1, unchanged */
    void run_task(); void render(); void render_screen_overlay();  /* REAL slices spec §1.1 */
    void render_geometry(); void render_effects(); void render_particle_effects();
    void render_objects();  void render_ui();      void render_clipout_and_fade();
                                            /* COUNTED externs (spec §1.3) — homes below */
    void render_after_scene(); void debug_render_overlay();        /* REAL era-empty  */
};
extern TMainTask main_task;                 /* era global (TObject.cpp home)     */
extern TObject end_of_render_root, keyboard_task_object;  /* PLAY-3 roots, defs spec §1.3 */
void render_info_icon_overlay(void);        /* COUNTED — free fn, TLobby_803382E4.cpp:145 */
extern u32 render_pass, update_flags, old_update_flags, render_flags, old_render_flags;
struct TLogo { void run_frame(); };         /* bare carrier — gx precedent       */

/* request_main_phase: REAL verbatim slice (SystemBootInit.cpp:219, .s C6/C7
 * pins — same anchor gen_ladder6_logo.py uses). Trap leg can never fire with
 * is_ready()==0 (rung4 §5 invariant); storage is the era zero global. */
void request_main_phase(s32 phase);
struct MainPhaseState { s32 cur, prev, next, result; };
extern MainPhaseState main_phase_state;

/* ---- PLAY-3 (spec docs/p2-ladder7-play23.md §2.3): render_tick decomposition
 * zero-defs, GX capture seams, counted legs. ---- */
extern s32 lbl_805D3584;              /* .data INIT 1 — TLobby_801E2F94.cpp:108 (spec §3 D7: debug text era-ENABLED; host def must == 1, P11 asserts the captured arg == 1) */
extern s8  scene_fog_enable_flag;     /* .bss zero — SceneFog.cpp:13 (era s8, verified this lane) */
extern void *video_current_framebuffer; /* .bss zero — gjSystem.c:23 */
/* Era GXCompare/GXCullMode values from libs/dolphin/include/dolphin/gx/GXEnum.h:28–37/:499–504
   (host-local names; the spec §2.3 snippet is incomplete — GX_ALWAYS is needed to
   compile leg :494, and the era ordering is NEVER,LESS,EQUAL,LEQUAL,GREATER,NEQUAL,
   GEQUAL,ALWAYS — the snippet swapped EQUAL/LEQUAL. Full era set pasted so log
   values are era-faithful; deviation noted in the receipt). */
enum { GX_FALSE = 0, GX_TRUE = 1 };
enum { GX_NEVER, GX_LESS, GX_EQUAL, GX_LEQUAL, GX_GREATER, GX_NEQUAL, GX_GEQUAL, GX_ALWAYS };
enum { GX_CULL_NONE = 0, GX_CULL_FRONT, GX_CULL_BACK, GX_CULL_ALL };
/* GX capture seams (bodies ladder7_seam.cpp): legs 3/7/11-inner, 5/9, 8, 11 */
void GXSetZMode(s32 test, s32 comp, s32 mask);
void GXSetViewport(float left, float top, float width, float height, float near, float far);
void GXSetCullMode(s32 mode);  void GXSetColorUpdate(s32 on);
void GXCopyDisp(void *fb, s32 clear);  void GXDrawDone(void);
void present_frame_and_wait_vsync(void);        /* SINGLE-present seam (leg 12, (c)) */
/* sliced-leg decls (bodies arrive VERBATIM in the glue): */
void set_cull_none(void);      void copy_efb_to_framebuffer(void);  void present_frame(void);
void run_end_of_render_tasks(void);      /* render_tick :504 tail — def later in the glue */
void enable_scene_fog(void);   void disable_scene_fog(void);
/* counted/fail-loud externs behind the slices: */
void apply_scene_fog(void);                     /* SceneFog.cpp:53 — (d) pixel chain, COUNTED */
void update_global_sound_frame(void);           /* SoundPlayback.h:27 / .cpp:232 — COUNTED (musyx chain) */
void update_music_sequencer(void);              /* MusicControl.cpp — LB-3 fail-loud (era-unreachable: sound_requests_enabled==0, guard :103 REAL) */
void render_debug_text(u32 enabled);            /* DebugFont.cpp:140 — COUNTED, arg-captured (leg 6) */
struct TKeyBoardControl { static void run_tasks(); };   /* TKeyBoardControl.h:36 static — body VERBATIM slice SystemBootInit.cpp:100 */
struct TParticleCtrl { static void begin_effect_render(); static void end_effect_render(); };  /* TParticleCtrl.h:56-57 static — bodies VERBATIM EffectRender.cpp:10/:12 */

/* ---- counted legs around the promoted five (LOGO-2 shape; the L7-R3 order
 * invariant compares this sequence against TLogo.cpp:95-114 text order).
 * Slots with NO event by design: LC7_MT_BEGIN (empty era body), LC7_ASYNC,
 * LC7_SOUND, LC7_POLL (verbatim slices cannot be logged from inside) — each
 * is bracketed at its era text position by the events before/after it, and
 * execution is proven by flip-probe real reads (receipts L7-P*). LC7_READY
 * fires INSIDE the era body chain at wait_ready_loop (LB-2 seam). PLAY-2:
 * the LC7_MT_RUN/MT_RENDER/MT_OVERLAY counter slots LEAVE (their seams are
 * REAL slices now); the flag spine counts through the walk extern
 * (LC7_WALK_MAIN/KBD/EOR, spec §1.3) and the 8-sub-pass roster
 * (GEOM/EFF/PTEX/OBJ/UI/CLIP/INFO — run_tasks is the walk, not a 9th).
 * LC7_WALK_* must stay in that exact order: the extern notes
 * LC7_WALK_MAIN + (root - WALK_MAIN). ---- */
enum L7Call {
    LC7_BEGIN_GX = 1, LC7_ACT_CAM_MODE, LC7_SET_POSE, LC7_CTRL_STUFF, LC7_KBD_FRAME,
    LC7_ASYNC, LC7_SOUND, LC7_POLL, LC7_READY, LC7_RENDER_TICK,
    LC7_WALK_MAIN, LC7_WALK_KBD, LC7_WALK_EOR,
    LC7_GEOM, LC7_EFF, LC7_PTEX, LC7_OBJ, LC7_UI, LC7_CLIP, LC7_INFO,
    /* PLAY-3 (spec §2.3): the 13 render_tick sub-leg notes — each fires at the
     * leg's first INNER counted/capture extern (LB-2 LC7_READY precedent).
     * LC7_RENDER_TICK retires with the render_tick counter body (seam-4). */
    LC7_RT1, LC7_RT2, LC7_RT3, LC7_RT4, LC7_RT5, LC7_RT6, LC7_RT7,
    LC7_RT8, LC7_RT9, LC7_RT10, LC7_RT11, LC7_RT12, LC7_RT13,
    LC7_COUNT
};
enum { WALK_MAIN, WALK_KBD, WALK_EOR };   /* run_tasks root classification (spec §1.3) */
extern const char *const l7_call_names[];
extern int g_call_seq[512];               /* PLAY-3 sizing: 29 events/frame post-PLAY-3
                                           * x 6 harness frames + probes > the spec §2.3
                                           * [160] estimate (receipt deviation D8) */
extern int g_call_n;
inline void l7_note(int c) { if (g_call_n < 512) g_call_seq[g_call_n++] = c; }

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
extern int g_render_tick_calls;           /* PLAY-3: tick count rides render_tick's first statement (leg 1 epoch in seam) */
extern int g_cam_mode;
extern Vec g_pose_a, g_pose_b;            /* captured set_active_camera_pose args */
extern int g_adxf_open_calls, g_adxf_readnw_calls, g_adxf_getstat_calls;
extern const void *g_adxf_last_name;    /* ADXF_Open arg capture (probe read-back) */
extern int g_tmpbuf_use_calls;
extern int g_voice_calls, g_fade_calls, g_gpbci_calls;
extern s32 g_gpbci_last_client;
extern int g_loadbml_calls;
extern BmlResource *g_loadbml_last_bml;
extern const char *g_loadbml_last_name;
extern int g_ready_pump_calls;          /* LB-2 wait_ready_loop pump count */

/* ---- PLAY-2 flag-spine seam state (spec docs/p2-ladder7-play23.md §1.3/§1.5;
 * definitions in ladder7_seam.cpp; the P5-P9 probes + I2a..e invariants REAL-read
 * these — the same objects the verbatim slices drive). ---- */
extern int g_walk_calls;                /* TObject::run_tasks extern count */
extern int g_walk_root;                 /* WALK_MAIN / WALK_KBD / WALK_EOR */
extern u32 g_walk_flags;                /* main_task.task_flags LIVE at run_tasks() */
extern int g_geom, g_eff, g_ptex, g_obj, g_ui, g_clip, g_info;  /* 8 counted sub-pass externs */
extern u8 g_pass_trace[16];             /* render_pass value at each sub-pass entry (I2a) */
extern int g_pass_n;                    /* trace write cursor (clamped, spec §1.3) */
extern u32 g_flags_at_subpass;          /* task_flags LIVE at the most recent sub-pass */

/* ---- PLAY-3 render_tick seam state (spec §2.2/§2.4: probes P10-P12 + I3a..d
 * REAL-read these; definitions in ladder7_seam.cpp). ---- */
extern int g_fog_reads;                 /* apply_scene_fog count (legs 2/10 inner) */
extern int g_clearshadow_calls;         /* TObjPlayer::clear_shadow_pass_request (leg 1 first-inner) */
extern int g_soundframe_calls;          /* update_global_sound_frame (leg 1) */
extern int g_rfs_calls;                 /* TCameraCtrl::restore_full_screen_view (leg 4, counted (d)) */
extern int g_dbgtext_calls;  extern u32 g_dbgtext_arg;   /* render_debug_text + arg capture (leg 6, lbl_805D3584) */
extern int g_zmode_calls;               /* GXSetZMode capture count (legs 3/7/11-inner) */
extern int g_zmode_trace[3][3];         /* (test,comp,mask) per capture — P11 */
extern int g_cull_calls, g_cull_mode;   /* GXSetCullMode capture (legs 5/9) — must reach 2/frame */
extern int g_vp_calls;                  /* GXSetViewport capture (leg 8) */
extern float g_vp_l, g_vp_t, g_vp_w, g_vp_h, g_vp_n, g_vp_f;
extern int g_colorupdate_calls;         /* GXSetColorUpdate capture (leg 11 inner) */
extern void *g_copydisp_fb; extern int g_copydisp_clear, g_copydisp_calls;  /* GXCopyDisp args (leg 11) */
extern int g_drawdone_calls;            /* GXDrawDone capture (leg 11) */
extern int g_present_calls;             /* present_frame_and_wait_vsync — SINGLE present (I3b) */

#endif /* LADDER7_RUN_PRELUDE_H */
