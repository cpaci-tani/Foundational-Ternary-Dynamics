# Oriented plaquette C4 flat band and shared-edge price v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT ORIENTED PLAQUETTE FOUR-CYCLE]** +
**[CLOSED NEGATIVE, SCOPED — ISOLATED C4 CIRCULATION AS A LIGHT-CONE
CARRIER]** +
**[OPEN — REVERSIBLE SHARED-EDGE PLAQUETTE EXCHANGE]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_oriented_plaquette_c4_flat_band_boundary.py](../../../../../scripts/proofs/proof_oriented_plaquette_c4_flat_band_boundary.py)
performs 145 exact Laurent-polynomial checks on all 24 oriented plaquette
carriers and both circulations. No target dispersion or physical constant is
used.

---

## 1. The tempting finite lift

The
[oriented Hodge-Maxwell target](THEOREM_ORIENTED_BOND_PLAQUETTE_HODGE_MAXWELL_TARGET_AND_FINITE_LIFT_BOUNDARY_v1.md)
identifies an axial plaquette record as the minimum carrier missing from the
phase-independent current collision. The simplest finite implementation seems
obvious: let one token advance around the four directed boundary edges of an
elementary plaquette.

For an oriented plane with ordered spanning directions $(d,e)$, the four
steps are

\[
 d,\quad e,\quad-d,\quad-e.                        \tag{1}
\]

This is local, reversible, and has the desired C4 recurrence. The question is
whether that recurrence also propagates.

---

## 2. Exact Bloch transfer

Let $X_a=e^{ik_a}$ be formal Bloch translation variables. The one-step
four-state transfer matrix $P(k)$ advances the internal boundary pointer and
multiplies by the monomial associated with the corresponding step in equation
(1).

The net displacement is exactly

\[
 d+e-d-e=0.                                        \tag{2}
\]

Therefore, for every one of the 24 oriented plane states and for both
circulation signs,

\[
 \boxed{P(k)^4=I_4.}                               \tag{3}
\]

The characteristic polynomial is

\[
 \boxed{\chi_P(\lambda)=\lambda^4-1,}              \tag{4}
\]

independent of $k$. All four quasiphases are flat:

\[
 \lambda\in\{1,i,-1,-i\},
 \qquad \nabla_k\omega=0.                          \tag{5}
\]

The reverse circulation has the same polynomial and the inverse cycle.

---

## 3. Scoped closure

An isolated oriented plaquette cycle is a genuine finite clock and a genuine
circulation carrier, but not a radiative carrier:

\[
 \boxed{
 \text{closed local C4 orbit}\;\ne\;\text{Maxwell propagation}.} \tag{6}
\]

This is the finite counterpart of the earlier result that a common C4 phase
advance cannot repair a missing Bloch cone. A clock supplies ordered
recurrence. Propagation requires relational transfer between distinct spatial
cells.

The closure is scoped to an isolated fixed plaquette. It does not exclude a
network in which a token arriving at a shared edge reversibly changes which
adjacent plaquette owns its next segment.

---

## 4. Shared-edge price

Every cubic edge is incident to four oriented plaquette planes. A propagating
finite lift must therefore add a local shared-edge collision

\[
 (\text{incoming plaquette flag},\text{edge record},\text{orientation})
 \longleftrightarrow
 (\text{outgoing plaquette flag},\text{updated edge record},\text{memory})
                                                               \tag{7}
\]

that:

1. changes plaquette ownership rather than completing the same closed loop;
2. preserves the orientation sign and complete C4 payload;
3. is bijective on all allowed and backpressured states;
4. produces the signed incidence difference, not an unsigned random walk;
5. leaves no hidden controller or copied token; and
6. yields the Hodge generator's nonzero first-order $k$ term after blocking.

Without equation (7), the Hodge-Maxwell operator remains a real-linear target
with no finite microscopic lift.

---

## 5. Relation to clocks, matter, and measurement

The result clarifies the role of the same C4 ontology in different sectors:

- a localized recurrent material token may use equation (3) as a proper
  internal clock;
- a detector record may use the ordered phase to gate actualization; but
- light requires shared-edge ownership transfer between such local cycles.

Thus FTD does not need separate notions of time and phase, but it does need a
separate **spatial transaction topology**. Reusing the clock does not mean
equating a clock orbit with a propagating field.

---

## 6. Next locked gate

Enumerate the smallest payload-complete shared-edge state space and classify
its cubic/C4-equivariant reversible permutations. A passing map must have a
nonzero first-order polar-edge/axial-face Bloch block and reduce to the exact
incidence signs of the Hodge-Maxwell target. The classification must be done
before evaluating a light speed, source response, lensing, or alpha.

**Executed successor:** the
[shared-edge Hodge flag theorem](THEOREM_SHARED_EDGE_HODGE_FLAG_BCC_PROPAGATION_AND_MAXWELL_REDUCTION_BOUNDARY_v1.md)
constructs a 48-state finite permutation with exact inverse and nonflat
propagation. Its order-three internal frame turns three orthogonal SC hops into
one BCC body-diagonal displacement at speed $1/\sqrt3$. This passes the finite
transport sub-gate, but leaves sixteen ballistic flag cycles rather than two
Maxwell modes; the interacting flag collision and constraints remain open.
