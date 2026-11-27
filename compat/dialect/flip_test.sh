#!/bin/bash
# P2-T prototype flip test: FAIL -> LP64-parse-PASS on the zero-layout-assert
# dialect-class pool (top bucket, docs/p2-dialect-triage.md §3). Probe flags only,
# no make, decomp read-only. BEFORE = exact P2-R run_p2r.sh flag set (uncapped).
DECOMP=/Users/scott/Hermes/pso-decomp/pso-decomp
REPO=$(cd "$(dirname "$0")/../.." && pwd)
SC=${TMPDIR:-/tmp}/p2t_flips
COMMON="-w -fms-extensions -ffreestanding -Wno-c++11-narrowing -ferror-limit=0 -include $REPO/compat/pso_metal_compat.h -include $REPO/build/waivers/pso_assert_waivers.h -Iinclude -Iinclude/clib -Ilibs/dolphin/include -Ilibs/dolphin/include/dolphin -Isrc"
# -I pshim MUST precede -I include / -I libs so the shadow headers win lookup
DIALECT="-I$REPO/compat/dialect/pshim -include $REPO/compat/dialect/pso_dialect_compat.h"
cd "$DECOMP" || exit 1
mkdir -p "$SC"; : > "$SC/results.txt"
while read -r f; do
  case "$f" in
    # musyx runtime .c files carry source-side extern "C" blocks (MWCC C dialect):
    # host build mode for that library is C++. pso .cpp: language unchanged.
    *.c)                 BEFORE="clang   -fsyntax-only -std=c11"; AFTER="clang++ -fsyntax-only -std=c++17 -x c++" ;;
    *)                   BEFORE="clang++ -fsyntax-only -std=c++17"; AFTER="clang++ -fsyntax-only -std=c++17" ;;
  esac
  $BEFORE $COMMON "$f" > "$SC/before.err" 2>&1; B=$?
  $AFTER  $DIALECT $COMMON "$f" > "$SC/after.err" 2>&1; A=$?
  echo "== $f BEFORE=$( [ $B -ne 0 ] && echo FAIL || echo PASS ) AFTER=$( [ $A -ne 0 ] && echo FAIL || echo PASS )" | tee -a "$SC/results.txt"
  [ $A -ne 0 ] && grep -m3 "error:" "$SC/after.err" | sed 's/^/   after: /' | tee -a "$SC/results.txt"
done
echo "FLIPS=$(grep -c 'AFTER=PASS' $SC/results.txt)/$(grep -c '^== ' $SC/results.txt)"
