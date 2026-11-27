#ifndef _DOLPHIN_DVD_H
#define _DOLPHIN_DVD_H

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DVDCommandBlock DVDCommandBlock;
typedef struct DVDFileInfo DVDFileInfo;

typedef void (*DVDCallback)(s32 result, DVDFileInfo* fileInfo);
typedef void (*DVDCBCallback)(s32 result, DVDCommandBlock* block);
typedef void (*DVDLowCallback)(u32 intType);
typedef void (*DVDDoneReadCallback)(s32, DVDFileInfo*);
typedef void (*DVDOptionalCommandChecker)(DVDCommandBlock* block, DVDLowCallback callback);

typedef struct DVDDriveInfo {
    u16 revisionLevel;
    u16 deviceCode;
    u32 releaseDate;
    u8 padding[24];
} DVDDriveInfo;

typedef struct DVDDiskID {
    char gameName[4];
    char company[2];
    u8 diskNumber;
    u8 gameVersion;
    u8 streaming;
    u8 streamBufSize;
    u8 padding[22];
} DVDDiskID;

struct DVDCommandBlock {
    DVDCommandBlock* next;
    DVDCommandBlock* prev;
    u32 command;
    s32 state;
    u32 offset;
    u32 length;
    void* addr;
    u32 currTransferSize;
    u32 transferredSize;
    DVDDiskID* id;
    DVDCBCallback callback;
    void* userData;
};

struct DVDFileInfo
{
 DVDCommandBlock cb;
 u32 startAddr;
 u32 length;
 DVDCallback callback;
};

typedef struct DVDDir {
    u32 entryNum;
    u32 location;
    u32 next;
} DVDDir;

typedef struct DVDDirEntry {
    u32 entryNum;
    BOOL isDir;
    char* name;
} DVDDirEntry;

typedef struct DVDQueue DVDQueue;

struct DVDQueue {
    DVDQueue* mHead;
    DVDQueue* mTail;
};

typedef struct DVDBB1 {
    u32 appLoaderLength;
    void* appLoaderFunc1;
    void* appLoaderFunc2;
    void* appLoaderFunc3;
} DVDBB1;

typedef struct DVDBB2 {
    u32 bootFilePosition;
    u32 FSTPosition;
    u32 FSTLength;
    u32 FSTMaxLength;
    void* FSTAddress;
    u32 userPosition;
    u32 userLength;
    u32 reserved_1C;
} DVDBB2;

void DVDInit();
BOOL DVDOpen(char* filename, DVDFileInfo* fileInfo);
BOOL DVDFastOpen(s32 entryNum, DVDFileInfo* fileInfo);
s32 DVDReadPrio(DVDFileInfo* fileInfo, void* addr, s32 length, s32 offset, s32 prio);
BOOL DVDReadAsyncPrio(DVDFileInfo* fileInfo, void* addr, s32 length, s32 offset, DVDCallback callback, s32 prio);
BOOL DVDClose(DVDFileInfo* fileInfo);

void DVDResume();
void DVDReset();

BOOL DVDCancelAsync(DVDCommandBlock* block, DVDCBCallback callback);
s32 DVDCancel(volatile DVDCommandBlock* block);

s32 DVDChangeDisk(DVDCommandBlock* block, DVDDiskID* id);
BOOL DVDChangeDiskAsync(DVDCommandBlock* block, DVDDiskID* id, DVDCBCallback callback);

s32 DVDGetCommandBlockStatus(const DVDCommandBlock* block);
s32 DVDGetDriveStatus();
BOOL DVDSetAutoInvalidation(BOOL doAutoInval);
void* DVDGetFSTLocation();

BOOL DVDOpenDir(char* dirName, DVDDir* dir);
BOOL DVDReadDir(DVDDir* dir, DVDDirEntry* dirEntry);
BOOL DVDCloseDir(DVDDir* dir);
BOOL DVDGetCurrentDir(char* path, u32 maxLength);
BOOL DVDChangeDir(char* dirName);
s32 DVDConvertPathToEntrynum(char* path);

s32 DVDGetTransferredSize(DVDFileInfo* fileInfo);
DVDDiskID* DVDGetCurrentDiskID();
BOOL DVDCompareDiskID(DVDDiskID* id1, DVDDiskID* id2);
DVDLowCallback DVDLowClearCallback();
BOOL DVDPrepareStreamAsync(DVDFileInfo* fileInfo, u32 length, u32 offset, DVDCallback callback);
s32 DVDCancelStream(DVDCommandBlock* block);

BOOL DVDCheckDisk();

