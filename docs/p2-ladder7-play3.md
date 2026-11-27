# P2-L7P3 — Rung-5 PLAY-3 receipts: render_tick R9 decomposition EXECUTES era bodies

Lane L7P3 (sole bringup builder), 2026-11-27 (card `t_449e0175`). Contract:
`docs/p2-ladder7-play23.md` §2 (PLAY-3) + §4 (rung-5 EXIT evidence shape) over
`docs/p2-rung5-seams-1.md` §1 row R9 (RS); inputs `docs/p2-ladder7-play2.md`
(PLAY-2 landed: roots, enum slots, order-log machinery). Decomp READ-ONLY @
`6b38b53837`: zero tracked modifications verified after every step. Waiver pin
`d4382fcd0659357f283e675726c1076338e909550c067513459280025e9f0eee`
(`host/bringup/waiver-header.sha256`) re-hashed PRE and POST every make event
this lane (7 make invocations): pin every time, bytes never moved, zero regen.
Writes: `host/bringup/**` + this doc only; no `make -C build parse*/gen_waivers`;
no `host/gx/**` touched (sibling ORCL seat builds gx/dl concurrently).

## 1. What landed (per-seam commits 3fd635b / d3e7c69 / c81d4f6 / 00f9c02)

| seam | landing | proof |
|---|---|---|
| slices (gen_ladder7_runframe.py, spec §2.1) | `render_tick` TLobby_801E2F94.cpp:491–505 VERBATIM (closure `:505` — §3 D2; body sha `9301982ede5d`; all 13 statements must_contain) + the 9 sliceable sub-legs: `TKeyBoardControl::run_tasks` SystemBootInit.cpp:100–107 (§3 D5), `run_end_of_render_tasks` :113–115, `begin/end_effect_render` EffectRender.cpp:10/:12 (§3 D4, file sha `da8c01180088`), `set_cull_none` render_util.cpp:39, `enable/disable_scene_fog` SceneFog.cpp:26–28/:30–32 (REAL stores), `copy_efb_to_framebuffer` gjSystem.c:52–57, `present_frame` :59–61; + 5 byte-gated dispatcher DATA anchors (:342/:343/:346 rows, :389/:390 walk stmts — fail-loud at generation, emitted as glue comment block, NOT compiled) | 24 slices + 5 anchors, every sig exact-unique re-verified @6b38b5383 this turn (rt `bf97a50e79cb`); replay `cmp` byte-identical; scratch glue == build-tree glue; `nm` glue defines all 10 new symbols |
| prelude (§2.3) | `lbl_805D3584` / `scene_fog_enable_flag` / `video_current_framebuffer` externs; GX capture-seam decls with the FULL era `GXCompare` set (GXEnum.h:28–37 — spec snippet lacked `GX_ALWAYS` and swapped EQUAL/LEQUAL, deviation §4.1); TObjPlayer/TKeyBoardControl/TParticleCtrl/TCameraCtrl static-method carriers (era statics verified: TObjPlayer.h:1105, TKeyBoardControl.h:36, TParticleCtrl.h:56–57, TCameraCtrl.h:151); `LC7_RT1..RT13` appended lock-step; P10–P12/I3 seam-state externs; `g_call_seq[160]->[512]` (§4.2) | compiles against the slices with zero era-header joins (L7-1 holds) |
| seam (§2.2 roster) | era zero-defs `lbl_805D3584 = 1` (:108, §3 D7), fog flag + framebuffer .bss; render_tick counter body LEAVES (seam now REAL); leg-1 `clear_shadow_pass_request` opens the tick capture epoch + carries the tick count; `update_global_sound_frame` counted, `update_music_sequencer` LB-3 fail-loud; `apply_scene_fog` counted (RT2/RT10); leg-4 camera restore + leg-6 debug-text counted arg-captured; GXSetZMode/CullMode/Viewport/ColorUpdate/CopyDisp/DrawDone capture seams; `present_frame_and_wait_vsync` = SINGLE present (rides LOGO-3 gx-side, never re-minted); walk extern gains RT13 at root==EOR + per-root tally | homes re-opened from the pinned tree this lane; both TUs compile rc=0 standalone |
| test + run.sh (§2.4/§4) | P10–P12 + I3a..d + EXIT Ea..Ec all REAL reads (§2/§3 below); spec-forced re-targets R3a/R3b/R3d (29-event vector), I2c/P9 (walk deltas 1→3/frame); run.sh insert-only (+1 PLAY-3 banner line, 0 removed) | `git show --stat` matches claims |

