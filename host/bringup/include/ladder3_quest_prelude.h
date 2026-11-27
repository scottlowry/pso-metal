/* ladder3 quest-glue prelude — prepended to the VERBATIM
 * qst_op_call_image_data slice (QuestOpcodeHandlers.cpp:2278-2286, home
 * 0x801EB5AC). The era cross-unit globals it touches (defs at :173-174,
 * qst_args via QuestVm.h:82) are instantiated HERE; the era label resolver
 * body (QuestOpcodeHandlers.s 0x801F2C1C.., def lives in a different
 * cross-TU fragment) is reproduced instruction-for-instruction as
 * qst_label_address_era_body — deviation DA-2 in docs/p2-bringup-3.md:
 *   801F2C1C  lwz r4, qst_function_table   -> tbl = qst_function_table
 *   801F2C20  clrlslwi r0,r3,16,2          -> (u16)label * 4  (s32 index)
 *   801F2C24  lwz r3, qst_code             -> base = qst_code
 *   801F2C28  lwzx r0,r4,r0                -> off = tbl[label*4/4]
 *   801F2C2C  add  r3,r3,r0                -> base + off
 *   blr   (s32 arithmetic, era PPC int==pointer)
 */
#include "types.h"
#include "ladder3_harness.h"
#include "pso/system/prs.h"                /* prs_decompress_buffer decl (0x800F7DAC) */

/* QuestOpcodeHandlers.cpp:173-174 (era .s sbss 0x805DB870/0x805DB874): */
u8  *qst_code;
s32 *qst_function_table;

/* QuestVm.h:82 extern QuestValue qst_args[8]; — union stride proven by
 * 0x801EB5C4 `lwz r0,0x4(r3)` (args[1]) / 0x801EB5C8 `lwz r31,0x0(r3)`
 * (args[0]) off one qst_args base. Era def lives in a different unit. */
QuestValue qst_args[8];

/* Forward-decl the harness resolver BEFORE the slice: the slice's
 * `((void *(*)(void *))qst_label_address)` reinterpretation (vs the stale
 * `int qst_label_address(int);` at QuestOpcodeHandlers.cpp:166) is exactly
 * what 0x801EB5D0 `bl qst_label_address__Fi` + the caller-side cast encode
 * on PPC; on the LP64 host the real def returns void* (deviation DA-2). */
void *qst_label_address(void *label_ptr);

/* TObjSinBoard.cpp:111-121 continues into TQuestVmDownLoadServer::alloc ->
 * GinjaTextureInput::set_render_rect/set_texture_name/load_texture_if_valid
 * (texture-table + GX upload). This rung records the era call-site args
 * instead — deviation DA-3 in docs/p2-bringup-3.md; the .s call site:
 *   0x801EB5E4 lwz r3, resource_decompress_buffer@sda21
 *   0x801EB5E8 mr  r4, r31
 *   0x801EB5EC bl load_sin_board_image__FPCvUl                            */
void *recorder_image_ptr;
u32   recorder_image_size;
int   recorder_image_calls;
void load_sin_board_image(const void *image, u32 size) {
    recorder_image_ptr = (void *)image;
    recorder_image_size = size;
    recorder_image_calls++;
}

/* SystemBootInit.cpp:86 definition (the REAL buffer; :283 allocs
 * xmalloc(0x80000), :147 xfrees it). */
u8 *resource_decompress_buffer;
