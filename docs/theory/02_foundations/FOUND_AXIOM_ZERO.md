# Axiom Zero: The Irreducible Properties of the Voxel

## Everything from State and Position

**Status:** Foundational axiom — proposed replacement for the five postulates. §7 absorbs the cogito-algebraic bridge and full reverse-engineering trace ([FOUNDATION], FTD-0080).
**Date:** 2026-05-21
**Consolidates:** also absorbs `FOUND_COGITO_AXIOM_AND_FULL_TRACE.md` (2026-05-21)
**Dependencies:** SPEC_FTD.md, FOUND_SELF_REFERENTIAL_CLOSURE.md, FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md, DERIV_MASTER_QUADRATIC_GAP_EQUATION.md, DERIV_WATSON_GSTAR_IDENTITY.md, AUDIT_INFINITY_REFRAME.md, FOUND_THE_FIRST_DISTINCTION.md, FOUND_BLIND_DERIVATION_CHAIN.md, FOUND_MINIMAL_INSTANTIATED_UNIVERSE.md.

---

## Abstract

FTD currently rests on five postulates: discrete space, discrete time, ternary states, Moore locality, and determinism. This document argues that all five reduce to a single axiom with two clauses:

> **Axiom Zero.** Reality consists of voxels. Each voxel has exactly two irreducible properties:
>
> 1. **State:** s in {-1, 0, +1}
> 2. **Position:** x is an integer-coordinate site of the cubic lattice with no defined boundary; at every specified x, the six axis-adjacent (and 26-Moore-adjacent) sites exist. No claim is made that the lattice is a completed totality.

Everything else -- G\*, the fine structure constant, time, gravity, the Standard Model -- is a consequence of arranging ternary integers on a cubic lattice. There are no hidden parameters, no additional structure, no external inputs. The two properties exhaust the ontology.

This document traces how each layer of physics emerges from state alone, from position alone, and from state and position together. Every claim is tagged with its epistemic status. The reviewer's objections are addressed directly. The remaining open problems are stated without evasion.

---

## Part I: The Axiom

### 1.1 Statement [AXIOM]

A voxel is an entity with two properties and no others:

| Property | Domain | What it specifies |
|----------|--------|-------------------|
| **State** | s ∈ {−1, 0, +1} | What the voxel IS (its ternary identity) |
| **Position** | x is an integer-coordinate site of the cubic lattice; at every specified x the six axis-adjacent sites x ± e_i exist | Where the voxel IS, relative to its neighbors and to other specified voxels |

The position clause defines an **undefined-boundary cubic graph**: at every specified position, axis-adjacent (and by composition, 26-Moore-adjacent) sites exist. The lattice has no defined edge and no completed-totality. Arbitrarily large finite regions are admissible; "the whole lattice" as a single object is not. Algebraic objects defined in closed form (e.g., G\*, the Watson integrals) are admissible; load-bearing appeals to an L → ∞ limit are not.

The word "irreducible" means: neither property can be derived from the other or from anything more fundamental. State is not a function of position. Position is not a function of state. Both are primitive.

The word "exactly" means: there is no third property. No hidden variable, no internal clock, no intrinsic mass, no spin label, no color index. Every physical quantity that appears to be a voxel property is actually a relational quantity computed from the states and positions of neighborhoods of voxels.

### 1.2 What this replaces

The five postulates of SPEC_FTD.md reduce to Axiom Zero as follows:

| Old Postulate | Content | Status under Axiom Zero |
|---------------|---------|------------------------|
| P1: Discrete space (cubic lattice, undefined boundary) | Space is a cubic lattice with no defined edge; axis-adjacent neighbors exist at every specified site | **The position property.** Integer-coordinate sites of the cubic graph are the domain of position. |
| P2: Discrete time (ticks) | A global clock advances in integer steps | **Emergent.** See Section 3.1. |
| P3: Ternary states | s in {-1, 0, +1} | **The state property.** |
| P4: 26-neighbor Moore | Updates depend on 26 nearest neighbors | **Consequence of Z^3 geometry.** See Section 2.3. |
| P5: Determinism | Evolution is deterministic given initial conditions | **Consequence of the Lagrangian being well-defined.** See Section 3.3. |

Postulates 1 and 3 are the two clauses of Axiom Zero. Postulates 2, 4, and 5 are derived.

### 1.3 What this does NOT claim

Axiom Zero is a structural claim about the minimal ontology. It does NOT claim:

