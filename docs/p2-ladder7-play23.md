# P2-L7B — Rung-5 PLAY-2 / PLAY-3 executable spec (zero-build; feeds the NEXT builder)

Lane L7B, 2026-11-27 (card `t_028ebd34`). ZERO builds, ZERO `make`, writes limited to
this doc. Decomp READ-ONLY @ `6b38b5383` (worktree sha256 == `git show 6b38b5383:<f>`
for all 21 cited files, this turn; receipts `cache/scratch/l7b/anchors.json`,
generator `cache/scratch/l7b/verify_anchors.py` — python anchor-grep only). Waiver pin
`host/bringup/waiver-header.sha256` era `d4382fcd0659357f` untouched (no build = no
re-hash event; zero regen, per shared law).

Contract inputs: `docs/p2-rung5-seams-1.md` §1 rows L2/R3/R4/R9 + §2 Play 2/Play 3
(hereafter RS), `docs/p2-ladder7-1.md` (what PLAY-1 LANDED — the five promoted legs,
carrier L7-1, counted roster L7-2, seam split L7-3: NONE of those seams are re-specced
here), `docs/p2-bringup-1.md` (substrate), `docs/p2-rung4-contract.md` rows 10–12
(dispatcher, cited only for the EXIT section).

Classification note: RS §1 tables L2/R3/R4/R9 as class **(c)** (callee is frame-pump /
pixel / present seam), and RS's own play text carves them exactly the class-(b) way —
verbatim body + prelude externs; no LB-1/LB-2 crossing inside the sliced line range.
This spec carves all five seams that way and says (c)-as-(b) wherever it classifies.

Everything below is paste-ready for `host/bringup/gen_ladder7_runframe.py` (same
`slice_fn`/`slice_empty`/`slice_line`/`keep` machinery, PLAY-1 receipt §1) +
`include/ladder7_run_prelude.h` + `ladder7_seam.cpp` + `ladder7_test.cpp`, and mirrors
the insert-only `bringup.mk` ladder7 block (rules at bringup.mk:395–414). Generator
replay-check rule (byte-identical second run, PLAY-1 §2) and D8 pre/post re-hash apply
unchanged to every build this spec proposes.

---

## 1. PLAY-2 — "main_task flag spine": seams L2 + R3 + R4 (1–2 builds)

