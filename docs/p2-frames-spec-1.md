# P2-FRMSPEC — genuine-frames BUILDER SPEC (logo_e → title), paste-ready

Fleet wave-5 lane FRMSPEC, 2026-11-27. Card `t_958e16a2`. ZERO builds, spec
lane: this doc is the sole write. Authority: `docs/p2-logo-e-supply-1.md`
§S2/§S4 (manifest + I4P brief), `docs/p2-hg1-1.md` (window registry — any
builder handle traffic goes through `pso_njs_g2h/h2g_*`, HG-1 law), and
`docs/p2-v-wave3.md` (BB2 GX_LOGO_GVR seam — the gate pattern copied here).
Decomp `pso-decomp @6b38b5383` READ-ONLY, quoted verbatim below. Waiver pin
`d4382fcd…` untouched (no build run). Disc path never printed; no asset
bytes committed.

## F0 — same-turn supply verification (precondition, MET)

Against `host/bringup/local_assets/logo_e.gvm` (gitignored; README-cited):

| check | expected | measured same-turn |
|---|---|---|
| size | 0x78530 | 0x78530 ✓ (492,848 B) |
| fnv1a64 (whole file) | 0x1C29B433471C69A8 | ✓ |
| magic / hdr.size LE / flags BE / count BE | GVMH / 0x3a8 / 0x000F / 24 | ✓ all |
| chunk walk (start `hdr.size+8`, each `size+8`) | end == 0x78530 | 24 GVRT, end == file end ✓ |
| per-slice fnv1a64 at manifest §S2 offsets | 24/24 == S2 table | ✓ 24/24 re-hashed from THIS file this turn |
| directory decode (see F2) | 24 entries stride 0x26, attr 0..23 | ✓ names == manifest, GBIX as below |

Slice receipts persisted to lane scratch
(`$TMP/frmspec/logo_e_slices.json`, off-repo) for the builder to diff against.

**Two I4P-brief corrections (set-math, same-turn):** §S4.5 says the dup tail
hash `0x5B73ADEAFF301D0C` occurs "x6" — across Logo_e.prs it occurs **7×**
(GVRT[06] gc_adx07, [12] sega_e05, [13] e06, [14] e07, [20] st05, [21] st06,
[22] st07); `0xC8D2B06B7CF1BB0C` ×3 confirmed ([07],[15],[23]). §S4.4 says
"24 I4 surfaces = 0xA8000" — packed pixel bytes total 12×0x8000 + 12×0x2000
= **0x78000** (0xA8000 overstates by 0x30000; the RGBA8 gate expansion leg
would hold 4× that = 0x1E0000 if the driver unpacks-then-keeps).

## F1 — anchor block: the `register_texture_logo_e` consumer path

Paste-ready, verbatim from decomp. All names/strings below are the era
spellings the builder must reproduce 1:1.

`src/pso/ui/frontend/TAdSEGAlogo.cpp:18-52` — descriptor + 24-entry table +
handle global (stream order == attr 0..23 == GVRT order in the archive):

```cpp
#pragma push
#pragma readonly_strings on
#pragma const_strings off
// The logo textures: eight ADX, eight SEGA and eight Sonic Team frames.
NJS_TEXNAME texture_logo_e_texnames[24] = {
    { "gc_adx01", 0, 0 },
    { "gc_adx02", 0, 0 },
    { "gc_adx03", 0, 0 },
    { "gc_adx04", 0, 0 },
    { "gc_adx05", 0, 0 },
    { "gc_adx06", 0, 0 },
    { "gc_adx07", 0, 0 },
    { "gc_adx08", 0, 0 },
    { "sega_logo_e01", 0, 0 },
    { "sega_logo_e02", 0, 0 },
    { "sega_logo_e03", 0, 0 },
    { "sega_logo_e04", 0, 0 },
    { "sega_logo_e05", 0, 0 },
    { "sega_logo_e06", 0, 0 },
    { "sega_logo_e07", 0, 0 },
    { "sega_logo_e08", 0, 0 },
    { "st_logo01", 0, 0 },
    { "st_logo02", 0, 0 },
    { "st_logo03", 0, 0 },
    { "st_logo04", 0, 0 },
    { "st_logo05", 0, 0 },
    { "st_logo06", 0, 0 },
    { "st_logo07", 0, 0 },
    { "st_logo08", 0, 0 },
};
__declspec(section ".data") NJS_TEXLIST texture_logo_e_descriptor = { texture_logo_e_texnames, 24 };
#pragma pop

NJS_TEXLIST *logo_texture_e_handle;
```

