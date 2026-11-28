#!/usr/bin/env python3
"""P2-RDPREP step-0/1 — gen_input_pump.py.
Extracts the controller-input producer chain VERBATIM (awk sig->^} block
grammar, ladder3/7/9 lineage) off the READ-ONLY decomp tree and emits one
generated TU, renamed at emit (L8-1 law): free functions -> pso_era_*;
methods keep era names. Guards: every signature line must occur EXACTLY
once in its home file; hand-traced body lines must be present; era data
lines copied verbatim. Writes the input_slices.sha256 manifest pinned to
6b38b53837 (era file shas + per-slice shas). Fail-loud on any drift.

usage: gen_input_pump.py DECOMP_ROOT OUT.cpp [MANIFEST.sha256]
"""
import hashlib, re, sys

if len(sys.argv) < 3:
    sys.exit("usage: gen_input_pump.py DECOMP_ROOT OUT.cpp [MANIFEST]")
DECOMP, OUT = sys.argv[1], sys.argv[2]
MANIFEST = sys.argv[3] if len(sys.argv) > 3 else None

FREE_RENAME = [
    # ChunkMaterialState.cpp (era extern "C" region)
    "init_pad_slots_and_read", "read_pads", "get_pad_slot", "map_pad_buttons",
    "scale_axis_value", "clamp_pad_axes",
    # ControllerInput.cpp
    "any_pad_input_active", "clear_controller_reset_request", "check_reset_button",
    "is_keyboard_control_held", "keyboard_check_and_read", "keyboard_read_input",
    "keyboard_is_key_down", "is_keyboard_key_pressed", "update_pad_recalibration",
    "update_keyboard_input_frame", "controller_stuff", "bind_controller_devices",
    "reset_controller_input_state", "copy_controller_input", "init_pad_state",
    "accumulate_key_bits", "set_stick_button_bits", "map_c_stick_to_buttons",
    "map_main_stick_to_buttons", "update_button_hold_counter",
    "update_button_hold_counters", "all_direction_mask", "cstick_direction_mask",
    "pack_button_mask", "test_flags_04_packed", "test_controller_flag_4c_color",
    "test_pressed_buttons_packed", "test_previous_buttons_color",
    "test_previous_buttons_remapped", "test_previous_buttons_packed",
    "test_buttons_press_or_repeat_color", "test_buttons_repeat_packed",
    "test_buttons_press_or_repeat", "check_packed_flag_match",
    "input_press_or_repeat",
]
RENAME_RE = re.compile(r"\b(" + "|".join(FREE_RENAME) + r")\b")

CIS = f"{DECOMP}/src/pso/system/ControllerInput.cpp"
CMS = f"{DECOMP}/src/pso/render/ChunkMaterialState.cpp"
KBS = f"{DECOMP}/src/pso/ui/keyboard/KeyboardKeyState.cpp"
MTH = f"{DECOMP}/src/pso/object/base/TObjChara.cpp"

def load(p):
    with open(p, encoding="utf-8") as f:
        return f.read().splitlines()

def slice_fn(lines, sig, oneline=False):
    hits = [i for i, l in enumerate(lines) if l == sig]
    if len(hits) != 1:
        sys.exit(f"GUARD: signature occurs {len(hits)}x (need exactly 1): {sig}")
    i = hits[0]
    if oneline:
        if not lines[i].rstrip().endswith("}"):
            sys.exit(f"GUARD: expected one-line body: {sig}")
        return lines[i]
    out = [lines[i]]
    i += 1
    while i < len(lines) and lines[i] != "}":
        out.append(lines[i]); i += 1
    if i >= len(lines):
        sys.exit(f"GUARD: no closing ^}} for {sig}")
    out.append("}")
    return "\n".join(out)

def slice_data(lines, start_sig, end="};"):
    hits = [i for i, l in enumerate(lines) if l.startswith(start_sig)]
    if len(hits) != 1:
        sys.exit(f"GUARD: data anchor occurs {len(hits)}x: {start_sig}")
    i = hits[0]; out = []
    while True:
        out.append(lines[i])
        if lines[i].rstrip().endswith("};"): break
        i += 1
    return "\n".join(out)

cis, cms, kbs, mth = load(CIS), load(CMS), load(KBS), load(MTH)

