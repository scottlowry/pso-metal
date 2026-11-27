/* ladder5 tex-upload prelude — prepended to the VERBATIM GinjaTextureHeader
 * slice TU (generated ladder5_tex_glue.cpp = GinjaTextureHeader.cpp lines
 * 62..105: init_ginja_texture_header :62-84, get_texture_buffer_size :86-94,
 * alloc_buffer_from_heap :96-105; era homes 0x803DB0AC / 0x803DB15C /
 * 0x803DB1B4 — port map docs/p2-an4-triage.md §4).
 *
 * WHY A PRELUDE AND NOT THE REAL HEADERS: GinjaTextureHeader.cpp includes
 * <dolphin/gx/GXTexture.h> + <dolphin/os/OSAlloc.h> + GinjaMaterialState.h
 * (which drags pso/system/NinjaTypes.h — the AR lane proved NJS pair is
 * stitch-pending). Deviation D4 (docs/p2-bringup-1.md) keeps the era clib/
 * + dolphin include trees off executing TUs. So this prelude carries the
 * minimal era-faithful bindings, each cited to its era definition site and
 * pinned against the PPC stores in build/GPOE8P/asm/pso/render/
 * GinjaTextureHeader.s (.fn blocks :133-182/:186-211/:214-239).
 */
#ifndef LADDER5_TEX_PRELUDE_H
#define LADDER5_TEX_PRELUDE_H

#include "ladder5_gx_defs.h"   /* types.h, GXTexFmt/GXBool, assert no-ops */
#include <stddef.h>

/* ---- era texel-header struct ----
 * Mirror of include/pso/render/GinjaMaterialState.h:13-21 (the REAL era
 * struct; the header itself is not included — NJS chain above). Every
 * offset below is pinned by an era store instruction, NOT by the C field
 * order alone:
 *   +0x00 format  stw r0,0x0(r30)   0x803DB0D0 (r0 = clrlwi r6,24 = fmt&0xFF :803DB0CC)
 *   +0x0C width   stw r4,0xc(r30)   0x803DB0D4
 *   +0x10 height  stw r5,0x10(r30)  0x803DB0D8
 *   +0x14 size    stw r3,0x14(r30)  0x803DB13C (r3 from bl :803DB138 get_texture_buffer_size)
 *   +0x18 flags   stw r0,0x18(r30)  0x803DB0F8 zero-init, or-back 0x803DB128-:803DB130
 *   +0x1C pixels  stw r3,0x1c(r30)  0x803DB1EC (OSAllocFromHeap result)
 * +0x04/+0x08 reserved: NO era store exists in any of the four fns — the
 * 0xCC-sentinel golden (E1c) proves the era writer touches exactly 6 fields. */
typedef struct GinjaTextureHeader {
    u32  format;      /* +0x00 (era +0x00) */
    u32  reserved04;  /* +0x04 (era +0x04) */
    u32  reserved08;  /* +0x08 (era +0x08) */
    u32  width;       /* +0x0C (era +0x0C) */
    u32  height;      /* +0x10 (era +0x10) */
    u32  size;        /* +0x14 (era +0x14) GXGetTexBufferSize of the image */
    u32  flags;       /* +0x18 (era +0x18) 0x80000000 mip, 0x8000 colour-indexed */
    u32 *pixels;      /* +0x20 host — DEVIATION AU-5: era +0x1C (LP64 pointer
                       * widening, exactly the V1 precedent of osalloc_era.c
                       * Cell{prev,next}; the era store `stw r3,0x1c(r30)`
                       * @0x803DB1EC pins the FIELD SLOT — 8th field, first
                       * pointer — every scalar keeps its era offset) */
} GinjaTextureHeader;
static_assert(sizeof(GinjaTextureHeader) == 0x28, "7 u32 + pad + 8B ptr (era 0x20 under ILP32; AU-5)");
static_assert(offsetof(GinjaTextureHeader, format)  == 0x00, "stw 0x0(r30) @0x803DB0D0");
static_assert(offsetof(GinjaTextureHeader, width)   == 0x0C, "stw 0xc(r30) @0x803DB0D4");
static_assert(offsetof(GinjaTextureHeader, height)  == 0x10, "stw 0x10(r30) @0x803DB0D8");
static_assert(offsetof(GinjaTextureHeader, size)    == 0x14, "stw 0x14(r30) @0x803DB13C");
static_assert(offsetof(GinjaTextureHeader, flags)   == 0x18, "stw 0x18(r30) @0x803DB0F8");
static_assert(offsetof(GinjaTextureHeader, pixels)  == 0x20, "AU-5: era slot 0x1C `stw r3,0x1c(r30)` @0x803DB1EC");

/* Era .sbss global — defined (plain global, this TU's owner is the seam TU
 * ladder5_seam.cpp) at RenderSbssGlobals.cpp:12 / RenderSbssGlobals.h:21.
 * alloc_buffer_from_heap reads it via lwz locked_cache_heap_handle@sda21
 * (0x803DB1D0) — the heap-id argument of OSAllocFromHeap. */
extern u32 locked_cache_heap_handle;

/* The era OSAlloc entry point (C TU osalloc_era.c; header contract
 * pso_bringup.h:70). */
extern "C" void *OSAllocFromHeap(int heap, u32 size);

/* GX SDK sizer — C TU ladder5_gxtexture_glue.c (era home 0x8038BE90; the
 * era bl 0x803DB1A0 `4B FB 0C F1` resolves here). */
extern "C" u32 GXGetTexBufferSize(u16 width, u16 height, u32 format, u8 mipmap, u8 max_lod);

/* Y-lane era mover (verbatim TLobby slice, move_data_glue.o). */
void move_data(void *dst, const void *src, s32 n);

/* The four ladder5 chain names (era-accurate signatures; the GVR caller
 * shape `s32 init_texture_header_and_alloc(u8*, s32, s32, s32)` is the
 * era CALLER's own declaration, text_803D96A8.cpp:23 — see AU-1). */
s32 init_ginja_texture_header(GinjaTextureHeader *header, u32 width, u32 height, u32 format);
u32 get_texture_buffer_size(GinjaTextureHeader *header);
s32 alloc_buffer_from_heap(GinjaTextureHeader *header);
s32 init_texture_header_and_alloc(u8 *p, s32 width, s32 height, s32 format);

#endif /* LADDER5_TEX_PRELUDE_H */
