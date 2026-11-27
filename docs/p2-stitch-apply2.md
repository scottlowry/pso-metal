# P2-STITCH APPLY-2 — full-corpus regen receipt (integrator, 2026-11-27 04:4x)

Consumes AT's tail (resolutions-5, ec781c4): every AK-named type (1,602/1,602) now in
the 8+1-source corpus. All steps executed; every rc read from logs, none inferred.

| Gate | Battery-1 (apply-1) | Battery-2 (this) | Delta |
|---|---|---|---|
| waived asserts / types | 2179/3333, 934 | **2806/3333, 1483 types** | +627 / +549 |
| arm64_32 `make parse` | 1576/1680 | **1578/1680** | +2 |
| parse_gate vs 669 floor | +907 | **+909, rc 0** | — |
| **LP64 materialized** | 150/1680 | **161/1680** | +11 |
| bringup suites | 82 green | **110/110 (16+13+26+27+28)** | AU era recert |
| dl 7-pt textured gate | green | **green rc 0** (verify_failures=0) | — |
| deny-violations / 1->0 flips | 0 / 0 | **0 / 0** | guard sha re-pinned 751f077b |

Override receipt still fires exactly once: NJS_POINT3COL(keeper@4->rw@6).

## Why 161, not AT's "+125"
AT's report was honest about its split: of its projected 125 pure-layout units,
**only 10 were src-TUs** — 115 were standalone header units not counted by the
1,680-file source sweep. 150 + 10 = 160 expected; measured 161 (one co-gated file
also cleared). No refutation — but the read-across is now stark:

**The LP64 wall is no longer the verdict queue.** Remaining 1,519 failing files are
dominated by NON-layout causes (T's buckets: 65 SDK-dialect + musyx extern-C,
intrinsics families, orig_asm/.inc, include-graph, sig-drift, 30 quest
source-rewrite, LOCKED-honest keeps) plus LOCKED-type co-gates. Cause-bucket
census of the fresh uncapped battery-2 error logs is the next measurement (lane
card: residual-classification), feeding AV's V-lane and the AR allow-list staging.

## Honest ceiling note
Materialization from waivers alone is ~saturated near the mid-160s. The big multi-file
wins from here are STRUCTURAL programs, in order per AP/AR/AU evidence:
1. AR allow-list application (integrator build/ edit — spec ready in p2-gx-tex-consumer §4)
2. T-bucket dialect queue (compat/dialect extension: 65-TU class)
3. loader-lane handle conversions (V-lane per consumer contract; logo = V4)
