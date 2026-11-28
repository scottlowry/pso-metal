/* P2-HG3 rung-6 "the walk" — card BRINGUP-4 golden (legs HG3-271..274).
 * Spec docs/p2-rung6-seams-1.md §1 S2/S3/S4/S5/S6/S7/S18 + §3 Card BRINGUP-4.
 * WHAT-GREEN (§3): cur steps 1->0xA through the REAL dispatcher loop;
 * table[1].exit fires exactly once per logo-row departure; table[0xA].init
 * entered; prev chain honest; the result==3 forced walk and the unregistered
 * row index both behave (S18 promotion + LB-3 canary); existing HG suites and
 * ladder7 stay byte-green (separate runs — this binary never touches them).
 *
 * Harness machinery (test-owned, zero era bytes): pso_walk_stop_requested /
 * p8_snapshot_loop_top are the two NON-ERA lines the walk glue ANDs into the
 * verbatim dispatcher (deviations L8-1/L8-2, gen_ladder8_walk.py header);
 * loud runs execute in forked children over MAP_SHARED witness records
 * (L7-P12 precedent, upgraded: counters survive the exit(9)). The era TLogo
 * state-7 fire rides the l7_walk_hook seam (the state machine itself is the
 * rung-3 ladder3 suite's proof — not re-minted).
 */
#include "ladder7_run_prelude.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

extern "C" void DVDInit(void);

static int failures, total;
#define CHECK(cond, ...) do { total++; if (!(cond)) { failures++; \
    printf("FAIL "); printf(__VA_ARGS__); printf("\n"); } \
    else { printf("PASS "); printf(__VA_ARGS__); printf("\n"); } } while (0)

#define SNAP_MAX 48
struct Shm {                    /* written by children, read by the parent */
    int magic;
    int log_n;  const char *log[128];
    int delkids, dk_n; const void *dk[32];
    int rev_n;                  /* release-slot fires (counted from log tags) */
    int trial_exit, fires, hook_fires, frame_tick, present;
    int snap_n; int snaps[SNAP_MAX][4];   /* {cur,prev,next,result} at loop tops */
    int fin[4];                           /* LIVE {cur,prev,next,result} at the last flush (the switch-arm stop happens BETWEEN snapshots) */
    int fire[2];                          /* {next,result} read RIGHT AFTER the REAL request_main_phase(0xA) slice at the state-7 hook site (loop-top snapshots see result pre-zeroed per era :371) */
    int r3fin[4];                         /* pinned at the R3A stop (r3fin[0]==-9 = unset) — R3B's later flushes must not clobber the queued-walk evidence */
    int rq;                               /* request_queued slice return value (child-side, persisted — COW would hide it) */
};
static struct Shm *shm;

/* ---- dispatcher hooks the glue ANDs in (deviation L8-1/L8-2 counterparts) ---- */
static int g_mode;                       /* M_* below */
enum { M_R1, M_R2, M_R3, M_R3B, M_R4, M_OFF };
static s32 (*stop_when)(void);
static int snap_n; static int snaps[SNAP_MAX][4];
static int fb_done, fb_ret = -7, rq_done, rq_ret = -7;
static int te_base;                      /* trial-exit baseline per run (counters persist across in-process runs) */

static void flush(void) {
    if (!shm) return;
    shm->magic = 0x48473301;
    shm->log_n = g8_log_n;  memcpy(shm->log, g8_log, sizeof g8_log);
    shm->delkids = g8_delkids_calls; shm->dk_n = g8_delkids_n;
    memcpy(shm->dk, g8_delkids_roots, sizeof g8_delkids_roots);
    shm->rev_n = g8_delkids_n;       /* release count derived below instead */
    shm->trial_exit = g8_trial_exit_calls; shm->fires = g8_fires;
    shm->hook_fires = l7_walk_hook_fires; shm->frame_tick = actor_frame_tick;
    shm->present = g_present_calls;
    shm->fin[0] = main_phase_state.cur; shm->fin[1] = main_phase_state.prev;
    shm->fin[2] = main_phase_state.next; shm->fin[3] = main_phase_state.result;
    shm->snap_n = snap_n; memcpy(shm->snaps, snaps, sizeof snaps);
    shm->rq = rq_done ? rq_ret : -7;     /* -7 = never fired in THIS process (the snapshot fires live in the R3 child) */
    if (g_mode == M_R3 && shm->fin[1] == 0xA && shm->r3fin[0] == -9) {
        memcpy(shm->r3fin, shm->fin, sizeof shm->fin);         /* pin EXACTLY the switch(2) transition seen live */
    }
}

