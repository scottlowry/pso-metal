# P2-AQP2 — NJS adapter-queue batch-2: the remaining-139 enumeration + 64-cluster three-way split (INERT, ZERO builds)

Fleet wave-4 lane AQP2 (zero-build), card `t_b2719eee`, 2026-11-27. Decomp
`pso-decomp @6b38b5383` READ-ONLY; NOTHING lands in the decomp tree; the only repo
writes are this doc + `compat/njs/adapters/batch2/` (inert index). Inputs: NA
`docs/p2-njs-adopt-1.md` §3 (the 169-row ADAPTER-QUEUE), AQP batch-1
`docs/p2-adapter-queue-1.md` (top-30 + §5 named-next-class), HP
`docs/p2-handle-program-1.md` (W-* window obligations), apply-8 ledger
`docs/p2-stitch-apply8.md`.

## 0. Era + probe shape (apply-8)

- Waiver header `build/waivers/pso_assert_waivers.h` sha256 = `d4382fcd0659357f283e675726c1076338e9095…`
  measured THIS turn, prefix-identical to the D8 pin `host/bringup/waiver-header.sha256`
  (`d4382fcd0659357f283e675726c1076338e90955`): apply-8 era certified, zero regen.
- Runner `aqp2_probe.sh` = FRESH whole file (not sed-derived) = official LP64 PARSE_RECIPE
  (compat trio `-include` in order compat → waivers → collision pin; `-DPSO_PORT_LAYOUT_WAIVER`;
  `-I` net, repin, facade, dialect/pshim, host/bringup/include — facade before bringup)
  **+ `-Icompat/njs` (all rows are `.cpp` — zero `.c` in the queue, NA §2; AR §5)**
  + `-target arm64-apple-macos12.0 -ferror-limit=0`; pool = BSD `xargs -P6`.
- Shape fidelity: BEFORE = decomp originals at NJS shape. **139/139 files still FAIL**;
  fresh error totals sum **786 == the NA §3 `total` column summed over the same 139 rows
  (exact)** — the facade/repin legs cured other-class residuals only on batch-1's rows,
  none on the remaining queue. A BASE-shape sweep (no `-Icompat/njs`, official fail side):
  139/139 FAIL, 139/139 contain the `NJS_OBJECT_size` wall error, and **136/139 carry it as
  FIRST error** — all 139 are members of apply-8's 485-wall set, not double-counted with
  batch-1's 30 (the 4 gj-family TUs newly advanced to the wall are `.c` and can never join
  the queue; no overlap with this lane). 3 rows first-block on the FloorPop-class
  `FloorObjectSetRow_size` assert instead (`TODoorAncient08`, `TODoorAncient09`,
  `TODoorCave01`) but still carry the NJS wall behind it — queue membership unaffected.
- Every one of the **509** remaining NA §3 dialect sites fires in the fresh sweep
  (python line-match: 509/509 hit, 0 orphan sites).

## 1. Queue reconciliation (python re-read of NA §3 by `aqp2_tally.py`, all asserts pass)

```
NA §3 rows                = 169 files / 673 enumerated sites   (row-parse assert)
batch-1 (top-30, units.txt)=  30 files / 164 sites              (covered, §2 of that doc)
remaining queue           = 139 files / 509 sites               (169-30, 673-164)   OK
fresh apply-8 NJS-shape   = 786 errors across the 139 files     (== NA totals sum)  OK
class split of the 786    = adapter 558 / no-matching-function CASCADE 197 /
                            static-init 30 / layout-assert other 1
un-enumerated lines       = 229 fresh error lines NOT on any NA site line, of which
                            197 are cascade sites (§3.2) — batch-1 §5.2's class, picked up here
```

Class taxonomy per line (mechanical, from message text): *adapter* = the five AV §3
shapes' error faces (int↔ptr convert, member-ref/subscript/cast on a `u32` slot);
*cascade* = `no matching function for call to` on slot-typed call args; *static-init* =
`cannot initialize a member subobject` at file scope (expression adapters cannot reach
initialisers — batch-1 §5.1, split adjudicated in §4); *other* = layout-assert tails.

## 2. Batch-3 selection rule (inherits batch-1 §1 tiers)

Remaining 139 rows sit in AQP §1 tiers 3–6 (system/map/ui, player, object, item/effect/
technique/weapon/enemy/boss) plus tier-0/1/2 leftovers (`TLanguage.cpp`, render/unk rows
outside the top-30). Batch-3 order within tier: cascade+sites desc, then path. One lever
kills most cascade volume: **157 of 197 un-enumerated cascade errors are the single
`draw_ginja_model` arg family** (PK-13 note, batch-1 §5.3) — per-file wrapper-side
`NJS_HANDLE` load at the call, 65 files listed §3.2, `draw_chunk_model` (14) is the same
leg for chunk draw. Authored edits are NOT in this doc's scope (enumeration + split lane).

## 3. Remaining queue n=139 — NA §3 grammar (file | sites | class | first-error)

Sites = the NA §3 enumerated dialect sites (all re-verified firing this turn); class =
this turn's fresh-sweep composition (`adapter:N cascade:N static-init:N other:N`);
first-error = fresh NJS-shape first error line + message family. Full err receipts:
`cache/scratch/aqp2/NJS/err/`.

