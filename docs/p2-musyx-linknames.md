# P2-AA — musyx-as-C++ link-name discipline audit

Phase-2 lane AA. Static analysis of `pso-decomp @ 6b38b5383` (read-only, zero project
builds; isolated `clang -fsyntax-only` / `clang++ -S` probes in scratch only). Companion
data: [`data/musyx-linknames.tsv`](../data/musyx-linknames.tsv) (27 rows, tab-delimited).

**Question.** P2-T flips the 26 zero-assert musyx files FAIL→PASS by compiling musyx
`.c` as `clang++ -x c++` (source-side bare `extern "C"` blocks + bare `extern "C"` at
`include/musyx/musyx.h:161` are C++-only syntax). C++ mode changes language linkage —
does every pso↔musyx call still resolve to the **same symbol name at link time**?

**Method.** (1) Brace-tracked scan of every `extern "C" {…}` region in `src/musyx/**`
(30 `.c`, 2 `.h`); (2) column-0 top-level definition extraction (multi-line signatures
walked to their `{`); (3) word-boundary cross-index of all 458 musyx def names against
every token in `src/pso/**` (1,588 files); (4) for each cross symbol, locate the exact
declaration the pso caller compiles against (direct/local/header, one include level,
manually resolved where the scan flagged ambiguity); (5) empirical symbol probes (§4).

## 1. Totals

| Quantity | Count |
|---|---:|
| musyx public/exported symbols (non-static top-level defs, `src/musyx/**/*.c`) | **458** |
| …declared under `extern "C"` in vendor/public headers (`musyx.h` 7 + `musyx_internal.h` + `hw_dspctrl.h`) | 310 |
| **Crossing into `src/pso/**` (the link-relevant set)** | **24** (all funcs, all `snd*`) |
| musyx **data** symbols referenced by pso | 0 |
| musyx defs **outside** any `extern "C"` region | **3** (all `synth_dbtab.c` data) |
| Referrer TUs: `SoundControlState.cpp` (10 syms), `SoundRequestVoice.cpp` (5), `GcrnaStream.cpp` (6), `GcrnaStreamControl.cpp` (3, two shared with `GcrnaStream.cpp`), `MusicControl.cpp` (2) | 5 |

The 24 crossers: `sndSetHooks`, `sndSetAuxProcessingCallbacks`, `sndAuxCallbackReverbHI`,
`sndAuxCallbackChorus`, `sndAuxCallbackPrepareChorus`, `sndAuxCallbackPrepareReverbHI`,
`sndFXStartParaInfo`, `sndFXCheck`, `sndFXCtrl`, `sndFXCtrl14`, `sndFXKeyOff`, `sndInit`,
`sndOutputMode`, `sndVolume`, `sndPushGroup`, `sndPopGroup`, `sndStreamMixParameter`,
`sndStreamMixParameterEx`, `sndStreamAllocEx`, `sndStreamActivate`, `sndStreamDeactivate`,
`sndStreamFree`, `sndStreamFrq`, `sndStreamARAMUpdate`. (Only 7 are vendor-API via
`musyx.h:161-170`; the other 17 ride on pso-side `extern "C"` reconvention decls —
`GcrnaMusyxStream.h:9-16`, local blocks in `SoundControlState.cpp:515/552/738/901/940/1072`,
`MusicControl.cpp:286`, `GcrnaStreamControl.cpp:123`, `SoundRequestVoice.cpp:96` + one
standalone `extern "C"` line at `SoundRequestVoice.cpp:48`.)

## 2. Risk classes (TSV `mangle_risk`, legend)

Under `clang++ -x c++`: a **function** outside `extern "C"` is mangled (`__Z…`); a
**file-scope variable** is *never* mangled (Itanium property, probe P5/P2 below); an
`extern "C"` declaration anywhere in the TU (including via force-include) pins C
linkage onto the definition (probe P6).

| Class | Meaning | Count |
|---|---|---:|
| `dup` | double-covered: def inside a source-side bare `extern "C"` block **and** caller's decl is inside `extern "C"` → both sides = unmangled C name → fine | **24** |
| `none` | link name agrees regardless of mode (here: musyx-internal data defs, safe by the data-not-mangled ABI property) | **3** |
| `cpp-only-if-def-not-C` | live hazard: def escapes `extern "C"` **and** is a function (or the TU moves into a namespace), while every consumer expects the C name → C++-mangled def vs unmangled refs = undefined symbol | **0 today** |

