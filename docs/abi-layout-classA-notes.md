# Class A handoff — real ABI layout deltas ("array size is negative")

The decomp verifies struct layouts with `LAYOUT_ASSERT(name, cond)`
(`include/types.h:69`) = `typedef char name[(cond) ? 1 : -1]`. Under Apple clang
arm64_32 these fire for **934 of the 1089** failing TUs — this is the dominant
residual class and it is REAL work for the port, not shim-able (faking it with
`-fshort-enums`/packing hacks would silently corrupt the layouts we're trying
to match).

## Top offender structs (by assert-hit frequency, baseline ilp32 run)

| Assert symbol | Hits | What the port must pin down |
|---|---|---|
| `PlayerWindowQuerySize` / `PlayerWindowCommandSize` | 119 + 119 | Net player-window structs; likely PPC bool=4B vs C99 `_Bool`=1B, plus enum-size deltas |
| `TBoss4DarkFalzState_size` / `TBoss4DarkFalzHandler_size` | 51 + 51 | Boss handler vtable/state packing |
| `TWindowHelpItem_size_check` | 36 | UI window item packing |
| `Type3StateSize` | 16 | Quest/state machine |
| `TWindowLockOn_size` | 12 | UI lock-on window |
| `CreationMenuEntrySize` | 11 | Lobby/character-creation menu |

## Suspected root causes (to confirm per struct in Phase 1)
1. **PPC `bool`**: MWCC `bool` is 4 bytes; clang `bool`/`_Bool` is 1. Need a
   `pso_bool` (u32) typedef strategy or `-D__boolbool...`-free explicit types.
2. **Unscoped enum width**: MWCC picks smallest fitting int type only under
   specific flags; clang uses `int` for unscoped enums by default → width deltas
   wherever an enum field isn't pinned to `s32`/`u8`.
3. **Bitfield allocation order**: PPC allocates bitfields MSB-first
   (`-Xclang -fno-ms-bitfields` vs big/little-endian packing); most structs
   contain flag words.
4. **`#pragma pack` interactions** with the above.

## Protocol for closing these out
Per struct: compile just the header under `-target arm64_32`, print
`sizeof`/`offsetof`, diff against the decomp's asserted size, fix with explicit
-width types in the pso-metal-side header overlays (compat/shims), never by
editing decomp source. Re-run `probes/tree_compile_probe.sh` after each batch.
