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
#include <sys/stat.h>          /* P2-SEQ L9: /tmp/cla artifact dir (mkdir -p) */
#include <sys/types.h>

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

/* ================= P2-FRMB L8: genuine logo_e frames (hook-gated) =========
 * Spec: docs/p2-frames-spec-1.md F1/F2/F5/F6, format resolution per
 * docs/p2-oracle-1.md §5 (fmt 0xE = GX_TF_CMPR — the manifest "I4" labels
 * are SUPERSEDED; DEVIATION FRM-1 in gx_logo_clut.h pins the host-decode
 * class). Hooks: GX_FRAMES_GVM=<local_assets/logo_e.gvm> (the era `buf`:
 * decompressed logo_e GVMH archive) + GX_FRAMES_SLICES=<slices.json>
 * (runtime-reference slice-fnv sidecar, gitignored). CI defaults run with
 * zero genuine bytes AND zero genuine hashes in this TU: every expected
 * hash arrives through the sidecar at runtime (L7-P3 posture, :463-466). */
#define LOGO_E_COUNT 24
/* F1 descriptor names, verbatim era strings (TAdSEGAlogo.cpp:18-52), stream
 * order == attr 0..23 == GVRT order: gc_adx01..08, sega_logo_e01..08,
 * st_logo01..08. */
static const char *const l8_texname_str[LOGO_E_COUNT] = {
    "gc_adx01","gc_adx02","gc_adx03","gc_adx04","gc_adx05","gc_adx06",
    "gc_adx07","gc_adx08","sega_logo_e01","sega_logo_e02","sega_logo_e03",
    "sega_logo_e04","sega_logo_e05","sega_logo_e06","sega_logo_e07",
    "sega_logo_e08","st_logo01","st_logo02","st_logo03","st_logo04",
    "st_logo05","st_logo06","st_logo07","st_logo08" };
/* F2 pinned era GBIX globals (builder-supplied era-shaped constants —
 * grep @6b38b5383 shows the values exist ONLY as directory bytes on disc;
 * these are era identifiers, not asset hashes — compile-time legal). */
#define LOGO_E_GBIX_GC_ADX(k) (0x0001B0A8u + (unsigned)(k))   /* k 0..7 */
#define LOGO_E_GBIX_SEGA(k)   (0x0001AFC2u + (unsigned)(k))   /* k 0..7 */
#define LOGO_E_GBIX_ST(k)     (0x0010CAB4u + (unsigned)(k))   /* k 0..7 */
static u32 logo_e_gbix(unsigned k) {
    return k < 8 ? LOGO_E_GBIX_GC_ADX(k)
         : k < 16 ? LOGO_E_GBIX_SEGA(k - 8) : LOGO_E_GBIX_ST(k - 16);
}
/* Host-resident mirror of the era descriptor (DEVIATION FRM-2: the era
 * NJS_TEXNAME/NJS_TEXLIST are u32 image-handle slot space under the P2-AR
 * shadow; this descriptor is host runtime state and never crosses the AR
 * boundary — the era shapes it mirrors are quoted in F1). */
struct L8Texname { const char *filename; u32 attr; GinjaTextureState *texture; };
struct L8Texlist { L8Texname *textures; u32 nbTexture; };
static L8Texname l8_texnames[LOGO_E_COUNT];
static L8Texlist texture_logo_e_descriptor = { l8_texnames, LOGO_E_COUNT };
static L8Texlist *logo_texture_e_handle;        /* era global, TAdSEGAlogo.cpp:52 */
/* F3 re-handle sites (TAdSoniclogo.cpp:60 / TAdADXlogo.cpp:65-71) */
static L8Texlist *sonic_logo_object;
static L8Texlist *adx_logo_texture;
static void free_sonic_logo_texture(void) { }   /* era bodies EMPTY (F3) */
static void free_adx_logo_texture(void) { }
static L8Texlist *get_logo_texture_e_handle(void) { return logo_texture_e_handle; }
static void load_sonic_logo_texture(void) { sonic_logo_object = get_logo_texture_e_handle(); }
static void load_adx_logo_texture(void) { adx_logo_texture = get_logo_texture_e_handle(); }
/* register_texture_logo_e (TAdSEGAlogo.cpp:102-106 era shape: handle +
 * ONE synchronous fill-all-24 load; the load leg is the driver-equivalent
 * walk below — docs/p2-logo-e-supply-1.md §S4 sanctions "body OR driver
 * equivalent"). Walk state (24 slots; pixels parked host-side per F6.7 —
 * supply-side malloc, NEVER the era cache heap, I4 §S4.4 law). */
static GinjaTextureState  l8_states[LOGO_E_COUNT];
static GinjaSurfaceState  l8_surfs[LOGO_E_COUNT];
static unsigned char     *l8_px[LOGO_E_COUNT];          /* packed CMPR payload */
static unsigned long      l8_plen[LOGO_E_COUNT];
static unsigned long long l8_chunk_fnv[LOGO_E_COUNT];
static unsigned l8_w[LOGO_E_COUNT], l8_h[LOGO_E_COUNT];
static u32 l8_dir_attr[LOGO_E_COUNT], l8_dir_fmt[LOGO_E_COUNT], l8_dir_gbix[LOGO_E_COUNT];
static unsigned l8_dir_dw[LOGO_E_COUNT], l8_dir_dh[LOGO_E_COUNT];
static int l8_tlut_neg;                                  /* count of tlut==-1 binds */
static int l8_walk_ok, l8_walk_chunks;
static unsigned long l8_walk_end;

static int frames_walk_gvm(u8 *b, unsigned long n) {   /* driver-equivalent walk */
    if (n < 0x3C || memcmp(b, "GVMH", 4)) return 0;
    unsigned long hdr = (unsigned long)(b[4] | (b[5] << 8) | (b[6] << 16) | ((unsigned long)b[7] << 24));
    unsigned flags = (unsigned)((b[8] << 8) | b[9]);
    unsigned count = (unsigned)((b[10] << 8) | b[11]);
    if (hdr != 0x3a8 || flags != 0x000F || count != LOGO_E_COUNT) return 0;
    unsigned long off = hdr + 8;                        /* era chunk-stream law */
    for (unsigned k = 0; k < LOGO_E_COUNT; k++) {
        const u8 *de = b + 0xC + (size_t)0x26 * k;      /* stride 0x26 (flags 0x000F) */
        l8_dir_attr[k] = (u32)((de[0] << 8) | de[1]);
        l8_dir_fmt[k]  = (u32)((de[0x1E] << 8) | de[0x1F]);
        u16 dims = (u16)((de[0x20] << 8) | de[0x21]);   /* nibble codes 2^(n+2) */
        l8_dir_dw[k] = 1u << (((dims >> 4) & 0xF) + 2);
        l8_dir_dh[k] = 1u << ((dims & 0xF) + 2);
        l8_dir_gbix[k] = ((u32)de[0x22] << 24) | ((u32)de[0x23] << 16) |
                         ((u32)de[0x24] << 8) | de[0x25];
        if (off + 0x10 > n || memcmp(b + off, "GVRT", 4)) return 0;
        unsigned long sz = (unsigned long)(b[off+4] | (b[off+5] << 8) |
                           (b[off+6] << 16) | ((unsigned long)b[off+7] << 24));
        u32 fmtw = ((u32)b[off+8] << 24) | ((u32)b[off+9] << 16) |
                   ((u32)b[off+10] << 8) | b[off+11];   /* desc fmt u32 BE */
        unsigned dw = (unsigned)((b[off+0xC] << 8) | b[off+0xD]);
        unsigned dh = (unsigned)((b[off+0xE] << 8) | b[off+0xF]);
        if (off + sz + 8 > n || sz < 8) return 0;
        l8_w[k] = dw; l8_h[k] = dh;
        l8_plen[k] = sz - 8;                            /* copy law: chunk->size-8 */
        l8_chunk_fnv[k] = p2an_fnv1a64(b + off, sz + 8);
        if ((fmtw & 0xFF) != 0x0E) return 0;            /* CMPR family (0x0E/0x80E) */
        l8_px[k] = (unsigned char *)malloc(l8_plen[k]); /* supply-side, not era heap */
        if (!l8_px[k]) return 0;
        memcpy(l8_px[k], b + off + 0x10, l8_plen[k]);
        /* bind: era regime — non-palette fmt (is_palette_texture_format hits
         * only 8/9/10) => create_texture_state else-leg tlut_name = -1;
         * surface id IS the GBIX global (F2, union slot). */
        if (is_palette_texture_format((u8)(fmtw & 0xFF))) return 0;
        l8_surfs[k].id = l8_dir_gbix[k];
        l8_surfs[k].header.format = fmtw & 0xFF;
        l8_surfs[k].header.width = dw; l8_surfs[k].header.height = dh;
        l8_surfs[k].header.size = (u32)l8_plen[k];
        l8_surfs[k].header.pixels = (u32 *)l8_px[k];
        l8_states[k].surface = &l8_surfs[k];
        l8_states[k].tlut_name = ~0u;
        l8_texnames[k].filename = l8_texname_str[k];
        l8_texnames[k].attr = k;
        l8_texnames[k].texture = &l8_states[k];
        if (l8_states[k].tlut_name == ~0u) l8_tlut_neg++;
        off += sz + 8;
        l8_walk_chunks++;
    }
    l8_walk_end = off;
    l8_walk_ok = 1;
    return 1;
}

