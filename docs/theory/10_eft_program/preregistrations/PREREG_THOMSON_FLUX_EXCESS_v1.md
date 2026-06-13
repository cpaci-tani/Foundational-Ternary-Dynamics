# PREREG: Thomson Flux-Excess Discriminator v1

**FTD ID:** FTD-0289
**Date:** 2026-06-13
**Status:** [PRE-REGISTRATION -- LOCKED]
**Parent:** FTD-0288 Thomson unlocked recoil
**Engine artifact:** `engine/tests/campaign_thomson_flux_excess.cpp`
**Artifact SHA256:** `1f562ac9e9e0f3fdeb72bce00fda2c00f70117271439ef27d418d05c29ec7589`
**Lock tag:** `preregister-thomson-flux-excess-v1`

---

## 1. Question

FTD-0288 showed deterministic recoil in the native emergent flux-gradient
force path. The dashboard follow-up then clarified that the visually dominant
motion is the flux field itself, not primarily the carrier particle.

FTD-0289 asks the narrower baseline-subtracted question:

```text
After subtracting the free propagating wave and the charge-only field, does the
charge-plus-beam run contain an excess flux/wave residual above machine gate?
```

This is an instrument discriminator. It cannot derive alpha, a Thomson
cross-section, or a QED scattering amplitude.

---

## 2. Frozen Artifact

Run-of-record artifact:

```text
engine/tests/campaign_thomson_flux_excess.cpp
```

Frozen source hash:

```text
1f562ac9e9e0f3fdeb72bce00fda2c00f70117271439ef27d418d05c29ec7589
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
| Local residual radius | `8` voxels |
| Machine repeat gate | `1e-12` |
| Excess gate | `1e-8` |

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

The field/wave residual is frozen as:

```text
residual = charge_plus_beam - beam_only - charge_only
```

The subtraction is performed componentwise over all six field components:
`Jx,Jy,Jz,wave_vel_x,wave_vel_y,wave_vel_z`.

---

## 4. Frozen Metrics

For each arm:

- field/wave `L2`
- field energy, wave energy, particle kinetic energy, total energy
- total Poynting vector
- carrier displacement and velocity when a charge exists

For each residual:

- `l2`
- `rel_l2`
- `max_abs`
- `mean_abs`
- residual energy
- local residual energy within radius `8`
- local residual-energy centroid `(local_cx, local_cy, local_cz)`
- `transverse_centroid = sqrt(local_cy^2 + local_cz^2)`
- component norms `comp_x_l2`, `comp_y_l2`, `comp_z_l2`

---

## 5. Gates And Outcomes

The run is invalid if:

- any metric is non-finite;
- any repeat residual exceeds `1e-12`;
- the locked-linear residual has `max_abs > 1e-12` or `rel_l2 > 1e-12`.

Outcome labels:

```text
BASELINE_SUBTRACTION_INVALIDATED
```

The locked control is not linear at machine precision, so the subtraction
instrument cannot be trusted.

```text
NATIVE_LEGACY_EXCESS_FLUX_DEFLECTION_DETECTED
```

The production legacy force/movement path creates a baseline-subtracted field
residual above gate.

```text
NATIVE_EMERGENT_EXCESS_TRANSVERSE_FLUX_DEFLECTION_DETECTED
```

The legacy path stays below gate, while the native emergent flux-gradient path
creates an above-gate residual with the local residual centroid dominated by
the y/lateral component.

```text
NATIVE_EMERGENT_EXCESS_FLUX_DEFLECTION_DETECTED
```

The legacy path stays below gate, while the native emergent flux-gradient path
creates an above-gate residual, but the frozen transverse-centroid criterion is
not met.

```text
NO_BASELINE_SUBTRACTED_EXCESS_FLUX_DEFLECTION
```

No native path creates an above-gate residual.

---

## 6. Non-Claims

This campaign does not:

- derive `alpha`;
- measure a Thomson cross-section;
- compute a QED scattering amplitude;
- tune parameters;
- scan amplitudes, modes, lattice sizes, or gates.

It only asks whether the fixed FTD-0288 setup contains an excess flux/wave
residual after the two obvious baselines are subtracted.

---

## 7. Frozen Commands

Build:

```sh
cmake --build engine/build --config Release --target campaign_thomson_flux_excess --parallel 24
```

Run:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_flux_excess$" --output-on-failure
```
