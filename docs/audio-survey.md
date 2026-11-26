# PSO-Metal Phase-0 Survey E: Audio Stack Decision

Date: 2026-11-26 · Sources: read-only against `/Users/scott/Hermes/pso-decomp/pso-decomp` (GPOE8P).
All counts measured with `wc`/`grep` on the decomp tree; inferences are labeled **[INFERENCE]**.

## TL;DR

- **ADX decode is pure software in-tree.** `src/pso/audio/AdxDecode4.cpp` is literally "CRI adx_dcd5.c:
  the 4-bit ADX block decoders" — plain C ADPCM expansion, no DSP calls. An ADX *encoder* is also
  decompiled (`AdxEncoder.cpp`).
- **But no sound reaches the speaker from the CPU.** 100% of audio output is produced by the MusyX
  DSP microcode, which is present in-tree only as an opaque 6,624-byte binary blob
  (`dspSlave[0x19E0]` in `src/musyx/runtime/dsp_import.c`). The decompiled CPU code is the
  *control half* of MusyX; the synthesis half is DSP machine code.
- **Recommendation: Option B** (native backend behind a small `snd*`/GCRNA-compatible shim, reusing
  the in-tree pure-C ADX decoders), *not* full-stack port. Detail in §5.

## 1. Inventory — what audio code exists in-tree

| Module | Path | Size | Status |
|---|---|---|---|
| CRI ADX/middleware (game layer) | `src/pso/audio/` | 42 .cpp, 9,215 LOC, ~360 KB | mostly matched C++; 509 funcs still asm |
| MusyX runtime (CPU control half) | `src/musyx/runtime/` | 28 .c + 2 .h, 13,409 LOC, ~520 KB | core matched; 333 funcs still asm |
| MusyX DSP microcode | `src/musyx/runtime/dsp_import.c` | 6,624 bytes of data | **binary blob, not decompilable-as-useful-C** |
| Dolphin OS AI/AR/ARQ/DSP drivers | `build/GPOE8P/asm/{ai,ar,dsp}/` | ~44 `.fn` blocks of live PPC asm | NOT decompiled (SDK reference source at `libs/dolphin/src/`, MMIO-bound) |
| Chorus / StdReverb FX | `src/musyx/runtime/{Chorus,StdReverb}/` | 3 .c | matched (CPU-side float delay-line models) |

Totals measured: **991 matched C/C++ function definitions** in the audio stack; **842 functions
remain as assembly** (509 CRI-layer + 333 MusyX) — i.e. the middleware is roughly 55% decompiled by
function count, including the entire decoder/streaming core.

Layer map (verified from `#include`s and call sites):

```
game code (weapon/player/lobby .cpp)
  └─ SoundRequestVoice.cpp / AdxUiBridge.cpp        (voice alloc, spatial params, sndFXCtrl/KeyOff)
       ├─ CRI stack: Adxt*/AdxManager (threads), AdxStream/AdxBsc (block decode),
       │   AdxDecode4/AdxExpand (pure-C ADX→PCM), AdxTalk (speech), AdxAiff/AdxAu/Aix* (FMV/containers),
       │   Lsc* + Sj* + AdxFs* (streaming file servers over AFS/ARK)
       ├─ Gcrna* bridge: decoded streams → RNA/ARAM streams   ("GCRNA Ver.0.97 Build:Jun 13 2002")
       └─ MusyX CPU: snd_init/seq/synth/synthvoice/stream/snd3d/hw_* 
            └─ HW: hw_dolphin.c → AIInit/AIInitDMA/DSPAddTask/DSPSendMail + ARAM (hw_aramdma.c)
```

Hardware-touching files (MMIO/DMA) are exactly: `src/musyx/runtime/hardware.c`, `hw_dolphin.c`,
`hw_dspctrl.c`, `hw_aramdma.c`, `dsp_import.c`, plus the thin `dolphin` asm drivers. Everything else
in `src/pso/audio` and `src/musyx/runtime` is portable computation. [INFERENCE: verified by grep for
`AI_|AR_|ARAM|DSP_` symbols across `src/` — the only hits outside those files are GBA-ARAM upload
modules (`AramPool`, `GbaAramCache`, `QuestDownloadAramFile`), which are a *non-audio* second user
of ARAM.]

## 2. The DSP / AI / AR boundary (what must be re-backed)

Boot sequence, verified in `src/pso/unk/TLobby_80226C3C.cpp:~588`:

```c
ARInit(...); ARQInit(); AIInit(NULL);            /* GC audio hw on */
ARAlloc(0xC00000);                               /* 12 MB ARAM arena */
sndSetHooks(&hooks);
sndInit(48, 48, 48, 4, 1, 0xC00000);             /* 48 voices/music/sfx, 4 studios, 12MB ARAM */
adxt_init(); setup_adx_filesystems(); gcrna_bridge_init_buffers(...);
```

