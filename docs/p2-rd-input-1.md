# P2-RDINPUT — R-D/R-E: controller-input single source + injection API (design only)

Lane RDPREP, 2026-11-27 (card `t_44d26780`). ZERO-BUILD design lane: no repo builds
run, no `gen_waivers`/`make parse` invoked, waiver pin `host/bringup/waiver-header.sha256`
never touched, **no `host/**` writes — proposal only**. Tip verified same-turn:
pso-metal `main` @ `09de0a8`. Era cites opened this turn through the read-only pin
`DECOMP_ROOT=/Users/scott/Hermes/pso-decomp/pso-decomp-port-pin` @ `6b38b53837`
(`src/...`/`include/...`/`config/...` paths below are relative to it). BRINGUP5 was
LIVE in the working tree at intake (`M host/bringup/include/ladder7_run_prelude.h`,
untracked `gen_ladder9_trial.py`/`ladder9_trial_prelude.h` seen via `git status`
only) — no file of that lane was edited, and its rung-6 trial-arrival state is the
base this design sits on. Inherits `docs/p2-rung7-seams-1.md` (RS) risk R-D/R-E;
this doc is the fix package the BRINGUP-6 build seat applies.

## 1. Era truth — how the real game feeds controller state

### 1.1 The two globals are DIFFERENT MAPPINGS of the same pads (the R-D law)

`include/pso/system/ControllerInputState.h:51-52` declares BOTH globals with the
SAME era type, defined adjacent in `src/pso/system/ControllerInput.cpp:39-40`:

| global | era def | era .bss addr (symbols.txt row) | meaning |
|---|---|---|---|
| `controller_input_state` | ControllerInput.cpp:39 | `.bss:0x8050E208` size 0x1B0 (psov3/symbols.txt:46358) | LOGICAL player slots |
| `controller_physical_port_bank` | ControllerInput.cpp:40 | `.bss:0x8050E3B8` size 0x1B0 (same file :46359) | PHYSICAL ports 0..3 |

The two addresses are exactly `0x1B0` apart — the era `.bss` placed them
definition-order adjacent. Any host promotion MUST keep the def order
(cis first, bank second) so the adjacency invariant survives a future
address-anchored leg. Layout (era header, LAYOUT_ASSERTs `:43-49`):
`sources[4] @0x0` (PadSlot*), `source_status[4] @0x10`, `controllers[4] @0x20`
(0x50 context), `keyboard @0x160`; total 0x1B0.

The semantic split lives in `bind_controller_devices()`
(ControllerInput.cpp:432, era `bind_controller_devices__Fv = .text:0x801A6510
size:0x1A8`):
- bank (always :445-454): slot i ← `get_pad_slot(controller_device_id_table[i])`
  — PHYSICAL 1:1, `source_status[i]=1` iff `*slot->conn & 1`, else the
  `controller_default_pad_state` sentinel (:458). Never reordered.
- cis (:457-483): if `is_split_screen()` — same physical 1:1; otherwise
  **compaction**: connected ports are gathered front-to-back into logical
  slots 0..count, the tail padded with defaults. This is why every
  `player->get_controller_port()` consumer reads `controller_input_state`
  and every attract-mode "port 0 is the physical controller" read uses the
  bank.

### 1.2 The supply chain (write side — nothing else writes the contexts)

Full era producer chain, every cite opened this turn:

