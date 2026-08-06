# FTD-0666 — Internal-mode return-time discriminator v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Production status:** unchanged; observer-only out-of-sample horizon extension  
**Parent FTD-0665 JSON:**
`3D9C7F4601C4932458F351A1DE412A6E6E849E2514691C2C21093944BEE9B5B2`

## 1. Prediction

FTD-0665's locked `4L` horizons detect the corrected doublet-return threshold
at tick 76 for both signs at `L=25` and `L=33`. The `L=17` horizon ends at tick
68 and therefore cannot test the same event. Equality at the two larger,
different circumferences predicts that the first threshold return is governed
primarily by an intrinsic composite beat time rather than scaling in direct
proportion to the periodic circumference.

## 2. Frozen protocol

- Run only the previously truncated `L=17` arm to tick 100.
- Use the same recentered FTD-0638 geometry, FTD-0640 first-doublet vector,
  `8e-6` momentum amplitude, both signs, same-volume unexcited control,
  instantaneous dressing observer, and selected common action as FTD-0665.
- Use FTD-0665's actual tick-zero paired modal normalization.
- Invert all three histories for 100 ticks.
- Retain common residual `<=1e-10`, decomposition `<=1e-10`, complete-energy
  drift `<=1e-10`, recovery `<=1e-8`, and sector/fibre preservation.
- Define the return exactly as in FTD-0665: after the corrected doublet ratio
  first falls below `0.60`, it later exceeds `0.80`.

The locked prediction passes only if both signs return in ticks `74..78`, their
return ticks differ by at most one, and the full execution gates pass. No
parameter or threshold is fitted to the new `L=17,t>68` data.

## 3. Verdicts and scope

- failed execution: `INTERNAL_MODE_RETURN_TIME_EXECUTION_INVALID`;
- prediction passes: `INTERNAL_MODE_ABSOLUTE_RETURN_TIME_CONSTRUCTIVE`;
- execution passes but prediction fails: `INTERNAL_MODE_RETURN_TIME_MIXED`.

A constructive result rejects simple first-return proportionality to box size
over `L={17,25,33}` and identifies a reproducible approximately 76-tick
internal recurrence threshold. It does not prove an infinite-volume bound
state, exclude all boundary influence, establish a resonance width, or imply
quantum recurrence.
