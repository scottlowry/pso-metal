# BE-Hazard Index — Phase-0 task F

Machine-scanned inventory of big-endian-dependent patterns in the pso-decomp
source, produced by `tools/be_scan.py` (stdlib-only, reproducible):

```
python3 tools/be_scan.py            # from repo root; --root to override decomp path
```

Scanned: **3,814 files** (`src/` + `include/` of pso-decomp: 1,600 .cpp,
2,134 .h, 80 .c; comments stripped before matching). **629 files (16.5%)
carry ≥1 hazard**; 93 carry a risk score ≥20; median risk of flagged files is
5. Full per-file data: `data/be-hazard-index.csv`, sorted by risk desc.

## Hazard categories

| cat | pattern | files | hits |
|-----|---------|-------|------|
| 1 | C bitfields (MSB/LSB packing differs PPC↔arm64) | 1 | 3 |
| 2 | byte-casts `(u8*)`/`(char*)` + assembly/split shifts `<<8/16/24` | 548 | 3,080 |
| 3 | explicit byte-swap machinery already present (`bswap_*`, `byte_swap`, `swpb`, `ntohl`-style) | 119 | 1,660 |
| 4 | multi-byte member reads via byte-offset pointer math (`*(s32*)((s8*)e+0x20)`, `buf[i]>>16`) | 248 | 995 |
| 5 | unions with mixed-width integer/float members (type-punning) | 23 | 26 |

**Dominant category: cat2** (byte-cast/shift assembly, 548 files). This is also
the noisiest — see Limitations. Bitfields are almost absent from this codebase
(one real site: `include/pso/network/NetTcpState.h`, 3 fields), so PPC-vs-ARM
bitfield packing is a small, named surface, not a systemic one.

## Risk formula

```
risk_score = max(0, 4·cat1 + 2·cat2 + 3·cat4 + 5·cat5 − 2·cat3)
```

cat5 (mixed-width unions) is weighted heaviest — reads through them silently
return wrong values under the flipped order. cat4 is precise evidence of
offset reinterpretation (weight 3). cat1 is the classic bitfield bug (4). cat2
is a candidate pool needing human triage (2). cat3 earns credit: the author
was already endianness-aware at that site (−2).

## Top 20 riskiest files (why)

| risk | file | why (evidence at worst line) |
|------|------|------------------------------|
| 956 | `src/musyx/runtime/synthmacros.c` | FM-synth params packed into words and byte-extracted (`((u8*)&…atime)[2]<<16`); 256 shift/cast + 148 offset sites |
| 178 | `src/pso/quest/AskDicConvert.cpp` | quest dictionary stream parsed byte-assembly style (`p[0] + (p[1]<<8)`) throughout |
| 158 | `src/pso/ui/frontend/TAdSinglePlyChrSelectGC_801FBF54.cpp` | manual word stitch/split from char-file buffers (`buf[3] \| buf[2]<<8 \| …`) |
| 139 | `src/pso/ui/charcreate/TObjCMName.cpp` | wide members read/written through `(s8*)base + off` puns + a mixed-width union |
| 134 | `src/pso/network/PppFraming.c` | network framing: `(p[2]<<8)\|p[3]` MSS/option parsing, byte-offset frame pointers, mixed union |
| 127 | `src/musyx/runtime/hw_dspctrl.c` | DSP command words split via `>>16`/`<<16` into shared-memory mailbox slots |
| 119 | `src/pso/audio/AdxAiff.cpp` | container parser with explicit `READ_LE32(b)` macros — file mixes BE/AIFF-C and LE/WAV orderings |
| 111 | `src/pso/system/save/McFsGc.cpp` | memory-card file entries addressed as `(u8*)base + off` wide-member puns |
| 96 | `src/pso/quest/QuestDownloadRequests.cpp` | quest-text/param blocks walked with byte casts and offset shifts |
| 96 | `src/pso/network/PppMd5.c` | MD5 assembles LE words from bytes (`input[j] \| <<8 \| <<16 \| <<24`) — correct-LE code written for BE data path |
| 95 | `src/pso/player/TObjPlayer_801AF2F0.cpp` | char-save blob walked by offset (`*(u8*)((s8*)char_data+3)`); already has bswap helpers (c3=22) |
| 84 | `src/pso/item/TItemData.cpp` | items own a `bswap_fields()` — wire-vs-RAM byte order is live here (c3=14 credit applied) |
| 82 | `src/musyx/runtime/stream.c` | sample/loop points extracted with `>>24`/`<<16` from packed stream words |
| 75 | `src/pso/unk/TLobby_801E2F94.cpp` | contains a hand-inlined 32-bit byteswap (`(v>>24)\|(v>>8)&0xFF00\|…`) — swap logic to review on LE |
| 72 | `src/pso/system/movie/MovieFramePack.cpp` | movie RLE packs high/low bytes (`hi = uw>>8`) into frame words |
| 69 | `src/pso/quest/QuestOpcodeHandlers.cpp` | quest opcode operands fetched via `(s8*)+off` wide puns (c4=19) alongside existing bswap sites (c3=30) |
| 68 | `src/pso/unk/text_803DD5E0.cpp` | second hand-inlined byteswap with mask form — endian-critical helper |
| 67 | `src/pso/ui/hud/TMainCursor.cpp` | HUD object fields written as `*(float*)((char*)a0 + 0x8)` offset floats |
| 62 | `src/musyx/runtime/s_data.c` | synth pool blocks walked through `(u8*)m + m->nextOff` then re-read as wide types |
| 61 | `src/pso/ui/charcreate/TCMCharacter.cpp` | creation-model structs copied as `*(u32*)((char*)p + off)` runs |

