# compat/facade — P2-D4 era declaration supply for the rebind facade world

Lane D4 (card `t_075ab4e1`), wave-2 2026-11-27. INERT SUPPLY: this directory
is on NO `-I` line (build/, host/, data/ greps prove it). It shadows era
headers BY `-I` PATH ONLY — nothing here is ever `-include`'d (collision law,
docs/p2-collision-debt.md §5; repin/net/musyx/njs precedent).

Two declaration headers, both re-exporting the host substrate contract
(`host/bringup/pso_bringup.h`, quoted-relative include — the exact
`host/bringup/include/dolphin/os/OSAlloc.h` pattern) and topping it with the
era decl set the victims reach, signatures line-cited to decomp
`pso-decomp @6b38b5383` `libs/dolphin/include/dolphin/`. Declarations only —
zero definitions, zero struct reinterpretation; implementations stay in the
host seat (osalloc_era.c etc.). `-I` ask + splice position + measurement:
docs/p2-d4facade-1.md.

    dolphin/os/OSAlloc.h   era tail missing from the bringup shadow:
                           OSAlloc/OSFree macro pair + 6 orphan decls
    dolphin/os/OSTime.h    era OSTime/OSTick/OSCalendarTime decl set,
                           era guard OS_TIME_H occupied so every edge skips
                           the colliding original (OSTime.h:15 s64-vs-u64)