(stream order == attr 0..23 == GVRT record order; brand order fixed:
`gc_adx01..08`, `sega_logo_e01..08`, `st_logo01..08`.)

`TAdSEGAlogo.cpp:102-106` — registration (ONE synchronous call fills all 24):

```cpp
#pragma readonly_strings on
void register_texture_logo_e(void) {
    logo_texture_e_handle = &texture_logo_e_descriptor;
    load_streamed_prs_texture("logo_e", &texture_logo_e_descriptor);
}
```

`TAdSEGAlogo.cpp:95-100` (release twin) and `src/pso/ui/TLogo.cpp:38-51,81`
(when it runs): `register_texture_logo_e, TLogo::free_held_object` is row 3
of `logo_load_callback_table[18]`; `TLogo::init_logo_scene()`
(`TLogo.cpp:66-135`) ends with
`run_load_callback_table(logo_load_callback_table, sizeof(...))` —
load front-to-back, release back-to-front.

Consumer render leg (`TAdSEGAlogo.cpp:66-68`):

```cpp
void TAdSEGAlogo::render() {
    render_logo_with_default(logo_texture_e_handle, 8, -1, -150.0f);
}
```

Loader chain the builder must reproduce byte-for-byte:
`src/pso/player/TObjPlayer_8023E480.cpp:37-52` —

```cpp
s32 load_streamed_prs_texture(const char *name, NJS_TEXLIST *texlist) {
    ...
    sprintf(texture_prs_stream_filename_buffer, "%s.prs", name);
    h = load_disc_file(texture_prs_stream_filename_buffer);
    ...
    buf = TQuestVmDownLoadServer::alloc(0x100000);
    prs_decompress_buffer(h, buf);
    load_gvm_texture_archive(buf, texlist);
    ...
}
```

`src/pso/unk/text_803DD5E0.cpp:394-462` — `load_gvm_texture_archive` body
(GVMH bswap pair, `list->nbTexture = header->texture_count`,
`replace_gvm_texture_directory(list, calc_gvm_entry_stride(header->flags),
header)`, `gvm_stream_seek(header->size + 8, 0)`, `seek_gvrt_chunk()`, then
per record: attr-low-16 re-scan for stream order (`:424-429`),
`resolve_texlist_entry`, GVRT magic+size bswap, `data =
gvm_stream_read(chunk->size - 8)` (text_803DD5E0.cpp:442), `bind_texlist_texture`, and when it
returns 0 → `reload_texture_slot_pixels(slot, data)` + spin on
`is_gvm_load_ok()`). Era PRS decoder = `prs_decompress_buffer`
(`src/pso/system/TScreenFadeController.cpp:309` → `PrsDecoder::prs_decompress`,
`src/pso/system/prs.cpp`, 16/16 stage-19 golden; byte-0 control-bit stream,
NO 0x0D type byte — I4 §S0 correction stands).

The `0x100000` alloc cap at `TObjPlayer_8023E480.cpp:45` covers both archives
(logo_e 0x78530, Title* 0xf0480) — builder keeps the constant, era-shaped.

## F2 — anchor block: GBIX globals → era symbols

Directory record decode (verified byte-by-byte against the file this turn;
matches I4 §S0 "stride 0x26": base 0xc, per entry
`attr u16 BE | name u8[0x1C] | fmt u16 BE | dims u16 BE (nibble codes
2^(n+2)) | GBIX u32 BE`):

