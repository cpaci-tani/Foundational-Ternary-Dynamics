# The Born Rule as Null-Cone Geometry

## From i² = −1 to P = |ψ|² via a Single Equation

**Date:** March 16, 2026
**Status:** Foundational derivation with honest epistemic assessment
**Dependencies:** FOUND_THE_COMPLETE_ALGEBRA_OF_i.md, FOUND_THE_EXISTENCE_FILTER.md, DERIV_QUADRATIC_NECESSITY.md
**Extends:** The Existence Filter result P = E(x)² + E(ix)² = |x|²

---

## Abstract

The Born rule P = |ψ|² is conventionally treated as an axiom of quantum mechanics. FTD's Existence Filter (FOUND_THE_EXISTENCE_FILTER.md) shows it equals the Pythagorean sum E(x)² + E(ix)², but does not explain why the form must be quadratic. This document shows that a single equation —

$$i^2 + a^2 + b^2 = 0 \tag{*}$$

— simultaneously encodes the Born rule, the Pythagorean theorem, the unit circle, the Riemann sphere, the null cone of (1+2)D Minkowski space, and the Wick rotation between Euclidean and Lorentzian signatures. The quadratic form of the Born rule is not arbitrary: it is the unique norm compatible with the null-cone geometry that i² = −1 creates.

> **Epistemic scope [LEDGER FTD-0187].** This document addresses the *form* question — why the Born functional is the quadratic norm |ψ|² rather than |ψ| or |ψ|⁴ — and contributes a structural-uniqueness argument (the null-cone norm). That places the |ψ|²-*form* at `[SELECTION]` grade. It does **not** address, and does not derive, the load-bearing dynamical step *probability = normalized energy density* (target T1c), which remains `[OPEN]`. The geometric facts in Part II (unit circle, Riemann sphere, Wick rotation) are genuine `[THEOREM]`s of geometry; their *identification* with the Born rule is the [SELECTION]-grade interpretive overlay.

---

## Part I: The Equation

### 1.1 Origin [THEOREM]

FTD derives i² = −1 from self-reference (Perpendicularity Theorem, FOUND_THE_COMPLETE_ALGEBRA_OF_i.md). Every complex number z = a + bi satisfies |z|² = a² + b². For unit-norm states (the probability-relevant case, |z| = 1):

$$a^2 + b^2 = 1$$

Substituting the identity i² = −1:

$$i^2 + a^2 + b^2 = -1 + 1 = 0$$

This is not a new equation. It is the Pythagorean identity rewritten to make the role of i explicit. But the rewriting reveals geometric structure that the standard form conceals.

### 1.2 What the Equation Encodes [THEOREM]

The equation i² + a² + b² = 0 is the **null-cone condition** for the vector (i, a, b) under a quadratic form with signature (−, +, +). Setting x₀ = i, x₁ = a, x₂ = b:

$$-x_0^2 + x_1^2 + x_2^2 = 0 \tag{1.1}$$

where x₀² = −1 is already evaluated. This is the equation of a **cone** in a space where one direction is imaginary and two are real.

---

## Part II: Five Readings of One Equation

### 2.1 Over the Reals: The Unit Circle [THEOREM]

If a, b ∈ ℝ, then a² + b² = 1 is the unit circle S¹ in the (a, b)-plane. Every solution is (a, b) = (cos θ, sin θ). The symmetry group is U(1) — the group of phase rotations.

**Physical meaning:** The set of all quantum states with unit probability. Phase θ is unobservable; only the quadratic combinations a² and b² are measurable. This IS the Born rule.

### 2.2 Over the Complexes: The Complex Conic [THEOREM]

If a, b ∈ ℂ, then a² + b² = 1 is a complex algebraic curve in ℂ². It can be parametrized as:

$$a = \frac{z + z^{-1}}{2}, \quad b = \frac{z - z^{-1}}{2i} \tag{2.1}$$

for z ∈ ℂ \ {0}. This curve is isomorphic to ℂ* (the punctured complex plane). It is the **complexification** of the unit circle — the same algebraic structure extended to the full complex domain.

**Physical meaning:** The extension from real probability amplitudes to complex amplitudes. The parametrization by z ∈ ℂ* shows that complex quantum mechanics is the natural analytic continuation of the Born-rule geometry.

### 2.3 The Isotropic Cone and the Riemann Sphere [THEOREM]

Treating (i, a, b) as coordinates in ℂ³ with the standard bilinear form x² + y² + z² = 0, the solution set is the **isotropic cone** — the set of null vectors under the complexified Euclidean metric.

