/* P1-L smoke: proves the LINK_TARGET toolchain does syntax+codegen+link+exec.
 * The decomp tree itself is not linkable yet (host glue is later Phase-1 work);
 * this only validates that the real LP64 link path the boot slice needs works
 * with the same clang invocation machinery the build system uses. */
#include <stdio.h>
int main(void) {
    printf("host_smoke: link target executes, sizeof(void*)=%zu (LP64 real-exec mode)\n",
           sizeof(void *));
    return 0;
}
