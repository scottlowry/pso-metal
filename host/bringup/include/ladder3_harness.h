/* P2-AE boot ladder rung 3 — host harness types.
 *
 * These are HOST-side era-SHAPE shadows. Each one exists so the generated
 * VERBATIM slices (ladder3_logo_glue.cpp / ladder3_quest_glue.cpp /
 * ladder3_gsl_glue.cpp) can compile against the same field layout the era
 * PPC .s encodes, without dragging the full era class graphs (which reach
 * GX). Every offset below is provenanced from the era disassembly @ 6b38b5383
 * in the comment; the generated slices are diff-guarded against the read-only
 * decomp source in bringup.mk.
 */
#ifndef LADDER3_HARNESS_H
#define LADDER3_HARNESS_H

#include "types.h"

/* ---- pso/system/task/TObject.h shadow -------------------------------------
 * Era base-object layout provenance (TAdSEGAlogo.s __ct__11TAdSEGAlogoFP7TObject):
 *   0x8001C0E4  lwz r4, class_name__11TAdSEGAlogo ; 0x8001C0E8 stw r4,0x0(r31)
 *               -> m_name @ 0x0
 *   0x8001C0EC  stw r0,0x1c(r31)   -> m_wait_frames @ 0x1C  (li r0,0 @ :C0DC)
 *   0x8001C0E0  stw r4,0x18(r31)   -> vtable pointer @ 0x18 (MWCC: vptr after
 *               non-virtual fields; era object size 0x20 -> 6 field words +
 *               vptr + 4 bytes of subclass-private storage below 0x20 total).
 * Shadow ctor takes the era parent arg (bl __ct__7TObjectFP7TObject, e.g.
 * TLogo.s __ct 0x8013F560) and stores vptr+name exactly like the era units.
 */
class TObject {
public:
    const char *m_name;      /* +0x00 [stw r4,0x0(r31)]                       */
    TObject *m_parent;       /* +0x04  (never touched by any slice here)      */
    TObject *m_next;         /* +0x08                                         */
    TObject *m_prev;         /* +0x0C                                         */
    TObject *m_children;     /* +0x10                                         */
    s32 m_flags;             /* +0x14                                         */
    void *era_vptr_slot;     /* +0x18 era vptr slot [stw r4,0x18(r31)] — a
                                host-side placeholder word; its LP64 width is
                                chosen so the FIRST derived member lands at
                                byte 0x48, the exact era PPC offset that
                                is_vi_config_answered dereferences
                                (0x8013F800 `lwz r3,0x48(r3)`) — asserted with
                                offsetof() in ladder3_test.cpp.             */
    s32 m_wait_frames;       /* +0x1C  [stw r0,0x1c]                          */
    TObject(TObject *parent);
    virtual ~TObject();
    virtual void run_task();
    virtual void render();
    /* Era call sites queue_destruction__7TObjectFv inside the TLogo.s
     * run_task slice: 0x8013F428 / 0x8013F474 / 0x8013F4C0. */
    virtual void queue_destruction();
};

/* ---- pso/ui/TLogo.h shadow --------------------------------------------------
 * Field offsets verbatim from __ct__5TLogoFP7TObject (TLogo.s):
 *   0x8013F578 lwz r4, logo_class_name@sda21 ; 0x8013F57C stw r4,0x0(r31)
 *   0x8013F580 stw r0,0x20(r31)  state
 *   0x8013F584 stw r0,0x24       vi_config     (run_task stores new ptr
 *                                0x8013F300 stw r0,0x24(r31))
 *   0x8013F588 stw r0,0x28       modem_check   (0x8013F3D8 stw r0,0x28)
 *   0x8013F58C stw r0,0x2C       sega_logo     (0x8013F32C stw r0,0x2c)
 *   0x8013F590 stw r0,0x30       sonic_logo    (0x8013F450 stw r0,0x30)
 *   0x8013F594 stw r0,0x34       adx_logo      (0x8013F49C stw r0,0x34)
 * era sizeof == 0x38 (TLogo.h LAYOUT_ASSERT(TLogoExtent) at decomp HEAD).
 */
