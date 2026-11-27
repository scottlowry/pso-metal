# P2-NP — PppFrame/NetworkRuntime residual class (collision-debt §4.3 item 3): survey + C-legal fix family

Phase-3 lane NP, 2026-11-27. Decomp `pso-decomp @6b38b5383` READ-ONLY, zero
repo builds: every number below is a scratch `clang -fsyntax-only
-ferror-limit=0 -target arm64-apple-macos11` sweep of the battery-3 corpus on
disk (`$TMP/b3/all.txt`, 1,680 units) with the pinned force-include trio
(`compat/pso_metal_compat.h`, `build/waivers/pso_assert_waivers.h` — sha256
re-verified `c6b514035f39b633…` this session — `compat/collision/pin_dolphin_types.h`).
**Note:** the official `build/parse/` output on disk (07:23) still self-labels
`target=arm64_32-apple-watchos7.0` — the mislabeled ILP32 shape the card warns
about; under it `sizeof(struct PppFrame)==8` passes vacuously (its 102-line
fail.txt lists only 12 net TUs). It is NOT ground truth for this class; true
LP64 scratch shapes are.

## 1. Exact failing set (measured)

Root: `include/pso/network/NetworkRuntime.h:18-19` — `struct PppFrame { u8
*data; u16 len; }` + `LAYOUT_ASSERT(PppFrame_size_check, sizeof == 8)`. Era
ILP32: 4B pointer ⇒ 8B ✓. True LP64: 8B pointer ⇒ 16 ✗. The waiver pin carries
`PSO_LA_FLAG_PppFrame_size_check 0` (KEEP) — every TU processing the header
fires it. Enumerated set = **50 TUs** (nominal 50 in p2-collision-debt §4.3 —
confirmed exact), **identical set** under N1 (=AZ H1C verbatim), N3R (full
C-legal stack) and N3 (=AZ H3 verbatim): 46 `.c` + 4 `.cpp`. Sweep counts:
N1/N3R = 261 clean, N3 = 1,278 clean / 402 fail (422 residual − cluster 50
overlap arithmetic per sibling lanes; cluster-zero among residuals only under
this doc's fix).

All 50 members reach the header via quoted `"pso/network/NetworkRuntime.h"`
and **no** same-directory sibling include of it exists in `include/` or `src/`
(grep: 0 hits) — unlike AZ edge-B, **every edge is `-I`-hijackable**.
`-H` receipt: `cache/scratch/np/edge_receipt.txt` (Ppp.c resolves to
`compat/net/pso/network/NetworkRuntime.h` when `-I compat/net` precedes
`-I include`).

## 2. Sub-classification + TU table (shape N3R; W/S = probe outcome of §3)

Buckets: **P** = PppFrame assert only (35). **Q** = PppFrame +
`TcpTcb_f204_check` (NetTcpState.h:160 — sibling KEEP-0 row on the RW type
`TcpTcb`, whose `size_check` is already waived; offset drifts under LP64) (7).
**Q+R** = both + a decl conflict (1: NetApi.c). **R** = PppFrame + C decl
conflict (static def vs era header decl of `NetMemcpy` /
`write_network_u16_be` / `dns_encode_name` / `validate_tcp_handle`) (3).
**T** = PppFrame + MWCC asm/declader dialect (§4.5 class, ~85 errors) (2).
**U** = PppFrame + out-of-lane assert (§4.1 `PsoCommandHandler_size` /
§4.2 `NJS_OBJECT_size`) (2).

| TU | bucket | @W | @S |
|---|---|---|---|
| | src/pso/network/ModemSession.c | P | clean | clean | |
| | src/pso/network/NetAddrFamily.c | P | clean | clean | |
| | src/pso/network/NetArp.c | P | clean | clean | |
| | src/pso/network/NetAuthSecrets.c | P | clean | clean | |
| | src/pso/network/NetBufferPool.c | P | clean | clean | |
| | src/pso/network/NetByteOrder.c | P | clean | clean | |
| | src/pso/network/NetDispatcher.c | P | clean | clean | |
| | src/pso/network/NetEthernet.c | P | clean | clean | |
| | src/pso/network/NetFragment.c | P | clean | clean | |
| | src/pso/network/NetHandlerTable.c | P | clean | clean | |
| | src/pso/network/NetIPv6NeighborDiscovery.c | P | clean | clean | |
| | src/pso/network/NetIcmp.c | P | clean | clean | |
| | src/pso/network/NetIcmp6.c | P | clean | clean | |
| | src/pso/network/NetIpv4Route.c | P | clean | clean | |
| | src/pso/network/NetIpv6Route.c | P | clean | clean | |
| | src/pso/network/NetLoopback.c | P | clean | clean | |
| | src/pso/network/NetPppLink.c | P | clean | clean | |
| | src/pso/network/NetPppoe.c | P | clean | clean | |
| | src/pso/network/NetworkMessageTransport.c | P | clean | clean | |
| | src/pso/network/NetworkPacketRuntime.c | P | clean | clean | |
| | src/pso/network/NetworkRuntime.c | P | clean | clean | |
| | src/pso/network/NetworkThreadRuntime.c | P | clean | clean | |
| | src/pso/network/NetworkTransport.c | P | clean | clean | |
| | src/pso/network/Ppp.c | P | clean | clean | |
| | src/pso/network/PppApi.c | P | clean | clean | |
| | src/pso/network/PppChap.c | P | clean | clean | |
| | src/pso/network/PppControl.c | P | clean | clean | |
| | src/pso/network/PppFraming.c | P | clean | clean | |
| | src/pso/network/PppIpcp.c | P | clean | clean | |
| | src/pso/network/PppLcp.c | P | clean | clean | |
| | src/pso/network/PppMd5.c | P | clean | clean | |
| | src/pso/network/PppPap.c | P | clean | clean | |
| | src/pso/network/PppVj.c | P | clean | clean | |
| | src/pso/network/PppVjCompress.c | P | clean | clean | |
| | src/pso/network/TUdpSocket.cpp | P | clean | clean | |
| | src/pso/network/NetConfig.c | Q | clean | fail | |
| | src/pso/network/NetDhcp.c | Q | clean | fail | |
| | src/pso/network/NetIpv4.c | Q | clean | fail | |
| | src/pso/network/NetIpv6.c | Q | clean | fail | |
| | src/pso/network/NetTcp.c | Q | clean | fail | |
| | src/pso/network/NetUdp.c | Q | clean | fail | |
| | src/pso/network/SocketProtocolRuntime.c | Q | clean | fail | |
| | src/pso/network/NetApi.c | Q+R | fail | fail | |
| | src/pso/network/NetDns.c | R | fail | fail | |
| | src/pso/network/PppBuffer.c | R | fail | fail | |
| | src/pso/network/PppFsm.c | R | fail | fail | |
| | src/pso/network/NetworkConnect.cpp | T | fail | fail | |
| | src/pso/network/eth.c | T | fail | fail | |
| | src/pso/network/TTcpSocket.cpp | U | fail | fail | |
| | src/pso/ui/frontend/TAdDialingGC.cpp | U+f204 | fail | fail | |

## 3. Fix families — candidates proven (before/after), BANNED route disproven

Constraint honored: `.c` TUs may NOT take the `compat/njs` reinterpretation
shadow (AR §5 silent truncation; collision law: `-Icompat/njs` never on `.c`).
Measured N3R (before) = 0/50 cluster clean.

| # | family | shape | cluster clean | corpus flips vs N3R | breakage | verdict |
|---|---|---|---|---|---|---|
| 1 | njs-style u32-field truncation shadow (**banned** on `.c`) | S2 | 25/50 but 25 TUs get **new** type errors (`incompatible integer to pointer conversion`, e.g. PppFraming.c 27) | — | — | **dead**: measured breakage = exactly the AR §5 failure mode |
| 2 | scalar rebind leg (restore 4B scalars so assert is era-true) | N1/N3R | 0/50 | — | 0 | **inapplicable**: firing members are pointer-bearing; pointer-free net KEEP-rows (`PppHeader_size_check`, `PppLinkProtocol_size_check`, both KEEP-0) fire **zero** times corpus-wide at N1/N3R — the rebind already makes them honest for free |
| 3 | **waiver flag-flip** `PSO_LA_FLAG_PppFrame_size_check` + `PSO_LA_FLAG_TcpTcb_f204_check` 0→1 (both types RUNTIME-WAIVE per p1-format-net-disc §1.3/§4) | W | **42/50** | **+50** (8 more from the f204 flip outside this class) | **0** | works; erases the two checks (`static int` stub) |
| 4 | **era-model layout shadow** `compat/net/pso/network/NetworkRuntime.h` (this lane's supply; host struct truthful, assert re-pins era layout via model struct, name preserved, check stays live) | S | 35/50 (=bucket P exactly) | +43 | **0** | works for family P while **keeping the invariant compile-visible**; Q rows need family 3's f204 line anyway |

Recommended landing = **3 ∪ 4**: adopt the shadow (fail-closed posture for
PppFrame) **and** flip `TcpTcb_f204_check`; or minimal-diff = family 3 alone.
Either way the remaining 5–8 are R/T/U rows owned by §4.1/§4.2/§4.5 classes —
this class contributes 0 unsolvable members.

## 4. Integrator flag-ask (paste-ready; never a fourth `-include`)

Shadow adoption, inserted before `-I include` (order-sensitive; keep the
pinned trio verbatim, `compat/net` is `-I`-only, inert until this lands —
repo grep confirms `compat/net` referenced nowhere else):

```
-I $REPO/compat/net -I $REPO/compat/repin -I $REPO/compat/njs -I $REPO/host/bringup/include \
-I include -I include/clib -I libs/dolphin/include -I libs/dolphin/include/dolphin -I src
```

(`-I $REPO/compat/njs` only on the C++ TU list, per collision law.)

Flag-flip alternative (or the Q-row complement to the shadow): re-verdict
`PppFrame` and the `TcpTcb_f204_check` row RUNTIME-WAIVE in
`data/assert-verdicts.tsv`, re-run `gen_waivers.py` (integrator-owned; this
lane never did) — target lines:

```
#define PSO_LA_FLAG_PppFrame_size_check 1
#define PSO_LA_FLAG_TcpTcb_f204_check 1
```

Gate after adoption: `grep -l PppFrame_size_check build/parse/err/*` = 0 under
the fixed true-LP64 target; residual net failures must equal §4.1/§4.2/§4.5
owners' counts, not this class.

## Repro

`cache/scratch/np/`: `np_probe.sh` (shapes N1/N3R/N3/W/S/S1/S2; fresh file
modeled on az_probe.sh, never sed-derived), `np_sweep.sh`, per-shape
`err/ pass.txt fail.txt`, `cluster.txt` (50), `table.json`,
`edge_receipt.txt`, `pso_assert_waivers_flip.h` + `shadow_bad/` (scratch proof
artifacts — repo waiver header and decomp untouched). Python-counted
before/after per §3. Writes this lane: `compat/net/README.md`,
`compat/net/pso/network/NetworkRuntime.h`, this doc — nothing else.
