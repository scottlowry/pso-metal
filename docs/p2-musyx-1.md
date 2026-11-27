# P2-MX — musyx C-dialect residual class: survey, inert compat supply, flip recipe

Phase-3 lane MX. Measured against `pso-decomp @ 6b38b5383` **read-only**, zero
repo builds; all numbers below are python-counted from same-turn scratch probe
logs (`$TMP/mx/{C0,CS,CP,CPS}/`, runner `mx_probe.sh`, corpus = the musyx rows
of the current official `build/parse/fail.txt`, 27 units). Waiver header
`build/waivers/pso_assert_waivers.h` re-hashed this session:
`c6b514035f39b633…` = the pin.

## 1. Intake reconciliation — measured N = 27 (not 26)

`docs/p2-collision-debt.md` §4.4 item 4 says 26; the CURRENT official fail.txt
carries **27** `src/musyx/runtime/**.c` rows (err/ dir has a 28th stale empty
`.err` for `synth_dbtab.c`, which parses clean today — no `extern "C"` and no
musyx.h include, confirmed by scratch C0 pass). Reproduced C0 census ==
official err census exactly: **56** ×`expected identifier or '('`, **30**
×`must use 'struct' tag` (26 `SoundReverb` + 4 `SoundReverbDelay`),
**2** ×fatal missing `orig_asm/*.inc`.

## 2. Sub-classes (per-file root tokens)

| ID | Token cluster | Site | Fix family |
|---|---|---|---|
| MX-A | bare `extern "C" {` (C++-only) in **vendor header** | `include/musyx/musyx.h:161` | shadow header (`-I` only) — `compat/musyx/musyx/musyx.h` |
| MX-B | bare `extern "C" {` in the **`.c` body itself** | 27 TUs (28 blocks; `synthvoice.c` ×2) | **NOT header-fixable** — kill = driver flip `clang++ -x c++` (P2-T), link-name safety pre-audited in [`p2-musyx-linknames.md`](p2-musyx-linknames.md): 24/24 crossers `dup`-safe, `cpp-only-if-def-not-C` = 0 |
| MX-C | unelaborated struct tags (`SoundReverb`, `SoundReverbDelay`) | `musyx_internal.h:13`, `pso/audio/SoundReverb.h:11,12,32` | era-truth scalar mirrors in shadows (`struct ` elaboration, linkage `#ifdef` guard, C-named param decl; zero layout change) |
| MX-D | MWCC `asm void f(…){ #include "orig_asm/…inc" }` | `Chorus/chorus_fx.c:113,117`, `StdReverb/reverb.c:86,90` | unfixable host-side: `asm`-keyword parse + the 4 `.inc` files **do not exist** (`orig_asm/` absent from repo) → genuine runtime-WAIVE / D4 facade (precedent: `compat/chorus/chorus_stand-in.c`, [`p2-chorus-standin.md`](p2-chorus-standin.md)) |
| MX-E | `AT_ADDRESS(0x800000F8)` colon-declarator data pin | `hw_dspctrl.c:277` (via `include/types.h:61` `#define AT_ADDRESS(addr) : (addr)`) | shadow `musyx_internal.h` `#undef AT_ADDRESS` right after `types.h` — neutralizes for every TU that includes it |

## 3. Compat supply (inert unless activated)

`compat/musyx/` (lane-owned, README inside): `musyx/musyx.h`,
`pso/audio/SoundReverb.h`, `musyx/runtime/musyx_internal.h` — faithful copies,
cited line-level diffs only. Activation = `-I compat/musyx` **ahead of**
`-I include`/`-I src`; never `-include`; no `compat/njs` on these `.c` TUs
(law honored; probes contain no njs path).

## 4. Probe shapes (python-counted receipts, `$TMP/mx/receipt.json`)

| shape | driver | stack | PASS | residual census |
|---|---|---|---:|---|
| C0 | clang c11 | compat+waiver+pin, repin/host `-I` (= official) | **0/27** | 28×extern-C(header), 30×bare-tag, 28×extern-C(body), 2×missing-inc |
| CS | clang c11 | C0 + `-I compat/musyx` | **0/27** | extern-C(header)→**0**, bare-tag→**0**; 28×extern-C(body) + 2×fatal remain = MX-B ceiling |
| CP | clang++ c++17 | same, no shadow | **24/27** | chorus_fx, reverb (MX-D), hw_dspctrl (MX-E) |
| CPS | clang++ c++17 | CP + `-I compat/musyx` | **25/27** | only MX-D: chorus_fx.c, reverb.c (4×asm-parse, 2×fatal-inc) |

Conclusion: the class-kill is **flip + shadow** (25/27); shadows alone are
provably capped at 0/27 (MX-B is source-side); the 2 MX-D files are the
lane's WAIVE/facade residue.

## 5. Paste-ready asks (integrator-owned lines — nothing below is applied by this lane)

Harness flag family for the 27 rows (adds to §5 full-stack shape of
`p2-collision-debt.md`; keeps the three-`-include` pin order untouched):

```
src/musyx/runtime/**/*.c  ->  clang++ -std=c++17
-I compat/musyx        # BEFORE -I include and -I src
```

Waiver rows (append to whatever ledger consumes; TSV):

```
unit	class	verdict	basis	ask
src/musyx/runtime/Chorus/chorus_fx.c	MX-D asm-in-body + orig_asm/Chorus/chorus_fx/{do_src1,do_src2}.inc absent	RUNTIME-WAIVE	CPS 25/27 probe, $TMP/mx/CPS/err	host-lane facade (chorus_stand-in precedent) or permanent parse-waive
src/musyx/runtime/StdReverb/reverb.c	MX-D asm-in-body + orig_asm/StdReverb/reverb/{DoCrossTalk,HandleReverb}.inc absent	RUNTIME-WAIVE	CPS 25/27 probe, $TMP/mx/CPS/err	same
```

Residual note for AA/AM ledger: after the flip, `synth_dbtab.c` (no
`extern "C"`, C-pure) keeps C linkage for its 3 data defs under clang++ (data
never mangles, AA §2) — no action.
