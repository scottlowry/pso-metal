/* ladder4 gvr-glue prelude — prepended to the VERBATIM
 * find_gvrt_texture_chunk slice (text_803D96A8.cpp:160-186, home
 * 0x803D9C98; full instruction trace in build/GPOE8P/asm/pso/unk/
 * text_803D96A8.s :9CD0-:9D88).
 *
 * DEVIATION AN-1 (this file): the era swap call site
 *   text_803D96A8.cpp:155-158 swap_gvrt_word  (0x803D9C60, .s :9C60-:9C84)
 * is provided here as the DOCUMENTED IDENTITY SEAM, routed through the
 * sanctioned psoProt32 accessor (compat/README-byteswap.md §2: "the port
 * keeps the *existing* swap site as the identity seam (psoProtSwapAll32...)
 * and does NOT add psoBe* there — adding one double-swaps the magic/size").
 * On PPC psoProt32 becomes the byte mirror again automatically
 * (pso_byteswap.h:93-98), so this is the era transform at the seam, not a
 * deletion. The slice's `swap_gvrt_word(&tag)` call remains structurally
 * present (net-disc §5 rule 1). Era-body proof lives in golden B1
 * (ladder4_test.cpp: p2an_era_mirror32 chain == psoProtRead32 chain).
 *
 * DEVIATION AN-2 (test TU): fields the era reads RAW (unswapped) —
 * GBIX id 0x803D9D40 `lwz r0,0(r29)` and the 8-byte desc move_data copy
 * 0x803D9D0C-:9D18 — receive their psoBe conversion at the SUPPLY boundary
 * in ladder4_test.cpp, never inside the slice (README §2.4 working-value
 * rule; this is the D1 "silent-flip, port must ADD swap" side of the trap).
 */
#ifndef LADDER4_GVR_PRELUDE_H
#define LADDER4_GVR_PRELUDE_H

#include "types.h"
#include <stddef.h>

struct TLobby;   /* opaque: the slice only treats it as the 8-byte desc dst */

/* The image description at the start of a GVRT chunk — declaring header
 * include/pso/unk/text_803DA798.h:11-15 (u32 format; u16 width; u16 height;
 * = 8 bytes, exactly the li r5,0x8 move length). */
typedef struct GvrTextureDesc {
    u32 format;
    u16 width;
    u16 height;
} GvrTextureDesc;

/* GinjaTextureError shadow of include/pso/render/GinjaTexture.h:6-14 (its
 * own LAYOUT_ASSERT pins 0x20; all-s32, so the host layout is identical).
 * Era store: texture_error.code — 0x803D9D5C-:9D68 `li r0,0x4 / lis+addi
 * texture_error@ha/@l / stw r0,0x8(r3)` proves code@0x8. */
typedef struct GinjaTextureError {
    s32 entry_index;   /* +0x00 */
    s32 pad_04;
    s32 code;          /* +0x08 */
    s32 pad_0C;
    s32 pad_10;
    s32 pad_14[3];
} GinjaTextureError;
_Static_assert(sizeof(GinjaTextureError) == 0x20, "GinjaTextureError 0x20 (GinjaTexture.h:14)");
_Static_assert(offsetof(GinjaTextureError, code) == 0x08, "code@8 (stw 0x8(r3) @0x803D9D68)");

/* Era global (GinjaTexture.h:18; def unit unparsed — .sbss symbol
 * texture_error@ha=0x8059 @l=0xD470 per text_803D96A8.s :9D60-:9D64). */
GinjaTextureError texture_error;

/* TLobby_803DE778.cpp move_data (memmove wrapper, memory.cpp:73 pattern) —
 * LINKED from the P2-Y generated verbatim slice move_data_glue.o. */
void move_data(void *dst, const void *src, s32 n);

/* pso_byteswap.h seam wrappers (C11 TU ladder4_supply.c). */
extern "C" uint32_t p2an_prot32(uint32_t v);

/* AN-1 identity seam, era-signature preserved (text_803D96A8.h:12). */
void swap_gvrt_word(u32 *p) { *p = (u32)p2an_prot32((uint32_t)*p); }

#endif /* LADDER4_GVR_PRELUDE_H */
