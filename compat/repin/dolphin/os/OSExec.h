#ifndef _DOLPHIN_OSEXEC_H_
#define _DOLPHIN_OSEXEC_H_

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    BOOL valid;
    u32 restartCode;
    u32 bootDol;
    void* regionStart;
    void* regionEnd;
    int argsUseDefault;
    void* argsAddr;
} OSExecParams;

typedef int (*appGetNextCallback)(void*, u32*, u32*);
typedef void (*appInitCallback)(void (*)(char*));
typedef void* (*appGetEntryCallback)();
typedef void (*AppLoaderCallback)(appInitCallback*, appGetNextCallback*, appGetEntryCallback*);

/* --- P2-MA MWCC-dialect preamble (shadow-only; docs/p2-mwccasm-1.md) ------
 * The era line below spells an MWCC/CodeWarrior absolute-address variable
 * binding:  `T name AT_ADDRESS(addr);`  with the era types.h spelling
 * `#define AT_ADDRESS(addr) : (addr)` (colon declarator). clang/C11/C++17
 * rejects it ("expected ';' after top level declarator" cascade). The pin is
 * a guest-linker contract with no clang parse-world analogue; host executing
 * builds route these names through D4 facades, so the macro is neutralized
 * empty here, per the P2-T precedent (compat/dialect/pso_dialect_compat.h
 * section 1: "a port defines the pin empty"). One neutralization serves the
 * whole TU: it also repairs era sites reached WITHOUT their own shadow
 * (os.h:125-136, OSLink.h:27-28, dsp/dsp.h:10) when any chain hits this file
 * first. Everything else below is era-VERBATIM. Guard name is era-identical,
 * so the era copy and this copy can never both process in one TU, and the
 * collision pin (compat/collision/pin_dolphin_types.h, which occupies the
 * _DOLPHIN_TYPES_H_ guard but never any header guard below) does not
 * suppress this file. */
#ifdef AT_ADDRESS
#undef AT_ADDRESS
#endif
#define AT_ADDRESS(addr) /* P2-MA: era address pin dropped host-side (parse world) */

OSExecParams* __OSExecParams AT_ADDRESS(0x800030F0);
s32 __OSAppLoaderOffset AT_ADDRESS(0x800030F4);

void OSExecv(const char* dolfile, const char** argv);
void OSExecl(const char* dolfile, const char* arg0, ...);

#ifdef __cplusplus
}
#endif

#endif
