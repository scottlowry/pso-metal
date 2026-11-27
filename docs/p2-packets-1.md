# P2-PK — packet_classes.h KEEP-assert cluster: survey, sub-classification, asks

Phase-3 lane PK (card t_b871495a), 2026-11-27. Decomp `pso-decomp @6b38b5383`
READ-ONLY; zero repo builds; waiver header untouched (sha `c6b514035f39b633…`
re-verified this session). Writes: `compat/packets/**` (new, inert) + this doc.
Inputs: docs/p2-collision-debt.md §4.1/§5, docs/p1-format-net-disc.md
(Keeper-of-FORMAT law), docs/assert-policy.md (§4 gate/deny-list),
docs/p2-njs-confirm.md. The on-disk official battery
`build/parse/summary.txt` (07:23) still reports `target=arm64_32-apple-watchos7.0`
— the pre-fix mislabel the card flags — so all numbers below come from scratch
`clang -fsyntax-only -ferror-limit=0 -target arm64-apple-macos11` sweeps over
the battery-3 corpus (1,680 units, `$TMP/b3/all.txt`), pin law exact:
`-include compat/pso_metal_compat.h -include build/waivers/pso_assert_waivers.h
-include compat/collision/pin_dolphin_types.h` in order; repin+njs `-I` only.

## 1. Enumerated failing set (measured — nominal "~57" is stale)

| shape | clean (full corpus) | cluster TUs (negative-size line in `packet_classes.h`) |
|---|---:|---:|
| H1C (rebind + pin) | 261 | **64** |
| H3 (full stack + pin) | 1,268 | **64** (same membership, diff=0) |

The cluster is **one header row poisoning 64 TUs**: `packet_classes.h:376
LAYOUT_ASSERT(PsoCommandHandler_size, sizeof(PsoCommandHandler) == 8)` — the
only negative-size line from this header that fires; 64/64 TUs carry exactly
it (python-counted over the 3,360 sweep logs; membership set-diffed
H1C↔H3 = empty). All 64 are `.cpp` (zero `.c` units, so the njs `-I` rule is
moot for this class). 57→64: §4.1's snapshot predates sibling-lane supplies
that now let 7 more TUs parse deep enough to reach row 376.

## 2. Assert-row sub-classification (per row, honoring the deny-list)

| row | assert | class | LP64 measured (1,680 TUs × 2 shapes) | action |
|---|---|---|---|---|
| 255 | TPlyJoinLobbyEntrySize == 0x43C | **FORMAT-KEEP** (wire, p1-net-disc §1.2) | PASS 1,680/1,680 | none — stays KEEP |
| 256 | TPlyJoinLobbyEntriesOffset == 0x10 | **FORMAT-KEEP** (wire) | PASS | none — stays KEEP |
| 257 | TPlyJoinLobbySize == 0x32E0 | **FORMAT-KEEP** (wire) | PASS | none — stays KEEP |
| 298 | TRegisterSize == 0x34 | **FORMAT-KEEP** (wire) | PASS | none — stays KEEP |
| 316 | TSendPsoRegistSize == 0xA4 | **FORMAT-KEEP** (wire) | PASS | none — stays KEEP |
| 521 | TPlyChallengeSize == 0x11C | **FORMAT-KEEP** (wire) | PASS | none — stays KEEP |
| 376 | PsoCommandHandler_size == 8 | **RUNTIME-WAIVE** (compile-time `.data` dispatch-table entry; not in p1-net-disc §1 wire/disc inventories) | **FAIL** on all 64 cluster TUs (u16+u16+8B fn-ptr = 12B LP64) | §5 asks below |

**Deny-list check passes:** the single ask roots in a runtime-table struct;
no FORMAT-root waiver is requested or needed — the six wire-record asserts
are measured honest at true LP64 and their flags stay `0`. The era 8-byte
image truth is *preserved*, not waived: `compat/packets/
pso_command_handler_era.h` pins it with `static_assert(sizeof==8)`.

## 3. Per-TU receipts (before/after = scratch, python-counted)

AFTER shape = identical pin law with the waiver header replaced by a
scratch copy whose **only diff (1 line, 4 diff lines) is
`PSO_LA_FLAG_PsoCommandHandler_size 0 -> 1`** (scratch sha
`4cd64e7b…`; pinned header never modified). Before/after clean among the
64: H1C 0 → H1CA 10; H3 0 → H3A **44**. In the 20 H3A residuals, zero
lines mention `packet_classes.h` or `PsoCommandHandler_size` (grep of all
20 logs = 0): they fail only on other lanes' classes.

