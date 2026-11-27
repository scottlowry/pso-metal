# P2-PUB — public community-readiness audit 1 (doc-consistency sweep + hygiene)

Fleet wave-4 lane PUB, card `t_85577c62`, 2026-11-27. ZERO builds, ZERO code
edits; decomp `pso-decomp @6b38b5383` READ-ONLY (never entered). Writes:
`README.md` (stale numbers only) + this doc + `docs/STATUS.md`.
Truth basis: `docs/p2-stitch-apply5.md` → `docs/p2-stitch-apply8.md` (apply-8
is the current era). Same-turn verifications this lane ran:

- waiver header `build/waivers/pso_assert_waivers.h` sha256
  `d4382fcd0659357f283e675726c1076338e909550c067513459280025e9f0eee` == pin
  `host/bringup/waiver-header.sha256` (byte-equal, zero regen);
  `build/waivers/waiver-status.txt` = `ACTIVE waived=2946 types=1549`
  (matches `docs/p2-stitch-apply7.md:51`).
- `tools/parse_gate.py` BASELINE on disk = `1605` (line 34) — matches
  `docs/p2-stitch-apply7.md:47`.
- All ledger arithmetic re-run in python this session (not head math):
  1,167+513=1,680; 485+9+2+17=513; 517−4=513; 21−4=17; 128+28=156;
  13+16+26+27+28+18=128; 1,578+25+2=1,605; 350+813=1,163; 1,163+4=1,167;
  169+258+80+2+5+3=517; 1,167/1,680=69.5%. Every check OK.

## 1. Contradictions vs the apply-7/8 ledger

### 1a. README (FIXED this wave — numbers only; AI banner untouched)

| was | line | now | ledger cite |
|---|---|---|---|
| suites "bring-up 128/128" | README.md:67 | **156/156** (prior 128 verbatim + frame-loop 28), receipts named inline | `docs/p2-ladder7-1.md:69` |
| residual classes "adapter-queue 169 · gated 260 · other-class tail 83 + TL census 19" | README.md:69 | apply-8 FAIL=513 first-error partition 485/9/2/17; 169-row queue + 258 gated stated as owned subsets | `docs/p2-stitch-apply8.md:24,36`, `docs/p2-tail-2.md:31-33,41`, `docs/p2-apply7-recensus.md:72-73` |
| "ladders 1–6" (layout) | README.md:79 | ladders 1–7 | `docs/p2-ladder7-1.md:17` |
| "(ladders 1-6)" (Building) | README.md:98 | (ladders 1-7) | `docs/p2-ladder7-1.md:69` |

README numbers verified NOT stale and left untouched: 1,605/1,680 + floor 1605
(:62 == `docs/p2-stitch-apply7.md:47` + disk gate), 1,167/1,680 (69.5%) +
empty collateral (:63 == `docs/p2-stitch-apply8.md:24`,
`docs/p2-stitch-apply7.md:48`), 1,258 full-stack stance (:64 ==
`docs/p2-stitch-apply4.md:11` — historical, cited in place), logo readback
76,800/76,800 (:18 == `docs/p2-v-wave.md:164`), 2,946/3,333 over 1,549 (:68,
disk read above), 8/8 link receipts (:66 == `docs/p2-stitch-apply7.md:60`),
decomp pin 6b38b5383 (:28).

### 1b. README prose flagged, NOT fixed (out of scope: numbers-only)

- README.md:110 roadmap "Now: … rung-5 seam ladder" — rung-5 PLAY-1 has
  landed (`docs/p2-ladder7-1.md:1`); Plays 2–3 remain (`:109`). Roadmap is
  still honest ("seam ladder" continues), but the phrase now undersells the
  tree. Integrator call at next README pass.
- README.md:27 "(`zeroz41/pso-decomp` … 100% matched)" — external claim about
  upstream; this repo's ledger has no line certifying it. Not contradicted,
  just uncited.

### 1c. Other docs / files asserting superseded values (flag; NOT touched)

These are dated receipts (historical record — ledger law keeps them as-run),
listed so a reader never mistakes them for current state:

- docs/p2-tail-1.md:43 — "unattributed after exclusions = 19" (apply-6 era);
  superseded by `docs/p2-tail-2.md:41` (21 @ apply-7; declares tail-1
  superseded at `:19`) and `docs/p2-stitch-apply8.md:36` (17). The 19 was
  itself internally inconsistent — buckets summed 16 (`docs/p2-apply7-recensus.md:75,85-88`).
