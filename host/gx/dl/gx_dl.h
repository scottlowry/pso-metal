/* pso-metal P2-Z — display-list slice: era GP FIFO-lite + vertex-format
 * plumbing + Metal translation of ONE textured quad.
 *
 * Era sources (decomp 6b38b5383, inventory in docs/p2-gx-dl1.md):
 *   libs/dolphin/src/gx/GXGeometry.c:37  GXBegin → BE32 word (vtxfmt|type)<<24 | nverts<<8
 *   libs/dolphin/src/gx/GXDisplayList.c:74 GXCallDisplayList → cmd 0x40 + list + nbytes
 *       (era asserts: nbytes & 0x1F == 0, list & 0x1F == 0)
 *   src/pso/ui/window/TWindowSubStatusDisp_803DEE6C.cpp:43 draw_textured_quad_at_depth
 *       — the exact quad recipe this port replays: GXBegin((GXPrimitive)0x80,
 *         (GXVtxFmt)4, 4) + FIFO_POSITION3F32 / FIFO_COLOR1U32 / FIFO_TEXCOORD2F32
 *   src/pso/render/CameraCapture.cpp:278-283 — VTXFMT4 := POS XYZ F32,
 *         CLR0 RGBA RGBA8, TEX0 ST F32 (GX_DIRECT)
 *   src/pso/render/ScreenPrimitiveDraw.cpp:124 — GXSetTevOrder(stage0,
 *         TEXCOORD0, TEXMAP0, COLOR0A0) + GX_MODULATE → our fixed-function key.
 * All wire words are BIG-ENDIAN, exactly like the era FIFO/display-list bytes;
 * the host is LE, so every write goes through the bswap helpers here.
 */
#ifndef PSO_GX_DL_H
#define PSO_GX_DL_H

#include "gx_era.h"   /* W's era type surface (u8/u32/f32, GXColor, enums) */

