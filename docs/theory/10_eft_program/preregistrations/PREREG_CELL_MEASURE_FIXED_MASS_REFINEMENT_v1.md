# FTD-0648 — Cell-measure fixed-mass refinement v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parent:** FTD-0647  
**Scope:** observer-only selected scaling law; no production or common-action change

## Question

Does the ordinary three-dimensional cell measure convert the exact connected
block sequence into a fixed-mass, fixed-integrated-polarity family whose
absolute static Peierls barrier decreases with resolution?

## Locked physical interpretation

The FTD-0645/0646 width-two object is the reference discretization. A width
`w` member represents the same physical-sized body on a lattice with relative
cell length

\[
a_w=2/w.
\]

This compares selected actions at different resolutions. It is not evolution
of one fixed fundamental lattice and is not derived from the five postulates.

The cell-volume law fixes

\[
r_m(w)=r_q(w)=r_\kappa(w)=a_w^3=(2/w)^3.
\]

The face field satisfies discrete Gauss with unit-source fields. To keep the
integrated positive and negative polarity magnitudes fixed, scale the source
and current by `r_q`. The physical Maxwell sum has one inverse-length factor,
so its coefficient scales as

\[
r_\beta(w)=a_w^{-1}=w/2.
\]

Therefore a unit-source field energy or Peierls barrier `X_unit` maps to

\[
X_{\rm phys}(w)=\beta_0 r_\beta(w)r_q(w)^2 X_{\rm unit}(w),
\]

where `beta_0` is the unchanged FTD-0478 face normalization.

The frozen extensive identities become

\[
2w^3r_m(w)E_{\rm REST}=16E_{\rm REST},
\qquad
w^3r_q(w)=8
\]

for every `w`.

## Locked matrix

- widths `w={2,3,4,5,6,8}`;
- `L=8w+1` for a common asymptotic physical box convention;
- all three bipole orientations;
- all three translation-phase axes;
- integer phase `0` and half phase `1/2`;
- zero momenta and the existing minimum-energy longitudinal matched field;
- existing unit-charge ternary constructor, followed only by the declared
  analytic action/source rescaling.

This gives 54 orientation/translation/width arms and 108 field solves.

## Observables

For every arm record:

1. exact constituent count and net polarity;
2. scaled integrated positive and negative polarity;
3. scaled total rest energy and inertial mass;
4. integer-phase and half-phase scaled field energy;
5. absolute scaled Peierls barrier, defined as
   `E_integer-E_half`;
6. relative pinning index `barrier/E_integer`;
7. cubic covariance.

For each cubic orbit fit log-log slopes on held-out widths `{4,5,6,8}` for
scaled integer-phase field energy and absolute scaled barrier.

## Gates

All are conjunctive:

1. every initializer, graph, site, Gauss, and positivity gate passes;
2. `N=2w^3`, net polarity zero, and both integrated polarity magnitudes equal
   `8` within `1e-13`;
3. total rest energy equals `16*E_REST` and inertial mass equals
   `16*M_INERTIAL` within `1e-13`;
4. every absolute scaled Peierls barrier is positive;
5. the barrier strictly decreases at every successive registered width in
   every cubic arm;
6. the held-out scaled-field-energy slope has absolute value at most `0.25`;
7. the held-out absolute-barrier slope lies in `[-3.5,-2.5]`;
8. the width-eight/width-four scaled field-energy ratio lies in `[0.8,1.2]`;
9. cubic relative residuals are below `1e-10`.

## Locked verdicts

- `CELL_MEASURE_FIXED_MASS_STATIC_DEPINNING_CONSTRUCTIVE` if all gates pass.
- `CELL_MEASURE_FIXED_MASS_SCALING_CLOSED` if algebra/initialization/covariance
  passes but any energy or barrier scaling gate fails.
- `CELL_MEASURE_REFINEMENT_EXECUTION_INVALID` if the locked matrix or an exact
  bookkeeping gate fails.

No tolerance or exponent may be changed after execution.

## Consequence policy

A constructive verdict licenses parameterizing the selected common action by
`r_m,r_q,r_kappa,r_beta` in a new default-off research branch. It does not
license a particle, pole, Lorentz, charge, production, or native-ontology
claim. A closed verdict retains FTD-0647 and closes this cell-measure repair.
