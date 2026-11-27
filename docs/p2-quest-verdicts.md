# P2-AH — Quest-cluster verdict pass (resolutions-2)

Phase-2 task AH. Adjudicates the verdict-needed residue of AC's quest blocker map
([p2-quest-link-1](p2-quest-link-1.md), `data/quest-blockers.tsv` 487 sites) for
`QuestOpcodeHandlers.cpp` (`pso-decomp @ 6b38b5383`, read-only, zero builds).
Output: [`data/verdict-resolutions-2.tsv`](../data/verdict-resolutions-2.tsv)
(84 rows, same 5-col grammar as AD's `verdict-resolutions-1.tsv`; resolutions
override tranches downstream).

## 1. Work performed by AC's numbers

AC's projected residual (post I1+I2): **272 sites / 106 types**, of which **242
sites / 95 types** carried effective verdict UNRESOLVED (138 sites, tranche-reviewed
but open) or UNREVIEWED (104 sites, base-census rows never tranche-queued). By the
time this lane ran, AD's `verdict-resolutions-1.tsv` had already settled 11 of those
95 types / **24 sites** (10 RW, 14 LOCKED). This lane adjudicated the remaining
**84 types / 218 sites** — no open type was left undecided, and no row is
STILL-UNRESOLVED.

Assert-macro family split of the 218: **DRAGON_OFFSET 42 + the one direct
`LAYOUT_ASSERT(TBoss1Dragon_size)` in the same block = 43 sites / 1 type**;
direct **LAYOUT_ASSERT 176 sites / 83 types**. Per lane law the ASSERT TYPE gets
the verdict, not the macro name — DRAGON_OFFSET is just a `##`-paste offsetof
wrapper (`TBoss1Dragon.h:267-268`), the same family as T's NET_DISPATCH_OFFSET
finding.

## 2. Verdicts by family (sites cleared per RW verdict on the next gen_waivers regen)

| family | types | RW sites | LOCKED sites |
|---|---:|---:|---:|
| DRAGON_OFFSET (TBoss1Dragon) | 1 | 43 | 0 |
| LAYOUT_ASSERT (direct) | 83 | 134 | 41 |
| **AH total** | **84** | **177** | **41** |
| + AD resolutions-1 (overlap) | 11 | 10 | 14 |

