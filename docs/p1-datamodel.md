# P1-J — Data-model spike: u32-as-pointer census + arena-handle proof

Context: Phase 1 must run the guest body on Apple clang arm64 **LP64**
(arm64_32 was parse-probe only — it cannot execute on macOS, phase0-verdict.md).
This lane measures where the decomp's data model collides with LP64, proves the
replacement scheme in a runnable spike, and states the verdict.

Decomp snapshot: `pso-decomp @ 6b38b5383`, measured over `src/` + `include/`
(READ-ONLY; musyx included, it is part of the shipped body).

## 1. Census (exact commands, run in /Users/scott/Hermes/pso-decomp/pso-decomp)

### 1a. Absolute-address literals `0x80[0-9A-Fa-f]{6}`

```
$ git grep -Eoin '0x80[0-9A-Fa-f]{6}' -- src include | wc -l
203
```

Classified over the same output (Python, per-value Counter):

| class | count | notes |
|---|---|---|
| sign/top-bit flags & masks | 130 | 101× `0x80000000`, pow-2 bit families, `0x80808030` … — **not addresses** |
| hash/entropy magic | 32 | `0x80C7C7C7`, `0x80035BAB` … — constants, not addresses |
| address-shaped | 41 | below |
| — vtable-placement *comments* | ~14 | `// places the vtable at 0x804C3F60` (TForceOperate.h:9 et al.) — no code |
| — **live absolute loads** | 9 sites | `*(u32 *)0x800000F8` = BootInfo timebase-frequency word (NetworkRuntime.c:140,145; NetworkThreadRuntime.c:58; TPlayerBattleCtrl.cpp:353; TPlayerBattleCtrl4P.cpp:280; SoundControlState.cpp:913; hw_dolphin.c:132; hw_dspctrl.c:277; QuestDownloadServer.h:134) — **one host glue class kills all 9** |
| — laundered data constants | ~18 | `(void *)0x808c1210` table entries in TLobby_800F8AA8.cpp (hash tables), `reserved00 = 0x808C0000` (ModelDrawContext.cpp:150, GX command word) — values that merely look like addresses |

**Verdict-grade number: only 9 live absolute-address dereferences tree-wide.**

### 1b. u32-typed fields / ints used as pointers

The decomp has already institutionalized the seam:
`include/types.h:15-17` — `/* An integer that holds an address: 32 bits on the
console, a pointer's width in a port. */ typedef u32 uaddr; typedef s32 saddr;`

```
$ git grep -ow uaddr -- src include | wc -l          # uaddr tokens
718
$ git grep -Fo '(uaddr)' -- src include | wc -l      # (uaddr) casts, occurrences
407
$ git grep -En 'uaddr[[:space:]]+[A-Za-z_][A-Za-z0-9_]*[[:space:]]*(=[^=]|;)' -- src include | wc -l
140                                                  # uaddr-typed fields/vars decls
$ git grep -Fo '(u32)' -- src include | wc -l        # (u32) casts, occurrences
779
$ git grep -En '\(u32\)[[:space:]]*this' -- src include | wc -l
26                                                   # raw this-pointer laundered through (u32)
$ git grep -F '(u32)&' -- src include | wc -l
0
$ git grep -En '\(uaddr\)[[:space:]]*this' -- src include | wc -l
45                                                   # this-pointer laundered through (uaddr)
```

Classification of the 407 `(uaddr)` sites:

* **Arena/pool-slot handles** — fixed-width ints naming slots/offsets that get
  base+translated: `NetBufferRef.value` `/* kinds 2 and 3: a pool slot handle
  (entry << 24 | segment << 16) */` (include/pso/network/BufferRef.h:9, with a
  union against a real `void *data` — exactly the format/runtime split);
  `uaddr player;` (TItemTimerEvent.h:17), `uaddr model_tree;` (TODoorFourLight*.h:14),
  `uaddr attachment_model;` (TObjPlayer.h:321), quest-VM `qst_label_address`
  taking `(void *)(uaddr)(u16)g->id` (QuestOpcodeHandlers.cpp:1417) — a 16-bit
  ID smuggled in pointer dress.
* **Raw-pointer laundering** — host-usable addresses stored/compared as int:
  45 `(uaddr)this` + 26 `(u32)this` sites, 92 laundered null tests
  (`(uaddr)x ==/!= 0`), table-of-addresses → `(u8 *)(uaddr) table[i]`
  (TOSymbolchatColli.cpp:96), int→pointer back-casts `(T*)((uaddr)…)` (17 sites).
* **Not host pointers at all** — DSP/ARAM hardware address space:
  hw_dspctrl.c alone holds 79 `(uaddr)` lines (`(uaddr)dsp_vptr->smp_info.addr`);
  these are guest-device addresses → host-glue problem, not C data-model.
* The 779 `(u32)` casts are overwhelmingly pure integer work (bit packing in
  synthmacros.c, protocol arithmetic in NetTcp.c); genuine pointer use inside
  `(u32)` is the 26 `(u32)this` sites — the rest of the repo consistently
  chose `uaddr`. **The `(u32)`-vs-`(uaddr)` census says the decomp's own
  convention already separates the two roles.**

### 1c. Pointer arithmetic stored as int

```
$ git grep -En '\(uaddr\)[^;]*[-+][^;]*|[-+][^;]*\(uaddr\)' -- src include | sort -u | wc -l
182                                                  # lines doing int-space address math
$ git grep -En '\([A-Za-z_][A-Za-z0-9_]*[[:space:]]*\*[[:space:]]*\)[[:space:]]*\([[:space:]]*\(uaddr\)' -- src include | wc -l
17                                                   # (T*)((uaddr)base + n) back-casts
$ git grep -En '\(uaddr\)[^;=]*-[^(]*\(uaddr\)' -- src include | wc -l
15                                                   # (uaddr)a - (uaddr)b distances
```

