# Theorem — Flux/wave-velocity Markov canonical carrier and production boundary v1

**Identifier:** `FTD-0876`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — EXACT HISTORY/MARKOV EQUIVALENCE]` + `[THEOREM — FREE-WAVE CANONICAL SYMPLECTIC CARRIER]` + `[ENGINE FACT — NATIVE FLUX/WAVE-VELOCITY STORAGE]` + `[CLOSED NEGATIVE — COMPLETE PRODUCTION TICK IS SYMPLECTIC]` + `[OPEN — NATIVE RECORD PREPARATION, ACTUATION, SCALE, ROUTING, G* SYNCHRONIZATION]`

## 1. Question and scope

FTD-0875 proved that a local Hamiltonian parity rail needs one canonical pair
per site in the registered onsite-direct-sum class. It left the pair's native
formation open. The production engine already stores, at every voxel,

```text
flux      = J_n,
wave_vel  = P_(n-1/2),
```

and advances the free wave by a kick followed by a drift. This document asks
exactly what that existing pair closes.

The answer has two sharply separated parts:

1. **carrier type:** the free undamped flux sector already supplies three
   local real canonical pairs per voxel; no Hilbert space and no newly adopted
   oscillator coordinate are required;
2. **production dynamics:** the complete live tick is not thereby a
   Hamiltonian parity rail. Damping, Langevin noise, Gauss projection,
   manifestation, boundaries, and other enabled maps lie outside the proved
   symplectic free-wave sector.

No Born, Bell, `G*`, Lorentz, biological, or completeness result is asserted.

## 2. Configuration history and phase-complete state

Let `V=R^(3|Lambda|)` be the finite-region flux configuration space and let
`h>0` be one declared time step. Two consecutive flux configurations determine
the staggered temporal difference

\[
 P_{n-1/2}=\frac{J_n-J_{n-1}}{h}.
\]

Define the history chart

\[
 \Phi_h:(J_{n-1},J_n)\longmapsto(J_n,P_{n-1/2}).
\]

Its inverse is

\[
 \Phi_h^{-1}(J_n,P_{n-1/2})=(J_n-hP_{n-1/2},J_n).
\]

Therefore `Phi_h` is a linear bijection. The phase-complete coordinate `P`
does not add trajectory information beyond two consecutive `J` slices. It is,
however, required if the instantaneous update is to be a first-order Markov
map. Consequently the v2 algebra must distinguish:

```text
configuration algebra:
  A_act^conf(Lambda) = B_b((R^3_J x T)^Lambda),

phase-complete Markov algebra:
  A_act^phase(Lambda) = B_b(((R^3_J x R^3_P) x T)^Lambda),

