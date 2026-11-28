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

/* RGB5A3 16-bit working value -> RGBA8 (TLUT/CI8 fmt==2 decode ONLY since
 * CMPRBE — the P2-FRMB CMPR endpoint leg moved to gxh_rgb565_rgba8 below:
 * GC hardware reads CMPR endpoints as plain RGB565 BIG-ENDIAN halfwords,
 * a DIFFERENT law from the signed TLUT entries. Era semantics per
 * docs/p2-oracle-1.md §1/§3 + endpoint census (0x0 black / 0xFFFF white /
 * 0x100 nettle green / 0xEF08 berry RGBA444). The CI8/TLUT signboard path
 * is human-verified under THIS law — CMPRBE must not be applied to it. */
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

/* CMPRBE (card t_41325af7): CMPR endpoint RGB565 -> RGBA8. NO alpha bits
 * exist in RGB565 — a = 255 is forced, the RGB5A3 sign bit NEVER branches
 * here (that confusion was the bug). GC CMPR law: endpoints are BIG-ENDIAN
 * u16 (PPC halfword load of the two DXT1 color bytes), plain RGB565.
 * External witness = owner boot footage (machine-local reference; only
 * numeric stats cited, and only here/at the receipts — the image is never
 * committed): sega_e01 re-measured same-turn over the landed bytes —
 * this law decodes a 94.7% white field with the mark chroma B-dominant
 * 2934 / R 0 (python port of the gate metric); the old LE/RGB5A3 law gave
 * the same field but R-dominant magenta 2980 / B 388, which the owner
 * frame (full-res tally: 101037 B-dominant / 0 R) refutes. */
