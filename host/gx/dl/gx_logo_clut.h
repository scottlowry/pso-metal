/* P2-BA V3 — PAL8 CLUT expand into the AQ display path.
 *
 * AV §2.2 binding (CHOSEN rung-5 route): CPU CLUT-expand -> RGBA8 feeding
 * the PROVEN AQ entry gxh_dl_set_texture(w,h,rgba8) — RGBA8Unorm /
 * ShaderRead / bytesPerRow=w*4 / default Shared storage (contract §2.2
 * bullets verbatim; the getBytes roundtrip regime holds there). R2/§2.5:
 * every fragment fetch that consumes this output is a uint2 read — the
 * GX_DL_TEXN shader variant (gx_dl_metal.m) is the only reader.
 *
 * DEVIATION AV-1 (P2-AV §2.2, honored here): the fixture's tlut=0 and the
 * era CLUT chunk (init_and_load_tlut, GinjaTextureHeader.cpp:56-60) is
 * unported this rung, so the palette is the PINNED STAND-IN below — index
 * space 0..15 reproduces AQ's 16-distinct diagnostic colors exactly (
 * (16*(i%4+1), 16*(i/4+1), 128, 255) — the GX_DL_TEX16 cell map in
 * gx_dl_driver.c:79-82, so V3's golden is checkable against the fixture's
 * own index stream through the ALREADY-VERIFIED AQ texel-addressing math),
 * 16..255 a gray ramp (i,i,i,255). The R8 + in-shader LUT era-faithful route
 * stays the contract's deferred alternative (§2.2 bullet 4). */
#ifndef GX_LOGO_CLUT_H
#define GX_LOGO_CLUT_H

#include <string.h>

static inline void gxh_pal8_rgba8(unsigned char index, unsigned char rgba[4]) {
    if (index < 16) {
        rgba[0] = (unsigned char)(16 * ((index % 4) + 1));
        rgba[1] = (unsigned char)(16 * ((index / 4) + 1));
        rgba[2] = 128;
        rgba[3] = 255;
    } else {
        rgba[0] = rgba[1] = rgba[2] = index;   /* gray ramp */
        rgba[3] = 255;
    }
}

/* row-major expand w*h PAL8 -> w*h*4 RGBA8 (top row first — the regime
 * gxh_dl_set_texture's replaceRegion documents, gx_dl.h:79-81) */
static inline void gxh_pal8_clut_expand(const unsigned char *idx, unsigned char *rgba8,
                                        unsigned w, unsigned h) {
    for (unsigned i = 0; i < w * h; i++)
        gxh_pal8_rgba8(idx[i], rgba8 + (size_t)i * 4);
}

#endif /* GX_LOGO_CLUT_H */
