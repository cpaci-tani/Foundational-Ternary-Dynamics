# FTD-0604 — Symmetric breathing matter core v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`  
**Scope:** observer-only deformation discriminator using the unchanged
FTD-0601 common-action transaction and FTD-0602 minimum-energy Gauss field.  
**Production change:** forbidden.  
**Protocol lock:** `protocol_sha256=CD8DB5F38A6E9F01BB8EDFAF63664EF940BF0D1F87C1CE8BF5B17789616FDACE`

## 1. Ontological question

Can the existing constituent phase space support a self-adjusting localized
matter core whose internal state responds to lattice phase, or does even the
least arbitrary internal deformation leave the FTD-0603 force-sign reversal?

This is not a new force or primitive. It activates the totally symmetric
internal breathing coordinate already contained in the six constituent
positions.

## 2. Frozen matter family

Use the FTD-0602 neutral pair at `L=17`, with unchanged charges, group centers,
relative center separation, quadratic polarity coat, minimum-energy periodic
Gauss solve, production dispersion, quartic intratrimer binding, normalization,
and common-action solver.

Let `r_a` be the three zero-sum reference offsets in the first trimer. The
second trimer has the exact charge-conjugate mirror offsets `-r_a`. At common
translation phase `f` along the principal pair axis, define

\[
 X_{A,a}=C_A+f e_x+\lambda r_a,
 \qquad
 X_{B,a}=C_B+f e_x-\lambda r_a.
\]

The single coordinate `lambda` is derived from constituent positions. Search
only the registered local basin `0.8 <= lambda <= 1.2`; a solution within
`1e-4` of either boundary fails the interior-core gate rather than widening
the interval.

## 3. Static common energy

For every candidate `lambda`, rebuild the exact coated density, solve

\[
 D D^T\phi=\rho,\qquad E_{\min}=D^T\phi,
\]

from zero with residual at most `1e-13`, and evaluate

\[
 U_f(\lambda)=V_{\rm bind}(\lambda)
 +\beta\,\frac12\lVert E_{\min}(\lambda)\rVert^2,
\]

where `beta` is the unchanged face-flux work normalization. Use deterministic
golden-section minimization, interval tolerance `1e-10`, maximum 96 iterations.
No physical target or observed force enters the minimization.

## 4. Locked campaign

For `f=j/32`, `j=0,...,31`:

1. evaluate the rigid `lambda=1` energy;
2. solve the breathing minimum `lambda_star(f)`;
3. verify central finite-difference stationarity with `h=1e-5`;
4. verify positive curvature with the same stencil;
5. initialize the relaxed minimum-energy field and zero constituent momenta;
6. run one unchanged FTD-0601 forward step and a state-only reverse step;
7. record inward impulse, separation change, all common-action residuals,
   inverse residual, internal distances, and pseudomomentum defect.

Run `f=1` as the exact integer-translation control against `f=0`.

## 5. Gates

- all 32 minima are interior by at least `1e-4`;
- every static Gauss residual is at most `1e-12`;
- `|dU/dlambda| <= 1e-8` and `d2U/dlambda2 > 1e-6` at every minimum;
- relaxed energy never exceeds rigid energy by more than `1e-12`;
- all forward common-action gates are at most `1e-12`;
- all state-only inverse residuals are at most `1e-10`;
- `f=1` agrees with integer translation of `f=0` to `1e-12`;
- phase-robust attraction requires inward impulse greater than `1e-10` and
  decreasing separation at all 32 phases.

Define the rigid and relaxed static barriers as `max_f U-min_f U`. A barrier
reduction is recorded, but no post-hoc reduction threshold changes the force-
sign verdict.

## 6. Verdicts

- `SYMMETRIC_BREATHING_CORE_PHASE_ROBUST_CONSTRUCTIVE`: every gate passes and
  attraction is phase robust;
- `SYMMETRIC_BREATHING_RELAXES_BUT_FORCE_SIGN_FAILS`: the interior stable
  relaxation and common-action gates pass, but any phase is non-attractive;
- `SYMMETRIC_BREATHING_CORE_STATIC_BRANCH_CLOSED_NEGATIVE`: any optimizer,
  interior, curvature, Gauss, common-action, inverse, or periodicity gate fails;
- `SYMMETRIC_BREATHING_CORE_UNRESOLVED`: no prior classification applies.

A constructive result would license only this selected deformable-core
existence claim. A force-sign failure closes only the one-coordinate symmetric
breathing mode, not all internal deformation. No verdict licenses a particle,
electron, electromagnetic ontology, continuum pole, Lorentz recovery, toggle,
or scenario.
