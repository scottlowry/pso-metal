# P2-L7 — Rung-5 PLAY-1 receipts: the five promoted `run_frame` legs execute era bodies

Lane L7, 2026-11-27 (card `t_8b6e735f`). THE wave's sole repo builder: host/bringup
builds ONLY (`make -f host/bringup/bringup.mk` + `run.sh`); ZERO `make -C build
parse*/gen_waivers`, ZERO writes under build/ data/ compat/ host/gx/.
Contract: `docs/p2-rung5-seams-1.md` Play 1 (+ AP `docs/p2-rung4-contract.md` §5,
BB `docs/p2-v-wave3.md`). Decomp READ-ONLY @ `6b38b53837` (rev verified same turn).
**Waiver-header D8 pin: file content sha `d4382fcd0659357f…`
(`host/bringup/waiver-header.sha256`, apply-7 era) — `build/waivers/pso_assert_waivers.h`
re-hashed PRE and POST every make: `d4382fcd0659357f` every time, bytes never moved,
zero regen.** Official LP64 corpus context (card-supplied, post-apply-7): 1,163 PASS;
`build/parse/summary.txt` at this lane's intake read `target=arm64_32-apple-watchos7.0`
→ integrator mid-battery per shared law — NO parse-state claim in this doc is inferred
from it; the R5 row-5 promotion is carried as the RS/BA-era documented status the card
pins (apply-7 era).

## 1. What landed (one build, WIP-1 `d91590af8`)

`host/bringup/gen_ladder7_runframe.py` (fail-loud anchored, `slice_fn`/`slice_line`
machinery of `gen_ladder6_slices.py` + `must_contain` guards of `gen_ladder6_logo.py`;
zero adapter EDITS — 8 VERBATIM slices, replay-verified byte-identical) →
`build/bringup/gen/ladder7_runframe_glue.cpp`:

| slice | era home @ pinned decomp | RS ref |
|---|---|---|
| `TMainTask::begin_frame` | `system/task/TObject.cpp:640` (3 lines, **byte-gated era-empty body** — sig + blank + `}`) | L1 (a) |
| `poll_reset_request` | `system/ControllerInput.cpp:258` (6) | R7 (b) |
| `process_async_file_reads` | `system/AsyncFileRead.cpp:198` (88) | R5 (b) — row-5 promoted |
| `update_sound_request_slots` | `audio/SoundRequests.cpp:340` (52, guard leg) | R6 (b) |
| `TQuestVmDownLoadServer::is_ready` | `unk/TLobby_800F8AA8.cpp:1371` (10) | R8 (c) via LB-2 |
| `async_file_read_stub` | `system/TGStream.cpp:203` (1-line exact, era tail callee) | era truth |
| `request_main_phase` | `ui/keyboard/SystemBootInit.cpp:219` (4) | carrier (gx-pinned anchor) |
| `TLogo::run_frame` | `ui/TLogo.cpp:94` (22) | the carrier itself |

Source shas[:12] this build read: TLogo `13889e46dcaa`, SystemBootInit `ed382a8ae2f8`,
TObject `6586bb705b8f`, ControllerInput `f84360c2afc5`, AsyncFileRead `92f724ec910e`,
SoundRequests `9c9ebe49965a`, TLobby_800F8AA8 `684182e5c65e`, TGStream `a704fc8aee9b`.

`host/bringup/include/ladder7_run_prelude.h` carries four era headers
(`pso/system/AsyncFileRead.h`, `pso/system/Adxf.h`, `pso/audio/SoundRequests.h`,
`pso/network/client_id.h` — narrow graphs; the `AsyncFileReadRequest*` LAYOUT_ASSERT
names all carry applied waiver flags) + host carriers. `ladder7_seam.cpp`: ALL era
.sbss globals zero-def'd (the 9 AsyncFileRead + sound/quest/controller sets), counted
LOGO-2-shape seams, LB-2 pump seam, LB-3 fail-loud roster. `bringup.mk`: `ladder7_*`
rules mirroring the ladder6 shape (insert-only, appended block; `all:` gains a line);
`run.sh`: insert-only block before `BRINGUP GREEN`. NO ladder6 shared seam edited;
`host/gx/**` untouched (the gx driver's return-0 stubs stay — they are the Z lane's
seams; rung-5 PLAY-1's "kills the five stubs" is proven IN BRINGUP: these binaries
link NO stub for a promoted leg, the era symbols do).

