#!/usr/bin/env python3
# AW P2: generate compat/repin shadow headers = verbatim decomp @6b38b5383
# copies + pin blocks. Deterministic; committed for reproducibility.
import os, re
D = "/Users/scott/Hermes/pso-decomp/pso-decomp"
R = "/Users/scott/Hermes/pso-metal/compat/repin"
GUARD = '''
/* ── compat/repin shadow (P2-AW, inert unless -Icompat/repin is earliest) ──
 * Era header copied VERBATIM from pso-decomp @6b38b5383; ONLY the pin block
 * below this shadow's tail marker adds content (field types/names/order
 * unchanged — the slot census proved this struct family carries ZERO
 * pointer fields, docs/p2-repin-saves-1.md §1, so unlike compat/njs there is
 * nothing to re-type; this shadow RESTORES byte-truth by pinning G's card
 * offsets as private compile-time asserts that the rebind must satisfy).
 * Requires the port types.h rebind (-DPSO_PORT_LAYOUT_WAIVER
 * -Ihost/bringup/include): under the era `typedef unsigned long u32` the
 * whole struct inflates on LP64 and these pins fail LOUD.  */
'''
PINS = {
"pso/item/TItemData.h": '''
/* G card (docs/p1-format-saves.md §2/§3.1): TItemData = mixed-width-union POD,
 * FORMAT-KEEP, inside BankItemRecord[200] + TPlyInventoryItem[30] (0x14 each).
 * Pointer-bearing fields: NONE (slot census §1). data1 lanes are BE wire —
 * the swap obligation lives in the loader (G §5.4 per-32-bit-word), not here. */
typedef char pso_repin_itemdata_gate[(sizeof(u32) == 4 && sizeof(uaddr) == 8) ? 1 : -1];
typedef char pso_repin_TItemData_size[      (sizeof(TItemData) == 0x14) ? 1 : -1];
typedef char pso_repin_TItemData_data1[     (offsetof(TItemData, data1) == 0x00) ? 1 : -1];
typedef char pso_repin_TItemData_id[        (offsetof(TItemData, id) == 0x0C) ? 1 : -1];
typedef char pso_repin_TItemData_data2[     (offsetof(TItemData, data2) == 0x10) ? 1 : -1];
typedef char pso_repin_TItemData_data1_w[   (sizeof(TItemData::data1) == 0x0C) ? 1 : -1];
typedef char pso_repin_TItemData_data2_w[   (sizeof(TItemData::data2) == 0x04) ? 1 : -1];
''',
"pso/quest/challenge/ChallengeRecord.h": '''
/* G card (docs/p1-format-saves.md §2): ChallengeRecord 0x100 lives at
 * CharacterSaveRecord +0x21C4 (saved; bswap walk MISSING per G §5.2). Verdict
 * contradiction AC-LOCKED vs base UNREVIEWED is PENDING AX — pins below are
 * the LOCKED-side byte-truth; silencing them for an RW verdict is the
 * -DPSO_REPIN_RW_CHALLENGERECORD opt-out (do NOT set it until AX rules).
 * Pointer-bearing fields: NONE (slot census §1; tombstone Vec = 3×f32 POD). */
typedef char pso_repin_challengerecord_gate[(sizeof(u32) == 4 && sizeof(uaddr) == 8) ? 1 : -1];
#ifndef PSO_REPIN_RW_CHALLENGERECORD
typedef char pso_repin_ChallengeRecord_size[          (sizeof(ChallengeRecord) == 0x100) ? 1 : -1];
typedef char pso_repin_ChallengeRecord_title_color[   (offsetof(ChallengeRecord, title_color) == 0x00) ? 1 : -1];
typedef char pso_repin_ChallengeRecord_stage_times[   (offsetof(ChallengeRecord, stage_times) == 0x04) ? 1 : -1];
typedef char pso_repin_ChallengeRecord_tombstone[     (offsetof(ChallengeRecord, tombstone) == 0x60) ? 1 : -1];
typedef char pso_repin_ChallengeRecord_spot[          (offsetof(ChallengeRecord, tombstone_spot) == 0xB0) ? 1 : -1];
typedef char pso_repin_ChallengeRecord_ranks[         (offsetof(ChallengeRecord, ranks) == 0xC0) ? 1 : -1];
typedef char pso_repin_ChallengeRecord_rank_title[    (offsetof(ChallengeRecord, rank_title) == 0xD8) ? 1 : -1];
typedef char pso_repin_ChallengeRecord_ep2_split_rank[(offsetof(ChallengeRecord, ep2_split_rank) == 0xE4) ? 1 : -1];
typedef char pso_repin_ChallengeRecord_ep2_split_times[(offsetof(ChallengeRecord, ep2_split_stage_times) == 0xEC) ? 1 : -1];
typedef char pso_repin_ChallengeStageRank_size[       (sizeof(ChallengeStageRank) == 0x08) ? 1 : -1];
typedef char pso_repin_TombstoneRecord_size[          (sizeof(TObjTombstoneRecord) == 0x50) ? 1 : -1];
typedef char pso_repin_TombstoneRecord_date[          (offsetof(TObjTombstoneRecord, date) == 0x08) ? 1 : -1];
typedef char pso_repin_TombstoneRecord_position[      (offsetof(TObjTombstoneRecord, position) == 0x10) ? 1 : -1];
typedef char pso_repin_TombstoneRecord_display_name[  (offsetof(TObjTombstoneRecord, display_name) == 0x1C) ? 1 : -1];
typedef char pso_repin_TombstoneRecord_unit_name[     (offsetof(TObjTombstoneRecord, unit_name) == 0x30) ? 1 : -1];
typedef char pso_repin_TombstoneSpot_size[            (sizeof(TObjTombstoneSpot) == 0x10) ? 1 : -1];
#endif
''',
"pso/ui/frontend/TAdSinglePlyChrSelectGC.h": '''
/* G card (docs/p1-format-saves.md §2): SystemSaveData = the ENTIRE PSO_SYSTEM
 * card payload after CardSaveHeader, 0x11C, no cipher, checksum-only.
 * Pointer-bearing fields: NONE (slot census §1). CharacterBufferTable ptrs
 * are heap-buffer indirection (runtime state, AR GinjaSceneState precedent) —
 * NOT slot-converted: the saved bytes are what those ptrs point AT. */
typedef char pso_repin_systemsavedata_gate[(sizeof(u32) == 4 && sizeof(uaddr) == 8) ? 1 : -1];
typedef char pso_repin_SystemSaveData_size[          (sizeof(SystemSaveData) == 0x11C) ? 1 : -1];
typedef char pso_repin_SystemSaveData_checksum[      (offsetof(SystemSaveData, checksum) == 0x000) ? 1 : -1];
typedef char pso_repin_SystemSaveData_music[         (offsetof(SystemSaveData, music_stream_gain) == 0x004) ? 1 : -1];
typedef char pso_repin_SystemSaveData_cue[           (offsetof(SystemSaveData, cue_stream_gain) == 0x006) ? 1 : -1];
typedef char pso_repin_SystemSaveData_lang[          (offsetof(SystemSaveData, language) == 0x007) ? 1 : -1];
typedef char pso_repin_SystemSaveData_clock[         (offsetof(SystemSaveData, clock_offset_frames) == 0x008) ? 1 : -1];
typedef char pso_repin_SystemSaveData_udp[           (offsetof(SystemSaveData, udp_option) == 0x00C) ? 1 : -1];
typedef char pso_repin_SystemSaveData_field_e[       (offsetof(SystemSaveData, field_e) == 0x00E) ? 1 : -1];
typedef char pso_repin_SystemSaveData_data10[        (offsetof(SystemSaveData, data_10) == 0x010) ? 1 : -1];
typedef char pso_repin_SystemSaveData_data10_w[      (sizeof(SystemSaveData::data_10) == 0x100) ? 1 : -1];
typedef char pso_repin_SystemSaveData_serial[        (offsetof(SystemSaveData, serial_number) == 0x110) ? 1 : -1];
typedef char pso_repin_SystemSaveData_seed[          (offsetof(SystemSaveData, serial_seed) == 0x118) ? 1 : -1];
''',
}
for rel, pins in PINS.items():
    src = open(os.path.join(D, "include", rel)).read()
    assert src.count("#endif") >= 1, rel
    # insert pins before the final #endif (closing the include guard)
    i = src.rstrip().rfind("#endif")
    tail = "/* ── end verbatim era copy ── */\n" + pins + "\n"
    out = src[:i] + GUARD + tail + src[i:]
    os.makedirs(os.path.join(R, os.path.dirname(rel)), exist_ok=True)
    open(os.path.join(R, rel), "w").write(out)
    print("wrote", rel, len(out), "bytes")
