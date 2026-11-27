/* P2-AR NJS handle re-pin — SHADOW header (compat/njs). NOT adopted anywhere:
 * no build file or committed header adds -Icompat/njs; this directory is only
 * ever consulted via explicit probe flags (docs/p2-njs-repin-1.md).
 *
 * Mechanism (assert-policy §4 / host/bringup/include/types.h precedent):
 * every era `#include "pso/system/NinjaTypes.h"` falls through to the -I
 * chain, and the earliest -I wins — placing compat/njs before the decomp's
 * include/ makes THIS file the definition of the NJS_* structs. Same include
 * guard as the era header, so even a same-directory quoted include of the era
 * file is a no-op once the shadow has been processed. The era text is copied
 * verbatim except where marked "P2-AR" below.
 *
 * What changes: the seven image-resident pointer slots that AJ's
 * docs/p2-njs-confirm.md obligation 1 names inside THIS header become u32
 * handle slots per p1-datamodel §3 rule 1 (u32 handles + one base register
 * per guest space, pso_g2h lineage from host/bringup/crt0.c). Field NAMES,
 * order, and every byte offset/size stay era-true, so the two era
 * LAYOUT_ASSERTs below (NJS_OBJECT_size, NJS_OBJECT_child_offset) plus
 * NinjaFloatKeySize PASS HONESTLY under the port types.h rebind (u32 fixed
 * 4 bytes — this shadow REQUIRES that rebind; with the era
 * `typedef unsigned long u32` nothing here is era-shaped).
 *
 * Slots keep holding what the disc holds: the 4-byte PPC image addresses
 * that read_ninja_chunk (TLobby_801E2F94.cpp:334-430) patches via the POF0
 * pointer-offset table. Convert at access with NJS_HANDLE(T, slot) /
 * pso_njs_g2h below; never re-spell a slot as a host pointer in the struct
 * (obligation 1). */
#ifndef PSO_SYSTEM_NINJATYPES_H
#define PSO_SYSTEM_NINJATYPES_H

#include <stddef.h>
#include "types.h"
#include <dolphin/mtx/GeoTypes.h>

/* ---- P2-AR: arena-base conversion (p1-datamodel §3 rule 1; the bringup
 * crt0 arena is THE base register: docs/p2-bringup-1.md, pso_bringup.h
 * pso_g2h). Declared, not defined, so pure-parse builds never link. ---- */
#ifndef PSO_NJS_GUEST_RAM_BASE
#define PSO_NJS_GUEST_RAM_BASE 0x80000000u /* nominal PPC RAM start */
#endif
extern unsigned char *pso_arena_base; /* host/bringup/crt0.c owns this */

static inline void *pso_njs_g2h(u32 g) {
    return g ? (void *)(pso_arena_base + (g - PSO_NJS_GUEST_RAM_BASE)) : (void *)0;
}
static inline u32 pso_njs_h2g(const void *p) {
    return p ? (u32)(PSO_NJS_GUEST_RAM_BASE +
                     ((const unsigned char *)p - pso_arena_base))
             : 0u;
}
/* The one legal way to cross the seam: slot (a 4-byte image handle) -> live
 * host pointer. NULL slot -> NULL pointer, matching the era's sentinel use. */
#define NJS_HANDLE(T, slot) ((T *)(pso_njs_g2h(slot)))

/* P2-AR byte-truth asserts: private macro so the generated waiver dispatch's
 * fail-closed PSO_LA_FLAG_<name> lookup is not disturbed by new names.
 * Each pins an offset/size the era header had NO assert for — the shadow's
 * contract is that every re-typed slot stays 4 bytes on LP64. */
#define PSO_NJS_BYTE_TRUTH(name, cond) typedef char pso_njs_bt_##name[(cond) ? 1 : -1]

// Ninja texture list: `nbTexture` names, each naming one texture
// (NJS_TEXNAME is defined with Ginja's loaded-texture fields in GinjaMaterialState.h).
typedef struct NJS_TEXNAME NJS_TEXNAME;

