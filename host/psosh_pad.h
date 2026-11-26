/* Input (pad.h, si.h) — pso-metal host re-host of Input
 * Tag format: original SDK symbol | call-site count (docs/host-surface.md) |
 * effort (TRIVIAL/MODERATE/HARD/BLOCKING-GPU) | native backing note.
 * Featured = real dolphin signature; tail = PSOSH_TAG placeholder prototype. */
#ifndef PSOSH_PAD_H
#define PSOSH_PAD_H
#include "psosh_types.h"


/* pad.h — 2 sites — MODERATE
 * Copy latest PADStatus per channel. Native backing: map GC Pro Adapter /
 * DualSense input -> PADStatus bit layout + stick calibration fields.
 * TODO(P2): PAD_BUTTON_* values already match; analog deadzone math per
 * GC; err field = PAD_ERR_* codes. */
u32 PADRead(PADStatus *status);

/* pad.h — MODERATE
 * Start SI polling. Native backing: GameController.framework profile scan +
 * polling thread filling PADStatus at 125Hz equivalent. */
BOOL PADInit(void);

/* ---- long tail: placeholder prototypes; exact signatures land in P2 ---- */
PSOSH_TAG(PADControlMotor, 4, "MODERATE", "GameController.framework; PADStatus GC bit layout + stick calibration")
PSOSH_TAG(PADRecalibrate, 3, "MODERATE", "GameController.framework; PADStatus GC bit layout + stick calibration")
PSOSH_TAG(SIDisablePolling, 3, "MODERATE", "GameController.framework; PADStatus GC bit layout + stick calibration")
PSOSH_TAG(PADReset, 1, "MODERATE", "GameController.framework; PADStatus GC bit layout + stick calibration")
PSOSH_TAG(PADSetSpec, 1, "MODERATE", "GameController.framework; PADStatus GC bit layout + stick calibration")
PSOSH_TAG(SIEnablePolling, 1, "MODERATE", "GameController.framework; PADStatus GC bit layout + stick calibration")
PSOSH_TAG(SITransfer, 1, "MODERATE", "GameController.framework; PADStatus GC bit layout + stick calibration")

#endif /* PSOSH_PAD_H */
