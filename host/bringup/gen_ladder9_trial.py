#!/usr/bin/env python3
"""P2-HG3 rung-6 "arrival at the trial screen" — BRINGUP-5 generator (HG3-275..281).

Reads the READ-ONLY decomp tree (DECOMP_ROOT argv[1], pin 6b38b53837) and emits
build/bringup/gen/ladder9_trial_glue.cpp:
  HG3-275 (S9+S8): the phase-10 init row body init_trial_agreement_phase (:541,
    one-line composition init+open+render_tick) VERBATIM + run_load_callback_table
    VERBATIM (TFontSmallTask.cpp:263-276, the S8 forward walker);
  HG3-276 (S10): init_game_mode_select_scene (:448-492) VERBATIM — the 23+7 pair
    callback tables are GENERATED host copies (roster below, two-source guarded);
  HG3-277 (S11): open_trial_agreement_scene/open_time_set_scene/open_title_scene
    + sync_mode_state + the era data lines (:88-95,:99-100 + the 801FBF54 payload
    globals) — 3-store mode opens, era defaults 29/29/0 pinned from the era text;
  HG3-278 (S12): run_game_mode_select_frame (:494-530) VERBATIM — change-state
    1->2->0 wrapper; the R-C gate TAdPlayerLoadGC::get_instance() lands as its
    REAL 3-line slice (TAdPlayerLoadGC_80076BBC.cpp:5-7) over card_load_sequence_state
    (era zero default VERIFIED at the pin before pinning, brief line 4);
  HG3-279 (S13): create_mode_scene (:318-424) — BGM skip switch VERBATIM; the
    SCENE switch keeps era lines for the 3 in-span cases {0,8,23} + the null
    case 29, every other case arm is replaced by p9_fail_loud_mode_scene(N)
    (deviation L9-1, same unregistered-slot canary discipline as S3/main_phase_table);
    TAdTrialAgreementGC ctor VERBATIM (TAdTrialAgreementGC.cpp:84-93, check_card_file
    :55-74 + wait_for_load :42-49 VERBATIM; probe_card_slot = counted era-zero —
    memcard skipped per boot-spine §4) + the TAdTimeSetGC/TTitleScene ctors counted
    shells (class (d), cited not re-minted);
  HG3-280 (S14): on_agree (:40) + on_decline (:39) VERBATIM one-liners — the
    store-level fires; decline rides request_main_phase (REAL rung-5 slice);
  HG3-281 (S15): TAdTimeSetGC::apply_settings (:885-899) + restart_main_phase
    (:901-903) VERBATIM over the REAL get_global_state slice — O1 era-true: the
    NULL branch is the ONLY landing branch (no shim, no SystemSaveData supply,
    docs/p2-memcard-1.md §3 O1).

Same slice_fn/slice_line anchor machinery as gen_ladder8_walk.py: every anchor
exact-line-unique, bodies must_contain the named era statements, fail-loud
BEFORE any write. Zero bytes of the decomp tree are touched.

Deviation L9-1 (create_mode_scene arm substitution): unlanded (c)/(d) case arms
become loud canaries; the era line is kept as a trailing comment at every
substituted arm (data guard). The second-switch case LABELS all stay (roster
shape = era truth); only the unlanded constructor bodies are replaced.
Deviation L9-2 (audio-dark): NO body edits — load_floor_sound_bank's DISPOSITION
is seam-side (LB-3 loud by default; the golden walk arms a counted witness;
canary run executes un-armed). apply_bgm_col_ctrl counted (boot-spine §4).

Usage: gen_ladder9_trial.py <DECOMP_ROOT> <out_glue.cpp>
"""
import sys, hashlib

ROOT, OUT = sys.argv[1], sys.argv[2]

def F(p): return f"{ROOT}/{p}"

SRC = {
    "fst":   F("src/pso/ui/font/TFontSmallTask.cpp"),
    "mfcg":  F("src/pso/ui/frontend/TAdMultiChgGameModeGC_8023E7A8.cpp"),
    "mcc":   F("src/pso/ui/frontend/TAdMultiChgGameModeGC.cpp"),
    "trial": F("src/pso/ui/frontend/TAdTrialAgreementGC.cpp"),
    "times": F("src/pso/ui/frontend/TAdTimeSetGC.cpp"),
    "plr":   F("src/pso/ui/frontend/TAdPlayerLoadGC_80076BBC.cpp"),
    "chs":   F("src/pso/ui/frontend/TAdSinglePlyChrSelectGC_801FBF54.cpp"),
    "chs2":  F("src/pso/ui/frontend/TAdSinglePlyChrSelectGC_8001ED80.cpp"),
}

