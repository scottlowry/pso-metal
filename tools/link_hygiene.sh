#!/bin/bash
# P2-AF link-hygiene gate — nm-level assertions over a (scratch) musyx object set.
# Converts the one-time P2-AA audit (docs/p2-musyx-linknames.md §5) into a reusable
# build gate. Companion data: data/musyx-linknames.tsv (27 rows: 24 func crossers +
# 3 synth_dbtab.c data tables). Read-only over <obj-dir>; writes only its own
# mktemp scratch. Bash 3.2-safe (macOS stock), absolute-path safe (REPO resolved
# from this script's location; CWD is never assumed).
#
# Usage: tools/link_hygiene.sh <obj-dir> [--musyx] [--tsv <file>]
#
#   <obj-dir>  directory scanned recursively for *.o (Mach-O objects)
#   --musyx    <obj-dir> contains ONLY musyx-owned objects -> assertion (a) is the
#              hard zero-__Z rule (any C++-mangled defined global fails)
#   (default)  mixed first-link battery -> assertion (a) flags only __Z symbols
#              whose demangled identifier is a golden name (pso C++ __Z are legal)
#   --tsv F    golden TSV override (default: <repo>/data/musyx-linknames.tsv)
#
# Assertions (a/b are gatekeeping; c is advisory):
#   (a) zero __Z-prefixed C++-mangled symbols among musyx-defined globals
#   (b) every golden name defined: func rows as exact `_name` type T,
#       data rows as exact `_name` type D/B/S/R — mangled twins count as ABSENT
#   (c) duplicate T definitions (same symbol type-T in >=2 objects) -> WARNINGS + count
#
# Exit: 0 iff (a) AND (b) pass | 1 assertion failure | 2 usage/environment error
set -u

usage() { echo "usage: $0 <obj-dir> [--musyx] [--tsv <file>]" >&2; exit 2; }

REPO=$(cd "$(dirname "$0")/.." && pwd)
TSV="$REPO/data/musyx-linknames.tsv"
STRICT=0 OBJ=""
while [ $# -gt 0 ]; do
  case "$1" in
    --musyx)   STRICT=1 ;;
    --tsv)     shift; [ $# -gt 0 ] || usage; TSV=$1 ;;
    -h|--help) usage ;;
    -*)        usage ;;
    *)         { [ -z "$OBJ" ] || usage; OBJ=$1; } ;;
  esac
  shift
done
[ -n "$OBJ" ] || usage
[ -d "$OBJ" ] || { echo "link_hygiene: no such directory: $OBJ" >&2; exit 2; }
[ -f "$TSV" ] || { echo "link_hygiene: golden TSV not found: $TSV" >&2; exit 2; }
command -v nm >/dev/null 2>&1 || { echo "link_hygiene: nm not found" >&2; exit 2; }

SC=$(mktemp -d "${TMPDIR:-/tmp}/p2af_gate.XXXXXX") || exit 2
trap 'rm -rf "$SC"' EXIT

MODE=$( [ "$STRICT" -eq 1 ] && echo musyx-strict || echo golden-scope )

# ---- 1. defined-global table: name \t TYPE \t objrel ----
find "$OBJ" -name '*.o' | sort > "$SC/objs.txt"
: > "$SC/defined.txt"
while IFS= read -r o; do
  nm -g --defined-only "$o" 2>/dev/null | awk -v obj="${o#"$OBJ"/}" '
    NF==3 && $2 ~ /^[A-Z]$/ { printf "%s\t%s\t%s\n", $3, $2, obj }' >> "$SC/defined.txt"
done < "$SC/objs.txt"
NOBJ=$(wc -l < "$SC/objs.txt" | tr -d ' ')
NSYM=$(wc -l < "$SC/defined.txt" | tr -d ' ')

echo "== link_hygiene $MODE: $OBJ"
echo "   objs=$NOBJ defined_globals=$NSYM tsv=$TSV"
if [ "$NOBJ" -eq 0 ]; then
  echo "SUMMARY objs=0 mangled=0 golden=0/0 dupT=0 verdict=FAIL[b]"; exit 1
fi

# ---- 2. (a) __Z scan ----
# name \t obj per distinct (__Z name, obj); golden-scope mode demangles via c++filt
# and keeps only symbols whose demangled identifier is a golden name.
awk -F'\t' '$1 ~ /^__Z/ {print $1 "\t" $3}' "$SC/defined.txt" | sort -u > "$SC/z.txt"
cut -f1 "$SC/z.txt" | sort -u > "$SC/znames.txt"
if [ -s "$SC/znames.txt" ] && [ "$STRICT" -eq 0 ]; then
  c++filt < "$SC/znames.txt" > "$SC/zdem.txt"
  paste -d'\t' "$SC/znames.txt" "$SC/zdem.txt" > "$SC/zpairs.txt"
  awk -F'\t' '
      NR == FNR { if (FNR > 1 && $1 != "") want[$1] = 1; next }
      { for (g in want)
          if ($2 ~ ("(^|[^A-Za-z0-9_])" g "([^A-Za-z0-9_]|$)")) { print $1; next } }
    ' "$TSV" "$SC/zpairs.txt" | sort -u > "$SC/zviol_names.txt"
