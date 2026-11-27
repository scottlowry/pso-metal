/* pso-metal P2-Y (ledger step 7): era Dolphin OSAlloc ported 1:1 to host.
 *
 * Source of truth: pso-decomp libs/dolphin/src/os/OSAlloc.c @ 6b38b5383 and
 * the era PPC text in build/GPOE8P/asm/os/OSAlloc.s (.text 0x80370A4C..
 * 0x80370D5C: DLInsert 0x80370A4C, OSAllocFromHeap 0x80370AF8, OSFreeToHeap
 * 0x80370BF4, OSSetCurrentHeap 0x80370C70, OSInitAlloc 0x80370C80,
 * OSCreateHeap 0x80370CF0). Era ground truth re-confirmed in the .s:
 *   - 0x80370BEC `addi r3, r6, 0x20`   -> payload = cell + HEADERSIZE(32)
 *   - 0x80370CCC/CE4, 0x80370CFC/D08 `clrrwi rX,rY,5` -> 32-byte ALIGNMENT
 *   - size round-up (size+0x1F)&~0x1F and MINOBJSIZE 64 split rule per the C.
 * This REPLACES the deliberately-simple P2-Q heap shim inside osinit.c
 * (Q's own comment: "the real decomp heap machinery ... lands at ledger step
 * 7 and re-grows on top of these entry points"). Q's observable contract is
 * preserved: same entry-point names, __OSCurrHeap, and OSCheckHeap = live
 * allocation count (a port-side helper; era asm exports no OSCheckHeap).
 *
 * LP64 deviations (all deliberate, documented in docs/p2-bringup-2.md):
 *   V1 pointer-width: era Cell{prev,next,size,hd,requested} = 20B inside a
 *      32B HEADERSIZE (12B slack). Host Cell = {8,8,4(pad4),8,4} = exactly
 *      32B — header size, payload offset (cell+32) and 32B alignment are
 *      bit-identical to era; only field offsets widen.
 *   V2 ASSERTMSGLINE/ASSERTLINE era error paths (dolphin.h panic stubs)
 *      become OSReport + graceful return. Never panic the bring-up host.
 *   V3 (u32) pointer casts in era range checks -> uintptr_t (D1 arena
 *      windows are >4GB on arm64; era truncating cast would miscompare).
 */
#include "pso_bringup.h"
#include <stdint.h>
#include <stdio.h>

#define ALIGNMENT  32
#define HEADERSIZE 32u
#define MINOBJSIZE 64u

typedef struct Cell Cell;
typedef struct HeapDesc HeapDesc;
struct Cell {            /* V1: era field order; header stays EXACTLY
                         * HEADERSIZE=32 as in the era (PPC cell was 20B with
                         * 12B slack; a host pointer-width `hd` would grow
                         * the struct to 40B and spill into the payload —
                         * measured in bring-up). Fix: hd is stored as
                         * u32 (heap index + 1); 0 keeps the era's null
                         * "free cell" sentinel semantics of cell->hd = 0. */
    Cell* prev;          /* +0  (era +0)  */
    Cell* next;          /* +8  (era +4)  */
    s32   size;          /* +16 (era +8)  */
    u32   hd_idx;        /* +20 (era +12) heap+1, 0 == free  */
    s32   requested;     /* +24 (era +16) */
    s32   _pad;          /* +28 -> 32B whole */
};
typedef char era_cell_guard[sizeof(Cell) == HEADERSIZE ? 1 : -1];
struct HeapDesc {        /* era {size, free, allocated, padding, header, payload} */
    s32 size;
    Cell* free;
    Cell* allocated;
    u32 paddingBytes;
    u32 headerBytes;
    u32 payloadBytes;
};

volatile OSHeapHandle __OSCurrHeap = -1;

static HeapDesc* HeapArray;
static int NumHeaps;
static void* ArenaStart;
static void* ArenaEnd;
static long LiveCount[8]; /* port-side: OSCheckHeap contract from P2-Q */

/* ---- era doubly-linked-list helpers, OSAlloc.c:56..148, verbatim ---- */
static Cell* DLAddFront(Cell* list, Cell* cell) {
    cell->next = list;
    cell->prev = 0;
    if (list) {
        list->prev = cell;
    }
    return cell;
}

static Cell* DLLookup(Cell* list, Cell* cell) {
    for (; list; list = list->next) {
        if (list == cell) {
            return list;
        }
    }
    return NULL;
}

static Cell* DLExtract(Cell* list, Cell* cell) {
    if (cell->next) {
        cell->next->prev = cell->prev;
    }
    if (cell->prev == NULL) {
        return cell->next;
    }
    cell->prev->next = cell->next;
    return list;
}

