/* ladder5 shared GX definitions — included by BOTH generated glues:
 *   ladder5_gxtexture_glue.c  (era C: __GXGetTexTileShift + GXGetTexBufferSize
 *                               verbatim slices, libs/dolphin/src/gx/GXTexture.c
 *                               :53-96/:98-139, era home 0x8038BE90 per
 *                               get_texture_buffer_size's bl 4B FB 0C F1
 *                               @ .s 0x803DB1A0 -> 0x8038BE90)
 *   ladder5_tex_prelude.h     (C++: GinjaTextureHeader.cpp :62-105 slices)
 * Must stay valid C11 AND C++17 (types.h is era-shared; only preprocessor,
 * plain enum, typedefs here).
 *
 * DEVIATION AU-2 — ASSERTMSGLINEV is the RETAIL-STRIPPED no-op. Era ground
 * truth, not convenience: build/GPOE8P/asm/gx/GXTexture.s .fn
 * GXGetTexBufferSize (0x8038BE90..0x8038BFEC, 0x15C bytes) is a LEAF — the
 * only control transfers are jumptable_804F50A8 bctr + local branches, no
 * bl __assert/OSReport anywhere. The C asserts (0x1AB width>1024, 0x1AC
 * height>1024, 0x1BE/0x1C1 power-of-2, 0x19B invalid format) generated ZERO
 * era instructions: retail behaviour is silent compute on every "assert
 * failure" — pinned by goldens G4/G5 (ladder5_test.cpp).
 *
 * DEVIATION AU-3 — __cntlzw(x) -> __builtin_clz((u32)x): PPC count-leading-
 * zeros intrinsic (GXTexture.c:115/119 host binding). Era .s shows the
 * RESULT of the nx/ny pow2 pre-compute was dead-code eliminated with the
 * asserts gone (.fn body has no cntlzw at all), so this only feeds asserts
 * that do not exist; semantics at the seam are unaffected.
 */
#ifndef LADDER5_GX_DEFS_H
#define LADDER5_GX_DEFS_H

#include "types.h"

/* GXTexFmt — VERBATIM mirror of the decomp SDK enum,
 * libs/dolphin/include/dolphin/gx/GXEnum.h:143-172 (TARGET_PC section,
 * _GX_TF_CTF 0x20 / _GX_TF_ZTF 0x10 at :141-142). The bpp/tile-shift table
 * that __GXGetTexTileShift switches on, and the raw .s evidence for the two
 * literals GXGetTexBufferSize tests for 64-byte tiles:
 *   0x8038BEE4 cmplwi r5,0x6  / :BEEC cmplwi r5,0x16 -> li r5,0x40 arms  =>
 *   GX_TF_RGBA8 == 6 and GX_TF_Z24X8 == 0x16 — pinned below so enum drift
 *   cannot silently change the tileBytes regime.
 * Colour-indexed identity of the fixture format: GXCITexFmt GX_TF_C8 = 0x9
 * (GXEnum.h:174-178) — the AN-4 "PAL8 = 1 byte/texel" assumption is now
 * era-table proven, not assumed (golden E2). */
#define _GX_TF_CTF 0x20
#define _GX_TF_ZTF 0x10

typedef enum _GXTexFmt {
    GX_TF_I4 = 0x0,
    GX_TF_I8 = 0x1,
    GX_TF_IA4 = 0x2,
    GX_TF_IA8 = 0x3,
    GX_TF_RGB565 = 0x4,
    GX_TF_RGB5A3 = 0x5,
    GX_TF_RGBA8 = 0x6,
    GX_TF_CMPR = 0xE,

    GX_CTF_R4 = 0x0 | _GX_TF_CTF,
    GX_CTF_RA4 = 0x2 | _GX_TF_CTF,
    GX_CTF_RA8 = 0x3 | _GX_TF_CTF,
    GX_CTF_YUVA8 = 0x6 | _GX_TF_CTF,
    GX_CTF_A8 = 0x7 | _GX_TF_CTF,
    GX_CTF_R8 = 0x8 | _GX_TF_CTF,
    GX_CTF_G8 = 0x9 | _GX_TF_CTF,
    GX_CTF_B8 = 0xA | _GX_TF_CTF,
    GX_CTF_RG8 = 0xB | _GX_TF_CTF,
    GX_CTF_GB8 = 0xC | _GX_TF_CTF,

    GX_TF_Z8 = 0x1 | _GX_TF_ZTF,
    GX_TF_Z16 = 0x3 | _GX_TF_ZTF,
    GX_TF_Z24X8 = 0x6 | _GX_TF_ZTF,

    GX_CTF_Z4 = 0x0 | _GX_TF_ZTF | _GX_TF_CTF,
    GX_CTF_Z8M = 0x9 | _GX_TF_ZTF | _GX_TF_CTF,
    GX_CTF_Z8L = 0xA | _GX_TF_ZTF | _GX_TF_CTF,
    GX_CTF_Z16L = 0xC | _GX_TF_ZTF | _GX_TF_CTF,

    GX_TF_A8 = GX_CTF_A8,
} GXTexFmt;

typedef enum _GXCITexFmt {
    GX_TF_C4 = 0x8,
    GX_TF_C8 = 0x9,
    GX_TF_C14X2 = 0xa,
} GXCITexFmt;

typedef unsigned char GXBool;   /* era dolphin types: get_texture_buffer_size
                                 * declares GXBool mipmap, .s passes 0/1 in
                                 * r6 (0x803DB190/:803DB198 li r6) */

#ifdef __cplusplus
#define L5_STATIC_ASSERT(c, m) static_assert((c), m)
#else
#define L5_STATIC_ASSERT(c, m) _Static_assert((c), m)
#endif

L5_STATIC_ASSERT(GX_TF_RGBA8 == 6, "RGBA8 literal 6 == .s 0x8038BEE4 cmplwi r5,6");
L5_STATIC_ASSERT(GX_TF_Z24X8 == 0x16, "Z24X8 literal 0x16 == .s 0x8038BEEC cmplwi r5,0x16");
L5_STATIC_ASSERT(GX_TF_C8 == 9, "fixture format word 9 == GX_TF_C8 (colour-indexed 8bpp)");

/* Retail-stripped era assert (AU-2 above): arguments are NOT evaluated,
 * exactly like the absent .s code. */
#define ASSERTMSGLINEV(line, cond, ...) do {} while (0)
#define ASSERTMSGLINE(line, ...)        do {} while (0)
#define ASSERTLINE(line, ...)           do {} while (0)

#define __cntlzw(x) __builtin_clz((u32)(x))   /* AU-3 */

#endif /* LADDER5_GX_DEFS_H */