Deviations (spelled in-file): **L7-1** minimal host carriers for TLogo/TMainTask/
TQuestVmDownLoadServer/TLobby/TCameraCtrl (D4 keep-the-graph-off; LB-1/bare-carrier
precedent — sliced bodies touch only declared members, verified against era text).
**L7-2** counted legs stay: P1–P4 + main_task run/render/overlay (Play 2), render_tick
(Play 3), begin_gx_frame real slice stays gx-side (LOGO-1) — counted here. **L7-3**
seam split: fail-loud = `reset_system` (card-named), `check_reset_button`,
`switch_to_fallback_main_phase`, `pop_and_close`, `notify_and_close`,
`destroy_instance`, `tmp_buf_release`, `move_words`, `copy_u16_array`,
`load_gvm_texture_archive`, `check_sound_pitch_request_range`, `stop_sound_request`;
counted era-defaults = `ADXF_Open/ReadNw/GetStat`, `tmp_buf_use_req` (RS R5 LB-2
stand-in), `get_sound_voice_state`, `update_music_fade`, `get_player_by_client_id`,
`LoadBmlExec`, `VecCopy`; `move_data` rides the EXISTING `move_data_glue.o` (era slice).

## 2. Green (same-turn runs)

| suite | result |
|---|---|
| `make -f host/bringup/bringup.mk -j6` | rc=0; D8 `d4382fcd0659357f` pre+post EVERY make (4 makes) |
| `host/bringup/run.sh` | **BRINGUP GREEN — 156/156 PASS, 0 FAIL** (prior 128 verbatim + ladder7 28), exit 0 |
| `build/bringup/bin/ladder7_test` | **28/28 PASS**, exit 0 |
| generator replay | second run byte-identical (`cmp` clean), 8 `#pragma push` blocks |

**Real-read invariant set (RS What-green, ladder7_test L7-I*):**
`controller_reset_requested==0 ∧ async_file_read_state==0 ∧ sound_requests_enabled==0 ∧
quest_download_server_ready==0` held across 4 real `run_frame` calls → poll==0, no
`switch_to_fallback_main_phase` fire, no `reset_system` fire, no `request_main_phase(0)`
(`main_phase_state` all-fields-zero read after every phase), and ADXF/tmp_buf/voice
counters stay 0 on the untouched legs (`L7-I1e`). `L7-R3` order log ==
`TLogo.cpp:95-114` text order ×4 frames (10 events/frame; the five promoted legs leave
no stub note — `LC7_READY` fires INSIDE the era chain at `wait_ready_loop`, the other
four slots bracketed at their era text positions). `L7-R3c` camera mode 9 + pose
{0,0,0}/{0,0,-100} captured through the REAL body.

**Flip probes (bodies EXECUTE, era stores read back):** P1 poll ternary both legs
(memcard-gate→0 despite reset=3; →returns 3 real global); P2a era `case 0` stamped
`request->flags |= 0x100` on a host-visible request and called `ADXF_Open` with the
request's OWN name buffer (pointer-equal), NULL handle → state stayed 0; P2b `state 4`
→ `LoadBmlExec(->mode,->name)` arg-captured, era kept state 4 on the not-done return;
P3 guard opened ONLY on the flip: `get_player_by_client_id(local_client_id=7)`,
camera-NULL else-leg zeroed `sound_listener_position[0..2]` (era stores), 40-slot
voice loop, `sound_pitch_requests[0].flags 0x400→0` era write, `update_music_fade`
reached; P4 `is_ready` all four era return legs (ready=1→1 with pump 4th entry,
in-progress gate, memcard gate first, never-ready restored). `L7-I2b`: after every
flip, all four invariants re-read zero — era-zero spine is the default, probes were
the exception paths.

## 3. Faithfulness notes / next rung

- begin_frame's era truth is the EMPTY body: no seam could log from inside a verbatim
  slice; its slot is occupied by the era symbol (link-level proof: `nm` of
  `ladder7_runframe_glue.o` defines `TMainTask::begin_frame`). Same argument RS §5
  makes for `.s` anchors — stated, not smoothed.
- `wait_ready_loop` is NOT sliced this rung (RS R8 plan: pump-count extern joining the
  LB-2 roster — era disk-present path pumps zero frames; fake_disc DVD shim reports
  present, matching the documented first-iteration return).
- No `.s` offsets are re-cited here; line anchors are the decomp file:line the RS
  ledger opened, re-verified against the pinned tree this lane.
- Next rung per ranking: Play 2 (flag spine L2+R3+R4), then Play 3 (render_tick
  roster); lane J still gates P1–P4/R6-full.
