/* pso-metal P2-W — the port's backing for the boot→logo GX/VI call surface.
 * Two tiers, per the P2-W method:
 *   REAL  — drives the Metal window (gx_metal.m) or the retrace pacing /
 *           framebuffer state machine. Ledger steps 5 (VIInit+window+retrace)
 *           and 8–9 (GXInit bring-up, clear color, GXCopyDisp→present, pacing).
 *   SHIM  — every other enumerated called fn (docs/gx inventory): printf on
 *           first fire, count thereafter. The count log is lane evidence for
 *           the display-list/pipeline translator (steps 10+).
 * Era semantics kept where cheap (state mirrors), dropped where the lane will
 * redo them properly (TODO(P2) below). All names/signatures = gx_era.h.
 */
#include "gx_era.h"
#include "gx_metal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PSO_GX_W 640
#define PSO_GX_H 480
/* NTSC retrace period — GXNtsc480IntDf interlaced field rate 59.94 Hz. */
#define RETRACE_PERIOD_NS 16684000LL

/* ---------------- shim registry: first-fire printf + total count ---------- */
#define MAX_FNS 64
typedef struct { const char *name; int real; u32 count; int logged; } FnRec;
static FnRec g_fns[MAX_FNS];
static int g_nfns;

static FnRec *rec(const char *name, int real) {
    for (int i = 0; i < g_nfns; i++)
        if (strcmp(g_fns[i].name, name) == 0) return &g_fns[i];
    if (g_nfns == MAX_FNS) { fprintf(stderr, "gx_shims: registry full\n"); abort(); }
    g_fns[g_nfns] = (FnRec){ name, real, 0, 0 };
    return &g_fns[g_nfns++];
}
/* Log first fire (with formatted args), silent-count the rest. */
#define ENTER(kind, fmt, ...) do {                                           \
    FnRec *_r = rec(__func__, kind); _r->count++;                            \
    if (!_r->logged) { _r->logged = 1;                                       \
        printf("[GX-%s] %s(" fmt ") fired\n",                                \
               kind == 1 ? "REAL" : "SHIM", __func__, __VA_ARGS__); }        \
} while (0)
#define ENTER0(kind) do {                                                    \
    FnRec *_r = rec(__func__, kind); _r->count++;                             \
    if (!_r->logged) { _r->logged = 1;                                        \
        printf("[GX-%s] %s() fired\n",                                       \
               kind == 1 ? "REAL" : "SHIM", __func__); }                      \
} while (0)

/* ---------------- host GX state mirror ------------------------------------ */
static GXFifoObj  g_fifo_storage;
static GXColor    g_clear = {0, 0, 0, 0};        /* GXSetCopyClear mirror      */
static u32        g_clear_z;
static void      *g_copy_disp_dst;
static f32        g_viewport[6];
static u32        g_scissor[4];
static int        g_pixel_fmt, g_z_cmp;
static void      *g_framebuffers[2];
static int        g_fb_next, g_black = 1;
static u32        g_retrace;
static u64        g_retrace_deadline;           /* monotonic ns pacing        */
static GxHostStats g_stats;

static u64 now_ns(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)ts.tv_sec * 1000000000ULL + (u64)ts.tv_nsec;
}
static void sleep_ns(u64 ns) {
    struct timespec ts = { (time_t)(ns / 1000000000ULL), (long)(ns % 1000000000ULL) };
    nanosleep(&ts, NULL);
}

const GxHostStats *gx_host_stats(void) { return &g_stats; }
u32 gx_shim_first_fire_count(void) {
    u32 n = 0;
    for (int i = 0; i < g_nfns; i++) if (g_fns[i].count) n++;
    return n;
}
void gx_shim_report_summary(void) {
    printf("\n=== P2-W called-fn firing table (%d distinct fns observed) ===\n", g_nfns);
    printf("%-26s %-6s %s\n", "function", "kind", "calls");
    for (int i = 0; i < g_nfns; i++)
        printf("%-26s %-6s %u\n", g_fns[i].name,
               g_fns[i].real == 1 ? "REAL" : "SHIM", g_fns[i].count);
    u32 real = 0, shim = 0;
    for (int i = 0; i < g_nfns; i++)
        g_fns[i].real == 1 ? real++ : shim++;
    printf("totals: %u REAL-backed, %u count-shims\n", real, shim);
}

