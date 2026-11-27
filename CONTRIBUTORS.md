# Contributing to pso-metal

pso-metal is a native recompiler port of *Phantasy Star Online Episode I & II Plus*
(GameCube) to Apple Silicon/Metal. It is built **on top of** the decompilation
[`zeroz41/pso-decomp`](https://github.com/zeroz41/pso-decomp), which it consumes as a
pinned, read-only input — it is not a fork of it and does not re-house its code.

**Please read [`AGENTS.md`](AGENTS.md) too.** This project is coordinated as a fleet of AI
agents under human direction; if you are contributing with (or as) an AI agent, that file is
binding on how you work here. If you are contributing by hand, everything below still applies.

## What this project is trying to do

Make PSO's *own decompiled code* execute natively on a machine that was never a GameCube:
recompile the game's C/C++ for arm64, re-host the PPC platform services (GX graphics, DSP
audio, OS/DVD) on Metal / AVAudioEngine / POSIX, and prove every step with byte-truth gates.
It is **"the game, rebuilt," not an emulator**, and it is **not a decompilation** — matching
the era binary is upstream pso-decomp's job; making that match *run here* is ours.

## Before you start

1. **Read the README** — the "approach" and "where the numbers actually are" sections are
   the honest current state.
2. **Read the relevant `docs/p2-*.md` receipt** for whatever area you're touching. The docs
   directory is the working ledger; every claim has a re-run command behind it. Do not
   re-derive something a doc already settled, and do not contradict a doc without updating it.
3. **Set up the pinned input.** Point `DECOMP_ROOT` at your own pso-decomp checkout. The
   suites pin that checkout by sha and fail loud on drift — that is by design.
4. **Open an issue (or a draft PR) early** if your change is more than a typo fix. This repo
   moves fast with coordinated agents; a one-line "I'm taking class X / mechanism Y" prevents
   two people doing the same work or stepping on a shared file.

## The rules that make a contribution landable

- **Bring your own content.** No ROMs, ISOs, game assets, or ripped bytes in any commit,
  ever. Test fixtures under `host/bringup/fake_disc/` must be *synthetic* and
  generator-reproducible — regeneration has to be byte-identical, or it doesn't merge.
- **Cite evidence by location.** Any factual claim in a PR (a size, an address, a behavior)
  points at code: `path:line` at the current pin, or a tool output re-runnable by the
  reviewer. "From memory" or "the original does X" is not evidence.
- **Prefer verbatim slices to rewrites.** Game logic comes from the pinned decomp by guarded
  generators, not hand-authored ports. If you must add host-side glue, it lives in a
  prelude/seam file, clearly marked, never silently inside a verbatim era body.
- **Respect the two ABIs.** `arm64_32` is a parse-probe target only; LP64 is the runtime. A
  fix for one is not automatically valid for the other; say which leg your evidence covers.
- **Parse-clean ≠ runtime-correct.** A passing `-fsyntax-only` count is not a claim that the
  code runs right. Don't sell a parse win as a runtime win; name the follow-on program.
- **Keep FORMAT-KEEP types honest.** Structs that cross a byte boundary (saves, network
  packets, disc images) stay layout-true; they are protected by a deny-list you cannot
  waive around. If you think a type is misclassified, fix the verdict corpus upstream —
  don't force a flag.
- **The tree ends green.** Before any PR is ready: `bash host/bringup/run.sh` and
  `bash host/gx/run_window.sh` pass, `python3 tools/parse_gate.py` is at/above its floor,
  and there are no unexplained working-tree changes.

## PR process

- **Fork → branch → PR against `main`.** Branch names describe the work (`fix-gx-viewport`,
  `class-musyx-tail`, not `patch-1`).
- **One coherent piece per PR.** Small and reviewable beats large and bundled.
- **The PR body must contain:** what changed, *why* (the problem/measurement), how it was
  verified (exact commands + their output/delta), what it does **not** prove (open questions),
  and the AI-disclosure line from AGENTS.md §0 if applicable.
- **A human reviews and merges.** The maintainer (Scott) and/or the upstream pso-decomp
  community own what lands. Expect requests for evidence, not rubber-stamps.
- **CI / gates print numbers.** "It compiles locally" is not a green check; the regression
  floors and receipt suites are the contract.

## What to work on

The `docs/p2-collision-debt.md §4` and `docs/p2-stitch-apply6.md` tables enumerate the
open residual classes (NJS-handle, packet KEEP-asserts, PppFrame/net C, MWCC-asm dialect,
the long tail) with owners and status. Good first contributions are the **survey** tasks —
enumerate + sub-classify a class and file a receipt doc with paste-ready, *not-yet-applied*
asks — because they're well-scoped, high-value, and don't require touching shared build
machinery. Pick an unowned class, say which, and go.

## Conduct & licensing

- Be precise and be kind. Disagreements get settled with evidence, not authority.
- MIT, matching upstream pso-decomp. Contributions are licensed under it. Game content is
  never part of the agreement because game content is never part of this repo.

## Getting help

Open an issue with the area you're in and what you're stuck on; link the doc receipt you're
working from. If you're an agent, your brief should already point at the right docs — if it
doesn't and you're guessing, that's a bug in the brief: say so.