| file | sites | class | first-error |
|---|---|---|---|
| src/pso/boss/barbaray/TBoss7BreakUp.cpp | 189,194,277,306 | adapter:5 cascade:4 | src/pso/boss/barbaray/TBoss7BreakUp.cpp:189 int-to-ptr assign |
| src/pso/boss/barbaray/TBoss7DeRolLeC.cpp | 505,523,3640,3642,3672,3674,3697,3698 | adapter:8 | src/pso/boss/barbaray/TBoss7DeRolLeC.cpp:505 int-to-ptr assign |
| src/pso/boss/barbaray/TBoss7TentacleNeedle.cpp | 58,60,63 | adapter:3 cascade:1 | src/pso/boss/barbaray/TBoss7TentacleNeedle.cpp:58 ptr-to-int assign |
| src/pso/boss/darkfalz/TBoss4ZakoDraw.cpp | 158,193,196 | adapter:3 | src/pso/boss/darkfalz/TBoss4ZakoDraw.cpp:158 member-ref on slot |
| src/pso/boss/darkfalz/TObjDFDemo1.cpp | 218,234 | adapter:2 | src/pso/boss/darkfalz/TObjDFDemo1.cpp:218 int-to-ptr assign |
| src/pso/boss/darkfalz/TObjDFJisin.cpp | 348,351 | adapter:2 | src/pso/boss/darkfalz/TObjDFJisin.cpp:348 slot-to-param |
| src/pso/boss/derolle/TBoss2BreakUp.cpp | 192,224,303,306 | adapter:4 cascade:4 | src/pso/boss/derolle/TBoss2BreakUp.cpp:192 slot-to-ptr init |
| src/pso/boss/derolle/TBoss2DeRolLe.cpp | 491,494,495,498,500,502,520,3759,3761 | adapter:9 | src/pso/boss/derolle/TBoss2DeRolLe.cpp:491 int-to-ptr assign |
| src/pso/boss/derolle/TBoss2Mine.cpp | 180,185,190 | adapter:3 | src/pso/boss/derolle/TBoss2Mine.cpp:180 slot-to-ptr init |
| src/pso/boss/derolle/TBoss2Tentacle.cpp | 413,415,418 | adapter:3 cascade:1 | src/pso/boss/derolle/TBoss2Tentacle.cpp:413 ptr-to-int assign |
| src/pso/boss/derolle/TObjBoss2Tunnel_8003B138.cpp | 125,127,564,565,644 | adapter:5 | src/pso/boss/derolle/TObjBoss2Tunnel_8003B138.cpp:125 member-ref on slot |
| src/pso/boss/dragon/TBoss1Dragon.cpp | 382,384,407,409,410,413,414,572,574,594,595 | adapter:11 | src/pso/boss/dragon/TBoss1Dragon.cpp:382 slot-to-ptr init |
| src/pso/boss/dragon/TObjBoss1MokoHill.cpp | 174,175,181,259,334 | adapter:5 | src/pso/boss/dragon/TObjBoss1MokoHill.cpp:174 ptr-to-int assign |
| src/pso/boss/galgryphon/TBoss5GryphonLightningEff.cpp | 52,54 | adapter:2 | src/pso/boss/galgryphon/TBoss5GryphonLightningEff.cpp:52 slot subscript |
| src/pso/boss/galgryphon/TObjBoss5FloorStone.cpp | 26 | adapter:2 | src/pso/boss/galgryphon/TObjBoss5FloorStone.cpp:26 int-to-ptr assign |
| src/pso/boss/galgryphon/TObjBoss5PillarCtrl.cpp | 46,55,57 | adapter:4 | src/pso/boss/galgryphon/TObjBoss5PillarCtrl.cpp:46 int-to-ptr assign |
| src/pso/boss/galgryphon/TObjBoss5PillarSmoke.cpp | 105,132,137 | adapter:4 | src/pso/boss/galgryphon/TObjBoss5PillarSmoke.cpp:105 int-to-ptr assign |
| src/pso/boss/goldragon/TBoss8Dragon.cpp | 318,320,321,324,325 | adapter:5 | src/pso/boss/goldragon/TBoss8Dragon.cpp:318 ptr-to-int assign |
| src/pso/boss/goldragon/TBoss8Dragon_8028A8F8.cpp | 15 | adapter:2 | src/pso/boss/goldragon/TBoss8Dragon_8028A8F8.cpp:15 slot-to-ptr init |
| src/pso/boss/olgaflow/TBoss6Type2.cpp | 680 | adapter:1 cascade:1 other:1 | src/pso/boss/olgaflow/TBoss6Type2.cpp:342 layout assert |
| src/pso/boss/volopt/TBoss3Volopt.cpp | 407,408 | adapter:2 | src/pso/boss/volopt/TBoss3Volopt.cpp:407 int-to-ptr assign |
| src/pso/boss/volopt/TBoss3VoloptMonitor.cpp | 203,323 | adapter:2 static-init:26 | src/pso/boss/volopt/TBoss3VoloptMonitor.cpp:77 static-init slot |
| src/pso/boss/volopt/TBoss3VoloptP01.cpp | 184 | adapter:1 | src/pso/boss/volopt/TBoss3VoloptP01.cpp:184 cast-from-slot |
| src/pso/boss/volopt/TBoss3VoloptP02.cpp | 293,294,295 | adapter:3 | src/pso/boss/volopt/TBoss3VoloptP02.cpp:293 cast-from-slot |
| src/pso/effect/TBreakUpMdl.cpp | 110,113,116,134,137,140 | adapter:6 | src/pso/effect/TBreakUpMdl.cpp:110 int-to-ptr assign |
| src/pso/effect/TEF_PvrWave.cpp | 9,10,11,24,25,26,27,28,29 | adapter:12 | src/pso/effect/TEF_PvrWave.cpp:9 slot subscript |
| src/pso/effect/TEffIndirectCtrl.cpp | 360,365 | adapter:2 | src/pso/effect/TEffIndirectCtrl.cpp:360 ptr-to-int assign |
| src/pso/effect/TEffect.cpp | 76,77,78,82,83,84,98 | adapter:13 | src/pso/effect/TEffect.cpp:76 slot subscript |
| src/pso/effect/TKomorebi.cpp | 46,108,110,111,113,114 | adapter:6 | src/pso/effect/TKomorebi.cpp:46 cast-from-slot |
| src/pso/effect/enemy/TeneFakeTecThunderEff.cpp | 279 | adapter:1 | src/pso/effect/enemy/TeneFakeTecThunderEff.cpp:279 slot subscript |
| src/pso/effect/item/TItemEffPG.cpp | 208 | adapter:1 cascade:1 | src/pso/effect/item/TItemEffPG.cpp:203 no-matching-fn |
| src/pso/enemy/TObjEnemyCustom.cpp | 323,328,342 | adapter:3 cascade:1 | src/pso/enemy/TObjEnemyCustom.cpp:323 int-to-ptr assign |
| src/pso/enemy/balclaw/TObjEneBalClaw.cpp | 169 | adapter:1 | src/pso/enemy/balclaw/TObjEneBalClaw.cpp:169 int-to-ptr assign |
| src/pso/enemy/berura/TObjEneRe7Berura.cpp | 222,482,485,515,517 | adapter:5 cascade:1 | src/pso/enemy/berura/TObjEneRe7Berura.cpp:222 int-to-ptr assign |
| src/pso/enemy/bringer/TObjEneDf2Bringer.cpp | 1344,1345,1371 | adapter:3 | src/pso/enemy/bringer/TObjEneDf2Bringer.cpp:1344 slot-to-ptr init |
| src/pso/enemy/darkgunner/DarkGunner.cpp | 377,378 | adapter:2 | src/pso/enemy/darkgunner/DarkGunner.cpp:377 member-ref on slot |
| src/pso/enemy/dellbiter/TObjEneDellBiter.cpp | 169 | adapter:1 | src/pso/enemy/dellbiter/TObjEneDellBiter.cpp:169 member-ref on slot |
| src/pso/enemy/dimedian/TObjEneDimedian.cpp | 234,283,418,419,420 | adapter:6 | src/pso/enemy/dimedian/TObjEneDimedian.cpp:234 member-ref on slot |
| src/pso/enemy/dolmolm/TObjEneDolmOlm.cpp | 373,411 | adapter:3 | src/pso/enemy/dolmolm/TObjEneDolmOlm.cpp:373 member-ref on slot |
| src/pso/enemy/dubchik/TOCtrlDubchik.cpp | 138,143,148,154 | adapter:4 cascade:5 | src/pso/enemy/dubchik/TOCtrlDubchik.cpp:136 no-matching-fn |
| src/pso/enemy/epsilon/TObjEneEpsilon.cpp | 75,76,77,78,79,80,81 | adapter:7 | src/pso/enemy/epsilon/TObjEneEpsilon.cpp:75 member-ref on slot |
| src/pso/enemy/flower/TObjEneRe2Flower_800C20D0.cpp | 57,58,59,60,74,75,108,109,110,111,161,162 | adapter:16 | src/pso/enemy/flower/TObjEneRe2Flower_800C20D0.cpp:57 slot subscript |
| src/pso/enemy/flower/TObjEneRe2Flower_EffCurse3.cpp | 89 | adapter:1 | src/pso/enemy/flower/TObjEneRe2Flower_EffCurse3.cpp:89 ptr-to-int assign |
| src/pso/enemy/mericarol/TObjEneBm9Mericarol.cpp | 1200,1201,1202,1203,1204,1205,1208,1209,1210,1211,1212,1213,1227,1257,1265,1266,1267,1280,1281,1282,1283,1284,1285,1288,1289,1290,1291,1292,1293,1308,1309,1310 | adapter:45 | src/pso/enemy/mericarol/TObjEneBm9Mericarol.cpp:1200 slot subscript |
| src/pso/enemy/morfos/TObjEneMorfos.cpp | 152,153 | adapter:2 cascade:2 | src/pso/enemy/morfos/TObjEneMorfos.cpp:152 member-ref on slot |
| src/pso/enemy/saver/SaverLifecycle.cpp | 140,142,700,701 | adapter:4 | src/pso/enemy/saver/SaverLifecycle.cpp:140 slot assign |
| src/pso/enemy/shark/TObjEneShark.cpp | 245,247,256,262,325,326,334 | adapter:12 | src/pso/enemy/shark/TObjEneShark.cpp:245 member-ref on slot |
| src/pso/enemy/shinowa/TObjEneMe3Shinowa.cpp | 167,169,172,1592,1594,1638,1640 | adapter:9 cascade:2 | src/pso/enemy/shinowa/TObjEneMe3Shinowa.cpp:167 ptr-to-int assign |
| src/pso/enemy/sinowzoa/TObjEneMe3SinowZoa.cpp | 170,172,175,197,203,209,1813,1861 | adapter:10 cascade:2 | src/pso/enemy/sinowzoa/TObjEneMe3SinowZoa.cpp:170 ptr-to-int assign |
| src/pso/enemy/sorcerer/TObjEneRe4Bit.cpp | 535,562 | adapter:2 | src/pso/enemy/sorcerer/TObjEneRe4Bit.cpp:535 int-to-ptr assign |
| src/pso/enemy/stelth/TObjEneMe3Stelth.cpp | 197,199,202,1753,1803 | adapter:7 cascade:2 | src/pso/enemy/stelth/TObjEneMe3Stelth.cpp:197 ptr-to-int assign |
| src/pso/item/TItemLoad.cpp | 487,552 | adapter:2 | src/pso/item/TItemLoad.cpp:487 slot subscript |
| src/pso/item/TItemReverseTech.cpp | 162,168 | adapter:2 | src/pso/item/TItemReverseTech.cpp:162 ptr-to-int assign |
| src/pso/object/TOHealGeneral.cpp | 191,197 | adapter:2 cascade:3 | src/pso/object/TOHealGeneral.cpp:189 no-matching-fn |
| src/pso/object/TObjComputer.cpp | 164 | adapter:1 cascade:1 | src/pso/object/TObjComputer.cpp:164 member-ref on slot |
| src/pso/object/TObjDoaEx01.cpp | 160,164,251,253 | adapter:4 cascade:6 | src/pso/object/TObjDoaEx01.cpp:151 no-matching-fn |
| src/pso/object/TObjEZBoneRotBase.cpp | 83,85,116,118 | adapter:4 | src/pso/object/TObjEZBoneRotBase.cpp:83 slot-to-param |
| src/pso/object/TObjLiveMonitor.cpp | 404,415,420 | adapter:3 cascade:1 | src/pso/object/TObjLiveMonitor.cpp:402 no-matching-fn |
| src/pso/object/TObjTailsTailsMove.cpp | 79,81 | adapter:2 | src/pso/object/TObjTailsTailsMove.cpp:79 slot-to-ptr init |
| src/pso/object/TObjTombstone.cpp | 458,462,463 | adapter:3 | src/pso/object/TObjTombstone.cpp:458 member-ref on slot |
| src/pso/object/base/TObjMotion.cpp | 404,410 | adapter:2 | src/pso/object/base/TObjMotion.cpp:404 int-to-ptr assign |
| src/pso/object/cave/TOCakeshopCave01.cpp | 72,82 | adapter:2 cascade:2 | src/pso/object/cave/TOCakeshopCave01.cpp:71 no-matching-fn |
| src/pso/object/cave/TOHangceilingCave01.cpp | 217,220,223,227,230,233 | adapter:6 | src/pso/object/cave/TOHangceilingCave01.cpp:217 member-ref on slot |
| src/pso/object/cave/TORevlightCave01.cpp | 44,46 | adapter:2 cascade:1 | src/pso/object/cave/TORevlightCave01.cpp:42 no-matching-fn |
| src/pso/object/cave/TOSignCave03.cpp | 34,39,43,51,56,60,68,73,80,85,89 | adapter:11 cascade:12 | src/pso/object/cave/TOSignCave03.cpp:32 no-matching-fn |
| src/pso/object/city/CitySeasonResources.cpp | 238,242,263 | adapter:5 | src/pso/object/city/CitySeasonResources.cpp:238 member-ref on slot |
| src/pso/object/city/CityTraffic.cpp | 392,400 | adapter:2 | src/pso/object/city/CityTraffic.cpp:392 int-to-ptr assign |
| src/pso/object/container/TOContainerAncientEnemy.cpp | 270,275,429,434 | adapter:4 cascade:6 | src/pso/object/container/TOContainerAncientEnemy.cpp:269 no-matching-fn |
| src/pso/object/door/TODoorAncient01.cpp | 105,111,116 | adapter:3 cascade:4 | src/pso/object/door/TODoorAncient01.cpp:104 no-matching-fn |
| src/pso/object/door/TODoorAncient03.cpp | 105,111,116 | adapter:3 cascade:4 | src/pso/object/door/TODoorAncient03.cpp:104 no-matching-fn |
| src/pso/object/door/TODoorAncient04.cpp | 105,111,116 | adapter:3 cascade:4 | src/pso/object/door/TODoorAncient04.cpp:104 no-matching-fn |
| src/pso/object/door/TODoorAncient05.cpp | 115 | adapter:1 cascade:1 | src/pso/object/door/TODoorAncient05.cpp:113 no-matching-fn |
| src/pso/object/door/TODoorAncient06.cpp | 116 | adapter:1 cascade:1 | src/pso/object/door/TODoorAncient06.cpp:114 no-matching-fn |
| src/pso/object/door/TODoorAncient07.cpp | 121 | adapter:1 cascade:1 | src/pso/object/door/TODoorAncient07.cpp:119 no-matching-fn |
| src/pso/object/door/TODoorAncient08.cpp | 208,209,210,211,212,213,214,215,241 | adapter:9 cascade:3 | src/pso/object/door/TODoorAncient08.cpp:208 member-ref on slot |
| src/pso/object/door/TODoorAncient09.cpp | 206,207,208,209,225 | adapter:5 cascade:3 | src/pso/object/door/TODoorAncient09.cpp:206 member-ref on slot |
| src/pso/object/door/TODoorCave01.cpp | 194,205,228,232 | adapter:4 cascade:3 | src/pso/object/door/TODoorCave01.cpp:193 no-matching-fn |
| src/pso/object/door/TODoorCave02.cpp | 95,100,117,118 | adapter:4 cascade:3 | src/pso/object/door/TODoorCave02.cpp:88 no-matching-fn |
| src/pso/object/door/TODoorCave03.cpp | 113,115,119 | adapter:3 cascade:3 | src/pso/object/door/TODoorCave03.cpp:107 no-matching-fn |
| src/pso/object/door/TODoorFourLightSpace.cpp | 151,353,354,356,357,358,359 | adapter:7 cascade:5 | src/pso/object/door/TODoorFourLightSpace.cpp:150 no-matching-fn |
| src/pso/object/door/TODoorLabo.cpp | 200,206 | adapter:2 cascade:1 | src/pso/object/door/TODoorLabo.cpp:197 no-matching-fn |
| src/pso/object/door/TODoorMachine01.cpp | 154,175,181,183,189,191,196,202 | adapter:8 cascade:5 | src/pso/object/door/TODoorMachine01.cpp:153 no-matching-fn |
| src/pso/object/door/TODoorMachine02.cpp | 86,96,101,109 | adapter:4 cascade:4 | src/pso/object/door/TODoorMachine02.cpp:86 int-to-ptr assign |
| src/pso/object/door/TODoorVS2Door01.cpp | 85,94,102,110,118 | adapter:5 cascade:6 | src/pso/object/door/TODoorVS2Door01.cpp:83 no-matching-fn |
| src/pso/object/door/TODoorVoShip.cpp | 84,91,98 | adapter:3 cascade:3 | src/pso/object/door/TODoorVoShip.cpp:84 int-to-ptr assign |
| src/pso/object/door/TObjDoorJung.cpp | 75,76,77,78,79,80,88 | adapter:7 | src/pso/object/door/TObjDoorJung.cpp:75 int-to-ptr assign |
| src/pso/object/door/TObjDoorJung_8023DBEC.cpp | 12 | adapter:1 cascade:1 | src/pso/object/door/TObjDoorJung_8023DBEC.cpp:12 int-to-ptr assign |
| src/pso/object/door/TObjDoorKey.cpp | 117 | adapter:1 | src/pso/object/door/TObjDoorKey.cpp:117 cast-from-slot |
| src/pso/object/fence/TLazerFenceSw.cpp | 95,245,247,249,251,253 | adapter:6 | src/pso/object/fence/TLazerFenceSw.cpp:95 cast-from-slot |
| src/pso/object/fence/TOVS2Wall01.cpp | 282,391 | adapter:2 | src/pso/object/fence/TOVS2Wall01.cpp:282 cast-from-slot |
| src/pso/object/fence/TObjLazerFence4.cpp | 91,94,95 | adapter:3 | src/pso/object/fence/TObjLazerFence4.cpp:91 int-to-ptr assign |
| src/pso/object/fence/TObjLazerFenceNorm.cpp | 240,242,243,245,246 | adapter:5 | src/pso/object/fence/TObjLazerFenceNorm.cpp:240 int-to-ptr assign |
| src/pso/object/key/TODummyKeyCave01.cpp | 135,143 | adapter:2 cascade:3 | src/pso/object/key/TODummyKeyCave01.cpp:127 no-matching-fn |
| src/pso/object/key/TOKeyAncient02.cpp | 50,58,70 | adapter:3 cascade:5 | src/pso/object/key/TOKeyAncient02.cpp:49 no-matching-fn |
| src/pso/object/key/TOKeyAncient03.cpp | 141,146 | adapter:2 cascade:3 | src/pso/object/key/TOKeyAncient03.cpp:134 no-matching-fn |
| src/pso/object/key/TOKeyCave01.cpp | 185,197 | adapter:2 cascade:3 | src/pso/object/key/TOKeyCave01.cpp:177 no-matching-fn |
| src/pso/object/key/TOKeyMachine01.cpp | 181,186 | adapter:2 cascade:3 | src/pso/object/key/TOKeyMachine01.cpp:173 no-matching-fn |
| src/pso/object/lab/TOCapsuleLabo.cpp | 72,77 | adapter:2 cascade:2 | src/pso/object/lab/TOCapsuleLabo.cpp:71 no-matching-fn |
| src/pso/object/lab/TObjLaboDesignBase.cpp | 168,174 | adapter:2 cascade:1 | src/pso/object/lab/TObjLaboDesignBase.cpp:165 no-matching-fn |
| src/pso/object/lobby/LobbyGoGoBall.cpp | 931,933,938,942 | adapter:4 | src/pso/object/lobby/LobbyGoGoBall.cpp:931 slot-to-param |
| src/pso/object/lobby/TObjLobbyMain.cpp | 118,120 | adapter:2 | src/pso/object/lobby/TObjLobbyMain.cpp:118 member-ref on slot |
| src/pso/object/mine/TOCapsuleMachine01.cpp | 71 | adapter:1 | src/pso/object/mine/TOCapsuleMachine01.cpp:71 member-ref on slot |
| src/pso/object/mine/TOComputerMachine01.cpp | 58,60,62,64,90,91,92,93,94 | adapter:9 cascade:4 | src/pso/object/mine/TOComputerMachine01.cpp:56 no-matching-fn |
| src/pso/object/mine/TOHangerMachine01.cpp | 43,48,51,54,64,67,77 | adapter:7 cascade:8 | src/pso/object/mine/TOHangerMachine01.cpp:42 no-matching-fn |
| src/pso/object/mine/TOLightMachine01.cpp | 56 | adapter:1 cascade:1 | src/pso/object/mine/TOLightMachine01.cpp:52 no-matching-fn |
| src/pso/object/mine/TOMonitorMachine01.cpp | 84,90,97,105,111,118,150,151,152 | adapter:9 cascade:6 | src/pso/object/mine/TOMonitorMachine01.cpp:84 int-to-ptr assign |
| src/pso/object/mine/TOSparkMachine01.cpp | 77,88 | adapter:2 cascade:1 | src/pso/object/mine/TOSparkMachine01.cpp:77 int-to-ptr assign |
| src/pso/object/ruins/TOCapsuleAncient01.cpp | 44 | adapter:1 cascade:2 | src/pso/object/ruins/TOCapsuleAncient01.cpp:41 no-matching-fn |
| src/pso/object/ruins/TOMonumentAncient02.cpp | 154 | adapter:1 cascade:2 | src/pso/object/ruins/TOMonumentAncient02.cpp:139 no-matching-fn |
| src/pso/object/seabed/TObjSuisoChip.cpp | 369,409,414,521,533 | adapter:5 | src/pso/object/seabed/TObjSuisoChip.cpp:369 cast-from-slot |
| src/pso/object/switch/TOSensorAncient01.cpp | 79,87,95,104,111,116,121,126 | adapter:8 cascade:9 | src/pso/object/switch/TOSensorAncient01.cpp:76 no-matching-fn |
| src/pso/object/switch/TOSensorGeneral01.cpp | 70,74,78,83 | adapter:4 cascade:5 | src/pso/object/switch/TOSensorGeneral01.cpp:68 no-matching-fn |
| src/pso/object/trap/TOTrapAncient01.cpp | 180 | adapter:1 cascade:2 | src/pso/object/trap/TOTrapAncient01.cpp:179 no-matching-fn |
| src/pso/object/trap/TOTrapAncient02.cpp | 182,197,204,247,252,257,280,289,298 | adapter:9 cascade:8 | src/pso/object/trap/TOTrapAncient02.cpp:182 cast-from-slot |
| src/pso/object/trap/TOTrapSeabed01.cpp | 264,267,275 | adapter:3 cascade:2 | src/pso/object/trap/TOTrapSeabed01.cpp:257 no-matching-fn |
| src/pso/object/warp/TObjAreaWarpEndingJung.cpp | 666,671,682,690,758,763,774,782,877,891 | adapter:12 cascade:6 | src/pso/object/warp/TObjAreaWarpEndingJung.cpp:666 int-to-ptr assign |
| src/pso/object/warp/TObjWarpJungMain.cpp | 289,290,291,292,293,294,295,296 | adapter:8 | src/pso/object/warp/TObjWarpJungMain.cpp:289 int-to-ptr assign |
| src/pso/player/GinjaRenderTarget.cpp | 13 | adapter:1 cascade:1 | src/pso/player/GinjaRenderTarget.cpp:13 ptr-to-int assign |
| src/pso/system/TLanguage.cpp | 49 | adapter:1 | src/pso/system/TLanguage.cpp:49 member-ref on slot |
| src/pso/technique/TTecCtrl.cpp | 185,189 | adapter:2 | src/pso/technique/TTecCtrl.cpp:185 member-ref on slot |
| src/pso/technique/TTecZondeEff.cpp | 294,381,383 | adapter:3 | src/pso/technique/TTecZondeEff.cpp:294 slot subscript |
| src/pso/ui/charcreate/TChCrLoadAfsTextureReq.cpp | 42,112 | adapter:2 | src/pso/ui/charcreate/TChCrLoadAfsTextureReq.cpp:42 slot subscript |
| src/pso/ui/frontend/TAdMcSelectCursorGC.cpp | 140 | adapter:1 static-init:2 | src/pso/ui/frontend/TAdMcSelectCursorGC.cpp:20 static-init slot |
| src/pso/ui/frontend/TAdMcSelectMcGC.cpp | 112 | adapter:1 static-init:2 | src/pso/ui/frontend/TAdMcSelectMcGC.cpp:14 static-init slot |
| src/pso/ui/frontend/TAdPlyStatusGC.cpp | 81 | adapter:1 | src/pso/ui/frontend/TAdPlyStatusGC.cpp:81 member-ref on slot |
| src/pso/ui/window/TWindowDD_8034435C.cpp | 2566 | adapter:1 | src/pso/ui/window/TWindowDD_8034435C.cpp:2566 slot subscript |
| src/pso/ui/window/TWindowDD_WS_Word.cpp | 346 | adapter:1 | src/pso/ui/window/TWindowDD_WS_Word.cpp:346 slot subscript |
| src/pso/ui/window/TWindowKeyBoardHard_801FBA48.cpp | 54,60 | adapter:2 cascade:1 | src/pso/ui/window/TWindowKeyBoardHard_801FBA48.cpp:51 no-matching-fn |
| src/pso/unk/SampledMotionPoints.cpp | 59 | adapter:1 cascade:1 | src/pso/unk/SampledMotionPoints.cpp:56 no-matching-fn |
| src/pso/unk/text_80066F5C.cpp | 21 | adapter:1 cascade:2 | src/pso/unk/text_80066F5C.cpp:21 slot-to-param |
| src/pso/unk/text_803279CC.cpp | 10,15 | adapter:2 cascade:1 | src/pso/unk/text_803279CC.cpp:10 cast-from-slot |
| src/pso/weapon/SpecialProjectileWeapons.cpp | 119,120 | adapter:2 cascade:1 | src/pso/weapon/SpecialProjectileWeapons.cpp:119 member-ref on slot |
| src/pso/weapon/TItemWeaponBelraCannon.cpp | 31 | adapter:1 | src/pso/weapon/TItemWeaponBelraCannon.cpp:31 slot-to-ptr init |
| src/pso/weapon/TItemWeaponDarkFlow.cpp | 38 | adapter:1 | src/pso/weapon/TItemWeaponDarkFlow.cpp:38 member-ref on slot |
| src/pso/weapon/TItemWeaponDrillLauncher.cpp | 45,125 | adapter:3 | src/pso/weapon/TItemWeaponDrillLauncher.cpp:45 member-ref on slot |
| src/pso/weapon/TItemWeaponPanzerFaust.cpp | 61 | adapter:1 | src/pso/weapon/TItemWeaponPanzerFaust.cpp:61 int-to-ptr assign |
| src/pso/weapon/TItemWeaponSRankWindmill.cpp | 23 | adapter:1 | src/pso/weapon/TItemWeaponSRankWindmill.cpp:23 member-ref on slot |
| src/pso/weapon/TItemWeaponStrikerOfChao.cpp | 21,23,25 | adapter:3 | src/pso/weapon/TItemWeaponStrikerOfChao.cpp:21 slot-to-ptr init |
| src/pso/weapon/TItemWeaponWindmill.cpp | 88 | adapter:1 | src/pso/weapon/TItemWeaponWindmill.cpp:88 member-ref on slot |

