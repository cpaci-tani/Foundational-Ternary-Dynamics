# FTD-0664 — Volume-scaled internal-mode transfer v1

**Status:** `[EXECUTION INVALID — NO PHYSICAL VERDICT]`  
**Verdict:** `VOLUME_SCALED_INTERNAL_TRANSFER_EXECUTION_INVALID`  
**Production impact:** none

## Result

All six excited/control histories completed over `L={17,25,33}` and retained
exact common-action, energy, decomposition, sector, and redressing behavior.
The locked execution conjunction nevertheless fails: state-only recoveries are
`1.710e-10..6.243e-10`, above the preregistered `1e-10` bound.

The observer also exposes two protocol defects. The inferred tick-zero
doublet ratio is `3.829642`, not one, because v1 does not measure the actual
mass-weighted paired projection. Its tick-16 doublet-loss gate assumes monotone
one-way transfer, while the raw histories show fast internal exchange.

Raw pre-return values are retained but cannot grade physics. At tick 16 the
dynamic-field energy is positive, its radius second moment grows from about
`4.33` to `17.11`, and the values are nearly volume independent. FTD-0665 is
the prospective correction; this document does not upgrade v1.
