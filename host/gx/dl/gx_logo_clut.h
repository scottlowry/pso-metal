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

/* RGB5A3 16-bit working value -> RGBA8 (the fmt==2 decode, shared with the
 * P2-FRMB CMPR endpoint leg so ONE decoder serves both). Era semantics per
 * docs/p2-oracle-1.md §1/§3 + endpoint census (0x0 black / 0xFFFF white /
 * 0x100 nettle green / 0xEF08 berry RGBA444). */
static inline void gxh_rgb5a3_rgba8(unsigned short v, unsigned char rgba[4]) {
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
    } else if (fmt == 2) {    /* RGB5A3 (GXTL_RGB5A3, GXEnum.h:785) — shared
                               * decoder leg (P2-ORCL 555/444 fix; witnesses
                               * in gxh_rgb5a3_rgba8). */
        gxh_rgb5a3_rgba8(v, rgba);
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

/* ---- P2-FRMB: GX_TF_CMPR (fmt 0xE) expand leg ----------------------------
 * DEVIATION FRM-1 (host decode vs era truth): the era resolves fmt 0xE as
 * GX hardware TCMP (DXT1-in-RGB5A3 endpoints) — GXTexture.s jumptable
 * idx 14 -> .L_8038BEB8 class (w*h/2, TI 8x8), SDK name GX_TF_CMPR
 * (GXEnum.h:144-180), gate surface->header.format == GX_TF_CMPR
 * (GinjaMaterial.cpp:372,390,423). The manifest "I4" label is SUPERSEDED
 * (docs/p2-oracle-1.md §2). On this host there is no TCMP hardware, so the
 * display path CPU-expands to RGBA8 exactly like the PAL8 leg does — the
 * era never decodes 0xE on host (F4: the walk copies packed bytes verbatim;
 * texel interpretation is DRAW-time GX). Texel law = owner-verified oracle
 * PICK-2 (board tiles sega_e01/e02, human-confirmed): 8x8 tiles of 32 B,
 * four 4x4 DXT1 blocks row-major within the tile (rm = bx + 2*by, the
 * surviving ordering probe), block bytes {c0 u16 LE, c1 u16 LE, 4 index
 * bytes — texel (x,y) at 2 bits, 2*(x&3) of byte 4+(y&3)}, 4-colour
 * interpolation: c0>c1 -> [c0,c1,(2c0+c1)/3,(c0+2c1)/3]; else 4th colour
 * transparent (0,0,0,0). Endpoints through the shared RGB5A3 decoder.
 * Oracle coherence anchors (this exact law): sega_e01 0.159 / e02 0.241 vs
 * I4-TI canary 0.835 / 0.834 (docs/p2-oracle-1.md §3). */
static inline void gxh_cmpr_block_rgba8(const unsigned char *blk,
                                        unsigned char out[16][4]) {
    unsigned short c0 = (unsigned short)(blk[0] | (blk[1] << 8));   /* DXT1 bytes: LE u16 */
    unsigned short c1 = (unsigned short)(blk[2] | (blk[3] << 8));
    unsigned char pal[4][4];
    gxh_rgb5a3_rgba8(c0, pal[0]);
    gxh_rgb5a3_rgba8(c1, pal[1]);
    if (c0 > c1) {
        for (int i = 0; i < 4; i++) {
            pal[2][i] = (unsigned char)((2 * pal[0][i] + pal[1][i]) / 3);
            pal[3][i] = (unsigned char)((pal[0][i] + 2 * pal[1][i]) / 3);
        }
    } else {
        for (int i = 0; i < 3; i++)
            pal[2][i] = (unsigned char)((pal[0][i] + pal[1][i]) / 2);
        pal[2][3] = (unsigned char)((pal[0][3] + pal[1][3]) / 2);
        pal[3][0] = pal[3][1] = pal[3][2] = 0; pal[3][3] = 0;   /* transparent */
    }
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++) {
            unsigned idx = (blk[4 + y] >> (2 * x)) & 3;
            out[y * 4 + x][0] = pal[idx][0]; out[y * 4 + x][1] = pal[idx][1];
            out[y * 4 + x][2] = pal[idx][2]; out[y * 4 + x][3] = pal[idx][3];
        }
}

/* expand w*h CMPR (TI 8x8) -> row-major RGBA8 (display regime): decode each
 * 4x4 block once via gxh_cmpr_block_rgba8, scatter into row-major output */
static inline void gxh_cmpr_expand_ti8x8(const unsigned char *px,
                                         unsigned char *rgba8,
                                         unsigned w, unsigned h) {
    for (unsigned ty = 0; ty < h; ty += 8)
        for (unsigned tx = 0; tx < w; tx += 8) {
            const unsigned char *tile = px + (size_t)32 * ((tx >> 3) + (w >> 3) * (ty >> 3));
            for (unsigned s = 0; s < 4; s++) {          /* row-major sub-blocks */
                unsigned sbx = s & 1, sby = s >> 1;
                unsigned char blk16[16][4];
                gxh_cmpr_block_rgba8(tile + 8 * s, blk16);
                for (unsigned y = 0; y < 4; y++)
                    for (unsigned x = 0; x < 4; x++) {
                        unsigned char *o = rgba8 + ((size_t)(ty + sby * 4 + y) * w + tx + sbx * 4 + x) * 4;
                        o[0] = blk16[y * 4 + x][0]; o[1] = blk16[y * 4 + x][1];
                        o[2] = blk16[y * 4 + x][2]; o[3] = blk16[y * 4 + x][3];
                    }
            }
        }
}

/* PLANTED WRONG-DECODE CANARY (gate instrument, like the L7-P4 flat leg):
 * the superseded manifest "I4" reading — TI 8x8 nibble gray. Must FAIL the
 * coherence gate on genuine frames (oracle: 0.835/0.834, noise ~1.0). */
static inline void gxh_i4_ti8x8_gray_expand(const unsigned char *px,
                                            unsigned char *rgba8,
                                            unsigned w, unsigned h) {
    for (unsigned y = 0; y < h; y++)
        for (unsigned x = 0; x < w; x++) {
            size_t t = (size_t)32 * ((x >> 3) + (w >> 3) * (y >> 3))
                     + (size_t)4 * (y & 7) + ((x >> 1) & 3);
            unsigned v = (x & 1) ? (px[t] & 0xF) : (px[t] >> 4);
            unsigned char *o = rgba8 + ((size_t)y * w + x) * 4;
            o[0] = o[1] = o[2] = (unsigned char)(v * 17); o[3] = 255;
        }
}

#endif /* GX_LOGO_CLUT_H */
