# P2-R7SEAMS — Rung-7 seam map: trial agreement presented → title menu live

Lane R7SEAMS, 2026-11-27 (card `t_411e2e3b`). READ-ONLY design lane: ZERO builds,
waiver pin `host/bringup/waiver-header.sha256` never touched, no host/** writes.
Tip verified same-turn: pso-metal `main` @ `573ba98`. All era cites opened this
turn through the pin `DECOMP_ROOT=/Users/scott/Hermes/pso-decomp/pso-decomp-port-pin`
@ `6b38b53837` (`src/pso/...` paths below are relative to it). Inherits the
rung-6 ledger (`docs/p2-rung6-seams-1.md`, S1–S19 / HG3-271..283) and everything
it inherits; S12/S13/S15/S16/S17 are LINKED, not re-derived. Host statuses are
same-turn greps of `host/bringup/**` + `host/gx/**`.

## 0. What rung-7 is (span definition)

**Start landmark** — rung-6's end: phase 10, `multi_chg_game_mode_state == 0x17`,
`game_mode_scene_instance` = a live `TAdTrialAgreementGC` whose `run_task` pumps
`TAdAgreementBaseGC::update_state` (`ui/frontend/TAdAgreementBaseGC.cpp:56-107`)
and whose scroll window has been constructed (`ui/frontend/TAdTrialAgreementGC.cpp:92`).
Rung-6's legs stop at the store-level dispatch (RS S14 note: "`HG3` legs stop at
the store-level dispatch … NOT the scroll render"); rung-7 takes the whole
window/scroll/input surface from there.

**End landmark** — "title menu live": `TTitleScene` constructed
(`ui/frontend/TTitleScene.cpp:336-382`), its `transition_controller` has left
state 0 (`TTitleMenu::update_title_prompt`, `ui/TTitleMenu.cpp:46-55`) and the
4-way cursor dispatch resolves (`TTitleScene::dispatch_title_choice` :277-286 →
`state_handler_pmf_table[4]` :100-105). The in-span scene walk is
**0x17 → 8 → 0** (agree → time-set → title), each hop a
`sync_mode_state`+2-store fire (see §1 T7-S6).

**First-boot reality check (verified this turn, re-grounds RS R-A):** the trial
screen is presented ONLY when `check_card_file()` returns 0
(`TAdTrialAgreementGC.cpp:55-74`, ctor branch :87-92): with no memory card
`probe_card_slot(0) == 0` short-circuits at `:58-60`, so the era machine reaches
the trial screen WITHOUT any save supply — and if a card DOES carry the system
file, the ctor calls `on_agree()` + `queue_destruction()` itself (`:87-90`) and
the trial text is never presented. Between those two outcomes sits
`wait_for_load()` (`:42-49`), an era-true BUSY LOOP around `render_tick()` — see
risk R-G.

## 1. Seam inventory (era call order)

Class legend is the rung-4/5/6 grammar: **(a)** verbatim slice, zero externs ·
**(b)** verbatim slice + prelude externs · **(c)** slice crosses an
allocator/frame-pump/pixel seam · **(d)** blocked — named lane must land first.

| # | seam | era home (file:line) | reads / writes | host status | class | smallest testable leg |
|---|---|---|---|---|---|---|
| T7-S1 | `TAdScrollWinGC` ctor = window build | `ui/frontend/TAdScrollWinGC.cpp:27-61`: pos `{44,131}`/size `{552,220}` :28-29, `scroll_win_texture_handle = get_player_tex_texture_list()` :33 (global :22; home `ui/frontend/TAdMultiPlyChrSelectGC_802C6F88.cpp:25`), `get_scroll_delay_for_mode()` :40 (home :112-130), `create_message_log_window(pos,size,0)` :41 (home `ui/window/TWindowMessageLog.cpp:267-270`), 10 × `TWindowMessage::append_line(get_localized_text_entry(0x1A, 0xA+i), 1)` :45-47 (trial = mode 1 → base `0xA` :123, entries `0xA..0x13`; resolver `unk/TLobby_80132ED0.cpp:22-24` = `text_language_group_table[group][id]`), `complete_reveal`/`update_scroll_position`/`reset_scroll` :54-56 (homes `unk/text_802693B4.cpp:26-43`, `TWindowMessageLog.cpp:72-89`, `:124-128`), marker UV :57-60 | window entry array + `lines[rows]` (`ui/window/TWindowMessage.cpp:340` `mem_alloc(rows*sizeof)`), globals | **MISSING** — whole TWindowCtrl/TWindowBase chain absent (same-turn grep: no `TWindow`, no `render_nodes` in `host/bringup|gx`; `set_message_log_window`/`get_field44` homes `ui/window/TWindowCtrl_8026AD44.cpp:19`/`:6`, no host twin) | **(c)/(d)** — verbatim walker bodies are (a)-grade but the window classes + text table are unbuilt; glyph RENDER is (d) on the font lane (T7-S4) | **HG3-284** |
| T7-S2 | scroll kinematics | `TWindowMessageLog.cpp:130` `advance_scroll(v){pending_scroll+=v;}`, `:133` `add_pending_scroll(v)` — SAME body, opposite call direction (source comment :132), `update_scroll_animation` `:107-117` (applies then zeroes `pending_scroll`, then clamp), `TWindowMessage.cpp:132-147` `update_scroll()` easing `step = max(15.0f, 0.15f*distance)`, `clamp_scroll_offset` `TWindowMessageLog.cpp:64-71` (range `frame_size[1]-20`, `WindowScrollBar::clamp_offset` `ui/window/WindowScrollBar.cpp:16`), `reset_scroll` `:124-128` | `pending_scroll`, `target_scroll_y`, `scroll_offset[1]` | **MISSING**; pure float math otherwise | **(a)** once the 3-float state is host-side | **HG3-285** |
| T7-S3 | analog scroll input law | `TAdScrollWinGC.cpp:93-102`: `d = controller_physical_port_bank.controllers[0].cstick_y` :93 (field `include/pso/system/ControllerInputState.h:26`); `d<0` → `add_pending_scroll(0.3f*(d+0x19))` :94-99; `d>0` → `advance_scroll(0.3f*(d-0x19))` :100-102; `d==0` dead; alpha ramp `m_alpha += 0xC` clamp `0xFF` :87-88; close timer `state==1 && ++m_close_timer > 4.5f → queue_destruction` :80-84 | bank struct field | **PARTIAL** — the bank global EXISTS host-side in the gx VI lane (`host/gx/vi_branch_driver.c:34` zero-init; layout twin + 0x50-size rationale `host/gx/vi_era_harness.h:35-47`); bringup has ZERO bank definition (same-turn grep) | **(a)** sign-split canary; (c) if it pulls the VI bank global — see risk R-D | **HG3-286** |
| T7-S4 | text supply + font surface (boundary only) | text: `get_localized_text_entry` `TLobby_80132ED0.cpp:22-24`; group `0x1A` scroll body, group `0x1B` the two item labels via `draw_agreement_text` `TAdAgreementBaseGC.cpp:38-48`; glyph draw chain: `WindowTextEntry::render` `ui/window/WindowTextEntry.cpp:172-200` → `draw_small_font_string_clipped` / `draw_window_text` (`ui/window/TWindowSubStatusDisp_8024FBA8.cpp:33`/`:86`); menu font: `load_menu_font` `ui/font/TNECFontEORFunction.cpp:528-585` — `load_disc_file("S18all04.fon")` :541, 2 glyph pages :543-551, palette :552-557, glyph table 2×12×12 (0xB40 bytes) :562-580, `TNECFontEORFunction` task object :583-584; boot caller `ui/keyboard/SystemBootInit.cpp:310`; `calc_text_width` home `TNECFontEORFunction.cpp:112` | disc file, glyph pages | **MISSING** — only `render_debug_text` counted stub exists (`host/bringup/ladder7_seam.cpp:173`); boot-spine §"deliberately skipped" puts font SECOND after logo (`docs/boot-spine.md` :72,:74) | **(d)** — font/asset lane must land; bringup legs prove text STORES (entry array + widths stubbed), never glyphs | rides **HG3-284** (font-lane card owns the draw) |
| T7-S5 | agree/decline state machine | `TAdAgreementBaseGC.cpp:56-107`: state0 `create_agreement_callback` :60-63 (home :52-54 → `TAdEnterSerialAccessGC::create_callback` `ui/frontend/TAdEnterSerialAccessGC_802ACF04.cpp:7`, pos/size `{85,384}`/`{470,60}` :26-27); state1 :64-80 — `cursor->confirmation_result()==1` (`:65`) → `selection_index()` 0→`next_state=3` `:68-69` / 1→`next_state=4` `:72-73`, else B-button `test_pressed_buttons_packed(&controller_physical_port_bank.controllers[0], 2)` or `test_controller_flag_4c_color(&controller_input_state.keyboard, 2)` (`:76`) → `state=2; next_state=5` `:77-78`; state2 :81-92 — `close()` callback window `:82-85` + `on_closing()` `:86` + `frames > 4.5f` latch `:87-91`; state3 `on_agree` :93-95, state4 `on_decline` :96-98, state5 `on_cancel(){state=4;}` :99-101 + `:50`; prev!=state resets `frames` :103-105; trial overrides `TAdTrialAgreementGC.cpp:39-41` (`on_agree → open_time_set_scene`, `on_decline → request_main_phase(0)`, `on_closing → scroll_window->close_window()`); cursor ctor law: 2 items `{89,394}`/`{89,414}` `TAdAgreementBaseGC.cpp:23`, `set_wait_directions_released` :128, **`set_selection_index(1)` :129 = DEFAULT DECLINE**; `TAdSelectCurGC::run_task` `ui/frontend/TAdSelectCurGC.cpp:124-191`: up/down `0x10`/`0x20` via `test_buttons_press_or_repeat*(…,6,1)` :140-149 with wrap :151-156, A-confirm mask 4 → `confirmation=1` :157-162, `wait_directions_released` gate reads `test_previous_buttons_remapped` :129-138; button primitives `system/ControllerInput.cpp:677-701` (`pressed_with_sticks &` packed masks, `pack_button_mask` :670-672) | cursor index, state/next_state, `pending_scroll` none | **MISSING**; bank/controller_input_state supply as T7-S3 (rider: `controller_stuff`/`update_keyboard_input_frame` are counted stubs, `host/bringup/ladder7_seam.cpp:119-120`) | **(b)** — slice `update_state` + `TAdSelectCurGC::run_task` with INJECTED `pressed_with_sticks` bits; sound externs (`play_sound_effect` :142/:147/:160) counted, audio-dark per boot-spine §4 | **HG3-287** (state machine), **HG3-288** (cursor nav slice) |
| T7-S6 | scene chain 0x17→8→0 (LINK to RS S12/S13 — not re-derived) | each hop = `sync_mode_state()` (`TAdMultiChgGameModeGC_8023E7A8.cpp:547`, one-behind law) + `multi_chg_game_mode_state` + `game_mode_scene_change_state=1`: agree → `open_time_set_scene` `:283-287` (mode 8), title → `open_title_scene` `:313-317` (mode 0), trial → `open_trial_agreement_scene` `:214-218`; destroy/create walk `run_game_mode_select_frame:494-530` case1 `:498-504` → case2 `:505-508`; `create_mode_scene:318-424` in-span cases **23→`new TAdTrialAgreementGC(tl_01)` :397-398**, **8→`new TAdTimeSetGC(tl_01)` :351-352**, **0→`new TTitleScene(tl_01)` :336-337** (case 19 second `TLogo` `:384-387` dead in-span); BGM gate `:320-334` skips `advr.adx` for modes {0,8,9,18,19,21,22,23,27} — all three in-span modes are on the skip list, so no BGM fire between them; time-set→title per RS S15 (`apply_settings` → `open_title_scene`, RS cite `TAdTimeSetGC.cpp:885-899`, gated by RS R-A memcard supply — MEMSCAN lane's decision, cited here as scope only) | same 3 globals as RS S11/S12 | **MISSING** at wrapper level; the store law itself is the rung-6 leg (RS **HG3-277/278**) | **(a)** on top of RS legs — first end-to-end two-hop walk proof | **HG3-290** |
| T7-S7 | trial close→destroy timing | `on_closing` closes the scroll window via `TAdScrollWinGC::close_window` `TAdScrollWinGC.cpp:105` (nulls `m_window`) / `close` `:107-110` (state 1 → 4.5-tick self-destruct `:80-84`); agreement state2 4.5-tick latch `TAdAgreementBaseGC.cpp:81-92`; base dtor queues both windows `:109-113`; scene destroy via change-state case1 `:498-503` | window pointers, timers | **PARTIAL** — `queue_destruction` exists ONLY as a counter (`host/bringup/ladder3_test.cpp:157`), no real linked-list slot like era `:F430` semantics | **(c)** — allocator/teardown seam; smallest leg = fire-count + NULL-out proof | **HG3-289** |
| T7-S8 | trial marker-quad render (gx surface) | `TAdScrollWinGC::render` `:132` (render-pass guard + blend-mode pairs) → `render_status_value` `:134-140`: `begin_screen_draw(1)`, `set_texture_list(scroll_win_texture_handle)`, `set_draw_texture_and_color(0, (m_alpha<<24)|0xFFFFFF)`, `draw_textured_quad_at_depth(&m_marker.x0, 0.1f)`; marker rect `531,353→603,373` set every frame `:89-92`; texture supply = 7-pair table slot `{load_player_tex_texture, free_player_tex_texture}` (`:135`) → `ui/frontend/TAdTitleTexGC.cpp:40-62` `load_streamed_prs_texture("f256_player_tex[_eng]", …)` language-picked | texname list, alpha byte | **PARTIAL** — gx lane owns the quad primitive + readback regime (RS S19 lineage, `host/gx/run_window.sh`); the `f256_player_tex` PRS asset is NOT in `host/bringup/fake_disc` (same-turn ls: `logo_e.prs`, `sinbd_tex.gvr`, `gsl_tab*.dat`, `title.cfg`, `title_prs.dat` only) | **(c)** gx seat (B3) — byte-diff gate rides RS, never re-minted | gx-lane leg (see §3) |
| T7-S9 | window-chain control surface | `ui/window/TWindowCtrlMain.cpp`: `reset_windows` `:374` → `reset_all` `:376-391` (HUD palette load, shared DrawContext clear+create `:379-380`, 6 viewport states init `:388-390`); ctor `:424-448` (task root + instance + 6 viewport roots under `window_task_root`, `TWindowCtrlScreen(tl_window)` :440, `TWindowCtrlSub(tl_01)` :443, `TWindowCtrlClipOut(tl_clipout)` :446); `free_all_windows` `:393-422` reverse-order teardown; ticking: `TWindowCtrlSub::run_task` `:775-779` (`window_task_root->run_tasks()`), render fan-out: `TWindowCtrlMain::render` `:481-488` (`get_main_window_root()->render_nodes()`), `TWindowCtrlScreen::render` `:751-765` (viewport fill + backdrop gradients + per-viewport roots), `TWindowCtrlClipOut::render` `:789-798`; DrawContext singleton `ui/window/WindowDrawContext.cpp:240-247`, shared-clear `:282`; window tick law `TWindowMessageLog::run_task` `TWindowMessageLog.cpp:286-298`, `TWindowMessage::run_task` `TWindowMessage.cpp:313-324`; entry walk `ui/window/WindowLayoutEntries.cpp:38-47` (`draw_visible_window_entries`) + `:49-61` (`update_window_entry_list`, gated `open_tick >= 7.5f`) | all window globals | **MISSING** — but its LOAD/RELEASE PAIRS are already RS S3/S10-visible (`game_mode_select_scene_callbacks` row `{TWindowCtrlMain::reset_windows, TWindowCtrlMain::free_all_windows}`, same-turn re-read `TAdMultiChgGameModeGC_8023E7A8.cpp:124`; RS S7 marked the same pair MISSING) | **(c)** — allocator (1003-line `TWindowBase.cpp` + 1321-line `TWindowCtrl.cpp` NOT in any span above: cite discipline) + task-tree seam | rides **HG3-284** |
| T7-S10 | title scene ctor roster | `TTitleScene.cpp:336-382`: `title_scene_instance=this` :338, `set_fallback_main_phase(1)` :343, resets `:344-346` (`TAdMultiPlyChrSelectGC::reset_slot_selections`, `TObjEneDolmOlm::reset_count`, `clear_multi_chr_select_slot_chosen` home `:404`), `lobby_game_mode_state=0` :347, attract timeout `0x4B0`/`0x320` by `get_video_standard()` :348-352, `title_layer=new TTitleBackground(this)` :353 (home `ui/frontend/TTitleBackground.cpp:146-148`), `transition_controller=new TTitleMenu(this)` :354 (home `ui/TTitleMenu.cpp:143-152`), `overlay_layer=new TUnk_804EE470(tl_clipout)` :355 (home `ui/TitleTextures.cpp:51-53`), `suspend_update()` :357-359, `apply_bgm_col_ctrl("title.adx")` :360 (home `:194-198`, audio-dark extern), `CameraViewParams` override 640×480/0.85 y-scale `:361-373`, 3 × `create_particle_controller(0x1E3/0x1E4/0x1E5)` :374-379, `new TScreenFadeIn(tl_fade, TLobby::destroy_download_server)` :380 (callback home `:237-242`), `TItemReverseTechCtrl::create` :381 | tl_01/tl_clipout/tl_fade roots, scene globals | **MISSING**; camera/particle members are the SAME lane-J blockers as RS S10 (never newly blocked) | **(c)/(d)** — verbatim roster with counted externs (camera lane J, particles, `TItemReverseTechCtrl`, fade objects ride rung-5 root regime) | **HG3-291** |
| T7-S11 | attract machine | `TTitleScene::run_task` `:289-317`: per-frame `TAdMultiChgGameModeGC::set_default_camera()` :290 (home `:177-182` — `activate_camera_mode(9)` + `set_active_camera_pose`, lane-J extern); no-card bounce `:291-300` = RS S15/R-A (cited, not re-derived); attract reset `:301-305` reads `hardware_keyboard_state.has_any_key_pressed()` (home decl `include/pso/ui/keyboard/KeyboardKeyState.h:56`) + `any_pad_input_active()` (home `system/ControllerInput.cpp:46-55`, walks `controller_input_state.sources[i]` PadSlot thresholds ±10); EXACT-timeout tick → `transition_controller->suspend_update()` + `start_music_crossfade(0x1E)` :307-310 (audio-dark); past-timeout → `set_fallback_main_phase(2)` + `set_opening_movie_purpose(0)` + `request_main_phase(3)` :311-315 (phase-3 = movie row, `SystemBootInit.cpp:344`, RS S3); `attract_tick++` :316 | attract_tick/timeout | **MISSING**; `controller_input_state` (distinct from the bank!) has zero host def (same-turn grep) | **(b)** — inject tick counts + pad flags; canary: suspend fires at `tick==timeout` EXACTLY once, request(3) only after fade-ready | **HG3-292** |
| T7-S12 | title menu state machine + dispatch | `ui/TTitleMenu.cpp`: `run_task` `:125-138` (0→prompt, 1→cursor, 2→hold, `frame++`); `update_title_prompt` `:46-55` — A-mask-8 press (`test_pressed_buttons_packed(&controller_physical_port_bank.controllers[0], 8)` :48) → `state=1` :51, pulse `color.a` via `sin_table_lookup(182.04445f*(frame%120*3))` :53-54; `handle_cursor_input` `:57-79` — 4-item wrap (`cursor<0→3`, `>=4→0`), confirm mask 4 → `state=2` :75-78; `is_title_choice_confirmed` `:40-42` (`state==2`); dispatch `TTitleScene.cpp:277-286`: confirmed → `set_fallback_main_phase(2)` → `start_offline_seq()` :45-51 (fade-out + `transition_flags`) → handler = `state_handler_pmf_table[get_transition_cursor(this)]` (`:100-105`; cursor reader home `ui/TitleTextures.cpp:68-70` = the MENU cursor, 0..3) → handlers `:273-276`: `enter_chg_game_mode`→mode 0x15, `enter_game_mode_change`→mode 5, `request_change_game_mode`→mode 3, `open_keyboard`→`request_main_phase(19)` (row [19] = `TAdTrialModemChkGC` modem check, `SystemBootInit.cpp:360` — era fact: title choice 4 leaves phase 10 via 19, NOT a keyboard phase); release side `hide_title_layers_and_release` `:244-261` uses `disallow_rendering` (host twin EXISTS: `host/bringup/include/ladder7_run_prelude.h:169`) | menu state/cursor, transition_flags | **MISSING**; menu FONT rides T7-S4 | **(b)** — store-level: prompt→menu→confirm→cursor-index→mode store, all 4 handlers as open-fn fires (RS S11 shape) | **HG3-293** |
| T7-S13 | title pair-tables: load halves | `TTitleScene.cpp:93` real load is `initialize_lobby_title_scene` `:393-402`: `TLobby::update_menu_cursor()` :395 (home `TLobby_80132ED0.cpp:9-21` — language from `TAdSinglePlyChrSelectGC::get_global_state()` byte `+7`, default-language fallback on NULL :13-16 → MEMSCAN supply surface, cited as scope), `lobby_title_texture_list=&title_texture_list` :397, `load_streamed_prs_texture(names.archive[current_language], …)` :398 (21 texnames `:57-79`, `.data` list `:81`, JP/EN archives `:213-215`), `run_load_callback_table(lobby_load_callback_table, 4*8)` :399 — VERIFIED THIS TURN: **3 of the 4 load halves are empty no-ops** (`load_title_menu_resources`/`free_title_menu_resources` `ui/TTitleMenu.cpp:154-155`; `load/free_title_background_resources` `TTitleBackground.cpp:156-157`; `load/free_title_texture_resources` `ui/TitleTextures.cpp:55-56`) — only `clear_lobby_global_flags` (`TTitleBackground.cpp:151`) does work; the 7-pair `lobby_scene_release_callback_table[7]` `:132-140` (despite the name, loaded FORWARD in `init_game_mode_select_scene` `:467`) carries the midashi/player_tex/select_gamen/select_cursor/chr_sel_slot loaders + pair-1/pair-7 title lifecycle | language global, texlists, callback tables | **MISSING**; streamed-PRS law ALREADY REAL (host `prs_support.cpp`/`prs_golden_test.cpp`, `title_prs.dat` fixture present same-turn) | **(b)** — empty-half truth means the era walk is observable even before assets; leg = reverse/forward order witnesses + no-op arity | **HG3-294** |
| T7-S14 | title pair-tables: release halves + time-set reload | `TLobby::destroy_global_object` `TTitleScene.cpp:384-391`: `call_funcs_reverse__6TLobbyFPvUi(lobby_load_callback_table, sizeof)` :386 (walker home RS S6 = `ui/font/TFontSmallTask.cpp:251-261`) + `release_texture_list(lobby_title_texture_list)` :388; the 8→0 hop reload: `TAdTimeSetGC::apply_settings` calls `unload_lobby_scene_resources`/`load_lobby_scene_resources` (homes `TAdMultiChgGameModeGC_8023E7A8.cpp:427-432` = full 7-pair reverse release then forward reload — same-turn re-read of the RS S15 cite `:426-433`); phase-10 EXIT release = RS S17 `release_scene_resources` `:434-447` (fires only when LEAVING phase 10 — title choice 4 (modem phase 19), attract→movie (phase 3), and the decline quit path ALL reach it inside rung-7's span, unlike rung-6 where it was tail-only) | instance ptr, both pair tables | **MISSING** (walkers land with RS HG3-273/275) | **(b)** once RS legs land | **HG3-295** |
| T7-S15 | background/overlay draw surface | `TTitleBackground::render` `TTitleBackground.cpp:126-137`: `render_logo_with_default(get_title_texture_list(), 0xD, …)` :130, pulse quad (frames 150-170 of 300-mod, slot 0xC) `:25-50`, `draw_title_background_layer(…, 3, …)` :132, rings (slots 11/12, `draw_rotated_quad` ×5, `lobby_global_flags` spin laws `:80-87`) `:73-123`; overlay `TUnk_804EE470::render` `ui/TitleTextures.cpp:22-45` (slots 1/2 half-screen quads); menu model-draw `TTitleMenu::render` `ui/TTitleMenu.cpp:81-123` — records 0xA (prompt), 0xB..0xE (frames loop :98-102), cursor+0xF/0x13/0x17 (state 1) / 0x1B/0x1F/0x23 (state 2) via `TLobby::draw_with_camera(TLobby::get_record(i), …)` (record accessor home `ui/TitleTextures.cpp:62`; 46-sprite table `TTitleScene.cpp:161-208` over 46 `title_sprite_frames` `:109-156`) | 46-record bank, texlist binds, camera restore pair | **MISSING**; camera pair (`restore_full_screen_view`/`restore_saved_view`) = lane-J extern; model-draw = beyond P2 frame law (logo/quad lane only) | **(d)** — asset+model heavy; rung-7 exits AT store/menu-state level, pixels ride the gx seat (T7-S8 lineage) | gx/model lane (see §3) |

**Counting laws used above (python set-math this turn):** legs 284..295 = 12 (R6
271..283 = 13, no overlap); `game_mode_select_scene_callbacks` 23 pairs / 46
distinct callees; `lobby_scene_release_callback_table` 7 pairs / 14 distinct;
`lobby_load_callback_table` 4 pairs / 8 distinct (3 load halves empty);
trial scroll body = 10 lines (`0xA..0x13` group `0x1A`); `title_texture_entries`
21; `title_sprite_frames`/`lobby_record_table` 46; attract timeouts
`0x4B0`=1200 (NTSC), `0x320`=800.

## 2. Anchor rows needed in `pso_era_window_anchors`

Rung-6 shipped 4 rows (`host/bringup/crt0.c:39-66`). Verdict: **zero new anchor
rows for HG3-284..295** —

* window/scroll objects allocate through the already-bound W-POOL regime
  (RS §2 row 2 precedent: `new`/`mem_alloc` churn, no tag-space crossing);
* `controller_physical_port_bank` is host-defined data, not a guest address —
  the VI lane already defines it as a zero-init host global
  (`host/gx/vi_branch_driver.c:34`) with era layout mirrored
  (`host/gx/vi_era_harness.h:41-46`, context size 0x50 per
  `include/pso/system/ControllerInputState.h` same-turn fields `:16,:25-36`);
  the era `.bss` identity stays a comment, matching RS §2's
  `main_phase_state` discipline;
* text tables (`text_language_group_table`), callback tables and the 21-string
  `title_texture_entries` ride host copies + W-STATIC-TREE (RS §2 bullet 4 law).

**Carried watch item, now CONCRETE:** RS §2 flagged the row-2 (16 KB) POOL
re-audit if the S14 window chain stamps pool handles. This turn confirms the
trial window alone takes `lines = mem_alloc(0x1E * sizeof(TWindowMessageLine))`
(`TWindowMessage.cpp:340`), one `WindowScrollBar` (`TWindowMessageLog.cpp:311`),
plus the shared DrawContext singleton (`WindowDrawContext.cpp:240-247`) —
the **HG3-284 authoring card owns the row-2 capacity re-audit** (mirrors HG-2 §4
A2 discipline; not a new row).

## 3. Smallest testable legs + build-order rec (next 2 bringup cards)

Numbering continues the golden counter from RS: legs **HG3-284 … HG3-295**
(each = smallest assertion set that can fail, one canary trip per leg).

**Card BRINGUP-6 — "the trial screen is live" (HG3-284..290).** Seven legs, all
downstream of RS BRINGUP-5's mode-0x17 instance: HG3-284 window-build store
proof (10 text lines host-copied from group `0x1A`, entry array + `lines[]`
populated, reveal/reset law fires, glyph draw explicitly OUT of scope — T7-S4
boundary) → HG3-285 pure scroll-kinematics slice (`pending_scroll` ≡ same-body
law :130/:133, `step=max(15,0.15·d)` easing, clamp against `frame_size[1]−20`) →
HG3-286 cstick_y sign-split canary (injected `s16` value: `d<0` ⇒ `+0x19` bias,
`d>0` ⇒ `−0x19`, `d==0` ⇒ no-op; alpha ramp +0xC clamp) → HG3-287 agreement
state machine (confirmation=1/selection 0 ⇒ mode-8 store via `open_time_set_scene`;
selection 1 ⇒ `request_main_phase(0)`; B ⇒ state5→4 cancel law; canary =
**default selection is DECLINE** `TAdAgreementBaseGC.cpp:129`, so an
un-injected run must quit, never agree) → HG3-288 cursor nav slice (wrap,
repeat-delay(6,1), directions-released gate latch-clear) → HG3-289 close/4.5
tick timing (window NULL-out + self-destruct order) → HG3-290 the 0x17→8 hop as
the first two-store end-to-end walk over RS S12/S13 machinery. **What-green:**
first interactive screen provably consumes input at the state level and moves
the scene chain without ever touching a glyph. Cost: **2 builds** (all
machinery — slice generator, byte anchors, counted-extern roster — exists; the
only new host artifact is an input-injection header, see R-D). Depends on: RS
BRINGUP-5 accepted; ROSTER lane's 23-pair callee census for the extern roster
(theirs is authoritative; this doc re-lists the table only as RS already did).

**Card BRINGUP-7 — "arrival at the live title" (HG3-291..295).** HG3-291 title
ctor roster (shell `TTitleBackground`/`TTitleMenu`/`TUnk_804EE470` objects with
`m_name` pins; camera/particle/fade members counted externs, same lane-J roster
as RS S10/P1-P2 — never newly blocked; attract timeout value pinned from
`get_video_standard()` canary) → HG3-292 attract machine walk (inject
`attract_tick`, prove suspend fires at `==timeout` once, `request_main_phase(3)`
with `fallback=2` only past timeout + fade-ready; no-card bounce linked to RS
HG3-281) → HG3-293 menu state machine + all-4-handler dispatch store proof
(fact pinned: choice 4 = `request_main_phase(19)` = MODEM-CHECK row
`SystemBootInit.cpp:360`, not a keyboard phase) → HG3-294 4-pair empty-half walk
(order witnesses: forward load via `initialize_lobby_title_scene`, reverse
release via `destroy_global_object`, arity of 3 no-ops asserted) → HG3-295
7-pair unload/reload cycle at the 8→0 hop (full reverse-then-forward walk over
both release tables). **What-green:** `main_phase_state.cur==0xA` the whole
span, `multi_chg_game_mode_state` 0x17→8→0 with `chg_game_mode_state_synced`
one-behind law, title scene instance alive with menu state 0→1→2 reachable by
injected buttons, present counter still 1/frame. Cost: **2–3 builds**. Hard
dependencies: RS R-A supply decision (MEMSCAN lane — without it the era machine
rests at time-set, `TAdTimeSetGC.cpp:888-891` per RS S15; HG3-295 degrades to a
NULL-branch-only leg), and ROSTER's census for the 7-pair extern arity.

**Then (not these 2 cards):** gx seat takes T7-S8 (marker-quad byte-diff vs the
logo gate, riding `host/gx/run_window.sh`, needs an `f256_player_tex` fixture —
absent from `host/bringup/fake_disc` this turn) and T7-S15 (model-record draw
surface beyond the quad law); font lane takes T7-S4 (`S18all04.fon` supply +
glyph pages; boot-spine :74 already frames a *text-bearing* title as exactly
this lane + phase-10 machine).

## 4. Risks / unknowns (rung-7 deltas only)

- **R-D (NEW, host-ownership collision):** `ControllerInputState
  controller_physical_port_bank` is currently DEFINED in `host/gx/vi_branch_driver.c:34`.
  Bringup legs HG3-286/287/288/292/293 need to inject fields into the SAME
  global. Two owners of one definition = link/behavior collision; the VI lane's
  header comment (`host/gx/vi_era_harness.h:28-46`) asserts the layout. Fix
  BEFORE BRINGUP-6 builds: promote the bank (and `controller_input_state`,
  which today has NO host def — same-turn grep) into the bringup prelude as the
  single era-shaped definition with an injection API; VI lane consumes it as an
  extern. Do not fork a second struct.
- **R-E (keyboard-mirror law, verified):** every front-end read pairs a pad
  source with a KEYBOARD mirror that is a DIFFERENT global
  (`controller_input_state.keyboard` in `TAdSelectCurGC.cpp:141/:146/:159`,
  `TTitleMenu.cpp:48/:61/:68/:75`; the bank's `.keyboard` slot is used only by
  `TTitleScene.cpp:302`'s separate `hardware_keyboard_state`). An injection
  seam that drives only the bank will silently miss cursor/A/confirm paths —
  the leg canaries must press BOTH sources or pin which one a test drives.
- **R-F (`is_input_phase_active` un-censused, NEW gate family):**
  `TAdMultiChgGameModeGC_8023E7A8.cpp:142-175` returns 1 for phase 10 ONLY at
  modes {7,14,24} — i.e. 0 for trial/time-set/title modes. It does NOT gate the
  agree/decline reads above (those hit the bank directly), but its callers are
  not censused this turn; if any front-end path consults it, rung-7's injected
  input could be gated off. Carry a caller-census sub-leg on BRINGUP-6
  (cheap: one grep at authoring time), same posture as RS R-C's gate global.
- **R-G (busy-loop hazard, verified):** `TAdTrialAgreementGC::wait_for_load`
  `:42-49` spins `render_tick()` until `probe_card_slot`/`poll_memcard_result`
  clear, and `check_card_file` `:64/:68` calls it TWICE — reached ONLY when a
  card probe says "card present". Host posture: keep `probe_card_slot(0)==0`
  (first-boot truth) so the loop is never entered; if the MEMSCAN supply card
  ever fakes a present card into the trial ctor, `wait_for_load` must land as a
  REAL bounded slice, not a shim — flag now because the supply decision
  (their lane) and this loop (rung-7's screen) are on a collision course.
- **R-H (cite drift in RS, harmless but fixed here):** RS §1 S10/S11/S12 cite
  lines shifted by −2..+2 against the pin re-read this turn (e.g. the 23-pair
  table is `:105-129` and the 7-pair table `:132-140`, RS wrote `:106-131`/
  `:134-142`; `TAdPlayerLoadGC::get_instance()==0` gate is `:521`, RS wrote
  `:520`). No law changed; rung-7's numbers supersede for these regions.
- **Sibling-lane scope collisions (per brief, scope-lines only):** lane ROSTER
  owns the game-mode-table callee census (`docs/p2-bringup5-roster-1.md`) —
  this doc re-lists pair tables ONLY as RS S3/S7/S10 already did, and their
  per-callee classing is authoritative for the extern rosters in §3; lane
  MEMSCAN owns SystemSaveData supply (`docs/p2-memcard-1.md`) — this doc names
  the trial screen's consumption points (R-G, T7-S5 language byte `+7` via
  `get_global_state`, attract bounce) as a boundary; no claim is made about
  their files, which were not read or waited on.

## Method / honesty

Every era file:line above was opened this turn from the pin worktree (no memory
cites; RS regions re-opened to check links); every host status from a same-turn
grep of `host/bringup|gx` (bank: `vi_branch_driver.c`/`vi_era_harness.h`; stubs:
`ladder7_seam.cpp:119-120/:173`; counters: `ladder3_test.cpp:157`;
`disallow_rendering`: `ladder7_run_prelude.h:169`; fixture list:
`ls host/bringup/fake_disc`). Zero builds, zero repo writes outside this doc.
Counts in §1's counting-laws paragraph are python set-math over this turn's
reads, not prose.
