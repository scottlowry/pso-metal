# P2-I4 — genuine logo_e + GVRA-walk SURVEY (lane I4P, wave-3)

Lane I4P, 2026-11-27. Card `t_8d9c436c`; contract `fleet/cards-20261127/I4P.md`.
READ-ONLY survey: disc under decomp `orig/` read-only (path never printed),
decomp @6b38b5383 read-only, ZERO builds, ZERO host edits. Waiver era pin
`d4382fcd0659357f` (apply-7) — untouched (no build run). Deliverable: manifest
+ builder brief; supply for the next genuine-frames lane.

## S0 — index regime (BB2 rules re-established, same-turn)

Stride-12 `{u32 name-pool-off, u32 data-off, u32 size}` records, table base
`0x1e008` (residue calibration over `0x1e000..0x1e00C`, 21,390 records,
4,357 named — exact BB2 reproduction). Name pool base `0x5CAB0`; `name-off`
columns below are pool-relative offsets (absolute = base + value). Loose-file
anchor gate: **8/8** `.rel` records re-read byte-identical (BB2 regime).

**Stream-format correction (important, proven):** the on-disc `.prs` records
carry the era PRS control-bit stream from **byte 0** — there is NO `0x0D`
type byte on this disc family (first byte `0x7F`/`0x3F` = first control byte;
Logo/Title/hyouji all decode cleanly). Decoder = verbatim port of
`PrsDecoder::prs_decompress` (`src/pso/system/prs.cpp`, the 16/16 stage-19
golden unit). Calibration: `f256_hyouji.prs` → first GVRT chunk slice is
**byte-identical** to landed `local_assets/sinbd.gvr` (fnv1a64
`0x580CC3F5451A3EDC`, 0x10010 B). Same-turn also: `f256_hyouji_a.gvp` raw
record fnv `0x7818A36ACF96C1EB` == landed `hyouji_a.gvp`.

Era walk byte order (verified against `text_803DD5E0.cpp` loaders):
magics are ASCII on disk; `GVMH.hdr.size` and every chunk `size` field are
**little-endian** (era reads BE then `bswap32`); `flags`/`texture_count` and
the `GVRT desc` (fmt u32 @+8, BE w @+C, BE h @+E) are **big-endian**; the
chunk stream starts at `hdr.size + 8` and each chunk occupies `size + 8`
bytes. `GVPL` file: size@+4 LE, format u16 BE @+8 (=2 C16), entry_count u16
BE @+E (=256), payload 512 B from +0x10. Directory entries here all use
flags `0x000F` → stride `0x26`: attr u16, 0x1C-byte name, fmt u16, dims u16,
global (GBIX) index u32. Attrs are sequential `0x0000..N-1` = stream order.

## S1 — MANIFEST: top-level records (fnv1a64 of raw record bytes; no bytes committed)

| record | name-off (pool) | offset | size | fmt | fnv1a64 (raw record) |
|---|---|---|---|---|---|
| Logo.prs | 0x5423 | 0x29e97a48 | 0x5e4b | PRS | `62574C186D8AEB0D` |
| Logo_e.prs | 0x542c | 0x29e9d894 | 0x7227 | PRS | `38F68B92898B8F60` |
| sega_logo.prs | 0xe9a8 | 0x3de687f4 | 0x1931 | PRS | `E734318862BFF335` |
| sega_logo_e.prs | 0xe9b6 | 0x3de6a128 | 0x195b | PRS | `666C9B656E61ACA5` |
| st_logo.prs | 0xee26 | 0x41fa3c40 | 0x192f | PRS | `801C7D4190189917` |
| nintendo_logo.GVM | 0x68f5 | 0x2b71a880 | 0x8060 | raw GVMH | `A82AE70B25197179` |
| f256_hyouji.prs | 0x3ec7 | 0x1f79da2c | 0x25186 | PRS | `2BC8657B50FCCF5C` |
| f256_hyoujismall.prs | 0x3ed7 | 0x1f7c2bb4 | 0x8f7d | PRS | `056E6D40F0E6951C` |
| f256_hyouji_a.gvp | 0x3eec | 0x1f7cbb34 | 0x210 | raw GVPL | `7818A36ACF96C1EB` |
| f256_hyouji_b.GVP | 0x3efe | 0x1f7cbd44 | 0x210 | raw GVPL | `FF42BF6219125798` |
| f256_hyouji_c.GVP | 0x3f10 | 0x1f7cbf54 | 0x210 | raw GVPL | `32A22F93AE58E6B9` |
| Title.prs | 0xf03a | 0x448b7d04 | 0x48104 | PRS | `30CF766998CA9B71` |
| Title_eng.prs | 0xf05e | 0x4493fec8 | 0x47eba | PRS | `F2BAFD5ABFA9500A` |
| Title_eng_trial.prs | 0xf06c | 0x44987d84 | 0x46d68 | PRS | `002372C3247589D8` |