## Limitations (honest)

**False positives.**
- cat2's `<<16`/`>>16` regex cannot distinguish byte assembly from
  fixed-point math (audio pan/volume, `si->pan << 16`) or scale factors;
  musyx files are inflated by fixed-point shifts that are value-semantics,
  not byte order. Triage, don't auto-fix.
- `(u8*)base + off` arithmetic is endian-neutral *if* the referenced struct's
  field order is preserved — cat2/cat4 flag review candidates, not proven bugs.
- The byte-pointer-math cat4 pattern also matches plain string/struct walking
  (e.g. `strcpy((char*)…)`) where no multi-byte reinterpretation happens.
- Counts are per-occurrence: hand-unrolled code can stack 4 hits on one
  logical read. A single hit inside a hot loop is worth more than the CSV shows.

**False negatives.**
- cat1 anchors on line start, so inline anonymous-struct bitfields
  (`struct { signed int a : 8; u8 r, g, b; }` in `TEF_LensFlare.h`) are missed.
- cat3 only credits known swap *names*. Custom endianness helpers with other
  names (`READ_LE32`/`READ_BE32` in `AdxAiff.cpp`) get zero credit despite the
  file being endian-aware — that file looks riskier than it is.
- Hand-inlined shifts-byteswap (TLobby, text_803DD5E0) count as *hazards* with
  no cat3 credit. Directionally right (they must be reviewed on LE) but it
  conflates "performs a swap" with "assumes BE".
- cat5 inspects the first token of each union member: typedef'd wire types,
  bitfield members, or nesting via included structs can escape width detection.
  `f32` overlapping `u32` is width-1-distinct and correctly *not* flagged, yet
  is still an endian reinterpretation site — cat5 only catches mixed *widths*.
- Dolphin SDK (`libs/PowerPC_EABI_Support`, 315 files) and headers outside
  `src|include` are out of scope; register-level BE accesses there are real
  work the index doesn't see.
- File-level cat3 credit is crude: one `bswap_32` in a 3,000-line file offsets
  every unrelated shift in it (capped at −2×cat3 hits, still fuzzy).

Use the index to *prioritize review*, not to auto-transform. A clean wave
should still start with cat5 files (26 sites, human-verifiable in one sitting),
then the network/packet layer where cat3 credit already concentrates.
