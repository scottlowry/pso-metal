/* pso-metal P2-AG first-link backings for the in-N-scaffold host symbols.
 *
 * Task contract (docs/p2-link-deps.md §"9 host symbols"): the first link
 * must back all 9 scaffold symbols. Three of them already have committed
 * implementations that are LINKED, not duplicated here:
 *   OSCreateHeap / OSAllocFromHeap / OSFreeToHeap  -> host/bringup/osalloc_era.c (P2-Y era OSAlloc port)
 * The remaining six get minimal-correct backings below. Each entry states
 * the era signature it matches and why the semantics are minimal-CORRECT
 * (not a wrong-behaving stub) on this host.
 *
 * Type widths: era `BOOL` = int (decomp include/types.h:31) and the host
 * scaffold's psosh_types.h:24 also uses int — no width mismatch anywhere.
 * The scaffold's PSOSH_TAG(fn,...) macro (psosh_types.h:123) expands to
 * `int fn(void)` placeholders, i.e. the scaffold is a symbol INVENTORY,
 * not a prototype surface; the real signatures below follow the era
 * dolphin headers (OSInterrupt.h:154/156, dvd.h:121).
 */
#include <stdio.h>
#include <string.h>

typedef int      BOOL;
typedef int      s32;
typedef unsigned u32;

/* ---- OSInterrupt pair (scaffold #4 / #10, MODERATE) --------------------
 * Era: BOOL OSDisableInterrupts(void); BOOL OSRestoreInterrupts(BOOL);
 * The bringup is single-threaded (no OS threads exist yet — docs/p2-
 * bringup-1.md D-plan), so the interrupt mask is correctly modelled by
 * one flag: disable returns the PREVIOUS enabled-state (era TRUE/FALSE
 * convention) and blocks; restore writes the saved flag back. Nested
 * pairs behave exactly as the era middleware expects: only the outermost
 * restore re-enables (the save/restore chain carries the mask). */
static volatile BOOL g_irq_enabled = 1;
BOOL OSDisableInterrupts(void)  { BOOL prev = g_irq_enabled; g_irq_enabled = 0; return prev; }
BOOL OSRestoreInterrupts(BOOL enable) { g_irq_enabled = enable; return enable; }
/* probe used by the boot slice's receipts (host/link-local, not era): */
int pso_host_irq_enabled(void)  { return g_irq_enabled; }

/* ---- DCStoreRange (scaffold tag, TRIVIAL, 6 sites) ---------------------
 * psosh_os.h:81's own entry rationale: "unified cache: no-op except GPU-
 * shared ranges". Apple Silicon's coherence point for the CPU is the
 * system level; the era semantic (flush dirty stores so DMA/GPU readers
 * see them) is covered by a store fence. dmb ish is the honest minimum:
 * correct for every consumer that is not the (later) Metal-shared buffer,
 * which joins when the display-list lane adds didModifyRange. */
void DCStoreRange(void *start, u32 length) {
    (void)start; (void)length;
    __asm__ volatile("dmb ish" ::: "memory");
}

/* ---- GXBegin / GXDrawDone (scaffold #12 / BLOCKING-GPU tag) ------------
 * PREFERRED backing was a link of host/gx/gx_shims.c (committed): it
 * DEFINES both, but referencing it would drag 4 unresolved gxh_* harness
 * symbols (open/flush/present_clear/wait_verify) whose implementation is
 * Z's UNTRACKED dl/ WIP (frozen stop-order e02706e) — linking that is not
 * honest. Neither symbol is referenced by the chosen first-link TU set
 * (ProjectedColorDraw/TLanguageFontPalette stay out; see link.mk comment),
 * so these entries satisfy the 9-symbol contract with call-counters and
 * zero GPU semantics. The real behaviour lands with Z's display-list
 * slice; nothing here pretends otherwise. */
static u32 g_gx_begin_calls, g_gx_drawdone_calls;
void GXBegin(int type, int fmt, unsigned short nverts) {
    (void)type; (void)fmt; (void)nverts;
    g_gx_begin_calls++;
}
void GXDrawDone(void) { g_gx_drawdone_calls++; }
u32  pso_host_gx_call_counts(u32 *begins, u32 *dones) {
    if (begins) *begins = g_gx_begin_calls;
    if (dones)  *dones  = g_gx_drawdone_calls;
    return g_gx_begin_calls + g_gx_drawdone_calls;
}

/* ---- DVDChangeDir (scaffold tag, MODERATE, 2 sites) --------------------
 * Era: BOOL DVDChangeDir(char* dirName) (dvd.h:121); era consumer
 * text_803DEE48.cpp re-declares `s32 DVDChangeDir(const char*)` — both
 * agree on an int-width symbol. Fake-bundle semantics: Y's dvd_shim
 * resolve() (host/bringup/dvd_shim.c:24) flattens the FST root, so a
 * directory change cannot alter which FILE a name maps to. Minimal-
 * CORRECT backing = record the cwd for later consumers (title.cfg loader)
 * and report success, matching the era success word (1). Escape attempts
 * are rejected like resolve() rejects "..". */
char pso_dvd_cwd[256];
BOOL DVDChangeDir(char *dirName) {
    if (!dirName || strstr(dirName, "..") || strlen(dirName) >= sizeof pso_dvd_cwd)
        return 0; /* era FALSE */
    snprintf(pso_dvd_cwd, sizeof pso_dvd_cwd, "%s", dirName);
    return 1;    /* era TRUE */
}
