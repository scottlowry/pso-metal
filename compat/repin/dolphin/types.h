#ifndef _DOLPHIN_TYPES_H_
#define _DOLPHIN_TYPES_H_
/* compat/repin shadow of libs/dolphin/include/dolphin/types.h (P2-AW).
 * WHY: era file re-typedefs s8..u64/uaddr/saddr/vu* + `typedef unsigned int
 * uintptr_t` with its OWN guard _DOLPHIN_TYPES_H_; under the port types.h
 * rebind those 5-6 typedefs collide (a 5-error cluster in 1,144/1,680 corpus
 * files — the AS2 §5 rebind-vs-dolphin class, now quantified tree-wide).
 * This shadow keeps the guard (era dupes then never process), delegates the
 * scalar block to the port rebind via <types.h> (earliest -I = fixed-width +
 * pointer-width uaddr/saddr, J rule-1, correct-width uintptr_t), and carries
 * the remaining era surface VERBATIM below (Ptr/uint/BOOL/TRUE/FALSE/
 * AT_ADDRESS/ATTRIBUTE_ALIGN/__REGISTER/INT_MIN|MAX/NULL + libc legs).
 * Nothing else in dolphin/types.h is layout-visible; Vec etc. live in mtx.h
 * untouched. Requires -Ihost/bringup/include (types.h fail-closed guard). */
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

#include "libc/stdio.h"
#include "libc/stdarg.h"
#include "libc/string.h"
#include "libc/ctype.h"

#include "libc/math.h"

#endif
