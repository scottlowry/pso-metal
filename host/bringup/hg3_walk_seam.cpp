/* P2-HG3 rung-6 "the walk" — bringup-side seam backings (card BRINGUP-4,
 * spec docs/p2-rung6-seams-1.md §1 S2-S7/S18 + §3 Card BRINGUP-4).
 *
 * Every symbol here is either (i) a class-(c)/(d) WITNESS stand-in for an era
 * body that has NOT landed (it records the entry and never fakes era
 * behavior), (ii) an LB-3 fail-loud per the brief ("shutdown_and_reset_system
 * = LB-3 fail-loud"; unregistered main_phase_table slots = loud canaries), or
 * (iii) a counted extern behind a REAL verbatim slice (delete_children —
 * allocator seam class (c); the 18 logo-table slots — the REAL texture legs
 * are the gx seat's, host/gx/dl/gx_logo_driver.cpp:233-238/:867-877, cited at
 * the generated table; the bringup binary does not link host/gx/**,
 * integrator-owned: deviation L8-5).
 *
 * NO l7_note events fire from this file: the rung-5 LC7_* order vector stays
 * byte-identical (WHAT-GREEN "ladder7_test stays byte-green").
 */
#include "ladder7_run_prelude.h"
#include <stdio.h>
#include <stdlib.h>

/* ---- dispatcher externs (S2) ---- */
s32 actor_frame_tick;                    /* era TLobby_800FD78C.h:8 — dispatcher :379 ++ */
int g_init_game_systems_calls;           /* seam stats (HG3) */
int g_shutdown_loud_fires;
int g_logo_rev_fires[18];                /* per-slot witness: only odd (release) slots can ever fire */
int g8_logo_init_calls;                  /* row-1 init witness entry count */

/* ---- harness-visible log (walk-side; separate from LC7_*) ---- */
int g8_log_n, g8_logo_exit_calls, g8_delkids_calls, g8_init10_calls;
int g8_trial_init_calls, g8_trial_exit_calls, g8_fires, g8_callfn_calls, g8_delkids_n;
const void *g8_delkids_roots[32];
const char *g8_log[128];
void g8_note(const char *tag) { if (g8_log_n < 128) g8_log[g8_log_n++] = tag; }  /* prelude-decl'd; test harness also notes */

/* ---- S5 roots: era TObject.cpp:49 zero-defs (host-carrier objects so the
 * verbatim 19-root body runs; W-POOL regime untouched — the counted extern
 * never dereferences members). Definition order = era body order. ---- */
static TObject tl_obj_00, tl_obj_camera, tl_obj_01, tl_obj_02, tl_obj_03,
               tl_obj_item_equip, tl_obj_loc_start, tl_obj_04, tl_obj_05,
               tl_obj_06, tl_obj_07, tl_obj_loc_end, tl_obj_window,
               tl_obj_particle, tl_obj_particlemodel, tl_obj_radermap,
               tl_obj_clipout, tl_obj_fade, tl_obj_fadeafter;
TObject *tl_00 = &tl_obj_00, *tl_camera = &tl_obj_camera, *tl_01 = &tl_obj_01,
        *tl_02 = &tl_obj_02, *tl_03 = &tl_obj_03, *tl_item_equip = &tl_obj_item_equip,
        *tl_loc_start = &tl_obj_loc_start, *tl_04 = &tl_obj_04, *tl_05 = &tl_obj_05,
        *tl_06 = &tl_obj_06, *tl_07 = &tl_obj_07, *tl_loc_end = &tl_obj_loc_end,
        *tl_window = &tl_obj_window, *tl_particle = &tl_obj_particle,
        *tl_particlemodel = &tl_obj_particlemodel, *tl_radermap = &tl_obj_radermap,
        *tl_clipout = &tl_obj_clipout, *tl_fade = &tl_obj_fade,
        *tl_fadeafter = &tl_obj_fadeafter;

/* HG3-274 (c): the allocator seam stays a counted extern PER CLASS — the
 * verbatim body drives the 19-root order; this records the caller identity
 * (pointer vs the era-ordered tl_* roster) without touching pool state. */
void TObject::delete_children() {
    g8_delkids_calls++;
    if (g8_delkids_n < 32) g8_delkids_roots[g8_delkids_n++] = this;
}

