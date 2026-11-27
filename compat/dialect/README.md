# compat/dialect — P2-T port-side MWCC/PPC dialect shims (PROTOTYPE)

See docs/p2-dialect-triage.md for the bucket census and sizing. This directory is the
validated prototype for the top blocker class; landing it (main-compat fold + full
1680-file non-regression re-sweep) is the follow-up task.

Contents (usage = add flags AFTER the existing P2-R probe flags):

    -I compat/dialect/pshim -include compat/dialect/pso_dialect_compat.h

* `pso_dialect_compat.h` — force-include shim. Neutralizes the MWCC `AT_ADDRESS(addr)`
  address-pin declarator (guard-preemption precedent: build/waivers header) and typedefs
  `struct SoundReverb` (musyx tag collision). Self-includes ../pso_metal_compat.h so
  include-chain order never strands libc/math.h from the __frsqrte/__fabsf shims.
* `pshim/musyx/musyx.h` — shadow of include/musyx/musyx.h; ONLY diff: `#ifdef
  __cplusplus` wrap around the bare extern "C" block (orig:161). -I pshim wins lookup.
* `pshim/dolphin/hw_regs.h` — shadow of libs/dolphin/include/dolphin/hw_regs.h; ONLY
  diff: 8 raw `volatile T name[..] : 0xADDR;` pins rewritten as `extern` declarations.
* `flip_test.sh` — before/after clang -fsyntax-only over the zero-layout-assert
  dialect-class pool (26 files at 6b38b5383; result 26/26 FAIL→PASS; musyx runtime TUs
  compile in C++ mode for their source-side extern "C" blocks — see README section 2 /
  doc §2 note on link-name audit).

Regenerate shadows after any decomp-side upstream change:
`mussyx shadow`: copy include/musyx/musyx.h, wrap extern "C" block; `hw_regs`: copy,
rewrite `^volatile ... : 0x...;$` → `extern ...;`. Keep diffs to exactly those lines.
