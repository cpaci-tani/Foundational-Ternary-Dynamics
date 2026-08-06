# PRE-REGISTRATION — Two-slab variational force v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0485`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0484`

## Question

Does the selected cubical connection action determine the full interior
particle impulse—including transverse electric and magnetic components—by a
two-slab discrete Euler--Lagrange variation, and is that impulse unique at the
existing `remainder=+/-1` movement threshold?

## Frozen evaluator

Use two adjacent reaction-free straight worldline slabs sharing `x_n`. Each
slab must remain within one spatial cell. Reconstruct the spatial connection
with the lowest Cartesian Nedelec one-form basis and the scalar potential with
the trilinear nodal basis. The slab action is exactly the `FTD-0484` action.

Because every within-cell integrand has degree at most three in normalized
time, evaluate it with the fixed two-point Gauss--Legendre rule, which is exact
for this polynomial class. Use three-component forward automatic
differentiation at `x_n`; finite-difference force estimates are forbidden.

The interaction impulse is

```text
I_n = D_2 S_int(x_(n-1),x_n) + D_1 S_int(x_n,x_(n+1)).
```

No field gather, division by a displacement component, `grad|J|`, Poisson
force, or explicit cross product may enter the implementation.

## Locked gates

At tolerance `1e-12`, require:

1. direct within-cell action equals the independent `FTD-0484` deposited
   action on both slabs;
2. arbitrary matched gauge transformations at all three time slices leave
   `I_n` invariant;
3. a nonzero pure-gauge connection produces zero `I_n`;
4. a stationary particle in a uniform time-varying transverse connection
   receives the exact nonzero electric impulse `g q lambda_t E`;
5. a uniform affine magnetic connection gives the exact curvature impulse
   `g q (Delta x cross B)` for equal adjacent displacements, without reading
   or gathering `B`;
6. polarity reversal reverses the impulse, and proper cubic rotation rotates
   it;
7. the source contains no call to the legacy interpolation or force helpers.

## Threshold gate

Use an allowed connection whose normal electric component differs in the two
cells adjacent to an integer plane. Evaluate stationary two-slab impulses at
`x=m-epsilon` and `x=m+epsilon`, with `epsilon=1e-8`. Production uniqueness
requires the left/right difference to be below `1e-12`.

If the interior gates pass but the threshold gate fails, record
`INTERIOR_VARIATIONAL_FORCE_DERIVED_THRESHOLD_NONUNIQUE`. That outcome closes
production integration for the frozen compact Q1/Nedelec shape: a one-sided
rule, subgradient, or smoother wider-support shape would be an additional
selection and is not authorized here.

## Normalization statement

The campaign uses a symbolic interaction coupling `g`. It separately records
the algebraic consequence of completing the matched field action: exact
source normalization forces the magnetic curvature term to appear as
`v cross B/C_SPEED`. It does not silently identify this with the frozen
`FTD-0479` magnetic gather.

No production toggle, scenario, reaction rule, or Lorentz claim follows from
either outcome.

Run-of-record test-source SHA256:
`1194C284A838ECB01F5940FAD4280558E4DBDF039F8C78A93E6CDA0614443948`.
