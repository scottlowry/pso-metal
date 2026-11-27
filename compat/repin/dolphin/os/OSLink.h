#ifndef OSLINK_H
#define OSLINK_H

#include <dolphin/os/OSUtil.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OS_MODULE_VERSION 3

typedef struct OSModuleHeader OSModuleHeader;

typedef u32 OSModuleID;
typedef struct OSModuleQueue OSModuleQueue;
typedef struct OSModuleLink OSModuleLink;
typedef struct OSModuleInfo OSModuleInfo;
typedef struct OSSectionInfo OSSectionInfo;
typedef struct OSImportInfo OSImportInfo;
typedef struct OSRel OSRel;

struct OSModuleQueue {
    OSModuleInfo* head;
    OSModuleInfo* tail;
};

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

OSModuleQueue __OSModuleList AT_ADDRESS(0x800030C8);
const void* __OSStringTable AT_ADDRESS(0x800030D0);

struct OSModuleLink {
    OSModuleInfo* next;
    OSModuleInfo* prev;
};

struct OSSectionInfo {
    u32 offset;
    u32 size;
};

struct OSModuleInfo {
    OSModuleID id;
    OSModuleLink link;
    u32 numSections;
    u32 sectionInfoOffset;
    u32 nameOffset;
    u32 nameSize;
    u32 version;
};

struct OSModuleHeader {
    OSModuleInfo info;

    u32 bssSize;
    u32 relOffset;
    u32 impOffset;
    u32 impSize;
    u8 prologSection;
    u8 epilogSection;
    u8 unresolvedSection;
    u8 bssSection;
    u32 prolog;
    u32 epilog;
    u32 unresolved;

#if (2 <= OS_MODULE_VERSION)
    u32 align;
    u32 bssAlign;
#endif

#if (3 <= OS_MODULE_VERSION)
    u32 fixSize;
#endif
};

#define OSGetSectionInfo(module) ((OSSectionInfo*)(((OSModuleInfo*)(module))->sectionInfoOffset))

#define OS_SECTIONINFO_EXEC 0x1
#define OS_SECTIONINFO_OFFSET(offset) ((offset) & ~0x1)

struct OSImportInfo {
    OSModuleID id;
    u32 offset;
};

struct OSRel {
    u16 offset;
    u8 type;
    u8 section;
    u32 addend;
};

#define R_DOLPHIN_NOP 201
#define R_DOLPHIN_SECTION 202
#define R_DOLPHIN_END 203
#define R_DOLPHIN_MRKREF 204

BOOL OSLink(OSModuleInfo* newModule, void* bss);
BOOL OSLinkFixed(OSModuleInfo* newModule, void* bss);
BOOL OSUnlink(OSModuleInfo* module);
void OSSetStringTable(const void* string_table);
void __OSModuleInit(void);

#ifdef __cplusplus
};
#endif

#endif
