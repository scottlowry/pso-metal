# P2-FONTQ — era font chain survey, supply reality, FONT build-card legs (t_0f0b6e0c)

Zero-build lane. Pins verified same-turn: DECOMP_ROOT `pso-decomp-port-pin` @6b38b538372ce6d9bba71d318709940398ae89d3 (READ-ONLY); pso-metal main @11d3054 (BRINGUP5 uncommitted state seen only via `git status`, never read); host census via `git grep HEAD` (committed state only). Disc gate: ISO sha1 prefix `0ff15531` measured this turn. No asset bytes committed anywhere; no disc path printed; no builds run; waiver untouched.

## 1. Era font chain end-to-end (all file:line read this turn at the pin)

**Card citation correction (R1):** the FONTQ card says "era TFontSmallTask.cpp:528-585" for `load_menu_font`. At the pin, `load_menu_font` lives in **`src/pso/ui/font/TNECFontEORFunction.cpp:528`** (span :528–585). `TFontSmallTask.cpp` is 276 lines and hosts the in-world float-text task + the lobby walkers (whose font-relevant slice bringup already consumes — see §3). `docs/p2-rung7-seams-1.md` T7-S4 already cites the correct home.

### 1.1 Producer / boot context
- `init_game_systems` `src/pso/ui/keyboard/SystemBootInit.cpp:249` calls `load_menu_font()` at **:310**, between `init_download_state()` and `TLobby::load_now_loading_texture()` — boot spine, before any menu frame (boot-spine doc puts font second after logo, consistent).

### 1.2 `load_menu_font` body (TNECFontEORFunction.cpp:528–585)
1. `TLobby::load_font_texture()` :540 — decompresses an **embedded PRS blob** (`lobby_font_texture_prs_data`, no disc dependency) into the shared `tmp_buf_use_req("FONT")` = `lobby_download_buffer` (`src/pso/unk/TLobby_800F8AA8.cpp:952`), then `load_gvm_texture_archive` into texlist `lbl_8049C61C`.
2. `menu_font_glyph_data = load_disc_file("S18all04.fon")` :541 — the glyph atlas (**the disc-supplied asset**, §2).
3. `TLobby::release_menu_font()` :542 — releases the step-1 GVM immediately (`…800F8AA8.cpp:945`); no pixel use of that texlist inside the cited span (R2).
4. **Two glyph pages**: `menu_font_render_rects[i].set_render_rect(NULL, 0x208, 0x100, 0x100)` + `set_texture_name(menu_font_texnames+i, rects+i, i+0xDCF28, 0x08800000)` (NJS_TEXNAME pair under `menu_font_texture_list` — Ginja/NJS texture plumbing, no NJS_MTD/material-transparency usage anywhere in `src/pso/ui/font` or `include/pso/ui/font` — `mtrans|NJS_MTD` grep: zero).
5. `load_texture_list(menu_font_texture_list)`; page pixel buffers taken from the surfaces (`…surface->header.pixels`), each **cleared 0x2000 words = 32768 B = a 256×256 CI4 page**; `mem_alloc(0xB40)` glyph table = **288 × MenuGlyphEntry(0xA)** (header layout assert in `include/pso/ui/font/TNECFontEORFunction.h`), sentinel-filled 2 pages × 12×12 grid, cell step 20px, `code=0x2EF width=9 epoch=0xF0`; palette `mem_alloc(0x100)` cleared 0x40 words, entries **[0]=0xFFF [1]=0x3FFF [2]=0x5FFF [3]=0x7FFF**; `set_context_texture_palette_bank(i, 0)` ×2; `menu_font_upload_epoch=0`, `menu_font_upload_flags=3`; spawns `TNECFontEORFunction` task object (end-of-render hook).
6. Per-frame `TNECFontEORFunction::run_task` :54 — when flags&2: `DCFlushRange(menu_font_palette,0x80)` + `init_and_load_tlut(0, 2, 0x80, menu_font_palette)` (bank/format/entry-count all era-pinned); when flags&1 (a glyph was uploaded): `epoch++` (eviction clock).

