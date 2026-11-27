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

/* P2-BB V5-S1: expand through a LOADED TLUT BANK entry (era supply route:
 * GVPL chunk -> load_gvpl_palette -> init_and_load_tlut -> GX seams ->
 * HostTlutBank {lut,fmt,count}). Decode models per GXTlutFmt
 * (libs/dolphin GXEnum.h:782-787 {IA8,RGB565,RGB5A3}) — 16-bit entries,
 * u16 working values BE-carrier; format 3 = RGBA8 byte-true carrier per
 * DEVIATION BB-1 (host CPU-expand route needs lossless 8-bit channels;
 * era HW formats decode quantized). */
static inline void gxh_tlut_rgba8(const void *lut, unsigned fmt,
                                  unsigned char index, unsigned char rgba[4]) {
    if (fmt == 3) {
        const unsigned char *e = (const unsigned char *)lut + (size_t)index * 4;
        rgba[0] = e[0]; rgba[1] = e[1]; rgba[2] = e[2]; rgba[3] = e[3];
        return;
    }
    /* era working value: 16-bit carriers are read BE (PPC lwz regime). The
     * GVPL payload stays byte-true through the BB-3 normalization, so the
     * host MUST byte-swap — a native LE u16 read on arm64 silently decoded
     * swapped channels in the genuine regime (never caught: AV-1 + the
     * synthetic golden ride the fmt==3 RGBA8 leg). P2-ORCL. */
    const unsigned char *eb = (const unsigned char *)lut + (size_t)index * 2;
    unsigned short v = (unsigned short)((eb[0] << 8) | eb[1]);
    if (fmt == 0) {           /* IA8: (I<<8)|A -> gray with data alpha */
        rgba[0] = rgba[1] = rgba[2] = (unsigned char)(v >> 8);
        rgba[3] = (unsigned char)(v & 0xFF);
    } else if (fmt == 1) {    /* RGB565 opaque */
        rgba[0] = (unsigned char)(((v >> 11) & 0x1F) << 3);
        rgba[1] = (unsigned char)(((v >> 5) & 0x3F) << 2);
        rgba[2] = (unsigned char)((v & 0x1F) << 3);
        rgba[3] = 255;
    } else if (fmt == 2) {    /* RGB5A3 (GXTL_RGB5A3, GXEnum.h:785): MSB=1 ->
                               * RGBA444+a3, MSB=0 -> opaque RGB555 in bits
                               * 14..0. P2-ORCL fix: both legs were lossy
                               * (opaque read RGB444 from bits 11-0; alpha
                               * leg read 2 of 4 bits). Era witnesses:
                               * docs/p2-oracle-1.md §1 (GXGetTexBufferSize
                               * jumptable) + §4 endpoint census of the
                               * genuine CMPR blocks — 0x100 dark-green nettle
                               * leaf / 0xEF08 RGBA444 berry / 0xFFFF white
                               * decode correctly ONLY under 555+444. */
        if (v & 0x8000) {
            unsigned a = (v >> 12) & 7;
            rgba[0] = (unsigned char)(((v >> 8) & 0xF) * 17);
            rgba[1] = (unsigned char)(((v >> 4) & 0xF) * 17);
            rgba[2] = (unsigned char)((v & 0xF) * 17);
            rgba[3] = (unsigned char)(a == 0 ? 0 : a * 255 / 7);
        } else {
            rgba[0] = (unsigned char)(((v >> 10) & 0x1F) * 255 / 31);
            rgba[1] = (unsigned char)(((v >> 5) & 0x1F) * 255 / 31);
            rgba[2] = (unsigned char)((v & 0x1F) * 255 / 31);
            rgba[3] = 255;
        }
    }
}

/* expand w*h PAL8 through the loaded bank (V5 era supply path) */
static inline void gxh_pal8_clut_expand_lut(const void *lut, unsigned fmt,
                                            const unsigned char *idx,
                                            unsigned char *rgba8, unsigned w, unsigned h) {
    for (unsigned i = 0; i < w * h; i++)
        gxh_tlut_rgba8(lut, fmt, idx[i], rgba8 + (size_t)i * 4);
}

/* P2-ORCL: GX_TF_C8 (fmt 9) container addressing is TI 8x4 tiled, NOT flat
 * row-major — era witness jumptable_804F50A8 case .L_8038BEC4 (rx,ry)=(3,2),
 * 32 B/tile, docs/p2-oracle-1.md §1; human-verified board (relay 2026-11-27).
 * Off-tile addressing: off = 32*((x>>3) + (w>>3)*(y>>2)) + 8*(y&3) + (x&7)
 * (tile-major across columns first, era GXGetTile row order; w,h are GX powers
 * so the shifts are exact). Output rgba8 stays row-major (display regime). */
static inline void gxh_pal8_clut_expand_ti8x4(const void *lut, unsigned fmt,
                                              const unsigned char *idx,
                                              unsigned char *rgba8, unsigned w, unsigned h) {
    unsigned tw = w >> 3;
    for (unsigned y = 0; y < h; y++)
        for (unsigned x = 0; x < w; x++) {
            size_t src = (size_t)32 * ((x >> 3) + tw * (y >> 2)) + (size_t)8 * (y & 3) + (x & 7);
            gxh_tlut_rgba8(lut, fmt, idx[src], rgba8 + ((size_t)y * w + x) * 4);
        }
}

#endif /* GX_LOGO_CLUT_H */
