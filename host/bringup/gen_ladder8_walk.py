#!/usr/bin/env python3
"""P2-HG3 rung-6 "the walk" — guarded dispatcher + exit-table generator (BRINGUP-4).

Reads the READ-ONLY decomp tree (DECOMP_ROOT argv[1], pin 6b38b53837) and emits
build/bringup/gen/ladder8_walk_glue.cpp:
  HG3-271 (S2/S18): the REAL dispatcher body (SystemBootInit.cpp main :365-395,
    renamed at EMIT — deviation L8-1), the five S18 phase-state helper slices
    (:157-176, :214-217) + reset_main_phase_state (:224);
  HG3-272 (S3): the HOST-SIDE main_phase_table copy — all 22 era rows parsed
    from the block (SystemBootInit.cpp:340-363) and byte-validated against the
    embedded roster, rows {0,1,2,5,10} + the three NULL sentinels get era-true
    slots/backings, every other slot is the p8_fail_loud_main_phase LB-3
    stand-in ("unregistered row index trips loud");
  HG3-273 (S4+S6+S7): TLogo::exit_logo_scene VERBATIM (TLogo.cpp:117-121),
    TLobby::call_funcs_reverse VERBATIM (TFontSmallTask.cpp:251-261), and the
    host copy of logo_load_callback_table — the 18-pointer roster PARSED from
    the era text (:39-49) and cross-validated against the embedded roster
    (two-source drift guard); every entry slot-backed, the release halves are
    what the reverse walk fires (LobbyFrameEntry pair reinterpretation);
  HG3-274 (S5): TMainTask::delete_layer_children VERBATIM (TObject.cpp:575-594,
    guard: exactly 19 tl_*->delete_children() calls) — delete_children stays a
    counted extern per class (allocator seam, class (c)).

Same slice_fn/slice_line anchor machinery as gen_ladder7_runframe.py: every
anchor exact-line-unique, bodies must_contain the named era statements,
fail-loud BEFORE any write. Zero bytes of the decomp tree are touched.

Two DISCLOSED body-level edits (both mechanical, guarded by an exact-unique
string replace that fail-louds on drift):
  L8-1  'void main(void) {'        -> 'void pso_main_dispatcher(void) {'
  L8-2  'while (main_phase_state.cur != 0) {'
        -> 'while (main_phase_state.cur != 0 && !pso_walk_stop_requested()) {'
        (test-harness clean-return hook; pso_walk_stop_requested()==0 outside
        the HG3 harness, so the walk matches era exactly there).
Plus two EMITTED lines with NO era counterpart (harness only):
  p8_snapshot_loop_top(); right after each loop-top 'result = 0;' store, and
  'return;' before the closing brace of the switch's case-1/2/3 arm (the
  iteration-boundary exit the stand-in slots arm via longjmp — see
  hg3_walk_test.cpp; with pso_walk_stop_requested()==0 the body still has one
  exit: shutdown_and_reset_system, era-faithful).

Usage: gen_ladder8_walk.py <DECOMP_ROOT> <out_walk.cpp> <out_s18.cpp>
  out_walk.cpp : dispatcher + exit-table legs + both GENERATED data tables.
  out_s18.cpp  : the S18 phase-state helper slices alone (SystemBootInit.cpp
                 :157-176/:214-229) — a separate TU so the rung-5 ladder7
                 binary can bind run_frame's trap leg to the REAL promoted
                 slice without dragging the dispatcher/harness deps.
"""
import sys, hashlib

ROOT, OUT, OUT2 = sys.argv[1], sys.argv[2], sys.argv[3]

def F(p): return f"{ROOT}/{p}"