Era homes, all verified byte-stable @`6b38b5383` this turn (TObject.cpp sha `6586bb705b8f`
== the PLAY-1 build's read, L7 doc §1):

| seam | carrier call (TLogo.cpp) | era home | body | closure |
|---|---|---|---|---|
| L2 | `main_task.run_task()` :103 | `src/pso/system/task/TObject.cpp:283–290` | 8 lines, sha256[:12] `b0c2df4fef9b` | `}` :290 |
| R3 | `main_task.render()` :104 | `TObject.cpp:509–525` | 17 lines, `7e3b3115e014` | `}` :525 |
| R4 | `main_task.render_screen_overlay()` :105 | `TObject.cpp:644–652` | 9 lines, `73a4fb1ade56` | `}` :652 |

### 1.1 Anchor blocks (append to the `frames` block of gen_ladder7_runframe.py, after the PLAY-1 roster — the four Play-2/3 counter seams leave `ladder7_seam.cpp` as their slices land, §1.5)

```python
SRC["rt"] = F("src/pso/unk/TLobby_801E2F94.cpp")  # (PLAY-3, §2)

# ---- PLAY-2 (RS L2/R3/R4, class-(c)-as-(b)): flag spine verbatim ----
keep("TMainTask::run_task",
     *slice_fn("tobj", "void TMainTask::run_task() {",
               ["PAUSE_OR_RESUME_CHILD(update_flags, old_update_flags);",
                "render_pass = 0;", "task_flags |= MAIN_TASK_UPDATING;", "run_tasks();",
                "task_flags &= ~MAIN_TASK_UPDATING;", "task_flags |= MAIN_TASK_UPDATED;"]), "tobj")
keep("TMainTask::render",
     *slice_fn("tobj", "void TMainTask::render() {",
               ["ALLOW_OR_DISALLOW_CHILD(render_flags, old_render_flags, rendering);",
                "task_flags |= MAIN_TASK_RENDERING;", "render_pass = 0;",
                "render_geometry();", "render_effects();", "render_particle_effects();",
                "render_objects();", "render_after_scene();", "render_ui();",
                "task_flags &= ~MAIN_TASK_RENDERING;", "task_flags |= MAIN_TASK_RENDERED;"]), "tobj")
keep("TMainTask::render_screen_overlay",
     *slice_fn("tobj", "void TMainTask::render_screen_overlay() {",
               ["if (task_flags & MAIN_TASK_RENDERED) {", "render_clipout_and_fade();",
                "debug_render();", "debug_render_overlay();", "render_info_icon_overlay();",
                "task_flags &= ~(MAIN_TASK_UPDATED | MAIN_TASK_RENDERED);"]), "tobj")
# era-empty bodies REAL (RS Play 2 names two; the R4 callee list adds a third — §3 D6):
keep("TMainTask::render_after_scene",   *slice_empty("tobj", "void TMainTask::render_after_scene() {"),   "tobj")  # :505
keep("TMainTask::debug_render_overlay", *slice_empty("tobj", "void TMainTask::debug_render_overlay() {"), "tobj")  # :533
keep("TObject::debug_render",           *slice_empty("tobj", "void TObject::debug_render() {"),           "tobj")  # :174
```

`slice_empty` byte-gates sig + one blank + col-0 `}` — all three verified in that exact
shape (`TObject.cpp:505–507`, `:533–535`, `:174–176`).

### 1.2 Prelude additions (ladder7_run_prelude.h) — macro grammar + carriers

The R3 body needs the `ALLOW_OR_DISALLOW_CHILD` statement at `:510` that RS's row-3 text
missed (§3 D1). The macro trio is `include/pso/system/task/TMainTask.h:17–38`
(byte-stable, sha `2865c241009f`); the walk iterator is `include/pso/macros.h:89–90`.
Paste verbatim into the prelude (NOT `#include "pso/system/task/TMainTask.h"` — that
header drags `TL_OBJECTS` externs + the full TObject include graph, the D4/L7-1
keep-the-graph-off rule), replacing the §112–117 carrier block:

```cpp
/* ---- PLAY-2: TMainTask.h:17–38 verbatim + macros.h:89–90 iterator ---- */
#define FOREACH_NODE_NODECL_MULTI_ITER(type, first, varname, ...) \
    for (varname = (type *)(first); varname != NULL; varname = (type *)(varname->m_next), __VA_ARGS__)
#define X_OR_Y_CHILD(flags, old_flags, one_prefix, zero_prefix, suffix) \
    if (flags != old_flags) { \
        TMainTask *child; \
        u32 bit = 1; \
        FOREACH_NODE_NODECL_MULTI_ITER(TMainTask, main_task.m_down, child, bit <<= 1) { \
            if (flags & bit) { \
                child->one_prefix##_##suffix(); \
            } else { \
                child->zero_prefix##_##suffix(); \
            } \
        } \
        old_flags = flags; \
    }
#define PAUSE_OR_RESUME_CHILD(flags, old_flags) \
    X_OR_Y_CHILD(flags, old_flags, pause, resume, update)
#define ALLOW_OR_DISALLOW_CHILD(flags, old_flags, flag_name) \
    X_OR_Y_CHILD(flags, old_flags, disallow, allow, flag_name)

/* main_task_flags TMainTask.h:80–89 (UPDATING 0x001 :81, RENDERING 0x002 :82,
   UPDATED 0x080 :86, RENDERED 0x100 :87); TObject m_flags bits TObject.h:29–36
   (PAUSE_UPDATE 0x0008 :32, DISALLOW_RENDER 0x0010 :34). */
enum { PAUSE_UPDATE = 0x0008, DISALLOW_RENDER = 0x0010 };

/* carriers: inline method bodies verbatim from TObject.h:119–122 */
struct TObject {
    u32 m_flags; TObject *m_prev, *m_down, *m_next;
    void run_tasks();                       /* COUNTED extern (§1.3) — era walk :116 */
    void debug_render();                    /* REAL era-empty slice (TObject.cpp:174) */
    void allow_rendering()    { m_flags &= ~DISALLOW_RENDER; }
    void disallow_rendering() { m_flags |=  DISALLOW_RENDER; }
    void resume_update()      { m_flags &= ~PAUSE_UPDATE; }
    void pause_update()       { m_flags |=  PAUSE_UPDATE; }
};
struct TMainTask : TObject {
    u32 task_flags;                         /* TObject.cpp member; era def :33 */
    void begin_frame();                     /* REAL slice — PLAY-1, unchanged */
    void run_task(); void render(); void render_screen_overlay();  /* REAL slices §1.1 */
    void render_geometry(); void render_effects(); void render_particle_effects();
    void render_objects();  void render_ui();      void render_clipout_and_fade();
                                            /* COUNTED externs (§1.3) — homes below */
    void render_after_scene(); void debug_render_overlay();        /* REAL era-empty  */
};
extern TMainTask main_task;                 /* already declared (PLAY-1) */
extern TObject end_of_render_root, keyboard_task_object;  /* defs §1.3 */
void render_info_icon_overlay(void);        /* COUNTED — free fn, TLobby_803382E4.cpp:145 */
extern u32 render_pass, update_flags, old_update_flags, render_flags, old_render_flags;
```

### 1.3 Host zero-defs + counted externs (ladder7_seam.cpp — ADD, and REMOVE the four
`TMainTask::run_task/render/render_screen_overlay` + `render_tick` counter bodies at
:73–76; `l7_call_names` and the enum extend in lock-step)