/* sidecar (gitignored slices.json) — runtime-reference anchor reader.
 * Fixed format written by the lane; order == records array order. */
static unsigned long long sc_arch_fnv;
static unsigned long sc_arch_size;
static char sc_name[LOGO_E_COUNT][32];
static unsigned long long sc_fnv[LOGO_E_COUNT];
static int slices_parse(const char *js) {
    const char *p = strstr(js, "\"fnv\"");
    if (!p || sscanf(p, "\"fnv\": \"%16llX\"", &sc_arch_fnv) != 1) return 0;
    p = strstr(js, "\"size\"");
    if (!p || sscanf(p, "\"size\": %lu", &sc_arch_size) != 1) return 0;
    for (unsigned k = 0; k < LOGO_E_COUNT; k++) {
        p = strstr(p, "\"name\"");
        if (!p) return 0;
        const char *q = strchr(p + 8, '"'); if (!q) return 0; q++;
        const char *r = strchr(q, '"'); if (!r || (size_t)(r - q) >= 32) return 0;
        memcpy(sc_name[k], q, (size_t)(r - q)); sc_name[k][r - q] = 0;
        p = strstr(r, "\"fnv\""); if (!p) return 0;
        if (sscanf(p, "\"fnv\": \"%16llX\"", &sc_fnv[k]) != 1) return 0;
    }
    return 1;
}

