# FTD-0735 — Capture-root regularity and finite-time neighborhood v1

**Status:** `[PRE-REGISTRATION — LOCKED / NOT YET RUN]`  
**Date:** 2026-07-29  
**Parent:** FTD-0734  
**Scope:** observer-only selected dynamics; no action, state, coefficient,
toggle, production rule, scenario, physical target, or perturbation search

## Question

Are the implicit common-action roots along the FTD-0734 captured centers and
their already-selected hostile corners uniformly regular, so that the strict
finite-horizon capture margins imply a genuine open neighborhood on the
admissible complete-state constraint manifold rather than only a finite list
of surviving points?

This protocol does not claim an invariant basin or attractor.  The theorem at
issue is finite-time and relative to the fixed-count, fixed-polarity,
Gauss-admissible matter--field state manifold.

## Frozen theorem

Let `X` be a finite-dimensional admissible state manifold and let one step be
defined implicitly by `R(x_t,p_{t+1})=0`.  For a finite history
`x_0,...,x_T`, assume:

1. `R` is continuously differentiable in a neighborhood of every accepted
   transaction;
2. `dR/dp_{t+1}` is nonsingular at every forward root and the corresponding
   reverse residual is nonsingular at every reverse root; and
3. every capture classifier is a continuous strict inequality with positive
   margin along the history.

The implicit-function theorem gives a locally unique continuous step at every
history point.  A finite composition is continuous, and the finite
intersection of the inverse images of the strict capture sets is open.
Therefore an open neighborhood of `x_0` shares the same finite-time capture
class.  This conclusion does not imply invariance after `T`, attraction,
dissipation, asymptotic stability, or positive-measure formation.

For the selected branch, the quadratic B-spline coat is `C1`; field transport
is linear; the relativistic kinetic discrete gradient is smooth at positive
rest energy; and the compact pair interaction is polynomial inside its open
graph domain.  FTD-0734 supplies strictly positive graph and energy margins.
The remaining application-level question is numerical root regularity.

## Frozen histories

Use `L=33`, `dt=1/4`, the unchanged FTD-0734 parent construction, and exactly
18 histories:

```text
3 directions x 2 polarity orders x
  {center, FTD-0734 minimum-energy selector,
   FTD-0734 minimum-graph selector}.
```

The selector names are imported from the locked FTD-0734 JSON and are not
reselected:

| direction | energy selector | graph selector |
|---|---|---|
| `0_0_1` | `srp_s1p_s2m_rin_fminus` | `srp_s1m_s2m_rin_fminus` |
| `0_1_-1` | `srp_s1m_s2m_rin_fminus` | `srp_s1m_s2p_rin_fminus` |
| `1_1_1` | `srp_s1m_s2m_rin_fminus` | `srp_s1p_s2m_rin_fminus` |

Reconstruct each state independently from its tick-128 parent.  Run 256
forward transactions and the corresponding 256 state-only reverse
transactions.  No fresh perturbation amplitude is introduced.

## Root observer

At each accepted root, evaluate the unchanged residual Jacobian `J=dR/dp` by
centered differences at the already-frozen scale `h=2e-7` and at `h/2`.
Diagonalize `J^T J` independently of the nonlinear solver.  Record:

- minimum and maximum singular values;
- condition number;
- relative change of the minimum singular value between `h` and `h/2`;
- observer residual evaluations; and
- the complete endpoint difference with the observer disabled in a dedicated
  regression.

The measured Jacobian is never supplied to the nonlinear root solve.

## Gates

Every one of the `18 x 512 = 9216` roots must satisfy:

```text
regularity observer present                    true
minimum singular value                         >= 1e-3
condition number                               <= 1e4
two-scale relative sigma_min difference        <= 1e-5
common-action residual                         <= 1e-10
```

Every forward state must remain graph-inside with `E_pair<-1e-6`, nonnegative
field energy, exact Gauss/continuity accounting, and causal speed.  Every
history must recover its initial complete state within `1e-8`.  The dedicated
observer-on/off regression must reproduce the endpoint within `1e-12`.

## Verdict map

- A parent, selector, reconstruction, action, inverse, or observer-isolation
  gate fails: `CAPTURE_REGULARITY_TRANSACTION_UNRESOLVED`.
- A history remains captured but any singular-value/condition/scale gate
  fails: `CAPTURE_ROOT_REGULARITY_NOT_ESTABLISHED`.
- Any registered history loses capture: `CAPTURE_FINITE_TIME_BOUNDARY_FOUND`.
- All gates pass:
  `CAPTURE_FINITE_TIME_OPEN_NEIGHBORHOOD_NUMERICALLY_SUPPORTED`.

The positive verdict supports an open finite-time neighborhood only on the
specified admissible state manifold and only for the selected dynamics.  It
does not derive the compact interaction, prove a stable particle, establish
an invariant basin, or show persistence in an uncontained environment.
