#!/usr/bin/env python3
"""P1-L parse regression gate.

Runs `make -C <repo>/build parse`, extracts the PASS count from the summary,
and FAILS (exit 1) if PASS < BASELINE. This proves the tree never drops below
Phase 0's achievement of 669 units parsing under the compat shim at the
arm64_32 parse target. Prints the delta either way.

    python3 tools/parse_gate.py            # gate at 669
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
BASELINE = int(os.environ.get("PSO_PARSE_BASELINE", "669"))
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