typedef struct NJS_TEXLIST NJS_TEXLIST;
struct NJS_TEXLIST {
    u32 textures; // P2-AR: was NJS_TEXNAME * — NJTL image slot, POF0-patched
    u32 nbTexture;
};
PSO_NJS_BYTE_TRUTH(NJS_TEXLIST_size, sizeof(NJS_TEXLIST) == 0x8);
PSO_NJS_BYTE_TRUTH(NJS_TEXLIST_textures_offset, offsetof(NJS_TEXLIST, textures) == 0x0);
PSO_NJS_BYTE_TRUTH(NJS_TEXLIST_nbTexture_offset, offsetof(NJS_TEXLIST, nbTexture) == 0x4);
/* Era access idiom: NJS_HANDLE(NJS_TEXNAME, tl->textures) */

// One sprite cell of a texture: size, centre, uv rectangle and texture index.
typedef struct NJS_TEXANIM NJS_TEXANIM;
struct NJS_TEXANIM {
    s16 sx, sy;
    s16 cx, cy;
    s16 u1, v1, u2, v2;
    s16 texid, attr;
};

typedef Vec NJS_POINT3;

// A Ninja camera: eye position, view direction, roll, view angle (0x10000 a
// turn) and the near and far clip planes (negative view-space z).
typedef struct NJS_CAMERA NJS_CAMERA;
struct NJS_CAMERA {
    f32 px, py, pz;
    f32 vx, vy, vz;
    s32 roll;
    s32 ang;
    f32 n_clip;
    f32 f_clip;
};

// A 2D sprite: position, scale and angle, drawn from cells of `tanim`
// (texture ids into `tlist`).
// P2-AR: tlist/tanim stay host pointers — no sprite chunk tag exists in
// read_ninja_chunk (:350-392), so NJS_SPRITE is runtime-only like
// NJS_POINT3COL (AJ flip). Layout is NOT era-pinned and no assert fires.
typedef struct NJS_SPRITE NJS_SPRITE;
struct NJS_SPRITE {
    NJS_POINT3 p;
    f32 sx, sy;
    s32 ang;
    NJS_TEXLIST *tlist;
    NJS_TEXANIM *tanim;
};

typedef struct NJS_POINT2 NJS_POINT2;
struct NJS_POINT2 {
    f32 x, y;
};

typedef struct NJS_TEX NJS_TEX;
struct NJS_TEX {
    s16 u, v;
};

// `num` points with one colour each and optional texture coordinates.
// P2-AR: host-runtime-only per AJ's NJS_POINT3COL RUNTIME-WAIVE
// (docs/p2-njs-confirm.md) — fields stay REAL host pointers; its asserts are
// waived by the stitch battery, not flipped here.
typedef struct NJS_POINT3COL NJS_POINT3COL;
struct NJS_POINT3COL {
    NJS_POINT3 *p;
    u32 *col;
    NJS_TEX *tex;
    u32 num;
};

// A Ninja colour with float channels, alpha first.
typedef struct NJS_ARGB NJS_ARGB;
// Copies the four channels of `src` into `dst`.
void copy_argb(NJS_ARGB *dst, NJS_ARGB *src);

struct NJS_ARGB {
    f32 a;
    f32 r;
    f32 g;
    f32 b;
};

// NJS_OBJECT evalflags bits.
#define NJD_EVAL_UNIT_POS 0x1  // pos is zero: no translation
#define NJD_EVAL_UNIT_ANG 0x2  // ang is zero: no rotation
#define NJD_EVAL_UNIT_SCL 0x4  // scl is one: no scaling
#define NJD_EVAL_HIDE 0x8      // the node is not drawn
#define NJD_EVAL_BREAK 0x10    // the node's children are skipped
#define NJD_EVAL_ZXY_ANG 0x20  // ang is applied in Z, X, Y order
#define NJD_EVAL_SKIP 0x40     // the node takes no motion keys: its own pos, ang and scl are used
#define NJD_EVAL_QUATERNION 0x400  // ang (x, y, z) and quat_w are a rotation quaternion

// Chunk polygon list heads (Ninja ninjacnk.h). A chunk starts with a 16-bit
// [flags(8)|type(8)] head; the type ranges are bits, tiny, material (16-31), vertex (32-55),
// volume (56-63) and strip chunks.
#define NJD_BITSOFF 1    // bits chunks (16-bit, no size)
#define NJD_TINYOFF 8    // tiny chunks (32-bit)
#define NJD_MATOFF 16    // material chunks
#define NJD_VERTOFF 32   // vertex chunks
#define NJD_STRIPOFF 64  // strip chunks
#define NJD_ENDOFF 255   // end chunk

