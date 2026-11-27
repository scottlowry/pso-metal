# compat/njs/handle/static — W-STATIC-TREE registration contract (sketch, INERT)

Nothing in this repo references this directory: no build line, no `-I`, no
`-include`. This is the PASTE SHAPE for `docs/p2-static-init-1.md` (P2-STA1,
fleet wave-5, card `t_f52a5a63`) — the handle-program obligation AQP2
`docs/p2-adapter-queue-2.md` §4 named: the **81 static-init sites** (54
`CameraCapture.cpp:35-88`, 25 `TBoss3VoloptMonitor.cpp:77-101`,
`TAdMcSelectCursorGC.cpp:20`, `TAdMcSelectMcGC.cpp:14`) whose `NJS_TEXNAME`
filename slots hold guest addresses of compiled-in strings. Verdict: all 81
**guest-literal-able** — slot value = integer constant expression over the
window contract, payload bytes = loader-staged arena cells. Zero
runtime-hook, zero blocked. `compat/njs/handle/**` posture applies (read-intake
only; names aligned with `pso_handle_windows.h` / `host/bringup/include/
hg1_windows.h` so migration lanes swap bodies, never spellings).

## The sketch (paste-ready; NOT compiled by anything)

```c
/* one global blob, census order = docs/p2-static-init-1.md §2 table index */
#define PSO_ST_GUEST_BEGIN 0x80080000u   /* proposed span; §6 A2 re-audit   */
#define PSO_ST_GUEST_END   0x80081000u
#define PSO_ST_ARENA_OFF   0x00080000u   /* host = pso_arena_base + this    */
#define PSO_ST_CELLS       82u           /* 81 names + texlist_null_name    */
#define PSO_ST_CELL        32u           /* era loader idiom (str += 0x20,  */
                                         /*  [0x20] buffer, name[32])       */
/* THE slot law — pure contract arithmetic, no host term, NA §4 ban-clean:  */
#define PSO_ST_SLOT(k)     (PSO_ST_GUEST_BEGIN + PSO_ST_CELL * (u32)(k))

static const char pso_st_blob[PSO_ST_CELLS][PSO_ST_CELL] = {
    "f008_white_npr01",        /* k=0  -> CameraCapture.cpp:35  (rewritten: */
                               /*        { PSO_ST_SLOT(0), 0, NULL } )       */
    /* ... k=1..80 in §2 census order (generator-owned, era strings exact) */
    { 0 },                     /* k=81 = texlist_null_name (NinjaUtil.cpp:7)*/
};
```

```c
/* crt0.c — pso_era_window_anchors row #4 (PSO_ERA_WINDOW_COUNT 3->4):      */
{ PSO_W_IMAGE, 0x80080000u, 0x80081000u, 0x00080000u,
  "W-STATIC-TREE string tree (STA1; texnames + texlist_null_name)" },

/* L-IMAGE loader (single bind owns ALL 81 sites — windows, not TUs):      */
void pso_static_tree_stage(void) {                 /* before first resolve  */
    memcpy(pso_arena_base + PSO_ST_ARENA_OFF, pso_st_blob, sizeof pso_st_blob);
    pso_handle_window_bind(PSO_W_IMAGE, PSO_ST_GUEST_BEGIN,
                           PSO_ST_GUEST_END,
                           pso_arena_base + PSO_ST_ARENA_OFF);
}
/* NO unbind, ever (never unloaded). RW arena keeps the era
   replace_gvm_texture_directory memmove-into-name path (text_803DD5E0.cpp
   :334) era-true; host .rodata would SIGSEGV — this is why staging, not
   casting, is the landing. */
```

## Invariants (all inherited, none new)

* Value must be authored as `PSO_ST_SLOT(k)` ONLY — any expression containing
  a host address (`(u32)&blob[k]`, `h2g` in a constant context) is the
  `addr-or-this->int` laundering NA §4 bans; the ban's own cure ("loader
  stamping + guest↔host translation at these sites") is exactly bind+stage.
* Read sites cross via `NJS_HANDLE(char, tn->filename)` /
  `pso_njs_handle_g2h` (registry, HP §3.2 resolution order); NULL law,
  stamp-at-genesis ledger, slot-space comparison, 4-byte slots unchanged.
* Cell fit is compile-time-checked: census max strlen+1 = 24 ≤ 32; blob
  82×32 = 2624 B ≤ 4096 B span; HG-2.1 asserts value+content per cell.
* HG-2 golden (doc §5): stage-before-first-resolve order leg, 81×3 byte-exact
  leg (value law / crossing / content+NUL), consumer chain-walk leg, and the
  ONE negative canary `g2h(PSO_ST_GUEST_END)` → must fail loud (the legacy
  single-base body resolves it silently — the canary pins the registry).
* Out of scope: the 13 batch-3 TEXLIST link slots (array-address stores),
  W-STATIC-TREE tranche-2 (tables themselves arena-resident), era-absolute
  .rodata addresses (no map at @6b38b5383; audit item A4).
