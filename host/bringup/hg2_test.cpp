/* P2-HG2 golden — W-STATIC-TREE static-init byte-exact test
 * (docs/p2-static-init-1.md §5, card t_e554b48a). Gate for ANY of the 81
 * site edits: nothing in the authoring lane rewrites an initializer before
 * this binary exits 0.
 *
 * Legs: HG2.0 stage+bind (unbound-reject mirror of HG1-I0e -> pso_static_-
 * tree_stage() -> 4 records; idempotent re-stage/re-bind) · HG2.1 THE
 * ACCEPTANCE LEG — per-cell k in 0..81 value-law (formula vs python-generated
 * table, index drift trips) + crossing (g2h byte-exact, reloc-relative, dev
 * D1 honest) + content (era string bytes verbatim, zeros to cell end; rider
 * cell 81 = 32 zero bytes) · HG2.2 consumer leg (TEffect.cpp:98-style
 * texlist chain-walk over the registry, rider-cell clear-store shape, the
 * replace_gvm_texture_directory memmove-into-payload era write path) ·
 * HG2.3 NEGATIVE CANARY: the window's EXCLUSIVE END g2h(0x80081000) MUST trip
 * — the legacy single-base fallback (base + slot - 0x80000000) would resolve
 * it to a LIVE arena byte silently, so this leg fails green only on the
 * registry-backed resolver (gate-can-fail proof, STA1 §5).
 * Expected vectors: pso_st_{blob,name,slot,cell_len,site}[] are EMITTED by
 * host/bringup/gen_hg2_static_tree.py from the era sources READ-ONLY — this
 * file hand-types no census bytes (the one literal in HG2.2 is the shorter
 * replacement name the era memmove path itself invents at runtime).
 */
#include "hg2_static_tree.h"
#include "pso_bringup.h"
#include <stdio.h>
#include <string.h>

static int failures, total;
#define CHECK(cond, ...) do { total++; if (!(cond)) { failures++; \
    printf("FAIL "); printf(__VA_ARGS__); printf("\n"); } \
    else { printf("PASS "); printf(__VA_ARGS__); printf("\n"); } } while (0)

static u32 rejects_since(u32 base) { return pso_handle_reject_count - base; }

/* zeros [from,to) inside cell k's staged bytes */
static int cell_tail_zero(const u8 *host, u32 from) {
    for (u32 j = from; j < PSO_ST_CELL; j++) if (host[j]) return 0;
    return 1;
}

