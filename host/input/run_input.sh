#!/bin/sh
# P2-RDPREP step-1 gate runner: build + run the hg3_input canary suite.
# GATE-CHAIN rc law: rc from the redirected log, never from a pipe.
set -e
B=$(cd "$(dirname "$0")" && pwd)
R=$(cd "$B/../.." && pwd)
L=${PSO_INPUT_LOG:-/tmp/pso_input_run.log}
echo "== hg3_input: build seat (waiver verify-only, zero regen) =="
make -C "$B" -f input.mk all >"$L" 2>&1; rc=$?
if [ $rc -ne 0 ]; then tail -30 "$L"; echo "hg3_input: BUILD FAILED rc=$rc"; exit $rc; fi
echo "== hg3_input: C0 era-idle promotion canary (+HG-287 default-decline rider), C1 compaction, C2 edge law, C3 mirror pump-only =="
"$R/build/input/bin/input_test" >>"$L" 2>&1; rc=$?
tail -8 "$L"
[ $rc -eq 0 ] || { echo "hg3_input: TEST FAILED rc=$rc"; exit $rc; }
echo "hg3_input: EXIT=0"