## 2. Green (same-turn disk)

| suite | result |
|---|---|
| `make -C host/bringup -f bringup.mk` (via run.sh) | rc=0; D8 pre==post==pin every make event (7 total, zero regen) |
| `build/bringup/bin/ladder7_test` | **62/62 PASS**, rc=0 (43 PLAY-1/2 + 19 new PLAY-3 lines) |
| `host/bringup/run.sh` ×2 back-to-back | **BRINGUP GREEN — 219/219 PASS, 0 FAIL**, exit 0 both (prior 200 + 19 new) |
| determinism | ladder7 section byte-identical across the two x2 runs (74 lines; 62 L7 PASS lines) |

**Prior-line audit (baseline 200 vs new):** 191/200 byte-identical. The 4-line
tick/ASLR-shape set (`clock`, `E1g`, `L6-G0`, `L6-G9`) moves by design. 5 lines
re-targeted BY SPEC: `L7-R3a` 15→29 events/frame, `L7-R3b` order string now
carries the `rt1..rt13 + walk_kbd/walk_eor` expansion, `L7-R3d` counts the full
§2.2 roster cadence, `L7-I2c`/`L7-P9` walk deltas 1→3/frame (spec §2.3 rebuilt
the FRAME_EVENTS vector; §1.5 I2c pre-announced the PLAY-3 roots).

## 3. Flag/leg honesty + EXIT shape proven (REAL reads)

- **P10** fog-flag: the one-liner effect_render slices carry the REAL SceneFog
  stores — `begin_effect_render` → `scene_fog_enable_flag==0`, `end` → `==1`;
  counted `apply_scene_fog` +2 per pair (sees the flag both ways).
