# P2 apply-5 — adoption wave: collision pin + NJS per-TU allow-list land in build/ (integrator)

Date: 2026-11-27. Scott GO: "run the adoption wave (collision recipe + NJS
allow-list into build/)". Inputs: `docs/p2-collision-debt.md` §5 (VERBATIM
rebind-only block), `docs/p2-gx-tex-consumer.md` §4 (allow-list mechanics +
stage-1 trio), `docs/p2-stitch-apply4.md` (adoption rule + repin caveat),
AR `docs/p2-njs-repin-1.md` §6 (per-TU opt-in; never tree-wide, never on `.c`).
Writes: `build/Makefile`, `build/njs_allowlist.txt` (new), this doc,
`tools/parse_gate.py` floor, `build/README.md` line. Nothing else.

## What landed

1. **Rebind+pin block (build/Makefile)** = §5 rebind-only block verbatim,
   exported as target-specific `REBIND_FLAGS` spliced between `CORE` and
   `SDK_INCS`, **LP64-target-only**:
   `-DPSO_PORT_LAYOUT_WAIVER` + force-include `compat/pso_metal_compat.h`,
   `build/waivers/pso_assert_waivers.h`, `compat/collision/pin_dolphin_types.h`
   (pin AFTER the other two, as measured) + `-I host/bringup/include` earliest
   among repo dirs, before `SDK_INCS`. Spraying the block into the shared
   COMMON was tried AND measured this wave: it fabricates ILP32 membership
   churn absent from AZ's ledger (`TItemWeaponDrillLauncher.cpp`
   NJS_OBJECT_size negative-array under arm64_32+rebind + 5 unrequested
   flips) — the collision program's corpus is by construction the LP64 world,
   so the legacy `make parse` anchor keeps its exact historical shape.
   `compat/repin`/`compat/njs` are NOT
   global (apply4 caveat + AR §6); repin joins the corpus only via a future
   full-stack re-record, njs per-TU below (AR §6: never on .c);
   `-ferror-limit=0` added so the official err
   corpus is directly comparable to the AZ battery shapes.
2. **PARSE_RECIPE**: §4 mechanism verbatim — `extra="-Icompat/njs"` iff the TU
   is `*.cpp` AND `grep -qxF`-listed in `build/njs_allowlist.txt`; spliced
   FIRST on the clang++ line; `.c` branch untouched (never receives it).
3. **build/njs_allowlist.txt**: stage-1 logo trio live
   (`psoLoadTexture.cpp`, `GinjaTextureTable.cpp`, `GinjaMaterial.cpp`);
   stage-2 rows (§3 #4-#6) shipped COMMENTED pending NC/V6 probe receipts.

## THE TARGET-LABEL FINDING (fix rides this wave)

`parse-lp64` was silently measuring **arm64_32**: `CXXFLAGS_PARSE :=` baked
`-target $(PARSE_TARGET)` at file-parse time, so the target-specific
`export PARSE_TARGET := $(LINK_TARGET)` could never reach clang. Proof, this
session: official `parse-lp64` printed PASS=*** == the arm64_32 gate number,
and `TItemData.cpp` (fails LP64 natively at both macos11/12 triples with the
current waiver header, passes ILP32) was ABSENT from its fail.txt. Fix:
recursive `=` assignments so `-target` re-expands per goal. Consequence:
pre-adoption true-LP64 numbers were only ever the scratch batteries (162,
apply3/apply4 measured); all "official parse-lp64" claims before today
certified the wrong triple (no prior decision depended on them — the V/BA/AZ
gates all ran scratch probes or host suites).

## Measurements (all legs rc=0, same-turn logs: /tmp/{pre,post}_adopt_parse*.log)

| shape | before | after | gate |
|---|---|---|---|
| parse (arm64_32, legacy shape untouched) | 1,578/1,680 | **1,576/1,680** | stage-1 cost ONLY: fail-list delta = exactly {psoLoadTexture.cpp, GinjaTextureTable.cpp} (previously-ILP32-clean allow-list TUs now pay the u32-slot cluster); zero other membership change; `parse_gate` floor re-pinned 669 -> **1576** (docstring carries history) |
| parse-lp64 control (H0 battery shape, re-run this session) | 162 (apply3/4) | **162 / 1,518 fail** exact reproduction | control |
| parse-lp64 (official, adopted rebind+pin+trio, TRUE macos12 triple) | — (mislabeled before) | **261 PASS / 1,419 FAIL** | §6.2 re-record: 162 -> 261 |
| flips vs H0 | — | 100 (AZ H1C said 96 @macos11 probe; +4 triple-availability, measured not assumed) | accounted |
| cluster grep over `build/parse/err` | — | **0** files | §6.1 PASS |
| collateral (H0-pass minus post-pass) | — | **{GcrnaStream.cpp} exactly** | §6.3 PASS (D4 facade pending) |
| allow-list trio err-delta (macos12 NOSHADOW vs SHADOWFIRST probes) | — | psoLoadTexture 3->4, GinjaMaterial 5->6, GinjaTextureTable 2->2 (sites swap to §3#1 :20/:24) | §4 PASS-with-note: every shadow-introduced error sits on an AR §3 enumerated site (`:89/:145/:149`, `:367/:388/:421`, `:20/:24`); §4's literal "+3/+3/+6" were battery-3 (macos11, global-njs) absolute counts — the official per-TU deltas are +1/+1/0-swap, same clusters |
| waiver header sha across all four battery legs | c6b514035f39b633… | **identical pre/post every leg** (regen runs, bytes stable, D8 verify intact) | R1 honored |
| target-fix canary | — | `TItemData.cpp` (fails native LP64, passes ILP32) **present** in official parse-lp64 fail list | gate that can fail, did fail-into-place |

## Suites

Host `.mk` flag lines untouched — suites are provably file-invariant under
this wave (host builds read `host/*/**.mk` + the byte-identical waiver header,
never `build/Makefile`). Re-run evidence this window: lane BB S4 battery
(run.sh 128/128, run_dl 210/210, run_window, run_vi) on the post-wave tree —
that is the one-build-tree-law re-run while BB holds the host build seat;
integrator bringup re-run at the first lane-free window closes the ring.

## Deferred / named follow-ons

- **host-side pin**: `host/*/**.mk` COMMON lines do NOT yet carry
  `-include compat/collision/pin_dolphin_types.h`. Today's executing worlds
  are green without it (D4 include-set drops most era dolphin headers;
  ladder6's `NJS6_CXXFLAGS` adds `-I<decomp>/libs/dolphin/include` and is the
  first world where edge-B can bite). Deliberately deferred out of this wave
  to keep lane BB's V5 build window on stable host flags; adopt at the next
  no-lane-build boundary with the suite battery as the gate.
- **repin leg / full-stack re-record**: the H2FN/H3 global
  `-I compat/repin -I compat/njs` battery shape stays a re-record decision
  for AFTER the residual-class lanes (NJS ~112, packets 57, PppFrame 50,
  MWCC-asm 38, musyx 26) land their fixes — tree-wide njs is AR §6-banned
  until the .c silent-truncation class is audited.
- **stage-2 allow-list**: flips live only with NC's per-TU probe receipts.
