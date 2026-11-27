# P2-Q — bring-up core transcript (init-ledger §4 steps 1–4 + step 19 PRS)

Date: 2026-11-26 (PST). Decomp snapshot: `pso-decomp @ 6b38b5383` (READ-ONLY,
never written). Lane N (`host/pso_host.h`, `host/README.md`, `host/selftest.c`)
untouched/unstaged by this lane; commit staged only `host/bringup/**` + this doc.

## 1. RESULT — FIRST EXECUTING GAME LOGIC ON M-SERIES ✅

One command: `host/bringup/run.sh` → **exit 0**. Real transcript (trimmed):

```
== boot spine: crt0 -> mini OSInit -> DVD shim -> era PrsDecoder::prs_decompress ==
crt0: arena 0x108d30000 (main 24MB + ARAM 2MB @ 0x10a530000), busClock @arena+0xF8 = 32419584
dvd_shim: fake bundle root '.../host/bringup/fake_disc' -> drive READY
crt0: DVD read logo_e.prs: 9 of 9 bytes
crt0: PRS-decoded 9 bytes -> "ABCDABCDA"  <== FIRST ERA GAME LOGIC ON ARM64
crt0: OSGetTick 3282544824 -> 3282546672 (delta 1848 @8104896Hz)
crt0: OSAllocFromHeap(0x4560) -> 0x109131018
boot_spine: EXIT=0 (era game function executed on arm64)

== PRS golden vectors (oracle: hand-traced from era prs.s) ==
PASS sysret: busClock word at arena+0xF8 == 32419584 (TB = bus/4 = 8104896)
PASS substrate: PRS window pinned at arena+0x1800 (era literal 0x80001800 slot)
PASS G0 empty stream: ret=0 expect 0
PASS G1 literal run: ret=4 out=ABCD
PASS G2 near-token overlapping RLE: ret=6 out=XXXXXX
PASS G3 far-token back-reference: ret=9 out=ABCDABCDA
PASS G6 DVDOpen(logo_e.prs) -> 1
PASS G6 fake-disc -> arena -> era PRS decode end-to-end: ret=9 out=ABCDABCDA
PASS G5 corrupt negative offset -> error: ret=-1 expect -1
PASS G4 window-flush RLE stream (82 bytes in): ret=6145 content=6145x'Z'
PASS roundtrip property: 64 synthetic RLE streams, all byte-exact
PASS clock shim: OSGetTick advanced 3283885341 -> 3283897620
PASS clock shim: 1 timebase second == 1000 ms
PASS heap: two OSAllocFromHeap distinct blocks
PASS heap: free+realloc, live count=2 expect 2
PASS heap: balanced free -> 0 live
== PRS golden + substrate: 16/16 PASS ==
prs_golden_test: EXIT=0
BRINGUP GREEN
```

Era functions that executed (compiled from decomp source, not reimplemented):
`PrsDecoder::prs_decompress`, `PrsReader::prs_read_control_bit`,
`PrsReader::prs_read_src_byte`, `PrsDecoder::prs_read_byte_at`,
`PrsDecoder::prs_next_output_byte` (incl. both memmove flush sites),
`PrsDecoder::PrsDecoder/~PrsDecoder`, `PrsDecoder::prs_release_window`.
PRS unit self-containedness: compiled with only `types.h` + `pso/system/prs.h`
(+ `pso/system/memory.h` decls, zero calls) — **1 decomp unit**, no narrowing
needed. Golden correctness evidence beyond vectors: the two vector-encoding
slips during bring-up (§5) mis-decoded *exactly* where the hand-trace predicted,
i.e. the decoder never moved — the oracle math was re-derived until it agreed.

## 2. Deliverables (host/bringup/)

