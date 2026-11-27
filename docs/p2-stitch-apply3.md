# P2 stitch apply-3 — resolutions-6 (AX) + resolutions-7 (AY-encodings)

Date: 2026-11-27 (~06:00). Integrator-applied; corpus inputs were lane-authored.

## Inputs
- `data/verdict-resolutions-6.tsv` (AX, `91911e9`): 78 rows = 71 RUNTIME-WAIVE / 7 LOCKED-FORMAT.
  Headline: ChallengeRecord LOCKED-FORMAT HIGH; 75 keep-firers closed (0 open); 16 ghosts = harness path-rot.
- `data/flip-risk-ledger.tsv` + `docs/p2-flip-risk.md` (AY, `c114d9b`): 67 unique types, buckets 13 promote-to-yes /
  11 lock-now / 40 rw-confirmed / 3 needs-decision; `packet_header` LOCKED via send-verbatim (`TProtocol.cpp:27-34`).
- `data/verdict-resolutions-7.tsv` (integrator encoding of AY rulings): 25 LOCKED-FORMAT rows, tab-delimited,
  evidence cites back into the ledger + its file:line (single provenance chain).

## Measured effects
- Regen: waived **2,806 -> 2,949 (res-6 only, battery-3) -> 2,943/3,333 over 1,548 types (res-6+res-7)**;
  parse (arm64_32) 1,578/1,680 unchanged; `parse_gate` OK (+909); bringup suites 110/110 + dl gate green
  (frames=30 tris=60) in battery-3 run, header pin re-verified by stamp.
- Flag diff vs committed apply-2 header: **up=143, down=6, dropped-lines=0**. All six downs are documented
  ledger revocations (`ChangeShipOrBlockPacket_size`, `CharacterSaveInfoPacket_size`, `Ipv6NdMtuOption_size_check`,
  `Ipv6NdPrefixInfoOption_size_check`, `TradeProposalAmountOffset`, `TradeProposalSize`) — first down-delta of the
  stitch system, and the override-receipt discipline holds in the safe (keeper) direction.
- LP64: battery-3 measured **162 PASS / 1,518 FAIL** (res-6 corpus). Post-res-7 bounded re-measure: the six revoked
  sites live in exactly 3 units (`TProtocol.cpp`, `TradeSession.cpp`, `NetIPv6NeighborDiscovery.c`), and **all three
  were already FAIL** (blocked by TItemData.h:151 / IPv6.h:144 class asserts) — FAIL->FAIL, so **162 stands**.
  No full re-sweep run for this batch; the bounded proof is exact because the delta cannot affect other files.
- Promise-vs-real corrections (recorded, not smoothed):
  - AX promised "6 files flip green-eligible on next re-record" -> materialized **+1** (`TAdGuideButtonGC.cpp`);
    the other five carried non-res-6 blockers (deny-root SystemSaveData family + multi-bucket files).
  - AY predicted "-17 sites (2,806->2,789)" — computed against the apply-2 baseline; against live res-6 corpus the
    true revocation set is the 6 sites above. Both claims were correct vs their stated baselines; the lesson is
    banked: cite the baseline the prediction was computed against.

## Guard
- `host/bringup/waiver-header.sha256` re-pinned **c6b514035f39b633...** (matches on-disk header stamp, verified by
  double read-back). D8 guard suites re-run green in battery-3 against this batch's predecessor header; this
  header's own re-pin verified by stamp + regen-time parse gate.

## State of programs after apply-3
- Re-pin program: PREMISS FLIPPED by AW (`befe299`) — trio giants are scalar-width keeps; the rebind is the lever
  (215 clean rebind-only; 379 full-stack). Adoption shape = per-TU staged (save-root 6 -> TItemData 6 -> wide 23),
  global rollout gated on the quoted-`dolphin/types.h` collision debt (1,109 units) -> landing-lane card.
- Dialect queue: 41+24 owner rows (AS2) — unchanged, sequenced behind collision debt (same rebind family).
- needs-decision 3 (incl. FloorEventSet overlay-vs-embed contradiction): open, evidence named in ledger rows.
- Corpus closure: 0 battery-firing UNREVIEWED types (AX); census will re-confirm on next full sweep.
