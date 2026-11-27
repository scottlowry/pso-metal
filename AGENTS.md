# AGENTS.md — guidance for AI agents working on pso-metal

This repo is **entirely AI-generated** (see the README banner) and is built by a fleet of
agents coordinated through a human-directed integrator model. If you are an AI agent
(Claude Code, Codex, Hermes, Cursor, aider, or anything else) contributing, this file is
for you. It is not optional reading — the rules below exist because breaking each one has
already caused a real, measured incident in this project's history.

Follow `CONTRIBUTORS.md` for the human-facing contribution contract (PR shape, scope,
licensing). This file is the agent-specific overlay.

## 0. Disclosure — always, unambiguously

- Every PR you open or comment you post must state plainly that the work is **AI-assisted**
  and name the human who reviewed and submitted it. This repo's entire authorship is public
  as AI-generated; do not muddle it.
- Commit and PR metadata must reflect reality. Do not present agent output as human-authored.

## 1. Ground truth, always

The single most repeated lesson in this repo's ledger: **a claim is not a fact.**

- Every number you report (a pass count, a hash, a git tip, a diff size, an issue state)
  must come from a tool execution **in the same turn** you state it. Not from memory, not
  from a summary, not from a prior session's notes. Past session summaries are not evidence.
- **Verify gates by their delta, never by their output file existing.** A generator that
  wrote a file with rc=0 but changed nothing is a silent no-op. Diff before/after and count.
- **A gate that cannot fail is decoration.** Before trusting any new check (deny-list,
  collision screen, placeholder filter, canary), plant a known-bad input and watch it
  reject. (Real incident: an inverted deny-list printed "fail-closed" while waiving the very
  types it existed to protect.)
- **"Almost matches" is not progress.** The meters here are exact integers (parse PASS=N,
  byte-identical fixtures, exit-0 receipts) or they are not reported.

## 2. The hard boundaries (do not cross; ask a human first)

- **Never touch the game content boundary.** No ROMs, ISOs, assets, or ripped bytes go in
  any commit. `host/bringup/fake_disc/` holds only synthetic, *generator-reproducible*
  fixtures (regeneration must be byte-identical). If you find yourself reading an actual
  disc image to author a test, stop — reference it by hash, do not embed.
- **Never hand-edit a generated file.** `build/waivers/pso_assert_waivers.h` and the
  `host/*/gen/*.cpp` slice files are produced by generators with fail-loud drift guards.
  Edit the generator or its TSV inputs, re-run, and let the drift assertion certify you.
- **Never regenerate the waiver header casually, and never re-pin its sha without
  re-running every downstream verify guard** (`host/bringup/waiver-header.sha256` is the
  authority for three separate build verify-stamps; the link gate independently checks it).
  A sha bump with stale pins is a build that "passes" on the wrong era of corpus.
- **Never `git add -A` / `git commit -a`.** Stage only the exact paths you authored. This
  repo is worked by multiple agents in shared checkouts; sweeping flags commit a sibling's
  half-finished work under your name.
- **Never `--amend` before reading `git log -1`.** With nothing staged, `--amend` silently
  rewrites the tip commit (someone else's authorship and message, shifted hash).
- **Read before you overwrite.** `read_file` the current state of a file before
  `write_file`/patch. This checkout moves between your steps.

## 3. Domain invariants specific to this codebase

- **`pso-decomp` is a pinned, read-only input.** Compile/analyze against it; never write to
  it, never copy its contents into this repo as static files. If a rule or line you depend
  on lives there, cite it by `path:line` at the *current* pin and let the generators re-extract.
- **Parse-clean ≠ runtime-correct.** `-fsyntax-only` PASS means compilable, not that the
  code runs right on LP64. Pointer-in-u32 (Ninja handle) sites and cross-format byte images
  carry obligations the parse meter cannot see. Do not claim runtime confidence from a parse
  win; name the separate program that would earn it.
- **FORMAT-KEEP is absolute.** Types that cross a byte boundary (saves, wire packets, disc
  images) must stay layout-honest. The waiver deny-root list protects them; a verdict row
  that tries to waive one is a defect to fix upstream in the corpus, not a flag to force.
- **Include-edge laws (compat/).** The collision pin must come *after* `pso_metal_compat.h`
  and the waiver header in any `-include` list; `compat/repin` and `compat/njs` are `-I`-path
  only, never `-include`. `-Icompat/njs` (the C++ handle-shadow) is per-TU allow-listed and
  **never on a `.c` TU** (C silently launders pointer↔u32 under `-w`). See
  `docs/p2-collision-debt.md §5` and `docs/p2-njs-repin-1.md §6`.
- **Two ABIs are separate worlds.** `arm64_32` is a probe target (cannot link on macOS);
  LP64 is the runtime. A flag change intended for one is not automatically valid for the
  other — the rebind/pin program is measured LP64-only for exactly this reason.

## 4. Method

- **Understand before you edit.** Read the relevant `docs/p2-*.md` receipt and the actual
  headers/sources first. Address literals and behavior must come from the code in front of
  you, not from prior knowledge of the console or the game.
- **Time-box exploration.** If a match or mechanism hasn't progressed after ~10 build
  iterations, stop, record your findings as a doc/comment, and hand it back rather than
  sweeping variants hoping one sticks. (This repo already banked that as doctrine upstream.)
- **The tree must end green.** Revert experimental edits, re-run the suites
  (`bash host/bringup/run.sh && bash host/gx/run_window.sh`), and leave no unexplained
  working-tree changes before you stop.
- **One coherent piece per PR.** A PR is "one class surveyed", "one mechanism landed", "one
  doc receipt" — not a bundle of unrelated cleanups.

## 5. Naming and evidence

- Names come only from executable evidence (call sites, byte traces, format records).
  Otherwise keep `fn_`/`lbl_`/`sub_` placeholders. A wrong confident name is worse than an
  honest placeholder — it propagates.
- Any identifier you quote (a sha, an address, a report figure) must be pasted from live
  command output in the same turn, labeled with its kind. Do not reconstruct an identifier
  from memory; conflating an md5 with a git-sha, or a container hash with a payload hash,
  has sent agents on expensive wild goose chases here.

## 6. If you get stuck or unsure

- Prefer `fn_/lbl_` placeholders and a documented open question over an invented answer.
- Surface conflicts and doubts explicitly in the PR description; do not quietly paper over
  them. Honesty about what a change does *not* prove is a feature of this repo's culture.
- When a rule in this file conflicts with what you were about to do, the rule wins; if you
  believe the rule is wrong, say so in the PR and let the human adjudicate — do not silently
  deviate.

---
_This AGENTS.md is itself AI-authored and maintained as part of the repo's working doctrine.
Propose changes by PR with the same evidence discipline it asks of you._
