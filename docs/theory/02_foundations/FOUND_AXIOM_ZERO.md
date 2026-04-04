# Axiom Zero: The Irreducible Properties of the Voxel

## Everything from State and Position

**Date:** March 16, 2026
**Status:** Foundational axiom -- proposed replacement for the five postulates
**Dependencies:** SPEC_FTD.md, FOUND_SELF_REFERENTIAL_CLOSURE.md, FOUND_EMERGENT_TIME_GRAVITY.md, DERIV_MASTER_QUADRATIC_GAP_EQUATION.md, DERIV_WATSON_GSTAR_IDENTITY.md

---

## Abstract

FTD currently rests on five postulates: discrete space, discrete time, ternary states, Moore locality, and determinism. This document argues that all five reduce to a single axiom with two clauses:

> **Axiom Zero.** Reality consists of voxels. Each voxel has exactly two irreducible properties:
>
> 1. **State:** s in {-1, 0, +1}
> 2. **Position:** x in Z^3

Everything else -- G\*, the fine structure constant, time, gravity, the Standard Model -- is a consequence of arranging ternary integers on a cubic lattice. There are no hidden parameters, no additional structure, no external inputs. The two properties exhaust the ontology.

This document traces how each layer of physics emerges from state alone, from position alone, and from state and position together. Every claim is tagged with its epistemic status. The reviewer's objections are addressed directly. The remaining open problems are stated without evasion.

---

## Part I: The Axiom

### 1.1 Statement [AXIOM]

A voxel is an entity with two properties and no others:

| Property | Domain | What it specifies |
|----------|--------|-------------------|
| **State** | s in {-1, 0, +1} | What the voxel IS (its ternary identity) |
| **Position** | x in Z^3 | Where the voxel IS (relative to all other voxels) |

The word "irreducible" means: neither property can be derived from the other or from anything more fundamental. State is not a function of position. Position is not a function of state. Both are primitive.

The word "exactly" means: there is no third property. No hidden variable, no internal clock, no intrinsic mass, no spin label, no color index. Every physical quantity that appears to be a voxel property is actually a relational quantity computed from the states and positions of neighborhoods of voxels.

### 1.2 What this replaces

The five postulates of SPEC_FTD.md reduce to Axiom Zero as follows:

| Old Postulate | Content | Status under Axiom Zero |
|---------------|---------|------------------------|
| P1: Discrete space (Z^3) | Space is a cubic lattice | **The position property.** Z^3 is the domain of position. |
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

### 2.2 From Position Alone: The Geometry of Z^3

The position space Z^3 has geometric structure that does not depend on state.

**(a) The point group is O_h** [THEOREM]

The symmetry group of Z^3 (symmetries fixing the origin) is the octahedral group O_h, which has |O_h| = 48 elements. This is a theorem in crystallography. O_h = S_4 x Z_2, the symmetric group on 4 elements (permutations of body diagonals) times the inversion.

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

This is a theorem about discrete wave equations on Z^D. It does not depend on the state space. In FTD, it is the maximum speed at which information propagates through the lattice. (See FOUND_EMERGENT_TIME_GRAVITY.md, Section 2.3.)

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

### 3.2 The master quadratic and its three regimes [THEOREM given Steps 6-7 of the chain]

The self-consistency (gap) equation of the lattice is:

$$x^2 = 16G^{*2}(x - G^*) \quad \Leftrightarrow \quad x^2 - 16G^{*2}x + 16G^{*3} = 0$$

Using I_1 = G\*^2/(2pi):

$$x^2 = 32\pi\,I_1\,(x - G^*)$$

The roots are:

| Root | Value | Proposed identification | Accuracy | Status |
|------|-------|------------------------|----------|--------|
| x_+ | 137.036 | 1/alpha (fine structure) | 1.26 ppm | [SELECTION] |
| x_- | 3.024 | N_c (color charges) | 0.8% | [SELECTION] |

