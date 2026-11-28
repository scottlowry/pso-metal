/* P2-HG3 rung-6 "arrival at the trial screen" — BRINGUP-5 prelude (HG3-275..281).
 *
 * Prepended (via include) to the GENERATED slice TU host/bringup/gen_ladder9_trial.py
 * -> build/bringup/gen/ladder9_trial_glue.cpp. Carries every external the verbatim
 * bodies touch that the rung-5/6 prelude does not already declare, in the L7-1
 * minimal-carrier grammar: class shapes reduced to the members the sliced bodies
 * actually touch, each with its era header cite (verified same-turn at the pin
 * 6b38b53837 — docs/p2-bringup5-roster-1.md grades govern every counted slot).
 *
 * Host status legend per the roster: REAL = verbatim era body in the glue TU;
 * COUNTED = records, never fakes era behavior (hg3_trial_seam.cpp); LOUD = LB-3
 * fail-loud canary; gx-landed = cited, counter-backed bringup-side (L8-5 regime).
 */
#ifndef LADDER9_TRIAL_PRELUDE_H
#define LADDER9_TRIAL_PRELUDE_H

#include "ladder7_run_prelude.h"

/* ---- callback-table carrier (era include/pso/map/FloorResourceCallbacks.h:6-9
 * verbatim shape {load, unload} = 2 function pointers). Host stride 16B keeps the
 * L8-4 parity law EXACT: run_load_callback_table (LoadFrameEntry {fn,id}) reads
 * offset 0 (LOAD half fires forward); TLobby::call_funcs_reverse (LobbyFrameEntry
 * {id,fn}) reads offset 8 (RELEASE half fires back-to-front); n = sizeof/16 = the
 * era row counts 23/7 both walkers, both sides. ---- */
struct FloorResourceCallbacks { void (*load)(void); void (*unload)(void); };
extern FloorResourceCallbacks game_mode_select_scene_callbacks[23];      /* GENERATED, row-count census canary */
extern FloorResourceCallbacks lobby_scene_release_callback_table[7];     /* GENERATED */
/* ---- S8 walker + S15 tail riders (defs: REAL verbatim slices in the glue TU): ---- */
s32 run_load_callback_table(void *table, u32 size);
void unload_lobby_scene_resources(void);
void load_lobby_scene_resources(void);
/* ---- S9/S10/S12 unit-level legs (REAL verbatim slices in the glue TU): ---- */
void init_game_mode_select_scene(void);
void run_game_mode_select_frame(void);
/* ---- the four era scene stores (VERBATIM data lines in the glue TU; test reads/writes them) ---- */
extern s32 multi_chg_game_mode_state;
extern int chg_game_mode_state_synced;
extern s32 game_mode_scene_change_state;
extern TObject *game_mode_scene_instance;

/* ---- light carrier (init_game_mode_select_scene touches ONLY .direction.x/y/z;
 * era TObjLight_80133568.h:56-61: TObjLightSceneLightState : LightEntry, the
 * direction Vec is the era member read; default_scene_light_config passes by
 * address ONLY). Angle lookup decls: include/pso/math/AngleLookup.h:18-19. ---- */
struct LightEntry { Vec direction; };
extern LightEntry default_scene_light_config, scene_light_state;
void init_menu_scene_lights(LightEntry *cfg, s32 n);       /* COUNTED (lights lane, roster (c)) */
f32 sin_table_lookup(s32 angle);                            /* COUNTED; args asserted (0x1555/0x9000) */
f32 cos_table_lookup(s32 angle);

/* ---- S10 prelude externs (all COUNTED shells, defs hg3_trial_seam.cpp): ---- */
void set_screen_size(f32 width, f32 height);                /* render/GinjaView.cpp:69 */
void set_copy_clear_color_argb(u32 argb, s32 a, s32 b);     /* effect/particle/EffectRender.cpp:14 */
void reset_controller_input_state(void);                    /* system/ControllerInput.h:18 */
void clear_resource_load_tasks(void);                       /* TLobby_800F8AA8.cpp:1328 */
void apply_phase_keyboard_mode(void);                       /* TLobby_800F8AA8.cpp:834 */
s32 is_area_transfer_pending(void);                         /* TLobby_800F8AA8.h:37 — era-zero host default (rung-5 pending==0 guard precedent) */
void load_floor_sound_bank(s32 floor);                      /* audio/SoundPlayback.h:22 — L9-2: LB-3 audio-dark by DEFAULT; the golden walk arms a counted witness; canary runs un-armed */
void play_area_music(char *name);                           /* audio lane (apply_bgm_col_ctrl tail; not sliced — never selected in-span) */

/* ---- scene carriers (L7-1 grammar; members = exactly what the sliced bodies
 * touch; era cites per docs/p2-bringup5-roster-1.md) ---- */
