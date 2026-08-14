# C18 finite-range characteristic and rigid-translator obstruction v1

**Identifier:** `FTD-0943`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — NO SCALAR FINITE-RANGE C18 CHARACTERISTIC FACTOR]` + `[THEOREM — NO NONZERO FINITE-SUPPORT EXACT TRANSLATOR OR PERIODIC COMPLETE STATE]` + `[CLOSED NEGATIVE — ISOLATED LINEAR PROTECTED-PULSE ROUTE]` + `[OPEN — EVENT-MEDIATED NONLINEAR CARRIER]`  
**Protocol:** [`PREREG_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md), pre-run SHA-256 `0B6F8C0B3C8EC1BA1E65E1FD31E78887BC5FBB6498BEF5C0C807A7EF11179104`  
**Certificate:** `scripts/proofs/proof_c18_finite_range_characteristic_rigid_translator_obstruction.py`, SHA-256 `D94B419F2FF433E6477C8D9DCEC0878A70930F77180A28AFDF3CFDBAC8D00C0C`, `478/478`, **Outcome B**

## 1. Result

For the existing isolated, undamped production `C18` relative canonical
field, two exact local-carrier routes are closed:

> **[THEOREM — CHARACTERISTIC OBSTRUCTION]** Neither the positive scalar
> stiffness symbol nor the exact kick--drift characteristic discriminant is
> a square in the finite-range Laurent ring. Therefore the existing scalar
> component has no exact finite-range square-root factorization and the
> two-component kick--drift state has no exact finite-range spectral
> diagonalization into one-way characteristics.

> **[THEOREM — TRANSLATOR/RECURRENCE OBSTRUCTION]** For every positive tick
> count `m` and every lattice displacement `d`, the only finite-support
> complete state satisfying `U^m X=z^d X` is the zero state. This includes
> `d=0`: no nonzero finite-support complete state is exactly periodic under
> the isolated free map.

The result applies componentwise to the three-component relative flux and
momentum. It does not exclude global modes, nonlocal Fourier
characteristics, dispersive packets, driven bodies, or nonlinear
event-mediated production dynamics. No new primitive type is forced.

## 2. Frozen production symbol

Let

\[
R=\mathbb C[z_x^{\pm1},z_y^{\pm1},z_z^{\pm1}]
\]

be the Laurent-polynomial ring of scalar finite-range translation-invariant
operators. It is an integral domain. The frozen face/edge stencil is

\[
L_{18}(z)=\frac13\sum_{e\in F_6}z^e
          +\frac16\sum_{e\in E_{12}}z^e-4,                         \tag{1}
\]

and production uses `C_WAVE^2=1/3`. Define the positive stiffness

\[
K(z)=-\frac13L_{18}(z)
=\frac13\left[4-\frac13\sum_{e\in F_6}z^e
                 -\frac16\sum_{e\in E_{12}}z^e\right].             \tag{2}
\]

For `z_j=e^{ik_j}` and `c_j=cos k_j`, equation (2) becomes

\[
K(k)=\frac43-\frac29(c_x+c_y+c_z+c_xc_y+c_yc_z+c_zc_x).            \tag{3}
\]

At the vacuum mode,

\[
K(0)=0,qquad \nabla K(0)=0,qquad
\operatorname{Hess}K(0)=\frac23I_3,                                \tag{4}
\]

so

\[
K(k)=\frac13|k|^2+O(|k|^4).                                       \tag{5}
\]

The rank-three Hessian in (4) is the decisive local invariant.

## 3. Scalar finite-range square-root obstruction

Suppose a scalar finite-range characteristic factor existed. Its Laurent
symbol `b in R` would obey

\[
b(z)^2=K(z).                                                        \tag{6}
\]

At `z=(1,1,1)`, equation (6) and `K=0` imply `b=0`. Differentiating twice in
local wave-number coordinates gives

\[
\operatorname{Hess}(b^2)(0)
=2\,\nabla b(0)\nabla b(0)^T.                                      \tag{7}
\]

The matrix on the right has rank at most one, whereas equation (4) has rank
three. This contradiction proves:

\[
\boxed{K\text{ is not a square in }R.}                             \tag{8}
\]