```
dolphin PADRead()
  -> pad_status[4]                    src/pso/render/ChunkMaterialState.cpp:198 (def; extern decl :22-24)
  -> read_pads()                      ChunkMaterialState.cpp:43  (.text:0x803DEA18 size:0x15C, symbols :26771)
       memmove prev_pad_status, PADRead, diff = old^cur,
       held/notHeld/pressed/released = map_pad_buttons(...) (:59-70),
       stickX = scale_axis_value, stickY = -scale_axis_value, substick
       raw, trigL/R raw (:65-71); pad->err!=0 => *conn=0 + PADReset (:78-81)
  -> keyboard_control_pad_slots[4]    ChunkMaterialState.cpp:200 (def; PadSlot struct include/pso/ui/keyboard/TKeyBoardControl_803DE9C4.h:8-24, 0x34B; conn ptrs wired into pad_slot_state_buffer[0x78] by init_pad_slots_and_read :30-38, def at :201)
  -> get_pad_slot(device)             ChunkMaterialState.cpp:87 (.text:0x803DEB74, symbols :26772) — device codes 0/6/12/18 -> slots 0..3, anything else -> slot 0
  -> bind_controller_devices()        ControllerInput.cpp:432 (both banks' sources[]/source_status[])
  -> controller_stuff()               ControllerInput.cpp:278 (.text:0x801A5F1C size:0x5F4, symbols :11509)
       for bank in {0=cis, 1=bank} (:296-305), per context (:306-...):
       raw field copy (:309-322) with C-STICK Y FLIP `cstick_y = -slot->substickY` (:321),
       ±31.0f three-way deadzones ZEROING the s16 too (:323-339),
       magnitude = |stick|*0.0078125 clamped 1 (:340-354),
       0x30000 trigger-bit surgery + trig>0x70 synth buttons 0x10000/0x20000
       with prev-edge press/release (:355-378),
       held_with_sticks = pressed_with_sticks = buttons &= 0xC30FFFFF (:379-381),
       stick->directional-bit maps at ±0x30: main 0x100000-0x800000, C 0x04000000-0x20000000
       (map_main_stick_to_buttons :622 / map_c_stick_to_buttons :605 / set_stick_button_bits :595),
       stick_angle/cstick_angle = 10430.38*atan2 + camera yaw when camera_ctrl_instance
       != NULL (:387-400), update_button_hold_counters (:401; body :644, 14 slots of
       input_button_mask_table :17),
       disconnected => era DEFAULT-fill every field incl. not_held=0xFFFFFFFF and
       zeroed derived bits (:402-419); tail update_pad_recalibration() (:426; body :219)
  -> contexts + DERIVED fields (the only era writers of both globals' context arrays)
```

Keyboard mirror producer: `update_keyboard_input_frame()`
(ControllerInput.cpp:268, `.text:0x801A5EC4 size:0x58`, symbols :11508) —
`hardware_keyboard_state.process_key_reports()`, then ONLY if
`!is_split_screen()`: `controller_input_state.keyboard.poll_key_state()`
(body :563: rebuilds `buttons`/`pressed_buttons` from the 6 polled key codes
`keyboard_poll_key_codes[6] = {94,93,92,95,97,76}` (:31) via
`keyboard_read_input` (:86) + `TKeyBoardControl_key_bit_table[6] =
{16,32,64,128,4,2}` (:30), then copies both into
`held_with_sticks`/`pressed_with_sticks`) + `update_button_hold_counters`.
`hardware_keyboard_state` is `.bss:0x80506270` size 0xB0 (symbols :46274,
`KeyboardKeyState` shape `include/pso/ui/keyboard/KeyboardKeyState.h:14-49`).

Reset path: `reset_controller_input_state()` (ControllerInput.cpp:486,
`.text:0x801A66B8`, symbols :11511) default-fills cis.controllers[0..3] then
**copies controllers[0] OVER cis.keyboard** via `copy_controller_input`
(:514, `.text:0x801A67A4`) — a P1-pad snapshot into the keyboard context, not
a zero. Callers include `TAdMultiChgGameModeGC_8023E7A8.cpp:456` (scene
creation) — so rung-6/7 legs that re-create a scene silently reseed the
keyboard mirror from P1; an injection API that writes cis.keyboard before a
reset-triggered store loses the write.

### 1.3 The per-frame pump (rung-7 span, era order matters)

`run_game_mode_select_frame()` (TAdMultiChgGameModeGC_8023E7A8.cpp:494) fires
every frame: `begin_gx_frame(); controller_stuff(); update_keyboard_input_frame();
main_task.begin_frame(); ... render_tick();` (:510-520). Same sampler pair
also pumps from TLogo.cpp:100-101 (logo rung), TLoadCtrl.cpp:70-71,
TMovie.cpp:310-311, TCharSaveSequence.cpp:351, EndingSequence.cpp:52, plus
raw `controller_stuff()` calls in TTcpSocket/derolle/TLoadCtrl loops —
**46 call sites tree-wide** (same-turn grep count of
`controller_stuff();|update_keyboard_input_frame();` in `src/`). The 512-entry
`g_call_seq` event-vector law (RS ladder7 rung-5) is driven by exactly this pair.

