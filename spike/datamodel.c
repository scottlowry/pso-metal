/* P1-J data-model spike (pso-metal Phase 1).
 *
 * Question: the guest (Dreamcast/GC MWCC, ILP32) stores pointers-as-ints all
 * over the format-crossing data. Apple clang arm64 is LP64 and arm64_32 can't
 * execute on macOS. What data model does the port actually run?
 *
 * This spike (1) models the GC 24 MB main arena + 2 MB ARAM as malloc'd
 * blocks behind base "registers", (2) proves a stored u32 handle ->
 * base+handle translation round-trips, including the composite pool-slot
 * handle the decomp already uses (NetBufferRef: entry<<24 | segment<<16), and
 * (3) demonstrates exactly what breaks when the same code assumes a 4-byte
 * pointer: the sizeof(void*) assert set, and truncation of a laundered raw
 * pointer stored in a u32 field.
 *
 * Build (LP64 default on macOS arm64):
 *   cc -std=c11 -Wall -Wextra -o spike/datamodel spike/datamodel.c && spike/datamodel
 * Exit 0 = handle scheme round-trips AND the ILP32-assumption failures fire as
 * predicted (we *expect* the sizeof(void*)==4 checks to fail on LP64; failing
 * to fail would mean the demo is lying).
 */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GC_ARENA_BYTES (24u * 1024u * 1024u) /* MEM1+MEM2 heap the boot spine exposes */
#define ARAM_BYTES     (2u * 1024u * 1024u)  /* DSP ARAM pool (AramPool.cpp) */

/* ---- the port's fixed-width guest types (format-keep side) ---- */
/* WHY: decomp types.h says `typedef unsigned long u32` — true on the ILP32
 * console, but on LP64 `unsigned long` is 8 bytes. The compat layer MUST
 * bind u32 to a fixed 32-bit type or every LAYOUT_ASSERT blows up. */
typedef uint32_t u32g;
typedef uint16_t u16g;

/* uaddr is the seam types.h already reserves: "An integer that holds an
 * address: 32 bits on the console, a pointer's width in a port." */
typedef uintptr_t uaddr;

/* ---- base "registers": one per guest address space ---- */
static uint8_t *g_arena; /* runtime-heap space  (guest: OS arena) */
static uint8_t *g_aram;  /* ARAM space          (guest: 0x??? separate) */

static void *arena_ptr(u32g h)  { return g_arena + h; }           /* base + handle */
static u32g  arena_handle(void *p) { return (u32g)((uaddr)p - (uaddr)g_arena); }

/* NetBufferRef-style composite pool-slot handle (include/pso/network/BufferRef.h) */
static void *poolslot_ptr(u32g h) {
    u32g entry   = h >> 24;
    u32g segment = (h >> 16) & 0xFFu;
    return g_aram + segment * 65536u + entry * 256u; /* 256B slots */
}

/* a guest record that crosses the disc/save format: fixed-width, NO pointers */
typedef struct {
    u32g next;       /* arena-slot handle, not void* */
    u32g name;       /* string-pool offset handle    */
    u16g x, y;       /* mixed 2B/4B run: record is 16B iff u32 stays 4B */
    u32g payload;
} GcRecord;

/* runtime-only struct: real host pointers live HERE and nowhere else */
typedef struct {
    void *cached_ptr; /* rebuilt from handle at load; never serialized */
    u32g  origin_handle;
} RuntimeView;

static int xfails_observed = 0, failures_must_pass = 0, unexpected_passes = 0;
static void check(const char *what, int cond, int expect_ok) {
    printf("  [%s] %-46s : %s\n",
           cond ? "PASS" : (expect_ok ? "FAIL" : "XFAIL"), what,
           cond ? "holds" : "does NOT hold");
    if (cond && expect_ok) return;
    if (!cond && !expect_ok) { xfails_observed++; return; } /* expected to break on LP64 */
    if (expect_ok) failures_must_pass++; else unexpected_passes++;
}

/* the thing the guest code does: launder a pointer through a u32 field */
static void marker_function(void) { }

