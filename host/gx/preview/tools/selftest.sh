#!/bin/sh
# P2-PREVIEW end-to-end self-test against gx_window (docs/p2-preview-1.md).
# Proves BOTH branches and logs both outcomes:
#   A) capture-green: >=3 labeled PNGs at /tmp/preview-selftest/ + md5s
#   B) preflight-known-mode: PREVIEW_FORCE_NO_TCC=1 must fire the loud TCC
#      message with its exact fix instructions (exit 4)
# Exit 0 only if the run yields artifacts from A or a correct B firing, with
# B ALWAYS exercised (the known-mode branch must never rot silently).
set -e
B=$(cd "$(dirname "$0")/.." && pwd)
OUT=${OUT:-/tmp/preview-selftest}
REP="$OUT/SELFTEST.md"
rm -rf "$OUT"; mkdir -p "$OUT"
{ echo "# P2-PREVIEW self-test $(date '+%Y-%m-%d %H:%M:%S %Z')"; echo; } > "$REP"

echo "== branch B (preflight known-mode — always exercised) =="
set +e
PREVIEW_FORCE_NO_TCC=1 OUTDIR="$OUT" "$B/preview.sh" window 1 \
  > "$OUT/preflight-known-mode.out" 2> "$OUT/preflight-known-mode.err"
BRC=$?
set -e
if [ $BRC -eq 4 ] && grep -q "SCREEN RECORDING (TCC) NOT GRANTED" "$OUT/preflight-known-mode.err" \
   && grep -q "Privacy & Security" "$OUT/preflight-known-mode.err"; then
  BVERDICT="PASS — exit 4 + loud fix instructions fired"
else
  BVERDICT="FAIL — rc=$BRC (expected 4 with loud banner)"
fi
echo "- branch B: $BVERDICT" >> "$REP"

echo "== branch A (capture-green vs gx_window, 3 iterations) =="
set +e
OUTDIR="$OUT" "$B/preview.sh" window 3 > "$OUT/capture-green.out" 2>&1
ARC=$?
set -e
NPNG=$(ls "$OUT"/window-iter*.png 2>/dev/null | wc -l | tr -d ' ')
{ echo "- branch A: rc=$ARC, labeled PNGs=$NPNG (need >=3 for PASS)"; echo; } >> "$REP"
if [ -d "$OUT" ]; then
  if [ $ARC -eq 0 ] && [ "$NPNG" -ge 3 ]; then AVERDICT="PASS"; else AVERDICT="see rc/artifact count above"; fi
else
  AVERDICT="FAIL"
fi
echo '```' >> "$REP"; (cd "$OUT" && md5 window-iter*.png 2>/dev/null) >> "$REP"
(cd "$OUT" && md5 window-iter*.png 2>/dev/null | sed 's/^MD5 (\([^)]*\)).*/\1/') | while read -r p; do
  file "$OUT/$p"
done >> "$REP" || true
echo '```' >> "$REP"; echo >> "$REP"

echo "verdict: branchA=$AVERDICT branchB=$BVERDICT (report: $REP)" >> "$REP"
cat "$REP"
# PASS rule: known-mode branch B must fire correctly; branch A either
# produces >=3 PNGs (capture-green context) or dies LOUD through the
# preflight/known-mode path (TCC-blind context) — never a silent zero.
case "$BVERDICT" in PASS*) : ;; *) exit 1 ;; esac
if [ "$NPNG" -ge 3 ] && [ $ARC -eq 0 ]; then
  echo "SELFTEST PASS: capture-green branch A ($NPNG PNGs) + known-mode branch B both proven"
  exit 0
fi
if [ $ARC -eq 4 ]; then
  echo "SELFTEST PASS: context is TCC-blind -> branch A rode the known-mode preflight (loud, not silent); branch B proven"
  exit 0
fi
exit 1
