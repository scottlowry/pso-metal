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

# FORMAT-ROOT DENY-LIST (integrator, 2026-11-26): byte-true record roots and format
# structs that must NEVER be waived even if the verdict TSV labels them RW — the TSV is
# machine-merged and can carry inverted labels (found: CharacterFile/GuildCardSaveData/
# SystemSaveData mislabeled RUNTIME-WAIVE by lane I's string-match, committing waived=1
# into the live build). Source of truth: FORMAT-KEEP tables in docs/p1-format-saves.md
# (card images) and docs/p1-format-net-disc.md (wire/disc images). Names are exact types.
DENY_FORMAT_ROOTS = {
    "CardSaveHeader", "CharacterFile", "CharacterFileSlot", "CharacterSaveRecord",
    "GuildCardSaveData", "SystemSaveData", "SymbolChatPreset", "TItemData",
    "NetworkInfoConfig", "GslArchiveEntry", "GvmTextureHeader", "GvrTextureChunk",
    "MfciFileEntry",
    # P2-AO stitch EDIT 1 (tranche2 LOCKED rows, disk-verified 2026-11-26):
    "TPlyCharData", "GuildCardEntry", "QuestBinHeader",
    "G_DropItem_DC_6x5F", "G_DropItem_PC_V3_BB_6x5F", "G_PlaySound_6xB2",
    # Integrator extension (AJ/resolutions-3 + AD/resolutions-1 positive
    # loaded-image verdicts; belt+braces vs any future label inversion):
    "NJS_OBJECT", "NJS_MKEY_F", "NJS_TEXNAME", "NJS_TEXLIST",
    "EnemySetRecord", "FloorObjectSetRow",
    "CardFileInfo", "NetworkInfoCardFile", "G_FloorEvent_6x67",
    "ChallengeClientRecords",
}
# X rule-correction carve-out: the NetworkInfoCipher* family stays RUNTIME-WAIVE
# per net-disc —4 despite the narrow whole-header LOCKED rule; no keeper row for
# it exists in any tranche/resolution (verified), and only the EXACT name
# NetworkInfoConfig above is denied. Keepers are collected solely on the
# "FORMAT-KEEP"/"LOCKED-FORMAT" marker substrings, never on family prefixes.

# LAYOUT_ASSERT(<name>, <cond>)  — direct call sites (not the #define itself)
SITE_RE = re.compile(
    r"LAYOUT_ASSERT\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*,(.*)$", re.S)
# permissive type extraction from the condition
SIZEOF_RE = re.compile(r"sizeof\s*\(\s*([A-Za-z_][\w:<>~]*)")
# apply-6: `sizeof(struct X)` — keyword must not be read as the type name
SIZEOF_KW_RE = re.compile(
    r"sizeof\s*\(\s*(?:struct|union|enum|class)\s+([A-Za-z_][\w:]*)")
