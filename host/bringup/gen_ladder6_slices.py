#!/usr/bin/env python3
"""P2-BA V1 — guarded consumer-chain slice generator (ladder6).

Reads the READ-ONLY decomp tree (pso-decomp @ 6b38b5383, DECOMP_ROOT argv),
extracts the contract-named era functions (docs/p2-gx-tex-consumer.md §1.3
named consumer chain + §3 on-logo sites) as VERBATIM line slices, applies the
enumerated NJS_HANDLE adapter table (V1), and emits
  build/bringup/gen/ladder6_tex_consumer.cpp
Every anchor is hard-gated (exact signature line at file uniqueness; slice
terminates at the first column-0 '}'); every adapter edit is count-gated
(apply-and-verify, abort on drift). The emitted header lists the full edit
table = the V1 receipt. Zero bytes of the decomp tree are touched.

Usage: gen_ladder6_slices.py <DECOMP_ROOT> <out.cpp>
"""
import sys, hashlib

ROOT, OUT = sys.argv[1], sys.argv[2]

def F(p): return f"{ROOT}/{p}"

SRC = {
    "dd5e0": F("src/pso/unk/text_803DD5E0.cpp"),
    "da798": F("src/pso/unk/text_803DA798.cpp"),
    "d96a8": F("src/pso/unk/text_803D96A8.cpp"),
    "gtt":   F("src/pso/render/GinjaTextureTable.cpp"),
    "plt":   F("src/pso/render/psoLoadTexture.cpp"),
    "aps":   F("src/pso/ui/frontend/TAdSinglePlyChrSelectGC_803DA038.cpp"),
    "top":   F("src/pso/player/TObjPlayer_8023E480.cpp"),
    "tsb":   F("src/pso/object/TObjSinBoard.cpp"),
    "gmq":   F("src/pso/render/GinjaModelQueue.cpp"),
    "gth":   F("src/pso/render/GinjaTextureHeader.cpp"),
}

def lines(path):
    with open(path, "r") as fh:
        return fh.read().splitlines(keepends=True)

def slice_fn(src, sig):
    """Verbatim slice: exact-unique signature line through first column-0 '}'.
    Returns (text, start_lineno)."""
    L = lines(SRC[src])
    idx = [i for i, l in enumerate(L) if l == sig + "\n"]
    if len(idx) != 1:
        sys.exit(f"GUARD: {SRC[src]}: signature count {len(idx)} (want 1): {sig!r}")
    s = idx[0]
    end = next(i for i in range(s, len(L)) if L[i] == "}\n")
    return "".join(L[s:end + 1]), s + 1

def slice_block(src, sig, terminator="};\n"):
    L = lines(SRC[src])
    idx = [i for i, l in enumerate(L) if l == sig + "\n"]
    if len(idx) != 1:
        sys.exit(f"GUARD: {SRC[src]}: block signature count {len(idx)}: {sig!r}")
    s = idx[0]
    end = next(i for i in range(s, len(L)) if L[i] == terminator)
    return "".join(L[s:end + 1]), s + 1

def slice_line(src, exact):
    L = lines(SRC[src])
    idx = [i for i, l in enumerate(L) if l == exact + "\n"]
    if len(idx) != 1:
        sys.exit(f"GUARD: {SRC[src]}: line count {len(idx)}: {exact!r}")
    return exact + "\n", idx[0] + 1