# handles lineage (provided, unused by trio — zero-slot census)
open(os.path.join(R, "repin_handles.h"), "w").write('''/* compat/repin/repin_handles.h — handle-slot lineage for FUTURE save-adjacent
 * shadows (e.g. AR §4.3 FloorPopulation gap). The P2-AW trio (TItemData,
 * SystemSaveData, ChallengeRecord) consumes NONE of it: slot census
 * docs/p2-repin-saves-1.md §1 proves zero pointer-bearing fields, so the trio
 * re-pin is width-only (port types.h rebind) — the POF0 analogue: the copy
 * path through the card boundary stays unchanged, 4-byte words all the way.
 * Reuse the pso_g2h lineage VERBATIM (AR compat/njs + bringup crt0): a slot
 * is a guest address; h2g translates, g2h stamps. slot==0 <-> NULL.
 * Requires pso_arena_base (loader lane, same as AR). */
#ifndef PSO_REPIN_HANDLES_H
#define PSO_REPIN_HANDLES_H
#include "types.h"
#ifndef PSO_NJS_ARENA_GUEST_BASE
#define PSO_NJS_ARENA_GUEST_BASE 0x80000000u
#endif
static inline void *pso_repin_g2h(u32 slot, unsigned char *arena_base) {
    return slot ? arena_base + (slot - PSO_NJS_ARENA_GUEST_BASE) : (void *)0;
}
static inline u32 pso_repin_h2g(const void *p, unsigned char *arena_base) {
    return p ? PSO_NJS_ARENA_GUEST_BASE + (u32)((const unsigned char *)p - arena_base) : 0u;
}
#endif
''')
print("done")
