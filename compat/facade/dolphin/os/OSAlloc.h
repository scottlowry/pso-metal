/* pso-metal P2-D4 facade DECLARATION header (card t_075ab4e1). INERT until
 * `-I compat/facade` lands (docs/p2-d4facade-1.md; -I-path-only, never
 * -include — collision law).
 *
 * WHY: `host/bringup/include/dolphin/os/OSAlloc.h` shadows the era header but
 * ships ONLY pso_bringup.h — the era OSAlloc.h macro tail
 * (libs/dolphin/include/dolphin/os/OSAlloc.h:25-26, decomp @6b38b5383) is the
 * sole collateral `GcrnaStream.cpp:415 undeclared OSAlloc` in the rebind
 * ledger (docs/p2-collision-debt.md §3/§4.8), plus six era declarations with
 * no substrate twin. The bringup shadow is host-lane seat; this file is the
 * parse-side supply. Declarations only — implementations stay osalloc_era.c
 * (host/bringup). NO re-declarations of substrate-owned signatures
 * (pso_bringup.h:65-72 owns OSHeapHandle, __OSCurrHeap, OSInitAlloc,
 * OSCreateHeap, OSSetCurrentHeap, OSAllocFromHeap(int,u32), OSFreeToHeap,
 * OSCheckHeap — the substrate OSAllocFromHeap size arg is u32, deliberately
 * narrower than era `unsigned long` (:12); era callers pass literals). */
#ifndef PSO_FACADE_DOLPHIN_OS_OSALLOC_H
#define PSO_FACADE_DOLPHIN_OS_OSALLOC_H
#include "../../../../host/bringup/pso_bringup.h" /* substrate contract,
                                                   * bringup-shadow pattern */
#ifndef OS_ALLOC_H
#define OS_ALLOC_H /* occupy era guard (OSAlloc.h:1) so a facade-vs-bringup
                    * splice order can never double-process the original */
#endif
#ifdef __cplusplus
extern "C" {
#endif

/* Era decls with NO pso_bringup.h twin — verbatim signatures, line-cited: */
void* OSAllocFixed(void* rstart, void* rend);                 /* OSAlloc.h:13 */
void  OSDestroyHeap(int heap);                                /* OSAlloc.h:18 */
void  OSAddToHeap(int heap, void* start, void* end);          /* OSAlloc.h:19 */
unsigned long OSReferentSize(void* ptr);                      /* OSAlloc.h:21 */
void  OSDumpHeap(int heap);                                   /* OSAlloc.h:22 */
void  OSVisitAllocated(void (*visitor)(void*, unsigned long));/* OSAlloc.h:23 */

/* The collateral-closing tail, verbatim era OSAlloc.h:25-26: */
#define OSAlloc(size) OSAllocFromHeap(__OSCurrHeap, (size))
#define OSFree(ptr)   OSFreeToHeap(__OSCurrHeap, (ptr))

#ifdef __cplusplus
}
#endif
#endif /* PSO_FACADE_DOLPHIN_OS_OSALLOC_H */