### 3.2 Un-enumerated cascade sites picked up by batch-2 (197 sites, 65 files)

Lines carrying `no matching function` that NA §3 did NOT enumerate (call-arg legs behind
the enumerated conversions; same AV §3 shapes, new sites — these ADD to each file's row
above; not counted in the 509 reconciliation, which is NA-grammar-only):

| file | un-enumerated cascade sites |
|---|---|
| src/pso/boss/barbaray/TBoss7BreakUp.cpp | 202,211,291,300 | 
| src/pso/boss/barbaray/TBoss7TentacleNeedle.cpp | 64 | 
| src/pso/boss/derolle/TBoss2BreakUp.cpp | 209,218,316,325 | 
| src/pso/boss/derolle/TBoss2Tentacle.cpp | 419 | 
| src/pso/boss/olgaflow/TBoss6Type2.cpp | 681 | 
| src/pso/effect/item/TItemEffPG.cpp | 203 | 
| src/pso/enemy/TObjEnemyCustom.cpp | 340 | 
| src/pso/enemy/berura/TObjEneRe7Berura.cpp | 511 | 
| src/pso/enemy/dubchik/TOCtrlDubchik.cpp | 136,140,145,151,157 | 
| src/pso/enemy/morfos/TObjEneMorfos.cpp | 1266,1270 | 
| src/pso/enemy/shinowa/TObjEneMe3Shinowa.cpp | 277,405 | 
| src/pso/enemy/sinowzoa/TObjEneMe3SinowZoa.cpp | 240,437 | 
| src/pso/enemy/stelth/TObjEneMe3Stelth.cpp | 288,436 | 
| src/pso/object/TOHealGeneral.cpp | 189,194,200 | 
| src/pso/object/TObjComputer.cpp | 174 | 
| src/pso/object/TObjDoaEx01.cpp | 151,156,173,174,187,188 | 
| src/pso/object/TObjLiveMonitor.cpp | 402 | 
| src/pso/object/cave/TOCakeshopCave01.cpp | 71,79 | 
| src/pso/object/cave/TORevlightCave01.cpp | 42 | 
| src/pso/object/cave/TOSignCave03.cpp | 32,36,41,45,53,58,62,70,75,82,87,91 | 
| src/pso/object/container/TOContainerAncientEnemy.cpp | 269,273,279,428,432,438 | 
| src/pso/object/door/TODoorAncient01.cpp | 104,108,114,119 | 
| src/pso/object/door/TODoorAncient03.cpp | 104,108,114,119 | 
| src/pso/object/door/TODoorAncient04.cpp | 104,108,114,119 | 
| src/pso/object/door/TODoorAncient05.cpp | 113 | 
| src/pso/object/door/TODoorAncient06.cpp | 114 | 
| src/pso/object/door/TODoorAncient07.cpp | 119 | 
| src/pso/object/door/TODoorAncient08.cpp | 240,252,256 | 
| src/pso/object/door/TODoorAncient09.cpp | 224,236,240 | 
| src/pso/object/door/TODoorCave01.cpp | 193,204,208 | 
| src/pso/object/door/TODoorCave02.cpp | 88,97,102 | 
| src/pso/object/door/TODoorCave03.cpp | 107,118,121 | 
| src/pso/object/door/TODoorFourLightSpace.cpp | 150,163,169,371,377 | 
| src/pso/object/door/TODoorLabo.cpp | 197 | 
| src/pso/object/door/TODoorMachine01.cpp | 153,157,180,188,201 | 
| src/pso/object/door/TODoorMachine02.cpp | 88,98,103,111 | 
| src/pso/object/door/TODoorVS2Door01.cpp | 83,90,99,107,115,123 | 
| src/pso/object/door/TODoorVoShip.cpp | 88,95,102 | 
| src/pso/object/door/TObjDoorJung_8023DBEC.cpp | 15 | 
| src/pso/object/key/TODummyKeyCave01.cpp | 127,142,145 | 
| src/pso/object/key/TOKeyAncient02.cpp | 49,55,61,69,74 | 
| src/pso/object/key/TOKeyAncient03.cpp | 134,145,150 | 
| src/pso/object/key/TOKeyCave01.cpp | 177,196,200 | 
| src/pso/object/key/TOKeyMachine01.cpp | 173,185,189 | 
| src/pso/object/lab/TOCapsuleLabo.cpp | 71,76 | 
| src/pso/object/lab/TObjLaboDesignBase.cpp | 165 | 
| src/pso/object/mine/TOComputerMachine01.cpp | 56,59,61,63 | 
| src/pso/object/mine/TOHangerMachine01.cpp | 42,46,50,53,61,66,74,80 | 
| src/pso/object/mine/TOLightMachine01.cpp | 52 | 
| src/pso/object/mine/TOMonitorMachine01.cpp | 87,94,101,108,115,122 | 
| src/pso/object/mine/TOSparkMachine01.cpp | 84 | 
| src/pso/object/ruins/TOCapsuleAncient01.cpp | 41,48 | 
| src/pso/object/ruins/TOMonumentAncient02.cpp | 139,161 | 
| src/pso/object/switch/TOSensorAncient01.cpp | 76,84,92,100,108,113,118,123,128 | 
| src/pso/object/switch/TOSensorGeneral01.cpp | 68,73,77,81,88 | 
| src/pso/object/trap/TOTrapAncient01.cpp | 179,184 | 
| src/pso/object/trap/TOTrapAncient02.cpp | 245,249,254,260,277,285,294,304 | 
| src/pso/object/trap/TOTrapSeabed01.cpp | 257,260 | 
| src/pso/object/warp/TObjAreaWarpEndingJung.cpp | 680,688,772,780,889,896 | 
| src/pso/player/GinjaRenderTarget.cpp | 16 | 
| src/pso/ui/window/TWindowKeyBoardHard_801FBA48.cpp | 51 | 
| src/pso/unk/SampledMotionPoints.cpp | 56 | 
| src/pso/unk/text_80066F5C.cpp | 24,26 | 
| src/pso/unk/text_803279CC.cpp | 13 | 
| src/pso/weapon/SpecialProjectileWeapons.cpp | 134 | 

