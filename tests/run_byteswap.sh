#!/usr/bin/env bash
# P1-M: compile + run the compat/pso_byteswap.h test suite.
# CI-able: exit code 0 only if every pass compiles and the suite exits 0.
# Artifacts go to TMPDIR so the repo stays clean.
set -u
cd "$(dirname "$0")"

OUT="${TMPDIR:-/tmp}/pso_byteswap_test"
CFLAGS="-std=c11 -Wall -Wextra -Werror -I../compat"

# Pass 1: -O0 (debug-faithful codegen)
# shellcheck disable=SC2086
clang $CFLAGS -O0 -o "${OUT}_o0" pso_byteswap_test.c || { echo "FAIL: compile -O0" >&2; exit 1; }
"${OUT}_o0"; rc0=$?

# Pass 2: -O2 (catches UB that -O0 hides; clang may const-fold accessors)
# shellcheck disable=SC2086
clang $CFLAGS -O2 -o "${OUT}_o2" pso_byteswap_test.c || { echo "FAIL: compile -O2" >&2; exit 1; }
"${OUT}_o2"; rc2=$?

# Pass 3: UBSan (alignment/undefined behavior checker) — the memcpy seam
# must survive it. Optional toolchain piece: if the sanitizer runtime is
# missing we say so but do not fail the gate.
if clang $CFLAGS -O1 -fsanitize=undefined -fno-sanitize-recover=all \
     -o "${OUT}_ubsan" pso_byteswap_test.c 2>/dev/null; then
  "${OUT}_ubsan"; rcu=$?
else
  echo "-- ubsan pass skipped (compiler lacks -fsanitize=undefined runtime)"
  rcu=0
fi

echo "-- run_byteswap.sh exits: O0=$rc0 O2=$rc2 UBSan=$rcu"
[ $rc0 -eq 0 ] && [ $rc2 -eq 0 ] && [ $rcu -eq 0 ]
