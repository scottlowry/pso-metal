/* pso-metal P2-AG boot slice — scripted boot executed by the first link.
 * Chain (task): arena -> fake_disc -> PRS era chain -> heap alloc/free,
 * extended with the three era TUs that close inside the first-link set
 * (MiddlewareInterruptLock, text_803DEE48, text_803D9FF0) and memory.cpp's
 * system-heap machinery. Every stage prints exactly one RECEIPT line;
 * any CHECK failure flips the exit code. Era functions are called through
 * their REAL C++ mangled names (declarations below reproduce the mangled
 * spellings measured with `nm` on the era objects).
 */
#include "pso_bringup.h"            /* arena/OSInit/DVD/OSAlloc (extern "C") */
#include <stdio.h>
#include <string.h>

/* ---- era C++ entries (mangling-verified against host/link/obj/*.o) ---- */
void  middleware_interrupt_lock_enter(void);           /* __Z31...enter v */
void  middleware_interrupt_lock_leave(void);           /* __Z31...leave v */
extern volatile s32 middleware_interrupt_lock_depth;   /* data: unmangled */
extern s32          middleware_saved_interrupt_state;
int   change_disc_directory(const char *dir);          /* __Z21...PKc     */
void  convert_string_chars_via_table(u8 *buf);         /* __Z30...Ph      */
int   prs_decompress_alloc(const void *src, void *dst, int size_limit);
void  init_system_heap_and_global_allocator(u8 *start, s32 size);
void *sys_alloc(u32 size);
void  free_to_sys_heap(void *ptr);
void *xmalloc(u32 bytes);                      /* __Z7xmallocj   */
void  xfree(void *p);                          /* __Z5xfreePv    */
extern OSHeapHandle sys_heap_handle;           /* data: unmangled */

/* ---- host_backings.c probes ---- */
extern "C" {
extern char pso_dvd_cwd[256];
int pso_host_irq_enabled(void);
int DVDChangeDir(char *dir);   /* host_backings.c; era dvd.h:121 BOOL=int */
}

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL "); printf(__VA_ARGS__); printf("\n"); } } while (0)
#define RECEIPT(...)     do { printf("RECEIPT "); printf(__VA_ARGS__); printf("\n"); } while (0)