## S2 — decompressed GVMH archives (PRS leg, scratch; per-record fnv = full GVRT chunk incl. 0x10 header, the `sinbd.gvr` regime)

Every archive consumed EXACTLY (walk end == decompressed length; GVRT count ==
directory count; no skip-chunks present in any logo/sinbd/title archive —
`seek_gvrt_chunk`'s COMM/CONV/MDLN/PVMI/GVPL/IMGC legs stay untaken).

| archive | comp fnv1a64 | decompressed size | decompressed fnv1a64 | contents |
|---|---|---|---|---|
| Logo.prs | `62574C186D8AEB0D` | 0x78530 | `EFA2593F226EB17C` | 24 GVRT I4 256/128 (JP set) |
| Logo_e.prs | `38F68B92898B8F60` | 0x78530 | `1C29B433471C69A8` | 24 GVRT I4 256/128 (EN set: gc_adx01..08, sega_logo_e01..08, st_logo01..08) |
| sega_logo.prs | `E734318862BFF335` | 0x281d0 | `17E215BAC66910AE` | 8 GVRT I4 (sega_logo01..08) |
| sega_logo_e.prs | `666C9B656E61ACA5` | 0x281d0 | `988BFFD8B969FD9F` | 8 GVRT I4 (EN sega block) |
| st_logo.prs | `801C7D4190189917` | 0x281d0 | `3BC14A223E7DB58F` | 8 GVRT I4 (st_logo01..08) |
| nintendo_logo.GVM | `A82AE70B25197179` | (none) | `(same)` | 1 GVRT, uncompressed GVM |
| f256_hyouji.prs | `2BC8657B50FCCF5C` | 0xa42e0 | `0F3C018D267FF615` | 13 GVRT: 9 PAL8 + 4 C4 (sinbd HUD family, BB2 supply host) |
| f256_hyoujismall.prs | `056E6D40F0E6951C` | 0x300c0 | `F5E11BF6FBCF920F` | 3 GVRT PAL8 (hyouji_a/b/c — frame-identical to hyouji 0..2) |
| f256_hyouji_a.gvp | `7818A36ACF96C1EB` | (none) | `(same)` | C16 TLUT 256 entries; == local_assets/hyouji_a.gvp |
| f256_hyouji_b.GVP | `FF42BF6219125798` | (none) | `(same)` | C16 TLUT 256 (hud bank 0x41) |
| f256_hyouji_c.GVP | `32A22F93AE58E6B9` | (none) | `(same)` | C16 TLUT 256 (bank 0x42) |
| Title.prs | `30CF766998CA9B71` | 0xf0480 | `45EA0D84665F1CA5` | 21 GVRT (JP title archive the title path walks) |
| Title_eng.prs | `F2BAFD5ABFA9500A` | 0xf0480 | `E95FA89B756C5D79` | 21 GVRT (EN title archive, current_language!=ja) |
| Title_eng_trial.prs | `002372C3247589D8` | 0xf0480 | `BE33530324BC34EA` | 21 GVRT (trial build variant) |


### Logo_e.prs — 24 I4 frames (fmt word 0x0000000E = 4bpp; 0x8010 B chunks @256x256, 0x2010 @128x128)

| record | name | in-archive offset | size | fmt word | dims | fnv1a64 (GVRT chunk) |
|---|---|---|---|---|---|---|
| GVRT[00] | gc_adx01 | 0x3b0 | 0x8010 | fmt=0x0000000E | 256x256 | 004075B4779DA374 |
| GVRT[01] | gc_adx02 | 0x83c0 | 0x8010 | fmt=0x0000000E | 256x256 | 9145B1657183FE4A |
| GVRT[02] | gc_adx03 | 0x103d0 | 0x8010 | fmt=0x0000000E | 256x256 | EC7350D7705E3CB5 |
| GVRT[03] | gc_adx04 | 0x183e0 | 0x8010 | fmt=0x0000000E | 256x256 | CA08712AA39BB83A |
| GVRT[04] | gc_adx05 | 0x203f0 | 0x2010 | fmt=0x0000000E | 128x128 | CA976BB52B992D8C |
| GVRT[05] | gc_adx06 | 0x22400 | 0x2010 | fmt=0x0000000E | 128x128 | 6C76EBAFA97CE58D |
| GVRT[06] | gc_adx07 | 0x24410 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[07] | gc_adx08 | 0x26420 | 0x2010 | fmt=0x0000000E | 128x128 | C8D2B06B7CF1BB0C |
| GVRT[08] | sega_logo_e01 | 0x28430 | 0x8010 | fmt=0x0000000E | 256x256 | A98C4BF3926BC315 |
| GVRT[09] | sega_logo_e02 | 0x30440 | 0x8010 | fmt=0x0000000E | 256x256 | E604EFC97A26324A |
| GVRT[10] | sega_logo_e03 | 0x38450 | 0x8010 | fmt=0x0000000E | 256x256 | 21CB09DE9F06F010 |
| GVRT[11] | sega_logo_e04 | 0x40460 | 0x8010 | fmt=0x0000000E | 256x256 | 2AF27918A5EFA17A |
| GVRT[12] | sega_logo_e05 | 0x48470 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[13] | sega_logo_e06 | 0x4a480 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[14] | sega_logo_e07 | 0x4c490 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[15] | sega_logo_e08 | 0x4e4a0 | 0x2010 | fmt=0x0000000E | 128x128 | C8D2B06B7CF1BB0C |
| GVRT[16] | st_logo01 | 0x504b0 | 0x8010 | fmt=0x0000000E | 256x256 | 6B68A20C27DFB45A |
| GVRT[17] | st_logo02 | 0x584c0 | 0x8010 | fmt=0x0000000E | 256x256 | 59DFB39C8A13C82E |
| GVRT[18] | st_logo03 | 0x604d0 | 0x8010 | fmt=0x0000000E | 256x256 | 8D837BB76E5B10F0 |
| GVRT[19] | st_logo04 | 0x684e0 | 0x8010 | fmt=0x0000000E | 256x256 | 766830662629C474 |
| GVRT[20] | st_logo05 | 0x704f0 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[21] | st_logo06 | 0x72500 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[22] | st_logo07 | 0x74510 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[23] | st_logo08 | 0x76520 | 0x2010 | fmt=0x0000000E | 128x128 | C8D2B06B7CF1BB0C |


### Logo.prs — 24 I4 frames (JP twin; gc_adx/sega_logo/st_logo JP names)

| record | name | in-archive offset | size | fmt word | dims | fnv1a64 (GVRT chunk) |
|---|---|---|---|---|---|---|
| GVRT[00] | gc_adx01 | 0x3b0 | 0x8010 | fmt=0x0000000E | 256x256 | 37B3961058682AA2 |
| GVRT[01] | gc_adx02 | 0x83c0 | 0x8010 | fmt=0x0000000E | 256x256 | C417DF098C021754 |
| GVRT[02] | gc_adx03 | 0x103d0 | 0x8010 | fmt=0x0000000E | 256x256 | 3E8B8340D8DB82D1 |
| GVRT[03] | gc_adx04 | 0x183e0 | 0x8010 | fmt=0x0000000E | 256x256 | E9C212D9EAA752C7 |
| GVRT[04] | gc_adx05 | 0x203f0 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[05] | gc_adx06 | 0x22400 | 0x2010 | fmt=0x0000000E | 128x128 | 71819F81D87C7073 |
| GVRT[06] | gc_adx07 | 0x24410 | 0x2010 | fmt=0x0000000E | 128x128 | D3FE6D902BE1F27F |
| GVRT[07] | gc_adx08 | 0x26420 | 0x2010 | fmt=0x0000000E | 128x128 | 69E3AB8046ED9C04 |
| GVRT[08] | sega_logo01 | 0x28430 | 0x8010 | fmt=0x0000000E | 256x256 | 1BC89E51A383BC35 |
| GVRT[09] | sega_logo02 | 0x30440 | 0x8010 | fmt=0x0000000E | 256x256 | 3986CC324F90360E |
| GVRT[10] | sega_logo03 | 0x38450 | 0x8010 | fmt=0x0000000E | 256x256 | D4588B20E1CDBB84 |
| GVRT[11] | sega_logo04 | 0x40460 | 0x8010 | fmt=0x0000000E | 256x256 | 4DB9745E12A18982 |
| GVRT[12] | sega_logo05 | 0x48470 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[13] | sega_logo06 | 0x4a480 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[14] | sega_logo07 | 0x4c490 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[15] | sega_logo08 | 0x4e4a0 | 0x2010 | fmt=0x0000000E | 128x128 | C8D2B06B7CF1BB0C |
| GVRT[16] | st_logo01 | 0x504b0 | 0x8010 | fmt=0x0000000E | 256x256 | 6B68A20C27DFB45A |
| GVRT[17] | st_logo02 | 0x584c0 | 0x8010 | fmt=0x0000000E | 256x256 | 59DFB39C8A13C82E |
| GVRT[18] | st_logo03 | 0x604d0 | 0x8010 | fmt=0x0000000E | 256x256 | 8D837BB76E5B10F0 |
| GVRT[19] | st_logo04 | 0x684e0 | 0x8010 | fmt=0x0000000E | 256x256 | 766830662629C474 |
| GVRT[20] | st_logo05 | 0x704f0 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[21] | st_logo06 | 0x72500 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[22] | st_logo07 | 0x74510 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[23] | st_logo08 | 0x76520 | 0x2010 | fmt=0x0000000E | 128x128 | C8D2B06B7CF1BB0C |


### sega_logo.prs / sega_logo_e.prs / st_logo.prs — 8-frame blocks (logo_e's per-brand subsets stand separately too)

| record | name | in-archive offset | size | fmt word | dims | fnv1a64 (GVRT chunk) |
|---|---|---|---|---|---|---|
| GVRT[00] | sega_logo01 | 0x150 | 0x8010 | fmt=0x0000000E | 256x256 | 1BC89E51A383BC35 |
| GVRT[01] | sega_logo02 | 0x8160 | 0x8010 | fmt=0x0000000E | 256x256 | 3986CC324F90360E |
| GVRT[02] | sega_logo03 | 0x10170 | 0x8010 | fmt=0x0000000E | 256x256 | D4588B20E1CDBB84 |
| GVRT[03] | sega_logo04 | 0x18180 | 0x8010 | fmt=0x0000000E | 256x256 | 4DB9745E12A18982 |
| GVRT[04] | sega_logo05 | 0x20190 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[05] | sega_logo06 | 0x221a0 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[06] | sega_logo07 | 0x241b0 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[07] | sega_logo08 | 0x261c0 | 0x2010 | fmt=0x0000000E | 128x128 | C8D2B06B7CF1BB0C |

| record | name | in-archive offset | size | fmt word | dims | fnv1a64 (GVRT chunk) |
|---|---|---|---|---|---|---|
| GVRT[00] | sega_logo_e01 | 0x150 | 0x8010 | fmt=0x0000000E | 256x256 | A98C4BF3926BC315 |
| GVRT[01] | sega_logo_e02 | 0x8160 | 0x8010 | fmt=0x0000000E | 256x256 | E604EFC97A26324A |
| GVRT[02] | sega_logo_e03 | 0x10170 | 0x8010 | fmt=0x0000000E | 256x256 | D870B71BEB7C8D09 |
| GVRT[03] | sega_logo_e04 | 0x18180 | 0x8010 | fmt=0x0000000E | 256x256 | 2AF27918A5EFA17A |
| GVRT[04] | sega_logo_e05 | 0x20190 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[05] | sega_logo_e06 | 0x221a0 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[06] | sega_logo_e07 | 0x241b0 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[07] | sega_logo_e08 | 0x261c0 | 0x2010 | fmt=0x0000000E | 128x128 | C8D2B06B7CF1BB0C |

| record | name | in-archive offset | size | fmt word | dims | fnv1a64 (GVRT chunk) |
|---|---|---|---|---|---|---|
| GVRT[00] | st_logo01 | 0x150 | 0x8010 | fmt=0x0000000E | 256x256 | 6B68A20C27DFB45A |
| GVRT[01] | st_logo02 | 0x8160 | 0x8010 | fmt=0x0000000E | 256x256 | 59DFB39C8A13C82E |
| GVRT[02] | st_logo03 | 0x10170 | 0x8010 | fmt=0x0000000E | 256x256 | 8D837BB76E5B10F0 |
| GVRT[03] | st_logo04 | 0x18180 | 0x8010 | fmt=0x0000000E | 256x256 | 766830662629C474 |
| GVRT[04] | st_logo05 | 0x20190 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[05] | st_logo06 | 0x221a0 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[06] | st_logo07 | 0x241b0 | 0x2010 | fmt=0x0000000E | 128x128 | 5B73ADEAFF301D0C |
| GVRT[07] | st_logo08 | 0x261c0 | 0x2010 | fmt=0x0000000E | 128x128 | C8D2B06B7CF1BB0C |


### nintendo_logo.GVM — 1 GVRT, stored UNCOMPRESSED (no PRS leg)

| record | name | in-archive offset | size | fmt word | dims | fnv1a64 (GVRT chunk) |
|---|---|---|---|---|---|---|
| GVRT[00] | f256_license | 0x50 | 0x8010 | fmt=0x0000000E | 256x256 | E57EA677B8F61B92 |


### f256_hyouji.prs — 13 GVRT: 9 PAL8 (fmt 0x00000A09) + 4 C4 (0x00000A08); GVRT[00] == landed sinbd.gvr byte-true

| record | name | in-archive offset | size | fmt word | dims | fnv1a64 (GVRT chunk) |
|---|---|---|---|---|---|---|
| GVRT[00] | f256_hyouji_a | 0x210 | 0x10010 | fmt=0x00000A09 | 256x256 | 580CC3F5451A3EDC |
| GVRT[01] | f256_hyouji_b | 0x10220 | 0x10010 | fmt=0x00000A09 | 256x256 | BE6AC171A9B2F54B |
| GVRT[02] | f256_hyouji_c | 0x20230 | 0x10010 | fmt=0x00000A09 | 256x256 | 4E1DC5665F6EEB8B |
| GVRT[03] | f256_actionpallet_a | 0x30240 | 0x10010 | fmt=0x00000A09 | 256x256 | D5D1E3153FC2BF6D |
| GVRT[04] | f256_actionpallet_b | 0x40250 | 0x10010 | fmt=0x00000A09 | 256x256 | A765DEB3DDEE62CE |
| GVRT[05] | f256_actionpallet_c | 0x50260 | 0x10010 | fmt=0x00000A09 | 256x256 | D5D63F0EF8580A59 |
| GVRT[06] | f256_duelcount | 0x60270 | 0x10010 | fmt=0x00000A09 | 256x256 | 371371B9DC63C4F0 |
| GVRT[07] | r256_symobj01 | 0x70280 | 0x8010 | fmt=0x00000A08 | 256x256 | 8ABFE353F7446A36 |
| GVRT[08] | r256_symobj02 | 0x78290 | 0x8010 | fmt=0x00000A08 | 256x256 | 6A4F446377802C31 |
| GVRT[09] | r256_symobj03 | 0x802a0 | 0x8010 | fmt=0x00000A08 | 256x256 | 31CA7E938844B749 |
| GVRT[10] | r256_symparts | 0x882b0 | 0x8010 | fmt=0x00000A08 | 256x256 | 17D38F75B51C7A45 |
| GVRT[11] | b256_mainmenu | 0x902c0 | 0x10010 | fmt=0x00000A09 | 256x256 | D39D215FAED3AEDF |
| GVRT[12] | f256_carsol | 0xa02d0 | 0x4010 | fmt=0x00000A09 | 128x128 | A2FDFFC0110F919C |


### f256_hyoujismall.prs — 3 GVRT PAL8; all three fnv-IDENTICAL to hyouji GVRT[00..02] (same frames, smaller wrapper)

| record | name | in-archive offset | size | fmt word | dims | fnv1a64 (GVRT chunk) |
|---|---|---|---|---|---|---|
| GVRT[00] | f256_hyouji_a | 0x90 | 0x10010 | fmt=0x00000A09 | 256x256 | 580CC3F5451A3EDC |
| GVRT[01] | f256_hyouji_b | 0x100a0 | 0x10010 | fmt=0x00000A09 | 256x256 | BE6AC171A9B2F54B |
| GVRT[02] | f256_hyouji_c | 0x200b0 | 0x10010 | fmt=0x00000A09 | 256x256 | 4E1DC5665F6EEB8B |


### Title.prs — 21 GVRT multi-record archive the title path walks (mixed fmt: 0x00000805 RGB565 16bpp, 0x0000080E I4+attr, 0x0000000E I4)

| record | name | in-archive offset | size | fmt word | dims | fnv1a64 (GVRT chunk) |
|---|---|---|---|---|---|---|
| GVRT[00] | f256_ad | 0x330 | 0x20010 | fmt=0x00000805 | 256x256 | C51FA84BE708B078 |
| GVRT[01] | f256_psotitle | 0x20340 | 0x20010 | fmt=0x00000805 | 256x256 | 90FA84A33578C830 |
| GVRT[02] | f256_psotitle02 | 0x40350 | 0x20010 | fmt=0x00000805 | 256x256 | 35565ED47D96D7FB |
| GVRT[03] | f256_titlealpha01 | 0x60360 | 0x8010 | fmt=0x0000080E | 256x256 | F7829C997ADDCFB8 |
| GVRT[04] | f256_titlealpha02 | 0x68370 | 0x8010 | fmt=0x0000080E | 256x256 | C1F6B0559E651B42 |
| GVRT[05] | f256_titlealpha03 | 0x70380 | 0x8010 | fmt=0x0000080E | 256x256 | 07A1F07454E44718 |
| GVRT[06] | f256_titlealpha04 | 0x78390 | 0x8010 | fmt=0x0000080E | 256x256 | 946C75EF4A1F78A7 |
| GVRT[07] | f256_titlealpha05 | 0x803a0 | 0x2010 | fmt=0x0000080E | 128x128 | 028B71359C6C8061 |
| GVRT[08] | f256_titlealpha06 | 0x823b0 | 0x2010 | fmt=0x0000080E | 128x128 | 2860285A66A47086 |
| GVRT[09] | f256_titlealpha07 | 0x843c0 | 0x2010 | fmt=0x0000080E | 128x128 | C8E8EAF00CFD7FC2 |
| GVRT[10] | f256_titlealpha08 | 0x863d0 | 0x2010 | fmt=0x0000080E | 128x128 | 5101FF6711EFB4EF |
| GVRT[11] | f512_mahoujin | 0x883e0 | 0x20010 | fmt=0x0000080E | 512x512 | 7B260219B786B5B5 |
| GVRT[12] | f512_mahoujin_b | 0xa83f0 | 0x20010 | fmt=0x0000080E | 512x512 | BE1792EFBF59F064 |
| GVRT[13] | s256_1st_title01 | 0xc8400 | 0x8010 | fmt=0x0000000E | 256x256 | B091290BD5F544A9 |
| GVRT[14] | s256_1st_title02 | 0xd0410 | 0x8010 | fmt=0x0000000E | 256x256 | D2B77CBBF254BAA9 |
| GVRT[15] | s256_1st_title03 | 0xd8420 | 0x8010 | fmt=0x0000000E | 256x256 | 3BED3A5D1F2D20CA |
| GVRT[16] | s256_1st_title04 | 0xe0430 | 0x8010 | fmt=0x0000000E | 256x256 | 1229E7CCEA081FB7 |
| GVRT[17] | s256_1st_title05 | 0xe8440 | 0x2010 | fmt=0x0000000E | 128x128 | C9A3D4060B4867F6 |
| GVRT[18] | s256_1st_title06 | 0xea450 | 0x2010 | fmt=0x0000000E | 128x128 | 8EC4F887831A4A9A |
| GVRT[19] | s256_1st_title07 | 0xec460 | 0x2010 | fmt=0x0000000E | 128x128 | 6139F5E5E502D6B9 |
| GVRT[20] | s256_1st_title08 | 0xee470 | 0x2010 | fmt=0x0000000E | 128x128 | FD5FE0B4F9DDAB8C |


### Title_eng.prs / Title_eng_trial.prs — 21 GVRT each (EN/trial variants; identical sizes, distinct fnv)

| record | name | in-archive offset | size | fmt word | dims | fnv1a64 (GVRT chunk) |
|---|---|---|---|---|---|---|
| GVRT[00] | f256_ad_eng | 0x330 | 0x20010 | fmt=0x00000805 | 256x256 | AFD23EF5D3DCD7D6 |
| GVRT[01] | f256_psotitle | 0x20340 | 0x20010 | fmt=0x00000805 | 256x256 | 90FA84A33578C830 |
| GVRT[02] | f256_psotitle02 | 0x40350 | 0x20010 | fmt=0x00000805 | 256x256 | 35565ED47D96D7FB |
| GVRT[03] | f256_titlealpha01 | 0x60360 | 0x8010 | fmt=0x0000080E | 256x256 | F7829C997ADDCFB8 |
| GVRT[04] | f256_titlealpha02 | 0x68370 | 0x8010 | fmt=0x0000080E | 256x256 | C1F6B0559E651B42 |
| GVRT[05] | f256_titlealpha03 | 0x70380 | 0x8010 | fmt=0x0000080E | 256x256 | 07A1F07454E44718 |
| GVRT[06] | f256_titlealpha04 | 0x78390 | 0x8010 | fmt=0x0000080E | 256x256 | 946C75EF4A1F78A7 |
| GVRT[07] | f256_titlealpha05 | 0x803a0 | 0x2010 | fmt=0x0000080E | 128x128 | 028B71359C6C8061 |
| GVRT[08] | f256_titlealpha06 | 0x823b0 | 0x2010 | fmt=0x0000080E | 128x128 | 2860285A66A47086 |
| GVRT[09] | f256_titlealpha07 | 0x843c0 | 0x2010 | fmt=0x0000080E | 128x128 | C8E8EAF00CFD7FC2 |
| GVRT[10] | f256_titlealpha08 | 0x863d0 | 0x2010 | fmt=0x0000080E | 128x128 | 5101FF6711EFB4EF |
| GVRT[11] | f512_mahoujin | 0x883e0 | 0x20010 | fmt=0x0000080E | 512x512 | 7B260219B786B5B5 |
| GVRT[12] | f512_mahoujin_b | 0xa83f0 | 0x20010 | fmt=0x0000080E | 512x512 | BE1792EFBF59F064 |
| GVRT[13] | s256_1st_title01 | 0xc8400 | 0x8010 | fmt=0x0000000E | 256x256 | B091290BD5F544A9 |
| GVRT[14] | s256_1st_title02 | 0xd0410 | 0x8010 | fmt=0x0000000E | 256x256 | D2B77CBBF254BAA9 |
| GVRT[15] | s256_1st_title03 | 0xd8420 | 0x8010 | fmt=0x0000000E | 256x256 | 3BED3A5D1F2D20CA |
| GVRT[16] | s256_1st_title04 | 0xe0430 | 0x8010 | fmt=0x0000000E | 256x256 | 1229E7CCEA081FB7 |
| GVRT[17] | s256_1st_title05 | 0xe8440 | 0x2010 | fmt=0x0000000E | 128x128 | C9A3D4060B4867F6 |
| GVRT[18] | s256_1st_title06 | 0xea450 | 0x2010 | fmt=0x0000000E | 128x128 | 8EC4F887831A4A9A |
| GVRT[19] | s256_1st_title07 | 0xec460 | 0x2010 | fmt=0x0000000E | 128x128 | 6139F5E5E502D6B9 |
| GVRT[20] | s256_1st_title08 | 0xee470 | 0x2010 | fmt=0x0000000E | 128x128 | FD5FE0B4F9DDAB8C |

| record | name | in-archive offset | size | fmt word | dims | fnv1a64 (GVRT chunk) |
|---|---|---|---|---|---|---|
| GVRT[00] | f256_ad_eng | 0x330 | 0x20010 | fmt=0x00000805 | 256x256 | 4750DFC6E184920A |
| GVRT[01] | f256_psotitle | 0x20340 | 0x20010 | fmt=0x00000805 | 256x256 | A1FBBB5308944A52 |
| GVRT[02] | f256_psotitle02 | 0x40350 | 0x20010 | fmt=0x00000805 | 256x256 | DBA97E4F29D4E873 |
| GVRT[03] | f256_titlealpha01 | 0x60360 | 0x8010 | fmt=0x0000080E | 256x256 | BB5D4CD91B164FB1 |
| GVRT[04] | f256_titlealpha02 | 0x68370 | 0x8010 | fmt=0x0000080E | 256x256 | 26C6139109291650 |
| GVRT[05] | f256_titlealpha03 | 0x70380 | 0x8010 | fmt=0x0000080E | 256x256 | 73A354262E6D4704 |
| GVRT[06] | f256_titlealpha04 | 0x78390 | 0x8010 | fmt=0x0000080E | 256x256 | 17BC6DC3BE5F578F |
| GVRT[07] | f256_titlealpha05 | 0x803a0 | 0x2010 | fmt=0x0000080E | 128x128 | 2FB25BCD3F3436CC |
| GVRT[08] | f256_titlealpha06 | 0x823b0 | 0x2010 | fmt=0x0000080E | 128x128 | 29CEF1629CBAB3E5 |
| GVRT[09] | f256_titlealpha07 | 0x843c0 | 0x2010 | fmt=0x0000080E | 128x128 | 92FC450539BBBC72 |
| GVRT[10] | f256_titlealpha08 | 0x863d0 | 0x2010 | fmt=0x0000080E | 128x128 | 1B3C632A46EF427C |
| GVRT[11] | f512_mahoujin | 0x883e0 | 0x20010 | fmt=0x0000080E | 512x512 | 7B260219B786B5B5 |
| GVRT[12] | f512_mahoujin_b | 0xa83f0 | 0x20010 | fmt=0x0000080E | 512x512 | BE1792EFBF59F064 |
| GVRT[13] | s256_1st_title01 | 0xc8400 | 0x8010 | fmt=0x0000000E | 256x256 | 2B55CC7FF888A664 |
| GVRT[14] | s256_1st_title02 | 0xd0410 | 0x8010 | fmt=0x0000000E | 256x256 | 7DC401FAEC003998 |
| GVRT[15] | s256_1st_title03 | 0xd8420 | 0x8010 | fmt=0x0000000E | 256x256 | 705F42E579F8CCB0 |
| GVRT[16] | s256_1st_title04 | 0xe0430 | 0x8010 | fmt=0x0000000E | 256x256 | F0A24E23B88DA9E5 |
| GVRT[17] | s256_1st_title05 | 0xe8440 | 0x2010 | fmt=0x0000000E | 128x128 | 3B02BE1200EC9003 |
| GVRT[18] | s256_1st_title06 | 0xea450 | 0x2010 | fmt=0x0000000E | 128x128 | 32B6039A9C019C61 |
| GVRT[19] | s256_1st_title07 | 0xec460 | 0x2010 | fmt=0x0000000E | 128x128 | DD750C7DC3F497D1 |
| GVRT[20] | s256_1st_title08 | 0xee470 | 0x2010 | fmt=0x0000000E | 128x128 | EB64774E3382C761 |


## S3 — logo_e supply LANDED (gitignored local_assets; bytes never committed)

`host/bringup/local_assets/logo_e.gvm` — **492,848 B (0x78530)**, fnv1a64
`0x1C29B433471C69A8`, magic `GVMH`, decompressed logo_e archive (PRS stream
decoded with the era decoder, byte-0 control bits, calibrated via the sinbd
identity). **Consumed NOW by the next lane**: `register_texture_logo_e`
(TAdSEGAlogo.cpp:103-105) → `load_streamed_prs_texture("logo_e")` is exactly
"decode PRS → `load_gvm_texture_archive(buf, &texture_logo_e_descriptor)`";
this file IS that `buf`. The genuine-frames lane slices per-frame GVRT records
from it at the S2 offsets above with NO disc access; the 9-byte
`fake_disc/logo_e.prs` step-19 stand-in stays untouched.

## S4 — builder brief: next genuine-frames lane (logo_e → title)

**Consumers / routes (era, from decomp @6b38b5383):**
- logo_e leg: `TLogo::init_logo_scene` → `run_load_callback_table` →
  `register_texture_logo_e` → `load_streamed_prs_texture("logo_e",
  &texture_logo_e_descriptor)` — ONE synchronous call fills the 24-entry
  `texture_logo_e_texnames` list (TAdSEGAlogo.cpp:20-45) in stream order
  (attr 0..23), binds all 24 GVRT records, and hands the handle to
  `TAdSEGAlogo::render` via `render_logo_with_default(handle, 8, -1, -150.0f)`;
  `TAdSoniclogo::load_sonic_logo_texture` just re-handles the same object
  (`get_logo_texture_e_handle()`) — one supply serves ADX+SEGA+ST blocks.
- title leg: `initialize_lobby_title_scene` (TTitleScene.cpp:393-399) →
  `title_texture_filename_table` = `{"Title", "Title_eng" x4}` →
  `load_streamed_prs_texture(names.archive[current_language],
  &title_texture_list)` — the multi-record walk, 21 records, sprites index it
  via `TITLE_SPRITE_TEXTURE_LIST`/`title_sprite_frames`.
- sinbd leg (landed, BB2): `load_hud_palette_textures`
  (TWindowBase.cpp:869-900) = GVPL sidecars → tlut banks 0x40.., THEN
  `load_disc_prs_texture("f256_hyouji", ...)` — palette-attached PAL8.
- **Gcci is NOT a GVR consumer**: `src/pso/system/movie/Gcci.cpp` is the
  async DVD request engine (cancel/status/transferred-size) used by
  `AdxManager` for `.adx` movie/audio streams — its seam is the step-4 DVD
  shim surface (already ported in the bringup shim), never
  `load_streamed_prs_texture`. Do not route logo/title frames through Gcci.

**Per-record expand-leg differences vs the landed PAL8 (BB2/sinbd) leg:**
1. **No CLUT attach**: logo_e + Title I4 frames (fmt 0x0E / 0x80E) carry no
   palette; the sinbd leg's GVPL→tlut normalization (BB-3) and
   `set_context_texture_palette_bank` attach DO NOT apply. PAL8
   (0x0A09)/C4 (0x0A08) frames in hyouji keep it; logo_e frames skip it.
2. **4bpp sizing**: pixels = w*h/2 (0x8000 @256x256, 0x2000 @128x128,
   0x20000 @512x512 I4 in Title) — BB-4's N x N 1:1 quad still applies, but
   the texel-exact L7-P1 leg must unpack 2 texels/byte before comparison.
3. **Hybrid byte order** inside one stream: LE sizes vs BE desc/dir (S0) —
   driver-side readers that hard-coded BE sizes for the sinbd fixture need
   the era bswap legs or they overrun (era walk end == buffer end is the
   gate invariant).
4. **24 records, one 0x100000 buffer** (era `alloc(0x100000)` cap in
   `load_streamed_prs_texture`): logo_e decodes to 0x78530 and Title* to
   0xf0480 — both fit; cache heap stays 0x40000-era unless the driver holds
   all surfaces simultaneously (24 I4 surfaces = 0xA8000 of pixels — the
   driver seam must stream-per-frame or grow the supply arena, NOT the era
   heap).
5. **Duplicate tail frames are real supply** (blank/tail I4 tiles repeat:
   fnv `0x5B73ADEAFF301D0C` x6, `0xC8D2B06B7CF1BB0C` x3 across blocks in
   Logo_e.prs) — dedupe in the gate, not in the walk.

**Gate shape (proposed, mirrors BB2's 12-point regime per lane scope):**
slice each needed GVRT from `local_assets/logo_e.gvm` at the S2 offsets,
re-hash fnv1a64 per slice == S2 table (byte-identity, disc-free); bind
through the era walk (`load_gvm_texture_archive` body or driver equivalent)
with `texture_logo_e_descriptor` names; per consumed frame assert dir-attr
index == stream order and dims/fmt vs S2; L7-P1-style texel-exact leg on at
least one 256x256 + one 128x128 frame after 4bpp expand; palette legs
explicitly SKIPPED for 0x0E (assert tlut -1 path); end-state gate =
`render_logo_with_default`-equivalent quad per block (adx/sega/st) with
drawn-coverage (BB-4 genuine regime). Title lane reuses the same gate over
21 records incl. RGB565 (0x805: 2 bytes/texel, no CLUT) — second seam, not
this lane's blocker. No repo make outside the ONE build-lane; this lane ran
ZERO builds (waiver pin untouched).

## Deviations introduced
None (survey + one gitignored asset + README line). No host/build edits.
