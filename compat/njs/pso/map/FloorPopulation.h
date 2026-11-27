/* P2-NC NJS handle re-pin — 3rd SHADOW header (compat/njs/pso/map/).
 * Inert until the integrator lands -Icompat/njs on listed C++ TUs
 * (docs/p2-njs-wave-1.md; AV docs/p2-gx-tex-consumer.md §4 machinery).
 *
 * Mechanism is identical to the AR shadow pair
 * (compat/njs/pso/system/NinjaTypes.h + pso/render/GinjaMaterialState.h,
 * docs/p2-njs-repin-1.md §2): earliest -I wins for quoted
 * `#include "pso/map/FloorPopulation.h"`, same include guard as the era
 * header, era text copied VERBATIM except where marked "P2-NC" below.
 * This closes the AR §4.3 named gap ("Floor-set rows ... 3rd shadow, same
 * mechanic"; AV §5 V7).
 *
 * What changes: the six pointer slots inside the era-PINNED row structs
 * become u32 handle slots (p1-datamodel §3 rule 1 lineage):
 *   FloorObjectSetRow.object  (era @0x40)   EnemySetRecord.enemy (era @0x44)
 *   LobbyFloorRows.object_rows (era @0x10)  LobbyFloorRows.enemy_rows (era @0x14)
 *   LobbyPoolEntry.objects     (era @0x00)  LobbyPoolEntry.enemies   (era @0x04)
 * Field NAMES, order, and every byte offset/size stay era-true, so the era
 * LAYOUT_ASSERTs below (FloorObjectSetRow_size 0x44, EnemySetRecordSize 0x48,
 * LobbyFloorRows_size 0x18 + row offsets, LobbyPoolEntry_size 0x8,
 * FloorObjectSetRowObjectOffset 0x40, EnemySetRecordEnemy 0x44) PASS
 * HONESTLY under the port types.h rebind instead of going negative — the
 * measured H3 residual this closes is exactly those three size/offset
 * negative-array asserts (docs/p2-collision-debt.md §4.6: 18 row-assert TUs;
 * NC H3 sweep: 21 citing, 18 pure-class — receipts in docs/p2-njs-wave-1.md).
 *
 * Era byte truth (comments only, ZERO asset bytes): floor set rows are
 * copied byte-true from ad.dat/d.dat archives into the ARAM pool + row heap
 * (floor_set_archive_pool / floor_saved_row_heap), so the four-byte slots
 * hold PPC image-window addresses exactly like read_ninja_chunk's POF0
 * slots. Convert at access with NJS_HANDLE(T, slot) / pso_njs_g2h /
 * pso_njs_h2g (declared by the NinjaTypes shadow, included below); the
 * runtime window base for ARAM-resident rows is the loader lane's
 * enablement decision (AR §6: this header changes *typing*, not behavior).
 * Never re-spell a slot as a host pointer in the struct.
 *
 * C++ TUs ONLY — same law as the pair: -Icompat/njs never lands on a .c TU
 * (AV §4 / AR §5: C launders ptr<->u32 silently under -w). */
#ifndef PSO_MAP_FLOORPOPULATION_H
#define PSO_MAP_FLOORPOPULATION_H

#include "types.h"
#include "pso/system/NinjaTypes.h" // P2-NC: conversion helpers + PSO_NJS_BYTE_TRUTH (guard-idempotent)
#include "pso/enemy/EnemyPlacement.h"
#include "pso/object/base/ObjectSetData.h"
#include "pso/system/AramPool.h"
#include <stddef.h>

struct TObjEnemy;
class TObject;
class TObjBase;
class TSetManager;
struct ObjectFactoryEntry;
struct EnemyFactoryEntry;
class THeap;

// Live object and enemy rows in the floor set buffers.
struct FloorObjectSetRow {
    s16 type;
    s16 flags;
    ObjectSetData placement;
    u32 object; // P2-NC: was TObject * — floor-set row image slot (bytes @0x40)
};
PSO_NJS_BYTE_TRUTH(FloorObjectSetRow_object_slot4, sizeof(FloorObjectSetRow::object) == 4);

struct EnemySetRecord {
    s16 type;
    s16 flags;
    EnemyPlacement placement;
    u32 enemy; // P2-NC: was TObjEnemy * — floor-set row image slot (bytes @0x44)
};
PSO_NJS_BYTE_TRUTH(EnemySetRecord_enemy_slot4, sizeof(EnemySetRecord::enemy) == 4);

