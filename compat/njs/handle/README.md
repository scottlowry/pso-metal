# compat/njs/handle — P2-HP uaddr-handle program STAMPING SEAM (sketch, INERT)

Nothing in this repo references this directory: no build line, no `-I`, no
`-include`. This is the CONTRACT SHAPE for `docs/p2-handle-program-1.md`,
supplied inert per the P2-HP card — exactly the AR/NC posture for
`compat/njs/pso/**` (measure/design now, runtime lands with a builder lane).

- `pso_handle_windows.h` — the multi-window seam: window registry
  (`pso_handle_window_bind`), per-space crossing helpers
  (`pso_njs_handle_g2h` / `pso_njs_handle_h2g_main` / `pso_njs_handle_h2g_aram`),
  the `NJS_HANDLE(T, slot)` migration target (same name as the AR shadow's —
  sites never change twice), and the slot-space-comparison helper
  (`pso_handle_slot_in_window`). NULL law kept: slot==0 <-> NULL.

Relationship to the AR shadow (`compat/njs/pso/system/NinjaTypes.h`): the
shadow's `pso_njs_g2h` is single-base (main-RAM arena only). The handle
program supersedes it at RUNTIME with the registry-backed version; typing
(the shadow's job) does not move. Adoption order: HG-1 golden (doc §5) passes
in a builder lane → registry lands in host/bringup → shadow's helper bodies
migrate here → the RUNTIME-GATED 260 rows (docs/p2-njs-adopt-1.md §4) start
their per-row obligations (doc §4 table).