elif [ -s "$SC/znames.txt" ]; then
  cp "$SC/znames.txt" "$SC/zviol_names.txt"   # musyx-strict: every __Z name violates
else
  : > "$SC/zviol_names.txt"
fi
sort -u "$SC/zviol_names.txt" -o "$SC/zviol_names.txt"
NZA=$(wc -l < "$SC/znames.txt" | tr -d ' ')
NA=$(wc -l < "$SC/zviol_names.txt" | tr -d ' ')

echo "-- (a) C++-mangled defined globals: $NA violation(s) (of $NZA distinct __Z)"
head -10 "$SC/zviol_names.txt" | while IFS= read -r zn; do
  objs=$(awk -F'\t' -v s="$zn" '$1==s {printf "%s ", $2}' "$SC/z.txt")
  dem=$(awk -F'\t' -v s="$zn" '$1==s {print $2; exit}' "$SC/zdem.txt" 2>/dev/null)
  echo "   FAIL a: $zn  [$objs]${dem:+  dem: $dem}"
done
[ "$NA" -gt 10 ] && echo "   ... (+$((NA-10)) more)"

# ---- 3. (b) golden presence ----
# For each TSV row, the exact Mach-O name `_<symbol>` must be defined with a
# kind-appropriate type: func -> T, data -> D|B|S|R.
awk -F'\t' -v tsv="$TSV" -v zfile="$SC/zviol_names.txt" '
  NR == FNR { if (FNR > 1 && $1 != "") { GK[$1] = $2; NG++ } next }
  { t[$1] = t[$1] $2 "," ; o[$1] = o[$1] $3 " " }
  END {
    nf = 0; nd = 0; okf = 0; okd = 0
    for (g in GK) {
      nm_ = "_" g; kinds = GK[g]
      hit = 0; found = ""
      if (nm_ in t) {
        n = split(t[nm_], ty, ",")
        for (i = 1; i <= n; i++) {
          ok = (kinds == "func") ? (ty[i] == "T") : (ty[i] ~ /^[DBSR]$/)
          if (ok) { hit = 1; break }
          found = found ty[i] " in " o[nm_]
        }
      }
      if (kinds == "func") { nf++; if (hit) okf++ } else { nd++; if (hit) okd++ }
      if (!hit) printf "MISS\t%s\t%s\t%s\n", g, kinds, \
        (found != "" ? "wrong-type:" found : "absent")
    }
    printf "OKCNT\t%d\t%d\t%d\t%d\n", okf, nf, okd, nd
  }
' "$TSV" "$SC/defined.txt" > "$SC/golden.raw"
NB=$(awk -F'\t' '$1=="MISS"' "$SC/golden.raw" | wc -l | tr -d ' ')
GOKF=$(awk -F'\t' '$1=="OKCNT"{print $2}' "$SC/golden.raw")
GNF=$(awk -F'\t' '$1=="OKCNT"{print $3}' "$SC/golden.raw")
GOKD=$(awk -F'\t' '$1=="OKCNT"{print $4}' "$SC/golden.raw")
GND=$(awk -F'\t' '$1=="OKCNT"{print $5}' "$SC/golden.raw")

echo "-- (b) golden names: func ${GOKF}/${GNF} as T, data ${GOKD}/${GND} as D/B/S/R"
head -10 "$SC/golden.raw" | awk -F'\t' '$1=="MISS" {printf "   FAIL b: %s (%s): %s\n", $2, $3, $4}'
[ "$NB" -gt 10 ] && echo "   ... (+$((NB-10)) more)"

# ---- 4. (c) duplicate T definitions (advisory) ----
awk -F'\t' '$2=="T" {c[$1]++; o[$1]=o[$1] " " $3} END {for (n in c) if (c[n]>1) printf "%d\t%s\t%s\n", c[n], n, o[n]}' \
  "$SC/defined.txt" | sort -k2,2 > "$SC/dups.txt"
NC=$(wc -l < "$SC/dups.txt" | tr -d ' ')
echo "-- (c) duplicate T definitions: $NC symbol name(s) [WARNING only]"
head -10 "$SC/dups.txt" | awk -F'\t' '{printf "   WARN c: %s x%s (%s)\n", $2, $1, $3}'
[ "$NC" -gt 10 ] && echo "   ... (+$((NC-10)) more)"

# ---- 5. verdict ----
VERDICT=PASS
[ "$NA" -gt 0 ] && VERDICT=FAIL[a]
[ "$NB" -gt 0 ] && VERDICT=$( [ "$NA" -gt 0 ] && echo "FAIL[a+b]" || echo "FAIL[b]")
echo "SUMMARY objs=$NOBJ mangled=$NA/$NZA golden=${GOKF}/${GNF}+${GOKD}/${GND} dupT=$NC verdict=$VERDICT"
[ "$VERDICT" = PASS ] && exit 0
exit 1
