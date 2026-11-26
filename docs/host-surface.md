# Host-Surface Map — GameCube (Dolphin SDK) API dependencies of PSO EP&I

Phase-0 task C. Enumerates the platform API surface the MIT decomp actually calls, so the
Apple-native host layer has a finite checklist. All numbers below were measured with the
method in §1 against the decomp tree; every function name is cited to the header that
declares it (paths relative to `pso-decomp/libs/dolphin/include/dolphin/`).

## Headline

**272 distinct platform symbols (266 functions + 6 SDK macros) called from 1,415 call
sites** in `src/` (1,680 `.cpp`/`.c` files). Only **121 of 1,680 source files (7.2%)**
touch the platform surface at all — the game's engine code is heavily wrapped (Ginja/T*
classes), which is good news for porting isolation.

Reference population for context: the bundled SDK headers declare **649 functions + 187
macro-functions**, so the game uses ~39% of the declared surface. Additionally the game
calls **32 libc names × 1,943 sites** (`memset`×429, `strncpy`×309, `memcpy`×179,
`sprintf`×117, …; declared in `include/clib`) — free on macOS — and leans on CodeWarrior
intrinsics/keywords (`__declspec`×1,034, `__memcpy`×57, `__cntlzw`×52,
`__cvt_fp2unsigned`×25, `__frsqrte`×12, `__fabs`×7): pure codegen surface, mappable to
`__builtin_*` + attribute shims, not Dolphin API.

## 1. Method (reproducible)

1. Tokenized every identifier followed by `(` in all 1,680 `src/**/*.{cpp,c}` files
   (Python regex `([A-Za-z_]\w*)\s*\(` over latin-1 text) → call-site counts per symbol.
2. Parsed all 95 headers under `libs/dolphin/include/dolphin/**/*.h`: statement-based
   extraction of prototypes and `static inline` definitions (comments + preprocessor lines
   stripped; `typedef` excluded) → 649 declared functions; `#define NAME(` → 187 macro
   functions.
3. Intersected (1)∩(2); subtracted libc names declared in `include/clib` (system-provided
   on macOS). Four real SDK calls (`OSInit`, `EXIGetType`, `GXGetViewportv`, `GXProject`)
   were re-added after manual triage (e.g. `OSInit` used at
   `src/pso/ui/keyboard/SystemBootInit.cpp`, `EXIGetType` only in
   `src/pso/ui/frontend/TAdTrialModemChkGC_8042E9FC.cpp`).
4. Triaged every remaining SDK-prefix token to a non-call or game-local artifact:
   `ARR_PTR` (local macro, `src/musyx/runtime/seq.c:5`), `VID_UNLINK`
   (`src/musyx/runtime/synthvoice.c:23`), `AIM_AT_TARGET`
   (`src/pso/technique/TTecCtrl.cpp:160`), `GX_FIFO`
   (`src/pso/ui/window/TWindowSubStatusDisp_803DEE6C.cpp:16`), `PADO/PADS/PADT/AIFF`
   (prose in `src/pso/network/NetPppoe.c:464` comment). Suffixed in-file aliases
   `GXPosition3f32_803E2D70`×2, `GXColor1u32_803E2D70`×2, `GXTexCoord2f32_803E2D70`×2,
   `GXWGFifo_803E2BCC`×1 are duplicate copies of the `gx/GXVert.h` inlines; counted as
   call sites under their base functions, not as extra distinct symbols.
5. `include/**/*.h` contains **declarations only** for SDK symbols (spot-verified: the
   `VIWaitForRetrace`/`OSGetTick`/`OSReport`/`DCInvalidateRange` hits in e.g.
   `include/pso/audio/AdxManager.h:23` and `include/pso/quest/QuestDownloadServer.h:132-135`
   are re-prototype lines, not call sites) — hence "src/ only" for call counting.
6. No `Nu*`/`NJ*`/`thd*` *calls* exist in the tree (NuSystem is not linked here; `NJS_POINT2`
   appears only as a type name inside mangled symbols). The matrix layer actually used is
   **PSMTX/C_MTX**, declared in the vendored `mtx.h`.

## 2. Per-subsystem surface