// Flags and identifiers retained for a floor after its live rows are released.
struct FloorSavedSetRow {
    u16 id;
    s16 flags;
    s16 room;
    s16 wave;
};

struct LobbyFloorRows {
    s32 object_count;
    s32 enemy_count;
    u32 object_placement;
    u32 enemy_placement;
    u32 object_rows; // P2-NC: was FloorSavedSetRow * — heap window slot (bytes @0x10)
    u32 enemy_rows;  // P2-NC: was FloorSavedSetRow * — heap window slot (bytes @0x14)
};
PSO_NJS_BYTE_TRUTH(LobbyFloorRows_rows_slot4, sizeof(LobbyFloorRows::object_rows) == 4);

// Raw object and enemy set archive blocks for each floor.
struct LobbyPoolEntry {
    u32 objects; // P2-NC: was AramBlock * — ARAM window slot (bytes @0x00)
    u32 enemies; // P2-NC: was AramBlock * — ARAM window slot (bytes @0x04)
};
/* Era access idioms this replaces, per TU: row->object ->
 * NJS_HANDLE(TObject, row->object); entry.objects ->
 * NJS_HANDLE(AramBlock, entry.objects); comparisons of slots against window
 * bounds happen in SLOT space (no convert), per AR §4.1 / TChCrLoadReq
 * precedent. Slots written from host pointers go through pso_njs_h2g. */

LAYOUT_ASSERT(FloorObjectSetRow_size, sizeof(FloorObjectSetRow) == 0x44);
LAYOUT_ASSERT(FloorObjectSetRowPlacementOffset, offsetof(FloorObjectSetRow, placement) == 4);
LAYOUT_ASSERT(FloorObjectSetRowPositionOffset, offsetof(FloorObjectSetRow, placement) + offsetof(ObjectSetData, position) == 0x10);
LAYOUT_ASSERT(FloorObjectSetRowObjectOffset, offsetof(FloorObjectSetRow, object) == 0x40);
LAYOUT_ASSERT(EnemySetRecordSize, sizeof(EnemySetRecord) == 0x48);
LAYOUT_ASSERT(EnemySetRecordFlags, offsetof(EnemySetRecord, flags) == 2);
LAYOUT_ASSERT(EnemySetRecordPlacement, offsetof(EnemySetRecord, placement) == 4);
LAYOUT_ASSERT(EnemySetRecordId, offsetof(EnemySetRecord, placement) + offsetof(EnemyPlacement, id) == 0xA);
LAYOUT_ASSERT(EnemySetRecordIndex, offsetof(EnemySetRecord, placement) + offsetof(EnemyPlacement, enemy_index) == 4);
LAYOUT_ASSERT(EnemySetRecordRoom, offsetof(EnemySetRecord, placement) + offsetof(EnemyPlacement, room) == 0xC);
LAYOUT_ASSERT(EnemySetRecordWave, offsetof(EnemySetRecord, placement) + offsetof(EnemyPlacement, wave) == 0xE);
LAYOUT_ASSERT(EnemySetRecordEnemy, offsetof(EnemySetRecord,enemy)==0x44);
LAYOUT_ASSERT(FloorSavedSetRow_size, sizeof(FloorSavedSetRow) == 8);
LAYOUT_ASSERT(LobbyFloorRows_size, sizeof(LobbyFloorRows) == 0x18);
LAYOUT_ASSERT(LobbyFloorRows_object_rows_offset, offsetof(LobbyFloorRows, object_rows) == 0x10);
LAYOUT_ASSERT(LobbyFloorRows_enemy_rows_offset, offsetof(LobbyFloorRows, enemy_rows) == 0x14);
LAYOUT_ASSERT(LobbyPoolEntry_size, sizeof(LobbyPoolEntry) == 8);

// Set rows of the floor being played and their counts.
extern FloorObjectSetRow *current_floor_object_set_rows;
extern s32 current_floor_object_set_count;
extern EnemySetRecord *current_floor_enemy_set_rows;
extern s32 current_floor_enemy_set_count;
// Decoration rows of the floor's map set (the ad.dat / d.dat file).
extern FloorObjectSetRow *floor_decoration_rows;
extern s32 floor_decoration_row_count;
// Floor (and its area, -1 without a map selection) whose set rows were last set up.
extern s32 active_set_floor;
extern s32 active_set_floor_area;
// Bit 0: the floor sets are loaded.
extern s16 floor_set_flags;
// Every floor's set rows, kept in the ARAM pool, and the flags saved in the row heap while a floor is left.
extern AramPool *floor_set_archive_pool;
extern THeap *floor_saved_row_heap;
extern LobbyPoolEntry floor_set_archives[18];
extern LobbyFloorRows floor_enemy_group_table[18];
// Object and enemy set row counts of each floor, -1 when its set file failed to load.
extern s32 floor_object_set_counts[18];
extern s32 floor_enemy_set_counts[18];
extern FloorObjectSetRow *object_set_row_buffer;
extern EnemySetRecord *enemy_set_row_buffer;

