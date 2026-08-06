# Connected-block analytic static and dynamical rest

**Campaigns:** FTD-0637 through FTD-0639  
**Status:** `[DERIVED — FIXED-SECTOR STATIC JET] + [MEASURED — ANALYTIC
STATIC BASIN AND REVERSIBLE DYNAMICAL REST]`  
**Production impact:** none

## Analytic static jet

For the selected quadratic coat, let `rho(x)` be the deposited neutral density
and let the periodic zero-mean potential satisfy

\[
(-\Delta)\phi=\rho.
\]

The minimized longitudinal field energy is

\[
U_F(x)=\frac{\beta}{2}\rho(x)^T\phi(x).
\]

Inside one quadratic-B-spline polynomial sector, the envelope theorem and the
linearity of the Poisson solve give

\[
\partial_i U_F=\beta\,\rho_i^T\phi,
\qquad
\partial_i\partial_j U_F
=\beta\bigl(\rho_i^T(-\Delta)^+\rho_j+\phi^T\rho_{ij}\bigr).
\]

Adding the exact Moore-edge derivatives produces the complete 48-coordinate
gradient and Hessian without a finite-difference step. FTD-0637 verifies the
charge, derivative-charge, derivative-dipole, Poisson, energy, translation,
and cyclic identities. The analytic matrix agrees with the FTD-0636
knot-local finite-difference matrix to `5.25e-7`. Its minimum eigenvalue is
`0.0019084607`; all 48 eigenvalues are positive.

FTD-0637 nevertheless returns
`CONNECTED_BLOCK_ANALYTIC_NONSTATIONARY`: the FTD-0633 position has analytic
gradient infinity norm `1.1204e-8`, 12% above the locked `1e-8` gate. This
confirms that FTD-0636's narrow gradient failure was a real residual force, not
a spline-crossing artifact.

## Full-coordinate refinement

FTD-0638 applies the registered Newton equation

\[
H\,\delta x=-g
\]

without projection back into the four-coordinate family. One full step is
accepted in each cyclic arm. The largest constituent displacement is only
`6.81e-10`, far inside the original `9.47e-5` knot clearance. The stable
energy decrement is about `4.6e-17`, while the analytic gradient falls to
`1.35e-14`. The final minimum eigenvalue is `0.0019084783`, and cyclic energy,
spectrum, and displacement covariance all pass by more than three orders of
magnitude.

The result is
`CONNECTED_BLOCK_ANALYTIC_STATIC_BASIN_CONSTRUCTIVE`: the selected action has
a genuine 48-coordinate local classical minimum under the finite eight-record
chart.

## Dynamical rest

FTD-0639 rebuilds the longitudinal dressing at the analytic minimum and runs
the unchanged constituent common-action map for 128 forward and 128
state-only reverse ticks in both orientations. Across 512 recorded steps:

| diagnostic | worst value | gate |
|---|---:|---:|
| common-action residual | `4.066e-14` | `1e-10` |
| constituent impulse | `1.944e-14` | `1e-9` |
| full-state excursion | `3.053e-16` | `1e-8` |
| centre displacement | `0` | `1e-10` |
| energy drift | `4.441e-16` | `1e-12` |
| inverse recovery | `2.776e-16` | `1e-10` |

There are no hops, spline-sector crossings, or cap violations. The result is
`CONNECTED_BLOCK_ANALYTIC_DYNAMICAL_REST_CONSTRUCTIVE`.

## What this changes

The matter mainline now contains an observer-level, finite, neutral,
connected, dressed constituent configuration that is simultaneously:

- a stationary point of the complete 48-coordinate selected static action;
- a strictly positive local basin inside its occupied coat sector;
- an actual numerical fixed point of the reversible common-action tick;
- state-only invertible over the registered finite history.

No new production primitive was required beyond the already selected
constituent phase space and finite chart fibre. The result does not show that a
single ternary voxel is matter; the object has 16 signed constituents, a Moore
binding graph, a continuous subcell chart, and a self-consistent face field.

## Remaining boundary

This is rest, not propagation. FTD-0640 subsequently qualifies all 48 analytic
small-amplitude matter-coordinate modes and is the controlling successor for
that question. Independent field modes, finite boost/depinning from the
centered state, and eventually identifiable retarded poles remain open. Until
those pass, `particle`, `mass`, `photon`, `charge`, and infrared Lorentz claims
remain open.
