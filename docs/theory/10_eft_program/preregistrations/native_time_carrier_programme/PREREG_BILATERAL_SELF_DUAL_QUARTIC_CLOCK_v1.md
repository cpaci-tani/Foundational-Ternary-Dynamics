# FTD-0835 — Bilateral self-dual quartic clock v1

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; CERTIFICATE INVALID 16/17]`  
**Date:** 2026-08-10  
**Scope:** exact two-channel recursion, nonlinear quartic energy coordinate,
weighted traversal period, and conditional stabilization ledger  
**Production impact:** none

## 1. Registered question

Starting from the already selected critical quartic Hamiltonian

\[
 H(q,p)=\frac{p^2}{2m}+\lambda q^4,
 \qquad m,\lambda>0,
\]

does its normalized energy shell admit a minimal oriented two-channel
description in which:

1. the channel exchange has order four and preserves a self-dual quadratic
   energy;
2. the quartic shell is exactly the nonlinear lift of that self-dual circle;
3. Hamiltonian time becomes a state-dependent traversal weight whose full
   integral is `sqrt(pi) G*`;
4. a declared radial feedback has a locally stable unit-energy shell; and
5. an explicit environment variable closes the feedback energy ledger?

## 2. Input firewall

This protocol assumes rather than derives the critical quartic Hamiltonian.
It does not test or assert:

- a native production-substrate mechanism selecting `lambda q^4`;
- a bounded matter clock, brain model, neural lateralization law, or theory of
  consciousness;
- a physical identification of successive ticks with increasing primes or
  Frobenius operators;
- Born-frequency recovery, actualization outcomes, or production-engine
  integration; or
- autonomous stabilization. The radial feedback is an `[IMPOSED]` reference
  controller whose energy exchange must be booked explicitly.

The labels `left` and `right` mean two mathematical channels only.

## 3. Frozen construction

Use the forward oriented quarter-turn

\[
 \mathcal J=
 \begin{pmatrix}0&1\\-1&0\end{pmatrix},
 \qquad \mathcal J^2=-I,
 \qquad \mathcal J^4=I.
\]

For normalized quartic coordinates

\[
 x=\frac qA,
 \qquad
 y=\frac{p}{\sqrt{2m\lambda}\,A^2},
 \qquad
 s=A\sqrt{\frac{2\lambda}{m}}\,t,
\]

freeze

\[
 y^2+x^4=1,
 \qquad
 \frac{dx}{ds}=y,
 \qquad
 \frac{dy}{ds}=-2x^3.
\]

Define the signed potential-energy coordinate

\[
 u=f(x)=x|x|,
 \qquad f^{-1}(u)=\operatorname{sgn}(u)\sqrt{|u|}.
\]

Then the registered self-dual energy is

\[
 \mathcal E=u^2+y^2=x^4+y^2.
\]

The nonlinear quarter-turn lift is fixed by conjugacy,

\[
 \mathcal D_4=\Phi^{-1}\mathcal J\Phi,
 \qquad
 \Phi(x,y)=(f(x),y).
\]

No alternative coordinate warp, denominator, period normalization, or
stability law may be substituted after execution.

## 4. Frozen exact checks

The certificate must check, without fitted tolerances:

1. `J^T J=I`;
2. `J^2=-I`;
3. `J^4=I`;
4. the exact four-state ternary orbit;
5. quadratic channel-energy invariance;
6. nonzero forward phase current with fixed negative sign;
7. sign reversal for `J^-1=-J`;
8. `Sym^2(J)=Sym^2(-J)`;
9. `u=x|x|` maps `x^4+y^2=1` to `u^2+y^2=1`;
10. `D_4^2=-I` and `D_4^4=I` by conjugacy;
11. the normalized quartic flow preserves its shell;
12. in each open quadrant the induced `(u,y)` flow is
    `2 sqrt(|u|) J(u,y)`;
13. the angular velocity is `-2 sqrt(|u|)`;
14. the full weighted traversal is
    `Beta(1/4,1/2)=sqrt(pi) Gamma(1/4)/Gamma(3/4)`;
15. dimensional restoration gives
    `T A=sqrt(pi) G* sqrt(m/(2 lambda))`;
16. the declared radial controller has local energy multiplier
    `1-2 eta`, stable conditionally for `0<eta<1`; and
17. the environment update closes total energy exactly.

## 5. Locked implementation and execution

Implementation:

```text
scripts/proofs/proof_bilateral_self_dual_quartic_clock.py
```

Script SHA-256:
`D303CD1B7F3DF18B3803EB074752E9C1C719A44A1C999AA5D978486A90FDDAD9`

With this document locked, record its SHA-256 in the manifest and run exactly
once:

```text
python scripts/proofs/proof_bilateral_self_dual_quartic_clock.py
```

## 6. Outcomes

- **Outcome A — exact conditional theorem:** all 17 checks pass. The
  self-dual coordinate representation, order-four lift, weighted `G*`
  traversal, and conditional feedback ledger may be booked exactly within
  the selected quartic model.
- **Outcome B — construction fails:** any check fails. Archive the proposed
  coordinate package as closed negative; no partial numerical near-match may
  be promoted.

Neither outcome derives native quarticity or autonomous clock hardware.

## 7. Recorded outcome

The hash-matching script ran once and returned `16/17`. Checks C1--C16
passed. C17 failed because the script compared the structurally different but
algebraically identical SymPy expressions

```text
-4*eta*(eta - 1)
4*eta*(1 - eta)
```

with Python structural equality. Their symbolic difference simplifies
exactly to zero, and the separately computed bath-ledger residual is zero,
but those diagnostics were obtained after the locked run. Under the frozen
outcomes, FTD-0835 does not book the coordinate theorem. The v1 certificate
package is invalid and no partial pass is promoted.
