# Theorem — Cubic canonical form and genesis bath price (FTD-0573)

**Status:** `[THEOREM — UNIQUE CONSTANT ONSITE FORM UNDER EQUIVALENT CUBIC VECTOR REPRESENTATIONS]` +
`[THEOREM — BRANCHWISE DEFECT-RANK MINIMA]` +
`[DERIVED — ONE-BATH-PAIR CUBIC-SYMMETRY PRICE]` +
`[RESOLVED BY FTD-0574 — NATIVE FREE-FIELD ACTION]` +
`[OPEN — GENESIS BATH ACTION/TRANSPORT]`

**Verdict:** `CUBIC_COVARIANCE_SELECTS_STANDARD_PAIRING_AND_PRICES_ONE_BATH_PAIR`

**Dependencies:** FTD-0570, FTD-0571, FTD-0572.

## 1. Unique constant onsite cubic form

Let

\[
V=\mathbb R_J^3\oplus\mathbb R_W^3
\]

and assume both triples transform by the same cubic vector representation,

\[
G(R)=\operatorname{diag}(R,R).
\]

Write a general real antisymmetric form in blocks,

\[
\Omega=\begin{pmatrix}A&C\\-C^{\mathsf T}&D\end{pmatrix}.
\]

Invariance under the proper signed-permutation group forces the axial vectors
dual to the skew blocks `A` and `D` to be invariant under every cubic
rotation. No nonzero such vector exists, so `A=D=0`. The cross block must
commute with every signed permutation. Axis sign flips kill its off-diagonal
entries and axis permutations make its diagonal entries equal. Hence

\[
\boxed{\Omega=c\Omega_0,\qquad
\Omega_0=\begin{pmatrix}0&I_3\\-I_3&0\end{pmatrix}.}
\tag{1}
\]

The form is nondegenerate exactly when `c` is nonzero. An independent exact
enumeration of the 24 proper cubic matrices gives constraint rank 14 on the
15 skew coefficients and nullity one. The same span is invariant under all
48 signed permutations.

Equation (1) is a classification theorem, not a derivation that `(J,W)` is a
canonical pair. It assumes a constant onsite bilinear form and equivalent
vector transformation laws. In particular, if one triple is assigned an
axial rather than polar transformation under improper operations, the stated
full-`O_h` representation is different and (1) does not follow as written.

## 2. Unconstrained defect-rank minima

In the radial/tangential eigenbasis the accepted genesis derivative is

\[
M=\operatorname{diag}(1,t,t,a,a,a),
\qquad 0<t<1,\quad 0\le a\le1.
\]

For any skew form,

\[
\Delta=\Omega-M^{\mathsf T}\Omega M,
\qquad
\Delta_{ij}=(1-m_i m_j)\Omega_{ij}.
\tag{2}
\]

### Repeated-eigenspace lemma

Suppose `rank Delta=2r` and write the skew matrix as a sum of `r` wedges.
Let `E_mu` be a `g`-dimensional eigenspace and assume
`1-mu m_j` is nonzero for every coordinate direction. If `g>2r`, a nonzero
vector in `E_mu` annihilates the restrictions of all `2r` wedge covectors.
It therefore annihilates `Delta`. Equation (2), with nonzero denominators,
then makes it annihilate `Omega`, contradicting nondegeneracy. Thus

\[
\operatorname{rank}\Delta\ge
\text{the least even integer not smaller than }g.
\tag{3}
\]

### Zero drain

At `a=1`, `det M=t^2<1`, so the defect cannot vanish for a nondegenerate
form. Its rank is therefore at least two. Pairing the directions
`(0,3)`, `(4,5)`, and `(1,2)` gives `det Omega=1` and

\[
\boxed{\min_\Omega\operatorname{rank}\Delta=2.}
\tag{4}
\]

### Generic positive drain

For `0<=a<1` and `a!=t`, the `a` eigenspace has dimension three and satisfies
the denominator condition, so (3) excludes rank two. Define

\[
r=\frac{1-at}{(1-a)(1+t)}
\]

and take the independent entries

\[
\Omega_{12}=\Omega_{34}=\Omega_{56}=\Omega_{14}=1,
\qquad \Omega_{23}=-r.
\]

Direct evaluation gives

\[
\det\Omega=\frac{(t-a)^2}{(1-a)^2(1+t)^2}>0,
\qquad \operatorname{rank}\Delta=4.
\]

Therefore

\[
\boxed{\min_\Omega\operatorname{rank}\Delta=4.}
\tag{5}
\]

### Degenerate positive drain

At `a=t`, the contracting eigenspace has dimension five. Equation (3)
requires rank at least six, and `Omega_0` attains it:

\[
\boxed{\min_\Omega\operatorname{rank}\Delta=6.}
\tag{6}
\]

## 3. Price of cubic covariance

For the cubic form (1), FTD-0572 gives defect rank four at zero drain and six
at positive drain. None of the 120 registered production arms has `a=t`.
Comparing with (4)--(5), cubic covariance increases the minimum defect rank
by exactly two in every arm:

\[
\boxed{\operatorname{rank}\Delta_{\rm cubic}
-\min_\Omega\operatorname{rank}\Delta=2.}
\tag{7}
\]

Because one canonical bath pair supplies two dimensions, the exact symmetry
price is one additional bath pair. The 30 degeneracy controls independently
close (6).

The lower-rank alternatives do not define one native phase-space geometry:
they depend on the event eigenbasis and, at positive drain, on `t` and `a`.
They are branchwise counterexamples used to establish the price in (7), not
candidate global canonical structures.

## 4. What is and is not established

- The standard `(J,W)` pairing is no longer arbitrary within the constant,
  onsite, equivalent-vector cubic class.
- The FTD-0572 two-/three-pair count is the cubic-covariant count.
- Dropping cubic covariance reduces the branchwise count to one/two pairs,
  except at `a=t`, where three remain necessary.
- FTD-0574 subsequently derives the native free-field discrete action and
  identifies `W` as the discrete Legendre momentum of `J`.
- No genesis Hamiltonian, bath ontology, reset mechanism, or active energy
  transport has been derived.
- Derivative-dependent, field-dependent, nonlocal, nonlinear, and
  presymplectic forms remain outside the theorem.

## 5. Reproducibility

- preregistration:
  `preregistrations/PREREG_GENESIS_CUBIC_CANONICAL_FORM_v1.md`;
- independent proof:
  `scripts/proofs/proof_genesis_cubic_canonical_form.py`;
- observer implementation:
  `engine/include/ftd/eft/genesis_cubic_canonical_form.h` and
  `engine/src/eft/genesis_cubic_canonical_form.cpp`;
- native test:
  `engine/tests/test_genesis_cubic_canonical_form.cpp`;
- run of record:
  `engine/results/ftd_0573/windows_msvc_cpu.json`.

No production state, tick phase, toggle, force, or default is changed.
