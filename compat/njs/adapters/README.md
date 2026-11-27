# compat/njs/adapters/ — INERT adapter supply (P2-AQP batch-1)

Nothing here is referenced by any build line; this directory documents +
indexes paste-ready NJS adapter specs. THIS IS NOT TREE EDITING: the decomp
tree stays read-only; the slice generator consumes the edits.

- The full deliverable is `docs/p2-adapter-queue-1.md`: per-file
  (site | fix-shape | probe-status) tables + unified-diff blocks for the
  boot-proximal top 30 ADAPTER-QUEUE files from `docs/p2-njs-adopt-1.md` §3
  (169 rows / 673 sites), authored in the AV §3 five-shape taxonomy
  (g2h-load / h2g-store / NJS_HANDLE-slot / slot-space-compare / slot-subscript)
  against the helpers in `compat/njs/pso/system/NinjaTypes.h:43-53`.
- Probe regime: apply-7 official LP64 shape + `-Icompat/njs` (C++ only),
  waiver era `d4382fcd0659357f` — 16/30 files probe fully CLEAN, 0 remaining
  pointer↔slot-class errors across all 30 (receipts: profile
  `cache/scratch/aqp/{BEFORE,AFTER}/err/`).
- Diff blocks are the single source of truth; no copies live here (the
  scratch `diffs/` mirror is lane-local evidence, not repo content).