| File | Ledger step | Content |
|---|---|---|
| `bringup.mk` | 1 | Port flag set: `-DPSO_PORT_LAYOUT_WAIVER`, `-include compat + build/waivers/pso_assert_waivers.h` (the generated `PSO_LA_FLAG_*` name set IS the waiver list; regenerated via `gen_waivers.py` as order-only prereq, fail-closed + deny-list intact), shadow `-I include` first (assert-policy §4 mechanism), `LINK_TARGET arm64-apple-macos12.0`. Does not touch build/Makefile. |
| `include/types.h` | 1 | Port shadow of `include/types.h`: fixed-width `<stdint>` rebind u8…u64/s8…s64 (era `unsigned long`→64-bit trap), `uaddr/saddr = uintptr/intptr_t`, era macros mirrored incl. the verbatim `size_t` block (D6); installs KEEP-honest `LAYOUT_ASSERT` + hard tripwire if the waiver flag is missing. |
| `crt0.c` | 2 | One 26 MB zero-filled mmap arena (24 MB main RAM + 2 MB ARAM back-half), SysRet BootInfo anchor at arena+0xF0, window base at arena+0x1800, `pso_g2h()` guest→host translator, guarded `main()` = boot spine. |
| `osinit.c` | 3 | Mini OSInit: arena lo=+0x3100/hi=+24 MB globals, timebase shim (wall-clock-backed, 8,104,896 Hz = bus/4, matching all 9 era consumers), `OSReport→vprintf`, OSCreateHeap-shaped heap shim (OSInitAlloc/OSCreateHeap/OSAllocFromHeap/OSFreeToHeap/OSSetCurrentHeap/__OSCurrHeap/OSCheckHeap over a 0x518000-sized game heap at arena+0x400000). No threads/interrupts/MPU. |
| `dvd_shim.c` | 4 | DVD over `$PSO_FAKE_DISC` dir: `DVDInit/DVDOpen/DVDReadPrio/DVDReadAsyncPrio/DVDClose/DVDGetDriveStatus/DVDCheckDisk/DVDGetCommandBlockStatus` — exactly the API the decomp game units call (SystemBootInit.cpp:258, text_803DD5E0.cpp:91–115, movie/Gcci.cpp:255–257/407–461). pread-backed, fd in `cb.userData`, transferred-size words filled, status=READY, async completes synchronously + fires callback result 0. Structs mirror `libs/dolphin/include/dolphin/dvd.h` field-for-field. |
| `fake_disc/` | 4 | Synthetic-only bundle: `logo_e.prs` (9-byte hand-built PRS stream = step-19 TLogo stand-in), `title.cfg`, README. No disc/ISO data. |
| `prs_golden_test.cpp` | 19 | Vectors G0–G6 (hand-trace comments citing prs.s addresses 0x8036F4DC..0x8036F7A8), window-flush case G4, error case G5, DVD→arena e2e G6, 64-stream roundtrip, substrate smokes. Per-case PASS/FAIL, exit 0 only all-green. |
| `include/prs_window_glue.h`, `prs_support.cpp` | 2/19 | Window-base glue + era-decoder C entry. |
| `run.sh` | — | One-command build+boot-spine+golden; `set -e`, exit 0 = GREEN. |

## 3. BootInfo anchor — what the decomp actually reads at 0x800000F8

9 live derefs, all `*(u32 *)0x800000F8` = the **bus-clock word**
(NetworkRuntime.c:140,145; NetworkThreadRuntime.c:58; SoundControlState.cpp:913;
TPlayerBattleCtrl.cpp:353; TPlayerBattleCtrl4P.cpp:280; hw_dolphin.c:132;
hw_dspctrl.c:277 `__OSBusClock AT_ADDRESS(0x800000F8)`; QuestDownloadServer.h:134).
Anchor `PsoSysRet` mirrors the retail SysRet intra-block layout at arena+0xF0:
memSize(F0)=0x01800000, exiFreq(F4)=16,209,792, busFreq(F8)=32,419,584,
cpuFreq(FC)=486,293,760 (15×bus). Values host-native LE — the port executes as
arm64; BE only on the disc/wire path (compat/pso_byteswap.h domain).
Timebase = bus/4 = 8,104,896 Hz is the single rate all consumers divide by.

## 4. Deviations from plan/era (every one, deliberate)

* **D1 — no absolute arena.** `mmap(0x80000000, 26M, MAP_FIXED)` **and**
  `mach_vm_allocate` both fail on this macOS arm64 (ENOMEM/KERN_INVALID_ADDRESS;
  even 0x10000000 fails — low VA refused for normal tasks; probe in session log).
  Adopted the p1-datamodel fallback: arena base + `(guest - 0x80000000)` via
  `pso_g2h()`. Guest-side semantics preserved, absolute literals become glue.
* **D2 — bootinfo via anchor pointer.** Era readers load literal 0x800000F8.
  Those units are NOT linked yet; when they are, each such load-site unit gets
  the same one-token rebase treatment as D3, or readers move to
  `pso_g2h(0x800000F8)`. Anchor layout ships now (tested).
