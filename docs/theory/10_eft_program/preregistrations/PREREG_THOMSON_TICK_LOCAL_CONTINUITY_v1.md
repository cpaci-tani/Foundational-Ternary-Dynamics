# PREREG: Source-Free Discrete Tick Local Continuity v1

**FTD ID:** FTD-0294
**Date:** 2026-06-13
**Status:** [PRE-REGISTRATION -- LOCKED/RUN]
**Parent:** FTD-0293 Source-free discrete tick energy invariant v2
**Engine artifact:** `engine/tests/campaign_thomson_tick_local_continuity.cpp`
**Artifact SHA256:** `6b137c83016b9aefb10d47d22df0094487ab761c06e167870a209004ada99aa3`
**Lock commit:** `7ebc236e`
**Lock tag:** `preregister-thomson-tick-local-continuity-v1`
**Run analysis:** `docs/theory/10_eft_program/ANALYSIS_THOMSON_TICK_LOCAL_CONTINUITY_v1.md`

---

## 1. Question

FTD-0293 confirmed the source-free global modified tick energy:

```text
H = 0.5 W^2 + 0.5 J K J - 0.5 W K J
```

FTD-0294 asks the local question:

```text
Does the same source-free tick energy satisfy an exact finite-volume
continuity identity over fixed spherical balls?
```

This is the local-current control required before adding state-coupling
source/work terms. It cannot derive `alpha`, a Thomson cross-section, or a
radiation amplitude.

---

## 2. Frozen Artifact

Run-of-record artifact:

```text
engine/tests/campaign_thomson_tick_local_continuity.cpp
```

Frozen source hash:

```text
6b137c83016b9aefb10d47d22df0094487ab761c06e167870a209004ada99aa3
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
| Particle | none |
| Wave | y-polarized flux/wave-velocity plane wave along x |
| Ball radii | `{5, 7, 9, 11, 13}` |
| Accumulation | long-double Kahan |
| Balance absolute gate | `1e-10` |
| Balance relative gate | `1e-12` |

Shared toggles:

| Toggle | Value |
|---|---|
| `wave_propagation` | `true` |
| `coupling` | `false` |
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

## 4. Frozen Local Density And Current

Let:

```text
K = -c^2 L
```

The frozen per-site density is:

```text
h_i = 0.5 |W_i|^2 + 0.5 J_i dot (KJ)_i - 0.5 W_i dot (KJ)_i
```

For one tick:

```text
W' = W - KJ
J' = J + W'
```

Algebra gives:

```text
Delta h_i
  = 0.5 [J_i dot (KW')_i - W'_i dot (KJ)_i]
  = sum_j 0.5 c^2 w_ij [W'_i dot J_j - J_i dot W'_j]
```

Therefore the frozen outward current from inside site `i` to outside site `j`
is:

```text
Phi_i->j = 0.5 c^2 w_ij [J_i(old) dot W_j(next) - W_i(next) dot J_j(old)]
```

The finite-volume identity is:

```text
Delta H_V + Phi_out(boundary V) = 0
```

where:

```text
H_V = sum_{i in V} h_i
```

---

## 5. Frozen Metrics

For each radius and tick interval:

- `Delta H_V`
- outward boundary flux
- inward/outward signed extrema
- balance residual `Delta H_V + Phi_out`
- relative balance residual
- RMS and mean absolute residual

---

## 6. Gates And Outcomes

The run is invalid if any metric is non-finite.

The local continuity law is confirmed if:

```text
max_abs_balance <= 1e-10
and
max_rel_balance <= 1e-12
```

Outcome labels:

```text
SOURCE_FREE_LOCAL_TICK_CONTINUITY_CONFIRMED
```

The finite-volume identity closes under the frozen gates.

```text
SOURCE_FREE_LOCAL_TICK_CONTINUITY_INVALIDATED
```

The finite-volume identity does not close under the frozen gates.

---

## 7. Non-Claims

This campaign does not:

- derive `alpha`;
- measure a Thomson cross-section;
- compute a QED scattering amplitude;
- claim radiation;
- include state-coupling source/work terms;
- scan amplitudes, modes, lattice sizes, gates, or initial conditions.

It only checks the source-free local current required before returning to the
charge-plus-beam recoil setup.

---

## 8. Frozen Commands

Build:

```sh
cmake --build engine/build --config Release --target campaign_thomson_tick_local_continuity --parallel 24
```

Run:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_tick_local_continuity$" --output-on-failure
```
