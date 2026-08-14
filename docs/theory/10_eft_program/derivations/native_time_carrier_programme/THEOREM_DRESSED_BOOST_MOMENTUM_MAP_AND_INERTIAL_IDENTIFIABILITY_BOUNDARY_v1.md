# FTD-0893 — dressed-boost momentum map and inertial-identifiability boundary

**Status:** `[THEOREM — CONDITIONAL DRESSED INERTIAL TENSOR]` +
`[CLOSED NEGATIVE — ENERGY HESSIAN ALONE DOES NOT IDENTIFY MASS]` +
`[BOUNDARY — TOTAL FIELD-MATTER MOMENTUM MAP REMAINS OPEN]` +
`[IMPOSED — TWO-CHANNEL REFERENCE REALIZATION]`  
**Date:** 2026-08-11  
**Certificate:** `57/57` exact checks, first locked execution  
**Production impact:** none

## Result

There is a precise answer to the question “does the energy of a stable dressed
object determine its inertia?” The answer is no unless the theory also says
what total physical momentum is.

Let `y` be the complete time-odd tangent state near a stable rest solution. If

```text
H(y) = E0 + (1/2) y^T A y + O(|y|^3)
```

with `A` symmetric positive definite, and if an independently defined additive
physical total-momentum map has linearization

```text
P(y) = B y + O(|y|^2),
```

where `B` has rank three, then the minimum-energy state at fixed small `P` is

```text
y*(P) = A^-1 B^T (B A^-1 B^T)^-1 P.
```

Its effective energy is

```text
E_eff(P) = E0 + (1/2) P^T (B A^-1 B^T)^-1 P + O(|P|^3).
```

Therefore the dressed inertial tensor is

```text
M = B A^-1 B^T.                                      (1)
```

Equation (1) is exact at quadratic order and is the strongest current local
mass theorem. It also establishes the missing ingredient sharply: `A` is not
enough. The momentum map `B` is part of the physical definition of inertia.

## Proof

Minimize

```text
(1/2) y^T A y
```

subject to `B y=P`. With multiplier `lambda`, stationarity gives

```text
A y = B^T lambda.
```

Since `A` is invertible,

```text
y = A^-1 B^T lambda.
```

The constraint then gives

```text
B A^-1 B^T lambda = P.
```

Rank three of `B` and positive definiteness of `A` make
`B A^-1 B^T` positive definite. Solving for `lambda` yields the displayed
minimizer. Substitution into the energy gives (1). Any fixed-momentum
perturbation lies in `ker B`; its cross term with `y*` vanishes, and its
remaining energy is positive. The stationary point is therefore the unique
constrained minimum.

## What the formula means

The odd tangent space is where a moving object's response lives. A field coat
contributes to inertia only if its time-odd degrees of freedom are included in
`y` and participate through the energy matrix `A`, the physical momentum map
`B`, or both. A static field-energy offset is not enough.

This distinguishes three objects that were previously easy to blur:

1. `E0`: rest energy of the static configuration;
2. `A`: energetic cost of time-odd motion near rest;
3. `B`: conversion from those odd amplitudes to physical total momentum.

Only the pair `(A,B)` fixes the inertial tensor.

## Exact matter--field reference

For one axis, take a matter-like odd amplitude `p` and a field-like odd
amplitude `f`:

```text
A = [ a  g ],       P = b_m p + b_f f,
    [ g  k ]
```

with `a>0`, `k>0`, and `a k-g^2>0`. Equation (1) becomes

```text
M = (k b_m^2 - 2 g b_m b_f + a b_f^2)/(a k-g^2).     (2)
```

The minimum-energy allocation is

```text
[p*]          [b_m]
[f*] = A^-1   [b_f] P/M,
```

and `E_min-E0=P^2/(2M)`. Replicating the same block on all three axes gives
`M I_3`, invariant under signed cubic permutations. This is an
`[IMPOSED reference realization]`; its coefficients are not derived from the
production substrate.

The uncoupled matter control `g=0`, `b=(1,0)`, `a=1/m` returns `M=m`. With an
independent field channel `g=0`, `b=(1,1)`, `a=1/m`, and `k=1/m_f`, equation
(2) gives `M=m+m_f`. This is the minimal mathematical sense in which a moving
field coat can carry part of a composite object's inertia.

## Identifiability no-go

Keep `A` fixed and replace `B` by `sB`. Then

```text
M -> s^2 M.
```

Thus the same rest configuration, rest energy, static Hessian, and complete
odd energy Hessian permit different inertias under different physical
momentum normalizations. An arbitrarily chosen moving path does not repair the
problem: if its parameter is rescaled, the apparent energy curvature rescales
too. The physical momentum constraint is what removes this ambiguity.

A static contribution `U0` also drops out of every derivative with respect to
`P`; it cannot be counted as inertial mass merely because it contributes to
rest energy. It must participate dynamically in the odd tangent sector and
the total-momentum ledger.

## Status of the selected common action

The current corpus supplies several necessary pieces:

- FTD-0892 supplies exact collective matter momentum inside the selected
  constituent canonical phase space;
- the connected common action supplies exact energy bookkeeping;
- FTD-0656 measures a co-moving energetic field dressing;
- the lattice supplies exact discrete translation covariance.

It does not yet supply the required exact total field--matter `B`:

- the natural spline-Poynting observer is source-free conserved on its locked
  wave control but fails the coupled recoil ledger (FTD-0619);
- an instantaneous matter boost of a static dressing is not a uniformly moving
  dressed orbit (FTD-0709);
- `Z^3` translation covariance does not automatically create a continuous
  additive `R^3` Noether generator.

Therefore the selected common action does not currently derive an absolute
physical dressed mass. The obstruction is no longer “we need more energy
data.” It is “we need an independently closed total-momentum map.”

## Isolated reference implementation

`ftd::eft::analyze_dressed_boost_momentum_map` evaluates equation (2), the
minimum-energy matter/field allocation, cubic covariance, the static-offset
control, and the `B -> sB` ambiguity. It fails closed unless `A` is positive
definite and the momentum row is nonzero.

Its public result denies all unearned promotions: it does not claim that the
momentum map, absolute mass, common-action Noether closure, stable pole, or
production coupling has been derived, and it reads neither Born targets nor a
native `G*` clock.

## Next physical discriminator

The next constructive front is to derive one of:

1. a local substrate stress/momentum state with an exact update and exchange
   law; or
2. an exact operational quasimomentum/hop ledger that is additive on the
   admissible dressed sector.

The candidate must then recover the same inertial tensor from all three routes:

```text
constrained energy curvature
= impulse / center velocity
= matter-field momentum partition.
```

Any disagreement closes the candidate negative. Co-moving pictures or a good
fit to a selected dispersion do not substitute for this cross-route identity.

## Scope firewall

```text
DRESSED_MASS_FORMULA=EXACT_CONDITIONAL_ON_A_AND_B
ENERGY_HESSIAN_ALONE_IDENTIFIES_MASS=FALSE
STATIC_REST_OFFSET_CONTRIBUTES_TO_INERTIA=FALSE
TOTAL_FIELD_MATTER_MOMENTUM_MAP=OPEN
ABSOLUTE_MASS_SCALE=NOT_DERIVED
STABLE_MATTER_POLE=OPEN
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_VECTOR_TYPE=TRUE
GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

The exact certificate is
`scripts/proofs/proof_dressed_boost_momentum_map_inertial_identifiability.py`.
The locked protocol is
`PREREG_DRESSED_BOOST_MOMENTUM_MAP_INERTIAL_IDENTIFIABILITY_BOUNDARY_v1.md`.
