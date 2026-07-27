# DERIVATION — Quadratic-coat orbit gather and commuting curl

**Identifier:** `FTD-0550`  
**Status:** `[SELECTION — QUADRATIC COAT] + [THEOREM — ELECTRIC ADJOINT AND SPLINE-CURL COMMUTATION] + [NUMERICAL FACT — LOCKED CAMPAIGN]`  
**Scope:** observer-only representation and gather; no production force or
mobile law is installed.

## 1. Staggered reconstruction forced by the coat current

Let `B2` be the centered quadratic B-spline and `B1` the centered hat. The
FTD-0541 charge coat is the tensor product of three `B2` factors, while its
oriented x-face current uses `B1` in x and `B2` transversely. Therefore the
face-field reconstruction adjoint to that current is

```text
E_x(x)=sum_ijk E_x[i,j,k]
 B1(x-i-1/2) B2(y-j) B2(z-k),                    (1)
```

with cyclic formulas for y and z. For the straight orbit
`x(tau)=x0+tau d`, define

```text
F_E=q integral_0^1 E(x(tau)) d tau.              (2)
```

The deposited current coefficient is

```text
K_x[i,j,k]=q d_x integral_0^1
 B1(x-i-1/2) B2(y-j) B2(z-k) d tau.              (3)
```

Taking the Euclidean face pairing and interchanging the finite sum and
integral gives the exact action-adjoint identity

```text
<E,K>=d dot F_E.                                 (4)
```

Unlike the old componentwise quotient by `d_x`, equation (2) remains defined
when an axial orbit has two zero displacement components. The transverse
electric force is consequently determined by the same interpolated field.

## 2. Edge reconstruction and exact commuting curl

Use the dual edge reconstruction

```text
B_x(x)=sum_ijk B_x[i,j,k]
 B2(x-i) B1(y-j-1/2) B1(z-k-1/2),                (5)
```

again cyclically. The one-dimensional spline derivative identity is

```text
B2'(u)=B1(u+1/2)-B1(u-1/2).                      (6)
```

For example, differentiating the face reconstruction gives
`(curl A)_x=partial_y A_z-partial_z A_y`. Using (6) and shifting the finite
periodic sums, the coefficient multiplying the x-edge basis in (5) is

```text
A_z[i,j+1,k]-A_z[i,j,k]
-A_y[i,j,k+1]+A_y[i,j,k],                        (7)
```

which is exactly the x component of the engine's matched `C^T A`. Cyclic
permutation proves

```text
interpolate_edge(C^T A)=curl(interpolate_face(A)). (8)
```

Thus the edge gather is not an independent midpoint prescription. It belongs
to the same staggered spline/de Rham complex as the face connection.

## 3. Magnetic work

With the orbit average `Bbar=int B(x(tau))d tau` and the relativistic
discrete-gradient velocity `vbar`, define

```text
Delta p_B=h beta q vbar cross Bbar.               (9)
```

Antisymmetry of the cross product gives `vbar dot Delta p_B=0` identically.
The orbit is admitted only when `d=h vbar` and `|vbar|<=C_SPEED`.

## 4. Boundary

The exact adjoint, commuting curl, and zero-work identities repair the old
trilinear gather's representation and magnetic-origin gaps. They do not solve
the nonlinear simultaneous endpoint, trajectory, current, and Maxwell
equations. FTD-0549 still requires those within-tick stages to be solved
atomically. No production toggle or scenario is licensed by this derivation.
