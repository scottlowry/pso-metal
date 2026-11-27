# P2-AS — AN-4 triage: `init_texture_header_and_alloc(u8*,s32,s32,s32)` DOES have a definition

Lane AS (triage, zero builds). Resolves deviation **AN-4** of `docs/p2-tex-supply.md`
(§3 + §4). Evidence source: pso-decomp `6b38b5383` (read-only greps + era `.s`).

## 1. Verdict: class (a) — defined, but under a DIFFERENT PROTOTYPE; AN's signature grep missed it

The function exists **under exactly the name AN grepped for**, in a TU AN's probe
didn't land on:

- **Definition:** `src/pso/unk/text_803DA12C.cpp:9` — `void init_texture_header_and_alloc(void *p)`
  (decomp's documented prototype, `include/pso/unk/text_803DA12C.h:7`, is ONE arg / `void`).
- **Caller's forward decl:** `src/pso/unk/text_803D96A8.cpp:23` — `s32 init_texture_header_and_alloc(u8 *, s32, s32, s32)`
  (FOUR args / `s32`) — a second, incompatible `extern "C"` declaration in a different TU.

So `grep` for the 4-arg spelling ("`init_texture_header_and_alloc(u8*`", "`s32 init_texture_header`")
finds only the forward decl → AN's "no definition" reading. Grep for the bare name finds the
definition. AN's doc §3 claim "has **no definition in the decomp at 6b38b5383** (forward decl only,
text_803D96A8.cpp:23)" is **FALSE as written**: the definition is at text_803DA12C.cpp:9.

The 4-arg call still *works* at the era level because of the PPC register ABI: the wrapper at
0x803DA12C never touches r4/r5/r6, so `width`/`height`/`format` pass through it **transparently**
into the real 4-arg texel-sizer two bl's down. The mismatched decls are decomp authoring noise
(byte-matching is per-TU against `.s`; the arg-count fiction costs nothing in the decomp build).

## 2. Era `.s` chain (logo/sin-board texture init), address:ins

Caller `init_texture_from_gvr_desc` @ 0x803D9DA8 (size 0x5C, unit `text_803D96A8`):

```
build/GPOE8P/asm/pso/unk/text_803D96A8.s:569
  803D9DBC  38 63 00 04   addi r3, r3, 0x4        ; arg1 = texture->p + 4  (header ptr)
  803D9DC0  A0 86 00 04   lhz  r4, 0x4(r6)        ; arg2 = desc width
  803D9DC4  A0 A6 00 06   lhz  r5, 0x6(r6)        ; arg3 = desc height
  803D9DC8  80 C6 00 00   lwz  r6, 0x0(r6)        ; arg4 = desc format word
  803D9DCC  48 00 03 61   bl   init_texture_header_and_alloc   ; → 0x803D9DD0+0x35C = 0x803DA12C
```

Wrapper @ 0x803DA12C (size 0x40) — only r3 is consumed; r4–r6 flow through:

```
build/GPOE8P/asm/pso/unk/text_803DA12C.s:11-13,16
  803DA13C  7C 7F 1B 78   mr   r31, r3
  803DA140  48 00 0F 6D   bl   init_ginja_texture_header       ; → 0x803DA144+0xF68 = 0x803DB0AC
  803DA150  7F E3 FB 78   mr   r3, r31
  803DA154  48 00 10 61   bl   alloc_buffer_from_heap          ; → 0x803DA158+0x1060 = 0x803DB1B4
```

The real 4-arg texel-sizer @ 0x803DB0AC (size 0xB0):

```
build/GPOE8P/asm/pso/render/GinjaTextureHeader.s:166
  803DB138  48 00 00 25   bl   get_texture_buffer_size__FP18GinjaTextureHeader  ; → 0x803DB15C
```

`symbols.txt` address→name map (`config/GPOE8P/psov3/symbols.txt`):

```
26636: init_texture_header_and_alloc = .text:0x803DA12C; // type:function size:0x40
26669: init_ginja_texture_header     = .text:0x803DB0AC; // type:function size:0xB0
26670: get_texture_buffer_size__FP18GinjaTextureHeader = .text:0x803DB15C; // type:function size:0x58
26671: alloc_buffer_from_heap        = .text:0x803DB1B4; // type:function size:0x58
```

Every bl target resolves to a named, **defined, byte-matched** function. No gap anywhere
in the chain (class (d) excluded; class (b) excluded — plain `.cpp` TUs, no header-only/macro
trick; class (c) partly true but the name AN used *is* the era-linked name).

