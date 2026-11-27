# P2-AD — Verdict resolutions, settlement pass 1 (tranche-2 UNRESOLVED × 60)

Phase-2 lane AD. Resolves **all 60** UNRESOLVED types of `data/verdict-tranche2.tsv`
into `data/verdict-resolutions-1.tsv` (type | new-verdict | evidence-type | file:line |
confidence). Decomp `6b38b5383` READ-ONLY, zero builds. This resumes and completes the
storm-killed first AD run (which had read the FORMAT tables + declaring headers but died
at the overlay-grep step; no resolution rows were ever written — all evidence below is
re-derived fresh in this pass).

## 1. Counts & site delta

| Verdict | Types | LP64-fail sites |
|---|---:|---:|
| RUNTIME-WAIVE | **51** | **94 newly-waivable** |
| LOCKED-FORMAT | **9** | 27 stay pinned |
| STILL-UNRESOLVED | **0** | 0 |
| total | 60 | 121 (per-type `lp64-fail-sites` attribution of tranche-2) |

Confidence: 49 HIGH / 11 MED (MED rows listed in §4).
Delta lands on R/S's sim: +94 sites waiver-eligible on top of tranche1–4's 1196-RW-site
aggregate (S's 825-site anchor sim in `p2-verdict-tranche3.md` unaffected; stitch lane
recomputes the global 1680 tally at consumption). Note: the (1/2) commit message quoted
"102 newly-waivable" — pre-script estimate; **94 is the measured number** (sum of the
`lp64-fail-sites` column over the 51 RW rows, script `ad_emit.py`).

## 2. Precedence

**These resolutions OVERRIDE the tranche-2 UNRESOLVED rows.** Stitch consumes last:
where `verdict-tranche2.tsv` says UNRESOLVED and `verdict-resolutions-1.tsv` gives a
verdict, the resolution wins; the tranche row's `evidence` text is superseded, not merged.
LOCKED-FORMAT rows are permanent format pins — a future waiver re-review must re-prove
byte-absence against the G/H tables, same bar as X/S used. RW claims here were each
verified ABSENT from the p1-format-saves.md §2/§3.1 record tables and p1-format-net-disc.md
§1.1–1.3/§2.1 D1–D11 inventories first (fail-closed ordering respected).

## 3. Method (what settled each class)

1. **Pointer-member exclusion** (`POINTER-CLASS`/`FUNCTION-TABLE`): a struct holding live
   `T*`/fnptr/OS-handle members cannot be a byte image — its PPC bytes would be host
   addresses. Settles all four NetworkRuntimeSlots, the pointer-chained QuestDownload
   nodes, AramBlock/Pool, PppLink, TcpSendRequest, ModemFieldSlot, MapSet{Entry,Area,
   Variant}, LobbyFloorRows, LobbyHandler(Table), NetworkInfoLoadState.
2. **Direct overlay-cast grep** (`OVERLAY-CAST`): who casts a load buffer to the type.
   Positive hits → LOCKED (EnemySetRecord/FloorObjectSetRow, NetworkInfoCardFile);
   negative (only field-wise copies) → RW (ParsedIpHeader — the write path overlays the
   SEPARATE `IPv4Header` type on a `u8 wire[0x40]` image buffer, `PppFsm.c:726-760`;
   NetworkConfig — assembled field-wise at `NetworkConnect.cpp:471ff`).
3. **G/H FORMAT-table explicit rows** (`FORMAT-TABLE-G/H`): types named verbatim in the
   Phase-1 inventories settle by citation — net-disc §1.3/§2.2/§4 waive the modem/PPP/
   NetworkRuntime*/TGStream/CvFs/RlcArchive families; §1.2 counts FloorEventCommand.h in
   the 314-record wire set → G_FloorEvent_6x67 LOCKED; saves §1 waives the ROM metadata
   constants but pins CardFileInfo (card-volume BE) → split verdicts for
   Character/EpisodeCardMetadata (RW) vs CardFileInfo (LOCKED).
4. **abi.md vclass/PMF policy** (`VIRTUAL-CLASS`): TObject/TShopWindowBase/TSocket
   subclasses (TQuestListVM, T{Challenge,Challenge4P,MapCsnCtrl,QuestBgmCtrl,
   QuestListChallenge,UdpSocket,Packet}) — host layout authoritative, asserted independently
   format-free by H §3 ("assert-crossing: 0") and G §3.2.
5. **Compiled-in tables** (`COMPILED-TABLE`): const .rodata arrays defined in the decomp's
   own code (`quest_download_entry_table`, QuestDownloadInputEntry walk at
   AskDicLookup.cpp:1038-1075; QuestListStageEntry tables TQuestListChallenge.cpp:281-297)
   — same class as abi.md cause-B PMF tables: recompiled, never loaded.

## 4. Per-type rationale

MED rows carry the residual-risk note; HIGH rows: the cited file:line proves it directly.

