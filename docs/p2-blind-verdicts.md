# P2-AH-adjacent — P2-AL blind-set adjudication, priority tier (101 types / 507 sites)

Phase-2 lane AL. Adjudicates the ≥3-site priority tier of AK's blind census
(`data/census-audit.tsv` @ 9cc6e20, 667 types / 1,130 sites) into
[`data/verdict-resolutions-4.tsv`](../data/verdict-resolutions-4.tsv)
(101 rows, 5-col resolutions grammar; resolutions override tranches at stitch, per
resolve-1 §2). Method = S/AH precedent verbatim: declaring-struct read → pointer/vtable
exclusion → decisive overlay grep on DVD/archive/quest-bin/card paths → G/H FORMAT-table
belt check → era `.s` not needed (no C-ambiguous cases reached that step). Decomp
`6b38b5383` READ-ONLY, zero builds. All `file:line` cites resolve against the decomp
tree unless they point at a pso-metal doc/data row.

## 1. Verdict split (101 types / 507 sites)

| Verdict | Types | Sites | Evidence mix (types) |
|---|---:|---:|---|
| RUNTIME-WAIVE | **91** | **452** | VIRTUAL-CLASS 66, POINTER-CLASS 15, FIELDWISE-RUNTIME 8, FORMAT-TABLE-H 2 (doc-named waivers) |
| LOCKED-FORMAT | **10** | **55** | AK-pre-collision 5, OVERLAY-CAST 2, FORMAT-TABLE-G 1, FORMAT-TABLE-H 2 |
| STILL-UNRESOLVED | 0 | 0 | every priority type decided |

Rows are sorted by files-hit leverage (65→1), checkpoint-committed every ~15 rows.

## 2. The 15 pre-collision rows — 5 consumed, 10 refuted (LOUD)

The priority tier carries 1 of AK's 8 hard `yes` rows + 14 of the 72 `probable` rows.
Per task law the `yes` row and any `probable` row whose confirm-read holds were emitted
as LOCKED-FORMAT / `AK-pre-collision`, not re-derived:
`CharacterSaveRecord` (8s — G:66 FORMAT-KEEP aggregate), `SetDestroyEventRecord` (6s —
confirm found the real pin: `read_disc_file` → `(FloorEventSet *)set` → `bswap_floor_event_set`,
`AreaEventSet.cpp:290-292`, abi §2 names `.evt` streams), `BootpPacket` (5s) +
`DhcpClientBuffers` (3s) (BOOTP/DHCP wire images, AK §5 Bootp/Dhcp family), `BmlArchiveEntry`
(3s — confirm pinned: `reinterpret_cast<BmlArchiveHeader *>(bml_load_buffer)`, `BmlResource.cpp:172`).

**The other 10 `probable` rows FAIL the quick-confirm read** (AK §5 itself required a
doc check before trusting `probable`; the check refutes them — they hold live pointers,
vtables, or are doc-named RUNTIME-WAIVE):

