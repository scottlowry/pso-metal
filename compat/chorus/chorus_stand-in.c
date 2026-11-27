/* pso-metal P2-AM — port STAND-IN for the 2 chorus golden names.
 *
 * WHY (docs/p2-link-hygiene.md §4): the audio path defined by
 * src/pso/audio/SoundControlState.cpp:978-979 references
 * sndAuxCallbackPrepareChorus + sndAuxCallbackChorus, whose real bodies live
 * in decomp src/musyx/runtime/Chorus/chorus_fx.c (:205, :123). That TU has
 * NO host-compilable path at pso-decomp @ 6b38b5383: :113/:117 are MWCC
 * `asm void` + include of orig_asm/musyx/runtime/Chorus/chorus_fx/
 * do_src{1,2}.inc (:114/:118) holding nofralloc/opword blobs — clang
 * rejects both the missing-include route and the GPOE8P content route
 * (probes + error classes: docs/p2-chorus-standin.md §1).
 *
 * WHAT THIS IS: NOT the chorus. Both bodies fail LOUD via the bringup
 * OSReport channel (declared decomp libs/dolphin/include/dolphin/os.h:41,
 * defined host/bringup/osinit.c:57, wired to printf there) saying
 * UNIMPLEMENTED(chorus), then maintain era-SHAPED static state so that
 * register-only callers (sndSetAuxProcessingCallbacks with these fn ptrs,
 * SoundControlState.cpp:979-980) and query-only callers (the work fields on
 * their own SND_AUX_CHORUS) do not crash. No audio is processed: the
 * reason-0 buffer leg is dry pass-through (output buffers untouched).
 *
 * HOW TO REMOVE: when chorus_fx is translated port-side, delete this .c/.h
 * from the link battery — the real T defs then come from the chorus_fx.o
 * rule; gate (c) dup-T warning catches any half-migration. Checklist:
 * docs/p2-chorus-standin.md §4.
 *
 * Dialect: compile via the P2-T pattern (DIALECT + COMMON of
 * docs/p2-link-hygiene.md §2); the pshim shadow musyx.h makes this TU legal
 * in both -x c and -x c++ (docs/p2-chorus-standin.md §2).
 */
#include "chorus_stand-in.h" /* dir-relative; musyx.h arrives through it */

#ifdef __cplusplus
extern "C" {
#endif
/* Era report channel. Identical prototype to libs/dolphin/include/dolphin/
 * os.h:41, so a later real include merges cleanly; hosted definition is
 * host/bringup/osinit.c:57 (OSReport -> vprintf). */
void OSReport(const char *fmt, ...);
#ifdef __cplusplus
}
#endif

/* ---- era-shaped static state ------------------------------------------------
 * The real prepare salMalloc()s one 0x1680-byte block per chorus and carves
 * 3 x 0x1e0 s32 slots (L/R/Sur) plus 3 rotation banks at 0xa0 stride
 * (chorus_fx.c:211-220). The stand-in backs that with module statics so the
 * work pointers handed to callers are non-NULL, 4-byte aligned, and
 * persistent across calls — no allocator dependency, no free path. */
#define CHORUS_STANDIN_SLOT_COUNT 0x1e0 /* chorus_fx.c:213-214 slot stride  */
#define CHORUS_STANDIN_BANK_COUNT 3     /* chorus_fx.c:216 loop bound       */
#define CHORUS_STANDIN_BANK_STRIDE 0xa0 /* chorus_fx.c:217-219 stride       */

static s32 sDelayLine[3 * CHORUS_STANDIN_SLOT_COUNT]; /* == 0x1680 bytes, chorus_fx.c:211 */
static SND_AUX_CHORUS sFallbackChorus;                /* user==0 defensive path           */
static u16 sLastPara[4];                              /* reason-1 parameterUpdate record  */

/* Stand-in bookkeeping (port-side observability; not era state). */
static u32 sPrepareCalls;
static u32 sCallbackCalls;
static int sLoudChorus;   /* one-shot latches: the callback runs per audio    */
static int sLoudPrepare;  /* buffer, so "fail loud" must not mean "spam".     */
static int sLoudBadReason;

/* ---- fail-loud report ------------------------------------------------------ */
static void chorusStandInReportOnce(int *latch, const char *fn) {
    if (!*latch) {
        *latch = 1;
        OSReport("UNIMPLEMENTED(chorus): %s — compat/chorus/chorus_stand-in.c "
                 "stand-in, real body is chorus_fx.c (docs/p2-chorus-standin.md)\n", fn);
    }
}

/* ---- golden #2: u32 sndAuxCallbackPrepareChorus(SND_AUX_CHORUS *) ----------
 * Signature VERBATIM from include/musyx/musyx.h:167. Era body allocates the
 * delay line, wires L/R/Sur banks, zeros 0x140 slots, seeds work state and
 * returns sndAuxCallbackUpdateSettingsChorus()==1 (chorus_fx.c:205-239).
 * The stand-in mirrors the state shape over static memory (so callers that
 * only prepare+register, or read back work fields, see a prepared chorus)
 * and returns the era success value 1 AFTER the loud report. */
