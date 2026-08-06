# FTD-0707 — L=33 symmetry-rest refinement v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Parent:** FTD-0706

## Question

Can the nonstationary `L=33` control exposed by FTD-0706 be repaired using
only the existing constituent positions and matched face/edge fields, by
refining the four symmetry-preserving matter coordinates and redressing the
field from Gauss after every trial?

## Frozen candidate

Begin from the exact FTD-0704/0706 `L=33`, orientation-0 rest preparation.
Keep charges, constituent count, edge graph, edge rest lengths, coat, action,
normalization, volume, and field variables unchanged. Introduce four derived
optimization coordinates, all initially zero:

1. signed axial displacement of the eight outer constituents;
2. signed axial displacement of the eight inner constituents;
3. signed transverse displacement of outer constituents on both transverse
   axes;
4. signed transverse displacement of inner constituents on both transverse
   axes.

These are search coordinates over the existing state, not new primitives.
Every energy evaluation reconstructs the constituent geometry and applies the
unchanged finite-fibre minimum-energy longitudinal redressing.

Use centered differences with `h_g=2e-5`, `h_H=2e-4`; at most eight Newton
iterations; eigenvalue floor `1e-6`; and backtracking scales
`1,1/2,...,1/1024`. Accept a step only if it strictly lowers complete static
binding-plus-field energy. No momentum, damping, force, or field replacement
is added to the accepted dynamics.

## Frozen qualification

Require:

- final reduced gradient infinity norm `<=1e-9`;
- reduced Hessian minimum eigenvalue `>1e-6`;
- one complete rest tick with maximum impulse, total momentum, and complete
  state change each `<=1e-9`;
- eight forward ticks with maximum complete-state excursion `<=1e-8`, center
  displacement `<=1e-10`, energy drift `<=1e-10`, and common residual
  `<=1e-10`;
- eight state-only reverse ticks with recovery `<=1e-9`;
- integer-translation covariance of the refined accepted state and its first
  tick at shift `(3,0,0)` within `1e-9`.

Verdicts:

- `L33_SYMMETRY_REST_FIXED_POINT_CONSTRUCTIVE` if every gate passes;
- `L33_REST_REQUIRES_FULL_COORDINATE_REFINEMENT` if every evaluation and
  action step is valid but any stationarity gate fails;
- `L33_SYMMETRY_REST_REFINEMENT_EXECUTION_INVALID` if provenance,
  redressing, energy, derivative, action, or covariance evaluation is invalid.

The second verdict licenses only a fresh 48-coordinate rest solve. It does not
license tolerance relaxation or reuse of FTD-0706 as a valid moving-state
classification.

