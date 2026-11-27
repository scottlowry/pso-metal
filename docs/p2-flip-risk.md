# P2-AY — Flip-risk ledger closure (data/docs only, zero builds)

Closes AT's three escalations (`docs/p2-tail-verdicts.md` §4.1–4.3 @ ec781c4) against the
H/G membership oracle (`docs/p1-format-net-disc.md` §1.2–§1.3/§2, `docs/p1-format-saves.md` §2)
and AK's flag column (`data/census-audit.tsv` — read-only here; this lane produces a NEW file).
Ledger: [`data/flip-risk-ledger.tsv`](../data/flip-risk-ledger.tsv) — 70 rows + SUMMARY:
10 promotions + 47 HELD-RW + 1 packet_header gate + 12 named `packet_header` embedders
(3 cross-ref dupes → 67 unique types). Enum grammar:
`type|ask{AK-yes-promotion|wire-membership|leave}|evidence file:line|verdict-sustain{LOCKED|flip-to-RW|needs-decision}`.
Glossary: `flip-to-RW` = final recommended verdict is RUNTIME-WAIVE (confirm for HELD-RW rows,
demote if it ever were LOCKED); `lock-now` = LOCKED stands with no census-flag edit.

**Reconstruction honesty note:** AT's 47-row list lived only in `cache/scratch/at/` (pruned;
never committed — verified against all 3 doc versions ec781c4/1db871b/73231c4). Rule that
reproduces 47 exactly: resolutions-5 RW rows declared under `include/pso/network/**` (46)
+ `Ipv6PseudoHeader` (`src/pso/network/NetIpv6.c:714`, the only src-row with a wire-byte
image semantics). Remaining 10 `src/pso/network/*.c` RW rows are the candidate pool if AT's
47th differed — none is a §1.2/§1.3 member, so bucket outcomes would not change.

## 1. The 10 promotions (AT §4.1) — 7 promote, 2 no-promotion-keep-lock, 1 needs-decision

