/* P2-RDPREP step-1 — input_test.cpp: the hg3_input canary suite.
 * C0 era-idle promotion canary (HG-287 DEFAULT-DECLINE rider), C1 compaction
 * divergence, C2 derived-word edge law, C3 keyboard mirror pump-only.
 * Numbers from live reads of the REAL globals, never from the injection.
 * R-E law: every press gate is probed on BOTH pad (bank) and keyboard-mirror
 * sources; teeth proven by planted-A flip + restore. */
#include "pso_input.h"
#include <stdio.h>

/* the seat links exactly ONE def site for the camera reader (bringup keeps
 * its own in ladder7_seam.o for its binaries; the input binary is standalone) */
TCameraCtrl *camera_ctrl_instance;

static int pass_n, total_n, failed;
#define CHECK(cond, name) do { total_n++; if (cond) pass_n++; \
    else { failed = 1; printf("FAIL %s\n", name); } } while (0)

static void pump_n(int n) { for (int i = 0; i < n; i++) pso_input_pump_frame(); }

static int idle_fields(const ControllerInputContext *c) {
    if (c->buttons != 0 || c->not_held != 0xFFFFFFFFu || c->pressed_buttons != 0 ||
        c->released != 0 || c->trigger_l != 0 || c->trigger_r != 0 ||
        c->stick_x != 0 || c->stick_y != 0 || c->cstick_x != 0 || c->cstick_y != 0 ||
        c->held_with_sticks != 0 || c->pressed_with_sticks != 0 ||
        c->stick_magnitude != 0.0f) return 0;
    for (int j = 0; j < 14; j++) if (c->hold_frames[j] != 0) return 0;
    return 1;
}

