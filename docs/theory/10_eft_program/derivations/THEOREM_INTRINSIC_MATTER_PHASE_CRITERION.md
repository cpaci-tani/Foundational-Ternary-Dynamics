# FTD-0658 — Intrinsic matter-phase criterion and registered-candidate census

**Status:** `[THEOREM — PHASE CRITERION]` +
`[CLOSED NEGATIVE — CURRENT REGISTERED INTRINSIC-CLOCK CANDIDATE SET]` +
`[CONSTRUCTIVE — EXCITED ACTION-ANGLE PHASE REMAINS LIVE]`  
**Production status:** unchanged

## 1. Why phase must be defined carefully

FTD already contains several objects called phase: lattice Fourier phase,
field quadrature, constituent oscillator phase, chart branch, gait angle, and
global tick number. They are not interchangeable. A phase capable of closing
FTD-0657's matter-pole boundary must be an observable of the complete localized
matter state, not a timestamp or a phase assigned by an analyst.

## 2. Criterion

Let `Phi` be the reversible matter--field tick on an invariant localized set
`M`, and let `G` contain exact translations, cubic rotations, chart changes,
and any polarity mirror relevant to object identity. An **intrinsic matter
phase** is a map

\[
\theta:M\to S^1
\]

with all of the following properties:

1. **State-functional:** `theta` is determined by one complete state; it does
   not use launch time, trajectory history, or an external clock.
2. **Autonomous advance:** on the phase-bearing invariant set,
   `theta(Phi z)=theta(z)+Omega (mod 2pi)` with one nonzero `Omega`.
3. **Object covariance:** exact spatial translations and chart changes do not
   alter the internal phase; cubic/polarity operations act by a registered
   fixed representation, not by a refitted basis.
4. **Nondegeneracy:** the phase-bearing observable has nonzero amplitude and a
   finite conjugate action or equivalent invariant. Phase is not inferred from
   numerical noise at zero amplitude.
5. **Robustness:** `Omega` and the phase map survive a preregistered amplitude,
   sign, orientation, duration, and refinement test.
6. **Ontological selection:** if the phase is claimed intrinsic to the matter
   species rather than an excitation, native formation/rest dynamics must
   select the phase-bearing invariant set. An arbitrarily prepared normal mode
   is an excited clock, not an intrinsic rest clock.

Items 1--5 define a native **phase-bearing matter excitation**. Item 6 is the
additional requirement for an **intrinsic rest phase**.

## 3. Exact consequences

### 3.1 Fixed-point obstruction

If `z_*` is a fixed point, `Phi z_*=z_*`. State-functionality and autonomous
advance imply

\[
\theta(z_*)=\theta(z_*)+\Omega\pmod{2\pi},
\]

so `Omega=0 mod 2pi`. Therefore a fixed point cannot carry a nontrivial
intrinsic clock.

FTD-0639's analytic dressed rest state is an exact engine-resolution fixed
point. It is a valid classical rest object and fails the nonzero-phase
condition by theorem, not by insufficient run length.

### 3.2 Normal-mode phase is real but excitation-conditional

For a nondegenerate classical mode with coordinate `q`, momentum `p`, and
frequency `omega`,

\[
I={p^2+\omega^2q^2\over2\omega},\qquad
\theta=\operatorname{atan2}(\omega q,p)
\]

give an action--angle pair whenever `I>0`. The angle has no continuous
extension to `q=p=0`: limits approaching the origin from different rays give
different phases. A degenerate eigenspace also has no preferred scalar phase
without a covariant representation choice.

FTD-0640 therefore establishes a complete set of candidate **excited matter
clocks**, not an intrinsic phase of the unexcited rest object. The first
internal mode after the six soft rigid modes is the least arbitrary next
candidate because the rigid/internal split is independently fixed by the
Hessian gap.

### 3.3 Convective phase is external motion

FTD-0656's line is `omega=k·v`. It vanishes at rest and changes with the chosen
spatial wavevector. It is a valid co-motion observable but fails internality
and cannot supply a rest intercept.

## 4. Registered-candidate census

| candidate | result against criterion |
|---|---|
| FTD-0639 analytic dressed rest | fails nonzero advance exactly: fixed point |
| FTD-0640 analytic matter modes | passes phase availability for prepared `I>0`; intrinsic selection and zero-amplitude extension fail |
| FTD-0620 balanced internal gait | fails registered one-phase recurrence; closest normalized return distance `5.22037` versus `0.05` |
| FTD-0627 rigid-start breathing | fails recurrence and all-observable spectral concentration; later superseded as displaced from the true rest center |
| FTD-0656 co-moving dressing | convective `k·v`, not an internal rest phase |
| free matched field waves | possess field quadrature phase but are not a localized matter clock |
| connection/plaquette holonomy | mathematically phase-bearing candidate, not yet part of the selected matter action |
| global tick/proper-time counter | external time label, not state-functional internal phase |

Hence no registered candidate currently supplies an intrinsic rest phase.
This is a census closure, not a universal no-go.

## 5. Ontological consequence

The current evidence supports a two-level matter ontology:

1. **Unexcited matter:** a localized, reversible, fixed dressed configuration
   with mass/inertia and a co-moving field-energy dressing, but no internal
   clock yet established.
2. **Excited matter:** the same object displaced onto one or more native
   constituent/field action--angle modes, which can carry phase without a new
   primitive if robustness and covariance close.

The next constructive gate is the first non-rigid analytic internal mode under
a locked action--angle phase test with multiple amplitudes, four quadratures,
sign/cubic controls, long duration, and state-only inversion. Success would
establish a native excited matter clock. It would still not identify that
frequency with rest mass or prove quantum phase.
