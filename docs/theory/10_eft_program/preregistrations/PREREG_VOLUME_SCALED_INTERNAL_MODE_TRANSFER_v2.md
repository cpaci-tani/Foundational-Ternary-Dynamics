# FTD-0665 — Volume-scaled internal-mode transfer v2

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Production status:** unchanged; fresh observer-correction run  
**Parent v1 protocol:**
`B6C7E2632884FA6CC98499D42EE6E4CE1AE790C9B6261E034278ABABB2FFB933`

## 1. V1 disposition

FTD-0664 is execution-invalid and remains so. Its six state-only recoveries are
`1.71e-10..6.24e-10`, above the locked `1e-10` gate. Its initial doublet ratio
is also `3.829642...`, not one, because v1 inferred the initial modal energy
from a Euclidean eigenvector normalization instead of measuring the actual
mass-weighted paired projection. Finally, v1's requirement that the doublet
already lose energy by tick 16 assumes monotone one-way transfer, while its
ungraded histories show rapid matter/field exchange and a positive outward
dynamic residual before later doublet depletion.

These are observer/protocol defects. No v1 verdict or threshold is changed.

## 2. Frozen corrections

Retain the v1 volumes, recentered geometry, fixed localized doublet vector,
two momentum signs, same-volume control, `4L` forward/reverse horizons,
instantaneous redressing, tick-16 causal window, common action, and all field
and sector observers.

Change only:

1. define each arm's initial doublet energy from its actual excited-minus-
   control modal projection at tick zero, so the recorded ratio is exactly one;
2. apply the same measured denominator to dynamic-field energy and norm;
3. replace the invalid monotone-doublet-loss gate with the direct field gate:
   dynamic residual energy and positive norm must both be positive at tick 16;
4. retain radial spreading `R2(16)-R2(4)>=4`;
5. set reverse recovery to `<=1e-8`. This is an execution-precision bound for
   up to `2(4L)=264` implicit steps, each independently required to have common
   residual `<=1e-10`; it is not a physical tolerance.

The pre-return locality RMS still compares doublet ratio and normalized
dynamic-field energy over ticks `0..16` and must be `<=0.05`.

The return classifier is recomputed with the corrected ratio: after first
falling below `0.60`, recovery above `0.80` at `t>=L` defines a return. The
`SCALED_RETURN`, `NO_RETURN_IN_WINDOW`, and `MIXED_RETURN` rules are unchanged
and remain descriptive rather than verdict gates.

## 3. Verdicts and scope

- failed execution: `VOLUME_SCALED_INTERNAL_TRANSFER_V2_EXECUTION_INVALID`;
- all locality, positive-field, and radial-spreading gates pass:
  `VOLUME_SCALED_PRE_RETURN_TRANSFER_V2_CONSTRUCTIVE`;
- otherwise: `VOLUME_SCALED_INTERNAL_TRANSFER_V2_MIXED`.

A constructive result proves only that this prepared internal deformation
generates a volume-stable outward dynamic field residual before causal
wraparound. It does not prove monotone core decay, an exponential law,
asymptotic radiation, a resonance pole, a photon, or a lifetime.
