/* P2-HG1 golden — handle-program RUNTIME contract test (docs/p2-handle-program-1.md
 * §5, card t_79d4de8f). This is the gate the 260 RUNTIME-GATED rows wait on:
 * NOTHING of them is adopted before this binary exits 0.
 *
 * Legs: HG1-I0 anchors (bind a REAL base — the crt0 arena; dev D1: macOS
 * refuses fixed guest mappings, so every assert is RELOC-RELATIVE while the
 * era-true guest BYTE VALUES stay formula-exact) · HG1-B bind via the API
 * off the crt0 era-window anchor table (HP §5 HG-1.0: IMAGE/POOL/ARAM) ·
 * HG1-R NJS_HANDLE(T,slot) BYTE-EXACT round-trip (the value must BE
 * 0x80034040, not merely g2h(h2g(p))==p) · HG1-N slot==0 <-> NULL law ·
 * HG1-A ARAM leg incl. L-FLOOR-shaped unbind -> resolver fails loud ·
 * HG1-W stamp-at-genesis WRITE-ONCE · HG1-J NEGATIVE CANARY: W-INT/stack-
 * derived sites MUST loud-reject — one planted known-bad site is asserted
 * to trip (gate-can-fail proof; a green run without the trip is a failure) ·
 * HG1-E exact reject arithmetic + determinism receipt.
 * Shape follows the ladder7 test regime (CHECK -> PASS/FAIL lines, summary,
 * exit 0 iff zero failures). No NJS shadow flags here: this TU crosses the
 * seam directly, it does not compile era bodies.
 */
#include "hg1_windows.h"
#include "pso_bringup.h"
#include <stdio.h>
#include <stddef.h>
#include <string.h>

