/* P2-BA V4 — begin_gx_frame glue prelude (C11 TU side; era home gjSystem.c,
 * a C unit — compiled as C exactly like ladder5_gxtexture_glue.c).
 *
 * DEVIATION LOGO-1: GXRenderModeObj stand-in — the era rmode is filled by
 * VIConfigure from the disc's video config (AB lane territory); this rung
 * pins the non-interlaced 640x480 shape the port's window is (W/AQ regime):
 * fbWidth=640 efbHeight=480 field_rendering=0, so the VERBATIM begin_gx_frame
 * body takes its else leg (GXSetViewport, not Jitter) — AP §5 drives the
 * check_progressive_ready()==0 / non-interlaced chain. The viewport seam
 * records its args; GXInvalidateVtxCache/GXInvalidateTexAll are W-style
 * count shims (host coherence needs no ops; TEXALL is why the texture upload
 * rides render_tick AFTER begin_gx_frame, LOGO-3). */
#ifndef LADDER6_GX_PRELUDE_H
#define LADDER6_GX_PRELUDE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct GXRenderModeObj {
    u32 fbWidth;
    u32 fbHeight;
    u32 efbWidth;
    u32 efbHeight;
    s32 field_rendering;
} GXRenderModeObj;

extern GXRenderModeObj g_host_rmode;
#define active_gx_render_mode (&g_host_rmode)

/* GX seams with arg capture (driver-asserted): viewport rect + near/far. */
extern double g_vp_left, g_vp_top, g_vp_w, g_vp_h, g_vp_n, g_vp_f;
void GXSetViewport(float l, float t, float w, float h, float n, float f);
void GXSetViewportJitter(float l, float t, float w, float h, float n, float f, u32 field);
void GXInvalidateVtxCache(void);
void GXInvalidateTexAll(void);
int VIGetNextField(void);   /* W gx_era.h:50 signature parity */
#ifdef __cplusplus
}
#endif

#endif /* LADDER6_GX_PRELUDE_H */
