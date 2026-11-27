# compat/repin — re-pin + dialect shadows (P2-AW trio, P2-MA MWCC-dialect; INERT)

Evidence-pack prototypes. Nothing in the repo build files references this dir:
adoption is an integrator decision (`docs/p2-repin-saves-1.md` §7,
`docs/p2-mwccasm-1.md` §4). Enable by putting `-Icompat/repin` FIRST on the
parse line, together with the port rebind `-DPSO_PORT_LAYOUT_WAIVER
-Ihost/bringup/include` (the trio shadows pin era 4-byte scalars and fail
loud — negative-size typedefs — without it, mirroring compat/njs's dependency
contract; the MA dialect shadows degrade safe: their only rewrite is the
`AT_ADDRESS` macro neutralization, active exactly where clang could not parse
the era form anyway).

PIN/COLLISION LAWS (apply to every file here): `-Icompat/repin` ONLY, NEVER
`-include` any file in this dir (apply4 caveat: force-including repin broke
the ceil-chain in 5 TUs). The collision pin
(`compat/collision/pin_dolphin_types.h`, 3rd in the 3-file -include list)
occupies `_DOLPHIN_TYPES_H_` before any edge fires; MA shadows therefore carry
their OWN era guards (`_DOLPHIN_GXVERT`, `_DOLPHIN_OSEXEC_H_`, `_DOLPHIN_DVD_H`,
`PSO_QUEST_QUESTDOWNLOADSERVER_H`), which the pin never touches, and their
preamble `#undef`s the pin's carried era-form `AT_ADDRESS` at first site hit.

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

P2-MA MWCC-asm/decl dialect shadows (collision-debt §4.5 class, ~38 TUs):

- `dolphin/gx/GXVert.h`, `dolphin/os/OSExec.h`, `dolphin/dvd.h`,
  `pso/quest/QuestDownloadServer.h` — era headers copied VERBATIM
  (regenerate with `tools/gen_dialect_shadows.py`, pinned line numbers assert
  on drift) + ONE preamble block before the first `AT_ADDRESS(` site:
  `#undef`/empty-redefine of `AT_ADDRESS(addr)` (era types.h spells it
  `: (addr)`, an MWCC absolute-address declarator clang rejects; neutral form
  per P2-T precedent `compat/dialect/pso_dialect_compat.h` §1 — "a port
  defines the pin empty", placement is host/linker territory). The macro fix
  is TU-global: it also repairs era sites reached WITHOUT their own shadow
  (os.h:125-136, OSLink.h:27-28, dsp/dsp.h:10). Spelling classes NOT
  shadow-repairable (per-TU `asm void f(){}` bodies, musyx bare `extern "C"`
  in .c, hw_regs raw `: 0x…` pins) are inventoried in `docs/p2-mwccasm-1.md`
  §3 with pointers to the P2-T `compat/dialect` prototype (different -I dir;
  its landing is that lane's decision, not folded here).

Measured effect & recommendation: `docs/p2-repin-saves-1.md` (trio),
`docs/p2-mwccasm-1.md` (dialect class).
