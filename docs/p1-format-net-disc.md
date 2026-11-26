# P1-H — Network + Disc Format-Boundary Audit

Phase-1 task H. Method: read the decomp's own packet read/write and disc-loader code at
`/Users/scott/Hermes/pso-decomp/pso-decomp` (READ-ONLY; all file:line below relative to
that root). No newserv code was fetched or read; direction claims are derived from the
decomp's swap call sites. Verdict frame: **FORMAT-KEEP** = bytes must stay byte-true
across a boundary (wire image or disc image); **RUNTIME-WAIVE** = host layout free, no
bytes ever leave the process in that struct.

## 0. Headline finding: the netcode has TWO opposite endian regimes

| Regime | Where | Wire byte order | Evidence |
|---|---|---|---|
| **A — game protocol (LE on the wire)** | `packet_header`, all `S_/C_/SC_*` (`commands.h`), all `G_*` 6x subcommands (`subcmd_6x.h`), lobby/char/item records | **little-endian** multi-byte fields | Every send path builds a host-order struct then calls an **unconditional** `bswap()` right before `send()` and again right after receive (e.g. `TProtocol.cpp:26-34` keepalive: set `header.size`, `header.bswap()`, `send()`; `packet_header::bswap()` = `bswap_16(&size)` at `TProtocol.cpp:1891-1893`; ~76 `bswap()` call sites in `TProtocol.cpp`, `bswap_and_send_extended_subcommand` `TLobby.cpp:524,673,702,736`). On the BE PPC client, swap-before-send ⇒ the **bytes on the wire are LE** (DC/PC-client heritage). |
| **B — raw TCP/IP/PPP stack (BE on the wire)** | Ethernet/IP/TCP/PPP frames under the game protocol | **big-endian** (RFC order) | `NetByteOrder.c:18-40` `read/write_network_u16/u32_be` are byte-serial and endian-safe; but `NetTcp.c:494-571` writes `th->seq/th->ack/th->win/th->sum/th->off` **directly as host-order integers into the frame image** (valid only because PPC is BE; `NetTcp.c:900-901` hand-swaps `seq` with a 4-byte `NetMemcpy` pair), and `NetworkMessageTransport.c:143-149` matches `EthernetFrameHeader.ether_type` via a raw `u16*` load against registration constants. `PppFraming.c` assembles MSS BE (`(p[2]<<8)|p[3]`). |

Consequence for pso-metal: regime-A `bswap()` calls are **host-order converters** and must
become no-ops on arm64 (host already LE); regime-B direct host-order reads/writes are
**silent corruption sites** and must become explicit BE accessors. One global "keep all
swaps" or "delete all swaps" policy is wrong — they sit on opposite sides of this split.

## 1. WIRE — the audited record inventory

### 1.1 Packet header format
`packet_header { u8 command; u8 flags; u16 size; }` — `include/pso/network/protocol/protocol.h:14-19`,
pinned by `LAYOUT_ASSERT`s at `src/pso/network/TProtocol.cpp:23-25` (size 4, flags@1, size@2).
`size` is LE on the wire (§0-A). Game packets carry it in every `T*` recv class
(`packet_classes.h`, 10 structs, 38 `bswap` members).

6x broadcast subcommands carry a second-level header family in
`protocol_types.h:48-95`: `G_ClientIDHeader {u8 subcommand; u8 size; u16 client_id;}`,
`G_EntityIDHeader`, `G_ParameterHeader`, `G_UnusedHeader` — `client_id`@2 is LE-on-wire;
`bswap_g_client_id_header` / `bswap_extended_subcommand_header`
(`TPlyJoinLobbyEntry.h:57-58`) run at `TLobby.cpp:491` etc.

