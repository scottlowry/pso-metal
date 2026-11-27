#!/bin/sh
# P2-PREVIEW — owner preview tooling. One command to SEE any render, on
# demand, repeatedly (docs/p2-preview-1.md). ZERO edits to driver lanes:
# this script only INVOKES prebuilt binaries and captures their output.
#
# usage: preview.sh <window|dl|logo|logo-genuine> [iterations=3]
#   window        build/gxwindow/bin/gx_window — no dump hook exists, so proof
#                 frames come from CGWindowList window-id screencapture
#                 (Screen-Recording/TCC-gated -> PREFLIGHTED, see below)
#   dl            build/gxdl/bin/gx_dl         — GX_DL_DEBUG -> /tmp/dl_dump.ppm
#   logo          build/gxdl/bin/gx_logo       — GX_DL_DEBUG -> /tmp/logo_dump.ppm
#   logo-genuine  gx_logo + GX_LOGO_GVR/GVP from host/bringup/local_assets
#                 (fails CLOSED when local_assets is absent — by design,
#                 genuine disc art is gitignored and machine-local)
#
# env:  PREVIEW_HOLD=secs   re-open the driver back-to-back until the deadline
#                         (human-watch mode: the display sees each render again
#                         and again); overrides the iterations cap
#       OUTDIR=dir          PNG destination; default /tmp/preview-<subject>-<stamp>;
#                         MUST be outside the repo (proof frames never land in git)
#       PREVIEW_FORCE_NO_TCC=1   simulate the TCC-denied context to exercise
#                         the known-mode preflight branch (self-test branch B)
#
# exit: 0 green | 2 usage | 3 subject unavailable | 4 preflight TCC known-mode
#       5 driver binary missing | 1 iteration(s) produced no artifact
set -e
B=$(cd "$(dirname "$0")" && pwd)
R=$(cd "$B/../../.." && pwd)

SUBJECT=${1:-}
ITERS=${2:-3}
HOLD=${PREVIEW_HOLD:-0}
WINLIST="$R/build/gxpreview/bin/winlist"
PPM2PNG="$B/tools/ppm2png.py"
FAKE_DISC="$R/host/bringup/fake_disc"
LOGO_ASSETS="$R/host/bringup/local_assets"
case "$ITERS" in (''|*[!0-9]*) echo "preview: iterations must be a number" >&2; exit 2;; esac
case "$SUBJECT" in window|dl|logo|logo-genuine) ;; *)
  echo "usage: preview.sh <window|dl|logo|logo-genuine> [iterations=3]" >&2; exit 2;; esac

