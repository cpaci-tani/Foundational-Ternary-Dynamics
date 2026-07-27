# Theorem — Full-Surface Finite-Source Obstruction

**Record:** FTD-0562
**Status:** [THEOREM — FIXED FINITE RIGID LINEAR SOURCE SLOW-HOP NO-GO] + [OPEN — NONLINEAR/DEFORMING CARRIER]
**Scope:** production `FULL`-stencil linear field operator; any fixed nonzero
finite-support source profile translated rigidly by one coordinate-site hop
every `T` ticks; `l=1` slow-hop branch

## 1. Full-direction resonance branch

Let the hop use axis `a`, let `n` be any unit vector, and set

\[
 \mathbf k=\frac{r\mathbf n}{T},\qquad
 \Omega=\frac{2\pi+k_a}{T}.
\]

The exact driven denominator is

\[
 D_T(r,\mathbf n)=C_{\rm WAVE}^2M(\mathbf k)
 -4\sin^2(\Omega/2).
\]

The production symbol has the uniform small-momentum expansion

\[
 M(\mathbf k)=|\mathbf k|^2-rac{|mathbf k|^4}{12}
 +O(|\mathbf k|^6).
\]

Consequently, uniformly for `n` on the sphere and `r` in a compact
neighborhood of the positive root,

\[
 T^2D_T=
 c^2r^2-4\pi^2-rac{4\pi r n_a}{T}+O(T^{-2}),
 \qquad c^2=C_{\rm WAVE}^2=1/3.
\]

The limiting equation has the simple positive root

\[
 r_0=\frac{2\pi}{c}=2\pi\sqrt3,
 \qquad \partial_r(c^2r^2-4\pi^2)|_{r_0}=2c^2r_0>0.
\]

The implicit-function theorem therefore supplies a regular root for every
direction and every sufficiently large integer `T`.  Substitution of
`r_T=r_0+a/T+O(T^-2)` gives

\[
 \boxed{r_T=r_0+\frac{6\pi n_a}{T}+O(T^{-2}).}
\]

This is a two-dimensional family of oblique resonances, not the axial line
used in FTD-0561.

## 2. General finite source

For any fixed finite profile `rho_x`, define its exact form factor

\[
 S(\mathbf k)=\sum_{\mathbf x}\rho_{\mathbf x}
 e^{i\mathbf k\cdot\mathbf x}.
\]

It is a finite trigonometric polynomial and hence real analytic.  If the
profile is nonzero, its Taylor series at the origin has a first nonzero
homogeneous term of some finite total degree `m`:

\[
 S(\mathbf k)=P_m(\mathbf k)+O(|\mathbf k|^{m+1}),
\]

\[
 P_m(\mathbf n)=\frac{i^m}{m!}
 \sum_{\mathbf x}\rho_{\mathbf x}(\mathbf n\cdot\mathbf x)^m.
\]

Because `P_m` is a nonzero homogeneous polynomial, it cannot vanish on an
open subset of the unit sphere.  In particular the set

\[
 U=\{\mathbf n\in S^2:n_aP_m(\mathbf n)\ne0\}
\]

is nonempty and open.

## 3. On-shell numerator

The periodic-hop Floquet coefficient is

\[
 c_1=\frac{1-e^{ik_a}}
 {T[1-e^{i(k_a+2\pi)/T}]}.
\]

On the branch above,

\[
 Tc_1\longrightarrow\sqrt3,n_a,
 \qquad T\mathbf q(\mathbf k_T)\longrightarrow r_0\mathbf n,
\]

where `q=(sin k_x,sin k_y,sin k_z)`.  The current-curl contribution is
orthogonal to the gradient source and cannot cancel it.  For the rigid
velocity current it is additionally one order smaller on this branch.
Therefore, for every `n in U`,

\[
 \boxed{
 \frac{T^{m+2}}{G_C}|\mathbf f_T|
 \longrightarrow
 \sqrt3\,r_0^{m+1}|n_aP_m(\mathbf n)|>0.}
\]

Thus the exact numerator is nonzero for every sufficiently large `T` along at
least one regular oblique resonance direction.  Charge neutrality and any
finite number of cancelled multipoles can raise `m`; they cannot make a
nonzero finite profile vanish in every direction.

## 4. Contradiction form

Assume instead that a nonzero finite source cancels the complete slow branch
for every sufficiently large `T`.  Then for every `n` with `n_a != 0`,

\[
 S(r_T\mathbf n/T)=0.
\]

Multiplication by `T^m` followed by `T -> infinity` gives

\[
 r_0^mP_m(\mathbf n)=0
\]

on both open hemispheres `n_a != 0`.  A homogeneous polynomial that vanishes
there vanishes identically, contradicting the choice of `m`.  Hence no
nonzero fixed finite profile can cancel the full slow-hop surface.

## 5. Dressing consequence

At the surviving witness, the radial derivative of `D_T` is nonzero.  A local
normal coordinate `z` therefore gives

\[
 D_T=\lambda z+O(z^2),\qquad \lambda\ne0,
\]

while the source numerator is continuous and nonzero.  The putative co-moving
linear response satisfies `|J|^2 >= C/z^2`; its normal integral diverges.
Therefore a fixed finite rigid source has no square-summable exactly co-moving
linear dressing for all sufficiently slow periodic hops.

## 6. Relation to FTD-0560/0561

- FTD-0560 is the all-period point-source result.
- FTD-0561 gives exact axial multipole coefficients and proves that charged
  width retains `T^-2` forcing.
- FTD-0562 closes the remaining fixed finite neutral form-factor escape on the
  complete three-dimensional slow branch.

The theorem does not prove physical matter radiation power.  It also does not
exclude a profile whose support grows with `T`, internal deformation during a
hop, nonlinear spectral detuning, a defect/topological carrier, or a
self-consistent matter-field periodic orbit.  Those mechanisms are no longer
finite rigid linear dressings and require a distinct dynamical construction.