# ---- slices (name, file-lines, exact sig, hand-traced guard lines) ----
S = []  # (era-file, sig, text, guards)
def add(home, lines, sig, guards, oneline=False):
    body = slice_fn(lines, sig, oneline)
    for g in guards:
        if g not in body:
            sys.exit(f"GUARD: slice {sig} missing traced line {g!r}")
    S.append((home, sig, body))

add(CMS, cms, "s16 scale_axis_value(s16 x) {", ["(x * 128) / 89"])
add(CMS, cms, "s32 map_pad_buttons(u16 released) {", ["keyboard_pad_button_count", "pad_button_map_table[i].pad_mask"])
add(CMS, cms, "void clamp_pad_axes(PADStatus *pad) {", ["0x59"])
add(CMS, cms, "PadSlot *get_pad_slot(u32 device) {", ["case 12:", "keyboard_control_pad_slots[slot]"])
add(CMS, cms, "void init_pad_slots_and_read(s32, s32, s32) {", ["pad_slot_state_buffer[0x5A]", "PADRead(pad_status);"])
add(CMS, cms, "void read_pads() {", ["*out->conn = 0x101;", "PADReset(0x80000000U >> i);", "out->stickY = -scale_axis_value(pad->stickY);"])
add(CIS, cis, "void clear_controller_reset_request() { controller_reset_requested = 0; }", ["= 0; }"], oneline=True)
add(CIS, cis, "s32 is_keyboard_key_pressed(u8 key) {", ["is_key_pressed(key)"])
add(CIS, cis, "s32 keyboard_read_input(u8 key) {", ["check_input_ready", "is_keyboard_key_pressed"])
add(CIS, cis, "s32 keyboard_is_key_down(s32 key) {", ["get_pending_key(i)"])
add(CIS, cis, "s32 keyboard_check_and_read(s32 key) {", ["keyboard_is_key_down"])
add(CIS, cis, "s32 is_keyboard_control_held(void) {", ["is_control_held()"])
add(CIS, cis, "s32 any_pad_input_active() {", ["stickX <= -10"])
add(CIS, cis, "void set_stick_button_bits(ControllerInputContext *pad, u32 mask, u32 previous) {", ["pressed_with_sticks |= mask"])
add(CIS, cis, "void map_c_stick_to_buttons(ControllerInputContext *pad, s16 x, s16 y, s32 previous) {", ["0x08000000", "0x10000000"])
add(CIS, cis, "void map_main_stick_to_buttons(ControllerInputContext *pad, s16 x, s16 y, s32 previous) {", ["0x200000", "0x400000"])
add(CIS, cis, "void update_button_hold_counter(ControllerInputContext *pad, s16 *counter, s32 mask, s32 held) {", ["*counter = frames"])
add(CIS, cis, "void update_button_hold_counters(ControllerInputContext *pad) {", ["hold_frames[13], 0x10000"])
add(CIS, cis, "s32 all_direction_mask(ControllerInputContext *pad, s32 mask) {", ["0xF00000", "0x3C000000"])
add(CIS, cis, "s32 cstick_direction_mask(ControllerInputContext *pad, s32 mask) {", ["0xFFFFFF0F"])
add(CIS, cis, "s32 pack_button_mask(ControllerInputContext *pad, s32 mask) {", ["0xFF0FFFFF"])
add(CIS, cis, "s32 test_flags_04_packed(ControllerInputContext *pad, s32 mask) {", ["not_held &"])
add(CIS, cis, "s32 test_controller_flag_4c_color(ControllerInputContext *pad, s32 mask) {", ["all_direction_mask"])
add(CIS, cis, "s32 test_pressed_buttons_packed(void *pad, s32 mask) {", ["pressed_with_sticks & pack_button_mask"])
add(CIS, cis, "s32 test_previous_buttons_color(ControllerInputContext *pad, s32 mask) {", ["held_with_sticks &"])
add(CIS, cis, "s32 test_previous_buttons_remapped(ControllerInputContext *pad, s32 mask) {", ["cstick_direction_mask"])
add(CIS, cis, "s32 test_previous_buttons_packed(void *pad, s32 mask) {", ["held_with_sticks & pack_button_mask"])
add(CIS, cis, "s32 check_packed_flag_match(ControllerInputContext *pad, u32 mask, s32 delay, s32 period) {", ["input_button_mask_table[i]"])
add(CIS, cis, "s32 input_press_or_repeat(ControllerInputContext *pad, u32 mask, s32 delay, s32 period) {", ["pressed_with_sticks & mask"])
add(CIS, cis, "s32 test_buttons_press_or_repeat_color(ControllerInputContext *input, s32 buttons, s32 delay, s32 period) {", ["input_press_or_repeat"])
add(CIS, cis, "s32 test_buttons_repeat_packed(ControllerInputContext *input, s32 buttons, s32 delay, s32 period) {", ["check_packed_flag_match"])
add(CIS, cis, "s32 test_buttons_press_or_repeat(ControllerInputContext *pad, s32 buttons, s32 delay, s32 period) {", ["input_press_or_repeat"])
add(CIS, cis, "s32 check_reset_button() {", ["OSGetResetButtonState"])
add(CIS, cis, "void update_pad_recalibration(void) {", ["PADRecalibrate(channels[j]);", "0x40A", "9.0e1f"])
add(CIS, cis, "void copy_controller_input(ControllerInputContext *destination, ControllerInputContext *source) {", ["ControllerHoldFrameBlock", "*d = *s;"])
add(CIS, cis, "void bind_controller_devices(void) {", ["*slot->conn & 1", "is_split_screen() != 0", "controller_default_pad_state;"])
add(CIS, cis, "void controller_stuff(void) {",
    ["cstick_y = -slot->substickY;", "pad->held_with_sticks = pad->buttons &= 0xC30FFFFF;",
     "newton_sqrtf(stick_x * stick_x + stick_y * stick_y)", "atan2_float(cstick_x, cstick_y)",
     "pad->not_held = controller_default_pad_state.notHeld;", "update_pad_recalibration();"])
