/* P2-Y boot ladder — era PRS wrappers (TScreenFadeController.cpp) + era
 * heap machinery (memory.cpp over the OSAlloc.c port). Every expected value
 * below is hand-derived from the era PPC sources of truth; NONE is produced
 * by the C under test.
 *
 * ORACLE PROVENANCE
 *  [fade]  build/GPOE8P/asm/pso/system/TScreenFadeController.s @ 6b38b5383:
 *    prs_decompress_alloc__FPCvPvl  0x800F7D30 (size 0x7C)
 *    prs_decompress_buffer__FPCvPv  0x800F7DAC (size 0x7C)
 *  Both wrappers, instruction-for-instruction (traced from the .s dump):
 *    li r3,0x30; bl __nw__FUl; (bl __ct__10PrsDecoderFv if ctor returns
 *    non-self); cmplwi r31,0/beq skip; then bl prs_decompress__10PrsDecoder
 *    FPUcPUcl with r3=decoder, r4=dst(orig r4), r5=src(orig r3), r6=0
 *    (buffer 0x800F7DF4 `38 C0 00 00`, alloc 0x800F7D74 `38 C0 00 00`);
 *    result kept in r30, dtor if r31!=0, `mr r3,r30` return (0x800F7E10).
 *  FACT F-A: alloc's 3rd arg size_limit is saved by `stmw r28,0x10(r1)` and
 *    never reloaded before the epilogue lmw — the era wrapper DISCARDS the
 *    limit. The decode is identical to buffer's. (TItemReverseTech.cpp:172
 *    callers pass 0x200000; era-wise that number does nothing.)
 *  FACT F-B: src/dst are SWAPPED into prs_decompress (r4=dst r2=…, r5=src):
 *    prs_decompress(u8* out, u8* in, s32) — wrapper arg order is (src,dst).
 *
 *  [prs]  Hand-trace of decode CONTENT reuses the rules proven per-address
 *    in prs_golden_test.cpp (prs.s 0x8036F4DC..0x8036F7A8): literal bits
 *    LSB-first 8036F5FC; near token size=(a<<1)+b+2 @8036F6A4-6AC,
 *    off=byte-0x100, off+=n @8036F6C0; far END = 0x00 0x00 `or.` 8036F654;
 *    error -> -1 8036F710.
 *
 *  [heap] build/GPOE8P/asm/pso/system/memory.s (.text 0x803DE7E0..0x803DE9C4):
 *    fill_memory 0x803DE7E0, init_system_heap_and_global_allocator 0x803DE804,
 *    xmalloc 0x803DE838, xcalloc 0x803DE864, xfree 0x803DE890,
 *    sys_alloc 0x803DE8BC, free_to_sys_heap 0x803DE8E4, sys_calloc 0x803DE90C,
 *    sys_heap_realloc 0x803DE960. .sbss 0x805D77A8 sys_heap_handle +
 *    0x805D77AC global_allocator (two 4-byte words, memory.s lines 186-195)
 *    — exactly the two globals init_system_heap_and_global_allocator writes
 *    (memory.cpp:26-29). Allocator API semantics + block-header layout:
 *    libs/dolphin/src/os/OSAlloc.c + asm/os/OSAlloc.s: HEADERSIZE 32
 *    (0x80370BEC `addi r3,r6,0x20`), 32B ALIGNMENT (`clrrwi rX,rY,5` at
 *    0x80370CCC/CE4/0x80370CFC/D08), MINOBJSIZE 64 split rule (OSAlloc.c
 *    OSAllocFromHeap 429..448), DLInsert address-sorted merge-on-free
 *    (0x80370A4C), cell fields prev/next/size/hd/requested.
 *  All host-side expected sizes are computed HERE by hand, not asked of the
 *  allocator: (0x400+0x20+0x1F)&~0x1F = 0x420 etc.
 */
