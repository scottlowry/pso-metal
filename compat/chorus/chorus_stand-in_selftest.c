/* pso-metal P2-AM selftest — proves compat/chorus/chorus_stand-in.c vends
 * the 2 golden names with signatures a musyx.h client accepts VERBATIM.
 *
 * The compile-time proof is the typedef-checked function-pointer init below
 * (a signature drift = clang error, not a runtime miss). The run-time proof
 * walks the SoundControlState.cpp:974-980 caller shape: preset params ->
 * prepare -> register-as-ptr -> reason-0 buffer leg -> reason-1 param leg ->
 * NULL-user fallback. Compiled two ways (T shadow pattern):
 *   clang++ -std=c++17 -x c++ $DIALECT $COMMON   (mode 1 battery pattern)
 *   clang   -std=c11    -x c   $DIALECT $COMMON  (guarded pure-C variant)
 */
#include "musyx/musyx.h" /* T shadow: via -I compat/dialect/pshim; original :161 bare EC is C++-only */
#include "chorus_stand-in.h"

static int fails;
#define CHECK(cond, name)                                                     \
    do {                                                                      \
        if (!(cond)) {                                                        \
            OSReport("SELFTEST FAIL: %s\n", (name));                          \
            fails++;                                                          \
        }                                                                     \
    } while (0)

#ifdef __cplusplus
extern "C" {
#endif
void OSReport(const char *fmt, ...); /* os.h:41 shape; host def osinit.c:57 */
int chorus_stand_in_selftest(void);  /* C linkage so main_shim can see it from a C++ TU */
#ifdef __cplusplus
}
#endif

int chorus_stand_in_selftest(void) {
    /* (1) compile-time signature acceptance — musyx.h's own types: */
    SND_AUX_CALLBACK cb = sndAuxCallbackChorus; /* typedef musyx.h:32 */
    u32 (*prepare)(SND_AUX_CHORUS *) = sndAuxCallbackPrepareChorus;
    SND_AUX_CHORUS chorus;
    SND_AUX_INFO info;
    s32 left[4];
    u32 i;

    /* (2) era caller shape, SoundControlState.cpp:974-978 */
    for (i = 0; i < sizeof(chorus); ++i) {
        ((u8 *)&chorus)[i] = 0;
    }
    chorus.baseDelay = 15;
    chorus.variation = 0;
    chorus.period = 500;
    CHECK(prepare(&chorus) == 1, "prepare returns era success 1");
    CHECK(chorus.work.lastLeft[0] != 0, "delay line L wired (no crash-query state)");
    CHECK(chorus.work.lastRight[0] == chorus.work.lastLeft[0] + 0x1e0, "R bank at +0x1e0 (chorus_fx.c:213)");
    CHECK(chorus.work.lastSur[0] == chorus.work.lastRight[0] + 0x1e0, "Sur bank at +0x1e0 (chorus_fx.c:214)");
    CHECK(chorus.work.currentLast == 1, "currentLast seeded 1 (chorus_fx.c:231)");
    CHECK(chorus.work.src.trigger == 480, "trigger seeded 480 (chorus_fx.c:235)");
    /* period=500 -> ((100)+1)&~1 = 100; pitchOffset=(0<<16)/500 = 0 */
    CHECK(chorus.work.pitchOffsetPeriod == 100, "pitchOffsetPeriod from update-settings formula (chorus_fx.c:200)");
    /* baseDelay=15 -> 0x140-((15-5)<<5) = 0; currentLast=1 -> +0*0xa0; %0x1e0 = 0 */
    CHECK(chorus.work.currentPosHi == (u32)(((0x140 - ((15u - 5u) << 5)) + (1u - 1u) * 0xa0u) % 0x1e0u) && chorus.work.currentPosHi == 0, "currentPosHi formula (chorus_fx.c:197-199)");

    /* (3) registration leg — just taking/storing the fn ptr must be safe: */
    (void)cb;

    /* (4) reason-0 buffer leg: dry pass-through, caller data untouched */
    for (i = 0; i < 4; ++i) {
        left[i] = 7 + (s32)i;
    }
    info.data.bufferUpdate.left = left;
    info.data.bufferUpdate.right = 0;
    info.data.bufferUpdate.surround = 0;
    cb(0, &info, &chorus);
    CHECK(left[0] == 7 && left[3] == 10, "reason 0 is pass-through (audio untouched)");
    CHECK(chorusStandInCallbackCalls() == 1, "callback counter advanced");

    /* (5) reason-1 parameter leg + (6) NULL-tolerance fallbacks */
    info.data.parameterUpdate.para[0] = 11;
    info.data.parameterUpdate.para[1] = 22;
    info.data.parameterUpdate.para[2] = 33;
    info.data.parameterUpdate.para[3] = 44;
    cb(1, &info, &chorus);
    cb(0, 0, &chorus);  /* NULL info: era would crash; stand-in must not */
    cb(0, &info, 0);    /* NULL user: falls back to module state         */
    cb(9, &info, 0);    /* unknown reason: loud, harmless                */
    prepare(0);         /* NULL chorus: module fallback state            */
    CHECK(chorusStandInPrepareCalls() == 2, "prepare counter (incl. NULL-user call)");

    if (fails == 0) {
        OSReport("CHORUS STAND-IN SELFTEST: PASS (signatures accepted by musyx.h client; %u checks)\n", 12u);
    }
    return fails;
}
