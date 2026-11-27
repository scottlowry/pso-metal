/* pso-metal P2-AB (lane V successor) — host VI/AV-mode surface for the logo
 * progressive branch. Era spec: libs/dolphin/include/dolphin/vi.h:80 +
 * os/OSRtc.h:83-84 @ pso-decomp 6b38b5383; consumers TAdVIConfigGC.cpp:201
 * (on_accept) and :226-236 (TLogo::check_progressive_ready).
 *
 * This TU REPLACES W's counted VIGetDTVStatus (gx_shims.c:156 hardcodes the
 * W1 logo-min contract return 0) with a BOTH-OUTCOMES-SELECTABLE version so
 * the progressive branch becomes port-testable:
 *   - compile flag  : -DGX_VI_DTV_OUTCOME=0|1 (+ -DGX_VI_PROG_OUTCOME=0|1)
 *   - runtime       : gx_vi_boot(dtv, progressive) from vi_branch_driver
 * Never link gx_shims.o and gx_vi.o together (duplicate VIGetDTVStatus).
 *
 * Era semantics preserved (register-mirror grade — full VI register behaviour
 * is the display lane's problem, docs/p2-gx-window.md):
 *   VIGetDTVStatus        1 = progressive-capable display attached (VI_MODE
 *                         DTV bit). era u32 zero-arg (vi.h:80); the decomp-
 *                         era call sites + gx_era.h:52 widen to int(void) —
 *                         value set identical {0,1}, decomp wins per
 *                         gx_era.h's "decomp call sites are the spec".
 *   OSSetProgressiveMode  set/clear the progressive mode mirror (OSRtc.h:84,
 *                         era routes VIChangeMode; port records write + value).
 *   OSGetProgressiveMode  read the mirror (OSRtc.h:83, era compares != 1U).
 * All instrumented with first-fire logging + counters (own registry, same
 * discipline as W's ENTER macros) — this is the branch evidence source.
 */
#ifndef PSO_GX_VI_H
#define PSO_GX_VI_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* era OSRtc pair (dolphin/os/OSRtc.h:83-84). VIGetDTVStatus lives in gx_era.h. */
u32 OSGetProgressiveMode(void);
void OSSetProgressiveMode(u32 on);

/* compile-flag cold-boot defaults */
#ifndef GX_VI_DTV_OUTCOME
#define GX_VI_DTV_OUTCOME 0
#endif
#ifndef GX_VI_PROG_OUTCOME
#define GX_VI_PROG_OUTCOME 0
#endif

/* scenario control */
void gx_vi_cold_boot(void);                  /* apply compile-flag defaults  */
void gx_vi_boot(int dtv, int progressive);   /* explicit scenario outcome    */
void gx_vi_reset_counters(void);

/* instrumented counters (gate reads these) */
u32 gx_vi_dtv_calls(void);
u32 gx_vi_set_prog_calls(void);
u32 gx_vi_last_prog_value(void);             /* value of last OSSetProgressiveMode write */
u32 gx_vi_get_prog_calls(void);
void gx_vi_report(const char *tag);          /* one-line [GX-VI] summary     */

#ifdef __cplusplus
}
#endif
#endif /* PSO_GX_VI_H */
