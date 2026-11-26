# Phase-0 task A — compat-shim sweep results

Baseline → after: **591 → 669 / 1680 files parse (35.2% → 39.8%)** with **zero
edits to decomp source**. LP64 sanity column also moved 47 → 70 (incidental:
freestanding + intrinsic shims help there too). Repro: `bash
probes/tree_compile_probe.sh`; per-file `.err` under
`$SCRATCH/clangprobe/ilp32/`; baseline fail list archived at
`$SCRATCH/pso-metal/baseline_fail_591.txt`.

Changes made:
- `compat/pso_metal_compat.h`: added `__fabs`, `__cntlzw` (0→32 semantics),
  `__abs`, `__memcpy`, `__dcbt` shims; **removed** `#define fabs/fabsf
  __builtin_*` (they turned decomp's own `inline double fabs(...)` definitions
  in `include/clib/math.h` into "definition of builtin function" errors).
- `probes/tree_compile_probe.sh`: ilp32 run adds `-ffreestanding` and
  `-Wno-c++11-narrowing` (WHY comments inline).

## Residual classes (failing TUs, after sweep; classes overlap)

| Class | Baseline TUs | After | Verdict |
|---|---|---|---|
| A `array size is negative` (LAYOUT_ASSERT typedefs) | 934 | 934 | **REAL-ABI-WORK (deferred to port)** — see `docs/abi-layout-classA-notes.md`; deliberately not shimmed away |
| B `'main' must return 'int'` | 428 | **0** | **LEGITIMATELY-FIXED** (`-ffreestanding`; decomp declares GC entry point `void main(void)` — upstream NOTE below) |
| C dialect parse (`expected …`) | 78 | 78 | **NEEDS-DECOMP-UPSTREAM-NOTE** — three causes, all unshimmable via force-include (see below) |
| D `case -1 → u32` narrowing | 14 | **0** | **LEGITIMATELY-FIXED** (`-Wno-c++11-narrowing`; MWCC wrap-around is valid) |
| E missing intrinsics | 62 | 9 | **LEGITIMATELY-FIXED** (compat shims; residual 9 are `__builtin_va_info`/`memmove` fallout in TUs also poisoned by class C) |
| F `must use 'struct' tag` (SoundReverb etc.) | 26 | 26 | **NEEDS-DECOMP-UPSTREAM-NOTE** — tag vs typedef mismatch across `include/musyx` + `include/pso/audio/SoundReverb.h` |
| G `definition of builtin` | 14 | **0** | **LEGITIMATELY-FIXED** (removed the fabs/fabsf macros) |
| H `orig_asm/*.inc` not found | 11 | 11 | **REAL-PORT-WORK (deferred)** — decomp-only asm include files absent from the repo; port replaces these bodies anyway |
| Z other | 30 | 20 | mixed long tail (array-size declarator, `__register_global_object`) |

## Decomp-output findings worth upstreaming
1. **`void main(void)`** declared in `include/pso/object/base/TObjCharaHelpers.h:21`
   (and definitions) — faithful GC output, but breaks every conforming C++
   front end hosted-mode; decomp could alias it (`pso_main`) in a header-only
   wrapper.
2. **`AT_ADDRESS(addr)` → `: (addr)`** (`include/types.h:61`,
   `libs/dolphin/include/dolphin/types.h:43`) abuses bitfield syntax; only MWCC
   parses it. Suggest `#ifdef __GNUC__` → `__attribute__((section(".abs")))`
   with linker-script placement.
3. **`libs/dolphin/include/dolphin/hw_regs.h:4-6`** uses raw MWCC absolute-register
   syntax (`volatile u16 __VIRegs[59] : 0xCC002000;`) — needs the same guard.
4. **Unguarded `extern "C"`** in `include/musyx/musyx.h:161` and
   `include/pso/audio/SoundReverb.h:32` reached from `.c` TUs (28 parse-failing
   TUs); wrap in `#ifdef __cplusplus`.
5. `LAYOUT_ASSERT` is the right pattern — keep; class A volume is expected
   ABI delta, not a bug.

## Next lever (not in scope for this sweep)
Class A dominates: resolving the PPC-`bool`/enum-width strategy (doc above)
is the single biggest remaining win (~900 files gated). Class C+F ≈ 70-80
files gated on upstream header-dialect notes.
