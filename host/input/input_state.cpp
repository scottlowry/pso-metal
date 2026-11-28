/* P2-RDPREP step-1 — input_state.cpp: THE single definition TU for all
 * controller-input data + the raw-layer injection API + the host shims the
 * verbatim slices call. Definitions and initializers copied VERBATIM from
 * ControllerInput.cpp:17-41 and ChunkMaterialState.cpp:24/198-206 at pin
 * 6b38b53837; cis-before-bank def order preserved (era .bss adjacency law,
 * docs/p2-rd-input-1.md §1.1). No context field is ever written by the API. */
#include "pso_input.h"
#include <string.h>

/* ---- ControllerInput.cpp data (era order 17..41, verbatim initializers) ---- */
const u32 input_button_mask_table[14] = {
    0x10, 0x20, 0x40, 0x80, 0x100000, 0x200000, 0x400000, 0x800000,
    0x4000000, 0x8000000, 0x10000000, 0x20000000, 0x20000, 0x10000,
};
s8 lbl_805D5D08;
u8 controller_reset_requested;
s8 keyboard_reset_button_pressed;
s8 pad_recalibration_hold_frames[4];
PadSlot controller_default_pad_state = { 0, 0, 0, 0xFFFFFFFF, 0, 0, 0, 0, 0, 0, 0, 0, { 0 }, 0 };
u32 TKeyBoardControl_key_bit_table[6] = { 16, 32, 64, 128, 4, 2 };
u32 keyboard_poll_key_codes[6] = { 94, 93, 92, 95, 97, 76 };
s32 controller_device_id_table[4] = { 0, 6, 12, 18 };
s32 controller_polling_enabled = 1;
ControllerInputState controller_input_state;        /* era def order: cis first */
ControllerInputState controller_physical_port_bank; /* .bss adjacency law       */
s32 pad_recalibration_counters[4];

/* ---- ChunkMaterialState.cpp data (:24/:198-206, verbatim) ---- */
u32 keyboard_pad_button_count = 12;
PADStatus pad_status[4];
PADStatus prev_pad_status[4];
PadSlot keyboard_control_pad_slots[4];
s32 pad_slot_state_buffer[0x78];
extern PadButtonMapping pad_button_map_table[12];   /* EMITTED by the pump glue (data leg) */

/* ---- KeyboardKeyState.cpp data (.bss:0x80506270) ---- */
KeyboardKeyState hardware_keyboard_state;

/* ---- host witnesses (pinned, never ambient — §4 split-screen law) ---- */
s32 pso_split_screen_witness = 0;   /* 0 = era attract truth */
int pso_input_pump_live = 0;        /* step-3 gate: DEFAULT OFF keeps rung-5/6 byte-green */
s32 is_split_screen(void) { return pso_split_screen_witness; }

/* ---- newton_sqrtf coefficient pair (TObjChara.cpp:160-161 externs; the era
 * .data values are 0.5/3.0 — standard Newton half/three constants. Deviation
 * IN0-2 disclosed: harness-supplied constants; no C0-C3 leg asserts a
 * magnitude/angle word. Math-slice BODY rides the generated glue. ---- */
extern const f64 newton_sqrt_half;
extern const f64 newton_sqrt_three;
const f64 newton_sqrt_half = 0.5;
const f64 newton_sqrt_three = 3.0;

/* ---- dolphin OS shims: the injection face for the verbatim sampler ---- */
u32 pso_shim_pad_reset_mask;      /* PADReset channel mask, recorded */
u32 pso_shim_pad_recal_mask;      /* PADRecalibrate mask, recorded   */
u32 pso_shim_pad_read_calls;
u32 pso_shim_os_reset_btn_state;  /* OSGetResetButtonState answer (era-hw absent, default 0) */
extern "C" {
s32 PADRead(PADStatus *status) {
    pso_shim_pad_read_calls++;
    memcpy(status, pad_status, sizeof(pad_status));
    return 4;
}
s32 PADReset(u32 mask) { pso_shim_pad_reset_mask |= mask; return 1; }
s32 PADRecalibrate(u32 mask) { pso_shim_pad_recal_mask |= mask; return 1; }
s32 OSGetResetButtonState(void) { return pso_shim_os_reset_btn_state; }
}

