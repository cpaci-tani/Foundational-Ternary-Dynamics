# FTD-0622 — Connected Moore-block common action v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parent:** FTD-0621 result SHA-256
`D6ED6A0BF3C9B351ED59E4B16C0FD82430A4713B4ED06B0092F9BDCBB4026383`  
**Scope:** selected observer dynamics; one-step connected-action feasibility  
**Date:** 2026-07-27

## 1. Question

Can the exact integer FTD-0621 block-bipole architecture be embedded in one
local, constituent-complete, reversible matter-field action without a global
rigid-centre constraint, independent compact copies, fractional primitive
polarity, legacy forces, or post-hoc energy correction?

This is the construction gate preceding infrared scaling. It does not test a
long-lived rest state, coherent many-cell motion, or a particle pole.

## 2. Locked state and source family

For `w in {1,2,3}`, use the FTD-0621 source with dimensions
`(2w,w,w)` and orientation `d`. Every occupied site is one explicit
`MatchedMatterPoint` with primitive charge `+1` in the first block and `-1`
in the second. Thus

| `w` | constituents | positive | negative |
|---:|---:|---:|---:|
| 1 | 2 | 1 | 1 |
| 2 | 16 | 8 | 8 |
| 3 | 54 | 27 | 27 |

All anchors are distinct. A common fractional phase `f` is represented by the
already selected remainder of every constituent along the registered phase
axis. Primitive site charges remain integer; the FTD-0541 quadratic coat is
only their derived coupling representation.

The initial electric field is the unique zero-mean minimum-energy periodic
solution of `div E=rho` for the complete derived density. The magnetic field
and all initial momenta are zero. No stationary neutralizer is present.

## 3. Locked connected local binding graph

Connect two constituents iff their initial integer coordinate difference is a
nonzero Moore-neighbor vector:

\[
\delta\in\{-1,0,1\}^3\setminus\{0\}.
\]

Each undirected edge `e=(a,b)` has its own frozen geometric rest length
`ell_e^2=|delta_e|^2 in {1,2,3}` and the inherited FTD-0600 coefficient
`kappa=1`:

\[
V_{\rm bind}=\frac14\sum_{e=(a,b)}
\left(|X_a-X_b|^2-\ell_e^2\right)^2.
\]

The registered edge counts are exactly `1`, `72`, and `365` for
`w=1,2,3`. The graph must be connected, every edge must remain Moore-local in
its reference configuration, and binding impulses must be equal-and-opposite
edge by edge. No all-to-all, centre-of-mass, shape-template, or rigidity force
is admitted.

## 4. Locked common transaction

Generalize the FTD-0601 transaction from a fixed six-element array to runtime
constituent and edge vectors without changing its equations:

1. production dispersion
   `H(p)^2=E_REST^2+C_SPEED^2 |p|^2`;
2. endpoint discrete-gradient velocity
   `v=C_SPEED^2(p0+p1)/(H0+H1)`;
3. exact quadratic-coat straight-segment current for every constituent;
4. the unchanged matched face/edge Maxwell update;
5. electric and magnetic impulse from the unchanged orbit gather;
6. edge binding impulse from the same endpoint discrete gradient as
   `V_bind`;
7. solve all endpoint momenta simultaneously;
8. reconstruct the earlier state from the later state using only the later
   state, charges, edge graph, and fixed options.

The implementation may replace determinant products with pivot diagnostics and
may use compensated summation. It may not alter any physical equation,
normalization, graph, arm, or tolerance after a result is inspected.

## 5. Locked arm matrix

Use `L=17`, `dt=1`, `wave_speed=C_SPEED`, `kappa=1`, the unchanged FTD-0468
interaction normalization, and zero initial momenta.

Primary orientation `d=x`, for every `w in {1,2,3}`:

1. integer-phase rest response `f=0`;
2. parallel fractional response, phase axis `x`, `f=1/4`;
3. transverse fractional response, phase axis `y`, `f=1/4`.

