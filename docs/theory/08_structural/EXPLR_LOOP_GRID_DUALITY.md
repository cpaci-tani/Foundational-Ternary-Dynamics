# Loop-Grid Duality: The Two-Layer Ontology of FTD

**Version:** 1.0
**Date:** February 10, 2026
**Status:** [SELECTION]
**Epistemic Tag:** [SELECTION] -- The duality describes existing FTD structure; the deeper interpretation is proposed

> FTD has two layers: a continuous vector flux field and a discrete ternary state lattice. This is not a computational convenience. It is the structural manifestation of a fundamental duality between the continuous (Loop) and the discrete (Grid).

---

## 1. The Two Layers

FTD's ontology has always had two distinct layers:

| Layer | Mathematical Object | Properties | Role |
|-------|-------------------|------------|------|
| **Flux (Loop)** | J(v,t) in R^3 | Continuous, wavelike, interfering, dispositional | Encodes potential, propagates as waves |
| **State (Grid)** | s(v,t) in {-1, 0, +1} | Discrete, definite, non-superposable, actual | Encodes actuality, transitions at threshold |

The flux field is the *potential* -- what could happen. The state field is the *actual* -- what has happened. The manifestation threshold KB mediates the transition from potential to actual.

This two-layer structure is usually presented as an implementation detail (CLAUDE.md Section 3.1). We propose it is more fundamental: it reflects a duality between two modes of existence.

---

## 2. The Loop

The "Loop" is the continuous, self-referential aspect of reality. In FTD, it is embodied by:

### 2.1 The Flux Field

The flux J(v,t) in R^3 is continuous (takes any real value), propagates as waves via the discrete wave equation, and exhibits interference (vector addition of flux from multiple sources). It is the substrate of quantum-like behavior.

### 2.2 The Lemniscate Constant

The lemniscatic constant G* = sqrt(2) * Gamma(1/4)^2 / (2*pi) = 2.9587... arises from the lemniscate of Bernoulli -- the simplest self-crossing closed curve. The lemniscate is the "Loop" made geometric: a curve that crosses itself, creating a self-referential structure.

The master quadratic x^2 - 16*G*^2*x + 16*G*^3 = 0 is the algebraic encoding of the Loop's self-intersection.

### 2.3 The Elliptic Structure

The lemniscate is an elliptic curve. The flux field's complexification psi = J_x + i*J_y lives on this elliptic structure. The Complex Multiplication (CM) property with j-invariant j = 1728 selects the unique elliptic curve that is maximally symmetric -- the Loop with the most self-referential structure.

---

## 3. The Grid

The "Grid" is the discrete, resolution-limiting aspect of reality. In FTD, it is embodied by:

### 3.1 The Lattice

The voxel lattice L in Z^3 is discrete (integer coordinates only), has finite coordination (6, 12, or 26 neighbors depending on connectivity), and imposes a maximum propagation speed C = 1 voxel/tick. It is the substrate of definite outcomes.

### 3.2 The Gauss Constant

Gauss's constant G = 1/AGM(1, sqrt(2)) = 0.8346... is the reciprocal of the arithmetic-geometric mean. The AGM algorithm converges by alternating between arithmetic mean (additive, linear) and geometric mean (multiplicative, logarithmic) -- exactly the two operations that define the Loop and Grid:

| Operation | Type | Layer |
|-----------|------|-------|
| Arithmetic mean | Additive, linear | Grid (accumulation, counting) |
| Geometric mean | Multiplicative, scaling | Loop (interference, coupling) |

The AGM is the algorithm that reconciles these two modes. Gauss's constant is the rate at which they converge.

### 3.3 The Ternary States

The ternary state set {-1, 0, +1} is the minimal non-trivial discrete structure. It is the Grid reduced to its simplest form: negative, void, positive. No fractions, no superpositions, no ambiguity.

---

## 4. The Duality

### 4.1 Complementary Properties

| Property | Loop (Flux) | Grid (States) |
|----------|-------------|---------------|
| Values | Continuous (R^3) | Discrete ({-1,0,+1}) |
| Evolution | Wave equation (smooth) | Threshold transitions (sharp) |
| Superposition | Yes (vector addition) | No (definite state) |
| Information | Amplitude + phase | Identity + location |
| Propagation | Diffusion (spread) | Particle (localized) |
| Constant | G* = 2.9587 (lemniscatic) | G = 0.8346 (Gauss) |

### 4.2 The Relationship Between G* and G

The lemniscatic constant and Gauss's constant are related through the lemniscate constant varpi:

```
G* = 2 * varpi * sqrt(2) / pi = 2.9587...
G  = 2 * varpi / pi           = 0.8346...
```

Therefore:

```
G* / G = sqrt(2) = 1.4142...
```

The ratio between the Loop constant and the Grid constant is **exactly sqrt(2)** -- the diagonal of the unit square, the critical coupling strength that appears in the Gauss constraint geometry.

This is not a coincidence. The sqrt(2) factor is the same one that appears in:
- The FCC nearest-neighbor distance (sqrt(2) lattice units)
- The critical coupling from the Gauss constraint (DERIV_ALPHA_PRECISION_FORMULA.md)
- The Boltzmann factor for diagonal vs face neighbors

### 4.3 The Inversion Symmetry

At the self-dual point, Loop and Grid descriptions coincide. This occurs at:

```
G* * G = 2.9587 * 0.8346 = 2.4697...
sqrt(G* * G) = 1.5716...
```

Compare with phi = 1.618... (2.9% difference). The geometric mean of the Loop and Grid constants is *near* but not equal to the golden ratio. Whether this can be made exact with a correction term is an open question (see EXPLR_GOLDEN_RATIO_SCALE_BRIDGE.md).

