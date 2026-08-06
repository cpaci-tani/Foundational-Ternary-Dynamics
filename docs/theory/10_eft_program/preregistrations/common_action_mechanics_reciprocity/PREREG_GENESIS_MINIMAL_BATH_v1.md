# Pre-registration — Minimal symplectic bath for accepted genesis (FTD-0572)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`
**Date:** 2026-07-26
**Parent results:** FTD-0569, FTD-0570, FTD-0571.
**Production changes permitted:** none.

## 1. Question

FTD-0571 proves that any symplectic completion of the noncanonical accepted
genesis map needs bath-to-system feedback. This campaign asks four narrower
questions:

1. What is the minimum canonical bath dimension forced by the symplectic
   defect?
2. Is that lower bound constructively attainable on a prepared bath slice?
3. Can the prepared bath repeat the projected production map without reset?
4. Can the completion be passive with respect to the equal-weight positive
   quadratic energy of system plus bath?

## 2. Frozen system derivative

In the radial/tangential canonical basis `x=(J,W)=(q,p)`, freeze

\[
M=\operatorname{diag}(\Lambda,aI_3),\qquad
\Lambda=\operatorname{diag}(1,t,t),
\]

with

\[
t=\frac{x}{x+k_g}\in(0,1),\qquad a=1-d\in[0,1].
\]

The registered arm grid is:

- ten directions: the six axes and four body-diagonal representatives;
- excesses `x/k_g in {0.125, 0.5, 1.25}`;
- drains `d in {0, 0.5, 0.9, 1}`.

This gives 120 matrix arms and 360 canonical-pair arms. Thirty matrix arms
must have defect rank four and ninety must have rank six.

## 3. Rank theorem to prove

For an enlarged derivative

\[
S=\begin{pmatrix}M&B\\ C&D\end{pmatrix}
\]

on system plus a `2m`-dimensional canonical bath, derive from both equivalent
symplectic identities

\[
S^{\mathsf T}\Omega S=\Omega,
\qquad
S\Omega^{-1}S^{\mathsf T}=\Omega^{-1}
\]

that

\[
\Delta=\Omega_x-M^{\mathsf T}\Omega_xM
      =C^{\mathsf T}\Omega_eC,
\]

and

\[
\Xi=\Omega_x^{-1}-M\Omega_x^{-1}M^{\mathsf T}
   =B\Omega_e^{-1}B^{\mathsf T}.
\]

For invertible `M`, prove `rank Xi = rank Delta`. Treat `a=0` directly.
Require

\[
2m\ge \operatorname{rank}\Delta,
\quad
\operatorname{rank}B\ge\operatorname{rank}\Delta,
\quad
\operatorname{rank}C\ge\operatorname{rank}\Delta.
\]

The target lower bounds are two canonical bath pairs at `d=0` and three at
every `d>0`, including unit drain.

## 4. Constructive minimum dilation

For each defective canonical pair with system scales `(lambda,a)` and
`0<a<=1`, set

\[
\beta=\sqrt{1-a\lambda}
\]

and freeze the map

\[
\begin{aligned}
q'&=\lambda q+\beta Q, &
p'&=a p+\beta P,\\
Q'&=-\frac{\beta}{a}q+Q, &
P'&=-a\beta p+a\lambda P.
\end{aligned}
\]

At `a=0`, freeze the finite boundary map

\[
q'=\lambda q+Q,\quad p'=P,\quad
Q'=-q,\quad P'=-p+\lambda P.
\]

The direct sum over defective directions, plus the unchanged defect-free
system pair, must be symplectic below `1e-12`. With `Q=P=0`, its projected
system output must reproduce the frozen genesis derivative below `1e-12`.
The number of coupled bath pairs must attain `rank Delta/2` exactly.

## 5. No-reset discriminator

Apply the same minimum dilation twice from a zero bath. For `a>0`, verify the
second projected step differs from two production steps by

\[
\delta q_2=-\frac{\beta^2}{a}q,
\qquad
\delta p_2=-a\beta^2p.
\]

At `a=0`, verify `delta q_2=-q` and `delta p_2=-p`. Use nonzero registered
fixtures for both coordinates. Every defective pair must show the analytic
nonzero deviation. No force multiplier or bath reset may be inserted.

Also prove the general fixed-preparation statement: if `e=0` is invariant for
all system states, then `C=0`; the upper-left symplectic identity forces
`Delta=0`. A noncanonical system map therefore cannot repeat indefinitely on
one fixed invariant zero-bath section.

## 6. Passive-energy discriminator

Assume the enlarged linear map preserves the equal-weight positive quadratic
energy

\[
H=\tfrac12\left(|x|^2+|e|^2\right).
\]

Then `S` is both orthogonal and symplectic, hence commutes with the canonical
complex structure. Its system block must satisfy

\[
\Omega_xM=M\Omega_x.
\]

For `M=diag(Lambda,aI)`, this requires `Lambda=aI`. Register the exact
commutator and require it to be nonzero in all 120 genesis arms. This closes
only a passive equal-weight quadratic reservoir; active, cross-coupled, or
nonquadratic environmental energies remain open.

## 7. Acceptance and failure consequences

Pass requires:

- all 120 defect-rank arms correct;
- all 330 defective-pair minimum dilations symplectic below `1e-12`;
- prepared projection below `1e-12`;
- bath-pair count exactly `rank Delta/2`;
- analytic two-step deviations below `1e-12` and nonzero in every defective
  pair arm;
- the fixed-zero-section theorem and passive-energy commutator theorem proved;
- an independent symbolic/numerical implementation reproducing the result;
- golden production hashes unchanged.

If the construction fails, only the lower bound survives. If it passes, the
result is classified as a constructive prepared-bath dilation, not a native
environment. It licenses no production toggle, stochastic ontology, particle,
unitarity, scenario, or Lorentz claim.

The preregistered verdict string is:

```text
MINIMAL_FEEDBACK_DILATION_REQUIRES_RESET_OR_ACTIVE_ENERGY_RESERVOIR
```

## 8. Locked source provenance

```text
FTD-0571 theorem   B7BD5590D5DDD2B5A23DC4E5DF0B6BD3DDC7ED124FD93A4C66C8BA3FE80B45B6
FTD-0571 header    2F8B7A7610E06E49957B35ED795A3A9DCF43BF0FE2288B4296D7B2214FCC76AB
FTD-0571 source    4DE62DC51CF6C660020D8FC8DEE9D38BE11C5FF2A774C08CE0E3707346B28CCB
phase_write.cpp    2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4
voxel.h            8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3
```
