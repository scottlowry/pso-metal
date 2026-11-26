/* selftest.c — proves the pso_host.h scaffold compiles standalone.
 * Build: clang -std=c11 -Wall -Wextra -c selftest.c
 * No implementation is linked here; this is the API-contract gate only. */
#include "pso_host.h"

int main(void)
{
    /* Spot-check that the 15 most-called SDK symbols are declared with real
     * prototypes (taking their addresses is a declaration-exists assertion). */
    void *refs[] = {
        (void *)EXIImm, (void *)EXISync, (void *)CARDSetDiskID,
        (void *)OSRestoreInterrupts, (void *)GXSetZMode, (void *)EXISelect,
        (void *)EXIDeselect, (void *)EXIUnlock, (void *)GXSetVtxDesc,
        (void *)OSDisableInterrupts, (void *)GXSetZCompLoc, (void *)GXBegin,
        (void *)GXSetVtxAttrFmt, (void *)CARDGetResultCode, (void *)PSMTXConcat,
        /* tied-16th */ (void *)DCInvalidateRange, (void *)GXSetBlendMode,
    };
    (void)refs; /* unused until implementations link — contract check only */
    return 0;
}
