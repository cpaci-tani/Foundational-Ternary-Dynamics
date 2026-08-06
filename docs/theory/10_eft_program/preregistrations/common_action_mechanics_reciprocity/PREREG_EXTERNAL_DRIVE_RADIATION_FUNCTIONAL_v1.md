# FTD-0559 — External-Drive Radiation Functional v1

**Status:** [PRE-REGISTRATION — LOCKED/RUN; POSITIVE EXTERNAL-DRIVE THEOREM]
**Date locked:** 2026-07-26
**Scope:** analytic work/energy theorem plus observer-only CPU verification of
the corrected FTD-0558 production modal map
**Production changes:** none

## 1. Question

FTD-0558 derived the exact discrete-time field pole but deliberately left
radiation power open.  This protocol asks the narrower question that can be
answered without inventing mobile matter: how much exact production field
energy is deposited by a *prescribed external modal drive*, and what
large-time radiation functional follows?

This protocol does not identify the drive with conserved charge and does not
derive matter recoil.

## 2. Frozen modal transaction

For each nonzero production `FULL`-stencil mode, set

\[
 a=C_{\rm WAVE}^2M(\mathbf k),\qquad
 \theta=2\arcsin(\sqrt a/2),\qquad 0<a<4.
\]

The forced kick-drift step is

\[
 W_{t+1}=W_t-aJ_t+f_t,
 \qquad
 J_{t+1}=J_t+W_{t+1}.
\]

The production modal invariant is

\[
 H_a(J,W)=|W|^2+a|J|^2-a\operatorname{Re}(J^*W).
\]

No damping, Gauss projection, material force, reaction, or reciprocal source
term is added.

## 3. Analytic statements locked before execution

### 3.1 Exact source-work identity

Let `(J°,W°)` be the unforced proposal and `(J',W')=(J°+f,W°+f)` the forced
endpoint.  With midpoint values between those two endpoints,

\[
 \Delta H_a=
 \operatorname{Re}\!\left[
 f^*\{a\bar J+(2-a)\bar W\}\right].
\]

This is an exact quadratic discrete-gradient identity.  The cumulative source
work equals final minus initial field energy at every finite time.

### 3.2 Exact finite-time retarded response

Eliminating `W` gives

\[
 J_{t+1}-(2-a)J_t+J_{t-1}=f_t.
\]

For zero initial data, its retarded impulse kernel is

\[
 G_m=\frac{\sin(m\theta)}{\sin\theta},\qquad m\ge1.
\]

For `f_t=F exp(-i Omega t)` applied at `t=0,...,N-1`, define

\[
 S_N(x)=\sum_{r=1}^N e^{ixr},
\]

\[
 R_N=\frac{S_N(\Omega+\theta)-S_N(\Omega-\theta)}
 {2i\sin\theta}.
\]

Then

\[
 J_N=F e^{-i\Omega N}R_N,
\]

\[
 W_N=F e^{-i\Omega N}
 \left(R_N-e^{i\Omega}R_{N-1}\right).
\]

### 3.3 Finite-volume resonance dichotomy

If `Omega` is separated modulo `2pi` from both `+theta` and `-theta`, both
geometric sums are bounded, hence `H_N` is bounded uniformly in `N`.

At `Omega=theta`,

\[
 R_N=\frac{S_N(2\theta)-N}{2i\sin\theta}
\]

and

\[
 \lim_{N\to\infty}\frac{H_N}{N^2}=\frac{|F|^2}{2}.
\]

Thus a lossless finite lattice does not generically produce a stationary
constant power: off resonance its stored energy is bounded, while exact
resonance produces coherent quadratic growth.

### 3.4 Continuum/large-time radiation functional

The Fejer kernel satisfies

\[
 \frac1N|S_N(x)|^2\ \Longrightarrow\ 2\pi\delta_{2\pi}(x).
\]

Away from the zero/Nyquist degeneracies, the distributional field-energy rate
for a drive with spatial amplitude `F(k)` and frequency `Omega(k)` is

\[
 P_{\rm ext}=\pi\int_{BZ}\frac{d^3k}{(2\pi)^3}|F(\mathbf k)|^2
 \sum_{\sigma=\pm1}
 \delta_{2\pi}(\Omega(\mathbf k)-\sigma\theta(\mathbf k)).
\]

