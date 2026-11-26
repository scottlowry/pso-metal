/* Math (mtx.h vendored) — pso-metal host re-host of Math
 * Tag format: original SDK symbol | call-site count (docs/host-surface.md) |
 * effort (TRIVIAL/MODERATE/HARD/BLOCKING-GPU) | native backing note.
 * Featured = real dolphin signature; tail = PSOSH_TAG placeholder prototype. */
#ifndef PSOSH_MTX_H
#define PSOSH_MTX_H
#include "psosh_types.h"


/* #15 PSMTXConcat — mtx.h — 20 sites — TRIVIAL
 * 3x4 row-major matrix multiply (PSO vendored math, not hardware).
 * Native backing: compile definitions natively (pure FP); arm64 NEON later.
 * TODO(P2): land mtx definitions .c in port build (host-surface.md §4). */
void PSMTXConcat(const GC_Mtx mA, const GC_Mtx mB, GC_Mtx mAB);

/* mtx.h — TRIVIAL — 3x4 identity store. Native: memset+4 stores. */
void PSMTXIdentity(GC_Mtx mtx);

/* ---- long tail: placeholder prototypes; exact signatures land in P2 ---- */
PSOSH_TAG(C_MTXLightPerspective, 8, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(PSMTXInverse, 5, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(PSMTXScale, 5, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(PSMTXTranspose, 4, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(PSMTXRotRad, 3, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(PSMTXTrans, 3, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(PSMTXCopy, 2, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(PSMTXQuat, 2, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(C_MTXFrustum, 1, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(C_MTXLightFrustum, 1, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(C_MTXLookAt, 1, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(C_MTXOrtho, 1, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(C_MTXPerspective, 1, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(PSMTXMultVec, 1, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(PSMTXMultVecArray, 1, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(PSMTXMultVecArraySR, 1, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(PSMTXMultVecSR, 1, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(PSMTXRotAxisRad, 1, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")
PSOSH_TAG(PSVECDistance, 1, "TRIVIAL", "pure FP CPU math - compile natively, no I/O")

#endif /* PSOSH_MTX_H */
