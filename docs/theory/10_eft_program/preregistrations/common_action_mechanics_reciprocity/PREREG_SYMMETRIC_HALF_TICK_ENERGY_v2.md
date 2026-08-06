# PRE-REGISTRATION — Symmetric half-tick transaction energy gate v2

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0469`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Supersedes:** `PREREG_SYMMETRIC_HALF_TICK_ENERGY_v1.md`  
**Parents:** `FTD-0293`, `FTD-0443`, `FTD-0452`, `FTD-0467`, `FTD-0468`  
**Engine artifact:** `engine/tests/campaign_symmetric_half_tick_energy.cpp`  
**Campaign SHA256 (v2):** `8FD735ABE1339A16888A4A133D852C509C8281C3319E00ED5F82B59884D4CCFD`  
**Helper SHA256 (unchanged):** `BE2EF960DBEE706EA28CC8F1D9E34F4592253B97B67C73834BA1F517ECA56031`

## 1. Change relative to v1, and why

v1 gated ENERGY residuals at `1e-12` ABSOLUTE. The v1 MSVC run
(`engine/results/ftd_0469/windows_msvc_cpu_v1_absolute_gate.csv`) returned
`SYMMETRIC_HALF_TICK_ENERGY_FAILS`: the pair-cubic static arms reached
`2.11e-12` absolute. Those arms carry `|E_shadow_0| = 56.17`, so the
excursion is `3.76e-14` RELATIVE, at the double-accumulation noise floor
established by FTD-0452 (`7.69e-14` relative over 64 ticks) and matched by
the independent replica (`6.4e-14`). The v1 gate compared an absolute
residual against ledgers fifty times larger than its implicit unit scale;
identical noise appears in the v1 production-ordering control.

v2 therefore gates energy residuals RELATIVE to
`max(1, |E_shadow_0|)` per arm (production control: `max(1,
|invariant_0|)`), still at `1e-12`. Momentum, reversal, and impulse-floor
gates remain absolute and unchanged. Fixtures, tick counts, identities
under test (H1-H5), and outcome mapping are unchanged from v1. The v1
verdict stands as recorded for the v1 gate definition; v2 is the
registered gate for the theorem claims, which are relative-scale
statements about exact identities, not absolute-noise statements.

v2 also separates stderr from the run record (the v1 CSV interleaved
RenderBridge constructor banners into stdout rows).

## 2. Run of record (executed)

- compiler: pinned MSVC `14.44.35207`, Release, Ninja Multi-Config via
  `engine/build_native.bat`
- backend: forced CPU (constructor banner reports CUDA availability;
  `backend_kind()` gate enforces CPU)
- record: `engine/results/ftd_0469/windows_msvc_cpu.csv`, SHA256
  `A8167B48C440AA8F9A737AC54777E3BBA3294BC49C0F040EBE87C89A6E9CF05D`
- focused CTest: `100% tests passed, 0 tests failed out of 1`
- verdict: `SYMMETRIC_HALF_TICK_SHADOW_ENERGY_EXACT`, `valid,true`
- summary: `worst_shadow_rel 7.53e-14`, `worst_naive_identity_rel
  7.53e-14`, `worst_total_momentum 1.83e-13`, `worst_reversal 3.89e-15`,
  `worst_production_invariant_rel 3.76e-14`, static records 3072, dynamic
  records 384
