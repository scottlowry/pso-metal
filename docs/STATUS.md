# STATUS — pso-metal snapshot (2026-11-27)

Current truth era: battery **apply-8** (`docs/p2-stitch-apply8.md`), waiver
header `d4382fcd0659357f…` verified equal to its pin `host/bringup/
waiver-header.sha256` by same-turn read, decomp input pinned at
`6b38b5383` (read-only, bring-your-own). Every number below cites its ledger
line; where the receipt predates apply-8 the era is named. Consistency audit:
`docs/p2-public-audit-1.md`.

## 1. Meters

| meter | value | ledger cite |
|---|---|---|
| true-LP64 parse (official `make -C build parse-lp64`) | **1,167 PASS / 513 FAIL** of 1,680 (69.5%) | `docs/p2-stitch-apply8.md:24` |
| arm64_32 legacy parse anchor | **1,605 / 1,680**; regression floor 1,605 (`python3 tools/parse_gate.py`) | `docs/p2-stitch-apply7.md:47`; gate on disk `tools/parse_gate.py:34` |
| rebind collateral (regressions vs control battery) | **empty set** | `docs/p2-stitch-apply7.md:48`, `docs/p2-apply7-recensus.md:37` |
| executing suites (`bash host/bringup/run.sh`) | **200/200 PASS**, exit 0 | `docs/p2-ladder7-play2.md`, re-verified integrator 2026-11-27 |
| frame-loop test binary | **43/43 PASS** (28 PLAY-1 + 15 PLAY-2) | `docs/p2-ladder7-play2.md:29` |
| display-list replay | **210/210** samples, verify_failures=0 | `docs/p2-stitch-apply7.md:56` |
| logo window gates | era **12/12** + alt regime **10/10**; readback coverage + fnv anchor — **all self-consistency gates; genuine-record decode disputed (CI14), see issue #8** | `docs/p2-stitch-apply7.md:56-57`, `docs/p2-v-wave.md:164` |
| first-link boot slice | **8/8 receipts** (Mach-O) | `docs/p2-stitch-apply7.md:60` |
| waiver machine | **2,946 / 3,333** asserts over **1,549** types | `docs/p2-stitch-apply7.md:51`; disk `build/waivers/waiver-status.txt` |
| big-endian hazard index | 629 files carry ≥1 hazard (of 3,814 scanned) | `docs/be-index-summary.md` §intro |

Honesty law (see `AGENTS.md` §3): parse-clean means *compilable*, never
*runtime-correct*. The 1,167 is a parse meter; executed logic is the ladder
table below, nothing else.

## 2. What actually executes (rung table)

Era code enters as byte-verbatim slices from the pinned decomp tree via
fail-loud generators (`AGENTS.md` §2); host code supplies seams only.

| rung | what runs natively | green receipt |
|---|---|---|
| 1–2 | crt0/arena init, PRS decode of a synthetic archive entry | `host/bringup/run.sh` (156/156, `docs/p2-ladder7-1.md:69`); `docs/p2-bringup-1.md`..`p2-bringup-3.md` |
| 3 | GSL archive table walk (era `find_gsl_entry` verbatim) | same run.sh; `docs/p2-tex-supply.md` |
| 4 | GVRT/GVPL texture supply: synthetic 32×32 PAL8 GVR walked by the era chunk finder | same run.sh; `docs/p2-tex-supply.md`, commit-era note in `docs/p2-public-audit-1.md` §1c |
| 5 | texture consumer chain (Ginja table → upload doorstep) | same run.sh; `docs/p2-gx-tex-consumer.md` |
| 6 | logo driver: era CLUT supply chain end-to-end into a Metal drawable, pixel-readback gated — **caveat:** gates are self-consistency proofs; genuine-record decode is disputed (CI14 vs PAL8 label), see issue #8 | `docs/p2-v-wave2.md`, `docs/p2-v-wave3.md:38,57-77` + erratum in `docs/p2-oracle-1.md` (in flight) |
| 7 | title frame loop: five promoted `TLogo::run_frame` legs execute era bodies verbatim, order-matched against the era source, with flip probes reading real era stores back | `docs/p2-ladder7-1.md` §1-2 (28/28) |
| boot slice | first linked LP64 Mach-O: era PRS/heap/DVD chain, 8/8 receipts | `docs/p2-stitch-apply7.md:59-60`, `docs/p2-first-link.md` |

## 3. Known gaps by class (counts python-verified this session; the
partitions sum exactly: 485+9+2+17 = 513 = FAIL total)

