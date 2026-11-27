# P2-AT — Blind-tail verdicts (resolutions-5)

Lane AT tail adjudication of the AK census blind set, `pso-decomp @ 6b38b5383` READ-ONLY,
zero builds (generator not re-run; flag-state projection simulated read-only per §5).
Verdict table: [`data/verdict-resolutions-5.tsv`](../data/verdict-resolutions-5.tsv) —
same 5-col grammar, consume order 1>2>3>4>5 (later RUNTIME-WAIVE beats earlier keeper
per the NJS_POINT3COL precedence note in parse_gate).

## 1. Worklist

Queue = `data/census-audit.tsv` (667 BLIND rows) minus resolutions-4 (AL's 101 priority
types / 507 sites) = **566 types / 623 sites** — exactly the remainder AK sized
(1130 blind − 507 AL = 623 ✓; 667 − 101 = 566 ✓). Grouped by declaring header into ~40
batches across 95 distinct dirs; families collapsed where one header declares siblings
##-paste (38 multi-type headers / 101 types — NetIpv6Route.h 9, NetIPv6NeighborDiscovery.h 9,
PppLcp.h 5, NetworkDispatch.h 4, PppIpcp.h 4, PppVj.h 3, NetTcpState.h 3,
QuestDownloadCacheTypes.h 3; dominant dirs ui/window 49, npc 46, network 45, object 27,
technique 20 — the VIRTUAL-CLASS scene family continues AL's collapse pattern).

## 2. Verdicts

**566 rows: 549 RUNTIME-WAIVE / 17 LOCKED-FORMAT / 0 open.** 507 HIGH / 59 MED confidence.

Evidence split: VIRTUAL-CLASS 454 (vptr self-or-transitive-base — the tail is overwhelmingly
TObj*/TWindow*/TTec*/TBoss* scene classes), FIELDWISE-RUNTIME 51 (pure scalar fields; lean
RW at MED per the S/X default — no format pin, incl. the PPP/LCP/IPv6-ND control blocks),
POINTER-CLASS 42, FAMILY-PRECEDENT 8 + FORMAT-TABLE-H 5 + AK-pre-collision 3 +
FORMAT-TABLE-G 1 (LOCKED set, §3), COMPILED-TABLE 2 (PMF/fn-ptr state tables:
`TBoss6PlotFalzAction`, `trade_proposal_state_handlers` — MWCC-PMF Cause-B RW-from-saves
precedent, abi.md §2 / DRAGON line).

7 types have no struct body in-tree (typedef/extern/macro-decl); each got an individually
argued override row (OnlineQuestMenu typedef chain, QuestListEntry TMenuListEntry macro,
DVDFileInfo opaque SDK fwd-decl, QuestDownloadPoolCell pool union, gsl_archive_entries
D1 blob array, the two handler tables).

## 3. Correction law compliance (AL)

