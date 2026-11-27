/* pso-metal P2-Z — GP FIFO-lite + vertex-format dispatch (capture & replay).
 *
 * The ring holds BIG-ENDIAN BE32 words, byte-identical to what the era's
 * GX_WRITE_U8/U16/U32 sequence lands in GXFifoObj->base / a baked display
 * list:  word0 = (cmd<<24) | (data&0xFFFFFF); vertex-descriptor cmds
 * (type|vtxfmt in {0x80..0xBF nibble pair}, GXGeometry.c:37) carry nverts in
 * bits 8..23; CP 0x40 (GXDisplayList.c:74) is followed by the two payload
 * words (host: pointer-in-arena, we accept a u64 slot instead — see
 * DEVIATIONS). Data words after a vtxdesc cmd are sliced per the recorded
 * VTXFMT stream layout — the same slicing the GP hardware does from
 * GXSetVtxDesc + GXSetVtxAttrFmt (GXAttr.c:82/:228), proven by the decomp's
 * own wire consumers (TWindowSubStatusDisp draw_textured_quad_at_depth:
 * POS3F32, CLR1U32, TEX0 2F32 in that order under fmt4).
 *
 * DEVIATIONS (documented in docs/p2-gx-dl1.md):
 *  - CP 0x40 payload: era words are 32-bit PPC addresses; host pointers are
 *    64-bit, so a called list's payload is host-native ptr (u64) + nbytes.
 *  - No XF (vertex colors are already screen-space for this path — the era
 *    load_screen_space_matrix makes NDC trivially affine; full XF/mtx engine
 *    is a later lane).
 *  - GXSetTevOrder/MODULATE reduced to a fixed "tex × color0" multiply, the
 *    stage-1 recipe of ScreenPrimitiveDraw.cpp:124.
 */
#include "gx_dl.h"
#include "pso_bringup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------- BE helpers (era FIFO is PPC big-endian) ---------------- */
static u32 be(u32 le) {
    return __builtin_bswap32(le);
}
static u32 put_f32(f32 f) { u32 u; memcpy(&u, &f, 4); return be(u); }
static f32 get_f32(u32 bew) { u32 u = be(bew); f32 f; memcpy(&f, &u, 4); return f; }

/* ---------------- FIFO-lite ring (arena-backed) --------------------------- */
#define FIFO_WORDS 0x10000
static u32 *g_fifo;          /* BE32 words                                   */
static u32  g_fifo_n;

/* VTXFMT table: GXSetVtxDesc is GLOBAL era state (one vertex-descriptor reg
 * file); GXSetVtxAttrFmt is per-fmt. Model that split exactly. */
typedef struct { u8 en, type; } VDesc;                          /* global     */
static VDesc g_vdesc[3];
typedef struct { u8 comp, frac; } Vat;           /* per-fmt comp type/frac   */
static Vat g_vat[8][3];      /* [fmt][POS|CLR0|TEX0 compressed slot]         */
static u32 g_cur_fmt;

static int vat_slot(u32 attr) {
    switch (attr) {
        case GX_DL_VA_POS:  return 0;
        case GX_DL_VA_CLR0: return 1;
        case GX_DL_VA_TEX0: return 2;
        default: return -1;   /* NRM etc.: recorded-ignored in this subset   */
    }
}

typedef struct { u32 w[6]; u32 n; } VBatch;        /* decoded attrs per vert */
typedef struct {
    /* stream slice config for the current vtxdesc */
    u32 pos_w, clr_w, tex_w;      /* words per attribute (0 = absent)        */
    u32 pos_s16, tex_s16, clr_u16;/* compaction flags                        */
    u32 tex_frac;                 /* fixed-point exponent for s16 texcoords  */
} VLayout;

/* ---------------- stats ---------------------------------------------------- */
static GxDlStats g_st;
const GxDlStats *gx_dl_stats(void) { return &g_st; }