# ---- V1 adapter table: (slice-key, exact old line, new line, expected count
#      within that slice).  Every entry = a docs/p2-gx-tex-consumer.md §3 site.
EDITS = {
    # site #1 — set_texture_name, GinjaTextureTable.cpp:18-28 (TObjSinBoard:119 caller)
    "gtt/set_texture_name": [
        ("        entry->filename = reinterpret_cast<char *>(input);",
         "        entry->filename = pso_njs_h2g(input); /* BA-V1 §3#1 h2g stamp */", 2),
    ],
    # site #2 — psoLoadTexture.cpp:89 TEXNAME-table double deref
    "plt/psoLoadTexture": [
        ('            TLobby::debug_print("psoLoadTexture:%s load error.", texlist->textures->filename);',
         '            TLobby::debug_print("psoLoadTexture:%s load error.", (const char *)pso_njs_g2h(NJS_HANDLE(NJS_TEXNAME, texlist->textures)->filename)); /* BA-V1 §3#2 */', 1),
    ],
    # texlist walk derefs — attr dispatch + the memory-input leg (:134-:143)
    "dd5e0/load_texture_entry": [
        ("    switch (list->textures[index].attr & 0x5C000000) {",
         "    switch (NJS_HANDLE(NJS_TEXNAME, list->textures)[index].attr & 0x5C000000) { /* BA-V1 §3#2 table handle */", 1),
    ],
    "dd5e0/load_texture_from_memory": [
        ("    chunk = (uaddr)find_gvrt_texture_chunk((TLobby *)&desc, (u32 *)&global_index, (u32 *)list->textures[index].filename);",
         "    chunk = pso_njs_h2g(find_gvrt_texture_chunk((TLobby *)&desc, (u32 *)&global_index, (u32 *)pso_njs_g2h(NJS_HANDLE(NJS_TEXNAME, list->textures)[index].filename))); /* BA-V1 §3#2 table handle + g2h + S4 guest-stamp */", 1),
        ("        chunk = (uaddr)find_gvrt_texture_chunk((TLobby *)&desc, (u32 *)&global_index,",
         "        chunk = pso_njs_h2g(find_gvrt_texture_chunk((TLobby *)&desc, (u32 *)&global_index,", 1),
        ("                                               (u32 *)reinterpret_cast<GinjaTextureInput *>(list->textures[index].filename)->data);",
         "                                               (u32 *)reinterpret_cast<GinjaTextureInput *>(pso_njs_g2h(NJS_HANDLE(NJS_TEXNAME, list->textures)[index].filename))->data));", 1),
        ("            input = reinterpret_cast<GinjaTextureInput *>(list->textures[index].filename);",
         "            input = reinterpret_cast<GinjaTextureInput *>(pso_njs_g2h(NJS_HANDLE(NJS_TEXNAME, list->textures)[index].filename)); /* BA-V1 §3#2 */", 1),
        ("            chunk = (uaddr)input->data;",
         "            chunk = pso_njs_h2g(input->data); /* BA-V1 S4: src travels as u32 guest handle */", 1),
        # BA LB-9 (D1 "port must ADD swap" side, README-byteswap §2.4 working-
        # (README §2.4 working-value rule + AV §0 R3). Contract-scope note:
        # this function's desc is BE-populated whenever the walk found the
        # GVRT chunk (both sinbd attempts land there); the host-order
        # input-record fallback leg (chunk=input->data) is NOT psoBe-correct
        # through this block — that leg belongs to create_texture_from_input
        # semantics and is out of scope for rung 6 (loader LB-3 guard keeps
        # it unreachable here). Named, not smoothed.
        ("    if ((u32)id != 0xFFFFFFFF) {",
         "    /* BA LB-9: psoBe loads of the walk-copied desc + GBIX id — the\n"
         "     * era PPC lwz of BE bytes is exactly these accessors on host. */\n"
         "    desc.format = p2an_be32(desc.format); desc.width = p2an_be16(desc.width);\n"
         "    desc.height = p2an_be16(desc.height); global_index = (s32)p2an_be32((u32)global_index);\n"
         "    if ((u32)id != 0xFFFFFFFF) {", 1),
    ],
    # slot pool writes
    "da798/set_texlist_texture": [
        ("    list->textures[index].texture = texture_states + slot;",
         "    NJS_HANDLE(NJS_TEXNAME, list->textures)[index].texture = pso_njs_h2g(texture_states + slot); /* BA-V1 §3#3 slot h2g */", 1),
    ],
    "d96a8/resolve_texlist_entry": [
        ("    table = texlist->textures;",
         "    table = NJS_HANDLE(NJS_TEXNAME, texlist->textures); /* BA-V1 §3#2 table handle */", 1),
        ("            entry->texture = slot;",
         "            entry->texture = pso_njs_h2g(slot); /* BA-V1 §3#3 */", 1),
        ("        entry->texture = state;",
         "        entry->texture = pso_njs_h2g(state); /* BA-V1 §3#3 */", 1),
    ],
    # draw-side selection (GinjaMaterial.cpp:367/388/421 subscript class, §3#3)
    "da798/set_texture_num": [
        ("    GinjaTextureState *texture = current_render_context->textures->textures[i].texture;",
         "    GinjaTextureState *texture = NJS_HANDLE(GinjaTextureState,\n        NJS_HANDLE(NJS_TEXNAME, current_render_context->textures->textures)[i].texture); /* BA-V1 §3#3 */", 1),
    ],
    # runtime-src guest regime (AV §1.4 S4: src slots travel u32 handles)
    "da798/copy_texture_data_and_flush": [
        ("    move_data(buf, (const void *)src, size);",
         "    move_data(buf, pso_njs_g2h((u32)(uintptr_t)src), size); /* BA-V1 S4 g2h at the sole deref */", 1),
    ],
    # sinboard texlist head write (TObjSinBoard.cpp:116)
    "tsb/load_sin_board_image": [
        ("        signboard_texture_list.textures = &sin_board_texture_name;",
         "        signboard_texture_list.textures = pso_njs_h2g(&sin_board_texture_name); /* BA-V1 §3#2 h2g */", 1),
        # LP64 runtime-widening: GinjaTextureState.surface (u8* p in the A-pun)
        # +&surface->header era +4 -> host +8 (id u32 + align). Deviation LB-7,
        # pinned by the header-through-surface goldens (ladder6_test G5).
        ("    if (init_texture_header_and_alloc(texture->p + 4, desc->b, desc->c, desc->a) != 0) {",
         "    if (init_texture_header_and_alloc(texture->p + 8, desc->b, desc->c, desc->a) != 0) { /* BA LB-7 LP64 header-in-surface widening */", 1),
    ],
}
# init_texture_from_gvr_desc lives in d96a8, so the LB-7 edit belongs there:
EDITS["d96a8/init_texture_from_gvr_desc"] = EDITS["tsb/load_sin_board_image"][1:]
EDITS["tsb/load_sin_board_image"] = EDITS["tsb/load_sin_board_image"][:1]

