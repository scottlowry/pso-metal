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

/* ---- P2-BB V5-S1: era CLUT supply leg (GinjaTextureHeader.cpp home) -----
 * The three bodies below arrive VERBATIM from the decomp in
 * build/gxdl/gen/ladder6_logo_glue.cpp:
 *   swap_u32_bytes      (GinjaTextureHeader.cpp:17, GVPL magic/size swap op)
 *   load_gvpl_palette   (:35, era GVPL chunk consumer: magic gate, size-8
 *                        byte-true move_data into palette->data, format +
 *                        entry_count recorded)
 *   init_and_load_tlut  (:56, era TLUT store: GXInitTlutObj + GXLoadTlut)
 * Types mirror the era layout (include/pso/unk/text_803DAFA8.h:9-26) as a
 * host-port copy — D4 keep-the-dolphin-tree-off-decl-rule. GXTlutObj is the
 * era record (GXStruct.h:43 opaque u32[3] on TARGET_GC); the host seam holds
 * the decoded {lut,fmt,count} so the CPU display path can read the loaded
 * bank (era GXLoadTlut stores r->tlutObj = *t — GXTexture.c:445 — same job).
 * DEVIATION BB-1 (this lane): GX_TL_RGB8-style 24-bit carrier — the era
 * GXTlutFmt set is IA8/RGB565/RGB5A3 (GXEnum.h:782-787); the host CPU
 * expand route (AV §2.2 bullet 2) needs an 8-bit-per-channel lossless
 * carrier to reproduce 256 gray-ramp entries byte-exactly, so the seam
 * decodes formats 0/1/2 era-quantized AND 3 = RGBA8 byte-true. The golden
 * feeds format 3 through the era chain. R3: chunk words are supplied the
 * psoBe working-value regime (magic u32 = PPC-lwz interpretation of 'GVPL',
 * size likewise) exactly like the AN GVR fixture's GVRT desc bytes. */
struct GvplChunk {
    u32 magic;
    u32 size;
    u16 format;
    s16 pad_0A;
    s16 pad_0C;
    s16 entry_count;
    u8 data[1];
};
struct GvplPalette {
    void *data;
    u16 format;
    s16 pad_06;
    s16 pad_08;
    s16 entry_count;
};
struct GXTlutObj { u32 dummy[3]; };
typedef int GXTlutFmt;   /* same alias as host/gx/gx_era.h:23 (identical C++ retypedef is legal) */
enum { GX_TL_IA8 = 0, GX_TL_RGB565 = 1, GX_TL_RGB5A3 = 2, GX_TL_RGBA8_PORT = 3 };  /* :3 = BB-1 carrier */

s32 load_gvpl_palette(GvplChunk *chunk, GvplPalette *palette);   /* era slice */
void init_and_load_tlut(s32 tlut_name, s32 format, s32 entry_count, void *entries);
void swap_u32_bytes(u32 *word);

/* GX TLUT seams (driver-owned storage; era call sites are the slices above). */
extern "C" {
void GXInitTlutObj(GXTlutObj *tlut_obj, void *lut, int fmt, unsigned short n_entries);
void GXLoadTlut(GXTlutObj *tlut_obj, unsigned int tlut_name);
}
struct HostTlutBank {
    void *lut; unsigned fmt; unsigned count; int loaded;
};
#define GXH_TLUT_BANKS 16
extern HostTlutBank g_host_tlut_banks[GXH_TLUT_BANKS];
extern int g_gxinit_tlut_calls, g_gxload_tlut_calls;

#endif /* LADDER6_LOGO_PRELUDE_H */
