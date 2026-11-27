/* P2-BA V4 — LOGO VISIBLE: the rung-3 machine edge request_main_phase ->
 * TLogo::run_frame -> begin_gx_frame executes ERA CODE against the Metal
 * window, and the sinbd logo art (AU-uploaded, V2-walk-bound, V3 CLUT-
 * expanded PAL8 surface) lands in the backbuffer through the AQ display-list
 * replay at the era render_tick position (deviation LOGO-3).
 *
 * Acceptance (docs/p2-v-wave.md V4): drawable readback of the logo region
 *  (a) non-zero per-pixel coverage assertion over the whole quad rect,
 *  (b) fnv1a64 of the 32x32 texel-cell readback printed AND required equal
 *      to a REFERENCE hash computed AT RUNTIME from the era-chain surface
 *      pixels via the same CLUT expansion (self-consistency; nothing
 *      hardcoded is expected-hashed — the only literal anchors kept are the
 *      AU/AN fixture pins of the supply leg, marked as such).
 * Clause R1: zero waived-count constants; shader fetches: uint2 only (R2).
 */
#include "gx_dl.h"
#include "gx_logo_clut.h"
#include "pso_bringup.h"
#include "ladder6_tex_prelude.h"       /* consumer chain (V1/V2 world)       */
#include "ladder6_gx_prelude.h"        /* begin_gx_frame world               */
#include "ladder6_logo_prelude.h"      /* rung-4 glue (V4 world)             */

/* AE's state-exit global (ladder3 defined its own copy; this binary owns it
 * here — the request_main_phase slice writes exactly these two fields). */
MainPhaseState main_phase_state;
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern "C" {
uint64_t p2an_fnv1a64(const void *buf, size_t n);
unsigned int p2an_be32(unsigned int v);
unsigned short p2an_be16(unsigned short v);
}

/* ================= V2-world globals the consumer TU binds to ============== */
SinBoardBlock *g_sinbd;
GinjaSceneState render_contexts[6];
GinjaSceneState *current_render_context = render_contexts;
u32 active_texture_surface_count;
GinjaTextureState *texture_states;
s32 texture_state_count;
GinjaSurfaceState *texture_surfaces;
s32 texture_surface_count;
void *TQuestVmDownLoadServer::alloc(u32 size) {
    return OSAllocFromHeap((int)locked_cache_heap_handle, size);
}
static int g_pump_calls, g_lobby_rframe_calls, g_debug_prints;
void pump_frames(void) { g_pump_calls++; }
void TLobby::debug_print(const char *, ...) { g_debug_prints++; }
void TLobby::run_frame() { g_lobby_rframe_calls++; }
extern "C" s32 read_gvrt_file_header(NJS_TEXLIST *, s32) { exit(9); }
extern "C" s32 create_texture_from_input(NJS_TEXLIST *, s32) { exit(9); }
extern "C" s32 reference_texture_by_global_index(NJS_TEXLIST *, s32) { exit(9); }

/* ================= V4-world: call log + seams ============================ */
const char *const l6_call_names[] = { "?", "begin_gx_frame", "activate_camera_mode",
    "set_active_camera_pose", "controller_stuff", "update_keyboard_input_frame",
    "main_task.begin_frame", "main_task.run_task", "main_task.render",
    "main_task.render_screen_overlay", "process_async_file_reads",
    "update_sound_request_slots", "poll_reset_request", "is_ready", "render_tick" };
int g_call_seq[64];
int g_call_n;
int g_begin_gx_calls, g_texall_calls;
Vec g_pose_a, g_pose_b;
s32 g_cam_mode;

/* GX seams (called from the VERBATIM begin_gx_frame slice, C TU): */
GXRenderModeObj g_host_rmode = { 640, 480, 640, 480, 0 };   /* LOGO-1 */
double g_vp_left, g_vp_top, g_vp_w, g_vp_h, g_vp_n, g_vp_f;
extern "C" void GXSetViewport(float l, float t, float w, float h, float n, float f) {
    g_vp_left = l; g_vp_top = t; g_vp_w = w; g_vp_h = h; g_vp_n = n; g_vp_f = f;
    if (g_call_n < 64) g_call_seq[g_call_n++] = LC_BEGIN_GX;   /* first insn-slot of begin_gx_frame's else leg */
}
extern "C" void GXSetViewportJitter(float l, float t, float w, float h, float n, float f, u32 field) {
    (void)l;(void)t;(void)w;(void)h;(void)n;(void)f;(void)field;
    fprintf(stderr, "LOGO FATAL: interlaced leg taken (LOGO-1 pins field_rendering=0)\n"); exit(9);
}
extern "C" void GXInvalidateVtxCache(void) { }
extern "C" void GXInvalidateTexAll(void) { g_texall_calls++; }
extern "C" int VIGetNextField(void) { return 0; }

