# P2-NC — NJS-handle residual class: survey, inert 3rd shadow, stage-2 allow-list proposal

Phase-3 lane NC, 2026-11-27. Decomp `pso-decomp @6b38b5383` READ-ONLY; zero
make/ninja — every number is a scratch `clang -fsyntax-only -ferror-limit=0`
probe over the battery-3 corpus (`$TMP/b3/all.txt`, 1,680 units) with AZ's
committed shape runner (H1C = rebind+pin; H3 = + `-Icompat/repin -Icompat/njs`
stack; probe `cache/scratch/nc/nc_probe.sh`, fresh copy of `az_probe.sh`, NC
paths). Inputs: p2-njs-repin-1.md, p2-gx-tex-consumer.md §3–§4,
p2-collision-debt.md §4.2/§4.6/§5, p2-stitch-apply4.md, p2-v-wave.md V5–V7.
Writes: `compat/njs/**` (one new file, inert) + this doc. ZERO build/ edits.

## 1. Exact failing-TU sets (pre-shadow snapshot)

- AZ same-day H3 sweep of the compat/njs state at `36abf57` (err/ + fail.txt,
  1,680 units, 422 fail): **NJS-handle dialect (u32<->pointer) = 125 cpp TUs**
  (collision-debt §4.2 "~112" — measured exact set here); **FloorPopulation.h
  row-assert citing = 21 cpp TUs** (§4.6 "18"); overlap 2
  (TObjLiveMonitor, TODoorAncient08/09 + TODoorCave01 carry BOTH classes: FP
  row-assert + ninja-model dialect). Union = 144 TUs, table in §3.
- Note baseline drift: AZ swept b2_lp64/all.txt — 16 units since renamed away,
  16 new names in b3; all deltas below are computed on the 1,664-unit
  intersection of the two sweeps, never across renamed units.
- The FP class pre-state was exactly 3 negative-array asserts per TU
  (`FloorObjectSetRow_size`@:56, `EnemySetRecordSize`@:60, `LobbyFloorRows_size`@:67
  family; census: 21/21 both of the first two) — NOT dialect errors: consumers
  were assert-blocked *above* their row-walk sites. §4.2's ~112 vs 125 measured:
  +13 = files whose dialect sites were also hidden behind other classes (e.g.
  packet_classes/pin ordering) that still count as NJS-handle residuals.

## 2. Supply: 3rd shadow `compat/njs/pso/map/FloorPopulation.h` (inert)

AR §4.3 pattern, AV §5 V7: era header copied verbatim, ONLY six field TYPES
change to u32 image slots — `FloorObjectSetRow.object` (@0x40),
`EnemySetRecord.enemy` (@0x44), `LobbyFloorRows.object_rows`/`enemy_rows`
(@0x10/@0x14), `LobbyPoolEntry.objects`/`enemies` (@0x00/@0x04). Names/order/
offsets era-true; all era LAYOUT_ASSERTs keep their names (waiver dispatch
untouched) and pass on byte truth; PSO_NJS_BYTE_TRUTH slot-width pins added
(private macro, same lineage); era offsets as comments; ZERO asset bytes.
Conversion helpers come from the NinjaTypes shadow via guard-idempotent
include; access idioms + slot-space-compare rule documented in the header.

### Before/after (same corpus intersection, my sweeps)

```
H3 clean 1258 pre -> 1268 post (on 1664-unit intersection)
FLIPS (H3 fail->clean): 10  all pure row-assert TUs
BREAKAGE (H3 clean->fail): 0
post-sweep err files citing FloorPopulation.h asserts: 0 (class eliminated)
```
Flipped: TObjBoss7Tunnel, TObjBoss5Ctrl, Boss8DemoObjects, TBoss3VoloptCtrl,
TEnemyTaskList, TSetEvtDestroy, PlayerTaskEffects, TPlayerBattleCtrl4P,
TWindowDD_803430BC, TLobby_80207DA8.
Assert-blocked row walkers now break LOUD with enumerated dialect sites instead
(AR §6 posture, zero net errors lost): TSetManager 3->50 (row create/destroy
legs — the heaviest adapter queue), TLobby 11->8, TObjLiveMonitor 7->4,
TODoorAncient08 15->12, TPlayerBattleCtrl 5->2, QuestOpcodeHandlers 5->2,
TObjCollidable 4->1, TLobby_801F3A24 4->1; TLobby_80209524 6->7 (+1: two
`FloorSavedSetRow* = LobbyFloorRows.enemy_rows` loads + 3 pre-existing
pso_la_waived redefinition errors are its own class).
The existing pair was NOT edited (probes showed no mechanical gap in
NinjaTypes.h/GinjaMaterialState.h: every residual needs a per-TU SOURCE-side
adapter, which headers must not launder — loud-failure law AR §5).

## 3. Per-TU table (union of both classes, H3 shape)

sites = first probed dialect error line numbers (decomp-side, adapters land in
ported slices); fix-shape per AV §3 taxonomy; probe-status = az->nc error
count delta on my H3 sweeps (FLIPPED-CLEAN = parses 0 errors post-shadow).