static Cell* DLInsert(Cell* list, Cell* cell) {   /* 0x80370A4C: addr-sorted
                                                   * insert + both-side merge */
    Cell* prev;
    Cell* next;

    for (next = list, prev = NULL; next != 0; prev = next, next = next->next) {
        if (cell <= next) {
            break;
        }
    }

    cell->next = next;
    cell->prev = prev;
    if (next) {
        next->prev = cell;
        if ((u8*)cell + cell->size == (u8*)next) {
            cell->size += next->size;
            next = next->next;
            cell->next = next;
            if (next) {
                next->prev = cell;
            }
        }
    }
    if (prev) {
        prev->next = cell;
        if ((u8*)prev + prev->size == (u8*)cell) {
            prev->size += cell->size;
            prev->next = next;
            if (next) {
                next->prev = prev;
            }
        }
        return list;
    }
    return cell;
}

static int DLOverlap(Cell* list, void* start, void* end) {
    Cell* cell = list;
    while (cell) {
        if (((start <= cell && cell < end) ||
             (start < (void*)((u8*)cell + cell->size) &&
              (void*)((u8*)cell + cell->size) <= end))) {
            return 1;
        }
        cell = cell->next;
    }
    return 0;
}

/* ---- OSInitAlloc, era OSAlloc.c:644..683 / .s 0x80370C80 ---- */
void *OSInitAlloc(void *arenaStart, void *arenaEnd, int maxHeaps) {
    u32 arraySize;
    int i;
    HeapDesc* hd;

    if (!(maxHeaps > 0)) { OSReport("OSInitAlloc(): invalid number of heaps.\n"); return 0; }
    if (!((uintptr_t)arenaStart < (uintptr_t)arenaEnd)) { /* V3 */
        OSReport("OSInitAlloc(): invalid range.\n"); return 0;
    }
    /* era check maxHeaps <= range/24U (era PPC HeapDesc = 24B); port uses
     * the era constant 24 as the sanity floor and the actual array size. */
    arraySize = (u32)(maxHeaps * (s32)sizeof(HeapDesc));
    HeapArray = arenaStart;
    NumHeaps = maxHeaps;
    for (i = 0; i < NumHeaps; i++) {
        hd = &HeapArray[i];
        hd->size = -1;
        hd->free = hd->allocated = 0;
        hd->paddingBytes = hd->headerBytes = hd->payloadBytes = 0;
        if (i < (int)(sizeof LiveCount / sizeof LiveCount[0])) LiveCount[i] = 0;
    }
    __OSCurrHeap = -1;
    arenaStart = (void*)(uintptr_t)(((uintptr_t)((char*)HeapArray + arraySize) + 0x1F)
                                    & ~(uintptr_t)0x1F);   /* era 0x80370CCC clrrwi,5 */
    ArenaStart = arenaStart;
    ArenaEnd = (void*)(uintptr_t)((uintptr_t)arenaEnd & ~(uintptr_t)0x1F);
    if ((uintptr_t)ArenaEnd - (uintptr_t)ArenaStart < MINOBJSIZE) {
        OSReport("OSInitAlloc(): too small range.\n");
    }
    return arenaStart;
}

/* ---- OSCreateHeap, era OSAlloc.c:685..737 / .s 0x80370CF0 ---- */
int OSCreateHeap(void *start, void *end) {
    int heap;
    HeapDesc* hd;
    Cell* cell;

    if (!HeapArray) { OSReport("OSCreateHeap(): heap is not initialized.\n"); return -1; }
    start = (void*)(uintptr_t)(((uintptr_t)start + 0x1F) & ~(uintptr_t)0x1F);
    end   = (void*)(uintptr_t)((uintptr_t)end & ~(uintptr_t)0x1F);
    if (!((uintptr_t)start < (uintptr_t)end)) {
        OSReport("OSCreateHeap(): invalid range.\n"); return -1;
    }
    /* V2 soft clamp instead of era ASSERT 711 on arena range */
    if ((uintptr_t)start < (uintptr_t)ArenaStart) start = ArenaStart;
    if ((uintptr_t)end   > (uintptr_t)ArenaEnd)   end   = ArenaEnd;
    if ((uintptr_t)end - (uintptr_t)start < MINOBJSIZE) {
        OSReport("OSCreateHeap(): too small range.\n"); return -1;
    }
    for (heap = 0; heap < NumHeaps; heap++) {
        hd = &HeapArray[heap];
        if (hd->size < 0) {
            if (DLOverlap(hd->free, start, end) || DLOverlap(hd->allocated, start, end)) {
                continue; /* era DEBUG 723/725 check */
            }
            hd->size = (s32)((uintptr_t)end - (uintptr_t)start);
            cell = start;
            cell->prev = 0;
            cell->next = 0;
            cell->size = hd->size;
            cell->hd_idx = 0;
            cell->requested = 0;
            hd->free = cell;
            hd->allocated = 0;
            hd->paddingBytes = hd->headerBytes = hd->payloadBytes = 0;
            return heap;
        }
    }
    OSReport("OSCreateHeap: Warning - Failed to find free heap descriptor.\n");
    return -1;
}

