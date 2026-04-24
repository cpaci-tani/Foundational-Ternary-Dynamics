# Projected Response Eigenvalue and x_+ Attempt

**Date:** 2026-04-22
**Status:** [CLOSED NEGATIVE under current projected action] / [OPEN] if a new two-sector response matrix is derived
**Purpose:** Test whether the projected FTD-to-EFT bridge derives a kinetic-response matrix whose physical eigenvalue is `x_+`.

---

## Executive result

We do **not** need eigenvalues for a one-field coupling.

If the electromagnetic observable is a single coefficient,

```text
S_A = (K / 2) F^2,
```

then the physical question is just:

```text
is K = x_+?
```

No matrix and no eigenvalue are required.

Eigenvalues become relevant only if the projected EFT has multiple coupled sectors:

```text
S_quad = 1/2 Phi^T K Phi,
```

and the measured electromagnetic mode is a normal mode of `K`, rather than one raw matrix entry.

The master quadratic makes this possibility tempting because:

```text
x^2 - 16 G*^2 x + 16 G*^3 = 0
```

is naturally the characteristic polynomial of some `2 x 2` response matrix. But the current projected FTD action does not derive such a matrix. The longitudinal/source sector and transverse/radiative sector separate after Helmholtz projection, and the transverse sector remains canonically normalized.

Therefore the R3 route from `OPEN_PROJECTED_EFT_RENORMALIZATION_AND_ALPHA_OBSERVABLE.md`:

```text
R3. Derive a projected kinetic matrix whose physical eigenvalue is x_+.
```

is closed negative **for the current projected action**.

It remains a possible future matching rule only if a physical two-sector response basis is independently derived.

---

## Why eigenvalues were considered

The master quadratic gives two roots:

```text
x_+ = 137.036171...
x_- = 3.024...
```

with Vieta data:

```text
x_+ + x_- = 16 G*^2
x_+ x_- = 16 G*^3
```

Those are exactly the data of a characteristic polynomial:

```text
det(lambda I - K_2) = lambda^2 - tr(K_2) lambda + det(K_2).
```

For a symmetric two-sector response matrix

```text
K_2 = [[a, b],
       [b, d]],
```

matching the master quadratic requires:

```text
a + d     = 16 G*^2
ad - b^2  = 16 G*^3.
```

The equal-diagonal representative is:

```text
K_2 = [[8 G*^2,  b],
       [b,       8 G*^2]]

b^2 = 16 G*^3 (4G* - 1).
```

This is algebraically valid. It is also underdetermined: infinitely many matrices have the same trace and determinant. Therefore:

```text
matrix representation != physical derivation
```

To become physics, FTD must derive:

1. the two physical sectors,
2. the matrix entries,
3. the rule selecting the `x_+` eigenmode as electromagnetic `1/alpha`,
4. the matter and renormalization prescription.

Without those steps, the eigenvalue form is only a useful algebraic wrapper.

---

## Candidate sector tests

### 1. Longitudinal/source plus transverse/radiative sectors

The projected-flux bridge gives:

```text
J = J_L[rho] + J_T
khat . J_T = 0
```

with longitudinal flux fixed by the Gauss/source constraint.

At quadratic order the action has the schematic form:

```text
S_quad = S_L[rho, J_L] + S_T[J_T].
```

That is block diagonal. The longitudinal and transverse sectors do not produce a forced `2 x 2` kinetic matrix with trace `16G*^2` and determinant `16G*^3`.

The transverse block remains:

```text
S_T ~ 1/2 |Delta_t J_T|^2 - 1/2 c^2 |grad J_T|^2.
```

So the current action gives:

```text
K_T,0 = 1
```

up to conventional speed/stencil normalization, not:

```text
K_T,0 = x_+.
```

### 2. Source-current plus field variables

The projected matter coupling gives:

```text
S = 1/2 rho V rho + 1/2 A_T K_T A_T - j_T . A_T + S_matter.
```

The current-field term is an interaction. Treating `(j_T, A_T)` as a finite `2 x 2` kinetic matrix would mix a matter current with a gauge field and would depend on the matter action, external momentum, and renormalization prescription.

After integrating out `A_T`, the response is nonlocal and momentum-dependent:

```text
j_T K_T^-1 j_T.
```

That is not the master quadratic's constant two-root structure.

### 3. Two-U(1) P/D response matrix

The Structure-2 handoff used a two-U(1)-style matrix with roots arranged to match the master quadratic. That is a legitimate algebraic model of the roots.

But two facts matter:

1. The projected FTD action has not derived the P/D two-sector basis as physical electromagnetic response variables.
2. The Ward-valid Structure-2 scalar completion failed to reproduce the Structure-1 ppb closure for the tested natural scalar matter cases.

So the P/D matrix is not currently a completed physical bridge.

### 4. RG-step eigenvalues

FTD-0050 already tested the idea that the master quadratic is the characteristic polynomial of a natural engine RG step. That route is closed negative.

The key distinction is:

```text
RG-step eigenvalues       scaling dimensions, usually O(1)
master-quadratic roots    physical coupling candidates, about 137 and 3
```

So R3 should not revive the RG interpretation. If eigenvalues are used, they must be eigenvalues of a physical response matrix, not a blocking-flow matrix.

---

## What would be needed to revive R3

R3 becomes viable only if a future derivation supplies a physical two-sector response matrix before looking at the alpha residual.

The minimum acceptable statement would look like:

```text
FTD projection yields two response coordinates Phi_1, Phi_2.

Their quadratic action is:

S = 1/2 Phi^T K_FTD Phi

with:

tr(K_FTD)  = 16 G*^2
det(K_FTD) = 16 G*^3.

The electromagnetic mode is the larger-normal-stiffness eigenmode:

K_EM = x_+.
```

That statement would still need matter content, regulator, counterterms, and a physical alpha observable. But it would pass the first R3 gate.

The current projected bridge does not provide this.

---

## Consequence for alpha

After closing R1 and R3 negative under the current action, the remaining options are narrower:

```text
R2. Derive e0^2 = 1/x_+ from source-current normalization.
R4. Decide x_+ is arithmetic-only, not EFT charge normalization.
```

R3 stays on the shelf as a future route only if a genuine two-sector response matrix is derived from FTD structure.

For now, eigenvalues should be described as:

```text
natural algebraic language for the master quadratic,
not a required feature of the projected EFT.
```

`DERIV_SOURCE_CURRENT_NORMALIZATION_XPLUS_ATTEMPT.md` later tests R2 and closes it negative under the current projected action, leaving R4 as the current endpoint.

---

## Claim impact

| Claim | Status after this attempt |
|---|---|
| Eigenvalues are needed for a single coupling coefficient | False |
| Eigenvalues are relevant for coupled normal-mode response | True |
| Master quadratic can be represented as a `2 x 2` characteristic polynomial | Algebraically true |
| Current projected FTD action derives that response matrix | Closed negative |
| Structure-2 P/D matrix completes the bridge | Not established |
| R3 physical eigenvalue route | Closed negative under current action; future matching rule only |
| Source-current normalization route | Later closed negative under current action |
| Arithmetic-only endpoint | Current projected-action endpoint |

---

## Bottom line

We do not need eigenvalues unless FTD derives a coupled response matrix.

The master quadratic says:

```text
there exist two algebraic roots x_+ and x_-.
```

It does not by itself say:

```text
physical electromagnetism measures the larger eigenvalue of a derived FTD kinetic matrix.
```

That second sentence is the missing bridge. Under the current projected action, it is not derived.
