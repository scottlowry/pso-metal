/* pso-metal P2-Q bringup DVD shim (p1-init-ledger §4 step 4).
 * DVDInit/open/read over a FAKE BUNDLE: the directory in $PSO_FAKE_DISC
 * (default ./fake_disc next to this file's build) is the disc root; every
 * DVD name maps to one file opened/closed with a real fd, every read is a
 * pread. Status words report present/ready synchronously:
 *   DVDGetDriveStatus / DVDGetCommandBlockStatus -> 0 (DVD_STATE_READY)
 *   DVDCheckDisk -> TRUE;  async reads complete before returning, callback
 *   fired with result 0 (era DVDCallback success convention).
 * The era async DVDCommandBlock state machine (GCD/dispatch queue per
 * host-surface.md) is NOT needed for bringup: callers poll status then use
 * the transferred-size words, which we fill exactly.
 */
#include "pso_bringup.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

static int dvd_ready;
static char disc_root[1024];

static int resolve(char *filename, char *out, size_t cap) {
    /* era names are absolute-FST paths ("/files/..."); fake bundle flattens
     * the leading slash and rejects anything that tries to escape the root. */
    if (filename[0] == '/') filename++;
    if (strstr(filename, "..")) return -1;
    int r = snprintf(out, cap, "%s/%s", disc_root, filename);
    return (r > 0 && (size_t)r < cap) ? 0 : -1;
}

void DVDInit(void) {
    const char *env = getenv("PSO_FAKE_DISC");
    snprintf(disc_root, sizeof disc_root, "%s",
             env ? env : "."); /* run.sh always sets this */
    struct stat st;
    dvd_ready = (stat(disc_root, &st) == 0 && S_ISDIR(st.st_mode));
    OSReport("dvd_shim: fake bundle root '%s' -> drive %s\n",
             disc_root, dvd_ready ? "READY" : "NO_DISC");
}

s32  DVDGetDriveStatus(void) { return dvd_ready ? 0 : -1; } /* 0 = READY */
BOOL DVDCheckDisk(void)      { return dvd_ready ? TRUE : FALSE; }

BOOL DVDOpen(char *filename, DVDFileInfo *fileInfo) {
    char path[1200];
    if (!dvd_ready || resolve(filename, path, sizeof path) != 0) return FALSE;
    int fd = open(path, O_RDONLY);
    if (fd < 0) return FALSE;
    struct stat st;
    if (fstat(fd, &st) != 0) { close(fd); return FALSE; }
    /* mirror era DVDOpen side effects: file block carries start offset,
     * rounded-up length, and a READY command block */
    fileInfo->startAddr = 0;
    fileInfo->length = (u32)st.st_size;
    fileInfo->cb.state = 0;                 /* idle/ready */
    fileInfo->cb.offset = 0;
    fileInfo->cb.length = (u32)st.st_size;
    fileInfo->cb.transferredSize = 0;
    fileInfo->cb.command = 4;               /* DVD_CMD_READ marker (shape) */
    fileInfo->cb.userData = (void *)(intptr_t)fd; /* the handle */
    fileInfo->callback = 0;
    return TRUE;
}

static s32 dvd_pread_impl(DVDFileInfo *fileInfo, void *addr, s32 length, s32 offset) {
    int fd = (int)(intptr_t)fileInfo->cb.userData;
    if (fd <= 0 || !dvd_ready) return -1;
    ssize_t got = pread(fd, addr, (size_t)length, (off_t)offset);
    fileInfo->cb.transferredSize = got < 0 ? 0 : (u32)got;
    fileInfo->cb.currTransferSize = fileInfo->cb.transferredSize;
    fileInfo->cb.offset = (u32)offset;
    return got < 0 ? -1 : (s32)got; /* era: transferred bytes, -1 = error */
}

s32 DVDReadPrio(DVDFileInfo *fileInfo, void *addr, s32 length, s32 offset, s32 prio) {
    (void)prio; /* single-threaded sync shim */
    return dvd_pread_impl(fileInfo, addr, length, offset);
}

BOOL DVDReadAsyncPrio(DVDFileInfo *fileInfo, void *addr, s32 length, s32 offset,
                      void (*cb)(s32 result, DVDFileInfo *fileInfo), s32 prio) {
    (void)prio;
    s32 got = dvd_pread_impl(fileInfo, addr, length, offset);
    if (cb) cb(got < 0 ? -1 : 0, fileInfo); /* era result: 0 success, <0 err */
    return TRUE;
}

BOOL DVDClose(DVDFileInfo *fileInfo) {
    int fd = (int)(intptr_t)fileInfo->cb.userData;
    fileInfo->cb.userData = 0;
    fileInfo->cb.state = 0;
    if (fd > 0) { close(fd); return TRUE; }
    return FALSE;
}

s32 DVDGetCommandBlockStatus(DVDCommandBlock *block) {
    (void)block;
    return 0; /* everything completes synchronously: always ready */
}
