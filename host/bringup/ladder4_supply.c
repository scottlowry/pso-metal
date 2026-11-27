/* P2-AN ladder4 supply helpers — THE one C11 TU of the rung so it can bind
 * compat/pso_byteswap.h directly (that header _Static_asserts on C11 and
 * refuses C++: "requires a C11 compiler (-std=c11)"). The C++ glue/test TUs
 * call these extern-"C" wrappers; every endian decision in the ladder thus
 * flows through the sanctioned psoProtNN / psoBeNN accessors - nothing here
 * hand-rolls a swap (lane rule; docs/p1-format-net-disc.md section 5,
 * compat/README-byteswap.md section 2).
 */
#include "../../compat/pso_byteswap.h"

#include <stdint.h>
#include <stddef.h>

/* --- regime conversions (value in, working value out) --- */
uint32_t p2an_prot32(uint32_t v) { return psoProt32(v); }
uint16_t p2an_prot16(uint16_t v) { return psoProt16(v); }
uint32_t p2an_be32(uint32_t v)   { return psoBe32(v); }
uint16_t p2an_be16(uint16_t v)   { return psoBe16(v); }

/* --- unaligned-safe buffer loads (byte image in, working value out) --- */
uint32_t p2an_prot_read32(const void *img) { return psoProtRead32(img); }
uint32_t p2an_be_read32(const void *img)   { return psoBeRead32(img); }
uint16_t p2an_be_read16(const void *img)   { return psoBeRead16(img); }

/* --- era reference mirror, VERBATIM expression from the decomp:
 * text_803D96A8.cpp:155-158 swap_gvrt_word body (also bswap32_in_place
 * text_803DD5E0.cpp:243-247; identical formula). Home 0x803D9C60; .s uses
 * clrlwi/slwi/rlwinm pairs (text_803D96A8.s :9C60-:9C84). Used ONLY to
 * compute the PPC-side working value in the golden arithmetic — the walk
 * itself calls the psoProt32 identity seam (deviation AN-1). */
uint32_t p2an_era_mirror32(uint32_t v) {
    return ((v & 0xFF000000) >> 24) | ((v & 0xFF0000) >> 8) |
           ((v & 0xFF00) << 8) | ((v & 0xFF) << 24);
}

/* --- FNV-1a 64-bit (stdlib-only, byte-serial, endian-neutral) --- */
uint64_t p2an_fnv1a64(const void *buf, size_t n) {
    const uint8_t *p = (const uint8_t *)buf;
    uint64_t h = 0xCBF29CE484222325ull;
    size_t i;
    for (i = 0; i < n; i++) { h ^= p[i]; h *= 0x100000001B3ull; }
    return h;
}
