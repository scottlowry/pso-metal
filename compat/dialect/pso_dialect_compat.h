/* P2-T dialect shim — force-included (-include) AFTER compat/pso_metal_compat.h.
 * Port-side fixes for MWCC/PPC C-dialect constructs clang rejects. Precedent for
 * the guard-preemption trick: build/waivers/pso_assert_waivers.h ("Force the real
 * types.h to be processed now, under its TYPES_H guard, so the later #include
 * in decomp sources is a no-op and cannot clobber the macros installed below"). */
#ifndef PSO_DIALECT_COMPAT_H
#define PSO_DIALECT_COMPAT_H

/* Ordering-hardening: pull the existing compat shims first, so the types.h
 * pre-include below (whose <dolphin/types.h> chain reaches libc/math.h, which
 * *calls* __frsqrte/__fabsf) always sees them declared, regardless of the
 * -include order on the command line. */
#include "../pso_metal_compat.h"

/* 1) AT_ADDRESS neutralization. MWCC/CodeWarrior pins a variable to a fixed
 *    address with a bitfield-like declarator: `OSExecParams* __OSExecParams
 *    AT_ADDRESS(0x800030F0);` where types.h defines AT_ADDRESS(addr) : (addr).
 *    clang: "expected ';' after top level declarator" (OSExec.h:25, OSLink.h:27,
 *    GXVert.h:25, dvd.h:196, QuestDownloadServer.h:134). A port defines the pin
 *    empty — placement is host/linker territory (pso_metal_compat.h precedent:
 *    ATTRIBUTE_ALIGN_PARAM "a port defines it empty"; host lane Q owns stubs).
 *    Pre-include both types.h variants so their include guards suppress the
 *    later redefinition; then neutralize. */
#include "types.h"          /* -I include  -> include/types.h (guard TYPES_H) */
#include <dolphin/types.h>  /* -I libs/dolphin/include (guard _DOLPHIN_TYPES_H_) */
#undef AT_ADDRESS
#define AT_ADDRESS(addr) /* P2-T: address pin dropped host-side */

/* 2) musyx vendor-header struct-tag collision. musyx_internal.h:12-13 does
 *    `struct SoundReverb;` then uses `SoundReverb *rv` without the tag keyword
 *    (valid C++ / MWCC; clang C: "must use 'struct' tag to refer to type").
 *    A typedef before the header resolves both spellings (26 TUs). */
typedef struct SoundReverb SoundReverb;

/* (3) musyx/musyx.h bare `extern "C"` and (4) hw_regs.h raw `: 0xADDR` pins are
 *    pure syntax, unreachable by macro shims: shadow headers in
 *    compat/dialect/pshim/ (activated by -I compat/dialect/pshim first). */

#endif
