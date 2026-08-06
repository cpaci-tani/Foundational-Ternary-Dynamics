# FTD-0707 — L=33 symmetry-rest refinement v1

**Status:** `[SELECTED DYNAMICS — REDUCED REFINEMENT INSUFFICIENT]`  
**Verdict:** `L33_REST_REQUIRES_FULL_COORDINATE_REFINEMENT`  
**Production status:** unchanged

The locked four-coordinate refinement performed 83 valid complete-energy
evaluations. Its reduced gradient is numerically zero and its reduced Hessian
minimum eigenvalue is `65.7476`, so it accepts no step. The actual reciprocal
tick nevertheless produces maximum individual impulse `1.06261e-5` and
one-step state change `1.06261e-5`; net momentum is only `6.27e-17`.

Eight forward/reverse ticks remain algebraically valid and recover within
`1.00e-13`, but their maximum state excursion is `1.88e-5`. Thus the defect is
an internal force pattern orthogonal to the registered four symmetry
coordinates, not collective acceleration or numerical non-invertibility.

The correct statement is that reduced symmetry coordinates are not a complete
rest chart at `L=33`. This licenses the preregistered full 48-coordinate
impulse solve; it neither changes the ontology nor validates FTD-0706.

Record: protocol `0E1C61DD...27FB0`; JSON `E6AFF7C6...CE367`; runner
`2FAB51A8...30589`; proof `CB45D202...05748`.

