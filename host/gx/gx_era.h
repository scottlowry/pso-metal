/* pso-metal P2-W GX→Metal window slice — era GX/VI call surface actually
 * invoked on the boot→logo path (ledger steps 5 + 8–9). Signatures mirror the
 * DECOMP call sites (git grep vs 6b38b5383, inventory in docs/p2-gx-window.md),
 * not the pristine SDK — where the decomp's local variant differs (e.g.
 * GXSetDispCopyDst takes 2 args in gjSystem.c), the decomp wins.
 * These are the port's backing for N's host/psosh_gx.h + host/psosh_vi.h
 * scaffold (those are declaration-only tags; this header is the compiling set).
 */
#ifndef PSO_GX_ERA_H
#define PSO_GX_ERA_H

#include "types.h"            /* bringup/include/types.h (shadow, -I host/bringup/include) */

#ifdef __cplusplus
extern "C" {
#endif

/* ---- era GX types (subset; enum values per include/gx/GXEnum.h layout) ---- */
typedef u8     GXBool;
typedef int    GXCompare;     /* GX_NEVER 0 … GX_LEQUAL 7 */
typedef int    GXPrimitive;
typedef int    GXVtxFmt;
typedef int    GXTlutFmt;
/* Host-side FIFO handle: the era openclose.c GXFifoObj is a ring of BE
 * command words; the port's "fifo" is the no-op staging ring of ledger
 * step 6 — fields exist so GXInit can return a real handle. */
struct GXFifoObj { void *base; u32 size; };
typedef struct GXFifoObj GXFifoObj;

/* include/gx/GXTypes.h GXColor — 4 raw bytes (BE bitfield order R,G,B,A). */
typedef struct GXColor { u8 r, g, b, a; } GXColor;

/* ---- REAL (Metal window / pacing / state-tracked) ------------------------ */
GXFifoObj *GXInit(void *base, u32 size);          /* gjSystem.c:125 */
void  GXSetViewport(f32 l, f32 t, f32 w, f32 h, f32 n, f32 f);
void  GXSetViewportJitter(f32 l, f32 t, f32 w, f32 h, f32 n, f32 f, u32 field);
void  GXSetScissor(u32 xl, u32 yl, u32 wd, u32 ht);
void  GXSetPixelFmt(int pix_fmt, int z_cmp);      /* SystemBootInit.cpp:325 */
f32   GXGetYScaleFactor(u16 efbHeight, u16 xfbHeight);
void  GXSetCopyClear(GXColor clear, u32 zvalue);  /* SystemBootInit.cpp:237 */
void  GXCopyDisp(void *dst, GXBool doGather);     /* EFB→XFB = clear+present */
void  GXDrawDone(void);                           /* FIFO drain + readback gate */
void  VIInit(void);                               /* SystemBootInit.cpp:259 */
void  VIConfigure(void *rmode);                   /* gjSystem.c:38 */
void  VISetBlack(BOOL enable);
void  VISetNextFrameBuffer(void *fb);
void  VIFlush(void);
void  VIWaitForRetrace(void);
u32   VIGetRetraceCount(void);
int   VIGetNextField(void);
int   VIGetTvFormat(void);
int   VIGetDTVStatus(void);                       /* TAdVIConfigGC.cpp:201 (decomp 0-arg) */
u32   VIPadFrameBufferWidth(void);

/* ---- COUNTED SHIMS (fire-and-record; semantics land in later lanes) ------ */
void  GXSetZMode(GXBool compareEnable, GXCompare func, GXBool updateEnable);
void  GXSetZCompLoc(GXBool beforeTex);
void  GXSetColorUpdate(GXBool colorUpdate);
void  GXSetCullMode(int cullMode);
void  GXSetAlphaCompare(int comp0, u8 ref0, int op, int comp1, u8 ref1);
void  GXBegin(GXPrimitive type, GXVtxFmt vtxfmt, u16 nverts);
void  GXSetLineWidth(u8 xfmt, u8 yfmt);           /* TLobby_801E2F94.cpp:169 */
void  GXSetChanAmbColor(int chanID, GXColor color);
void  GXSetChanMatColor(int chanID, GXColor color);
void  GXLoadLightObjImm(const void *lobj, int whichSlot);
void  GXInitLightPos(void *light, f32 x, f32 y, f32 z);
void  GXInitLightColor(void *light, GXColor color);
void  GXInitTlutRegion(void *tlutRegion, u32 addr, GXTlutFmt fmt);
void  GXSetTlutRegionCallback(void *cb);
void  GXSetDispCopySrc(u32 tlx, u32 tly, u32 wd, u32 ht);
void  GXSetDispCopyDst(u16 wd, u16 ht);           /* decomp-local 2-arg variant */
void  GXSetDispCopyYScale(f32 yscale);
void  GXSetCopyFilter(GXBool aa, const void *sample_pattern, GXBool vfilter,
                      const void *vfilter_coeffs);
void  GXInvalidateVtxCache(void);
void  GXInvalidateTexAll(void);
void  GXAdjustForOverscan(const void *in, void *out, u16 xl, u16 xr, u16 yt, u16 yb);

/* ---- host-GX introspection for the driver/gate --------------------------- */
typedef struct GxHostStats {
    u32 frames_presented;     /* GXCopyDisp commits that reached the GPU      */
    u32 verifications;        /* GXDrawDone corner readbacks performed        */
    u32 verify_failures;      /* readback mismatches (gate: must stay 0)      */
    u32 retraces;             /* VI retrace counter                           */
    u32 window_open;          /* 1 = NSWindow+CAMetalLayer live               */
} GxHostStats;
const GxHostStats *gx_host_stats(void);
u32  gx_shim_first_fire_count(void);              /* distinct fn with ≥1 call  */
void gx_shim_report_summary(void);                /* dump fn|kind|count table  */

#ifdef __cplusplus
}
#endif
#endif /* PSO_GX_ERA_H */
