# P2-BA — V-wave receipts (build-heavy host/gx + host/bringup lane)

Lane BA, 2026-11-27. Contract: `docs/p2-gx-tex-consumer.md` (AV). Tasks per
the BA brief (V1 NJS_HANDLE resolution · V2 texlist walk · V3 CLUT expand ·
V4 LOGO VISIBLE), mechanism specs per AV §2–§5. Clause R1 honored: no
hardcoded waived counts anywhere in lane code or these receipts; the D8
sha pin (`host/bringup/waiver-header.sha256`) was recorded at intake
(`c6b514035f39b633…`) and re-hashed after EVERY build run (all equal —
zero regen; `make parse` never invoked).

## V1 — NJS_HANDLE resolution for the 3 on-logo sites (per-TU AR allow-list)

What landed (commit ``, files: `host/bringup/include/ladder6_tex_prelude.h`,
`host/bringup/gen_ladder6_slices.py`, `host/bringup/bringup.mk`):

- `gen_ladder6_slices.py` — guarded generator: 37 VERBATIM slices of the AV
  §1.3 consumer chain off the READ-ONLY decomp (`@6b38b5383`, source shas
  printed per run: dd5e0 `951c3a9e8448`, da798 `31ba2586e8c9`, gtt
  `67f094f35ef7`), hard-gated anchors (exact-unique signature, column-0 `}`
  terminator, fail-loud before any write), and the enumerated **V1 adapter
  table = 17 count-gated edits** implementing AV §3:
  - **#1 `GinjaTextureTable.cpp:18-28` `set_texture_name`** — 2×
    `entry->filename = pso_njs_h2g(input)` (both attr legs, per §3 row 1).
  - **#2 `psoLoadTexture.cpp:89`** — `NJS_HANDLE(NJS_TEXNAME, texlist->textures)->filename`
    + `pso_njs_g2h` for the `%s` leg (psoLoadTexture loop sliced verbatim).
    **#2b `LoadPvmFileTls` (:97-101/:142-153)** — enumerated, NOT sliced in
    this rung: its builder deps (TLanguage/GvmFileHeader/tmp_buf class-(d)
    homes) are off the sinbd contract path; status = open, listed below.
  - **walk derefs** `text_803DD5E0.cpp` — attr dispatch + the 3
    `filename`-slot + 2 `chunk` sites → `NJS_HANDLE`/`g2h`/`h2g`
    (S4 guest-stamp regime: texel src travels as u32 handles end-to-end;
    `copy_texture_data_and_flush` converts at the SOLE deref via
    `pso_njs_g2h`).
  - **#3 draw-side subscripts (GinjaMaterial.cpp:367/388/421 class)** —
    `set_texture_num` (same chain, text_803DA798.cpp:34) resolved through
    the double `NJS_HANDLE`; `set_texlist_texture` + `resolve_texlist_entry`
    slot writes via `pso_njs_h2g`. The literal GinjaMaterial.cpp fn bodies
    are not ported yet (DRAW_TPL :478/:495 untouched per AR §4.5 — status
    open, same pattern).
  - **LB-7** `init_texture_from_gvr_desc` era `texture->p + 4` → `+ 8`
    (LP64 header-in-surface widening; proven by the header goldens, V2).
  - **LB-9** psoBe register-boundary loads (`p2an_be32/p2an_be16` through
    `ladder4_supply.o`) of the walk-copied desc + GBIX id — the D1
    "port-must-add-swap" side named in `p2-tex-supply` AN-2; R3 accessor
    rule honored (bytes stay byte-true, only the load mirrors PPC lwz).
- **Per-TU allow-list**: `NJS6_CXXFLAGS := -Icompat/njs -I<decomp>/libs/dolphin/include`
  FIRST, applied to EXACTLY the ladder6 `.cpp` TUs (consumer slice + test);
  zero `.c` TU receives it (AR §5 silent-truncation ban). The AV §4
  build/Makefile + `build/njs_allowlist.txt` edit stays **submitted-not-
  applied** (integrator-owned; this lane cannot touch build/ — the §4
  paste-ready block in the contract doc is the submission).
