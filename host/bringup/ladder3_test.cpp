/* P2-AE boot ladder rung 3 — resource_decompress_buffer REAL-CALLER path,
 * GSL config-table load off fake_disc (Y's dvd_shim), and the TLogo logo
 * state machine driven to its LAST pure-logic state (7).
 *
 * Every expected value/offset/transition below is hand-derived from the era
 * PPC disassembly @ 6b38b5383 (build/GPOE8P/asm/pso — cited addr+instruction
 * at each CHECK); NONE is produced by the code under test. Era function
 * bodies run VERBATIM through the guarded generated slices in bringup.mk
 * (ladder3_logo_glue / ladder3_quest_glue / ladder3_gsl_glue); what is NOT
 * era code is the small harness in this TU + include/ladder3_harness.h
 * (class shapes .s-shaped, recorders at the two GX walls) — deviations
 * DA-1..DA-5 in docs/p2-bringup-3.md.
 *
 * ORACLE PROVENANCE (all @ 6b38b5383):
 *  [TLogo.s ui] run_task__5TLogoFv 0x8013F294:
 *     :F2A8 lwz r0,0x20(r3) / :F2AC cmplwi r0,0x7 / :F2C0 lwzx (jumptable)
 *     case0 :F2CC bl check_progressive_ready  :F2D8 bl is_logo_clear_flag_set
 *           :F2E4 li r3,0x4c new TAdVIConfigGC :F300 stw 0x24 :F304 li 1
 *           :F330 stw r0,0x20 | else :F310/F33C li r3,0x20 new TAdSEGAlogo
 *           :F32C/:F220C stw 0x2c, :F330 li 0x4 -> stw 0x20
 *     case1 :F368 lwz 0x24 :F374 bl is_vi_config_answered -> 4-construct
 *     case2 :F3AC li r3,1 :F3B0 bl has_network_adapter  no-adapter:
 *           :F3BC li r3,0x30 new TAdTrialModemChkGC :F3D8 stw 0x28 :F3DC li3
 *     case4 :F414 lwz 0x2c :F418 bl is_sega_logo_done :F428 bl
 *           queue_destruction__7TObjectFv :F430 stw 0,0x2c :F438 li r3,0x20
 *           new TAdSoniclogo :F450 stw 0x30 :F454 li 5 :F458 stw 0x20
 *     case5 :F460/:F464 bl is_logo_exit_requested ... :F47C li r3,0x20 new
 *           TAdADXlogo :F49C stw 0x34 :F4A0 li 6
 *     case6 :F4AC/:F4B0 bl is_adx_logo_done :F4C0 bl queue_destruction
 *           :F4C8 li r0,7 -> stw 0x20 (adx slot cleared :F4CC)
 *     case7 :F4D8 li r3,0xa :F4DC bl request_main_phase__Fl   <- NO state
 *           store after it: state 7 is the LAST reachable pure-logic state.
 *  predicates: :F7E8 lwz done__10TAdADXlogo | :F7F0 lwz exit_requested_flag
 *           | :F7F8 lwz sega_logo_done_flag | :F800 lwz r3,0x48(r3)
 *           | :F808 lwz logo_clear_flag      (all .sda21)
 *  __ct__5TLogoFP7TObject 0x8013F54C: :F57C stw name 0x0 / :F580..:F594
 *     stw 0 -> 0x20/0x24/0x28/0x2C/0x30/0x34 (state + 5 object slots).
 *  run_frame__5TLogoFv 0x8013F5E4: PROLOGUE's FIRST bl = 0x8013F5F4
 *     `bl begin_gx_frame` — GX-wall proof (§GX in docs).
 *  [TAdSEGAlogo.s] __ct 0x8001C0B8: :C0EC stw 0,0x1c(m_wait_frames)
 *     :C0F0 stw state :C0F4 stw sega_logo_done_flag :C0F8 bl
 *     init_logo_state(0) :C0FC li r3,0x28 new TScreenFadeIn
 *  [TAdSoniclogo.s] __ct 0x8001C2E4: :C31C stw state :C320 stw
 *     exit_requested_flag :C328 li r3,1 init_logo_state :C32C li r3,0x28 fade
 *  [TAdADXlogo.s] __ct 0x800199B4: :99EC stw state :99F0 stw done :99D8 li
 *     r3,0x28 new TScreenFadeIn(...,fade_in_done — bl :9A0C)
 *  [SystemBootInit.s] request_main_phase__Fl 0x8000F9CC: :F9D8 stw r3,0x4(r4)
 *     (main_phase_state.next) :F9DC li r0,1 / :F9E0 stw r0,0x10(r4)(result).
 *  [QuestOpcodeHandlers.s] qst_op_call_image_data 0x801EB5AC:
 *     :EB5BC lis/addi qst_args :EB5C4 lwz r0,0x4(r3)=args[1].i :EB5C8 lwz
 *     r31,0(args[0].i)=size :EB5CC clrlwi r3,r0,16=(u16) :EB5D0 bl
 *     qst_label_address__Fi :EB5D4 cmplwi r3,0/:EB5D8 beq skip :EB5DC lwz r4,
 *     resource_decompress_buffer@sda21 :EB5E0 bl prs_decompress_buffer__FPCvPv
 *     :EB5E4 lwz r3,res... :EB5E8 mr r4,r31 :EB5EC bl load_sin_board_image
 *  qst_label_address body 0x801F2C1C: lwz r4,qst_function_table / clrlslwi
 *     r0,r3,16,2 ((u16)label<<2 bytes = element index) / lwz r3,qst_code /
 *     lwzx r0,r4,r0 / add r3,r3,r0 / blr. (defs: qst_code :173,
 *     qst_function_table :174, qst_args QuestVm.h:82 — union stride 4.)
 *  [TGStream.s] find_gsl_entry__FPCcl 0x800FDD70: :DD84 lwz handle / :DD88
 *     cmplwi / :DD90 li r3,0 (gate) / :DDA0 addi base 0x11E0 (entries) /
 *     :DDB4 bl strcmp / :DDC8 addi i,1 / :DDCC addi r31,r31,0x30 (STRIDE) /
 *     :DDD0 lwz gsl_entry_count / :DDD4 cmpw / :DDD8 blt / :DDDC li r3,0.
 *  [prs] decode content rules + the G3 vector trace are proven per-address
 *     in prs_golden_test.cpp and ladder_test.cpp (prs.s 0x8036F4DC..; wrapper
 *     0x800F7DAC). This rung reuses the BYTE-IDENTICAL stream:
 *       {AF 'A' 'B' 'C' 'D' E3 FF 00 00} -> "ABCDABCDA" (9 bytes).
 *  [heap] xmalloc/OSAllocFromHeap header facts (HEADERSIZE 32, cell size =
 *     (req+0x20+0x1F)&~0x1F) proven in ladder_test.cpp V1/H2 (OSAlloc.s
 *     0x80370BEC/0x80370CFC).
 */
