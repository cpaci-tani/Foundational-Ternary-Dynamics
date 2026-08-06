# Pre-registration — Minimal Moore compatibility coat (FTD-0577)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date:** 2026-07-26  
**Parents:** FTD-0478, FTD-0574, FTD-0575, FTD-0576.  
**Production changes permitted:** none. Observer code, tests, proofs, theorem,
audit, and documentation corrections are permitted.

## 1. Question

FTD-0576 proved that a cardinal one-site polarity hop cannot be represented by
a finite-range current under the native central divergence. This campaign asks
whether the smallest symmetric separable coupling coat can cancel the central
checkerboard zero while leaving primitive manifestation exactly ternary.

The coat is a derived/selected coupling sidecar. It is not a fractional
primitive state, a production movement rule, a force law, or a promotion of
the face-flux ontology.

## 2. Frozen one-dimensional problem

Let a real, centered, radius-one filter have Laurent symbol

```text
B(z)=a(z+z^-1)+b.                                (1)
```

Require:

```text
B(1)=2a+b=1                         normalization,
B(-1)=-2a+b=0                       checkerboard cancellation. (2)
```

The registered solution is unique within this class:

```text
a=1/4, b=1/2,
B(z)=(z^-1+2+z)/4=(z+1)^2/(4z).                  (3)
```

The coefficients are nonnegative, have zero first moment, and sum to one.
No numerical search over filters is permitted.

## 3. Three-dimensional Moore coat

Use the separable cubic filter

```text
B_M=B_x B_y B_z.                                 (4)
```

At an integer-centered source its 27 Moore-neighborhood weights are:

```text
center:       1/8,
6 faces:      1/16 each, shell total 3/8,
12 edges:     1/32 each, shell total 3/8,
8 corners:    1/64 each, shell total 1/8.         (5)
```

The coat must have unit partition, zero first moment, nonnegative weights,
integer-translation covariance, and proper-cubic covariance. Applied to the
FTD-0478 trilinear subcell density, convolution must preserve signed polarity
and first-moment reproduction. The integer-centered representation is
explicitly non-cardinal because its central weight is `1/8`, not `1`.

The Fourier form factor is

```text
B_M(k)=product_i cos^2(k_i/2)
      =1-|k|^2/4+O(|k|^4).                        (6)
```

It annihilates every central-difference null sector with any `k_i=pi` while
preserving the zero mode.

## 4. Local face-to-central current bridge

Let the already-derived FTD-0478 oriented face current `K_i` obey

```text
delta rho_CIC + sum_i d_f,i K_i=0,
d_f,i(z_i)=1-z_i^-1.                              (7)
```

Define the coated density and site-centered current by

```text
rho_M=B_M rho_CIC,
Q_i=A_i product_(j!=i) B_j K_i,
A_i(z_i)=(1+z_i^-1)/2.                            (8)
```

With native central divergence

```text
d_c,i(z_i)=(z_i-z_i^-1)/2,                        (9)
```

the registered factorization is

```text
d_c,i A_i=B_i d_f,i,                              (10)
delta rho_M+sum_i d_c,i Q_i=0.                    (11)
```

Equation (8) is finite-range: smooth each face-current component in its two
transverse directions with `B`, then average it with its negative-axis
neighbor. It introduces no path-order variable beyond the continuous straight
segment already fixed by FTD-0478.

## 5. Conditional inherited energy identity

For any coated endpoint densities and bridged current satisfying (11), replay
the conditional FTD-0576 identity with the same central operators:

```text
rho_bar=(rho_M,0+rho_M,1)/2,
S=-G_C G rho_bar+G_C C Q,

delta H_field
 =G_C<rho_bar,D delta_R>+G_C<Q,C delta_R>,

delta U_int
 =-G_C<rho_bar,D delta_R>-G_C<Q,G D R_bar>,

delta H_matter
 =G_C<Q,G D R_bar-C delta_R>.                     (12)
```

The sum must vanish to the registered tolerance. This is a conditional work
ledger, not a derivation of a production matter update.

## 6. Registered arms and tolerances

- exact symbolic checks of equations (1)--(11), including scoped uniqueness;
- exact integer checks of all 27 weights, shell totals, partition, and first
  moment;
- volumes `L in {17,33}` and both polarities;
- all six axial directions plus the straight diagonal paths
  `(0.70,0.45,0.30)`, `(-0.60,0.55,-0.35)`, and `(0.80,-0.40,0.65)` in
  subcell lattice units;
- translated copies and all 24 proper-cubic rotations of a generic path;
- coated endpoint partition, signed first moment, positivity, local support,
  and central continuity;
- four deterministic conditional-energy fixtures using the bridged current;
- zero-mode preservation and coordinate-checkerboard annihilation;
- explicit cardinality discriminator at integer remainder;
- frozen production hashes and absence of a new toggle or scenario.

Every floating algebraic, continuity, covariance, first-moment, and energy
residual must be `<=1e-12`. Nonzero signed weights must have the polarity's
sign. Support must remain bounded independently of volume for paths confined
to one local chart.

## 7. Outcome map

Positive registered verdict:

```text
MINIMAL_MOORE_COAT_RESTORES_LOCAL_CENTRAL_CONTINUITY_NONCARDINAL_SELECTED
```

It establishes:

1. uniqueness only in the normalized, centered, separable, symmetric
   radius-one class;
2. an exact finite-range bridge from FTD-0478 face continuity to the native
   central complex;
3. compatibility with the conditional FTD-0576 energy ledger.

It does **not** establish uniqueness among nonseparable or nonlinear carriers,
a reciprocal force, self-force cancellation, a stable mobile particle, a
Coulomb pole, electromagnetism, Lorentz recovery, unitarity, or production
closure. The FTD-0575 static pole/sign obstruction remains unchanged.

Negative verdicts are registered separately for failure of uniqueness,
positivity/moments, exact central continuity, locality/covariance, or the
conditional energy identity. No post-hoc support enlargement, tolerance
change, or force amplification is authorized.

## 8. Frozen production provenance

The implementation must verify the following SHA-256 hashes and must not edit
these files:

```text
phase_read.cpp                  D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8
phase_write.cpp                 2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4
field_operators.h               25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48
native_energy_contract.h        3DB8F2DC573E7F4A87E17409878915E7B5A52CE1673713998C544516E0175621
```
