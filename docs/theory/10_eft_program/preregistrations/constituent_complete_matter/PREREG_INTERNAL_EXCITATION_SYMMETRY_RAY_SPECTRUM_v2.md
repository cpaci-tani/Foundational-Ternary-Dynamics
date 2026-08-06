# FTD-0699 — Internal-excitation symmetry-ray spectrum v2

**Status:** `[PRE-REGISTRATION — CORRECTED FRESH RUN]`  
**Production status:** unchanged

## 1. Parent defect

FTD-0698 is execution-invalid. Its dynamics, spectra, and inverse completed,
but its classifier received the continuous Hessian frequency
`omega_int=1.2140869502262857` rather than the discrete tick-map phase
`phi_int=2 atan(omega_int/2)=1.0911648733663635`. It therefore evaluated the
wrong resonant surface. Its all-harmonic sign-power gate also compared modes
below the registered current-support relevance floor; the worst residual was
`4.67884e-4` on field power `~2.31e-29`.

No FTD-0698 raw peak is promoted or used to change a physical threshold.

## 2. Frozen corrections

Retain the FTD-0698 observer, harmonics, rays, response statistic, contrast 5,
one-bin resonance window, current eligibility floor `1e-6`, exact gates, and
outcome classes, with only these corrections:

1. pass the exact discrete phase
   `0.5*(mode6.phase+mode7.phase)` to the classifier;
2. apply the unchanged relative sign tolerance `1e-4` only where both signs
   satisfy their ray's registered current eligibility floor;
3. use fresh maximum initial momentum `5.0e-8`.

The sign-domain correction aligns the execution gate with the response
statistic's already registered domain. It does not relax the numerical
tolerance.

## 3. Acceptance and outcomes

All FTD-0698 requirements otherwise remain unchanged. In particular,
`SYMMETRY_RAY_RESONANT_TRANSFER_CONSTRUCTIVE` still requires all six sign/ray
arms to be within the locked one-bin phase window with response contrast at
least 5 and sign-paired peaks within one harmonic.

Any other fully valid execution is
`SYMMETRY_RAY_SPECTRAL_TRANSFER_MIXED`; any algebraic, coverage, sign-domain,
common-action, or inverse failure is execution-invalid.

## 4. Claim boundary

A constructive result remains restricted to current-normalized morphology on
three finite-volume symmetry rays under the selected common action. It is not
a complete Brillouin-zone energy spectrum, pole, photon, quantum decay, or
Lorentz result.