// Spawns the hidden enemies of one room wave on the current floor, or flags them on another floor.
s32 unhide_enemies_at(s32 floor, s16 room, s16 wave);

// The original callers pass the floor; these return the currently loaded rows.
FloorObjectSetRow *get_active_floor_object_rows(s32 floor);
EnemySetRecord *get_active_floor_enemy_rows(s32 floor);
TObjBase *call_area_set_factory(TSetManager *manager, FloorObjectSetRow *row);
s32 init_quest_floor_enemies(s32 floor, EnemySetRecord *source, u32 size);
// Looks the row's type up in the current area's object factory table: copies the entry to out and returns its
// index, or -1.
s32 find_set_factory_index(FloorObjectSetRow *row, ObjectFactoryEntry *out);
// Builds the object of a set row through its factory; NULL for an empty row or an unknown type.
TObjBase *create_set_object_from_row(FloorObjectSetRow *row);
void copy_object_factory_entry(ObjectFactoryEntry *out, ObjectFactoryEntry *entry);
void copy_enemy_factory_entry(EnemyFactoryEntry *out, EnemyFactoryEntry *entry);
TObjEnemy *construct_enemy_from_set_row(EnemySetRecord *row);
// Number of child enemies the area's factory entry for that enemy type spawns with it.
s32 get_enemy_factory_child_count(s32 area, s16 *enemy_type);

// Followed by the object flag stream and then the enemy flag stream in game-state packet 6x6E.
struct FloorSetFlagsHeader {
    u32 object_offset;
    s32 object_count;
    u32 enemy_offset;
    s32 enemy_count;
};

// The decompressed sync scratch has 0x4000 payload bytes after its length words.
struct FloorSetFlagsPayload : FloorSetFlagsHeader {
    s16 flags[(0x4000 - sizeof(FloorSetFlagsHeader)) / sizeof(s16)];
};

LAYOUT_ASSERT(FloorSetFlagsPayloadSize, sizeof(FloorSetFlagsPayload) == 0x4000);
LAYOUT_ASSERT(FloorSetFlagsHeaderSize, sizeof(FloorSetFlagsHeader) == 0x10);
LAYOUT_ASSERT(FloorSetFlagsEnemyOffset, offsetof(FloorSetFlagsHeader, enemy_offset) == 8);

void swap_floor_set_flags_header(FloorSetFlagsHeader *header);
void unpack_floor_set_flags(FloorSetFlagsHeader *buffer);
void apply_floor_set_flags_packet(FloorSetFlagsHeader *buffer, u16 length);
int pack_floor_set_flags(FloorSetFlagsPayload *buffer);
int serialize_area_set_flags(FloorSetFlagsPayload *buffer);
int get_floor_set_flags_size(void);

void init_scene_objects();
void destroy_current_floor_objects(void);
void destroy_floor_decoration_objects(void);
extern "C" {
void mark_floor_enemies_dead(void);
}
void send_6x76_set_entity_set_flags(TObjBase *object, s16 flags);
void set_wave_object_flags(s32 pl, s16 b, s16 a, s32 mask, s32 set);
s32 is_room_free_of_active_enemies(s32 floor, s16 room);
// 1 when every enemy set row of the room on that floor has flag 8 (the quest zone-clear check).
s32 is_room_enemy_set_cleared(s32 floor, s16 room);
// Creates the hidden objects of one room group on the current floor, or flags them on another floor.
s32 unhide_room_set_objects(s32 floor, s16 room, s16 group);
s32 spawn_objects_at_pos(s16 room, s16 wave);
void assign_display_objects(EnemySetRecord *rows, s32 enemy_count);
void assign_objects_to_slots(u8 *rows, s32 object_count);
void create_objects(s32 floor);
void attach_objects_by_id(s32 floor);
EnemySetRecord *get_loaded_enemy_set_rows();
FloorObjectSetRow *get_loaded_object_set_rows();
#endif