def lines(path):
    with open(path, "r") as fh:
        return fh.read().splitlines(keepends=True)

def slice_fn(src, sig, must_contain):
    L = lines(SRC[src])
    idx = [i for i, l in enumerate(L) if l == sig + "\n"]
    if len(idx) != 1:
        sys.exit(f"GUARD: {SRC[src]}: signature count {len(idx)} (want 1): {sig!r}")
    s = idx[0]
    end = next(i for i in range(s, len(L)) if L[i] == "}\n")
    body = "".join(L[s:end + 1])
    for m in must_contain:
        if m not in body:
            sys.exit(f"GUARD: {SRC[src]}:{s+1} slice missing {m!r} — anchor drift")
    return body, s + 1

def slice_line(src, exact):
    L = lines(SRC[src])
    idx = [i for i, l in enumerate(L) if l.rstrip("\n") == exact]
    if len(idx) != 1:
        sys.exit(f"GUARD: {SRC[src]}: exact-line count {len(idx)} (want 1): {exact!r}")
    return exact + "\n", idx[0] + 1

frames = []
def keep(name, body, ln, src):
    frames.append((name, body, ln, SRC[src]))

# ---------------- era DATA lines, verbatim (init defaults ARE era truth) -----
DATA_MFCG_88_95 = [
    "s32 multi_chg_game_mode_state = 29;",
    "int chg_game_mode_state_synced = 29;",
    "s32 game_mode_scene_change_state;",
    "TObject *game_mode_scene_instance;",
    "int TAdPlayerLoadGC::load_state;",
]
DATA_MFCG_99_100 = [
    "static const Vec game_mode_scene_camera_eye = {0.0f, 0.0f, 0.0f};",
    "static const Vec game_mode_scene_camera_target = {0.0f, 0.0f, -100.0f};",
]
DATA_801FBF54 = [
    "SystemSaveData *system_save_payload;",
    "s32 card_load_sequence_state;",
    'char pso_system_card_filename[0x20] = "PSO_SYSTEM";',
]
def emit_data(src, want):
    L = [l.rstrip("\n") for l in lines(SRC[src])]
    out = []
    for w in want:
        c = L.count(w)
        if c != 1:
            sys.exit(f"GUARD: {SRC[src]}: data line count {c} (want 1): {w!r}")
        out.append((w, L.index(w) + 1))
    return out
DATA_BLOCKS = [("mfcg", "era globals :86-95 shape (mode defaults 29/29 — WHAT-GREEN)", emit_data("mfcg", DATA_MFCG_88_95)),
               ("mfcg", "camera pose statics (:97-100)", emit_data("mfcg", DATA_MFCG_99_100)),
               ("chs",  "save-payload + card-state globals (O1: system_save_payload stays era-zero NULL)", emit_data("chs", DATA_801FBF54))]

# ---------------- HG3-275 S8: run_load_callback_table (forward walker) -------
keep("run_load_callback_table  [S8]",
     *slice_fn("fst", "s32 run_load_callback_table(void *table, u32 size) {",
               ["s32 n = size / sizeof(LoadFrameEntry);",
                "if (is_area_transfer_pending() != 0) {",
                "return 0;", "return 1;"]), "fst")

# ---------------- HG3-277 S11: 3-store opens + sync ---------------------------
for nm, sig, mc in [
    ("open_trial_agreement_scene", "void TAdMultiChgGameModeGC::open_trial_agreement_scene() {",
     ["TAdMultiChgGameModeGC::sync_mode_state();", "multi_chg_game_mode_state = 0x17;", "game_mode_scene_change_state = 1;"]),
    ("open_time_set_scene", "void TAdMultiChgGameModeGC::open_time_set_scene() {",
     ["TAdMultiChgGameModeGC::sync_mode_state();", "multi_chg_game_mode_state = 8;", "game_mode_scene_change_state = 1;"]),
    ("open_title_scene", "void TAdMultiChgGameModeGC::open_title_scene() {",
     ["TAdMultiChgGameModeGC::sync_mode_state();", "multi_chg_game_mode_state = 0;", "game_mode_scene_change_state = 1;"])]:
    keep(nm + "  [S11]", *slice_fn("mfcg", sig, mc), "mfcg")
