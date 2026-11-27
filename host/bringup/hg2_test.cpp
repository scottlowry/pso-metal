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
 * file hand-types no census vectors (the only literals here are the two names
 * STA1 §5 itself quotes for the HG2.2 legs, held as triple-against-generated
 * cross-checks, and the "f256_mc" replacement the memmove path invents).
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

/* HG2.2 fabricated consumer shapes — era slot-space forms (NJS_TEXNAME 3 x u32
 * per compat GinjaMaterialState.h; NJS_TEXLIST {textures@0, nbTexture@4} per
 * NinjaTypes.h:65-68). Golden-internal mirrors: no era TU is linked here. */
struct HG2TexName { u32 filename, attr, texture; };
struct HG2TexList { u32 textures, nbTexture; };
struct HG2Rec     { u32 textures; };

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
    /* ============ HG2.2 consumer leg: texlist chain-walk (STA1 §5) ============
     * TEffect.cpp:10/:98 shape without touching batch-3's files: a POOL record
     * stamps its texlist slot, the texlist stamps its entries slot, entries
     * hold PSO_ST_SLOT values — texlist->texname->string ALL resolve through
     * the SAME registry the 81 sites rely on. */
    HG2Rec *rec = (HG2Rec *)(pso_arena_base + 0x34040u);
    HG2TexList *L = (HG2TexList *)(pso_arena_base + 0x34100u);
    HG2TexName *E = (HG2TexName *)(pso_arena_base + 0x34200u);
    memset(rec, 0, sizeof *rec); memset(L, 0, sizeof *L);
    memset(E, 0, 3 * sizeof *E);
    E[0].filename = PSO_ST_SLOT(0);    /* "f008_white_npr01" (class A)       */
    E[1].filename = PSO_ST_SLOT(54);   /* "ki_u256_vo_mo_moji001" (class B)  */
    E[2].filename = PSO_ST_SLOT(81);   /* clear_texlist_names store shape    */
    L->nbTexture = 3u;
    u32 ts = pso_handle_stamp_once(&rec->textures, (const u8 *)L, PSO_W_POOL);
    u32 es = pso_handle_stamp_once(&L->textures, (const u8 *)E, PSO_W_POOL);
    CHECK(ts == 0x80034100u && rec->textures == 0x80034100u,
          "HG2.2a genesis stamp 1: record@+0x34040 textures-slot := h2g_main(arena+0x34100) == 0x80034100 (TEffect.cpp:10 effect_texture_file shape)");
    CHECK(es == 0x80034200u && L->textures == 0x80034200u && pso_handle_stamp_count() == 2u,
          "HG2.2b genesis stamp 2: list@+0x34100 textures-slot := h2g_main(arena+0x34200) == 0x80034200; ledger holds exactly 2 genesis stamps");
    HG2TexList *Lw = NJS_HANDLE(HG2TexList, rec->textures);
    CHECK(Lw == L && Lw->nbTexture == 3u,
          "HG2.2c link-1 resolve: rec->textures -> fabricated NJS_TEXLIST (registry walk, not fallback arithmetic)");
    HG2TexName *tn = NJS_HANDLE(HG2TexName, Lw->textures);
    CHECK(tn == E, "HG2.2d link-2 resolve: L->textures -> entries array (chain-walk like TEffect.cpp:98)");
    const char *w0 = (const char *)NJS_HANDLE(char, tn[0].filename);
    const char *w54 = (const char *)NJS_HANDLE(char, tn[1].filename);
    const char *wr = NJS_HANDLE(char, tn[2].filename);
    CHECK(tn[0].filename == PSO_ST_SLOT(0) && w0 && strcmp(w0, pso_st_name[0]) == 0
          && strcmp(w0, "f008_white_npr01") == 0,
          "HG2.2e entry0: filename slot 0x%08X -> staged \"%s\" byte-exact (the STA1 §1 rewrite pattern end-to-end)",
          tn[0].filename, w0 ? w0 : "?");
    CHECK(tn[1].filename == PSO_ST_SLOT(54) && w54 && strcmp(w54, pso_st_name[54]) == 0
          && strcmp(w54, "ki_u256_vo_mo_moji001") == 0,
          "HG2.2f entry1: cross-block cell 54 -> staged \"%s\" byte-exact (one window owns ALL four TUs)",
          w54 ? w54 : "?");
    CHECK(tn[2].filename == PSO_ST_SLOT(81) && wr && cell_tail_zero((const u8 *)wr, 0),
          "HG2.2g RIDER: texlist_null_name cell resolves to 32 zero bytes — the clear-store target (NinjaUtil.cpp:12) + every aliased reader resolve INSIDE the window (STA1 §2 RIDER)");
    rj = pso_handle_reject_count;
    u32 cellval = PSO_ST_SLOT(79);
    u8 *c79 = pso_njs_handle_g2h(cellval);
    memmove(c79, "f256_mc", 8);        /* era replace_gvm_texture_directory / */
                                       /* move_data write-INTO-the-name path  */
    CHECK(rejects_since(rj) == 0 && cellval == PSO_ST_SLOT(79)
          && pso_njs_handle_g2h(cellval) == c79
          && strcmp((const char *)c79, "f256_mc") == 0
          && c79[8] == (u8)'o' && cell_tail_zero(c79, 23),
          "HG2.2h era write fidelity (optional STA1 §5 extension leg): memmove into cell 79 (TAdMcSelectCursorGC.cpp:20 payload) lands in the RW arena — slot value unchanged, bytes updated (new name visible, era remnant byte[8]='o' stands exactly as era move_data leaves it), ZERO rejects (a host .rodata target would SIGSEGV; the staging design is what keeps this path era-true)");

    /* ============ HG2.3 NEGATIVE CANARY — exactly one (STA1 §5) ============ */
    rj = pso_handle_reject_count;
    u8 *got = pso_njs_handle_g2h(0x80081000u);   /* EXCLUSIVE end of the tree */
    CHECK(got == 0 && rejects_since(rj) == 1,
          "HG2.3a NEGATIVE CANARY TRIPS: g2h(0x80081000) — the static tree's exclusive end, covered by NO record — LOUD-REJECTS to NULL with reject_count +1 (gate-can-fail proof; a 4 KB -> 4 KB+32 span drift trips this)");
    CHECK(got != pso_arena_base + 0x81000u
          && pso_arena_base + (0x80081000u - PSO_HANDLE_GUEST_RAM_BASE) < pso_aram_base,
          "HG2.3b legacy-single-base fallback TRIPPED: the AR-shadow body (base + slot - 0x80000000) would resolve 0x80081000 to arena+0x81000, a LIVE byte INSIDE [arena,aram) — silently; the registry-backed resolver refused it, so this golden rode the window contract (why STA1 §5 calls this the right canary)");
    rj = pso_handle_reject_count;
    CHECK(pso_njs_handle_g2h(PSO_ST_SLOT(0)) == pso_arena_base + PSO_ST_ARENA_OFF
          && rejects_since(rj) == 0,
          "HG2.3c contrast leg after the canary (HG1-J pattern): PSO_ST_SLOT(0) still resolves byte-exact — the registry never got wedged");
    CHECK(pso_handle_reject_count == 2u && pso_handle_window_records() == 4u
          && pso_handle_stamp_count() == 2u,
          "HG2.4 reject arithmetic exact: 2 planted rejects (HG2.0c unbound-pre-stage + HG2.3a exclusive-end), 0 missing; 4 window records + 2 genesis stamps at exit");
    printf("== P2-HG2 golden (legs 0-3): %d/%d PASS (rejects=%u) ==\n",
           total - failures, total, pso_handle_reject_count);
    return failures != 0;
}
