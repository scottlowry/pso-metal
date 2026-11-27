/* P2-HG2 — W-STATIC-TREE contract (docs/p2-static-init-1.md §3, card t_e554b48a).
 * The static-init string tree: 81 AQP2-census texname payloads + the
 * texlist_null_name rider staged as compile-time-visible cells inside ONE
 * PSO_W_IMAGE window, so every era initializer becomes pure contract
 * arithmetic (STA1 §1) and every slot resolves byte-exact through the HG-1
 * registry (STA1 §5 HG2.1). compat/njs stays INERT; this is the landed form.
 *
 * Slot law (STA1 §1/§4): PSO_ST_SLOT(k) is an INTEGER CONSTANT EXPRESSION over
 * window-contract constants only — no cast, no &symbol, no host term is even
 * expressible here. That is the grammar line that keeps the class outside the
 * NA §4 ban (STA1 §4 "Forbidden boundary").
 */
#ifndef HG2_STATIC_TREE_H
#define HG2_STATIC_TREE_H

#include "hg1_windows.h"   /* window enum, anchors, bind/g2h API */

#ifdef __cplusplus
extern "C" {
#endif

/* ---- span (STA1 §3): guest [0x80080000, 0x80081000) <-> arena+0x80000,
 * 4 KB window, L-IMAGE stamped. Collision posture (crt0.c 4th-anchor comment)
 * is re-audited at every land; HG2 golden HG2.3 pins the exclusive end. ---- */
#define PSO_ST_GUEST_BEGIN 0x80080000u
#define PSO_ST_GUEST_END   0x80081000u
#define PSO_ST_ARENA_OFF   0x00080000u
#define PSO_ST_CELL        32u                  /* STA1 §2: cells are 32 B    */
#define PSO_ST_SITES       81u                  /* census §2 (54+25+1+1)      */
#define PSO_ST_CELLS       (PSO_ST_SITES + 1u)  /* + rider cell 81 (§2 RIDER) */

/* The authored-site form (STA1 §1): { "lit", ... } -> { PSO_ST_SLOT(idx), ... } */
#define PSO_ST_SLOT(k) (PSO_ST_GUEST_BEGIN + PSO_ST_CELL * (u32)(k))

/* ---- vectors emitted by host/bringup/gen_hg2_static_tree.py (the builder-
 * lane census generator, STA1 §3 "one source of truth"): blob bytes are the
 * era .rodata-equivalent payloads NUL-padded to cell end; name/slot/cell_len/
 * site are the golden's expected vectors (never hand-typed). pso_st_name[81]
 * == NULL marks the rider cell — its content law is 32 zero bytes. ---- */
extern const u8   pso_st_blob[PSO_ST_CELLS][PSO_ST_CELL];
extern const char *const pso_st_name[PSO_ST_CELLS];
extern const u32  pso_st_slot[PSO_ST_CELLS];
extern const u32  pso_st_cell_len[PSO_ST_CELLS];   /* strlen+1; rider 0        */
extern const char *const pso_st_site[PSO_ST_CELLS];

/* ---- L-IMAGE staging (STA1 §3 "Stage"): one-time memcpy of the blob to
 * arena+0x80000 + the program's ONE pso_handle_window_bind off the crt0
 * anchor row. Legal call site: after pso_bringup_install(), before the first
 * name-resolving era init ("stage-before-first-resolve", STA1 §3). Re-stage
 * is idempotent (same bytes, same-triple re-bind is legal per HG1-Bb).
 * Returns 0 iff installed-and-staged; -1 if the arena does not exist yet. ---- */
int pso_static_tree_stage(void);

#ifdef __cplusplus
}
#endif

#endif /* HG2_STATIC_TREE_H */