### 1.4 The two-globals READ law (R-E, re-verified with census)

Consumers (same-turn grep over `src include` excluding the sampler TU
`ControllerInput.cpp` and the era header):
- `controller_physical_port_bank`: **66 read sites / 28 files** (all
  `.controllers[port]`; `.keyboard` slot of the bank is NEVER read era-wide —
  grep zero; it is era-dead data, do not inject into it).
- `controller_input_state`: **333 read sites / 111 files**, split three ways:
  `controller_input_state.controllers[...]` 181 sites / 81 files (logical
  player pads), `controller_input_state.keyboard` 130 sites / 48 files
  (mirror), `any_pad_input_active()` (ControllerInput.cpp:46) walks
  cis `.sources[]` PadSlots directly with ±10 thresholds.
- Single-line pad‖keyboard pairs (the R-E shape — same `if` reads bank or
  cis pad AND `controller_input_state.keyboard`): 7 exact same-line sites
  (TAdEnterPasswordGC.cpp:71/:84, TAdSinglePlyChrSelectGC.cpp:305,
  TAdSelectGameModeGC_802C4748.cpp:73, TAdYesNoBaseGC.cpp:72,
  TAdAgreementBaseGC.cpp:76, TTitleMenu.cpp:48) plus the multi-line twins
  (TAdEp3Exist.cpp:77-78, TAdLdQuestPrtGC.cpp:26-27, TAdEnterPPPPassGC.cpp:67-68,
  TAdTimeSetGC.cpp:906-907, TAdEnterSerialAccessGC.cpp:189-190,
  TAdWarningBaseGC.cpp:60-61, TAdSelectCurGC.cpp:140-146/158-159...). Within
  the rung-7 span files specifically: `TAdScrollWinGC.cpp:93` reads the BANK
  only; `TAdAgreementBaseGC.cpp:76` reads BANK pad ‖ cis KEYBOARD;
  `TAdSelectCurGC.cpp:131-159` reads cis CONTROLLERS (repeat-gate
  :131-132/:140-141/:145-146) ‖ cis KEYBOARD, and the BANK only for the
  A-confirm at :158; `TTitleMenu.cpp:48` reads BANK ‖ cis KEYBOARD;
  `TTitleScene.cpp:302` reads `hardware_keyboard_state` DIRECTLY (third
  source) + `any_pad_input_active()` (fourth access path, via cis.sources).
  **A rung-7 leg that injects only one of {bank, cis.controllers, cis.keyboard,
  hardware_keyboard_state} silently misses at least one gate — the R-E
  "silent miss" is four-way, not two-way.**
- Test primitives the legs gate on (ControllerInput.cpp, era addrs in
  psov3/symbols.txt): `pack_button_mask` :670 (0x801A6D10),
  `test_controller_flag_4c_color` :677 (0x801A6D60) =
  `pressed_with_sticks & all_direction_mask`, `test_pressed_buttons_packed`
  :681 (0x801A6DA0) = `pressed_with_sticks & pack_button_mask`,
  `test_previous_buttons_packed`/`_remapped` :690-694 =
  `held_with_sticks & ...`, `test_buttons_press_or_repeat*` :698-708 =
  hold_frames repeat law (delay 6, period 1). **Every consumer tests the
  DERIVED words, never raw `buttons`** — an injector that writes
  `pressed_buttons` without the prev-frame edge + stick-bit packing is
  invisible to the era reads. This is what forces injection at the PadSlot/
  PADStatus layer (§3), not at the context layer.

## 2. Host census (same-turn greps of pso-metal @09de0a8, tracked content)

`git grep` over `host/` for every input symbol:

