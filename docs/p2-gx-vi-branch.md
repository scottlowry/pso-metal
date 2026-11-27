# P2-AB — logo VI progressive-branch, made port-testable (lane V contract W1/W2)

**Scope.** Lane V (docs/p2-defer-audit.md) refuted A3: `logo_clear_flag` **ships 1**
(TAdVIConfigGC.cpp:87), so the boot-to-logo path only reaches the SEGA logo while
`VIGetDTVStatus()==0`. W1 asked for that glue contract to be explicit; W2 for the
`is_ready()` stub discipline. This micro turns both into an executable gate: the
**real decomp branch code** is compiled verbatim and driven under both DTV outcomes.

## 1. What the logo frame-1 path now provably does (per VI outcome)

Driver: `host/gx/vi_branch_driver.c` → `TLogo::run_task()` state 0 = logo frame 1,
built on verbatim slices @ pso-decomp `6b38b5383`:

| boot state | `check_progressive_ready` (:226-236) | disable fires? | run_task outcome (:137-150) | proven by |
|---|---|---|---|---|
| DTV=0 (any prog) | `VIGetDTVStatus()==0U` → `return 0` (:229) | **yes** — `disable_progressive_mode()` :192 → `OSSetProgressiveMode(0)` exactly 1 write, value 0 | `sega_logo = new TAdSEGAlogo` , `state=4` (:148-149); **no** `OSGetProgressiveMode` read (short-circuit) | scenario `dtv0` |
| DTV=1, progressive-reg=1 | second gate false (`OSGetProgressiveMode()==1`, :231) → `return 1` | no | `logo_clear_flag==1` (sliced :87) → `vi_config = new TAdVIConfigGC`, `state=1` (:140-141) — **the A3 dialog hazard, now demonstrated**: frame 1 on a progressive boot is a yes/no dialog, not the logo | scenario `dtv1p1` |
| DTV=1, progressive-reg=0, no pads | second gate true: `test_previous_buttons_packed==0` (:231) → `return 0` | yes | `state=4`, SEGA logo | scenario `dtv1p0`; A3's "zero-controller-input → Previous-test false" leg |

Gate: `./host/gx/run_vi.sh` — **exit 0** requires `PROOF <key> PASS` lines in
`build/vi_branch/run.log` for all of `dtv0`, `dtv1p1`, `dtv1p0` (+ `/frame1` twin of
each) plus the single-shot `argv dtv=0|dtv=1` re-runs and the **compile-flag** binary
`vi_branch_flagdtv auto` (`-DGX_VI_DTV_OUTCOME=1 -DGX_VI_PROG_OUTCOME=1`). Both
selection axes the task asked for are exercised, not just one. Control-flow proof
only — no window/readback; pixel proof stays with `run_window.sh` (W) and the Z
display-list runners.

## 2. Method: verbatim slices, harness surface, selectable VI

- **Real era code, zero edits.** `vi_slice_gen.py` + `vi_slice_manifest.txt` extract
  exact line ranges from the pinned decomp checkout at build time, verify per-range
  sha256, and emit `build/vi_branch/vi_era_slice.cpp` with `#line` tags. A decomp sha
  move or any byte drift in these ranges **fails the build** — re-pinning is explicit.

  | range | content | sha256 (first 12) |
  |---|---|---|
  | TLogo.cpp:36 | `logo_class_name` | `99d995bef6d0` |
  | TLogo.cpp:56 | `is_logo_clear_flag_set` | `0948b10c5ee9` |
  | TLogo.cpp:123-131 | `TLogo::TLogo` ctor | `b8823af14df4` |
  | TLogo.cpp:135-197 | `TLogo::run_task` (state machine) | `7958610bad1e` |
  | TAdVIConfigGC.cpp:87 | `s32 logo_clear_flag = 1;` | `c7db818d37ee` |
  | TAdVIConfigGC.cpp:192 | `disable_progressive_mode` | `f1b4ea3b3cfd` |
  | TAdVIConfigGC.cpp:226-236 | `check_progressive_ready` | `58af3acef6ed` |

  Whole-TU compilation is P2-T's dialect triage problem (113 residual files); these
  seven ranges compile clean as-is. Notably `disable_progressive_mode` and
  `check_progressive_ready` are **`static`** in the era headers
  (TAdVIConfigGC.h:21, TLogo.h:26), which is what makes the qualified call without an
  object at :228 legal era C++ — the slice proves it compiles, not just reads.