keep("sync_mode_state  [S11 one-behind law :547]",
     *slice_line("mfcg", "void TAdMultiChgGameModeGC::sync_mode_state() { chg_game_mode_state_synced = multi_chg_game_mode_state; }"), "mfcg")

# ---------------- HG3-276 S10: scene-init body (verbatim, externs counted) ---
keep("init_game_mode_select_scene  [S10]",
     *slice_fn("mfcg", "void init_game_mode_select_scene(void) {",
               ["set_screen_size(1.0f, 1.0f);",
                "set_copy_clear_color_argb(0, 0, 0);",
                "reset_controller_input_state();", "clear_resource_load_tasks();",
                "TLobby::reset_keyboard_state();", "apply_phase_keyboard_mode();",
                "run_load_callback_table(game_mode_select_scene_callbacks, sizeof(game_mode_select_scene_callbacks));",
                "run_load_callback_table(lobby_scene_release_callback_table, sizeof(lobby_scene_release_callback_table));",
                "TParticleCtrl::init_heap(0x100000);",
                "init_menu_scene_lights(&default_scene_light_config, 4);",
                "scene_light_state.direction.y = -1.0f * sin_table_lookup(0x1555);",
                "c = cos_table_lookup(0x1555);",
                "new TCameraCtrl(tl_camera);",
                "TCameraCtrl::select_active_controller(9);",
                "TParticleCtrl::create_quad_sprites();",
                "p = (void *)new TWindowCtrlMain();",
                "game_mode_scene_instance = 0;",
                "load_floor_sound_bank(0);"]), "mfcg")

# ---------------- HG3-275 S9: the phase-10 init row ---------------------------
keep("TAdMultiChgGameModeGC::init_trial_agreement_phase  [S9 :541]",
     *slice_line("mfcg", "void TAdMultiChgGameModeGC::init_trial_agreement_phase() { init_game_mode_select_scene(); TAdMultiChgGameModeGC::open_trial_agreement_scene(); render_tick(); }"), "mfcg")

# ---------------- GMS (a)(a) empty pair: the only self-contained REAL slots ---
keep("TAdMultiChgGameModeGC::free_scene_resources  [(a) era-empty]",
     *slice_line("mcc", "void TAdMultiChgGameModeGC::free_scene_resources() {  }"), "mcc")
keep("TAdMultiChgGameModeGC::load_scene_resources  [(a) era-empty]",
     *slice_line("mcc", "void TAdMultiChgGameModeGC::load_scene_resources() {  }"), "mcc")

# ---------------- lobby load/unload free fns (walker riders, REAL) ------------
keep("unload_lobby_scene_resources  [S15 tail rider]",
     *slice_fn("mfcg", "void unload_lobby_scene_resources(void) {",
               ["TLobby::call_funcs_reverse(lobby_scene_release_callback_table, sizeof(lobby_scene_release_callback_table));"]), "mfcg")
keep("load_lobby_scene_resources  [S15 tail rider]",
     *slice_fn("mfcg", "void load_lobby_scene_resources(void) {",
               ["run_load_callback_table(lobby_scene_release_callback_table, sizeof(lobby_scene_release_callback_table));"]), "mfcg")

# ---------------- HG3-278 S12: the frame wrapper ------------------------------
keep("run_game_mode_select_frame  [S12]",
     *slice_fn("mfcg", "void run_game_mode_select_frame(void) {",
               ["case 1:", "game_mode_scene_instance->queue_destruction();",
                "game_mode_scene_change_state = 2;", "game_mode_scene_change_state = 0;",
                "TAdMultiChgGameModeGC::create_mode_scene();",
                "TAdSinglePlyChrSelectGC::run_save_load_sequence();",
                "TAdSinglePlyChrSelectGC::noop();",
                "if (TAdPlayerLoadGC::get_instance() == 0) {",
                "switch_to_fallback_main_phase();", "request_main_phase(0);",
                "render_tick();"]), "mfcg")
# R-C: get_instance REAL slice (gate global's era default pinned AFTER same-turn
# verification: card_load_sequence_state is an uninitialized .bss s32 @:66 => 0).
keep("TAdPlayerLoadGC::get_instance  [R-C REAL slice :5-7]",
     *slice_fn("plr", "s32 TAdPlayerLoadGC::get_instance() {",
               ["return card_load_sequence_state;"]), "plr")

