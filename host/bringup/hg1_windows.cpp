/* P2-HG1 — window registry + runtime crossing helpers, REAL form
 * (docs/p2-handle-program-1.md §3; sketch compat/njs/handle/ stays inert —
 * this TU is the landed version, same names, no repo file -I's the sketch).
 *
 * The crt0 base registers (pso_arena_base / pso_aram_base, crt0.c) are the
 * ONLY host memory this seam ever converts against; windows are records in
 * the table below, bound by loaders (here: the HG-1 golden drives binds off
 * the crt0 era-window anchor table). Resolution law (HP §3.2/§3.4):
 *   slot == 0              -> NULL                      (NULL law)
 *   slot >= 0x80000000     -> covering main-RAM record  (or FAIL LOUD)
 *   aligned, >= ARAM floor -> covering ARAM record      (or FAIL LOUD)
 *   anything else          -> FAIL LOUD                 (W-INT guard)
 * The stamp ledger enforces stamp-at-genesis WRITE-ONCE (HP §3.2): a second
 * stamp at the same slot location is a loud reject and never overwrites.
 */
#include "hg1_windows.h"
#include "pso_bringup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN_BAD_SLOT 0xFFFFFFFFu   /* reject sentinel from h2g in test mode  */

/* ---------- registry ---------- */
struct rec {
    enum pso_handle_window win;
    u32 guest_begin, guest_end;
    u8 *host_begin;
};
static struct rec g_recs[PSO_HANDLE_WINDOW_MAX];
static u32 g_nrecs;

static int g_test_mode;
volatile u32 pso_handle_reject_count;

static void fail_loud(const char *why, u32 v) {
    fprintf(stderr, "HG1-REJECT: %s (slot/val 0x%08X)\n", why, v);
    pso_handle_reject_count++;
    if (!g_test_mode) abort();     /* no silent path outside a golden harness */
}

void pso_handle_test_mode(int on) { g_test_mode = on; }
u32 pso_handle_window_records(void) { return g_nrecs; }

void pso_handle_window_bind(enum pso_handle_window win,
                            u32 guest_begin, u32 guest_end, u8 *host_begin) {
    if (win == PSO_W_NONE || win >= PSO_W__COUNT || guest_end <= guest_begin ||
        !host_begin) {
        fail_loud("malformed window_bind", guest_begin);
        return;
    }
    /* main-RAM windows must speak tag space; ARAM must speak offset space —
       an off-by-space bind is exactly the offset-space collision AR §5 named */
    int aram = (win == PSO_W_ARAM);
    if (aram == (guest_begin >= PSO_HANDLE_GUEST_RAM_BASE)) {
        fail_loud("window_bind wrong address space for class", guest_begin);
        return;
    }
    for (u32 i = 0; i < g_nrecs; i++) {
        struct rec *r = &g_recs[i];
        if (r->win != win) continue;
        /* same window class: overlapping spans conflict; identical triples
           re-bind idempotently (deterministic boot, HP §3.1) */
        int overlap = guest_begin < r->guest_end && r->guest_begin < guest_end;
        if (overlap) {
            if (r->guest_begin == guest_begin && r->guest_end == guest_end &&
                r->host_begin == host_begin)
                return;                        /* idempotent same-triple re-bind */
            fail_loud("conflicting re-bind (record kept, not shadowed)",
                      guest_begin);
            return;
        }
    }
    if (g_nrecs >= PSO_HANDLE_WINDOW_MAX) { fail_loud("registry full", g_nrecs); return; }
    struct rec *r = &g_recs[g_nrecs++];
    r->win = win; r->guest_begin = guest_begin; r->guest_end = guest_end;
    r->host_begin = host_begin;
}

void pso_handle_window_unbind(enum pso_handle_window win) {
    for (u32 i = 0; i < g_nrecs; ) {
        if (g_recs[i].win == win) {
            g_recs[i] = g_recs[g_nrecs - 1];
            memset(&g_recs[g_nrecs - 1], 0, sizeof(struct rec));
            g_nrecs--;
        } else i++;
    }
}

static const struct rec *find_main(u32 slot) {
    for (u32 i = 0; i < g_nrecs; i++)
        if (g_recs[i].win != PSO_W_ARAM &&
            slot >= g_recs[i].guest_begin && slot < g_recs[i].guest_end)
            return &g_recs[i];
    return 0;
}
static const struct rec *find_aram(u32 slot) {
    for (u32 i = 0; i < g_nrecs; i++)
        if (g_recs[i].win == PSO_W_ARAM &&
            slot >= g_recs[i].guest_begin && slot < g_recs[i].guest_end)
            return &g_recs[i];
    return 0;
}

