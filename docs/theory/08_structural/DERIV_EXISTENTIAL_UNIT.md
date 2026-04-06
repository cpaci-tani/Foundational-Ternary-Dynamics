# The Existential Unit: The 3^3 Minimal Complete Lattice

**Status:** v1.0 | April 5, 2026 | [THEOREM] (Sections 1-8), [CONJECTURE] (Section 9)
**Proof script:** `scripts/exploration/lattice_3x3x3_symmetries.py`
**Dependencies:** DERIV_MOORE_GAUGE_STRUCTURE, DERIV_CUBOCTAHEDRAL_INTEGERS, DERIV_STABILIZER_DECOMPOSITION

---

## Abstract

The 3x3x3 cubic lattice is the **minimal complete lattice**: the smallest periodic cubic lattice in which a center point possesses a full, non-self-intersecting 26-neighbor Moore neighborhood. We prove that the requirement for the four framework integers {N_c, N_base, b_3, N_eff} to sum to the lattice volume N_c^3 uniquely selects N_c = 3, given |Aut(E_i)| = 4 from the CM elliptic curve y^2 = x^3 - x. This self-consistency condition, combined with the O_h representation theory of the 27-site permutation representation, recovers all framework integers as structural invariants of the lattice geometry.

---

## 1. The Minimal Complete Lattice [THEOREM]

**Definition.** A *complete lattice* is a periodic cubic lattice Z_L^3 in which at least one site has a Moore neighborhood (26 nearest neighbors at distances 1, sqrt(2), sqrt(3)) consisting of 26 *distinct* sites with no self-identification under periodic boundary conditions.

**Theorem 1.1.** The minimal complete lattice is L = 3.

*Proof.* At L = 2, the lattice Z_2^3 has 8 sites. The Moore neighborhood of any site requires neighbors at offsets (+/-1, 0, 0), (0, +/-1, 0), etc. Under periodic identification modulo 2, the offset +1 and -1 map to the same site (since 1 = -1 mod 2). Therefore every "pair" of opposing neighbors collapses to a single site. The Moore neighborhood is not 26 distinct sites but a smaller set with multiplicities. More precisely, every site is a neighbor of every other site — the graph is complete on 8 vertices, and no site has a "non-neighbor."

At L = 3, the lattice Z_3^3 has 27 sites. The center point (1,1,1) has offsets in {-1, 0, +1}^3 \ {(0,0,0)}, which are all distinct modulo 3 (since no offset component exceeds 1 in absolute value). The 26 neighbors are 26 distinct sites. The center has a full, non-degenerate Moore neighborhood. QED.

**Corollary 1.2.** The 3^3 lattice is the unique lattice where a center point has a complete Moore neighborhood that coincides with all other lattice sites: 1 (center) + 26 (neighbors) = 27 = 3^3. The center's Moore neighborhood IS the rest of the lattice.

---

## 2. Shell Decomposition [THEOREM]

The 26 neighbors of the center point (1,1,1) decompose into three concentric shells under the octahedral point group O_h:

| Shell | Sites | Distance | Geometry | Sublattice |
|-------|-------|----------|----------|------------|
| 1 | 6 | 1 | Octahedron | Simple Cubic (SC) |
| 2 | 12 | sqrt(2) | Cuboctahedron | Face-Centered Cubic (FCC) |
| 3 | 8 | sqrt(3) | Cube | Body-Centered Cubic (BCC) |

**Total:** 1 + 6 + 12 + 8 = 27 = 3^3 = N_c^3.

The 8 cube-corner sites decompose further into two interlocking tetrahedra (stella octangula):

- T+ (even parity): {(0,0,0), (0,2,2), (2,0,2), (2,2,0)} — 4 sites
- T- (odd parity): {(2,2,2), (2,0,0), (0,2,0), (0,0,2)} — 4 sites

Each tetrahedron has **4 = N_base = |Aut(E_i)|** vertices.

---

## 3. The Self-Consistency Equation [THEOREM]

**Definitions.** Given the CM elliptic curve E_i: y^2 = x^3 - x with |Aut(E_i)| = 4:

