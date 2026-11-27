/* P2-BA ladder6 consumer-chain prelude — prepended to the GENERATED slice TU
 * host/bringup/gen_ladder6_slices.py -> build/bringup/gen/ladder6_tex_consumer.cpp
 * (era homes: text_803DD5E0.cpp load_texture_entry 0x803DD5E0-shape dispatch,
 * load_texture_from_memory, load_texture_list; text_803DA798.cpp state/slot
 * chain; text_803D96A8.cpp resolve/init_from_desc; GinjaTextureTable.cpp
 * set_texture_name + tables; psoLoadTexture.cpp:77 psoLoadTexture;
 * TObjSinBoard.cpp load_sin_board_image — contract docs/p2-gx-tex-consumer.md
 * §1.3 named consumer chain, V-tasks V1/V2 of docs/p2-v-wave.md).
 *
 * PER-TU NJS ALLOW-LIST (AR policy, docs/p2-njs-repin-1.md §6): this TU and
 * ladder6_test.cpp are compiled with -I$(REPO)/compat/njs FIRST (bringup.mk
 * rule L6), so "pso/render/GinjaMaterialState.h" and
 * "pso/system/NinjaTypes.h" resolve to the AR SHADOW headers: the
 * NJS_TEXNAME.filename / texaddr / texture slots and NJS_TEXLIST.textures
 * are u32 image handles; every crossing uses pso_njs_g2h / pso_njs_h2g /
 * NJS_HANDLE(T,slot) (compat/njs/pso/system/NinjaTypes.h:43-53). No .c TU
 * receives this flag set (AR §5 silent-truncation ban). The build/Makefile
 * parse-side request (docs/p2-gx-tex-consumer.md §4) stays UNAPPLIED —
 * integrator-owned; this fragment carries the port-slice half.
 *
 * The slice bodies are era-verbatim EXCEPT the enumerated NJS_HANDLE adapter
 * lines (contract §3 sites #1/#2/#3): gen_ladder6_slices.py rewrites exactly
 * those lines, proves the count of every other byte, and prints the edit
 * table into the generated file header — the receipt for V1.
 */
#ifndef LADDER6_TEX_PRELUDE_H
#define LADDER6_TEX_PRELUDE_H

#include "types.h"                      /* PORT types.h (bringup rebind)     */
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

/* Shadow regime: NJS_TEXNAME/NJS_TEXLIST u32 slots + Ginja runtime structs. */
#include "pso/render/GinjaMaterialState.h"   /* -> compat/njs shadow (§ -I L6) */
#include "pso/system/NinjaTypes.h"           /* -> compat/njs shadow           */

/* ---- era headers whose include graph is narrow and assert-safe ----
 * GinjaTextureInput_size/_width_offset/_height_offset LAYOUT_ASSERTs carry
 * waiver flag 1 (RUNTIME-WAIVE — build/waivers/pso_assert_waivers.h,
 * grep-verified at lane intake), so the LP64-widened `void *data` class
 * parses honestly here (AU-5 precedent). */
#include "pso/render/GinjaTexture.h"          /* texture_states globals + lookups */
#include "pso/render/GinjaTextureInput.h"     /* GinjaTextureInput class          */
#include "pso/render/RenderSbssGlobals.h"     /* current_render_context etc.      */
#include "pso/render/GinjaModelQueue.h"       /* texture_transfer_pending (ext C) */
#include "pso/unk/text_803DA798.h"            /* GvrTextureDesc + consumer decls  */
#include "pso/unk/text_803DD5E0.h"            /* load_texture_* decls             */
#include "pso/player/TObjPlayer_8023E480.h"   /* load_texture_if_valid            */

/* ---- DEVIATION LB-1 (TQuestVmDownLoadServer::alloc seam) --------------
 * Home class (d) layout-assert-LOCKED (docs/p2-rung4-contract.md rows 5-8:
 * ChallengeRecord/NJS/TItemData) — cannot execute as a TU yet; the era CALL
 * SITE (TObjSinBoard.cpp:112 `TQuestVmDownLoadServer::alloc(size)`) is kept
 * structurally present in the slice; the allocator is backed by the AU-4
 * locked-cache heap (OSAllocFromHeap) — test TU owns the definition.        */