# ---------------- HG3-279 S13: create_mode_scene (BGM switch verbatim, scene
# switch: in-span cases verbatim, unlanded arms = loud canaries, L9-1) ---------
ML = lines(SRC["mfcg"])
si = [i for i, l in enumerate(ML) if l == "void TAdMultiChgGameModeGC::create_mode_scene() {\n"]
if len(si) != 1:
    sys.exit(f"GUARD: create_mode_scene signature count {len(si)} (want 1)")
s0 = si[0]
e0 = next(i for i in range(s0, len(ML)) if ML[i] == "}\n")
block = ML[s0:e0 + 1]
# the two switches inside the block:
sw_idx = [i for i, l in enumerate(block) if l.strip() == "switch (multi_chg_game_mode_state) {"]
if len(sw_idx) != 2:
    sys.exit(f"GUARD: create_mode_scene switch count {len(sw_idx)} (want 2)")
bgm_lines, scene_lines = block[sw_idx[0]:sw_idx[1]], block[sw_idx[1]:]
bgm_cases = [l.strip() for l in bgm_lines if l.strip().startswith("case ")]
if bgm_cases != [f"case {n}:" for n in (0, 8, 9, 18, 19, 21, 22, 23, 27)]:
    sys.exit(f"GUARD: BGM skip-set drifted: {bgm_cases}")
if 'TAdMultiChgGameModeGC::apply_bgm_col_ctrl("advr.adx");' not in "".join(bgm_lines):
    sys.exit("GUARD: BGM default arm missing")
KEEP = {0, 8, 23, 29}   # 3 in-span scene cases + era-null case 29 (initial mode)
head, tail, arms, order, cur, arm = [], [], {}, [], None, None
for l in scene_lines:
    t = l.strip()
    if t.startswith("case "):
        if cur is not None:
            sys.exit("GUARD: create_mode_scene case without break")
        cur = int(t[5:-1]); order.append(cur); arm = [l]
        continue
    if cur is None:
        (tail if len(arms) == 29 else head).append(l)
        continue
    arm.append(l)
    if t == "break;":
        arms[cur] = arm; cur = None; arm = None
if cur is not None or set(order) != set(range(30)) - {2} or len(order) != 29:
    sys.exit(f"GUARD: scene switch case set drifted: {sorted(order)}")  # era covers 0..29 EXCEPT 2 (verified this turn)
kept_body = "".join(block[0:2]) + "".join(bgm_lines) + "".join(head)
for n in order:
    if n in KEEP:
        kept_body += "".join(arms[n])
    else:
        era_arm_src = " ".join(x.strip() for x in arms[n][1:-1])
        kept_body += (f"    case {n}:\n    p9_fail_loud_mode_scene({n});"
                      f"  /* L9-1 era arm: {era_arm_src} */\n    break;\n")
kept_body += "".join(tail)
for probe in ("game_mode_scene_instance = new TTitleScene(tl_01);",
              "game_mode_scene_instance = (TObject *)new TAdTimeSetGC(tl_01);",
              "game_mode_scene_instance = new TAdTrialAgreementGC(tl_01);",
              "    case 29:\n        break;\n"):
    if probe not in kept_body:
        sys.exit(f"GUARD: kept case lost era line {probe!r}")
if kept_body.startswith("void TAdMultiChgGameModeGC::create_mode_scene() {\n    TObject *p;\n") is False:
    sys.exit("GUARD: create_mode_scene block head drifted")
n_loud = kept_body.count("p9_fail_loud_mode_scene(")
if n_loud != 25:
    sys.exit(f"GUARD: loud-arm count {n_loud} (want 25 = 29 cases - {sorted(KEEP)})")
keep("create_mode_scene  [S13; L9-1 unlanded arms = loud canaries]", kept_body, s0 + 1, "mfcg")

# ---------------- HG3-280 S14: trial agreement legs ---------------------------
keep("TAdTrialAgreementGC::name  [era data :52]",
     *slice_line("trial", 'const char *TAdTrialAgreementGC::name = "TAdTrialAgreementGC";'), "trial")
keep("TAdTrialAgreementGC::on_decline  [S14 :39]",
     *slice_line("trial", "void TAdTrialAgreementGC::on_decline() { request_main_phase((saddr)((void*)0)); }"), "trial")
