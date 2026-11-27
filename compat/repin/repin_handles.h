/* compat/repin/repin_handles.h — handle-slot lineage for FUTURE save-adjacent
 * shadows (e.g. AR §4.3 FloorPopulation gap). The P2-AW trio (TItemData,
 * SystemSaveData, ChallengeRecord) consumes NONE of it: slot census
 * docs/p2-repin-saves-1.md §1 proves zero pointer-bearing fields, so the trio
 * re-pin is width-only (port types.h rebind) — the POF0 analogue: the copy
 * path through the card boundary stays unchanged, 4-byte words all the way.
 * Reuse the pso_g2h lineage VERBATIM (AR compat/njs + bringup crt0): a slot
 * is a guest address; h2g translates, g2h stamps. slot==0 <-> NULL.
 * Requires pso_arena_base (loader lane, same as AR). */
#ifndef PSO_REPIN_HANDLES_H
#define PSO_REPIN_HANDLES_H
#include "types.h"
#ifndef PSO_NJS_ARENA_GUEST_BASE
#define PSO_NJS_ARENA_GUEST_BASE 0x80000000u
#endif
static inline void *pso_repin_g2h(u32 slot, unsigned char *arena_base) {
    return slot ? arena_base + (slot - PSO_NJS_ARENA_GUEST_BASE) : (void *)0;
}
static inline u32 pso_repin_h2g(const void *p, unsigned char *arena_base) {
    return p ? PSO_NJS_ARENA_GUEST_BASE + (u32)((const unsigned char *)p - arena_base) : 0u;
}
#endif
