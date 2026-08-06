# Audit — FTD-0707 L=33 symmetry-rest refinement

**Verdict:** `[AUDIT PASS — REDUCED REST CHART INSUFFICIENT]`

The certificate pins all four result files, runner, and preregistration; checks
83 valid evaluations, the zero reduced gradient, positive reduced curvature,
failed impulse/state stationarity, eight forward ticks, exact reverse, energy,
common-action, and covariance gates. It passes.

The zero reduced gradient does not imply full stationarity. Individual forces
of order `1e-5` cancel in net momentum and are invisible to the chosen four
coordinates. No fixed-point or moving-state claim follows from this run.

