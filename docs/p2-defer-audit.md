# P2-V — DEFER-row + assumption verification sweep

Re-test of every **DEFER** verdict row (18) and every assumption **A1–A8** from
`docs/p1-init-ledger.md` (75-row ledger, boot-spine deltas §7), executed against
`pso-decomp @ 6b38b5383` by reading function bodies + callers. Question per item:
*can this actually be skipped while still reaching the first logo frame?*

Test set: 18 DEFER rows + 8 assumptions = **26 items**, plus a deref cross-check
of §4's 23 checklist steps.

**Verdict tally: CONFIRMED-DEFER 16 · RISK 3 (rows 49/50 + A6) · REFUTED 4 (A3, A4, A5, A7 —
all assumption-level; zero DEFER rows refuted for the logo frame).**
§4 cross-check found **one real gap** (W4: TLanguage fonts on the logo path).

Verdict vocabulary: CONFIRMED = skip is inert for first logo frame, evidence cited.
REFUTED = the stated assumption does not hold; minimal work item listed.
RISK = skippable on static evidence, decided at first-frame bring-up.

## 1. DEFER-row re-test (18)

| # | Action | Verdict | Evidence (source-verified) |
|---|---|---|---|
| 22 | `TObjBoss2Tunnel::enable_and_trigger` :292 | **CONFIRMED** | Static default is already the value it sets: `display_copy_filter_enabled = 1` (TObjBoss2Tunnel_8036EF0C.cpp:10 vs :22). Body = re-assert 1 + `GXSetCopyFilter` with same value (:12-16). Only consumers: CameraCapture.cpp:473 (display-copy path) and TObjBoss2Tunnel.cpp:853-856 (boss in-game) — both read the global, value identical without the call. |
| 23 | `init_pad_rumble` :293 | **CONFIRMED** | Body → `reset_pad_rumble_effects_and_channels(0)` (PadRumble.cpp:288-290) which writes all-zeros (:242-255) = `.bss` default. The per-frame consumer `update_pad_rumble` (live via `poll_input_devices_frame`, SystemBootInit.cpp:96) routes to `PADControlMotor(STOP)` regardless: `get_pad_rumble_client` returns `NO_CLIENT(0xFFFF)` because `get_player_by_client_id==NULL` during logo (PadRumble.cpp:198-213). Skip is state-equivalent. |
| 38 | `TLobby::load_now_loading_texture` :311 | **CONFIRMED** | Sole consumer of `loading_texture_list` = NOW-LOADING overlay `TLobby_803382E4.cpp:96-121` (`set_texture_list` + strips + compass), lobby scope, gated on `lobby_request_flags & 2` — a bit set only by this row (:141). Zero refs in the logo scene files. Pairing caveat → see row 64/A7. |
| 43 | `load_info_icon` :316 | **CONFIRMED** | Reader is NULL-guarded: `if ((u32)info_icon_texture != 0)` before `set_texture_list` (TLobby_803382E4.cpp:177-178). Info-icon overlay is player-load/lobby scope. |
| 44 | `alloc_item_drop_memory_pool` :317 | **CONFIRMED** | Sole consumer `floor_set_archive_pool = new AramPool(item_drop_…)` (TSetManager.cpp:1529) = floor load, in-game. `free_item_drop_memory_pool()` is itself an empty stub (:1557) — the teardown half is a no-op even in retail. |
| 45 | `alloc_player_motion_aram` :318 | **CONFIRMED** | Sole consumer `upload_player_motion_data` (TWindowSnapShot.cpp:29) — snapshot window, in-game. Reads need `player_motion_archive` which doesn't exist pre-game. |
| 46 | `TAdWarningBaseGC::init_heap` :319 | **CONFIRMED** | Consumers all warning-scene-local (TAdWarningBaseGC.cpp:139/186/192) + `is_warning_aram_load_done` (QuestOpcodeHandlers.cpp:5415) = quest opcodes, in-game. Values written except the ARAlloc are 0 = bss default. |
| 47 | `TLobby::load_ask_dictionary` :320 | **CONFIRMED** | ARAM dictionary + `start_askdic` serve the on-screen keyboard UI (TWindowKeyBoardBase scope); no caller on the logo/first-title path. |
| 48 | `request_chara_making_archives` :321 | **CONFIRMED** | Char-create texture AFS pulls (TCharaMaking.cpp:558-583); buffers only read by char-create screens. Bonus: retail runs ~20 synchronous `open_afs_file_wait_render` calls here at boot — skipping strictly shortens the port boot. |
| 49 | `load_effect_resources` :322 | **RISK** | No refs in TAdSEGAlogo/TAdSoniclogo/TAdADXlogo. BUT `TParticleCtrl::create_quad_sprites` (live in `init_logo_scene`, TLogo.cpp:89) builds prototypes from `particle_definitions` whose static entries reference `&shared_effect_texture_list` (ParticleDefinition.cpp:132-147), and the Col/Scl prototype `render()` overrides are non-empty (TQuadSpriteCol.cpp:23+; only base `TQuadSprite::render` is empty, :18). Residual risk = whether prototypes draw zero-node lists (believed yes) . Deciding evidence: first logo frame — if particle-quad output is missing/crashes, promote this row. |
| 50 | `init_particle_sprite_atlas` :323 | **RISK** | Same atlas-rect refs as row 49. Pure 4×4 arithmetic into `particle_sprite_atlas_uv_rects` (ParticleDefinition.cpp:62-93), zero external deps → **recommend re-class to KEEP-AS-IS**: costs nothing and deletes the atlas half of the A6 risk permanently. |
| 53 | `init_disc_error_messages` :326 | **CONFIRMED** (conditional) | Message table (`disk_error_msg_table`) is consumed only when `DVDGetDriveStatus()` ∈ {−1,4,5,6,11} (run_disk_error_check, TLobby_800F8AA8.cpp:1040-1113 → `show_disk_error_msg`). §4 step-4's shim contract (always present/ready) keeps it inert. If ever activated, the body reads absolute guest `0x8000AA30` (SwitcherLaunch.cpp:52, `p[-0x55d0]` from 0x80010000) — BootInfo-class glue needed; keep as link stub. |
| 54 | `init_keyboard_buffer` :327 | **CONFIRMED** | `network_info_work` sole consumer = `load_network_info` (text_80145A94.cpp:25-50); callers of that = 3 net screens (TAdExNetConfirmGC:43, TAdTrialModemChkGC:17, TAdEnterPPPPassGC:172) + this row itself. No logo consumer. Net-screen lane must call this init first (unchanged from ledger). |
| 55 | `TAdPlayerLoadGC::load_banner_files` :328 | **CONFIRMED** | Banner buffers consumed only inside TAdPlayerLoadGC (player-load screen; :320). Function self-NULL-guards on OOM (:291-306); skipping leaves the same NULL state. |
| 56 | `TLobby::load_player_data` :329 | **CONFIRMED (logo)** | `init_memcard(0,0,player_data_load_buffer,0)` — memcard lane; `memcard_access_active` writers are entirely char-select (TAdSinglePlyChrSelectGC_801FBF54.cpp), unreachable at logo. Title-chain exposure is logged under A5 (separate work item, not this row). |
| 57 | `init_account_entries` :330 | **CONFIRMED** | Zeroes `.data`/bss-state + `load_network_configs` (sole callers NetworkConnect.cpp:850↔init_account_entries). Consumers = net confirm/PPP screens. Inert for logo+title. |
| 58 | `load_ha_u032_kage` :331 | **CONFIRMED** | `blob_shadow_texture_list` readers NULL-guard (BlobShadow.cpp:53; LobbyGoGoBall.cpp:528). Blob shadows = in-game ground decals. |
| 62 | `load_ply_level_tbl` :335 | **CONFIRMED** | `ply_level_table` dereffers live only in `get_ply_level_table_row` callers = player XP/level math (TObjPlayer_801AF2F0.cpp:2406/2539/2582). Row 32's `init_buffers` (TObjPlayer_8023E480.cpp:115-121) re-verified: pure xmalloc+memset, no table touch. `.cpt` cipher stays in the save lane. |

