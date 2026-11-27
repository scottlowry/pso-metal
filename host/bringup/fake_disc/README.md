# fake_disc — synthetic DVD bundle (P2-Q, no ISO access)

Files here are the entire 'disc' the dvd_shim mounts (root via $PSO_FAKE_DISC).
- logo_e.prs: 9-byte PRS stream hand-constructed from build/GPOE8P/asm/pso/system/prs.s
  (control 0xAF = literals 'ABCD' + far-token 0xE3/0xFF back-ref + 0x00 0x00 END);
  decodes to "ABCDABCDA". Stand-in for the TLogo logo_e PRS texture (ledger step 19).
- title.cfg: placeholder file for open/read smoke.
- gsl_tab.dat: 3x GslArchiveEntry (0x30 stride, era GslArchive.h layout) — P2-AE rung-3 GSL table stand-in.
- title_prs.dat: 9-byte G3 PRS stream hand-trace (== logo_e.prs bytes), title.cfg stand-in read by find_gsl_entry offset/size fields.
