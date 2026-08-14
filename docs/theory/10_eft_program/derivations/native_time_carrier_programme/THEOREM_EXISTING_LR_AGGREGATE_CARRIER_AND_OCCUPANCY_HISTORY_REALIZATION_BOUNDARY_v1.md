# Existing L/R aggregate carrier and occupancy-history realization boundary v1

**Identifier:** `FTD-0942`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — EXACT COMMON/RELATIVE STORAGE AND BARE-WAVE AGGREGATE REVERSIBILITY]` + `[CLOSED NEGATIVE — CURRENT LINEAR L/R DYNAMICS DO NOT REALIZE THE FTD-0941 TOKEN CARRIER]` + `[CLASSIFICATION — DERIVED PROTECTED PULSE / SELECTED CHANNEL PORT / EXTERNAL JOURNAL]` + `[OPEN — PHYSICAL REALIZATION]`  
**Protocol:** [`PREREG_EXISTING_LR_OCCUPANCY_HISTORY_CARRIER_CLASSIFIER_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_EXISTING_LR_OCCUPANCY_HISTORY_CARRIER_CLASSIFIER_v1.md), pre-run SHA-256 `F7550994C541D209A63F7B936A4DE96A3B0AA50B43AD9DB8217CE5C100097F82`  
**Certificate:** `scripts/proofs/proof_existing_lr_occupancy_history_carrier_classifier.py`, SHA-256 `54AFAA09E6588A04B702A0F7368874ECA25AC21810E8532E8F04FB550E8C4808`, `330/330`, **Outcome B**

## 1. Result

The existing production L/R fields pass a real but narrower carrier theorem:

> **[THEOREM]** The local variables
> `(flux_L, flux_R, wave_vel_L, wave_vel_R)` are exactly isomorphic to common
> and relative canonical pairs. In the isolated undamped, identically operated
> wave sector, the relative kick--drift map is exactly invertible and has
> determinant one. It is therefore an aggregate reversible field carrier.

They do **not** pass the stronger FTD-0941 occupancy-history carrier gates:

> **[CLOSED NEGATIVE — CURRENT REALIZATION]** The frozen production dynamics
> do not deposit occupancy hops into the relative channel, do not translate a
> source-local event along a selected direction channel, do not preserve the
> factorization of co-located opposite event tokens, do not implement token
> backpressure, and do not close an event-source energy transaction.

This result does not prove that new primitive hardware is necessary. It closes
only the **current linear realization**. A nonlinear protected pulse or
characteristic sector derived inside the existing fields remains open.

## 2. Exact storage theorem

At each site write

\[
F=J_L+J_R,\qquad D=J_L-J_R,
\qquad P_F=P_L+P_R,\qquad P_D=P_L-P_R.       \tag{1}
\]

Then

\[
J_L={F+D\over2},\qquad J_R={F-D\over2},
\qquad P_L={P_F+P_D\over2},\qquad
P_R={P_F-P_D\over2}.                         \tag{2}
\]

Equations (1)--(2) are mutually inverse over the real field. The current
`Voxel` therefore contains one common and one relative vector canonical pair;
the relative coordinates are not merely a readout convention.

This coordinate isomorphism does **not** supply a decomposition into event
tokens. A field value is an aggregate state unless the dynamics supplies an
injective code, protected modes, or explicit channels.

## 3. Equal-source cancellation theorem

The frozen `phase_read.cpp` applies the same 18-point wave operator to L and R
and adds the matter coupling source equally:

\[
P'_L=P_L+hKJ_L+b,\qquad P'_R=P_R+hKJ_R+b.     \tag{3}
\]

Adding and subtracting gives

\[
P'_F=P_F+hKF+2b,\qquad P'_D=P_D+hKD.          \tag{4}
\]

Thus the ordinary coupling source drives the common field while cancelling
exactly from the relative field. The normal flux and wave-velocity injection
APIs likewise split additions half-and-half between L and R.

