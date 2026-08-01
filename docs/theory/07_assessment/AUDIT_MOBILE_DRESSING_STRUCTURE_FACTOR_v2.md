# Audit — mobile dressing structure factor v2

**Ledger ID:** FTD-0656  
**Verdict:** `MOBILE_DRESSED_STRUCTURE_FACTOR_V2_CONSTRUCTIVE`

FTD-0656 corrects only FTD-0655's contradictory tick-count clause before the
new runner was implemented. The protocol now consistently gives
`T_phys/a=32w`, and the complete 18-arm matrix was rerun from scratch. No
physics equation, normalization, observable, fit, threshold, or accepted root
changed.

All registered gates pass. The run contains exactly `32w+1` forward samples
per arm and complete state-only reverse histories. The independent certificate
reconstructs phase fits, amplitude CVs, centre velocity, relative-phase RMS,
mirror/cubic residuals, and refinement trends directly from the v2 series.

The qualified claim is classical co-motion of a selected constituent pattern
and its field-energy dressing. A retarded external perturbation, pole fit,
linewidth, residue, and spectral-positivity test remain necessary before a
matter-pole or particle statement.

## Reproducibility

- protocol SHA-256: `898AF1958713038FC945D09DD4DEA434A213BC6F79DE44006F64D35A208C99E3`
- runner SHA-256: `9CFAAF76171553AF19AE68367EE2A6F6999C71062E9B86857742B53828D62695`
- JSON SHA-256: `FCEFF8B0162D131D0FE380635A4CA516168782EC793B417C337D46BCB0B86717`
- arms CSV SHA-256: `38668AA76D59DBF3F98B932D8784107257204D5ADBB889CB8AFCAECE8CCF92B0`
- series CSV SHA-256: `08EF4B4127F51863EB9A6314E4FB144187800A050AB0D5F59F73487DBBD7F336`
- independent certificate SHA-256: `BF81B6157587D3D832EDE85237855F7DF3E2C2FA6D1619D54B6B2A9A5ECA289F`
