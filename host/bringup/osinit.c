/* pso-metal P2-Q bringup mini-OSInit (p1-init-ledger §4 step 3).
 * Arena lo/hi globals, GC-timebase clock shim, OSCreateHeap-shaped heap shim,
 * OSReport->printf. NO threads, NO interrupts, NO MPU/cache ops.
 *
 * Tick rate: PPC timebase = OSBusClock/4 = 8,104,896 Hz. All 9 era consumers
 * that divide by "*(u32*)0x800000F8 >> 2" (p1-datamodel §1a) get a consistent
 * seconds/ms conversion from this shim.
 */
#include "pso_bringup.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define PSO_TB_HZ 8104896ull /* = pso_sysret->busFreq / 4 */

static void *arena_lo;
static void *arena_hi;

/* ---- arena ---- */
void *OSGetArenaLo(void) { return arena_lo; }
void *OSGetArenaHi(void) { return arena_hi; }
void  OSSetArenaLo(void *lo) { arena_lo = lo; }
void  OSSetArenaHi(void *hi) { arena_hi = hi; }

void OSInit(void) {
    /* Era boot flow: OSInit sets the default arena to the full 24 MB minus
     * the static data region. We mirror the shape: lo = arena+0x3100 (era
     * first usable offset after exceptions/BootInfo/ipl work), hi = arena end.
     * init_game_systems then carves fb/fifo/heap below hi. */
    arena_lo = pso_arena_base + 0x3100;
    arena_hi = pso_arena_base + PSO_MAIN_RAM_SIZE;

    /* Heap arena: OSInitAlloc between the (not yet carved) fifo region and
     * arena_hi. We place the game-heap control block + free lists at
     * arena+0x400000 and let the 0x518000 game heap start at 0x518000-relative
     * offset per init_game_systems: cfg.heap = OSInitAlloc(...) return,
     * sys heap starts at cfg.heap + 0x518000. 0x80518000-ish. */
    void *heapbase = OSInitAlloc(pso_arena_base + 0x400000, arena_hi, 2);
    (void)heapbase;
    int h = OSCreateHeap(pso_arena_base + 0x400000, pso_arena_base + 0x400000 + 0x518000);
    OSSetCurrentHeap(h);
}

/* ---- clock ---- */
static u64 now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)ts.tv_sec * 1000000000ull + (u64)ts.tv_nsec;
}
u64 OSGetTime(void) { return now_ns() / 1000000000ull * PSO_TB_HZ +
                            (now_ns() % 1000000000ull) * PSO_TB_HZ / 1000000000ull; }
u32 OSGetTick(void) { return (u32)(OSGetTime() & 0xFFFFFFFFull); }
s64 OSTicksToMilliseconds(u64 ticks) { return (s64)(ticks * 1000ull / PSO_TB_HZ); }

/* ---- report ---- */
void OSReport(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

/* ---- OSCreateHeap-shaped heap shim (OSAlloc.h contract) ----
 * Block header: magic + size + heap tag; free list per heap, sorted singly
 * list with coalescing. Deliberately simple — the real decomp heap machinery
 * (init_system_heap_and_global_allocator, memory.cpp) lands at ledger step 7
 * and re-grows on top of these entry points. */
#define HEAP_MAGIC 0x4B534F50u /* "POSK" */
#define MAXHEAPS 2
typedef struct HeapCtl {
    void *start, *end;
    long allocs;
} HeapCtl;
typedef struct BlkHdr {
    u32 magic;
    u32 freed;
    u32 size;      /* payload bytes */
    u32 heap;
    struct BlkHdr *next_free;
} BlkHdr;

static HeapCtl heaps[MAXHEAPS];
static int nheaps;
static BlkHdr *free_lists[MAXHEAPS];
volatile OSHeapHandle __OSCurrHeap = -1;

void *OSInitAlloc(void *arenaStart, void *arenaEnd, int maxHeaps) {
    (void)maxHeaps; /* fixed MAXHEAPS=2 matches init_game_systems's request */
    /* control block lives at arenaStart; usable payload above it —
     * era OSCInitHeap returned the same arena start (heap handles follow). */
    volatile u8 *p = (volatile u8 *)arenaStart;
    for (size_t i = 0; i < sizeof(HeapCtl) * MAXHEAPS + 64; i += 8) p[i] = 0;
    return arenaStart;
}

int OSCreateHeap(void *start, void *end) {
    if (nheaps >= MAXHEAPS) return -1;
    /* skip the control-block zone (first 0x1000) */
    u8 *s = (u8 *)start + 0x1000;
    if (s >= (u8 *)end) return -1;
    heaps[nheaps].start = s;
    heaps[nheaps].end = end;
    heaps[nheaps].allocs = 0;
    free_lists[nheaps] = 0;
    return nheaps++;
}

int OSSetCurrentHeap(int heap) {
    int prev = __OSCurrHeap;
    __OSCurrHeap = heap;
    return prev;
}

void *OSAllocFromHeap(int heap, u32 size) {
    if (heap < 0 || heap >= nheaps || size == 0) return 0;
    size = (size + 7u) & ~7u; /* era OSAlloc aligned 8 */
    /* first-fit free list */
    BlkHdr **pp = &free_lists[heap];
    while (*pp) {
        BlkHdr *b = *pp;
        if (b->size >= size) {
            *pp = b->next_free;
            b->size = b->size; /* keep total; alloc-size tracked separately */
            b->freed = 0;
            heaps[heap].allocs++;
            if (b->size - size >= sizeof(BlkHdr) + 8) { /* split */
                BlkHdr *tail = (BlkHdr *)((u8 *)(b + 1) + size);
                tail->magic = HEAP_MAGIC; tail->freed = 1;
                tail->size = b->size - size - sizeof(BlkHdr);
                tail->heap = (u32)heap; tail->next_free = free_lists[heap];
                free_lists[heap] = tail;
                b->size = size;
            }
            return (u8 *)(b + 1);
        }
        pp = &b->next_free;
    }
    /* bump from untouched region */
    static u8 *bump[MAXHEAPS];
    if (!bump[heap]) bump[heap] = (u8 *)heaps[heap].start;
    u8 *p = bump[heap];
    if (p + sizeof(BlkHdr) + size > (u8 *)heaps[heap].end) return 0;
    BlkHdr *b = (BlkHdr *)p;
    b->magic = HEAP_MAGIC; b->freed = 0; b->size = size; b->heap = (u32)heap;
    b->next_free = 0;
    bump[heap] = p + sizeof(BlkHdr) + size;
    heaps[heap].allocs++;
    return p + sizeof(BlkHdr);
}

void OSFreeToHeap(int heap, void *ptr) {
    if (!ptr || heap < 0 || heap >= nheaps) return;
    BlkHdr *b = (BlkHdr *)((u8 *)ptr - sizeof(BlkHdr));
    if (b->magic != HEAP_MAGIC) return; /* era: OSReport + panic; keep soft */
    b->freed = 1;
    b->next_free = free_lists[heap];
    free_lists[heap] = b;
    heaps[heap].allocs--;
}

long OSCheckHeap(int heap) {
    return (heap >= 0 && heap < nheaps) ? heaps[heap].allocs : -1;
}
