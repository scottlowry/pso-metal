/* OS: cache, interrupts, threads, time, heap, reset, debug (os and base headers) — pso-metal host re-host of OS: cache, interrupts, threads, time, heap, reset, debug
 * Tag format: original SDK symbol | call-site count (docs/host-surface.md) |
 * effort (TRIVIAL/MODERATE/HARD/BLOCKING-GPU) | native backing note.
 * Featured = real dolphin signature; tail = PSOSH_TAG placeholder prototype. */
#ifndef PSOSH_OS_H
#define PSOSH_OS_H
#include "psosh_types.h"


/* #4 OSRestoreInterrupts — os/OSInterrupt.h — 33 sites — MODERATE
 * Re-enable interrupts from a saved OSDisableInterrupts mask.
 * Native backing: release global gate iff saved mask was TRUE.
 * TODO(P2): same lock as OSDisableInterrupts; assert balanced pairs in debug. */
BOOL OSRestoreInterrupts(BOOL enable);

/* #10 OSDisableInterrupts — os/OSInterrupt.h — 28 sites — MODERATE
 * Mask machine interrupts, returning prior state (critical-section idiom).
 * Native backing: process-wide gate, NOT real interrupt masking: atomically
 * take a global spinlock, return prior bool in an int. Must nest per docs.
 * TODO(P2): verify no call site sleeps inside the disabled window. */
BOOL OSDisableInterrupts(void);

/* os/OSCache.h — 19 sites — TRIVIAL (with one trap)
 * Invalidate dcache before DMA-in reads. Native backing: no-op on Apple
 * unified cache EXCEPT buffers shared with Metal: replace with
 * [buffer didModifyRange:] / managed storage. TODO(P2): tag GX vertex/dlist
 * ranges and route those to didModifyRange; everything else no-ops. */
void DCInvalidateRange(void *addr, u32 nbytes);

/* os/OSCache.h — 18 sites — TRIVIAL (same trap as DCInvalidateRange)
 * Clean dcache before DMA-out writes. Native backing: didModifyRange on
 * GPU-visible buffers; no-op otherwise. */
void DCFlushRange(void *addr, u32 nbytes);

/* base/PPCArch.h — 17 sites — TRIVIAL
 * e500 sync (publish ordered writes). Native backing: `dmb ish` (or C11
 * atomic_thread_fence(seq_cst)). One inline, zero cost. */
void PPCSync(void);

/* os/OSTime.h — 13 sites — TRIVIAL
 * 32-bit timebase ticks. Native backing: mach_absolute_time scaled to keep
 * GC TB = 79.824327 MHz semantics (host-surface.md §2 clock row). */
OSTick OSGetTick(void);

/* os/OSTime.h — 8 sites — TRIVIAL
 * 64-bit timebase. Native backing: same mach_absolute_time scaler, u64.
 * TODO(P2): 27.963ms-per-tick convention; drift vs wall clock irrelevant. */
OSTime OSGetTime(void);

/* os.h — 8 sites — TRIVIAL
 * Debug printf to serial. Native backing: os_log / vfprintf(stderr). */
void OSReport(const char *fmt, ...);

/* os/OSAlloc.h — 7 sites — MODERATE — return block to a named OSHeap. */
void OSFreeToHeap(int heap, void *ptr);

/* os/OSAlloc.h — 6 sites — MODERATE
 * Allocate from a named OSHeap. Native backing: macOS malloc zone / bump
 * allocator preserving OSHeap semantics; handle == index into heap table.
 * `__OSCurrHeap` (extern, psosh_types.h) is the slot OSAlloc/OSFree alias to. */
void *OSAllocFromHeap(int heap, unsigned long size);

/* os/OSAlloc.h — 2 sites — MODERATE
 * Build a heap over [start,end). Native backing: bookkeeping header over the
 * caller range (game passes arena memory; arena itself = one big mmap). */
int OSCreateHeap(void *start, void *end);

/* os.h — 1 site (src/pso/ui/keyboard/SystemBootInit.cpp) — MODERATE
 * Boot the OS runtime (heaps, threads, allocators, VI). Native backing:
 * port bootstrap entry: create arena mmap, current heap, main OSThread,
 * start card/net workers. TODO(P2): this is the host layer's own main(). */
void OSInit(void);