| file | class | sites | fix-shape | probe-status |
|---|---|---|---|---|
| src/pso/boss/barbaray/TBoss7BreakUp.cpp | NJS | 189,194,194,277,306 | g2h-load / NJS_HANDLE slot | 9->9 |
| src/pso/boss/barbaray/TBoss7DeRolLeC.cpp | NJS | 505,523,3640,3642 | g2h-load / NJS_HANDLE slot, h2g store | 8->8 |
| src/pso/boss/barbaray/TBoss7TentacleNeedle.cpp | NJS | 58,60,63 | g2h-load / NJS_HANDLE slot, h2g store | 4->4 |
| src/pso/boss/barbaray/TObjBoss7Tunnel.cpp | FP | - | row-assert only (3rd shadow) | FLIPPED-CLEAN |
| src/pso/boss/darkfalz/TObjDFJisin.cpp | NJS | 348,351 | g2h-load / NJS_HANDLE slot | 2->2 |
| src/pso/boss/derolle/TBoss2BreakUp.cpp | NJS | 192,224,303,306 | g2h-load / NJS_HANDLE slot | 8->8 |
| src/pso/boss/derolle/TBoss2DeRolLe.cpp | NJS | 491,495,498,502,520,3761 | g2h-load / NJS_HANDLE slot, h2g store | 9->9 |
| src/pso/boss/derolle/TBoss2Mine.cpp | NJS | 180,185,190 | g2h-load / NJS_HANDLE slot | 3->3 |
| src/pso/boss/derolle/TBoss2Tentacle.cpp | NJS | 413,415,418 | g2h-load / NJS_HANDLE slot, h2g store | 4->4 |
| src/pso/boss/derolle/TObjBoss2Tunnel.cpp | NJS | 332,336,356 | g2h-load / NJS_HANDLE slot | 10->10 |
| src/pso/boss/derolle/TObjBoss2Tunnel_8003B138.cpp | NJS | 127,564,565 | g2h-load / NJS_HANDLE slot | 5->5 |
| src/pso/boss/dragon/TBoss1Dragon.cpp | NJS | 382,384,409,410,413,414 | g2h-load / NJS_HANDLE slot, h2g store | 12->12 |
| src/pso/boss/dragon/TObjBoss1MokoHill.cpp | NJS | 181 | h2g store | 5->5 |
| src/pso/boss/galgryphon/TObjBoss5Ctrl.cpp | FP | - | row-assert only (3rd shadow) | FLIPPED-CLEAN |
| src/pso/boss/galgryphon/TObjBoss5FloorStone.cpp | NJS | 26,26 | g2h-load / NJS_HANDLE slot | 2->2 |
| src/pso/boss/galgryphon/TObjBoss5PillarCtrl.cpp | NJS | 46,46,55,57 | g2h-load / NJS_HANDLE slot | 4->4 |
| src/pso/boss/galgryphon/TObjBoss5PillarSmoke.cpp | NJS | 105,105,132,137 | g2h-load / NJS_HANDLE slot | 4->4 |
| src/pso/boss/goldragon/Boss8DemoObjects.cpp | FP | - | row-assert only (3rd shadow) | FLIPPED-CLEAN |
| src/pso/boss/goldragon/TBoss8Dragon.cpp | NJS | 318,320,321,324,325 | g2h-load / NJS_HANDLE slot, h2g store | 5->5 |
| src/pso/boss/goldragon/TBoss8Dragon_8028A8F8.cpp | NJS | 15,15 | g2h-load / NJS_HANDLE slot | 2->2 |
| src/pso/boss/volopt/TBoss3VoloptCtrl.cpp | FP | - | row-assert only (3rd shadow) | FLIPPED-CLEAN |
| src/pso/boss/volopt/TBoss3VoloptMonitor.cpp | NJS | - | see residual errs | 28->28 |
| src/pso/effect/TBreakUpMdl.cpp | NJS | 113,116,137,140 | g2h-load / NJS_HANDLE slot | 6->6 |
| src/pso/effect/TEffClitical.cpp | NJS | - | see residual errs | 1->1 |
| src/pso/effect/TEffIndirectCtrl.cpp | NJS | 360 | h2g store | 2->2 |
| src/pso/effect/TKomorebi.cpp | NJS | 108,111,114 | g2h-load / NJS_HANDLE slot | 6->6 |
| src/pso/enemy/TEnemyTaskList.cpp | FP | - | row-assert only (3rd shadow) | FLIPPED-CLEAN |
| src/pso/enemy/TObjEnemyCustom.cpp | NJS | 323,328,340,342 | g2h-load / NJS_HANDLE slot, h2g store | 4->4 |
| src/pso/enemy/berura/TObjEneRe7Berura.cpp | NJS | 482,485,515,517 | g2h-load / NJS_HANDLE slot | 6->6 |
| src/pso/enemy/bringer/TObjEneDf2Bringer.cpp | NJS | 1344,1345 | g2h-load / NJS_HANDLE slot | 3->3 |
| src/pso/enemy/dubchik/TOCtrlDubchik.cpp | NJS | 138,143,148,154 | g2h-load / NJS_HANDLE slot | 9->9 |
| src/pso/enemy/flower/TObjEneRe2Flower_EffCurse3.cpp | NJS | 89 | h2g store | 1->1 |
| src/pso/enemy/saver/SaverLifecycle.cpp | NJS | - | see residual errs | 4->4 |
| src/pso/enemy/shinowa/TObjEneMe3Shinowa.cpp | NJS | 167,1592,1594,1638,1640 | g2h-load / NJS_HANDLE slot, h2g store | 11->11 |
| src/pso/enemy/sinowzoa/TObjEneMe3SinowZoa.cpp | NJS | 170,1813,1861 | g2h-load / NJS_HANDLE slot, h2g store | 12->12 |
| src/pso/enemy/sorcerer/TObjEneRe4Bit.cpp | NJS | 535 | g2h-load / NJS_HANDLE slot | 2->2 |
| src/pso/enemy/stelth/TObjEneMe3Stelth.cpp | NJS | 197,1753,1803 | g2h-load / NJS_HANDLE slot, h2g store | 9->9 |
| src/pso/item/TItem.cpp | NJS | 900 | g2h-load / NJS_HANDLE slot | 8->8 |
| src/pso/item/TItemReverseTech.cpp | NJS | 168 | h2g store | 2->2 |
| src/pso/map/TSetEvtDestroy.cpp | FP | - | row-assert only (3rd shadow) | FLIPPED-CLEAN |
| src/pso/map/TSetManager.cpp | FP | - | row-assert only (3rd shadow) | 3->50 |
| src/pso/network/TLobby.cpp | FP | - | row-assert only (3rd shadow) | 11->8 |
| src/pso/object/TOHealGeneral.cpp | NJS | 191,197 | g2h-load / NJS_HANDLE slot | 5->5 |
| src/pso/object/TObjDoaEx01.cpp | NJS | 160,164,253 | g2h-load / NJS_HANDLE slot | 10->10 |
| src/pso/object/TObjEZBoneRotBase.cpp | NJS | 83,85,116,118 | g2h-load / NJS_HANDLE slot | 4->4 |
| src/pso/object/TObjLiveMonitor.cpp | FP+NJS | 415,420 | g2h-load / NJS_HANDLE slot | 7->4 |
| src/pso/object/TObjSinBoard.cpp | NJS | 116 | h2g store | 2->2 |
| src/pso/object/base/TObjCollidable.cpp | FP | - | row-assert only (3rd shadow) | 4->1 |
| src/pso/object/base/TObjMotion.cpp | NJS | 404,410 | g2h-load / NJS_HANDLE slot | 2->2 |
| src/pso/object/cave/TOCakeshopCave01.cpp | NJS | 72,82 | g2h-load / NJS_HANDLE slot | 4->4 |
| src/pso/object/cave/TOSignCave01.cpp | NJS | - | g2h-load / NJS_HANDLE slot | 17->17 |
| src/pso/object/cave/TOSignCave02.cpp | NJS | - | g2h-load / NJS_HANDLE slot | 17->17 |
| src/pso/object/cave/TOSignCave03.cpp | NJS | 34,39,43,56,60,73 | g2h-load / NJS_HANDLE slot | 23->23 |
| src/pso/object/city/CityTraffic.cpp | NJS | 392 | g2h-load / NJS_HANDLE slot | 2->2 |
| src/pso/object/container/TOContainerAncientEnemy.cpp | NJS | 270,275,429,434 | g2h-load / NJS_HANDLE slot | 11->11 |
| src/pso/object/door/TODoorAncient01.cpp | NJS | 105,111,116 | g2h-load / NJS_HANDLE slot | 7->7 |
| src/pso/object/door/TODoorAncient03.cpp | NJS | 105,111,116 | g2h-load / NJS_HANDLE slot | 7->7 |
| src/pso/object/door/TODoorAncient04.cpp | NJS | 105,111,116 | g2h-load / NJS_HANDLE slot | 7->7 |
| src/pso/object/door/TODoorAncient08.cpp | FP | - | row-assert only (3rd shadow) | 15->12 |
| src/pso/object/door/TODoorAncient09.cpp | FP | - | row-assert only (3rd shadow) | 11->8 |
| src/pso/object/door/TODoorCave01.cpp | FP+NJS | 194,205,228,232 | g2h-load / NJS_HANDLE slot | 11->8 |
| src/pso/object/door/TODoorCave02.cpp | NJS | 95,100,117 | g2h-load / NJS_HANDLE slot | 8->8 |
| src/pso/object/door/TODoorCave03.cpp | NJS | 115,119 | g2h-load / NJS_HANDLE slot | 7->7 |
| src/pso/object/door/TODoorLabo.cpp | NJS | 200,206 | g2h-load / NJS_HANDLE slot | 3->3 |
| src/pso/object/door/TODoorMachine01.cpp | NJS | 154,175,181,189,191,202 | g2h-load / NJS_HANDLE slot | 14->14 |
| src/pso/object/door/TODoorMachine02.cpp | NJS | 86,96,101 | g2h-load / NJS_HANDLE slot | 9->9 |
| src/pso/object/door/TODoorVS2Door01.cpp | NJS | 85,94,102,118 | g2h-load / NJS_HANDLE slot | 11->11 |
| src/pso/object/door/TODoorVoShip.cpp | NJS | 84,91,98 | g2h-load / NJS_HANDLE slot | 6->6 |
| src/pso/object/door/TObjDoorJung.cpp | NJS | 75 | g2h-load / NJS_HANDLE slot | 7->7 |
| src/pso/object/door/TObjDoorJung_8023DBEC.cpp | NJS | 12 | g2h-load / NJS_HANDLE slot | 2->2 |
| src/pso/object/fence/TLazerFenceSw.cpp | NJS | 245,249,253 | g2h-load / NJS_HANDLE slot | 6->6 |
| src/pso/object/fence/TObjLazerFence4.cpp | NJS | 91,95 | g2h-load / NJS_HANDLE slot | 3->3 |
| src/pso/object/fence/TObjLazerFenceNorm.cpp | NJS | 240,243,246 | g2h-load / NJS_HANDLE slot | 5->5 |
| src/pso/object/key/TODummyKeyCave01.cpp | NJS | 135,143 | g2h-load / NJS_HANDLE slot | 5->5 |
| src/pso/object/key/TOKeyAncient02.cpp | NJS | 50,58,70 | g2h-load / NJS_HANDLE slot | 8->8 |
| src/pso/object/key/TOKeyAncient03.cpp | NJS | 141,146 | g2h-load / NJS_HANDLE slot | 5->5 |
| src/pso/object/key/TOKeyCave01.cpp | NJS | 185,197 | g2h-load / NJS_HANDLE slot | 6->6 |
| src/pso/object/key/TOKeyMachine01.cpp | NJS | 181,186 | g2h-load / NJS_HANDLE slot | 6->6 |
| src/pso/object/lab/TOCapsuleLabo.cpp | NJS | 72,77 | g2h-load / NJS_HANDLE slot | 5->5 |
| src/pso/object/lab/TObjLaboDesignBase.cpp | NJS | 168,174 | g2h-load / NJS_HANDLE slot | 3->3 |
| src/pso/object/mine/TOComputerMachine01.cpp | NJS | 58,60,62,90 | g2h-load / NJS_HANDLE slot | 13->13 |
| src/pso/object/mine/TOHangerMachine01.cpp | NJS | 43,48,51,54,67 | g2h-load / NJS_HANDLE slot | 15->15 |
| src/pso/object/mine/TOMonitorMachine01.cpp | NJS | 84,90,97,105,111,118 | g2h-load / NJS_HANDLE slot | 15->15 |
| src/pso/object/mine/TOSparkMachine01.cpp | NJS | 77,88 | g2h-load / NJS_HANDLE slot | 3->3 |
| src/pso/object/ruins/TOCapsuleAncient01.cpp | NJS | 44 | g2h-load / NJS_HANDLE slot | 4->4 |
| src/pso/object/ruins/TOMonumentAncient02.cpp | NJS | 154 | g2h-load / NJS_HANDLE slot | 4->4 |
| src/pso/object/switch/TOSensorAncient01.cpp | NJS | 79,87,95,104,111,116 | g2h-load / NJS_HANDLE slot | 17->17 |
| src/pso/object/switch/TOSensorGeneral01.cpp | NJS | 70,74,78,83 | g2h-load / NJS_HANDLE slot | 9->9 |
| src/pso/object/trap/TOTrapAncient01.cpp | NJS | 180 | g2h-load / NJS_HANDLE slot | 3->3 |
| src/pso/object/trap/TOTrapAncient02.cpp | NJS | 247,252,257,280,289,298 | g2h-load / NJS_HANDLE slot | 17->17 |
| src/pso/object/trap/TOTrapSeabed01.cpp | NJS | 264,267 | g2h-load / NJS_HANDLE slot | 5->5 |
| src/pso/object/warp/TObjAreaWarpEndingJung.cpp | NJS | 666,671,682,690,758,763 | g2h-load / NJS_HANDLE slot | 18->18 |
| src/pso/object/warp/TObjWarpJungMain.cpp | NJS | 289 | g2h-load / NJS_HANDLE slot | 8->8 |
| src/pso/player/GinjaRenderTarget.cpp | NJS | 13 | h2g store | 2->2 |
| src/pso/player/PlayerTaskEffects.cpp | FP | - | row-assert only (3rd shadow) | FLIPPED-CLEAN |
| src/pso/player/TObjPlayer_8023E480.cpp | NJS | 91 | g2h-load / NJS_HANDLE slot | 1->1 |
| src/pso/player/TPlayerBattleCtrl.cpp | FP | - | row-assert only (3rd shadow) | 5->2 |
| src/pso/player/TPlayerBattleCtrl4P.cpp | FP | - | row-assert only (3rd shadow) | FLIPPED-CLEAN |
| src/pso/quest/QuestOpcodeHandlers.cpp | FP | - | row-assert only (3rd shadow) | 5->2 |
| src/pso/render/AltModel.cpp | NJS | 36,65 | h2g store | 3->3 |
| src/pso/render/CameraCapture.cpp | NJS | - | see residual errs | 74->74 |
| src/pso/render/ChunkMotionDraw.cpp | NJS | 50,103 | g2h-load / NJS_HANDLE slot | 6->6 |
| src/pso/render/ChunkTextureStripCollector.cpp | NJS | 20,23,25,81,84,86 | g2h-load / NJS_HANDLE slot | 8->8 |
| src/pso/render/GinjaMotionDraw.cpp | NJS | 39,76 | g2h-load / NJS_HANDLE slot | 6->6 |
| src/pso/render/ModelDrawContext.cpp | NJS | 60,65,121,141 | g2h-load / NJS_HANDLE slot | 11->11 |
| src/pso/render/ShapeMotionDraw.cpp | NJS | 63 | g2h-load / NJS_HANDLE slot | 14->14 |
| src/pso/render/TNodeMatrixRecorder.cpp | NJS | 43,45 | g2h-load / NJS_HANDLE slot | 3->3 |
| src/pso/render/psoLoadTexture.cpp | NJS | 145 | h2g store | 4->4 |
| src/pso/system/scene/TEndingAB.cpp | NJS | - | see residual errs | 60->60 |
| src/pso/system/scene/TEndingCWall.cpp | NJS | - | see residual errs | 3->3 |
| src/pso/technique/TTecGrants.cpp | NJS | - | see residual errs | 1->1 |
| src/pso/technique/TTecZondeEff.cpp | NJS | - | see residual errs | 3->3 |
| src/pso/technique/TechniqueRingwaveList.cpp | NJS | - | see residual errs | 10->10 |
| src/pso/ui/UISprite.cpp | NJS | - | see residual errs | 21->21 |
| src/pso/ui/charcreate/TChCrLoadReq.cpp | NJS | 38,38,39,39 | compare in slot space | 8->8 |
| src/pso/ui/charcreate/TCharaMaking.cpp | NJS | - | see residual errs | 2->2 |
| src/pso/ui/font/TNECFontEORFunction.cpp | NJS | - | see residual errs | 2->2 |
| src/pso/ui/frontend/TAdBackGC.cpp | NJS | - | see residual errs | 9->9 |
| src/pso/ui/frontend/TAdBackTxtGC.cpp | NJS | - | see residual errs | 6->6 |
| src/pso/ui/frontend/TAdChrSelSlotGC.cpp | NJS | - | see residual errs | 4->4 |
| src/pso/ui/frontend/TAdEndText.cpp | NJS | - | see residual errs | 6->6 |
| src/pso/ui/frontend/TAdGameStartGC.cpp | NJS | - | see residual errs | 2->2 |
| src/pso/ui/frontend/TAdMultiOkGC.cpp | NJS | - | see residual errs | 2->2 |
| src/pso/ui/frontend/TAdRayGC.cpp | NJS | - | see residual errs | 41->41 |
| src/pso/ui/frontend/TAdSEGAlogo.cpp | NJS | - | see residual errs | 25->25 |
| src/pso/ui/frontend/TAdSelectTxtGC.cpp | NJS | - | see residual errs | 6->6 |
| src/pso/ui/frontend/TAdTitleGC.cpp | NJS | - | see residual errs | 6->6 |
| src/pso/ui/frontend/TAdTitleTexGC.cpp | NJS | - | see residual errs | 4->4 |
| src/pso/ui/frontend/TTitleScene.cpp | NJS | - | see residual errs | 22->22 |
| src/pso/ui/window/TWindowDD_803430BC.cpp | FP | - | row-assert only (3rd shadow) | FLIPPED-CLEAN |
| src/pso/ui/window/TWindowKeyBoardHard_801FBA48.cpp | NJS | 54,60 | g2h-load / NJS_HANDLE slot | 3->3 |
| src/pso/ui/window/WindowHeading.cpp | NJS | - | see residual errs | 14->14 |
| src/pso/unk/SampledMotionPoints.cpp | NJS | 59 | g2h-load / NJS_HANDLE slot | 2->2 |
| src/pso/unk/TLobby_801F3A24.cpp | FP | - | row-assert only (3rd shadow) | 4->1 |
| src/pso/unk/TLobby_80207DA8.cpp | FP | - | row-assert only (3rd shadow) | FLIPPED-CLEAN |
| src/pso/unk/TLobby_80209524.cpp | FP | - | row-assert only (3rd shadow) | 6->7 |
| src/pso/unk/TLobby_803382E4.cpp | NJS | - | see residual errs | 4->4 |
| src/pso/unk/text_8028A998.cpp | NJS | 8,8,9,20,20,23 | compare in slot space, g2h-load / NJS_HANDLE slot, h2g store | 6->6 |
| src/pso/unk/text_80327844.cpp | NJS | 16 | g2h-load / NJS_HANDLE slot | 3->3 |
| src/pso/unk/text_803279CC.cpp | NJS | 15 | g2h-load / NJS_HANDLE slot | 3->3 |
| src/pso/unk/text_803DD5E0.cpp | NJS | 40 | h2g store | 15->15 |
| src/pso/weapon/SpecialProjectileWeapons.cpp | NJS | 120 | g2h-load / NJS_HANDLE slot | 3->3 |
| src/pso/weapon/TItemWeaponDrillLauncher.cpp | NJS | 125 | g2h-load / NJS_HANDLE slot | 3->3 |
| src/pso/weapon/TItemWeaponPanzerFaust.cpp | NJS | 61 | g2h-load / NJS_HANDLE slot | 1->1 |

