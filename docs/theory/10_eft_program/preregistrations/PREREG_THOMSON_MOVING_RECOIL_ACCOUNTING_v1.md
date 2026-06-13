# PREREG: Thomson Moving-Recoil Source/Work Accounting v1

**FTD ID:** FTD-0297
**Date:** 2026-06-13
**Status:** [PRE-REGISTRATION -- LOCKED]
**Parent:** FTD-0296 Fixed-charge coupled tick source/work continuity
**Engine artifact:** `engine/tests/campaign_thomson_moving_recoil_accounting.cpp`
**Artifact SHA256:** `aae604ea897943102273f89b819735283804474d26a2a531f769835dc46f5c89`
**Lock commit:** pending pre-run commit
**Lock tag:** `preregister-thomson-moving-recoil-accounting-v1`
**Run analysis:** pending

---

## 1. Question

FTD-0296 confirmed that a fixed charge with coupling on and movement off obeys
the finite-volume source/work identity:

```text
Delta H_V + Phi_out_source_free - Work_V = 0
```

FTD-0297 asks the next moving-source question:

```text
When the charge is unlocked, do native legacy and native emergent recoil modes
still close this source/work balance after the full engine tick?
```

This separates subvoxel recoil from integer transport. In the engine,
`phase_forces` changes particle velocity, while `phase_movement` accumulates
subvoxel remainder until an integer hop occurs. Only an integer hop carries
particle state and self-field flux to a new voxel. Therefore the frozen
discriminator is:

- if moving recoil occurs but no integer transport occurs, the additive
  source/work law may still close;
- if integer transport occurs, any above-gate balance residual is post-write
  transport work still requiring a separate accounting law.

This campaign cannot derive `alpha`, a Thomson cross-section, a QED amplitude,
or radiation.

---

## 2. Frozen Artifact

Run-of-record artifact:

```text
engine/tests/campaign_thomson_moving_recoil_accounting.cpp
```

Frozen source hash:

```text
aae604ea897943102273f89b819735283804474d26a2a531f769835dc46f5c89
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
| Particle | one negative charge at lattice center |
| Wave | y-polarized flux/wave-velocity plane wave along x |
| Ball radii | `{5, 7, 9, 11, 13}` |
| Accumulation | long-double Kahan |
| Balance absolute gate | `1e-10` |
| Balance scale-relative gate | `1e-12` |
| Repeat determinism gate | `1e-12` |
| Recoil gate | `1e-8` |

Modes:

| Mode | `forces` | `movement` | `emergent_forces` | Charge lock |
|---|---|---|---|---|
| `locked_fixed_source` | `false` | `false` | `false` | locked |
| `native_legacy_unlocked` | `true` | `true` | `false` | unlocked |
| `native_emergent_unlocked` | `true` | `true` | `true` | unlocked |

For each mode, the campaign runs:

```text
charge_only
charge_plus_beam
charge_plus_beam_repeat
```

All other relevant toggles are frozen:

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
| `symmetric_movement_order` | `false` |
| `symplectic_leapfrog` | `false` |
| `strict_validation` | `true` |

---

## 4. Frozen Identity

For each tick the meter captures the pre-tick source:

```text
S = G_C (grad_state + curl_state_velocity)
```

and tests the FTD-0296 additive source/work law against the full post-tick
field:

```text
W* = W - KJ
W' = W* + S
J' = J + W'
```

with modified tick energy:

```text
H = 0.5 W^2 + 0.5 J K J - 0.5 W K J
```

and work density:

```text
Work_i = W*_i dot S_i + 0.5 |S_i|^2 + 0.5 J_i dot (K S)_i
```

The measured residual is:

```text
R_V = Delta H_V + Phi_out_source_free(boundary V) - Work_V
```

where `Phi_out_source_free` is the FTD-0295 boundary current evaluated with
`W*`. In fixed-source mode `R_V` should close to roundoff. In moving modes,
above-gate `R_V` is classified as unaccounted post-write recoil/transport work.

---

## 5. Frozen Metrics

For each mode, arm, radius, and tick interval:

- `Delta H_V`;
- source-free outward boundary flux;
- source/work term;
- balance residual `R_V`;
- scale-relative balance residual;
- RMS and mean absolute residual.

For each mode and arm:

- final charge position;
- integer hop totals;
- number of transport events;
- displacement including subvoxel remainder;
- velocity, speed, max speed, and max acceleration.

For each mode:

- repeat delta for `charge_plus_beam`;
- extra motion delta `charge_plus_beam - charge_only`.

---

## 6. Gates And Outcomes

The run is invalid if any metric is non-finite.

The repeat determinism gate passes if every repeat delta is:

```text
max_abs <= 1e-12
```

The fixed-source accounting control passes if locked `charge_only` and
`charge_plus_beam` both satisfy:

```text
max_abs_balance <= 1e-10
and
max_scale_rel_balance <= 1e-12
```

Moving-mode accounting passes under the same balance gates for native legacy
and native emergent modes.

Outcome labels:

```text
SUBVOXEL_RECOIL_ACCOUNTED_BY_ADDITIVE_SOURCE_WORK
```

At least one moving mode shows above-gate extra motion, no integer transport
events occur, and the source/work balance closes.

```text
MOVING_RECOIL_ACCOUNTED_THROUGH_TRANSPORT_EVENT
```

At least one moving mode shows above-gate extra motion, at least one integer
transport event occurs, and the source/work balance still closes.

```text
MOVING_TRANSPORT_RESIDUAL_DETECTED
```

The locked control passes, but a moving mode fails the source/work balance.
This is a classified measurement outcome, not a protocol failure.

```text
MOVING_SOURCE_ACCOUNTING_CONFIRMED_NO_EXTRA_RECOIL
```

All accounting gates pass, but no moving mode shows above-gate extra motion.

Protocol failure labels:

```text
NONFINITE_PROTOCOL
NONDETERMINISTIC_PROTOCOL
LOCKED_FIXED_SOURCE_ACCOUNTING_FAILED
```

---

## 7. Non-Claims

This campaign does not:

- derive `alpha`;
- measure or derive a Thomson cross-section;
- compute a QED scattering amplitude;
- claim radiation;
- scan amplitudes, modes, lattice sizes, gates, or initial conditions;
- prove the full moving-source recoil theorem if no integer transport event
  occurs.

It only measures whether the already-derived additive source/work law remains
sufficient for the fixed unlocked recoil protocol.

---

## 8. Frozen Commands

Build:

```sh
cmake --build engine/build --config Release --target campaign_thomson_moving_recoil_accounting --parallel 24
```

Run:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_moving_recoil_accounting$" --output-on-failure
```
