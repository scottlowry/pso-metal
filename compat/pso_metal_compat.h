/* pso-metal Phase-0 compat shim, force-included (-include) so era SDK headers
 * resolve PPC intrinsics before they poison the translation unit. */
#ifndef PSO_METAL_COMPAT_H
#define PSO_METAL_COMPAT_H

/* MWCC/PPC floating intrinsics used by libs/dolphin/include/libc/math.h */
#ifndef __PSO_METAL_NO_MATH_SHIMS
#define fabsf(x) __builtin_fabsf(x)
#define fabs  __builtin_fabs
static inline float __fabsf(float x) { return __builtin_fabsf(x); }
static inline double __frsqrte(double x) { return 1.0 / __builtin_sqrt(x); }
static inline float __frsqrtes(float x) { return 1.0f / __builtin_sqrtf(x); }
#endif

/* PPC time base stub (real host implementation lands in Phase 1) */
static inline unsigned long long __mftb(void) { return 0ULL; }

#endif