| Type | Verdict | Rationale |
|---|---|---|
| ChallengeModeState | RW (MED) | Session global; holds `TWindowMessage*` (ChallengeModeState.h:49), copied only via implicit `operator=` RAM-to-RAM (header comment :31); format bytes flow at **ChallengeRecord** granularity (field-wise copy + save-blob cast `*(ChallengeRecord*)(save+0x21C4)`, ChallengePackets.cpp:146,158) — that layout stays pinned by ChallengeRecord's own LOCKED row. |
| ChallengeClientRecords | LOCKED (MED) | `{s32 present; ChallengeRecord records}` embeds the card-saved ChallengeRecord (saves §2:66) by value; fail-closed containment per lane law (waiving the wrapper tolerates format-member drift at a format-adjacent site). Re-review candidate: wrapper itself is never imaged (field-wise assignment only). |
| ChallengeRankOverride | RW (MED) | Written by quest VM opcodes (`set_cmode_rank_override`, header comment :22); sole widening field `s32 color` is ARGB runtime; no load/overlay site in tree. |
| CardFileInfo | LOCKED | SDK card-volume dir entry, BE, "belongs to the host card-image shim's format contract" — saves §1:51-53 verbatim. |
| MD5Context | RW | Hasher context; PppMd5 is LE-word-assembly, endian-safe (net-disc §1.4:105, §4:187 decoders row). |
| PppChapAuthenticationState | RW | Embeds `PppFsmProtocol`; "PppFsm/Lcp state — pure in-process" RW (net-disc §1.3:78); CHAP wire bytes go byte-serial. |
| PppLink | RW | `s16 *used; u8 *buffers` live pool (NetPppLink.h:23). |
| ParsedIpHeader | RW | Decoded cache (`"An IPv4 header decoded by NetIPv4ReadHeader"`); wire image uses separate pinned `IPv4Header` overlay + field-wise write (PppFsm.c:726-760). |
| AramBlock / AramPool | RW | Free-list `next*` + C++ pool class (AramPool.h:9,21); pool PAYLOADS carry disc bytes but the headers are bookkeeping (net-disc §2.2 runtime-built). |
| ModemSessionState / ModemFieldSlot / ModemConnectionInfo | RW | net-disc §1.3:78 names ModemRuntime/ModemSession RW; union holds `u8* text`; ModemConnectionInfo MED (EXI stack retires wholesale per host-surface). |
| CharacterCardMetadata / EpisodeCardMetadata | RW | ROM lookup constants "never written to card" — saves §1:46-50 verbatim. |
| EnemySetRecord / FloorObjectSetRow | LOCKED | Direct overlay on disc archive: `(EnemySetRecord*)source` (TLobby_80207DA8.cpp:172, TLobby_80209524.cpp:1063), `(FloorObjectSetRow*)map_set_file_buffer` (+ `byte_size/68U` stride walk, TLobby_801F3A24.cpp:284, :714, :764, :895). Pointer slots hold PPC-era addresses patched post-load = p1-datamodel §3 rule-1 handle class. |
| LobbyFloorRows | RW | Bookkeeping with `FloorSavedSetRow*` pair (FloorPopulation.h:46); placement u32s are offsets retained for reload. |
| LobbyHandler / LobbyHandlerTable | RW | Ten-function-pointer link dispatch table (netcfg.h:22-46). |
| TQuestListVM | RW | Virtual window subclass; quest bins' payload "opaque encrypted blob — no host struct exposure" (saves §2:19,69). |
| TcpSendRequest | RW | Queue node: `NetBufferRef` composite handle + callback fnptr (TcpSendQueue.h:10-14). |
| TChallenge4P / TChallenge | RW | TObject subclasses (abi vclass policy). |
| TGStream | RW | net-disc §2.2:127 "whole stream cursor is runtime state — except the archive_entry pointer" (D1, a different type). |
| NetworkConfig | RW (MED) | Runtime aggregate fed field-wise by `load_network_configs` (NetworkConnect.cpp:471+); no memcpy/&-cast of the struct anywhere (verified absence); its SOURCES (Account/Lan records in the NetworkInfo blob) stay LOCKED via D-family rows. |
| CvFsDeviceInterface | RW | fnptr device-op table; named RW net-disc §2.2:126 ("CvFs/Adxf device state"). |
| G_FloorEvent_6x67 | LOCKED | 6x wire packet; FloorEventCommand.h counted in the 314 FORMAT-KEEP wire records (net-disc §1.2:44,46); embeds `G_ClientIDHeader` (explicit FORMAT-KEEP, :31). |
| QuestDownload{Request,Slot,TextNode,EntryNode,EntryPrefix,MED,ItemNode,QueryResult,QueueNode,Segment,Workspace} | RW | Pointer-chained RAM nodes built by the recursive dictionary search (`insert_quest_download_node`/`build_item_list`, AskDicLookup.cpp:758-917); download bins never expose host structs at the boundary (saves §2:19). EntryPrefix/QueryResult MED (struct-head fields not individually read; node-chain usage is the proof). |
| QuestDownloadInputEntry / quest_download_entry_table | RW | Compiled-in lookup table (`extern const`, dictionary walk AskDicLookup.cpp:1038-1075) — cause-B class. |
| NetworkThreadSlot / NetworkAlarmSlot / NetworkMessageQueueSlot / NetworkMutexSlot | RW | "NetworkRuntime*" RW, named net-disc §4:181; stack/handle pointers inside. |
| RlcArchive | RW | Named RW net-disc §2.2:126 ("RlcArchive runtime tables"); holds `RlcHeader*`. |
| TUdpSocket / TPacket | RW | Virtual socket classes; the 0x5B4 send buffer crosses by BYTES (net-disc §1.4:96), the class layout doesn't. |
| NetworkInfoLoadState | RW | Loader phase machine with `void *caller_buffer` (NetworkInfoLoader.h:7-8). |
| TMapCsnCtrl / TQuestBgmCtrl | RW | TObject subclasses. |
| NetworkInfoCardFile | LOCKED | The `sega_network_information` card file itself: heap block of `get_network_info_file_size()` cast to the struct (NetworkInfoLoader.cpp:100,110); `crc/payload_size` are card bytes — same D5-D10 blob pipeline, §5-rule-3 chokepoint (net-disc:222). |
| TQuestListChallenge | RW | Virtual window subclass. |
| QuestListStageEntry | RW | `extern __declspec(section ".rodata") const` tables DEFINED in decomp code (TQuestListChallenge.cpp:281-297) — compiled-in, recompute with host. |
| LobbyBufferSlot | RW (MED) | Per-slot bookkeeping {active,size,addr} (NetConfig.h:10-14); `u32 addr` is a pointer-space handle → TYPE-MODEL item (uaddr rule-2), not a layout-format item. |
| MapSetEntry / MapSetArea / MapSetVariant | RW | Compiled/constructed catalogs holding `const char*` + child pointers (MapSetArchive.h:8-19); the buffers they NAME are the LOCKED row sets above — the catalog structs themselves are not overlaid anywhere. |
| NJS_POINT3COL / NJS_TEXNAME / NJS_TEXLIST | LOCKED (MED) | Family precedent: tranche-1 NJS_OBJECT/NJS_MKEY_F UNRESOLVED notes settle NJS_* as **disc-overlay class: "handle conversion (lane J), not a waiver"**; p1-datamodel §3 rule-1 (u32 handles + base register) is the mandated fix. Point3COL in the loaded triangle streams; TEXNAME/TEXLIST `texture*`/`filename*` slots are load-patched (abi.md §2:88 "cross-module pointer table loaded from data"). |

