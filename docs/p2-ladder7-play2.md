# P2-L7P2 — Rung-5 PLAY-2 receipts: the main_task flag spine EXECUTES era bodies

Lane L7P2 (sole repo builder), 2026-11-27 (card `t_2db03aaf`). Contract:
`docs/p2-ladder7-play23.md` §1 (PLAY-2) over `docs/p2-rung5-seams-1.md` §1 rows
L2/R3/R4 + §2 Play 2 (RS); inputs `docs/p2-ladder7-1.md` (PLAY-1 landed) +
`docs/p2-hg1-1.md` (wave-4 green baseline). Decomp READ-ONLY @ `6b38b53837`:
zero tracked modifications verified after every step; no era header joined the
prelude graph (L7-1 holds — macro trio pasted from `TMainTask.h:17-38` +
`macros.h:89-90` instead, per spec §1.2). Waiver pin `d4382fcd0659357f…`
(`host/bringup/waiver-header.sha256`) re-hashed PRE and POST every make this
lane (baseline suite, 2 standalone builds, 2 full suites): pin every time,
bytes never moved, zero regen. Writes: `host/bringup/**` + this doc only; no
`make -C build parse*/gen_waivers`; ladder6 shared seams untouched.

## 1. What landed (per-seam commits a7562c8 / b10489d / 5c3e740 / bd3ed3e)

| seam | landing | proof |
|---|---|---|
| slices (gen_ladder7_runframe.py §1.1) | L2 `run_task` :283-290 (8L), R3 `render` :509-525 (17L, incl. the :510 `ALLOW_OR_DISALLOW_CHILD` re-pin — spec §3 D1), R4 `render_screen_overlay` :644-652 (9L) VERBATIM + 3 era-empty REAL bodies byte-gated: `render_after_scene` :505, `debug_render_overlay` :533, `TObject::debug_render` :174 (D6 promotion) | 14/14 fail-loud anchors fire; generator replay byte-identical (`cmp` clean); source shas unchanged from PLAY-1 (TObject `6586bb705b8f`); `nm ladder7_runframe_glue.o` defines all six new symbols — no stub links for the spine |
| prelude (§1.2) | `X_OR_Y_CHILD`/`PAUSE_OR_RESUME_CHILD`/`ALLOW_OR_DISALLOW_CHILD` + `FOREACH_NODE_NODECL_MULTI_ITER` verbatim; `main_task_flags` 8-value enum verbatim (era values byte-audited against `TMainTask.h:80-89`); `TObject`/`TMainTask` carriers with era-inline `allow/disallow/pause/resume` bodies (`TObject.h:119-122`); PLAY-2 externs; `L7Call` drops `LC7_MT_RUN/MT_RENDER/MT_OVERLAY` in lock-step, adds adjacent `LC7_WALK_MAIN/KBD/EOR` + 7 sub-pass events; `g_call_seq[64]->[160]` (§2.3 sizing) | compiles against the slices with zero era-header includes |
| host defs (§1.3) | `u32 render_pass = 1;` (.data era INIT — `TObject.cpp:25`; zero-init IS drift, §3 D7); `update/old_update/render/old_render_flags` .bss zero-defs (:53-56); `end_of_render_root`/`keyboard_task_object` def-once (:35/:37, PLAY-3-ready); shared counted `TObject::run_tasks` walk (3-root classify + LIVE flag capture); 8 counted sub-pass externs, each recording LIVE `render_pass` (trace) + `task_flags`; `render_tick` stays counted until PLAY-3 slices it | era homes re-opened from the pinned tree this lane |
| test + run.sh (§1.5) | probes P5-P9 + invariants L7-I2a..e as REAL reads (see §3); `run.sh` insert-only: +1 PLAY-2 banner line, 0 removed lines | `git diff` on run.sh shows pure addition |

## 2. Green (same-turn disk)

| suite | result |
|---|---|
| `make -C host/bringup -f bringup.mk -j6` | rc=0; D8 pre==post==pin every make (5 make invocations, zero regen) |
| `build/bringup/bin/ladder7_test` | **43/43 PASS**, rc=0 (28 PLAY-1 + 15 PLAY-2 lines) |
| `host/bringup/run.sh` ×2 back-to-back | **BRINGUP GREEN — 200/200 PASS, 0 FAIL**, exit 0 both (prior 185 + 15 new PLAY-2 lines) |
| determinism | ladder7+HG1 PASS regions byte-identical across the two runs; generator output byte-identical scratch-vs-build-tree |

**Prior-line audit (baseline 185 vs new):** 176/185 byte-identical. 4 lines are the
pre-existing tick/ASLR-shape set from the HG-1 receipt (`clock`, `E1g`, `L6-G0`,
`L6-G9` — host pointer/tick values move every run by design, shape identical).
5 lines re-targeted BY SPEC DESIGN (§1.5: the three counter seams left the roster —
`L7-R3a` 10→15 events, `L7-R3b` order string now `walk,geom,eff,ptex,obj,ui,clip,info`
inline, `L7-R3d` counts the walk + 7 sub-pass externs instead of the retired trio);
the two PLAY-1 post-probe restoration lines keep their assertions VERBATIM under
new labels `L7-I2f`/`L7-I2g`, handing the `L7-I2a..e` namespace to the spec's
flag-spine invariant set (collision note §4).

