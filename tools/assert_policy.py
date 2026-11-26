#!/usr/bin/env python3
"""P1-I assert-policy classifier.

Enumerates every static layout assert (``LAYOUT_ASSERT(name, cond)`` ->
``typedef char name[(cond) ? 1 : -1]``, defined in the decomp's
``include/types.h:69``) across the READ-ONLY pso-decomp tree, determines which
ones currently FAIL under the real Phase-1 runtime ABI (Apple clang, default
arm64 target = LP64) by compiling each assert-bearing file plus every src TU
with the pso-metal compat shim and harvesting compiler diagnostics at exact
assert lines, then cross-references each failing type against the sibling
format-disposition lanes' verdict docs (``docs/p1-format-saves.md``,
``docs/p1-format-net-disc.md``; absent/WIP -> UNREVIEWED).

Reproducible from the pso-metal repo root:

    python3 tools/assert_policy.py

Optional env:  PSO_DECOMP_ROOT (default /Users/scott/Hermes/pso-decomp/pso-decomp)
Flags:         --no-probe   reuse cached probe log (cache/assert-probe/diag.json)
               --jobs N     parallel clang jobs (default min(16, cpu))

Outputs:
    data/assert-verdicts.tsv   committed: per-type asserts/failing/verdict/evidence
    cache/assert-probe/        untracked: raw clang logs, diag.json, stats.json

Method note (how step 2 was decided): we COMPILE per-file probes under LP64,
we do NOT intersect docs/abi.md's failing list. abi.md measured the
arm64_32/ILP32 watchos target; Phase 1's real runtime is LP64, where pointer
and ``long`` width differences move far more layouts, so the abi.md list is
the wrong ground truth for this policy.
"""

from __future__ import annotations

import argparse
import concurrent.futures as cf
import json
import os
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent  # pso-metal repo root
DEFAULT_DECOMP = Path(os.environ.get(
    "PSO_DECOMP_ROOT", "/Users/scott/Hermes/pso-decomp/pso-decomp"))
COMPAT = ROOT / "compat" / "pso_metal_compat.h"
CACHE = ROOT / "cache" / "assert-probe"
VERDICT_DOCS = ["docs/p1-format-saves.md", "docs/p1-format-net-disc.md"]

NEG_SIZE = "declared as an array with a negative size"
DIAG_RE = re.compile(r"^(?P<f>.+?):(?P<l>\d+):\d+: (?:error|fatal error): (?P<msg>.*)$")
LA_RE = re.compile(r"\bLAYOUT_ASSERT\s*\(")
SIZE_RE = re.compile(r"^sizeof\(([A-Za-z_][\w:<>~]*)\)==([-+0-9A-Fa-fxX]+)$")
OFF_RE = re.compile(r"^offsetof\(([A-Za-z_][\w:<>~]*),([^()]+)\)==([-+0-9A-Fa-fxX]+)$")
KEEP_RE = re.compile(r"FORMAT[- _]KEEP|\bL-P\b")
WAIVE_RE = re.compile(r"RUNTIME[- _]WAIVE|\bRW\b")


def _balanced(s: str) -> bool:
    d = 0
    for ch in s:
        if ch == "(":
            d += 1
        elif ch == ")":
            d -= 1
            if d < 0:
                return False
    return d == 0


def is_negsize(msg: str) -> bool:
    """Both clang spellings of a false (cond)?1:-1 typedef size: the named
    one for sizeof-asserts, the bare one for offsetof-asserts."""
    return ("declared as an array with a negative size" in msg
            or msg.strip() == "array size is negative")


