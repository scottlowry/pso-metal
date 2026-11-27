# compat/packets — P2-PK packet_classes lane (INERT)

Phase-3 lane PK, 2026-11-27. Nothing in this directory is referenced by any
build/host/data file: inert until the integrator opts in. Decomp tree
READ-ONLY; zero repo builds were used to produce anything here (scratch
`clang -fsyntax-only` probes only; receipts + counts: `docs/p2-packets-1.md`).

## Contents

- `pso_command_handler_era.h` — era-image mirror of the PPC dispatch-table
  record `PsoCommandHandler` (`include/pso/network/protocol/packet_classes.h:370-376`).
  The port cannot keep `sizeof == 8` under LP64 (the handler is a real function
  pointer, 12 B), but the 8-byte era image is *runtime-side* layout knowledge
  (the table is compile-time `.data`, never wire, never disc). The lane's ask
  flips ONLY `PSO_LA_FLAG_PsoCommandHandler_size` to 1 (RUNTIME-WAIVE, see doc);
  this header keeps the era image pinned and compile-visible afterwards, so the
  waiver loses no truth: `static_assert(sizeof(PsoCommandHandlerEra) == 8)`
  stands in place of the waived assert, plus era↔port handle accessors for any
  future byte-true table supply (rung-4 fixtures).

## Pin/collision law compliance

- Not force-includable: this dir is NOT part of the three-file pin
  (`compat/pso_metal_compat.h`, `build/waivers/pso_assert_waivers.h`,
  `compat/collision/pin_dolphin_types.h`) and must not be added to it.
- If the integrator ever adopts it, it is `-I`-path material only (repin
  pattern). The header is stdint-self-contained; no include-path hijacks,
  zero effect on any current TU, so `-fsyntax-only` adoption risk is nil.
- The 6 FORMAT-KEEP asserts in packet_classes.h (rows 255, 256, 257, 298,
  316, 521) PASS under true LP64 measured over the 1,680-unit battery —
  they need no fix here and no waiver anywhere (deny-list honored).