- Deviations LB-1..LB-9 declared in `ladder6_tex_prelude.h` (class-(d)
  allocator/frame-pump seams = AP §5 injection precedent; LB-3 fail-loud
  loader stand-ins for the 3 unreachable texture_loaders legs; LB-6/LB-8
  single-TU linkage-regime normalizations of era cross-file puns).

**17-chain list status** (AR §3 chain-24; re-measured zero-build from the
committed AR doc + this lane's battery): 17/17 owner TUs keep the AR
FAIL→PASS pair flip (nothing this rung edits the decomp side, flips are
immutable against `6b38b5383`). Of those 17, the 3 on-logo-file legs:
`GinjaTextureTable.cpp` fully resolved + executing (V2); `psoLoadTexture.cpp`
:77-94 resolved + executing, :142-153 enumerated open; `GinjaMaterial.cpp`
pattern resolved at the `set_texture_num` leg, 3 literal sites open. The
other 14 chain TUs remain parse-pending on the RW-stitch/unreviewed-type
residuals exactly as AR measured (no NJS block, no BA change).

Gate: `make -C host/bringup -f bringup.mk build/bringup/gen/ladder6_tex_consumer.o`
RC=0 (shadow regime compiles clean — every adapter crossing typed); full
suite `sh host/bringup/run.sh` RC=0, 110/110 PASS (prior boot/prs/Y/AE/AN/AU
lines verbatim green), waiver sha unchanged pre/post.

## V2 — texlist walk (TLogo refs resolve against the AU-loaded GVR chain)

Landed (`__V2__`: `host/bringup/ladder6_test.cpp` + binary rule + run.sh
line). `load_sin_board_image(rawGVR, 0x41C)` — ONE era call — drives the
whole chain with zero host-side lookups (gate L6, 18/18 PASS, suite now
128/128):

- L6-G3/G4: `set_texture_name` stamped `filename == pso_njs_h2g(&input)` and
  `attr == 0x40000000`; the ERA `load_texture_entry` switch dispatched
  `texture_loaders[1]` (union-slot aliasing of `global_index` by the bound
  `.texture` handle is era-true — one field, three names).
- L6-G5/G6: `find_gvrt_texture_chunk` (AE/AN slice) read the GBIX id raw →
  `surface->id == 0x999` established by the era walk (LB-9 made the PPC-load
  semantics explicit on LP64).
- L6-G7: format-9 palette leg KEPT `tlut_name == 0` (not −1) per
  text_803DA798.cpp:148-152 — contract §5-V3 golden honored.
- L6-G8/G9: header `{9,32,32,1024}` written through the era upload chain
  via the LB-7 `texture->p + 8` slot (proof the widened surface layout is
  the one the era body addressed); pixels heap-resident (AU-4 carve).
- L6-G10 RECEIPT: `reload_texture_slot_pixels → copy_texture_pixel_range →
  move_data(g2h(u32 src))` delivered fnv1a64 `0xA92D59BE5839FCE5` == AN
  fixture texels in the SURFACE header pixels (same anchor ladder5 E4a
  pinned at the raw alloc — supply and consumption now agree inside the
  era chain).
- L6-G11: `texture_error.code == 4` asserted as the era RESIDUAL of the
  attempt-1 magic-miss on the GinjaTextureInput record (0x803D9D5C-:9D68
  store, fallback leg resolved) — named, not hidden.
- L6-G12/G13: draw selection `set_texture_list/set_texture_num/select_-
  texture_by_surface_id` runs through the double NJS_HANDLE (V1 §3#3
  pattern executing, not just compiling).
- L6-G14: second `load_texture_list` shares the surface
  (`active_texture_surface_count == 1`) — palette reference leg.

Deviation ledger V2: LB-1 (alloc seam = AU-4 cache heap), LB-2 (frame-pump
seams untouched on success — counted), LB-3 (loader stand-ins never fired).
Sinbd .data block placed arena-resident (`g_sinbd` @+0x32000, pool @+0x34000/+0x38000)
per AR §5 slot-space rule — the h2g/g2h round-trip only closes inside the
guest window.

Gate: run.sh full-suite RC=0, 128/128 PASS (prior 110 verbatim), waiver sha
c6b51403… unchanged pre/post.

## V3 — PAL8 CLUT expand into the AQ display path

status: pending.

## V4 — LOGO VISIBLE

status: pending.
