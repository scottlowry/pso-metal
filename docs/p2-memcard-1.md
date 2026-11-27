# P2-MEMCARD-1 — SystemSaveData supply evidence for the title-gate decision

Lane MEMSCAN (card t_5e484999), zero-build seat. DECOMP_ROOT = pso-decomp-port-pin
@6b38b53837, pso-metal tip 573ba98, all cites read same-turn at that pin.
This doc PRICES the three supply options; the owner decides (per docs/p2-rung6-seams-1.md
R-A, `p2-rung6-seams-1.md:137-141`). No repo build was run; no asset bytes committed.

## 1. What SystemSaveData IS (L1)

- **Type**: `struct SystemSaveData`, declared in
  `include/pso/ui/frontend/TAdSinglePlyChrSelectGC.h:35-47`; layout asserted
  `sizeof == 0x11C` (`:49`). Comment `:34`: "The 0x11C bytes of the system card
  file (the part after the card header)."
  Fields (offsets from decl order): `checksum:u32@0x00`, `music_stream_gain:s16@0x04`,
  `cue_stream_gain:s8@0x06`, `language:s8@0x07`, `clock_offset_frames:s32@0x08`,
  `udp_option:s16@0x0C`, `field_e:s16@0x0E`, `data_10[0x100]@0x10`,
  `serial_number[8]@0x110`, `serial_seed:s32@0x118`.
- **The live object is a heap pointer, not a static block**: global
  `SystemSaveData *system_save_payload;` defined at
  `src/pso/ui/frontend/TAdSinglePlyChrSelectGC_801FBF54.cpp:63` (defining unit =
  `TAdSinglePlyChrSelectGC_801FBF54`; sibling `secondary_card_system_save_payload` `:64`,
  the slot-1 card copy). Accessor `TAdSinglePlyChrSelectGC::get_global_state()` returns
  `(u8*)system_save_payload` verbatim — `src/pso/ui/frontend/TAdSinglePlyChrSelectGC_8001ED80.cpp:5-7`;
  symbols.txt `config/GPOE8P/psov3/symbols.txt:918`
  `get_global_state__23TAdSinglePlyChrSelectGCFv = .text:0x8001ED80 size:0x8` (a 2-insn
  pointer return, corroborating the accessor shape). Note the name-collision globals:
  `TAdChrSelBSlotGC::get_global_state` / `TAdMultiPlyChrSelectGC::get_global_state`
  (symbols.txt `:22048`, `:20883`) are DIFFERENT state-int accessors — not this payload.
