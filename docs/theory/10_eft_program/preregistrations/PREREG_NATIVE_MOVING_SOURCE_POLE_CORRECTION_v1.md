# FTD-0558 — Native Moving-Source Pole Correction v1

**Status:** [PRE-REGISTRATION — LOCKED/RUN; CORRECTION PASSED]
**Date locked:** 2026-07-26
**Scope:** analytic correction plus observer-only CPU verification of the frozen
production `FULL`-stencil `(flux,wave_vel)` map
**Production changes:** none

## 1. Question

FTD-0115 and FTD-0120 used a continuous-time seven-point wave operator and
unwrapped Fourier labels to claim that a uniformly moving source has a
Cherenkov pole at every nonzero speed.  This protocol tests that claim against
the actual production map and separates a smooth travelling source from an
integer-hop source history.

No source-force reciprocity, matter equation of motion, radiation power, or
phenomenological conclusion is tested here.

## 2. Frozen production operator

Disable every engine toggle except `wave_propagation`, select the periodic
boundary and production `FULL` stencil, use unit tick and
`C_WAVE = 1/sqrt(3)`.  For one Fourier mode and Cartesian polarization,

\[
 \binom{J_{t+1}}{W_{t+1}}
 =U(\mathbf k)\binom{J_t}{W_t}+\binom11 f_t,
 \qquad
 U=\begin{pmatrix}1-a&1\\-a&1\end{pmatrix},
 \qquad a=C_{\rm WAVE}^2M(\mathbf k).
\]

The additive `f_t` is an observer drive applied to the production kick-drift
recurrence.  It is not a new engine source primitive.

The exact free phase is

\[
 \theta(\mathbf k)=2\arcsin\!\left(\frac{C_{\rm WAVE}\sqrt{M(\mathbf k)}}2\right).
\]

## 3. Analytic statements locked before execution

### 3.1 Exact discrete-time driven pole

For `f_t=f z^t`, `z=exp(-i Omega)`, the steady response obeys

\[
 (zI-U)\psi=\binom11 f,
 \qquad
 \det(zI-U)=z^2-(2-a)z+1,
\]

and its flux component is

\[
 J=\frac{f}{a-4\sin^2(\Omega/2)}.
\]

The exact retarded pole therefore satisfies

\[
 4\sin^2(\Omega/2)=C_{\rm WAVE}^2M(\mathbf k),
 \qquad \Omega=\pm\theta(\mathbf k)\pmod{2\pi}.
\]

The retarded prescription displaces the pole and retains its imaginary
on-shell contribution.  A principal-value-only expression cannot by itself
represent radiation.

### 3.2 Wrapped Brillouin zone and positive speed floor

Crystal momentum is represented in `[-pi,pi)` on each axis.  The old `L=16`
label `n=15` is the wrapped mode `n=-1`, not a distinct momentum near
`15*2pi/16`.

For the auxiliary seven-point symbol,

\[
 \widehat k^2=4\sum_i\sin^2(k_i/2)
 \ge \frac4{\pi^2}|\mathbf k|^2.
\]

Its phase-speed ratio is therefore at least `2/pi`, not arbitrarily small.

For the production `FULL` symbol, retaining only its positive face terms gives

\[
 M(\mathbf k)\ge\frac23\sum_i(1-\cos k_i)
 \ge\frac4{3\pi^2}|\mathbf k|^2.
\]

Since `asin(x) >= x`, every nonzero production mode obeys

\[
 \frac{\theta(\mathbf k)}{|\mathbf k|}
 \ge v_*:=\frac{2C_{\rm WAVE}}{\pi\sqrt3}=\frac2{3\pi}.
\]

For any unit source direction `e`, `|k.e| <= |k|`, so a smooth source with
`|v| < v_*` has no one-frequency Cherenkov resonance.  The bound is universal,
not asserted sharp.

Increasing `L` at fixed lattice spacing increases volume and momentum
resolution; it does not move the Brillouin-zone cutoff to infinity.  A
continuum limit requires a separate `a -> 0` scaling.

### 3.3 Integer-hop source is Floquet, not uniform fractional motion

