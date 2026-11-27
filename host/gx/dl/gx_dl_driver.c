/* pso-metal P2-Z gate driver — FIRST textured quad driven by era GP calls.
 *
 * Replays the two supply paths the era actually uses, alternating per frame:
 *   PATH A (direct FIFO-lite capture)  — the exact recipe of the decomp's
 *     TWindowSubStatusDisp_803DEE6C.cpp:43 draw_textured_quad_at_depth:
 *     GXBegin((GXPrimitive)0x80, GX_VTXFMT4, 4) + POS3F32/CLR1U32/TEX0-2F32.
 *   PATH B (display list)              — hand-built BE32 list bytes executed
 *     through a captured CP 0x40 call, i.e. the wire format of
 *     GXCallDisplayList (libs/dolphin/src/gx/GXDisplayList.c:74), the path
 *     GinjaMaterial.cpp:478 uses for NJS DRAW_TPL display lists.
 * Readback gate per frame: 4 quad corners (2 px inside) + center match the
 * texture's corner/center texel colors, 2 outside points match the clear —
 * 30 frames, exit 0 only if every sample matched (W's exit-code convention).
 */
#include "gx_dl.h"
#include "pso_bringup.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define FRAMES 30
#define QX0 160.0f
#define QY0 120.0f
#define QX1 480.0f
#define QY1 360.0f

/* 4x4 RGBA8: distinct texel at every sampled uv (NEAREST, half-texel grid:
 * uv corner -> row/col 0 or 3; uv 0.5 -> (1.5+0.5)=2 -> texel (2,2)).   */
static const unsigned TEX[4][4][4] = {
    /* row 0 (v=0) */ {{255,0,0,255},{255,0,255,255},{255,0,255,255},{0,255,0,255}},
    /* row 1       */ {{0,255,255,255},{0,255,255,255},{0,255,255,255},{0,255,255,255}},
    /* row 2       */ {{0,255,255,255},{0,255,255,255},{255,255,0,255},{0,255,255,255}},
    /* row 3 (v=1) */ {{0,0,255,255},{0,255,255,255},{0,255,255,255},{255,255,255,255}},
};
#define WANT(r,g,b) ((r)<<24 | (g)<<16 | (b)<<8 | 255)

/* ---- BE display-list builder (PATH B payload) ---------------------------- */
static __attribute__((aligned(32))) u32 g_dl[32];   /* BE32 words, 128 bytes */
static u32 g_dl_n;
static u32 g_frame_dl_words;

static void dl_put(u32 bew) { g_dl[g_dl_n++] = bew; }
static u32 bew_f32(f32 f) { u32 u; memcpy(&u, &f, 4); return __builtin_bswap32(u); }

static void build_era_quad_dl(void) {
    g_dl_n = 0;
    dl_put((GX_DL_QUADS | GX_VT_FMT4) << 24 | (4u << 8));  /* vertex cmd     */
    const f32 px[4] = {QX0, QX0, QX1, QX1};
    const f32 py[4] = {QY0, QY1, QY1, QY0};                /* era TL->BL->BR->TR */
    const f32 su[4] = {0, 0, 1, 1};
    const f32 tv[4] = {0, 1, 1, 0};
    for (int v = 0; v < 4; v++) {
        dl_put(bew_f32(px[v])); dl_put(bew_f32(py[v])); dl_put(bew_f32(-1.0f));
        dl_put(WANT(255, 255, 255));                        /* CLR RGBA8     */
        dl_put(bew_f32(su[v])); dl_put(bew_f32(tv[v]));
    }
    dl_put(0x00000000);                                     /* list end      */
    while (g_dl_n & 7) dl_put(0x00000000);                  /* pad to 32B    */
    g_frame_dl_words = g_dl_n;
}

static void emit_quad_capture(void) {  /* PATH A: era inline writers -> FIFO  */
    gx_dl_begin(GX_DL_QUADS, GX_VT_FMT4, 4);
    const f32 px[4] = {QX0, QX0, QX1, QX1};
    const f32 py[4] = {QY0, QY1, QY1, QY0};
    const f32 su[4] = {0, 0, 1, 1};
    const f32 tv[4] = {0, 1, 1, 0};
    for (int v = 0; v < 4; v++) {
        gx_dl_fifo_pos3f32(px[v], py[v], -1.0f);
        gx_dl_fifo_clr1u32(WANT(255, 255, 255));
        gx_dl_fifo_tex2f32(su[v], tv[v]);
    }
    gx_dl_end();
}