Selected particle and wavepacket initializers, plus the neutrino constructor,
can seed L/R asymmetry. That proves the storage is writable. It does not define
an autonomous occupancy-hop event deposit, a direction-labelled port, a
source debit, or an inverse event transaction.

## 4. Bare-wave aggregate reversibility

For the isolated undamped relative kick--drift map

\[
P'_D=P_D+hKD,\qquad D'=D+hP'_D,               \tag{5}
\]

the inverse is explicit:

\[
D=D'-hP'_D,\qquad P_D=P'_D-hKD.               \tag{6}
\]

For a finite symmetric operator `K`, the block matrix is

\[
M=\begin{pmatrix}
I+h^2K & hI\\
hK & I
\end{pmatrix},                                \tag{7}
\]

and the inverse from equation (6) multiplies it to the identity on both sides.
Exact elimination gives `det M=1`. The certificate verifies the identities
over rational matrices and states without a numerical tolerance.

This reaffirms FTD-0876: the bare wave sector possesses a legitimate Markov
canonical carrier. It does not promote the complete production tick to a
Hamiltonian permutation. Damping, manifestation, projection, movement,
annihilation, boundary processing, and other enabled phases have independent
reversibility and energy debts.

## 5. Exact one-hop routing obstruction

The production Laplacian weights the six face neighbors by `1/3`, the twelve
edge neighbors by `1/6`, the eight corners by zero, and the center by `-4`.
Let a nonzero relative pulse `v` and arbitrary relative momentum be supported
only at the event site before one kick--drift step. Every face neighbor then
receives

\[
{h^2c_w^2\over3}v,                             \tag{8}
\]

and every edge neighbor receives

\[
{h^2c_w^2\over6}v.                             \tag{9}
\]

The source-local momentum changes the source value but cannot cancel those
neighbor values on that step. A nonzero field pulse therefore fans into all
18 coupled neighbors. If the field pulse is zero and only source-local
momentum is loaded, the first drift remains at the source. Neither case is the
direction-channel permutation

\[
c_\nu(x)\longmapsto c_\nu(x+\nu)               \tag{10}
\]

used by FTD-0941. In particular, corners are not one-tick wave neighbors.

This is a routing result, not a denial of causal propagation. The stencil is a
valid local wave operator; it simply does not implement a selected one-port
occupancy token hop.

## 6. Collision-factorization obstruction

Suppose a co-located event token in direction `nu` is encoded as an odd,
cubic-covariant relative vector pulse `e(nu)`. Cubic inversion requires

\[
e(-\nu)=-e(\nu).                               \tag{11}
\]

Linearity then gives

