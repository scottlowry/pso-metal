/* Memory card (card.h) — pso-metal host re-host of Memory card
 * Tag format: original SDK symbol | call-site count (docs/host-surface.md) |
 * effort (TRIVIAL/MODERATE/HARD/BLOCKING-GPU) | native backing note.
 * Featured = real dolphin signature; tail = PSOSH_TAG placeholder prototype. */
#ifndef PSOSH_CARD_H
#define PSOSH_CARD_H
#include "psosh_types.h"


/* #3 CARDSetDiskID — card.h — 34 sites — MODERATE
 * Bind card API to this game's DVDDiskID (per-game card space ownership).
 * Native backing: mmap-file card image; ID stored in image header; game-name
 * 4cc from DVDDiskID selects the save directory. TODO(P2): BE-safe header r/w. */
s32 CARDSetDiskID(s32 chan, const DVDDiskID *diskID);

/* #14 CARDGetResultCode — card.h — 20 sites — MODERATE
 * Async result word of the last card command on `chan`.
 * Native backing: thread-local/atomic result slot written by the card worker
 * thread; values are CARD_ERROR_* codes from card.h. */
s32 CARDGetResultCode(s32 chan);

/* CARDGetDiskID — card.h — 12 sites — MODERATE
 * Current bound disk ID. Native backing: card-image header copy. */
const DVDDiskID *CARDGetDiskID(s32 chan);

/* CARDProbeEx — card.h — 6 sites — MODERATE
 * Report card size/sector size. Native backing: stat() of card image file. */
s32 CARDProbeEx(s32 chan, s32 *memSize, s32 *sectorSize);

/* ---- long tail: placeholder prototypes; exact signatures land in P2 ---- */
PSOSH_TAG(CARDGetStatus, 7, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDUnmount, 6, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDClose, 5, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDCheckAsync, 4, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDGetSectorSize, 4, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDMountAsync, 3, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDOpen, 3, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDFreeBlocks, 2, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDGetSerialNo, 2, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDInit, 2, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDReadAsync, 2, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDWriteAsync, 2, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDCheck, 1, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDCreate, 1, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDCreateAsync, 1, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDDelete, 1, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDDeleteAsync, 1, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDFormatAsync, 1, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDGetUniqueCode, 1, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDProbe, 1, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDRenameAsync, 1, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDSetAttributes, 1, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDSetAttributesAsync, 1, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDSetStatus, 1, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")
PSOSH_TAG(CARDSetStatusAsync, 1, "MODERATE", "mmap-file card image (FAT+slot copy) + async worker, CARDAsyncResult kept")

#endif /* PSOSH_CARD_H */
