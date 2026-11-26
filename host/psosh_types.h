/* psosh_types.h — pso-metal host type shims for the re-hosted dolphin surface.
 * Names match libs/dolphin/include/dolphin/types.h so game sources compile
 * unmodified. Signatures in the subsystem headers are copied from the SDK
 * headers at pso-decomp @6b38b5383. */
#ifndef PSOSH_TYPES_H
#define PSOSH_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ---- dolphin base/std types (types.h) -------------------------------------- */
typedef int8_t   s8;
typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint16_t u16;
typedef int32_t  s32;
typedef uint32_t u32;
typedef int64_t  s64;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;
typedef u32      uaddr;   /* dolphin: address-sized word (types.h) */
typedef int      BOOL;    /* types.h:38 */
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
/* ATTRIBUTE_ALIGN — types.h macro — 209 sites — TRIVIAL — same semantics natively. */
#ifndef ATTRIBUTE_ALIGN
#define ATTRIBUTE_ALIGN(num) __attribute__((aligned(num)))
#endif

/* ---- GameCube timebase (os/OSTime.h) ---------------------------------------- */
typedef s64 OSTime;   /* 64-bit timebase */
typedef u32 OSTick;   /* 32-bit tick     */
/* GC timebase Hz — keep for OSGetTick/OSTicksTo* rate math (host-surface.md §2). */
#define OS_TIMER_CLOCK 79824327u
/* Heap slot the OSAlloc/OSFree macro aliases use (os/OSAlloc.h). */
extern int __OSCurrHeap;

/* ---- GX scalar/enum shims (gx/GXEnum.h) -------------------------------------
 * SDK declares these as enums; the host keeps the names but plain integer
 * typedefs. Exact enumerator values must be exported from GXEnum.h when the
 * GX state tracker lands — TODO(P2-gx). */
#if defined(__cplusplus) || defined(PSOSH_USE_BOOL_GXBOOL)
typedef bool GXBool;   /* GXEnum.h:13 path */
#else
typedef u8   GXBool;   /* GXEnum.h:15 path */
#endif
typedef int  GXCompare;
typedef int  GXAttr;
typedef int  GXAttrType;
typedef int  GXVtxFmt;
typedef int  GXCompCnt;
typedef int  GXCompType;
typedef int  GXPrimitive;
typedef int  GXBlendMode;
typedef int  GXBlendFactor;
typedef int  GXLogicOp;
typedef int  GXTevStageID;
typedef int  GXTevMode;
typedef int  GXTexCoordID;
typedef int  GXTexMapID;
typedef int  GXChannelID;

/* GXFifoObj (gx/GXFifo.h) — host: handle to the CPU vertex-streamer staging. */
typedef struct GXFifoObj GXFifoObj;

/* ----_mtx types (mtx/GeoTypes.h) — row-major, compiled natively ------------- */
typedef f32 Mtx34[3][4];
typedef f32 Mtx44[4][4];
typedef Mtx34 GC_Mtx;

/* ---- DVD types (dvd.h) — byte-order debt lives in these payloads ---------- */
typedef struct DVDDiskID {
    char gameName[4];
    char company[2];
    u8   diskNumber;
    u8   gameVersion;
    u8   streaming;
    u8   streamBufSize;
    u8   padding[22];
} DVDDiskID;                       /* dvd.h:26 — BE-sensitive */
typedef struct DVDCommandBlock DVDCommandBlock;
typedef struct DVDFileInfo       DVDFileInfo;
typedef struct DVDDirEntry       DVDDirEntry;
typedef void (*DVDCallback)(s32 result, struct DVDCommandBlock *block);

/* ---- Card types (card.h) --------------------------------------------------- */
typedef struct CARDInfo   CARDInfo;
typedef struct CARDStat   CARDStat;
typedef struct CARDCommandBlock CARDCommandBlock;
typedef void (*CARDAsyncResult)(s32 result, void *userData);

/* ---- Input (pad.h) — layout copied verbatim; GC BE bit values -------------- */
typedef struct PADStatus
{
    u16 button;
    s8  stickX;
    s8  stickY;
    s8  substickX;
    s8  substickY;
    u8  triggerLeft;
    u8  triggerRight;
    u8  analogA;
    u8  analogB;
    s8  err;
} PADStatus;
typedef void (*EXICallback)(s32 len);   /* exi.h — channel transfer callback */

/* ---- Audio (ar.h/ai.h) ------------------------------------------------------ */
typedef struct ARQRequest ARQRequest;   /* DSP-transfer queue node */
typedef void (*ARQCallback)(u8 *task);

/* ---- Long-tail stub macro ---------------------------------------------------
 * Each tail entry declares the SDK symbol with a PLACEHOLDER `int(void)`
 * prototype plus machine-readable tags: call sites (docs/host-surface.md),
 * effort (TRIVIAL/MODERATE/HARD/BLOCKING-GPU), native backing. Phase-2 lanes
 * replace each line with the exact signature when they implement it. */
#define PSOSH_TAG(fn, sites, effort, native) int fn(void);

#endif /* PSOSH_TYPES_H */