- N_c = color charge number (to be determined)
- N_base = |Aut(E_i)| = 4
- b_3 = N_c + N_base (third Betti number)
- N_eff = N_c^2 + N_base (effective degrees of freedom)

**Theorem 3.1.** The condition that the framework integers fill the minimal complete lattice:

    N_c + N_base + b_3 + N_eff = N_c^3

has a unique positive real solution: **N_c = 3**.

*Proof.* Substituting the definitions:

    N_c + 4 + (N_c + 4) + (N_c^2 + 4) = N_c^3
    N_c^2 + 2*N_c + 12 = N_c^3
    N_c^3 - N_c^2 - 2*N_c - 12 = 0

Factoring: **(N_c - 3)(N_c^2 + 2*N_c + 4) = 0**

The quadratic factor has discriminant 4 - 16 = -12 < 0 (no real roots). Therefore the unique positive real solution is **N_c = 3**. QED.

**Corollary 3.2.** The framework integers are uniquely determined:
- N_c = 3 (color charges)
- N_base = 4 (automorphism multiplicity)
- b_3 = 7 (Betti number)
- N_eff = 13 (effective DOF)

And these are the shell sizes: 1 (center) + 6 (oct) + 12 (cuboct) + 8 (cube) repackaged as 3 + 4 + 7 + 13 = 27. The framework integers literally *count the sites* when distributed across the shells.

---

## 4. O_h Representation Theory [THEOREM]

The 27 lattice sites carry a permutation representation of the octahedral group O_h (order 48). By computing the character on each conjugacy class and decomposing:

**Theorem 4.1.** The 27-dimensional permutation representation decomposes as:

    27 = 4*A1g + A2u + 2*Eg + T1g + T2g + 3*T1u + T2u

where A1g, A2g, Eg, T1g, T2g, A1u, A2u, Eu, T1u, T2u are the 10 irreducible representations of O_h.

**Per shell:**

| Shell | Sites | O_h decomposition |
|-------|-------|-------------------|
| Center | 1 | A1g |
| Octahedron | 6 | A1g + Eg + T1u |
| Cuboctahedron | 12 | A1g + Eg + T1g + T1u + T2u |
| Cube | 8 | A1g + A2u + T1u + T2g |

**Theorem 4.2.** The multiplicities of the irreducible representations reproduce the framework integers:

| Quantity | Value | Framework integer |
|----------|-------|-------------------|
| Number of A1g (scalar) reps | 4 | N_base = |Aut(E_i)| |
| Number of T1u (vector) reps | 3 | N_c |
| Number of distinct irreps appearing | 7 | b_3 |
| Total dimension in triplet reps | 18 | 18-point stencil |
| Number of singlet types (1-dim reps) | 5 | Number of FTD postulates |

---

## 5. Parity Decompositions [THEOREM]

### 5.1 Inversion Parity (Gerade/Ungerade)

The 26-dimensional neighbor representation (27 minus center) splits under the inversion element of O_h:

    Gerade (even): 3*A1g + 2*Eg + T1g + T2g = 3 + 4 + 3 + 3 = 13
    Ungerade (odd): A2u + 3*T1u + T2u = 1 + 9 + 3 = 13

**Theorem 5.1.** The Moore neighborhood splits as 13 + 13 = N_eff + N_eff under inversion parity.

### 5.2 Translational Parity (BCC/FCC Split)

Sites classified by (x + y + z) mod 2:

| Parity | Sites | Shell composition |
|--------|-------|-------------------|
| Even | 13 | 1 (center) + 12 (cuboctahedron) |
| Odd | 14 | 6 (octahedron) + 8 (cube) |

The center point (1,1,1) has parity 3 mod 2 = 1 (ODD) on the original lattice coordinates, but when centered at origin: (0,0,0) has parity 0 (EVEN). The structural result:

**Theorem 5.2.** The 27 sites of the 3^3 lattice split under translational parity as 13 + 14, where:

- The 13-site sublattice contains the center + cuboctahedron
- The 14-site sublattice contains the octahedron + cube = 6 + 8 = 14 = 2*b_3

---

## 6. Vieta Structure of the Framework Polynomial [THEOREM]

