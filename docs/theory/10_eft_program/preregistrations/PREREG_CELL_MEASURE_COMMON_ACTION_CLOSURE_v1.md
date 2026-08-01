# FTD-0649 — Cell-measure common-action closure v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`  
**Parent:** FTD-0648  
**Scope:** default-off research parameters in the selected connected-block action

## Question

Can the four FTD-0648 resolution factors be inserted into one reciprocal
matter--field transaction while preserving exact current, Gauss, work, energy,
causality, cubic covariance, and state-only inversion?

This is an action-compatibility gate. It does not test long-horizon depinning.

## Frozen action

For width `w`, define `a=2/w` and

\[
r_m=r_q=r_\kappa=a^3,\qquad r_\beta=a^{-1}.
\]

The connected action is changed only as follows:

1. constituent dispersion becomes
   \[
   h_w(p)=\sqrt{(r_mE_{\rm REST})^2+C_{\rm SPEED}^2|p|^2};
   \]
2. the exact quadratic-coat density and deposited face current are both
   multiplied by `r_q`;
3. the electric and magnetic orbit gather uses effective polarity
   `r_q*s`;
4. the matched field energy/work coefficient is `beta_0*r_beta`;
5. the existing Moore binding stiffness is multiplied by `r_kappa`.

The initial minimum-energy longitudinal field is multiplied by `r_q`.
No post-step correction, external force, route variable, or separately scaled
recoil is allowed.

Defaults for every new action parameter are exactly one. Therefore all earlier
connected-block tests retain their original equations.

## Locked implementation identities

The same `r_q` must occur in all four places: initial Gauss source, deposited
current, density used by Gauss diagnostics, and orbit-gather force. The same
`beta_0*r_beta` must multiply field energy, current work, electric impulse,
magnetic impulse, and field momentum. The same `r_m` must define energy,
velocity, launch momentum, and the discrete kinetic gradient.

Any missing occurrence is an execution defect, not a tunable coefficient.

## Locked matrix

- widths `w={2,3,4}` with `L=8w+1`;
- all three bipole orientations;
- all three Cartesian launch axes;
- positive uniform lattice velocity magnitude `0.01` for all 27 primary arms;
- negative-launch controls for orientation `0` in all three axes and all three
  widths: 9 controls;
- zero-launch controls in all three orientations and widths: 9 controls;
- one forward step followed by one state-only reverse step per arm;
- zero fractional phase, the existing minimum-energy longitudinal dressing,
  finite chart fibre enabled, and no transverse seed.

Total: 45 arms, 90 common-action solves.

Launch momentum is computed from the scaled dispersion, not copied from the
unit-mass action.

## Gates

Every arm must satisfy:

1. initializer, graph, chart, and action validation;
2. exact scale values and fixed `E_rest=16*E_REST`,
   `M=16*M_INERTIAL`, `Q+=Q-=8` within `1e-13`;
3. nonlinear root residual and force residual `<=1e-9`;
4. continuity, Gauss before/after, kinetic-gradient, electric-adjoint,
   magnetic-work, binding-work, binding-impulse-sum, matter-work, field-work,
   and total-energy residuals `<=1e-9`;
5. causal-speed excess `<=1e-12`;
6. state-only inverse recovery `<=1e-8`;
7. no graph/locality or finite-fibre violation;
8. positive/negative controls mirror center displacement, matter momentum,
   field energy, and recovery within `1e-7`;
9. cyclic orientation/axis copies agree in all scalar residuals and rotated
   center displacement within `1e-7`;
10. every zero-launch control has center displacement `<=1e-6` for this
    one-step action-closure gate.

The existing focused width-two default-scale common-action and inverse tests
must remain green without changing their frozen outputs or thresholds.

## Locked verdicts

- `CELL_MEASURE_RECIPROCAL_COMMON_ACTION_CONSTRUCTIVE` if all gates pass.
- `CELL_MEASURE_COMMON_ACTION_CLOSED` if all roots execute but an exact
  action/work/Gauss/inverse gate fails.
- `CELL_MEASURE_COMMON_ACTION_EXECUTION_INVALID` if coverage, initialization,
  source scaling, or root convergence prevents the conjunction from being
  evaluated.

No width, factor, velocity, tolerance, or classification may change after
execution.

## Consequence policy

A constructive verdict licenses a separate long-horizon resolution campaign.
It does not license production adoption, a particle pole, electromagnetic
charge, Lorentz recovery, or a claim that the scale law is forced by the five
postulates.
