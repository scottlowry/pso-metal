/* pso-metal compat/pso_byteswap.h — the port's single byteswap seam (P1-M).
 *
 * WHY: docs/p1-format-net-disc.md §0 proves the game has TWO opposite endian
 * regimes, so a single global "swap all" or "delete all swaps" policy is wrong:
 *
 *   Regime A — game protocol (S_/C_/SC_*, G_* 6x, packet_header): LE on the
 *     wire. The PPC client swapped host→wire before send and wire→host after
 *     recv; on our LE arm64 host those ~150 in-tree bswap()/bswap_16/bswap_32
 *     call sites must collapse to identity — but stay as calls (audit-visible
 *     boundary markers), routed through the psoProt* family here.
 *
 *   Regime B + Disc — TCP/IP/PPP frame headers (TcpHdr, IPv4Header,
 *     EthernetFrameHeader) and disc images (GSL TOC, network-info blob):
 *     BE on the wire / on the platter. PPC read these as raw host-order
 *     integers (silent corruption sites on arm64); every multi-byte field
 *     must go through the psoBe* family here.
 *
 * Rules (per net-disc §5): Regime A conversions live at the existing
 * boundary calls; Regime B conversions live at the struct accessors; disc
 * conversion happens swap-on-load ONCE at the DMA chokepoints
 * (open_gsl_archive TOC pass, load_gvm_texture_archive/gvm_stream_read,
 * NetworkInfoLoader blob reader) — never in per-field readers downstream.
 * See compat/README-byteswap.md.
 *
 * Safety contract: every *buffer* accessor here loads/stores through memcpy,
 * never through a cast to uintN_t*, so unaligned images (disc TOC entries at
 * odd offsets, frame headers pulled from byte FIFOs) cannot raise an
 * alignment trap and the code carries no strict-aliasing UB on any host.
 *
 * C11, stdlib only. Host types are stdint names (uint16_t/...), NOT the
 * decomp's u16/u32 typedefs, so this header can be force-included alongside
 * include/pso/types.h without redefinition.
 */
#ifndef PSO_BYTESWAP_H
#define PSO_BYTESWAP_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ------------------------------------------------------------------ *
 * Host endianness. The port builds with clang (LP64 arm64 today), so
 * the predefined __BYTE_ORDER__ is the seam; a future BE host picks
 * the other branch and every identity below becomes a real swap with
 * zero source changes. Fail loudly rather than silently default to LE.
 * ------------------------------------------------------------------ */
#if defined(PSO_HOST_IS_BIG_ENDIAN)
   /* preprocessor override already set by the build (-D...=0/1) */
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && defined(__ORDER_LITTLE_ENDIAN__)
#  if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#    define PSO_HOST_IS_BIG_ENDIAN 1
#  elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#    define PSO_HOST_IS_BIG_ENDIAN 0
#  else
#    error "pso_byteswap: unknown __BYTE_ORDER__; add -DPSO_HOST_IS_BIG_ENDIAN=0|1"
#  endif
#else
#  error "pso_byteswap: compiler does not predefine __BYTE_ORDER__; add -DPSO_HOST_IS_BIG_ENDIAN=0|1"
#endif

/* The port is C11 (clang, LP64 arm64); the width gate below is a hard
 * compile-time requirement, so refuse anything older. */
#if defined(__cplusplus) || !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#  error "pso_byteswap.h requires a C11 compiler (-std=c11) for _Static_assert"
#endif
#define PSO_STATIC_ASSERT(cond, msg) _Static_assert((cond), msg)

/* ------------------------------------------------------------------ *
 * Compile-time width asserts. FORMAT-KEEP records are pinned by these
 * widths (net-disc §4: max member align 4 across all 314+10+11 audited
 * records; net-disc rule 5 reserves pack(4) for a future u64 — none
 * exists today). If a toolchain ever hands us different stdint widths,
 * this header refuses to build before any byte is misread.
 * ------------------------------------------------------------------ */
PSO_STATIC_ASSERT(sizeof(uint16_t) == 2, "uint16_t must be exactly 16 bits");
PSO_STATIC_ASSERT(sizeof(uint32_t) == 4, "uint32_t must be exactly 32 bits");
PSO_STATIC_ASSERT(sizeof(uint64_t) == 8, "uint64_t must be exactly 64 bits");

/* ------------------------------------------------------------------ *
 * Core unconditional reversals (host-independent byte mirrors).
 * ------------------------------------------------------------------ */