#define NJD_CE (NJD_ENDOFF + 0)     // end of the list
#define NJD_CB_BA (NJD_BITSOFF + 0) // blend alpha: bits 13-8 = src/dst alpha instructions
#define NJD_CB_CP (NJD_BITSOFF + 3) // cache polygon list: bits 15-8 = cache number
#define NJD_CB_DP (NJD_BITSOFF + 4) // draw polygon list: bits 15-8 = cache number
#define NJD_CV (NJD_VERTOFF + 2)       // vertices: position
#define NJD_CV_D8 (NJD_VERTOFF + 3)    // position, diffuse colour
#define NJD_CV_UF (NJD_VERTOFF + 4)    // position, user flags
#define NJD_CV_VN (NJD_VERTOFF + 9)    // position, normal
#define NJD_CV_VN_D8 (NJD_VERTOFF + 10) // position, normal, diffuse colour
#define NJD_CV_VN_UF (NJD_VERTOFF + 11) // position, normal, user flags
#define NJD_CV_VN_NF (NJD_VERTOFF + 12) // position, normal, ninja flags
#define NJD_CS (NJD_STRIPOFF + 0)     // strip
#define NJD_CS_UVN (NJD_STRIPOFF + 1) // strip with uv in 0-255
#define NJD_CS_UVH (NJD_STRIPOFF + 2) // strip with uv in 0-1023

// Strip chunk flags (the head's high byte).
#define NJD_FST_SHIFT 8
#define NJD_FST_IL (0x01 << NJD_FST_SHIFT)  // ignore light
#define NJD_FST_UA (0x08 << NJD_FST_SHIFT)  // use alpha
#define NJD_FST_ENV (0x40 << NJD_FST_SHIFT) // environment mapping

// Texture-id chunk flags (the head's high byte) and the id field of its data.
#define NJD_FFL_U 0x8000U  // flip U
#define NJD_FFL_V 0x4000U  // flip V
#define NJD_FCL_U 0x2000U  // clamp U
#define NJD_FCL_V 0x1000U  // clamp V
#define NJD_TID_MASK 0x1FFF

// Material attribute bits (texture addressing).
#define NJD_FLAG_FLIP_U 0x40000
#define NJD_FLAG_FLIP_V 0x20000
#define NJD_FLAG_CLAMP_U 0x10000
#define NJD_FLAG_CLAMP_V 0x8000

// Texture attribute: the texture is in memory (not a file).
#define NJD_TEXATTR_TYPE_MEMORY 0x40000000

// Chunk model streams referenced by an NJS_OBJECT.
// P2-AR: vertex_list/polygon_list are inside verbatim-copied NJCM/GJCM chunk
// images (the polygon-list heads below walk them as byte streams) -> u32 image
// slots, POF0-patched like NJS_OBJECT.model.
typedef struct NJS_CNK_MODEL NJS_CNK_MODEL;
struct NJS_CNK_MODEL {
    u32 vertex_list;     // P2-AR: was void *
    u32 polygon_list;    // P2-AR: was s16 *
    f32 center[3];
    f32 r;  // bounding radius around center
};
PSO_NJS_BYTE_TRUTH(NJS_CNK_MODEL_size, sizeof(NJS_CNK_MODEL) == 0x18);
PSO_NJS_BYTE_TRUTH(NJS_CNK_MODEL_vertex_list_offset, offsetof(NJS_CNK_MODEL, vertex_list) == 0x0);
PSO_NJS_BYTE_TRUTH(NJS_CNK_MODEL_polygon_list_offset, offsetof(NJS_CNK_MODEL, polygon_list) == 0x4);
PSO_NJS_BYTE_TRUTH(NJS_CNK_MODEL_center_offset, offsetof(NJS_CNK_MODEL, center) == 0x8);
PSO_NJS_BYTE_TRUTH(NJS_CNK_MODEL_r_offset, offsetof(NJS_CNK_MODEL, r) == 0x14);