def parse_asserts(decomp: Path):
    """Every LAYOUT_ASSERT call site in src/ + include/ (not the #define),
    including sites generated through per-header offset-helper macros such as
    ``#define CHAT_OFFSET(field, offset) LAYOUT_ASSERT(..., offsetof(T,field)==offset)``."""
    sites = []
    helpers = {}  # macro name -> (file, type, field_arg, offset_arg)
    DEF_RE = re.compile(
        r"^#define\s+(\w+)\(([^)]*)\)\s+LAYOUT_ASSERT\(\s*"
        r"([A-Za-z0-9_]*)(?:##([A-Za-z0-9_]+))?([A-Za-z0-9_]*)\s*,"
        r"\s*offsetof\((\w+)\s*,\s*(\w+)\)\s*==\s*(\w+)")
    for sub in ("include", "src"):
        for path in sorted((decomp / sub).rglob("*")):
            if path.suffix not in (".h", ".hpp", ".c", ".cpp", ".cc"):
                continue
            try:
                text = path.read_text(errors="replace")
            except OSError:
                continue
            if "LAYOUT_ASSERT(" not in text:
                continue
            lines = text.splitlines()
            for i, line in enumerate(lines):
                dm = DEF_RE.search(line)
                if dm:
                    helpers[dm.group(1)] = {
                        "file": str(path.relative_to(decomp)), "type": dm.group(6),
                        "field_arg": dm.group(7), "offset_arg": dm.group(8),
                        "params": [p.strip() for p in dm.group(2).split(",")],
                        "paste_prefix": dm.group(3), "paste_suffix": dm.group(5),
                        "paste_arg": dm.group(4)}
                    continue
                if dm is None and "#define" in line and "LAYOUT_ASSERT(" in line \
                        and not re.match(r"^#define\s+LAYOUT_ASSERT\b", line.strip()):
                    print(f"WARNING unmapped assert-macro helper: "
                          f"{path.relative_to(decomp)}:{i+1}: {line[:100]}",
                          file=sys.stderr)
                for m in LA_RE.finditer(line):
                    if line.lstrip().startswith("#"):  # the #define itself
                        continue
                    seg, j, depth = line[m.end():], i, 1
                    while depth and j < len(lines):
                        for ch in seg:
                            if ch == "(":
                                depth += 1
                            elif ch == ")":
                                depth -= 1
                            if depth == 0:
                                break
                        if depth == 0:
                            break
                        j += 1
                        if j >= len(lines):
                            break
                        seg += "\n" + lines[j]
                    body = " ".join(seg[:seg.rfind(")")].split())
                    name, _, cond = body.partition(",")
                    cond = " ".join(cond.split())
                    # the tree sometimes wraps the condition in extra parens
                    while (cond.startswith("(") and cond.endswith(")")
                           and _balanced(cond[1:-1])):
                        cond = cond[1:-1].strip()
                    cond = re.sub(r"\s+", "", cond)
                    sm, om = SIZE_RE.match(cond), OFF_RE.match(cond)
                    if sm:
                        kind, typ, member, val = "size", sm.group(1), "", sm.group(2)
                    elif om:
                        kind, typ, member, val = "offset", om.group(1), om.group(2), om.group(3)
                    else:
                        kind, typ, member, val = "other", f"(const) {name.strip()}", "", ""
                    sites.append({
                        "file": str(path.relative_to(decomp)), "line": i + 1,
                        "name": name.strip(), "kind": kind, "type": typ,
                        "member": member, "value": val, "cond": cond,
                    })
    # Second pass: enumerate helper-macro usages (each is one offset assert).
    if helpers:
        use_re = re.compile(r"\b(" + "|".join(map(re.escape, helpers)) + r")\s*\(")
        for sub in ("include", "src"):
            for path in sorted((decomp / sub).rglob("*.h")):
                rel = str(path.relative_to(decomp))
                lines = path.read_text(errors="replace").splitlines()
                for i, line in enumerate(lines):
                    if line.lstrip().startswith("#") or "/*" in line:
                        continue
                    for m in use_re.finditer(line):
                        seg = line[m.end():]
                        depth, body = 1, ""
                        for ch in seg:
                            if ch == "(":
                                depth += 1
                            elif ch == ")":
                                depth -= 1
                                if depth == 0:
                                    break
                            body += ch
                        args = [a.strip() for a in body.split(",")]
                        if len(args) != 2:
                            print(f"WARNING odd helper args {rel}:{i+1}: {body[:60]}",
                                  file=sys.stderr)
                            continue
                        h = helpers[m.group(1)]
                        if h["paste_arg"]:
                            idx = h["params"].index(h["paste_arg"])
                            exp_name = (h["paste_prefix"] + args[idx]
                                        + h["paste_suffix"])
                        else:
                            exp_name = h["paste_prefix"]
                        sites.append({
                            "file": rel, "line": i + 1,
                            "name": exp_name, "kind": "offset",
                            "type": h["type"], "member": args[0], "value": args[1],
                            "cond": f"offsetof({h['type']},{args[0]})=={args[1]}",
                        })
    sites.sort(key=lambda s: (s["file"], s["line"]))
    return sites


