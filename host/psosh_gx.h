/* Graphics pipe (gx headers) — pso-metal host re-host of Graphics pipe
 * Tag format: original SDK symbol | call-site count (docs/host-surface.md) |
 * effort (TRIVIAL/MODERATE/HARD/BLOCKING-GPU) | native backing note.
 * Featured = real dolphin signature; tail = PSOSH_TAG placeholder prototype. */
#ifndef PSOSH_GX_H
#define PSOSH_GX_H
#include "psosh_types.h"


/* #5 GXSetZMode — gx/GXPixel.h — 33 sites — BLOCKING-GPU
 * Depth test enable / compare fn / write enable.
 * Native backing: MTLDepthStencilState keyed by (compare_enable, func,
 * update_enable); GXCompare -> MTLCompareFunction table. TODO(P2): state
 * tracker caches key, binds lazily at draw time. */
void GXSetZMode(GXBool compare_enable, GXCompare func, GXBool update_enable);

/* #9 GXSetVtxDesc — gx/GXGeometry.h — 30 sites — BLOCKING-GPU
 * Enable one vertex attribute + its fetch mode (direct/index/none).
 * Native backing: MTLVertexDescriptor assembly + attribute-type table;
 * GXFetchMode direct vs index decides divisor/instance vs gather.
 * TODO(P2): per-GXVtxFmt descriptor cache, keyed by desc+fmt edits. */
void GXSetVtxDesc(GXAttr attr, GXAttrType type);

/* #11 GXSetZCompLoc — gx/GXPixel.h — 25 sites — BLOCKING-GPU
 * Early-Z vs post-combine Z compare.
 * Native backing: MTLDepthStencilState has no equivalent toggle; model by
 * picking between two pipeline variants. TODO(P2): key into pipeline cache. */
void GXSetZCompLoc(GXBool before_tex);

/* #12 GXBegin — gx/GXGeometry.h — 23 sites — BLOCKING-GPU
 * Start primitive stream into the vertex FIFO.
 * Native backing: opens a CPU-side vertex streamer replacing gx/GXVert.h
 * inline FIFO writers; close at GXEnd/GXFlush -> one drawPrimitives with a
 * pre-built (ring) vertex buffer. TODO(P2): the display-list path
 * (GXCallDisplayList) shares this streamer. */
void GXBegin(GXPrimitive type, GXVtxFmt vtxfmt, u16 nverts);

/* #13 GXSetVtxAttrFmt — gx/GXGeometry.h — 21 sites — BLOCKING-GPU
 * Type/count/scale(frac) of one attribute in the stream.
 * Native backing: feeds the same MTLVertexDescriptor cache key; GXCompType
 * fix16/s16/s8/u8 norms -> MTLVertexFormat + normalized flag. */
void GXSetVtxAttrFmt(GXVtxFmt vtxfmt, GXAttr attr, GXCompCnt cnt,
                     GXCompType type, u8 frac);

/* gx/GXPixel.h — 19 sites (tied-16) — BLOCKING-GPU
 * Blend type + src/dst factors + logic op.
 * Native backing: MTLRenderPipelineColorBlendDescriptor fields; GXLogicOp
 * rarely used — fall back to bitwise blend presets. TODO(P2): map
 * GXBlendFactor -> MTLBlendFactor 1:1 (it nearly is). */
void GXSetBlendMode(GXBlendMode type, GXBlendFactor src_factor,
                    GXBlendFactor dst_factor, GXLogicOp op);

/* gx/GXCull.h — 17 sites — BLOCKING-GPU
 * Scissor rect. Native backing: MTLScissorRect on encode. Cheap. */
void GXSetScissor(u32 left, u32 top, u32 wd, u32 ht);

/* gx/GXTev.h — 17 sites — BLOCKING-GPU
 * Convenience tev-stage preset (PASSTHROUGH/MODULATE/BLEND/...).
 * Native backing: expands into GXSetTevOrder+GXSetTevColorOp+GXSetTevAlphaOp;
 * each preset -> a baked fragment-pipeline variant in the pipeline cache. */