# ---- slice manifest: (out-name, src, kind, anchor) ----
MANIFEST = [
    ("texture_loaders[4] table", "dd5e0", "block",
     "s32 (*texture_loaders[4])(NJS_TEXLIST *, s32) = {"),
    ("load_texture_entry",       "dd5e0", "fn",
     "s32 load_texture_entry(NJS_TEXLIST *list, s32 index) {"),
    ("load_texture_from_memory", "dd5e0", "fn",
     "s32 load_texture_from_memory(NJS_TEXLIST *list, s32 index) {"),
    ("load_texture_list",        "dd5e0", "fn",
     "s32 load_texture_list(NJS_TEXLIST *list) {"),
    ("set_texture_list",         "da798", "fn",
     "s32 set_texture_list(NJS_TEXLIST *texlist) {"),
    ("set_texture_num",          "da798", "fn",
     "s32 set_texture_num(s32 i) {"),
    ("reset_texture_error",      "da798", "fn",
     "void reset_texture_error(void) {"),
    ("copy_texture_data_and_flush", "da798", "fn",
     "void copy_texture_data_and_flush(void *buf, saddr src, u32 size) {"),
    ("copy_texture_pixel_range", "da798", "fn",
     "s32 copy_texture_pixel_range(GinjaTextureHeader *header, s32 src, s32 offsetRaw, s32 lengthRaw) {"),
    ("copy_texture_pixels",      "da798", "fn",
     "s32 copy_texture_pixels(GinjaTextureHeader *header, s32 src) {"),
    ("reference_loaded_texture", "da798", "fn",
     "s32 reference_loaded_texture(u32 id, u32 tlut_name, s32 format, s32 *slot_out) {"),
    ("create_texture_state",     "da798", "fn",
     "s32 create_texture_state(u32 id, u32 tlut_name, GvrTextureDesc *desc) {"),
    ("set_texlist_texture",      "da798", "fn",
     "void set_texlist_texture(NJS_TEXLIST *list, s32 index, s32 slot) {"),
    ("reload_texture_slot_pixels", "da798", "fn",
     "s32 reload_texture_slot_pixels(s32 slot, s32 src) {"),
    ("release_texture_slot",     "da798", "fn",
     "void release_texture_slot(s32 slot) {"),
    ("bind_texlist_texture",     "da798", "fn",
     "s32 bind_texlist_texture(NJS_TEXLIST *list, s32 index, u32 id, u32 tlut_name, GvrTextureDesc *desc, s32 *slot) {"),
    ("notify_texture_state_ready", "da798", "fn",
     "void notify_texture_state_ready(GinjaTextureState *) {"),
    ("select_texture_by_surface_id", "da798", "fn",
     "s32 select_texture_by_surface_id(u32 id) {"),
    ("release_texture_state",    "da798", "fn",
     "s32 release_texture_state(GinjaTextureState *texture) {"),
    ("init_texture_from_gvr_desc", "d96a8", "fn",
     "s32 init_texture_from_gvr_desc(InitTextureFromGvrDescA *texture, InitTextureFromGvrDescB *desc) {"),
    ("is_gvm_load_ok",           "d96a8", "fn",
     "s32 is_gvm_load_ok() {"),
    ("resolve_texlist_entry",    "d96a8", "fn",
     "s32 resolve_texlist_entry(s32 *texlist_arg, u32 idx, s32 *out_surface, u32 *out_tlut) {"),
    ("GinjaTextureInput::set_render_rect", "gtt", "fn",
     "void GinjaTextureInput::set_render_rect(void *pixels, s32 input_format, s32 input_width, s32 input_height) {"),
    ("set_texture_name",         "gtt", "fn",
     "void set_texture_name(NJS_TEXNAME *entry, GinjaTextureInput *input, u32 index, u32 attributes) {"),
    ("init_texture_states",      "gtt", "fn",
     "void init_texture_states(GinjaTextureState *states, s32 count) {"),
    ("init_texture_surfaces",    "gtt", "fn",
     "void init_texture_surfaces(GinjaSurfaceState *surfaces, s32 count) {"),
    ("find_texture_by_surface_id", "gtt", "fn",
     "s32 find_texture_by_surface_id(u32 id) {"),
    ("find_free_texture_slot",   "gtt", "fn",
     "s32 find_free_texture_slot(GinjaTextureState **texture, GinjaSurfaceState **surface) {"),
    ("find_texture_by_surface_and_tlut", "gtt", "fn",
     "s32 find_texture_by_surface_and_tlut(u32 id, u32 tlut_name) {"),
    ("clear_unreferenced_texture_entry", "gtt", "fn",
     "void clear_unreferenced_texture_entry(GinjaTextureState *texture) {"),
    ("init_texture_tables",      "gtt", "fn",
     "void init_texture_tables(GinjaTextureState *states, s32 count, GinjaSurfaceState *surfaces, s32 surface_count) {"),
    ("psoLoadTexture",           "plt", "fn",
     "s32 psoLoadTexture(NJS_TEXLIST *texlist) {"),
    ("is_palette_texture_format", "aps", "fn",
     "s32 is_palette_texture_format(u8 format) {"),
    ("load_texture_if_valid",    "top", "fn",
     "s32 load_texture_if_valid(NJS_TEXLIST *texlist) {"),
    ("load_sin_board_image",     "tsb", "fn",
     "void load_sin_board_image(const void *image, u32 size) {"),
    ("texture_transfer_pending", "gmq", "line",
     "s32 texture_transfer_pending() { return 0; }"),
    ("free_texture_pixels",      "gth", "fn",
     "s32 free_texture_pixels(GinjaTextureHeader *header) {"),
]

