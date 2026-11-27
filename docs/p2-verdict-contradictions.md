# P2-AX — Verdict-contradiction pass (resolutions-6): closing AS2's roster

Phase-2 lane AX, 2026-11-27. Data-only: zero builds, no `make/ninja`, no `build/` or
`host/` writes, decomp `pso-decomp @ 6b38b5383` READ-ONLY. Input = AS2's surprises
roster ([`p2-residual-census`](p2-residual-census.md) §6 +
[`data/residual-census.tsv`](../data/residual-census.tsv)) recomputed from AS2's own
per-file class roster (`cache/scratch/as2_rows.json`, all 1,519 fail-files, detail =
`(site, class, type)` triples) so every count below is checkable against the same data
AS2 published. Output: [`data/verdict-resolutions-6.tsv`](../data/verdict-resolutions-6.tsv)
— 78 rows, same 5-col grammar `type|new-verdict|evidence-type|file:line|confidence`,
consume order 1>2>3>4>5>6 (later RUNTIME-WAIVE beats earlier keeper; deny-roots
immune, per `gen_waivers.read_verdicts` precedence law).

## 1. HEADLINE — ChallengeRecord is LOCKED-FORMAT (HIGH); the 903-file bucket is re-pin-owned, not parse-eligible

**The contradiction, fully rooted.** Three lenses labeled `ChallengeRecord` three ways:

* `data/assert-verdicts.tsv:43` — **UNREVIEWED** (base row, stale since the capped era).
* `data/verdict-tranche1.tsv:11` — **LOCKED-FORMAT**, justified "named in G/H FORMAT
  table or declared in a wire/disc-image header".
* AC ([`p2-quest-link-1`](p2-quest-link-1.md) §3) computed the **effective** verdict via
  `gen_waivers.read_verdicts`, which merges tranche rows — tranche1's LOCKED-FORMAT wins
  (keepers are collected from ALL present files) — so AC saw LOCKED-FORMAT and put
  `ChallengeRecord.h :43-49` on the decomp-side worklist (§5).
* AS2's classifier (`as2_census.py:20-31`) overlaid only **base ∪ resolutions-1..5 ∪
  deny-roots — never the tranche files** — so `ChallengeRecord ∉ LOCKED(AS2)` (verified:
  `as2_rows.json["LOCKED"]`, 78 names, contains no ChallengeRecord) → 6,321 sites across
  903 files bucketed `verdict-open`.

No bytes ever disagreed: the committed era header carries
`PSO_LA_FLAG_ChallengeRecord_* = 0` (KEEP) for all 7 names
(`build/waivers/pso_assert_waivers.h:210-216`) — identical behavior under either label,
since UNREVIEWED and LOCKED-FORMAT are both non-waivable. The disagreement was pure
**queue accounting**: AS2's "verdict queue NOT done, 75 types" partially consisted of
types the generator had already fail-closed as keepers.

**Adjudication (decisive evidence).** ChallengeRecord is a save-image byte record:
G pins it by-value at a fixed 0x100 slot inside `CharacterSaveRecord` —
`docs/p1-format-saves.md:66`: "CharacterSaveRecord (quest flags 4×0x100 … ChatShortcut[20],
BattleRecord×2, **ChallengeRecord 0x100** …)" with buffer citation
`include/pso/system/CharacterSaveRecordLookup.h:45-101`; `CharacterSaveRecord` is itself a
`DENY_FORMAT_ROOTS` member (`build/gen_waivers.py:57`). The class carries its own
`void bswap();` (`include/pso/quest/challenge/ChallengeRecord.h:41`) — the byte-image
marker used throughout H/G doctrine — and its 7 asserts pin the 0x100 image (size +
member offsets 0x60/0xB0/0xC0/0xD8/0xE4/0xEC, `:43-49`). Containment precedent is AD's
`ChallengeClientRecords` (`verdict-resolutions-1.tsv:5`, FORMAT-TABLE-G same line 66) and
AH's `TLobbyFloorItemEntry` ("embeds by value → fails closed", `p2-quest-verdicts.md` §2).
G's own honesty line — "Missing coverage to write: … BattleRecord, ChatShortcut.kind,
**ChallengeRecord** …" (`p1-format-saves.md:158-159`) — says the walk is unwritten, not
the containment doubted; the containment row + bswap + offsets are the yes-pin.
**Verdict: LOCKED-FORMAT, HIGH, FORMAT-TABLE-G `docs/p1-format-saves.md:66`.** AC and
tranche1 were right; the base row is the stale one; AS2's bucket was the drift.

