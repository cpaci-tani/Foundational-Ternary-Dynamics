# FTD-0577 — Minimal Moore Compatibility Coat

**Status:** `[THEOREM — UNIQUE SEPARABLE SYMMETRIC RADIUS-ONE FILTER]` +
`[THEOREM — EXACT LOCAL FACE-TO-CENTRAL CONTINUITY BRIDGE]` +
`[SELECTED — NONCARDINAL COUPLING COAT]` +
`[OPEN — RECIPROCAL FORCE, SELF-FORCE, AND MOBILE CARRIER]`  
**Date:** 2026-07-26  
**Verdict:**
`MINIMAL_MOORE_COAT_RESTORES_LOCAL_CENTRAL_CONTINUITY_NONCARDINAL_SELECTED`

## 1. Scope

FTD-0576 proved that cardinal endpoint density and the native central
divergence cannot support a finite-range hopping current. The obstruction is
the extra central-difference zero at the checkerboard mode. This theorem
derives the smallest symmetric separable coat that cancels that zero and
shows how to turn the already exact FTD-0478 face current into a local native
site current.

The primitive state remains `s in {-1,0,+1}` at one site. The coat is a
deterministic coupling representation derived from `(s,remainder)`. It is not
a fractional primitive state and does not change production.

## 2. Unique radius-one axial filter

Let a centered real radius-one filter have symbol

\[
 B(z)=a(z+z^{-1})+b.
\]

Unit normalization and cancellation of the central checkerboard zero require

\[
 2a+b=1,\qquad -2a+b=0.
\]

The coefficient matrix has determinant four, so within this class the unique
solution is

\[
 \boxed{a=\frac14,\qquad b=\frac12,\qquad
 B(z)=\frac{z^{-1}+2+z}{4}=\frac{(z+1)^2}{4z}.}
\]

All coefficients are positive. Their sum is one and their first moment is
zero. The uniqueness statement is intentionally limited: it does not cover
nonseparable, wider, nonlinear, or state-dependent filters.

## 3. The 27-site coat

Tensor the axial filter:

\[
 B_M=B_xB_yB_z.
\]

At integer remainder the weights occupy exactly the Moore neighborhood:

| class | multiplicity | weight per site | class total |
|---|---:|---:|---:|
| center | 1 | `1/8` | `1/8` |
| face | 6 | `1/16` | `3/8` |
| edge | 12 | `1/32` | `3/8` |
| corner | 8 | `1/64` | `1/8` |

The 27 weights are positive, sum exactly to one, have zero first moment, and
are invariant under the cubic group. Convolving the trilinear FTD-0478 shape
with them therefore preserves signed polarity, partition of unity, and the
effective position `site+remainder`.

The integer-centered coupling representation is not cardinal: only `1/8` of
its coupling weight is at the manifested site. That is the explicit price of
cancelling the checkerboard component while keeping primitive manifestation
ternary.

In momentum space,

\[
 \boxed{B_M(k)=\prod_{i=1}^3\cos^2\frac{k_i}{2}
 =1-\frac{|k|^2}{4}+O(|k|^4).}
\]

Thus the zero mode and infrared polarity survive, while any mode with a
coordinate `k_i=pi` is removed from the coupling source.

## 4. Exact local current bridge

Let `K_i` be the oriented face current derived by integrating the straight
FTD-0478 subcell trajectory. It obeys

\[
 \delta\rho_{\rm CIC}+\sum_i d_{f,i}K_i=0,
 \qquad d_f(z)=1-z^{-1}.
\]

Define coated density and site-centered current

\[
 \rho_M=B_M\rho_{\rm CIC},
\]

\[
 \boxed{Q_i=A_i\prod_{j\ne i}B_jK_i,
 \qquad A_i(z_i)=\frac{1+z_i^{-1}}2.}
\]

This prescription is local: smooth a face-current component in its two
transverse directions, then average it with its negative-axis neighbor. For
the native central symbol

\[
 d_c(z)=\frac{z-z^{-1}}2,
\]

direct factorization gives

\[
 \boxed{d_c(z)A(z)=B(z)d_f(z).}
\]

Consequently,

\[
 \boxed{\delta\rho_M+\sum_i d_{c,i}Q_i=0}
\]

exactly. The `1/(z+1)` pole of the direct FTD-0576 projection has been
cancelled by the `(z+1)^2` zero in the coupling coat.

## 5. Compatibility with the exact energy ledger

Because the coated endpoints and bridged current obey the same central
continuity equation used by FTD-0576, its conditional energy theorem applies
without alteration:

\[
 \Delta H_f+Delta U_{\rm int}+\Delta H_m=0,
\]

where

\[
 U_{\rm int}=-G_C\langle\rho_M,DR\rangle,
\]

\[
 \Delta H_m=G_C\langle Q,GD\bar R-C\delta R\rangle.
\]

Four independent periodic field fixtures close this identity below
`4.4e-18`. This remains a conditional work ledger. It does not yet provide
the implicit particle step that realizes that work through the production
dispersion.

## 6. Verification and limitations

The compiled observer covers:

- 36 path arms on `L=17,33`, both polarities, all six axial hops, and three
  diagonal straight segments;
- three integer translations;
- all 24 proper cubic rotations of a generic path;
- four conditional Hodge-energy fixtures.

Measured maxima are:

| diagnostic | maximum |
|---|---:|
| partition | `2.22e-16` |
| first moment | `1.07e-14` |
| central continuity | `9.02e-17` |
| translation covariance | `1.25e-16` |
| cubic covariance | `1.04e-16` |
| total-energy ledger | `4.34e-18` |

Local support is volume-independent: endpoint density uses 27--64 sites and
current uses 18--56 sites on both registered volumes.

The independent exact proof verifies the Laurent identities, all rational
weights and moments, a rational periodic continuity replay, the infrared form
factor, and the continued absence of a static Coulomb pole. Coating source
and probe multiplies the finite FTD-0575 response by a regular form factor; it
cannot create `1/k^2`.

Therefore FTD-0577 repairs one precise incompatibility—local continuity—but
does not derive reciprocal force, self-force cancellation, a stable mobile
manifested configuration, electromagnetic phenomenology, or production
dynamics. Those remain the next action-level gates.

**Successor disposition (FTD-0578):** the reciprocal coated worldline action
and its adjoint path gather are now derived. The self-force does not cancel:
the compact point carrier has a positive Peierls barrier. Its generic
diagonal time-exact source also disagrees with the FTD-0576 energy-centered
source. The unmodified point-action route is therefore closed as freely
mobile matter; extended-carrier and registered multistage routes remain open.

The run of record is `engine/results/ftd_0577/windows_msvc_cpu.json`. The
locked preregistration SHA-256 is
`94C706936189B077A144ACA7B64D4FEBE93DCDB93AEA36BA604C466480C80F8D`.
