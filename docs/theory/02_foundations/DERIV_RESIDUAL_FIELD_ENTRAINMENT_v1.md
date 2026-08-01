# FTD-0765 — Residual-field entrainment and the lag identity

**Status:** `[DERIVED OBSERVER IDENTITY] + [POST-HOC NUMERICAL FACT FROM LOCKED FTD-0764 ARTIFACTS] + [MATTER INTERPRETATION OPEN]`  
**Production status:** unchanged  
**New engine run:** none

## 1. Why the FTD-0764 wake label needs refinement

FTD-0764 measured a negative energy-centroid moment behind each moving core.
The preregistered predicate called this a `TRAILING_WAKE_CANDIDATE`. That
predicate detects lag in a chart transported with the core. It does not by
itself distinguish two physically different cases:

1. the moving core creates a new trailing excitation; or
2. an older residual cloud remains near its preparation site while the core
   moves away from it.

The locked rest/plus artifacts contain enough information to separate those
readings without rerunning the engine.

## 2. Windowed residual-energy centroid

Let `c(t)` be the measured matter center. FTD-0764 records the residual-field
energy and normalized first moment in the near and outer windows,

```text
E_N(t), mu_N(t),       r_infinity <= 8,
E_O(t), mu_O(t),       8 < r_infinity <= 48.              (1)
```

The combined relative centroid is

```text
mu_R(t) = [E_N mu_N + E_O mu_O] / [E_N + E_O].             (2)
```

The corresponding unwrapped absolute windowed centroid is

```text
X_R(t) = c(t) + mu_R(t).                                   (3)
```

All registered displacements are below one half-period, so this local unwrap
is unique. Equation (3) concerns only the selected residual energy inside
radius 48. It is not the centroid of the entire field and is conditional on
the FTD-0763 selected-bound subtraction.

## 3. Rest-subtracted entrainment

For the plus arm and its matched rest control define

```text
Delta X_R^mot(t)
 = [X_R^+(t)-X_R^+(t0)] - [X_R^0(t)-X_R^0(t0)],            (4)

Delta c(t) = c^+(t)-c^+(t0).                               (5)
```

For the registered direction `d_hat`, the finite-time entrainment fraction is

```text
epsilon_R(t)
 = [Delta X_R^mot(t) dot d_hat] / [Delta c(t) dot d_hat],   (6)
```

and the longitudinal lag is

```text
ell_R(t)
 = [Delta c(t)-Delta X_R^mot(t)] dot d_hat.                 (7)
```

`epsilon_R=1` would mean centroid entrainment at that checkpoint;
`epsilon_R=0` means that the measured residual-energy centroid has no net
motion after rest drift is removed. Neither value establishes shape
coherence.

## 4. Exact relation to the old wake moment

FTD-0764's `longitudinal_combined_moment` is precisely

```text
m(t) = mu_R(t) dot d_hat.                                  (8)
```

Substituting (3) into (4)--(7) gives the observer identity

```text
ell_R(t)
 = -[m^+(t)-m^+(t0)] + [m^0(t)-m^0(t0)].                  (9)
```

Thus the earlier trailing-moment predicate is a lag predicate by construction.
Its monotonic growth is not independent evidence that the dynamics creates a
new wake. A physical wake requires an additional discriminator, such as a
rest-subtracted trailing energy excess or a retarded field component whose
support, energy, and velocity scaling cannot be explained by an old
unentrained cloud.

## 5. Locked-artifact result

The independent FTD-0765 certificate evaluates (1)--(9) directly from the
final auditable FTD-0764 JSON files:

| ray | core displacement | rest-subtracted residual displacement | final entrainment | final lag |
|---|---:|---:|---:|---:|
| face | `0.4042465806` | `-0.0043098565` | `-0.0106614544` | `0.4085564371` |
| edge | `0.4000532621` | `0.0626338519` | `0.1565637825` | `0.3374194102` |
| body | `0.4156733218` | `0.0702433693` | `0.1689869558` | `0.3454299525` |

The lag identity closes within `5e-13` at all 12 moved checkpoints. No ray
entrains even 20% of the radius-48 residual-energy centroid by tick 224. The
finite-scale orientation spread exceeds `0.15`: face transport is slightly
anti-entrained while edge/body transport entrains about 16--17%.

The correct numerical statement is therefore:

```text
the manifested core moves substantially;
the registered residual-energy window remains mostly near its old location;
the former wake moment measures their growing separation.
```

## 6. Intuitive questions forced by the result

1. If a field cloud moves only 0--17% as far as the core, in what sense can it
   be counted as part of that moving object?
2. Is inertia the cost of continually abandoning and rebuilding a local field
   environment, rather than accelerating a rigidly dressed object?
3. Does an old field memory decay until a new co-moving profile dominates, or
   does the core indefinitely outrun its preparation field?
4. Why do edge and body motion entrain a similar fraction while face motion is
   slightly anti-entrained? Is that a cubic-stencil effect, a support-chart
   effect, or a property of the selected core geometry?
5. Does reversing velocity reverse only the lag sign, or does it reproduce the
   full residual-energy distribution by cubic/reflection covariance?
6. Is a genuine wake an excess field deposited per unit distance, while the
   present signal is merely a pre-existing cloud seen from a moving chart?
7. Does the matter momentum defect correlate with the rate of residual
   entrainment, the rate of Peierls-energy change, or neither?
8. Can a wider native carrier entrain more of its field environment and drive
   both the Peierls index and momentum defect toward zero?
9. Would a face/link connection variable supply a shared translation
   generator, or merely rename the same fixed-lattice reaction?
10. Is the durable object the ternary core alone, or a limit cycle in which
    old field is shed and new field is recruited at equal rates?

## 7. Matter ontology after FTD-0765

The strongest current ontology is not a rigid core-plus-aura composite.
Instead it has three empirically distinct layers:

1. **mobile manifested kernel:** the relational constituent core that moves;
2. **instantaneous constraint relation:** the state-selected bound/Gauss field,
   which changes with subcell chart phase and is not independently material;
3. **environmental field memory:** actual residual energy that evolves
   causally but is only weakly entrained over the registered horizon.

The third layer may later become part of a stationary moving eigenmode, a wake,
radiation, or discarded preparation history. FTD-0765 does not decide which.
It does show that attaching all visible field lines to the moving object is not
licensed.

A particle-grade matter state now requires more than a moving core. It needs a
late-time transported attractor: after preparation transients are removed,
its rest-subtracted residual profile must become stationary or periodic in the
moving chart, with controlled orientation dependence and a translation ledger
whose defect vanishes or scales away in the infrared.

## 8. Next discriminator

The next campaign must freeze preparation ages and signed boost amplitudes
before execution. For each age it must compare rest, plus, and minus arms and
record:

- windowed residual entrainment, not the raw lag alone;
- rest-subtracted trailing energy in slabs behind/ahead of the core;
- formation-front subtraction at fixed absolute coordinates;
- Peierls phase and energy change;
- matter, local-field, spline-field, and boundary-stress impulse ledgers.

Only an odd-in-velocity trailing excess that persists as preparation age
increases licenses a dynamical wake. Only a field/stress observable derived
independently from translation covariance may close momentum. Neither result
may be manufactured from the measured defect.