### 1.2 Record count (game protocol, FORMAT-KEEP)
| File | structs | notes |
|---|---:|---|
| `protocol/subcmd_6x.h` | **174** | `G_*_6xNN` game subcommands (moves, item drop, switch, chat, sync) |
| `protocol/commands.h` | **54** | `S_/C_/SC_*` lobby/ship/file/login commands (`C_Login_DC_PC_GC_9D`, `S_JoinGame_GC_64`, guild-card chunk, reconnect) |
| `protocol/protocol_types.h` | **16** | shared payloads: `VectorXZF/XYZI`, `FloorItem` (**embeds `TItemData` by value**, `:36-45`), `GuildCardPC`, `PlayerLobbyDataDCGC`, `PlayerRecordsEntry_V3`, `StatusEffectState`, `TelepipeState` |
| `protocol/packet_classes.h` | **10** | recv classes: `TRegister`, `TMenuList<QuestListEntry/LobbyListEntry/GameListEntry>`, `TRecvPort`, `TRecvDownload`, `TPlyJoinGame`, `TPlyStartGame`, `TPlyJoinLobbyData` |
| `protocol/cast_packets.h` + `choice_search.h` | **12** | tech-cast + choice-search records |
| `TProtocol.h` / `TLobby.h` / `FloorEventCommand.h` / `TPlyMeetUserExtension.h` | **48** | remaining transport + lobby state/packet structs (16+30+2) |

**Total game-protocol wire records: 314 structs.** Their multi-byte fields are the
newserv-compatible LE wire format (`protocol_types.h:1-19` header comment: generated from
newserv's `le_*` types via `tools/newserv_match.py`; that is a format cross-check of
names/sizes, not code reuse).

Item transfer: `G_DropItem_*_6x5F` embeds `FloorItem` by value
(`ItemDropPacket.h:9-12` LAYOUT_ASSERTs: item@0x10, packet 0x2C); `bswap_item` /
`bswap_item_packet` (`ItemDropPacket.h:21-27`) do a **fixed 0x24-byte record swap** —
the item's wire image is `TItemData` (24B), whose own `bswap()` (`TItemData.cpp:458-459`)
swaps `data2.data_u32`. Char-transfer blocks are **opaque byte blobs**, not struct
embeds: `C_CharacterData_V3_61_98 { u8 inventory[0x34C]; u8 disp[0xD0]; … }`
(`commands.h:49-57`) — good for the port: no host-layout dependency inside.

Lobby join records: `TPlyJoinLobbyData::bswap()` (`packet_classes.cpp:194-198`) =
`tag.bswap()` (the `gc_tag` helper of `TPlyGuildCardTag.h:24`) + `bswap_32(ip_addr)` +
`bswap_32(client_id)`; `GameListEntry::bswap()` (`:232`), `LobbyListEntry::bswap()`
(`:117`), `QuestListEntry::bswap()` (`:54`). Lobby rule block =
`LobbyRuleSettings` (`LobbyRuleSettings.h:9-50`, all ≤s32 scalar fields, no bitfields):
travels as byte fields, endian-safe as declared.

### 1.3 TCP/UDP/PPP state structs (regime B)
FORMAT-KEEP (frame images — bytes on the wire must be RFC-BE):
`TcpHdr` (`NetTcpState.h:30-42`, 0x14, asserted), `IPv4Header` (`IPv4Header.h:6-18`,
0x14), `EthernetFrameHeader`/ARP/IPv6 headers (`NetEthernet.h`, 6 structs; `NetArp.h`,
`IPv6.h`). Multi-byte fields: `TcpHdr.sport/dport/seq/ack/win/sum/urp`,
`IPv4Header.len/id/frag/cksum/src/dst`, `EthernetFrameHeader.ether_type` — all written
and parsed **as host-order loads** today (`NetTcp.c:494-593,783,1778-1802`;
`NetworkMessageTransport.c:144`), i.e. correct only on BE hosts → every one is a flip
site on arm64.

RUNTIME-WAIVE (never image these to bytes): `TcpTcb` 0x544B
(`NetTcpState.h:51+`), `TcpSackSeg`, `TcpSeqRange`, `TcpWait`, `TcpCompletion`,
`NetRuntime`/socket tables, `ModemRuntime`/`ModemSession`, `PppFsm`/`PppLcp` state —
pure in-process state machines. The `SEQ_*` wrap macros (`NetTcpState.h:20-23`) are
signed-diff arithmetic, endian-neutral.