/* ==================== REAL: window bring-up (steps 5+8) =================== */
GXFifoObj *GXInit(void *base, u32 size) {
    ENTER(1, "fifo=%p size=0x%x host=%ux%u", base, size,
          (unsigned)PSO_GX_W, (unsigned)PSO_GX_H);
    int rc = gxh_open(PSO_GX_W, PSO_GX_H);
    if (rc != 0) { fprintf(stderr, "GXInit: window open failed rc=%d\n", rc); g_stats.window_open = 0; return NULL; }
    g_stats.window_open = 1;
    /* era contract: return the active FIFO handle. Port-side "fifo" is the
     * no-op ring of ledger step 6; vertex semantics = later lanes (GXBegin). */
    return &g_fifo_storage;
}
void VIInit(void) { ENTER0(1); g_retrace_deadline = now_ns(); }
void VIConfigure(void *rmode) {
    /* Ledger step 8: GXNtsc480IntDf → hardcoded 640x480 progressive here.
     * TODO(P2): real rmode decode once display-list lane feeds TV modes. */
    ENTER(1, "rmode=%p -> 640x480 prog (hardcoded)", rmode);
}

/* ==================== REAL: clear color + present path (step 8) =========== */
void GXSetCopyClear(GXColor clear, u32 zvalue) {
    ENTER(1, "clear=(%u,%u,%u,%u) z=0x%x", clear.r, clear.g, clear.b, clear.a, zvalue);
    g_clear = clear; g_clear_z = zvalue;
}
void GXCopyDisp(void *dst, GXBool doGather) {
    ENTER(1, "dst=%p gather=%d (EFB->XFB = clear+present drawable)", dst, doGather);
    g_copy_disp_dst = dst;
    int rc = gxh_present_clear(g_clear.r / 255.0, g_clear.g / 255.0,
                               g_clear.b / 255.0, g_clear.a / 255.0);
    if (rc != 0) fprintf(stderr, "GXCopyDisp: present failed rc=%d\n", rc);
    else g_stats.frames_presented++;
}
void GXDrawDone(void) {
    ENTER0(1);
    int rc = gxh_wait_verify(g_clear.r / 255.0, g_clear.g / 255.0,
                             g_clear.b / 255.0, (int)g_stats.frames_presented);
    g_stats.verifications++;
    if (rc != 0) {
        g_stats.verify_failures++;
        fprintf(stderr, "GXDrawDone: READBACK ASSERT FAILED rc=%d (frame %u)\n",
                rc, g_stats.frames_presented);
    }
}

/* ==================== REAL: pacing state machine (step 9) ================= */
void VISetNextFrameBuffer(void *fb) {
    ENTER(1, "fb=%p", fb);
    g_framebuffers[g_fb_next & 1] = fb;  /* era double-buffer ping-pong mirror */
    g_fb_next++;
}
void VISetBlack(BOOL enable) { ENTER(1, "black=%d", enable); g_black = enable; }
void VIFlush(void) {
    ENTER0(1);
    gxh_flush();                          /* CA commit + event pump = XFB out  */
}
void VIWaitForRetrace(void) {
    ENTER0(1);
    u64 now = now_ns();
    if (g_retrace_deadline > now) sleep_ns(g_retrace_deadline - now);
    g_retrace_deadline = now_ns() + RETRACE_PERIOD_NS;
    g_retrace++; g_stats.retraces++;
}
u32 VIGetRetraceCount(void) { ENTER0(1); return g_retrace; }
int VIGetNextField(void) { ENTER0(1); return (int)(g_retrace & 1); } /* interlace mirror */
int VIGetTvFormat(void) { ENTER0(1); return 1; }  /* VI_TVNTSC */
int VIGetDTVStatus(void) { ENTER0(1); return 0; } /* analog: era consumers branch on this */
u32 VIPadFrameBufferWidth(void) { ENTER0(1); return PSO_GX_W; } /* already 32-pixel aligned */

