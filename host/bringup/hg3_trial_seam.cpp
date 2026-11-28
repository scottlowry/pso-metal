/* P2-HG3 rung-6 "arrival at the trial screen" — bringup-side seam backings
 * (card BRINGUP-5, legs HG3-275..281; spec docs/p2-rung6-seams-1.md §1 S8-S15,
 * §3 Card BRINGUP-5 + docs/p2-bringup5-roster-1.md grades).
 *
 * Every symbol here is (i) a COUNTED extern for a class-(c)/(d) era body that
 * has NOT landed (records entry + args, never fakes era behavior — the ROSTER
 * grades govern each disposition: lane-J camera halves (d), font lane (d),
 * texture/heap/window (c) members, the six gx-landed legs cited not re-minted
 * per L8-5), (ii) an LB-3 fail-loud per the brief: p9_fail_loud_mode_scene
 * (L9-1 unlanded create_mode_scene arms) and load_floor_sound_bank UN-ARMED
 * (L9-2 audio-dark; boot-spine §4 memcard/audio skip regime), or (iii) an
 * era-zero honest default: is_area_transfer_pending()==0 (rung-5 pending==0
 * guard precedent) and probe_card_slot()==0 (no memcard wired — the era
 * no-card first-boot shape per docs/p2-memcard-1.md §2, which is exactly the
 * branch that SHOWS the trial screen).
 *
 * NO l7_note events fire from this file: the rung-5 LC7_* order vector stays
 * byte-identical; the witness log here is the separate g9_* vector.
 */
#include "ladder9_trial_prelude.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- witness log (separate from g8_* / LC7_*) ---- */
const char *g9_log[256]; int g9_log_n;
void g9_note(const char *tag) { if (g9_log_n < 256) g9_log[g9_log_n++] = tag; }

int g9_slot_calls[64];
int g9_ld[30], g9_rl[30];
int g9_fires, g9_audio_loud_fires, g9_audio_arm;
int g9_init_leg_calls[10];
int g9_cam_ctor, g9_cam_parent_is_tl_camera, g9_cam_mode;
int g9_quad_sprites, g9_winmain_ctor, g9_particle_heap;
int g9_sin_arg, g9_cos_arg, g9_light_n;
int g9_bgm_calls, g9_bgm_never_selected;
int g9_queued_destroy_calls, g9_runsls_calls, g9_noop_calls;
int g9_probe_slot, g9_probe_and_check, g9_check_status, g9_find_file, g9_poll_memcard;
int g9_title_ctor, g9_title_parent_is_tl_01, g9_timeset_ctor, g9_timeset_parent_is_tl_01;
int g9_scroll_ctor, g9_scroll_args[2];
int g9_change_state_calls, g9_change_state_last;
int g9_tail_calls[6];
const void *g9_loud_mode;
int g9_area_pending;                        /* is_area_transfer_pending backing (era-zero default 0; S10 abort-branch canary arms 1) */

static void l9_fatal(const char *what) {
    fprintf(stderr, "L9 FATAL: %s — era-zero/contract violation (LB-3)\n", what);
    exit(9);
}
extern void (*g8_flush_hook)(void);         /* shared-canary persistence hook (hg3_walk_seam.cpp) */

/* ---- L9-1 / L9-2 fail-loud seams ---- */
void p9_fail_loud_mode_scene(s32 mode) {    /* create_mode_scene arm whose scene class has NOT landed (25 arms) */
    g9_fires++;
    g9_loud_mode = (const void *)(long)mode;
    if (g8_flush_hook) g8_flush_hook();
    char buf[160];
    snprintf(buf, sizeof buf, "create_mode_scene launched UNLANDED scene arm (mode %d) — L9-1 loud canary", mode);
    l9_fatal(buf);
}
void load_floor_sound_bank(s32 floor) {     /* audio/SoundPlayback.h:22 — L9-2 audio-dark */
    if (!g9_audio_arm) {
        g9_audio_loud_fires++;
        if (g8_flush_hook) g8_flush_hook();
        l9_fatal("load_floor_sound_bank under audio-dark (no musyx/ARAM host analog — boot-spine §4); golden walk arms counted witness");
    }
    g9_init_leg_calls[9]++;
    g9_note("i:soundbank");
}