SRC = {
    "sbi":   F("src/pso/ui/keyboard/SystemBootInit.cpp"),
    "tlogo": F("src/pso/ui/TLogo.cpp"),
    "fst":   F("src/pso/ui/font/TFontSmallTask.cpp"),
    "tobj":  F("src/pso/system/task/TObject.cpp"),
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

def replace_once(body, src, old, new, label):
    if body.count(old) != 1:
        sys.exit(f"GUARD: {SRC[src]}: {label} anchor count {body.count(old)} (want 1): {old!r}")
    return body.replace(old, new)

frames = []
def keep(name, body, ln, src):
    frames.append((name, body, ln, SRC[src]))

# ---------------- HG3-271 S2: the dispatcher body (renamed, harness lines) ----
disp, disp_ln = slice_fn("sbi", "void main(void) {", [
    "init_game_systems();",
    "while (main_phase_state.cur != 0) {",
    "main_phase_state.result = 0;",
    "fn = main_phase_table[main_phase_state.cur].init;",
    "update = main_phase_table[main_phase_state.cur].update;",
    "while (main_phase_state.result == 0 && update != NULL) {",
    "update();",
    "actor_frame_tick++;",
    "fn = main_phase_table[main_phase_state.cur].exit;",
    "switch (main_phase_state.result) {",
    "main_phase_state.prev = main_phase_state.cur;",
    "main_phase_state.cur = main_phase_state.next;",
    "shutdown_and_reset_system();",
])
disp = replace_once(disp, "sbi", "void main(void) {", "void pso_main_dispatcher(void) {", "L8-1 sig rename")
disp = replace_once(disp, "sbi",
    "    while (main_phase_state.cur != 0) {",
    "    while (main_phase_state.cur != 0 && !pso_walk_stop_requested()) {", "L8-2 loop-top AND")
# emitted (non-era) harness lines, anchored on exact-unique era text:
def insert_after(body, src, anchor, add, label):
    if body.count(anchor) != 1:
        sys.exit(f"GUARD: {SRC[src]}: {label} anchor count {body.count(anchor)} (want 1)")
    return body.replace(anchor, anchor + add)
disp = insert_after(disp, "sbi",
    "        main_phase_state.result = 0;\n",
    "        p8_snapshot_loop_top();\n", "loop-top snapshot")
disp = insert_after(disp, "sbi",
    "            main_phase_state.cur = main_phase_state.next;\n",
    "            if (pso_walk_stop_requested()) return;\n", "switch-arm exit")
keep("main [pso_main_dispatcher — L8-1/L8-2 + 2 emitted harness lines]", disp, disp_ln, "sbi")

# ---------------- HG3-271 S18: the five helpers + reset (all class (a)) ------
# → emitted SEPARATELY into OUT2 (shared by ladder7 + hg3 link lines).
helps = []
def keeph(name, body, ln, src): helps.append((name, body, ln, SRC[src]))
keeph("get_previous_main_phase", *slice_fn("sbi", "s32 get_previous_main_phase(void) {", ["return main_phase_state.prev;"]), "sbi")
keeph("get_main_phase",          *slice_fn("sbi", "s32 get_main_phase(void) {", ["return main_phase_state.cur;"]), "sbi")
keeph("set_fallback_main_phase", *slice_fn("sbi", "void set_fallback_main_phase(s32 phase) {", ["main_phase_state.fallback = phase;"]), "sbi")
keeph("set_queued_main_phase",   *slice_fn("sbi", "void set_queued_main_phase(s32 phase) {", ["main_phase_state.queued_main_phase = phase;"]), "sbi")
keeph("switch_to_fallback_main_phase  [S18 PROMOTED from LB-3]",
     *slice_fn("sbi", "s32 switch_to_fallback_main_phase(void) {",
               ["main_phase_state.result = 3;", "return main_phase_state.next = main_phase_state.fallback;"]), "sbi")   # :173-176, result=3 path
keeph("request_queued_main_phase",
     *slice_fn("sbi", "s32 request_queued_main_phase(void) {",
               ["main_phase_state.result = 2;", "return main_phase_state.next = main_phase_state.queued_main_phase;"]), "sbi")
keeph("reset_main_phase_state",  *slice_fn("sbi", "void reset_main_phase_state(void) {",
               ["main_phase_state.cur = 1;", "main_phase_state.queued_main_phase = 1;",
                "main_phase_state.fallback = 1;", "main_phase_state.prev = 1;"]), "sbi")

# ---------------- HG3-273 S6: call_funcs_reverse (11-line release-half walker)
keep("TLobby::call_funcs_reverse",
     *slice_fn("fst", "void TLobby::call_funcs_reverse(void *table, unsigned int size) {",
               ["s32 n = size / sizeof(LobbyFrameEntry);",
                "LobbyFrameEntry *p = (LobbyFrameEntry *)table + n;",
                "while (n > 0) {", "p--;", "if (p->fn != NULL) {", "p->fn();"]), "fst")

# ---------------- HG3-273 S4: TLogo::exit_logo_scene (fires exactly once) ----
keep("TLogo::exit_logo_scene",
     *slice_fn("tlogo", "void TLogo::exit_logo_scene() {",
               ["main_task.delete_layer_children();",
                "TLobby::call_funcs_reverse(logo_load_callback_table, sizeof(logo_load_callback_table));",
                "render_tick();"]), "tlogo")

# ---------------- HG3-274 S5: delete_layer_children (19-root roster) ---------
dlc, dlc_ln = slice_fn("tobj", "void TMainTask::delete_layer_children() {", [])
nkids = sum(1 for l in dlc.splitlines() if "->delete_children();" in l)
if nkids != 19:
    sys.exit(f"GUARD: delete_layer_children has {nkids} delete_children calls (want 19) — era drift")
keep("TMainTask::delete_layer_children [19 tl_* roots]", dlc, dlc_ln, "tobj")

# ---------------- HG3-272 S3: main_phase_table — era 22-row block validated,
# host copy emitted with the era TEXT as the per-row guard (two-source: the
# parsed block must equal ER_ROWS; drift = loud exit, spec §4 R-B discipline).
ER_ROWS = [
    "    { NULL, NULL, NULL },",
    "    { TLogo::init_logo_scene, TLogo::run_frame, TLogo::exit_logo_scene },",
    "    { TAdMultiChgGameModeGC::init_title_phase, run_game_mode_select_frame, TLobby::release_scene_resources },",
    "    { TKeyBoardControl::update_menu_scene, update_movie_phase, exit_movie_phase },",
    "    { TAdMultiChgGameModeGC::init_dialing_select_phase, run_game_mode_select_frame, TLobby::release_scene_resources },",
    "    { TKeyBoardControl::update_main_menu_scene, run_main_frame, exit_main_menu_scene },",
    "    { TAdMultiChgGameModeGC::init_story_select_phase, run_game_mode_select_frame, TLobby::release_scene_resources },",
    "    { TAdMultiChgGameModeGC::init_multi_save_phase, run_game_mode_select_frame, TLobby::release_scene_resources },",
    "    { start_ending_sequence, TKeyBoardControl::run_main_frame, TAdEndText::cleanup },",
    "    { TAdMultiChgGameModeGC::init_time_set_phase, run_game_mode_select_frame, TLobby::release_scene_resources },",
    "    { TAdMultiChgGameModeGC::init_trial_agreement_phase, run_game_mode_select_frame, TLobby::release_scene_resources },",
    "    { TAdMultiChgGameModeGC::init_enter_password_phase, run_game_mode_select_frame, TLobby::release_scene_resources },",
    "    { TEnding::set_flag_and_advance, run_game_frame, release_ending_scene },",
    "    { TAdMultiChgGameModeGC::init_level_select_phase, run_game_mode_select_frame, TLobby::release_scene_resources },",
    "    { init_offline_game_scene, dispatch_offline_main_frame, TLobby::teardown_scene },",
    "    { run_online_lobby_scene, update_camera_state, cleanup_online_lobby_scene },",
    "    { TEnding::start_ending_scene, run_game_frame, release_ending_scene },",
    "    { NULL, NULL, NULL },",
    "    { init_single_player_char_select_scene, TAdSinglePlyChrSelectGC::run_frame, release_single_ply_chr_select_scene },",
    "    { TAdTrialModemChkGC::init_modem_check_phase, TAdTrialModemChkGC::check_modem_result, TAdTrialModemChkGC::exit_modem_check_phase },",
    "    { NULL, NULL, NULL },",
    "    { TItemReverseTech::init_scene, TItemReverseTech::run_frame, TItemReverseTech::run_cleanup_callbacks },",
]
L = lines(SRC["sbi"])
ti = [i for i, l in enumerate(L) if l == "MainPhaseEntry main_phase_table[] = {\n"]
if len(ti) != 1:
    sys.exit(f"GUARD: table signature count {len(ti)} (want 1)")
tj = next(i for i in range(ti[0], len(L)) if L[i] == "};\n")
er_rows = [l.rstrip("\n") for l in L[ti[0] + 1:tj] if l.strip().startswith("{")]
if er_rows != ER_ROWS:
    sys.exit(f"GUARD: era main_phase_table block drifted from the embedded roster ({len(er_rows)} rows parsed) — stop")
TAB_LN = ti[0] + 1

# Host backing map: era text is the DATA guard; the C symbol lands REAL
# (linked verbatim slice), WITNESS (era body not landed — class (c)/(d) stand-
# in that RECORDS and never fakes era behavior), LOUD (fail-l LB-3), NULL.
R = p8 = "p8_"
BACK = {
    "TLogo::init_logo_scene":        R + "enter_logo_init",     # era body is the (d) load chain (lane J + S8 forward walk) — WITNESS entry only
    "TLogo::run_frame":              "TLogo::run_frame",        # REAL rung-5 slice (linked, ladder7_runframe_glue.o)
    "TLogo::exit_logo_scene":        "TLogo::exit_logo_scene",  # REAL slice above (HG3-273)
    "TAdMultiChgGameModeGC::init_trial_agreement_phase": R + "enter_trial_init",  # S9/HG3-275 body — witness entry ('table[0xA].init entered', spec §3)
    "NULL":                          None,                       # the three sentinel rows verbatim-NULL
}
def slots(row):
    if row == ER_ROWS[0] or row == ER_ROWS[17] or row == ER_ROWS[20]:
        return None, None, None                      # era NULL sentinel — host copy stays NULL
    inner = row.strip()[1:-2]   # '{ a, b, c },' -> ' a, b, c '
    names = [s.strip() for s in inner.split(",")]
    if len(names) != 3:
        sys.exit(f"GUARD: row parse arity {len(names)}: {row!r}")
    return [BACK.get(n, R + "fail_loud_main_phase") for n in names]

# ---------------- HG3-273 S7: logo_load_callback_table — era 18-ptr roster ----
LOGO_ROSTER = [
    "TCameraCtrl::reset_view_state", "TCameraCtrl::reset",
    "TCameraCtrl::init_default_view", "TCameraCtrl::reset_default_view",
    "TLobby::update_keyboard_menu", "TLobby::free_keyboard_soft_resources",
    "register_texture_f256_ad_pic_2", "TAdSinglePlyChrSelectGC::free_global",
    "register_texture_f256_backtext", "TAdSinglePlyChrSelectGC::free_singleton_805D65C0",
    "register_texture_logo_e", "TLogo::free_held_object",
    "load_sonic_logo_texture", "free_sonic_logo_texture",
    "TWindowCtrlMain::reset_windows", "TWindowCtrlMain::free_all_windows",
    "load_adx_logo_texture", "free_adx_logo_texture",
]
slogo = SRC["tlogo"].split("/")[-1]
LL = lines(SRC["tlogo"])
ki = [i for i, l in enumerate(LL) if l == "void (*logo_load_callback_table[18])(void) = {\n"]
if len(ki) != 1:
    sys.exit(f"GUARD: logo table signature count {len(ki)} (want 1)")
kj = next(i for i in range(ki[0], len(LL)) if LL[i] == "};\n")
text = "".join(LL[ki[0] + 1:kj])
names = [s.strip() for chunk in text.replace("\n", " ").split(",") for s in [chunk] if s.strip()]
names = [n.rstrip(";").strip() for n in names]
if names != LOGO_ROSTER:
    sys.exit(f"GUARD: era logo_load_callback_table text drifted from the embedded roster: {names}")
LOGO_LN = ki[0] + 1
# slot backings (host names — era names may be class-method refs). REAL where
# the gx seat already landed the leg (cite gx_logo_driver.cpp lines; the
# bringup binary does NOT link host/gx/**, integrator-owned: the slot is a
# counter whose body NEVER fakes the gx effect — deviation L8-5).
LOGO_BACK = {
    "TCameraCtrl::reset_view_state":            R + "cam_reset_view_state",        # (d) lane-J counted
    "TCameraCtrl::reset":                       R + "cam_reset",                   # (d) lane-J counted — RELEASE half (fires)
    "TCameraCtrl::init_default_view":           R + "cam_init_default_view",       # (d) lane-J counted
    "TCameraCtrl::reset_default_view":          R + "cam_reset_default_view",      # (d) lane-J counted — RELEASE half (fires)
    "TLobby::update_keyboard_menu":             R + "lobby_update_keyboard_menu",
    "TLobby::free_keyboard_soft_resources":     R + "lobby_free_keyboard_soft",    # (d) font/lobby lane counted — RELEASE half (fires)
    "register_texture_f256_ad_pic_2":           R + "tex_register_f256_ad_pic_2",  # ID-slot (load half) — never called by the reverse walk
    "TAdSinglePlyChrSelectGC::free_global":     R + "chrsel_free_global",          # RELEASE half (fires) — (d) counted
    "register_texture_f256_backtext":           R + "tex_register_f256_backtext",
    "TAdSinglePlyChrSelectGC::free_singleton_805D65C0": R + "chrsel_free_singleton",
    "register_texture_logo_e":                  R + "tex_register_logo_e",         # REAL gx seat: gx_logo_driver.cpp:238-:867
    "TLogo::free_held_object":                  R + "tex_free_held_object",        # REAL gx seat: gx_logo_driver.cpp:877 leg — bringup counter (L8-5)
    "load_sonic_logo_texture":                  R + "tex_load_sonic",              # REAL gx seat: gx_logo_driver.cpp:236
    "free_sonic_logo_texture":                  R + "tex_free_sonic",              # gx :233 era-EMPTY — bringup counter
    "TWindowCtrlMain::reset_windows":           R + "win_reset_windows",
    "TWindowCtrlMain::free_all_windows":        R + "win_free_all_windows",        # RELEASE half (fires) — (d) counted
    "load_adx_logo_texture":                    R + "tex_load_adx",                # REAL gx seat: gx_logo_driver.cpp:237
    "free_adx_logo_texture":                    R + "tex_free_adx",                # gx :234 era-EMPTY — bringup counter (RELEASE half, fires)
}

# ---------------- emit --------------------------------------------------------
hdr = ("/* GENERATED by host/bringup/gen_ladder8_walk.py — P2-HG3 rung-6 \"the walk\".\n"
       " * Era sources READ-ONLY @ decomp root argv[1] (pin 6b38b53837); every slice\n"
       " * anchored by an exact-unique signature line, bodies re-guarded (fail-loud\n"
       " * before any write). Deviations L8-1 (main rename), L8-2 (loop-top AND),\n"
       " * 2 emitted harness lines, L8-5 (logo slot backings are bringup-side\n"
       " * counters; the REAL texture legs are the gx seat's, cited inline). */\n"
       '#include "ladder7_run_prelude.h"\n\n')
body = hdr
for name, text_, ln, src in frames:
    body += f"/* ---- verbatim {name} @ {src.split('/src/')[-1]}:{ln} ---- */\n#pragma push\n{text_}\n#pragma pop\n\n"

body += ("/* ---- HG3-272 S3: host-side main_phase_table copy. DATA generated;\n"
         f" * era text byte-validated against the 22-row block @{SRC['sbi'].split('/src/')[-1]}:{TAB_LN}\n"
         " * (drift = loud exit at generation). Slots: REAL = verbatim slice linked;\n"
         " * enter_* = class-(c)/(d) WITNESS stand-ins (record entry, never fake era\n"
         " * behavior); p8_fail_loud_main_phase = LB-3 unregistered-row canary; NULL\n"
         " * sentinels = era rows 0/17/20 verbatim. Mutable .data = era truth (the\n"
         " * era table is non-const); the HG3 test swaps stand-ins between runs. ---- */\n"
         "MainPhaseEntry main_phase_table[] = {\n")
for i, row in enumerate(ER_ROWS):
    ini, upd, ex = slots(row)
    fmt = lambda s: "NULL" if s is None else s
    body += f"    {{ {fmt(ini)}, {fmt(upd)}, {fmt(ex)} }},  /* [{i}] era: {row.strip()} */\n"
body += "};\n\n"

body += ("/* ---- HG3-273 S7: host copy of logo_load_callback_table. The 18 era\n"
         f" * names were PARSED from the era text @{slogo}:{LOGO_LN} this turn and\n"
         " * must equal the embedded roster (two-source guard above). Layout note\n"
         " * L8-4: call_funcs_reverse reinterprets the pairs as LobbyFrameEntry\n"
         " * {u32 id, fn} — the load halves ride the id slot and are NEVER called;\n"
         " * the odd slots (release halves) fire, back-to-front. ---- */\n"
         "void (*logo_load_callback_table[18])(void) = {\n")
for i in range(0, 18, 2):
    a, b = LOGO_ROSTER[i], LOGO_ROSTER[i + 1]
    body += (f"    {LOGO_BACK[a]}, {LOGO_BACK[b]},  /* pair {i//2}: era ({a}, {b}) */\n")
body += "};\n\n"

with open(OUT, "w") as fh:
    fh.write(body)

hdr2 = ("/* GENERATED by host/bringup/gen_ladder8_walk.py — P2-HG3 S18 phase-state\n"
        " * helper slices, class (a), SystemBootInit.cpp :157-176/:214-229 — a separate\n"
        " * TU so ladder7 (rung-5) binds run_frame's trap leg to the PROMOTED REAL\n"
        " * slice. Era sources READ-ONLY @ decomp root argv[1]; fail-loud anchors. */\n"
        '#include "ladder7_run_prelude.h"\n\n')
b2 = hdr2
for name, text_, ln, src in helps:
    b2 += f"/* ---- verbatim {name} @ {src.split('/src/')[-1]}:{ln} ---- */\n#pragma push\n{text_}\n#pragma pop\n\n"
with open(OUT2, "w") as fh:
    fh.write(b2)

for src in SRC.values():
    h = hashlib.sha256(open(src, "rb").read()).hexdigest()[:12]
    print(f"sha {h} {src.split('/src/')[-1]}")
for name, text_, ln, src in frames:
    print(f"slice: {name} @ {src.split('/src/')[-1]}:{ln} ({len(text_.splitlines())} lines)")
for name, text_, ln, src in helps:
    print(f"slice: {name} @ {src.split('/src/')[-1]}:{ln} ({len(text_.splitlines())} lines) [S18 TU]")
print(f"data: main_phase_table 22 rows (era block @{TAB_LN}, two-source byte-validated)")
print(f"data: logo_load_callback_table 18 ptrs / 9 pairs (era text @{slogo}:{LOGO_LN}, roster parsed+embedded cross-check)")
print(f"gen: {OUT} ({len(frames)} verbatim slices + 2 generated data tables) + {OUT2} ({len(helps)} S18 helper slices), zero hand-edit of generated files)")