out = []
receipt = []
for name, src, kind, anchor in MANIFEST:
    if kind == "fn":
        body, ln = slice_fn(src, anchor)
    elif kind == "block":
        body, ln = slice_block(src, anchor)
    else:
        body, ln = slice_line(src, anchor)
    key = f"{src}/{name}"
    for old, new, want in EDITS.get(key, []):
        got = body.count(old)
        if got != want:
            sys.exit(f"GUARD: adapter edit count {got} != {want} in {name}: {old!r}")
        body = body.replace(old, new)
        receipt.append(f"  {name}: {want}x {old.strip()[:60]} -> NJS_HANDLE/g2h/h2g form")
    out.append(f"/* ---- verbatim {name} @ {SRC[src]}:{ln} ---- */\n#pragma push\n{body}\n#pragma pop\n")

hdr = ("/* GENERATED by host/bringup/gen_ladder6_slices.py — P2-BA V1/V2 consumer chain.\n"
       " * Era sources READ-ONLY @ decomp root argv[1]; every slice anchor + the full\n"
       " * adapter edit table is replay-verified by the generator (fail-loud before\n"
       " * any write). Adapter edits applied (V1 receipt):\n"
       + "\n".join(receipt) + "\n */\n"
       '#include "ladder6_tex_prelude.h"\n')

with open(OUT, "w") as fh:
    fh.write(hdr + "\n".join(out))
print(f"gen: {OUT} ({len(MANIFEST)} slices, {sum(len(v) for v in EDITS.values())} adapter edit rules)")
print(f"     source-sha dd5e0 {hashlib.sha256(open(SRC['dd5e0'],'rb').read()).hexdigest()[:12]} "
      f"da798 {hashlib.sha256(open(SRC['da798'],'rb').read()).hexdigest()[:12]} "
      f"gtt {hashlib.sha256(open(SRC['gtt'],'rb').read()).hexdigest()[:12]}")
for r in receipt:
    print("EDIT", r.strip())
