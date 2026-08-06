# FTD-0722 — Field-assisted derived-pair capture v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE VALIDATION RUN]`  
**Identifier:** `FTD-0722`  
**Date:** 2026-07-28  
**Parents:** `FTD-0479`, `FTD-0541`, `FTD-0550`, `FTD-0551`, `FTD-0622`,
`FTD-0719`, `FTD-0720`, `FTD-0721`  
**Scope:** observer-only fixed-count constituent/face-field formation test; no
production state, default, toggle, scenario, particle identity, or ontology
promotion.

## 1. Locked question

Can the existing constituent phase space and matched face-electric/edge-
magnetic variables turn an initially unbound opposite-polarity encounter into
a negative-internal-energy pair by exporting the excess energy into a
propagating field, while one atomic transaction preserves continuity, Gauss,
total energy, locality, causal speed, recoil symmetry, and state-only
inversion?

This is the constructive successor to FTD-0721's closed-two-body energy
threshold. It forbids phenomenological damping, post-step energy correction,
stored bond bits, and hand-authored graph edits.

## 2. Frozen state and action

The complete state is

\[
X=(x_1,p_1,q_1;x_2,p_2,q_2;E_f,B_e),\qquad q_1=-q_2=1,
\]

where each `x_a` is represented by the established nearest-site anchor plus
continuous remainder, `E_f` is oriented face flux, and `B_e` is the matched
edge field. The ternary polarities remain exactly `+1` and `-1`; quadratic
coats are derived coupling shapes only.

The interaction graph is not stored:

\[
(1,2)\in G(X)\quad\Longleftrightarrow\quad |x_1-x_2|^2<3/2.
\]

The compact pair potential is exactly the FTD-0721 selection

\[
U(d)=\begin{cases}
-16\epsilon(d-3/2)^2(d-3/4),&d<3/2,\\
0,&d\ge3/2,
\end{cases}\qquad \epsilon=10^{-2}.
\]

The one-step matter update uses the production dispersion and its exact
endpoint discrete gradient. The pair impulse is the radial divided gradient
of `U`. Each straight constituent segment generates its established exact
quadratic-coat face current. Their sum drives the established staggered field
step

\[
B' = B-\lambda C^T E,\qquad
E_* = E+\lambda C B',\qquad
E'=E_*-K,
\]

with `lambda=C_SPEED*dt`, `dt=1/4`, canonical FTD-0468/0479 interaction
normalization, unit polarity scale, and unit field-energy scale. Electric
impulse is the exact current-adjoint orbit gather; magnetic impulse is the
matched curl-compatible zero-work gather. The six endpoint momenta are solved
simultaneously. No separate Coulomb, Lorentz, damping, confinement, or legacy
force is present.

The initial field is the deterministic minimum-energy periodic longitudinal
solution of Gauss for the two quadratic coats, with `B=0`, CG tolerance
`1e-13`, and at most 4096 iterations. Multiplicity-two anchor fibres are
allowed because a subcell pair is not two identical ontic records.

## 3. Identities and observers

For every accepted root, check:

1. exact coat continuity and before/after Gauss;
2. the production kinetic discrete-gradient identity;
3. exact electric deposition/gather adjointness;
4. zero scalar magnetic work;
5. exact radial-potential work and equal/opposite pair impulse;
6. matter work equals current work and field work is its negative;
7. total `kinetic + U + modified field energy` conservation;
8. causal constituent speeds and local current support;
9. zero-COM recoil symmetry in the registered equal/opposite arms;
10. reconstruction of the complete earlier state by the reverse root using
    only the later state and the same frozen action.

At the final state, recompute the minimum-energy longitudinal field for the
final density. Its difference from the actual field is the dynamic dressing/
radiative remainder. Record its positive component-aware norm, magnetic
energy, and radial quantiles. These are morphology observers, not photon
number.

## 4. Fresh validation arms

Use `L=33` and 24 forward plus 24 reverse steps. Run:

- all 13 unoriented Moore rays;
- both polarity orders;
- centers at the lattice center and its translated copy `(4,-3,2)`;
- unbound encounters: separation `1.30`, inward momentum magnitude `0.07`;
- already-bound controls: separation `1.00`, inward momentum magnitude
  `0.015`.

This gives `2 x 13 x 2 x 2 = 104` histories. The momenta, horizon, volume,
potential, field normalization, solver, tolerances, classifiers, and verdict
map may not change after the first validation output.

## 5. Execution and formation gates

Every step must converge and satisfy the established common-action gates with
maximum residual `<=1e-10`. Every 24-step reverse history must recover the
complete initial state within `1e-8`. Scalar histories of translated copies
must agree within `1e-9`; polarity mirrors must agree within `1e-9`. Matter
center momentum and registered total-recoil defect must remain below `1e-9`
in the symmetric arms.

An unbound arm is classified as captured only if:

- it starts outside `G` with pair internal energy
  `2(H-E_REST)+U > 1e-6`;
- it enters `G`, never exits afterward, and is in `G` for the final eight
  steps;
- its pair internal energy is `<-1e-6` at every one of the final eight steps;
- field-energy gain balances pair-internal-energy loss within `1e-8`;
- the final dynamic-field norm exceeds `1e-8`, magnetic energy exceeds
  `1e-10`, and the dynamic-field median doubled radius is at least four.

An already-bound control passes if it starts negative, remains in `G`, and
stays below `-1e-6` for the final eight steps.

## 6. Locked verdict map

- All algebraic/inverse/covariance gates pass, every bound control remains
  bound, and every unbound arm captures with the dynamic-field gates:
  `FIELD_ASSISTED_DERIVED_PAIR_CAPTURE_CONSTRUCTIVE`.
- The same gates pass and only a strict subset of unbound arms captures:
  `FIELD_ASSISTED_CAPTURE_DIRECTION_CONDITIONAL`.
- The same gates pass, bound controls remain bound, and no unbound arm
  captures: `FIELD_ASSISTED_CAPTURE_NOT_OBSERVED_LOCKED_V1`.
- An unbound arm enters the negative sector but its outgoing-field gates fail:
  `CAPTURE_WITHOUT_OUTGOING_FIELD_QUALIFICATION`.
- Any bound control fails: `DERIVED_PAIR_BOUND_STATE_UNSTABLE_WITH_NATIVE_FIELD`.
- Any atomic identity, root, Gauss, inverse, covariance, or recoil gate fails:
  `FIELD_ASSISTED_PAIR_TRANSACTION_UNRESOLVED`.

Only the first verdict establishes a constructive formation witness for this
selected pair law. It would still not derive the potential, physical particle
content, count-changing reactions, quantum stability, or production adoption.
A negative verdict remains binding for this locked v1 and may be repaired only
by a fresh versioned candidate.