- **P11** one real `render_tick()`: GXSetZMode trace ==
  `[(1,ALWAYS,0),(1,LEQUAL,1),(1,LEQUAL,1)]` (legs :494/:498 + copy_efb's :53),
  viewport `(0,0,640,480,0,1)` captured at :499, `set_cull_none` x2 →
  `GX_CULL_NONE`, `render_debug_text` arg `== lbl_805D3584 == 1` (§3 D7),
  GXCopyDisp(host-zero fb, GX_TRUE)+GXDrawDone chain, present +1, leg-1 externs
  reached through the kbd slice **with the :103 guard closed**.
- **P12** guard honesty: `sound_requests_enabled=1` → `run_frame` in a FORKED
  child takes the REAL `:103` load into `update_music_sequencer` → LB-3 exit 9
  (stderr transcript line lands in the log); parent state COW-intact, restore-0
  frame runs clean. Fork is required — an LB-3 fire ends the process.
- **I3a** 15-event tick order log == `:492–504` text order (13 RT notes + two
  walk-root events); **I3b** `g_present_calls == g_render_tick_calls` 1:1 — a
  second present never appears (gx LOGO-3 ride + viewport-twice + L7-P1/P2/P3
  pixel-gate byte-identity are the sibling gx lane's B3 receipts); **I3c**
  roots {MAIN `:287`, KBD `:492→:106`, EOR `:504→:114`} exactly once/frame;
  **I3d** fog flag == 1 at every tick/frame boundary.
- **EXIT Ea** is_ready gate → REAL fire: `server_ready=1` makes the run_frame
  slice's `:110` test pass through the real `is_ready` (pump +1) and
  `request_main_phase(0)` actually fires — `{next==0, result==1}` read back from
  the PLAY-1 slice (L7-I1f's never-fired zero state flipped by real stores).
  **EXIT Eb** cur `1→0xA→2→5`: each hop is a REAL `request_main_phase` fire
  (case-7 value 0xA at TLogo.cpp:193, title 2, file-select 5) plus
  `prev=cur; cur=next` replication of the byte-anchored `:389/:390`, against
  generator-verified table rows `:342/:343/:346`; exactly one fire per
  transition, `{next,result}` read back each step; **Eb4** a frame under
  `cur==5` keeps the PLAY-2/3 invariant vector (task_flags==0, I2a trace,
  present 1). **Ec** era-zero restore, no drift.

## 4. Deviations / honesty notes

1. **Spec §2.3 GX enum re-pinned:** snippet omitted `GX_ALWAYS` (leg :494 needs
   it to compile) and its ordering swapped `EQUAL/LEQUAL` vs era
   (`GXEnum.h:28–37`). Full era ordering pasted; values only ever logged.
2. **`g_call_seq` 160→512:** actual post-PLAY-3 cadence is **29 events/frame**
   (spec estimated ≈21); the existing 6-frame harness + probes ≈ 316 events, so
   the spec's own [160] sizing would silently truncate its own order log.
3. **RT13 note placement:** leg 13's first-inner counted extern IS the shared
   walk extern — the note fires right after `WALK_EOR` (LB-2 `LC7_READY`
   precedent), keeping one note per leg.
4. **Tick count rides leg 1:** `g_render_tick_calls` increments inside
   `clear_shadow_pass_request` (`:492`, a render_tick statement's first-inner
   extern) — verbatim slices can't be logged from inside; era-faithful 1/tick.
5. **EXIT walk is HOST-DRIVEN (stub-honest):** the dispatcher body itself
   (`main` :365–395) is rung-6's FIRST slice per spec §4 — bringup lands only
   what the sliced legs allow: REAL `request_main_phase` fires + the byte-anchored
   `:389/:390` walk + REAL is_ready gate. **Exit-table-once**
   (`main_phase_table[cur].exit` → `TLogo::exit_logo_scene`, TLogo.cpp:117–121)
   and the **window-rect byte-diff at one present** (`host/gx/run_window.sh`
   L7-P1/P2/P3 lineage) are explicitly NOT faked here — rung-6 / gx-lane deltas.
6. **`render_pass` I2a trace untouched by render_tick legs** (no sub-pass trace
   writes in the new bodies) — the PLAY-2 invariant set survives verbatim.
7. No scrubber artifacts (§4.4 note in the PLAY-2 receipt): `***` grep clean
   across all five tracked paths; `render_pass = 1;` intact on disk.

## 5. Rung-5 delta after PLAY-3 (counted + stub-honest, per spec §4)

ALL 13 `TLogo::run_frame` legs now execute era bodies or era-ordered verbatim
decomposition. Remaining crossings, none gating the rung-5 EXIT:

| group | legs | backing today | owner |
|---|---|---|---|
| P1–P4 frame legs | activate_camera_mode :722, set_active_camera_pose :796, controller_stuff :278+, update_keyboard_input_frame :268–274 | counted externs (lane-J first blocker `NJS_OBJECT_size` @NinjaTypes.h:162) | lane J |
| 8 counted sub-pass BODIES | geometry :399, effects :432, particle :449, objects :477, ui :460, clipout :292, info-icon TLobby_803382E4.cpp:145, render_info_icon_overlay | counted externs (pixel chain) | lane J |
| real `TObject::run_tasks` child bodies | era walk :116 | shared 3-root counted walk (rung-5-faithful) | lane J |
| render_tick leg internals | leg 4 camera statics (TCameraCtrl.cpp:1148–1156), leg 6 DebugFont.cpp:140 chain, legs 2/10 apply_scene_fog :53, leg 1 musyx/shadow stores (SoundPlayback.cpp:232, CameraCapture.cpp:384) | counted / capture / arg-captured externs | lane J |
| `wait_ready_loop` disk-error pump path | LB-2 seam | counted (era disk-present pumps zero) | lane J |
| R6 full body | flag-gated | guard slice REAL, loop counted | lanes J + M |
| dispatcher + exit-table | `main` SystemBootInit.cpp:365–395, `main_phase_table` dispatch, `TLogo::exit_logo_scene` TLogo.cpp:117–121 | REAL fires + :389/:390 replication; table rows byte-anchored | **rung-6** |
| pixel gate | window-rect byte-diff at exactly one present per frame | single-present 1:1 PROVEN here; byte-diff rides `host/gx/run_window.sh` | gx sibling seat (B3) |

## Repro

`host/bringup/run.sh` → `ladder7_test: 62/62 PASS` inside **219/219 BRINGUP
GREEN** (×2 back-to-back, exit 0); generator receipt:
`python3 host/bringup/gen_ladder7_runframe.py $DECOMP_ROOT /tmp/x.cpp`
(24 slices + 5 anchors; replay `cmp` clean). Tips: 3fd635b (slices),
d3e7c69 (prelude), c81d4f6 (seam), 00f9c02 (test+run.sh), this doc.