INCLUDES = ["include", "include/clib", "libs/dolphin/include",
            "libs/dolphin/include/dolphin", "src"]


def compile_unit(decomp: Path, rel: str):
    """One clang -fsyntax-only probe. .h goes through clang++ (C++ header)."""
    is_c = rel.endswith(".c")
    exe = "clang" if is_c else "clang++"
    std = "-std=c11" if is_c else "-std=c++17"
    # LP64 flags = Phase-0 ILP32 flag set minus -target arm64_32 (default
    # host target IS arm64 LP64 here). See docs/abi.md §7 for the ILP32 run.
    cmd = [exe, "-fsyntax-only", std, "-ferror-limit=0", "-w",
           "-fms-extensions", "-ffreestanding", "-Wno-c++11-narrowing",
           "-include", str(COMPAT)] + \
          [f"-I{decomp}/{d}" for d in INCLUDES] + [str(decomp / rel)]
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=180,
                           cwd=decomp)
    except subprocess.TimeoutExpired:
        return rel, "TIMEOUT"
    log = CACHE / "logs" / (rel + ".err")
    log.parent.mkdir(parents=True, exist_ok=True)
    log.write_text(r.stdout + r.stderr)
    return rel, str(log)


def harvest(logpath: str, decomp: Path, wanted: set):
    """[(normfile, line, msg)] for diagnostics inside assert-bearing files."""
    hits = []
    if logpath == "TIMEOUT" or not Path(logpath).exists():
        return hits
    for ln in Path(logpath).read_text(errors="replace").splitlines():
        m = DIAG_RE.match(ln)
        if not m:
            continue
        f = m.group("f")
        if f.startswith("/"):
            try:
                f = os.path.relpath(f, decomp)
            except ValueError:
                pass
        if f in wanted:
            hits.append((f, int(m.group("l")), m.group("msg")))
    return hits


def probe(decomp: Path, sites, jobs: int):
    """Returns file -> {line: status} plus per-file derail diagnostics."""
    files = sorted(set(s["file"] for s in sites))
    units = files + [str(p.relative_to(decomp))
                     for p in (decomp / "src").rglob("*")
                     if p.suffix in (".c", ".cpp", ".cc")]
    units = sorted(set(units))
    assert_lines = {f: set(s["line"] for s in sites if s["file"] == f)
                    for f in files}
    wanted = set(files)
    status = defaultdict(dict)   # file -> line -> FAIL/INCONCLUSIVE
    healthy = defaultdict(bool)  # file -> some unit parsed it cleanly
    dirty_msgs = defaultdict(dict)  # file -> line -> first non-assert msg
    print(f"probing {len(units)} TUs under LP64 with {jobs} jobs ...")
    with cf.ThreadPoolExecutor(max_workers=jobs) as ex:
        for rel, logpath in ex.map(lambda u: compile_unit(decomp, u), units):
            diag = defaultdict(list)  # file -> [(line,msg)]
            for f, l, msg in harvest(logpath, decomp, wanted):
                diag[f].append((l, msg))
            for f, ds in diag.items():
                al = assert_lines.get(f, set())
                off = [(l, m) for l, m in ds if l not in al or not is_negsize(m)]
                if not off:  # healthy: only negative-size asserts complained
                    healthy[f] = True
                else:
                    for l, m in off:
                        dirty_msgs[f].setdefault(l, m)
                for l, m in ds:
                    if is_negsize(m):
                        status[f][l] = "FAIL"
                    else:
                        status[f].setdefault(l, "INCONCLUSIVE")
    return status, healthy, dirty_msgs


