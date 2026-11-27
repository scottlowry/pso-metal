/* P2-AR NJS handle re-pin — SHADOW header (compat/njs). NOT adopted; see
 * compat/njs/pso/system/NinjaTypes.h for the mechanism and constraints
 * (requires the port types.h rebind; earliest -I wins; era text verbatim
 * except P2-AR-marked lines).
 *
 * Re-typed here: NJS_TEXNAME.filename and the texaddr/texture union members
 * — AJ's obligation-1 slots `NJS_TEXNAME.filename/texaddr` (the texname
 * records a POF0-patched NJS_TEXLIST.textures handle addresses; the GVM
 * runtime builder LoadPvmFileTls fills THE SAME layout, which is why AJ
 * LOCKED the type rather than waiving it). Every other struct in this header
 * is host-runtime state with no era assert — untouched. */
#ifndef PSO_RENDER_GINJAMATERIALSTATE_H
#define PSO_RENDER_GINJAMATERIALSTATE_H
#ifndef TYPES_H
#include "types.h"
#endif
#include "pso/system/NinjaTypes.h" /* P2-AR: resolves to the compat/njs shadow under -Icompat/njs first */
typedef struct GinjaParameter {
  u8 kind;
  u8 reserved[3];
  u32 value;
} GinjaParameter;
// A texture's GX image: init_ginja_texture_header fills it from the format word of the texture's source.
typedef struct GinjaTextureHeader {
  u32 format;
  u32 reserved04;
  u32 reserved08;
  u32 width;
  u32 height;
  u32 size; /* GXGetTexBufferSize of the image */
  u32 flags; /* 0x80000000 mipmapped, 0x8000 colour-indexed */
  u32 *pixels; /* texel buffer, allocated from the locked cache heap */
} GinjaTextureHeader;
typedef struct GinjaSurfaceState {
  u32 id;
  GinjaTextureHeader header;
  s32 ref_count; /* textures sharing this surface; freed at 0 */
} GinjaSurfaceState;
typedef struct GinjaTextureState {
  u32 reserved00;
  u32 reserved04;
  u32 tlut_name; /* palette bank for GXInitTexObjCI */
  GinjaSurfaceState *surface;
  s32 ref_count; /* decremented on release, entry cleared at 0 */
  u32 reserved14;
} GinjaTextureState;
// One texture of an NJS_TEXLIST: its name and, once loaded, its texture (or a global index).
struct NJS_TEXNAME {
  u32 filename; /* P2-AR: was char * — name-string image handle (POF0 slot) */
  u32 attr;
  union {
    u32 texaddr;      /* P2-AR: was uaddr — texel/image handle slot */
    u32 texture;      /* P2-AR: was GinjaTextureState * — runtime state handle */
    u32 global_index;
  };
};
/* Era width was 0xC on PPC (uaddr == u32 == pointer there). The era header
 * carries NO LAYOUT_ASSERT for this struct — the byte-truth pins below are
 * the shadow's own contract; pso_njs_h2g/NJS_HANDLE convert at access. */
PSO_NJS_BYTE_TRUTH(NJS_TEXNAME_size, sizeof(NJS_TEXNAME) == 0xC);
PSO_NJS_BYTE_TRUTH(NJS_TEXNAME_filename_offset, offsetof(NJS_TEXNAME, filename) == 0x0);
PSO_NJS_BYTE_TRUTH(NJS_TEXNAME_attr_offset, offsetof(NJS_TEXNAME, attr) == 0x4);
PSO_NJS_BYTE_TRUTH(NJS_TEXNAME_texaddr_offset, offsetof(NJS_TEXNAME, texaddr) == 0x8);
PSO_NJS_BYTE_TRUTH(NJS_TEXNAME_global_index_offset, offsetof(NJS_TEXNAME, global_index) == 0x8);
// Era idiom: (GinjaTextureState *)tn->texture -> NJS_HANDLE(GinjaTextureState, tn->texture);
// set_texture_name (GinjaTextureTable.cpp:18) must keep these u32 per AJ.

// A render context: render_contexts holds six, select_render_context makes one current.
typedef struct GinjaSceneState {
  u32 reserved00;
  u32 reserved04;
  GinjaTextureHeader *texture_header; /* image of the texture set_texture_num selected */
  u32 blend_mode; /* +0x0C: GX blend factors, source in bits 29-31, destination in bits 26-28 */
  u32 reserved10;
  s32 reserved14; /* set to -1 on init and whenever a texture surface is freed */
  u32 tlut_name; /* palette bank of the selected texture */
  u32 reserved1C;
  NJS_TEXLIST *textures;
  u32 reserved24;
} GinjaSceneState;

// The six render contexts (gjSystem.c).
extern GinjaSceneState render_contexts[6];

#ifdef __cplusplus
extern "C" {
#endif
// Resets a render context to the default blend mode, no texture and no palette.
void init_render_context(GinjaSceneState *context);
#ifdef __cplusplus
}
#endif
#endif
