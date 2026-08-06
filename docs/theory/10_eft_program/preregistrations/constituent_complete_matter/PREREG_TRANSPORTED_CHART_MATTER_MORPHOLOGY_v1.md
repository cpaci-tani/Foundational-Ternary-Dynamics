# FTD-0764 — Transported-chart matter morphology v1

**Status:** `[PREREGISTERED OBSERVER QUALIFICATION + OUTCOME-AWARE CUDA REPLAY]`

## Scope

FTD-0764 tests whether the actual FTD-0761/0763 field has a stable morphology
in coordinates transported with its moving core. It does not alter dynamics,
construct a boost, or promote the selected Gauss representative into an ontic
material component.

## Frozen observer

- physical center, support chart, bound preparation, midpoint magnetic readout,
  and tolerances are exactly FTD-0763;
- staggered component locations are those in
  `DERIV_TRANSPORTED_CHART_MATTER_MORPHOLOGY_v1.md`;
- channels are actual, selected-bound, residual, interference, and near
  residual, with modewise reconstruction required;
- near support is Chebyshev radius `8`; the outer morphology annulus ends at
  radius `48` in the registered replay; qualification uses near/outer radii
  `4/6` at `L=17` and `6/12` at `L=33`;
- ray mode bases are:
  - face: longitudinal `(0,0,1)`, transverse `(1,0,0),(0,1,0)`;
  - edge: longitudinal `(0,1,-1)`, transverse `(1,0,0),(0,1,1)`;
  - body: longitudinal `(1,1,1)`, transverse `(1,-1,0),(1,1,-2)`;
- harmonics are `{1,2,4,8,16,32}` with no post-run additions;
- transported coefficients use the measured center and no fitted phase,
  velocity, offset, gain, or window;
- field momentum candidates and normalizations are unchanged from FTD-0473
  and FTD-0619.

## Qualification

At `L=17,33`, both polarities and face/edge/body geometries must satisfy:

- use the maximal non-aliased harmonic subsets `{1,2,4}` at `L=17` and
  `{1,2,4,8}` at `L=33`; the registered `L=321` replay uses the complete
  frozen set;

- CPU/CUDA coefficient parity within `1e-11`;
- modewise actual = bound + residual + interference within `1e-12` relative;
- zero-mode energy agrees with direct energy within `1e-12`;
- exact integer-translation transported coefficients within `1e-11`;
- proper cubic rotation and polarity conjugation within `1e-11`;
- repeated-state morphology distance below `1e-13`;
- legacy FTD-0763 integer and fractional observer regressions pass;
- no complete CUDA field download.

No registered large-volume artifact may be written unless every qualification
gate passes.

## Registered replay

Use WSL2 CUDA and the exact FTD-0761/0763 `L=321`, `q=0.015` plus histories on
face, edge, and body. Record ticks `160,176,192,208,224`. For each checkpoint
record:

- complete fractional observer and `{4,6,8}` support ladder;
- all frozen transported coefficients and channel energies;
- near/outer residual first moments and radii;
- shell characteristic energies and signed radial fluxes;
- matter momentum, local field pseudomomentum, spline-Poynting momentum, and
  both cumulative defects;
- common-action, exact-energy, causality, regularity, and inversion gates.

Run a no-boost rest control through tick 224 on each ray. Rest is a numerical
control, not a denominator used to loosen any gate.

## Outcome map

The run first reports continuous metrics. The following classifications are
locked:

- `TRANSPORTED_NEAR_FIELD_COHERENT`: near-residual morphology distance from
  tick 160 to every later checkpoint is `<=0.10`, its energy stays within
  `[0.8,1.2]` of tick 160, and the bound construction control is `<=0.02`;
- `BOUND_CONTROL_ONLY`: the bound control is `<=0.02`, but the near-residual
  conjunction fails;
- `NO_TRANSPORTED_FIELD_COHERENCE`: even the bound construction control exceeds
  `0.02` after qualification;
- `MORPHOLOGY_EXECUTION_INVALID`: any algebraic, CUDA, common-action, observer,
  ladder, energy, causality, regularity, or inverse gate fails.

Independently label:

- `DETACHED_OUTGOING_COMPONENT` only if outer residual energy grows over three
  consecutive moved intervals (`E176<E192<E208<E224`) and the radius-48 signed
  outward characteristic flux is positive at ticks `192,208,224`;
- `TRAILING_WAKE_CANDIDATE` only if the near/outer residual longitudinal first
  moment is negative at all four moved checkpoints and its magnitude increases
  on at least three of the successive moved intervals;
- `MOMENTUM_CANDIDATE_CLOSES` only if either pre-existing field candidate gives
  cumulative matter-plus-field defect `<=1e-9` on every moved checkpoint;
  otherwise report the two defects without inventing a substrate account.

These morphology thresholds are selected finite-resolution research criteria,
not derivations of particlehood.

## Frozen consequences

- A coherent result licenses “co-moving actual near field” only for this
  selected family and horizon.
- `BOUND_CONTROL_ONLY` leaves the moving core constructive but complete matter
  dressing open.
- Failure of both momentum candidates repeats the FTD-0619 preferred-lattice
  defect and moves the live question to width scaling or a new dynamical
  substrate/connection primitive.
- No production default, ontology primitive, toggle, scenario, constant, mass
  formula, or Lorentz claim changes under any outcome.