/* era state-7 fire carrier (TLogo.cpp:193-195 rides this hook, see prelude) */
static int hook_arm;
static void hg3_fire_0a(void) { if (hook_arm > 0) { hook_arm--; g8_note("fire-0xA");
    request_main_phase(0xA);                                   /* REAL rung-5 slice */
    shm->fire[0] = main_phase_state.next; shm->fire[1] = main_phase_state.result; } }

static s32 stop_never(void) { return 0; }
static s32 stop_after_trial_exit(void) { return g8_trial_exit_calls >= te_base + 1; }

void p8_snapshot_loop_top(void) {
    if (snap_n < SNAP_MAX) { snaps[snap_n][0] = main_phase_state.cur;
        snaps[snap_n][1] = main_phase_state.prev; snaps[snap_n][2] = main_phase_state.next;
        snaps[snap_n][3] = main_phase_state.result; }
    snap_n++;
    flush();
    if (g_mode == M_R2 && main_phase_state.cur == 0xA && !fb_done) {
        fb_done = 1;
        set_fallback_main_phase(2);      /* era TTitleScene grammar (:311) sets fallback before switching */
        fb_ret = switch_to_fallback_main_phase(); }                /* forced result==3 walk (S18) */
    if (g_mode == M_R3 && main_phase_state.cur == 0xA && !rq_done) {
        rq_done = 1; rq_ret = request_queued_main_phase(); }       /* result==2 walk (S18 roster) */
}
s32 pso_walk_stop_requested(void) { flush(); return stop_when ? stop_when() : 0; }

static int child_run(void) {
    pso_main_dispatcher();
    if (g_mode == M_R3) {               /* second entry: 1->0 quit walk -> era shutdown LB-3 */
        g_mode = M_R3B; stop_when = stop_never;
        quest_download_server_ready = 1;  /* rung-5 Ea gate REAL: run_frame tail fires request_main_phase(0) */
        pso_main_dispatcher();
    }
    flush();
    return 0;                           /* clean return (only reachable via the L8 return) */
}

static const char *REL_ORDER[9] = {     /* reverse walk over the 9 pairs, back-to-front */
    "p8_tex_free_adx", "p8_win_free_all_windows", "p8_tex_free_sonic",
    "p8_tex_free_held_object", "p8_chrsel_free_singleton", "p8_chrsel_free_global",
    "p8_lobby_free_keyboard_soft", "p8_cam_reset_default_view", "p8_cam_reset",
};
static const char *LOAD_HALVES[9] = {
    "p8_cam_reset_view_state", "p8_cam_init_default_view", "p8_lobby_update_keyboard_menu",
    "p8_tex_register_f256_ad_pic_2", "p8_tex_register_f256_backtext", "p8_tex_register_logo_e",
    "p8_tex_load_sonic", "p8_win_reset_windows", "p8_tex_load_adx",
};
static int count_tag(const char *tag) { int n = 0; for (int i = 0; i < g8_log_n; i++) if (!strcmp(g8_log[i], tag)) n++; return n; }
static int count_tag2(const char *const *log, int n, const char *tag) { int c = 0; for (int i = 0; i < n; i++) if (!strcmp(log[i], tag)) c++; return c; }
static int rev_seq_ok(const char *const *log, int n) {
    int expect = 0;
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < 9; k++) {
            if (!strcmp(log[i], REL_ORDER[k])) { if (k != expect) return 0; expect = (expect + 1) % 9; }
        }
    }
    return expect == 0;                 /* whole cycles of 9 in exact reverse order */
}

