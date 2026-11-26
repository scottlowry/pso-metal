#!/bin/bash
# arm64_32 (ILP32) syntax probe over the full pso-decomp tree
cd /Users/scott/Hermes/pso-decomp/pso-decomp || exit 1
SC=/Users/scott/.hermes/profiles/pso-decomp/cache/scratch/clangprobe
OUT="$SC/f32"; rm -rf "$OUT"; mkdir -p "$OUT"
INCS="-Iinclude -Iinclude/clib -Ilibs/dolphin/include -Ilibs/dolphin/include/dolphin -Isrc"
T="-target arm64_32-apple-watchos7.0"
probe() {
  f="$1"
  safe=$(echo "$f" | tr / _)
  case "$f" in
    *.cpp) clang++ -fsyntax-only -std=c++17 -w $T $INCS "$f" > "$OUT/$safe.err" 2>&1 ;;
    *.c)   clang   -fsyntax-only -std=c11   -w $T $INCS "$f" > "$OUT/$safe.err" 2>&1 ;;
  esac
  [ $? -ne 0 ] && echo "$f" >> "$OUT/fail.txt"
  return 0
}
export -f probe; export OUT INCS T
: > "$OUT/fail.txt"
cat "$SC/cpp.txt" "$SC/c.txt" | xargs -P8 -I@ bash -c 'probe "@"'
total=$(cat "$SC/cpp.txt" "$SC/c.txt" | wc -l | tr -d ' ')
fail=$(wc -l < "$OUT/fail.txt" | tr -d ' ')
echo "TOTAL=$total FAIL=$fail PASS=$((total-fail))"
grep -h "error:" "$OUT"/*.err 2>/dev/null | sed 's/.*error: //' | cut -c1-70 | sort | uniq -c | sort -rn | head -12
