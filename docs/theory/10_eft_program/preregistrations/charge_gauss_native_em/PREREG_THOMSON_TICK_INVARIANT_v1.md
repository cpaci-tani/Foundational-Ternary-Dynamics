# PREREG: Source-Free Discrete Tick Energy Invariant v1

**FTD ID:** FTD-0292
**Date:** 2026-06-13
**Status:** [PRE-REGISTRATION -- LOCKED/RUN]
**Parent:** FTD-0291 Thomson native finite-volume continuity meter
**Engine artifact:** `engine/tests/campaign_thomson_tick_invariant.cpp`
**Artifact SHA256:** `5e6e2b77796d8a91f02bc7b2a85c9c862dd1f4e91b832be19ae5d5b41c455e16`
**Lock commit:** `87f0cda2`
**Lock tag:** `preregister-thomson-tick-invariant-v1`
**Run analysis:** `docs/theory/10_eft_program/charge_gauss_native_em/ANALYSIS_THOMSON_TICK_INVARIANT_v1.md`

---

## 1. Question

FTD-0291 invalidated the first native finite-volume graph-current candidate.
The failure happened on the free-wave beam-only control, before any recoil or
source claim could be promoted.

FTD-0292 asks the narrower source-free question:

```text
Does the actual source-free single-substrate engine tick preserve the modified
quadratic energy implied by its discrete update map?
```

This is an invariant/control campaign. It cannot derive `alpha`, a Thomson
cross-section, or a radiation amplitude.

---

## 2. Frozen Artifact

Run-of-record artifact:

```text
engine/tests/campaign_thomson_tick_invariant.cpp
```

Frozen source hash:

```text
5e6e2b77796d8a91f02bc7b2a85c9c862dd1f4e91b832be19ae5d5b41c455e16
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

The source-free update in the default single-substrate path is:

```text
W' = W + c^2 L J
J' = J + W'
```

Let:

```text
K = -c^2 L
delta = c^2 L J = -KJ
```

For each scalar eigenmode with stiffness `k`, the map is:

```text
[q']   [1-k  1] [q]
[w'] = [ -k  1] [w]
```

Solving `A^T M A = M` gives:

```text
M = [[k, -k/2],
     [-k/2, 1]]
```

Therefore the frozen global modified energy is:

```text
E_tick = 0.5 W^2 + 0.5 J K J - 0.5 W K J
       = 0.5 W^2 + E_grad + 0.5 W dot delta
```

The comparison observable is the older continuum-style energy:

```text
E_naive = 0.5 W^2 + E_grad
```

FTD-0292 checks whether `E_tick` is invariant while `E_naive` is allowed to
oscillate under the same tick.

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
cmake --build engine/build --config Release --target campaign_thomson_tick_invariant --parallel 24
```

Run:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_tick_invariant$" --output-on-failure
```