- **`vi_era_harness.h`** supplies only the minimal class/symbol surface so the bodies
  parse+link (era `#line` fidelity preserved; layout is the layout lane's job). Logo
  object ctors are counting stubs — *which branch constructed what* is the assertion.
- **`gx_vi.c/.h`** implements the branch-path era fns with **both outcomes
  selectable** (compile flags `GX_VI_DTV_OUTCOME`/`GX_VI_PROG_OUTCOME` + runtime
  `gx_vi_boot(dtv, prog)`), register-mirror semantics and W-style first-fire
  counters: `VIGetDTVStatus` (era vi.h:80, decomp-era `int(void)` form per
  gx_era.h:52), `OSSetProgressiveMode`/`OSGetProgressiveMode` (era OSRtc.h:83-84 —
  `disable_progressive_mode`'s chain terminates here).
- **W-linkage split (gotcha).** `gx_shims.c:156` already hardcodes
  `VIGetDTVStatus→0` — that IS the W1 contract, but it duplicate-symbol-collides with
  the selectable `gx_vi.o`, so this runner links **one or the other, never both**.
  W's header (`gx_era.h`), flag conventions (`gx_window.mk` patterns, `-x c++` entry
  named `vi_branch_driver.c` per the lane write-list) and build hygiene are reused;
  no existing file was touched. `enable_progressive_mode` (:183-190) is intentionally
  not sliced — it runs from `on_accept` (dialog input), not the state-0 path; it joins
  the fonts/dialog lane (W4) with the rest of TAdVIConfigGC's body.

## 3. W-item status

- **W1 — DONE.** The `VIGetDTVStatus→0` logo-min glue is now a compiling,
  counter-instrumented host function with a gate that proves both sides of the
  contract (`dtv0` path = W's hardcoded shim behavior reproduced dynamically).
- **W2 — satisfied by construction; `is_ready_note.c` NOT created.** The `is_ready()`
  hazard lives in `TLogo::run_frame()` (TLogo.cpp:108), which this lane deliberately
  does **not** slice — the branch under test is reached from `run_task` alone, so
  `TQuestVmDownLoadServer::is_ready` has zero link references in this binary and the
  W2 stub would be dead weight. When a runner first links `run_frame` (logo-scene
  lane), the `is_ready(){return 0;}` link stub goes in per W2 — one line, source noted.
- **W4 — untouched, left for the fonts lane** (TLanguage `load_fonts`/
  `load_text_resources` before first logo frame). The `dtv1p1` scenario now *demonstrates
  why*: the dialog branch constructs `TAdVIConfigGC`, whose rendering needs those fonts.
- W3/W5/W6 unchanged (title lane / bookkeeping).

## 4. Next-lane notes

- Display-list/scene lane: when linking `TLogo::run_frame`, add the W2 stub and keep
  `dtv` via `gx_vi_boot(0,·)` default — or reuse `run_window.sh`'s shim (they already
  return 0). Never co-link `gx_shims.o` + `gx_vi.o`.
- Dialog/fonts lane (W4): slice-in targets are `TAdVIConfigGC` ctor (:211-216),
  `set_field48_clear_global` (:178-181), `on_cancel/on_accept` (:194-205) and
  `enable_progressive_mode` (:183-190 — needs `GXNtsc480Prog` + VIConfigure/VIFlush/
  2×VIWaitForRetrace; that trio IS present in `gx_shims.o`, so the dialog binary wants
  a DTV-selectable variant of gx_vi plus the rest of W's shims — split there too).
  `is_vi_config_answered` real body (TLogo.cpp:57) reads offset 0x48 — keep it until
  the class gets its layout pass.
- Re-pin drill: after any decomp update touching these TUs, re-run
  `python3 host/gx/vi_slice_gen.py` failure output, re-check bodies against the audit,
  then update `vi_slice_manifest.txt` + `DECOMP_SHA` in `vi_branch.mk` together.

Files: `host/gx/{gx_vi.h,gx_vi.c,vi_era_harness.h,vi_slice_gen.py,vi_slice_manifest.txt,vi_branch.mk,vi_branch_driver.c,run_vi.sh}`.
Run evidence: `build/vi_branch/run.log` (7× `PROOF … PASS`, `VI-BRANCH: PASS`, `run_vi: gate OK`, exit 0).
