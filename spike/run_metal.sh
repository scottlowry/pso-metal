#!/usr/bin/env bash
# P1-K: compile + run the headless Metal offscreen clear spike.
# CI-able: exit code is the spike's own (0 = pass). Artifacts go to TMPDIR
# so the repo stays clean.
set -u
cd "$(dirname "$0")"

OUT="${TMPDIR:-/tmp}/metal_clear"
LOG="${TMPDIR:-/tmp}/metal_clear.log"

SDK="$(xcrun --sdk macosx --show-sdk-path)" || { echo "FAIL: no macOS SDK" >&2; exit 1; }

# shellcheck disable=SC2086
clang -fobjc-arc -Wall -Werror \
  -isysroot "$SDK" \
  -framework Foundation -framework Metal \
  -o "$OUT" metal_clear.m || { echo "FAIL: compile" >&2; exit 1; }

"$OUT" 2>&1 | tee "$LOG"
# tee masks the binary's exit code; recover it from PIPESTATUS
rc="${PIPESTATUS[0]}"
echo "-- metal_clear exit: $rc | log: $LOG"
exit "$rc"