| surface | status | cite |
|---|---|---|
| `ControllerInputState controller_physical_port_bank;` DEFINITION | gx VI lane only | `host/gx/vi_branch_driver.c:34` |
| struct twin | **PARAPHRASED, WRONG SHAPE** | `host/gx/vi_era_harness.h:42-46` |
| `controller_input_state` | **ZERO host defs/uses** | same-turn grep: no hit under `host/` |
| `hardware_keyboard_state` / `KeyboardKeyState` / `PadSlot` / `get_pad_slot` / `read_pads` | **ZERO under host/** | same-turn grep |
| `controller_stuff` + `update_keyboard_input_frame` | COUNTED STUBS, two independent definitions in two trees | bringup `host/bringup/ladder7_seam.cpp:121-122` (defs; roster name list :47; decls `include/ladder7_run_prelude.h:374-375`); gx dl `host/gx/dl/gx_logo_driver.cpp:88-89` (decls `ladder6_logo_prelude.h:77-78`) |
| era slice reader of the bank | `build/vi_branch/vi_era_slice.cpp:107` (GENERATED from `TAdVIConfigGC.cpp:231` by `host/gx/vi_slice_gen.py`, manifest `vi_slice_manifest.txt`) | reads `test_previous_buttons_packed(&controller_physical_port_bank.controllers[0], 2)` |
| VI stub of the test primitive | `vi_branch_driver.c:35-39` returns 0 "era zero-input answer" (audit A3 doctrine, `docs/p2-defer-audit.md`) | |

**The R-D collision is worse than two owners of one def — the existing owner's
type is already wrong.** `vi_era_harness.h:42` states
`struct ControllerInputContext { u8 raw[0x50]; }; struct ControllerInputState
{ controllers[4]; keyboard; }` = **0x190 bytes with `controllers` @0x0**, while
the era type is 0x1B0 with `sources`/`source_status` first (contexts @0x20,
keyboard @0x160). Its own comment (:35-40) cites the era header as if it were
faithful. It survives today ONLY because the VI slice passes
`&controllers[0]` as `void*` into a stub that ignores it (comment :37-38 admits
this). Any bringup TU that includes the era header and links the VI object
reads field offsets 0x20-0x3A past a 0x190-byte object: silent garbage, or a
link-clash if both trees define. Census-verified: no other host paraphrase of
these globals exists (table above), so there is exactly ONE offender to retire.

Suite ledger to keep byte-green (README `suites` row, integrator re-run @ecbc11c,
same-turn disk): `run.sh` GREEN 0 FAIL · hg3_walk 26/26 · HG-1 29/29 · HG-2
270/270 · ladder7 62/62 · display-list 210/210 · window FIXTURE 13/13 ·
GENUINE 13/13 · FRAMES 24/24 · frames+exit 32/32 · AV-1 10/10 · **VI both
branches** (7 `PROOF` keys asserted by `host/gx/run_vi.sh`: `dtv0`,
`dtv0/frame1`, `dtv1p1`, `dtv1p1/frame1`, `dtv1p0`, `dtv1p0/frame1`,
`auto/frame1`) · link 8/8.

## 3. Design — one era-shaped source + injection API

### 3.1 The seat

New **`host/input/`** directory (new files only — zero overlap with
BRINGUP5's `host/bringup/**` surface or the gx lane's `host/gx/**`):

```
host/input/
  include/pso_input.h        # THE single source: era shapes + externs + injection API
  input_state.c              # THE single definition TU (all data globals)
  gen_input_pump.py          # generator: verbatim slices, bringup awk-block grammar
  input_test.cpp             # canary suite (C0..C3 of §4)
  run_input.sh               # gate runner
  input.mk                   # builds bin/input_test; exports INPUT_OBJS for consumers
```

`pso_input.h` does NOT paraphrase anything: it `#include`s the era headers
through the existing pin-include regime (`$(DECOMP_ROOT)/include` is already
on the bringup include path via the waiver/stamp pattern of `bringup.mk`;
`LAYOUT_ASSERT`s in `ControllerInputState.h:43-49` are satisfied natively at
LP64 per the corpus — era sizes are all sub-pointer: no pointer-bearing
field crosses, verified same-turn by reading the header; if the build seat's
parse probe ever says otherwise, the seat's own waiver-adjacent include shim
mirrors the ladder6 precedent, never a type paraphrase).

Data DEFINED once, in `input_state.c`, preserving era def order + adjacency
law (§1.1):
`controller_input_state`, `controller_physical_port_bank`,
`controller_default_pad_state` (era initializer ControllerInput.cpp:28 —
`notHeld=0xFFFFFFFF` at word index 3), `input_button_mask_table[14]` (:17),
`controller_device_id_table[4] = {0,6,12,18}` (:33), `controller_polling_enabled`
(:35), `keyboard_control_pad_slots[4]` + `pad_status[4]` +
`prev_pad_status[4]` + `pad_slot_state_buffer[0x78]`
(ChunkMaterialState.cpp:198-201), `TKeyBoardControl_key_bit_table[6]` +
`keyboard_poll_key_codes[6]` (ControllerInput.cpp:30-31),
`hardware_keyboard_state` (KeyboardKeyState def site — `.bss:0x80506270`),
plus the era-zero recalibration trio (`pad_recalibration_counters[4]`,
`pad_recalibration_hold_frames[4]`, `controller_reset_requested`,
`keyboard_reset_button_pressed`, `lbl_805D5D08`). PADStatus + `pad.h` shapes
come from the existing host dolphin shim (`host/bringup/include/dolphin/`
already fronts SDK headers for the ladder; the input seat is the first
consumer needing `dolphin/pad.h` — the seat adds a PADStatus-only shim header
under `host/input/include/dolphin/` matching SDK field order, NEVER redefining
game structs).

### 3.2 Verbatim slices (generator legs, era addr pinned in manifest)

Same grammar as ladder3/7/9: awk sig→`^}` block, GUARD lines on hand-traced
anchors, sha256 slice manifest at the pin era shas. Legs, lowest-address-first:

1. `map_pad_buttons`/`scale_axis_value`/`clamp_pad_axes` +
   `pad_button_map_table[12]` (ChunkMaterialState.cpp:108+/decls :60-71 region)
2. `init_pad_slots_and_read` + `read_pads` + `get_pad_slot`
   (ChunkMaterialState.cpp:30/43/87) — the conn-word law `0x101 / 0` (:76/:80)
3. `bind_controller_devices` + `controller_stuff` +
   `update_button_hold_counter(s)` + `all_direction_mask`/`cstick_direction_mask`/
   `pack_button_mask` + the seven `test_*` primitives + `set_stick_button_bits`/
   `map_main_stick_to_buttons`/`map_c_stick_to_buttons` +
   `update_pad_recalibration` (ControllerInput.cpp:219,278,432,595-708)
4. `update_keyboard_input_frame` + `ControllerInputContext::poll_key_state` +
   `accumulate_key_bits` + `keyboard_read_input` (:86,268,563,585) + the
   `KeyboardKeyState` methods actually touched (`process_key_reports`,
   `get_pending_count`, `get_pending_key`, `is_key_pressed`, `has_any_key_pressed`)
5. `reset_controller_input_state` + `copy_controller_input` +
   `any_pad_input_active` (:46,486,514)

Counted externs inside those bodies (LB-3 fail-loud or witness, per roster
grammar): `PADRead`/`PADReset` (host shim: return the injected PADStatus set /
record the reset mask), `DVDCheckDisk`/`reset_system`/`memcard_access_active`
(poll_reset_request path — only if a later leg promotes it),
`camera_ctrl_instance` (host def = NULL ⇒ the era branch :397-400 computes
yaw-free angles — era-legal for a no-camera attract frame; disclose as a
deviation row), `is_split_screen` (host witness global, default 0 = era
attract truth), `newton_sqrtf`/`atan2_float` (math lane externs if absent —
same-turn host grep shows neither symbol defined under `host/`; the seat
either binds the existing era math slices or adds an era .s-traced golden
for both, NEVER libm substitution: `atan2_float` returns the era fixed-ish
s32 word, a libm drop-in silently changes bit patterns).

### 3.3 Injection API (the whole surface; consumers link, nobody re-defines)

```c
/* lifetime (tests, once per binary): */
void  pso_input_init(void);                    /* mirrors init_pad_slots_and_read +
                                                  bind semantics; leaves ALL slots err!=0
                                                  => era idle through the real chains */
