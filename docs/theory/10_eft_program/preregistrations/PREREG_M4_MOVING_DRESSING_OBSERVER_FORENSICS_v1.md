# FTD-0762 — M4 Moving-Dressing Observer Forensics

**Status:** [PREREGISTERED POST-HOC FORENSIC]

## Scope

This protocol is outcome-aware: FTD-0761 already reported false observer and
support-ladder bits after its initial boosted checkpoint.  It makes no new
mobility claim.  It localizes that registered failure without changing the
production tick, action, boost, tolerances, or observer equations.

## Frozen replay

- backend: WSL2 Ubuntu-22.04 CUDA on the configured RTX GPU;
- volume: `L=321`;
- formation: the certified M3 parent through tick 160;
- boost: `+0.015` per constituent along each registered face, edge, and body
  direction;
- replay: 64 transactions to tick 224, the first failed FTD-0761 checkpoint;
- action and solve options: exactly the FTD-0761 values;
- observer support: 4; support ladder: 4, 6, 8;
- Poisson tolerance: `1e-13`; observer gate: `1e-12`;
- no minus arm because FTD-0761 already established boost-mirror agreement;
- no parameter scan, tolerance change, or post-hoc force.

## Recorded controls

For every direction record:

- evolved centroid and distance to the nearest integer center;
- exact 64-step execution/common-action status;
- CPU observer and ladder validity;
- CUDA observer and ladder validity plus telemetry error;
- fresh same-geometry finite-support preparation validity and its residuals;
- a rigid-recenter control preserving relative separation and momenta;
- recentered preparation, CUDA observer, and CUDA ladder validity;
- relative-geometry and momentum preservation residuals.

## Frozen verdict rule

`OBSERVER_INTEGER_CENTER_CHART_OBSTRUCTION` requires all three rays to satisfy:

1. the evolved center is fractional by more than `1e-12`;
2. the 64 CUDA transactions pass their existing common-action gates;
3. evolved CPU and CUDA observer/ladder records are invalid;
4. same-geometry finite-support preparation is invalid;
5. rigid recenter preserves relative geometry and both momenta within `1e-12`;
6. recentered preparation, CUDA observer, and CUDA ladder all pass;
7. the recentered boundary-energy ledger passes.

`PHYSICAL_DRESSING_MISMATCH_EXPOSED` requires a valid same-geometry preparation
and invalid evolved CUDA observation.  Any failure of both same-geometry and
recentered preparation is `INFRASTRUCTURE_UNRESOLVED`.  No other outcome is
promoted.

## Consequences

An observer-chart verdict retracts only the inference that FTD-0761's Boolean
observer failures demonstrate moving-state incoherence.  It does not establish
a co-moving field.  A physical-dressing verdict instead sends the program to a
joint matter/field preparation.  Production defaults remain frozen.