# OUTDIR: refuse any path inside the checkout (PNGs gitignored == PNGs kept
# out of the repo entirely).
OUTDIR=${OUTDIR:-/tmp/preview-$SUBJECT-$(date +%Y%m%d-%H%M%S)}
case "$OUTDIR" in "$R"/*) echo "preview: OUTDIR must live OUTSIDE the repo ($R): proof frames are never committable" >&2; exit 2;; esac
mkdir -p "$OUTDIR"
LOGF="$OUTDIR/preview.log"

log() { echo "preview[$SUBJECT] $*" | tee -a "$LOGF"; }

loud_no_tcc() {  # known-mode: the wave-5.5b daemon-context blindness, as a NAME, not a zero
  cat >&2 <<'EOF'
X==========================================================================X
X  PREVIEW PREFLIGHT FAILED — SCREEN RECORDING (TCC) NOT GRANTED           X
X                                                                          X
X  Window-id capture needs CGWindowList + screencapture -l<id> permission  X
X  for the PROCESS CONTEXT that launches preview.sh.                       X
X                                                                          X
X  FIX (owner GUI session):                                                X
X   1. System Settings -> Privacy & Security -> Screen and System Audio    X
X      Recording (older macOS: "Screen Recording")                         X
X   2. ENABLE the app hosting your shell (Terminal / iTerm / Warp / ...)   X
X   3. Fully QUIT and RELAUNCH that app (TCC caches per-process), rerun    X
X                                                                          X
X  KNOWN MODE: launchd/SSH/CI daemon contexts cannot hold this grant —     X
X  that is exactly the TCC-blindness seen fleet-side today. It now reads   X
X  as THIS message, never a silent zero. Capture-free subjects still work  X
X  there: the dump-hook paths (dl, logo, logo-genuine) need NO permission. X
X==========================================================================X
EOF
  log "PREFLIGHT FAIL (known mode): Screen Recording not granted to launching context (exit 4)"
  exit 4
}

# ---- subject setup: build helper if missing (own mk only), verify drivers ----
if [ "$SUBJECT" = window ]; then
  if [ ! -x "$WINLIST" ]; then
    log "winlist helper absent -> make -C host/gx/preview (this lane's only build)"
    make -C "$B" >/dev/null || { echo "preview: winlist build failed — run 'make -C host/gx/preview' to see the error" >&2; exit 5; }
  fi
  DRIVER="$R/build/gxwindow/bin/gx_window"
  [ -x "$DRIVER" ] || { log "driver binary missing: $DRIVER (gx_window is L7P3/Q's lane output — build it owner-side, e.g. host/gx/run_window.sh; this lane never builds drivers)"; exit 5; }
  [ -d "$FAKE_DISC" ] || { log "PSO_FAKE_DISC tree missing: $FAKE_DISC"; exit 5; }
  export PSO_FAKE_DISC="$FAKE_DISC"
  # ---- PREFLIGHT: real probe (or simulated denial) BEFORE any driver launch ----
  if [ "${PREVIEW_FORCE_NO_TCC:-0}" = 1 ]; then loud_no_tcc; fi
  # Try up to 5 on-screen windows: a writable per-window PNG is the real
  # proof that window-id capture works in this context (screencapture
  # refuses hidden dot-paths, so use a normal name and rm after).
  PREF_OK=0
  for PROBE_ID in $("$WINLIST" --all 2>/dev/null | head -5 | cut -d'|' -f1); do
    if screencapture -o -x -l"$PROBE_ID" "$OUTDIR/preflight-probe.png" 2>>"$LOGF" \
       && [ -s "$OUTDIR/preflight-probe.png" ]; then PREF_OK=1; PROBE_WON=$PROBE_ID; break; fi
  done
  rm -f "$OUTDIR/preflight-probe.png"
  if [ "$PREF_OK" != 1 ]; then
    log "preflight: NO on-screen window was capturable via screencapture -l (window-id capture dead in this context)"
    loud_no_tcc
  fi
  log "preflight PASS: window-id capture works in this context (probe window $PROBE_WON)"
elif [ "$SUBJECT" = dl ] || [ "$SUBJECT" = logo ] || [ "$SUBJECT" = logo-genuine ]; then
  if [ "$SUBJECT" = logo-genuine ]; then
    GVR="$LOGO_ASSETS/sinbd.gvr"
    GVP="$LOGO_ASSETS/hyouji_a.gvp"
    if [ ! -f "$GVR" ]; then
      log "logo-genuine UNAVAILABLE: $GVR not present (subject is conditional on local_assets by design)"
      echo "  Genuine disc-derived art is gitignored (fnv+dims receipts only — host/bringup/local_assets/)." >&2
      echo "  Land the sinbd GVR (and the f256_hyouji_a.gvp palette sidecar) there machine-side, or use the" >&2
      echo "  fixture regime: preview.sh logo   (exit 3; not a render failure)" >&2
      exit 3
    fi
    [ -f "$GVP" ] || log "WARN: palette sidecar $GVP absent — running genuine GVR with fixture CLUT; expect the logo hash readback to differ"
    export GX_LOGO_GVR="$GVR"; export GX_LOGO_GVP="$GVP"
  fi
  if [ "$SUBJECT" = dl ]; then
    DRIVER="$R/build/gxdl/bin/gx_dl"; DUMP=/tmp/dl_dump.ppm
  else
    DRIVER="$R/build/gxdl/bin/gx_logo"; DUMP=/tmp/logo_dump.ppm
  fi
  [ -x "$DRIVER" ] || { log "driver binary missing: $DRIVER (ORCL's lane output — run_dl.sh / gx_dl.mk owner-side; this lane never builds drivers)"; exit 5; }
  [ -d "$FAKE_DISC" ] || { log "PSO_FAKE_DISC tree missing: $FAKE_DISC"; exit 5; }
  export PSO_FAKE_DISC="$FAKE_DISC"
  export GX_DL_DEBUG=1
  DUMPBASE=$(basename "$DUMP" .ppm)
fi

# ---- per-iteration capture bodies ----
iter_window() {  # $1 = iteration index
  i=$1; png="$OUTDIR/window-iter$i-$(date +%H%M%S).png"
  "$DRIVER" >/dev/null 2>&1 & pid=$!
  w=""
  n=0; while [ $n -lt 200 ]; do
    w=$("$WINLIST" gx_window 2>/dev/null || echo none)
    [ "$w" != none ] && break
    kill -0 $pid 2>/dev/null || break
    sleep 0.05; n=$((n+1))
  done
  rc=0
  if [ -n "$w" ] && [ "$w" != none ]; then
    ID=$(echo "$w" | cut -d';' -f1); GEO=$(echo "$w" | cut -d';' -f2)
    screencapture -o -x -l"$ID" "$png" 2>>"$LOGF" || rc=1
    [ -s "$png" ] || rc=1
    log "iter $i: window $ID @$GEO -> $png (screencapture rc=$rc)"
  else
    rc=1; log "iter $i: NO WINDOW before driver exited — no artifact (loud zero, rc=$rc)"
  fi
  set +e; wait $pid; drc=$?; set -e
  log "iter $i: gx_window exit $drc"
  ARTIFACTS="$ARTIFACTS $png"; FAILS=$((FAILS+rc))
}

iter_dump() {  # $1 = iteration index — dump-hook subjects: ppm -> stdlib PNG
  i=$1; png="$OUTDIR/$DUMPBASE-iter$i-$(date +%H%M%S).png"
  rm -f "$DUMP"
  set +e; "$DRIVER" >>"$OUTDIR/driver-iter$i.out" 2>&1; drc=$?; set -e
  if [ -s "$DUMP" ]; then
    python3 "$PPM2PNG" "$DUMP" "$png" >>"$LOGF" 2>&1 || { log "iter $i: ppm2png FAILED"; FAILS=$((FAILS+1)); return; }
    log "iter $i: driver rc=$drc, dump hook fired -> $png"
    ARTIFACTS="$ARTIFACTS $png"
  else
    log "iter $i: driver rc=$drc, DUMP HOOK DID NOT FIRE ($DUMP absent) — no artifact (loud zero)"
    FAILS=$((FAILS+1))
  fi
}

# ---- loop mode: N iterations, or PREVIEW_HOLD=secs back-to-back ----
log "start: subject=$SUBJECT iters=$ITERS hold=${HOLD}s outdir=$OUTDIR"
ARTIFACTS=""; FAILS=0; i=0
DEADLINE=$(( $(date +%s) + ${HOLD%.*} ))
while : ; do
  i=$((i+1))
  case "$SUBJECT" in
    window) iter_window $i ;;
    *)      iter_dump $i ;;
  esac
  if [ "${HOLD%.*}" -gt 0 ] 2>/dev/null; then
    [ "$(date +%s)" -lt "$DEADLINE" ] || break
  else
    [ $i -lt $((ITERS + 0)) ] || break
  fi
done

log "done: $i iteration(s), $FAILS failure(s); artifacts:$ARTIFACTS"
[ "$FAILS" -eq 0 ] || exit 1
exit 0
