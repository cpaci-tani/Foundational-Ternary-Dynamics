# Theorem — Alternating oriented ternary parity rail and one-shot boundary v1

**Identifier:** `FTD-0874`  
**Status:** `[THEOREM — EXACT FINITE-HORIZON PREPARED-PULSE TRANSPORT]` +
`[THEOREM — RECIPROCAL BACKPRESSURE RETAINS LABELS BUT DOES NOT ENSURE PROGRESS]` +
`[THEOREM — DISTINCT REVERSIBLE PREDECESSOR CANNOT ENTER A FIXED DONE STATE]` +
`[SELECTION — ALTERNATING BOND SCHEDULE, EXISTING SEL-CA-PHASE-RAIL TYPE]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]` +
`[OPEN — NATIVE INTERSITE HAMILTONIAN, ROUTING, BOUNDARY COMPLETION, PRODUCTION, G*]`  
**Date:** 2026-08-11  
**Certificate:** locked first execution `48/48`; no repair

## 1. Result

Let a finite ternary rail be

\[
 x=(x_0,\ldots,x_{L-1}),\qquad x_j\in\{-1,0,+1\}.
\]

At global tick `n`, activate every disjoint nearest-neighbour bond whose left
coordinate has the same parity as `n`, and apply

\[
 R(a,b)=(-b,a).                                                  \tag{1}
\]

The resulting layer `U_n` is a bijective, norm-preserving, nearest-neighbour
map. Its inverse applies `R^{-1}(a,b)=(b,-a)` on the same matching.

For an isolated prepared record

\[
 x_j^{(0)}=s\delta_{j0},\qquad s\in\{-1,+1\},                  \tag{2}
\]

the exact finite-horizon solution is

\[
 x_j^{(t)}=s\delta_{jt}.                                       \tag{3}
\]

Thus the record moves exactly one adjacent cell per integer global tick,
retains its sign, and leaves a cleared trail. Applying the inverse layers in
reverse order recovers the complete initial state.

## 2. Layer proof

The active bonds form a matching, so they share no sites. Equation (1) is the
nine-state permutation already classified in FTD-0872, with

\[
 R^{-1}(a,b)=(b,-a),\qquad R^2=-I,
 \qquad \det R=+1.                                             \tag{4}
\]

It preserves `a^2+b^2`, the number of nonzero labels, and global sign
reversal. A product of disjoint copies therefore preserves

\[
 Q(x)=\sum_jx_j^2                                               \tag{5}
\]

and remains a full finite-state permutation. Unmatched finite endpoints are
held as explicit state; none is silently dropped.

Each output site depends on itself and at most its one matched neighbour.
Therefore one layer has a one-edge causal radius.

## 3. Propagation proof and minimal schedule

At `t=0`, bond `(0,1)` is active and sends `(s,0)` to `(0,s)`. If (3) holds at
tick `t`, then the pulse sits at site `t`, bond `(t,t+1)` belongs to the
matching selected by `t mod 2`, and equation (1) sends it to site `t+1` with
unchanged sign. This proves (3) by induction.

The claim is finite-horizon: for any specified `T`, retain at least `T+1`
sites and stop before the pulse reaches the boundary. No actually-infinite
lattice or thermodynamic limit is required.

A fixed disjoint matching partitions the rail into two-site blocks, so no
label can leave its initial block. The two alternating parity matchings do
carry a label across arbitrarily many edges at any preregistered finite
horizon. Hence two matchings are minimal inside the declared class of
translation-periodic disjoint nearest-neighbour bond layers.

## 4. What occupied sites do

For an occupied downstream site,

\[
 (a,b)\mapsto(-b,a),\qquad a\ne0,\ b\ne0.                     \tag{6}
\]

The outbound label `a` advances while the old downstream label `b` returns
upstream with reversed orientation. Both label magnitudes and the full
two-trit information are retained. This is the reversible alternative to the
noninjective empty-port/otherwise-hold rule excluded by FTD-0872.

