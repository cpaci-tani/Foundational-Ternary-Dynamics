# FTD-0718 — Period-three field-bound common-action selector v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Parents:** FTD-0715, FTD-0716, and FTD-0717

## Question

Can the source-free homogeneous freedom of the exact FTD-0716 translated
field supply the complete constituent impulses of the FTD-0715 period-three
orbit, without a separately imposed binding force or a new matter primitive?

## Frozen matter history

Use the 16 registered constituent positions and polarities reconstructed by
FTD-0712, the three FTD-0715 straight segments, and the three registered pairs
of endpoint momenta.  The trajectory, charges, dispersion, current deposition,
field normalization, and tick order are fixed.  Binding stiffness is fixed to
zero: this campaign tests whether the common field itself can bind and carry
the recurrent pattern.

## Frozen homogeneous field family

At every Fourier momentum on the periodic `L=33` lattice, construct the exact
source-free matched-field tick matrix `U(k)` and the translated-return operator

\[
A_3(k)=e^{ik_x}U(k)^3-I.
\]

Take the right nullspace with the predeclared relative singular-value threshold
`1e-12`, then intersect it with the zero-electric-divergence condition.  Pair
conjugate momenta lexicographically and form the normalized real cosine/sine
basis.  No modes may be selected or removed after force responses are known.

For every basis vector, evaluate the exact quadratic-spline orbit gather along
all 48 locked segments.  Use the same eight-node Gauss rule, half-integer path
partition, electric midpoint, later magnetic field, polarity, and mapped
interaction coefficient as the engine implementation.

## Selector

Let `M` be the real `144 x N` matrix mapping homogeneous coefficients to the
three-vector impulse of every constituent on every tick.  Let `r` be the
registered momentum increment minus the impulse supplied by the FTD-0716
minimum-norm particular field.  Compute the unique Moore–Penrose minimum-norm
coefficient vector solving `M c = r`.  This algorithm, ordering, norm, and
tolerance are fixed before execution.  No binding column, regularization
weight, post-hoc mode, field rescaling, or momentum retuning is allowed.

## Independent engine replay

Write the selected source-free correction as face-electric and edge-magnetic
coefficients and replay it in the C++ matched transaction.  Require:

1. parent fingerprints and all 48 registered histories reconstruct;
2. response-matrix residual and independent engine force residual `<=1e-10`;
3. correction divergence, source-free translated return, absolute Gauss,
   continuity, electric adjoint, magnetic zero-work, per-tick total energy,
   and complete sourced translated return each `<=1e-10`;
4. every constituent speed remains at or below `C_SPEED`;
5. the selected coefficient and field norms are finite.

The local-translation and spline-Poynting momentum defects are both reported,
but neither is promoted to an exact momentum theorem by this campaign.  Exact
force balance is the registered local recoil statement.

## Verdicts

- `PERIOD_THREE_FIELD_BOUND_COMMON_ACTION_CONSTRUCTIVE_MOMENTUM_OPEN` requires
  every locked gate above to pass.  It establishes an existing-variable,
  field-bound recurrent transaction candidate, not stability, formation, a
  particle pole, or an emergent momentum theorem.
- `PERIOD_THREE_HOMOGENEOUS_FIELD_FORCE_SPACE_INSUFFICIENT` applies when the
  frozen real response matrix cannot reproduce the required 144 impulses to
  `1e-10`.
- `PERIOD_THREE_FIELD_BOUND_SELECTOR_REPLAY_NEGATIVE` applies when the linear
  solve passes but the independently replayed Gauss, return, work, energy,
  force, or causal gates fail.
- `PERIOD_THREE_FIELD_BOUND_SELECTOR_EXECUTION_INVALID` applies to missing
  parents, reconstruction failure, non-finite data, conjugacy failure, or a
  mismatch between the registered Python and C++ response seeds.

A constructive verdict advances only to perturbative stability and state-only
cycle selection.  A negative verdict forbids fitting another homogeneous
subset to this orbit; the next existing-variable alternatives are causal
formation and constituent permutation.  A new primitive is not licensed by a
single negative campaign.
