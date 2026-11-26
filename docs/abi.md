# pso-metal Phase-0 — ABI autopsy: GameCube PPC32/MWCC vs Apple arm64/clang

Status: Phase-0 final analysis. All arm64_32 numbers below were **measured** on this
machine (Apple clang 21.0.0, target `arm64_32-apple-watchos7.0`, compat shim
`pso-metal/compat/pso_metal_compat.h` + `-fms-extensions`) against the READ-ONLY
decomp at `/Users/scott/Hermes/pso-decomp/pso-decomp`. PPC numbers are the
`LAYOUT_ASSERT` constants themselves — ground truth because the decomp builds and
matches against the original MWCC objects. Repro commands in §7.

All file:line references are relative to the decomp root.

---

## 0. TL;DR

* The tree has **3256 `LAYOUT_ASSERT`s across 1377 files** (grep of `LAYOUT_ASSERT(`).
  Under arm64_32 + compat shim, **3213 of them already pass.** Only **43 assert lines,
  in ~20 distinct types, in 15 headers** fail (1680-file probe: 1089 files fail, of
  which **913 fail ONLY from these layout asserts**; +21 fail on layout AND other
  syntax errors).
* **The double-alignment hypothesis is NOT the cause for this codebase.** Zero of the
  43 failures are attributable to `double`/`long long` alignment. The PSO structs are
  f32-only (25 `f64` occurrences in `src/pso`/`include/pso`, mostly Newton–Raphson
  temporaries calling `__frsqrte`); the few struct members that are `f64`
  (`AdxEncChannel.inv_scale`, `AdxEncoder.block_inv_scale`) happen to sit at
  8-compatible offsets — **measured pass** (0x90 and member@0x80/0x298 on both).
* The two real causes are **MWCC C++ object model** differences:
  * **Cause A — vptr placement** (~70% of failure traffic): MWCC (this build) puts the
    vptr *after the data members of the first polymorphic class*; clang/Itanium puts it
    at offset 0. Every member of `TObject` — the ancestor of the entire scene graph —
    shifts +4.
  * **Cause B — pointer-to-member-function size** (~30%): MWCC PMF is a 12-byte,
    3-word Cfront-style record (ptr/delta/voffset); clang PMF is **8 bytes**. Measured:
    `sizeof(void (T::*)()) == 8` on arm64_32 (`abi/rules_probe.cpp:6`, clang note
    `expression evaluates to '8 == 4'`), vs the decomp's asserted `== 0xC` at
    `include/pso/ui/window/WindowCtrlCallback.h:9-10`.
* **`long` is not the problem** (4B on both — confirmed by the ILP32 improvement:
  probe fails went 1614 [LP64+compat] → 1089 [ILP32+compat]). Pointers are 4B on both.
  `long double` occurs **zero times** in the tree (grep), and is 8 bytes on both
  anyway (measured `sizeof(long double)==8` pass on arm64_32, `abi/rules_probe.cpp:12`).
* The **real port hazard is not compile-blocking at all**: 21 bitfield declarations and
  a handful of byte-index-into-multibyte-field accesses compile fine and are **bit-flipped
  at runtime** (§5). Size asserts cannot catch these because the *sizes* are identical.

---

## 1. Precise ABI diff table

| # | Rule | PPC32 (MWCC 2.6, `-align powerpc`, this decomp) | Apple arm64_32 (clang 21) | Observed in pso-decomp? |
|---|------|--------------------------------------------------|---------------------------|--------------------------|
| 1 | pointer, `int`, `long` | 4 | 4 | no diff (ILP32 choice validated) |
| 2 | `long long` / `u64` | 8B size, **4B align** | 8B size, **8B align** | latent only — every s64 member measured is coincidentally 8-aligned (`KeyboardChannelState.delay@0x28` pass; `TPlayerBattleCtrl.start_time@0x30` pass) |
| 3 | `double` / `f64` | 8B size, **4B align** | 8B size, **8B align** | latent only — `AdxEncChannel`/`AdxEncoder` measured pass (0x90 / 0x2D0) |
| 4 | `long double` | 8 (f64), align 4 | 8, align 8 | n/a — 0 occurrences (grep) |
| 5 | **vptr placement** | after data of the class that first declares `virtual` (`TObject` vptr@0x18, data m_name@0x0) | **offset 0** of the most-derived object | **Cause A.** `TObject` m_name measured @0x4, m_down @0x18 (all +4) |
| 6 | **PMF representation** | 12 bytes {ptr, delta, voffset}, `sizeof==0xC` asserted tree-wide | 8 bytes {ptr, delta} | **Cause B.** measured 8; tables of PMFs shrink 0x34→0x24 |
| 7 | bitfield packing within unit | MSB-first (declared-first gets highest bit) | LSB-first | **silent** (§5) — sizes equal, so no assert fires |
| 8 | struct alignment / trailing pad | max member alignment (≤4 with `-align powerpc`) | max member alignment (≤8 if 64-bit scalar present) | feeds #2/#3: latent only |
| 9 | base-class data vs derived data | derived members start at end of base *including its vptr slot* | same for a shared single vptr; deltas only inside the first-polymorphic base | explains why `TSetEvtDestroy.m_event_record@0x1C` **passes on both** while `TObject`'s own members fail |