/* ---- generated-table slot counters (the GENERATED glue table wires these by
 * pair index: 0..22 game_mode_select_scene_callbacks, 23..29 lobby table; era
 * callee names baked into the tags, matching the two-source roster) ---- */
#define P9_LD(i, tag) void p9_slot_ld_##i(void) { g9_ld[i]++; g9_slot_calls[i]++; g9_note(tag); }
#define P9_RL(i, tag) void p9_slot_rl_##i(void) { g9_rl[i]++; g9_slot_calls[32 + i]++; g9_note(tag); }
/* GMS rows 0..22 (load half fires forward; release half fires only on a reverse walk) */
P9_LD(0,  "ld:cam.reset_view_state")    P9_RL(0,  "rl:cam.reset")            /* (d) lane-J */
P9_LD(1,  "ld:cam.init_default_view")   P9_RL(1,  "rl:cam.reset_default_view") /* (d) lane-J */
P9_LD(2,  "ld:kbd.update_menu")         P9_RL(2,  "rl:kbd.free_soft")        /* (d) font lane */
P9_LD(3,  "ld:tex.f256_ad_pic_2")       P9_RL(3,  "rl:chrsel.free_global")
P9_LD(4,  "ld:tex.f256_backtext")       P9_RL(4,  "rl:chrsel.free_singleton")
P9_LD(5,  "ld:tex.f128_ray")            P9_RL(5,  "rl:ray.destroy_instance")
P9_LD(6,  "ld:chrsel.multi_load")       P9_RL(6,  "rl:chrsel.multi_free")
P9_LD(7,  "ld:timeset.load_scene")      P9_RL(7,  "rl:timeset.free_scene")   /* row 8 pair: era bodies EMPTY (ROSTER §2) */
P9_LD(8,  "ld:tex.f512_multiplay")      P9_RL(8,  "rl:gamestartsub.destroy")
P9_LD(9,  "ld:chrsel.alloc_char_bufs")  P9_RL(9,  "rl:chrsel.free_callbacks")/* deny-roots watch (ROSTER §6.4) — counted, never executed */
/* row 11 (i=10) pair = REAL era-empty verbatim slices in the glue TU (self-contained init witness — no counter here) */
P9_LD(11, "ld:multi.load_confirm_tex")  P9_RL(11, "rl:multi.free_confirm_tex")
P9_LD(12, "ld:tex.logo_e")              P9_RL(12, "rl:logo.free_held")       /* gx REAL: gx_logo_driver.cpp:236,865 / :875 — L8-5 counter-backed */
P9_LD(13, "ld:tex.sonic")               P9_RL(13, "rl:tex.sonic_free")       /* gx REAL: :234 / :231 — L8-5 */
P9_LD(14, "ld:chat.init_heap")          P9_RL(14, "rl:chat.free_lines")
P9_LD(15, "ld:prate.register")          P9_RL(15, "rl:prate.free")
P9_LD(16, "ld:snapshot.load_buffers")   P9_RL(16, "rl:snapshot.free_buffers")
P9_LD(17, "ld:kbd.reset_b")             P9_RL(17, "rl:kbd.reset_a")
P9_LD(18, "ld:win.reset_windows")       P9_RL(18, "rl:win.free_all")         /* GVPL bank watch (ROSTER §6.4) */
P9_LD(19, "ld:lobby.set_ending_cb")     P9_RL(19, "rl:lobby.free_ending_cb")
P9_LD(20, "ld:kbd.load_resources")      P9_RL(20, "rl:kbd.free_resources")   /* era bodies EMPTY (ROSTER §2 row 21) */
P9_LD(21, "ld:quest.alloc_buffer")      P9_RL(21, "rl:chrsel.free_singleton2") /* quest-VM trap watch (ROSTER §6.4) */
P9_LD(22, "ld:drawline.load")           P9_RL(22, "rl:drawline.free")
/* lobby table rows 0..6 -> pair indices 23..29 */
P9_LD(23, "ld:lobby.init_title_scene")   P9_RL(23, "rl:lobby.destroy_global")
P9_LD(24, "ld:tex.midashi")              P9_RL(24, "rl:tex.midashi_free")
P9_LD(25, "ld:tex.player_tex")           P9_RL(25, "rl:tex.player_tex_free")
P9_LD(26, "ld:tex.select_gamen")         P9_RL(26, "rl:tex.select_gamen_free")
P9_LD(27, "ld:tex.select_cursor")        P9_RL(27, "rl:tex.select_cursor_free")
P9_LD(28, "ld:tex.chr_sel_slot")         P9_RL(28, "rl:tex.chr_sel_slot_free")
P9_LD(29, "ld:lobby.update_cursor")      P9_RL(29, "rl:lobby.free_global")