static inline uint16_t psoSwap16(uint16_t v) { return __builtin_bswap16(v); }
static inline uint32_t psoSwap32(uint32_t v) { return __builtin_bswap32(v); }
static inline uint64_t psoSwap64(uint64_t v) { return __builtin_bswap64(v); }

/* ------------------------------------------------------------------ *
 * Regime A — game protocol: wire bytes are LE.
 * psoProtNN converts between a wire-image value and a host working value.
 * On an LE host (arm64 today) these are the documented identity/no-ops;
 * on a BE host they become real swaps. They exist so the ~150 in-tree
 * bswap() sites have exactly one seam and stay structurally present.
 * ------------------------------------------------------------------ */
static inline uint16_t psoProt16(uint16_t v)
{ return PSO_HOST_IS_BIG_ENDIAN ? psoSwap16(v) : v; }
static inline uint32_t psoProt32(uint32_t v)
{ return PSO_HOST_IS_BIG_ENDIAN ? psoSwap32(v) : v; }
static inline uint64_t psoProt64(uint64_t v)
{ return PSO_HOST_IS_BIG_ENDIAN ? psoSwap64(v) : v; }

/* ------------------------------------------------------------------ *
 * Regime B + Disc: image bytes are BE (RFC order / PPC platter).
 * psoBeNN converts between a BE-interpreted value and a host working
 * value. On an LE host (arm64 today) these are real swaps.
 * ------------------------------------------------------------------ */
static inline uint16_t psoBe16(uint16_t v)
{ return PSO_HOST_IS_BIG_ENDIAN ? v : psoSwap16(v); }
static inline uint32_t psoBe32(uint32_t v)
{ return PSO_HOST_IS_BIG_ENDIAN ? v : psoSwap32(v); }
static inline uint64_t psoBe64(uint64_t v)
{ return PSO_HOST_IS_BIG_ENDIAN ? v : psoSwap64(v); }

/* ------------------------------------------------------------------ *
 * Unaligned-safe buffer accessors: byte image <--> host working value.
 * memcpy in, memcpy out; no cast of the (possibly misaligned) image
 * pointer to a wide integer type ever happens here.
 *
 * Naming: psoBeRead32(img) == psoBe32(memcpy-load of img); the value
 * returned is always the host-order integer whose RFC/BE byte image is
 * the bytes at img.
 * ------------------------------------------------------------------ */
#define PSO_BYTESWAP_DEFINE_READ(suf, type, valfn)                    \
    static inline type psoBeRead##suf(const void *img) {              \
        type t; memcpy(&t, img, sizeof t); return valfn(t);           \
    }                                                                 \
    static inline type psoProtRead##suf(const void *img) {            \
        type t; memcpy(&t, img, sizeof t); return psoProt##suf(t);    \
    }

#define PSO_BYTESWAP_DEFINE_WRITE(suf, type, valfn)                   \
    static inline void psoBeWrite##suf(void *img, type v) {           \
        type t = valfn(v); memcpy(img, &t, sizeof t);                 \
    }                                                                 \
    static inline void psoProtWrite##suf(void *img, type v) {         \
        type t = psoProt##suf(v); memcpy(img, &t, sizeof t);          \
    }

PSO_BYTESWAP_DEFINE_READ(16, uint16_t, psoBe16)
PSO_BYTESWAP_DEFINE_READ(32, uint32_t, psoBe32)
PSO_BYTESWAP_DEFINE_READ(64, uint64_t, psoBe64)
PSO_BYTESWAP_DEFINE_WRITE(16, uint16_t, psoBe16)
PSO_BYTESWAP_DEFINE_WRITE(32, uint32_t, psoBe32)
PSO_BYTESWAP_DEFINE_WRITE(64, uint64_t, psoBe64)

/* ------------------------------------------------------------------ *
 * In-place fixed-width walkers (the psoProtSwapAll()/psoBeSwapAll()
 * pair named by net-disc §5). n = element COUNTS, not byte counts, so
 * a short tail can never be half-swapped. Element loads/stores are
 * memcpy-based: the buffer start may be unaligned (e.g. a GVR chunk
 * header pulled from a sector at +12).
 *
 *   psoBeSwapAllNN  : flip a BE image in place between BE-interpreted
 *                     and host working order. Identity on a BE host.
 *   psoProtSwapAllNN: same, for a Regime-A LE image. Identity on an LE
 *                     host (the documented protocol no-op seam).
 * ------------------------------------------------------------------ */
/* Implementation note: the in-place flip converts the RAW bytes once
 * (load raw -> psoBe/psoProt -> store raw). Composing psoBeRead +
 * psoBeWrite would convert twice and be a silent identity. */
