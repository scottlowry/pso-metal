/* GBA link e+ (gba.h) — pso-metal host re-host of GBA link e+
 * Tag format: original SDK symbol | call-site count (docs/host-surface.md) |
 * effort (TRIVIAL/MODERATE/HARD/BLOCKING-GPU) | native backing note.
 * Featured = real dolphin signature; tail = PSOSH_TAG placeholder prototype. */
#ifndef PSOSH_GBA_H
#define PSOSH_GBA_H
#include "psosh_types.h"



/* ---- long tail: placeholder prototypes; exact signatures land in P2 ---- */
PSOSH_TAG(GBAGetProcessStatus, 1, "TRIVIAL", "stub: report 'no GBA attached' via probe paths")
PSOSH_TAG(GBAInit, 1, "TRIVIAL", "stub: report 'no GBA attached' via probe paths")
PSOSH_TAG(GBAJoyBootAsync, 1, "TRIVIAL", "stub: report 'no GBA attached' via probe paths")

#endif /* PSOSH_GBA_H */
