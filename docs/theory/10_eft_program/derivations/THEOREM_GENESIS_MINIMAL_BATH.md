# Theorem — Minimum symplectic bath for accepted genesis (FTD-0572)

**Status:** `[THEOREM — MINIMUM BATH RANK FOR THE FTD-0570 CANONICAL PAIRING]` +
`[CONSTRUCTIVE — MINIMUM PREPARED-BATH DILATION]` +
`[THEOREM — FIXED-PREPARATION NONREPEATABILITY]` +
`[SCOPED NO-GO — PASSIVE EQUAL-WEIGHT QUADRATIC ENERGY]`

**Verdict:** `MINIMAL_FEEDBACK_DILATION_REQUIRES_RESET_OR_ACTIVE_ENERGY_RESERVOIR`

**Dependencies:** FTD-0569, FTD-0570, FTD-0571.

## 1. Symplectic-defect rank fixes a minimum bath size

Fix the FTD-0570 canonical test pairing `x=(J,W)` with standard form
`Omega_x`. Let an added
`2m`-dimensional bath have canonical form `Omega_e`. Write one differentiable
branch of an enlarged map as

\[
S=\begin{pmatrix}M&B\\ C&D\end{pmatrix}.
\]

The upper-left block of `S^T Omega S=Omega` gives

\[
\boxed{\Delta
=\Omega_x-M^{\mathsf T}\Omega_xM
=C^{\mathsf T}\Omega_eC.}
\]

The upper-left block of the equivalent Poisson-form identity
`S Omega^{-1} S^T=Omega^{-1}` gives

\[
\boxed{\Xi
=\Omega_x^{-1}-M\Omega_x^{-1}M^{\mathsf T}
=B\Omega_e^{-1}B^{\mathsf T}.}
\]

Consequently,

\[
\operatorname{rank}C\ge\operatorname{rank}\Delta,
\qquad
\operatorname{rank}B\ge\operatorname{rank}\Xi,
\qquad
2m\ge\max(\operatorname{rank}\Delta,
           \operatorname{rank}\Xi).
\]

When `M` is invertible, the two defect ranks agree. Indeed,

\[
M^{-1}\Xi M^{-\mathsf T}
=(M^{\mathsf T}\Omega_xM)^{-1}-\Omega_x^{-1}
=(\Omega_x-\Delta)^{-1}\Delta\Omega_x^{-1},
\]

whose rank equals `rank Delta`. The unit-drain singular boundary is evaluated
directly below and has the same full rank.

For accepted genesis, in the radial/tangential basis,

\[
M=\operatorname{diag}(\Lambda,aI_3),\qquad
\Lambda=\operatorname{diag}(1,t,t),
\]

with `0<t<1` and `a=1-d`. The defect is

\[
\Delta=
\begin{pmatrix}0&K\\-K&0\end{pmatrix},
\qquad
K=I-a\Lambda
=\operatorname{diag}(1-a,1-at,1-at).
\]

Therefore

\[
\operatorname{rank}\Delta=
\begin{cases}
4,&d=0,\\
6,&0<d\le1.
\end{cases}
\]

Every exact symplectic dilation under this fixed canonical pairing thus
requires at least two canonical bath
pairs at zero drain and three at positive drain. Both bath-to-system feedback
`B` and system-to-bath record transfer `C` must have at least the same rank.

### Pairing scope

The numerical pair counts are not invariant under an arbitrary replacement of
`Omega_x`. At zero drain, the spectrum of `M` is `(1,1,1,1,t,t)`. A
nonstandard symplectic form can pair the four unit-eigenvalue directions among
themselves and the two tangent directions with each other. Its defect then has
rank two rather than the standard pairing's rank four.

What is pairing-independent is the need for some enlargement. For every
nondegenerate symplectic form, `M^T Omega M=Omega` would imply

\[
(\det M)^2=1.
\]

Production genesis instead has

\[
\det M=t^2a^3<1
\]

(and determinant zero at unit drain). No nondegenerate alternative symplectic
form can make the raw genesis derivative canonical. It can alter the minimum
bath-rank count, but cannot eliminate the reservoir requirement.

## 2. The lower bound is attained

Consider one defective canonical pair with production scales `(lambda,a)`.
For `0<a<=1`, define

\[
\beta=\sqrt{1-a\lambda}>0
\]

and add one bath pair `(Q,P)`. The map

\[
\begin{aligned}
q'&=\lambda q+\beta Q,&
p'&=a p+\beta P,\\
Q'&=-\frac{\beta}{a}q+Q,&
P'&=-a\beta p+a\lambda P
\end{aligned}
\tag{1}
\]