u32 sndAuxCallbackPrepareChorus(SND_AUX_CHORUS *chorus) {
    u32 i;
    chorusStandInReportOnce(&sLoudPrepare, "sndAuxCallbackPrepareChorus");
    sPrepareCalls++;

    if (chorus == 0) {
        chorus = &sFallbackChorus; /* era would deref caller struct (chorus_fx.c:211) */
    }

    /* Delay-line wiring, chorus_fx.c:211-220 (salMalloc replaced by statics). */
    chorus->work.lastLeft[0] = sDelayLine;
    chorus->work.lastRight[0] = chorus->work.lastLeft[0] + CHORUS_STANDIN_SLOT_COUNT;
    chorus->work.lastSur[0] = chorus->work.lastRight[0] + CHORUS_STANDIN_SLOT_COUNT;
    for (i = 1; i < CHORUS_STANDIN_BANK_COUNT; ++i) {
        chorus->work.lastLeft[i] = chorus->work.lastLeft[0] + i * CHORUS_STANDIN_BANK_STRIDE;
        chorus->work.lastRight[i] = chorus->work.lastRight[0] + i * CHORUS_STANDIN_BANK_STRIDE;
        chorus->work.lastSur[i] = chorus->work.lastSur[0] + i * CHORUS_STANDIN_BANK_STRIDE;
    }

    /* Era zeros the first 0x140 slots per channel (chorus_fx.c:222-229); the
     * stand-in zeroes the whole static block (superset, same start state). */
    for (i = 0; i < 3 * CHORUS_STANDIN_SLOT_COUNT; ++i) {
        sDelayLine[i] = 0;
    }

    /* Seed values, chorus_fx.c:231-236 verbatim. */
    chorus->work.currentLast = 1;
    chorus->work.oldLeft[0] = chorus->work.oldLeft[1] = chorus->work.oldLeft[2] = chorus->work.oldLeft[3] = 0;
    chorus->work.oldRight[0] = chorus->work.oldRight[1] = chorus->work.oldRight[2] = chorus->work.oldRight[3] = 0;
    chorus->work.oldSur[0] = chorus->work.oldSur[1] = chorus->work.oldSur[2] = chorus->work.oldSur[3] = 0;
    chorus->work.src.trigger = 480;
    chorus->work.src.target = 0;

    /* sndAuxCallbackUpdateSettingsChorus body, chorus_fx.c:197-203, with the
     * era caller contract that baseDelay/variation/period are preset (see
     * SoundControlState.cpp:974-976: 15/0/500). Deviation D1: era divides by
     * pitchOffsetPeriod*5 unguarded; period<=4 yields 0 -> PPC/ARM trap. The
     * stand-in substitutes pitchOffset 0 (silence) instead of dividing. */
    chorus->work.currentPosHi = 0x140 - ((chorus->baseDelay - 5) << 5);
    chorus->work.currentPosLo = 0;
    chorus->work.currentPosHi = (chorus->work.currentPosHi + (chorus->work.currentLast - 1) * 0xa0) % 0x1e0;
    chorus->work.pitchOffsetPeriod = ((chorus->period / 5) + 1) & ~1;
    chorus->work.pitchOffsetPeriodCount = (u32)chorus->work.pitchOffsetPeriod >> 1;
    if (chorus->work.pitchOffsetPeriod * 5 != 0) {
        chorus->work.pitchOffset = (chorus->variation << 16) / (chorus->work.pitchOffsetPeriod * 5);
    } else {
        chorus->work.pitchOffset = 0; /* D1 fail-safe, era had no guard */
    }
    return 1; /* == era return at chorus_fx.c:203/237 */
}

/* ---- golden #1: void sndAuxCallbackChorus(u8, SND_AUX_INFO *, void *) ------
 * Signature VERBATIM from include/musyx/musyx.h:166; matches the
 * SND_AUX_CALLBACK typedef (musyx.h:32) so registration at
 * SoundControlState.cpp:979-980 type-checks. Era body mixes the chorus into
 * info->data.bufferUpdate buffers (reason 0) or updates parameters
 * (reason 1) — chorus_fx.c:133-189. The stand-in keeps the reason dispatch
 * shape, touches NO audio buffers (dry pass-through), records reason-1
 * parameters into static state, and is NULL-tolerant on info/user. */
void sndAuxCallbackChorus(u8 reason, SND_AUX_INFO *info, void *user) {
    SND_AUX_CHORUS *c = (SND_AUX_CHORUS *)user; /* cast shape, chorus_fx.c:124 */
    chorusStandInReportOnce(&sLoudChorus, "sndAuxCallbackChorus");

    if (c == 0) {
        c = &sFallbackChorus; /* era derefs user unguarded */
    }

    switch (reason) {
    case 0: /* buffer update leg, chorus_fx.c:134 — stand-in = pass-through */
        if (info == 0 || c->work.lastLeft[0] == 0) {
            return; /* unprepared: era would read NULL delay line; we do nothing */
        }
        sCallbackCalls++;
        break;
    case 1: /* parameter update leg, chorus_fx.c:186 — record, do not apply */
        if (info == 0) {
            return;
        }
        {
            u32 i;
            for (i = 0; i < 4; ++i) {
                sLastPara[i] = info->data.parameterUpdate.para[i]; /* recorded, never applied: stand-in has no filter state to retune */
            }
        }
        break;
    default: /* era switch has only 0/1 legs (chorus_fx.c:133-189) */
        chorusStandInReportOnce(&sLoudBadReason, "sndAuxCallbackChorus unexpected reason");
        OSReport("UNIMPLEMENTED(chorus): reason=%u ignored by stand-in\n", (u32)reason);
        break;
    }
}

/* ---- stand-in observability ------------------------------------------------ */
u32 chorusStandInPrepareCalls(void) { return sPrepareCalls; }
u32 chorusStandInCallbackCalls(void) { return sCallbackCalls; }

void chorusStandInReset(void) {
    u32 i;
    sPrepareCalls = 0;
    sCallbackCalls = 0;
    sLoudChorus = 0;
    sLoudPrepare = 0;
    sLoudBadReason = 0;
    sFallbackChorus.work.lastLeft[0] = 0;
    sFallbackChorus.work.lastRight[0] = 0;
    sFallbackChorus.work.lastSur[0] = 0;
    for (i = 0; i < 4; ++i) {
        sLastPara[i] = 0;
    }
}
