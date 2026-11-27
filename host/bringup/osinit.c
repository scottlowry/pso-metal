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

    /* Heap arena: era SystemBootInit.cpp:271-275 shape — cfg.heap =
     * OSInitAlloc(fifo_end, hi, 2); the game heap region is cfg.heap +
     * 0x518000. HeapArray (2 era HeapDescs) lands at arena+0x400000.
     * P2-Y: the allocator entry points are now the era OSAlloc port
     * (osalloc_era.c), replacing this file's P2-Q bump/free-list shim
     * internals; names, handles and Q's OSCheckHeap live-count contract
     * are unchanged (docs/p2-bringup-2.md). */
    void *heapbase = OSInitAlloc(pso_arena_base + 0x400000, arena_hi, 2);
    int h = OSCreateHeap(heapbase, (u8 *)heapbase + 0x518000);
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

/* ---- heap ----
 * OSCreateHeap-era entry points (OSInitAlloc/OSCreateHeap/OSAllocFromHeap/
 * OSFreeToHeap/OSSetCurrentHeap/__OSCurrHeap/OSCheckHeap) now live in
 * osalloc_era.c — the era Dolphin OSAlloc port (P2-Y, ledger step 7). The
 * P2-Q bump/free-list shim that used to sit here was explicitly temporary
 * ("the real decomp heap machinery ... lands at ledger step 7"). */

