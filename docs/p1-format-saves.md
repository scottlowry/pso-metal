# P1-G — Memory-Card / Save-Data Format-Boundary Audit

Phase-1 task G. Maps what actually crosses the memory-card byte boundary in PSO EP&I,
sizes each saved record, and tests every Phase-0 assert-failing type (abi.md) plus the
BE-hazard structs (be-index-summary.md) for save-record containment. All file:line refs
are relative to the READ-ONLY decomp `/Users/scott/Hermes/pso-decomp/pso-decomp`
(master 6b38b5383) unless prefixed `pso-metal/`.

## 0. TL;DR

* The card surface is narrow: 5 src files call CARD* (127 sites, matches host-surface.md);
  the driver is `src/pso/system/save/McFsGc*.cpp` +
  `src/pso/ui/frontend/TAdPlayerLoadGC_801E0C30.cpp`.
* **Three game save records** cross the boundary, each prefixed by one shared
  `CardSaveHeader` (0x2048): **CharacterFile 0x1156C**, **GuildCardSaveData 0xE28C**,
  **SystemSaveData 0x11C** (files `PSO_CHARACTER`, guild-card file, `PSO_SYSTEM` —
  `src/pso/ui/frontend/TAdSinglePlyChrSelectGC_801FBF54.cpp:85,92`). Quest-download
  bins ride the same header (`src/pso/quest/TQuestVmDownLoadServer.cpp:802,871,948`)
  but their payload is an opaque encrypted blob — no host struct exposure.