struct TQuestVmDownLoadServer {
    static void *alloc(u32 size);
    static s32 is_ready();      /* V4 rung4 §5 never-ready invariant owner */
};

/* ---- DEVIATION LB-2 (frame-pump seams) ---------------------------------
 * psoLoadTexture / LoadPvmFileTls pump frames while the GVM stream fills.
 * pump_frames + TLobby::run_frame/debug_print are class (d) homes
 * (TLobby_*.cpp NJS-LOCKED); here they are declared and backed by test-TU
 * no-op counters (AP §5 injection precedent, bringup-3 DA-3: assert
 * call-order/inputs, never pixels). Only the FAILURE legs of psoLoadTexture
 * reach them; the sinbd success walk never calls them.                      */
void pump_frames(void);
struct TLobby {
    static void debug_print(const char *fmt, ...);
    static void run_frame();
};

/* ---- era bindings the slices call whose homes sit in OTHER ladders ------
 * find_gvrt_texture_chunk: text_803D96A8.cpp:160 (0x803D9C98) — linked from
 *   the AE/AN rung's ladder4_gvr_glue.o (decl copied from era header
 *   text_803D96A8.h:18, minus its <dolphin/mtx.h> include, D4).
 * init_texture_header_and_alloc: AU-1 seam (0x803DA12C composition), era
 *   caller decl text_803D96A8.cpp:21 `extern "C" s32 init_texture_header_and_alloc(u8*, s32, s32, s32)`.
 * move_data: P2-Y TLobby_803DE778 slice. DCFlushRange: era PPC cache op,
 *   no-op seam on host coherence regime (deviation LB-5: arm64 coherence
 *   point; era call site copy_texture_data_and_flush :57 stays present).   */
u32 *find_gvrt_texture_chunk(TLobby *lobby, u32 *out_id, u32 *data);
s32 resolve_texlist_entry(s32 *texlist, u32 idx, s32 *out_surface, u32 *out_tlut);
s32 is_gvm_load_ok(void);
/* AU-1 seam — the ladder5 TU defines it with PLAIN C++ linkage (its prelude
 * declares it non-extern-C); match that linkage exactly at this call site. */
s32 init_texture_header_and_alloc(u8 *p, s32 width, s32 height, s32 format);
void move_data(void *dst, const void *src, s32 n);

/* ---- copied era structs (text_803D96A8.h:34-45 verbatim lines; the era
 * header itself pulls <dolphin/mtx.h> — D4 keeps it off executing TUs) ----
 * The A/B puns describe the SAME memory as GinjaTextureState / GvrTextureDesc
 * (surface ptr at 0xC; format/width/height) — the era cross-file call is a
 * type pun; on this unified tree it is spelled as an explicit overload. */
struct InitTextureFromGvrDescA { u8 pad[0xC]; u8 *p; };

struct InitTextureFromGvrDescB { s32 a; u16 b; u16 c; };

struct Ent_803D9E38 { u8 *p; s32 a; s32 b; };

struct ResolveTexlistEntryTable { u8 pad[8]; Ent_803D9E38 arr[1]; };

s32 init_texture_from_gvr_desc(InitTextureFromGvrDescA *texture, InitTextureFromGvrDescB *desc);
/* DEVIATION LB-8: era linkage pun expressed as a typed forwarder — under the
 * era MWCC regime the call site (text_803DA798.cpp:11) bound the extern-C
 * name directly; the pun structs have byte-identical layout at LP64
 * (GinjaTextureState.surface is the u8* p; GvrTextureDesc a/b/c are
 * format/width/height — era text_803D96A8.h shapes kept verbatim above). */