#include "pso_bringup.h"
#include "pso/system/prs.h"                 /* prs_decompress_buffer/alloc */
#include "pso/system/AllocatorCallbacks.h"  /* global_allocator table */
#include "pso/system/memory.h"              /* xmalloc/xcalloc/xfree */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static int failures, total;
#define CHECK(cond, ...) do { total++; if (!(cond)) { failures++; \
    printf("FAIL "); printf(__VA_ARGS__); printf("\n"); } \
    else { printf("PASS "); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* ---- symbols living in the decomp units being executed ---- */
extern "C" {
extern OSHeapHandle sys_heap_handle;        /* memory.cpp:9  (@sbss 0x805D77A8) */
}
extern const AllocatorCallbacks sys_allocator_func_table;  /* memory.cpp:12 */

/* Era Cell view at payload-32 (host layout per V1 in osalloc_era.c: era PPC
 * offsets prev@0 next@4 size@8 hd@12 req@16, host prev@0 next@8 size@16
 * hd_idx@20 req@24, total EXACTLY HEADERSIZE=32). hd_idx stores heap+1;
 * 0 reproduces the era's cell->hd==0 free sentinel. */
typedef struct EraCellView {
    void *prev;        /* +0  */
    void *next;        /* +8  */
    s32   size;        /* +16 block total incl. 32B header */
    u32   hd_idx;      /* +20 heap handle + 1 while allocated, 0 free */
    s32   requested;   /* +24 */
    s32   _pad;        /* +28 */
} EraCellView;
#define CELL(p) ((EraCellView *)((u8 *)(p) - 32))

/* HeapDesc host mirror (same padding rules): era PPC 24B -> host 40B. */
typedef struct HeapDescView {
    s32   size;
    s32   _pad;
    void *free_list;
    void *alloc_list;
    u32   paddingBytes, headerBytes, payloadBytes;
} HeapDescView;

/* PRS stream bytes, hand-encodings traced exactly like Q's G1-G6 vectors: */
/* Q-G3 vector (trace reused verbatim from prs_golden_test.cpp header):
 * ctrl 0xAF = lit A B C D; b4:0 end-run; b5:1 far: 0xE3,0xFF ->
 * off=(255<<5)+(0xE3>>3)-0x2000=-4, +n(4)=0, size&7=3 -> 5 copies abs0..4
 * -> "ABCDABCDA" n=9; b7:1 far 0x00 0x00 END. */
static const u8 s_g3[]  = {0xAF, 'A','B','C','D', 0xE3, 0xFF, 0x00, 0x00};
/* L2 new hand-trace: ctrl 0x47 = 0b01000111: b0..b2=1,1,1 literals A,B,C;
 * b3=0 end-run; b4=0 near token; a=b5=0, b=b6=1 -> size=(0<<1)+1+2=3;
 * offset byte 0xFD -> off = 0xFD-0x100 = -3, += n(3) -> abs 0; 3 copies
 * window[0..2]='A','B','C' (read_byte_at: flushed=0 not > offset, offset <
 * window_len) -> out "ABCABC" n=6; b7=0 end-run (no-op). ctrl 0x01: b0=1
 * far selector, src 0x00 0x00 -> or.==0 -> END (8036F654). ret 6. */
static const u8 s_l2[]  = {0x47, 'A','B','C', 0xFD, 0x01, 0x00, 0x00};
/* Q-G5 vector: {0x01,'A',0x00} -> corrupt negative offset, error=1 ->
 * epilogue li r3,-1 (8036F710). */
static const u8 s_g5[]  = {0x01, 'A', 0x00};

/* sys heap carved per era SystemBootInit.cpp:273-275 geometry
 * (init_system_heap_and_global_allocator(cfg.heap + 0x518000, hi-start)):
 * here an arena region that cannot alias the step-3 game heap 0. */
#define SYSHEAP_START_OFF 0x1000000u   /* arena+16 MB, 6 MB long */
#define SYSHEAP_SIZE      0x00600000

int main() {
    if (pso_bringup_install() != 0) { fprintf(stderr, "arena install failed\n"); return 2; }
    OSInit();
    CHECK(sys_heap_handle == 0, "L0 memory.cpp: sys_heap_handle global is bss-zero (%d) before init_system_heap_and_global_allocator", (int)sys_heap_handle);

    /* ============ H1: era system-heap install (memory.cpp:26-29) ============ */
    u8 *sysstart = pso_arena_base + SYSHEAP_START_OFF;
    init_system_heap_and_global_allocator(sysstart, (s32)SYSHEAP_SIZE);
    int hh = sys_heap_handle;
    CHECK(hh == 1 && global_allocator == &sys_allocator_func_table &&
          global_allocator->allocate == sys_alloc &&
          global_allocator->release == free_to_sys_heap &&
          global_allocator->allocate_zeroed == sys_calloc &&
          global_allocator->reallocate == sys_heap_realloc,
          "H1 init_system_heap_and_global_allocator: heap=%d, global_allocator == &sys_allocator_func_table, 4/4 slots (memory.s 0x803DE804)", hh);

    /* ============ H2: xmalloc era block header (OSAllocFromHeap) ============ */
    void *p = xmalloc(0x400);
    EraCellView *c = CELL(p);
    CHECK(p && (uintptr_t)p % 32 == 0 && (u8*)p - 32 == sysstart &&
          c->size == 0x420 && c->requested == 0x400 && c->hd_idx == (u32)hh + 1,
          "H2 xmalloc(0x400): cell@p-32 size=0x%x want 0x420, requested=0x%x, hd_idx=%u==heap+1 (&HeapArray[%d] era mapping), payload 32-aligned at OSCreateHeap start (OSAlloc.s 0x80370BEC addi r3,r6,0x20)",
          (unsigned)c->size, (unsigned)c->requested, (unsigned)c->hd_idx, hh);

    /* ============ H3: xcalloc zeroes via fill_memory ============ */
    void *q = xcalloc(4, 0x100);
    int zeroed = q != 0;
    for (int i = 0; zeroed && i < 0x400; i++) zeroed = ((u8 *)q)[i] == 0;
    CHECK(zeroed && CELL(q)->requested == 0x400 && CELL(q)->size == 0x420,
          "H3 xcalloc(4,0x100): 0x400 bytes zero-filled (fill_memory 0x803DE7E0), header 0x420/0x400");

    /* ============ H4: sys_heap_realloc copies, old cell released ============ */
    memset(p, 0xA5, 0x400);
    void *r = sys_heap_realloc(p, 0x80);
    int copied = r != 0;
    for (int i = 0; copied && i < 0x80; i++) copied = ((u8 *)r)[i] == 0xA5;
    CHECK(copied && CELL(r)->requested == 0x80 && CELL(p)->hd_idx == 0,
          "H4 sys_heap_realloc(p,0x80): 0x80 pattern bytes moved (move_data=memmove, TLobby_803DE778.cpp:8), new requested=0x%x, old cell->hd==0 (OSFreeToHeap 0x80370BF4 clears hd)",
          r ? (unsigned)CELL(r)->requested : 0u);

    /* ============ H5: free-list coalesce + first-fit reuse ============ */
    /* Use game heap 0 (pristine in this process) so the three blocks are
     * provably adjacent: each 0x200 request rounds to a 0x220 cell. */
    void *a = OSAllocFromHeap(0, 0x200), *b = OSAllocFromHeap(0, 0x200),
         *cc = OSAllocFromHeap(0, 0x200);
    int adj = (u8*)b == (u8*)a + 0x220 && (u8*)cc == (u8*)b + 0x220; /* 0x200+32 */
    OSFreeToHeap(0, b); OSFreeToHeap(0, a); OSFreeToHeap(0, cc); /* DLInsert merges all three */
    void *d = OSAllocFromHeap(0, 0x600);    /* (0x600+32)=0x620, leftover
                                             * huge -> split at first cell */
    CHECK(adj && d == a && CELL(d)->size == 0x620 && CELL(d)->requested == 0x600,
          "H5 coalesce+first-fit: allocs adjacent (stride 0x220), free b,a,c merges, xmalloc(0x600) reuses block a (d==a: %d), cell 0x%x/0x%x (DLInsert 0x80370A4C address-sorted merge)",
          d == a, d ? (unsigned)CELL(d)->size : 0u, d ? (unsigned)CELL(d)->requested : 0u);

    /* ============ H6: OSCheckHeap live-count contract (P2-Q) ============ */
    long n0 = OSCheckHeap(hh);
    void *e1 = xmalloc(0x40), *e2 = xmalloc(0x40);
    long n1 = OSCheckHeap(hh);
    xfree(e1); xfree(e2);
    CHECK(n1 == n0 + 2 && OSCheckHeap(hh) == n0,
          "H6 OSCheckHeap: %ld -> %ld -> %ld (count +2 then balanced free)", n0, n1, OSCheckHeap(hh));
    OSFreeToHeap(0, d); xfree(q);

    /* ============ H7: oversize alloc fails soft, heap intact ============ */
    void *big = xmalloc((u32)SYSHEAP_SIZE + 0x1000);
    void *ok = xmalloc(0x100);
    CHECK(big == 0 && ok != 0, "H7 xmalloc(0x%x) > heap -> NULL, next xmalloc(0x100) still succeeds",
          (unsigned)SYSHEAP_SIZE + 0x1000);
    xfree(ok);

    /* ============ W1: era wrapper -> caller-provided heap dst ============ */
    u8 *wdst = (u8 *)xmalloc(0x2000);
    memset(wdst, 0xCC, 0x2000);
    u32 n = prs_decompress_buffer(s_g3, wdst);
    CHECK(n == 9 && memcmp(wdst, "ABCDABCDA", 9) == 0,
          "W1 prs_decompress_buffer(G3): ret=%u out=%.9s into xmalloc'd dst (NOT the internal window) — .s 0x800F7DAC r4=dst r5=src r6=0",
          (unsigned)n, (char *)wdst);

    /* ============ W2: NEW hand-traced vector through the wrapper =========== */
    memset(wdst, 0xCC, 64);
    n = prs_decompress_buffer(s_l2, wdst);
    CHECK(n == 6 && memcmp(wdst, "ABCABC", 6) == 0,
          "W2 prs_decompress_buffer(L2 trace 0x47+0xFD): ret=%u out=%.6s expect ABCABC",
          (unsigned)n, (char *)wdst);

    /* ============ W3: alloc discards size_limit (FACT F-A) ============ */
    memset(wdst, 0xCC, 64);
    s32 m = prs_decompress_alloc(s_l2, wdst, 1);
    CHECK(m == 6 && memcmp(wdst, "ABCABC", 6) == 0,
          "W3 prs_decompress_alloc(limit=1): ret=%d wrote ALL 6 bytes — limit arg is dead in era .s (0x800F7D74 li r6,0; r5 never reloaded)", (int)m);

    /* ============ W4: fake disc -> arena -> era wrapper (step 19 chain) ==== */
    DVDInit();
    DVDFileInfo fi; memset(&fi, 0, sizeof fi);
    bool okd = DVDOpen((char *)"logo_e.prs", &fi);
    if (okd) {
        u8 *src = pso_arena_base + 0x20000;
        s32 got = DVDReadPrio(&fi, src, (s32)fi.length, 0, 2);
        DVDClose(&fi);
        memset(wdst, 0xCC, 64);
        u32 w = got == (s32)sizeof(s_g3) ? prs_decompress_buffer(src, wdst) : 0;
        CHECK(w == 9 && memcmp(wdst, "ABCDABCDA", 9) == 0,
              "W4 DVD->arena->prs_decompress_buffer: ret=%u out=%.9s (logo_e.prs bytes == G3 trace)",
              (unsigned)w, (char *)wdst);
    } else CHECK(0, "W4 DVDOpen(logo_e.prs) failed");

    /* ============ W5: error path through both wrappers ============ */
    memset(wdst, 0xCC, 64);
    s32 g = prs_decompress_alloc(s_g5, wdst, 0x100);
    u32 h = prs_decompress_buffer(s_g5, wdst);
    CHECK(g == -1 && h == 0xFFFFFFFFu,
          "W5 corrupt stream: alloc ret=%d, buffer u32 wrap=0x%08x (s32 -1 through `mr r3,r30` at 0x800F7E10)",
          (int)g, (unsigned)h);
    xfree(wdst);

    printf("== P2-Y ladder: %d/%d PASS ==\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