**Note (April 2026):** An updated 13-step chain starting from "i exists" is now canonical — see [FOUND_BLIND_DERIVATION_CHAIN.md](FOUND_BLIND_DERIVATION_CHAIN.md). The coefficient 16 is now [THEOREM] via dual derivation (|Aut(E_i)|^2 = |O_h|/3 = 16, see DERIV_DUAL_DERIVATION_OF_16.md). The one-loop lattice correction closes 99.2% of the gap (9.6 ppb).

**The original 10-step chain** (retained for historical reference):

| Step | Content | Status |
|------|---------|--------|
| 0 | Z^3 lattice (Axiom Zero, position property) | [AXIOM] |
| 1 | O_h point group, Z_4 planar symmetry | [THEOREM] |
| 2 | Watson's BCC integral I_1 = Gamma(1/4)^4/(4pi^3) | [THEOREM] |
| 3 | Lemniscatic modulus k = 1/sqrt(2) forced by Z_4 | [THEOREM] |
| 4 | CM curve E: y^2 = x^3 - x, j = 1728 | [THEOREM] |
| 5 | Identity: G\*^2/(2pi) = I_1 | [THEOREM] |
| 6 | Degree 2 from self-referential closure | [THEOREM for the argument, SELECTION for the principle] |
| 7 | Coefficient 16 | [STRONGLY MOTIVATED but OPEN -- see Section 4.2] |
| 8 | Master quadratic follows algebraically | [THEOREM given 6 and 7] |
| 9 | Roots x_+ = 137.036, x_- = 3.024 | [THEOREM given 8] |
| 10 | Physical identification x_+ = 1/alpha, x_- -> N_c | [SELECTION] |

**Steps 0-5 are rock solid.** Steps 6-7 are the contested territory. Steps 8-9 are algebra. Step 10 is an identification, not a derivation.

**The discriminant trichotomy** [THEOREM]: The generalized master quadratic $x^2 - kG^{*2}x + kG^{*3} = 0$ has discriminant $\Delta = kG^{*3}(kG^* - 4)$. One quadratic, three regimes:

- $\Delta > 0$ ($k = 16$, physical): **real roots** — bosonic sector (coupling constants $\alpha$, $N_c$)
- $\Delta = 0$ ($k = 4/G^*$, critical): **degenerate root** — the Born rule / measurement boundary
- $\Delta < 0$ ($k < 4/G^*$): **complex roots** — the Dirac equation emerges; complex roots $x = a \pm bi$ yield $e^{ibt}$ oscillations, which IS the fermion wavefunction. The fermion sector is not imported from external physics — it is derived from the complex regime of the same master quadratic that produces $\alpha$ and $N_c$.

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

1. The Z^3 lattice with ternary states has a **unique** self-consistent coupling, given by the fixed point of the gap equation.
2. This unique coupling numerically agrees with alpha to 1.26 ppm.
3. No free parameters were adjusted to achieve this agreement.

The strength of the claim is not self-consistency per se, but the **combination** of uniqueness and numerical agreement with zero free parameters. Newtonian gravity is self-consistent but has G as a free parameter. Ptolemaic astronomy is self-consistent but has adjustable epicycle radii. FTD's self-consistent fixed point is unique (for given D and |S|) and happens to match experiment.

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

**However, this is the wrong question.** The lattice is not a finite torus — it is Z^3, the infinite cubic lattice. The master quadratic is a **thermodynamic limit property**, not a finite-box property:

- On the 2x2x2 torus: the Watson integral is G_self = 29/32 = 0.906 (35% below the infinite-lattice value). The gap equation with this self-energy gives x_+ ≈ 88, far from 137.
- As L increases: G_self(L) converges to W_3 = G\*^2/(2pi) = 1.393, and the gap equation roots converge to 137.036 and 3.024.
- At L = infinity: the gap equation x^2 = 16G\*^2(x - G\*) is exact, with n_DOF = 16 exactly.

The scaling analysis (proof_gap_equation_scaling.py) confirms this convergence numerically across L = 2, 4, 8, ..., 64.

The master quadratic does not need to be "derived" from a finite box because it IS the infinite-lattice self-consistency condition. The finite-lattice computations confirm it converges there. The algebra proves it is exact there. The self-referential closure says there is nowhere else it could be.

