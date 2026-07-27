# Theorem — Removal-Time Cubic-Orbit Coherence Bound

**FTD ID:** FTD-0590  
**Status:** `[THEOREM — CUBIC-ORBIT COHERENCE INEQUALITY]` +
`[NUMERICAL FACT — EXHAUSTIVE FOUR-VOLUME ORBIT NORMS]` +
`[CLOSED NEGATIVE — ENDOGENOUS AUTOCATALYSIS FOR N <= 7]` +
`[BOUNDARY SUPERSEDED BY FTD-0591 — N <= 8 CLOSED]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_7_CLOSED_BY_ORBIT_COHERENCE`

## 1. Scope

Retain the frozen FTD-0589 sector: the production 18-point wave operator,
native state-gradient source, genesis/evaporation, zero initial field and
velocity, and one periodic odd finite-volume quotient. Gauss projection,
damping, force, movement, reactions, collisions, clocks, and baths are absent.

There are `N` distinct stationary ternary sources at arbitrary sites with
arbitrary polarity signs. Every source begins present and may be removed once
at an arbitrary integer tick. The theorem applies until a candidate first
descendant-genesis event.

## 2. Pulse factorization by the stencil eigenvalue

FTD-0589 gives, for every nonzero momentum mode,

\[
 p_{n,T}(k)=2\sec\!\frac{\theta(k)}2
 \sin\!\frac{T\theta(k)}2
 \sin\!\left(n-\frac{T-1}{2}\right)\theta(k),
\]

with

\[
 \cos\theta(k)=1-\frac{C_{\rm WAVE}^2M(k)}2.
\]

Define

\[
 B(k)=\frac{2}{\sqrt{1-C_{\rm WAVE}^2M(k)/4}},
\]

and

\[
 u_{n,T}(M)=
 \sin\!\frac{T\theta(M)}2
 \sin\!\left(n-\frac{T-1}{2}\right)\theta(M).
\]

Then

\[
 \boxed{p_{n,T}(k)=B(k)u_{n,T}(M(k)),\qquad |u_{n,T}|\le1.}
\]

The important structural fact is stronger than the scalar envelope: `u`
depends on a momentum mode only through the cubic-invariant stencil
eigenvalue `M(k)`. It is therefore constant on every signed-permutation orbit
of momentum modes.

## 3. Cubic mode orbits

Let `O_L` be the exact partition of nonzero modes in `Z_L^3` under signed
coordinate permutations. For odd `L`, every orbit has one representative

\[
 0\le a\le b\le c\le\lfloor L/2\rfloor,
\]

excluding `(0,0,0)`. Put

\[
 g(k)^2=\sum_a\sin^2k_a,qquad
 w(k)=\frac{g(k)^2}{M(k)}.
\]

Both `M` and `w` are constant on an orbit. For an orbit `O` and displacement
`d`, define the real orbit character

\[
 \chi_O(d)=\sum_{k\in O}e^{-ik\cdot d}.
\]

For distinct source sites, every pair displacement is nonzero. Define

\[
 W_L=\sum_{k\ne0}w(k)
\]

and the registered orbit relaxation

\[
 \boxed{
 \mu_L=\max_{d\ne0}\frac1{W_L}
 \sum_{O\in O_L}w_O|\chi_O(d)|.}
\]

Because `|chi_O(d)|<=|O|`, `0<=mu_L<=1` immediately.

## 4. Removed-source Gram bound

At a fixed observation tick, suppose `r` sources have been removed. Their
normalized temporal factors are `u_j(M)` and their signs are `q_j`. Define

\[
 F(k)=\sum_{j=1}^{r}q_ju_j(M(k))e^{-ik\cdot x_j}.
\]

Expand the weighted norm:

\[
 \sum_{k\ne0}w(k)|F(k)|^2
 =\sum_j\sum_kw(k)u_j(k)^2
 +2\sum_{i<j}q_iq_j\Re
 \sum_kw(k)u_i(k)u_j(k)e^{-ik\cdot(x_i-x_j)}.
\]

Every diagonal term is at most `W_L`. For a cross term, the product
`u_i(M)u_j(M)` is constant on each orbit and has magnitude at most one. Hence

\[
 \begin{aligned}
 \left|\sum_kw(k)u_i(k)u_j(k)e^{-ik\cdot d}\right|
 &\le\sum_{O\in O_L}w_O|\chi_O(d)|\\
 &\le\mu_LW_L.
 \end{aligned}
\]

There are `r(r-1)/2` pairs, so