int main(void) {
    printf("== P1-J data-model spike (host: %s, sizeof(void*)=%zu, sizeof(long)=%zu)\n",
           sizeof(void*) == 8 ? "LP64" : "?", sizeof(void*), sizeof(long));

    printf("\n-- (0) LP64 traps in the era typedefs --\n");
    check("sizeof(unsigned long)==4 (decomp `typedef unsigned long u32`)",
          sizeof(unsigned long) == 4, 0);   /* XFAIL on LP64: the trap */
    check("u32g fixed-width binding keeps GcRecord at 16B",
          sizeof(GcRecord) == 16, 1);

    printf("\n-- (1) ILP32 assumption set the guest body relies on --\n");
    check("sizeof(void*)==4", sizeof(void*) == 4, 0);            /* XFAIL on LP64 */
    check("sizeof(u32g)==sizeof(void*)", sizeof(u32g) == sizeof(void*), 0); /* XFAIL */
    check("pointer difference fits in u32g without truncation",
          sizeof(ptrdiff_t) == sizeof(u32g), 0);                  /* XFAIL */

    printf("\n-- (2) GC arenas as malloc'd blocks behind base registers --\n");
    g_arena = (uint8_t *)malloc(GC_ARENA_BYTES);
    g_aram  = (uint8_t *)malloc(ARAM_BYTES);
    check("24MB main arena + 2MB ARAM malloc'd", g_arena && g_aram, 1);
    memset(g_arena, 0, GC_ARENA_BYTES);
    memset(g_aram,  0, ARAM_BYTES);
    printf("  arena base = %p  aram base = %p\n", (void*)g_arena, (void*)g_aram);

    printf("\n-- (3) stored u32 handle -> base+handle round-trip --\n");
    GcRecord *r = (GcRecord *)(g_arena + 0x400000);  /* pretend heap slot */
    u32g h = arena_handle(r);
    r->next = h; /* self-reference stored as an int, exactly like the guest */
    check("handle fits in 32 bits (< 24MB)", h < GC_ARENA_BYTES, 1);
    check("base+handle lands back on the record", arena_ptr(r->next) == (void*)r, 1);
    RuntimeView view = { .cached_ptr = arena_ptr(r->next), .origin_handle = r->next };
    check("runtime view rebuilds from handle", view.cached_ptr == (void*)r, 1);
    u32g slot = (u32g)((3u << 24) | (5u << 16));  /* BufferRef-style pool handle */
    void *sp = poolslot_ptr(slot);
    check("composite entry<<24|segment<<16 handle decodes inside ARAM",
          (uint8_t *)sp >= g_aram && (uint8_t *)sp < g_aram + ARAM_BYTES, 1);
    printf("  slot handle 0x%08X -> aram offset 0x%05lX\n",
           slot, (unsigned long)((uint8_t *)sp - g_aram));

    printf("\n-- (4) what BREAKS if this body assumes 4-byte pointers --\n");
    /* Same line the decomp uses: `x = (u32)ptr; ... ptr = base + x;`
     * (26 `(u32)this` sites, e.g. src/pso/ui/window/TWindowHelpItem.cpp:176). */
    u32g laundered = (u32g)(uintptr_t)&marker_function;  /* lossless iff pointers<=32b */
    void *restored = (void *)(uintptr_t)laundered;
    printf("  real ptr = 0x%llX   after u32 round-trip = 0x%llX\n",
           (unsigned long long)(uintptr_t)&marker_function,
           (unsigned long long)(uintptr_t)restored);
    check("pointer survives u32 laundering (guest assumption)",
          restored == (void *)&marker_function, 0);      /* XFAIL on LP64 */
    /* base+handle scheme for a *pointer-shaped* value is fine, but taking the
     * address of ANY object outside the arena and laundering it is not —
     * which is why raw-pointer-in-u32 sites need per-site fixes. */
    void *high = g_arena + GC_ARENA_BYTES - 1;
    check("arena-relative handle survives for in-arena objects",
          arena_ptr(arena_handle(high)) == high, 1);

    printf("\n-- verdict inputs --\n");
    printf("  must-pass checks failed : %d\n", failures_must_pass);
    printf("  ILP32 assumptions observed broken on LP64 (XFAIL): %d\n", xfails_observed);
    printf("  expected-to-break checks that held (should be 0) : %d\n", unexpected_passes);
    free(g_arena); free(g_aram);
    return (failures_must_pass || unexpected_passes) ? 1 : 0;
}
