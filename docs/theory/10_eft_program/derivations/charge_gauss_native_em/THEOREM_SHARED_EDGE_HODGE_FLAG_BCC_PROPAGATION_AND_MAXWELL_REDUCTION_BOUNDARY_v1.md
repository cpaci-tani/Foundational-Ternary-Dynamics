# Shared-edge Hodge flag BCC propagation and Maxwell reduction boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT SHARED-EDGE CENTRALIZER/HANDEDNESS PRICE]** +
**[THEOREM — EXACT FINITE CUBIC-COVARIANT HODGE-FLAG PERMUTATION]** +
**[THEOREM — EXACT SC-TO-BCC THREE-TICK PROPAGATION AT SPEED
$1/\sqrt3$]** +
**[REFERENCE CONSTRUCTION — FINITE TRANSPORT HALF OF A HODGE LIFT]** +
**[BOUNDARY — SIXTEEN BALLISTIC FLAG RAYS, NOT TWO MAXWELL MODES]**  
**Production status:** unchanged  
**Ledger status:** no row minted; no physical $c$ identification

**Exact certificate:**
[proof_shared_edge_hodge_flag_bcc_propagation.py](../../../../../scripts/proofs/proof_shared_edge_hodge_flag_bcc_propagation.py)
performs 3,340 exact checks. It classifies every directed-edge stabilizer,
verifies the 48-state flag permutation under all 48 signed cubic
transformations, proves its inverse and finite-box bijection, enumerates all
orbits and BCC displacements, and derives each Laurent Bloch polynomial. No
physical target, fit, or numerical eigensolver is used.

---

## 1. The shared-edge routing obstruction

Fix a directed polar SC edge $d$. Four axial face normals $n\perp d$ meet at
that edge. The eight-element cubic stabilizer of $d$ acts on those four
normals as $D_4$.

The certificate enumerates all $4!$ possible normal permutations and proves
that the centralizer of this stabilizer action contains only

\[
 \{\mathrm{id},\,n\mapsto-n\}.                     \tag{1}
\]

Neither quarter turn is equivariant in the absence of a handed datum. Thus a
context-free fixed shared-edge rule can only retain or antipode the face
normal; it cannot choose left versus right around the edge.

Introduce one handed sign $h$ with pseudoscalar transformation law

\[
 h\mapsto\det(R)h.                                  \tag{2}
\]

Then $h$ reverses exactly when an improper transformation exchanges the two
quarter-turn senses. This is the finite routing datum required by the
stabilizer classification.

Equation (2) is a flag-orientation label in this construction. It is not yet
identified with the scalar orientation $\epsilon$ of the actualization token.
That identification would require a transformation-law and ownership proof.

---

## 2. The finite Hodge flag

A flag is

\[
 f=(d,n,h),                                         \tag{3}
\]

where

- $d$ is a polar directed SC tangent;
- $n$ is a perpendicular axial directed SC face normal; and
- $h\in\{-1,+1\}$ is the handed flag.

There are

\[
 6\times4\times2=48                                \tag{4}
\]

such flags. Under $R\in O_h$,

\[
 (d,n,h)\mapsto
 (Rd,\det(R)Rn,\det(R)h).                           \tag{5}
\]

Define the parity-twisted local update

\[
 \boxed{
 T(d,n,h)=\bigl(hn,\;h(d\times n),\;h\bigr).}       \tag{6}
\]

The first output is polar because pseudoscalar times axial is polar. The
second is axial because $d\times n$ is polar and multiplication by $h$ twists
its parity.

The certificate proves exactly

\[
 T^3=I,                                             \tag{7}
\]

and

\[
 T(Rf)=R(Tf)                                        \tag{8}
\]

with the transformations in equation (5), for all 48 flags and all 48 cubic
transformations.

---

## 3. One native streaming tick

At each global tick, first stream the flag one SC step along its current
tangent $d$, then apply equation (6):

\[
 (x,f,p)\mapsto(x+d,Tf,p+1\bmod4).                 \tag{9}
\]

Here $p$ is the already available common C4 phase. Equation (9) is an exact
permutation on every finite periodic box. Its inverse reconstructs the prior
flag with $T^2$, subtracts its tangent step, and decrements $p$.

Thus equation (9) is a strict finite local transport rule with:

- no copied token;
- no stochastic branch;
- exact inverse;
- one-site causal propagation per global tick; and
- simultaneous C3 frame cycling and C4 phase advance.

The internal frame and phase return together after

\[
 \operatorname{lcm}(3,4)=12                       \tag{10}
\]

ticks, while the spatial position continues to advance.

---

## 4. Exact SC-to-BCC bridge

Starting from $f_0=(d,n,h)$, the three successive SC tangents are

\[
 d,qquad hn,qquad d\times n.                     \tag{11}
\]

They are mutually orthogonal unit coordinate directions. The three-tick
displacement is

\[
 \boxed{
 \Delta(f)=d+hn+d\times n.}                        \tag{12}
\]

Every component of $\Delta$ is $\pm1$, so

\[
 \Delta\in\{(\pm1,\pm1,\pm1)\},                   \tag{13}
\]