Era consumers, verbatim legs:

`src/pso/unk/text_803DD5E0.cpp:302-349` `replace_gvm_texture_directory` —
flags 0x000F ⇒ names|formats|dimensions|globals legs taken: copies the u32
into `texture->global_index` and stamps `texture->attr |= 0x00800000`
(no `alternateGlobal` leg — flags bit 0x400 clear on this archive).
`src/pso/unk/text_803D96A8.cpp:207-260` `resolve_texlist_entry` — the
`flags & 0x800000` leg: `*out_surface = entry->texaddr; *out_tlut = -1U;`
— i.e. **the GBIX global IS the era surface-id** for every logo_e frame
(union slot: `texaddr/texture/global_index` — the in-repo L6-G3 note,
`host/bringup/ladder6_test.cpp:144`).

**Era symbol names:** grep at @6b38b5383 for `0x1B0A8`/`0x1AFC2`/`0x10CAB4`
(and lowercase variants) returns **zero code/data symbols** — these values
exist ONLY as directory bytes on disc; the era's *named* handle for them is
the `texture_logo_e_texnames[k]` slot itself (name string + global_index
union). The builder therefore supplies its own era-shaped constants; pinned
naming:

```
LOGO_E_GBIX_GC_ADX(k)  = 0x0001B0A8 + k   // k = 0..7  gc_adx01..08
LOGO_E_GBIX_SEGA(k)    = 0x0001AFC2 + k   // k = 0..7  sega_logo_e01..08
LOGO_E_GBIX_ST(k)      = 0x0010CAB4 + k   // k = 0..7  st_logo01..08
```

All 24 GBIX values are distinct ⇒ the era walk never merges frames by id
(`find_texture_by_surface_id` misses every time); the 7×/3× duplicate
content frames still get **separate surfaces + separate byte copies** in the
era regime. That is why dedupe belongs in the gate (F4), not the walk.

## F3 — anchor block: TAdSoniclogo (+TAdADXlogo) re-handle sites

`src/pso/ui/frontend/TAdSoniclogo.cpp:60` —

```cpp
void load_sonic_logo_texture() { sonic_logo_object = get_logo_texture_e_handle(); }
```

`src/pso/ui/frontend/TAdADXlogo.cpp:71` + `:65-67` —

```cpp
NJS_TEXLIST *get_logo_texture_e_handle() { return logo_texture_e_handle; }
...
void load_adx_logo_texture() { TAdADXlogo::texture = get_logo_texture_e_handle(); }
```

renders: `TAdSoniclogo.cpp:30` `render_logo_with_default(sonic_logo_object,
16, -1, -150.0f)` (ST block, tex_base 16), `TAdADXlogo.cpp:33`
`render_logo_with_default(TAdADXlogo::texture, 0, -1, -150.0f)` (ADX block,
tex_base 0), SEGA leg tex_base 8 (F1). `render_logo_with_default` =
`src/pso/ui/frontend/TitleBackground.cpp:58-60` → `draw_title_background_quads`
(tex_base indexes the 24-entry list directly — **tex_base 0/8/16 IS the
brand-block selector**). Table rows in `TLogo.cpp:45-48` add
`load_sonic_logo_texture/free_sonic_logo_texture` and
`load_adx_logo_texture/free_adx_logo_texture` after the register row;
`free_sonic_logo_texture` / `free_adx_logo_texture` are EMPTY bodies
(`TAdSoniclogo.cpp:58`, `TAdADXlogo.cpp:62-64`) — only
`TLogo::free_held_object` releases, once. Builder gate invariant: after
registration, all three handles are the SAME pointer
(`logo_texture_e_handle == sonic_logo_object == TAdADXlogo::texture
== &texture_logo_e_descriptor`); one supply serves ADX+SEGA+ST.

## F4 — I4 expand-leg spec vs era decoder (+ dedupe plan)

