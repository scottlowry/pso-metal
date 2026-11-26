# Phase-0 verdict — pso-metal

Date: 2026-11-26. Decomp basis: `zeroz41/pso-decomp` @ `6b38b5383` (100% matched+linked,
DOL-sealed). All numbers below were integrator-re-measured or grep-verified at this sha;
lane self-reports that could not be reproduced were corrected in place (see audio-survey).

## The question Phase 0 had to answer
Can GC PSO EP&I's fully-decompiled, MIT-licensed source realistically be recompiled to run
natively on Apple Silicon with a Metal backend — or does some structural property of the
original make it a non-starter?

## The answer
**No blocker was found.** The port is viable on current evidence. The gap that looks huge
("only 669/1680 files parse") decomposes into two policy decisions and a small pile of
named exceptions:

| Step | Files clang-parses | How |
|---|---|---|
| Raw LP64 | 47 / 1680 | baseline |
| + arm64_32 (ILP32) + compat header + `-fms-extensions` | 591 | data model + intrinsics, zero decomp edits |
| + legitimate probe flags & shims (task A) | **669** | `-ffreestanding` (GC `void main`), narrowing waiver, `__fabs/__cntlzw/__abs/__dcbt` |
| + resolve Cause A/B assert policy (task B) | **~1,580 projected** | 913 failing files fail ONLY on asserts rooted in MWCC vptr placement (70%) + 12-byte PMF (30%); the asserts live in ~20 types / 43 lines in 15 headers |
| residual after policy | ~100 | 78 MWCC dialect (`AT_ADDRESS` absolute-bitfield overlays = SDK glue headers we replace wholesale), 54 extern-C-in-.c (flag/document), misc |

## What each survey found (condensed)
- **ABI (B):** not double-alignment, not `long`, not pointer size — it's MWCC's C++ object
  model: vptr placed *after* first polymorphic class's data (clang: offset 0), and 12-byte
  pointer-to-member-functions (clang: 8). One `TObject`-derived assert alone gates 823 TUs.
  98.7% of the tree's 3,256 size-asserts already pass unchanged.
- **Endianness (B+F):** the feared pervasive hazard is NOT pervasive. Live corruption sites
  found: **4** (`NetTcpState.h` u8:1/:1/:6 bitfields, `TEF_LensFlare.h` `int a:8` packed into
  a color word, `TAdChrMoveEffectGC.cpp` byte-indexed word reads) + 15 dead HW-register
  overlays. The broad class is byte-assembly shifts / buffer indexing: 629 files carry ≥1
  heuristic hit (`tools/be_scan.py`, reproducible in 4s), dominated by fixed-point noise;
  26 mixed-width unions in 23 files are the human-verifiable core. Sized, not scary.
- **Host surface (C):** 272 distinct dolphin SDK symbols at 1,415 call sites; only 121 files
  touch platform code at all. Heaviest: GX (86 fn / 503 sites), EXI bit-bang (236 sites but
  confined to `network/{eth,mdm}.c` — wholesale-replaceable with BSD sockets later), CARD
  (29 fn / 127). The Metal port surface is a few hundred functions with ~35 boundary fns per
  subsystem.
- **Boot (D):** entry `__start → main` (`SystemBootInit.cpp:365`) → `init_game_systems`
  (48 actions, 12 stages) → `main_phase_table` dispatch; title = phase 10 (attract). A
  **minimum viable boot** (logo frame, no audio/font/GBA) is explicitly cut and documented.
- **Audio (E):** ADX decode is already portable C; the MusyX mixer is a 6,624-byte DSP
  microcode blob (`dsp_import.c`) with no CPU-side PCM tap → **Option B**: keep the CRI CPU
  layer, shim ~22 `snd*` + ~13 CRI→MusyX boundary calls onto AVAudioEngine. DSP ucode RE
  stays optional (bit-exact fidelity path), not on the critical path.

## The honest caveats (Phase 1 gates)
1. **Parses ≠ correct.** The size-asserts exist because layout is load-bearing against
   *binary formats* (memory-card saves, network packets, disc data). The assert-policy step
   must split **runtime-object guards** (safely waived on host) from **format guards** (must
   be kept byte-true or byteswapped at the loader). This is Phase 1's first work item — a
   format-boundary audit keyed off the BE-hazard index + save/packet structs.
2. **No differential oracle yet.** Byte-exact matching gave the decomp a perfect verifier;
   the port has none yet. Phase 1 needs a behavior-diff harness (era-compiled PPC goldens
   vs host behavior) before trusting subsystem rewrites. Design exists (compile both sides
   from one source; PPC side via the repo's pinned wibo+MWCC), build cost unmeasured.
3. **PMF/vptr semantics under waiving:** waiving the assert doesn't make clang's virtual
   dispatch wrong for host purposes — it makes it *different*, which is fine for pure
   runtime objects and fatal only where a format or hand-written offset touches them. The
   audit in (1) is what licenses each waiver — per-header, logged, no blanket -D.

## Recommendation
Proceed to **Phase 1: minimum viable boot** in the documented order:
1. assert-policy split + format audit (the (1) above) — the one true unknown that could bite;
2. host glue: arena/heap model, FS shim (BYO-disc locator, reads the user's own ISO locally),
   Metal 640×480 present;
3. boot spine through `init_game_systems` to first logo frame;
4. font pass → text title screen → decision point for public/community handoff (the gate
   Scott sets: a visible title screen + ≥90% parse rate is the natural "ask people for help"
   artifact).

Exit bar for Phase 0 formally: met. Every Phase-0 failure class is measured, named, and has
a strategy; the two refuted brief-hypotheses (double-align, bitfield-pervasiveness) were
caught by lanes measuring instead of complying.
