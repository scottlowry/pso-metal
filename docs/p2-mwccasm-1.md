# P2-MA — MWCC asm/decl dialect residual (collision-debt §4.5): survey + inert supply

Phase-3 lane MA, 2026-11-27 (card `t_f74fc627`). Decomp `pso-decomp @6b38b5383`
READ-ONLY; **zero make/ninja** — every number is scratch `clang -fsyntax-only
-ferror-limit=0` over the battery-3 corpus (`$TMP/b3/all.txt`, 1,680 units),
shapes = AZ's committed recipe (`$TMP/az_probe.sh` H3, cloned fresh as
`$TMP/ma/ma_probe.sh` M3, `-P6`). Inputs: `docs/p2-collision-debt.md` §4.5/§5,
`docs/p2-repin-saves-1.md` (AW `compat/repin` anatomy — this lane owns that dir
as successor), `compat/repin/**`, `compat/dialect/**` (P2-T precedent). Writes:
`compat/repin/**` + this doc only.

## 1. The class, measured (not "~38" — exactly 38)

Of AZ's 422 H3-failing TUs, 72 carry an `expected ...`-style syntax-dialect FIRST
error. Sub-classified by failing header and MWCC spelling (python pass over
`az/H3/err/`):

| spelling class | TUs | site(s) | fixable by `-I` shadow? |
|---|---|---|---|
| **`AT_ADDRESS` absolute-address var decl** — era `dolphin/types.h:43` `#define AT_ADDRESS(addr) : (addr)`, MWCC colon declarator; sites GXVert.h:25 (20 TUs) / OSExec.h:25-26 (7) / QuestDownloadServer.h:134 (6) / dvd.h:196 (5) | **38** | all reached by edge-A-hijackable includes (`<dolphin/gx/GXVert.h>` gx.h:33, `"dolphin/os/OSExec.h"` os.h:11, `<dolphin/dvd.h>` os.h:88/card.h:6, `"pso/quest/…"` from src/) | **YES — supplied here** |
| musyx.h:161 bare `extern "C" {` in .c TUs (musyx class §4.4) | 27 | `pshim/musyx/musyx.h` exists in P2-T prototype (other dir, other lane) | no (not MA supply) |
| TU-body `asm void f(...) {…}` MWCC asm-typed definitions (§4.9 tail) | 6 | `src/pso/math/Matrix{Transforms,Vector}.cpp`, `animation/MotionContext.cpp`, `unk/text_803D9650/803D96A8/8042E8C4.cpp` | no (per-TU port/waiver) |
| hw_regs.h raw `volatile u16 __VIRegs[59] : 0xCC002000;` (§4.9) | 1 | 8 raw-colon pins | P2-T `pshim/dolphin/hw_regs.h` covers (needs its own -I) |
| dsp_import.c `extern "C"`-in-.c (musyx co-class) | 1 | — | §4.4 lane |

The §4.5 "~38" is the `AT_ADDRESS` row: **38 TUs exactly** (4 .c / 34 .cpp).
Root cause needs no header-body rewrite: whatever processes `AT_ADDRESS` expands
the colon form; at H3 the ACTIVE definition is carried by the collision pin
(verbatim era tail) and the rebind (`host/bringup/include/types.h:68`), both
off-limits to this lane — so supply = shadow files that own the first site.

## 2. Supply (compat/repin/**, AW-pattern, inert until flags land)

Four era headers copied **VERBATIM** + exactly ONE preamble block inserted before
the file's first `AT_ADDRESS(` site: `#undef AT_ADDRESS` + empty re-define — the
P2-T-proven neutral form ("a port defines the pin empty",
`compat/dialect/pso_dialect_compat.h` §1; flip_test.sh precedent). The macro fix
is TU-global: it also silences the era sites reached WITHOUT their own shadow
(os.h:125-136, OSLink.h:27-28, dsp/dsp.h:10 — 19 further sites, none first-error
today only because chains hit §1 sites first). Generator:
`compat/repin/tools/gen_dialect_shadows.py` (era line numbers asserted, drift
fails loud). Guard-compatible with the pin: shadows carry their OWN era guards
(`_DOLPHIN_GXVERT`/`_DOLPHIN_OSEXEC_H_`/`_DOLPHIN_DVD_H`/
`PSO_QUEST_QUESTDOWNLOADSERVER_H`) which `pin_dolphin_types.h` never occupies;
zero quoted-sibling includes of these four headers exist in the era tree (grepped),
so era-copy double-processing is impossible; `#undef` kills the pin-carried era
define at first hit. **Never `-include`** anything in this dir (apply4 caveat);
`-Icompat/repin` only — the shadows therefore ride AZ's landing recipe with **no
new flag lines** (§5 of the debt doc already carries `-Icompat/repin` first).
Rebind-only per-TU adoption rows add just `-Icompat/repin` ahead of the base
list; the shadows self-heal even if AW's `dolphin/types.h` shadow is also
admitted (that file is skipped at H3 by the pin guard anyway; the preamble is
idempotent under `#ifdef`).

