# P2 apply-7 — GO wave: 808-row NJS adoption + facade/pshim routing + WT mechanism + dvd.h regression fix (integrator)

Date: 2026-11-27. Scott GO (clarify, verbatim choice): "Everything: 808-row NJS
adoption (350→1,158) + WT waiver mechanism + D4/pshim routing + paired dvd.h
regression fix (collateral → empty set)". Inputs: NA `docs/p2-njs-adopt-1.md`
(3717cea), WT `docs/p2-waivetail-1.md` (a51888c), D4 `docs/p2-d4facade-1.md`
(6a6d509), TL `docs/p2-tail-1.md` (37e997e+021c03e), RS `docs/p2-rung5-seams-1.md`
(d47f085). Regen + sha re-pin executed at the zero-lane boundary; all six
wave-2 lanes had exited and their tips were disk-verified commit-by-commit
(scope + author + single-path staging) before anything landed.

## What landed

1. **build/njs_allowlist.txt stage 3**: NA's 808 ADOPT-NOW rows appended
   (programmatically extracted from §6, dedup-checked 0, all `.cpp`, all
   present in units.txt). Set identity inherited: 808 adopt + 260
   RUNTIME-GATED = 1,068 FLIP; BmlResource/TCMCharacter stay live but are
   ledger-TAGGED gated (NA §2 headline — parse adoption holds, runtime waits
   on the uaddr handle program; no rollback claim).
2. **NJS allow-list is now LP64-LEG-ONLY** (`NJS_LEG` export: set on
   parse-lp64, empty on parse). The u32-slot shadow is a *LP64* fix (ILP32
   pointers are already 4B); gating keeps the arm64_32 legacy anchor pure and
   removes apply-6's "-2 stage-1 cost" term from the 32-bit decomposition.
3. **WT mechanism** (gen_waivers TEMPLATE): `PSO_LA_NOP` static-int →
   `typedef int` (identical-typedef repeat is legal c11+c++17; KEEP arm
   already relied on it). WT scratch-proven: 3,333 flag lines identical,
   collision redefs 4→0, zero new.
4. **LP64_REBIND routing bundle**: `-I compat/facade` (MUST precede
   host/bringup/include — D4 OF-A/OF-B proof: bringup's OSAlloc.h otherwise
   wins the angle) + `-I compat/dialect/pshim` (hw_regs raw-colon supply).
5. **dvd.h facade-skip (the paired fix TL demanded before facade routing)**:
   compat/repin/dvd.h gains THREE anchor-guarded skip blocks
   (`struct DVDCommandBlock` body, `struct DVDFileInfo` body,
   `DVDGetCommandBlockStatus` const-proto) under `#ifndef PSO_BRINGUP_H` —
   the host facade's spellings DIVERGE (void* id, non-const proto), so the
   era copies must not double-fire; everything else stays era-verbatim,
   crucially `DVDDiskID` (era os.h OSBootInfo embeds it; pso_bringup does
   NOT define it — first whole-body-skip attempt broke mdm.c exactly here;
   per-block guard restored it). Generator-implemented with fail-loud anchor
   checks; regeneration byte-certifies.

## Measurements (fills from this wave's logs)

| quantity | apply-6 | apply-7 | ledger |
|---|---|---|---|
| parse-lp64 (official) | 350 | **1,163** (+813) | NA projected 1,158 for the block alone; the +5 over projection = exactly the bundle: mdm.c, NetworkInfoLoader.cpp, McFsGc.cpp (facade+pshim+dvd-skip clean victims), GcrnaStream.cpp (collateral kill), KBDXfer.c (TL §4 blocked line). **Every one of the 813 gained rows attribute-checked** (python set-diff vs the 808 list + named bundle); **LOST = ∅** — zero regressions corpus-wide. 69.2% of 1,680 |
| parse (arm64_32) | 1601 | **1,605** | +4, zero new fails: NJS_LEG gate recovers psoLoadTexture + GinjaTextureTable (shadow removed from ILP32 where it was never needed) and WT recovers ChallengeMode + TLobby_80209524 (waiver-dup redef gone). Floor re-pinned **1605** = 1,578 + 25 + 2, decomposition in docstring |
| collateral vs true-LP64 H0 (162) | {GcrnaStream} | **∅ — EMPTY SET** | GO target met: first time since apply-4 with zero known rebind collateral |
| cluster / PppFrame / DVD-redef / OSTime.h:15 / pso_la_waived dupes | 0 / 0 / 12 / 12 / 2 | **0 / 0 / 0 / 0 / 0** | TL §4 regression eradicated at source; WT collisions gone; NP/PK holds |
| musyx fails | 2 (named) | 2 (named) | MX residuals stable |
| waiver header | 495cc91a… | **d4382fcd…** (mechanism-only delta; flags 2,946/3,333 over 1,549 types UNCHANGED; 3,333 flag lines identical per WT proof) | D8 re-pinned this commit; pre-gen/post-gen read back on the leg |

## Suites (new-header era d4382fcd) — ALL GREEN

run.sh BRINGUP GREEN (13+16+26+27+28+18 = 128/128), run_dl 210/210
verify_failures=0 frames=30 dl_calls=15 tris=60, run_window era 12/12 +
AV-1 10/10 (default synthetic-fixture path — the genuine 256x256 regime is
GX_LOGO_GVR-hooked and re-certified by BB2 on local assets, intentionally
NOT wired into committed defaults), run_vi both DTV legs, link boot slice
ALL RECEIPTS OK 8/8 under the apply-6 sha-guard with header==pin
(d4382fcd0659357f), parse_gate rc=0 at the 1,605 floor.

## Next-wave supply

RS plays 1–3 (ladder7), NA ADAPTER-QUEUE 169 (site table ready §3),
NA RUNTIME-GATED 260 (uaddr handle program), TL plays 2–3 (decl-align +
asm-ten census), PK/NP residuals already re-attributed. Fleet seats refill
against the apply-7 numbers.