Worked examples (one real failing struct per rule, expected-PPC → measured-arm64_32):

* **#5** `include/pso/player/PlayerActionPaletteState.h:25`
  `LAYOUT_ASSERT(PlayerActionPaletteTagOffset, offsetof(PlayerActionPaletteState, tag) == 4)`
  Struct (lines 8–23): `u32 disabled_slots; ActionPaletteTag tag;` + virtual dtor.
  PPC: `disabled_slots@0, tag@4, vptr@8, size 0xC`. arm64_32 measured: `vptr@0,
  tag@8, size 0xC` → size passes, **offsetof fails**. Fires in **823 TUs** (traffic #1 —
  reached via the umbrella `pso.h` chain).
* **#6** `include/pso/boss/darkfalz/TBoss4DarkFalz.h:137-142`
  `TBoss4DarkFalzState { const char* name; 4× TBoss4DarkFalzHandler; }`
  PPC arithmetic: 4 + 4×12 = 0x34 (enter@4, leave@0x10, update_leader@0x1C, update@0x28 —
  exactly the asserted offsets). arm64_32 measured: 4 + 4×8 = **0x24** (enter@4,
  leave@0xC, update_leader@0x14, update@0x1C). Arithmetic closes to the byte.
* **#2/#3 (latent)** `include/pso/audio/AdxEncoder.h` `f64 inv_scale` after
  `u8 pad_7e[2]` at 0x7E: PPC align-4 → @0x80; arm64_32 align-8 → @0x80 (pad already
  got it there) → **measured pass** (0x80, size 0x90). Had the pad been one u8 shorter
  this would have failed. Same luck at `KeyboardDeviceDriver.h:41` (`delay@0x28`).

---

## 2. Fix strategy per cause

**First, decide what must match at all.** pso-metal is a *native recompile*, not an
objdiff-parity rebuild: the host compiler fabricates vtables and PMFs from its own ABI,
so layout parity is required only where a struct **overlays original bytes** —
`gsl`/`gct`/`lgt` archive records, `.evt` streams, memory-card saves
(`include/pso/system/save/*`), BBA network packets (`include/pso/network/**`), and any
cross-module pointer table loaded from data. Pure in-memory class graphs (TObject
subtree, windows, bosses) need *consistent host layout*, not *PPC layout*.