/* ==================== REAL-cheap: state mirrors (tracked, no GPU effect == */
void GXSetViewport(f32 l, f32 t, f32 w, f32 h, f32 n, f32 f) {
    ENTER(1, "%.0f,%.0f,%.0f,%.0f,%.1f,%.1f", l, t, w, h, n, f);
    g_viewport[0]=l; g_viewport[1]=t; g_viewport[2]=w; g_viewport[3]=h; g_viewport[4]=n; g_viewport[5]=f;
}
void GXSetViewportJitter(f32 l, f32 t, f32 w, f32 h, f32 n, f32 f, u32 field) {
    ENTER(1, "%.0f,%.0f,%.0f,%.0f,%.1f,%.1f field=%u", l, t, w, h, n, f, field);
    GXSetViewport(l, t, w, h, n, f);
}
void GXSetScissor(u32 xl, u32 yl, u32 wd, u32 ht) {
    ENTER(1, "%u,%u,%u,%u", xl, yl, wd, ht);
    g_scissor[0]=xl; g_scissor[1]=yl; g_scissor[2]=wd; g_scissor[3]=ht;
}
void GXSetPixelFmt(int pix_fmt, int z_cmp) {
    ENTER(1, "pix=%d zcmp=%d (EFB format choice — host stays BGRA8Unorm)", pix_fmt, z_cmp);
    g_pixel_fmt = pix_fmt; g_z_cmp = z_cmp;
}
f32 GXGetYScaleFactor(u16 efbHeight, u16 xfbHeight) {
    /* era table-driven yscale; progressive 480→480 = 1.0 (gx/GXDispCopy.c) */
    ENTER(1, "efb=%u xfb=%u", efbHeight, xfbHeight);
    return efbHeight == xfbHeight ? 1.0f : (f32)xfbHeight / (f32)efbHeight;
}

/* ==================== COUNTED SHIMS (lane evidence, no semantics yet) ==== */
void GXSetZMode(GXBool ce, GXCompare func, GXBool ue) { ENTER(2, "%d,cmp=%d,%d", ce, func, ue); }
void GXSetZCompLoc(GXBool beforeTex) { ENTER(2, "%d", beforeTex); }
void GXSetColorUpdate(GXBool cu) { ENTER(2, "%d", cu); }
void GXSetCullMode(int mode) { ENTER(2, "%d", mode); }
void GXSetAlphaCompare(int c0, u8 r0, int op, int c1, u8 r1) { ENTER(2, "%d,%u,%d,%d,%u", c0, r0, op, c1, r1); }
void GXBegin(GXPrimitive type, GXVtxFmt fmt, u16 nverts) { ENTER(2, "prim=%d fmt=%d n=%u (vertex FIFO = display-list lane)", type, fmt, nverts); }
void GXSetLineWidth(u8 xfmt, u8 yfmt) { ENTER(2, "%u,%u", xfmt, yfmt); }
void GXSetChanAmbColor(int id, GXColor c) { ENTER(2, "chan=%d (%u,%u,%u,%u)", id, c.r, c.g, c.b, c.a); }
void GXSetChanMatColor(int id, GXColor c) { ENTER(2, "chan=%d (%u,%u,%u,%u)", id, c.r, c.g, c.b, c.a); }
void GXLoadLightObjImm(const void *l, int slot) { ENTER(2, "lobj=%p slot=%d (BE bitpacked hw struct)", l, slot); }
void GXInitLightPos(void *l, f32 x, f32 y, f32 z) { ENTER(2, "%p %.1f,%.1f,%.1f", l, x, y, z); }
void GXInitLightColor(void *l, GXColor c) { ENTER(2, "%p (%u,%u,%u,%u)", l, c.r, c.g, c.b, c.a); }
void GXInitTlutRegion(void *r, u32 addr, GXTlutFmt fmt) { ENTER(2, "%p addr=0x%x fmt=%d", r, addr, fmt); }
void GXSetTlutRegionCallback(void *cb) { ENTER(2, "cb=%p", cb); }
void GXSetDispCopySrc(u32 tlx, u32 tly, u32 wd, u32 ht) { ENTER(2, "%u,%u,%u,%u", tlx, tly, wd, ht); }
void GXSetDispCopyDst(u16 wd, u16 ht) { ENTER(2, "%u,%u", wd, ht); }
void GXSetDispCopyYScale(f32 ys) { ENTER(2, "%.4f", ys); }
void GXSetCopyFilter(GXBool aa, const void *sp, GXBool vf, const void *vc) { ENTER(2, "aa=%d sp=%p vf=%d vc=%p", aa, sp, vf, vc); }
void GXInvalidateVtxCache(void) { ENTER0(2); }
void GXInvalidateTexAll(void) { ENTER0(2); }
void GXAdjustForOverscan(const void *in, void *out, u16 xl, u16 xr, u16 yt, u16 yb) { ENTER(2, "in=%p out=%p %u,%u,%u,%u", in, out, xl, xr, yt, yb); }