**Bitfield verdict (`NetTcpState.h:92-94` `f204_b7/b6/rest`)**: a full-tree grep for
`f204` shows the byte is accessed **only through the three named bitfields**
(`NetTcp.c:443,2566,2577`-area `get_snd_queued/get_snd_space`,
`SocketProtocolRuntime.c:22,135,326,549,572,685,927-940`); the `0x40/0x80` masks in the
neighboring header comment (`:60-61`) belong to the **separate plain `u8 flags`** field,
which is the only byte-masked flag byte in that region. So the abi.md §3 collision is
latent, not live: today's code is bitfield-consistent and survives the PPC→arm64 flip
*self-consistently*. **Verdict: RUNTIME-WAIVE** — but keep abi.md's policy anyway
(replace with one `u8 f204;` + `#define F204_B7(b) (((b)>>7)&1)`) so that a future
`flags`-style mask on this byte cannot silently disagree with clang's LSB-first
allocation. Not a wire-format type: `f204` never leaves the TCB.

### 1.4 Where read/write happens (map for the swap layer)
- Send side: `TProtocol.cpp` (38 fns), `TLobby.cpp` (`bswap_and_send_extended_subcommand`,
  14 fns), `TPacket.cpp`/`TUdpSocket.cpp` (0x5B4 send buffer, `TPacket.h:8-9`),
  `TTcpSocket.cpp:42-48` → `tcp_send`.
- Recv side: `TProtocol::m_recv_handlers` dispatch table (offset-pinned, e.g.
  `TProtocol.cpp:20-22` asserts handler@0xBC9C — runtime layout, WAIVE), each handler
  starts with `x.bswap()`.
- Stack: `NetTcp.c` / `NetIpv4.c` / `NetUdp.c` / `NetworkMessageTransport.c` /
  `eth.c`+`mdm.c` (236 EXI sites — replaced wholesale by BSD sockets per host-surface.md
  §2; the regime-B structs above the driver stay).
- Crypto/path framing: `PSOV3Encryption.cpp` / `PSOV3EncryptionTCP.cpp` (byte-serial),
  `PppVj*` (byte-serial), `PppMd5.c` (LE word assembly — **correct** MD5, endian-safe).

## 2. DISC — raw-image loaders vs runtime-built

### 2.1 FORMAT-KEEP records (structs overlaid on disc bytes as read)
| # | Struct | File:line (def / raw-read site) | Multi-byte BE fields | Current swap? |
|---|---|---|---|---|
| D1 | `GslArchiveEntry` (0x30) | `system/GslArchive.h:8-18`; DMA'd whole in `open_gsl_archive` `TGStream.cpp:244` (`ADXF_ReadNw`), then read **host-order** at `:98,170-178,357-358` | `archive_offset@0x20`, `file_size@0x24`, `decoded_size@0x28` (s32, disc-BE) | **NONE — silent-flip site #1 on arm64** |
| D2 | `GvmTextureHeader` | `unk/text_803DD5E0.h:13-17`; `load_gvm_texture_archive` `text_803DD5E0.cpp:409-413` | `magic`, `size` (u32, disc-BE) | `bswap32_in_place` (explicit; DC-lineage swap — see §2.4) |
| D3 | `GvrTextureChunk` | `text_803DD5E0.h:21-25`; chunk walk `:432-436` | `magic`, `size` | `bswap32_in_place` |
| D4 | GVRT pixel/tlut payload | stream walk `text_803DD5E0.cpp:368-375,424-437` | texel words are BE HW format | none needed — uploaded/copied as **bytes**; keep byte-true |
| D5–D10 | `NetworkInfoNamedConfig`(0x28)/`ConnectionConfig`(0x210)/`MailConfig`(0x124)/`CredentialConfig`(0x84)/`Text400`/`Text256`/`NetworkInfoConfig`(0x19D4) | `network/NetworkInfoCodec.h:8-116` (7 offset asserts pin the blob) | mixed; loaded as blob via `NetworkInfoLoader.cpp` | offsets pinned — treat as blob image, field access through accessors |
| D11 | `MfciFileEntry` (0x38, name@0x1C, read_offset@0x30, request_size@0x34) | `system/movie/Mfci.h:6-29` | offsets (s32) | pool `MfciFilePool` is runtime-built, but the entry values originate from the movie TOC stream — verify at Mfci.cpp read path before host build |