static inline void gxh_rgb565_rgba8(unsigned short v, unsigned char rgba[4]) {
    rgba[0] = (unsigned char)(((v >> 11) & 0x1F) * 255 / 31);
    rgba[1] = (unsigned char)(((v >> 5) & 0x3F) * 255 / 63);
    rgba[2] = (unsigned char)((v & 0x1F) * 255 / 31);
    rgba[3] = 255;
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
 * GX hardware TCMP (DXT1-in-BIG-ENDIAN-RGB565 endpoints since CMPRBE; the
 * pre-fix "RGB5A3 endpoints + transparent index 3" reading was a bug that
 * became gate law) — GXTexture.s jumptable idx 14 -> .L_8038BEB8 class
 * (w*h/2, TI 8x8), SDK name GX_TF_CMPR (GXEnum.h:144-180), gate surface
 * ->header.format == GX_TF_CMPR (GinjaMaterial.cpp:372,390,423). The
 * manifest "I4" label is SUPERSEDED (docs/p2-oracle-1.md §2). On this host
 * there is no TCMP hardware, so the display path CPU-expands to RGBA8
 * exactly like the PAL8 leg does — the era never decodes 0xE on host (F4:
 * the walk copies packed bytes verbatim; texel interpretation is DRAW-time
 * GX). Texel law = owner-verified oracle PICK-2 (board tiles sega_e01/e02,
 * human-confirmed) + CMPRBE endianness fix (card t_41325af7, owner boot
 * footage as external witness — numeric stats citable, image never
 * committed): 8x8 tiles of 32 B, four 4x4 DXT1 blocks row-major within the
 * tile (rm = bx + 2*by, the surviving ordering probe), block bytes {c0 u16
 * BE, c1 u16 BE, 4 index bytes — texel (x,y) at 2 bits, 2*(x&3) of byte
 * 4+(y&3)}, 4-colour interpolation: c0>c1 -> [c0,c1,(2c0+c1)/3,(c0+2c1)/3];
 * else 3-colour [c0,c1,(c0+c1)/2] + GC index-3 OPAQUE BLACK (0,0,0,255)
 * (hardware has no CMPR alpha; RGB565 carries none). Endpoints through the
 * dedicated RGB565 decoder gxh_rgb565_rgba8 — NOT the TLUT RGB5A3 path
 * (different format, different law: CI8/TLUT RGB5A3 stays human-verified
 * as-is). Footage numerics (same-turn re-measure, python port of the C
 * gate metric): the old LE/RGB5A3 law presented the mark R-dominant
 * (canvas tally R 2895 / B 351, ~8.5:1) where the owner frame is
 * B-dominant with ZERO R-dominant pixels (full-frame tally 101037 B / 0 R,
 * 82.8% white field); the fixed law presents R 0 / B 3177 on a 94.4%
 * white field — glyph structure was already correct under both readings
 * (index bits unaffected), which is why every self-consistent gate passed
 * for days. Oracle coherence anchors RE-MEASURED under this law (was
 * 0.159/0.241 under RGB5A3 endpoints): sega_e01 0.1058 / e02 0.1738 vs
 * I4-TI canary 0.8321/0.8332 (layout-only, law-independent)
 * (docs/p2-oracle-1.md §3). */
static inline void gxh_cmpr_block_rgba8(const unsigned char *blk,
                                        unsigned char out[16][4]) {
    unsigned short c0 = (unsigned short)(blk[1] | (blk[0] << 8));   /* GC CMPR: BE u16 */
    unsigned short c1 = (unsigned short)(blk[3] | (blk[2] << 8));
    unsigned char pal[4][4];
    gxh_rgb565_rgba8(c0, pal[0]);
    gxh_rgb565_rgba8(c1, pal[1]);
    if (c0 > c1) {
        for (int i = 0; i < 4; i++) {
            pal[2][i] = (unsigned char)((2 * pal[0][i] + pal[1][i]) / 3);
            pal[3][i] = (unsigned char)((pal[0][i] + 2 * pal[1][i]) / 3);
        }
    } else {
        for (int i = 0; i < 3; i++)
            pal[2][i] = (unsigned char)((pal[0][i] + pal[1][i]) / 2);
        pal[2][3] = (unsigned char)((pal[0][3] + pal[1][3]) / 2);
        pal[3][0] = pal[3][1] = pal[3][2] = 0; pal[3][3] = 255;   /* GC: opaque black */
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

/* PLANTED WRONG-DECODE CANARY (CMPRBE, card t_41325af7 §6 — teeth proof for
 * the footage-anchored L9-U1 gate): the SUPERSEDED endpoint law verbatim —
 * endpoints as PC-DXT1 little-endian u16 through the TLUT RGB5A3 unpacker
 * with transparent index-3. Structure (glyphs, positions, white field) is
 * identical to the fixed law; only the chroma inverts (R-dominant magenta
 * ~8.5:1 where the owner footage tallies ZERO R-dominant pixels), so this
 * canary MUST FAIL the inverted hue legs while passing every structural
 * leg (docs/p2-seq-1.md §2 L9-U3). */
static inline void gxh_cmpr_block_rgba8_le5a3_canary(const unsigned char *blk,
                                                     unsigned char out[16][4]) {
    unsigned short c0 = (unsigned short)(blk[0] | (blk[1] << 8));
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
        pal[3][0] = pal[3][1] = pal[3][2] = 0; pal[3][3] = 0;
    }
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++) {
            unsigned idx = (blk[4 + y] >> (2 * x)) & 3;
            out[y * 4 + x][0] = pal[idx][0]; out[y * 4 + x][1] = pal[idx][1];
            out[y * 4 + x][2] = pal[idx][2]; out[y * 4 + x][3] = pal[idx][3];
        }
}

/* PLANTED BLACK-FIELD MODEL CANARY (CMPRBE L9-U2): the halfword-EXCHANGED
 * BE reading (color0 taken from bytes 2-3) — the law the pre-fix triage
 * prose believed ("BE+565 = black 93% field"). It renders the era mark on
 * a near-black field; the owner footage numerics (82.8% white pixels,
 * 0.003% strict-black, full-frame) refute it externally. Must FAIL the
 * white-field floor leg. Kept beside the truth decoder so the gate shows
 * BOTH wrong-law canaries (swap-field + old LE/5A3) tripping side by side. */
static inline void gxh_cmpr_block_rgba8_swapfield_canary(const unsigned char *blk,
                                                         unsigned char out[16][4]) {
    unsigned short c1 = (unsigned short)(blk[0] | (blk[1] << 8));
    unsigned short c0 = (unsigned short)(blk[2] | (blk[3] << 8));
    unsigned char pal[4][4];
    gxh_rgb565_rgba8(c0, pal[0]);
    gxh_rgb565_rgba8(c1, pal[1]);
    if (c0 > c1) {
        for (int i = 0; i < 4; i++) {
            pal[2][i] = (unsigned char)((2 * pal[0][i] + pal[1][i]) / 3);
            pal[3][i] = (unsigned char)((pal[0][i] + 2 * pal[1][i]) / 3);
        }
    } else {
        for (int i = 0; i < 3; i++)
            pal[2][i] = (unsigned char)((pal[0][i] + pal[1][i]) / 2);
        pal[2][3] = (unsigned char)((pal[0][3] + pal[1][3]) / 2);
        pal[3][0] = pal[3][1] = pal[3][2] = 0; pal[3][3] = 255;
    }
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++) {
            unsigned idx = (blk[4 + y] >> (2 * x)) & 3;
            out[y * 4 + x][0] = pal[idx][0]; out[y * 4 + x][1] = pal[idx][1];
            out[y * 4 + x][2] = pal[idx][2]; out[y * 4 + x][3] = pal[idx][3];
        }
}

/* Generic TI 8x8 expand through a caller-supplied block decoder — the two
 * planted canaries ride the SAME addressing legs as truth (only the palette
 * law differs), exactly like gxh_cmpr_expand_ti8x8. */
static inline void gxh_cmpr_expand_ti8x8_fn(const unsigned char *px,
                                            unsigned char *rgba8,
                                            unsigned w, unsigned h,
                                            void (*blockfn)(const unsigned char *,
                                                            unsigned char [16][4])) {
    for (unsigned ty = 0; ty < h; ty += 8)
        for (unsigned tx = 0; tx < w; tx += 8) {
            const unsigned char *tile = px + (size_t)32 * ((tx >> 3) + (w >> 3) * (ty >> 3));
            for (unsigned s = 0; s < 4; s++) {
                unsigned sbx = s & 1, sby = s >> 1;
                unsigned char blk16[16][4];
                blockfn(tile + 8 * s, blk16);
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
