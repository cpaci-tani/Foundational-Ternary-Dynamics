# FTD-0436 — Neutral-pair wave response

**Date:** 2026-07-24  
**Status:** `[MEASURED — SELECTED FLUX-GRADIENT FORCE]`  
**Verdict:** `COMMON_MODE_NEUTRAL_TRANSLATION`  
**Electric-force identification:** `[CLOSED NEGATIVE — NEUTRAL-PAIR GATE]`

## 1. Result

A mobile `+1/-1` pair was exposed to the same `+x`, `y`-polarized travelling
wave used by FTD-0435. Pair-only histories were subtracted sign by sign. Both
orientations passed every finite, survival, backend, toggle, response, and
exact-repeat gate.

The transverse common-mode fractions were

| initial pair axis | transverse common fraction | full-vector common fraction |
|---|---:|---:|
| `y` | `0.999999999999997` | `0.8088534216` |
| `z` | `0.9999880325` | `0.8090943437` |

The locked threshold for common-mode behavior was `0.80`. The result is not
near the boundary.

## 2. Physical interpretation

Writing the wave-induced forces as `Delta F_+` and `Delta F_-`, the campaign
measured

$$
F_C=\frac{\Delta F_++\Delta F_-}{2},\qquad
F_P=\frac{\Delta F_+-\Delta F_-}{2}.
$$

Almost the entire polarization-axis force lies in `F_C`. The neutral pair is
translated by the wave rather than polarized by opposite transverse forces.
That is incompatible with identifying the selected `G_C s grad|J|` channel as
ordinary electric `qE` coupling.

The result strengthens FTD-0435. Its same-direction isolated-polarity response
survives when both signs coexist and interact through one total field. The
supported description remains **self-field-mediated common wave force**.

## 3. Trajectory and survival

Both particle IDs survive all 200 ticks. The minimum separation remains `8`
to numerical precision in both registered orientations. After subtracting the
pair-only control, center-of-mass displacement dominates half the relative
displacement for both orientations.

This is not an atomic or bound-state result: no binding term is enabled and the
pair is only a neutral two-site probe.

## 4. Accounted energy

The normalized inclusion-exclusion residuals are
`7.3258e-7` and `6.9747e-7`, below the registered `1e-6` diagnostic threshold.
The label is `ACCOUNTED_ENERGY_CLOSED` for this finite protocol. Because the
current dynamic-energy audit omits interaction energies, this is an internal
accounting result rather than a general conservation theorem.

## 5. New control finding: isolated-pair self-translation

The registered pair-only controls expose a separate defect. With no incident
wave, both signs translate together by

$$
0.628381254469\ \text{lattice sites}
$$

along the oriented pair axis while maintaining separation. The same magnitude
appears for `y` and `z`, so it rotates cubically, but an isolated two-body
system has acquired center-of-mass motion from internal field forces.

This control finding is measured, not inferred. It was not a primary locked
outcome and is therefore not promoted to a final action-reaction verdict here.
A polarity-order mirror is required: swapping which end holds `+1` must reveal
whether the drift follows dipole orientation or a lattice/update-order bias.
Until that mirror is run, the selected force has no admissible momentum-
conservation or closed two-body mechanics claim.

## 6. Correct statement

> The selected flux-gradient force drives a neutral polarity pair almost
> entirely in transverse common mode under an incident travelling wave. It
> does not produce the opposite-sign transverse acceleration required for an
> electric-force interpretation. Its wave-free pair control also self-
> translates, leaving action-reaction balance unresolved and presumptively
> violated pending a polarity-order mirror.

No photon, pilot-wave, Thomson, atomic, gauge, conserved-charge, or empirical
normalization claim follows.

## 7. Artifacts

- preregistration:
  `docs/theory/10_eft_program/preregistrations/PREREG_NEUTRAL_PAIR_WAVE_RESPONSE_v1.md`
- campaign: `engine/tests/campaign_neutral_pair_wave_response.cpp`
- run record: `engine/results/ftd_0436/windows_msvc_cpu_L33.csv`
- manifest: `engine/results/ftd_0436/manifest.json`
- source SHA256:
  `a546e5ec38fd4cc0df9f13b0f367e83f35c278ed0d1f8f569b9b8d65fdf1654a`

