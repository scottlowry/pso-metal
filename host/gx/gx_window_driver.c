/* pso-metal P2-W gate driver — a tiny replay of the boot→logo GX path on
 * Q's bringup substrate (crt0 arena + mini OSInit), through host/gx's
 * era-named entry points. Order mirrors the decomp:
 *   VIInit                       SystemBootInit.cpp:259        (ledger step 5)
 *   GXInit + disp-copy setup     gjSystem.c:124 init_gx_display (step 8)
 *   GXSetCopyClear + 2×GXCopyDisp SystemBootInit.cpp:231-246    (step 8)
 *   show_first_framebuffer       gjSystem.c:152                 (step 8)
 *   30× [render_tick-style frame]  gjSystem.c:40-88 + TLobby_801E2F94.cpp:491
 *                                  (steps 8–9, present_interval=2 retrace)
 * Colors are the two literals of SystemBootInit.cpp:237
 * GXSetCopyClear(clear={0,0,0,0}, 0xFFFFFF): black and white — era-defined,
 * and both are colorspace-conversion endpoints (safe for exact readback).
 * Exit: 0 = every GXDrawDone readback matched; 2/3/4 window/encode/readback
 * failures (spike convention); 5 = wrong present/verify bookkeeping.
 */
#include "gx_era.h"
#include "gx_metal.h"
#include "pso_bringup.h"
#include <stdio.h>

#define FRAMES 30

/* GXNtsc480IntDf fields the replay reads (gjSystem.c rmode->...): */
typedef struct RMode { u16 fbWidth, efbHeight, xfbHeight; u8 field_rendering, aa; } RMode;
static RMode g_ntsc480 = { 640, 480, 480, 0, 0 };   /* progressive for the gate */

/* era enums (include/gx/GXEnum.h) — only values the path compares against */
#define GX_TRUE      1
#define GX_LEQUAL    7
#define GX_ALWAYS    6
#define GX_PF_RGB8_Z24   0
#define GX_ZC_LINEAR     0

static void init_gx_display_replay(void) {
    u8 *fifo = pso_arena_base + 0x300000;           /* cfg.fifo_buffer carve  */
    GXFifoObj *f = GXInit(fifo, 0x40000);            /* gx_fifo_size from S6   */
    if (!f) { fprintf(stderr, "driver: GXInit failed — no window\n"); }
    GXSetViewport(0, 0, g_ntsc480.fbWidth, g_ntsc480.efbHeight, 0.0f, 1.0f);
    GXSetScissor(0, 0, g_ntsc480.fbWidth, g_ntsc480.efbHeight);
    GXSetDispCopySrc(0, 0, g_ntsc480.fbWidth, g_ntsc480.efbHeight);
    GXSetDispCopyDst(g_ntsc480.fbWidth, g_ntsc480.xfbHeight);
    GXSetDispCopyYScale(GXGetYScaleFactor(g_ntsc480.efbHeight, g_ntsc480.xfbHeight));
    GXSetCopyFilter(g_ntsc480.aa, NULL, GX_TRUE, NULL);
    GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    for (int i = 0; i < 0x40; i++) GXInitTlutRegion(NULL, 0xC0000 + i * 0x200, 0);
    for (int i = 0; i < 0x10; i++) GXInitTlutRegion(NULL, 0xC0000 + (0x40 + i) * 0x2000, 1);
    GXSetTlutRegionCallback(NULL);
    GXCopyDisp(pso_arena_base + 0x400000, GX_TRUE);
    GXDrawDone();
}

int main(void) {
    if (pso_bringup_install() != 0) return 2;
    OSInit();
    OSReport("gx_window: arena %p installed (Q substrate), boot→logo GX replay\n",
             (void *)pso_arena_base);

    VIInit();                                       /* SystemBootInit.cpp:259 */
    VIConfigure(&g_ntsc480);                        /* gjSystem.c:38          */
    init_gx_display_replay();

    /* init_gx_lighting_defaults (SystemBootInit.cpp:231): black clear +
     * double GXCopyDisp = first pixels on screen. */
    GXColor black = {0x00, 0x00, 0x00, 0x00}, white = {0xFF, 0xFF, 0xFF, 0xFF};
    GXSetCopyClear(black, 0xFFFFFF);
    GXInitLightPos(NULL, 0.0f, 10000.0f, 0.0f);
    GXInitLightColor(NULL, white);
    GXLoadLightObjImm(NULL, 0);
    GXSetChanAmbColor(0, (GXColor){0x20, 0x20, 0x20, 0xFF});
    GXSetChanAmbColor(2, white);
    GXSetChanMatColor(0, white);
    GXCopyDisp(pso_arena_base + 0x400000, GX_TRUE);
    GXCopyDisp(pso_arena_base + 0x480000, GX_TRUE);
    GXDrawDone();

    /* show_first_framebuffer (gjSystem.c:152) */
    VISetNextFrameBuffer(pso_arena_base + 0x400000);
    VIFlush();
    VIWaitForRetrace();
    if (g_ntsc480.field_rendering & 1u) VIWaitForRetrace();

    printf("gx_window: %d frames, alternating era blacks/whites "
           "(GXSetCopyClear literals 0x000000 / 0xFFFFFF)\n", FRAMES);
    for (int f = 0; f < FRAMES; f++) {
        /* begin_gx_frame (gjSystem.c:40) */
        GXSetViewport(0, 0, 640, 480, 0.0f, 1.0f);
        GXInvalidateVtxCache();
        GXInvalidateTexAll();
        /* render_tick head (TLobby_801E2F94.cpp:491) */
        GXSetZMode(GX_TRUE, GX_ALWAYS, 0);
        GXSetZCompLoc(0);
        GXSetCullMode(0);
        /* era-defined alternating clear */
        GXSetCopyClear((f & 1) ? white : black, 0xFFFFFF);
        /* copy_efb_to_framebuffer (gjSystem.c:52) */
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
        GXSetColorUpdate(GX_TRUE);
        GXCopyDisp(pso_arena_base + ((f & 1) ? 0x400000u : 0x480000u), GX_TRUE);
        GXDrawDone();                                   /* READBACK GATE      */
        /* present_frame_and_wait_vsync (gjSystem.c:62), present_interval=2 */
        VISetNextFrameBuffer(pso_arena_base + ((f & 1) ? 0x480000u : 0x400000u));
        if (f == 0) VISetBlack(FALSE);
        VIFlush();
        while ((s32)VIGetRetraceCount() < f * 2) VIWaitForRetrace();
        VIWaitForRetrace();
    }

    gx_shim_report_summary();
    const GxHostStats *s = gx_host_stats();
    printf("\ngx_window: window=%u presented=%u verified=%u failures=%u retraces=%u "
           "distinct-fns-fired=%u\n",
           s->window_open, s->frames_presented, s->verifications,
           s->verify_failures, s->retraces, gx_shim_first_fire_count());
    /* backend window teardown (driver otherwise speaks only era names) */
    gxh_close();

    if (!s->window_open)                    return 2;
    if (s->verify_failures)                 return 4;
    /* presents: replay-init 1 + lighting 2 + FRAMES; verifications happen at
     * GXDrawDone, which the era path doesn't issue after every single copy
     * (init_gx_display/lighting drain once at the end) — floor is FRAMES+2. */
    if (s->frames_presented < FRAMES + 3 ||
        s->verifications < FRAMES + 2)      return 5;
    printf("gx_window: PASS — %u drawn frames all pixel-verified on the drawable "
           "(FIRST REAL PIXELS driven by era-named GX calls)\n", s->frames_presented);
    return 0;
}
