# FTD-0574 — Native Field Discrete Action and Source-Operator Boundary

**Status:** `[THEOREM — EXACT LOCAL SOURCE-FREE DISCRETE ACTION]` +
`[THEOREM — NATIVE LEGENDRE MOMENTUM/CANONICAL FORM]` +
`[THEOREM — NORMALIZED QUADRATIC TICK INVARIANT]` +
`[DERIVED — PRESCRIBED-SOURCE ACTION]` +
`[SCOPED NO-GO — FIXED-RANGE EXACT CONTINUOUS GENERATOR]` +
`[CORRECTION — DOCUMENTED MAGNETIC SOURCE ACTION MISMATCH]` +
`[OPEN — DYNAMIC COMMON MATTER/FIELD ACTION]`  
**Date:** 2026-07-26  
**Verdict:**
`NATIVE_FIELD_DISCRETE_ACTION_DERIVED_MAGNETIC_SOURCE_ACTION_MISMATCH`

## 1. Scope

This theorem concerns the frozen single-substrate, unit-step production field
map with damping, genesis, evaporation, Gauss projection, reactions, clocks,
and forces disabled. It then admits `s` and `v` only as externally prescribed
source histories. It does not claim that the full production tick is
variational.

Let `L` be the symmetric periodic 18-point production Laplacian and

\[
 K=-C_{\rm WAVE}^{2}L.
\]

Then `K` is positive semidefinite on every finite periodic computational
quotient. The source-free production update is

\[
 W_{n+1}=W_n-KJ_n,
 \qquad
 J_{n+1}=J_n+W_{n+1}.
\]

## 2. Exact discrete action

Define the nearest-time-slice discrete Lagrangian

\[
 \boxed{
 L_d(J_n,J_{n+1})=
 \frac12\lVert J_{n+1}-J_n\rVert^2
 -\frac12\langle J_n,KJ_n\rangle.}
\]

Its discrete Euler--Lagrange equation is

\[
 D_2L_d(J_{n-1},J_n)+D_1L_d(J_n,J_{n+1})=0,
\]

or

\[
 J_{n+1}-2J_n+J_{n-1}=-KJ_n.
\]

Setting `W_n=J_n-J_{n-1}` gives the production kick-drift map exactly. No
continuum limit and no fitted coefficient enter this derivation. The action is
nearest-neighbour in discrete time and uses exactly the finite 18-point spatial
stencil.

## 3. `wave_vel` is the canonical momentum

The two discrete Legendre transforms are

\[
 p_n^-=-D_1L_d(J_n,J_{n+1})
      =(J_{n+1}-J_n)+KJ_n=W_n,
\]

\[
 p_n^+=D_2L_d(J_{n-1},J_n)=J_n-J_{n-1}=W_n.
\]

Thus the production field called `wave_vel` is exactly the discrete canonical
momentum of `J` in this sector. The regular mixed Hessian of `L_d` is `-I`, so
the variational map preserves

\[
 \Omega_0=
 \begin{pmatrix}0&I\\-I&0\end{pmatrix}.
\]

This resolves FTD-0573's native-action question for the free field: the
standard `(J,W)` pairing is no longer merely a symmetry-selected structure
there. It does not make genesis or any other non-free event symplectic; it
instead makes their previously measured symplectic defects relative to a
native surrounding field structure.

## 4. Unique normalized tick invariant

For a mode with `K` eigenvalue `a`, the transfer matrix is

\[
 U_a=\begin{pmatrix}1-a&1\\-a&1\end{pmatrix}.
\]

Write a real symmetric quadratic form as

\[
 G=\begin{pmatrix}x&y\\y&z\end{pmatrix}.
\]

The linear system `U_a^T G U_a=G` has rank two and one-dimensional nullspace

\[
 (x,y,z)=\lambda(a,-a/2,1).
\]

With onsite kinetic normalization `z=1`, the unique form is

\[
 G_a=\begin{pmatrix}a&-a/2\\-a/2&1\end{pmatrix},
 \qquad
 \det G_a=a\left(1-\frac a4\right).
\]

It is positive definite for `0<a<4`; the spatial zero mode retains only the
degenerate constant `W` energy. The production `FULL` stencil has
`a<=16/9`, so every nonzero production mode lies strictly inside the positive
region. In position space the normalized invariant is

\[
 \boxed{
 H_{\rm tick}=
 \frac12\langle W,W\rangle
 +\frac12\langle J,KJ\rangle
 -\frac12\langle W,KJ\rangle.}
\]

This independently recovers the FTD-0293 invariant. Translation-invariant
quadratic invariants can be multiplied modewise by other scalar weights; the
uniqueness statement fixes the `W-W` block to the onsite identity. Without
that normalization, uniqueness is only up to a mode weight.

## 5. Exact shadow Hamiltonian and its locality price

For `0<a<4`, let

\[
 \cos\theta=1-\frac a2,
 \qquad
 \sin\theta=\sqrt{a(1-a/4)},
 \qquad
 \mu(a)=\frac{\theta}{\sin\theta}.
\]

Since

\[
 \left(U_a-\cos\theta I\right)^2=-\sin^2\theta I,
\]

the principal logarithm is

\[
 \log U_a=\mu(a)(U_a-\cos\theta I).
\]

With the convention `dot X=Omega_0 grad H`, its exact shadow-Hamiltonian
metric is

\[
 \boxed{G_{\log}(a)=-\Omega_0\log U_a=\mu(a)G_a,}
\]

and `exp(Omega_0 G_log)=U_a` exactly.

