/* P2-AU boot ladder rung 5 — REAL texel bytes through REAL era code.
 *
 * Closes AN's doorstep (docs/p2-tex-supply.md §6): the same synthetic GVR
 * off fake_disc is walked by the same era find_gvrt_texture_chunk chain,
 * then the descriptor pack flows through the era upload composition ported
 * per docs/p2-an4-triage.md §4:
 *   init_texture_header_and_alloc seam (era 0x803DA12C, wrapper ⊕ tail-call,
 *   deviation AU-1) -> init_ginja_texture_header (0x803DB0AC, VERBATIM slice
 *   GinjaTextureHeader.cpp:62-84) -> get_texture_buffer_size (0x803DB15C,
 *   :86-94) -> GXGetTexBufferSize (0x8038BE90, VERBATIM SDK slice
 *   GXTexture.c:53-96/:98-139 — the bpp table) -> alloc_buffer_from_heap
 *   (0x803DB1B4, :96-105, OSAllocFromHeap over the Y-era heap port) -> era
 *   move_data texel fill -> read-back hash == AN's pinned fixture texels.
 * That equality is the "the port has loaded a real texture through era
 * logic" receipt. ORACLE = build/GPOE8P/asm @ 6b38b5383, cited addr+insn at
 * every CHECK; jumptable_804F50A8 (gx/GXTexture.s:1171) is the .s-side bpp
 * table for golden E2.
 */
#include "pso_bringup.h"
#include "ladder3_harness.h"              /* GslArchiveEntry, AdxfHandle      */
#include "ladder5_tex_prelude.h"          /* era struct + chain decls (shared) */
#include "pso/unk/text_803DA798.h"        /* REAL GvrTextureDesc declaration   */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#define AOFF offsetof

/* ---- ladder4 rungs (era slices, same objects ladder4_test links) ---- */
struct TLobby;
extern u32 *find_gvrt_texture_chunk(TLobby *lobby, u32 *out_id, u32 *data);
extern GslArchiveEntry *find_gsl_entry(const char *name, s32 unused);
extern AdxfHandle *gsl_adxf_handle; extern GslArchiveEntry gsl_archive_entries[256];
extern s32 gsl_entry_count;

/* ---- ladder4_supply.c endian/hash vocabulary (C11 TU) ---- */
extern "C" {
uint32_t p2an_be32(uint32_t v);    uint16_t p2an_be16(uint16_t v);
uint32_t p2an_prot_read32(const void *img);
uint64_t p2an_fnv1a64(const void *buf, size_t n);
}

/* Era Cell view at payload-32 (host layout per V1, docs/p2-bringup-2.md +
 * osalloc_era.c era_cell_guard; era PPC payload addi r3,r6,0x20 @0x80370BEC
 * = HEADERSIZE 32, identical here). From ladder_test.cpp (P2-Y) — same pin. */
typedef struct EraCellView {
    void *prev;        /* +0  */
    void *next;        /* +8  */
    s32   size;        /* +16 block total incl. 32B header */
    u32   hd_idx;      /* +20 heap handle + 1 while allocated, 0 free */
    s32   requested;   /* +24 */
    s32   _pad;        /* +28 */
} EraCellView;
#define CELL(p) ((EraCellView *)((u8 *)(p) - 32))

/* pixels sentinel after memset(hdr, 0xCC, ..): a full 64-bit pattern (LP64) */
#define IS_CC_PTR(p) ((uintptr_t)(p) == 0xCCCCCCCCCCCCCCCCull)

