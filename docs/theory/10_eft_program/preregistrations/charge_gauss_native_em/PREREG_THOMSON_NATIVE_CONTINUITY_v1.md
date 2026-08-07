# PREREG: Thomson Native Finite-Volume Continuity Meter v1

**FTD ID:** FTD-0291
**Date:** 2026-06-13
**Status:** [PRE-REGISTRATION -- LOCKED/RUN]
**Parent:** FTD-0290 Thomson radiation shell meter
**Engine artifact:** `engine/tests/campaign_thomson_native_continuity.cpp`
**Artifact SHA256:** `357a2a2b4bd7fb8d8604a4c30490f68ab9a404e8574ed6e55b034056a5b3f3e8`
**Lock commit:** `47ccbee4`
**Lock tag:** `preregister-thomson-native-continuity-v1`
**Run analysis:** `docs/theory/10_eft_program/charge_gauss_native_em/ANALYSIS_THOMSON_NATIVE_CONTINUITY_v1.md`

---

## 1. Question

FTD-0290 found no above-gate outward residual Poynting power in the frozen
Thomson recoil setup. That result used an imported electromagnetic diagnostic:

```text
S = E x B, E = -W, B = curl(J)
```

FTD-0291 asks a narrower native question:

```text
Does the same setup close a finite-volume graph-energy continuity balance
using the 18-neighbor stencil current implied by the engine wave operator?
```

This is an accounting-law discriminator. It cannot derive `alpha`, a Thomson
cross-section, or a QED scattering amplitude.

---

## 2. Frozen Artifact

Run-of-record artifact:

```text
engine/tests/campaign_thomson_native_continuity.cpp
```

Frozen source hash:

```text
357a2a2b4bd7fb8d8604a4c30490f68ab9a404e8574ed6e55b034056a5b3f3e8
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
| Ball radii | `{5, 7, 9, 11, 13}` |
| Machine repeat gate | `1e-12` |
| Balance absolute gate | `1e-8` |
| Balance relative gate | `1e-6` |
| Graph outward-flux gate | `1e-8` |

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
| `weak_transmutation` | `false` |
| `symmetric_movement_order` | `false` |
| `strict_validation` | `true` |

Modes:

| Mode | `forces` | `movement` | `emergent_forces` | Charge locked? |
|---|---|---|---|---|
| `locked_linear` | `false` | `false` | `false` | yes |
| `native_legacy` | `true` | `true` | `false` | no |
| `native_emergent` | `true` | `true` | `true` | no |

For each mode, the campaign runs lockstep arms:

```text
beam_only
charge_only
charge_plus_beam
charge_plus_beam_repeat
```

The baseline-subtracted residual field is frozen as:

```text
J_res = J_charge_plus_beam - J_beam_only - J_charge_only
W_res = W_charge_plus_beam - W_beam_only - W_charge_only
```

The repeat field is frozen as:

```text
J_repeat = J_charge_plus_beam - J_charge_plus_beam_repeat
W_repeat = W_charge_plus_beam - W_charge_plus_beam_repeat
```

---

## 4. Frozen Native Observable

The campaign uses the engine's 18-neighbor Laplacian graph:

```text
face-edge weight: w = 1/3
edge-diagonal weight: w = 1/6
c = C_WAVE = 1/sqrt(3)
```

For a finite ball `V`, graph energy is endpoint-attributed:

```text
E_V = sum_{i in V} 0.5 |W_i|^2
    + sum_{i in V} sum_{j~i} 0.25 c^2 w_ij |J_j - J_i|^2
```

For every stencil edge crossing from inside `i` to outside `j`, the frozen
candidate outward current is:

```text
F_i->j = -c^2 w_ij 0.5 (W_i + W_j) dot (J_j - J_i)
```

The per-step balance is:

```text
balance = Delta E_V + sum_boundary F_i->j
```

This is a candidate native continuity meter. If the free-wave beam-only arm
does not close under the fixed gates, the candidate is invalidated rather than
promoted.

---

## 5. Frozen Metrics

For each ball and each tick interval:

- graph-energy change `Delta E_V`
- signed boundary graph flux
- outward graph flux
- inward graph flux
- balance residual `Delta E_V + flux`
- relative balance residual

For each mode:

- beam-only graph-balance summary
- baseline-subtracted residual graph-balance summary
- repeat graph-balance summary

---

## 6. Gates And Outcomes

The run is invalid if:

- any metric is non-finite;
- any repeat balance, repeat flux, or repeat energy change exceeds `1e-12`;
- the locked-linear residual balance, flux, or energy change exceeds `1e-12`.

The native current candidate is invalidated if:

```text
max_abs_balance > 1e-8
or
max_rel_balance > 1e-6
```

on any beam-only free-wave arm.

Outcome labels:

```text
NATIVE_GRAPH_CONTINUITY_CANDIDATE_INVALIDATED
```

The fixed 18-neighbor graph-current candidate does not close the free-wave
finite-volume balance under the pre-declared gates.

```text
NATIVE_LEGACY_GRAPH_FLUX_OR_SOURCE_DETECTED
```

The free-wave candidate closes, but the production legacy force/movement path
creates above-gate residual graph flux or source-like balance.

```text
NATIVE_EMERGENT_OUTWARD_GRAPH_FLUX_DETECTED
```

The legacy path stays below gate, while the native emergent flux-gradient path
creates above-gate outward graph flux.

```text
NATIVE_EMERGENT_LOCAL_SOURCE_WITHOUT_OUTWARD_FLUX
```

The legacy path stays below gate, and the native emergent path creates an
above-gate local/source-like balance residual without above-gate outward graph
flux.

```text
NO_NATIVE_GRAPH_RADIATION_OR_SOURCE_ABOVE_GATE
```

No native path creates above-gate outward graph flux or source-like residual
under the fixed protocol.

---

## 7. Non-Claims

This campaign does not:

- derive `alpha`;
- measure a Thomson cross-section;
- compute a QED scattering amplitude;
- tune parameters;
- scan amplitudes, modes, lattice sizes, ball radii, or gates;
- assert that the candidate current is the final native continuity theorem
  unless the free-wave control closes under the frozen gates.

---

## 8. Frozen Commands

Build:

```sh
cmake --build engine/build --config Release --target campaign_thomson_native_continuity --parallel 24
```

Run:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_native_continuity$" --output-on-failure
```
