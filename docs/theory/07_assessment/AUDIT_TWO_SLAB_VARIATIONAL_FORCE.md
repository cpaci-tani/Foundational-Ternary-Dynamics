# FTD-0485 — Two-slab variational force

**Date:** 2026-07-25  
**Status:** `[THEOREM — INTERIOR VARIATIONAL FORCE] + [CLOSED NEGATIVE — FROZEN THRESHOLD UNIQUENESS]`  
**Verdict:** `INTERIOR_VARIATIONAL_FORCE_DERIVED_THRESHOLD_NONUNIQUE`

## Interior result

The exact `FTD-0484` connection action was differentiated at the shared point
of two adjacent worldline slabs with three-component forward automatic
differentiation. Within one cell, the Q1/Nedelec integrand is cubic or lower,
so the fixed two-point Gauss--Legendre evaluation is algebraically exact.

All interior gates pass:

- direct action equals the independent deposited-current action exactly on
  both slabs;
- arbitrary three-time-slice gauge transformations change the interior
  impulse by only `3.69e-18`;
- a nonzero pure gauge gives residual impulse `2.60e-18`;
- a stationary polarity in a time-varying transverse connection receives the
  exact nonzero impulse `0.021361959960016143`;
- an affine connection gives the exact magnetic curvature impulse with
  residual `1.56e-17`;
- polarity reversal and proper-axis rotation are exact below `1e-12`.

The implementation contains no legacy field interpolation, Poisson force,
`grad|J|`, displacement division, or explicit vector cross product. Thus the
missing `FTD-0480` transverse components are genuinely supplied by varying the
same action that supplies the current.

## Threshold failure

The compact Q1/Nedelec representation is only piecewise differentiable. Its
normal link component may differ on the two cells adjacent to an integer
plane. The locked fixture uses such an allowed connection and evaluates a
stationary worldline at `x=5+/-1e-8`:

```text
left impulse   =  0.011547005383792516
right impulse  = -0.017320508075688766
absolute gap   =  0.028867513459481284
required gap   <  1e-12
```

The action is continuous as a line integral, but the force is not uniquely
defined at the existing hop threshold for a generic allowed connection. A
one-sided convention, subgradient, or smoother wider-support basis would be a
new selection. None is introduced.

## Consequence

The conceptual origin problem is solved in cell interiors: current and force
are two variations of one selected action. The frozen compact representation
nevertheless fails the production uniqueness gate precisely where a remainder
triggers a lattice hop. Together with the independent connection and
history/canonical-momentum cost identified by `FTD-0484`, this blocks
`common_action_face_dynamics` for the frozen variables.

The matched field-action normalization boundary also remains: exact source
normalization produces the magnetic term `v cross B/C_SPEED`, not the frozen
`FTD-0479` coefficient.

Run of record: `engine/results/ftd_0485/windows_msvc_cpu.json`.
