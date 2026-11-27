# P2 apply-6 — adjudication wave: five class lanes' asks landed (integrator)

Date: 2026-11-27, immediately following apply-5 (`docs/p2-stitch-apply5.md`)
and the 6-lane residual-class fleet (NC/PK/NP/MA/MX + BB V5 — receipts in
`docs/p2-njs-wave-1.md`, `p2-packets-1.md`, `p2-pppframe-1.md`,
`p2-mwccasm-1.md`, `p2-musyx-1.md`, `p2-v-wave2.md`). Boundary law honored:
generator/regen ran only with ZERO lanes building (all six had exited);
the link GUARD independently proved the pin discipline works (§Ledger).
No push; origin untouched.

## Asks → dispositions

| ask (lane) | disposition | landing |
|---|---|---|
| PK (a) base-row UNREVIEWED→RW | **RESTRUCTURED** — base row is overridden by `verdict-resolutions-2.tsv:1893` `PsoCommandHandler LOCKED-FORMAT`; (a) alone would have been a NO-OP through the later-wins chain | `data/verdict-resolutions-8.tsv` row: type RW, basis `packet_classes.h:370-376` carries `PsoCommandFn handle` (function pointer — wire records carry none; type ABSENT from the 10 named recv classes of net-disc:42; doc `p2-packets-1.md` §2). Precedence fired as designed: `gen_waivers: NOTE — resolution-precedence override: PsoCommandHandler(keeper@5->rw@12)`. res2 row kept as honest history. Deny-list: not a root ✓ |
| PK (b) flag flip | ACCEPTED via the chain (regenerated, no hand-edit) | `PSO_LA_FLAG_PsoCommandHandler_size 0→1` |
| NP legal family (two flag flips) | ACCEPTED with MECHANISM FIX — both flips were unreachable, not unwarranted: (1) `sizeof(struct PppFrame)` — the keyword was eaten as the type name (exactly 1 such site corpus-wide); (2) `TcpTcb_f204_check` compares macro `TCPTCB_F204_BYTE` = pure `offsetof(TcpTcb,...)+sizeof(...)` (only ==literal-without-type site whose macro resolves). Types already RW (tranche3 / res6) — extraction fix lands them honestly, no new verdict rows | `build/gen_waivers.py`: `SIZEOF_KW_RE` + bounded one-level macro-indirection in `scan_tree`. Flags `PppFrame_size_check` and `TcpTcb_f204_check` → 1 |
| NP shadow `-I` | ACCEPTED | `-I$(REPO)/compat/net` into `LP64_REBIND` (NP paste-block order: net → repin → bringup; njs stays per-TU) |
| MA repin shadows | ACCEPTED (flag lines = zero new flags, repin `-I` rides `LP64_REBIND`) | 4 shadows on `-I compat/repin`; class gate below; **ERRATUM**: "AT repaired TU-globally by first shadow hit, OSLink needs no own shadow" held for 22/23 — `src/pso/system/RelModule.cpp` includes OSLink.h with no shadowed header in its chain → 5th shadow generated via MA's own `gen_dialect_shadows.py` (+1 flip) |
| MX routing | ACCEPTED | `PARSE_RECIPE` `.c` branch: `src/musyx/runtime/*` → `clang++ $(MUSYX_CXXFLAGS) -x c++` (`-Icompat/musyx` before all other repo dirs; P2-T precedent + AA `docs/p2-musyx-linknames.md` audit, joint acceptance test = no `__Z*snd*` on link landing) |
| MX 2 TSV WAIVE rows (chorus_fx, reverb) | **DECLINED (no consumer)** — the waiver machine is assert-keyed; these fail on `asm void` bodies + absent `orig_asm/*.inc`, not layout asserts. Inventing a parse-waive ledger outside the fail-closed machine would launder. Named residuals stay honest: they are the measured 2/27 | recorded as host-facade queue (chorus-standin precedent) |
| NC stage-2 rows | ACCEPTED per probe receipts | `build/njs_allowlist.txt`: `BmlResource.cpp` + `TCMCharacter.cpp` LIVE (H3-clean adapter-free, ILP32-clean both legs); `TChCrLoadReq.cpp` stays commented (§3#6 8-error cluster) |
| NC 1,001-row tree-wide list | **HELD for decision** (AR §6 stands) | audit seat dispatched (`P2-NA`, `docs/p2-njs-adopt-1.md` evidence package) |
| MA `compat/dialect` -I ask | HELD | MX routing already covers the 27; hw_regs raw-colon 1 → tail census seat (`P2-TL`) |

## Battery (all legs rc=0; logs /tmp/{a6,a6b}_*.log)

| quantity | apply-5 | apply-6 | accounted |
|---|---|---|---|
| parse-lp64 (true LP64, official) | 261 | **350** | +50 NP class (PppFrame citations 0) +44 PK line-effect overlaps resolved +25 MX (2 named residuals) +8 TcpTcb-beyond-class +1 MA erratum RelModule — python set-diffs recomputed per leg, no head math |
| parse (arm64_32 anchor) | 1576 | **1601** | musyx +25 (both legs); stage-2 rows cost ZERO (ILP32-clean under shadow); `parse_gate` floor re-pinned 1576→1601 |
| collateral vs H0-clean | {GcrnaStream} | **{GcrnaStream}** exact — zero new collateral despite net+repin+flags+musyx-flip | §6.3 holds; D4 facade seat dispatched |
| cluster grep / PppFrame grep | 0 | **0 / 0** | §6.1 + NP gate |
| MA class | — | RelModule was the LAST AT-syntax residual (`docs/p2-mwccasm-1.md` 38-row list + 15 dual-class remain in fail corpus for NJS_OBJECT_size KEEP asserts — deny-list correct, they are NA-seat rows) | gate by FILE MEMBERSHIP, not token grep (context-note lines inflate token counts — measured 43 vs real 0) |
| waiver header | c6b51403… | **495cc91a5b9694c4…** (tracked file + `waiver-status.txt` land in THIS commit; D8 `waiver-header.sha256` re-pinned same commit) | exactly-3-flag-line + STATE delta vs PK's scratch sim (`2946/3333 over 1549 types`; ntypes shows +1 where set-math says +2 — display decoration only, nothing branches on it per R1; banked as known) |
| suites (new-header era) | — | run.sh 128/128 (ladders 13+16+26+27+28+18 all PASS), run_dl 210/210 verify_failures=0, run_window era 12/12 + AV-1 10/10 (anchor fnv `0x88A8EF94CB09FCD0` stable across the header bump), run_vi PASS | link: first pass stopped on the STALE literal GUARD (see section below); after the sha-pin fix + re-pin landing, `make -f host/link/link.mk run` = **ALL RECEIPTS OK (8/8)**, rc=0 |

## The link-GUARD landmine (pre-existing, exposed this wave, fixed here)

`host/link/link.mk:48` verified the waiver header with a LITERAL grep
`'STATE: 99 waived'` — a relic from the first waiver era. It had been dead
everywhere since the 2179/2943 expansions (i.e. `make -f host/link/link.mk
run` would fail on the committed tree from BEFORE this session; the boot-slice
receipts predate the count jumps). My link re-run lit it under a live camera.
Fixed per the banked law (literal-state verify-guards -> sha-pinned approval):
the stamp now verifies `sha256sum(header) == host/bringup/waiver-header.sha256`
(D8 authority). Guard verified BOTH directions this session: planted-bad sha
=> stop (rc!=0, message); real sha => ALL RECEIPTS OK (8/8 boot receipts).

## Meaning

One fleet wave + one adjudication pass moved the official LP64 leg 162 → 261 →
**350** (21%) with zero collateral drift, and every class ask was landed
through its OWN mechanism chain (precedence file, generator fix, recipe
routing, allow-list), not by hand-editing generated artifacts. The remaining
fail corpus is now fully class-attributed: NJS dual-class 15 + dialect
adoption backlog (NA audit), §4.9 tail (TL census), facades (D4 seat), and
the two named musyx WAIVE legs.
