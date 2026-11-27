# pso-metal

> ## ⚠️ Everything in this repository is AI-generated.
> Every line of code, every document, every commit — written by AI agents (a fleet of
> Hermes subagents coordinated by an integrator agent) under the direction and review of
> **Scott Lowry**. Humans have review authority; no human has typed a line of this code.
> PRs from agents: please follow [`AGENTS.md`](AGENTS.md). PRs from humans:
> [`CONTRIBUTORS.md`](CONTRIBUTORS.md).

Native recompiler port of **Phantasy Star Online Episode I & II Plus** (GameCube,
`GPOE8P`) for **Apple Silicon**, rendering through **Metal**. Built *on* — not forked
from — [`zeroz41/pso-decomp`](https://github.com/zeroz41/pso-decomp), consumed as a
pinned read-only input.

Status: **Phase 3 — the logo supply chain runs; the logo picture was broken — root cause found.**
Era texture-supply code (`init_and_load_tlut` → GVRT/GVPL chain → CLUT expand)
executes natively end-to-end in real era call order into a Metal drawable. The
genuine disc record's bytes and the CI8 palette label were always right; the era
container stores texels **TI 8×4-tiled** (decisive witness: `GXGetTexBufferSize`
in the era disassembly + `GXEnum.h:144-179`), and our readback gates — all
self-consistency proofs — never noticed the flat-vs-tiled addressing error.
Fix in flight, tracked in [issue #8](https://github.com/scottlowry/pso-metal/issues/8);
decode candidate board passed the machine coherence metric (0.276) and awaits
owner confirmation. The first linked LP64 Mach-O boots the era logic with 8/8 receipts.

## What this is / isn't

- **Not an emulator.** PSO's decompiled C/C++ is compiled natively for arm64; the PPC
  GX/DSP/OS service calls are re-implemented against Metal, AVAudioEngine, and POSIX.
  "The game, rebuilt" — not "the console, simulated."
- **Not a decompilation.** Byte-matching the era binary is
  [`zeroz41/pso-decomp`](https://github.com/zeroz41/pso-decomp)'s job (100% matched).
  This repo takes that decomp as a **pinned input** (currently `6b38b5383`) and makes it
  *run* on a machine that was never a GameCube.
- **Bring your own content.** No game assets, ROMs, or ISOs here, ever — not in any
  commit, past or future. Everything under `host/bringup/fake_disc/` is synthetic —
  hand-constructed micro-fixtures or generator-reproducible files (regeneration is
  byte-identical; see that directory's README for each file's provenance). Real game
  data stays on the owner's disk and is referenced by hash only.

## The approach (as measured)

1. **Two ABIs, honestly separated.** `arm64_32` (ILP32) reproduces the era data model as
   a *parse-probe target only* — it cannot link on macOS. The runtime is LP64, where
   every layout assert must be *earned*: a fail-closed per-type waiver machine
   (`build/gen_waivers.py`, 8-source verdict corpus, deny-root list that can never be
   overridden by a mislabeled row) neutralizes only RAM-only layouts. FORMAT-KEEP types
   (wire/save/disc byte images) stay compile-honest forever.
2. **Parse ≠ correct.** Passing `-fsyntax-only` under the rebind regime means
   *compilable*, not runtime-correct. Pointer-in-u32 sites (the Ninja handle regime)
   carry their own obligations: guest↔host address translation at stamped boundaries
   (`compat/njs/`, per-TU C++-only allow-list), loader stamping, and a handle program —
   tracked, not hand-waved.
3. **Executing code arrives as verbatim slices.** Era functions are extracted
   byte-verbatim from the pinned decomp tree at build time by guarded generators
   (anchor checks fail loud on drift); host code supplies prelude seams, never
   re-authored game logic. Goldens are hand-traced from the era PPC disassembly so
   tests cannot share bugs with the ported code.
4. **Big-endian bounded.** 629 hazard files indexed (`tools/be_scan.py`); wire/save
   bytes stay byte-true through a proven swap layer (`compat/pso_byteswap.h`, 38/38
   incl. UBSan); live dangerous sites: few and named.

## Where the numbers actually are (2026-11-27, all integrator-run)

| meter | value |
|---|---|
| arm64_32 parse gate (legacy anchor) | **1,605 / 1,680** — `tools/parse_gate.py` floor 1605 |
| true-LP64 parse, official adopted shape | **1,167 / 1,680 (69.5%)** — `make -C build parse-lp64`; collateral-vs-control now the **empty set** |
| true-LP64 parse, full-stack battery stance | **1,258 / 1,680** (measured `docs/p2-stitch-apply4.md`; adoption gated on the handle/adapter programs) |
| logo | era CLUT supply chain **executing**; genuine 256×256 record byte-verified end-to-end, palette label (CI8) correct per era `GXEnum.h`/`GXGetTexBufferSize` disassembly witness — **rendering defect root-caused to flat-vs-TI-8×4 texel addressing** (+ RGB5A3 444-vs-555 leg), machine-coherent fixed decode awaiting owner eyeball; gates so far are self-consistency proofs, coherence gate being added — issue #8, `docs/p2-oracle-1.md` |
| boot | first linked Mach-O, **8/8 receipts**, era PRS/heap/DVD chain end-to-end |
| suites | bring-up **200/200** (ladders 1–7 PLAY-2 + HG-1 handle golden) · display-list 210/210 samples · window era 12/12 + AV-1 10/10 (self-consistency gates — see logo row) · VI both branches · link 8/8 — measured `docs/p2-ladder7-play2.md`, `docs/p2-stitch-apply7.md` |
| waiver machine | 2,946 / 3,333 asserts over 1,549 types, sha-pinned (D8) across three verify guards |
| residual classes (measured, owned) | apply-8 LP64 FAIL=513, first-error partition: NJS wall 485 · FloorPop 9 · musyx 2 named · named tail 17 (`docs/p2-stitch-apply8.md` §Battery, `docs/p2-tail-2.md` §1); inert supply authored: adapter-queue 169 rows/673 sites batch-1 top-30 (`docs/p2-adapter-queue-1.md` §0-1), runtime-gated handle rows 258 (`docs/p2-apply7-recensus.md` §3) · packets/PppFrame/MWCC-asm/musyx-main all landed |

The `docs/p2-*.md` series is the working ledger — every claim above has a receipt file
and a re-run command. "Almost matches" does not count here; gates print numbers or the
change doesn't land.

## Layout

| dir | contents |
|---|---|
| `host/bringup/` | executing bring-up ladders 1–7 (crt0/arena → PRS → GSL/GVRT → texture supply → consumer chain → logo driver → frame-loop legs) + generators + fake_disc fixtures |
| `host/gx/` | GX→Metal re-host: display-list replay, window/VI regimes, logo driver + readback gates |
| `host/link/` | first-link boot slice (Mach-O receipts) |
| `compat/` | parse-world machinery: collision pin (`collision/`), era-type rebinds (`repin/`), NJS handle shadows (`njs/`), era-model shadows (`net/`, `packets/`, `musyx/`, `dialect/`, `chorus/`), byteswap layer |
| `build/` | parse batteries (both ABIs), fail-closed waiver generator + per-TU NJS allow-list, gates |
| `data/` | verdict corpus TSVs (the waiver machine's authority), hazard indices, link-deps |
| `docs/` | the ledger: `p1-*` (formats/abi/build-system/init-ledger), `p2-*` (per-lane receipts, stitch apply series, contracts) |
| `probes/ spike/ tools/ tests/` | measurement runners, proofs-of-feasibility, gates (`parse_gate.py`, `link_hygiene.sh`, `be_scan.py`), byteswap tests |

## Building

Prereqs: Apple Silicon, Xcode clang, python3.9+. This repo **does not contain the game
code** — point it at your own [`pso-decomp`](https://github.com/zeroz41/pso-decomp)
checkout (the decomp itself requires your own legally-obtained disc to produce):

```sh
make -C build parse DECOMP_ROOT=/path/to/pso-decomp   # arm64_32 legacy gate
make -C build parse-lp64 DECOMP_ROOT=/path/to/pso-decomp  # true LP64 battery
python3 tools/parse_gate.py                            # regression floor
bash host/bringup/run.sh                               # executing-logic suites (ladders 1-7)
bash host/gx/run_window.sh                             # logo-visible gates, both regimes
make -f host/link/link.mk run                          # first-link boot slice receipts
```

The suites pin the decomp input by sha (AB manifests) and **fail loud on drift** — that
is a feature; see `docs/p1-buildsystem.md` and `docs/p2-stitch-apply*.md`.

## Roadmap (honest)

1. **Now**: genuine disc-supplied logo through the era archive-index; rung-5 seam ladder
   (title-screen main-phase machine); long-tail class census.
2. **Next**: uaddr-handle program (parse-clean → runtime-correct), link scale-up from the
   boot slice toward real game frame states, MusyX host-shim audio.
3. **Later**: title → char-select → first area. Playable single-player is the milestone;
   online is a separate program. No dates promised.

## License

MIT (matching upstream pso-decomp). Game content is never included.
