/* P2-AU upload seam — the era wrapper at 0x803DA12C composed for a host
 * register ABI that has no argument pass-through.
 *
 * DEVIATION AU-1 (the only hand-written fn in this rung; docs/p2-tex-upload.md
 * §2). The era definition text_803DA12C.cpp:9-15 is `void
 * init_texture_header_and_alloc(void *p)` and its body
 *   switch (init_ginja_texture_header(p)) { case 0: alloc_buffer_from_heap(p); break; }
 * depends on the PPC ABI to reach the 4-arg sizer: init_ginja_texture_header
 * is forward-declared 1-arg THERE, but the .s proves the caller's r4/r5/r6
 * (width/height/format) flow through the untouched wrapper
 * (0x803DA13C `mr r31,r3` is the only arg touch; bl init at :803DA140) —
 * docs/p2-an4-triage.md §1. On arm64 a 1-arg C call cannot forward ghost
 * registers; executing the literal body would feed the real 4-arg
 * init_ginja_texture_header garbage. So the port implements what the triage
 * §4 prescribes — "the era 4-arg behavior is wrapper ⊕ tail-call" — in the
 * ONLY 4-arg shape the era itself ever used for this name: the caller's
 * extern "C" forward decl text_803D96A8.cpp:23 `s32 init_texture_header_and_alloc(u8*, s32, s32, s32)`.
 * Control flow AND the r3 return flow are era-verbatim:
 *   0x803DA144 cmpwi r3,0 / :803DA148 beq → alloc(:803DA154, r3 = alloc's ret)
 *                \ else fall to :803DA158 return r3 = init's ret
 * switch/case-0/break below reproduces exactly that, nothing more: on ret!=0
 * the wrapper does NOTHING else (no size-reset, no pixel write — golden F2
 * pins header filled + size computed + pixels untouched).
 *
 * locked_cache_heap_handle: era def site RenderSbssGlobals.cpp:12 (.sbss);
 * zero-init here == era bss zero. The era set it during render-system init;
 * this port's writer is the ladder5 test (heap carved over the Y arena —
 * deviation AU-4, host stand-in for the locked-cache region).
 */
#include "ladder5_tex_prelude.h"

u32 locked_cache_heap_handle;

s32 init_texture_header_and_alloc(u8 *p, s32 width, s32 height, s32 format) {
    GinjaTextureHeader *header = (GinjaTextureHeader *)p;
    s32 ret = init_ginja_texture_header(header, (u32)width, (u32)height, (u32)format);
    switch (ret) {
    case 0:
        ret = alloc_buffer_from_heap(header);
        break;
    }
    return ret;
}