Distinct = SDK symbols called ≥1×; Sites = call occurrences in `src/`. Native-backend and
difficulty are **estimates/guesses** (labeled as such), assuming the rest of the engine
compiles per the ILP32+shim result.

| Subsystem | Declaring headers | Distinct | Sites | Representative (sites) | Proposed native backend | Difficulty (guess) |
|---|---|---:|---:|---|---|---|
| Graphics pipe | `gx/*.h` | 86 | 503 | GXSetZMode×33 `gx/GXPixel.h`, GXSetVtxDesc×30 + GXBegin×23 + GXSetVtxAttrFmt×21 `gx/GXGeometry.h`, GXSetZCompLoc×25 `gx/GXPixel.h` | Metal: fixed-function GX state → MTLRenderPipelineState + MTLDepthStencilState + MTLVertexDescriptor; display lists → pre-built vertex buffers; `gx/GXVert.h` inline FIFO writers → CPU-side vertex streamer | **XL** — deepest rewrite; state-tracking semantics, tev combiner ≈ MTL fragment but exact fixed-function blend order must be matched |
| Network adapter (EXI bit-bang) | `exi.h` | 10 | 236 | EXIImm×52, EXISync×52, EXISelect×33, EXIDeselect×33, EXIUnlock×30 (all 236 sites live in `src/pso/network/` — `eth.c`, `mdm.c`) | Replace driver wholesale: BSD sockets / Network.framework behind the same T* net classes; the byte-level EXI protocol layer is GC-only | **L** — small API, but the code above it assumes EXI transfer timing/semantics |
| Memory card | `card.h` | 29 | 127 | CARDSetDiskID×34, CARDGetResultCode×20, CARDGetDiskID×12, CARDProbeEx×6 | File-backed card image (FAT + slot copy semantics) + async worker thread preserving the CARDAsyncResult contract | **M** |
| Cache / CPU ops | `os/OSCache.h`, `os/OSContext.h`, `base/PPCArch.h`, `os/OSUtil.h` | 12 | 113 | DCInvalidateRange×19, DCFlushRange×18, PPCSync×17, DCStoreRangeNoSync×16, OSClearContext×7 `os/OSContext.h` | Apple-silicon unified cache → DC* become no-ops **except** where the buffer crosses to the GPU (Metal shared buffers need didModifyRange/device mapping instead); PPCSync → `dmb ish`; OSContext = thread-context save/restore → ucontext/pthread or restructure | **M–L** — mechanically small, semantically trap-laden (see §4) |
| Interrupts / reset | `os/OSInterrupt.h`, `os/OSReset*.h` | 6 | 71 | OSRestoreInterrupts×33, OSDisableInterrupts×28, OSResetSystem×4 | Critical sections: per-process "disable" → spinlock/atomic gate honoring the nesting-mask return value | **M** — the save/restore-mask idiom must round-trip across threads |
| Threads / sync | `os/OSThread.h`, `OSMessage.h`, `OSMutex.h`, `OSAlarm.h` | 24 | 70 | OSSendMessage×7, OSResumeThread×7, OSInitMessageQueue×6, OSReceiveMessage×6, OSSleepThread×6 | pthreads + condvar-backed OSMessageQueue/OSMutex; OSAlarm → dispatch_after or timer fd | **S–M** — near 1:1 |
| Math (PSMTX/C_MTX) | `mtx.h` (vendored in bundle) | 21 | 66 | PSMTXConcat×20, C_MTXLightPerspective×8, PSMTXScale×5, PSMTXInverse×5 | Pure FP CPU code — compile natively as-is (no I/O); **no host shim needed if definitions land in the port build** (see §4) | **S** |
| Video / display | `vi.h` + `os/OSRtc.h` AV-mode | 16 | 56 | VIWaitForRetrace×16, VIFlush×8, VISetBlack×6, OSSetSoundMode×3 | CAMetalLayer + CVDisplayLink (or MTKView frame callback) for retrace pacing; progressive/50-60Hz → native mode switch | **M** |
| Filesystem (DVD) | `dvd.h` | 12 | 39 | DVDGetCommandBlockStatus×8, DVDClose×6, DVDGetDriveStatus×5, DVDCheckDisk×5, DVDOpen×3 | Pre-extracted bundle on disk read via mmap/pread; the async DVDCommandBlock state machine → GCD/dispatch queue returning the same status words | **M** — API tiny, but callers poll `DVDGetCommandBlockStatus` and parse `DVDDiskID` (byte order!) |
| Audio (AR/AI/DSP) | `ar.h`, `ai.h`, `dsp.h`, `os/OSAudioSystem.h` | 15 | 32 | ARQPostRequest×8, ARAlloc×6, AIInitDMA×2, DSPSendMailToDSP×2 | MusyX itself is decompiled in-tree (`src/musyx`, only 5 files touch SDK) → compile it; ARQ request queue → thin shim executing mix requests on a CoreAudio/AudioQueue render thread; AI DMA → ring buffer; **DSP ucode task (`dsp/lib`) is the wildcard** | **L** |
| Clock / timer | `os/OSTime.h`, `OSRtc.h` | 4 | 24 | OSGetTick×13, OSGetTime×8, OSTicksToCalendarTime×2 | mach_absolute_time scaled to the GameCube timebase semantics (keep TB=79.824327 MHz units for tick math!) | **S** |
| Memory / heap | `os/OSAlloc.h`, `OSArena.h` | 10 | 24 | OSFreeToHeap×7, OSAllocFromHeap×6, OSCreateHeap×2 `os/OSAlloc.h`, OSGetArenaLo×2 (also `OSAlloc` macro×1) | Keep OSHeap semantics over a macOS malloc zone / custom bump allocator; arena lo/hi → large `mmap` reservation | **S–M** |
| Debug / report | `os.h` | 6 | 20 | OSReport×8, OSTicksToMilliseconds×4, OSPanic×3, OSCachedToPhysical×3, OSInit×1 | os_log/fprintf; OSPanic → assert-with-dump; OSCachedToPhysical → identity (no guest phys map needed if we avoid DMA-addressed APIs) | **S** |
| Input (PAD/SI polling) | `pad.h`, `si.h` | 9 | 17 | PADControlMotor×4, SIDisablePolling×3, PADRecalibrate×3, PADRead×2 | GameController.framework (GC Pro Adapter or mapped DualSense/Xbox); PADRead sample struct must be filled with GC `PADStatus` bit layout incl. analog stick calibration fields | **M** — the PADStatus struct bit-packing + error states is where bugs will hide |
| Flash / EEPROM serial | `os/OSSerial.h` | 9 | 14 | SIGetResponse×3, SIRegisterPollingHandler×2, SIProbe×2, SIGetTypeAsync×1 | SI device tree emulated in-process: EEPROM/flash (game-time, settings) → files; handler registry preserved | **M** |
| GBA link (e+) | `gba.h` | 3 | 3 | GBAInit, GBAGetProcessStatus, GBAJoyBootAsync (1 each) | v1: stub reporting "no GBA attached" via SI/GBA probe paths | **S** to stub, **XL** to emulate e+ cable honestly |