struct TAdMultiChgGameModeGC {
    static void init_trial_agreement_phase();   /* REAL slice S9 :541 */
    static void open_trial_agreement_scene();   /* REAL slices S11 :214/:283/:313 */
    static void open_time_set_scene();
    static void open_title_scene();
    static void sync_mode_state();              /* REAL slice :547 (one-behind law) */
    static void create_mode_scene();            /* REAL slice S13 (L9-1 loud arms) */
    static void load_scene_resources();         /* REAL era-empty slice (a)(a) pair — the self-contained init witness */
    static void free_scene_resources();
    static void apply_bgm_col_ctrl(const char *name);  /* COUNTED :194-198 (audio-dark; skip-set law asserts it never fires for {0,8,23}) */
};
struct TAdScrollWinGC {                          /* window chain = (c)/(d) font lane — NOT built (card S13); shell ctor counts */
    TAdScrollWinGC(class TAdTrialAgreementGC *owner, s32 a, s32 b);
};
struct TAdAgreementBaseGC : TObject {            /* era header :9 (base state machine = next rung; only the ctor init-list rides here) */
    TAdAgreementBaseGC(TObject *parent);
};
struct TAdTrialAgreementGC : TAdAgreementBaseGC {
    static const char *name;                     /* REAL data slice :52 (m_name pin) */
    TAdTrialAgreementGC(TObject *parent);        /* REAL ctor slice :84-93 */
    void on_agree();                             /* REAL one-liner :40 (mode-8 store proof) */
    void on_decline();                           /* REAL one-liner :39 (request_main_phase(0) honest quit) */
    s32 check_card_file();                       /* REAL slice :55-74 (probe chain rides counted era-zero externs) */
    void wait_for_load();                        /* REAL slice :42-49 (unreachable w/o card — assert 0) */
    TAdScrollWinGC *scroll_window;
};
struct TAdTimeSetGC : TObject {
    TAdTimeSetGC(TObject *parent);               /* COUNTED shell ((c) time-set screen chain, O1: rests at NULL branch) */
    void apply_settings();                       /* REAL slice :885-899 — NULL-branch leg ONLY (owner default O1, docs/p2-memcard-1.md §3) */
    void restart_main_phase();                   /* REAL slice :901-903 */
    void change_state(s32 new_state);            /* COUNTED (state-table leg = TAdTimeSetGC machine, next rung); arg captured (O1: ==1 exactly) */
    void load_scene_resources();                 /* GMS row 8 pair — COUNTED (era body EMPTY at pin, roster §2) */
    void free_scene_resources();
};
struct TTitleScene : TObject {
    TTitleScene(TObject *parent);                /* COUNTED shell — (d) S16 asset+input heavy (HG3-282 stretch; O1 never reaches it) */
};
struct TWindowCtrlMain {                          /* S10 window-ctrl member — COUNTED shell (window lane, roster row 19 family) */
    TWindowCtrlMain();
};
struct TAdSinglePlyChrSelectGC {
    static void run_save_load_sequence();        /* S12 :519 — COUNTED (memcard save/load chain, boot-spine §4 skipped) */
    static void noop();                          /* S12 :520 — COUNTED (era name verbatim; roster deny-roots family, row 10 watch) */
    static u8 *get_global_state();               /* REAL slice _8001ED80.cpp:5-7 (returns system_save_payload verbatim — MEMSCAN §1) */
};
struct TAdPlayerLoadGC {
    static int load_state;                        /* era data def, verbatim line :95 of the GMS unit */
    static s32 get_instance();                    /* REAL slice TAdPlayerLoadGC_80076BBC.cpp:5-7 — R-C gate (era default VERIFIED = 0: card_load_sequence_state is an un-init'd .bss s32 at _801FBF54.cpp:66) */
    static s32 probe_card_slot(saddr chan);       /* COUNTED — returns 0: no memcard on the host (boot-spine §4 'S11 memcard deliberately skipped'); era-true no-card shape per docs/p2-memcard-1.md §2 */
    static s32 probe_and_check_card(saddr chan);  /* COUNTED — unreachable when probe==0; call-count asserted 0 (canary-can-fail) */
    static s32 check_memcard_status(s32 chan);    /* COUNTED — unreachable, assert 0 */
    static s32 find_card_file(saddr chan, const char *name);  /* COUNTED — unreachable, assert 0 */
};
struct TLanguage { static s32 set_language(s8 language); };  /* S15 tail — COUNTED (font/language lane; fires ONLY on a non-NULL payload — O1 keeps 0) */
s32 poll_memcard_result(s32 chan);                            /* TAdPlayerLoadGC_801E0C30.cpp:501 — COUNTED, unreachable, assert 0 */
void update_music_stream_gain(s32 user);                      /* audio/SoundRequests.h:12 — S15 tail COUNTED */
void set_cue_stream_user_gain(s32 gain);                      /* SoundRequests.h:6 — S15 tail COUNTED */
extern s32 quest_download_in_progress_now_unused;             /* marker (unused): ROSTER row-22 carrier already host-side; keeps this header honest about not re-declaring it */

