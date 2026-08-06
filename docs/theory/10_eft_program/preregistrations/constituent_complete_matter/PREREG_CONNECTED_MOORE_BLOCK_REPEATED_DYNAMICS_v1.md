# FTD-0623 — Connected Moore-block repeated dynamics v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parent:** FTD-0622 result SHA-256
`6ED5287FB9AD84BACED79885E24E2352FE05CA82FA77636DD968297D6DF73396`  
**Scope:** selected repeated-dynamics feasibility; no production adoption  
**Date:** 2026-07-27

## 1. Question

Does the least nontrivial three-dimensional FTD-0622 connected integer object
remain coherent through repeated exact common-action updates, execute a real
site crossing under a uniform boost, and reconstruct its initial state by
state-only reversal?

This is a stability and transport gate for one selected object. It is not a
width limit, fixed-mass construction, particle pole, or native derivation of
the bond graph.

## 2. Frozen action and state

Use the FTD-0622 action without changing any equation or normalization:

- `L=17`, periodic boundary;
- exact `w=2` block bipole: 8 primitive `+1` and 8 primitive `-1`
  constituents;
- all 72 initial Moore-neighbour quartic edges with their registered rest
  lengths and `kappa=1`;
- quadratic-coat derived coupling, matched face/edge field update, production
  dispersion, endpoint discrete gradients, and the FTD-0468 interaction
  normalization;
- minimum-energy Gauss field, zero magnetic field, and integer phase `f=0`;
- `dt=1`, `C_SPEED=1/sqrt(3)`, action gate `1e-10`, solve tolerance
  `2e-11`, and at most 48 Newton iterations.

No damping, reaction, collision, graph rewiring, external force, neutralizer,
legacy force branch, fitted current, or post-hoc correction is admitted.

## 3. Registered arms

Each arm receives 16 consecutive forward solves followed by 16 consecutive
state-only reverse solves from its final state.

| arm | body orientation | uniform initial constituent momentum |
|---|---:|---|
| rest | `x` | `(0,0,0)` |
| parallel positive | `x` | `(+0.12,0,0)` |
| parallel negative | `x` | `(-0.12,0,0)` |
| transverse positive | `x` | `(0,+0.12,0)` |
| cyclic parallel | `y` | `(0,+0.12,0)` |

The free single-constituent speed corresponding to `|p|=0.12` is computed
from the production dispersion and recorded. No arm may be replaced after a
failure.

## 4. Per-tick exactness gates

Every forward and reverse step must satisfy:

- converged simultaneous solve;
- connected registered graph and unique site projection;
- continuity, Gauss, force, kinematic, kinetic-gradient, electric-adjoint,
  magnetic-work, binding-work, binding-impulse, matter-work, field-work, and
  total-energy residuals `<=1e-10`;
- causal-speed excess zero to `1e-12`;
- finite fields, energies, positions, momenta, and observables.

Across each complete forward trajectory, absolute total-energy drift from its
initial value must be `<=1e-9`. After the reverse trajectory, the complete
state must recover within `1e-8` under the existing state-difference metric.

## 5. Coherence and transport gates

Unwrap constituent positions using their continuous trajectory; the object is
far enough from the periodic boundary that no global wrap is expected.

Define the centre-subtracted shape error

\[
R_{\rm shape}(t)=\sqrt{\frac1{16}\sum_a
\left|[X_a(t)-X_{\rm cm}(t)]-[X_a(0)-X_{\rm cm}(0)]\right|^2}.
\]

For every arm require:

- maximum squared-edge strain `<=0.25`;
- maximum `R_shape <=0.25` cell;
- graph and unique site projection remain valid at every tick.

The rest arm additionally requires centre displacement `<=1e-8`, total
matter-momentum norm `<=1e-8`, and zero site hops.

Every boosted arm requires:

- projected centre displacement in its launch direction `>=0.75` cell;
- total centre displacement `<=1.5` times the free-dispersion displacement;
- transverse centre displacement `<=0.10` cell;
- at least 16 legitimate constituent site hops in aggregate;
- final projected centre velocity, defined as the average production velocity
  reconstructed from the 16 final constituent momenta, has the launch sign.

The positive and negative parallel arms must be sign mirrors: the sum of their
centre-displacement vectors, the difference of their scalar field-energy
histories, and the difference of their shape-error histories must each remain
`<=1e-8`.

The positive parallel and cyclic-parallel arms must agree after the cyclic
map `(x,y,z)->(z,x,y)` to `<=1e-8` for centre displacement, total matter
momentum, field energy, shape error, edge strain, hop count, and accumulated
normalized spline translation-reaction defect.

## 6. Recorded diagnostics

Record every tick's centre, total matter momentum, all energy sectors, maximum
edge strain, shape error, site hops, maximum common-action residual, local and
spline matter-plus-field defects, and the cumulative normalized spline defect

\[
\mathcal D_{\rm cum}(t)=\frac{C_{\rm SPEED}
\left|\sum_{n\le t}\delta P_{\rm spline,n}\right|}{E_{\rm field,0}}.
\]

`D_cum` is diagnostic, not an exact conservation gate. FTD-0619 already shows
that the spline momentum is not the exact coupled Noether charge.

## 7. Verdicts

- `CONNECTED_INTEGER_OBJECT_REPEATED_MOBILITY_CONSTRUCTIVE`: every exactness,
  coherence, rest, transport, sign-mirror, covariance, and inverse gate passes.
- `CONNECTED_INTEGER_OBJECT_STABLE_BUT_MOBILITY_NEGATIVE`: exactness, inverse,
  rest, and bounded-coherence gates pass, but at least one registered boost
  fails displacement, hop, direction, sign-mirror, or covariance.
- `CONNECTED_INTEGER_OBJECT_REPEATED_DYNAMICS_INVALID`: any exactness,
  inversion, rest-stability, projection, graph, or bounded-coherence gate fails.

No verdict licenses a fixed mass, free-particle dispersion, gapless mode,
charge, `U(1)`, particle pole, Lorentz recovery, unitarity, or production use.

## 8. Consequence lock

A constructive mobility verdict licenses a separately preregistered normal-
mode and boost-dispersion campaign. A stable-but-immobile verdict closes only
this launch protocol. An invalid verdict closes repeated dynamics for the
unit-stiffness reference-Moore graph and requires a new versioned binding or
relational-state candidate. It does not authorize threshold changes.