int main(void) {
    /* ================= C0: un-injected frame reads era-idle ================= */
    pso_input_init();
    pump_n(3);
    int idle_ok = 1;
    for (int i = 0; i < 4; i++)
        idle_ok &= idle_fields(&controller_physical_port_bank.controllers[i]) &&
                   idle_fields(&controller_input_state.controllers[i]);
    CHECK(idle_ok, "C0a era disconnected-branch values (ControllerInput.cpp:402-419) on bank+cis");
    int era_zero = 1;
    for (int i = 0; i < 14; i++) {
        s32 m = (s32)input_button_mask_table[i];
        era_zero &= pso_era_test_pressed_buttons_packed(&controller_physical_port_bank.controllers[0], m) == 0;
        era_zero &= pso_era_test_controller_flag_4c_color(&controller_input_state.keyboard, m) == 0;
    }
    CHECK(era_zero, "C0b all-14-mask era-answer-for-zero-input, bank pad AND keyboard mirror (R-E both sides)");
    /* HG-287 default-decline rider: UN-injected, the agree gate (A, mask 4)
     * and the B-cancel gate (mask 2) read era-zero from BOTH sources the
     * state machine consults (TAdAgreementBaseGC.cpp:76 pair) — with
     * set_selection_index(1) = DECLINE default, an un-injected run can only
     * quit, never agree. */
    CHECK(pso_era_test_pressed_buttons_packed(&controller_physical_port_bank.controllers[0], 4) == 0 &&
          pso_era_test_pressed_buttons_packed(&controller_physical_port_bank.controllers[0], 2) == 0 &&
          pso_era_test_controller_flag_4c_color(&controller_input_state.keyboard, 4) == 0 &&
          pso_era_test_controller_flag_4c_color(&controller_input_state.keyboard, 2) == 0,
          "C0/HG-287 no-injected-input agree path: DEFAULT-DECLINE held at the gate level");
    /* (c) TEETH: plant ONE PADStatus-native A (PAD bit 0x100 -> game 0x4). */
    pso_input_pad_connect(0, 1);
    pump_n(1);                                  /* settle the connect edge */
    pso_input_pad_press(0, 0x100);
    pump_n(1);                                  /* frame: press edge */
    int flip = pso_era_test_pressed_buttons_packed(&controller_physical_port_bank.controllers[0], 4) &&
               pso_era_test_pressed_buttons_packed(&controller_input_state.controllers[0], 4);
    CHECK(flip == 1, "C0c teeth: planted A FLIPS mask 4 on bank AND cis (one pump feeds both)");
    pso_input_pad_press(0, 0x0);
    pump_n(2);                                  /* release + settle */
    CHECK(pso_era_test_pressed_buttons_packed(&controller_physical_port_bank.controllers[0], 4) == 0,
          "C0c teeth: restore returns to idle");
    pso_input_pad_connect(0, 0);
    pump_n(2);
    CHECK(idle_fields(&controller_physical_port_bank.controllers[0]),
          "C0c disconnect rides back to era disconnected values");

    /* ================= C1: compaction divergence ================= */
    pso_input_init();
    pso_input_pad_connect(0, 1);
    pso_input_pad_connect(2, 1);                /* devices {0,12} only */
    pump_n(2);
    CHECK(controller_physical_port_bank.sources[1] == &controller_default_pad_state &&
          controller_physical_port_bank.source_status[1] == 0,
          "C1 bank stays PHYSICAL 1:1 (port 1 default, status 0)");
    CHECK(controller_input_state.sources[1] == &keyboard_control_pad_slots[2] &&
          controller_input_state.source_status[1] == 1,
          "C1 cis COMPACTION (port 2 lands at logical slot 1, status 1)");
    pso_input_pad_press(2, 0x100);
    pump_n(1);
    CHECK((controller_input_state.controllers[1].pressed_with_sticks & 0x4) &&
          (controller_physical_port_bank.controllers[2].pressed_with_sticks & 0x4),
          "C1 same raw, two mappings: press visible at cis[1] AND bank[2]");
    CHECK(pso_input_logical_slot(2) == 1 && pso_input_logical_slot(1) == -1,
          "C1 logical_slot helper reads the era compaction (no hand-replication)");

    /* ================= C2: derived-word edge law (stick) ================= */
    pso_input_init();
    pso_input_pad_connect(0, 1);
    pso_input_pad_axes(0, 0, -60, 0, 0);        /* PADStatus-native; era flips stickY */
    pump_n(1);
    CHECK(controller_physical_port_bank.controllers[0].stick_y == 86,
          "C2 era scale/flip: -scale_axis_value(-60) == 86 at the context");
    CHECK(controller_physical_port_bank.controllers[0].pressed_with_sticks & 0x200000,
          "C2 pressed (up-bit) on FIRST frame");
    CHECK(controller_physical_port_bank.controllers[0].held_with_sticks & 0x200000,
          "C2 held on second... (first-frame held set too via map)");
    pump_n(1);
    CHECK((controller_physical_port_bank.controllers[0].pressed_with_sticks & 0x200000) == 0 &&
          (controller_physical_port_bank.controllers[0].held_with_sticks & 0x200000) != 0,
          "C2 edge law: pressed CLEARED on frame 3, held persists (set_stick_button_bits gate)");

    /* ================= C3: keyboard mirror is pump-produced ================= */
    pso_input_init();
    pso_input_key_report((const u8[]){ 94 }, 1);
    CHECK(pso_era_test_controller_flag_4c_color(&controller_input_state.keyboard, 16) == 0,
          "C3 no pump => no press (the mirror is NOT a setter)");
    pump_n(1);
    CHECK(pso_era_test_controller_flag_4c_color(&controller_input_state.keyboard, 16) != 0,
          "C3 report 94 -> key_bit 16 at cis.keyboard THROUGH the pump");
    pump_n(1);
    CHECK(pso_era_test_controller_flag_4c_color(&controller_input_state.keyboard, 16) == 0,
          "C3 exactly one frame (pending/pressed drain per pump — poll_key_state rebuild)");

    printf("hg3_input: %d/%d%s\n", pass_n, total_n, failed ? " FAIL" : "");
    return failed;
}