int main(void) {
    /* 1 — arena (crt0 install: mmap 26 MB, SysRet, PRS window) */
    CHECK(pso_bringup_install() == 0, "arena install");
    u32 bus = ((u32 *)pso_g2h(0x80000000u + GUEST_SYSRET_OFF))[0];
    CHECK(bus == 32419584u, "sysret busClock %u", bus);
    RECEIPT("1 arena: base=%p 24MB+2MB, sysret.busFreq=32419584, prs_window=%p",
            (void *)pso_arena_base, (void *)pso_prs_window_base);

    /* 2 — OSInit + era OSAlloc heap spine (Y's osalloc_era.c port) */
    OSInit();
    long live0 = OSCheckHeap(__OSCurrHeap);
    void *hp = OSAllocFromHeap(__OSCurrHeap, 0x4560);
    CHECK(hp != NULL, "OSAllocFromHeap NULL");
    CHECK(OSCheckHeap(__OSCurrHeap) == live0 + 1, "live count after alloc");
    OSFreeToHeap(__OSCurrHeap, hp);
    CHECK(OSCheckHeap(__OSCurrHeap) == live0, "live count after free");
    RECEIPT("2 heap-spine: OSAllocFromHeap(0x4560)->%p, live %ld->%ld->%ld (era OSAlloc port)",
            hp, live0, live0 + 1, OSCheckHeap(__OSCurrHeap));

    /* 3 — fake_disc mount + logo stream read (Y's dvd_shim) */
    DVDInit();
    CHECK(DVDGetDriveStatus() == 0, "drive not READY");
    CHECK(DVDChangeDir((char *)"/files/") == 1, "DVDChangeDir backing");
    DVDFileInfo fi; memset(&fi, 0, sizeof fi);
    CHECK(DVDOpen((char *)"logo_e.prs", &fi) != 0, "DVDOpen");
    u8 *src = pso_arena_base + 0x20000;
    u8 *dst = pso_arena_base + 0x10000;
    s32 got = DVDReadPrio(&fi, src, (s32)fi.length, 0, 2);
    CHECK(got == (s32)fi.length && got > 0, "DVDReadPrio %d/%u", got, fi.length);
    DVDClose(&fi);
    RECEIPT("3 fake_disc: logo_e.prs read %d/%u bytes via pread bundle", got, fi.length);

    /* 4 — PRS era chain: TScreenFadeController wrapper -> PrsDecoder::
     *     prs_decompress (era prs.cpp, 1-literal rebase D3) */
    s32 n = prs_decompress_alloc(src, dst, 0x800000);
    CHECK(n == 9, "prs_decompress_alloc n=%d", (int)n);
    dst[n < 0 ? 0 : n] = 0;
    CHECK(n == 9 && memcmp(dst, "ABCDABCDA", 9) == 0, "prs decoded \"%s\"", (char *)dst);
    RECEIPT("4 prs-chain: prs_decompress_alloc -> 9 bytes \"%s\" (era wrapper+era decoder)",
            (char *)dst);

    /* 5 — era TU MiddlewareInterruptLock.cpp executing over the mask backing */
    middleware_interrupt_lock_enter();
    CHECK(middleware_interrupt_lock_depth == 1, "depth after enter1");
    CHECK(middleware_saved_interrupt_state == 1, "saved mask after enter1");
    CHECK(pso_host_irq_enabled() == 0, "irq still enabled under lock");
    middleware_interrupt_lock_enter();
    CHECK(middleware_interrupt_lock_depth == 2, "depth after enter2");
    middleware_interrupt_lock_leave();
    CHECK(pso_host_irq_enabled() == 0, "irq re-enabled too early (nested)");
    middleware_interrupt_lock_leave();
    CHECK(middleware_interrupt_lock_depth == 0, "depth after leaves");
    CHECK(pso_host_irq_enabled() == 1, "irq not restored at depth 0");
    RECEIPT("5 irq-gate: middleware lock enter x2/leave x2, nested mask honored");

    /* 6 — era TU memory.cpp: system heap + allocator-callback spine */
    init_system_heap_and_global_allocator(pso_arena_base + 0x1000000, 0x200000);
    void *p = sys_alloc(0x1234);
    CHECK(p, "sys_alloc NULL");
    CHECK((u8 *)p >= pso_arena_base + 0x1000000 && (u8 *)p < pso_arena_base + 0x1200000,
          "sys_alloc out of carve");
    memset(p, 0x5A, 0x1234);
    void *q = xmalloc(0x100);   /* via global_allocator callback table */
    CHECK(q, "xmalloc NULL");
    CHECK(OSCheckHeap(sys_heap_handle) == 2, "sys-heap live=%ld want 2",
          OSCheckHeap(sys_heap_handle));
    free_to_sys_heap(p);
    xfree(q);
    CHECK(OSCheckHeap(sys_heap_handle) == 0, "sys-heap not drained");
    RECEIPT("6 sys-heap: init_system_heap_and_global_allocator carve + sys_alloc/xmalloc pair drained via era callback table (live 2->0)", p);

    /* 7 — era TU text_803DEE48.cpp over the DVDChangeDir extension */
    CHECK(change_disc_directory("/files/item/") == 0, "change_disc_directory ret");
    CHECK(strcmp(pso_dvd_cwd, "/files/item/") == 0, "cwd=\"%s\"", pso_dvd_cwd);
    RECEIPT("7 dvd-chdir: change_disc_directory(\"/files/item/\") -> cwd recorded");

    /* 8 — era TU text_803D9FF0.cpp over the MSL __upper_map shim */
    char buf[] = "pso-logo test!";
    convert_string_chars_via_table((u8 *)buf);
    CHECK(strcmp(buf, "PSO-LOGO TEST!") == 0, "table upper got \"%s\"", buf);
    RECEIPT("8 ctype-table: convert_string_chars_via_table(\"pso-logo test!\") -> \"%s\"", buf);

    printf("== P2-AG first-link boot slice: %s (%d failures) ==\n",
           failures ? "FAILED" : "ALL RECEIPTS OK", failures);
    return failures ? 1 : 0;
}
