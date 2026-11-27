/* P2-AN boot ladder rung 4 — TEXTURE SUPPLY CHAIN to the upload doorstep.
 *
 * Extends AE's rung-3 GSL chain one full link further: the quest path
 * 0x801EB5EC `bl load_sin_board_image` hands the decoded bytes to the GVR
 * walk (text_803D96A8 find_gvrt_texture_chunk, home 0x803D9C98). This rung
 * supplies a synthetic GVR off fake_disc through the REAL era walk and
 * lands the populated texture descriptor + byte-true texel buffer at the
 * exact shape the next gx lane (Z-lineage quad) consumes.
 *
 * Era bodies run VERBATIM through guarded generated slices:
 *   find_gsl_entry (AE's ladder3_gsl_glue.o)  +  find_gvrt_texture_chunk
 *   (new ladder4_gvr_glue.o)  +  move_data (Y's move_data_glue.o).
 * Everything else is this harness. ORACLE = the PPC disassembly
 * build/GPOE8P/asm/pso @ 6b38b5383, cited addr+instruction at each CHECK,
 * plus H's byte tables (docs/p1-format-net-disc.md section 2.1 D1-D4 and
 * compat/README-byteswap.md section 2). Fixture bytes come from
 * fake_disc/gen_gvr_fixture.py (byte map in docs/p2-tex-supply.md).
 *
 * ORACLE PROVENANCE (find_gvrt_texture_chunk, unk/text_803D96A8.s):
 *  :9CC0 cmplwi r28,0 / :9CC8 li r0,-0x1 / :9CCC stw r0,0(r28)  sentinel
 *  :9CD0 lis 0x5849 / :9CD4 addi 0x4247                  GBIX work=0x58494247
 *  :9CD8 lwz r0,0(r29) / :9CE0-:9CE4 bl swap_gvrt_word   tag = swap(*p)
 *  :9CEC cmpw r4,r30 / :9CF0 beq .L_803D9D24 (GBIX) / :9CF4 bge .L_803D9D5C
 *  :9CF8-:9CFC tag2 = 0x54525647 / :9D00 cmpw / :9D04 beq GVRT / :9D08 b err
 *  GVRT :9D0C mr r3,r31(dst) / :9D10 addi r4,r29,8 / :9D14 li r5,8 / :9D18
 *       bl move_data__FPvPCvl   -> desc = 8 RAW bytes at chunk+8 (UNSWAPPED)
 *       :9D1C addi r3,r29,0x10  -> returns chunk+16 = texel pointer
 *  GBIX :9D24 lwz r0,4(r29) / :9D28 addi r29,r29,8 / :9D30 bl swap(&len)
 *       :9D40 lwz r0,0(r29) / :9D44 stw r0,0(r28)  -> id RAW, UNSWAPPED
 *       :9D48 srwi r0,2 / :9D4C slwi r0,2 / :9D54 add r29,r29,r0
 *       -> p = &id + (len & ~3) bytes
 *  err  :9D5C li r0,4 / :9D60-:9D64 lis/addi texture_error / :9D68 stw r0,8
 *       / :9D6C li r3,0                          -> code@+8, return NULL
 *  :9D74-:9D80 loop-again iff tag != 0x54525647 (subis/cmplwi pair)
 * (load_gvm_texture_archive, text_803DD5E0.cpp: GVMH cmp 0x484D5647 :415,
 *  chunk magic/size swap sites :436-438, texel-run length `size - 8` :442.)
 */
#include "pso_bringup.h"
#include "ladder3_harness.h"              /* GslArchiveEntry, AdxfHandle     */
#include "pso/unk/text_803DA798.h"        /* REAL GvrTextureDesc declaration  */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#define AOFF offsetof

/* ---- era slice (generated glue TU) ---- */
struct TLobby;
extern u32 *find_gvrt_texture_chunk(TLobby *lobby, u32 *out_id, u32 *data);
extern GslArchiveEntry *find_gsl_entry(const char *name, s32 unused);
extern AdxfHandle *gsl_adxf_handle; extern GslArchiveEntry gsl_archive_entries[256];
extern s32 gsl_entry_count;

