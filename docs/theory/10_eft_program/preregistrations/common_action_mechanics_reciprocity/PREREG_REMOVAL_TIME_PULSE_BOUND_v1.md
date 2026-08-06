# Preregistration — Removal-Time Pulse Bound (FTD-0589)

**Status:** `[LOCKED — RUN OF RECORD NOT YET EXECUTED]`  
**Date:** 2026-07-26  
**Production effect:** none.

## 1. Question

FTD-0588 closes every asynchronous history through four sources and closes a
five-source history until its last original source disappears. Its remaining
envelope treats every removal as an independently bounded negative step and
therefore retains the constant part of the step response twice.

This gate asks whether the exact finite-duration rectangular-pulse response
closes that all-off tail, and how far the resulting arbitrary-removal theorem
extends before a removal-time Gram estimate is needed. No position, polarity,
duration, source-count, amplitude, or threshold search is permitted.

## 2. Frozen production contract

| source | SHA-256 |
|---|---|
| `engine/src/render_bridge.cpp` | `A822E0FAFAF71FE5458B2A7450868A8414B1C8564089BF6C6484FC34B7559359` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/term_toggles.h` | `2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA` |
| `engine/include/ftd/ontic/gauge_couplings.h` | `BC862D8120E0F3D83B7FAD0201F8D4DF46B5BAD5E7D52CD571AF68BECA3EB0F3` |
| `engine/include/ftd/ontic/particle_masses.h` | `E43E01D5F1F870EE019754BEA7E932529346C9B8EB704B40215CA559FC5A4F57` |

The live sector is unchanged from FTD-0586/0588: CPU, periodic field
quotient, unit tick, single substrate, zero initial flux/wave velocity/matter
kinematics, and only wave propagation, state-gradient coupling, and
genesis/evaporation. Gauss projection, damping, forces, movement, pair
production, reactions, clocks, and baths remain off.

## 3. Locked derivation

For each nonzero production mode, put

\[
 a(k)=C_{\rm WAVE}^2M(k),\qquad
 \cos\theta(k)=1-\frac{a(k)}2,
\]

and use the exact zero-field step response already derived in FTD-0586,

\[
 r_n=1-\cos(n\theta)+\tan(\theta/2)\sin(n\theta).
\]

A source present from tick zero through removal tick `T` has, at `n>=T`, the
normalized rectangular-pulse response

\[
 p_{n,T}=r_n-r_{n-T}.
\]

The constant terms cancel before any inequality is taken. The required exact
identity is

\[
 \boxed{
 p_{n,T}=2\sec(\theta/2)\sin(T\theta/2)
 \sin\!\left((n-(T-1)/2)\theta\right)}.
\]

Consequently every duration and observation tick obeys

\[
 |p_{n,T}(k)|\le 2\sec(\theta(k)/2)
 =\frac{2}{\sqrt{1-a(k)/4}}.
\]

Define the exact one-source pulse triangle bound

\[
 \boxed{
 P_L=\frac{2G_C}{C_{\rm WAVE}^2L^3}
 \sum_{k\ne0}
 \frac{\sqrt{\sum_a\sin^2k_a}}
 {M(k)\sqrt{1-C_{\rm WAVE}^2M(k)/4}}.}
\]

Let `C_L` be the FTD-0588 common-step coefficient. At an arbitrary tick
before the first descendant genesis event, suppose `r` of `N` distinct
original sources have been removed. The `N-r` sources still present share one
common step history and the removed sources are exact finite pulses. Hence

\[
 \boxed{
 |J(x,n)|\le H_L(N,r):=C_L\sqrt{N-r}+rP_L,
 \qquad 0\le r\le N.}
\]

This statement is uniform in source positions, polarity signs, removal ticks,
observation site, and observation tick. In particular, the all-off residual
tail obeys `|J|<=N P_L`.

The independent verifier must also establish the continuous relaxation

\[
 H_L(N,r)\le NP_L+\frac{C_L^2}{4P_L},
\]

by completing the square in `sqrt(N-r)`. The exact registered decision uses
the sharper discrete maximum

\[
 H_L^{\max}(N)=\max_{r\in\{0,\ldots,N\}}H_L(N,r).
\]

For each `L={9,17,33,65}`, compute `P_L` independently in C++ long double and
Python binary64. Starting at `N=1`, report the largest consecutive source
count satisfying

\[
 H_L^{\max}(N)+10^{-12}<K_{\rm GENESIS}.
\]

This monotone threshold extraction is part of the theorem and is not a search
over physical constants, geometries, or near matches. The uniform closed count
is the minimum across the four registered volumes; the next count is reported
only as not excluded by this bound.

## 4. Exact removal-time Gram record

For conformance, define the real vector kernel

\[
 K_L(d,n,T)=\frac{G_C}{C_{\rm WAVE}^2L^3}
 \sum_{k\ne0}\frac{-i\sin k}{M(k)}p_{n,T}(k)e^{ik\cdot d}.
\]

For fixed prescribed source histories, the exact residual norm must satisfy

\[
 \left|\sum_jq_jK_L(x-x_j,n,T_j)\right|^2
 =q^T\mathcal Gq,
 \qquad
 \mathcal G_{ij}=K_i\cdot K_j.
\]

The Gram identity is a diagnostic equality, not an optimization over removal
times. No schedule may be selected because it raises or lowers the field.

## 5. Independently fixed conformance fixtures

Run 64 prescribed-history arms:

- `L={9,17}`, `N={5,6}`, both global polarities, two moment-isotropic shape
  variants, and four histories;
- `N=5`: either tetrahedral chirality plus the center;
- `N=6`: the axial octahedral orbit at radius one or radius two;
- histories: permanent step, synchronous removal at tick 16, staggered
  removals `T_j=4(j+1)`, and paired removals
  `(8,8,16,16,24[,24])`;
- 128 ticks per arm.

Run 32 native-unlocked arms over the same volumes, counts, polarities, and
shape variants with two fixed seeds beginning at `0x05890000`; 128 ticks per
arm. These histories are measurements of production evaporation, not a sample
from which the theorem is inferred.

The observer must additionally verify the pulse identity, Gram equality,
translations, and all 24 proper cubic rotations on the prescribed histories.

## 6. Acceptance gates

- every frozen source hash and the preregistration hash match;
- symbolic expansion proves the pulse identity exactly;
- the C++ and independent Python `P_L`, `H_L(N,r)`, closed count, and first
  unexcluded count agree within `5e-15`;
- every prescribed arm respects its exact history bound within `1e-12`;
- every live arm whose source count is theorem-closed records zero descendant
  genesis;
- at least one native-unlocked arm in every `(L,N)` cell removes all original
  sources, so the residual branch is exercised;
- direct vector norm and the removal-time Gram quadratic form agree within
  `1e-12`;
- translations and 24 proper cubic rotations agree within `1e-12`;
- velocity and remainder remain bit-exact zero;
- history journaling is state/RNG neutral;
- no production/default/toggle/scenario file is modified by the observer.

## 7. Verdicts

- `ARBITRARY_REMOVAL_N_LE_Q_CLOSED_NEXT_COUNT_UNRESOLVED`: all analytic and
  conformance gates pass, where `Q` is the uniform count forced by the locked
  inequality.
- `BOUND_CONTRADICTION`: a theorem-closed live arm generates a descendant or
  exceeds its registered bound.
- `INVALID`: a frozen hash changes, an identity/numerical cross-check fails,
  a forbidden mechanism enters, or any tolerance is relaxed.

No verdict establishes reciprocal recoil, identity transport, a particle,
self-maintaining matter, a pole, Lorentz recovery, or a dashboard scenario.
The only permitted next step after a positive bound result is a preregistered
Gram/coherence attack at the first source count not excluded.