This is stronger than a failed coefficient search: it excludes every scalar
finite-range Laurent factor at once. It does not exclude adding a selected
multicomponent Clifford/Dirac representation. Such an extension is not
already present in one scalar component of the relative field and would
carry its own adoption price.

## 4. Exact discrete characteristic obstruction

For one scalar relative component, write the isolated default kick--drift
step with positive stiffness `K` as

\[
P' = P-KD,qquad D'=D+P'.                                          \tag{9}
\]

On the phase-complete state `X=(D,P)^T`,

\[
U(z)=\begin{pmatrix}1-K(z)&1\\-K(z)&1\end{pmatrix},
\qquad \det U=1,
\qquad \operatorname{tr}U=2-K.                                   \tag{10}
\]

The characteristic discriminant is

\[
\Delta=(\operatorname{tr}U)^2-4\det U=K(K-4).                    \tag{11}
\]

Equations (4)--(5) imply

\[
\Delta(0)=0,qquad \nabla\Delta(0)=0,qquad
\operatorname{Hess}\Delta(0)=-\frac83I_3.                         \tag{12}
\]

If `Delta=r^2` for `r in R`, its Hessian at the vacuum mode would again be an
outer product of rank at most one. Equation (12) has rank three. Therefore

\[
\boxed{\Delta\text{ is not a square in }R.}                        \tag{13}
\]

The exact eigenvalues

\[
\lambda_\pm={2-K\pm\sqrt{K(K-4)}\over2}                            \tag{14}
\]

and their projectors consequently require a non-Laurent square root. An
exact spectral split can be made in Fourier/modal space, but it is not a
finite-range local characteristic transformation of the existing state.

This sharpens FTD-0858. The earlier local bond variables remain an exact
energy/current chart, but they are not dynamically protected one-way
characteristics of the full `C18` update.

## 5. Arbitrary-tick rigid-translation theorem

A scalar finite-support complete state corresponds to a vector

\[
X(z)\in R^2.                                                        \tag{15}
\]

Assume that after `m>=1` free ticks it translates rigidly by
`d in Z^3`:

\[
U(z)^mX(z)=z^dX(z).                                                 \tag{16}
\]

Set `A=U^m-z^dI`. If `det A` is a nonzero Laurent polynomial, the adjugate
identity gives

\[
\det(A)X=0.                                                        \tag{17}
\]

Because `R` is an integral domain, equation (17) forces each component of
`X` to vanish. Thus a nonzero translator requires `det A` to vanish
identically.

Since `det U=1`, Cayley--Hamilton gives

\[
\operatorname{tr}(U^m)=2T_m(1-K/2),                               \tag{18}
\]

where `T_m` is the Chebyshev polynomial. The determinant condition becomes

\[
z^d+z^{-d}=2T_m(1-K/2).                                            \tag{19}
\]

The right side is invariant under the full signed cubic group: equation (2)
is invariant, hence every polynomial in `K` is invariant.

For every nonzero `d`, the left side is not fully cubic invariant. There are
two exhaustive cases:

1. If exactly one coordinate of `d` is nonzero, a coordinate permutation
   moves it to another axis, producing an exponent other than `d` or `-d`.
2. If at least two coordinates are nonzero, flipping exactly one nonzero
   coordinate again produces an exponent other than `d` or `-d`.

Distinct Laurent monomials are linearly independent. Therefore the
two-monomial orbit `z^d+z^{-d}` cannot equal the invariant right side of
equation (19). Hence `det A` is nonzero and

\[
\boxed{U^mX=z^dX,\ d\ne0\quad\Longrightarrow\quad X=0}             \tag{20}
\]

for every positive `m`.

This is not a finite-box or finite-tick enumeration. Cubic invariance and the
integral-domain argument cover all integer displacements and all positive
tick counts.

## 6. Exact recurrence obstruction

The remaining case is `d=0`. Here

\[
\det(U^m-I)=2-\operatorname{tr}(U^m)
=2-2T_m(1-K/2).                                                     \tag{21}
\]

The Chebyshev endpoint identity `T_m'(1)=m^2` gives the exact expansion

\[
2-2T_m(1-K/2)=m^2K+O(K^2).                                        \tag{22}
\]

