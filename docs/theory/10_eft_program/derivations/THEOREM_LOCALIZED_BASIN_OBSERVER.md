# THEOREM — Symmetry-aware localized-basin observer

**Identifier:** `FTD-0677`  
**Status:** `[THEOREM — EXACT SELECTED OBSERVER]`  
**Scope:** labelled connected-composite states with the same graph and periodic
matched face/edge fields; observer only

**Implementation correction (FTD-0680):** the first qualification decoded the
flat matched-field index as z-major instead of the engine's x-major order.  The
mathematical theorem was unchanged; the implementation and qualification now
use an asymmetric shell origin that detects any x/z reversal.

## Statement

Let `X_0` and `X_1` contain the same `N` labelled constituents, charge labels,
binding graph, width, orientation, periodic volume, and field representation.
Unwrap both constituent configurations in the same periodic chart.  For
positions `x_i`, momenta `p_i`, constituent mass `m>0`, and a selected reference
frequency `omega>0`, define

```text
D_x = m sum_i |(x_1i-xbar_1) - (x_0i-xbar_0)|^2,
D_p = (1/m) sum_i |(p_1i-pbar_1) - (p_0i-pbar_0)|^2,
D_phase = omega^2 D_x + D_p.                       (1)
```

Then `D_phase` is nonnegative, invariant under a common whole-object
translation and common momentum boost, and covariant under the signed cubic
group.  It vanishes exactly when the two labelled internal phase states agree
up to those collective offsets in the chosen periodic chart.

For control-relative field differences `delta E` and `delta B`, field scale
`beta>0`, and wave speed `c>0`, define the positive difference-field norm

```text
H_delta = (beta/2) sum_cells (|delta E|^2+c^2|delta B|^2). (2)
```

Partition storage cells by periodic Chebyshev radius about a selected origin:
`r<=R_in`, `R_in<r<=R_out`, and `r>R_out`.  The resulting near, intermediate,
and far terms are nonnegative, signed-cubic covariant, and sum exactly to (2).

## Proof

Subtracting the constituent means applies the orthogonal projector
`Q=I-(1/N)11^T` independently to each Cartesian coordinate.  Because `Q1=0`,
common translations and boosts vanish.  Positive weights `m`, `1/m`, and
`omega^2` prove nonnegativity.  A signed coordinate permutation is orthogonal,
so it preserves every squared Euclidean norm.  It also preserves the maximum
absolute coordinate, hence the periodic Chebyshev shell assignment.

Equation (2) is a sum of nonnegative cell terms.  The three shell predicates
are mutually exclusive and exhaustive; therefore their sum is exactly
`H_delta`.  The exact-rational certificate checks all 48 signed coordinate
permutations, common translation and boost quotienting, a nonzero phase-space
witness, and an exact three-shell field witness.  The C++ qualification checks
the same identities, periodic integer translation covariance, topology
rejection, and a full field-component cyclic rotation.

## Locked interpretation

The observer supplies an operational test for one necessary feature of
matter: persistence or return of an internal relational state while field
disturbance can occupy remote cells.  It deliberately distinguishes:

- collective position and momentum, which are reported but not counted as
  internal deformation;
- internal constituent phase distance;
- spatial distribution of control-relative field disturbance.

The selected `omega`, shell origin, and shell radii are observer scales.  They
must be preregistered before a campaign and cannot be tuned after inspecting a
trajectory.

## Boundary of the theorem

- `D_phase` is a distance diagnostic, not the exact nonlinear action or energy.
- `H_delta` is a positive difference-field self norm.  In a dressed background
  it is not the complete excited-minus-control field energy because the
  control/difference interference term is separate.
- “Near” means only within the selected shell.  It does not mean bound,
  co-moving, or ontically part of matter.
- Constituent labels and graph orientation are fixed.  The observer does not
  minimize over relabellings, graph automorphisms, or arbitrary rotations.
- Periodic unwrapping is local to the first labelled constituent; configurations
  spanning a periodic cut outside one consistent chart are outside scope.
- The theorem proves no attracting basin, asymptotic return, particle pole,
  radiation law, or production ontology.

No production state, tick phase, force, toggle, scenario, default, or postulate
is changed.
