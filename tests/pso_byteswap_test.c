/* pso-metal tests/pso_byteswap_test.c — runtime gate for compat/pso_byteswap.h (P1-M).
 *
 * Four families, per docs/p1-format-net-disc.md §5 and docs/p1-format-saves.md §5.3:
 *   T1  roundtrip identity for every accessor at every width (16/32/64)
 *   T2  known byte-pattern -> expected host value, per accessor (incl. the
 *       GVRT "working value" trap and a D1-style GSL TOC s32)
 *   T3  unaligned buffer reads/writes (memcpy seam: offsets 1,3,7 — also
 *       caught under -fsanitize=undefined by run_byteswap.sh)
 *   T4  cipher sequencing: swap-around-cipher is byte-identical to the PPC
 *       semantics; ciphering raw LE words is garbage (the G finding)
 *
 * Exit 0 = all pass. No external deps beyond libc.
 */
#include <stdio.h>
#include <string.h>

#include "pso_byteswap.h"

static int g_total, g_pass, g_fail;

#define CHECK(name, cond) do {                                        \
    g_total++;                                                        \
    if (cond) { g_pass++; }                                           \
    else { g_fail++; printf("FAIL %s (line %d)\n", (name), __LINE__); }\
} while (0)

#define HEX32 "0x%08x"

/* ------------------------------------------------------------------ *
 * T1 — roundtrip identity, all widths, both namespaces + walkers.
 * ------------------------------------------------------------------ */
static uint32_t lcg(uint32_t *s) { *s = *s * 1664525u + 1013904223u; return *s; }

static void t1_roundtrip(void)
{
    uint32_t seed = 0xC0FFEEu;
    int i;

    /* exhaustive 16-bit */
    for (i = 0; i <= 0xFFFF; i++) {
        uint16_t v = (uint16_t)i;
        if (psoBe16(psoBe16(v)) != v || psoProt16(psoProt16(v)) != v) break;
    }
    CHECK("T1 psoBe16/psoProt16 exhaustive roundtrip", i > 0xFFFF);

    for (i = 0; i < 4096; i++) {
        uint32_t v = lcg(&seed);
        uint64_t w = ((uint64_t)lcg(&seed) << 32) | lcg(&seed);
        if (psoBe32(psoBe32(v)) != v) { CHECK("T1 psoBe32 roundtrip", 0); return; }
        if (psoProt32(psoProt32(v)) != v) { CHECK("T1 psoProt32 roundtrip", 0); return; }
        if (psoBe64(psoBe64(w)) != w) { CHECK("T1 psoBe64 roundtrip", 0); return; }
        if (psoProt64(psoProt64(w)) != w) { CHECK("T1 psoProt64 roundtrip", 0); return; }
        /* buffer accessors are involutions through the image */
        uint8_t b[8];
        psoBeWrite64(b, w);
        if (psoBeRead64(b) != w) { CHECK("T1 psoBe buffer roundtrip", 0); return; }
        psoProtWrite64(b, w);
        if (psoProtRead64(b) != w) { CHECK("T1 psoProt buffer roundtrip", 0); return; }
    }
    CHECK("T1 32/64-bit + buffer roundtrips (4096 samples)", 1);

    /* unconditional swaps compose: psoBe = swap on LE host, = id on BE host */
#if PSO_HOST_IS_BIG_ENDIAN
    CHECK("T1 psoBe == identity on BE host", psoBe32(0xDEADBEEFu) == 0xDEADBEEFu);
    CHECK("T1 psoProt == bswap on BE host", psoProt32(0xDEADBEEFu) == 0xEFBEADDEu);
#else
    CHECK("T1 psoBe == bswap on LE host", psoBe32(0xDEADBEEFu) == 0xEFBEADDEu);
    CHECK("T1 psoProt == identity on LE host", psoProt32(0xDEADBEEFu) == 0xDEADBEEFu);
#endif

    /* in-place walkers: self-inverse on random buffers */
    uint8_t buf[64], orig[64], ref[64];
    for (i = 0; i < 64; i++) buf[i] = (uint8_t)lcg(&seed);
    memcpy(orig, buf, 64);
    psoBeSwapAll32(buf, 16); psoBeSwapAll32(buf, 16);
    CHECK("T1 psoBeSwapAll32 self-inverse", memcmp(buf, orig, 64) == 0);
    /* one psoCipherSwap pass must equal one psoBeSwapAll32 pass */
    memcpy(ref, orig, 64); psoCipherSwap(ref, 16);
    memcpy(buf, orig, 64); psoBeSwapAll32(buf, 16);
    CHECK("T1 psoCipherSwap == psoBeSwapAll32", memcmp(buf, ref, 64) == 0);
    psoCipherSwap(buf, 16);
    CHECK("T1 psoCipherSwap self-inverse", memcmp(buf, orig, 64) == 0);
    psoProtSwapAll16(buf, 32); psoProtSwapAll64(buf, 8);
    memcpy(orig, buf, 64);
    psoBeSwapAll16(buf, 32); psoBeSwapAll16(buf, 32);
    psoBeSwapAll64(buf, 8); psoBeSwapAll64(buf, 8);
    CHECK("T1 psoBeSwapAll16/64 self-inverse", memcmp(buf, orig, 64) == 0);
#if !PSO_HOST_IS_BIG_ENDIAN
    memcpy(orig, buf, 64);
    psoProtSwapAll32(buf, 16);
    CHECK("T1 psoProtSwapAll32 identity on LE host", memcmp(buf, orig, 64) == 0);
#endif
}