/* ---------------- vertex batch (post-NDC, 8 floats: ndc2 clr4 uv2) -------- */
#define MAX_TRIS 4096
#define MAX_VERTS 2048
static float g_vc[MAX_TRIS * 3 * 8];
static u32   g_vcn;                 /* vertex count in emitted batch         */
static float g_scratch[MAX_VERTS * 8];  /* raw decoded stream (pre-expand)   */
static f32   g_vp_l, g_vp_t, g_vp_w = 640, g_vp_h = 480;

void gx_dl_viewport(f32 l, f32 t, f32 w, f32 h) { g_vp_l=l; g_vp_t=t; g_vp_w=w; g_vp_h=h; }
void gx_dl_set_vtx_desc(u32 attr, u32 type) {
    int s = vat_slot(attr);
    if (s < 0) return;
    g_vdesc[s].en   = (type != 0);
    g_vdesc[s].type = type;
}
void gx_dl_set_vtx_attr_fmt(u32 fmt, u32 attr, u32 cnt, u32 comp, u8 frac) {
    int s = vat_slot(attr);
    if (s < 0 || fmt >= 8) return;
    g_vat[fmt][s].comp = comp; g_vat[fmt][s].frac = frac;
    (void)cnt;  /* X/Y-Z/ST count: this subset takes the full-arity case     */
}

/* ---------------- capture side (driver = what the era inlines did) -------- */
static void fifo_put(u32 bew) {
    if (g_fifo_n >= FIFO_WORDS) { fprintf(stderr, "gx_dl: FIFO-lite overflow\n"); exit(3); }
    g_fifo[g_fifo_n++] = bew;
}
void gx_dl_fifo_reset(void) { g_fifo_n = 0; g_vcn = 0; }

void gx_dl_begin(u32 prim_type, u32 vtxfmt, u16 nverts) {
    g_cur_fmt = vtxfmt;
    fifo_put((prim_type | vtxfmt) << 24 | ((u32)nverts << 8));
}
void gx_dl_fifo_pos3f32(f32 x, f32 y, f32 z) {
    fifo_put(put_f32(x)); fifo_put(put_f32(y)); fifo_put(put_f32(z));
}
void gx_dl_fifo_clr1u32(u32 rgba) { fifo_put(be(rgba)); }
void gx_dl_fifo_tex2f32(f32 s, f32 t) { fifo_put(put_f32(s)); fifo_put(put_f32(t)); }
/* CP call word sequence (era GXCallDisplayList: 0x40 + payload; host-ptr
 * deviation: u64 address split over two BE words, then nbytes). */
void gx_dl_fifo_call_display_list(const void *list, u32 nbytes) {
    u64 p = (u64)(uintptr_t)list;
    fifo_put(GX_DL_CP_CALL << 24);
    fifo_put(be((u32)(p >> 32)));
    fifo_put(be((u32)(p & 0xFFFFFFFFu)));
    fifo_put(be(nbytes));
}
void gx_dl_end(void) { /* era GXEnd is the no-op inline (GXVert.h:151)       */ }
u32 gx_dl_fifo_len(void) { return g_fifo_n; }

/* ---------------- decode side (GP dispatch) -------------------------------- */
static void layout_for(u32 fmt, VLayout *L) {
    memset(L, 0, sizeof *L);
    VDesc *p = &g_vdesc[0], *c = &g_vdesc[1], *t = &g_vdesc[2];
    Vat *pv = &g_vat[fmt][0], *cv = &g_vat[fmt][1], *tv = &g_vat[fmt][2];
    /* era comp types (GXEnum.h:396): GX_S16=3, GX_F32=4; RGBA8 clr = 1 word */
    if (p->en) { L->pos_w = (pv->comp == 3) ? 2 : 3; L->pos_s16 = (pv->comp == 3); }
    if (c->en) { L->clr_w = 1; (void)cv; }
    if (t->en) { L->tex_w = (tv->comp == 3) ? 1 : 2; L->tex_s16 = (tv->comp == 3);
                 L->tex_frac = tv->frac; }
}