#include "pso_bringup.h"
#include "ladder3_harness.h"
#include "pso/system/prs.h"                /* era wrapper decls (linked: prs_fade_glue) */
#include "pso/system/memory.h"             /* xmalloc/xfree */
#include "pso/system/AllocatorCallbacks.h"
#include "pso/system/MainPhaseState.h"     /* era struct + extern main_phase_state */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#define AOFF offsetof

static int failures, total;
#define CHECK(cond, ...) do { total++; if (!(cond)) { failures++; \
    printf("FAIL "); printf(__VA_ARGS__); printf("\n"); } \
    else { printf("PASS "); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* G3 vector (hand-trace lives in prs_golden_test.cpp/ladder_test.cpp): */
static const u8 s_g3[] = {0xAF, 'A','B','C','D', 0xE3, 0xFF, 0x00, 0x00};

/* ---- glue-TU symbols (generated slices + preludes) ---- */
extern "C" {
extern const char *logo_class_name; extern int logo_clear_flag; extern int sega_logo_done_flag;
}
int g_check_progressive_ready_calls;      /* harness counters (this TU)      */
int g_has_network_adapter;
int g_check_progressive_ready_answer;
extern "C" int logo_clear_flag = 0;
extern "C" int sega_logo_done_flag = 0;   /* era .sbss words: the predicates
                                             load 0x8013F808 / 0x8013F7F8     */
int g_queue_destruction_calls;
int g_fade_ctors;  FadeCb g_fade_last_cb;  TObject *g_fade_last_parent;
s32 g_init_logo_state_calls[4]; int g_init_logo_state_count;
extern "C" void init_logo_state__23TAdSinglePlyChrSelectGCFl(s32 v) {
    if (g_init_logo_state_count < 4) g_init_logo_state_calls[g_init_logo_state_count] = v;
    g_init_logo_state_count++;
}
void fade_recorder_ctor(TObject *parent, FadeCb cb) {
    g_fade_ctors++; g_fade_last_cb = cb; g_fade_last_parent = parent;
}
void request_destroy(TObject *) {}        /* cb identity markers: era SEGA/  */
void fade_in_done(TObject *) {}           /* Sonic pass request_destroy,     */
                                          /* ADX passes fade_in_done (.s)    */

/* Sliced era functions (defined in generated glue TUs, same headers): */
int  is_logo_clear_flag_set(void);
s32  is_vi_config_answered(void *p);
int  is_sega_logo_done(TAdSEGAlogo *logo);
int  is_logo_exit_requested(TAdSoniclogo *logo);
int  is_adx_logo_done(TAdADXlogo *logo);
void qst_op_call_image_data(void);
void request_main_phase(s32 phase);
void reset_main_phase_state(void);
GslArchiveEntry *find_gsl_entry(const char *name, s32 unused);

/* Quest glue prelude symbols */
extern QuestValue qst_args[8]; extern u8 *qst_code; extern s32 *qst_function_table;
extern u8 *resource_decompress_buffer;
extern void *recorder_image_ptr; extern u32 recorder_image_size; extern int recorder_image_calls;

/* GSL glue prelude symbols (bss-zero at entry — the era gate golden) */
extern AdxfHandle *gsl_adxf_handle; extern GslArchiveEntry gsl_archive_entries[256];
extern s32 gsl_entry_count;

/* ---- harness behaviour the slices call into ---- */
extern "C" s32 has_network_adapter(s32) { return g_has_network_adapter; }
s32 TLogo::check_progressive_ready() {
    g_check_progressive_ready_calls++; return g_check_progressive_ready_answer;
}
/* Era label resolver (QuestOpcodeHandlers.s .fn qst_label_address__Fi
 * 0x801F2C1C..0x801F2C30, def TU-external — reproduced instruction-for-
 * instruction, deviation DA-2; era PPC int==pointer returns as void* here):
 *   lwz r4,qst_function_table | clrlslwi r0,r3,16,2 -> (u16)label (elem idx)
 *   lwz r3,qst_code           | lwzx r0,r4,r0      | add r3,r3,r0 | blr   */
void *qst_label_address(void *label_as_ptr) {
    u32 label = (u32)(uintptr_t)label_as_ptr & 0xFFFFu;   /* clrlwi @:EB5CC  */
    return qst_code + qst_function_table[label];          /* add r3,r3,r0    */
}

/* ---- harness class definitions (era-shaped bodies; the run_task slice is
 * what CONSUMES them). Era .s evidence quoted in the file header. ---- */
TObject::TObject(TObject *parent) { m_name = 0; m_parent = parent; m_next = 0;
    m_prev = 0; m_children = 0; m_flags = 0; era_vptr_slot = 0; m_wait_frames = 0; }
TObject::~TObject() {}
void TObject::run_task() {}
void TObject::render() {}
void TObject::queue_destruction() { g_queue_destruction_calls++; }

void *harness_sega_class_name, *harness_sonic_class_name, *harness_adx_class_name;
void *harness_vi_config_class_name, *harness_modem_class_name;
static char k_sega[]  = "TAdSEGAlogo", k_sonic[] = "TAdSoniclogo",
           k_adx[]    = "TAdADXlogo",  k_vi[]    = "TAdVIConfigGC",
           k_mod[]    = "TAdTrialModemChkGC";

s32 TAdSEGAlogo::state;
s32 TAdSoniclogo::state;
s32 TAdADXlogo::state;   int TAdADXlogo::done;
int TLogo::exit_requested_flag;

TAdSEGAlogo::TAdSEGAlogo(TObject *parent) : TObject(parent) {
    m_name = k_sega;                       /* .s C0E4/C0E8 class_name->+0    */
    harness_sega_class_name = (void *)m_name;
    m_wait_frames = 0;                     /* C0EC stw 0,0x1c                 */
    TAdSEGAlogo::state = 0;                /* C0F0                            */
    sega_logo_done_flag = 0;               /* C0F4                            */
    init_logo_state__23TAdSinglePlyChrSelectGCFl(0);  /* C0F8 (r3=0)          */
    fade_recorder_ctor(0, request_destroy);           /* C0FC li 0x28 + :C100 */
}
TAdSEGAlogo::~TAdSEGAlogo() {} void TAdSEGAlogo::run_task() {} void TAdSEGAlogo::render() {}

TAdSoniclogo::TAdSoniclogo(TObject *parent) : TObject(parent) {
    m_name = k_sonic; m_wait_frames = 0; TAdSoniclogo::state = 0;
    TLogo::exit_requested_flag = 0;        /* .s C320                         */
    init_logo_state__23TAdSinglePlyChrSelectGCFl(1);  /* C308 li 1 / C324     */
    fade_recorder_ctor(0, request_destroy);           /* C32C li 0x28 fade    */
}
TAdSoniclogo::~TAdSoniclogo() {} void TAdSoniclogo::run_task() {} void TAdSoniclogo::render() {}

TAdADXlogo::TAdADXlogo(TObject *parent) : TObject(parent) {
    m_name = k_adx; m_wait_frames = 0; TAdADXlogo::state = 0; TAdADXlogo::done = 0;
    fade_recorder_ctor(0, fade_in_done);   /* ADX-specific cb (TAdADXlogo.cpp:58,
                                              bl __ct__13TScreenFadeIn :9A0C)  */
}
TAdADXlogo::~TAdADXlogo() {} void TAdADXlogo::run_task() {} void TAdADXlogo::render() {}

TAdVIConfigGC::TAdVIConfigGC(TObject *parent) : TObject(parent) {
    m_name = k_vi; answered = 0; harness_vi_config_class_name = (void *)m_name;
}
TAdVIConfigGC::~TAdVIConfigGC() {} void TAdVIConfigGC::run_task() {} void TAdVIConfigGC::render() {}
TAdTrialModemChkGC::TAdTrialModemChkGC(TObject *parent) : TObject(parent) { m_name = k_mod; }
TAdTrialModemChkGC::~TAdTrialModemChkGC() {} void TAdTrialModemChkGC::run_task() {}
void TAdTrialModemChkGC::render() {}

extern "C" const char *logo_class_name = "TLogo";   /* TLogo.cpp:37 */

/* xmalloc cell view (same host mirror as ladder_test.cpp, OSAlloc-proven) */
typedef struct EraCellView { void *prev; void *next; s32 size; u32 hd_idx; s32 requested; s32 _pad; } EraCellView;
#define CELL(p) ((EraCellView *)((u8 *)(p) - 32))

#define SYSHEAP_START_OFF 0x1000000u
#define SYSHEAP_SIZE      0x00600000

/* disc helper: read a fake_disc file fully into arena via Y's shim
 * (same usage shape as ladder_test W4 and the era DVDOpen/DVDReadPrio
 * pattern text_803DD5E0.cpp:93-97) */
static s32 disc_read(const char *name, u8 *dst, u32 cap) {
    DVDFileInfo fi; memset(&fi, 0, sizeof fi);
    if (!DVDOpen((char *)name, &fi)) return -1;
    s32 got = fi.length > cap ? -2 : DVDReadPrio(&fi, dst, (s32)fi.length, 0, 2);
    DVDClose(&fi);
    return got;
}

int main() {
    setvbuf(stdout, 0, _IOLBF, 0);   /* goldens survive a crash */
    if (pso_bringup_install() != 0) { fprintf(stderr, "arena install failed\n"); return 2; }
    OSInit();
    init_system_heap_and_global_allocator(pso_arena_base + SYSHEAP_START_OFF, (s32)SYSHEAP_SIZE);
    DVDInit();

    /* ============ L0: era layout mirror proofs (compile-time facts,
     * read back at runtime for the golden lines) ============ */
    CHECK(sizeof(GslArchiveEntry) == 0x30 && AOFF(GslArchiveEntry, archive_offset) == 0x20
          && AOFF(GslArchiveEntry, file_size) == 0x24
          && AOFF(GslArchiveEntry, decoded_size) == 0x28,
          "L0 GslArchiveEntry host layout 0x30/0x20/0x24/0x28 == find_gsl_entry .s stride 0x800FDDCC `addi r31,r31,0x30` + strcmp-at-name@0");
    CHECK(AOFF(TAdVIConfigGC, answered) == 0x48,
          "L0 TAdVIConfigGC.answered at host offset 0x48 == is_vi_config_answered deref 0x8013F800 `lwz r3,0x48(r3)` (era PPC offset kept through padded base)");
    CHECK(sizeof(QuestValue) == 4, "L0 QuestValue union stride 4 == qst_args loads 0x801EB5C4 `lwz r0,0x4(r3)`/0x801EB5C8");

    /* ============ A: resource_decompress_buffer REAL-CALLER path ============ */
    /* A0: the actual global, allocated per SystemBootInit.cpp:283 pattern
     * `resource_decompress_buffer = (u8 *)xmalloc(0x80000);` — trimmed to
     * 0x2000 on host (DA-5); header rules per ladder_test H2. */
    resource_decompress_buffer = (u8 *)xmalloc(0x2000);
    memset(resource_decompress_buffer, 0xCC, 0x2000);
    CHECK(resource_decompress_buffer && (uintptr_t)resource_decompress_buffer % 32 == 0
          && CELL(resource_decompress_buffer)->requested == 0x2000,
          "A0 resource_decompress_buffer via xmalloc (SystemBootInit.cpp:283 pattern), cell requested=0x%x (OSAlloc.s 0x80370BEC)",
          (unsigned)CELL(resource_decompress_buffer)->requested);

    /* A1: era label resolver body reproduced @ qst_label_address —
     * qst_code + qst_function_table[(u16)label], .s 0x801F2C1C..0x801F2C2C */
    u8 *code = pso_arena_base + 0x80000;
    static s32 ftable[16];
    ftable[8] = 0x200;
    qst_code = code; qst_function_table = ftable;
    memcpy(code + 0x200, s_g3, sizeof s_g3);
    void *sd = qst_label_address((void *)(uintptr_t)8);
    CHECK(sd == code + 0x200,
          "A1 qst_label_address((u16)8): code+table[8]=arena+0x80200 (lwz r4,qst_function_table :C1C / clrlslwi<<2 :C20 / add r3,r3,r0 :C2C)");

    /* A2: era caller qst_op_call_image_data drives the REAL global */
    qst_args[0].i = 9;      /* size          (lwz r31,0 @:EB5C8)             */
    qst_args[1].i = 8;      /* label (u16)   (lwz r0,4 / clrlwi :EB5C4/CC)   */
    qst_op_call_image_data();
    CHECK(recorder_image_calls == 1 && recorder_image_ptr == (void *)resource_decompress_buffer
          && recorder_image_size == 9,
          "A2 era qst_op_call_image_data: dst==resource_decompress_buffer global (:EB5DC/:EB5E4 lwz @sda21), load_sin_board_image size==args[0].i");
    CHECK(memcmp(resource_decompress_buffer, "ABCDABCDA", 9) == 0 && resource_decompress_buffer[9] == 0xCC,
          "A2 decoded bytes at caller dst: 9 bytes 'ABCDABCDA' read back AT the real buffer, byte[9] untouched 0xCC (prs_G3 trace, wrapper 0x800F7DAC)");

    /* A3: skip branch: NULL source -> beq :EB5D8 -> recorder NOT called */
    qst_code = 0; ftable[8] = 0;
    qst_args[1].i = 8;
    qst_op_call_image_data();
    CHECK(recorder_image_calls == 1,
          "A3 NULL label target: cmplwi r3,0 / beq .L_801EB5F0 (0x801EB5D4-5D8) — decompress+sin-board skipped");

    /* ============ B: GSL config table + PRS off fake_disc ============ */
    CHECK(find_gsl_entry((const char *)"title_prs.dat", 0) == 0,
          "B0 find_gsl_entry handle gate: gsl_adxf_handle==0 -> li r3,0 (0x800FDD84-:DD94, .bss-zero)");

    u8 *raw = pso_arena_base + 0x30000;
    s32 got = disc_read("gsl_tab.dat", raw, 4096);
    CHECK(got == 3 * (s32)sizeof(GslArchiveEntry),
          "B1 DVDOpen/DVDReadPrio(gsl_tab.dat) via era-shape shim == 3 x 0x30 entries (got=%d)", got);
    memcpy(gsl_archive_entries, raw, (size_t)got);
    gsl_entry_count = 3; gsl_adxf_handle = (AdxfHandle *)0x1;   /* host fill stands for era open_gsl_archive (DA-4) */
    GslArchiveEntry *e = find_gsl_entry((const char *)"menu_font.prs", 0);
    CHECK(e == &gsl_archive_entries[1] && (u8 *)e - (u8 *)gsl_archive_entries == 0x30
          && e->archive_offset == 0x800 && e->file_size == 0x200 && e->decoded_size == 0x1000,
          "B2 find_gsl_entry hit @idx1: stride-0x30 walk (:DDCC), fields name@0 offset@0x20=0x800 size@0x24=0x200 decoded@0x28=0x1000");
    CHECK(find_gsl_entry((const char *)"ghost.gsl", 0) == 0 && gsl_entry_count == 3,
          "B3 miss walks full count then li r3,0 (0x800FDDDC-:DDE0)");

    e = find_gsl_entry((const char *)"title_prs.dat", 0);
    got = e ? disc_read("title_prs.dat", raw, 4096) : -1;
    u32 dn = prs_decompress_buffer(raw, resource_decompress_buffer);
    CHECK(e && e->file_size == 9 && dn == 9 && e->decoded_size == (s32)dn
          && memcmp(resource_decompress_buffer, "ABCDABCDA", 9) == 0,
          "B4 disc PRS -> era wrapper -> REAL resource_decompress_buffer: ret=%u == entry.decoded_size 9, bytes 'ABCDABCDA' (:EB5DC-style dst, wrapper 0x800F7DAC r4=dst r5=src r6=0)",
          (unsigned)dn);

    /* ============ C: TLogo state machine — full pure-logic chain ============ */
    g_check_progressive_ready_answer = 0; g_has_network_adapter = 0;
    TLogo *logo = new TLogo(0);
    CHECK(logo->m_name == logo_class_name && logo->state == 0 && logo->vi_config == 0
          && logo->modem_check == 0 && logo->sega_logo == 0 && logo->sonic_logo == 0 && logo->adx_logo == 0,
          "C0 TLogo ctor slice (:F57C stw name; :F580-:F594 six stw 0 -> 0x20..0x34): all slots zero");

    logo->run_task();   /* case 0, prog not ready -> SEGA leg :F33C..:F360 */
    CHECK(logo->state == 4 && logo->sega_logo != 0 && g_check_progressive_ready_calls == 1
          && g_fade_ctors == 1 && g_fade_last_cb == request_destroy
          && g_init_logo_state_count == 1 && g_init_logo_state_calls[0] == 0,
          "C1 state 0->4: check_progressive_ready()==0 route (:F2D4 beq :F33C), TAdSEGAlogo new + stw 0x2c/0x20 (:F32C/:F360), fade cb request_destroy");

    logo->run_task();   /* case 4, not done: no-op */
    CHECK(logo->state == 4 && g_queue_destruction_calls == 0,
          "C2 case4 hold: is_sega_logo_done==0 -> beq :F4E0 return, no destruction queued");

    sega_logo_done_flag = 1; logo->run_task();
    CHECK(logo->state == 5 && logo->sega_logo == 0 && logo->sonic_logo != 0
          && g_queue_destruction_calls == 1 && TLogo::exit_requested_flag == 0
          && g_init_logo_state_calls[1] == 1,
          "C3 4->5: flag(:F7F8)->queue_destruction(:F428)->slot0(:F430)->new TAdSoniclogo(:F438 li 0x20)->stw 0x30 li5(:F450-58); sonic ctor clears exit flag (:C320), init_logo_state(1) (:C308)");

    TLogo::exit_requested_flag = 1; logo->run_task();
    CHECK(logo->state == 6 && logo->sonic_logo == 0 && logo->adx_logo != 0
          && g_queue_destruction_calls == 2 && g_fade_last_cb == fade_in_done
          && TAdADXlogo::done == 0,
          "C4 5->6: :F7F0 flag->queue(:F474)->new TAdADXlogo(:F47C li 0x20)->stw 0x34 li6(:F49C-4A4); ADX ctor fade cb fade_in_done, done cleared :99F0");

    TAdADXlogo::done = 1; logo->run_task();
    CHECK(logo->state == 7 && logo->adx_logo == 0 && g_queue_destruction_calls == 3,
          "C5 6->7: :F7E8 done->queue(:F4C0)->li r3,0 stw 0x34 (:F4C4/:F4CC) li r0,7 stw 0x20 (:F4C8/:F4D0)");

    reset_main_phase_state();
    CHECK(main_phase_state.cur == 1 && main_phase_state.queued_main_phase == 1
          && main_phase_state.fallback == 1 && main_phase_state.prev == 1,
          "C6 reset_main_phase_state (SystemBootInit.cpp): cur/queued/fallback/prev=1, result/next untouched");

    int q_before = g_queue_destruction_calls;
    logo->run_task();   /* case 7: the terminal pure transition */
    CHECK(main_phase_state.next == 0xA && main_phase_state.result == 1
          && main_phase_state.cur == 1 && logo->state == 7
          && g_queue_destruction_calls == q_before,
          "C7 STATE 7 = LAST pure-logic state: li r3,0xa/bl request_main_phase (:F4D8/:F4DC), request writes next@+4/result=1@+0x10 (0x8000F9D8/:F9E0); no state store, no object churn after");

    /* VI dialog route (case 0 answered-branch + case 1) */
    g_check_progressive_ready_answer = 1; logo_clear_flag = 1;
    TLogo *logo2 = new TLogo(0);
    logo2->run_task();
    CHECK(logo2->state == 1 && logo2->vi_config != 0 && logo2->sega_logo == 0,
          "C8 0->1: prog==1 & logo_clear_flag set -> li r3,0x4c new TAdVIConfigGC (:F2E4-:F2C8), stw 0x24 li 1 (:F300-:F308)");
    logo2->vi_config->answered = 1;
    logo2->run_task();
    CHECK(logo2->state == 4 && logo2->sega_logo != 0 && is_vi_config_answered(logo2->vi_config) == 1,
          "C9 1->4: case1 lwz 0x24 (:F368) + is_vi_config_answered (:F374, deref +0x48) -> SEGA new (:F380 li 0x20) stw 0x2c li 4 (:F39C-:F3A4)");

    /* modem legs (state 2 entry is DEAD in this unit: run_task stores only
     * 1/3/4/5/6/7 — grep of every stw-to-0x20 in the slice) */
    TLogo *logo3 = new TLogo(0); logo3->state = 2;
    logo3->run_task();
    CHECK(logo3->state == 3 && logo3->modem_check != 0,
          "C10 case2 no-adapter: has_network_adapter(1)==0 (:F3AC/:F3B0/:F3B4 bne) -> li 0x30 new TAdTrialModemChkGC (:F3BC) stw 0x28 li 3 (:F3D8-:F3E0)");
    TLogo *logo4 = new TLogo(0); logo4->state = 2; g_has_network_adapter = 1;
    logo4->run_task();
    CHECK(logo4->state == 4 && logo4->sega_logo != 0 && logo4->modem_check == 0,
          "C11 case2 adapter-present: bne :F3E8 -> SEGA leg (:F3E8 li 0x20), stw 0x2c li 4");
    CHECK(logo->state == 7 && logo2->state == 4,
          "C12 state-2 DEAD as an EXIT: no run_task branch stores 2 (only 1/3/4/5/6/7 at :F304/:F3DC/:F330/:F454/:F4A0/:F4C8) — nothing routes INTO 2 from 0/1/4/5/6/7");

    xfree(resource_decompress_buffer);
    printf("== P2-AE ladder3: %d/%d PASS ==\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