Re-verified side-claims: row 34 `init_quest_vm_stub` **is** empty (TQuestListVM_8007EAF8.cpp:32 `{}`),
row 39 `chara_making_init_stub` **is** empty (TCharaMaking.cpp:584), row 59 `system_init_stub`
**is** empty (text_800FFE04.cpp:6) — the ledger's three "verified empty" tags hold.

## 2. Assumption re-test (A1–A8)

| A | Claim (ledger §8) | Verdict | Deciding evidence / minimal work item |
|---|---|---|---|
| A1 | Row 28/D3: logo uses alpha-depth state | **CONFIRMED — keep** (upgrades `yes*` → `yes`) | Callbacks NULL-guarded (ChunkModelDraw.cpp:742-743; ModelDrawContext.cpp:39-40) so *installing* nothing never crashes — but `restore_saved_depth_alpha_state()` is also called from **non-optional** sites: run_disk_error_check tail (TLobby_800F8AA8.cpp:1113, inside every is_ready()!) and the lobby overlay tail (TLobby_803382E4.cpp:119). Without row 28, `depth_buffer_settings`/`saved_depth_alpha_scene_state` stay zero-init (z_update=0, z_func=0, alpha_ref=0) and those live calls emit *wrong* state, not absent state. Keep blocking. |
| A2 | Row 34/D4: quest-VM never ready during logo | **CONFIRMED — + mechanism correction** | `quest_download_server_ready` (TLobby_800F8AA8.cpp:91) has **zero writers in the entire tree** → `is_ready()` can never return true → the phase-0 shutdown trap (TLogo.cpp:112-114) is structurally unreachable, not just assumption-guarded. **But**: `is_ready()` first calls `TLobby::wait_ready_loop` (:1371→:981-994) — an infinite loop that only exits when `run_disk_error_check()==1`, which requires `DVDGetDriveStatus()` ∉ {−1,4,5,6,11}; each iteration does a full-screen quad draw + `TAdPlayerLoadGC::set_state(1)` + **nested `render_tick()`**. → **W2**: link-stub `is_ready(){return 0;}` (preferred — kills the per-frame nested present) or hard DVD-shim contract: non-error drive status from frame 0. |
| A3 | D5: shipped `logo_clear_flag`=0, progressive-ready true | **REFUTED — biggest finding** | `s32 logo_clear_flag = 1;` **ships as 1** (TAdVIConfigGC.cpp:87); cleared only by `set_field48_clear_global()` when the user answers the VI dialog (:179-181). Saves the logo anyway: state-0 consults the flag only under `check_progressive_ready()==1`, which needs `VIGetDTVStatus()!=0` (:226-236; zero-controller-input → Previous-test false too). → **W1**: make `VIGetDTVStatus()→0` an explicit logo-min glue contract (or force-init the flag to 0); otherwise the first "logo frame" is a TAdVIConfigGC yes/no dialog needing font+real input that don't exist until steps 21-22. **Boot-spine U3 resolved.** |
| A4 | D5: `has_network_adapter(1)` nonzero → modem check skipped | **REFUTED-as-stated — no work (favorable)** | The claim inverts the function AND the path is dead: `has_network_adapter(1)` returns 1 **only for adapter_type==2 (modem)** — broadband returns 0 (text_80145A94.cpp:169-189); and **no code ever sets `TLogo.state=2`** (writes are {0,1,3,4,5,6,7}, TLogo.cpp:125-189; state-3 has no case either). The modem-check branch is retail-dead → remove "force `has_network_adapter(1)`=present" from §4 step 19. Adapter still matters for phase-10 net screens (other callers). |
| A5 | D9: time-set survives missing memcard with a default clock | **REFUTED — title-scope work item** | `TAdTimeSetGC::apply_settings` (TAdTimeSetGC.cpp:885-899): `SystemSaveData *state = TAdSinglePlyChrSelectGC::get_global_state(); if (state == NULL) { change_state(1); return; }` — there is **no default-date fallback**; NULL routes the agree→time-set→title chain into state 1, never `open_title_scene`. → **W3**: minimal default-`SystemSaveData` provider (stub-grade `init_memcard` synthesizing a sys-save, or a static blob) added to step 22. First-logo-frame scope: unaffected (row 56 stays CONFIRMED). |
| A6 | Rows 49/50: no particle atlas on logo chain | **RISK** | No textual refs in the three logo-scene files (grep across TAdSEGAlogo/TAdSoniclogo/TAdADXlogo = clean), but see row 49/50: quad-sprite prototypes created at `init_logo_scene` carry `shared_effect_texture_list` + atlas-rect definitions, and the derived `render()` overrides are non-empty. Decides itself on the first real logo frame; row 50's reclass (W6) pre-resolves half. |
| A7 | Row 64: flag2 request has no producer when integrity is stubbed | **REFUTED — dependency error; skip still valid** | "flag2" = `lobby_request_flags & 2`. Its producer is **row 38** `load_now_loading_texture` (TLobby_803382E4.cpp:141), *not* the integrity gate: row 64 `handle_flag2_request` (:123-128) just releases the NOW-LOADING texlist. Ledger row 64 "Depends on 63" → **fix to 38**. Skip-safe as long as 38 stays deferred (bit never set → no-op ✓). **Pairing rule (W5): implement 38 ⇒ implement 64** (else the bit latches: gated draw at :96 + leaked texlist). Integrity gate itself: only caller discards the return (SystemBootInit.cpp:336); failure branch only sets `game_files_modified_flag` (text_802A1020.cpp:30/65/88), consumed solely by in-game player-record marking (TNpcPlayer_802C3CD0.cpp:142, TObjPlayer_802C422C.cpp:77) — plus a **fail-open** branch (load returns NULL → flag=0, :70-74). Checklist "set flag=0" note is redundant (bss default). |
| A8 | Checklist step 8: `get_video_standard()` constant 0 | **CONFIRMED** | Body is literally `int get_video_standard() { return 0; }` at **SystemBootInit.cpp:119-121** (boot-spine's gjSystem.c:120 cite is a file-location slip — it lives in the same TU as `main`). All consumers (TTitleScene.cpp:348, endings, MusicControl, battle rate) are title+ scope. Safe. |

## 3. §4 checklist cross-check — DEFER-skipped actions dereferenced by logo steps

- **W4 (real gap, step 19):** `logo_load_callback_table` (TLogo.cpp:39-49) includes
  `TLobby::update_keyboard_menu` → `TLanguage::free..` counterpart + **`TLanguage::load_text_resources(current_language)` + `TLanguage::load_fonts(current_language)`** (TLobby_80132ED0.cpp:30-34) — these run *before the first logo frame*. The ledger parks "font" at step 21/row 37; the keyboard-menu text/font resources must move into the logo asset step. `current_language` = bss default 0 at this point (only `apply_settings` sets it from SystemSaveData, A5) — no memcard dependency, just missing from the bill of materials.
- Step 12/15 per-frame consumers of DEFER rows 23 (rumble) and 34 (is_ready/wait_ready_loop): verified safe, W2/W-row-23 conditions above.
- Exit-path safety: step 20's `exit_logo_scene` reverse-callbacks `free_global`/`free_singleton_805D65C0` are NULL-guarded (TAdBackGC.cpp:84-89, TAdBackTxtGC.cpp:80-85) — surviving row 56 deferral ✓. `shutdown_and_reset_system` (D11 STUB) free-chain on never-allocated DEFER resources never runs while D11 is stubbed; when D11 gets a real host-teardown hook, every `free_*` in it must tolerate NULL (spot-checked: info_icon/banner/blob-shadow/chrselect all guard).
- No logo step (1–20) dereferences any other DEFER row's allocation. Rows 49/50 remain the open visual question (A6).

## 4. Count deltas to P's ledger (p1-init-ledger.md §3/§4/§5)

1. **Row count / verdict tally unchanged**: 75 rows, DEFER=18 — all 18 remain DEFER-viable
   for the first logo frame (16 CONFIRMED, 2 RISK). No DEFER row is REFUTED for the logo goal.
2. **Assumptions**: of A1–A8 → **3 CONFIRMED (A1, A2, A8), 4 REFUTED (A3, A4, A5, A7), 1 RISK (A6)**.
3. **Blocking tally**: stays 23 init + 6 dispatch — but row 28's qualifier upgrades `yes*`→`yes`
   (A1 evidence), and rows 49/50's `yes*-risk` label gets its first *negative* static evidence
   (clean logo-file grep) while §4 step 19 **gains** an implicit blocker (W4: TLanguage fonts;
   0 net new rows — pure-logic loads inside existing D3).
4. **Ledger corrections to carry forward**: row 64 dependency `63 → 38` (A7); D5/step-19 drops
   the `has_network_adapter` force (A4, dead branch); step-19 `logo_clear_flag=0` re-expressed as
   `VIGetDTVStatus→0` glue contract (A3); step-15/row-34 note gains the is_ready/wait_ready_loop
   hazard + W2 stub requirement (A2); boot-spine U3 resolved (flag ships 1); row 50 re-class
   KEEP-AS-IS recommended (W6); boot-spine §1 S10 row-38/release row-64 pairing rule stated (W5).
5. **Work-item queue (new, this audit)**: W1 VI-glue DTV contract · W2 `is_ready` link-stub or
   DVD-status contract · W3 default SystemSaveData (title lane) · W4 TLanguage fonts onto logo
   step 19 · W5 38⇄64 pairing rule + dep fix · W6 row-50 reclass. Only W1/W2/W4 touch the
   logo-frame critical path; W3 is title; W5/W6 are bookkeeping.

**Biggest refutation: A3** — `logo_clear_flag` ships **1**, not 0. The logo-min boot only reaches
the SEGA logo if the host VI glue reports non-DTV (`VIGetDTVStatus()==0`); otherwise frame one is
the progressive-mode dialog, which is unrenderable before font+input land (steps 21–22).