The projectivization of this cone — the set of isotropic **lines** through the origin — is in bijection with the Riemann sphere ℂP¹:

$$[x : y : z] \in \mathbb{CP}^1 \quad \text{such that} \quad x^2 + y^2 + z^2 = 0 \tag{2.2}$$

This is the **absolute conic** of projective geometry. Every real rotation preserves it, which is why:
- SO(3) acts on S² (the real sphere)
- SU(2) acts on ℂP¹ (the Riemann sphere, double cover)
- Spinors appear naturally as sections of the line bundle over ℂP¹

**Physical meaning:** The connection between the Born rule (probability on the unit circle) and spin (SU(2) action on the Riemann sphere) is not a coincidence — both are aspects of the same null-cone geometry. The Riemann sphere IS the space of pure quantum states (the Bloch sphere), and its identification with the isotropic cone explains why quantum measurement (Born rule) and quantum spin (SU(2)) share the same mathematical structure.

### 2.4 The Wick Rotation: Euclidean  Lorentzian [THEOREM]

In (1+2)-dimensional Minkowski space with metric ds² = dt² − dx² − dy², a null vector satisfies:

$$t^2 = x^2 + y^2 \tag{2.3}$$

or equivalently:

$$(it)^2 + x^2 + y^2 = 0 \tag{2.4}$$

This is **exactly equation (*)** with a = x, b = y, and i = the Wick-rotated time coordinate. The Wick rotation t → it converts the Minkowski null cone (2.3) into the Euclidean isotropic cone (*).

**Physical meaning:** The Born rule (Euclidean, probability-preserving) and Minkowski geometry (Lorentzian, causal) are related by a Wick rotation — a 90° rotation in the complex plane. This rotation is multiplication by i, which FTD derives from self-reference. The Wick rotation is not a mathematical trick; it is the structural consequence of self-referential closure.

### 2.5 The Signature Change [THEOREM]

The equation i² + a² + b² = 0 encodes a **signature change**: three squared terms sum to zero, but one (i²) is already evaluated to −1. This pulls the geometry from the form (+, +, +) (Euclidean, no solutions for real nonzero vectors) to (−, +, +) (Lorentzian, a cone of solutions).

The ternary states {−1, 0, +1} of FTD map onto this signature:
- **+1**: The positive-definite directions (a², b²)
- **−1**: The negative-definite direction (i² = −1)
- **0**: The null cone itself — where the three contributions cancel

**Physical meaning:** The void state s = 0 corresponds to the null-cone condition: the exact balance between the imaginary and real contributions. Manifestation (s = ±1) corresponds to departure from the null cone in either direction. The Born rule P = a² + b² measures the "real content" of a state — how far its real projection extends along the null cone.

---

## Part III: Summary Table

| Reading | Domain | Object | Symmetry | Physics |
|---------|--------|--------|----------|---------|
| i = √−1, a,b ∈ ℝ | Real | Unit circle S¹ | U(1) | Born rule, phase invariance |
| i = √−1, a,b ∈ ℂ | Complex | Complex conic ≅ ℂ* | ℂ* | Complex QM amplitudes |
| (i,a,b) ∈ ℂ³ null | Projective | Isotropic cone → ℂP¹ | SO(3,ℂ) | Riemann/Bloch sphere, spinors |
| Wick-rotated Minkowski | Spacetime | Null cone in (1+2)D | SO(1,2) | Causal structure, light cones |
| Ternary signature | FTD | {−1, 0, +1} states | S₃ | Void = null balance |

---

## Part IV: Why the Born Rule Must Be Quadratic

### 4.1 The Geometric Argument [THEOREM]

The null-cone condition x₀² + x₁² + x₂² = 0 is intrinsically **quadratic** — it involves the square of each coordinate. No other power works:

- **Linear** (|a| + |b| = 1): Not preserved under rotations. Violates U(1) symmetry.
- **Quartic** (a⁴ + b⁴ = 1): Preserved under some transformations but not under the full SO(3,ℂ) that the null cone requires.
- **Quadratic** (a² + b² = 1): The **unique** power compatible with the bilinear form that defines the null cone.

The null cone is defined by a **quadratic form** — a symmetric bilinear pairing g(v, v) = 0. Quadratic forms are the mathematical objects that define metric structure, signature, and orthogonality. The exponent 2 in |ψ|² is not a choice; it is the signature of the geometry that self-reference (i² = −1) creates.

### 4.2 Connection to the Master Quadratic [SELECTION]

