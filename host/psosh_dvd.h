/* Filesystem (dvd.h) — pso-metal host re-host of Filesystem
 * Tag format: original SDK symbol | call-site count (docs/host-surface.md) |
 * effort (TRIVIAL/MODERATE/HARD/BLOCKING-GPU) | native backing note.
 * Featured = real dolphin signature; tail = PSOSH_TAG placeholder prototype. */
#ifndef PSOSH_DVD_H
#define PSOSH_DVD_H
#include "psosh_types.h"


/* dvd.h — 8 sites — MODERATE
 * Poll command-block state word. Callers spin on this — status codes must
 * match dolphin enum values exactly. Native backing: atomic load of shim. */
s32 DVDGetCommandBlockStatus(const DVDCommandBlock *block);

/* dvd.h — 3 sites — MODERATE
 * Open path on disc, fill DVDFileInfo. Native backing: resolve guest path
 * against pre-extracted bundle dir; fd kept in the file-info shim.
 * TODO(P2): case-fold + game's ISO9660/FST path grammar. */
BOOL DVDOpen(char *filename, DVDFileInfo *fileInfo);

/* ---- long tail: placeholder prototypes; exact signatures land in P2 ---- */
PSOSH_TAG(DVDClose, 6, "MODERATE", "mmap-file game bundle; dispatch-queue command blocks, same status words (BE)")
PSOSH_TAG(DVDCheckDisk, 5, "MODERATE", "mmap-file game bundle; dispatch-queue command blocks, same status words (BE)")
PSOSH_TAG(DVDGetDriveStatus, 5, "MODERATE", "mmap-file game bundle; dispatch-queue command blocks, same status words (BE)")
PSOSH_TAG(DVDReadPrio, 3, "MODERATE", "mmap-file game bundle; dispatch-queue command blocks, same status words (BE)")
PSOSH_TAG(DVDCancel, 2, "MODERATE", "mmap-file game bundle; dispatch-queue command blocks, same status words (BE)")
PSOSH_TAG(DVDChangeDir, 2, "MODERATE", "mmap-file game bundle; dispatch-queue command blocks, same status words (BE)")
PSOSH_TAG(DVDGetTransferredSize, 2, "MODERATE", "mmap-file game bundle; dispatch-queue command blocks, same status words (BE)")
PSOSH_TAG(AT_ADDRESS, 1, "MODERATE", "mmap-file game bundle; dispatch-queue command blocks, same status words (BE)")
PSOSH_TAG(DVDGetCurrentDiskID, 1, "MODERATE", "mmap-file game bundle; dispatch-queue command blocks, same status words (BE)")
PSOSH_TAG(DVDInit, 1, "MODERATE", "mmap-file game bundle; dispatch-queue command blocks, same status words (BE)")
PSOSH_TAG(DVDReadAsyncPrio, 1, "MODERATE", "mmap-file game bundle; dispatch-queue command blocks, same status words (BE)")

#endif /* PSOSH_DVD_H */
