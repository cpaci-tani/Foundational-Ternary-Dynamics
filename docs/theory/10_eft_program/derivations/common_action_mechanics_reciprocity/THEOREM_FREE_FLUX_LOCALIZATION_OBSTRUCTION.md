# FTD-0557 — Free-Flux Localization Obstruction

**Status:** [THEOREM — INFINITE-LATTICE FREE OPERATOR] + [NUMERICAL FACT — FINITE CPU REPLAY]  
**Date:** 2026-07-26  
**Depends on:** the frozen isolated production `FULL`-stencil unit-tick
kick-drift map characterized by FTD-0556  
**Does not establish:** absence of nonlinear carriers, absence of finite-volume
plane-wave eigenmodes, absence of defect/topological binding, or absence of a
stable manifested `(s,J,W)` composite

## 1. Result

The native free `(flux,wave_vel)` band transports extended wavepackets but
cannot by itself supply a nonzero localized particle-like carrier on the
infinite lattice.

More precisely, the frozen translation-invariant free operator on
`l2(Z^3;C^2)` has:

1. no nonzero square-summable eigenstate;
2. no nonzero square-summable state that returns after a positive finite
   number of ticks as an exact integer translate times a phase; and
3. positive ballistic width growth for an unchirped localized one-band packet
   whenever its group-velocity variance is nonzero.

This sharpens FTD-0556.  Continuous motion of an extended Bloch centroid is
real free-field transport, but it is not particle localization.

## 2. Frozen Fourier multiplier

For one flux polarization, write

\[
 \widehat\Psi=(\widehat J,\widehat W)^T.
\]

The production kick-drift update is multiplication by

\[
 U(\mathbf k)=
 \begin{pmatrix}1-a(\mathbf k)&1\\-a(\mathbf k)&1\end{pmatrix},
 \qquad a(\mathbf k)=C_{\rm WAVE}^2M(\mathbf k),
\]

where

\[
 M(\mathbf k)=4-\frac23(c_x+c_y+c_z)
 -\frac23(c_xc_y+c_xc_z+c_yc_z).
\]

Every entry of `U` is a finite trigonometric polynomial and hence real
analytic on the Brillouin torus.  Its determinant is one and its trace is

\[
 \operatorname{tr}U=2-a(\mathbf k).
\]

Because `M(0)=0` while `M(k,0,0)>0` for nonzero sufficiently small `k`, the
trace and the two bands are nonconstant.

## 3. No localized point spectrum

Assume `Psi` is a square-summable eigenstate with eigenvalue `lambda`.  The
unitary Fourier transform from `l2(Z^3;C^2)` to
`L2(T^3;C^2)` gives

\[
 [U(\mathbf k)-\lambda I]\widehat\Psi(\mathbf k)=0
\]

almost everywhere.  A nonzero Fourier value is possible only on

\[
 Z_\lambda=\{\mathbf k:
 \det(U(\mathbf k)-\lambda I)=0\}.
\]

The determinant is a real-analytic function.  It cannot vanish identically,
because that would make `lambda` a constant eigenvalue while
`tr U=2-a(k)` is nonconstant and `det U=1`.  A nonzero real-analytic function
on the torus has a measure-zero zero set.  An `L2` function supported on a
measure-zero set is zero almost everywhere.  Therefore

\[
 \boxed{\Psi=0}.
\]

The free operator has no nonzero `l2` point spectrum.  Finite periodic boxes
do have normalizable Bloch eigenvectors because their momentum set is finite;
those states fill the box and are not infinite-volume localized carriers.

## 4. No finite-time rigid translate

Assume that for an integer displacement `d`, positive integer `T`, and phase
`phi`,

\[
 \Psi_T(n)=e^{i\phi}\Psi_0(n-d).
\]

In momentum space,

\[
 [U(\mathbf k)^T-z(\mathbf k)I]\widehat\Psi_0(\mathbf k)=0,
 \qquad
 z(\mathbf k)=e^{i\phi-i\mathbf k\cdot d}.
\]

As before, a nonzero state would need positive-measure support in the zero set
of the real-analytic determinant

\[
 F(\mathbf k)=\det[U(\mathbf k)^T-z(\mathbf k)I].
\]