## 4. Stage-2 allow-list — SUBMISSION ONLY (integrator applies; law: never lane-side)

Probe receipts: `H1C-fail -> H3-clean` = adding `-Icompat/njs` to that C++ TU
at the adopted shape makes it parse CLEAN with zero collateral (1001 TUs,
all .cpp — the C-TU ban holds trivially, 0 .c in the set). Named §3#4-#6 rows:
- `src/pso/system/BmlResource.cpp` — H3-CLEAN pre-adoption (no adapter needed:
  its §4 casts land on host-int `data` values, the u32 slots typecheck as-is).
- `src/pso/ui/charcreate/TCMCharacter.cpp` — H3-CLEAN, same finding.
- `src/pso/ui/charcreate/TChCrLoadReq.cpp` — 8 errors: exactly §3#6
  compare-in-slot-space sites :38/:38/:39/:39/:40/:40 + is_node_tree_valid
  legs :43/:46. NOT probe-safe until the window-test adapter lands — listed as
  loud-cluster row per AV §4 acceptance gate (delta == enumerated AR §3 cluster).
On-logo trio UNCHANGED by this wave's header work (psoLoadTexture 4, GinjaTextureTable
2, GinjaMaterial 6 before = after; residual legs are the V5-slice adapters, not headers):
psoLoadTexture :89 g2h-load, :145/:149 h2g-store (+1 packet_classes residual);
GinjaTextureTable :20/:24 h2g-store; GinjaMaterial :367/:388/:421 NJS_HANDLE slots
(+3 GXVert asm-dialect residuals, class §4.5 not NJS).

Paste-ready `build/njs_allowlist.txt` addition (C++ TUs only; suffix-enforced):