/* unpack one position stream slice -> f32 x,y,z */
static void pos_unpack(const u32 *w, const VLayout *L, f32 *xyz) {
    if (L->pos_s16) {   /* (x,y) packed word + (z,pad) word, era s16 triple  */
        u32 w0 = be(w[0]), w1 = be(w[1]);
        xyz[0] = (f32)(s16)(w0 >> 16); xyz[1] = (f32)(s16)(w0 & 0xFFFF);
        xyz[2] = (f32)(s16)(w1 >> 16);
    } else {
        xyz[0]=get_f32(w[0]); xyz[1]=get_f32(w[1]); xyz[2]=get_f32(w[2]);
    }
}

static void push_scratch_vert(u32 *n, f32 *scr, u32 rgba, f32 s, f32 t) {
    if (*n + 1 > MAX_VERTS) { fprintf(stderr, "gx_dl: stream overflow\n"); exit(3); }
    float *v = &g_scratch[*n * 8];
    /* era GXSetViewport is (l,t,w,h) with y down; NDC y flips for Metal     */
    v[0] = 2.0f * (scr[0] - g_vp_l) / g_vp_w - 1.0f;
    v[1] = 1.0f - 2.0f * (scr[1] - g_vp_t) / g_vp_h;
    v[2] = ((rgba >> 24) & 0xFF) / 255.0f;   /* BE wire: R MSB (decomp's     */
    v[3] = ((rgba >> 16) & 0xFF) / 255.0f;   /* (c&0xFFFFFF)<<8 swap result) */
    v[4] = ((rgba >> 8)  & 0xFF) / 255.0f;
    v[5] = ( rgba        & 0xFF) / 255.0f;
    v[6] = s; v[7] = t;
    (*n)++;
}
static void emit_tri(const u32 a, const u32 b, const u32 c) {
    if (g_vcn + 3 > MAX_TRIS * 3) { fprintf(stderr, "gx_dl: batch overflow\n"); exit(3); }
    float *d = &g_vc[g_vcn * 8];
    memcpy(d,      &g_scratch[a * 8], 8 * sizeof(float));
    memcpy(d +  8, &g_scratch[b * 8], 8 * sizeof(float));
    memcpy(d + 16, &g_scratch[c * 8], 8 * sizeof(float));
    g_vcn += 3; g_st.tris_emitted++;
}

static void run_stream(const u32 *w, u32 nwords, u32 fmt, u32 prim, u32 nverts) {
    VLayout L; layout_for(fmt, &L);
    u32 vw = L.pos_w + L.clr_w + L.tex_w;
    if (!vw || nverts > MAX_VERTS || nverts * vw > nwords) {
        fprintf(stderr, "gx_dl: vtxdesc stream overrun (fmt=%u n=%u vw=%u avail=%u)\n",
                fmt, nverts, vw, nwords);
        exit(3);
    }
    u32 nv = 0;
    f32 scr[3];
    for (u32 i = 0; i < nverts; i++) {
        const u32 *p = w + i * vw; u32 k = 0, rgba = 0xFFFFFFFF; f32 s = 0, t = 0;
        if (L.pos_w) { pos_unpack(p + k, &L, scr); k += L.pos_w; }
        if (L.clr_w) { rgba = be(p[k]); k += L.clr_w; }
        if (L.tex_w) {
            if (L.tex_s16) {   /* era: s,t packed in one BE32 word, frac-scaled */
                u32 w0 = be(p[k]); f32 fs = (f32)(1u << (L.tex_frac ? L.tex_frac : 14));
                s = (s16)(w0 >> 16) / fs; t = (s16)(w0 & 0xFFFF) / fs; k++;
            }
            else { s = get_f32(p[k]); t = get_f32(p[k+1]); k += 2; }
        }
        push_scratch_vert(&nv, scr, rgba, s, t);
    }
    switch (prim & ~7u) {
        case GX_DL_QUADS:
            for (u32 q = 0; q + 4 <= nv; q += 4) {
                emit_tri(q, q+1, q+2);
                emit_tri(q, q+2, q+3);
            }
            break;
        case GX_DL_TRIANGLES:
            for (u32 i = 0; i + 3 <= nv; i += 3) emit_tri(i, i+1, i+2);
            break;
        case GX_DL_TRI_STRIP:
            for (u32 i = 0; i + 3 <= nv; i++) {
                if (i & 1) emit_tri(i+1, i, i+2);
                else       emit_tri(i,   i+1, i+2);
            }
            break;
        case GX_DL_TRI_FAN:
            for (u32 i = 1; i + 2 <= nv; i++) emit_tri(0, i, i+1);
            break;
        default:
            fprintf(stderr, "gx_dl: unsupported prim 0x%02x (slice covers QUADS/TRIANGLES/STRIP/FAN)\n", prim);
            exit(3);
    }
    g_st.verts_decoded += nverts;
}

