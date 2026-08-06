# AUDIT — Discrete Legendre worldline

**Date:** 2026-07-25  
**Identifier:** `FTD-0490`  
**Status:** `[THEOREM — INTERIOR GAUGE-COVARIANT DISCRETE LEGENDRE MAP]` +
`[CONSTRUCTIVE — PRODUCTION DISPERSION INVERSE]` +
`[OPEN — KNOT BRANCH]`  
**Verdict:** `INTERIOR_DISCRETE_LEGENDRE_GAUGE_COVARIANT`  
**Pre-registration:**
[`PREREG_DISCRETE_LEGENDRE_WORLDLINE_v1.md`](../10_eft_program/preregistrations/PREREG_DISCRETE_LEGENDRE_WORLDLINE_v1.md)  
**Run of record:** `engine/results/ftd_0490/windows_msvc_cpu.json`

## 1. Corrected classical equation

FTD-0489 proves that bare open-worldline action values cannot be ordered
across different charged endpoints. That is not the classical variational
update. For a one-step action

```text
S_d(x0,x1)=S_m(x1-x0)+S_int(x0,x1),
```

the endpoint equations use the discrete Legendre transforms

```text
P0=-D1 S_d,
P1= D2 S_d.
```

The exact gauge endpoint term implies

```text
P0'=P0+gq grad chi0(x0),
P1'=P1+gq grad chi1(x1).
```

The Q1/Nedelec commuting identity gives the same shift to the interpolated
connection. Therefore

```text
Pi_n=P_n-gq A_n(x_n)
```

is gauge invariant. The locked general-field fixtures close canonical
covariance to `6.94e-18` and kinetic invariance to `1.39e-17`. A nonzero pure
gauge reproduces the free kinetic endpoint momenta exactly.

## 2. Production dispersion is the matter action

With `lambda=c dt`, the frozen free action is

```text
S_m=-(E_REST lambda/c) sqrt(1-|d|^2/lambda^2).
```

Its derivative and inverse are

```text
p=E_REST d/(c lambda sqrt(1-|d|^2/lambda^2)),
d=lambda c p/sqrt(E_REST^2+c^2|p|^2).
```

This is exactly the existing production dispersion
`E(p)^2=E_REST^2+c^2|p|^2`. Axial and diagonal inverse fixtures close to
`3.99e-17`, without importing a continuum force formula.

## 3. Action/current identity

Six-variable automatic differentiation evaluates both endpoint derivatives of
the exact within-cell Q1/Nedelec line action. Its action value agrees with the
independent FTD-0484 deposited-current contraction to `4.34e-19`. Thus the
Legendre observer differentiates the same current-generating interaction; it
does not append an electric or magnetic gather.

All `12/12` registered checks pass, including both polarities, pure gauge,
translation, proper cubic rotation, and invalid-input controls.

## 4. Claim boundary

This result reopens one statement at restricted scope: a common-action
classical update is possible in the interior of a single cubical cell. It does
not reopen the frozen FTD-0479/0480 force gather and does not license a
production branch.

The implementation deliberately rejects cell faces. At a manifestation knot,
the connection/action has distinct adjacent-cell derivatives, and FTD-0487
proves nonzero Gauss charge forces at least one such jump. The next gate must
solve the same canonical input against every incident cell branch and count
the admissible endpoints. A unique gauge-covariant branch is required before
mobile matter can proceed.

## 5. Reproducibility

- test SHA256:
  `057C63E6794F5AD274A3027BDE3F126223B7F9E6C6EB79A488816EA0D22936F5`;
- header SHA256:
  `AE670F2E06C442FE0AEF652E916F22D2F424EB523AE1DF6D67941F4EAEDD8C43`;
- implementation SHA256:
  `FFCDAE3F6D99655E1FD607E0354A7FCFA4FFB0460B45737BFB588511142BE9C3`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state: unchanged.