def load_verdicts(types):
    """type -> (verdict, evidence-doc) from sibling format lanes if present."""
    docs = {}
    for d in VERDICT_DOCS:
        p = ROOT / d
        if p.exists():
            docs[d] = p.read_text(errors="replace").splitlines()
    out = {}
    for t in types:
        if not t or t.startswith("(const)"):
            out[t] = ("N/A-CONSTANT", "")
            continue
        pat = re.compile(rf"\b{re.escape(t)}\b")
        keep = waive = None
        for d, lines in docs.items():
            for ln in lines:
                if not pat.search(ln):
                    continue
                if keep is None and KEEP_RE.search(ln):
                    keep = d
                if waive is None and WAIVE_RE.search(ln):
                    waive = d
        if keep and not waive:
            out[t] = ("L-P (FORMAT-KEEP)", keep)
        elif waive and not keep:
            out[t] = ("RW (RUNTIME-WAIVE)", waive)
        elif keep and waive:
            out[t] = ("CONFLICT", keep)
        else:
            out[t] = ("UNREVIEWED", "")
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--decomp", type=Path, default=DEFAULT_DECOMP)
    ap.add_argument("--jobs", type=int, default=min(16, os.cpu_count() or 4))
    ap.add_argument("--no-probe", action="store_true")
    ap.add_argument("--emit-waiver-list", action="store_true",
                    help="write cache/assert-probe/pso_layout_waiver_list.h "
                         "(selective waiver list for the compat/waive shim)")
    args = ap.parse_args()
    decomp = args.decomp.resolve()
    if not (decomp / "include" / "types.h").exists():
        sys.exit(f"decomp root not found: {decomp}")
    CACHE.mkdir(parents=True, exist_ok=True)
    sites = parse_asserts(decomp)
    print(f"parsed {len(sites)} LAYOUT_ASSERT call sites")

    diag_path = CACHE / "diag.json"
    if args.no_probe and diag_path.exists():
        blob = json.loads(diag_path.read_text())
        status = {f: {int(l): s for l, s in v.items()}
                  for f, v in blob["status"].items()}
        healthy = set(blob["healthy"])
    else:
        status, healthy_map, dirty_msgs = probe(decomp, sites, args.jobs)
        healthy = set(f for f, ok in healthy_map.items() if ok)
        diag_path.write_text(json.dumps({
            "status": {f: {str(l): s for l, s in v.items()}
                       for f, v in status.items()},
            "healthy": sorted(healthy),
            "dirty": {f: dirty_msgs[f] for f in dirty_msgs},
        }))

    def site_status(s):
        st = status.get(s["file"], {}).get(s["line"])
        if st == "FAIL":
            return "FAIL"
        if s["file"] in healthy:
            return "PASS"
        return st or "UNPROBED"  # file derailed everywhere / not compiled

    counts = defaultdict(int)
    for s in sites:
        counts[site_status(s)] += 1
    print("assert status:", dict(counts))

    per_type = defaultdict(lambda: {"asserts": 0, "failing": 0, "inconclusive": 0,
                                    "unprobed": 0, "first": ""})
    for s in sites:
        st = site_status(s)
        t = per_type[s["type"]]
        t["asserts"] += 1
        t["failing"] += st == "FAIL"
        t["inconclusive"] += st == "INCONCLUSIVE"
        t["unprobed"] += st == "UNPROBED"
        if not t["first"]:
            t["first"] = f"{s['file']}:{s['line']}"
    failing_types = sorted(k for k, v in per_type.items() if v["failing"])
    verdicts = load_verdicts(failing_types)

    rows = []
    for t, v in sorted(per_type.items(), key=lambda kv: (-kv[1]["failing"], kv[0])):
        if v["failing"]:
            verdict, evid = verdicts[t]
        else:
            verdict, evid = "NOT-AT-FAULT", ""
        rows.append((t, v["asserts"], v["failing"], verdict,
                     evid or v["first"]))
    out = ROOT / "data" / "assert-verdicts.tsv"
    out.parent.mkdir(exist_ok=True)
    with out.open("w") as fh:
        fh.write("type\tasserts\tfailing\tverdict\tevidence-file\n")
        for r in rows:
            fh.write("\t".join(str(x) for x in r) + "\n")
    print(f"wrote {out} ({len(rows)} types)")

    stats = {
        "total_asserts": len(sites),
        "assert_files": len(set(s["file"] for s in sites)),
        "kinds": {k: sum(1 for s in sites if s["kind"] == k)
                  for k in ("size", "offset", "other")},
        "assert_status": dict(counts),
        "types_total": len(per_type),
        "types_failing": len(failing_types),
        "types_inconclusive_only": sum(
            1 for v in per_type.values()
            if not v["failing"] and v["inconclusive"]),
        "verdict_buckets": {
            b: sum(1 for t in failing_types if verdicts[t][0] == b)
            for b in ("L-P (FORMAT-KEEP)", "RW (RUNTIME-WAIVE)",
                      "CONFLICT", "N/A-CONSTANT", "UNREVIEWED")},
        "docs_present": {d: (ROOT / d).exists() for d in VERDICT_DOCS},
        "failing_types": [
            {"type": t, "failing": per_type[t]["failing"],
             "asserts": per_type[t]["asserts"], "verdict": verdicts[t][0],
             "first": per_type[t]["first"]} for t in failing_types],
    }
    (CACHE / "stats.json").write_text(json.dumps(stats, indent=2))
    if args.emit_waiver_list:
        # Selective waiver list for the compat/waive/types.h shim (see
        # docs/assert-policy.md): PSO_LA_<assert-name> = 1 only for asserts on
        # types whose format-lane verdict is RW (RUNTIME-WAIVE). L-P,
        # UNREVIEWED, CONFLICT and constant sites stay 0 => assert stays live.
        name_waived = {}
        for s in sites:
            st = site_status(s)
            v = verdicts.get(s["type"], ("NOT-AT-FAULT",))[0] if st == "FAIL" else ""
            name_waived[s["name"]] = name_waived.get(s["name"], False) or (
                st == "FAIL" and v == "RW (RUNTIME-WAIVE)")
        wl = CACHE / "pso_layout_waiver_list.h"
        with wl.open("w") as fh:
            fh.write("/* GENERATED by tools/assert_policy.py --emit-waiver-list.\n"
                     " * 1 = layout assert waived (RUNTIME-WAIVE verdict only).\n"
                     " * Re-run the tool after adding/removing LAYOUT_ASSERTs. */\n"
                     "#ifndef PSO_LAYOUT_WAIVER_LIST_H\n#define PSO_LAYOUT_WAIVER_LIST_H\n")
            for n in sorted(name_waived):
                fh.write(f"#define PSO_LA_{n} {1 if name_waived[n] else 0}\n")
            fh.write("#endif\n")
        n1 = sum(name_waived.values())
        print(f"wrote {wl}: {len(name_waived)} names, {n1} waived")
    print(json.dumps({k: v for k, v in stats.items() if k != "failing_types"},
                     indent=2))


if __name__ == "__main__":
    main()