add(CIS, cis, "void update_keyboard_input_frame() {", ["process_key_reports();", "keyboard->poll_key_state();"])
add(CIS, cis, "void reset_controller_input_state(void) {", ["copy_controller_input(&controller_input_state.keyboard", "clear_controller_reset_request();"])
add(CIS, cis, "void init_pad_state() {", ["get_pad_slot(0xC)", "controller_polling_enabled = 1;"])
add(CIS, cis, "void accumulate_key_bits(s32 key, u32 *buttons) {", ["*buttons |= TKeyBoardControl_key_bit_table[i];"])
add(CIS, cis, "s32 ControllerInputContext::poll_key_state() {", ["keyboard_poll_key_codes[i]", "accumulate_key_bits"])
add(MTH, mth, "f32 newton_sqrtf(f32 x) {", ["newton_sqrt_half", "__frsqrte"])
add(MTH, mth, "f32 atan2_float(f32 y, f32 x) {", ["atan2((f64) y, (f64) x)"])
add(KBS, kbs, "void KeyboardKeyState::process_key_reports() {", ["pending_count = 0;","process_raw_key(this, raw[i], bits);"])
add(KBS, kbs, "s32 KeyboardKeyState::check_input_ready() {", ["is_alt_held()"])
add(KBS, kbs, "s32 KeyboardKeyState::has_any_key_pressed() {", ["pressed[i] != 0"])
add(KBS, kbs, "u8 KeyboardKeyState::get_pending_key(u8 index) {", ["pending[index]"])
add(KBS, kbs, "u8 KeyboardKeyState::get_pending_count() {", ["return pending_count;"])
add(KBS, kbs, "s32 KeyboardKeyState::is_key_pressed(u8 key) {", ["key / 32", "key % 32"])
add(KBS, kbs, "s32 KeyboardKeyState::is_control_held() {", ["is_key_pressed(0x5B)"])
add(KBS, kbs, "s32 KeyboardKeyState::is_alt_held() {", ["is_key_pressed(0x57)"])
add(KBS, kbs, "s32 KeyboardKeyState::is_shift_held() {", ["is_key_pressed(0x55)"])
add(KBS, kbs, "void KeyboardKeyState::push_key_to_buffer(u8 key) {", ["pending[count] = key;"])
add(KBS, kbs, "void KeyboardKeyState::reset_inactive_keys() {", ["p->timer = 0x23;"])
add(KBS, kbs, "void KeyboardKeyState::tick_key_repeat_timer(u8 key) {", ["p->timer += 6;"])
add(KBS, kbs, "void KeyboardKeyState::process_raw_key(KeyboardKeyState *kb, u8 key, u32 *pressed) {", ["valid_key_mask[word]", "kb->tick_key_repeat_timer(key);"])
add(KBS, kbs, "void KeyboardKeyState::reset_keys() {", ["raw[i] = 0;"])
add(KBS, kbs, "void KeyboardKeyState::clear_key_state() {", ["repeat[i].timer = 0x23;"])
add(KBS, kbs, "u8 KeyboardKeyState::get_key_state() {", ["return"])

