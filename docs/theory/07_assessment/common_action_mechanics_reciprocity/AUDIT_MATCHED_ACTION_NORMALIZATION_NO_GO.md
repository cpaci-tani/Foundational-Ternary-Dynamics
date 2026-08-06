# FTD-0486 — Matched-action normalization no-go

**Date:** 2026-07-25  
**Status:** `[THEOREM — CONDITIONAL ON SELECTED MINIMAL ACTION]`  
**Verdict:** `MATCHED_SOURCE_AND_EQUAL_FORCE_COEFFICIENTS_INCOMPATIBLE`

For the selected matched connection action

```text
E = -(1/c_f) A_dot - G Phi,
B = R A,
L_field = kappa/2 (|E|^2-|B|^2),
L_int = g q (A.v-c_f Phi),
```

variation gives the same source coefficient in Gauss and Ampere:

```text
D E = (g c_f/kappa) rho,
E_dot = c_f C B - (g c_f/kappa) j.
```

The exact matched requirements `D E=rho` and source update `E<-E-K` force

```text
g = kappa/c_f.
```

The particle variation is then not optional:

```text
F = kappa q (E + (v cross B)/c_f).
```

For the frozen values

```text
c_f = C_SPEED = 1/sqrt(3),
kappa = 0.021892057692994273,
```

the electric coefficient is `0.021892057692994273`, while the magnetic
coefficient is `0.037918156206495184`; their ratio is exactly `sqrt(3)`.
The frozen `FTD-0479` equal-coefficient gather misses by
`0.016026098513500911`. A unit-speed control restores equality, proving that
the mismatch is the explicit `c_f!=1` normalization and not rounding.

Electric work still closes: field power is `-kappa<E,j>`, particle electric
power is `+kappa<E,j>`, and magnetic work is zero. Thus energy conservation
does not select the incorrect equal magnetic coefficient.

This theorem is conditional on the selected minimal connection action. A
non-minimal coupling or the explicit definition `B_phys=B/c_f` evades it, but
either is a new selection. No such repair, production toggle, or scenario is
introduced.

Run of record: `engine/results/ftd_0486/windows_msvc_cpu.json`.