where T = {-1,0,+1}.
```

Both algebras are commutative. The state-only record algebra remains
`D_Lambda ~= C^(3^|Lambda|)`. This refinement does not turn an actual ternary
record into `M_3(C)`.

## 3. Exact free-wave canonical map

Write the periodic finite-region free-wave acceleration as

\[
 \dot P=-KJ,
\]

where `K=K^T` is the symmetric positive-semidefinite stiffness matrix
`-c^2 Delta`. The production kick/drift ordering is

\[
 P^+=P-hKJ,
 \qquad
 J^+=J+hP^+.
\]

In the ordered coordinates `(J,P)`, its matrix is

\[
 S_h=
 \begin{pmatrix}
 I-h^2K & hI\\
 -hK & I
 \end{pmatrix}.
\]

For the canonical form

\[
 \mathbb J=
 \begin{pmatrix}0&I\\-I&0\end{pmatrix},
\]

direct multiplication gives

\[
 S_h^T\mathbb J S_h=\mathbb J
 \quad\Longleftrightarrow\quad K^T=K.
\]

Thus the registered free kick/drift is exactly symplectic, has determinant
one, and has exact inverse

\[
 J=J^+-hP^+,
 \qquad
 P=P^+ + hKJ.
\]

Eliminating `P` gives the second-order recurrence

\[
 J_{n+1}-2J_n+J_{n-1}=-h^2KJ_n.
\]

This proves that the two-slice history and the first-order canonical Markov
description are the same discrete dynamics in different coordinates.

## 4. Locality and the FTD-0875 carrier

At a single site `x`, the production pair is

\[
 (J_x,P_x)\in\mathbb R^3\oplus\mathbb R^3,
 \qquad
 \{J_x^a,P_y^b\}=\delta_{xy}\delta^{ab}.
\]

It therefore contains three onsite canonical pairs. For any fixed unit vector
`e`, the projection

\[
 q_x=e\cdot J_x,
 \qquad p_x=e\cdot P_x
\]

obeys `{q_x,p_y}=delta_xy` and is a witness of the minimum FTD-0875 scalar
carrier. Equivalently, the FTD-0875 bond generator can be applied componentwise
without choosing `e`:

\[
 L_n^{(3)}=
 \sum_{(j,k)\in M_n}(J_j\cdot P_k-J_k\cdot P_j).
\]

This vector generator is cubic-coordinate covariant and reduces to the scalar
generator on every fixed component. It identifies an available carrier type;
it does not show that the production tick contains this bond interaction or
prepares `J_j=a s e`, `P_j=0`.

## 5. Exact boundary of the result

### 5.1 Uniform damping

The production damping branch scales both fields by `rho`:

\[
 D_\rho(J,P)=(\rho J,\rho P).
\]

It obeys

\[
 D_\rho^T\mathbb J D_\rho=\rho^2\mathbb J,
 \qquad
 \det D_\rho=\rho^{2\dim V}.
\]

For the live dissipative case `0<=rho<1`, the map is conformally symplectic,
not symplectic. At `rho=0` it is noninvertible.

### 5.2 Langevin update

The Ornstein-Uhlenbeck branch reads fresh random draws and contracts the
stored momentum. It is not a deterministic symplectic map on `(J,P)` unless
the bath variables and their update are added to the state. That enlarged bath
is not supplied by this theorem.

### 5.3 Gauss projection

Any nonidentity idempotent projection `G` has a nontrivial kernel and is not
injective. A symplectic map is invertible. Therefore a nonidentity Gauss
projection cannot itself be a symplectic automorphism of the unconstrained
`(J,P)` phase space. A reduced constrained-phase-space proof would be a
different construction and remains open.

### 5.4 Manifestation, evaporation, and boundary maps

Threshold/RNG transitions, absorbing sponges, dispersal, and state-changing
maps are not included in the free-wave proof. Their information and energy
ledgers must be evaluated separately. The exact free-wave result cannot be
promoted to a theorem about the complete production tick.

## 6. Energy statement at the correct strength

The continuous free-wave Hamiltonian is

\[
 H(J,P)=\frac12P^TP+\frac12J^TKJ.
\]

The kick/drift map is symplectic, but it does not conserve this exact
continuous Hamiltonian at every finite step. It conserves a nearby shadow
Hamiltonian in its stability domain and has no secular Hamiltonian drift in
the standard symplectic sense. FTD-0876 therefore claims exact symplectic-form
preservation and exact invertibility—not exact equality of the naive
finite-step energy expression.

The exact FTD-0875 stroboscopic bond-energy ledger remains a separate result
for its imposed clock-gated Hamiltonian.

## 7. Epistemic accounting

### Closed

- `[THEOREM]` two-slice flux history is bijective with the staggered
  flux/wave-velocity Markov state;
- `[THEOREM]` the symmetric-stiffness free kick/drift is exactly symplectic and
  invertible;
- `[ENGINE FACT]` production `Voxel` already stores `flux` and `wave_vel`;
- `[THEOREM]` the vector pair supplies three local canonical pairs per site;
- `[THEOREM]` uniform damping is conformally symplectic rather than
  symplectic; and
- `[CLOSED NEGATIVE]` the free-wave proof licenses a symplectic-complete-
  production-tick claim.

### Still open

- dynamic preparation and persistence of the ternary record section;
- derivation of the amplitude, clock frequency, and energy scale;
- insertion of the FTD-0875 intersite bond generator into production;
- routing, branching, collisions, congestion, and reciprocal finite
  boundaries;
- constrained symplectic treatment of Gauss projection;
- environment-complete treatment of damping, noise, genesis, and loss;
- synchronization to the separate critical-quartic `G*` calendar;
- operational hiding, Born recovery, and Bell laboratory recovery.

No new selected type is booked. The result retires the **carrier-coordinate
availability** portion of `OPEN-CA-TRANSDUCER`, while leaving carrier
preparation, actuation, and physical closure open.

## 8. Verification and implementation contract

The locked certificate is
`scripts/proofs/proof_flux_wave_velocity_markov_canonical_carrier.py`.
The isolated reference implementation is under `ftd::eft` and must not modify
production `Voxel`, toggles, or tick phases.

Required terminal markers are:

```text
FLUX_WAVE_VELOCITY_MARKOV_CANONICAL_CARRIER_THEOREM
HISTORY_MARKOV_CHART=EXACT_BIJECTION
FREE_WAVE_KICK_DRIFT=SYMPLECTIC
NATIVE_CANONICAL_PAIRS_PER_SITE=3
COMPLETE_PRODUCTION_TICK_SYMPLECTIC=NO
GSTAR_ROLE=SEPARATE_CALENDAR
```
