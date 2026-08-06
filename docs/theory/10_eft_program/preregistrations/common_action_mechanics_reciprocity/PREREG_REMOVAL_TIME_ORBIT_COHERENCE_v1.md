# PRE-REGISTRATION — Removal-time cubic-orbit coherence bound v1

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0590`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE SPECTRAL EVALUATION]`  
**Parent:** `FTD-0589`  

## Question

Does the exact cubic-orbit coherence retained by the FTD-0589 rectangular
pulse kernel make every arbitrary one-time-removal history with seven distinct
stationary sources strictly subcritical on the four registered finite
quotients?

This protocol does not select source geometry, polarity, removal time,
observation time, or observation site by the resulting field. It evaluates a
single enlarged operator bound uniform over all of them.

## Frozen sector

Use exactly the FTD-0589 linear source sector and conventions:

- periodic odd quotients `L={9,17,33,65}`;
- production 18-point wave symbol `M(k)` and centered-gradient magnitude
  `g(k)^2=sum_a sin(k_a)^2`;
- native constants `G_C`, `C_WAVE`, and `K_GENESIS`;
- `N=7` distinct stationary ternary sites with arbitrary signs;
- every source begins present and may be removed once at an arbitrary integer
  tick;
- zero initial field and velocity;
- no Gauss projection, damping, force, movement, collision, reaction, clock,
  bath, scenario, or production change.

The result applies only before a candidate first descendant-genesis event.

## Frozen derivation

For a removed source, write the exact FTD-0589 pulse as

\[
 p_{n,T}(k)=B(k)u_{n,T}(M(k)),\qquad
 B(k)=\frac{2}{\sqrt{1-C_{\rm WAVE}^2M(k)/4}},
 \qquad |u_{n,T}|\le1.
\]

Because `u` depends on a mode only through `M(k)`, it is constant on every
signed-permutation cubic orbit of momentum modes. Let `O_L` be the exact
partition of all nonzero modes into those orbits. Define

\[
 w(k)=\frac{g(k)^2}{M(k)},\qquad
 W_L=\sum_{k\ne0}w(k),\qquad
 A_L=\sum_{k\ne0}\frac{B(k)^2}{M(k)}.
\]

For an orbit `O` and nonzero displacement `d`, define its character

\[
 \chi_O(d)=\sum_{k\in O}e^{-ik\cdot d}.
\]

The preregistered coherence relaxation is

\[
 \boxed{
 \mu_L=\max_{d\ne0}\frac{1}{W_L}
 \sum_{O\in O_L}w_O|\chi_O(d)|.}
\]

Here `w_O` is the common value of `w(k)` on the orbit. Relaxing the pulse
product independently on each orbit can only enlarge the exact cross term, so
for `r` removed sources

\[
 \sum_{k\ne0}w(k)
 \left|\sum_{j=1}^r q_j u_j(M(k))e^{-ik\cdot x_j}\right|^2
 \le W_L\,[r+\mu_Lr(r-1)].
\]

Cauchy--Schwarz then gives the removed-source field bound

\[
 R_L(r)=Q_L\sqrt{r+\mu_Lr(r-1)},\qquad
 Q_L=\frac{G_C}{C_{\rm WAVE}^2L^3}\sqrt{A_LW_L}.
\]

Combining it by the vector triangle inequality with the FTD-0588 common-step
bound for the `N-r` sources still present gives

\[
 \boxed{
 H_L^{\rm orb}(N,r)=C_L\sqrt{N-r}
 +Q_L\sqrt{r+\mu_Lr(r-1)}.}
\]

The registered decision statistic is

\[
 H_L^{\rm orb,max}(7)=\max_{r=0,\ldots,7}H_L^{\rm orb}(7,r).
\]

No tighter grouping, fitted weight, geometry restriction, schedule
restriction, or tolerance change is permitted after evaluating `mu_L`.

## Exact finite enumeration

Because all registered `L` are odd, every mode and displacement orbit has one
canonical representative

```text
0 <= a <= b <= c <= floor(L/2).
```

The implementation must:

1. prove by explicit integer enumeration that the nonzero mode orbits are
   disjoint and contain exactly `L^3-1` modes;
2. prove `M`, `g^2`, and `w` are invariant inside each orbit;
3. evaluate every nonzero displacement orbit, not a sample;
4. compute each orbit character from the exact signed-permutation membership;
5. verify the maximizing character sum independently by direct full-mode
   accumulation;
6. evaluate the same four volumes in C++ `long double` and an independent
   Python binary64 implementation.

## Tolerances

- orbit coverage/count errors: exact integer zero;
- within-orbit symbol invariance: `<=5e-14`;
- character formula versus direct membership: `<=5e-13`;
- `0<=mu_L<=1`: exact logical gate with `1e-14` numerical slack;
- C++/Python agreement for `A_L`, `W_L`, `Q_L`, `mu_L`, and
  `H_L^{orb,max}`: `<=5e-12`;
- final threshold comparison: strict, with no tolerance counted as margin.

## Outcome map

- If `H_L^{orb,max}(7) < K_GENESIS` for all four volumes:
  `ARBITRARY_REMOVAL_N_LE_7_CLOSED_BY_ORBIT_COHERENCE`.
- Otherwise:
  `ORBIT_COHERENCE_BOUND_INCONCLUSIVE_AT_N7`.
- Any coverage, invariance, direct-character, cross-language, or finiteness
  gate failure:
  `PROTOCOL_INVALID`.

A positive closure licenses only a first-event theorem in the frozen source
sector. An inconclusive result is not a seven-source genesis mechanism. Neither
outcome licenses geometry/schedule search, reciprocal force, mobile matter,
particle language, Lorentz recovery, a toggle, or a scenario.

## Required artifacts

- `engine/include/ftd/eft/removal_time_orbit_coherence.h`;
- `engine/src/eft/removal_time_orbit_coherence.cpp`;
- `engine/tests/test_removal_time_orbit_coherence.cpp`;
- `scripts/proofs/proof_removal_time_orbit_coherence.py`;
- `engine/results/ftd_0590/windows_msvc_cpu.{json,csv}`;
- theorem, analysis, audit, manifest, ledger, tracker, and index updates after
  adjudication.