Exemplars: NetworkRuntime.c:89-106 `(NetworkMutexSlot *)((uaddr)alarm_base +
(MUTEX_SLOTS_OFFSET + k*sizeof(NetworkMutexSlot)))` (arena slot walk);
hw_dspctrl.c:406 block size `((uaddr)(dspCmdPtr+4) - (uaddr)dspCmdCurBase + 3) & ~3`.
Every distance fits a ≤26 MB arena, so the *values* are 32-bit-representable;
what must be pointer-width on LP64 is the *intermediate type* — precisely what
types.h's uaddr comment mandates.

## 2. Host spike (LP64): `spike/datamodel.c` — compiles, RUNS, exit 0

```
cc -std=c11 -Wall -Wextra -o $TMPDIR/datamodel spike/datamodel.c && $TMPDIR/datamodel
```

Models the GC 24 MB main arena + 2 MB ARAM as two `malloc`'d blocks behind base
registers ("base register" = one static `uint8_t*` per guest address space; a
real arm64 port can pin it to a callee-saved global or, later, an actual
register-resident TLS base). Proves u32-handle ↔ base+handle round-trip incl.
the BufferRef composite `entry<<24|segment<<16` decode, and fires the
ILP32-assumption assert set as XFAIL. Real run output (Apple M3 Ultra-class
arm64 host, 2026-11-26, EXIT=0, reproduced twice):

```
== P1-J data-model spike (host: LP64, sizeof(void*)=8, sizeof(long)=8)

-- (0) LP64 traps in the era typedefs --
  [XFAIL] sizeof(unsigned long)==4 (decomp `typedef unsigned long u32`) : does NOT hold
  [PASS] u32g fixed-width binding keeps GcRecord at 16B : holds

-- (1) ILP32 assumption set the guest body relies on --
  [XFAIL] sizeof(void*)==4                               : does NOT hold
  [XFAIL] sizeof(u32g)==sizeof(void*)                    : does NOT hold
  [XFAIL] pointer difference fits in u32g without truncation : does NOT hold

-- (2) GC arenas as malloc'd blocks behind base registers --
  [PASS] 24MB main arena + 2MB ARAM malloc'd            : holds
  arena base = 0x746e400000  aram base = 0x77cb400000

-- (3) stored u32 handle -> base+handle round-trip --
  [PASS] handle fits in 32 bits (< 24MB)                : holds
  [PASS] base+handle lands back on the record           : holds
  [PASS] runtime view rebuilds from handle              : holds
  [PASS] composite entry<<24|segment<<16 handle decodes inside ARAM : holds
  slot handle 0x03050000 -> aram offset 0x50300

-- (4) what BREAKS if this body assumes 4-byte pointers --
  real ptr = 0x104944B38   after u32 round-trip = 0x4944B38
  [XFAIL] pointer survives u32 laundering (guest assumption) : does NOT hold
  [PASS] arena-relative handle survives for in-arena objects : holds

-- verdict inputs --
  must-pass checks failed : 0
  ILP32 assumptions observed broken on LP64 (XFAIL): 5
  expected-to-break checks that held (should be 0) : 0
```

Note the addresses: even the `malloc`'d arena sits at 0x74_6e400000 (43-bit)
and text at 0x10_4944B38 — the truncation demo is *structural* on macOS arm64,
not luck: any laundered host pointer loses its top bits (`0x104944B38 →
0x4944B38`). Handle round-trip through the same width is exact.

## 3. Verdict — the port's data model

**Hybrid, three-rules scheme — and the decomp already leans this way:**

1. **Fixed-width ints for every format-crossing field.** `u32/u16/u8` rebind to
   `<stdint>` (never `unsigned long` — spike §0 trap; that single typedef
   change would 2× every field and detonate all 3071 `LAYOUT_ASSERT` uses).
   Save/GBA/network/disc-format pointers stay as **u32 handles + per-space base
   register** (24 MB arena, 2 MB ARAM): values ≤26 MB always fit; spike §3
   proves the round-trip, BufferRef-style composite handles included.
2. **`uaddr`/`saddr` = `uintptr_t` in the port** (its documented contract), used
   for all runtime-only address arithmetic. The 182 int-space-arithmetic lines
   and 17 back-casts become correct-by-construction under this binding; keep
   real `T*` only in runtime-only structs (never serialized).
3. **Per-site fixes required — the census says this is small:** 26 `(u32)this`
   launderings (retype to `uaddr` or a handle), 9 `*(u32*)0x800000F8` BootInfo
   loads (host clock/glue), ~18 look-like-address data constants in TLobby/GX
   tables (review, probably inert), ~79 hw_dspctrl DSP-address-space sites
   (audio-glue lane, not data-model).

**Implication for the FORMAT-KEEP side of the assert policy:**
`LAYOUT_ASSERT` (types.h:69, compile-time `typedef char name[cond?1:-1]`, 3071
uses) keeps **layout** but is blind to **value corruption**: with `u32` fixed at
4 bytes, a `(u32)this` site compiles and passes every layout assert while
silently truncating on LP64 — same silent-corruption shape as the endianness
class (abi.md §3). Therefore: (a) the 3071 asserts are the enforcement that
uaddr must **not** be widened *inside format structs* (widening it in
`NetBufferRef` turns 8→16 and fails at compile time — the assert does its job);
(b) the assert policy gains a data-model arm — any file whose format structs
still carry pointer *values* in u32 fields must carry a `sizeof(void*)==4`
trip-wire (never true on the LP64 host) or be converted; the 26+9 site list is
the conversion queue, so **prefer conversion over trip-wires** and keep
pointer-width `uaddr` strictly in runtime-only code.