class TAdVIConfigGC;
class TAdTrialModemChkGC;
class TAdSEGAlogo;
class TAdSoniclogo;
class TAdADXlogo;

class TLogo : public TObject {
public:
    static int exit_requested_flag;   /* .s 0x8013F7F0 lwz @sda21 (sbss)      */
    TLogo(TObject *parent);
    virtual void render();
    virtual ~TLogo();
    virtual void run_task();
    static void request_exit(TObject *fade);
    static s32 check_progressive_ready();
    u8 pad_1C[4];
    u32 state;                /* +0x20 */
    TAdVIConfigGC *vi_config; /* +0x24 */
    TAdTrialModemChkGC *modem_check; /* +0x28 */
    TAdSEGAlogo *sega_logo;   /* +0x2C */
    TAdSoniclogo *sonic_logo; /* +0x30 */
    TAdADXlogo *adx_logo;     /* +0x34 */
};

/* ---- pso/ui/frontend/TAdSEGAlogo.h shadow -----------------------------------
 * __ct__11TAdSEGAlogoFP7TObject (TAdSEGAlogo.s 0x8001C0B8..): state (static)
 * + sega_logo_done_flag stores 0x8001C0F0/0x8001C0F4, fade new 0x8001C0FC
 * `li r3,0x28` + 0x8001C100 bl __nw__7TObjectFUl. Object size 0x20 proven by
 * BOTH TLogo.s run_task allocs: 0x8013F310/0x8013F33C/0x8013F380/0x8013F3E8
 * `li r3,0x20`.
 */
class TAdSEGAlogo : public TObject {
public:
    static s32 state;                /* static, not a member: era object is
                                        exactly 0x20 (TLogo.s li r3,0x20)     */
    TAdSEGAlogo(TObject *parent);
    virtual void run_task();
    virtual void render();
    virtual ~TAdSEGAlogo();
};
/* The era ctor body's four stores (m_wait_frames/state/sega_logo_done_flag/
 * fade) + init_logo_state(0) are replaced by the harness ctor; the recorder
 * globals below observe them instead of living in the object. */

/* ---- TAdSoniclogo shadow: size 0x20 (TLogo.s 0x8013F438 `li r3,0x20`),
 * ctor .s 0x8001C2E4..: stw r0,state__12TAdSoniclogo(0x8001C31C),
 * stw r0,exit_requested_flag__5TLogo(0x8001C320), li r3,1 ->
 * init_logo_state(1)(0x8001C328), li r3,0x28 fade new(0x8001C32C+). */
class TAdSoniclogo : public TObject {
public:
    static s32 state;
    TAdSoniclogo(TObject *parent);
    virtual void run_task();
    virtual void render();
    virtual ~TAdSoniclogo();
};

/* ---- TAdADXlogo shadow: size 0x20 (TLogo.s 0x8013F47C `li r3,0x20`),
 * ctor .s 0x800199B4..: stw r0,state__10TAdADXlogo(0x800199EC),
 * stw r0,done__10TAdADXlogo(0x800199F0), li r3,0x28 new TScreenFadeIn
 * (0x800199F4/0x80019A0C bl __ct__13TScreenFadeInFP7TObjectPFP7TObject_v). */
class TAdADXlogo : public TObject {
public:
    static s32 state;
    static int done;          /* is_adx_logo_done reads it @ 0x8013F7E8       */
    TAdADXlogo(TObject *parent);
    virtual void run_task();
    virtual void render();
    virtual ~TAdADXlogo();
};

class TAdVIConfigGC : public TObject {
public:
    /* era object size 0x4C (TLogo.s 0x8013F2E4 `li r3,0x4c`);
     * is_vi_config_answered reads s32 @ +0x48 (0x8013F800 `lwz r3,0x48(r3)`). */
    u8 pad_to_era48[4];       /* engineered: TObject's last field ends at
                                 0x44 (measured), this pad puts the FIRST live
                                 byte exactly on the era deref 0x48          */
    s32 answered;             /* == 0x48, is_vi_config_answered deref
                                 (0x8013F800 `lwz r3,0x48(r3)`)               */
    TAdVIConfigGC(TObject *parent);
    virtual void run_task();
    virtual void render();
    virtual ~TAdVIConfigGC();
};

