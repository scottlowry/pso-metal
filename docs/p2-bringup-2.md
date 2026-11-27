# P2-Y — boot ladder 2 (PRS wrappers + era heap machinery)

Date: 2026-11-26 (PST). Decomp snapshot `pso-decomp @ 6b38b5383` READ-ONLY,
never written; its `make` never invoked. Staged ONLY `host/bringup/**` +
this doc. Builds on P2-Q (`docs/p2-bringup-1.md`) — Q's run.sh sections all
still execute; nothing replaced, everything grown.

## 0. RESULT

One command: `host/bringup/run.sh` → **exit 0**. Q's PRS golden 16/16
regression-green + 13 new ladder cases = **29/29**, FAIL lines: 0.
`all` target now builds THREE binaries (boot_spine, prs_golden_test,
ladder_test).

## 1. Inventory — where the targets actually live (git grep @ 6b38b5383)

| Function | Home (file:line) | Era .text |
|---|---|---|
| `prs_decompress_buffer(const void*, void*)` | `src/pso/system/TScreenFadeController.cpp:309` | 0x800F7DAC (`.fn prs_decompress_buffer__FPCvPv`) |
| `prs_decompress_alloc(const void*, void*, s32)` | `src/pso/system/TScreenFadeController.cpp:321` | 0x800F7D30 (`.fn prs_decompress_alloc__FPCvPvl`) |
| `fill_memory` | `src/pso/system/memory.cpp:17` | 0x803DE7E0 |
| `init_system_heap_and_global_allocator` | `src/pso/system/memory.cpp:26` | 0x803DE804 |
| `xmalloc` | `src/pso/system/memory.cpp:34` | 0x803DE838 |
| `xcalloc` | `src/pso/system/memory.cpp:39` | 0x803DE864 |
| `xfree` | `src/pso/system/memory.cpp:45` | 0x803DE890 |
| `sys_alloc` | `src/pso/system/memory.cpp:52` | 0x803DE8BC |
| `free_to_sys_heap` | `src/pso/system/memory.cpp:59` | 0x803DE8E4 |
| `sys_calloc` | `src/pso/system/memory.cpp:64` | 0x803DE90C |
| `sys_heap_realloc` | `src/pso/system/memory.cpp:71` | 0x803DE960 |
| `move_data` (realloc's copier) | `src/pso/unk/TLobby_803DE778.cpp:8` | (in TLobby unit) |
| OSAlloc allocator (what memory.cpp calls) | `libs/dolphin/src/os/OSAlloc.c` | `.text 0x80370A4C..0x80370D5C`: DLInsert 0x80370A4C, OSAllocFromHeap 0x80370AF8, OSFreeToHeap 0x80370BF4, OSSetCurrentHeap 0x80370C70, OSInitAlloc 0x80370C80, OSCreateHeap 0x80370CF0 |

Q's suspicion confirmed: the two PRS wrappers were NOT in prs.cpp — they are
C-callable free-function wrappers in the screen-fade unit that each do
`new PrsDecoder; prs->prs_decompress(dst, src, 0); delete; return size`
(prs.h:45-46 declarations; 27 call sites across item/quest/bml/lobby units).

## 2. What executes now (new since P2-Q)

- **Verbatim decomp units**: `memory.cpp` (all 10 functions above) compiled
  straight from the read-only tree. `prs_decompress_buffer` +
  `prs_decompress_alloc` run from a **line-range verbatim slice**
  (`build/bringup/gen/prs_fade_glue.cpp` = TScreenFadeController.cpp
  lines 309..331, anchored + brace-counted by guards in bringup.mk — same
  drift-refuses-to-build discipline as Q's prs_glue). `move_data` likewise
  sliced verbatim from TLobby_803DE778.cpp:8-10.
- **Era allocator port**: `host/bringup/osalloc_era.c` — 1:1 port of
  OSAlloc.c semantics (HEADERSIZE 32, ALIGNMENT 32, MINOBJSIZE 64
  take/split rule, address-sorted DLInsert merge-on-free, first-fit scan,
  headerBytes/paddingBytes/payloadBytes bookkeeping) replacing Q's
  "deliberately simple" shim internals, which Q explicitly earmarked for
  this step. Entry-point names, `__OSCurrHeap`, and Q's `OSCheckHeap`
  live-count contract are byte-compatible for callers, so Q's three heap
  goldens pass UNMODIFIED against the era-mechanics allocator.
- **End-to-end shapes proven on fake data**: DVD→arena→`prs_decompress_buffer`
  into an `xmalloc(0x2000)` heap block (the era `resource_decompress_buffer`
  pattern, SystemBootInit.cpp:283); `init_system_heap_and_global_allocator`
  installing `global_allocator = &sys_allocator_func_table` over a real
  6 MB sys heap inside the Q arena, then xmalloc/xcalloc/xfree/
  sys_heap_realloc managing real blocks with headers read back.

## 3. Golden provenance (per assertion — none derived from the C under test)

| Case | Oracle hand-derived from |
|---|---|
| W1 decode `ABCDABCDA` via wrapper | decode bytes: Q's prs.s trace (0x8036F5FC/650/660/6C0/6EC); delegation shape: TScreenFadeController.s 0x800F7DAC `mr r4,r29 / mr r5,r28 / li r6,0` — wrapper passes (dst,src,0) to `prs_decompress__10PrsDecoderFPUcPUcl` 0x8036F5B0 |
| W2 NEW vector `ABCABC` (stream `47 'A' 'B' 'C' FD 01 00 00`) | ctrl 0x47 = 0b01000111: b0..2 literals A,B,C; b3=0 end-run; b4=0 near, a=b5=0 b=b6=1 → size=(0<<1)+1+2=3 (8036F6A4-6AC); offset byte 0xFD → off=−3, +n=0 (8036F6BC/6C0) → copy abs[0..2]; ctrl 0x01 b0=1 far, `00 00` → or.==0 END (8036F654). ret 6 |
| W3 alloc(limit=1) writes ALL bytes | .s 0x800F7D30: r5 (=size_limit) saved by `stmw r28,0x10(r1)`, NEVER reloaded before the epilogue `lmw`; the decompress call takes `li r6,0` (0x800F7D74). The era discards the limit — verified dead, not reimplemented |
| W4 disc→wrapper→`ABCDABCDA` | logo_e.prs bytes == W1's hand-encoded stream; wrapper shape as W1 |
| W5 error −1 / 0xFFFFFFFF | prs.s epilogue `li r3,-1` 0x8036F710; result kept `mr r30,r3` and returned `mr r3,r30` (0x800F7E10) — u32 return type wraps to 0xFFFFFFFF in buffer, s32 keeps −1 in alloc (signature diff matches decomp C + .s mangled names) |
| L0/H1 | memory.cpp:8-9,26-29 + memory.s `.sbss 0x805D77A8/0x805D77AC` (two 4-byte words = sys_heap_handle + global_allocator); H1 also compares 4 table slots vs function symbols (AllocatorCallbacks.h order: allocate/release/allocate_zeroed/reallocate) |
| H2 header `0x420/0x400/hd` | size arithmetic done HERE: (0x400+0x20+0x1F)&~0x1F = 0x420; payload@cell+32: OSAlloc.s 0x80370BEC `addi r3,r6,0x20`; 32B align: `clrrwi rX,rY,5` 0x80370CCC/CE4/CFC/D08; hd==&HeapArray[heap]: OSAlloc.c `cell->hd = hd` |
| H3 zero-fill + header 0x420/0x400 | memory.cpp:64-67 (`OSAllocFromHeap(h,count*size); fill_memory(p,0,count*size)`) + same .s facts |
| H4 realloc | memory.cpp:71-76 (alloc→`move_data(p,ptr,size)`→free) + OSFreeToHeap clears `cell->hd` (OSAlloc.c:588 region, .s 0x80370BF4) |
| H5 coalesce+first-fit | DLInsert .s 0x80370A4C = address-sorted insert merging BOTH neighbours when `(u8*)cell+size==next` / `prev+prev->size==cell`; three freed 0x220-cells → one 0x660 cell covering block a → 0x620 request splits it at a → `d == a` |
| H6/H7 | Q's OSCheckHeap count contract (§4 D10); oversize → era NULL-return path (no panic) |

## 4. Deviations (continuing Q's D-numbering)

* **D8 — waiver regen removed from bringup.mk.** The integrator has a staged
  waiver-stitch pending in `build/**`; `gen_waivers.py` is no longer invoked
  by any bringup path. The committed `build/waivers/pso_assert_waivers.h`
  (99 ILP32-targeted LAYOUT_ASSERT waivers) is force-included AS-IS; the
  stamp rule now fail-closed VERIFIES `STATE: 99 waived` instead of
  regenerating.
* **D9 — d4-follow-through facade:** first port-side `dolphin` header
  (`host/bringup/include/dolphin/os/OSAlloc.h`) — a declaration shim that
  forwards to `pso_bringup.h`'s OSAlloc-contract decls so
  `memory.cpp`'s `#include <dolphin/os/OSAlloc.h>` resolves WITHOUT the era
  dolphin include tree (which shadowed host libc — Q D4). It mirrors the
  real header's declarations field-for-field.
* **D10 — OSCheckHeap stays Q's live-allocation count.** Era truth: no
  such function exists in the retail text (OSAlloc.s exports stop at
  OSCreateHeap; OSAlloc.c carries no OSCheckHeap definition) — it is a
  port-side helper, so Q's counter contract (and its 3 heap goldens) carry
  over onto the era allocator unchanged.
* **D11 — era OSAlloc.c not compiled wholesale; ported.** `libs/dolphin/src/os/OSAlloc.c`
  cannot execute unmodified on LP64: pointer-width `hd` grows Cell 20B→40B
  > HEADERSIZE 32, so `requested` (era +16) WOULD spill 8 bytes into the
  payload — measured live (the first ladder run failed exactly this way:
  `requested=0` after a `fill_memory`, header stomped). Fix keeps the era
  contract where it counts — 32-byte header, payload at cell+32, 32-byte
  alignment, field ORDER — encoding `hd` as `u32 heap+1` (0 = era null-free
  sentinel), i.e. the project's u32-handle model. A
  `sizeof(Cell)==HEADERSIZE` compile-time tripwire guards the layout.
  Era `(u32)` pointer casts → `uintptr_t`; `ASSERTMSGLINE` panic paths →
  `OSReport` + graceful return (V2 in the file header).
* Slice-glue rule: `prs_fade_glue.cpp` / `move_data_glue.cpp` are line-range
  verbatim extractions (anchor-signature count == 1, brace-count == 2, body
  line == memmove call) — any drift in the decomp sources refuses to build,
  exactly like Q's D3 roundtrip guard. Whole-TU compiles were rejected
  because TScreenFadeController.cpp drags GX quad/camera deps (step 8+
  territory) and TLobby_803DE778.cpp is a 4k-line lobby unit.
* Ladder heap region: sys heap carved at arena+0x1000000 (6 MB) rather than
  the literal SystemBootInit geometry `cfg.heap + 0x518000`, to guarantee
  non-aliasing with Q's heap 0 (arena+0x400060..0x918060). The CALL SHAPE
  is the era's (SystemBootInit.cpp:271-275: OSInitAlloc(...,2) →
  +0x518000 → init_system_heap_and_global_allocator(start, hi−start));
  Q's OSInit now also matches that shape (heapbase from OSInitAlloc,
  era-aligned, replaces the old raw+0x1000 skip).