| class | count | what it blocks / status | ledger cite |
|---|---|---|---|
| NJS handle wall (`NJS_OBJECT_size` asserts) | **485** | the big parse blocker: pointer-in-u32 era regime; ~805 TUs already absorbed, remaining need adapter edits + the address-stamping (handle) program | `docs/p2-tail-2.md:31` |
| — subset: enumerated adapter-queue | 169 rows / 673 sites | fix shapes authored as paste-ready diffs; batch-1 top-30 files supplied (164 sites, probe-verified before/after) | `docs/p2-adapter-queue-1.md:28,49-50` |
| — subset: runtime-gated rows | **258** | parse-adopted, runtime correct only after the handle program; ledger-tagged | `docs/p2-apply7-recensus.md:72` |
| FloorPopulation layout asserts | **9** | one named struct family, verdict-owned | `docs/p2-tail-2.md:32` |
| MusyX audio TU residues | **2** | named files, body-level asm needs host facade supply, not waivers | `docs/p2-tail-2.md:33`, `docs/p2-apply7-recensus.md:74` |
| named tail | **17** | 5 new-wall-behind TUs, 3 inline-asm ports specced (2 flip whole TU), 4 one-line network decl alignments specced 4/4 probe-clean, 3 handle-dialect, 2 layout/ABI verdict asks | `docs/p2-stitch-apply8.md:36`, `docs/p2-tail-2.md:34-40,75-111,126-192` |

Beyond parse: runtime correctness for stamped pointers, link scale-up from
the boot slice toward real frame states, and audio host-shim are separate
programs — see README Roadmap.

## 4. How to pick a unit of work

1. Read `AGENTS.md` (rules) and `CONTRIBUTORS.md` (PR contract) first; both
   are binding for PRs.
2. Pick a class from §3 you can hold in your head; open its cited ledger doc
   and reproduce its *measurement command* on your machine (you need your own
   legally-obtained `pso-decomp` checkout for anything that compiles era
   code — see README "Building").
3. Small & self-contained → §5 list below. Class work → start with authored
   one-liners (the 4 network declaration alignments, the 2 specced asm-port
   flips) before anything touching the 485-row wall.
4. Verify by delta, never by file existence (`AGENTS.md` §1); keep the whole
   suite green (`bash host/bringup/run.sh`); one coherent piece per PR.
5. This repo is often worked by several agents at once: builds and generated
   files are serialized by the maintainer — if your PR touches `build/`
   generated outputs or the waiver pin, say so in the PR text.

## 5. Good first issues

All runnable with a contributor's own checkout; none needs game content —
fixtures are synthetic by design (`host/bringup/fake_disc/README.md`).

1. **Extend the byte-swap test suite.** `tests/pso_byteswap_test.c` carries
   38 `CHECK` invocation lines (counted python-grep this session; 38 checks
   at authoring, commit `1b32197`); add round-trip/unaligned cases for any
   accessor not pinched by an existing check, incl. the swap-around cipher.
   Must stay green under `-O0/-O2/UBSan` via `bash tests/run_byteswap.sh`.
   Pure host tooling, no era code.
2. **Make the pin guards prove they can fail.** Add a negative test that
   plants a wrong header hash against `host/bringup/waiver-header.sha256`
   verification and asserts a non-zero stop (the manual canary is recorded in
   `docs/p2-stitch-apply6.md` §"link-GUARD"). Rationale: `AGENTS.md` §1 —
   "a gate that cannot fail is decoration."
3. **Unit-test the parse-floor gate.** `tools/parse_gate.py` is stdlib-only
   but its summary-regex/floor math has no tests; add pytest-free asserts for
   `TOTAL=/FAIL=/PASS=` parsing and baseline arithmetic with synthetic summary
   fixtures. CI-friendly.
4. **Fixture-tree regression test for the hazard scanner.** `tools/be_scan.py`
   indexes big-endian hazards (`docs/be-index-summary.md`, 5 categories);
   add a tiny synthetic source tree pinning one hit per category plus the
   risk-formula edges, so scanner changes can't silently lose a pattern.
5. **Fixture regeneration verifier.** `host/bringup/fake_disc/README.md`
   promises byte-identical regeneration; wire a check that runs
   `python3 host/bringup/fake_disc/gen_gvr_fixture.py` into a scratch copy
   and `cmp`s the emitted fixture against the committed bytes, as a script
   others can call in CI.
6. **Shell hardening pass.** `bash -n` + `shellcheck` clean the suite
   runners (`host/bringup/run.sh`, `host/gx/run_window.sh`,
   `host/gx/run_vi.sh`, `tests/*.sh`), quoting all word-splitting sites;
   output lines' formats are parsed by gates, so keep stdout byte-identical.
7. **Newcomer glossary doc.** One page defining, plainly and with links: the
   two data models (ILP32 probe vs LP64 runtime), layout waiver, header pin,
   format-keep types, verbatim slices, fail-loud guards — the README
   "approach" section assumes them. Docs-only PR.

*Every item above was phrased for public readers: no personal paths, no
game content, no internal process vocabulary.*
