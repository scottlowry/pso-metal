#!/usr/bin/env python3
"""P1-L FAIL-CLOSED waiver generator.

Generates build/waivers/pso_assert_waivers.h, a force-included header that
neutralizes ONLY the LAYOUT_ASSERT instances belonging to types whose verdict
in data/assert-verdicts.tsv is exactly ``RUNTIME-WAIVE``.

Safety contract (see docs/p1-buildsystem.md):
  * TSV absent                      -> no-op header + 'no verdicts yet' notice.
  * verdict RUNTIME-WAIVE           -> that type's asserts may be waived.
  * verdict FORMAT-KEEP             -> NEVER waived (hard guard; not matchable
                                       by any code path other than the
                                       exact-string RUNTIME-WAIVE check).
  * verdict UNREVIEWED/unknown/other-> NEVER waived (compile stays honest).
  * A type with conflicting verdict rows is kept (fail-closed) with a warning.
  * Every LAYOUT_ASSERT name found in the tree gets an explicit flag (0=keep,
    1=waive). If the generator misses a name, the dispatched macro expands to
    an undeclared token and the unit FAILS to compile -> a waiver can never
    silently swallow an unknown assert, and the header must be regenerated
    whenever the decomp tree gains asserts (make regenerates it every run).

Mechanism: the decomp's include/types.h defines LAYOUT_ASSERT unguarded, so a
plain -include header cannot win against it. Instead, when at least one waiver
is active, this header #includes <types.h> FIRST (its TYPES_H guard then makes
every later include by the sources a no-op), #undefs LAYOUT_ASSERT and installs
a per-name dispatch macro. When zero waivers are active the header is empty:
byte-for-byte the Phase-0 probe behavior.

Usage:  python3 build/gen_waivers.py [--decomp ROOT] [--tsv PATH] [--out DIR]
"""
from __future__ import annotations

import argparse
import os
import re
import sys
from pathlib import Path

BUILD_DIR = Path(__file__).resolve().parent
REPO = BUILD_DIR.parent
DEFAULT_DECOMP = Path(os.environ.get(
    "DECOMP_ROOT", Path.home() / "Hermes/pso-decomp/pso-decomp"))
DEFAULT_TSV = REPO / "data" / "assert-verdicts.tsv"

VERDICT_WAIVE = "RUNTIME-WAIVE"   # the ONLY waivable verdict, exact match

# LAYOUT_ASSERT(<name>, <cond>)  — direct call sites (not the #define itself)
SITE_RE = re.compile(
    r"LAYOUT_ASSERT\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*,(.*)$", re.S)
# permissive type extraction from the condition
SIZEOF_RE = re.compile(r"sizeof\s*\(\s*([A-Za-z_][\w:<>~]*)")
OFFSETOF_RE = re.compile(r"offsetof\s*\(\s*([A-Za-z_][\w:<>~]*)\s*,")
# helper: #define H(args) LAYOUT_ASSERT(Name##param, offsetof(TYPE,param)==off)
HELP_RE = re.compile(
    r"#define\s+(\w+)\s*\(([^)]*)\)\s+LAYOUT_ASSERT\s*\(\s*([A-Za-z_]\w*(?:\s*##\s*\w+)?)"
    r"\s*,\s*(?:offsetof|sizeof)\s*\(\s*([A-Za-z_][\w:]*)")



