# P2-AM — chorus stand-in for link-day hygiene

Phase-2 lane AM. Vends the **2 chorus golden names** that block an honest
24/24 `tools/link_hygiene.sh --musyx` pass (AF finding,
[`p2-link-hygiene.md`](p2-link-hygiene.md) §4 · rows 2–3 of
[`musyx-linknames.tsv`](../data/musyx-linknames.tsv)), measured against
`pso-decomp @ 6b38b5383` **read-only**. Files:
[`compat/chorus/chorus_stand-in.c`](../compat/chorus/chorus_stand-in.c) ·
[`chorus_stand-in.h`](../compat/chorus/chorus_stand-in.h) ·
selftest [`chorus_stand-in_selftest.c`](../compat/chorus/chorus_stand-in_selftest.c).

The two names: **`sndAuxCallbackChorus`** (decl `include/musyx/musyx.h:166`,
real def `src/musyx/runtime/Chorus/chorus_fx.c:123`) and
**`sndAuxCallbackPrepareChorus`** (decl `musyx.h:167`, real def
`chorus_fx.c:205`), both referenced from
`src/pso/audio/SoundControlState.cpp:978-980` (prepare, then register as the
`auxB` `SND_AUX_CALLBACK` via `sndSetAuxProcessingCallbacks`).

## 1. AF's unhostability claim — CONFIRMED independently (2 scratch probes)

Probe A = AF's exact §2 flags (`clang++ -std=c++17 -x c++ $DIALECT $COMMON`,
`-target arm64-apple-macos11`), no GPOE8P include path:

```
src/musyx/runtime/Chorus/chorus_fx.c:113:5: error: expected '(' after 'asm'
src/musyx/runtime/Chorus/chorus_fx.c:113:4: error: expected ';' after top-level asm block
src/musyx/runtime/Chorus/chorus_fx.c:114:10: fatal error: 'orig_asm/musyx/runtime/Chorus/chorus_fx/do_src1.inc' file not found
```

Probe B = same + `-Ibuild/GPOE8P/include` (the `.inc` blobs exist there):

```
src/musyx/runtime/Chorus/chorus_fx.c:113:5: error: expected '(' after 'asm'
build/GPOE8P/include/orig_asm/musyx/runtime/Chorus/chorus_fx/do_src1.inc:1:1: error: unknown type name 'nofralloc'
build/GPOE8P/include/orig_asm/musyx/runtime/Chorus/chorus_fx/do_src1.inc:2:7: error: expected ';' at end of declaration
```

Error classes: MWCC-only `asm void fn(...)` function bodies (:113/:117) whose
`#include`d content is raw PPC assembler with `nofralloc` /
`opword 0x9421FFC0` directives — no host clang flag set parses this. Claim
stands: the TU has **no** host-compilable path at this sha.

## 2. What the stand-in is vs what is real

| golden fn | real era body (chorus_fx.c) | stand-in body (chorus_stand-in.c) |
|---|---|---|
| `sndAuxCallbackChorus` (`u8 reason, SND_AUX_INFO *info, void *user`) | reason-0 mixes chorus over L/R/Sur s32 buffers via `do_src1/2` asm + sinc resamp table; reason-1 applies `parameterUpdate.para[]` (:133-189) | **fail-loud once** `UNIMPLEMENTED(chorus): sndAuxCallbackChorus …` via `OSReport` (decl `libs/dolphin/include/dolphin/os.h:41`, bringup def `host/bringup/osinit.c:57` wires it to vprintf); reason dispatch kept era-shaped; reason-0 = dry pass-through (zero audio buffers touched), reason-1 records `para[4]` to static state without applying; `info`/`user` NULL-tolerant (era derefs unguarded) |
| `sndAuxCallbackPrepareChorus` (`SND_AUX_CHORUS *chorus` → `u32`) | `salMalloc(0x1680)` delay line, bank wiring `+0x1e0`/`+0xa0` strides (:211-220), zero 0x140 slots, seed `currentLast=1`/`trigger=480`/`target=0` (:231-236), inline update-settings formula returning 1 (:196-203) | loud-once report, then the **era-shaped state fill over module statics** (static `s32 sDelayLine[3*0x1e0]` = the same 0x1680 B block, pointer wiring and seeds verbatim per lines above), returns era success `1` |

Signatures are verbatim `musyx.h:166-167` — the selftest binds them through
musyx.h's own typedef `SND_AUX_CALLBACK` (musyx.h:32), so signature drift is
a compile error, not a runtime discovery. Era-shaped static state means a
caller doing only prepare + register + field-query (exactly the
`SoundControlState.cpp:974-980` sequence) runs to completion.

