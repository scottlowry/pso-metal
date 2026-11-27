# compat/njs/adapters/batch2/ — INERT adapter-queue batch-2 index (P2-AQP2)

Nothing here is referenced by any build line; no adapter code lives in this
directory (inert landing only). It exists so `compat/njs/adapters/` has a
stable batch-2 slot and points at the single source of truth:

- Deliverable doc: `docs/p2-adapter-queue-2.md` —
  §3 the remaining-queue enumeration: 139 files / 509 NA §3 sites (reconciles
  169-30, 673-164), python-counted fresh apply-8 `-fsyntax-only` sweep
  (waiver era `d4382fcd…` measured == pin; 139/139 FAIL at NJS shape,
  509/509 sites fire, totals 786 == NA row sums);
  §3.2 197 un-enumerated no-matching-function cascade sites on queue files +
  §3.3 27 on batch-1 files (batch-1 §5.2 pick-up);
  §4 the 64-error static-initializer cluster split — HP stamping 54
  (`CameraCapture.cpp:35-88` .rodata filename slots; new `W-STATIC-TREE`
  obligation for P2-HP) / adapter batch-3 10 (same-TU links, h2g-store with
  init hooks per shipped MericarolTexture/TCharaMaking precedent → AQP3) /
  other 0 — plus 30 same-shape static-init riders found in queue files
  (HP +27 / batch-3 +3), so HP and AQP3 each own a disjoint, site-listed set.
- Probe regime for consumers: identical laws to batch-1 — `-Icompat/njs`
  `.cpp`-only, three-`-include` order, facade before `host/bringup/include`,
  waiver pin `d4382fcd…` era, never regen. Receipts: profile
  `cache/scratch/aqp2/{NJS,BASE}/err/`.