# era data lines (verbatim)
VKM = slice_data(kbs, "static u32 valid_key_mask[4]")
PBMT = slice_data(cms, "PadButtonMapping pad_button_map_table[12] = {")

def rename(text):
    return RENAME_RE.sub(lambda m: "pso_era_" + m.group(1), text)

PROTOS = "\n".join(
    ["// generated prologues: pso_era_* free-fn decls (rename-at-emit, L8-1 law)",
     "extern const f64 newton_sqrt_half;", "extern const f64 newton_sqrt_three;",
     "s32 OSGetResetButtonState(void);"] +
    [f"s32 pso_era_{n}(ControllerInputContext *, s32);" for n in []] )

CHUNK_HOMES = {CMS}
parts = []
chunk_block, plain_block = [], []
for home, sig, body in S:
    b = rename(body)
    if home == CMS and "::" not in sig:
        chunk_block.append(f"/* verbatim: {home.split('/')[-1]} */\n{b}")
    else:
        plain_block.append(f"/* verbatim: {home.split('/')[-1]} */\n{b}")

protos_c = "\n".join(
    rename(sig.rstrip("{").rstrip()).strip() + ";"
    for home, sig, body in S if home == CMS and "::" not in sig)
protos_cpp = "\n".join(
    rename(sig.rstrip("{").rstrip()).strip() + ";"
    for home, sig, body in S if home != CMS and "::" not in sig and "clear_controller" not in sig)
hdr = f"""/* GENERATED by gen_input_pump.py — VERBATIM slices, renamed at emit
 * (pso_era_* free fns; methods keep era names). Era sources READ-ONLY at the
 * pin; every body guarded by sig + hand-traced lines. DO NOT EDIT.
 * Legs: RDPREP design steps 1-5 (docs/p2-rd-input-1.md §3.2). */
#include "pso_input.h"
#include <string.h>
#include <math.h>

/* era math homes (TObjChara.cpp decls; __frsqrte fronted by compat shim —
 * deviation IN0-1, magnitude/angle words NOT asserted by any C0-C3 leg) */
extern const f64 newton_sqrt_half;
extern const f64 newton_sqrt_three;
f32 newton_sqrtf(f32 x);
f32 atan2_float(f32 y, f32 x);
/* renamed free-fn forward prototypes (order-independent emission) */
extern "C" {{
{protos_c}
}}
{protos_cpp}

/* verbatim era data */
{VKM}
{PBMT}

extern "C" {{
{chr(10).join(chunk_block)}
}}

{chr(10).join(plain_block)}
"""
with open(OUT, "w", encoding="utf-8") as f:
    f.write(hdr)

if MANIFEST:
    h = hashlib.sha256
    era_shas = {p: h(open(p, "rb").read()).hexdigest() for p in (CIS, CMS, KBS, MTH)}
    with open(MANIFEST, "w", encoding="utf-8") as f:
        f.write("# P2-RDPREP input slice manifest — pin 6b38b53837 (era shas + slice shas)\n")
        for p, s in era_shas.items():
            rel = p[len(DECOMP) + 1:]
            f.write("ERA %s  %s\n" % (s, rel))
        for home, sig, body in S:
            f.write(f"SLICE {h(body.encode()).hexdigest()}  {home.split('/')[-1]}  {sig[:72]}\n")
mtag = MANIFEST if MANIFEST else "(no manifest)"
print("gen: %d verbatim slices -> %s (manifest %s)" % (len(S), OUT, mtag))
