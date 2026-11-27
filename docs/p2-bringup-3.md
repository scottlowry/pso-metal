# P2 bringup-3 — boot ladder rung 3 (lane AE, 2026-11-26)

Pure-logic rung over the era decomp @ 6b38b5383 (read-only; `run.sh` exit 0,
55 goldens = Q 16/16 + Y 13/13 UNMODIFIED + AE 26/26). Everything executed is
either a verbatim generated slice (guards in `bringup.mk`), a previously
landed era object file, or a harness whose shape is provenanced to era PPC
disassembly (`build/GPOE8P/asm/pso/...`), instruction-quoted at each golden.

## 1. Roster — functions now EXECUTING on arm64

| # | era fn (home) | era src | slice / link | golden |
|---|---------------|---------|--------------|--------|
| A | `qst_op_call_image_data` @0x801EB5AC | `src/pso/quest/QuestOpcodeHandlers.cpp:2278-2286` | `gen/ladder3_quest_glue.cpp` (mk-anchored verbatim awk slice, body-line guards) | A2/A3 |
| A | `resource_decompress_buffer` global (SystemBootInit.cpp:86) alloc pattern `xmalloc` :283 | `src/pso/ui/keyboard/SystemBootInit.cpp` | quest prelude + Y's `memory.o` | A0 (cell requested 0x2000, OSAlloc.s 0x80370BEC) |
| A | PRS wrapper `prs_decompress_buffer` @0x800F7DAC + era PrsDecoder | TScreenFadeController.cpp:309 / prs.cpp | (Y's `prs_fade_glue.o`, `prs_glue.o` — unchanged) | A2/B4 |
| B | `find_gsl_entry` @0x800FDD70 | `src/pso/system/TGStream.cpp:207-219` | `gen/ladder3_gsl_glue.cpp` verbatim slice | B0-B3 (gate/hit/stride/miss paths, each .s-addr'd) |
| B | `GslArchiveEntry` table read off fake_disc via Y's `dvd_shim` (`gsl_tab.dat`, 3×0x30) | `include/pso/system/GslArchive.h:5-17` | `disc_read` (test TU) + shim | B1/B2/L0 (stride constant = `0x800FDDCC addi r31,r31,0x30`) |
| C | `TLogo::run_task` @0x8013F294, ctor @0x8013F54C, dtor, render | `src/pso/ui/TLogo.cpp:126-199` | `gen/ladder3_logo_glue.cpp` verbatim slice | C0-C12 |
| C | predicates `is_logo_clear_flag_set`/`is_vi_config_answered`/`is_sega_logo_done`/`is_logo_exit_requested`/`is_adx_logo_done` @0x8013F7E8-0x8013F80C | `src/pso/ui/TLogo.cpp:57-63` | same slice | C1/C2/C3/C4/C5/C9 |
| C | `request_main_phase` @0x8000F9CC + `reset_main_phase_state` | `SystemBootInit.cpp` (sbi slice block) | same glue TU | C6/C7 (stw next@+4 :F9D8 / result=1@+0x10 :F9E0) |

New files: `host/bringup/ladder3_test.cpp`, `include/ladder3_harness.h`,
`include/ladder3_{logo,quest,gsl}_prelude.h`, `fake_disc/gsl_tab.dat`,
`fake_disc/title_prs.dat`, mk rung-3 section, run.sh rung-3 section.
Every assertion in the binary carries the addr+instruction it derives from
(see the CHECK strings); nothing asserts a value the code under test computed
for it (decoded length is re-measured against the hand G3 trace, disc
`decoded_size` against wrapper return, etc.).

## 2. What drove how far

- **Quest chain:** qst_args[8] (4-byte stride per :EB5C4/C8) → era caller
  resolves the label **through qst_code/qst_function_table** exactly as
  0x801F2C1C-:C2C, decompresses a synthetic PRS stream **into the real
  `resource_decompress_buffer` global**, and hands that global +
  args[0].i to the sin-board call site (:EB5E4-E8); decoded bytes are read
  back at the caller dst and byte[9] is proven untouched (0xCC canary).
- **GSL config chain:** fake_disc → DVD shim (Y) → 96-byte entry table →
  era `find_gsl_entry` hit/miss/gate paths → entry.offset/size drive a
  second disc read of `title_prs.dat` → era wrapper → real buffer →
  `decoded_size` field agrees with measured output.
- **TLogo machine:** full chain 0→4→5→6→7 (check_progressive_ready=0 leg),
  VI-dialog leg 0→1→4 (AB's A3 hazard route: logo_clear_flag=1), modem legs
  both ways from a hand-entered state 2 (3 and 4). Object-slot stores,
  queue_destruction count (3 by state 7), fade-callback identity
  (SEGA/Sonic `request_destroy` vs ADX `fade_in_done`), and init_logo_state
  args (0 then 1) all match the quoted .s.

## 3. GX wall (the exact fn)

**`TLogo::run_frame` — first executed bl is `begin_gx_frame` at
`TLogo.s 0x8013F5F4` (`48 29 D8 09`), TLogo.cpp:97-99.** The state machine
itself is pure through **state 7 = last pure-logic state** (its branch at
0x8013F4D8/DC issues `request_main_phase(0xA)` and stores nothing else);
past 7 the phase switch advances the era frame driver, whose prologue opens
a GX frame before any logic — the ladder cannot take another logo-era step
without the GX substrate (lane Z territory). Named secondary walls:
`load_sin_board_image` (era continuation of the quest path reaches
`load_texture_if_valid` texture upload; call site 0x801EB5EC — recorded at
the seam here) and `TLogo::check_progressive_ready` (VI hardware — already
modeled by AB in `host/gx/gx_vi.c`; injected per DA-1).

## 4. Deviations (all host-side harness, never inside a sliced body)

- **DA-1** `TLogo::check_progressive_ready` injected (AB models the VI/OS
  bodies); era call site 0x8013F2CC keeps its bl in the slice.
- **DA-2** `qst_label_address` def lives in another era fragment; body
  reproduced instruction-for-instruction (return is void*: LP64 can't carry
  a pointer in the era `int`, 0x801F2C1C provenance in the test header).
- **DA-3** `load_sin_board_image` replaced by a call-site recorder (GX wall
  §3); the era caller passing the real global is still era code.
- **DA-4** `gsl_archive_entries` is filled via Y's DVD shim + memcpy
  (era `open_gsl_archive` needs ADXF); the table bytes and the scan are
  era-shaped/era-verbatim.
- **DA-5** `resource_decompress_buffer` sized 0x2000 (era 0x80000,
  SystemBootInit.cpp:283) to keep the ladder heap carve small; allocation
  path + header asserts are era rules.
- **DA-6** host `QuestValue` drops `void *p` to keep the .s-proven 4-byte
  stride; `TAdVIConfigGC` carries a 4-byte pad so `answered` lands on the
  era deref offset 0x48 that the verbatim predicate uses.

## 5. Discipline notes

- `run.sh`: boot_spine + 16/16 + 13/13 lines byte-identical to the P2-Y
  ladder-2 output; rung 3 appended; exit 0 only when all four sections pass.
  Verified from `make clean` full rebuild (this session) and an incremental
  re-run. `build/` untouched except regenerated rung-3 objects;
  `gen_waivers.py` never invoked (D8 stamp guard consumed as-is); no
  gx_window.mk, no host/gx/, no host/link, no tools/ paths touched;
  decomp tree read-only.
- fake_disc grows two synthetic files; `logo_e.prs`/`title.cfg` unchanged
  (backward-clean for AG's link lane which already points at this dir).
- Transcript recovery: this run resumed from the wedged predecessor's
  inventory (deleg_a0458f7d task-0.log, 17:24 freeze) — the function list
  (resource_decompress_buffer callers incl. BmlResource/SwitcherLaunch,
  TGStream/GslArchive config path, TLogo run_task .s) carried over as-is.
