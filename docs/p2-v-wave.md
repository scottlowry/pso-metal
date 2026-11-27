# P2-BA — V-wave receipts (build-heavy host/gx + host/bringup lane)

Lane BA, 2026-11-27. Contract: `docs/p2-gx-tex-consumer.md` (AV). Tasks per
the BA brief (V1 NJS_HANDLE resolution · V2 texlist walk · V3 CLUT expand ·
V4 LOGO VISIBLE), mechanism specs per AV §2–§5. Clause R1 honored: no
hardcoded waived counts anywhere in lane code or these receipts; the D8
sha pin (`host/bringup/waiver-header.sha256`) was recorded at intake
(`c6b514035f39b633…`) and re-hashed after EVERY build run (all equal —
zero regen; `make parse` never invoked).

## V1 — NJS_HANDLE resolution for the 3 on-logo sites (per-TU AR allow-list)

What landed (commit `08c7db1`, files: `host/bringup/include/ladder6_tex_prelude.h`,
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

Landed (commit `3d8fea0`: `host/bringup/ladder6_test.cpp` + binary rule + run.sh
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

Landed (`host/gx/dl/gx_logo_clut.h` + `GX_DL_TEXN` fragment variant in
`host/gx/dl/gx_dl_metal.m`):

- CPU CLUT-expand -> RGBA8 feeding the PROVEN AQ entry
  `gxh_dl_set_texture(w,h,rgba8)` exactly per AV §2.2 bullet list
  (RGBA8Unorm/ShaderRead/bpr=w*4/default-Shared — the getBytes roundtrip
  regime untouched).
- DEVIATION AV-1 honored: tlut=0, no CLUT chunk this rung
  (`init_and_load_tlut` unported) => PINNED stand-in palette: indices 0..15
  reproduce AQ's 16-distinct diagnostic colors cell-for-cell
  (gx_dl_driver.c:79-82 lineage), 16..255 gray ramp; golden-checked 256/256
  (L7-C1) and verified against the AN FIXTURE index stream: row0 texels
  {0,7,14,5} (gen_gvr_fixture formula / ladder5 E4b bytes) expand to
  pal[i] — the "vs AN fixture palette" clause resolved as expand-vs-fixture-
  indices through the pinned table.
- R2/§2.5 (uint2): the new 32-texel fetch is a runtime-formatted
  `tex.read(uint2(cx,cy))` body; scalar-coordinate read NEVER introduced.
  Shipped path unchanged: TEXN unset == today's byte-identical pipeline;
  `run_dl.sh` re-run post-edit GREEN frames=30 vtxdesc=30 dl_calls=15
  tris=60 samples_ok=210 verify_failures=0 (the pre-state gate verbatim).

## V4 — LOGO VISIBLE (rung-4 frame attempt: LANDED — fixture-path chain executing + supply byte-verified; every readback gate below is a SELF-CONSISTENCY proof, not decode ground truth — issue #8)

Landed: `host/gx/dl/{gen_ladder6_logo.py, ladder6_logo_prelude.h,
ladder6_gx_prelude.h, gx_logo_driver.cpp}` + `gx_dl.mk logo` target +
`run_window.sh` phase-2 + `gx_window.mk` link fix (pre-existing break:
`bin/gx_window` died `Undefined symbols: _OSCreateHeap/_OSInitAlloc/
_OSSetCurrentHeap` since Q's osinit split — W's mk predated it, the dl lane
carries the same `osalloc_era.o` patch; found because run_window.sh is the
V4 acceptance runner, fixed in-lane per that precedent, ZERO lane-code
otherwise touched).

Chain actually EXECUTING (not injected) in `build/gxdl/bin/gx_logo`:
supply (fixture->arena) -> `load_sin_board_image` era chain (V2, re-runs
in-process, L7-W1) -> `request_main_phase(0xA)` (SystemBootInit verbatim
slice, L7-R1 next==10/result==1) -> `TLogo::run_frame` (TLogo.cpp:94
verbatim slice) -> **`begin_gx_frame` (gjSystem.c:41 verbatim, C11) — the
0x8013F5F4 wall crossed for the first time**, taking its non-interlaced
leg and setting the host viewport (0,0,640,480,0,1) (LOGO-1; L7-R2), with
GXInvalidateTexAll counted -> 14-leg era call order asserted ==
TLogo.cpp:95-114 (LOGO-2 seams counted; poll==0/is_ready==0 invariants
held — L7-R3) -> `render_tick` seam (LOGO-3) presents the CLUT-expanded
sinbd surface as the era textured-quad recipe through the AQ FIFO-lite
replay.

Acceptance (all via `sh host/gx/run_window.sh`, exit 0):
- L7-P1: 32x32 texel cell centers read back == CLUT-expanded surface
  texels, EXACT (±0) — vs the pipeline's own CLUT-expanded surface (self-consistency gate; issue #8).
- L7-P2 region-nonzero: 76800/76800 pixels of the 320x240 logo rect
  non-black.
- L7-P3 hash: logo texel region (32x32 RGBA words, drawable readback)
  **fnv1a64 = 0x88A8EF94CB09FCD0**; reference = 0x88A8EF94CB09FCD0
  computed AT RUNTIME from the era-chain surface pixels through
  `gxh_pal8_clut_expand` (self-consistency anchor — stated, per brief;
  the only literal pins are the supply-side AU/AN fixture hashes,
  0x93B9BE4E18606D07 image / 0xA92D59BE5839FCE5 texels, which are
  fixture content, not waived counts). Deterministic across two runs +
  the run_window.sh phase.

## Battery state after the wave

- `make -C host/bringup check` -> no `check` RULE ever existed in bringup.mk
  (.PHONY name without recipe); the suite runner is `host/bringup/run.sh` —
  run verbatim: BRINGUP GREEN, 128/128 PASS (prior 110 + ladder6 18), twice
  (intake + post-V4).
- `run_window.sh` GREEN both phases (W clear gate + LOGO VISIBLE).
- `run_dl.sh` GREEN 210/210 samples (post-shader-patch, default path).
- `run_vi.sh` GREEN (VI-BRANCH both legs).
- Waiver header sha `c6b514035f39b633…` recorded pre-wave, re-hashed after
  EVERY make/run above: unchanged. No `make parse`, no gen_waivers
  invocation (dry-run proved zero regen triggers before first gx make).
- Decomp tree read-only @6b38b5383 (verified `git rev-parse` at intake;
  zero writes; generator anchors fail-loud on drift).

## Where V5-V7 stand (AV §5 numbering vs this brief's V1-V4)

- V5 (contract: on-logo §3#1-#3 adapters + submit §4 request): MOSTLY landed
  as this brief's V1 — the three files' logo-relevant legs resolved AND
  executing; what remains V5-shaped: slice `LoadPvmFileTls`
  (psoLoadTexture.cpp:97-101/142-153) + `GinjaMaterial.cpp:367/388/421`
  literal bodies (patterns proven at set_texture_num; deps: ginja_*
  draw-state globals), and the integrator applying `build/njs_allowlist.txt`
  + PARSE_RECIPE edit (AV §4 paste-ready block = the submission).
- V6 (deferred adapters §3#4-#6 BmlResource/TCMCharacter/TChCrLoadReq):
  untouched, unchanged plan, NOT on any critical path (logo visible without
  them — proven).
- V7 (3rd shadow FloorPopulation.h): untouched — compat/njs owned by the
  loader lane; AR §4.3 row stays the named gap.

## Honest blockers / open items

1. Era CLUT supply: `init_and_load_tlut` (GinjaTextureHeader.cpp:56-60)
   unported + no tlut chunk in the fixture -> V3 rides AV-1 stand-in
   palette; era-faithful R8+in-shader-LUT route (§2.2 bullet 4) waits for a
   CLUT-carrying GVR and the tlut leg.
2. run_frame rows 3-9 + render_tick bodies stay counted seams (LOGO-2/3):
   TMainTask::render overlaying, async/sound/controller legs are class
   (c)/(d) homes; the actor-driven real sinbd QUAD (TObjSinBoard::render
   GP list + camera) needs rung-4 proper (AP §4 target TU parse).
3. psoLoadTexture.cpp:142-153 + GinjaMaterial.cpp literal sites: adapters
   enumerated, files not yet sliced (deps above); build/Makefile NJS
   allow-list awaits integrator.
4. gx_dl capture path never clears between the W-literal regime and the
   logo frame alpha: quad color0 white passthrough keeps readback exact,
   but a real TEV alpha chain (GX_MODULATE with vertex alpha) is untested
   with the 32-texel variant (single-frame gate used here).
5. `make -C host/bringup check` as named in the brief has no rule
   (pre-existing .PHONY-only); run.sh is the gate (documented above).
