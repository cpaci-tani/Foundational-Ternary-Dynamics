# FTD-0739 — Finite-support outgoing-tail formation v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date:** 2026-07-29  
**Parents:** FTD-0736--0738  
**Scope:** observer-only selected dynamics; no production rule, potential,
field equation, force, coefficient, state type, scenario, or default changes

## 1. Question

FTD-0737 establishes negative-core formation through the last tick before a
newly emitted disturbance can contact a periodic image, but its tick-zero
electric field is a quotient-wide minimum-energy Poisson dress. Replace that
global preparation with an exactly Gauss-compatible field whose support is a
finite cube around the neutral pair. Does the same selected matter--current--
field action form and retain a negative relational core while a source-free
field tail crosses two exterior shells?

This campaign tests autonomous local preparation, not production adoption or
an asymptotic particle.

## 2. Locked compact preparation

For a neutral two-constituent quadratic-coat density `rho`, take the integer
site nearest the pair centroid and the induced axis-edge graph on the centered
Chebyshev cube of half-width

```text
R0 = 4.
```

All face flux crossing the cube boundary is fixed to zero. Among internal face
fields satisfying `div(E)=rho`, select the unique minimum of

```text
(1/2) sum_faces E_f^2.
```

Equivalently, solve the zero-mean graph-Poisson equation on the induced cube
and set each internal oriented face to the potential difference across that
edge. Outside and boundary-crossing faces are exactly zero; the initial edge
magnetic field is zero.

The implementation must expose the preparation as an observer/research API,
not as a production initializer. It must prove or directly check:

- neutral compatibility and complete containment of the coat density;
- exact compact support and zero boundary-crossing flux;
- Gauss residual `<=1e-12` and local Poisson residual `<=1e-13`;
- uniqueness of the face field under the strictly convex constrained energy;
- integer-translation, polarity-conjugation, and proper cubic covariance to
  `1e-11`;
- no hidden route order, graph edge, exterior field, or persistent history.

The field need not have zero full-periodic curl-adjoint. A generic compact
Gauss field for this density is not the quotient-wide longitudinal minimum;
its boundary transverse content is part of the selected causal preparation
and must be measured, not projected away.

## 3. Locked dynamical matrix

- periodic computational quotient `L=145`;
- horizon `T=136`, followed by 136 state-only inverse ticks;
- `dt=1/4`, wave speed `C_SPEED`, compact-pair depth `D=0.01`, cutoff squared
  `3/2`, exact sparse current, local residual evaluation, solve tolerance
  `2e-14`, gate tolerance `1e-10`, and 384 nonlinear iterations;
- unbound separation `1.30`, opposing momentum magnitude `0.0120`;
- bound control separation `1.00`, opposing momentum magnitude `0.0150`;
- no damping, reaction, collision, absorption, legacy force, global redress,
  field rescaling, post-hoc correction, or parameter retuning.

Run five complete histories:

1. unbound `plus_minus` face `<001>`;
2. unbound `plus_minus` edge `<01-1>`;
3. unbound `plus_minus` body `<111>`;
4. unbound `minus_plus` body `<111>` conjugation control;
5. initially bound `plus_minus` face control.

Persist tick zero, every forward state, every reverse root, every graph
transition, pair and field energy, current support, and both regional ledgers.
The expected row count is `5*(1+136+136)=1365`.

## 4. Causal and regional field gates

The initial field support has conservative Chebyshev radius `R0=4`. The locked
earliest possible periodic self-contact is therefore

```text
T_contact = L - 2 R0 = 145 - 8 = 137.
```

The run ends at tick 136. Deposited current must remain within radius three;
otherwise the execution is invalid.

At every forward step evaluate the exact FTD-0671 regional modified-energy
ledger about the fixed pair centroid at radii 8 and 12. Reconstruct the
source-free pre-current electric field from the accepted before-field and
after magnetic half-step. Require every regional observer to be valid with
maximum magnetic-update, electric-pre-current, partition, global source-free,
and regional-ledger residual `<=1e-10`.

For each unbound arm require:

- initial outside energy at radius 12 `<=1e-12`;
- maximum outside energy at radius 12 `>1e-6`;
- maximum cumulative outward boundary transport through radius 12 `>1e-6`;
- positive outside energy `>1e-7` at the last tick;
- the first tail-crossing tick occurs while all current sources remain inside
  radius three.

These conditions define an outgoing source-free tail. They do not label the
tail a photon, pilot wave, wake, aura, or radiation quantum.

## 5. Formation and first-passage gates

Every unbound arm must begin graph-outside with pair energy `>1e-6`, later
enter the graph, and begin a continuous graph-inside `E_pair<-1e-6` tail no
later than tick 120. The tail must persist for at least 16 ticks and through
tick 136. No expected transition or onset tick is imported from the global-
dress histories.

Let `t_e` be the final graph-entry tick preceding that negative tail. The exact
two-sector energy identity predicts

```text
E_pair(t) = E_pair(t_e) - [E_field(t)-E_field(t_e)].
```

The first tick at which the right side falls below `-1e-6` must equal the
observed energetic-onset tick. The maximum pointwise first-passage energy
residual must be `<=1e-8`.

The bound control must remain graph-inside and `E_pair<-1e-6` for all stored
states. The two body-conjugate histories must have identical transition and
onset classes with maximum scalar-history difference `<=1e-9`.

## 6. Common-action gates

Every forward and reverse step must satisfy:

- a valid accepted root and all common-action gates;
- maximum registered residual `<=1e-10`;
- matter recoil defect `<=1e-9`;
- causal-speed excess `<=1e-12`;
- complete pair-plus-field balance `<=1e-8`;
- final state-only inverse recovery `<=1e-8`.

## 7. Locked verdicts

- preparation, matrix, serialization, support, regional ledger, root, action,
  energy, recoil, speed, or inverse failure:
  `FINITE_SUPPORT_FORMATION_EXECUTION_INVALID`;
- bound control releases:
  `FINITE_SUPPORT_BOUND_CONTROL_UNSTABLE`;
- conjugate histories disagree:
  `FINITE_SUPPORT_FORMATION_POLARITY_SENSITIVE`;
- any unbound arm lacks a negative tail of at least 16 ticks:
  `FINITE_SUPPORT_NO_DURABLE_NEGATIVE_CORE_ALL_RAYS`;
- cores form but any first-passage prediction fails:
  `FINITE_SUPPORT_CAPTURE_ENERGY_LEDGER_MISMATCH`;
- cores form but any arm lacks the outgoing-tail gate:
  `FINITE_SUPPORT_CORE_WITHOUT_OUTGOING_TAIL`;
- every gate passes:
  `FINITE_SUPPORT_OUTGOING_TAIL_FORMATION_CONSTRUCTIVE`.

## 8. Interpretation boundary

A constructive result establishes a finite-support causal preparation that
forms a selected negative core while field energy leaves a source-free local
shell before periodic contact. It removes the quotient-wide tick-zero dress
from this witness and supports matter as a localized transaction embedded in
an uncontained environment.

It does not establish persistence after tick 136, a completed-infinity limit,
an invariant basin, asymptotic stability, production adoption, a native
binding law, or a physical particle. Failure does not automatically price a
new primitive: it first distinguishes preparation instability, absence of
capture, energy-ledger mismatch, and absence of a detached tail.
