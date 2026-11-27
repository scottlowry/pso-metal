# P2-RC2 apply-8 + wave-4 integrity RE-AUDIT (independent audit sweep; zero repo builds)

Fleet wave-5 lane RC2, card t_f0656240, 2026-11-27. Decomp `pso-decomp @6b38b5383`
READ-ONLY (rev re-verified post-sweep; porcelain state unchanged, zero files touched).
Repo HEAD `beccedd` at intake AND after all sweeps. Waiver header `d4382fcd0659357f…`
byte-verified pre/post every sweep, never re-hashed up, never regen; collision pin
`fdfadd5e6f261a7c…` (apply-8 era) stable pre/post. Zero `make`/`ninja`/`gen_waivers`.
Runner: wave-3-certified `rc_probe.sh`+`rc_sweep.sh` copied from
`fleet/receipts/apply7-rc/` to `$TMPDIR/rc2/` (`rc2_probe.sh`/`rc2_sweep.sh`; flags
still valid — `build/Makefile` and `build/njs_allowlist.txt` byte-unchanged since
apply-7 payload `9c5aba7`), corpus `all.txt` set-identical to `build/units.txt`
(diff 0, 1,680). Persisted receipts: `cache/scratch/rc2/`.

**VERDICT: apply-8 confirmed exactly as claimed (+4, zero breaks); wave-4 confirmed
INERT on the parse corpus; LOST = ∅ vs the cached wave-3 official set. One REAL
anomaly: the certified A7N shape did not mean what wave-3 §0 documented (see §4).**

## 0. Shapes and gates

Every shape recorded exactly (pass+fail)=1,680, disjoint, union=corpus (python-verified):

| shape | definition (all: CORE compat+waiver -include, SDK -I, macos12.0, -ferror-limit=0) | PASS | FAIL |
|---|---|---|---|
| H0′ | plain LP64, no rebind/pin/njs (certified H0 verbatim) | **167** | 1,513 |
| A7N-cert | certified rc_probe `A7N` case (see §4 — actually ZERO rebind) | 192 | 1,488 |
| A8N | **corrected** apply-8-minus-njs: RB7 (−D, pin, net, repin, facade, pshim, bringup) with ZERO `-Icompat/njs` anywhere | **357** | 1,323 |
| A8 | apply-8 official: A8N + allow-list `-Icompat/njs` first on `.cpp` rows (813 rows, `.c` never, Makefile guard mirrored) | **1,167** | **513** |

## 1. H0′ vs pinned 167-control — every delta attributed

H0′ = **167, set-identical to `H0-CONTROL.md`/certified `H0/pass.txt` (sym-diff 0)**.
The integrator note's "expect H0′=171" does NOT hold at plain LP64 — attribution,
not smoothing: apply-8 (`3e4ba9d`) touched exactly ONE parse-relevant file,
`compat/collision/pin_dolphin_types.h` (git show --stat); H0′ never `-include`s the
pin, so no va supply exists in that shape. All 4 va-class rows still fail at H0′ —
and their H0 first-errors aren't even the va wall (`CriSvm/LscLog/NetworkConnect`:
`ItemDataSize` negative-array; `Gcci`: decl-parse) — the va wall only surfaces
behind the rebind include order. 171 is the right expectation for any **pin-carrying**
shape, and §2/§3 deliver it (A7: 1,163→1,167; corrected A8N carries pin too).

## 2. FLIP-proof of the apply-8 +4 (pin-carry is load-bearing)

`rc2_flip.sh` = certified probe + `RC_PIN` env override; official-shape cmdline,
4 rows, pre-apply-8 pin (`git show 3e4ba9d^:…pin_dolphin_types.h`, `49f94cd3…`)
vs current pin (`fdfadd5e…`):

| row | pre-pin first error | current pin |
|---|---|---|
| `src/pso/audio/CriSvm.cpp` | FAIL `__builtin_va_info` undeclared (:190) | PASS, 0-byte err |
| `src/pso/audio/LscLog.cpp` | FAIL `__builtin_va_info` undeclared (:16) | PASS, 0-byte err |
| `src/pso/network/NetworkConnect.cpp` | FAIL unknown type `va_list` @ era `os.h:40:33` | PASS, 0-byte err |
| `src/pso/system/movie/Gcci.cpp` | FAIL unknown type `va_list` @ era `os.h:40:33` | PASS, 0-byte err |

4/4 FLIP (fail without pin-carry ∧ pass with ∧ 0-byte receipt) — the va_info pair
and the os.h:40 pair, exactly the stitch §FIXED list. All four are NOT allow-listed
(grep 0 hits in `njs_allowlist.txt`), so njs plays no role; all four pass at corrected
A8N too. A7 vs wave-3 A7-receipt pass sets: gained = **exactly these 4**, lost = ∅.

## 3. LOST vs cached wave-3 official + corrected njs attribution

