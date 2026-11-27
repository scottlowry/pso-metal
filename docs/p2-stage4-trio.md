# P2-STG4 — stage-4 trio: apply-8 residuals reproduced, PORT-SIDE fix-shape achieved (INERT PROPOSAL)

Fleet wave-5 lane STG4, card `t_d8c7aeea`, 2026-11-27. ZERO repo builds; decomp
`pso-decomp @6b38b5383` READ-ONLY; `compat/njs/**` READ-INTAKE — the shadow
patches below are PROPOSED paste-ready blocks (integrator lands), nothing
applied lane-side. Writes: this doc only.

## 0. Shape + era (same-turn disk)

- Corpus state check first: `build/parse/summary.txt` =
  `== parse-lp64: TOTAL=1680 FAIL=513 PASS=1167 (target=arm64-apple-macos12.0)`
  — **LP64 apply-8 shape**, per card expectation (not the ILP32 leg).
- Waiver era: `build/waivers/pso_assert_waivers.h` sha256 `d4382fcd0659357f…`
  measured == pin `host/bringup/waiver-header.sha256` PRE sweep, and
  `d4382fcd0659357f…` POST sweep — zero regen, zero bump.
- Fresh whole scripts (not re-used): `cache/scratch/stg4/{stg4_probe.sh,
  stg4_sweep.sh, stg4_sweep813.sh}` — official PARSE_RECIPE legs replicated
  verbatim (compat/waiver/pin `-include` trio order, `-DPSO_PORT_LAYOUT_WAIVER`,
  net→repin→facade→pshim→bringup `-I` order, facade before bringup;
  `-target arm64-apple-macos12.0 -ferror-limit=0`; `-Icompat/njs` first on
  allow-listed `.cpp` per Makefile NJS_LEG; musyx/`.c` never). Pool `-P6`.
- `build/njs_allowlist.txt` rows python-counted: **813 live rows, 813 `.cpp`,
  0 `.c`** (stage-1 trio 3 + stage-2 live 2 + stage-3 808). Official-corpus
  set-diff: **810 PASS / 3 FAIL at landed shape, and the 3 fails ARE the trio**
  — exactly this card's residuals.

## 1. Repro (fresh sweep, landed shadow shape `BASE`)

Trio first-errors reproduce the p2-tail-2 §1 hand-off row exactly, and the
counts equal the official `build/parse/err/` logs error-for-error:

| file | n | sites × class (apply-8 measured) |
|---|---|---|
| src/pso/render/GinjaMaterial.cpp | 3 | :367 :388 :421 — `subscripted value is not an array, pointer, or vector` (slot-subscript over `NJS_TEXLIST.textures` u32, then `.texture->` chain) |
| src/pso/render/GinjaTextureTable.cpp | 2 | :20 :24 — `incompatible pointer to integer conversion assigning to 'u32' from 'char *'` (h2g-store into `NJS_TEXNAME.filename`) |
| src/pso/render/psoLoadTexture.cpp | 3 | :89 member reference on u32 (g2h-load `.filename`); :145 `NJS_TEXNAME*`→u32 and :149 `char*`→u32 assigns (h2g-store) |

Total **8 errors, all pointer↔slot class**. NC §4's apply-7-era extras are gone
at apply-8: GinjaMaterial's +3 GXVert asm-dialect residuals and psoLoadTexture's
+1 packet_classes residual no longer fire (measured: 3 and 3, none naming those
classes). All 8 are expression-seam sites — unreachable by per-TU macro supply
or a new adapter header without editing the TU body (that's the excluded
source-overlay class; batch-A/B ports NOT touched, NOT consumed).

## 2. Port-side fix-shape: `PSO_SLOT(T)` shadow-body extension (PROPOSED, inert)

