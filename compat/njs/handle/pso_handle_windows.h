/* P2-HP uaddr-handle program — STAMPING-SEAM API SKETCH (compat/njs/handle).
 * INERT: nothing references this directory; no build line adds -I to it.
 * This is the contract shape for docs/p2-handle-program-1.md, NOT adopted
 * code. It must stay parse-clean under clang -fsyntax-only (C11 and C++17)
 * with no repo headers — hence the self-contained scalar typedefs guarded
 * below (they rebind to the repo's types.h when one is on the -I chain,
 * which nothing does today).
 *
 * Lineage: compat/njs/pso/system/NinjaTypes.h (AR shadow) declares the
 * single-base helpers pso_njs_g2h/h2g/NJS_HANDLE against crt0's
 * pso_arena_base (docs/p2-njs-repin-1.md §2). That regime is RUNTIME-CORRECT
 * only for slots whose payload lives in the 24 MB main-RAM window. NA's 260
 * RUNTIME-GATED rows (docs/p2-njs-adopt-1.md §4) touch payload in MORE than
 * one guest window, and several carry values that are not addresses at all.
 * The handle program generalizes the seam: a WINDOW REGISTRY bound by the
 * loaders, stamp-at-genesis conversion, and slot-space comparison rules.
 */
#ifndef PSO_NJS_HANDLE_SKETCH_H
#define PSO_NJS_HANDLE_SKETCH_H

/* ---- scalars (sketch-local; real adoption takes include/types.h rebind) ---- */
#ifndef PSO_HANDLE_SCALARS_DEFINED
#define PSO_HANDLE_SCALARS_DEFINED
#include <stdint.h>
typedef uint32_t pso_u32;
typedef int32_t  pso_s32;
typedef uintptr_t pso_uaddr_sketch; /* the port's `uaddr` (p1-datamodel §3 r2) */
#endif

/* ================= 1. Window classes (docs/p2-handle-program-1.md §2) =====
 * One guest address space per BASE REGISTER; one registry record per window.
 * Main-RAM windows (POOL/IMAGE/HEAP/CACHE/RESERVE) share pso_arena_base and
 * are distinguished by RANGE (classification, range-check, fail-loud on
 * unregistered slots). ARAM is the second base register. Code/stack/int
 * payloads are NOT windows — they are per-site classes that MUST NOT be
 * base+slot stamped (§5). */
enum pso_handle_window {
    PSO_W_NONE = 0,      /* slot is zero  <=> NULL sentinel (rule, §3.1)     */
    PSO_W_POOL,          /* main RAM: runtime object/node pools (THeap       *
                          * obj_heap/alt_heap, TLobby nodes, BmlResource      *
                          * model/motion pools) — guest 0x80000000..0x81800000*/
    PSO_W_IMAGE,         /* main RAM: image-resident statics (POF0-patched    *
                          * NJS trees, tables, g_sinbd placement regime)      */
    PSO_W_HEAP,          /* main RAM: sys_heap window (OSCreateHeap via       *
                          * memory.cpp; xmalloc/mem_alloc/operator-new legs,  *
                          * TQuestVmDownLoadServer::alloc copies)             */
    PSO_W_CACHE,         /* main RAM: locked-cache heap window (OSInitAlloc;  *
                          * AU-4 carve; AV §1.4-S4 texel/pixels regime)       */
    PSO_W_RESERVE,       /* main RAM: boot reserve carve (framebuffers,       *
                          * GX FIFO, global xmalloc buffers — SystemBootInit  *
                          * init_game_systems lo..heap span)                  */
    PSO_W_ARAM,          /* AUXILIARY RAM 2 MB: ARAlloc-offset space          *
                          * (floor_set_archive_pool AramBlocks,               *
                          * quest_download_aram_base, ad_warning_aram_base)   *
                          * -> pso_aram_base (crt0 back-half of the arena)    */
    PSO_W__COUNT
};

/* Registry record. Windows are registered by their LOADER at the moment the
 * span exists (stamp-at-genesis law, §3.2): nothing may resolve a slot whose
 * window has no record. */
struct pso_handle_window_rec {
    enum pso_handle_window win;
    pso_u32  guest_begin;  /* tag-space address of the window start          */
    pso_u32  guest_end;    /* exclusive                                    */
    unsigned char *host_begin; /* host address of guest_begin              */
    /* host_end is host_begin + (guest_end - guest_begin): contiguous model,
     * exactly the crt0 arena regime (26 MB: main [0,24MB), ARAM [24MB,26MB)).*/
};