- **NULL semantics = never-initialized, NOT cleared-on-absent-card.** The pointer is
  assigned only by `new SystemSaveData` inside `init_char_select_globals`
  (`_801FBF54.cpp:2836-3093`, every switch arm allocates it, e.g. `:2859`) and is NEVER
  set back to NULL anywhere (`grep -rn 'system_save_payload = NULL|delete system_save_payload'`
  over src hits only the *secondary* pointer at `:3135` in `free_callbacks`). So
  NULL ⇔ "boot init hasn't run yet" (BSS-zero). `init_char_select_globals` is called
  from `init_game_systems` (`src/pso/ui/keyboard/SystemBootInit.cpp:313`;
  `init_game_systems` header `:249`) which `main()` calls before any phase runs
  (`SystemBootInit.cpp:365,369`). **In a fully-era-booted game the apply_settings NULL
  branch is therefore dead code** — the NULL branch is load-bearing only for a host
  bring-up that does not run the era boot init (which is exactly pso-metal's rung regime).
- **Consumed fields in `apply_settings`** (`src/pso/ui/frontend/TAdTimeSetGC.cpp:885-899`):
  `state->language` `:892` (→ `TLanguage::set_language`), `state->music_stream_gain`
  `:893`, `state->cue_stream_gain` `:894`; NULL → `change_state(1)` early-return `:888-891`;
  success tail → `TLobby::init_sound_mode()` `:895`, unload/load lobby scene resources
  `:896-897`, `TAdMultiChgGameModeGC::open_title_scene()` `:898`. `change_state(1)` lands
  on state-table row `{enter_state_1, ensure_system_file_loaded}` (`TAdTimeSetGC.cpp:109`);
  `ensure_system_file_loaded` `:283-288` calls `load_system_file(0)` then `change_state(2)`
  — i.e. the NULL branch is a *retry-load* path, not a default-install path.
  `apply_settings` is fired from `update_state` (`:690-700`, case 0 `:696`) reached when
  `timer<=0` (`run_task` `:162-174`).
- **First-boot defaults exist in-era**: `TAdSinglePlyChrSelectGC::reset_save_header()`
  (`_801FBF54.cpp:2540-2554`): memset 0 over 0x11C, `music_stream_gain=-0x32`,
  `cue_stream_gain=0`, `language=TLanguage::get_default_language()` (= LANGUAGE_ENGLISH,
  `src/pso/system/TLanguage.cpp:192-194`), `clock_offset_frames=default_clock_offset_frames()`
  (= 0x1A5E00, `src/pso/unk/TLobby_8001D6A0.cpp:469-471`), `udp_option=0`, `field_e=0`,
  `serial_number` from `TAdPlayerLoadGC::reset_load_state` → `CARDGetSerialNo`
  (`_801FBF54.cpp:2550-2551`; `src/pso/ui/frontend/TAdPlayerLoadGC_801E0C30.cpp:34-36`),
  `serial_seed=DateToSeconds(now)` `:2552-2553`. Callers: `init_char_select_globals`
  `:3098` (post-memset boot fill), `verify_save_checksum` on CRC mismatch
  `:2005` (damaged-card repair-to-defaults), `TAdTimeSetGC::init_screen`
  `TAdTimeSetGC.cpp:453` (time-set entry resets, then writes to card via
  `start_system_file_write` `:458`).
- **Era producer chain (who writes the payload):** (1) boot default above; (2) card read:
  `verify_save_checksum(int)` memcpys the file body out of `system_save_file_buffer`
  (`_801FBF54.cpp:75`, global raw read buffer) at `+sizeof(CardSaveHeader)` into the
  payload and CRC-checks `calc_crc32_bitwise` over 0x11C `:1999-2003`
  (`CardSaveHeader` = title[0x20]+comment[0x20]+banner[0x1800]+icon[0x800]+size+crc =
  0x2048, `include/pso/ui/frontend/TAdPlayerLoadGC_801E0C30.h:11-18` — asset bytes named,
  never committed here); (3) option edits: `TWindowOptionCtrl.cpp:570-571` (gains),
  `:580` (udp_option write); (4) card write-back: `start_system_file_write`
  (`_801FBF54.cpp:942-947`, rebuilds PSO_SYSTEM from `system_save_file_buffer`) and
  `create_instance` (`:2036-2040`, recompute checksum + `build_default_save_file`).
  Card file name is `"PSO_SYSTEM"` (`_801FBF54.cpp:85`), 3 blocks (`system_save_file_block_count=3`,
  `:3102`; buffer alloc 0x6000 at `:164` in the `load_system_file(chan,out)` variant,
  `:147-176`, which CRCs `buf+0x2048`); a second dir name `"PSO3_SYSTEM"` exists at
  `:86` used by `TAdSinglePlyChrSelectGC::load_system_file(s32,s32*)` `:157`.
- **Other consumers of the same pointer (multi-view global):** `TWindowSoundBalance.cpp:188`
  (as `SoundBalanceSettings*`), `TWindowOptionCtrl.cpp:566-580`, `TLobby_8001D6A0.cpp:484-493`
  (`get_clock_frames` — NULL-tolerant, falls back to `default_clock_offset_frames()`),
  `TLobby_80132ED0.cpp:12-14` (language, default-fallback), `TLanguage.cpp:180`,
  `SoundRequests.cpp:138,150`, `GbaDownloadRuntime.cpp:159`, and `QuestOpcodeHandlers.cpp:1876,
  2011,2024,2266,2300,2339,3387,3396` (as `QuestEventFlagRecord*`). One buffer, several casts.

## 2. Trial-screen independence (L2) — proven per gate, not inherited

Every card read on the 0x17 agreement path (mode set by
`open_trial_agreement_scene`, `_8023E7A8.cpp:214-218`, `multi_chg_game_mode_state=0x17`;
scene instance `new TAdTrialAgreementGC` at `TAdMultiChgGameModeGC_8023E7A8.cpp:398` inside
`create_mode_scene` case 23). `check_card_file` is defined `src/pso/ui/frontend/TAdTrialAgreementGC.cpp:55-74`
(brief's ":53-74" corrected by same-turn read) and its result has **exactly one consumer
in the whole tree**: the ctor test at `:87` (`grep -rn check_card_file src/` → `:55`, `:87`).
Read sites and no-card outcomes, all read-only:

| site | call | no-card outcome |
|---|---|---|
| `:58` | `TAdPlayerLoadGC::probe_card_slot(0)` (→ `CARDProbeEx`/`CARDGetResultCode`, `_801E0C30.cpp:589-625`) | returns 0 → `check_card_file`=0 |
| `:61` | `probe_and_check_card(0)` (`_801E0C30.cpp:82-…`) | unreachable w/o card |
| `:64,:67` | `wait_for_load()` loop `:42-47` (`probe_card_slot`+`poll_memcard_result`) | breaks immediately (`probe==0`) |
| `:65` | `check_memcard_status(0)` (`_801E0C30.cpp:625-…`) | unreachable w/o card |
| `:69` | `find_card_file(0, pso_system_card_filename)` (`_801E0C30.cpp:652-…`) | unreachable w/o card |

ctor `:84-93`: `check_card_file()!=0` (a *valid existing PSO_SYSTEM* — find==0 at `:70`
means found) → auto-`on_agree()` `:88` + self-destruct; else the scroll window is shown
`:92`. `on_agree` `:40` = `open_time_set_scene()` (mode-8 store, `_8023E7A8.cpp:283-287`);
`on_decline` `:39` = `request_main_phase(0)`. **No site on this path writes any card
state, and `TAdTrialAgreementGC.cpp` contains zero references to `system_save_payload`/
`get_global_state`/`SystemSaveData` (grep same-turn, empty).** So the trial screen is
reachable with no card, no payload, no shim: PROVEN. The gates that *do* block title are
downstream: (G1) `TAdTimeSetGC::run_task:162-165` — `poll_load_status()` `:915-…`
returns bit-1 (no card) ⇒ forced `change_state(0)` every frame + no-card message
(`report_card_status` `:186-…`); (G2) `apply_settings` NULL branch `:888-891` (host-only,
see §1); (G3) `TTitleScene::run_task:289-300` — `probe_card_slot(0)==0` ⇒ destroy
transition children + `open_time_set_scene()` bounce every frame.

## 3. Supply options (L3) — priced, owner decides

| opt | what | era cites | host integration points (grep-verified) | seam-doc legs changed | cost notes |
|---|---|---|---|---|---|
| **O1 era-true rest-on-time-set** | zero new supply; bring-up stops where era stops: title unreachable while G1 holds (era: with a card, `init_screen` `TAdTimeSetGC.cpp:452-458` resets + `start_system_file_write` `_801FBF54.cpp:942-947` provisions PSO_SYSTEM first-boot) | `TAdTimeSetGC.cpp:162-165,915-945`; `TTitleScene.cpp:289-300`; `_801FBF54.cpp:942-947,2540-2554` | none new; `host/bringup/ladder7_seam.cpp` keeps fail-loud stance (`:238` pattern) | HG3-281 stays "(b) NULL-branch leg only"; HG3-282 remains stretch/never-landed | cheapest, zero risk; leaves rung-6 terminal landmark = trial screen (seams doc §build order, `:112`) |
| **O2 host default SystemSaveData block** | bringup TU defines the payload + sets `system_save_payload`: 0x11C block copied from `reset_save_header` defaults (§1: gains −0x32/0, LANGUAGE_ENGLISH, clock 0x1A5E00, udp 0; serial/seed host-chosen), checksum per `create_instance` law `calc_crc32_bitwise` over 0x11C with field zeroed (`_801FBF54.cpp:2036-2038`) | `_801FBF54.cpp:63,2540-2554,2836-3098,2036-2040`; `TLobby_8001D6A0.cpp:469-471`; `TLanguage.cpp:192-194` | mirror-able via `ladder7_seam.cpp` global-supply pattern (`:36` `memcard_access_active` precedent); era TUs compile against the declared extern (`include/pso/ui/frontend/TAdSinglePlyChrSelectGC.h:255`) | G2 dies: apply_settings reaches `:898 open_title_scene`; **G1/G3 remain** — time-set still frame-locked no-card and title still bounces, so O2 alone does NOT make title reachable/sticky | + one small TU + a layout assert copy; low risk; only pays off combined with O3 (or with accepting the bounce as evidence) |
| **O3 memcard-API shim** | host fakes the CARD surface the era probe chain consumes so `probe_card_slot(0)!=0` and `find_card_file(0,"PSO_SYSTEM")` answers. Minimum read-only lie = `CARDProbeEx` (rc 0, `*sectorSize=0x2000`, `CARDGetResultCode` 0), `CARDCheckAsync`/`CARDCheck` 0, `CARDGetStatus` enumerating one slot named `PSO_SYSTEM` (`iconAddr != 0xFFFFFFFF`), `CARDGetSerialNo`, `poll_memcard_result`⇒ `CARDGetResultCode` quiescent — surface enumerated at `_801E0C30.cpp:46,99-130,177-200,501-…,589-625,652-712`; full write path additionally needs `CARDOpen/CARDCreate/CARDReadAsync/CARDWriteAsync/CARDClose/CARDUnmount/CARDMountAsync/CARDSetStatus/CARDGetSectorSize/CARDFreeBlocks/CARDSetAttributes/CARDDelete` (call census in `_801E0C30.cpp`: 21 distinct CARD* symbols) | `_801E0C30.cpp` (all probe/read/write chains); `CardSaveHeader` 0x2048 incl. banner/icon (`_801E0C30.h:11-18` — a real card image needs banner/icon bytes: NOT committable in-repo, host-side generated) | declared surface already tagged in `host/psosh_card.h` (CARDProbeEx #6-sites featured; CARDGetStatus/CheckAsync/Open/… PSOSH_TAG "mmap-file card image + async worker"); shim pattern precedent `host/bringup/dvd_shim.c`; selftest references already exist (`host/selftest.c`) | G1+G3 open: time-set exits, title sticky; with O2 also G2 moot. HG3-281 (b)→(a)-style full-leg, HG3-282 becomes terminal landmark, HG3-277/280 unaffected | highest cost: async-state machine (`card_slot_state_flags` bits `4<<chan`/`0x10<<chan`/`0x400<<chan`, error words −0x7D/−0xFF/−0xFB/−0xBF) must stay coherent across ~20 call sites; risk of the shim becoming the thing under test (era-card truth unverified); O2 ⊂ O3 in effect |

Unverifiable at this pin, named as such: (i) TAdSystemFileCheckGC mode-branch internals
(mode-selection at `TAdSystemFileCheckGC.cpp:329-332,373` read only as lines, its
full check-card flow not traced); (ii) newserv-era network-side system-file paths —
no newserv consumer of `system_save_payload` found by the greps above, so "newserv-era
paths" as producer is UNCONFIRMED in this tree; (iii) exact `.s` bytes behind the
0x8001ED80 accessor (symbols.txt line is the only same-turn authority; no orig/ read).
No build was run anywhere; no disc path printed; no asset bytes committed.