/* Walk a BE32 word stream (CP semantics). Returns on 0x00 terminator or
 * exhaustion; `list` mode stops at terminator, FIFO drain runs to the end. */
static int run_words(const u32 *w, u32 nwords, int is_list, u32 depth) {
    if (depth > 4) { fprintf(stderr, "gx_dl: DL call nesting too deep\n"); exit(3); }
    u32 i = 0;
    while (i < nwords) {
        u32 word = w[i];
        u32 cmd  = word >> 24;
        if (cmd == 0x00) { if (is_list) return 0; i++; continue; }
        if (cmd == GX_DL_CP_CALL) {
            if (i + 3 > nwords) return 0;
            /* era: 0x40 then GX_WRITE_U32(list) GX_WRITE_U32(nbytes); host
             * ptr width deviation: payload is native ptr (2 slots? no — we
             * store ptr as u64 spanning words i+1,i+2 BE) then nbytes.      */
            u64 ptr = ((u64)be(w[i+1]) << 32) | be(w[i+2]);
            if (i + 4 > nwords) return 0;
            u32 nbytes = be(w[i+3]);
            if ((nbytes & 0x1F) != 0) { fprintf(stderr, "gx_dl: DL size not 32B aligned\n"); exit(3); }
            run_words((const u32 *)(uintptr_t)ptr, nbytes / 4, 1, depth + 1);
            g_st.dl_calls++;
            i += 4; continue;
        }
        if (cmd == GX_DL_CLRPOS) {          /* __GXSendFlushPrim: 0x98+vNum+pad */
            u32 vnum = (word >> 8) & 0xFFFF;
            g_st.clrpos_flushes++;
            i += 1 + 2 * vnum; continue;    /* vNum u32s, 3 words each on fmt4 */
        }
        /* vertex descriptor: type in high nibble pair, fmt low 3 bits */
        u32 prim = cmd & ~7u, fmt = cmd & 7u;
        switch (prim) {
            case GX_DL_QUADS: case GX_DL_TRIANGLES: case GX_DL_TRI_STRIP:
            case GX_DL_TRI_FAN: {
                u32 nverts = (word >> 8) & 0xFFFF;
                VLayout L; layout_for(fmt, &L);
                u32 need = nverts * (L.pos_w + L.clr_w + L.tex_w);
                if (i + 1 + need > nwords) {
                    fprintf(stderr, "gx_dl: vtxdesc stream past end of list (fmt=%u n=%u)\n", fmt, nverts);
                    exit(3);
                }
                run_stream(w + i + 1, need, fmt, prim, nverts);
                g_st.vtxdesc_cmds++;
                i += 1 + need; continue;
            }
            default: break;
        }
        if (cmd >= 0x60 && cmd <= 0x7F) { /* XF/BP-ish: skip data (1 word) */ i++; continue; }
        fprintf(stderr, "gx_dl: unhandled CP/GP cmd 0x%02x at word %u\n", cmd, i);
        exit(3);
    }
    return 0;
}

