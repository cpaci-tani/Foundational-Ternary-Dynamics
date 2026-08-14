# Minimum-suspension production pair ownership and merged-square boundary

**Identifier:** `FTD-0975`  
**Status:** `[THEOREM — ALTERNATIVE/SPECIALIZED EXISTING-PAIR CAPACITY]` +
`[THEOREM — SEVEN-PAIR INDEPENDENT-COEXISTENCE OBSTRUCTION]` +
`[THEOREM — SHARED-CLOCK SIX-PAIR CAPACITY]` +
`[CLOSED NEGATIVE — SUMMED COMPLETE SQUARES ON ONE CLOCK]` +
`[OPEN — SELECTED MERGED SQUARE/FORMATION/PRODUCTION]`  
**Date:** 2026-08-12

## 1. Result

The FTD-0974 minimum suspension is conditionally representable in the existing
six dual-field canonical pairs, but its relationship to the five-pair
FTD-0963 gearbox must be declared precisely.

- As an alternative mechanism, the suspension uses two pairs and fits.
- As a specialization, it reuses the gearbox clock and one existing exchange
  pair and fits without adding state.
- As a wholly independent simultaneous mechanism, it would require seven
  pairs and cannot fit in six.
- Sharing the gearbox clock and using the sixth unused pair fits
  dimensionally, but the two Hamiltonians cannot be added independently:
  doing so double-counts the clock kinetic term.

The minimum coherent coexistence candidate is one merged complete square,

\[
 H_{\rm merge}={ (\Pi+X-I)^2\over2M}+H_{\rm rest},          \tag{1}
\]

where `X` is the prior gearbox load and `I` is the new field action. Equation
(1) introduces a genuine `-XI/M` interaction. It is a new selected law, not a
consequence of storage capacity or the unchanged production tick.

## 2. Exact pair inventory

Write the six conditional regional production pairs as

\[
 c_0,c_1,c_2,c_3,c_4,c_5.                                  \tag{2}
\]

The FTD-0963/0965 capacity witness assigns

\[
 c_0=(\delta,\Pi),qquad c_1,c_2,c_3,c_4
 \text{ to four exchange modes},                            \tag{3}
\]

and leaves `c_5` as one whole unused pair.

The FTD-0974 suspension needs

\[
 s_0=(\theta,A),qquad s_1=(Q,P).                           \tag{4}
\]

All capacity statements below preserve whole canonical pairs. No scalar half
pair is split or counted twice.

## 3. Alternative and specialization capacity

For an alternative realization choose, for example,

\[
 s_0=c_0,qquad s_1=c_5.                                   \tag{5}
\]

The associated rank-four projection obeys

\[
 P\Omega_{12}P^T=\Omega_4.                                 \tag{6}
\]

Four complete pairs remain available. Thus the suspension alone does not
force a new continuous state type.

For a specialization choose

\[
 s_0=c_0,qquad s_1=c_j,qquad j\in\{1,2,3,4\}.             \tag{7}
\]

Every choice in (7) is rank four and symplectic. It is a subsystem or
replacement of the five-pair reference gearbox, not an additional
independent mechanism. One cannot count the same clock/exchange pair once for
FTD-0963 and again as newly derived FTD-0974 capacity.

## 4. Independent coexistence obstruction

The full gearbox owns five pairs. A disjoint suspension owns two. Independent
coexistence would therefore require

\[
 5+2=7\text{ pairs}=14\text{ symplectic dimensions}.        \tag{8}
\]

The existing dual-field space has only

\[
 6\text{ pairs}=12\text{ dimensions}.                       \tag{9}
\]

No rank-fourteen injection exists into a twelve-dimensional space. A wholly
independent suspension alongside the complete five-pair gearbox is closed
negative unless another complete pair is adopted.

## 5. Shared-clock dimensional capacity

There is one lower-price coexistence layout:

- share `c_0=(delta,Pi)` as the controller clock;
- retain `c_1,...,c_4` for the gearbox; and
- use the unused `c_5` as `(Q,P)`.

The union owns each of the six pairs exactly once and its projection has rank
twelve with the full ambient symplectic form. There is no representation
obstruction.

This is only coordinate ownership. The two laws still compete for one clock
kinetic term.

## 6. Why summing the Hamiltonians is invalid

Let `X` denote the complete FTD-0963 connection load and `I` the FTD-0974
field action. Treating the two laws as independent gives

\[
 H_{\rm sum}={ (\Pi+X)^2\over2M}
             +{ (\Pi-I)^2\over2M_s}+H_{\rm rest}.           \tag{10}
\]

At zero loads,

\[
 H_{\rm sum}|_{X=I=0}
 =\Pi^2\left({1\over2M}+{1\over2M_s}\right),               \tag{11}
\]

and

\[
 \dot\delta={\Pi+X\over M}+{\Pi-I\over M_s}.              \tag{12}
\]

For every finite positive `M_s`, equation (12) contains the additional bare
term `Pi/M_s`. It changes the clock rate and effective inverse mass even when
both loads vanish. Removing it requires the singular limit `M_s -> infinity`,
which removes the second clock dynamics.

Therefore equation (10) is double-booking, not two independent couplings to
one clock.

## 7. The merged-square candidate

One shared controller must have one mechanical momentum and one kinetic term.
The minimum combined candidate is equation (1), with

\[
 K=\Pi+X-I,qquad \dot\delta={K\over M}.                    \tag{13}
\]

Expanding the square gives

\[
 {\Pi^2+X^2+I^2+2\Pi X-2\Pi I-2XI\over2M}.                 \tag{14}
\]

The term

\[
 -{XI\over M}                                               \tag{15}
\]

is a new interaction between the old gearbox load and the new field action.
Its sign and unit coefficient follow from the proposed merged square, but the
choice to merge in precisely this way is not forced by pair capacity.

Equation (1) must therefore be treated as a fresh selected candidate. It
requires its own positivity, Hamilton-equation, endpoint ledger, inverse,
repeated-map, and production-source certificate before use.

## 8. Certificate

- protocol SHA-256:
  `27086B3B15762DB544EFEA35299B58C41DDED283FD1D289C34168FBCE9487F17`;
- proof SHA-256:
  `4AEE80B47DE8ABE5780FF01AEAA7A83537708161898CFF6F63F7BF7B346D1B78`;
- first immutable execution: `46/46`, Outcome B;
- no repair and no engine or production mutation.

## 9. Scope firewall

This theorem does not establish:

- the physical identity, formation, or protection of `c_5`;
- that the regional frame persists under production dynamics;
- equation (1) as a native or adopted production law;
- switched/edge-localized coupling, work ports, replenishment, routing,
  backpressure, positive export, or erasure;
- repeated-map stability or attraction;
- quadrant dwell time or `G*`/CM provenance;
- Born/Bell recovery or preferred-tick hiding; or
- production integration or whole-framework completeness.

The next admissible mathematical branch is a fresh preregistration of the
merged square (1), with exact reaction and energy ledgers. Production source
integration remains forbidden until that candidate passes and its physical
pair identities are independently justified.
