# PREREG: Source-Free Discrete Tick Energy Invariant v2

**FTD ID:** FTD-0293
**Date:** 2026-06-13
**Status:** [PRE-REGISTRATION -- LOCKED/RUN]
**Parent:** FTD-0292 Source-free discrete tick energy invariant v1
**Engine artifact:** `engine/tests/campaign_thomson_tick_invariant_v2.cpp`
**Artifact SHA256:** `c362d35e1a2c61216982bb7ae2c8cf4ee916e59f1e3bcc77a62cee993caa8b5f`
**Lock commit:** `83863d5e`
**Lock tag:** `preregister-thomson-tick-invariant-v2`
**Run analysis:** `docs/theory/10_eft_program/charge_gauss_native_em/ANALYSIS_THOMSON_TICK_INVARIANT_v2.md`

---

## 1. Question

FTD-0292 v1 used the modified energy implied by the source-free tick but
missed its predeclared relative gate under ordinary double accumulation:

```text
max_abs_modified_drift = 2.11e-11
max_rel_modified_drift = 2.55e-12
relative gate = 1e-12
```

FTD-0293 asks the same invariant question with precision-controlled
accumulation:

```text
Does the source-free single-substrate engine tick preserve the modified
quadratic energy when the measurement uses long-double Kahan summation?
```

This is not a gate relaxation. The update, initial condition, invariant
formula, and gates remain unchanged from v1.

---

## 2. Frozen Artifact

Run-of-record artifact:

```text
engine/tests/campaign_thomson_tick_invariant_v2.cpp
```

Frozen source hash:

```text
c362d35e1a2c61216982bb7ae2c8cf4ee916e59f1e3bcc77a62cee993caa8b5f
```

Any source edit after this lock requires another version or a new FTD ID.

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
| Accumulation | long-double Kahan |
| Modified-energy absolute gate | `1e-10` |
| Modified-energy relative gate | `1e-12` |
| Naive-energy drift visibility gate | `1e-6` |

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

## 4. Frozen Invariant

The source-free update is:

```text
W' = W + c^2 L J
J' = J + W'
```

Let:

```text
K = -c^2 L
delta = c^2 L J = -KJ
```

The frozen modified energy is unchanged from v1:

```text
E_tick = 0.5 W^2 + 0.5 J K J - 0.5 W K J
       = 0.5 W^2 + E_grad + 0.5 W dot delta
```

The comparison energy is also unchanged:

```text
E_naive = 0.5 W^2 + E_grad
```

---

## 5. Frozen Metrics

The campaign records:

- kinetic energy
- graph-gradient energy
- cross term `0.5 W dot c^2 L J`
- naive energy
- modified tick energy
- maximum absolute and relative drift of naive energy
- maximum absolute and relative drift of modified tick energy
- tick locations of maximum drift

---

## 6. Gates And Outcomes

The run is invalid if any metric is non-finite.

The modified invariant is confirmed if:

```text
max_abs_modified_drift <= 1e-10
and
max_rel_modified_drift <= 1e-12
```

Outcome labels:

```text
DISCRETE_TICK_MODIFIED_ENERGY_CONFIRMED
```

The modified energy closes under gate, while the naive continuum-style energy
drifts above `1e-6`.

```text
DISCRETE_TICK_MODIFIED_ENERGY_CONFIRMED_NAIVE_QUIET
```

The modified energy closes under gate, but the naive energy does not drift
above the visibility gate in this specific initial condition.

```text
DISCRETE_TICK_INVARIANT_INVALIDATED
```

The modified energy does not close under the pre-declared gates.

---

## 7. Non-Claims

This campaign does not:

- derive `alpha`;
- measure a Thomson cross-section;
- compute a QED scattering amplitude;
- claim radiation;
- scan amplitudes, modes, lattice sizes, gates, or initial conditions;
- prove the local finite-volume current.

It only checks the source-free global invariant needed before a local native
continuity theorem can be derived.

---

## 8. Frozen Commands

Build:

```sh
cmake --build engine/build --config Release --target campaign_thomson_tick_invariant_v2 --parallel 24
```

Run:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_tick_invariant_v2$" --output-on-failure
```
