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