int main() {
    setvbuf(stdout, 0, _IOLBF, 0);
    DVDInit();
    shm = (struct Shm *)mmap(NULL, sizeof *shm, PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    CHECK(shm != MAP_FAILED, "HG3-H0 harness: MAP_SHARED witness region live (forked loud runs survive exit(9))");
    g8_flush_hook = flush;

    /* ================= HG3-272 S3: table DATA shape (era-validated by the generator,
     * this is the in-memory read-back of the GENERATED table) ================= */
    CHECK(main_phase_table[0].init == NULL && main_phase_table[0].update == NULL && main_phase_table[0].exit == NULL
          && main_phase_table[17].init == NULL
          && main_phase_table[20].exit == NULL,
          "HG3.272a NULL sentinels: rows [0]/[17]/[20] verbatim-NULL ({NULL,NULL,NULL} x3 era :341/:358/:361)");
    CHECK(main_phase_table[1].update == TLogo::run_frame && main_phase_table[1].exit == TLogo::exit_logo_scene,
          "HG3.272b row[1] landed REAL where rung-5/6 sliced: update==TLogo::run_frame, exit==TLogo::exit_logo_scene (era :342 byte-validated at generation)");
    CHECK(main_phase_table[1].init == p8_enter_logo_init && main_phase_table[10].init == p8_enter_trial_init
          && main_phase_table[2].init == p8_fail_loud_main_phase && main_phase_table[5].update == p8_fail_loud_main_phase
          && main_phase_table[10].update == p8_fail_loud_main_phase,
          "HG3.272c rows {2,5} + row[10] update + row[1] init carry witness/LB-3 stand-ins (era names byte-validated; unlanded bodies never faked — BRINGUP-5 legs own them)");
    CHECK(logo_load_callback_table[1] == p8_cam_reset && logo_load_callback_table[17] == p8_tex_free_adx
          && logo_load_callback_table[0] == p8_cam_reset_view_state,
          "HG3.272d logo table host copy: 18 ptrs / 9 pairs, era order + parity slot backings (generated data, era text roster two-source guarded)");

    /* ================= R1: the REAL walk 1 -> 0xA (forked: ends loud) ========= */
    g_mode = M_R1; stop_when = stop_never; l7_walk_hook = hg3_fire_0a; hook_arm = 1;
    te_base = g8_trial_exit_calls;       /* era default: quest_download_server_ready stays 0 — the is_ready quit leg must NOT steal the first walk */
    p8_fail_slot_name = "main_phase_table[10].update (era run_game_mode_select_frame — S12/HG3-275)";
    int snap0 = snap_n, dk0 = g8_delkids_calls;
    pid_t kid = fork();
    if (kid == 0) _exit(child_run());
    int st = 0; while (waitpid(kid, &st, 0) < 0) { }
    CHECK(kid > 0 && WIFEXITED(st) && WEXITSTATUS(st) == 9,
          "HG3.271a dispatcher child exit(9): the REAL loop drove cur 1->0xA and the UNREGISTERED row-10 update slot tripped the LB-3 loud canary (spec §3 'canary = unregistered row index trips loud')");
    CHECK(shm->snap_n > snap0 + 1 && shm->snaps[snap0][0] == 1 && shm->snaps[snap0][1] == 1
          && shm->snaps[snap0 + 1][0] == 0xA && shm->snaps[snap0 + 1][1] == 1,
          "HG3.271b loop-top snapshots through the REAL switch: iteration1 {cur=1,prev=1} (era reset_main_phase_state seeds — production boot shape), iteration2 {cur=0xA,prev=1} — prev=cur; cur=next is era :389-390 code, not harness replication (L7-Eb1 host-driven shape SUPERSEDED for this leg)");
    CHECK(shm->snaps[snap0 + 1][2] == 0xA && shm->fire[0] == 0xA && shm->fire[1] == 1,
          "HG3.271c loop top iteration2 next==0xA AND {next,result}==(0xA,1) read back RIGHT AFTER the REAL request_main_phase(0xA) slice at the state-7 hook site (era fire TLogo.cpp:193-195; loop-top snapshots see result==0 by era :371 store — both shapes asserted)");
    CHECK(shm->log_n > 0 && !strcmp(shm->log[0], "enter-logo-init")
          && count_tag2(shm->log, shm->log_n, "enter-trial-init") == 1
          && count_tag2(shm->log, shm->log_n, "fire-0xA") == 1,
          "HG3.271d/272e witness order: table[1].init ENTERED (witness), hook fired once, table[0xA].init ENTERED (witness for the HG3-275 body)");
    CHECK(shm->delkids == dk0 + 19 && shm->dk_n >= 19,
          "HG3.274a exit_logo_scene fired exactly ONCE through the dispatcher (19-root delete_children batch: 0->19, no second batch before the loud canary) — rung-6 exit-table-once proof");
    {   /* HG3-274 root roster == era body order */
        TObject *ERA_ROSTER[19] = { tl_00, tl_camera, tl_01, tl_02, tl_03, tl_item_equip,
            tl_loc_start, tl_04, tl_05, tl_06, tl_07, tl_loc_end, tl_window, tl_particle,
            tl_particlemodel, tl_radermap, tl_clipout, tl_fade, tl_fadeafter };
        int ok = 1;
        for (int i = 0; i < 19; i++) if (shm->dk[i] != ERA_ROSTER[i]) ok = 0;
        CHECK(ok, "HG3.274b delete_children callers == the 19 tl_* roots in EXACT era body order (TObject.cpp:576-594 roster driven by the VERBATIM slice; counted extern per class, allocator never touched)");
    }
    {   /* HG3-273 reverse-order witness */
        int rel = 0; for (int i = 0; i < shm->log_n; i++) for (int k = 0; k < 9; k++)
            if (!strcmp(shm->log[i], REL_ORDER[k])) rel++;
        CHECK(rel == 9 && rev_seq_ok(shm->log, shm->log_n),
          "HG3.273a call_funcs_reverse REAL walk fired the 9 RELEASE halves back-to-front exactly once (witness log == free_adx..cam_reset reverse cycle — LobbyFrameEntry pair reinterpretation, TFontSmallTask.cpp:251-261 verbatim)");
        int load_called = 0; for (int i = 0; i < shm->log_n; i++) for (int k = 0; k < 9; k++)
            if (!strcmp(shm->log[i], LOAD_HALVES[k])) load_called++;
        CHECK(load_called == 0, "HG3.273b ZERO load halves called (they ride the id slot — never fn — era pair semantics proven on host layout, L8-4)");
        CHECK(count_tag2(shm->log, shm->log_n, "p8_tex_free_adx") == 1
              && count_tag2(shm->log, shm->log_n, "p8_tex_free_held_object") == 1,
              "HG3.273c texture-pair releases fired once (bringup-side counters; the REAL legs are the gx seat's — gx_logo_driver.cpp:233-238/:867-877 cited in the generated table, never re-minted here — L8-5)");
    }
    CHECK(shm->present >= 1 && shm->frame_tick >= 1 && shm->hook_fires == 1,
          "HG3.271e the walk rode REAL frames: actor_frame_tick advanced in the era update loop (:379), present(s) via the verbatim run_frame/exit render_tick legs, hook fired once at the era fire site");

    /* ================= R2: in-process result==3 forced walk (S18 promotion) === */
    g_mode = M_R2; hook_arm = 1; fb_done = 0; te_base = g8_trial_exit_calls;
    stop_when = stop_after_trial_exit;
    main_phase_table[10].exit = p8_counting_exit_trial;   /* harness swap (era .data is mutable); HG3-283 lands the real release body */
    pso_main_dispatcher();               /* returns via the L8 switch-arm exit once the stop hook arms */
    CHECK(main_phase_state.cur == 2 && main_phase_state.prev == 0xA && main_phase_state.result == 3
          && main_phase_state.next == 2 && fb_ret == 2,
          "HG3.271f/S18 switch_to_fallback_main_phase PROMOTED from the LB-3 stand-in (rung-5 seam :238) to the REAL :173-176 slice: forced result==3 walk — dispatcher switch case 3 ran prev=0xA cur=2, slice RETURN VALUE ==fallback==2");
    CHECK(g8_trial_exit_calls == te_base + 1 && g8_delkids_calls == dk0 + 19,
          "HG3.271g walk 2 (in-process): exit[1] REAL once more (19 NEW delkids batch in THIS process — R1's batch lived in the forked child, COW) + row-10 counting witness entered exactly once — no second logo-exit storm across re-entry");

    /* ================= R3: queued result==2 + shutdown LB-3 (forked) ========== */
    g_mode = M_R3; hook_arm = 1; rq_done = 0; te_base = g8_trial_exit_calls;
    stop_when = stop_after_trial_exit;
    quest_download_server_ready = 0;     /* walk 1 needs the never-ready spine */
    shm->r3fin[0] = -9;                  /* arm the pinned-R3A-transition capture */
    kid = fork();
    if (kid == 0) _exit(child_run());
    while (waitpid(kid, &st, 0) < 0) { }
    CHECK(WIFEXITED(st) && WEXITSTATUS(st) == 9,
          "HG3.271h queued-walk child ended in the era tail: after the second walk (1->0 quit) the REAL loop fell through and shutdown_and_reset_system fired its LB-3 (brief line 4 — fail-loud terminus, era main.cpp tail :394)");
    CHECK(shm->trial_exit == te_base + 1 && shm->hook_fires >= 3,
          "HG3.271i queued-walk child counters frozen in shared memory across the fork: row-10 counting witness entered exactly once, state-7 hook crossed 3x this run (R1+R2+R3) — loud evidence rides the R1/R4 children");
    {   /* the queued transition: the switch(2) arm ran prev=0xA cur=next — the stop-flush
         * captured it live (loop-top snapshot of iteration 3 is never reached: the L8
         * return exits from INSIDE the switch, era-faithful position otherwise). The
         * slice RETURN is persisted via flush (the walk runs in the forked child). */
        CHECK(shm->r3fin[0] == 1 && shm->r3fin[1] == 0xA && shm->r3fin[2] == 1 && shm->r3fin[3] == 2 && shm->rq == 1,
          "HG3.272f request_queued_main_phase (S18 roster, era :214-217) drove the result==2 walk: {cur,prev,next,result}=(1,0xA,1,2) carried by the REAL switch case 2, slice returned queued==1");
    }

    /* ================= R4: unregistered INIT index (row 2) =================== */
    g_mode = M_R4; g8_init_skip_reset = 1; stop_when = stop_never;  /* L8-3: continue from the R2 landing (cur==2) */
    p8_fail_slot_name = "main_phase_table[2].init (era TAdMultiChgGameModeGC::init_title_phase — S9/HG3-275)";
    int fires0 = g8_fires;
    kid = fork();
    if (kid == 0) { _exit(child_run()); }
    while (waitpid(kid, &st, 0) < 0) { }
    CHECK(WIFEXITED(st) && WEXITSTATUS(st) == 9 && shm->fires == fires0 + 1,
          "HG3.272g UNREGISTERED INIT canary: landing on row[2] launched p8_fail_loud_main_phase (exit 9, exactly one new launch) — the {0,1,2,5,10} roster trips loud everywhere else, spec §3");
    g8_init_skip_reset = 0;

    /* ================= R5: S18 helper direct semantics (in-process) ========== */
    MainPhaseState save = main_phase_state;
    main_phase_state = MainPhaseState{};
    set_fallback_main_phase(5); set_queued_main_phase(7);
    CHECK(switch_to_fallback_main_phase() == 5 && main_phase_state.result == 3 && main_phase_state.next == 5,
          "HG3.273h/S18 switch_to_fallback: result=3, next=fallback returned — era :173-176 stores read back direct");
    CHECK(request_queued_main_phase() == 7 && main_phase_state.result == 2 && main_phase_state.next == 7,
          "HG3.273i/S18 request_queued: result=2, next=queued returned — era :214-217");
    main_phase_state.cur = 3; main_phase_state.prev = 1;
    CHECK(get_main_phase() == 3 && get_previous_main_phase() == 1, "HG3.273j get_main_phase/get_previous_main_phase REAL reads (era :157-163)");
    reset_main_phase_state();
    CHECK(main_phase_state.cur == 1 && main_phase_state.prev == 1 && main_phase_state.fallback == 1
          && main_phase_state.queued_main_phase == 1,
          "HG3.271j reset_main_phase_state era :224-229 verbatim: cur/prev/queued/fallback all 1 (production-boot seeding; MainPhaseState carries the FULL 6-field era layout, L8-2)");
    main_phase_state = save;

    main_phase_table[10].exit = p8_fail_loud_main_phase;   /* restore the generated stand-in */
    g8_note("harness-end");
    CHECK(count_tag2(g8_log, g8_log_n, "harness-end") == 1, "HG3-H9 harness closure: witness log bounded, table stand-ins restored to generated state");

    printf("\n== P2-HG3 rung-6 the walk (HG3-271..274) ==\n");
    printf("  dispatcher body (SystemBootInit.cpp main, renamed L8-1) + 5 S18 helpers + exit_logo_scene + call_funcs_reverse + delete_layer_children EXECUTE era bodies verbatim; the 22-row main_phase_table + 18-ptr logo table are GENERATED data (era-text byte-validated, two-source); loud canaries + shutdown LB-3 prove the roster trips loud\n");
    printf("== P2-HG3 hg3_walk: %d/%d PASS ==\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