Residual ledger: NJS-handle §4.2 ×14, FloorPopulation §4.6 ×2, PppFrame
§4.3 ×1, long-tail §4.5/§4.9 ×4 (`__destroy_new_array`, MWCC decl dialect,
`read_byte_advance__6TLobbyFv` conflict, and `ChallengeMode.cpp`:
`redefinition of pso_la_waived_TChallenge_size` — a duplicate waived assert
*name* across two headers, pre-existing under the pinned header, mechanism
tail item for the waiver-gen lane, not caused by this ask).

| TU | cluster row | other named KEEP-asserts @H3-before | after H1CA | after H3A |
|---|---|---|---|---|
| `src/pso/audio/SoundRequestVoice.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/boss/dragon/TBoss1Dragon.cpp` | 376 PsoCommandHandler_size | — | fail | NJS-4.2 |
| `src/pso/enemy/dd/TObjEneDD.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/enemy/illgill/TObjEneIllGill.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/item/item_create.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/network/GuildCardRecord.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/network/InfoBoardRequests.cpp` | 376 PsoCommandHandler_size | — | clean | clean |
| `src/pso/network/InfoBoardState.cpp` | 376 PsoCommandHandler_size | — | clean | clean |
| `src/pso/network/LobbyName.cpp` | 376 PsoCommandHandler_size | — | clean | clean |
| `src/pso/network/TGameProtocol.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/network/TLobby.cpp` | 376 PsoCommandHandler_size | EnemySetRecordSize,FloorObjectSetRow_size | fail | FloorPop-4.6+NJS-4.2 |
| `src/pso/network/TProtocol.cpp` | 376 PsoCommandHandler_size | — | fail | FloorPop-4.6 |
| `src/pso/network/TTcpSocket.cpp` | 376 PsoCommandHandler_size | PppFrame_size_check | fail | PppFrame-4.3 |
| `src/pso/network/TradeSession.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/network/protocol/choice_search.cpp` | 376 PsoCommandHandler_size | — | clean | clean |
| `src/pso/network/protocol/packet_classes.cpp` | 376 PsoCommandHandler_size | — | clean | clean |
| `src/pso/object/ambient/TObjGrass.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/object/base/TObjCollidable.cpp` | 376 PsoCommandHandler_size | EnemySetRecordSize,FloorObjectSetRow_size | fail | clean |
| `src/pso/object/container/TOContainerAncientEnemy.cpp` | 376 PsoCommandHandler_size | — | fail | NJS-4.2 |
| `src/pso/object/door/TODoorBase.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/object/door/TODoorCave01.cpp` | 376 PsoCommandHandler_size | EnemySetRecordSize,FloorObjectSetRow_size | fail | NJS-4.2 |
| `src/pso/object/door/TODoorCave02.cpp` | 376 PsoCommandHandler_size | — | fail | NJS-4.2 |
| `src/pso/object/door/TODoorCave03.cpp` | 376 PsoCommandHandler_size | — | fail | NJS-4.2 |
| `src/pso/object/door/TODoorMachine01.cpp` | 376 PsoCommandHandler_size | — | fail | NJS-4.2 |
| `src/pso/object/door/TODoorMachine02.cpp` | 376 PsoCommandHandler_size | — | fail | NJS-4.2 |
| `src/pso/object/key/TOKey.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/object/key/TOKeyCave01.cpp` | 376 PsoCommandHandler_size | — | fail | NJS-4.2 |
| `src/pso/object/key/TOKeyMachine01.cpp` | 376 PsoCommandHandler_size | — | fail | NJS-4.2 |
| `src/pso/object/lab/TOCapsuleLabo.cpp` | 376 PsoCommandHandler_size | — | fail | NJS-4.2 |
| `src/pso/object/ruins/TOCapsuleAncient01.cpp` | 376 PsoCommandHandler_size | — | fail | NJS-4.2 |
| `src/pso/object/ruins/TOMonumentAncient02.cpp` | 376 PsoCommandHandler_size | — | fail | NJS-4.2 |
| `src/pso/object/trap/TOMine.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/player/TObjPlayer.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/player/TPlayerBattleCtrl.cpp` | 376 PsoCommandHandler_size | EnemySetRecordSize,FloorObjectSetRow_size | fail | other |
| `src/pso/quest/QuestOpcodeHandlers.cpp` | 376 PsoCommandHandler_size | EnemySetRecordSize,FloorObjectSetRow_size | fail | other |
| `src/pso/quest/TQuestListOnline.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/quest/TQuestListVM_8007EAF8.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/quest/TQuestListVM_801F4398.cpp` | 376 PsoCommandHandler_size | — | clean | clean |
| `src/pso/quest/TQuestThread.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/quest/TQuestVmDownLoadServer.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/quest/challenge/ChallengeMode.cpp` | 376 PsoCommandHandler_size | — | fail | other |
| `src/pso/render/psoLoadTexture.cpp` | 376 PsoCommandHandler_size | — | fail | NJS-4.2 |
| `src/pso/system/scene/OfflineGameScene.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/system/scene/OnlineLobbyScene.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/ui/chat/ShortMail.cpp` | 376 PsoCommandHandler_size | — | clean | clean |
| `src/pso/ui/city/THunterGuildSeqOnline.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/ui/city/THunterGuildSeqOnlineTask.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/ui/city/THunterGuildWarning.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/ui/frontend/TAdSinglePlyChrSelectGC_801FBF54.cpp` | 376 PsoCommandHandler_size | — | fail | other |
| `src/pso/ui/frontend/TAdSinglePlyChrSelectGC_80202A68.cpp` | 376 PsoCommandHandler_size | — | clean | clean |
| `src/pso/ui/window/TMessageWindowController.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/ui/window/TWindowCtrl.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/ui/window/TWindowCtrlMain_800FC91C.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/ui/window/TWindowKeyBoardHard_801A529C.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/ui/window/TWindowMainMenu.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/ui/window/TWindowSubComCard.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/ui/window/TWindowSubComCard_8007A810.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/ui/window/TWindowSubInformationBoard.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/ui/window/TWindowSubInformationBoard_8034C224.cpp` | 376 PsoCommandHandler_size | — | clean | clean |
| `src/pso/unk/TLobbyAutoReply.cpp` | 376 PsoCommandHandler_size | — | clean | clean |
| `src/pso/unk/TLobby_8001D6A0.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/unk/TLobby_8007B858.cpp` | 376 PsoCommandHandler_size | — | fail | clean |
| `src/pso/unk/TLobby_801F3A24.cpp` | 376 PsoCommandHandler_size | EnemySetRecordSize,FloorObjectSetRow_size | fail | clean |
| `src/pso/unk/TLobby_802D3970.cpp` | 376 PsoCommandHandler_size | — | fail | clean |

