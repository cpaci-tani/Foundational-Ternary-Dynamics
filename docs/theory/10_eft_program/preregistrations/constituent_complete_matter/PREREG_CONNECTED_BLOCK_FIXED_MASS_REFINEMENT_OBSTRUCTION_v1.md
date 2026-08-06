# FTD-0647 — Connected-block fixed-mass refinement obstruction v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parent:** FTD-0646  
**Scope:** selected connected-block common action only; no production change

## Question

Can the current connected ternary bipole be made progressively wider while
retaining a finite nonzero rest mass if every constituent and action
coefficient remains fixed?

This campaign tests only the naive frozen-coefficient refinement. It does not
test a running lattice spacing, a scale-dependent cell measure, a collective
mass term, a vacuum subtraction, or a different graph/action.

## Frozen definitions

For a width `w` connected block bipole,

\[
N(w)=2w^3.
\]

The selected matter-plus-field energy is

\[
H=\sum_{a=1}^{N}
\sqrt{E_0^2+c^2|p_a|^2}
+\frac{\kappa}{4}\sum_{(ab)}
\bigl(|x_a-x_b|^2-\ell_{ab}^2\bigr)^2
+\beta\widetilde H_{EB},
\]

where `E_0=E_REST=M_INERTIAL*C_SPEED^2`, `kappa` is the unchanged
connected-block binding stiffness, `beta` is the unique FTD-0478 face-field
normalization, and

\[
\widetilde H_{EB}=\frac12\|E\|^2+\frac12\|B\|^2
-\frac{\lambda}{2}\langle B,C^T E\rangle,
\qquad \lambda=C_{\rm SPEED}.
\]

For the periodic cubic matched complex, the Fourier symbol of the curl obeys
`||C|| <= 2*sqrt(3)`. At `lambda=1/sqrt(3)`, Cauchy--Schwarz and
`2uv <= u^2+v^2` therefore give `H_tilde_EB >= 0`. The binding term is a
sum of squares and `beta>0`. Hence

\[
H_{\rm rest}(w)\ge N(w)E_0=2w^3E_0,
\qquad
M_{\rm rest}(w)=H_{\rm rest}(w)/c^2
\ge 2w^3M_{\rm INERTIAL}.
\]

The same additive obstruction appears in the minimum kinetic energy at fixed
total momentum `P`. Convexity gives

\[
\sum_a h(p_a)\ge N h(P/N)
=NE_0+\frac{|P|^2}{2NM_{\rm INERTIAL}}+O(|P|^4),
\]

so the uniform collective kinetic sector has inertial mass
`N*M_INERTIAL`, not a width-independent mass.

## Locked engine certificate

Use the unchanged initializer and energy functions at:

- `L=17`;
- widths `w={1,2,3,4}`;
- orientations `x,y,z`;
- zero remainder and zero momentum;
- the existing Poisson dressing, face normalization, binding graph, and
  production constants.

For every one of the 12 arms, record constituent count, net polarity,
constituent rest sum, binding energy, modified field energy, total energy,
rest floor, and collective inertial-mass floor.

## Gates

All of the following are conjunctive:

1. initialization, graph locality/connectivity, site projection, and Gauss
   gates pass;
2. `N=2w^3` and net polarity is zero exactly;
3. the zero-momentum constituent sum equals `N*E_REST` within `1e-14`
   relative;
4. binding energy is nonnegative and below `1e-14` at the reference graph;
5. modified field energy is nonnegative within `1e-12` absolute;
6. total energy is at least `N*E_REST` within `1e-12` absolute;
7. cubic copies agree in all scalar energies within `1e-10` relative;
8. the measured rest floor divided by `w^3` equals `2*E_REST` within
   `1e-14` relative;
9. the measured inertial-mass floor divided by `w^3` equals
   `2*M_INERTIAL` within `1e-14` relative.

## Locked verdicts

- `FROZEN_ADDITIVE_CONSTITUENT_FIXED_MASS_REFINEMENT_CLOSED` if every gate
  passes. This is a theorem/certificate that the present width sequence cannot
  be a fixed-mass refinement family.
- `FIXED_MASS_OBSTRUCTION_EXECUTION_INVALID` if initialization, bookkeeping,
  cubic covariance, or an independently stated algebraic gate fails.
- `MODIFIED_ENERGY_POSITIVITY_ASSUMPTION_FALSIFIED` if a valid arm has
  modified field energy below `-1e-12`; the proposed lower-bound proof is then
  not promoted until the counterexample is resolved.

No intermediate numerical trend can change these verdicts.

## Consequence policy

A closed verdict does not close extended matter. It closes only refinement by
adding more identical fixed-mass constituents under unchanged coefficients.
The next candidate must declare one of the following before execution:

1. a scale-dependent cell measure/rest coefficient;
2. a collective graph mass functional;
3. a background/vacuum term that changes the energy and inertia bookkeeping;
4. a finite-constituent carrier whose pinning is reduced by another derived
   mechanism.

Each is a fresh selected action. Constructive equation and ontology changes
are allowed under the FTD-0598 repair policy, but no locked result may be
tuned post hoc.