Totals: **272 distinct / 1,415 sites** (rows sum after the AT_ADDRESS artifact removal).

## 3. 15 most-called platform functions

| # | Function | Sites | Declared in | What it does (dolphin semantics) |
|---|---|---:|---|---|
| 1 | `EXIImm` | 52 | `exi.h` | Queue an immediate/PIO word transfer to an EXI peripheral channel |
| 2 | `EXISync` | 52 | `exi.h` | Block until the pending EXI transfer completes |
| 3 | `CARDSetDiskID` | 34 | `card.h` | Bind the memory-card API to this game's DVDDiskID (card space ownership) |
| 4 | `OSRestoreInterrupts` | 33 | `os/OSInterrupt.h` | Re-enable machine interrupts with a previously-saved MSR mask |
| 5 | `GXSetZMode` | 33 | `gx/GXPixel.h` | Depth test enable/compare-fn/write-enable |
| 6 | `EXISelect` | 33 | `exi.h` | Chip-select an EXI device on a channel |
| 7 | `EXIDeselect` | 33 | `exi.h` | Release the chip-select |
| 8 | `EXIUnlock` | 30 | `exi.h` | Release the EXI bus lock |
| 9 | `GXSetVtxDesc` | 30 | `gx/GXGeometry.h` | Enable a vertex attribute + its fetch mode |
| 10 | `OSDisableInterrupts` | 28 | `os/OSInterrupt.h` | Mask interrupts, returning prior state (critical-section idiom) |
| 11 | `GXSetZCompLoc` | 25 | `gx/GXPixel.h` | Choose Z-compare before (early-Z) or after texture combine |
| 12 | `GXBegin` | 23 | `gx/GXGeometry.h` | Begin primitive drawing into the vertex FIFO |
| 13 | `GXSetVtxAttrFmt` | 21 | `gx/GXGeometry.h` | Type/count/scale of each vertex attribute in the stream |
| 14 | `CARDGetResultCode` | 20 | `card.h` | Async result word of the last card command |
| 15 | `PSMTXConcat` | 20 | `mtx.h` | PSO's 4×4 matrix multiply helper (vendored math, not hardware) |