/* ---- injection API (raw layer ONLY — §3.3 design law) ---- */
void pso_input_init(void) {
    memset(&controller_input_state, 0, sizeof(controller_input_state));
    memset(&controller_physical_port_bank, 0, sizeof(controller_physical_port_bank));
    memset(keyboard_control_pad_slots, 0, sizeof(keyboard_control_pad_slots));
    memset(pad_status, 0, sizeof(pad_status));
    memset(prev_pad_status, 0, sizeof(prev_pad_status));
    memset(pad_slot_state_buffer, 0, sizeof(pad_slot_state_buffer));
    memset(&hardware_keyboard_state, 0, sizeof(hardware_keyboard_state));
    pso_split_screen_witness = 0;
    controller_polling_enabled = 1;
    pso_shim_pad_reset_mask = pso_shim_pad_recal_mask = pso_shim_pad_read_calls = 0;
    pso_shim_os_reset_btn_state = 0;
    /* era-idle posture: every slot reports an error => read_pads takes the
     * *conn=0 + PADReset branch through the REAL chain (ControllerInput's
     * disconnected default-fill answers at the contexts). */
    for (int i = 0; i < 4; i++) { pad_status[i].err = 1; pad_status[i].button = 0; }
    hardware_keyboard_state.clear_key_state();
    hardware_keyboard_state.reset_keys();
    pso_era_init_pad_slots_and_read(0, 0, 0);  /* wires conn ptrs + era first read */
    pso_era_bind_controller_devices();         /* era bind semantics (idle answer) */
}

void pso_input_pad_connect(s32 port0_3, int present) {
    if ((u32)port0_3 >= 4) return;
    pad_status[port0_3].err = present ? 0 : 1;
    if (!present) { pad_status[port0_3].button = 0;
        pad_status[port0_3].stickX = pad_status[port0_3].stickY = 0;
        pad_status[port0_3].substickX = pad_status[port0_3].substickY = 0;
        pad_status[port0_3].triggerLeft = pad_status[port0_3].triggerRight = 0; }
}

void pso_input_pad_press(s32 port0_3, u16 pad_bits) {
    if ((u32)port0_3 >= 4) return;
    pad_status[port0_3].button = pad_bits;   /* PADStatus-native; map_pad_buttons runs era-verbatim */
}

void pso_input_pad_axes(s32 port0_3, s16 sx, s16 sy, s16 cx, s16 cy) {
    if ((u32)port0_3 >= 4) return;
    pad_status[port0_3].stickX = (s8)sx;     /* raw PADStatus units (+-89 clamp era-side) */
    pad_status[port0_3].stickY = (s8)sy;
    pad_status[port0_3].substickX = (s8)cx;
    pad_status[port0_3].substickY = (s8)cy;  /* era flips cstick_y = -substickY downstream */
}

void pso_input_pad_triggers(s32 port0_3, u8 l, u8 r) {
    if ((u32)port0_3 >= 4) return;
    pad_status[port0_3].triggerLeft = l;
    pad_status[port0_3].triggerRight = r;
}

void pso_input_key_report(const u8 *raw, u32 n) {
    if (n > 0x80) n = 0x80;
    memcpy(hardware_keyboard_state.raw, raw, n);
    if (n < 0x80) memset(&hardware_keyboard_state.raw[n], 0, 0x80 - n);
    hardware_keyboard_state.raw_count = (u8)n;
    hardware_keyboard_state.reports = 1;     /* the era scanner would set this on a hw report */
}

void pso_input_pump_frame(void) {
    pso_era_read_pads();                     /* EXACT era order (one read_pads per frame) */
    pso_era_bind_controller_devices();
    pso_era_controller_stuff();
    pso_era_update_keyboard_input_frame();
}

s32 pso_input_logical_slot(s32 physical_port) {
    if ((u32)physical_port >= 4) return -1;
    for (s32 i = 0; i < 4; i++)
        if (controller_input_state.sources[i] == &keyboard_control_pad_slots[physical_port])
            return i;
    return -1;                               /* port not bound (disconnected under compaction) */
}
