# Cotangent rank-20 chiral commutant and parity-pair price v1

**Date:** 2026-08-24

**Status:** **[THEOREM — SPATIAL-GENERATOR COMMUTANT DIMENSION FOUR]** +
**[THEOREM — PARITY-EXCHANGED TEN-DIMENSIONAL SECTORS]** +
**[THEOREM — CHIRAL TT-SEED LEAKAGE]** +
**[CONDITIONAL — EIGHT-DIMENSIONAL REDUCTION PER PARITY SECTOR]** +
**[OPEN — NATIVE CONSTRAINT/REALITY ALGEBRA]**

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_cotangent_rank20_chiral_commutant_parity_pair.py](../../../../../scripts/proofs/proof_cotangent_rank20_chiral_commutant_parity_pair.py)
performs 1,007 exact checks, including all 98 registered nonzero primitive
wavevectors.

**C4 scope:** the two rank-ten sectors live inside one fixed C4 quadrature's
tensor-20 closure. Restoring the native quadrature pair gives two C4-related
copies and raises the phase-complete tensor carrier to rank forty.

---

## 1. Question left by the constraint count

The
[rank-twenty constraint-count theorem](THEOREM_COTANGENT_RANK20_CONSTRAINT_COUNT_OBSTRUCTION_v1.md)
proved that the physical reduction must remove sixteen phase-space
dimensions. The present theorem asks whether that price has an internal
structural decomposition or is merely a raw count.

Let $A_x,A_y,A_z$ be the three co-rotating first moments of the selected
rank-twenty carrier. Their exact commutant is

\[
 \operatorname{Comm}(A_x,A_y,A_z)
 =\{X:[X,A_a]=0\ \forall a\}.                      \tag{1}
\]

The $1200\times400$ commutator system has rank 396, hence

\[
 \boxed{\dim\operatorname{Comm}=4.}                \tag{2}
\]

---

## 2. Canonical involution

In the ordered moment basis

\[
 (S,hS,S',hS'),                                    \tag{3}
\]

where the primed tensors are the retained collision copies, define

\[
 Q=
 \begin{pmatrix}
 0&I_5&0&0\\
 I_5&0&0&0\\
 0&0&0&I_5\\
 0&0&I_5&0
 \end{pmatrix}.                                    \tag{4}
\]

The certificate proves

\[
 Q^2=I,\qquad Q^TG^{-1}Q=G^{-1},\qquad[Q,A_a]=0.   \tag{5}
\]

Therefore

\[
 P_\pm={I\pm Q\over2}                              \tag{6}
\]

are exact energy-orthogonal invariant projectors of rank ten.

These are not inserted TT projectors. They are elements of the exact
commutant of the selected finite carrier.

---

## 3. Parity exchanges the sectors

Let $\Pi$ be inversion induced from the finite cotangent flag action. Then

\[
 \Pi^2=I,\qquad \Pi^TG^{-1}\Pi=G^{-1},              \tag{7}
\]

and

\[
 \boxed{\Pi Q=-Q\Pi.}                              \tag{8}
\]

Consequently

\[
 \Pi P_\pm=P_\mp\Pi.                               \tag{9}
\]

The two ten-dimensional sectors are parity partners. Selecting only one
would break inversion symmetry; a parity-complete action must retain both or
provide a declared reality/identification law relating them.

This motivates the term **chiral sectors**, but no physical helicity
identification is promoted by this theorem.

---

## 4. Sector spectra

For every registered nonzero primitive wavevector, the restricted operators

\[
 A_\pm(k)=P_\pm A(k)P_\pm                          \tag{10}
\]

have

\[
 \operatorname{rank}A_\pm(k)=8,\qquad
 \operatorname{nullity}A_\pm(k)=2.                 \tag{11}
\]

Their exact characteristic polynomials agree:

\[
 \chi_+(k,\lambda)=\chi_-(k,\lambda),              \tag{12}
\]

and multiply to the full rank-twenty polynomial:

\[
 \chi_{20}=\chi_+\chi_-.                           \tag{13}
\]

Thus the doubled spectrum in the rank-twenty theorem is precisely a
parity-paired duplication at the level of the selected first moment.

---

## 5. TT seed by sector

The parity-complete four-dimensional TT seed splits exactly:

\[
 \dim P_+V_{\rm TT}
 =\dim P_-V_{\rm TT}=2.                            \tag{14}
\]

But neither two-dimensional seed is invariant. Its Krylov closure dimensions
are:

| Wavevector | $\dim\mathcal K_+$ | $\dim\mathcal K_-$ |
|---|---:|---:|
| $(1,0,0)$ | 4 | 4 |
| $(1,1,1)$ | 8 | 8 |
| $(1,2,3)$ | 9 | 9 |

So the commutant decomposition explains the doubling but does not remove the
helicity-zero/one or collision-copy contamination.

---

## 6. Constraint price by parity sector

If the final physical kernel is:

1. four-dimensional;
2. invariant under $Q$; and
3. parity complete,

then equation (9) forces equal physical dimensions in the two sectors:

\[
 \dim\mathcal H_{\rm phys,+}
 =\dim\mathcal H_{\rm phys,-}=2.                   \tag{15}
\]

Each ten-dimensional sector must therefore lose eight dimensions:

\[
 10-2=8,                                          \tag{16}
\]

and parity doubles this:

\[
 8+8=16.                                          \tag{17}
\]

This recovers the independent Dirac-count price $2F+S=16$ as two
parity-exchanged eight-dimensional reductions.

---

## 7. Exact scope

The theorem proves a finite invariant decomposition. It does not prove:

1. that $Q$ is a microscopic observable rather than a blocked commutant;
2. that either sector is a physical helicity sector;
3. a reality condition relating the sectors;
4. a first-/second-class constraint algebra;
5. an isolated tensor pole;
6. compatibility with the rank-thirty common Maxwell closure;
7. static gravity or lensing; or
8. any coupling normalization.

Production remains unchanged and class 0.

The
[rank-thirty common irreducibility successor](THEOREM_COTANGENT_RANK30_COMMON_IRREDUCIBILITY_AND_PARITY_INDEX_OBSTRUCTION_v1.md)
answers item 6 negatively for a constant extension. Once the independent
Maxwell-10 collision closure is retained, the common first-moment commutant
collapses from dimension four to the scalar identity. Thus the tensor-only
$Q$ sectors are diagnostic structure, not projectors that can be carried
unchanged into the unified action.
The successor's coprime normalized axis/body-diagonal spectra further show
that constraints alone cannot repair the selected common generator into an
exact isotropic cone.

---

## 8. Next locked gate

Construct a parity-complete common collision whose constraint algebra:

1. first supplies a repaired layer-covariant common symbol with
   direction-stable Maxwell and tensor factors, then replaces the tensor-only
   $Q$ split by a momentum-dependent common constraint/reality complex;
2. reduces the rank-thirty carrier to eight physical phase dimensions,
   satisfying $2F+S=22$;
3. recovers two Maxwell and two tensor polarizations without inserting a
   constant projector or retaining the FCC directional defect;
4. retains the dual-A9 permission and work ledgers; and
5. couples the actualization source without exciting the excluded modes.
