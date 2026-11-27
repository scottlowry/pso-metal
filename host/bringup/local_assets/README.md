# local_assets (gitignored contents)

Genuine disc-derived fixtures live here, extracted READ-ONLY from the local
disc copy (FST walk under the decomp tree; disc path never recorded in any
receipt, artifact bytes never committed). This repo carries only this note,
the .gitignore entry, and the loader (host/gx/dl/gx_logo_driver.cpp,
GX_LOGO_GVR env hook): receipts = size + fnv1a64 + header dims, printed at
runtime.