int OSSetCurrentHeap(int heap) {           /* .s 0x80370C70 */
    int prev;
    if (!HeapArray || !(heap >= 0 && heap < NumHeaps) || HeapArray[heap].size < 0) {
        OSReport("OSSetCurrentHeap(): invalid heap.\n");
        return __OSCurrHeap;
    }
    prev = __OSCurrHeap;
    __OSCurrHeap = heap;
    return prev;
}

/* ---- OSAllocFromHeap, era OSAlloc.c:336..414 / .s 0x80370AF8 ---- */
void *OSAllocFromHeap(int heap, u32 size) {
    HeapDesc* hd;
    Cell* cell;
    Cell* newCell;
    s32 leftoverSize;
    s32 requested;

    requested = (s32)size;
    if (!HeapArray)  { OSReport("OSAllocFromHeap(): heap is not initialized.\n"); return 0; }
    if (requested <= 0) { OSReport("OSAllocFromHeap(): invalid size.\n"); return 0; }
    if (!(heap >= 0 && heap < NumHeaps) || HeapArray[heap].size < 0) {
        OSReport("OSAllocFromHeap(): invalid heap handle.\n"); return 0;
    }

    hd = &HeapArray[heap];
    size += HEADERSIZE;                       /* era HEADERSIZE 32 */
    size = (size + 0x1Fu) & ~0x1Fu;           /* era 32B round-up; align check
                                               * era ASSERT 364 `!((s32)cell & 0x1F)` */

    for (cell = hd->free; cell != NULL; cell = cell->next) {
        if ((signed)size <= (signed)cell->size) {
            break;
        }
    }
    if (cell == NULL) {
        return NULL; /* era DEBUG OSReport warning path */
    }
    if (cell->hd_idx != 0) { OSReport("OSAllocFromHeap(): heap is broken.\n"); return 0; }

    leftoverSize = cell->size - (s32)size;
    if (leftoverSize < (s32)MINOBJSIZE) {     /* take whole cell (era 429) */
        hd->free = DLExtract(hd->free, cell);
    } else {                                  /* split (era 432..448) */
        cell->size = (s32)size;
        newCell = (void*)((u8*)cell + size);
        newCell->size = leftoverSize;
        newCell->hd_idx = 0;
        newCell->prev = cell->prev;
        newCell->next = cell->next;
        if (newCell->next != NULL) {
            newCell->next->prev = newCell;
        }
        if (newCell->prev != NULL) {
            newCell->prev->next = newCell;
        } else {
            hd->free = newCell;
        }
    }

    hd->allocated = DLAddFront(hd->allocated, cell);
    cell->hd_idx = (u32)heap + 1; /* era: cell->hd = &HeapArray[heap] */
    cell->requested = requested;
    hd->headerBytes += HEADERSIZE;
    hd->paddingBytes += (u32)(cell->size - (requested + (s32)HEADERSIZE));
    hd->payloadBytes += (u32)requested;
    if (heap < (int)(sizeof LiveCount / sizeof LiveCount[0])) LiveCount[heap]++;
    return (u8*)cell + HEADERSIZE;            /* era 0x80370BEC addi r3,r6,0x20 */
}

/* ---- OSFreeToHeap, era OSAlloc.c:574..598 / .s 0x80370BF4 ---- */
void OSFreeToHeap(int heap, void *ptr) {
    HeapDesc* hd;
    Cell* cell;

    if (!HeapArray || !ptr) { OSReport("OSFreeToHeap(): bad arg.\n"); return; }
    if (!(heap >= 0 && heap < NumHeaps) || HeapArray[heap].size < 0) {
        OSReport("OSFreeToHeap(): invalid heap handle.\n"); return;
    }
    if ((uintptr_t)ptr % ALIGNMENT != 0) {    /* era ASSERTMSGLINE 580 */
        OSReport("OSFreeToHeap(): invalid pointer.\n"); return;
    }
    cell = (void*)((u8*)ptr - HEADERSIZE);
    hd = &HeapArray[heap];
    if (cell->hd_idx != (u32)heap + 1) { OSReport("OSFreeToHeap(): invalid pointer.\n"); return; }
    if (!DLLookup(hd->allocated, cell)) { OSReport("OSFreeToHeap(): invalid pointer.\n"); return; }

    cell->hd_idx = 0;
    hd->headerBytes -= HEADERSIZE;
    hd->paddingBytes -= (u32)(cell->size - (cell->requested + (s32)HEADERSIZE));
    hd->payloadBytes -= (u32)cell->requested;
    hd->allocated = DLExtract(hd->allocated, cell);
    hd->free = DLInsert(hd->free, cell);
    if (heap < (int)(sizeof LiveCount / sizeof LiveCount[0])) LiveCount[heap]--;
}

/* Port-side helper (P2-Q contract; era exports none — no OSCheckHeap fn in
 * OSAlloc.s). Returns live allocation count for the heap. */
long OSCheckHeap(int heap) {
    return (heap >= 0 && heap < NumHeaps) ? LiveCount[heap] : -1;
}
