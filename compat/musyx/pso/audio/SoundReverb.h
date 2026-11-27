/* P2-MX PORT-SIDE SHADOW of include/pso/audio/SoundReverb.h:11,12,32 (decomp @6b38b5383, read-only).
 * Era-truth edits ONLY (line-cited in docs/p2-musyx-1.md): linkage guards +
 * elaborated struct tags. Zero layout reinterpretation. Active only when
 * -I compat/musyx precedes -I include/-I src on the C(-side) include path. */
#ifndef PSO_SOUND_REVERB_H
#define PSO_SOUND_REVERB_H
#include "types.h"

struct SoundReverbDelay {
    s32 writeOffset, readOffset, byteLength;
    f32* samples;
    f32 last;
};
struct SoundReverb {
    struct SoundReverbDelay allPass[9];
    struct SoundReverbDelay comb[9];
    f32 coloration;
    f32 feedback[9];
    f32 lowPassLast[3];
    f32 mix;
    f32 damping;
    s32 preDelayFrames;
    f32 crossTalk;
    f32* preDelay[3];
    f32* preDelayCurrent[3];
    u8 disabled;
    f32 parameterColoration;
    f32 parameterMix;
    f32 parameterTime;
    f32 parameterDamping;
    f32 parameterPreDelay;
    f32 parameterCrossTalk;
};
LAYOUT_ASSERT(SoundReverbDelaySize, sizeof(struct SoundReverbDelay) == 0x14);
LAYOUT_ASSERT(SoundReverbSize, sizeof(struct SoundReverb) == 0x1E0);
#ifdef __cplusplus  /* P2-MX shadow: guarded linkage + C-legal named decl */
extern "C" s32 ReverbHICreate(struct SoundReverb*, f32 coloration, f32 time,
    f32 mix, f32 damping, f32 preDelay, f32 crossTalk);
#else
s32 ReverbHICreate(struct SoundReverb *rv, f32 coloration, f32 time,
    f32 mix, f32 damping, f32 preDelay, f32 crossTalk);
#endif
#endif