#ifdef __cplusplus
extern "C" {
#endif

/* ---- era GP/CP command bytes (values from the decomp's own SDK sources) -- */
#define GX_DL_CP_CALL        0x40u   /* GXCallDisplayList jump            */
#define GX_DL_CLRPOS         0x98u   /* __GXSendFlushPrim reset+pad flush */
#define GX_DL_QUADS          0x80u   /* GXPrimitive (GXEnum.h:79)         */
#define GX_DL_TRIANGLES      0x90u
#define GX_DL_TRI_STRIP      0x98u
#define GX_DL_TRI_FAN        0xA0u
#define GX_VT_FMT4           4u
#define GX_DL_END            0x00000000u /* list terminator we accept (CP nop) */

/* GXAttr ids / GXAttrType / comp types (subset of include/gx/GXEnum.h) */
#define GX_DL_VA_POS         0x09
#define GX_DL_VA_NRM         0x0A
#define GX_DL_VA_CLR0        0x0D
#define GX_DL_VA_TEX0        0x17
#define GX_DL_DIRECT         1
#define GX_DL_INDEX8         2
#define GX_DL_INDEX16        3

/* ---- state + FIFO-lite ----------------------------------------------------
 * One global FIFO-lite ring of BE32 words living in Q's arena (era
 * cfg.fifo_buffer analogue). GXBegin + the inline writers capture into it;
 * gx_dl_flush() replays it through the same decoder GXCallDisplayList uses.
 */
void  gx_dl_fifo_reset(void);                    /* rewind capture + batch     */
void  gx_dl_set_vtx_desc(u32 attr, u32 type);    /* GXSetVtxDesc mirror        */
void  gx_dl_set_vtx_attr_fmt(u32 vtxfmt, u32 attr, u32 cnt, u32 comp, u8 frac);
void  gx_dl_begin(u32 prim_type, u32 vtxfmt, u16 nverts); /* writes vtxdesc cmd */
void  gx_dl_fifo_pos3f32(f32 x, f32 y, f32 z);   /* data words → FIFO-lite     */
void  gx_dl_fifo_clr1u32(u32 rgba);              /* BE RGBA8 packed            */
void  gx_dl_fifo_tex2f32(f32 s, f32 t);
void  gx_dl_end(void);                           /* era: no-op inline          */
u32   gx_dl_fifo_len(void);                      /* BE32 words captured        */

void  gx_dl_fifo_call_display_list(const void *list, u32 nbytes);
/* CP replay: execute a display list of BE32 words (era asserts enforced). */
void  gx_dl_call_display_list(const void *list, u32 nbytes);
/* Drain the capture FIFO through the same decoder, then draw+clear the batch */
void  gx_dl_flush(void);

/* Texture supply (host-side stand-in for GXSetTexObj + GXLoadTexObj):
 * bytes = RGBA8, row-major, top row first, w*h*4 long. */
void  gx_dl_set_texture(unsigned w, unsigned h, const unsigned char *rgba8);

/* viewport mirror for CPU-side GP→NDC transform (called with era values) */
void  gx_dl_viewport(f32 l, f32 t, f32 w, f32 h);
/* clear color for the frame (GXSetCopyClear analogue on this path) */
void  gx_dl_clear_color(GXColor c);

/* ---- host Metal backend (gx_dl_metal.m; W's gx_metal.m pattern, extended
 *      with a textured-quad pipeline — host/gx top level is W-closed, so the
 *      proven window/readback code is duplicated here, not refactored). ---- */
int   gxh_dl_open(unsigned w, unsigned h);
void  gxh_dl_close(void);
void  gxh_dl_set_texture(unsigned w, unsigned h, const unsigned char *rgba8);
/* draw the accumulated triangle batch over the clear, present. 0 ok. */
int   gxh_dl_draw(const float *vc, unsigned nverts, /* 8 floats/v: ndc2 clr4 uv2 */
                  double r, double g, double b, double a);
/* wait GPU; sample (x,y) points, compare to expected rgba (±2/255).
 * returns bitmask of failed points (0 = all match). */
int   gxh_dl_verify(const unsigned *xs, const unsigned *ys, const unsigned *want_rgba,
                    int npoints, int frame_no);
void  gxh_dl_flush(void);                        /* CA flush + event pump      */
void  gxh_dl_dump(const char *path);             /* P2 integrator: last drawable -> PPM */
/* P2-SEQ: last drawable -> PNG (NSBitmapImageRep, no new deps). 0 ok. */
int   gxh_dl_dump_png(const char *path);
/* P2-AQ diagnostics: raw point sampler of the last drawable (packed RGBA)
 * and a CPU-side roundtrip print of the live GPU texture. */
int   gxh_dl_sample(const unsigned *xs, const unsigned *ys,
                    unsigned *out_rgba, int n);
void  gxh_dl_tex_roundtrip(unsigned w, unsigned h);

/* ---- lane stats for the gate / report ------------------------------------- */
typedef struct GxDlStats {
    u32 vtxdesc_cmds;      /* vertex-descriptor commands decoded (GXBegin)     */
    u32 clrpos_flushes;    /* 0x98 resets seen                                 */
    u32 dl_calls;          /* CP 0x40 display-list calls executed              */
    u32 verts_decoded;     /* attribute triples sliced off the wire            */
    u32 tris_emitted;      /* triangles after primitive expansion              */
    u32 frames_presented;  /* gxh_dl_draw commits that reached the GPU         */
    u32 verify_failures;   /* readback mismatches (gate: must stay 0)          */
    u32 samples_ok;        /* total matching sample points                     */
} GxDlStats;
const GxDlStats *gx_dl_stats(void);

/* boot (call after pso_bringup_install) + present/verify glue */
int   gx_dl_boot(void);
int   gx_dl_present(void);
void  gx_dl_note_verify(int bad, int total);

#ifdef __cplusplus
}
#endif
#endif /* PSO_GX_DL_H */
