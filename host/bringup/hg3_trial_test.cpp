/* P2-HG3 rung-6 "arrival at the trial screen" — card BRINGUP-5 golden
 * (legs HG3-275..281). Spec docs/p2-rung6-seams-1.md §1 S8-S15 + §3 Card
 * BRINGUP-5; grades per docs/p2-bringup5-roster-1.md; owner default O1 per
 * docs/p2-memcard-1.md §3 (HG3-281 = NULL-branch leg ONLY — no card shim, no
 * SystemSaveData supply; the payload pointer stays era-zero NULL at every
 * walk; the single non-NULL probe is an in-process, immediately-restored
 * branch-discipline canary, never a boot state).
 *
 * Builds ON TOP of the BRINGUP-4 walk (hg3_walk 26/26 untouched — this is a
 * separate binary; the dispatcher/exit-table S18 machinery and the 22-row
 * GENERATED main_phase_table are LINKED, never re-minted). The row-10 swap to
 * REAL slots is the era-mutable-.data harness precedent (HG3-271g/R2: "the
 * HG3 test swaps stand-ins between runs"; docs/p2-hg3-1.md §2).
 * Loud runs execute in forked children over MAP_SHARED witness records
 * (L7-P12/L8 discipline — counters survive the exit(9)).
 */
#include "ladder9_trial_prelude.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

extern "C" void DVDInit(void);

static int failures, total;
#define CHECK(cond, ...) do { total++; if (!(cond)) { failures++; \
    printf("FAIL \x1b[31m"); printf(__VA_ARGS__); printf("\x1b[0m\n"); } \
    else { printf("PASS \x1b[32m"); printf(__VA_ARGS__); printf("\x1b[0m\n"); } } while (0)

#define SNAP_MAX 24
struct Shm {
    int magic;
    int log_n;  const char *log[256];           /* g9 witness vector (tags are static literals — same binary, pointers valid across fork) */
    int ld[30], rl[30];
    int cam_ctor, cam_mode, particle_heap, light_n, quad, winmain, sin_arg, cos_arg;
    float dir_x, dir_y, dir_z;
    int multi, synced, change;                  /* era scene stores at the last flush */
    int present, rtick;
    int probe_slot, scroll, qdestroy, create_total, bgm;
    int loud_fires, audio_loud, shutdown_fires, trial_exit;
    int snap_n; int snaps[SNAP_MAX][4];
    int fin[4];
    int change_state_calls, change_state_last, tail6, pend_abort_legs[3];
};
static struct Shm *shm;
static int snap_n; static int snaps[SNAP_MAX][4];

static void flush(void) {
    if (!shm) return;
    shm->magic = 0x48473305;
    shm->log_n = g9_log_n; memcpy(shm->log, g9_log, sizeof g9_log);
    memcpy(shm->ld, g9_ld, sizeof g9_ld); memcpy(shm->rl, g9_rl, sizeof g9_rl);
    shm->cam_ctor = g9_cam_ctor; shm->cam_mode = g9_cam_mode;
    shm->particle_heap = g9_particle_heap; shm->light_n = g9_light_n;
    shm->quad = g9_quad_sprites; shm->winmain = g9_winmain_ctor;
    shm->sin_arg = g9_sin_arg; shm->cos_arg = g9_cos_arg;
    shm->dir_x = scene_light_state.direction.x; shm->dir_y = scene_light_state.direction.y; shm->dir_z = scene_light_state.direction.z;
    shm->multi = multi_chg_game_mode_state; shm->synced = chg_game_mode_state_synced; shm->change = game_mode_scene_change_state;
    shm->present = g_present_calls; shm->rtick = g_render_tick_calls;
    shm->probe_slot = g9_probe_slot; shm->scroll = g9_scroll_ctor;
    shm->qdestroy = g9_queued_destroy_calls; shm->bgm = g9_bgm_calls;
    shm->loud_fires = g9_fires; shm->audio_loud = g9_audio_loud_fires;
    shm->shutdown_fires = g_shutdown_loud_fires; shm->trial_exit = g8_trial_exit_calls;
    shm->change_state_calls = g9_change_state_calls; shm->change_state_last = g9_change_state_last;
    shm->tail6 = g9_tail_calls[0] + g9_tail_calls[1] + g9_tail_calls[2] + g9_tail_calls[3] + g9_tail_calls[4] + g9_tail_calls[5];
    shm->fin[0] = main_phase_state.cur; shm->fin[1] = main_phase_state.prev;
    shm->fin[2] = main_phase_state.next; shm->fin[3] = main_phase_state.result;
    shm->snap_n = snap_n; memcpy(shm->snaps, snaps, sizeof snaps);
}
void p8_snapshot_loop_top(void) {
    if (snap_n < SNAP_MAX) { snaps[snap_n][0] = main_phase_state.cur;
        snaps[snap_n][1] = main_phase_state.prev; snaps[snap_n][2] = main_phase_state.next;
        snaps[snap_n][3] = main_phase_state.result; }
    snap_n++;
    flush();
}
static int g_walking;                           /* hook only active during the forked golden canary runs */
static int fx;
static void hg5_fire(void) {                    /* rides the counted WALK_MAIN seam (L8-6 regime — ladder3's state machine stays its own proof) */
    if (!g_walking) return;
    if (main_phase_state.cur == 1) { g9_note("fire-0xA"); request_main_phase(0xA); return; }
    if (main_phase_state.cur == 0xA && game_mode_scene_instance != NULL && ++fx >= 3) {
        g9_note("x:decline-hook");
        static_cast<TAdTrialAgreementGC *>(game_mode_scene_instance)->on_decline();   /* REAL slice :39 */
    }
}
static L7WalkHook saved_hook;
static s32 no_stop(void) { return 0; }
s32 pso_walk_stop_requested(void) { flush(); return 0; }

