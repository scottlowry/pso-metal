# P2-AK — Census blind-spot audit (uncapped evidence, tree-wide)

Phase-2 task AK. Measures what the per-file-capped census (R/S/X: `lp64native/err`,
clang default 20-error cap) missed, using **T's uncapped corpus** (`p2t_uncapped/err`,
1,610 prior-failers re-run `-ferror-limit=0`, same flag set) as the sole
uncapped LP64 evidence. `pso-decomp @ 6b38b5383` read-only, zero builds.
Output: [`data/census-audit.tsv`](../data/census-audit.tsv) (667 BLIND rows,
6-col grammar `type|sites|files-hit|first-error-file:line|in-G/H-FORMAT-table|tranche-gap-reason`).
Scratch: `cache/scratch/p2ak_audit.py` (parse+classify), `p2ak_stats.json`,
`p2ak_extras.json`.

## 1. Headline — the 829 was a floor, not a census

| quantity | capped (R/S/X era) | **uncapped (T corpus, this audit)** | delta |
|---|---:|---:|---:|
| LP64-failing **types** | **829** (828 named + `'?'`) | **1,602** (+1 `'?'`, 1 site) | **+773** |
| LP64-failing **assert sites** | 1,467 (X snapshot) / 1,447 current bytes | **2,951** | **+1,504** |
| Failing TUs | 1,610 | 1,610 (file set byte-identical, `diff fail.txt` empty) | 0 |

Capped **1,316 of 1,610** failing logs (82%) ended on
`fatal error: too many errors emitted, stopping now` — the cap was binding nearly
everywhere, so the type census undercounted by ~93%. The single TU that is
QuestOpcodeHandlers.cpp alone shows **486 sites uncapped vs 20 visible capped**.

**BLIND = uncapped-failing − named = 667 types / 1,130 sites.**
Named corpus = `verdict-tranche1-3.tsv` $1 (901) ∪ `verdict-resolutions-1/2.tsv` $1
(144) = 936 unique; 935 of them fail uncapped (the one that doesn't is
`structPppFrame` — X's documented regex artifact, real name `PppFrame` is named and
fails). Zero capped-census types are unnamed (queue-closed claim holds for what the
cap could see).

## 2. Extraction grammar + two fixes found along the way

Site detection = R/S grammar verbatim (`ERR` / `NEG` / `LAYOUT_ASSERT(<Name>` /
`sizeof|offsetof(TYPE` — `build_tranche2/23/4.py` family, dedup+upgrade semantics).
Verified grammar-exact: my capped parse is **set-identical** to S's formula on the
same log bytes. Two attribution defects repaired, both inherited from `census.py`:

1. **Blob window 3→8 lines (stop at next diagnostic).** `DRAGON_OFFSET`-style
   `##`-paste wrappers put `offsetof(TBoss1Dragon, member)` on the macro-body line
   *past* a 3-line window; capped `'?'` went 7→0 (X-era 27), uncapped `'?'` 55→1, and
   TBoss1Dragon now measures **43 sites — AH's count exactly**.
2. **Cap detection must use the truncation marker**, not error-count≥20 (re-diagnostics
   share a `path:line`, so counts under-read the cap).