static int failures, total;
#define CHECK(cond, ...) do { total++; if (!(cond)) { failures++; \
    printf("FAIL "); printf(__VA_ARGS__); printf("\n"); } \
    else { printf("PASS "); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* pooled record, fabricated at arena+0x34040 (POOL window) — HP §5 HG-1.1 */
struct HG1Rec { u32 a; u32 link; f32 v; u32 tail; };

/* sinbd-shaped image record at arena+0x32010 (IMAGE window) */
struct HG1Img { u32 magic; u32 ptr; };

static u32 rejects_since(u32 base) { return pso_handle_reject_count - base; }

/* planted W-INT site in the era dress: HP §2 W-INT "immediates/scalars in
 * pointer dress" + W-DERIVED "host-stack addresses laundered into slots" —
 * this function is the NEGATIVE CANARY. It MUST trip the loud reject. */
static HG1Rec *canary_launder_stack(int *tripped) {
    volatile u32 flag_in_pointer_dress = 0xDu;
    u32 slot = (u32)(uintptr_t)&flag_in_pointer_dress;  /* stack -> slot */
    int shape_off = (slot < PSO_HANDLE_GUEST_RAM_BASE) ||
                    (slot >= PSO_HANDLE_ARAM_CAP);      /* host stack is
        * outside every guest window; low-32 truncation lands between ARAM
        * cap and the tag base (arm64 macOS stacks are > 4 GB), so the
        * resolver MUST classify it unresolvable, never launder it */
    *tripped = !shape_off;
    if (shape_off) return NJS_HANDLE(HG1Rec, slot);      /* -> reject -> NULL */
    return (HG1Rec *)1;                                   /* canary itself bad */
}

int main(void) {
    setvbuf(stdout, 0, _IOLBF, 0);
    pso_handle_test_mode(1);           /* golden harness observes rejects    */

    /* ================= HG1-I0: crt0 anchors are REAL ================= */
    CHECK(pso_bringup_install() == 0 && pso_arena_base != 0,
          "HG1-I0a pso_bringup_install: real host-allocated 26 MB arena bound (dev D1 reloc regime; docs/p2-bringup-1.md)");
    CHECK(pso_aram_base == pso_arena_base + 0x1800000u,
          "HG1-I0b HP §5 HG-1.0 anchor law: pso_aram_base == pso_arena_base + 0x1800000 ([0,24MB) main / [24MB,26MB) ARAM, crt0.c)");
    CHECK(PSO_ERA_WINDOW_COUNT == 3u
          && pso_era_window_anchors[0].win == PSO_W_IMAGE
          && pso_era_window_anchors[0].guest_begin == 0x80032000u
          && pso_era_window_anchors[0].guest_end == 0x80034000u
          && pso_era_window_anchors[1].win == PSO_W_POOL
          && pso_era_window_anchors[1].guest_begin == 0x80034000u
          && pso_era_window_anchors[1].guest_end == 0x80038000u
          && pso_era_window_anchors[2].win == PSO_W_ARAM
          && pso_era_window_anchors[2].guest_end == 0x200000u,
          "HG1-I0c crt0 era-window anchor table == HP §2/§5 window table byte-exact (IMAGE g_sinbd +0x32000, POOL THeap +0x34000, ARAM offset cap 0x200000)");
    CHECK(pso_era_window_host_base(2u) == pso_aram_base
          && pso_era_window_host_base(0u) == pso_arena_base + 0x32000u
          && pso_era_window_host_base(1u) == pso_arena_base + 0x34000u
          && pso_handle_window_records() == 0u,
          "HG1-I0d anchor host bases derive from the two base registers (registry owns nothing); crt0 remains the ONLY base owner (HP §3.1) — registry starts empty");
    u32 rj = pso_handle_reject_count;
    CHECK(pso_njs_handle_g2h(0x80034040u) == 0 && rejects_since(rj) == 1,
          "HG1-I0e unbound window FAILS LOUD pre-bind (nothing resolves before a loader materializes it — stamp-at-genesis law, HP §3.2)");

    /* ================= HG1-B: bind via the API off the crt0 table ================= */
    for (u32 i = 0; i < PSO_ERA_WINDOW_COUNT; i++)
        pso_handle_window_bind(pso_era_window_anchors[i].win,
                               pso_era_window_anchors[i].guest_begin,
                               pso_era_window_anchors[i].guest_end,
                               pso_era_window_host_base(i));
    CHECK(pso_handle_window_records() == 3u,
          "HG1-Ba L-IMAGE/L-POOL/L-ARAM bound off the crt0 anchor table (HG-1.0 set: [arena+0x32000,arena+0x34000) <-> [0x80032000,0x80034000), +0x34000 POOL, ARAM [0,0x200000) <-> pso_aram_base)");
    pso_handle_window_bind(PSO_W_IMAGE, 0x80032000u, 0x80034000u, pso_arena_base + 0x32000u);
    rj = pso_handle_reject_count;
    pso_handle_window_bind(PSO_W_IMAGE, 0x80032000u, 0x80034000u, pso_arena_base + 0x32000u);
    CHECK(pso_handle_window_records() == 3u && rejects_since(rj) == 0,
          "HG1-Bb idempotent same-triple re-bind legal, no reject, no duplicate record (deterministic boot, HP §3.1)");
    rj = pso_handle_reject_count;
    pso_handle_window_bind(PSO_W_IMAGE, 0x80032000u, 0x80034000u, pso_arena_base + 0x90000u);
    CHECK(rejects_since(rj) == 1 && pso_handle_window_records() == 3u
          && pso_njs_handle_g2h(0x80032004u) == pso_arena_base + 0x32004u,
          "HG1-Bc conflicting re-bind fails loud and the ORIGINAL record stands (never silently shadowed)");
    CHECK(pso_handle_slot_in_window(0x80034040u, PSO_W_POOL) == 1
          && pso_handle_slot_in_window(0x80034040u, PSO_W_IMAGE) == 0
          && pso_handle_slot_in_window(0x80032010u, PSO_W_IMAGE) == 1
          && pso_handle_slot_in_window(0x100000u, PSO_W_ARAM) == 1
          && pso_handle_slot_in_window(0x80030000u, PSO_W_POOL) == 0,
          "HG1-Bd slot-space classification (pso_handle_slot_in_window, TChCrLoadReq AR §4.1 precedent): window membership without ever converting");

    /* ================= HG1-R: byte-exact round-trip (HG-1.1) ================= */
    HG1Rec *R = (HG1Rec *)(pso_arena_base + 0x34040u);
    memset(R, 0, sizeof *R);
    u32 s = pso_njs_handle_h2g_main((const u8 *)R);
    CHECK(s == 0x80034040u,
          "HG1-R1a h2g_main(arena+0x34040) == 0x80034040 EXACT — the stored value IS the era-true guest byte address, not just a closed round-trip");
    CHECK(NJS_HANDLE(HG1Rec, s) == R,
          "HG1-R1b NJS_HANDLE(HG1Rec, s) pointer identity back to the fabricated pooled record (macro keeps the AR-shadow name — sites never change twice)");
    CHECK(pso_njs_handle_h2g_main(pso_njs_handle_g2h(s)) == s,
          "HG1-R1c h2g(g2h(s)) == s byte-exact");
    CHECK(pso_njs_handle_g2h(s + (u32)offsetof(HG1Rec, link)) == (u8 *)R + offsetof(HG1Rec, link),
          "HG1-R1d field-of-record: g2h(s + offsetof(link)) lands exactly on R.link (slot arithmetic stays in guest space)");
    HG1Img *S = (HG1Img *)(pso_arena_base + 0x32010u);
    CHECK(pso_njs_handle_h2g_main((const u8 *)S) == 0x80032010u
          && NJS_HANDLE(HG1Img, 0x80032010u) == S,
          "HG1-R1e IMAGE window leg closes at the g_sinbd placement span (0x80032010 byte-exact) — one base register, per-window classification");
    rj = pso_handle_reject_count;
    CHECK(pso_njs_handle_h2g_main(pso_arena_base + 0x33ffcu) == 0x80033ffcu
          && pso_njs_handle_g2h(0x80034000u) == pso_arena_base + 0x34000u
          && rejects_since(rj) == 0,
          "HG1-R1f exclusive window ends exact: last IMAGE byte 0x80033ffc stamps; 0x80034000 resolves into POOL (adjacent spans never overlap, never reject at the seam)");

    /* ================= HG1-N: NULL law (HG-1.2) ================= */
    CHECK(pso_njs_handle_g2h(0u) == 0, "HG1-N2a g2h(0) == NULL (era sentinel semantics stay honest, 92 (uaddr)x==0 tests, J §1b)");
    rj = pso_handle_reject_count;
    CHECK(pso_njs_handle_h2g_main(0) == 0u && pso_njs_handle_h2g_aram(0) == 0u
          && rejects_since(rj) == 0
          && pso_handle_slot_in_window(0u, PSO_W_POOL) == 0,
          "HG1-N2b h2g(NULL) == 0 BOTH spaces, no reject; slot 0 belongs to no window (offset 0 is NOT a valid object address — no sentinel collision)");

    /* ================= HG1-A: ARAM leg + L-FLOOR unbind (HG-1.3) ================= */
    u8 *ap = pso_aram_base + 0x100000u;   /* quest_download-style stamp       */
    u32 as_ = pso_njs_handle_h2g_aram(ap);
    CHECK(as_ == 0x100000u && pso_njs_handle_g2h(as_) == ap,
          "HG1-A3a ARAM leg byte-exact: h2g_aram(pso_aram_base+0x100000) == 0x100000 (ARAlloc-offset space, NOT 0x80-tagged) and g2h dispatches back to the same host byte");
    pso_handle_window_unbind(PSO_W_ARAM);
    rj = pso_handle_reject_count;
    CHECK(pso_njs_handle_g2h(0x100000u) == 0 && rejects_since(rj) == 1
          && pso_handle_window_records() == 2u,
          "HG1-A3b L-FLOOR-shaped window_unbind(ARAM): resolver FAILS LOUD on the orphaned slot (floor unload law)");
    pso_handle_window_bind(PSO_W_ARAM, 0u, PSO_HANDLE_ARAM_CAP, pso_aram_base);
    CHECK(pso_njs_handle_g2h(0x100000u) == ap,
          "HG1-A3c rebind re-materializes the window (loader owns base, once — floor reload path closes)");
    rj = pso_handle_reject_count;
    CHECK(pso_njs_handle_h2g_aram(pso_aram_base + 0x40u) == 0xFFFFFFFFu
          && rejects_since(rj) == 1,
          "HG1-A3d non-ARAlloc-shaped offset (0x40 < 0x1000 floor) refused at the STAMP side too (era ARAlloc bases are aligned, >= 0x80000 — offset-space collision guard)");

    /* ================= HG1-W: stamp-at-genesis WRITE-ONCE (§3.2) ================= */
    u32 w = pso_handle_stamp_once(&R->link, pso_arena_base + 0x34100u, PSO_W_POOL);
    CHECK(w == 0x80034100u && R->link == 0x80034100u && pso_handle_stamp_count() == 1u,
          "HG1-W1a stamp_once: the slot WRITE happens once, from a registered host pointer, value era-true (0x80034100); ledger holds 1 genesis stamp");
    rj = pso_handle_reject_count;
    u32 w2 = pso_handle_stamp_once(&R->link, pso_arena_base + 0x34200u, PSO_W_POOL);
    CHECK(rejects_since(rj) == 1 && R->link == 0x80034100u && w2 == 0x80034100u
          && pso_handle_stamp_count() == 1u,
          "HG1-W1b SECOND stamp at the same slot location fails loud and the bytes NEVER move (write-once law enforced by real read-back, ledger unchanged)");
    rj = pso_handle_reject_count;
    volatile u32 stack_slot = 0;
    u32 w3 = pso_handle_stamp_once(&R->tail, (const u8 *)&stack_slot, PSO_W_POOL);
    CHECK(rejects_since(rj) == 1 && w3 == 0xFFFFFFFFu && R->tail == 0u
          && pso_handle_stamp_count() == 1u,
          "HG1-W1c stamp from an UNREGISTERED host pointer (a stack address — W-DERIVED dress) is rejected BEFORE the write lands (read sites never stamp what they did not materialize)");

    /* ================= HG1-J: NEGATIVE CANARY + reject legs (HG-1.4) ================= */
    rj = pso_handle_reject_count;
    HG1Rec *bad = NJS_HANDLE(HG1Rec, 0xDu);
    CHECK(bad == 0 && rejects_since(rj) == 1,
          "HG1-J4a the W-INT guard: g2h(0xD) LOUD-REJECTS (immediate-in-pointer-dress can never fabricate an ARAM/main-RAM pointer — HP §5 reject leg, the law protecting the 12 W-INT rows)");
    rj = pso_handle_reject_count;
    int tripped = 0;
    HG1Rec *canary = canary_launder_stack(&tripped);
    CHECK(tripped == 0 && canary == 0 && rejects_since(rj) == 1,
          "HG1-J4b NEGATIVE CANARY TRIPS: the planted W-INT stack-derived site (a real host-stack address laundered into a u32 slot) LOUD-REJECTS -> NULL — gate-can-fail proof for this suite");
    rj = pso_handle_reject_count;
    CHECK(pso_njs_handle_g2h(0x80030000u) == 0 && pso_njs_handle_g2h(0x80038000u) == 0
          && rejects_since(rj) == 2,
          "HG1-J4c main-RAM gaps/over-run reject (no record covers 0x80030000; 0x80038000 is POOL's exclusive end) — range law, never launder");
    u32 total_expected = pso_handle_reject_count;
    rj = pso_handle_reject_count;
    CHECK(pso_njs_handle_g2h(0x80034040u) == (u8 *)R && rejects_since(rj) == 0,
          "HG1-J4d contrast leg INSIDE the reject storm: registered slots still resolve byte-exact — rejects fire per-slot, the registry never got wedged");

    /* ================= HG1-E: arithmetic + determinism receipt ================= */
    CHECK(total_expected == 10u && pso_handle_window_records() == 3u,
          "HG1-E1 reject arithmetic exact: 10 planted rejects (I0e Bc A3b A3d W1b W1c J4a J4b J4cx2) all observed, 0 missing; 3 live window records at exit");
    /* non-test mode aborts on the SAME code path (fail_loud !g_test_mode ->
       abort) — the golden harness arms test mode; no silent path exists. */
    printf("== P2-HG1 golden: %d/%d PASS (rejects=%u) ==\n",
           total - failures, total, pso_handle_reject_count);
    return failures != 0;
}
