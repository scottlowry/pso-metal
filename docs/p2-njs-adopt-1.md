# P2-NA — NJS tree-wide adoption EVIDENCE PACKAGE (the 1,001-row audit; NOT the decision)

Fleet wave-2 lane NA (zero-build), card t_55912438, 2026-11-27. Decomp
`pso-decomp @6b38b5383` READ-ONLY; zero repo builds; ZERO build/ or compat/ edits —
the paste-ready block below lives in THIS doc only (law: never lane-side into build/).
AR §6 stands: this is the evidence package for per-TU staged expansion, not the
tree-wide decision. Inputs: docs/p2-njs-wave-1.md §4–§5, docs/p2-njs-repin-1.md §6,
docs/p2-stitch-apply4.md §Meaning, docs/p2-stitch-apply6.md, p2-mwccasm-1.md, p2-packets-1.md.

## 0. Intake era + shape fidelity (D8 vigilance)

- Waiver header `build/waivers/pso_assert_waivers.h` sha256 = `495cc91a5b9694c4…` at
  intake AND re-hashed after the sweep (identical): the NEW apply-6 era, equal to the D8
  pin `host/bringup/waiver-header.sha256`. Every number here certifies the 495cc91a era.
- Official LP64 corpus at intake: `build/parse/summary.txt` TOTAL=1680 FAIL=1330 PASS=350
  (target=arm64-apple-macos12.0 — macos; battery not mid-run).
- Runner: `cache/scratch/na/na_probe.sh` — FRESH file (not sed-derived) reproducing the
  apply-6 official PARSE_RECIPE verbatim: `-include compat/pso_metal_compat.h,
  build/waivers/pso_assert_waivers.h, compat/collision/pin_dolphin_types.h` (that order),
  `-DPSO_PORT_LAYOUT_WAIVER -Icompat/net -Icompat/repin -Ihost/bringup/include`,
  SDK -I list, `-target arm64-apple-macos12.0 -ferror-limit=0`; musyx .c routed
  `clang++ -x c++ -Icompat/musyx`. Shape BASE = official; shape NJS = BASE + `-Icompat/njs`
  on **.cpp ONLY** (case-guard mirrors PARSE_RECIPE; AR §5 .c silent-truncation ban
  enforced mechanically — all three lists python-verified 100% .cpp, zero .c).
  Full corpus both shapes over `$TMP/b3/all.txt` (1,680), BSD xargs -P6.
- **Shape fidelity: 0 mismatches** between BASE verdicts and official `build/parse/fail.txt`
  across all 1,675 units outside the committed allow-list (the 5 live rows fail BASE by
  construction — they carry `-Icompat/njs` officially; calibrated musyx/NetApi legs match too).

## 1. Re-verification of NC's 1,001 (row set shifts confirmed)

```
FLIP     (BASE-fail -> NJS-clean)   = 1068   # python-counted
BREAKAGE (BASE-clean -> NJS-fail)   = 0   # tree-wide — zero collateral
BOTHFAIL (fail under both shapes)   = 252
NC probe-safe rows re-verified: 1001/1001 still FLIP under apply-6 (0 drops, 0 breaks)
NEW flips outside NC's list         = 67
```

Named shifts vs NC's snapshot (apply-6 landed net+repin+-I, musyx flip, PK flag, MA shadows):
- **MA dual-class 15** (apply-6 ledger: "NA-seat rows"): all 15 FLIP — the repin shadows
  repaired their AT class and compat/njs cures the `NJS_OBJECT_size` KEEP-assert leg.
  12 land in ADOPT-NOW; 3 in RUNTIME-GATED (TParticle :253, QuestDownloadAramFile :109,
  TLobby_800F8AA8 :838/:840 — grep receipts in §4).
- **PK residual 14** (`p2-packets-1.md` NJS-4.2 ledger): 13/14 stay NJS-fail →
  ADAPTER-QUEUE with enumerated sites (incl. loud §3#4 row psoLoadTexture.cpp, 3 sites —
  was 8; packet legs resolved). **1 reclassified**: src/pso/network/TLobby.cpp now fails
  only negative-array asserts (:147-149) + `__construct_array` runtime-init class
  (:2646-2655) at NJS shape — no NJS-dialect leg remains; it is other-class tail (§5), not queue.
- 6 PK packet-class rows (SoundRequestVoice, TObjEneDD, TObjEneIllGill, item_create,
  GuildCardRecord, TGameProtocol) are NEW FLIPs (PsoCommandHandler_size flag landed in apply-6).
- NC §2's 10 FloorPopulation FLIPPED-CLEAN rows (TObjBoss7Tunnel, TObjBoss5Ctrl,
  Boss8DemoObjects, TBoss3VoloptCtrl, TEnemyTaskList, TSetEvtDestroy, PlayerTaskEffects,
  TPlayerBattleCtrl4P, TWindowDD_803430BC, TLobby_80207DA8) all re-verified as NEW FLIPs.
- The remaining NEW flips are quest/window/ui/unk rows unblocked by the NP net-shadow and
  MA repin legs (full list in §6 block membership: FLIP = NC∩1001 ⊎ newadd 67).

## 2. Three lists (python counts, machine-validated this turn)

| list | n= | definition |
|---|---|---|
| **ADOPT-NOW** | 808 | FLIP ∧ runtime-screen clear: parses CLEAN at NJS shape (0-byte err receipt each), no source adapter, no grep-visible host-ptr↔slot laundering |
| **ADAPTER-QUEUE** | 169 | BOTHFAIL rows whose NJS-shape errors ARE the reinterpretation class — per-file sites enumerated §3; fix-shape per AV §3 taxonomy (g2h-load / h2g-store / NJS_HANDLE slot / slot-space compare / slot subscript) |
| **RUNTIME-GATED** | 260 | FLIP rows where grep proves host pointers are coerced into/out of slot space at RUNTIME (§4) — parse-clean is NOT runtime-correct; gate on the uaddr handle program (apply4 §Meaning) |
| other-class (NOT NJS) | 83 | BOTHFAIL rows with zero NJS-dialect errors at NJS shape (assert tail, MWCC-asm init, dup-waiver-name class) — outside this audit |

Adopt∩NC=773, adopt∩new=35; gated∩NC=228, gated∩new=32; queue rows are all outside NC's probe-safe block (NC §3 loud walkers + PK-13 + tail).
Set identity: 808 + 260 = 1068 FLIP; 169 + 83 = 252 BOTHFAIL; FLIP+BOTHFAIL = 1320 = all BASE-fail .cpp.

**Headline for the integrator ledger:** the two rows apply-6 put LIVE —
`src/pso/system/BmlResource.cpp` and `src/pso/ui/charcreate/TCMCharacter.cpp` — both land
in RUNTIME-GATED here. NC §4's own finding confirms it: "its §4 casts land on host-int
`data` values" — `(NJS_OBJECT*)data` tree-walk reinterprections (BmlResource :427-481,
TCMCharacter :441-449) flowing through u32 slots. Parse adoption holds (zero-collateral
gate passed); runtime correctness of those rows, like all gated rows, waits on the
loader-stamping program. Tag them gated in the ledger; no rollback claim is made.

## 3. ADAPTER-QUEUE n=169 — per-file enumerated sites (NJS-shape error lines)

