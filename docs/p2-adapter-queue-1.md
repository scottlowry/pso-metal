# P2-AQP — NJS adapter-queue batch-1: top-30 boot-proximal paste-ready adapter edits (INERT SUPPLY)

Fleet wave-3 lane AQP (zero-build), card `t_8dcd7f38`, 2026-11-27. Decomp
`pso-decomp @6b38b5383` READ-ONLY — NOTHING in this doc is landed into the decomp
tree; the diff blocks are the supply the SLICE GENERATOR applies at slice time
(same inert pattern as NA's §6 block / AV §4 request: submit, never lane-apply).
Zero repo builds; every number below is a same-turn python count over fresh
scratch `clang -fsyntax-only` probes (`cache/scratch/aqp/`). Inputs: NA
`docs/p2-njs-adopt-1.md` §3 (the 169-row ADAPTER-QUEUE), AV `docs/p2-gx-tex-consumer.md`
§3 (fix-shape taxonomy), AR `docs/p2-njs-repin-1.md` §4, RS `docs/p2-rung5-seams-1.md`.

## 0. Era + probe shape (apply-7)

- Waiver header `build/waivers/pso_assert_waivers.h` sha256 = `d4382fcd0659357f…`
  measured THIS turn, equal to the D8 pin `host/bringup/waiver-header.sha256`
  (`d4382fcd0659357f283e675726c1076338e9095…`): apply-7 era certified, zero regen.
- Probe = official apply-7 LP64 PARSE_RECIPE (compat trio `-include` in order:
  compat, waivers, collision pin; `-DPSO_PORT_LAYOUT_WAIVER`; `-I` net, repin, facade,
  dialect/pshim, host/bringup/include — facade before bringup per collision law;
  `-target arm64-apple-macos12.0 -ferror-limit=0`) **+ `-Icompat/njs` (C++ TUs only,**
  never `.c`, AR §5). Runner `aqp_probe.sh` = fresh file, `-P6` equivalent (6-thread
  argv-safe pool). BEFORE = decomp originals at NJS shape; AFTER = identical files
  with this doc's diff applied in a scratch mirror (`$TMP/aqp/after/`).
- Shape fidelity: BEFORE (no-adapter) NJS-shape error totals equal NA §3's `total`
  column on 28/30 rows; the 2 that differ are strictly LOWER under apply-7
  (`TLobby_80209524.cpp` 7->4, `ChunkModelDraw.cpp` 44->22 — facade/repin legs cured
  other-class residuals since NA's apply-6 sweep). Every enumerated dialect site still
  fires in BEFORE 30/30. Queue counts re-read from NA §3 by python: 169 rows, 673 sites.

## 1. Selection: 169 -> boot-proximal subset -> top 30 (python-rule, reproducible)

RS §4 names the boot wall behind ladder7: `NJS_OBJECT_size` first-blocks ALL 8
remaining lane-J homes (`TCameraCtrl`, `ControllerInput` ×2 legs, `TObject` render/
overlay, `TLobby_803382E4`, `SoundRequests`, `TLobby_800F8AA8`, `TLobby_801E2F94` +
the `TKeyBoardControl_80290CB0`/`TParticle` sub-legs). None of those 8 is itself a
queue row — they are adopted/gated — so the NEXT boot blocker behind ladder7 is the
adapter queue on the TUs the ladder7 legs CALL THROUGH: the NJS texture/consumer
chain (AV §1.3/§3), floor-population loaders (AR §4.3), and boot-named UI/input
files. Ranking rule (`aqp_select2.py`, deterministic):

- **tier 0** = file stem cited verbatim in `docs/boot-spine.md`, AV
  `docs/p2-gx-tex-consumer.md`, or RS `docs/p2-rung5-seams-1.md` (boot/chain evidence);
- **tier 1** `src/pso/render/` (Ginja/chunk draw = init_texture_tables S8, render_tick);
- **tier 2** `src/pso/unk/` (texture-walk + lobby-chain bodies on the same calls);
- **tier 3** `system|map|ui/**`; 4 `player/`; 5 `object/`; 6 `item|effect|technique|
  weapon|enemy|boss|…` (in-game depth). Within tier: site count desc, then path.

Queue = 169 rows / 673 sites; all 15 tier-0
files fit inside the top 30 (in-queue tier-0 = 15 files / 94 sites). Top 30 =
**164 sites, 165 adapter edits** (multi-edit lines counted once per edit).

| # | tier | file | sites n | NA dialect/total | boot evidence |
|---|---|---|---|---|---|
| 1 | 0 | src/pso/map/TSetManager.cpp | 48 | 48/50 | evidence:gx-tex |
| 2 | 0 | src/pso/unk/text_803DD5E0.cpp | 11 | 11/12 | evidence:gx-tex |
| 3 | 0 | src/pso/item/TItem.cpp | 5 | 5/8 | evidence:rung5 |
| 4 | 0 | src/pso/ui/charcreate/TChCrLoadReq.cpp | 5 | 8/8 | evidence:gx-tex |
| 5 | 0 | src/pso/unk/TLobby_80209524.cpp | 4 | 4/7 | evidence:gx-tex |
| 6 | 0 | src/pso/boss/derolle/TObjBoss2Tunnel.cpp | 3 | 3/10 | evidence:boot-spine |
| 7 | 0 | src/pso/render/GinjaMaterial.cpp | 3 | 3/3 | evidence:gx-tex |
| 8 | 0 | src/pso/render/psoLoadTexture.cpp | 3 | 3/3 | evidence:gx-tex |
| 9 | 0 | src/pso/unk/text_803DA798.cpp | 3 | 3/3 | evidence:gx-tex |
| 10 | 0 | src/pso/object/TObjSinBoard.cpp | 2 | 2/2 | evidence:gx-tex+rung5 |
| 11 | 0 | src/pso/render/GinjaTextureTable.cpp | 2 | 2/2 | evidence:gx-tex |
| 12 | 0 | src/pso/ui/charcreate/TCharaMaking.cpp | 2 | 2/2 | evidence:boot-spine |
| 13 | 0 | src/pso/player/TObjPlayer_8023E480.cpp | 1 | 1/1 | evidence:boot-spine |
| 14 | 0 | src/pso/render/GinjaTextureHeader.cpp | 1 | 1/1 | evidence:gx-tex |
| 15 | 0 | src/pso/ui/font/TNECFontEORFunction.cpp | 1 | 1/2 | evidence:boot-spine |
| 16 | 1 | src/pso/render/CameraCapture.cpp | 15 | 15/74 | render-layer |
| 17 | 1 | src/pso/render/ShapeMotionDraw.cpp | 9 | 9/11 | render-layer |
| 18 | 1 | src/pso/render/ChunkTextureStripCollector.cpp | 8 | 8/8 | render-layer |
| 19 | 1 | src/pso/render/ModelDrawContext.cpp | 8 | 8/11 | render-layer |
| 20 | 1 | src/pso/render/ChunkModelDraw.cpp | 5 | 5/44 | render-layer |
| 21 | 1 | src/pso/render/ChunkMotionDraw.cpp | 4 | 4/6 | render-layer |
| 22 | 1 | src/pso/render/GinjaMotionDraw.cpp | 4 | 4/6 | render-layer |
| 23 | 1 | src/pso/render/AltModel.cpp | 3 | 3/3 | render-layer |
| 24 | 1 | src/pso/render/TNodeMatrixRecorder.cpp | 2 | 2/3 | render-layer |
| 25 | 1 | src/pso/render/MericarolTexture.cpp | 1 | 1/1 | render-layer |
| 26 | 1 | src/pso/render/NinjaUtil.cpp | 1 | 1/5 | render-layer |
| 27 | 1 | src/pso/render/reflection.cpp | 1 | 1/1 | render-layer |
| 28 | 2 | src/pso/unk/text_8028A998.cpp | 4 | 6/6 | unk-chain |
| 29 | 2 | src/pso/unk/text_803DA16C.cpp | 3 | 3/3 | unk-chain |
| 30 | 2 | src/pso/unk/text_80327844.cpp | 2 | 2/3 | unk-chain |

## 2. Fix shapes (AV §3 taxonomy, helpers per `compat/njs/pso/system/NinjaTypes.h:43-53`)

- **g2h-load** — read a u32 slot into host code: `pso_njs_g2h(slot)` / `NJS_HANDLE(T, slot)`.
- **h2g-store** — write a host pointer into a u32 slot: `slot = pso_njs_h2g(p)`.
- **NJS_HANDLE-slot** — cross the seam once, keep the era type: `NJS_HANDLE(T, x->field)`.
- **slot-space-compare** — window/bounds/NULL tests on the slot, no conversion (`!= 0`,
  u32 bound literals; AR §4.1, AV §3 #6).
- **slot-subscript** — slot base + era index: `NJS_HANDLE(T, tl->textures)[i].field`
  (single or doubled over `NJS_TEXNAME.texture`/`.filename`).

Slot==0 ↔ NULL preserved by `pso_njs_g2h`. The 30 files parse under `-Icompat/njs`
ONLY once added to `build/njs_allowlist.txt` (integrator-owned; §6).

## 3. Per-file (site | fix-shape | probe-status) + paste-ready unified diff

Probe-status = python-counted `-fsyntax-only` error totals at NJS shape, BEFORE
(decomp original) -> AFTER (original + this file's diff). `CLEAN` = 0 errors AFTER.
NO-NJS-RESIDUAL = zero pointer↔slot-class errors remain after the adapter (residue
names other-class legs: no-matching-function cascades on un-enumerated call args,
assert tails, and the static-initializer cluster — §5).

### 3.1 src/pso/map/TSetManager.cpp

sites: 80,109,157,160,171,173,178,182,209,212,223,225,230,234,246,247,248,249,280,281,282,283,316,337,433,461,552,570,622,670,711,756,794,858,889,960,990,1165,1384,1390,1396,1402,1413,1423,1490,1492,1507,1509 · edits: 48 · shapes: g2h-load+h2g-store+NJS_HANDLE-slot+slot-subscript
· probe: **50 -> 2** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/map/TSetManager.cpp
+++ b/src/pso/map/TSetManager.cpp
@@ -77,7 +77,7 @@
     out->object_offset = sizeof(FloorSetFlagsHeader);
     for (s32 i = 0; i < 18; i++) {
         if (floor_enemy_group_table[i].object_count > 0) {
-            FloorSavedSetRow *e = floor_enemy_group_table[i].object_rows;
+            FloorSavedSetRow *e = NJS_HANDLE(FloorSavedSetRow, floor_enemy_group_table[i].object_rows);
             s32 n = floor_enemy_group_table[i].object_count;
             if (get_current_floor() == i) {
                 FloorObjectSetRow *s = current_floor_object_set_rows;
@@ -106,7 +106,7 @@
     out->enemy_offset = out->object_count * 2 + 0x10;
     for (s32 i = 0; i < 18; i++) {
         if (floor_enemy_group_table[i].enemy_count > 0) {
-            FloorSavedSetRow *e = floor_enemy_group_table[i].enemy_rows;
+            FloorSavedSetRow *e = NJS_HANDLE(FloorSavedSetRow, floor_enemy_group_table[i].enemy_rows);
             s32 n = floor_enemy_group_table[i].enemy_count;
             if (get_current_floor() == i) {
                 EnemySetRecord *s = current_floor_enemy_set_rows;
@@ -154,10 +154,10 @@
         }
         s++;
     }
-    t->enemy_rows = (FloorSavedSetRow *)floor_saved_row_heap->heap_alloc(t->enemy_count * 8);
+    t->enemy_rows = pso_njs_h2g(floor_saved_row_heap->heap_alloc(t->enemy_count * 8));
     memset(t->enemy_rows, 0, t->enemy_count * 8);
     s = src;
-    d = t->enemy_rows;
+    d = NJS_HANDLE(FloorSavedSetRow, t->enemy_rows);
     for (i = 0; i < n; i++) {
         if (s->type != -1) {
             d->id = s->placement.id;
@@ -168,18 +168,18 @@
         s++;
     }
     if (floor_set_archives[idx].enemies != NULL) {
-        floor_set_archive_pool->free(floor_set_archives[idx].enemies);
-    }
-    floor_set_archives[idx].enemies = floor_set_archive_pool->alloc(sz);
+        floor_set_archive_pool->free(NJS_HANDLE(AramBlock, floor_set_archives[idx].enemies));
+    }
+    floor_set_archives[idx].enemies = pso_njs_h2g(floor_set_archive_pool->alloc(sz));
     if ((uaddr)src & 0x1F) {
         buf = TQuestVmDownLoadServer::alloc(sz);
         memcpy(buf, src, sz);
         if (floor_set_archives[idx].enemies != NULL) {
-            floor_set_archive_pool->transfer(buf, floor_set_archives[idx].enemies, sz, 0);
+            floor_set_archive_pool->transfer(buf, NJS_HANDLE(AramBlock, floor_set_archives[idx].enemies), sz, 0);
         }
         safe_free(buf);
     } else if (floor_set_archives[idx].enemies != NULL) {
-        floor_set_archive_pool->transfer(src, floor_set_archives[idx].enemies, sz, 0);
+        floor_set_archive_pool->transfer(src, NJS_HANDLE(AramBlock, floor_set_archives[idx].enemies), sz, 0);
     }
     return 1;
     }
@@ -206,10 +206,10 @@
         }
         s++;
     }
-    t->object_rows = (FloorSavedSetRow *)floor_saved_row_heap->heap_alloc(t->object_count * 8);
+    t->object_rows = pso_njs_h2g(floor_saved_row_heap->heap_alloc(t->object_count * 8));
     memset(t->object_rows, 0, t->object_count * 8);
     s = src;
-    d = t->object_rows;
+    d = NJS_HANDLE(FloorSavedSetRow, t->object_rows);
     for (i = 0; i < n; i++) {
         if (s->type != -1) {
             d->id = s->placement.entity_id;
@@ -220,18 +220,18 @@
         s++;
     }
     if (floor_set_archives[idx].objects != NULL) {
-        floor_set_archive_pool->free(floor_set_archives[idx].objects);
-    }
-    floor_set_archives[idx].objects = floor_set_archive_pool->alloc(sz);
+        floor_set_archive_pool->free(NJS_HANDLE(AramBlock, floor_set_archives[idx].objects));
+    }
+    floor_set_archives[idx].objects = pso_njs_h2g(floor_set_archive_pool->alloc(sz));
     if ((uaddr)src & 0x1F) {
         buf = TQuestVmDownLoadServer::alloc(sz);
         memcpy(buf, src, sz);
         if (floor_set_archives[idx].objects != NULL) {
-            floor_set_archive_pool->transfer(buf, floor_set_archives[idx].objects, sz, 0);
+            floor_set_archive_pool->transfer(buf, NJS_HANDLE(AramBlock, floor_set_archives[idx].objects), sz, 0);
         }
         safe_free(buf);
     } else if (floor_set_archives[idx].objects != NULL) {
-        floor_set_archive_pool->transfer(src, floor_set_archives[idx].objects, sz, 0);
+        floor_set_archive_pool->transfer(src, NJS_HANDLE(AramBlock, floor_set_archives[idx].objects), sz, 0);
     }
     return 1;
     }
@@ -243,10 +243,10 @@
     s32 i;
     for (i = 0; i < floor_decoration_row_count; i++, p++) {
         if (p->object != 0) {
-            TLobby::destroy_child_recursive(tl_02, p->object);
-            TLobby::destroy_child_recursive(tl_05, p->object);
-            TLobby::destroy_child_recursive(tl_06, p->object);
-            TLobby::destroy_child_recursive(tl_07, p->object);
+            TLobby::destroy_child_recursive(tl_02, NJS_HANDLE(TObject, p->object));
+            TLobby::destroy_child_recursive(tl_05, NJS_HANDLE(TObject, p->object));
+            TLobby::destroy_child_recursive(tl_06, NJS_HANDLE(TObject, p->object));
+            TLobby::destroy_child_recursive(tl_07, NJS_HANDLE(TObject, p->object));
             p->flags = 0;
             p->object = 0;
         }
@@ -277,10 +277,10 @@
     n = TLobby::get_obj_table_entry(get_current_floor());
     for (i = 0; i < n; i++, p++) {
         if (p->object != 0) {
-            TLobby::destroy_child_recursive(tl_02, p->object);
-            TLobby::destroy_child_recursive(tl_05, p->object);
-            TLobby::destroy_child_recursive(tl_06, p->object);
-            TLobby::destroy_child_recursive(tl_07, p->object);
+            TLobby::destroy_child_recursive(tl_02, NJS_HANDLE(TObject, p->object));
+            TLobby::destroy_child_recursive(tl_05, NJS_HANDLE(TObject, p->object));
+            TLobby::destroy_child_recursive(tl_06, NJS_HANDLE(TObject, p->object));
+            TLobby::destroy_child_recursive(tl_07, NJS_HANDLE(TObject, p->object));
             p->object = 0;
         }
     }
@@ -313,7 +313,7 @@
     if (id >= 0x1000 && id < 0x4000) {
         a = &floor_enemy_group_table[command->floor];
         if (a->enemy_rows != NULL) {
-            e = a->enemy_rows;
+            e = NJS_HANDLE(FloorSavedSetRow, a->enemy_rows);
             for (i = 0; i < a->enemy_count; i++, e++) {
                 if (e->id == id) {
                     e->flags |= command->flags;
@@ -334,7 +334,7 @@
     if (id >= 0x4000 && id < 0xFFFF) {
         a2 = &floor_enemy_group_table[command->floor];
         if (a2->object_rows != NULL) {
-            e2 = a2->object_rows;
+            e2 = NJS_HANDLE(FloorSavedSetRow, a2->object_rows);
             for (i = 0; i < a2->object_count; i++, e2++) {
                 if (e2->id == id) {
                     e2->flags |= command->flags;
@@ -430,7 +430,7 @@
     for (i = 0; i < 0x12; i++) {
         if (i != get_current_floor()) {
             LobbyFloorRows *t = &floor_enemy_group_table[i];
-            FloorSavedSetRow *b = t->enemy_rows;
+            FloorSavedSetRow *b = NJS_HANDLE(FloorSavedSetRow, t->enemy_rows);
             if (b != NULL) {
                 for (j = 0; j < t->enemy_count; j++) {
                     if (b->flags & 8) {
@@ -458,7 +458,7 @@
         return 1;
     } else {
         LobbyFloorRows *saved = &floor_enemy_group_table[floor];
-        FloorSavedSetRow *row = saved->enemy_rows;
+        FloorSavedSetRow *row = NJS_HANDLE(FloorSavedSetRow, saved->enemy_rows);
         if (row != NULL) {
             for (i = 0; i < saved->enemy_count; i++, row++) {
                 if (row->room == room && !(row->flags & 8)) {
@@ -549,7 +549,7 @@
     for (k = 0; k < 0x12; k++) {
         if (floor_enemy_group_table[k].enemy_rows != NULL) {
             for (j = 0; j < floor_enemy_group_table[k].enemy_count; j++) {
-                floor_enemy_group_table[k].enemy_rows[j].flags = 0;
+                NJS_HANDLE(FloorSavedSetRow, floor_enemy_group_table[k].enemy_rows)[j].flags = 0;
             }
         }
     }
@@ -567,7 +567,7 @@
     for (k = 0; k < 0x12; k++) {
         if (floor_enemy_group_table[k].object_rows != NULL) {
             for (j = 0; j < floor_enemy_group_table[k].object_count; j++) {
-                floor_enemy_group_table[k].object_rows[j].flags = 0;
+                NJS_HANDLE(FloorSavedSetRow, floor_enemy_group_table[k].object_rows)[j].flags = 0;
             }
         }
     }
@@ -619,7 +619,7 @@
         if (!(flags & 8) && !(flags & 2) && (row->placement.room == room) && (row->placement.wave == wave)) {
             enemy = construct_enemy_from_set_row(row);
             if (enemy != NULL) {
-                row->enemy = enemy;
+                row->enemy = pso_njs_h2g(enemy);
                 row->flags |= 2;
                 enemy->room = row->placement.room;
                 enemy->set_group = row->placement.wave;
@@ -667,7 +667,7 @@
         if (local != 0) {
             obj = construct_enemy_from_set_row(set);
             if (obj != NULL) {
-                set->enemy = obj;
+                set->enemy = pso_njs_h2g(obj);
                 set->flags |= 2;
                 obj->room = set->placement.room;
                 obj->set_group = set->placement.wave;
@@ -708,7 +708,7 @@
         if (!(flags & 8) && !(flags & 2) && (row->placement.room == room) && (row->placement.wave == wave)) {
             enemy = construct_enemy_from_set_row(row);
             if (enemy != NULL) {
-                row->enemy = enemy;
+                row->enemy = pso_njs_h2g(enemy);
                 row->flags |= 2;
                 enemy->room = row->placement.room;
                 enemy->set_group = row->placement.wave;
@@ -753,7 +753,7 @@
         if (local != 0) {
             obj = create_set_object_from_row(set);
             if (obj != NULL) {
-                set->object = obj;
+                set->object = pso_njs_h2g(obj);
                 set->flags |= 0x12;
                 obj->room = set->placement.room;
                 obj->set_group = set->placement.wave;
@@ -791,7 +791,7 @@
         if (!(flags & 8) && !(flags & 2) && (row->placement.room == room) && (row->placement.wave == wave)) {
             set_object = create_set_object_from_row(row);
             if (set_object != NULL) {
-                row->object = set_object;
+                row->object = pso_njs_h2g(set_object);
                 row->flags = (s16) (row->flags | 0x12);
                 set_object->room = row->placement.room;
                 set_object->set_group = row->placement.wave;
@@ -855,7 +855,7 @@
         if (!(flags & 8) && !(flags & 2) && (row->type != -1) && ((flags & 1) || (row->placement.wave == 0))) {
             enemy = construct_enemy_from_set_row(row);
             if (enemy != NULL) {
-                row->enemy = enemy;
+                row->enemy = pso_njs_h2g(enemy);
                 row->flags |= 2;
                 enemy->room = row->placement.room;
                 enemy->set_group = row->placement.wave;
@@ -886,7 +886,7 @@
         if (!(flags & 8) && !(flags & 2) && (row->type != -1) && ((flags & 1) || (row->placement.wave == 0))) {
             enemy = construct_enemy_from_set_row(row);
             if (enemy != NULL) {
-                row->enemy = enemy;
+                row->enemy = pso_njs_h2g(enemy);
                 row->flags |= 2;
                 enemy->room = row->placement.room;
                 enemy->set_group = row->placement.wave;
@@ -957,7 +957,7 @@
         if (!(flags & 8) && !(flags & 2) && (row->type != -1) && ((flags & 0x10) || (flags & 1) || (row->placement.wave == 0))) {
             set_object = create_set_object_from_row(row);
             if (set_object != NULL) {
-                row->object = set_object;
+                row->object = pso_njs_h2g(set_object);
                 row->flags = (s16) (row->flags | 2);
                 set_object->room = row->placement.room;
                 set_object->set_index = row->placement.set_index;
@@ -987,7 +987,7 @@
         if (!(flags & 8) && !(flags & 2) && (row->type != -1) && ((flags & 0x10) || (flags & 1) || (row->placement.wave == 0))) {
             set_object = create_set_object_from_row(row);
             if (set_object != NULL) {
-                row->object = set_object;
+                row->object = pso_njs_h2g(set_object);
                 row->flags = (s16) (row->flags | 2);
                 set_object->room = row->placement.room;
                 set_object->set_index = row->placement.set_index;
@@ -1162,7 +1162,7 @@
             p->placement.set_index = -1;
             obj = call_area_set_factory(manager, p);
             if (obj != NULL) {
-                p->object = obj;
+                p->object = pso_njs_h2g(obj);
                 obj->room = p->placement.room;
                 obj->set_entry = p;
                 p->flags |= 2;
@@ -1381,25 +1381,25 @@
     lbl_805D6100 = 0;
     for (s32 i = 17; i >= 0; i--) {
         if (floor_set_archives[i].enemies != NULL) {
-            floor_set_archive_pool->free(floor_set_archives[i].enemies);
+            floor_set_archive_pool->free(NJS_HANDLE(AramBlock, floor_set_archives[i].enemies));
             floor_set_archives[i].enemies = NULL;
         }
     }
     for (s32 i = 17; i >= 0; i--) {
         if (floor_set_archives[i].objects != NULL) {
-            floor_set_archive_pool->free(floor_set_archives[i].objects);
+            floor_set_archive_pool->free(NJS_HANDLE(AramBlock, floor_set_archives[i].objects));
             floor_set_archives[i].objects = NULL;
         }
     }
     for (s32 i = 17; i >= 0; i--) {
         if (floor_enemy_group_table[i].enemy_rows != NULL) {
-            floor_saved_row_heap->heap_free(floor_enemy_group_table[i].enemy_rows);
+            floor_saved_row_heap->heap_free(pso_njs_g2h(floor_enemy_group_table[i].enemy_rows));
             floor_enemy_group_table[i].enemy_rows = NULL;
         }
     }
     for (s32 i = 17; i >= 0; i--) {
         if (floor_enemy_group_table[i].object_rows != NULL) {
-            floor_saved_row_heap->heap_free(floor_enemy_group_table[i].object_rows);
+            floor_saved_row_heap->heap_free(pso_njs_g2h(floor_enemy_group_table[i].object_rows));
             floor_enemy_group_table[i].object_rows = NULL;
         }
     }
@@ -1410,7 +1410,7 @@
     LobbyFloorRows *area = &floor_enemy_group_table[get_current_floor()];
     if (current_floor_object_set_rows != 0) {
         FloorObjectSetRow *s = current_floor_object_set_rows;
-        FloorSavedSetRow *d = area->object_rows;
+        FloorSavedSetRow *d = NJS_HANDLE(FloorSavedSetRow, area->object_rows);
         s32 i;
         for (i = 0; i < area->object_count; i++, s++, d++) {
             if (s->placement.entity_id == d->id) {
@@ -1420,7 +1420,7 @@
     }
     if (current_floor_enemy_set_rows != 0) {
         EnemySetRecord *s = current_floor_enemy_set_rows;
-        FloorSavedSetRow *d = area->enemy_rows;
+        FloorSavedSetRow *d = NJS_HANDLE(FloorSavedSetRow, area->enemy_rows);
         s32 i;
         for (i = 0; i < area->enemy_count; i++, s++, d++) {
             if (s->placement.id == d->id) {
@@ -1487,9 +1487,9 @@
         current_floor_object_set_rows = (FloorObjectSetRow *)TQuestVmDownLoadServer::alloc(bytes);
     }
     if (current_floor_object_set_rows != NULL) {
-        floor_set_archive_pool->transfer(floor_set_archives[index].objects, current_floor_object_set_rows, bytes, 1);
+        floor_set_archive_pool->transfer(NJS_HANDLE(AramBlock, floor_set_archives[index].objects), current_floor_object_set_rows, bytes, 1);
         o = current_floor_object_set_rows;
-        r = floor->object_rows;
+        r = NJS_HANDLE(FloorSavedSetRow, floor->object_rows);
         for (i = 0; i < floor->object_count; i++, o++, r++) {
             if (o->placement.entity_id == r->id) {
                 o->flags = r->flags;
@@ -1504,9 +1504,9 @@
         current_floor_enemy_set_rows = (EnemySetRecord *)TQuestVmDownLoadServer::alloc(bytes);
     }
     if (current_floor_enemy_set_rows != NULL) {
-        floor_set_archive_pool->transfer(floor_set_archives[index].enemies, current_floor_enemy_set_rows, bytes, 1);
+        floor_set_archive_pool->transfer(NJS_HANDLE(AramBlock, floor_set_archives[index].enemies), current_floor_enemy_set_rows, bytes, 1);
         e = current_floor_enemy_set_rows;
-        r2 = floor->enemy_rows;
+        r2 = NJS_HANDLE(FloorSavedSetRow, floor->enemy_rows);
         for (j = 0; j < floor->enemy_count; j++, e++, r2++) {
             if (e->placement.id == r2->id) {
                 e->flags = r2->flags;
```

### 3.2 src/pso/unk/text_803DD5E0.cpp

sites: 38,40,46,89,134,137,139,175,179,202,426 · edits: 11 · shapes: g2h-load+h2g-store+NJS_HANDLE-slot+slot-subscript
· probe: **12 -> 1** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/unk/text_803DD5E0.cpp
+++ b/src/pso/unk/text_803DD5E0.cpp
@@ -35,15 +35,15 @@
     int i;
 
     for (i = 0; i < count; ++i) {
-        entries[i].filename = (char *)(names + i * 0x1C);
-    }
-    list->textures = entries;
+        entries[i].filename = (u32)(names + i * 0x1C);
+    }
+    list->textures = pso_njs_h2g(entries);
 }
 
 s32 load_texture_entry(NJS_TEXLIST *list, s32 index) {
     s32 loader;
 
-    switch (list->textures[index].attr & 0x5C000000) {
+    switch (NJS_HANDLE(NJS_TEXNAME, list->textures)[index].attr & 0x5C000000) {
     case 0x0:
         loader = 0;
         break;
@@ -86,7 +86,7 @@
     if (r != 0) {
         return r;
     }
-    append_extension_if_missing(name, list->textures[index].filename, ".GVR");
+    append_extension_if_missing(name, (char *)pso_njs_g2h(NJS_HANDLE(NJS_TEXNAME, list->textures)[index].filename), ".GVR");
     pfile = &file;
     if (DVDOpen(name, pfile) == 0) {
         texture_error.code = 2;
@@ -131,12 +131,12 @@
     if (r != 0) {
         return r;
     }
-    chunk = (uaddr)find_gvrt_texture_chunk((TLobby *)&desc, (u32 *)&global_index, (u32 *)list->textures[index].filename);
+    chunk = (uaddr)find_gvrt_texture_chunk((TLobby *)&desc, (u32 *)&global_index, (u32 *)pso_njs_g2h(NJS_HANDLE(NJS_TEXNAME, list->textures)[index].filename));
     if (chunk == 0) {
         chunk = (uaddr)find_gvrt_texture_chunk((TLobby *)&desc, (u32 *)&global_index,
-                                               (u32 *)reinterpret_cast<GinjaTextureInput *>(list->textures[index].filename)->data);
+                                               (u32 *)reinterpret_cast<GinjaTextureInput *>(pso_njs_g2h(NJS_HANDLE(NJS_TEXNAME, list->textures)[index].filename))->data);
         if (chunk == 0) {
-            input = reinterpret_cast<GinjaTextureInput *>(list->textures[index].filename);
+            input = reinterpret_cast<GinjaTextureInput *>(pso_njs_g2h(NJS_HANDLE(NJS_TEXNAME, list->textures)[index].filename));
             desc.width = input->width;
             desc.height = input->height;
             desc.format = input->format;
@@ -172,11 +172,11 @@
     if (r != 0) {
         return r;
     }
-    input = reinterpret_cast<GinjaTextureInput *>(list->textures[index].filename);
+    input = reinterpret_cast<GinjaTextureInput *>(pso_njs_g2h(NJS_HANDLE(NJS_TEXNAME, list->textures)[index].filename));
     desc.width = input->width;
     desc.height = input->height;
     desc.format = input->format;
-    global_index = list->textures[index].global_index;
+    global_index = NJS_HANDLE(NJS_TEXNAME, list->textures)[index].global_index;
     if ((u32)id != 0xFFFFFFFF) {
         global_index = id;
     }
@@ -199,7 +199,7 @@
     if (r != 0) {
         return r;
     }
-    global_index = list->textures[index].global_index;
+    global_index = NJS_HANDLE(NJS_TEXNAME, list->textures)[index].global_index;
     if ((u32)id != 0xFFFFFFFF) {
         global_index = id;
     }
@@ -423,7 +423,7 @@
     n = list->nbTexture;
     for (i = 0; i < n; i++) {
         for (k = 0; k < n; k++) {
-            if (i == (list->textures[k].attr & 0xFFFF)) {
+            if (i == (NJS_HANDLE(NJS_TEXNAME, list->textures)[k].attr & 0xFFFF)) {
                 break;
             }
         }
```

### 3.3 src/pso/item/TItem.cpp

sites: 868,893,900,1419,1518 · edits: 5 · shapes: g2h-load+NJS_HANDLE-slot
· probe: **8 -> 3** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/item/TItem.cpp
+++ b/src/pso/item/TItem.cpp
@@ -865,7 +865,7 @@
                 model_node_visit_callback(object);
             }
             if (!(flags & NJD_EVAL_HIDE) && model_node_draw_callback != NULL) {
-                model_node_draw_callback(object->model);
+                model_node_draw_callback(pso_njs_g2h(object->model));
             }
             if (!(flags & NJD_EVAL_BREAK)) {
                 draw_model_node_tree(object->child);
@@ -890,14 +890,14 @@
                 ++model_node_matrix_output_cursor;
             }
             if (!(flags & NJD_EVAL_HIDE) && model_node_draw_callback != NULL) {
-                model_node_draw_callback(object->model);
+                model_node_draw_callback(pso_njs_g2h(object->model));
             }
             if (!(flags & NJD_EVAL_BREAK)) {
                 draw_model_node_tree(object->child);
             }
             pop_matrix();
         }
-        object = object->sibling;
+        object = NJS_HANDLE(NJS_OBJECT, object->sibling);
     } while (object != NULL);
 }
 
@@ -1416,7 +1416,7 @@
 }
 
 void scroll_item_model_uvs(NJS_OBJECT *object, s32 du, s32 dv, u16 mesh) {
-    scroll_ginja_mesh_uvs(static_cast<GinjaModel *>(object->model), du, dv, mesh, 1);
+    scroll_ginja_mesh_uvs(NJS_HANDLE(GinjaModel, object->model), du, dv, mesh, 1);
     PPCSync();
 }
 
@@ -1515,7 +1515,7 @@
         set_constant_material(&item_box_primary_colors[box_color]);
         draw_ginja_model(box->model);
         set_constant_material(&item_box_secondary_colors[box_color]);
-        draw_ginja_model(box->child->model);
+        draw_ginja_model(pso_njs_g2h(NJS_HANDLE(NJS_OBJECT, box->child)->model));
         pop_matrix();
         TParticleCtrl::end_effect_render();
         set_default_alpha_blend();
```

### 3.4 src/pso/ui/charcreate/TChCrLoadReq.cpp

sites: 38,39,40,43,46 · edits: 6 · shapes: NJS_HANDLE-slot+slot-space-compare
· probe: **8 -> 0** **(CLEAN)**

```diff
--- a/src/pso/ui/charcreate/TChCrLoadReq.cpp
+++ b/src/pso/ui/charcreate/TChCrLoadReq.cpp
@@ -31,19 +31,19 @@
     if (object->evalflags & 0xFFFFE000) {
         return 0;
     }
-    static NJS_OBJECT *node_tree_range_start = (NJS_OBJECT *)0x0;
-    static NJS_OBJECT *node_tree_range_end = (NJS_OBJECT *)0x200000;
-    static void *model_range_start = (void *)0x0;
-    static void *model_range_end = (void *)0x200000;
+    static const u32 node_tree_range_start = 0x0; // P2-AQP slot-space bound (AV-3#6)
+    static const u32 node_tree_range_end = 0x200000;
+    static const u32 model_range_start = 0x0;
+    static const u32 model_range_end = 0x200000;
     if ((object->child > node_tree_range_start && object->child < node_tree_range_end) ||
         (object->sibling > node_tree_range_start && object->sibling < node_tree_range_end) ||
         (object->model > model_range_start && object->model < model_range_end)) {
         return 0;
     }
-    if (object->child != NULL && is_node_tree_valid(object->child) == 0) {
-        return 0;
-    }
-    if (object->sibling != NULL && is_node_tree_valid(object->sibling) == 0) {
+    if (object->child != 0 && is_node_tree_valid(NJS_HANDLE(NJS_OBJECT, object->child)) == 0) {
+        return 0;
+    }
+    if (object->sibling != 0 && is_node_tree_valid(NJS_HANDLE(NJS_OBJECT, object->sibling)) == 0) {
         return 0;
     }
     return 1;
```

### 3.5 src/pso/unk/TLobby_80209524.cpp

sites: 1010,1029,1097,1110 · edits: 4 · shapes: NJS_HANDLE-slot
· probe: **4 -> 0** **(CLEAN)**

```diff
--- a/src/pso/unk/TLobby_80209524.cpp
+++ b/src/pso/unk/TLobby_80209524.cpp
@@ -1007,7 +1007,7 @@
     s32 n = 0;
     s32 i;
     if (g->enemy_count > 0) {
-        e = g->enemy_rows;
+        e = NJS_HANDLE(FloorSavedSetRow, g->enemy_rows);
         if (e != 0) {
             for (i = 0; i < g->enemy_count; i++, e++) {
                 if (e->room == room && e->wave == wave) {
@@ -1026,7 +1026,7 @@
     FloorSavedSetRow *e;
     s32 i;
     if (t->object_count > 0) {
-        e = t->object_rows;
+        e = NJS_HANDLE(FloorSavedSetRow, t->object_rows);
         if (e != NULL) {
             for (i = 0; i < t->object_count; i++, e++) {
                 if (e->room == room && e->wave == group) {
@@ -1094,7 +1094,7 @@
     for (i = 0; i < 18; i++) {
         if (floor_enemy_group_table[i].object_count > 0) {
             count = floor_enemy_group_table[i].object_count;
-            row = floor_enemy_group_table[i].object_rows;
+            row = NJS_HANDLE(FloorSavedSetRow, floor_enemy_group_table[i].object_rows);
             for (j = 0; j < count; j++) {
                 s16 flags = *src;
                 bswap_16((u16 *)&flags);
@@ -1107,7 +1107,7 @@
     src = (s16 *)((u8 *)block + block->enemy_offset);
     for (j = 0; j < 18; j++) {
         if (floor_enemy_group_table[j].enemy_count > 0) {
-            row = floor_enemy_group_table[j].enemy_rows;
+            row = NJS_HANDLE(FloorSavedSetRow, floor_enemy_group_table[j].enemy_rows);
             count = floor_enemy_group_table[j].enemy_count;
             for (i = 0; i < count; i++) {
                 s16 flags = *src;
```

### 3.6 src/pso/boss/derolle/TObjBoss2Tunnel.cpp

sites: 332,336,356 · edits: 3 · shapes: NJS_HANDLE-slot
· probe: **10 -> 7** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/boss/derolle/TObjBoss2Tunnel.cpp
+++ b/src/pso/boss/derolle/TObjBoss2Tunnel.cpp
@@ -329,11 +329,11 @@
     NJS_OBJECT *child;
     NJS_OBJECT *sibling;
     count = 1;
-    child = node->child;
+    child = NJS_HANDLE(NJS_OBJECT, node->child);
     if (child != NULL) {
         count = count_tree_nodes(child) + 1;
     }
-    sibling = node->sibling;
+    sibling = NJS_HANDLE(NJS_OBJECT, node->sibling);
     if (sibling != NULL) {
         count += count_tree_nodes(sibling);
     }
@@ -353,7 +353,7 @@
         if (!(flags & 0x10)) {
             count += count_visible_nodes(node->child);
         }
-        node = node->sibling;
+        node = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (node != NULL);
     return count;
 }
```

### 3.7 src/pso/render/GinjaMaterial.cpp

sites: 367,388,421 · edits: 3 · shapes: NJS_HANDLE-slot+slot-subscript
· probe: **3 -> 0** **(CLEAN)**

```diff
--- a/src/pso/render/GinjaMaterial.cpp
+++ b/src/pso/render/GinjaMaterial.cpp
@@ -364,7 +364,7 @@
                     0.0f != model_material_color.a) {
                     opaque = 0;
                 }
-                const u32 &format = current_render_context->textures->textures[texture].texture->surface->header.format;
+                const u32 &format = NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, current_render_context->textures->textures)[texture].texture)->surface->header.format;
                 GXSetZCompLoc(GX_FALSE);
                 if (opaque) {
                     GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
@@ -385,8 +385,7 @@
                 GXSetBlendMode(mode, source, destination, operation);
             }
         } else if (mode != GX_BM_NONE) {
-            surface = current_render_context->textures->textures[
-                ginja_material_cache.texture & 0x1FFF].texture->surface;
+            surface = NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, current_render_context->textures->textures)[
                ginja_material_cache.texture & 0x1FFF].texture)->surface;
             if (ginja_material_cache.textured != 0 && surface->header.format == GX_TF_CMPR &&
                 source == GX_BL_SRCALPHA && destination == GX_BL_INVSRCALPHA && (surface->header.flags & 0x40000000) != 0) {
                 if ((u32)(ginja_draw_list_state & 8) == 0 &&
@@ -418,8 +417,7 @@
                 }
             }
         } else {
-            surface = current_render_context->textures->textures[
-                ginja_material_cache.texture & 0x1FFF].texture->surface;
+            surface = NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, current_render_context->textures->textures)[
                ginja_material_cache.texture & 0x1FFF].texture)->surface;
             if (surface->header.format == GX_TF_CMPR && (surface->header.flags & 0x40000000) == 0) {
                 GXSetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
                 GXSetZCompLoc(GX_TRUE);
```

### 3.8 src/pso/render/psoLoadTexture.cpp

sites: 89,145,149 · edits: 3 · shapes: g2h-load+h2g-store+NJS_HANDLE-slot
· probe: **3 -> 0** **(CLEAN)**

```diff
--- a/src/pso/render/psoLoadTexture.cpp
+++ b/src/pso/render/psoLoadTexture.cpp
@@ -86,7 +86,7 @@
         }
         if (r == -1) {
             ret = -1;
-            TLobby::debug_print("psoLoadTexture:%s load error.", texlist->textures->filename);
+            TLobby::debug_print("psoLoadTexture:%s load error.", (char *)pso_njs_g2h(NJS_HANDLE(NJS_TEXNAME, texlist->textures)->filename));
             break;
         }
         TLobby::run_frame();
@@ -142,11 +142,11 @@
     res = (NJS_TEXLIST *)alloc_zeroed_array(1, count * 0x2C + 8);
     res->nbTexture = count;
     i = 0;
-    res->textures = (NJS_TEXNAME *)(res + 1);
+    res->textures = pso_njs_h2g((NJS_TEXNAME *)(res + 1));
     ent = (NJS_TEXNAME *)(res + 1);
     str = (char *)(ent + count);
     for (; i < count; i++, ent++, str += 0x20) {
-        ent->filename = str;
+        ent->filename = pso_njs_h2g(str);
         language_read_cursor += 2;
         if (hdr.flags & 1) {
             TLanguage::read_into_buffer(str, 0x1C);
```

### 3.9 src/pso/unk/text_803DA798.cpp

sites: 32,164,250 · edits: 3 · shapes: h2g-store+NJS_HANDLE-slot+slot-subscript
· probe: **3 -> 0** **(CLEAN)**

```diff
--- a/src/pso/unk/text_803DA798.cpp
+++ b/src/pso/unk/text_803DA798.cpp
@@ -29,7 +29,7 @@
 #pragma scheduling off
 #pragma peephole off
 s32 set_texture_num(s32 i) {
-    GinjaTextureState *texture = current_render_context->textures->textures[i].texture;
+    GinjaTextureState *texture = NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, current_render_context->textures->textures)[i].texture);
     current_render_context->texture_header = &texture->surface->header;
     current_render_context->tlut_name = texture->tlut_name;
     return 1;
@@ -161,7 +161,7 @@
 #pragma scheduling off
 #pragma peephole off
 void set_texlist_texture(NJS_TEXLIST *list, s32 index, s32 slot) {
-    list->textures[index].texture = texture_states + slot;
+    NJS_HANDLE(NJS_TEXNAME, list->textures)[index].texture = pso_njs_h2g(texture_states + slot);
 }
 #pragma pop
 
@@ -247,7 +247,7 @@
 
 s32 reload_list_texture_pixels(s32 index, s32 src, u32 use_header) {
     reset_texture_error();
-    s32 r = reload_texture_state_pixels(current_render_context->textures->textures[index].texture, src, use_header);
+    s32 r = reload_texture_state_pixels(NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, current_render_context->textures->textures)[index].texture), src, use_header);
     if (r < 0) {
         texture_error.entry_index = index;
     }
```

### 3.10 src/pso/object/TObjSinBoard.cpp

sites: 103,116 · edits: 2 · shapes: h2g-store+NJS_HANDLE-slot
· probe: **2 -> 0** **(CLEAN)**

```diff
--- a/src/pso/object/TObjSinBoard.cpp
+++ b/src/pso/object/TObjSinBoard.cpp
@@ -100,7 +100,7 @@
     if (sin_board_texture_buffer != 0) {
         safe_free(sin_board_texture_buffer);
         sin_board_texture_buffer = 0;
-        if (signboard_texture_list.textures->texture != NULL) {
+        if (NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, signboard_texture_list.textures)->texture) != NULL) {
             release_texture_list(&signboard_texture_list);
         }
     }
@@ -113,7 +113,7 @@
     sin_board_texture_buffer = buffer;
     if (buffer != NULL) {
         move_data(buffer, image, size);
-        signboard_texture_list.textures = &sin_board_texture_name;
+        signboard_texture_list.textures = pso_njs_h2g(&sin_board_texture_name);
         signboard_texture_list.nbTexture = 1;
         sin_board_texture_surface.set_render_rect(sin_board_texture_buffer, 0, 0, 0);
         set_texture_name(&sin_board_texture_name, &sin_board_texture_surface, 0, 0x40000000);
```

### 3.11 src/pso/render/GinjaTextureTable.cpp

sites: 20,24 · edits: 2 · shapes: h2g-store
· probe: **2 -> 0** **(CLEAN)**

```diff
--- a/src/pso/render/GinjaTextureTable.cpp
+++ b/src/pso/render/GinjaTextureTable.cpp
@@ -17,11 +17,11 @@
 #pragma optimize_for_size off
 void set_texture_name(NJS_TEXNAME *entry, GinjaTextureInput *input, u32 index, u32 attributes) {
     if (attributes & NJD_TEXATTR_TYPE_MEMORY) {
-        entry->filename = reinterpret_cast<char *>(input);
+        entry->filename = pso_njs_h2g(input);
         entry->attr = attributes;
         entry->global_index = index;
     } else {
-        entry->filename = reinterpret_cast<char *>(input);
+        entry->filename = pso_njs_h2g(input);
         entry->attr = attributes;
         entry->global_index = index;
     }
```

### 3.12 src/pso/ui/charcreate/TCharaMaking.cpp

sites: 516,551 · edits: 2 · shapes: h2g-store
· probe: **2 -> 0** **(CLEAN)**

```diff
--- a/src/pso/ui/charcreate/TCharaMaking.cpp
+++ b/src/pso/ui/charcreate/TCharaMaking.cpp
@@ -513,7 +513,7 @@
         NJS_TEXNAME *slot;
         const char **entry;
         char *name;
-        slot = list.textures = entries;
+        slot = entries;
        list.textures = pso_njs_h2g(entries);
         index = 0;
         list.nbTexture = source->count;
         entry = source->entries;
@@ -548,7 +548,7 @@
                 if (appearance->flags & 1 || (s8)appearance->section == -1) name[8] = 'z';
                 else name[8] = appearance->section + 'a';
             }
-            slot->filename = name;
+            slot->filename = pso_njs_h2g(name);
             slot->texaddr = 0;
             slot->attr = 0;
         }
```

### 3.13 src/pso/player/TObjPlayer_8023E480.cpp

sites: 91 · edits: 1 · shapes: NJS_HANDLE-slot
· probe: **1 -> 0** **(CLEAN)**

```diff
--- a/src/pso/player/TObjPlayer_8023E480.cpp
+++ b/src/pso/player/TObjPlayer_8023E480.cpp
@@ -88,7 +88,7 @@
         return 0;
     }
     n = texlist->nbTexture;
-    d = texlist->textures;
+    d = NJS_HANDLE(NJS_TEXNAME, texlist->textures);
     for (i = 0; i < n; i++) {
         d[i].texture = temporary_texture_entries[i].texture;
         d[i].attr = temporary_texture_entries[i].attr;
```

### 3.14 src/pso/render/GinjaTextureHeader.cpp

sites: 31 · edits: 1 · shapes: NJS_HANDLE-slot+slot-subscript
· probe: **1 -> 0** **(CLEAN)**

```diff
--- a/src/pso/render/GinjaTextureHeader.cpp
+++ b/src/pso/render/GinjaTextureHeader.cpp
@@ -28,7 +28,7 @@
 #pragma scheduling off
 #pragma peephole off
 void set_context_texture_palette_bank(s32 index, s32 bank) {
-    set_texture_palette_bank(current_render_context->textures->textures[index].texture, bank);
+    set_texture_palette_bank(NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, current_render_context->textures->textures)[index].texture), bank);
 }
 #pragma pop
 
```

### 3.15 src/pso/ui/font/TNECFontEORFunction.cpp

sites: 549 · edits: 1 · shapes: NJS_HANDLE-slot+slot-subscript
· probe: **2 -> 1** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/ui/font/TNECFontEORFunction.cpp
+++ b/src/pso/ui/font/TNECFontEORFunction.cpp
@@ -546,7 +546,7 @@
     }
     load_texture_list(menu_font_texture_list);
     for (m = 0; m < 2; m++) {
-        *(menu_font_glyph_pixel_buffers + m) = menu_font_texture_list[0].textures[m].texture->surface->header.pixels;
+        *(menu_font_glyph_pixel_buffers + m) = NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, menu_font_texture_list[0].textures)[m].texture)->surface->header.pixels;
         clear_words_80145804(*(menu_font_glyph_pixel_buffers + m), 0x2000);
     }
     menu_font_palette = (s16 *)mem_alloc(0x100);
```

### 3.16 src/pso/render/CameraCapture.cpp

sites: 214,235,245,247,264,302,317,358,399,470,728,785,802,810,827 · edits: 15 · shapes: NJS_HANDLE-slot
· probe: **74 -> 59** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/render/CameraCapture.cpp
+++ b/src/pso/render/CameraCapture.cpp
@@ -211,7 +211,7 @@
 }
 
 void release_capture_texture() {
-    if (shared_capture_texture_list.textures->texture != NULL) {
+    if (NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shared_capture_texture_list.textures)->texture) != NULL) {
         release_texture_list(&shared_capture_texture_list);
     }
 }
@@ -232,7 +232,7 @@
 }
 
 u32 *get_capture_texture_pixels() {
-    return shared_capture_texture_list.textures->texture->surface->header.pixels;
+    return NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shared_capture_texture_list.textures)->texture)->surface->header.pixels;
 }
 
 void capture_viewport_texture() {
@@ -242,9 +242,9 @@
         GXSetTexCopySrc(0, 0, 640, 480);
         GXSetTexCopyDst(640, 480, GX_TF_RGB565, GX_FALSE);
         if (camera_capture_flags & CAPTURE_CLEAR_ON_COPY) {
-            GXCopyTex(shared_capture_texture_list.textures->texture->surface->header.pixels, GX_TRUE);
+            GXCopyTex(NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shared_capture_texture_list.textures)->texture)->surface->header.pixels, GX_TRUE);
         } else {
-            GXCopyTex(shared_capture_texture_list.textures->texture->surface->header.pixels, GX_FALSE);
+            GXCopyTex(NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shared_capture_texture_list.textures)->texture)->surface->header.pixels, GX_FALSE);
         }
         GXPixModeSync();
     }
@@ -261,7 +261,7 @@
     f32 z;
     set_texture_list(&shared_capture_texture_list);
     if (cropped != 0) {
-        surface = shared_capture_texture_list.textures->texture->surface;
+        surface = NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shared_capture_texture_list.textures)->texture)->surface;
         surface->header.width = 256;
         surface->header.height = 192;
     }
@@ -299,7 +299,7 @@
     TObjPlayer::end_screen_quad();
     set_default_alpha_blend();
     if (cropped != 0) {
-        surface = shared_capture_texture_list.textures->texture->surface;
+        surface = NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shared_capture_texture_list.textures)->texture)->surface;
         surface->header.width = 640;
         surface->header.height = 480;
     }
@@ -314,7 +314,7 @@
     GinjaSurfaceState *surface;
     set_texture_list(&shared_capture_texture_list);
     if (cropped != 0) {
-        surface = shared_capture_texture_list.textures->texture->surface;
+        surface = NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shared_capture_texture_list.textures)->texture)->surface;
         surface->header.width = 256;
         surface->header.height = 192;
     }
@@ -355,7 +355,7 @@
     TObjPlayer::end_screen_quad();
     set_default_alpha_blend();
     if (cropped != 0) {
-        surface = shared_capture_texture_list.textures->texture->surface;
+        surface = NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shared_capture_texture_list.textures)->texture)->surface;
         surface->header.width = 640;
         surface->header.height = 480;
     }
@@ -396,7 +396,7 @@
 }
 
 void release_shadow_map_texture() {
-    if (shadow_map_texture_list.textures->texture != NULL) {
+    if (NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shadow_map_texture_list.textures)->texture) != NULL) {
         release_texture_list(&shadow_map_texture_list);
     }
 }
@@ -467,7 +467,7 @@
         GXSetCopyClear(clear, 0xFFFFFF);
         GXSetTexCopySrc(0, 0, 256, 256);
         GXSetTexCopyDst(256, 256, GX_TF_IA4, GX_FALSE);
-        GXCopyTex(shadow_map_texture_list.textures->texture->surface->header.pixels, GX_TRUE);
+        GXCopyTex(NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shadow_map_texture_list.textures)->texture)->surface->header.pixels, GX_TRUE);
         GXPixModeSync();
         mode = active_gx_render_mode;
         GXSetCopyFilter(mode->aa, mode->sample_pattern, get_display_copy_filter(), mode->vfilter);
@@ -725,7 +725,7 @@
             GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
             GXSetTexCopySrc(0, 0, 640, 480);
             GXSetTexCopyDst(640, 480, GX_TF_RGB565, GX_FALSE);
-            GXCopyTex(camera_render_target_singleton->texture_list.textures->texture->surface->header.pixels, GX_TRUE);
+            GXCopyTex(NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, camera_render_target_singleton->texture_list.textures)->texture)->surface->header.pixels, GX_TRUE);
             GXPixModeSync();
         }
         GXSetViewport(0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
@@ -782,7 +782,7 @@
         }
         GXSetTexCopySrc(corner.x, corner.y, 256, 192);
         GXSetTexCopyDst(256, 192, GX_TF_RGB565, GX_FALSE);
-        GXCopyTex(shared_capture_texture_list.textures->texture->surface->header.pixels, GX_TRUE);
+        GXCopyTex(NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shared_capture_texture_list.textures)->texture)->surface->header.pixels, GX_TRUE);
         GXPixModeSync();
         pad = &controller_physical_port_bank.controllers[3];
         pos[0] = 192.0f;
@@ -799,7 +799,7 @@
         TObjPlayer::draw_capture_caption_bar();
         GXSetTexCopySrc(192, 144, 256, 192);
         GXSetTexCopyDst(256, 192, GX_TF_RGB565, GX_FALSE);
-        GXCopyTex(shared_capture_texture_list.textures->texture->surface->header.pixels, GX_TRUE);
+        GXCopyTex(NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shared_capture_texture_list.textures)->texture)->surface->header.pixels, GX_TRUE);
         GXPixModeSync();
         break;
     case 1:
@@ -807,7 +807,7 @@
         GXSetCopyClear(full_clear, 0xFFFFFF);
         GXSetTexCopySrc(0, 0, 640, 480);
         GXSetTexCopyDst(640, 480, GX_TF_RGB565, GX_FALSE);
-        GXCopyTex(shared_capture_texture_list.textures->texture->surface->header.pixels, GX_TRUE);
+        GXCopyTex(NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shared_capture_texture_list.textures)->texture)->surface->header.pixels, GX_TRUE);
         GXPixModeSync();
         pad = &controller_physical_port_bank.controllers[3];
         pos[0] = 192.0f;
@@ -824,7 +824,7 @@
         TObjPlayer::draw_capture_caption_bar();
         GXSetTexCopySrc(192, 144, 256, 192);
         GXSetTexCopyDst(256, 192, GX_TF_RGB565, GX_FALSE);
-        GXCopyTex(shared_capture_texture_list.textures->texture->surface->header.pixels, GX_TRUE);
+        GXCopyTex(NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, shared_capture_texture_list.textures)->texture)->surface->header.pixels, GX_TRUE);
         GXPixModeSync();
         break;
     }
```

### 3.17 src/pso/render/ShapeMotionDraw.cpp

sites: 49,50,52,57,63,143,144,149,150 · edits: 9 · shapes: g2h-load+h2g-store+NJS_HANDLE-slot
· probe: **11 -> 2** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/render/ShapeMotionDraw.cpp
+++ b/src/pso/render/ShapeMotionDraw.cpp
@@ -46,21 +46,21 @@
             model = (NJS_CNK_MODEL *)node->model;
             if (model != NULL && !(node->evalflags & 8)) {
                 sample_shape_node_vertices(&model->vertex_list, &vertices);
-                saved = model->vertex_list;
-                model->vertex_list = vertices;
+                saved = pso_njs_g2h(model->vertex_list);
+                model->vertex_list = pso_njs_h2g(vertices);
                 ctx->draw(model);
-                model->vertex_list = saved;
+                model->vertex_list = pso_njs_h2g(saved);
             } else {
                 advance_shape_motion_row();
             }
         } else if (node->model != NULL && !(node->evalflags & 8)) {
-            ctx->draw(node->model);
+            ctx->draw(pso_njs_g2h(node->model));
         }
         if (node->child != NULL) {
             draw_shape_motion_node_tree(node->child);
         }
         pop_matrix();
-        node = node->sibling;
+        node = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (node != NULL);
 }
 
@@ -140,14 +140,14 @@
     if (vertex_keys != NULL) {
         find_motion_key_interval(vertex_keys, sizeof(NJS_MKEY_P), vertex_key_count, frame, &vertex_key0, &vertex_key1, &vertex_rate);
         vertex_weight = 1.0f - vertex_rate;
-        vertex0 = vertex_key0->key;
-        vertex1 = vertex_key1->key;
+        vertex0 = NJS_HANDLE(f32, vertex_key0->key);
+        vertex1 = NJS_HANDLE(f32, vertex_key1->key);
     }
     if (normal_keys != NULL) {
         find_motion_key_interval(normal_keys, sizeof(NJS_MKEY_P), normal_key_count, frame, &normal_key0, &normal_key1, &normal_rate);
         normal_weight = 1.0f - normal_rate;
-        normal0 = normal_key0->key;
-        normal1 = normal_key1->key;
+        normal0 = NJS_HANDLE(f32, normal_key0->key);
+        normal1 = NJS_HANDLE(f32, normal_key1->key);
     }
     p = src;
     *out = d = shape_vertex_buffer;
```

### 3.18 src/pso/render/ChunkTextureStripCollector.cpp

sites: 18,20,23,25,79,81,84,86 · edits: 8 · shapes: NJS_HANDLE-slot
· probe: **8 -> 0** **(CLEAN)**

```diff
--- a/src/pso/render/ChunkTextureStripCollector.cpp
+++ b/src/pso/render/ChunkTextureStripCollector.cpp
@@ -15,14 +15,14 @@
 
 void StripBufferPair::collect_model_strips_by_texture(NJS_OBJECT *node, s32 texture_filter) {
     do {
-        NJS_CNK_MODEL *model = static_cast<NJS_CNK_MODEL *>(node->model);
+        NJS_CNK_MODEL *model = NJS_HANDLE(NJS_CNK_MODEL, node->model);
         if (model != NULL && model->polygon_list != NULL) {
-            collect_plist_strips_by_texture(model->polygon_list, texture_filter);
+            collect_plist_strips_by_texture(NJS_HANDLE(s16, model->polygon_list), texture_filter);
         }
         if (node->child != 0) {
-            collect_model_strips_by_texture(node->child, texture_filter);
+            collect_model_strips_by_texture(NJS_HANDLE(NJS_OBJECT, node->child), texture_filter);
         }
-        node = node->sibling;
+        node = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (node != 0);
 }
 
@@ -76,14 +76,14 @@
 
 void StripBufferPair::count_model_strips_by_texture(NJS_OBJECT *node, s32 texture_filter) {
     do {
-        NJS_CNK_MODEL *model = static_cast<NJS_CNK_MODEL *>(node->model);
+        NJS_CNK_MODEL *model = NJS_HANDLE(NJS_CNK_MODEL, node->model);
         if (model != NULL && model->polygon_list != NULL) {
-            count_plist_strips_by_texture(model->polygon_list, texture_filter);
+            count_plist_strips_by_texture(NJS_HANDLE(s16, model->polygon_list), texture_filter);
         }
         if (node->child != 0) {
-            count_model_strips_by_texture(node->child, texture_filter);
+            count_model_strips_by_texture(NJS_HANDLE(NJS_OBJECT, node->child), texture_filter);
         }
-        node = node->sibling;
+        node = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (node != 0);
 }
 
```

### 3.19 src/pso/render/ModelDrawContext.cpp

sites: 56,60,65,91,114,121,133,141 · edits: 8 · shapes: g2h-load+NJS_HANDLE-slot
· probe: **11 -> 3** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/render/ModelDrawContext.cpp
+++ b/src/pso/render/ModelDrawContext.cpp
@@ -53,16 +53,16 @@
         if (model_node_callback != NULL) {
             model_node_callback(node);
         }
-        model = node->model;
+        model = pso_njs_g2h(node->model);
         if (model != NULL && (node->evalflags & 8) == 0) {
             queue_model_draw_matrix(model, *current_matrix);
         }
-        child = node->child;
+        child = NJS_HANDLE(NJS_OBJECT, node->child);
         if (child != NULL) {
             draw_ginja_motion_node_tree(child);
         }
         pop_matrix();
-        node = node->sibling;
+        node = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (node != NULL);
 }
 #pragma pop
@@ -88,7 +88,7 @@
         }
         if ((f & 7) == 7) {
             if (!(f & 8)) {
-                draw(node->model);
+                draw(pso_njs_g2h(node->model));
             }
             if (!(f & 0x10)) {
                 traverse_model_node_tree(node->child, draw);
@@ -111,14 +111,14 @@
                 scale_current_matrix(node->scl);
             }
             if (!(f & 8)) {
-                draw(node->model);
+                draw(pso_njs_g2h(node->model));
             }
             if (!(f & 0x10)) {
                 traverse_model_node_tree(node->child, draw);
             }
             pop_matrix();
         }
-        node = node->sibling;
+        node = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (node != NULL);
 }
 
@@ -130,7 +130,7 @@
         if (motion_node_draw_callback != NULL) {
             motion_node_draw_callback(node);
         }
-        t = node->model;
+        t = pso_njs_g2h(node->model);
         if (t != 0 && (node->evalflags & 8) == 0) {
             model_draw_context->draw(t);
         }
@@ -138,7 +138,7 @@
             draw_motion_node_tree_with_context(node->child);
         }
         pop_matrix();
-        node = node->sibling;
+        node = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (node != NULL);
 }
 
```

### 3.20 src/pso/render/ChunkModelDraw.cpp

sites: 99,122,129,498,556 · edits: 5 · shapes: g2h-load+NJS_HANDLE-slot+slot-subscript
· probe: **22 -> 17** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/render/ChunkModelDraw.cpp
+++ b/src/pso/render/ChunkModelDraw.cpp
@@ -96,7 +96,7 @@
         }
         if ((flags & 7) == 7) {
             if (!(flags & 8)) {
-                r = draw(node->model);
+                r = draw(pso_njs_g2h(node->model));
             }
             if (!(flags & 0x10) && (!(flags & 0x100) || r == 0)) {
                 transform_chunk_object_tree(node->child, draw);
@@ -119,14 +119,14 @@
                 scale_current_matrix(node->scl);
             }
             if (!(flags & 8)) {
-                r = draw(node->model);
+                r = draw(pso_njs_g2h(node->model));
             }
             if (!(flags & 0x10) && (!(flags & 0x100) || r == 0)) {
                 transform_chunk_object_tree(node->child, draw);
             }
             pop_matrix();
         }
-        object = node->sibling;
+        object = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (object != NULL);
 }
 #undef node
@@ -495,7 +495,7 @@
     } else if (blend_mode != GX_BM_NONE) {
         alpha_reasons = 0;
         material_opaque = 1U;
-        alpha_surface = current_render_context->textures->textures[chunk_texture_record_index].texture->surface;
+        alpha_surface = NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, current_render_context->textures->textures)[chunk_texture_record_index].texture)->surface;
         id = model_render_flags;
         if (((u32) (id & 0x10) != 0) && (1.0f != model_material_color.a)) {
             material_opaque = 0U;
@@ -553,7 +553,7 @@
         if (chunk_render_flags != 0) {
             chunk_render_flags |= 0x400;
         }
-        zcomp_surface = current_render_context->textures->textures[chunk_texture_record_index].texture->surface;
+        zcomp_surface = NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, current_render_context->textures->textures)[chunk_texture_record_index].texture)->surface;
         if ((zcomp_surface->header.format == 0xEU) && ((u32) (zcomp_surface->header.flags & 0x40000000) != 0)) {
             set_chunk_alpha_compare_ref(0x1F);
             GXSetBlendMode(GX_BM_NONE, src_factor, dst_factor, GX_LO_CLEAR);
```

### 3.21 src/pso/render/ChunkMotionDraw.cpp

sites: 42,50,95,103 · edits: 4 · shapes: g2h-load+NJS_HANDLE-slot
· probe: **6 -> 2** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/render/ChunkMotionDraw.cpp
+++ b/src/pso/render/ChunkMotionDraw.cpp
@@ -39,7 +39,7 @@
         if (chunk_model_node_callback != NULL) {
             chunk_model_node_callback(node);
         }
-        model = node->model;
+        model = pso_njs_g2h(node->model);
         if (model != NULL && (node->evalflags & 8) == 0) {
             model_draw_context->draw(model);
         }
@@ -47,7 +47,7 @@
             draw_object_tree_recursive(node->child);
         }
         pop_matrix();
-        node = node->sibling;
+        node = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (node != NULL);
 }
 
@@ -92,7 +92,7 @@
         if (chunk_model_node_callback != NULL) {
             chunk_model_node_callback(node);
         }
-        model = node->model;
+        model = pso_njs_g2h(node->model);
         if (model != NULL && (node->evalflags & 8) == 0) {
             model_draw_context->draw(model);
         }
@@ -100,6 +100,6 @@
             traverse_animated_node_tree(node->child);
         }
         pop_matrix();
-        node = node->sibling;
+        node = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (node != NULL);
 }
```

### 3.22 src/pso/render/GinjaMotionDraw.cpp

sites: 33,39,70,76 · edits: 4 · shapes: g2h-load+NJS_HANDLE-slot
· probe: **6 -> 2** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/render/GinjaMotionDraw.cpp
+++ b/src/pso/render/GinjaMotionDraw.cpp
@@ -30,13 +30,13 @@
             PSMTXInverse(*current_matrix, inverse);
             PSMTXTranspose(inverse, normal_matrix);
             GXLoadNrmMtxImm(normal_matrix, 0);
-            model_draw_context->draw(node->model);
+            model_draw_context->draw(pso_njs_g2h(node->model));
         }
         if (node->child != NULL) {
             render_model_tree(node->child);
         }
         pop_matrix();
-        node = node->sibling;
+        node = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (node != NULL);
 }
 
@@ -67,12 +67,12 @@
             PSMTXInverse(*current_matrix, inverse);
             PSMTXTranspose(inverse, normal_matrix);
             GXLoadNrmMtxImm(normal_matrix, 0);
-            model_draw_context->draw(node->model);
+            model_draw_context->draw(pso_njs_g2h(node->model));
         }
         if (node->child != NULL) {
             render_matrix_chain(node->child);
         }
         pop_matrix();
-        node = node->sibling;
+        node = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (node != NULL);
 }
```

### 3.23 src/pso/render/AltModel.cpp

sites: 36,59,65 · edits: 3 · shapes: h2g-store
· probe: **3 -> 0** **(CLEAN)**

```diff
--- a/src/pso/render/AltModel.cpp
+++ b/src/pso/render/AltModel.cpp
@@ -33,7 +33,7 @@
 
 NJS_OBJECT *prepend_alt_model(NJS_OBJECT *next, s32 diffuse) {
     alt_model_strips.diffuse = diffuse;
-    alt_model_object.sibling = next;
+    alt_model_object.sibling = pso_njs_h2g(next);
     return &alt_model_object;
 }
 void TObjEneDD_ST::draw_with_alt_model(s32 diffuse) {
@@ -56,13 +56,13 @@
     strips[4] = 0xFF;
     strips[5] = 0xFF;
     alt_model.vertex_list = NULL;
-    alt_model.polygon_list = strips;
+    alt_model.polygon_list = pso_njs_h2g(strips);
     alt_model.center[0] = 0.0f;
     alt_model.center[1] = 0.0f;
     alt_model.center[2] = 0.0f;
     alt_model.r = 0.0f;
     object->evalflags = 0x57;
-    object->model = &alt_model;
+    object->model = pso_njs_h2g(&alt_model);
     object->pos[0] = 0.0f;
     object->pos[1] = 0.0f;
     object->pos[2] = 0.0f;
```

### 3.24 src/pso/render/TNodeMatrixRecorder.cpp

sites: 43,45 · edits: 2 · shapes: NJS_HANDLE-slot
· probe: **3 -> 1** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/render/TNodeMatrixRecorder.cpp
+++ b/src/pso/render/TNodeMatrixRecorder.cpp
@@ -40,9 +40,9 @@
         }
         pop_matrix();
         if (!(flags & NJD_EVAL_BREAK)) {
-            draw_node_tree(node->child);
+            draw_node_tree(NJS_HANDLE(NJS_OBJECT, node->child));
         }
-        node = node->sibling;
+        node = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (node != NULL);
 }
 
```

### 3.25 src/pso/render/MericarolTexture.cpp

sites: 29 · edits: 1 · shapes: h2g-store
· probe: **1 -> 0** **(CLEAN)**

```diff
--- a/src/pso/render/MericarolTexture.cpp
+++ b/src/pso/render/MericarolTexture.cpp
@@ -26,7 +26,7 @@
 void create_mericarol_render_texture(void) {
     mericarol_render_target.set_render_rect(mericarol_render_texture_buffer = mem_alloc(0x800), 0x902, 0x20, 0x20);
     set_texture_name(&mericarol_render_texture_name, &mericarol_render_target, 0x18696, 0x40800000);
-    mericarol_render_texlist[0].textures = &mericarol_render_texture_name;
+    mericarol_render_texlist[0].textures = pso_njs_h2g(&mericarol_render_texture_name);
     mericarol_render_texlist[0].nbTexture = 1;
     psoLoadTexture(mericarol_render_texlist);
 }
```

### 3.26 src/pso/render/NinjaUtil.cpp

sites: 12 · edits: 1 · shapes: h2g-store+NJS_HANDLE-slot+slot-subscript
· probe: **5 -> 4** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/render/NinjaUtil.cpp
+++ b/src/pso/render/NinjaUtil.cpp
@@ -9,7 +9,7 @@
 void clear_texlist_names(NJS_TEXLIST *texlist) {
     u32 i;
     for (i = 0; i < texlist->nbTexture; i++) {
-        texlist->textures[i].filename = texlist_null_name;
+        NJS_HANDLE(NJS_TEXNAME, texlist->textures)[i].filename = pso_njs_h2g(texlist_null_name);
     }
 }
 
```

### 3.27 src/pso/render/reflection.cpp

sites: 523 · edits: 1 · shapes: NJS_HANDLE-slot
· probe: **1 -> 0** **(CLEAN)**

```diff
--- a/src/pso/render/reflection.cpp
+++ b/src/pso/render/reflection.cpp
@@ -520,6 +520,6 @@
     GXSetCopyClear(clear_color, 0xFFFFFF);
     GXSetTexCopySrc(0, 0, 640, 480);
     GXSetTexCopyDst(640, 480, GX_TF_RGB565, GX_FALSE);
-    GXCopyTex(texture_list.textures->texture->surface->header.pixels, GX_TRUE);
+    GXCopyTex(NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, texture_list.textures)->texture)->surface->header.pixels, GX_TRUE);
     GXPixModeSync();
 }
```

### 3.28 src/pso/unk/text_8028A998.cpp

sites: 8,9,20,23 · edits: 4 · shapes: h2g-store+NJS_HANDLE-slot
· probe: **6 -> 0** **(CLEAN)**

```diff
--- a/src/pso/unk/text_8028A998.cpp
+++ b/src/pso/unk/text_8028A998.cpp
@@ -5,8 +5,8 @@
     if (root == NULL) {
         return;
     }
-    for (NJS_OBJECT *node = root->child; node != NULL; node = node->sibling) {
-        if (node->child == target) {
+    for (NJS_OBJECT *node = NJS_HANDLE(NJS_OBJECT, root->child); node != NULL; node = NJS_HANDLE(NJS_OBJECT, node->sibling)) {
+        if (node->child == pso_njs_h2g(target)) {
             node->child = NULL;
         }
     }
@@ -17,10 +17,10 @@
         return;
     }
     root->evalflags &= ~NJD_EVAL_BREAK;
-    for (NJS_OBJECT *node = root->child; node != NULL; node = node->sibling) {
+    for (NJS_OBJECT *node = NJS_HANDLE(NJS_OBJECT, root->child); node != NULL; node = NJS_HANDLE(NJS_OBJECT, node->sibling)) {
         node->evalflags &= ~NJD_EVAL_BREAK;
         if (node->child == NULL) {
-            node->child = target;
+            node->child = pso_njs_h2g(target);
         }
     }
 }
```

### 3.29 src/pso/unk/text_803DA16C.cpp

sites: 11,12,16 · edits: 3 · shapes: NJS_HANDLE-slot+slot-subscript
· probe: **3 -> 0** **(CLEAN)**

```diff
--- a/src/pso/unk/text_803DA16C.cpp
+++ b/src/pso/unk/text_803DA16C.cpp
@@ -8,12 +8,12 @@
     u32 i;
     s32 result = 1;
     for (i = 0; i < textures->nbTexture; ++i) {
-        if (textures->textures[i].texture != NULL &&
-            release_texture_state(textures->textures[i].texture) < 0) {
+        if (NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, textures->textures)[i].texture) != NULL &&
+            release_texture_state(NJS_HANDLE(GinjaTextureState, NJS_HANDLE(NJS_TEXNAME, textures->textures)[i].texture)) < 0) {
             texture_error.entry_index = i;
             result = -1;
         }
-        textures->textures[i].texture = NULL;
+        NJS_HANDLE(NJS_TEXNAME, textures->textures)[i].texture = 0;
     }
     return result;
 }
```

### 3.30 src/pso/unk/text_80327844.cpp

sites: 11,16 · edits: 2 · shapes: NJS_HANDLE-slot
· probe: **3 -> 1** (NO-NJS-RESIDUAL)

```diff
--- a/src/pso/unk/text_80327844.cpp
+++ b/src/pso/unk/text_80327844.cpp
@@ -8,11 +8,11 @@
     do {
         id = texture_id;
         if (node->model != NULL) {
-            collect_scroll_uv_entries(buffer, static_cast<GinjaModel *>(node->model), id);
+            collect_scroll_uv_entries(buffer, NJS_HANDLE(GinjaModel, node->model), id);
         }
         if (node->child != NULL) {
             visit_matching_models(buffer, node->child, id);
         }
-        node = node->sibling;
+        node = NJS_HANDLE(NJS_OBJECT, node->sibling);
     } while (node != NULL);
 }
```

## 4. Battery roll-up (python-set-math over probe outputs)

- files probed: 30 (C++ TUs only; zero `.c` rows in queue by NA §2 construction)
- edits authored: 165 · sites covered: 164 · sites with no authored edit: 0
- BEFORE NJS-shape errors: **274** -> AFTER: **105** (**-169**, the enumerated
  169 dialect sites + cascades die)
- **verified clean: 16/30** (adapter-shape files that go 0-error at NJS shape:
  TChCrLoadReq.cpp, TLobby_80209524.cpp, GinjaMaterial.cpp, psoLoadTexture.cpp, text_803DA798.cpp, TObjSinBoard.cpp, GinjaTextureTable.cpp, TCharaMaking.cpp, TObjPlayer_8023E480.cpp, GinjaTextureHeader.cpp, ChunkTextureStripCollector.cpp, AltModel.cpp, MericarolTexture.cpp, reflection.cpp, text_8028A998.cpp, text_803DA16C.cpp)
- remaining pointer↔slot-class errors across all 30 AFTER probes: **0**
- every one of the 164 enumerated sites fires in BEFORE (verified line-by-line against
  the NA §3 site lists) and is silent in AFTER.
- top-30 rows flipped CLEAN vs NA-queue: 16 of 30 -> if the integrator applies those
  16 diffs + allowlist rows against the post-apply-7 official LP64 certification
  (1,163 PASS/517 FAIL; disk corpus was mid-legacy-anchor-battery at write time —
  re-verify membership when the battery lands), the count moves +16. Queue shrinks
  169 -> 139 files pending the other-class tails below.

## 5. Named next-class findings (NOT AV-§3 — batch-2/other-lane supply)

1. **Static-initializer cluster** (AR §5 "static NJS trees with embedded pointers"):
   64 errors across 3 of these 30 files persist after their enumerated adapters —
   `CameraCapture.cpp:73-97` (57, static NJS_TEXNAME/NJS_TEXLIST arrays incl.
   `shadow_map_texnames`/filename string slots), `TObjBoss2Tunnel.cpp` static tree
   (6), `TNECFontEORFunction.cpp:39` (1). An expression adapter cannot fix an
   initialiser: the slot literals need either era-absolute guest constants or the
   handle program's stamp-at-genesis seam (`compat/njs/handle/pso_handle_windows.h`
   sketch). Named for P2-HP + batch-2, NOT waived silently.
2. **no-matching-function cascades** on call args NA did not enumerate (e.g.
   `TItem.cpp:871/896/1516` passing `object->child`/`box->model`): same adapter
   shapes, additional sites — batch-2 picks them up per-file (residual file list in
   `cache/scratch/aqp/aqp_rows.json`).
3. `PK-13 draw_ginja_model wrapper` (NA §3 priority note): resolved here at the
   per-call-site level for the queue members inside the top 30 (`TItem :1518`
   g2h at the wrapper arg). The remaining 12 PK rows ride batch-2 with §1 tiers 5-6.

## 6. Slice-generator contract (how this supply is consumed)

1. For each §3 diff: `git -C <decomp@6b38b5383> apply -p1` the block onto a
   throwaway worktree (NEVER the read-only main tree), slice the touched fns
   verbatim-with-adapter per the ladder6 `slice_fn` anchor machinery, revert.
2. Integrator adds the file rows to `build/njs_allowlist.txt` ONLY for TUs whose
   AFTER probe is CLEAN (the 16 in §4); others stay queued until §5 classes close.
3. Laws that bind the consumer: `-Icompat/njs` never on `.c`; compat/facade before
   host/bringup/include; `-include` trio order; waiver pin d4382fcd era — never regen.
4. This repo commit touches exactly: `docs/p2-adapter-queue-1.md` +
   `compat/njs/adapters/**` (new, inert). No decomp, build/, host/, data/ writes.

## Repro

`cache/scratch/aqp/` (this lane, profile scratch): `aqp_select2.py` (rank),
`aqp_author.py` (165 edits + diffs), `aqp_probe.sh` + driver (BEFORE/AFTER sweeps,
apply-7 shape), `aqp_tally.py` (this doc's tables/counts), `aqp_rows.json`,
`diffs/*.diff` (identical to the §3 blocks), `BEFORE/`, `AFTER/` err receipts.
