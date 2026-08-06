# DERIVATION — Quadratic coupling coat and exact face current

**Identifier:** `FTD-0541`  
**Status:** `[SELECTION — SMOOTH POSITIVE NON-CARDINAL COAT]` +
`[THEOREM — EXACT DISCRETE CONTINUITY]` +
`[NUMERICAL FACT — INTEGER-PLANE C1 ACTION RESPONSE]`  
**Inputs:** FTD-0540's priced escape branch and the existing site-plus-remainder
effective position. Primitive ternary manifestation is unchanged.

## 1. The coat is coupling, not fractional manifestation

For effective position `x` and polarity `q`, define

```text
rho_i(x)=q product_d B2(x_d-i_d),                    (1)
```

where `B2` is the centered quadratic B-spline. Equation (1) is a deterministic
coupling sidecar. The primitive state remains one `s in {-1,0,+1}` at its
anchor site; the 27 coefficients are not 27 fractional manifested particles.

The one-dimensional translate identities

```text
sum_i B2(x-i)=1,
sum_i i B2(x-i)=x                                    (2)
```

tensor to partition and first-moment reproduction in three dimensions. The
unsigned coefficients are nonnegative and `B2` is `C1`. At an integer
position, the three one-dimensional weights are `(1/8,3/4,1/8)`, so the full
coat has 27 positive sites. Its center and one-axis-neighbor weights are

```text
(3/4)^3=27/64,
(1/8)(3/4)^2=9/128.                                 (3)
```

This deliberately pays FTD-0540's non-cardinality price.

## 2. Matched current follows from the spline derivative

Let `B1(u)=max(1-|u|,0)`. The exact identity

```text
B2'(x-i)=B1(x-(i-1/2))-B1(x-(i+1/2))                (4)
```

is already in backward-divergence form. Along the straight segment
`x(t)=x_0+t Delta x`, differentiate (1):

```text
d rho_i/dt
=q sum_d Delta x_d B2'(x_d-i_d)
                 product_(e!=d) B2(x_e-i_e)
=-div J_i(t),                                        (5)
```

with positive-face current density

```text
J^d_(i+1/2)(t)=q Delta x_d B1(x_d-(i_d+1/2))
                     product_(e!=d) B2(x_e-i_e).     (6)
```

Integrating (5) over one slab proves

```text
rho_after-rho_before+div K=0,
K=integral_0^1 J(t) dt.                              (7)
```

Summing (6) over all faces and using both partitions in (2) gives the second
exact identity

```text
sum_faces_d K^d=q Delta x_d.                         (8)
```

There is no chosen x/y/z route: every component is integrated on the same
continuous straight segment.

## 3. Polynomial-exact implementation

The segment is split at every crossed half-integer coordinate plane. On each
piece, the longitudinal `B1` factor has degree one and the two transverse `B2`
factors have degree two, so the integrand degree is at most five. Three-point
Gauss-Legendre integration is exact for degree five. The floating
implementation is therefore an evaluation of the analytic piecewise
polynomial integral, not a convergence-tuned quadrature rule.

## 4. What regularity is gained

The FTD-0539 cusp occurred when an inactive endpoint coordinate lay on an
integer plane and the trilinear coupling used a transverse hat factor. In (6)
an inactive/transverse coordinate uses `B2`, which is differentiable there.
For the locked nontrivial face connection, the fourth-order derivatives are

```text
left                         0.004250000000714256
right                        0.004250000000638465
jump                         7.58e-14
same-side convergence        1.23e-12.               (9)
```

Thus the integer reflection-plane cusp is removed for this selected
representation. This does not prove global smoothness at every spline knot or
solve any particle Legendre equation.

## 5. Boundary of the result

FTD-0541 supplies an exact, local, positive, smooth-coat charge/current pair.
It does not inherit the closed FTD-0536 action: that action was derived for the
FTD-0478 trilinear worldline coupling. The required new common action was
subsequently derived from (6) in FTD-0542 rather than obtained by swapping
arrays inside the old solver.

Exact energy is still absent. FTD-0543 proves that a fixed-step discrete action
does not acquire an energy equation merely because its spatial interpolation
is smoother. No mobile toggle, force, scenario, dressing morphology, pole,
Lorentz, photon, or particle claim follows.