## 4. Provenance / repro (scratch, `$TMP/pk/`)

`pk_probe.sh` (H1C/H3, fresh file per AZ pattern), `pk_probe_after.sh`
(H1CA/H3A), corpus loop `xargs -P6 -I{}`, logs `H1C|H3|H1CA|H3A/err/`,
membership `cluster_tus.txt`, census `census64.json`, ledger
`ledger.json`. Era-mirror header probe: C11 + C++17 TU, rc=0/0.
Inert receipt: `grep -rl compat/packets build host data Makefile*` = 0.

## 5. Paste-ready asks (integrator applies — lane never touches `data/` or `build/`)

**(a) `data/assert-verdicts.tsv`** — change the one existing row
(UNREVIEWED → RW; merges from this doc's §2 line naming the type
`RUNTIME-WAIVE` if `assert_policy.py` is re-run instead):

```
- PsoCommandHandler	1	1	UNREVIEWED	include/pso/network/protocol/packet_classes.h:376
+ PsoCommandHandler	1	1	RW	docs/p2-packets-1.md
```

**(b) `build/waivers/pso_assert_waivers.h`** (line 1255) — one flag flip,
or equivalently re-run `gen_waivers.py` after (a) and diff against scratch
`$TMP/pk/pso_assert_waivers_pk.h` (expected exactly this 1-line change):

```
- #define PSO_LA_FLAG_PsoCommandHandler_size 0
+ #define PSO_LA_FLAG_PsoCommandHandler_size 1
```

Then re-pin `host/bringup/waiver-header.sha256` (D8) — new sha is the
scratch header's `4cd64e7bddb8eab7…` if no other pending ask lands first.

**(c) No KEEP-flag additions, no TMenuList/union shape edits, no `-I`
additions required.** `compat/packets/**` is inert by design; adopting the
era-mirror is optional (fixtures/rung-4 only).

## 6. Post-adoption expectations + gates

- Post-fix official `parse-lp64`: expect **≥ +44 flips** at H3 stack shape
  attributable to this class (42 TUs are blocked by row 376 alone; 44 clean
  because 2 more resolve their remaining packet_classes-adjacent noise at
  full stack), and **≥ +10** at rebind-only; re-verify against the
  post-adoption build when the Makefile label fix lands.
- §6.1 cluster-zero gate unaffected (typedef-redefinition census stays 0).
- Watch item (not this lane): the 278 edge-B files carrying §4 classes
  include these 64 — after this flip, AR/AS2/net-C lanes own the residuals;
  ordering note: PK's flip unblocks counting for §4.2/§4.6 sweeps (rows
  double-listed here so siblings don't re-attribute them).