/* ---- long tail: placeholder prototypes; exact signatures land in P2 ---- */
PSOSH_TAG(DCStoreRangeNoSync, 16, "TRIVIAL", "unified cache: no-op except GPU-shared ranges (didModifyRange); PPCSync -> dmb ish")
PSOSH_TAG(DCFlushRangeNoSync, 15, "TRIVIAL", "unified cache: no-op except GPU-shared ranges (didModifyRange); PPCSync -> dmb ish")
PSOSH_TAG(OSClearContext, 7, "HARD", "ucontext/pthread restructure of context save/restore")
PSOSH_TAG(OSResumeThread, 7, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSSendMessage, 7, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSSetCurrentContext, 7, "HARD", "ucontext/pthread restructure of context save/restore")
PSOSH_TAG(DCStoreRange, 6, "TRIVIAL", "unified cache: no-op except GPU-shared ranges (didModifyRange); PPCSync -> dmb ish")
PSOSH_TAG(OSGetCurrentThread, 6, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSInitMessageQueue, 6, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSReceiveMessage, 6, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSSleepThread, 6, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSSetThreadPriority, 5, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSCreateThread, 4, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSResetSystem, 4, "HARD", "process relaunch/teardown semantics")
PSOSH_TAG(OSInitThreadQueue, 3, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSPanic, 3, "TRIVIAL", "os_log/fprintf shim; OSPanic -> abort+dump; OSCachedToPhysical -> identity")
PSOSH_TAG(OSSuspendThread, 3, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSEnableInterrupts, 2, "MODERATE", "process-wide critical-section gate returning saved mask")
PSOSH_TAG(OSGetArenaLo, 2, "MODERATE", "macOS malloc-zone heap + large mmap arena reservation")
PSOSH_TAG(OSGetResetButtonState, 2, "HARD", "process relaunch/teardown semantics")
PSOSH_TAG(OSIsThreadSuspended, 2, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSRegisterResetFunction, 2, "HARD", "process relaunch/teardown semantics")
PSOSH_TAG(OSSetArenaLo, 2, "MODERATE", "macOS malloc-zone heap + large mmap arena reservation")
PSOSH_TAG(OSTicksToCalendarTime, 2, "TRIVIAL", "mach_absolute_time scaled to TB=79.824327 MHz")
PSOSH_TAG(OSWakeupThread, 2, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSYieldThread, 2, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(LCEnable, 1, "TRIVIAL", "unified cache: no-op except GPU-shared ranges (didModifyRange); PPCSync -> dmb ish")
PSOSH_TAG(OSCancelAlarm, 1, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSCancelThread, 1, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSCreateAlarm, 1, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSDisableScheduler, 1, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSEnableScheduler, 1, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSGetArenaHi, 1, "MODERATE", "macOS malloc-zone heap + large mmap arena reservation")
PSOSH_TAG(OSGetCurrentContext, 1, "HARD", "ucontext/pthread restructure of context save/restore")
PSOSH_TAG(OSGetSoundMode, 1, "TRIVIAL", "mach_absolute_time scaled to TB=79.824327 MHz")
PSOSH_TAG(OSGetThreadPriority, 1, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSInitAlloc, 1, "MODERATE", "macOS malloc-zone heap + large mmap arena reservation")
PSOSH_TAG(OSInitMutex, 1, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSIsThreadTerminated, 1, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSLink, 1, "TRIVIAL", "os_log/fprintf shim; OSPanic -> abort+dump; OSCachedToPhysical -> identity")
PSOSH_TAG(OSLockMutex, 1, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSSetAlarm, 1, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
PSOSH_TAG(OSUnlink, 1, "TRIVIAL", "os_log/fprintf shim; OSPanic -> abort+dump; OSCachedToPhysical -> identity")
PSOSH_TAG(OSUnlockMutex, 1, "TRIVIAL", "pthread + condvar 1:1; OSAlarm -> dispatch_after")
/* SDK macros the game calls (tagged here; host shims replace CodeWarrior defs) */
/* OSRoundUp32B — os.h — 3 sites — TRIVIAL — align-up 32B (cache lines). */
#define OSRoundUp32B(x)   (((u32)(x) + 31u) & ~31u)
/* OSRoundDown32B — os.h — 3 sites — TRIVIAL */
#define OSRoundDown32B(x) ((u32)(x) & ~31u)
/* OSTicksToMilliseconds — os.h — 4 sites — TRIVIAL — TB-rate divide. */
#define OSTicksToMilliseconds(t)   ((t) / (OS_TIMER_CLOCK / 1000u))
/* OSMicrosecondsToTicks — os.h — 1 site — TRIVIAL */
#define OSMicrosecondsToTicks(u)   ((u) * (OS_TIMER_CLOCK / 1000000u))
/* OSAlloc — os/OSAlloc.h — 1 site — MODERATE — current-heap alloc alias
 * (exact SDK name + semantics; heap slot lives in psosh_types.h). */
#define OSAlloc(size)       OSAllocFromHeap(__OSCurrHeap, (size))
#define OSFree(ptr)         OSFreeToHeap(__OSCurrHeap, (ptr))
/* OSCachedToPhysical — os.h — 3 sites — TRIVIAL — identity (no guest phys map). */
#define OSCachedToPhysical(a) ((u32)(uintptr_t)(void *)(a))

#endif /* PSOSH_OS_H */