- docs/p2-njs-adopt-1.md:65 — "other-class (NOT NJS) 83" — stale by 3
  post-apply-7: real 80 (`docs/p2-apply7-recensus.md:73,89`).
- tools/parse_gate.py:17 (docstring) — LP64 ledger stops at "1,163";
  apply-8's 1,167 (`docs/p2-stitch-apply8.md:24`) not reflected. CODE file —
  out of this lane's write scope.
- build/README.md:13 — "162 -> 261 -> 350 PASS" stops at apply-6; official
  parse-lp64 is 1,167 (`docs/p2-stitch-apply8.md:24`). `build/` — out of scope.
- docs/p2-stitch-apply6.md:53 — "350 (21%)": 350/1,680 = 20.8% (python).
  Rounding stated as if exact; cosmetic.
- docs/p2-handle-program-1.md:444 — re-run instruction cites "bringup 128/128";
  written pre-ladder7, would read 156/156 today (`docs/p2-ladder7-1.md:69`).
- docs/p2-stitch-apply5.md:55 / apply6:31 / apply7:47 — arm64_32 1,576→1,601→1,605
  chain: each correct at its date, apply-7 holds (`:47`); no fix.
- docs/p2-collision-debt.md / p2-stitch-apply4.md collateral {GcrnaStream.cpp}
  (`docs/p2-stitch-apply4.md:13`, `docs/p2-stitch-apply5.md:60`) — dead era:
  collateral = ∅ since apply-7 (`docs/p2-stitch-apply7.md:48`,
  `docs/p2-apply7-recensus.md:37-39`). Timestamped receipts; no fix.

## 2. Good-first-issue derivation

7 contributor-sized tasks derived from public-safe facts only (repo-visible
tools/tests/docs; no disc paths, no fleet/lane vocabulary). Full text in
`docs/STATUS.md` §5. Every task is runnable with the contributor's OWN
by-you-decomp checkout or no checkout at all (tests/tools are stdlib+clang).

## 3. Public hygiene — full-history leak sweep (python over every blob)

`git rev-list --objects --all` (154 commits, 1,049 objects, **399 blobs
ever**) streamed through `git cat-file --batch`, regex-scanned
(`cache/scratch/pub/leak_sweep.py`, stdlib, reproducible):

- Disc-path patterns — `/Volumes/`, `.iso`/`.gcm`/`.dol`/`.elf` filenames,
  Windows drive letters, disc-image mount-name+disc/iso collocations,
  rip/dump/backup directory prefixes: **ZERO hits. CLEAN.**
- Commit messages (all refs): same disc patterns → **zero hits**.
- Binary blobs (content with NUL bytes): **4 total**, all documented
  synthetic micro-fixtures under `host/bringup/fake_disc/`
  (`logo_e.prs` 9 B, `gsl_tab.dat` 144 B, `gsl_tab2.dat` 192 B,
  `sinbd_tex.gvr` 1,052 B) — each with per-file provenance in
  `host/bringup/fake_disc/README.md` (hand-constructed / generator-
  reproducible; `sinbd_tex.gvr` regenerates via `gen_gvr_fixture.py`).
  **No genuine asset bytes committed, in any historical version.**
- Aside (non-leak, cosmetic): 29 blobs reference the project's own working-
  copy paths under the maintainer's home directory (`/Users/<user>/Hermes/…`,
  lane-scratch dirs), not disc paths; `local_assets/README.md` states the
  disc path is never recorded.
  Consider normalizing to `$REPO`-style placeholders in a future doc pass.

### 3a. `.gitignore` verdict — PASS

`.gitignore:6` `host/bringup/local_assets/` ignores the directory and
everything under it: `git check-ignore -v host/bringup/local_assets/<x>`
confirms; `git ls-files host/bringup/local_assets/` = exactly one tracked
file (`README.md` — the deliberate provenance note, committed with the
ignore entry itself, no bytes). `/cache/` (`:3`) keeps lane scratch out.
No other `local_assets/` dirs exist in the tree. Genuine extracted art
(e.g. the 492,848-B `logo_e.gvm`) appears ONLY as size+fnv receipts in docs
(`host/bringup/local_assets/README.md`, `docs/p2-logo-e-supply-1.md`).

## 4. Disposition

One commit, staging exactly `README.md`, `docs/STATUS.md`, and this file.
No builds, no code, no other docs touched.
