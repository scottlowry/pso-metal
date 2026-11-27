# P2-S — LP64 verdict queue, tranches 2–3 (430 types)

Phase-2 task S. Continuation of the P2-R queue (`p2-lp64-tranche1.md`): the **next
430 LP64-failing types by include-closure leverage** (rank 129–558 of the 829
LP64-failing types; the 200 tranche-1 types skipped), judged with R's method over
the same measured corpus (`cache/scratch/clangprobe/lp64native` + `ilp32check`,
`arm64-apple-macos11` vs `arm64_32-apple-watchos7.0`, decomp @ `6b38b5383`,
force-include compat + current waiver header, 20-error cap ⇒ every count a floor).
Census drift check: 1467 LP64 sites / 1444 LP64-only re-measured identical before
judging. Queue split for reporting: **tranche 2 = ranks 1–200, tranche 3 = ranks
201–430**; both land in one sheet, `data/verdict-tranche2.tsv` (same 8-col grammar).

## 1. Verdict split (430 types)

| Verdict | T2 | T3 | Total | LP64-fail sites | Meaning |
|---|---:|---:|---:|---:|---|
| RUNTIME-WAIVE | 162 | 197 | **359** | 664 | fail **only** at LP64 (pointer 4→8 B class), not in a format-adjacent dir, not G/H-named — waiving is safe, bytes never leave the process |
| LOCKED-FORMAT | 6 | 5 | **11** | 20 | named in the G/H FORMAT tables (or re-extracted from any FORMAT-KEEP line) → **hard refuse**: re-pin/pad, never waive |
| UNRESOLVED | 32 | 28 | **60** | 121 | format-adjacent dirs (`network/`, `quest/`, `map/`, `system/save|movie|Gsl|Rlc|Bml|Aram|TGStream|CvFs`, `SubtractiveRng`) + `NJS_*` overlay class; row names the settling evidence (overlay audit, p1-format-saves §3 / lane J) |
| FORMAT-KEEP candidate | 0 | 0 | **0** | 0 | see §2 — the both-regime class was exhausted by tranche 1's ranking |

## 2. Why zero FORMAT-KEEP candidates (and one exception to escalate)

The FORMAT-KEEP signal in R's method = a type whose assert **also fails at
arm64_32** (ILP32-residual class: failure not explained by pointer widening) and
that is not already G/H-named. Across the whole corpus only **3 such sites exist
on 2 types**: `PlayerWindowQuery` (1 site) and `TSaveChCrStateActions` (2 sites).
`PlayerWindowQuery` was judged RUNTIME-WAIVE in tranche 1 (explicit doc RW type) —
**tranche-1 re-audit flag**: its 1 residual site means one of its asserts fails in
both regimes; the waiver is still doc-sanctioned, but the residual assert should be
eyeballed by the G/H lane. `TSaveChCrStateActions` (`system/save/TSaveChCr.h:66`,
memory-card save state machine) has leverage below the tranche-3 cutoff → still
untranced; it is the one live FORMAT-KEEP candidate in the queue and should settle
with the save records re-pin, **not** a waiver.

## 3. New LOCKED-FORMAT escalations (11)

| Type | LP64 sites | TU traffic | Header | Why locked |
|---|---:|---:|---|---|
| `TItemData` | 3 | **511** | `item/TItemData.h` | saves §1 row 1 (mixed-width unions, `serialize_mag`); member of `TPlyInventoryItem`[30]/`BankItemRecord`[200]/`FloorItem` — its LP64 failure means every saved item slot is layout-shifted; the single biggest save landmine found by this tranche |
| `TPlyCharData` | 2 | 16 | via `ui/frontend/TAdSinglePlyChrSelectGC.h` | character blob 0x41C (`player_stats`/stats + inventory inline) |
| `SystemSaveData` | 1 | 31 | same chain | system save record |
| `SymbolChatPreset` | 3 | 24 | via `ui/chat/TWindowChatSymbol.h` | `bswap_entry()`, explicit wire format, in CharacterSaveRecord |
| `GuildCardEntry` | 2 | 4 | via `ui/window/TWindowSubComCard.h` | guild-card disc + wire container of `gc_tag` |
| `CardSaveHeader` | 2 | 3 | via `TAdPlayerLoadGC_801E0C30.h` | CRC'd card header |
| `QuestBinHeader` | 1 | 0 | `quest/QuestScriptFormat.h` | disc quest-bin header |
| `GslArchiveEntry` | 3 | 4 | via `unk/text_800FFE04.h` | net-disc D1: **currently UNSWAPPED** highest-priority flip site |
| `G_DropItem_DC_6x5F` / `G_DropItem_PC_V3_BB_6x5F` / `G_PlaySound_6xB2` | 1 ea | 1–3 | `network/protocol/*.h` | §1.2 game-protocol wire records (314 structs class) |

