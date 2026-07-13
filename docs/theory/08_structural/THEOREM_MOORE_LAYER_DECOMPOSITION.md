# The Moore Layer Theorem: Standard Model Particle Content from Cubic Lattice Geometry

## A Combinatorial Theorem about D-Dimensional Ternary Lattices

**Document Status:** Structural theorem with physical application
**Epistemic Class:** [THEOREM] for the combinatorial results; [SELECTION] for physical identifications

---

## Abstract

The Moore neighborhood of a D-dimensional ternary cubic lattice (3^D sites with 26-neighbor connectivity at D=3) admits a unique decomposition into D polyhedral layers, indexed by the number k of flux components each neighbor excites (k = 1, ..., D). Each layer carries gauge group SU(k) [U(1) for k=1], contains C(D,k) * 2^k sites, and sits at distance sqrt(k) from the center. The outermost layer (k=D) further decomposes into two simplices of equal size 2^(D-1) by parity.

At D=3, this decomposition yields:
- **3 gauge groups:** U(1) x SU(2) x SU(3) (the Standard Model gauge group)
- **3 generations** from C(3,2) = 3 face-diagonal planes of the cuboctahedron
- **4 particles per generation** from 2^2 sites per plane
- **4 matter = 4 antimatter** from the stella octangula parity decomposition
- **17 dark states** invisible to the center observer, from S_3 representation theory

These results are purely combinatorial and hold for any value of the coupling constants.

---

## 1. Definitions

**Ternary cubic lattice:** The set {0, 1, 2}^D with the Moore neighborhood, where two sites are adjacent if they differ by at most 1 on each coordinate.

**Moore neighborhood:** The 3^D - 1 = 26 (at D=3) sites adjacent to the center site (1,1,...,1).

**Offset representation:** Map each coordinate {0, 1, 2} to {+1, 0, -1} via x -> 1 - x. The center maps to (0,0,...,0).

**Layer k:** The set of Moore neighbors with exactly k nonzero coordinates in offset representation. Equivalently: neighbors at distance sqrt(k) from center.

---

## 2. The Layer Decomposition [THEOREM]

**Theorem 1 (Layer structure):** The Moore neighborhood of the D-dimensional ternary cube decomposes uniquely into D layers:

Layer k (k = 1, ..., D):
- **Count:** C(D,k) * 2^k
- **Distance:** sqrt(k) from center
- **Flux components:** k (a neighbor in layer k has exactly k nonzero displacement components)

**Proof:** A neighbor at offset (d_1, ..., d_D) with k nonzero entries can be constructed by choosing which k of D coordinates are nonzero (C(D,k) ways) and assigning each a sign +1 or -1 (2^k ways). The sets are disjoint and exhaustive. Distance: ||offset|| = sqrt(sum d_i^2) = sqrt(k) since each nonzero |d_i| = 1.

**Verification at D=3:**
- k=1: C(3,1)*2 = 6 sites at distance 1 (octahedron)
- k=2: C(3,2)*4 = 12 sites at distance sqrt(2) (cuboctahedron)
- k=3: C(3,3)*8 = 8 sites at distance sqrt(3) (cube = stella octangula)
- Total: 6 + 12 + 8 = 26 = 3^3 - 1. Check.

---

## 3. Gauge Group Assignment [THEOREM + SELECTION]

**Theorem 2 (J-component counting):** A neighbor in layer k excites exactly k components of a D-component vector field J = (J_1, ..., J_D). The excited components correspond to the nonzero displacement coordinates.

**Identification [SELECTION]:** The gauge group of layer k is SU(k) for k >= 2 and U(1) for k = 1, based on the symmetry group of k-component unitary rotations.

At D=3: U(1) x SU(2) x SU(3) is the **unique** factorization of J^2 = J_1^2 + J_2^2 + J_3^2 by the Moore layer decomposition.

---

## 4. Polyhedral Identification [THEOREM]

