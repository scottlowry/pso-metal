/* pso_host.h — umbrella for the pso-metal native host platform API scaffold.
 *
 * Re-hosts the 272-symbol GameCube dolphin SDK surface that PSO EP&I actually
 * calls (blueprint: docs/host-surface.md; site counts re-measured against
 * pso-decomp @6b38b5383 and confirmed matching for the top-17). Include THIS
 * header from port code; it mirrors the SDK names so game sources compile
 * unmodified. Declarations only — implementations land in later lanes.
 */
#ifndef PSO_HOST_H
#define PSO_HOST_H

#define PSOSH_VERSION_MAJOR 0
#define PSOSH_VERSION_MINOR 1   /* P1-N API contract scaffold */

#ifdef __cplusplus
extern "C" {
#endif

#include "psosh_types.h"
#include "psosh_gx.h"
#include "psosh_exi.h"
#include "psosh_card.h"
#include "psosh_os.h"
#include "psosh_vi.h"
#include "psosh_dvd.h"
#include "psosh_audio.h"
#include "psosh_pad.h"
#include "psosh_serial.h"
#include "psosh_mtx.h"
#include "psosh_gba.h"

#ifdef __cplusplus
}
#endif

#endif /* PSO_HOST_H */
