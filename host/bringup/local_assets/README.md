# local_assets (gitignored contents)

Genuine disc-derived fixtures live here, extracted READ-ONLY from the local
disc copy (FST walk under the decomp tree; disc path never recorded in any
receipt, artifact bytes never committed). This repo carries only this note,
the .gitignore entry, and the loader (host/gx/dl/gx_logo_driver.cpp,
GX_LOGO_GVR env hook): receipts = size + fnv1a64 + header dims, printed at
runtime.

`logo_e.gvm` — genuine decompressed logo_e GVMH archive (492,848 B,
fnv1a64 0x1C29B433471C69A8, 24 I4 GVRT frames 256x256/128x128; PRS stream
decoded era-style from disc index record, sinbd-identity calibrated). Receipts +
per-frame offsets/fnv: docs/p2-logo-e-supply-1.md (lane I4P).

P2-FRMB correction (docs/p2-oracle-1.md §5, owner-verified): the fmt-0xE
frames are **GX_TF_CMPR (DXT1-in-RGB5A3, TI 8x8)**, not I4 — the manifest
"I4" label is superseded (sizes agree only because both are 4bpp).
`logo_e.slices.json` (gitignored, lane FRMB) is the runtime-reference
per-slice fnv sidecar for the `GX_FRAMES_SLICES` gate hook; expected hashes
are NEVER compiled into host code. Consumed by `build/gxdl/bin/gx_logo`
under GX_FRAMES_GVM + GX_FRAMES_SLICES — see docs/p2-frames-1.md.