### 2.2 RUNTIME-WAIVE (built at runtime; nothing to keep byte-true)
`PrsReader/PrsDecoder` (`system/prs.cpp:21-31`, `prs.h`) — decoder is **bit/byte-serial**
(`prs_read_src_byte`, `prs_read_control_bit` shifting a single control byte MSB-first):
endian-clean as written, port verbatim. `TcpTcb` et al. (§1.3), `PlacementQueue`
hierarchies, Ginja/Ninja runtime structs built *from* D4 payloads,
`gsl_archive_entries[256]` RAM table *after* one host-order fixup pass,
`MovieFramePack.cpp` unpacked frame words (RLE packs `hi=uw>>8` — byte math, review but
no struct overlay), `Lzss.cpp`, `RlcArchive` runtime tables, `CvFs`/`Adxf` device state
(`Adxf.cpp` bottoms out at DVD). The whole `TGStream` stream cursor is runtime state —
except the `archive_entry` pointer (§D1 image).

### 2.3 Archive paths (boot-spine cross-check)
`load_disc_file` (`unk/TLobby_800F8AA8.cpp:1382`) → `TGStream::load_file`
(`TGStream.cpp:120`) → GSL TOC (D1) else raw ADXF sectors (2048-aligned;
`ADXF_ReadNw` demands 32B-aligned DMA targets, `Adxf.cpp:270-276` — the alignment
requirement itself is a format-boundary artifact; host pread path can drop it).
Sound/movie: `.gsl` ADX streams via `AdxManager`/`Adxt`, movies via `Mfci`/`Gcci` over
`CvFs`. Font/logo/texture chunks all land in the GVM/GVR + PRS path (D2–D4). Integrity
gate `text_802A1020.cpp` CRC is bit-serial (boot-spine S12) — endian-safe but
disc-content-pinned.

### 2.4 Where the port-side byteswap layer must sit (DISC)
The DC-legacy loaders (D2/D3) call `bswap32_in_place` **on every read**; the GC disc
image is PPC-BE, so on PPC the swap converts BE→LE *working values* (e.g. magic compared
as `0x54525647` = the byte string "GVRT" reversed, `text_803DD5E0.cpp:414-415`). On an LE
arm64 host the raw BE bytes already load to exactly those working values ⇒ **the explicit
swap must become identity on LE, while the raw host-order reads (D1, stack §1.3) must
gain a swap.** In both regimes the *disc bytes* are the invariant; the *host working
representation* (here: LE-order integers) is what both paths converge on. Same logic for
D1's s32 trio: working values = BE-interpreted on PPC = LE-order byte reads on arm64.

## 3. Cross-reference vs the ~20 assert-failing types (abi.md §4 + runners-up)

Search space: every struct in §1 and §2 (all of `include/pso/network/**`, `src/pso/network/**`,
`system/{GslArchive,TGStream,prs}.h/.cpp`, `unk/text_803DD5E0.*`, `system/movie/*`,
`network/NetworkInfoCodec.h`). Grep of the 20 type names across all of it: **zero
by-value embeds** (the single hit is a prose comment naming `TBoss4DarkFalz` as a packet
*sender*, `subcmd_6x.h:298`).

