/* Thin caller around the era decoder so crt0's boot spine and any later
 * bring-up step can use it from C. Lives here (not in the decomp copy) so
 * prs_glue.cpp stays a one-literal diff of src/pso/system/prs.cpp. */
#include "pso_bringup.h"
#include "pso/system/prs.h"

extern "C" s32 pso_bringup_run_prs(u8 *out, u8 *in, s32 cap) {
    (void)cap; /* output capacity checking is a later-step host guard;
                * the era decoder itself is unbounded by design */
    PrsDecoder d;
    return d.prs_decompress(out, in, 0);
}