\[
e(\nu)+e(-\nu)=0.                              \tag{12}

Consequently, the multiset containing one `nu` token and one `-nu` token has
the same aggregate field state as the empty multiset. The field evolution may
remain perfectly invertible after this encoding, but no invertible future map
can recover a factorization absent from its input.

The no-go is scoped carefully. It does **not** follow from the invalid shortcut
“six real coordinates cannot encode 26 labels.” Real coordinates can encode
finite labels. It follows from co-location, odd covariance, linear
superposition, and the absence of a protected channel or nonlinear pulse
identity. Separate spatial pulses, solitons, nonlinear topological labels, or
explicit direction ports evade the premises.

## 7. Transmutation, movement, and the observation journal

Current weak transmutation swaps L and R:

\[
(J_L,J_R,P_L,P_R)\mapsto(J_R,J_L,P_R,P_L),
\quad D\mapsto-D.                              \tag{13}

It is an exact relative reflection, not a quarter-turn or a direction-port
translation. Movement and annihilation include clears and neighbor
redistribution. They do not implement the protected swap/stream/backpressure
law of FTD-0941.

The production history journal can record before/after snapshots. It is
explicitly observation-only. It is valuable diagnostic evidence, but it is
not substrate state, cannot react back, and cannot serve as the physical
owner of winding or reciprocal carry.

## 8. Energy-ledger correction and boundary

The current diagnostics now report separate onsite quadratic telemetry:

\[
{1\over2}|J_L|^2,\quad {1\over2}|J_R|^2,
\quad {1\over2}|P_L|^2,\quad {1\over2}|P_R|^2. \tag{14}

Therefore earlier implementation descriptions saying that diagnostics expose
only the common field are stale for the frozen 2026-08-11 source. This is an
engine-fact correction, not a theoretical promotion.

Equation (14) still is not the required occupancy-history transaction. It is
read-only onsite telemetry and does not include all of:

- an exact stiffness-plus-kinetic invariant for the finite-step map;
- a face-resolved energy current;
- a pre-hop occupancy-event debit;
- a protected token/source exchange; or
- the FTD-0941 token normalization `epsilon_*`.

The naive quadratic wave energy need not be exactly conserved by symplectic
Euler even though the map is symplectic. A shadow Hamiltonian or another exact
discrete action could exist, but must be derived and connected to the source
transaction separately.

## 9. Realization trilemma

FTD-0942 leaves exactly three honest branches:

1. **[OPEN — DERIVED FIELD ROUTE]** Derive collision-resolving nonlinear
   pulses, solitons, or characteristic sectors within `(D,P_D)`, together with
   an occupancy source, inverse, backpressure, and energy current. This adds
   dynamics/invariants but no primitive storage type.
2. **[SELECTION REQUIRED — CHANNELIZED PORT ROUTE]** Add an oriented channel
   family `nu in M_26`, bounded lanes or multiplicity, reversible
   backpressure, and an energy scale. This is a separately priced selected
   carrier type and update law. No bit count is claimed here.
3. **[DIAGNOSTIC ONLY — JOURNAL ROUTE]** Retain history externally in the
   observation journal. This supports audit and replay but does not close the
   ontic physical carrier.

Logic does not yet choose between branches 1 and 2. The priority rule is to
test branch 1 first because the required canonical storage already exists.
Failure of all preregistered protected-pulse classes would then price branch 2
without pretending it was derived.

## 10. Verification record

The immutable exact run reported:

```text
FTD-0942 exact certificate: 330/330 checks passed
Outcome B -- existing L/R fields are an exact aggregate canonical carrier in the
isolated bare-wave sector, but current production dynamics do not realize the
collision-separated occupancy-history carrier of FTD-0941.
Missing gates: event deposit, nu-channel routing, collision separation,
backpressure, and an exact source-energy transaction.
Type verdict: no new primitive is forced; protected nonlinear field pulses
remain open alongside a separately priced channelized-port realization.
```

Focused frozen-production controls passed `5/5`:

- `dual_substrate`;
- `audit_regression`;
- `flux_wave_velocity_markov_carrier`;
- `full_state_irreversibility`; and
- `native_engine_history_flow`.

These controls confirm the storage, telemetry, aggregate Markov-carrier, full
tick boundary, and observation-only journal facts. They are not evidence that
the missing occupancy carrier exists.

## 11. What is and is not closed

**Closed:**

- exact common/relative storage isomorphism;
- exact isolated bare-wave aggregate inverse and determinant one;
- exact cancellation of equally split sources from the relative channel;
- exact 18-neighbor fanout rather than a one-port hop;
- exact co-located opposite-token collision in the registered linear class;
- current separate L/R telemetry correction; and
- failure of the frozen production realization to meet FTD-0941.

**Open:**

- a derived nonlinear or characteristic pulse inside existing fields;
- autonomous occupancy-hop deposit and inverse;
- collision-separated multiplicity and backpressure;
- an exact discrete source--field energy current and normalization;
- body-local attachment and moving-hub transport;
- identification with the FTD-0933 wake and Hodge chord;
- physical impulse, mass, `gamma`, formation, recovery, and production;
- finite-tick `G*` clock synchronization; and
- Born/Bell/context/outcome and operational hiding.

No production file, `Voxel` type, CMake target, physical parameter, or
ontology was changed by FTD-0942.