Equation (6) is not congestion resolution. A fully occupied rail stays fully
occupied, so the map cannot guarantee a fresh ready port or universal forward
progress. It proves **retention under backpressure**, not clearance of
backpressure.

## 5. Exact reversible one-shot boundary

Let `F` be injective. If

\[
 F(x)=y\ne x,
\]

then `F(y)` cannot equal `y`, because otherwise `x` and `y` would be distinct
preimages of `y`. A finite bijection consists only of cycles; it has no
transient state that enters a fixed point.

Therefore a closed reversible time-homogeneous system cannot make a
nontrivial transition and then remain in a literal fixed `done` state. Exact
one-shot behavior requires at least one of:

1. continuing motion of the record;
2. continuing controller/history evolution;
3. reciprocal export to retained environment state; or
4. an explicitly noninjective loss channel.

The parity rail realizes option 1. The local source clears while the global
record keeps moving, so the full state does not collide with a fixed endpoint.
This is the simplest exact reconciliation of local one-shot clearing with
global reversibility found in the registered architecture.

## 6. Composition with FTD-0873

FTD-0873 supplies the imposed onsite Hamiltonian lift

\[
 (s,0)_{\rm latch,port}\mapsto(0,s)_{\rm latch,port}.           \tag{7}
\]

Treating that port as rail site zero, the first spatial bond applies

\[
 (s,0)_{0,1}\mapsto(0,s)_{0,1}.                               \tag{8}
\]

The composition clears the local latch and then transports the record. It
never crosses more than one spatial lattice edge in a global tick. The onsite
actuator remains the imposed harmonic reference of FTD-0873; FTD-0874 does
not derive a native intersite Hamiltonian that realizes every bond layer.

## 7. Existing clock and selection accounting

The schedule uses two structures already present in FTD:

- the integer global update index `n`; and
- parity of a coordinate along a chosen cubic rail.

It adds no new selected public type and refines the existing
`SEL-CA-PHASE-RAIL` reference architecture. The schedule is nevertheless a
**[SELECTION]**: P1--P5 do not uniquely force this alternating oriented bond
law.

The schedule is not the quartic clock. `G*` may determine eligibility dates in
a separate maintained local calendar; `n mod 2` determines which bonds are
updated. No `G*`-to-rail gearbox is derived here.

## 8. Certificate and implementation

The byte-frozen preregistration has SHA-256
`92C090ED43306249B963F757AD205F8C2B948944759A75CA46436606DDDC9BBB`.
The frozen exact certificate
`scripts/proofs/proof_alternating_oriented_ternary_parity_rail.py` has SHA-256
`2269C404912324A5C49FAA881FF7B1D151BE1BDB4A95B726ED26C2E889FD4C98`
and passed `48/48` on its first execution without repair.

The isolated implementation is:

- `engine/include/ftd/eft/alternating_oriented_ternary_parity_rail.h`;
- `engine/src/eft/alternating_oriented_ternary_parity_rail.cpp`; and
- `engine/tests/test_alternating_oriented_ternary_parity_rail.cpp`.

It exhaustively checks finite ternary layers through length six, exact public
inverse recovery, twelve-tick prepared-pulse propagation for both signs, a
fixed-matching control, occupied backpressure, actuator handoff, and fail-
closed input validation. It changes no production `Voxel`, field, render,
boundary, or default tick phase.

## 9. Boundary statement

FTD-0874 closes the exact **reference scheduling and finite-horizon transport
logic** for one oriented ternary rail. It does not close:

- native formation and energetic actuation of intersite bond rotations;
- automatic axis/orientation choice on the full cubic lattice;
- collision, branching, merging, or sustained backpressure resolution;
- a reciprocal finite-boundary/tail-port implementation;
- interaction with matter, fields, genesis, or production tick phases;
- robustness under noise, defects, missed layers, or concurrent events;
- synchronization with the separate quartic `G*` calendar;
- Born recovery, Bell correlations, operational Lorentz hiding; or
- whole-framework completeness.