inline s32 init_texture_from_gvr_desc(GinjaTextureState *t, GvrTextureDesc *d) {
    return init_texture_from_gvr_desc((InitTextureFromGvrDescA *)t, (InitTextureFromGvrDescB *)d);
}
static inline void DCFlushRange(void *p, u32 n) { (void)p; (void)n; }

/* is_palette_texture_format: TAdSinglePlyChrSelectGC_803DA038.cpp:26-34
 * (cases 8/9/10). Era homes declare it BOTH as extern "C" s32(int)
 * (text_803DA798.cpp:11-13) and define s32(u8) — MWCC tolerated the drift.
 * The unified slice TU carries ONE regime: C++ decl matching the definition
 * (deviation LB-6, documented in the V1 receipt; call sites pass desc->format). */
s32 is_palette_texture_format(u8 format);

/* psoBe/psoProt accessors (clause R3): bound in the C11 ladder4_supply TU —
 * compat/pso_byteswap.h refuses C++ TUs, the supply TU is the single binder
 * (docs/p2-gx-tex-consumer.md §0 R3; ladder4_supply.c:16-27 precedent). */
extern "C" unsigned int  p2an_be32(unsigned int v);
extern "C" unsigned short p2an_be16(unsigned short v);

/* free_texture_pixels: GinjaTextureHeader.cpp:110-114 (sliced below the
 * release path it serves); OSFreeToHeap = the Y-era OSAlloc port entry
 * (osalloc_era.c:296, era .s home 0x80370BF4), C TU -> extern "C". */
extern "C" void OSFreeToHeap(int heap, void *ptr);
s32 free_texture_pixels(GinjaTextureHeader *header);

/* ---- DEVIATION LB-3 (fail-loud stand-ins for the three loaders the
 * sinbd attr path never reaches). texture_loaders[4] (text_803DD5E0.cpp:26)
 * references all four; attr 0x40000000 == NJD_TEXATTR_TYPE_MEMORY
 * (NinjaTypes.h:201 shadow / :138 era) dispatches to index 1 =
 * load_texture_from_memory, the ONLY loader this rung executes. The other
 * three homes (read_gvrt_file_header = file stream, create_texture_from_input,
 * reference_texture_by_global_index) are class (c)/(d) legs; AG fail-loud
 * stand-in pattern: abort with the era addr if ever called. Test TU defines. */

/* ---- .sbss/.data globals the slices reference (definitions in the test TU)
 * era homes: RenderSbssGlobals.cpp (current_render_context,
 * active_texture_surface_count, locked_cache_heap_handle), GinjaTextureTable
 * .sbss (texture_states/counts, texture_surfaces/counts), text_803D96A8.s
 * (texture_error), TObjSinBoard.cpp:37-40 (sin_board_* block). */
/* current_render_context / texture_states / texture_surfaces / counts /
 * texture_error / active_texture_surface_count / locked_cache_heap_handle
 * all arrive via the headers included above. */
/* ---- sinbd .data block: ERA-GUEST-RESIDENT (pso_njs_h2g round-trips only
 * for arena-window addresses — AR §5 slot-space rule; the era homes of
 * TObjSinBoard.cpp:37-40 are .data inside the guest RAM image, so the port
 * places them in the arena and the sliced bodies address them through the
 * block pointer (D3 rebase precedent, docs/p2-bringup-1.md). */
struct SinBoardBlock {
    NJS_TEXNAME   texname;
    NJS_TEXLIST   texlist;
    GinjaTextureInput input;
    void         *buffer;
};
extern SinBoardBlock *g_sinbd;   /* test binds to arena+0x32000 */
#define sin_board_texture_name     (g_sinbd->texname)
#define signboard_texture_list     (g_sinbd->texlist)
#define sin_board_texture_surface  (g_sinbd->input)
#define sin_board_texture_buffer   (g_sinbd->buffer)
/* TObjSinBoard.cpp:111 — the logo ref entry point (era header TObjSinBoard.h
 * drags the TObject class chain; the def-unit shape is enough here). */
void load_sin_board_image(const void *image, u32 size);

#endif /* LADDER6_TEX_PRELUDE_H */