// Ninja model hierarchy node.
typedef struct NJS_OBJECT NJS_OBJECT;
struct NJS_OBJECT {
    u32 evalflags;
    u32 model;           // P2-AR: was void * — NJCM image handle (POF0 slot)
    f32 pos[3];
    s32 ang[3];
    f32 scl[3];
    u32 child;           // P2-AR: was NJS_OBJECT * — image-tree handle
    u32 sibling;         // P2-AR: was NJS_OBJECT * — image-tree handle
    f32 quat_w; // with evalflags 0x400 the rotation is a quaternion: ang holds x, y, z as floats, this w (name inferred)
};
/* Era asserts below are KEEP-flagged in build/waivers/pso_assert_waivers.h —
 * under this shadow + the port types.h rebind they PASS with era bytes. */
LAYOUT_ASSERT(NJS_OBJECT_size, sizeof(NJS_OBJECT) == 0x38);
LAYOUT_ASSERT(NJS_OBJECT_child_offset, offsetof(NJS_OBJECT, child) == 0x2C);
PSO_NJS_BYTE_TRUTH(NJS_OBJECT_evalflags_offset, offsetof(NJS_OBJECT, evalflags) == 0x00);
PSO_NJS_BYTE_TRUTH(NJS_OBJECT_model_offset, offsetof(NJS_OBJECT, model) == 0x04);
PSO_NJS_BYTE_TRUTH(NJS_OBJECT_pos_offset, offsetof(NJS_OBJECT, pos) == 0x08);
PSO_NJS_BYTE_TRUTH(NJS_OBJECT_ang_offset, offsetof(NJS_OBJECT, ang) == 0x14);
PSO_NJS_BYTE_TRUTH(NJS_OBJECT_scl_offset, offsetof(NJS_OBJECT, scl) == 0x20);
PSO_NJS_BYTE_TRUTH(NJS_OBJECT_sibling_offset, offsetof(NJS_OBJECT, sibling) == 0x30);
PSO_NJS_BYTE_TRUTH(NJS_OBJECT_quat_w_offset, offsetof(NJS_OBJECT, quat_w) == 0x34);
/* Era access idioms this replaces, per TU: obj->child -> NJS_HANDLE(NJS_OBJECT, obj->child);
 * (NJS_OBJECT *)0x0 / PPC-window tests (TChCrLoadReq.cpp:34-35) -> compare slots against
 * pso_njs_h2g() of the arena window instead. */

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
void copy_model_node(NJS_OBJECT *dst, const NJS_OBJECT *src);
#endif

// Ninja byte colour; on this big-endian target the bytes of the ARGB word are a, r, g, b.
typedef struct NJS_BGRA NJS_BGRA;
struct NJS_BGRA {
    u8 a;
    u8 r;
    u8 g;
    u8 b;
};

// Ninja colour: one packed ARGB word or its bytes.
typedef union NJS_COLOR NJS_COLOR;
union NJS_COLOR {
    u32 color;
    NJS_BGRA argb;
};

// `num` 2D points with one colour each and optional texture coordinates.
// P2-AR: runtime-only descriptor family (same class as NJS_POINT3COL); stays
// host pointers, not era-pinned.
typedef struct NJS_POINT2COL NJS_POINT2COL;
struct NJS_POINT2COL {
    NJS_POINT2 *p;
    NJS_COLOR *col;
    NJS_TEX *tex;
    u32 num;
};

// Ninja motion: per-node key data played over nbFrame frames.
// P2-AR: mdata is the NDMD/GMDM image handle (AJ: AnimationMotion ->
// NJS_MOTION::mdata -> NJS_MDATA2::p[0] into the same copied chunk).
typedef struct NJS_MOTION NJS_MOTION;
struct NJS_MOTION {
    u32 mdata;   // P2-AR: was void *
    u32 nbFrame;
    u16 type;
    u16 inp_fn;
};
PSO_NJS_BYTE_TRUTH(NJS_MOTION_size, sizeof(NJS_MOTION) == 0xC);
PSO_NJS_BYTE_TRUTH(NJS_MOTION_mdata_offset, offsetof(NJS_MOTION, mdata) == 0x0);
PSO_NJS_BYTE_TRUTH(NJS_MOTION_nbFrame_offset, offsetof(NJS_MOTION, nbFrame) == 0x4);
PSO_NJS_BYTE_TRUTH(NJS_MOTION_type_offset, offsetof(NJS_MOTION, type) == 0x8);
PSO_NJS_BYTE_TRUTH(NJS_MOTION_inp_fn_offset, offsetof(NJS_MOTION, inp_fn) == 0xA);