## 3. Measurement (battery-3, same-turn logs; pairwise vs AZ's H3 sweep)

| quantity | value | receipt |
|---|---|---|
| corpus | 1,680 units (`b3/all.txt`) | MA sweep M3 = AZ §5 full-stack recipe |
| AZ H3 clean (06:14 baseline) | 1259 | `az/H3/pass.txt` |
| MA M3 clean (this sweep) | **1301** | `ma/M3/pass.txt` |
| class cascade repaired (`expected …` from §1 sites gone) | **38/38** | AT-regex pass over `ma/M3/err/` |
| class TUs fully clean | 23/38 | table below |
| class TUs residual (co-class wall, 15/15 signatures PRE-EXIST in AZ logs) | 15 | `ma/counts.json` |
| breakage (az-clean → ma-fail, shared universe) | **0** | python set-diff |
| shared-universe flips | 32 = 23 MA class + 9 P2-NC drift | see note |
| control: same 23 TUs WITHOUT `-Icompat/repin` (njs+pin kept) | 23/23 fail again (69 AT-cascade errors) | `ma/C_norepin/` |
| control: same 23 WITHOUT `-Icompat/njs` (repin+pin kept) | 8/23 still clean (15 dual-class ride §4.2 too; all 23 .cpp, so .c-ban untouched) | `ma/C_nons/` |

**Baseline-drift note (honest attribution):** between AZ's 06:14 sweep and now,
NC (`221f683`) landed `compat/njs/pso/map/FloorPopulation.h` INSIDE the H3
`-Icompat/njs` path and the corpus gained 16 renamed units (AZ universe kept 16
stale names; intersection 1,664). The 9 non-class flips (src/pso/boss/galgryphon/TObjBoss5Ctrl.cpp… all
FloorPopulation.h:56/60/67 asserts, zero AT lines in their AZ logs) are NC's
repairs surfacing in my sweep, NOT MA's — AZ's own harness re-run reproduces
them without my files. The 16 MA-side-unmeasured renamed units are excluded
from all flip math here. Waiver header re-hashed mid-window: bytes still
`c6b514035f39b633…` (pin intact, mtime only).

## 4. TU table — class members, header class, spelling, probe status

| TU | first-fail site (AZ H3) | MWCC spelling (era text) | MA M3 status |
|---|---|---|---|
| `src/pso/effect/particle/ParticleStrip.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN |
| `src/pso/effect/particle/TParticle.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/effect/particle/TTelepipeMarker.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/network/mdm.c` | OSExec.h:25 | OSExecParams* __OSExecParams AT_ADDRESS(0x800030F0); | residual: OSTime.h facade vs era `OSTime OSGetTime(void)` (D4 §4.8 co-class) |
| `src/pso/quest/AskDicConvert.cpp` | QuestDownloadServer.h:134 | u32 __OSBusClock AT_ADDRESS(0x800000F8); | CLEAN |
| `src/pso/quest/AskDicFile.cpp` | QuestDownloadServer.h:134 | u32 __OSBusClock AT_ADDRESS(0x800000F8); | CLEAN |
| `src/pso/quest/AskDicLookup.cpp` | QuestDownloadServer.h:134 | u32 __OSBusClock AT_ADDRESS(0x800000F8); | CLEAN |
| `src/pso/quest/QuestDownloadAramFile.cpp` | QuestDownloadServer.h:134 | u32 __OSBusClock AT_ADDRESS(0x800000F8); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/quest/QuestDownloadNameCache.cpp` | QuestDownloadServer.h:134 | u32 __OSBusClock AT_ADDRESS(0x800000F8); | CLEAN |
| `src/pso/quest/QuestDownloadRequests.cpp` | QuestDownloadServer.h:134 | u32 __OSBusClock AT_ADDRESS(0x800000F8); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/render/DebugFont.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | residual: TU-body: MWCC `__builtin_va_info` (§4.9 tail) |
| `src/pso/render/GinjaDrawUtil.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | residual: OSTime.h facade vs era `OSTime OSGetTime(void)` (D4 §4.8 co-class) |
| `src/pso/render/GinjaMaterial.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | residual: TU-body: NJS u32-handle subscript (§4.2 co-class) |
| `src/pso/render/GinjaModelQueue.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/render/SceneFog.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | residual: TU-body: `asm void f(){}` definition (§4.9 tail) |
| `src/pso/render/ScreenPrimitiveDraw.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/render/ScreenSpaceMatrix.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/render/ShapeMotionDraw.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | residual: TU-body: overload residual (§4.9 tail) |
| `src/pso/render/TexturedVertexDraw.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN |
| `src/pso/render/gjLight.c` | OSExec.h:25 | OSExecParams* __OSExecParams AT_ADDRESS(0x800030F0); | residual: OSTime.h facade vs era `OSTime OSGetTime(void)` (D4 §4.8 co-class) |
| `src/pso/render/gjSystem.c` | OSExec.h:25 | OSExecParams* __OSExecParams AT_ADDRESS(0x800030F0); | residual: OSTime.h facade vs era `OSTime OSGetTime(void)` (D4 §4.8 co-class) |
| `src/pso/render/nuReLoadTexture.c` | OSExec.h:25 | OSExecParams* __OSExecParams AT_ADDRESS(0x800030F0); | residual: OSTime.h facade vs era `OSTime OSGetTime(void)` (D4 §4.8 co-class) |
| `src/pso/render/reflection.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | residual: TU-body: NJS-handle dialect (§4.2 co-class) |
| `src/pso/render/render_util.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/render/render_util_head.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/system/TrailStrip.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN |
| `src/pso/system/movie/Gcci.cpp` | OSExec.h:25 | OSExecParams* __OSExecParams AT_ADDRESS(0x800030F0); | residual: OSTime.h facade vs era `OSTime OSGetTime(void)` (D4 §4.8 co-class) |
| `src/pso/system/save/McFsGc.cpp` | OSExec.h:25 | OSExecParams* __OSExecParams AT_ADDRESS(0x800030F0); | residual: OSTime.h facade vs era `OSTime OSGetTime(void)` (D4 §4.8 co-class) |
| `src/pso/system/save/McFsGcRequests.cpp` | dvd.h:196 | DVDDiskID DiskID AT_ADDRESS(0x80000000); | CLEAN |
| `src/pso/ui/frontend/TAdMcCheckGC.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/ui/frontend/TAdPlayerLoadGC_801E0C30.cpp` | dvd.h:196 | DVDDiskID DiskID AT_ADDRESS(0x80000000); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/ui/frontend/TAdSinglePlyChrSelectGC_801FBF54.cpp` | dvd.h:196 | DVDDiskID DiskID AT_ADDRESS(0x80000000); | residual: packet_classes.h KEEP-assert (§4.1 co-class) |
| `src/pso/ui/keyboard/SystemBootInit.cpp` | OSExec.h:25 | OSExecParams* __OSExecParams AT_ADDRESS(0x800030F0); | residual: OSTime.h facade vs era `OSTime OSGetTime(void)` (D4 §4.8 co-class) |
| `src/pso/ui/window/TWindowSubStatusDisp_803DEE6C.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/unk/TLobby_800F8AA8.cpp` | dvd.h:196 | DVDDiskID DiskID AT_ADDRESS(0x80000000); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/unk/TLobby_801E2F94.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN (also rides §4.2 NJS shadow) |
| `src/pso/unk/text_803DD5E0.cpp` | dvd.h:196 | DVDDiskID DiskID AT_ADDRESS(0x80000000); | residual: TU-body: NJS-handle dialect (§4.2 co-class) |
| `src/pso/unk/text_803E2D70.cpp` | GXVert.h:25 | volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR); | CLEAN (also rides §4.2 NJS shadow) |

