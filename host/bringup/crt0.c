/* pso-metal P2-Q bringup crt0 (p1-init-ledger §4 step 2).
 * One ~26 MB zero-filled mmap arena: [0,24MB) main RAM (guest 0x80000000…
 * rebased — fixed low mappings are impossible on this host, see
 * docs/p2-bringup-1.md "deviation D1"), [24MB,26MB) ARAM. Hosts the
 * BootInfo-replacement SysRet anchor at arena+0xF0 and the PRS decoder
 * window pointer (arena+0x1800) the decomp's prs.cpp window literal maps to.
 * "Zero bss" = explicit zero-fill pass (MAP_ANON already zeroes; we assert it
 * rather than trust it). Calls the main-equivalent spine below.
 */
#include "pso_bringup.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

u8 *pso_arena_base;
u8 *pso_aram_base;
u8 *pso_prs_window_base;
PsoSysRet *pso_sysret;

/* Mini OSInit globals live in osinit.c but their anchor values come from here. */
extern void OSInit(void);

int pso_bringup_install(void) {
    void *p = mmap(NULL, PSO_ARENA_SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANON, -1, 0);
    if (p == MAP_FAILED) {
        fprintf(stderr, "crt0: arena mmap(%u MB) failed\n",
                (unsigned)(PSO_ARENA_SIZE >> 20));
        return -1;
    }
    pso_arena_base = (u8 *)p;
    /* "zero bss": explicit pass + verification probe (MAP_ANON guarantees it;
     * the check documents the invariant for later steps that rely on it). */
    memset(pso_arena_base, 0, PSO_ARENA_SIZE);
    if (pso_arena_base[0] || pso_arena_base[PSO_MAIN_RAM_SIZE - 1] ||
        pso_arena_base[PSO_ARENA_SIZE - 1]) {
        fprintf(stderr, "crt0: arena not zero-filled\n");
        return -1;
    }
    pso_aram_base = pso_arena_base + PSO_MAIN_RAM_SIZE;

    /* BootInfo replacement anchor (deviation D2): the 9 era readers deref
     * absolute 0x800000F8 for the bus clock. Host readers will go through
     * pso_sysret / pso_g2h(0x800000F8); the WORD LAYOUT is the observed era
     * one (SysRet @+0xF0: memSize, exiFreq, busFreq @+0xF8, cpuFreq).
     * Values stored HOST-NATIVE (LE): the port executes as arm64, byte order
     * only matters on the disc/wire path (compat/pso_byteswap.h domain). */
    pso_sysret = (PsoSysRet *)(pso_arena_base + (GUEST_SYSRET_OFF - 8));
    pso_sysret->memSize = PSO_MAIN_RAM_SIZE;      /* guest reads memSize @F0 */
    pso_sysret->exiFreq = 16209792u;              /* 16.209792 MHz           */
    pso_sysret->busFreq = 32419584u;              /* 32.419584 MHz @F8       */
    pso_sysret->cpuFreq = 486293760u;             /* 15x bus                 */

    /* Era prs.cpp pins the decoder window to literal 0x80001800; the port
     * build rebinds that single literal to this base (deviation D3). The
     * window itself lives at the SAME arena offset the era used. */
    pso_prs_window_base = pso_arena_base + GUEST_PRS_WINDOW_OFF;
    return 0;
}

#ifndef PSOB_TEST_BUILD
/* main-equivalent boot spine (ledger steps 1-4 smoke, step-19 decode):
 * install → OSInit → DVDInit → open/read fake-disc logo stream → PRS decode
 * into the arena → heap + clock smoke. The PRS unit linked in here is the
 * decomp's own src/pso/system/prs.cpp (one-literal glue copy — see
 * bringup.mk), so THIS FUNCTION IS RUNNING ERA GAME LOGIC. */
int main(void) {
    if (pso_bringup_install() != 0) return 2;
    OSInit();
    OSReport("crt0: arena %p (main 24MB + ARAM 2MB @ %p), busClock @arena+0xF8 = %u\n",
             (void *)pso_arena_base, (void *)pso_aram_base,
             ((u32 *)pso_g2h(0x80000000u + GUEST_SYSRET_OFF))[0]);

    DVDInit();
    DVDFileInfo fi;
    u8 *src = pso_arena_base + 0x20000;   /* era-ish scratch: PRS input      */
    u8 *dst = pso_arena_base + 0x10000;   /* decoded output                  */
    memset(&fi, 0, sizeof fi);
    if (!DVDOpen((char *)"logo_e.prs", &fi)) {
        OSReport("crt0: DVDOpen(logo_e.prs) FAILED\n");
        return 3;
    }
    s32 got = DVDReadPrio(&fi, src, (s32)fi.length, 0, 2);
    OSReport("crt0: DVD read logo_e.prs: %d of %u bytes\n", got, fi.length);
    DVDClose(&fi);

    /* PrsDecoder::prs_decompress — era game function, executing. */
    extern s32 pso_bringup_run_prs(u8 *out, u8 *in, s32 cap);
    s32 n = pso_bringup_run_prs(dst, src, 0x800000);
    if (n < 0) { OSReport("crt0: PRS decode error\n"); return 4; }
    dst[n] = 0;
    OSReport("crt0: PRS-decoded %d bytes -> \"%s\"  <== FIRST ERA GAME LOGIC ON ARM64\n",
             (int)n, (char *)dst);

    /* clock + heap shims */
    u32 t0 = OSGetTick();
    volatile long k = 0; for (; k < 300000; k++) {}
    u32 t1 = OSGetTick();
    OSReport("crt0: OSGetTick %u -> %u (delta %d @8104896Hz)\n",
             t0, t1, (s32)(t1 - t0));
    void *hp = OSAllocFromHeap(__OSCurrHeap, 0x4560); /* texture_state_buffer sized */
    OSReport("crt0: OSAllocFromHeap(0x4560) -> %p\n", hp);
    OSFreeToHeap(__OSCurrHeap, hp);
    return (n == 9) ? 0 : 5;
}
#endif
