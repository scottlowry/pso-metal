/* pso-metal P2-AB — era-shape harness for the verbatim decomp slices.
 *
 * The generated slice TU (build/vi_branch/vi_era_slice.cpp, produced by
 * vi_slice_gen.py from pso-decomp @ 6b38b5383, sha-pinned per range) contains
 * REAL function bodies copied byte-verbatim:
 *   TLogo.cpp:36 (logo_class_name), :56 (is_logo_clear_flag_set),
 *   :123-131 (TLogo ctor), :135-197 (TLogo::run_task, state machine)
 *   TAdVIConfigGC.cpp:87 (logo_clear_flag = 1  — the W1 shipped-value fact),
 *   :192 (TAdVIConfigGC::disable_progressive_mode),
 *   :226-236 (TLogo::check_progressive_ready — the W1 contract gate)
 * The bodies are NOT edited; this header only supplies the minimal class/
 * symbol surface so they PARSE and LINK in a host TU set. Class shapes here
 * are declared-precision mirrors of the era headers (cited per member);
 * struct LAYOUT is host-irrelevant for control-flow proof and stays the
 * layout lane's job (compat/P2-T). Real era headers are not included because
 * their include-graph cost is exactly what P2-T triage is budgeting.
 */
#ifndef PSO_VI_ERA_HARNESS_H
#define PSO_VI_ERA_HARNESS_H

#include "types.h"        /* bringup shadow (same -I contract as gx_era.h) */
#include "gx_era.h"       /* era GX/VI decls incl. int VIGetDTVStatus(void) (:52) */
#include <cstddef>        /* NULL */

/* ---- era OSRtc progressive-mode pair ------------------------------------- *
 * libs/dolphin/include/dolphin/os/OSRtc.h:83-84 (u32 return/arg). Backed by
 * host/gx/gx_vi.c. */
extern "C" {
u32 OSGetProgressiveMode(void);
void OSSetProgressiveMode(u32 on);
}

/* ---- era controller surface (branch's second gate) ------------------------ *
 * include/pso/system/ControllerInputState.h:41-56 declares
 * `ControllerInputState controller_physical_port_bank` with
 * `ControllerInputContext controllers[4]` (context size 0x50,
 * LAYOUT_ASSERT ControllerInputContext_size). The slice only takes
 * &controllers[0] as void*; host truth = zero controllers (no PAD lane at
 * logo) — bank is zero-init and the stubbed test_previous_buttons_packed
 * (include/pso/system/ControllerInput.h:52) returns "not pressed", which is
 * the era answer for zero controller input (docs/p2-defer-audit.md A3). */
struct ControllerInputContext { u8 raw[0x50]; };
struct ControllerInputState {
    ControllerInputContext controllers[4];
    ControllerInputContext keyboard;
};
extern ControllerInputState controller_physical_port_bank;
s32 test_previous_buttons_packed(void *pad, s32 mask);

/* ---- era object tree (minimal shapes; ctor bodies count fires) ----------- */
struct TObject {
    TObject(TObject *parent);
    const char *m_name;      /* TLogo ctor sets m_name = logo_class_name */
};

struct TAdTrialModemChkGC { TAdTrialModemChkGC(TObject *parent); };
struct TAdSEGAlogo   { TAdSEGAlogo(TObject *parent);   void queue_destruction(); };
struct TAdSoniclogo  { TAdSoniclogo(TObject *parent);  void queue_destruction(); };
struct TAdADXlogo    { TAdADXlogo(TObject *parent);    void queue_destruction(); };

/* include/pso/ui/frontend/TAdVIConfigGC.h — disable_progressive_mode is
 * STATIC there (:21), which is what makes the unqualified-object call at
 * TAdVIConfigGC.cpp:228 valid era C++. The real ctor body belongs to the
 * dialog/fonts lane (W4); the harness ctor records the construction. */
struct TAdVIConfigGC {
    TAdVIConfigGC(TObject *parent);
    static void disable_progressive_mode();
    u32 m_48;                /* TAdVIConfigGC.h:18 — set by set_field48_clear_global */
};

/* include/pso/ui/TLogo.h — check_progressive_ready is static (:26); fields
 * mirror :29-36 (u32 state; five object pointers). run_task is virtual in
 * the era class; non-virtual here is link-equivalent for direct calls. */
struct TLogo : TObject {
    TLogo(TObject *parent);
    static s32 check_progressive_ready();
    void run_task();
    u32 state;
    TAdVIConfigGC      *vi_config;
    TAdTrialModemChkGC *modem_check;
    TAdSEGAlogo        *sega_logo;
    TAdSoniclogo       *sonic_logo;
    TAdADXlogo         *adx_logo;
};

/* logo_clear_flag ships 1 (TAdVIConfigGC.cpp:87 — sliced verbatim; declared
 * bare-C++ like TAdVIConfigGC.h:30). is_logo_clear_flag_set is extern "C"
 * per TLogo.h:41-44 — the slice at TLogo.cpp:56 defines it verbatim. */
extern s32 logo_clear_flag;

/* Remaining extern "C" helpers referenced by the verbatim run_task slice —
 * defined as counting stubs in vi_branch_driver.c (host-truth values; none
 * is on the asserted state-0 frame-1 path except via the branch under test). */
extern "C" {
s32 is_vi_config_answered(void *p);          /* TLogo.h:42 / TLogo.cpp:57    */
int is_sega_logo_done(TAdSEGAlogo *logo);    /* TLogo.cpp:59                 */
int is_logo_exit_requested(TAdSoniclogo *l); /* TLogo.cpp:61 era decl        */
int is_adx_logo_done(TAdADXlogo *logo);      /* TLogo.h:41                   */
int has_network_adapter(int kind);           /* text_80145A94.cpp:169 (A4:
                                                state-2 path is retail-dead) */
void request_main_phase(int phase);          /* state-7 exit, out of scope   */
}

#endif /* PSO_VI_ERA_HARNESS_H */