/* raw pad injection — era-shaped, drives BOTH globals coherently: */
void  pso_input_pad_connect (s32 port0_3, int present);        /* sets pad_status[port].err
                                                  0 / 1 — the conn-word law runs in era code */
void  pso_input_pad_press   (s32 port0_3, u16 pad_button_bits);// PADStatus-native bits, NOT
                                                  game masks — map_pad_buttons runs era-verbatim */
void  pso_input_pad_axes    (s32 port0_3, s16 sx, s16 sy, s16 cx, s16 cy); /* raw PADStatus units;
                                                  deadzone/flip/scale all run in era code */
void  pso_input_pad_triggers(s32 port0_3, u8 l, u8 r);
void  pso_input_pump_frame  (void);            /* read_pads(); bind_controller_devices();
                                                  controller_stuff(); update_keyboard_input_frame();
                                                  EXACT era order (ChunkMaterialState pump contract:
                                                  one read_pads per frame — double-pumping within
                                                  a frame loses pressed-edges to prev_pad_status) */
/* keyboard mirror injection: */
void  pso_input_key_report  (const u8 *raw, u32 n); /* drives hardware_keyboard_state; the
                                                  cis.keyboard fields are produced ONLY by
                                                  update_keyboard_input_frame — poll_key_state
                                                  is edge-consumed (pending drained per frame) */