is symplectic. The four independent Poisson brackets are

\[
\{q',p'\}=a\lambda+\beta^2=1,
\qquad
\{Q',P'\}=\beta^2+a\lambda=1,
\]

and

\[
\{q',P'\}=\{Q',p'\}=0.
\]

At unit drain `a=0`, the nonsingular boundary construction is

\[
q'=\lambda q+Q,\qquad p'=P,\qquad
Q'=-q,\qquad P'=-p+\lambda P.
\tag{2}
\]

It obeys the same canonical brackets. For a prepared zero bath, both (1) and
(2) project exactly to the production assignment

\[
(q,p)\mapsto(\lambda q,ap).
\]

The feedback and record blocks of each defective pair have rank two. Taking
their direct sum over only the defective radial/tangential directions uses
exactly `rank Delta/2` bath pairs and saturates the lower bound. The 330
registered defective-pair arms close symplecticity below
`2.23e-16` and the prepared projection exactly.

This is stronger than a lower-bound no-go: the minimum local linear
symplectic dilation exists explicitly.

## 3. The minimum bath cannot remain an inert zero reservoir

Start (1) at `Q=P=0`. After one event the bath contains

\[
Q_1=-\frac{\beta}{a}q,
\qquad
P_1=-a\beta p.
\]

Applying the same enlarged map again yields a projected deviation from two
production steps:

\[
\boxed{
\delta q_2=-\frac{\beta^2}{a}q,
\qquad
\delta p_2=-a\beta^2p.}
\tag{3}
\]

At `a=0`, (2) instead gives

\[
\boxed{\delta q_2=-q,\qquad\delta p_2=-p.}
\tag{4}
\]

All 330 defective-pair arms reproduce (3)--(4) below `1.78e-15`; the minimum
registered nonzero deviation norm is `0.3583225665910466`.

There is also a general reason. If the fixed zero-bath section were invariant
for all system states, then `e'=0` at `e=0` would imply `C=0` along that
section. But the upper-left symplectic identity would then give `Delta=0`, in
contradiction with the genesis defect. A noncanonical projected map cannot
repeat on one fixed invariant zero-bath section.

Thus the exact one-event match requires the bath to be reset/replaced, its
record to be transported elsewhere, or the later system update to depend on
the accumulated bath state.

## 4. A passive equal-weight quadratic bath is impossible

Assume additionally that the enlarged linear map preserves

\[
H=\frac12\left(|x|^2+|e|^2\right).
\]

Then `S` is orthogonal as well as symplectic. These two identities imply

\[
S\Omega=\Omega S.
\]

Its system block must therefore satisfy

\[
\Omega_xM=M\Omega_x.
\]

For the genesis block this is equivalent to

\[
\Lambda=aI_3.
\]

But `Lambda` has eigenvalues `(1,t,t)` with `0<t<1`, so it cannot equal any
scalar `aI_3`. The commutator is nonzero in all 120 registered arms. Hence no
orthogonal-symplectic enlargement can both reproduce the genesis block and
conserve the equal-weight positive quadratic system-plus-bath energy.

This does not rule out a weighted or cross-coupled energy, active squeezing,
a nonlinear reservoir, or the time--energy extension constructed in
FTD-0570. It proves that the minimum bath is not a passive copy of the native
quadratic `(J,W)` energy.

## 5. Classification

- **minimum bath size:** theorem; two pairs at zero drain, three at positive
  drain, conditional on the FTD-0570 `(J,W)` canonical pairing;
- **minimum prepared-bath symplectic dilation:** constructed exactly;
- **one-event production projection:** exact on the selected zero-bath slice;
- **repeat without bath evolution/reset:** closed negative;
- **passive equal-weight quadratic energy:** closed negative;
- **native environmental variables and transport:** not derived;
- **production common action:** not recovered.

The environmental route now has a concrete minimum architecture. It must
contain at least the required canonical pairs, receive a full-rank record of
the genesis contraction, feed that record back, and provide an active energy
and reset/transport law. Otherwise it is only a one-event formal dilation.

## 6. Non-implications

- This theorem does not prohibit fundamental irreversible dynamics.
- It does not prove that `(J,W)` is the unique native canonical pairing or
  exclude a separately derived nonstandard symplectic structure. Such a
  structure can change the defect rank, but the determinant obstruction shows
  that it cannot make raw genesis symplectic without enlargement.
- It does not rule out a state-dependent invariant bath graph with a
  degenerate pulled-back symplectic form.
- It does not derive a bath Hamiltonian, thermodynamic temperature, entropy
  production, or Landauer cost.
- It does not license a production toggle, mobile particle, unitarity,
  scenario, or Lorentz claim.