Site-count reconciliation vs X: 1,467→1,447 on current bytes = corpus-era drift
(waiver headers edited after X's snapshot); the **type-count invariant 828+`'?'`=829
reproduced exactly**, so the diff below is against the same census X tranched.

## 3. Gap-reason anatomy — it's the cap, everywhere

All 667 blind types classify **`cap-hidden`** (every one is reported by ≥1 TU whose
capped log hit the 20-error truncation). `closure-only` = 0: only 8 capped TUs died on
include-closure fatals (`orig_asm/**/*.inc` not-found — T's "include-graph 0-first/
13-secondary") and a fatal stops compilation in **both** eras, so closure contributed
zero type-loss tree-wide. AH's quest story generalizes: it was never quest-specific —
the cap ate the queue's tail tree-wide. `late-added` = 0 (no blind type had
capped-visible sites, i.e. the 829 never saw any of them).

## 4. Top-20 blind types by leverage (`files-hit` = TUs whose uncapped log fires a site)

| type | sites | files | first blind site | G/H table |
|---|---:|---:|---|---|
| CharacterSaveRecord | 8 | 65 | include/pso/system/CharacterSaveRecordLookup.h:88 | **yes** |
| IPv4Header | 1 | 41 | include/pso/network/IPv4Header.h:19 | **yes** |
| TBoss2MotionProfile | 1 | 36 | include/pso/boss/derolle/TBoss2MotionProfile.h:32 | no |
| NetBufferPool | 3 | 31 | include/pso/network/NetIpv6Route.h:53 | probable |
| NetBufferPoolEntry | 2 | 31 | include/pso/network/NetIpv6Route.h:52 | probable |
| Ipv6NdNeighbor | 1 | 31 | include/pso/network/NetIpv6Route.h:123 | probable |
| Ipv6NdPending | 1 | 31 | include/pso/network/NetIpv6Route.h:122 | probable |
| Ipv6NdRouter | 1 | 31 | include/pso/network/NetIpv6Route.h:124 | probable |
| Ipv6NdState | 1 | 31 | include/pso/network/NetIpv6Route.h:125 | probable |
| LoopbackState | 1 | 31 | include/pso/network/NetIpv6Route.h:31 | probable |
| NetFragPacket | 1 | 31 | include/pso/network/NetIpv6Route.h:166 | probable |
| NetFragPool | 1 | 31 | include/pso/network/NetIpv6Route.h:167 | probable |
| NetFragment | 1 | 31 | include/pso/network/NetIpv6Route.h:165 | probable |
| TTecPlayerInfluence | 1 | 31 | include/pso/technique/TTecPlayerInfluence.h:41 | no |
| TBoss2DeRolLe | 1 | 30 | include/pso/boss/derolle/TBoss2DeRolLe.h:218 | no |
| ParsedTcpHeader | 1 | 29 | include/pso/network/PppVj.h:50 | probable |
| PppPapAuthenticationState | 1 | 29 | include/pso/network/Ppp.h:23 | probable |
| PppVjCompressionState | 1 | 29 | include/pso/network/PppVj.h:85 | probable |
| PppVjConnectionState | 1 | 29 | include/pso/network/PppVj.h:60 | probable |
| TWindowKeyBoardSoft | 3 | 28 | include/pso/ui/window/TWindowKeyBoardSoft.h:57 | no |

(16 of the top-20 carry a FORMAT signal — the network/save clusters hid almost whole
headers behind cap line-budgets, e.g. all 10 `NetIpv6Route.h` asserts × 31 TUs.)

## 5. FORMAT-table collisions — loud: 80 blind rows are LOCKED-by-evidence

`in-G/H-FORMAT-table=yes` — **8 blind types are named FORMAT-KEEP in the G/H doctrine
(S-curated LOCKED ∪ backticked FORMAT-KEEP doc lines) yet never carried a tranche row**:
`BankItemRecord CharacterFile CharacterSaveRecord GuildCardSaveData GuildCardSlot
IPv4Header QuestListEntry TcpHdr`. These are save/wire byte images: their verdict is
**already pinned by the G/H tables — lane AL must consume the pin, not re-derive it**
(re-pin/pad per p1-format-saves §3; never waive; p1-datamodel §3 rule-1 remains the
compile precondition). `probable` = 72 more (site lives in S's LOCKED_PATHS or a
format-adjacent dir, incl. the whole `NetIpv6Route/PppVj/Ppp/Bootp/Dhcp/Ipcp` wire
family, `Bml*`, challenge/quest tables) — treat as fail-closed UNRESOLVED, doc-check
required before any waiver. `no` = 587.

## 6. AH's 35 quest types — subset check PASSES

Identified mechanically as the 35 `verdict-resolutions-2` types absent from
tranche1-3 names (the other 49 r2 rows re-judge tranced names). Verified:

* **35/35 appear in the uncapped failing-type set** (recovered by log parse alone —
  T's corpus reproduces AH's hand-probed quest result without any quest-specific work);
* **35/35 are named** (resolutions-2) → **0/35 leak into BLIND post-resolutions-2**;
* pre-resolutions-2 all 35 would have been blind (none appears in tranche1-3), which
  is AH's "UNREVIEWED story" confirmed as a queue fact, not a verdict fact;
* spot-counts match AH exactly: **TBoss1Dragon 43 sites, TItemMag 8, TChallenge4PResult 6**.

NJS_* membership note (no re-adjudication — AJ owns): `NJS_OBJECT`/`NJS_MKEY_F` remain
named via tranche1 + resolutions-2 rows and both still fail uncapped (sites 2/1 →
2/1 stable); anything AJ-side lands in `verdict-resolutions-3.tsv` and is automatically
excluded from BLIND by the $1-diff once it exists.

## 7. Dialect-flip reconciliation (the "26 pass" footnote)

T's 26-file `p2t_flips` pool: **every one contributes 0 layout-assert sites to the
uncapped corpus** (verified per-TU) — they are dialect-mode failures (musyx.h
`extern "C"`, AT_ADDRESS), never assert traffic, so the flips neither shrink the
1,602-type census nor touch the 667 blind rows. File-union claim checked directly:
`p2t_uncapped/fail.txt ≡ lp64native/fail.txt` (1,610, byte-identical set), and **no
named capped-era type disappeared uncapped** (`named_capped_but_not_uncapped = 0`) —
the brief's "−2 regressions" is not observable in this corpus pair; the 70 era-passers
were never re-run uncapped (their capped logs are zero-error, so cap hid nothing there).

## 8. Sized queue for AL (blind-set adjudication)

Site-count distribution over the 667: 509×1-site, 57×2, 40×3, 18×4, 17×5, 8×6, 6×7,
4×8, 2×9, 6×10+.

* **Priority tier (≥3 sites): 101 types / 507 sites** — that is where assert-policy
  buys file-cleanings; adjudicate first (S's tranche-3 rule ladder applies verbatim,
  with §5's yes/probable pins pre-baked).
* Long tail: 566 types / 623 sites (1–2 sites each) — batch by declaring header for
  class-level verdicts (the NetIpv6Route.h 10-type / PppVj-family clusters show why
  header-batching beats per-type for the tail).
* The uncapped census also invalidates X's waive-all ceiling math (1,467 sites): on
  2,951 sites the file-clean projection must be re-simulated from
  `p2t_uncapped/err` before the next parse-gate target is quoted (AL or a sim lane).

## 9. Caveats

* `files-hit` counts distinct reporting TUs (uncapped era), not include-graph leverage;
  leverage ranking stays R's `LEV` measure (not recomputed here).
* `first-error-file:line` = lowest `(path,line)` of the type's uncapped blind sites.
* `'?'` bucket (1 site uncapped) left unattributed by design; blob-window repair (2) is
  the only grammar deviation from S/X, and it only *reduces* mis-attribution.
* Closure-class caveats inherited from T's 113-file triage (`docs/p2-dialect-triage.md`)
  — those are compat-shim lane blockers, orthogonal to assert policy.
