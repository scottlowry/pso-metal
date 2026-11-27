/* P2-HG1 — handle-program window registry, RUNTIME form (host/bringup).
 * This is docs/p2-handle-program-1.md's contract (compat/njs/handle/
 * pso_handle_windows.h sketch) WIRED REAL per the HG-1 card: crt0 owns the
 * two base registers (pso_arena_base / pso_aram_base) and the era-window
 * anchor table below; host/bringup/hg1_windows.cpp owns the registry, the
 * crossing helpers and the stamp-at-genesis ledger. compat/njs/handle/ stays
 * INERT (read-intake only — no -I is ever pointed at it; names and shapes are
 * kept aligned with the sketch so a later shadow-migration lane swaps bodies,
 * never spellings).
 *
 * Window classes/spans = HP §2 table. The HG-1.0 bind set (doc §5) is:
 *   PSO_W_IMAGE  arena+0x32000..0x34000  <-> guest [0x80032000,0x80034000)
 *                (V5b g_sinbd placement regime, docs/p2-v-wave.md Deviation
 *                 ledger V2; era .data home TObjSinBoard.cpp:37-40)
 *   PSO_W_POOL   arena+0x34000..0x38000  <-> guest [0x80034000,0x80038000)
 *                (ladder6 L6_POOL_OFF/L6_SURF_OFF texture-state/surface
 *                 pools = era THeap obj_heap regime shape, THeap.cpp
 *                 operator-new path; HP §2 L-POOL)
 *   PSO_W_ARAM   [0x0,0x200000) offset-space <-> pso_aram_base
 *                (ARAlloc(0x100000) item_drop_memory_pool_ar_addr,
 *                 TSetManager.cpp:1561 + AramPool floor_set_archive_pool
 *                 :1529 + L-FLOOR row-heap regime; HP §2 L-ARAM/L-FLOOR)
 * The remaining main-RAM classes (HEAP/CACHE/RESERVE) share pso_arena_base
 * with these and get bound by their loaders (L-SYSHEAP memory.cpp:26-29,
 * L-CACHE OSInitAlloc SystemBootInit.cpp:272, L-RESERVE init_game_systems
 * :264-271) in later lanes — the AU-4 carve regime already exists at
 * arena+0x1400000 (ladder5/6 CACHEHEAP_START_OFF) but binds nothing yet.
 */
#ifndef HG1_WINDOWS_H
#define HG1_WINDOWS_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- window classes (identical enum to compat/njs/handle sketch §1) ---- */
enum pso_handle_window {
    PSO_W_NONE = 0,     /* slot 0 <=> NULL sentinel (NULL law)              */
    PSO_W_POOL,         /* main RAM: runtime object/node pools              */
    PSO_W_IMAGE,        /* main RAM: image-resident statics (g_sinbd ...)   */
    PSO_W_HEAP,         /* main RAM: sys_heap window                        */
    PSO_W_CACHE,        /* main RAM: locked-cache heap window               */
    PSO_W_RESERVE,      /* main RAM: boot reserve carve                     */
    PSO_W_ARAM,         /* AUX RAM 2 MB: ARAlloc-offset space               */
    PSO_W__COUNT
};

/* ---- guest-space constants (HP §2/§3.2) ---- */
#define PSO_HANDLE_GUEST_RAM_BASE 0x80000000u
#define PSO_HANDLE_ARAM_CAP       0x00200000u  /* 2 MB ARAM window           */
/* W-INT guard (HP §3.4: "converter must reject unregistered small ints
 * loudly"): a real ARAlloc base is 32-byte aligned and never lives under
 * the first 4 KB of offset space (era allocations observed: 0x80000,
 * 0x100000 — TAdWarningBaseGC.cpp:101, TSetManager.cpp:1561). */
#define PSO_HANDLE_ARAM_MIN_OFFSET 0x00001000u

/* ---- crt0-owned era-window anchors (crt0.c defines the table; base
 * registers are pso_arena_base/pso_aram_base, dev D1: ASLR-respecting mmap,
 * so every assertion in the golden is RELOC-RELATIVE — the era-true guest
 * BYTE VALUES (0x80032000...) are formula-derived and address-independent) */
struct PsoEraWindowAnchor {
    enum pso_handle_window win;
    u32 guest_begin;         /* tag-space window start                       */
    u32 guest_end;           /* exclusive                                    */
    u32 arena_off;           /* host base = pso_arena_base + arena_off       */
    const char *era_cite;    /* what carves this span in the era             */
};
#define PSO_ERA_WINDOW_COUNT 3u   /* IMAGE, POOL, ARAM (HG-1.0 bind set)     */
extern const struct PsoEraWindowAnchor pso_era_window_anchors[PSO_ERA_WINDOW_COUNT];
u8 *pso_era_window_host_base(u32 i); /* crt0.c — derived from the anchors    */

/* ---- registry (hg1_windows.cpp). Idempotent same-triple re-bind is legal
 * (deterministic boot, HP §3.1); a CONFLICTING re-bind fails loud.
 * Multi-span per class allowed (span counts boot-bounded). ---- */
#define PSO_HANDLE_WINDOW_MAX 16u
void pso_handle_window_bind(enum pso_handle_window win,
                            u32 guest_begin, u32 guest_end, u8 *host_begin);
void pso_handle_window_unbind(enum pso_handle_window win); /* floor unload   */
u32  pso_handle_window_records(void);                       /* live records   */

/* ---- crossing helpers (runtime-correct versions of AR-shadow single-base
 * pso_njs_g2h/h2g; resolution order per HP §3.2: 0 -> NULL; slot >=
 * 0x80000000 -> covering main-RAM record; ARAM-offset (aligned, >= MIN) ->
 * covering ARAM record; ANYTHING ELSE -> FAIL LOUD ---- */
u8  *pso_njs_handle_g2h(u32 slot);
u32  pso_njs_handle_h2g_main(const u8 *p);  /* host ptr -> guest 0x80xxxxxx  */
u32  pso_njs_handle_h2g_aram(const u8 *p);  /* host ptr -> ARAlloc offset    */
int  pso_handle_slot_in_window(u32 slot, enum pso_handle_window win);

/* The migration-target macro: SAME NAME as the AR shadow's NJS_HANDLE
 * (compat/njs/pso/system/NinjaTypes.h:53) — site code never changes twice.
 * slot==0 -> NULL preserved. */
#ifndef NJS_HANDLE
#define NJS_HANDLE(T, slot) ((T *)(pso_njs_handle_g2h(slot)))
#endif

/* ---- stamp-at-genesis, write-once (HP §3.2): the value is computed from
 * the host pointer through the window-tagged converter (so an unregistered
 * host pointer is rejected BEFORE any write), then written EXACTLY ONCE per
 * slot location. A second stamp at a location fails loud and leaves the
 * bytes untouched. Returns the slot value the location holds. ---- */
#define PSO_HANDLE_STAMPS_MAX 64u
u32 pso_handle_stamp_once(u32 *slot_loc, const u8 *host_ptr,
                          enum pso_handle_window win);
u32 pso_handle_stamp_count(void);

/* ---- fail-loud plumbing: every reject prints "HG1-REJECT ..." to stderr
 * and bumps pso_handle_reject_count. Test mode (golden harnesses) returns
 * control to the caller with the sentinel result; non-test mode ABORTS —
 * there is no silent path, ever (AR §5 posture). ---- */
extern volatile u32 pso_handle_reject_count;
void pso_handle_test_mode(int on);

#ifdef __cplusplus
}
#endif

#endif /* HG1_WINDOWS_H */