static int child_walk(void) {
    pso_main_dispatcher();
    flush();
    return 0;
}

static int count_tag2(const char *const *log, int from, int to, const char *tag) {
    int c = 0; for (int i = from; i < to; i++) if (!strcmp(log[i], tag)) c++; return c;
}
/* expected S10 init call-order vector (era body :452-491 read at the pin):
 * 6 pre-walk legs, 22 GMS load halves (row 11 pair = REAL era-empty slices,
 * no counter), 7 lobby load halves, kbd-clear, part-heap, lights, 4 table
 * lookups (sin/cos/sin/cos), cam-ctor, cam-mode, quad, winmain, soundbank. */
static const char *INIT_PRE[6] = { "i:screen-size", "i:clear-color", "i:reset-ctrl", "i:clear-resload", "i:kbd-reset", "i:kbd-mode" };
static const char *INIT_LD[29] = {
    "ld:cam.reset_view_state", "ld:cam.init_default_view", "ld:kbd.update_menu",
    "ld:tex.f256_ad_pic_2", "ld:tex.f256_backtext", "ld:tex.f128_ray",
    "ld:chrsel.multi_load", "ld:timeset.load_scene", "ld:tex.f512_multiplay",
    "ld:chrsel.alloc_char_bufs",                                   /* GMS row 11 REAL (no tag) sits between 9 and 11 */
    "ld:multi.load_confirm_tex", "ld:tex.logo_e", "ld:tex.sonic", "ld:chat.init_heap",
    "ld:prate.register", "ld:snapshot.load_buffers", "ld:kbd.reset_b",
    "ld:win.reset_windows", "ld:lobby.set_ending_cb", "ld:kbd.load_resources",
    "ld:quest.alloc_buffer", "ld:drawline.load",
    "ld:lobby.init_title_scene", "ld:tex.midashi", "ld:tex.player_tex", "ld:tex.select_gamen",
    "ld:tex.select_cursor", "ld:tex.chr_sel_slot", "ld:lobby.update_cursor",
};
static const char *INIT_POST[9] = { "i:kbd-clear", "i:part-heap", "i:lights", "i:sin", "i:cos", "i:sin", "i:cos", "i:cam-ctor", "i:cam-mode" };
static const char *INIT_TAIL[2] = { "i:quad", "i:winmain" };

