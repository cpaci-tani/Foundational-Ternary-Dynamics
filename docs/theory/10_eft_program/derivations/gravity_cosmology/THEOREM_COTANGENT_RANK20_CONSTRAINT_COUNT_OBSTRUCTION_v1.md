# Cotangent rank-20 constraint-count obstruction v1

**Date:** 2026-08-24

**Status:** **[THEOREM — EXACT NONZERO-WAVEVECTOR NULLITY FOUR]** +
**[THEOREM — FOUR CONSERVED ROWS]** +
**[CONDITIONAL DIRAC COUNT — FOUR FIRST-CLASS CONSTRAINTS ARE INSUFFICIENT]** +
**[OPEN — NATIVE CONSTRAINT/GAUGE ALGEBRA]**

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_cotangent_rank20_constraint_count_obstruction.py](../../../../../scripts/proofs/proof_cotangent_rank20_constraint_count_obstruction.py)
performs 988 exact checks over all 98 registered nonzero primitive
wavevectors.

**C4 scope:** the \(2F+S=16\) price applies to the fixed-quadrature
tensor-20 carrier. The
[phase-complete common successor](THEOREM_COTANGENT_PHASE_COMPLETE_COMMON_CLOSURE_AND_C4_SELECTION_v1.md)
retains tensor-40 plus Maxwell-10 and has conditional common price
\(2F+S=42\), unless a native phase-reality quotient is first derived.

---

## 1. Correction to the four-constraint language

A symmetric tensor coordinate and its conjugate momentum form a
twelve-dimensional phase space. Four first-class linearized-gravity
constraints can reduce that familiar phase space to four physical dimensions.

The collision-closed carrier found in the
[rank-twenty theorem](THEOREM_COTANGENT_RANK20_COLLISION_CLOSURE_AND_TT_LEAKAGE_v1.md)
does not have dimension twelve. It has dimension twenty. Reusing the phrase
“four constraints” without paying for the eight collision-copy dimensions is
therefore insufficient.

---

## 2. Exact zero-mode census

For every registered nonzero primitive wavevector, the selected co-rotating
first moment satisfies

\[
 \operatorname{rank}A(k)=16,\qquad
 \operatorname{nullity}A(k)=4.                     \tag{1}
\]

There are also four left zero modes. With the induced positive moment metric
$G^{-1}$, every right zero mode $v$ supplies a conserved row:

\[
 A(k)v=0
 \quad\Longrightarrow\quad
 v^TG^{-1}A(k)=0.                                  \tag{2}
\]

Thus the linear witness has four exact conserved quantities at each nonzero
$k$.

This does not prove that they are first-class constraints. It proves only
their linear conservation.

---

## 3. Conditional Dirac count

For phase-space dimension $N$, with $F$ first-class and $S$ second-class
constraints, the reduced dimension is

\[
 N_{\rm phys}=N-2F-S.                              \tag{3}
\]

Two helicity-two polarizations plus their conjugates require

\[
 N_{\rm phys}=4.                                   \tag{4}
\]

With $N=20$, the required reduction is

\[
 \boxed{2F+S=16.}                                  \tag{5}
\]

Even under the optimistic assumption that all four conserved rows in
equation (2) become first-class,

\[
 20-2(4)=12\ne4.                                   \tag{6}
\]

The exact nonnegative solutions of equation (5) are

\[
 (F,S)=(0,16),(1,14),\ldots,(7,2),(8,0).           \tag{7}
\]

Therefore:

- a first-class-only reduction requires at least $F=8$;
- retaining only four first-class constraints additionally requires $S=8$;
  or
- an equivalent eight-dimensional collision-copy reduction must occur before
  the familiar four-constraint tensor phase space is reached.

---

## 4. Consequence

The four zero modes are not enough to turn the rank-twenty carrier into an
isolated spin-2 sector:

\[
 \boxed{
 \text{four conserved rows}
 \not\Rightarrow
 \text{helicity-two physical phase space}.}        \tag{8}
\]

This corrects the prior shorthand “derive four constraints.” The actual gate
is:

> derive a total sixteen-dimensional phase-space reduction, with a declared
> first-/second-class or gauge/redundancy interpretation, from the finite
> action.

No projector may be inserted after inspecting the spectrum.

---

## 5. Exact scope

This theorem is dimension and linear-algebra accounting. It does not:

1. classify the four conserved rows as first-class;
2. construct Poisson brackets or gauge generators;
3. prove the rank-twenty carrier is symplectic;
4. recover a physical tensor pole;
5. preserve the common Maxwell sector;
6. derive static gravity or lensing; or
7. derive any coupling normalization.

Production remains unchanged and class 0.

---

## 6. Next locked gate

The next finite collision must either:

1. reduce the common carrier to a twelve-dimensional invariant tensor
   phase-space subcarrier before the GR-like four-constraint stage; or
2. directly generate a constraint/gauge system satisfying $2F+S=16$ on the
   rank-twenty tensor carrier.

It must do so while preserving the enlarged common Maxwell closure, Gauss,
the dual-A9 work/permission ledger, and the actualization source.

The
[chiral commutant successor](THEOREM_COTANGENT_RANK20_CHIRAL_COMMUTANT_AND_PARITY_PAIR_PRICE_v1.md)
explains the sixteen-dimensional price internally. The spatial generators
split into two invariant rank-ten sectors exchanged by inversion. A
parity-complete four-dimensional physical kernel would contain two dimensions
from each sector, requiring an eight-dimensional reduction per parity
partner.