int main(void) {
    setvbuf(stdout, 0, _IOLBF, 0);
    pso_handle_test_mode(1);           /* golden harness observes rejects    */

    /* ================= HG2.0 stage + bind (STA1 §5) ================= */
    CHECK(pso_bringup_install() == 0 && pso_arena_base != 0,
          "HG2.0a arena installed (stage runs AFTER pso_bringup_install — 'stage-before-first-resolve' ordering law, STA1 §3)");
    for (u32 i = 0; i < 3u; i++)       /* HG-1.0 set; row 3 is L-IMAGE's own  */
        pso_handle_window_bind(pso_era_window_anchors[i].win,
                               pso_era_window_anchors[i].guest_begin,
                               pso_era_window_anchors[i].guest_end,
                               pso_era_window_host_base(i));
    CHECK(pso_handle_window_records() == 3u,
          "HG2.0b HG-1.0 rows 0..2 bound; the static tree is NOT registered yet");
    u32 rj = pso_handle_reject_count;
    CHECK(pso_njs_handle_g2h(PSO_ST_GUEST_BEGIN) == 0 && rejects_since(rj) == 1,
          "HG2.0c UNBOUND-reject precondition: g2h(0x80080000) fails loud (no record covers it yet — mirrors HG1-I0e; nothing resolves before the loader stages it)");
    CHECK(pso_era_window_anchors[3].win == PSO_W_IMAGE
          && pso_era_window_anchors[3].guest_begin == 0x80080000u
          && pso_era_window_anchors[3].guest_end == 0x80081000u
          && pso_era_window_anchors[3].arena_off == 0x00080000u,
          "HG2.0d crt0 anchor[3] == STA1 §3 row byte-exact (W-STATIC-TREE [0x80080000,0x80081000) <-> arena_off 0x80000, L-IMAGE)");
    CHECK(pso_era_window_host_base(3u) == pso_arena_base + PSO_ST_ARENA_OFF,
          "HG2.0e host base derives from the two base registers — crt0 remains the ONLY base owner, the tree never mmaps (HP §3.1)");
    rj = pso_handle_reject_count;
    CHECK(pso_static_tree_stage() == 0 && rejects_since(rj) == 0
          && pso_handle_window_records() == 4u,
          "HG2.0f pso_static_tree_stage(): blob memcpy'd to arena+0x80000 + the program's ONE pso_handle_window_bind (STA1 §3) -> 4 records, zero rejects");
    CHECK(pso_njs_handle_g2h(PSO_ST_GUEST_BEGIN) == pso_arena_base + PSO_ST_ARENA_OFF,
          "HG2.0g window span opens: g2h(0x80080000) == pso_arena_base + 0x80000");
    rj = pso_handle_reject_count;
    CHECK(pso_static_tree_stage() == 0 && pso_handle_window_records() == 4u
          && rejects_since(rj) == 0,
          "HG2.0h same-triple re-stage/re-bind is idempotent (HG1-Bb law): 0 added rejects, no duplicate record");
    CHECK(pso_njs_handle_g2h(PSO_ST_GUEST_END - 1u) == pso_arena_base + 0x80000u + 0xFFFu,
          "HG2.0i last IN-WINDOW byte 0x80080FFF resolves (seam complement; the exclusive end is HG2.3's canary)");

    /* ============ HG2.1 byte-exact resolution — THE acceptance leg ============ */
    /* Rollups over the python-generated vectors first (generator truth vs
       STA1 §2 set-math), then the per-cell triple. */
    int vec_ok = 1, distinct_ok = 1;
    u32 maxl = 0;
    for (u32 k = 0; k < PSO_ST_CELLS; k++) {
        u32 l = pso_st_cell_len[k];
        if (l > maxl) maxl = l;
        if (pso_st_name[k]) {
            if (l == 0 || l > PSO_ST_CELL) vec_ok = 0;
            if (l && (u32)strlen(pso_st_name[k]) + 1u != l) vec_ok = 0;
            for (u32 j = 0; j < l && vec_ok; j++)
                if (pso_st_blob[k][j] != (u8)pso_st_name[k][j]) vec_ok = 0;
            if (!cell_tail_zero(pso_st_blob[k], l)) vec_ok = 0;
        } else if (k == PSO_ST_SITES) {           /* rider: 32 zero bytes */
            if (l != 0 || !cell_tail_zero(pso_st_blob[k], 0)) vec_ok = 0;
        } else vec_ok = 0;
        for (u32 m = k + 1; m < PSO_ST_CELLS; m++)
            if (pso_st_name[k] && pso_st_name[m] &&
                strcmp(pso_st_name[k], pso_st_name[m]) == 0) distinct_ok = 0;
    }
    CHECK(sizeof(pso_st_blob) == 82u * 32u && sizeof(pso_st_blob) <= 0x1000u,
          "HG2.1r1 STA1 §2/§3 set math: padded blob 82x32 = %lu B fits the 4 KB window",
          (unsigned long)sizeof(pso_st_blob));
    CHECK(vec_ok,
          "HG2.1r2 generated vectors self-consistent: blob row == name NUL-padded to cell end, rider cell 81 = 32 zero bytes (texlist_null_name, NinjaUtil.cpp:7)");
    CHECK(distinct_ok && maxl <= 32u,
          "HG2.1r3 all 81 census names distinct, max payload %u B <= 32 (STA1 §2 'no dup collapse', 'cells are 32 B')", maxl);

    for (u32 k = 0; k < PSO_ST_CELLS; k++) {
        /* (a) VALUE LAW — table vs formula: an index-order drift trips here */
        CHECK(pso_st_slot[k] == PSO_ST_SLOT(k)
              && pso_st_slot[k] == 0x80080000u + 32u * k,
              "HG2.1a[%2u] value law: pso_st_slot == PSO_ST_SLOT(%u) == 0x80080000+32*k == 0x%08X (pure contract arithmetic, no host term — STA1 §4 grammar test)",
              k, k, pso_st_slot[k]);
        /* (b) CROSSING — byte-exact, reloc-relative (dev D1 honest) */
        CHECK(pso_njs_handle_g2h(pso_st_slot[k]) ==
              pso_arena_base + PSO_ST_ARENA_OFF + 32u * k,
              "HG2.1b[%2u] crossing: g2h(0x%08X) == pso_arena_base + 0x80000 + 32*%u (reloc-relative, era-true guest bytes)",
              k, pso_st_slot[k], k);
        /* (c) CONTENT — era bytes verbatim, zeros to cell end (no alias) */
        const u8 *host = pso_njs_handle_g2h(pso_st_slot[k]);
        int ok;
        if (pso_st_name[k])
            ok = host && strcmp((const char *)host, pso_st_name[k]) == 0
                 && cell_tail_zero(host, pso_st_cell_len[k]);
        else
            ok = host && cell_tail_zero(host, 0);   /* rider: all zero */
        CHECK(ok, "HG2.1c[%2u] content byte-exact @%s: \"%s\"%s", k,
              pso_st_site[k], pso_st_name[k] ? pso_st_name[k] : "(rider)",
              pso_st_name[k] ? "" : " = 32 zero bytes");
    }
    printf("== P2-HG2 golden (legs 0-1): %d/%d PASS (rejects=%u) ==\n",
           total - failures, total, pso_handle_reject_count);
    return failures != 0;
}
