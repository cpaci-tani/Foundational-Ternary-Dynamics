# Finite Cartesian linearization and tensor composition v1

**Date:** 2026-08-26
**Ledger:** FTD-1024
**Status:** **[THEOREM, CONDITIONAL — FINITE CARTESIAN JOINT ALTERNATIVES
LINEARIZE CANONICALLY TO THE HILBERT TENSOR PRODUCT]** +
**[FOUNDATIONAL PRICE — INDEPENDENT CONJUNCTION / JOINT-LABEL COMPLETENESS]** +
**[CLOSED NEGATIVE — LOCALITY AND INDEPENDENT PHASE ACTIONS ALONE DO NOT
FORCE TENSOR COMPOSITION]** + **[CONDITIONAL COROLLARY — ENTANGLED VECTORS
AND THE TSIRELSON OPERATOR BOUND]** + **[OPEN — PHYSICAL ENTANGLED
PREPARATION, LABORATORY BELL RECOVERY, EXCHANGE STATISTICS, CAR/Fock, QFT]**
**Constitutional status:** no new v3 postulate is adopted here
**Production status:** unchanged
**Exact certificate:**
[`proof_finite_cartesian_tensor_composition.py`](../../../../../scripts/proofs/proof_finite_cartesian_tensor_composition.py)

---

## 1. Question and verdict

The composition question is

\[
 \mathcal H_{AB}\stackrel{?}{=}\mathcal H_A\otimes\mathcal H_B
 \quad\hbox{rather than}\quad
 \mathcal H_A\oplus\mathcal H_B.
\]

The exact verdict has two parts.

1. Once independent simultaneous alternatives have the complete Cartesian
   type
   \(X_{AB}=X_A\times X_B\), the finite complex linearization used by the
   one-system FQCR calculus forces, canonically,

   \[
    \boxed{\mathbb C[X_A\times X_B]
    \simeq \mathbb C[X_A]\otimes\mathbb C[X_B].}
   \]

2. Locality and independently acting phase transformations do **not** force
   the Cartesian type. They also admit the direct-sum representation

   \[
    U_A\mapsto U_A\oplus I_B,
    \qquad
    U_B\mapsto I_A\oplus U_B.
   \]

Therefore the tensor product is a theorem after one explicit composition
price. It is not a theorem from A1--A7 alone.

---

## 2. The type that must be set

Let \(X_A\) and \(X_B\) be finite sets of resolved alternatives for two
systems. The needed composition type is:

> **IC — independent conjunction / joint-label completeness
> `[FOUNDATIONAL PRICE]`.** A simultaneous resolved alternative of \(AB\) is
> uniquely a pair \((a,b)\in X_A\times X_B\); every locally admissible pair
> occurs; distinct pairs remain distinct; and there are no additional
> unresolved joint labels outside this pair set.

In formulas,

\[
 \boxed{X_{AB}=X_A\times X_B.}                       \tag{1}
\]

The four clauses in IC matter:

- **every pair occurs** excludes a global constraint that removes product
  alternatives;
- **pairs remain distinct** excludes a quotient that identifies different
  local records;
- **no extra joint labels** excludes holistic sectors not spanned by local
  product preparations; and
- **simultaneous** distinguishes conjunction from the exclusive alternative
  represented by a disjoint union.

Calling systems “independent” does not prove equation (1) unless independence
is defined to include these clauses. Type-priority requires the price to be
visible rather than hidden in that word.

For v3 complete records on disjoint prepared regions, state completeness makes
equation (1) a natural candidate: a joint complete record is the pair of the
two regional complete records. It is not yet an unconditional physical result
for public alternatives. Ternary manifestation is many-to-one, expiry can
remove distinctions, and global admissibility constraints can correlate
regions. IC is therefore not silently added to P1--P5.

---

## 3. Finite complex linearization

For a finite alternative set \(X\), define its effective coherent
linearization