### 1.3 Glyph decode (CI4, no TLUT lookup at decode time)
- `upload_font_glyph` :308: entry sentinel `code=c`; if `c>=0xE000 → c-=0xDF20`; `width=get_font_char_advance(c)`; **source row stride**: `src = menu_font_glyph_data + (c-0x20)*0x5A` — **90 B/glyph = 18 rows × 5 B = 20px × 18px at 4bpp**; inner loop writes 5 bytes/row via `pack_status_flags` (:293, the CI4 texel nibble interleave `((b>>4)&3)|((b>>2)&0x30)` / `(b&3)|(b<<2)&0x30`) at `calc_ci4_tile_offset(x+i*4, y+j)` (:300 — GX CI4 4×4 texel-matrix addressing); `DCStoreRange(tex, 0x8000)`; `flags |= 1` (forces next-frame TLUT reload only via :57 branch — the store marks upload, epoch bump happens same tick).
- Colors come from the **bank-0 TLUT ramp** (the 4 palette entries) — glyph texture is pure 4bpp indices.
- Advance: `get_font_char_advance` :236 — `c<0x21 → 9`, `c>0xDF → 0x12`, else `menu_font_char_advance_table[c-0x21]` (:36, 191 rows, era `.data`).
- Code mapping: `map_char_to_font_code` :259 — Shift-JIS ranges (0x8140/0x824F/…/0xF040 branches + `+0xB1B` tail); kana route lands ≥0xE000 → the −0xDF20 fold above (R7 bounds).
- Cache: `find_cached_glyph` :353 — 128×2-way hash (`code & 0x7F`, way1 at +0x80) + 31-entry linear pool `[0x100..0x11E]` + permanent fallback `[0x11F]`; `upload_glyph_if_needed` :334 — epoch==cur hit / epoch==cur−1 pending-evict / else re-upload.

### 1.4 Menu text render path
- `draw_menu_font_string` :419: per-char SJIS mb join; `parse_text_control_codes` :71 (`\tJ`/`\tE` mb, `\tC0..C9` → `text_control_color_palette` :31, `\tCG` → `TWindowCtrl::get_pulse_color`); `\n` new-line +20px (or +9 inline when flags&1); space +9; per glyph: `map_char_to_font_code` → `bind_menu_font_glyph` :401 (`find_cached_glyph`, stamp `epoch`, `set_draw_texture_and_color(entry->page, color)`, `u0=x/256, v0=y/256`) → `set_glyph_quad_extents` :99 (`x1=x0+adv`, `y1=18+y0`, `u1=u0+adv/256`, `v1=v0+0.0703125`=18/256) → `draw_textured_quad_at_depth(&quad, scale)`. Brackets: `reset_gauge` (=`begin_screen_draw(1)` + `set_texture_list(menu_font_texture_list)` + `set_draw_texture_and_color(0,-1)`) … `end_render_forward` (=`end_screen_draw`). Wrappers `draw_menu_text` (scale 0.5) / `draw_menu_text_scaled` / `draw_menu_font_string_white` :506; widths `TWindowSubStatusDisp::calc_text_width` :112 + `calc_fullwidth_text_width`.
- Consumers (census): keyboards `TWindowKeyBoardBase.cpp:781/:812`, `TWindowKeyBoardSoft.cpp:580`; `TWindowLayoutNumber.cpp:96`; window text `TWindowSubStatusDisp_8024FBA8.cpp:166` (+ small-font siblings :33/:86, §1.5); frontend `TAdDialingGC.cpp:152/:156/:258`, `TAdDialingMessageWindow.cpp:35`, `TAdEnterBaseGC.cpp:361`, `TAdTimeSetGC.cpp:1059–1078`, `TAdGuideButtonGC.cpp:26`, agreement/width-only `TAdAgreementBaseGC.cpp:43`, `TAdEnterSerialAccessGC.cpp:118`, `TAdEp3Exist.cpp:43`, `TAdSubPlyItemGC.cpp:38/:53`, `TAdTitleGC.cpp:108`; world `TObjTombstone.cpp:521/:525`, `TDLGraph.cpp:242–251`, `TObjAreaWarpEndingJung.cpp:1099–1109`, `TObjOnlineEndingMain.cpp:311`. The rung-7 scroll surface rides this via `WindowTextEntry::render` `src/pso/ui/window/WindowTextEntry.cpp:172–200`.