---

## 5. Loop-Grid Transition: Manifestation

The manifestation process (CLAUDE.md Section 4.1) is the transition from Loop to Grid:

```
Loop regime:    |J| < KB     (flux below threshold, wave-like, dispositional)
Transition:     |J| = KB     (threshold crossing, probabilistic)
Grid regime:    s = +/- 1    (manifested, particle-like, actual)
```

### 5.1 Why the Threshold Exists

In pure Loop dynamics (no Grid), the flux field would evolve forever as waves -- no particles, no definite outcomes, no measurement. The threshold KB imposes a minimum density for "actualization."

In pure Grid dynamics (no Loop), states would be static or random -- no waves, no interference, no quantum behavior. The flux field provides the dynamics that connect distant states.

The threshold is the **interface between modes**. It is where the continuous collapses into the discrete, potential into actual, wave into particle.

### 5.2 The Born Rule as Loop-Grid Translation

The Born rule P(v) = |psi(v)|^2 / ||psi||^2 converts Loop information (complex amplitude psi) into Grid information (probability of manifestation at site v). It is the translation dictionary between the two layers:

| Loop (Flux) | Translation | Grid (State) |
|-------------|------------|--------------|
| psi(v) = J_x + i*J_y | Born rule: P = |psi|^2 | s(v) = +/- 1 with probability P |
| Amplitude | Squared | Probability |
| Phase | Lost | Definite outcome |

The phase information (the "which direction" of the flux vector) is lost in the transition. This is the source of quantum randomness in FTD: not ontological indeterminacy, but information loss at the Loop-Grid interface.

---

## 6. Implications for the Simulation

### 6.1 The Two-Layer Architecture Is Physical

FTD's architecture -- continuous flux arrays alongside discrete state arrays -- is usually presented as a design choice. Loop-Grid duality reinterprets it: the two layers are not two representations of the same thing. They are two genuinely different ontological modes that coexist and interact.

A simulation that tried to use *only* flux (continuous, no states) would produce a linear wave equation with no particles. A simulation that tried to use *only* states (discrete, no flux) would produce a cellular automaton with no wave-like behavior. The physics requires *both*.

### 6.2 The Lattice Geometry Connection

The choice of lattice geometry (cubic vs cuboctahedral) affects the Grid layer but not the Loop layer:

| Component | Lattice-dependent? | Why? |
|-----------|-------------------|------|
| Flux field values | No | R^3 vectors on any lattice |
| Discrete operators | **Yes** | Laplacian, gradient depend on neighbors |
| Manifestation threshold | No | KB is a flux density, not a lattice property |
| State transitions | **Indirectly** | Neighbors for annihilation depend on lattice |
| G* and alpha | No | Derived from Oh symmetry (shared by cubic and FCC) |

The Loop layer is geometry-independent; the Grid layer is geometry-dependent. This is consistent with the duality: the continuous (Loop) transcends the discrete (Grid), while the discrete constrains the continuous.

---

## 7. The AGM as Reconciliation Algorithm

The arithmetic-geometric mean iteration can be interpreted as a numerical algorithm for reconciling Loop and Grid:

```
Iteration 0: a_0 = 1 (Grid: unit lattice spacing)
              g_0 = sqrt(2) (Loop: diagonal/FCC neighbor distance)

Iteration 1: a_1 = (1 + sqrt(2))/2 = 1.2071 (arithmetic average)
              g_1 = 2^{1/4} = 1.1892 (geometric average)

Iteration 2: a_2 = 1.1981, g_2 = 1.1981 (converged to 4 digits)

Limit: M = AGM(1, sqrt(2)) = 1/G = 1.1981...
```

The convergence is remarkably fast (quadratic in the number of digits). By iteration 4, the agreement exceeds 50 decimal places.

**Interpretation:** The AGM represents the universe "computing" the reconciliation between its continuous (Loop) and discrete (Grid) modes. Gauss's constant G = 1/M is the residual after reconciliation -- the irreducible mismatch between continuous potential and discrete actuality.

---

## 8. Summary

| Concept | Loop | Grid | Interface |
|---------|------|------|-----------|
| Mathematical object | R^3 flux field | {-1,0,+1} state lattice | Manifestation threshold KB |
| Fundamental constant | G* = 2.9587 | G = 0.8346 | G*/G = sqrt(2) |
| Behavior | Waves, interference | Particles, transitions | Born rule |
| Information | Amplitude + phase | Identity + location | Phase lost at transition |
| Scale | Continuous | Discrete | AGM reconciliation |

FTD's two-layer ontology is not an accident of simulation design. It is the minimal structure that supports both wave-like potential and particle-like actuality. The Loop provides the geometry (lemniscate, elliptic curves, G*); the Grid provides the arithmetic (integers, states, Gauss constant). Physics is what happens at their interface.

---

## References

- CLAUDE.md, Sections 3.1 (Flux Field), 4.1 (Genesis), 13.1 (Measurement = Manifestation)
- DERIV_ALPHA_PRECISION_FORMULA.md (G* derivation, sqrt(2) factor)
- EXPLR_CUBOCTAHEDRAL_GEOMETRY.md (Oh symmetry invariance)
- EXPLR_GOLDEN_RATIO_SCALE_BRIDGE.md (phi near sqrt(G* * G))
- EXPLR_VACUUM_DRAG_DERIVATION.md (Gauss constant as resolution ratio)
- Borwein, J.M. and Borwein, P.B. (1987). *Pi and the AGM.* Wiley. (AGM theory)
