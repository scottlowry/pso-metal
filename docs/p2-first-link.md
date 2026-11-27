# P2-AG — First Link (Milestone-A): real LP64 Mach-O containing era logic

Executed 2026-11-26 (resumed lane; predecessor AG transcript died at 17:24 in the
backend storm — inventory facts recovered from `deleg_bfbe584c/task-0.log`).
Dep-tree inputs: `docs/p2-link-deps.md` + `data/link-deps.tsv` (P2-U),
boot recipes `host/bringup/bringup.mk` (P2-Q/Y), decomp @ `6b38b5383` READ-ONLY.
Everything this lane wrote lives under `host/link/**` (+ this doc). No file under
`host/bringup/`, `host/gx/`, `build/`, or `tools/` was touched; objects were
rebuilt into `host/link/obj/` scratch, never `build/bringup/`. AF's
`tools/link_hygiene.sh` had not landed when this lane's inventory ran (the
17:24 transcript confirms); it committed at `3354fb0` during this lane's
execution but is musyx-golden-scoped, so the nm link-closure checks here stay
inlined in `host/link/link.mk` as the `gate`/`run` targets (reconciling the two
gates is AF's call).

## Verdict

**LINKED.** `host/link/out/pso_first_link` = Mach-O 64-bit executable arm64,
73,088 bytes, **zero non-system undefined** (ld default `-undefined error` is the
hard proof; the nm gate confirms the 26 remaining imports are all libSystem/libc++
— exactly P2-U's excluded system class). Boot slice runs **exit 0** with all
**8 RECEIPT lines green** (transcript below).

## 1. The dependency-closing set + selection rule

**Rule (task step 1):** start from the P2-Y boot-spine closure (bringup objects,
recipes mirrored verbatim into `host/link/link.mk` — never re-running `gen_waivers`;
the committed `build/waivers/pso_assert_waivers.h` is force-included as-is with a
`STATE: 99 waived` fail-closed guard). Then add every P2-U 70-pass-set TU
**whole-file** whose `nm -u` closes inside the growing set once the 9 scaffold
backings + the MSL `__upper_map` shim are admitted. Measured per-candidate on
2026-11-26 (compilation at U's exact object flags, `-target arm64-apple-macos11`):

| Pass-set TU | its `nm -u` | closes? |
|---|---|---|
| system/MiddlewareInterruptLock.cpp | OSDisableInterrupts, OSRestoreInterrupts | **IN** (backings) |
| unk/text_803DEE48.cpp | DVDChangeDir | **IN** (backing) |
| unk/text_803D9FF0.cpp | ___upper_map | **IN** (MSL shim) |
| system/memory.cpp (already in boot spine) | heap trio + move_data | **IN** (osalloc_era.o + slice) |
| boss/derolle/ProjectedColorDraw.cpp | GXBegin + **4 game-internal** (end_primitive_draw, setup_opaque/blended_color_draw, load_screen_space_matrix — defined in non-pass ScreenPrimitiveDraw.cpp) | OUT |
| system/TLanguageFontPalette.cpp | GXDrawDone, DCStoreRange + **2 game-internal** (init_and_load_tlut, load_gvpl_palette_file) | OUT |
| system/Lzss.cpp | **5 game-internal** (calc_delta, debug_print, get_coarse_time, coarse_time_usec, bswap_32) | OUT |

Slicing the def-TUs of those 11 game-internals would silently replace "first link"
with "second LP64 tranche" — the selection rule keeps the milestone honest.

Byte counts (era sources as read at 6b38b5383): memory.cpp 1,831 ·
MiddlewareInterruptLock.cpp 701 · text_803DEE48.cpp 236 · text_803D9FF0.cpp 352 ·
prs.cpp 2,859 (whole, D3 one-literal rebase) · TScreenFadeController.cpp 8,534
(23-line verbatim slice, 631 B) · TLobby_803DE778.cpp 555-line parent (3-line
`move_data` verbatim slice, 152 B) · MSL ctype.c tables extracted 3,660 B
(2×256, `_Static_assert(sizeof)==256` each). Link unit total: **15 objects**,
of which **5 whole-file era TUs** (memory, MiddlewareInterruptLock,
text_803DEE48, text_803D9FF0, prs.cpp-rebased) + **2 guarded verbatim era
slices** (prs_fade_glue, move_data_glue).

## 2. The 9 scaffold backings (+ ___upper_map)

| Symbol | Backed by | Kind |
|---|---|---|
| OSCreateHeap | `host/bringup/osalloc_era.c` (P2-Y era OSAlloc port) | **LINK of committed impl** |
| OSAllocFromHeap | same | LINK |
| OSFreeToHeap | same | LINK |
| OSDisableInterrupts | `host/link/host_backings.c` | new: prev-state mask on single-thread host; era sig `BOOL OSDisableInterrupts(void)` (OSInterrupt.h:154, BOOL=int at types.h:31) |
| OSRestoreInterrupts | `host/link/host_backings.c` | new: writes saved flag; nesting proven by RECEIPT 5 |
| DCStoreRange | `host/link/host_backings.c` | new: `dmb ish` store fence — psosh_os.h:81's own "unified cache: no-op except GPU-shared ranges" rationale; Metal-buffer path joins with Z's display-list lane |
| DVDChangeDir | `host/link/host_backings.c` | new **extension** (task-sanctioned): records cwd, rejects `..`, returns era TRUE; correct because Y's `dvd_shim resolve()` flattens the FST root — dir cannot change file mapping |
| GXBegin | `host/link/host_backings.c` | new: call-counter, **no GPU semantics**. Preferred backing host/gx/gx_shims.c cannot be linked: it references 4 `gxh_*` harness symbols whose impl is Z's UNTRACKED dl/ WIP (frozen e02706e, unverified) — linking that would be dishonest. Neither symbol is referenced by the chosen TU set. |
| GXDrawDone | same file | same rationale |
| ___upper_map (+___lower_map) | `host/link/gen/msl_maps.c` | build-time verbatim extraction from era MSL `ctype.c` (whole-file adoption would collide with host libc `isalpha` etc.) |

Note: the scaffold's `PSOSH_TAG(fn,…)` macro expands to `int fn(void)`
placeholders — it is a symbol inventory, not a prototype surface; the backings
follow the real era prototypes.

## 3. The ld command (reproducible: `make -f host/link/link.mk all run`)

```
clang++ -target arm64-apple-macos12.0 -o host/link/out/pso_first_link \
  host/link/obj/{boot_slice,crt0_nomain,osinit,osalloc_era,dvd_shim,prs_glue,\
  prs_support,prs_fade_glue,move_data_glue,memory,era_system_MiddlewareInterruptLock,\
  era_unk_text_803DEE48,era_unk_text_803D9FF0,host_backings,msl_maps}.o
```

(Verified re-runnable outside make: `LDCMD_RERUN_OK`. Deviation carried from
P2-Y: `new PrsDecoder` in the era wrapper binds libc++ `operator new` — era
`global new` rebinding waits for the allocator boot ordering.)

## 4. Transcript (gate + exe census + scripted boot slice)

```
GATE PASS: 15 objects; open=system-only; 9 scaffold backings + ___upper_map all defined
linked: /Users/scott/Hermes/pso-metal/host/link/out/pso_first_link
exe undefined imports: 26, non-system: 0 (must be 0 — ld -undefined error already proves it)
RECEIPT 1 arena: base=0x105048000 24MB+2MB, sysret.busFreq=32419584, prs_window=0x105049800
RECEIPT 2 heap-spine: OSAllocFromHeap(0x4560)->0x105448080, live 0->1->0 (era OSAlloc port)
dvd_shim: fake bundle root '/Users/scott/Hermes/pso-metal/host/bringup/fake_disc' -> drive READY
RECEIPT 3 fake_disc: logo_e.prs read 9/9 bytes via pread bundle
RECEIPT 4 prs-chain: prs_decompress_alloc -> 9 bytes "ABCDABCDA" (era wrapper+era decoder)
RECEIPT 5 irq-gate: middleware lock enter x2/leave x2, nested mask honored
RECEIPT 6 sys-heap: init_system_heap_and_global_allocator carve + sys_alloc/xmalloc pair drained via era callback table (live 2->0)
RECEIPT 7 dvd-chdir: change_disc_directory("/files/item/") -> cwd recorded
RECEIPT 8 ctype-table: convert_string_chars_via_table("pso-logo test!") -> "PSO-LOGO TEST!"
== P2-AG first-link boot slice: ALL RECEIPTS OK (0 failures) ==
exit 0
```

The full era→era chain that executes inside the exe: boot_slice (host) →
`prs_decompress_alloc` (era TScreenFadeController slice) → `PrsDecoder::
prs_decompress` (era prs.cpp, D3 rebase) — four separate era TUs calling each
other in one arm64 process, not isolated port-side reimplementations.

## 5. What this proves / does not prove

PROVES: (a) era TUs at their verbatim decomp source compile+LINK into one LP64
Mach-O with honest backing for exactly the 9+1 symbols U's census says the early
surface needs; (b) the LP64 assert/waiver pipeline (not the host API surface)
remains the critical path: 11 of the 12 near-miss gaps were game-internals;
(c) the boot spine chain arena→fake_disc→PRS→heap now runs in a MULTI-TU link
(P2-Y's ladder was a hand-curated object list; this one is symbol-gated);
(d) the interrupt-lock and ctype-table era logic is behaviourally correct under
their minimal backings (nested-mask and mapping receipts).

DOES NOT PROVE: any GX/VI/frame behaviour (no frame producer linked — GX backings
are unreferenced counters); disc-chdir path semantics beyond the flat bundle;
multithreaded interrupt-mask semantics; that the 560 game-internal gap cluster
shrinks by anything — the logo chain still runs through the TAd*/TLanguage TUs
that fail LP64.

## 6. Delta first-link → logo frame (fn list, evidence-anchored)

Blocking set, ordered by criticality (refs: p2-defer-audit W1–W6, U census top rows):

1. **LP64 pass expansion**: TAdVIConfigGC / TLogo / TAdSEGAlogo / TLanguage +
   font/GVPL loaders are NOT in the 70-pass set — every logo state fn
   (`TLogo::run_task/run_frame/state-change`, `TAdVIConfigGC::dialog/calc`,
   fonts `load_fonts/load_text_resources` — W4) is inside the 560
   game-internal cluster. QuestOpcodeHandlers.cpp alone unblocks 91% of one
   branch's table traffic (U doc).
2. `init_and_load_tlut(long,long,long,void*)`, `load_gvpl_palette_file(char*,GvplPalette*)`
   — TLanguageFontPalette's two non-scaffold gaps (font palette path).
3. GX frame substrate: gx_shims' 4 `gxh_*` harness fns + GXBegin vertex FIFO
   (Z's dl/ — frozen unverified, run_dl.sh re-run mandatory per ledger).
4. VI chain (AB, committed, linkable): `VIGetDTVStatus`,
   `OS{Set,Get}ProgressiveMode` — W1 says logo-min MUST route VIGetDTVStatus→0
   until the progressive check is exercised.
5. PAD read for the logo second gate (A3 zero-input leg) — `PADInit/PADScan`
   (scaffold, no era refs yet in pass set).
6. `OSInit`→`init_game_systems` ordering + default sys-save (W3) and the
   `is_ready/wait_ready_loop` stub (W2, satisfied-by-construction in AB).
7. Title.cfg consumer + DVDChangeDir real path resolution once files are
   hierarchical on the bundle (extension seam left in host_backings.c).