Runtime loop, verified in `src/musyx/runtime/hw_dolphin.c`:

- AI DMA double-buffers (4 × 0x280-byte buffers @ 32 kHz, `AIRegisterDMACallback` → `salCallback`).
- Each block: CPU `salBuildCommandList()` (`hw_dspctrl.c`) walks up to 64 `DSPvoice` structs and
  packs a ≤0xC0-word ARAM command list; `salCtrlDsp()` hands it to the microcode via
  `DSPSendMailToDSP` mailbox handshake and spin-waits.
- `dspSlave` (6,624 bytes, loaded as `DSPTaskInfo.iram`, `hw_dolphin.c:85–110`) executes that list:
  fetches sample/stream data **in ARAM**, synthesizes (GC-ADPCM/RNA decode with `coefTab[8][2]`,
  interpolation, ADSR deltas, volume via `dspScale2IndexTab`/`hw_volconv.c`, aux sends), and the mix
  is streamed out by AI DMA. **[INFERENCE]** the exact mix loop lives only in the ucode — the CPU
  side merely encodes voice start/end/loop addresses, `zeroAddr`, ADSR u16 deltas, cycle budget
  (`cyclesUsed = 10430` per 20-sample block at <200 ns delay).
- ADX path: `AdxBsc/AdxExpand` decodes ADX→PCM **on CPU** into ring buffers; `GcrnaStreamControl`
  then uploads and drives MusyX *streams* (`sndStreamAllocEx` + `sndStreamARAMUpdate`) — i.e. even
  the CRI PCM/RNA output is consumed as a MusyX stream voice on the DSP, not written to the DAC.
- There is **no CPU→DAC path**: `AIInitDMA`'s buffers are DSP-filled; zero direct PCM writes to AI
  anywhere in `src/` (grep-verified).

**Implication:** Option A ("port the in-tree stack, feed its PCM to CoreAudio") has no PCM tap
point. The PCM that would need tapping is generated *inside the DSP microcode*, which does not
exist as source. Option A therefore silently grows into "emulate or reverse-engineer the MusyX DSP
ucode + 12 MB ARAM + ARQ + mailbox timing" — a project on the order of Dolphin's DSP backend, not a
re-link.

## 3. Option A — port the in-tree MusyX/CRI stack, re-back the hardware

Scope: keep all 23k LOC of matched C; replace `hardware.c`/`hw_*.c`/`dsp_import.c` + ai/ar/dsp asm
with a macOS HAL that (a) emulates the MusyX ucode interpreter for the command list, (b) provides an
ARAM arena + stream buffers + ARQ, (c) drives a fixed 32 kHz / 20-sample render quantum from
CoreAudio/AudioQueue callbacks.

- **Pros:** maximum behavioral fidelity once done (exact volume/pan tables, ADSR delta semantics,
  aux-studio routing, all the quirks of the GCRNA 0.97 bridge); every game-side call site compiles
  unchanged; the hard decompilation of the 842 remaining asm functions (mixer-adjacent ones) still
  needed.
- **Cons:** the ucode's semantics must be recovered from a 6.6 KB PPC-DSP binary + command format
  (no source, no published disassembly of this MusyX variant); ARAM addressing, cache-sync and
  mailbox-spin timing quirks become our bugs; heavy coupling to OS threading/interrupt semantics
  (`OSThread`, `OSDisableInterrupts`, VI retrace hooks in the boot path).
- **Effort:** High — 4 decomp sub-projects (842 asm funcs incl. mixers), ucode RE, and a HAL.
  Weeks-to-months before first sound. **[INFERENCE: effort scale from boundary analysis above.]**
- **Faithfulness:** potentially bit-exact mix; the only route to *identical* sound.

## 4. Option B — native AVAudioEngine backend under a snd*/GCRNA shim

Scope: implement the ~22 distinct `snd*` MusyX functions the game uses + the ~13 used across the
CRI↔MusyX boundary as a native library (exact signatures already in `include/musyx/musyx.h` /
`GcrnaMusyxStream.h`), backed by AVAudioEngine: one player/stream node per GCRNA stream, one-shot
SFX nodes, `AVAudioUnitReverb`/aux studios, `AVAudioEnvironmentNode` for snd3d. Keep the portable
CRI layer in-tree as-is (ADXT server threads, LSC/SJ ring buffers, AFS access, voice logic) and
route its decoded output to the shim — or reuse `AdxDecode4.cpp` directly (it is portable C)
instead of libadx/vgmstream.

