/* pso-metal P2-D4 facade DECLARATION header (card t_075ab4e1). INERT until
 * `-I compat/facade` lands (docs/p2-d4facade-1.md; -I-path-only, never
 * -include — collision law).
 *
 * WHY: era `libs/dolphin/include/dolphin/os/OSTime.h:15` declares
 * `OSTime OSGetTime(void)` with `typedef s64 OSTime` (:12), while the rebind
 * substrate `host/bringup/pso_bringup.h:60` declares `u64 OSGetTime(void)`
 * (:61 `u32 OSGetTick(void)`). Every TU chain that hits the bringup OSAlloc.h
 * shadow first (era os.h:5) and then OSTime.h (os.h:22) gets
 * `conflicting types for 'OSGetTime'` at OSTime.h:15 — 12 official LP64 TUs
 * carry it (docs/p2-d4facade-1.md table; docs/p2-mwccasm-1.md §4 co-class).
 * There is NO bringup OSTime.h shadow, so any -I splice of the facade ahead
 * of -I libs/dolphin/include wins the lookup; occupying era guard OS_TIME_H
 * (:1) additionally neutralizes the era file on edges that reach it anyway.
 *
 * SELF-CONTAINED BY MEASURE: this header must NOT re-export pso_bringup.h —
 * a scratch OF-B sweep proved wholesale re-export drags the substrate's
 * DVDCommandBlock/DVDFileInfo structs into TUs that previously reached
 * OSTime without the substrate, redefining compat/repin/dvd.h:36/:51
 * (collateral on official-clean McFsGcRequests.cpp; receipt
 * cache/scratch/d4/, docs/p2-d4facade-1.md §3). It therefore mirrors the era
 * file's OWN legs: "types.h" verbatim era :3 (same -I resolution chain as
 * today) + declarations only. OSGetTime/OSGetTick are declared with the
 * SUBSTRATE-consistent signatures (pso_bringup.h:60-61: u64/u32) — the one
 * deliberate deviation from era-verbatim return types, required so either
 * inclusion order (facade-first or substrate-first) is a legal
 * re-declaration, never a collision; era OSTime.h:15/17 return `OSTime`
 * (s64), substrate pins u64 — substrate wins in the facade world. OSTime
 * itself keeps its era type s64 (:12; layout-identical int64_t on LP64).
 * No definitions; time implementations stay host/bringup (osinit.c). */
#ifndef PSO_FACADE_DOLPHIN_OS_OSTIME_H
#define PSO_FACADE_DOLPHIN_OS_OSTIME_H
#include "types.h" /* era OSTime.h:3 verbatim — same resolution edge as the
                    * era file under the rebind (bringup types.h wins -I) */
#ifndef OS_TIME_H
#define OS_TIME_H /* occupy era guard (OSTime.h:1) so the colliding original
                   * is skipped on every edge regardless of splice order */
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef s64 OSTime; /* OSTime.h:12 era-verbatim */
typedef u32 OSTick; /* OSTime.h:13 era-verbatim */

/* Substrate-consistent twins (pso_bringup.h:60-61; return-type deviation
 * from era :15/:17 is deliberate — see header why): */
u64   OSGetTime(void);
OSTick OSGetTick(void); /* OSTick==u32: era :17 type-identical */

/* Era decls with no substrate twin — verbatim signatures, line-cited: */
OSTime __OSGetSystemTime(void);                    /* OSTime.h:16 */

typedef struct OSCalendarTime_s {                  /* OSTime.h:19-31 */
  int sec;
  int min;
  int hour;
  int mday;
  int mon;
  int year;
  int wday;
  int yday;

  int msec;
  int usec;
} OSCalendarTime;

OSTime OSCalendarTimeToTicks(OSCalendarTime* td);  /* OSTime.h:33 */
void OSTicksToCalendarTime(OSTime ticks, OSCalendarTime* td); /* OSTime.h:34 */

/* Era macro + constants, verbatim: */
#define __OSSystemTime (OSTime*)0x800030D8         /* OSTime.h:8 (guest
                                                   * absolute — host mapping
                                                   * is per-TU port ground,
                                                   * not this supply) */
#define OSDiffTick(tick1, tick0) ((s32)(tick1) - (s32)(tick0)) /* :10 */

#define USEC_MAX     1000                          /* OSTime.h:36 */
#define MSEC_MAX     1000                          /* OSTime.h:37 */
#define MONTH_MAX    12                            /* OSTime.h:38 */
#define WEEK_DAY_MAX 7                             /* OSTime.h:39 */
#define YEAR_DAY_MAX 365                           /* OSTime.h:40 */

#define SECS_IN_MIN  60                            /* OSTime.h:42 */
#define SECS_IN_HOUR (SECS_IN_MIN * 60)            /* OSTime.h:43 */
#define SECS_IN_DAY  (SECS_IN_HOUR * 24)           /* OSTime.h:44 */
#define SECS_IN_YEAR (SECS_IN_DAY * 365)           /* OSTime.h:45 */

#define BIAS 0xB2575                               /* OSTime.h:47 */

#ifdef __cplusplus
}
#endif
#endif /* PSO_FACADE_DOLPHIN_OS_OSTIME_H */
