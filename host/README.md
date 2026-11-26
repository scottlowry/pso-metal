# host/ — pso-metal Native Host Platform API Scaffold (P1-N)

The re-hosted equivalents of the GameCube **dolphin SDK surface PSO EP&I actually
calls**, declared as the port's own API contract. Blueprint:
[`docs/host-surface.md`](../docs/host-surface.md) (272 distinct symbols / 1,415
call sites). All symbol lists and site counts were **re-measured** here with that
doc's §1 method against `pso-decomp @ 6b38b5383` (read-only); the 17 most-called
functions match the doc's per-symbol counts exactly, totals agree within
tokenizer noise (274 distinct incl. 8 macro shims vs doc's 272).

**This is declarations + effort map only — no implementations.** Bodies land in
the Phase-2 lanes named below.

## Layout

| File | Subsystem |
|---|---|
| `pso_host.h` | Umbrella (extern "C" guarded — game sources are C++) |
| `psosh_types.h` | u8…f64/BOOL/GXBool/Mtx/PADStatus/DVDDiskID shims, `ATTRIBUTE_ALIGN`, `PSOSH_TAG` macro |
| `psosh_gx.h` | Graphics pipe (86) |
| `psosh_exi.h` | Network adapter / EXI bit-bang (10) |
| `psosh_card.h` | Memory card (29) |
| `psosh_os.h` | Cache, interrupts, threads, time, heap, reset, debug (55) |
| `psosh_vi.h` | Video + AV mode (16) |
| `psosh_dvd.h` | Filesystem/DVD (13) |
| `psosh_audio.h` | AR/AI/DSP (15) |
| `psosh_pad.h` | PAD + SI polling (9) |
| `psosh_serial.h` | Flash/EEPROM serial (10) |
| `psosh_mtx.h` | PSMTX/C_MTX math (21) |
| `psosh_gba.h` | GBA link e+ (4) |
| `selftest.c` | Standalone compile gate (see below) |

Every entry carries: original SDK symbol name (kept identical — game sources
compile unmodified), call-site count, an effort tag, and a one-line native
backing. **45 featured** entries are real dolphin signatures with a
`TODO(P2)` plan; **221 long-tail** entries use `PSOSH_TAG(name, sites, effort,
backing)` — a machine-greppable placeholder `int name(void)` prototype that each
P2 lane replaces with the exact signature when it implements; **8 SDK macros**
(`OSAlloc`, `OSRoundUp32B`, `OSTicksToMilliseconds`, …) ship as direct shims.

## Subsystem → native backend

| Subsystem (symbols) | Effort | Native backing |
|---|---|---|
| GX graphics pipe (86) | **BLOCKING-GPU** | Metal: GX fixed-function state → MTLRenderPipelineState / DepthStencilState / VertexDescriptor cache; FIFO inline writers → CPU vertex streamer |
| OS cache/CPU/context (12) | TRIVIAL→HARD | Unified cache: DC* no-op **except** GPU-shared ranges (didModifyRange); PPCSync → `dmb ish`; OSContext → ucontext/pthread |
| OS interrupts/reset/exec (7) | MODERATE→HARD | Process-wide critical-section gate returning saved mask; OSResetSystem → relaunch |
| OS threads/message/mutex/alarm (24) | TRIVIAL | pthread + condvar 1:1; OSAlarm → dispatch_after |
| OS time/rtc (4) | TRIVIAL | mach_absolute_time at GC TB = 79.824327 MHz |
| OS heap/arena (10) | MODERATE | malloc-zone heaps + big mmap arena |
| OS debug/report (6) | TRIVIAL | os_log/fprintf; OSPanic → abort+dump |
| VI + AV mode (16) | MODERATE | CAMetalLayer + CVDisplayLink retrace pacing |
| DVD filesystem (13) | MODERATE | mmap/pread over extracted bundle; dispatch-queue command blocks, same status words (**BE**) |
| Memory card (29) | MODERATE | mmap-file card image (FAT+slot copy) + async worker keeping CARDAsyncResult |
| Input PAD/SI (9) | MODERATE | GameController.framework; PADStatus GC bit layout |
| Flash/EEPROM serial (10) | MODERATE | file-backed EEPROM + in-process SI device tree |
| Math PSMTX (21) | TRIVIAL | Pure FP CPU code, compile as-is (NEON later) |
| EXI network bit-bang (10) | **HARD** | No EXI: BSD sockets/Network.framework device model (all 236 sites in `src/pso/network/`) |
| Audio AR/AI/DSP (15) | **HARD** | AVAudioEngine/AudioQueue render worker; MusyX in-tree; DSP ucode = wildcard |
| GBA e+ (4) | TRIVIAL | Stub "no GBA attached" (honest emulation out of scope) |

## Effort distribution (all 274 entries, measured)

| Tag | Count | Share |
|---|---:|---:|
| TRIVIAL | 72 | 26% |
| MODERATE | 90 | 33% |
| HARD | 30 | 11% |
| BLOCKING-GPU | 82 | 30% |

The BLOCKING-GPU column is GX (+ its two inferred getters). The dominant hidden
cost remains the **big-endian payload sweep** at ~1,415 call boundaries
(host-surface.md §4) — it is not represented by any single tag here.

## Standalone compile gate

```
$ cd host && clang -std=c11 -Wall -Wextra -c selftest.c -o /tmp/psosh_selftest.o
$ echo $?
0
```

No warnings, no errors; `selftest.c` takes the addresses of the 17 most-called
SDK symbols as a declaration-exists check.

## Recommended Phase-2 build order (goal: boot-spine min-viable logo frame)

1. **OS spine** — `OSInit`, time (`OSGetTick`/`OSGetTime`), report, heap/arena,
   interrupts gate, threads (all TRIVIAL/MODERATE, no GPU): game runtime boots.
2. **DVD → bundle** — `DVDOpen`/`DVDRead`/command-block status over mmap/pread,
   then **VI + minimal GX** (`GXInit`, `GXSetViewport`/`Scissor`,
   `GXSetVtxDesc`/`GXSetVtxAttrFmt`/`GXBegin` streamer, `GXSetZMode`/`ZCompLoc`/
   `BlendMode`) — the first Metal-presented logo frame is the P2 gate.
3. **PAD** (GameController) for skip-attract, then **CARD** saves and the two
   long poles in parallel lanes: full GX tev/pixel-parity tracker and the BE
   struct sweep; EXI-net and audio follow (they gate nothing at boot).

## Status / WIP notes (P1-N)

- Per-subsystem symbol counts re-measured here vs doc totals differ by ≤1
  (dvd 13 vs 12, gba 4 vs 3) — tokenizer-classification noise, flagged, not
  resolved; doc remains the canonical surface inventory.
- GX enum typedefs are `int` aliases; exact enumerators must be exported from
  `gx/GXEnum.h` with the state tracker (TODO(P2-gx) in `psosh_types.h`).
- `PSMTX*` definitions still absent from the decomp bundle (§4 open question):
  P2-mtx lane lands `mtx.c` in the port build.
- No decomp tree was modified; only `host/` files were written.
