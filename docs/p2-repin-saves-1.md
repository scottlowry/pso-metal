# P2-AW — Save-format trio re-pin prototypes: measure, don't adopt (evidence pack)

Phase-2 lane AW, 2026-11-27. Decomp `pso-decomp @6b38b5383` READ-ONLY; **zero
make/ninja** — every number is a scratch `clang -fsyntax-only`/`-c` probe over the
battery-2 corpus (`$TMPDIR/b2_lp64/err/`, 1,680 units / 1,519 failing). Inputs:
`docs/p1-format-saves.md` (G's byte tables — the format ground truth),
`docs/p1-datamodel.md` (J rule-1 rebind), `docs/p2-njs-repin-1.md` (AR shadow
pattern + 11/38 blast class), `docs/p2-residual-census.md` (AS2 wall + the
ChallengeRecord verdict contradiction). Writes: `compat/repin/**` (new, inert) +
this doc only.

## 0. The headline finding (changes the ask's premise)

The trio's LOCKED wall is **not a pointer wall — it is a scalar-width wall**. Era
`include/types.h` spells `typedef unsigned long u32`: on LP64 that is 8 bytes, so
`sizeof(TItemData)` inflates 0x14→0x28 and every KEEP-format assert
(`pso_assert_waivers.h` flags all 11 trio names to 0 = fail-closed) fires. The
three structs contain **zero pointer-bearing fields** (§1 census, static + compile
proof), so the AR "pointer→u32 handle slot" conversion has *nothing to convert*
here. The mechanism that restores byte-truth is the committed port rebind
(`host/bringup/include/types.h`, J rule-1, `-DPSO_PORT_LAYOUT_WAIVER`); the
shadows in `compat/repin/` are the **audit layer** — verbatim era copies + 37
private `pso_repin_*` negative-typedef pins that re-assert G's card offsets
under whatever build admits them. The POF0 analogue holds in its strongest form:
the card copy path (memcpy + word BE swap, G §5) is **unchanged and
un-widenable** — 4-byte words all the way, zero slot stamps.

## 1. Slot census (era offsets; cited to header + G card)

**Pointer-bearing fields in the trio and its whole 35-header save include-closure:
NONE.** (Scripted closure walk + struct-body regex over
`pso/item/TItemData.h`, `pso/quest/challenge/ChallengeRecord.h`,
`pso/ui/frontend/TAdSinglePlyChrSelectGC.h` @6b38b5383; only pointer fields found
are runtime state, correctly outside the saved image.)

| struct (era decl) | fields (offset:size) — G card `p1-format-saves.md` §2/§3.1 | ptr fields |
|---|---|---|
| `TItemData` `TItemData.h:56-69` (asserts :151-153) | `data1{u8[12]/u16[6]/u32[3]}`@0x00:0xC · `id u32`@0x0C:4 · `data2{u8[4]/u16[2]/u32/s32}`@0x10:4 · size 0x14 | 0 |
| `SystemSaveData` `TAdSinglePlyChrSelectGC.h:35-49` (= whole PSO_SYSTEM payload) | `checksum u32`@0x00 · `music_stream_gain s16`@0x04 · `cue_stream_gain s8`@0x06 · `language s8`@0x07 · `clock_offset_frames s32`@0x08 · `udp_option s16`@0x0C · `field_e s16`@0x0E · `data_10 u8[0x100]`@0x10 · `serial_number u8[8]`@0x110 · `serial_seed s32`@0x118 · size 0x11C | 0 |
| `ChallengeRecord` `ChallengeRecord.h:31-42` (7 era asserts) | `title_color u16`@0x00 · `stage_times s32[23]`@0x04 · `tombstone`@0x60 (0x50) · `tombstone_spot`@0xB0 (0x10) · `ranks ChallengeStageRank[3]{u32,s32}`@0xC0 (0x18) · `rank_title char[0xC]`@0xD8 · `ep2_split_rank`@0xE4 · `ep2_split_stage_times s32[5]`@0xEC → size 0x100; embedded at CharacterSaveRecord+0x21C4 (G §2) | 0 |
| runtime-only (NOT slot candidates) | `CharacterBufferTable.char_data[4]/*records[4]` (`TAdSinglePlyChrSelectGC.h:30-31`, heap refs — AR `GinjaSceneState` precedent); `TAdSinglePlyChrSelectGC` window ptrs @0x24-0x54; `TObject` vptr chain; `CardSaveDescriptor` (G §1 RUNTIME-WAIVE) | 24 sites all in-scene-graph |

`repin_handles.h` ships `pso_repin_g2h/h2g` (pso_g2h lineage, slot==0 ↔ NULL) as
the landing pad for the *next* shadow that does need slots (AR §4.3
FloorPopulation gap); the trio consumes none of it.

## 2. Belt-and-braces: pins vs G, live-executed

Probe TU (`cache/scratch` `aw_pinprobe.cpp`) compiled under
`-Icompat/repin -Icompat/njs -DPSO_PORT_LAYOUT_WAIVER -Ihost/bringup/include` +
committed force-includes → **all 37 `pso_repin_*` pins + all 11 era KEEP asserts
pass**; executed on-host prints (identical to G's card, zero mismatches):
`TItemData 0x14 (data1@0,id@0xC,data2@0x10)` · `SystemSaveData 0x11C
(0,4,6,7,8,0xC,0xE,0x10[sz0x100],0x110,0x118)` · `ChallengeRecord 0x100
(0,4,0x60,0xB0,0xC0,0xD8,0xE4,0xEC)`, `ChallengeStageRank 0x8`,
`TObjTombstoneRecord 0x50 (date@8,pos@0x10)`, `TObjTombstoneSpot 0x10`.
Note the re-pins *also* pass honestly at H1 without any shadow — era headers +
rebind = byte-truth; the shadow delta is audit, not unlock (§3, by measurement).

## 3. Measurement matrix (battery-2 corpus, 1,680 units, uncapped)

Shapes (all = committed 2806-waive era + compat force-includes + LP64 triple):
H1 = +rebind · H2 = H1+`-Icompat/repin` FIRST (trio shadows) · H2N = H2+
`-Icompat/njs` · H2F = H2 + `dolphin/types.h` shadow (now in compat/repin) ·
H2FN = H2F+`-Icompat/njs`. **Flip** = H0-fail → clean; **breakage** = H0-clean
(161) → fails; **gated14** = files whose ONLY H0 blockers were trio KEEP names
(TItemData-only 8, SystemSaveData-only 6, ChallengeRecord-only 0).

| shape | total clean (of 1680) | flips | breakage | NET vs 161 | gated14 flips (I/S/CR) |
|---|---|---|---|---|---|
| H0 (battery-2) | 161 | — | — | — | baseline |
| H1 rebind | 215 | 72 | 18 | +54 | 6 / 6 / 0 (12 of 14) |
| H2 +repin shadows | 215 | 72 | 18 | +54 | 6 / 6 / 0 (12 of 14) |
| H2N +njs shadows | 374 | 231 | 18 | +153 | 6 / 6 / 0 |
| H2F +dolphin shadow | 217 | 72 | **16** | +56 | 6 / 6 / 0 |
| H2FN full stack | 379 | **234** | **16** | **+173** | 6 / 6 / 0 |

Honest intersections:
- **Shadow-pair delta H1→H2: 0 flips, 0 breakage, 0 per-file error regressions
  (checked pairwise, all 1,680):** the trio shadows add pins, never parse risk.
- **Trio-attributable unlock = the 12 gated files** (6 SystemSaveData incl. the
  whole save-root: `TAdPlayerLoadGC_80076BBC`, 4× `TAdSinglePlyChrSelectGC_*`,
  `text_801A5778` — AS2 §4's "cheapest structural ask"; 6 TItemData-only:
  `BankItemRecord`, `ShopItemSort`, `TShopEquipmentListWindow_8010ED54`,
  `TWindowShortListCtrl_80221828`, `WindowHelpItemTextB`, `TLobby_8021B268`).
  The other 2 gated (`subcommand_validation_technique`, `text_8036BF10`) die on
  the rebind-vs-dolphin collision, not on layout.
- Wide attribution: 59 of the 72 H1 flips carry TItemData keeps among other
  scalar-LOCK names (rebind fixes ALL scalar format keeps at once — ItemData
  co-gate 1,106, SystemSaveData 236, ChallengeRecord 903 carried exactly,
  re-derived from b2 logs = AS2 §6.6). Under H2FN, 101 flips carry ItemData keeps.
- **Blast radius (AR 11/38 class)**: breakage is 18 files ALL rebind-caused
  (17 = dolphin/types.h 5-error collision cluster, 1 = `GcrnaStream.cpp`
  OSAlloc rebind-surfaced), identical sets H1=H2=H2N → *zero repin-shadow
  breakage*; the dolphin shadow repairs 2, breaks 0 (16 remain: quoted-include
  siblings, §4.3). NJS shadow adds 0 breakage on b2-clean files in this corpus
  era (its +25/+22 initializer clusters were *failing*-pool regressions per AR).
- **Rebind-vs-dolphin collision is tree-wide: 1,136/1,680 TUs carry the
  5-error cluster at H1** (vs AS2's 58-in-172 pool estimate). My
  `<dolphin/types.h>` shadow repairs only the angle-form (1,109 still collide —
  dominant form is SDK-internal quoted siblings, e.g. `mtx.h:4` re-including
  `types.h` from the dolphin dir, immune to `-I` ordering). Net measured effect
  of the dolphin shadow: +2 repaired breakage, +3 flips over H2N (231→234).

## 4. ChallengeRecord: both AX verdicts, pre-plugged (no re-measurement needed)

- **RW (AC/`QuestScriptFormat`-style label)**: flags flip to 1 in the next
  `gen_waivers` regen → **0 gates, 0 flips**: zero files are solo-blocked by
  CR names under any shape (0/903), and 0 files' H2FN residual is CR-only
  ("CR-solo unlocks" measured = ∅). RW unlocks NOTHING parse-wise — and it is
  actively unsafe unrebound: CR is inside the saved `CharacterSaveRecord` bytes
  at +0x21C4 (G §2), so waiving + era-`long` scalars = compiles green, writes
  corrupt cards (silent class).
- **LOCKED (re-pin)**: same count (0 solo flips) but *honest*: under H1+ the 7
  CR era asserts + my 16 CR pins pass on real 0x100 layout; the 903 carriers
  stay gated behind NJS/dialect/other-LOCK co-gates (0 flip even under H2FN).
  CR's parse value is realized only as a co-pinned passenger; its format value
  is realized by the rebind regardless of label.
→ **AX's verdict changes honesty posture, not unlock math. Either way, 0 files
ride on CR alone.** The pins are pre-gated `-DPSO_REPIN_RW_CHALLENGERECORD` for
a future RW adopter (unset by default = LOCKED side measured).

## 5. Consumer obligations (loaders/stamps)

1. **No handle stamps for the trio** (zero slots) — the runtime obligation is
   G §5's: memcpy boundaries `TAdSinglePlyChrSelectGC_801FBF54.cpp:399,488,1955,
   1999,2017` (load) / `:2237,2255,2276` + encrypt calls (save); swap walk
   must cover `ChallengeRecord` + `SystemSaveData` scalars + `TItemData`
   per-32-bit-word granularity (G §5.2-§5.4, currently MISSING coverage).
2. Any consumer derefing these fields as pointers: **none exist** (census §1),
   so no `NJS_HANDLE`-style conversions are owed; grep-for-breakage ceiling is 0.
3. `dolphin/types.h` shadow (H2F) obliges the landing lane to solve the quoted
   sibling form before global adoption (SDK pre-include pin or guard macro) —
   27 TUs clear the angle form; 1,109 still collide.
4. Cipher: `SubtractiveRng::apply_rng_cipher` u32 arithmetic (G §5.3) — the
   rebind *makes this byte-correct again* on LP64 (u32 wraps at 32 bits like
   the era); it must NOT be re-widened.

## 6. Risk table

| risk | severity | evidence |
|---|---|---|
| Global rebind without collision fix | HIGH | 1,136 TUs gain the 5-error dolphin cluster; 18 files flip clean→broken; 2 of the 14 gated files lost |
| Silent-layout lies if RW labels spread to format PODs | HIGH | CR inside saved image (§4); G §0: the real saves danger is the opposite of the assert wall |
| Shadow drift vs future decomp headers | MED | pins fail loud on drift (negative typedefs); `tools/gen_shadows.py` regenerates verbatim |
| C-TU laundering | LOW here | trio shadows are C++-safe PODs; battery pool has 0 C-TU needing them (all 14 gated + 72 flips are `.cpp`; still honor AR's `.c` ban on `-Icompat/repin`) |
| `-DPSO_REPIN_RW_CHALLENGERECORD` misuse | LOW | documented, default off |
| CharacterBufferTable/window ptrs tempted into slots | LOW | §1: heap indirection, host-ptr is correct (AR precedent) |

## 7. Recommendation (integrator + Scott) — adoption shape

**Adopt the rebind path, not the shadow path, and stage by chain proximity:**
(1) the trio needs no per-TU shadow for unlock — `-Ihost/bringup/include`
(+`-DPSO_PORT_LAYOUT_WAIVER`) delivers all 12 trio-attributable flips; the
shadows go in **first, everywhere the rebind is admitted**, because they cost
0 parse risk (measured H1≡H2) and pin G's card permanently. (2) Ship the
dolphin/types.h collision work (SDK-side pin for quoted siblings) as the gate:
until it lands, keep rebind+repin per-TU opt-in in
`build/njs_allowlist.txt`-style staging — start with the three chains closest
to green: **SystemSaveData 6 (save-root/character-select), TItemData-only 6
(shop/bank/lobby UI), then the 59-wide ItemData-family co-gated Adx/Lsc
audio cluster** (~23 already flipping at H1). (3) Global flip expectation
with full honest stack (H2FN): 161→379 clean (+234 flips, −16 breakage) ≈
+213 net — but the marginal +159 belongs to the NJS allow-list, +3 to the
dolphin shadow; the trio's own realistic unlock is **12 direct / 72-101 wide
co-attributable**. (4) Gate any RW-vs-LOCKED CR decision on §4: it is a
honesty choice, not an unlock choice; recommend LOCKED (byte-truth) since the
struct is provably inside the card image.

## Repro / zero-adoption

`cache/scratch/`: `aw_battery.sh` (H1/H2/H2N), `aw_battery_f.sh` (H2F/H2FN),
`aw_closure.py` (census), `aw_analyze.py`+`aw_matrix.py` (matrices),
`aw_pinprobe.cpp` (byte-truth dump). Gated file rosters in
`aw_repine/analysis2.json`. Repo grep (proved at commit time): `compat/repin`
appears in no build/host/data file — this dir + this doc only. Caveat carried
from AS2 §7: bucket sets over raw counts; 16 stale-corpus units counted as
never-flipping (they need clang's missing-file exit, not a re-pin).
