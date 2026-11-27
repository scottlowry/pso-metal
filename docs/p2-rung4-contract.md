# P2-AP — Rung-4 transition contract: logo state 7 → first title-phase frame-ready

Lane AP, 2026-11-26. READ-ONLY analysis: git grep + file reads + era `.s` reads over
`pso-decomp @ 6b38b5383`; zero compiles, zero writes outside this doc. Inputs consumed:
`docs/p2-bringup-3.md` (AE: state-7 exit, GX wall `TLogo::run_frame` first bl
`begin_gx_frame` @ `TLogo.s 0x8013F5F4`; `request_main_phase` @0x8000F9CC executing),
`docs/p1-init-ledger.md` §2/§4/§5 (D1–D10 dispatch ledger), `docs/boot-spine.md` §3,
`docs/p2-first-link.md` (AG), `docs/p2-link-deps.md` + `data/link-deps.tsv` (U),
`docs/p2-dialect-triage.md` (T blocker grammar), `data/verdict-tranche1/2/3.tsv` +
`data/verdict-resolutions-1/2/3.tsv` (verdict state incl. AJ's NJS confirmations,
commit `e966ea8`), and the P2-R/P2-T probe corpus
(`cache/scratch/clangprobe/lp64native/err/*.err`, `p2t_rows.json`,
`p2rLP64perttype.json`) re-read here — classification re-measured, not inherited.

## 0. Grammar (T, docs/p2-dialect-triage.md §1)

`(d)` classes used below: **layout-assert-LOCKED** (fired LAYOUT_ASSERT names map to
types judged LOCKED-FORMAT in a tranche sheet or resolutions — stitch hard-refuses),
**dialect** (first blocking error is an `AT_ADDRESS`/address-pin in a SDK or decomp
header, T §2), **asm** (MWCC `asm`-function bodies via untracked `orig_asm/**/*.inc`,
T's "hardest class"), **intrinsic** (MWCC ABI/libc decl gaps), **verdict-open** (fired
assert on a type still UNRESOLVED after resolutions, or waiver-harness / signature
drift per-file work). Assert-name → type mapping is the probe corpus's
`p2rLP64perttype.json` (`ItemDataSize→TItemData`, `NinjaFloatKeySize→NJS_MKEY_F`,
`NJS_OBJECT_size→NJS_OBJECT`). Capped-corpus caveat: per-file assert rosters below are
the LP64 20-error-cap floor (P2-R states); absence ≠ clean.

Class legend: **(a)** already executing on arm64 · **(b)** IN-PARSE-SET post-stitch ·
**(c)** GX-BLOCKED (needs host/gx pixel substrate; may ALSO be class-d as a TU) ·
**(d)** still-unparsed + blocker class.

## 1. The chain — AE state-7 exit through `main_phase_table` dispatch to the GX boundary

Era-true order. AE's ladder exited at step 1–2 (driving `TLogo::run_task` directly —
`TLogo::run_frame` itself has never executed; its prologue `begin_gx_frame()`
(TLogo.cpp:95, `.s` 0x8013F5F4) sits at the head of every row in Leg 1).
All `file:line` are decomp-relative unless marked "(metal)".