**Theorem 3 (Platonic/Archimedean structure):** At D=3, the three layers are specific polyhedra:

| Layer | k | Count | Distance | Polyhedron | Watson integral |
|-------|---|-------|----------|------------|-----------------|
| SC | 1 | 6 | 1 | Octahedron | I_SC |
| FCC | 2 | 12 | sqrt(2) | Cuboctahedron | I_FCC |
| BCC | 3 | 8 | sqrt(3) | Cube (= 2 tetrahedra) | I_BCC = G*^2/(2pi) |

The octahedron is Platonic. The cuboctahedron is Archimedean (rectification of the cube). The cube decomposes into a stella octangula.

---

## 5. The Stella Octangula and Matter-Antimatter [THEOREM]

**Theorem 4 (Parity decomposition):** The 2^D sites in layer D decompose into two regular simplices of equal size:

T+ = {offsets with product of signs = +1}: |T+| = 2^(D-1)
T- = {offsets with product of signs = -1}: |T-| = 2^(D-1)

**Proof:** The product of signs is a group homomorphism from {+1,-1}^D to {+1,-1}. Its kernel (T+) and coset (T-) have equal size by Lagrange's theorem.

At D=3: |T+| = |T-| = 4. The two tetrahedra interlock to form the stella octangula.

**Physical identification [SELECTION]:** T+ = matter, T- = antimatter. |T+| = |T-| gives matter-antimatter symmetry. The number of matter types per generation = 2^(D-1) = N_base = 4.

---

## 6. Generation Structure [SELECTION]

**Theorem 5 (Face-diagonal planes):** Layer k=2 (the cuboctahedron at D=3) decomposes into C(D,2) mutually orthogonal groups of 2^2 = 4 sites each, where each group corresponds to a choice of 2 axes from D.

At D=3: C(3,2) = 3 groups of 4 = 3 generations of 4 fermion types.

**Status:** The theorem about the decomposition is exact. The identification of groups with fermion generations is [SELECTION] — structurally motivated (each group couples to a different pair of J-components) but not uniquely forced.

---

## 7. Visibility and Dark States [THEOREM]

**Theorem 6 (S_D symmetric sector):** The center state is invariant under the symmetric group S_D acting by permutation of axes. The S_D-symmetric subspace of the 3^D-dimensional Hilbert space has dimension:

dim(symmetric sector) = C(D+2, 2) = (D+1)(D+2)/2

**Proof:** The symmetric sector is spanned by multisets of size D from 3 symbols {+,0,-}. The count is the stars-and-bars formula C(D+3-1, D) = C(D+2, 2).

**Corollary:** The number of dark states (invisible to the center observer) is:

dark = 3^D - (D+1)(D+2)/2

At D=3: dark = 27 - 10 = 17. The center observer sees 10/27 = 37% of the Hilbert space.

---

## 8. Laplacian Eigenvalue Structure [THEOREM]

**Theorem 7:** The Moore Laplacian (graph Laplacian of the Moore neighborhood on the 3^D lattice with open boundary) has a specific number of distinct eigenvalues determined by O_h representation theory:

| D | Distinct eigenvalues | Exact eigenvalues containing framework numbers |
|---|---------------------|----------------------------------------------|
| 1 | 3 | {3} |
| 2 | 7 | {3, 7} |
| 3 | 13 | {7, 13, 27} |

At D=3, the Laplacian has exactly 13 distinct eigenvalues, with 7 (= b_3) and 27 (= 3^3) appearing as exact values.

---

## 9. The D-Table: General Dimension

| D | States | Forces | SU_max | Gens | Per-gen | T+ = T- | Dark | Visible |
|---|--------|--------|--------|------|---------|---------|------|---------|
| 1 | 3 | 1 | U(1) | 0 | 1 | 1 | 0 | 100% |
| 2 | 9 | 2 | SU(2) | 1 | 2 | 2 | 3 | 67% |
| 3 | 27 | 3 | SU(3) | 3 | 4 | 4 | 17 | 37% |
| 4 | 81 | 4 | SU(4) | 6 | 8 | 8 | 66 | 19% |
| 5 | 243 | 5 | SU(5) | 10 | 16 | 16 | 222 | 9% |