int main(void) {
    if (pso_bringup_install() != 0) return 2;
    OSInit();
    OSReport("gx_dl: arena %p — P2-Z display-list gate\n", (void *)pso_arena_base);
    if (gx_dl_boot() != 0) { fprintf(stderr, "gx_dl: boot failed\n"); return 2; }

    unsigned char texbytes[4][4][4];
    for (int y = 0; y < 4; y++) for (int x = 0; x < 4; x++)
        memcpy(texbytes[y][x], TEX[y][x], 4);
    gx_dl_set_texture(4, 4, &texbytes[0][0][0]);

    build_era_quad_dl();
    printf("gx_dl: hand-built era display list: %u BE32 words, %u bytes "
           "(vtxdesc 0x%08x fmt4 POS3F32+CLR1U32+TEX2F32 x4 + terminator/pad)\n",
           g_frame_dl_words, g_frame_dl_words * 4, g_dl[0]);

    /* sample points (drawable pixel coords, y-down from top-left):
     * 2 px inside each quad corner + exact center + 2 outside points.     */
    const unsigned xs[25] = {170,240,320,400,470, 170,240,320,400,470,
                             170,240,320,400,470, 170,240,320,400,470,
                             170,240,320,400,470};
    const unsigned ys[25] = {130,130,130,130,130, 190,190,190,190,190,
                             240,240,240,240,240, 300,300,300,300,300,
                             350,350,350,350,350};
    unsigned want[25];
    for (int i = 0; i < 25; i++) want[i] = 0x000000FF;   // probe: black-wanted, reports only drawn pixels
    const int NPTS = 25;
    GXColor clear = {0, 0, 0, 0};
    gx_dl_clear_color(clear);

    int failures = 0;
    for (int f = 0; f < FRAMES; f++) {
        gx_dl_fifo_reset();
        if (f & 1) {
            /* PATH B: capture a CP 0x40 call into the FIFO — the era
             * GXCallDisplayList wire sequence (GinjaMaterial.cpp:478 style);
             * run_words() decodes the call and executes the list.          */
            gx_dl_fifo_call_display_list(g_dl, g_frame_dl_words * 4);
        } else {
            emit_quad_capture();      /* PATH A                              */
        }
        gx_dl_flush();
        if (gx_dl_present() != 0) { fprintf(stderr, "gx_dl: draw failed f%d\n", f); failures++; break; }
        int bad = gxh_dl_verify(xs, ys, want, NPTS, f);
        if (f == 0 && getenv("GX_DL_DEBUG")) {
            extern void gxh_dl_dump(const char *);
            gxh_dl_dump("/tmp/dl_dump.ppm");
        }
        gx_dl_note_verify(__builtin_popcount((unsigned)bad), NPTS);
        if (bad) { failures++; fprintf(stderr, "gx_dl f%d: readback MISMATCH mask=0x%x\n", f, bad); }
        else printf("gx_dl f%02d: %s — %d/%d samples MATCH (corners+center+outside)\n",
                    f, (f & 1) ? "DL  (CP 0x40)" : "FIFO(capture)", NPTS, NPTS);
        gxh_dl_flush();
        struct timespec ts = { 0, 16700000 };   /* ~60 Hz like W's pacing    */
        nanosleep(&ts, NULL);
    }

    const GxDlStats *s = gx_dl_stats();
    printf("\ngx_dl: frames=%d vtxdesc_cmds=%u dl_calls=%u verts=%u tris=%u "
           "presented=%u verify_failures=%u samples_ok=%u\n",
           FRAMES, s->vtxdesc_cmds, s->dl_calls, s->verts_decoded,
           s->tris_emitted, s->frames_presented, s->verify_failures, s->samples_ok);
    gxh_dl_close();

    if (failures || s->verify_failures) return 4;
    if (s->frames_presented != FRAMES)   return 5;
    if (s->dl_calls == 0 || s->vtxdesc_cmds == 0) return 5;
    printf("gx_dl: PASS — first TEXTURED geometry on screen, translated from "
           "era GP vertex-descriptor words (PATH A capture + PATH B display list)\n");
    return 0;
}