### 1.5 TFontSmallTask family roles (font-relevant only)
- `TFontSmallTask::render` `src/pso/ui/font/TFontSmallTask.cpp:46` — in-world floating text (damage/boss/labels). It binds **`render_forward`** (`TFontSmallHelp.cpp:30` → `set_texture_list(localized_alphabet_list)`), quads via `build_small_font_glyph_quad` / `get_small_font_glyph_width` (`TWindowSubStatusDisp_800FBD74.cpp:126`) — the **TLanguage alphabet atlas, not S18all04**. Same for `TFontSmallHelp` and the `draw_window_text`/`draw_small_font_string_clipped` scroll chain.
- Same TU hosts `TLobby::call_funcs_reverse` :251 and `run_load_callback_table` :263 — already promoted VERBATIM into bringup (HG3-273/BRINGUP5); font lane does not own them.
- **Second font supply (the roster's "TLanguage"):** `TLanguage::load_fonts` `src/pso/system/TLanguage.cpp:65` — per-language `load_palette_tlut` of `f256_alphabet.gvp` (:144) + symbolsetumei/key/wordmojitex/menutext01 `.gvp` into TLUT banks 0x48/0x49/2/3/0x4B/0x4C (`TLanguageFontPalette.cpp:29–38`, each followed by `GXDrawDone`), then `localized_alphabet_list = LoadPvmFileTls(Texture<Lang>.gvm)` (:76–108; names :163 `TextureEnglish.gvm` etc.). Load half `TLobby::update_keyboard_menu`, release half `TLobby::free_keyboard_soft_resources` (`TLobby_80132ED0.cpp:25–28`) = exactly the counted (d) pair bringup walks; `set_language` :170 re-runs both on change.

## 2. Asset supply reality (path class only)
- `.fon` inventory: **zero `*.fon` files on this machine** (full-Hermes `find -iname '*.fon'` = 0). The local extracted-disc layout other lanes use (gitignored, tree-sibling class under the decomp main checkout's `orig/GPOE8P/`) is a **curated 659-file subset (648 `.rel` + 1 `.dol` + 5 `.pr2` + 5 `.pr3`)** — font assets were never in scope (`tools/pso_assets.py` CATEGORIES has no `.fon`; `config/GPOE8P/data.json` 658 rows, zero font entries).
- The asset IS on disc: ISO sha1 prefix `0ff15531` (gate-matched); raw scan finds the FST name **`S18all04.fon`** (name-table hit near image end + a DOL-region filename-table hit `…R_N.adx / S18all04.fon / Scene…`). Era resolution order proves loose-file supply: `load_disc_file` → `TGStream::load_file` (`TLobby_800F8AA8.cpp:1382`, `TGStream.cpp:120`) → constructor `TGStream.cpp:348–369`: GSL-archive entry first, else `get_disc_file_size`/`ADXF_Open(name)` direct disc FS. At boot (§1.1) no GSL is open ⇒ **S18all04.fon is a direct disc-FS file, not inside any GSL/GVM archive**. Buffer = sector-rounded alloc: `ceil(size/2048)*2048`, min plausible content 192 glyphs × 0x5A = **0x4380 (17280 B)** for codes 0x20–0xDF; the 0xE000-folding kana range (R7) can raise the true floor — exact dims pin at F0.
- **Extraction step the build lane needs (verbatim ask):** `dtk vfs cp --no-decompress <pinned game.iso>:S18all04.fon <dest>` using the decomp-tree dtk binary (`build/tools/dtk`, same executable `tools/pso_assets.py:413–433 extract_missing` uses); dest class = the gitignored local extraction layout — pso-metal already hosts disc-derived art in `host/bringup/local_assets/` (pattern: `hyouji_a.gvp`, `logo_e.gvm`, `logo_e.slices.json`, `sinbd.gvr`), which `.gitignore` keeps out of git. Record **fnv1a64 + byte size** in the card receipt; commit bytes NEVER. F7 adds the second supply set: `f256_alphabet.gvp` + the language's `Texture<Lang>.gvm` (+ symbolsetumei/keyeng/wordmojitex/menutext01 `.gvp` rows) through the same dtk step; the FONT GVM of §1.2-step1 needs **no** extraction (embedded PRS blob).

## 3. Host census (committed HEAD @11d3054; same-turn `git grep -i font|glyph|S18all04|alphabet HEAD -- host/`)
- **Font implementation: zero** (as expected). `load_menu_font`/`menu_font_*`/CI4 decode absent.
- Counter/stub rows the FONT lane flips:

| host row | state | flips when |
|---|---|---|
| `hg3_walk_seam.cpp:102` `p8_lobby_free_keyboard_soft` (d) | counted release-half | F7 (TLanguage free pair real) |
| `hg3_trial_seam.cpp:79` `rl:kbd.free_soft` (d) | counted | F7 |
| `gen_ladder8_walk.py:251` `(d) font/lobby lane — RELEASE half (fires)` | generator marker | F7 |
| `ladder9_trial_prelude.h:74` `TAdScrollWinGC` "(c)/(d) font lane — NOT built (card S13)" | shim struct | F6 (glyph surface) |
| `ladder9_trial_prelude.h:116` `TLanguage::set_language` COUNTED (font/language lane) | stub | F7 |
| `host/link/link.mk:15` `TLanguageFontPalette` (+`init_and_load_tlut`, `load_gvpl_palette_file`) projected out | link census | F4/F7 (real symbols) |
| `gx/dl/gx_logo_driver.cpp:144` cites `TLanguageFontPalette.cpp:35–37` as era caller pattern | host note | consistency check, no flip |

- Font-ADJACENT, not flipped by font: `ladder7_seam.cpp:179` `render_debug_text` (DebugFont family — seam doc names it as the ONLY existing text stub); `bringup.mk:482–484/:532` consume era `TFontSmallTask.cpp` as generator source (TFST_SRC) for the walkers — do NOT break that mtime/dep edge when the font lane touches DECOMP_ROOT reads.
- Supply-test precedent exists: `host/bringup/fake_disc/gsl_tab.dat`/`gsl_tab2.dat` carry a `menu_font.prs` GSL entry used by `ladder3_test.cpp:290` (`find_gsl_entry` grammar) — reuse this fixture pattern for the F4 resolver leg (note: `menu_font.prs` is a host fiction; era never loads that name).

## 4. FONT build card — smallest testable legs
- **F0 Supply gate:** dtk extract (or local_assets drop) of `S18all04.fon` + receipt {fnv1a64, size}; assert `size % 0x5A == 0` and `size ≥ 0x4380`, and record `glyph_count = size/0x5A` (settles R7). Failure mode loud, no fallback.
- **F1 Statics:** verbatim `menu_font_char_advance_table[191]`, `text_control_color_palette[10]`, sentinel fills, page geometry constants (0x208/0x100/0xDCF28+i/0x08800000, 0xB40, palette 4 × s16). Byte-golden vs era source at gate time (generator pattern, cf. HG3-272 two-source validation).
- **F2 Decode:** `calc_ci4_tile_offset` + `pack_status_flags` + `upload_font_glyph` against the extracted asset (FST cache semantics = era `(c-0x20)*0x5A`, −0xDF20 fold). **Golden: decode a known glyph rect at gate time from the era asset, runtime-only, fnv-pinned** — anchor set: 0x20 (space-row floor), 0x41 'A', 0xDF (last table code), 0xE000→0xE0 (fold edge); fnvs recorded by the extracting lane against the same asset that yields the F0 file fnv (single-source, re-computable every gate run).
- **F3 Mapping:** `map_char_to_font_code` range branches + `get_font_char_advance` edges — pure-function table golden (pinned (input,code,advance) triples).
- **F4 Load/bind:** `load_menu_font` span on host substrate: two CI4 pages + NJS_TEXNAME equivalents in the host texture layer, TLUT bank-0 load (`init_and_load_tlut(0,2,0x80,…)` twin), `set_context_texture_palette_bank(i,0)`×2, EOR task spawn, `free_menu_font` teardown order (delete object → release list → free table/palette/data); disc resolver leg = GSL-first/else-loose semantics against the fake_disc fixture.
- **F5 Cache/eviction:** hash+pool+fallback walk, epoch clock (upload ⇒ flags, run_task ⇒ reload + epoch++), pending-slot law.
- **F6 Render:** `draw_menu_font_string` + wrappers + `set_glyph_quad_extents` law (18px cell ⇒ `v1=v0+0.0703125`); gate = quad-stream fnv for a pinned ASCII/SJIS string incl. `\tC` color + `\n`; unlocks the ladder9 scroll shell (card S13) + HG3-284 T7-S4 ride.
- **F7 Language set:** `TLanguage::load_fonts`/`set_language` + free pair with per-language `.gvp/.gvm` supply (second extraction, §2) — flips every (d) host row in §3.
- **F8 Consumers:** keyboards/LayoutNumber/8024FBA8/agreement text — screen-level, after F6/F7.

## 5. Canaries (must be armed before any F2/F4 green counts)
- **Planted wrong-bank canary:** variant loads the menu-font TLUT ramp into host TLUT bank ≠ 0 (era: bank 0 ×2 pages, `init_and_load_tlut(0,…)` :57 + `set_context_texture_palette_bank(i,0)` span :552–561) or flips palette entries; gate asserts bank-0 contents == era ramp and neighbor banks untouched ⇒ variant MUST fail the gate (teeth test, per fleet gate law).
- **Planted wrong-tile canary:** permute `calc_ci4_tile_offset` interleave terms ⇒ F2 rect fnv must flip; **planted wrong-stride** canary: `*0x64` instead of `*0x5A` ⇒ same.

## 6. Risks
- **R1** card-vs-pin citation drift (§1) — briefs re-anchor to TNECFontEORFunction.cpp.
- **R2** FONT GVM load→release (:540→:542) has no cited pixel use inside the span — either era warm-up/heap-shaping or a missing consumer; build lane decides stub-vs-real, disclose either way.
- **R3** sector-rounded alloc ≠ content size — pin F0 on FST-reported dims (exact file length), not the 2048-multiple buffer.
- **R4** era read pumps `TLobby::run_frame()` while waiting (`read_sectors_sync` busy loop) — host equivalent must ride the existing pump seam, not mint a second frame loop (rung-7 R-G collision).
- **R5** cache coherence semantics (DCFlushRange/DCStoreRange/GXDrawDone in TLanguage paths) — host no-ops must be chosen so upload ordering vs epoch bump keeps eviction behavior identical.
- **R6** three distinct supplies (disc `.fon` / embedded PRS GVM / per-language `.gvp`+`.gvm`) — never collapse into one "font asset" assumption in gates.
- **R7** glyph-count floor: kana fold needs codes >0xDF; if F0 measures size < (0x11F·0x5A) the high codes clamp — F0 receipt settles it before F2 pins its anchors.