OFFSETOF_RE = re.compile(r"offsetof\s*\(\s*([A-Za-z_][\w:<>~]*)\s*,")
# helper: #define H(args) LAYOUT_ASSERT(Name##param, offsetof(TYPE,param)==off)
HELP_RE = re.compile(
    r"#define\s+(\w+)\s*\(([^)]*)\)\s+LAYOUT_ASSERT\s*\(\s*([A-Za-z_]\w*(?:\s*##\s*\w+)*)"
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
            plist = [x.strip() for x in params.split(",")]
            # P2-AO step-0 fix: multi-## templates with literal segments
            # (NET_DISPATCH_OFFSET: type##_##member##_offset). The old code
            # required exactly one paste whose RHS was a param, so this
            # helper (and any type##_##member##suffix shape) was skipped.
            segs = [s.strip() for s in name_tmpl.split("##")]
            if not any(s in plist for s in segs):
                continue
            helpers[macro] = (segs, plist, typ)
        # direct (non-#define) sites
        # apply-6 adjudication: two extraction gaps kept legitimately-RW types
        # at flag 0 (bounded, verified corpus-wide):
        #  (1) `sizeof(struct X)` — keyword prefix ate the type (exactly 1 site
        #      tree-wide: NetworkRuntime.h:19 PppFrame; SIZEOF_KW_RE below);
        #  (2) macro-indirect const checks `LAYOUT_ASSERT(nm, MACRO == lit)`
        #      where the same-file #define is a pure layout expression
        #      (NetTcpState.h:159-160 TcpTcb_f204_check -> offsetof/sizeof
        #      TcpTcb; the ONLY ==literal-without-type site whose macro
        #      resolves; others stay unmapped and fail closed at flag 0).
        defines = {}
        for dm in re.finditer(r"#define\s+([A-Za-z_]\w*)\s+([^\n]+)", text):
            defines.setdefault(dm.group(1), dm.group(2))
        for line in text.splitlines():
            s = line.strip()
            if s.startswith("#define"):
                continue
            m = SITE_RE.match(s)
            if not m:
                continue
            name, cond = m.group(1), m.group(2).split("*/")[0]
            tm = (SIZEOF_KW_RE.search(cond) or SIZEOF_RE.search(cond)
                  or OFFSETOF_RE.search(cond))
            if not tm:
                mm = re.match(r"\s*([A-Za-z_]\w*)\s*==", cond)
                if mm and mm.group(1) in defines:
                    body = defines[mm.group(1)]
                    tm = (SIZEOF_KW_RE.search(body) or SIZEOF_RE.search(body)
                          or OFFSETOF_RE.search(body))
            names.setdefault(name, tm.group(1) if tm else None)

    unresolved = []
    for path, text in texts.items():
        for macro, (segs, plist, typ) in helpers.items():
            call_re = re.compile(
                r"\b" + re.escape(macro) + r"\s*\(([^()]*)\)")
            for m in call_re.finditer(text):
                ls = text.rfind("\n", 0, m.start()) + 1
                if text[ls:m.start()].lstrip().startswith("#define"):
                    continue  # the definition line: args are param names
                args = [a.strip() for a in m.group(1).split(",")]
                parts, ok = [], True
                for s in segs:
                    if s in plist:
                        i = plist.index(s)
                        if i >= len(args) or not re.fullmatch(r"\w+", args[i]):
                            ok = False
                            break
                        parts.append(args[i])
                    else:
                        parts.append(s)
                if not ok:
                    unresolved.append((macro, m.group(0)[:60], str(path)))
                    continue
                ctype = typ
                if typ in plist:  # generic helper: concrete type from call arg
                    i = plist.index(typ)
                    ctype = (args[i] if i < len(args)
                             and re.fullmatch(r"\w+", args[i]) else None)
                names["".join(parts)] = ctype
    return names, unresolved


def read_verdicts(tsvs):
    """Multi-source fail-closed verdicts (P2-AO stitch EDIT 2/3).

    Two passes so cross-file precedence is absolute: pass 1 collects KEEPERS
    on markers {"FORMAT-KEEP", "LOCKED-FORMAT"} across every present file;
    pass 2 collects RUNTIME-WAIVE; any RW type already in keepers is a
    CONFLICT and is never waived. Returns (waived, conflicts, per_source)
    or None only when ALL files are absent. Verdict column: 'verdict' or
    'new-verdict' (resolutions files).
    """
    present = [Path(p) for p in tsvs if Path(p).is_file()]
    if not present:
        return None

    def rows(tsv):
        with tsv.open(encoding="utf-8") as fh:
            header = fh.readline().rstrip("\n").split("\t")
            tcol = header.index("type") if "type" in header else None
            vcol = None
            for name in ("verdict", "new-verdict"):
                if name in header:
                    vcol = header.index(name)
                    break
            if tcol is None or vcol is None:
                print(f"gen_waivers: ERROR: {tsv} lacks type + "
                      "verdict/new-verdict column; SKIPPING (fail-closed).",
                      file=sys.stderr)
                return None
            out = []
            for raw in fh:
                cols = raw.rstrip("\n").split("\t")
                if len(cols) <= max(tcol, vcol):
                    continue
                out.append((cols[tcol].strip(), cols[vcol].strip().upper()))
        return out

    parsed = []
    for tsv in present:
        r = rows(tsv)
        if r is not None:
            parsed.append((tsv, r))

    keepers = set()
    keep_src: dict[str, int] = {}
    for si, (_, r) in enumerate(parsed):
        for typ, verdict in r:
            if "FORMAT-KEEP" in verdict or "LOCKED-FORMAT" in verdict:
                keepers.add(typ)
                keep_src[typ] = si
    waived, conflicts, rw_seen = set(), set(), set()
    override_ok: set[str] = set()
    overrides: list[str] = []
    for si, (_, r) in enumerate(parsed):
        for typ, verdict in r:
            if "RUNTIME-WAIVE" in verdict:
                rw_seen.add(typ)
                # Precedence law (integrator, per resolutions docs + AJ/NJS_POINT3COL
                # case): RESOLUTIONS files consume LAST and may override an earlier
                # LOCKED claim -> RW wins iff its source is strictly later than the
                # last keeper claim for the same type. Same-file and earlier-file
                # keepers still win (fail-closed). DENY_FORMAT_ROOTS are absolute.
                if typ in keepers:
                    if keep_src.get(typ, len(parsed)) >= si or typ in DENY_FORMAT_ROOTS:
                        conflicts.add(typ)
                        waived.discard(typ)
                        continue
                    override_ok.add(typ)
                    overrides.append(f"{typ}(keeper@{keep_src[typ]}->rw@{si})")
                waived.add(typ)
    leaked = rw_seen & DENY_FORMAT_ROOTS
    if leaked:
        print("gen_waivers: WARNING — TSV carried RUNTIME-WAIVE labels on FORMAT "
              f"roots {sorted(leaked)}; deny-list overrode. Fix the labels upstream.",
              file=sys.stderr)
    waived -= (keepers - override_ok) | conflicts | DENY_FORMAT_ROOTS
    for o in overrides:
        print(f"gen_waivers: NOTE — resolution-precedence override: {o} "
              "later RUNTIME-WAIVE beats earlier keeper (deny-roots immune).",
              file=sys.stderr)
    per_source = [(p.name, len(r)) for p, r in parsed]
    return waived, conflicts, per_source


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
    ap.add_argument("--tsv", type=Path, nargs="*", default=None)
    ap.add_argument("--out", type=Path, default=BUILD_DIR / "waivers")
    args = ap.parse_args()

    out = args.out
    out.mkdir(parents=True, exist_ok=True)
    header = out / "pso_assert_waivers.h"

    if not args.decomp.is_dir():
        sys.exit(f"gen_waivers: DECOMP_ROOT {args.decomp} not found — "
                 "cannot enumerate assert names (fail-closed).")

    tsvs = list(args.tsv) if args.tsv else []
    if not tsvs:
        env = os.environ.get("ASSERT_VERDICTS_TSV")
        if env:
            tsvs.append(Path(env))  # env honored as FIRST entry
        tsvs += [DEFAULT_TSV,
                 REPO / "data" / "verdict-tranche1.tsv",
                 REPO / "data" / "verdict-tranche2.tsv",
                 REPO / "data" / "verdict-tranche3.tsv"]
        tsvs += sorted((REPO / "data").glob("verdict-resolutions-*.tsv"))
        # resolutions LAST = override layer (plan EDIT 3)
    names, unresolved = scan_tree(args.decomp)
    if unresolved:
        for macro, snippet, f in unresolved[:10]:
            print(f"gen_waivers: WARN: unresolved helper invocation {snippet} "
                  f"in {f} — its assert name stays unwarned and fail-closed.",
                  file=sys.stderr)

    verdicts = read_verdicts(tsvs)
    if verdicts is None:
        header.write_text(NOOP_HEADER)
        print("gen_waivers: NO VERDICTS YET — data/assert-verdicts.tsv absent; "
              f"wrote no-op waiver header ({out}). Asserts stay fully active.")
        (out / "waiver-status.txt").write_text("ABSENT\n")
        return

    waived_types, conflicts, per_source = verdicts
    src_note = " ".join(f"{n}={c}" for n, c in per_source)
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
    print("gen_waivers: ACTIVE \u2014 waived "
          f"{nwaive}/{len(names)} asserts over "
          f"{len(waived_types)} types; sources: {src_note}")


if __name__ == "__main__":
    main()