## 5. Rule corrections (flagged loudly)

1. **The tranche-2 blanket evidence line is invalid as stated.** "LP64-only fail site"
   was treated as pointer-overlay suspicion, but `types.h:8-9` typedefs `s32/u32` as
   `long` — on LP64 **every** s32/u32 field doubles width. A pure-POD struct with zero
   pointers fails size asserts purely from the typedef. Overlay suspicion must require a
   positive overlay-cast hit, which this pass performed for every non-table-cited type.
   Consequence: p1-datamodel §3 rule-1 (rebind `u32/u16/u8` to `<stdint>`) is a
   PREREQUISITE for compiling the 9 LOCKED types byte-true; until it lands, "LOCKED"
   here means *assert held + type-model fix owed*, and gen_waivers must keep their KEEP
   flags (they fail closed already — no action needed beyond not waiving them).
2. **net-disc §2.2 overreaches on Ninja/floor rows.** "Ginja/Ninja runtime structs built
   *from* D4 payloads → WAIVE" contradicts the tranche-1 NJS_* note AND the direct
   overlay casts found on the floor rows (`(FloorObjectSetRow*)map_set_file_buffer`).
   Reading: structs that are the loaded image itself (rows, texname/texlist, point3col)
   are FORMAT-KEEP; only structs built via field-wise copy from payloads waive. Lane J
   handle-conversion is the resolving mechanism for all six of these (EnemySetRecord,
   FloorObjectSetRow, NJS×3 — rows stay LOCKED meanwhile).
3. **X's Cipher\* correction consumed as-is**: the NetworkInfoCodec carve-out (doc-named
   beats path) is why NetworkInfoLoadState (NetworkInfoLoader.h) reads RW while the D-family
   blob types and the CardFile envelope they stream through stay LOCKED — no conflict.
4. Tranche row for `quest_download_entry_table` lists it as a type; it is an extern const
   ARRAY of QuestDownloadInputEntry — same verdict either way, noted for stitch parsing.

## 6. Scope & handoff

Nothing open: 0 STILL-UNRESOLVED, so no per-type evidence-state handoff list is needed.
`data/quest-blockers.tsv` (lane AC) untouched; `data/verdict-tranche*.tsv` untouched
(resolutions live only in `data/verdict-resolutions-1.tsv` + this doc).
For gen_waivers v2: add the 51 RW types (94 sites) here and dedupe against tranche-1/3/4
lists; do NOT add the 9 LOCKED types.