`F` is not identically zero.  At `k=0`, the double eigenvalue of `U(0)^T` is
one, so an identity would force `e^{i phi}=1`.  If `d` is nonzero, choose a
nonzero momentum direction orthogonal to `d`; if `d=0`, choose any nonzero
direction.  Along a sufficiently small segment in that direction, the
translation phase remains one while the native eigenphases are
`exp(+-i T theta(k))` with nonzero continuously varying `theta`.  Thus `F` is
nonzero there.  Its zero set has measure zero, forcing

\[
 \boxed{\Psi_0=0}.
\]

This excludes exact finite-time shape-preserving translation of any nonzero
square-summable free packet, not merely the Gaussian used in the observer.

## 5. Exact ballistic second-moment law

For one eigenbranch,

\[
 \widehat\psi_t(\mathbf k)
 =e^{-it\theta(\mathbf k)}\widehat\psi_0(\mathbf k).
\]

In Fourier representation `X_i=i partial_{k_i}`.  Direct differentiation gives

\[
 X_i\widehat\psi_t=e^{-it\theta}
 (X_i+t v_i)\widehat\psi_0,
 \qquad v_i=\partial_{k_i}\theta.
\]

After centering and taking the squared norm,

\[
 \boxed{
 \operatorname{Var}X_i(t)=\operatorname{Var}X_i(0)
 +2t\operatorname{Cov}_{\rm sym}(X_i,v_i)
 +t^2\operatorname{Var}(v_i)}.
\]

For a real nonnegative unchirped spectral envelope, the symmetrized covariance
term vanishes.  A packet localized in all spatial directions has Fourier
support of positive three-dimensional measure.  The joint analytic group
velocity is not constant on any open set, so its total velocity variance is
positive.  Hence at least one spatial width grows quadratically.  The
componentwise identity remains exact; the positivity conclusion is for the
sum of component variances unless a particular directional packet is fixed.

The registered `<100>`, `<110>`, and `<111>` packets each have positive
coordinate-velocity variance, so each registered propagation coordinate
broadens.

## 6. Why this is the actual frozen boundary

Direct inspection of the production phases shows two disconnected mechanisms:

- `phase_read` plus the undamped `phase_write` kick-drift branch is linear in
  `(J,W)` for fixed ternary state;
- `phase_movement` increments `remainder` from stored velocity and performs an
  integer hop when a component crosses `+-1`;
- with forces off, that manifested motion does not read the free wave field;
- with movement off, a site state is static while the coupling term can source
  the field one way through `-G_C grad(s)+G_C curl(sv)`;
- the active EM, Poisson, gravity, Lorentz, color, and other force laws are
  separately selected branches, not consequences of the isolated free-band
  operator.

Therefore the frozen core currently supplies transport without binding in the
free field, and localization without a derived reciprocal binding law in the
site state.  The missing object is a common-action nonlinear composite, not a
different label for the free packet.

## 7. Locked production replay

The explicitly forced CPU run of `test_free_flux_localization` returned:

| Diagnostic | Result |
|---|---:|
| packet arms | `3` |
| arm-tick replays | `48` |
| CPU backend checks | `3/3` |
| manifested sites observed | `0` |
| minimum group-velocity variance | `2.7168028537805089e-4` |
| maximum engine/Bloch residual | `1.6012399789238215e-16` |
| maximum density-moment residual | `1.8189894035458565e-12` |
| spectral-norm residual | `6.9388939039072284e-18` |
| maximum direct variance increase | `1.9157054836437055` |
| minimum seam margin | `7.615692133454667` initial RMS widths |

The locked verdict is

`FREE_FLUX_TRANSPORT_HAS_NO_LOCALIZED_RIGID_CARRIER`.

The finite CPU arm is a realization check and broadening witness.  The
no-point-spectrum and no-rigid-translate conclusions come from the
infinite-lattice analytic proof, not from observing 16 ticks.

## 8. Research consequence

Calling `flux-soliton` a soliton would now be mathematically false for the
isolated configuration used by that scenario.  It is correctly retained as a
dispersion diagnostic.

The remaining carrier gate must involve a genuine binding mechanism:

- a native nonlinear `(s,J,W)` feedback;
- a defect or topological sector;
- a flat band from additional internal structure; or
- an explicitly selected extension whose ontological cost is stated.

Legacy imposed forces, the imposed de Broglie clock, a finite-volume standing
wave, and a visually persistent transient do not satisfy this gate.