| Failing type (abi.md site) | Cause | In wire record? | In disc record? | Proven unrelated because |
|---|---|---|---|---|
| `PlayerActionPaletteState` (`PlayerActionPaletteState.h:25`) | A | no | no | hotbar RAM state; wire char blob carries it inside opaque `disp[0xD0]`/`inventory[0x34C]` byte arrays (`commands.h:49-57`), never as the C++ type; zero name refs under `network/` |
| `MaterialColorFade` / `RenderFadeState` (`MaterialColorFade.h:27-28`, `RenderFadeState.h:23`) | A | no | no | render-loop objects; GVR/GVM headers contain neither |
| `PlayerWindowCommand`/`PlayerWindowQuery` PMFs (`WindowCtrlCallback.h:9-10`) | B | no | no | compiled callback tables |
| `TBoss4DarkFalzHandler/State`, `TBoss4Type3State`, `TBoss6PlotFalzState`, `TBoss6Type2BattleAction` | B | no | no | boss FSM tables in `.data`; the 6x packets they trigger are `G_*` records, swapped field-by-field (`bswap_extended_subcommand_header`) |
| `TWindowHelpItem` (`TWindowHelpItem.h:187-188`), `TWindowLockOn`, `CreationMenuEntry` (`TObjCMMainMenu.h:19`) | A+B/B | no | no | UI windows, never persisted |
| `PlacementQueue` (+`PositionPlacementQueue`/`ItemDropQueue`) (`TPlyItem.h:109`) | A | **no** — adjacent! | no | it *feeds* the 6x5F drop packet but is not embedded: packet = `G_ClientIDHeader` + `FloorItem`{plain fields + `TItemData`} (`ItemDropPacket.h:9-12`); users are `TPlyItem.cpp`, `TTecLuker.cpp`, `TPlacementSearch.h` only |
| `TSetEvtDestroy` (`TSetEvtDestroy.h:70-76`) | A | no | no | class parses `.evt` streams at runtime; the stream is FORMAT-KEEP data, the overlay is not pinned — event records walked by byte math |
| `TItemDisconnectCtrl`, `TObjDFRecover` | A | no | no | scene objects |
| `TSaveChCr` (+`TSaveChCrStateActions`) (`TSaveChCr.h:59-67`) | B | no | no | *lives in* `system/save/` but is the memcard **UI controller** (TObject subclass, PMF state table); the card blob formats are the `McFsGc`/char-save walkers (`McFsGc.cpp`, `TObjPlayer_801AF2F0.cpp`), none of which embed it |
| `TAdTimeSetGC` (`.cpp:85-86`), `TCheckMemoryCard` (`.cpp:35-36`) | B | no | no | phase/UI controllers with PMF tables |

**Assert-crossing types: 0.** None of the 43 failing assert lines sits on a FORMAT-KEEP
struct, so abi.md §6 policy (host layout authoritative for these 15 headers) has **no
wire/disc-format interaction** — the two workstreams can proceed independently.

## 4. Verdict table (audited types, grouped)

| Group | Types | Verdict | Action |
|---|---|---|---|
| Game-protocol wire records | 314 structs (§1.1-1.2), incl. `packet_header`, `G_ClientIDHeader`, `FloorItem`+`TItemData`, lobby join/list records, char blobs | **FORMAT-KEEP** (LE-on-wire) | keep field order+packing; protocol `bswap()` fns → identity on LE host via one shim (§5); never `#pragma pack`-shrink (no alignment exposure: max member align 4) |
| Raw-stack frame headers | `TcpHdr`, `IPv4Header`, `EthernetFrameHeader`+ARP/IPv6 (10 structs) | **FORMAT-KEEP** (BE-on-wire) | wrap every multi-byte field in BE accessors (§5) or replace stack with BSD sockets above `NetworkMessageTransport` boundary |
| Net runtime state | `TcpTcb`, `TcpSackSeg/SeqRange/Wait`, `ModemRuntime`, `PppFsm/Lcp`, `NetworkRuntime*`, info-struct `NetworkInfoCipher*` (`NetworkInfoCodec.h:87-105`) | **RUNTIME-WAIVE** | host layout free; keep asserts host-recomputed |
| `NetTcpState.h:92-94` bitfields | `f204_b7/b6/rest` in `TcpTcb` | **RUNTIME-WAIVE** (no live mask conflict found — full-tree `f204` grep clean) | convert to `u8`+pinned shift macros anyway (abi.md §3 policy) |
| Disc TOC/archive images | D1 `GslArchiveEntry` | **FORMAT-KEEP — currently UNSWAPPED**, highest-priority flip site | add BE accessor or swap-on-load pass |
| Texture archives | D2–D4 `GvmTextureHeader`, `GvrTextureChunk`, GVR payload | **FORMAT-KEEP** | DC-legacy `bswap32_in_place` sites: make identity on LE host (working-value rule §2.4); pixel bytes untouched |
| Network-info config blob | D5–D10 `NetworkInfoConfig` family | **FORMAT-KEEP** (offset-pinned load path) | accessors over `b[]`/`c[]`/`d[]` sub-blobs; keep the 7 offset asserts as format guards |
| Movie TOC/frames | D11 `MfciFileEntry`, `MovieFramePack` | **mixed / verify** | entry offsets from stream → swap-on-load check at `Mfci.cpp` before pooling; frame RLE is byte math, port as-is |
| Decompressors | `PrsDecoder`, `Lzss`, MD5/VJ | endian-clean | port verbatim; outputs stay byte-true |

