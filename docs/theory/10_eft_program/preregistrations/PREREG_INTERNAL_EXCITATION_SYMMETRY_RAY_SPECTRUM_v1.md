# FTD-0698 — Internal-excitation symmetry-ray spectrum v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged

## 1. Question

Does a fresh connected-matter internal excitation populate native face/edge
field modes preferentially near the exact constant-frequency surface
`Omega(k)=phi_int`, after dividing out the measured deposited-current form
factor on the same registered symmetry rays?

This is the first direct spectral follow-up to FTD-0694. It does not infer
wavevector content from radial slopes.

## 2. Frozen history

- volume `L=113`, periodic;
- horizon 96 forward ticks followed by 96 state-only reverse ticks;
- conservative self-contact tick 97 under the existing radius-8 bound;
- the FTD-0638 dressed connected block at the integer center;
- the FTD-0640 mode-6/7 doublet, both signs;
- maximum initial constituent momentum `7.5e-8`, not the FTD-0694 amplitude;
- the FTD-0692 local residual and FTD-0694 ordered current index;
- exact common-action, sector, energy, Gauss, continuity, and inverse gates are
  unchanged from FTD-0694.

## 3. Frozen spectral observer

At every tick `0..96`, apply the FTD-0697 batched observer to the excited-minus-
control field and excited-minus-control deposited current on the canonical
rays:

- `<100>` direction `(1,0,0)`;
- `<110>` direction `(1,1,0)`;
- `<111>` direction `(1,1,1)`;

and every positive harmonic `n=1..56`.

Record complex transverse `E`, `B`, and current coefficients, their morphology
powers, and

\[
\Omega_n=2\arcsin\sqrt{\frac13\sum_a
\sin^2\frac{\pi n d_a}{L}}.
\]

Current is reconstructed from the accepted sparse segments in their stored
addition order and scaled by the accepted `polarity_scale`.

## 4. Locked response statistic

For each sign, ray, and harmonic define over ticks `1..96`

\[
W_F(n)=\sum_t P_{T,\mathrm{field}}(n,t),\qquad
W_K(n)=\sum_t P_{\mathrm{current}}(n,t).
\]

Eligible harmonics satisfy `W_K(n) >= 1e-6 max_m W_K(m)`. For eligible modes,

\[
R(n)=W_F(n)/W_K(n).
\]

The registered resonant harmonic is the eligible maximizer of `R`. Its
frequency is called near-resonant when its distance from `phi_int` is no larger
than the larger of the adjacent native frequency spacings at the closest
harmonic. Its contrast is `max R / median R` over eligible harmonics.

## 5. Execution gates

- 97 valid spectra for each sign;
- all 56 harmonics and three rays present in fixed order at every tick;
- field and current projection residuals at most `1e-12`;
- complete common-action gates and reverse recovery at most `1e-8`;
- sign-paired integrated field and current powers agree within relative
  `1e-4` per registered mode;
- every ray has at least eight eligible harmonics.

Failure makes the run execution-invalid and licenses no spectral inference.

## 6. Locked outcome classes

- `SYMMETRY_RAY_RESONANT_TRANSFER_CONSTRUCTIVE`: all six sign/ray arms are
  near-resonant, have contrast at least 5, and sign-paired peak harmonics agree
  within one bin.
- `SYMMETRY_RAY_SPECTRAL_TRANSFER_MIXED`: execution passes but the constructive
  conjunction does not.
- `INTERNAL_EXCITATION_SYMMETRY_RAY_SPECTRUM_EXECUTION_INVALID`: any execution
  gate fails.

The mixed outcome is not a negative result for all of the Brillouin zone. The
three rays may miss dominant off-ray support.

## 7. Claim boundary

Even a constructive result establishes only current-normalized enhancement on
three finite-volume symmetry rays under one selected classical common action.
It is not a pole, photon, quantum decay, full radiation pattern, exact spectral
energy decomposition, or Lorentz result.
