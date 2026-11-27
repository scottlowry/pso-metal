/* P2-BA V4 — logo-frame glue prelude (C++ TU side).
 *
 * Drives the rung-4 transition contract's Leg-0→Leg-1 edge:
 *   request_main_phase (SystemBootInit.cpp:219-222, .s 0x8000F9CC — already
 *   the AE state-7 exit golden C6/C7)  then  TLogo::run_frame
 *   (TLogo.cpp:94, first executed bl begin_gx_frame @ TLogo.s 0x8013F5F4 —
 *   THE wall, docs/p2-rung4-contract.md §2).
 *
 * DEVIATION LOGO-2 (call-order injection, AP §5 + bringup-3 DA-3 precedent):
 * run_frame's legs BEHIND begin_gx_frame (rung4 rows 3-9) are class (c)/(d)
 * homes — TMainTask::render/overlay (NJS-LOCKED TObject.cpp), async reads,
 * sound slots, controller poll, quest-VM ready. They are prelude seams that
 * COUNT and never produce pixels; the invariants are asserted:
 * poll_reset_request()==0 (ledger U6), TQuestVmDownLoadServer::is_ready()==0
 * (row 8/35 never-ready trap — a 1 boots straight off title), camera pose
 * args {0,0,0}/{0,0,-100} (era .s Vec literals).
 * DEVIATION LOGO-3: render_tick (TLobby_801E2F94.cpp:491, row 9, class (c))
 * is the pixel seam: its host backing presents the CLUT-expanded sinbd
 * surface through the AQ display-list replay — the logo art reaches the
 * backbuffer AT the era call-order position of the present leg.
 */
#ifndef LADDER6_LOGO_PRELUDE_H
#define LADDER6_LOGO_PRELUDE_H

#include "types.h"                 /* PORT types.h (arena regime)             */
#include <stddef.h>
#include "ladder6_tex_prelude.h"   /* ONE world: shadow structs + seams shared */

/* Vec: era dolphin/mtx/GeoTypes.h math type; the run_frame body only builds
 * {0,0,0}/{0,0,-100} literals + passes addresses — 3xf32 shape pinned here
 * (D4 keeps the full dolphin tree off the GLUE TU; when the era header chain
 * is already bound in a consumer TU — guard DOLPHIN_MTX_GEOTYPES_H — that
 * definition wins, the layouts are identical f32[3]). */
#ifndef DOLPHIN_MTX_GEOTYPES_H
struct Vec { f32 x, y, z; };
#endif

/* main_phase_state: the era global (MainPhaseState.cpp definition, .s
 * 0x8000F9D4 addi-leg; storage pinned by AE goldens C6/C7 in ladder3). */
struct MainPhaseState { s32 cur, prev, next, result; };
extern MainPhaseState main_phase_state;

/* request_main_phase slice target (SystemBootInit.cpp:219). */
void request_main_phase(s32 phase);

/* begin_gx_frame: THE era GX boundary (gjSystem.c:41) — real C11 slice in
 * ladder6_gx_glue.c (first time this function EXECUTES on arm64). */
extern "C" void begin_gx_frame(void);

/* TLogo: only run_frame is sliced this rung (era ctor/dtor chain stays with
 * the ladder3 harness; run_frame touches no member — verified against the
 * era body: every callee is global/static, so a bare class carrier is
 * behavior-identical to the era member call). */
struct TLogo { void run_frame(); };

/* ---- Leg-1 seam inventory (LOGO-2): call-order log ---------------------
 * g_call_seq collects the era call sequence INSIDE run_frame; the driver
 * asserts the recorded order equals the era text order (TLogo.cpp:95-114). */
enum L6Call {
    LC_BEGIN_GX = 1, LC_ACT_CAM_MODE, LC_SET_POSE, LC_CTRL_STUFF, LC_KBD_FRAME,
    LC_MT_BEGIN, LC_MT_RUN, LC_MT_RENDER, LC_MT_OVERLAY, LC_ASYNC, LC_SOUND,
    LC_POLL, LC_READY, LC_RENDER_TICK, LC_COUNT
};
extern const char *const l6_call_names[];
extern int g_call_seq[64];
extern int g_call_n;
extern int g_begin_gx_calls;      /* bumped by the GX seams in glue.c TU */
extern int g_texall_calls;
extern Vec g_pose_a, g_pose_b;    /* captured set_active_camera_pose args */
extern s32 g_cam_mode;

inline void l6_note(int c) { if (g_call_n < 64) g_call_seq[g_call_n++] = c; }

/* free-function legs (era homes per rung4 rows) */
void activate_camera_mode(s32 mode);
void set_active_camera_pose(Vec *a, Vec *b);
void controller_stuff(void);
void update_keyboard_input_frame(void);
void process_async_file_reads(void);
void update_sound_request_slots(void);
s32 poll_reset_request(void);
void switch_to_fallback_main_phase(void);   /* fail-loud: must never fire */
void render_tick(void);                      /* LOGO-3 pixel seam (driver) */

/* TMainTask main_task (era global TObject.cpp; row 3/4/13 home NJS-LOCKED) */
struct TMainTaskStub {
    void begin_frame();
    void run_task();
    void render();
    void render_screen_overlay();
};
extern TMainTaskStub main_task;

/* never-ready trap guard (row 8 / ledger 34): is_ready() member declared in
 * the shared tex prelude; returns 0, INVARIANT asserted at L7-R3. */

#endif /* LADDER6_LOGO_PRELUDE_H */