```cpp
/* ---- PLAY-2 flag-spine globals (era homes verified this turn) ---- */
u32 render_pass = 1;      /* .data — era INIT is 1 (TObject.cpp:25). ZERO-INIT IS DRIFT (§3 D7) */
u32 update_flags;         /* TObject.cpp:53 (.bss zero-def) */
u32 old_update_flags;     /* :54 */
u32 render_flags;         /* :55 */
u32 old_render_flags;     /* :56 */
TObject end_of_render_root;      /* TObject.cpp:35 */
TObject keyboard_task_object;    /* TObject.cpp:37 — PLAY-3 roots, def here once */

/* ---- counted tree-walk stand-in (LB-2 roster joins: RS §3 — reuse, no new mechanism;
   era body TObject::run_tasks @TObject.cpp:116 walks child run_task/render bodies,
   all lane-J homes -> the counter is the rung-5-faithful backing) ---- */
void TObject::run_tasks() {
    g_walk_calls++;
    g_walk_root = (this == (TObject *)&main_task) ? WALK_MAIN
              : (this == &keyboard_task_object)   ? WALK_KBD : WALK_EOR;
    g_walk_flags = task_flags;                    /* capture the caller's LIVE flags */
    l7_note(LC7_WALK_MAIN + (g_walk_root - WALK_MAIN));
}
/* ---- counted pixel sub-passes (RS Play 2: the 8 stay counters; pixels keep riding
   LOGO-3's single present). Each records the LIVE render_pass/task_flags: ---- */
static void l7_subpass(int ev, int *ctr) { l7_note(ev); (*ctr)++;
    g_pass_trace[g_pass_n < 16 ? g_pass_n++ : 15] = (u8)render_pass;
    g_flags_at_subpass = main_task.task_flags; }
void TMainTask::render_geometry()        { l7_subpass(LC7_GEOM, &g_geom); }
void TMainTask::render_effects()         { l7_subpass(LC7_EFF,  &g_eff);  }
void TMainTask::render_particle_effects(){ l7_subpass(LC7_PTEX, &g_ptex); }
void TMainTask::render_objects()         { l7_subpass(LC7_OBJ,  &g_obj);  }
void TMainTask::render_ui()              { l7_subpass(LC7_UI,   &g_ui);   }
void TMainTask::render_clipout_and_fade(){ l7_subpass(LC7_CLIP, &g_clip); }
void render_info_icon_overlay(void)      { l7_subpass(LC7_INFO, &g_info); }
```

Sub-pass homes (all `void TMainTask::X() {` unique-in-file, verified): geometry :399,
effects :432, particle_effects :449, ui :460, objects :477, clipout_and_fade :292
(body :292–313, pixel-chain callee set — count, do NOT slice);
`render_info_icon_overlay` is a FREE function @ `src/pso/unk/TLobby_803382E4.cpp:145`
(lane-J `(d)` home; RS R4 row).

### 1.4 LB-carve needs (PLAY-2)

- **LB-2 (counted seams)**: the 8 above join the L7-3 counted roster — the same LOGO-2
  shape PLAY-1 shipped; the tree-walk extern is the SINGLE `TObject::run_tasks` backing
  all three roots (RS §3: R8's `wait_ready_loop` precedent — reuse, don't re-mint).