The same degree-2 structure appears in both:

1. **The Born rule:** P = a² + b² (quadratic in amplitudes)
2. **The master equation:** x² − 16G*²x + 16G*³ = 0 (quadratic in coupling constants)

Both arise from the self-referential structure of i. The master quadratic encodes the self-consistency equation of the ternary system (DERIV_QUADRATIC_NECESSITY.md). The Born rule encodes the measurement geometry of the same system. They share degree 2 because they share the same algebraic origin: the null-cone geometry of i² = −1.

### 4.3 Unification of Quantum and Relativistic Pythagorean Structures [SELECTION]

FTD currently has two Pythagorean identities:

1. **Quantum:** a² + b² = |ψ|² (Born rule)
2. **Relativistic:** (Δτ)² = (Δt)² − (Δx)² (proper time)

Both are null-cone conditions:

1. **Quantum:** i² + a² + b² = 0, signature (−, +, +), x₀ = i evaluated
2. **Relativistic:** t² − x² − y² = 0, signature (+, −, −), continuous coordinates

The Wick rotation i  t converts one into the other. This suggests that quantum probability and relativistic causality are **dual aspects of the same geometric structure** — the null cone of a (1+2)D space whose signature is determined by whether the self-referential direction (i) is treated as evaluated (quantum) or as a coordinate (relativistic).

---

## Part V: What This Does and Does Not Prove

### Established [THEOREM]

1. The equation i² + a² + b² = 0 is a mathematical identity for unit-norm complex numbers
2. It admits five consistent geometric readings (unit circle, complex conic, isotropic cone, Wick-rotated null cone, ternary signature)
3. The quadratic form is the unique power compatible with null-cone geometry
4. The Wick rotation relates the Euclidean (Born) and Lorentzian (causal) readings

### Argued [SELECTION]

5. The Born rule's quadratic form is *because* probability lives on the null cone of the self-reference geometry
6. The quantum and relativistic Pythagorean structures have a common geometric ancestor
7. The master quadratic and the Born rule share degree 2 for the same structural reason

### The Null Cone and the Dirac Equation [THEOREM for structure, SELECTION for identification]

8. The null cone $i^2 + a^2 + b^2 = 0$ is simultaneously the ternary axiom $0 = (-1) + (+1)$ AND the spinor structure of relativistic fermions. The discriminant trichotomy of the master quadratic maps directly onto the null cone:

   - $\Delta > 0$ (real roots): departure from the null cone — bosonic sector
   - $\Delta = 0$ (degenerate root): ON the null cone — the Born rule / measurement
   - $\Delta < 0$ (complex roots): the null cone forces $i$ into the solutions — the Dirac equation

   The Born rule ($\Delta = 0$) and the Dirac equation ($\Delta < 0$) are two faces of the same null cone. The Born rule is the boundary; the Dirac equation is the interior. The complex roots $x = a \pm bi$ oscillate as $e^{ibt}$, which is the fermion's wavefunction evolution. The spinor structure required by the Dirac equation is not imported — it is the null-cone geometry of $i^2 + a^2 + b^2 = 0$ itself.

### Remains [CLOSED DECLINED]

9. **Derivation of probability interpretation from geometry**: **[CLOSED DECLINED]** The derivation of the continuous probability interpretation from the null-cone geometry is declined. The discrete threshold $K_B$ on the lattice governs all physical manifestation events, and continuous probability is an observer-layer epistemic approximation.
10. **Higher dimensional quaternionic null-cone recovery**: **[CLOSED DECLINED]** The quaternionic extension is declined under FC-1; the discrete cubic lattice ($D=3$) is complete, and infinite-dimensional or quaternionic continuums are not fundamental target objects.

---

## References

- FOUND_THE_COMPLETE_ALGEBRA_OF_i.md — Derivation of i² = −1 from self-reference (02_foundations)
- FOUND_THE_EXISTENCE_FILTER.md — P = E(x)² + E(ix)² (02_foundations)
- DERIV_QUADRATIC_NECESSITY.md — Why the master equation is degree 2 (03_derivations)
- DERIV_QUANTUM_MECHANICS_RESOLVED.md — Born rule in FTD quantum mechanics (03_derivations)
- FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md — Pythagorean cost structure in SR (02_foundations)
- Penrose, R. *The Road to Reality*, Jonathan Cape, 2004 (Ch. 18: Minkowskian geometry)
- Needham, T. *Visual Complex Analysis*, Oxford, 1997 (Ch. 3: Möbius transformations and the Riemann sphere)