**Which era fn owns the 4bpp unpack on this path: NONE.** The walk copies
packed bytes verbatim and never decodes 2-in-1:

- `src/pso/unk/text_803DA798.cpp:168-177` `reload_texture_slot_pixels` →
  `copy_texture_pixel_range(&surface->header, src, 0, header.size)`
  (`:57-76`) → `copy_texture_data_and_flush` (`:52-55`, move + `DCFlushRange`).
  Payload length seen by the walk = `chunk->size - 8` (text_803DD5E0.cpp:442)
  = 0x8000 @256×256, 0x2000 @128×128 — exactly `w*h/2`.
- Surface sizing is the foreign leg `init_texture_from_gvr_desc`
  (`text_803D96A8.cpp:192-198`) → `init_texture_header_and_alloc(u8*,s32,s32,s32)`
  (declared `text_803D96A8.cpp:23`, four-arg body not decompiled at
  @6b38b5383 — builder must key off `chunk->size-8` for copy length, NOT
  off a re-derived format sizing).
- Palette legs are structurally skipped: `is_palette_texture_format`
  (`src/pso/ui/frontend/TAdSinglePlyChrSelectGC_803DA038.cpp:26-34`) is a
  `switch (u8 format)` over **8, 9, 10 only**. GVRT desc fmt is read low
  byte: 0x0000000E → 14, 0x0000080E → 14 ⇒ non-palette ⇒ the
  `create_texture_state` else-leg `texture->tlut_name = -1`
  (`text_803DA798.cpp:148-152`). PAL8 (0x0A09→9) / C4 (0x0A08→8) frames
  keep the BB2 tlut-attach regime; logo_e frames take the -1 path and the
  gate ASSERTS it (no `set_context_texture_palette_bank`, no GVPL sidecar
  hook — BB-3 normalization is PAL8-leg-only and does not apply).
- Real texel interpretation happens at DRAW time (Ginja/GX format fetch,
  DC-side `GX_TF_I4`): the gate's CPU unpack is a host-side comparison aid
  only. BB2's precedent lives in `host/gx/dl/gx_logo_clut.h`
  (`gxh_pal8_clut_expand_lut`, PAL8→RGBA8 through the loaded bank).

**Builder expansion step (spelled, this lane builds nothing):** add a
`gxh_i4_rgba8(const unsigned char *packed, unsigned char *rgba8, size_t
texels)` sibling — byte k covers texels 2k (HIGH nibble) and 2k+1 (LOW
nibble), row-major, top row first (same regime comment law as gx_logo_clut.h:36);
value v → rgba = (v<<4|v, v<<4|v, v<<4|v, 0xFF) — I4 is opaque replicate
(to be pinned by the L7-P1-style exactness leg; if a corner sample inverts,
the fix is the nibble order, not the walk). Nib­ble order expectation: GX
I4 texel 0 in the high nibble — the era never unpacks on host so this is
the ONE working assumption in this spec; the gate leg is what falsifies it.

**Dedupe plan (by fnv, in the gate only):** slice-fnv groups over the 24
records = 16 distinct hashes; groups: `0x5B73ADEAFF301D0C` {06,12,13,14,20,
21,22} (7 slots), `0xC8D2B06B7CF1BB0C` {07,15,23} (3 slots), all others
singletons. Gate runs the texel-exact leg over the 16-unique SET (once per
hash), while binding/walk legs run for all 24 slots (era keeps them separate
— F2). Cross-file identity (st_logo01..04 bytes are shared with Logo.prs /
st_logo.prs per manifest §S2 tables; gc_adx01..04 differ EN-vs-JP;
sega_logo_e03 exists in TWO variants — Logo_e.prs `21CB09DE…` vs
sega_logo_e.prs `D870B71B…`) is expected: the gate slices ONLY from
`logo_e.gvm` and compares against the F5 table.

## F5 — slice-offset table for logo_e.gvm (file offsets; slice == [off, off+size))