Deviations rostered in-file: **D1** — era `updateSettings` divides by
`pitchOffsetPeriod*5` unguarded (`period<=4` → zero-divide); the stand-in
writes `pitchOffset=0` instead. **D2** — `chorusStandIn{PrepareCalls,
CallbackCalls,Reset}` are port-side observability helpers, NOT golden, NOT
era names; delete with the stand-in.

## 3. Verification transcript (scratch `$TMPDIR/p2am_scratch.*`, decomp read-only)

Selftest TU (`compat/chorus/chorus_stand-in_selftest.c`) compiles in **both**
dialect modes (era `sqrt`/`sqrtf` shim leak in `compat/pso_metal_compat.h`
only bites when two DIALECT `-x c` objects link together — AF §2 already
notes the leak; mode-2 is compile-proof only, mode-1 is the run):

```
$ clang++ -c -std=c++17 -x c++ $DIALECT $COMMON -target arm64-apple-macos11 …   # standin + selftest, T pattern
$ clang   -c -std=c11    -x c   $DIALECT $COMMON -target arm64-apple-macos11 …   # pure-C variant (pshim shadow musyx.h)
compile-both-modes-ok
$ clang++ -o selftest_cpp standin_cpp.o selftest_cpp.o main_shim.o && ./selftest_cpp
UNIMPLEMENTED(chorus): sndAuxCallbackPrepareChorus — compat/chorus/chorus_stand-in.c stand-in, real body is chorus_fx.c (docs/p2-chorus-standin.md)
UNIMPLEMENTED(chorus): sndAuxCallbackChorus — compat/chorus/chorus_stand-in.c stand-in, real body is chorus_fx.c (docs/p2-chorus-standin.md)
UNIMPLEMENTED(chorus): sndAuxCallbackChorus unexpected reason — …
UNIMPLEMENTED(chorus): reason=9 ignored by stand-in
CHORUS STAND-IN SELFTEST: PASS (signatures accepted by musyx.h client; 12 checks)
SELFTEST-RUN(caller -x c++ T-pattern) exit=0
```

(`main_shim.o` = scratch-only `int main(void){return chorus_stand_in_selftest();}`
plus an OSReport→vprintf def standing in for `host/bringup/osinit.c:57`.)

Gate — AF §2 battery verbatim (7 real `.o`, clang++ T pattern) + my
`chorus_stand-in.o` in one dir, **zero scratch stand-in hacks of AF's**:

```
== link_hygiene musyx-strict: …/p2am_scratch.*/bat_am
   objs=8 defined_globals=100 tsv=/Users/scott/Hermes/pso-metal/data/musyx-linknames.tsv
-- (a) C++mangled defined globals: 0 violation(s) (of 0 distinct __Z)
-- (b) golden names: func 24/24 as T, data 3/3 as D/B/S/R
-- (c) duplicate T definitions: 0 symbol name(s) [WARNING only]
SUMMARY objs=8 mangled=0/0 golden=24/24+3/3 dupT=0 verdict=PASS
GATE exit=0
```

`nm -g --defined-only` on the `-x c` standin object: `_sndAuxCallbackChorus`
/ `_sndAuxCallbackPrepareChorus` type `T`, unmangled → EC plumbing correct in
both modes.

## 4. Audio-phase removal checklist (replace-site hunt)

When chorus_fx gets properly translated port-side, this file goes away:

1. Find the replace-site: the `.o` rule that builds
   `src/musyx/runtime/Chorus/chorus_fx.c` into the musyx archive/obj dir —
   today that rule cannot exist (§1); it appears the moment the translated
   TU lands (watch `data/musyx-linknames.tsv` rows 2–3 "defined-by" column
   flipping from the GPOE asm stubs to a host TU, and the flip pool in
   `compat/dialect/flip_test.sh` gaining `chorus_fx`).
2. Remove `compat/chorus/chorus_stand-in.{c,h}` from the build battery in
   the same commit — if you forget, gate (c) flags the 2 dup-T golden names
   and (if the real TU isn't in the scanned dir) §3's dup-link error fires
   first.
3. Delete the `chorusStandIn*` observability helpers (D2) — grep
   `chorusStandIn` must go to zero hits, harness counters rewired to the
   real prepare return.
4. Re-run the two §3 commands: gate must stay `24/24+3/3 PASS` with the real
   `chorus_fx.o` in place of `chorus_stand-in.o`, and the selftest's 12
   state checks must move onto the real prepare (its post-prepare work-state
   is bit-comparable — the stand-in seeds are verbatim-era).
5. Only THEN does the lane-AK verdict line for `SoundControlState.cpp:978-979`
   change from "stood-in (P2-AM)" to "real"; update this doc's §2 table.

## 5. Boundaries

Touched: the 3 `compat/chorus/` files + this doc only. `tools/link_hygiene.sh`
and `data/musyx-linknames.tsv` were RUN/read, not edited (AF closed). No
writes to `host/`, `build/`, `data/`, or the decomp tree.