One legal port-side shape reaches all 8 sites: make the retyped slots
conversion-carrying instead of raw u32. The proxy performs the SAME arena
arithmetic as the helpers (pointer→slot via `h2g`, slot→pointer via `g2h`), so
it is a correct-conversion proxy, not a cast laundering — but it IS silent at
the type system, which collides with AR §5 loud-failure law (NC §2: "headers
must not launder"). Card law ("shadow bodies … PORT-SIDE ONLY") takes the
proposal; **the integrator adjudicates law-vs-gate**. Default-OFF
`#ifdef PSO_NJS_SLOT_PROXY` keeps every existing leg byte-equivalent until the
Makefile NJS_LEG arm adds `-DPSO_NJS_SLOT_PROXY` (integrator-owned build edit;
supplies the macro only to the already-opted `.cpp` rows — `.c` ban untouched).

Paste-ready block 1 — `compat/njs/pso/system/NinjaTypes.h`, insert immediately
after the `#define NJS_HANDLE(T, slot)` line (measured scratch copy:
`cache/scratch/stg4/shadow/`, proposal.diff):

```diff
 #define NJS_HANDLE(T, slot) ((T *)(pso_njs_g2h(slot)))
 
+/* ---- P2-STG4 §2: slot proxy body (docs/p2-stage4-trio.md). Default-OFF
+ * gate: with -DPSO_NJS_SLOT_PROXY (Makefile NJS_LEG arm, integrator-owned)
+ * the retyped slots below convert via the same h2g/g2h arithmetic that
+ * batch-1 spelled per-site; without it nothing in this header changes. ---- */
+#if defined(__cplusplus) && defined(PSO_NJS_SLOT_PROXY)
+template <class T> struct pso_slot32 {
+    u32 v;
+    pso_slot32() = default;
+    pso_slot32(u32 x) : v(x) {}
+    template <class U> pso_slot32(U *p)
+        : v(p ? (u32)(PSO_NJS_GUEST_RAM_BASE +
+                      ((const unsigned char *)p - pso_arena_base))
+              : 0u) {}
+    operator u32() const { return v; }
+    template <class U> operator U *() const { return (U *)pso_njs_g2h(v); }
+    T *operator->() const { return (T *)pso_njs_g2h(v); }
+    T &operator[](u32 i) const { return ((T *)pso_njs_g2h(v))[i]; }
+};
+#define PSO_SLOT(T) pso_slot32<T>
+#else
+#define PSO_SLOT(T) u32
+#endif
```

Paste-ready block 2 — retype the three audited slots (same pattern the batch-1
adapters spelled explicitly; `PSO_NJS_BYTE_TRUTH` width/offset pins below each
line are UNTOUCHED and pass under the proxy — 4-byte member, verified by the
sweeps):

```diff
--- a/compat/njs/pso/system/NinjaTypes.h        (struct NJS_TEXLIST)
-    u32 textures; // P2-AR: was NJS_TEXNAME * — NJTL image slot, POF0-patched
+    PSO_SLOT(NJS_TEXNAME) textures; // P2-STG4: slot-proxy form when gated
--- a/compat/njs/pso/render/GinjaMaterialState.h (struct NJS_TEXNAME)
-  u32 filename; /* P2-AR: was char * — name-string image handle (POF0 slot) */
+  PSO_SLOT(char) filename; /* P2-STG4: slot-proxy form when gated */
-    u32 texture;      /* P2-AR: was GinjaTextureState * — runtime state handle */
+    PSO_SLOT(GinjaTextureState) texture; /* P2-STG4: slot-proxy form when gated */
```

`texaddr`/`global_index` stay raw u32 (no site in the trio or the 813-list
needs a proxy there at parse level; union width unchanged).

## 3. Measurement (fresh sweeps, this lane, same-turn)

- **Trio at proxy shape** (`VAR`, full variant): `GinjaMaterial.cpp CLEAN`,
  `GinjaTextureTable.cpp CLEAN`, `psoLoadTexture.cpp CLEAN` — 8 → **0** errors,
  zero port-side… zero SOURCE-side edits; decomp untouched.
- **Regression guard, 60-file sample** (44 landed stage-3 rows, python-strided,
  + batch-1's 16 verified-clean files, per card): BASE = 44 CLEAN / 16 FAIL
  (the 16 are exactly the un-landed batch-1 adapters incl. the trio 3/2/3);
  VAR = **60/60 CLEAN. Landed-row regressions (clean→fail): 0.** Side finding:
  the other 13 batch-1 files also flip CLEAN without their source diffs.
- **FULL 813-row list at proxy shape** (`VAR813`): **813 CLEAN / 0 FAIL** —
  the 808 landed rows + stage-2 pair unaffected; trio flips. Expected official
  effect if the integrator lands §2 + the Makefile gate: LP64 ledger
  1,167→1,170 PASS / 513→510 FAIL (integrator's battery certifies, not me).
- Receipts: `cache/scratch/stg4/{NJS,BASE,VAR,VAR813}/err/*.err`,
  `BASE_results.txt`, `VAR_results.txt`, `VAR813_results.txt`, `sample60.txt`,
  `sample_meta.json`, `rows813.txt`, `proposal.diff`.

## 4. Disposition — clean-without-source-overlay vs parked

- **With §2 landed:** all three (`GinjaMaterial.cpp`, `GinjaTextureTable.cpp`,
  `psoLoadTexture.cpp`) go CLEAN **without any source-overlay**; batch-1 §3.7/
  §3.8/§3.11 trio diffs become superseded-at-parse (they remain valid for
  explicitness if a future wave wants loud spellings; the other-class tails
  NC §4 listed for them are already gone at apply-8 — §1).
- **Without §2 (integrator upholds AR §5 launder-ban):** all three stay
  PARKED behind the batch-1 source-side adapter landings — no partial port-side
  win exists: none of the 8 sites is a macro-reachable spelling (all are
  expression-level type mismatches inside era body text).
- Allow-list row state — the trio rows are already live stage-1 rows; propose
  the annotation update (paste-ready block 3, `build/njs_allowlist.txt`):

```diff
-# stage 1 (rung-5 logo trio, per §3 #1-#3):
+# stage 1 (rung-5 logo trio, per §3 #1-#3): stage-4 (P2-STG4
+# docs/p2-stage4-trio.md, d4382fcd era): 3/2/3 residuals measured at apply-8;
+# go 0-error iff the PSO_NJS_SLOT_PROXY shadow gate lands (§2/§3, 813/813),
+# else stay loud-cluster rows parked behind batch-1 §3.7/§3.8/§3.11 adapters.
```

## 5. Inertness

Repo writes: `docs/p2-stage4-trio.md` only (this commit's `git show --stat`).
`compat/njs/**` untouched (variant lives in lane scratch); `build/`, `host/`,
`data/`, decomp, disc untouched; zero make/gen_waivers (pin re-hashed pre/post,
equal); no numbers reused across shapes; set-math in python, disk reads
same-turn.
