# pso-metal Phase-1 build skeleton

Host build of the READ-ONLY decomp sources (`pso-decomp/src`) with Apple clang,
force-including `compat/pso_metal_compat.h`. Nothing here writes into the decomp
tree — the tree is only read (plus its sources compiled).

## Quick start

```sh
make -C build parse          # syntax-only pass over every unit; prints TOTAL/FAIL/PASS
make -C build list-fail      # same, then writes build/parse_fail.txt
python3 tools/parse_gate.py  # regression gate: FAILs if arm64_32 PASS < 1601 (apply-6 floor; history 669->1416->1578->1576->1601)
make -C build parse-lp64     # TRUE LP64 leg (rebind+pin+net/repin+flags+musyx, docs/p2-stitch-apply5/6.md): 162 -> 261 -> 350 PASS; not floor-gated
make -C build smoke-link     # proves the real LP64 link toolchain compiles+links+execs
make -C build clean
```

Point the build at any checkout — nothing hardcodes a machine:

```sh
make -C build parse DECOMP_ROOT=/path/to/pso-decomp
```

(Due to concurrent decomp-lane drift the numbers above are measured against
`pso-decomp @ 6b38b5383`; see `../docs/p1-buildsystem.md` §Baseline.)

## Layout

| path | role |
|---|---|
| `Makefile` | parse driver; variables `DECOMP_ROOT`, `PARSE_TARGET`, `LINK_TARGET`, `JOBS` |
| `units.txt` | generated unit list (`find src -name '*.cpp' -o -name '*.c'`, sorted, relative to `DECOMP_ROOT`) |
| `gen_waivers.py` | fail-closed waiver generator (reads `../data/assert-verdicts.tsv`) |
| `waivers/pso_assert_waivers.h` | generated force-include header; ACTIVE today — 103 RUNTIME-WAIVE asserts neutralized, all other verdicts keep their asserts |
| `parse/` | per-run artifacts: `fail.txt`, `summary.txt`, `err/<unit>.err` (gitignored) |
| `parse_fail.txt` | `list-fail` output (gitignored) |
| `smoke/host_smoke.c` | link-target proof program |

## The two targets (do not conflate)

- `PARSE_TARGET = arm64_32-apple-watchos7.0` — ILP32, reproduces the Phase-0
  669-unit parse baseline. **Cannot link on macOS**; measurement-only.
- `LINK_TARGET = arm64-apple-macos12.0` — LP64, the triple the boot slice
  really executes under. `make parse-lp64` measures it; expect a *different
  pass set* even when counts match (measured: 598 differing units vs ILP32).

Full rationale, waiver safety argument, and drift history: `../docs/p1-buildsystem.md`.
