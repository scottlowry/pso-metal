/* Video / display + AV mode (vi.h, os/OSRtc.h) — pso-metal host re-host of Video / display + AV mode
 * Tag format: original SDK symbol | call-site count (docs/host-surface.md) |
 * effort (TRIVIAL/MODERATE/HARD/BLOCKING-GPU) | native backing note.
 * Featured = real dolphin signature; tail = PSOSH_TAG placeholder prototype. */
#ifndef PSOSH_VI_H
#define PSOSH_VI_H
#include "psosh_types.h"


/* vi.h — 16 sites — MODERATE
 * Block one display retrace. Native backing: CVDisplayLink callback /
 * MTKView presentDrawable pacing semaphore. TODO(P2): 60Hz fixed cap. */
void VIWaitForRetrace(void);

/* vi.h — 8 sites — MODERATE
 * Push pending XFB writes to display. Native backing: commit the Metal
 * drawable (present). */
void VIFlush(void);

/* ---- long tail: placeholder prototypes; exact signatures land in P2 ---- */
PSOSH_TAG(VISetBlack, 6, "MODERATE", "CAMetalLayer + CVDisplayLink retrace pacing; native mode switch")
PSOSH_TAG(OSSetSoundMode, 3, "MODERATE", "CAMetalLayer + CVDisplayLink retrace pacing; native mode switch")
PSOSH_TAG(VIGetRetraceCount, 3, "MODERATE", "CAMetalLayer + CVDisplayLink retrace pacing; native mode switch")
PSOSH_TAG(VISetNextFrameBuffer, 3, "MODERATE", "CAMetalLayer + CVDisplayLink retrace pacing; native mode switch")
PSOSH_TAG(VISetPreRetraceCallback, 3, "MODERATE", "CAMetalLayer + CVDisplayLink retrace pacing; native mode switch")
PSOSH_TAG(OSSetProgressiveMode, 2, "MODERATE", "CAMetalLayer + CVDisplayLink retrace pacing; native mode switch")
PSOSH_TAG(VIConfigure, 2, "MODERATE", "CAMetalLayer + CVDisplayLink retrace pacing; native mode switch")
PSOSH_TAG(VIGetDTVStatus, 2, "MODERATE", "CAMetalLayer + CVDisplayLink retrace pacing; native mode switch")
PSOSH_TAG(VIGetNextField, 2, "MODERATE", "CAMetalLayer + CVDisplayLink retrace pacing; native mode switch")
PSOSH_TAG(VISetPostRetraceCallback, 2, "MODERATE", "CAMetalLayer + CVDisplayLink retrace pacing; native mode switch")
PSOSH_TAG(OSGetProgressiveMode, 1, "MODERATE", "CAMetalLayer + CVDisplayLink retrace pacing; native mode switch")
PSOSH_TAG(VIGetTvFormat, 1, "MODERATE", "CAMetalLayer + CVDisplayLink retrace pacing; native mode switch")
PSOSH_TAG(VIInit, 1, "MODERATE", "CAMetalLayer + CVDisplayLink retrace pacing; native mode switch")
/* VIPadFrameBufferWidth — vi.h — 1 site — TRIVIAL — pad fb width to xfb mode. */
u32 VIPadFrameBufferWidth(void);

#endif /* PSOSH_VI_H */