static double p2frmb_coherence(const unsigned *img, int cw, int ch) {
    /* identical metric shape to L7-P4 (adjacent RGB MAD / row-shuffled
     * baseline, deterministic xorshift shuffle) — file-scope so the genuine
     * frames leg and any follow-on share one instrument */
    auto rgb = [](unsigned w, int c) { return (int)((w >> (24 - 8 * c)) & 0xFF); };
    unsigned long long adj = 0, n = 0;
    for (int y = 0; y < ch; y++) for (int x = 0; x + 1 < cw; x++) {
        unsigned a = img[y*cw+x], b = img[y*cw+x+1];
        adj += (unsigned)(abs(rgb(a,0)-rgb(b,0)) + abs(rgb(a,1)-rgb(b,1)) + abs(rgb(a,2)-rgb(b,2))); n++;
    }
    for (int y = 0; y + 1 < ch; y++) for (int x = 0; x < cw; x++) {
        unsigned a = img[y*cw+x], b = img[(y+1)*cw+x];
        adj += (unsigned)(abs(rgb(a,0)-rgb(b,0)) + abs(rgb(a,1)-rgb(b,1)) + abs(rgb(a,2)-rgb(b,2))); n++;
    }
    unsigned long long sadj = 0, sn = 0;
    static unsigned shuf[1024 * 1024];
    unsigned long long st = 0x9E3779B97F4A7C15ull;
    for (int y = 0; y < ch; y++) {
        for (int i = 0; i < cw; i++) shuf[(size_t)y*cw+i] = img[(size_t)y*cw+i];
        for (int i = cw - 1; i > 0; i--) {
            st ^= st << 13; st ^= st >> 7; st ^= st << 17;
            unsigned j = (unsigned)(st % (unsigned long long)(i + 1));
            unsigned t = shuf[(size_t)y*cw+i]; shuf[(size_t)y*cw+i] = shuf[(size_t)y*cw+j]; shuf[(size_t)y*cw+j] = t;
        }
    }
    for (int y = 0; y < ch; y++) for (int x = 0; x + 1 < cw; x++) {
        unsigned a = shuf[(size_t)y*cw+x], b = shuf[(size_t)y*cw+x+1];
        sadj += (unsigned)(abs(rgb(a,0)-rgb(b,0)) + abs(rgb(a,1)-rgb(b,1)) + abs(rgb(a,2)-rgb(b,2))); sn++;
    }
    for (int y = 0; y + 1 < ch; y++) for (int x = 0; x < cw; x++) {
        unsigned a = shuf[(size_t)y*cw+x], b = shuf[(size_t)(y+1)*cw+x];
        sadj += (unsigned)(abs(rgb(a,0)-rgb(b,0)) + abs(rgb(a,1)-rgb(b,1)) + abs(rgb(a,2)-rgb(b,2))); sn++;
    }
    double d_adj = (double)adj / (double)n;
    return d_adj / ((double)sadj / (double)sn);
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
    /* P2-ORCL fix (human-verified oracle, relay 2026-11-27): the fmt-9
     * (GX_TF_C8) container is TI 8x4 TILED, not flat row-major — flat here
     * WAS the owner-visible "random lines of color" defect. Era witness:
     * GXTexture.s jumptable_804F50A8 idx 9 -> .L_8038BEC4 (8x4, 32 B) +
     * docs/p2-oracle-1.md §1/§3. */
    gxh_pal8_clut_expand_ti8x4(g_host_tlut_banks[0].lut, g_host_tlut_banks[0].fmt,
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
#ifndef GX_LOGO_AV1
    /* ---------- P2-ORCL L7-P4: container-layout coherence gate ----------
     * Mean adjacent-pixel color distance (H+V) over the readback logo rect
     * vs its row-shuffled baseline: correct TI 8x4 decode of real art lands
     * ~0.3, the planted WRONG-decode CANARY (same pixels expanded FLAT —
     * the pre-fix behavior, the owner's "random lines") must land >=0.5.
     * Gate can fail both ways: PASS requires tiled-metric < 0.5 AND
     * canary-metric >= 0.5. Both printed. Fixture regime (no genuine blob)
     * SKIPS the metric but keeps the count (era regime = 13 legs). */
    if (gvr_env) {
        static unsigned canary_flat[1024 * 1024];
        {
            static unsigned char canary_rgba[1024 * 1024 * 4];
            gxh_pal8_clut_expand_lut(g_host_tlut_banks[0].lut, g_host_tlut_banks[0].fmt,
                                     (const unsigned char *)g_logo_hdr->pixels,
                                     canary_rgba, g_tex_w, g_tex_h);
            for (int k = 0; k < NT; k++) {
                const unsigned char *tx = canary_rgba + (size_t)k * 4;
                canary_flat[k] = ((unsigned)tx[0] << 24) | ((unsigned)tx[1] << 16) |
                                 ((unsigned)tx[2] << 8) | tx[3];
            }
        }
        auto rgb = [](unsigned w, int c) { return (int)((w >> (24 - 8 * c)) & 0xFF); };
        auto coh = [&](const unsigned *img, int cw, int ch) -> double {
            unsigned long long adj = 0, n = 0;
            for (int y = 0; y < ch; y++) for (int x = 0; x + 1 < cw; x++) {
                unsigned a = img[y*cw+x], b = img[y*cw+x+1];
                adj += (unsigned)(abs(rgb(a,0)-rgb(b,0)) + abs(rgb(a,1)-rgb(b,1)) + abs(rgb(a,2)-rgb(b,2)));
                n++;
            }
            for (int y = 0; y + 1 < ch; y++) for (int x = 0; x < cw; x++) {
                unsigned a = img[y*cw+x], b = img[(y+1)*cw+x];
                adj += (unsigned)(abs(rgb(a,0)-rgb(b,0)) + abs(rgb(a,1)-rgb(b,1)) + abs(rgb(a,2)-rgb(b,2)));
                n++;
            }
            double d_adj = (double)adj / (double)n;
            /* row-shuffled baseline: deterministic xorshift shuffle per row */
            static unsigned shuf[1024 * 1024];
            unsigned long long st = 0x9E3779B97F4A7C15ull;
            for (int y = 0; y < ch; y++) {
                for (int i = 0; i < cw; i++) shuf[y*cw+i] = img[y*cw+i];
                for (int i = cw - 1; i > 0; i--) {
                    st ^= st << 13; st ^= st >> 7; st ^= st << 17;
                    unsigned j = (unsigned)(st % (unsigned long long)(i + 1));
                    unsigned t = shuf[y*cw+i]; shuf[y*cw+i] = shuf[y*cw+j]; shuf[y*cw+j] = t;
                }
            }
            unsigned long long sadj = 0, sn = 0;
            for (int y = 0; y < ch; y++) for (int x = 0; x + 1 < cw; x++) {
                unsigned a = shuf[y*cw+x], b = shuf[y*cw+x+1];
                sadj += (unsigned)(abs(rgb(a,0)-rgb(b,0)) + abs(rgb(a,1)-rgb(b,1)) + abs(rgb(a,2)-rgb(b,2)));
                sn++;
            }
            for (int y = 0; y + 1 < ch; y++) for (int x = 0; x < cw; x++) {
                unsigned a = shuf[y*cw+x], b = shuf[(y+1)*cw+x];
                sadj += (unsigned)(abs(rgb(a,0)-rgb(b,0)) + abs(rgb(a,1)-rgb(b,1)) + abs(rgb(a,2)-rgb(b,2)));
                sn++;
            }
            return d_adj / ((double)sadj / (double)sn);
        };
        double c_ti = coh(got, (int)g_tex_w, (int)g_tex_h);
        double c_flat = coh(canary_flat, (int)g_tex_w, (int)g_tex_h);
        printf("  P2-ORCL coherence: TI 8x4 readback = %.4f | FLAT-decode CANARY = %.4f (threshold 0.500; noise ~1.0)\n",
               c_ti, c_flat);
        CHECK(c_ti < 0.5 && c_flat >= 0.5,
              "L7-P4 coherence gate (P2-ORCL): genuine readback TI %s (<0.5) AND planted flat-decode CANARY %s (>=0.5) — the wrong layout FAILS the gate, the era TI layout passes (board: /tmp/orcl/grid.png, owner-verified)",
              c_ti < 0.5 ? "PASSES" : "FAILS", c_flat >= 0.5 ? "FAILS-as-wrong-decode OK" : "PASSES-UNEXPECTEDLY");
    } else {
        CHECK(1, "L7-P4 coherence gate SKIPPED (fixture regime — metric defined for genuine supply only)");
    }
#endif

    /* ================= P2-FRMB L8: genuine logo_e frames legs ==============
     * Hook-gated (F6): GX_FRAMES_GVM + GX_FRAMES_SLICES. Fixture regime
     * (no hook) prints nothing and counts nothing — 13/13 + 10/10 untouched. */
    const char *frames_env = getenv("GX_FRAMES_GVM");
    const char *slices_env = getenv("GX_FRAMES_SLICES");
    if (frames_env && !gvr_env) {
        printf("  L8 note: GX_FRAMES_GVM set without GX_LOGO_GVR genuine supply — "
               "regime combo unsupported, L8 legs SKIPPED (not counted)\n");
    }
    if (frames_env && gvr_env) {
        static u8 *fvm = 0; static unsigned long fn_sz = 0;
        if (!fvm) fvm = pso_arena_base + 0x1680000;   /* supply arena; clear of +0x1600000/+0x1640000 */
        FILE *ff = fopen(frames_env, "rb");
        if (!ff) { fprintf(stderr, "frames: GX_FRAMES_GVM set but unreadable\n"); return 2; }
        fseek(ff, 0, SEEK_END); fn_sz = (unsigned long)ftell(ff); fseek(ff, 0, SEEK_SET);
        if (fn_sz == 0 || fn_sz > 0x80000 || fread(fvm, 1, fn_sz, ff) != fn_sz) {
            fprintf(stderr, "frames: local gvm too large/short (%lu)\n", fn_sz); return 2; }
        fclose(ff);
        unsigned long long arch_fnv = p2an_fnv1a64(fvm, fn_sz);
        int sc_ok = 0;
        if (slices_env) {
            FILE *sf = fopen(slices_env, "rb");
            if (sf) {
                static char sj[262144];
                size_t sn = fread(sj, 1, sizeof sj - 1, sf); fclose(sf);
                sj[sn] = 0;
                sc_ok = slices_parse(sj);
            }
        }
        printf("  L8-S genuine supply: local logo_e gvm %lu bytes fnv1a64 = 0x%016llX "
               "(path withheld by policy; bytes never committed)%s\n",
               fn_sz, arch_fnv, sc_ok ? "" : " — SIDECAR MISSING (reference legs degraded)");
        CHECK(sc_ok && fn_sz == sc_arch_size && arch_fnv == sc_arch_fnv,
              "L8-S1 supply receipt: archive size+fnv == runtime-reference sidecar (no compiled-in genuine hash; F6.1)");
        int w_ok = frames_walk_gvm(fvm, fn_sz);
        CHECK(w_ok && l8_walk_chunks == LOGO_E_COUNT && l8_walk_end == fn_sz,
              "L8-S2 era walk: GVMH hdr 0x3a8/flags 0x000F/count 24, %d GVRT chunks, walk end 0x%lX == file end (F0 invariant)",
              l8_walk_chunks, l8_walk_end);
        int dir_ok = w_ok, pattern_ok = w_ok;
        for (int k = 0; dir_ok && k < LOGO_E_COUNT; k++) {
            if (l8_dir_attr[k] != (u32)k || l8_dir_fmt[k] != 0x000E || l8_dir_gbix[k] != logo_e_gbix((unsigned)k)) dir_ok = 0;
            unsigned want = (k % 8) < 4 ? 256u : 128u;      /* F5 pattern law */
            if (l8_dir_dw[k] != want || l8_dir_dh[k] != want || l8_w[k] != want || l8_h[k] != want) pattern_ok = 0;
            if (l8_plen[k] != (unsigned long)(want * want / 2)) pattern_ok = 0;   /* cmpR w*h/2 */
        }
        CHECK(dir_ok && pattern_ok,
              "L8-W1 dir/descriptor: attr==stream index x24, fmt 0xE, dims dir==desc==F5 pattern (0x8010/0x2010 => w*h/2 CMPR law), GBIX == F2-pinned LOGO_E_GBIX_* {0x1B0A8,0x1AFC2,0x10CAB4}+k");
        CHECK(l8_tlut_neg == LOGO_E_COUNT,
              "L8-W2 palette legs explicitly SKIPPED: is_palette_texture_format(14)==0 => tlut_name==-1 on all %d binds (no GVPL bank, no BB-3 normalization — F4/I4 §S4.1)", l8_tlut_neg);
        int x_ok = sc_ok;
        for (int k = 0; x_ok && k < LOGO_E_COUNT; k++)
            if (l8_chunk_fnv[k] != sc_fnv[k] || strcmp(sc_name[k], l8_texname_str[k])) x_ok = 0;
        CHECK(x_ok, "L8-X1 per-slice fnv equality: 24/24 GVRT chunk hashes (incl. 8 B hdr) == sidecar table loaded AT RUNTIME (F5 regime, byte-identity, disc-free)");
        /* dedupe by fnv over the 24 records (gate-side, F4): expect 16 unique,
         * dup groups {x7, x3} — structural counts, values measured at runtime */
        int uniq = 0, g7 = 0, g3 = 0;
        unsigned long long uniq_fnv[LOGO_E_COUNT]; int uniq_cnt[LOGO_E_COUNT];
        for (int k = 0; k < LOGO_E_COUNT; k++) {
            int f = -1;
            for (int u = 0; u < uniq; u++) if (uniq_fnv[u] == l8_chunk_fnv[k]) { f = u; break; }
            if (f < 0) { uniq_fnv[uniq] = l8_chunk_fnv[k]; uniq_cnt[uniq] = 1; uniq++; }
            else uniq_cnt[f]++;
        }
        for (int u = 0; u < uniq; u++) { if (uniq_cnt[u] == 7) g7++; if (uniq_cnt[u] == 3) g3++; }
        CHECK(uniq == 16 && g7 == 1 && g3 == 1,
              "L8-D1 fnv dedupe: %d distinct hashes across 24 records, dup groups x7 (gc_adx07/sega_e05-07/st05-07) + x3 (slot 07/15/23 tail) — F0 correction set-math; binding stays per-slot (era keeps separate surfaces, F2)",
              uniq);

        /* ---- draw + readback: one logo_e frame (gc_adx01) + one sega frame
         * (sega_logo_e01), 256x256 CMPR, on the SAME 1:1 quad the sinbd
         * genuine supply sized (BB-4). Host-seam direct call (DEVIATION
         * FRM-3: after the L7 era-chain legs complete, the era frame chain
         * has already presented once this process — the second present uses
         * the identical render_tick replay, era replay stays verbatim). */
        unsigned char *frgb = (unsigned char *)malloc(256u * 256u * 4);
        unsigned char *cgrid = (unsigned char *)malloc(256u * 256u * 4);
        int p1_ok = frgb && cgrid && g_tex_w == 256 && g_tex_h == 256;
        unsigned long long reg_h[2] = { 0, 0 };
        int escape_hits[2] = { 0, 0 };
        const int slots[2] = { 0, 8 };
        for (int s = 0; p1_ok && s < 2; s++) {
            int k = slots[s];
            if (l8_w[k] != 256 || l8_h[k] != 256) { p1_ok = 0; break; }
            gxh_cmpr_expand_ti8x8(l8_px[k], frgb, 256, 256);
            gx_dl_set_texture(256, 256, frgb);
            render_tick();
            gxh_dl_flush();
            int NT2 = 256 * 256;
            for (int j = 0; j < 256; j++) for (int i = 0; i < 256; i++) {
                int kk = j * 256 + i;
                xs[kk] = (unsigned)(QX0 + (i + 0.5) * (QX1 - QX0) / 256.0);
                ys[kk] = (unsigned)(QY0 + (j + 0.5) * (QY1 - QY0) / 256.0);
                const unsigned char *tx = frgb + (size_t)kk * 4;
                want[kk] = ((unsigned)tx[0] << 24) | ((unsigned)tx[1] << 16) |
                           ((unsigned)tx[2] << 8) | tx[3];
            }
            if (gxh_dl_sample(xs, ys, got, NT2) != 0) { p1_ok = 0; break; }
            int mism = 0;
            for (int kk = 0; kk < NT2; kk++) if (got[kk] != want[kk]) mism++;
            reg_h[s] = p2an_fnv1a64(got, (size_t)NT2 * 4);
            if (mism) p1_ok = 0;
            /* drawn coverage (F6.5 BB-4 shape): non-black OR matches its
             * unpacked transparent texel; escape counted and PRINTED (F6.5's
             * escape==0 assumption was pinned under the superseded I4
             * reading — CMPR mode c0<=c1 index 3 is genuinely transparent,
             * this is the documented era-truth deviation class of FRM-1) */
            int nz = 0;
            for (int yy = 0; yy < 256; yy++) for (int xx = 0; xx < 256; xx++) {
                int kk = yy * 256 + xx;
                (void)xx; (void)yy;
                if (got[kk] != 0) nz++;
                else if (want[kk] == 0) { nz++; escape_hits[s]++; }
            }
            if (nz != 256 * 256) p1_ok = 0;
            if (s == 1) {   /* coherence + planted I4-TI canary on the SEGA tile */
                gxh_i4_ti8x8_gray_expand(l8_px[k], cgrid, 256, 256);
                static unsigned canary_w[256 * 256];
                for (int kk = 0; kk < 256 * 256; kk++) {
                    const unsigned char *tx = cgrid + (size_t)kk * 4;
                    canary_w[kk] = ((unsigned)tx[0] << 24) | ((unsigned)tx[1] << 16) |
                                   ((unsigned)tx[2] << 8) | tx[3];
                }
                double c_cmpr = p2frmb_coherence(got, 256, 256);
                double c_i4 = p2frmb_coherence(canary_w, 256, 256);
                printf("  L8-P4 coherence (sega_logo_e01 readback): CMPR = %.4f | planted I4-TI CANARY = %.4f "
                       "(oracle python: 0.159 / 0.835; threshold 0.500)\n", c_cmpr, c_i4);
                CHECK(c_cmpr < 0.5 && c_i4 >= 0.5,
                      "L8-P4 CMPR-vs-I4 coherence gate: genuine sega_logo_e01 readback decodes coherent under the CMPR TI 8x8 law (<0.5) AND the superseded manifest-\"I4\" decode FAILS the same metric (>=0.5) — owner-verified board tiles sega_e01/e02");
            }
        }
        CHECK(p1_ok, "L8-P1/P2 draw+readback: gc_adx01 + sega_logo_e01 drawn 1:1 on the (256 x 256) BB-4 quad, ALL %d cell centers == CMPR-expanded texels ±0; coverage full (escape-proven transparent texels: adx %d, sega %d — CMPR alpha-0, FRM-1 class)",
              256 * 256, escape_hits[0], escape_hits[1]);
        printf("  L8-P3 drawn region fnv1a64 (runtime readback): gc_adx01 = 0x%016llX | sega_logo_e01 = 0x%016llX "
               "(self-consistency anchor against the runtime-expanded reference; no compiled-in expected hash)\n",
               reg_h[0], reg_h[1]);
        CHECK(reg_h[0] != 0 && reg_h[1] != 0 && reg_h[0] != reg_h[1] &&
              sc_ok && l8_chunk_fnv[0] != l8_chunk_fnv[8],
              "L8-P3 hash equality posture: readback regions present, brand blocks distinct, packed slices distinct (equality leg is the ±0 map above)");
        /* 128x128 unique-frame decode law (CPU; TEXN=256 shader variant
         * cannot address a 128 grid — draw is 256-regime only, BB-4):
         * decode consumes EXACTLY w*h/2 bytes for slot 4 (gc_adx05). */
        CHECK(l8_w[4] == 128 && l8_h[4] == 128 && l8_plen[4] == 128u * 128u / 2,
              "L8-P5 128px byte law: gc_adx05 desc/dir 128x128, payload %lu == w*h/2 CMPR (F5 pattern; dup slots ride X1 fnv equality)", l8_plen[4]);

        /* ---- FR-H handle identity (F1/F3) ---- */
        logo_texture_e_handle = &texture_logo_e_descriptor;      /* register_texture_logo_e */
        load_sonic_logo_texture();
        load_adx_logo_texture();
        int id_ok = logo_texture_e_handle == &texture_logo_e_descriptor &&
                    sonic_logo_object == logo_texture_e_handle &&
                    adx_logo_texture == logo_texture_e_handle &&
                    texture_logo_e_descriptor.nbTexture == LOGO_E_COUNT;
        for (int k = 0; id_ok && k < LOGO_E_COUNT; k++)
            if (l8_texnames[k].filename != l8_texname_str[k] || l8_texnames[k].attr != (u32)k ||
                l8_texnames[k].texture != &l8_states[k]) id_ok = 0;
        logo_texture_e_handle = 0;      /* TLogo::free_held_object leg */
        CHECK(id_ok && logo_texture_e_handle == 0 &&
              sonic_logo_object == &texture_logo_e_descriptor &&
              adx_logo_texture == &texture_logo_e_descriptor,
              "L8-H1 handle identity: register fills all 24 slots {name,attr,bound state}; ADX+SEGA+ST share ONE descriptor pointer; post-free handle NULL with the two EMPTY release twins intact (F3 era bodies)");

        /* ================= P2-SEQ L9: GX_FRAMES_EXIT sega-logo scene legs ======
         * ERA MODEL (evidence: docs/p2-seq-1.md §1, all @6b38b53837):
         * render_logo_with_default(logo_texture_e_handle, 8, -1, -150.0f)
         * (TAdSEGAlogo.cpp:67) == draw_title_background_quads mode 0
         * (TitleBackground.cpp:58-60): ONE frame draws EIGHT quads — the
         * static mosaic title_background_tiles[8] (TitleBackground.cpp:43-52),
         * each via set_draw_texture_and_color(tex_base + tiles[i].tex, -1)
         * (:29) => slices sega_logo_e01..e08 (list indices 8..15) composited
         * as 4x256x256 + 4x128x128 tiles (640x512 layout, VI clips y>=480).
         * The index arg is a FAMILY BASE, not a final-slice pick: ADX base 0
         * (TAdADXlogo.cpp:33), SEGA base 8, Sonic base 16 (TAdSoniclogo.cpp:30)
         * share the same descriptor + tile table (TLogo chain: TLogo.cpp:169-189).
         * The ONLY per-frame animation in the SEGA scene is the TScreenFade
         * black overlay alpha (TScreenFadeController.cpp:30-59; overlay drawn
         * under tl_fade, AFTER the tl_00 scene sequencers — TMainTask.h:48/65;
         * parent arg ignored, both ctors attach to tl_fade :61/:85). Ledger:
         * FadeIn 255-8t (32 ticks) -> request_destroy => state 1
         * (TAdSEGAlogo.cpp:61-64); hold ++m_wait_frames > 60.0f (61 ticks, :74);
         * FadeOut 0+8t (32 ticks) -> destroy_and_mark_done =>
         * sega_logo_done_flag=1 (:56-59, consumed TLogo.cpp:60/170).
         * DEVIATIONS SEQ-1 (host tick = 1 immediate present vs era 30 Hz
         * retrace gate), SEQ-2 (fade composited CPU-side: the host pipeline
         * has no blend stage; the (0,8)/(1,6) pair == set_default_alpha_blend,
         * render_util.cpp:286, black src => out = dst*(255-a) rounded), SEQ-3
         * (mosaic presented through the TEXN=256 canvas; native-res mosaic is
         * CPU-verified ±0 in L9-C2). Zero compiled-in genuine hashes: every
         * fnv here is runtime-self (L7-P3/L8 posture). */
        if (getenv("GX_FRAMES_EXIT")) {
            printf("\n== P2-SEQ L9: sega-logo scene regime (GX_FRAMES_EXIT) ==\n");
            /* L9-C1: tile table verbatim (TitleBackground.cpp:43-52):
             * {x,y,w,h,tex}; draw order i=0..7 is right-column-first,
             * disjoint => order-insensitive geometry. */
            static const int seq_tiles[8][5] = {
                { 0x200, 0x180, 0x80, 0x80, 7 }, { 0x200, 0x100, 0x80, 0x80, 6 },
                { 0x200, 0x80,  0x80, 0x80, 5 }, { 0x200, 0x0,   0x80, 0x80, 4 },
                { 0x0,   0x100, 0x100, 0x100, 1 },{ 0x0,   0x0,   0x100, 0x100, 0 },
                { 0x100, 0x100, 0x100, 0x100, 3 },{ 0x100, 0x0,   0x100, 0x100, 2 } };
            int seen[LOGO_E_COUNT]; memset(seen, 0, sizeof seen);
            long area_total = 0, area_vis = 0;
            int c1_ok = 1;
            for (int i = 0; i < 8; i++) {
                int k = 8 + seq_tiles[i][4];
                if (++seen[k] != 1) c1_ok = 0;                  /* each once   */
                area_total += (long)seq_tiles[i][2] * seq_tiles[i][3];
                int vx = seq_tiles[i][2], vy = seq_tiles[i][3];
                if (seq_tiles[i][0] + vx > 640) vx = 640 - seq_tiles[i][0];
                if (seq_tiles[i][1] + vy > 480) vy = 480 - seq_tiles[i][1];
                area_vis += (long)vx * vy;
                for (int j = 0; j < i; j++)                     /* disjoint     */
                    if (seq_tiles[i][0] < seq_tiles[j][0] + seq_tiles[j][2] &&
                        seq_tiles[j][0] < seq_tiles[i][0] + seq_tiles[i][2] &&
                        seq_tiles[i][1] < seq_tiles[j][1] + seq_tiles[j][3] &&
                        seq_tiles[j][1] < seq_tiles[i][1] + seq_tiles[i][3]) c1_ok = 0;
            }
            for (int k = 0; k < LOGO_E_COUNT; k++)
                if ((k >= 8 && k < 16) ? seen[k] != 1 : seen[k] != 0) c1_ok = 0;
            int c1_cover_ok = 1;   /* disjoint tiles + 480 clip cover 640x480? */
            { static unsigned char cover[640 * 480]; memset(cover, 0, sizeof cover);
              for (int i = 0; i < 8; i++)
                  for (int y = seq_tiles[i][1]; y < seq_tiles[i][1] + seq_tiles[i][3] && y < 480; y++)
                      for (int x = seq_tiles[i][0]; x < seq_tiles[i][0] + seq_tiles[i][2] && x < 640; x++)
                          cover[y * 640 + x] = 1;
              for (int p = 0; c1_cover_ok && p < 640 * 480; p++) if (!cover[p]) c1_cover_ok = 0; }
            CHECK(c1_ok && area_total == 327680L && area_vis == 640L * 480L && c1_cover_ok,
                  "L9-C1 mosaic law: tex_base 8 selects the WHOLE sega family — 8 disjoint tiles map indices 8..15 exactly once (4x256x256 + 4x128x128 = %ld slice texels), the 640x512 era layout clipped to the 640x480 VI field covers every screen pixel (TitleBackground.cpp:43-52)",
                  area_total);

            /* L9-C2: CPU composite at NATIVE resolution — every slice expanded
             * CMPR TI8x8, every visible texel ±0 against its own expansion. */
            static unsigned seq_raster[640 * 480];
            memset(seq_raster, 0, sizeof seq_raster);       /* era clear = black */
            long tile_mism = 0, tile_px = 0;
            int dim_ok = frgb != 0;
            for (int i = 0; dim_ok && i < 8; i++) {
                int k = 8 + seq_tiles[i][4];
                if (l8_w[k] != (unsigned)seq_tiles[i][2] || l8_h[k] != (unsigned)seq_tiles[i][3]) { dim_ok = 0; break; }
                gxh_cmpr_expand_ti8x8(l8_px[k], frgb, (unsigned)seq_tiles[i][2], (unsigned)seq_tiles[i][3]);
                for (int ty = 0; ty < seq_tiles[i][3]; ty++) {
                    int sy = seq_tiles[i][1] + ty; if (sy >= 480) continue;
                    for (int tx = 0; tx < seq_tiles[i][2]; tx++) {
                        int sx = seq_tiles[i][0] + tx; if (sx >= 640) continue;
                        const unsigned char *p = frgb + ((size_t)ty * seq_tiles[i][2] + tx) * 4;
                        seq_raster[sy * 640 + sx] = ((unsigned)p[0] << 24) | ((unsigned)p[1] << 16)
                                                  | ((unsigned)p[2] << 8) | p[3];
                        tile_px++;
                    }
                }
                for (int ty = 0; ty < seq_tiles[i][3]; ty++) {   /* re-read ±0  */
                    int sy = seq_tiles[i][1] + ty; if (sy >= 480) continue;
                    for (int tx = 0; tx < seq_tiles[i][2]; tx++) {
                        int sx = seq_tiles[i][0] + tx; if (sx >= 640) continue;
                        const unsigned char *p = frgb + ((size_t)ty * seq_tiles[i][2] + tx) * 4;
                        unsigned w = ((unsigned)p[0] << 24) | ((unsigned)p[1] << 16)
                                   | ((unsigned)p[2] << 8) | p[3];
                        if (seq_raster[sy * 640 + sx] != w) tile_mism++;
                    }
                }
                printf("  L9 stage texel fnv1a64: %s (%dx%d expanded) = 0x%016llX (runtime-self)\n",
                       l8_texname_str[k], seq_tiles[i][2], seq_tiles[i][3],
                       (unsigned long long)p2an_fnv1a64(frgb, (size_t)seq_tiles[i][2] * seq_tiles[i][3] * 4));
            }
            CHECK(dim_ok && tile_mism == 0 && tile_px == 640L * 480L,
                  "L9-C2 native mosaic: 8/8 slices compose the 640x480 presented field, %ld/%ld visible texels == own CMPR expansion ±0 (screen fnv 0x%016llX, runtime-self)",
                  tile_px, tile_px, (unsigned long long)p2an_fnv1a64(seq_raster, sizeof seq_raster));

            /* L9-G1: present the composite through the TEXN=256 pipeline as the
             * ENTRY frame (L8-P2 remains the entry regime). Canvas texel (i,j)
             * = raster at cell-center screen px; the uint2-read shader maps
             * screen px (x,y) -> texel (floor((2x+1)*256/1280), floor((2y+1)*256/960)). */
            static unsigned seq_tex[256 * 256];
            int seq_xc[256], seq_yc[256];     /* margin-safe sample px per cell */
            { int bij_ok = 1;
              /* cell i = screen px with floor((2x+1)/5)==i  (x-span (5i-1)/2..);
               * cell j = floor((2y+1)*4/15)==j (y-span (15j-2)/8..). Pick the px
               * with LARGEST distance of uv*256 to an integer boundary. */
              for (int i = 0; i < 256; i++) {
                  int a = (i == 0 ? 0 : (5 * i - 1 + 1) / 2), b = (5 * (i + 1) - 1) / 2;
                  if (5 * i - 1 > 0 && (5 * i - 1) % 2) a = (5 * i - 1) / 2 + 1;
                  int best = -1; double bm = -1;
                  for (int x = a; x <= b && x < 640; x++) {
                      int r = (2 * x + 1) % 5; if (!r) continue;
                      double m = r > 2 ? 5 - r : r;          /* /5 units */
                      if (m > bm) { bm = m; best = x; }
                  }
                  if (best < 0) { bij_ok = 0; best = a; }
                  seq_xc[i] = best;
                  if (256 * (2 * best + 1) / 1280 != i) bij_ok = 0;
              }
              for (int j = 0; j < 256; j++) {
                  int a = (15 * j + 5) / 8, b = (15 * j + 20) / 8 - 1;
                  int best = -1; double bm = -1;
                  for (int y = a; y <= b && y < 480; y++) {
                      int r = (2 * y + 1) * 4 % 15; if (!r) continue;
                      double m = r > 7 ? 15 - r : r;          /* /15 units */
                      if (m > bm) { bm = m; best = y; }
                  }
                  if (best < 0) { bij_ok = 0; best = a; }
                  seq_yc[j] = best;
                  if (256 * (2 * best + 1) / 960 != j) bij_ok = 0;
              }
              for (int j = 0; j < 256; j++) for (int i = 0; i < 256; i++)
                  seq_tex[j * 256 + i] = seq_raster[seq_yc[j] * 640 + seq_xc[i]];
              if (!bij_ok) printf("  L9-G1 WARNING: cell-map bijection failed\n"); }
            static unsigned char seq_texb[256 * 256 * 4];   /* byte-staged upload */
            for (int k = 0; k < 256 * 256; k++) {
                unsigned w = seq_tex[k];
                seq_texb[k * 4] = (unsigned char)(w >> 24); seq_texb[k * 4 + 1] = (unsigned char)(w >> 16);
                seq_texb[k * 4 + 2] = (unsigned char)(w >> 8); seq_texb[k * 4 + 3] = (unsigned char)w;
            }
            gx_dl_set_texture(256, 256, seq_texb);
            QX0 = 0; QY0 = 0; QX1 = 640; QY1 = 480;          /* full-screen quad */
            render_tick();
            gxh_dl_flush();
            /* sample grid for every later leg: one margin-safe px per cell */
            for (int j = 0; j < 256; j++) for (int i = 0; i < 256; i++) {
                int k = j * 256 + i; xsf[k] = (unsigned)seq_xc[i]; ysf[k] = (unsigned)seq_yc[j];
                full[k] = seq_tex[k];
            }
            { int mism = 0; static unsigned g1[256 * 256];
              if (gxh_dl_sample(xsf, ysf, g1, 256 * 256) != 0) return 4;
              for (int k = 0; k < 256 * 256; k++) if (g1[k] != full[k]) {
                  mism++;
                  if (mism <= 10) printf("  L9-G1 dbg cell(%d,%d) px(%d,%d) want 0x%08X got 0x%08X\n",
                                         k % 256, k / 256, xsf[k], ysf[k], full[k], g1[k]);
              }
              mkdir("/tmp/cla", 0755);
              int prc = gxh_dl_dump_png("/tmp/cla/seq_sega_composite.png");
              FILE *pf = fopen("/tmp/cla/seq_sega_composite.png", "rb");
              long psz = 0; if (pf) { fseek(pf, 0, SEEK_END); psz = ftell(pf); fclose(pf); }
              printf("  L9-G1 composite readback fnv1a64 = 0x%016llX | canvas fnv1a64 = 0x%016llX (runtime-self pair)\n",
                     (unsigned long long)p2an_fnv1a64(g1, sizeof g1),
                     (unsigned long long)p2an_fnv1a64(seq_tex, sizeof seq_tex));
              CHECK(mism == 0 && prc == 0 && psz > 0,
                    "L9-G1 entry composite on screen: all %d/65536 canvas cells resolved ±0 at margin-safe centers through the uint2 read map, PNG artifact /tmp/cla/seq_sega_composite.png (%ld bytes) — the SEGA-logo scene entry frame",
                    256 * 256 - mism, psz);
              memcpy(full, g1, sizeof g1); }   /* keep composite readback ref   */

            /* L9-U1: ERA-PINNED HUE-BAND gate on the entry present (card SEQ3,
             * fleet/defect-dig-20261127-sega.md follow-up). The SEQ2 ±0 gate
             * was CPU-model vs GPU — BOTH sides of the same transform, so a
             * self-consistent white-keyed-black model could never be caught
             * here. This leg measures the PRESENTED canvas against a reference
             * computed AT GATE TIME from the decoded slices themselves (era
             * CMPR/RGB5A3 expansion — the law pinned by the three-way
             * byte-exact agreement host C / oracle python / probe on the same
             * bytes, receipt §1), never from the model under test:
             *   (a) opaque-white field: white texels (r,g,b >= 240, a == 255)
             *       >= 90% of visible texels on BOTH sides, presented within
             *       3 pts of the decode reference (receipt §3: 94% of the era
             *       art is opaque white background; GC-CMPR keying ruled out,
             *       index-3 <= 0.2%);
             *   (b) chromatic tally R:B within [4,16] on the presented canvas
             *       and within 2x of the decode reference (magenta wordmark
             *       dominates blue; receipt §2 — a channel-inverted model
             *       lands at ~0.12 and FAILS);
             *   (c) 'Presented by:' helper-line BLUE PRESENT: >= 150 blue-
             *       dominant cells measured (native 640x480 tally 1557-1738
             *       B-dominant rescales to ~340 at the 256x256 canvas). */
            struct hue_stats { long vis, white, rdom, bdom, at0; };
            const double WHITENE = 240.0;   /* >= per channel (receipt #3 tally law) */
            struct hue_stats rfs = {0,0,0,0,0}, pes = {0,0,0,0,0};
            { const unsigned *pp = seq_raster; const unsigned *qq = full;
              for (long p = 0; p < 640L * 480L; p++) {
                  unsigned w = pp[p];
                  int r = (int)((w >> 24) & 0xFF), g = (int)((w >> 16) & 0xFF),
                      b = (int)((w >> 8) & 0xFF), a = (int)(w & 0xFF);
                  if (!a) { rfs.at0++; continue; }
                  rfs.vis++;
                  if (r >= WHITENE && g >= WHITENE && b >= WHITENE && a == 255) rfs.white++;
                  if (r > b + 32) rfs.rdom++; else if (b > r + 32) rfs.bdom++;
              }
              for (long p = 0; p < 256L * 256L; p++) {
                  unsigned w = qq[p];
                  int r = (int)((w >> 24) & 0xFF), g = (int)((w >> 16) & 0xFF),
                      b = (int)((w >> 8) & 0xFF), a = (int)(w & 0xFF);
                  if (!a) { pes.at0++; continue; }
                  pes.vis++;
                  if (r >= WHITENE && g >= WHITENE && b >= WHITENE && a == 255) pes.white++;
                  if (r > b + 32) pes.rdom++; else if (b > r + 32) pes.bdom++;
              } }
            double wf_ref = rfs.vis ? (double)rfs.white / (double)rfs.vis : 0.0,
                   wf_pre = pes.vis ? (double)pes.white / (double)pes.vis : 0.0,
                   rb_ref = rfs.bdom ? (double)rfs.rdom / (double)rfs.bdom : 0.0,
                   rb_pre = pes.bdom ? (double)pes.rdom / (double)pes.bdom : 0.0;
            printf("  L9-U1 era decode reference: visible=%ld white=%ld (%.2f%%) R-dom=%ld B-dom=%ld R:B=%.2f\n",
                   rfs.vis, rfs.white, 100.0 * wf_ref, rfs.rdom, rfs.bdom, rb_ref);
            printf("  L9-U1 presented canvas   : visible=%ld white=%ld (%.2f%%) R-dom=%ld B-dom=%ld R:B=%.2f\n",
                   pes.vis, pes.white, 100.0 * wf_pre, pes.rdom, pes.bdom, rb_pre);
            CHECK(wf_ref >= 0.90 && wf_pre >= 0.90 &&
                  (wf_pre > wf_ref ? wf_pre - wf_ref : wf_ref - wf_pre) <= 0.03 &&
                  rb_pre >= 4.0 && rb_pre <= 16.0 &&
                  rb_ref > 0.0 && rb_pre >= rb_ref / 2.0 && rb_pre <= rb_ref * 2.0 &&
                  pes.bdom >= 150,
                  "L9-U1 hue-band gate (era-pinned, not model-derived): presented white-field %.2f%% of visible vs decode reference %.2f%% (era floor 90%%, delta cap 3 pts), presented R:B %.2f in band [4,16] and within 2x of decode reference %.2f, helper-line blue present (%ld cells >= 150) — a white-keyed or channel-inverted present CANNOT pass (defect-dig 20261127 #3)",
                  100.0 * wf_pre, 100.0 * wf_ref, rb_pre, rb_ref, pes.bdom);

            /* L9-U2 keyed-white CANARY: the forbidden model itself — the same
             * composite with the opaque white field keyed out over black (the
             * defect class the SEQ2 gate could not see). Present it through
             * the REAL pipeline, measure with the SAME U1 legs, and prove the
             * white-field assertion REJECTS it; then restore the correct
             * canvas so the ledger/fade legs keep running on era truth. */
            { static unsigned seq_key[256 * 256];
              static unsigned char keyb[256 * 256 * 4];
              for (int p = 0; p < 256 * 256; p++) {
                  unsigned w = seq_tex[p];
                  int r = (int)((w >> 24) & 0xFF), g = (int)((w >> 16) & 0xFF),
                      b = (int)((w >> 8) & 0xFF), a = (int)(w & 0xFF);
                  if (r >= WHITENE && g >= WHITENE && b >= WHITENE && a == 255)
                      w = 0xFF000000u;                    /* keyed out to black */
                  seq_key[p] = w;
                  keyb[p * 4] = (unsigned char)(w >> 24); keyb[p * 4 + 1] = (unsigned char)(w >> 16);
                  keyb[p * 4 + 2] = (unsigned char)(w >> 8); keyb[p * 4 + 3] = (unsigned char)w;
              }
              gx_dl_set_texture(256, 256, keyb);
              render_tick(); gxh_dl_flush();
              static unsigned kr[256 * 256];
              if (gxh_dl_sample(xsf, ysf, kr, 256 * 256) != 0) return 4;
              struct hue_stats ks = {0,0,0,0,0};
              for (long p = 0; p < 256L * 256L; p++) {
                  unsigned w = kr[p];
                  int r = (int)((w >> 24) & 0xFF), g = (int)((w >> 16) & 0xFF),
                      b = (int)((w >> 8) & 0xFF), a = (int)(w & 0xFF);
                  if (!a) { ks.at0++; continue; }
                  ks.vis++;
                  if (r >= WHITENE && g >= WHITENE && b >= WHITENE && a == 255) ks.white++;
                  if (r > b + 32) ks.rdom++; else if (b > r + 32) ks.bdom++;
              }
              double wf_can = ks.vis ? (double)ks.white / (double)ks.vis : 0.0;
              printf("  L9-U2 canary (white keyed over black, GPU-presented): visible=%ld white=%ld (%.2f%%) R-dom=%ld B-dom=%ld — U1 floor verdict = %s\n",
                     ks.vis, ks.white, 100.0 * wf_can, ks.rdom, ks.bdom,
                     wf_can >= 0.90 ? "WOULD PASS (gate broken)" : "REJECTED");
              CHECK(wf_can < 0.90,
                    "L9-U2 keyed-white canary FAILS the U1 leg as required: the black-background model presents only %.2f%% white field vs the era 90%% floor — self-consistent black present can never pass this gate again",
                    100.0 * wf_can);
              /* restore the correct canvas + present state for L9-H1/F1 */
              gx_dl_set_texture(256, 256, seq_texb);
              render_tick(); gxh_dl_flush(); }

            /* L9-H1: era tick ledger + 61-tick hold, scene-static proof. */
            f32 af = 255.0f; int tin = 0;
            for (;;) { af = af - 8.0f; tin++; if (af <= 0.0f) break; }        /* 32 */
            int hold = 0;
            for (;;) { if ((f32)++hold > 60.0f) break; }                       /* 61 */
            f32 ao = 0.0f; int tout = 0;
            for (;;) { ao += 8.0f; tout++; if (ao >= 255.0f) break; }          /* 32 */
            long hold_mism = 0; unsigned long long fh[3] = { 0, 0, 0 };
            for (int t = 1; t <= hold; t++) {
                render_tick(); gxh_dl_flush();
                for (int j = 0; j < 8; j++) for (int i = 0; i < 8; i++) {
                    int m = j * 8 + i; xs[m] = (unsigned)(40 + i * 80); ys[m] = (unsigned)(30 + j * 60);
                }
                static unsigned hs[64];
                if (gxh_dl_sample(xs, ys, hs, 64) != 0) return 4;
                for (int m = 0; m < 64; m++) {
                    unsigned exp_px = seq_tex[((2 * ys[m] + 1) * 256 / 960) * 256 + (2 * xs[m] + 1) * 256 / 1280];
                    if (hs[m] != exp_px) hold_mism++;
                }
                if (t == 1 || t == 31 || t == hold) fh[t == 1 ? 0 : (t == 31 ? 1 : 2)] = p2an_fnv1a64(hs, sizeof hs);
            }
            printf("  L9-H1 pacing: era tick = 30 Hz retrace gate (present_interval=2, gjSystem.c:34/71/:108-116 + docs/boot-spine.md S7/§2.2) => ledger 32+61+32 ticks ~= %.2fs; host tick = 1 immediate CAMetalLayer present per render_tick (SEQ-1, no retrace wait — counts identical, wallclock differs)\n",
                   (tin + hold + tout) / 30.0);
            CHECK(tin == 32 && hold == 61 && tout == 32 && hold_mism == 0 &&
                  fh[0] != 0 && fh[0] == fh[1] && fh[1] == fh[2],
                  "L9-H1 hold ledger: fade-in sim DERIVES 32 ticks (-8/tick from 255), state1 holds %d ticks (++m_wait_frames > 60.0f), fade-out derives 32 (+8/tick to 255); %d-tick hold scene-static (64-pt grid ±0 every tick, fnv 0x%016llX stable t=1/31/%d)",
                  hold, hold, fh[0], hold);

            /* L9-F1: fade-out composite leg — CPU model (SEQ-2) per era tick,
             * GPU-verified at the t=16 checkpoint and the terminal black frame. */
            static unsigned seq_mix[256 * 256];
            long lum_prev = -1, mono_ok = 1; int f1_mism16 = 0, f1_final_bad = 0;
            unsigned long long ffnv16 = 0, ffnv32 = 0; int seq_done_flag = 0;
            for (int t = 1; t <= tout; t++) {
                int al = 8 * t; if (al > 255) al = 255;
                long lum = 0;
                for (int p = 0; p < 256 * 256; p++) {
                    unsigned w = seq_tex[p];
                    int r = (int)((w >> 24) & 0xFF), g = (int)((w >> 16) & 0xFF),
                        b = (int)((w >> 8) & 0xFF), a = (int)(w & 0xFF);
                    int nr = (r * (255 - al) + 127) / 255, ng = (g * (255 - al) + 127) / 255,
                        nb = (b * (255 - al) + 127) / 255, na = al + (a * (255 - al) + 127) / 255;
                    seq_mix[p] = ((unsigned)nr << 24) | ((unsigned)ng << 16) | ((unsigned)nb << 8) | (unsigned)na;
                    seq_texb[p * 4] = (unsigned char)nr; seq_texb[p * 4 + 1] = (unsigned char)ng;
                    seq_texb[p * 4 + 2] = (unsigned char)nb; seq_texb[p * 4 + 3] = (unsigned char)na;
                    lum += nr + ng + nb;
                }
                if (lum_prev >= 0 && lum > lum_prev) mono_ok = 0;
                lum_prev = lum;
                gx_dl_set_texture(256, 256, seq_texb);
                render_tick(); gxh_dl_flush();
                if (t == 16 || t == tout) {
                    static unsigned fr[256 * 256];
                    for (int k = 0; k < 256 * 256; k++) fr[k] = seq_mix[k];
                    if (gxh_dl_sample(xsf, ysf, full, 256 * 256) != 0) return 4;
                    for (int k = 0; k < 256 * 256; k++) if (full[k] != fr[k]) {
                        if (t == 16) f1_mism16++; else f1_final_bad++;
                    }
                    if (t == 16) ffnv16 = p2an_fnv1a64(full, (size_t)256 * 256 * 4);
                    else         ffnv32 = p2an_fnv1a64(full, (size_t)256 * 256 * 4);
                }
                if (t == tout && ao >= 255.0f) seq_done_flag = 1;   /* destroy_and_mark_done */
            }
            int f1_black = 1;
            for (int k = 0; k < 256 * 256; k++) if ((full[k] >> 8) != 0) { f1_black = 0; break; }
            mkdir("/tmp/cla", 0755);
            int prc2 = gxh_dl_dump_png("/tmp/cla/seq_sega_faded.png");
            FILE *pf2 = fopen("/tmp/cla/seq_sega_faded.png", "rb");
            long psz2 = 0; if (pf2) { fseek(pf2, 0, SEEK_END); psz2 = ftell(pf2); fclose(pf2); }
            printf("  L9-F1 readback fnv1a64: t=16 (a=128) = 0x%016llX | t=%d (a=255) = 0x%016llX (runtime-self)\n",
                   ffnv16, tout, ffnv32);
            CHECK(f1_mism16 == 0 && f1_final_bad == 0 && f1_black && mono_ok &&
                  seq_done_flag == 1 && prc2 == 0 && psz2 > 0,
                  "L9-F1 fade-out leg: %d ticks alpha=min(8t,255); GPU readback == CPU black-overlay model ±0 at t=16 AND terminal; terminal frame all-RGB-black; luminance non-increasing; sega_logo_done_flag model set at clamp tick; PNG /tmp/cla/seq_sega_faded.png (%ld bytes)",
                  tout, psz2);

            /* L9-K1 canaries: WRONG base and WRONG pacing MUST fail. */
            { static unsigned seq_can[256 * 256];
              /* (a) index canary: compose with tex_base 0 (the ADX family) on
               * the same tile table; a correct scene must NOT match it. */
              for (int p = 0; p < 8; p++) {
                  int k = seq_tiles[p][4];                    /* base 0         */
                  gxh_cmpr_expand_ti8x8(l8_px[k], frgb, (unsigned)seq_tiles[p][2], (unsigned)seq_tiles[p][3]);
                  for (int ty = 0; ty < seq_tiles[p][3]; ty++) {
                      int sy = seq_tiles[p][1] + ty; if (sy >= 480) continue;
                      for (int tx = 0; tx < seq_tiles[p][2]; tx++) {
                          int sx = seq_tiles[p][0] + tx; if (sx >= 640) continue;
                          const unsigned char *q = frgb + ((size_t)ty * seq_tiles[p][2] + tx) * 4;
                          /* place into the canvas cell the screen px lands in   */
                          seq_raster[sy * 640 + sx] = ((unsigned)q[0] << 24) | ((unsigned)q[1] << 16)
                                                    | ((unsigned)q[2] << 8) | q[3];
                      }
                  }
              }
              for (int j = 0; j < 256; j++) for (int i = 0; i < 256; i++)
                  seq_can[j * 256 + i] = seq_raster[seq_yc[j] * 640 + seq_xc[i]];
              long cx_mism = 0;
              for (int p = 0; p < 256 * 256; p++) if (seq_can[p] != seq_tex[p]) cx_mism++;
              /* (b) pacing canary: limit 30.0f holds 31 != 61; step 1.0f at
               * tick 32 reaches alpha 32 < 255 (never completes the fade). */
              int hold30 = 0; for (;;) { if ((f32)++hold30 > 30.0f) break; }
              f32 a1 = 0.0f; int t32_bad = 0;
              for (int t = 1; t <= 32; t++) { a1 += 1.0f; if (a1 >= 255.0f) t32_bad = 1; }
              printf("  L9-K1 canary measurements: base-0 canvas mismatches vs evidence = %ld/65536; 30.0f-limit hold = %d ticks; +1.0f-step fade at tick 32 complete = %d\n",
                     cx_mism, hold30, t32_bad);
              CHECK(cx_mism > 0 && hold30 != hold && !t32_bad,
                    "L9-K1 canaries FAIL AS REQUIRED: the wrong family base (0 == ADX, TAdADXlogo.cpp:33) breaks the composite map (%ld cells differ), 30.0f pacing derives %d != 61 hold ticks, and a 1.0f/tick fade cannot reach 255 by the era's 32nd tick — only the evidenced model passes",
                    cx_mism, hold30);
            }
        }
    }

    if (failures == 0 && getenv("GX_DL_DEBUG")) {
        gxh_dl_dump("/tmp/logo_dump.ppm");
        printf("  debug: backbuffer dumped to /tmp/logo_dump.ppm\n");
    }
    printf("== P2-BA logo: %d/%d PASS ==\n", total - failures, total);
    gxh_dl_flush();
    return failures == 0 ? 0 : 1;
}
