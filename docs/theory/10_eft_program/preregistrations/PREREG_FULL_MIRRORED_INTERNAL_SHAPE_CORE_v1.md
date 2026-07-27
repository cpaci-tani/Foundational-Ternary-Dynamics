# FTD-0605 — Full mirrored internal-shape matter core v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`  
**Scope:** observer-only local deformation discriminator using the unchanged
FTD-0601 common-action transaction and FTD-0602 minimum-energy Gauss field.  
**Production change:** forbidden.  
**Protocol lock:** `protocol_sha256=388926B3947F0C0A378FC3B52BD99E3C94D8F9BBB0A4D325E26CE1252B79C70F`

## 1. Ontological question

Can all internal coordinates already contained in the selected six-
constituent state adapt coherently to fractional lattice phase, or is the
compact composite still pinned after breathing, shear, and orientation are
released together?

This protocol adds no force, history, constituent, field component, or
persistent variable. It tests the full six-dimensional local zero-centroid
shape space continuously connected to the FTD-0601 trimer.

## 2. Frozen configuration family

Use the FTD-0602 neutral pair at `L=17`, with unchanged charges, group centres,
relative centre separation, quadratic polarity coat, face-flux normalization,
minimum-energy periodic Gauss field, production dispersion, quartic
intratrimer binding, and common-action solver.

Let the first trimer offsets be

\[
 r_0=r_0^{(0)}+u_0,\qquad
 r_1=r_1^{(0)}+u_1,\qquad
 r_2=-r_0-r_1,
\]

where `u_0,u_1 in R^3`. The second trimer uses the exact charge-conjugate
mirror offsets `-r_a`. At common principal-axis phase `f`,

\[
 X_{A,a}=C_A+f e_x+r_a,
 \qquad
 X_{B,a}=C_B+f e_x-r_a.
\]

The two centres therefore remain fixed exactly. The registered local identity
basin is `|u_i| <= 0.20` componentwise. A solution within `1e-4` of the basin
boundary is a boundary failure; the interval is not widened after inspection.
Every internal pair distance must remain in `[0.5,2.0]`.

## 3. Static energy and fast evaluator

At each trial shape, rebuild the exact coated density and minimize

\[
 U_f(u)=V_{\rm bind}(u)
 +\beta\,\frac12\langle\rho(u),G_L\rho(u)\rangle,
\]

where `G_L` is the zero-mean periodic inverse of `D D^T`. Construct `G_L` once
from a zero-mean point source using deterministic conjugate gradients with
maximum residual `1e-15`. Evaluate the quadratic form by direct sparse
convolution; this is an acceleration of the same minimum-energy field, not a
new field model.

At every reported minimum, independently solve `D D^T phi=rho` from zero and
construct `E_min=D^T phi`. Require Gauss, curl-adjoint, and fast/direct energy
agreement at most `1e-11`.

## 4. Locked optimizer and campaign

For `f=j/32`, `j=0,...,31`, start at `u=0` independently and run deterministic
six-dimensional Nelder-Mead with coefficients `(1,2,1/2,1/2)`, coordinate
simplex step `0.01`, maximum `900` objective evaluations, simplex-diameter
tolerance `2e-8`, and energy-spread tolerance `1e-14`. No warm start or
favourable-phase selection is allowed.

At the returned point:

1. compare with the undeformed reference energy;
2. compute a central finite-difference gradient with `h=1e-4`;
3. compute the symmetric `6x6` Hessian with `h=2e-3` and its eigenvalues;
4. initialize the independently solved minimum-energy face field;
5. run one unchanged FTD-0601 forward step and a state-only reverse step;
6. record inward impulse, separation change, common-action residuals,
   inversion, shape coordinates, internal distances, and pseudomomentum;
7. compare `f=1` with exact integer translation of the `f=0` relaxed state.

## 5. Gates

- all 32 optimizations terminate within 900 evaluations;
- every minimum remains at least `1e-4` inside the registered shape basin;
- every internal pair distance remains in `[0.5,2.0]`;
- maximum gradient component is at most `5e-7`;
- the Hessian has no eigenvalue below `-5e-6` and has at least three
  eigenvalues above `1e-3`;
- relaxed energy never exceeds reference energy by more than `1e-12`;
- Green-kernel construction, direct Gauss, curl-adjoint, and energy cross-
  checks are at most `1e-11`;
- all forward common-action gates are at most `1e-12`;
- all state-only inverse residuals are at most `1e-10`;
- exact integer translation agrees to `1e-12`;
- phase-robust attraction requires inward impulse greater than `1e-10` and
  decreasing centre separation at every phase.

Record reference and relaxed barriers. No post-hoc barrier threshold changes
the force-sign verdict.

## 6. Verdicts

- `FULL_MIRRORED_SHAPE_PHASE_ROBUST_CONSTRUCTIVE`: all gates pass and all 32
  phases are attractive;
- `FULL_MIRRORED_SHAPE_RELAXES_BUT_FORCE_SIGN_FAILS`: all static and
  transaction gates pass but at least one phase is non-attractive;
- `FULL_MIRRORED_SHAPE_STATIC_BRANCH_CLOSED_NEGATIVE`: any optimizer,
  interior, distance, gradient, Hessian, direct-field, energy, common-action,
  inverse, or periodicity gate fails;
- `FULL_MIRRORED_SHAPE_UNRESOLVED`: no earlier classification applies.

A negative result closes only the registered compact local shape basin. It
does not close a field-dressing deformation outside constituent geometry or a
native extended low-momentum carrier. A constructive result licenses only
this selected deformable-core existence claim. No verdict licenses a physical
particle, electron, electromagnetic ontology, pole, Lorentz recovery, toggle,
scenario, or production adoption.