## 3. Flag-honesty proven (RS Play-2 What-green, all REAL reads)

- **P5** macro fires for real: `update_flags=0x1` walks the linked `m_down` chain —
  child0 `pause_update()` sets PAUSE_UPDATE (bit 1), child1 takes the else leg
  `resume_update()` (pre-set bit cleared); tail store `old_update_flags==update_flags`;
  second call with flags unchanged fires NOTHING (gate is the real load pair).
- **P6** L2 spine: walk extern observes `MAIN_TASK_UPDATING` LIVE at :287 (root==MAIN);
  after — `task_flags==MAIN_TASK_UPDATED` (0x080), UPDATING cleared, `render_pass=0;` (:285).
- **P7** R3 spine: `g_pass_trace == [0,1,0,1,0]` at geom/eff/ptex/obj/ui entries —
  every :512-:520 `render_pass` store executed; `g_flags_at_subpass == UPDATED|RENDERING`
  at the ui entry (RENDERING live across :511-:522); after — `UPDATED|RENDERED` (0x180),
  RENDERING cleared (:523), RENDERED set by the REAL :524 store.
- **P8** guard honesty both directions: `task_flags=0` → overlay fires NOTHING (real
  :645 load); `UPDATED|RENDERED` → clip+info once each, the three REAL era-empty bodies
  execute (unobservable by design, L7 §3 argument), `task_flags==0` after (:651).
- **P9** full `TLogo::run_frame()` from era-zero: the `g_info` extern observes
  `task_flags & MAIN_TASK_RENDERED != 0` — the guard opened because of :524's REAL
  store, not stub-honesty — sub-pass order == :513-522 text order, `task_flags==0`
  at the frame boundary.
- **I2a** per-frame trace == `{0}`(:285) ∪ ⟨0,1,0,1,0⟩(:512-520) ∪ ⟨0,0⟩ (overlay legs
  ride the trailing 0 — RS's `{0,1,0,1,0(,0)}`); **I2b** 0x800 (outside `~0x3FF`) seeded,
  survives full frames untouched; **I2c** walk == 1/frame, root MAIN (until PLAY-3's
  KBD/EOR roots); **I2d** .bss quartet zero outside P5-style probes; **I2e** every PLAY-1
  invariant (including ADXF/LoadBml/voice counter states) frozen across the spine frames.

## 4. Deviations / honesty notes

1. **Spec §1.3 spell-out:** `g_walk_flags = task_flags;` inside `TObject::run_tasks()`
   names a `TMainTask` member from a `TObject` method — does not compile. Written as
   `main_task.task_flags`; PLAY-2's only walk caller is `main_task.run_task` (:287),
   so the captured value is exactly the spec's intended LIVE flags (same observable).
2. **Label collision:** the spec spells the PLAY-2 invariant set `L7-I2a..e`, which
   PLAY-1 had used for its two post-probe restoration lines. Assertions kept verbatim,
   relabeled `L7-I2f`/`L7-I2g`; spec names now own `I2a..e` (noted in-file).
3. **`render_tick` stays counted** with its `LC7_RENDER_TICK` roster slot: §1.3's
   "remove the four" completes only when PLAY-3 §2 lands; PLAY-2 removes its own three.
4. **Tooling scrubber artifact (fleet-relevant):** this session's file-transfer path
   mangles the literal token `1;` after `render_pass =` into `***`; two restore lines
   and the run.sh banner caught it during landing, caught by the compiler/disk hex-dump,
   repaired at byte level and re-verified (zero residual `***` anywhere in the five
   tracked paths; disk values: seam `u32 render_pass = 1;`, test restores `= 1;`).
   The seam-3 COMMIT MESSAGE (5c3e740) contains the scrubbed token
   `render_pass=***` (token as committed, space-terminated); amend is fleet-law-forbidden so this
   the committed FILE says `u32 render_pass = 1;`.
5. RS's rows stay class-(c)-as-(b) per the spec's classification note; no LB-1/LB-2
   crossing inside any sliced range; the 8 sub-passes stay counters (pixels keep riding
   LOGO-3's single present — RS §3).

## 5. What this unlocks

PLAY-3's seams are pre-positioned: `WALK_KBD`/`WALK_EOR` + roots defined once, the
160-slot order log, and the sub-pass trace machinery. Rung-5 EXIT after PLAY-3:
all 13 `run_frame` legs execute era bodies (5 promoted + 3 flag-spine here +
render_tick decomposition), remaining crossings counted behind RS-tabled seams.

## Repro

`host/bringup/run.sh` → `ladder7_test: 43/43 PASS` inside 200/200 BRINGUP GREEN;
generator receipt: `python3 host/bringup/gen_ladder7_runframe.py $DECOMP_ROOT /tmp/x.cpp`
(14 slices; replay `cmp` clean).