For a smooth translating drive `Omega=k.v`, the coarea denominator on each
resonance surface is

\[
 |\mathbf v-\sigma\mathbf v_g(\mathbf k)|,
 \qquad \mathbf v_g=\nabla_{\mathbf k}\theta.
\]

This group-velocity-mismatch Jacobian replaces the unspecified Jacobian in the
retracted FTD-0120 power expression.

For one integer hop `d` every `T` ticks, insert the FTD-0558 Floquet components

\[
 \Omega_l=(\mathbf k\cdot d+2\pi l)/T,
 \qquad |F_l|^2=|F|^2|c_l|^2.
\]

The corresponding coarea denominator is
`|d/T-sigma v_g|`.  This remains an external-drive functional, not particle
energy loss.

## 4. Locked observer campaign

### 4.1 Work arms

Use the four momenta registered by FTD-0558 and the following three complex
triples `(J,W,f)`:

1. `(0.31-0.17i, -0.22+0.41i, 0.07+0.03i)`;
2. `(-0.19+0.28i, 0.36-0.11i, -0.04+0.09i)`;
3. `(0.08+0.13i, -0.27-0.32i, 0.05-0.06i)`.

Require exactly 12 one-step arms and maximum direct-energy/work residual at or
below `1e-12`.

### 4.2 Harmonic response arms

Use momenta `(pi,0,0)`, `(pi,pi,0)`, and `(pi,pi,pi)`, unit complex drive,
frequencies `Omega=theta` and `Omega=theta+0.3`, and
`N in {16,32,64,128}`.  Require exactly 24 direct/closed response arms, with
state and cumulative-work residuals at or below `1e-10`.

For resonant arms, verify

\[
 |H_N/N^2-1/2|\le
 2\lambda_{max}|v|B/N+\lambda_{max}B^2/N^2,
\]

using the analytic conservative choices

\[
 \lambda_{max}=a+1,
\]

\[
 |v|^2=1/(4\sin^2\theta)+1/(4\cos^2(\theta/2)),
\]

\[
 B^2=1/(4\sin^4\theta)
 +[1/\sin^2\theta+1/(2|\sin\theta|)]^2.
\]

For off-resonant arms, verify every `t<=128` against

\[
 H_t\le |F|^2 A^2(4+3a),
\]

\[
 A=\frac{1}{2|\sin\theta|}
 \left(
 \frac1{|\sin((\Omega+\theta)/2)|}
 +\frac1{|\sin((\Omega-\theta)/2)|}
 \right).
\]

### 4.3 Fejer normalization

For `N in {16,32,64,128}`, evaluate the periodic rectangle rule with 4096
uniform points over `[-pi,pi)`.  Because the integrand is a trigonometric
polynomial of degree below the grid Nyquist value, require its normalized
integral to equal one within `1e-12`.  Require exactly four Fejer arms.

## 5. Verdicts

- `EXTERNAL_DRIVE_RADIATION_FUNCTIONAL_DERIVED`: all exact identities,
  cardinalities, analytic bounds, and Fejer normalizations pass.  This advances
  a field-energy theorem and selected external-drive functional only.
- `EXTERNAL_DRIVE_RADIATION_FUNCTIONAL_FAILED`: any locked gate fails.  No
  radiation statement advances.

Passing does not promote FTD-0120 power, establish a radiating particle,
derive charge conservation, or license a force/toggle/scenario.  Physical
matter energy loss remains gated on a common reciprocal matter-current-field
transaction.

## 6. Execution record

The protocol was locked before compilation and execution with SHA-256
`26D23CDEC4BD5B723C7834AB95FE98A6219068C7689CF130D9B105105F2D054F`.
All 12 work arms, 24 harmonic-response arms, and 4 Fejer arms passed under the
pinned MSVC 14.44 CPU observer.  Maximum work, response, cumulative-ledger,
and Fejer residuals were respectively `7.29e-17`, `5.12e-13`, `1.82e-12`, and
`1.55e-15`.  Verdict: `EXTERNAL_DRIVE_RADIATION_FUNCTIONAL_DERIVED`.