keep("TAdTrialAgreementGC::on_agree  [S14 :40 -> mode-8 store]",
     *slice_line("trial", "void TAdTrialAgreementGC::on_agree() { TAdMultiChgGameModeGC::open_time_set_scene(); }"), "trial")
keep("TAdTrialAgreementGC::wait_for_load  [probe chain]",
     *slice_fn("trial", "void TAdTrialAgreementGC::wait_for_load() {",
               ["TAdPlayerLoadGC::probe_card_slot(0) == 0", "poll_memcard_result(0) == 0", "render_tick();"]), "trial")
keep("TAdTrialAgreementGC::check_card_file  [S14 probe :55-74]",
     *slice_fn("trial", "s32 TAdTrialAgreementGC::check_card_file() {",
               ["if (TAdPlayerLoadGC::probe_card_slot(0) == 0) {", "return 0;",
                "wait_for_load();", "check_memcard_status(0)", "find_card_file(0, pso_system_card_filename)",
                "(card_file + 0x10000) == 0xFF41U"]), "trial")
keep("TAdTrialAgreementGC::TAdTrialAgreementGC  [S13/S14 ctor VERBATIM :84-93]",
     *slice_fn("trial", "TAdTrialAgreementGC::TAdTrialAgreementGC(TObject *parent) : TAdAgreementBaseGC(parent) {",
               ["m_name = TAdTrialAgreementGC::name;", "scroll_window = NULL;",
                "if (check_card_file() != 0) {", "on_agree();", "queue_destruction();",
                "scroll_window = new TAdScrollWinGC(this, 1, 0);"]), "trial")

# ---------------- HG3-281 S15: time-set NULL-branch legs ----------------------
keep("TAdSinglePlyChrSelectGC::get_global_state  [REAL accessor :5-7]",
     *slice_fn("chs2", "u8 *TAdSinglePlyChrSelectGC::get_global_state() {",
               ["return (u8 *)system_save_payload;"]), "chs2")
keep("TAdTimeSetGC::apply_settings  [S15 :885-899 — O1: NULL branch is the landing branch]",
     *slice_fn("times", "void TAdTimeSetGC::apply_settings() {",
               ["SystemSaveData *state = (SystemSaveData *)TAdSinglePlyChrSelectGC::get_global_state();",
                "if (state == NULL) {", "change_state(1);", "return;",
                "TLanguage::set_language(state->language);",
                "update_music_stream_gain(state->music_stream_gain);",
                "set_cue_stream_user_gain(state->cue_stream_gain);",
                "TLobby::init_sound_mode();",
                "unload_lobby_scene_resources();", "load_lobby_scene_resources();",
                "TAdMultiChgGameModeGC::open_title_scene();"]), "times")
keep("TAdTimeSetGC::restart_main_phase  [S15 :901-903]",
     *slice_fn("times", "void TAdTimeSetGC::restart_main_phase() {",
               ["request_main_phase(0);"]), "times")

