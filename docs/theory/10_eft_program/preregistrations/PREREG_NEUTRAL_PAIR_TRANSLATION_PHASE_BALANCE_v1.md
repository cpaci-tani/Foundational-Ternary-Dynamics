# FTD-0603 — Neutral-pair translation-phase balance v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Scope:** observer-only discriminator using the unchanged FTD-0601 transaction
and the FTD-0602 minimum-energy Gauss initializer.  
**Production change:** forbidden.  
**Protocol lock:** `protocol_sha256=9C88B2B593C2E31EA08999010E71EF85204ECB3F8C63AA248B7A86A937E16595`

## 1. Question

Does the FTD-0602 matter-plus-field pseudomomentum defect behave as reversible
exchange with the fixed cubic lattice, or does it contain a secular component
that cannot be explained by the absence of continuous translation symmetry?

This protocol does not assume that a cubic lattice has an exactly conserved
ordinary momentum. It separates two observables:

1. the relative inward impulse between the two composites; and
2. the total centre-of-mass impulse and matched-field pseudomomentum defect.

## 2. Frozen system

Use exactly the FTD-0602 six-constituent neutral pair, charges, rest geometry,
quartic intratrimer binding, quadratic polarity coat, production dispersion,
matched face/edge field update, implicit endpoint solver, tolerances, and
minimum-energy periodic Gauss initialization. No coupling, normalization,
potential, field equation, or root-selection change is permitted.

For a phase `u`, rigidly translate all six effective positions by `u` along
one Cartesian axis, re-express them in the existing centred `(anchor,remainder)`
chart, and recompute the unique zero-mean minimum-energy Gauss field. This is a
new initial condition, not an interpolated translation of a prior field.

## 3. Locked campaign

For each axis `a in {x,y,z}` and each phase resolution `N in {8,16,32}`, run
one forward step from rest at phases

\[
 u_j=j/N,\qquad j=0,\ldots,N-1.
\]

Also run `u=1` and compare it with the integer-translated `u=0` result. Record
for every arm:

- all FTD-0602 initializer and common-action residuals;
- relative inward impulse and separation change;
- total matter impulse;
- matched-field pseudomomentum change;
- total pseudomomentum defect.

For each `(axis,N)`, compute the phase mean of the component parallel to that
axis. Define `M_N` as the maximum absolute mean across the three axes, both
for total matter impulse and for total pseudomomentum defect.

## 4. Locked gates and verdicts

Algebraic/numerical gates:

- every initializer and common-action residual is at most `1e-12`;
- `u=1` agrees with integer translation of `u=0` to `1e-12`;
- all 168 registered phase arms are attempted (`3*(8+16+32)`);
- every phase has inward impulse greater than `1e-10` and decreases the
  composite separation on the first step.

Classify the centre-of-mass and pseudomomentum means separately:

- **phase-balanced:** `M_32 <= 1e-8` and either `M_32 <= M_16/2` or
  `M_32 <= 1e-12`;
- **secular:** `M_32 > 1e-8`, `|M_32-M_16| <= 0.1 M_32`, and
  `|M_16-M_8| <= 0.2 M_16`;
- **unresolved:** neither condition holds.

The registered verdict is:

- `RELATIVE_ATTRACTION_WITH_PHASE_BALANCED_LATTICE_EXCHANGE` if attraction is
  robust and both means are phase-balanced;
- `RELATIVE_ATTRACTION_WITH_SECULAR_MOMENTUM_DEFECT` if attraction is robust
  and either mean is secular;
- `RELATIVE_ATTRACTION_PHASE_BALANCE_UNRESOLVED` if attraction is robust but
  neither prior verdict applies;
- `TRANSLATION_PHASE_COMMON_ACTION_CLOSED_NEGATIVE` if an initializer or
  common-action gate fails;
- `TRANSLATION_PHASE_ATTRACTION_NOT_ROBUST` if the algebraic gates pass but
  attraction changes sign at any phase.

## 5. Interpretation lock

A phase-balanced result licenses only the statement that the observed defect
is consistent with bounded periodic exchange with the nondynamical lattice.
It does not prove a microscopic momentum theorem. A secular result licenses
only a missing momentum-carrier diagnosis for this selected field-strength
state. An unresolved result requires a separately preregistered refinement.

No verdict licenses a particle identification, electromagnetic ontology,
Lorentz recovery, continuum pole, production toggle, or scenario.
