# Clock-compatible Hodge-triad collision and acoustic boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — MINIMAL CLOCK-INVARIANT POLAR/AXIAL READOUT]** +
**[THEOREM — EXACT $O_h\times C_{12}$ SEVEN-INVARIANT COLLISION]** +
**[CLOSED NEGATIVE, SCOPED — TRIAD-AVERAGED MAXWELL/HODGE CONE]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificates:**

- [proof_clock_compatible_hodge_triad_readout.py](../../../../../scripts/proofs/proof_clock_compatible_hodge_triad_readout.py):
  9,452 exact checks;
- [proof_clock_compatible_hodge_triad_equivariant_collision.py](../../../../../scripts/proofs/proof_clock_compatible_hodge_triad_equivariant_collision.py):
  73,403 exact checks; and
- [proof_clock_compatible_hodge_triad_bloch_boundary.py](../../../../../scripts/proofs/proof_clock_compatible_hodge_triad_bloch_boundary.py):
  exact product-reference and first-order Bloch closure.

No measured coefficient, dispersion fit, or physical target search enters any
certificate.

---

## 1. Minimal clock-compatible readout

For $f=(d,n,h)$ define $c=d\times n$. The polar legs

\[
 d,\qquad hn,\qquad c                              \tag{1}
\]

cycle under the shared-edge flag tick. Their sum and axial partner are

\[
 \boxed{
 \widetilde E=d+hn+c,
 \qquad
 \widetilde B=h\widetilde E.}                     \tag{2}
\]

Equation (2) is invariant under the complete internal C3 flag turn and C4
phase advance. Under $R\in O_h$,

\[
 \widetilde E\mapsto R\widetilde E,
 \qquad
 \widetilde B\mapsto\det(R)R\widetilde B.         \tag{3}
\]

The 192 one-particle states realize sixteen $(\widetilde E,\widetilde B)$
types, each twelve times. In the registered linear triad×phase ansatz, the
combined shift on three legs and four phases is one twelve-cycle. Its fixed
coefficient space is one-dimensional for each vector type, proving minimality
within that ansatz.

Record number plus the six components of equation (2) have rank seven and
remain a seven-dimensional clock orbit. By contrast, the former
phase-weighted fixed-frame readout generates 37 rows under the clock.

## 2. Complete relation and deterministic collision

All 18,336 unordered pairs form 117 exact field sectors with histogram

\[
 \{66^{16},144^{88},288^{12},1152^1\}.             \tag{4}
\]

Their complete transition relation has rank 185 and exactly seven additive
invariants.

Under the full $O_h\times C_{12}$ action, the pair space decomposes into 47
orbits:

\[
 \{96^3,192^{10},288^{12},576^{22}\}.              \tag{5}
\]

Every orbit admits a field-preserving fixed-point-free self-involution. A
deterministic exact rank-greedy choice produces one global collision $C$ with

\[
 C^2=1,
 \qquad C(gz)=gC(z),
 \qquad \operatorname{rank}T_C=185                \tag{6}
\]

for every $g\in O_h\times C_{12}$. Its only additive invariants are record
number and $(\widetilde E,\widetilde B)$.

## 3. Exact propagation closure

Clock compatibility passes, but the first-order Bloch generator does not have
the Hodge form. In conserved-variable order

\[
 (n,E_x,E_y,E_z,B_x,B_y,B_z),                     \tag{7}
\]

the exact axis generators have only

\[
 A_a(0,E_a)=A_a(E_a,0)=\frac13.                   \tag{8}
\]

For general $k$,

\[
 \chi_{-iA(k)}(\lambda)
 =\lambda^5\left(\lambda^2+\frac{|k|^2}{9}\right). \tag{9}
\]

Thus there is one scalar--longitudinal electric wave with speed $1/3$. Along
$k\parallel z$, the entire $(E_x,E_y,B_x,B_y)$ transverse block is exactly
zero. There is no antisymmetric $E\leftrightarrow B$ curl pair and hence no
Maxwell cone.

## 4. Structural lesson and next gate

The two closures bracket the design problem:

1. preserving fixed edge/face quadratures gives the right field types but is
   incompatible with the internal clock; while
2. averaging the complete triad restores clock compatibility but collapses
   oriented circulation into a BCC ray label and yields longitudinal
   acoustics.

A surviving finite carrier must therefore retain an **edge--face stagger or
cotangent orientation** that is clock-compatible while keeping a nonzero
antisymmetric first spatial moment. The next certificate must construct that
carrier algebraically and test its exact $k$-linear block before any source,
work, or fine-structure interpretation is attached.