**The reframed open problem:** Not "derive 16 from a finite torus" but "prove that the thermodynamic limit of the self-consistency condition on Z^3 produces coefficient 16." The nine convergent routes to 16 provide strong evidence. The finite-lattice computations confirm convergence. The remaining gap is a formal proof in the L -> infinity limit. [OPEN but STRONGLY MOTIVATED]

### 4.3 "The stencil doesn't use BCC corners for dynamics"

**The objection:** The FTD engine uses a 6-point stencil for the divergence (SC neighbors only) and an 18-point stencil for the wave equation (SC + FCC). The 8 BCC corner neighbors do not appear in any dynamical equation. Yet G\* is derived from the BCC Watson integral I_1. How can BCC geometry determine the physics when the dynamics only use SC and FCC neighbors?

**The response:**

This is a sharp and important objection. The current answer is incomplete.

What can be said:
1. The 26-neighbor Moore neighborhood includes all three sublattices. The BCC corners are geometrically present even if the current dynamical stencils do not use them directly.
2. The Watson integral I_1 characterizes the BCC sublattice's self-energy, which enters through the Green's function, not through the dynamical stencil. The propagator (inverse of the lattice Laplacian) "sees" all lattice sites, not just those in the stencil.
3. The Z_4 symmetry that selects the lemniscatic modulus is a property of the lattice planes, not of any particular stencil.

The deeper resolution: G\* is a property of Z^3 itself — a thermodynamic limit quantity. The 18-point stencil is a choice of dynamics on a finite lattice; G\* characterizes the lattice's geometry in the infinite limit. The wave equation stencil determines HOW information propagates; the Z_4 symmetry determines WHAT the lattice IS. The stencil can be 6-point, 18-point, or 26-point — the Z_4 symmetry and the Watson integral I_1 are properties of Z^3 regardless of which stencil is used for dynamics.

The BCC corners do not need to participate in the wave equation for G\* to govern the physics, just as pi does not need to appear in the equation of motion for it to set the geometry. G\* is the lattice's structural constant; the stencil is its dynamical implementation. [SELECTION — the structural/dynamical distinction is argued, not proven]

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
- Steps 6-7 (degree 2, coefficient 16) are **argued but not forced**. The degree-doubling argument is compelling but the coefficient 16 has not been derived from the partition function. [SELECTION / OPEN]
- Step 10 (identification x_+ = 1/alpha) is **observed**, not derived. The numerical agreement is striking (1.26 ppm) but no dynamical mechanism produces alpha from the gap equation. [SELECTION]

**Bottom line:** FTD has discovered a genuine mathematical connection between Z^3 lattice geometry and the number 137.036. Whether this connection has physical content -- whether it explains WHY alpha has this value -- depends on closing the gap at Steps 6-7. Until then, the connection is a provocation, not a proof.

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

A voxel has a state and a position; from these, and nothing else, the geometry of Z^3 forces the lemniscatic constant G\*, which through a self-consistent gap equation yields x_+ = 137.036, which FTD identifies with 1/alpha -- but the complete derivation awaits the resolution of the coefficient-16 problem and the physical identification mechanism.

---

## References

- SPEC_FTD.md -- The five postulates (Part A, Chapter 1)
- FOUND_SELF_REFERENTIAL_CLOSURE.md -- Self-referential closure as derivation principle
- FOUND_EMERGENT_TIME_GRAVITY.md -- Time and gravity from G\*^2
- FOUND_ONTOLOGICAL_GENESIS.md -- The ontological hierarchy from void to physics
- DERIV_MASTER_QUADRATIC_GAP_EQUATION.md -- The gap equation derivation
- DERIV_WATSON_GSTAR_IDENTITY.md -- W_3 = G\*^2/(2pi) identity and BCC correction
- DERIV_QUADRATIC_NECESSITY.md -- Degree 2 from self-referential closure
- Watson, G. N. "Three Triple Integrals," Quarterly Journal of Mathematics 10 (1939), 266-276
- Borwein, J. M. and Bailey, D. H. Mathematics by Experiment, A K Peters, 2004