# ---------------- GENERATED callback tables (two-source, R-B census) ---------
GMS_ROSTER = [  # 23 pairs, era rows :106-128 VERBATIM names (ROSTER doc §2)
    ("TCameraCtrl::reset_view_state", "TCameraCtrl::reset"),
    ("TCameraCtrl::init_default_view", "TCameraCtrl::reset_default_view"),
    ("TLobby::update_keyboard_menu", "TLobby::free_keyboard_soft_resources"),
    ("register_texture_f256_ad_pic_2", "TAdSinglePlyChrSelectGC::free_global"),
    ("register_texture_f256_backtext", "TAdSinglePlyChrSelectGC::free_singleton_805D65C0"),
    ("register_texture_f128_ray", "TAdRayGC::destroy_instance"),
    ("load_multi_ply_chr_select_resources", "free_multi_ply_chr_select_resources"),
    ("TAdTimeSetGC::load_scene_resources", "TAdTimeSetGC::free_scene_resources"),
    ("register_texture_f512_multiplay", "TAdGameStartSubGC::destroy_instance"),
    ("TAdSinglePlyChrSelectGC::alloc_char_buffers", "TAdSinglePlyChrSelectGC::free_callbacks"),
    ("TAdMultiChgGameModeGC::load_scene_resources", "TAdMultiChgGameModeGC::free_scene_resources"),
    ("load_multi_confirm_textures", "free_multi_confirm_textures"),
    ("register_texture_logo_e", "TLogo::free_held_object"),
    ("load_sonic_logo_texture", "free_sonic_logo_texture"),
    ("init_chat_log_heap", "free_chat_log_lines"),
    ("register_player_prate", "free_player_prate"),
    ("load_data_buffers", "TObjBoss2Tunnel::free_buffers"),
    ("KeyboardKeyState::reset_global_keys_b", "KeyboardKeyState::reset_global_keys_a"),
    ("TWindowCtrlMain::reset_windows", "TWindowCtrlMain::free_all_windows"),
    ("TLobby::set_ending_callback", "TLobby::free_ending_callback"),
    ("load_keyboard_resources", "free_keyboard_resources"),
    ("alloc_quest_download_buffer", "TLobby::free_chr_select_singleton"),
    ("load_draw_line_task_resource", "free_draw_line_task_resource"),
]
LOBBY_ROSTER = [  # 7 pairs, era rows :133-139
    ("initialize_lobby_title_scene", "TLobby::destroy_global_object"),
    ("load_title_midashi_texture", "free_title_midashi_texture"),
    ("load_player_tex_texture", "free_player_tex_texture"),
    ("load_select_gamen_texture", "free_select_gamen_texture"),
    ("load_select_cursor_texture", "free_select_cursor_texture"),
    ("load_chr_sel_slot_texture", "free_chr_sel_slot_texture"),
    ("TLobby::update_menu_cursor_selection", "TLobby::free_global_object"),
]
def parse_table(sig):
    ti = [i for i, l in enumerate(ML) if l.startswith(sig)]
    if len(ti) != 1:
        sys.exit(f"GUARD: {sig} count {len(ti)} (want 1)")
    tj = next(i for i in range(ti[0], len(ML)) if ML[i] == "};\n")
    rows = [l.strip().rstrip(",").strip() for l in ML[ti[0] + 1:tj] if l.strip().startswith("{")]
    names = [[s.strip() for s in r[1:-1].split(",")] for r in rows]
    return names, ti[0] + 1
gms_names, gms_ln = parse_table("FloorResourceCallbacks game_mode_select_scene_callbacks[23] = {")
lobby_names, lobby_ln = parse_table("FloorResourceCallbacks lobby_scene_release_callback_table[7] = {")
if gms_names != [list(p) for p in GMS_ROSTER]:
    sys.exit(f"GUARD: era GMS table text drifted from the embedded roster ({len(gms_names)} rows) — stop")
if lobby_names != [list(p) for p in LOBBY_ROSTER]:
    sys.exit(f"GUARD: era lobby table text drifted from the embedded roster ({len(lobby_names)} rows) — stop")
# census canary (ROSTER §5): 62 distinct callees across the three rung-6 tables
# (logo 18-name table already byte-guarded by gen_ladder8_walk.py; 46 + 14 + 2)
distinct = set()
for p in GMS_ROSTER + LOBBY_ROSTER:
    distinct.update(p)
LOGO_ONLY = {"load_adx_logo_texture", "free_adx_logo_texture"}
if len(distinct) + len(LOGO_ONLY) != 62:
    sys.exit(f"GUARD: distinct callees {len(distinct)} + 2 logo-exclusive (want 62 total, ROSTER §5) — roster drifted")

# host backing map: GMS 11 REAL (empty era slices); the 6 landed texture legs
# are bringup counters citing the gx seat (L8-5 regime, ROSTER §2 rows 13-14 +
# §4 row 9); everything else counted (d)/(c) per ROSTER grades.
REAL = {("TAdMultiChgGameModeGC::load_scene_resources", "m"), ("TAdMultiChgGameModeGC::free_scene_resources", "m")}
GX_CITE = {
    "register_texture_logo_e": "gx_logo_driver.cpp:236,865",
    "TLogo::free_held_object": "gx_logo_driver.cpp:875",
    "load_sonic_logo_texture": "gx_logo_driver.cpp:234",
    "free_sonic_logo_texture": "gx_logo_driver.cpp:231",
}
def back(side, i, name):
    if name == "TAdMultiChgGameModeGC::load_scene_resources" or name == "TAdMultiChgGameModeGC::free_scene_resources":
        return name  # REAL verbatim slice (defined above in this TU)
    return f"p9_slot_{'ld' if side=='L' else 'rl'}_{i}"