The price is spatial nonlocality if one insists on a continuous-time generator.
The function

\[
 \mu(a)=1+\frac a6+\frac{a^2}{30}+\frac{a^3}{140}
 +\frac{a^4}{630}+\cdots
\]

is not a polynomial. More strongly, it has a square-root branch at `a=4`.
Along a production `<100>` line,

\[
 a=\frac23(1-\cos k).
\]

The branch corresponds to `cos k=-5`, hence to the finite nonzero Laurent
points

\[
 z=e^{ik}=-5\pm2\sqrt6.
\]

A fixed finite-range translation-invariant continuous generator has a finite
Laurent-polynomial symbol and cannot possess this branch. Therefore no
volume-independent fixed-range exact continuous generator produces every
production mode. On each finite box a global interpolation is possible, but
its range/order grows with the spectrum. The exact discrete action and exact
tick invariant remain finite-range; the no-go applies only to replacing the
tick by one exact autonomous continuous flow.

## 6. The exact prescribed-source action

Let `D`, `G`, and `C` be the periodic central divergence, gradient, and curl.
Direct index shifts give

\[
 D^{\mathsf T}=-G,
 \qquad
 C^{\mathsf T}=C.
\]

For externally prescribed `s_n,v_n`, define

\[
 \boxed{
 I_{\rm src}(J;s,v)=
 G_C\langle s,DJ\rangle
 +G_C\langle CJ,sv\rangle.}
\]

Its field variation is

\[
 \nabla_J I_{\rm src}
 =-G_CGs+G_CC(sv),
\]

which is exactly the source coded in `phase_read`. Adding `I_src` to `L_d`
therefore gives

\[
 W_{n+1}=W_n-KJ_n-G_CGs+G_CC(sv).
\]

For a prescribed source this is an affine translation of the canonical map;
its Jacobian remains `U`, so it is symplectic. Source work prevents a
source-free energy-conservation claim.

## 7. Exact mismatch in the documented six-term action

The engine action diagnostic instead documents the magnetic interaction

\[
 I_{\rm doc}=-G_C\langle sv,J\rangle.
\]

Its field variation is

\[
 \nabla_J I_{\rm doc}=-G_Csv,
\]

not `+G_C C(sv)`.

The periodic uniform fixture is decisive. Take `s=+1` and constant nonzero
`v=v_0`. Then

\[
 Gs=0,\qquad C(sv_0)=0,
\]

so the coded source is exactly zero, while

\[
 \nabla_J I_{\rm doc}=-G_Cv_0\ne0.
\]

All eight registered axis/generic fixtures return coded source zero and
documented-source mismatch

\[
 \lVert\Delta\rVert=G_C=0.08542454310285437.
\]

This is an operator counterexample, not a normalization discrepancy. The
onsite velocity interaction may be studied as a selected matter-force model,
but it cannot be cited as the field-source origin of the coded curl term.

## 8. Why this does not yet produce a common matter action

For orientation only, the continuum point-worldline reduction of the correct
source interaction is

\[
 L_{\rm int}=G_Cq[\operatorname{div}J(X)
 +v\cdot\operatorname{curl}J(X)].
\]

Varying the path gives the interaction force

\[
 F_{\rm int}=G_Cq[\nabla\operatorname{div}J
 -\partial_t(\operatorname{curl}J)
 +v\times\operatorname{curl}(\operatorname{curl}J)].
\]

This is not the optional production law
`alpha q v cross curl(J)`. A fully discrete path variation must also choose
the correct temporal staggering, as the FTD-0484 worldline program already
showed. Hence FTD-0574 derives a free-field action and a prescribed-source
field action, not reciprocal mobile matter.

## 9. Verification

The native MSVC observer executes:

- 36 production mode arms;
- four periodic full-lattice action arms;
- four source-operator arms;
- 96 proper-cubic covariance arms;
- eight uniform moving-source counterexamples.

Worst native residuals are:

| quantity | residual |
|---|---:|
| discrete Euler--Lagrange | `1.11e-16` |
| Legendre momentum | `5.55e-17` |
| exact tick invariant | `2.08e-15` |
| exact shadow flow | `1.11e-16` |
| electric adjoint | `7.47e-16` |
| curl adjoint | `5.03e-17` |
| prescribed-source action derivative | `1.73e-16` |
| proper-cubic covariance | `1.22e-15` |

The independent SymPy/NumPy proof reproduces the symbolic nullspace, Legendre
maps, branch obstruction, operator adjoints, and uniform counterexample. The
run of record is `engine/results/ftd_0574/windows_msvc_cpu.json`.

## 10. Boundary and next gate

What is now derived:

- the isolated production wave sector is a local variational integrator;
- `(J,W)` is a native canonical pair in that sector;
- the measured modified energy is its unique onsite-kinetic-normalized
  quadratic invariant;
- the coded moving source has an exact prescribed-source interaction.

What remains open:

- the source interaction's reciprocal matter path variation is not present in
  production;
- genesis, evaporation, damping, projection, movement, collision, and weak
  transmutation are not part of this action;
- the genesis bath found in FTD-0572 still needs an active energy and a local
  reset/export/transport law;
- no photon, particle, unitary quantum theory, or Lorentz recovery follows.

FTD-0575 performs the first reciprocal variation. It derives a Lorentz-form
Hodge force, but proves that the channel's two derivative vertices cancel the
static massless pole and make equal polarity attractive. The remaining gate is
therefore exact finite-step common energy and a stable mobile carrier for a
force that is explicitly not Coulomb electromagnetism, or a genuinely
different nonlinear/enlarged native charge mechanism.