/* ---- light carrier storage (era .data homes TObjLight_80133568.h:59/:61; the
 * verbatim S10 body reads &default_scene_light_config by ADDRESS only and WRITES
 * scene_light_state.direction.x/y/z — REAL stores asserted against harness
 * lookup constants, args 0x1555/0x9000 are the era truth) ---- */
LightEntry default_scene_light_config, scene_light_state;

/* ---- S10 prelude legs (defs for the counted externs; every one notes an
 * "i:" tag so the golden walk can assert the VERBATIM body's call order) ---- */
void set_screen_size(f32 w, f32 h) { (void)w; (void)h; g9_init_leg_calls[0]++; g9_note("i:screen-size"); }
void set_copy_clear_color_argb(u32 argb, s32 a, s32 b) { (void)argb; (void)a; (void)b; g9_init_leg_calls[1]++; g9_note("i:clear-color"); }
void reset_controller_input_state(void) { g9_init_leg_calls[2]++; g9_note("i:reset-ctrl"); }
void clear_resource_load_tasks(void) { g9_init_leg_calls[3]++; g9_note("i:clear-resload"); }
void apply_phase_keyboard_mode(void) { g9_init_leg_calls[5]++; g9_note("i:kbd-mode"); }
s32 is_area_transfer_pending(void) { return g9_area_pending; }   /* era-zero default host state (rung-5 pending==0 guard precedent) */
void play_area_music(char *name) { (void)name; g9_bgm_calls++; } /* audio-dark counted (never selected in-span — asserted) */

void TLobby::reset_keyboard_state() { g9_init_leg_calls[4]++; g9_note("i:kbd-reset"); }
void TLobby::clear_callback_and_reset_keyboard() { g9_init_leg_calls[6]++; g9_note("i:kbd-clear"); }
void TLobby::invoke_callback() { g9_note("i:kbd-invoke"); }      /* S10 abort branch only (pending!=0 canary) */
int TLobby::init_sound_mode() { g9_tail_calls[3]++; g9_note("t:sound-mode"); return 0; }  /* S15 tail — O1: must stay 0 */