/* ---- SystemSaveData carrier for the apply_settings cast (MEMSCAN §1: era
 * decl TAdSinglePlyChrSelectGC.h:35-47, sizeof 0x11C asserted there; ONLY the
 * three consumed fields carry names here — the era graph stays off (D4); the
 * full-layout assert copy rides the supply-decision leg, NOT O1). ---- */
struct SystemSaveData { u32 checksum; s16 music_stream_gain; s8 cue_stream_gain; s8 language; };
extern SystemSaveData *system_save_payload;      /* era data def VERBATIM (_801FBF54.cpp:63) — NULL ⇔ boot-init not run (MEMSCAN §1); O1 never supplies */
extern s32 card_load_sequence_state;             /* era data def VERBATIM (:66) — R-C era default 0, verified at pin */
extern char pso_system_card_filename[0x20];      /* era data def VERBATIM (:85) */

/* ---- generated-table slot counters (defs hg3_trial_seam.cpp; pair indices
 * 0..22 = game_mode_select_scene_callbacks rows, 23..29 = lobby table rows;
 * the GMS row-11 pair carries NO slot — its table entry references the REAL
 * era-empty slices defined in the glue TU) ---- */
#define P9_DECL_SLOT(i) void p9_slot_ld_##i(void); void p9_slot_rl_##i(void);
P9_DECL_SLOT(0)  P9_DECL_SLOT(1)  P9_DECL_SLOT(2)  P9_DECL_SLOT(3)  P9_DECL_SLOT(4)
P9_DECL_SLOT(5)  P9_DECL_SLOT(6)  P9_DECL_SLOT(7)  P9_DECL_SLOT(8)  P9_DECL_SLOT(9)
/* (10 = REAL pair — no counters) */
P9_DECL_SLOT(11) P9_DECL_SLOT(12) P9_DECL_SLOT(13) P9_DECL_SLOT(14) P9_DECL_SLOT(15)
P9_DECL_SLOT(16) P9_DECL_SLOT(17) P9_DECL_SLOT(18) P9_DECL_SLOT(19) P9_DECL_SLOT(20)
P9_DECL_SLOT(21) P9_DECL_SLOT(22) P9_DECL_SLOT(23) P9_DECL_SLOT(24) P9_DECL_SLOT(25)
P9_DECL_SLOT(26) P9_DECL_SLOT(27) P9_DECL_SLOT(28) P9_DECL_SLOT(29)

/* ---- canary (L9-1): create_mode_scene arm for a mode whose scene class has
 * NOT landed. LB-3 loud, records + flushes (forked-canary regime). ---- */
void p9_fail_loud_mode_scene(s32 mode);

/* ---- seam state the HG3-275..281 test reads (defs hg3_trial_seam.cpp) ---- */
extern int g9_slot_calls[64];                    /* per generated-table slot: 0..22 GMS, 23..29 lobby (load side mirrored at +0, release g9_rl) */
extern int g9_ld[30], g9_rl[30];                 /* g9_ld[i] = load-half fires, g9_rl[i] = release-half fires (i: 0..22 GMS, 23..29 lobby; era callee names baked into the g9_note tags at the defs) */
extern int g9_fires;                             /* loud p9_fail_loud_mode_scene launches */
extern int g9_audio_loud_fires;                  /* load_floor_sound_bank LB-3 launches (un-armed) */
extern int g9_audio_arm;                         /* 0 = LB-3 default (era-honest), 1 = counted golden-walk arm (L9-2, test-set) */
extern int g9_init_leg_calls[10];                /* order legs: 0 set_screen_size .. 9 floor_sound_bank (see seam defs) */
extern int g9_cam_ctor, g9_cam_parent_is_tl_camera, g9_cam_mode;
extern int g9_quad_sprites, g9_winmain_ctor, g9_particle_heap;
extern int g9_sin_arg, g9_cos_arg, g9_light_n;
extern int g9_bgm_calls, g9_bgm_never_selected;  /* apply_bgm_col_ctrl fires (must stay 0 across in-span creates) */
extern int g9_queued_destroy_calls, g9_runsls_calls, g9_noop_calls;
extern int g9_probe_slot, g9_probe_and_check, g9_check_status, g9_find_file, g9_poll_memcard;
extern int g9_title_ctor, g9_title_parent_is_tl_01, g9_timeset_ctor, g9_timeset_parent_is_tl_01;
extern int g9_scroll_ctor, g9_scroll_args[2];
extern int g9_change_state_calls, g9_change_state_last;
extern int g9_tail_calls[6];                     /* set_language, gain, cue_gain, init_sound_mode, unload, load (S15 non-NULL tail) */
extern int g9_area_pending;                      /* is_area_transfer_pending backing (era-zero 0; S10 abort-branch canary arms 1) */
void g9_note(const char *tag);                   /* separate witness log (g9_log/g9_log_n; the g8/LC7 vectors stay untouched) */
extern const char *g9_log[256]; extern int g9_log_n;
extern const void *g9_loud_mode;                 /* mode value at the last loud arm (test-set naming optional) */

#endif /* LADDER9_TRIAL_PRELUDE_H */
