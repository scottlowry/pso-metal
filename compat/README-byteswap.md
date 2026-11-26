# compat/pso_byteswap.h — usage notes (P1-M)

Single seam for the port's two endian regimes (docs/p1-format-net-disc.md §0/§5,
docs/p1-format-saves.md §5.3). Build with `-Icompat`; C11; all buffer accessors
are memcpy-based, so unaligned images are trap-free and alias-clean.

Namespaces:
- `psoProt16/32/64` — game protocol. Wire is **LE**; on arm64 these are
  identity, but every existing in-tree `bswap()` call site must route here so
  the boundary stays auditable and a BE host works unchanged.
- `psoBe16/32/64` + `psoBeReadNN/psoBeWriteNN(buf)` — TCP/IP frame headers and
  disc images. Wire/platter is **BE**; on arm64 these swap.
- `psoBeSwapAllNN/psoProtSwapAllNN(buf, count)` — in-place whole-struct walkers
  (count = elements, not bytes).
- `psoCipherSwap(buf, nwords32)` — swap-around-cipher (below).

## The three disc DMA chokepoints (swap-on-load, ONCE each)

Everything downstream of these three calls sees native host-order working
values and must contain **zero** per-field swaps (net-disc §5 rule 3).

1. **`open_gsl_archive()` TOC pass** (`src/pso/system/TGStream.cpp:244` reads
   `GslArchiveEntry` whole via ADXF_ReadNw; fields are *never* swapped today —
   the #1 silent-flip site). Ported host loader: right after the read, walk the
   256-entry table once and fix the three s32s per entry:
   ```c
   /* bytes at entry+0x20/0x24/0x28 are BE on the platter */
   int32_t off = (int32_t)psoBeRead32(e + 0x20);   /* archive_offset */
   int32_t sz  = (int32_t)psoBeRead32(e + 0x24);   /* file_size     */
   int32_t dec = (int32_t)psoBeRead32(e + 0x28);   /* decoded_size  */
   psoBeWrite32(e + 0x20, (uint32_t)off); psoBeWrite32(e + 0x24, (uint32_t)sz);
   psoBeWrite32(e + 0x28, (uint32_t)dec);
   ```
   (Or `psoBeSwapAll32` over just those fields; do not bulk-flip the 0x30-byte
   entry — `name[0x20]` is a byte array and must stay byte-true.)

2. **`load_gvm_texture_archive()` / `gvm_stream_read()`** (`text_803DD5E0.cpp:409-436`,
   D2/D3). The DC-legacy code already calls `bswap32_in_place` after the load, so
   PPC's **working** magic value is `0x54525647` ("GVRT" byte-reversed). On arm64 a
   raw LE load of the same platter bytes already yields that working value ⇒ the
   port keeps the *existing* swap site as the identity seam (`psoProtSwapAll32`, or
   delete-in-place-with-comment only at review) and does **not** add `psoBe*`
   there — adding one double-swaps the magic/size. GVR pixel/tlut payload (D4)
   stays byte-true: copy as bytes, never widen.

3. **`NetworkInfoLoader` blob reader** (D5–D10, `NetworkInfoCodec.h`, 7 offset
   asserts pin it). Load the blob byte-true, then one `psoBe*` accessor pass over
   the named multi-byte scalars at their pinned offsets before anything parses
   it; `Text400/Text256` sub-blobs are strings — untouched.

## Save cards: swap around the cipher, never inside it

`SubtractiveRng::apply_rng_cipher` does `data[i] = next() - data[i]` on u32
words; subtraction does **not** commute with byte reversal, so on an LE host:

```c
psoCipherSwap(buf, nwords);        /* BE image -> host-order words   */
for (i ...) w[i] = rng_next() - w[i];   /* cipher exactly as PPC saw it  */
psoCipherSwap(buf, nwords);        /* host words -> BE image again   */
```

Never per-field swaps around individual cipher reads, and never cipher raw LE
loads of the BE image — it is self-inverse against itself, so it only corrupts
*GC-compatible* output (writes a garbage image that host-reloads fine). The
gate test for the bug is a byte-compare against a reference GC dump
(p1-format-saves §5.3 P1 test); the sequencing itself is covered by
`tests/run_byteswap.sh` T4.

## Gate

`tests/run_byteswap.sh` compiles `tests/pso_byteswap_test.c` with
`clang -std=c11 -Wall -Wextra -Werror` at -O0/-O2 plus a UBSan pass and runs
all three: roundtrips, known BE/LE patterns (incl. the GVRT working-value
trap), unaligned reads, cipher sequencing. Exit 0 = pass.
