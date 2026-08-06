# FTD-0614 — Refined-core Peierls landscape and covariance v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`
**Scope:** observer-only energy-landscape and whole-state covariance test
**Production change:** forbidden

## 1. Frozen input

Reconstruct only the FTD-0612 refined uniformly neutralized charged-trimer
rest state and require its energy fingerprint
`0.0015517955076684577` within `1e-15`, gradient at most `1e-10`, nine
positive modes, and the complete 64-tick rest/inverse gate.  Keep the same
three explicit ternary-polarity constituents, uniform `-1/L^3` compensator,
quadratic coupling coat, selected quartic binding, matched face/edge field,
production dispersion, shared-anchor fibre, and common-action transaction.

No constituent, charge, binding coefficient, neutralizer, force, field scale,
or launch phase may be changed.

## 2. Registered translation paths

For each signed Cartesian direction `+x,-x,+y,-y,+z,-z`, translate the
refined centre from `q=0` to the exactly equivalent integer translate at
`q=1`.  Evaluate `q=j/64`, `j=0,...,64`.

Record two curves:

1. **rigid:** hold the refined orientation and in-plane strain fixed;
2. **locally relaxed:** at fixed centre and fixed constituents, minimize only
   the three orientation and three in-plane-strain coordinates.

The relaxed scan uses deterministic six-dimensional Nelder--Mead continuation
in both directions.  Each image starts from both the refined rest shape and
the previous accepted image; retain the lower admissible result.  Each
simplex has coordinate increments `0.02` for rotations and `0.01` for strain,
at most `1,500` evaluations, and terminates only when parameter diameter is at
most `1e-8` and energy spread is at most `1e-14`.  The registered strain basin
and pair-distance admissibility conditions are unchanged.  Forward and
backward relaxed energies must agree within `1e-9`.

For each curve define the sampled barrier

```text
Delta_path = max_j E(j/64) - E(0).
```

Endpoint energy periodicity must close within `1e-12`; the relaxed curve may
not exceed the rigid curve by more than `1e-12`; every reported barrier must
be finite and nonnegative.  A positive sampled barrier is a property of the
registered path family, not a proof that every path in configuration space
is obstructed.

## 3. Composite energy threshold

All three constituents receive one common speed.  With

```text
H(p)=sqrt(E_REST^2+C_SPEED^2 |p|^2),
K_3(v)=3[H(p(v))-E_REST],
```

define the registered selected-path threshold by `K_3(v_path)=Delta_path`:

```text
delta = Delta_path/3,
p_path = sqrt(2 E_REST delta + delta^2)/C_SPEED,
v_path = C_SPEED^2 p_path/(E_REST+delta).
```

Verify the inverse dispersion identity to `1e-12`.  Compare these energy
budgets with the already locked FTD-0613 launches at `1/128`, `1/64`, and
`1/32`.  A mobile arm below the corresponding locally relaxed selected-path
barrier is a contradiction.  A pinned arm above it is allowed because field
and internal dynamical overhead can exceed a static path barrier.

## 4. Correct cubic-covariance comparator

Use the two nontrivial cyclic proper-cubic rotations

```text
R(x,y,z)=(y,z,x),   R^2(x,y,z)=(z,x,y).
```

For every base `x` landscape, rotate the complete rest configuration and its
translation direction together.  Compare rigid and locally relaxed energies
image by image.  Maximum energy residual must be at most `1e-12`.

Also perform one forward and one state-only inverse common-action tick at
speeds `1/64` and `1/32`, both signs, for the base state and both rotated
copies.  Rotate positions, momenta, electric face flux, magnetic edge field,
and launch together.  Every common-action gate must be at most `1e-12`, and
the rotated later-state and inverse-recovery residuals must be at most
`1e-10`.  This is the correct comparator replacing FTD-0613's fixed-body
cross-axis equality demand.

## 5. Verdicts

- `REFINED_CORE_SELECTED_PATH_BARRIER_AND_COVARIANCE_RESOLVED`: all coverage,
  periodicity, relaxation, threshold, and whole-state covariance gates pass,
  and every locally relaxed sampled barrier is greater than `1e-12`;
- `REFINED_CORE_REGISTERED_PASSIVE_PATH_GAPLESS`: all gates pass and at least
  one locally relaxed sampled barrier is at most `1e-12`;
- `REFINED_CORE_PEIERLS_LANDSCAPE_NUMERICALLY_UNRESOLVED`: any reconstruction,
  optimization, energy, threshold, covariance, or record gate is incomplete.

A positive result licenses only a finite selected-path Peierls barrier for
this selected compact constituent model.  It directs the next test toward a
phase-carrying active internal mode or a genuinely extended low-momentum
carrier.  It does not derive a physical particle, exclude all deforming
paths, or change production.

**Protocol lock:** `protocol_sha256=D409501414737F70D884A553CA05E86200EA42876854FCFD834BE04581493D82`