For every `m>=1`, the leading coefficient is nonzero and `K` is a nonzero
Laurent polynomial. Equation (21) is therefore not identically zero. The
same adjugate/domain argument yields

\[
\boxed{U^mX=X\quad\Longrightarrow\quad X=0}                        \tag{23}
\]

for every finite-support complete state and every positive `m`.

Global constant or Bloch modes are not counterexamples: their lattice
support is infinite. On a finite periodic simulation box, discrete normal
modes can recur for specially selected maps; theorem (23) concerns compact
support on the infinite translation lattice, which is the local-body
question registered here.

## 7. Extension to the production vector pair

The isolated `C18` stencil acts independently on the three spatial
components of `(D,P_D)`. If a nonzero vector state translated or recurred,
at least one scalar component would be a nonzero solution of equation (16).
Equations (20) and (23) exclude that possibility. The theorem therefore
applies to the full six-real-coordinate relative canonical pair.

The conclusion is compatible with FTD-0919 but not redundant with it:

- FTD-0919 excluded finite-support stiffness eigenfields and compact
  finite-dimensional invariant modal doublets;
- FTD-0943 excludes a phase-complete rigid translator or recurrent state
  after **any** positive number of free ticks and independently excludes an
  exact finite-range characteristic diagonalization.

## 8. What the theorem does not say

This theorem closes one narrow route: an exact isolated-linear,
finite-range, compactly supported protected pulse already hidden in the
production relative field. It does not exclude:

- infinite-support Bloch or normal modes;
- the exact but nonlocal modal square root in equation (14);
- approximate wave packets and dispersive transport;
- exponentially localized but noncompact profiles;
- externally driven or maintained structures;
- nonlinear breathers or solitons from a separately preregistered action;
- genesis, weak transmutation, movement, boundary, or other event-mediated
  piecewise production actions; or
- an explicit selected direction-port carrier.

Most importantly, the proof is not a theorem about the entire production
tick. The full engine contains state-dependent and piecewise actions outside
the isolated free map. Those must be audited on their own transition
relation before any hardware addition is justified.

## 9. Physical interpretation

The common/relative pair is real clock-capable **aggregate hardware**, but
its native free wave dynamics does not supply a compact gear tooth that
moves unchanged from site to site. Its exact directional branches live in
modal space and therefore depend nonlocally on the field configuration.

That distinguishes two ideas which had been easy to conflate:

1. a global arithmetic or Fourier phase calendar can exist coherently; and
2. a localized body can own a protected, finite-range, recursively returning
   clock state.

The first does not imply the second. FTD now has an exact algebraic reason:
the three-dimensional isotropic quadratic cone cannot be the square of one
scalar local Laurent direction, and cubic symmetry forbids any compact state
from being an exact translation eigenvector of an arbitrary power of the
free update.

## 10. Verification record

The immutable exact run reported:

```text
FTD-0943 exact certificate: 478/478 checks passed
OUTCOME B — isolated linear C18 has no scalar finite-range exact
characteristic factor and no nonzero finite-support exact translator
or periodic complete state at any positive tick count.
Global/nonlocal, approximate, maintained, and event-mediated nonlinear
routes remain open; no new primitive type is forced.
```

The certificate checked source hashes and production markers, exact face and
edge symbols, the rank-three Hessians, both Laurent-square obstructions, all
48 signed cubic symmetries, the Cayley--Hamilton/Chebyshev trace relation,
the displacement-orbit lemma, exact translator determinants, recurrence
coefficients, adjugate identities, vector extension, and scope firewalls.

No engine or CMake source was changed and no numerical search was performed.

## 11. Next gate

The next live gate is not another free-wave packet ansatz. It is an exact
transition audit of the **existing event-mediated production actions**:

> Can genesis, weak L/R exchange, manifested movement, or their composition
> write and transport a reversible relative-field direction/history record
> with an explicit inverse and source-energy transaction, without consulting
> a target outcome or an external journal?

If that existing-action audit is also closed negative, the design fork
becomes explicit: adopt a preregistered nonlinear self-trapping action in the
current fields, or price a channelized direction-port type. Until then, a new
primitive is not logically compelled.
