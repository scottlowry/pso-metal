#!/usr/bin/env python3
"""P2-L7 rung-5 PLAY-1 + PLAY-2 + PLAY-3 — guarded logo main-phase slice generator (ladder7).

Reads the READ-ONLY decomp tree (pso-decomp @ 6b38b5383, DECOMP_ROOT argv[1])
and emits build/bringup/gen/ladder7_runframe_glue.cpp: the FIVE promoted legs
of TLogo::run_frame (docs/p2-rung5-seams-1.md Play 1) plus the run_frame
carrier and request_main_phase, since PLAY-2 (docs/p2-ladder7-play23.md
§1.1) the three flag-spine seams L2/R3/R4 verbatim + the three era-empty
REAL bodies (render_after_scene :505, debug_render_overlay :533,
TObject::debug_render :174 — the §3 D6 promotion), and since PLAY-3 (§2.1)
the render_tick R9 verbatim body (:491-505, §3 D2 closure) + its 9 sliceable
sub-leg slices + 5 byte-gated dispatcher data anchors for the rung-5 EXIT —
same
slice_fn/slice_line anchor machinery as gen_ladder6_slices.py / the must_contain
guards of gen_ladder6_logo.py. Every anchor is exact-line-unique (fail-loud
before any write); the empty begin_frame body is byte-gated; each slice must
contain its named era statements. Zero bytes of the decomp tree are touched.

Usage: gen_ladder7_runframe.py <DECOMP_ROOT> <out.cpp>
"""
import sys, hashlib

ROOT, OUT = sys.argv[1], sys.argv[2]

def F(p): return f"{ROOT}/{p}"

SRC = {
    "tlogo":  F("src/pso/ui/TLogo.cpp"),
    "sbi":    F("src/pso/ui/keyboard/SystemBootInit.cpp"),
    "tobj":   F("src/pso/system/task/TObject.cpp"),
    "ctrl":   F("src/pso/system/ControllerInput.cpp"),
    "afr":    F("src/pso/system/AsyncFileRead.cpp"),
    "snd":    F("src/pso/audio/SoundRequests.cpp"),
    "lobby":  F("src/pso/unk/TLobby_800F8AA8.cpp"),
    "tgs":    F("src/pso/system/TGStream.cpp"),
    # ---- PLAY-3 (spec §2.1): render_tick home + its sub-leg era homes ----
    "rt":     F("src/pso/unk/TLobby_801E2F94.cpp"),   # sha bf97a50e79cb
    "er":     F("src/pso/effect/particle/EffectRender.cpp"),  # sha da8c01180088 (§3 D4)
    "sf":     F("src/pso/render/SceneFog.cpp"),
    "ru":     F("src/pso/render/render_util.cpp"),
    "gj":     F("src/pso/render/gjSystem.c"),
}

def lines(path):
    with open(path, "r") as fh:
        return fh.read().splitlines(keepends=True)

def slice_fn(src, sig, must_contain):
    """Verbatim slice: exact-unique signature line through first column-0 '}'.
    Every must_contain string must appear in the slice (fail-loud)."""
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

def slice_empty(src, sig):
    """Byte-gated empty-body era function: sig line, ONE blank line, column-0 '}'."""
    want = sig + "\n" + "\n" + "}\n"
    body, ln = slice_fn(src, sig, [])
    if body != want:
        sys.exit(f"GUARD: {SRC[src]}:{ln} not the era-empty 3-line body: {body!r}")
    return body, ln

def slice_line(src, exact):
    L = lines(SRC[src])
    idx = [i for i, l in enumerate(L) if l == exact + "\n"]
    if len(idx) != 1:
        sys.exit(f"GUARD: {SRC[src]}: line count {len(idx)}: {exact!r}")
    return exact + "\n", idx[0] + 1

# ---- the five promoted legs + carrier glue (Play 1; era homes are the RS
# §1 row refs; anchors re-opened from the pinned decomp this lane) ----
frames = []
def keep(name, body, ln, src):
    frames.append((name, body, ln, SRC[src]))