Cached `a8b_64fail.txt` (513, survived — no reconstruction needed): my A8 fail set is
**set-identical (sym-diff ∅ both directions) ⇒ LOST = ∅, GAINED = ∅** vs wave-3
official. `build/parse/fail.txt` (official, generated 10:31 = apply-8 landing,
**pre-wave-4**) is also sym-diff ∅ vs both — third independent agreement.
Corrected-shape attribution: A8 ⊃ A8N strictly; njs-additive = **810 rows = 808
stage-3 ∪ {BmlResource.cpp, TCMCharacter.cpp}** — the wave-3 §2 allow-list∩gated-live
pair, now proven with a control that actually isolates `-Icompat/njs` (pin, net,
repin, facade, pshim, bringup constant). 808/808 flip; named +5 bundle rows
(`mdm.c NetworkInfoLoader McFsGc GcrnaStream KBDXfer`) all pass at A8N (facade/pshim
leg, zero njs need) — partition holds. Behind-the-va walls per stitch §27 verified at
A8 receipts: `eth.c` first-error `__lhbrx` (now SOLE), `gjLight.c/nuReLoadTexture.c/
SystemBootInit.cpp/GinjaDrawUtil.cpp` all `NJS_OBJECT_size` negative-array — 4/4, no
double-count into the 485 queue.

## 4. Wave-4 inertness — confirmed, with one wave-3 runner finding

(a) **Tree:** `git diff 3e4ba9d..HEAD` under `compat/`+`build/` = exactly ONE file,
`compat/njs/adapters/batch2/README.md` (not a header, never includable); under
`-I host/bringup/include`: `hg1_windows.h` — 0 name-collisions vs decomp
include/libs/src, 0 corpus refs (full 1,680-TU grep). `build/Makefile`, allow-list,
waiver, pin: 0-line diffs. (b) **Set:** my wave-4-tree official sweep == official
pre-wave-4 513 set == cached a8b receipt, sym-diff ∅ (§3). (c) **Sibling WIP:**
`host/bringup/{gen_ladder7_runframe.py,ladder7_seam.cpp,include/ladder7_run_prelude.h}`
were dirty-edited by lane L7P2 during this audit (mtimes 11:30–11:36, clean at intake
11:16); every full-sweep and FLIP probe completed at or before 11:32:53, all three
files 0-corpus-reachable (prelude is bringup-internal; generator is a script) — and
the §3 set identity independently bounds any leakage at zero.
**ANOMALY (ledger hygiene, wave-3 doc vs its own runner):** the certified `A7N` case
never sets `RB` (rc_probe.sh line 43 guards `A7|A6E` only) → wave-3's "A7 verbatim
with ZERO -Icompat/njs" column was actually a **zero-rebind** shape: 192 = H0 167 +
25 musyx-as-C++ flips (set-diff python: all 25 extra rows are `src/musyx/runtime/*`).
Wave-3 §2's FLIP predicate (fail@A7N ∧ pass@A7) therefore did NOT isolate the njs
flag. Verdict impact: none — the 808 ADOPT rows were re-proven here against corrected
A8N (§3: 808/808 flip exactly). Lesson: re-pin the A7N ledger like wave-3 re-pinned H0.

## 5. Promise-vs-real (unsmoothed)

| metric | promised (docs) | real (this sweep) | drift |
|---|---|---|---|
| LP64 official PASS | 1,167 (`p2-stitch-apply8.md` §24) | **1,167** | 0 |
| LP64 official FAIL | 513 (stitch §24; 485+9+2+17 §36) | **513**, set-identical to 2 official artifacts | 0 |
| apply-8 +4 FIXED, 0 BROKE | CriSvm/LscLog/NetworkConnect/Gcci | 4/4 FLIP-proven; gained set == named set exactly | 0 |
| H0′ | 171 (`H0-CONTROL.md` integrator note) | **167** (set-identical to control) | −4: expectation conflated shapes — supply is pin-only; 171 holds only for pin-carry shapes (H0+pin analog not run; A8 official 1,167) |
| LOST vs wave-3 official | ∅ | **∅** | 0 |
| wave-4 corpus effect | none claimed | none measured (§4a+b+c) | 0 |
| certified A7N semantics | "A7 minus njs" (wave-3 §0) | zero-rebind shape; true minus-njs = **357** | doc-vs-runner (§4) |

## Repro

`cache/scratch/rc2/` — `rc2_probe.sh` (certified + `RC_PIN` env + corrected `A8N`
case), `rc2_sweep.sh`, `rc2_flip.sh` (FLIPP/FLIPC), `pin_pre_apply8.h` (49f94cd3…),
`sweep.log` + `{H0,A7N,A8N,A7,FLIPP,FLIPC}/{pass,fail,err}/`. Waiver pin read-only;
decomp untouched; pso-metal writes = this doc only; receipts dir never written.
