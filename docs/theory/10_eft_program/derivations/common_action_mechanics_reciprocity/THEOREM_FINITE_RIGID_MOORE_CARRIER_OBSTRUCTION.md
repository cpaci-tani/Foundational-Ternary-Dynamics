# FTD-0579 — Finite Rigid Moore-Carrier Obstruction

**Status:** `[THEOREM — FINITE RIGID DIAGONAL-CENTERING NO-GO]` +
`[THEOREM — FINITE RIGID PEIERLS POSITIVITY]` +
`[DERIVED — SMOOTH-ENVELOPE SUPPRESSION LAW]` +
`[CLOSED NEGATIVE — FINITE RIGID EXTENSION AS EXACT CURE]`  
**Date:** 2026-07-26  
**Verdict:**
`FINITE_RIGID_MOORE_CARRIER_CANNOT_REMOVE_CENTERING_OR_PEIERLS_EXTENSION_SUPPRESSES_ONLY`

## 1. Scope

FTD-0578 derived a reciprocal Moore/Hodge action but found two defects for a
compact point carrier: diagonal energy centering and a Peierls self-force.
FTD-0579 tests the entire class of nonzero, finite-support, rigid carrier
profiles. It does not select such a profile as native matter.

## 2. Finite rigid centering theorem

Let a nonzero real finite-support source have Laurent symbol

\[
 A(z)=\sum_n a(n)z^{-n}.
\]

All coefficients follow the same straight CIC Moore displacement `d`. With

\[
 u_i=z_i^{-d_i}-1,\quad
 P_d(t)=\prod_{i:d_i\ne0}(1+t u_i),\quad
 T_d=\int_0^1P_d(t)dt,
\]

and endpoint midpoint `H_d=(1+z^{-d})/2`, the FTD-0577 coated mismatch is

\[
 \boxed{\Delta_d(z)=B_M(z)A(z)M_d(z)},\qquad M_d=T_d-H_d.
\]

Direct integration gives

\[
 M_{\rm axial}=0,
\]

\[
 M_{\rm edge}=-\frac{u_i u_j}{6},
\]

\[
 M_{\rm body}=-\frac{u_i u_j+u_i u_k+u_j u_k}{6}
                -\frac{u_i u_j u_k}{4}.
\]

The real Laurent-polynomial ring is an integral domain. For an edge or body
path, `B_M`, nonzero finite `A`, and `M_d` are all nonzero Laurent
polynomials. Therefore

\[
 \boxed{B_M A M_d\ne0}
\]

for every nonzero finite rigid carrier on the infinite lattice. Finite
periodic checks require boxes larger than the unwrapped support so that a
quotient-ring alias cannot imitate cancellation.

The compiled observer tests five signed profiles, two volumes, both overall
polarity mirrors, and all 26 signed Moore directions. All 520 direct-space
and Fourier-space arms agree within `6.90e-17`. Axial mismatch is zero; the
minimum registered diagonal norm squared is `4.3402777777777775e-4`.

## 3. Finite rigid Peierls theorem

For a common fractional axial displacement `r`,

\[
 \widehat\rho_r=B_M A[(1-r)+r e^{-ik_i}].
\]

Using the FTD-0575 static Hodge response `R_H`, elimination of the common
field gives

\[
 V_{\rm self}(r)=V_{\rm self}(0)+C_i[a]r(1-r),
\]

\[
 \boxed{C_i[a]=\frac{G_C^2}{L^3}\sum_k
 R_H(k)|B_M(k)A(k)|^2(1-\cos k_i).}
\]

On the infinite lattice the integrand is nonnegative and is strictly
positive on an open set unless `A` vanishes there. A nonzero Laurent
polynomial cannot vanish on an open set. Hence

\[
 \boxed{C_i[a]>0}
\]

for every axis and every nonzero finite-support rigid carrier. The potential
is polarity-even and has barrier `C_i/4`.

All 60 registered coefficient arms are positive. The minimum coefficient is
`1.1786374081877269e-4`, the minimum barrier is
`2.9465935204693173e-5`, and 540 direct potential samples obey the quadratic
law within `3.04e-17`. Cubic covariance closes within `1.87e-17`.

## 4. Smooth-envelope suppression

For the selected control family

\[
 h_m(n)=2^{-m}{m\choose n},\qquad
 a_m(n_x,n_y,n_z)=\prod_j h_m(n_j),
\]

the combined Moore coat has

\[
 |B_M A_m|^2=\prod_j\cos^{2N}(k_j/2),\qquad N=m+2.
\]

Since `R_H(k)=3+O(k^2)`, Gaussian rescaling gives

\[
 U_0\sim\frac{3G_C^2}{2\pi^{3/2}}N^{-3/2},\qquad
 C_i\sim\frac{3G_C^2}{\pi^{3/2}}N^{-5/2},
\]

\[
 \Delta V_i\sim\frac{3G_C^2}{4\pi^{3/2}}N^{-5/2},\qquad
 \boxed{\Pi_i\sim\frac1{2N}}.
\]

The exact relative centering norms are

\[
 \boxed{\frac{\|\Delta_{\rm edge}\|_2^2}{\|B_Ma_m\|_2^2}
 =\frac1{9(N+1)^2}},
\]

\[
 \boxed{\frac{\|\Delta_{\rm body}\|_2^2}{\|B_Ma_m\|_2^2}
 =\frac{2(N+1)-1}{6(N+1)^3}}.
\]

Thus relative L2 centering mismatch and the relative Peierls barrier both
fall as `O(N^-1)=O(R_rms^-2)`, but neither vanishes for finite `N`. The 12
registered finite-volume controls reproduce the exact ratios within
`7.83e-15`; at `m=32`, `N Pi_i` lies in
`[0.47974892672496616,0.48149388677329275]`.

## 5. Consequence

Replacing the compact carrier by any nonzero finite rigid profile cannot
exactly repair either FTD-0578 obstruction. A broad smooth profile can make
both defects small, but this is a quantitative approximation and not a
native derivation of mobile matter. The result leaves open a deforming
composite, volume-spanning or band-limited excitation, integer-only hopping,
or a separately selected energy-centered multistage action.

No production rule, toggle, default, scenario, carrier ontology, particle,
or Lorentz claim is changed. The locked preregistration SHA-256 is
`7E9C64012B5595969CBE645302450F234387747138A420D371E834FAB705914A`.

**Successor status (FTD-0580):** abandoning rigid trilinear transport in favor
of the positive endpoint chord resolves diagonal energy centering and admits a
democratic route-free face current. Its Peierls coefficient remains strictly
positive, so the gapless mobility obstruction survives.