## 5. run.sh transcript (clean `rm -rf build/bringup` rebuild)

```
== PRS golden vectors (oracle: hand-traced from era prs.s) ==
... Q's 16 cases all PASS, unmodified lines ...
== PRS golden + substrate: 16/16 PASS ==
prs_golden_test: EXIT=0

== P2-Y boot ladder: era PRS wrappers (TScreenFadeController) + era heap (memory.cpp over OSAlloc port) ==
PASS L0 memory.cpp: sys_heap_handle global is bss-zero (0) before init_system_heap_and_global_allocator
PASS H1 init_system_heap_and_global_allocator: heap=1, global_allocator == &sys_allocator_func_table, 4/4 slots (memory.s 0x803DE804)
PASS H2 xmalloc(0x400): cell@p-32 size=0x420 want 0x420, requested=0x400, hd_idx=2==heap+1 (&HeapArray[1] era mapping), payload 32-aligned at OSCreateHeap start (OSAlloc.s 0x80370BEC addi r3,r6,0x20)
PASS H3 xcalloc(4,0x100): 0x400 bytes zero-filled (fill_memory 0x803DE7E0), header 0x420/0x400
PASS H4 sys_heap_realloc(p,0x80): 0x80 pattern bytes moved (move_data=memmove, TLobby_803DE778.cpp:8), new requested=0x80, old cell->hd==0 (OSFreeToHeap 0x80370BF4 clears hd)
PASS H5 coalesce+first-fit: allocs adjacent (stride 0x220), free b,a,c merges, xmalloc(0x600) reuses block a (d==a: 1), cell 0x620/0x600 (DLInsert 0x80370A4C address-sorted merge)
PASS H6 OSCheckHeap: 2 -> 4 -> 2 (count +2 then balanced free)
PASS H7 xmalloc(0x601000) > heap -> NULL, next xmalloc(0x100) still succeeds
PASS W1 prs_decompress_buffer(G3): ret=9 out=ABCDABCDA into xmalloc'd dst (NOT the internal window) — .s 0x800F7DAC r4=dst r5=src r6=0
PASS W2 prs_decompress_buffer(L2 trace 0x47+0xFD): ret=6 out=ABCABC expect ABCABC
PASS W3 prs_decompress_alloc(limit=1): ret=6 wrote ALL 6 bytes — limit arg is dead in era .s (0x800F7D74 li r6,0; r5 never reloaded)
PASS W4 DVD->arena->prs_decompress_buffer: ret=9 out=ABCDABCDA (logo_e.prs bytes == G3 trace)
PASS W5 corrupt stream: alloc ret=-1, buffer u32 wrap=0xffffffff (s32 -1 through `mr r3,r30` at 0x800F7E10)
== P2-Y ladder: 13/13 PASS ==
ladder_test: EXIT=0

BRINGUP GREEN          (run.sh exit 0; 31 PASS / 0 FAIL total)
```

## 6. Next blocker toward the logo frame (exact)

Ledger steps 1-4, 6-7 (heap machinery), and 19's decode path are now
logical-execution-green. The logo frame itself is
**GX-blocked at step 5/8**, not logic-blocked: `init_video_and_gx(&cfg)`
(SystemBootInit.cpp:276) — VIInit/GXInit/framebuffer present — is the first
row requiring a Metal CAMetalLayer + display-link retrace counter; no pure
CPU row in front of it is unimplemented anymore. Pure-logic-able next rows
(no GX): step 9 `load_text_file`/config over the DVD shim, step 10 first
real asset loader using `prs_decompress_buffer` into
`resource_decompress_buffer` (now executable end-to-end from disc bytes to
decoded block), and step 15's text/logo-state logic. GX-free ladder can keep
growing; the FRAME cannot before step 5.