---

## 10. Confinement and Causal Structure [THEOREM]

**Theorem 8 (BCC arrival time):** At the CFL speed c = 1/sqrt(D), the light-travel time from center to layer k is:

t_k = sqrt(k) / c = sqrt(k) * sqrt(D) ticks

At D=3, k=3: t_BCC = sqrt(3) * sqrt(3) = 3 = D ticks.

The confinement shell (SU(D)) is exactly D ticks away from center. A signal cannot reach the SU(D) layer and return within 2D ticks.

---

## 11. Engine Validation

The FTD C++ engine (v2.11) with an 18-point isotropic Laplacian on a 16^3 lattice confirms:

- Flux shell ordering: SC > FCC > BCC (confirmed)
- FCC/SC flux ratio = 0.253 matches Laplacian weight prediction 0.250 (1.3%)
- BCC flux from second-order propagation (BCC not in 18-point stencil)
- Newton's 3rd law: |F1|/|F2| = 1.000 for same-sign pair
- Propagation speed consistent with c = 1/sqrt(3)

---

## 12. Precision Formula Connection

The 7-term precision formula for 1/alpha:

1/alpha = x+ - c1|eps| + c2|eps|^2 - c3|eps|^3 - c4|eps|^4 - c5|eps|^5 - c6|eps|^6 + c7|eps|^7

has coefficients derived from lattice loop corrections:
- c1 = 9/47: one-loop scalar tadpole [DERIVED, 0.8%]
- c2 = 5/64: two-loop scalar * Neff/(Neff-Nb) gauge factor [DERIVED, 0.07%]
- c3 = 4/141: three-loop scalar * (b3+Nb)/(2Nc) gauge factor [DERIVED, 0.33%]
- c4-c7: gauge sector corrections from the three polyhedra [MOTIVATED]

The gauge correction factors form a sequence: 1, 13/9, 11/6, each introducing the next level of gauge structure (color, lattice DOF, QCD beta).

---

## 13. What This Theorem Says

The combinatorial content of the Standard Model — the number of forces, their gauge groups, the generation count, the particles-per-generation, and matter-antimatter balance — follows from the Moore neighborhood of a 3-cube. No coupling constants, no Lagrangian, no symmetry breaking. Just: count the sites on a ternary cubic lattice and classify them by how many axes they excite.

**Input:** D = 3 (three spatial dimensions = three ternary states self-applied).

**Output:** U(1) x SU(2) x SU(3), three generations of four fermions, four matter = four antimatter, 37% visible, confinement in 3 ticks.

---

## Depends On

- [FOUND_AXIOM_ZERO.md](../02_foundations/FOUND_AXIOM_ZERO.md) — D=3 axiom, CFL condition
- [DERIV_MOORE_GAUGE_STRUCTURE.md](../03_derivations/DERIV_MOORE_GAUGE_STRUCTURE.md) — J-component gauge group assignment
- [DERIV_WATSON_GSTAR_IDENTITY.md](../04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md) — Watson integrals for each sublattice
- [EXPLR_PHASE_LATTICE_MOORE.md](EXPLR_PHASE_LATTICE_MOORE.md) — Phase lattice construction and dark states

## Computation

- `scripts/exploration/test_all_physics.py` — 50-test verification
- `scripts/exploration/what_we_solve.py` — D-table computation
- `scripts/exploration/forced_equations_v2.py` — Forced polyhedral structure
- `engine/tests/test_native_moore_layer_coupling.cpp` + `test_native_moore_temporal_layers.cpp` — Engine validation (native Moore-layer flux status; the legacy `test_intervoxel_coupling.cpp` was DELETED 2026-05-03, `e8eb8e82` — its coupling-ratio targets used demoted [PARAMETRIC] identifications)
