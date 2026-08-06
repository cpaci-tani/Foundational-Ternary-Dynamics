# PRE-REGISTRATION — Native dynamical polarity response v1

**Date locked:** 2026-07-23  
**Identifier reservation:** `FTD-0429`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION RUN]`  
**Campaign:** `engine/tests/campaign_native_dynamic_polarity_response.cpp`  
**Observer:** `engine/include/ftd/eft/native_dynamic_polarity_response.h`  
**Source lock:** `scripts/proofs/native_dynamic_polarity_response_lock.json`  

## 1. Question under test

The exact additive-nullspace result in FTD-0421 addresses microscopic event
invariants. It does not decide whether the frozen native dynamics produces a
long-wavelength, charge-like response from primitive ternary polarity. This
campaign therefore asks a different question:

> Starting with zero flux and a frozen neutral polarity pattern, does the
> production wave/coupling tick dynamically generate a longitudinal flux
> response whose divergence approaches a nonzero constant multiple of the
> polarity density as `|k| -> 0`?

The operational effective charge is the infrared closed-flux source. It is not
assumed to equal the microscopic sign at unit normalization.

## 2. Frozen engine sector

The production tick, constants, 18-point Moore Laplacian, central state
gradient, and kick-drift update are unchanged. Every toggle is disabled except

- `wave_propagation=true`;
- `coupling=true`;
- `dual_substrate=false`.

In particular, `gauss_projection`, `matched_gauss_dynamics`, damping,
movement, forces, genesis, evaporation, pair production, annihilation, and
weak transmutation remain off. The initial `flux` and `wave_vel` fields are
zero. No field is solved or projected before or during the run.

The ternary source is held fixed only because movement and reactions are off;
the observer does not lock or rewrite it during evolution. This is the exact
reaction-free linear sector of the production engine, not a new update rule.

## 3. Source family

Use periodic `L in {32,64}` and directions

`d in {(1,0,0),(1,1,0),(1,1,1)}`.

For base harmonic `b in {1,2}`, define

`m(r) = (d_x x + d_y y + d_z z) mod L`

and the neutral square-wave polarity

`s_b(r)=sigma` when `(b*m(r)) mod L < L/2`, and `-sigma` otherwise,
with `sigma in {+1,-1}`. `L` is divisible by four, so every active transverse
lane is exactly neutral. Base `b=1` supplies the measured harmonics `n=1,3`;
base `b=2` supplies `n=2`.

Amplitude is changed without changing any coupling constant: only transverse
lanes satisfying `(y + L*z) mod duty = 0` are populated, for
`duty in {1,2,4}`. The primary matrix uses `duty=2` for all directions,
harmonics, and both signs. Amplitude controls use `duty=1,4` for direction
`(1,0,0)` and both base waves. Every realized source mode must satisfy
`|S_k| >= 10^-3`; failure is an invalid fixture, not a physical result.

## 4. Fourier observables

For `k=(2*pi*n/L)d`, use the volume-normalized convention

`S_k = N^-1 sum_r s(r) exp(-i k.r)`,

`J_k = N^-1 sum_r J(r) exp(-i k.r)`.

The central-difference divergence mode is evaluated without a second stencil
pass:

`D_k = i [sin(k_x) J_x,k + sin(k_y) J_y,k + sin(k_z) J_z,k]`.

The complex response is `R_k(t)=D_k(t)/S_k`. Samples are taken at `t=0` and
at sixteen preregistered, approximately uniform phases spanning two periods
of the slowest mode in each arm. The exact discrete pole used in the fit is

`omega(k)=acos(1-C_WAVE^2 M(k)/2)`,

where

`M(k)=4-(2/3)(cos kx+cos ky+cos kz)
          -(2/3)(cos kx cos ky+cos kx cos kz+cos ky cos kz)`.

Fit the real and imaginary parts independently to

`R_k(t)=Z_k+B_k cos(omega t)+C_k sin(omega t)`.

`Z_k` is the dynamically generated static susceptibility; it is not read from
a Gauss projector. The exact frozen-sector prediction, used as an internal
operator check rather than a fitted physical target, is

`Z_exact(k)=(G_C/C_WAVE^2) [sum_a sin^2(k_a)]/M(k)`.

## 5. Locked gates

The run is valid only if all of the following hold:

- the source is globally neutral and unchanged bit-for-bit after evolution;
- `gauss_projection=false` and `matched_gauss_dynamics=false` throughout;
- `|S_k|>=10^-3`;
- normalized complex fit residual `<=10^-8`;
- `|Im Z_k|<=10^-8 max(1,|Re Z_k|)`;
- `|Re Z_k-Z_exact(k)|/|Z_exact(k)|<=10^-7`;
- polarity-mirror susceptibilities agree within relative `10^-7`;
- the three duty factors in the `(1,0,0)` controls agree within relative
  `10^-7` after division by their measured `S_k`;
- Windows MSVC CPU and WSL2 GCC/CUDA scalar results agree within relative
  `10^-6` wherever their matrices overlap.

For the infrared decision, fit all primary `Z_k` values to

`M_const: Z=Z0+A q2+B h4+C q2^2`,

where `q2=sum_a k_a^2` and `h4=(sum_a k_a^4)/q2`, and compare it with

`M_zero: Z=A q2+B h4+C q2^2`.

Use ordinary least squares with a fixed residual floor of `10^-12` before
computing `BIC = n ln(RSS/n)+p ln n`. The infrared gate requires

- `Delta BIC = BIC_zero-BIC_const >= 10`;
- `Z0>0` with `|Z0-3 G_C|/(3 G_C)<=0.01`;
- the constant-model RMS residual `<=10^-4`.

The `3 G_C` comparison is the independently derived continuum limit
`G_C/C_WAVE^2`, because `C_WAVE^2=1/3`; it is not an empirical charge target.

## 6. Locked outcomes

| outcome | interpretation |
|---|---|
| A: all validity and infrared gates pass | `[DERIVED + MEASURED — RESTRICTED NATIVE LINEAR SECTOR]`: primitive polarity dynamically sources an infrared, Gauss-like effective charge with finite response normalization. Combined with the already established reaction-free production transport identity, this supports coarse-scale emergent charge in that sector. |
| B: time fits pass but `Z0` is zero or the zero-intercept model is not rejected | `[CLOSED NEGATIVE — NATIVE IR CHARGE RESPONSE]`: the field response is derivative-suppressed and polarity does not become a long-range effective charge. |
| C: nonzero response exists but mirror, amplitude, direction, or continuum gates fail | `[NONUNIVERSAL RESPONSE]`: polarity drives flux, but no single coarse-scale charge normalization is licensed. |
| D: source mutation, forbidden toggle, backend disagreement, or observer failure | `[INVALID CAMPAIGN]`: repair instrumentation only and rerun under a new versioned preregistration. |

## 7. Explicit scope

A passing result does not prove a microscopic `U(1)` generator, gauge
redundancy, exact conservation through reactions, photon quantization, the
empirical value of electric charge, or a force law. It establishes a dynamic
infrared source relation in the frozen reaction-free linear engine sector.
The separate questions of moving-source retardation, pole residues, reaction
leakage, and Ward identities remain open.
