/* pso-metal P2-Y port-side facade (deviation D4 follow-through).
 * Executing TUs must NOT see libs/dolphin/include (era PPC headers shadow
 * host libc — see docs/p2-bringup-1.md D4). memory.cpp's `#include
 * <dolphin/os/OSAlloc.h>` therefore lands here: the declarations are the
 * ones in pso_bringup.h, field-for-field the era OSAlloc.h contract
 * (libs/dolphin/include/dolphin/os/OSAlloc.h: OSHeapHandle, __OSCurrHeap,
 * OSAllocFromHeap/OSFreeToHeap/OSCreateHeap/OSInitAlloc/OSSetCurrentHeap/
 * OSCheckHeap). Implementations: osalloc_era.c (era OSAlloc.c port). */
#ifndef PSO_PORT_DOLPHIN_OS_OSALLOC_H
#define PSO_PORT_DOLPHIN_OS_OSALLOC_H
#include "../../../pso_bringup.h"  /* host/bringup/pso_bringup.h (quoted-include
                                 * resolves relative to this file's dir) */
#endif
