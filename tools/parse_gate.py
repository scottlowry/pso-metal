#!/usr/bin/env python3
"""P1-L parse regression gate.

Runs `make -C <repo>/build parse`, extracts the PASS count from the summary,
and FAILS (exit 1) if PASS < BASELINE. This proves the tree never drops below
the arm64_32 achievement. Prints the delta either way.

Baseline history: 669 (Phase-0 compat shim) -> 1416 -> 1,578 (arm64_32
achieved at apply-2) -> 1,576 (apply-5 adoption wave, 2026-11-27). The
current floor is the 1,578 anchor minus 2: stage-1 NJS allow-list TUs
(docs/p2-gx-tex-consumer.md §4) that passed ILP32 plain now pay the
enumerated u32-slot cluster while opted into the shadow. The collision pin +
rebind (docs/p2-collision-debt.md §5) is LP64-only: parse-lp64 measures the
adopted shape (162 -> 261, ledger in docs/p2-stitch-apply5.md); it is not
gated here.

    python3 tools/parse_gate.py            # gate at 1576
    PSO_PARSE_BASELINE=700 python3 tools/parse_gate.py
    make -C build parse  # (what the gate drives)

Exit codes: 0 = at/above baseline, 1 = regression (or gate could not run).
"""
from __future__ import annotations

import os
import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
BASELINE = int(os.environ.get("PSO_PARSE_BASELINE", "1576"))
SUMMARY_RE = re.compile(r"TOTAL=(\d+)\s+FAIL=(\d+)\s+PASS=(\d+)")


def main() -> int:
    proc = subprocess.run(
        ["make", "-C", str(REPO / "build"), "parse"],
        capture_output=True, text=True)
    out = proc.stdout + proc.stderr
    sys.stdout.write(out)
    if proc.returncode != 0:
        print(f"parse_gate: ERROR: `make parse` exited {proc.returncode}; "
              "cannot certify baseline.", file=sys.stderr)
        return 1
    m = None
    for m in SUMMARY_RE.finditer(out):
        pass  # keep the last match (summary line is printed last)
    if not m:
        print("parse_gate: ERROR: no `TOTAL=.. FAIL=.. PASS=..` summary line "
              "found in make output.", file=sys.stderr)
        return 1
    total, fail, passed = (int(x) for x in m.groups())
    delta = passed - BASELINE
    print(f"parse_gate: baseline={BASELINE} (Phase-0 achievement, "
          f"arm64_32 parse target)  actual={passed}/{total}  delta={delta:+d}")
    if passed < BASELINE:
        print(f"parse_gate: FAIL — regression of {BASELINE - passed} unit(s) "
              "below the Phase-0 parse baseline.", file=sys.stderr)
        return 1
    print(f"parse_gate: OK — at or above baseline"
          + (f" (${delta:+d} improvement)" if delta > 0 else " (no drift)." if delta == 0 else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