#define PSO_BYTESWAP_DEFINE_SWAPALL(suf, type)                        \
    static inline void psoBeSwapAll##suf(void *buf, size_t n) {       \
        unsigned char *p = (unsigned char *)buf;                      \
        size_t i;                                                     \
        for (i = 0; i < n; i++, p += sizeof(type)) {                  \
            type t; memcpy(&t, p, sizeof t);                          \
            t = psoBe##suf(t); memcpy(p, &t, sizeof t);               \
        }                                                             \
    }                                                                 \
    static inline void psoProtSwapAll##suf(void *buf, size_t n) {     \
        unsigned char *p = (unsigned char *)buf;                      \
        size_t i;                                                     \
        for (i = 0; i < n; i++, p += sizeof(type)) {                  \
            type t; memcpy(&t, p, sizeof t);                          \
            t = psoProt##suf(t); memcpy(p, &t, sizeof t);             \
        }                                                             \
    }

PSO_BYTESWAP_DEFINE_SWAPALL(16, uint16_t)
PSO_BYTESWAP_DEFINE_SWAPALL(32, uint32_t)
PSO_BYTESWAP_DEFINE_SWAPALL(64, uint64_t)

/* ------------------------------------------------------------------ *
 * psoCipherSwap — the save-card cipher seam (docs/p1-format-saves.md §3,
 * §5.3: the ONE case where a pure byteswap layer can silently corrupt).
 *
 * SubtractiveRng::apply_rng_cipher does WORD-ARITHMETIC on the card
 * image:   data[i] = next() - data[i]   (u32 words, len rounded up as
 * (len+3)/4 — per the documented semantics in src/pso/system/
 * SubtractiveRng.cpp). Subtraction does NOT commute with byte reversal:
 *   bswap(a - b) != a - bswap(b)
 * (counterexample in tests). So on an LE host you must NOT cipher the
 * raw host-order words of the BE card image, and you must NOT per-field
 * swap around individual reads. The correct sequence is WHOLE-BUFFER,
 * ONCE, around the cipher pass:
 *
 *   load BE card image buf (n 32-bit words, byte-true)
 *   psoCipherSwap(buf, n);          // BE-interpreted -> host words
 *   for i: w[i] = rng_next() - w[i] // cipher on host-order words,
 *                                   // exactly as PPC evaluated it
 *   psoCipherSwap(buf, n);          // host words -> BE image again
 *   store buf to card
 *
 * psoCipherSwap IS psoBeSwapAll32 (it is its own inverse); it gets its
 * own name because the swap-around-cipher pattern must never be written
 * with a hand-rolled per-field loop. On a future BE host it is identity
 * and the sequence degrades gracefully to the PPC original.
 * ------------------------------------------------------------------ */
static inline void psoCipherSwap(void *buf, size_t nwords32)
{ psoBeSwapAll32(buf, nwords32); }

/* Width re-checks through the public API surface (unevaluated calls). */
PSO_STATIC_ASSERT(sizeof(psoSwap16((uint16_t)0)) == 2, "psoSwap16 width");
PSO_STATIC_ASSERT(sizeof(psoSwap32((uint32_t)0)) == 4, "psoSwap32 width");
PSO_STATIC_ASSERT(sizeof(psoSwap64((uint64_t)0)) == 8, "psoSwap64 width");
PSO_STATIC_ASSERT(sizeof(psoProt16((uint16_t)0)) == 2, "psoProt16 width");
PSO_STATIC_ASSERT(sizeof(psoProt32((uint32_t)0)) == 4, "psoProt32 width");
PSO_STATIC_ASSERT(sizeof(psoProt64((uint64_t)0)) == 8, "psoProt64 width");
PSO_STATIC_ASSERT(sizeof(psoBe16((uint16_t)0)) == 2, "psoBe16 width");
PSO_STATIC_ASSERT(sizeof(psoBe32((uint32_t)0)) == 4, "psoBe32 width");
PSO_STATIC_ASSERT(sizeof(psoBe64((uint64_t)0)) == 8, "psoBe64 width");
PSO_STATIC_ASSERT(sizeof(psoBeRead16((const void *)0)) == 2, "psoBeRead16 width");
PSO_STATIC_ASSERT(sizeof(psoProtRead64((const void *)0)) == 8, "psoProtRead64 width");
PSO_STATIC_ASSERT(PSO_HOST_IS_BIG_ENDIAN == 0 || PSO_HOST_IS_BIG_ENDIAN == 1, "host endian flag is 0/1");

#endif /* PSO_BYTESWAP_H */
