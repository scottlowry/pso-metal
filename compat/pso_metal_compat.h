/* pso-metal Phase-0 compat shim, force-included (-include) so era SDK headers
 * resolve PPC intrinsics before they poison the translation unit. */
#ifndef PSO_METAL_COMPAT_H
#define PSO_METAL_COMPAT_H

#include <string.h> /* for memcpy used by the __memcpy shim below */

/* MWCC/PPC floating intrinsics used by libs/dolphin/include/libc/math.h and
 * include/clib/math.h (real implementations land in Phase 1). */
#ifndef __PSO_METAL_NO_MATH_SHIMS
static inline float __fabsf(float x) { return __builtin_fabsf(x); }
/* WHY: decomp code calls __fabs directly; SDK math.h also *defines* inline fabs,
 * so this must stay a distinct intrinsic symbol, not a fabs macro. */
static inline double __fabs(double x) { return __builtin_fabs(x); }
static inline double __frsqrte(double x) { return 1.0 / __builtin_sqrt(x); }
static inline float __frsqrtes(float x) { return 1.0f / __builtin_sqrtf(x); }
/* Note: deliberately NO '#define fabs/fabsf __builtin_*' — several decomp TUs
 * (include/clib/math.h) define inline fabs/fabsf themselves; the macro turned
 * those into "definition of builtin function" errors (14 TUs). */
#endif

/* PPC time base stub (real host implementation lands in Phase 1) */
static inline unsigned long long __mftb(void) { return 0ULL; }

/* MWCC integer/cache intrinsics referenced directly by decomp source (class E) */
/* WHY: PPC count-leading-zeros; MWCC yields 32 for input 0 while __builtin_clz(0) is UB. */
static inline unsigned int __cntlzw(unsigned int x) { return x ? __builtin_clz(x) : 32u; }
/* WHY: MWCC absolute-value intrinsic called directly in decomp source. */
static inline int __abs(int x) { return x < 0 ? -x : x; }
/* WHY: MWCC register-argument memcpy intrinsic; host memcpy is a drop-in. */
#define __memcpy(dst, src, n) memcpy((dst), (src), (n))
/* WHY: PPC "data cache touch" hint; semantically a no-op on Apple Silicon. */
static inline void __dcbt(const void *addr) { (void)addr; }

#endif
