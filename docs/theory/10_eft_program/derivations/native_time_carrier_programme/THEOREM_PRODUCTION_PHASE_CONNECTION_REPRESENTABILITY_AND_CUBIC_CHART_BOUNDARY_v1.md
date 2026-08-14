# Production phase-connection representability and cubic-chart boundary

**Identifiers:** `FTD-0964`, `FTD-0965`  
**Status:** `[THEOREM — CONDITIONAL FIXED-FRAME CAPACITY]` +
`[THEOREM — SITE-LOCAL CUBIC LINEAR-CHART OBSTRUCTION]` +
`[CLOSED NEGATIVE — UNCHANGED PRODUCTION CONNECTION LAW]` +
`[OPEN — REGIONAL FRAME/NATIVE ACTION/REPEATED STABILITY/EXPORT]`  
**Date:** 2026-08-11

## 1. Result

The existing dual-substrate production registers have enough raw local
continuous capacity to store the FTD-0962/0963 connection chart, but the
current production engine does not realize that connection.

More precisely:

1. the two vector canonical field pairs
   `(flux_L,wave_vel_L)` and `(flux_R,wave_vel_R)` contain six scalar
   canonical pairs per site;
2. the connection chart needs five complete pairs: one clock pair and four
   exchange modes;
3. after selecting a fixed orthonormal frame, an explicit rank-ten symplectic
   packing embeds those five pairs and leaves one complete pair unused;
4. no site-local linear `O_h`-covariant scalar can be extracted from the two
   raw vector registers, so the fixed-frame packing is not a native cubic
   chart; and
5. the unchanged tick contains neither the complete-square connection nor its
   continuous exchange generator, reciprocal reaction, energy term, gate
   profile, reserve/backpressure law, or inverse.

Therefore no new public continuous storage type is forced by scalar capacity
alone, but production emergence is not established.

## 2. Conditional capacity theorem

For a selected fixed orthonormal frame `(e1,e2,e3)`, write

\[
 z_{L,a}=(e_a\cdot J_L,e_a\cdot P_L),\qquad
 z_{R,a}=(e_a\cdot J_R,e_a\cdot P_R),                 \tag{1}
\]

where the production field action identifies `P_L=wave_vel_L` and
`P_R=wave_vel_R` as the conjugate momenta. Define

\[
 (\delta,\Pi)=z_{L,1},\quad B=z_{L,2},\quad D=z_{L,3},
 \quad C=z_{R,1},\quad R=z_{R,2}.                     \tag{2}
\]

In pair ordering, (2) is the projection from six whole canonical pairs onto
the first five. Its matrix has rank ten and

\[
 P\Omega_{12}P^T=\Omega_{10}.                         \tag{3}
\]

The unused `z_(R,3)` remains a complete pair. Thus the selected local scalar
chart is kinematically representable without adding storage.

This is conditional on dual-substrate mode, a fixed frame, and treating the
local field components as the chart. It does not show formation, protection,
regional restriction consistency, or the target dynamics.

## 3. Cubic linear-chart obstruction

Let `V` be the three-dimensional polar-vector representation of the full
signed-cubic group `O_h`. A site-local linear scalar coordinate from the raw
dual fields requires an invariant covector in `V+V`.

The exact 48-element signed-permutation average is

\[
 {1\over48}\sum_{g\in O_h}(g\oplus g)=0.              \tag{4}
\]

The rank of the invariant projector is therefore zero:

\[
 \operatorname{Hom}_{O_h}(V\oplus V,{\bf1})=0.        \tag{5}
\]

Consequently, equation (2) necessarily imports a frame. A global Cartesian
component choice breaks cubic covariance. This theorem does not exclude a
nonlinear or regional chart built from a dynamically formed ordered body
frame and pseudoscalar; that is the next recovery debt.

## 4. Why the diagnostic clock does not close the gap

Production `Voxel::phase` is explicitly read-only diagnostic state. It is
advanced by

\[
 \phi\leftarrow\phi+\omega_0\,\Delta\tau,             \tag{6}
\]

but has no stored conjugate momentum and no tick consumer. `tau` is likewise
an accumulator. Neither supplies the canonical pair `(delta,Pi)` required by
the connection Hamiltonian.

The integer `tick_` remains the global update order. It is not a local
canonical clock pair.

## 5. Why the weak `L/R` swap is not the gearbox

Weak transmutation swaps the whole dual field registers. On one scalar
`L/R` sector its matrix is

\[
 S=\begin{pmatrix}0&1\\1&0\end{pmatrix},\qquad
 S^2=I,\quad\det S=-1.                                \tag{7}
\]

The oriented exchange quarter-turn is

\[
 R=\begin{pmatrix}0&-1\\1&0\end{pmatrix},\qquad
 R^2=-I,\quad\det R=+1.                               \tag{8}

Thus the discrete involutive swap cannot be relabelled as the continuous
orientation-sensitive holonomy.

## 6. Current-production closure

The frozen production read phase supplies separate linear `L/R` wave
accelerations. The write phase advances the two field pairs separately and
then writes their observable sums. The frozen sources contain no

\[
 {\bigl[\Pi+{\cal A}(\delta)G\bigr]^2\over2M},         \tag{9}
\]

no continuous `G_T+G_C` exchange, and no connection force. The energy audit
contains quadratic `L/R` telemetry but no connection energy, gearbox reserve,
or backpressure term. There is no reverse production tick.

This closes the unchanged current production realization negative. It is not
a no-go theorem against adding a separately declared and tested action.

## 7. What is now missing

The minimum admissible successor must supply both of the following:

1. **chart provenance:** a native regional ordered frame plus pseudoscalar, or
   an explicitly priced selected frame/connection interface; and
2. **dynamic provenance:** a local action or exact symplectic tick whose
   complete square generates the connection, books reciprocal reaction and
   energy, exposes reserve/backpressure, and has a complete inverse.

Only after those gates may the programme test full nonlinear repeated-map
stability and an open positive port that exports complete phase-error history.
The `G*` cadence question remains separate: FTD-0962/0963 conditionally puts
`G*` in traversal time, not in the selection or production of the connection.

## 8. Certificate

- parent protocol SHA-256:
  `B44C925D56BC66B3C9FCA2781AC29C86D0E8EADCF60DCA90FAA0BAD67B6A3E21`;
- immutable parent proof SHA-256:
  `2199DE8A4FDB5239B27D1973880B27D7C886DBF34D7BA22FB40A948786FB1C09`;
- first parent execution: `70/72`, Outcome D on two source-marker defects;
- repair protocol SHA-256:
  `4B3E916D72A83958FB4660488FE0B16CD7B27963044859554545204926736B4C`;
- repair wrapper SHA-256:
  `7C2A65A386B2E7474E579AD226339D68F11BDA97EA1C48FB9A65B85591BB29F1`;
- repaired inherited certificate: `72/72`, Outcome B;
- repair integrity: `13/13`.

No production file changed under either protocol.

## 9. Scope firewall

This result is not:

- a production implementation;
- a derivation of the selected frame or connection profile;
- evidence that dual-substrate energy self-organizes into the five modes;
- a `G*` gearbox derivation;
- Born/Bell recovery;
- operational hiding of the preferred tick; or
- whole-framework completeness.
