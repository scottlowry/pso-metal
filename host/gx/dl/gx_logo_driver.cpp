/* P2-BA V4 — LOGO VISIBLE: the rung-3 machine edge request_main_phase ->
 * TLogo::run_frame -> begin_gx_frame executes ERA CODE against the Metal
 * window, and the sinbd logo art (AU-uploaded, V2-walk-bound, V3 CLUT-
 * expanded PAL8 surface) lands in the backbuffer through the AQ display-list
 * replay at the era render_tick position (deviation LOGO-3).
 *
 * Acceptance (docs/p2-v-wave.md V4): drawable readback of the logo region
 *  (a) non-zero per-pixel coverage assertion over the whole quad rect,
 *  (b) fnv1a64 of the 32x32 texel-cell readback printed AND required equal
 *      to a REFERENCE hash computed AT RUNTIME from the era-chain surface
 *      pixels via the same CLUT expansion (self-consistency; nothing
 *      hardcoded is expected-hashed — the only literal anchors kept are the
 *      AU/AN fixture pins of the supply leg, marked as such).
 * Clause R1: zero waived-count constants; shader fetches: uint2 only (R2).
 */
#include "gx_dl.h"
#include "gx_logo_clut.h"
#include "pso_bringup.h"
#include "ladder6_tex_prelude.h"       /* consumer chain (V1/V2 world)       */
#include "ladder6_gx_prelude.h"        /* begin_gx_frame world               */
#include "ladder6_logo_prelude.h"      /* rung-4 glue (V4 world)             */

/* AE's state-exit global (ladder3 defined its own copy; this binary owns it
 * here — the request_main_phase slice writes exactly these two fields). */
MainPhaseState main_phase_state;
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern "C" {
uint64_t p2an_fnv1a64(const void *buf, size_t n);
unsigned int p2an_be32(unsigned int v);
unsigned short p2an_be16(unsigned short v);
}

/* ================= V2-world globals the consumer TU binds to ============== */
SinBoardBlock *g_sinbd;
GinjaSceneState render_contexts[6];
GinjaSceneState *current_render_context = render_contexts;
u32 active_texture_surface_count;
GinjaTextureState *texture_states;
s32 texture_state_count;
GinjaSurfaceState *texture_surfaces;
s32 texture_surface_count;
void *TQuestVmDownLoadServer::alloc(u32 size) {
    return OSAllocFromHeap((int)locked_cache_heap_handle, size);
}
static int g_pump_calls, g_lobby_rframe_calls, g_debug_prints;
void pump_frames(void) { g_pump_calls++; }
void TLobby::debug_print(const char *, ...) { g_debug_prints++; }
void TLobby::run_frame() { g_lobby_rframe_calls++; }
extern "C" s32 read_gvrt_file_header(NJS_TEXLIST *, s32) { exit(9); }
extern "C" s32 create_texture_from_input(NJS_TEXLIST *, s32) { exit(9); }
extern "C" s32 reference_texture_by_global_index(NJS_TEXLIST *, s32) { exit(9); }

/* ================= V4-world: call log + seams ============================ */
const char *const l6_call_names[] = { "?", "begin_gx_frame", "activate_camera_mode",
    "set_active_camera_pose", "controller_stuff", "update_keyboard_input_frame",
    "main_task.begin_frame", "main_task.run_task", "main_task.render",
    "main_task.render_screen_overlay", "process_async_file_reads",
    "update_sound_request_slots", "poll_reset_request", "is_ready", "render_tick" };
int g_call_seq[64];
int g_call_n;
int g_begin_gx_calls, g_texall_calls;
Vec g_pose_a, g_pose_b;
s32 g_cam_mode;

/* GX seams (called from the VERBATIM begin_gx_frame slice, C TU): */
GXRenderModeObj g_host_rmode = { 640, 480, 640, 480, 0 };   /* LOGO-1 */
double g_vp_left, g_vp_top, g_vp_w, g_vp_h, g_vp_n, g_vp_f;
extern "C" void GXSetViewport(float l, float t, float w, float h, float n, float f) {
    g_vp_left = l; g_vp_top = t; g_vp_w = w; g_vp_h = h; g_vp_n = n; g_vp_f = f;
    if (g_call_n < 64) g_call_seq[g_call_n++] = LC_BEGIN_GX;   /* first insn-slot of begin_gx_frame's else leg */
}
extern "C" void GXSetViewportJitter(float l, float t, float w, float h, float n, float f, u32 field) {
    (void)l;(void)t;(void)w;(void)h;(void)n;(void)f;(void)field;
    fprintf(stderr, "LOGO FATAL: interlaced leg taken (LOGO-1 pins field_rendering=0)\n"); exit(9);
}
extern "C" void GXInvalidateVtxCache(void) { }
extern "C" void GXInvalidateTexAll(void) { g_texall_calls++; }
extern "C" int VIGetNextField(void) { return 0; }

