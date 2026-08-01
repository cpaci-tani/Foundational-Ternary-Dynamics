# FTD-0676 — Canonical pre-contact mode-decay discriminator v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE NEW DATA]`  
**Production status:** unchanged; observer-only selected connected action  
**Parent FTD-0674 JSON SHA256:**
`1848283E5AF91B076E7DD69CB24B4677159FED8594F2C78A5D8D858F441044CB`  
**Parent FTD-0674 tick CSV SHA256:**
`DEA0582DD2E135071524CBAB6F532A74FCCEE49D2F88E163966E6CC6DE4364E9`  
**Canonical-observer theorem:** `FTD-0675`

## 1. Question and exploratory basis

After the FTD-0675 mass-metric correction, the fresh FTD-0674 target-mode
energy declines from one to `~0.60157` through tick 80. A post-result ordinary
least-squares fit, used only to generate this preregistration, gives

```text
log(E_target/E_target(0)) = a - Gamma_E t,
Gamma_E = 0.006537123419844565,
R^2 = 0.999331992978897,
fit window = ticks 8..64.
```

This campaign asks whether that canonical pre-contact decay rate survives at a
fresh half amplitude. It does not assume that the decay is irreversible, that
the target coordinate is a physical particle, or that the fitted rate is an
infinite-volume pole width.

## 2. Frozen execution

- Use `L=97`, horizon `T=80`, the same recentered FTD-0638 control geometry,
  selected connected-block common action, default-off sparse-current storage,
  and FTD-0640 complete tangent basis used by FTD-0674.
- Run one control plus both signs of the mode-6 momentum excitation. Rescale
  each sign so the actual maximum constituent momentum is exactly `5e-7`
  within `1e-15`. Initial face and edge fields must be bitwise equal to the
  control.
- Each path has an independent state and nonlinear-solve cache. Parallel paths
  are allowed; arithmetic and tick order inside each path remain serial.
- At every tick evaluate the exact FTD-0673 decomposition with target modes
  `{6,7}` and the corrected FTD-0675 coordinates
  `q=v^T M delta x`, `P=v^T delta p`. Normalize by each sign's tick-zero target
  energy.
- Require parent fingerprints, valid normalization, a valid mass-orthonormal
  basis with modes 6/7 in one degeneracy group, unchanged sector/fibre
  signatures, common-action residual and total-energy drift `<=1e-10`,
  normalized decomposition residual `<=1e-8`, state-only forward/reverse
  recovery `<=1e-8`, and complete JSON/CSV schemas.
- The registered source radius is eight. For `L=97`, conservative periodic
  self-contact is tick 81, so all ticks `0..80` are classified as pre-contact.
  No claim is made that this periodic calculation is an open boundary.

## 3. Frozen fit and gates

For each sign, fit ticks `8..64` only:

```text
M0: y(t) = a,
M1: y(t) = a - Gamma_E t,
y(t) = log(E_target(t)/E_target(0)).
```

Ordinary unweighted least squares is used. For `n=57` samples and `k` fitted
parameters,

```text
BIC = n log(RSS/n) + k log(n).
```

No tick, weight, endpoint, or model may change after execution. Require:

1. every fitted target value is finite and strictly positive;
2. `Gamma_E>0` for both signs;
3. `BIC(M0)-BIC(M1)>=10` and `R^2(M1)>=0.995` for both signs;
4. target decline `1-E_target(64)/E_target(8)>=0.20` for both signs;
5. each fresh `Gamma_E` is within `5%` of the exploratory parent value
   `0.006537123419844565`;
6. the two fresh rates agree relatively within `1e-4` and their complete
   normalized target histories agree by RMS within `1e-5` over ticks `0..80`.

The parent rate is frozen as a validation target, not pooled into the new fit.

## 4. Locked verdict map

- Any fingerprint, initialization, mode-basis, exact observer, action, energy,
  sector, inverse, positivity, or schema failure:
  `CANONICAL_PRECONTACT_MODE_DECAY_EXECUTION_INVALID`.
- Exact execution passes, gates 2--4 pass, but gate 5 or 6 fails:
  `CANONICAL_PRECONTACT_TRANSFER_AMPLITUDE_OR_POLARITY_DEPENDENT`.
- Exact execution passes, the tick-8--64 decline is at least `0.20`, but the
  registered exponential gates fail:
  `CANONICAL_PRECONTACT_TRANSFER_NONEXPONENTIAL`.
- Exact execution passes and the registered decline is below `0.20`:
  `CANONICAL_PRECONTACT_TRANSFER_ABSENT`.
- Every gate passes:
  `CANONICAL_PRECONTACT_EXPONENTIAL_TRANSFER_CONSTRUCTIVE`.

These classes are exhaustive and may not be repaired by changing the fit
window, fitting an amplitude rather than an energy, deleting ticks, or adding
oscillatory terms after viewing the result.

## 5. Interpretation boundary and next gate

A constructive result means only that the prepared canonical matter coordinate
loses energy into the other exact reservoirs at an amplitude-stable
pre-contact exponential rate in the selected reversible finite system. It is
evidence that the bare matter doublet is an embedded, non-autonomous coordinate.

It is not evidence of fundamental dissipation, a quantum decay width, a
localized hybrid pole, an asymptotic particle, or production ontology. The
next required discriminator is an outgoing/reflection-controlled or increasing-
volume complete-system response showing whether a localized positive-residue
resonance survives after detached field energy can escape.

No result changes production defaults or licenses a dashboard scenario.