File IS the decompressed archive (I4 §S3: `logo_e.gvm` == the era `buf`), so
in-archive offsets are FILE offsets. dir = directory record base; desc read
from GVRT hdr @+8 fmt u32 BE, +C w, +E h. Verified same-turn (F0).

| i | name | dir_off | attr | GBIX | slice_off | size | dims | fnv1a64 (GVRT incl. 8 B hdr) |
|---|---|---|---|---|---|---|---|---|
| 0 | gc_adx01 | 0x000c | 0 | 0x0001B0A8 | 0x003b0 | 0x8010 | 256² | 004075B4779DA374 |
| 1 | gc_adx02 | 0x0032 | 1 | 0x0001B0A9 | 0x83c0 | 0x8010 | 256² | 9145B1657183FE4A |
| 2 | gc_adx03 | 0x0058 | 2 | 0x0001B0AA | 0x103d0 | 0x8010 | 256² | EC7350D7705E3CB5 |
| 3 | gc_adx04 | 0x007e | 3 | 0x0001B0AB | 0x183e0 | 0x8010 | 256² | CA08712AA39BB83A |
| 4 | gc_adx05 | 0x00a4 | 4 | 0x0001B0AC | 0x203f0 | 0x2010 | 128² | CA976BB52B992D8C |
| 5 | gc_adx06 | 0x00ca | 5 | 0x0001B0AD | 0x22400 | 0x2010 | 128² | 6C76EBAFA97CE58D |
| 6 | gc_adx07 | 0x00f0 | 6 | 0x0001B0AE | 0x24410 | 0x2010 | 128² | 5B73ADEAFF301D0C |
| 7 | gc_adx08 | 0x0116 | 7 | 0x0001B0AF | 0x26420 | 0x2010 | 128² | C8D2B06B7CF1BB0C |
| 8 | sega_logo_e01 | 0x013c | 8 | 0x0001AFC2 | 0x28430 | 0x8010 | 256² | A98C4BF3926BC315 |
| 9 | sega_logo_e02 | 0x0162 | 9 | 0x0001AFC3 | 0x30440 | 0x8010 | 256² | E604EFC97A26324A |
| 10 | sega_logo_e03 | 0x0188 | 10 | 0x0001AFC4 | 0x38450 | 0x8010 | 256² | 21CB09DE9F06F010 |
| 11 | sega_logo_e04 | 0x01ae | 11 | 0x0001AFC5 | 0x40460 | 0x8010 | 256² | 2AF27918A5EFA17A |
| 12 | sega_logo_e05 | 0x01d4 | 12 | 0x0001AFC6 | 0x48470 | 0x2010 | 128² | 5B73ADEAFF301D0C |
| 13 | sega_logo_e06 | 0x01fa | 13 | 0x0001AFC7 | 0x4a480 | 0x2010 | 128² | 5B73ADEAFF301D0C |
| 14 | sega_logo_e07 | 0x0220 | 14 | 0x0001AFC8 | 0x4c490 | 0x2010 | 128² | 5B73ADEAFF301D0C |
| 15 | sega_logo_e08 | 0x0246 | 15 | 0x0001AFC9 | 0x4e4a0 | 0x2010 | 128² | C8D2B06B7CF1BB0C |
| 16 | st_logo01 | 0x026c | 16 | 0x0010CAB4 | 0x504b0 | 0x8010 | 256² | 6B68A20C27DFB45A |
| 17 | st_logo02 | 0x0292 | 17 | 0x0010CAB5 | 0x584c0 | 0x8010 | 256² | 59DFB39C8A13C82E |
| 18 | st_logo03 | 0x02b8 | 18 | 0x0010CAB6 | 0x604d0 | 0x8010 | 256² | 8D837BB76E5B10F0 |
| 19 | st_logo04 | 0x02de | 19 | 0x0010CAB7 | 0x684e0 | 0x8010 | 256² | 766830662629C474 |
| 20 | st_logo05 | 0x0304 | 20 | 0x0010CAB8 | 0x704f0 | 0x2010 | 128² | 5B73ADEAFF301D0C |
| 21 | st_logo06 | 0x032a | 21 | 0x0010CAB9 | 0x72500 | 0x2010 | 128² | 5B73ADEAFF301D0C |
| 22 | st_logo07 | 0x0350 | 22 | 0x0010CABA | 0x74510 | 0x2010 | 128² | 5B73ADEAFF301D0C |
| 23 | st_logo08 | 0x0376 | 23 | 0x0010CABB | 0x76520 | 0x2010 | 128² | C8D2B06B7CF1BB0C |

