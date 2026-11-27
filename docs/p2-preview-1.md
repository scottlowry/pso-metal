# P2-PREVIEW — owner preview tooling (host/gx/preview)

One command to see any render, on demand, repeatedly. Lane: PREVIEW (wave-5.5b).
Nothing here rebuilds or edits a driver: `preview.sh` only **invokes** existing
binaries (L7P3's `build/gxwindow/bin/gx_window`, ORCL's `build/gxdl/bin/{gx_dl,gx_logo}`)
and captures proof frames. The lane's single build (granted exception) is
`make -C host/gx/preview` → `build/gxpreview/bin/winlist` (binary gitignored
via `build/.gitignore: bin/`; source committed).

## One command per render

| render | command | capture mechanism |
|---|---|---|
| GX→Metal window replay | `host/gx/preview/preview.sh window` | CGWindowList window-id `screencapture -l<id>` (gx_window has **no** dump hook) |
| display-list quad gate | `host/gx/preview/preview.sh dl` | driver's own `GX_DL_DEBUG` → `/tmp/dl_dump.ppm` → stdlib PNG |
| LOGO (era CLUT fixture) | `host/gx/preview/preview.sh logo` | `GX_DL_DEBUG` → `/tmp/logo_dump.ppm` → stdlib PNG |
| LOGO genuine disc art | `host/gx/preview/preview.sh logo-genuine` | same, with `GX_LOGO_GVR`/`GX_LOGO_GVP` pointed at `host/bringup/local_assets/` — **fails closed (exit 3) when local_assets is absent**, by design (genuine bytes are gitignored, fnv+dims receipts only) |

Default `iterations=3`; PNGs land in `OUTDIR` (default
`/tmp/preview-<subject>-<stamp>`). `OUTDIR` **must be outside the repo** —
proof frames never become commit candidates. Per-run transcript:
`$OUTDIR/preview.log`, driver stdout kept at `$OUTDIR/driver-iter<N>.out`
(dump-hook subjects). Exit codes: `0` green, `2` usage/bad OUTDIR,
`3` subject unavailable (missing local_assets), `4` preflight known-mode
(TCC), `5` driver binary/helper missing, `1` some iteration produced no
artifact (each miss is a LOUD log line, never a silent zero).

## Repeated viewing: PREVIEW_HOLD

`PREVIEW_HOLD=30 host/gx/preview/preview.sh window 1` re-opens the render
back-to-back for 30 seconds — watching the display, each pass appears again
and again (one capture per pass). Works for every subject.

## Screen-Recording PREFLIGHT (window subject)

Window-id capture is TCC-gated. Before launching anything, `preview.sh
window` probes: enumerate on-screen windows via `winlist --all`, capture up
to 5 candidates with `screencapture -o -x -l<id>`; zero writable captures ⇒
**LOUD preflight failure (exit 4)** printing the exact fix:

1. System Settings → Privacy & Security → **Screen and System Audio Recording**
2. enable the app hosting your shell
3. fully quit + relaunch it (TCC caches per-process), rerun

This is the wave-5.5b daemon-context blindness as a **known mode**: a
launchd/SSH/CI context that cannot hold the grant reads as that banner, and
`dl`/`logo`/`logo-genuine` (dump-hook paths, no permission needed) still work
there. Simulate it any time with `PREVIEW_FORCE_NO_TCC=1`.

## Self-test (both branches proven)

`sh host/gx/preview/tools/selftest.sh` → `/tmp/preview-selftest/SELFTEST.md`:

- **branch B** (always exercised): forced denial must exit 4 with the fix
  text — PASS.
- **branch A**: 3 iterations vs `gx_window` → ≥3 labeled PNGs + md5s.
  In this repo's owner session (Screen Recording granted) branch A is
  capture-green; in a TCC-blind context branch A itself rides the exit-4
  known-mode path and that is **also** a PASS (loud, never silent).

Landed run (2026-11-27, capture-green): rc=0, branch A 3 PNGs
(`window-iter1-131852.png` md5 `91ca214eb1aa28f997d3b215e90fe04b`,
`window-iter2-131858.png` + `iter3` md5 `e9e5ea6fd02534e59d0f362ee70d1ce5`;
decoded: 640×512, alternating white/black era clear with 126 distinct colors
— real pixels) and branch B fired with exit 4 + fix instructions.

## Files

```
host/gx/preview/preview.sh          entry point (POSIX sh)
host/gx/preview/Makefile            own build -> build/gxpreview/bin/winlist
host/gx/preview/tools/winlist.swift CGWindowList window-id probe (committed source)
host/gx/preview/tools/ppm2png.py    P6/P3 -> PNG, pure stdlib (zlib+crc32)
host/gx/preview/tools/selftest.sh   branch A/B self-test, report writer
```