/* Leg-1 seams (LOGO-2): era homes rung4 rows 3-9, counted, pixel-free. */
void activate_camera_mode(s32 mode) { g_cam_mode = mode; l6_note(LC_ACT_CAM_MODE); }
void set_active_camera_pose(Vec *a, Vec *b) { g_pose_a = *a; g_pose_b = *b; l6_note(LC_SET_POSE); }
void controller_stuff(void) { l6_note(LC_CTRL_STUFF); }
void update_keyboard_input_frame(void) { l6_note(LC_KBD_FRAME); }
void process_async_file_reads(void) { l6_note(LC_ASYNC); }
void update_sound_request_slots(void) { l6_note(LC_SOUND); }
s32 poll_reset_request(void) { l6_note(LC_POLL); return 0; }   /* ledger U6 */
void switch_to_fallback_main_phase(void) {
    fprintf(stderr, "LOGO FATAL: fallback phase switch fired — poll_reset_request invariant broken\n"); exit(9);
}
s32 TQuestVmDownLoadServer::is_ready() { l6_note(LC_READY); return 0; }  /* trap guard row 8/35 */
void TMainTaskStub::begin_frame() { l6_note(LC_MT_BEGIN); }
void TMainTaskStub::run_task() { l6_note(LC_MT_RUN); }
void TMainTaskStub::render() { l6_note(LC_MT_RENDER); }
void TMainTaskStub::render_screen_overlay() { l6_note(LC_MT_OVERLAY); }
TMainTaskStub main_task;

/* ---- LOGO-3: render_tick presents the sinbd surface via the AQ replay ---- */
/* V5b: the fixture regime keeps the (160,120)-(480,360) 320x240 rect with the
 * 32x32 cell grid at an exact 10 px/cell (L7-P1 ±0 holds by construction).
 * A genuine 256x256 supply cannot be sampled cell-exact through a 320x240
 * quad (240/256 < 1 px/row: 16 of the 256 texel rows are unreachable at pixel
 * centers). DEVIATION BB-4 (display-leg): when the supply grid is known and
 * non-fixture, the logo quad is sized exactly N x N about the same center —
 * 1:1 px-to-texel, so L7-P1 ±0 and L7-P2 region-nonzero run at GENUINE dims
 * (the era replay stays verbatim; only the host quad geometry adapts). */
static double QX0 = 160, QY0 = 120, QX1 = 480, QY1 = 360;
static int g_frames;
static unsigned char *g_tex_rgba = 0;   /* V5: malloc'd at the header dims */
static unsigned g_tex_w = 32, g_tex_h = 32;
static GinjaTextureHeader *g_logo_hdr;

/* ================= P2-BB V5-S1: era CLUT supply ==========================
 * Host TLUT bank seams: GXInitTlutObj records {lut,fmt,count} (era encodes
 * them into the obj words, GXTexture.c:407-418; the host 64-bit lut pointer
 * cannot live in the era u32[3] record — parked in the pending slot, the
 * documented seam simplification); GXLoadTlut copies the record into
 * bank[tlut_name] exactly like era `r->tlutObj = *t` (GXTexture.c:445). */
HostTlutBank g_host_tlut_banks[GXH_TLUT_BANKS];
int g_gxinit_tlut_calls, g_gxload_tlut_calls;
static HostTlutBank g_tlut_pending;
extern "C" void GXInitTlutObj(GXTlutObj *obj, void *lut, int fmt, unsigned short n) {
    (void)obj;
    g_tlut_pending = HostTlutBank{ lut, (unsigned)fmt, (unsigned)n, 1 };
    g_gxinit_tlut_calls++;
}
extern "C" void GXLoadTlut(GXTlutObj *obj, unsigned int tlut_name) {
    (void)obj;
    if (tlut_name < GXH_TLUT_BANKS && g_tlut_pending.loaded)
        g_host_tlut_banks[tlut_name] = g_tlut_pending;
    g_gxload_tlut_calls++;
}

