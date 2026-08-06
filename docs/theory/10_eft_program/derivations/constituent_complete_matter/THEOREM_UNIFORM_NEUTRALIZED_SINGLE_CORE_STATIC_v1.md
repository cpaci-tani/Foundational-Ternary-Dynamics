# FTD-0611 — Uniform-neutralized single-core static state v1

**Status:** `[MEASURED — NINE-MODE POSITIVE BASIN]` +
`[CLOSED NEGATIVE — LOCKED PRECISION CONJUNCTION]` +
`[PROTOCOL PRECISION MISMATCH]`
**Protocol:**
[`PREREG_UNIFORM_NEUTRALIZED_SINGLE_CORE_STATIC_v1.md`](../preregistrations/PREREG_UNIFORM_NEUTRALIZED_SINGLE_CORE_STATIC_v1.md),
prefix SHA-256 `45FC3250CE24A236EBC231DAD9AA171CADFD754FA8289601892B73C107279B69`
**Production status:** unchanged

All 16 registered translation/orientation starts are admissible and terminate;
two reach the best energy `0.00155179550766847`. The full
translation/orientation/strain Hessian has nine positive eigenvalues,

```text
lambda_min = 0.00100185,
lambda_max = 12.00495,
```

and all 18 signed perturbations increase energy. Direct Poisson, Gauss, curl,
energy, and integer-translation covariance gates pass.

The locked conjunction nevertheless fails. Its finite-difference gradient is
`4.40e-8` against `1e-8`; the 16-tick rest orbit drifts approximately
`1.49e-8` transversely against `1e-8`. Every common-action, energy, geometry,
fibre, and inverse gate passes.

The registered search stopped at simplex diameter `1e-7`, then demanded
smaller downstream gradient/rest scales. The negative verdict therefore
closes that numerical protocol, not the positive basin. FTD-0612 performs the
only licensed repair: deterministic refinement of the same state and action.