**Definition.** The framework polynomial is P(x) = (x - N_c)(x - N_base)(x - b_3)(x - N_eff) = (x-3)(x-4)(x-7)(x-13).

**Theorem 6.1.** The elementary symmetric polynomials of the framework integers are:

    e_1 = 3 + 4 + 7 + 13 = 27 = N_c^3 = 3^3
    e_2 = 3*4 + 3*7 + 3*13 + 4*7 + 4*13 + 7*13 = 243 = N_c^5 = 3^5
    e_3 = 3*4*7 + 3*4*13 + 3*7*13 + 4*7*13 = 877
    e_4 = 3*4*7*13 = 1092

The first two Vieta coefficients are exact powers of N_c with exponents 3 = D and 5 = D + 2.

**Corollary 6.2.** P(x) = x^4 - 27x^3 + 243x^2 - 877x + 1092, and since 27 | 27 and 27 | 243:

    P(x) mod 27 = x^4 + 14x + 12 = (x - 1)(x - 3)(x^2 + 4x + 13) mod 27

The complex roots of x^2 + 4x + 13 are -2 +/- 3i, with modulus |root| = sqrt(4 + 9) = sqrt(13) = sqrt(N_eff).

---

## 7. Stabilizer Structure [THEOREM]

By the orbit-stabilizer theorem (|O_h| = |orbit| * |stabilizer|):

| Shell | Orbit size | Stabilizer | |Stab| |
|-------|-----------|------------|--------|
| Center | 1 | O_h | 48 |
| Octahedron | 6 | C_4v | 8 |
| Cuboctahedron | 12 | C_2v | 4 |
| Cube | 8 | C_3v | 6 |

**Theorem 7.1.** The stabilizer of a cuboctahedral site has order 4 = N_base = |Aut(E_i)|. This is the shell where the CM curve automorphism group acts most directly as a geometric stabilizer.

---

## 8. Full Symmetry Group [THEOREM]

The 3^3 periodic torus has symmetry group (Z/3Z)^3 (semi-direct) O_h.

**Theorem 8.1.** The full symmetry group has order:

    |G| = 27 * 48 = 1296 = 2^4 * 3^4 = 16 * 81 = |Aut(E_i)|^2 * N_c^4

Equivalently: 1296 = 6^4 = (2*N_c)^4. The symmetry group order is a perfect fourth power.

---

## 9. Physical Interpretation: The Existential Unit [CONJECTURE]

The 3^3 lattice is proposed as the **existential unit** of FTD: the minimal geometric structure that self-consistently determines all framework integers from the single input |Aut(E_i)| = 4.

The ontological reading:
- The center point is the locus of the CM point i, the seat of observation
- The octahedron (6 faces, SC) provides the 3 spatial axes (each axis has 2 directions)
- The cuboctahedron (12 edges, FCC) provides the 12 edge-diagonal propagation channels
- The cube (8 corners, BCC) provides the 2 tetrahedra whose self-energy equals G*^2/(2*pi)
- Together: 1 observer + 26 observable directions = 27 = N_c^3 possibilities

The ternary state space is 3^27 = 7,625,597,484,987 configurations — a triple-exponential tower: N_c -> N_c^3 -> N_c^(N_c^3).

Whether this minimal structure has physical significance beyond its mathematical beauty remains an open question. The self-consistency equation is a theorem; its interpretation as an ontological primitive is a conjecture.

---

## Summary of Results

| Result | Status | Section |
|--------|--------|---------|
| 3^3 is the minimal complete lattice | [THEOREM] | 1 |
| Shell decomposition 1+6+12+8 = 27 | [THEOREM] | 2 |
| N_c = 3 uniquely from self-consistency | [THEOREM] | 3 |
| O_h irrep decomposition with framework integers | [THEOREM] | 4 |
| 13+13 parity split = N_eff + N_eff | [THEOREM] | 5 |
| Vieta coefficients e_1 = 3^3, e_2 = 3^5 | [THEOREM] | 6 |
| Stabilizer of cuboctahedron = |Aut(E_i)| | [THEOREM] | 7 |
| |G| = |Aut|^2 * N_c^4 = 6^4 | [THEOREM] | 8 |
| Existential unit interpretation | [CONJECTURE] | 9 |