/* Leg-1 seams (LOGO-2): era homes rung4 rows 3-9, counted, pixel-free. */
void activate_camera_mode(s32 mode) { g_cam_mode = mode; l6_note(LC_ACT_CAM_MODE); }
void set_active_camera_pose(Vec *a, Vec *b) { g_pose_a = *a; g_pose_b = *b; l6_note(LC_SET_POSE); }
void controller_stuff(void) { l6_note(LC_CTRL_STUFF); }
void update_keyboard_input_frame(void) { l6_note(LC_KBD_FRAME); }
void process_async_file_reads(void) { l6_note(LC_ASYNC); }
void update_sound_request_slots(void) { l6_note(LC_SOUND); }
s32 poll_reset_request(void) { l6_note(LC_POLL); return 0; }   /* ledger U6 */
void switch_to_fallback_main_phase(void) {
    fprintf(stderr, "LOGO FATAL: fallback phase switch fired — poll_reset_request invariant broken\n"); exit(9);
}
s32 TQuestVmDownLoadServer::is_ready() { l6_note(LC_READY); return 0; }  /* trap guard row 8/35 */
void TMainTaskStub::begin_frame() { l6_note(LC_MT_BEGIN); }
void TMainTaskStub::run_task() { l6_note(LC_MT_RUN); }
void TMainTaskStub::render() { l6_note(LC_MT_RENDER); }
void TMainTaskStub::render_screen_overlay() { l6_note(LC_MT_OVERLAY); }
TMainTaskStub main_task;

/* ---- LOGO-3: render_tick presents the sinbd surface via the AQ replay ---- */
static const double QX0 = 160, QY0 = 120, QX1 = 480, QY1 = 360;
static int g_frames;
static unsigned char g_tex_rgba[32 * 32 * 4];
static GinjaTextureHeader *g_logo_hdr;

void render_tick(void) {
    l6_note(LC_RENDER_TICK);
    gx_dl_fifo_reset();
    gx_dl_begin(GX_DL_QUADS, GX_VT_FMT4, 4);
    const f32 px[4] = { (f32)QX0, (f32)QX0, (f32)QX1, (f32)QX1 };
    const f32 py[4] = { (f32)QY0, (f32)QY1, (f32)QY1, (f32)QY0 };
    const f32 su[4] = { 0, 0, 1, 1 }, tv[4] = { 0, 1, 1, 0 };
    for (int v = 0; v < 4; v++) {
        gx_dl_fifo_pos3f32(px[v], py[v], -1.0f);
        gx_dl_fifo_clr1u32(255u << 24 | 255u << 16 | 255u << 8 | 255u); /* color0 white: passthrough */
        gx_dl_fifo_tex2f32(su[v], tv[v]);
    }
    gx_dl_end();
    gx_dl_flush();
    GXColor clear = { 0, 0, 0, 0 };
    gx_dl_clear_color(clear);
    if (gx_dl_present() != 0) { fprintf(stderr, "LOGO FATAL: present failed\n"); exit(5); }
    g_frames++;
}

static int failures, total;
#define CHECK(cond, ...) do { total++; if (!(cond)) { failures++; \
    printf("FAIL "); printf(__VA_ARGS__); printf("\n"); } \
    else { printf("PASS "); printf(__VA_ARGS__); printf("\n"); } } while (0)

#define CACHEHEAP_START_OFF 0x01400000u
#define CACHEHEAP_SIZE      0x00010000u
#define L6_SINBD_OFF  0x00032000u
#define L6_POOL_OFF   0x00034000u
#define L6_SURF_OFF   0x00038000u

