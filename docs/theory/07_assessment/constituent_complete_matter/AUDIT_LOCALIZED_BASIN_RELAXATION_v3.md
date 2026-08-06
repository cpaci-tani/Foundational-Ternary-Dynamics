# AUDIT — Localized-basin relaxation v3

**Date:** 2026-07-28  
**Identifier:** `FTD-0681`  
**Status:** `[EXECUTION INVALID — REPLICATION GATE]`  
**Verdict:** `LOCALIZED_BASIN_RELAXATION_V3_EXECUTION_INVALID`

## Result

The corrected-output, corrected-decoder replication completed all 80 forward
and 80 reverse ticks for the control and both signs.  The target contract
residual is exactly zero.  The registered execution still fails because v3
required every corrected shell value to reproduce the value produced by the
known-wrong field-index decoder to relative `1e-12`.

The maximum pointwise residual, `0.987985`, occurs at tick 27 in a far-shell
value of order `1e-28`.  This relative comparison is ill-conditioned near
zero.  More fundamentally, a corrected decoder is allowed to change which
boundary cells belong to a shell when the instantaneous floating origin is
not exactly coordinate-symmetric.  The gate was false, not merely too tight.

The registered verdict remains execution-invalid.  No tolerance is relaxed
and no v3 physical verdict is promoted.

## Corrected raw facts

The run nevertheless supplies a complete corrected dataset:

- target energy/ratio contract residual: `0`;
- core history difference from FTD-0679: `0`;
- total difference-field history difference from FTD-0679: `0`;
- maximum shell-partition residual: `2.984e-26`;
- exact selected-energy drift: `1.066e-14`;
- maximum common-action residual: `7.605e-13`;
- state-only inverse residual: `1.108e-12` / `6.963e-13`.

An independent 690-check certificate recomputes the corrected target contract,
fits, shell closure, polarity comparison, and post-hoc classifier.  Those facts
are assessed separately in FTD-0682 and do not repair the v3 registration.

## Reproducibility

- protocol SHA256:
  `6E653BBD9D133F78ACE56E2E974EA322A275930C77E147478A8D4F31299D7E3A`;
- runner SHA256:
  `F3CDB0DABDE25899B648E7811B3712F7DE60543E587374EE4D72875718361388`;
- executed Release binary SHA256:
  `82078C827DE93514BDF6666B63E15B96A8EFA70D21DBA48E1EE961E5B9ECD91D`;
- JSON SHA256:
  `CD9EF8D29971F862EA26E0BF08DD64F9DFAF7BD70BA0631AEB706D0449F75EF7`;
- CSV SHA256:
  `53D723A466EC7861C74D096E8E31CB3BF8AD1F469431E360354CB599375CD0C8`;
- independent certificate SHA256:
  `F6084A7CB8F3E5F8F74F4DF4D732AA2A36FF5A7F04FAF0FB2EDB9ACF8358F357`.

No production state or behavior changed.
