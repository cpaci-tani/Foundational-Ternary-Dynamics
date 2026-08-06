# DERIVATION — Quadratic-coat lattice self-force

**Identifier:** `FTD-0552`  
**Status:** `[DERIVED — SELF-ENERGY MECHANISM] + [NUMERICAL FACT — GENERIC SELF-FORCE] + [CLOSED NEGATIVE — UNSUBTRACTED ISOLATED MOBILE LAW]`

## 1. Periodic self-energy

For a quadratic coat centered at continuous subcell coordinate `r`, define
the neutral periodic source

```text
rho_r=q W_r-q/L^3.                                (1)
```

Let `D` be matched face divergence. The minimum-energy longitudinal field is

```text
E_r=D^T(DD^T)^+ rho_r,                            (2)
```

up to the sign convention already fixed by `D E=rho_r`. Its selected physical
energy is

```text
U_self(r)=g/2 ||E_r||^2.                          (3)
```

The lattice is invariant under integer translations and cubic rotations, but
not under arbitrary continuous shifts of `r`. Therefore (3) is only
lattice-periodic; no identity forces it to be constant inside a cell.

## 2. Why exact energy conservation does not remove self-force

For an infinitesimal coat displacement, FTD-0550 gives

```text
dU_field=-g <E,K>=-g d dot F_E.                   (4)
```

FTD-0551 transfers the opposite amount to matter. Consequently the gathered
self-field drives the particle along the gradient of its own lattice-periodic
self-energy:

```text
F_self(r)=-grad_r U_self(r)                       (5)
```

with orientation determined by the work convention. Exact total energy makes
this force conservative; it does not make it vanish.

At integer and half-cell symmetry planes, reflection forces the relevant
components of (5) to zero. A generic point has no such stabilizer. Static
success at those special coordinates therefore demonstrates pinning extrema,
not continuous translation invariance.

## 3. Campaign consequence

The locked campaign finds all eight integer/half-cell arms static but all four
generic-subcell arms mobile. After 64 ticks the largest displacement is
`0.8464862214540756` cells and the largest momentum is
`0.010170940522624974`, while accumulated total-energy error remains
`2.78e-17`.

The unmodified FTD-0551 map is therefore closed negative as an isolated
mobile-particle law. Removing the self-field, moving a resolved neutral
composite instead of a bare polarity, or accepting a physical Peierls
potential are distinct model choices. None is implied by the one-step
transaction and none is authorized by this result.