/* ------------------------------------------------------------------ *
 * T2 — known byte images -> expected host values, per accessor.
 * ------------------------------------------------------------------ */
static void t2_known_patterns(void)
{
    /* Regime B / disc image: RFC-BE byte order. */
    const uint8_t be16img[2] = { 0x12, 0x34 };
    const uint8_t be32img[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
    const uint8_t be64img[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    CHECK("T2 psoBeRead16 1234", psoBeRead16(be16img) == 0x1234u);
    CHECK("T2 psoBeRead32 DEADBEEF", psoBeRead32(be32img) == 0xDEADBEEFu);
    CHECK("T2 psoBeRead64 0102030405060708",
          psoBeRead64(be64img) == 0x0102030405060708ull);

    uint8_t out[8];
    psoBeWrite16(out, 0x1234u);
    CHECK("T2 psoBeWrite16 -> RFC bytes", memcmp(out, be16img, 2) == 0);
    psoBeWrite32(out, 0xDEADBEEFu);
    CHECK("T2 psoBeWrite32 -> RFC bytes", memcmp(out, be32img, 4) == 0);
    psoBeWrite64(out, 0x0102030405060708ull);
    CHECK("T2 psoBeWrite64 -> RFC bytes", memcmp(out, be64img, 8) == 0);

    /* Regime A: game-protocol LE image (packet_header.size 0x01F4 = 500 ->
     * bytes F4 01; newserv-compatible wire). */
    const uint8_t le16img[2] = { 0xF4, 0x01 };
    const uint8_t le32img[4] = { 0x78, 0x56, 0x34, 0x12 };
    const uint8_t le64img[8] = { 0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01 };
    CHECK("T2 psoProtRead16 packet size 500", psoProtRead16(le16img) == 500u);
    CHECK("T2 psoProtRead32 12345678", psoProtRead32(le32img) == 0x12345678u);
    CHECK("T2 psoProtRead64 0123456789ABCDEF",
          psoProtRead64(le64img) == 0x0123456789ABCDEFull);
    psoProtWrite16(out, 500u);
    CHECK("T2 psoProtWrite16 -> LE bytes", memcmp(out, le16img, 2) == 0);
    psoProtWrite32(out, 0x12345678u);
    CHECK("T2 psoProtWrite32 -> LE bytes", memcmp(out, le32img, 4) == 0);
    psoProtWrite64(out, 0x0123456789ABCDEFull);
    CHECK("T2 psoProtWrite64 -> LE bytes", memcmp(out, le64img, 8) == 0);

    /* D1 (GSL TOC): an archive_offset of 1 MiB stored BE on the platter. */
    const uint8_t d1_off[4] = { 0x00, 0x10, 0x00, 0x00 };
    CHECK("T2 D1 gsl archive_offset BE->host 0x100000",
          psoBeRead32(d1_off) == 0x00100000u);

    /* D2/D3 trap (net-disc §2.4): the DC-legacy GVRT loaders swapped AFTER
     * a BE host-order load, so the PPC *working value* of disc bytes
     * 'G','V','R','T' is 0x54525647 ("TRVG"), not 0x47565254. On arm64 the
     * raw LE load of the same bytes already yields the working value, so
     * the port-side swap there must be IDENTITY — i.e. those sites take
     * psoProtRead32 (host-order raw read), while never-swapped D1-style
     * fields take psoBeRead32. Same bytes, opposite accessors: this is the
     * single easiest place to get the direction wrong. */
    const uint8_t gvrt[4] = { 'G', 'V', 'R', 'T' }; /* 47 56 52 54 on platter */
    CHECK("T2 GVRT working value via psoProtRead32 == 0x54525647",
          psoProtRead32(gvrt) == 0x54525647u);
    CHECK("T2 GVRT raw BE interpretation via psoBeRead32 == 0x47565254",
          psoBeRead32(gvrt) == 0x47565254u);
    CHECK("T2 TcpHdr-style: ether_type 0x0800 image",
          psoBeRead16((const uint8_t[]){ 0x08, 0x00 }) == 0x0800u);
}

/* ------------------------------------------------------------------ *
 * T3 — unaligned images. All buffer accessors route through memcpy, so
 * reads/writes at non-natural offsets are value-correct and trap-free
 * (run_byteswap.sh re-runs this under UBSan for the alignment checker).
 * ------------------------------------------------------------------ */
static void t3_unaligned(void)
{
    /* one backing store, image deliberately misaligned in every way */
    static const uint8_t pattern32[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
    static const uint8_t pattern64[8] = { 0x01, 0x02, 0x03, 0x04,
                                          0x05, 0x06, 0x07, 0x08 };
    uint8_t buf[96];
    const size_t offs[] = { 1, 3, 7, 13, 31 };
    size_t k;
    int bad = 0;

    for (k = 0; k < sizeof offs / sizeof offs[0]; k++) {
        size_t o = offs[k];
        memset(buf, 0x5A, sizeof buf);
        memcpy(buf + o, pattern32, 4);
        memcpy(buf + o + 8, pattern64, 8);
        if (psoBeRead32(buf + o) != 0xDEADBEEFu) bad = 1;
        if (psoBeRead64(buf + o + 8) != 0x0102030405060708ull) bad = 1;
        psoBeWrite32(buf + o, 0xCAFEBABEu);
        if (memcmp(buf + o, (const uint8_t[]){ 0xCA, 0xFE, 0xBA, 0xBE }, 4) != 0) bad = 1;
        psoProtWrite16(buf + o, 0x1234u);
        if (psoProtRead16(buf + o) != 0x1234u) bad = 1;
        /* in-place walker on an unaligned start */
        memcpy(buf + o, pattern32, 4);
        psoBeSwapAll32(buf + o, 1);
        if (memcmp(buf + o, (const uint8_t[]){ 0xEF, 0xBE, 0xAD, 0xDE }, 4) != 0) bad = 1;
        /* untouched neighbour byte check (no overrun) */
        if (buf[o - 1] != 0x5Au) bad = 1;
    }
    CHECK("T3 unaligned read/write/swap at offsets {1,3,7,13,31}, no overrun", !bad);

    /* raw memcpy of the same bytes must equal psoBeRead XOR host order:
     * proves the accessor did not change the image in place. */
    memset(buf, 0, sizeof buf);
    memcpy(buf + 3, pattern32, 4);
    uint32_t raw;
    memcpy(&raw, buf + 3, 4);
    CHECK("T3 image bytes preserved by psoBeRead32 (read-only seam)",
          psoBeRead32(buf + 3) == psoBe32(raw));
}

/* ------------------------------------------------------------------ *
 * T4 — swap-around-cipher (the G finding, docs/p1-format-saves.md §5.3).
 *
 * Local model of SubtractiveRng semantics ONLY (word-subtract cipher
 * data[i] = next() - data[i] over u32 words, 55-cell subtractive lag,
 * set_seed per the read-only decomp at src/pso/system/SubtractiveRng.cpp).
 * NOT a byte-for-byte validated copy of the game binary's generator —
 * the test needs a deterministic word-subtract stream with correct
 * *structure*, nothing more.
 * ------------------------------------------------------------------ */
typedef struct { uint32_t table[56]; int pos; } SubRng;

static void subrng_refill(SubRng *r)
{
    int i;
    for (i = 1; i < 25; i++)  r->table[i] -= r->table[i + 31];
    for (i = 25; i < 56; i++) r->table[i] -= r->table[i - 24];
}

static void subrng_seed(SubRng *r, uint32_t seed)
{
    uint32_t prev = 1, s = seed;
    int i;
    r->table[55] = seed;
    for (i = 1; i < 55; i++) {
        int idx = (21 * i) % 55;
        r->table[idx] = prev;
        prev = s - prev;
        s = r->table[idx];
    }
    subrng_refill(r); subrng_refill(r); subrng_refill(r); subrng_refill(r);
    r->pos = 55;
}

static uint32_t subrng_next(SubRng *r)
{
    int last = r->pos;
    r->pos = last + 1;
    if (last + 1 > 55) { subrng_refill(r); r->pos = 1; }
    return r->table[r->pos];
}

/* Ground truth = PPC semantics expressed endian-safely: each BE word is
 * read with psoBeRead32 (same integer PPC saw), ciphered, written back
 * with psoBeWrite32. Per-word accessors, no in-place trickery. */
static void cipher_reference(uint8_t *buf, size_t nwords, uint32_t seed)
{
    SubRng r; subrng_seed(&r, seed);
    size_t i;
    for (i = 0; i < nwords; i++) {
        uint32_t w = psoBeRead32(buf + 4 * i);
        w = subrng_next(&r) - w;              /* data[i] = next() - data[i] */
        psoBeWrite32(buf + 4 * i, w);
    }
}

/* Port path = whole-buffer swap around the cipher, exactly once each way. */
static void cipher_swaparound(uint8_t *buf, size_t nwords, uint32_t seed)
{
    SubRng r; subrng_seed(&r, seed);
    size_t i;
    psoCipherSwap(buf, nwords);               /* BE image -> host words */
    for (i = 0; i < nwords; i++) {
        uint32_t w;
        memcpy(&w, buf + 4 * i, sizeof w);    /* aligned-enough: our own buffer */
        w = subrng_next(&r) - w;
        memcpy(buf + 4 * i, &w, sizeof w);
    }
    psoCipherSwap(buf, nwords);               /* host words -> BE image */
}

/* Mis-sequenced path = cipher raw host-order words of the BE image. */
static void cipher_naive(uint8_t *buf, size_t nwords, uint32_t seed)
{
    SubRng r; subrng_seed(&r, seed);
    size_t i;
    for (i = 0; i < nwords; i++) {
        uint32_t w;
        memcpy(&w, buf + 4 * i, sizeof w);
        w = subrng_next(&r) - w;
        memcpy(buf + 4 * i, &w, sizeof w);
    }
}

#define PLAIN_WORDS 16u
static const uint32_t SEEDS[] = { 0x00000000u, 0x0000ABCDu, 0x12345678u, 0xDEADBEEFu };

static void t4_cipher(void)
{
    uint8_t plain[PLAIN_WORDS * 4], a[PLAIN_WORDS * 4], b[PLAIN_WORDS * 4];
    size_t i, s;

    /* a fixed pseudo-plaintext card payload */
    uint32_t seed = 42u;
    for (i = 0; i < sizeof plain; i += 4) {
        uint32_t v = lcg(&seed);
        psoBeWrite32(plain + i, v);           /* stored as BE image */
    }

    /* algebraic witness that the hazard is real: subtraction does not
     * commute with byte reversal. With a = 0x12345678 (an RNG word) and
     * v = 0xCAFEBABE (a BE card word): bswap(a - v) = 0xBA9B3547 but
     * a - bswap(v) = 0x537957AE. (A palindromic a like 0xA5A5A5A5 would
     * accidentally commute — avoided on purpose.) */
    CHECK("T4 bswap(a-v) != a-bswap(v) (non-commutativity witness)",
          psoSwap32(0x12345678u - 0xCAFEBABEu) == 0xBA9B3547u
          && (0x12345678u - psoSwap32(0xCAFEBABEu)) == 0x537957AEu);

    for (s = 0; s < sizeof SEEDS / sizeof SEEDS[0]; s++) {
        uint32_t sd = SEEDS[s];
        memcpy(a, plain, sizeof plain);
        memcpy(b, plain, sizeof plain);
        cipher_reference(a, PLAIN_WORDS, sd);
        cipher_swaparound(b, PLAIN_WORDS, sd);
        char nm[96];
        snprintf(nm, sizeof nm, "T4 seed %08X: swap-around == PPC reference", sd);
        CHECK(nm, memcmp(a, b, sizeof plain) == 0);

        cipher_naive(b, PLAIN_WORDS, sd);     /* restart: naive on plain */
        memcpy(b, plain, sizeof plain);
        cipher_naive(b, PLAIN_WORDS, sd);
        snprintf(nm, sizeof nm, "T4 seed %08X: naive per-word cipher != reference", sd);
#if !PSO_HOST_IS_BIG_ENDIAN
        CHECK(nm, memcmp(a, b, sizeof plain) != 0); /* garbage on LE host */
#else
        (void)b; (void)nm; /* on a BE host naive coincides — no-op seam */
        CHECK(nm, memcmp(a, b, sizeof plain) == 0);
#endif
    }

    /* decrypt = same sequence with RNG restarted (next()-x is involutive
     * only when the identical stream is replayed). */
    memcpy(a, plain, sizeof plain);
    cipher_swaparound(a, PLAIN_WORDS, SEEDS[2]);
    cipher_swaparound(a, PLAIN_WORDS, SEEDS[2]);
    CHECK("T4 swap-around cipher/decrypt roundtrip restores image",
          memcmp(a, plain, sizeof plain) == 0);

    /* the naive double-pass is self-inverse too — corruption is silent:
     * it round-trips against itself and only fails vs the reference. */
    memcpy(b, plain, sizeof plain);
    cipher_naive(b, PLAIN_WORDS, SEEDS[2]);
    cipher_naive(b, PLAIN_WORDS, SEEDS[2]);
    CHECK("T4 naive path also self-inverse (why the bug is silent)",
          memcmp(b, plain, sizeof plain) == 0);
}

int main(void)
{
    printf("pso_byteswap_test — host is %s-endian (PSO_HOST_IS_BIG_ENDIAN=%d)\n",
#if PSO_HOST_IS_BIG_ENDIAN
           "big", 1
#else
           "little", 0
#endif
           );
    t1_roundtrip();
    t2_known_patterns();
    t3_unaligned();
    t4_cipher();
    printf("TOTAL %d  PASS %d  FAIL %d\n", g_total, g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