| Type | sites | Why refuted (evidence in row) |
|---|---:|---|
| `TSetEvtDestroy` | 6 | **G:120 AND H:166 explicitly rule it RUNTIME-WAIVE** (cause-A TObject offsets; "the overlay is not pinned — event records walked by byte math") — the pre-collision row contradicts the very tables it cites |
| `PppProtoBlocks` | 6 | H §1.3:78 waives "PppFsm/Lcp state — pure in-process"; struct = 5 inline `PppFsm` + phase byte (AD's PppChapAuthenticationState/PppLink RW rows, same family) |
| `TQuestListOnline` | 15 | virtual `TShopWindowBase` subclass + 4 fn-ptrs + window-pointers — AD's TQuestListVM/TQuestListChallenge VIRTUAL-CLASS precedent |
| `TObjMapCsn` | 5 | virtual scene object `TObjCollidable` subclass; AD ruled the same-header `TMapCsnCtrl` RW |
| `NetSession` | 4 | `char *user/*password/*phone_number` + `NetStatusCallback` — same header as AD's `ModemSessionState` RW row (ModemSession.h) |
| `NetBufferPool` | 3 | `u8 *buf/*data` pool header = bookkeeping, AD's AramBlock/AramPool precedent ("payloads carry bytes, the header doesn't") |
| `QuestDownloadCachePage` | 4 | AD's 12-row QuestDownload* family RW (pointer-chained RAM cache; bins stay opaque, saves §2:19) |
| `BmlModelLoadRequest` | 3 | `const char*` + `NJS_OBJECT**` + loader pointers — runtime request node (AJ's BmlResource walk builds these) |
| `OnlineQuestDownloadState` | 3 | `u8 *buffer` download-state node, same AD family |
| `ChallengeFixedEvent` | 3 | decoded cache: `memset` + field-wise assign walk (`TLobby_80209524.cpp:371-399`) — the raw image stays in `challenge_event_source`/`challenge_enemy_buffer` u8* (AD's ParsedIpHeader precedent) |

The 10 flips ride MED confidence where they rely on family analogy (NetBufferPool,
ChallengeFixedEvent) and HIGH where the doc/AD row names the same header or type.
**Rule correction proposed (§6.1): `probable` must not be pre-pinned as LOCKED — only G/H
*table-named* types (`yes`) meet AD's LOCKED naming bar.**

## 3. Family collapses (single-verdict site wins)

The `gh=no` residue is dominated by the scene-graph/window class family that AH settled
for quest types: **66 VIRTUAL-CLASS verdicts clear 320 sites** — every one is a
`TObject`/`TObjEnemy`/`TObjBase`/`TWindowBase`/`TItemAttackColBase`-lineage live object
whose header casts are runtime downcasts. Top single-type collapses: `TObjEneNanoDrago`
26s, `TObjEneBm5Wolf` 16s, `TPhotonBlastSequence` 12s, `TBoss4Type3` 11s, `TBoss6Type2`
10s, `TWindowCtrlPlayer` 8s, `TWindowFukidasi` 8s, `TBoss6Type1`/`TBoss5Gryphon`/
`TBoss7DeRolLeC` 7s each (VIRTUAL-CLASS 66 types / **347 sites**; POINTER-CLASS 15/60;
FIELDWISE-RUNTIME 8/33; doc-named waiver 2/12). Sister types share declaring headers and take one grep each:
`TContainerAncient01(+R)` (TObjChara), slime quad (`TObjEneSlimeOne/TailAttack/WaterBomb`
+ `TObjEneSlime*` family), keyboard pair (`TWindowKeyBoardSoft/Base`), DD_WS pair,
symbol-chat trio, DarkFalz cluster (`TBoss4Type1/Type3/Zako`, `TObjDF*`), OlgaFlow
cluster, DeRolLeC/Crawfish (Barbaray). POINTER-CLASS catches 15 more (audio streamers
`AdxtPlayer`/`SoundRequestVoice`, `DelDepthMotion`'s 7 NJS-host pointers, `GcciRequest`
DVD-callback arg, `EventShuffleTable`, `RawIpv6Receive` via `NetworkReceivePrefix.u8 *buf`,
`NetworkPacketBufferDescriptor` (`data <= packet < data+0x600` bookkeeping),
`ShinowaBattleTables` — pointer arrays either way). FIELDWISE-RUNTIME for 8 overlay-negative
runtime PODs: `DragonAttackParams` (TBoss1Dragon family, dedicated `copy_...` fn),
`TObjFishEntry` (arena-alloc `sizeof`-strided, per-frame retargeting),
`Boss4ZakoDrawBuffer/Entry` (extern scratch globals), `EquippedItemDrawQueue`,
`CreationTextureCacheEntry`/`CharacterPreview` (hold `TChCrLoad*Req*` → also POINTER-class),
`TEnding` cluster.

## 4. New LOCKED-FORMAT escalations among `gh=no` (5 types / 27 sites)

| Type | sites | Decisive evidence |
|---|---:|---|
| `PlayerSyncRecord` | 7 | `TLobby::bswap_player_data(PlayerSyncRecord*)` — `TLobby.cpp:2668`; embeds AH-locked `PlayerPkState`/`PlayerTelepipe` (6x game-state, fail-closed both ways) |
| `TProtocol` | 8 | embeds **four** locked byte images by value: `TPlyCharData` (0x41C blob), `QuestListEntry m_quest_entries[30]`, `TPlyClientConfig`, `TPlyGuildCardTag` — and its own assert pins them: `offsetof(TProtocol, m_guildcard_tag) == 0x1900` (`TProtocol.cpp:590`). Containment lock per AD's ChallengeClientRecords/TLobbyFloorItemEntry logic; MED — the *session-state* asserts (m_connected/m_game_entries) don't cross bytes, re-review candidate post rule-1 |
| `TSendAction` | 9 | `packet_classes.h:413+` template: `packet_header + TPlyGuildCardTag + bswap()` — exactly the shape AH locked for the other 5 packet_classes.h types and the TSend* trio (§1.2 wire census family); 6th member was cap-hidden |
| `Pso3CharacterSaveRecord` | 3 | `TAdSinglePlyChrSelectGC_80202A68.h:43`: `BankItemRecord[4]`/`SymbolChatPreset[12]`/`ChatShortcut[20]`/`BattleRecord×2`/`ChallengeRecord`/`ChoiceSearchSelection[5]` — the PSo3 GC-card clone of G:66's aggregate layout |
| `WallHitPacket` | 3 | `reinterpret_cast<WallHitPacket *>(command)` over a received game-command (`TOVS2Wall01.cpp:94`) + `byteswap()` + `send()`; base = `G_UpdateObjectState_6x0B` wire record |

These 5 are the priority tier's real landmines: two of them (TProtocol 2 files,
TSendAction 1 file) are low-files-hit types that per-type site-count ranking buried —
name-by-header batching in the tail lane should hit the rest of `packet_classes.h`.

## 5. Projected stitch-anchor movement — measured, with corpus caveat

**Caveat (loud):** AK's canonical `p2t_uncapped/err` tree is no longer on disk (scratch
cleaned); `cache/assert-probe/logs` holds an uncapped LP64 probe of **3,010 units**
(2,928 failing) — a superset that includes standalone-header units and predates the
current force-include waiver header (it carries `TObject`/`TAdTimeSetGC` sites absent
from AK's named ∪ blind universe). Flip counts below are measured on THIS corpus with
AK's blob-window grammar + source-window fallback; treat them as corpus-shifted, not
AK-exact.

* Replayed per-unit (file flips iff every failing type is named RW after merge, zero
  non-layout errors, zero unattributed sites): **20 units flip purely from resolutions-4**
  (pre-AL only 812 named RW, none of them blind). All 20 are the standalone-header unit
  of the flipped type (`TWindowInputNumber.h`, `TWindowFukidasi.h`, `AdxtInternal.h`,
  `EventShuffleTable.h`, `TEnding.h`, …); **0 src-TU units flip** — every src TU that
  fires my types also fires tail-blind (1–2 site) or LOCKED wire types in the same unit.
* One-step-away after AL: 2 units (`TObjCity_Season_FireWork.h` — residual is the
  corpus-only `sizeof(TObject)` site; `NetworkPacketRuntime.c` — residual is the
  `Ipv6cp*`/`NetBufferRef`/`NetThreadState` tail cluster).
* Tail sample (378 units improved but not cleared, 93 of them at ≤3 residual types):
  residuals concentrate in the `NetIpv6Route`/`PppVj`/`Bootp`/`Dhcp`/`Ipcp` wire family
  (LOCKED-destined if the tail lane consumes pins like §2), plus `Bml*`, `Ipv6Nd*`, `Sj*`/`Adx*`
  audio 1-site types. Extrapolation, honest: my 507-site tier alone cannot flip a single
  heavy src TU (QuestOpcodeHandlers-style files mix RW bosses with ~55 LOCKED sites +
  source-rewrite asserts — AH §3 result repeats); the 20-unit header flip is the ceiling
  for this tier, and the tail tier (566 types, header-batched) is where src-TU flips
  unlock. Re-sim on AK's actual `p2t_uncapped` corpus (or AO's stitch regen) before any
  parse-gate target is quoted — AK §8 already demands that.

## 6. Rule corrections flagged

1. **AK's `probable` ≠ pinned evidence.** 10 of the 14 priority-tier `probable` rows
   refute the quick-confirm read (§2 table) — path-adjacency flags a *file's format
   density*, not a *type's byte-image status*. Lock only on `yes` (G/H table-named);
   keep `probable` as "check first", exactly as AK §5's own sentence says. The 73
   remaining probable tail rows should not be pre-pinned when the tail lane batches.
2. **A pre-collision row contradicted its source table.** `TSetEvtDestroy` is written
   as RUNTIME-WAIVE in *both* p1-format-saves.md:120 and p1-format-net-disc.md:166 yet
   landed in AK's "LOCKED-by-evidence" 80-row set (the row cites it as evidence of a
   collision, the evidence says the opposite). AK's §5 yes/probable split conflated
   "site in a format-adjacent dir" with "named in the tables" for 72 rows.
3. **TProtocol-type containment needs a member-offset carve-out.** Its 8 sites split:
   ~2 pin embedded wire-blob offsets (locked), ~6 pin runtime session ints (would be
   RW standalone). Lane law locks the type (fail-closed, AD precedent), but gen_waivers
   *site*-granularity (a future idea) would reclaim 6 sites — flagged for the stitch lane,
   no row changed here.
4. **Corpus drift on disk:** `cache/assert-probe/logs` ≠ AK's `p2t_uncapped` (TObject,
   TAdTimeSetGC, FireWorkOffset-families fail there but exist in no census universe,
   3,010 vs 1,610 units). Any per-file flip math downstream must state which corpus it
   re-simmed.

## 7. What remains

Priority tier: 0 STILL-UNRESOLVED. Open work is queue-level, not verdict-level:
tail 566 types / 623 sites (AK §8 header-batch rule, now with §6.1's pin-bar);
re-spell sources for the 55+10 LOCKED priority sites (source lane, same class as AH §5.2);
p1-datamodel §3 rule-1 stays the compile precondition for every LOCKED row; AO's stitch
dry-run should consume `verdict-resolutions-4.tsv` last (override order 1→2→3→4).