* **Zero of the 46 failing LAYOUT_ASSERT sites (abi.md's 43) appear in any save record.**
  The transitive include closure of the four save roots is 38 headers; none of the
  assert-failing types occurs in it as a by-value member or base (scripted token scan,
  §3). Every failing type is a scene-graph class, UI/window class, or compiled-in PMF
  state table → **RUNTIME-WAIVE from the saves angle, 46/46**.
* The save-format danger is the *opposite* set: plain-POD structs whose size asserts all
  PASS on arm64_32 but whose multibyte fields are BE wire — `TItemData` mixed-width
  unions, `gc_tag`, and especially the **word-arithmetic cipher**
  (`SubtractiveRng::apply_rng_cipher`, `src/pso/system/SubtractiveRng.cpp`:
  `data[i] = next() - data[i]`) which is byte-order-dependent and silently produces
  non-GC-compatible images on LE. abi.md's live-BE-hazard list (NetTcpState, TEF_LensFlare)
  contains **no** save-relevant struct; neither save header appears in the closure.

## 1. CARD* boundary map (what I re-measured)

`grep -rn 'CARD[A-Za-z]*(' src` → **127 sites in 5 files** (confirms host-surface.md):
`src/pso/system/save/McFsGc.cpp`, `McFsGcRequests.cpp` (async driver loop:
CARDWriteAsync :323, CARDReadAsync :451), `src/pso/network/NetworkInfoLoader.cpp`,
`src/pso/ui/frontend/TAdPlayerLoadGC_801E0C30.cpp` (start_memcard_write :348 →
CARDWriteAsync :382; start_memcard_read :438 → CARDReadAsync :474; banner/icon load),
`TAdSinglePlyChrSelectGC_801FBF54.cpp` (the three-file open/read/write/format logic).

Runtime-only structs at this layer (NOT saved — they hold pointers/handles):
`CardSaveDescriptor` 0x60 (`include/pso/system/save/CardSaveFile.h:6-20`; `.banner/.icon/.data`
are host pointers → RUNTIME-WAIVE), `CardFileEntry` (`CardFileEntry.h:9`, holds SDK
`CARDFileInfo`+`CARDStat`), `CardSlotInfo` (`McFsGc.h:7`), and the ROM constant
metadata globals `CharacterCardMetadata`/`EpisodeCardMetadata`/`GuildCardFile_804C89BC`
(`CardSaveFile.h:23-45`, read back via `CardSaveMetadataBlock` reinterpret at
`TAdSinglePlyChrSelectGC_801FBF54.cpp:2224`) — filename/title/icon-lookup strings,
never written to card (banner/icon bytes go inside CardSaveHeader instead;
`start_memcard_write` pins `commentAddr=0, iconAddr=0x40`, :373-374).
The SDK card-volume format itself (FAT, dir entries, `CardWriteDate` in `CardFileInfo`
0x14 `include/pso/system/save/CardFileInfo.h:7-16`) is BE and belongs to the host
card-image shim's format contract, not the game structs.

## 2. Save-format inventory (records that ARE the card bytes)

Wire layout of every card file: `CardSaveHeader` + payload, then padded to 8KB blocks
(`card_blocks_for_size` = `(size>>13 + 3)<<13 / 8192`, `TAdPlayerLoadGC_801E0C30.cpp:328`;
`build_save_file_image` returns `size/8192 + 3` blocks :439).

| Record | Size | Declared | Build (write) | Consume (read) |
|---|---:|---|---|---|
| CardSaveHeader (title 0x20, comment 0x20, banner 0x1800, icon 0x800, s32 size, u32 crc) | 0x2048 | `include/pso/ui/frontend/TAdPlayerLoadGC_801E0C30.h:11-19` | `build_save_file_image` `src/pso/ui/frontend/TAdPlayerLoadGC_801E0C30.cpp:404-440` | `verify_card_save_header` (CRC + split) :332-344 |
| CharacterFile `PSO_CHARACTER` (u32 crc + 7×CharacterFileSlot 0x2798 + serial/access/password + music_flags u32[2] + save_count + cipher_seed) | 0x1156C | `include/pso/system/save/CharacterFile.h:27-46` | `save_character_file` f.size=0x1156C `src/pso/ui/frontend/TAdSinglePlyChrSelectGC_801FBF54.cpp:2258-2283`; `encrypt_save_data` :2192-2211 (seed→CRC32→word-XOR→RNG-cipher→block-permute) | `load_save_from_card` :406-441 (`start_memcard_read` :418 → `decrypt_char_data` :439); payload memcpy at :2110 |
| ├ TPlyCharData (TPlyInventory 30×TPlyInventoryItem + player_stats + disp data + config + u8 tech[20]) | 0x41C | `include/pso/player/TPlyCharData.h:14-27` (slot offsets via `CharacterFile.h:42-46`) | in-RAM copy of slot | `get_selected_character_slot` |
| ├ CharacterSaveRecord (quest flags 4×0x80, BankItemRecord[200], guild_card u8[0x90], SymbolChatPreset[12], ChatShortcut[20], BattleRecord×2, ChallengeRecord 0x100, ChoiceSearchSelection[5], s32 quest_counters[16], texts) | 0x2370 | `include/pso/system/CharacterSaveRecordLookup.h:45-101` | same buffer | same buffer |
| GuildCardSaveData (u32 checksum + GuildCardList {pad 0xC0 + GuildCardSlot[0xD2]×0x100} + GuildCardEntry[0x1C]×0x90 + timestamp + cipher_seed) | 0xE28C | `include/pso/player/GuildCardSaveData.h:8-31` | `build_save_file` f.size=0xE28C `TAdSinglePlyChrSelectGC_801FBF54.cpp:2223-2239`; `encrypt_guild_card_payload` :2065ff (called :2031) | payload memcpy :1955 (+`decrypt_guild_card_payload` :1956) |
| SystemSaveData (u32 checksum, s16 music_gain, s8 cue/lang, s32 clock_offset, s16 udp_option, s16 field_e, u8 data_10[0x110], serial[8], s32 serial_seed) | 0x11C | `include/pso/ui/frontend/TAdSinglePlyChrSelectGC.h:35-49` | `build_default_save_file` f.size=0x11C :2241-2257 | memcpy :399, :488, :1999, :2017; `verify_system_file` :379 (`fields_equal`) — **no cipher, no CRC over payload fields beyond `checksum`** |
| Quest download bins (same header; payload = own cipher/compression, opaque) | var | `QuestBinHeader` `src/pso/quest/TQuestListVM.cpp:314-330` | `TQuestVmDownLoadServer.cpp:802,871,948` | `TQuestListVM.cpp:319` `verify_card_save_header` |

RAM copies stay plaintext (decrypt after write / before use, :2031-2033, :2279-2282);
the card always holds cipher-shuffled bytes. Duplicate primary/secondary buffers
(`secondary_card_character_save_buffer` :59) round-trip via `save_count` compare
(`CharacterFile.h:57` comment).

## 3. Assert-failing types vs save records — containment test

Method: full failing-site census re-extracted from the Phase-0 probe
(`cache/scratch/clangprobe/ilp32/*.err`): **46 LAYOUT_ASSERT lines across 20 files**
(abi.md's headline 43 undercounts; it omits `SoundReverb.h:30,31`, `TMovie.h:107` and
quotes the TCheckMemoryCard pins, which actually live at
`include/pso/system/save/TCheckMemoryCardTypes.h:18-19` — update abi.md).
Then: transitive `#include` closure of the save roots
(CharacterFile.h, GuildCardSaveData.h, TAdSinglePlyChrSelectGC.h, CardSaveFile.h,
TAdPlayerLoadGC_801E0C30.h, TPlyCharData.h, TItemData.h) = **38 headers**; scripted
scan of every failing type name for by-value members (`T name[;/=`) → **zero hits**.

### 3.1 Verdict table (deliverable 2): failing + BE-hazard types appearing in save records

**Empty for assert-failing types.** The BE-hazard-bearing types that DO appear in save
records are all assert-PASSING PODs:

| Type (in-save by value) | Site | Hazard class | Verdict | Evidence |
|---|---|---|---|---|
| `TItemData` (inside `BankItemRecord`[200] and `TPlyInventoryItem`[30]) | `include/pso/item/TItemData.h:56-69` | mixed-width union `data1{u8[12]/u16[6]/u32[3]}`, `data2{u8[4]/u16[2]/u32}` = cat5 (be-hazard-index: `include/pso/item/TItemData.h` cat3=1 lines 44;61;69) | **FORMAT-KEEP** | saved in CharacterSaveRecord.bank_items + TPlyCharData.m_inventory; own `bswap()` declared `TItemData.h:44`; mag stats packed via `serialize_mag` :51 |
| `gc_tag` (inside `TPlyGuildCardTag`, in `GuildCardEntry` ×0x1C+0xD2 and `TPlyDispData`) | `include/pso/player/TPlyGuildCardTag.h:17-33` | u16 `tag2` BE wire; named `bswap()` :24 | **FORMAT-KEEP** | `GuildCardEntry.h:31` by value; comment at :27 ties it to network packets too |
| `player_stats`/`stats` (TPlyCharData.m_stats) | `include/pso/player/PlayerStats.h:7-36` | 7×s16 + f32 + 3×s32; `bswap()` :9,:20 | **FORMAT-KEEP** | inside TPlyCharData 0x41C (`TPlyCharData.h:20`) |
| `TPlyInventoryItem.m_present[4]` (char array reinterpreted as packed u32 words) | `include/pso/player/TPlyInventory.h:16`; pack/unpack `TPlyCharData.h:40-43` (`unpack_u32_hi/lo`) | word-in-bytes packing — safe iff packing is byte-assembly; audit `fill_player_record` before trusting | **FORMAT-KEEP, swap TBD** | 30 items/slot in every saved character |
| `SymbolChatPreset`[12] | `include/pso/ui/chat/SymbolChatPreset.h:33,39-42` | has `bswap_entry()`; explicit wire format | **FORMAT-KEEP** | CharacterSaveRecord.symbol_chat_presets |
| `CardSaveHeader.size/crc`, cipher words (§5) | above | multibyte + word arithmetic | **FORMAT-KEEP** | CRC'd on both ends |

Not save-crossing despite living in `include/pso/system/save/`: `TSaveChCr*` PMF tables
(`TSaveChCr.h:59-67`), `CheckMemoryCardState` (`TCheckMemoryCardTypes.h:14-19`) — both
pure Cause-B UI state machines that *drive* the writes; their bytes never reach the card.

### 3.2 The 46/43 failing asserts are all save-free (deliverable 3)

Per-group containment argument (all types above are absent from the 38-header closure —
the strongest form of "no by-value path"; pointer fields into heap objects are runtime
references and do not serialize):

| Failing type(s) (sites) | Cause | Save-angle verdict + argument |
|---|---|---|
| `PlayerActionPaletteState` (1: `PlayerActionPaletteState.h:25`) | A | RUNTIME-WAIVE — action-palette wheel state; referenced only from player runtime (`TPlyItem.h` neighborhood); no save root contains it |
| `MaterialColorFade` (2: `MaterialColorFade.h:27,28`), `RenderFadeState` (1: `RenderFadeState.h:23`) | A | RUNTIME-WAIVE — render fade controllers; closure scan: no occurrence |
| `PlayerWindowCommand/Query` (2: `WindowCtrlCallback.h:9,10`) | B | RUNTIME-WAIVE — PMF callback pairs in compiled code, window ctrl only |
| `TBoss4DarkFalzHandler/State` (5), `TBoss4Type3State` (3), `TBoss6PlotFalz*` (5), `TBoss6Type2BattleAction` (3) | B | RUNTIME-WAIVE — boss state-machine tables, `static const` in code (pattern of `check_memory_card_state_table`, `TCheckMemoryCard.cpp:39`); bosses never serialize |
| `TWindowHelpItem` (2), `TWindowLockOn` (1), `CreationMenuEntry` (1) | A+B | RUNTIME-WAIVE — window objects are heap/TObject-graph, never saved |
| `PlacementQueue` (+`PositionPlacementQueue`/`ItemDropQueue` same header) (1: `TPlyItem.h:109`) | A | RUNTIME-WAIVE — in-world item-drop placement queue; held by pointer from `TTecLuker.h:20` and `TPlacementSearch.h`; the *saved* item form is `TItemData`/`TPlyInventoryItem` PODs, not the queue |
| `TObject`'s own offsets (7: `TSetEvtDestroy.h:70-76`), `TItemDisconnectCtrl` (1), `TObjDFRecover` (1) | A | RUNTIME-WAIVE — vptr shift in the scene-graph ancestor. Save records embed **no TObject-derived value member**: `TPlyCharData`/`CharacterFile` ctors/dtors are non-virtual (`TPlyCharData.h:15-16`, `CharacterFile.h:17-18,29-30`) so they carry no vptr; all window/object fields in save-path classes are pointers |
| `TSaveChCrStateAction/Actions` (3: `TSaveChCr.h:65-67`), `CheckMemoryCardState` (2: `TCheckMemoryCardTypes.h:18-19`), `TAdTimeSetGCState` (2: `src/pso/ui/frontend/TAdTimeSetGC.cpp:85-86`) | B | RUNTIME-WAIVE-from-saves — despite being the save-UI classes themselves, the failing members are the PMF state dispatch tables of transient windows; the bytes those windows hand to `CARDWriteAsync` are the CharacterFile/GuildCardSaveData/SystemSaveData images, whose asserts all pass |
| `SoundReverbDelay/SoundReverb` (2: `SoundReverb.h:30-31`), `MovieState` (1: `TMovie.h:107`) | (not in abi.md list) | RUNTIME-WAIVE — audio engine scratch / movie playback state; not in closure |

**Net: 0 of 46 (0 of abi.md's 43) failing asserts touch the save format; all 46 are
RUNTIME-WAIVE at this boundary.** Phase-0 policy (host layout authoritative for virtual
classes + PMF tables) is therefore safe for saves unconditionally; no save struct needs
Cause-A/B pinning. Caveat: this waives the *saves* angle only — network (BBA) and disc
archives remain separate audits (several of these types, e.g. `PlayerWindowCommand`, are
network-adjacent runtime code, but wire structs there use protocol.h PODs).

## 4. Bitfield hazards vs saves

`be-hazard-index.csv`: the two live bitfield structs (`include/pso/network/NetTcpState.h:92-94`
TCB = network runtime, `include/pso/effect/TEF_LensFlare.h:25` = effect render) are **not
in the 38-header save closure**, and a scripted bitfield-decl scan over all 38 closure
headers returned **zero declarations**. No saved record uses a C bitfield. Closest in-save
analogue: `TObjTombstoneRecord.h:28` `u32 date` = explicit shift-pack
(`year-2000 << 28 | ...`) — value arithmetic, endian-safe, only whole-word swap needed.

## 5. Byte-swap needs + where the port's swap layer goes

All card bytes are BE. Requirements by layer:

1. **Host card shim** (per host-surface.md memory-card row): own file-backed image;
   GC card-volume BE layout (dir entries, CARDFileInfo/CardStat, CardWriteDate year u16)
   is *its* format contract. Decide: byte-true GC image vs. private container.
2. **Payload schema swap on the game side** — natural seam is the existing memcpy
   boundaries between raw image and typed RAM copies:
   * load: `TAdSinglePlyChrSelectGC_801FBF54.cpp:399,488` (SystemSaveData), `:1955`
     (GuildCardSaveData), `:1999,2017`; character path after `verify_card_save_header` +
     `decrypt_char_data` (`:439,672,798,1846,2110`) — swap BE→host;
   * save: before `build_save_file_image` `:2237,2255,2276` and the `encrypt_save_data`
     call sites (`:701,842,1574,1632,1911,2153,2162,2171`) — swap host→BE.
   Scaffolding already in-tree (currently wired only for network packets,
   `src/pso/network/protocol/packet_classes.cpp:24+`): `TPlyCharData::bswap`,
   `TPlyInventory(:Item)::bswap`, `TItemData::bswap`, `player_stats/stats::bswap`,
   `gc_tag::bswap`, `GuildCardEntry::bswap`, `SymbolChatPreset::bswap_entry`.
   Missing coverage to write: CharacterSaveRecord aggregate walk, BattleRecord,
   ChatShortcut.kind, ChallengeRecord, ChoiceSearchSelection, SystemSaveData scalars,
   GuildCardSaveData scalars (checksum/timestamp/seed), CardSaveHeader.size/crc.
3. **Cipher/permute must stay byte-true**: `apply_rng_cipher`/`xor_buffer` do u32
   arithmetic (`src/pso/system/SubtractiveRng.cpp`, `data[i] = next() - data[i]` / XOR).
   Port fix = swap each word in before and after (or reimplement byte-wise):
   `out[i] = bswap(next() - bswap(in[i]))`. `permute_save_blocks`/`unshuffle`/`calc_crc32_bitwise`
   already operate on `u8*` — endian-safe as written. Without the cipher fix, host
   saves load on host but corrupt GC images — treat as a P1 test: byte-compare
   encrypt(default seed) vs a reference GC dump.
4. **Union reinterpret risk**: `TItemData.data1/data2` (mixed-width unions) must be
   swapped *as words* and then read via u8/u16 lanes; a whole-record swap of the item
   is wrong for the u8-pair lanes only if swap is partial — pin one swap granularity
   (per 32-bit word) in the port's item layer. `m_present[4]`-as-u32 packing
   (`TPlyCharData.h:40-43`) needs the same convention (hypothesis: it is byte-assembly;
   verify `fill_player_record` before shipping).

## 6. Unknowns (labeled)

* `SystemSaveData.data_10[0x110]` is unnamed option bytes — schema of inner multibyte
  fields un-decompiled; the swap walk cannot be completed until those offsets are named
  (P1-H candidate). Options UI (`TWindowOptions*`) writes through it by pointer —
  containment *inside the payload* is certain (it's an inline array, FORMAT-KEEP).
* `GuildCardList.pad_0[0xC0]` head block unnamed (likely counts/flags + banner cache) —
  same treatment: byte-true opaque until decomposed.
* Whether pso-metal targets *GC-card-compatible* saves is a product call; everything in
  §5 assumes yes (FORMAT-KEEP). If host-private saves are acceptable, the swap layer
  collapses to the shim and the cipher becomes internal — but guild-card/network
  exchanges of `TPlyCharData`/`gc_tag` still force BE wire (host-surface.md verdict).
