# PREREG: Thomson Radiation Shell Meter v1

**FTD ID:** FTD-0290
**Date:** 2026-06-13
**Status:** [PRE-REGISTRATION -- LOCKED/RUN]
**Parent:** FTD-0289 Thomson flux-excess discriminator
**Engine artifact:** `engine/tests/campaign_thomson_radiation_shells.cpp`
**Artifact SHA256:** `a47de9c1bb52f92a6dc35471f4eba516fb76acaf9b66abd3de44dd6431d67edf`
**Lock commit:** `8ccfee7b`
**Lock tag:** `preregister-thomson-radiation-shells-v1`
**Run analysis:** `docs/theory/10_eft_program/ANALYSIS_THOMSON_RADIATION_SHELLS_v1.md`

---

## 1. Question

FTD-0289 showed that the native emergent flux-gradient channel produces an
above-gate residual field/wave response after subtracting the free-wave and
charge-only baselines.

FTD-0290 asks the narrower radiation question:

```text
Does that residual field carry outward Poynting flux through fixed spherical
shells around the charge?
```

This is an instrument discriminator. It cannot derive `alpha`, a Thomson
cross-section, or a QED scattering amplitude.

---

## 2. Frozen Artifact

Run-of-record artifact:

```text
engine/tests/campaign_thomson_radiation_shells.cpp
```

Frozen source hash:

```text
a47de9c1bb52f92a6dc35471f4eba516fb76acaf9b66abd3de44dd6431d67edf
```

Any source edit after this lock requires a v2 or a new FTD ID.

---

## 3. Fixed Protocol

Shared setup:

| Quantity | Value |
|---|---:|
| Lattice size | `L = 33` |
| Ticks | `200` |
| Plane-wave mode | `mode_n = 4` |
| Plane-wave amplitude | `0.05` |
| Particle | one negative charge at lattice center when the arm includes charge |
| Wave | y-polarized flux/wave-velocity plane wave along x |
| Shell radii | `{5, 7, 9, 11, 13, 15}` |
| Shell half-width | `0.5` lattice units |
| Machine repeat gate | `1e-12` |
| Outward power gate | `1e-8` |
| Angular structure gate | `1e-3` |

Shared toggles:

| Toggle | Value |
|---|---|
| `wave_propagation` | `true` |
| `coupling` | `true` |
| `damping` | `false` |
| `genesis` | `false` |
| `gauss_projection` | `false` |
| `gravity` | `false` |
| `poisson_coulomb` | `false` |
| `lorentz_force` | `false` |
| `dual_substrate` | `false` |
| `pair_production` | `false` |
| `symmetric_movement_order` | `false` |
| `strict_validation` | `true` |

Modes:

| Mode | `forces` | `movement` | `emergent_forces` | Charge locked? |
|---|---|---|---|---|
| `locked_linear` | `false` | `false` | `false` | yes |
| `native_legacy` | `true` | `true` | `false` | no |
| `native_emergent` | `true` | `true` | `true` | no |

For each mode, the campaign runs:

```text
beam_only
charge_only
charge_plus_beam
charge_plus_beam_repeat
```

The residual field is frozen as:

```text
J_res = J_charge_plus_beam - J_beam_only - J_charge_only
W_res = W_charge_plus_beam - W_beam_only - W_charge_only
```

The shell Poynting observable is then computed from the residual field itself:

```text
E_res = -W_res
B_res = curl(J_res)
S_res = E_res × B_res
P_out(R) = Σ_shell max(0, S_res · r_hat)
```

This deliberately avoids using `S(charge_plus_beam) - S(beam_only) -
S(charge_only)` because Poynting flux is nonlinear and would include linear
interference cross terms even when the field equations are exactly linear.

---

## 4. Frozen Metrics

For each shell:

- voxel sample count
- signed net radial shell power
- outward radial shell power
- inward radial shell power
- absolute radial shell power
- normalized dipole angular moment
- normalized quadrupole angular moment
- shell-summed residual Poynting momentum vector

For each mode:

- maximum absolute shell power
- maximum outward shell power
- maximum absolute net shell power
- strongest-shell dipole and quadrupole values
- deterministic repeat shell residuals

---

## 5. Gates And Outcomes

The run is invalid if:

- any metric is non-finite;
- any repeat shell residual exceeds `1e-12`;
- the locked-linear residual-field shell meter exceeds `1e-12`.

Outcome labels:

```text
BASELINE_RADIATION_METER_INVALIDATED
```

The locked-linear residual-field shell meter is not machine-zero, so the
instrument cannot be trusted.

```text
NATIVE_LEGACY_RESIDUAL_RADIATION_DETECTED
```

The production legacy force/movement path creates baseline-subtracted outward
residual shell power above gate.

```text
NATIVE_EMERGENT_STRUCTURED_RESIDUAL_RADIATION_DETECTED
```

The legacy path stays below gate, while the native emergent flux-gradient path
creates above-gate outward residual shell power with normalized quadrupole
structure above `1e-3`.

```text
NATIVE_EMERGENT_OUTWARD_RESIDUAL_POWER_DETECTED
```

The legacy path stays below gate, while the native emergent flux-gradient path
creates above-gate outward residual shell power, but the fixed angular
structure gate is not met.

```text
NO_BASELINE_SUBTRACTED_OUTWARD_POWER
```

No native path creates above-gate outward residual shell power.

---

## 6. Non-Claims

This campaign does not:

- derive `alpha`;
- measure a Thomson cross-section;
- compute a QED scattering amplitude;
- tune parameters;
- scan amplitudes, modes, lattice sizes, shell radii, or gates.

It only asks whether the fixed FTD-0288/0289 setup contains outward residual
Poynting flux through pre-declared shells.

---

## 7. Frozen Commands

Build:

```sh
cmake --build engine/build --config Release --target campaign_thomson_radiation_shells --parallel 24
```

Run:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_radiation_shells$" --output-on-failure
```