/* ---- S2 dispatcher pre/post seams ---- */
int g8_init_skip_reset = 0;              /* test arms: continue a walk across re-entry (deviation L8-3 disclosed at the seam; era init_game_systems :296+ is the full boot chain — only the phase-state reset effect is in HG3 scope; the ladder3 suite owns the rest) */
void init_game_systems(void) {
    g_init_game_systems_calls++;
    if (!g8_init_skip_reset) reset_main_phase_state();   /* era-true: sets cur=1 (.bss init + :303 call) */
}
const char *p8_fail_slot_name = "unset";                 /* test names the slot before each dispatch */
static void l8_fatal(const char *what) {
    fprintf(stderr, "L8 FATAL: %s — era-zero/contract violation (LB-3)\n", what);
    exit(9);
}
void (*g8_flush_hook)(void);             /* test arms: persists counters into SHARED mem before a loud exit (forked canary runs) */
void shutdown_and_reset_system(void) {   /* era SystemBootInit.cpp:124 — LB-3 fail-loud (brief: 'shutdown_and_reset_system = LB-3 fail-loud') */
    g_shutdown_loud_fires++;
    if (g8_flush_hook) g8_flush_hook();
    l8_fatal("shutdown_and_reset_system (era main tail — loop-exit walk completed)");
}
void p8_fail_loud_main_phase(void) {     /* unregistered main_phase_table slot: the loud canary (spec §3 'canary = unregistered row index trips loud') */
    g8_fires++;
    if (g8_flush_hook) g8_flush_hook();
    char buf[160];
    snprintf(buf, sizeof buf, "main_phase_table slot LAUNCHED unregistered: %s", p8_fail_slot_name);
    l8_fatal(buf);
}

/* ---- S3 row witnesses (class (c)/(d) stand-ins; era bodies land on later
 * BRINGUP-5 legs — these RECORD, never fake) ---- */
void p8_enter_logo_init(void) { g8_logo_init_calls++; g8_note("enter-logo-init"); }   /* era TLogo::init_logo_scene @TLogo.cpp:66 — (d) load chain (lane J + S8 forward walk) */
void p8_enter_trial_init(void) { g8_trial_init_calls++; g8_note("enter-trial-init"); } /* era init_trial_agreement_phase — S9/HG3-275; 'table[0xA].init entered' witness */
void p8_counting_exit_trial(void) { g8_trial_exit_calls++; g8_note("exit-trial-witness"); } /* era TLobby::release_scene_resources — S17/HG3-283 (next-rung tail) */

/* ---- S7 logo-table slot backings (deviation L8-5; the GENERATED table
 * (gen_ladder8_walk.py) wires these into the 9 era pairs in era order.
 * REAL-in-gx-seat cites are printed at the table. The reverse walk fires the
 9 RELEASE halves ONLY; load halves ride the LobbyFrameEntry.id slot (never
 * called) — their counters MUST stay 0 (test asserts). ---- */
#define P8SLOT(name) void name(void) { g8_note(#name); }
/* pair 0 */ void p8_cam_reset_view_state(void) { }  /* load half — never called */
             P8SLOT(p8_cam_reset)                    /* (d) lane-J — RELEASE */
/* pair 1 */ void p8_cam_init_default_view(void) { }
             P8SLOT(p8_cam_reset_default_view)        /* (d) lane-J — RELEASE */
/* pair 2 */ void p8_lobby_update_keyboard_menu(void) { }
             P8SLOT(p8_lobby_free_keyboard_soft)      /* (d) font/lobby lane — RELEASE */
/* pair 3 */ void p8_tex_register_f256_ad_pic_2(void) { }
             P8SLOT(p8_chrsel_free_global)            /* (d) chr-select lane — RELEASE */
/* pair 4 */ void p8_tex_register_f256_backtext(void) { }
             P8SLOT(p8_chrsel_free_singleton)         /* (d) — RELEASE */
/* pair 5 */ void p8_tex_register_logo_e(void) { }    /* gx REAL: gx_logo_driver.cpp:238-:867 */
             P8SLOT(p8_tex_free_held_object)          /* gx REAL leg: :877 — bringup counter */
/* pair 6 */ void p8_tex_load_sonic(void) { }         /* gx REAL: :236 */
             P8SLOT(p8_tex_free_sonic)                /* gx era-EMPTY: :233 */
/* pair 7 */ void p8_win_reset_windows(void) { }
             P8SLOT(p8_win_free_all_windows)          /* (d) window lane — RELEASE */
/* pair 8 */ void p8_tex_load_adx(void) { }           /* gx REAL: :237 */
             P8SLOT(p8_tex_free_adx)                  /* gx era-EMPTY: :234 — RELEASE */
/* call_funcs_reverse ENTERS once per exit fire (REAL verbatim slice drives it;
 * the per-slot witnesses below are the reverse-order log). */
void p8_callfn_enter(void) { g8_callfn_calls++; }     /* unused hook kept for symmetry; slice is verbatim, no interior note */