hdr = ("/* GENERATED by host/bringup/gen_ladder9_trial.py — P2-HG3 rung-6 \"arrival at the\n"
       " * trial screen\" (card BRINGUP-5, HG3-275..281). Era sources READ-ONLY @ decomp\n"
       " * root argv[1] (pin 6b38b53837); every slice anchored by an exact-unique line,\n"
       " * bodies re-guarded (fail-loud before any write). Deviations: L9-1 (create_mode_\n"
       " * scene unlanded case arms -> p9_fail_loud_mode_scene, era line kept as comment),\n"
       " * L9-2 (audio-dark load_floor_sound_bank: seam LB-3 by default, counted only when\n"
       " * the HG3 golden walk arms it), L8-5-regime (texture slots are bringup counters\n"
       " * citing gx seat lines). Tables: two-source byte-validated (era text vs embedded\n"
       " * roster) — drift = loud exit at generation, R-B census discharged per\n"
       " * docs/p2-bringup5-roster-1.md. */\n"
       '#include "ladder9_trial_prelude.h"\n\n')
body = hdr

for src, note, rows in DATA_BLOCKS:
    body += f"/* ---- era data VERBATIM @ {SRC[src].split('/src/')[-1]} — {note} ---- */\n"
    for txt, ln in rows:
        body += f"{txt}  /* :{ln} */\n"
    body += "\n"

body += (f"/* ---- GENERATED host copy: game_mode_select_scene_callbacks (23 pairs,\n"
         f" * era text @{SRC['mfcg'].split('/src/')[-1]}:{gms_ln}, two-source byte-validated).\n"
         f" * Pair layout = host FloorResourceCallbacks (2 ptrs = 16B) — the SAME stride-\n"
         f" * parity-invariant interpretation as L8-4: run_load_callback_table reads the\n"
         f" * LOAD halves (offset 0), call_funcs_reverse the RELEASE halves (offset 8).\n"
         f" * Slots: counted externs per ROSTER grades; GMS row 11 pair REAL (era-empty\n"
         f" * slices in this TU); gx-landed legs cited at their rows (never re-minted). ---- */\n"
         "FloorResourceCallbacks game_mode_select_scene_callbacks[23] = {\n")
for i, (a, b) in enumerate(GMS_ROSTER):
    aa = a if (a, "m") in REAL else back("L", i, a)
    bb = b if (b, "m") in REAL else back("R", i, b)
    cite = ""
    if a in GX_CITE: cite = f"  /* gx REAL: {GX_CITE[a]} */"
    if b in GX_CITE: cite = f"  /* gx REAL: {GX_CITE[b]} */"
    body += f"    {{ {aa}, {bb} }},  /* pair {i} era: ({a}, {b}) */{cite}\n"
body += "};\n\n"

body += (f"/* ---- GENERATED host copy: lobby_scene_release_callback_table (7 pairs,\n"
         f" * era text @{SRC['mfcg'].split('/src/')[-1]}:{lobby_ln}, byte-validated). ---- */\n"
         "FloorResourceCallbacks lobby_scene_release_callback_table[7] = {\n")
for i, (a, b) in enumerate(LOBBY_ROSTER):
    body += f"    {{ {back('L', 23 + i, a)}, {back('R', 23 + i, b)} }},  /* pair {i} era: ({a}, {b}) */\n"
body += "};\n\n"

for name, text_, ln, src in frames:
    body += f"/* ---- verbatim {name} @ {src.split('/src/')[-1]}:{ln} ---- */\n#pragma push\n{text_}\n#pragma pop\n\n"

with open(OUT, "w") as fh:
    fh.write(body)

for src in SRC.values():
    h = hashlib.sha256(open(src, "rb").read()).hexdigest()[:12]
    print(f"sha {h} {src.split('/src/')[-1]}")
for name, text_, ln, src in frames:
    print(f"slice: {name} @ {src.split('/src/')[-1]}:{ln} ({len(text_.splitlines())} lines)")
print("data: game_mode_select_scene_callbacks 23 pairs (two-source byte-validated) + lobby_scene_release_callback_table 7 pairs")
print(f"data: 10 era data lines verbatim; create_mode_scene: 4 verbatim cases, 25 loud arms (L9-1)")
print(f"gen: {OUT} ({len(frames)} slices + 2 generated tables + 8 verbatim data lines)")