Sites = decomp-side line numbers of NJS-reinterpretation errors at NJS shape
(`dialect/total` = NJS-class errors over all errors in the NJS err file; residue is
other-class and is NOT the queue's cost). Probes: `cache/scratch/na/NJS/err/*.err`.

| file | sites | dialect/total |
|---|---|---|
| src/pso/boss/barbaray/TBoss7BreakUp.cpp | 189,194,277,306 | 5/9 |
| src/pso/boss/barbaray/TBoss7DeRolLeC.cpp | 505,523,3640,3642,3672,3674,3697,3698 | 8/8 |
| src/pso/boss/barbaray/TBoss7TentacleNeedle.cpp | 58,60,63 | 3/4 |
| src/pso/boss/darkfalz/TBoss4ZakoDraw.cpp | 158,193,196 | 3/3 |
| src/pso/boss/darkfalz/TObjDFDemo1.cpp | 218,234 | 2/2 |
| src/pso/boss/darkfalz/TObjDFJisin.cpp | 348,351 | 2/2 |
| src/pso/boss/derolle/TBoss2BreakUp.cpp | 192,224,303,306 | 4/8 |
| src/pso/boss/derolle/TBoss2DeRolLe.cpp | 491,494,495,498,500,502,520,3759,3761 | 9/9 |
| src/pso/boss/derolle/TBoss2Mine.cpp | 180,185,190 | 3/3 |
| src/pso/boss/derolle/TBoss2Tentacle.cpp | 413,415,418 | 3/4 |
| src/pso/boss/derolle/TObjBoss2Tunnel.cpp | 332,336,356 | 3/10 |
| src/pso/boss/derolle/TObjBoss2Tunnel_8003B138.cpp | 125,127,564,565,644 | 5/5 |
| src/pso/boss/dragon/TBoss1Dragon.cpp | 382,384,407,409,410,413,414,572,574,594,595 | 11/11 |
| src/pso/boss/dragon/TObjBoss1MokoHill.cpp | 174,175,181,259,334 | 5/5 |
| src/pso/boss/galgryphon/TBoss5GryphonLightningEff.cpp | 52,54 | 2/2 |
| src/pso/boss/galgryphon/TObjBoss5FloorStone.cpp | 26 | 2/2 |
| src/pso/boss/galgryphon/TObjBoss5PillarCtrl.cpp | 46,55,57 | 4/4 |
| src/pso/boss/galgryphon/TObjBoss5PillarSmoke.cpp | 105,132,137 | 4/4 |
| src/pso/boss/goldragon/TBoss8Dragon.cpp | 318,320,321,324,325 | 5/5 |
| src/pso/boss/goldragon/TBoss8Dragon_8028A8F8.cpp | 15 | 2/2 |
| src/pso/boss/olgaflow/TBoss6Type2.cpp | 680 | 1/3 |
| src/pso/boss/volopt/TBoss3Volopt.cpp | 407,408 | 2/2 |
| src/pso/boss/volopt/TBoss3VoloptMonitor.cpp | 203,323 | 2/28 |
| src/pso/boss/volopt/TBoss3VoloptP01.cpp | 184 | 1/1 |
| src/pso/boss/volopt/TBoss3VoloptP02.cpp | 293,294,295 | 3/3 |
| src/pso/effect/TBreakUpMdl.cpp | 110,113,116,134,137,140 | 6/6 |
| src/pso/effect/TEF_PvrWave.cpp | 9,10,11,24,25,26,27,28,29 | 12/12 |
| src/pso/effect/TEffIndirectCtrl.cpp | 360,365 | 2/2 |
| src/pso/effect/TEffect.cpp | 76,77,78,82,83,84,98 | 13/13 |
| src/pso/effect/TKomorebi.cpp | 46,108,110,111,113,114 | 6/6 |
| src/pso/effect/enemy/TeneFakeTecThunderEff.cpp | 279 | 1/1 |
| src/pso/effect/item/TItemEffPG.cpp | 208 | 1/2 |
| src/pso/enemy/TObjEnemyCustom.cpp | 323,328,342 | 3/4 |
| src/pso/enemy/balclaw/TObjEneBalClaw.cpp | 169 | 1/1 |
| src/pso/enemy/berura/TObjEneRe7Berura.cpp | 222,482,485,515,517 | 5/6 |
| src/pso/enemy/bringer/TObjEneDf2Bringer.cpp | 1344,1345,1371 | 3/3 |
| src/pso/enemy/darkgunner/DarkGunner.cpp | 377,378 | 2/2 |
| src/pso/enemy/dellbiter/TObjEneDellBiter.cpp | 169 | 1/1 |
| src/pso/enemy/dimedian/TObjEneDimedian.cpp | 234,283,418,419,420 | 6/6 |
| src/pso/enemy/dolmolm/TObjEneDolmOlm.cpp | 373,411 | 3/3 |
| src/pso/enemy/dubchik/TOCtrlDubchik.cpp | 138,143,148,154 | 4/9 |
| src/pso/enemy/epsilon/TObjEneEpsilon.cpp | 75,76,77,78,79,80,81 | 7/7 |
| src/pso/enemy/flower/TObjEneRe2Flower_800C20D0.cpp | 57,58,59,60,74,75,108,109,110,111,161,162 | 16/16 |
| src/pso/enemy/flower/TObjEneRe2Flower_EffCurse3.cpp | 89 | 1/1 |
| src/pso/enemy/mericarol/TObjEneBm9Mericarol.cpp | 1200,1201,1202,1203,1204,1205,1208,1209,1210,1211,1212,1213,1227,1257,1265,1266,1267,1280,1281,1282,1283,1284,1285,1288,1289,1290,1291,1292,1293,1308,1309,1310 | 45/45 |
| src/pso/enemy/morfos/TObjEneMorfos.cpp | 152,153 | 2/4 |
| src/pso/enemy/saver/SaverLifecycle.cpp | 140,142,700,701 | 4/4 |
| src/pso/enemy/shark/TObjEneShark.cpp | 245,247,256,262,325,326,334 | 12/12 |
| src/pso/enemy/shinowa/TObjEneMe3Shinowa.cpp | 167,169,172,1592,1594,1638,1640 | 9/11 |
| src/pso/enemy/sinowzoa/TObjEneMe3SinowZoa.cpp | 170,172,175,197,203,209,1813,1861 | 10/12 |
| src/pso/enemy/sorcerer/TObjEneRe4Bit.cpp | 535,562 | 2/2 |
| src/pso/enemy/stelth/TObjEneMe3Stelth.cpp | 197,199,202,1753,1803 | 7/9 |
| src/pso/item/TItem.cpp | 868,893,900,1419,1518 | 5/8 |
| src/pso/item/TItemLoad.cpp | 487,552 | 2/2 |
| src/pso/item/TItemReverseTech.cpp | 162,168 | 2/2 |
| src/pso/map/TSetManager.cpp | 80,109,157,160,171,173,178,182,209,212,223,225,230,234,246,247,248,249,280,281,282,283,316,337,433,461,552,570,622,670,711,756,794,858,889,960,990,1165,1384,1390,1396,1402,1413,1423,1490,1492,1507,1509 | 48/50 |
| src/pso/object/TOHealGeneral.cpp | 191,197 | 2/5 |
| src/pso/object/TObjComputer.cpp | 164 | 1/2 |
| src/pso/object/TObjDoaEx01.cpp | 160,164,251,253 | 4/10 |
| src/pso/object/TObjEZBoneRotBase.cpp | 83,85,116,118 | 4/4 |
| src/pso/object/TObjLiveMonitor.cpp | 404,415,420 | 3/4 |
| src/pso/object/TObjSinBoard.cpp | 103,116 | 2/2 |
| src/pso/object/TObjTailsTailsMove.cpp | 79,81 | 2/2 |
| src/pso/object/TObjTombstone.cpp | 458,462,463 | 3/3 |
| src/pso/object/base/TObjMotion.cpp | 404,410 | 2/2 |
| src/pso/object/cave/TOCakeshopCave01.cpp | 72,82 | 2/4 |
| src/pso/object/cave/TOHangceilingCave01.cpp | 217,220,223,227,230,233 | 6/6 |
| src/pso/object/cave/TORevlightCave01.cpp | 44,46 | 2/3 |
| src/pso/object/cave/TOSignCave03.cpp | 34,39,43,51,56,60,68,73,80,85,89 | 11/23 |
| src/pso/object/city/CitySeasonResources.cpp | 238,242,263 | 5/5 |
| src/pso/object/city/CityTraffic.cpp | 392,400 | 2/2 |
| src/pso/object/container/TOContainerAncientEnemy.cpp | 270,275,429,434 | 4/10 |
| src/pso/object/door/TODoorAncient01.cpp | 105,111,116 | 3/7 |
| src/pso/object/door/TODoorAncient03.cpp | 105,111,116 | 3/7 |
| src/pso/object/door/TODoorAncient04.cpp | 105,111,116 | 3/7 |
| src/pso/object/door/TODoorAncient05.cpp | 115 | 1/2 |
| src/pso/object/door/TODoorAncient06.cpp | 116 | 1/2 |
| src/pso/object/door/TODoorAncient07.cpp | 121 | 1/2 |
| src/pso/object/door/TODoorAncient08.cpp | 208,209,210,211,212,213,214,215,241 | 9/12 |
| src/pso/object/door/TODoorAncient09.cpp | 206,207,208,209,225 | 5/8 |
| src/pso/object/door/TODoorCave01.cpp | 194,205,228,232 | 4/7 |
| src/pso/object/door/TODoorCave02.cpp | 95,100,117,118 | 4/7 |
| src/pso/object/door/TODoorCave03.cpp | 113,115,119 | 3/6 |
| src/pso/object/door/TODoorFourLightSpace.cpp | 151,353,354,356,357,358,359 | 7/12 |
| src/pso/object/door/TODoorLabo.cpp | 200,206 | 2/3 |
| src/pso/object/door/TODoorMachine01.cpp | 154,175,181,183,189,191,196,202 | 8/13 |
| src/pso/object/door/TODoorMachine02.cpp | 86,96,101,109 | 4/8 |
| src/pso/object/door/TODoorVS2Door01.cpp | 85,94,102,110,118 | 5/11 |
| src/pso/object/door/TODoorVoShip.cpp | 84,91,98 | 3/6 |
| src/pso/object/door/TObjDoorJung.cpp | 75,76,77,78,79,80,88 | 7/7 |
| src/pso/object/door/TObjDoorJung_8023DBEC.cpp | 12 | 1/2 |
| src/pso/object/door/TObjDoorKey.cpp | 117 | 1/1 |
| src/pso/object/fence/TLazerFenceSw.cpp | 95,245,247,249,251,253 | 6/6 |
| src/pso/object/fence/TOVS2Wall01.cpp | 282,391 | 2/2 |
| src/pso/object/fence/TObjLazerFence4.cpp | 91,94,95 | 3/3 |
| src/pso/object/fence/TObjLazerFenceNorm.cpp | 240,242,243,245,246 | 5/5 |
| src/pso/object/key/TODummyKeyCave01.cpp | 135,143 | 2/5 |
| src/pso/object/key/TOKeyAncient02.cpp | 50,58,70 | 3/8 |
| src/pso/object/key/TOKeyAncient03.cpp | 141,146 | 2/5 |
| src/pso/object/key/TOKeyCave01.cpp | 185,197 | 2/5 |
| src/pso/object/key/TOKeyMachine01.cpp | 181,186 | 2/5 |
| src/pso/object/lab/TOCapsuleLabo.cpp | 72,77 | 2/4 |
| src/pso/object/lab/TObjLaboDesignBase.cpp | 168,174 | 2/3 |
| src/pso/object/lobby/LobbyGoGoBall.cpp | 931,933,938,942 | 4/4 |
| src/pso/object/lobby/TObjLobbyMain.cpp | 118,120 | 2/2 |
| src/pso/object/mine/TOCapsuleMachine01.cpp | 71 | 1/1 |
| src/pso/object/mine/TOComputerMachine01.cpp | 58,60,62,64,90,91,92,93,94 | 9/13 |
| src/pso/object/mine/TOHangerMachine01.cpp | 43,48,51,54,64,67,77 | 7/15 |
| src/pso/object/mine/TOLightMachine01.cpp | 56 | 1/2 |
| src/pso/object/mine/TOMonitorMachine01.cpp | 84,90,97,105,111,118,150,151,152 | 9/15 |
| src/pso/object/mine/TOSparkMachine01.cpp | 77,88 | 2/3 |
| src/pso/object/ruins/TOCapsuleAncient01.cpp | 44 | 1/3 |
| src/pso/object/ruins/TOMonumentAncient02.cpp | 154 | 1/3 |
| src/pso/object/seabed/TObjSuisoChip.cpp | 369,409,414,521,533 | 5/5 |
| src/pso/object/switch/TOSensorAncient01.cpp | 79,87,95,104,111,116,121,126 | 8/17 |
| src/pso/object/switch/TOSensorGeneral01.cpp | 70,74,78,83 | 4/9 |
| src/pso/object/trap/TOTrapAncient01.cpp | 180 | 1/3 |
| src/pso/object/trap/TOTrapAncient02.cpp | 182,197,204,247,252,257,280,289,298 | 9/17 |
| src/pso/object/trap/TOTrapSeabed01.cpp | 264,267,275 | 3/5 |
| src/pso/object/warp/TObjAreaWarpEndingJung.cpp | 666,671,682,690,758,763,774,782,877,891 | 12/18 |
| src/pso/object/warp/TObjWarpJungMain.cpp | 289,290,291,292,293,294,295,296 | 8/8 |
| src/pso/player/GinjaRenderTarget.cpp | 13 | 1/2 |
| src/pso/player/TObjPlayer_8023E480.cpp | 91 | 1/1 |
| src/pso/render/AltModel.cpp | 36,59,65 | 3/3 |
| src/pso/render/CameraCapture.cpp | 214,235,245,247,264,302,317,358,399,470,728,785,802,810,827 | 15/74 |
| src/pso/render/ChunkModelDraw.cpp | 99,122,129,498,556 | 5/44 |
| src/pso/render/ChunkMotionDraw.cpp | 42,50,95,103 | 4/6 |
| src/pso/render/ChunkTextureStripCollector.cpp | 18,20,23,25,79,81,84,86 | 8/8 |
| src/pso/render/GinjaMaterial.cpp | 367,388,421 | 3/3 |
| src/pso/render/GinjaMotionDraw.cpp | 33,39,70,76 | 4/6 |
| src/pso/render/GinjaTextureHeader.cpp | 31 | 1/1 |
| src/pso/render/GinjaTextureTable.cpp | 20,24 | 2/2 |
| src/pso/render/MericarolTexture.cpp | 29 | 1/1 |
| src/pso/render/ModelDrawContext.cpp | 56,60,65,91,114,121,133,141 | 8/11 |
| src/pso/render/NinjaUtil.cpp | 12 | 1/5 |
| src/pso/render/ShapeMotionDraw.cpp | 49,50,52,57,63,143,144,149,150 | 9/11 |
| src/pso/render/TNodeMatrixRecorder.cpp | 43,45 | 2/3 |
| src/pso/render/psoLoadTexture.cpp | 89,145,149 | 3/3 |
| src/pso/render/reflection.cpp | 523 | 1/1 |
| src/pso/system/TLanguage.cpp | 49 | 1/1 |
| src/pso/technique/TTecCtrl.cpp | 185,189 | 2/2 |
| src/pso/technique/TTecZondeEff.cpp | 294,381,383 | 3/3 |
| src/pso/ui/charcreate/TChCrLoadAfsTextureReq.cpp | 42,112 | 2/2 |
| src/pso/ui/charcreate/TChCrLoadReq.cpp | 38,39,40,43,46 | 8/8 |
| src/pso/ui/charcreate/TCharaMaking.cpp | 516,551 | 2/2 |
| src/pso/ui/font/TNECFontEORFunction.cpp | 549 | 1/2 |
| src/pso/ui/frontend/TAdMcSelectCursorGC.cpp | 140 | 1/3 |
| src/pso/ui/frontend/TAdMcSelectMcGC.cpp | 112 | 1/3 |
| src/pso/ui/frontend/TAdPlyStatusGC.cpp | 81 | 1/1 |
| src/pso/ui/window/TWindowDD_8034435C.cpp | 2566 | 1/1 |
| src/pso/ui/window/TWindowDD_WS_Word.cpp | 346 | 1/1 |
| src/pso/ui/window/TWindowKeyBoardHard_801FBA48.cpp | 54,60 | 2/3 |
| src/pso/unk/SampledMotionPoints.cpp | 59 | 1/2 |
| src/pso/unk/TLobby_80209524.cpp | 1010,1029,1097,1110 | 4/7 |
| src/pso/unk/text_80066F5C.cpp | 21 | 1/3 |
| src/pso/unk/text_8028A998.cpp | 8,9,20,23 | 6/6 |
| src/pso/unk/text_80327844.cpp | 11,16 | 2/3 |
| src/pso/unk/text_803279CC.cpp | 10,15 | 2/3 |
| src/pso/unk/text_803DA16C.cpp | 11,12,16 | 3/3 |
| src/pso/unk/text_803DA798.cpp | 32,164,250 | 3/3 |
| src/pso/unk/text_803DD5E0.cpp | 38,40,46,89,134,137,139,175,179,202,426 | 11/12 |
| src/pso/weapon/SpecialProjectileWeapons.cpp | 119,120 | 2/3 |
| src/pso/weapon/TItemWeaponBelraCannon.cpp | 31 | 1/1 |
| src/pso/weapon/TItemWeaponDarkFlow.cpp | 38 | 1/1 |
| src/pso/weapon/TItemWeaponDrillLauncher.cpp | 45,125 | 2/3 |
| src/pso/weapon/TItemWeaponPanzerFaust.cpp | 61 | 1/1 |
| src/pso/weapon/TItemWeaponSRankWindmill.cpp | 23 | 1/1 |
| src/pso/weapon/TItemWeaponStrikerOfChao.cpp | 21,23,25 | 3/3 |
| src/pso/weapon/TItemWeaponWindmill.cpp | 88 | 1/1 |

Heaviest legs (adapter queue in priority order, reuse of NC §3 taxonomy):
- TSetManager.cpp — 48 dialect sites (FloorPopulation row create/destroy legs; NC §2: heaviest).
- TBoss2DeRolLe / TBoss7DeRolLeC / TBoss7BreakUp / TBoss2BreakUp — g2h-load + h2g-store clusters.
- PK-13 rows (TOContainerAncientEnemy, TODoorCave01-03, TODoorMachine01-02, TOKeyCave01,
  TOKeyMachine01, TOCapsuleLabo, TOCapsuleAncient01, TOMonumentAncient02, TBoss1Dragon,
  psoLoadTexture) — shared `draw_ginja_model(NJS_MODEL*, s16*)` signature: one adapter at
  the call wrapper converts all 13 (per PK doc: `NJS_HANDLE` load at the call site).
- GinjaMaterial :367/:388/:421 + GinjaTextureTable :20/:24 + psoLoadTexture :89/:145/:149 —
  already LIVE under the AV §4 delta==cluster gate; listed here for completeness of site map.
- TChCrLoadReq.cpp :38/:39/:40/:43/:46 — 8/8 errors, exactly NC §3#6 (PPC-window compare
  legs + is_node_tree_valid); stays commented until the window-test adapter lands.

## 4. RUNTIME-GATED n=260 — grep proof per row

Screen (line-level grep, first evidence cited per row; conservative): a row is gated if
its source contains host-pointer↔guest-address coercion that flows through slot space —
the exact class AR §5 named (parse-clean cannot expose it because C++ casts launder it:
`reinterpret_cast`, `saddr`/`uaddr` scalars, int-typed carriers):

- `ptr->saddr-cast` (49 sites): reinterpret_cast<saddr|uaddr|u32|…>(ptr-expression).
- `int->ptr-rcast` (137): reinterpret_cast<NJS_*/void*>(int-typed variable).
- `saddr-uaddr-coerce` (499): C-style `(saddr)`/`(uaddr)` casts of objects/this/globals —
  era guest-address idiom; under the shadow these values ARE slot payloads.
- `addr-or-this->int` (105): (u32|s32|int|long|…)(&symbol | this | new T).
- `slotint->ptr` (58): (NJS_*|void|TObject…) cast of an int-declared carrier variable.

Rows (first grep receipt; full evidence `cache/scratch/na/gated_final.json`):