Pattern law for the builder (no table needed at runtime): hdr 0x3a8 ⇒ dir
0xc..0x39c (24×0x26), pad to 0x3a8, first GVRT at 0x3b0; slices are
contiguous — per brand block `0x8010×4` then `0x2010×4`, walking
`off += (u32 LE at off+4) + 8`. Gate invariant = era walk end == file end
(F0). Every desc word: fmt `0x0000000E`, w/h BE.

## F6 — gate shape (hook-gated; NO genuine hashes in CI defaults)

Mirror the GX_LOGO_GVR regime (`host/gx/dl/gx_logo_driver.cpp:264-291`:
env hook ⇒ read local blob into arena-parked buffer, print size+fnv
receipts, never echo the path beyond basename policy lines; hook unset ⇒
synthetic fixture legs byte-identical). Builder seam, per lane scope:

1. **Hook:** `GX_FRAMES_GVM=<path-to logo_e.gvm>` (plus optional
   `GX_FRAMES_SLICES=<path-to slices.json>`), consumed the same way. CI
   defaults run WITHOUT the hooks: zero genuine bytes, zero genuine hashes
   anywhere in the TU — the same posture as L7-P3's runtime-reference
   anchor ("computed AT RUNTIME … no hardcoded expected hash",
   gx_logo_driver.cpp:463-466). The F5 fnv table lives in THIS doc + a
   gitignored `local_assets/logo_e.slices.json` sidecar the builder writes;
   with hooks set, per-slice equality is checked against the sidecar loaded
   at runtime — never against literals compiled in.
2. **Supply receipts (FR-S):** magic `GVMH`, hdr.size 0x3a8 LE, flags 0x000F
   BE, count 24 BE; walk 24 GVRT, end == file size; print whole-file fnv.
3. **Walk/bind (FR-W):** feed the buffer through the ported
   `load_gvm_texture_archive` body (F1) with the F1 descriptor; per record
   assert dir attr == stream index, name == descriptor string, dims/fmt ==
   F5, bound surface id == F2 GBIX, `tlut_name == -1` on all 24 (palette
   legs explicitly SKIPPED — I4 §S4.1).
4. **Texel-exactness sample (FR-P1), L7-P1 style but deduped (F4):**
   ≥1 256×256 and ≥1 128×128 frame PER BRAND BLOCK (6 slots; pick unique
   hashes {0,4,8,12*|13,16,20*} — * the one dup slot per block proves slot
   content == dup hash identity), each after the F4 `gxh_i4_rgba8` CPU
   unpack, ±0 against the era-chain surface readback. Remaining 10 unique
   slots ride slice-fnv equality (step 1).
5. **Drawn-coverage end-state (FR-P2), BB-4 regime:** one
   `render_logo_with_default`-equivalent quad per brand block
   (tex_base 0/8/16) at N×N 1:1; coverage = every pixel non-black OR
   matches its unpacked texel. NOTE: I4 is opaque (F4), so unlike BB2's
   a=0 PAL8 escape, strict non-black is provable here; keep the BB-4 shape
   anyway for regime parity — the escape clause simply never fires and the
   gate MUST NOT count it (assert escape hits == 0 on logo_e frames).