**No symbol is defined C-only and referenced from C++ *without* `extern "C"`** — the
reverse hazard of the brief: zero pso call-sites see a bare-C++ declaration of a musyx
symbol. The audit is therefore a PASS for T's flip *as currently scoped*, with the
latent-trigger analysis below.

**Worst offender (fragility, not today-failure): `src/musyx/runtime/synth_dbtab.c`** —
the only musyx TU with no `extern "C"` anywhere *and* it includes only `types.h` (not
`musyx_internal.h`), so nothing pins C linkage on its defs:
`dspAttenuationTab` (synth_dbtab.c:5 ↔ musyx_internal.h:517), `dspScale2IndexTab`
(synth_dbtab.c:27 ↔ musyx_internal.h:418), `dspDLSVolTab` (synth_dbtab.c:96 ↔
musyx_internal.h:419). Consumers `synth_adsr.c:90/127/192`, `hardware.c:167`,
`synthmacros.c:333/418/1125/1128`, `hw_volconv.c:122` all expect the C names. They link
today **only by the data-mangling accident**; add any function to that file (or wrap the
bundle in a namespace) and it breaks.

## 3. Top-10 riskiest, both sides exact (def ↔ ref)

| # | symbol | def | ref/decl | risk | why ranked |
|---|---|---|---|---|---|
| 1 | `dspAttenuationTab` | `src/musyx/runtime/synth_dbtab.c:5` (no EC) | `src/musyx/runtime/musyx_internal.h:517` EC → `synth_adsr.c:90` | none (data) | def unguarded; C++-mode-safe only via ABI property |
| 2 | `dspScale2IndexTab` | `synth_dbtab.c:27` (no EC) | `musyx_internal.h:418` EC → `synth_adsr.c:127`, `hardware.c:167`, `synthmacros.c:418` | none (data) | same |
| 3 | `dspDLSVolTab` | `synth_dbtab.c:96` (no EC) | `musyx_internal.h:419` EC → `synthmacros.c:333`, `hw_volconv.c:122` | none (data) | same |
| 4 | `sndVolume` | `src/musyx/runtime/snd_synthapi.c:60` `void(u8,u16,u8)` | `src/pso/audio/SoundControlState.cpp:1073` `s32(s32,s32,s32);` | dup | link-safe, **real ABI lie** (return + widths) — calls garbage-collect wrong values |
| 5 | `sndFXCtrl` | `snd_synthapi.c:6` `s32(u32,u8,u8)` | `src/pso/audio/SoundRequestVoice.cpp:97` `s32(int,int,int);` | dup | width drift u8→int args |
| 6 | `sndPopGroup` | `src/musyx/runtime/s_data.c:256` `u32(void)` | `SoundControlState.cpp:902` `s32 sndPopGroup();` | dup | empty-paren = `(void)` in C++; returns u32 read as s32 |
| 7 | `sndAuxCallbackPrepareReverbHI` | `src/musyx/runtime/StdReverb/reverb_fx.c:19` `void(SoundReverb*)` | `SoundControlState.cpp:941` `void(void*);` | dup | struct-tag/type drift — needs P2-T's `SoundReverb` typedef to even parse |
| 8 | `sndFXStartParaInfo` | `snd_synthapi.c:30` | `SoundRequestVoice.cpp:153` via `include/pso/audio/SoundControlState.h:9 → musyx.h:168` EC | dup | link-name ok only through a **2-hop** include chain — fragile to header shuffling |
| 9 | `sndFXCheck` | `snd_synthapi.c:55` | `SoundRequestVoice.cpp:196` same 2-hop chain → `musyx.h:169` | dup | same |
| 10 | `sndInit` | `src/musyx/runtime/snd_init.c:24` | `src/pso/audio/MusicControl.cpp:288` (local EC block @286) | dup | reconvention decl, no vendor header — signature lies possible |

## 4. Empirical probes (scratch, read-only decomp, T's exact flip flags from `compat/dialect/flip_test.sh`)

