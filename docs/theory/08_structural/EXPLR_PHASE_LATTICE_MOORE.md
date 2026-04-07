# The Phase Lattice {pi, varpi, G*}^3: Phase-Space Avatar of the Moore Neighborhood

## Ternary Phase Cube on (S^1)^3

**Date:** April 5, 2026
**Framework:** Foundational Ternary Dynamics v5.29
**Document Status:** Exploratory -- mathematical construction with structural isomorphism
**Epistemic Class:** [THEOREM] for algebraic identities and Laplacian spectrum; [SELECTION] for Moore mapping; [CONJECTURE] for physical significance
**Category:** 8 (Structural / Geometry)

---

## Depends On

- [FOUND_EULER_IDENTITY_TERNARY.md](../02_foundations/FOUND_EULER_IDENTITY_TERNARY.md) -- Ternary states {-1,0,+1} on the unit circle
- [MATH_MASTER_QUADRATIC.md](../01_reference/MATH_MASTER_QUADRATIC.md) -- Master quadratic roots x+, x-
- [MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md](../01_reference/MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md) -- G* as the bridge between dispositional and actual
- [DERIV_WATSON_GSTAR_IDENTITY.md](../04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md) -- Watson BCC integral W_3 = G*^2/(2*pi)
- [EXPLR_HALF_MOBIUS_LEMNISCATE.md](../09_mathematical/EXPLR_HALF_MOBIUS_LEMNISCATE.md) -- Z_4 symmetry of the lemniscatic period lattice

---

## Honesty Note

The construction of the phase lattice and its bijection to the Moore neighborhood are mathematically exact. The Laplacian eigenvalue results (7, 13, 27 appearing in the spectrum; 13 distinct eigenvalues) are numerical facts about the Moore adjacency matrix, not specific to FTD. The identification of these numbers with FTD framework integers is [SELECTION] -- structurally compelling but not uniquely forced. The physical interpretation of the phase clustering and the G*-at-center mapping is [CONJECTURE].

---

## Abstract

The three FTD ontic constants {varpi, G*, pi} -- where varpi = 2.622 is the lemniscate constant, G* = 2.959 is the bridge constant, and pi = 4*varpi^2/G*^2 is derived -- serve as independent phase angles on the unit circle S^1. The exponentiated alphabet E = {e^{i*pi}, e^{i*varpi}, e^{i*G*}} generates a 27-state phase lattice L_3(E) = E^3 in the 3-torus (S^1)^3.

This lattice maps bijectively to the 27 cells of the Moore neighborhood via the ontic ordering varpi -> -1, G* -> 0, pi -> +1, placing the bridge constant at the center. The Moore Laplacian on this lattice has exactly N_eff = 13 distinct eigenvalues, with b_3 = 7, N_eff = 13, and N_c^3 = 27 appearing as exact eigenvalues.

**Computation:** `scripts/exploration/phase_lattice_27.py`

---

## 1. The Phase Alphabet [THEOREM]

### 1.1 Three Phases on S^1

Using three FTD constants as phase angles:

| Constant | Value (rad) | Value (deg) | e^{i*theta} |
|----------|------------|-------------|--------------|
| varpi | 2.62206 | 150.23 | -0.8681 + 0.4965i |
| G* | 2.95868 | 169.52 | -0.9833 + 0.1819i |
| pi | 3.14159 | 180.00 | -1.0000 + 0.0000i |

All three lie in the second quadrant (Re < 0, Im >= 0), spanning an arc of 29.77 degrees. The ontic ordering (most primitive to most derived) matches the angular ordering: varpi < G* < pi.

### 1.2 Phase Distinctness [THEOREM]

The pairwise separations are:

- G* - varpi = 0.3366 rad (19.29 deg)
- pi - G* = 0.1829 rad (10.48 deg)
- pi - varpi = 0.5195 rad (29.77 deg)

All three are distinct mod 2*pi. Since G* is transcendental and algebraically independent of pi (Nesterenko 1996), and varpi = G*sqrt(pi)/2, the three constants are genuinely independent as phases.

---

## 2. The 27-State Lattice [THEOREM]

The lattice L_3(E) = E^3 has 27 states in (S^1)^3. Each state is a triple (e^{i*a_1}, e^{i*a_2}, e^{i*a_3}) with a_j in {varpi, G*, pi}.

Under the S_3 axis-permutation symmetry (order 6), the 27 states decompose into 10 orbits:

| Type | Orbits | Size | Total | Example |
|------|--------|------|-------|---------|
| All same | 3 | 1 | 3 | (G*, G*, G*) |
| Two equal | 6 | 3 | 18 | (varpi, G*, G*) |
| All distinct | 1 | 6 | 6 | (varpi, G*, pi) |

---

## 3. Moore Neighborhood Mapping [SELECTION]

### 3.1 The Bijection

The ontic ordering provides a natural bijection to the 3x3x3 Moore neighborhood:

- varpi -> offset -1 (most primitive)
- G* -> offset 0 (bridge, center)
- pi -> offset +1 (most derived)

This places G* at the **center** of the Moore neighborhood -- the bridge constant mediates from the origin.

### 3.2 Shell Decomposition

| Shell | Count | Phase content | Geometric role |
|-------|-------|---------------|----------------|
| Center (d=0) | 1 | (G*, G*, G*) | Pure bridge |
| SC / Octahedron (d=1) | 6 | Two G* + one extreme | Bridge mediating one axis |
| FCC / Cuboctahedron (d=sqrt2) | 12 | One G* + two extremes | Mixed |
| BCC / Cube corners (d=sqrt3) | 8 | Zero G* -- pure {varpi, pi} | Extremes only |

