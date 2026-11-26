# pso-metal

Native recompiler port of **Phantasy Star Online Episode I & II** (GameCube) for
Apple Silicon, rendering through **Metal**.

Status: **Phase 0 — portability spike.** Nothing runs yet. The goal of this phase is to
measure and chip away at the GameCube-ABI → Apple-ABI gap, not to boot the game.

## What this is / isn't

- **Not an emulator.** PSO's own decompiled C/C++ is compiled natively for arm64; the
  original PPC GX/DSP/OS service calls are re-implemented against Metal, AVAudioEngine,
  and POSIX. Think "the game, rebuilt," not "the console, simulated."
- **Not a decompilation.** The decomp that makes this possible is
  [`zeroz41/pso-decomp`](https://github.com/zeroz41/pso-decomp) — 27,572 functions,
  100% byte-matched, MIT-licensed. This repo vendors/links against it; it does not
  fork its contents into a separate identity.
- **Bring your own content.** No game assets, ROMs, or ISOs here, ever. You supply your
  own legally-obtained PSO GPOE8P dump; a locator tool reads it locally.

## The approach

Modeled on what the community proved feasible for other GameCube titles (Super Mario
Sunshine on Apple Silicon being the visible 2026 milestone): decompile → recompile
natively → re-host platform services on Metal. PSO's specific twists:

1. **ILP32 data model.** The GameCube is 32-bit PowerPC; Apple Silicon is LP64.
   A probe (see `probes/`) shows a single compat header + `-fms-extensions` collapses
   the vast majority of clang failures, and arm64's ILP32 mode resolves struct-size
   asserts. Decision pending, documented in `docs/abi.md`.
2. **Big-endian.** PSO reads its own `.bbs`/`.ndo`/model data as native BE. Every asset
   loader and save path needs explicit byteswap — the deepest and most interesting
   work, and the endianness audit doubles as a decomp quality signal.
3. **Two renderers, one codebase.** PSO uses MetaCore's GX-on-NuSystem (`C_XBFONT`,
   N-JPEG) plus raw GX in places. The Metal backend wraps the decompiled GX layer;
   the decompiled game logic is untouched.

## Layout

| dir | contents |
|---|---|
| `compat/` | force-include header + shim headers that make era code clang-parseable |
| `probes/` | the tree-wide clang probes that drive Phase 0 decisions (reproducible) |
| `docs/` | abi.md, be-audit.md, decisions.md |
| `host/` | (later) SDL-free macOS host: Metal renderer, AVAudio, save/mmap layer |
| `tools/` | (later) content locator — reads the user's own ISO, writes nothing |

## Benchmarks

Phase 0 exit: **≥ 90% of `src/` units compile natively on arm64 with zero edits to
upstream decomp source** (all deltas confined to `compat/`), and a documented endianness
work map. Phase 1 exit: a PSO-created character walking in CotL, rendered by Metal, at
unlocked framerate.

## License

MIT (matching upstream pso-decomp). Game content is never included.