// NJS_MOTION::type bits: the channels each node's row of keys holds, in this order.
#define NJD_MTYPE_POS_0 0x1
#define NJD_MTYPE_ANG_1 0x2
#define NJD_MTYPE_SCL_2 0x4
#define NJD_MTYPE_VEC_3 0x8
#define NJD_MTYPE_VERT_4 0x10
#define NJD_MTYPE_NORM_5 0x20
#define NJD_MTYPE_TARGET_3 0x40
#define NJD_MTYPE_ROLL_6 0x80
#define NJD_MTYPE_ANGLE_7 0x100
#define NJD_MTYPE_QUAT_1 0x2000

// Two motions blended: motion[0] at frame[0] into motion[1] at frame[1].
// P2-AR: runtime-only blend state (never image-resident); stays pointers.
typedef struct NJS_MOTION_LINK NJS_MOTION_LINK;
struct NJS_MOTION_LINK {
    NJS_MOTION *motion[2];
    f32 frame[2];
};

// NJS_MOTION::mdata for a motion with position and angle keys per node.
// P2-AR: p[] are the key-array image handles inside the verbatim NDMD copy
// (TBoss1Dragon.cpp:574,:595 walks them; keys are mutated in place over the
// image — the arena base is what makes that legal on LP64).
typedef struct NJS_MDATA2 NJS_MDATA2;
struct NJS_MDATA2 {
    u32 p[2];  // P2-AR: was void * — position keys, angle keys (image handles)
    u32 nb[2]; // their counts
};
PSO_NJS_BYTE_TRUTH(NJS_MDATA2_size, sizeof(NJS_MDATA2) == 0x10);
PSO_NJS_BYTE_TRUTH(NJS_MDATA2_nb_offset, offsetof(NJS_MDATA2, nb) == 0x8);

// A float key: three values at a frame.
// P2-AR: NO pointer member — era layout is LP64-invariant under the port
// rebind (AJ, p2-njs-confirm.md); nothing re-typed, the lock stays honest.
typedef struct NJS_MKEY_F NJS_MKEY_F;
struct NJS_MKEY_F {
    u32 keyframe;
    f32 key[3];
};

LAYOUT_ASSERT(NinjaFloatKeySize, sizeof(NJS_MKEY_F) == 0x10);
PSO_NJS_BYTE_TRUTH(NJS_MKEY_F_key_offset, offsetof(NJS_MKEY_F, key) == 0x4);


// An angle key: three angles at a frame.
typedef struct NJS_MKEY_A NJS_MKEY_A;
struct NJS_MKEY_A {
    u32 keyframe;
    s32 key[3];
};

// A short angle key: three 16-bit angles at a 16-bit frame.
typedef struct NJS_MKEY_SA NJS_MKEY_SA;
struct NJS_MKEY_SA {
    u16 keyframe;
    s16 key[3];
};

// One angle at a frame.
typedef struct NJS_MKEY_A1 NJS_MKEY_A1;
struct NJS_MKEY_A1 {
    u32 keyframe;
    s32 key;
};

// A shape key: a whole vertex (or normal) array at a frame.
// P2-AR: VERT/NORM keys live in the same verbatim NDMD copy as MKEY_F rows
// (NJD_MTYPE_VERT_4/NORM_5) -> image handle.
typedef struct NJS_MKEY_P NJS_MKEY_P;
struct NJS_MKEY_P {
    u32 keyframe;
    u32 key;  // P2-AR: was f32 * — shape-key image handle
};
PSO_NJS_BYTE_TRUTH(NJS_MKEY_P_size, sizeof(NJS_MKEY_P) == 0x8);
PSO_NJS_BYTE_TRUTH(NJS_MKEY_P_key_offset, offsetof(NJS_MKEY_P, key) == 0x4);

// A rotation quaternion at a frame, real part first.
typedef struct NJS_MKEY_QUAT NJS_MKEY_QUAT;
struct NJS_MKEY_QUAT {
    u32 keyframe;
    f32 key[4];
};

#endif
