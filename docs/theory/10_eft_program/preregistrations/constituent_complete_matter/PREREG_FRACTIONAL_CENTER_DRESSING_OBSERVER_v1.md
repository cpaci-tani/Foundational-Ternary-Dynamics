# FTD-0763 — Fractional-Center Dressing Observer

**Status:** `[PREREGISTERED CONSTRUCTIVE OBSERVER EXTENSION + OUTCOME-AWARE REPLAY]`

## Scope

FTD-0762 has already localized the old observer failure to its integer-center
domain. This protocol may establish a fractional-center observer and inspect
the untouched first failed FTD-0761 states. It may not claim mobile matter or
a co-moving dressing from observer validity alone.

## Frozen construction

- physical center: exact constituent centroid;
- support chart: componentwise nearest integer using the existing `llround`
  convention;
- support graph: the unchanged integer cube of half-width `R`;
- density, Poisson equation, zero-crossing boundary, field normalization,
  centered face/edge readout, and tolerances: unchanged;
- radial characteristic vector: periodic displacement from the physical
  center;
- shell membership and boundary ledger: integer support-chart coordinates;
- default behavior remains integer-center-only; fractional support is an
  explicit observer option;
- no production, action, current, force, momentum, or tick change.

## Algebraic qualification

On `L=17,33`, both polarities and face/edge/body relative orientations, use
fractional centroid offsets inside one chart and require:

- legacy mode rejects a genuinely fractional centroid;
- enabled preparation is neutral, contained, compact, and zero-crossing;
- Poisson residual `<=1e-13`, Gauss residual `<=1e-12`;
- CPU observer, boundary ledger, and the maximal non-wrapping ladder pass:
  `{3,4,5}` at `L=17`, `{4,6,8}` at `L=33`;
- CUDA values agree with CPU within `1e-12`;
- integer translations, proper cubic rotations, and conjugation agree within
  `1e-11`;
- physical center and support center are recorded separately;
- default integer-center corpus remains unchanged.

Record the one-sided representative difference across each `+/-0.5` chart
seam. It is a numerical fact, not a failure gate, because exact continuous
fractional covariance is not claimed.

## Untouched CUDA replay

After qualification, replay the exact FTD-0761 `L=321`, `q=0.015` plus arm on
face, edge, and body through tick 224. No field is recentered or rebuilt in the
dynamics. At ticks 160 and 224 record the fractional observer and support
ladder, complete scalar energy decomposition, boundary ledger, shell profiles,
centroid, support chart, common-action status, and existing spline momentum
defect.

## Verdicts

- `FRACTIONAL_CENTER_OBSERVER_CONSTRUCTED` requires every algebraic and
  CPU/CUDA parity gate and all three untouched tick-224 observer/ladder calls.
- `FRACTIONAL_CENTER_OBSERVER_CLOSED` applies if the selected Poisson/Gauss,
  covariance, or parity gates cannot coexist.
- No co-moving-dressing verdict is permitted. The replay metrics define the
  next time-relative morphology protocol.
