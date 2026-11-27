# P2-T — Non-layout dialect/blocker triage (residual queue after P2-R)

Phase-2 task T. **All numbers re-measured 2026-11-26** from a FULL-UNCAPPED LP64 re-probe
(`-ferror-limit=0`, same flag set as P2-R / `probes/tree_compile_probe.sh`, compat + waiver
headers force-included) over the 1,610 P2-R `lp64native` failures at
`pso-decomp @ 6b38b5383` — no `make`, decomp untouched. Sweep:
`cache/scratch/clangprobe/run_p2t_uncapped.sh`; per-file classifier `p2t_classify.py`;
rows `p2t_rows.json`. Flip test: `compat/dialect/flip_test.sh`.

**Correction to the tranche-1 doc:** the residual non-layout-blocked set is **113 files,
not 66**. P2-R counted first-visible errors under clang's default 20-error cap; uncapped,
47 files previously bucketed as assert-only revealed additional non-layout blockers.
(The task brief's 136 predates this sweep; 113 is the exact uncapped measurement, and the
cap-hiding direction is now fully enumerated, not a floor.) Every ILP32-visible class
remains; composition below.

## 1. Bucket table (first blocking non-layout error per file, 113 files)

| Bucket (class) | Files | Dominant idioms (first-error attribution) |
|---|---:|---|
| `SDK-dialect-header` | **65** | 39 × MWCC address-pin (`AT_ADDRESS` macro or raw `: 0xADDR`) in dolphin/musyx vendor headers; 26 × bare `extern "C"` at `include/musyx/musyx.h:161` in C TUs |
| `mussyx-tag` (struct-tag collision) | **0 first / 26 secondary** | always hides behind `musyx.h:161` (same TUs): `musyx_internal.h:13` `SoundReverb` etc. — one bundle with the vendor-header fix |
| `intrinsic-family` | **11** | ~13 distinct missing families, see §3 (MWCC C++-ABI stubs, `__builtin_va_info`, `__dcbt` arity, `SI*PollingHandler` arity, `acos/asin/atan2/fmaf` decls) |
| `include-graph` | **0 first / 13 secondary** | every missing header is `orig_asm/**/*.inc` (untracked PPC asm include bodies) inside asm-block TUs |
| `other` | **37** | 16 × waiver-harness tripwire (`NET_DISPATCH_OFFSET` ##-generated assert names have no `PSO_LA_FLAG_*`); 6 × `AT_ADDRESS` inside **decomp's own** headers (`QuestDownloadServer.h:134` — same fix as SDK class); 7 × MWCC `asm`-function definition blocks; 7 × signature/language-linkage drift; 1 × address-of-temporary |

Sum by first blocker: 65 + 11 + 37 = 113. `other`-bucket AT_ADDRESS (6) makes the true
address-pin class **45 files**.

<details><summary>Exact per-file table (113 rows, generated from p2t_rows.json)</summary>

| TU (decomp-relative) | first blocking error:line | msg (trunc) | class | sub-idiom |
|---|---|---|---|---|
| `src/musyx/runtime/Chorus/chorus_fx.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/StdReverb/reverb.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/StdReverb/reverb_fx.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/hardware.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/hw_aramdma.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/hw_dolphin.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/hw_dspctrl.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/hw_memory.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/hw_volconv.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/s_data.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/seq.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/seq_api.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/snd3d.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/snd_init.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/snd_math.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/snd_midictrl.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/snd_service.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/snd_synthapi.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/stream.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/synth.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/synth_ac.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/synth_adsr.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/synth_vsamples.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/synthdata.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/synthmacros.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/musyx/runtime/synthvoice.c` | `include/musyx/musyx.h:161` | expected identifier or '(' | SDK-dialect-header | musyx.h bare extern "C" (vendor header) |
| `src/pso/network/NetworkInfoLoader.cpp` | `libs/dolphin/include/dolphin/dvd.h:196` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/system/save/McFsGcRequests.cpp` | `libs/dolphin/include/dolphin/dvd.h:196` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/ui/frontend/TAdPlayerLoadGC_801E0C30.cpp` | `libs/dolphin/include/dolphin/dvd.h:196` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/ui/frontend/TAdSinglePlyChrSelectGC_801FBF54.cpp` | `libs/dolphin/include/dolphin/dvd.h:196` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/unk/TLobby_800F8AA8.cpp` | `libs/dolphin/include/dolphin/dvd.h:196` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/unk/text_803DD5E0.cpp` | `libs/dolphin/include/dolphin/dvd.h:196` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/effect/particle/ParticleStrip.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/effect/particle/TParticle.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/effect/particle/TTelepipeMarker.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/DebugFont.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/GinjaDrawUtil.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/GinjaMaterial.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/GinjaModelQueue.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/SceneFog.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/ScreenPrimitiveDraw.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/ScreenSpaceMatrix.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/ShapeMotionDraw.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/TexturedVertexDraw.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/reflection.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/render_util.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/render_util_head.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/system/TrailStrip.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/ui/frontend/TAdMcCheckGC.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/ui/window/TWindowSubStatusDisp_803DEE6C.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/unk/TLobby_801E2F94.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/unk/text_803E2D70.cpp` | `libs/dolphin/include/dolphin/gx/GXVert.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/network/eth.c` | `libs/dolphin/include/dolphin/hw_regs.h:4` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/ui/keyboard/KBDXfer.c` | `libs/dolphin/include/dolphin/hw_regs.h:4` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/network/NetworkConnect.cpp` | `libs/dolphin/include/dolphin/os/OSExec.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/network/mdm.c` | `libs/dolphin/include/dolphin/os/OSExec.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/ChunkModelDraw.cpp` | `libs/dolphin/include/dolphin/os/OSExec.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/gjLight.c` | `libs/dolphin/include/dolphin/os/OSExec.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/gjSystem.c` | `libs/dolphin/include/dolphin/os/OSExec.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/render/nuReLoadTexture.c` | `libs/dolphin/include/dolphin/os/OSExec.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/system/movie/Gcci.cpp` | `libs/dolphin/include/dolphin/os/OSExec.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/system/save/McFsGc.cpp` | `libs/dolphin/include/dolphin/os/OSExec.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/ui/keyboard/SystemBootInit.cpp` | `libs/dolphin/include/dolphin/os/OSExec.h:25` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/pso/system/RelModule.cpp` | `libs/dolphin/include/dolphin/os/OSLink.h:27` | expected ';' after top level declarator | SDK-dialect-header | AT_ADDRESS / ':' address-pin |
| `src/musyx/runtime/dsp_import.c` | `src/musyx/runtime/dsp_import.c:4` | expected identifier or '(' | SDK-dialect-header | other: expected identifier or '(' |
| `src/pso/audio/CriSvm.cpp` | `src/pso/audio/CriSvm.cpp:190` | use of undeclared identifier '__builtin_va_info' | intrinsic-family | intrinsic/libc decl missing |
| `src/pso/audio/LscLog.cpp` | `src/pso/audio/LscLog.cpp:16` | use of undeclared identifier '__builtin_va_info' | intrinsic-family | intrinsic/libc decl missing |
| `src/pso/boss/derolle/TBoss2Fin.cpp` | `src/pso/boss/derolle/TBoss2Fin.cpp:133` | use of undeclared identifier 'acos' | intrinsic-family | intrinsic/libc decl missing |
| `src/pso/boss/olgaflow/TBoss6Mine.cpp` | `src/pso/boss/olgaflow/TBoss6Mine.cpp:397` | use of undeclared identifier 'asin' | intrinsic-family | intrinsic/libc decl missing |
| `src/pso/network/TLobby.cpp` | `src/pso/network/TLobby.cpp:2646` | no matching function for call to '__construct_array' | intrinsic-family | intrinsic/ABI decl missing |
| `src/pso/npc/TNpcPlayer.cpp` | `src/pso/npc/TNpcPlayer.cpp:1016` | no matching function for call to '__register_global_object' | intrinsic-family | intrinsic/ABI decl missing |
| `src/pso/object/base/TObjChara.cpp` | `src/pso/object/base/TObjChara.cpp:226` | use of undeclared identifier 'atan2' | intrinsic-family | intrinsic/libc decl missing |
| `src/pso/player/TPlayerBattleCtrl.cpp` | `src/pso/player/TPlayerBattleCtrl.cpp:775` | no matching function for call to '__destroy_new_array' | intrinsic-family | intrinsic/ABI decl missing |
| `src/pso/system/SiPolling.cpp` | `src/pso/system/SiPolling.cpp:29` | no matching function for call to 'SIRegisterPollingHandler' | intrinsic-family | intrinsic/ABI decl missing |
| `src/pso/system/movie/MovieFramePack.cpp` | `src/pso/system/movie/MovieFramePack.cpp:62` | no matching function for call to '__dcbt' | intrinsic-family | intrinsic/ABI decl missing |
| `src/pso/unk/text_801D34E0.cpp` | `src/pso/unk/text_801D34E0.cpp:9` | no matching function for call to '__register_global_object' | intrinsic-family | intrinsic/ABI decl missing |
| `src/pso/network/NetAddrFamily.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetApi.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetArp.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetDhcp.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetDispatcher.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetDns.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetFragment.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetHandlerTable.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetIPv6NeighborDiscovery.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetIcmp.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetIpv4.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetIpv4Route.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetIpv6.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetIpv6Route.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/NetTcp.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/network/PppFraming.c` | `include/pso/network/NetworkDispatch.h:65` | expected identifier | other | waiver-harness gap (##-generated names) |
| `src/pso/quest/AskDicConvert.cpp` | `include/pso/quest/QuestDownloadServer.h:134` | expected ';' after top level declarator | other | AT_ADDRESS / ':' address-pin |
| `src/pso/quest/AskDicFile.cpp` | `include/pso/quest/QuestDownloadServer.h:134` | expected ';' after top level declarator | other | AT_ADDRESS / ':' address-pin |
| `src/pso/quest/AskDicLookup.cpp` | `include/pso/quest/QuestDownloadServer.h:134` | expected ';' after top level declarator | other | AT_ADDRESS / ':' address-pin |
| `src/pso/quest/QuestDownloadAramFile.cpp` | `include/pso/quest/QuestDownloadServer.h:134` | expected ';' after top level declarator | other | AT_ADDRESS / ':' address-pin |
| `src/pso/quest/QuestDownloadNameCache.cpp` | `include/pso/quest/QuestDownloadServer.h:134` | expected ';' after top level declarator | other | AT_ADDRESS / ':' address-pin |
| `src/pso/quest/QuestDownloadRequests.cpp` | `include/pso/quest/QuestDownloadServer.h:134` | expected ';' after top level declarator | other | AT_ADDRESS / ':' address-pin |
| `src/pso/animation/MotionContext.cpp` | `src/pso/animation/MotionContext.cpp:388` | expected '(' after 'asm' | other | MWCC asm-function block |
| `src/pso/math/MatrixTransforms.cpp` | `src/pso/math/MatrixTransforms.cpp:45` | expected '(' after 'asm' | other | MWCC asm-function block |
| `src/pso/math/MatrixVector.cpp` | `src/pso/math/MatrixVector.cpp:18` | expected '(' after 'asm' | other | MWCC asm-function block |
| `src/pso/math/SinCosTable.cpp` | `src/pso/math/SinCosTable.cpp:16` | expected '(' after 'asm' | other | MWCC asm-function block |
| `src/pso/network/NetLoopback.c` | `src/pso/network/NetLoopback.c:99` | conflicting types for 'forward_loopback_packet' | other | signature/linkage drift |
| `src/pso/network/PppBuffer.c` | `src/pso/network/PppBuffer.c:27` | conflicting types for 'NetMemcpy' | other | signature/linkage drift |
| `src/pso/network/PppFsm.c` | `src/pso/network/PppFsm.c:536` | conflicting types for 'write_network_u16_be' | other | signature/linkage drift |
| `src/pso/quest/QuestOpcodeHandlers.cpp` | `src/pso/quest/QuestOpcodeHandlers.cpp:484` | conflicting types for 'read_byte_advance__6TLobbyFv' | other | signature/linkage drift |
| `src/pso/ui/charcreate/TChCrEffFade.cpp` | `src/pso/ui/charcreate/TChCrEffFade.cpp:19` | declaration of 'draw_hexagon' has a different language linka | other | signature/linkage drift |
| `src/pso/ui/city/THunterGuildSeqOffline.cpp` | `src/pso/ui/city/THunterGuildSeqOffline.cpp:333` | declaration of 'reset_quest_result_callbacks_on_unload' has  | other | signature/linkage drift |
| `src/pso/ui/hud/TLoadingCur.cpp` | `src/pso/ui/hud/TLoadingCur.cpp:156` | taking the address of a temporary object of type 'Vec' [-Wad | other | C++ semantic error |
| `src/pso/ui/window/TWindowSubTechnique.cpp` | `src/pso/ui/window/TWindowSubTechnique.cpp:109` | conflicting types for 'add_gray_text_entry' | other | signature/linkage drift |
| `src/pso/unk/text_803D9650.cpp` | `src/pso/unk/text_803D9650.cpp:6` | expected '(' after 'asm' | other | MWCC asm-function block |
| `src/pso/unk/text_803D96A8.cpp` | `src/pso/unk/text_803D96A8.cpp:66` | expected '(' after 'asm' | other | MWCC asm-function block |
| `src/pso/unk/text_8042E8C4.cpp` | `src/pso/unk/text_8042E8C4.cpp:27` | expected '(' after 'asm' | other | MWCC asm-function block |

</details>

## 2. Proposed port-side fix per class (precedent = `compat/pso_metal_compat.h`)

| Class | Fix | Precedent / mechanism |
|---|---|---|
| AT_ADDRESS address-pin (45: SDK 38 + quest-header 6 + hw_regs raw 2-ish) | `#include` both `types.h` variants in a force-include shim, then `#undef AT_ADDRESS` → empty. Guard-preemption = the exact trick `build/waivers/pso_assert_waivers.h:10-13` uses for `LAYOUT_ASSERT`; "a port defines it empty" is already compat vocabulary (`ATTRIBUTE_ALIGN_PARAM` note). Raw `: 0xADDR` pins (hw_regs.h, no macro) → shadow header `compat/dialect/pshim/dolphin/hw_regs.h` rewrites 8 pins to `extern` decls; host lane Q stubs own placement. | PROTOTYPED + measured |
| musyx vendor-header `extern "C"` + struct tags (26) | shadow `compat/dialect/pshim/musyx/musyx.h` (2-line `#ifdef __cplusplus` wrap) + `typedef struct SoundReverb SoundReverb;` in the dialect shim (compat macro-shim precedent). Residual source-side `extern "C"` blocks inside the 24 musyx `.c` files are C++-only syntax: host build compiles musyx runtime TUs in C++ mode (clang `-x c++`) — parse-verified 24/24 PASS; link-name reconciliation is a flagged follow-up (musyx is internally-referenced; nothing flipped that pso C++ links against today). | PROTOTYPED + measured |
| intrinsic-family (11) | extend compat: `static inline` stubs / macros for `__register_global_object`, `__construct_array`, `__destroy_arr`, `__destroy_new_array`, `__dcbt` (2-arg overload — compat already has the 1-arg), `__builtin_va_info` (MWCC varargs ABI — needs real shim), `SIRegisterPollingHandler`/`SIUnregisterPollingHandler` (prototype-arity drift), `acos/asin/atan2/fmaf` (freestanding `-ffreestanding` drops libc math decls: add `#include <math.h>` in dialect shim). | sized, not prototyped |
| waiver-harness gap (16) | `build/gen_waivers.py`: enumerate wrapper-macro-generated assert names (`NET_DISPATCH_OFFSET` et al.) when emitting `PSO_LA_FLAG_*` — the fail-closed tripwire is working as designed, the generator just never saw ##-concatenated names. Then normal verdicts apply. | sized, not prototyped |
| PPC asm blocks + orig_asm includes (7 first + 13 secondary ≈ 14 unique TUs) | no header-side path: bodies are untracked PPC asm (`orig_asm/**/*.inc`). Port decision per fn: rewrite as intrinsics/host code, or exclude TU from the LP64 gate. | sized, not prototyped |
| signature/linkage drift (7) + address-of-temp (1) | per-file header↔impl reconciliation (decomp-side fix, small, mechanical once verdict'd). | sized |

## 3. Prototype result — compat/dialect/ (top bucket)

Files: `pso_dialect_compat.h` (AT_ADDRESS neutralize + SoundReverb typedef + ordering
hardening: self-includes `../pso_metal_compat.h` so `dolphin/types.h → libc/math.h`
always sees the `__frsqrte`/`__fabsf` shims), `pshim/musyx/musyx.h`, `pshim/dolphin/hw_regs.h`,
`flip_test.sh`, `README.md`. Probe flags only (`clang -fsyntax-only`), no make, no decomp writes.

**26/26 zero-assert dialect-class files flipped FAIL→PASS** (≥5 required; the full
flippable-now pool — remaining class files still carry layout asserts, assert lane, not
dialect). Exact clang lines for the two pure-header files (no mode change):
`TexturedVertexDraw.cpp`/`ParticleStrip.cpp` before: `libs/dolphin/include/dolphin/gx/GXVert.h:25:51: error: expected ';' after top level declarator` → after: exit 0.
Example musyx file `src/musyx/runtime/s_data.c` before: `include/musyx/musyx.h:161:8: error: expected identifier or '('` → after (shim + `-x c++`): exit 0.
All 26 rows: `cache/scratch/clangprobe/p2t_flips/results.txt`.
Side-effect check: files that PASS today must stay PASS — dialect shim adds no redefinitions beyond the guarded `AT_ADDRESS`; flip_test covers only class members. A full-corpus non-regression re-sweep (1680) with the shim is queued for the landing task.

## 4. Effort sizing for the queue

| Work item | Files | Size | Notes |
|---|---:|---|---|
| Fold `compat/dialect/` AT_ADDRESS+typedef into main compat (or permanent -include), non-regression re-sweep | 45 | **S** (≤half-day) | prototype done; review pin-drop semantics vs lane-Q linker placement |
| musyx host-build rule (C++ mode + shadow header) + link-name audit | 26 | **S→M** (1 day) | parse gate already green 24/24; link phase is the open question |
| gen_waivers wrapper-macro name enumeration + verdicts for those names | 16 | **S** (tool change ~2h + verdict pass) | fail-closed harness means zero silent risk |
| intrinsic-family shim batch (~13 named decls) | 11 | **S** (half-day) | `__builtin_va_info` + SI*Polling arity are the two nontrivial ones |
| signature/linkage drift + address-of-temp | 8 | **S–M** (1–2 days) | decomp-side per-file fixes, needs verdict |
| PPC asm-block / orig_asm .inc class | 14 | **M–L** (1+ wk, or gate-exclude) | **hardest class** — no compile-time shim exists |

Queue total for the 113: everything except the asm class is header/tooling/S–M work;
after it, LP64 parse is bounded only by the assert-waiver ladder (tranches 2/3).
