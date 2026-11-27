#!/usr/bin/env python3
"""P2-MA: regenerate the MWCC AT_ADDRESS dialect shadows under compat/repin.

Copies each era header VERBATIM from the read-only decomp tree
(pso-decomp @6b38b5383) and inserts exactly ONE preamble block immediately
before the file's first `AT_ADDRESS(` site — the neutralizer for the MWCC
absolute-address variable declarator (`T name AT_ADDRESS(addr);` where era
types.h spells `#define AT_ADDRESS(addr) : (addr)`; clang rejects the colon
form with "expected ';' after top level declarator"). Everything else is
era bytes, field-for-field. Re-run after any decomp-side upstream change
and re-check the pinned era line numbers below.

Usage: python3 gen_dialect_shadows.py [DECOMP_DIR]
Doc:   docs/p2-mwccasm-1.md. Never -include these files; -Icompat/repin only.
"""
import os
import sys

DECOMP = sys.argv[1] if len(sys.argv) > 1 else "/Users/scott/Hermes/pso-decomp/pso-decomp"
OUT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))  # compat/repin

PREAMBLE = """\
/* --- P2-MA MWCC-dialect preamble (shadow-only; docs/p2-mwccasm-1.md) ------
 * The era line below spells an MWCC/CodeWarrior absolute-address variable
 * binding:  `T name AT_ADDRESS(addr);`  with the era types.h spelling
 * `#define AT_ADDRESS(addr) : (addr)` (colon declarator). clang/C11/C++17
 * rejects it ("expected ';' after top level declarator" cascade). The pin is
 * a guest-linker contract with no clang parse-world analogue; host executing
 * builds route these names through D4 facades, so the macro is neutralized
 * empty here, per the P2-T precedent (compat/dialect/pso_dialect_compat.h
 * section 1: "a port defines the pin empty"). One neutralization serves the
 * whole TU: it also repairs era sites reached WITHOUT their own shadow
 * (os.h:125-136, OSLink.h:27-28, dsp/dsp.h:10) when any chain hits this file
 * first. Everything else below is era-VERBATIM. Guard name is era-identical,
 * so the era copy and this copy can never both process in one TU, and the
 * collision pin (compat/collision/pin_dolphin_types.h, which occupies the
 * _DOLPHIN_TYPES_H_ guard but never any header guard below) does not
 * suppress this file. */
#ifdef AT_ADDRESS
#undef AT_ADDRESS
#endif
#define AT_ADDRESS(addr) /* P2-MA: era address pin dropped host-side (parse world) */

"""

# (era path in decomp, shadow path under compat/repin, 1-based era line of first AT_ADDRESS site)
FILES = [
    ("libs/dolphin/include/dolphin/gx/GXVert.h", "dolphin/gx/GXVert.h", 25),
    ("libs/dolphin/include/dolphin/os/OSExec.h", "dolphin/os/OSExec.h", 25),
    ("libs/dolphin/include/dolphin/dvd.h", "dolphin/dvd.h", 196),
    ("include/pso/quest/QuestDownloadServer.h", "pso/quest/QuestDownloadServer.h", 134),
    # apply-6 integrator erratum: self-heal claim holds only for TUs that hit
    # a shipped shadow first; src/pso/system/RelModule.cpp includes OSLink.h
    # with no other shadowed header in its chain (measured: the ONE corpus
    # residual AT site after MA's four landed) -> own shadow.
    ("libs/dolphin/include/dolphin/os/OSLink.h", "dolphin/os/OSLink.h", 27),
]

for src, dest, site in FILES:
    with open(os.path.join(DECOMP, src), "r", encoding="utf-8", errors="strict") as fh:
        lines = fh.readlines()
    idx = next(i for i, l in enumerate(lines) if "AT_ADDRESS(" in l)
    if idx + 1 != site:
        sys.exit(f"STALE PIN: {src}: first AT_ADDRESS at line {idx+1}, generator says {site}")
    dst = os.path.join(OUT, dest)
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    with open(dst, "w", encoding="utf-8") as fh:
        fh.writelines(lines[:idx] + [PREAMBLE] + lines[idx:])
    print(f"{dest}: era {len(lines)} lines verbatim + preamble before :{site}")
