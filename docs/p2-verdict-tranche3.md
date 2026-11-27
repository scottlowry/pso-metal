# P2-X — LP64 verdict queue tail (final 271 types; sheet `verdict-tranche3.tsv`)

Phase-2 task X, the tail of the P2-R/P2-S queue: **every remaining LP64-failing type
not yet in a tranche sheet** (S's "271 types / 395 sites floor"), completing coverage
of the 829 failing types R reported. Same corpus, same harness, same 8-col grammar as
`verdict-tranche2.tsv` (S packed tranches 2+3 there; this sheet is the next
sequential name — by S's rank numbering it is queue **tranche 4**, ranks 559–829).
Method = R's rules exactly (S's `build_tranche23.py` selection/classification code
reused verbatim), narrow doc-named LOCKED rule, census-invariance assert
(1467 LP64 sites / 1444 LP64-only) re-passed before judging, decomp @ `6b38b5383`,
floor counts (20-error cap).

## 1. Verdict split (271 types, 390 measured LP64-fail sites)

| Verdict | Types | LP64 sites | Meaning |
|---|---:|---:|---|
| RUNTIME-WAIVE | **264** | 371 | LP64-only pointer-widening class or doc-named runtime type; not in a FORMAT table; waiving is safe |
| LOCKED-FORMAT | **3** | 10 | `NetworkInfoConfig` (D5–D10 blob, 5 sites), `NetworkInfoConnectionConfig` (= D6 `ConnectionConfig(0x210)`), `MfciFileEntry` (D11 movie-TOC entry) — **hard refuse**; the only format-surface types left in the whole tail |
| UNRESOLVED | **4** | 9 | quest-challenge dir (`TQuestListOffline`, `TChallenge4PItemSelect` 6 sites, `TQuestDamageTask`) + `TPlyOnCountOver` (network/); settle by overlay audit (grep load/memcpy into disc/wire image, p1-format-saves §3); none is named in any FORMAT table |

The tail is as predicted: per-file `enemy/`/`boss/`/`ui/`/audio headers, leverage ≤ 50
— nearly all pure pointer-width failures, almost no format surface (16 types / 19 sites
held, vs 664/20 in tranches 2–3).

## 2. S's two escalations re-audited — both explicit verdicts

* **`TSaveChCrStateActions`** → **RUNTIME-WAIVE** (judged in this sheet). The 2
  ILP32-residual sites (`TSaveChCr.h:66-67`: `sizeof(TSaveChCrStateActions)==24`,
  `offsetof(task)==12`) are the MWCC 12-byte-PMF dispatch-table class (Cause B, same
  pattern as `PlayerWindowCommand/Query`, abi.md §2 row 4) — the FORMAT docs already
  name this exact type: p1-format-saves §3.1 "TSaveChCr* PMF tables … pure Cause-B UI
  state machines that *drive* the writes; their bytes never reach the card", §3.2
  "RUNTIME-WAIVE-from-saves", net-disc §4 ("no walker embeds it; card blob formats are
  the McFsGc/char-save walkers"). S's FORMAT-KEEP suspicion resolves **against**
  LOCKED: the residual is explained by PMF representation, not a format signal.
  Settle by the compat/dialect PMF shim lane, not save re-pin.
* **`PlayerWindowQuery`** (tranche-1 row, RW) → **RUNTIME-WAIVE STANDS** — re-audit
  flag closed. Its 1 residual site is `WindowCtrlCallback.h:10`
  `sizeof(PlayerWindowQuery)==0xC`: MWCC 3-word PMF vs clang 2-word (abi.md §2 row 4:
  class B, 119 TU) — fails in both regimes by construction, which is exactly why it
  is ILP32-residual. p1-format-saves §3.2 names it B / RUNTIME-WAIVE ("PMF callback
  pairs in compiled code, window ctrl only"). No G/H-lane eyeball needed; same shim
  as TSaveChCr. No new sheet row (already tranced in tranche 1).

## 3. One correction to S's narrow rule (discovered this pass, same lesson)

S's LOCKED list includes a whole-header path rule on `NetworkInfoCodec.h`. The net-disc
§4 table carves `info-struct NetworkInfoCipher*` (lines 87–105) out as **RUNTIME-WAIVE
inside that same header** — the path rule was false-flagging `NetworkInfoCipherKey/
PoolBinding/Workspace` (3 types, 3 sites) that the tail surfaces. Doc-named row beats
path (the same rule S established after the broad-`network/` revert): released to RW,
evidence cited. D5–D10 blob family (`NetworkInfoConfig*` non-Cipher names, size-scan
confirmed `NetworkInfoConnectionConfig` = D6) stays LOCKED.

## 4. Aggregate ceiling simulation (all four queues; S's 206 anchor reproduced first)

Same deduped per-file error-event replay. Anchor check: t1+2+3 RW = **825 sites →
+136 clean → 206/1680**, exactly S's saved `p2s_tranche23.json` output.

| Waiver set | RW sites | LP64 parse | Δ |
|---|---:|---:|---|
| today (ILP32-fitted list) | — | 70/1680 | baseline |
| tranche 1 (R) | 161 | 138 | reproduced |
| tranche 1+2+3 (R+S) | 825 | **206** | anchor, reproduced |
| **+ T4 RW 371 sites → queue total 1196** | 1196 | **229/1680** | **+23 files** |
| waive ALL measured failing types (1467 sites) | 1467 | 1640/1680 (97.6%) | assert-policy ceiling, re-confirmed |

Diminishing per-file yield as predicted (tail ≈ 2.6 files/100 sites vs 68/161 in t1):
front-loaded umbrella types are exhausted. Above 229/1680 the only route is the
non-assert lane (§5) plus LOCKED re-pinning + the 4 UNRESOLVED overlay audits.

## 5. The 40 "unreachable by assert policy" files — verified classification

Waive-all replay: 1570 newly clean, **0 files hidden behind the cap**, 40 files with
residual *visible* non-assert errors (verified: every one has ≥1 non-layout error, so
none is a truncation artifact). This confirms S's headline (1640/1680 ceiling, 40
unreachable) but corrects the prose in S's §4 — the "1570 / 12 / 28" hidden/still split
does not match S's own stored `simall` output `[1467, 1570, 0, 40]`. Error families
(files can carry two): dolphin C-dialect header parse **34** (`extern`+`asm` idioms in
OSExec/OSLink chains, mostly `src/musyx/runtime/*.c`), musyx `SoundReverb`-style
struct-tag collisions **26** (overlapping set), PPC `__dcbt`/`SI*Polling` intrinsics
2–3, MWCC-isms 6 (`__builtin_va_info` ×2, `__register_global_object`, negative-size
macro form ×2, `-Waddress-temporary`). These are compat-shim + host-stub lane (abi.md
§6), not assert policy; R's "66 files" was the raw-line decomposition of the same 40 +
26 truncated (reconciled in S §4).

## 6. Census completeness — queue closed

* Measured named failing types this pass: **828**. R's headline **829** = 828 named +
  the `'?'` unattributable bucket (27 LP64 sites / 7 LP64-only). No type drift: the
  earlier fake type `struct` is a `sizeof(struct PppFrame)` regex artifact — the site
  re-attributed to **`PppFrame`** here (tranche 1's `structPppFrame` row claimed
  0-sites; it has 1 LP64-only site, judged RW in this sheet).
* **828 = 127 (tranche 1 ∩ failing) + 430 (verdict-tranche2) + 271 (this sheet) — 100%
  of named LP64-failing types now tranced.** The other 73 tranche-1 rows are
  UNREVIEWED-queue entries with zero measured traffic under the cap.
* LP64-only site ledger (1444): 244 (t1) + 805 (t2-3) + 388 (t4) = **1437 judged**;
  residual **7** = `CHAT_OFFSET`/`TRADE_MESSAGE_OFFSET` macro-form asserts
  (`TWindowChat.h:66-70`, `TWindowSubTradeMessage.h:26-27`) with no type token to key
  a row on — waivable by assert-label in the waiver header (chat/trade windows are the
  §3.2 never-saved window class); they account for S's "395" vs my 388 (395 = 388 + 7).
* First `-ferror-limit=0` re-probe recommendation is unchanged (p2-lp64 §3.6): all
  counts above remain floors.

Repro: `python3 cache/scratch/clangprobe/build_tranche4.py` (rebuilds the corpus,
asserts 1467/1444 invariance, re-runs S's anchor sim before extending). TSV schema:
`type verdict asserts lp64-fail-sites lp64-tu-traffic header-include-leverage
declaring-header evidence`.