This confirms and extends tranche-1's headline escalation: LP64 **does** touch the
save/wire format surface (`ChallengeRecord`/`BattleRecord` there; `TItemData` and
the card/quest headers here). The ILP32-era claim "0 of 46 failing asserts touch
the save format" is now dead for the link target.

## 4. Ceiling simulation — tranche 1+2+3 aggregate

Same harness as tranche-1's sim (deduped per-file error-event replay over
`lp64native/err`, R's script reproduced tranche-1 exactly: 70 → 138, +68, 0 files
hidden by the cap):

| Waiver set | newly parse-clean files | LP64 parse | vs floor |
|---|---:|---:|---|
| today (ILP32-fitted 99-name list) | — | **70/1680** | baseline |
| tranche 1 RUNTIME-WAIVE (161 sites) | +68 | 138 | R, reproduced |
| **tranche 1+2+3 RUNTIME-WAIVE (825 sites)** | **+136** | **206/1680** | this tranche adds **+68 files / +664 enablable site-waivers** (queue ≈ as productive as tranche 1 per file, front-loaded umbrella types are done) |
| waive ALL 829 measured failing types (1467 sites) | +1570 | **1640/1680 (97.6%)** | assert-policy ceiling on this harness |

Ceiling reconciliation (honesty note): tranche-1 doc says 1544/1680; that number
came from the first census's raw-line decomposition (1544 assert-only / 29 mixed /
37 non-layout), which classifies 20-error-cap-truncated files conservatively as
mixed. The deduped site-replay harness used by both simulations measures the
partition as **1570 / 12 / 28** (70 pass ⇒ 1640 ceiling; 40 files unreachable by
any waiver: compat-header shims + PPC `asm`/intrinsic class, host-stub lane, not
assert policy). Both counts stand on their own method; the delta is 26 truncated
files, not missing errors.

## 5. Coverage of the 1444 LP64-only sites

* Tranches 2–3 judge types carrying **805 / 1444 = 55.7%** of the LP64-only sites
  (664 → waivable now, 121 → held for overlay audit, 20 → LOCKED re-pin).
* Cumulative tranche 1+2+3: **1049 / 1444 = 72.6%** judged; 825 sites (57.1%)
  currently waivable; remaining 395 sites sit on the **271 still-untranced**
  failing types (leverage < cutoff) — floor counts, the cap hides more.
* Next queue head: per-file `enemy/`, `boss/`, `ui/` headers (as predicted), then
  the `-ferror-limit=0` re-probe (p2-lp64 §3.6) to turn every floor exact.

Repro: `python3 cache/scratch/clangprobe/build_tranche23.py` (reads the two probe
corpora + `p2r_leverage.json`, asserts census invariance 1467/1444 before judging).
TSV schema: `type verdict asserts lp64-fail-sites lp64-tu-traffic
header-include-leverage declaring-header evidence`.

## Corpus corrections (AI)

Additive flag note from lane AI (P2-AI corpus hygiene) — no row in
`data/verdict-tranche2.tsv` was rewritten. Two rules this tranche relied on are
invalidated per `p2-verdict-resolve-1.md` §5; future lanes must NOT re-apply them:

1. **The blanket "LP64-fail-site ⇒ overlay-suspicion" gate (§2 table :19, RW class
   def :17, coverage :80) is invalid.** Era `types.h:8-9` typedefs `s32/u32` as
   `long`, so ALL PODs widen at LP64 — many "passes arm64_32, breaks only when
   pointers widen" sites are pure width-noise, not pointer-overlay or
   format-crossing evidence. AD's settlement pass re-proved the 60 gated UNRESOLVED
   types with positive overlay-cast grep / G-H table containment (357 RW rows + the
   55 non-NJS gate rows flagged as evidence-confounded; resolutions override per
   resolve-1 §2 precedence).
2. **net-disc §2.2's blanket "Ninja runtime = WAIVE" overreaches** — loaded-image
   structs (NJS_*, EnemySetRecord, FloorObjectSetRow: overlaid on disc/GVM/GVR byte
   images) are FORMAT-KEEP; lane J handle conversion (p1-datamodel §3 rule-1) is the
   resolving mechanism, rows stay LOCKED meanwhile (resolutions-1:16,17,52,60,61).

Row-level enumeration: `data/corpus-corrections.tsv`. Verified (not re-edited): the
94-vs-102 newly-waivable site count is already corrected in resolve-1 §1 — 94 is the
measured number; 102 survives only in the immutable 1/2 commit message.