/* Base registers — OWNED by host/bringup/crt0.c (pso_arena_base /
 * pso_aram_base). Declared, never defined here (parse-clean, never links). */
extern unsigned char *pso_arena_base; /* guest 0x80000000 == pso_arena_base  */
extern unsigned char *pso_aram_base;  /* ARAM window host base               */

#define PSO_GUEST_RAM_BASE 0x80000000u
#define PSO_ARAM_WINDOW_CAP 0x00200000u /* 2 MB; ARAlloc returns offsets     */

/* ================= 2. Loader-side binding (the seam this program adds) =====
 * Each loader calls pso_handle_window_bind() ONCE when its window materializes
 * on the host, passing the host base for the window's guest start. Idempotent
 * re-bind with the same triple is legal (deterministic boot); a conflicting
 * re-bind must fail loud (abort), never silently shadow. */
void pso_handle_window_bind(enum pso_handle_window win,
                            pso_u32 guest_begin, pso_u32 guest_end,
                            unsigned char *host_begin);
void pso_handle_window_unbind(enum pso_handle_window win); /* floor unload   */

/* ================= 3. The crossing helpers (runtime-correct versions) ======
 * Invariants the golden test HG-1 pins (docs/p2-handle-program-1.md §5):
 *  3.1 NULL law:  slot 0  <=> pso_njs_g2h gives NULL; h2g(NULL) gives 0.
 *      Never invent a 0x80000000 offset-0 sentinel — slot==0 is era-true
 *      "no object" and must keep resolving to NULL.
 *  3.2 STAMP-AT-GENESIS: a slot is written with pso_njs_h2g* exactly once, by
 *      the allocator/loader that materialized the object, from a host pointer
 *      INSIDE a bound window. Read sites only convert (NJS_HANDLE), never
 *      re-stamp from an unregistered pointer.
 *  3.3 RANGE LAW: g2h of a slot with no covering record must fail loud in
 *      debug (and is UNDEFINED in release — same posture as AR §5: C sites
 *      forbidden without -Wint-conversion, no silent laundering).
 *  3.4 ARAM tagging: ARAlloc offsets are small integers that RANGE-OVERLAP
 *      nothing in main RAM, but they collide with smuggled immediates
 *      (W-INT rows: (saddr)((void*)0xD) flags). So ARAM h2g takes the window
 *      tag explicitly; g2h dispatches arena-regex first (g >= 0x80000000),
 *      then bound ARAM window, then fails loud — never "guesses" ARAM for an
 *      unregistered value. */
static inline unsigned char *pso_njs_handle_g2h(pso_u32 slot) {
    /* sketch body — real one walks the registry; kept call-shaped here so a
     * later lane can drop in the table-driven version without API churn: */
    (void)slot;
    return (unsigned char *)0;
}
static inline pso_u32 pso_njs_handle_h2g_main(const unsigned char *p) {
    return p ? (pso_u32)(PSO_GUEST_RAM_BASE + (p - pso_arena_base)) : 0u;
}
static inline pso_u32 pso_njs_handle_h2g_aram(const unsigned char *p) {
    return p ? (pso_u32)(p - pso_aram_base) : 0u; /* ARAlloc-offset space    */
}

/* The macro every era access site migrates to. SAME NAME contract as the AR
 * shadow's NJS_HANDLE (compat/njs/pso/system/NinjaTypes.h) — when the registry
 * lands, the shadow's one-base definition is superseded by this multi-window
 * one; site code never changes twice. slot==0 -> NULL preserved. */
#ifndef NJS_HANDLE
#define NJS_HANDLE(T, slot) ((T *)(pso_njs_handle_g2h(slot)))
#endif

/* Slot-space comparison law (AR §4.1 TChCrLoadReq precedent; W-INT rows):
 * validity/range tests on slot payloads run on the u32 slot, never on a
 * converted pointer:  pso_handle_slot_in_window(s, w)  instead of
 * g2h(s) >= some host base. NULL test is `slot == 0` (== era `x == 0`). */
int pso_handle_slot_in_window(pso_u32 slot, enum pso_handle_window win);

#endif /* PSO_NJS_HANDLE_SKETCH_H */