void DVDPause();
s32 DVDSeekPrio(DVDFileInfo* fileInfo, s32 offset, s32 prio);
BOOL DVDSeekAsyncPrio(DVDFileInfo* fileInfo, s32 offset, DVDCallback callback, s32 prio);
BOOL DVDFastOpenDir(s32 entryNum, DVDDir* dir);
BOOL DVDCancelAllAsync(DVDCBCallback callback);
s32 DVDCancelAll();
void DVDDumpWaitingQueue();

#define DVDReadAsync(fileInfo, addr, length, offset, callback) \
    DVDReadAsyncPrio((fileInfo), (addr), (length), (offset), (callback), 2)
#define DVDGetFileInfoStatus(fileInfo) DVDGetCommandBlockStatus(&(fileInfo)->cb)

#define DVD_MIN_TRANSFER_SIZE 32

#define DVD_STATE_FATAL_ERROR -1
#define DVD_STATE_END 0
#define DVD_STATE_BUSY 1
#define DVD_STATE_WAITING 2
#define DVD_STATE_COVER_CLOSED 3
#define DVD_STATE_NO_DISK 4
#define DVD_STATE_COVER_OPEN 5
#define DVD_STATE_WRONG_DISK 6
#define DVD_STATE_MOTOR_STOPPED 7
#define DVD_STATE_PAUSING 8
#define DVD_STATE_IGNORED 9
#define DVD_STATE_CANCELED 10
#define DVD_STATE_RETRY 11

#define DVD_FILEINFO_READY 0
#define DVD_FILEINFO_BUSY 1

#define DVD_RESULT_GOOD 0
#define DVD_RESULT_FATAL_ERROR -1
#define DVD_RESULT_IGNORED -2
#define DVD_RESULT_CANCELED -3

#define DVD_AIS_SUCCESS 0

#define DVD_MIN_TRANSFER_SIZE    32

#define DVD_INTTYPE_TC 1
#define DVD_INTTYPE_DE 2
#define DVD_INTTYPE_CVR 4

#define DVD_COMMAND_NONE 0
#define DVD_COMMAND_READ 1
#define DVD_COMMAND_SEEK 2
#define DVD_COMMAND_CHANGE_DISK 3
#define DVD_COMMAND_BSREAD 4
#define DVD_COMMAND_READID 5
#define DVD_COMMAND_INITSTREAM 6
#define DVD_COMMAND_CANCELSTREAM 7
#define DVD_COMMAND_STOP_STREAM_AT_END 8
#define DVD_COMMAND_REQUEST_AUDIO_ERROR 9
#define DVD_COMMAND_REQUEST_PLAY_ADDR 10
#define DVD_COMMAND_REQUEST_START_ADDR 11
#define DVD_COMMAND_REQUEST_LENGTH 12
#define DVD_COMMAND_AUDIO_BUFFER_CONFIG 13
#define DVD_COMMAND_INQUIRY 14
#define DVD_COMMAND_BS_CHANGE_DISK 15

#define DVD_WATYPE_MAX 2

/* --- P2-MA MWCC-dialect preamble (shadow-only; docs/p2-mwccasm-1.md) ------
 * The era line below spells an MWCC/CodeWarrior absolute-address variable
 * binding:  `T name AT_ADDRESS(addr);`  with the era types.h spelling
 * `#define AT_ADDRESS(addr) : (addr)` (colon declarator). clang/C11/C++17
 * rejects it ("expected ';' after top level declarator" cascade). The pin is
 * a guest-linker contract with no clang parse-world analogue; host executing
 * builds route these names through D4 facades, so the macro is neutralized
 * empty here, per the P2-T precedent (compat/dialect/pso_dialect_compat.h
 * section 1: "a port defines the pin empty"). One neutralization serves the
 * whole TU: it also repairs era sites reached WITHOUT their own shadow
 * (os.h:125-136, OSLink.h:27-28, dsp/dsp.h:10) when any chain hits this file
 * first. Everything else below is era-VERBATIM. Guard name is era-identical,
 * so the era copy and this copy can never both process in one TU, and the
 * collision pin (compat/collision/pin_dolphin_types.h, which occupies the
 * _DOLPHIN_TYPES_H_ guard but never any header guard below) does not
 * suppress this file. */
#ifdef AT_ADDRESS
#undef AT_ADDRESS
#endif
#define AT_ADDRESS(addr) /* P2-MA: era address pin dropped host-side (parse world) */

DVDDiskID DiskID AT_ADDRESS(0x80000000);

#ifdef __cplusplus
};
#endif

#endif