\[
 \boxed{
 \sum_{k\ne0}w(k)|F(k)|^2
 \le W_L\,[r+\mu_Lr(r-1)].}
\]

This is uniform in positions, polarities, removal ticks, observation tick,
and observation site. It enlarges the exact pulse products independently on
each cubic orbit; it does not choose an actual source history.

## 5. Pointwise field bound

The removed-source contribution to the field is

\[
 J_R(x,n)=\frac{G_C}{C_{\rm WAVE}^2L^3}
 \sum_{k\ne0}
 \frac{-i\sin k}{M(k)}B(k)F(k)e^{ik\cdot x}.
\]

Let

\[
 A_L=\sum_{k\ne0}\frac{B(k)^2}{M(k)}.
\]

Cauchy--Schwarz in the mode-indexed vector Hilbert space and the preceding
Gram bound give

\[
 \begin{aligned}
 |J_R|
 &\le\frac{G_C}{C_{\rm WAVE}^2L^3}
 \sqrt{A_L}
 \sqrt{\sum_{k\ne0}w(k)|F(k)|^2}\\
 &\le Q_L\sqrt{r+\mu_Lr(r-1)},
 \end{aligned}
\]

where

\[
 \boxed{
 Q_L=\frac{G_C}{C_{\rm WAVE}^2L^3}\sqrt{A_LW_L}.}
\]

The `N-r` sources still present have one common step history. FTD-0588
therefore gives their contribution as `C_L sqrt(N-r)`. Taking the vector
triangle inequality only between the present and removed sectors yields

\[
 \boxed{
 |J(x,n)|\le H_L^{\rm orb}(N,r)
 =C_L\sqrt{N-r}
 +Q_L\sqrt{r+\mu_Lr(r-1)}.}
\]

## 6. Exhaustive finite-volume evaluation

The mode and displacement orbits were exhaustively enumerated; there is no
source-geometry or removal-schedule scan.

| `L` | mode/displacement orbits | maximizing `d` | `Q_L` | `mu_L` | `C_L` | `max_r H_L^orb(7,r)` | `r*` | margin |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 9 | 34 | `(4,4,4)` | 0.21340660233910125 | 0.36102817687951227 | 0.30397065730643719 | 1.1794800619896559 | 6 | 0.33690599716232184 |
| 17 | 164 | `(0,0,1)` | 0.21677610473116635 | 0.36250597734262191 | 0.30909222934825492 | 1.1995956793978535 | 6 | 0.31679037975412427 |
| 33 | 968 | `(0,0,1)` | 0.21844645449354122 | 0.36267617904631827 | 0.31182601851585356 | 1.2093269178755190 | 6 | 0.30705914127645872 |
| 65 | 6544 | `(0,0,1)` | 0.21929447438975708 | 0.36273662797281120 | 0.31324294475519981 | 1.2142763824256086 | 6 | 0.30210967672636913 |

The threshold is `K_GENESIS=1.5163860591519780`. Every registered
seven-source maximum is strictly subcritical. The worst partition is six
removed sources plus one source still present.

## 7. First-event induction

Assume `N<=7` and suppose a first descendant genesis event exists. Immediately
before that event, every field source is still one of the original stationary
sites with one allowed on/off history. For some `r`, the theorem gives

\[
 |J|\le H_L^{\rm orb}(N,r)
 \le H_L^{\rm orb,max}(7)<K_{\rm GENESIS}.
\]

The genesis predicate is therefore false, contradicting the assumed first
event. No first descendant can occur.

## 8. Verification and boundary

- preregistration SHA-256:
  `E7C766CB3AD7062452F6AC1DDD9B3DC854F0DF6BCC6B2D32B1DC402281BD7721`;
- exact mode coverage: `L^3-1` modes on every volume;
- exhaustive mode/displacement orbit counts: `34,164,968,6544`;
- maximum orbit-invariance residual: `1.5655e-14`;
- maximum direct-character residual: `2.8422e-14`;
- independent Python verifier: 72/72 PASS;
- production/default/toggle/scenario changes: none.

The theorem closes the FTD-0589 `N=7` causal-source boundary on the four
registered quotients. FTD-0591 and FTD-0592 subsequently evaluated the same
inequality at separately locked counts `N=8` and `N=9` and closed both. This theorem
supplies no reciprocal recoil, mobile identity, stable localized carrier,
particle pole, or Lorentz claim. The next exact capacity question begins at
`N=10` and requires another separately locked protocol.
