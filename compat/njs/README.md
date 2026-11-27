# compat/njs — P2-AR NJS handle re-pin shadow (PROTOTYPE, NOT ADOPTED)

Nothing in this repo references this directory. To opt a TU in, put
`-I<repo>/compat/njs` FIRST in its include chain (earliest -I wins,
assert-policy §4 / host/bringup/include/types.h precedent), together with:
the port types.h rebind (`-Ihost/bringup/include` +
`-DPSO_PORT_LAYOUT_WAIVER`) — with the era `unsigned long u32` this shadow is
meaningless — and the committed compat+waiver force-includes.

- `pso/system/NinjaTypes.h` — NJS_OBJECT.model/child/sibling,
  NJS_TEXLIST.textures, NJS_MOTION.mdata, NJS_MDATA2.p[], NJS_MKEY_P.key,
  NJS_CNK_MODEL.vertex_list/polygon_list -> u32 image-handle slots (AJ
  obligation 1) + `NJS_HANDLE/pso_njs_g2h/pso_njs_h2g` (bringup crt0 arena).
- `pso/render/GinjaMaterialState.h` — NJS_TEXNAME.filename + texaddr/texture
  union -> u32 slots.

C++ TUs only (C silently launders ptr<->u32 under -w). Evidence + decision
memo: `docs/p2-njs-repin-1.md`.
