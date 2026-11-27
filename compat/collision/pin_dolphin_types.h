#ifndef _DOLPHIN_TYPES_H_
#define _DOLPHIN_TYPES_H_
/* compat/collision P2-AZ pin of the era SDK header
 * libs/dolphin/include/dolphin/types.h (decomp @6b38b5383).
 *
 * WHY a force-include pin instead of another shadow file: the era file is
 * reached by TWO different resolution edges, and only one of them obeys -I
 * order (proved by clang -H, receipts in receipts/):
 *   edge A (angle, 19 SDK files):  #include <dolphin/types.h>  -> -I order,
 *           hijackable by a shadow dir (this is the form AW's
 *           compat/repin/dolphin/types.h repairs, 27 TUs);
 *   edge B (quoted sibling, 28 SDK files): a header in
 *           libs/dolphin/include/dolphin/ doing  #include "types.h"  ->
 *           resolves relative to the INCLUDING FILE'S DIRECTORY first, so it
 *           binds the era file no matter which -I come first (the 1,109-TU
 *           class; -I ordering and any shadow OUTSIDE the read-only SDK dir
 *           are structurally immune-proofed against it).
 * Force-including THIS file occupies the era guard `_DOLPHIN_TYPES_H_` at TU
 * start, BEFORE either edge fires, so the era file is never processed under
 * the port rebind: the collision (5-6 "typedef redefinition with different
 * types" errors: s32/u32/uaddr/saddr/(vu32)/uintptr_t era `unsigned long` vs
 * the port `int32_t/uint32_t/uintptr_t` ) disappears on both edges at once,
 * while every consumer of the era surface keeps the exact names it expects.
 * This is the "SDK pre-include pin or guard macro" that AW 5.3 named as the
 * landing gate. No hijack-correct directory placement exists for edge B
 * inside a read-only decomp; a per-shape shadow matrix is unnecessary
 * because guard occupancy is resolution-order-independent by construction.
 *
 * Contract: requires the port rebind on the -I path (-Ihost/bringup/include
 * earliest -I, + -DPSO_PORT_LAYOUT_WAIVER) so the delegated
 * `#include <types.h>` resolves to host/bringup/include/types.h. Without the
 * rebind the delegate lands on era include/types.h and the typedefs stay
 * era-identical: a silent no-op (H0 behavior preserved), never a hazard.
 *
 * Surface carried below: the era tail EXCEPT (1) the scalar typedef block
 * (s8..u64/uaddr/saddr/vu*), which the rebind owns, (2) era
 * `typedef unsigned int uintptr_t`, which is itself a collision error and is
 * superseded by <stdint.h> via the rebind, and (3) the four + one libc legs
 * (`#include "libc/stdio.h" ... "libc/math.h"`), which are DELIBERATELY NOT
 * force-loaded: battery-3 measurement shows zero corpus consumers need them
 * through this edge (bare `-D_DOLPHIN_TYPES_H_` = 0 collateral), and
 * force-loading the era math.h poisons TUs that get math from host/clib
 * (guard-collateral on `ceil`, 1 TU: TObjEneMe1CanadinLeader.cpp). TUs that
 * genuinely need era SDK libc declarations include them by their own edges,
 * exactly as they do today.
 *
 * TRUE/FALSE/BOOL/NULL/ATTRIBUTE_ALIGN are redundantly carried here (the
 * rebind mirror of era include/types.h already defines them); carried so
 * suppression stays lossless for any TU that DID rely on the dolphin copy.
 */
#include <types.h>

typedef char *Ptr;
typedef unsigned int uint;
#if !defined(__cplusplus) && !defined(nullptr)
#define nullptr 0
#endif

typedef int BOOL;

#define FALSE 0
#define TRUE 1

#define AT_ADDRESS(addr) : (addr)

#define ATTRIBUTE_ALIGN(num) __attribute__((aligned(num)))

#define __REGISTER register

#define INT_MIN -2147483648
#define INT_MAX 2147483647

#ifndef NULL
#ifndef __cplusplus
#define NULL ((void*)0)
#else
#define NULL 0
#endif
#endif

/* va_list supply (P2-TL docs/p2-tail-2.md §2 ask, apply-8, integrator-
 * adjudicated): era os.h:40 OSVReport needs va_list but the suppressed libc
 * legs above removed its supplier. VERBATIM struct carry was tried and
 * CONVICTED (this battery): two occurrences of `typedef struct {..}
 * __va_list[1]` are DISTINCT types in C++ (anonymous-struct uniqueness) and
 * collide with the era stdarg re-include at libc/stdarg.h:13-14 (18 TUs
 * regressed). Supplied instead as the HOST-ABI type: clang's stdarg defines
 * va_list identically (chained typedef, same underlying type - legal), and
 * NO corpus TU can survive the era macros anyway (__builtin_va_info is not
 * a clang builtin: any user was already a fail; zero corpus refs to
 * __va_list/__va_arg/_var_arg_typeof - python-grepped). _STDARG_H_ is
 * OCCUPIED so the era file self-skips; va_* map to clang builtins (parse
 * passes; runtime executes host ABI, per port doctrine). */
#ifndef _STDARG_H_
#define _STDARG_H_
typedef __builtin_va_list va_list;
#define va_start(ap, fmt) __builtin_va_start(ap, fmt)
#define va_arg(ap, t) __builtin_va_arg(ap, t)
#define va_end(ap) __builtin_va_end(ap)
#define va_copy(d, s) __builtin_va_copy(d, s)
#endif

#endif