- **Pros:** no ucode RE at all; API surface is provably tiny (game side: 53 call sites, 22 funcs —
  measured; top users `sndFXCtrl`, `sndStreamMixParameterEx`, `sndFXKeyOff`, `sndStreamFrq`,
  `sndStreamARAMUpdate`); pure-C ADX decode stays in-house (option-A-grade fidelity for the
  *format*, which is what players actually notice); progressive: BGM streams → SFX → 3D → reverb
  can land independently; native low-latency I/O for free.
- **Cons:** mix fidelity differs (interpolation, rounding, aux send levels, ~48-voice polyphony
  semantics approximated); the 842 asm functions on the CRI side still need finishing *if we keep
  that layer* — but a Phase-B2 could substitute vgmstream/libadx wholesale and skip them, at the
  cost of re-doing GCRNA/Adxt call-site plumbing; timing semantics (streaming callback cadence)
  need a fixed render quantum to stay glitch-free.
- **Effort:** Medium — a shim over AVAudioEngine is ~1–2k LOC for BGM+SFX+3D parity; decoder risk
  near zero (in-tree C, or mature third-party).
- **Faithfulness:** high on content (same files, same data rates, same trigger logic), approximate
  on mix arithmetic.

## 5. Recommendation

**Option B**, with the deliberate twist of *keeping the in-tree CRI layer* (threads, filesystems,
GCRNA bridge, and the pure-C ADX decoders) and replacing only the MusyX runtime below the `snd*`
boundary.

**The single deciding fact:** every sample of sound in the game is mixed by the MusyX DSP
microcode, which exists in-tree only as a 6,624-byte binary blob (`dspSlave`,
`src/musyx/runtime/dsp_import.c`) with no CPU equivalent — so Option A's "portable PCM stack" is an
illusion, while the one piece everyone fears (ADX decode) is already pure, portable, matched C.

Why the small facts reinforce it: the hardware boundary is narrow (13 CRI→MusyX functions, 22
game-facing `snd*` calls), the PCM output path is single-pointed (AI DMA buffers, 32 kHz/20-sample
quantum maps 1:1 to an AudioQueue render callback), and the 12 MB ARAM arena plus GBA-ARAM sharing
(non-audio `AramPool`/`GbaAramCache` users) vanishes as a constraint the moment we stop
re-implementing ARAM.

### Suggested landing plan (B)
1. `pso-audio-hal`: `snd*` shim over AVAudioEngine exporting exactly the measured 22+13 signatures;
   boot stubs for `ARInit/AIInit/sndInit/sndSetHooks` in the TLobby path.
2. BGM first: `Adxt` server + `AdxDecode4`/`AdxExpand` (linked as-is) → `sndStreamAllocEx` PCM
   branches → verify against decomp behavior with 2 BGM players (`adx_create_stream_player`).
3. SFX/voices: `SoundRequestVoice.cpp` tick logic + `sndFXStart/KeyOff/Ctrl`.
4. 3D + aux: `snd3d`/studios → environment node + reverb units.
5. Optional fidelity pass: implement a software MusyX-semantics mixer (8-coef RNA decode,
   `dspAttenuationTab`, `hw_volconv` tables are all in-tree) if AVAudioEngine approximation
   audibly diverges — this is the *incremental* path to Option-A-level fidelity without betting
   the schedule on ucode RE.

## Appendix — measurement notes (all commands run against pso-decomp, GPOE8P)

- File/LOC counts: `wc -l src/pso/audio/*.cpp` = 9,215; `src/musyx` .c total = 13,409; du 360K/520K.
- Matched C functions counted by header-line regex across both dirs: 991. Unmatched: `.fn` grep in
  `build/GPOE8P/asm/{pso/audio,musyx}` = 509 + 333 = 842. (The asm trees also hold extab/ctab +
  .bss/.sdata data mirrors — those carry no instructions; zero `.globl`-style code survived in
  matched-module asm.)
- OS drivers `asm/ai/ai.s, ar/ar.s, ar/arq.s, dsp/*.s`: contain live `.fn` PPC code (AIRegisterDMA
  Callback, AIInitDMA, AIStartDMA, AISetStreamPlayState, __AI_set_stream_sample_rate, …) — reference
  C exists only in `libs/dolphin/src` and is MMIO-bound either way.
- `dspSlave` length = 0x19E0 (6,624 B) loaded at `hw_dolphin.c:93`; command list ≤0xC0 words, 64
  voice slots, `cyclesUsed=10430` in `hw_dspctrl.c:332+`.
- Game-side `snd*` usage grepped across `src/pso` (excluding musyx): 53 call sites / 22 distinct.
- CRI→MusyX boundary grepped across `src/pso/audio` + `include/pso/audio`: 13 distinct functions.
- No direct DAC PCM writes anywhere in `src/` outside `src/musyx/runtime` (grep `AIInit|OSInitAudio|
  AISetStreamPlayState|AISetVolume`).
