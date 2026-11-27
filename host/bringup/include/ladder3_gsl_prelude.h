/* ladder3 gsl-glue prelude — prepended to the VERBATIM find_gsl_entry slice
 * (TGStream.cpp:206-219, home 0x800FDD70). Era globals the loop reads:
 *   0x800FDD84 lwz r0, gsl_adxf_handle@sda21   (0x800FDD88 cmplwi/bne gate)
 *   0x800FDD98 lis r3, gsl_archive_entries@ha / :DDA0 addi 0x11E0 (base)
 *   0x800FDDC8 addi r30,1        (i++)
 *   0x800FDDCC addi r31,r31,0x30 (entry stride CONSTANT — era-layout proof)
 *   0x800FDDD0 lwz r0, gsl_entry_count@sda21 ; :DDD4 cmpw ; :DDD8 blt loop
 *   0x800FDDB4 bl strcmp
 * GslArchiveEntry (decomp GslArchive.h): char name[0x20]; s32 offset; s32
 * size; s32 decoded; u8 f2C[4] — identical 48-byte (0x30) host layout, no
 * pointers, so offsets need no waiver.
 */
#include "types.h"
#include "string.h"
#include "ladder3_harness.h"   /* GslArchiveEntry, AdxfHandle */

/* TGStream.cpp:68-71 definitions (era .bss; :69 array decl era-typed): */
AdxfHandle *gsl_adxf_handle;
GslArchiveEntry gsl_archive_entries[256];
s32 gsl_entry_count;