\[
 \mathcal L(X):=\mathbb C[X]
 =\left\{\sum_{x\in X}\psi_x|x\rangle:\psi_x\in\mathbb C\right\}, \tag{2}
\]

with the public alternative basis orthonormal:

\[
 \langle x|x'\rangle=\delta_{xx'}.                 \tag{3}
\]

This is the finite \(\mathbb C^N\) calculus already reached by the one-system
quarter-recurrence/norm argument. It is an effective amplitude space, not an
ontic complex field in v3.

---

## 4. Tensor-composition theorem

### Theorem

Let \(X_A,X_B\) be finite and assume IC. There is a unique unitary map

\[
 \boxed{
 \mathcal U_{A,B}:\mathcal L(X_A)\otimes\mathcal L(X_B)
 \longrightarrow \mathcal L(X_A\times X_B)}       \tag{4}
\]

such that

\[
 \mathcal U_{A,B}(|a\rangle\otimes|b\rangle)=|a,b\rangle. \tag{5}
\]

### Proof

The elementary tensors \(|a\rangle\otimes|b\rangle\) form an orthonormal
basis of the left-hand side. IC says that \(|a,b\rangle\), over the same
ordered pairs, is an orthonormal basis of the right-hand side. Equation (5)
therefore maps one orthonormal basis bijectively onto the other and extends
uniquely by linearity to a unitary map. \(\square\)

This is not a dimension fit. It is a basis-labelled natural isomorphism. In
particular, it remains structurally meaningful in the accidental case
\(2+2=2\cdot2\), where dimension alone cannot distinguish a direct sum from a
tensor product.

---

## 5. Product preparations, norm, and phases

For

\[
 |\psi\rangle=\sum_a\psi_a|a\rangle,
 \qquad
 |\phi\rangle=\sum_b\phi_b|b\rangle,
\]

the corresponding independent preparation is

\[
 |\psi\rangle\boxtimes|\phi\rangle
 =\sum_{a,b}\psi_a\phi_b|a,b\rangle.              \tag{6}
\]

The norm factors exactly:

\[
 \|\psi\boxtimes\phi\|^2
 =\sum_{a,b}|\psi_a|^2|\phi_b|^2
 =\|\psi\|^2\|\phi\|^2.                          \tag{7}
\]

Thus conditional Born weights for product preparations factor over joint
outcomes. This is a theorem in the effective calculus; it is not the v3
physical Born pushforward, whose preparation and detector gates remain open.

Independent diagonal phase actions satisfy

\[
 D_A(\theta)|a\rangle=e^{i\theta_a}|a\rangle,
 \qquad
 D_B(\varphi)|b\rangle=e^{i\varphi_b}|b\rangle,
\]

and equation (4) gives

\[
 (D_A\otimes D_B)|a,b\rangle
 =e^{i(\theta_a+\varphi_b)}|a,b\rangle.            \tag{8}
\]

The additive joint character is therefore downstream of Cartesian pair
labels and bilinear linearization. It does not independently select them.

---

## 6. Why the direct sum is the wrong composition type

Finite complex linearization sends a disjoint union to a direct sum:

\[
 \boxed{
 \mathcal L(X_A\sqcup X_B)
 \simeq\mathcal L(X_A)\oplus\mathcal L(X_B).}      \tag{9}
\]

The basis of equation (9) is labelled by

\[
 (A,a)\quad\text{or}\quad(B,b),                    \tag{10}
\]

not by a simultaneous pair \((a,b)\). A direct sum therefore represents a
tagged disjunction—“an A alternative or a B alternative”—rather than two
jointly present systems.

It is still a valid structure elsewhere. Superselection sectors and different
particle-number sectors are naturally combined by direct sums. The theorem
does not ban \(\oplus\); it assigns \(\oplus\) and \(\otimes\) to different
types.

---

## 7. Closed negative: phases and locality alone are insufficient

Let

