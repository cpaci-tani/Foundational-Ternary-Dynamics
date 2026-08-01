# FTD-0660 — Internal-mode action-transfer ledger v1

**Status:** `[SELECTED DYNAMICS — MIXED]`  
**Verdict:** `INTERNAL_MODE_ACTION_TRANSFER_MIXED`  
**Production impact:** none

FTD-0660 decomposes the actual field at every complete state into the
instantaneous minimum-energy dressing of the same constituent geometry, a
dynamic residual field, and their exact quadratic interference term. All 18
histories execute, conserve complete energy, and invert.

The physical transfer and morphology gates pass in every nonzero arm:

- the first internal doublet falls to `0.424255..0.424256` of its initial
  harmonic energy by tick 128;
- the positive dynamic-residual norm reaches at least `0.836963` of initial
  excitation;
- its far-shell fraction reaches `0.85270..0.85280`;
- registered shell onsets are ordered near, middle, far at approximately
  ticks `5`, `10--11`, and `35`;
- amplitude and sign residuals are `0.001480` and `0.001323`;
- complete energy drift is `2.665e-15`, field-decomposition residual
  `7.567e-16`, and inverse recovery `1.835e-11`.

The conjunction remains mixed because the `1e-20` zero-observer tolerance is
below the independent Poisson redressing floor (`2.776e-17`) and the cyclic
control compares arbitrary coordinates inside a degenerate eigenspace
(`0.13525`). The favorable transfer observations are preserved but do not
upgrade v1.