LOCKED pins ONLY on: (a) the 7 census `yes` rows (IPv4Header, GuildCardSlot, BankItemRecord,
CharacterFile, TcpHdr, QuestListEntry, GuildCardSaveData); (b) G/H-table names verified in
the docs (the 3 doc-name rows are all `yes`-flagged — zero locks from doc-naming alone);
(c) embeds-locked-by-value propagation to fixpoint (TProtocol precedent), seeded ONLY from
confirmed keepers: the 46 resolutions-1..4 LOCKED + the 12 assert-verdicts `L-P
(FORMAT-KEEP)` types. `probable` and path-adjacency contributed ZERO locks: the 58 `probable`
tail rows were re-judged structurally — 53 → RW, 5 pinned ONLY via confirmed-keeper
embeds, never via the flag. LOCKED beats RW where both signals exist (CharacterFileSlot,
FloorEventSet, G_SendGuildCard_6x06, DhcpMessage, OnlineQuestMenu are the `probable`-flagged
tail rows pinned by confirmed-keeper embeds/typedef-chains, not by the flag
(Pso3CharacterFileSlot/Pso3CharacterFile/TPlyJoinData carry `no`/`yes` flags and pin via
the same embeds law).

## 4. NEW escalations (loud)

1. **10 new structural LOCKED pins beyond the 7 AK-yes rows**, all by-value containment of
   confirmed keepers/wire records: `CharacterFileSlot` (embeds CharacterSaveRecord),
   `Pso3CharacterFileSlot`/`Pso3CharacterFile` (GC-card clone chain), `DhcpMessage`
   (embeds BootpPacket — AL-confirmed at BmlResource.cpp:172), `FloorEventSet`
   (embeds SetDestroyEventRecord), `TPlyJoinData` (embeds TPlyChallenge),
   `EndingWallTextureSet` (embeds NJS_TEXNAME), `G_SendGuildCard_6x06` (embeds
   GuildCardEntry keeper + G_ClientIDHeader), `OnlineQuestMenu`
   (TMenuList<QuestListEntry,30> typedef), `gsl_archive_entries` (extern GslArchiveEntry[256]
   = H-D1 disc-DMA blob, silent-flip site #1). **AK: promote to `yes` at next regen.**
   CharacterSaveRecord-lookup/GC-card images must not waive these when the loader lanes land.
2. **47 wire-adjacent rows HELD RUNTIME-WAIVE despite protocol-path/bswap signals** —
   correction-law hold-down, e.g. `ChangeShipOrBlockPacket` + `CharacterSaveInfoPacket`
   (declared in locked `TProtocol.h` but no by-value wire-blob embed),
   `G_ModifyTradeProposal_6xA6`, `G_SyncItemState_6x6D_Decompressed`, the Ipv6Nd/Lcp/Ipcp
   option records (RFC-BE regime B payloads if ever read off the wire),
   `NetProtocolMessage`/`NetworkDispatchState`. These embed NO keeper type and lack a
   bswap-confirmed collision; if any is later found loaded-image-overlaid, it is a
   FLIP-RISK — needs the AK `yes`-confirm tranche (read-the-bswap-site), same 4/14
   survival rate as AL's probables argues for RW until then.
3. **`packet_header` is NOT-AT-FAULT (not locked)** while every regime-A packet embeds it —
   if AK later pins packet_header as FORMAT-KEEP, propagation here (§3 seeds) must be re-run;
   ~12 tail packets flip to LOCKED in one step (deferred, honest).

## 5. Closure math — FULL corpus after this pass

Named-universe = 935 uncapped-failing types named pre-tail (tranche1-3 ∪ resolutions-1..3)
+ 101 AL (resolutions-4) + 566 AT (resolutions-5) = **1,602 / 1,602 AK-uncapped failing
types named — tail closed, 100%**. Residual open honestly: 0 rows in this batch, but
(a) the anonymous `'?'` pseudo-type (1 site, AK §1) stays unnamed-by-construction;
(b) the 17 LOCKED pins await AK `yes` promotion (§4.1); (c) `PppFrame`/`structPppFrame`
regex artifact (§2 of AK doc) — named, cosmetic only.

Projected LP64 movement on the stitched header basis (2,179/3,333 waived, 934 types,
@b85a254; 150 materialized flips, parse 1576/1680 arm64_32): this pass adds 549 RW rows
covering **597 previously-UNREVIEWED assert sites** (548 of my types have rows in
assert-verdicts.tsv) → projected ≈2,776/3,333 waivable, and my 17 LOCKED rows add 24
keeper sites (not waivable — loader-lane work). Unit-level sim (AL's verbatim grammar over
the 3,010 uncapped `.err` logs, read-only, committed header untouched): **125 units become
pure-layout-clean from these rows alone (10 src-TUs + 115 header units)** — the src-TU
unlock AL said was owed to the tail batch starts arriving; remaining src units still die
on non-layout residuals (dialect/intrinsic/asm). **Materialize estimate REQUIRES the
integrator battery (make -C build parse re-pin); not run here by rule — do not quote
150+125 as a certified ceiling.**

Scratch: `cache/scratch/at/` (parse index, fixpoint sim, sim5.json). No host/, build/,
or decomp writes; 29 checkpoint commits (20-row storm law), no -A/amend/push.