| type | census flag | membership line | → |
|---|---|---|---|
| CharacterFileSlot | probable | saves.md:64 ("7×CharacterFileSlot 0x2798") + CharacterFile.h:43 | **yes/LOCKED** |
| Pso3CharacterFileSlot | no | 80202A68.h:47, asserts :80-82 (record@0x41C = slot-clone geometry), saves.md:64 family | **yes/LOCKED** |
| Pso3CharacterFile | no | 80202A68.h:65-66 comment "PSO3_CHARACTER memory card file… CRC32", :72/:77 cipher seed, :82 0x194B0 | **yes/LOCKED** |
| gsl_archive_entries | no | net-disc.md:112 (H-D1: DMA'd whole TGStream.cpp:244, host-order reads :98) — silent-flip #1 | **yes/LOCKED** |
| TPlyJoinData | no | packet_classes.h:536; embeds packet_header :541 + TPlyCharData :542 + TPlyChallenge :543; bswap :550; census net-disc.md:42 | **yes/LOCKED** |
| G_SendGuildCard_6x06 | probable | net-disc.md:14 regime-A row (all `G_* 6x`); SendGuildCardPacket.h:11-12 embeds G_ClientIDHeader(protocol_types.h:48)+GuildCardEntry(saves.md:67); bswap/send_to :16-17 | **yes/LOCKED** |
| OnlineQuestMenu | probable | net-disc.md:42 "TMenuList<QuestListEntry/…>" recv class; TMenuList.h:36; OnlineQuestMenuRecords.h:8-10 | **yes/LOCKED** |
| DhcpMessage | probable | **NO LINE** — NetDhcp.h absent from §1.2/§1.3 enumerations (H audit gap, §3:152 searched tree but table skipped DHCP) | LOCKED via embeds: NetDhcp.h:36 embeds BootpPacket (r4 AK-pre-collision, asserts :29-33). ask=leave, no yes-flag |
| EndingWallTextureSet | no | **NO LINE** — H §2.2:123 puts Ninja runtime structs WAIVE-side | LOCKED via embeds: TEndingABTypes.h:7-8 NJS_TEXNAME[14]+NJS_TEXLIST (r3 OVERLAY-CAST BmlResource.cpp:427). ask=leave |
| FloorEventSet | probable | conflict: net-disc.md:166 "overlay is not pinned, walked by byte math" vs TSetEvtDestroy.h:20-26 typed overlay + :25 embeds SetDestroyEventRecord (r4 LOCKED) + AreaEventSet.cpp:96-99 direct field reads | **needs-decision** — missing: floor_event_sets fill site in AreaEventSet.cpp load path (bulk disc DMA of .evt vs typed build) |

## 2. The 47 HELD-RW vs §1.2 membership — 5 IN, 2 AMBIGUOUS, 40 OUT

**IN → propose LOCKED (promote to yes):**

| type | §1.2/§1.3 line | wire-byte proof |
|---|---|---|
| G_ModifyTradeProposal_6xA6 | net-disc.md:39 (subcmd_6x.h census) | subcmd_6x.h:703 embeds G_ClientIDHeader :704; bswap :711; prepare_and_send :714 |
| ChangeShipOrBlockPacket | net-disc.md:44 (TProtocol.h ∈ 48-row) | TProtocol.h:179 packet_header embed, :180 TPlyGuildCardTag(gc_tag, saves.md:96), bswap :183 |
| CharacterSaveInfoPacket | net-disc.md:44 | TProtocol.h:168 packet_header embed, bswap :172 |
| Ipv6NdMtuOption | regime-B family net-disc.md:15, :71-74 | NetIPv6NeighborDiscovery.c:954-957: 8 raw wire bytes copied INTO struct, then `mtu_opt.mtu` host-order load |
| Ipv6NdPrefixInfoOption | regime-B family :15, :71-74 | NetIPv6NeighborDiscovery.c:939-945: 0x20 wire bytes copied in, lifetime/prefix_len fields read |

**AMBIGUOUS → needs-decision (missing evidence named):** `G_SyncItemState_6x6D_Decompressed`
— declared in §1.2 census file (subcmd_6x.h:1201) but zero header/bswap/embed: it is the
*decompressed* working view; the wire image is the compressed blob (G_SyncGameStateHeader…
:1195-1199). Missing: 6x6D send-side PRS compress input site (is this layout compressed back
onto the wire?). `NetIcmpv6Redirect` — NetIPv6NeighborDiscovery.c:414 local + parse target
:464; missing: whether ParseRedirectOptions/HandleRedirect copies wire words vs per-field
assign (the RA case at :939/:954 shows the copy-into-struct idiom is live in this file).

**OUT → RW stands (absence cited per row):** 9×NetIpv6Route + 6×Ipv6Nd event/state + 4×NetworkDispatch
→ :77 "NetRuntime/socket tables — pure in-process state machines"; 3×NetTcpState (TcpSeqRange/TcpWait
**named** at :77; TcpState family :79); 5×Lcp **named** :78 "PppFsm/Lcp state"; 4×Ipcp same family +
:104-105 byte-serial; 3×PppVj + PppPap :104-105 byte-serial; 2×ModemSession **named** :78;
TPlyOnCreated (vptr — cannot overlay bytes); trade_proposal_state_handlers (compiled table, :162);
Ipv6PseudoHeader (NetIpv6.c:717 local-built, never sent — its regime-B checksum hazard is an
accessor-layer fix per §5.2, NOT a struct-format binding; noted for the stack lane).
AL's correction law held throughout: none of the 40 has a §1.2 file definition, an enumerated
:69-70/:76-78 FORMAT-KEEP line, or a wire-copy site.

## 3. packet_header gate — DECISION: **LOCKED (FORMAT-KEEP), promote to census `yes`**

AT §4.3 deferred; the evidence is not close:
- The struct image is copied **to the wire verbatim**: `send((u8 *)&header, sizeof(header))`,
  `src/pso/network/TProtocol.cpp:27-34` (keepalive) — nothing else is "the packet"; every
  regime-A send is header-bytes then payload-bytes.
- Layout pinned by asserts: `TProtocol.cpp:23-25` (size 4, flags@1, size@2) vs `protocol.h:14-19`.
- Multi-byte field goes through the wire converter unconditionally: `packet_header::bswap()` =
  `bswap_16(&size)` `TProtocol.cpp:1891-1893`.
- H's own verdict table already names it: `docs/p1-format-net-disc.md:179` ("…incl. packet_header…
  FORMAT-KEEP"), census rows :14, :25-28.
packet_header has **0 census sites** (it never fails a layout assert — u8/u8/u16 packs identically);
the flag change is a *correctness-of-oracle* edit, not a waiver delta. Propagation consequence =
§1.2 embedders block: of the 12 corpus-named embedders, 2 flip RW→LOCKED (ChangeShipOrBlockPacket,
CharacterSaveInfoPacket — also §1.2 members independently), 10 confirm (already LOCKED:
ChoiceSearchPacket csearch.h:12, ChoiceSearchResultList :64, TPlyJoinLobby pc.h:240,
TRegister :287, TSendAction :415, TSendLogin TP.h:95, TSendPsoRegist pc.h:304,
TSendQuestStatistic TP.h:221, TSendVerifyLicense TP.h:75, TPlyJoinData pc.h:541).
**Honest delta vs AT's "~12 tail packets flip in one step": the tail contains exactly 2
packet_header embedders (grep of all headers cross-joined to resolutions-5); AT's ~12 matches
the 12 *named* embedders (10 already LOCKED + the 2 flips). The other ~30 embedder classes
(TProtocol.h:43-242, packet_classes.h remainder, TMenuList.h) are unnamed in the 1,602-type
corpus — membership-oracle coverage only, no rows to flip.**

## 4. Bucket totals (unique types, precedence: needs > promote > lock > rw)

`promote-to-yes: 13 · lock-now: 11 · rw-confirmed: 40 · needs-decision: 3` (rows=70+SUMMARY, unique=67)

## 5. INTEGRATOR ACTION (paste-ready consequences — honest deltas only)

1. **Committed waiver header DOES need regen** at next stitch. The 12 newly-LOCKED types
   (7 promos + 5 membership-IN; exclusions: packet_header has no assert) own **17 census
   sites** (sum of sites col: CharacterFileSlot 2, Pso3Slot 2, Pso3File 1, gsl 1, TPlyJoinData 2,
   SendGuildCard 1, OnlineQuestMenu 2, G_ModifyTradeProposal 2, ChangeShip 1, CharSaveInfo 1,
   MtuOption 1, PrefixInfoOption 1). These sites were waived by AT's RW rows → they become
   non-waivable keeper sites: expected post-regen wall ≈ **2,806 − 17 = 2,789 waived of 3,333**
   (apply-2 basis), i.e. AT's "17 LOCKED rows add 24 keeper sites" projection is superseded
   by 12 of those rows flipping back to keeper + their sites. `gen_waivers` re-run + battery
   re-pin is integrator work (lane cannot run make).
2. **Expected flips, files-wise:** units whose only layout blockers were the 12 flipped types
   re-enter the loader-lane queue; AT's "125 units pure-layout-clean" (10 src-TU + 115 header)
   is an upper bound — the header-unit subset touching `TProtocol.h`, `packet_classes.h`,
   `subcmd_6x.h`, `NetIPv6NeighborDiscovery.h`, `NetDhcp.h`, `SendGuildCardPacket.h` loses
   clean status. Do not re-quote 150+125 until the battery re-runs (AT's own caveat stands).
3. **Census edits (AK next regen):** add `packet_header` row (sites=0, yes); flip 12 rows
   no/probable → yes (§1 col); leave `DhcpMessage`, `EndingWallTextureSet` flags — their LOCK
   is embeds-law, not table membership.
4. **Doc corrections owed to H/G authors (non-blocking):** G §2 table misses PSO3_CHARACTER
   as a 4th card file (this ledger promotes it on :65-66 comment + geometry); H §1.3 misses
   DHCP/BOOTP and the ND option records that the copy-into-struct idiom makes frame images.
5. **AS2/AX site-math cross-check (6,321-site shift):** unaffected. ChallengeRecord stays
   LOCKED via G:66 (AX resolutions-6); none of the 67 ledger types is ChallengeRecord nor a
   co-firing site-owner of its 903-file bucket. TPlyJoinData's yes-promotion *agrees with*
   AX: it embeds TPlyChallenge (r2 LOCKED). No AW (compat/repin) interaction — no repin
   math changes since packet_header has 0 assert sites.
6. **Loader-lane sequencing:** gsl_archive_entries LOCK is conditional-long-term — when the
   §5.3 swap-on-load pass (`open_gsl_archive` fixup over D1's s32 trio) lands, the RAM table
   legitimately becomes RW per H §2.2:124; same for ND options once regime-B BE accessors
   wrap the copy sites (NetIPv6NeighborDiscovery.c:939/:954). Do NOT waive before the pass
   exists: today it is silent-flip site #1.

Scratch: `cache/scratch/ay_index.json` (314+ §1.2 definition-line index), `ay47.json`
(embed/bswap probes). No build//host//make; no census-audit edit; no -A/amend/push.