keep("TMainTask::begin_frame", *slice_empty("tobj", "void TMainTask::begin_frame() {"), "tobj")  # RS L1 (a): era-empty body, TObject.cpp:640-642
keep("poll_reset_request",
     *slice_fn("ctrl", "s32 poll_reset_request(void) {",
               ["DVDCheckDisk() == 0", "check_reset_button() != 0", "reset_system();",
                "memcard_access_active != 0", "controller_reset_requested;"]), "ctrl")           # RS R7 (b)
keep("process_async_file_reads",
     *slice_fn("afr", "void process_async_file_reads(void) {",
               ["switch (async_file_read_state) {", 'tmp_buf_use_req("FSVR_READPVM")',
                "ADXF_Open(async_file_read_current_request->name, 0)",
                "ADXF_GetStat(lobby_download_state_ptr) == 3",
                "LoadBmlExec(async_file_read_current_request->mode, async_file_read_current_request->name)",
                "async_file_read_stub();"]), "afr")                                               # RS R5 (b) — row-5 promoted, home TU parse-PASS @apply-7
keep("update_sound_request_slots",
     *slice_fn("snd", "void update_sound_request_slots(void) {",
               ["if (sound_requests_enabled != 0) {", "get_sound_voice_state(i);",
                "r == -1 || r == 1", "update_music_fade();"]), "snd")                             # RS R6 (b) guard leg
keep("TQuestVmDownLoadServer::is_ready",
     *slice_fn("lobby", "s32 TQuestVmDownLoadServer::is_ready() {",
               ["TLobby::wait_ready_loop();", "memcard_access_active != 0",
                "quest_download_in_progress != 0", "quest_download_server_ready ? 1 : 0;"]), "lobby")  # RS R8 (c) via LB-2 seam
# era tail callee of the async leg (empty one-line body, TGStream.cpp:203):
keep("async_file_read_stub",
     *slice_line("tgs", "void async_file_read_stub(void) {  }"), "tgs")
# carrier legs (same anchors gen_ladder6_logo.py pins gx-side):
keep("request_main_phase",
     *slice_fn("sbi", "void request_main_phase(s32 phase) {",
               ["main_phase_state.next = phase", "main_phase_state.result = 1"]), "sbi")
keep("TLogo::run_frame",
     *slice_fn("tlogo", "void TLogo::run_frame() {",
               ["begin_gx_frame();", "activate_camera_mode(9)", "main_task.begin_frame();",
                "main_task.render_screen_overlay()", "poll_reset_request()",
                "TQuestVmDownLoadServer::is_ready()", "render_tick();"]), "tlogo")

# ---- PLAY-2 (RS L2/R3/R4, class-(c)-as-(b), spec docs/p2-ladder7-play23.md §1.1):
# the main_task flag spine verbatim. All anchors re-opened from the pinned decomp
# this lane (era text read this turn; TObject sha 6586bb705b8f unchanged). ----
keep("TMainTask::run_task",
     *slice_fn("tobj", "void TMainTask::run_task() {",
               ["PAUSE_OR_RESUME_CHILD(update_flags, old_update_flags);",
                "render_pass = 0;", "task_flags |= MAIN_TASK_UPDATING;", "run_tasks();",
                "task_flags &= ~MAIN_TASK_UPDATING;", "task_flags |= MAIN_TASK_UPDATED;"]), "tobj")  # :283-290
keep("TMainTask::render",
     *slice_fn("tobj", "void TMainTask::render() {",
               ["ALLOW_OR_DISALLOW_CHILD(render_flags, old_render_flags, rendering);",  # :510 — spec §3 D1 re-pin
                "task_flags |= MAIN_TASK_RENDERING;", "render_pass = 0;",
                "render_geometry();", "render_effects();", "render_particle_effects();",
                "render_objects();", "render_after_scene();", "render_ui();",
                "task_flags &= ~MAIN_TASK_RENDERING;", "task_flags |= MAIN_TASK_RENDERED;"]), "tobj")  # :509-525
