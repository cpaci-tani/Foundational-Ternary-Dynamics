# Mobility convergence criterion for refined lattice matter

**Claim:** FTD-0653  
**Status:** `[THEOREM — DIAGNOSTIC LOGIC]` + `[CORRECTION — PROSPECTIVE ONLY]`  
**Production impact:** none

## Statement

Let `mu_w(d,v)` be the finite-horizon mobility of a refinement family at width
`w`, direction `d`, and nonzero launch label `v`. The condition

\[
\min_d\mu_{w+1}(d,v)\ge \min_d\mu_w(d,v)
\]

is neither necessary nor sufficient for an isotropic mobile continuum limit.

The invariant physical requirement is instead the existence of a finite,
positive, direction-independent limit

\[
\mu_w(d,v)\longrightarrow \mu_0(v)>0
\]

over the declared linear-response window, together with vanishing directional
span and controlled translation-reaction defect. The stronger target
`mu_0=1` is valid only if the declared clock/ruler and launch momentum are
already normalized to the full dressed inertial response.

## Proof

Nondecrease is not necessary. The sequence

\[
\mu_w=1+{1\over w}
\]

decreases strictly while converging to the isotropic finite limit one.

Nondecrease is not sufficient. For `w>=3`,

\[
\mu_w={1\over2}-{1\over w}
\]

increases strictly but converges to `1/2`, not to a predeclared target one.
More generally, a direction-dependent family can be nondecreasing in every
direction while retaining distinct directional limits.

Therefore the sign of the raw width difference does not determine continuum
recovery. Convergence must be evaluated relative to a declared common target
or by a common-intercept model, with anisotropy tested independently. QED.

## Application boundary

FTD-0652 used the disproven raw-nondecrease condition in a locked conjunction.
Its `MIXED` verdict remains immutable. The theorem corrects only future
diagnostic design.

Post hoc, FTD-0652 is compatible with convergence near one because its
directional span and maximum absolute error from one shrink strongly. That is
motivation, not a new verdict. A fresh campaign must use unseen velocities or
horizons and lock both alternatives:

1. **normalized target:** errors `|mu-1|` shrink when full dressed
   normalization is assumed;
2. **renormalized target:** all directions approach one common fitted positive
   intercept even if it differs from one.

Neither alternative by itself establishes a pole; identifiable spectral
weight and finite-volume stability remain separate gates.