\[
 \mathcal K=\mathcal H_A\oplus\mathcal H_B.        \tag{11}
\]

Define local actions

\[
 \rho_A(U)=U\oplus I_B,
 \qquad
 \rho_B(V)=I_A\oplus V.                            \tag{12}
\]

Then

\[
 [\rho_A(U),\rho_B(V)]=0,                          \tag{13}
\]

and both local phase groups act independently and norm-preservingly. This is
an exact countermodel to the claim

\[
 \text{locality + independent phases}\Longrightarrow\text{tensor product}.
\]

What fails is not locality. What fails is IC: equation (11) contains no
complete basis of simultaneous pair labels. Consequently the implication is
**[CLOSED NEGATIVE]** at the stated assumption set.

---

## 8. Associativity and many-body kinematics

The basis maps are canonically associative:

\[
 |a,b,c\rangle
 \longleftrightarrow
 (|a\rangle\otimes|b\rangle)\otimes|c\rangle
 \longleftrightarrow
 |a\rangle\otimes(|b\rangle\otimes|c\rangle),     \tag{14}
\]

and symmetric under exchange:

\[
 \Sigma_{A,B}|a,b\rangle=|b,a\rangle.              \tag{15}
\]

Therefore repeated IC composition gives the labelled many-body space

\[
 \boxed{\mathcal H_{1\cdots N}
 \simeq\bigotimes_{k=1}^N\mathcal H_k.}            \tag{16}
\]

No parenthesization becomes physical content.

---

## 9. Entanglement follows mathematically

If \(\dim\mathcal H_A,\dim\mathcal H_B\ge2\), the tensor product contains
non-simple vectors. For example,

\[
 |\Omega\rangle=|0,0\rangle+|1,1\rangle.           \tag{17}
\]

Its coefficient matrix has rank two, whereas every product vector
\(\psi_a\phi_b\) has coefficient-matrix rank at most one. Hence

\[
 \boxed{|\Omega\rangle\ne|\psi\rangle\otimes|\phi\rangle} \tag{18}
\]

for all local vectors. Entanglement is therefore available as mathematical
content of the composed effective calculus.

This does not show that homogeneous v3 \(\Phi\) prepares, preserves, or
measures an entangled physical state.

---

## 10. Tsirelson bound: conditional operator corollary

Let \(A_0,A_1\) and \(B_0,B_1\) be Hermitian local observables with spectra in
\([-1,1]\). On the tensor product define

\[
 \mathcal B=
 A_0\otimes(B_0+B_1)+A_1\otimes(B_0-B_1).          \tag{19}
\]

For dichotomic unitaries \(A_i^2=B_j^2=I\),

\[
 \mathcal B^2=4I-[A_0,A_1]\otimes[B_0,B_1].        \tag{20}
\]

Since each commutator has norm at most two,

\[
 \|\mathcal B\|^2\le8,
 \qquad
 \boxed{\|\mathcal B\|\le2\sqrt2}.              \tag{21}
\]

Contractions follow by the standard dilation/convexity extension. The Pauli
choices attain the bound in the effective Hilbert model.

This corollary does **not** promote FTD's physical Bell status. The prepared
source-complete, measurement-independent local v3 sector proved in
[`THEOREM_V3_BIPARTITE_PREPARED_BORN_NO_SIGNALLING_AND_LOCAL_CHSH_BOUNDARY_v1.md`](THEOREM_V3_BIPARTITE_PREPARED_BORN_NO_SIGNALLING_AND_LOCAL_CHSH_BOUNDARY_v1.md)
still obeys \(|S|\le2\). Moving from that finite physical sector to the full
tensor-operator correlation set requires an explicit preparation/readout
bridge, not vocabulary.

---

## 11. What does not yet follow

The composition theorem closes only finite labelled kinematics.

- **Bosons/fermions:** permutation covariance and the choice of symmetric or
  antisymmetric exchange sector are additional physical inputs.