keep("TMainTask::render_screen_overlay",
     *slice_fn("tobj", "void TMainTask::render_screen_overlay() {",
               ["if (task_flags & MAIN_TASK_RENDERED) {", "render_clipout_and_fade();",
                "debug_render();", "debug_render_overlay();", "render_info_icon_overlay();",
                "task_flags &= ~(MAIN_TASK_UPDATED | MAIN_TASK_RENDERED);"]), "tobj")  # :644-652
# era-empty bodies REAL (RS Play 2 names two; the R4 callee list adds a third — spec §3 D6):
keep("TMainTask::render_after_scene",   *slice_empty("tobj", "void TMainTask::render_after_scene() {"),   "tobj")  # :505
keep("TMainTask::debug_render_overlay", *slice_empty("tobj", "void TMainTask::debug_render_overlay() {"), "tobj")  # :533
keep("TObject::debug_render",           *slice_empty("tobj", "void TObject::debug_render() {"),           "tobj")  # :174

# ---- PLAY-3 (RS R9, spec docs/p2-ladder7-play23.md §2.1): render_tick verbatim
# + its sliceable sub-legs. Every anchor re-opened from the pinned decomp this
# lane (rt sha bf97a50e79cb, er da8c01180088 — the §3 D4/D5 home re-pins;
# render_tick closure :505 — §3 D2; copy_efb body :52-57, present :59-61). ----
keep("render_tick",
     *slice_fn("rt", "void render_tick(void) {",
               ["TKeyBoardControl::run_tasks();", "TParticleCtrl::begin_effect_render();",
                "GXSetZMode(GX_TRUE, GX_ALWAYS, GX_FALSE);",
                "TCameraCtrl::restore_full_screen_view();", "set_cull_none();",
                "render_debug_text(lbl_805D3584);",
                "GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);",
                "GXSetViewport((0.0f), (0.0f), (6.4e+02f), (4.8e+02f), (0.0f), (1.0f));",
                "set_cull_none();", "TParticleCtrl::end_effect_render();",
                "copy_efb_to_framebuffer();", "present_frame();",
                "run_end_of_render_tasks();"]), "rt")                                # :491-505
keep("TKeyBoardControl::run_tasks",
     *slice_fn("sbi", "void TKeyBoardControl::run_tasks() {",
               ["TObjPlayer::clear_shadow_pass_request();", "update_global_sound_frame();",
                "if (sound_requests_enabled != 0) {", "update_music_sequencer();",
                "keyboard_task_object.run_tasks();"]), "sbi")                        # SystemBootInit.cpp:100-107 (§3 D5 — RS home wrong)
keep("run_end_of_render_tasks",
     *slice_fn("sbi", "void run_end_of_render_tasks(void) {",
               ["end_of_render_root.run_tasks();"]), "sbi")                          # :113-115
keep("TParticleCtrl::begin_effect_render",
     *slice_line("er", "void TParticleCtrl::begin_effect_render() { disable_scene_fog(); apply_scene_fog(); }"), "er")   # EffectRender.cpp:10 (§3 D4)
keep("TParticleCtrl::end_effect_render",
     *slice_line("er", "void TParticleCtrl::end_effect_render() { enable_scene_fog(); apply_scene_fog(); }"), "er")       # :12
keep("set_cull_none",
     *slice_line("ru", "void set_cull_none() { GXSetCullMode(GX_CULL_NONE); }"), "ru")                                # render_util.cpp:39
keep("enable_scene_fog",  *slice_fn("sf", "void enable_scene_fog() {",  ["scene_fog_enable_flag = 1;"]), "sf")          # SceneFog.cpp:26-28
keep("disable_scene_fog", *slice_fn("sf", "void disable_scene_fog() {", ["scene_fog_enable_flag = 0;"]), "sf")          # :30-32
keep("copy_efb_to_framebuffer",
     *slice_fn("gj", "void copy_efb_to_framebuffer(void) {",
               ["GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);", "GXSetColorUpdate(GX_TRUE);",
                "GXCopyDisp(video_current_framebuffer, GX_TRUE);", "GXDrawDone();"]), "gj")                            # gjSystem.c:52-57
