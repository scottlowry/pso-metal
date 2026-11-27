# P2 apply-8 — va_list supply into the collision pin (integrator, Scott auto-GO 2026-11-27)

Ask: P2-TL `docs/p2-tail-2.md` §2 (7 first-error TUs + 2 secondary at era
`os.h:40` `OSVReport`, whose `va_list` supplier — the suppressed `types.h:61`
libc leg — is gone by pin law; `compat/**` is integrator-owned, so it was a
supply ask).

**v1 (TLP's verbatim struct carry) CONVICTED within the wave**: two
occurrences of `typedef struct {…} __va_list[1]` are *distinct types* in
C++ (anonymous-struct uniqueness) — era `libc/stdarg.h:13-14` re-include
collides with the pin copy: 12 fixed, **18 BROKE** (all first-error
stdarg.h:14; includes a `.c` TU, mdm — clang's C handling of the array-typedef
chain diverged too). The "identical-typedef repeat is legal (WT precedent)"
assumption transferred wrongly: WT's dupes were *named* typedefs.

**v2 (landed)**: guard-**occupying** `_STDARG_H_` + host-ABI supply —
`typedef __builtin_va_list va_list;` + `va_*` → clang builtins. Safe because
(a) clang's own stdarg chains to the same underlying type (legal repeat),
(b) zero corpus refs to `__va_list`/`__va_arg`/`_var_arg_typeof`
(python-grepped), (c) the era macros expand `__builtin_va_info`, which is not
a clang builtin — *no currently-passing TU can have used them*, so
suppressing the era file loses nothing and the va_info pair itself flips free.

Battery (rc=0, LP64 macos12): **FAIL 517 → 513, PASS 1,163 → 1,167**.
FIXED (4): `CriSvm.cpp`, `LscLog.cpp` (the §2 va_info pair), `NetworkConnect.cpp`,
`Gcci.cpp`. BROKE: ∅. Family status: va wall dead everywhere; behind it
eth.c → `__lhbrx` (MWCC-asm dialect class), gjLight/nuReLoadTexture/
SystemBootInit/GinjaDrawUtil → `NJS_OBJECT_size` wall — **already booked in
the 485 adapter-queue count, no double-count: the +4 net already excludes
them** (they were fails before and remain fails). arm64_32 anchor untouched
(pin is LP64-leg-only by REBIND_FLAGS construction); floor 1,605 holds;
waiver header/pin `d4382fcd…` unmoved (no regen in this wave).

Next-wall ledger updates: TL §6 item 1 DONE; item 5's va_info half DONE
(CriSvm/LscLog flipped via supply, no shim needed); residual tail after
apply-8 = 513 − 485 (NJS) − 9 FloorPop − 2 MX = 17 named-tail.