## 5. Swap-layer placement recommendation (one page)

**One header, three namespaces, zero scattered conditionals.** pso-metal gets
`compat/pso_byteswap.h` (companion to `pso_metal_compat.h`):

```
/* Regime A — game protocol: wire == LE. Host LE ⇒ identity; host BE ⇒ swap. */
static inline u16 psoProt16(u16 v){ return v; }   /* #if __BYTE_ORDER__==BIG: __builtin_bswap16 */
static inline u32 psoProt32(u32 v){ return v; }
/* Regime B + Disc — wire/blob == BE. Host LE ⇒ swap. */
static inline u16 psoBe16(u16 v){ return __builtin_bswap16(v); }
static inline u32 psoBe32(u32 v){ return __builtin_bswap32(v); }
/* psoProtSwapAll()/psoBeSwapAll(): the in-place struct walkers — see below. */
```

Placement rules, in order:

1. **Regime A stays where the game already put it.** Do *not* delete the ~150
   `bswap()` calls / `bswap_16`/`bswap_32` primitives
   (`protocol.h:11-12`, `NetByteOrder`-independent). Redefine the two primitives once in
   the port build (`-DPSO_HOST_IS_WIRE_ORDER` ⇒ no-ops). The calls then self-document
   the boundary, stay audit-visible, and the tree keeps compiling identically. Gate: any
   new protocol struct **must** carry a `bswap()`/`psoProtSwapAll()` even though it is a
   no-op on this host.
2. **Regime B gets accessors at the struct, not at the call site.** Convert
   `TcpHdr`/`IPv4Header`/`EthernetFrameHeader` field R/W to `psoBe*` (or typedef
   `be_u16`/`be_u32` read/write wrappers) inside the 10 stack headers; `NetByteOrder.c`
   and PPP byte-serial code untouched. If/when the stack is swapped for BSD sockets, the
   whole regime-B row retires at once — that isolation is the point.
3. **Disc: swap-on-load, once, at the DMA boundary.** Single chokepoints are
   `open_gsl_archive()` (TOC pass over `gsl_archive_entries` fixing D1's three s32),
   `gvm_stream_read`/`load_gvm_texture_archive` (D2/D3), and the `NetworkInfoLoader`
   blob reader. Everything downstream (TGStream seeks, texlist builds, PRS calls) then
   sees native LE host values and stays swap-free. **Do not** put swaps in per-field
   readers sprinkled through render code.
4. **Invariant + CI gate:** no struct that overlays wire or disc bytes is ever read with
   a host-order multi-byte load without going through rule 1/2/3. Grep gates (extend
   be_scan.py): `->(len|id|frag|cksum|win|sum|seq|ack|sport|dport|ether_type|archive_offset|file_size|decoded_size)`
   raw loads, `(u8*)&` casts into non-u8 format structs, `:N;` bitfields in FORMAT-KEEP
   groups (currently zero after §3 — none of the 21 bitfields is format-embedded), and
   any `#pragma pack` touching a regime-A record (forbidden; layout is the format).
5. **64-bit alignment reserve:** `#pragma pack(push,4)` per abi.md §2-C only if a future
   audited struct adds a `u64` — none of the 314+10+11 audited records has one today.

## 6. Numbers & unknowns

- Wire records audited: **314** game-protocol structs (+10 raw-stack frame headers, +14
  modem/PPP state structs reviewed).
- Disc-image records audited: **11** offset-pinned structs across 4 archive families
  (GSL, GVM/GVR, network-info, MFCl) + 3 byte-serial codec paths (PRS, LZSS, ADXF DMA).
- Assert-failing types crossing a format boundary: **0 of ~20** — abi.md's Cause-A/B
  cleanup is provably independent of the netcode/disc work.
- Unknowns: D11 (Mfci TOC read path) needs a direct read before Mfci is ported; the
  GC-disc confirmation of D2/D3 direction rests on the swap-then-compare pattern in the
  decomp (BE disc image assumption consistent with boot-spine S8 chunk notes);
  `disp[0xD0]` char-save internal layout is a further sub-format (item/save lane), opaque
  at this boundary.