void init_menu_scene_lights(LightEntry *cfg, s32 n) { (void)cfg; g9_light_n = n; g9_note("i:lights"); }
f32 sin_table_lookup(s32 angle) { g9_sin_arg = angle; g9_note("i:sin"); return 0.25f; }   /* table DATA = math lane; ARG is the era truth checked by the test */
f32 cos_table_lookup(s32 angle) { g9_cos_arg = angle; g9_note("i:cos"); return 0.75f; }
void TParticleCtrl::init_heap(u32 size) { g9_particle_heap = (int)size; g9_init_leg_calls[7]++; g9_note("i:part-heap"); }
void TParticleCtrl::create_quad_sprites(void) { g9_quad_sprites++; g9_init_leg_calls[8]++; g9_note("i:quad"); }
TCameraCtrl::TCameraCtrl(TObject *parent) { g9_cam_ctor++; g9_cam_parent_is_tl_camera = (parent == tl_camera); g9_note("i:cam-ctor"); }  /* (d) lane-J counted; parent arg is era truth */
void TCameraCtrl::select_active_controller(s32 mode) { g9_cam_mode = mode; g9_note("i:cam-mode"); }  /* distinct from rung-5 activate_camera_mode (era names differ) — mode 9 pin */
TWindowCtrlMain::TWindowCtrlMain() { g9_winmain_ctor++; g9_note("i:winmain"); }

/* ---- S12/S13/S14 shells ---- */
void TObject::queue_destruction() { g9_queued_destroy_calls++; g9_note("x:queue-destroy"); }
TAdAgreementBaseGC::TAdAgreementBaseGC(TObject *parent) { (void)parent; g9_note("x:agree-base-ctor"); }
TAdScrollWinGC::TAdScrollWinGC(TAdTrialAgreementGC *owner, s32 a, s32 b) { (void)owner; g9_scroll_ctor++; g9_scroll_args[0] = a; g9_scroll_args[1] = b; g9_note("x:scroll-shell"); }  /* window chain NOT built (card S13) */
TAdTimeSetGC::TAdTimeSetGC(TObject *parent) { g9_timeset_ctor++; g9_timeset_parent_is_tl_01 = (parent == tl_01); g9_note("x:timeset-ctor"); }
void TAdTimeSetGC::change_state(s32 new_state) { g9_change_state_calls++; g9_change_state_last = new_state; g9_note("x:change-state"); }
void TAdTimeSetGC::load_scene_resources() { g9_note("x:timeset-load-w"); }
void TAdTimeSetGC::free_scene_resources() { g9_note("x:timeset-free-w"); }
TTitleScene::TTitleScene(TObject *parent) { g9_title_ctor++; g9_title_parent_is_tl_01 = (parent == tl_01); g9_note("x:title-ctor"); }  /* (d) S16 shell — O1 path never reaches title */
void TAdSinglePlyChrSelectGC::run_save_load_sequence() { g9_runsls_calls++; }
void TAdSinglePlyChrSelectGC::noop() { g9_noop_calls++; }
void TAdMultiChgGameModeGC::apply_bgm_col_ctrl(const char *name) { (void)name; g9_bgm_calls++; g9_note("x:bgm"); }
s32 TLanguage::set_language(s8 lang) { (void)lang; g9_tail_calls[0]++; g9_note("t:language"); return 0; }   /* S15 tail — O1: must stay 0 */
void update_music_stream_gain(s32 user) { g9_tail_calls[1]++; g9_note("t:gain"); }
void set_cue_stream_user_gain(s32 gain) { g9_tail_calls[2]++; g9_note("t:cue-gain"); }

/* ---- memcard probe chain: era-zero no-card shape (boot-spine §4 skipped;
 * docs/p2-memcard-1.md §2 proves the trial screen IS the no-card branch) ---- */
s32 TAdPlayerLoadGC::probe_card_slot(saddr chan) { (void)chan; g9_probe_slot++; g9_note("p:slot"); return 0; }
s32 TAdPlayerLoadGC::probe_and_check_card(saddr chan) { (void)chan; g9_probe_and_check++; g9_note("p:and-check"); return 0; }
s32 TAdPlayerLoadGC::check_memcard_status(s32 chan) { (void)chan; g9_check_status++; g9_note("p:status"); return 0; }
s32 TAdPlayerLoadGC::find_card_file(saddr chan, const char *name) { (void)chan; (void)name; g9_find_file++; g9_note("p:find"); return 0; }
s32 poll_memcard_result(s32 chan) { (void)chan; g9_poll_memcard++; g9_note("p:poll"); return 0; }
