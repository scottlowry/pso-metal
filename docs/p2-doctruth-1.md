# P2-DOCT — repo-wide doc-truth sweep + corrections (card t_e9752ebc, 2026-11-27)

Lane DOCT, fleet wave-5.5b. ZERO builds, ZERO code edits (one tools/*.py
**docstring** line included). Ledger of record read first: `docs/p2-stitch-apply8.md`,
`docs/p2-rc2-recensus.md`, issue #8 body. Definitions of record used throughout:
LP64 official **1,167 PASS / 513 FAIL** (stitch-apply8:24), arm64_32 floor **1,605**,
**H0′=167 / corrected A8N=357** (rc2 §0/§4 — certified A7N=192 was zero-REBIND, NOT
minus-njs), run.sh **200/200**, ladder7_test **43/43** (play2:29), waiver header pin
**d4382fcd0659357f**, waived=**2,946 over 1,549 types** (disk `build/waivers/waiver-status.txt`),
genuine-logo rule (issue #8): *extraction layer may be genuine; every green readback gate
is self-consistency (pipeline vs itself); visual/semantic claims need independent ground truth;
fmt-9 palette semantics DISPUTED (era `is_palette_texture_format{8,9,10}` ⇒ CI14 class).*

## 1. Sweep method + positive control (gates proven able to fail)

Python sweep (`cache/scratch/doct/sweep.py`, wave scratch — grammar below) over
`docs/**/*.md` + `build/README.md` + `tools/*.py` **docstrings** (ast-extracted).
7 category grammars: `PAL8-FMT` (`\bPAL8\b|PAL 8|8-bit indices`), `RENDER`
(`renders?…(logo|sega)` + `LOGO VISIBLE`), `EXACT` (`readback[- ]EXACT|texel map EXACT|EXACT (±0)|BIT-EXACT|…`),
`LANDED`, `GENUINE`, `METER` (`1,163|517 FAIL|1601|350 PASS|128/128|156/156|28/28 PASS|185/185|H0…171|103 RUNTIME-WAIVE`),
`CITE` (`commit <sha7-40>` cross-checked against `git cat-file`).

**Control proof:** planted 6-line `CONTROL.md` of false claims fired **7/7** categories
(`CONTROL-FIRED: 7/7`). The controller genuinely failed first: v1 grammar
(`[0-9a-f]{7,12}\b`) missed the 15-hex planted SHA — gate aborted the run
(`AssertionError: GATE DID NOT FAIL ON CONTROL`) until widened. Corpus: 128 hits
(109 EDITABLE / 11 LIVE-LANE / 8 BASELINE); all adjudicated below. Line counts of
all edited files verified unchanged (zero `file:line` cite drift); `parse_gate.py`
still `ast.parse`-clean.

## 2. Corrections APPLIED (this commit — 17 edits, 10 files; every one ledger-cited)

| file:line | claim | verdict | applied wording (delta) |
|---|---|---|---|
| build/README.md:12 | gate floor "< 1601 (apply-6)" | FALSE vs disk `parse_gate.py:34`=1605 + STATUS:15 | `< 1605 (apply-7 floor; history …1601->1605; disk gate tools/parse_gate.py:34)` |
| build/README.md:13 | parse-lp64 "162->261->350 PASS" | STALE (stops apply-6; audit :65 flagged, out of its scope) | chain extended `-> 1,163 -> 1,167 PASS (apply-8 official, docs/p2-stitch-apply8.md:24)`, ledger cites /8 |
| build/README.md:34 | "ACTIVE today — **103 RUNTIME-WAIVE asserts** neutralized" | FALSE vs disk: ACTIVE waived=2946/1549 (103 was Phase-1, `p1-buildsystem.md:46`) | `2,946 of 3,333 LAYOUT_ASSERTs … over 1,549 … types (disk waiver-status.txt; era stitch-apply7.md:51; pin d4382fcd0659357f)` |
| tools/parse_gate.py:17 | docstring LP64 ledger stops at 1,163 | STALE (audit :62 flagged, out of its scope) | `1,163 -> 1,167 apply-8, ledger …apply5/6/7/8.md` |
| docs/STATUS.md:18 | meter "frame-loop test binary 28/28" | STALE — superseded by play2:29 43/43 | `**43/43 PASS** (28 PLAY-1 + 15 PLAY-2) | docs/p2-ladder7-play2.md:29` |
| docs/p2-handle-program-1.md:444 | FORWARD gate "re-runs run.sh (bringup 128/128)" | STALE as instruction (audit :69 flagged) | `bringup GREEN — 200/200 today per docs/STATUS.md §1` |
| docs/p2-v-wave.md:137 | "V4 — LOGO VISIBLE (…: LANDED)" | OVERSTATED render claim (issue #8 rule) | + `— fixture-path chain executing + supply byte-verified; every readback gate below is a SELF-CONSISTENCY proof, not decode ground truth — issue #8` |
| docs/p2-v-wave.md:163 | "texels, EXACT (±0)" | self-consistency only | + `— vs the pipeline's own CLUT-expanded surface (self-consistency gate; issue #8)` |
| docs/p2-v-wave2.md:58 | "readback EXACT vs CLUT-expanded surface" | self-consistency only | + `— self-consistency gate vs own expanded surface` |
| docs/p2-v-wave3.md:27 | "256x256 PAL8)" label for fmt-9 family | label DISPUTED (issue #8) | + `label — fmt-9 palette semantics DISPUTED, era predicate is_palette_texture_format{8,9,10} ⇒ CI14 class, issue #8` |
| docs/p2-v-wave3.md:38 | "u8 9 = PAL8" on sinbd.gvr | label DISPUTED — the exact record issue #8 convicts | `u8 9, labeled PAL8 — label DISPUTED per issue #8 … decode-oracle in flight` |
| docs/p2-v-wave3.md:56 | "Genuine run … 12/12 PASS" | gates pass but prove only self-consistency | + `(all gates below are self-consistency proofs; genuine-record DECODE semantics DISPUTED — issue #8)` |
| docs/p2-v-wave3.md:63 | "texel map EXACT … vs the era-chain surface" | self-consistency, not era truth | + `— self-consistency gate: pipeline vs its own era-chain output, not disc ground truth (issue #8)` |
| docs/p2-gx-tex-consumer.md:88 | heading "format9 PAL8 → MTLTexture" | decode-semantics claim; label DISPUTED | + `(PAL8 label DISPUTED for fmt-9 records — issue #8 …; the 1 B/texel SIZE class below is era-table-proven and unaffected)` |
| docs/p2-gx-tex-consumer.md:92 | "⇒ PAL8 = 1 B/texel" | naming claim (size claim holds) | `⇒ PAL8 (label DISPUTED — issue #8) =` |
| docs/p2-logo-e-supply-1.md:184 | "9 PAL8 (fmt 0x00000A09)" | label DISPUTED; supply truth holds | `9 PAL8-labeled (… label DISPUTED … byte-true supply unaffected)` |
| docs/boot-spine.md:60 | "Smallest ladder that should show the SEGA logo animation" | capability phrasing read as fact | + `[2026-11-27 ledger note: PLAN, not achieved capability — rung-6 runs the FIXTURE path; genuine-record SEGA/logo decode DISPUTED (issue #8; docs/STATUS.md §2 rung 6)]` |

## 3. UNTOUCHED — adjudicated or ambiguous (integrator call)

**Ambiguous — need adjudication, deliberately NOT edited:**
- **fmt-9 naming contradiction inside the repo itself**: this decomp SDK's
  `GXCITexFmt` (`libs/dolphin/include/dolphin/gx/GXEnum.h:176` region) names
  `GX_TF_C8 = 0x9` (`C4=0x8, C14X2=0xa`), while issue #8 asserts fmt 9 = **CI14**
  using standard-Dolphin numbering (CI8/CI14/CI24). The oracle lane's era-table
  extraction (ORCL) is the decisive witness — no doc edit may pre-empt it. All
  applied edits therefore mark DISPUTED + cite issue #8 **without declaring a winner**.
- docs/p2-tex-upload.md:87 "AN-4 … era-table PROVEN (and GX_TF_C8 == 0x9 in the SDK enum)" —
  size-class proof stands; naming claim sits directly in the contradiction above.
- docs/p2-an4-triage.md:109, docs/p2-tex-supply.md:89/:131 — PAL8 as **synthetic-fixture
  label / bpp-sizing claim** (our own authored bytes; not a disc-semantics claim).
- docs/p2-tail-3.md:74 "BIT-EXACT against the era" (fma/`ps_madd` semantics) — runtime-math
  claim outside the logo ledger; needs its own ground-truth gate before it can be kept or cut.
- docs/p2-gx-tex-consumer.md:1 title + :34 "byte-true PAL8 texels" (fixture pack) + :194
  plan row (already discloses "pinned stand-in palette, deviation AV-1"); :114 heading uses
  internal fn name `gxh_pal8_clut_expand` (code name — editing it would lie about the code).
- docs/STATUS.md:36 `run.sh (156/156, ladder7-1.md:69)` + :41 `(28/28)` — rung-table rows cite
  their **own historical receipt lines correctly** under STATUS's preamble rule ("where the
  receipt predates apply-8 the era is named"), unlike §1 Meters which claims current values.

**Verified-historical / verified-supply / irrelevant — no edit needed:**
- Era receipts w/ date-or-era qualifiers (correct at their date; `p2-public-audit-1.md:71`
  precedent): stitch-apply5:56/:70, apply6:31/:36, apply7:46/:47/:55, d4facade-1:10,
  tail-2:13-14, njs-adopt-1:1462, adapter-queue-1:1628, hg1-1:46 (185/185), ladder7-1:11
  ("post-apply-7"), ladder7-1:69/:70, v-wave:77/:111/:179, v-wave2:69, v-wave3:78,
  tail-3:187 (already shows the correct 1,163→1,167). `public-audit-1.md:18/27/62/65/69/70`
  is the audit-of-record *describing* these flags — now being landed **by** this commit.
- `genuine` at the extraction layer (issue #8 explicitly allows): logo-e-supply-1:1/:7/:290/:298/:302/:358,
  v-wave2:38/:44/:48/:63/:79, v-wave3:1/:4/:21/:31/:48/:52/:85/:90/:92/:93, stitch-apply7:57
  (already honest: "default synthetic-fixture path … genuinely NOT wired into committed defaults").
- Non-render "genuine"/"render" noise: p1-datamodel:74, link-deps:116, musyx-1:27,
  public-audit-1:101/:115, verdict-contradictions:191, bringup-3:18 / rung5-seams-1:1/:101 /
  logo-e-supply-1:310 (era fn names / call sites / code-landing), v-wave:5/:181/:200 (section refs).
- **Cite drift: none.** All `commit <sha>` cites in scope resolve (`27b6209`, `3d8fea0`,
  `1b32197`, `9c5aba7`, `3e4ba9d`); pin prefixes (`d4382fcd…`, `fdfadd5e…`, `495cc91a…`) are
  header/pin hashes, not commits — correctly era-labeled where cited. Only missing-file cite in
  the corpus is `docs/p2-oracle-1.md`, honestly marked "(in flight)" at STATUS:40. No doc claims
  H0=171 (that lived in `H0-CONTROL.md` integrator note, rc2 §1 convicts it, baseline owns it).
- STATUS.md:20/:40 — already caveated (7fb35d8/245d502 family); verified matching issue #8.

## 4. Live-lane + baseline rows (LISTED ONLY — sibling lanes own them)

`docs/p2-frames-spec-1.md` (FRMSPEC/live): :230 "PAL8 (0x0A09→9) / C4" frames spec,
:233/:237/:332 PAL8 legs — **the strongest phantom-gate risk left in the fleet**: builder-spec
lines that a future lane will paste as fact while the oracle is still in flight. Flagged for
the ORCL/FRMSPEC owner to cite issue #8 in-file. :101/:104/:194 render references are era call
sites (fine). :299/:308 hook-gated-no-genuine-bytes (fine).
`docs/p2-ladder7-play23.md:11` (L7P3/live): PLAY-1 LANDED legs — code-landing, accurate.
Baselines (stitch-apply8:24, rc2-recensus:32/:39/:103, apply7-recensus:13/:28/:32/:37) —
these ARE the ledger; untouched by law. apply7-recensus already carries the A7N erratum (b4fb249).

## 5. Survivor re-sweep

Post-edit run: control still 7/7; corpus 128→108 EDITABLE hits (17 corrected lines now carry
DISPUTED/self-consistency/200-200 wording; corrected lines still trip the grammar — expected,
the tokens remain with qualifiers). Programmatic qualifier scan (line + ±3 context, classes
{CORRECTED/RECEIPT/SUPPLY/FIXTURE/IRRELEVANT/AUDIT per §3}): **zero remaining RENDER/EXACT/LANDED
claims without backing qualifier except the adjudicated-ambiguous set in §3** (tail-3 BIT-EXACT,
gx-tex-consumer title/plan rows, fixture naming). Every genuine-DISPUTED statement in docs now
cites issue #8 or a line that does.

## 6. Repro
`cache/scratch/doct/{sweep.py,CONTROL.md,editable.tsv,sweep-pre.txt,sweep-post.txt,apply_edits.py}`.
Runner: `python3 sweep.py CONTROL.md` (asserts control fires all 7 categories before corpus scan).

## 7. POSTSCRIPT — sibling-event reconciliation (same wave, minutes after this sweep)

`6ac9f40` (lane ORCL, 13:02:08 — two minutes before correction commit `7be93ab`) landed
`docs/p2-oracle-1.md`: era witness `GXGetTexBufferSize` + caller leg pins **fmt 9 =
`GX_TF_C8` (CI8, 1 B/px)** and states the CI14 mapping of issue #8 is **refuted by its own
decisive witness — "the BB PAL8 label stands"**, with vision-read coherence on the C8-tiled
decode. Human-oracle gate is STILL PENDING (labeled board), so every DISPUTED marker this
commit applied remains correctly non-committal on decode truth — but the specific parenthetical
"⇒ CI14 class" I carried (prescribed by the card + issue #8 wording) is now the *losing* side
of the argument in `v-wave3:27`, `v-wave3:38`, `gx-tex-consumer:88`. Disposition: left as-is
per lane law (they cite issue #8 honestly; ORCL owns the supersede once the human gate closes;
DOCT does not edit another lane's verdict and issues were already told to integrator in §3).
Also ORCL convicts the manifest "I4" label for sega_e frames (0xE = CMPR) — a second
format-label item §3 flagged as ambiguous (`v-wave3:24`) is now live-lane-owned, untouched here.