### 3.3 BCC Corners and the Watson Integral

The 8 BCC corners are exactly the states that **avoid G* entirely**. They use only {varpi, pi} -- the two "extreme" phases at opposite ends of the ontic arc. This connects to the Watson BCC integral: W_3 = G*^2/(2*pi) = 1.393 emerges from the BCC sublattice, which in the phase picture corresponds to the states that do not carry the bridge constant on any axis.

The bridge constant G* acts from the center, mediating between the extremes that generate the BCC self-energy.

### 3.4 Stella Octangula

The 8 BCC corners split into two interlocking tetrahedra by parity:

- T+ (product of offsets > 0): {(-1,-1,+1), (-1,+1,-1), (+1,-1,-1), (+1,+1,+1)} = 4 states
- T- (product of offsets < 0): {(-1,-1,-1), (-1,+1,+1), (+1,-1,+1), (+1,+1,-1)} = 4 states

T+ contains the state (pi, pi, pi) = (-1,-1,-1) in the standard ternary basis.
T- contains the state (varpi, varpi, varpi) = the pure lemniscatic corner.

---

## 4. Laplacian Eigenvalue Analysis [THEOREM]

### 4.1 The Unweighted Moore Laplacian

The 27x27 Moore adjacency matrix (open boundary, 3x3x3) has degrees:
- Center: 26 (connected to all)
- SC: 17
- FCC: 11
- BCC: 7

Total edges: 158.

The Laplacian L = D - A has eigenvalue spectrum with **exactly 13 distinct values** and multiplicities:

| Eigenvalue | Multiplicity | Note |
|-----------|-------------|------|
| 0.0000 | 1 | Trivial (connected graph) |
| 5.1215 | 3 | Spectral gap |
| 6.5505 | 3 | |
| **7.0000** | **1** | **= b_3** |
| 9.2583 | 1 | |
| 10.1865 | 3 | |
| 11.4495 | 3 | |
| 12.6834 | 2 | |
| **13.0000** | **3** | **= N_eff** |
| 16.7417 | 1 | |
| 17.6919 | 3 | |
| 19.3166 | 2 | |
| **27.0000** | **1** | **= N_c^3 = lattice size** |

**Key findings:**
1. Three framework integers appear as **exact eigenvalues**: b_3 = 7, N_eff = 13, N_c^3 = 27
2. The number of distinct eigenvalues is **13 = N_eff**
3. The maximum multiplicity is **3 = N_c** (reflecting S_3 symmetry)
4. The spectral gap ratio lambda_max/lambda_1 = 5.272

### 4.2 Phase-Weighted Laplacian

Weighting edges by the Euclidean distance between states in C^3 breaks the triplet degeneracies, producing 19 distinct eigenvalues (max multiplicity 2). The framework integers no longer appear as exact eigenvalues -- they are properties of the **unweighted** (topological) Moore structure, not the metric structure.

---

## 5. Master Quadratic Connections [CONJECTURE]

### 5.1 Phase Sums

The total phase angle sum(theta_1 + theta_2 + theta_3) ranges from 3*varpi = 7.866 (BCC minimum) to 3*pi = 9.425 (BCC maximum), with the center at 3*G* = 8.876.

### 5.2 Key Identity

The center state phase sum satisfies:

**sum(center) / G* = 3*G* / G* = 3 = N_c** (exactly, trivially)

**16*G* / sum(center) = 16/3** (exactly)

### 5.3 Inverse Gap Equation

Solving F(x) = s for x gives x = 16*G*^3 / (16*G*^2 - s). All 10 unique phase sums map to x values near **pi** (ranging 3.13 to 3.17), placing them in the neighborhood of the fundamental circle constant.

---

## 6. Interpolation Ratio

The position of G* within the ontic arc [varpi, pi] is:

r = (G* - varpi) / (pi - varpi) = 0.6479...

**Exact algebraic form:**

r = G*(2 - sqrt(pi)) / (2*pi - sqrt(pi)*G*)

This is NOT equal to 2/3 (diff = 2.81%). The best simple rational approximation is 11/17 (error 0.086%). The continued fraction is [0; 1, 1, 1, 5, 3, 1, 5, 5, ...] with no obvious pattern. The ratio is transcendental (depending on G* and pi).

---

## 7. Distance Spectrum

The 351 pairwise Euclidean distances have 19 unique values. The lattice is **tightly clustered**: mean distance is only 20.6% of the mean for an equally-spaced reference lattice {0, 2*pi/3, 4*pi/3}^3.

---

## 8. Multiplicative Semigroup [THEOREM]

The alphabet {e^{i*pi}, e^{i*varpi}, e^{i*G*}} is NOT closed under multiplication. Since varpi/pi is irrational (Nesterenko 1996), the additive semigroup of phases is dense in [0, 2*pi). The phase lattice is a **generating set**, not a finite group -- it seeds a dense filling of the circle.

---

## Summary Table

| Property | Value | Status |
|----------|-------|--------|
| Lattice size | 27 = N_c^3 | [THEOREM] |
| Phase alphabet | {e^{i*pi}, e^{i*varpi}, e^{i*G*}} | [THEOREM] |
| Arc span | 29.77 deg | [THEOREM] |
| S_3 orbits | 10 | [THEOREM] |
| Moore center | G* | [SELECTION] |
| BCC = G*-free states | 8 | [THEOREM] |
| Distinct Laplacian eigenvalues | 13 = N_eff | [THEOREM] |
| Exact integer eigenvalues | 7, 13, 27 | [THEOREM] |
| Interpolation ratio | 0.6479 (not 2/3) | [THEOREM] |
| Semigroup density | Dense in S^1 | [THEOREM] |
