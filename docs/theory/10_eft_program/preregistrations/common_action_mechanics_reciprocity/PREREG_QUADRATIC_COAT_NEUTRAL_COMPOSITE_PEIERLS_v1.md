# PRE-REGISTRATION — Quadratic-coat neutral-composite Peierls gate

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0553`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0501`, `FTD-0550`, `FTD-0551`, `FTD-0552`  
**Scope:** observer-only theorem/campaign for rigid neutral composites built
from existing signed ternary constituents. No production state, force, phase,
toggle, default, scenario, self-field subtraction, or new primitive variable.

## 1. Locked carrier class

Let a candidate rigid composite be a finite list of distinct integer offsets
`n_a` and polarities `q_a in {-1,+1}` satisfying

```text
sum_a q_a = 0.                                    (1)
```

All constituents retain their individual worldlines as required by FTD-0501.
For a common subcell translation `f` along coordinate axis `i`, deposit

```text
rho_f(n)=sum_a q_a W(n-R-n_a-f e_i),              (2)
```

where `W` is the FTD-0541 tensor quadratic B-spline coat. Constituent offsets
and relative separation are fixed. Coincident opposite polarities, repeated
primitive sites, self-field subtraction, nonlocal interpolation, and a fitted
binding force are excluded.

## 2. Locked spectral theorem

For `-1/2 <= f <= 1/2`, the one-dimensional coat has weights

```text
w_- = (1/2)(1/2-f)^2,
w_0 = 3/4-f^2,
w_+ = (1/2)(1/2+f)^2.                             (3)
```

With `k=2 pi m/L`, define

```text
b_0(k)=(3+cos k)/4,
A(k)=sum_a q_a exp(-i k dot n_a),
lambda(k)=2 sum_j (1-cos k_j).                    (4)
```

The locked algebraic identity is

```text
|w_- exp(+ik)+w_0+w_+ exp(-ik)|^2
=b_0(k)^2+(1-cos k)^2(f^4-f^2/2).                (5)
```

For the minimum-energy periodic face field `D E_f=rho_f`, the physical field
energy `U(f)=beta ||E_f||^2/2` must therefore obey

```text
U(f)=U(0)+C_i(f^4-f^2/2),                         (6)

C_i=beta/(2L^3) sum_{k != 0}
    |A(k)|^2 (1-cos k_i)^2
    prod_{j != i} b_0(k_j)^2 / lambda(k).         (7)
```

Every term in (7) is nonnegative. Since `b_0(k)>0`, `C_i=0` exactly when the
primitive integer-offset source is invariant along axis `i`. A nonzero
localized composite cannot be invariant along every tested translation axis.
For every axis with `C_i>0`, the exact barrier is

```text
U(0)-U(1/2)=C_i/16,                               (8)
```

and `dU/df=C_i f(4f^2-1)`. Integer centers are maxima and half-cell centers
are minima along that axis.

## 3. Locked work identity

For a rigid straight translation from `f_0` to `f_1`, sum the exact
constituent face currents into `K`. Let `E_0,E_1` be the minimum-energy
longitudinal fields for the two endpoint densities and
`Ebar=(E_0+E_1)/2`. Exact continuity and longitudinal orthogonality imply

```text
U(f_1)-U(f_0)=-beta <Ebar,K>.                     (9)
```

The FTD-0550 adjoint gather therefore gives a nonzero integrated center-of-mass
self-force whenever (6) changes. Exact energy conservation makes this force
conservative; neutrality does not remove it.

## 4. Locked finite campaign

Use `L=17,33`, axes `x,y,z`, both global polarity mirrors, integer translations,
and these non-fitted localized structures:

```text
D1: + at (0,0,0), - at (1,0,0)
D3: + at (0,0,0), - at (3,0,0)
DB: + at (0,0,0), - at (1,1,1)
Q4: + at (-1,0,0),(1,0,0); - at (0,-1,0),(0,1,0). (10)
```

Evaluate `f in {0,1/8,1/4,3/8,1/2}`. Compute `U(0)` and `C_i` directly from
the finite spectral sums, and independently compute deposited-density Poisson
energies. For work, use the registered segments
`0->1/16`, `1/8->3/16`, `1/4->5/16`, and `3/8->7/16`.

Required gates:

- neutrality, coat partition, first moment, and exact aggregate continuity
  residuals at or below `1e-12`;
- Poisson residual below `1e-13`;
- spectral-versus-Poisson energy, quartic-law, and exact work residuals below
  `1e-12`;
- polarity, integer-translation, and cubic-rotation residuals below `1e-12`;
- every registered non-invariant arm has `C_i>0` and barrier `C_i/16>1e-12`.

No tolerance or geometry changes are permitted after execution.

## 5. Locked verdicts

- all identities close and all localized non-invariant arms have `C_i>0`:
  `RIGID_NEUTRAL_COMPOSITE_PEIERLS_OBSTRUCTION`;
- an admissible nonzero localized structure has `C_i=0` on every axis while
  the algebra closes: `NEUTRAL_COMPOSITE_CANCELLATION_WITNESS`;
- any theorem, Poisson, current, work, or covariance identity misses its gate:
  `NEUTRAL_COMPOSITE_OBSERVER_INVALID`.

The first verdict closes only the rigid integer-offset neutral-composite cure
for the compact quadratic coat. It does not close an internally deforming
carrier, a noncompact band-limited representation, or a new binding variable.
Those are separate ontology/dynamics choices and receive no automatic license.

## 6. Run disposition

Run 2026-07-26 on the pinned MSVC CPU observer. All 96 arms satisfy the
spectral, Poisson, current, work, polarity, translation, and cubic gates.
Every registered localized arm has a strictly positive coefficient; the
smallest barrier is `7.3632527388345549e-05`. The locked verdict is
`RIGID_NEUTRAL_COMPOSITE_PEIERLS_OBSTRUCTION`. See
[`AUDIT_QUADRATIC_COAT_COMPOSITE_PEIERLS.md`](../../../07_assessment/common_action_mechanics_reciprocity/AUDIT_QUADRATIC_COAT_COMPOSITE_PEIERLS.md).
