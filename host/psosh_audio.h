/* Audio (ar.h, ai.h, dsp.h) — pso-metal host re-host of Audio
 * Tag format: original SDK symbol | call-site count (docs/host-surface.md) |
 * effort (TRIVIAL/MODERATE/HARD/BLOCKING-GPU) | native backing note.
 * Featured = real dolphin signature; tail = PSOSH_TAG placeholder prototype. */
#ifndef PSOSH_AUDIO_H
#define PSOSH_AUDIO_H
#include "psosh_types.h"


/* ar.h — 8 sites — HARD
 * Queue a DSP RAM transfer request. Native backing: ARQ queue executes on
 * the audio render worker; DSP RAM replaced by host staging buffer feeding
 * AVAudioEngine/AudioQueue mixer graph. TODO(P2): keep priority order. */
void ARQPostRequest(ARQRequest *task, uaddr owner, u32 type, u32 priority,
                    uaddr source, uaddr dest, u32 length, ARQCallback callback);

/* ar.h — 6 sites — HARD — allocate in DSP-RAM arena (host staging slab). */
u32 ARAlloc(u32 length);

/* ar.h — HARD — init ARQ stack + DSP RAM arena (host slab). TODO(P2): the
 * MusyX ucode protocol is the audio wildcard (host-surface.md §4). */
u32 ARInit(u32 *stack_index_addr, u32 num_entries);

/* ai.h — 2 sites — HARD — audio interface init (ring buffer into
 * AVAudioEngine inputNode-equivalent output stream). */
void AIInit(u8 *stack);

/* ---- long tail: placeholder prototypes; exact signatures land in P2 ---- */
PSOSH_TAG(AIInitDMA, 2, "HARD", "AVAudioEngine/AudioQueue render thread; MusyX compiled in-tree; DSP ucode wildcard")
PSOSH_TAG(ARGetBaseAddress, 2, "HARD", "AVAudioEngine/AudioQueue render thread; MusyX compiled in-tree; DSP ucode wildcard")
PSOSH_TAG(ARGetSize, 2, "HARD", "AVAudioEngine/AudioQueue render thread; MusyX compiled in-tree; DSP ucode wildcard")
PSOSH_TAG(DSPCheckMailToDSP, 2, "HARD", "AVAudioEngine/AudioQueue render thread; MusyX compiled in-tree; DSP ucode wildcard")
PSOSH_TAG(DSPSendMailToDSP, 2, "HARD", "AVAudioEngine/AudioQueue render thread; MusyX compiled in-tree; DSP ucode wildcard")
PSOSH_TAG(AIRegisterDMACallback, 1, "HARD", "AVAudioEngine/AudioQueue render thread; MusyX compiled in-tree; DSP ucode wildcard")
PSOSH_TAG(AIStartDMA, 1, "HARD", "AVAudioEngine/AudioQueue render thread; MusyX compiled in-tree; DSP ucode wildcard")
PSOSH_TAG(ARQInit, 1, "HARD", "AVAudioEngine/AudioQueue render thread; MusyX compiled in-tree; DSP ucode wildcard")
PSOSH_TAG(ARQRemoveRequest, 1, "HARD", "AVAudioEngine/AudioQueue render thread; MusyX compiled in-tree; DSP ucode wildcard")
PSOSH_TAG(DSPAddTask, 1, "HARD", "AVAudioEngine/AudioQueue render thread; MusyX compiled in-tree; DSP ucode wildcard")
PSOSH_TAG(DSPInit, 1, "HARD", "AVAudioEngine/AudioQueue render thread; MusyX compiled in-tree; DSP ucode wildcard")

#endif /* PSOSH_AUDIO_H */