| Cause | Fix | Tradeoff / notes |
|-------|-----|------------------|
| **A. vptr@0 vs vptr@end** | (a) For behavioral port: accept clang layout; recompute/retire the offsetof-asserts of virtual classes. (b) For overlay-critical classes: drop `virtual`, hold an explicit `void *_vptr` member at the PPC offset, and replace dispatch with host function-pointer tables — the layout then pins via plain fields. | There is **no clang flag** to move the vptr (unlike MWCC's `-vpo`). `packed`/`#pragma pack` does **not** help — packing affects padding, not vptr position or PMF width. Option (b) costs the `virtual` niceties but keeps sizes provable; option (a) is free but means those asserts must be edited out — a policy the decomp (read-only!) forbids upstream, so pso-metal keeps its **own** copies of headers with corrected constants (compat strategy from Phase-0 commit `4a907a1`). |
| **B. 12B vs 8B PMF** | The state/callback tables are *compiled into code* (e.g. `dark_falz_state_table`, `boss6_plot_falz_script_states[9]`, `boss6_type2_battle_actions[36]`) and never persisted → accept clang's 8B PMF, re-derive the size/offset asserts. Where a table must be byte-identical, wrap with `struct MwPmf { void *ptr; u32 delta; u32 voffset; };` + a cast shim. | MWCC's {ptr,delta,voffset} and clang's {ptr,delta} are semantically equivalent under **single inheritance**, which every failing table uses (checked: all are `void (T::*)()` on singly-derived classes). If a future unit stores PMFs *inside loaded data*, treat it as cause-A option (b) — explicit field, no compiler PMF. |
| **C. 64-bit scalar alignment (latent)** | If a future wire/blob struct trips it: `#pragma pack(push,4)` around the struct (clang supports it; it reproduces `-align powerpc`'s max-4 scalar rule) or insert explicit `u8 pad[]` fields (the decomp already does this style). | packed drops the struct's own alignment → array stride also becomes PPC-sized (correct here). Cost: unaligned `u64` loads on ARM64 (handled by HW; avoid `std::atomic` on those fields). Do **not** blanket-pack the tree: 3213 asserts already pass, and packing polymorphic classes would break clang's vptr assumptions for no gain. |
| **D. trailing/overall alignment** | Follows from C automatically once 64-bit scalars are handled; no independent failures observed. | — |
| Phase-0 build unblock (cheap, immediate) | In pso-metal's own include layer, `#undef LAYOUT_ASSERT` and redefine it to a host-checked form (the sentinel `static_assert` trick from §7 did exactly this, keeping the checks *live* with corrected constants). | A blanket no-op unblocks the 913 size-only files instantly but **hides cause C forever and §5-adjacent size bugs**; re-asserting with computed host constants costs nothing and keeps the guards meaningful. |

---

## 3. ⚠️ ENDIANNESS — the silent-corruption class (NOT fixed by ILP32, NOT caught by any assert)

Nothing in this section produces a compile error. All structs below keep identical
**sizes** under both ABIs; only the *bit/byte meaning* changes. This is the real port
hazard for pso-metal.

**Grep census** (`^\s*<type> <name>\s*:\s*N;`): **21 bitfield declarations total**, in
exactly 4 files:

| Location | Declaration | Why it breaks silently |
|----------|-------------|------------------------|
| `include/pso/network/NetTcpState.h:92-94` | `u8 f204_b7:1; u8 f204_b6:1; u8 f204_rest:6;` | PPC allocates b7 at **bit 7**, clang LE allocates b7 at **bit 0** of the same byte. The field *names* encode the PPC bit positions (b7/b6), and the neighboring comment (`NetTcpState.h:60-61`) documents mask-style access (`0x40`, `0x80`) for that TCB region — so raw-mask code paths and bitfield code paths will **disagree on which bit is which**. Users: `src/pso/network/NetTcp.c:443,2566,2577`, `src/pso/network/SocketProtocolRuntime.c:22,135,326,549`. Self-consistent *only if* nothing touches that byte via masks or raw copies — audit required. **Fix: replace all three with one `u8 f204;` + shift macros pinned to BE semantics (`#define F204_B7(b) (((b)>>7)&1)`), or renumber for LE — one or the other, never mixed.** |
| `include/pso/effect/TEF_LensFlare.h:25` | `struct { signed int a : 8; u8 r, g, b; } argb;` | The `a:8` occupies the **MSB byte** of the int on PPC; clang puts it in the **LSB byte** (hypothesis from the two ABIs' allocation rules; not runtime-verified here). `src/pso/effect/TEF_LensFlare.cpp:95-104` fills `glow.r/g/b/a` then does `__memcpy(&sprite->color, &glow, sizeof(glow))` — a **raw byte copy of a bitfield struct into a colour word**. On LE alpha lands in the wrong byte ⇒ wrong colour, silently, every frame. Fix: make `argb` four plain `u8`s in explicit ARGB byte order, drop the bitfield. |
| `libs/dolphin/src/si/SIBios.c:286-297` (12 fields) and `libs/dolphin/src/os/OSCache.c:539-543` (5 fields) | `u32` bitfields overlaying **memory-mapped BE hardware registers** (SI CSR, DMA CPR) | These only work because the register reads are BE. pso-metal never maps these registers (host stubs per Phase-1 plan), so they are moot *as code*, but **must not be copy-pasted** into host drivers. 17 of the 21 bitfields are in this dead category; 4 are live hazards above. |

**Explicit byte-indexing into multi-byte fields** (the other endianness vector):

* `src/pso/ui/frontend/TAdChrMoveEffectGC.cpp:54,58,62` —
  `reinterpret_cast<u8 *>(&desc.col[i].color)[3] = e->entry.alpha;`
  On PPC byte `[3]` of the BE word is the **LSB**; on LE it is the **MSB**. This line
  writes the alpha into a *different channel* on the host. Fix: shift/mask arithmetic
  on the u32 with an explicit channel encoding, or a `u8[4]` field.
* Counterweight (good news): the decomp is already endianness-honest where it matters —
  it models byte order with **named swap helpers** that compile to identical
  arithmetic on any host: `include/pso/player/TPlyGuildCardTag.h:24`
  (`gc_tag::bswap()`), `include/pso/ui/TMenuList.h:13` (bswap macro),
  `include/pso/ui/chat/SymbolChatPreset.h:33`, `.../TWindowSubSymbolChat.h:54`,
  `include/pso/ui/window/WordSelectMessage.h:22,37`,
  `src/pso/unk/text_803DD5E0.cpp:243` `bswap32_in_place()` applied to archive-header
  `magic`/`size` fields (lines 368–437). **Audit rule for the port: every struct that
  has a `bswap` routine is BE wire format on purpose — keep the helpers, fix only
  implicit vectors (bitfields, `(u8*)&` casts, `>>`/`&` masks that assume bit
  numbering).** Grep gates for Phase 1: `: *[0-9]+;` in struct bodies,
  `(u8 ?\*)&` / `reinterpret_cast<u8 ?\*>` into non-u8 objects, and any mask constants
  (`0x80`, `0x40`, …) applied to bytes that bitfields also describe.
* `long double`: zero occurrences tree-wide — non-issue.

---

## 4. Top-10 highest-traffic failing structs (fix in this order)

Traffic = number of TUs whose ilp32 probe run reported that assert site
(`cache/scratch/clangprobe/ilp32/*.err`); sites truncated by clang's 20-error limit
are undercounted, so these are floors.

| Rank | Type(s) | Site (decomp file:line) | TU traffic | Assert (PPC truth) | arm64_32 measured | Cause / why it fails |
|------|---------|--------------------------|-----------|--------------------|-------------------|----------------------|
| 1 | `PlayerActionPaletteState` | `include/pso/player/PlayerActionPaletteState.h:25` | **823** | `offsetof(tag)==4` | tag@**8**, size 0xC (size ok) | A: clang vptr@0 pushes `disabled_slots`,`tag` +4 |
| 2 | `MaterialColorFade` | `include/pso/render/MaterialColorFade.h:27,28` | **208** | color@0, rate@0x10, size 0x18 (ok) | color@**4**, rate@**0x14** | A: own vptr before members; NJS_ARGB is 4×f32 (`include/pso/system/NinjaTypes.h:76-81`) so no 64-bit involvement |
| 3 | `RenderFadeState` | `include/pso/render/RenderFadeState.h:23` | **144** | inherited color@0 | color@4 | A: via base; note `restoring@0x18` and size 0x1C pass (base is size-identical — rule #9) |
| 4 | `PlayerWindowCommand/Query` (PMFs) | `include/pso/ui/window/WindowCtrlCallback.h:9,10` | **119** | `sizeof(PMF)==0xC` | **8** | B: MWCC 3-word PMF vs clang 2-word |
| 5 | `TBoss4DarkFalzHandler/State` | `include/pso/boss/darkfalz/TBoss4DarkFalz.h:137,138,140,141,142` | **51** | hdlr 0xC; state 0x34; leave@0x10, leader@0x1C, update@0x28 | 8; 0x24; 0xC/0x14/0x1C | B: 4×PMF × −4 bytes |
| 6 | `TWindowHelpItem` | `include/pso/ui/window/TWindowHelpItem.h:187,188` | **36** | size 0xBC; m_name@0 | 0xB8 (`m_callback` PMF −4); m_name@4 | **B + A** mixed: shrink from the `void (TWindowHelpItem::*m_callback)()` member, shift from TObject vptr; `active_item@0x68` passes |
| 7 | `PlacementQueue` | `include/pso/player/TPlyItem.h:109` | **18** | `offsetof(slots)==4` | slots@**8** (`positions@0x28` coincidentally still passes) | A: `virtual ~PlacementQueue` moves vptr 0x24→0x0, absorbing at both ends |
| 8 | `TBoss4Type3State` | `include/pso/boss/darkfalz/TBoss4Type3.h:167,169,170` | **16** | 0x2C; advance@0x14, update@0x20 | 0x20, 0x10, 0x18 | B (enter@0x8 passes) |
| 9 | `TWindowLockOn` | `include/pso/ui/window/TWindowLockOn.h:37` | **12** | size 0x7C | **0x78** | B: single 12→8 PMF member (`refresh`, @0x70); A shifts members but not the total (vptr was already in base object) |
| 10 | `CreationMenuEntry` | `include/pso/ui/charcreate/TObjCMMainMenu.h:19` | **11** | size 0x10 | **0xC** | B: `kind`(4)+PMF(12); `select@4` passes |

Runners-up (same classes, low traffic): `TSetEvtDestroy.h:70-76` — 7 asserts pinning
*TObject's own* member offsets (A; 6 TUs, and the template for every TObject assert the
port will add); `TItemDisconnectCtrl.h:21` (A); `TObjDFRecover.h:27` (A);
`TSaveChCr.h:65-67`, `TBoss6PlotFalzState.h:17-23`, `TBoss6Type2BattleAction.h:19-22`,
`TAdTimeSetGC.cpp:85-86`, `TCheckMemoryCard.cpp:35-36` (all pure B).

**Impact summary:** fixing Cause A's *policy* (declare host layout authoritative for
virtual classes) retires ~1486 assert-hits across 913 otherwise-clean files; fixing
Cause B's (PMF tables rebuilt host-side) retires ~626 hits / ~200 files; zero files need
Cause C. Both together = the entire Phase-0 size-assert blocker.

---

## 5. Hypotheses vs measurements (honesty ledger)

* **Measured** (this machine, §7): all arm64_32 sizes/offsets/PMF/vptr/f64/u64/long-double
  numbers; the 43-site failure census; file classification (913/21/77).
* **Ground truth by construction**: PPC constants = the decomp's own asserts (build-verified
  against MWCC objects upstream; we did not rebuild the decomp).
* **Hypothesis** (labelled): (i) MWCC `-align powerpc` semantics as max-4 alignment for
  64-bit scalars — inferred from the decomp's pad fields (`_pad36[2]`-style) and assert
  constants being 4-multiples even around f64; no PPC cross-compiler exists on this host
  (Apple clang 21 ships no `powerpc` target, confirmed via `-print-targets`) so it was
  not independently re-measured. (ii) `int:8` MSB-byte placement on PPC (§3 LensFlare row).
  (iii) Firing sites hidden behind clang's 20-error/TU limit → site counts are lower
  bounds; per-file totals (913/21/77) are exact.

---

## 6. What the port phase should actually do (order of attack)

1. Adopt pso-metal-local header overrides with **host constants** for the ~20 types
   above (keep the assert machinery, change the numbers) → 913 files parse-clean
   immediately (verified: the 913-file set has *no other error class*).
2. Land the compat shim's PMF policy: tables use clang PMFs; add `static_assert`s
   pinning `sizeof(void (T::*)()) == 8` once, tree-wide, so nobody re-adds 0xC constants.
3. Run the §3 audit greps as a CI gate *before* feature work touches netcode or
   effect code; convert the 4 live bitfields and 3 byte-cast sites to explicit
   shift/mask.
4. Keep `#pragma pack(push,4)` in reserve for wire structs that later trip 64-bit
   alignment; do not apply preemptively.

## 7. Repro

```sh
SC=/Users/scott/.hermes/profiles/pso-decomp/cache/scratch/clangprobe
# tree-wide probe (outputs in $SC/ilp32/*.err, fail list ilp32/fail.txt):
bash $SC/run_compat.sh          # lp64 + ilp32 with pso-metal compat shim + -fms-extensions
python3 $SC/analyze_ilp32.py    # site census + file classification
# exact arm64_32 measurements (sentinel static_assert trick):
cd $SC/abi && clang++ -fsyntax-only -ferror-limit=0 -std=c++17 -w \
  -target arm64_32-apple-watchos7.0 -fms-extensions \
  -include /Users/scott/Hermes/pso-metal/compat/pso_metal_compat.h \
  -I<decomp>/include -I<decomp>/include/clib -I<decomp>/libs/dolphin/include \
  -I<decomp>/libs/dolphin/include/dolphin -I<decomp>/src layout_probe3.cpp
# clang's 'expression evaluates to N == 12648430' notes carry the true values.
```