### 3.3 Batch-1-file residuals (the other half of batch-1 §5.2's pick-up) — 14 files, 41 supply sites

Fresh python-parse of the LANE RECEIPT sweep (`cache/scratch/aqp/AFTER/err/`, apply-7 shape;
the 64-error static-init cluster rows are shown but adjudicated in §4): the 27 enumerated
cascade sites below join the batch-3 supply for files ALREADY in batch-1 (their 164+27
sites, plus §4's 10 batch-3 links, = the full re-scope of the top-30):

| file | un-enumerated cascade sites | AFTER total | first-error |
|---|---|---|---|
| src/pso/boss/derolle/TObjBoss2Tunnel.cpp | 354 | 7 ({'static-init': 6, 'cascade': 1}) | src/pso/boss/derolle/TObjBoss2Tunnel.cpp:94 static-init slot |
| src/pso/item/TItem.cpp | 871,896,1516 | 3 ({'cascade': 3}) | src/pso/item/TItem.cpp:871 no-matching-fn |
| src/pso/map/TSetManager.cpp | 158,210 | 2 ({'cascade': 2}) | src/pso/map/TSetManager.cpp:158 no-matching-fn |
| src/pso/render/CameraCapture.cpp | 209,393 | 59 ({'static-init': 57, 'cascade': 2}) | src/pso/render/CameraCapture.cpp:35 static-init slot |
| src/pso/render/ChunkModelDraw.cpp | 102,125,779 | 17 ({'other': 14, 'cascade': 3}) | src/pso/render/ChunkModelDraw.cpp:25 declaration of 'pso_arena_base' has a di |
| src/pso/render/ChunkMotionDraw.cpp | 47,100 | 2 ({'cascade': 2}) | src/pso/render/ChunkMotionDraw.cpp:47 no-matching-fn |
| src/pso/render/GinjaMotionDraw.cpp | 36,73 | 2 ({'cascade': 2}) | src/pso/render/GinjaMotionDraw.cpp:36 no-matching-fn |
| src/pso/render/ModelDrawContext.cpp | 94,117,138 | 3 ({'cascade': 3}) | src/pso/render/ModelDrawContext.cpp:94 no-matching-fn |
| src/pso/render/NinjaUtil.cpp | 62,65,88,91 | 4 ({'cascade': 4}) | src/pso/render/NinjaUtil.cpp:62 no-matching-fn |
| src/pso/render/ShapeMotionDraw.cpp | 48,60 | 2 ({'cascade': 2}) | src/pso/render/ShapeMotionDraw.cpp:48 no-matching-fn |
| src/pso/render/TNodeMatrixRecorder.cpp | 39 | 1 ({'cascade': 1}) | src/pso/render/TNodeMatrixRecorder.cpp:39 no-matching-fn |
| src/pso/ui/font/TNECFontEORFunction.cpp | — | 1 ({'static-init': 1}) | src/pso/ui/font/TNECFontEORFunction.cpp:39 static-init slot |
| src/pso/unk/text_80327844.cpp | 14 | 1 ({'cascade': 1}) | src/pso/unk/text_80327844.cpp:14 no-matching-fn |
| src/pso/unk/text_803DD5E0.cpp | 333 | 1 ({'cascade': 1}) | src/pso/unk/text_803DD5E0.cpp:333 no-matching-fn |

## 4. The 64-error static-initializer cluster — three-way split (per-site, stops HP/AQP3 double-booking)

Membership (python-verified from batch-1 AFTER receipts — the cluster is EXACTLY the 64
`cannot initialize a member subobject` errors, nothing else): `CameraCapture.cpp` 57
(:35–:88 ×54, :91, :94, :97), `TObjBoss2Tunnel.cpp` 6 (:94 ×2, :96, :179 ×2, :181),
`TNECFontEORFunction.cpp` 1 (:39). 57+6+1 = 64 ✓. Batch-1's three adjacent
`no matching function` residuals (CameraCapture :209/:393, TObjBoss2Tunnel :354) are NOT
cluster members (§3.3 cascade rows); ChunkModelDraw's 14 `fmaf`/`pso_arena_base` residuals
are facade legs, not cluster, not queue (§5).

**Adjudication rule** (declared, applied per-site): batch-1 §5.1 named two resolutions —
*era-absolute guest constants* vs *stamp-at-genesis*. A site goes **HP** iff its slot
payload has **no TU-internal runtime home**: the initialiser RHS is a host `.rodata`
string literal whose slot value is only meaningful once the handle program registers/
places that string space (no window class in `compat/njs/handle/pso_handle_windows.h`
covers compiled-in `.rodata` — POOL/IMAGE/HEAP/ARAM/CACHE/RESERVE are all arena spans;
this is a NEW obligation for HP, tag it `W-STATIC-TREE`). A site goes **AQP3/batch-3** iff
the fix shape is ALREADY in batch-1's shipped, probe-verified supply: de-brace the link
(`{0, n}` or `= 0`) + `pso_njs_h2g(&peer)` store in an existing per-file init hook —
exactly `MericarolTexture.cpp:29` / `TCharaMaking.cpp` / `text_803DD5E0` `list->textures =
pso_njs_h2g(entries)` — parse-legal today, runtime validity riding the SAME HG-1 window
binding those 16 shipped-CLEAN files already ride (booking these 10 to HP again is the
double-count this split kills). **Other**: none (see exclusions above).

### List 1 — handle-program stamping (HP's W-* obligations): **54 sites**

`NJS_TEXNAME effect_textures[54]` filename slots, `CameraCapture.cpp:35-88` — every one:
`cannot initialize a member subobject of type 'u32' with an lvalue of type 'const char[N]'`.
String payloads are compiled-in `.rodata`; under the slot law the value must be a guest
handle resolved through a base register — not expressible in a constant initializer
(`pso_njs_h2g` is runtime, `pso_arena_base` extern), and a `(u32)` truncation cast is
precisely NA §4's forbidden host-ptr laundering. Consumers prove g2h resolution:
`TEffect.cpp:98` walks `shared_effect_texture_list.textures[i]`, `text_803DD5E0:89`-style
name walkers `pso_njs_g2h(filename)`; `TMainCursor.cpp:329/:1143` `set_texture_list(&…)`.

| site | initialiser evidence |
|---|---|
| CameraCapture.cpp:35 | filename slot <- string literal `"f008_white_npr01"` |
| CameraCapture.cpp:36 | filename slot <- string literal `"f032_awa_apn01"` |
| CameraCapture.cpp:37 | filename slot <- string literal `"f032_chun_npn01"` |
| CameraCapture.cpp:38 | filename slot <- string literal `"f032_flash_apr01"` |
| CameraCapture.cpp:39 | filename slot <- string literal `"f032_fog_npn01"` |
| CameraCapture.cpp:40 | filename slot <- string literal `"f032_gunbullet_apr01"` |
| CameraCapture.cpp:41 | filename slot <- string literal `"f032_kisekigreen_apn01"` |
| CameraCapture.cpp:42 | filename slot <- string literal `"f032_kisekired_apn01"` |
| CameraCapture.cpp:43 | filename slot <- string literal `"f032_kisekiviolet_apn01"` |
| CameraCapture.cpp:44 | filename slot <- string literal `"f032_lens01_apn01"` |
| CameraCapture.cpp:45 | filename slot <- string literal `"f032_lens02_apn01"` |
| CameraCapture.cpp:46 | filename slot <- string literal `"f032_lens03_apn01"` |
| CameraCapture.cpp:47 | filename slot <- string literal `"f032_lens04_apn01"` |
| CameraCapture.cpp:48 | filename slot <- string literal `"f032_lens05_apn01"` |
| CameraCapture.cpp:49 | filename slot <- string literal `"f032_lens06_apn01"` |
| CameraCapture.cpp:50 | filename slot <- string literal `"f032_lens07_apn01"` |
| CameraCapture.cpp:51 | filename slot <- string literal `"f032_mshield01_apn01"` |
| CameraCapture.cpp:52 | filename slot <- string literal `"f032_mshield02_apn01"` |
| CameraCapture.cpp:53 | filename slot <- string literal `"f032_netload_apr01"` |
| CameraCapture.cpp:54 | filename slot <- string literal `"f032_redgray_apn01"` |
| CameraCapture.cpp:55 | filename slot <- string literal `"f032_senkou_apr01"` |
| CameraCapture.cpp:56 | filename slot <- string literal `"f032_sennkou_apr01"` |
| CameraCapture.cpp:57 | filename slot <- string literal `"f032_shield01_apn01"` |
| CameraCapture.cpp:58 | filename slot <- string literal `"f032_slow_apn01"` |
| CameraCapture.cpp:59 | filename slot <- string literal `"f032_tama_apn01"` |
| CameraCapture.cpp:60 | filename slot <- string literal `"f064_hane_apr01"` |
| CameraCapture.cpp:61 | filename slot <- string literal `"f064_hitfirec_apr01"` |
| CameraCapture.cpp:62 | filename slot <- string literal `"f064_hitfirered_apr01"` |
| CameraCapture.cpp:63 | filename slot <- string literal `"f064_hitmark_apr01"` |
| CameraCapture.cpp:64 | filename slot <- string literal `"f064_hitmarkblue_apr01"` |
| CameraCapture.cpp:65 | filename slot <- string literal `"f064_impacte_apn01"` |
| CameraCapture.cpp:66 | filename slot <- string literal `"f064_lens01resize_apn01"` |
| CameraCapture.cpp:67 | filename slot <- string literal `"f064_megido_apr01"` |
| CameraCapture.cpp:68 | filename slot <- string literal `"f064_monshou_apr01"` |
| CameraCapture.cpp:69 | filename slot <- string literal `"f064_questloadapn16"` |
| CameraCapture.cpp:70 | filename slot <- string literal `"f064_senblue_apn01"` |
| CameraCapture.cpp:71 | filename slot <- string literal `"f064_shield02_apn01"` |
| CameraCapture.cpp:72 | filename slot <- string literal `"f064_shield03_apn01"` |
| CameraCapture.cpp:73 | filename slot <- string literal `"f064_shield04_apn01"` |
| CameraCapture.cpp:74 | filename slot <- string literal `"f064_shield05_apn01"` |
| CameraCapture.cpp:75 | filename slot <- string literal `"f128_bakuhatu_apn16"` |
| CameraCapture.cpp:76 | filename slot <- string literal `"f128_blackfog_nln16"` |
| CameraCapture.cpp:77 | filename slot <- string literal `"f128_feio_aln16"` |
| CameraCapture.cpp:78 | filename slot <- string literal `"f128_fog_aln16"` |
| CameraCapture.cpp:79 | filename slot <- string literal `"f128_granz_aln16"` |
| CameraCapture.cpp:80 | filename slot <- string literal `"f128_hibana_aln16"` |
| CameraCapture.cpp:81 | filename slot <- string literal `"f128_houden_apr16"` |
| CameraCapture.cpp:82 | filename slot <- string literal `"f128_lightning_alr16"` |
| CameraCapture.cpp:83 | filename slot <- string literal `"f128_smoke_apn16"` |
| CameraCapture.cpp:84 | filename slot <- string literal `"f128_worm_alr16"` |
| CameraCapture.cpp:85 | filename slot <- string literal `"f256_zonde_alr16"` |
| CameraCapture.cpp:86 | filename slot <- string literal `"k016_happa_npr01"` |
| CameraCapture.cpp:87 | filename slot <- string literal `"t032_heart_apn01"` |
| CameraCapture.cpp:88 | filename slot <- string literal `"u064_glasshahen_npr01"` |

### List 2 — adapter-shape (batch-3 supply, AQP3): **10 sites** (8 lines)

All are same-TU static→static links with an existing init hook; shipped precedent cited
above. Hook evidence per site: `:94/:97` — `init_capture_texture()` / `init_shadow_map_texture()`
are the very functions whose `set_texture_name` tails sit at CameraCapture :209/:393
(the two lists' link-store + arg-wrap co-land in one edit pair each); `:91` —
`shared_effect_texture_list` first touched by `set_texture_list` call sites (TMainCursor),
storable at the TEffect/ParticleDefinition init leg or a TU-local stamp fn; `:354`-adjacent
TObjBoss2Tunnel links — `debug_chunk_object`/`debug_cylinder_object` (lines :184/:185) are
consumed by construct paths, a TU-local static-init fn (era `__sinit` analogue, NA §5
dialect) binds vertices/strips/model slots; `:39` — insert before the batch-1-sited
`load_texture_list(menu_font_texture_list)` at TNECFontEORFunction :547 (same function,
:545 loop already stores into `menu_font_texnames`).

| site | initialiser evidence |
|---|---|
| CameraCapture.cpp:91 | textures slot <- `effect_textures` (NJS_TEXNAME[54] array address) |
| CameraCapture.cpp:94 | textures slot <- `&shared_capture_texture_entry` |
| CameraCapture.cpp:97 | textures slot <- `&shadow_map_texture_entry` |
| TObjBoss2Tunnel.cpp:94 | vertex_list+polygon_list slots <- `&debug_chunk_a_vertices`, `debug_chunk_a_strips` (2 errors) |
| TObjBoss2Tunnel.cpp:96 | model slot <- `&debug_chunk_a_model` |
| TObjBoss2Tunnel.cpp:179 | vertex_list+polygon_list slots <- `&debug_chunk_b_vertices`, `debug_chunk_b_strips` (2 errors) |
| TObjBoss2Tunnel.cpp:181 | model slot <- `&debug_chunk_b_model` |
| TNECFontEORFunction.cpp:39 | textures slot <- `menu_font_texnames` (NJS_TEXNAME[2] array address) |

### List 3 — genuinely other: **0 sites**

After the §4-preamble exclusions (3 nmf cascades → §3.3; 14 facade legs → not NJS class),
no cluster member falls outside lists 1–2. Declared, not assumed: 54 + 10 + 0 = 64.

### Same-rule riders (NOT part of the 64; fresh in the remaining queue, §0/§3 sweep)

The apply-8 sweep finds 30 static-init errors in 3 of the 139 queue files — same shapes,
same adjudication: **HP 27** string-literal slots (TBoss3VoloptMonitor.cpp:77-101 ×25,
TAdMcSelectCursorGC.cpp:20, TAdMcSelectMcGC.cpp:14) and **batch-3 3** same-TU links
(TBoss3VoloptMonitor.cpp:104 `{…entries, 25}`, TAdMcSelectCursorGC.cpp:21,
TAdMcSelectMcGC.cpp:15 — all three are the exact Mericarol precedent shape). HP's total
named obligation from this lane: 54 + 27 = 81 sites; AQP3's: 10 + 3 = 13 sites.

## 5. Ledger (no double-count)

- Queue: batch-1 30/164 + batch-2 139/509 = 169/673 (NA §3). Fresh totals 786 = apply-8
  truth for the 139; batch-1 BEFORE-sum drift (299→274) stays on those rows only.
- Static-init universe: 64 (batch-1 files) + 30 (queue files) = 94 → HP 81 / batch-3 13 /
  other 0. HP's W-* line-counts (223+37) and NA's gated 260 are untouched — no cluster
  site appears in any RUNTIME-GATED row (grep-verified membership disjoint at §3 parse).
- Un-enumerated cascade supply: 197 (queue files, §3.2) + 27 (batch-1 files, §3.3) = 224
  sites for batch-3 authoring, dominated by the `draw_ginja_model` wrapper leg (157/197).
- LP64 corpus (apply-8): 1,167 PASS / 513 FAIL — this lane lands ZERO flips (inert);
  the 139 all remain FAIL at NJS shape until batch-3 edits + allowlist rows land.

## 6. Slice-generator contract (unchanged, inherits batch-1 §6)

§3/§3.2/§3.3 site lists drive per-file edit authoring (batch-3); §4 list-2 (and riders)
edits may be authored against existing hooks NOW, list-1 (and riders) waits on HP
`W-STATIC-TREE` adjudication; `-Icompat/njs` never on `.c`; three-`-include` order;
facade before host/bringup/include; waiver pin d4382fcd era — never regen. This commit
touches exactly `docs/p2-adapter-queue-2.md` + `compat/njs/adapters/batch2/**`.

## Repro

`cache/scratch/aqp2/` (this lane): `aqp2_probe.sh` + `aqp2_base_probe.sh` (fresh, -P6),
`sweep139.txt` / `sweep139_base.txt`, `NJS/err/` + `BASE/err/` receipts (139 each),
`rem139.txt`, `rows139.json` (per-file class maps, hit/orphan lists), `b1_residual.json`,
`aqp2_tally.py` / `aqp2_emit.py` / `aqp2_doc.py` (this doc's tables + all §0–§5 numbers).