**What it means for the 903-file bucket.** Those 6,321 ChallengeRecord sites are NOT
queue-clearable: no verdict, waiver, or allow-list touch makes a deny-root-contained
byte image compile at LP64. The 903 files are **re-pin-owned** (SOURCE-REWRITE-ASK:
decomp-side re-spell / ILP32-only gate of `ChallengeRecord.h:43-49`, per AC §5 and
p1-datamodel §3 rule-1 doctrine) — say plainly: **not parse-eligible from the queue**.

**What it means for AW's both-ways numbers.** Move 6,321 sites (and any co-firing file
credit) OUT of the "verdict queue clears it" side and ONTO the structural re-pin side;
ChallengeRecord joins the §6.6 re-pin pricing roots as the third giant:
`TItemData` keeps in 1,106 files, `ChallengeRecord` in **903** (confirmed exact),
`SystemSaveData` in 236. AS2's own "adjudicate these ~10 types doesn't green files"
instinct was right, but for a stronger reason: the top blocker was never in the queue.

Also co-drifted (tranche1 LOCKED-FORMAT vs base UNREVIEWED, same mechanism, all KEEP
flags 0 in the header): **BattleRecord** (`CharacterSaveRecord` contains `BattleRecord×2`,
G:66 — LOCKED-FORMAT HIGH), **ChatShortcut** (`ChatShortcut[20]` in same row — LOCKED,
HIGH), **TRegister** (H names it directly: `p1-format-net-disc.md:42` "`packet_classes.h`
| 10 | recv classes: `TRegister`, …", wire recv class — LOCKED-FORMAT HIGH),
**NetworkInfoConnectionConfig** (H `:116` D5–D10 "ConnectionConfig(0x210) … offsets
pinned — treat as blob image" — LOCKED-FORMAT HIGH). Two more wire-shape LOCKED pins from
the H raw-stack family table (`:180` "TcpHdr, IPv4Header, EthernetFrameHeader+ARP/IPv6
(10 structs) FORMAT-KEEP", scope names `IPv6.h` at `:68-69`): **Ipv6Header** and
**Ipv6FragmentHeader** (pointer-free frame headers, IPv6.h:144/:152) — MED (family-row
pin, membership by named file, name-level yes-pin within §1.2 scope).

## 2. The 75 keep-firers — how they escaped, and the judgment

**Recount.** AS2 said 75 types / 21,236 sites; recomputing from `as2_rows.json` gives
76 distinct attributed names / 21,234 sites. The extra name is **`'struct'` (50 sites,
50 files)** — an anonymous-struct mis-attribution of the same family as AK's `'?'` and
X's `structPppFrame` regex artifacts, not a type; it gets no row, and the honest
"75 types" = my 75 named types (6,321+…+2). Every one of them exists in base
`assert-verdicts.tsv` as **UNREVIEWED** and none appears in `data/census-audit.tsv`.

**Escape mechanism (verified, replaces the "because…" hypothesis).** AT's worklist was
census-audit's 667 BLIND rows (`p2-tail-verdicts.md` §1), and S defined
`BLIND = uncapped-failing − named` (`p2-census-audit.md:25`) where **presence in
tranche1–3 counted as "named"** — the audit doc even flags the trap on its own behalf:
"named via tranche1 + resolutions-2 rows and both still fail uncapped" (`:114`) and
"family, `Bml*`, challenge/quest tables — treat as fail-closed UNRESOLVED" (`:98`).
Tranche1 gave these 75 types **UNRESOLVED placeholders (71) or LOCKED labels (4)** — a
row, but no verdict. So: in base (UNREVIEWED), named-in-tranche (so never BLIND, so
never AT's queue), never in resolutions-1..5 (never quest/census-audit-owned) —
"UNRESOLVED ≠ adjudicated" plus "present-in-tranche = named" is the complete escape
story. AC's §3 line "UNREVIEWED … tranches 4+ or targeted review" is the queue item
that was never run. AT's "tail 100% of 1,602" is true of the 1,602 AK-uncapped universe
AT was given; these 75 sat inside the 935 pre-named (tranche-UNRESOLVED) portion.

**Grammar applied (S/AH/AL/AT law).** LOCKED-FORMAT requires a G/H name-level yes-pin
(AL rule correction: "probable must not be pre-pinned as LOCKED — only G/H yes",
`p2-blind-verdicts.md:53`; AT kept 47 wire-adjacent RW under the same law). Everything
without a pin gets RUNTIME-WAIVE on the structural evidence class read from the decomp
header bodies (member-scan: `virtual` → VIRTUAL-CLASS; pointer members →
POINTER-CLASS; scalar-only runtime state → FIELDWISE-RUNTIME), exactly the
resolutions-5 precedent for the same headers (e.g. `NetIpv6Route.h` cluster, `PppFsm.h`,
`IPv6.h` all RW'd there or by AD/AH for sibling lines).

**Split: 7 LOCKED / 68 RUNTIME-WAIVE / 0 open.** Per-family adjudication:

* **Virtual runtime classes (12) — VIRTUAL-CLASS HIGH**: `TObjNpcBase`, `TObjNpcEnemy`,
  `TObjNpcEnemyEntry`(POINTER — table node, no vptr; but members hold pointers),
  `TBoss8Dragon` (:313, 51 pointer members, `class TBoss8Dragon : public TObjEneDD_ST`),
  `TOBoss4`, `TOBoss4Rock`, `TObjEneRe7Berura`, `TObjEneRe7BeruraPart`, `TItemWeapon`,
  `TWindowChatSymbol`, `TWindowSubSymbolChatPartsSet`, `TWindowSubComCardList`,
  `TQuestListOffline`, `TPlyOnCountOver`. Precedent: AD `TQuestListVM`/`TChallenge4P`,
  AH `TObjPlayer` ("231 `(TObjPlayer*)` hits are downcasts"). The `TPly*` name smells wire
  (`TPlyMeetUserExtension` is H-locked) but `TPlyOnCountOver` appears in NO G/H row —
  probable ≠ pinned → RW VIRTUAL-CLASS (it is `class TPlyOnCountOver : public TObject`
  with 8 pointers, `TPlyOnCountOver.h:20` — a live object, not the payload it sounds like).
  `TItemWeapon` flagged **MED with an AW note**: it embeds deny-root `TItemData`
  (`TItemWeapon.h:176` body contains a by-value TItemData member) — the containment law
  is what locked `ChallengeClientRecords`; here the *persisted* image is the member's
  own (its asserts stay KEEP as a deny-root), while the surrounding runtime weapon
  object (25 pointers, vptr) is host-layout-free, same as AH's hold-RW calls for
  wire-adjacent classes. If the 6x game-state audit shows whole-record persistence,
  this row flips LOCKED (one of only two possible flips here).
* **Net-SDK runtime state (44) — POINTER-CLASS HIGH**: the Dns*/Arp*/Eth*/Ipv4*/Ipv6*
  (state-node)/Ppp*/Net* clusters of `NetDns.h:155-163`, `NetArp.h:98-102`,
  `NetEthernet.h:64-68`, `NetIpv4.h:22-57`, `IPv6.h:37-91,166-167`, `IPv6Send.h:28-48`,
  `NetStackTypes.h`, `PppFsm.h:80-81`, `UdpRuntime.h:52-59`, `BufferRef.h`,
  `NetworkReceiveCallback.h`, `Csn*` pointer-carrying geometry/query types. H §4
  `:181` states the regime outright for the named siblings: "**RUNTIME-WAIVE** — host
  layout free; keep asserts host-recomputed". The BCN/Csn and SDK-stack structs are the
  same re-spelled runtime models (every one carries pointer members except those
  listed FIELDWISE). `BmlResource`/`BmlModelEntry`/`BmlMotionEntry` (3,304+1,239+826
  sites, 413 files each) are the parsed-archive runtime wrapper (17/2/1 pointer
  members, `BmlResource.h:16-67`); the actual BML *disc image* is `BmlArchiveHeader`
  (overlay-cast LOCKED, `reinterpret_cast<BmlArchiveHeader *>(bml_load_buffer)`,
  `BmlResource.cpp:172`, AL confirm-pin `p2-blind-verdicts.md:32`) — wrapper RW, image
  LOCKED, exactly AJ's NJS-in-Bml overlay pattern (`verdict-resolutions-3.tsv:2,5-6`).
  AP's "resolutions missed BmlModelEntry" (`p2-rung4-contract.md:45`) is hereby closed.
* **Scalar-only runtime state (12) — FIELDWISE-RUNTIME MED**: `CsnContact`,
  `UdpDatagram`, `Ipv6cpOptionSet/Options/Config`, `NetThreadState`, `NetPacket`,
  `NetIPv6SendOptions`, `UdpSendOptions`, `DnsPtr`, `Ipv4Route`, `Ipv6RawSocket`, and —
  loudly — **`Icmp4Header`** (`NetStack.h:150`): wire-shaped, but H `:180` names only
  "TcpHdr, IPv4Header, EthernetFrameHeader+ARP/IPv6"; ICMP is outside every named scope,
  so the correction law holds it RW (second possible 6x-audit flip).
* **7 LOCKED** — the tranche-drift quintet + 2 IPv6 frame headers (Section 1).

**Honest materialization.** 68 RW rows × regen clears **14,635 of 21,234** open sites.
Files whose ONLY remaining layout blocker was a non-LOCKED open type: **6 flip to
green-eligible candidates** on the next era re-record (plus ~158 more where the queue
segment vanishes behind allow-list/LOCKED co-gates: 78 AL-only + 69 AL+LOCKED + 12
other combos). Nothing here contradicts AS2's §2 ownership table — the 86 verdict-open
"owning" files mostly carry ChallengeRecord/BattleRecord keeps that are LOCKED-cogate
material under the corrected labels.

## 3. The 23-24 "RW (RUNTIME-WAIVE)" artifact rows — root cause is NOT the label

**AS2 §6.4 mechanism is refuted by the generator source.** `build/gen_waivers.py:224`
collects waivers on **substring** `if "RUNTIME-WAIVE" in verdict:` — "RW (RUNTIME-WAIVE)"
*does* match, so the 31 odd-labeled base rows were waived, not keep-flagged. Proof in
the committed header: `TcpTcb_size_check` (base `:1630` = odd label) → flag `1` WAIVED
(`pso_assert_waivers.h:3167`), while `MovieState_*`, `SoundReverb*`, `TSaveChCr*` all
carry flag 1 too. Semantics confirmed in `gen_waivers.py` TEMPLATE: `0 = KEEP / assert
honest, 1 = WAIVED`.

**Recount of the bucket** (AS2 said 23 sites/8 names): `as2_rows.json` detail carries
**24 sites / 3 types**, and the 2 waiver-label-gap-OwNING files are
`src/pso/boss/olgaflow/TBoss6Demo11_TYPE1_DEAD.cpp` and `TBoss6Type1Actions.cpp` — not
"TBoss6Type1.cpp, NetTcpState consumers". The three real sub-causes:

1. **`TcpTcb` × 8 files** — fire site `NetTcpState.h:160` is
   `LAYOUT_ASSERT(TcpTcb_f204_check, TCPTCB_F204_BYTE == 0x204)`; the offsetof lives
   inside the `#define` (`:159`), invisible to `gen_waivers` type extraction →
   fail-closed KEEP (`flag 0`, header `:3166`). The *type* was waived. T's
   NET_DISPATCH_OFFSET lesson, third instance. **Correct owner: SOURCE-REWRITE-ASK**
   (decomp re-spell or macro-aware extraction), not the queue.
2. **`TBoss6Type1` × 14 files** — site `TBoss6Type1.h:342`,
   `(offsetof(TBoss6Type1,arm_node_positions) + 5 * sizeof(Vec) == 0x638)`: the compound
   arith condition mis-captures the type in gen's extraction (sibling `:341`
   `TBoss6Type1_receiver_offset` with a plain condition IS waived, header `:1823/1826`);
   base `:456` already carries the honest label `N/A-CONSTANT` for this exact name.
   KEEP is BY DESIGN for const-arithmetic asserts.
3. **`CaveRockArchive<4>/<7>` × 2 sites** (`TORockCave.cpp:74-75`) — template-instance
   names: base rows are `CaveRockArchive<4>`/`<7>` (UNREVIEWED, `:495-496`), while AL/AT
   keyed the bare `CaveRockArchive` (`resolutions-5:485`) — exact-string waiver lookup
   never sees the instance. Genuine data gap → fixed with two res-6 instance rows
   (POINTER-CLASS HIGH: `const char* model_filenames[N]` + `BmlResourceDescriptor`,
   `TORockCave.cpp:59-62` — parsed-archive runtime table).

**Chosen normalization spec (primary = generator-side one-liner; secondary = the 3
res-6 rows above).** Justification: the label defect lives in base
`assert-verdicts.tsv` (integrator-owned, overlay-only for us), behavior is already
correct, and only *future censuses/lenses* are hurt; a label normalization is the
minimal fix and is behavior-preserving by construction (substring already matched).
Paste-ready integrator line:

```bash
awk -F'\t' 'BEGIN{OFS="\t"} NR==1 || $4=="RW (RUNTIME-WAIVE)"{$4="RUNTIME-WAIVE"} 1' \
  data/assert-verdicts.tsv > data/assert-verdicts.tsv.n && \
  mv data/assert-verdicts.tsv.n data/assert-verdicts.tsv   # 31 rows; waived-set unchanged
```

(Alternative rejected: widening `gen_waivers.py` matching — moot, it already matches;
it is the AS2-side "exact match" belief that was wrong. If R prefers belt+braces, an
equal set of 31 res-6 canonical rows can be emitted, but they would be pure duplicates
of rows 42/13-style precedent lines.) The `(const)`/`N/A-CONSTANT` (8) and
`NOT-AT-FAULT` (123) families are *keep-by-design* labels and must NOT be normalized
into the waiver channel.

## 4. 16 ghost units — all harness-path-rot, zero real types, zero res-6 rows

Every `src/pso/unk/TLobby_*`/`text_*` "missing unit" was **renamed or re-cut in-tree**,
most by pso-decomp `c056b38ac` "Give 16 unknown units real names" (2026-11-26, which
also updated decomp's OWN lists `config/GPOE8P/psov3/linked_units.txt`+`splits.txt`);
the pso-metal battery unit list was never re-rolled. None is a corpus gap; none is a
type; nothing joins resolutions-6. First-seen names added in the `f52fa585a` naming
pass; five ghosts carry re-cut lineages (`b2d3a7fc4`, `5a5f87f54`, `3b61e37a8`,
`9575dbb7c`, `62a7ebcd6`).

| ghost unit (battery list) | classification | in-tree successor / evidence |
|---|---|---|
| `src/pso/unk/TLobby_80226C3C.cpp` | path-rot (rename) | `src/pso/audio/MusicControl.cpp` @ `c056b38ac` |
| `src/pso/unk/TLobby_8023F734.cpp` | path-rot (rename) | `src/pso/audio/SoundControlState.cpp` @ `c056b38ac` |
| `src/pso/unk/TLobby_80227AA0.cpp` | path-rot (rename) | `src/pso/audio/SoundPlayback.cpp` @ `c056b38ac` (earlier cut `3b61e37a8`) |
| `src/pso/unk/TLobby_80226070.cpp` | path-rot (rename) | `src/pso/audio/SoundRequests.cpp` @ `c056b38ac` |
| `src/pso/unk/text_8026CB5C.cpp` | path-rot (rename) | `src/pso/math/Vec2Constants.cpp` @ `c056b38ac` |
| `src/pso/unk/text_80072230.cpp` | path-rot (rename) | `src/pso/network/ChallengePackets.cpp` @ `c056b38ac` |
| `src/pso/unk/text_803F3BF8.cpp` | path-rot (rename) | `src/pso/network/NetworkConfig.cpp` @ `c056b38ac` (earlier split `62a7ebcd6`) |
| `src/pso/unk/text_801F3774.cpp` | path-rot (rename) | `src/pso/quest/QuestFlagSync.cpp` @ `c056b38ac` |
| `src/pso/unk/text_802D1C84.cpp` | path-rot (rename) | `src/pso/render/MericarolTexture.cpp` @ `c056b38ac` |
| `src/pso/unk/text_80320358.cpp` | path-rot (rename) | `src/pso/system/RelModule.cpp` @ `c056b38ac` |
| `src/pso/unk/text_803F1804.cpp` | path-rot (rename) | `src/pso/system/SiPolling.cpp` @ `c056b38ac` |
| `src/pso/unk/TLobby_80131794.cpp` | path-rot (re-cut/fold) | removed by pool re-cut chain (`f52fa585a` add) |
| `src/pso/unk/TLobby_8034C23C.cpp` | path-rot (re-cut/fold) | `f52fa585a` naming-pass lineage |
| `src/pso/unk/TLobby_8034C18C.cpp` | path-rot (re-cut/fold) | `f52fa585a` naming-pass lineage |
| `src/pso/unk/TLobby_80335E7C.cpp` | path-rot (re-cut/fold) | split chain `5a5f87f54`; header folded `c056b38ac` |
| `src/pso/unk/text_803D647C.cpp` | path-rot (re-cut/fold) | split chain `9575dbb7c` |

Ask (UPSTREAM, one-line relist, AS2 §0 already flagged): re-roll pso-metal's battery
corpus from decomp `config/GPOE8P/psov3/linked_units.txt` at or after `c056b38ac`;
the 16 units are NOT new parse wall (they re-appear as real files at new paths).

## 5. Closure math (honest)

* Named-verdict universe before AX: **1,602** (AT §5: 935 pre-tail + 101 AL + 566 AT).
* Newly adjudicated by AX: **75 keep-firing types** (7 LOCKED-FORMAT / 68 RUNTIME-WAIVE)
  **+ 3 more named rows** (TcpTcb canonical, `CaveRockArchive<4>`,
  `CaveRockArchive<7>`) = 78 res-6 rows total.
* **Corpus named beyond AS2's gaps: 1,602 + 77 = 1,679 type-names with effective
  verdicts** (the 77 = 75 keep-firers + 2 template-instance names; TcpTcb was already
  counted in the 1,602 via its base row — its res-6 line canonicalizes, it does not add).
* Residual open, honestly: **0 battery-firing UNREVIEWED types remain** after this
  overlay (recheck: every AS2 verdict-open name has a res-6 row except `'struct'`);
  still open by construction: the `'struct'` mis-attribution artifact (50 sites,
  attribution-side not verdict-side — same family as AK's `'?'`), the 5 `Icmp4Header`-
  class wire-shape holds awaiting the 6x packet audit, `TItemWeapon` possible flip, and
  AH §5.4's `PlayerTelepipe`/`PlayerPkState` pair. Deny-root/LOCKED work is unchanged:
  SOURCE-REWRITE-ASK stays ~1,144 owned files — ChallengeRecord's 903 re-classified sites
  confirm rather than grow it (they were already KEEP).
* Recount deltas vs AS2 text, for the record: open sites 21,234 (not 21,236); gap bucket
  24 sites/3 types (not 23/8); gap-owning files as tabled in §3.

## 6. Next-lane asks

1. **Integrator**: overlay resolutions-6 (it beats tranches: consume order), run
   `gen_waivers`, assert waived-type count grows ≈ +68 and the 6 pure-queue files
   re-classify before trusting; apply §3 normalization line to the base tsv in the same
   stitch commit (zero waiver-behavior change).
2. **AW**: re-pin both-ways math per §1 — ChallengeRecord's 6,321 sites/903 files are
   structural; the 68 RW types are now priced queue, de-risking every re-pin estimate.
3. **Decomp source lane**: `ChallengeRecord.h:43-49`, `NetTcpState.h:159-160`
   macro-transparency, `TBoss6Type1.h:342` const-arithmetic re-spell (AC §5 worklist trio).
4. **Corpus re-pin** (§4 relist) before battery-3; retire `as2_census.py`'s
   resolutions-only LOCKED overlay — include tranche files, or AS2 §6.2 recurs.