the eight directed BCC body diagonals. The exact census is:

\[
 48\text{ flags}
 \longrightarrow16\text{ internal 3-cycles}
 \longrightarrow8\text{ BCC directions},          \tag{14}
\]

with two cycles per BCC direction and six flags per direction.

The coarse path speed is

\[
 \boxed{
 v_{\rm flag}^2={|\Delta|^2\over3^2}
 ={3\over9}={1\over3},
 \qquad v_{\rm flag}={1\over\sqrt3}.}              \tag{15}
\]

Equation (15) is derived from three orthogonal unit hops in three ticks. Its
equality to the engine's current lattice-unit `C_SPEED=1/sqrt(3)` is an exact
internal consistency observation, not a derivation that the flag is physical
light or that the engine constant is uniquely selected by this rule.

This supplies a literal finite bridge

\[
 \text{SC edge transactions}\longrightarrow
 \text{BCC coarse propagation}.                    \tag{16}
\]

---

## 5. Exact Bloch rays

For one internal three-cycle, let $X^\Delta$ denote the Bloch monomial of the
BCC displacement. The exact $3\times3$ transfer matrix obeys

\[
 P(k)^3=X^\Delta I_3,                              \tag{17}
\]

with characteristic polynomial

\[
 \boxed{
 \chi_P(\lambda)=\lambda^3-X^\Delta.}              \tag{18}
\]

Each branch has quasiphase

\[
 \omega_m(k)={k\cdot\Delta+2\pi m\over3},
 \qquad m=0,1,2,                                    \tag{19}
\]

and group velocity

\[
 \nabla_k\omega_m={\Delta\over3}.                 \tag{20}
\]

Unlike an isolated plaquette loop, equation (18) is not flat. Shared-edge
Hodge routing has produced exact finite propagation and a nonzero first-order
cone along eight BCC rays.

---

## 6. Why this is still not Maxwell

The Hodge-Maxwell target requires exactly two transverse polarization pairs
with a divergence constraint. Equation (9) instead contains sixteen
independent ballistic flag cycles. It has:

- no local collision mixing the eight ray directions;
- no protected polar/axial field moments derived from the flag ensemble;
- no Gauss constraint or magnetic-divergence identity at the finite-token
  level;
- no positive shared capacity/work ledger; and
- no source coupling to the actualization vertex.

Therefore:

\[
 \boxed{
 \text{finite Hodge transport passed; Maxwell mode reduction open.}} \tag{21}
\]

Calling $v_{\rm flag}$ the speed of light would be premature. It is the speed
of the registered microscopic flag rays.

---

## 7. Relevance to the unified action

Equation (9) is the first strict finite permutation in this chain that joins:

1. an SC causal hop;
2. an axial face relation;
3. a handed shared-edge routing rule;
4. a common C4 phase tick; and
5. a BCC coarse displacement.

That architecture is compatible with the user's discrete-first intuition:
smooth propagation need not be primitive; it can arise from rapidly growing
families of discrete flag histories. But a continuum limit must still be
proved by blocking the interacting flag ensemble, not inferred from one ray.

The same capacity variable could condition flag admission and local material
clock admission, creating a route for a common light/body geometry. No such
capacity-weighted permutation or lensing result exists yet.

---

## 8. Next locked gate

Classify reversible local collisions among the 48 flags at a shared cube or
vertex. A passing collision must:

1. preserve exactly the polar-edge and axial-face moments required by the
   Hodge target;
2. reduce sixteen ballistic ray cycles to two transverse hydrodynamic
   polarization pairs;
3. derive both divergence constraints from finite continuity;
4. retain equation (15) or derive a different cone without tuning;
5. accept and reverse the manifestation source transaction with exact work;
6. share capacity with the recurrent material clock; and
7. introduce no target value for alpha or gravity.

Only after that collision kernel passes is a native Maxwell response or
source-force measurement admissible.

The first invariant-space sub-gate now passes in the
[Hodge-flag pair theorem](THEOREM_HODGE_FLAG_PAIR_COLLISION_INVARIANT_SPACE_AND_EQUIVARIANT_MATCHING_BOUNDARY_v1.md).
On the 192 flag-phase states, the complete field-preserving two-record
relation has exactly seven additive invariants: record number plus polar $E$
and axial $B$. Every one of its 73 field sectors is even, so abstract
involutive matchings exist and no extra ballistic-ray label is forced. The
physical step remains the symmetry problem: an $O_h$/C4-equivariant
deterministic matching and its full transport kernel have not been derived.

The later
[Hodge-framed common-source theorem](../common_action_mechanics_reciprocity/THEOREM_HODGE_FRAMED_ALL_AXIS_CONSTRAINT_LIFT_AND_ONE_SIGNED_EVENT_GENERATOR_BOUNDARY_v1.md)
uses the same flag for a second purpose. For tangent \(r\), its
\(u=hn\) and \(v=r\times n\) axes orient both finite bundles required by an
axial STF-divergence source. This closes the gravity-source plane context and
one prepared source generator without promoting the sixteen flag rays to
Maxwell modes. The collision reduction and native field pole remain open.
