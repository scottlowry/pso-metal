# compat/repin — save-format trio re-pin shadows (P2-AW, INERT)

Evidence-pack prototypes for the three LOCKED save-format keeps (TItemData,
SystemSaveData, ChallengeRecord). Nothing in the repo references this dir:
adoption is an integrator decision (`docs/p2-repin-saves-1.md` §7). Enable
per-TU by putting `-Icompat/repin` FIRST on the parse line, together with the
port rebind `-DPSO_PORT_LAYOUT_WAIVER -Ihost/bringup/include` (the shadows
pin era 4-byte scalars and fail loud — negative-size typedefs — without it,
mirroring compat/njs's dependency contract).

Files:

- `pso/item/TItemData.h`, `pso/quest/challenge/ChallengeRecord.h`,
  `pso/ui/frontend/TAdSinglePlyChrSelectGC.h` — era headers copied VERBATIM
  from decomp `pso-decomp @6b38b5383` (field names/types/order unchanged;
  regenerate with `tools/gen_shadows.py`). Each carries a tail pin block
  (`pso_repin_*`, private typedefs, never colliding with LAYOUT_ASSERT
  waiver dispatch) asserting G's card offsets from `docs/p1-format-saves.md`
  §2/§3.1 byte-exactly. Slot census result (§1 of the doc): **zero
  pointer-bearing fields** in these three records — there is nothing to
  re-type to u32 handle slots; unlike compat/njs the shadows convert no
  fields, they PIN the restored byte-truth.
- `dolphin/types.h` — era-guard shadow that deletes the duplicate scalar
  typedef block (s8..u64/uaddr/saddr/vu*/uintptr_t) so the port rebind is
  the single typedef source. Without it the rebind collides with
  `libs/dolphin/include/dolphin/types.h` in 1,144/1,680 corpus TUs (a
  5-error cluster; the AS2 §5 class, tree-wide). Everything else era-verbatim
  (Ptr/uint/BOOL/AT_ADDRESS/…/libc legs).
- `repin_handles.h` — `pso_repin_g2h/h2g` (pso_g2h lineage from compat/njs +
  bringup crt0, slot==0 ↔ NULL preserved). **Provided, not consumed by the
  trio** — landing pad for future slot work (AR §4.3 FloorPopulation gap).
  Runtime obligation per AR pattern: loaders stamp guest handles; for the
  trio the equivalent obligation is the memcpy/bswap boundary (G §5.2),
  i.e. keep the 4-byte words byte-true + BE, never widen, no slot stamping.

Measured effect & recommendation: `docs/p2-repin-saves-1.md`.