Residual walls are queue items §4.1/§4.2/§4.8/§4.9 — none re-enters this class
(AT cascade verified gone in all 15 MA logs).

## 5. Paste-ready flags (integrator-owned; zero new flags vs AZ §5)

Full stack (MA shadows already in `compat/repin` — AZ recipe UNCHANGED):

```
-DPSO_PORT_LAYOUT_WAIVER
-include compat/pso_metal_compat.h -include build/waivers/pso_assert_waivers.h -include compat/collision/pin_dolphin_types.h
-I compat/repin -I compat/njs -I host/bringup/include -I include -I include/clib -I libs/dolphin/include -I libs/dolphin/include/dolphin -I src
```

Rebind-only adoption rows (per-TU; 4 .c class TUs stay blocked on the OSTime
co-class under this shape either way):

```
-DPSO_PORT_LAYOUT_WAIVER
-include compat/pso_metal_compat.h -include build/waivers/pso_assert_waivers.h -include compat/collision/pin_dolphin_types.h
-I compat/repin -I host/bringup/include -I include -I include/clib -I libs/dolphin/include -I libs/dolphin/include/dolphin -I src
```

(`-Icompat/njs` stays C++-allowlist-only; all 23 clean class TUs are `.cpp`, so
the full-stack row is legal for every one of them.)

## 6. Gate + repro

1. Class gate: `grep -lE "(GXVert|OSExec|OSLink|dvd|dsp|QuestDownloadServer)\.h:[0-9]+.*expected" <err corpus>` must be 0 under the adopted shape (MA sweep: 0).
2. Re-pin §3 numbers when the official post-adoption build/parse lands (card: official parse-lp64 target mislabels arm64_32 until the integrator Makefile fix; scratch numbers are the authority this window).
3. Repro: `cache/scratch/ma/` — `ma_probe.sh` (shapes M0/M3), `ma_sweep.sh`, `ma_ctrl.sh` (attribution controls), `ma_classify.py`/`ma_sites.py`/`ma_doc.py` (census), `M3/err/` per-file receipts, `counts.json`.
4. Inertness: `grep -rn 'compat/repin' build/ host/ data/` = 1 hit, a
   comment-only statement of the never-`-include` law at `build/Makefile:47`;
   zero flag wiring (AW pattern preserved).
