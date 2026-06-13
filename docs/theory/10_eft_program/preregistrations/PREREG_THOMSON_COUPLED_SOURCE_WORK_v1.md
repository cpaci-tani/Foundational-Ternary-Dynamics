# PREREG: Fixed-Charge Coupled Tick Source/Work Continuity v1

**FTD ID:** FTD-0296
**Date:** 2026-06-13
**Status:** [PRE-REGISTRATION -- LOCKED/PENDING RUN]
**Parent:** FTD-0295 Source-free discrete tick local continuity v2
**Engine artifact:** `engine/tests/campaign_thomson_coupled_source_work.cpp`
**Artifact SHA256:** `95747a57895973577e0054d075752b79e74173507097652e31498b125d7ec88e`
**Lock commit:** pending pre-run lock commit
**Lock tag:** `preregister-thomson-coupled-source-work-v1`
**Run analysis:** `docs/theory/10_eft_program/ANALYSIS_THOMSON_COUPLED_SOURCE_WORK_v1.md`

---

## 1. Question

FTD-0295 confirmed the source-free local continuity law for the actual
discrete tick:

```text
Delta H_V + Phi_out = 0
```

FTD-0296 asks the next coupled question in the simplest isolated setting:

```text
With one fixed charge, coupling on, and movement off, does the same native
tick energy close as boundary flux plus an exact state-flux source/work term?
```

This is the fixed-source work control required before returning to the unlocked
charge-plus-beam recoil setup. It cannot derive `alpha`, a Thomson
cross-section, or a radiation amplitude.

---

## 2. Frozen Artifact

Run-of-record artifact:

```text
engine/tests/campaign_thomson_coupled_source_work.cpp
```

Frozen source hash:

```text
95747a57895973577e0054d075752b79e74173507097652e31498b125d7ec88e
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
| Particle | one locked negative charge at lattice center |
| Wave | y-polarized flux/wave-velocity plane wave along x |
| Ball radii | `{5, 7, 9, 11, 13}` |
| Accumulation | long-double Kahan |
| Balance absolute gate | `1e-10` |
| Balance scale-relative gate | `1e-12` |

Toggles:

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
| `weak_transmutation` | `false` |
| `forces` | `false` |
| `movement` | `false` |
| `emergent_forces` | `false` |
| `symmetric_movement_order` | `false` |
| `symplectic_leapfrog` | `false` |
| `strict_validation` | `true` |

---

## 4. Frozen Coupled Identity

The update is:

```text
W* = W + c^2 L J
W' = W* + S
J' = J + W'
```

where the frozen source is the production source term:

```text
S = G_C (grad_state + curl_state_velocity)
```

For the same modified tick energy:

```text
H = 0.5 W^2 + 0.5 J K J - 0.5 W K J
```

the exact source/work density is:

```text
Work_i = W*_i dot S_i + 0.5 |S_i|^2 + 0.5 J_i dot (K S)_i
```

The finite-volume identity is:

```text
Delta H_V + Phi_out_source_free(boundary V) - Work_V = 0
```

where `Phi_out_source_free` is the FTD-0295 boundary current evaluated with
`W*`, not with the source-updated `W'`.

---

## 5. Frozen Metrics

For each radius and tick interval:

- `Delta H_V`
- source-free outward boundary flux
- source/work term
- balance residual `Delta H_V + Phi_out - Work_V`
- scale-relative balance residual
- RMS and mean absolute residual

---

## 6. Gates And Outcomes

The run is invalid if any metric is non-finite.

The fixed-source work law is confirmed if:

```text
max_abs_balance <= 1e-10
and
max_scale_rel_balance <= 1e-12
```

Outcome labels:

```text
FIXED_CHARGE_SOURCE_WORK_CONTINUITY_CONFIRMED
```

The finite-volume source/work identity closes under the frozen gates.

```text
FIXED_CHARGE_SOURCE_WORK_CONTINUITY_INVALIDATED
```

The finite-volume source/work identity does not close under the frozen gates.

---

## 7. Non-Claims

This campaign does not:

- derive `alpha`;
- measure a Thomson cross-section;
- compute a QED scattering amplitude;
- claim radiation;
- include particle movement or recoil;
- scan amplitudes, modes, lattice sizes, gates, or initial conditions.

It only checks fixed-source work accounting before returning to unlocked recoil.

---

## 8. Frozen Commands

Build:

```sh
cmake --build engine/build --config Release --target campaign_thomson_coupled_source_work --parallel 24
```

Run:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_coupled_source_work$" --output-on-failure
```