def scan_tree(decomp: Path):
    """Return (names, name_types, helper_names, unresolved_helpers).

    names          : set of every concrete LAYOUT_ASSERT name found.
    name_types     : name -> type whose layout it checks (may be None).
    """
    names: dict[str, str | None] = {}
    helpers = {}  # macro -> (name_tmpl, param_index, TYPE, file)
    texts = {}
    for sub in ("include", "src"):
        root = decomp / sub
        if not root.is_dir():
            continue
        for p in root.rglob("*"):
            if p.suffix not in (".h", ".hpp", ".c", ".cpp"):
                continue
            try:
                text = p.read_text(encoding="utf-8", errors="replace")
                # join backslash-continuations so multi-line LAYOUT_ASSERT
                # helpers/call-sites are visible to the single-line regexes
                text = text.replace("\\\n", " ")
                texts[p] = text
            except OSError:
                continue

    for path, text in texts.items():
        for m in HELP_RE.finditer(text):
            macro, params, name_tmpl, typ = m.groups()
            if "##" not in name_tmpl:
                continue
            paste = re.search(r"(\w+)\s*##\s*(\w+)", name_tmpl)
            if not paste:
                continue
            plist = [x.strip() for x in params.split(",")]
            if paste.group(2) not in plist:
                continue
            helpers[macro] = (name_tmpl, plist.index(paste.group(2)), typ)
        # direct (non-#define) sites
        for line in text.splitlines():
            s = line.strip()
            if s.startswith("#define"):
                continue
            m = SITE_RE.match(s)
            if not m:
                continue
            name, cond = m.group(1), m.group(2).split("*/")[0]
            tm = SIZEOF_RE.search(cond) or OFFSETOF_RE.search(cond)
            names.setdefault(name, tm.group(1) if tm else None)

    unresolved = []
    for path, text in texts.items():
        for macro, (tmpl, idx, typ) in helpers.items():
            call_re = re.compile(
                r"\b" + re.escape(macro) + r"\s*\(([^()]*)\)")
            for m in call_re.finditer(text):
                args = [a.strip() for a in m.group(1).split(",")]
                if len(args) <= idx or not re.fullmatch(r"\w+", args[idx]):
                    unresolved.append((macro, m.group(0)[:60], str(path)))
                    continue
                name = tmpl.replace("##", "")
                # reconstruct: prefix before ## + substituted param
                prefix = tmpl.split("##")[0].strip()
                name = prefix + args[idx]
                names[name] = typ
    return names, unresolved


def read_verdicts(tsv: Path):
    """Return (waived_types, conflicts) or None when the TSV is absent."""
    if not tsv.is_file():
        return None
    waived, keepers, conflicts = set(), set(), set()
    seen_types = {}
    with tsv.open(encoding="utf-8") as fh:
        header = fh.readline().rstrip("\n").split("\t")
        try:
            tcol, vcol = header.index("type"), header.index("verdict")
        except ValueError:
            print(f"gen_waivers: ERROR: {tsv} lacks type/verdict columns; "
                  "treating as ABSENT (fail-closed).", file=sys.stderr)
            return None
        for ln, raw in enumerate(fh, start=2):
            cols = raw.rstrip("\n").split("\t")
            if len(cols) <= max(tcol, vcol):
                continue
            typ, verdict = cols[tcol].strip(), cols[vcol].strip().upper()
            # HARD GUARD: FORMAT-KEEP has absolute precedence and NO waiver path.
            # Lane I emits verdict labels like "RW (RUNTIME-WAIVE)" /
            # "L-P (FORMAT-KEEP)"; we match the canonical markers as substrings,
            # FORMAT-KEEP first. Anything else (UNREVIEWED/CONFLICT/unknown)
            # stays KEEP.
            if "FORMAT-KEEP" in verdict:
                keepers.add(typ)
                continue
            if "RUNTIME-WAIVE" in verdict:
                if typ in keepers or "FORMAT-KEEP" in seen_types.get(typ, verdict):
                    conflicts.add(typ)
                seen_types.setdefault(typ, verdict)
                if seen_types[typ] != verdict:
                    conflicts.add(typ)
                else:
                    waived.add(typ)
    waived -= keepers | conflicts
    return waived, conflicts


NOOP_HEADER = """\
/* GENERATED by build/gen_waivers.py — DO NOT EDIT.
 * STATE: no active assert waivers (verdicts absent or no RUNTIME-WAIVE rows).
 * This no-op header keeps the parse baseline byte-equivalent to Phase 0. */
#ifndef PSO_ASSERT_WAIVERS_H
#define PSO_ASSERT_WAIVERS_H
#define PSO_ASSERT_WAIVER_ACTIVE 0
#endif
"""