/* logical-slot helper (tests that want "player 1's logical context" under the
   compaction law of §1.1 without hand-replicating it): */
s32   pso_input_logical_slot(s32 physical_port);
```

Design law: **no setter writes any context field directly.** The API's entire
write face is the era raw layer (PADStatus + keyboard raw/pending + conn);
every derived word (`pressed_with_sticks`, hold_frames, angles, magnitudes,
edge semantics) comes from the verbatim samplers. That is what makes an
injected frame indistinguishable from an era frame at the 399 consumer sites,
and what makes the R-E four-source problem disappear by construction (one
pump feeds bank, cis, and keyboard mirror in the era's own order).

Who links whom: `input.mk` exports `INPUT_OBJS`; bringup `bringup.mk` and gx
`vi_branch.mk`/`gx_dl.mk` add `$INPUT_OBJS` to their link lines and
`-Ihost/input/include`. Era-slice TUs needing the samplers include
`pso_input.h` INSTEAD OF declaring stubs. Counted seam stubs keep their
names/l7_note behavior (§3.4 step 3) — the pump wrapper calls the renamed
era bodies (L8-1 rename-at-emit precedent, `docs/p2-hg3-1.md §3`) and fires
the LC7 notes itself.

### 3.4 Migration steps (ordered, for the build seat; gate-preservation per step)

**Step 0 (precondition, zero behavior):** parse-probe `ControllerInput.cpp`,
`ChunkMaterialState.cpp`, `KeyboardKeyState.cpp` under the seat's include+
waiver regime (`probes/tree_compile_probe.sh` shape; the slice cluster is
(b)-grade only if these parse). Record slice shas into a new
`input_slices.sha256` manifest pinned to `6b38b53837`. Gate: none (no file
moved).

**Step 1: add the seat standalone.** New `host/input/**` files +
`run_input.sh` wired into `run.sh` as an ADDITIVE section (run.sh
insert-only law per HG2 precedent). `make -f host/input/input.mk &&
./host/input/run_input.sh` proves §4 canaries C0-C3. Existing suites must be
byte-green **unchanged** (nothing was touched): run.sh GREEN, VI both
branches, logo suites. Waiver sha PRE==POST across every make event (pin
`host/bringup/waiver-header.sha256`, D8 verify-only pattern).

**Step 2: VI lane retires its owner-ship (the R-D fix proper).** Edits:
`vi_branch_driver.c` delete :34 def + :35-39 `test_previous_buttons_packed`
stub (real era primitive from the seat answers it; era idle path yields the
same 0 — same "not pressed" result, now PRODUCED not stubbed);
`vi_era_harness.h` delete :32-47 paraphrase block, `#include "pso_input.h"`;
`vi_branch.mk` add `$(INPUT_OBJS)` to both binaries. The generated
`vi_era_slice.cpp` body does NOT change (same expression, era-correct offsets
now) — manifest fnv must stay identical; if it moves, the slice regen drifted
and the step stops. Gates that must stay byte-green: `run_vi.sh` 7 PROOF keys
+ exit 0 (both binaries, all three routes). This is the step where the
0x190-wrong-shape twin dies; after it exactly one definition exists repo-wide
(re-assert with the §2 census grep, expect the same 4 rows minus the two
paraphrase rows).

**Step 3: bringup pump flip (rides BRINGUP5's accepted tree — sequence AFTER
its disposition commit; do NOT hand-edit its rung-6 files in flight).** In
that lane's next rung: `ladder7_run_prelude.h` :374-375 + `ladder7_seam.cpp`
:121-122 + `gx_logo_driver.cpp` :88-89 keep the seam NAMES but their bodies
become wrappers: `l7_note(...)`, counters, then `pso_input_pump_frame()` —
gated by an `int pso_input_pump_live` flag defaulting **0** so every existing
test's 29-event vector, `g_ctrl_calls`/`g_kbd_calls` and the L7-I3a order
invariant read byte-identical; HG3-286+ legs flip the flag in their probes.
Gates: ladder7 62/62, hg3_walk 26/26, run.sh GREEN all unchanged with the
flag OFF; with the flag ON, a fixed-seed 6-frame walk must reproduce the
era-idle reads of §4 C0. (Alternative shape if the integrator prefers:
rename-at-emit the era slice and point only rung-7 generators at it — either
keeps step-2's single-def law.)

**Step 4: consumption.** BRINGUP-6 legs bind per RS §3: HG3-286
`pso_input_pad_axes(0, 0,0, 0, d)` (note: era flips substickY — inject the
PADStatus-side value so `bank.controllers[0].cstick_y == d` post-pump);
HG3-287 B-disagree via `pso_input_pad_press` + the bank/cis pair, plus one
keyboard-pair probe (`pso_input_key_report`) asserting `TAdAgreementBaseGC.cpp:76`
accepts EITHER source; HG3-288 cursor repeat probes drive cis via physical
port 0. Each leg's canary presses BOTH gate sources or pins which one it
drives (R-E law). HG3-292/293 (rung-7 card 2) additionally probe
`hardware_keyboard_state` direct (`TTitleScene.cpp:302`) and
`any_pad_input_active` (cis.sources threshold walk) — the 3rd/4th sources.

## 4. Risks + the canary spec (exact, testable)

**C0 — un-injected frame reads era-idle (THE promotion canary; run_input.sh
asserts, and it is the FAIL-LOUD gate for every later leg):** after
`pso_input_init()` with zero `pso_input_pad_*`/`pso_input_key_report` calls,
`pso_input_pump_frame()` x3; assert on the REAL globals:
(a) every `controller_physical_port_bank.controllers[i]` and every
`controller_input_state.controllers[i]`: `buttons==0`, `not_held==0xFFFFFFFF`,
`pressed_buttons==0`, `released==0`, both triggers/sticks `==0`,
`held_with_sticks==0`, `pressed_with_sticks==0`, `stick_magnitude==0.0f`, all
14 `hold_frames[i]==0` — these are the era disconnected-branch values
(ControllerInput.cpp:402-419), NOT a host-invented idle;
(b) `test_pressed_buttons_packed(&bank.controllers[0], m)==0` and
`test_controller_flag_4c_color(&cis.keyboard, m)==0` for all 14
`input_button_mask_table` masks (era-answer-for-zero-input, the A3 doctrine
now enforced through era code instead of a stub);
(c) TEETH — this canary must be able to fail: plant ONE
`pso_input_pad_press(0, PAD_BUTTON_A_bit)` + pump, assert (b) flips to 1 for
mask 4 on bank AND cis (both are fed), then restore and assert back to idle.
A C0 that cannot reject the planted press is decoration (readback-gate law).

**C1 — compaction divergence (single-source is not a fork):** connect
physical ports {0,12} only; after pump assert
`bank.sources[1]==&controller_default_pad_state && bank.source_status[1]==0`
but `cis.sources[1]==&keyboard_control_pad_slots[2] && cis.source_status[1]==1`
(§1.1 law). Injected port-12 presses must be visible at
`cis.controllers[1]` and `bank.controllers[2]` — same raw, two mappings.

**C2 — derived-word discipline:** set stick to (0, 60) with no buttons;
assert `pressed_with_sticks & 0x200000` (up-bit) set on first frame,
`held_with_sticks` set on second, and `pressed_with_sticks` bit CLEARED on
the third (edge law `set_stick_button_bits` :595-602 + the `previous` gate at
:379-381) — proves the API cannot be shortcut by context writes.

**C3 — keyboard mirror is pump-produced:** `pso_input_key_report({94})` then
pump ⇒ `test_controller_flag_4c_color(&cis.keyboard, 16) != 0` for exactly
one frame (pending drains: `poll_key_state` rebuilds from zero each pump,
:565-566); no pump ⇒ no press (mirror is NOT a setter).

Risks beyond R-D/R-E (both resolved by §3):
- **R-D′ type-drift:** VI's paraphrase (§2) proves the failure mode is
  silent-offset, not link-error — the fix's acceptance test is the §2 census
  grep returning exactly one def, plus C0(a) reading through era offsets.
- **R-E′ four-source spread:** pad-bank / cis-controllers / cis-keyboard /
  hardware_keyboard_state direct reads + `any_pad_input_active` raw-slot walk;
  single pump covers the first three; the last two read the SAME promoted
  globals so injection remains coherent. Any future reader added by a lane
  must be re-censused with the §1.4 greps (cheap, same-turn rule).
- **Reset-reseed trap:** `reset_controller_input_state` copies P1 pad over
  cis.keyboard (:512); any leg firing a scene-create store must re-inject
  after it or drive pads before it. HG3-290's 0x17→8 hop crosses
  `:456`'s reset — pin the injection AFTER the store in that leg's body.
- **Split-screen law:** `update_keyboard_input_frame` SKIPS the keyboard
  mirror when `is_split_screen()` (:270-274) and `bind` switches cis to
  physical 1:1 (:457-467) — the host witness default 0 (attract truth) must
  be a pinned extern in every receipt, never ambient.
- **Stub-shadowing:** two same-name counted stubs exist today
  (`ladder7_seam.cpp:121` / `gx_logo_driver.cpp:88`) in separate link scopes —
  harmless now, but a future shared B3 link double-defines; step 3 resolves
  both into one wrapper.
- **Waiver-adjacency:** including era headers into a new TU touches the
  LAYOUT_ASSERT/waiver regime — the seat is VERIFY-only against the pinned
  waiver sha (D8 pattern); it must not add waiver rows (§1.1 note: the four
  asserts here are era-native passes, probe confirms).
- Scope discipline carried from RS: this doc makes no claim about lane MEMSCAN
  (card slot) supply files, prints no disc path, and requests no board action.

## Method / honesty

Zero builds; zero writes outside this doc (single-file commit, `git show
--stat` = this path only). Every era file:line opened this turn from the pin;
every era address pasted from `config/GPOE8P/psov3/symbols.txt` this turn
(addresses are truth, lines are hints — pin moves ~daily); every host cite is
a same-turn `git grep`/`sed` at tip `09de0a8`; consumer counts are python
set-math over this turn's greps (exclusions stated in §1.4), README suite
row read same-turn. BRINGUP5 in-flight files observed via `git status` only,
never read into any design claim beyond their existence. `camera_ctrl_instance`
NULL posture + the PADStatus shim are the only two disclosed design-deviation
candidates; the build seat grades them against the roster grammar.