| Probe | Command | Result |
|---|---|---|
| P1 | `clang++ -S` `snd_init.c` `-x c++` + dialect shim | emits `.globl _sndInit` — **bare source-side `extern "C"` works in C++ mode** (unmangled) |
| P2 | `clang++ -S` `synth_dbtab.c` `-x c++` | emits `_dspAttenuationTab`, `_dspScale2IndexTab`, `_dspDLSVolTab` — **unmangled despite no EC**: Itanium never mangles file-scope variables |
| P3 | `clang -fsyntax-only -std=c11` `synth_dbtab.c` | **exit 0** — the file is legal C; a per-TU `-x c` exception is available |
| P5 | synthetic control (`-x c++`) | `fn()` → `__Z3fnv` (functions DO mangle) while `tab` stays `_tab` |
| P6 | synthetic + `-include` shim declaring `extern "C" unsigned short fn_naked(int);`, def outside any block | def emits `_fn_naked` vs `__Z8fn_nakedi` without shim — **force-included EC declaration pins C linkage on the definition** |

## 5. Recommended port-side fixes (zero decomp edits)

- **Class `dup` (24) — keep as-is + lock it in.** T's mode decision (musyx `*.c` →
  `clang++ -x c++`, `pshim/musyx/musyx.h` shadow, `-I compat/dialect/pshim` first) is
  provably link-name-safe for every crosser. Add a **link-name regression guard** to
  the eventual musyx archive rule: `nm -gU` every compiled musyx `.o` and assert it
  contains the 24 golden names from this TSV and matches **zero** `__Z` symbols. That
  converts this one-time audit into a permanent gate without touching decomp.
- **Class `cpp-only-if-def-not-C` (0 today) — prevent trigger.** The class can only
  appear if (a) a musyx TU loses/never has its top-level `extern "C"` block *and*
  defines a function, or (b) T-mode files get wrapped in a namespace. The same
  `__Z`-zero `nm` assertion catches both at build time. If a future TU trips it, the
  recipe is a port force-include declaring that TU's exported symbols under
  `extern "C"` (probe P6) — same mechanism `compat/dialect/pso_dialect_compat.h`
  already uses; do not edit the decomp source.
- **`synth_dbtab.c` trio (3 `none`) — make safety explicit.** Recommended: extend the
  pshim bundle with `compat/dialect/pshim/musyx_dbtab_shim.h` declaring the 3 tables
  `extern "C" extern <type> name[];`, applied **port-side** via
  `-include compat/dialect/pshim/musyx_dbtab_shim.h` limited to the
  `synth_dbtab.c` compile rule (P6-proven). Alternative one-liner: per-TU mode
  exception `-x c` for `synth_dbtab.c` (P3-proven clean). Either removes the
  dependency on the data-mangling accident.
- **Shadow-header caveat (TU-mode discipline).** `pshim/musyx/musyx.h` declares the 7
  vendor functions **only** under `#ifdef __cplusplus` (correct for the flip) — a C TU
  consuming them would see no declarations. Verified today: the only `musyx.h`
  includers are `MusicControl.cpp`, `SoundControlState.cpp`, `SoundControlState.h`
  (all C++-mode). Rule for the port: *musyx.h consumers must be C++ TUs*; if a C
  consumer ever appears, add a plain-decl `#else` branch to the shadow header (port
  file), not to decomp.
- **Signature-drift side-order (rows 4-7).** `sndVolume` etc. link by name and lie by
  type — invisible to any linker/`nm` check. This belongs to P2-T §2's
  signature/linkage-drift decomp-verdict class, flagged here so the verdict lane owns
  it explicitly.

## 6. Cross-check vs P2-U (static file, no dependency)

`docs/p2-link-deps.md` + `data/link-deps.tsv` exist and were read. No conflict: their
census covers only the 70 LP64-parse-pass TUs, which contain **no musyx file and none
of the 5 audio referrers** (`data/link-deps.tsv` has 0 musyx rows) — so the 24
crossers are simply outside the first-link closure today, consistent with this audit
predicting zero link failures at that boundary. When T's flip lands those 31 TUs into
the compilable set, regenerate `link-deps.tsv`: none of the 24 names (or the 3 tables)
may appear as gaps, and no `__Z…snd*` symbol may appear anywhere — that is the joint
acceptance test of this doc and P2-U.

## Repro

Scanner/probes: `cache/scratch/…/p2aa/{scan2.py,step3.py,step4_tsv.py,probes/}` (this
lane's scratch; regenerates `musyx_scan.json`/`defs2.json` from decomp read-only).
Flags per `compat/dialect/flip_test.sh` §COMMON+DIALECT.