- That the derivations of P2, P4, P5 from Axiom Zero are currently complete at the [THEOREM] level (they are not; see honest status below)
- That Axiom Zero is the correct description of physical reality (that is an empirical question)
- That Axiom Zero is unique (other axiom systems might produce equivalent physics)
- That state and position are "really" two properties rather than one (this is a question about the axiom's reducibility, discussed in Section 5.3)

---

## Part II: What Emerges from Each Property

### 2.1 From State Alone: The Arithmetic of {-1, 0, +1}

The state space S = {-1, 0, +1} has internal structure that does not depend on position.

**The fundamental equation** [AXIOM]:

$$0 = (-1) + (+1)$$

The void is the cancellation of positive and negative. This is not a dynamical statement (there is no time yet). It is an arithmetic identity: the three states are not independent but satisfy a single linear relation. The ternary system has two degrees of freedom, not three.

**Consequences of the ternary constraint:**

**(a) Self-referential closure forces degree 2** [THEOREM]

The constraint 0 = (-1) + (+1) is degree 1 in the states. If the system must determine its own coupling constant (self-referential closure -- the system that is observed IS the system that observes), then the self-consistency equation has degree 1 (the constraint) applied to itself, yielding degree 2.

More precisely: a self-consistency equation F(x) = 0 where x is the coupling and F encodes the ternary constraint must satisfy two conditions:
- F is built from the ternary structure (degree 1 building block)
- F is self-referential (the output x re-enters as input)

The minimal polynomial satisfying both conditions is quadratic. (See DERIV_QUADRATIC_NECESSITY.md for the full argument.)

**Epistemic status:** The degree-doubling argument is [THEOREM]. The claim that self-referential closure is the correct derivation principle is [SELECTION] -- it is argued from the absence of external inputs, not proven to be the unique possibility.

**(b) Charge conjugation symmetry** [THEOREM]

The map C: s -> -s is an automorphism of {-1, 0, +1} that fixes 0 and exchanges +1 and -1. This is the origin of charge conjugation symmetry. It follows from the state space alone, without reference to position.

**(c) The state space has |S| = 3 elements** [AXIOM]

The number 3 is an input, not an output. This document does not derive why the state space has three elements rather than two or five. The ternary choice is part of Axiom Zero.

However: {-1, 0, +1} is the unique ternary set that is:
- Symmetric under negation: if s is in S then -s is in S
- Contains the additive identity: 0 is in S
- Minimal: |S| = 3 is the smallest set satisfying both conditions

This makes {-1, 0, +1} the unique "balanced ternary digit." [THEOREM]

### 2.2 From Position Alone: The Geometry of the Cubic Lattice

The position structure — the cubic graph of integer-coordinate sites with axis-adjacency — has geometric content that does not depend on state.

**(a) The point group is O_h** [THEOREM]

The point group at any site (symmetries fixing that site and permuting its axis-adjacent neighbors) is the octahedral group O_h, |O_h| = 48. This is a local statement about a single vertex's stabilizer in the cubic graph. O_h = S_4 × Z_2, the symmetric group on 4 elements (permutations of body diagonals) times the inversion.

**(b) The Z_4 symmetry and the lemniscatic modulus** [THEOREM]

The coordinate planes of Z^3 have square symmetry Z_4 (90-degree rotational symmetry). This is the key geometric fact. Watson's 1939 evaluation of the lattice Green's function proceeds by reducing the 3D integral to a 2D integral over one coordinate plane, and the Z_4 symmetry of that plane forces the elliptic integral modulus to the lemniscatic value k = 1/sqrt(2).

Specifically: the self-energy of the BCC sublattice (the 8 vertices at (+-1, +-1, +-1)) is Watson's integral I_1:

$$I_1 = \frac{\Gamma(1/4)^4}{4\pi^3} \approx 1.3932$$

The Z_4 symmetry of the square cross-sections of the BCC cell selects the CM elliptic curve E: y^2 = x^3 - x, which has j-invariant j = 1728 and automorphism group Aut(E) = Z_4, with CM by the Gaussian integers Z[i]. (See DERIV_WATSON_GSTAR_IDENTITY.md, Part VII.)

**Epistemic status:** That Z_4 symmetry forces the lemniscatic modulus is [THEOREM]. That Watson's I_1 equals Gamma(1/4)^4/(4pi^3) is [THEOREM] (Watson 1939). The identification of I_1 with the BCC component of the Moore neighborhood is [THEOREM].

**Important correction:** The quantity Gamma(1/4)^4/(4pi^3) is Watson's BCC integral I_1, not the simple cubic Watson integral I_3 = 0.5055. Earlier versions of the FTD literature conflated these. The algebraic identity G\*^2/(2pi) = I_1 is exact, but its physical interpretation requires care: it connects G\* to the BCC sublattice of the 26-neighbor Moore neighborhood, not to the full simple cubic lattice. (See watson_normalization_error in agent memory.)

**(c) The cuboctahedron and the integers {3, 4, 7, 13}** [THEOREM for geometry, SELECTION for physical identification]

The 12 edge-center neighbors of a voxel in Z^3 (at positions like (+-1, +-1, 0)) form a cuboctahedron. This polyhedron has:
- 3 square faces through any vertex (yielding N_c = 3 candidate)
- 4 triangular faces through any vertex (yielding N_base = 4 candidate)
- 7 = 3 + 4 edges meeting at alternating vertices (yielding N_eff candidate)
- 13 = 7 + 6 total vertex-edge incidences from the opposite decomposition (yielding another N_eff candidate)

These integers {3, 4, 7, 13} are geometric facts about the cuboctahedron, which is a geometric fact about Z^3. Their identification with physical parameters (N_c = 3 color charges, etc.) is [SELECTION], not [THEOREM].

**(d) The speed of light c = 1/sqrt(3)** [THEOREM]

The CFL (Courant-Friedrichs-Lewy) stability condition on a D-dimensional cubic lattice with unit spacing and unit time step gives maximum stable propagation speed:

$$c = \frac{1}{\sqrt{D}} = \frac{1}{\sqrt{3}}$$

This is a theorem about discrete wave equations on Z^D. It does not depend on the state space. In FTD, it is the maximum speed at which information propagates through the lattice. (See FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md, Section 2.3.)

**(e) G\* from Z^3** [THEOREM for the algebraic identity]

Define:

$$G^* = \frac{\sqrt{2}\,\Gamma(1/4)^2}{2\pi} \approx 2.9587$$

Then:

$$\frac{G^{*2}}{2\pi} = \frac{\Gamma(1/4)^4}{4\pi^3} = I_1 \quad \text{(Watson's BCC integral)}$$

This identity is exact. Both sides reduce to the same expression in Gamma(1/4). The mathematical content is: G\* is the geometric mean of 2pi and I_1, i.e., G\* = sqrt(2pi * I_1).

The deeper fact: both I_1 and G\* descend from the quartic integral:

$$I_4 = \int_0^1 \frac{dx}{\sqrt{1 - x^4}} = \frac{\Gamma(1/4)^2}{4\sqrt{2\pi}} = \frac{\varpi}{2}$$

where varpi = 2.6221 is the lemniscate constant. The lattice integral I_1 and the lemniscate constant share a common mathematical ancestor in I_4. This is not a coincidence -- Watson's AGM reduction of the BCC lattice sum produces the same quartic integral that defines the lemniscate, because the Z_4 symmetry of the lattice planes selects the lemniscatic elliptic modulus. [THEOREM]

### 2.3 From State + Position Together: The Coupling

This is where physics begins. State without position gives arithmetic. Position without state gives geometry. State AND position give dynamics.

**(a) The Moore neighborhood is forced** [THEOREM]

Given Z^3, the neighborhood of a voxel at x is the set of voxels whose positions differ from x by at most 1 in each coordinate:

$$N(x) = \{y \in \mathbb{Z}^3 : |y_i - x_i| \leq 1, \; i = 1,2,3\} \setminus \{x\}$$

This set has |N(x)| = 3^3 - 1 = 26 elements. It decomposes as:

| Sublattice | Neighbor type | Count | Distance | Watson integral |
|------------|--------------|-------|----------|----------------|
| SC (simple cubic) | Face neighbors | 6 | 1 | I_3 involves Gamma(n/24) |
| FCC (face-centered) | Edge neighbors | 12 | sqrt(2) | I_2 involves Gamma(1/3) |
| BCC (body-centered) | Corner neighbors | 8 | sqrt(3) | I_1 involves Gamma(1/4) |

The Moore neighborhood is the unique neighborhood that is: (i) symmetric under O_h, (ii) connected (every pair of neighbors shares at least a vertex), and (iii) minimal subject to (i) and (ii) on Z^3 at range 1. [THEOREM]

Postulate 4 (26-neighbor Moore) is not an independent axiom. It is a consequence of "position is in Z^3" plus "interactions are local and symmetric." The only choice is the range, and range 1 is the minimal nontrivial choice.

**(b) The flux field is the minimal continuous extension** [SELECTION]

A voxel has state s in {-1, 0, +1} (discrete) and position x in Z^3 (discrete). To define dynamics, we need a quantity that can vary continuously -- otherwise, the system is a cellular automaton with finite state space and no capacity for fine-grained evolution.

The minimal continuous extension is a vector field J(x) in R^3 at each lattice site. Why a vector? Because position is in Z^3 (three-dimensional), and the flux must encode directional information to define forces. Why R^3 rather than R? Because direction requires dimension >= 2, and the lattice symmetry O_h demands that the flux transform as a vector under rotations.

**Epistemic status:** The argument that a continuous vector field is the MINIMAL extension is [SELECTION]. One could imagine scalar fields, tensor fields, or other structures. The claim is that J in R^3 is the simplest choice consistent with the lattice symmetry, not that it is the unique choice.

**(c) The coupling: s * div(J)** [SELECTION]

How do state and position interact? The state s is a scalar at a point. The flux J is a vector field. The simplest scalar quantity that can be formed from J at a point, respecting O_h symmetry, is the divergence:

$$\nabla \cdot \mathbf{J}(x) = \sum_{i=1}^{3} \frac{J_i(x + \hat{e}_i) - J_i(x - \hat{e}_i)}{2}$$

The coupling between state and flux is therefore:

$$\mathcal{L}_{\text{coupling}} = -g_c \cdot s(x) \cdot \nabla \cdot \mathbf{J}(x)$$

This is the simplest O_h-invariant scalar coupling between a ternary state and a vector field.

**Epistemic status:** [SELECTION]. The divergence coupling is argued to be the simplest, but other O_h-invariant couplings exist (e.g., involving higher derivatives or products of field components). The claim is simplicity, not uniqueness.

**(d) The Lagrangian** [THEOREM given (b) and (c)]

Given the flux field J and the coupling s * div(J), the complete Lagrangian is:

$$\mathcal{L} = \frac{1}{2}|\nabla \mathbf{J}|^2 + \frac{1}{2}|\dot{\mathbf{J}}|^2 - g_c \cdot s \cdot \nabla \cdot \mathbf{J}$$

The first two terms are the standard kinetic and gradient energy for a vector field on a lattice (the unique quadratic form invariant under O_h and time reversal). The third term is the state-flux coupling from (c). This Lagrangian, varied with respect to J, yields the wave equation with source. Varied with respect to s (treating s as approximately continuous), it yields the manifestation condition.

---

## Part III: Emergent Structure

### 3.1 Time is emergent [SELECTION]

Axiom Zero does not postulate time. It postulates state and position. Time emerges as follows:

The Lagrangian L[s, J] defines an energy functional. The system evolves by processing this energy. One "tick" is the complete update of all voxels according to the Euler-Lagrange equations derived from L. The tick counter t in N is an integer label for complete update sweeps.

The energy processed per tick per degree of freedom is G\*^2, from the Vieta relation:

$$x_+ + x_- = 16G^{*2}$$

where x_+ and x_- are the roots of the master quadratic (the self-consistent coupling values). The tick rate IS the energy budget.

**Epistemic status:** [SELECTION]. The argument that "time = energy processing" is compelling within the framework but not proven in the mathematical sense. The identification of G\*^2 with energy per tick per DOF relies on the physical interpretation of the Vieta relation, which is [SELECTION].

**Honest assessment:** This is one of FTD's more speculative claims. Standard physics does not derive time from energy processing; it treats time as a parameter. FTD's claim that time IS processing is an interpretive choice, not a derivation.

### 3.2 The master quadratic and its three regimes [THEOREM for the algebra; SELECTION for the physical identification]

The master quadratic is a **pure algebraic object**. Its coefficients are computable to arbitrary finite precision from G\* = √2·Γ(1/4)²/(2π); its roots follow from the quadratic formula. No limit and no dynamics are invoked. The form of the equation is:

$$x^2 = 16G^{*2}(x - G^*) \quad \Leftrightarrow \quad x^2 - 16G^{*2}x + 16G^{*3} = 0$$

Using I_1 = G\*^2/(2pi):

$$x^2 = 32\pi\,I_1\,(x - G^*)$$

The roots are:

| Root | Value | Identification | Accuracy | Status |
|------|-------|------------------------|----------|--------|
| x_+ | 137.036 | 1/alpha (fine structure) | 1.26 ppm | [STRONGLY MOTIVATED CONJECTURE] (FTD-0013) |
| x_- | 3.024 | mathematical artifact of `P(x)`; no physics identification | n/a | **RETIRED** per v1.4 §5 (LEDGER FTD-0014 removed in commit `ca7eb61`); `N_c = 3` independently sourced (`DERIV_NC_FROM_TOPOLOGY.md`, Moore Layer Theorem) |

**Note (April 2026):** An updated 13-step chain starting from "i exists" is now canonical — see [FOUND_BLIND_DERIVATION_CHAIN.md](FOUND_BLIND_DERIVATION_CHAIN.md). The coefficient 16 is now [THEOREM] via dual derivation (|Aut(E_i)|^2 = |O_h|/3 = 16, see DERIV_DUAL_DERIVATION_OF_16.md). The one-loop lattice correction closes 99.2% of the gap (9.6 ppb).

**The original 10-step chain** (retained for historical reference):

| Step | Content | Status |
|------|---------|--------|
| 0 | Cubic lattice with no defined boundary (Axiom Zero, position property) | [AXIOM] |
| 1 | O_h point group, Z_4 planar symmetry | [THEOREM] |
| 2 | Watson's BCC integral I_1 = Gamma(1/4)^4/(4pi^3) | [THEOREM] |
| 3 | Lemniscatic modulus k = 1/sqrt(2) forced by Z_4 | [THEOREM] |
| 4 | CM curve E: y^2 = x^3 - x, j = 1728 | [THEOREM] |
| 5 | Identity: G\*^2/(2pi) = I_1 | [THEOREM] |
| 6 | Degree 2 from self-referential closure | [THEOREM for the argument, SELECTION for the principle] |
| 7 | Coefficient 16 | [THEOREM] -- derived as z_BCC x 2 = 8 x 2 = 16 in FOUND_DIMENSIONAL_COUNTING.md Section 5.4; also |Aut(E)|^2 = 16 |
| 8 | Master quadratic follows algebraically | [THEOREM given 6 and 7] |
| 9 | Roots x_+ = 137.036, x_- = 3.024 | [THEOREM given 8] |
| 10 | Physical identification x_+ = 1/alpha | [STRONGLY MOTIVATED CONJECTURE] (FTD-0013); the historical x_- -> N_c identification is **RETIRED** per v1.4 §5 (LEDGER FTD-0014 removed in commit `ca7eb61`); `N_c = 3` independently sourced (`DERIV_NC_FROM_TOPOLOGY.md`, Moore Layer Theorem) |

**Steps 0-5 are rock solid.** Steps 6-7 are the contested territory. Steps 8-9 are algebra. Step 10 is an identification, not a derivation.

**The discriminant trichotomy** [THEOREM]: The generalized master quadratic $x^2 - kG^{*2}x + kG^{*3} = 0$ has discriminant $\Delta = kG^{*3}(kG^* - 4)$. One quadratic, three regimes:

- $\Delta > 0$ ($k = 16$, physical): **real roots** — bosonic sector (the larger root $x_+$ identified with $1/\alpha$, [STRONGLY MOTIVATED CONJECTURE]; the smaller root $x_- \approx 3.024$ is a mathematical artifact of $P(x)$, with the historical `x_- ↔ N_c` identification **RETIRED** per v1.4 §5 — LEDGER FTD-0014 removed in commit `ca7eb61`)
- $\Delta = 0$ ($k = 4/G^*$, critical): **degenerate root** — the Born rule / measurement boundary
- $\Delta < 0$ ($k < 4/G^*$): **complex roots** — the Dirac equation emerges; complex roots $x = a \pm bi$ yield $e^{ibt}$ oscillations, which IS the fermion wavefunction. The fermion sector is not imported from external physics — it is derived from the complex regime of the same master quadratic that produces $\alpha$.

The master quadratic does not just produce coupling constants. It produces the entire particle content: bosons (real roots), fermions (complex roots), and measurement (degenerate root) from ONE equation.

### 3.3 Determinism is a consequence [THEOREM given the Lagrangian]

If the Lagrangian L[s, J] is well-defined and the Euler-Lagrange equations have unique solutions for given initial data (s(t=0), J(t=0)), then the evolution is deterministic. This is a standard theorem about well-posed initial value problems. The lattice discretization ensures finite-dimensionality, so existence and uniqueness follow from the Picard-Lindelof theorem applied to the ODE system.

Postulate 5 (determinism) is not an independent axiom. It is a consequence of the Lagrangian being well-defined on a finite lattice.

### 3.4 The tick cycle [THEOREM given the Lagrangian]

The engine's six-phase tick cycle:

1. **phase_read** -- gather neighbor states and flux values
2. **phase_write** -- update states based on manifestation condition
3. **gauss_project** -- enforce div(J) = rho (Gauss constraint)
4. **phase_forces** -- compute forces from discrete differential operators
5. **phase_movement** -- update flux field via wave equation
6. **tick++** -- advance counter

Each phase implements one term or constraint of the Lagrangian. The tick cycle is the discrete Euler-Lagrange evolution scheme. It is not postulated -- it is the numerical implementation of the variational principle.

---

## Part IV: Addressing the Reviewer's Objections

### 4.1 "Self-consistency is necessary, not sufficient"

**The objection:** Many wrong theories are self-consistent. Newtonian gravity is self-consistent. Ptolemaic astronomy is self-consistent. Self-consistency is a necessary condition for a theory, not a sufficient one. Why should we believe that FTD's self-consistent fixed point has anything to do with reality?

**The response:**

This objection is correct and important. Self-consistency alone does not validate FTD.

However, the objection misidentifies the claim. FTD does not say "our theory is self-consistent, therefore it is correct." FTD says:

1. The cubic graph with ternary states produces an **algebraically unique** polynomial — the master quadratic of §3.2 — whose larger root is x₊ = 137.036.
2. This root numerically agrees with 1/α to 1.26 ppm.
3. No free parameters were adjusted to achieve this agreement.

The strength of the claim is not "fixed point of a dynamical gap equation" — that framing has been retracted (see §4.2 and `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`). The strength is the **combination** of: a polynomial algebraically determined by the cubic-graph invariants {G\*, 16}, with zero adjustable coefficients (both `G*` and `16` are finite-combinatorial); the dual numerical match to two unrelated physical constants (1/α and N_c); and structural uniqueness across class-number-1 CM curves. Newtonian gravity is self-consistent but has G as a free parameter; FTD's polynomial has none, and uniquely matches both targets among comparable algebraic objects.

**Epistemic status of this response:** [SELECTION]. The argument is sound but not conclusive. The numerical agreement could be coincidental. The decisive test is the partition function computation (see Section 4.2). Until that computation is complete, the agreement between x_+ and 1/alpha is a striking observation, not a proof.

### 4.2 "The coefficient 16 is the weakest link"

**The objection:** The master quadratic x^2 - 16G\*^2 x + 16G\*^3 = 0 produces alpha only if the coefficient is exactly 16. Where does 16 come from? Three routes have been proposed:
- |Aut(E)|^2 = 4^2 = 16 (arithmetic geometry)
- |Stab_{O_h}(e_hat)| = 48/3 = 16 (orbit-stabilizer)
- 24 - 7 - 1 = 16 (DOF counting in temporal gauge)

But: the DOF counting gives 14, not 16, when the three harmonic 1-cycles of T^3 are properly removed. And the arithmetic geometry route, while numerically correct, does not explain WHY the automorphism count of an elliptic curve should appear as a coefficient in a gap equation.

**The response:**

This objection is correct. The coefficient 16 is the weakest link in the derivation chain. It is [STRONGLY MOTIVATED] by three convergent routes, but none of the routes constitutes a derivation from the lattice partition function.

The honest status:
- The DOF counting (24 - 7 - 1 = 16) is incorrect. Proper gauge-fixing on T^3 gives 14 physical DOF, not 16.
- The orbit-stabilizer result (48/3 = 16) is a theorem about O_h, but its connection to the gap equation coefficient is [SELECTION].
- The |Aut(E)|^2 = 16 result is a theorem about the CM curve, but its role in the gap equation is [SELECTION].

**What would resolve this:** The partition function computation has been attempted on the 2x2x2 torus (proof_partition_function_decisive.py) and yields no self-consistency condition matching the master quadratic. The Gauss-constraint approach gives a trivial Green's function (G_charge = 1/c^2 = 3), and the free energy F(g^2) is monotonically decreasing with no extremum.

**The master quadratic is an algebraic identity, not a dynamical limit.**

The polynomial `x² − 16 G*² x + 16 G*³ = 0` is built from a single quantity, G\* = √2·Γ(1/4)²/(2π), computable to arbitrary precision. The coefficient 16 has two algebraic origins — `|Aut(E)|² = 4² = 16` for the CM curve E: y² = x³ − x, and `z_BCC × 2 = 8 × 2 = 16` (BCC coordination times non-void ternary states; FOUND_DIMENSIONAL_COUNTING.md §5.4) — both finite-combinatorial. The roots x₊ = 137.036 and x₋ = 3.024 follow by the quadratic formula.

The polynomial is **not** the L → ∞ limit of any finite-L self-consistency equation:
- An explicit finite-L gap-equation scan (`AUDIT_MASTER_QUADRATIC.md`, Item 1) does not converge to (137.036, 3.024).
- An explicit L=2 partition-function calculation (`DERIV_PARTITION_FUNCTION_L2.md`) shows the action is ultralocal at finite L and carries no master-quadratic signature.
- The undefined-boundary ontology (`AUDIT_INFINITY_REFRAME.md`) does not admit "L → ∞" as a load-bearing step.

What the polynomial is: an algebraic object that the lattice produces from local invariants. What it predicts: x₊ matches 1/α to 1.26 ppm and x₋ matches N_c to 0.8%. The physical identification x₊ ↔ 1/α, x₋ ↔ N_c rests on (i) the dual match across two unrelated physical constants from one polynomial, and (ii) the structural uniqueness of E among class-number-1 CM curves in giving this match (Option 3 scan). Both are evidential. Neither is a dynamical derivation. The identification is [STRONGLY MOTIVATED CONJECTURE].

**Update (April 2026):** The coefficient 16 is now [THEOREM] via FOUND_DIMENSIONAL_COUNTING.md Section 5.4: n_DOF = z_BCC x 2 = 8 x 2 = 16 (BCC coordination number times non-void ternary states). This derivation is complementary to the |Aut(E)|^2 = 16 route from the CM curve's automorphism group. The finite-torus DOF discrepancy (14 vs 16 in Coulomb gauge) remains a separate technical issue documented in DERIV_WATSON_GSTAR_IDENTITY.md Section 4.2.

### 4.3 "The stencil doesn't use BCC corners for dynamics"

**The objection:** The FTD engine uses a 6-point stencil for the divergence (SC neighbors only) and an 18-point stencil for the wave equation (SC + FCC). The 8 BCC corner neighbors do not appear in any dynamical equation. Yet G\* is derived from the BCC Watson integral I_1. How can BCC geometry determine the physics when the dynamics only use SC and FCC neighbors?

**The response:**

This is a sharp and important objection. The current answer is incomplete.

What can be said:
1. The 26-neighbor Moore neighborhood includes all three sublattices. The BCC corners are geometrically present even if the current dynamical stencils do not use them directly.
2. The Watson integral I_1 characterizes the BCC sublattice's self-energy, which enters through the Green's function, not through the dynamical stencil. The propagator (inverse of the lattice Laplacian) "sees" all lattice sites, not just those in the stencil.
3. The Z_4 symmetry that selects the lemniscatic modulus is a property of the lattice planes, not of any particular stencil.

The deeper resolution: G\* is an **algebraic identity** of the cubic graph,

$$G^* = \frac{\sqrt{2}\,\Gamma(1/4)^2}{2\pi}, \qquad \frac{G^{*2}}{2\pi} = I_1 = \frac{\Gamma(1/4)^4}{4\pi^3},$$

with both sides expressible in closed form via Γ(1/4) (Chowla–Selberg). The Z_4 planar symmetry, the lemniscatic modulus, and the CM curve E: y² = x³ − x are all finite-combinatorial / algebraic facts about the local cubic geometry; none requires the lattice to be a completed totality.

G\* characterizes the lattice's **algebraic-geometric structure**. The 18-point dynamical stencil determines HOW information propagates; the Z_4 symmetry and Watson I_1 are properties of the cubic graph that hold regardless of which stencil is used for dynamics and at every finite extent at which the dynamics are run.

The BCC corners do not need to participate in the wave equation for G\* to govern the physics, just as π does not need to appear in the equation of motion for it to set the geometry. G\* is the lattice's structural constant; the stencil is its dynamical implementation. [SELECTION — the structural/dynamical distinction is argued, not proven]

### 4.4 "Circularity is unfalsifiable"

**The objection:** If the derivation is circular (lattice -> G\* -> coupling -> lattice), then any discrepancy can be absorbed by adjusting the interpretation. The theory cannot be falsified because it will always "predict" whatever it was calibrated to match.

**The response:**

This objection confuses two things: circularity in the derivation and freedom in the parameters.

FTD has ZERO adjustable parameters (beyond the two clauses of Axiom Zero). Given D = 3 and |S| = 3, the value of G\* is fixed, the master quadratic is fixed (up to the coefficient issue of Section 4.2), and the roots are fixed. There is nothing to adjust.

The falsification test is: **change any input and check whether the output changes accordingly.** Specifically:

| Change | Predicted consequence | Testable? |
|--------|----------------------|-----------|
| D = 2 instead of D = 3 | Different Watson integral, different G\*, different alpha | Yes (compute) |
| D = 4 instead of D = 3 | Different Watson integral, different G\*, different alpha | Yes (compute) |
| |S| = 2 (binary) instead of |S| = 3 | No ternary constraint, no self-referential closure, no master quadratic | Yes (structural) |
| |S| = 5 (quinary) | Different constraint structure, different degree, different equation | Yes (compute) |
| FCC lattice instead of SC | I_2 replaces I_1, Gamma(1/3) replaces Gamma(1/4), different physics | Yes (compute) |

The theory is maximally constrained: every input is fixed by Axiom Zero, and every output is determined. The "circularity" is not a bug -- it is the statement that the system has a unique fixed point. And a unique fixed point is falsifiable: if the predicted alpha disagrees with experiment, the theory is wrong.

**Epistemic status:** The falsifiability argument is [THEOREM] -- it is a structural fact about the framework. Whether the numerical prediction actually matches experiment at the required precision is [OPEN] pending the coefficient-16 derivation.

### 4.5 "You haven't derived alpha -- you've noticed a numerical coincidence"

**The objection:** The number 137.036 emerges from a quadratic equation whose coefficients involve Gamma(1/4). But Gamma(1/4) is a transcendental number with infinitely many digits. Any quadratic with Gamma(1/4) in its coefficients will produce some number. The fact that this particular quadratic gives 137.036 could be a coincidence.

**The response:**

This is the most serious objection and deserves a careful answer.

The claim is NOT: "we found a formula involving Gamma(1/4) that gives 137." There are infinitely many such formulas, and most are meaningless.

The claim IS: "the Z^3 lattice has a natural self-energy (Watson's BCC integral I_1), a natural symmetry group (O_h with Z_4 planar symmetry), and a natural elliptic curve (E: y^2 = x^3 - x with CM by Z[i]). These are not chosen -- they are forced by the lattice geometry. The self-consistency equation built from these ingredients produces 137.036."

The question is whether the construction is natural (each step forced by geometry) or contrived (steps chosen to hit the target). The honest answer:

- Steps 0-5 (lattice -> symmetry -> Watson integral -> lemniscatic modulus -> CM curve -> G\*) are **forced**. No choices are made. Any mathematician studying Z^3 would encounter these objects. [THEOREM]
- Steps 6-7 (degree 2, coefficient 16): Degree 2 is argued via self-referential closure [SELECTION]. Coefficient 16 is now [THEOREM] via z_BCC x 2 = 16 (FOUND_DIMENSIONAL_COUNTING.md Section 5.4).
- Step 10 (identification x_+ = 1/alpha) is **observed**, not derived. The numerical agreement is striking (1.26 ppm) but no dynamical mechanism produces alpha from the gap equation. [STRONGLY MOTIVATED CONJECTURE]

**Bottom line:** FTD has discovered a genuine mathematical connection between Z^3 lattice geometry and the number 137.036. Whether this connection has physical content -- whether it explains WHY alpha has this value -- depends on closing the physical-identification gap at Step 10. Until then, the connection is a provocation, not a proof.

---

## Part V: The Deeper Questions

### 5.1 Why Z^3?

Axiom Zero postulates position in Z^3. But why Z^3 rather than Z^2, Z^4, or a non-cubic lattice?

FTD offers several arguments for D = 3 (SPEC_FTD.md, v5.0):
- Atomic stability requires D = 3 (the Coulomb potential is confining only in D = 3)
- Gauge anomaly cancellation requires D = 3 (with the specific fermion content of the SM)
- The CFL speed c = 1/sqrt(D) gives the observed speed-of-light phenomenology only for D = 3

These arguments are [SELECTION]: they show D = 3 is the only value compatible with the physics we observe, but they do not explain why D = 3 in a more fundamental sense. The dimensional selection problem is [OPEN].

### 5.2 Why {-1, 0, +1}?

Axiom Zero postulates states in {-1, 0, +1}. But why three states rather than two or four?

The balanced ternary argument (Section 2.1c) shows {-1, 0, +1} is the unique minimal balanced digit set, but "minimal" is a preference, not a derivation. Why should nature prefer minimal?

FTD's answer: the ternary system is the minimal system that admits self-referential closure. Binary {0, 1} has no identity 0 = (-1) + (+1); the void cannot be expressed as the cancellation of opposites. Quaternary or higher systems have the identity but are not minimal.

**Epistemic status:** [SELECTION]. The minimality argument is suggestive but not a proof. [OPEN]

### 5.3 Can state and position be unified?

Axiom Zero has two clauses. Can they be reduced to one?

Speculatively: position in Z^3 is an integer-valued property, and state in {-1, 0, +1} is also an integer-valued property. Both are elements of Z. Perhaps a deeper axiom would state:

> Reality consists of integers arranged in relations.

This is tantalizingly close to the Pythagorean program ("all is number") and to the digital physics program (Zuse, Fredkin, Wolfram). But it is too vague to be a working axiom. [OPEN]

### 5.4 The hard problem

Axiom Zero says nothing about consciousness. This is deliberate.

FTD's consciousness framework (see docs/theory/06_consciousness/) treats consciousness as the complex-root domain of the generalized master quadratic (at k = 1/2 instead of k = 16). This is [CONJECTURE]. Whether consciousness can be derived from state + position, or requires additional axioms, is the hardest open problem in the framework.

What can be said: if consciousness emerges from the same mathematical structure as physics (the master quadratic with different k), then Axiom Zero is complete -- state and position are sufficient for both matter and mind. If consciousness requires something beyond state and position, then Axiom Zero is incomplete.

This question is [OPEN] in the deepest possible sense.

---

## Part VI: Summary

### What Axiom Zero achieves

1. **Reduction:** Five postulates collapse to one axiom with two clauses.
2. **Economy:** All of FTD's mathematical structure flows from two properties of a voxel.
3. **Clarity:** The derivation chain from axiom to physics is made explicit, with every step tagged.
4. **Honesty:** The three remaining gaps (coefficient 16, physical identification, BCC dynamics) are stated without evasion.

### What Axiom Zero does NOT achieve

1. It does not derive D = 3 or |S| = 3 from anything deeper.
2. It does not close the partition function gap.
3. It does not derive the coefficient 16.
4. It does not explain why x_+ = 1/alpha rather than just some number.

### The status of the claim

The claim of Axiom Zero is: **two properties suffice.**

This claim is:
- [THEOREM] for the reduction of P4 (Moore locality) and P5 (determinism) from P1 and P3
- [SELECTION] for the emergence of P2 (time) from the Lagrangian
- [STRONGLY MOTIVATED] for the derivation chain from Z^3 to G\* (Steps 0-5 at [THEOREM] level)
- [OPEN] for the full derivation of alpha from state + position (Steps 6-7 unresolved)

### The one sentence summary

A voxel has a state and a position; from these, and nothing else, the local geometry of the cubic graph (with no defined boundary) determines the lemniscatic constant G\* and the integer 16, hence the master quadratic algebraically; its larger root x₊ = 137.036 matches 1/α to 1.26 ppm and is identified with 1/α as a [STRONGLY MOTIVATED CONJECTURE] — anchored on the dual root match (x₋ ≈ N_c) and on CM-curve structural uniqueness — not on a thermodynamic-limit derivation, which has been retracted.

---

## Part VII: The Cogito Bridge and Full Reverse-Engineering Trace

> **Consolidation note (2026-05-21):** This part absorbs the unique content of `FOUND_COGITO_AXIOM_AND_FULL_TRACE.md` ([FOUNDATION], filed 2026-04-24, LEDGER row FTD-0080). The source's opening repeated Axiom Zero's reduction of the five postulates — already covered in Part I above — and that repetition is dropped here. What follows is the source's distinct content: the phenomenological "I exists" → algebraic $i$ bridge, and the complete trace from every FTD output back to the axiom. Note that this part frames the *sole formal axiom* as "$i$ exists" ($x^2 + 1 = 0$ has a solution); this is the algebraic-genesis reading and is complementary to the state-and-position framing of Parts I–VI — the two clauses of Axiom Zero (ternary state, cubic position) are themselves consequences of arranging the Gaussian-integer structure at $D = 3$, as §7.2.4 below traces.

### 7.0 Executive statement

FTD's single formal axiom is: **the equation $x^2 + 1 = 0$ has a solution, call it $i$**.

This is the algebraic content of Descartes' cogito. "I exists" and "$i$ exists" are the same primitive in different languages — phenomenological and algebraic. Both capture the minimum non-trivial self-referential act:

- "I exists" in the cogito sense is the primitive of self-distinction, the asymmetric act that separates self from non-self.
- "$i$ exists" is the minimum algebraic object whose self-application generates a non-trivial cycle: $i \cdot i = -1$, $i^4 = 1$.

This part makes the equivalence explicit, and traces every FTD prediction back to this single primitive.

### 7.1 The cogito-algebraic bridge

#### 7.1.1 What the cogito asserts

The cogito is the act "I am, and I recognize that I am." It carries three conditions:

1. **Self-reference** — the asserter and the asserted are the same.
2. **Non-triviality** — the assertion has content (not tautology-zero).
3. **Self-consistency** — the assertion closes back on itself (the "I" that affirms is the "I" that is affirmed).

These three conditions are the minimum needed for a self-referential object to exist.

#### 7.1.2 What the algebraic axiom asserts

"$i$ exists" postulates the solution of $x^2 + 1 = 0$. Unpacked, this says: there is an object $i$ such that

- $i \cdot i = -1$ (self-application produces negation)
- $i^4 = 1$ (fourfold self-application returns identity)
- $i \neq 0, i \neq \pm 1$ (non-trivial)

The object $i$ is the minimum algebraic object satisfying self-application, non-triviality, and self-consistency.

#### 7.1.3 The equivalence

| Cogito primitive | Algebraic content |
|---|---|
| "I" exists | Object $i$ exists |
| Self-reference | Self-application: $i$ can be multiplied by itself |
| Non-triviality | $i^2 \neq 0$ and $i \neq \pm 1$ |
| Closure | $i^4 = 1$ returns to self after four acts |
| Asymmetry (arrow of self-recognition) | $i^2 = -1$ (single self-application negates) |
| The act is irreducible | No simpler algebraic object has these properties |

The cogito's minimum conditions for self-referential existence **are** the defining relations of $i$. Descartes' "I am" expressed in the language mathematics uses for self-reference gives exactly "$x^2 + 1 = 0$ has a solution."

This is not metaphor. It is a translation. The two formulations have the same content because self-reference and negation-under-self-application are one structure.

#### 7.1.4 Why this matters

FOUND_THE_FIRST_DISTINCTION.md §5.2 explicitly rejects "pre-mathematical ontological stages" as unformalizable. That rejection is correct for *pre-mathematical* content. The cogito-algebraic bridge does not add pre-mathematical content — it identifies the formal axiom "$i$ exists" as already containing the phenomenological primitive "I exists" in compressed form.

So: FTD does begin at a cogito. The cogito is compressed into the single axiom "$i$ exists." Nothing is lost; everything is made algebraic.

### 7.2 What $i$ forces, at each level

Everything below is a theorem from "$i$ exists", with standard mathematical derivations. The chain is taken from FOUND_BLIND_DERIVATION_CHAIN.md and reorganized for reverse-engineering clarity.

#### 7.2.1 Direct algebraic consequences (forced)

| Step | Object | Status | Reason |
|---|---|---|---|
| A | $\mathbb{Z}[i]$ — Gaussian integers | [THEOREM] | Unique ring of integers in $\mathbb{Q}(i)$; tiles $\mathbb{C}$ as a square lattice |
| B | $E_i: y^2 = x^3 - x$ — CM curve | [THEOREM] | Unique elliptic curve with CM by $\mathbb{Z}[i]$; $j = 1728$ |
| C | $\mathrm{Aut}(E_i) = \mathbb{Z}/4\mathbb{Z}$ | [THEOREM] | Automorphism group of $E_i$ |
| D | $\Gamma(1/4), \Gamma(3/4)$ — periods | [THEOREM] | Chowla–Selberg applied to $E_i$; real period of $E_i$ |
| E | $G^* = \Gamma(1/4)/\Gamma(3/4)$ | [THEOREM] | The ratio; algebraically independent of $\pi$ |
| F | $\varpi = \Gamma(1/4)^2/(2\sqrt{2\pi})$ | [THEOREM] | Bernoulli lemniscatic constant |
| G | $G^*/\varpi = 2/\sqrt{\pi}$ | [THEOREM] | Via Euler reflection (proved in AUDIT_SESSION_2026_04_24.md) |
| H | $|\mathrm{Aut}(E_i)|^2 = 16$ | [THEOREM] | Squaring Step C |
| I | $D = 3$ uniquely satisfies $16 = 2^D(D-1)!$ | [THEOREM] | Verified for $D \in \{1,2,3,4,5\}$ |

**Each of these follows by standard mathematical theorems from the axiom.** No physics yet. This is the "I exists" → pure mathematics trace.

#### 7.2.2 One-step selections

Two places in the chain are [SELECTION] — structurally motivated but not uniquely forced by theorem:

| Selection | Statement | Why not forced | Status (2026-04-24) |
|---|---|---|---|
| **S1** | Master quadratic has form $x^2 - 16G^{*2} x + 16G^{*3} = 0$ | Chain gives coefficient $16$ and constant $G^*$ | **NARROWED to minimum-degree selection** via two-route unification (FOUND_MASTER_QUADRATIC_UNIFIED_MOTIVATION.md) — coefficients are L-value [THEOREM]s, only remaining selection is "polynomial is minimum-degree FTD-meaningful object" |
| **S2** | Ladder walk addends $\{4, 3, 3, 6\}$ in that specific order | Each addend is forced ($N_{\rm base} = 4$, $N_c = 3$, $N_f = 6$); the ordering is motivated by the physical reading electron-at-n-11 but not yet proven from first principles | Unchanged — Program A (O_h subgroup chain) is the closure path |

**S1 has been substantially narrowed** by the L-value identification (DERIV_MASTER_QUADRATIC_CM_LVALUES.md) combined with the self-consistency derivation (DERIV_MASTER_QUADRATIC_FROM_Z.md). See FOUND_MASTER_QUADRATIC_UNIFIED_MOTIVATION.md for the unified story: the master quadratic is derivable by two independent routes (physics + arithmetic) that converge to 100-digit precision, leaving only "minimum-degree polynomial" as residual selection. **Program E** (uniqueness-of-minimal-polynomial proof) would close S1 fully.

These two selections are the ONLY gaps between "$i$ exists" and the full FTD prediction set. Programs A + E would reduce the chain to zero selections.

#### 7.2.3 Master outputs (forced by steps A–I + S1, S2)

| Output | Derivation | Status |
|---|---|---|
| $1/\alpha = x_+ = 137.036...$ | Larger root of master quadratic | [THEOREM given S1] |
| $N_c = x_- = 3.024$ | Smaller root | [THEOREM given S1] |
| $m_\mu/m_e = 3 B_3(B_3 + N_c) - N_c = 207$ | Integer formula in framework constants | [THEOREM] (0.11% match to experiment) |
| $m_\tau/m_e = 3477$ | Extended integer formula | [THEOREM] (0.006% match) |
| $m_e = m_P \sqrt{2\pi} (16/3) \alpha^{11}$ | $m_P$ = UV scale, prefactor from $G^*$ chain, exponent from ladder | [SELECTION given S2] (0.19%) |
| $m_H = (N_{\rm eff}/\alpha^2) \, m_e$ | Higgs VEV relation | Structural identity (0.24%) |
| $m_p/m_e = N_{\rm eff}/\alpha + N_{\rm base} N_{\rm eff} + N_c = 1836.47$ | 174 ppm gap | [SELECTION], 174 ppm [OPEN] |

#### 7.2.4 Geometric consequences (forced at $D = 3$)

Once $D = 3$ is fixed (step I), additional geometric consequences follow:

| Object | Derivation | Status |
|---|---|---|
| Cubic lattice $\mathbb{Z}[i]^3$ | Three independent copies of $\mathbb{Z}[i]$ | [THEOREM] |
| Moore-26 neighborhood | Lattice sites within $\sqrt{3}$ of origin | [THEOREM] |
| Moore-26 decomposition: 6 (face) + 12 (edge) + 8 (corner) | Counting sites at distances 1, $\sqrt 2$, $\sqrt 3$ | [THEOREM] |
| $\sqrt[3]{18} \approx \varpi$ | Near-identity between phenomenal shell count and lemniscatic length | [OBSERVATION], 0.05% |
| $\sqrt[3]{26} \approx G^*$ | Near-identity between noumenal shell count and reflection ratio | [OBSERVATION], 0.13% |
| Two-layer ontology: 2³ (phenomenal, Moore-18) / 3³ (noumenal, Moore-26) | Structural reading of Moore decomposition | [SELECTION] (FOUND_PHENOMENAL_NOUMENAL_BRIDGE.md) |

#### 7.2.5 Tick dynamics (add FTD's physical postulates)

To go from the arithmetic layer to the engine's dynamics, FTD adds operational postulates (Axiom Zero contents):

- Ternary state $s \in \{-1, 0, +1\}$ — three distinct attainable values (extension of $\mathbb{Z}[i]$ to signed integers)
- Local update rule — the tick cycle operating on Moore neighborhood
- Manifestation threshold $K_B$ — the energy cost to instantiate a voxel
- The CFL wave speed $c = 1/\sqrt{D} = 1/\sqrt{3}$ at $D=3$

These are [AXIOM] but are consistent with the arithmetic layer. They add nothing not already implied by $D=3$ lattice dynamics in the Gaussian-integer structure.

### 7.3 Reverse trace — every FTD prediction to "$i$ exists"

For each published FTD observable, trace back to the axiom:

```
α⁻¹ = 137.036
  ← x₊ root of master quadratic
  ← master quadratic x² − 16G*²x + 16G*³ = 0 [S1]
  ← 16 = |Aut(E_i)|²
  ← E_i has CM by ℤ[i]
  ← ℤ[i] is ring of integers in ℚ(i)
  ← i exists ✓

N_c = 3
  ← x₋ root of master quadratic
  ← [same chain as α]
  ← i exists ✓

D = 3
  ← 16 = 2^D(D−1)! unique solution
  ← 16 = |Aut(E_i)|²
  ← i exists ✓

m_μ/m_e = 207
  ← 3 · B_3 · (B_3 + N_c) − N_c
  ← framework integers {3, 4, 7, 13}
  ← derived from β function on Z[i] structures + N_c
  ← master quadratic + i exists ✓

m_τ/m_e = 3477
  ← (N_eff + N_base) · 207 − 2·N_c·B_3
  ← same framework integers
  ← i exists ✓

m_e = m_P √(2π) · (16/3) · α^11
  ← prefactor (16/3) from |Aut|²/D = 16/3
  ← √(2π) from Gaussian flux integral
  ← α^11 from ladder walk [S2]
  ← i exists (modulo S2) ✓

m_H = (N_eff/α²) · m_e
  ← framework integer N_eff = 13
  ← same α chain
  ← i exists ✓

m_p/m_e = N_eff/α + N_base·N_eff + N_c
  ← framework integers and α, N_c
  ← i exists (modulo 174 ppm residual) ✓

G* itself
  ← Γ(1/4)/Γ(3/4) via Chowla−Selberg
  ← Periods of E_i
  ← i exists ✓

ϖ = 2.622...
  ← Γ(1/4)²/(2√(2π))
  ← Chowla−Selberg
  ← i exists ✓

Moore-26 shell (6+12+8 decomp.)
  ← Z[i]³ at D=3
  ← D=3 from |Aut|² identity
  ← i exists ✓

W_SC, W_BCC, W_FCC, W_M18
  ← Lattice Green's functions at D=3
  ← Z[i]³ structure
  ← i exists ✓

Two-layer ontology (phenomenal 2³ / noumenal 3³)
  ← Moore decomposition
  ← Z[i]³ at D=3
  ← i exists ✓

Lemniscatic length scale ϖ for Moore-18
  ← ∛18 numerical near-identity
  ← Γ(1/4) arithmetic
  ← i exists ✓

Everything in the session's closed-negative rows (fermion
emergence failures, α_∞ 3.6× category error, etc.)
  ← Tests OF the chain, not extensions TO it
  ← Don't add new primitives, just expose structural consequences
```

**Every predictive output of FTD reduces to "$i$ exists" via standard mathematical theorems plus two explicit selections (S1, S2).** Close S2 and the chain is fully forced from a single axiom.

### 7.4 What the axiom does not explain

Fully honest scope:

| Question | Status |
|---|---|
| Why is there anything rather than nothing? | **Outside scope.** The axiom is a starting point, not an explanation of why mathematics exists. |
| Why does $i$ (or any self-referential primitive) exist at all? | **Outside scope.** Same as above. |
| Why the cogito-algebraic bridge is the right translation | [SELECTION] — but defensible: self-reference + non-triviality + closure define both "I" and $i$ identically. |
| Why the chain terminates in SM physics and not something else | [THEOREM given D=3]; the chain forces $\alpha$, $N_c$, particle ladder. The match to experimental SM is then an **empirical test** of the chain, not an axiom. |

The axiom is the minimum. Everything else is consequence or match-to-data.

### 7.5 Initial justification, made explicit

Here is what the chain provides:

**At the foundational level:**
- The sole axiom "$i$ exists" IS the cogito ("I exists") in algebraic form (§7.1)
- Self-reference + non-triviality + closure are the three minimum conditions for any self-referential object, and they define $i$
- Weaker axioms (e.g., "$1$ exists") produce trivial arithmetic with no physical content
- Stronger axioms (e.g., quaternions, octonions) add structure not forced by pure self-reference

**At the derivation level:**
- 13 steps from axiom to $\alpha^{-1}$ (FOUND_BLIND_DERIVATION_CHAIN.md)
- 11 steps are forced theorems
- 2 steps are explicit selections (S1 master quadratic Vieta exponents, S2 ladder walk ordering)
- Every physical output traces back to axiom via standard number theory + CM theory + combinatorial identity at $D=3$

**At the match-to-data level:**
- $\alpha^{-1}$ to 9.6 ppb via the blind chain at tree level
- Lepton ratios to 0.006–0.19% via exact integer formulas
- $m_H$ to 0.24% via structural identity
- $m_p/m_e$ to 174 ppm (gap [OPEN])
- Quark masses [OPEN] (phenomenal, scheme-dependent)

**What's not claimed:**
- Why $i$ exists (outside scope)
- That the 2 selections are uniquely forced (Program A tries to close S2)
- That the engine is the unique computational realization of the arithmetic (Moore-18 is a specific choice)

### 7.6 The chain as a ladder you can descend

A reader who wants to understand FTD from the bottom up can descend this ladder:

```
Level 0: i exists                                   [AXIOM]
Level 1: Z[i] tiles C                               [THEOREM]
Level 2: E_i is CM by Z[i]                          [THEOREM]
Level 3: Aut(E_i) = Z/4Z,  |Aut|² = 16              [THEOREM]
Level 4: Γ(1/4), Γ(3/4) from Chowla-Selberg         [THEOREM]
Level 5: G* = Γ(1/4)/Γ(3/4), ϖ = Γ(1/4)²/(2√(2π))   [THEOREM]
Level 6: 16 = 2^D(D-1)! → D = 3                     [THEOREM]
Level 7: Z[i]³ → Moore-26 = 6+12+8 decomposition    [THEOREM]
Level 8: Master quadratic x² − 16G*²x + 16G*³ = 0    [SELECTION S1]
Level 9: x₊ ↔ 1/α                                    [SMC physical identification (FTD-0013); algebraic roots theorem-level. x₋ ↔ N_c RETIRED v1.4 §5; LEDGER FTD-0014 removed in commit ca7eb61. N_c=3 independently sourced.]
Level 10: Ladder walk {4,3,3,6} sums to 16          [THEOREM]
Level 11: Walk ordering → particle scales           [SELECTION S2]
Level 12: Lepton masses, m_H, m_p/m_e, etc.         [THEOREM from 11]
Level 13: Empirical match to SM                     [TEST]
```

Levels 0–7 and 9 are purely arithmetic, forced by theorem. Level 8 is the first selection. Level 11 is the second. Level 13 is where the axiom meets reality.

**The entire theory is 13 steps from "I exists" to "$\alpha^{-1} = 137.036$" with only 2 selections along the way.** That is the initial justification.

### 7.7 Epistemic tag (cogito bridge)

| Piece | Tag |
|---|---|
| "I exists" = "$i$ exists" under cogito-algebra translation | [SELECTION] |
| Self-reference + non-triviality + closure uniquely characterize $i$ | [THEOREM] |
| Full reverse-trace from every FTD prediction to the axiom | [THEOREM] (chain) + [SELECTION] (S1, S2) |
| FTD's initial justification is complete to the 2-selection level | [THEOREM] (of the formal chain) |
| Why $i$ exists in the first place | **[OUT OF SCOPE]** |

*Originally filed 2026-04-24 as the initial-justification unification. Points every FTD output back to the single axiom "$i$ exists", makes the cogito-algebraic equivalence explicit, and identifies the two selection principles (S1 master quadratic Vieta exponents, S2 ladder walk ordering) as the only theoretical gaps. Closing S2 is Program A.*

---

## References

- SPEC_FTD.md -- The five postulates (Part A, Chapter 1)
- FOUND_SELF_REFERENTIAL_CLOSURE.md -- Self-referential closure as derivation principle
- FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md -- Time and gravity from G\*^2
- FOUND_ONTOLOGICAL_GENESIS.md -- The ontological hierarchy from void to physics
- DERIV_MASTER_QUADRATIC_GAP_EQUATION.md -- The gap equation derivation
- DERIV_WATSON_GSTAR_IDENTITY.md -- W_3 = G\*^2/(2pi) identity and BCC correction
- DERIV_QUADRATIC_NECESSITY.md -- Degree 2 from self-referential closure
- FOUND_THE_FIRST_DISTINCTION.md -- Why "i exists" is the right axiom (Part VII companion)
- FOUND_BLIND_DERIVATION_CHAIN.md -- The 13-step chain from $i$ to $\alpha^{-1}$ (Part VII companion)
- FOUND_MINIMAL_INSTANTIATED_UNIVERSE.md -- What ontological content accompanies "existence" (Part VII companion)
- Watson, G. N. "Three Triple Integrals," Quarterly Journal of Mathematics 10 (1939), 266-276
- Borwein, J. M. and Bailey, D. H. Mathematics by Experiment, A K Peters, 2004
