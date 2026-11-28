/* P2-RDPREP step-1 — pso_input.h: THE single source of controller-input truth.
 * Era shapes are pulled from the era headers THROUGH the pin-include regime
 * (no paraphrase, ever — R-D law). Data DEFINED once in input_state.cpp;
 * sampler bodies are VERBATIM slices (gen_input_pump.py) emitted with the
 * L8-1 rename-at-emit law (free functions -> pso_era_*, methods keep era
 * names). The injection API writes ONLY the raw layer (PADStatus + keyboard
 * report + conn); every derived word is era-produced by the pump.
 * Docs: docs/p2-rd-input-1.md (design), docs/p2-rung7-seams-1.md (R-D/R-E). */
#ifndef PSO_INPUT_H
#define PSO_INPUT_H

#include "types.h"
#include <dolphin/pad.h>                        /* seat shim (field order = era SDK) */
#include "pso/system/ControllerInputState.h"    /* era 0x1B0 layout + LAYOUT_ASSERTs  */
#include "pso/ui/keyboard/TKeyBoardControl_803DE9C4.h"  /* PadSlot era shape           */
#include "pso/ui/keyboard/KeyboardKeyState.h"   /* KeyboardKeyState era shape          */
#include "pso/system/ControllerInputTypes.h"   /* ControllerHoldFrameBlock era shape  */
/* Deviation IN0-3 (disclosed): the era TCameraCtrl.h needs dolphin/mtx/*,
 * which the D4 exec-side include policy keeps OFF executing TUs (same reason
 * the bringup prelude ships its own counted shell). controller_stuff reads
 * ONLY ->view_yaw[i] and ONLY under camera_ctrl_instance != NULL — a branch
 * provably never taken in any seat run (host def NULL = era pre-camera truth,
 * RDPREP §3.2 disclosed posture). The shell below is the READER SURFACE ONLY;
 * its layout is asserted against nothing — the deref path is unreachable and
 * no C0-C3 leg touches an angle word. */
struct TCameraCtrl { s32 view_yaw[4]; };        /* reader face of TCameraCtrl.h:78      */
extern TCameraCtrl *camera_ctrl_instance;       /* era decl TCameraCtrl.h:209 (host NULL) */

/* ---- single-definition data (input_state.cpp; era order/defaults verbatim) ---- */
extern ControllerInputState controller_input_state;           /* cis  .bss:0x8050E208 */
extern ControllerInputState controller_physical_port_bank;    /* bank .bss:0x8050E3B8 */
extern PadSlot controller_default_pad_state;
extern u32 keyboard_pad_button_count;
extern PadSlot keyboard_control_pad_slots[4];
extern PADStatus pad_status[4];
extern PADStatus prev_pad_status[4];
extern s32 pad_slot_state_buffer[0x78];
extern s32 controller_device_id_table[4];
extern s32 controller_polling_enabled;
extern s32 pad_recalibration_counters[4];
extern s8  pad_recalibration_hold_frames[4];
extern u8  controller_reset_requested;
extern s8  keyboard_reset_button_pressed;
extern s8  lbl_805D5D08;
extern u32 TKeyBoardControl_key_bit_table[6];
extern u32 keyboard_poll_key_codes[6];
extern KeyboardKeyState hardware_keyboard_state;

/* ---- host witnesses (pinned externs in every receipt; never ambient) ---- */
extern s32 pso_split_screen_witness;      /* era is_split_screen() host answer: 0 = attract truth */
s32 is_split_screen(void);                /* consumed by the verbatim bind/controller slices      */
extern int pso_input_pump_live;           /* step-3 seam-wrapper gate, DEFAULT 0 (byte-green)     */

/* ---- verbatim pso_era_* samplers (generated: gen_input_pump.py) ---- */
extern "C" {
void  pso_era_init_pad_slots_and_read(s32, s32, s32);
void  pso_era_read_pads(void);
PadSlot *pso_era_get_pad_slot(u32 device);
s32   pso_era_map_pad_buttons(u16 button_bits);
s16   pso_era_scale_axis_value(s16 x);
void  pso_era_clamp_pad_axes(PADStatus *pad);
}
void  pso_era_controller_stuff(void);
void  pso_era_bind_controller_devices(void);
void  pso_era_update_keyboard_input_frame(void);
void  pso_era_reset_controller_input_state(void);
void  pso_era_init_pad_state(void);
s32   pso_era_any_pad_input_active(void);
s32   pso_era_test_pressed_buttons_packed(void *pad, s32 mask);
s32   pso_era_test_previous_buttons_packed(void *pad, s32 mask);
s32   pso_era_test_controller_flag_4c_color(ControllerInputContext *pad, s32 mask);

extern "C" {
s32 OSGetResetButtonState(void);                /* seat shim (input_state.cpp, host 0) */
}

/* ---- injection API (writes ONLY the raw layer — §3.3 design law) ---- */
void  pso_input_init(void);                              /* era-idle through the real chains */
void  pso_input_pad_connect(s32 port0_3, int present);   /* pad_status[port].err 0 / 1       */
void  pso_input_pad_press(s32 port0_3, u16 pad_bits);    /* PADStatus-native button word     */
void  pso_input_pad_axes(s32 port0_3, s16 sx, s16 sy, s16 cx, s16 cy); /* raw PADStatus units */
void  pso_input_pad_triggers(s32 port0_3, u8 l, u8 r);
void  pso_input_key_report(const u8 *raw, u32 n);        /* drives hardware_keyboard_state   */
void  pso_input_pump_frame(void);                        /* read->bind->stuff->kbd, era order */
s32   pso_input_logical_slot(s32 physical_port);         /* compaction-law reader (no writes) */

#endif