(16th by a hair: `DCInvalidateRange`×19 and `GXSetBlendMode`×19, tied.)

## 4. Unknowns / guesses (explicitly)

- **Byte-order debt is invisible to these counts.** Nearly every subsystem above passes
  big-endian payloads inside structs or on hardware queues: GX command words,
  `DVDDiskID`/`DVDDirEntry`, `CARDStat`/`CARDInfo`, `PADStatus`, ARQ request commands,
  EXI word streams, `OSGetTime` 64-bit ticks. A call-site→shim port must byte-swap at
  each of these ~1,415 boundaries or the structs' guest layout. This is the dominant
  hidden cost; my mapping-intent column does not solve it.
- **`GXSetRopMode`, `GXSetTevOrder`-family presence:** my tokenizer found `GXSetZMode`
  etc. but not `GXSetRopMode` (checked — undeclared and uncalled here); the 86-function GX
  list is exactly what `src/` calls, but calls hidden behind *game-side macros*
  (T* wrapper methods defined in headers with SDK calls inside) were counted once per
  textual occurrence. **Hypothesis, unquantified:** inline/macro-expanded SDK calls inside
  `src/` headers could shift site counts by low single-digit percent.
- **Are PSMTX/C_MTX functions host-provided?** They are declared in the bundled `mtx.h`,
  but I found no definitions in `src/` (grep `void PSMTXConcat` → none) nor in the
  bundle. Either the decomp build vendors a `.c` outside `libs/dolphin/include`, or the
  port must reimplement ~21 pure-math helpers. Flag for task follow-up; trivial either way.
- **Audio DSP ucode:** `DSPSendMailToDSP`×2/`DSPCheckMailToDSP`×2/`DSPAddTask`×1 suggest
  the MusyX ucode protocol; `src/musyx` decompiles the *host* side, but the ucode blob
  itself (`.hba`/coef data) is data, not API — how the port executes mix commands
  (interpret vs rewrite to Apple Audio) is undecided.
- **e+/GBA feature honesty:** 3 call sites prove the API surface exists; whether the
  shipped NTSC-J/U disc ever boots an e+ GBA cart determines if stubbing (1 week) or
  emulating (never) is correct. I did not verify runtime reachability.
- **`src/runtime/` (CodeWarrior runtime) does not exist in this checkout** — only
  `src/pso` + `src/musyx` (verified via `find src -maxdepth 1 -type d`); the task brief's
  layout reference to `src/runtime/`/`src/interface/`/`src/code/` did not apply here.
  The `__*` intrinsics above stand in for that dependency.
- **`EXIGetType`×2** has no declaration anywhere in the decomp tree (used in
  `TAdTrialModemChkGC_8042E9FC.cpp`); inferred to be the real SDK EXI routine — counted
  from knowledge, labeled inference.
- **`OSSetSoundMode`/`OSGetProgressiveMode`** (in `os/OSRtc.h`, not vi.h!) were bucketed
  with video as AV-mode concerns.
- **Difficulty column is judgment, not measurement.** S ≈ days, M ≈ weeks, L ≈ a month+,
  XL ≈ quarter-scale.

## 5. Suggested porting order (judgment)

S-tier first (timer/heap/debug/threads → game boots), then DVD→filesystem,
CARD→file saves, VI/PAD, and leave GX-tev semantics + the BE struct sweep as the two
dedicated long-poles. The EXI network layer can be cut out independently of everything
else because its 236 sites are confined to `src/pso/network/`.