void GXSetTevOp(GXTevStageID id, GXTevMode mode);

/* boot-spine — gx/GXTransform.h — BLOCKING-GPU
 * Viewport window incl. depth range. Native backing: MTLViewport; note GX Y
 * origin is top-left like Metal but Y-scale sign encodes interlace. */
void GXSetViewport(f32 left, f32 top, f32 wd, f32 ht, f32 nearz, f32 farz);

/* boot-spine — gx/GXManage.h — BLOCKING-GPU (lane entry point)
 * Bring up the GX pipe with a FIFO buffer. Native backing: create Metal
 * device/layer, allocate the CPU vertex-streamer staging buffer; returned
 * GXFifoObj points at it. TODO(P2): first milestone = logo frame through
 * this + GXBegin + GXSetViewport + setCopyClear. */
GXFifoObj *GXInit(void *base, u32 size);

/* ---- long tail: placeholder prototypes; exact signatures land in P2 ---- */
PSOSH_TAG(GXSetTevOrder, 15, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXInitLightDir, 13, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetNumTexGens, 12, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXClearVtxDesc, 11, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetChanCtrl, 10, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetNumTevStages, 10, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetTexCoordGen2, 10, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXCopyTex, 9, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXLoadPosMtxImm, 9, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetChanMatColor, 9, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetScissorBoxOffset, 9, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXPixModeSync, 8, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetCopyClear, 8, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetTexCopyDst, 8, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetTexCopySrc, 8, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXPosition3f32, 7, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetNumChans, 7, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetAlphaCompare, 6, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXLoadNrmMtxImm, 5, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXLoadTexMtxImm, 5, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetNumIndStages, 5, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetTevDirect, 5, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXColor1u32, 4, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXCopyDisp, 4, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXDrawDone, 4, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXNormal3f32, 4, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetCopyFilter, 4, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetCurrentMtx, 4, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetProjection, 4, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXInitTexObj, 3, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXLoadTexObj, 3, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetChanAmbColor, 3, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetCullMode, 3, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetIndTexMtx, 3, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetPixelFmt, 3, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetTevIndWarp, 3, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetTevSwapMode, 3, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXCallDisplayList, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXGetProjectionv, 2, "MODERATE", "Metal/CPU: read-back from host state mirror")
PSOSH_TAG(GXGetTexBufferSize, 2, "MODERATE", "Metal/CPU: read-back from host state mirror")
PSOSH_TAG(GXGetViewportv, 2, "MODERATE", "Metal/CPU: read-back from host state mirror")
PSOSH_TAG(GXInitFogAdjTable, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXInitLightColor, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXInitLightDistAttn, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXInitLightPos, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXInitLightSpot, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXInitTexObjCI, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXInitTlutRegion, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXLoadLightObjImm, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXProject, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetFogRangeAdj, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetIndTexCoordScale, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetIndTexOrder, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetLineWidth, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetTevSwapModeTable, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXTexCoord2s16, 2, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXAdjustForOverscan, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXGetYScaleFactor, 1, "MODERATE", "Metal/CPU: read-back from host state mirror")
PSOSH_TAG(GXInitLightAttnK, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXInitTexObjLOD, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXInitTlutObj, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXInvalidateTexAll, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXInvalidateVtxCache, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXLoadTlut, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXPosition3s16, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetArray, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetColorUpdate, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetDispCopyDst, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetDispCopySrc, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetDispCopyYScale, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetFog, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetTevColorIn, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetTevColorOp, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetTlutRegionCallback, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXSetViewportJitter, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")
PSOSH_TAG(GXTexCoord2f32, 1, "BLOCKING-GPU", "Metal: GX fixed-function state -> MTLRenderPipelineState/DepthStencil/VertexDescriptor")

#endif /* PSOSH_GX_H */
