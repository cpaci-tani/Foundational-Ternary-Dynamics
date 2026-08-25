# Hodge-flag pair collision invariant space and equivariant-matching boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT 192-STATE PARITY-TWISTED FIELD CARRIER]** +
**[THEOREM — COMPLETE SEVEN-DIMENSIONAL ADDITIVE-INVARIANT SPACE]** +
**[THEOREM — ABSTRACT INVOLUTIVE MATCHINGS EXIST IN EVERY FIELD SECTOR]** +
**[THEOREM — CUBIC-EQUIVARIANT DETERMINISTIC COLLISION EXISTS]** +
**[OPEN — C4/TIME-REVERSAL AND TRANSPORT COMPATIBILITY]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_hodge_flag_pair_collision_invariant_space.py](../../../../../scripts/proofs/proof_hodge_flag_pair_collision_invariant_space.py)
performs 9,233 exact checks, enumerates all 192 one-particle states and 18,336
unordered pairs, verifies full signed-cubic parity, and computes an exact
18,263-by-192 rational transition rank. No physical target or fitted
coefficient is used.

---

## 1. Why this collision space is needed

The
[shared-edge Hodge flag](THEOREM_SHARED_EDGE_HODGE_FLAG_BCC_PROPAGATION_AND_MAXWELL_REDUCTION_BOUNDARY_v1.md)
supplies exact finite propagation but leaves sixteen independent ballistic
flag cycles. The
[Hodge-Maxwell target](THEOREM_ORIENTED_BOND_PLAQUETTE_HODGE_MAXWELL_TARGET_AND_FINITE_LIFT_BOUNDARY_v1.md)
requires only a polar electric triplet, an axial magnetic triplet, and two
longitudinal constraints.

The collision problem is therefore precise: can the finite flag alphabet mix
away its ray/controller detail while protecting exactly those six field
components?

---

## 2. Parity-twisted C4 ownership

Take one of the 48 finite flags

\[
 f=(d,n,h),                                         \tag{1}
\]

with polar tangent $d$, axial normal $n$, and handed flag $h$. Attach one C4
phase

\[
 i^p=u_p+iv_p,
 \qquad p\in\mathbb Z_4.                            \tag{2}
\]

There are $48\times4=192$ one-particle states. Define the parity-twisted field
value

\[
 \boxed{
 E(f,p)=u_p d,
 \qquad B(f,p)=v_p n.}                              \tag{3}
\]

Thus even C4 phases own signed polar-edge content and odd phases own signed
axial-face content. Under every $R\in O_h$,

\[
 E\mapsto RE,
 \qquad B\mapsto\det(R)RB.                         \tag{4}
\]

The certificate verifies equation (4) on all 192 states and all 48 signed
cubic transformations.

Equation (3) is a candidate field readout, not yet a claim that a bare onsite
phase shift is the physical Maxwell tick. The prior Bloch closure already
proved that geometry must move with phase.

---

## 3. Complete two-record field sectors

Enumerate every unordered pair of distinct one-particle states and group pairs
by the exact six-component total

\[
 \mathcal Q(z)=\bigl(E_1+E_2,\;B_1+B_2\bigr).       \tag{5}
\]

The exact census is

\[
 {192\choose2}=18336\text{ pair states},           \tag{6}
\]

distributed among only 73 field sectors. Their size histogram is

\[
 \boxed{
 \{120^{12},\;256^{60},\;1536^1\}.}                \tag{7}
\]

Every sector has even size. Therefore every sector admits an abstract
fixed-point-free involutive matching that preserves equation (5).

This existence statement alone is not yet a physical collision: an arbitrary
lexicographic matching would privilege a coordinate presentation. The
subsequent
[equivariant-collision theorem](THEOREM_HODGE_FLAG_OH_EQUIVARIANT_SEVEN_INVARIANT_COLLISION_AND_FULL_TICK_BOUNDARY_v1.md)
constructs one matching commuting with $O_h$; C4/time-reversal and transport
compatibility remain open.

---

## 4. Complete additive-invariant theorem

Let $a(f,p)$ be any real one-particle function. Require

\[
 a(s_1)+a(s_2)=a(s'_1)+a(s'_2)                     \tag{8}
\]

for every pair transition within any one sector of equation (5).

Using one reference pair per sector, the exact transition matrix has shape

\[
 18263\times192                                    \tag{9}
\]

and rank

\[
 \operatorname{rank}T=185.                         \tag{10}
\]

Hence

\[
 \dim\ker T=192-185=7.                             \tag{11}
\]

Seven explicit independent rows are

\[
 1,\quad E_x,E_y,E_z,\quad B_x,B_y,B_z.             \tag{12}
\]

Equations (10)--(12) prove

\[
 \boxed{
 \text{record number plus polar }E\text{ and axial }B
 \text{ span the complete additive invariant space}.}          \tag{13}
\]

No handedness count, individual C4 phase count, internal C3 cycle label, BCC
ray label, or extra vector/tensor moment is forced to remain gapless by the
complete field-preserving pair relation.

This pays the exact collision-invariant count required by the Hodge-Maxwell
target:

\[
 6\text{ field components}-2\text{ divergences}
 =4\text{ transverse phase-space dimensions}.      \tag{14}
\]

---

## 5. What this advances

The strict-discrete electromagnetic chain now contains all three structural
pieces separately:

1. **source type:** actualization injects a finite oriented phase token;
2. **transport type:** the shared-edge flag has an exact finite inverse and a
   nonflat BCC cone; and
3. **collision-invariant type:** the full flag-phase alphabet can protect
   exactly $E$ and $B$ with no unavoidable extra modes.

This is substantially narrower than saying “some collision might work.” The
remaining collision must choose an actual equivariant permutation inside the
73 sectors and show that its blocked streaming kernel is the signed Hodge
operator.

---

## 6. What remains open

### Equivariant matching

**[RESOLVED FOR $O_h$]:** the subsequent exact orbit census constructs a
single fixed-point-free involution commuting with all 48 signed cubic
transformations and preserving exactly the seven invariants in equation (12).
It also proves that three distinct-orbit exchanges are necessary beyond the
self-orbit option space. Compatibility with the parity-twisted C4/flag update
and time reversal is still open.

### Full-tick compatibility

The free flag tick changes $(d,n,p)$ together. Equation (3) is not conserved
record by record under that tick; Maxwell requires its **spatial differences**
to exchange $E$ and $B$. The collision, streaming, phase advance, and readout
must therefore be linearized as one complete tick. Proving equation (13) for a
static collision relation does not prove the Hodge curl.

### Work and sources

No capacity debit, reserve compensation, ternary manifestation, material
clock, or detector route has been composed with the pair relation. The result
contains no source force and no physical alpha.

---

## 7. Next locked gate

Decompose the 18,336 pair states into orbits of the full signed-cubic,
parity-twisted C4, and time-reversal action. For each of the 73 field sectors:

1. compute the stabilizer-fixed candidate partners;
2. determine whether an equivariant fixed-point-free matching exists;
3. if it exists, choose it without a physical target and prove the global
   permutation/inverse;
4. derive the exact product-reference collision kernel;
5. compose it with the shared-edge flag transport; and
6. test whether the four constrained slow modes reproduce the Hodge-Maxwell
   first-order generator rather than diffusion or excess ballistic rays.

Only that pass would produce a finite native electromagnetic action sector.