void gx_dl_call_display_list(const void *list, u32 nbytes) {
    if (((uintptr_t)list & 0x1F) != 0) { fprintf(stderr, "gx_dl: DL not 32B aligned\n"); exit(3); }
    if ((nbytes & 0x1F) != 0) { fprintf(stderr, "gx_dl: DL size not 32B aligned\n"); exit(3); }
    g_st.dl_calls++;
    run_words((const u32 *)list, nbytes / 4, 1, 0);
}

void gx_dl_flush(void) {
    run_words(g_fifo, g_fifo_n, 0, 0);   /* decode only; present() draws      */
}

/* clear color the backend uses on next draw */
static GXColor g_clear = {0, 0, 0, 0};
void gx_dl_clear_color(GXColor c) { g_clear = c; }

void gx_dl_set_texture(unsigned w, unsigned h, const unsigned char *rgba8) {
    gxh_dl_set_texture(w, h, rgba8);
}

/* ---------------- init / backend glue ------------------------------------- */
__attribute__((constructor)) static void gx_dl_init(void) {
    /* arena-backed like the era's cfg.fifo_buffer; bringup installs before
     * main, constructor only records that we exist (lazy alloc ok too).     */
    g_fifo = NULL;
}
int gx_dl_boot(void) {           /* driver calls after pso_bringup_install()  */
    g_fifo = (u32 *)(pso_arena_base + 0x300000);  /* era fifo_buffer slot     */
    memset(g_fifo, 0, FIFO_WORDS * 4);
    g_fifo_n = 0;
    /* fmt4 default = the era screen-quad recipe (CameraCapture.cpp:278-283) */
    gx_dl_set_vtx_desc(GX_DL_VA_POS, GX_DL_DIRECT);
    gx_dl_set_vtx_desc(GX_DL_VA_CLR0, GX_DL_DIRECT);
    gx_dl_set_vtx_desc(GX_DL_VA_TEX0, GX_DL_DIRECT);
    gx_dl_set_vtx_attr_fmt(4, GX_DL_VA_POS,  1 /*GX_POS_XYZ*/, 4 /*GX_F32*/, 0);
    gx_dl_set_vtx_attr_fmt(4, GX_DL_VA_CLR0, 1 /*GX_CLR_RGBA*/, 5 /*GX_RGBA8*/, 0);
    gx_dl_set_vtx_attr_fmt(4, GX_DL_VA_TEX0, 1 /*GX_TEX_ST*/,  4 /*GX_F32*/, 0);
    gx_dl_viewport(0, 0, 640, 480);
    return gxh_dl_open(640, 480);
}

/* draw hook needs the clear color — route through one internal call */
int gx_dl_present(void) {
    if (getenv("GX_DL_DEBUG") && g_st.frames_presented == 0) {
        for (u32 i = 0; i < g_vcn; i++)
            printf("  vert%u: ndc(%.3f,%.3f) rgba(%.2f,%.2f,%.2f,%.2f) uv(%.3f,%.3f)\n",
                   i, g_vc[i*8], g_vc[i*8+1], g_vc[i*8+2], g_vc[i*8+3], g_vc[i*8+4],
                   g_vc[i*8+5], g_vc[i*8+6], g_vc[i*8+7]);
    }
    int rc = gxh_dl_draw(g_vc, g_vcn, g_clear.r/255.0, g_clear.g/255.0,
                         g_clear.b/255.0, 1.0);
    if (rc == 0) g_st.frames_presented++;
    return rc;
}
void gx_dl_note_verify(int bad, int total) {
    if (bad) g_st.verify_failures++;
    g_st.samples_ok += (unsigned)(total - bad);
}
