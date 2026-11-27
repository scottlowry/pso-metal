/* pso-metal bringup — host substrate contract (P2-Q).
 * Guest-address-space model per p1-datamodel.md §2/§4: NO absolute mappings
 * (macOS arm64 refuses fixed mappings at 0x80000000 — measured in
 * docs/p2-bringup-1.md), everything is arena_base + (guest - 0x80000000).
 */
#ifndef PSO_BRINGUP_H
#define PSO_BRINGUP_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- guest memory map (crt0.c) ---- */
#define GUEST_RAM_BASE   0x80000000u          /* nominal PPC RAM start        */
#define PSO_MAIN_RAM_SIZE 0x01800000u         /* 24 MB main arena             */
#define PSO_ARAM_SIZE     0x00200000u         /* 2 MB ARAM                    */
#define PSO_ARENA_SIZE    (PSO_MAIN_RAM_SIZE + PSO_ARAM_SIZE) /* ~26 MB       */

/* Guest offsets used inside the arena (era values, rebased): */
#define GUEST_SYSRET_OFF   0xF8u              /* 0x800000F8 __OSBusClock word */
#define GUEST_PRS_WINDOW_OFF 0x1800u          /* 0x80001800 decoder window    */

extern u8 *pso_arena_base;      /* [PSO_ARENA_SIZE) zero-filled              */
extern u8 *pso_aram_base;       /* == pso_arena_base + PSO_MAIN_RAM_SIZE     */
extern u8 *pso_prs_window_base; /* == pso_arena_base + GUEST_PRS_WINDOW_OFF  */

/* BootInfo replacement anchor: mirrors the retail GC SysRet block the decomp
 * reads (`*(u32 *)0x800000F8` — 9 live sites, all the bus-clock word; see
 * p1-datamodel §1a). Located at pso_arena_base + 0xF0 so that the bus-clock
 * word sits at +0xF8, exactly the era intra-block layout. */
typedef struct PsoSysRet {
    u32 memSize;   /* +0xF0  = 24 MB main RAM                    */
    u32 exiFreq;   /* +0xF4  = 16,209,792 Hz                     */
    u32 busFreq;   /* +0xF8  = 32,419,584 Hz (ONLY live-read fld)*/
    u32 cpuFreq;   /* +0xFC  = 486,293,760 Hz (15x bus)          */
} PsoSysRet;
extern PsoSysRet *pso_sysret;

/* Map a guest absolute address (0x80xxxxxx) into the arena. Deviation from
 * era literals is deliberate: fixed low mappings are impossible here. */
static inline void *pso_g2h(uaddr g) {
    return pso_arena_base + (g - GUEST_RAM_BASE);
}

int  pso_bringup_install(void);   /* mmap+zero+sysret+window; 0 on success  */

/* ---- mini OSInit (osinit.c) — NO threads/interrupts/MPU ---- */
void  OSInit(void);
void *OSGetArenaLo(void);
void *OSGetArenaHi(void);
void  OSSetArenaLo(void *lo);
void  OSSetArenaHi(void *hi);
void  OSReport(const char *fmt, ...);

/* GC timebase shim. Tick rate = busFreq/4 = 8,104,896 Hz, matching every
 * consumer that does  / (*(u32*)0x800000F8 >> 2)  (NetworkRuntime.c:140,
 * SoundControlState.cpp:913, ...). Wall-clock backed, 64-bit. */
u64   OSGetTime(void);            /* raw timebase ticks                      */
u32   OSGetTick(void);            /* low 32 bits of OSGetTime (era contract) */
s64   OSTicksToMilliseconds(u64 ticks);

/* OSCreateHeap-shaped heap shim over the arena (OSAlloc.h contract). */
typedef int OSHeapHandle;
extern volatile OSHeapHandle __OSCurrHeap;
void *OSInitAlloc(void *arenaStart, void *arenaEnd, int maxHeaps);
int   OSCreateHeap(void *start, void *end);
int   OSSetCurrentHeap(int heap);
void *OSAllocFromHeap(int heap, u32 size);
void  OSFreeToHeap(int heap, void *ptr);
long  OSCheckHeap(int heap);

/* ---- DVD shim over fake_disc/ (dvd_shim.c) — callback surface the decomp
 * actually uses: DVDInit (SystemBootInit.cpp:258), DVDOpen/DVDClose/
 * DVDReadPrio (text_803DD5E0.cpp:91-115), DVDReadPrio/DVDReadAsyncPrio
 * (Gcci.cpp:255-257). Layouts mirror libs/dolphin/include/dolphin/dvd.h. */
#define PSO_DVD_NAME_MAX 128
struct DVDCommandBlock;
typedef void (*DVDCBCallback)(s32 result, struct DVDCommandBlock *block);
typedef struct DVDCommandBlock {
    struct DVDCommandBlock *next;
    struct DVDCommandBlock *prev;
    u32  command;
    s32  state;
    u32  offset;
    u32  length;
    void *addr;
    u32  currTransferSize;
    u32  transferredSize;
    void *id;                /* DVDDiskID* */
    DVDCBCallback callback;
    void *userData;
} DVDCommandBlock;
typedef struct DVDFileInfo {
    DVDCommandBlock cb;
    u32 startAddr;
    u32 length;
    void (*callback)(s32 result, struct DVDFileInfo *fileInfo);
} DVDFileInfo;

void DVDInit(void);
BOOL DVDOpen(char *filename, DVDFileInfo *fileInfo);
s32  DVDReadPrio(DVDFileInfo *fileInfo, void *addr, s32 length, s32 offset, s32 prio);
BOOL DVDReadAsyncPrio(DVDFileInfo *fileInfo, void *addr, s32 length, s32 offset,
                      void (*cb)(s32 result, DVDFileInfo *fileInfo), s32 prio);
BOOL DVDClose(DVDFileInfo *fileInfo);
s32  DVDGetDriveStatus(void);        /* 0 = DVD_STATE_READY */
BOOL DVDCheckDisk(void);
s32  DVDGetCommandBlockStatus(DVDCommandBlock *block); /* 0 = ready */

#ifdef __cplusplus
}
#endif

#endif /* PSO_BRINGUP_H */