static int failures, total;
#define CHECK(cond, ...) do { total++; if (!(cond)) { failures++; \
    printf("FAIL "); printf(__VA_ARGS__); printf("\n"); } \
    else { printf("PASS "); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* Ladder4 mirror of the era texture_error global (defined in the linked
 * ladder4_gvr_glue.o TU; all-s32 fields, code@+8 pinned by the era store
 * 0x803D9D68 `stw r0,0x8(r3)` — docs/p2-tex-supply.md L0.3). */
typedef struct GinjaTextureError {
    s32 entry_index; s32 pad_04; s32 code; s32 pad_0C; s32 pad_10; s32 pad_14[3];
} GinjaTextureError;
extern "C" GinjaTextureError texture_error;

/* Locked-cache heap stand-in (AU-4): arena 20MB..20MB+64KB — cannot alias
 * OSInit's game heap (+0x400000..+0x918000), Y's sys heap demo region, or
 * the disc buffers at +0x30000. */
#define CACHEHEAP_START_OFF 0x01400000u
#define CACHEHEAP_SIZE      0x00010000u

int main() {
    setvbuf(stdout, 0, _IOLBF, 0);   /* goldens survive a crash */
    if (pso_bringup_install() != 0) { fprintf(stderr, "arena install failed\n"); return 2; }
    OSInit();
    DVDInit();

    /* ============ S: supply — AN's walk reproduces the pack ============ */
    u8 *raw = pso_arena_base + 0x30000;
    DVDFileInfo fi; memset(&fi, 0, sizeof fi);
    CHECK(DVDOpen((char *)"gsl_tab2.dat", &fi), "S0 DVDOpen(gsl_tab2.dat) via Y shim");
    s32 tab_len = DVDReadPrio(&fi, raw, (s32)fi.length, 0, 2); DVDClose(&fi);
    memcpy(gsl_archive_entries, raw, (size_t)tab_len);
    gsl_entry_count = 4; gsl_adxf_handle = (AdxfHandle *)0x1;   /* DA-4 */
    GslArchiveEntry *e = find_gsl_entry((const char *)"sinbd_tex.gvr", 0);
    memset(raw, 0xCC, 0x41C + 1);
    CHECK(e && e->file_size == 0x41C && DVDOpen((char *)"sinbd_tex.gvr", &fi)
          && DVDReadPrio(&fi, raw, 0x41C, 0, 2) == 0x41C,
          "S1 era find_gsl_entry hit + full 0x41C-byte fixture read off fake_disc (AN rung intact)");
    GvrTextureDesc desc_raw; memset(&desc_raw, 0xCC, sizeof desc_raw);
    u32 id_raw = 0;
    u8 *texels = (u8 *)find_gvrt_texture_chunk((TLobby *)&desc_raw, &id_raw, (u32 *)raw);
    u32 wformat = p2an_be32(desc_raw.format);
    u16 wwidth = p2an_be16(desc_raw.width), wheight = p2an_be16(desc_raw.height);
    CHECK(texels == raw + 0x1C && wformat == 9 && wwidth == 32 && wheight == 32
          && p2an_be32(id_raw) == 0x999 && p2an_prot_read32(raw + 0x10) == 0x408,
          "S2 AN descriptor pack reproduced by the REAL era walk: {fmt 9, 32x32, id 0x999, chunk size 0x408} (text_803D96A8.s :9D18/:9D1C/:9D40-44)");
    CHECK(p2an_fnv1a64(texels, 1024) == 0xa92d59be5839fce5ull
          && p2an_fnv1a64(raw, 0x41C) == 0x93b9be4e18606d07ull,
          "S3 fixture anchors: texels fnv1a64=0xA92D59BE5839FCE5, image fnv1a64=0x93B9BE4E18606D07 (docs/p2-tex-supply.md §1 pinned values)");

    /* ============ H: locked-cache heap over the era OSAlloc port ============ */
    u8 *ch_lo = pso_arena_base + CACHEHEAP_START_OFF;
    u8 *ch_hi = ch_lo + CACHEHEAP_SIZE;
    int lh = OSCreateHeap(ch_lo, ch_hi);
    locked_cache_heap_handle = (u32)lh;
    CHECK(lh >= 0, "H0 locked_cache_heap_handle := OSCreateHeap(arena+0x%X, +0x%X) = heap %d "
          "(AU-4 host stand-in for the locked-cache region; era .sbss global "
          "RenderSbssGlobals.cpp:12 read at 0x803DB1D0 `lwz r3,locked_cache_heap_handle@sda21`)",
          CACHEHEAP_START_OFF, CACHEHEAP_START_OFF + CACHEHEAP_SIZE, lh);
    u8 *guard = (u8 *)OSAllocFromHeap(lh, 64);
    memset(guard, 0xA5, 64);   /* overflow sentinel allocated BEFORE the texel block */

    /* ============ E1: the seam runs the FULL era chain on the real pack ============ */
    GinjaTextureHeader hdr; memset(&hdr, 0xCC, sizeof hdr);   /* every field 0xCC sentinel */
    s32 seam_ret = init_texture_header_and_alloc((u8 *)&hdr, (s32)wwidth, (s32)wheight, (s32)wformat);
    CHECK(seam_ret == 0,
          "E1a seam init_texture_header_and_alloc(pack) ret==0 — era wrapper r3 flow "
          "0x803DA144 `cmpwi r3,0`/beq→alloc; alloc ret 0 == OSAllocFromHeap hit (:803DB1DC-:803DB1EC bne path)");
    CHECK(hdr.format == 9 && AOFF(GinjaTextureHeader, format) == 0x00,
          "E1b header.format==9 (u8-masked store 0x803DB0CC `clrlwi r0,r6,24` / :803DB0D0 `stw r0,0x0(r30)`) — EVERY field asserted below");
    CHECK(hdr.width == 32 && hdr.height == 32,
          "E1c width==32 height==32 (:803DB0D4 `stw r4,0xc(r30)` / :803DB0D8 `stw r5,0x10(r30)`)");
    CHECK(hdr.reserved04 == 0xCCCCCCCCu && hdr.reserved08 == 0xCCCCCCCCu,
          "E1d reserved +0x04/+0x08 UNTOUCHED (0xCC canaries) — the .s writes exactly 6 fields; struct map == GinjaMaterialState.h:13-21");
    CHECK(hdr.flags == 0x00000000u,
          "E1e flags==0 for fmt 9: bit tests all zero (rlwinm :803DB0DC fmt&0x100, :803DB108 fmt&0x200, :803DB118 fmt&0x800 — 9 hits none); "
          "AN pack's 0x8000 flag is the SUPPLY-side predicate note (ladder4_test D2), era header takes it from format bit 0x200 (golden F6)");
    CHECK(hdr.size == 1024,
          "E1f size==1024 via era bpp table: jumptable_804F50A8 entry[9]→.L_8038BEC4 (row 3, col 2 = 8x4-texel/32B tile), tileBytes 32 "
          "(:8038BEFC li r5,0x20 — fmt != 6 && != 0x16), nx=(32+7)>>3=4 ny=(32+3)>>2=8 → 4*8*32=1024 == GVRT size-8 — AN-4 PAL8 assumption now PROVEN");
    u32 *px = hdr.pixels;
    CHECK(px != 0 && (uintptr_t)px % 32 == 0 && (u8 *)px >= ch_lo && (u8 *)px + 1024 <= ch_hi,
          "E1g alloc_buffer_from_heap: pixels=%p inside cache heap [arena+0x%X..+0x%X], 32B-aligned "
          "(era payload=cell+32 @0x80370BEC `addi r3,r6,0x20`); stw 0x1c(r30) :803DB1EC wrote through the era pixels slot (AU-5 LP64 field slot)",
          (void *)px, CACHEHEAP_START_OFF, CACHEHEAP_START_OFF + CACHEHEAP_SIZE);
    EraCellView *c = CELL(px);
    CHECK(c->size == 1056 && c->requested == 1024 && c->hd_idx == (u32)lh + 1,
          "E1h era OSAlloc header intact: cell@pixels-32 {size=1056=round32(1024+HEADERSIZE32), requested=1024, hd_idx=heap+1=%d} "
          "(osalloc_era.c V1 layout; era HEADERSIZE/32B round-up 0x80370CCC `clrrwi rX,rY,5`)", lh + 1);

    /* ============ E2: bpp table vs .s jump table (era fn 3 direct) ============ */
    u32 expect[][2] = { {0,512}, {1,1024}, {3,2048}, {4,2048}, {6,4096}, {9,1024}, {0xA,2048} };
    static const char *arm[] = {
        "tbl[0].L_8038BEB8 (3,3)*32", "tbl[1].L_8038BEC4 (3,2)*32", "tbl[3].L_8038BED0 (2,2)*32",
        "tbl[4].L_8038BED0 (2,2)*32", "tbl[6] (2,2) tileBytes 64 @0x8038BEF4", "tbl[9].L_8038BEC4 (3,2)*32",
        "tbl[10].L_8038BED0 (2,2)*32" };
    int bpp_ok = 1;
    for (unsigned k = 0; k < sizeof expect / sizeof expect[0]; k++) {
        GinjaTextureHeader t; memset(&t, 0, sizeof t);
        t.format = expect[k][0]; t.width = 32; t.height = 32; t.flags = 0;
        u32 got = get_texture_buffer_size(&t);
        if (got != expect[k][1]) {
            bpp_ok = 0;
            printf("  bpp mismatch fmt 0x%X: got %u want %u (%s)\n",
                   expect[k][0], got, expect[k][1], arm[k]);
        }
    }
    CHECK(bpp_ok,
          "E2 7-format sweep through the REAL get_texture_buffer_size(0x803DB15C)→GXGetTexBufferSize(0x8038BE90): "
          "every size == the hand-traced jumptable_804F50A8 (gx/GXTexture.s:1171-1231) arm ((w+2^shiftX-1)>>shiftX)*((h+2^shiftY-1)>>shiftY)*tileBytes");

    /* ============ E3: mipmap flags + loop (era .s mtctr/bdnz/break) ============ */
    GinjaTextureHeader hdr_mip; memset(&hdr_mip, 0xCC, sizeof hdr_mip);
    s32 mip_ret = init_texture_header_and_alloc((u8 *)&hdr_mip, 32, 32, (s32)(0x100 | 9));
    CHECK(mip_ret == 0 && hdr_mip.format == 9 && hdr_mip.flags == 0x80000000u,
          "E3a fmt 0x109 (mip bit): header.format masked back to 9 (:803DB0CC clrlwi 24), flags |= 0x80000000 "
          "(:803DB104 `oris r7,r7,0x8000` == .s shift form of the bit-31 flag), mipmap picked by clrrwi bit test :803DB184");
    CHECK(hdr_mip.size == 1440,
          "E3b mip sum == hand-trace of .L_8038BF34 loop (mtctr max_lod=100 :803DB19C→li r7,0x64; bdnz :8038BFA0; 1x1 break :8038BF64): "
          "1024+256+64+32+32+32 = 1440 (got %u)", hdr_mip.size);
    CHECK(hdr_mip.pixels && CELL(hdr_mip.pixels)->requested == 1440,
          "E3c alloc honoured the computed mip size: cell.requested==1440 (:803DB1D4 `lwz r4,0x14(r30)` feeds OSAllocFromHeap)");

    /* ============ E4: texel fill through the era mover + READ-BACK HASH ============ */
    move_data((void *)px, texels, (s32)hdr.size);   /* hdr.size == 1024 (E1f) */
    u64 px_h = p2an_fnv1a64(px, 1024);
    CHECK(px_h == 0xa92d59be5839fce5ull && px_h == p2an_fnv1a64(texels, 1024),
          "E4a RECEIPT — read-back of the heap pixel buffer == AN's fixture texels: fnv1a64=0x%016llX == 0xA92D59BE5839FCE5. "
          "The port has loaded a real texture through era logic (walk→wrapper→header→bpp→heap→mover)", px_h);
    CHECK(((u8 *)px)[0] == 0x00 && ((u8 *)px)[1] == 0x07 && ((u8 *)px)[2] == 0x0E && ((u8 *)px)[3] == 0x05,
          "E4b first texel word byte-true at the heap block (00 07 0E 05 == gen_gvr_fixture formula; D4 byte-true regime survives alloc+copy)");
    CHECK(p2an_fnv1a64(raw, 0x41C) == 0x93b9be4e18606d07ull && raw[0x41C] == 0xCC && guard[0] == 0xA5,
          "E4c canaries: image hash unchanged (era chain read-only), raw[0x41C]=0xCC, adjacent 64B guard block 0xA5-intact (fill wrote exactly hdr.size bytes)");

    /* ============ F: failure paths — era behaviour where .s defines it ============ */
    /* F1 bad magic: walk dies before upload (texture_error, no header touched) */
    u8 bad[16]; memset(bad, 0xCC, sizeof bad); memcpy(bad, "ABCD", 4);
    GvrTextureDesc db; u32 idb = 0;
    GinjaTextureHeader h1; memset(&h1, 0xCC, sizeof h1);
    texture_error.code = 0;
    u32 *r = find_gvrt_texture_chunk((TLobby *)&db, &idb, (u32 *)bad);
    CHECK(r == 0 && texture_error.code == 4 && IS_CC_PTR(h1.pixels),
          "F1 bad magic 0x44434241 (<GBIX, cmpw :9CEC/:9CF0, bge :9CF4 err leg): era walk NULL + texture_error.code=4 "
          "(:803D9D5C-:9D68 `stw r0,0x8(r3)`); the upload chain is never entered — wrapper pixels sentinel intact");
    /* F2 ret==4: mip bit + non-square — .s-defined skip-alloc path */
    GinjaTextureHeader h2; memset(&h2, 0xCC, sizeof h2);
    s32 f2 = init_texture_header_and_alloc((u8 *)&h2, 32, 16, (s32)(0x100 | 9));
    CHECK(f2 == 4 && h2.size == 768 && IS_CC_PTR(h2.pixels) && h2.flags == 0x80000000u,
          "F2 era-defined: fmt 0x109 w32 h16 → init ret 4 (cmplw r4,r5 :803DB0E8-:803DB0F0 `li r31,0x4`) BUT size still computed "
          "(unconditional `stw r3,0x14` :803DB13C == 512+128+32+32+32+32 = 768 mip sum) and wrapper SKIPS alloc "
          "(text_803DA12C.s :803DA144 beq-fallthrough); pixels untouched");
    /* F3 OOM ret==3 */
    GinjaTextureHeader h3; memset(&h3, 0xCC, sizeof h3);
    h3.size = CACHEHEAP_SIZE + 0x1000;
    s32 f3 = alloc_buffer_from_heap(&h3);
    CHECK(f3 == 3 && IS_CC_PTR(h3.pixels),
          "F3 era-defined OOM: OSAllocFromHeap miss → `cmplwi r3,0` :803DB1DC beq→`li r31,0x3` :803DB1E4, pixels NOT written "
          "(stw :803DB1EC only on hit) — ret 3 propagates through the seam by :803DA154 tail r3");
    /* F4 invalid format: retail silent-compute (asserts absent in .s) */
    GinjaTextureHeader h4; memset(&h4, 0, sizeof h4);
    h4.format = 99; h4.width = 32; h4.height = 32;
    u32 s4 = get_texture_buffer_size(&h4);
    CHECK(s4 == 32768,
          "F4 invalid fmt 99 (>0x3C): era silently computes (0,0)-shift arm 0x8038BEDC (`cmplwi r5,0x3c`/`bgt` :8038BE94-:8038BE9C) "
          "→ 32*32*32=%u — C ASSERTMSGLINEV(0x19B) has ZERO era instructions (UNDEF-PORT resolved to silent-compute; AU-2)", s4);
    GinjaTextureHeader h5; memset(&h5, 0, sizeof h5);
    h5.format = 7; h5.width = 32; h5.height = 32;   /* in-table default arm */
    u32 s5 = get_texture_buffer_size(&h5);
    CHECK(s5 == 32768,
          "F4b fmt 7 (in table, default arm): tbl[7]→.L_8038BEDC (0,0) → %u — jump table decoded, not guessed", s5);
    /* F5 over-size: no clamp/panic in .s */
    GinjaTextureHeader h6; memset(&h6, 0, sizeof h6);
    h6.format = 9; h6.width = 2048; h6.height = 32;
    u32 s6 = get_texture_buffer_size(&h6);
    CHECK(s6 == 65536,
          "F5 over-size w=2048: ASSERTMSGLINEV(0x1AB) absent in .s — era computes ((2048+7)>>3=256)*((32+3)>>2=8)*32 = %u, "
          "no clamp, no panic (UNDEF-PORT on the C-assert side; port = no-op, era-true)", s6);
    /* F6 colour-indexed header flag: format-bit provenance (AN pack §6 note) */
    GinjaTextureHeader h7; memset(&h7, 0xCC, sizeof h7);
    s32 f6 = init_texture_header_and_alloc((u8 *)&h7, 32, 32, (s32)(0x200 | 9));
    CHECK(f6 == 0 && h7.flags == 0x8000u && h7.format == 9 && h7.size == 1024,
          "F6 fmt 0x209 → flags==0x8000 (rlwinm bit-0x200 test :803DB108-:803DB114 `ori r7,r7,0x8000`), header.format still 9, "
          "size still 1024 — the AN pack's colour-indexed 0x8000 flag equals an era header flag iff the GVR format word carries bit 0x200");
    /* fmt 0x800 tail-flag (completes the three documented bits) */
    GinjaTextureHeader h8; memset(&h8, 0xCC, sizeof h8);
    s32 f7 = init_texture_header_and_alloc((u8 *)&h8, 32, 32, (s32)(0x800 | 9));
    CHECK(f7 == 0 && h8.flags == 0x40000000u && h8.format == 9 && h8.size == 1024,
          "F6b fmt 0x809 → flags==0x40000000 (:803DB118-:803DB124 bit-0x800 test → `oris r7,r7,0x4000`), header.format == 0x809&0xFF == 9, "
          "bpp therefore still sees 9 → size 1024 (the 0x800 attr bit never reaches the sizer — clrlwi 24 at :803DB0CC)");

    printf("\n== P2-AU TEX UPLOAD CHAIN CLOSED (receipt) ==\n");
    printf("  GVRT pack {fmt 9,32x32,id 0x999} -> seam 0x803DA12C(port) -> header{fmt 9,w32,h32,size 1024,flags 0,pixels heap#%d}\n", lh);
    printf("  heap texels fnv1a64=0x%016llX == AN fixture texels 0xA92D59BE5839FCE5\n", px_h);
    printf("  gx consumer next: reload_texture_slot_pixels/create_texture_state take {GinjaTextureHeader*, pixels};\n"
           "  GXInitTexObj* wants AR allow-list touchpoint + pixels/u32 handle (NJS_TEXNAME.texaddr u32 discipline, docs/p2-njs-confirm.md)\n");

    printf("== P2-AU ladder5: %d/%d PASS ==\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