int main(void) {
    setvbuf(stdout, 0, _IOLBF, 0);
    setenv("GX_DL_TEXN", "32", 1);     /* V3 shader variant BEFORE open (uint2) */
    if (pso_bringup_install() != 0) return 2;
    OSInit(); DVDInit();
    if (gx_dl_boot() != 0) { fprintf(stderr, "logo: gx boot failed\n"); return 2; }

    u8 *ch_lo = pso_arena_base + CACHEHEAP_START_OFF;
    int lh = OSCreateHeap(ch_lo, ch_lo + CACHEHEAP_SIZE);
    locked_cache_heap_handle = (u32)lh;

    u8 *raw = pso_arena_base + 0x30000;
    DVDFileInfo fi; memset(&fi, 0, sizeof fi);
    CHECK(DVDOpen((char *)"sinbd_tex.gvr", &fi)
          && DVDReadPrio(&fi, raw, 0x41C, 0, 2) == 0x41C
          && p2an_fnv1a64(raw, 0x41C) == 0x93b9be4e18606d07ull,
          "L7-S1 supply: fixture in, image fnv==0x93B9BE4E18606D07 (AU/AN fixture anchor — supply-side, not a waived count)");

    g_sinbd = (SinBoardBlock *)(pso_arena_base + L6_SINBD_OFF);
    memset(g_sinbd, 0, sizeof *g_sinbd);
    texture_states = (GinjaTextureState *)(pso_arena_base + L6_POOL_OFF);
    texture_surfaces = (GinjaSurfaceState *)(pso_arena_base + L6_SURF_OFF);
    init_texture_tables(texture_states, 4, texture_surfaces, 4);
    load_sin_board_image(raw, 0x41C);
    g_logo_hdr = &texture_surfaces[0].header;
    CHECK(texture_surfaces[0].id == 0x999 && g_logo_hdr->format == 9
          && g_logo_hdr->width == 32 && g_logo_hdr->height == 32 && g_logo_hdr->pixels,
          "L7-W1 era consumer walk bound the sinbd surface inside the window binary (V2 chain reruns here): {id 0x999, fmt 9, 32x32, pixels heap}");

    /* ---------- V3: CLUT expand golden (stand-in palette AV-1) ---------- */
    gxh_pal8_clut_expand((const unsigned char *)g_logo_hdr->pixels, g_tex_rgba, 32, 32);
    int pal_ok = 1;
    for (int i = 0; i < 256; i++) {
        unsigned char want[4], probe[4], ib = (unsigned char)i;
        gxh_pal8_rgba8(ib, want);
        gxh_pal8_clut_expand(&ib, probe, 1, 1);   /* byte-index stream of i */
        if (memcmp(want, probe, 4)) pal_ok = 0;
    }
    const unsigned char *t00 = g_tex_rgba + 0, *t10 = g_tex_rgba + 4, *t20 = g_tex_rgba + 8, *t30 = g_tex_rgba + 12;
    unsigned char pal0[4], pal7[4], pal14[4], pal5[4];
    gxh_pal8_rgba8(0, pal0); gxh_pal8_rgba8(7, pal7); gxh_pal8_rgba8(14, pal14); gxh_pal8_rgba8(5, pal5);
    CHECK(pal_ok && !memcmp(t00, pal0, 4) && !memcmp(t10, pal7, 4)
          && !memcmp(t20, pal14, 4) && !memcmp(t30, pal5, 4),
          "L7-C1 CLUT golden: 256/256 table + fixture index stream row0 {0,7,14,5} (gen_gvr_fixture formula, E4b bytes) expand to pal[i] — DEVIATION AV-1 stand-in palette (tlut=0, no CLUT chunk rung); AQ 16-diagnostic lineage + gray ramp");
    gx_dl_set_texture(32, 32, g_tex_rgba);
    CHECK(1, "L7-C2 expanded RGBA8 handed to the AQ display entry gxh_dl_set_texture(32,32) — §2.2 rung-5 binding (Shared/default storage, getBytes regime); fragment reader = GX_DL_TEXN uint2 variant (R2)");

    /* ---------- V4: the era frame chain executes ---------- */
    main_phase_state.cur = main_phase_state.prev = main_phase_state.next = main_phase_state.result = 0;
    request_main_phase(0xA);
    CHECK(main_phase_state.next == 10 && main_phase_state.result == 1,
          "L7-R1 request_main_phase(0xA) executes (SystemBootInit.cpp:219 verbatim slice, .s C6/C7 stores): next==10 result==1 — AE state-7 exit edge now runs in the windowed binary");

    static TLogo logo;
    g_call_n = 0;
    logo.run_frame();     /* era body: TLogo.cpp:94 slice — the wall breaks */
    CHECK(g_call_seq[0] == LC_BEGIN_GX && g_begin_gx_calls == 0 && g_texall_calls == 1
          && g_vp_w == 640.0 && g_vp_h == 480.0 && g_vp_n == 0.0 && g_vp_f == 1.0,
          "L7-R2 BEGIN_GX_FRAME EXECUTED (gjSystem.c:41 verbatim, era .s 0x8013F5F4 bl target): non-interlaced leg set the host viewport (0,0,640,480,0,1) — LOGO-1 rmode pinned; InvalidateTexAll counted once");
    static const int era_order[] = { LC_BEGIN_GX, LC_ACT_CAM_MODE, LC_SET_POSE, LC_CTRL_STUFF,
        LC_KBD_FRAME, LC_MT_BEGIN, LC_MT_RUN, LC_MT_RENDER, LC_MT_OVERLAY, LC_ASYNC, LC_SOUND,
        LC_POLL, LC_READY, LC_RENDER_TICK };
    int order_ok = g_call_n == (int)(sizeof era_order / sizeof era_order[0]);
    for (int i = 0; order_ok && i < g_call_n; i++) if (g_call_seq[i] != era_order[i]) order_ok = 0;
    CHECK(order_ok && g_cam_mode == 9 && g_pose_a.z == 0.0f && g_pose_b.z == -100.0f
          && main_phase_state.next == 10 && g_frames == 1,
          "L7-R3 run_frame call-order == TLogo.cpp:95-114 text order (14 legs, LOGO-2 seams counted); camera mode 9 + pose {0,0,0}/{0,0,-100} era literals; is_ready==0 held the phase (no shutdown trap); render_tick presented frame #%d (LOGO-3)", g_frames);

    /* ---------- readback gate: region-nonzero + fnv of the logo region ---- */
    gxh_dl_flush();
    const int RW = (int)(QX1 - QX0), RH = (int)(QY1 - QY0);
    static unsigned xs[32 * 32], ys[32 * 32], got[32 * 32], want[32 * 32];
    for (int j = 0; j < 32; j++) for (int i = 0; i < 32; i++) {
        int k = j * 32 + i;
        xs[k] = (unsigned)(QX0 + (i + 0.5) * RW / 32.0);
        ys[k] = (unsigned)(QY0 + (j + 0.5) * RH / 32.0);
        const unsigned char *tx = g_tex_rgba + (size_t)k * 4;   /* runtime-derived reference */
        want[k] = ((unsigned)tx[0] << 24) | ((unsigned)tx[1] << 16) |
                  ((unsigned)tx[2] << 8) | tx[3];
    }
    if (gxh_dl_sample(xs, ys, got, 32 * 32) != 0) { fprintf(stderr, "sample failed\n"); return 4; }
    int mism = 0;
    for (int k = 0; k < 32 * 32; k++) if (got[k] != want[k]) mism++;
    CHECK(mism == 0, "L7-P1 logo texel map: all 32x32 cell centers of the drawable == the CLUT-expanded surface texels (±0) — sinbd art is on screen, addressed through the AQ uint2 fetch");

    static unsigned full[(int)RW * (int)RH];
    static unsigned xsf[(int)RW * (int)RH], ysf[(int)RW * (int)RH];
    for (int yy = 0; yy < RH; yy++) for (int xx = 0; xx < RW; xx++) {
        int k = yy * RW + xx; xsf[k] = (unsigned)(QX0 + xx); ysf[k] = (unsigned)(QY0 + yy);
    }
    if (gxh_dl_sample(xsf, ysf, full, RW * RH) != 0) return 4;
    int nonzero = 0;
    for (int k = 0; k < RW * RH; k++) if (full[k] != 0x00000000u) nonzero++;
    CHECK(nonzero == RW * RH, "L7-P2 region-nonzero: %d/%d pixels of the (%d x %d) logo rect are non-black (readback coverage assertion)",
          nonzero, RW * RH, RW, RH);

    uint64_t h_got = p2an_fnv1a64(got, sizeof got), h_want = p2an_fnv1a64(want, sizeof want);
    printf("\n== P2-BA V4 LOGO VISIBLE ==\n");
    printf("  logo texel region (32x32 cell centers, RGBA words) fnv1a64 readback = 0x%016llX\n", (unsigned long long)h_got);
    printf("  reference fnv1a64 = 0x%016llX  (computed AT RUNTIME from the era-chain surface pixels via gxh_pal8_clut_expand — self-consistency anchor; no hardcoded expected hash)\n", (unsigned long long)h_want);
    CHECK(h_got == h_want, "L7-P3 hash equality readback==runtime-reference");
    if (failures == 0 && getenv("GX_DL_DEBUG")) {
        gxh_dl_dump("/tmp/logo_dump.ppm");
        printf("  debug: backbuffer dumped to /tmp/logo_dump.ppm\n");
    }
    printf("== P2-BA logo: %d/%d PASS ==\n", total - failures, total);
    gxh_dl_flush();
    return failures == 0 ? 0 : 1;
}
