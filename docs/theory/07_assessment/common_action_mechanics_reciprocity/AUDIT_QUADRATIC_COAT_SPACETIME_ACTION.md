# AUDIT — Quadratic-coat spacetime action

**Date:** 2026-07-26  
**Identifier:** `FTD-0542`  
**Status:** `[SELECTION — SMOOTH NON-CARDINAL COUPLING COAT]` +
`[THEOREM — EXACT SPACETIME CONTINUITY AND GAUGE-ENDPOINT IDENTITY]`  
**Verdict:** `QUADRATIC_COAT_SPACETIME_ACTION_CONSTRUCTIVE`  
**Pre-registration:**
[`PREREG_QUADRATIC_COAT_SPACETIME_ACTION_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_QUADRATIC_COAT_SPACETIME_ACTION_v1.md)  
**Derivation:**
[`DERIV_QUADRATIC_COAT_SPACETIME_ACTION.md`](../../10_eft_program/derivations/common_action_mechanics_reciprocity/DERIV_QUADRATIC_COAT_SPACETIME_ACTION.md)  
**Run of record:** `engine/results/ftd_0542/windows_msvc_cpu.json`

## Result

The FTD-0541 quadratic coat admits exact endpoint-weighted currents `K0,K1`
and temporal coat `T`. Their common interaction is

```text
S_int=g[<A0,K0>+<A1,K1>-h<Phi,T>].
```

All 24 locked polarity/path arms passed. Worst residuals were

```text
total-current split                  1.1102230246251565e-16
temporal partition                   2.2204460492503131e-16
split spacetime continuity           1.6653345369377348e-16
source variations                    3.4369208867790491e-17
gauge endpoint identity              2.0816681711721685e-17
electric invariance                  5.5511151231257827e-17
magnetic invariance                  1.0408340855860843e-17
curl-gradient identity               1.7347234759768071e-18
translation                          0
proper cubic rotation                1.3877787807814457e-17
path reversal                        2.0816681711721685e-17
```

The four-point quadrature is polynomial-exact on each knot interval. Invalid
polarity, duration, nonfinite field, and over-causal inputs fail closed.

## Scope

The missing common-action origin of FTD-0479 is repaired for the new smooth
coat representation. This does not reopen the closed FTD-0536 trilinear law.
No particle Legendre solve or exact energy law has been supplied. FTD-0543
proves why fixed-step action stationarity alone cannot supply the latter.

No production state, default, tick phase, force, toggle, scenario,
normalization, field ontology, or tolerance changed.

## Reproducibility

- test: `test_quadratic_coat_spacetime_action`, `24` arms, failures `0`;
- preregistration SHA256:
  `44840289D112095600C05F415EBED7FCA6F297D371AF3E7A164BCE390831D195`;
- test SHA256:
  `C75F032F3B35A27FBFF9C95BA7B177EDF327EEE7FB92C2B04F509C766112BAE5`;
- header SHA256:
  `9366AAF012BFF7E546DB6424E78099A914CF98213215F3A15840E209769BAFA2`;
- source SHA256:
  `9DC23CAAD86D3DC0C0C4375AB4291C293BEE4F66F38BDE47AE271A1AA4251D3A`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.
