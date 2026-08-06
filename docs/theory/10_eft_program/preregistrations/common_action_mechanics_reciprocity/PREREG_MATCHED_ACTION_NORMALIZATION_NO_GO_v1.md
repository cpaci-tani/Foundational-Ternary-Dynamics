# PRE-REGISTRATION — Matched-action normalization no-go v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0486`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0484` and `FTD-0485`

## Question

Can one minimally coupled selected link-connection action simultaneously
reproduce the normalized matched Gauss law, the matched source update
`E <- E-K`, and the frozen `FTD-0479` force coefficient multiplying both
`E` and `v cross B` when `C_SPEED != 1`?

## Frozen action

Let `G=-D^T`, `R=C^T`,

```text
E = -(1/c_f) A_dot - G Phi,
B = R A,
L_field = kappa/2 (|E|^2-|B|^2),
L_int = g q (A.v-c_f Phi).
```

Use `c_f=C_SPEED=C_WAVE` and the already locked face normalization

```text
kappa = C_WAVE^2 z^2 = G_C z,
z = G_C/C_WAVE^2.
```

No alternative field normalization, magnetic redefinition, or force gather is
admitted in this gate.

## Algebraic gates

Variation must give

```text
D E = (g c_f/kappa) rho,
E_dot = c_f C B - (g c_f/kappa) j.
```

Exact matched Gauss and source normalization therefore require

```text
g = kappa/c_f.
```

The same worldline variation then gives

```text
F = kappa q (E + (v cross B)/c_f).
```

The test must verify below `1e-15` that Gauss/source coefficients equal one,
electric work has coefficient `kappa`, field/matter power cancels, and the
magnetic/electric force-coefficient ratio is `1/c_f`.

The frozen equal-coefficient `FTD-0479` force is compatible only if `c_f=1`
or `kappa=0`. Since the production values have `c_f=1/sqrt(3)` and
`kappa>0`, the predeclared expected verdict is
`MATCHED_SOURCE_AND_EQUAL_FORCE_COEFFICIENTS_INCOMPATIBLE`.

This is a theorem conditional on the selected minimal action. It is not a
no-go against non-minimal actions or an explicitly redefined physical magnetic
field. Neither escape is authorized in this campaign.

Run-of-record test-source SHA256:
`00C18C323E581E9678EABBF31F01AD0981EB7BF23EE34488F70EE927AEF2CA90`.