| # | fn (home file:line) | leg | class · evidence |
|---|---|---|---|
| 1 | `TLogo::run_task` case 7 branch — `src/pso/ui/TLogo.cpp:193-194` | 0 | **(a)** bringup-3 §1-C, goldens C6/C7 (commit `e7badd2`) |
| 2 | `request_main_phase(0xA)` — `src/pso/ui/keyboard/SystemBootInit.cpp:219-222` (sets `next=10`, `result=1`; `.s` C6/C7 stores at :F9D8/:F9E0) | 0 | **(a)** bringup-3 §1-C |
| 3 | `TMainTask::render` — `src/pso/system/task/TObject.cpp:509` (call TLogo.cpp:104) | 1 | **(c)** actor draw pass = pixel producer; `TObject.cpp` itself (d): NJS LOCKED |
| 4 | `TMainTask::render_screen_overlay` — `TObject.cpp:644` (call TLogo.cpp:105) | 1 | **(c)** same |
| 5 | `process_async_file_reads` — `src/pso/system/AsyncFileRead.cpp:198` (call TLogo.cpp:106) | 1 | **(d)** layout-assert-LOCKED (`GslArchiveEntry`,`TLobbyFloorItemEntry`) + verdict-open (`BmlModelEntry` UNRESOLVED, resolutions missed it) |
| 6 | `update_sound_request_slots` — `src/pso/audio/SoundRequests.cpp:340` (call TLogo.cpp:107; KEEP-AS-IS-blocking row 41 p1-init-ledger §7.4) | 1 | **(d)** layout-assert-LOCKED (`SystemSaveData`) |
| 7 | `poll_reset_request` — `src/pso/system/ControllerInput.cpp:258` (call TLogo.cpp:108) | 1 | **(d)** layout-assert-LOCKED (`TItemData`); host must read no-request (ledger U6) |
| 8 | `TQuestVmDownLoadServer::is_ready` — `src/pso/unk/TLobby_800F8AA8.cpp:1371` (call TLogo.cpp:111) | 1 | **(d)** layout-assert-LOCKED (`ChallengeRecord`,`NJS_*`,`TItemData`); INVARIANT stays 0 (ledger row 34) |
| 9 | `render_tick` — `src/pso/unk/TLobby_801E2F94.cpp:491` (call TLogo.cpp:114) | 1 | **(c)** present path (Z territory, `docs/p2-gx-dl1.md`); TU (d): dialect (GXVert.h:25) + NJS LOCKED |
| 10 | dispatcher: `while (result==0)` exit + `actor_frame_tick++` — `SystemBootInit.cpp:377-379` (`actor_frame_tick` def `src/pso/unk/TLobby_800FD78C.cpp:10`) | 2 | **(d)** dialect — SystemBootInit first blocker `dolphin/os/OSExec.h:25` AT_ADDRESS (T §1 row); uncapped 255 assert sites behind it |
| 11 | dispatcher: `table[1].exit` fire — `SystemBootInit.cpp:381-384` | 2 | **(d)** dialect (same TU) |
| 12 | `TLogo::exit_logo_scene` — `src/pso/ui/TLogo.cpp:117-121` | 3 | **(d)** layout-assert-LOCKED (NJS_OBJECT, NJS_MKEY_F, TItemData + 8 RW types; bringup-3 sliced this same file — verbatim-slice route already proven) |
| 13 | `TMainTask::delete_layer_children` — `TObject.cpp:575` (call TLogo.cpp:118) | 3 | **(d)** layout-assert-LOCKED (NJS) |
| 14 | `TLobby::call_funcs_reverse(logo_load_callback_table,…)` — `src/pso/ui/font/TFontSmallTask.cpp:251` (call TLogo.cpp:119; table `TLogo.cpp:39-49`, 18 hooks run in reverse = release side only: `TCameraCtrl::reset_view_state/reset/init_default_view/reset_default_view` (`TCameraCtrl.cpp:546/577/1033/1055`), `TLobby::free_keyboard_soft_resources`, `TAdSinglePlyChrSelectGC::free_global/free_singleton_805D65C0`, `TLogo::free_held_object` (`TAdSEGAlogo.cpp:95`), `free_sonic_logo_texture`, `TWindowCtrlMain::free_all_windows`, `free_adx_logo_texture`) | 3 | **(d)** layout-assert-LOCKED (owner TUs `TFontSmallTask.cpp`, `TCameraCtrl.cpp`, `TWindowCtrlMain.cpp` all fire NJS/OVERLAY locks); teardown legs are pixel-free → injectable per DA-3 |
| 15 | `render_tick` (exit tail) — call TLogo.cpp:120 | 3 | **(c)** — first pixel-needing call strictly after state-7 exit |
| 16 | dispatcher: `case 1:` `prev=1, cur=10` — `SystemBootInit.cpp:385-391` | 2 | **(d)** dialect (SystemBootInit) |
| 17 | dispatcher: `table[10].init` fire — `SystemBootInit.cpp:370-375` | 4 | **(d)** dialect |
| 18 | `TAdMultiChgGameModeGC::init_trial_agreement_phase` — `src/pso/ui/frontend/TAdMultiChgGameModeGC_8023E7A8.cpp:541` | 4 | **(d)** layout-assert-LOCKED (measured: fired types = 8×RW + `NJS_OBJECT`,`NJS_MKEY_F`,`TItemData` LOCKED) |
| 19 | `init_game_mode_select_scene` — `…_8023E7A8.cpp:448-492`; inline legs: `set_screen_size` `GinjaView.cpp:69`(d) · `set_copy_clear_color_argb` `EffectRender.cpp:14`(d) · `reset_controller_input_state` `ControllerInput.cpp`(d) · `clear_resource_load_tasks` `TLobby_800F8AA8.cpp:1328`(d) · `TLobby::reset_keyboard_state` `TLobby_800F8AA8.cpp:819`(d) · `apply_phase_keyboard_mode` `TLobby_800F8AA8.cpp:834`(d) · `run_load_callback_table` `TFontSmallTask.cpp:263`(d) · `is_area_transfer_pending` `TLobby_800F8AA8.cpp:828`(d; INVARIANT 0 → straight-through) | 4 | **(d)** owner TU layout-assert-LOCKED only (no dialect/asm first-blocker) |
| 20 | load fan-out: `game_mode_select_scene_callbacks` 23 pairs `…_8023E7A8.cpp:105-129` + `lobby_scene_release_callback_table` 7 pairs `:132-140` (run at :460/:467) — texture/upload hooks (`register_texture_logo_e` `TAdSEGAlogo.cpp:103`, `load_title_midashi_texture` `TAdTitleGC.cpp:38`, `initialize_lobby_title_scene` `TTitleScene.cpp:393`, …) + logic hooks (`init_chat_log_heap` `text_8026ACDC.cpp:7`, `load_data_buffers` `TObjBoss2Tunnel.cpp:1049`, …) | 4 | **(c)/(d)** mixed — texture hooks = GX upload + disc asset class (fake_disc + Y DVD-shim route precedenced by bringup-3 §2; DA-1/DA-3 says inject per-hook); rung-4 contract: **inject the whole table, drive logic legs only** |
| 21 | `TParticleCtrl::init_heap` — `src/pso/effect/particle/TParticle.cpp:53` (call :475) | 4 | **(d)** layout-assert-LOCKED (NJS) + dialect (GXVert.h:25) |
| 22 | `init_menu_scene_lights` — `src/pso/object/ambient/TObjLight_80133568.cpp:374` (call :476) | 4 | **(d)** layout-assert-LOCKED (NJS/object set) |
| 23 | `sin_table_lookup`/`cos_table_lookup` — `src/pso/math/SinCosTable.cpp:16/20` (call :477-480) | 4 | **(d)** **asm** class — `asm` functions whose bodies are untracked `orig_asm/pso/math/SinCosTable/*.inc` (T §1/§2: no compile-time shim; per-fn host rewrite or gate-exclude is the named fix) |
| 24 | `new TCameraCtrl(tl_camera)` + `TCameraCtrl::select_active_controller` + `set_active_camera_pose` — call `…_8023E7A8.cpp:481-486`; home `src/pso/camera/TCameraCtrl.cpp` | 4 | **(d)** layout-assert-LOCKED (NJS, MotionContext) |
| 25 | `TParticleCtrl::create_quad_sprites` — `src/pso/effect/particle/ParticleDefinition.cpp:30` (call :488; already required by `init_logo_scene` TLogo.cpp:89 → same trap as ledger row 51) | 4 | **(d)** layout-assert-LOCKED (NJS, TParticleCtrl=RW, TQuadSprite*=RW) |
| 26 | `new TWindowCtrlMain` — call `…_8023E7A8.cpp:489`; home `src/pso/ui/window/TWindowCtrlMain.cpp` | 4 | **(d)** layout-assert-LOCKED (NJS) |
| 27 | `load_floor_sound_bank(0)` — call `…_8023E7A8.cpp:491` | 4 | **(c/host)** audio-dark by ledger rule (row 35 STUB; boot-spine §4 "deliberately skipped") |
| 28 | `open_trial_agreement_scene` — `…_8023E7A8.cpp:214-218` + `sync_mode_state` `:547` (mode `0x17`, change-state 1) | 4 | **(d)** same owner TU (trivial 3-store body; sliceable per DA pattern) |
| 29 | `render_tick` (init tail) — call `…_8023E7A8.cpp:541` | 4 | **(c)** |
| 30 | dispatcher: `table[10].update` fire — `SystemBootInit.cpp:376-378` | 5 | **(d)** dialect |
| 31 | `run_game_mode_select_frame` first call, scene-change `case 1` — `…_8023E7A8.cpp:494-509` (instance is NULL: zeroed at :490 → pure store `change_state=2`) | 5 | **(d)** layout-assert-LOCKED (owner TU) — **last pure-logic step before the boundary** |
| 32 | `begin_gx_frame` — `…_8023E7A8.cpp:510`; home `src/pso/render/gjSystem.c:41` | 5 | **(c)** ★ **THE rung-4 GX boundary** (§2) |
| 33 | `run_game_mode_select_frame` frame-2, `case 2` → `create_mode_scene` — `…_8023E7A8.cpp:505-508` → `:318`, `case 23 → new TAdTrialAgreementGC(tl_01)` `:397-398`; ctor home `src/pso/ui/frontend/TAdTrialAgreementGC.cpp:84` | 6 | **(d)** layout-assert-LOCKED (`CardFileInfo`,`NJS_*`); reached in the pre-GX scene-change leg ⇒ frame-ready iff the GX cycle seam (32 + tail `:511-529`) is closed by injection |
| 34 | `apply_bgm_col_ctrl("advr.adx")` — `…_8023E7A8.cpp:332` → `:194-198` → `play_area_music` `src/pso/audio/MusicControl.cpp:487` | 6 | **(c/host)** audio-dark stub (ledger D7 "BGM calls stubbed") |
| 35 | per-frame tail `controller_stuff … render_tick` — `…_8023E7A8.cpp:511-529` (`TAdPlayerLoadGC::get_instance` :521, `TQuestVmDownLoadServer::is_ready` :525, `render_tick` :529) | 5 | **(c)/(d)** one GX cycle per frame; present = **(c)**, guards = rows 5–8 above |