int main() {
    setvbuf(stdout, 0, _IOLBF, 0);
    DVDInit();
    shm = (struct Shm *)mmap(NULL, sizeof *shm, PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    CHECK(shm != MAP_FAILED, "HG5-H0 harness: MAP_SHARED witness region live (forked loud runs survive exit(9))");
    g8_flush_hook = flush;

    /* ================= HG3-277a era data defaults (verbatim lines) ============ */
    CHECK(multi_chg_game_mode_state == 29 && chg_game_mode_state_synced == 29
          && game_mode_scene_change_state == 0 && game_mode_scene_instance == NULL,
          "HG3.277a era scene-store defaults VERBATIM (:88-94): mode 29 / synced 29 / change 0 / instance NULL — the 29->0x17 walk starts from era truth");
    CHECK(card_load_sequence_state == 0 && system_save_payload == NULL,
          "HG3.277b/R-C+O1 pinned era defaults at the pin: card_load_sequence_state==0 (un-init'd .bss s32 @801FBF54:66 -> get_instance()==0 gate OPENS) and system_save_payload==NULL (boot-init not run — MEMSCAN §1; no supply per owner default O1)");
    CHECK(strcmp(pso_system_card_filename, "PSO_SYSTEM") == 0,
          "HG3.277c pso_system_card_filename era data line verbatim (\"PSO_SYSTEM\", :85)");

    /* ================= HG3-277 S11: 3-store opens + one-behind law =========== */
    TAdMultiChgGameModeGC::open_trial_agreement_scene();
    CHECK(chg_game_mode_state_synced == 29 && multi_chg_game_mode_state == 0x17
          && game_mode_scene_change_state == 1,
          "HG3.277a/S11 open_trial_agreement_scene REAL (:214-218): sync copies 29 FIRST, then 0x17 store, then change=1 — the one-behind law (:547) read live, WHAT-GREEN shape 29->0x17");
    TAdMultiChgGameModeGC::open_time_set_scene();
    CHECK(chg_game_mode_state_synced == 0x17 && multi_chg_game_mode_state == 8
          && game_mode_scene_change_state == 1,
          "HG3.277b/S11 open_time_set_scene REAL (:283-287): synced catches 0x17 one-behind, mode 8, change 1 — the SAME 3-store shape");
    TAdMultiChgGameModeGC::open_title_scene();
    CHECK(chg_game_mode_state_synced == 8 && multi_chg_game_mode_state == 0
          && game_mode_scene_change_state == 1,
          "HG3.277c/S11 open_title_scene REAL (:313-317): synced 8, mode 0 (the mode-0 store exists; O1 keeps title UNREACHED — this is a store-level call, not a walk)");
    TAdMultiChgGameModeGC::sync_mode_state();
    CHECK(chg_game_mode_state_synced == multi_chg_game_mode_state, "HG3.277d sync_mode_state REAL one-liner (:547): synced := mode");
    multi_chg_game_mode_state = 29; chg_game_mode_state_synced = 29; game_mode_scene_change_state = 0;   /* harness restore for the legs below (era .data mutable) */

    /* ================= HG3-276 S10: direct scene-init (audio armed) ========== */
    g9_audio_arm = 1;                            /* L9-2: golden regime arms the counted witness; LB-3 remains the DEFAULT (canary T-J un-armed) */
    int log0 = g9_log_n;
    game_mode_scene_instance = (TObject *)0xDEAD;   /* poison: the verbatim :490 store must overwrite */
    init_game_mode_select_scene();
    int log1 = g9_log_n;
    {   /* call-order == era body order across the WHOLE span */
        int p = log0, ok = 1;
        for (int i = 0; i < 6 && ok; i++) ok = p < log1 && !strcmp(g9_log[p++], INIT_PRE[i]);
        for (int i = 0; i < 29 && ok; i++) ok = p < log1 && !strcmp(g9_log[p++], INIT_LD[i]);
        for (int i = 0; i < 9 && ok; i++) ok = p < log1 && !strcmp(g9_log[p++], INIT_POST[i]);
        /* pose leg rides the rung-5 captured seam (set_active_camera_pose, ladder7_seam.cpp:120) between cam-mode and quad */
        for (int i = 0; i < 2 && ok; i++) ok = p < log1 && !strcmp(g9_log[p++], INIT_TAIL[i]);
        ok = ok && (p + 1 == log1) && !strcmp(g9_log[p], "i:soundbank");
        CHECK(ok, "HG3.276a/S10 init_game_mode_select_scene VERBATIM call order == era body (:452-491): 6 pre-legs -> S8 forward walk over 23+7 pairs -> 9 post-legs incl. the 4 angle-table lookups -> soundbank tail (pose leg rides the rung-5 captured seam)");
    }
    {   int loads = 0; for (int i = 0; i < 30; i++) loads += g9_ld[i];
        CHECK(loads == 29 && g9_ld[10] == 0,
              "HG3.276b/S8 run_load_callback_table REAL walker: 29 counter-legs fired (46-half table -> LOAD halves only), GMS row 11 pair carries NO counter because its slot is the REAL era-empty verbatim slice — self-contained init witness (ROSTER §8)");
        int rel = 0; for (int i = 0; i < 30; i++) rel += g9_rl[i];
        CHECK(rel == 0, "HG3.276c ZERO release halves called by the forward walker (S17/HG3-283 owns the reverse ride — pair parity law L8-4 holds on the generated 23+7 tables)");
        int rl_all0 = 1; for (int i = 0; i < 30; i++) if (g9_rl[i]) rl_all0 = 0;
        CHECK(rl_all0 && g9_ld[12] == 1 && g9_ld[13] == 1, "HG3.276d texture-pair load slots fired as bringup counters citing the gx seat (L8-5 regime; register_texture_logo_e/load_sonic cited at the generated table, never re-minted)");
    }
    CHECK(g9_cam_ctor == 1 && g9_cam_parent_is_tl_camera && g9_cam_mode == 9
          && g_pose_a.x == 0.0f && g_pose_a.y == 0.0f && g_pose_a.z == 0.0f
          && g_pose_b.x == 0.0f && g_pose_b.y == 0.0f && g_pose_b.z == -100.0f,
          "HG3.276e camera legs: new TCameraCtrl(tl_camera) lane-J counted (d) + select_active_controller(9) + the era {0,0,0}->{0,0,-100} pose captured through the rung-5 seam (statics :99-100 verbatim)");
    CHECK(g9_particle_heap == 0x100000 && g9_quad_sprites == 1 && g9_winmain_ctor == 1
          && game_mode_scene_instance == NULL,
          "HG3.276f particle heap 0x100000 + quad sprites + TWindowCtrlMain counted (allocator/window seams class (c), ROSTER grades); :490 game_mode_scene_instance = 0 store executed over the poisoned slot");
    CHECK(g9_light_n == 4 && g9_sin_arg == 0x9000 && g9_cos_arg == 0x9000
          && scene_light_state.direction.y == -0.25f && scene_light_state.direction.x == 0.1875f
          && scene_light_state.direction.z == 0.5625f,
          "HG3.276g menu-light pose: init_menu_scene_lights(&cfg,4) + the :477-480 expression chain read live (lookup ARGS 0x1555/0x9000 are era truth; the returned table VALUES are deterministic harness constants — math lane supplies the real tables later; the store ORDER and arithmetic shape are the era body's)");
    {   /* S10/S8 abort-branch canary: pending==1 short-circuits the walker + init (era :461-466/:468-473 REAL branches) */
        g9_area_pending = 1;
        int l0 = g9_log_n, loads0 = g9_ld[14]; log0 = g9_log_n; (void)loads0;
        init_game_mode_select_scene();
        CHECK(g9_ld[14] == loads0 && count_tag2(g9_log, l0, g9_log_n, "i:kbd-invoke") == 1
              && g9_particle_heap == 0x100000 && g9_audio_loud_fires == 0,
              "HG3.276h canary (pending==1): the S8 walker ABORTS on its first is_area_transfer_pending check (:266-268) and init takes the era invoke/clear/return arms — no load halves, no scene build, no sound-bank reach (canary can fail: pending==0 golden run above passed the same lines through)");
        g9_area_pending = 0; (void)log0;
    }

    /* ================= HG3-279 S13: create_mode_scene 3 in-span cases ======== */
    game_mode_scene_instance = NULL;
    multi_chg_game_mode_state = 0; TAdMultiChgGameModeGC::create_mode_scene();
    int title1 = g9_title_ctor;
    CHECK(title1 == 1 && g9_title_parent_is_tl_01 && game_mode_scene_instance != NULL,
          "HG3.279a case 0 -> new TTitleScene(tl_01) VERBATIM arm (class-(d) shell ctor counted — S16/HG3-282 owns the real ctor; O1 never walks title)");
    multi_chg_game_mode_state = 8; TAdMultiChgGameModeGC::create_mode_scene();
    CHECK(g9_timeset_ctor == 1 && g9_timeset_parent_is_tl_01 && game_mode_scene_instance != NULL,
          "HG3.279b case 8 -> (TObject *)new TAdTimeSetGC(tl_01) VERBATIM arm (screen chain (c) shell; the O1 rest-point scene)");
    int probe0 = g9_probe_slot, scroll0 = g9_scroll_ctor, qd0 = g9_queued_destroy_calls, bgm0 = g9_bgm_calls;
    multi_chg_game_mode_state = 23; TAdMultiChgGameModeGC::create_mode_scene();   /* case 0x17 = 23 */
    TAdTrialAgreementGC *trial = static_cast<TAdTrialAgreementGC *>(game_mode_scene_instance);
    CHECK(g9_probe_slot == probe0 + 1 && g9_scroll_ctor == scroll0 + 1 && g9_scroll_args[0] == 1 && g9_scroll_args[1] == 0,
          "HG3.279c case 23 -> new TAdTrialAgreementGC(tl_01) REAL ctor VERBATIM: check_card_file executed (probe_card_slot==0 -> the no-card SHOWS-the-screen branch, MEMSCAN §2), scroll_window shell ctor args (1,0) — the window chain itself NOT built (card S13)");
    CHECK(trial->m_name == TAdTrialAgreementGC::name
          && strcmp(trial->m_name, "TAdTrialAgreementGC") == 0 && trial->scroll_window != NULL,
          "HG3.279d trial ctor = the m_name pin: instance->m_name == TAdTrialAgreementGC::name (era data slice :52) — shell TObject pinned, agree branch NOT taken (probe zero)");
    CHECK(g9_probe_and_check == 0 && g9_check_status == 0 && g9_find_file == 0 && g9_poll_memcard == 0,
          "HG3.279e check_card_file returns at the FIRST probe (:58-60): the deeper memcard chain (probe_and_check/wait_for_load/find) is provably UNREACHED with no card — the era first-boot shape, not a stubbed branch");
    CHECK(g9_queued_destroy_calls == qd0 && g9_bgm_calls == bgm0,
          "HG3.279f BGM skip-set law + ctor no-destroy: creates at {0,8,23} fired apply_bgm_col_ctrl ZERO times (all three live in the era :321-330 skip case set) and the REAL ctor never queued destruction (agree auto-advance branch stayed closed)");
    game_mode_scene_instance = NULL;
    multi_chg_game_mode_state = 29; bgm0 = g9_bgm_calls;
    TAdMultiChgGameModeGC::create_mode_scene();
    CHECK(g9_bgm_calls == bgm0 + 1 && game_mode_scene_instance == NULL,
          "HG3.279g default-mode discipline: mode 29 takes the BGM default arm (apply_bgm_col_ctrl(\"advr.adx\") counted, boot-spine §4 audio-dark) and the era case-29 NULL arm (:421-422 verbatim) creates NOTHING — the initial mode cannot mint a scene");
    {   /* L9-1 loud canary: an UNLANDED scene arm trips LB-3 */
        int fires0 = g9_fires;
        pid_t kid = fork();
        if (kid == 0) { multi_chg_game_mode_state = 6; TAdMultiChgGameModeGC::create_mode_scene(); _exit(0); }
        int st = 0; while (waitpid(kid, &st, 0) < 0) { }
        CHECK(WIFEXITED(st) && WEXITSTATUS(st) == 9 && shm->loud_fires == fires0 + 1
              && shm->fin[0] == main_phase_state.cur,   /* parent state untouched by the child (COW) */
              "HG3.279h canary TRIPPED x1: mode 6 (TAdSinglePlyChrSelectGC — (c) deny-roots family) launched p9_fail_loud_mode_scene — the 25 unlanded arms are LOUD, spec §3 canary discipline (L9-1); era arm text kept at the generated site");
    }

    /* ================= HG3-280 S14: on_agree / on_decline store fires ======== */
    multi_chg_game_mode_state = 0x17; chg_game_mode_state_synced = 29; game_mode_scene_change_state = 0;  /* the arrival state (trial screen live) */
    trial->on_agree();                                   /* REAL one-liner :40 */
    CHECK(chg_game_mode_state_synced == 0x17 && multi_chg_game_mode_state == 8
          && game_mode_scene_change_state == 1,
          "HG3.280a on_agree fire == the MODE-8 store (open_time_set_scene, HG3 §1 S14 'stop at store-level dispatch'): synced catches 0x17 one-behind — the agreement advance is proven WITHOUT the scroll render surface");
    reset_main_phase_state();
    trial->on_decline();                                 /* REAL one-liner :39 */
    CHECK(main_phase_state.next == 0 && main_phase_state.result == 1,
          "HG3.280b on_decline -> request_main_phase(0) (REAL rung-5 slice): {next,result} == (0,1) — the honest-quit request, era cast (saddr)((void*)0) == 0 read back");

    /* ================= HG3-278 S12: change-state 1->2->0 walk ================ */
    reset_main_phase_state();
    /* the era dispatcher clears {result,next} at its loop-top (:371/:372-375 seed);
     * a DIRECT frame call bypasses that, so clear T-F's decline residue here —
     * era-identical values, harness placement only (L8-2-family disclosure). */
    main_phase_state.result = 0; main_phase_state.next = main_phase_state.cur;
    multi_chg_game_mode_state = 23;              /* the create-arm must re-mint the TRIAL scene */
    game_mode_scene_instance = trial;
    game_mode_scene_change_state = 1;
    int qd1 = g9_queued_destroy_calls, scroll1 = g9_scroll_ctor, probe1 = g9_probe_slot;
    int pres0 = g_present_calls, rt0 = g_render_tick_calls, sls0 = g9_runsls_calls, noop0 = g9_noop_calls;
    run_game_mode_select_frame();                        /* case 1: destroy */
    CHECK(g9_queued_destroy_calls == qd1 + 1 && game_mode_scene_instance == NULL
          && game_mode_scene_change_state == 2,
          "HG3.278a/S12 change-state 1: REAL wrapper destroys the live instance (queue_destruction counted deferral — allocator walk is the pool lane's) and lands on state 2");
    run_game_mode_select_frame();                        /* case 2: create */
    CHECK(game_mode_scene_change_state == 0 && g9_scroll_ctor == scroll1 + 1
          && g9_probe_slot == probe1 + 1
          && game_mode_scene_instance != NULL && game_mode_scene_instance != trial
          && static_cast<TAdTrialAgreementGC *>(game_mode_scene_instance)->m_name == TAdTrialAgreementGC::name,
          "HG3.278b/S12 change-state 2: create_mode_scene REAL (mode 0x17) re-mints the trial scene EXACTLY ONCE (new REAL ctor pass: +1 probe, +1 scroll shell, m_name pinned), state -> 0 — destroy->create exactly once per change-state (WHAT-GREEN)");
    game_mode_scene_change_state = 29;                   /* out-of-range: era switch falls through */
    run_game_mode_select_frame();
    CHECK(game_mode_scene_change_state == 29 && g9_queued_destroy_calls == qd1 + 1 && g9_scroll_ctor == scroll1 + 1,
          "HG3.278c change-state 0 / out-of-range: no era case fires — the wrapper body (:495-509) is a pure three-case machine (state 29 restored as the no-op probe)");
    game_mode_scene_change_state = 0;
    run_game_mode_select_frame();
    printf("    [278d diag: present %d->%d tick %d->%d sls %d->%d noop %d->%d result %d next %d]\n",
           pres0, g_present_calls, rt0, g_render_tick_calls, sls0, g9_runsls_calls, noop0, g9_noop_calls,
           main_phase_state.result, main_phase_state.next);
    CHECK(g_present_calls == pres0 + 4 && g_render_tick_calls == rt0 + 4
          && g9_runsls_calls == sls0 + 4 && g9_noop_calls == noop0 + 4 && main_phase_state.result == 0,
          "HG3.278d frame spine: 4 update frames rode begin_gx/run_save_load_sequence/noop counters + render_tick REAL with present still EXACTLY 1/frame (4 new ticks -> 4 new presents, WHAT-GREEN law); get_instance()==0 gate opened the fallback/is_ready legs and BOTH stayed quiet at era-zero (result==0, no request_main_phase) — R-C verified: gate opens, era-zero conditions hold");
    game_mode_scene_instance = NULL;
    multi_chg_game_mode_state = 29; chg_game_mode_state_synced = 29; game_mode_scene_change_state = 0;

    /* ================= HG3-281 S15: apply_settings NULL-branch (O1) ========== */
    TAdTimeSetGC timeset(tl_01);
    int cs0 = g9_change_state_calls;
    timeset.apply_settings();                            /* REAL slice :885-899 */
    CHECK(g9_change_state_calls == cs0 + 1 && g9_change_state_last == 1
          && shm && system_save_payload == NULL && multi_chg_game_mode_state == 29,
          "HG3.281a/S15 O1 landing: get_global_state REAL accessor reads the era-zero NULL payload -> apply_settings takes the :888-891 change_state(1) EARLY-RETURN and returns — the mode stores NEVER move (title remains unreachable era-truth: no card shim, no SystemSaveData supply)");
    int t0 = 0; for (int i = 0; i < 6; i++) t0 += g9_tail_calls[i];
    CHECK(t0 == 0 && g9_bgm_calls,   /* bgm fired earlier (279g) — just proves the counter is live, tails are separate */
          "HG3.281b the non-NULL tail (set_language/gains/init_sound_mode/unload+load lobby/open_title) is provably NOT executed on the NULL leg: all six tail counters stay 0 across every golden run so far");
    {   /* branch-discipline canary (in-process, immediately restored — NOT a
         * supply: no boot state, no file, payload back to NULL on the next line). */
        static SystemSaveData probe_block; memset(&probe_block, 0, sizeof probe_block);
        reset_main_phase_state();
        int ld24b = g9_ld[24], rl23b = g9_rl[23];
        system_save_payload = &probe_block;
        timeset.apply_settings();
        int good = g9_change_state_calls == cs0 + 1       /* NO second change_state — NULL branch not taken */
            && g9_tail_calls[0] == 1 && g9_tail_calls[3] == 1
            && g9_ld[24] == ld24b + 1 && g9_rl[23] == rl23b + 1
            && chg_game_mode_state_synced == 29 && multi_chg_game_mode_state == 0;
        system_save_payload = NULL;
        multi_chg_game_mode_state = 29; chg_game_mode_state_synced = 29; game_mode_scene_change_state = 0;
        CHECK(good, "HG3.281c canary-can-fail: a transient non-NULL payload (restored the very next statement) flips apply_settings to the era tail — gains/sound-mode counted, unload+load lobby walk the REAL reverse/forward walkers over the generated 7-pair table, open_title_scene REAL store fires (synced 29 one-behind, mode 0). Under O1 this branch NEVER executes at boot — the payload stays NULL; the canary only proves the :888 branch is load-bearing, not decorative");
    }
    reset_main_phase_state();
    timeset.restart_main_phase();                        /* REAL slice :901-903 */
    CHECK(main_phase_state.next == 0 && main_phase_state.result == 1,
          "HG3.281d restart_main_phase REAL (:901-903): request_main_phase(0) — the time-set quit arm shares the decline path's era behavior");

    /* ================= HG3-275 S9: end-to-end walk 1->0xA->TRIAL->decline->quit
     * forked: table[0xA].init/update swap to the REAL slices (era .data,
     * HG3-271g precedent); row-10 exit keeps the counting witness (S17/HG3-283
     * owns the release body — swap exactly as BRINGUP-4 R2 did). ============== */
    main_phase_table[10].init = TAdMultiChgGameModeGC::init_trial_agreement_phase;  /* REAL S9 slice */
    main_phase_table[10].update = run_game_mode_select_frame;                        /* REAL S12 slice */
    main_phase_table[10].exit = p8_counting_exit_trial;                              /* S17 witness (BRINGUP-4 R2 regime) */
    reset_main_phase_state();
    g8_init_skip_reset = 0;
    fx = 0; g_walking = 1; saved_hook = l7_walk_hook;
    l7_walk_hook = hg5_fire;
    int snap0 = snap_n, logb = g9_log_n, sc_b = g9_scroll_ctor, ps_b = g9_probe_slot,
        ld12_b = g9_ld[12], te_b = g8_trial_exit_calls, sd_b = g_shutdown_loud_fires,
        pd_b = g_present_calls, rt_b = g_render_tick_calls;
    pid_t kid = fork();
    if (kid == 0) _exit(child_walk());
    int st = 0; while (waitpid(kid, &st, 0) < 0) { }
    g_walking = 0; l7_walk_hook = saved_hook;
    {   const char *const *L = shm->log; int n = shm->log_n;
        int i_fire = -1, i_init = -1, i_sound = -1, i_scroll = -1;
        for (int i = logb; i < n; i++) {
            if (!strcmp(L[i], "fire-0xA") && i_fire < 0) i_fire = i;
            if (!strcmp(L[i], "i:screen-size") && i_init < 0) i_init = i;
            if (!strcmp(L[i], "i:soundbank")) i_sound = i;
            if (!strcmp(L[i], "x:scroll-shell")) i_scroll = i;
        }
        CHECK(i_fire >= 0 && i_init > i_fire && i_sound > i_init && i_scroll > i_sound,
              "HG3.275a/S9 composition order THROUGH THE REAL DISPATCHER: state-7 hook fire(0xA) -> table[0xA].init entered -> init_game_mode_select_scene legs (..soundbank tail) -> open_trial_agreement_scene -> update frames mint the trial scene (scroll shell after the init tail) — the one-line era composition (:541 verbatim) executed end-to-end cur 1->0xA");
    }
    CHECK(shm->multi == 0x17 && shm->synced == 29 && shm->change == 0
          && shm->scroll == sc_b + 1 && shm->probe_slot == ps_b + 1 && shm->ld[12] == ld12_b + 1,
          "HG3.275b WHAT-GREEN stores: multi 29->0x17 with synced==29 one-behind AT ARRIVAL, change-state machine completed 1->2->0 (scene created EXACTLY once: +1 scroll shell, +1 probe, +1 full walker pass); decline fired after the walk's frames");
    CHECK(WIFEXITED(st) && WEXITSTATUS(st) == 9 && shm->shutdown_fires == sd_b + 1
          && shm->trial_exit == te_b + 1,
          "HG3.280c the honest quit, end-to-end: on_decline REAL -> dispatcher switch(1) prev=0xA cur=0 -> loop-top exit -> row-10 exit WITNESS (S17 owns the release body) -> shutdown_and_reset_system LB-3 exit(9) exactly once — the decline path IS the era quit path (HG2.3 canary pattern)");
    CHECK(shm->fin[0] == 0 && shm->fin[1] == 0xA && shm->fin[2] == 0 && shm->fin[3] == 1,
          "HG3.275c final flushed dispatcher state at the loud tail: {cur,prev,next,result}==(0,0xA,0,1) — prev=0xA proves the REAL :389-390 arm carried the walk OUT of phase 10 (rung-6 exit-table grammar); no second 0xA entry anywhere in the snaps");
    CHECK(shm->present - pd_b == shm->rtick - rt_b && shm->rtick - rt_b >= 5 && g9_audio_loud_fires == 0,
          "HG3.275d frame law held across the whole span (logo frames + the init composition + update frames): presents == render_ticks EXACTLY delta-for-delta (single-present law R9), >=5 new ticks through the walk, audio-dark seam stayed COUNTED (arm==1, zero loud fires in this child)");
    main_phase_table[10].exit = p8_fail_loud_main_phase;    /* restore generated stand-ins for the canary below */

    /* ================= HG3-276i audio-dark canary: UN-ARMED walk ============= */
    reset_main_phase_state();
    fx = 0; g_walking = 1; g9_audio_arm = 0;             /* era-honest default: LB-3 */
    saved_hook = l7_walk_hook; l7_walk_hook = hg5_fire;
    int al_b = g9_audio_loud_fires, sc_b2 = g9_scroll_ctor;
    kid = fork();
    if (kid == 0) { multi_chg_game_mode_state = 29; chg_game_mode_state_synced = 29; game_mode_scene_change_state = 0; _exit(child_walk()); }
    while (waitpid(kid, &st, 0) < 0) { }
    g_walking = 0; l7_walk_hook = saved_hook; g9_audio_arm = 1;
    CHECK(WIFEXITED(st) && WEXITSTATUS(st) == 9 && shm->audio_loud == al_b + 1
          && shm->multi == 29 && shm->synced == 29 && shm->scroll == sc_b2,
          "HG3.276i canary TRIPPED x1 (L9-2): un-armed, the REAL dispatcher walk dies LOUD at load_floor_sound_bank inside table[0xA].init — multi/synced stay 29, the trial scene is never minted (child's walker delta 0): audio-dark is honestly blocked, not silently skipped");

    /* ---- harness closure ---- */
    main_phase_table[10].init = p8_enter_trial_init;      /* restore the generated stand-ins (HG3.272c shape) */
    main_phase_table[10].update = p8_fail_loud_main_phase;
    g9_audio_arm = 0;
    CHECK(main_phase_table[10].init == p8_enter_trial_init && main_phase_table[10].update == p8_fail_loud_main_phase
          && g9_audio_arm == 0 && system_save_payload == NULL,
          "HG5-H9 harness closure: row-10 slots restored to the GENERATED stand-ins, audio arm back to the era-honest LB-3 default, payload NULL (O1 stance intact at exit)");

    printf("\n== P2-HG3 rung-6 arrival at the trial screen (HG3-275..281) ==\n");
    printf("  S8/S9/S10/S11/S12/S13(4 arms)/S14/S15 + R-C + O1: dispatcher walks 1->0xA into the REAL init_trial_agreement_phase composition; the 23+7 pair tables GENERATED (two-source byte-validated, 62-callee census); unlanded scene arms + audio-dark trip LB-3 canaries; decline -> request_main_phase(0) -> loop exit -> honest-quit loud\n");
    printf("== P2-HG3 hg3_trial: %d/%d PASS ==\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