* **D3 — PRS window literal rebase.** `prs.cpp` pins `window = (u8*)0x80001800`.
  bringup.mk generates `build/bringup/gen/prs_glue.cpp` = the decomp source with
  exactly that one literal → `pso_prs_window_base` (= arena+0x1800, the SAME
  arena slot the era used, ending at the era heap-lo boundary 0x3100 — verified).
  Guards: literal count must be 1; reverse-sed roundtrip `diff -q` must be
  byte-identical to the source; drift = build failure. Window logic itself untouched.
* **D4 — include-path split from build/Makefile.** Executing TUs drop `-I
  include/clib` + both `-I libs/dolphin/include*`: those shadow host libc
  (`dolphin/string.h` hijacked `-include <string.h>`; `dolphin/types.h` redef'd
  uintptr_t; `libc/math.h` vs compat shims). Parse targets keep them. Nothing
  executing today includes dolphin.h; when a unit needs it (step 4+), a
  port-side `dolphin` facade over host libc is the intended answer.
* **D5 — no `-ffreestanding`** in bringup COMMON: with it, clang mangles C++
  `main` (`__Z4mainv`) → no entry point. It was a parse-only flag.
* **D6 — shadow types.h must re-export the era `size_t` block verbatim**: the
  `_SIZE_T_DEFINED` guard interplay silently deleted `size_t` from macOS
  `<stdio.h>` until restored. Tripwire typedef (compile-fail without
  `-DPSO_PORT_LAYOUT_WAIVER`) is by design; the IDE LSP shows it red — that is
  the mechanism working.
* **D7 — DVD struct glue is bringup-local** (`pso_bringup.h` mirrors dvd.h
  fields incl. `DVDCommandBlock` pointer members at host width). When a real
  DVD-caller unit (step 7+) compiles against the decomp's `dvd.h`, the shim's
  decls must come from that header instead; re-verify field usage then.
* Heap shim is shape-only (bump + free-list). `init_system_heap_and_global_
  allocator` (memory.cpp) stays step 7 per ledger — deliberate, not found
  entangled, just out of scope here.
* Fake disc path: era absolute-FST names accepted, leading `/` stripped, `..`
  rejected. No FST parsing (Adxf/CvFs step).

## 5. Lessons (golden-vector discipline)

Both failures during bring-up were MY hand-encodings, never the decoder:
(a) 0xBF has b4=1 → the trace predicted a 5th literal consuming the token's
size byte → observed "ABCD\xE3\xFF" n=6 exactly; (b) 15+32+128 = **0xAF**, I
wrote 0xA7 → predicted-and-observed exact error-path divergence at the
near-token branch. The hand-trace comments in
prs_golden_test.cpp now show the LSB-first bit expansions inline — keep that
style for all future vectors.

## 6. Blockers / next steps (exact)

1. **Step 5 (GX window) is the first hard blocker after this lane**: nothing
   before it needed Metal; `VIInit`/GXInit/first-framebuffer rows (ledger
   step 8) need CAMetalLayer + CVDisplayLink retrace counter. Everything
   steps 1–4 substrate is ready for arena/heap/clock/DVD consumers.
2. **Ledger step 7**: compile `src/pso/system/memory.cpp` over osinit.c's
   heap shape; `init_system_heap_and_global_allocator((u8*)(cfg.heap +
   0x518000), hi - start)` per SystemBootInit.cpp:275 — check memory.cpp's
   LAYOUT_ASSERTs against the waiver list first.
3. **D2 sweep**: first unit that derefs 0x800000F8 absolute (e.g. linking
   NetworkRuntime/SoundControlState consumers) needs the guarded one-token
   rebase (`pso_sysret->busFreq`) like prs_glue; extend the bringup.mk
   pattern-rule.
4. **`uaddr` in game structs**: my rebind makes uaddr 64-bit; per p1-datamodel
   §1b handle round-trips assume base+handle — spot-check the first unit that
   stores `this` into uaddr fields.
5. **Phase-P lane updates (mid-task, from parent)**: (a)
   `reset_sound_request_state` (SoundRequests.cpp:395) runs EVERY logo frame —
   keep-as-is pure decomp logic, compiles into step-15+ builds; NOT stub-able.
   (b) quest-VM `is_ready()==true` at logo time hits a phase-0 shutdown trap
   (dispatch D-series) — if the dispatcher is ever linked before step 21, force
   not-ready. Neither touched this lane's steps 1–4/19 (nothing linked here
   calls either).
6. `prs_decompress_buffer/prs_decompress_alloc` (declared in prs.h) are defined
   in other units — first real asset loader (step 10) will need their homes
   located (TLobby/BmlResource call sites exist).
