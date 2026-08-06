# Relational matter ontology lower bound v1

**Ledger ID:** FTD-0741  
**Status:** `[DERIVED — ASSEMBLED CONDITIONAL LOWER BOUND] + [SYNTHESIS]`  
**Date:** 2026-07-29  
**Production status:** unchanged

## 1. Question

FTD-0740 defines a candidate matter ontology and a roadmap. This document asks
a narrower question:

> Which parts of that candidate are already forced by exact results, and which
> remain selected?

The answer is a lower bound on information and organization. It does not prove
that a stable matter object exists.

## 2. Assumptions and source theorems

The result is conditional on the following explicitly scoped premises.

1. **Mobile identity premise.** One candidate object is to retain identity
   while its manifestation anchors change.
2. **Selected current complex.** Matter sources the registered oriented-face
   field through a continuity complex with nontrivial cycle space
   (FTD-0719).
3. **Reciprocal matter--field coupling.** In a differentiable sector, complete
   matter and field variables obey a coupled Markov map. The field can affect
   later matter on the relevant subspace (FTD-0667's stated condition).
4. **Current field topology.** The fixed-source field coordinates are ordinary
   real arrays with the frozen Gauss/harmonic constraints (FTD-0584).
5. **Selected compact formation branch.** For the FTD-0721 compact pair,
   interaction support turns on at the force-continuous cutoff and the complete
   energy ledger has only pair and matched-field sectors (FTD-0738).

Premises 2--5 are not claims about all possible matter theories. Removing a
premise removes only the corresponding conclusion below.

## 3. Lower-bound results

### L1. Fixed-site identity is impossible for a mobile object `[DERIVED]`

Let `a_n` be a manifested site anchor. If identity were the value of one fixed
site, then an anchor-changing motion `a_{n+1} != a_n` would either destroy the
object or create a numerically distinct object. This contradicts the mobile
identity premise. Therefore the identity carrier must be invariant under
anchor replacement: a relation, extended pattern, or exactly equivalent
state—not a permanent voxel.

This is a logical consequence of what “the same object moves” means. It does
not prove that the engine contains such an object.

### L2. Manifestation snapshots do not determine the transaction `[THEOREM COROLLARY]`

FTD-0719 supplies two causal currents between the same unordered signed
endpoint densities with

\[
D(I_1-I_2)=0,\qquad I_1-I_2\ne0,
\qquad C^T(I_1-I_2)\ne0.
\]

The endpoint snapshots determine only the divergence of current. Because the
cycle difference changes the transverse field update, a snapshot-only Markov
state cannot represent both histories.

Consequently a complete theory must contain one of:

1. a unique action-selected correspondence/current;
2. explicit current or internal-phase information;
3. connection/holonomy information carrying the same cycle class.

Persistent constituent labels are one possible representation, not a theorem-
forced ontology.

### L3. Reciprocal field state or memory-equivalent information is necessary `[CONDITIONAL THEOREM COROLLARY]`

For the linearized complete map

\[
\binom{x_{n+1}}{y_{n+1}}=
\begin{pmatrix}A&B\\C&D\end{pmatrix}\binom{x_n}{y_n},
\]

FTD-0667 proves the exact matter-only reduction

\[
x_{n+1}=Ax_n+BD^ny_0+
\sum_{m=0}^{n-1}BD^{n-1-m}Cx_m.
\]

If two allowed states have the same matter coordinate but
`B delta y != 0`, their next matter coordinates differ. The matter projection
is then not a complete first-order Markov state. An adequate ontology must
retain the relevant field state, the resulting memory, or an injective
equivalent.

The conclusion is conditional on relevant reciprocal coupling. It does not
claim every formal field coordinate is part of one object's bound identity;
outgoing environmental modes may be external while still belonging to the
complete universe state.

### L4. Current ordinary-real fields do not topologically protect the object `[THEOREM COROLLARY]`

FTD-0584 proves every nonempty fixed-source/fixed-harmonic real-field fibre is
affine and contractible, including finite-support and finite-energy uncontained
spaces. The frozen vacuum manifold is a point. Therefore the current field
variables supply no localized integer homotopy sector that can protect matter.

Any surviving object in these variables must be dynamically invariant or
metastable. A protected defect requires an explicitly enlarged compact,
constrained, singular, or otherwise noncontractible state space. Compactness
alone still does not supply mobility, energy stability, or charge.

### L5. Interaction support is not formation `[THEOREM COROLLARY FOR THE SELECTED PAIR]`

For the selected compact potential,

\[
U(d_c)=U'(d_c)=0,\qquad d_c=3/2.
\]

A moving crossing therefore has `E_pair=K>0` at entry and stays positive on a
nonzero interval. If it later reaches `E_pair<0` in the closed pair--field
ledger, the field must gain the compensating energy. Thus adjacency, graph
membership, and first contact are not object predicates. Formation is an
energy-routing transition into a persistent family.

This conclusion is exact only for the selected potential/action. The broader
requirement—that a proposed formation predicate distinguish mere contact from
durable identity—belongs to the FTD-0740 charter rather than this theorem.

## 4. Assembled lower bound

Under the premises above, any adequate ontology for the current branch must be:

1. **relational or extended:** identity cannot reside in one permanent site;
2. **transaction-complete:** endpoint manifestation alone cannot select all
   physically distinct currents;
3. **field-complete or memory-complete:** reciprocal environmental state cannot
   generally be projected away;
4. **dynamically stabilized:** existing ordinary-real fields provide no local
   topological protection;
5. **formation-aware:** encounter topology must be distinguished from entry
   into a persistent energetic/dynamical family.

Symbolically, the lower bound is not a unique type equation but an information
requirement:

\[
X_{\rm adequate}\succeq
\{\text{manifestation},\ \text{transaction selector},\
\text{reciprocal field/memory},\ \text{relational identity}\}.
\]

The symbol `succeq` means “contains enough information to reconstruct,” not
“must use these exact C++ records.”

## 5. What remains selected

The lower bound does **not** force:

- finite labeled constituents or the existing `C` representation;
- the quadratic coat, compact pair potential, its depth or cutoff;
- face-electric/edge-magnetic variables as the unique representation;
- negative energy as a universal definition;
- an internal phase primitive;
- a compact `U(1)` connection;
- topological stability;
- charge, mass, spin, statistics, species, or a particle pole.

Those remain the discrimination targets of FTD-0740 Tracks A and B.

## 6. Falsifiers and escape routes

- L1 is escaped only by denying persistent mobile identity.
- L2 is escaped by proving the common action uniquely selects the cycle class
  from the committed state; that satisfies rather than contradicts the
  information lower bound.
- L3 is escaped on a sector where every relevant `BD^jC` vanishes and field
  preparation is fixed, or by an exact finite Markov closure equivalent to the
  eliminated field.
- L4 is escaped by changing the field target/configuration space or by seeking
  dynamical rather than topological stability.
- L5 is scoped away by changing the selected interaction; the replacement
  still needs an independently justified formation criterion.

## 7. Verification

The independent exact-arithmetic certificate freezes the four source-theorem
hashes and checks representative algebra for cycle-current ambiguity, the
matter--field memory kernel, affine-fibre contraction, and the compact-pair
boundary/energy-routing identities. It performs no parameter search or
physical-constant comparison.