For one lattice hop `d in Z^3` every `T` ticks,

\[
 x_{nT+r}=n d,\qquad
 s_{t+T}(\mathbf k)=e^{-i\mathbf k\cdot d}s_t(\mathbf k).
\]

Writing the periodic factor as a `T`-term discrete Fourier series gives

\[
 \Omega_\ell=\frac{\mathbf k\cdot d+2\pi\ell}{T},
\]

\[
 c_\ell=\frac{1-e^{i\mathbf k\cdot d}}
 {T\left[1-e^{i(\mathbf k\cdot d+2\pi\ell)/T}\right]},
 \qquad \ell=0,\ldots,T-1,
\]

with the removable zero-over-zero limit taken continuously.  Abrupt hopping
therefore contains schedule-dependent harmonics.  A harmonic may intersect a
field pole even when the mean speed `|d|/T` lies below `v_*`; that is a
stutter/acceleration spectrum, not evidence that the smooth-source phase speed
vanishes.

## 4. Locked observer campaign

### 4.1 Algebraic response arms

Use exactly 12 nonresonant combinations: the four registered wrapped momentum
triples

- `(1,-2,3)*2pi/17`,
- `(-3,2,1)*2pi/19`,
- `(4,1,-2)*2pi/23`,
- `(-2,-3,1)*2pi/29`,

and `Omega in {0.17,0.41,0.73}`.  If a registered arm lies within `1e-6` of a
pole, the protocol is invalid rather than moved.  Require the direct complex
`2x2` solve, closed flux response, and recurrence residuals at or below
`1e-12`.

### 4.2 Wrapped-zone threshold arms

Enumerate all nonzero canonical wrapped modes for
`L in {16,32,64}` and directions `<100>`, `<110>`, `<111>`.  Report the minimum
of `theta/|k.e|` where the denominator is nonzero, for exactly 9 arms.  Require
every minimum to exceed `v_* - 1e-12`.

At `L=16`, explicitly compare array labels `n=15` and `n=-1`.  Their production
symbols and phases must agree within `1e-12`; the old unwrapped ratio and the
wrapped ratio must differ by a factor greater than 10.

### 4.3 Floquet arms

Use `d=(1,0,0)`, `T in {4,8,16,32}`, and
`k_x in {2pi/17, 4pi/17, 6pi/17}` for exactly 12 schedules.  Reconstruct all
`T` periodic samples from the analytic coefficients and require maximum
reconstruction and Parseval residuals at or below `1e-12`.  Require at least
one non-fundamental coefficient above `1e-6` in every arm.

### 4.4 Static retarded identity

For every nonzero response momentum arm, verify that the zero-frequency flux
resolvent is `1/(C_WAVE^2 M)`.  This is the Abel/static-resolvent version of the
time-summed retarded Green identity for the production discrete-time map; no
ordinary convergence of the undamped unit-modulus impulse response is claimed.

## 5. Verdicts

- `NATIVE_MOVING_SOURCE_POLE_CORRECTED`: all registered identities and
  cardinalities pass.  FTD-0115's any-speed Cherenkov claim and FTD-0120's
  derivative power conclusions are retracted.  A smooth production source has
  a positive one-frequency resonance floor; integer hopping has a separate
  Floquet spectrum.
- `NATIVE_MOVING_SOURCE_CORRECTION_FAILED`: any locked identity, bound, or
  cardinality fails.  No replacement moving-source result advances.

Passing does not establish radiated power, a mobile particle, source recoil,
charge conservation, Lorentz recovery, or a production force law.

## 6. Execution record

The protocol was locked before compilation and execution with SHA-256
`9B58BF017AC1648C5081267933E39C3F4494B97A0883FC81578E65773BAFBC37`.
The pinned MSVC 14.44 observer then passed all 12 driven arms, 9 wrapped-zone
threshold arms, and 12 Floquet schedules.  The maximum C++ identity residual
was `7.10542735760e-14`; the independent Python response residual was
`1.47200070370e-13`.  Verdict:
`NATIVE_MOVING_SOURCE_POLE_CORRECTED`.
