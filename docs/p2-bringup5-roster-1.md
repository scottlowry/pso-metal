# P2-BR5ROSTER — BRINGUP-5 per-pair callee roster (3 callback tables @ pin)

Lane ROSTER, 2026-11-27 (card `t_1e3aad47`). ZERO-BUILD seat: no make/parse/gen_waivers,
no host/** writes, no waiver-header touch. Discharges `docs/p2-rung6-seams-1.md` §4 **R-B**
("enumerate per-pair before the build") for card BRINGUP-5 (HG3-275..281).
All era cites opened this turn from the pin
`DECOMP_ROOT=/Users/scott/Hermes/pso-decomp/pso-decomp-port-pin` @ `6b38b53837`
(`src/pso/...` paths below are relative to it); every host status is a same-turn grep of
`host/bringup/**` + `host/gx/**`. Repo tip verified this turn: pso-metal `main` @ `573ba98`.

**Grammar** (rung-4/5, per `p2-rung6-seams-1.md` §1): **(a)** verbatim slice, zero externs ·
**(b)** slice + prelude externs · **(c)** crosses an allocator/frame-pump/pixel-supply seam ·
**(d)** blocked — named lane first. Host status: **REAL** (executing era-logic backing) ·
**GLUE-ONLY** (name present but counter/fail-loud/comment only) · **MISSING** (zero hits).
Table names below are VERBATIM from the era source rows, not paraphrases.

## 1. Table anchors (pin, read this turn)

- `game_mode_select_scene_callbacks[23]` — `ui/frontend/TAdMultiChgGameModeGC_8023E7A8.cpp:105`
  (declaration; pair rows `:106–128`), run forward by `run_load_callback_table` from
  `init_game_mode_select_scene` (`:448–492`).
- `lobby_scene_release_callback_table[7]` — same file `:132` (rows `:133–139`), "run after the
  table above and released before it".
- `logo_load_callback_table[9 pairs]` — `ui/TLogo.cpp:39` as `void (*logo_load_callback_table[18])(void)`
  (flat 18 entries, rows `:40–49`), walked fwd at `TLogo.cpp:81`, reverse at `:119` (`exit_logo_scene`).
- Logo table rows 1–5, 6, 7, 8 reuse the SAME callee homes as game-mode rows 1–5, 13, 14, 19;
  only row 9 (`load/free_adx_logo_texture`) is logo-exclusive. **Distinct callees across all
  three tables: 62** (46 + 14 + 2), verified `sort -u` count this turn.

## 2. `game_mode_select_scene_callbacks` — 23 pairs (load half | release half)

Homes are DEFINITION sites at the pin (grep of `Class::method(` / free-fn `name(` in `$DECOMP_ROOT/src`).

| # | load fn — home — host — grade | release fn — home — host — grade |
|---|---|---|
| 1 | `TCameraCtrl::reset_view_state` — `camera/TCameraCtrl.cpp:546` — MISSING — **(d)** J | `TCameraCtrl::reset` — `camera/TCameraCtrl.cpp:577` — MISSING — **(d)** J |
| 2 | `TCameraCtrl::init_default_view` — `camera/TCameraCtrl.cpp:1033` — MISSING — **(d)** J | `TCameraCtrl::reset_default_view` — `camera/TCameraCtrl.cpp:1055` — MISSING — **(d)** J |
| 3 | `TLobby::update_keyboard_menu` — `unk/TLobby_80132ED0.cpp:31` — MISSING — **(d)** font | `TLobby::free_keyboard_soft_resources` — `unk/TLobby_80132ED0.cpp:25` — MISSING — **(d)** font |
| 4 | `register_texture_f256_ad_pic_2` — `ui/frontend/TAdBackGC.cpp:90` — MISSING — **(c)** | `TAdSinglePlyChrSelectGC::free_global` — `ui/frontend/TAdBackGC.cpp:84` — MISSING — **(c)** |
| 5 | `register_texture_f256_backtext` — `ui/frontend/TAdBackTxtGC.cpp:86` — MISSING — **(c)** | `TAdSinglePlyChrSelectGC::free_singleton_805D65C0` — `ui/frontend/TAdBackTxtGC.cpp:80` — MISSING — **(c)** |
| 6 | `register_texture_f128_ray` — `ui/frontend/TAdRayGC.cpp:160` — MISSING — **(c)** | `TAdRayGC::destroy_instance` — `ui/frontend/TAdRayGC.cpp:152` — MISSING — **(c)** |
| 7 | `load_multi_ply_chr_select_resources` — `ui/frontend/TAdChrSelPlayerGC.cpp:30` — MISSING — **(c)** | `free_multi_ply_chr_select_resources` — `ui/frontend/TAdChrSelPlayerGC.cpp:34` — MISSING — **(c)** |
| 8 | `TAdTimeSetGC::load_scene_resources` — `ui/frontend/TAdTimeSetGC.cpp:135` (era body EMPTY) — MISSING — **(a)** | `TAdTimeSetGC::free_scene_resources` — `:138` (era body EMPTY) — MISSING — **(a)** |
| 9 | `register_texture_f512_multiplay` — `ui/frontend/TAdGameStartGC.cpp:163` — MISSING — **(c)** | `TAdGameStartSubGC::destroy_instance` — `ui/frontend/TAdGameStartGC.cpp:155` — MISSING — **(c)** |
| 10 | `TAdSinglePlyChrSelectGC::alloc_char_buffers` — `ui/frontend/TAdSinglePlyChrSelectGC_801FBF54.cpp:3140` — MISSING — **(c)** ⚠ deny-roots | `TAdSinglePlyChrSelectGC::free_callbacks` — same file `:3128` — MISSING — **(c)** ⚠ deny-roots |
| 11 | `TAdMultiChgGameModeGC::load_scene_resources` — `ui/frontend/TAdMultiChgGameModeGC.cpp:35` (EMPTY) — MISSING — **(a)** | `TAdMultiChgGameModeGC::free_scene_resources` — same `:34` (EMPTY) — MISSING — **(a)** |
| 12 | `load_multi_confirm_textures` — `ui/frontend/TAdMultiOkGC.cpp:21` — MISSING — **(c)** | `free_multi_confirm_textures` — `ui/frontend/TAdMultiOkGC.cpp:23` — MISSING — **(c)** |
| 13 | `register_texture_logo_e` — `ui/frontend/TAdSEGAlogo.cpp:103` — **REAL** `host/gx/dl/gx_logo_driver.cpp:236,865` — landed | `TLogo::free_held_object` — `ui/frontend/TAdSEGAlogo.cpp:95` — **REAL** `gx_logo_driver.cpp:875` (host-local mirror leg) — landed |
| 14 | `load_sonic_logo_texture` — `ui/frontend/TAdSoniclogo.cpp:60` — **REAL** `gx_logo_driver.cpp:234` — landed | `free_sonic_logo_texture` — `ui/frontend/TAdSoniclogo.cpp:58` (EMPTY) — **REAL** `gx_logo_driver.cpp:231` — landed |
| 15 | `init_chat_log_heap` — `unk/text_8026ACDC.cpp:7` (`new THeap(0x2000,8)`) — MISSING — **(c)** | `free_chat_log_lines` — `unk/text_8026AC50.cpp:7` — MISSING — **(c)** |
| 16 | `register_player_prate` — `ui/frontend/TAdPlyPltAnimGC.cpp:126` — MISSING — **(c)** | `free_player_prate` — `ui/frontend/TAdPlyPltAnimGC.cpp:119` — MISSING — **(c)** |
| 17 | `load_data_buffers` — `boss/derolle/TObjBoss2Tunnel.cpp:1049` (ScreenSnapshot) — MISSING — **(c)** | `TObjBoss2Tunnel::free_buffers` — same `:1075` (`xfree`+`release_capture_texture`) — MISSING — **(c)** |
| 18 | `KeyboardKeyState::reset_global_keys_b` — `ui/keyboard/KeyboardKeyState.cpp:310` — MISSING — **(b)** | `KeyboardKeyState::reset_global_keys_a` — same `:306` — MISSING — **(b)** |
| 19 | `TWindowCtrlMain::reset_windows` — `ui/window/TWindowCtrlMain.cpp:374` (→`reset_all`:377 →`load_hud_palette_textures`) — MISSING — **(c)** ⚠ GVPL | `TWindowCtrlMain::free_all_windows` — same `:393` — MISSING — **(c)** |
| 20 | `TLobby::set_ending_callback` — `ui/window/TWindowSubStatusDisp_800FBD74.cpp:262` (1 store, font-metrics ptr) — MISSING — **(b)** | `TLobby::free_ending_callback` — same `:260` (EMPTY) — MISSING — **(a)** |
| 21 | `load_keyboard_resources` — `ui/keyboard/KeyboardKeyState.cpp:22` (EMPTY) — MISSING — **(a)** | `free_keyboard_resources` — same `:21` (EMPTY) — MISSING — **(a)** |
| 22 | `alloc_quest_download_buffer` — `ui/frontend/TAdSinglePlyChrSelectGC_8028D5D4.cpp:117` (`TQuestVmDownLoadServer::alloc(0x28000)`) — MISSING — **(c)** ⚠ quest-VM | `TLobby::free_chr_select_singleton` — same `:108` (`safe_free`) — MISSING — **(c)** |
| 23 | `load_draw_line_task_resource` — `ui/frontend/DrawLine.cpp:144` (`new RelocatedFile`) — MISSING — **(c)** | `free_draw_line_task_resource` — same `:152` — MISSING — **(c)** |

## 3. `lobby_scene_release_callback_table` — 7 pairs

| # | load fn — home — host — grade | release fn — home — host — grade |
|---|---|---|
| 1 | `initialize_lobby_title_scene` — `ui/frontend/TTitleScene.cpp:393` (also walks `lobby_load_callback_table[4]` + `TLobby::reset_counter` + `set_current_floor_area(0)`) — MISSING — **(c)** | `TLobby::destroy_global_object` — `ui/frontend/TTitleScene.cpp:384` (reverse-walks `lobby_load_callback_table` → rides S6 walker/HG3-273) — MISSING — **(c)** |
| 2 | `load_title_midashi_texture` — `ui/frontend/TAdTitleGC.cpp:38` — MISSING — **(c)** | `free_title_midashi_texture` — `ui/frontend/TAdTitleGC.cpp:46` — MISSING — **(c)** |
| 3 | `load_player_tex_texture` — `ui/frontend/TAdTitleTexGC.cpp:40` — MISSING — **(c)** | `free_player_tex_texture` — `ui/frontend/TAdTitleTexGC.cpp:57` — MISSING — **(c)** |
| 4 | `load_select_gamen_texture` — `ui/frontend/TAdSelectTxtGC.cpp:125` — MISSING — **(c)** | `free_select_gamen_texture` — `ui/frontend/TAdSelectTxtGC.cpp:119` — MISSING — **(c)** |
| 5 | `load_select_cursor_texture` — `ui/frontend/TAdSelectCurGC.cpp:221` — MISSING — **(c)** | `free_select_cursor_texture` — `ui/frontend/TAdSelectCurGC.cpp:214` — MISSING — **(c)** |
| 6 | `load_chr_sel_slot_texture` — `ui/frontend/TAdChrSelSlotGC.cpp:177` — MISSING — **(c)** | `free_chr_sel_slot_texture` — `ui/frontend/TAdChrSelSlotGC.cpp:169` — MISSING — **(c)** |
| 7 | `TLobby::update_menu_cursor_selection` — `ui/frontend/TAdMultiPlayerNoGC.cpp:28` — MISSING — **(c)** | `TLobby::free_global_object` — `ui/frontend/TAdMultiPlayerNoGC.cpp:43` — MISSING — **(c)** |

All five `load_*_texture` legs index a 5-slot per-language `names[]`/`lists[]` table on
`current_language` (era bodies read this turn) — the language global is a shared prelude extern.

## 4. `logo_load_callback_table` — 9 pairs (`ui/TLogo.cpp:39–49`)

| # | pair (flat array order = fwd walk order) | grade |
|---|---|---|
| 1 | `TCameraCtrl::reset_view_state` / `TCameraCtrl::reset` — same homes as GMS §2 row 1 | **(d)** J |
| 2 | `TCameraCtrl::init_default_view` / `TCameraCtrl::reset_default_view` — GMS row 2 homes | **(d)** J |
| 3 | `TLobby::update_keyboard_menu` / `TLobby::free_keyboard_soft_resources` — GMS row 3 homes | **(d)** font |
| 4 | `register_texture_f256_ad_pic_2` / `TAdSinglePlyChrSelectGC::free_global` — GMS row 4 homes | **(c)** |
| 5 | `register_texture_f256_backtext` / `TAdSinglePlyChrSelectGC::free_singleton_805D65C0` — GMS row 5 homes | **(c)** |
| 6 | `register_texture_logo_e` / `TLogo::free_held_object` — GMS row 13 homes | **landed REAL** |
| 7 | `load_sonic_logo_texture` / `free_sonic_logo_texture` — GMS row 14 homes | **landed REAL** |
| 8 | `TWindowCtrlMain::reset_windows` / `TWindowCtrlMain::free_all_windows` — GMS row 19 homes | **(c)** |
| 9 | `load_adx_logo_texture` — `ui/frontend/TAdADXlogo.cpp:65` / `free_adx_logo_texture` — `:61` (EMPTY) — **REAL** `gx_logo_driver.cpp:235,232` | **landed REAL** |

HG3-273 (BRINGUP-4) fires this table once via real `call_funcs_reverse` — rows 1–5, 8 are the
counter-extern members rung-6 §3 already anticipated ("(d) members counter-externed"):
**confirmed, 6 of 9 pairs counter-only, 3 pairs already REAL.** No new blocker classes appear here.

## 5. Census summary (62 distinct callees)

- **REAL 6 / GLUE-ONLY 0 / MISSING 56** (REAL = the six `gx_logo_driver.cpp` legs, §2 rows 13–14 + §4 row 9).
- Grade split: **(a) 7** (era-empty / zero-extern bodies: GMS rows 8, 11, 20-rel, 21 halves) ·
  **(b) 3** (GMS row 18 both halves + row 20 load) · **(c) 40** · **(d) 6** · **landed 6**. 6+7+3+40+6 = 62 ✓
- Substring-collision note: `TCameraCtrl::reset`'s naive grep hits `controller_reset_requested` /
  `poll_reset_request` (`ladder7_test.cpp:71,131`) — unrelated globals; word-scoped re-grep
  (`reset_view_state|init_default_view|reset_default_view`, `TCameraCtrl` in host) shows the ONLY
  camera host presence is the counted `restore_full_screen_view` stand-in
  (`ladder7_seam.cpp:172`, `g_rfs_calls`) — never callable as `reset` backing.
- Similarly `destroy_instance` in `ladder7_seam.cpp:241` is an unrelated LB-3 fail-loud free fn;
  it does NOT back `TAdRayGC::destroy_instance` / `TAdGameStartSubGC::destroy_instance` (both MISSING).

## 6. (d) blocker list (named lanes, cited not re-derived)

1. **lane-J NJS re-pin** — GMS/LOGO rows 1–2, 4 `TCameraCtrl` halves (homes `camera/TCameraCtrl.cpp:546/577/1033/1055`).
   RS §1 P1/P2, `docs/p2-rung5-seams-1.md:43–44`: first blocker `NJS_OBJECT_size` @ `NinjaTypes.h:162`
   (measured err 2156B). Ladder7 backing rule `docs/p2-ladder7-play23.md:150` ("all lane-J homes →
   the counter is the rung-5-faithful backing") — BRINGUP-5 rides the same counter regime.
2. **Font lane** (`load_menu_font` / TLanguage alphabet+text resources, boot-spine S10; named in
   rung-6 S14) — `TLobby::update_keyboard_menu` (`TLanguage::load_text_resources` + `load_fonts`,
   `unk/TLobby_80132ED0.cpp:31`) and `TLobby::free_keyboard_soft_resources` (`TLanguage::free_alphabet_textures`
   + `free_text_resources`, `:25`). `TLanguage` is comment-only on host (`gx_logo_driver.cpp:141–142`).
3. No audio callee is IN these tables — cross-check answer: `load_floor_sound_bank`
   (`TAdMultiChgGameModeGC_8023E7A8.cpp:488`) and `apply_bgm_col_ctrl` (`:332`) live in the
   **init/update bodies**, not the callback tables, so the audio-dark fail-loud lane blocks
   HG3-276 legs, never a table row.
4. Watch-not-(d) flags on (c) rows: row 10 `new CharacterFile`/`new SystemSaveData` (FORMAT deny-roots,
   phase0 waiver-machine law — needs the boot-slice layout verdict); row 19 `reset_windows→reset_all`
   calls `load_hud_palette_textures` (multi-GVPL bank open-defect); row 22
   `TQuestVmDownLoadServer::alloc` (quest-VM `is_ready` boot trap; carrier already host-present,
   `ladder7_test.cpp:77,175`). None changes the (c) disposition; each is a canary candidate.

## 7. Shared-extern closure census (host hits this turn)

`load_streamed_prs_texture` **0** · `release_texture_list` **0** · `update_menu_cursor` **0** ·
`call_funcs_reverse` **0** · `run_load_callback_table` **0** · `safe_free` **0** ·
`load_hud_palette_textures`/`reset_all` **0** · `RelocatedFile`/`ScreenSnapshot` **0** ·
`TQuestVmDownLoadServer` 27 (carrier/zero-defs) · `BmlResource` 7 (carrier) · `THeap` 15
(OS alloc layer `osalloc_era.c`, C++ class absent). Consequence: the 13 unlanded
`load_streamed_prs_texture` legs and the 20 `release_texture_list`-style free legs are ALL
closed by the SAME two prelude externs (counted, per R-B) — the roster needs **no new host
implementation class** beyond the rung-6 §3 BRINGUP-4/5 machinery; the era PRS decode chain
(Milestone Q/AG) stays unattached to them by design (BYO-content supply rides the gx/dl lane).

## 8. R-B answer (per-pair enumeration for BRINGUP-5)

- 23 + 7 + 9 = **39 pair rows / 78 half-rows / 62 distinct callees**; rung-6's "~46 distinct
  callees from the 23-pair table" measured against this roster: GMS alone = 46 distinct halves ✓.
- Ride policy, per R-B and rung-6 §3: **6 landed** (skip re-slicing; counter them as already-real);
  **7 (a) + 3 (b)** = verbatim-slice candidates (10 trivial/extern-bound halves, zero seam risk);
  **40 (c)** = counter/fail-loud externs EXCEPT the four pairs BRINGUP-5 needs live for store-proof
  shape (GMS 11 `(a)(a)` empty pair is the only self-contained init witness); **6 (d)** = counter
  externs naming lane-J / font-lane owners on the row.
- Drift law preserved: table DATA copied host-side (rung-6 §2 — no new anchor rows), row-count
  assertion 23/7/9 baked as the census canary (drift = loud exit, same discipline as HG-2).

## Method / honesty

Zero builds, zero repo writes outside this doc. Every era home line is a same-turn grep + sed
read at pin `6b38b53837`; every host status a same-turn `grep -rni` over `host/bringup` +
`host/gx` (positive controls: `get_logo_texture_e_handle`, `TQuestVmDownLoadServer` returned hits;
all zero-rows re-run word-scoped). No disc paths touched, no asset bytes read, no compiled hashes
emitted. Grades are SUGGESTIONS per the rung-4/5 grammar for the build cards to accept or override.