| file | evidence |
|---|---|
| src/pso/audio/MusicControl.cpp | int->ptr-rcast @564: `safe_free(reinterpret_cast<void *>(lbl_805D62EC));` |
| src/pso/audio/SoundControlState.cpp | ptr->saddr-cast @641: `(s32)(id & 0xFFFF) == sound_request_voices[i].index && reinterpret_cast<saddr>(actor)…` |
| src/pso/audio/SoundRequests.cpp | saddr-uaddr-coerce @290: `idx = play_packed_sound_once(id, (saddr)position, volume, flags);` |
| src/pso/boss/TBoss3VoloptMissile.cpp | saddr-uaddr-coerce @150: `model = (saddr)boss3_volopt_resources->models[0x16].model;` |
| src/pso/boss/darkfalz/TBoss4DarkFalz.cpp | int->ptr-rcast @76: `CameraAngles *TObjBase::get_rotation() { return reinterpret_cast<CameraAngles *>(&rot…` |
| src/pso/boss/darkfalz/TOBoss4.cpp | slotint->ptr @74: `model_radius = ((GinjaModelBounds *)model->model)->radius;` |
| src/pso/boss/darkfalz/TObjDFDemo2.cpp | int->ptr-rcast @128: `set_texture_list(reinterpret_cast<NJS_TEXLIST *>(parts[i].texlist));` |
| src/pso/boss/darkfalz/TObjDFDemo4.cpp | int->ptr-rcast @184: `set_texture_list(reinterpret_cast<NJS_TEXLIST *>(zanzo_parts[i].texlist));` |
| src/pso/boss/darkfalz/TObjDFDemoF.cpp | int->ptr-rcast @97: `set_texture_list(reinterpret_cast<NJS_TEXLIST *>(textures[i]));` |
| src/pso/boss/galgryphon/Boss5GryphonBehavior.cpp | saddr-uaddr-coerce @814: `s32 self = (saddr)this;` |
| src/pso/boss/galgryphon/TBoss5GryphonLightningWing.cpp | saddr-uaddr-coerce @109: `wing = (saddr)operator new(sizeof(TBoss5GryphonLightningWing));` |
| src/pso/boss/goldragon/TBoss8DragonEffBreath.cpp | saddr-uaddr-coerce @31: `if ((y < 30.0f) && ((ent = (uaddr)get_enemy_entity(owner_entity_id)) != 0U)) {` |
| src/pso/boss/goldragon/TBoss8DragonLeaderPart.cpp | saddr-uaddr-coerce @225: `return (saddr)new TBoss8DragonPart(tl_04, &entry, hp);` |
| src/pso/boss/goldragon/TObjBoss8DemoRoom.cpp | saddr-uaddr-coerce @61: `resource = (saddr)new BmlResource(1, &TObjBoss8DemoRoom::resource_desc, 0, 0, NULL);` |
| src/pso/boss/olgaflow/TBoss6Mag.cpp | saddr-uaddr-coerce @665: `TBoss6Type2::send_6xAD_grid_positions(index, (saddr)pts, 3);` |
| src/pso/boss/olgaflow/TBoss6Rock.cpp | saddr-uaddr-coerce @160: `saddr model = (saddr)get_bml_model(boss6_rock_resource, boss6_rock_kind_params[kind_i…` |
| src/pso/boss/volopt/TBoss3Volopt_800D8ACC.cpp | saddr-uaddr-coerce @145: `spawn_particles_at((saddr)position);` |
| src/pso/boss/volopt/TObjVoloptPillar.cpp | int->ptr-rcast @70: `set_texture_list(reinterpret_cast<NJS_TEXLIST *>(volopt_pillar_textures));` |
| src/pso/boss/volopt/TObjVoloptShutter.cpp | int->ptr-rcast @62: `set_texture_list(reinterpret_cast<NJS_TEXLIST *>(volopt_shutter_global_param));` |
| src/pso/effect/item/TItemEffLandHit.cpp | saddr-uaddr-coerce @47: `((TItemWeapon *)item)->call_impact_callback((saddr)&start, (saddr)&end);` |
| src/pso/effect/particle/TParticle.cpp | int->ptr-rcast @253: `set_texture_list(reinterpret_cast<NJS_TEXLIST *>(particle_definitions[kind].texlist));` |
| src/pso/effect/particle/TPlyFollowParticleCtrl.cpp | saddr-uaddr-coerce @112: `return (saddr)particle;` |
| src/pso/enemy/deldepth/TObjEneDelDepth.cpp | saddr-uaddr-coerce @520: `this->set_draw_mode((saddr)((void *)3));` |
| src/pso/enemy/dubchik/TObjEneDubchik.cpp | saddr-uaddr-coerce @351: `TObjPlayer::free_if_valid((saddr)dubchik_variant_texlist);` |
| src/pso/enemy/epsilon/TObjEneEpsilonBody.cpp | saddr-uaddr-coerce @389: `if (TObjPlayer::is_valid_and_active((uaddr)player) != 0) {` |
| src/pso/enemy/epsilon/TObjEneEpsilonGuard.cpp | saddr-uaddr-coerce @521: `body = (uaddr)find_entity_by_id(body_id);` |
| src/pso/enemy/flower/TObjEneRe2Flower_Bomb.cpp | int->ptr-rcast @57: `p = reinterpret_cast<TObjChara *>(TObjPlayer::get_battle_param(i));` |
| src/pso/enemy/gibbles/TObjEneGibbles.cpp | saddr-uaddr-coerce @464: `if (TObjPlayer::is_valid_and_active((uaddr)p) != 0 && p != (TObjChara *)this) {` |
| src/pso/enemy/gibon/TObjEneBm5GibonU.cpp | slotint->ptr @2468: `__declspec(section ".ctors") static void *const __sinit_TObjEneBm5GibonU_cpp_ref = (v…` |
| src/pso/enemy/gyaranzo/TObjEneGyaranzo.cpp | saddr-uaddr-coerce @823: `if (TObjPlayer::is_valid_and_active((uaddr)p) == 0) continue;` |
| src/pso/enemy/gyaranzo/TObjGyaranzoMissile.cpp | saddr-uaddr-coerce @59: `if (TObjPlayer::is_valid_and_active((uaddr)target) != 0) {` |
| src/pso/enemy/gyaranzo/TObjGyaranzoMissile_800D40D8.cpp | saddr-uaddr-coerce @184: `mine_textures = (saddr)gyaranzo_bml_resource->models[5].texlist;` |
| src/pso/enemy/illgill/TObjEneIllGill.cpp | saddr-uaddr-coerce @1127: `void *m = (void *)new TObjIllGillKage((saddr)tl_05, (uaddr)this->target);` |
| src/pso/enemy/mericarol/TObjEneBm9Mericarol_Bomb.cpp | int->ptr-rcast @46: `obj = reinterpret_cast<TObjChara *>(TObjPlayer::get_battle_param(i));` |
| src/pso/enemy/moja/TObjEneMoja.cpp | saddr-uaddr-coerce @578: `copy_position_if_valid((saddr)this, (saddr)&cur);` |
| src/pso/enemy/morfos/TObjEneMorfosGroundShot.cpp | saddr-uaddr-coerce @80: `if (TObjPlayer::is_valid_and_active((uaddr)p) != 0 && fabs_float(pos.y - p->body_cent…` |
| src/pso/enemy/nanodrago/TObjEneNanoDrago.cpp | saddr-uaddr-coerce @564: `if ((uaddr) target == 0U) {` |
| src/pso/enemy/nanodrago/TObjNanoDragoSingleBeam.cpp | saddr-uaddr-coerce @128: `if (player != NULL && TObjPlayer::is_valid_and_active((uaddr)player) != 0) {` |
| src/pso/enemy/panarms/TObjEnePanarms.cpp | saddr-uaddr-coerce @1028: `set_state((saddr)((void*)(0xD)));` |
| src/pso/enemy/recon/TObjEneRecobox.cpp | saddr-uaddr-coerce @174: `recobox_behavior_params = (saddr)TLobby::dispatch_shared_handler_a(0x41);` |
| src/pso/enemy/recon/TObjEneRecon.cpp | saddr-uaddr-coerce @700: `if (TObjPlayer::is_valid_and_active((uaddr)p) != 0 && p != NULL && p->get_entity_id()…` |
| src/pso/enemy/recon/TObjEneReconBomb.cpp | saddr-uaddr-coerce @69: `bml_model = (saddr)get_bml_model(recon_bomb_bml_resource, 2);` |
| src/pso/enemy/slime/TObjEneSlimeOne.cpp | int->ptr-rcast @228: `texture_list = reinterpret_cast<NJS_TEXLIST *>(get_bml_texlist(resource, id));` |
| src/pso/enemy/slime/TObjEneSlimeTailAttack.cpp | saddr-uaddr-coerce @405: `slot->slime = (TObjEneSlimeOne *)TObjEneSlimeOne::create((saddr)origin, (saddr)&t);` |
| src/pso/enemy/sorcerer/TObjEneRe4Sorcerer.cpp | saddr-uaddr-coerce @615: `rotation_y = approach_angle_limited(rotation_y, (saddr)(RADIANS_TO_ANGLE * atan2_floa…` |
| src/pso/enemy/wolf/TObjEneBm5Wolf.cpp | slotint->ptr @135: `__declspec(section ".ctors") static void *const __sinit_TObjEneBm5Wolf_cpp_ref = (voi…` |
| src/pso/item/TItemData.cpp | saddr-uaddr-coerce @780: `if (TItemData::is_item_usable((saddr)player, data1.data_u8) != 0)` |
| src/pso/item/TItemData_8010EDA8.cpp | saddr-uaddr-coerce @68: `return (saddr)text_language_group_table[1][name];` |
| src/pso/item/TItemEquipBase.cpp | saddr-uaddr-coerce @299: `s32 item = (saddr)this;` |
| src/pso/item/TItemFollowStarLight.cpp | saddr-uaddr-coerce @21: `item = ((saddr)find_item_by_id(item_id));` |
| src/pso/item/TItemIdCounters.cpp | saddr-uaddr-coerce @94: `TObjPlayer::drop_meseta_from_slot(extended.effective_area, (saddr)&item);` |
| src/pso/item/TItemLockCountCtrl_80119238.cpp | saddr-uaddr-coerce @37: `uaddr temp = (uaddr)get_player_by_client_id((u16)client);` |
| src/pso/item/TItemObject.cpp | saddr-uaddr-coerce @59: `return encoded_amount ^ (saddr)&encoded_amount;` |
| src/pso/item/TItemTexLoad.cpp | saddr-uaddr-coerce @119: `bind_texture_name_table(&m_texture_list, &m_texture_entry, (uaddr)item_texture_filena…` |
| src/pso/item/TItemTimerEvent.cpp | saddr-uaddr-coerce @63: `if ((player = ((uaddr)get_player_by_client_id((u16)client))) != 0U) {` |
| src/pso/item/TItemToolTechniqueDisk.cpp | saddr-uaddr-coerce @9: `saddr disk = (saddr)this;` |
| src/pso/item/TObjItemLight.cpp | slotint->ptr @51: `apply_object_set_data((void *)set_data);` |
| src/pso/item/item_create.cpp | saddr-uaddr-coerce @845: `return (saddr)rec;` |
| src/pso/item/item_pmt.cpp | saddr-uaddr-coerce @48: `return (saddr)&table->lists[index];` |
| src/pso/item/shop/ShopWeaponStock.cpp | ptr->saddr-cast @551: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(lobby_weapon_random_tables_ptr));` |
| src/pso/item/shop/TShopGenerator.cpp | saddr-uaddr-coerce @142: `if (is_no_service_active(this, (saddr)q) == 0) return;` |
| src/pso/map/TSetEvtDestroy.cpp | saddr-uaddr-coerce @39: `if (TLobby::spawn_matching_objects(m_event_record->spawn_arg0, m_event_record->spawn_…` |
| src/pso/map/area/AreaCity.cpp | saddr-uaddr-coerce @72: `create_bag_warn_window_4p((saddr)tl_07);` |
| src/pso/network/GuildCardRecord.cpp | int->ptr-rcast @253: `init_player_entry(entry, reinterpret_cast<u8 *>(&response));` |
| src/pso/npc/TNpcPlayer_800D87A8.cpp | int->ptr-rcast @8: `TObjCollidable *player = reinterpret_cast<TObjCollidable *>(TObjPlayer::get_battle_pa…` |
| src/pso/npc/TObjNpcAHM01.cpp | ptr->saddr-cast @39: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(shared_object));` |
| src/pso/npc/TObjNpcARM00.cpp | ptr->saddr-cast @39: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(model_resource));` |
| src/pso/npc/TObjNpcARW00.cpp | ptr->saddr-cast @35: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(static_model));` |
| src/pso/npc/TObjNpcBase.cpp | saddr-uaddr-coerce @493: `npc_town_motion_data = (uaddr)load_encrypted_archive_data("PlyMotionNpcTown.cpt");` |
| src/pso/npc/TObjNpcHFW00.cpp | ptr->saddr-cast @35: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(shared_resource_obj));` |
| src/pso/npc/TObjNpcHFW01.cpp | ptr->saddr-cast @35: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(resource_node));` |
| src/pso/npc/TObjNpcHHM00.cpp | ptr->saddr-cast @31: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(model_object));` |
| src/pso/npc/TObjNpcHRM00.cpp | ptr->saddr-cast @31: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(named_object_resource));` |
| src/pso/npc/TObjNpcHRM01.cpp | ptr->saddr-cast @31: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(shared_object_resource));` |
| src/pso/npc/TObjNpcNFM00.cpp | ptr->saddr-cast @31: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(shared_named_model_object));` |
| src/pso/npc/TObjNpcNFM01.cpp | ptr->saddr-cast @31: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(model_object));` |
| src/pso/npc/TObjNpcNFW00.cpp | ptr->saddr-cast @31: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(named_object_resource));` |
| src/pso/npc/TObjNpcNFW01.cpp | ptr->saddr-cast @31: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(shared_object_resource));` |
| src/pso/npc/TObjNpcNHW00.cpp | ptr->saddr-cast @31: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(shared_named_model_object));` |
| src/pso/npc/TObjNpcNHW01.cpp | ptr->saddr-cast @31: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(model_object));` |
| src/pso/object/TObjControl.cpp | ptr->saddr-cast @234: `return reinterpret_cast<saddr>(groupRoot()) ? 1 : 0;` |
| src/pso/object/TObjHMS_EZRope_Player.cpp | saddr-uaddr-coerce @48: `return (saddr)this->joint_matrices + (joint * 0x30);` |
| src/pso/object/TObjLuker.cpp | saddr-uaddr-coerce @63: `saddr self_addr = (saddr)this;` |
| src/pso/object/base/TAttackCollider.cpp | int->ptr-rcast @78: `t = reinterpret_cast<TObjChara *>(TObjPlayer::get_battle_param(i));` |
| src/pso/object/base/TObjChara.cpp | saddr-uaddr-coerce @1426: `TOCtrlDubchik::create_font((saddr)pos, 0xFFFF0000, 0x14, miss_label_text);` |
| src/pso/object/base/TObjCollidable.cpp | ptr->saddr-cast @875: `find_or_alloc_battle_param_slot(reinterpret_cast<uaddr>(target));` |
| src/pso/object/cave/TORockCave.cpp | int->ptr-rcast @143: `u8 *archives = reinterpret_cast<u8 *>(&rock_cave_resource_table);` |
| src/pso/object/collision/TOAttackableCol.cpp | saddr-uaddr-coerce @271: `o = (void *)new TOAttackableCol((saddr)set_data);` |
| src/pso/object/collision/TOSymbolchatColli.cpp | saddr-uaddr-coerce @96: `balloon_chats[i] = (u8 *) (uaddr) symbolchat_colli_param_table[i];` |
| src/pso/object/collision/TObjFogCollision.cpp | saddr-uaddr-coerce @332: `p = (saddr)get_player_by_client_id(client & 0xFFFF);` |
| src/pso/object/collision/TObjInfoCol.cpp | slotint->ptr @48: `TObjInfoCol::TObjInfoCol(saddr parent, saddr set_data) : TObjCollidable((TObject *)((…` |
| src/pso/object/collision/TObjTradeCollision.cpp | slotint->ptr @39: `TObjTradeCollision::TObjTradeCollision(saddr parent, saddr set_data) : TObjCollidable…` |
| src/pso/object/container/TContainerAncient01.cpp | saddr-uaddr-coerce @294: `if (((uaddr) this->set_entry != 0U) && (access_object_saved_state(&saved_state, 1) !=…` |
| src/pso/object/door/TODoorJungleMain.cpp | saddr-uaddr-coerce @233: `created = (saddr)new BmlResource(0x1, &bm_o_jungle_doormain_resource_descriptor, 0, 0…` |
| src/pso/object/ending/TObjOnlineEndingMain.cpp | ptr->saddr-cast @318: `TObjPlayer::draw_player_model_by_client(entry->id, reinterpret_cast<saddr>(entry->mot…` |
| src/pso/object/fence/TOBarrierEnergy01.cpp | saddr-uaddr-coerce @90: `TObjPlayer::free_if_valid((saddr)barrier_energy_texlists[0]);` |
| src/pso/object/fence/TOFenceAncient01.cpp | saddr-uaddr-coerce @48: `TObjPlayer::free_if_valid((saddr)texlists[i]);` |
| src/pso/object/fence/TOFenceAncient02.cpp | saddr-uaddr-coerce @48: `TObjPlayer::free_if_valid((saddr)texlists[i]);` |
| src/pso/object/fence/TOFenceAncient03.cpp | saddr-uaddr-coerce @59: `TObjPlayer::free_if_valid((saddr)texlists[i]);` |
| src/pso/object/fence/TOFenceAncient04.cpp | saddr-uaddr-coerce @57: `TObjPlayer::free_if_valid((saddr)texlists[1]);` |
| src/pso/object/key/TOKeyGenericSw.cpp | saddr-uaddr-coerce @109: `TObjPlayer::free_if_valid((saddr)key_generic_sw_texture_lists[2]);` |
| src/pso/object/switch/TOChatSensor.cpp | saddr-uaddr-coerce @89: `TLobby::free_nodes_with_id((uaddr)chat_sensor_table);` |
| src/pso/object/switch/TOSwitchAttack.cpp | saddr-uaddr-coerce @112: `TOSwitchAttack::create_sub((saddr)this, stop_switch, trigger_switch, effect_type - 3,…` |
| src/pso/object/switch/TOSwitchTimer.cpp | saddr-uaddr-coerce @156: `void *o = (void *)new TOSwitchTimer((saddr)tl_05, (saddr)set_data);` |
| src/pso/object/trap/TOMine.cpp | saddr-uaddr-coerce @271: `if (TObjPlayer::is_valid_and_active((uaddr)obj) != 0) {` |
| src/pso/object/warp/TObjAreaWarp_80307B70.cpp | saddr-uaddr-coerce @13: `if ((saddr)m_down != 0) {` |
| src/pso/object/warp/TObjCityMainWarp.cpp | saddr-uaddr-coerce @153: `model = (saddr)city_common_model_archive->models[0].model;` |
| src/pso/object/warp/TObjCityMainWarpChallenge.cpp | saddr-uaddr-coerce @79: `model = (saddr)city_common_model_archive->models[0].model;` |
| src/pso/object/warp/TObjLaboMapWarp.cpp | int->ptr-rcast @617: `align_menu_entry_positions(reinterpret_cast<TWindowMenu *>(playerPopups[client_id]));` |
| src/pso/object/warp/TObjMapWarpAncient.cpp | saddr-uaddr-coerce @70: `TObjPlayer::free_if_valid((saddr)goal_warp_ancient_gvm_texture);` |
| src/pso/player/PlayerTaskEffects.cpp | saddr-uaddr-coerce @749: `s32 TObjPlayer::check_flag_04000000() { return check_flag((saddr)((void*)4194304)); }` |
| src/pso/player/TObjEndingPlayer.cpp | saddr-uaddr-coerce @63: `set_equip_slot_entry((saddr)equipped_items.items, slot, 0);` |
| src/pso/player/TObjPlayer.cpp | saddr-uaddr-coerce @542: `emit_photon_blast_feedback_effects((saddr)((s8 *)record + 4), *record);` |
| src/pso/player/TObjPlayerActions.cpp | saddr-uaddr-coerce @827: `void TObjPlayer::init_flags_vec4() { set_indexed_flags((saddr)((void*)0), (saddr)((vo…` |
| src/pso/player/TObjPlayerCreate.cpp | saddr-uaddr-coerce @515: `TObjPlayer::free_owned_buffer_b((saddr)&this->attack_state, (s16)(-1));` |
| src/pso/player/TObjPlayer_801AF2F0.cpp | int->ptr-rcast @435: `if ((lobby_font != 0U) && (reinterpret_cast<TObject *>(lobby_font)->check_value_equal…` |
| src/pso/player/TObjPlayer_801D8ADC.cpp | saddr-uaddr-coerce @294: `void free_ply_level_tbl() { TLobby::free_nodes_with_id((uaddr)ply_level_table); }` |
| src/pso/player/TObjPlayer_802C422C.cpp | saddr-uaddr-coerce @152: `void TObjPlayer::clear_flag_80000() { clear_state_flags((saddr)((void*)524288)); }` |
| src/pso/player/TPlayerBattleCtrl4P.cpp | saddr-uaddr-coerce @126: `saddr pl = (saddr)get_player_by_client_id(i);` |
| src/pso/player/TPlayerFootStamp.cpp | saddr-uaddr-coerce @70: `return new TPlayerFootStamp((TObjPlayer *)player, (saddr)side);` |
| src/pso/player/TPlyItem.cpp | saddr-uaddr-coerce @719: `saddr reset(void* p) { return (saddr)memset(p, 0, 28); }` |
| src/pso/quest/QuestDownloadAramFile.cpp | ptr->saddr-cast @109: `quest_download_aram_base + quest_download_aram_file.pos, reinterpret_cast<uaddr>(base…` |
| src/pso/quest/TQuestListOffline.cpp | ptr->saddr-cast @74: `TLobby::free_nodes_with_id(reinterpret_cast<uaddr>(story_quest_list_resource));` |
| src/pso/quest/TQuestListOnline.cpp | ptr->saddr-cast @72: `temp = reinterpret_cast<uaddr>(heading);` |
| src/pso/quest/TQuestThread.cpp | saddr-uaddr-coerce @488: `tmp = (saddr)src->code;` |
| src/pso/quest/TQuestVmDownLoadServer.cpp | saddr-uaddr-coerce @593: `if ((uaddr) this->mc_select != 0U) {` |
| src/pso/quest/challenge/TChallenge4P.cpp | saddr-uaddr-coerce @411: `player = (u8 *)(((saddr)get_player_by_client_id(i)));` |
| src/pso/quest/challenge/TChallenge4PItemSelect.cpp | slotint->ptr @254: `new TChallenge4PResult((TObject *)tl_01, client, result_code);` |
| src/pso/system/BmlResource.cpp | ptr->saddr-cast @175: `TLobby::set_read_params(&lobby_bml_file_handle, (int)n, reinterpret_cast<saddr>(bml_a…` |
| src/pso/system/GbaDownloadRuntime.cpp | slotint->ptr @37: `((TObject *)gba_download_graph)->queue_destruction();` |
| src/pso/system/TGStream.cpp | saddr-uaddr-coerce @186: `TLobby::set_read_params(&adxf_handle, (int)sector_count, (saddr)destination);` |
| src/pso/system/movie/TMovie.cpp | saddr-uaddr-coerce @300: `calc_frame_buffer_size((saddr)video_initial_display_framebuffer, (saddr)video_initial…` |
| src/pso/system/save/TSaveChCr.cpp | saddr-uaddr-coerce @155: `if ((uaddr) this->owned_window == 0U) {` |
| src/pso/system/scene/OnlineLobbyScene.cpp | int->ptr-rcast @266: `load_area_scene_and_init(reinterpret_cast<u8 *>(&floor_transition_state));` |
| src/pso/technique/TTecAnti.cpp | saddr-uaddr-coerce @107: `if ((TTecLuker::is_parents_unqueued(member) != 0) && (TObjPlayer::is_valid_and_active…` |
| src/pso/technique/TTecBarta.cpp | int->ptr-rcast @117: `attach_particle_model(reinterpret_cast<TParticleCtrl *>(this->emitters[i]), get_bml_m…` |
| src/pso/technique/TTecDeband.cpp | saddr-uaddr-coerce @80: `if ((TTecLuker::is_parents_unqueued(member) != 0) && (TObjPlayer::is_valid_and_active…` |
| src/pso/technique/TTecGiZonde.cpp | saddr-uaddr-coerce @114: `TObjPlayer::is_valid_and_active((uaddr) a) != 0 && TObjPlayer::is_valid_and_active((u…` |
| src/pso/technique/TTecPlayerInfluence.cpp | saddr-uaddr-coerce @116: `if (TObjPlayer::is_valid_and_active((uaddr)list->actors[i]) != 0) {` |
| src/pso/technique/TTecRaBarta.cpp | saddr-uaddr-coerce @191: `if ((uaddr) this->collidable == 0U) {` |
| src/pso/technique/TTecRaFoie.cpp | saddr-uaddr-coerce @120: `if (TTecLuker::is_parents_unqueued(chara) != 0 && TObjPlayer::is_valid_and_active((ua…` |
| src/pso/technique/TTecReverser.cpp | saddr-uaddr-coerce @66: `if (TObjPlayer::is_valid_and_active((uaddr) list->actors[i]) != 0) {` |
| src/pso/technique/TTecZonde.cpp | saddr-uaddr-coerce @177: `if (t != NULL && TTecLuker::is_parents_unqueued(t) != 0 && TObjPlayer::is_valid_and_a…` |
| src/pso/ui/TCharSaveSequence.cpp | saddr-uaddr-coerce @233: `src = (saddr)download_buffer;` |
| src/pso/ui/charcreate/TCMCharacter.cpp | saddr-uaddr-coerce @253: `res = (saddr)new BmlResource(6, (character_making_resource_descriptor_table)[characte…` |
| src/pso/ui/charcreate/TObjCMName.cpp | int->ptr-rcast @1833: `reinterpret_cast<ColorLineList *>(&proportion_frame_strip)->draw_strip_swap_color(4, …` |
| src/pso/ui/chat/TSymbolChatCapture.cpp | slotint->ptr @36: `TLobby::copy_symbol_chat_entry(&entries[i], (void *)src);` |
| src/pso/ui/chat/TWindowChat.cpp | saddr-uaddr-coerce @94: `uaddr kb = (uaddr)this->composer;` |
| src/pso/ui/chat/TWindowChatSymbol.cpp | saddr-uaddr-coerce @257: `saddr window_addr = (saddr)this;` |
| src/pso/ui/chat/TWindowSubShortMail.cpp | saddr-uaddr-coerce @479: `return (saddr)dialog;` |
| src/pso/ui/chat/TWindowSubSymbolChat.cpp | saddr-uaddr-coerce @182: `return (saddr)this;` |
| src/pso/ui/chat/TWindowSubSymbolChatHelp.cpp | saddr-uaddr-coerce @214: `saddr self_addr = (saddr)this;` |
| src/pso/ui/chat/TWindowSubSymbolChatPartsSet.cpp | saddr-uaddr-coerce @59: `second_entry = get_parts_entry((saddr)i);` |
| src/pso/ui/chat/TWindowSubSymbolChat_802ACF5C.cpp | saddr-uaddr-coerce @9: `zero_vec2 = (saddr)get_zero_vec2();` |
| src/pso/ui/city/TCheckRoomSequence.cpp | saddr-uaddr-coerce @550: `TCheckRoomSequence::active_instance = (saddr)this;` |
| src/pso/ui/city/THunterGuildSeqOnlineTask_802156A0.cpp | saddr-uaddr-coerce @204: `TLobby::free_nodes_with_id((uaddr)((void*)item_appraisal_rules));` |
| src/pso/ui/city/THunterGuildWarning.cpp | slotint->ptr @141: `open_warning_windows((void *)mode);` |
| src/pso/ui/city/TShopItemListWindow.cpp | int->ptr-rcast @193: `if (player->can_inventory_accept_item(reinterpret_cast<u8 *>(&shop_items[sel])) == 0) {` |
| src/pso/ui/city/TShopSequence.cpp | saddr-uaddr-coerce @49: `return (uaddr)message_window->create_message_window(position, extent, (u16)((TObjBase…` |
| src/pso/ui/font/TFontSmallTask.cpp | saddr-uaddr-coerce @171: `return (saddr)new TFontSmallTask(pos, color, life, delay, get_shared_text_buffer(), v…` |
| src/pso/ui/frontend/TAdChrMoveEffectGC.cpp | int->ptr-rcast @54: `reinterpret_cast<u8 *>(&desc.col[0].color)[3] = e->entry.alpha;` |
| src/pso/ui/frontend/TAdChrSelPlayerGC.cpp | ptr->saddr-cast @146: `TAdPlyStatusGC::set_status_index(reinterpret_cast<saddr>(status), status_index);` |
| src/pso/ui/frontend/TAdChrSelPlyPlateGC.cpp | saddr-uaddr-coerce @117: `TAdPlyPltAnimCtrlGC::get_instance((saddr)tl_clipout);` |
| src/pso/ui/frontend/TAdDialingGC.cpp | saddr-uaddr-coerce @424: `if (is_skip_pressed((saddr)this) != 0) {` |
| src/pso/ui/frontend/TAdEnterBaseGC.cpp | saddr-uaddr-coerce @319: `draw_masked_password_field((saddr)context);` |
| src/pso/ui/frontend/TAdEnterPPPPassGC.cpp | saddr-uaddr-coerce @130: `password_entry_keyboard_control = TAdEnterSerialAccessGC::create_keyboard_control((sa…` |
| src/pso/ui/frontend/TAdEnterPasswordGC.cpp | saddr-uaddr-coerce @68: `saddr buf = (saddr)password_entry_keyboard_control->realloc_buffer();` |
| src/pso/ui/frontend/TAdEnterSerialAccessGC.cpp | saddr-uaddr-coerce @136: `keyboard_control = create_keyboard_control((saddr)this, 0);` |
| src/pso/ui/frontend/TAdFadeGC.cpp | int->ptr-rcast @84: `draw_polygon_2d(reinterpret_cast<NJS_POINT2COL *>(&d), 4, 7, -1.0f);` |
| src/pso/ui/frontend/TAdGameStartBackGC.cpp | int->ptr-rcast @18: `set_texture_list(reinterpret_cast<NJS_TEXLIST *>(game_start_background_texture_list));` |
| src/pso/ui/frontend/TAdGameStartShadowGC.cpp | int->ptr-rcast @17: `set_texture_list(reinterpret_cast<NJS_TEXLIST *>(TAdGameStartShadowGC::texture_list));` |
| src/pso/ui/frontend/TAdGameStartShineGC.cpp | int->ptr-rcast @21: `set_texture_list(reinterpret_cast<NJS_TEXLIST *>(multiplay_texlist));` |
| src/pso/ui/frontend/TAdGameStartSubGC.cpp | int->ptr-rcast @24: `set_texture_list(reinterpret_cast<NJS_TEXLIST *>(game_start_player_instance));` |
| src/pso/ui/frontend/TAdMultiPlyChrSelectGC.cpp | int->ptr-rcast @177: `this->fade_task = reinterpret_cast<TAdFadeGC *>(v);` |
| src/pso/ui/frontend/TAdMultiPlyChrSelectGC_802C6F88.cpp | saddr-uaddr-coerce @23: `int TAdMultiPlyChrSelectGC::get_global_6700() {  return (saddr)menu_selection_cursor_…` |
| src/pso/ui/frontend/TAdMultiSelGameModeGC.cpp | saddr-uaddr-coerce @119: `return (saddr)r;` |
| src/pso/ui/frontend/TAdPlyPltAnimCtrlGC.cpp | saddr-uaddr-coerce @27: `ad_ply_plt_anim_ctrl_gc = (uaddr)new TAdPlyPltAnimCtrlGC((TObject *)parent);` |
| src/pso/ui/frontend/TAdSelectTxtSubGC.cpp | int->ptr-rcast @20: `set_texture_list(reinterpret_cast<NJS_TEXLIST *>(status_disp_texture_handle));` |
| src/pso/ui/frontend/TAdSinglePlyChrSelectGC_801FBF54.cpp | ptr->saddr-cast @1890: `p = reinterpret_cast<saddr>(reinterpret_cast<TPlyCharData *>(get_selected_character_s…` |
| src/pso/ui/frontend/TAdSystemFileCheckGC.cpp | saddr-uaddr-coerce @368: `s32 this_addr = (saddr)this;` |
| src/pso/ui/frontend/TAdTimeSetGC.cpp | saddr-uaddr-coerce @832: `menu_window = (TWindowMenu *)(saddr)create_window_menu_default(&time_set_menu_pos, 2);` |
| src/pso/ui/frontend/TAdTrialAgreementGC.cpp | saddr-uaddr-coerce @39: `void TAdTrialAgreementGC::on_decline() { request_main_phase((saddr)((void*)0)); }` |
| src/pso/ui/frontend/TAdYesNoBaseGC.cpp | saddr-uaddr-coerce @25: `window = (saddr)create_message_window_for_size(&this->message_position, &this->messag…` |
| src/pso/ui/frontend/TTextureWave.cpp | saddr-uaddr-coerce @41: `this->output = (saddr)TQuestVmDownLoadServer::alloc((this->height * (this->width * 2)…` |
| src/pso/ui/frontend/TTitleBackground.cpp | saddr-uaddr-coerce @131: `draw_title_background_pulse((saddr)this, -140.0f, frame);` |
| src/pso/ui/hud/TMainCursor.cpp | saddr-uaddr-coerce @227: `resource = (saddr)new BmlResource(5, &target_cursor_resource_descriptor, 0, 0, NULL);` |
| src/pso/ui/hud/TRaderMapCore.cpp | slotint->ptr @255: `if ((void *) TRaderMapCore::floor_map_resource != NULL) {` |
| src/pso/ui/keyboard/KeyboardKeyState.cpp | saddr-uaddr-coerce @304: `set_si_polling_callback((uaddr)((s32 *)KeyboardKeyState::append_next_char_global));` |
| src/pso/ui/window/ItemListRow.cpp | saddr-uaddr-coerce @512: `item_data = get_item_data((uaddr)item);` |
| src/pso/ui/window/TCustomizeMenuCtrl.cpp | saddr-uaddr-coerce @45: `custom_equip = (uaddr)new TWindowSubCustomEquip(client_id);` |
| src/pso/ui/window/TObjResultWin.cpp | saddr-uaddr-coerce @139: `m_window = (uaddr)TWindowMessage::create((const NJS_POINT2 *)(get_window_layout_posit…` |
| src/pso/ui/window/TWindowActionPalette.cpp | saddr-uaddr-coerce @158: `TWindowHelpItem::set_help_text((saddr)help, client_index);` |
| src/pso/ui/window/TWindowChoiceSetting_80249078.cpp | ptr->saddr-cast @32: `steps = reinterpret_cast<uaddr>(this->get_array_entry(this->get_field_74()));` |
| src/pso/ui/window/TWindowCtrl.cpp | saddr-uaddr-coerce @439: `if ((chat_window->is_mode_bit4() != 0) && (chat_accept_submission((saddr)&this->submi…` |
| src/pso/ui/window/TWindowCtrlMain_800FC91C.cpp | saddr-uaddr-coerce @25: `saddr TWindowCtrlMain::get_instance() {  return (saddr)instance; }` |
| src/pso/ui/window/TWindowDD_803430BC.cpp | saddr-uaddr-coerce @260: `TLobby::free_nodes_with_id((uaddr)((void*)TWindowDD::ws_data_fr));` |
| src/pso/ui/window/TWindowEffectLight.cpp | int->ptr-rcast @42: `WindowDrawContext::adjust_pos_for_split_screen(reinterpret_cast<NJS_POINT2 *>(&x), TC…` |
| src/pso/ui/window/TWindowEffectParty.cpp | saddr-uaddr-coerce @42: `this->m_particle = spawn_continuous_particle_effect_with_flags(&position, (saddr)effe…` |
| src/pso/ui/window/TWindowFukidasi.cpp | saddr-uaddr-coerce @91: `if ((uaddr)this->name != 0U) {` |
| src/pso/ui/window/TWindowGetComCard.cpp | saddr-uaddr-coerce @92: `TWindowSubComCard::start_player_data_load((saddr)get_received_guild_card_tags());` |
| src/pso/ui/window/TWindowHelpItem.cpp | saddr-uaddr-coerce @176: `call_if_split_screen((uaddr)this, this->client_index);` |
| src/pso/ui/window/TWindowKeyBoardBase.cpp | saddr-uaddr-coerce @299: `p = (u8 *)(idx + (saddr)rec);` |
| src/pso/ui/window/TWindowKeyBoardHard.cpp | ptr->saddr-cast @194: `return reinterpret_cast<saddr>(this);` |
| src/pso/ui/window/TWindowKeyBoardHard_801A529C.cpp | slotint->ptr @68: `memcpy((void *)(deferred_packet_buffer + deferred_packet_buffer_used), pkt, len);` |
| src/pso/ui/window/TWindowLockOn.cpp | int->ptr-rcast @163: `reinterpret_cast<WindowLayoutEntries *>(&entry_list)->destroy_all_items();` |
| src/pso/ui/window/TWindowMainMenu.cpp | saddr-uaddr-coerce @238: `this->submenu = (saddr)new TWindowStatusCategoryMenu(this, this->client_index);` |
| src/pso/ui/window/TWindowMenuIconLineLists.cpp | int->ptr-rcast @143: `count = player->inventory_objects->run_and_get_result(reinterpret_cast<s32 *>(ids));` |
| src/pso/ui/window/TWindowMessageYesNo.cpp | int->ptr-rcast @108: `dialog->message_menu = reinterpret_cast<TWindowMenu *>(create_titled_window_menu(mess…` |
| src/pso/ui/window/TWindowOptionCtrl.cpp | saddr-uaddr-coerce @71: `return (saddr)TAdSinglePlyChrSelectGC::get_instance()->password;` |
| src/pso/ui/window/TWindowQuantity.cpp | saddr-uaddr-coerce @75: `return new TWindowQuantity((saddr)client);` |
| src/pso/ui/window/TWindowShortListCtrl.cpp | saddr-uaddr-coerce @272: `refresh_technique_rows((saddr)tech_window, &short_list_window_pos);` |
| src/pso/ui/window/TWindowSnapShot.cpp | saddr-uaddr-coerce @70: `player_motion_archive->start_capture((saddr)player_motion_data_filename_ptr);` |
| src/pso/ui/window/TWindowSubComCard.cpp | saddr-uaddr-coerce @456: `((TWindowSubComCard *)(this->selected_card))->set_card_text((saddr) (get_shared_text_…` |
| src/pso/ui/window/TWindowSubComCardList.cpp | saddr-uaddr-coerce @77: `msg = (saddr)TWindowSubStatusDisp::get_message_10(0xD4);` |
| src/pso/ui/window/TWindowSubComCard_8007341C.cpp | saddr-uaddr-coerce @165: `TLobby::open_challenge_window((saddr)this, mode);` |
| src/pso/ui/window/TWindowSubComCard_8007A810.cpp | ptr->saddr-cast @31: `slot = TWindowSubComCard::get_card_entry(static_cast<s32>(reinterpret_cast<saddr>(thi…` |
| src/pso/ui/window/TWindowSubComCard_8007BD24.cpp | int->ptr-rcast @12: `return has_free_card_slot(reinterpret_cast<GuildCardEntry *>(TWindowSubComCard::get_c…` |
| src/pso/ui/window/TWindowSubCustomEquip.cpp | saddr-uaddr-coerce @273: `TWindowHelpItem::set_help_text((saddr)b, (s32)this->client_index);` |
| src/pso/ui/window/TWindowSubEquip.cpp | saddr-uaddr-coerce @174: `temp_r3b = (uaddr)find_item_by_id(item);` |
| src/pso/ui/window/TWindowSubEquipKind_80278300.cpp | saddr-uaddr-coerce @53: `msg = (saddr)TWindowSubStatusDisp::get_message_10(status_disp_tab_label_message_ids[t…` |
| src/pso/ui/window/TWindowSubItem.cpp | saddr-uaddr-coerce @505: `item = find_item_by_id((saddr)((void *) item_id));` |
| src/pso/ui/window/TWindowSubLog.cpp | saddr-uaddr-coerce @31: `if ((chat->is_mode_bit4() != 0) && (chat_accept_submission((saddr)&submission) != 0)) {` |
| src/pso/ui/window/TWindowSubMag.cpp | saddr-uaddr-coerce @191: `call_if_split_screen((uaddr)help, client_id);` |
| src/pso/ui/window/TWindowSubQuestBoard.cpp | saddr-uaddr-coerce @184: `return (saddr)titled_window_menu;` |
| src/pso/ui/window/TWindowSubStatusDisp_800FBD74.cpp | saddr-uaddr-coerce @64: `set_draw_texture_and_color(0, (saddr)color);` |
| src/pso/ui/window/TWindowSubStatusDisp_8036D774.cpp | int->ptr-rcast @49: `reinterpret_cast<ColorLineList *>(&d)->draw_strip_swap_color(4, 0x16, (-2.0f));` |
| src/pso/ui/window/TWindowSubTradeMain.cpp | saddr-uaddr-coerce @150: `if (this->chat->is_mode_bit4() != 0 && chat_accept_submission((saddr)&this->submissio…` |
| src/pso/ui/window/TWindowTimer.cpp | saddr-uaddr-coerce @72: `window_layout_position = (saddr)get_window_layout_position(0xA9);` |
| src/pso/ui/window/WindowLayoutEntries.cpp | ptr->saddr-cast @68: `address = reinterpret_cast<uaddr>(items);` |
| src/pso/unk/TLobby_8001D6A0.cpp | saddr-uaddr-coerce @90: `move_data(((bank->attacks)), ((void *) ((((saddr) file) + offsetof(BattleParameterFil…` |
| src/pso/unk/TLobby_800F8AA8.cpp | saddr-uaddr-coerce @838: `TLobby::set_keyboard_mode((saddr)(init_chr_select));` |
| src/pso/unk/TLobby_801F3A24.cpp | saddr-uaddr-coerce @217: `return add_floor_event_set(record_floor, (saddr)(record + 1), size) != 0;` |
| src/pso/unk/TLobby_80207644.cpp | saddr-uaddr-coerce @408: `TAdStorySelectGC::set_state((saddr)selected_episode);` |
| src/pso/unk/TLobby_80207DA8.cpp | saddr-uaddr-coerce @91: `void TLobby::reset_story_select() { TAdStorySelectGC::set_state((saddr)((void*)0)); s…` |
| src/pso/unk/text_80076BC4.cpp | saddr-uaddr-coerce @62: `void send_chat_shortcut(s32 index) { chat_accept_submission((saddr)get_chat_shortcut(…` |
| src/pso/unk/text_80103674.cpp | saddr-uaddr-coerce @12: `item_nj_chunk_buffer_table[i] = (uaddr)mem_alloc(0x200);` |
| src/pso/unk/text_8018E258.cpp | saddr-uaddr-coerce @16: `p = (saddr)get_player_by_client_id(i);` |
| src/pso/unk/text_801A2710.cpp | saddr-uaddr-coerce @37: `void *o = (void *)new TODoorFourLightRuins((saddr)tl_05, (saddr)set_data);` |
| src/pso/unk/text_801A2B38.cpp | saddr-uaddr-coerce @39: `return new TObjItemLight(tl_05, (saddr)set_data);` |
| src/pso/unk/text_80278460.cpp | saddr-uaddr-coerce @24: `if ((t = (uaddr)find_item_by_id(e->get_item())) != 0 && ((TItem *)t)->check_equip_req…` |
| src/pso/weapon/TItemAttackCol.cpp | saddr-uaddr-coerce @272: `model = (uaddr)params->model;` |
| src/pso/weapon/TItemFollowEffect.cpp | saddr-uaddr-coerce @20: `item = (saddr)find_item_by_id(item_id);` |
| src/pso/weapon/TItemWeapon.cpp | saddr-uaddr-coerce @127: `void TItemWeapon::release_model() { memset(trails, 0, 16); clear_weapon_flags((uaddr)…` |
| src/pso/weapon/TItemWeaponBaranzLauncher.cpp | saddr-uaddr-coerce @17: `model_shot = TObjPlayer::create_shoot_model((uaddr)player, pos, dir, life);` |
| src/pso/weapon/TItemWeaponBranchOfPakupaku.cpp | saddr-uaddr-coerce @10: `saddr item = (saddr)this;` |
| src/pso/weapon/TItemWeaponBroom.cpp | saddr-uaddr-coerce @10: `saddr item = (saddr)this;` |
| src/pso/weapon/TItemWeaponFlowerBouquet.cpp | saddr-uaddr-coerce @11: `saddr weapon = (saddr)this;` |
| src/pso/weapon/TItemWeaponInfernoBazooka.cpp | saddr-uaddr-coerce @67: `TObjPlayer::launch_weapon_casing((saddr)tl_05, (uaddr)this, 3, (uaddr)&v, (uaddr)rota…` |
| src/pso/weapon/TItemWeaponNeisClaw.cpp | saddr-uaddr-coerce @131: `uaddr p = (uaddr)find_item_by_id(id);` |
| src/pso/weapon/TItemWeaponPlantainHugeFan.cpp | saddr-uaddr-coerce @13: `if ((is_owner_alive() != 0) && ((uaddr)model != 0U) && ((uaddr)texture_list != 0U)) {` |
| src/pso/weapon/TItemWeaponRocketPunch.cpp | saddr-uaddr-coerce @51: `if ((TItemShootModel::is_owner_alive() != 0) && ((uaddr) this->model != 0U) && ((uadd…` |
| src/pso/weapon/TItemWeaponSRank.cpp | saddr-uaddr-coerce @144: `send_srank_stat_extra((uaddr)target, body);` |
| src/pso/weapon/TItemWeaponSRankBazooka.cpp | saddr-uaddr-coerce @38: `TObjPlayer::launch_weapon_casing((saddr)tl_05, (uaddr)this, 3, (uaddr)&v, (uaddr)a);` |
| src/pso/weapon/TItemWeaponSRankPunch.cpp | saddr-uaddr-coerce @48: `if ((is_owner_alive() != 0) && ((uaddr)model != 0U) && ((uaddr)texture_list != 0U)) {` |
| src/pso/weapon/TItemWeaponWand.cpp | saddr-uaddr-coerce @10: `saddr item = (saddr)this;` |
| src/pso/weapon/TItemWeaponYasminkov2000H.cpp | saddr-uaddr-coerce @65: `TObjPlayer::launch_weapon_casing((saddr)tl_05, (uaddr)this, 3, (uaddr)&dir, (uaddr)an…` |
| src/pso/weapon/TItemWeaponYasminkov3000R.cpp | saddr-uaddr-coerce @31: `TObjPlayer::launch_weapon_casing((saddr)tl_05, (uaddr)this, 3, (uaddr)&dir, (uaddr)an…` |
| src/pso/weapon/TItemWeaponYasminkov7000V.cpp | saddr-uaddr-coerce @51: `TObjPlayer::launch_weapon_casing((saddr)tl_05, (uaddr) this, 2, (uaddr) &casing_veloc…` |
| src/pso/weapon/TItemWeaponYasminkov9000M.cpp | saddr-uaddr-coerce @75: `TObjPlayer::spawn_weapon_casing((saddr)tl_05, (uaddr)this, (uaddr)pos, (uaddr)&dir, (…` |
| src/pso/weapon/WeaponAttackActions.cpp | saddr-uaddr-coerce @186: `((s32 (*)(s32, void *, s32, s32))impact_callback)((saddr)tl_05, this, start, end);` |

These rows PARSE CLEAN with `-Icompat/njs` (§1 receipts) but touch host pointers in u32
slots at runtime; adopting them before the uaddr handle program (loader stamping +
guest↔host translation at these sites) would convert loud-parse risk into silent runtime
risk. Per-site adjudication stays with the handle program; this list is the named blast radius.

## 5. other-class BOTHFAIL rows (83, not NJS — tail for §4.9/TL census)

| file | n-errors |
|---|---|
| src/pso/animation/MotionContext.cpp | 2 |
| src/pso/audio/CriSvm.cpp | 1 |
| src/pso/audio/GcrnaStream.cpp | 1 |
| src/pso/audio/LscLog.cpp | 1 |
| src/pso/boss/dragon/TObjBoss1Floor.cpp | 1 |
| src/pso/boss/olgaflow/TBoss6Demo10_TYPE1_APPEAR.cpp | 1 |
| src/pso/boss/olgaflow/TBoss6Demo11_TYPE1_DEAD.cpp | 1 |
| src/pso/boss/olgaflow/TBoss6Demo15_TYPE2_WAIT.cpp | 1 |
| src/pso/boss/olgaflow/TBoss6Demo20_TYPE2_APPEAR.cpp | 1 |
| src/pso/boss/olgaflow/TBoss6Demo21_TYPE2_DEAD.cpp | 1 |
| src/pso/boss/olgaflow/TBoss6GravBall.cpp | 1 |
| src/pso/boss/olgaflow/TBoss6Kakusan.cpp | 1 |
| src/pso/boss/olgaflow/TBoss6MagMine.cpp | 1 |
| src/pso/boss/olgaflow/TBoss6Mine.cpp | 1 |
| src/pso/boss/olgaflow/TBoss6PlotFalz.cpp | 1 |
| src/pso/boss/olgaflow/TBoss6Type1.cpp | 1 |
| src/pso/boss/olgaflow/TBoss6Type1Actions.cpp | 1 |
| src/pso/effect/TEffClitical.cpp | 1 |
| src/pso/effect/photon/TPhotonEstlla.cpp | 1 |
| src/pso/effect/photon/TPhotonLeilla.cpp | 1 |
| src/pso/effect/photon/TPhotonPilla.cpp | 1 |
| src/pso/math/MatrixTransforms.cpp | 2 |
| src/pso/math/MatrixVector.cpp | 2 |
| src/pso/math/SinCosTable.cpp | 2 |
| src/pso/network/NetworkConnect.cpp | 23 |
| src/pso/network/NetworkInfoLoader.cpp | 4 |
| src/pso/network/TLobby.cpp | 7 |
| src/pso/network/TProtocol.cpp | 5 |
| src/pso/npc/TNpcPlayer.cpp | 1 |
| src/pso/object/cave/TOAirconCave01.cpp | 1 |
| src/pso/object/cave/TOSignCave01.cpp | 17 |
| src/pso/object/cave/TOSignCave02.cpp | 17 |
| src/pso/object/door/TObjDoor.cpp | 13 |
| src/pso/object/fence/TOFenceBase.cpp | 1 |
| src/pso/object/key/TOKeyAncient01.cpp | 1 |
| src/pso/player/EquippedItemDraw.cpp | 1 |
| src/pso/player/TPlayerBattleCtrl.cpp | 1 |
| src/pso/quest/QuestOpcodeHandlers.cpp | 1 |
| src/pso/quest/TOQuestTalkBase.cpp | 1 |
| src/pso/quest/challenge/ChallengeMode.cpp | 1 |
| src/pso/render/DebugFont.cpp | 3 |
| src/pso/render/GinjaDrawUtil.cpp | 25 |
| src/pso/render/SceneFog.cpp | 2 |
| src/pso/system/SiPolling.cpp | 2 |
| src/pso/system/movie/Gcci.cpp | 23 |
| src/pso/system/movie/MovieFramePack.cpp | 2 |
| src/pso/system/save/McFsGc.cpp | 22 |
| src/pso/system/scene/TEndingAB.cpp | 60 |
| src/pso/system/scene/TEndingCWall.cpp | 3 |
| src/pso/technique/TTecGrants.cpp | 1 |
| src/pso/technique/TechniqueRingwaveList.cpp | 10 |
| src/pso/ui/UISprite.cpp | 21 |
| src/pso/ui/charcreate/TChCrEffFade.cpp | 4 |
| src/pso/ui/city/THunterGuildSeqOffline.cpp | 1 |
| src/pso/ui/frontend/TAdBackGC.cpp | 9 |
| src/pso/ui/frontend/TAdBackTxtGC.cpp | 6 |
| src/pso/ui/frontend/TAdChrSelSlotGC.cpp | 4 |
| src/pso/ui/frontend/TAdEndText.cpp | 6 |
| src/pso/ui/frontend/TAdGameStartGC.cpp | 2 |
| src/pso/ui/frontend/TAdMultiOkGC.cpp | 2 |
| src/pso/ui/frontend/TAdMultiPlayerNoGC.cpp | 4 |
| src/pso/ui/frontend/TAdPlyPltAnimGC.cpp | 2 |
| src/pso/ui/frontend/TAdRayGC.cpp | 41 |
| src/pso/ui/frontend/TAdSEGAlogo.cpp | 25 |
| src/pso/ui/frontend/TAdSelectCurGC.cpp | 4 |
| src/pso/ui/frontend/TAdSelectTxtGC.cpp | 6 |
| src/pso/ui/frontend/TAdTitleGC.cpp | 6 |
| src/pso/ui/frontend/TAdTitleTexGC.cpp | 4 |
| src/pso/ui/frontend/TTitleScene.cpp | 22 |
| src/pso/ui/hud/TLoadingCur.cpp | 1 |
| src/pso/ui/keyboard/SystemBootInit.cpp | 23 |
| src/pso/ui/window/TWindowCallBack.cpp | 1 |
| src/pso/ui/window/TWindowSubTechnique.cpp | 1 |
| src/pso/ui/window/WindowHeading.cpp | 14 |
| src/pso/unk/TLobby_803382E4.cpp | 4 |
| src/pso/unk/text_801D34E0.cpp | 1 |
| src/pso/unk/text_803D9650.cpp | 2 |
| src/pso/unk/text_803D96A8.cpp | 2 |
| src/pso/unk/text_8042E8C4.cpp | 3 |
| src/pso/weapon/TItemWeaponBringersRifle.cpp | 1 |
| src/pso/weapon/TItemWeaponChainSawd.cpp | 1 |
| src/pso/weapon/TItemWeaponDelsabersBuster.cpp | 1 |
| src/pso/weapon/shoot/TItemShoot.cpp | 1 |

(error text unclassified as NJS reinterpretation: negative-array asserts outside the
shadow classes, MWCC ctor/dtor runtime-init dialect, duplicate waived-assert names,
asm bodies, tail. None is cured by `-Icompat/njs` — measured, not assumed.)

## 6. Paste-ready `build/njs_allowlist.txt` block — ADOPT-NOW ONLY (808 rows)

SUBMISSION ONLY (integrator applies; law: never lane-side). C++ TUs only; every row
python-verified `.cpp` AND FLIP ∧ NJS-err-0-byte ∧ not-already-listed ∧ not-runtime-gated.

```
# stage 3 ADOPT-NOW (P2-NA docs/p2-njs-adopt-1.md §6; probe receipts §1-§2, runtime screen §4):
src/pso/animation/MotionSampling.cpp
src/pso/audio/SoundPlayback.cpp
src/pso/audio/SoundRequestVoice.cpp
src/pso/boss/TBoss3VoloptCageShot.cpp
src/pso/boss/TBoss3VoloptFadeIn.cpp
src/pso/boss/TBoss3VoloptLine.cpp
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
src/pso/boss/barbaray/TObjBoss7Tunnel.cpp
src/pso/boss/barbaray/TObjBoss7Tunnel_802F190C.cpp
src/pso/boss/darkfalz/DrainPlayerHp.cpp
src/pso/boss/darkfalz/ModelNodeTables.cpp
src/pso/boss/darkfalz/TBoss4CheckEmpty.cpp
src/pso/boss/darkfalz/TBoss4DarkFalz_8004D8C8.cpp
src/pso/boss/darkfalz/TBoss4Fade.cpp
src/pso/boss/darkfalz/TBoss4Thanks.cpp
src/pso/boss/darkfalz/TBoss4Type1.cpp
src/pso/boss/darkfalz/TBoss4Type2_800522CC.cpp
src/pso/boss/darkfalz/TBoss4Type3.cpp
src/pso/boss/darkfalz/TBoss4Type3_800524E8.cpp
src/pso/boss/darkfalz/TBoss4Zako.cpp
src/pso/boss/darkfalz/TBoss4Zanzo.cpp
src/pso/boss/darkfalz/TOBoss4Rock.cpp
src/pso/boss/darkfalz/TObjDFDemo0.cpp
src/pso/boss/darkfalz/TObjDFDemo3.cpp
src/pso/boss/darkfalz/TObjDFDemo5.cpp
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
src/pso/boss/galgryphon/TBoss5GryphonColli.cpp
src/pso/boss/galgryphon/TBoss5GryphonDemo.cpp
src/pso/boss/galgryphon/TBoss5GryphonLightning.cpp
src/pso/boss/galgryphon/TBoss5GryphonLightningHorn.cpp
src/pso/boss/galgryphon/TBoss5GryphonShockWave.cpp
src/pso/boss/galgryphon/TBoss5GryphonSnarl.cpp
src/pso/boss/galgryphon/TBoss5GryphonTornado.cpp
src/pso/boss/galgryphon/TObjBoss5Ctrl.cpp
src/pso/boss/galgryphon/TObjBoss5FloorGrass.cpp
src/pso/boss/galgryphon/TObjBoss5Pillar.cpp
src/pso/boss/galgryphon/TObjBoss5PillarDemo.cpp
src/pso/boss/goldragon/AreaAttackHistory.cpp
src/pso/boss/goldragon/Boss8DemoObjects.cpp
src/pso/boss/goldragon/TBoss8DragonColli.cpp
src/pso/boss/goldragon/TBoss8DragonDemo.cpp
src/pso/boss/goldragon/TBoss8DragonEffBullet.cpp
src/pso/boss/goldragon/TBoss8DragonEffTail.cpp
src/pso/boss/goldragon/TBoss8DragonPart.cpp
src/pso/boss/goldragon/TBoss8Dragon_8005D6F4.cpp
src/pso/boss/goldragon/TBoss8Dragon_8005D700.cpp
src/pso/boss/goldragon/TBoss8Dragon_80067034.cpp
src/pso/boss/goldragon/TObjBoss8Piller.cpp
src/pso/boss/olgaflow/Boss6PlayerSnapshots.cpp
src/pso/boss/olgaflow/Boss6Type2Data.cpp
src/pso/boss/olgaflow/TBoss6CrushRock.cpp
src/pso/boss/olgaflow/TBoss6DPB.cpp
src/pso/boss/olgaflow/TBoss6EffFlowSoul.cpp
src/pso/boss/olgaflow/TBoss6EffHyoui.cpp
src/pso/boss/olgaflow/TBoss6EmptyWarp.cpp
src/pso/boss/olgaflow/TBoss6MagActions.cpp
src/pso/boss/olgaflow/TBoss6PlotFalz_802B0228.cpp
src/pso/boss/olgaflow/TBoss6RockChase.cpp
src/pso/boss/olgaflow/TBoss6SwordShot.cpp
src/pso/boss/olgaflow/TBoss6Thanks.cpp
src/pso/boss/olgaflow/TBoss6Tube.cpp
src/pso/boss/olgaflow/TBoss6Type2Actions.cpp
src/pso/boss/olgaflow/TBoss6Type2_80356E50.cpp
src/pso/boss/olgaflow/TBoss6Zanzo.cpp
src/pso/boss/volopt/TBoss3VoloptCore.cpp
src/pso/boss/volopt/TBoss3VoloptCtrl.cpp
src/pso/boss/volopt/TBoss3VoloptHiraisin.cpp
src/pso/boss/volopt/TObjVoloptCage.cpp
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
src/pso/effect/particle/TTelepipeMarker.cpp
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
src/pso/enemy/TEnemyTaskList.cpp
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
src/pso/enemy/dd/TObjEneDD.cpp
src/pso/enemy/deldepth/DelDepthMotion.cpp
src/pso/enemy/dellbiter/TObjEneDellBiterEffects.cpp
src/pso/enemy/dubchik/TDubchikPart.cpp
src/pso/enemy/dubchik/TDubchikShot.cpp
src/pso/enemy/flower/FlowerRe2Data.cpp
src/pso/enemy/flower/TObjEneRe2Flower.cpp
src/pso/enemy/flower/TObjEneRe2FlowerStates.cpp
src/pso/enemy/fly/TObjEneBm3Fly.cpp
src/pso/enemy/fly/TObjEneBm3FlyNest.cpp
src/pso/enemy/gee/TObjEneMe1Gee.cpp
src/pso/enemy/gee/TObjEneMe1Gee_NEEDLE.cpp
src/pso/enemy/gigue/TObjEneMe1GiGue.cpp
src/pso/enemy/gigue/TObjEneMe1GiGueBarrier.cpp
src/pso/enemy/gigue/TObjEneMe1GiGueHomingShot.cpp
src/pso/enemy/gigue/TObjEneMe1GiGueMissile.cpp
src/pso/enemy/lappy/TObjEneLappy.cpp
src/pso/enemy/mericarol/TObjEneBm9Mericarol_EffCurse3.cpp
src/pso/enemy/merilllia/TObjEneMerillLia.cpp
src/pso/enemy/morfos/MorfosData.cpp
src/pso/enemy/morfos/TObjEneMorfosArm.cpp
src/pso/enemy/nanodrago/TObjNanoDragoTargetedBeam.cpp
src/pso/enemy/panarms/TObjEnePanBARA.cpp
src/pso/enemy/panarms/TObjEnePanarmsAtkColli.cpp
src/pso/enemy/shinowa/ShinowaData.cpp
src/pso/enemy/sinowzoa/SinowZoaData.cpp
src/pso/enemy/sinowzoa/TObjEnemyColorFade.cpp
src/pso/enemy/stelth/StelthData.cpp
src/pso/item/TItemBuff.cpp
src/pso/item/TItemMenuCtrl.cpp
src/pso/item/TItemMoney.cpp
src/pso/item/TItemReverseTechCtrl.cpp
src/pso/item/TItemTimer.cpp
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
src/pso/item/TItemToolTelepipe.cpp
src/pso/item/TItemToolTrapVision.cpp
src/pso/item/TUnk_804F112C.cpp
src/pso/item/armor/TItemProArmor.cpp
src/pso/item/armor/TItemProArmorParticle.cpp
src/pso/item/armor/TItemProShield.cpp
src/pso/item/armor/TItemProUnit.cpp
src/pso/item/drop/TItemDrop.cpp
src/pso/item/mag/TItemMag.cpp
src/pso/item/mag/TItemMagSub.cpp
src/pso/item/mag/TItemMag_8001A468.cpp
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
src/pso/map/area/AreaForest01.cpp
src/pso/map/area/AreaForest02.cpp
src/pso/map/area/AreaLobby.cpp
src/pso/map/area/AreaMachine01.cpp
src/pso/map/area/AreaMachine02.cpp
src/pso/map/area/AreaVersus01.cpp
src/pso/map/area/AreaVersus02.cpp
src/pso/math/CurvePathPoint.cpp
src/pso/math/Vec2Constants.cpp
src/pso/network/LobbyRuleSettings.cpp
src/pso/network/NetworkWaitIcon.cpp
src/pso/network/TGameProtocol.cpp
src/pso/network/TradeItemList.cpp
src/pso/network/TradeSession.cpp
src/pso/network/protocol/subcommand_validation_actions.cpp
src/pso/network/protocol/subcommand_validation_movement.cpp
src/pso/network/protocol/subcommand_validation_photon_blast.cpp
src/pso/network/protocol/subcommand_validation_player.cpp
src/pso/npc/NpcPartnerControl.cpp
src/pso/npc/TNpcPlayer_802C3CD0.cpp
src/pso/npc/TNpcTalkTrigger.cpp
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
src/pso/object/TObjHashi.cpp
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
src/pso/object/ambient/TObjGrass.cpp
src/pso/object/ambient/TObjLight.cpp
src/pso/object/ambient/TObjLight_80133568.cpp
src/pso/object/ambient/TObjMistCtrl.cpp
src/pso/object/ambient/TObjRain.cpp
src/pso/object/ambient/TObjRainCtrl.cpp
src/pso/object/base/ObjectFactoryTables.cpp
src/pso/object/base/TObjBase.cpp
src/pso/object/base/TObjMapObject.cpp
src/pso/object/cave/TOAirconCave02.cpp
src/pso/object/cave/TODragonflyCave01.cpp
src/pso/object/cave/TORainbowCave01.cpp
src/pso/object/city/CityDoors.cpp
src/pso/object/city/CitySeason.cpp
src/pso/object/city/CityVerticalModel.cpp
src/pso/object/collision/TObjBgmColCtrl.cpp
src/pso/object/collision/TObjCollision.cpp
src/pso/object/collision/TObjEvtCollision.cpp
src/pso/object/collision/TObjRaderCol.cpp
src/pso/object/container/TContainerAncient01R.cpp
src/pso/object/container/TObjContainerIdo.cpp
src/pso/object/container/TObjContainerIdo_8018BEE4.cpp
src/pso/object/container/TObjContainerIdo_801A0B94.cpp
src/pso/object/container/TObjContainerItem.cpp
src/pso/object/container/TObjContainerJung.cpp
src/pso/object/door/GreenDoorResource.cpp
src/pso/object/door/TODoorBase.cpp
src/pso/object/door/TObjDoor_801F3754.cpp
src/pso/object/ending/TObjOnlineEnding.cpp
src/pso/object/ending/TObjOnlineEndingHexLine.cpp
src/pso/object/jungle/TOGrassJungle.cpp
src/pso/object/jungle/TOJungleDesign.cpp
src/pso/object/jungle/TORockJungleL01.cpp
src/pso/object/key/TOKey.cpp
src/pso/object/key/TOKeyCol.cpp
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
src/pso/object/switch/TOSensor.cpp
src/pso/object/switch/TOSwitchAttackSub.cpp
src/pso/object/switch/TOSwitchItem.cpp
src/pso/object/trap/TOExplosiveMachine.cpp
src/pso/object/trap/TOMineIcon01.cpp
src/pso/object/trap/TOMineIcon02.cpp
src/pso/object/trap/TOMineIcon03.cpp
src/pso/object/trap/TOMineIcon04.cpp
src/pso/object/trap/TOTrap.cpp
src/pso/object/trap/TOTrapChainSawEdge.cpp
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
src/pso/player/TObjPlayerDemoMode.cpp
src/pso/player/TObjPlayerSet.cpp
src/pso/player/TObjPlayerTechniqueLevels.cpp
src/pso/player/TObjPlayer_801D6BC8.cpp
src/pso/player/TObjPlayer_801D6C4C.cpp
src/pso/player/TObjPlayer_801D7C5C.cpp
src/pso/player/TObjPlayer_801D84A8.cpp
src/pso/player/TObjPlayer_801D95B8.cpp
src/pso/player/TObjPlayer_802A1440.cpp
src/pso/player/TObjPlayer_802C367C.cpp
src/pso/player/TObjPlayer_802C3B14.cpp
src/pso/player/TPlyCharData.cpp
src/pso/player/TPlyDispConfigSmth.cpp
src/pso/player/TPlyPhotonChair.cpp
src/pso/player/WeaponSpecialEffects.cpp
src/pso/quest/QuestDownloadRequests.cpp
src/pso/quest/QuestOpcodeHandlersData.cpp
src/pso/quest/QuestStoryFlags.cpp
src/pso/quest/TQuestListChallenge.cpp
src/pso/quest/TQuestListVM.cpp
src/pso/quest/TQuestListVM_8007EAF8.cpp
src/pso/quest/TQuestVmDownLoadServer_801F9010.cpp
src/pso/quest/challenge/ChallengeModeQuest.cpp
src/pso/render/BlobShadow.cpp
src/pso/render/ChunkMaterialState.cpp
src/pso/render/GinjaModelQueue.cpp
src/pso/render/GinjaPerspective.cpp
src/pso/render/GinjaTextureEntryCopy.cpp
src/pso/render/GinjaUvScroll.cpp
src/pso/render/GinjaView.cpp
src/pso/render/MaterialColorFade.cpp
src/pso/render/MaterialShadeFade.cpp
src/pso/render/MotionDrawState.cpp
src/pso/render/RenderFadeState.cpp
src/pso/render/RenderSbssGlobals.cpp
src/pso/render/ScreenPrimitiveDraw.cpp
src/pso/render/ScreenSpaceMatrix.cpp
src/pso/render/TDLGraph.cpp
src/pso/render/TFogCtrl.cpp
src/pso/render/render_util.cpp
src/pso/render/render_util_head.cpp
src/pso/system/ControllerInput.cpp
src/pso/system/PadRumble.cpp
src/pso/system/RlcArchive.cpp
src/pso/system/SwitcherLaunch.cpp
src/pso/system/TLoadCtrl.cpp
src/pso/system/TScreenFadeController.cpp
src/pso/system/data/pso_date.cpp
src/pso/system/save/TCheckMemoryCard.cpp
src/pso/system/scene/OfflineGameScene.cpp
src/pso/system/scene/TEnding.cpp
src/pso/system/scene/TEndingPic.cpp
src/pso/system/scene/TEndingPicStage.cpp
src/pso/system/scene/TScene.cpp
src/pso/system/scene/TSceneSora.cpp
src/pso/system/task/TObject.cpp
src/pso/technique/TTecCircle.cpp
src/pso/technique/TTecFireCharge.cpp
src/pso/technique/TTecFoie.cpp
src/pso/technique/TTecGiBarta.cpp
src/pso/technique/TTecGiFoie.cpp
src/pso/technique/TTecIceCharge.cpp
src/pso/technique/TTecJelin.cpp
src/pso/technique/TTecKafun.cpp
src/pso/technique/TTecLightCharge.cpp
src/pso/technique/TTecLuker.cpp
src/pso/technique/TTecMegid.cpp
src/pso/technique/TTecMegidCharge.cpp
src/pso/technique/TTecRaZonde.cpp
src/pso/technique/TTecResta.cpp
src/pso/technique/TTecShot.cpp
src/pso/technique/TTecSifta.cpp
src/pso/technique/TTecSlicer.cpp
src/pso/technique/TTecSupportCharge.cpp
src/pso/technique/TTecThunderCharge.cpp
src/pso/technique/TTecZarua.cpp
src/pso/technique/TechniqueRingwaveNames.cpp
src/pso/ui/TLevelSelect.cpp
src/pso/ui/TLogo.cpp
src/pso/ui/TTitleMenu.cpp
src/pso/ui/TitleTextures.cpp
src/pso/ui/charcreate/CharacterPreview.cpp
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
src/pso/ui/chat/TSymbolChatMenu.cpp
src/pso/ui/chat/TWindowSubSymbolChatFaceType.cpp
src/pso/ui/chat/TWindowSubSymbolChatParts.cpp
src/pso/ui/city/THunterGuildSeqOffline_80211FEC.cpp
src/pso/ui/city/THunterGuildSeqOnline.cpp
src/pso/ui/city/THunterGuildSeqOnlineTask.cpp
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
src/pso/ui/city/TShopItemTecSequence.cpp
src/pso/ui/city/TShopJudgmentItemWindow.cpp
src/pso/ui/city/TShopKeepItemWindow.cpp
src/pso/ui/city/TShopWindowBase.cpp
src/pso/ui/font/TFontSmallHelp.cpp
src/pso/ui/frontend/DrawLine.cpp
src/pso/ui/frontend/EndingSequence.cpp
src/pso/ui/frontend/TAdADXlogo.cpp
src/pso/ui/frontend/TAdAgreementBaseGC.cpp
src/pso/ui/frontend/TAdChrSelASlotGC.cpp
src/pso/ui/frontend/TAdChrSelBSlotGC.cpp
src/pso/ui/frontend/TAdDialingGC_801A4C90.cpp
src/pso/ui/frontend/TAdDialingMessageWindow.cpp
src/pso/ui/frontend/TAdEnterSerialAccessGC_802ACF04.cpp
src/pso/ui/frontend/TAdEp3Exist.cpp
src/pso/ui/frontend/TAdFloorLoadGC.cpp
src/pso/ui/frontend/TAdFloorLoadPrtGC.cpp
src/pso/ui/frontend/TAdGameStartLineGC.cpp
src/pso/ui/frontend/TAdGuideTxtGC.cpp
src/pso/ui/frontend/TAdLdQuestPrtGC.cpp
src/pso/ui/frontend/TAdLicAgreementGC.cpp
src/pso/ui/frontend/TAdLoadNetworkGC.cpp
src/pso/ui/frontend/TAdLoadQuestCellGC.cpp
src/pso/ui/frontend/TAdLoadQuestGC.cpp
src/pso/ui/frontend/TAdLoadQuestLineGC.cpp
src/pso/ui/frontend/TAdLoadQuestPrtTxtGC.cpp
src/pso/ui/frontend/TAdMcCheckGC.cpp
src/pso/ui/frontend/TAdMultiChgGameModeGC.cpp
src/pso/ui/frontend/TAdMultiChgGameModeGC_8023E7A8.cpp
src/pso/ui/frontend/TAdMultiConfirmGC.cpp
src/pso/ui/frontend/TAdMultiPlyChrSelectGC_802E0064.cpp
src/pso/ui/frontend/TAdMultiPlyChrSelectGC_802E0110.cpp
src/pso/ui/frontend/TAdMultiSaveGC.cpp
src/pso/ui/frontend/TAdPlayerLoadGC.cpp
src/pso/ui/frontend/TAdPlayerLoadGC_801E0C30.cpp
src/pso/ui/frontend/TAdPlyChrSelPlateGC.cpp
src/pso/ui/frontend/TAdPsoPassSaveGC.cpp
src/pso/ui/frontend/TAdRicoRingCtrl.cpp
src/pso/ui/frontend/TAdScrollWinGC.cpp
src/pso/ui/frontend/TAdSelectGameModeGC_802C4748.cpp
src/pso/ui/frontend/TAdSinglePlyChrSelectGC.cpp
src/pso/ui/frontend/TAdSinglePlyChrSelectGC_801334D4.cpp
src/pso/ui/frontend/TAdSinglePlyChrSelectGC_80243218.cpp
src/pso/ui/frontend/TAdSnAkPassConfirmGC.cpp
src/pso/ui/frontend/TAdSoniclogo.cpp
src/pso/ui/frontend/TAdStorySelectGC_802C3284.cpp
src/pso/ui/frontend/TAdSubPlyItemGC.cpp
src/pso/ui/frontend/TAdVIConfigGC.cpp
src/pso/ui/frontend/TAdVIConfigGC_80308200.cpp
src/pso/ui/frontend/TitleBackground.cpp
src/pso/ui/hud/TMainCursor_80154090.cpp
src/pso/ui/hud/TNowLoading.cpp
src/pso/ui/hud/TRaderMapCore_801F3710.cpp
src/pso/ui/hud/TRaderMapCore_8030A808.cpp
src/pso/ui/keyboard/KeyboardControlTask.cpp
src/pso/ui/keyboard/TKeyBoardControlLayout.cpp
src/pso/ui/keyboard/TKeyBoardControl_80290CB0.cpp
src/pso/ui/keyboard/WindowKeyBoardMode.cpp
src/pso/ui/window/HeadingEntry.cpp
src/pso/ui/window/HelpItemRarityStarRow.cpp
src/pso/ui/window/HelpItemStatCompareRow.cpp
src/pso/ui/window/LabelValueRow.cpp
src/pso/ui/window/TGameListRow.cpp
src/pso/ui/window/TMessageWindowController.cpp
src/pso/ui/window/TObjBagWarnWin.cpp
src/pso/ui/window/TObjDeadListWin.cpp
src/pso/ui/window/TObjNewFloorWin.cpp
src/pso/ui/window/TObjSoloPlayWin.cpp
src/pso/ui/window/TWindowAddition.cpp
src/pso/ui/window/TWindowArea.cpp
src/pso/ui/window/TWindowBase.cpp
src/pso/ui/window/TWindowBreakIn.cpp
src/pso/ui/window/TWindowChallengeResult.cpp
src/pso/ui/window/TWindowCheckBox.cpp
src/pso/ui/window/TWindowChoiceSetting.cpp
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
src/pso/ui/window/TWindowInputNumber.cpp
src/pso/ui/window/TWindowKeyBoardBase_8008102C.cpp
src/pso/ui/window/TWindowKeyBoardBase_8025B85C.cpp
src/pso/ui/window/TWindowKeyBoardSoft.cpp
src/pso/ui/window/TWindowKeyBoardSoft_80228BEC.cpp
src/pso/ui/window/TWindowLayoutCountingNumber.cpp
src/pso/ui/window/TWindowLayoutNumber.cpp
src/pso/ui/window/TWindowLoadItemTexture.cpp
src/pso/ui/window/TWindowLockOn_800F5DD8.cpp
src/pso/ui/window/TWindowLockOn_8026CB80.cpp
src/pso/ui/window/TWindowMainMenu1P.cpp
src/pso/ui/window/TWindowMainMenu4P.cpp
src/pso/ui/window/TWindowMakeTeam.cpp
src/pso/ui/window/TWindowMenu.cpp
src/pso/ui/window/TWindowMenuIconLine.cpp
src/pso/ui/window/TWindowMenuMainMenu.cpp
src/pso/ui/window/TWindowMeseta.cpp
src/pso/ui/window/TWindowMessage.cpp
src/pso/ui/window/TWindowMessageLog.cpp
src/pso/ui/window/TWindowMessageQuest.cpp
src/pso/ui/window/TWindowMessageScroll.cpp
src/pso/ui/window/TWindowOptionCtrl_8026CBA4.cpp
src/pso/ui/window/TWindowPartyStatus.cpp
src/pso/ui/window/TWindowReturnCity.cpp
src/pso/ui/window/TWindowRoom.cpp
src/pso/ui/window/TWindowSetLevelUp.cpp
src/pso/ui/window/TWindowShop.cpp
src/pso/ui/window/TWindowShortCut.cpp
src/pso/ui/window/TWindowSimpleStatus.cpp
src/pso/ui/window/TWindowSoundBalance.cpp
src/pso/ui/window/TWindowStatus.cpp
src/pso/ui/window/TWindowStatusGauge.cpp
src/pso/ui/window/TWindowStatusGaugeFeedback.cpp
src/pso/ui/window/TWindowSubAreaMap.cpp
src/pso/ui/window/TWindowSubAreaMapDisp.cpp
src/pso/ui/window/TWindowSubComCard_80225E0C.cpp
src/pso/ui/window/TWindowSubComCard_802495FC.cpp
src/pso/ui/window/TWindowSubCustomSelect.cpp
src/pso/ui/window/TWindowSubEquipKind.cpp
src/pso/ui/window/TWindowSubEquip_80279044.cpp
src/pso/ui/window/TWindowSubGameEnd.cpp
src/pso/ui/window/TWindowSubInformationBoard.cpp
src/pso/ui/window/TWindowSubItemList.cpp
src/pso/ui/window/TWindowSubMagPhoton.cpp
src/pso/ui/window/TWindowSubQuestBoardSelect.cpp
src/pso/ui/window/TWindowSubShortCut.cpp
src/pso/ui/window/TWindowSubShortCutList.cpp
src/pso/ui/window/TWindowSubShortCut_80246FB0.cpp
src/pso/ui/window/TWindowSubStatusDisp_80228C30.cpp
src/pso/ui/window/TWindowSubStatusDisp_80228CC0.cpp
src/pso/ui/window/TWindowSubStatusDisp_8024FBA8.cpp
src/pso/ui/window/TWindowSubStatusDisp_8027FD88.cpp
src/pso/ui/window/TWindowSubStatusDisp_803DEE6C.cpp
src/pso/ui/window/TWindowSubTrade.cpp
src/pso/ui/window/TWindowSubTradeMain_80187C98.cpp
src/pso/ui/window/TWindowSubTradeMessage.cpp
src/pso/ui/window/TWindowTradeRequest.cpp
src/pso/ui/window/TWindowUserSort.cpp
src/pso/ui/window/TWindowWordSelectMode.cpp
src/pso/ui/window/WindowDrawContext.cpp
src/pso/ui/window/WindowLayoutCamera2Data.cpp
src/pso/ui/window/WindowScrollBar.cpp
src/pso/ui/window/WindowSpritePart.cpp
src/pso/ui/window/WindowTextEntry.cpp
src/pso/unk/TBattleProximitySound.cpp
src/pso/unk/TLobbyEventTheme.cpp
src/pso/unk/TLobby_8002C140.cpp
src/pso/unk/TLobby_8004A90C.cpp
src/pso/unk/TLobby_8007B858.cpp
src/pso/unk/TLobby_800F795C.cpp
src/pso/unk/TLobby_800FD78C.cpp
src/pso/unk/TLobby_80103F5C.cpp
src/pso/unk/TLobby_80140FF4.cpp
src/pso/unk/TLobby_801D6BF8.cpp
src/pso/unk/TLobby_801E2F94.cpp
src/pso/unk/TLobby_8020E400.cpp
src/pso/unk/TLobby_80228154.cpp
src/pso/unk/TLobby_80228C84.cpp
src/pso/unk/TLobby_8028EA28.cpp
src/pso/unk/TLobby_80299A60.cpp
src/pso/unk/TLobby_80299B90.cpp
src/pso/unk/TLobby_802D3970.cpp
src/pso/unk/TLobby_802D79EC.cpp
src/pso/unk/TLobby_80300098.cpp
src/pso/unk/TLobby_8030A6A8.cpp
src/pso/unk/TLobby_8030A834.cpp
src/pso/unk/TObjFreezeShell.cpp
src/pso/unk/TWindowStatusCategoryMenu.cpp
src/pso/unk/text_80071B30.cpp
src/pso/unk/text_800C7FEC.cpp
src/pso/unk/text_801D9870.cpp
src/pso/unk/text_802233F8.cpp
src/pso/unk/text_8023DCB8.cpp
src/pso/unk/text_8025CD10.cpp
src/pso/unk/text_8026934C.cpp
src/pso/unk/text_8026A14C.cpp
src/pso/unk/text_8026C664.cpp
src/pso/unk/text_802A1020.cpp
src/pso/unk/text_802C3AC0.cpp
src/pso/unk/text_802F0F04.cpp
src/pso/unk/text_8035B6B0.cpp
src/pso/unk/text_8035B750.cpp
src/pso/unk/text_803D64F8.cpp
src/pso/unk/text_803D8DE0.cpp
src/pso/unk/text_803D8F74.cpp
src/pso/unk/text_803E2D70.cpp
src/pso/weapon/TItemAttacker.cpp
src/pso/weapon/TItemWeaponAgito.cpp
src/pso/weapon/TItemWeaponAngelHarp.cpp
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
src/pso/weapon/TItemWeaponNug2000Bazooka.cpp
src/pso/weapon/TItemWeaponPArmsBlade.cpp
src/pso/weapon/TItemWeaponPanthersClaw.cpp
src/pso/weapon/TItemWeaponPartisan.cpp
src/pso/weapon/TItemWeaponPartisanOfLightning.cpp
src/pso/weapon/TItemWeaponPhotonLauncher.cpp
src/pso/weapon/TItemWeaponPlantainFan.cpp
src/pso/weapon/TItemWeaponProphetsOfMotav.cpp
src/pso/weapon/TItemWeaponPsychoWand.cpp
src/pso/weapon/TItemWeaponRabbitWand.cpp
src/pso/weapon/TItemWeaponRainbowBaton.cpp
src/pso/weapon/TItemWeaponRappysFan.cpp
src/pso/weapon/TItemWeaponRifle.cpp
src/pso/weapon/TItemWeaponRod.cpp
src/pso/weapon/TItemWeaponRubyBullet.cpp
src/pso/weapon/TItemWeaponSBeatsBlade.cpp
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
src/pso/weapon/TItemWeaponWokOfAkikosShop.cpp
src/pso/weapon/TItemWeaponYamigarasu.cpp
src/pso/weapon/WeaponSpecialSelection.cpp
```

Measured corpus delta if applied (this lane's own sweeps, 495cc91a era, apply-6 shape):
- official LP64: 350 PASS → 1158 PASS (**+808 flips**, 350+808=1158 of 1,680 = 68.9%)
- breakage: **0** (BASE-clean→NJS-fail tree-wide = 0, §1; block rows all carry 0-byte NJS err receipts)
- NC §5 gate carry-overs: FloorPopulation assert citations 0 (10 FP flipped rows are inside
  this block); collateral ledger unchanged {GcrnaStream.cpp}; .cpp-only suffix guard holds.
- RUNTIME-GATED (260) and ADAPTER-QUEUE (169) rows are NOT in the block — adding them would
  break the fail corpus (queue) or bank silent runtime risk (gated).

## Repro

`cache/scratch/na/`: na_probe.sh (BASE/NJS shapes), BASE/+NJS/ sweeps over b3/all.txt,
lists.json / gated_final.json / queue.json / sets.json (python set-math). Repo writes:
this doc ONLY (git show --stat at commit). No build/, host/, data/, compat/ changes;
`grep -rl compat/njs build/` unchanged (5 live rows + Makefile case-guard, pre-existing).
