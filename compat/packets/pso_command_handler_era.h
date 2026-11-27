#ifndef PSO_METAL_COMPAT_PACKETS_COMMAND_HANDLER_ERA_H
#define PSO_METAL_COMPAT_PACKETS_COMMAND_HANDLER_ERA_H

/* P2-PK lane PK, 2026-11-27. INERT — referenced by no build/host/data file.
 *
 * Era-image mirror of `PsoCommandHandler`, the PPC dispatch-table entry of
 * `pso_command_dispatch_table` (decomp include/pso/network/protocol/
 * packet_classes.h:370-376, pinned there by LAYOUT_ASSERT(PsoCommandHandler_size,
 * sizeof == 8)). On PPC the handler field is a 4-byte code address; on this
 * LP64 host a real function pointer is 8 bytes, so the port-side struct is
 * 12 bytes and the era assert cannot hold — it pins RUNTIME table layout, not
 * a wire or disc image (packet_classes.h's FORMAT-KEEP records are the 6
 * wire asserts at rows 255/256/257/298/316/521, all of which PASS under
 * LP64; none is waived). The lane ask waives ONLY PsoCommandHandler_size.
 *
 * This header keeps the era image compile-visible afterwards: the true
 * 8-byte binary record with its u32 era code address, for any future
 * byte-true table-image supply (fixtures/rung-4 replay). stdint-self-
 * contained: no decomp include-path needed, zero blast radius.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Era (PPC) binary layout: u16 command; u16 area_mask; u32 handle. */
typedef struct PsoCommandHandlerEra {
    uint16_t command;    /* 6x subcommand id                       */
    uint16_t area_mask;  /* floor-area mask (subcommand_area_masks) */
    uint32_t handle;     /* PPC code address of the handler         */
} PsoCommandHandlerEra;

_Static_assert(sizeof(PsoCommandHandlerEra) == 8,
               "era dispatch-table record must stay 8 bytes (PPC image)");

/* Field accessors over a raw era image (byte-faithful; no host-order
 * reinterpretation — the table image is PPC-BE if it ever comes off a disc). */
static inline uint16_t pso_cmd_era_command(const PsoCommandHandlerEra *e) {
    return e->command;
}
static inline uint16_t pso_cmd_era_area_mask(const PsoCommandHandlerEra *e) {
    return e->area_mask;
}
static inline uint32_t pso_cmd_era_handle(const PsoCommandHandlerEra *e) {
    return e->handle;
}

#ifdef __cplusplus
} /* extern "C" */

/* Bridge for C++ supply code: port handler fn-ptr <-> era u32 slot. The
 * port-side record mirrors packet_classes.h rows 370-374 with the pointer
 * widened; the mapping is supply-time bookkeeping, never a wire conversion. */
typedef void (*PsoCommandFnEra)(void *);
struct PsoCommandHandlerPort {
    uint16_t command;
    uint16_t area_mask;
    PsoCommandFnEra handle;
};
#endif

#endif /* PSO_METAL_COMPAT_PACKETS_COMMAND_HANDLER_ERA_H */