/* ---- ladder4_supply.c (C11 TU bound to compat/pso_byteswap.h) ---- */
extern "C" {
uint32_t p2an_prot32(uint32_t v);  uint16_t p2an_prot16(uint16_t v);
uint32_t p2an_be32(uint32_t v);    uint16_t p2an_be16(uint16_t v);
uint32_t p2an_prot_read32(const void *img);
uint32_t p2an_be_read32(const void *img);
uint16_t p2an_be_read16(const void *img);
uint32_t p2an_era_mirror32(uint32_t v);
uint64_t p2an_fnv1a64(const void *buf, size_t n);
}

static int failures, total;
#define CHECK(cond, ...) do { total++; if (!(cond)) { failures++; \
    printf("FAIL "); printf(__VA_ARGS__); printf("\n"); } \
    else { printf("PASS "); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* era global from the prelude (glue TU; symbol unmangled plain global in
 * the Itanium ABI). Shadow of include/pso/render/GinjaTexture.h:6-14 —
 * all-s32 fields, host layout identical; code@8 proven by the era store
 * 0x803D9D68 `stw r0,0x8(r3)` (asserted in L0.3). */
typedef struct GinjaTextureError {
    s32 entry_index; s32 pad_04; s32 code; s32 pad_0C; s32 pad_10; s32 pad_14[3];
} GinjaTextureError;
extern "C" GinjaTextureError texture_error;

/* Host mirror of the era GVR-chunk header (GvrTextureChunk,
 * text_803DD5E0.h:21-25 == magic4+size4+desc8); the walk treats the image
 * as u32 words, so this is layout arithmetic only. */
typedef struct AnGvrChunkHdr { u32 magic; u32 size; GvrTextureDesc desc; } AnGvrChunkHdr;

/* Supply pack = what sits on the upload doorstep: the era-shaped texture
 * image header GinjaTextureHeader (GinjaMaterialState.h:13-22, host shadow)
 * filled from the decoded GVR, plus the identity the walk handed back. */
typedef struct AnTexSupply {
    u32 hdr_format;   /* GinjaTextureHeader.format   @0x00 */
    u32 hdr_w, hdr_h; /*                        width@0x0C height@0x10 */
    u32 hdr_size;     /*                        size  @0x14 (GXGetTexBufferSize) */
    u32 hdr_flags;    /*                        flags @0x18 (0x8000 colour-indexed) */
    u8 *pixels;       /*                        pixels@0x1C */
    u32 global_id;    /* GBIX surface id (-> surface->id, text_803DA798.cpp:143) */
    s32 tlut_name;    /* palettized => tlut kept (text_803DA798.cpp:147-151)     */
} AnTexSupply;

static const u8 k_gvr_sig[4] = {'G','V','R','T'};

int main() {
    setvbuf(stdout, 0, _IOLBF, 0);   /* goldens survive a crash */
    if (pso_bringup_install() != 0) { fprintf(stderr, "arena install failed\n"); return 2; }
    OSInit();
    DVDInit();

    /* ============ L0: era layout mirrors (compile-time facts, pinned) ============ */
    CHECK(sizeof(GvrTextureDesc) == 8 && AOFF(GvrTextureDesc, format) == 0
          && AOFF(GvrTextureDesc, width) == 4 && AOFF(GvrTextureDesc, height) == 6,
          "L0.1 GvrTextureDesc 8B/0/4/6 == REAL header text_803DA798.h:11-15 == walk li r5,0x8 + addi r4,r29,0x8 (0x803D9D10/:9D14)");
    CHECK(sizeof(AnGvrChunkHdr) == 0x10,
          "L0.2 GvrTextureChunk magic4+size4+desc8 = 0x10 == texel return `addi r3,r29,0x10` (0x803D9D1C)");
    CHECK(sizeof(GinjaTextureError) == 0x20 && AOFF(GinjaTextureError, code) == 8,
          "L0.3 texture_error.code @+8 == `stw r0,0x8(r3)` (0x803D9D68) + GinjaTexture.h:14 size 0x20");
    CHECK(AOFF(AnGvrChunkHdr, size) == 4 && AOFF(AnGvrChunkHdr, desc) == 8,
          "L0.4 chunk size@+4 desc@+8 == GBIX len lwz 0x4(r29) (0x803D9D24) + desc src addi r29,8 (0x803D9D10)");

    /* ============ A: fixture truth off fake_disc (AE chain extended) ============ */
    u8 *raw = pso_arena_base + 0x30000;   /* AE's table slot */
    DVDFileInfo fi; memset(&fi, 0, sizeof fi);
    CHECK(DVDOpen((char *)"gsl_tab2.dat", &fi), "A0 DVDOpen(gsl_tab2.dat) via Y shim");
    s32 tab_len = DVDReadPrio(&fi, raw, (s32)fi.length, 0, 2); DVDClose(&fi);
    CHECK(tab_len == 4 * (s32)sizeof(GslArchiveEntry),
          "A1 gsl_tab2.dat == 4 x 0x30 entries (got %d) — AE rows 0..2 verbatim + sinbd_tex.gvr row", tab_len);
    memcpy(gsl_archive_entries, raw, (size_t)tab_len);
    gsl_entry_count = 4; gsl_adxf_handle = (AdxfHandle *)0x1;   /* DA-4 host fill */
    GslArchiveEntry *e = find_gsl_entry((const char *)"sinbd_tex.gvr", 0);
    CHECK(e == &gsl_archive_entries[3] && e->file_size == 0x41C && e->decoded_size == 0x41C,
          "A2 era find_gsl_entry hit @idx3 (:DDC8 addi 1 / :DDCC addi r31,r31,0x30): file_size=decoded_size=0x41C");
    memset(raw, 0xCC, 4096);
    CHECK(DVDOpen((char *)"sinbd_tex.gvr", &fi) && fi.length == 0x41C,
          "A3 DVDOpen(sinbd_tex.gvr): shim length == entry.file_size 0x41C (D1 working-value overlay, rung regime DA-4)");
    s32 got = DVDReadPrio(&fi, raw, 0x41C, 0, 2); DVDClose(&fi);
    CHECK(got == 0x41C && raw[0x41C] == 0xCC && raw[0x1C] == 0x00 && raw[0x1D] == 0x07
          && raw[0x1E] == 0x0E && raw[0x1F] == 0x05,
          "A4 disc read %d/0x41C bytes: first texel word 00 07 0E 05 == gen_gvr_fixture formula, canary raw[0x41C] untouched 0xCC", got);
    u64 img_h0 = p2an_fnv1a64(raw, 0x41C);
    CHECK(memcmp(raw, "GBIX", 4) == 0 && memcmp(raw + 0x0C, k_gvr_sig, 4) == 0,
          "A5 IMAGE magic bytes are DC-legacy ASCII 'GBIX'/'GVRT' (H D3: bswap'd fields; psoProt regime)");
    CHECK(p2an_prot_read32(raw + 0x04) == 4 && p2an_prot_read32(raw + 0x10) == 0x408,
          "A6 GBIX len==4, GVRT size==0x408 read psoProt (identity seam): size-8 == 1024 texels == `gvm_stream_read(chunk->size - 8)` text_803DD5E0.cpp:442");
    CHECK(p2an_be_read32(raw + 0x08) == 0x00000999,
          "A7 GBIX id bytes BE 00 00 09 99 -> psoBeRead == PPC raw-lwz working 0x999 (:9D40 `lwz r0,0(r29)` UNSWAPPED)");
    CHECK(p2an_be_read32(raw + 0x14) == 9 && p2an_be_read16(raw + 0x18) == 32
          && p2an_be_read16(raw + 0x1A) == 32,
          "A8 desc bytes BE: format 9 (is_palette_texture_format cases 8/9/10 — TAdSinglePlyChrSelectGC_803DA038.cpp:26-33), w32 h32");

    /* ============ B: swap-regime proof + the pinned GVRT trap ============ */
    u32 ppc_raw = p2an_be_read32(raw + 0x0C);      /* PPC `lwz` of 'GVRT'  */
    CHECK(ppc_raw == 0x47565254u && p2an_era_mirror32(ppc_raw) == 0x54525647u
          && p2an_prot_read32(raw + 0x0C) == 0x54525647u,
          "B1 era chain lwz+mirror(0x803D9CD8+0x803D9C60) == psoProtRead32 on LE host — BOTH give working 0x54525647 (README-byteswap.md sec 2: keep the swap site as identity seam)");
    CHECK(p2an_be32(p2an_prot_read32(raw + 0x0C)) == 0x47565254u
          && p2an_be32(p2an_prot_read32(raw + 0x0C)) != 0x54525647u,
          "B2 TRAP pinned: adding psoBe32 at the era-swap'd magic double-swaps -> 0x47565254, walk would fall to default err4 (:9D5C) — README sec 2 'does NOT add psoBe* there'");
    CHECK(p2an_prot16((uint16_t)0x2000) == 0x2000 && p2an_be16(0x2000) == 0x0020,
          "B3 desc u16 halfword regime: BE bytes 00 20 -> working 32 via psoBe16 (raw-copy fields take the ADD-swap side of the split, like D1)");

    /* ============ C: REAL era walk (verbatim slice) over the decoded bytes ============ */
    GvrTextureDesc desc_raw; memset(&desc_raw, 0xCC, sizeof desc_raw);
    u32 id_raw = 0;
    u8 *pixels = (u8 *)find_gvrt_texture_chunk((TLobby *)&desc_raw, &id_raw, (u32 *)raw);
    CHECK(pixels == raw + 0x1C,
          "C1 era find_gvrt_texture_chunk(&desc,&id,img) -> img+0x1C : loop crossed GBIX then hit GVRT, texel return `addi r3,r29,0x10` (:9D1C)");
    CHECK(id_raw == 0x99090000u && p2an_be32(id_raw) == 0x00000999u,
          "C2 GBIX id raw host value 0x%08X (era UNSWAPPED stw :9D44) — psoBe fixup at supply boundary (AN-2) -> working 0x999 == PPC", id_raw);
    CHECK(memcmp(&desc_raw, raw + 0x14, 8) == 0,
          "C3 desc bytes byte-true: era move_data copied the raw 8 (:9D18) — walk is a BYTE mover, not a decoder");
    u32 wformat = p2an_be32(desc_raw.format);
    u16 wwidth  = p2an_be16(desc_raw.width), wheight = p2an_be16(desc_raw.height);
    CHECK(wformat == 9 && wwidth == 32 && wheight == 32,
          "C4 working descriptor recovered: {format %u, width %u, height %u} == PPC-view of the same bytes (H D4 payload rule: keep byte-true; convert only the ints)",
          (unsigned)wformat, (unsigned)wwidth, (unsigned)wheight);

    /* sentinel leg: walk starting AT the GVRT chunk never touches out_id */
    u32 id2 = 0xFFFFFFFFu;
    GvrTextureDesc d2;
    u8 *px2 = (u8 *)find_gvrt_texture_chunk((TLobby *)&d2, &id2, (u32 *)(raw + 0x0C));
    CHECK(px2 == raw + 0x1C && id2 == 0xFFFFFFFFu,
          "C5 no-GBIX leg: sentinel li r0,-0x1/stw (:9CC8-:9CCC) survives — 0xFFFFFFFF is endian-neutral; caller keeps global_index on the sentinel (`(u32)id != 0xFFFFFFFF`, text_803DD5E0.cpp:146-148)");

    /* error legs (both switch branches into :9D5C) */
    u8 bad_lo[16]; memset(bad_lo, 0xCC, sizeof bad_lo);
    memcpy(bad_lo, "ABCD", 4);
    u8 bad_hi[16]; memset(bad_hi, 0xCC, sizeof bad_hi);
    memcpy(bad_hi, "ZZZZ", 4);
    u32 id3; GvrTextureDesc d3;
    texture_error.code = 0;
    u32 *r1 = find_gvrt_texture_chunk((TLobby *)&d3, &id3, (u32 *)bad_lo);
    CHECK(r1 == 0 && texture_error.code == 4,
          "C6 unknown tag 0x44434241 (<GBIX, cmpw :9CEC / :9CF8-:9D08 b): NULL + texture_error.code=4 store `stw r0,0x8(r3)` (:9D68)");
    u32 *r2 = find_gvrt_texture_chunk((TLobby *)&d3, &id3, (u32 *)bad_hi);
    CHECK(r2 == 0 && texture_error.code == 4,
          "C7 unknown tag 0x5A5A5A5A (>GBIX, bge :9CF4 edge): same err4 exit — walk is total, no unbounded scan");

    /* ============ D: the supply pack at the upload doorstep ============ */
    AnTexSupply pack; memset(&pack, 0xCC, sizeof pack);
    pack.hdr_format = wformat; pack.hdr_w = wwidth; pack.hdr_h = wheight;
    pack.hdr_size = (u32)(p2an_prot_read32(raw + 0x10) - 8);   /* :442 rule */
    pack.hdr_flags = 0x8000;   /* colour-indexed (GinjaMaterialState.h:19) —
                                  justified by is_palette_texture_format(9)=1 */
    pack.pixels = pixels;
    pack.global_id = p2an_be32(id_raw);
    pack.tlut_name = 0;        /* tlut kept for palette fmt (text_803DA798.cpp:147-151) */
    CHECK(pack.hdr_size == 1024 && pack.hdr_size == pack.hdr_w * pack.hdr_h,
          "D1 size field - 8 == 1024 == w*h (PAL8 = 1 byte/texel, AN-4 assumption: init_texture_header_and_alloc(u8*,s32,s32,s32) def NOT in decomp @6b38b5383 — decl text_803D96A8.cpp:23)");
    CHECK(pixels == pack.pixels && (pack.hdr_flags & 0x8000) != 0,
          "D2 pack pixels@0x1C-shape == GinjaTextureHeader.pixels (GinjaMaterialState.h:21); 0x8000 colour-indexed bit set for fmt 9");

    /* texel payload: byte-true through the whole chain (D4 FORMAT-KEEP) */
    u64 tex_h = p2an_fnv1a64(pack.pixels, pack.hdr_size);
    int pat_ok = 1;
    for (u32 i = 0; i < pack.hdr_size; i++) {
        u32 x = i % pack.hdr_w, y = i / pack.hdr_w;
        if (pack.pixels[i] != (u8)(((x * 3 + y * 5) ^ ((x ^ y) << 2)) & 0xFF)) { pat_ok = 0; break; }
    }
    CHECK(pat_ok && tex_h == 0xa92d59be5839fce5ull,
          "D3 texel buffer byte-true + STABLE: fnv1a64=0x%016llX pinned (gen_gvr_fixture.py formula, all 1024 checked in-TU)", tex_h);
    CHECK(p2an_fnv1a64(raw, 0x41C) == img_h0 && img_h0 == 0x93b9be4e18606d07ull,
          "D4 era walk is READ-ONLY on the image: full-0x41C-byte fnv1a64 unchanged 0x%016llX; desc staging wrote exactly 8 bytes (canary raw[0x41C]=0x%02X)", img_h0, raw[0x41C]);

    printf("\n== P2-AN TEXTURE SUPPLY PACK (upload doorstep, what the Z-lineage quad consumes) ==\n");
    printf("  GinjaTextureHeader-shape: { format=%u, width=%u, height=%u, size=%u, flags=0x%04X,\n"
           "                              pixels=%p(img+0x1C byte-true) }  id=0x%08X  tlut=%d\n",
           pack.hdr_format, pack.hdr_w, pack.hdr_h, pack.hdr_size, pack.hdr_flags,
           (void *)pack.pixels, pack.global_id, (int)pack.tlut_name);
    printf("  texels fnv1a64=0x%016llX  image fnv1a64=0x%016llX  (stable across fresh runs)\n", tex_h, img_h0);
    printf("  consumer: load_texture_from_memory (attr 0x40000000 -> loader==1 in\n"
           "  load_texture_entry's `attr & 0x5C000000` switch -> texture_loaders[1],\n"
           "  text_803DD5E0.cpp:26-58/:120) -> bind_texlist_texture -> create_texture_state\n"
           "  -> init_texture_header_and_alloc + reload_texture_slot_pixels — the GX upload\n"
           "  itself is the next lane's door; this pack is the last pure-logic artifact.\n");

    printf("== P2-AN ladder4: %d/%d PASS ==\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