At `w=2`, add four cyclic controls: orientations `y,z` with one parallel and
one cyclic transverse phase axis at `f=1/4`. Total registered initial states:
`13`; each receives one forward and one state-only reverse solve.

No arm may be replaced if initialization, nonlinear solve, site projection, or
inverse recovery fails.

## 6. Locked exactness and locality gates

Every arm must satisfy:

- exact constituent/sign and edge counts;
- graph connectedness and reference Chebyshev edge radius exactly one;
- unique site anchors before and after;
- minimum-energy initialization residuals `<=1e-11`;
- root, force, continuity, Gauss, kinematic, kinetic-gradient, electric
  adjoint, magnetic-work, binding-work, total binding-impulse, matter-work,
  field-work, and total-energy residuals `<=1e-10`;
- causal-speed excess zero to `1e-12`;
- state-only reverse recovery `<=1e-8`;
- no unregistered site hop in this one-step small-response campaign.

The looser aggregate `1e-10` floating gate relative to the compact six-body
campaign is locked in advance because the `w=3` action sums 54 currents and
365 bond contributions. Per-edge action identities remain analytic; the
independent certificate must reconstruct all aggregate sums.

## 7. Locked response observables

For each forward arm record:

- total matter momentum change;
- both the FTD-0618 local field-pseudomomentum change and the qualified
  FTD-0619 spline-Poynting change;
- their matter-plus-field defects;
- field, kinetic, and binding energies;
- centre displacement and maximum edge strain;
- the exact FTD-0621 pinning index for the same `(L,w,d,phase axis)` class.

Define the dimensionless spline reaction defect

\[
\mathcal D_{w,d;i}=\frac{C_{\rm SPEED}
\left|\Delta P_{\rm matter}+\Delta P_{\rm spline}\right|}
{E_{\rm field,0}}.
\]

The registered preliminary infrared trend is strict decrease of both
`Pi_w` and `D_w` over `w=1,2,3` in the parallel and transverse `f=1/4`
classes. Three widths are not an asymptotic proof; this discriminator decides
whether the connected candidate deserves a larger FTD-0623 campaign.

Integer-phase controls require total matter impulse and centre displacement
`<=1e-8`; internal edge strain may be nonzero and is recorded. Cyclic `w=2`
controls must agree after component permutation to relative residual `<=1e-8`.

## 8. Verdicts

- `CONNECTED_MOORE_BLOCK_ACTION_CONSTRUCTIVE_IR_TREND_POSITIVE`: all 13
  forward/reverse arms and covariance gates pass, and both registered
  fractional classes decrease strictly in `Pi` and `D`. This licenses a
  separately locked stability/long-time/width campaign.
- `CONNECTED_MOORE_BLOCK_ACTION_CONSTRUCTIVE_IR_TREND_NEGATIVE`: all algebra,
  locality, inversion, and covariance gates pass, but `D` is not strictly
  decreasing in both classes. The action exists but is not promoted as the
  infrared repair.
- `CONNECTED_ACTION_SCALING_SOLVER_UNRESOLVED`: all `w<=2` arms pass but at
  least one `w=3` arm fails numerical completion without an analytic identity
  failure.
- `CONNECTED_MOORE_BLOCK_COMMON_ACTION_INVALID`: any `w<=2` algebra,
  locality, initialization, or inversion gate fails.

No verdict licenses stability, coherent transport, a fixed-mass limit,
electromagnetic charge, a physical particle, a pole, Lorentz recovery, or
production adoption.

## 9. Consequence lock

A positive construction advances to FTD-0623, which must find or reject a
stable dressed configuration and test multiple ticks/widths. A negative trend
closes only this unit-stiffness Moore graph; repairs may change the graph,
binding action, or ontology only as a new preregistered candidate. A solver-
unresolved verdict permits analytic/sparse solver work but no physical claim.

