# pso-metal

Native recompiler port of **Phantasy Star Online Episode I & II Plus** (GameCube) for
Apple Silicon, rendering through **Metal**.

Status: **Phase 1 complete; Phase 2 underway — the port's first era game logic executes on
Apple Silicon** (PRS decoder from the decomp's own `prs.cpp`, run under a bring-up substrate
and golden-checked against the original PPC disassembly; see `docs/p2-bringup-1.md`). The
first-pixel lane (GX→Metal window, logo-frame subset) is in progress. Nothing playable yet.

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

1. **Data model.** The GameCube is 32-bit PowerPC; Apple Silicon runs LP64. Measured both
   ways (`probes/`, `docs/abi.md`, `docs/p2-lp64-tranche1.md`): ILP32 (`arm64_32`) makes
   **1,416/1,680** files parse and resolves word/pointer size — but **cannot link on macOS**
   (no 32-bit arm libSystem), so it is a probe target only. The real runtime is **LP64:
   70/1,680 parse today**, because pointers widening to 8 bytes breaks ~1,444 layout asserts
   the ILP32 waiver list doesn't cover. This is the central open work: a per-type
   FORMAT-KEEP vs RUNTIME-WAIVE verdict queue (tranche 1 done: 80 waivable / 4 locked / 116
   unresolved), ceiling-simmed at 1,544/1,680 once all layout waivers are classified, with a
   residual 136 blocked by non-layout causes (SDK C-dialect headers, MusyX tag collisions,
   PPC intrinsics). Runtime absolute-address / u32-as-pointer patterns are handled by an
   arena + handle model (`docs/p1-datamodel.md`, proven in the bring-up).
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
| `compat/` | force-include headers: era-parse shims + the byteswap layer (`compat/RESULTS.md`) |
| `probes/` | reproducible tree-wide clang probes that drive the numbers |
| `docs/` | per-area surveys + the integration ledger: `abi`, `host-surface`, `boot-spine`, `audio-survey`, `be-index`, `p1-*` (formats, build-system, init ledger), `p2-*` (LP64 tranche, bring-up log) |
| `host/` | the re-hosted platform layer: N's 274-entry API scaffold (`pso_host.h` + subsystem headers, effort-tagged) + Q's executing bring-up substrate (`host/bringup/`) |
| `spike/` | runnable proofs: Metal offscreen clear+readback (green), data-model handles (green) |
| `tools/` | `be_scan.py` endian hazard index, `parse_gate.py` regression gate; content locator (planned — reads your own ISO, writes nothing) |
| `build/` | port build skeleton + fail-closed assert-waiver overlay + parse-regression gate |
| `data/` | machine-readable indices (hazard CSV, assert verdicts, verdict tranche 1) |

## Benchmarks

**What's green today** (each re-run by the integrator, not just lane-reported):

| Proof | State |
|---|---|
| Era logic executing | `PrsDecoder::prs_decompress` (+6 helpers) compiled from decomp source decodes on arm64; **16/16 golden vectors hand-traced from the original PPC `prs.s`** — `host/bringup/run.sh` exit 0 |
| Parse gate (probe target, arm64_32) | **1,416/1,680** (`make parse`), regression-gated at the 669 floor |
| Parse at real runtime target (LP64) | **70/1,680** — honest baseline for the verdict queue (see Data model) |
| Assert waivers | fail-closed generator, 99/3,317 waived, FORMAT-root deny-list live (catches mislabeled save/network roots; proven end-to-end) |
| Endianness layer | `compat/pso_byteswap.h` two-namespace API; **38/38 tests** incl. UBSan; cipher swap-around-cipher pinned |
| Metal | headless clear+readback proof green (`spike/`); window slice (GX-subset) in progress |
| Boot plan | 75-action init ledger, 23-step min-viable logo checklist (`docs/p1-init-ledger.md`) |

Phase 2 exit: PSO title screen rendered by Metal on Apple Silicon, from a **linkable LP64
build** reading your own dump. Game benchmark: a created character walking in CotL at
unlocked framerate. Estimates, revised once against the LP64 measurement: linked executable
~4–6 weeks of fleet-paced verdict+dialect work; title screen ~2–3 months.

## License

MIT (matching upstream pso-decomp). Game content is never included.