- **Fock space:** once exchange statistics and a particle-number grading are
  supplied,

  \[
   \mathcal F_\pm(\mathcal H)
   =\bigoplus_{N\ge0}\operatorname{Sym/Alt}^N\mathcal H, \tag{22}
  \]

  but neither the grading nor the exchange choice is derived here.
- **CAR:** the fermionic sign/graded composition law is not forced by IC.
- **QFT:** local algebras, causal propagation, field dynamics, vacuum, scale
  recovery, and interacting renormalization remain separate gates.
- **Physical Bell violation:** an entangled source, setting carriers, local
  measurements, detector completeness, and the laboratory correlation law
  remain open under v3.

Thus the milestone implication must be read as

\[
 \boxed{
 \text{IC + one-system complex linearization}
 \Longrightarrow
 \text{tensor kinematics + mathematical entanglement + Tsirelson ceiling},}
\]

not as a derivation of CAR, QFT, or observed Bell violation.

---

## 12. Epistemic accounting

| Claim | Status |
|---|---|
| \(\mathbb C[X_A\times X_B]\cong\mathbb C[X_A]\otimes\mathbb C[X_B]\) | **[THEOREM]** |
| norm/Born-weight factorization for product vectors | **[THEOREM inside effective calculus]** |
| associativity and swap maps | **[THEOREM]** |
| non-simple vectors exist when both local dimensions exceed one | **[THEOREM]** |
| Tsirelson operator bound in the tensor Hilbert model | **[THEOREM / standard corollary]** |
| IC follows from A1--A7 | **[OPEN; not shown]** |
| locality + independent phases alone force tensor product | **[CLOSED NEGATIVE]** |
| IC is adopted as v3 P6/A8 | **[NOT ADOPTED]** |
| homogeneous v3 \(\Phi\) forms physical entangled preparations | **[OPEN]** |
| v3 reproduces laboratory Bell violation | **[OPEN; native local prepared sector has \(S\le2\)]** |
| exchange statistics / CAR / Fock / QFT | **[OPEN beyond the stated conditional constructions]** |

The new type-price is one composition declaration, IC. It is not a numerical
calibration, coupling insertion, or target fit.

---

## 13. Relation to reconstruction literature

This theorem is elementary finite linear algebra, not a claim that FTD is the
first framework to make composition an explicit principle. Representative
reconstruction programmes likewise place composite-system or local-
distinguishability assumptions on the books:

- L. Hardy, [*Quantum Theory From Five Reasonable Axioms*](https://arxiv.org/abs/quant-ph/0101012), especially the composite-system multiplication requirements.
- L. Masanes and M. P. Mueller, [*A derivation of quantum theory from physical requirements*](https://arxiv.org/abs/1004.1483), New J. Phys. 13, 063001 (2011).
- G. Chiribella, G. M. D'Ariano, and P. Perinotti, [*Informational derivation of Quantum Theory*](https://arxiv.org/abs/1011.6451), Phys. Rev. A 84, 012311 (2011), where local distinguishability is explicit.
- B. S. Cirel'son, [*Quantum generalizations of Bell's inequality*](https://doi.org/10.1007/BF00417500), Lett. Math. Phys. 4, 93--100 (1980).

FTD's specific contribution here is the type-priority diagnosis:

\[
 \boxed{
 \text{disjoint-union type}\mapsto\oplus,
 \qquad
 \text{Cartesian conjunction type}\mapsto\otimes.}
\]

The mathematics is forced after the type is set; the type is not bootstrapped
from one-system content.

---

## 14. Reproduction

```bash
python scripts/proofs/proof_finite_cartesian_tensor_composition.py
```

The certificate checks exact finite basis bijection, Gram preservation, norm
factorization, phase-character addition, associativity, swap, the direct-sum
countermodel, an entangled rank witness, and exact Pauli saturation of
\(2\sqrt2\). It is a regression certificate for the displayed algebra; the
general theorem is the basis proof in Section 4.
