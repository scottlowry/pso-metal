/* pso-metal bringup PORT types.h — shadows the decomp include/types.h on the
 * -I path (assert-policy.md §4 mechanism: all 3553 decomp includes are quoted
 * "types.h", so the earliest -I wins). Deliberately occupies the TYPES_H
 * guard so the era header is never processed in a bringup TU.
 *
 * p1-init-ledger §4 step 1: fixed-width <stdint> rebind for u32/u16 (the era
 * header's `typedef unsigned long u32` silently becomes 64-bit on LP64 and
 * changes shift/wrap semantics the PPC body depends on) and uaddr/saddr to a
 * pointer's width (types.h's own port contract, p1-datamodel §3 rules 1-2).
 * Everything else mirrors the era header exactly so parse behavior stays
 * identical for later steps.
 */
#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef int8_t   s8;
typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint16_t u16;
typedef int32_t  s32;
typedef uint32_t u32;
typedef int64_t  s64;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

/* An integer that holds an address: pointer's width in a port. */
typedef uintptr_t uaddr;
typedef intptr_t  saddr;

typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;
typedef volatile s8 vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;
typedef volatile f32 vf32;
typedef volatile f64 vf64;

typedef int BOOL;
#define TRUE 1
#define FALSE 0

/* size_t block copied VERBATIM from era include/types.h — macOS <stdio.h>
 * etc. honor _SIZE_T_DEFINED and would otherwise lose size_t entirely. */
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
#ifndef __SIZE_TYPE__
typedef unsigned long size_t;
#else
typedef __SIZE_TYPE__ size_t;
#endif
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

#define AT_ADDRESS(addr) : (addr)
#define ATTRIBUTE_ALIGN(num) __attribute__((aligned(num)))
#define ATTRIBUTE_ALIGN_PARAM(num) __attribute__((aligned(num)))
#define DECL_WEAK __declspec(weak)
#define __REGISTER register

/* KEEP honest by default. The deployed build mechanism (build/Makefile)
 * force-includes build/waivers/pso_assert_waivers.h AFTER this header, which
 * #undefs this and installs the per-name PSO_LA_FLAG_* dispatch — waived
 * names collapse, unknown names stay fail-closed. -DPSO_PORT_LAYOUT_WAIVER
 * is passed by bringup.mk per ledger step 1; the dispatch already encodes
 * the generated waiver list, so the flag here only asserts the contract. */
#define LAYOUT_ASSERT(name, cond) typedef char name[(cond) ? 1 : -1]

#ifndef PSO_PORT_LAYOUT_WAIVER
#define PORT_LAYOUT_WAIVER_NOT_DEFINED 1
/* Intentional compile failure if the flag is missing — step 1 is mandatory.
 * Named oddly so no era unit ever defines it. */
typedef char pso_bringup_requires_PSO_PORT_LAYOUT_WAIVER_assert_flag[(-1)];
#endif

#endif /* TYPES_H */
