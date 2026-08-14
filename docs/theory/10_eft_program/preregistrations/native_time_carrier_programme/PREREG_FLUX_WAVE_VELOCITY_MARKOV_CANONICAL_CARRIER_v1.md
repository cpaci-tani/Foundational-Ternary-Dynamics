# FTD-0876 — Flux/wave-velocity Markov canonical carrier v1

**Identifier:** `FTD-0876`  
**Date frozen:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Search policy:** exact symbolic algebra and source inspection only; no
near-miss, parameter, or coincidence search is permitted.

## 1. Question frozen before execution

Does the existing production pair `Voxel::flux` / `Voxel::wave_vel` supply the
minimum local canonical carrier type required by FTD-0875, through an exact
bijection with two consecutive flux slices and an exact symplectic free-wave
kick/drift? What production maps prevent promotion of that free-sector result
to the complete tick?

## 2. Frozen sources

| source | SHA256 |
|---|---|
| `THEOREM_LOCAL_CANONICAL_HAMILTONIAN_PARITY_RAIL_AND_SCALAR_LOCALITY_BOUNDARY_v1.md` | `982C3B9D00798920A1BDAB96C75EBC9DB3A08111E8900F1D630382B0249B25F6` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/src/render_bridge.cpp` | `BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/tests/test_leapfrog_integrator_audit.cpp` | `725B6B66FE8A83960E572332FDA6CE5E0021FBEA6389B465EEE647E364E0313C` |

## 3. Frozen mathematical class

For finite-dimensional real `V`, step `h>0`, and symmetric stiffness
`K=K^T`, test

```text
P+ = P - h K J
J+ = J + h P+
```

against the canonical form `J0=[[0,I],[-I,0]]`. Test the exact two-slice chart

```text
(J_(n-1),J_n) <-> (J_n,(J_n-J_(n-1))/h).
```

The minimum-carrier statement is scoped to the already-ratified FTD-0875
onsite-direct-sum class. The production facts are source facts, not inferred
from numerical trajectories.

## 4. Frozen certificate gates

The certificate must pass all of the following without source or tolerance
repair:

1. six frozen source hashes;
2. protocol self-hash;
3. exact history-chart inverse in symbolic dimension-one and exact rational
   finite-vector witnesses;
4. exact equivalence with the second-order recurrence;
5. exact kick and drift symplectic identities;
6. exact composed-map symplectic identity iff `K` is symmetric;
7. determinant one and exact inverse;
8. three componentwise canonical pairs per voxel;
9. scalar projection bracket and vector bond-generator reduction;
10. engine source markers for `flux`, `wave_vel`, kick, drift, damping,
    Langevin, Gauss placement, and genesis/evaporation;
11. exact damping pullback `rho^2 J0` and determinant;
12. nonidentity idempotent projection noninvertibility;
13. explicit distinction between exact symplectic preservation and exact
    naive-energy conservation;
14. scope markers excluding production parity actuation, `G*`, Born, Bell,
    Lorentz, and completeness claims; and
15. a terminal gate reached only if every preceding check passes.

## 5. Frozen outcomes

- **Outcome A — carrier-coordinate closure:** history/Markov equivalence,
  native stored pair, and free-wave symplecticity all pass. Retire only the
  coordinate-availability sub-debt; keep preparation, actuation, scale,
  constrained dynamics, loss, routing, and `G*` open.
- **Outcome B — conditional algebra only:** the abstract chart/map passes but
  the engine sources do not realize the declared pair/order. Book no native
  closure.
- **Outcome C — closed negative:** an exact algebraic gate fails. Preserve the
  attempt and do not repair after execution.

## 6. Banned promotions

The following inferences are forbidden:

- `wave_vel` storage implies a prepared ternary carrier;
- symplectic free wave implies symplectic complete production tick;
- symplecticity implies exact naive finite-step energy conservation;
- a canonical pair implies Hilbert space, quantization, or the Born rule;
- a harmonic reference phase implies the quartic `G*` calendar;
- an available vector component selects a physical route or axis; or
- reference compatibility implies whole-framework completeness.

## 7. Execution rule

The protocol SHA256 and frozen certificate SHA256 must be entered in
`REF_PREREGISTER_MANIFEST.md` as `LOCKED/PRE-RUN` before the first execution.
Any failed mathematical or source gate archives this attempt at its frozen
outcome; only a separately preregistered verifier-representation repair may
follow.
