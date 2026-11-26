# pso-metal

Native recompiler port of **Phantasy Star Online Episode I & II Plus** (GameCube) for
Apple Silicon, rendering through **Metal**.

Status: **Phase 0 complete — port viable, no blocker found** (`docs/phase0-verdict.md`).
Phase 1 (build system, format-boundary audits, boot slice) in progress. Nothing playable
yet; the compile-gap is now decomposed into named, owned work items.

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

1. **Data model.** The GameCube is 32-bit PowerPC; Apple Silicon runs LP64. Measured
   position (`probes/`, `docs/abi.md`): ILP32 (`arm64_32`) makes ~40% of the tree parse and
   resolves word/pointer size — but **cannot link on macOS** (no 32-bit arm libSystem), so
   the runtime direction is LP64 + arena/handle translation for the game's absolute-address
   and u32-as-pointer patterns (spike in progress, `docs/p1-datamodel.md`). The dominant
   residual parse failures are the MWCC C++ object model (vptr placement, 12-byte
   pointers-to-member) — gated by ~20 headers, decided per-type as FORMAT-KEEP vs
   RUNTIME-WAIVE by the Phase 1 audits.
2. **Big-endian.** PSO's saves, network packets, and disc data are big-endian byte images.
   Measured, not vibes: `tools/be_scan.py` indexes 629 files carrying at least one
   byte-order hazard; genuinely dangerous live sites are few (4) and named in
   `docs/be-index-summary.md`. The audits underway split "layout that crosses a binary
   format" (must stay byte-true, swap-on-load layer) from "layout that only lives in RAM"
   (host-free). Endianness is bounded — the scary-sounding bitfield class turned out to be
   one header.
3. **Graphics.** Headless Metal is proven on this machine (`spike/metal_clear.m`: offscreen
   640×480 clear + pixel readback, CI-able, green). The port re-hosts the game's actual GX +
   MetaCore/NuSystem call surface — measured at **272 distinct SDK functions, 1,415 call
   sites, only 121 files touching platform code at all** (`docs/host-surface.md`). The
   decompiled game logic above it stays untouched.
4. **Audio.** The ADX decoder is already portable C in the decomp; the MusyX mixer is a
   6.6 KB DSP microcode blob with no CPU-side tap. Plan: keep the CRI CPU layer, shim the
   ~35 boundary calls onto AVAudioEngine (`docs/audio-survey.md`); ucode reverse-engineering
   stays optional for bit-exact fidelity later.

## Layout

| dir | contents |
|---|---|
| `compat/` | force-include header + shims that make era code clang-parseable (`compat/RESULTS.md`) |
| `probes/` | reproducible tree-wide clang probes that drive the numbers |
| `docs/` | `phase0-verdict.md` + per-area surveys (abi, host-surface, boot-spine, audio, be-index, p1-*) |
| `spike/` | runnable proofs: Metal offscreen clear+readback (green), data-model (in progress) |
| `tools/` | `be_scan.py` endian hazard index; content locator (planned — reads your own ISO, writes nothing) |
| `build/` | port build skeleton + fail-closed assert-waiver overlay + parse-regression gate |
| `data/` | machine-readable indices (hazard CSV, assert verdicts) |

## Benchmarks

Phase 0 exit (met): honest measurement of every failure class + a strategy per class.
Parse status today: **669/1680 game files parse with zero edits to decomp source**
(`make parse` gate), with ~1,580 projected once the per-header assert policy lands —
the remaining wall is 98.7% concentrated in ~20 headers. Endianness map: delivered.
Phase 1 exit: PSO title screen rendered by Metal on Apple Silicon. Game benchmark: a
created character walking in CotL at unlocked framerate.

## License

MIT (matching upstream pso-decomp). Game content is never included.
