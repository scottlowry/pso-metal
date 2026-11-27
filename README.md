# pso-metal

Native recompiler port of **Phantasy Star Online Episode I & II Plus** (GameCube) for
Apple Silicon, rendering through **Metal**.

Status: **Phase 2 — first linked executable achieved.** A real LP64 Mach-O (`host/link/`)
containing era game logic boots a scripted slice on Apple Silicon (arena, era OSAlloc heap,
fake-disc reads, the full PRS decode chain, irq-gate, `memory.cpp` sys-heap — 8/8 receipts,
exit 0). First real pixels are readback-proven through a Metal window, the logo state machine
runs to its last pure-logic state, and a textured display-list quad is in bring-up.

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
   ways (`probes/`, `docs/abi.md`, `docs/p2-lp64-tranche1.md`, `docs/p2-census-audit.md`):
   ILP32 (`arm64_32`) makes **1,416/1,680** files parse and resolves word/pointer size — but
   **cannot link on macOS** (no 32-bit arm libSystem), so it is a probe target only. The real
   runtime is **LP64: 70/1,680 parse with the initial waiver set**; an uncapped re-audit found
   the true failing population is **1,602 layout-assert types / 2,951 sites** — earlier
   censuses were halved by clang's error cap. The path is a per-type FORMAT-KEEP vs
   RUNTIME-WAIVE verdict queue (936+ types now named across tranches + resolutions; realized
   ~150 files and climbing with the tail) plus two structural programs the queue exposed:
   the **lane-J handle re-pin** (`NJS_OBJECT`/`NJS_MKEY_F` — loaded-image structs that every
   boot-spine TU touches and waivers must never cover) and **`TItemData` save-layout
   re-pin**. Runtime absolute-address / u32-as-pointer patterns are handled by an arena +
   handle model (`docs/p1-datamodel.md`, proven executing).
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
| **First linked executable** | `host/link/out/pso_first_link` — Mach-O arm64, 15 objects (10 boot-spine + 4 whole-file era TUs + backings), 0 non-system undefined, boot slice **8/8 receipts exit 0** incl era PRS chain end-to-end (`docs/p2-first-link.md`) |
| Era logic executing | 30+ named era functions on M-series: PRS full chain (`prs_decompress` + `_buffer`/`_alloc`), all 10 `memory.cpp` heap fns + era OSAlloc port, `find_gsl_entry`, logo state machine to **state 7** = last pure-logic state (GX wall: `begin_gx_frame` @0x8013F5F4); suites 16+13+26+27 green on clean rebuilds |
| First real pixels | Metal window (640×480) readback-proven on era `GXSetCopyClear` literals; 41 boot-path GX/VI fns backed (20 real/21 counted shims); VI progressive branch proven on byte-verbatim era slices; textured quad in bring-up (`cy>0` texel bug under active fix) |
| Texture supply | Synthetic GVR fixture through the era decode chain (both-regime GVRT swap trap executed), era-shaped descriptor pack ready for the gx consumer (`docs/p2-tex-supply.md`) |
| Parse gate (probe target, arm64_32) | **1,416/1,680** (`make parse`), regression-gated at the 669 floor |
| Parse at real runtime target (LP64) | **70/1,680** committed today; ~150 measured in the stitched-header dry-run (battery apply pending; true population 1,602 types/2,951 sites post-uncapped-audit, blind tail in adjudication) |
| Assert waivers | fail-closed generator: 8-source multi-TSV (tranches + resolutions with later-wins precedence), deny-root set (29 format records incl NJS_* loaded-image structs), zero 1→0 regressions, per-source receipts |
| Link hygiene | `tools/link_hygiene.sh`: musyx C++-mode mangling gate, 24/24 golden names incl `chorus_fx` stand-in (unhostable MWCC asm, fail-loud + removal checklist) |
| Endianness layer | `compat/pso_byteswap.h`; **38/38 tests** incl UBSan; GVRT identity-vs-add-swap trap pinned by named cases |
| Boot plan | 75-action init ledger → 23-step logo checklist → rung-4 chain contract (35 ordered rows to the GX boundary); DEFER audit 19 CONFIRMED/4 REFUTED with the VI-DTV glue contract proven executable |

Phase 2 exit: PSO title screen rendered by Metal on Apple Silicon, from a **linkable LP64
build** reading your own dump. Game benchmark: a created character walking in CotL at
unlocked framerate. Estimates, revised once against the LP64 measurement: linked executable
~4–6 weeks of fleet-paced verdict+dialect work; title screen ~2–3 months.

## License

MIT (matching upstream pso-decomp). Game content is never included.
