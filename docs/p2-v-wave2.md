# P2-BB — V5 wave receipts (lane BB: authentic logo art)

Lane BB, 2026-11-27. Contract: `docs/p2-gx-tex-consumer.md` §1–2; prior
receipts `docs/p2-v-wave.md` (V1–V4). Card: `fleet/cards-20261127/BB.md`.
R1 honored (zero waived-count constants; sha pin re-hashed after EVERY make:
all `c6b514035f39b633…`, zero regen, no `make parse`). R2 honored: no scalar
`tex.read` anywhere in the new code (expansion is CPU-side; only the shipped
`GX_DL_TEXN` uint2 variant reads the texture).

## S1 — era CLUT supply port (commit `0850d7c`)

- `gen_ladder6_logo.py` gains 3 anchored VERBATIM slices from
  `src/pso/render/GinjaTextureHeader.cpp` (decomp @6b38b5383, read-only):
  `swap_u32_bytes` :17, `load_gvpl_palette` :35, `init_and_load_tlut` :56 —
  emitted into `build/gxdl/gen/ladder6_logo_glue.cpp`, fail-loud must_contain
  guards (era magic gate 0x4C505647, `move_data` leg, `GXInitTlutObj`+
  `GXLoadTlut` call sites).
- The GVRT tlut-chunk leg: BB-2 walk (`find_gvpl_chunk`, driver port code,
  era chunk-scan regime: magic compared AFTER `swap_u32_bytes`, GBIX-family
  len fields psoBe working values like the AN fixture) hands the chunk to the
  era chain; `TLanguage::load_palette_tlut` (TLanguageFontPalette.cpp:35-37)
  is the caller pattern: `load_gvpl_palette` -> `init_and_load_tlut(0,
  palette->format, palette->entry_count, palette->data)`.
- Host seams: `GXInitTlutObj`/`GXLoadTlut` record `{lut,fmt,count}` into
  `g_host_tlut_banks[16]` (era `r->tlutObj = *t`, GXTexture.c:445; the 64-bit
  host ptr parks in a pending slot — era u32[3] record can't carry it, named
  seam). `gxh_pal8_clut_expand_lut` decodes bank entries per GXTlutFmt
  (IA8/RGB565/RGB5A3 era-quantized + format 3 RGBA8 carrier, DEVIATION BB-1:
  the CPU-expand route needs lossless 8-bit channels; era HW set has none
  that can carry AV-1's 256 gray values).
- AV-1 stand-in RETIRED from the default path, KEPT behind `-DGX_LOGO_AV1`
  (`logo-av1` target; run_window.sh runs BOTH regimes).
- GOLDEN FIDELITY (printed): era chain fed the synthetic CLUT -> palette
  fnv1a64 `0xE7F4FBE2487670E5` == AV-1 stand-in fnv1a64 `0xE7F4FBE2487670E5`,
  256/256 byte-for-byte (`memcmp`==0), `palette->data` fnv == chunk entry
  bytes (byte-true move_data). L7-T1 era leg executed (init+load counts 1/1).

## S2 — genuine sinbd (commit `27b6209`, loader landed; extraction WIP)

- `host/bringup/local_assets/` gitignored (contents never committed);
  tracked README + `.gitignore` entry; driver gains `GX_LOGO_GVR` loader:
  reads a local blob, prints size + fnv1a64 receipt (disc path never
  printed), feeds `load_sin_board_image` at that size; the era CLUT supply
  prefers a GVPL chunk found IN the genuine blob, falls back to synthetic.
- L7-W1/L7-C1E/L7-C2E now read dims/format from the era header (fixture pins
  retained when the fixture path runs).
- Honest state: PRS/GVRT scan of loose quest/cam/npc .rel candidates found
  NO inline GVRT stream this window — genuine sinbd bytes sit behind the
  resource-archive index (`.pr2/.pr3`/SF layer). NEXT ACTION: walk the
  archive index (TLanguage/resource_decompress naming, AE's GSL chain),
  extract to `host/bringup/local_assets/sinbd.gvr` (gitignored), re-run
  `GX_LOGO_GVR=host/bringup/local_assets/sinbd.gvr build/gxdl/bin/gx_logo`,
  record fnv+dims from that run.

## S3 — V4 gate rerun under era supply (era regime = default)

`sh host/gx/run_window.sh` exit 0, both phases + both regimes:
- L7-P1 cell-center readback EXACT vs CLUT-expanded surface — self-consistency gate vs own expanded surface (32x32 at the
  era-header dims, ±0) — era regime AND av1 regime.
- L7-P2 region-nonzero 76800/76800 of the 320x240 logo rect.
- L7-P3 readback fnv1a64 `0x88A8EF94CB09FCD0` == runtime reference
  `0x88A8EF94CB09FCD0` (computed from era-chain surface pixels through the
  loaded-CLUT expand; no literal pinned for genuine art).

## S4 — suites green under existing shapes

| suite | result |
|---|---|
| `host/bringup/run.sh` | BRINGUP GREEN — 128/128 PASS (13+16+26+27+28+18 counted from run output) |
| `host/gx/dl/run_dl.sh` | GREEN frames=30 dl_calls=15 tris=60 samples_ok=210 verify_failures=0; `--debug` TEX16 diagnostic 16-cell map exit 0 |
| `host/gx/run_window.sh` | GREEN both phases, era regime 12/12 + AV-1 regime 10/10 |
| `host/gx/run_vi.sh` | PASS (VI-BRANCH both legs) |
| waiver header | `c6b514035f39b633…` verified pre/post EVERY make this lane |

## Deviations introduced (this lane)

- BB-1: TLUT carrier format 3 (RGBA8 byte-true) for the host CPU-expand
  route; era GXTlutFmt 0/1/2 decode implemented and used when data carries
  them (genuine GVPL chunks decode era-quantized).
- BB-2: `find_gvpl_chunk` is port walk code (era homes skip GVPL inside the
  GVRT scan); its OUTPUT flows exclusively through the era bodies.
- GXInitTlutObj/GXLoadTlut host seams park the lut pointer in a single
  pending slot (era register-encoding can't hold a 64-bit host pointer).