TEMPLATE = """\
/* GENERATED by build/gen_waivers.py — DO NOT EDIT.
 * STATE: {nwaive} waived LAYOUT_ASSERT(s) over {ntypes} RUNTIME-WAIVE type(s),
 * {nkeep} keep-flags. Every other verdict (FORMAT-KEEP/UNREVIEWED/unknown)
 * stays KEEP: its assert still fails the compile if the layout is wrong. */
#ifndef PSO_ASSERT_WAIVERS_H
#define PSO_ASSERT_WAIVERS_H
#define PSO_ASSERT_WAIVER_ACTIVE 1

/* Force the real types.h to be processed now, under its TYPES_H guard, so the
 * later #include "types.h" in decomp sources is a no-op and cannot clobber the
 * LAYOUT_ASSERT dispatch installed below. */
#include <types.h>

#undef LAYOUT_ASSERT

#define PSO_LA_KEEP(nm, cond) typedef char nm[(cond) ? 1 : -1]
/* Waived case: a harmless zero-cost TU-local definition; consumes the ';'. */
#define PSO_LA_NOP(nm, cond)  static int pso_la_waived_##nm = 0

/* Dispatch: PSO_LA_FLAG_<nm> must be generated for every assert name.
 * If it is missing, PSO_LA_ACT_PSO_LA_FLAG_<nm> is not a macro and the unit
 * fails to compile — unknown asserts can NEVER be silently waived. */
#define LAYOUT_ASSERT(nm, cond) PSO_LA_PICK(PSO_LA_FLAG_##nm)(nm, cond)
#define PSO_LA_PICK(f)  PSO_LA_PICK_(f)
#define PSO_LA_PICK_(x) PSO_LA_ACT_##x
#define PSO_LA_ACT_0(nm, cond) PSO_LA_KEEP(nm, cond)
#define PSO_LA_ACT_1(nm, cond) PSO_LA_NOP(nm, cond)

/* ---- per-name flags (0 = KEEP / assert honest, 1 = WAIVED) ---- */
{flags}
#endif
"""


def main():
    ap = argparse.ArgumentParser(description="P1-L fail-closed waiver generator")
    ap.add_argument("--decomp", type=Path, default=DEFAULT_DECOMP)
    ap.add_argument("--tsv", type=Path,
                    default=Path(os.environ.get(
                        "ASSERT_VERDICTS_TSV", str(DEFAULT_TSV))))
    ap.add_argument("--out", type=Path, default=BUILD_DIR / "waivers")
    args = ap.parse_args()

    out = args.out
    out.mkdir(parents=True, exist_ok=True)
    header = out / "pso_assert_waivers.h"

    if not args.decomp.is_dir():
        sys.exit(f"gen_waivers: DECOMP_ROOT {args.decomp} not found — "
                 "cannot enumerate assert names (fail-closed).")

    names, unresolved = scan_tree(args.decomp)
    if unresolved:
        for macro, snippet, f in unresolved[:10]:
            print(f"gen_waivers: WARN: unresolved helper invocation {snippet} "
                  f"in {f} — its assert name stays unwarned and fail-closed.",
                  file=sys.stderr)

    verdicts = read_verdicts(args.tsv)
    if verdicts is None:
        header.write_text(NOOP_HEADER)
        print("gen_waivers: NO VERDICTS YET — data/assert-verdicts.tsv absent; "
              f"wrote no-op waiver header ({out}). Asserts stay fully active.")
        (out / "waiver-status.txt").write_text("ABSENT\n")
        return

    waived_types, conflicts = verdicts
    for typ in sorted(conflicts):
        print(f"gen_waivers: WARN: conflicting verdict rows for {typ} — "
              "kept (fail-closed).", file=sys.stderr)

    if not waived_types:
        header.write_text(NOOP_HEADER)
        print(f"gen_waivers: verdicts present ({len(names)} assert names "
              "enumerated) but zero RUNTIME-WAIVE types; no-op waiver header.")
        (out / "waiver-status.txt").write_text("NO-WAIVABLE-TYPES\n")
        return

    flags = []
    nwaive = 0
    for nm in sorted(names):
        w = names[nm] in waived_types
        nwaive += w
        flags.append(f"#define PSO_LA_FLAG_{nm} {1 if w else 0}")
    header.write_text(TEMPLATE.format(
        nwaive=nwaive, nkeep=len(names) - nwaive, ntypes=len(waived_types),
        flags="\n".join(flags)))
    (out / "waiver-status.txt").write_text(
        f"ACTIVE waived={nwaive} types={len(waived_types)}\n")
    print(f"gen_waivers: ACTIVE — waived {nwaive}/{len(names)} asserts for "
          f"types {' '.join(sorted(waived_types))}")


if __name__ == "__main__":
    main()
