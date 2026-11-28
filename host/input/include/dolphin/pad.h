/* P2-RDPREP step-1: PADStatus-only host shim for the input seat.
 * Field order copied EXACTLY from the era SDK header
 * (libs/dolphin/include/dolphin/pad.h :48-60 at pin 6b38b53837) — this shim
 * fronts ONLY the pad status word + the three OS entry points the verbatim
 * read_pads/bind slices touch. It never redefines any game struct and is
 * reachable ONLY through -Ihost/input/include (per R-D design law: no game
 * TU ever sees it). Executing-side: PADRead/PADReset/PADRecalibrate are the
 * seat's injection shims (input_state.c), never the PPC OS. */
#ifndef PSO_INPUT_DOLPHIN_PAD_H
#define PSO_INPUT_DOLPHIN_PAD_H

#include "types.h"

typedef struct PADStatus {
    u16 button;
    s8 stickX;
    s8 stickY;
    s8 substickX;
    s8 substickY;
    u8 triggerLeft;
    u8 triggerRight;
    u8 analogA;
    u8 analogB;
    s8 err;
} PADStatus;

#ifdef __cplusplus
extern "C" {
#endif

/* seat shims: return the injected pad image (input_state.c) */
s32 PADRead(PADStatus *status);            /* era returns u32; host s32 is word-safe */
s32 PADReset(u32 mask);
s32 PADRecalibrate(u32 mask);

#ifdef __cplusplus
}
#endif

#endif
