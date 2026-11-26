# INTEGRATOR HISTORY NOTE — 2026-11-26 ~15:40

- Commit 326d960 (gen_waivers deny-list) was authored while the TSV fix targeted a wrong column; its message claimed 'TSV rows corrected' prematurely. The correction landed one command later but was amend-absorbed into commit 3421b91 (lane P's init-ledger commit) because sibling lanes commit to main concurrently — 3421b91 therefore contains BOTH lane P's docs/p1-init-ledger.md AND the integrator's data/assert-verdicts.tsv + regenerated build/waivers.
- Content state after 3421b91 is fully verified: deny-list active (99/3317 waived, zero FORMAT roots), TSV labels corrected, make parse PASS=1416/1680, parse_gate exit 0.
- Standing rule adopted: with lanes live, integrator NEVER amends on pso-metal; follow-up commits only.
