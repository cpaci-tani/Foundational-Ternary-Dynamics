# Analysis — FTD-0628 connected-block static dressing refinement

**Status:** `[SELECTED ACTION] + [MEASURED — ENGINE-RESOLUTION STATIC DRESSED
FIXED POINT] + [OPEN — NORMAL MODES / GLOBAL MINIMUM / PHYSICAL PARTICLE]`  
**Verdict:** `CONNECTED_BLOCK_STATIC_DRESSED_FIXED_POINT_CONSTRUCTIVE`  
**Protocol:** `PREREG_CONNECTED_BLOCK_STATIC_DRESSING_REFINEMENT_v1.md`  
**Protocol SHA-256:**
`4B6CA4AD4ACF106124AAF9C791AF4F7B3374DC30DF3A5A9FDEDC784F66D640C6`

## 1. Result

The unchanged selected connected-block action has a nearby
symmetry-preserving dressed configuration that passes the registered static,
full-space, repeated-evolution, inverse, and cubic-covariance gates.

The x-oriented refined coordinates are

\[
(a,b,t_o,t_i)=
(1.4993153663084844,\ 0.4994670538459639,\
0.5000659053222903,\ 0.5001809664751735).
\]

The cyclic y arm agrees within `5.6e-14` in the four coordinates. Relative to
the rigid `4x2x2` geometry, the outer and inner axial layers contract by
`6.84634e-4` and `5.32946e-4`; the outer and inner transverse radii expand by
`6.59053e-5` and `1.80966e-4`. The static energy decreases from
`0.0367786431670905` to `0.0367648643204065`.

This explains the FTD-0626/0627 breathing background: the rigid integer block
was close to, but not at, the dressed equilibrium of its own field and binding
action.

## 2. Construction

The four registered shape coordinates are forced by the retained body-axis
reflection, charge-conjugation/reflection, and transverse square symmetries.
At every trial geometry the inherited fields are discarded. The observer
solves the periodic minimum-energy longitudinal Gauss field from the actual
quadratic polarity coats and sets the magnetic half-field to zero. It then
minimizes

\[
U(\theta)=U_{\rm bind}(\theta)+\beta U_{\rm field}(\theta)
\]

using only the hash-locked damped-Newton path from the rigid start. The search
converges in two accepted unit Newton steps; the third record satisfies the
locked gradient gate.

No production force, threshold, timestep, binding coefficient, field
normalization, state type, chart rule, or solver tolerance was fitted to the
outcome.

## 3. Stationarity and stability diagnostics

For the x arm:

- `||grad U||_inf = 2.35402e-10 < 1e-9`;
- Hessian eigenvalues are
  `(21.01987, 141.56319, 324.98848, 536.51875)`;
- the maximum unchanged one-step impulse over all 48 constituent components is
  `6.35742e-10 < 1e-9`;
- the one-step complete-state displacement is `6.35740e-10`;
- over 64 ticks, the maximum complete-state displacement is `1.57820e-9`,
  centre displacement is `5.18e-15`, and energy drift is `1.78e-15`;
- 64 state-only inverse steps recover the initial state to `3.16e-14`;
- the worst common-action residual is `7.95e-14`;
- maximum chart multiplicity remains two, and the minimum shared-anchor
  effective-position separation is `0.998934` cell.

The cyclic y arm passes independently. The complete rotated matter-plus-field
state agrees to `6.54e-14`; the larger aggregate covariance residual
`2.69e-10` comes from finite-difference scalar diagnostics and remains below
the locked `1e-9` gate.

## 4. Correct interpretation

This is an engine-resolution fixed point of one selected classical
constituent-plus-field action. It establishes that:

1. centre rest need not be a Peierls-maximum rigid site pattern;
2. a stationary extended object can be a self-consistent deformation of its
   integer reference graph plus its Gauss dressing;
3. the previously measured breathing is compatible with ordinary relaxation
   about a nearby equilibrium;
4. no additional primitive is forced merely to obtain static connected matter
   in this registered sector.

The result does **not** establish an exact algebraic fixed point. Its strongest
full-space margin is only about `1.57` against the `1e-9` gate, so higher-
precision continuation remains valuable. It also does not establish uniqueness,
a global energy minimum, stability outside the tested finite horizon, a native
formation mechanism, a quantum ground state, a particle identity, rest mass,
spin, charge, or Lorentz recovery.

## 5. Consequence for the matter ontology

The best current matter picture is no longer “a rigid cluster of occupied
sites.” It is:

> a finite, connected set of ternary-polarity constituents with subcell phase
> space and local chart fibre, bound through a Moore-local reference relation
> and inseparable from a self-consistent face/edge field dressing.

The site field remains a ternary manifestation projection. The constituent
configuration plus dressing carries the lossless dynamical state in this
selected research branch.

## 6. Next discriminating test

Normal-mode extraction is now licensed as a research diagnostic. It must begin
from this refined state, perturb the independently recorded Hessian directions
with both signs and multiple amplitudes, and demand amplitude-independent poles,
cubic covariance, positive mode energy, exact common-action evolution, and
state-only inversion before any clock or mass language is introduced. Only
after that should the refined dressing be boosted and tested for co-moving
transport.