- **LB-3 (fail-loud)**: none new. `switch_to_fallback_main_phase` stays fail-loud
  (poll==0 era-zero chain, L7-3).
- No era header joins the prelude graph (L7-1 holds): the sliced bodies touch only
  `task_flags` + the five .data/.bss globals + the macros' carrier methods.

### 1.5 Flip-probe plan (ladder7_test.cpp — probes L7-P5..P9; era stores are what the CHECKs read)

| probe | action (write era globals as era code would) | read-back through the REAL slices |
|---|---|---|
| P5 macro fire | link two `TMainTask` dummies `main_task.m_down → n0 → n1`; `update_flags=0x1` (old stays 0); call `main_task.run_task()` | `n0.m_flags & PAUSE_UPDATE` set (bit 1 fire), `n1` got `resume_update` (else leg); `old_update_flags == update_flags` (macro tail store). Second call with flags unchanged → NO method fires (gate `flags != old_flags` real) |
| P6 L2 spine | from era-zero, call `main_task.run_task()` | walk extern saw `task_flags & MAIN_TASK_UPDATING` LIVE at `:287`; after: `task_flags == MAIN_TASK_UPDATED` (0x080), `UPDATING` cleared, `render_pass == 0` (`:285`) |
| P7 R3 spine | set `task_flags = MAIN_TASK_UPDATED`; call `render()` | trace at sub-pass entries: `render_geometry` sees RENDERING set (`:511`) with `g_pass_trace == [0,1,0,1,0]` (geom/eff/ptex/obj/ui, `:512–520` — RS What-green); after: `task_flags == UPDATED|RENDERED` (0x180), RENDERING cleared (`:523–524`) |
| P8 R4 guard honesty | (a) `task_flags=0` → `render_screen_overlay()`; (b) `task_flags=UPDATED|RENDERED` → same | (a) `g_clip/g_info` +0 — guard `:645` false through the REAL load; (b) clip+info fire once each (debug_render/debug_render_overlay/after_scene execute their REAL empty bodies — unobservable by design, L7 §3 argument), and `task_flags == 0` after (`:651` real clear) |
| P9 full-frame chain | `TLogo::run_frame()` from era-zero | `g_info` extern observed `task_flags & MAIN_TASK_RENDERED` nonzero (flag-honesty: the guard opened because of `:524`'s REAL store, not stub-honesty); end-of-frame `task_flags == 0`; per-frame sub-pass order == `:513–522` text order |

**Invariants to read back (RS Play-2 What-green, spelled as checks L7-I2a..e):**
`I2a` per-frame `render_pass` trace `== {0}` ∪ (run_task `:285`) `⟨0,1,0,1,0⟩` (render
`:512–520`), i.e. RS's `{0,1,0,1,0(,0)}` — the trailing 0 is the next frame's `:285`;
`I2b` `MAIN_TASK_RENDERED` set ONLY by `:524`, consumed ONLY at `:645`, cleared ONLY by
`:651` — flag bits outside the enum universe (`~0x3FF` mask over all flags) never move;
`I2c` `g_walk_calls` per `run_frame` == 1 and root == MAIN (until PLAY-3 adds the two
render_tick roots); `I2d` `update_flags==old_update_flags==render_flags==old_render_flags==0`
outside P5-style probes (RS "all zero on host = era default" holds for the .bss quartet);
`I2e` ALL four PLAY-1 invariants (`L7-I0a..e` set) re-read zero after every frame — the
flag spine changes nothing behind the promoted five.

---

## 2. PLAY-3 — "render_tick leg decomposition": seam R9 (2 builds)

Era home `src/pso/unk/TLobby_801E2F94.cpp` (sha `bf97a50e79cb`): `render_tick` @
**:491–505** (RS cites 491–506; `}` is `:505` — §3 D2). 13 sub-legs @ `:492–504` — RS
enumerated 12 names; the 13th is `TCameraCtrl::restore_full_screen_view()` @ `:495`
(§3 D3). Body sha256[:12] `9301982ede5d`, 15 lines, col-0 closure verified.

### 2.1 Anchor blocks

```python
# ---- PLAY-3 (RS R9): render_tick verbatim + its sliceable sub-legs ----
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
                "keyboard_task_object.run_tasks();"]), "sbi")                        # SystemBootInit.cpp:100-107 (RS home wrong — §3 D5)
keep("run_end_of_render_tasks",
     *slice_fn("sbi", "void run_end_of_render_tasks(void) {",
               ["end_of_render_root.run_tasks();"]), "sbi")                          # :113-115
keep("TParticleCtrl::begin_effect_render",
     *slice_line("er", "void TParticleCtrl::begin_effect_render() { disable_scene_fog(); apply_scene_fog(); }"), "er")   # EffectRender.cpp:10 (RS home wrong — §3 D4)
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
```

New `SRC` keys: `"rt"` (TLobby_801E2F94), `"er"` (effect/particle/EffectRender.cpp),
`"sf"` (render/SceneFog.cpp), `"ru"` (render/render_util.cpp), `"gj"` (render/gjSystem.c).
All signature lines exact-unique-verified this turn; the `sbi` TU gains its third slice
(PLAY-1 already cuts `request_main_phase` @SystemBootInit.cpp:219 from it).

### 2.2 Per-leg roster — class · carve (13 legs, era order)

| # | : | leg | class | host backing |
|---|---|---|---|---|
| 1 | 492 | `TKeyBoardControl::run_tasks` | (b) | VERBATIM slice (8 lines) — externs: `TObjPlayer::clear_shadow_pass_request` counted (static, `TObjPlayer.h:1105`; era body CameraCapture.cpp:384 is a static store — count, don't drag the graph); `update_global_sound_frame` counted (`SoundPlayback.h:27`/`.cpp:232`, musyx chain); `update_music_sequencer` **LB-3 fail-loud** (era-unreachable: `sound_requests_enabled==0` — guard `:103` real, same global PLAY-1 zero-defs); `keyboard_task_object.run_tasks()` → SHARED counted walk extern (§1.3, root=KBD) |
| 2 | 493 | `TParticleCtrl::begin_effect_render` | (b) | VERBATIM one-liner; callees: `disable_scene_fog` VERBATIM slice (real store `scene_fog_enable_flag=0`), `apply_scene_fog` counted (`SceneFog.cpp:53` body reads fog globals + `GXSetFog` — (d) pixel chain) |
| 3 | 494 | `GXSetZMode(TRUE, ALWAYS, FALSE)` | (b) | REAL capture seam `GXSetZMode` (args logged) |
| 4 | 495 | `TCameraCtrl::restore_full_screen_view` | (d) | COUNTED stand-in — era body TCameraCtrl.cpp:1148–1156 writes camera class statics (lane-J home; `camera_ctrl_instance` era NULL on host, L7-1). RS's enumeration missed this leg (§3 D3) |
| 5 | 496 | `set_cull_none` | (b) | VERBATIM one-liner → `GXSetCullMode` capture seam |
| 6 | 497 | `render_debug_text(lbl_805D3584)` | (d) | COUNTED, **arg-captured** — era DebugFont.cpp:140 body is a full GX text chain; `lbl_805D3584` is `s32 lbl_805D3584 = 1;` (TLobby_801E2F94.cpp:108, .data INIT=1 — §3 D7: the era default is debug text ENABLED; host def must be `= 1`, capture must assert `== 1`) |
| 7 | 498 | `GXSetZMode(TRUE, LEQUAL, TRUE)` | (b) | capture seam (arg-trace leg) |
| 8 | 499 | `GXSetViewport(0,0,640,480,0,1)` | (b) | capture seam — assert w==640 h==480 (`6.4e+02f/4.8e+02f` literals verified `:499`) |
| 9 | 500 | `set_cull_none` #2 | (b) | same slice — counter MUST reach 2/frame |
| 10 | 501 | `TParticleCtrl::end_effect_render` | (b) | VERBATIM one-liner; `enable_scene_fog` VERBATIM (real store `=1`), `apply_scene_fog` counted #2 |
| 11 | 502 | `copy_efb_to_framebuffer` | (b) | VERBATIM slice (4 lines) → `GXSetColorUpdate`/`GXCopyDisp`/`GXDrawDone` capture seams; `video_current_framebuffer` host zero-def (`gjSystem.c:23`, .bss void*) |
| 12 | 503 | `present_frame` | (c) | VERBATIM slice (3 lines) → `present_frame_and_wait_vsync` = the SINGLE-present seam: on bringup a counter `g_present_calls` (must stay 1/render_tick — RS §3: "a second `present_frame` must never appear"); on the gx tree it rides the EXISTING LOGO-3 backing (`host/gx/dl/gx_logo_driver.cpp:190` host `render_tick` + its present path) — build-2 WRAPS it, never re-mints |
| 13 | 504 | `run_end_of_render_tasks` | (b) | VERBATIM slice (3 lines) → SHARED counted walk extern (root=EOR; era roots TObject.cpp:35/:37 host zero-def, `m_down=NULL` — static-ctor registrants (TNECFontEORFunction.cpp:69, TFogCtrl.cpp:614) are not linked, matching the era-empty-walk claim) |

### 2.3 Host zero-defs / prelude additions (PLAY-3)

```cpp
/* zero-def (§1.3 already carries end_of_render_root / keyboard_task_object) */
s32 lbl_805D3584 = 1;          /* .data INIT 1 — TLobby_801E2F94.cpp:108 (D7) */
s8  scene_fog_enable_flag;     /* .bss zero — SceneFog.cpp:13 */
void *video_current_framebuffer; /* .bss zero — gjSystem.c:23 */
/* seam decls (bodies ladder7_seam.cpp): */
enum { GX_FALSE = 0, GX_TRUE = 1 };  enum { GX_NEVER, GX_LESS, GX_LEQUAL, GX_EQUAL, GX_GEQUAL };
enum { GX_CULL_NONE = 0 };           /* host-local names — values only ever logged, never compared to hardware */
void GXSetZMode(s32 test, s32 comp, s32 mask);
void GXSetViewport(float l, float t, float w, float h, float n, float f);
void GXSetCullMode(s32 mode);  void GXSetColorUpdate(s32 on);
void GXCopyDisp(void *fb, s32 clear); void GXDrawDone(void);
void present_frame_and_wait_vsync(void);
void update_global_sound_frame(void); void update_music_sequencer(void);
struct TObjPlayer { static void clear_shadow_pass_request(); };  /* carrier: method only */
struct TKeyBoardControl { static void run_tasks(); };            /* TKeyBoardControl.h:36 static — verified */
struct TParticleCtrl { static void begin_effect_render(); static void end_effect_render(); }; /* TParticleCtrl.h:56-57 static — verified */
void render_debug_text(u32 enabled);   /* COUNTED arg-capture */
void TCameraCtrl_restore_full_screen_view_shim(void);  /* COUNTED; static-class call — declare `struct TCameraCtrl` carrier gains `static void restore_full_screen_view();` */
```

Prelude `enum L7Call` extends: drop `LC7_MT_RUN/MT_RENDER/MT_OVERLAY/RENDER_TICK` (the
four seams this spec slices away), add `LC7_WALK_MAIN/KBD/EOR, LC7_GEOM, LC7_EFF,
LC7_PTEX, LC7_OBJ, LC7_UI, LC7_CLIP, LC7_INFO` + `LC7_RT1..LC7_RT13` (sub-leg notes fire
at legs 1,2,3,4,5,6,7,8,9,10,11,12,13 → legs whose backing is a verbatim slice fire at
their first INNER counted/capture extern, exactly the LB-2 `LC7_READY` precedent);
`g_call_seq[64]` → `[160]` (expected ≈21 events/frame post-PLAY-2/3) and
`FRAME_EVENTS`/`L7-R3b` expected vector rebuilt to `TLogo.cpp:95–114` text order with
the sub-leg expansions inline.

### 2.4 Flip probes (L7-P10..P12) + invariants (L7-I3a..d)

- **P10 fog flag**: call `TParticleCtrl::begin_effect_render()` → `scene_fog_enable_flag
  == 0` REAL store read-back; `end_effect_render()` → `== 1`; `g_applyfog` reaches +2 per
  call pair (counted apply sees the flag both ways).
- **P11 GX arg trace**: one `render_tick()` → `g_zmode_trace == [(1,ALWAYS,0),(1,LEQUAL,1),
  (1,LEQUAL,1)]` (legs 3,7, copy_efb's internal), `g_vp_calls==1 ∧ (w,h)==(640,480)`,
  `g_cull_calls==2 ∧ mode==GX_CULL_NONE`, `g_dbgtext_arg==1`, `g_present_calls==1`.
- **P12 kbd guard**: `sound_requests_enabled=1` set, `run_frame` once → `update_music_sequencer`
  LB-3 fires (exit 9) — proves the guard is the REAL `:103` load; restore 0, all four
  PLAY-1 invariants re-read zero.
- **I3a** sub-leg order log per frame == `TLobby_801E2F94.cpp:492–504` text order
  (RS What-green, restated against the era-text range — see D2); **I3b** present exactly
  once per render_tick (gx tree: viewport recorded twice per frame — begin_gx_frame +
  render_tick — and L7-P1/P2/P3 pixel gates under `host/gx/run_window.sh` stay
  byte-identical, RS Play-3 What-green); **I3c** walk roots per frame ==
  {MAIN (run_task), KBD (leg 1), EOR (leg 13)} exactly once each; **I3d** fog flag == 1
  at every frame boundary (end_effect_render's era store restores it — the apply/copy
  legs must not leak state).

Build split: **B1** = §1 (PLAY-2 whole) — 1–2 builds; **B2** = §2 bringup-side roster;
**B3** = gx-side wrap of B2's shape in `gx_logo_driver.cpp` + pixel-gate re-check (2
builds total for Play 3, matching RS §2). Builder seat per wave law owns every make;
D8 pin re-hash pre/post every make, zero regen.

---

## 3. Drift vs RS's reading (all anchors byte-verified at 6b38b5383; drift = RS-text, not bytes)

121 anchors checked (exact-line + uniqueness + byte-gates + worktree==blob): **121
verified / 7 drifted-vs-RS** — none is era-body *change* (RS read the same pinned tree);
all seven are RS enumeration/home omissions this spec re-pins:

1. **R3 row omits the `:510` macro statement** — `ALLOW_OR_DISALLOW_CHILD(render_flags,
   old_render_flags, rendering)` is in the body (verified); RS cited only L2's
   PAUSE_OR_RESUME twin. Prelude grammar §1.2 covers both.
2. **R9 range cited 491–506** — era closure is `:505`; text-order anchor for the
   order-log invariant is `:492–504` (13 statements).
3. **R9 enumerates 12 of its own 13 legs** — `TCameraCtrl::restore_full_screen_view()`
   @`:495` is unnamed in RS §1 row R9 (it is leg 4 here, counted).
4. **`TParticleCtrl::begin/end_effect_render` home mis-cited** as
   `effect/particle/TParticle.cpp` (that's the failing TU RS quotes err 2162B against) —
   era defs are `effect/particle/EffectRender.cpp:10/:12`, one-liners (sha `da8c01180088`).
5. **`TKeyBoardControl::run_tasks` home mis-cited** as `TKeyBoardControl_80290CB0.cpp` —
   era def is `ui/keyboard/SystemBootInit.cpp:100–107` (unique-in-file verified; the
   slice TU already hosts `request_main_phase`, so this ADDS NO new include graph).
6. **RS's "slice real (both empty)" undercounts** — R4's `debug_render()` callee is
   `TObject::debug_render` @TObject.cpp:174–176, a THIRD era-empty body in the same
   pattern as `render_after_scene`/`debug_render_overlay` — promoted to verbatim slice.
7. **"all zero on host = era default" fails twice in PLAY-2/3** — `render_pass = 1`
   (.data init, TObject.cpp:25) and `lbl_805D3584 = 1` (.data init,
   TLobby_801E2F94.cpp:108) must host-DEF WITH their era initializers or the first frame
   diverges from era truth (render_pass trace; debug-text leg enabledness).
   Additionally: the L2/R3/R4/R9 rows are class (c) in RS's own table — "3 class-b
   seams" shorthand in the ladder plan is honored here as (c)-as-(b), RS §2's own carve.

PLAY-1 receipts cross-check clean: TObject `6586bb705b8f`, TLogo `13889e46dcaa`,
SystemBootInit `ed382a8ae2f8`, ControllerInput `f84360c2afc5` all re-measured identical
this turn (L7 doc §1) — nothing under the already-landed slices moved.

---

## 4. Rung-5 EXIT condition

**Definition (RS §0/§2, settled):** rung-5 exits when PLAY-1 + PLAY-2 + PLAY-3 are all
landed green under `run.sh`: all 13 legs of `TLogo::run_frame` (`TLogo.cpp:95–114`)
execute era bodies — 5 promoted (PLAY-1, landed) + 3 flag-spine (PLAY-2) + 1 verbatim
render_tick with a 13-leg era-ordered roster (PLAY-3) — with the remaining crossings
counted behind RS-tabled seams, and the flag spine honest end-to-end: `MAIN_TASK_UPDATED`
(`:289`) → consumed by nothing but re-armed, `MAIN_TASK_RENDERED` (`:524`) → consumed
(`:645`) → cleared (`:651`) by REAL loads/stores every frame.

**Legs remaining AFTER PLAY-2/3 (each named, none gates the exit — RS §2 "then lane J"):**
P1 `activate_camera_mode` (TCameraCtrl.cpp:722), P2 `set_active_camera_pose` (:796),
P3 `controller_stuff` (ControllerInput.cpp:278+), P4 `update_keyboard_input_frame`
(:268–274) — all (d), lane J first blocker `NJS_OBJECT_size` @NinjaTypes.h:162; the 8
counted sub-pass BODIES (geometry/effects/particle/objects/ui/clipout homes
TObject.cpp:399/432/449/477/460/292, info-icon TLobby_803382E4.cpp:145 — lane J pixel
chain); the real `TObject::run_tasks` child bodies; R6 full body (flag-gated, lanes
J + M); `wait_ready_loop`'s disk-error pump path; render_tick legs 4 (camera statics)
and 6 (DebugFont) internals; the `main()`/dispatcher chain itself (rung4 rows 10–12:
SystemBootInit.cpp:365–395 + `TLogo::exit_logo_scene` TLogo.cpp:117–121 — rung-6, it
sits OUTSIDE the 13-leg ledger).

**First visible behavior = the title→file-select transition; evidence shape (all
anchors verified this turn):**
- Era chain: logo `TLogo::run_task` case 7 (`TLogo.cpp:193–195`) calls
  `request_main_phase(0xA)` through the REAL PLAY-1 slice →
  `main_phase_state.next==0xA ∧ result==1` read-back (the L7-I1f "never fired" zero-state
  flips to a real move); dispatcher (`SystemBootInit.cpp:385–391`, rung-6's first slice)
  does `prev=cur; cur=next` and fires `main_phase_table[1].exit`
  (`TLogo::exit_logo_scene`, table :342); file-select is
  `main_phase_table[5] = { TKeyBoardControl::update_main_menu_scene, run_main_frame, … }`
  (:346; entry call-site `request_main_phase(5)` verified @
  TAdSinglePlyChrSelectGC.cpp:107; title is table[2] `init_title_phase` @:343).
- **Evidence shape** (what the rung-6 pixel/bringup receipts must show, in order):
  1. transcript: exactly one `request_main_phase` fire per transition with read-back
     `{next, result}` from the real slice, and `main_phase_state.cur` stepping
     1→0xA→…→2 (title)→5 (file-select) with `prev` consistent (`:389–390`);
  2. call-order log: each transition frame closes with the PLAY-2/3 invariant vector
     intact (`task_flags==0` at frame boundary, `render_pass` trace §1.5 I2a,
     present-count 1 — the spine stays honest across the phase swap);
  3. exit-table fire `TLogo::exit_logo_scene` exactly once (delete_layer_children →
     callback-reverse → render_tick tail, TLogo.cpp:118–120 order);
  4. pixel gate (`host/gx/run_window.sh`, L7-P1/P2/P3 lineage): the first present frame
     after `cur==5` byte-differs from the last title frame inside the window rect —
     the file-select list nodes arriving through the `tl_window` chain — with present
     count STILL exactly one per frame (single-present invariant, RS §3).

---

## 5. Method / honesty

Every `file:line` above was opened this turn from the pinned tree by
`cache/scratch/l7b/verify_anchors.py` (21 files, blob==worktree sha256, 121 exact-line /
uniqueness / byte-gate checks, 0 failures — the 4 enum line-number misses in my first
draft pass were corrected against the file, not assumed; final run 121/121). No compiler
invoked, no `make`, no writes outside this doc and the scratch dir. Line refs are decomp
line numbers; no `.s` offsets re-cited (PLAY-1 §3 rule). Statics (`TKeyBoardControl::
run_tasks`, `TParticleCtrl::*_effect_render`, `TObjPlayer::clear_shadow_pass_request`)
verified against their era headers' `static` decls before any carrier was drafted.
