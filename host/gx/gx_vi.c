/* pso-metal P2-AB — host VI/AV-mode implementation for the logo progressive
 * branch (see gx_vi.h for the era cites + W1/gx_shims.c collision note).
 * Register-mirror semantics + first-fire/count instrumentation, mirroring
 * W's gx_shims.c ENTER discipline as an INDEPENDENT registry (duplicate-
 * symbol-free split: exactly one of gx_shims.o / gx_vi.o links per binary).
 */
#include "gx_vi.h"
#include "gx_era.h"   /* int VIGetDTVStatus(void) decl — decomp-0-arg form */
#include <stdio.h>

/* ---- mirror state (era: VI_MODE DTV bit + VI_GEN_MODE prog bit) ---- */
static u32 g_dtv;
static u32 g_prog;

/* ---- counters ---- */
static u32 g_dtv_calls, g_set_calls, g_get_calls, g_last_prog_value;
static int g_dtv_logged, g_set_logged, g_get_logged;

void gx_vi_boot(int dtv, int progressive) {
    g_dtv  = dtv ? 1u : 0u;
    g_prog = progressive ? 1u : 0u;
    gx_vi_reset_counters();
}

void gx_vi_cold_boot(void) {
    gx_vi_boot(GX_VI_DTV_OUTCOME, GX_VI_PROG_OUTCOME);
}

void gx_vi_reset_counters(void) {
    g_dtv_calls = g_set_calls = g_get_calls = g_last_prog_value = 0;
    g_dtv_logged = g_set_logged = g_get_logged = 0;
}

/* era vi.h:80 — 1 when a progressive-capable (DTV) display is attached.
 * gx_era.h:52 pins the decomp-era int(void) form (TAdVIConfigGC.cpp:201,226). */
int VIGetDTVStatus(void) {
    g_dtv_calls++;
    if (!g_dtv_logged) {
        g_dtv_logged = 1;
        printf("[GX-VI] VIGetDTVStatus() -> %u\n", g_dtv);
    }
    return (int)g_dtv;
}

/* era OSRtc.h:84 — port mirror of the progressive-mode bit (real era routes
 * VIChangeMode through the VI interrupt; that machinery is the display lane).
 * disable_progressive_mode's chain terminates here (TAdVIConfigGC.cpp:192). */
void OSSetProgressiveMode(u32 on) {
    g_set_calls++;
    g_last_prog_value = on ? 1u : 0u;
    if (!g_set_logged) {
        g_set_logged = 1;
        printf("[GX-VI] OSSetProgressiveMode(%u) fired\n", on);
    }
    g_prog = g_last_prog_value;
}

/* era OSRtc.h:83 — compares != 1U in check_progressive_ready's second gate. */
u32 OSGetProgressiveMode(void) {
    g_get_calls++;
    if (!g_get_logged) {
        g_get_logged = 1;
        printf("[GX-VI] OSGetProgressiveMode() -> %u\n", g_prog);
    }
    return g_prog;
}

u32 gx_vi_dtv_calls(void)      { return g_dtv_calls; }
u32 gx_vi_set_prog_calls(void) { return g_set_calls; }
u32 gx_vi_last_prog_value(void){ return g_last_prog_value; }
u32 gx_vi_get_prog_calls(void) { return g_get_calls; }

void gx_vi_report(const char *tag) {
    printf("[GX-VI] %s: dtv_calls=%u prog_write_calls=%u last_write=%u prog_read_calls=%u\n",
           tag, g_dtv_calls, g_set_calls, g_last_prog_value, g_get_calls);
}
