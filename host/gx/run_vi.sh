#!/usr/bin/env bash
# pso-metal P2-AB gate — logo VI progressive-branch port test.
# exit 0 ONLY when both branch assertions pass, across all three selection
# routes: in-process scenarios (dtv0/dtv1p1/dtv1p0), argv single-shots, and
# the compile-flag binary. Control-flow proof; pixel proof stays in run_window.sh.
set -euo pipefail
cd "$(cd "$(dirname "$0")/../.." && pwd)"   # repo root
BIN=build/vi_branch/bin
LOG=build/vi_branch/run.log

make -f host/gx/vi_branch.mk

{
  echo "################ vi_branch: in-process scenarios ################"
  "$BIN/vi_branch"
  echo "################ vi_branch: argv dtv=0 ################"
  "$BIN/vi_branch" dtv=0
  echo "################ vi_branch: argv dtv=1 ################"
  "$BIN/vi_branch" dtv=1
  echo "################ vi_branch_flagdtv: compile-flag auto ################"
  "$BIN/vi_branch_flagdtv" auto
} | tee "$LOG"

# Hard assertions: every named PROOF line must have passed (not just exit 0).
for key in dtv0 dtv0/frame1 dtv1p1 dtv1p1/frame1 dtv1p0 dtv1p0/frame1 auto/frame1; do
  grep -q "^PROOF $key PASS$" "$LOG" || { echo "run_vi: GATE FAIL — missing 'PROOF $key PASS'"; exit 1; }
done
echo "run_vi: gate OK — DTV=0 disable+SEGA-logo branch and DTV=1 dialog branch both proven @ 6b38b5383"
