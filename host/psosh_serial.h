/* Flash / EEPROM serial (os/OSSerial.h) — pso-metal host re-host of Flash / EEPROM serial
 * Tag format: original SDK symbol | call-site count (docs/host-surface.md) |
 * effort (TRIVIAL/MODERATE/HARD/BLOCKING-GPU) | native backing note.
 * Featured = real dolphin signature; tail = PSOSH_TAG placeholder prototype. */
#ifndef PSOSH_SERIAL_H
#define PSOSH_SERIAL_H
#include "psosh_types.h"



/* ---- long tail: placeholder prototypes; exact signatures land in P2 ---- */
PSOSH_TAG(SIGetResponse, 3, "MODERATE", "file-backed EEPROM/flash + in-process SI device tree")
PSOSH_TAG(SIProbe, 2, "MODERATE", "file-backed EEPROM/flash + in-process SI device tree")
PSOSH_TAG(SIRegisterPollingHandler, 2, "MODERATE", "file-backed EEPROM/flash + in-process SI device tree")
PSOSH_TAG(SIUnregisterPollingHandler, 2, "MODERATE", "file-backed EEPROM/flash + in-process SI device tree")
PSOSH_TAG(SIGetStatus, 1, "MODERATE", "file-backed EEPROM/flash + in-process SI device tree")
PSOSH_TAG(SIGetTypeAsync, 1, "MODERATE", "file-backed EEPROM/flash + in-process SI device tree")
PSOSH_TAG(SIIsChanBusy, 1, "MODERATE", "file-backed EEPROM/flash + in-process SI device tree")
PSOSH_TAG(SIRefreshSamplingRate, 1, "MODERATE", "file-backed EEPROM/flash + in-process SI device tree")
PSOSH_TAG(SISetCommand, 1, "MODERATE", "file-backed EEPROM/flash + in-process SI device tree")

#endif /* PSOSH_SERIAL_H */