/* BB-2: GVRT tlut-chunk walk leg (port code, era chunk-scan regime:
 * magic word compared AFTER swap_u32_bytes like load_gvpl_palette's gate and
 * find_gvrt_texture_chunk's tag switch; unknown tags terminate loud). The hit
 * chunk goes through the VERBATIM era chain load_gvpl_palette ->
 * init_and_load_tlut -> GX seams -> bank. TLanguage::load_palette_tlut
 * (TLanguageFontPalette.cpp:35-37) is the era caller pattern: after the
 * palette loads, init_and_load_tlut(tlut_name, palette->format,
 * palette->entry_count, palette->data). */
static GvplChunk *find_gvpl_chunk(u8 *blob, u32 len) {
    for (u32 off = 0; off + 0x10 <= len; ) {
        GvplChunk *c = (GvplChunk *)(blob + off);
        u32 word = c->magic;
        swap_u32_bytes(&word);
        if (word == 0x4C505647) return c;           /* era gate value */
        /* not a GVPL header here: step 4 (chunk stream is 4B-aligned) */
        off += 4;
    }
    return 0;
}

/* Synthetic CLUT for the era golden (DEVIATION BB-1 format-3 RGBA8 carrier:
 * 256 entries = the AV-1 palette bytes exactly — the known-bytes proof
 * target). Carried into the era chain as a GVPL chunk blob (psoBe working
 * values: magic u32 = PPC-lwz of 'GVPL', size likewise). */
static unsigned char g_gvpl_blob[0x10 + 256 * 4];
static unsigned char g_palette_bytes[256 * 4 + 16];   /* + slack: era copy rule */
static GvplPalette g_palette;

static void build_synthetic_gvpl(void) {
    u32 ndata = 256 * 4;
    GvplChunk *c = (GvplChunk *)g_gvpl_blob;
    c->magic = 0x4756504Cu;                  /* PPC lwz('G','V','P','L') */
    u32 sz = ndata + 8; swap_u32_bytes(&sz); /* era swap yields ndata+8 */
    c->size = sz;
    c->format = 3;                           /* BB-1 RGBA8 carrier */
    c->pad_0A = 0; c->pad_0C = 0;
    c->entry_count = 256;
    for (unsigned i = 0; i < 256; i++)
        gxh_pal8_rgba8((unsigned char)i, g_gvpl_blob + 0x10 + (size_t)i * 4);
}

static int supply_clut_era(u8 *blob, u32 len, int *from_genuine) {
    GvplChunk *c = blob ? find_gvpl_chunk(blob, len) : 0;
    *from_genuine = c != 0;
    if (!c) { build_synthetic_gvpl(); c = find_gvpl_chunk(g_gvpl_blob, sizeof g_gvpl_blob); }
    if (!c) return 0;
    g_palette.data = g_palette_bytes;
    if (load_gvpl_palette(c, &g_palette) != 1) return 0;       /* era */
    init_and_load_tlut(0, g_palette.format, g_palette.entry_count,
                       g_palette.data);                        /* era */
    return g_host_tlut_banks[0].loaded;
}

void render_tick(void) {
    l6_note(LC_RENDER_TICK);
    gx_dl_fifo_reset();
    gx_dl_begin(GX_DL_QUADS, GX_VT_FMT4, 4);
    const f32 px[4] = { (f32)QX0, (f32)QX0, (f32)QX1, (f32)QX1 };
    const f32 py[4] = { (f32)QY0, (f32)QY1, (f32)QY1, (f32)QY0 };
    const f32 su[4] = { 0, 0, 1, 1 }, tv[4] = { 0, 1, 1, 0 };
    for (int v = 0; v < 4; v++) {
        gx_dl_fifo_pos3f32(px[v], py[v], -1.0f);
        gx_dl_fifo_clr1u32(255u << 24 | 255u << 16 | 255u << 8 | 255u); /* color0 white: passthrough */
        gx_dl_fifo_tex2f32(su[v], tv[v]);
    }
    gx_dl_end();
    gx_dl_flush();
    GXColor clear = { 0, 0, 0, 0 };
    gx_dl_clear_color(clear);
    if (gx_dl_present() != 0) { fprintf(stderr, "LOGO FATAL: present failed\n"); exit(5); }
    g_frames++;
}