/* ---------- crossing ---------- */
u8 *pso_njs_handle_g2h(u32 slot) {
    if (slot == 0) return (u8 *)0;                    /* NULL law (§3.2)     */
    if (slot >= PSO_HANDLE_GUEST_RAM_BASE) {          /* arena regex first   */
        const struct rec *r = find_main(slot);
        if (r) return r->host_begin + (slot - r->guest_begin);
        fail_loud("main-RAM slot with no covering record", slot);
        return (u8 *)0;
    }
    /* ARAM-offset leg: only registered-shape offsets qualify (W-INT guard) */
    if (slot < PSO_HANDLE_ARAM_MIN_OFFSET || (slot & 0x1Fu)) {
        fail_loud("unregistered small int / W-IMMEDIATE-in-slot-dress", slot);
        return (u8 *)0;
    }
    const struct rec *r = find_aram(slot);
    if (r) return r->host_begin + slot;               /* guest == offset     */
    fail_loud("offset slot with no bound ARAM window", slot);
    return (u8 *)0;
}

u32 pso_njs_handle_h2g_main(const u8 *p) {
    if (!p) return 0;                                 /* NULL law            */
    for (u32 i = 0; i < g_nrecs; i++) {
        const struct rec *r = &g_recs[i];
        if (r->win == PSO_W_ARAM) continue;
        u32 span = r->guest_end - r->guest_begin;
        if (p >= r->host_begin && p < r->host_begin + span)
            return r->guest_begin + (u32)(p - r->host_begin);
    }
    fail_loud("h2g_main on pointer outside every bound main-RAM window "
              "(stack/unregistered — W-DERIVED/W-CODE dress)", 0);
    return WIN_BAD_SLOT;
}

u32 pso_njs_handle_h2g_aram(const u8 *p) {
    if (!p) return 0;
    for (u32 i = 0; i < g_nrecs; i++) {
        const struct rec *r = &g_recs[i];
        if (r->win != PSO_W_ARAM) continue;
        u32 off = (u32)(p - r->host_begin);
        if (p >= r->host_begin && off < r->guest_end - r->guest_begin) {
            if (off < PSO_HANDLE_ARAM_MIN_OFFSET || (off & 0x1Fu)) {
                fail_loud("h2g_aram on non-ARAlloc-shaped offset", off);
                return WIN_BAD_SLOT;
            }
            return off;                               /* ARAlloc-offset space */
        }
    }
    fail_loud("h2g_aram on pointer outside every bound ARAM window", 0);
    return WIN_BAD_SLOT;
}

int pso_handle_slot_in_window(u32 slot, enum pso_handle_window win) {
    if (win == PSO_W_ARAM) {
        const struct rec *r = find_aram(slot);
        return r && r->win == win;
    }
    const struct rec *r = find_main(slot);
    return r && r->win == win;
}

/* ---------- stamp-at-genesis ledger (write-once) ---------- */
struct stamp { u32 *loc; u32 value; };
static struct stamp g_stamps[PSO_HANDLE_STAMPS_MAX];
static u32 g_nstamps;

u32 pso_handle_stamp_count(void) { return g_nstamps; }

u32 pso_handle_stamp_once(u32 *slot_loc, const u8 *host_ptr,
                          enum pso_handle_window win) {
    u32 value = (win == PSO_W_ARAM) ? pso_njs_handle_h2g_aram(host_ptr)
                                    : pso_njs_handle_h2g_main(host_ptr);
    if (value == WIN_BAD_SLOT) return WIN_BAD_SLOT;   /* never wrote         */
    for (u32 i = 0; i < g_nstamps; i++) {
        if (g_stamps[i].loc != slot_loc) continue;
        fail_loud("second stamp at a genesis-stamped slot location "
                  "(write-once law, HP §3.2)", g_stamps[i].value);
        return g_stamps[i].value;                     /* bytes stay untouched */
    }
    if (g_nstamps >= PSO_HANDLE_STAMPS_MAX) {
        fail_loud("stamp ledger full", g_nstamps);
        return WIN_BAD_SLOT;
    }
    g_stamps[g_nstamps].loc = slot_loc;
    g_stamps[g_nstamps].value = value;
    g_nstamps++;
    *slot_loc = value;                                /* THE one write        */
    return value;
}