## 3. Where AN-4's actual question ("the bpp table") lives

`hdr.size` is computed one hop deeper than AN assumed — **already fully decompiled and readable**:

- `src/pso/render/GinjaTextureHeader.cpp:62` `init_ginja_texture_header(GinjaTextureHeader*, u32
  width, u32 height, u32 format)` — fills format/width/height/flags (0x100→mipmap|0x80000000,
  0x200→0x8000 colour-indexed, 0x800→0x40000000), then `header->size = get_texture_buffer_size(header)`.
- `src/pso/render/GinjaTextureHeader.cpp:85` `get_texture_buffer_size` = `GXGetTexBufferSize(width,
  height, format, mipmap, 100)` — **the bpp table is the Dolphin SDK's**, at
  `libs/dolphin/src/gx/GXTexture.c` (decomp repo), not in PSO game code.
- `src/pso/render/GinjaTextureHeader.cpp:95` `alloc_buffer_from_heap` = `OSAllocFromHeap(
  locked_cache_heap_handle, header->size)` → `header->pixels`, ret 3 on OOM.

AN-4's stated consumer obligation ("re-derive size from the real header fn") is satisfiable
**verbatim from decomp source today** — nothing needs hand-tracing from `.s`.

## 4. ACTION for the next lane (gx-consumer / texture-upload lane)

Not needs-slice, not upstream-gap, not reachable-after-stitch. **Correct-function-name + port map:**

Implement the pso-metal `init_texture_header_and_alloc` seam as the era composition — there is no
single era fn with AN's 4-arg signature; the era 4-arg behavior is wrapper ⊕ tail-call:

| era addr | era name | decomp source | port to pso-metal as |
|---|---|---|---|
| 0x803DA12C | `init_texture_header_and_alloc(void*)` | `src/pso/unk/text_803DA12C.cpp:9-15` | 1-arg wrapper: header-init, then pixel-alloc iff ret==0 |
| 0x803DB0AC | `init_ginja_texture_header` | `src/pso/render/GinjaTextureHeader.cpp:62-82` | the 4-arg field-fill + size compute (AN's `s32(u8*,s32,s32,s32)` shape lives HERE) |
| 0x803DB15C | `get_texture_buffer_size` | `src/pso/render/GinjaTextureHeader.cpp:85-92` | bpp-table call — needs host `GXGetTexBufferSize` (from `libs/dolphin/src/gx/GXTexture.c`, SDK-side; same GX lane territory as GXInitTexObj*) |
| 0x803DB1B4 | `alloc_buffer_from_heap` | `src/pso/render/GinjaTextureHeader.cpp:95-103` | heap-alloc seam (host allocator injection point; AE/Y copy-leg regime) |

pso-metal grep confirms NONE of these four names exists in pso-metal src/include yet — this is a
clean 4-function port, all four byte-matched era bodies are readable in the decomp table above.
The only era-library dependency is `GXGetTexBufferSize`'s bpp table (Dolphin SDK source is in the
decomp repo, so no gap).

## 5. Also fix when convenient (pso-metal's own docs, not decomp)

AN-4's wording "has no definition in the decomp at 6b38b5383 (forward decl only,
text_803D96A8.cpp:23)" → "is defined as a 1-arg wrapper at text_803DA12C.cpp:9 whose r4–r6 pass
through to the 4-arg `init_ginja_texture_header` (GinjaTextureHeader.cpp:62) + bpp table
`get_texture_buffer_size` (:85) + `alloc_buffer_from_heap` (:95)". The PAL8=1B/texel golden
sanity check stays valid (GVRT `size`=0x408 field is era-authoritative regardless); it just no
longer has to pretend the sizing fn is a black box.

Greps run for this triage (case-insensitive, word-root): `texture_header`, `init_texture` over
decomp `src/` + `include/` (hits listed above; full variant sweep found no other spelling —
`.fn`/symbol names in `asm/` use the same base names quoted here); `symbols.txt` addr grep on
0x803DA12C / 0x803DB0AC / 0x803DB15C / 0x803DB1B4; bl-decoding by insn words
(`48 00 03 61`→0x803DA12C, `48 00 0F 6D`→0x803DB0AC, `48 00 10 61`→0x803DB1B4,
`48 00 00 25`→0x803DB15C), each cross-checked against the `.s` unit's `.fn` block.