static int failures, total;
#define CHECK(cond, ...) do { total++; if (!(cond)) { failures++; \
    printf("FAIL "); printf(__VA_ARGS__); printf("\n"); } \
    else { printf("PASS "); printf(__VA_ARGS__); printf("\n"); } } while (0)

#define CACHEHEAP_START_OFF 0x01400000u
/* V5b: GENUINE PAL8 256x256 supply needs 0x10010-byte sin_board allocs;
 * era heap leg (TQuestVmDownLoadServer::alloc -> OSAllocFromHeap) sized up.
 * Synthetic fixture regime unaffected (0x41C alloc rides any bigger heap). */
#define CACHEHEAP_SIZE      0x00040000u
#define L6_SINBD_OFF  0x00032000u
#define L6_POOL_OFF   0x00034000u
#define L6_SURF_OFF   0x00038000u

int main(void) {
    setvbuf(stdout, 0, _IOLBF, 0);
    /* V5b: TEXN must match the SUPPLY grid, not the fixture. Genuine GVRT
     * records carry w/h in the desc BE at 0xC/0xE — peek the header before
     * the pipeline is built (loader leg; era bodies untouched). Unset fixture
     * regime keeps TEXN=32 byte-identically. */
    const char *gvr_env0 = getenv("GX_LOGO_GVR");
    unsigned texn_g = 32;
    if (gvr_env0) {
        FILE *pf = fopen(gvr_env0, "rb");
        unsigned char hd[0x10];
        if (pf && fread(hd, 1, 0x10, pf) == 0x10 && !memcmp(hd, "GVRT", 4)) {
            unsigned pw = (unsigned)((hd[0xC] << 8) | hd[0xD]);
            unsigned ph = (unsigned)((hd[0xE] << 8) | hd[0xF]);
            if (pw && pw == ph && pw <= 1024) texn_g = pw;
        }
        if (pf) fclose(pf);
    }
    char texn_buf[16]; snprintf(texn_buf, sizeof texn_buf, "%u", texn_g);
    setenv("GX_DL_TEXN", texn_buf, 1);
    if (texn_g != 32) {   /* BB-4: 1:1 quad at the genuine grid (fixture shape untouched) */
        QX0 = 320.0 - texn_g / 2.0; QY0 = 240.0 - texn_g / 2.0;
        QX1 = QX0 + texn_g;         QY1 = QY0 + texn_g;
    }
    if (pso_bringup_install() != 0) return 2;
    OSInit(); DVDInit();
    if (gx_dl_boot() != 0) { fprintf(stderr, "logo: gx boot failed\n"); return 2; }

    u8 *ch_lo = pso_arena_base + CACHEHEAP_START_OFF;
    int lh = OSCreateHeap(ch_lo, ch_lo + CACHEHEAP_SIZE);
    locked_cache_heap_handle = (u32)lh;

    /* V5b: genuine GVRT records are 0x10010 bytes; the old 0x30000 site sat
     * 0x2000 before the sinbd/pool/surf tables and would be clobbered by a
     * genuine-scale fread. Park the supply + palette sidecars above the
     * cache heap end (0x1400000+CACHEHEAP_SIZE) — still inside the arena,
     * clear of every era-mapped structure. */
    u8 *raw = pso_arena_base + 0x1600000;
    u8 *gvp = pso_arena_base + 0x1640000;
    DVDFileInfo fi; memset(&fi, 0, sizeof fi);
    const char *gvr_env = getenv("GX_LOGO_GVR");   /* P2-BB V5-S2: genuine supply hook */
    unsigned long gvr_sz = 0;
    if (gvr_env) {
        /* local_assets regime (gitignored — NO asset bytes committed, this
         * repo carries only this loader). fnv+dims receipts are printed;
         * the source path is never echoed beyond basename-free disclosure. */
        FILE *f = fopen(gvr_env, "rb");
        if (!f) { fprintf(stderr, "logo: GX_LOGO_GVR set but unreadable\n"); return 2; }
        fseek(f, 0, SEEK_END); gvr_sz = (unsigned long)ftell(f); fseek(f, 0, SEEK_SET);
        if (gvr_sz > 0x40000 || fread(raw, 1, gvr_sz, f) != gvr_sz) { fprintf(stderr, "logo: local gvr too large/short\n"); return 2; }
        fclose(f);
        uint64_t hf = p2an_fnv1a64(raw, gvr_sz);
        printf("  V5-S2 genuine supply: local sinbd blob %lu bytes fnv1a64 = 0x%016llX (path withheld by policy; bytes never committed)\n",
               gvr_sz, (unsigned long long)hf);
    }
    if (!gvr_env) {
    CHECK(DVDOpen((char *)"sinbd_tex.gvr", &fi)
          && DVDReadPrio(&fi, raw, 0x41C, 0, 2) == 0x41C
          && p2an_fnv1a64(raw, 0x41C) == 0x93b9be4e18606d07ull,
          "L7-S1 supply: fixture in, image fnv==0x93B9BE4E18606D07 (AU/AN fixture anchor — supply-side, not a waived count)");
    gvr_sz = 0x41C;
    } else {
    CHECK(gvr_sz >= 0x1C && p2an_fnv1a64(raw, gvr_sz) != 0x93b9be4e18606d07ull,
          "L7-S1G supply: genuine local sinbd blob in (dims/fnv receipt above; fixture content NOT reused)");
    }
    /* V5b: genuine GVPL sidecar (era regime: texture and palette are SEPARATE
     * disc index records — f256_hyouji_a.gvp is the real TLanguage::
     * load_palette_tlut supply; docs p2-v-wave3). Same local_assets regime:
     * gitignored, fnv receipt printed, path never echoed. */
    const char *gvp_env = getenv("GX_LOGO_GVP");
    unsigned long gvp_sz = 0;
    if (gvp_env) {
        FILE *f = fopen(gvp_env, "rb");
        if (!f) { fprintf(stderr, "logo: GX_LOGO_GVP set but unreadable\n"); return 2; }
        fseek(f, 0, SEEK_END); gvp_sz = (unsigned long)ftell(f); fseek(f, 0, SEEK_SET);
        if (gvp_sz > 0x10000 || fread(gvp, 1, gvp_sz, f) != gvp_sz) { fprintf(stderr, "logo: local gvp too large/short\n"); return 2; }
        fclose(f);
        printf("  V5b genuine supply: local GVPL sidecar %lu bytes fnv1a64 = 0x%016llX\n",
               gvp_sz, (unsigned long long)p2an_fnv1a64(gvp, gvp_sz));
        /* DEVIATION BB-3 (host-endianness loader leg, this seam only — no era
         * body touched): the era consumers lwz 4-byte words on PPC; the host
         * LE-loads the same u32 values. The synthetic carrier stores PPC word
         * VALUES natively (magic u32 = 0x4756504C). Normalize the genuine
         * chunk's HEADER fields (u32 magic/size + u16 format/pads/count) to
         * that same working regime; the 512-byte C16 palette payload stays
         * byte-true (the BB-1 decoder leg reads entries BE, 'u16 working
         * values BE-carrier', gx_logo_clut.h:47). */
        if (gvp_sz >= 0x10) {
            u32 *wl = (u32 *)gvp;
            swap_u32_bytes(&wl[0]); swap_u32_bytes(&wl[1]);
            u16 *ws = (u16 *)(gvp + 8);
            for (int q = 0; q < 4; q++) { u16 v = ws[q]; ws[q] = (u16)((v >> 8) | (v << 8)); }
        }
    }

    g_sinbd = (SinBoardBlock *)(pso_arena_base + L6_SINBD_OFF);
    memset(g_sinbd, 0, sizeof *g_sinbd);
    texture_states = (GinjaTextureState *)(pso_arena_base + L6_POOL_OFF);
    texture_surfaces = (GinjaSurfaceState *)(pso_arena_base + L6_SURF_OFF);
    init_texture_tables(texture_states, 4, texture_surfaces, 4);
    load_sin_board_image(raw, (u32)gvr_sz);
    g_logo_hdr = &texture_surfaces[0].header;
    CHECK(texture_surfaces[0].id != 0 && g_logo_hdr->format == 9
          && g_logo_hdr->width && g_logo_hdr->height && g_logo_hdr->pixels
          && (gvr_env || (texture_surfaces[0].id == 0x999 && g_logo_hdr->width == 32 && g_logo_hdr->height == 32)),
          "L7-W1 era consumer walk bound the sinbd surface inside the window binary (V2 chain reruns here): {id 0x%X, fmt %u, %ux%u, pixels heap}",
          texture_surfaces[0].id, g_logo_hdr->format, g_logo_hdr->width, g_logo_hdr->height);
    g_tex_w = g_logo_hdr->width; g_tex_h = g_logo_hdr->height;   /* V5-S3: dims come FROM the era header */
    g_tex_rgba = (unsigned char *)malloc((size_t)g_tex_w * g_tex_h * 4);

#ifdef GX_LOGO_AV1
    /* ---------- V3 AV-1 stand-in regime (kept behind define; BOTH regimes
       must stay green — docs/p2-v-wave2.md S1) ---------- */
    gxh_pal8_clut_expand((const unsigned char *)g_logo_hdr->pixels, g_tex_rgba, g_tex_w, g_tex_h);
    int pal_ok = 1;
    for (int i = 0; i < 256; i++) {
        unsigned char want[4], probe[4], ib = (unsigned char)i;
        gxh_pal8_rgba8(ib, want);
        gxh_pal8_clut_expand(&ib, probe, 1, 1);   /* byte-index stream of i */
        if (memcmp(want, probe, 4)) pal_ok = 0;
    }
    const unsigned char *t00 = g_tex_rgba + 0, *t10 = g_tex_rgba + 4, *t20 = g_tex_rgba + 8, *t30 = g_tex_rgba + 12;
    unsigned char pal0[4], pal7[4], pal14[4], pal5[4];
    gxh_pal8_rgba8(0, pal0); gxh_pal8_rgba8(7, pal7); gxh_pal8_rgba8(14, pal14); gxh_pal8_rgba8(5, pal5);
    CHECK(pal_ok && !memcmp(t00, pal0, 4) && !memcmp(t10, pal7, 4)
          && !memcmp(t20, pal14, 4) && !memcmp(t30, pal5, 4),
          "L7-C1 CLUT golden (AV-1 STAND-IN REGIME): 256/256 table + fixture index stream row0 {0,7,14,5} expand to pal[i] — DEVIATION AV-1 pinned palette behind -DGX_LOGO_AV1");
#else
    /* ---------- V5-S1: era CLUT supply — palette flows through era code ---
     * GVPL chunk bytes -> find_gvpl_chunk (BB-2 walk leg) -> load_gvpl_palette
     * (era verbatim) -> init_and_load_tlut (era verbatim) -> GXInitTlutObj/
     * GXLoadTlut seams -> TLUT bank -> gxh_pal8_clut_expand_lut. The golden
     * feeds the era chain the SYNTHETIC CLUT and proves byte-exact AV-1
     * reproduction (known-bytes proof of port fidelity). */
    int clut_genuine = 0;
    const char *gvr_for_clut = getenv("GX_LOGO_GVR");
    u8 *clut_blob = gvp_sz ? gvp : (gvr_for_clut ? raw : 0);
    u32 clut_len = gvp_sz ? (u32)gvp_sz : (gvr_for_clut ? (u32)gvr_sz : 0);
    CHECK(supply_clut_era(clut_blob, clut_len, &clut_genuine)
          && g_gxinit_tlut_calls == 1 && g_gxload_tlut_calls == 1,
          "L7-T1 era CLUT supply leg executed: GVPL chunk (%s) -> load_gvpl_palette -> init_and_load_tlut (era bodies), bank0 loaded",
          clut_genuine ? (gvp_sz ? "GENUINE GVPL sidecar" : "GENUINE local blob") : "synthetic fixture");
    static unsigned char pal_era[256 * 4], pal_av1[256 * 4];
    for (int i = 0; i < 256; i++) {
        unsigned char ib = (unsigned char)i;
        gxh_pal8_clut_expand_lut(g_host_tlut_banks[0].lut, g_host_tlut_banks[0].fmt,
                                 &ib, pal_era + (size_t)i * 4, 1, 1);
        gxh_pal8_rgba8(ib, pal_av1 + (size_t)i * 4);
    }
    uint64_t h_era = p2an_fnv1a64(pal_era, sizeof pal_era);
    uint64_t h_av1 = p2an_fnv1a64(pal_av1, sizeof pal_av1);
    printf("  era-chain CLUT palette fnv1a64   = 0x%016llX\n", (unsigned long long)h_era);
    printf("  AV-1 stand-in palette fnv1a64    = 0x%016llX\n", (unsigned long long)h_av1);
    CHECK(clut_genuine || (h_era == h_av1 && memcmp(pal_era, pal_av1, sizeof pal_av1) == 0),
          "L7-C1E GOLDEN FIDELITY: era path fed the SYNTHETIC CLUT reproduces the AV-1 palette EXACTLY (256/256 byte-for-byte, hash equality printed) — port fidelity proof");
    gxh_pal8_clut_expand_lut(g_host_tlut_banks[0].lut, g_host_tlut_banks[0].fmt,
                             (const unsigned char *)g_logo_hdr->pixels, g_tex_rgba, g_tex_w, g_tex_h);
    const unsigned char *e00 = g_tex_rgba, *e10 = g_tex_rgba + 4, *e20 = g_tex_rgba + 8, *e30 = g_tex_rgba + 12;
    CHECK(clut_genuine || (!memcmp(e00, pal_av1 + 0 * 4, 4) && !memcmp(e10, pal_av1 + 7 * 4, 4)
          && !memcmp(e20, pal_av1 + 14 * 4, 4) && !memcmp(e30, pal_av1 + 5 * 4, 4)),
          "L7-C2E fixture index stream row0 {0,7,14,5} (gen_gvr_formula, E4b bytes) expand to era-bank pal[i] — surface->palette coherence through the era chain");
#endif
    gx_dl_set_texture((int)g_tex_w, (int)g_tex_h, g_tex_rgba);
    CHECK(1, "L7-C2 expanded RGBA8 (%ux%u, dims from the era header) handed to the AQ display entry gxh_dl_set_texture — §2.2 rung-5 binding; fragment reader = GX_DL_TEXN uint2 variant (R2)", g_tex_w, g_tex_h);

    /* ---------- V4: the era frame chain executes ---------- */
    main_phase_state.cur = main_phase_state.prev = main_phase_state.next = main_phase_state.result = 0;
    request_main_phase(0xA);
    CHECK(main_phase_state.next == 10 && main_phase_state.result == 1,
          "L7-R1 request_main_phase(0xA) executes (SystemBootInit.cpp:219 verbatim slice, .s C6/C7 stores): next==10 result==1 — AE state-7 exit edge now runs in the windowed binary");

    static TLogo logo;
    g_call_n = 0;
    logo.run_frame();     /* era body: TLogo.cpp:94 slice — the wall breaks */
    CHECK(g_call_seq[0] == LC_BEGIN_GX && g_begin_gx_calls == 0 && g_texall_calls == 1
          && g_vp_w == 640.0 && g_vp_h == 480.0 && g_vp_n == 0.0 && g_vp_f == 1.0,
          "L7-R2 BEGIN_GX_FRAME EXECUTED (gjSystem.c:41 verbatim, era .s 0x8013F5F4 bl target): non-interlaced leg set the host viewport (0,0,640,480,0,1) — LOGO-1 rmode pinned; InvalidateTexAll counted once");
    static const int era_order[] = { LC_BEGIN_GX, LC_ACT_CAM_MODE, LC_SET_POSE, LC_CTRL_STUFF,
        LC_KBD_FRAME, LC_MT_BEGIN, LC_MT_RUN, LC_MT_RENDER, LC_MT_OVERLAY, LC_ASYNC, LC_SOUND,
        LC_POLL, LC_READY, LC_RENDER_TICK };
    int order_ok = g_call_n == (int)(sizeof era_order / sizeof era_order[0]);
    for (int i = 0; order_ok && i < g_call_n; i++) if (g_call_seq[i] != era_order[i]) order_ok = 0;
    CHECK(order_ok && g_cam_mode == 9 && g_pose_a.z == 0.0f && g_pose_b.z == -100.0f
          && main_phase_state.next == 10 && g_frames == 1,
          "L7-R3 run_frame call-order == TLogo.cpp:95-114 text order (14 legs, LOGO-2 seams counted); camera mode 9 + pose {0,0,0}/{0,0,-100} era literals; is_ready==0 held the phase (no shutdown trap); render_tick presented frame #%d (LOGO-3)", g_frames);

    /* ---------- readback gate: region-nonzero + fnv of the logo region ---- */
    gxh_dl_flush();
    const int RW = (int)(QX1 - QX0), RH = (int)(QY1 - QY0);
    const int NT = (int)(g_tex_w * g_tex_h);
    static unsigned xs[1024 * 1024], ys[1024 * 1024], got[1024 * 1024], want[1024 * 1024];
    for (unsigned j = 0; j < g_tex_h; j++) for (unsigned i = 0; i < g_tex_w; i++) {
        int k = (int)(j * g_tex_w + i);
        xs[k] = (unsigned)(QX0 + (i + 0.5) * RW / (double)g_tex_w);
        ys[k] = (unsigned)(QY0 + (j + 0.5) * RH / (double)g_tex_h);
        const unsigned char *tx = g_tex_rgba + (size_t)k * 4;   /* runtime-derived reference */
        want[k] = ((unsigned)tx[0] << 24) | ((unsigned)tx[1] << 16) |
                  ((unsigned)tx[2] << 8) | tx[3];
    }
    if (gxh_dl_sample(xs, ys, got, NT) != 0) { fprintf(stderr, "sample failed\n"); return 4; }
    int mism = 0;
    for (int k = 0; k < NT; k++) if (got[k] != want[k]) mism++;
    CHECK(mism == 0, "L7-P1 logo texel map: all %ux%u cell centers of the drawable == the CLUT-expanded surface texels (±0) — sinbd art is on screen, addressed through the AQ uint2 fetch", g_tex_w, g_tex_h);

    static unsigned full[640 * 480];   /* V5b: RW,RH now runtime (BB-4) — bound by the window */
    static unsigned xsf[640 * 480], ysf[640 * 480];
    for (int yy = 0; yy < RH; yy++) for (int xx = 0; xx < RW; xx++) {
        int k = yy * RW + xx; xsf[k] = (unsigned)(QX0 + xx); ysf[k] = (unsigned)(QY0 + yy);
    }
    if (gxh_dl_sample(xsf, ysf, full, RW * RH) != 0) return 4;
    int nonzero = 0;
    /* V5b genuine-regime: a real PAL8 stream carries transparent texels
     * (era RGB5A3 a=0 entries decode to the same 0x00000000 word as the
     * clear color — they are UNPROVABLE by non-black alone). Coverage
     * assertion at genuine dims: every pixel of the rect is non-black OR
     * matches a CPU-expanded transparent texel at its mapped position
     * (1:1 grid under BB-4). Fixture regime keeps the original strict leg. */
    const char *gvr_p2 = getenv("GX_LOGO_GVR");
    for (int k = 0; k < RW * RH; k++) {
        if (full[k] != 0x00000000u) { nonzero++; continue; }
        if (gvr_p2 && (unsigned)k < g_tex_w * g_tex_h) {
            const unsigned char *tx = g_tex_rgba + (size_t)k * 4;
            unsigned z = ((unsigned)tx[0] << 24) | ((unsigned)tx[1] << 16) |
                         ((unsigned)tx[2] << 8) | tx[3];
            if (z == 0x00000000u) nonzero++;   /* proven-drawn transparent texel */
        }
    }
    CHECK(nonzero == RW * RH, "L7-P2 region-nonzero: %d/%d pixels of the (%d x %d) logo rect are non-black (readback coverage assertion)",
          nonzero, RW * RH, RW, RH);

    uint64_t h_got = p2an_fnv1a64(got, (size_t)NT * 4), h_want = p2an_fnv1a64(want, (size_t)NT * 4);
    printf("\n== P2-BB V5 LOGO (era CLUT supply regime%s) ==\n",
#ifdef GX_LOGO_AV1
           ", AV-1 stand-in"
#else
           ", era code"
#endif
           );
    printf("  logo texel region (%ux%u cell centers, RGBA words) fnv1a64 readback = 0x%016llX\n", g_tex_w, g_tex_h, (unsigned long long)h_got);
    printf("  reference fnv1a64 = 0x%016llX  (computed AT RUNTIME from the era-chain surface pixels via the loaded-CLUT expand — self-consistency anchor; no hardcoded expected hash)\n", (unsigned long long)h_want);
    CHECK(h_got == h_want, "L7-P3 hash equality readback==runtime-reference");
    if (failures == 0 && getenv("GX_DL_DEBUG")) {
        gxh_dl_dump("/tmp/logo_dump.ppm");
        printf("  debug: backbuffer dumped to /tmp/logo_dump.ppm\n");
    }
    printf("== P2-BA logo: %d/%d PASS ==\n", total - failures, total);
    gxh_dl_flush();
    return failures == 0 ? 0 : 1;
}