```
# stage 2 (P2-NC docs/p2-njs-wave-1.md §4; probe receipts in §3 table):
# loud-cluster rows (V6 adapters pending; AV §4 gate = delta equals cluster):
src/pso/ui/charcreate/TChCrLoadReq.cpp
src/pso/render/psoLoadTexture.cpp
src/pso/render/GinjaTextureTable.cpp
src/pso/render/GinjaMaterial.cpp
# probe-safe rows (H1C-fail -> H3-clean, zero dialect errors):
src/pso/animation/MotionSampling.cpp
src/pso/boss/TBoss3VoloptCageShot.cpp
src/pso/boss/TBoss3VoloptFadeIn.cpp
src/pso/boss/TBoss3VoloptLine.cpp
src/pso/boss/TBoss3VoloptMissile.cpp
src/pso/boss/TBoss3VoloptParticleSpray.cpp
src/pso/boss/TBoss3VoloptSweepLine.cpp
src/pso/boss/barbaray/Boss7CommonResource.cpp
src/pso/boss/barbaray/TBoss7Crawfish.cpp
src/pso/boss/barbaray/TBoss7Joint.cpp
src/pso/boss/barbaray/TBoss7LaserLight.cpp
src/pso/boss/barbaray/TBoss7LaserPtcl.cpp
src/pso/boss/barbaray/TBoss7PhotonBullet.cpp
src/pso/boss/barbaray/TObjBoss7Dust.cpp
src/pso/boss/barbaray/TObjBoss7RaftSplash.cpp
src/pso/boss/barbaray/TObjBoss7Tunnel_802F190C.cpp
src/pso/boss/darkfalz/DrainPlayerHp.cpp
src/pso/boss/darkfalz/ModelNodeTables.cpp
src/pso/boss/darkfalz/TBoss4CheckEmpty.cpp
src/pso/boss/darkfalz/TBoss4DarkFalz.cpp
src/pso/boss/darkfalz/TBoss4DarkFalz_8004D8C8.cpp
src/pso/boss/darkfalz/TBoss4Fade.cpp
src/pso/boss/darkfalz/TBoss4Thanks.cpp
src/pso/boss/darkfalz/TBoss4Type1.cpp
src/pso/boss/darkfalz/TBoss4Type2_800522CC.cpp
src/pso/boss/darkfalz/TBoss4Type3.cpp
src/pso/boss/darkfalz/TBoss4Type3_800524E8.cpp
src/pso/boss/darkfalz/TBoss4Zako.cpp
src/pso/boss/darkfalz/TBoss4Zanzo.cpp
src/pso/boss/darkfalz/TOBoss4.cpp
src/pso/boss/darkfalz/TOBoss4Rock.cpp
src/pso/boss/darkfalz/TObjDFDemo0.cpp
src/pso/boss/darkfalz/TObjDFDemo2.cpp
src/pso/boss/darkfalz/TObjDFDemo3.cpp
src/pso/boss/darkfalz/TObjDFDemo4.cpp
src/pso/boss/darkfalz/TObjDFDemo5.cpp
src/pso/boss/darkfalz/TObjDFDemoF.cpp
src/pso/boss/darkfalz/TObjDFHorming.cpp
src/pso/boss/derolle/Boss2CommonResource.cpp
src/pso/boss/derolle/TBoss2Fin.cpp
src/pso/boss/derolle/TBoss2Joint.cpp
src/pso/boss/derolle/TBoss2LaserLight.cpp
src/pso/boss/derolle/TBoss2PhotonBullet.cpp
src/pso/boss/derolle/TObjBoss2CeilJunk.cpp
src/pso/boss/derolle/TObjBoss2DustCtrl.cpp
src/pso/boss/derolle/TObjBoss2RaftSplash.cpp
src/pso/boss/derolle/TObjBoss2Tunnel_8003B1FC.cpp
src/pso/boss/dragon/TBoss1DragonColli.cpp
src/pso/boss/dragon/TBoss1DragonDemo.cpp
src/pso/boss/dragon/TBoss1DragonEffBreath.cpp
src/pso/boss/dragon/TBoss1DragonEffBullet.cpp
src/pso/boss/dragon/TBoss1DragonEffTail.cpp
src/pso/boss/dragon/TObjBoss1Crater.cpp
src/pso/boss/dragon/TObjBoss1Rock.cpp
src/pso/boss/galgryphon/Boss5GryphonBehavior.cpp
src/pso/boss/galgryphon/TBoss5GryphonColli.cpp
src/pso/boss/galgryphon/TBoss5GryphonDemo.cpp
src/pso/boss/galgryphon/TBoss5GryphonLightning.cpp
src/pso/boss/galgryphon/TBoss5GryphonLightningHorn.cpp
src/pso/boss/galgryphon/TBoss5GryphonLightningWing.cpp
src/pso/boss/galgryphon/TBoss5GryphonShockWave.cpp
src/pso/boss/galgryphon/TBoss5GryphonSnarl.cpp
src/pso/boss/galgryphon/TBoss5GryphonTornado.cpp
src/pso/boss/galgryphon/TObjBoss5FloorGrass.cpp
src/pso/boss/galgryphon/TObjBoss5Pillar.cpp
src/pso/boss/galgryphon/TObjBoss5PillarDemo.cpp
src/pso/boss/goldragon/AreaAttackHistory.cpp
src/pso/boss/goldragon/TBoss8DragonColli.cpp
src/pso/boss/goldragon/TBoss8DragonDemo.cpp
src/pso/boss/goldragon/TBoss8DragonEffBreath.cpp
src/pso/boss/goldragon/TBoss8DragonEffBullet.cpp
src/pso/boss/goldragon/TBoss8DragonEffTail.cpp
src/pso/boss/goldragon/TBoss8DragonLeaderPart.cpp
src/pso/boss/goldragon/TBoss8DragonPart.cpp
src/pso/boss/goldragon/TBoss8Dragon_8005D6F4.cpp
src/pso/boss/goldragon/TBoss8Dragon_8005D700.cpp
src/pso/boss/goldragon/TBoss8Dragon_80067034.cpp
src/pso/boss/goldragon/TObjBoss8DemoRoom.cpp
src/pso/boss/goldragon/TObjBoss8Piller.cpp
src/pso/boss/olgaflow/Boss6PlayerSnapshots.cpp
src/pso/boss/olgaflow/Boss6Type2Data.cpp
src/pso/boss/olgaflow/TBoss6CrushRock.cpp
src/pso/boss/olgaflow/TBoss6DPB.cpp
src/pso/boss/olgaflow/TBoss6EffFlowSoul.cpp
src/pso/boss/olgaflow/TBoss6EffHyoui.cpp
src/pso/boss/olgaflow/TBoss6EmptyWarp.cpp
src/pso/boss/olgaflow/TBoss6Mag.cpp
src/pso/boss/olgaflow/TBoss6MagActions.cpp
src/pso/boss/olgaflow/TBoss6PlotFalz_802B0228.cpp
src/pso/boss/olgaflow/TBoss6Rock.cpp
src/pso/boss/olgaflow/TBoss6RockChase.cpp
src/pso/boss/olgaflow/TBoss6SwordShot.cpp
src/pso/boss/olgaflow/TBoss6Thanks.cpp
src/pso/boss/olgaflow/TBoss6Tube.cpp
src/pso/boss/olgaflow/TBoss6Type2Actions.cpp
src/pso/boss/olgaflow/TBoss6Type2_80356E50.cpp
src/pso/boss/olgaflow/TBoss6Zanzo.cpp
src/pso/boss/volopt/TBoss3VoloptCore.cpp
src/pso/boss/volopt/TBoss3VoloptHiraisin.cpp
src/pso/boss/volopt/TBoss3Volopt_800D8ACC.cpp
src/pso/boss/volopt/TObjVoloptCage.cpp
src/pso/boss/volopt/TObjVoloptPillar.cpp
src/pso/boss/volopt/TObjVoloptShutter.cpp
src/pso/camera/CameraDistanceFade.cpp
src/pso/camera/CameraHelpers.cpp
src/pso/camera/CreationCamera.cpp
src/pso/camera/TCameraController.cpp
src/pso/camera/TCameraConversation.cpp
src/pso/camera/TCameraCtrl.cpp
src/pso/camera/TCameraDebugView.cpp
src/pso/camera/TCameraFollow.cpp
src/pso/camera/TCameraOrbit.cpp
src/pso/camera/TCameraScripted.cpp
src/pso/camera/TCameraVolOpt.cpp
src/pso/camera/TCameraVolOptFollow.cpp
src/pso/camera/TCameraWatchPlayer.cpp
src/pso/camera/TCineCtrl.cpp
src/pso/camera/TObjCamera.cpp
src/pso/effect/TAttackTrace.cpp
src/pso/effect/TBGLightningCtrl.cpp
src/pso/effect/TDuelEffectNumber.cpp
src/pso/effect/TDuelEffectString.cpp
src/pso/effect/TEF_LensFlare.cpp
src/pso/effect/TEffGuard.cpp
src/pso/effect/TEffHit.cpp
src/pso/effect/TEffStarLight2D_Base.cpp
src/pso/effect/TIndirectEffectDrawTask.cpp
src/pso/effect/enemy/TEneEffHamon.cpp
src/pso/effect/enemy/TEneParticleCtrl.cpp
src/pso/effect/enemy/TObjEneHoldEffect.cpp
src/pso/effect/item/TItemEffArmor.cpp
src/pso/effect/item/TItemEffBringer.cpp
src/pso/effect/item/TItemEffCane.cpp
src/pso/effect/item/TItemEffEquip.cpp
src/pso/effect/item/TItemEffFlame.cpp
src/pso/effect/item/TItemEffHeaven.cpp
src/pso/effect/item/TItemEffLandHit.cpp
src/pso/effect/item/TItemEffNeedle.cpp
src/pso/effect/item/TItemEffOrgaFlowShot.cpp
src/pso/effect/item/TItemEffShield.cpp
src/pso/effect/item/TItemEffectWeapon.cpp
src/pso/effect/item/TItemWpnCardEffect.cpp
src/pso/effect/particle/EffectRender.cpp
src/pso/effect/particle/LightGlare.cpp
src/pso/effect/particle/ParticleBallistic.cpp
src/pso/effect/particle/ParticleDefinition.cpp
src/pso/effect/particle/ParticleOrbit.cpp
src/pso/effect/particle/ParticleRadial.cpp
src/pso/effect/particle/ParticleResources.cpp
src/pso/effect/particle/TParticleCtrl.cpp
src/pso/effect/particle/TParticleCtrl_801D6B3C.cpp
src/pso/effect/particle/TPlyFollowParticleCtrl.cpp
src/pso/effect/photon/PhotonBlastSummon.cpp
src/pso/effect/photon/TPhotonConcentrate.cpp
src/pso/effect/photon/TPhotonFarlla.cpp
src/pso/effect/photon/TPhotonGolla.cpp
src/pso/effect/photon/TPhotonLaser.cpp
src/pso/effect/photon/TPhotonLine.cpp
src/pso/effect/photon/TPhotonMagicSquare.cpp
src/pso/effect/photon/TPhotonMylla.cpp
src/pso/effect/photon/TPhotonRainbow.cpp
src/pso/effect/photon/TPhotonYoulla.cpp
src/pso/effect/sprite/TQuadSprite.cpp
src/pso/effect/sprite/TQuadSpriteCol.cpp
src/pso/effect/sprite/TQuadSpriteNormal.cpp
src/pso/effect/sprite/TQuadSpriteScl.cpp
src/pso/effect/sprite/TQuadSpriteSclCol.cpp
src/pso/effect/sprite/TQuadSpriteSclColRot.cpp
src/pso/enemy/AreaSoundRemap.cpp
src/pso/enemy/EnemyFactoryTables.cpp
src/pso/enemy/TNeedleShoot.cpp
src/pso/enemy/TObjEneDarkGunnerShot.cpp
src/pso/enemy/balclaw/TObjEneBalClawBody.cpp
src/pso/enemy/balclaw/TObjEneBalClawClaw.cpp
src/pso/enemy/beast/TObjEneBeast.cpp
src/pso/enemy/beast/TObjEneBeastBase.cpp
src/pso/enemy/berura/TObjEneRe7BeruraPunch.cpp
src/pso/enemy/bringer/BringerEffects.cpp
src/pso/enemy/canadin/CanadinEffects.cpp
src/pso/enemy/canadin/TObjEneMe1Canadin.cpp
src/pso/enemy/darkgunner/TObjEneDarkGunCenter.cpp
src/pso/enemy/deldepth/DelDepthMotion.cpp
src/pso/enemy/deldepth/TObjEneDelDepth.cpp
src/pso/enemy/dellbiter/TObjEneDellBiterEffects.cpp
src/pso/enemy/dubchik/TDubchikPart.cpp
src/pso/enemy/dubchik/TDubchikShot.cpp
src/pso/enemy/dubchik/TObjEneDubchik.cpp
src/pso/enemy/epsilon/TObjEneEpsilonBody.cpp
src/pso/enemy/epsilon/TObjEneEpsilonGuard.cpp
src/pso/enemy/flower/FlowerRe2Data.cpp
src/pso/enemy/flower/TObjEneRe2Flower.cpp
src/pso/enemy/flower/TObjEneRe2FlowerStates.cpp
src/pso/enemy/flower/TObjEneRe2Flower_Bomb.cpp
src/pso/enemy/fly/TObjEneBm3Fly.cpp
src/pso/enemy/fly/TObjEneBm3FlyNest.cpp
src/pso/enemy/gee/TObjEneMe1Gee.cpp
src/pso/enemy/gee/TObjEneMe1Gee_NEEDLE.cpp
src/pso/enemy/gibbles/TObjEneGibbles.cpp
src/pso/enemy/gibon/TObjEneBm5GibonU.cpp
src/pso/enemy/gigue/TObjEneMe1GiGue.cpp
src/pso/enemy/gigue/TObjEneMe1GiGueBarrier.cpp
src/pso/enemy/gigue/TObjEneMe1GiGueHomingShot.cpp
src/pso/enemy/gigue/TObjEneMe1GiGueMissile.cpp
src/pso/enemy/gyaranzo/TObjEneGyaranzo.cpp
src/pso/enemy/gyaranzo/TObjGyaranzoMissile.cpp
src/pso/enemy/gyaranzo/TObjGyaranzoMissile_800D40D8.cpp
src/pso/enemy/lappy/TObjEneLappy.cpp
src/pso/enemy/mericarol/TObjEneBm9Mericarol_Bomb.cpp
src/pso/enemy/mericarol/TObjEneBm9Mericarol_EffCurse3.cpp
src/pso/enemy/merilllia/TObjEneMerillLia.cpp
src/pso/enemy/moja/TObjEneMoja.cpp
src/pso/enemy/morfos/MorfosData.cpp
src/pso/enemy/morfos/TObjEneMorfosArm.cpp
src/pso/enemy/morfos/TObjEneMorfosGroundShot.cpp
src/pso/enemy/nanodrago/TObjEneNanoDrago.cpp
src/pso/enemy/nanodrago/TObjNanoDragoSingleBeam.cpp
src/pso/enemy/nanodrago/TObjNanoDragoTargetedBeam.cpp
src/pso/enemy/panarms/TObjEnePanBARA.cpp
src/pso/enemy/panarms/TObjEnePanarms.cpp
src/pso/enemy/panarms/TObjEnePanarmsAtkColli.cpp
src/pso/enemy/recon/TObjEneRecobox.cpp
src/pso/enemy/recon/TObjEneRecon.cpp
src/pso/enemy/recon/TObjEneReconBomb.cpp
src/pso/enemy/shinowa/ShinowaData.cpp
src/pso/enemy/sinowzoa/SinowZoaData.cpp
src/pso/enemy/sinowzoa/TObjEnemyColorFade.cpp
src/pso/enemy/slime/TObjEneSlimeOne.cpp
src/pso/enemy/slime/TObjEneSlimeTailAttack.cpp
src/pso/enemy/sorcerer/TObjEneRe4Sorcerer.cpp
src/pso/enemy/stelth/StelthData.cpp
src/pso/enemy/wolf/TObjEneBm5Wolf.cpp
src/pso/item/TItemBuff.cpp
src/pso/item/TItemData.cpp
src/pso/item/TItemData_8010EDA8.cpp
src/pso/item/TItemEquipBase.cpp
src/pso/item/TItemFollowStarLight.cpp
src/pso/item/TItemIdCounters.cpp
src/pso/item/TItemLockCountCtrl_80119238.cpp
src/pso/item/TItemMenuCtrl.cpp
src/pso/item/TItemMoney.cpp
src/pso/item/TItemObject.cpp
src/pso/item/TItemReverseTechCtrl.cpp
src/pso/item/TItemTexLoad.cpp
src/pso/item/TItemTimer.cpp
src/pso/item/TItemTimerEvent.cpp
src/pso/item/TItemToolAddSlot.cpp
src/pso/item/TItemToolFluid.cpp
src/pso/item/TItemToolGrinder.cpp
src/pso/item/TItemToolKillCounter.cpp
src/pso/item/TItemToolMagCell.cpp
src/pso/item/TItemToolMate.cpp
src/pso/item/TItemToolMaterial.cpp
src/pso/item/TItemToolMoonAtomizer.cpp
src/pso/item/TItemToolQuestBgm.cpp
src/pso/item/TItemToolRandomItem.cpp
src/pso/item/TItemToolSolAtomizer.cpp
src/pso/item/TItemToolStarAtomizer.cpp
src/pso/item/TItemToolStatusCure.cpp
src/pso/item/TItemToolTechniqueDisk.cpp
src/pso/item/TItemToolTelepipe.cpp
src/pso/item/TItemToolTrapVision.cpp
src/pso/item/TObjItemLight.cpp
src/pso/item/TUnk_804F112C.cpp
src/pso/item/armor/TItemProArmor.cpp
src/pso/item/armor/TItemProArmorParticle.cpp
src/pso/item/armor/TItemProShield.cpp
src/pso/item/armor/TItemProUnit.cpp
src/pso/item/drop/TItemDrop.cpp
src/pso/item/item_pmt.cpp
src/pso/item/mag/TItemMag.cpp
src/pso/item/mag/TItemMagSub.cpp
src/pso/item/mag/TItemMag_8001A468.cpp
src/pso/item/shop/ShopWeaponStock.cpp
src/pso/item/shop/TShopGenerator.cpp
src/pso/map/CsnQuery.cpp
src/pso/map/FloorLifecycleTables.cpp
src/pso/map/TMapCsnCtrl.cpp
src/pso/map/TObjMapCsn.cpp
src/pso/map/area/AreaAncient01.cpp
src/pso/map/area/AreaAncient02.cpp
src/pso/map/area/AreaAncient03.cpp
src/pso/map/area/AreaBoss01.cpp
src/pso/map/area/AreaBoss02.cpp
src/pso/map/area/AreaBoss03.cpp
src/pso/map/area/AreaBoss04.cpp
src/pso/map/area/AreaBoss05.cpp
src/pso/map/area/AreaCave01.cpp
src/pso/map/area/AreaCave02.cpp
src/pso/map/area/AreaCave03.cpp
src/pso/map/area/AreaCity.cpp
src/pso/map/area/AreaForest01.cpp
src/pso/map/area/AreaForest02.cpp
src/pso/map/area/AreaLobby.cpp
src/pso/map/area/AreaMachine01.cpp
src/pso/map/area/AreaMachine02.cpp
src/pso/map/area/AreaVersus01.cpp
src/pso/map/area/AreaVersus02.cpp
src/pso/math/CurvePathPoint.cpp
src/pso/network/LobbyRuleSettings.cpp
src/pso/network/NetworkWaitIcon.cpp
src/pso/network/TradeItemList.cpp
src/pso/network/protocol/subcommand_validation_actions.cpp
src/pso/network/protocol/subcommand_validation_movement.cpp
src/pso/network/protocol/subcommand_validation_photon_blast.cpp
src/pso/network/protocol/subcommand_validation_player.cpp
src/pso/npc/NpcPartnerControl.cpp
src/pso/npc/TNpcPlayer_800D87A8.cpp
src/pso/npc/TNpcPlayer_802C3CD0.cpp
src/pso/npc/TNpcTalkTrigger.cpp
src/pso/npc/TObjNpcAHM01.cpp
src/pso/npc/TObjNpcARM00.cpp
src/pso/npc/TObjNpcARW00.cpp
src/pso/npc/TObjNpcBase.cpp
src/pso/npc/TObjNpcBringer.cpp
src/pso/npc/TObjNpcCloakroomBase.cpp
src/pso/npc/TObjNpcConnoisseur.cpp
src/pso/npc/TObjNpcEnemy.cpp
src/pso/npc/TObjNpcExpertBase.cpp
src/pso/npc/TObjNpcFemale.cpp
src/pso/npc/TObjNpcFemaleBase.cpp
src/pso/npc/TObjNpcFemaleChild.cpp
src/pso/npc/TObjNpcFemaleDwarf.cpp
src/pso/npc/TObjNpcFemaleFat.cpp
src/pso/npc/TObjNpcFemaleMacho.cpp
src/pso/npc/TObjNpcFemaleOld.cpp
src/pso/npc/TObjNpcFemaleTall.cpp
src/pso/npc/TObjNpcFomarl.cpp
src/pso/npc/TObjNpcGovernorBase.cpp
src/pso/npc/TObjNpcHFW00.cpp
src/pso/npc/TObjNpcHFW01.cpp
src/pso/npc/TObjNpcHHM00.cpp
src/pso/npc/TObjNpcHRM00.cpp
src/pso/npc/TObjNpcHRM01.cpp
src/pso/npc/TObjNpcHosa.cpp
src/pso/npc/TObjNpcHosa2.cpp
src/pso/npc/TObjNpcKenkyu.cpp
src/pso/npc/TObjNpcKenkyu2.cpp
src/pso/npc/TObjNpcKenkyuW.cpp
src/pso/npc/TObjNpcLappy.cpp
src/pso/npc/TObjNpcMale.cpp
src/pso/npc/TObjNpcMaleBase.cpp
src/pso/npc/TObjNpcMaleChild.cpp
src/pso/npc/TObjNpcMaleDwarf.cpp
src/pso/npc/TObjNpcMaleFat.cpp
src/pso/npc/TObjNpcMaleMacho.cpp
src/pso/npc/TObjNpcMaleOld.cpp
src/pso/npc/TObjNpcMaleTall.cpp
src/pso/npc/TObjNpcMoja.cpp
src/pso/npc/TObjNpcNFM00.cpp
src/pso/npc/TObjNpcNFM01.cpp
src/pso/npc/TObjNpcNFW00.cpp
src/pso/npc/TObjNpcNFW01.cpp
src/pso/npc/TObjNpcNHW00.cpp
src/pso/npc/TObjNpcNHW01.cpp
src/pso/npc/TObjNpcNgcBase.cpp
src/pso/npc/TObjNpcNurseBase.cpp
src/pso/npc/TObjNpcSecretaryBase.cpp
src/pso/npc/TObjNpcSoldierBase.cpp
src/pso/npc/TObjNpcSoldierMacho.cpp
src/pso/npc/TObjNpcSoutokufu.cpp
src/pso/object/TActorLight.cpp
src/pso/object/TDeadSpot.cpp
src/pso/object/TMotorcycle.cpp
src/pso/object/TOBind.cpp
src/pso/object/TObjBigTuitate.cpp
src/pso/object/TObjChallenge4PRetry.cpp
src/pso/object/TObjChallengeRetry.cpp
src/pso/object/TObjChip.cpp
src/pso/object/TObjControl.cpp
src/pso/object/TObjHMS_EZRope_Player.cpp
src/pso/object/TObjHashi.cpp
src/pso/object/TObjLuker.cpp
src/pso/object/TObjLuker_801329B0.cpp
src/pso/object/TObjMesBase.cpp
src/pso/object/TObjParticle.cpp
src/pso/object/TObjRoomId.cpp
src/pso/object/TObjRoomId_800FD8C8.cpp
src/pso/object/TObjTank.cpp
src/pso/object/TObjWaitEntryHairMoveSim.cpp
src/pso/object/TObjWarnChime.cpp
src/pso/object/ambient/TButterfly.cpp
src/pso/object/ambient/TOBiwaMushi.cpp
src/pso/object/ambient/TOKurage.cpp
src/pso/object/ambient/TObjCGrass.cpp
src/pso/object/ambient/TObjEnvSoundGlobal.cpp
src/pso/object/ambient/TObjFish.cpp
src/pso/object/ambient/TObjLight.cpp
src/pso/object/ambient/TObjLight_80133568.cpp
src/pso/object/ambient/TObjMistCtrl.cpp
src/pso/object/ambient/TObjRain.cpp
src/pso/object/ambient/TObjRainCtrl.cpp
src/pso/object/base/ObjectFactoryTables.cpp
src/pso/object/base/TAttackCollider.cpp
src/pso/object/base/TObjBase.cpp
src/pso/object/base/TObjChara.cpp
src/pso/object/base/TObjMapObject.cpp
src/pso/object/cave/TOAirconCave02.cpp
src/pso/object/cave/TODragonflyCave01.cpp
src/pso/object/cave/TORainbowCave01.cpp
src/pso/object/cave/TORockCave.cpp
src/pso/object/city/CityDoors.cpp
src/pso/object/city/CitySeason.cpp
src/pso/object/city/CityVerticalModel.cpp
src/pso/object/collision/TOAttackableCol.cpp
src/pso/object/collision/TOSymbolchatColli.cpp
src/pso/object/collision/TObjBgmColCtrl.cpp
src/pso/object/collision/TObjCollision.cpp
src/pso/object/collision/TObjEvtCollision.cpp
src/pso/object/collision/TObjFogCollision.cpp
src/pso/object/collision/TObjInfoCol.cpp
src/pso/object/collision/TObjRaderCol.cpp
src/pso/object/collision/TObjTradeCollision.cpp
src/pso/object/container/TContainerAncient01.cpp
src/pso/object/container/TContainerAncient01R.cpp
src/pso/object/container/TObjContainerIdo.cpp
src/pso/object/container/TObjContainerIdo_8018BEE4.cpp
src/pso/object/container/TObjContainerIdo_801A0B94.cpp
src/pso/object/container/TObjContainerItem.cpp
src/pso/object/container/TObjContainerJung.cpp
src/pso/object/door/GreenDoorResource.cpp
src/pso/object/door/TODoorJungleMain.cpp
src/pso/object/door/TObjDoor_801F3754.cpp
src/pso/object/ending/TObjOnlineEnding.cpp
src/pso/object/ending/TObjOnlineEndingHexLine.cpp
src/pso/object/ending/TObjOnlineEndingMain.cpp
src/pso/object/fence/TOBarrierEnergy01.cpp
src/pso/object/fence/TOFenceAncient01.cpp
src/pso/object/fence/TOFenceAncient02.cpp
src/pso/object/fence/TOFenceAncient03.cpp
src/pso/object/fence/TOFenceAncient04.cpp
src/pso/object/jungle/TOGrassJungle.cpp
src/pso/object/jungle/TOJungleDesign.cpp
src/pso/object/jungle/TORockJungleL01.cpp
src/pso/object/key/TOKeyCol.cpp
src/pso/object/key/TOKeyGenericSw.cpp
src/pso/object/key/TOKeyJungleMain.cpp
src/pso/object/lobby/LobbyPigeonPath.cpp
src/pso/object/lobby/TObjButterflyLobby.cpp
src/pso/object/lobby/TObjCurtainLobby.cpp
src/pso/object/lobby/TObjKabochaLobby.cpp
src/pso/object/lobby/TObjParticleLobby.cpp
src/pso/object/lobby/TObjRainbowLobby.cpp
src/pso/object/lobby/TObjStendGlassLobby.cpp
src/pso/object/lobby/TObjSuisouLobby.cpp
src/pso/object/lobby/TObjTreeLobby.cpp
src/pso/object/lobby/TObjWeddingLobby.cpp
src/pso/object/mine/TODragonflyMachine01.cpp
src/pso/object/quest/TObjQuestCol.cpp
src/pso/object/ruins/TOMonumentAncient01.cpp
src/pso/object/seabed/TObjSeabedSuisoBrakable.cpp
src/pso/object/switch/TOChatSensor.cpp
src/pso/object/switch/TOSensor.cpp
src/pso/object/switch/TOSwitchAttack.cpp
src/pso/object/switch/TOSwitchAttackSub.cpp
src/pso/object/switch/TOSwitchItem.cpp
src/pso/object/switch/TOSwitchTimer.cpp
src/pso/object/trap/TOExplosiveMachine.cpp
src/pso/object/trap/TOMineIcon01.cpp
src/pso/object/trap/TOMineIcon02.cpp
src/pso/object/trap/TOMineIcon03.cpp
src/pso/object/trap/TOMineIcon04.cpp
src/pso/object/trap/TOTrap.cpp
src/pso/object/trap/TOTrapChainSawEdge.cpp
src/pso/object/warp/TObjAreaWarp_80307B70.cpp
src/pso/object/warp/TObjCityMainWarp.cpp
src/pso/object/warp/TObjCityMainWarpChallenge.cpp
src/pso/object/warp/TObjLaboMapWarp.cpp
src/pso/object/warp/TObjMapWarpAncient.cpp
src/pso/object/warp/TObjWarpBoss03.cpp
src/pso/object/warp/TObjWarpBoss03_801A2AEC.cpp
src/pso/object/warp/TObjWarpBossMulti.cpp
src/pso/object/warp/TObjWarpLobby.cpp
src/pso/object/wreck/TOWreckAncient.cpp
src/pso/object/wreck/TOWreckAncient02.cpp
src/pso/object/wreck/TOWreckAncient03.cpp
src/pso/player/ActionPaletteSlots.cpp
src/pso/player/DamageStatSlot.cpp
src/pso/player/GamePlayerActionProvider.cpp
src/pso/player/LobbyPlayerActionProvider.cpp
src/pso/player/Palettex.cpp
src/pso/player/PhotonBlastGauge.cpp
src/pso/player/PhotonBlastRequests.cpp
src/pso/player/PhotonBlastResults.cpp
src/pso/player/PhotonChairSeat.cpp
src/pso/player/PlayerActionProvider.cpp
src/pso/player/PlayerAttackController.cpp
src/pso/player/PlayerClassStats.cpp
src/pso/player/PlayerTechniqueState.cpp
src/pso/player/TObjEndingPlayer.cpp
src/pso/player/TObjPlayerActions.cpp
src/pso/player/TObjPlayerCreate.cpp
src/pso/player/TObjPlayerDemoMode.cpp
src/pso/player/TObjPlayerSet.cpp
src/pso/player/TObjPlayerTechniqueLevels.cpp
src/pso/player/TObjPlayer_801AF2F0.cpp
src/pso/player/TObjPlayer_801D6BC8.cpp
src/pso/player/TObjPlayer_801D6C4C.cpp
src/pso/player/TObjPlayer_801D7C5C.cpp
src/pso/player/TObjPlayer_801D84A8.cpp
src/pso/player/TObjPlayer_801D8ADC.cpp
src/pso/player/TObjPlayer_801D95B8.cpp
src/pso/player/TObjPlayer_802A1440.cpp
src/pso/player/TObjPlayer_802C367C.cpp
src/pso/player/TObjPlayer_802C3B14.cpp
src/pso/player/TObjPlayer_802C422C.cpp
src/pso/player/TPlayerFootStamp.cpp
src/pso/player/TPlyCharData.cpp
src/pso/player/TPlyDispConfigSmth.cpp
src/pso/player/TPlyItem.cpp
src/pso/player/TPlyPhotonChair.cpp
src/pso/player/WeaponSpecialEffects.cpp
src/pso/quest/QuestOpcodeHandlersData.cpp
src/pso/quest/QuestStoryFlags.cpp
src/pso/quest/TQuestListChallenge.cpp
src/pso/quest/TQuestListOffline.cpp
src/pso/quest/TQuestListVM.cpp
src/pso/quest/TQuestVmDownLoadServer_801F9010.cpp
src/pso/quest/challenge/ChallengeModeQuest.cpp
src/pso/quest/challenge/TChallenge4P.cpp
src/pso/quest/challenge/TChallenge4PItemSelect.cpp
src/pso/render/BlobShadow.cpp
src/pso/render/ChunkMaterialState.cpp
src/pso/render/GinjaPerspective.cpp
src/pso/render/GinjaTextureEntryCopy.cpp
src/pso/render/GinjaUvScroll.cpp
src/pso/render/GinjaView.cpp
src/pso/render/MaterialColorFade.cpp
src/pso/render/MaterialShadeFade.cpp
src/pso/render/MotionDrawState.cpp
src/pso/render/RenderFadeState.cpp
src/pso/render/RenderSbssGlobals.cpp
src/pso/render/TDLGraph.cpp
src/pso/render/TFogCtrl.cpp
src/pso/system/BmlResource.cpp
src/pso/system/ControllerInput.cpp
src/pso/system/GbaDownloadRuntime.cpp
src/pso/system/PadRumble.cpp
src/pso/system/RlcArchive.cpp
src/pso/system/SwitcherLaunch.cpp
src/pso/system/TGStream.cpp
src/pso/system/TLoadCtrl.cpp
src/pso/system/TScreenFadeController.cpp
src/pso/system/data/pso_date.cpp
src/pso/system/movie/TMovie.cpp
src/pso/system/save/TCheckMemoryCard.cpp
src/pso/system/save/TSaveChCr.cpp
src/pso/system/scene/TEnding.cpp
src/pso/system/scene/TEndingPic.cpp
src/pso/system/scene/TEndingPicStage.cpp
src/pso/system/scene/TScene.cpp
src/pso/system/scene/TSceneSora.cpp
src/pso/system/task/TObject.cpp
src/pso/technique/TTecAnti.cpp
src/pso/technique/TTecBarta.cpp
src/pso/technique/TTecCircle.cpp
src/pso/technique/TTecDeband.cpp
src/pso/technique/TTecFireCharge.cpp
src/pso/technique/TTecFoie.cpp
src/pso/technique/TTecGiBarta.cpp
src/pso/technique/TTecGiFoie.cpp
src/pso/technique/TTecGiZonde.cpp
src/pso/technique/TTecIceCharge.cpp
src/pso/technique/TTecJelin.cpp
src/pso/technique/TTecKafun.cpp
src/pso/technique/TTecLightCharge.cpp
src/pso/technique/TTecLuker.cpp
src/pso/technique/TTecMegid.cpp
src/pso/technique/TTecMegidCharge.cpp
src/pso/technique/TTecPlayerInfluence.cpp
src/pso/technique/TTecRaBarta.cpp
src/pso/technique/TTecRaFoie.cpp
src/pso/technique/TTecRaZonde.cpp
src/pso/technique/TTecResta.cpp
src/pso/technique/TTecReverser.cpp
src/pso/technique/TTecShot.cpp
src/pso/technique/TTecSifta.cpp
src/pso/technique/TTecSlicer.cpp
src/pso/technique/TTecSupportCharge.cpp
src/pso/technique/TTecThunderCharge.cpp
src/pso/technique/TTecZarua.cpp
src/pso/technique/TTecZonde.cpp
src/pso/technique/TechniqueRingwaveNames.cpp
src/pso/ui/TCharSaveSequence.cpp
src/pso/ui/TLevelSelect.cpp
src/pso/ui/TLogo.cpp
src/pso/ui/TTitleMenu.cpp
src/pso/ui/charcreate/CharacterPreview.cpp
src/pso/ui/charcreate/TCMCharacter.cpp
src/pso/ui/charcreate/TChCrEffCtrlPl.cpp
src/pso/ui/charcreate/TChCrEffTag.cpp
src/pso/ui/charcreate/TChCrEffTagStr.cpp
src/pso/ui/charcreate/TChCrEffWall.cpp
src/pso/ui/charcreate/TChCrLoadAfs.cpp
src/pso/ui/charcreate/TChCrTagInformation.cpp
src/pso/ui/charcreate/TChCrTagInformationStr.cpp
src/pso/ui/charcreate/TChCrTagName.cpp
src/pso/ui/charcreate/TChCrTagOccHeader.cpp
src/pso/ui/charcreate/TChCrTagOccupation.cpp
src/pso/ui/charcreate/TChCrTagPlBack.cpp
src/pso/ui/charcreate/TChCrTagPlGraphic.cpp
src/pso/ui/charcreate/TObjCMHexTile.cpp
src/pso/ui/charcreate/TObjCMIcon.cpp
src/pso/ui/charcreate/TObjCMMainMenu_8028EA88.cpp
src/pso/ui/charcreate/TObjCMName.cpp
src/pso/ui/chat/TSymbolChatCapture.cpp
src/pso/ui/chat/TSymbolChatMenu.cpp
src/pso/ui/chat/TWindowChat.cpp
src/pso/ui/chat/TWindowChatSymbol.cpp
src/pso/ui/chat/TWindowSubShortMail.cpp
src/pso/ui/chat/TWindowSubSymbolChat.cpp
src/pso/ui/chat/TWindowSubSymbolChatFaceType.cpp
src/pso/ui/chat/TWindowSubSymbolChatHelp.cpp
src/pso/ui/chat/TWindowSubSymbolChatParts.cpp
src/pso/ui/chat/TWindowSubSymbolChatPartsSet.cpp
src/pso/ui/chat/TWindowSubSymbolChat_802ACF5C.cpp
src/pso/ui/city/TCheckRoomSequence.cpp
src/pso/ui/city/THunterGuildSeqOffline_80211FEC.cpp
src/pso/ui/city/THunterGuildSeqOnlineTask_802156A0.cpp
src/pso/ui/city/TJudgmentSequence.cpp
src/pso/ui/city/TMedicalCenterSequence.cpp
src/pso/ui/city/TShopArmorListWindow.cpp
src/pso/ui/city/TShopDeleteItemWindow.cpp
src/pso/ui/city/TShopEquipmentGearSequence.cpp
src/pso/ui/city/TShopEquipmentListWindow.cpp
src/pso/ui/city/TShopEquipmentSellWindow.cpp
src/pso/ui/city/TShopEquipmentSellWindow_8026CE78.cpp
src/pso/ui/city/TShopEquipmentSequence.cpp
src/pso/ui/city/TShopGetItemWindow.cpp
src/pso/ui/city/TShopItemListWindow.cpp
src/pso/ui/city/TShopItemTecSequence.cpp
src/pso/ui/city/TShopJudgmentItemWindow.cpp
src/pso/ui/city/TShopKeepItemWindow.cpp
src/pso/ui/city/TShopSequence.cpp
src/pso/ui/city/TShopWindowBase.cpp
src/pso/ui/font/TFontSmallHelp.cpp
src/pso/ui/font/TFontSmallTask.cpp
src/pso/ui/frontend/DrawLine.cpp
src/pso/ui/frontend/EndingSequence.cpp
src/pso/ui/frontend/TAdADXlogo.cpp
src/pso/ui/frontend/TAdAgreementBaseGC.cpp
src/pso/ui/frontend/TAdChrMoveEffectGC.cpp
src/pso/ui/frontend/TAdChrSelASlotGC.cpp
src/pso/ui/frontend/TAdChrSelBSlotGC.cpp
src/pso/ui/frontend/TAdChrSelPlayerGC.cpp
src/pso/ui/frontend/TAdChrSelPlyPlateGC.cpp
src/pso/ui/frontend/TAdDialingGC_801A4C90.cpp
src/pso/ui/frontend/TAdDialingMessageWindow.cpp
src/pso/ui/frontend/TAdEnterBaseGC.cpp
src/pso/ui/frontend/TAdEnterPPPPassGC.cpp
src/pso/ui/frontend/TAdEnterPasswordGC.cpp
src/pso/ui/frontend/TAdEnterSerialAccessGC.cpp
src/pso/ui/frontend/TAdEnterSerialAccessGC_802ACF04.cpp
src/pso/ui/frontend/TAdEp3Exist.cpp
src/pso/ui/frontend/TAdFadeGC.cpp
src/pso/ui/frontend/TAdFloorLoadGC.cpp
src/pso/ui/frontend/TAdFloorLoadPrtGC.cpp
src/pso/ui/frontend/TAdGameStartBackGC.cpp
src/pso/ui/frontend/TAdGameStartLineGC.cpp
src/pso/ui/frontend/TAdGameStartShadowGC.cpp
src/pso/ui/frontend/TAdGameStartShineGC.cpp
src/pso/ui/frontend/TAdGameStartSubGC.cpp
src/pso/ui/frontend/TAdGuideTxtGC.cpp
src/pso/ui/frontend/TAdLdQuestPrtGC.cpp
src/pso/ui/frontend/TAdLicAgreementGC.cpp
src/pso/ui/frontend/TAdLoadNetworkGC.cpp
src/pso/ui/frontend/TAdLoadQuestCellGC.cpp
src/pso/ui/frontend/TAdLoadQuestGC.cpp
src/pso/ui/frontend/TAdLoadQuestLineGC.cpp
src/pso/ui/frontend/TAdLoadQuestPrtTxtGC.cpp
src/pso/ui/frontend/TAdMultiChgGameModeGC.cpp
src/pso/ui/frontend/TAdMultiChgGameModeGC_8023E7A8.cpp
src/pso/ui/frontend/TAdMultiConfirmGC.cpp
src/pso/ui/frontend/TAdMultiPlyChrSelectGC.cpp
src/pso/ui/frontend/TAdMultiPlyChrSelectGC_802C6F88.cpp
src/pso/ui/frontend/TAdMultiPlyChrSelectGC_802E0064.cpp
src/pso/ui/frontend/TAdMultiPlyChrSelectGC_802E0110.cpp
src/pso/ui/frontend/TAdMultiSaveGC.cpp
src/pso/ui/frontend/TAdMultiSelGameModeGC.cpp
src/pso/ui/frontend/TAdPlayerLoadGC.cpp
src/pso/ui/frontend/TAdPlyChrSelPlateGC.cpp
src/pso/ui/frontend/TAdPlyPltAnimCtrlGC.cpp
src/pso/ui/frontend/TAdPsoPassSaveGC.cpp
src/pso/ui/frontend/TAdRicoRingCtrl.cpp
src/pso/ui/frontend/TAdScrollWinGC.cpp
src/pso/ui/frontend/TAdSelectGameModeGC_802C4748.cpp
src/pso/ui/frontend/TAdSelectTxtSubGC.cpp
src/pso/ui/frontend/TAdSinglePlyChrSelectGC.cpp
src/pso/ui/frontend/TAdSinglePlyChrSelectGC_801334D4.cpp
src/pso/ui/frontend/TAdSinglePlyChrSelectGC_80243218.cpp
src/pso/ui/frontend/TAdSnAkPassConfirmGC.cpp
src/pso/ui/frontend/TAdSoniclogo.cpp
src/pso/ui/frontend/TAdStorySelectGC_802C3284.cpp
src/pso/ui/frontend/TAdSubPlyItemGC.cpp
src/pso/ui/frontend/TAdSystemFileCheckGC.cpp
src/pso/ui/frontend/TAdTimeSetGC.cpp
src/pso/ui/frontend/TAdTrialAgreementGC.cpp
src/pso/ui/frontend/TAdVIConfigGC.cpp
src/pso/ui/frontend/TAdVIConfigGC_80308200.cpp
src/pso/ui/frontend/TAdYesNoBaseGC.cpp
src/pso/ui/frontend/TTextureWave.cpp
src/pso/ui/frontend/TTitleBackground.cpp
src/pso/ui/frontend/TitleBackground.cpp
src/pso/ui/hud/TMainCursor.cpp
src/pso/ui/hud/TMainCursor_80154090.cpp
src/pso/ui/hud/TNowLoading.cpp
src/pso/ui/hud/TRaderMapCore.cpp
src/pso/ui/hud/TRaderMapCore_801F3710.cpp
src/pso/ui/hud/TRaderMapCore_8030A808.cpp
src/pso/ui/keyboard/KeyboardControlTask.cpp
src/pso/ui/keyboard/KeyboardKeyState.cpp
src/pso/ui/keyboard/TKeyBoardControlLayout.cpp
src/pso/ui/keyboard/TKeyBoardControl_80290CB0.cpp
src/pso/ui/keyboard/WindowKeyBoardMode.cpp
src/pso/ui/window/HeadingEntry.cpp
src/pso/ui/window/HelpItemRarityStarRow.cpp
src/pso/ui/window/HelpItemStatCompareRow.cpp
src/pso/ui/window/ItemListRow.cpp
src/pso/ui/window/LabelValueRow.cpp
src/pso/ui/window/TCustomizeMenuCtrl.cpp
src/pso/ui/window/TGameListRow.cpp
src/pso/ui/window/TObjBagWarnWin.cpp
src/pso/ui/window/TObjDeadListWin.cpp
src/pso/ui/window/TObjNewFloorWin.cpp
src/pso/ui/window/TObjResultWin.cpp
src/pso/ui/window/TObjSoloPlayWin.cpp
src/pso/ui/window/TWindowActionPalette.cpp
src/pso/ui/window/TWindowAddition.cpp
src/pso/ui/window/TWindowArea.cpp
src/pso/ui/window/TWindowBase.cpp
src/pso/ui/window/TWindowBreakIn.cpp
src/pso/ui/window/TWindowChallengeResult.cpp
src/pso/ui/window/TWindowCheckBox.cpp
src/pso/ui/window/TWindowChoiceSetting.cpp
src/pso/ui/window/TWindowChoiceSetting_80249078.cpp
src/pso/ui/window/TWindowChoiceSetting_80249610.cpp
src/pso/ui/window/TWindowComCard.cpp
src/pso/ui/window/TWindowComCardFollower.cpp
src/pso/ui/window/TWindowCtrlMain.cpp
src/pso/ui/window/TWindowCtrl_80271DFC.cpp
src/pso/ui/window/TWindowDD.cpp
src/pso/ui/window/TWindowDD_80246F8C.cpp
src/pso/ui/window/TWindowDD_8034AF68.cpp
src/pso/ui/window/TWindowDD_WS_ScrollBar.cpp
src/pso/ui/window/TWindowDD_WS_String.cpp
src/pso/ui/window/TWindowDeposit.cpp
src/pso/ui/window/TWindowDuelResult.cpp
src/pso/ui/window/TWindowDuelResult_801C2528.cpp
src/pso/ui/window/TWindowEffectLight.cpp
src/pso/ui/window/TWindowEffectParty.cpp
src/pso/ui/window/TWindowFukidasi.cpp
src/pso/ui/window/TWindowGetComCard.cpp
src/pso/ui/window/TWindowHelpItem.cpp
src/pso/ui/window/TWindowInputNumber.cpp
src/pso/ui/window/TWindowKeyBoardBase.cpp
src/pso/ui/window/TWindowKeyBoardBase_8008102C.cpp
src/pso/ui/window/TWindowKeyBoardBase_8025B85C.cpp
src/pso/ui/window/TWindowKeyBoardHard.cpp
src/pso/ui/window/TWindowKeyBoardSoft.cpp
src/pso/ui/window/TWindowKeyBoardSoft_80228BEC.cpp
src/pso/ui/window/TWindowLayoutCountingNumber.cpp
src/pso/ui/window/TWindowLayoutNumber.cpp
src/pso/ui/window/TWindowLoadItemTexture.cpp
src/pso/ui/window/TWindowLockOn.cpp
src/pso/ui/window/TWindowLockOn_800F5DD8.cpp
src/pso/ui/window/TWindowLockOn_8026CB80.cpp
src/pso/ui/window/TWindowMainMenu1P.cpp
src/pso/ui/window/TWindowMainMenu4P.cpp
src/pso/ui/window/TWindowMakeTeam.cpp
src/pso/ui/window/TWindowMenu.cpp
src/pso/ui/window/TWindowMenuIconLine.cpp
src/pso/ui/window/TWindowMenuIconLineLists.cpp
src/pso/ui/window/TWindowMenuMainMenu.cpp
src/pso/ui/window/TWindowMeseta.cpp
src/pso/ui/window/TWindowMessage.cpp
src/pso/ui/window/TWindowMessageLog.cpp
src/pso/ui/window/TWindowMessageQuest.cpp
src/pso/ui/window/TWindowMessageScroll.cpp
src/pso/ui/window/TWindowMessageYesNo.cpp
src/pso/ui/window/TWindowOptionCtrl.cpp
src/pso/ui/window/TWindowOptionCtrl_8026CBA4.cpp
src/pso/ui/window/TWindowPartyStatus.cpp
src/pso/ui/window/TWindowQuantity.cpp
src/pso/ui/window/TWindowReturnCity.cpp
src/pso/ui/window/TWindowRoom.cpp
src/pso/ui/window/TWindowSetLevelUp.cpp
src/pso/ui/window/TWindowShop.cpp
src/pso/ui/window/TWindowShortCut.cpp
src/pso/ui/window/TWindowShortListCtrl.cpp
src/pso/ui/window/TWindowSimpleStatus.cpp
src/pso/ui/window/TWindowSnapShot.cpp
src/pso/ui/window/TWindowSoundBalance.cpp
src/pso/ui/window/TWindowStatus.cpp
src/pso/ui/window/TWindowStatusGauge.cpp
src/pso/ui/window/TWindowStatusGaugeFeedback.cpp
src/pso/ui/window/TWindowSubAreaMap.cpp
src/pso/ui/window/TWindowSubAreaMapDisp.cpp
src/pso/ui/window/TWindowSubComCardList.cpp
src/pso/ui/window/TWindowSubComCard_8007341C.cpp
src/pso/ui/window/TWindowSubComCard_8007BD24.cpp
src/pso/ui/window/TWindowSubComCard_80225E0C.cpp
src/pso/ui/window/TWindowSubComCard_802495FC.cpp
src/pso/ui/window/TWindowSubCustomEquip.cpp
src/pso/ui/window/TWindowSubCustomSelect.cpp
src/pso/ui/window/TWindowSubEquip.cpp
src/pso/ui/window/TWindowSubEquipKind.cpp
src/pso/ui/window/TWindowSubEquipKind_80278300.cpp
src/pso/ui/window/TWindowSubEquip_80279044.cpp
src/pso/ui/window/TWindowSubGameEnd.cpp
src/pso/ui/window/TWindowSubItem.cpp
src/pso/ui/window/TWindowSubItemList.cpp
src/pso/ui/window/TWindowSubLog.cpp
src/pso/ui/window/TWindowSubMag.cpp
src/pso/ui/window/TWindowSubMagPhoton.cpp
src/pso/ui/window/TWindowSubQuestBoard.cpp
src/pso/ui/window/TWindowSubQuestBoardSelect.cpp
src/pso/ui/window/TWindowSubShortCut.cpp
src/pso/ui/window/TWindowSubShortCutList.cpp
src/pso/ui/window/TWindowSubShortCut_80246FB0.cpp
src/pso/ui/window/TWindowSubStatusDisp_800FBD74.cpp
src/pso/ui/window/TWindowSubStatusDisp_80228C30.cpp
src/pso/ui/window/TWindowSubStatusDisp_80228CC0.cpp
src/pso/ui/window/TWindowSubStatusDisp_8024FBA8.cpp
src/pso/ui/window/TWindowSubStatusDisp_8027FD88.cpp
src/pso/ui/window/TWindowSubStatusDisp_8036D774.cpp
src/pso/ui/window/TWindowSubTrade.cpp
src/pso/ui/window/TWindowSubTradeMain.cpp
src/pso/ui/window/TWindowSubTradeMain_80187C98.cpp
src/pso/ui/window/TWindowSubTradeMessage.cpp
src/pso/ui/window/TWindowTimer.cpp
src/pso/ui/window/TWindowTradeRequest.cpp
src/pso/ui/window/TWindowUserSort.cpp
src/pso/ui/window/TWindowWordSelectMode.cpp
src/pso/ui/window/WindowDrawContext.cpp
src/pso/ui/window/WindowLayoutCamera2Data.cpp
src/pso/ui/window/WindowLayoutEntries.cpp
src/pso/ui/window/WindowScrollBar.cpp
src/pso/ui/window/WindowSpritePart.cpp
src/pso/ui/window/WindowTextEntry.cpp
src/pso/unk/TBattleProximitySound.cpp
src/pso/unk/TLobbyEventTheme.cpp
src/pso/unk/TLobby_8002C140.cpp
src/pso/unk/TLobby_8004A90C.cpp
src/pso/unk/TLobby_800F795C.cpp
src/pso/unk/TLobby_800FD78C.cpp
src/pso/unk/TLobby_80103F5C.cpp
src/pso/unk/TLobby_80140FF4.cpp
src/pso/unk/TLobby_801D6BF8.cpp
src/pso/unk/TLobby_80207644.cpp
src/pso/unk/TLobby_8020E400.cpp
src/pso/unk/TLobby_80228154.cpp
src/pso/unk/TLobby_80228C84.cpp
src/pso/unk/TLobby_8028EA28.cpp
src/pso/unk/TLobby_80299A60.cpp
src/pso/unk/TLobby_80299B90.cpp
src/pso/unk/TLobby_802D79EC.cpp
src/pso/unk/TLobby_80300098.cpp
src/pso/unk/TLobby_8030A6A8.cpp
src/pso/unk/TLobby_8030A834.cpp
src/pso/unk/TObjFreezeShell.cpp
src/pso/unk/TWindowStatusCategoryMenu.cpp
src/pso/unk/text_80071B30.cpp
src/pso/unk/text_80076BC4.cpp
src/pso/unk/text_800C7FEC.cpp
src/pso/unk/text_80103674.cpp
src/pso/unk/text_8018E258.cpp
src/pso/unk/text_801A2710.cpp
src/pso/unk/text_801A2B38.cpp
src/pso/unk/text_801D9870.cpp
src/pso/unk/text_802233F8.cpp
src/pso/unk/text_8023DCB8.cpp
src/pso/unk/text_8025CD10.cpp
src/pso/unk/text_8026934C.cpp
src/pso/unk/text_8026A14C.cpp
src/pso/unk/text_8026C664.cpp
src/pso/unk/text_80278460.cpp
src/pso/unk/text_802A1020.cpp
src/pso/unk/text_802C3AC0.cpp
src/pso/unk/text_802F0F04.cpp
src/pso/unk/text_8035B6B0.cpp
src/pso/unk/text_8035B750.cpp
src/pso/unk/text_803D64F8.cpp
src/pso/unk/text_803D8DE0.cpp
src/pso/unk/text_803D8F74.cpp
src/pso/weapon/TItemAttackCol.cpp
src/pso/weapon/TItemAttacker.cpp
src/pso/weapon/TItemFollowEffect.cpp
src/pso/weapon/TItemWeapon.cpp
src/pso/weapon/TItemWeaponAgito.cpp
src/pso/weapon/TItemWeaponAngelHarp.cpp
src/pso/weapon/TItemWeaponBaranzLauncher.cpp
src/pso/weapon/TItemWeaponBranchOfPakupaku.cpp
src/pso/weapon/TItemWeaponBroom.cpp
src/pso/weapon/TItemWeaponCaduceus.cpp
src/pso/weapon/TItemWeaponCane.cpp
src/pso/weapon/TItemWeaponCard.cpp
src/pso/weapon/TItemWeaponChameleonScythe.cpp
src/pso/weapon/TItemWeaponClaw.cpp
src/pso/weapon/TItemWeaponDagger.cpp
src/pso/weapon/TItemWeaponDarkBridge.cpp
src/pso/weapon/TItemWeaponDarkMeteor.cpp
src/pso/weapon/TItemWeaponDemolitionComet.cpp
src/pso/weapon/TItemWeaponDoubleCannon.cpp
src/pso/weapon/TItemWeaponDragonsClaw.cpp
src/pso/weapon/TItemWeaponEggBlaster.cpp
src/pso/weapon/TItemWeaponElysion.cpp
src/pso/weapon/TItemWeaponFist.cpp
src/pso/weapon/TItemWeaponFlameVisit.cpp
src/pso/weapon/TItemWeaponFlightCutter.cpp
src/pso/weapon/TItemWeaponFlightFan.cpp
src/pso/weapon/TItemWeaponFlowerBouquet.cpp
src/pso/weapon/TItemWeaponFlowerCane.cpp
src/pso/weapon/TItemWeaponFrozenShooter.cpp
src/pso/weapon/TItemWeaponGAssassinsSabers.cpp
src/pso/weapon/TItemWeaponGuldMilla.cpp
src/pso/weapon/TItemWeaponHandgun.cpp
src/pso/weapon/TItemWeaponHarisenBattleFan.cpp
src/pso/weapon/TItemWeaponHeartOfPoumn.cpp
src/pso/weapon/TItemWeaponHeavenPunisher.cpp
src/pso/weapon/TItemWeaponHildebearsCane.cpp
src/pso/weapon/TItemWeaponHildebluesCane.cpp
src/pso/weapon/TItemWeaponHolyRay.cpp
src/pso/weapon/TItemWeaponHugeBattleFan.cpp
src/pso/weapon/TItemWeaponImperialPick.cpp
src/pso/weapon/TItemWeaponInfernoBazooka.cpp
src/pso/weapon/TItemWeaponLavisBlade.cpp
src/pso/weapon/TItemWeaponLavisCannon.cpp
src/pso/weapon/TItemWeaponMadamsParasol.cpp
src/pso/weapon/TItemWeaponMadamsUmbrella.cpp
src/pso/weapon/TItemWeaponMagicalPiece.cpp
src/pso/weapon/TItemWeaponMarinasBag.cpp
src/pso/weapon/TItemWeaponMaserBeam.cpp
src/pso/weapon/TItemWeaponMechgun.cpp
src/pso/weapon/TItemWeaponMeteorCudgel.cpp
src/pso/weapon/TItemWeaponMonkeyKingBar.cpp
src/pso/weapon/TItemWeaponMorningGlory.cpp
src/pso/weapon/TItemWeaponMusashi.cpp
src/pso/weapon/TItemWeaponNeisClaw.cpp
src/pso/weapon/TItemWeaponNug2000Bazooka.cpp
src/pso/weapon/TItemWeaponPArmsBlade.cpp
src/pso/weapon/TItemWeaponPanthersClaw.cpp
src/pso/weapon/TItemWeaponPartisan.cpp
src/pso/weapon/TItemWeaponPartisanOfLightning.cpp
src/pso/weapon/TItemWeaponPhotonLauncher.cpp
src/pso/weapon/TItemWeaponPlantainFan.cpp
src/pso/weapon/TItemWeaponPlantainHugeFan.cpp
src/pso/weapon/TItemWeaponProphetsOfMotav.cpp
src/pso/weapon/TItemWeaponPsychoWand.cpp
src/pso/weapon/TItemWeaponRabbitWand.cpp
src/pso/weapon/TItemWeaponRainbowBaton.cpp
src/pso/weapon/TItemWeaponRappysFan.cpp
src/pso/weapon/TItemWeaponRifle.cpp
src/pso/weapon/TItemWeaponRocketPunch.cpp
src/pso/weapon/TItemWeaponRod.cpp
src/pso/weapon/TItemWeaponRubyBullet.cpp
src/pso/weapon/TItemWeaponSBeatsBlade.cpp
src/pso/weapon/TItemWeaponSRank.cpp
src/pso/weapon/TItemWeaponSRankBazooka.cpp
src/pso/weapon/TItemWeaponSRankBlade.cpp
src/pso/weapon/TItemWeaponSRankCard.cpp
src/pso/weapon/TItemWeaponSRankClaw.cpp
src/pso/weapon/TItemWeaponSRankGun.cpp
src/pso/weapon/TItemWeaponSRankHarisen.cpp
src/pso/weapon/TItemWeaponSRankJCutter.cpp
src/pso/weapon/TItemWeaponSRankKnuckle.cpp
src/pso/weapon/TItemWeaponSRankLauncher.cpp
src/pso/weapon/TItemWeaponSRankMechgun.cpp
src/pso/weapon/TItemWeaponSRankNeedle.cpp
src/pso/weapon/TItemWeaponSRankPsychogun.cpp
src/pso/weapon/TItemWeaponSRankPunch.cpp
src/pso/weapon/TItemWeaponSRankScythe.cpp
src/pso/weapon/TItemWeaponSRankShot.cpp
src/pso/weapon/TItemWeaponSRankSlicer.cpp
src/pso/weapon/TItemWeaponSRankWand.cpp
src/pso/weapon/TItemWeaponSRedsBlade.cpp
src/pso/weapon/TItemWeaponSaber.cpp
src/pso/weapon/TItemWeaponSambaMaracas.cpp
src/pso/weapon/TItemWeaponSealedJSword.cpp
src/pso/weapon/TItemWeaponShot.cpp
src/pso/weapon/TItemWeaponSlicer.cpp
src/pso/weapon/TItemWeaponSorcerersCane.cpp
src/pso/weapon/TItemWeaponSoulEater.cpp
src/pso/weapon/TItemWeaponSpecialGun.cpp
src/pso/weapon/TItemWeaponSpreadNeedle.cpp
src/pso/weapon/TItemWeaponStingTip.cpp
src/pso/weapon/TItemWeaponSword.cpp
src/pso/weapon/TItemWeaponTechnicalCrozier.cpp
src/pso/weapon/TItemWeaponTheSighOfAGod.cpp
src/pso/weapon/TItemWeaponTsumikiri.cpp
src/pso/weapon/TItemWeaponTwinBlaze.cpp
src/pso/weapon/TItemWeaponTwinChakram.cpp
src/pso/weapon/TItemWeaponTwinPsychogun.cpp
src/pso/weapon/TItemWeaponTwinkleStar.cpp
src/pso/weapon/TItemWeaponWand.cpp
src/pso/weapon/TItemWeaponWokOfAkikosShop.cpp
src/pso/weapon/TItemWeaponYamigarasu.cpp
src/pso/weapon/TItemWeaponYasminkov2000H.cpp
src/pso/weapon/TItemWeaponYasminkov3000R.cpp
src/pso/weapon/TItemWeaponYasminkov7000V.cpp
src/pso/weapon/TItemWeaponYasminkov9000M.cpp
src/pso/weapon/WeaponAttackActions.cpp
src/pso/weapon/WeaponSpecialSelection.cpp
```
(1001+4 rows. NOTE: stage-1 rows from AV §4 are already in this set where
measured clean — the block is deduplicated against the AV stage-1 six; the 4
loud-cluster rows sit under the AV §4 acceptance-gate proviso, integrator may
stage them separately. Grep-proved inert: no build/host/data file references
compat/njs additions from this wave.)

## 5. Gate proposal

1. Row-assert-zero gate: post-adoption battery must show 0 err files citing
   `FloorPopulation.h` negative-array asserts (measured true today, my sweeps).

2. Allow-list additions only from §4 block; each added TU re-probes at H3 shape
   (az_probe.sh H3) = 0 errors, or delta == its §3 table row exactly.
3. Collateral ledger unchanged: {GcrnaStream.cpp} + 0 new (measured 0 breakage
   on the intersection by this wave's shadow).
4. .c-TU ban: allowlist machinery keeps the `*.cpp` case-guard (AV §4);
   rescue set verified 100% .cpp.

## Repro

`cache/scratch/nc/`: nc_probe.sh (H1C/H3), b3 sweep -> nc/H3/{pass,fail}.txt,
nc_tables.py (classification -> rows.json, set_rescue.txt/set_fp/set_njs),
delta.json. Repo writes: compat/njs/pso/map/FloorPopulation.h + this doc only.