6. **Handle identity (FR-H):** F3 three-way `==` check post-register,
   post-free `logo_texture_e_handle == NULL` with the two empty free fns
   NOT NULLing (era bodies untouched).
7. **Budget:** pixels total 0x78000 packed (F0 correction) parked in the
   arena above the cache heap (gvr/gvp park sites `+0x1600000/+0x1640000`,
   gx_logo_driver.cpp:260-261 precedent); if the driver unpacks to RGBA8
   per frame it needs 0x1E0000 OR streams per-frame — supply-arena, never
   the era heap (I4 §S4.4 law), and any host pointers cross the era side
   through HG-1 (`pso_njs_h2g_*`; unregistered ⇒ fail loud, hg1_windows law).
   `fake_disc/logo_e.prs` 9-byte step-19 stand-in stays untouched.

## F7 — follow-on stub: title 21-record walk (RGB565 first-touch)

SPELLED, not built. Supply precondition first: no `Title*.gvm` exists in
local_assets yet — the follow-on lane decompresses `Title.prs` (I4 §S1 fnv
`30CF766998CA9B71`) with the era decoder to 0xf0480 (decomp-fnv
`45EA0D84665F1CA5`) as `local_assets/Title.gvm`, receipts only.

Era route: `src/pso/ui/frontend/TTitleScene.cpp:393-399` —

```cpp
void initialize_lobby_title_scene(void) {
    ...
    TitleTextureFilenameTable names = title_texture_filename_table;   // whole-table copy
    lobby_title_texture_list = &title_texture_list;
    load_streamed_prs_texture(names.archive[current_language], &title_texture_list);
    ...
}
```

`TTitleScene.cpp:213-215` `{"Title","Title_eng","Title_eng","Title_eng","Title_eng"}`;
descriptor `title_texture_entries[21]` + `title_texture_list` at
`:57-81` (order = f256_ad, f256_psotitle, f256_psotitle02, alpha01..08,
f512_mahoujin, _b, s256_1st_title01..08 — matches I4 §S2 Title.prs rows).
`TTitleBackground.cpp:130` consumes tex_base 0xD (s256 block).

Per-record byte-order groups (I4 §S2): fmt words `0x00000805` (rows 0-2,
0x20010 @256²), `0x0000080E` (rows 3-12, 4bpp incl. 512² 0x20010), `0x0000000E`
(rows 13-20). 0x805 low byte 5 ⇒ non-palette (F4 `is_palette_texture_format`
8/9/10) ⇒ also tlut -1; 0x80E low byte 14 ⇒ identical I4 leg as logo_e
(high-bit attr semantics unknown — builder treats only the low byte for
the palette decision, exactly like the era).

**RGB565 first-touch needs its OWN decode-verify step BEFORE the walk is
trusted** (first 16bpp records in repo history; every landed leg so far was
≤8bpp): step T0 — slice GVRT[00] f256_ad at 0x330/0x20010 from Title.gvm,
fnv `C51FA84BE708B078`-class receipt check vs §S2; unpack `u16 BE texel →
RGBA8` (r5<<3|r5>>2, g6<<2|g6>>4, b5<<3|b5>>2, a=255) — the host already
models RGB565 decode quantization in `gxh_tlut_rgba8` fmt==1
(`gx_logo_clut.h:62-66`) — pin: 4 corner texels + centroid of one 565
frame, ±0, AND the byte-count law `chunk->size-8 == w*h*2` (0x20000) as
the sizing gate. Only after T0 green does F6-shape gate run over all 21
records (sample set: one per fmt group + the 512² pair; dedup groups here
are across Title/Title_eng/Title_eng_trial variants — identity vs trial
divergence per §S2 rows, e.g. psotitle 01/02 are EN-shared but trial-distinct).

## Deviations introduced

None. Spec-only lane: one doc, zero builds, zero code/asset writes, decomp
read-only, waiver pin untouched. I4 §S4.5/§S4.4 counts corrected in F0
(from the manifest's own tables, set-math + same-turn re-hash).