**Deeper chain (leg 7, post-agree, contract-level only — input-gated, each step re-crosses
rows 31–33):** `TAdTrialAgreementGC::on_agree` (`TAdTrialAgreementGC.cpp:40`) →
`open_time_set_scene` (`…_8023E7A8.cpp:283`, mode 8) → `new TAdTimeSetGC` (`:351-352`,
home `TAdTimeSetGC.cpp`) → `TAdTimeSetGC::apply_settings` (`TAdTimeSetGC.cpp:885`) →
`open_title_scene` (`…_8023E7A8.cpp:313`, mode 0) → `create_mode_scene` case 0
(`:336-337`) → **`new TTitleScene(tl_01)`** (`TTitleScene.cpp:336`) — the first
title-phase object; `TTitleScene.cpp` measured **(d)** layout-assert-LOCKED
(`NJS_*`,`TItemData`,`SystemSaveData`) and `TAdTimeSetGC.cpp` **(d)**
(`CardFileInfo`,`NJS_*`). p1-init-ledger D9's assumptions (input real, memcard clock
default) apply.

## 2. The GX boundary line

- **Frame-N wall (AE's, behind the rung):** `TLogo::run_frame` first executed bl =
  `begin_gx_frame` at `TLogo.s 0x8013F5F4` (bringup-3 §3). Everything in run_frame
  after it (rows 3–9) lives inside an opened GX frame; frame N's tail cannot be
  *executed* (only injected) without the substrate.
- **First pixel-needing call after state-7 exit:** `render_tick`
  (`TLobby_801E2F94.cpp:491`) at `TLogo.cpp:114` (frame N tail) → `TLogo.cpp:120`
  (exit tail) → `…_8023E7A8.cpp:541` (phase-10 init tail) → the Z-mode 640×480
  EFB-copy/present tail (boot-spine §2.1).
- **The transition boundary the contract stops at:** `begin_gx_frame()` —
  `src/pso/render/gjSystem.c:41`, first executed at
  `src/pso/ui/frontend/TAdMultiChgGameModeGC_8023E7A8.cpp:510` (phase-10 frame driver
  prologue; same shape as TLogo.cpp:95). **Everything before it is rows 1–31.**
  Frame-2's `case 2` scene creation (row 33) sits in the *pre-GX leg* of the next
  driver call — i.e. rung-4 "title-phase frame-ready" = rows 1–31 executed +
  `begin_gx_frame`/`render_tick` seams fired (injected or real) + row 33 spawned
  `TAdTrialAgreementGC`. Real pixels are only demanded once the trial-agreement
  window/texture pass draws (leg 7 / Z lane).

## 3. Classification rollup (spine rows 1–35)

| class | count | members |
|---|---:|---|
| (a) executing | **2** | rows 1–2 (bringup-3 goldens C0–C12; commit `e7badd2`) |
| (b) in-parse-set post-stitch | **0** | — see §4: every spine TU is stitch-plus-locked |
| (c) GX-BLOCKED (needs host/gx as a *step*) | **9** | rows 3–4, 9, 15, 29, 32, 35 (+ 20, 27/34 host-audio/asset fan-outs marked (c)/(d)) |
| (d) still-unparsed | **24** | dialect: 10, 11, 16, 17, 30 (+SystemBootInit-coupled); layout-assert-LOCKED: 5–8, 12–14, 18–22, 24–26, 28, 31, 33; asm: 23; verdict-open secondary: 5 (`BmlModelEntry`) |
| also (c)∩(d) | rows 9, 20, 35 | GX steps whose owning TU is itself locked — injection precedes parsing there |

**Measured, not inherited:** re-scored each spine TU's
`lp64native/err/<TU>.err` assert names against tranche1/2/3 + all three resolutions.
Result: **no spine TU is in the post-stitch parse set.** The dominant shared lock is
the **NJS overlay pair**: `NJS_OBJECT_size` / `NinjaFloatKeySize` asserts
(`include/pso/system/NinjaTypes.h:162-163`, tranche-1 evidence col) on types
`NJS_OBJECT`/`NJS_MKEY_F` — **LOCKED-FORMAT, HIGH confidence, re-confirmed by AJ**
(`verdict-resolutions-3.tsv:2-3`, OVERLAY-CAST `BmlResource.cpp:441`,
`TBoss1Dragon.cpp:574`; `docs/p2-njs-confirm.md`). Traffic: 1015 / 976 TUs
(tranche-1 cols). Second lock on the spine: `ItemDataSize`→`TItemData`
(LOCKED-FORMAT escalation, `docs/p2-verdict-tranche2.md` §3, 511 TU traffic). Third
cluster on the frontend screens: `CardFileInfo`/`SystemSaveData` (save-blob LOCKED,
`verdict-resolutions-1`). RW-only control case: `TKeyBoardControl_80290CB0.cpp`
flips to parse-set under stitch alone — the battery's semantics are real, they just
don't cover these TUs.

## 4. Minimum next increment after the stitch — ONE TU

**Target: `src/pso/ui/frontend/TAdMultiChgGameModeGC_8023E7A8.cpp`** — highest
chain-position × currently-blocked on the spine: it OWNS rows 18–28 + 31 + 33, i.e.
the entire span from the phase switch to the first trial-agreement scene object. No
other single TU extends the executing chain further (TLogo/SystemBootInit legs it would
unlock behind it are already executing-by-slice or dialect-shimmed; leg-7 scene TUs sit
past it and can't be reached without it). It is also the *cheapest* spine member:
**layout-only** — first blocking error at LP64 is a LAYOUT_ASSERT, no dialect/asm/
intrinsic class (absent from T's 113-file set); of its 11 measured fired types, **8 are
RUNTIME-WAIVE** (ControllerInputContext/State, TAdMultiPlyChrSelectGC, TCameraCtrl,
TLevelSelect, TMessageWindowController, TParticleCtrl, TQuestVmDownLoadServer) —
cleared by the stitch battery itself.

**Unblock recipe (in order):**
1. **Lane-J handle conversion of the NJS overlay pair** — re-pin
   `NinjaTypes.h:162-163` per p1-datamodel §3 rule-1 (wire image keeps 32-bit handles;
   runtime accessors convert), the exact class AJ confirmed (chunk-copy + POF0
   pointer-offset patches, `read_ninja_chunk` `TLobby_801E2F94.cpp:334-430`,
   `docs/p2-njs-confirm.md`). This is a **single-header** fix and the universal key:
   it removes the NJS lock from rows 5–33 of the spine AND ~1000 TUs tree-wide.
2. **`TItemData` re-pin** (mixed-width unions, `serialize_mag`; p1-format-saves §1) —
   the only remaining LOCKED type on this TU after (1).
3. Stitch battery (this lands with the waiver set): covers the 8 RW types.
4. Then one named sub-step still needs its own seam: row 23
   `sin/cos_table_lookup` — **asm class, no parse path** (bodies are untracked
   `orig_asm/**.inc`): inject per AE's DA-1 pattern (host table-lookup reimplementation
   pinned against the era `.s`, or fold into the T §2 "rewrite as intrinsics" queue).
5. GX seams stay injected as counters (begin_gx_frame/render_tick) until Z's dl lane
   is linkable — rung-4 asserts call-order/inputs, not pixels, exactly the
   bringup-3 DA-3 precedent.

Fallback if lane-J slips: the same legs (18, 28, 31, 33) are verbatim-sliceable with
the proven `bringup.mk` awk+guard mechanism (rows 12/14 precedent) — that RUNS the
chain without PARSING the TU, but does not compound; the parse (steps 1–3) is the
deliverable.

## 5. Lane-4 execution invariants (contract clauses)

- **Never-ready traps:** `TQuestVmDownLoadServer::is_ready()==0` (rows 8/35; ledger
  row 34 — a true reads `request_main_phase(0)` → shutdown); `poll_reset_request` reads
  no-request (U6); `is_area_transfer_pending()==0` (rows 16h/20 select the
  straight-through legs of both init scenes).
- **Progressive/VI leg:** drive `check_progressive_ready()==0` leg — AB's
  `host/gx/gx_vi.c` + defer-audit A3 (`logo_clear_flag=1`) model the other leg; rung 4
  starts from AE's state-7 exit on the `check_progressive_ready()==0` chain.
- **Scene-machine handshake:** rows 28→31→33 are three global stores
  (`multi_chg_game_mode_state=0x17`, `game_mode_scene_change_state=1→2→0`) observed by
  the pre-GX switch — assert each with its `.s`-derived offset, bringup-3 style.
- `main_phase_table[]` stays FULL (22 entries, ledger D2); phase-10 exit
  `TLobby::release_scene_resources` (`…_8023E7A8.cpp:434-447`, ends in `render_tick`
  — (c)) is the rung-5 boundary, out of scope here.

## 6. Method note

All TU verdicts re-derived 2026-11-26 from the committed probe corpus at
`pso-decomp @ 6b38b5383` (P2-R LP64 capped run + P2-T uncapped row set + P2-R
type→assert-name map) crossed with `data/verdict-tranche*.tsv` +
`data/verdict-resolutions-*.tsv` exactly as the stitch battery consumes them; no
compiler was invoked by this lane. Counts floor-limited by the 20-error cap
(P2-R caveat) — a TU listed (d) can gain blockers, never lose them, under a cap lift.