keep("present_frame",
     *slice_fn("gj", "void present_frame(void) {", ["present_frame_and_wait_vsync();"]), "gj")                          # :59-61

# ---- rung-5 EXIT data anchors (spec §4): dispatcher table rows + the walk
# statements. rung-6 slices the dispatcher body itself; these are byte-gated
# era TEXT anchors only (fail-loud at generation, emitted as glue comments) —
# the harness's host-driver EXIT probe replicates :389-390 against them. ----
anchors = []
def anchor_line(src, exact, note):
    L = lines(SRC[src])
    idx = [i for i, l in enumerate(L) if l == exact + "\n"]
    if len(idx) != 1:
        sys.exit(f"GUARD: {SRC[src]}: anchor count {len(idx)} (want 1): {exact!r}")
    anchors.append((f"{SRC[src]}:{idx[0]+1}", note, exact))

anchor_line("sbi", "    { TLogo::init_logo_scene, TLogo::run_frame, TLogo::exit_logo_scene },",
            "main_phase_table[1] logo row — exit TLogo::exit_logo_scene (spec §4 :342)")
anchor_line("sbi", "    { TAdMultiChgGameModeGC::init_title_phase, run_game_mode_select_frame, TLobby::release_scene_resources },",
            "main_phase_table[2] title row (spec §4 :343)")
anchor_line("sbi", "    { TKeyBoardControl::update_main_menu_scene, run_main_frame, exit_main_menu_scene },",
            "main_phase_table[5] file-select row (spec §4 :346)")
anchor_line("sbi", "            main_phase_state.prev = main_phase_state.cur;",
            "dispatcher walk prev=cur (spec §4 :389)")
anchor_line("sbi", "            main_phase_state.cur = main_phase_state.next;",
            "dispatcher walk cur=next (spec §4 :390)")

hdr = ("/* GENERATED by host/bringup/gen_ladder7_runframe.py — P2-L7 rung-5 PLAY-1 + PLAY-2 + PLAY-3.\n"
       " * Era sources READ-ONLY @ decomp root argv[1]; every slice anchored by an\n"
       " * exact-unique signature line, body statements re-guarded (fail-loud before\n"
       " * any write). The five promoted legs + the PLAY-2 flag spine (L2/R3/R4 + the\n"
       " * three era-empty REAL bodies) + the PLAY-3 render_tick R9 decomposition (9\n"
       " * verbatim sub-leg slices, spec §2.1) EXECUTE era bodies; the seams they call\n"
       " * are ladder7_seam.cpp (prelude deviations L7-1..L7-3). */\n"
       '#include "ladder7_run_prelude.h"\n\n')
body = hdr
for name, text, ln, src in frames:
    body += f"/* ---- verbatim {name} @ {src}:{ln} ---- */\n#pragma push\n{text}\n#pragma pop\n\n"
body += ("/* ---- rung-5 EXIT era-text anchors (data-gated, NOT compiled; rung-6 slices\n"
         " * the dispatcher body — the harness host-driver replicates :389-390): ---- */\n")
for where, note, exact in anchors:
    body += f"/* anchor {where} — {note}\n *   {exact.strip()} */\n"
body += "\n"
with open(OUT, "w") as fh:
    fh.write(body)

for src in SRC.values():
    h = hashlib.sha256(open(src, "rb").read()).hexdigest()[:12]
    print(f"sha {h} {src.split('/src/')[-1]}")
for name, text, ln, src in frames:
    print(f"slice: {name} @ {src.split('/src/')[-1]}:{ln} ({len(text.splitlines())} lines)")
for where, note, _ in anchors:
    print(f"anchor: {where} {note}")
print(f"gen: {OUT} ({len(frames)} verbatim slices + {len(anchors)} data anchors, zero adapter edits)")
