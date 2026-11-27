#ifndef PSO_QUEST_CHALLENGE_CHALLENGERECORD_H
#define PSO_QUEST_CHALLENGE_CHALLENGERECORD_H

#include "types.h"
#include <stddef.h>
#include "pso/object/TObjTombstoneRecord.h"

// One record set's award: the award bits and the encoded grade (0 none, 1 B, 2 A, 3 S).
struct ChallengeStageRank {
    u32 award_flags;
    s32 rank;
    void bswap();
};

// The record sets of a ChallengeRecord. resolve_challenge_record_set maps RECORD_SET_CURRENT to the
// running game's set by selected_episode and is_split_screen; the sets have 9/5/9/5 stages.
enum ChallengeRecordSet {
    RECORD_SET_CURRENT = -1,
    RECORD_SET_EP1,
    RECORD_SET_EP2,
    RECORD_SET_EP1_SPLIT,
    RECORD_SET_EP2_SPLIT
};

// Swaps one stage time of a ChallengeRecord.
void bswap_packet_field(s32 *value);

// A character's challenge-mode records, kept in the save record at 0x21C4 and per client in
// ChallengeModeState::client_records. The record sets follow TWindowSubStatusDisp::RecordSet.
struct ChallengeRecord {
    u16 title_color;
    s16 pad_2;
    s32 stage_times[9 + 5 + 9];          // EP1, EP2 and EP1 split-screen stages in turn, obfuscated
    TObjTombstoneRecord tombstone;       // where the character last died
    TObjTombstoneSpot tombstone_spot;    // the split-screen challenge's grave
    ChallengeStageRank ranks[3];         // EP1, EP2 and EP1 split-screen
    char rank_title[0xC];                // obfuscated
    ChallengeStageRank ep2_split_rank;
    s32 ep2_split_stage_times[5];
    void bswap();
};

LAYOUT_ASSERT(ChallengeRecord_size, sizeof(ChallengeRecord) == 0x100);
LAYOUT_ASSERT(ChallengeRecord_tombstone, offsetof(ChallengeRecord, tombstone) == 0x60);
LAYOUT_ASSERT(ChallengeRecord_tombstone_spot, offsetof(ChallengeRecord, tombstone_spot) == 0xB0);
LAYOUT_ASSERT(ChallengeRecord_ranks, offsetof(ChallengeRecord, ranks) == 0xC0);
LAYOUT_ASSERT(ChallengeRecord_rank_title, offsetof(ChallengeRecord, rank_title) == 0xD8);
LAYOUT_ASSERT(ChallengeRecord_ep2_split_rank, offsetof(ChallengeRecord, ep2_split_rank) == 0xE4);
LAYOUT_ASSERT(ChallengeRecord_ep2_split_stage_times, offsetof(ChallengeRecord, ep2_split_stage_times) == 0xEC);

// set: a ChallengeRecordSet.
s32 resolve_challenge_record_set(s32 set);
s32 challenge_set_stage_count(s32 set);
// The valid range of a set's total time: 300 seconds to 9:59:59 per stage.
s32 challenge_set_min_time(s32 set);
s32 challenge_set_max_time(s32 set);
s32 *challenge_set_stage_times(ChallengeRecord *record, s32 set);
ChallengeStageRank *challenge_set_rank(ChallengeRecord *record, s32 set);


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
/* ── end verbatim era copy ── */

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

#endif