Top collapses: `TBoss1Dragon` **RUNTIME-WAIVE** (HIGH) — 43 sites die on one verdict;
the DRAGON_OFFSET asserts pin member offsets of the live boss object (quest VM reads
`demo_flag`, `breath_origin` on the running instance), the header's casts are all
`((TBoss1Dragon*)owner)`-style runtime downcasts, and H-doc §"by-value embeds"
explicitly records **zero** wire embeds. `TObjPlayer` (30, HIGH VIRTUAL-CLASS —
231 `(TObjPlayer*)` hits in src are downcasts; the lone doc string-match is the
filename `TObjPlayer_801AF2F0.cpp`), `TItemMag` (8), boss/window/challenge/collision
runtime classes (VIRTUAL-CLASS precedent of AD's `TQuestListVM`/`TChallenge4P` rows),
POINTER-CLASS exclusions (`BmlResourceDescriptor` holds `const char*` compiled-in
descriptors; `TItemList` is a TObject link-node; `LobbyPoolEntry` mirrors AD's
same-header `LobbyFloorRows` precedent), and TObjPlayer sub-state blobs
(FIELDWISE-RUNTIME: `MovementHistory` = "TObjPlayer + 0x6AC" ring, etc.).

LOCKED-FORMAT (41 sites / 27 types) — fail-closed byte images, all evidence-pinned:
* Wire-census classes per **net-disc §1.2** (H-table): `G_IntraMapWarp_6x55`
  (subcmd_6x.h, 174 recs), the 4 `choice_search.h` types, the 5 `packet_classes.h`
  types (`PsoCommandHandler` is embedded **by value** in the `game_command` wire
  union), `TSendLogin`/`TSendVerifyLicense`/`TSendQuestStatistic` (TProtocol.h,
  `packet_header` + class `bswap()`), `TPlyMeetUserExtension`,
  `TPlyClientConfig(::Config)` (magic+proxy blob).
* Direct overlay casts: `ShortMailEntry` (`reinterpret_cast<ShortMailEntry*>
  (storage + i*sizeof)`, ShortMail.cpp:75), `FloorSetFlagsHeader` (comment names
  game-state packet 6x6E + `swap_floor_set_flags_header(buffer)`),
  `BattleParameterBank/File` (`(BattleParameterBank*)buf` fed by `load_disc_file`
  → `TGStream::load_file`), with `ActorParamBlock`/`BattleMovementParams` locked as
  in-image blocks (FAMILY-PRECEDENT, MED).
* Containment / precedent: `TLobbyFloorItemEntry` embeds FORMAT-KEEP `TItemData`
  by value (fails-closed per AD's ChallengeClientRecords logic); `NJS_OBJECT`/
  `NJS_MKEY_F` are the very anchor rows AD's NJS_* FAMILY-PRECEDENT verdicts cite —
  overlay classes settled by **p1-datamodel §3 rule-1 handle conversion, not a
  waiver** (tranche-1 note, kept verbatim).

Belt+braces audit: every RW claim word-boundary-grepped against
`p1-format-saves.md` (G) + `p1-format-net-disc.md` (H). Three RW types have doc
hits, all verified **prose, not table entries**: TBoss4DarkFalz ("the single hit is
a prose comment naming TBoss4DarkFalz as a packet *sender*" — the sentence that
*proves* non-embedding), TLobby (call-site `.cpp` mentions; the wire structs in
TLobby.h that ARE locked: `TLobbyFloorItemEntry`), SubtractiveRng (cipher-on-buffer
arithmetic prose). Earlier substring scans produced false containments
(`Config`, `TObjTombstoneRecord`) — word-boundary recheck killed them all.

## 3. Projected QuestOpcodeHandlers residual after this lane's merge

| step | sites |
|---|---:|
| AC projected residual (I1+I2) | 272 |
| − AD resolutions-1 RW types (10 sites) | 262 |
| − AH RW 57 types / 177 sites | **85** |

85 = **55 LOCKED-FORMAT assert sites** (41 AH + 14 AD, ~30 types — byte images,
NEVER waivable; decomp-side re-spell or ILP32-only gate, same treatment class as
AC's 30) **+ 30 source-rewrite sites** (deny-root asserts + the
`read_byte_advance__6TLobbyFv` sigdrift). **Exit stays 1: still far from parse.**
AH moves the file from "242 undecidable" to "85 priced, zero undecided" — the gap
to compile is now 100% format doctrine + decomp source edits, and per AD's
corollary no LOCKED type compiles until p1-datamodel §3 rule-1 (s32/u32 typedef)
lands. Unlocked-name math unchanged until then: 0 of the 519 defs.

## 4. The 104 UNREVIEWED sites — why they were never in the tranche census

Hypothesis tested (task brief): ILP32-era-only types needing no LP64 verdict.
**REFUTED — but the coverage story is real.** All 35 UNREVIEWED types DO fail at
LP64 (they are on this TU's 487-site probe; zero deltas). Cross-check vs tranche
presence: **0 of 35 appear in `verdict-tranche1-3.tsv`** (901 types total). The 829
failing-type census that ranked tranches 1–3 measured LP64 traffic over the P2-R
probe corpora, which did not compile QuestOpcodeHandlers' include closure — these
types' LP64 traffic lives (almost) exclusively there, so they never entered the
queue. They are not "no-verdict-needed" ghosts; they were a queue blind spot, and
this lane is their first LP64 adjudication (35 rows in resolutions-2, incl. the
43-site TBoss1Dragon and 8-site TItemMag heads of the list).

## 5. Next-lane asks

1. **Integrator stitch (follows AC §4):** overlay BOTH `verdict-resolutions-1.tsv`
   and `-2.tsv` into `assert-verdicts.tsv` verdict-col(3) and regen
   `build/gen_waivers.py`; quest-site waiver count must move 1435 → ≥1622
   (AC's merged 734 RW types + 57 AH + 8 AD quest-relevant RW types) — assert the
   RW-label count changed before trusting the regen (AC's col-index pitfall).
2. **Source lane (decomp):** re-spell the 55 LOCKED quest assert sites (this file's
   types + AD's EnemySetRecord/FloorObjectSetRow/G_FloorEvent set) alongside AC's
   30-site worklist; same ILP32-gate doctrine as phase-0.
3. **p1-datamodel §3 rule-1** remains the compile precondition for every LOCKED type.
4. **6x game-state audit (optional flips):** `PlayerTelepipe`/`PlayerPkState` were
   failed-closed LOCKED (class-level `bswap()` byte images synced through TLobby.cpp,
   MED) — if the packet audit shows field-wise packing they become RW, +2 sites.
5. U's 520-vs-519 `TQuestThreadBoard` attribution delta is still open (AC §6).
