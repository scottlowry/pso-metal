/* pso-metal P2-AM — port STAND-IN for the 2 chorus golden names.
 *
 * The real definitions live in decomp src/musyx/runtime/Chorus/chorus_fx.c
 * (:123 sndAuxCallbackChorus, :205 sndAuxCallbackPrepareChorus), a TU with
 * NO host-compilable path at pso-decomp @ 6b38b5383 (MWCC `asm void` +
 * nofralloc/opword blobs in orig_asm/.../do_src{1,2}.inc — see
 * docs/p2-link-hygiene.md §4 and docs/p2-chorus-standin.md §1).
 *
 * This header vends the two golden symbols with signatures VERBATIM from
 * include/musyx/musyx.h:166-167 so callers compiled against musyx.h
 * (src/pso/audio/SoundControlState.cpp:978-979) accept them unchanged.
 *
 * Dialect note: includes musyx/musyx.h, which carries a bare `extern "C"`
 * block at :161 in the original. Compile with the P2-T dialect pattern
 * (-I compat/dialect/pshim ahead of -I include) so the shadow's
 * `#ifdef __cplusplus` guard makes this header legal in BOTH -x c and
 * -x c++ TUs, exactly like the other EC-block musyx TUs.
 */
#ifndef COMPAT_CHORUS_CHORUS_STAND_IN_H
#define COMPAT_CHORUS_CHORUS_STAND_IN_H

#include "musyx/musyx.h" /* golden type decls: SND_AUX_INFO :19-30, SND_AUX_CALLBACK :32, SND_AUX_CHORUS :149-154 */

#ifdef __cplusplus
extern "C" {
#endif

/* Signatures verbatim from include/musyx/musyx.h:166 and :167 (decomp
 * include/musyx/musyx.h @ 6b38b5383); definitions in chorus_stand-in.c. */
void sndAuxCallbackChorus(u8 reason, SND_AUX_INFO *info, void *user); /* musyx.h:166 */
u32 sndAuxCallbackPrepareChorus(SND_AUX_CHORUS *chorus);              /* musyx.h:167 */

/* Stand-in observability (port-side only — NOT golden, NOT era names):
 * counters so a bringup harness can prove registration/query do not crash
 * and can detect that the real chorus_fx landed (link would then collide —
 * gate (c) dup-T warning owns that, docs/p2-chorus-standin.md §4). */
u32 chorusStandInPrepareCalls(void);   /* times sndAuxCallbackPrepareChorus ran */
u32 chorusStandInCallbackCalls(void);  /* times the reason-0 buffer leg ran */
void chorusStandInReset(void);         /* zero counters + module fallback state */

#ifdef __cplusplus
}
#endif

#endif /* COMPAT_CHORUS_CHORUS_STAND_IN_H */
