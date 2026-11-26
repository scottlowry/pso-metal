#!/bin/bash
# Compat-shim probe: LP64 vs ILP32 with pso_metal_compat force-included.
cd /Users/scott/Hermes/pso-decomp/pso-decomp || exit 1
DECOMP=/Users/scott/Hermes/pso-decomp/pso-decomp
SC=/Users/scott/.hermes/profiles/pso-decomp/cache/scratch/clangprobe
COMPAT=/Users/scott/Hermes/pso-metal/compat/pso_metal_compat.h
INCS="-include $COMPAT -Iinclude -Iinclude/clib -Ilibs/dolphin/include -Ilibs/dolphin/include/dolphin -Isrc"
run() {
  TAG="$1"; shift
  OUT="$SC/$TAG"; rm -rf "$OUT"; mkdir -p "$OUT"
  probe() {
    f="$1"; safe=$(echo "$f" | tr / _)
    case "$f" in
      *.cpp) clang++ -fsyntax-only -std=c++17 -w $FLAGS $INCS "$f" > "$OUT/$safe.err" 2>&1 ;;
      *.c)   clang   -fsyntax-only -std=c11   -w $FLAGS $INCS "$f" > "$OUT/$safe.err" 2>&1 ;;
    esac
    [ $? -ne 0 ] && echo "$f" >> "$OUT/fail.txt"
    return 0
  }
  export -f probe; export OUT INCS FLAGS
  : > "$OUT/fail.txt"
  cat "$SC/cpp.txt" "$SC/c.txt" | xargs -P8 -I@ bash -c 'probe "@"'
  total=$(cat "$SC/cpp.txt" "$SC/c.txt" | wc -l | tr -d ' ')
  fail=$(wc -l < "$OUT/fail.txt" | tr -d ' ')
  echo "== $TAG: TOTAL=$total FAIL=$fail PASS=$((total-fail))"
  grep -h "error:" "$OUT"/*.err 2>/dev/null | sed 's/.*error: //' | cut -c1-64 | sort | uniq -c | sort -rn | head -8
}
FLAGS="-fms-extensions" run "lp64"
# WHY -ffreestanding: decomp declares the GC entry point as 'void main(void)'; clang's
#   hosted-mode "'main' must return 'int'" check is a host conformance rule, not a real
#   parse error — freestanding mode is also the semantically honest mode for console code.
# WHY -Wno-c++11-narrowing: 'case -1:' on a u32 switch-selector is valid MWCC wrap-around
#   (matches 0xFFFFFFFF); clang emits it as a default-error warning, suppress is sound.
FLAGS="-fms-extensions -target arm64_32-apple-watchos7.0 -ffreestanding -Wno-c++11-narrowing" run "ilp32"
