# Pre-registration — Cubic canonical-form uniqueness and bath-rank price (FTD-0573)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`
**Date:** 2026-07-26
**Parent results:** FTD-0570, FTD-0571, FTD-0572.
**Production changes permitted:** none.

## 1. Question

FTD-0572 derives a two-/three-pair minimum bath under the standard canonical
pairing `Omega_0=[[0,I],[-I,0]]`, while noting that arbitrary symplectic forms
can change the defect rank. This campaign asks:

1. Does onsite cubic covariance uniquely select `Omega_0` when `J` and `W`
   transform as equivalent three-vector representations?
2. What is the exact minimum defect rank if cubic covariance is dropped and a
   form may be chosen branch by branch?
3. What bath-rank price is paid by imposing cubic covariance?

The result remains conditional on treating `(J,W)` as a six-dimensional
canonical system. It does not derive that canonical ontology from the five
postulates.

## 2. Cubic-group classification

Enumerate all 48 signed permutation matrices `R in O_h` and their 24 proper
subgroup elements. Let

\[
G(R)=\operatorname{diag}(R,R)
\]

act on `V=R^3_J direct-sum R^3_W`. Parameterize the most general real
antisymmetric `6x6` form by 15 coefficients and impose

\[
G(R)^{\mathsf T}\Omega G(R)=\Omega
\]

for every proper cubic rotation. The exact constraint matrix must have rank 14
and one-dimensional nullspace spanned by

\[
\Omega_0=
\begin{pmatrix}0&I_3\\-I_3&0\end{pmatrix}.
\]

Verify that this span is also invariant under all 48 elements. Nondegeneracy
must be equivalent to a nonzero overall scale.

## 3. Frozen genesis derivative

Use the radial/tangential eigenbasis

\[
M=\operatorname{diag}(1,t,t,a,a,a),
\qquad 0<t<1,\quad 0\le a\le1.
\]

The production-grid arms are the same registered 10 directions, three
excesses `x/k_g in {0.125,0.5,1.25}`, and four drains
`d in {0,0.5,0.9,1}`: 120 arms. Add 30 exact degeneracy controls with `a=t`.

Under `Omega_0`, require defect rank four at `a=1` and rank six at `a<1`.

## 4. Unconstrained branchwise minima

For diagonal eigenvalues `m_i`, use

\[
\Delta_{ij}=(1-m_im_j)\Omega_{ij}.
\]

Prove the repeated-eigenspace lemma. If `Delta` has rank `2r` and an
eigenspace `E_mu` has dimension `g`, with all relevant denominators nonzero,
then `g>2r` forces a nonzero vector in `ker Omega`. Hence a nondegenerate
`Omega` requires `rank Delta>=g` rounded up to an even integer.

Register these exact minima:

### Zero drain `a=1`

The determinant obstruction gives nonzero even defect rank. Construct a
nondegenerate form pairing the four unit-eigenvalue directions internally and
the two tangent directions together. It must have defect rank two. Therefore
the unconstrained minimum is two: one bath pair.

### Generic positive drain `0<=a<1`, `a!=t`

The three-dimensional `a` eigenspace excludes rank two. Define

\[
r=\frac{1-at}{(1-a)(1+t)}
\]

and in the eigenbasis set the only independent nonzero entries

\[
\Omega_{12}=\Omega_{34}=\Omega_{56}=\Omega_{14}=1,
\qquad
\Omega_{23}=-r.
\]

Require

\[
\det\Omega
=\frac{(t-a)^2}{(1-a)^2(1+t)^2}>0,
\qquad
\operatorname{rank}\Delta=4.
\]

Thus the unconstrained minimum is four: two bath pairs.

### Degeneracy `a=t`

The contracting eigenspace has dimension five. The repeated-eigenspace lemma
excludes ranks two and four, while `Omega_0` attains rank six. The minimum is
six: three bath pairs.

## 5. Symmetry price and scope

For all 120 registered production-grid arms, `a!=t`. Require the cubic form's
defect rank to exceed the unconstrained branchwise minimum by exactly two,
equivalent to one canonical bath pair.

The unconstrained forms may depend on event direction, `t`, and `a`. They are
not a single native symplectic structure over the full phase space. The cubic
result applies only to constant onsite bilinear forms with `J` and `W`
transforming as equivalent vectors. Derivative-dependent, nonlocal,
field-dependent, presymplectic, or nonlinear forms remain outside scope.

## 6. Acceptance gates

Pass requires:

- 48/48 signed permutation matrices and 24/24 proper matrices enumerated;
- exact/numerical invariant constraint rank 14 and nullity one;
- `Omega_0` invariant under all 48 elements below `1e-12`;
- 120/120 cubic defect-rank arms correct;
- 30/30 zero-drain alternatives have nondegenerate `Omega` and rank-two
  defect;
- 90/90 generic positive-drain alternatives have nondegenerate `Omega` and
  rank-four defect;
- 30/30 `a=t` controls prove/measure rank-six minimum;
- symmetry price equals defect-rank two, or one bath pair, in every production
  arm;
- independent exact symbolic and numerical implementations agree;
- production golden hashes remain unchanged.

Failure leaves FTD-0572 explicitly pairing-selected. Pass upgrades the pairing
from arbitrary to unique within the stated onsite cubic representation class;
it does not derive a native Hamiltonian or bath.

The preregistered verdict string is:

```text
CUBIC_COVARIANCE_SELECTS_STANDARD_PAIRING_AND_PRICES_ONE_BATH_PAIR
```

## 7. Locked source provenance

```text
FTD-0572 theorem   8BCB40F379246EF36C6CA7CDFDD5757DAB66D3ABFF622C0439482B7C5BDEE8AA
FTD-0572 header    CCD7B09967D194498B50A7AFA449E04ED21FF06746F6B3E98651A18FD4AA1B42
FTD-0572 source    47664A3A83BDC7125DF8C5C84FB23B09EA1F159EC2F471AD368D9697BFA83223
phase_write.cpp    2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4
voxel.h            8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3
```
