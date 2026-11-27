# P2-STA1 — W-STATIC-TREE: static-init guest-literal program (design, ZERO builds)

Fleet wave-5 lane STA1 (zero-build design), card `t_f52a5a63`, 2026-11-27. Solves
what `pso_njs_h2g` CANNOT: the 81 AQP2-named static-init sites whose `.rodata`
string payloads must become u32 slot values inside compile-time initializers.
Inputs (all read-intake): HP `docs/p2-handle-program-1.md` §2/§3 (window classes,
seam API, loaders), HG-1 `docs/p2-hg1-1.md` + `host/bringup/include/hg1_windows.h`
(runtime registry FORM — anchors, bind, crossing helpers, fail-loud), AQP2
`docs/p2-adapter-queue-2.md` §4 (the 54 + 27 site census → this doc's 81), NA
`docs/p2-njs-adopt-1.md` §4 (the ban grammar), AR shadow
`compat/njs/pso/render/GinjaMaterialState.h` (the re-typed `NJS_TEXNAME`).
Decomp `pso-decomp @6b38b5383` READ-ONLY (git-verified same turn); zero repo
builds, zero `make`, waiver pin `d4382fcd0659357f…` era stands untouched
(measured==pin by AQP2 same-era, re-hash is the builder's duty per D8). Scratch:
`$TMP/sta1/` (fresh whole files, `clang -fsyntax-only` sweeps both languages).
Writes: this doc + `compat/njs/handle/static/README.md` ONLY — INERT landing.

## 1. The key question, answered: YES for all 81

The h2g dead end AQP2 §4 named is real: `pso_njs_h2g(&"f008_white_npr01")` is a
RUNTIME call (no constant initializer can contain it), its argument is a HOST
address (window law: unregistered host pointers are rejected before arithmetic),
and the `(u32)"lit"` truncation cast is precisely the NA §4 `addr-or-this->int`
laundering AQP2 §4 already refused. But HG-1 proved the other direction works
as pure arithmetic: with windows bound at fixed arena offsets, a guest byte
address like `0x80034040` is a COMPILE-TIME-visible constant of the contract
(HG1-R: `h2g_main(arena+0x34040) == 0x80034040` exactly). So separate the two
halves of what the era initializer really asserted:

* the **slot VALUE** — the guest address of the name string — is authored
  directly as an integer constant expression over window-contract constants:
  `PSO_ST_SLOT(k) = 0x80080000u + 32u*(k)`;
* the string **BYTES** — the payload the value names — are STAGED at load into
  an arena span that L-IMAGE binds as a `PSO_W_IMAGE` record (the g_sinbd
  `.data`-placement regime, V-wave V2, generalized).

The era's own contract shape survives verbatim: `NJS_TEXNAME.filename` was a
`char *` to an image-resident string (era header
`include/pso/render/GinjaMaterialState.h:37-43`); the port slots hold the GUEST
NAME of that string's staged home. No h2g call, no cast, no per-TU init hook —
the initializer is parse-legal u32 arithmetic. The rewrite pattern is
`{"f008_white_npr01", 0, NULL}` → `{PSO_ST_SLOT(idx), 0, NULL}`.

## 2. The 81-site census: 81 guest-literal-able / 0 runtime-hook / 0 blocked

Per-site rows (index = blob cell = census order; file:line + literal + init
shape read from the era source THIS turn, python-matched 81/81, zero
non-matching lines — `sites81.json`): class **A** = live read-key (the string
bytes are resolved and consumed at runtime through the slot), **B** =
era-faithful dead initializer (runtime rewrites the slot before any consumer),
**C** = live-key single texname. Evidence paths per class:

* **A / CameraCapture (54)** — `static NJS_TEXNAME effect_textures[54]`
  (:34), lines :35-:88, one filename slot each, shape `{"name", 0, NULL}`;
  `shared_effect_texture_list = {effect_textures, 54}` (:91) is bound by
  `TEffect.cpp:10` (`effect_texture_file = {&shared_effect_texture_list,
  "effect_nt.gvm"}`) → `:96 bind_gvm_texture_list` → the name-walker family:
  `text_803DD5E0.cpp:89` `append_extension_if_missing(name,
  list->textures[index].filename, ".GVR")` → `DVDOpen` — reads the STRING
  THROUGH THE SLOT ⇒ literal must resolve. `text_803DD5E0.cpp:302/:332-334`
  `replace_gvm_texture_directory` may `move_data(texture->filename, record,
  length)` (`TLobby_803DE778.cpp:8` = memmove) — writes INTO the bytes the slot
  points at: era-semantics preserved exactly IF the payload sits in the RW
  arena (a host `.rodata` target would SIGSEGV the port — the staging design is
  not merely typing-clean, it is what keeps this era write path legal).
  Downstream consumers: `TMainCursor.cpp:329/:1143 set_texture_list(&…)`,
  `ParticleDefinition.cpp:13/:98+` positional refs, `TEffect.cpp:98` walks
  `.texture->surface->id` (union slot @0x8 — batch-3/loader-filled, not ours).
* **B / TBoss3VoloptMonitor (25)** — `static NJS_TEXNAME
  volopt_monitor_texture_entries[25]` (:76), lines :77-:101, partial-brace
  shape `{ "name" }` (attr + union zero-init → slot 0 = NULL law, no extra
  error). `:198` points `volopt_monitor_textures` at the table, `:200`
  `clear_texlist_names(&volopt_monitor_texture_table)` REWRITES all 25 slots to
  `&texlist_null_name` (`NinjaUtil.cpp:7` `static char texlist_null_name[0x20]`,
  `:12`), `:201 psoLoadTexturePvmFile("fe_obj_vo_momitor.gvm", …)`
  (`psoLoadTexture.cpp:37/:65`) re-derives names from the archive. The 25
  literals are DEAD at runtime (era image bytes, never resolved) — but they are
  era data the compile must still type: same literal pattern, same cell map.
  RIDER (not a 82nd site): `texlist_null_name[0x20]` becomes blob cell 81 so
  the clear-store and every aliased reader resolve inside the window (§3).
* **C / TAdMcSelectCursorGC :20, TAdMcSelectMcGC :14 (2)** — global (external
  linkage) `NJS_TEXNAME mc_select_*_texname = { "name", 0, NULL }`; the same
  literal re-appears as a `const char*` ARGUMENT at `:147` / `:119`
  (`load_streamed_prs_texture("f256_memorycard_select", …)`, defined
  `TObjPlayer_8023E480.cpp:37-49`: `sprintf`+PRS into buffers, then
  `load_gvm_texture_archive` — the same memmove-into-payload path). No
  cross-TU address reference to either texname exists (grep all of
  `src/`+`include/`): the payload is TU-adjacent-owned, size fixed, value
  contract-derived ⇒ literal-able. (The adjacent `:21`/`:15` TEXLIST link
  slots stay AQP2 batch-3 — the 13 sites this program does NOT touch.)

**Why zero runtime-hook:** the hook shape (de-brace + `pso_njs_h2g(&peer)`
store in an init fn) exists because a peer ARRAY address is a host symbol the
initializer cannot name. Here the payload is NOT a host object any more — it is
a contract cell — so the value is constexpr and the store at init time is the
compiler's. A hook would ADD a write-once-ledger interaction for zero gain.
**Why zero blocked:** every site is (i) TU-local or single-referenced literal
payload, (ii) fixed size ≤ 23+1 bytes (max over census = 24 B; 70 of 81 are
>16 B so no 16-byte-name truncation convention is available — cells are 32 B),
(iii) read only through the slot or the identical literal argument, (iv) free of
disc-side address references (§6 audit item covers the residual).

| # | site (file:line) | literal | bytes | era init shape | cls |
|---|---|---|---|---|---|
| 0 | CameraCapture.cpp:35 | `"f008_white_npr01"` | 16+1=17 B | `{"…", 0, NULL}` | A |
| 1 | CameraCapture.cpp:36 | `"f032_awa_apn01"` | 14+1=15 B | `{"…", 0, NULL}` | A |
| 2 | CameraCapture.cpp:37 | `"f032_chun_npn01"` | 15+1=16 B | `{"…", 0, NULL}` | A |
| 3 | CameraCapture.cpp:38 | `"f032_flash_apr01"` | 16+1=17 B | `{"…", 0, NULL}` | A |
| 4 | CameraCapture.cpp:39 | `"f032_fog_npn01"` | 14+1=15 B | `{"…", 0, NULL}` | A |
| 5 | CameraCapture.cpp:40 | `"f032_gunbullet_apr01"` | 20+1=21 B | `{"…", 0, NULL}` | A |
| 6 | CameraCapture.cpp:41 | `"f032_kisekigreen_apn01"` | 22+1=23 B | `{"…", 0, NULL}` | A |
| 7 | CameraCapture.cpp:42 | `"f032_kisekired_apn01"` | 20+1=21 B | `{"…", 0, NULL}` | A |
| 8 | CameraCapture.cpp:43 | `"f032_kisekiviolet_apn01"` | 23+1=24 B | `{"…", 0, NULL}` | A |
| 9 | CameraCapture.cpp:44 | `"f032_lens01_apn01"` | 17+1=18 B | `{"…", 0, NULL}` | A |
| 10 | CameraCapture.cpp:45 | `"f032_lens02_apn01"` | 17+1=18 B | `{"…", 0, NULL}` | A |
| 11 | CameraCapture.cpp:46 | `"f032_lens03_apn01"` | 17+1=18 B | `{"…", 0, NULL}` | A |
| 12 | CameraCapture.cpp:47 | `"f032_lens04_apn01"` | 17+1=18 B | `{"…", 0, NULL}` | A |
| 13 | CameraCapture.cpp:48 | `"f032_lens05_apn01"` | 17+1=18 B | `{"…", 0, NULL}` | A |
| 14 | CameraCapture.cpp:49 | `"f032_lens06_apn01"` | 17+1=18 B | `{"…", 0, NULL}` | A |
| 15 | CameraCapture.cpp:50 | `"f032_lens07_apn01"` | 17+1=18 B | `{"…", 0, NULL}` | A |
| 16 | CameraCapture.cpp:51 | `"f032_mshield01_apn01"` | 20+1=21 B | `{"…", 0, NULL}` | A |
| 17 | CameraCapture.cpp:52 | `"f032_mshield02_apn01"` | 20+1=21 B | `{"…", 0, NULL}` | A |
| 18 | CameraCapture.cpp:53 | `"f032_netload_apr01"` | 18+1=19 B | `{"…", 0, NULL}` | A |
| 19 | CameraCapture.cpp:54 | `"f032_redgray_apn01"` | 18+1=19 B | `{"…", 0, NULL}` | A |
| 20 | CameraCapture.cpp:55 | `"f032_senkou_apr01"` | 17+1=18 B | `{"…", 0, NULL}` | A |
| 21 | CameraCapture.cpp:56 | `"f032_sennkou_apr01"` | 18+1=19 B | `{"…", 0, NULL}` | A |
| 22 | CameraCapture.cpp:57 | `"f032_shield01_apn01"` | 19+1=20 B | `{"…", 0, NULL}` | A |
| 23 | CameraCapture.cpp:58 | `"f032_slow_apn01"` | 15+1=16 B | `{"…", 0, NULL}` | A |
| 24 | CameraCapture.cpp:59 | `"f032_tama_apn01"` | 15+1=16 B | `{"…", 0, NULL}` | A |
| 25 | CameraCapture.cpp:60 | `"f064_hane_apr01"` | 15+1=16 B | `{"…", 0, NULL}` | A |
| 26 | CameraCapture.cpp:61 | `"f064_hitfirec_apr01"` | 19+1=20 B | `{"…", 0, NULL}` | A |
| 27 | CameraCapture.cpp:62 | `"f064_hitfirered_apr01"` | 21+1=22 B | `{"…", 0, NULL}` | A |
| 28 | CameraCapture.cpp:63 | `"f064_hitmark_apr01"` | 18+1=19 B | `{"…", 0, NULL}` | A |
| 29 | CameraCapture.cpp:64 | `"f064_hitmarkblue_apr01"` | 22+1=23 B | `{"…", 0, NULL}` | A |
| 30 | CameraCapture.cpp:65 | `"f064_impacte_apn01"` | 18+1=19 B | `{"…", 0, NULL}` | A |
| 31 | CameraCapture.cpp:66 | `"f064_lens01resize_apn01"` | 23+1=24 B | `{"…", 0, NULL}` | A |
| 32 | CameraCapture.cpp:67 | `"f064_megido_apr01"` | 17+1=18 B | `{"…", 0, NULL}` | A |
| 33 | CameraCapture.cpp:68 | `"f064_monshou_apr01"` | 18+1=19 B | `{"…", 0, NULL}` | A |
| 34 | CameraCapture.cpp:69 | `"f064_questloadapn16"` | 19+1=20 B | `{"…", 0, NULL}` | A |
| 35 | CameraCapture.cpp:70 | `"f064_senblue_apn01"` | 18+1=19 B | `{"…", 0, NULL}` | A |
| 36 | CameraCapture.cpp:71 | `"f064_shield02_apn01"` | 19+1=20 B | `{"…", 0, NULL}` | A |
| 37 | CameraCapture.cpp:72 | `"f064_shield03_apn01"` | 19+1=20 B | `{"…", 0, NULL}` | A |
| 38 | CameraCapture.cpp:73 | `"f064_shield04_apn01"` | 19+1=20 B | `{"…", 0, NULL}` | A |
| 39 | CameraCapture.cpp:74 | `"f064_shield05_apn01"` | 19+1=20 B | `{"…", 0, NULL}` | A |
| 40 | CameraCapture.cpp:75 | `"f128_bakuhatu_apn16"` | 19+1=20 B | `{"…", 0, NULL}` | A |
| 41 | CameraCapture.cpp:76 | `"f128_blackfog_nln16"` | 19+1=20 B | `{"…", 0, NULL}` | A |
| 42 | CameraCapture.cpp:77 | `"f128_feio_aln16"` | 15+1=16 B | `{"…", 0, NULL}` | A |
| 43 | CameraCapture.cpp:78 | `"f128_fog_aln16"` | 14+1=15 B | `{"…", 0, NULL}` | A |
| 44 | CameraCapture.cpp:79 | `"f128_granz_aln16"` | 16+1=17 B | `{"…", 0, NULL}` | A |
| 45 | CameraCapture.cpp:80 | `"f128_hibana_aln16"` | 17+1=18 B | `{"…", 0, NULL}` | A |
| 46 | CameraCapture.cpp:81 | `"f128_houden_apr16"` | 17+1=18 B | `{"…", 0, NULL}` | A |
| 47 | CameraCapture.cpp:82 | `"f128_lightning_alr16"` | 20+1=21 B | `{"…", 0, NULL}` | A |
| 48 | CameraCapture.cpp:83 | `"f128_smoke_apn16"` | 16+1=17 B | `{"…", 0, NULL}` | A |
| 49 | CameraCapture.cpp:84 | `"f128_worm_alr16"` | 15+1=16 B | `{"…", 0, NULL}` | A |
| 50 | CameraCapture.cpp:85 | `"f256_zonde_alr16"` | 16+1=17 B | `{"…", 0, NULL}` | A |
| 51 | CameraCapture.cpp:86 | `"k016_happa_npr01"` | 16+1=17 B | `{"…", 0, NULL}` | A |
| 52 | CameraCapture.cpp:87 | `"t032_heart_apn01"` | 16+1=17 B | `{"…", 0, NULL}` | A |
| 53 | CameraCapture.cpp:88 | `"u064_glasshahen_npr01"` | 21+1=22 B | `{"…", 0, NULL}` | A |
| 54 | TBoss3VoloptMonitor.cpp:77 | `"ki_u256_vo_mo_moji001"` | 21+1=22 B | `{ "…" }` | B |
| 55 | TBoss3VoloptMonitor.cpp:78 | `"ki_u256_vo_mo_moji002"` | 21+1=22 B | `{ "…" }` | B |
| 56 | TBoss3VoloptMonitor.cpp:79 | `"ki_u256_vo_mo_moji003"` | 21+1=22 B | `{ "…" }` | B |
| 57 | TBoss3VoloptMonitor.cpp:80 | `"ki_u256_vo_mo_moji004"` | 21+1=22 B | `{ "…" }` | B |
| 58 | TBoss3VoloptMonitor.cpp:81 | `"ki_u256_vo_mo_moji005"` | 21+1=22 B | `{ "…" }` | B |
| 59 | TBoss3VoloptMonitor.cpp:82 | `"ki_u256_vo_mo_moji006"` | 21+1=22 B | `{ "…" }` | B |
| 60 | TBoss3VoloptMonitor.cpp:83 | `"ki_u256_vo_mo_moji007"` | 21+1=22 B | `{ "…" }` | B |
| 61 | TBoss3VoloptMonitor.cpp:84 | `"ki_u256_vo_mo_moji008"` | 21+1=22 B | `{ "…" }` | B |
| 62 | TBoss3VoloptMonitor.cpp:85 | `"ki_u256_vo_mo_noize001"` | 22+1=23 B | `{ "…" }` | B |
| 63 | TBoss3VoloptMonitor.cpp:86 | `"ki_u256_vo_mo_noize002"` | 22+1=23 B | `{ "…" }` | B |
| 64 | TBoss3VoloptMonitor.cpp:87 | `"ki_u256_vo_mo_noize003"` | 22+1=23 B | `{ "…" }` | B |
| 65 | TBoss3VoloptMonitor.cpp:88 | `"ki_u256_vo_mo_noize004"` | 22+1=23 B | `{ "…" }` | B |
| 66 | TBoss3VoloptMonitor.cpp:89 | `"ki_u256_vo_mo_noize005"` | 22+1=23 B | `{ "…" }` | B |
| 67 | TBoss3VoloptMonitor.cpp:90 | `"ki_u256_vo_mo_noize006"` | 22+1=23 B | `{ "…" }` | B |
| 68 | TBoss3VoloptMonitor.cpp:91 | `"ki_u256_vo_mo_noize007"` | 22+1=23 B | `{ "…" }` | B |
| 69 | TBoss3VoloptMonitor.cpp:92 | `"ki_u256_vo_mo_noize008"` | 22+1=23 B | `{ "…" }` | B |
| 70 | TBoss3VoloptMonitor.cpp:93 | `"ki_u256_vo_mo_target001"` | 23+1=24 B | `{ "…" }` | B |
| 71 | TBoss3VoloptMonitor.cpp:94 | `"ki_u256_vo_mo_target002"` | 23+1=24 B | `{ "…" }` | B |
| 72 | TBoss3VoloptMonitor.cpp:95 | `"ki_u256_vo_mo_target003"` | 23+1=24 B | `{ "…" }` | B |
| 73 | TBoss3VoloptMonitor.cpp:96 | `"ki_u256_vo_mo_target004"` | 23+1=24 B | `{ "…" }` | B |
| 74 | TBoss3VoloptMonitor.cpp:97 | `"ki_u256_vo_mo_target005"` | 23+1=24 B | `{ "…" }` | B |
| 75 | TBoss3VoloptMonitor.cpp:98 | `"ki_u256_vo_mo_target006"` | 23+1=24 B | `{ "…" }` | B |
| 76 | TBoss3VoloptMonitor.cpp:99 | `"ki_u256_vo_mo_target007"` | 23+1=24 B | `{ "…" }` | B |
| 77 | TBoss3VoloptMonitor.cpp:100 | `"ki_u256_vo_mo_target008"` | 23+1=24 B | `{ "…" }` | B |
| 78 | TBoss3VoloptMonitor.cpp:101 | `"ki_u064_vo_mo"` | 13+1=14 B | `{ "…" }` | B |
| 79 | TAdMcSelectCursorGC.cpp:20 | `"f256_memorycard_select"` | 22+1=23 B | `{ "…", 0, NULL }` | C |
| 80 | TAdMcSelectMcGC.cpp:14 | `"f256_memorycard"` | 15+1=16 B | `{ "…", 0, NULL }` | C |

Set math (python, same turn): 54+25+1+1 = 81 sites = AQP2 §4 list-1 (54) +
riders (27); A = 54+2 = 56, B = 25; blob padded size 82×32 = 2624 B, compact
1619 B; max strlen+1 = 24 ≤ 32 (fit), all 81 strings distinct (no dup collapse).

## 3. Loader-side registration contract (HG-1 API names, zero new seam spellings)

Window class: **`PSO_W_IMAGE`** — HP §2 already defines it as "image statics",
and these are image statics that happen to be compiled-in rather than POF0-
loaded; AQP2's §4 note that no window SPAN covers compiled-in `.rodata` is
solved by STAGING A SPAN inside the existing class, not by a new enum member.
Stamping loader: **L-IMAGE** (HP §3.1 "chunk/archive loaders + data placement"
— same owner as the g_sinbd placement regime it generalizes).

* **Blob (single global, one source of truth).** `pso_st_blob[82][32]` in one
  bring-up TU: rows 0..80 = the 81 census strings in §2 order (NUL-padded),
  row 81 = 32 zero bytes = `texlist_null_name` cell (era `[0x20]`,
  `NinjaUtil.cpp:7`). Emitted/checked by a build-time census generator
  (builder-lane-owned, like `gen_waivers` is integrator-owned); the era TUs
  keep their strings ONLY in the rewritten slot-expression indices.
* **Span.** guest `[0x80080000, 0x80081000)` ↔ `arena_off 0x80000` (4 KB).
  Collision audit at intake vs every documented carve: IMAGE `0x32000-0x34000`
  (HG1-I0), POOL `0x34000-0x38000` (HG1-I0), V-wave cache-heap regime
  `@0x40000` (p2-v-wave:107), osinit HeapArray+heap `@0x400000`
  (`osinit.c:35/:40`), CACHEHEAP_START `@0x1400000` (hg1_windows.h:27) — the
  gap owns nothing documented; builder re-audits crt0 before landing.
* **Anchor + bind (exact re-use of the landed table/API).** Append a 4th row to
  `pso_era_window_anchors` (`crt0.c:39-46`, `PSO_ERA_WINDOW_COUNT` 3→4):
  `{ PSO_W_IMAGE, 0x80080000u, 0x80081000u, 0x00080000u,
  "W-STATIC-TREE string tree (STA1; texnames + texlist_null_name)" }`. crt0
  remains the only base owner — windows never mmap (HP §3.1). The bind at
  staging completion is `pso_handle_window_bind(PSO_W_IMAGE, 0x80080000u,
  0x80081000u, pso_arena_base + 0x00080000u)`; idempotent same-triple re-bind
  legal, conflicting re-bind fails loud keeping the record (HG1-B). NO unbind
  ever — the tree is never unloaded (floor semantics don't apply); L-IMAGE
  therefore owns the whole program in ONE bind call, and since binding is
  per-WINDOW not per-TU, the same single call owns all 81 sites across the
  four TUs (archive/TU → bind mapping table: CameraCapture 54, Volopt 25,
  MC-cursor 1, MC-memcard 1, NinjaUtil rider cell 1 — all → L-IMAGE / that one
  record).
* **Stage.** `pso_static_tree_stage()`: one-time `memcpy` of the blob to
  `pso_arena_base + 0x80000`, called from bring-up install AFTER
  `pso_bringup_install()` (arena exists) and BEFORE any era init that can
  resolve a name — the AV §1.4-S4 "stamp-before-GX" rule generalized to
  "stage-before-first-resolve" (named first consumers:
  `TEffect.cpp:96 bind_gvm_texture_list`, `TAdMcSelect*GC::init_resource/…`
  `load_streamed_prs_texture`, `TBoss3VoloptMonitor.cpp:200-201`). The RW
  arena (single `MAP_PRIVATE|MAP_ANON` PROT_READ|WRITE mmap, crt0:57) makes
  the `:334` memmove-into-name path behave era-true; host `.rodata` would not.
* **Authored site form.** `PSO_ST_SLOT(idx)` with `idx` from the census (the
  generator emits both). Slot law keeps everything else intact: NULL law
  untouched (`{NULL, 0, NULL}` sites stay 0), slot-space comparisons untouched,
  the `NJS_HANDLE(T, slot)`/`pso_njs_handle_g2h` crossing at every read site is
  the ONLY translation (HP §3.2 resolution order unchanged).
* **Not re-stamped.** The compile-time initializer is not a `pso_handle_stamp_once`
  write (the ledger governs runtime stamping; the static slot was never
  runtime-written until an era code path — replace_gvm / clear_texlist_names —
  legitimately rewrites it, exactly as on the disc).

## 4. The NA §4 ban boundary: static guest literals ARE the era values

The ban's own text (NA §4): a row is gated iff "its source contains
host-pointer↔guest-address coercion that flows through slot space — the exact
class AR §5 named (parse-clean cannot expose it because C++ casts launder it:
`reinterpret_cast`, `saddr`/`uaddr` scalars, int-typed carriers)", enumerated as
five mechanical shapes (`ptr->saddr-cast`, `int->ptr-rcast`,
`saddr-uaddr-coerce`, `addr-or-this->int`, `slotint->ptr`), and its tail states
the WHY: gated rows "touch host pointers in u32 slots at runtime; adopting them
before the uaddr handle program (loader stamping + guest↔host translation at
these sites) would convert loud-parse risk into silent runtime risk."

* **Grammar test.** `PSO_ST_SLOT(idx)` contains NO host-side operand: no cast
  of a pointer expression, no `this`, no `&symbol`, no int-typed carrier being
  reinterpreted. It is an integer constant expression over contract constants —
  none of the five shapes can match a literal whose parse tree has no address
  subexpression at all. The NA screen is line-grepped: every STA1-rewritten
  site ADDS zero gated rows (re-running NA §4's screen over the edited TUs
  finds the same rows as before, never more — §2 shows `(u32)"lit"`, the
  TRUNCATION shape, is what would fire `addr-or-this->int`; AQP2 §4 refused
  exactly that, and STA1 lands on the other side of its own refusal).
* **Substance test.** Laundering = a value with HOST PROVENANCE (ASLR base,
  section layout) smuggled into slot space where the resolver silently
  mis-converts (AR §5's named failure modes include "statics outside the
  arena" — the class this program ELIMINATES by putting the statics inside a
  window). A guest literal's provenance is the contract: byte-identical under
  any `mmap` base, dev-D1-reloc-stable by construction (the same property
  HG1-R asserts for its formula-derived `0x80034040`). The host↔guest EDGE the
  ban insists on is not deleted — it is MOVED to where the handle program
  already put every other crossing: the loader bind (guest→host association,
  dynamic, registry-side) + the registry resolver at read (slot→host,
  range-checked). "Loader stamping + guest↔host translation at these sites" —
  the ban's own stated CURE — is precisely what STA1 pre-applies: stage =
  loader placement, `pso_njs_handle_g2h` = the translation. The values ARE the
  era values in the only sense the slot law ever required: guest-space NAMES
  for the era-true bytes, resolvable byte-exact inside a bound window
  (HG-2.1 below pins it).
* **W-INT contrast (HP §2).** The 12 W-INT rows are illegal-resolvables because
  an unregistered small int is NOT a legitimate slot for any window;
  boundary = REGISTRATION, not magic ranges. STA1 slots are ≥ `0x80000000`
  AND covered by an anchor record before any consumer — they classify
  W-IMAGE. The reject path stays the backstop: a stage-omitted or off-span
  value fails LOUD (HG1-REJECT + abort outside harnesses), so the failure mode
  of a botched STA1 edit is a dead-on-arrival boot, never a silent mis-resolve
  — the exact asymmetry the ban's "loud-parse vs silent-runtime" sentence is
  about.
* **Forbidden boundary (declared).** The moment a slot value is DERIVED FROM A
  HOST ADDRESS at compile time (`(u32)&blob_cell[k]`, `pso_njs_h2g(&"lit")`
  smuggled into a constexpr, a linker-relocated absolute) it is back inside
  the ban regardless of numeric coincidence — `addr-or-this->int` with extra
  steps. The contract's arithmetic form (`GUEST_BEGIN + CELL*k`, no host term
  expressible) is what keeps the class honest, and HG-2.1(a) rejects
  implementation drift by asserting the value formula, not just round-trip.
* **A note on "era absolute".** No claim is made that the literals equal the
  disc binary's original `.rodata` addresses (no era link map is present at
  @6b38b5383 — checked). Era-truth in this program = era BYTES (the strings,
  staged verbatim, NULs inside cells) + era VALUE SEMANTICS (guest address of
  those bytes, per the port's fixed-window contract). §6 audit item A4 makes
  the one dependency that could break this (disc-side images pointing at
  string ADDRESSES) a pre-adoption check, with the name-keyed evidence
  (DVDOpen/GVR/PRS all key by string CONTENT) as the working answer.

## 5. HG-2 golden sketch (builder-lane buildable; nothing runs in this lane)

Regime = HG-1's: `host/bringup` test TU linked on `crt0_nomain.o` +
`hg1_windows.o`, insert-only `run.sh` block after the HG-1 lines. Legs:

* **HG2.0 Stage + bind.** Before stage: `pso_njs_handle_g2h(0x80080000u)`
  fails loud (test mode: reject + NULL) — the unbound-window precondition leg
  (mirrors HG1-I0e). After `pso_static_tree_stage()`:
  `pso_handle_window_records() == 4`; anchor[3] spans byte-exact
  (guest begin/end, `arena_off`, host base derives to `pso_arena_base +
  0x80000`); same-triple re-stage/re-bind = 0 added rejects.
* **HG2.1 Byte-exact resolution (THE acceptance leg).** For EVERY k in 0..81
  (census order, §2 table index): (a) VALUE LAW — the live static's slot reads
  back exactly `0x80080000u + 32u*k` (extern the rewritten arrays
  `effect_textures`/`volopt_monitor_texture_entries`/`mc_select_*_texname`, or
  read via `pso_st_expected[k][]` constexpr table generated alongside the
  blob — the golden asserts BOTH formula and per-name table so an index-order
  drift trips); (b) CROSSING — `pso_njs_handle_g2h(slot) == pso_arena_base +
  0x80000u + 32u*k` (reloc-relative, dev-D1 honest); (c) CONTENT —
  `strcmp(host, "<that census string>") == 0` AND every byte after NUL to the
  cell end is 0 (no alias/overflow); row 81 resolves to 32 zero bytes
  (null-name cell). Assert count 81×3 + 3 ≥ 246; green ⇒ "after loader regs,
  every static-init'd slot resolves through g2h byte-exact".
* **HG2.2 Consumer-leg.** Chain-walk like `TEffect.cpp:98`: stamp a
  throwaway POOL record's `textures` slot via `pso_handle_stamp_once` at
  `arena+0x34040` pointing at a fabricated 2-entry mini-texlist whose filename
  slots hold `PSO_ST_SLOT(0)`/`PSO_ST_SLOT(54)`
  (`NJS_HANDLE(NJS_TEXNAME, …)[i]` then `NJS_HANDLE(char, tn->filename)` ==
  the staged `"f008_white_npr01"`/`"ki_u256_vo_mo_moji001"` bytes) — proves
  texlist→texname→string all resolve through the SAME registry the 81 sites
  rely on, without touching batch-3's files. Optional era-semantics extension
  leg: memmove a shorter name INTO cell 79's staged bytes (the
  `replace_gvm_texture_directory` write path) — no reject fires, slot value
  unchanged, bytes updated (writable-arena era fidelity).
* **HG2.3 NEGATIVE CANARY (exactly one).** `g2h(0x80081000u)` — the static
  tree's EXCLUSIVE END, a main-RAM-dressed value covered by NO record: must
  trip `HG1-REJECT`, bump `pso_handle_reject_count`, return NULL in test mode,
  then the contrast leg re-resolves `PSO_ST_SLOT(0)` clean (HG1-J pattern).
  Why this canary is the right one: the legacy AR-shadow single-base body
  (`pso_njs_g2h` = base + slot − 0x80000000) would resolve `0x80081000` to a
  live arena byte SILENTLY — the canary fails green only on the registry-
  backed resolver, so it pins that the golden rode the window contract, not
  the fallback arithmetic, and it bounds the tree against off-by-one span
  drift (a 4 KB→4 KB+32 span bug trips it).
* **Gate (HG-1 §5 discipline).** exit 0; prior run.sh lines verbatim green
  (HG-1's 29 included); planted-reject arithmetic exact (HG1-E style: count
  every expected reject); waiver sha re-hashed pre/post, == pin
  `d4382fcd0659357f…`, zero regen; two back-to-back runs byte-identical on
  the reloc-free legs; insert-only `run.sh`/`.mk` diffs.

## 6. Sequencing, boundaries, audits (no double-count)

1. **Builder lane** lands anchor row + `pso_st_blob` + stage + HG-2
   (`host/bringup/**` — NOT this lane). HG-2 green is the gate for ANY site
   edit.
2. **Authoring lane** (upstream-track, script route per its own brief) rewrites
   the 81 initializers in the four decomp TUs to `PSO_ST_SLOT(idx)`. Today the
   4 files stay exactly as AQP2 left them (3 of them in the 139-file FAIL set;
   CameraCapture in batch-1's residual set) — this doc flips ZERO corpus units;
   LP64 ledger 1,167/513 untouched; AQP2's 81/13 HP-vs-batch-3 split is
   inherited unchanged (the 13 links are NOT W-STATIC-TREE sites and their
   hooks may need blob-adjacent thought later — see A3).
3. **Audit items the builder owes before slot-value freeze:**
   A1 re-run NA §4 screen on the 4 edited TUs (row count must not grow — §4);
   A2 re-run the arena collision audit (crt0/ladder/docs changed?);
   A3 confirm the batch-3 hooks for the 4 adjacent TEXLIST link slots
   (`CameraCapture.cpp:91/94/97`, `TObjBoss2Tunnel.cpp` family,
   `TNECFontEORFunction.cpp:39`, Volopt `:104`, MC `:21/:15`) still land in
   the shipped shape — noting their STORE TARGETS (the static arrays
   themselves) are host `.data`: if a later corpus wants the ARRAY slots
   guest-resident too, that is a W-STATIC-TREE tranche-2 (extension of this
   span to the table blobs), explicitly OUT of scope here;
   A4 the era-absolute question (§4 note): verify no disc-loaded image or POF0
   stream holds pointers at the era string addresses; current evidence is
   name-KEYED loading (DVDOpen `.GVR`, PRS `"%s.prs"`, GVM archive header
   records) — string CONTENTS are the keys, the addresses are not referenced.
4. **Untouched law:** NULL law, stamp-at-genesis ledger, slot-space
   comparison, 4-byte slots/translate-never-widen, C-TU ban (all authored
   sites are `.cpp`), `-Icompat/njs*` never on `.c`, per-TU allow-list
   integrator-owned. compat/njs/handle/** and static/ stay INERT: no `-I`,
   no references, names kept aligned so migration lanes swap bodies, never
   spellings.

## Repro

`$TMP/sta1/` (this lane, fresh files): `sites81.json` (81-row census: file,
line, string, shape, class — python regex over the era sources, 81/81 match),
`census_table.md` (§2 table verbatim), `samples.json` (rewritten-site
patterns), `sta1_shape.h` + `sta1_probe.{c,cpp}` (self-contained shape probe:
blob decl, `PSO_ST_SLOT` law, five constexpr byte-truths incl.
`PSO_ST_SLOT(3) == 0x80080060`, span-fit assert, all four per-TU init shapes;
`clang -std=c11` and `-std=c++17 -fsyntax-only` → rc=0 both; the only
diagnostics are pre-existing-style `-Wmissing-braces` union-init noise in the
probe's placeholder third fields, zero related to the slot arithmetic).
Numbers quoted from same-turn disk reads; decomp HEAD `6b38b5383` verified
`git rev-parse --short` this turn; lane writes exactly the two files named in
the header.
