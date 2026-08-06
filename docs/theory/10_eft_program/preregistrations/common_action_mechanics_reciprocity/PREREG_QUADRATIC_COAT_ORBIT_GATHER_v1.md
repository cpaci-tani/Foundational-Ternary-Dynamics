# PRE-REGISTRATION — Quadratic-coat orbit gather

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0550`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0479`, `FTD-0540`, `FTD-0541`, `FTD-0549`  
**Scope:** observer-only replacement of the old trilinear electric/magnetic
path gathers. Production state, current, force, tick, toggle, default, and
scenarios remain unchanged.

## 1. Locked compatible interpolants

For the quadratic coat, interpolate an oriented face field by

```text
E_x(x)=sum_ijk E_x[i,j,k]
  B1(x-i-1/2) B2(y-j) B2(z-k),                   (1)
```

and cyclic permutations. Interpolate an oriented edge field by

```text
B_x(x)=sum_ijk B_x[i,j,k]
  B2(x-i) B1(y-j-1/2) B1(z-k-1/2),               (2)
```

and cyclic permutations.

For a straight segment `x(tau)=x0+tau*d`, define

```text
F_E=q integral_0^1 E_interp(x(tau)) d tau,
Bbar=  integral_0^1 B_interp(x(tau)) d tau.       (3)
```

All integrals use 8-point Gauss-Legendre quadrature split at every
half-integer spline knot.

## 2. Locked exact gates

The electric gather must be the adjoint of the exact FTD-0541 current:

```text
d dot F_E = <E,K>.                                (4)
```

Require (4) below `5e-13`, including axial paths for which two displacement
components vanish. The transverse electric components must remain finite and
at least one registered axial arm must have a nonzero transverse gather above
`1e-8`; it may no longer be marked underdetermined by the representation.

For `B=C^T A`, require the spline commuting identity

```text
B_interp(x)=curl A_interp(x)                      (5)
```

below `5e-13` at generic non-knot path samples. This is the action-origin gate
for the edge interpolation.

For the relativistic discrete-gradient velocity `vbar`, define

```text
Delta p_B=h beta q vbar cross Bbar.               (6)
```

and require `vbar dot Delta p_B=0` below `5e-13`.

The registered orbit parameterization must also satisfy

```text
d=h vbar,  |vbar| <= C_SPEED,                     (7)
```

with kinematic and causal residuals below `5e-13`. This prevents a gather
from passing on a path that is unrelated to the velocity entering its
magnetic impulse.

Require polarity mirror, path reversal, integer translation, and cyclic cubic
rotation residuals below `5e-13`. Invalid sizes, charges, fields, and
overcausal segments fail closed.

## 3. Locked campaign and verdicts

Use `L=17`, both polarities, generic translated starts, axial and diagonal
segments, deterministic nonzero face and edge fields, and at least 72 arms.

- all adjoint, curl, zero-work, locality, reversal, translation, and rotation
  gates close:
  `QUADRATIC_COAT_ORBIT_GATHER_CONSTRUCTIVE`;
- electric adjoint closes but curl/action origin fails:
  `ELECTRIC_GATHER_CONSTRUCTIVE_MAGNETIC_ORIGIN_UNRESOLVED`;
- electric adjoint or covariance fails:
  `QUADRATIC_COAT_ORBIT_GATHER_CLOSED_NEGATIVE`.

A constructive result repairs only the representation/gather defects of
FTD-0479. It does not itself license a mobile toggle. The next registered gate
must insert these gathers into the implicit relativistic discrete-gradient
transaction and test self-consistent Gauss, energy, inverse, and multi-tick
motion.

## 4. Run disposition

Run 2026-07-26 on the pinned MSVC CPU observer: all 72 arms and every locked
gate passed. The registered verdict is
`QUADRATIC_COAT_ORBIT_GATHER_CONSTRUCTIVE`. See
[`AUDIT_QUADRATIC_COAT_ORBIT_GATHER.md`](../../07_assessment/AUDIT_QUADRATIC_COAT_ORBIT_GATHER.md).
