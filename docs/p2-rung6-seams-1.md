# P2-R6SEAMS — Rung-6 seam map: logo-presented → first interactive screen

Lane R6SEAMS, 2026-11-27 (card `t_4e861fe5`). READ-ONLY design lane: ZERO builds,
waiver pin `host/bringup/waiver-header.sha256` never touched, no host/** writes.
All era cites opened this turn through the pin
`DECOMP_ROOT=/Users/scott/Hermes/pso-decomp/pso-decomp-port-pin` @ `6b38b53837`
(`src/pso/...` paths below are relative to it). Inherits the rung-5 ledger
(`docs/p2-rung5-seams-1.md`), PLAY-1/2/3 receipts (`docs/p2-ladder7-*.md`,
suite `ladder7_test 62/62` inside BRINGUP GREEN), HG-2 static tree
(`docs/p2-hg2-1.md`, 270/270) and the genuine logo (§5 of `docs/p2-oracle-1.md`).

## 0. What rung-6 is (span definition)

**Start landmark** — "logo frame presented": phase 1's last `render_tick()`
present (`TLogo::run_frame` tail, `ui/TLogo.cpp:114`, single-present law
`p2-rung5-seams-1.md` §3/R9) in the frame where `TLogo::run_task` state 7
fires `request_main_phase(0xA)` (`ui/TLogo.cpp:193-195`). The fire itself is
already REAL (rung-5 slice; L7-Eb1 read-back `{next==0xA, result==1}`).

**End landmark** — "first interactive screen": per `docs/boot-spine.md` §3 the
first real presented-and-input-driven screen is the **trial agreement**
(phase-10 mode 0x17, `TAdTrialAgreementGC` — waits for agree/decline), with
the **title scene** (mode 0, `TTitleScene` menu) as the rung-6 terminal
landmark after the in-phase scene chain 0x17→8→0. The named title *phase*
(table row 2) is NOT on the production first-boot path (boot-spine U4).

Rung-6's two mechanisms, as the rung-5 exit comments pin them
(`host/bringup/ladder7_test.cpp:362-366`): **(i) exit-table walk** — the REAL
dispatcher body runs `main_phase_table[1].exit` and re-enters through
`main_phase_table[0xA].init`; **(ii) present-diff framing** — the gx lane's
window-rect byte-diff gate (`host/gx/run_window.sh`, L7-P1/P2/P3 lineage)
proves the first present under `cur==0xA` differs from the last logo present,
with present-count still 1/frame.

## 1. Seam inventory (era call order)

Class legend is rung-4/5 grammar: **(a)** verbatim slice, zero externs ·
**(b)** verbatim slice + prelude externs · **(c)** slice crosses an
allocator/frame-pump/pixel seam · **(d)** blocked — named lane must land first.
Host status read from `host/bringup/**` + `host/gx/**` this turn (grep-verified,
not inferred).

| # | seam | era home (file:line) | reads / writes | host status | class | smallest testable leg |
|---|---|---|---|---|---|---|
| S1 | `request_main_phase(0xA)` fire | `ui/TLogo.cpp:193-195` | `main_phase_state.{next,result}` | **EXISTS** (PLAY-1 REAL slice; L7-Eb1) | landed | — (input to S2) |
| S2 | dispatcher body `main` | `ui/keyboard/SystemBootInit.cpp:370-393` (loop-top `result=0` :371; init :372-375; update `while(result==0)` :376-380 + `actor_frame_tick++` :379; exit :381-384; `switch(result)` 1/2/3 → `prev=cur; cur=next` :385-391) | `main_phase_state` all 6 fields, table | **MISSING** — only host-driven `:389/:390` replication (`ladder7_test.cpp:367-373`, stub-honest per its own comment) | **(a)** 6-line loop + table indexing; externs = `shutdown_and_reset_system` (`:124`, fail-loud LB-3) | **HG3-271** |
| S3 | `main_phase_table[22]` | `SystemBootInit.cpp:340-363`; rows used: [1] :342, [2] :343 (post-ending), [3] movie :344, [5] file-select :346, **[10] trial :351**; NULL rows :341/:358/:361 | — | **PARTIAL** — rows :342/:343/:346 + walk stmts are generator BYTE ANCHORS emitted as glue comments (`gen_ladder7_runframe.py:184-198`); no host-side table DATA exists | **(a)** host-side copy of rows {0..2,5,10} + NULL sentinels, same fail-loud anchor regime | **HG3-272** |
| S4 | `TLogo::exit_logo_scene` | `ui/TLogo.cpp:117-121` (order: `delete_layer_children` :118 → `call_funcs_reverse(logo_load_callback_table, 18*4)` :119 → `render_tick` tail :120) | tree roots, callback table | **MISSING** (fires exactly once = rung-6 exit-table proof, `p2-ladder7-play23.md` §4 item 3) | **(c)** — tail `render_tick` is the present seam (already landed R9); head legs = S5/S6 | **HG3-273** |
| S5 | `TMainTask::delete_layer_children` | `system/task/TObject.cpp:575-594` (19 `tl_*` roots × `delete_children`) | `tl_00..tl_fadeafter` globals | **MISSING** (rung-5 counts walks on 3 roots; the 19-global roster + `delete_children` don't exist) | **(c)** allocator seam (`TObject::delete_children` = pool churn) — verbatim body + counted `delete_children` extern first | **HG3-274** |
| S6 | `TLobby::call_funcs_reverse` | `ui/font/TFontSmallTask.cpp:251-261` (walks `LobbyFrameEntry{fn}` half-pairs back-to-front — the RELEASE half of every pair table) | table ptr | **MISSING**; 11-line body | **(a)** + `LobbyFrameEntry`/`LoadFrameEntry` decls | rides **HG3-273** |
| S7 | `logo_load_callback_table[9 pairs]` | `ui/TLogo.cpp:39-49` | fns incl. `TCameraCtrl::reset*`, `TLobby::free_keyboard_soft_resources`, `register_texture_*`/`load_*_logo_texture`, `TWindowCtrlMain::free_all_windows`, `TLogo::free_held_object` | **PARTIAL** — texture load/release backed in gx tree (`host/gx/dl/gx_logo_driver.cpp`: `register_texture_logo_e`, `load_sonic/adx_logo_texture`, `free_held_object`); TCameraCtrl = counted externs (lane-J gate, RS §1 P1/P2); `TWindowCtrlMain::reset_windows/free_all_windows`, `TLobby::free_keyboard_soft_resources`, `TAdSinglePlyChrSelectGC::free_global/free_singleton_805D65C0` MISSING | **(b)** — same 18 names already census-visible; host-copy the table, NULL-out or counter-extern the (d) members | rides **HG3-273** |
| S8 | `run_load_callback_table` | `ui/font/TFontSmallTask.cpp:263-276` (forward walker, aborts on `is_area_transfer_pending` :266-268) | area-change flag | **MISSING** (rung-5 used the pending==0 path only as a guard) | **(a)**; `is_area_transfer_pending` already host-def'd | rides **HG3-275** |
| S9 | phase-10 `init` row = `init_trial_agreement_phase` | `ui/frontend/TAdMultiChgGameModeGC_8023E7A8.cpp:541` = `init_game_mode_select_scene` + `open_trial_agreement_scene` + `render_tick` | composes S10+S11 | **MISSING** | **(b)** | **HG3-275** |
| S10 | `init_game_mode_select_scene` | `:448-492`: S8 over `game_mode_select_scene_callbacks[23 pairs]` (`:106-131`) + `lobby_scene_release_callback_table[7]` (`:134-142`); `TParticleCtrl::init_heap(0x100000)` :473, menu lights + sin/cos-table pose :474-478, `new TCameraCtrl(tl_camera)` :479, camera mode-9 pose :480-484, `create_quad_sprites` :485, `new TWindowCtrlMain` :486, `game_mode_scene_instance=0` :487, `load_floor_sound_bank(0)` :488 | heaps, lights, camera, windows | **MISSING**; heavy **(d)** overlap with rung-5's lane-J blockers (TCameraCtrl ctor/camera legs) + audio lane (`load_floor_sound_bank`) + particle heap | **(c)/(d)** — roster: verbatim body, counted externs for TCameraCtrl (J), fail-loud `load_floor_sound_bank` (audio-dark per boot-spine §4) | **HG3-276** |
| S11 | `open_trial_agreement_scene` | `:214-218` (`sync_mode_state` :547; `multi_chg_game_mode_state=0x17` :216; `game_mode_scene_change_state=1` :217) — same 3-store shape for `open_time_set_scene` mode 8 `:283-287`, `open_title_scene` mode 0 `:313-317` | globals `:87-94` (mode inits `29`) | **MISSING**; trivial | **(a)** | **HG3-277** |
| S12 | `run_game_mode_select_frame` (phase-10 update row; also rows 2/4/6/7/9/11/13 :343-354) | `:494-530`: change-state machine case1 destroy `:499-504`, case2 `create_mode_scene` `:505-508`; then rung-5 frame legs reused; NEW externs `TAdSinglePlyChrSelectGC::run_save_load_sequence`/`noop` `:518-519`, gate `TAdPlayerLoadGC::get_instance()==0` `:520` (fallback/is_ready legs ride already-REAL slices) | `game_mode_scene_change_state`, `game_mode_scene_instance` | **MISSING** (frame legs landed; wrapper absent) | **(b)** | **HG3-278** |
| S13 | `create_mode_scene` | `:318-424`: BGM switch `:321-334` (`apply_bgm_col_ctrl("advr.adx")` :332 — audio-dark no-op per boot-spine §4); scene switch: **case 0 → `new TTitleScene(tl_01)` :336-337**, case 8 → `new TAdTimeSetGC(tl_01) :351-352`, **case 23 (0x17) → `new TAdTrialAgreementGC(tl_01)` :397-398**, case 19 → second `TLogo` :384-387 (never selected in this span) | parent = `tl_01` (S5 pool regime) | **MISSING** | **(b)** for the 3 in-span cases; **(d)** on scene ctors (S14-S16) | **HG3-279** |
| S14 | `TAdTrialAgreementGC` = **first interactive screen** | ctor `TAdTrialAgreementGC.cpp:83-93` (`check_card_file` :53-74 memcard probe; `new TAdScrollWinGC(this,1,0)` :92); `run_task` `:76-78` → base `update_state`; **`on_agree` `:40` → `open_time_set_scene`**; `on_decline` `:39` → `request_main_phase(0)` (the real quit path); `on_closing` `:41` closes `scroll_window` | window chain (`tl_window`), `TAdAgreementBaseGC` state | **MISSING** — `TAdScrollWinGC`/`TWindowCtrlMain`/window text (font lane's `load_menu_font`, boot-spine S10) are the residual surface | **(c)/(d)** — HG3 legs stop at the store-level dispatch (`on_agree` fire == mode-8 store), NOT the scroll render | **HG3-280** |
| S15 | `TAdTimeSetGC` | `update_state` `:690-700` (case 0 → `apply_settings` `:695-696; default → restart_main_phase`); `apply_settings` `:885-899`: **NULL `SystemSaveData` → `change_state(1)` early-return `:888-891`**, else language/gain/sound-mode + `unload/load_lobby_scene_resources` (`:426-433`) + `open_title_scene` `:898`; `restart_main_phase` `:901-903` = `request_main_phase(0)` | global state ptr | **MISSING**; **port-risk flagged**: without memcard/global-state supply the era machine CANNOT leave time-set (NULL branch `:888`) and title bounces back to time-set (`TTitleScene.cpp:291-300`) | **(b)** NULL-branch leg only | **HG3-281** |
| S16 | `TTitleScene` = terminal landmark | ctor `:336` (`title_layer=new TTitleBackground :353`, `transition_controller=new TTitleMenu :354`, `attract_timeout 0x4B0/0x320 :349-351`); `run_task` `:289-317` (card-absent → `open_time_set_scene` bounce `:291-300`; input resets `attract_tick` `:301-305`; attract expiry → `set_fallback_main_phase(2); request_main_phase(3)` movie `:311-315`); `dispatch_title_choice` `:277-286`; own pair-table `lobby_load_callback_table[4]` `:90-95` (`load_title_menu/texture/background_resources`); `open_keyboard` `:273` → phase 19 | title assets (DLT/font load lane) | **MISSING** | **(d)** — asset+input heavy; rung-6 exits AT the trial screen + store-chain proof | **HG3-282** (stretch) |
| S17 | `TLobby::release_scene_resources` (phase-10 EXIT row, table :351) | `:434-447`: `refresh_render_state`, `stop_audio_streams`, instance destroy `:437-440`, S5, two `call_funcs_reverse` walks `:442-443`, `TParticleCtrl::free_heap`, `clear_resource_load_tasks`, `render_tick` tail | scene tables | **MISSING** (fires only when LEAVING phase 10 — title→movie etc.) | **(b)** once S5/S6/S10 land | **HG3-283** (next-rung tail) |
| S18 | phase-state helpers | `get_main_phase :161-163`, `set_fallback_main_phase :165-167`, `set_queued_main_phase :169-171`, `switch_to_fallback_main_phase :173-176` (result=3), `request_queued_main_phase :214-217` | `main_phase_state` | **PARTIAL** — `request_main_phase`/`reset_main_phase_state` REAL (rung-5, `ladder3_logo_prelude.h:39-46`); `switch_to_fallback_main_phase` = LB-3 fail-loud stand-in (`ladder7_seam.cpp:238`) → PROMOTE to slice | **(a)** each ≤4 lines | rides **HG3-271** |
| S19 | present-diff gate | era `render_tick` `unk/TLobby_801E2F94.cpp:491-506` (landed R9); host `host/gx/run_window.sh` (P2-W gate) | window-rect bytes | **PARTIAL** — single-present 1:1 + region readback gates EXIST (LOGO-3/L7-P1..P3, oracle §5 13/13); the cross-transition BYTE-DIFF leg does not | **(c)** gx seat (B3) owns; bringup side supplies the frame harness | gx-lane leg (see §3) |

Volopt note (card ask): the `TBoss3VoloptMonitor.cpp:76-101` 25-cell
`volopt_monitor_texture_entries` table is **not on the boot spine** — it matters
here only as census block B of the HG-2 static tree
(`docs/p2-static-init-1.md` §2, cells 54-78). Every `register_texture_*` seam
above (S7/S10) resolves `NJS_TEXNAME` strings through that already-landed
W-STATIC-TREE window (anchor row 3, `crt0.c`), so no rung-6 leg re-mints it.

## 2. Anchor rows needed in `pso_era_window_anchors`

Current table: 4 rows (`host/bringup/crt0.c:39-66`; `PSO_ERA_WINDOW_COUNT 4u`,
`hg1_windows.h`). Verdict: **zero new anchor rows for HG3-271..281** —

* `main_phase_state` is host-def'd native (`ladder3_logo_prelude.h:46`; era
  `.bss 0x8050E120`, comment :44) and the dispatcher indexes a HOST-side table
  copy (S3) — the rung-5 precedent is byte-anchored generation, not guest-address
  dispatch, so no window covers `main_phase_table` `.data:0x804805E0`;
* callback tables (S7/S10 data) are host-copy globals read as host pointers — no
  tag-space crossing;
* scene objects (`new TAdTrialAgreementGC(tl_01)` etc.) allocate through the
  already-bound W-POOL regime (row 2, `[0x80034000,0x80038000)` — ladder6
  L6_POOL/L6_SURF carve precedent);
* texname strings ride row 3 (W-STATIC-TREE, HG-2 green).

**One watch item:** if the S14 window chain (TAdScrollWinGC nodes) later stamps
POOL-handle tags, row 2's 16 KB span is shared with the texture-state pools —
a capacity re-audit (not a new row) belongs to that authoring card, mirroring
HG-2 §4's A2 re-audit discipline.

## 3. Smallest testable legs + build-order rec (next 2 bringup cards)

Numbering continues the golden-counter from HG2-270 (`docs/p2-hg2-1.md` §2):
legs are **HG3-271 …** onward; each is the smallest assertion set that can
fail (one canary trip per leg, HG2.3 pattern).

**Card BRINGUP-4 — "the walk" (HG3-271..274).** Pure dispatcher+exit-table,
zero new era bodies beyond two 11-line walkers and one 6-line loop:
S2 dispatcher body slice (host-copy table S3 rows {0,1,2,5,10}, generator
byte-anchors extended from `gen_ladder7_runframe.py:184-198`, `shutdown_and_reset_system`
= LB-3 fail-loud) → S18 promote `switch_to_fallback_main_phase` to REAL slice
(canary: result==3 walk) → S4+S6+S7 exit-table fire exactly once through the
REAL `call_funcs_reverse` with the 9-pair logo table host copy (reverse-order
witness log; (d) members counter-externed) → S5 counted-body `delete_layer_children`
(19-root roster). **What-green:** `cur` steps 1→0xA through the REAL loop;
`table[1].exit` fires once, `table[0xA].init` entered; `prev` chain honest;
canary = unregistered row index trips loud. Cost: 1–2 builds; all machinery
(slice generator, LB-2 pump, LB-3 roster, byte anchors) exists.

**Card BRINGUP-5 — "arrival at the trial screen" (HG3-275..281).** Phase-10
init+update through to the first interactive store-chain: S9/S11 mode-3-store
open fns → S10 scene-init roster (camera/particle/window members counted
externs — same lane-J blockers as RS P1/P2, never newly blocked here) → S12
frame wrapper + change-state 1→2→0 walk → S13 `create_mode_scene` 3 in-span
cases (trial ctor = shell TObject with `m_name` pin; scroll window NOT built)
→ S14 `on_agree`/`on_decline` fire as store proofs (decline path exercises
`request_main_phase(0)` → loop-exit → fail-loud shutdown = the honest quit
canary) → S15 NULL-branch time-set leg. **What-green:** end-to-end
`main_phase_state.cur` walk 1→0xA, `multi_chg_game_mode_state` 29→0x17→8
with `chg_game_mode_state_synced` one-behind law (`:547`), scene instance
destroy→create exactly once per change-state, present counter still 1/frame.
Cost: 2–3 builds.

**Then (not these 2 cards):** the gx seat adds the rung-6 pixel leg (S19):
last-logo-present vs first-trial-present window-rect byte-diff, present count
STILL 1 (`p2-ladder7-play23.md` §4 item 4 — gate machinery rides
`host/gx/run_window.sh`, never re-minted in bringup). Title-scene terminal
(S16) and `release_scene_resources` (S17) wait on the font/asset lane
(`load_menu_font`, boot-spine S10) and a SystemSaveData supply decision
(S15 NULL-branch: supply a default save block vs. accept an era-true port that
rests at time-set until supply exists).

## 4. Risks / unknowns (rung-6 deltas only)

- **R-A (S15/S16, verified this turn):** two independent memcard dependencies
  gate the title: `apply_settings` NULL-state early return
  (`TAdTimeSetGC.cpp:888-891`) and `TTitleScene::run_task` no-card bounce
  (`TTitleScene.cpp:290-299`). First-boot can reach the TRIAL screen without
  any save supply; it cannot REACH title without one. Decide supply vs. rest-on-time-set before Card BRINGUP-5 acceptance.
- **R-B (S7/S10):** the 23-pair scene table pulls ~46 distinct callees
  (multi-play/chr-select/chat-log/prate loaders); rung-6 must ride them as
  counters/fail-loud EXCEPT the ones already real — enumerate per-pair before
  the build, same census discipline as HG-2 generator (drift = loud exit).
- **R-C:** `run_game_mode_select_frame:520` gates the is_ready/fallback legs on
  `TAdPlayerLoadGC::get_instance()==0` — new gate global; pin its era default.
- No unknowns re-checked against boot-spine U3/U4: both stay open but non-gating
  (dev-flag VI-config branch `TLogo.cpp:139-141` never selected; first title IS
  inside phase 10 — table :351 confirms).

## Method / honesty

Every era file:line above was opened this turn from the pin worktree (no
memory cites); every host status from a same-turn grep of `host/bringup|gx`.
Zero builds, zero repo writes outside this doc + the fleet title note. Class
grades reuse rung-5's measured lane-J blockers rather than re-deriving them.
