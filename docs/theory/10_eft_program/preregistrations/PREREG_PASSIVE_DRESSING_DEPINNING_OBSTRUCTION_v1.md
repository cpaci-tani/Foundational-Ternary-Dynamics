# Pre-registration — Passive dressing depinning obstruction (FTD-0581)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date:** 2026-07-26  
**Parents:** FTD-0574, FTD-0575, FTD-0578, FTD-0579, FTD-0580.  
**Production changes permitted:** none. Observer code, exact proof, test,
theorem, audit, and documentation reconciliation only.

## 1. Question

FTD-0580 derived the endpoint-chord Peierls potential

```text
V_d(r)=V_d(0)+C_d r(1-r),       0 <= r <= 1,            (1)
```

with `C_d>0` for every nonzero Moore direction. This campaign asks three
questions without changing the production ontology:

1. what momentum and speed are required to cross the barrier under the
   production dispersion;
2. whether a stable passive deformation of the existing `(J,W)` dressing can
   remove the barrier; and
3. what minimum energy and regularity price an active internal mode must pay
   before it can even be a candidate traversal mechanism.

No fitted counterforce, self-field deletion, damping, external drive, force
toggle, scenario, hidden route variable, or new primitive is permitted.

## 2. Exact relativistic depinning threshold

Use the production momentum convention

```text
H(p)=sqrt(E_REST^2+C_SPEED^2 |p|^2),
K(p)=H(p)-E_REST.                                      (2)
```

The half-cell barrier is `Delta_d=C_d/4`. A carrier prepared in the static
ground dressing at an integer site can reach the half-cell saddle only if

```text
K(p_0) >= Delta_d.                                     (3)
```

The threshold is therefore

```text
p_dep(d)=sqrt(2 E_REST Delta_d+Delta_d^2)/C_SPEED,
v_dep(d)=C_SPEED^2 p_dep(d)/(E_REST+Delta_d).           (4)
```

Equation (4), its inverse dispersion identity, strict positivity, polarity
evenness, and cubic covariance are theorem targets.

## 3. Passive-dressing theorem

For a fixed subcell coordinate `r`, let `z_*(r)` be the stationary native
linear-field solution obtained by eliminating the FTD-0574/0575 quadratic
Hodge field. The exact completed-square form is

```text
E_field+source(r,z)
  = V_d(r)+1/2 <z-z_*(r), K [z-z_*(r)]>,                (5)
```

on the gauge-fixed physical subspace, with `K` positive semidefinite. Hence

```text
E_field+source(r,z) >= V_d(r).                          (6)
```

No passive deformation can lower the pointwise effective potential or cancel
`Delta_d`. Dynamic lag raises the instantaneous energy above the relaxed
curve; it does not flatten the relaxed curve.

The same obstruction has a local regularity form. Periodically continue (1)
between integer sites. Near an integer minimum,

```text
V_d(r)-V_d(0)=C_d |r|+O(r^2).                           (7)
```

For a stable passive internal equilibrium `z_0`, a locally Lipschitz response
`z(r)-z_0=O(|r|)` in a positive quadratic energy obeys

```text
U(z(r))-U(z_0)=O(r^2).                                  (8)
```

It cannot cancel the nonzero `O(|r|)` cusp. Cancellation requires at least
one registered escape condition: a nonstationary excited state, a
non-Lipschitz response, a zero/negative-energy direction, a noncompact limit
with `C_d -> 0`, or an explicit counterterm.

## 4. Active-reservoir lower bound

Let `epsilon_0 >= 0` be internal excitation above the relaxed dressing at the
integer site. Energy conservation gives the necessary traversal condition

```text
K(p_0)+epsilon_0 >= Delta_d.                            (9)
```

At zero external momentum, any active traversal candidate therefore needs

```text
epsilon_0 >= Delta_d > 0.                               (10)
```

For a one-mode positive oscillator `U=(P^2+omega^2 Q^2)/2`, the explicit
energy-budget path

```text
U(r)=epsilon_0-C_d r(1-r)                               (11)
```

is real only when `epsilon_0 >= Delta_d`. At equality it reaches the oscillator
ground state at `r=1/2` and `Q(r)` is proportional to `|r-1/2|`: continuous
and Lipschitz, but not differentiable at the half cell.
For `epsilon_0>Delta_d` the path is smooth but begins and ends at a finite
excited state. Equation (11) is only a constructive energy budget; it is not
an equation-of-motion or common-action derivation.

Consequently a native active escape, if it exists, must exhibit a finite
internal excitation/phase that recurs across hops and must derive its coupling
from the frozen action. It is not a gapless ground-state translational mode.

## 5. Registered arms

Exact proof:

- derive (4) from (2)--(3) and prove `0<v_dep<C_SPEED`;
- complete the square in (5) and prove (6);
- prove the cusp/Lipschitz mismatch (7)--(8);
- prove the active lower bound (9)--(10) and the equality-case regularity of
  (11).

Compiled observer:

- `L in {17,33}`, both polarities, all 26 signed Moore directions: 104
  independently evaluated chord coefficients and depinning thresholds;
- verify threshold kinetic energy, inverse momentum, velocity, polarity, and
  proper-cubic covariance to `1e-12`;
- for each arm, four registered positive quadratic passive fixtures with
  analytic zero first derivative and nonnegative completed-square excess;
- for each arm, excitation ratios `epsilon_0/Delta_d in {1,2,4}` and
  `r in {0,1/8,...,1}`: 2,808 active energy-budget samples;
- equality arms must be real, touch zero energy only at the half cell, and be
  flagged nondifferentiable there; ratios 2 and 4 must remain positive and
  smooth;
- frozen production hash/default/toggle/scenario checks.

All algebraic residuals must be `<=1e-12`. Every `C_d`, `Delta_d`, `p_dep`,
and `v_dep` must be strictly positive. No passive fixture may show negative
completed-square excess or a nonzero linear cancellation coefficient.

## 6. Outcome map

If every registered gate passes, record

```text
PASSIVE_DRESSING_CANNOT_DEPIN_ACTIVE_TRAVERSAL_COSTS_FINITE_EXCITATION
```

This closes stable passive `(J,W)` deformation as the cure for the finite
chord Peierls barrier. It leaves one narrower live mechanism: a dynamically
phase-locked, finite-excitation native field configuration whose frozen action
actually transports and replenishes the required energy across repeated hops.
The algebraic budget alone does not establish that mechanism.

## 7. Frozen production provenance

```text
phase_read.cpp                  D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8
phase_write.cpp                 2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4
field_operators.h               25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48
native_energy_contract.h        3DB8F2DC573E7F4A87E17409878915E7B5A52CE1673713998C544516E0175621
```
