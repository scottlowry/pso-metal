/* Network adapter / EXI bit-bang (exi.h) — pso-metal host re-host of Network adapter / EXI bit-bang
 * Tag format: original SDK symbol | call-site count (docs/host-surface.md) |
 * effort (TRIVIAL/MODERATE/HARD/BLOCKING-GPU) | native backing note.
 * Featured = real dolphin signature; tail = PSOSH_TAG placeholder prototype. */
#ifndef PSOSH_EXI_H
#define PSOSH_EXI_H
#include "psosh_types.h"


/* #1 EXIImm — exi.h — 52 sites — HARD
 * Queue immediate/PIO word transfer to an EXI peripheral channel.
 * Native backing: no hardware EXI exists. All 236 EXI sites live in
 * src/pso/network/{eth,mdm}.c (BBA/modem drivers). Plan: in-process device
 * model — channel 2 (slot B) bound to a POSIX-socket/Network.framework
 * backend for the BBA; buffer/type/length copied into an async work item.
 * TODO(P2): define psosh_exi_device model; preserve BE word semantics;
 * callback fires on completion thread. */
BOOL EXIImm(s32 channel, void *buffer, s32 length, u32 type, EXICallback callback);

/* #2 EXISync — exi.h — 52 sites — HARD
 * Block until the pending EXI transfer on `channel` completes.
 * Native backing: condition-variable wait on the psosh_exi work item.
 * TODO(P2): the drivers rely on IMM-vs-DMA completion timing; measure
 * whether zero-latency completion breaks eth.c retry loops. */
BOOL EXISync(s32 channel);

/* #6 EXISelect — exi.h — 33 sites — HARD
 * Chip-select device on channel at given frequency.
 * Native backing: per-device handle lookup in psosh_exi registry; "peripheral
 * ID" (periph 8 = extension device) must answer as BBA/modem.
 * TODO(P2): fake EXI_ID_PERIPHERAL_8 responses matching eth.c's probe. */
BOOL EXISelect(s32 channel, u32 device, u32 frequency);

/* #7 EXIDeselect — exi.h — 33 sites — HARD
 * Release chip-select. Native backing: drop device-handle reference.
 * TODO(P2): no-op safe once EXISelect state model lands. */
BOOL EXIDeselect(s32 channel);

/* #8 EXIUnlock — exi.h — 30 sites — HARD
 * Release the EXI bus lock taken by EXILock.
 * Native backing: pthread_mutex (per-channel) release; must round-trip with
 * EXILock's callback-when-busy semantics. TODO(P2): 1:1 with osx_lock. */
BOOL EXIUnlock(s32 channel);

/* EXILock — exi.h — HARD — companion of EXIUnlock (bus mutex take).
 * Native backing: pthread_mutex_trylock + queued EXICallback on contention. */
BOOL EXILock(s32 channel, u32 device, EXICallback callback);

/* EXIGetType — inferred (undeclared in bundle; see host-surface.md §4) — 2 sites — TRIVIAL
 * Query device type/word-length on a channel+device.
 * Native backing: registry lookup. TODO(P2): return EXT_DEVICE type for ch2. */
u32 EXIGetType(s32 channel, u32 device, u32 *type);

/* ---- long tail: placeholder prototypes; exact signatures land in P2 ---- */
PSOSH_TAG(EXIImmEx, 14, "HARD", "no EXI bus: socket/Network.framework device model behind same signatures")
PSOSH_TAG(EXIDma, 6, "HARD", "no EXI bus: socket/Network.framework device model behind same signatures")
PSOSH_TAG(EXISetExiCallback, 2, "HARD", "no EXI bus: socket/Network.framework device model behind same signatures")

#endif /* PSOSH_EXI_H */