class TAdTrialModemChkGC : public TObject {
public:
    /* era object size 0x30 (TLogo.s 0x8013F3BC `li r3,0x30`) */
    TAdTrialModemChkGC(TObject *parent);
    virtual void run_task();
    virtual void render();
    virtual ~TAdTrialModemChkGC();
};

/* ---- TScreenFadeIn / TScreenFade recorder shadow ----------------------------
 * TAdSEGAlogo.cpp:92 / TAdSoniclogo.cpp:54 / TAdADXlogo.cpp:58 all end with
 *   new TScreenFadeIn(parent, cb);   era alloc size 0x28 (SEGA .s 0x8001C0FC
 * `li r3,0x28`, ADX .s 0x800199D8 `li r3,0x28`; TScreenFadeIn ctor .s ends
 * bl __ct__13TScreenFadeInFP7TObjectPFP7TObject_v). TScreenFadeIn's own era
 * body (TScreenFadeController.cpp:85-91) is PURE: callback=cb, alpha=255.0f,
 * color[0..2]=0 — recorded, not modeled, to keep this rung free of the fade
 * controller's task-list plumbing. */
typedef void (*FadeCb)(TObject *);
extern int   g_fade_ctors;         /* count of new TScreenFadeIn calls        */
extern FadeCb g_fade_last_cb;
extern TObject *g_fade_last_parent;
void fade_recorder_ctor(TObject *parent, FadeCb cb);
void request_destroy(TObject *fade);       /* era: TLogo::request_exit shape  */
void fade_in_done(TObject *fade);

/* ---- harness recorder externs referenced by slices/tests ------------------- */
extern int g_queue_destruction_calls;
extern void *harness_logo_class_name;
extern void *harness_sega_class_name;
extern void *harness_sonic_class_name;
extern void *harness_adx_class_name;
extern void *harness_vi_config_class_name;
extern void *harness_modem_class_name;
extern int g_check_progressive_ready_calls;

/* ---- pso/system/task (TAdSinglePlyChrSelectGC::init_logo_state) recorder ----
 * Era: TAdSEGAlogo.s 0x8001C0F8 bl init_logo_state__23TAdSinglePlyChrSelectGCFl
 * (r3=0), TAdSoniclogo.s 0x8001C324 bl (r3=1 per li r3,0x1 @0x8001C308). */
extern "C" void init_logo_state__23TAdSinglePlyChrSelectGCFl(s32 v);
extern s32 g_init_logo_state_calls[4];
extern int g_init_logo_state_count;

/* ---- quest VM shapes (QuestVm.h:32-36 union, :82 qst_args[8]) ---------------
 * qst_args era provenance (QuestOpcodeHandlers.s qst_op_call_image_data):
 *   0x801EB5B4 lis r3, qst_args@ha / 0x801EB5BC addi / 0x801EB5C4 lwz r0,0x4(r3)
 *   0x801EB5C8 lwz r31,0x0(r3)  -> element stride 4 (union {s32;f32;void*}).
 * qst_label_address REAL shape @ .s 0x801F2C1C (see ladder3_test.cpp header). */
union QuestValue { s32 i; f32 f; };  /* era QuestVm.h:32-36 union also holds
    void *p; — dropped on the LP64 host so the 4-byte element stride the era
    .s proves (0x801EB5C4/5C8 loads 4 apart) is preserved. Deviation DA-6. */

/* ---- GSL archive entry (era GslArchive.h {char name[0x20]; s32 offset;
 * s32 size; s32 decoded; u8 f2C[4];}) — stride provenance is in the era
 * loop itself: find_gsl_entry .s 0x800FDDCC `addi r31,r31,0x30`.
 * Host keeps the identical 0x30 byte layout (ILP32-safe: no pointers).
 * One definition shared by the gsl glue TU and the test TU (ODR-clean). */
struct GslArchiveEntry {
    char name[0x20];
    s32 archive_offset;   /* +0x20 */
    s32 file_size;        /* +0x24 */
    s32 decoded_size;     /* +0x28 */
    u8  field_2C[4];      /* +0x2C */
};
typedef struct AdxfHandle AdxfHandle;

#endif /* LADDER3_HARNESS_H */
