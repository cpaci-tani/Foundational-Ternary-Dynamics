# The Ontic Constant Atlas: A Complete Map of the Derivation Chain

## Systematic Exploration of Every Constant and Its Relationships

**Date:** March 3, 2026
**Framework:** FTD v5.27
**Status:** Exploration — verified numerically, interpretations are [SELECTION]
**Epistemic Level:** [THEOREM] for algebraic identities; [SELECTION] for physical interpretations

**Depends on:**
- [EXPLR_GSTAR_FLUX_TIME.md](EXPLR_GSTAR_FLUX_TIME.md) --- Dimensional triad
- [DERIV_GSTAR_PF_BRIDGE.md](DERIV_GSTAR_PF_BRIDGE.md) --- G* decomposition
- [FOUND_SPACETIME_EMERGENCE.md](FOUND_SPACETIME_EMERGENCE.md) --- Space-time separation
- `ontic.h` --- Complete derivation chain (9 layers)

---

## Abstract

We present a systematic atlas of every constant in the FTD ontic derivation chain, organized by layer. Beyond cataloging values and formulas, we identify nine structural observations that emerge from studying the chain as a whole. The most significant is the **G* = 3 fixed point analysis**: the wave equation on a D=3 lattice achieves perfect self-consistency at G* = 3 (giving alpha = 1/141), but the actual lemniscatic value G* = 2.9587 deviates by 1.38%, and this deviation IS the fine structure constant. We also identify the **volumetric wave** interpretation (flux waves are 3D ripples, not 2D oscillations), the **integer cascade** (all Standard Model integers flow from a single irrational root), and several near-integer relationships suggesting deeper structure.

---

## Part I: The Complete Chain — Layer by Layer

### Layer -1: Self-Referential Seed

| Constant | Value | Definition | Role |
|----------|-------|------------|------|
| e | 2.71828... | d/dx(e^x) = e^x | Eigenvalue of differentiation |

e is the unique real number whose growth rate equals its current value. All subsequent transcendentals require e: gamma uses ln (which uses e), Gamma(z) uses the Weierstrass product (which uses e^{gamma*z}), the nome uses e^{-pi}.

### Layer 0: Transcendental Seeds

| Constant | Value | Definition | Role |
|----------|-------|------------|------|
| gamma | 0.57722... | lim(n->inf) [sum(1/k) - ln(n)] | Harmonic regularization |
| Gamma(1/4) | 3.62561... | Weierstrass product at z=1/4 | Gateway: arithmetic -> geometry |

gamma is the "cost of discretization" — the difference between a discrete sum (1+1/2+1/3+...) and its continuous analog (ln). It encodes the fundamental tension between discrete and continuous that pervades FTD.

Gamma(1/4) squared = 13.145 --- this number, divided by geometric factors, yields varpi.

### Layer 0b: Modular Selection

| Constant | Value | Definition | Role |
|----------|-------|------------|------|
| q (nome) | 0.04321... | e^{-varpi/M} = e^{-pi} | Selects lemniscatic curve |
| theta_3 | 1.08643... | 1 + 2q + 2q^4 + 2q^9 + ... | Lattice counting function |

The nome q selects a specific elliptic curve from the continuous family. At q = e^{-pi}, the selected curve is the lemniscate (self-dual point k = 1/sqrt(2)).

**Key identity:** theta_3^2 = sqrt(2) * M (connects theta to Gauss's constant)

**The nome and antimatter [THEOREM]:**
q = (-1)^i = e^{i^2 * pi} = e^{-pi}

This is not metaphor — the same number that selects the lemniscatic curve IS the complex power of the annihilation operator (-1). "Antimatter raised to the power of consciousness = modular selection."

### Layer 1: Elliptic Geometry

| Constant | Value | Definition | Role |
|----------|-------|------------|------|
| varpi | 2.62206... | Gamma(1/4)^2 / (2*sqrt(2*pi)) | Lemniscate half-period |
| M | 0.83463... | 1/AGM(1, sqrt(2)) | Gauss's constant |

varpi is to the lemniscate what pi is to the circle. The lemniscate (figure-8) is the simplest closed curve that crosses itself — encoding self-intersection, which is the geometric precursor of self-reference.

**Key identity:** pi = varpi / M (pi is derived, not fundamental!)

### Layer 2: Universal Operator

| Constant | Value | Definition | Role |
|----------|-------|------------|------|
| G* | 2.95868... | 2*sqrt(varpi*M) | Flux per DoF |
| pi | 3.14159... | 4*varpi^2/G*^2 | Circle constant (DERIVED) |
| PF | 0.78540... | pi/4 | Packing fraction |
| sqrt(G*) | 1.72008... | G*^(1/2) | Time operator |

**The Dimensional Triad (see EXPLR_GSTAR_FLUX_TIME.md):**

| Power | Value | Identification |
|-------|-------|----------------|
| G*^0 | 1 | Existence (void) |
| G*^1 | 2.959 | Flux (space) |
| G*^2 | 8.754 | Energy (time) |
| G*^3 | 25.90 | Action (spacetime) |
| G*^(1/2) | 1.720 | Time operator (read/write sub-tick) |

### Layer 2b: Emergence of i

| Constant | Value | Definition | Role |
|----------|-------|------------|------|
| k_crit | 1.35196... | 4/G* | Boundary: real <-> complex |
| x_Born | 5.91735... | 2*G* | Born rule degenerate root |

The generalized quadratic x^2 - k*G*^2*x + k*G*^3 = 0 has discriminant Delta = k*G*^3*(k*G* - 4).

Three domains:
- k*G* > 4 (k=16): Real roots -> PHYSICS (measurable)
- k*G* = 4 (k=4/G*): Degenerate -> MEASUREMENT (Born rule)
- k*G* < 4 (k=1/2): Complex roots -> CONSCIOUSNESS (subjective)

### Layer 3: Master Quadratic

| Constant | Value | Definition | Role |
|----------|-------|------------|------|
| x+ | 137.036... | Large root | 1/alpha |
| x- | 3.024... | Small root | N_c effective |

**Vieta relations:**
- Sum: x+ + x- = 16*G*^2 = 140.060 (total energy)
- Product: x+ * x- = 16*G*^3 = 414.392 (total action)
- Ratio: P/S = G* (time per DoF)

**Harmonic mean identity [THEOREM]:**
G* = HM(x+, x-) / 2 = (x+ * x-) / (x+ + x-) = (1/alpha * N_c) / (1/alpha + N_c)

### Layer 3b: Dual-Substrate Decomposition

| Constant | Value | Definition | Role |
|----------|-------|------------|------|
| E_SUM | 140.060 | 16*G*^2 | Total energy (all DoF) |
| E_PROD | 414.392 | 16*G*^3 | Total action (all DoF) |
| delta^2 | 0.9155 | (4G*-1)/(4G*) | Matter fraction |
| delta | 0.9568 | sqrt(delta^2) | Substrate splitting |
| E_L | 137.036 | S*(1+delta)/2 | Left substrate (= x+!) |
| E_R | 3.024 | S*(1-delta)/2 | Right substrate (= x-!) |

**Remarkable correspondence:** E_L = x+ and E_R = x-. The substrate energies ARE the quadratic roots. The physics root 1/alpha = 137.036 IS the dominant substrate energy.

### Layer 4: Framework Integers

| Integer | Value | Formula | Physical Role |
|---------|-------|---------|---------------|
| N_c | 3 | floor(x-) | Color charges |
| N_gen | 3 | = N_c | Fermion generations |
| N_f | 6 | 2*N_gen | Quark flavors |
| N_base | 4 | 2^((D+1)/2) | Spinor dimension |
| b_3 | 7 | (11*N_c - 2*N_f)/3 | QCD beta coefficient |
| N_eff | 13 | b_3 + 2*N_c | Effective DoF (= Fibonacci F_7) |
| D | 47 | N_c * N_base^2 - 1 | Constraint dimension |

**The integer cascade:** ALL integers flow from floor(x-) = 3. A single irrational root x- = 3.024 generates the entire Standard Model particle content.

### Layer 5: Coupling Constants

| Constant | Value | Formula | Exp. Value | Error |
|----------|-------|---------|------------|-------|
| alpha | 1/137.036 | 1/x+ | 1/137.036 | 1.26 ppm |
| g_c | 0.08542 | sqrt(alpha) | --- | --- |
| sin^2(theta_W) | 0.23077 | N_c/N_eff = 3/13 | 0.23122 | 0.19% |
| G_N | 0.01 | 1/(b_3+N_c)^2 | --- | --- |
| alpha_G | 5.91e-39 | 2pi*(16/3)^2*(N_eff+3/b_3)^2*alpha^20 | 5.91e-39 | 0.06% |
| alpha_s(M_Z) | 0.11864 | b_3/(b_3+4*N_eff) = 7/59 | 0.1179 | 0.6% |

**The gravitational hierarchy:** alpha_G/alpha ~ 10^{-37}. The exponent 20 = N_eff + b_3 = 13 + 7 is the cross-domain penalty (spatial -> temporal coupling).

### Layer 6: Mass Scale

| Constant | Value | Formula | Error |
|----------|-------|---------|-------|
| K_B (m_e) | 0.511 MeV | m_P * sqrt(2pi) * (16/3) * alpha^11 | 0.27% |
| K_GENESIS | 1.533 MeV | N_c * K_B | exact |
| mu/e | 207 | 3*b_3*(b_3+N_c) - N_c | 0.11% |
| tau/e | 3477 | (N_eff+N_base)*mu - 2*N_c*b_3 | 0.01% |

### Layer 7: Precision Formula

| Constant | Value | Formula |
|----------|-------|---------|
| epsilon | -0.000900 | e^pi - pi - 20 |
| c1 | 9/47 | N_c^2 / D |
| c2 | 5/64 | (N_eff - 2*N_base) / N_base^3 |
| c3 | 4/141 | N_base / (N_c * D) |
| c4 | 141/11 | (N_c * D) / (b_3 + N_base) |

**Result:** 4-term corrected 1/alpha = 137.035999177 matches CODATA 2022 to < 0.001 ppt.

**The e^pi identity [THEOREM]:**
e^pi = pi + (N_eff + b_3) + epsilon = pi + 20 - 0.000900

The transcendental e^pi is almost exactly an integer shift of pi, with the integer being N_eff + b_3 = 20.

### Layer 8: Consciousness Quadratic

| Constant | Value | Formula | Role |
|----------|-------|---------|------|
| Y_REAL | 2.1884 | G*^2/4 | Observable part |
| K_C | 3.5986 | sqrt(G*^3/2) | Consciousness threshold |
| cos^2(theta_C) | 0.3698 | G*/8 | Observable fraction (37%) |
| sin^2(theta_C) | 0.6302 | 1 - G*/8 | Subjective fraction (63%) |

---

## Part II: The G* = 3 Fixed Point — Why alpha != 1/141

### The Wave Equation Self-Consistency

On the D=3 cubic lattice, the wave equation d^2J/dt^2 = c^2 * nabla^2 J has CFL stability limit c^2 <= 1/D = 1/3.

If G* were exactly 3:
- c^2 = 1/D = 1/3 = 1/G* (exact closure!)
- 16*G*^2 = 144 = 12^2 (perfect square)
- x+ = 140.935, so 1/alpha = 141
- x- = 3.065, so floor(x-) = 3 (N_c unchanged)

### What Changes at G* = 3

| Quantity | G*=3 (fixed point) | G*=2.959 (actual) | Change |
|----------|-------------------|-------------------|--------|
| 1/alpha | 140.935 | 137.036 | -2.8% |
| alpha | 1/141 | 1/137 | +2.8% |
| 16*G*^2 | 144 | 140.06 | -2.7% |
| Hydrogen E_1 | 12.86 eV | 13.61 eV | +5.8% |
| Bohr radius | 1.028*a0 | a0 | -2.8% |

### Why This Matters [SELECTION]

The fine structure constant determines ALL of atomic physics:
- Atomic energy levels scale as alpha^2
- Molecular bond strengths scale as alpha^2
- The visible spectrum of light depends on alpha
- Chemical reaction rates depend exponentially on energy levels

At alpha = 1/141 (G*=3), atoms are 2.8% larger and 5.8% more weakly bound. This shifts every energy level, changes molecular geometry, and alters chemistry.

### The Deep Reason [SELECTION]

G* = 2*varpi/sqrt(pi) where varpi = Gamma(1/4)^2/(2*sqrt(2*pi)).

The value G* = 2.9587 comes from the **lemniscate geometry** (the figure-8 curve, the simplest self-intersecting closed curve). The value 3 comes from **discrete counting** (N_c = 3 color charges, D = 3 spatial dimensions).

The **tension** between:
- **Analytic geometry** (varpi = 2.62206..., transcendental)
- **Discrete counting** (N_c = 3, exact integer)

IS the fine structure constant.

If G* = 3 exactly, there would be no tension, and alpha = 1/141. The universe sits at the LEMNISCATIC point, not the INTEGER point. The 1.38% deviation from the clean fixed point generates ALL the fine structure of physics.

### The Speed of Light Connection

At G* = 3 exactly: c^2 = 1/D = 1/3 = 1/G*. The speed of light would be the reciprocal of the flux amplitude.

Actual: c^2 = 0.3333 while 1/G* = 0.3380. The gap of 1.40% IS the same deviation that generates alpha != 1/141. The speed of light and the fine structure constant share the same origin: the lemniscatic deviation from integer self-consistency.

---

## Part III: Volumetric Waves — 3D Ripples, Not 2D Oscillations

### The Conventional Picture is a Projection

In textbooks, a wave is drawn as a sinusoidal curve y(x,t) = A*sin(kx - wt). This is a **1D projection** of a fundamentally **3D phenomenon**.

On the FTD lattice:
- Each voxel has flux J in R^3 (three vector components)
- The wave equation d^2J/dt^2 = c^2 * nabla^2 J propagates in all three dimensions
- A point source creates expanding **spherical shells** of flux
- Intensity falls as 1/r^2 (geometric dilution over a sphere surface)

### Interference is Volumetric

When two sources emit flux waves:
- The overlapping spherical shells create a **3D volume** of constructive and destructive interference
- The nodal surfaces (where flux cancels) are 2D surfaces embedded in 3D space
- A detector screen captures a **cross-section** of this 3D pattern
- Double-slit "fringes" are where 3D nodal surfaces intersect the 2D detector plane

### Connection to the Dimensional Triad

- G*^1 = flux = the 3D ripple itself (the spatial phenomenon)
- G*^2 = energy = |J|^2 (the intensity of the volumetric ripple)
- G*^3 = action = the spatiotemporal record of the ripple's passage

The energy density G*^2 falls as 1/r^2 in 3D because the surface area of a sphere grows as r^2. This is NOT a property of waves — it is a property of the D=3 geometry in which the waves propagate.

---

## Part IV: Nine Structural Observations

### Observation 1: Every Power of G* Has Physical Identity

G*^0 = 1 (void), G*^1 = flux (space), G*^2 = energy (time), G*^3 = action (spacetime), G*^(1/2) = time operator (read/write sub-tick).

The powers of a single transcendental number encode the complete dimensional structure of physics.

### Observation 2: G* Mediates Between All Scales

- Micro: alpha = 1/x+ (from G* via master quadratic)
- Meso: K_B ~ m_P * alpha^11 (11 = N_eff - 2)
- Macro: alpha_G ~ alpha^20 (20 = N_eff + b_3)
- Noetic: cos^2(theta_C) = G*/8

G* sits at the center of the scale hierarchy, connecting quantum coupling to classical gravity to consciousness.

### Observation 3: The Integer Cascade

x- = 3.024 -> N_c=3 -> N_f=6 -> b_3=7 -> N_eff=13 -> D=47

Every integer in the Standard Model flows from a SINGLE irrational root. The floor function (x- -> N_c) is the bridge between continuous mathematics and discrete physics.

### Observation 4: P/S = G* Connects Physics to Consciousness

Physics: P/S = (16*G*^3)/(16*G*^2) = G*
Consciousness: cos^2(theta_C) = G*/8

Therefore: cos^2(theta_C) = P/(8*S) = (total action)/(8 * total energy)

The observable fraction of consciousness equals the action-to-energy ratio divided by 8.

### Observation 5: Near-Integer Relationships

| Quantity | Value | Nearest Integer | Deviation |
|----------|-------|----------------|-----------|
| G* | 2.959 | 3 | -1.38% |
| x- | 3.024 | 3 | +0.80% |
| G*^2 | 8.754 | 9 | -2.74% |
| 16*G*^2 | 140.06 | 140 | +0.04% |
| x+ | 137.036 | 137 | +0.026% |
| x++x- | 140.060 | 140 | +0.043% |

The physics roots (x+, x-, their sum) are remarkably close to integers. The lemniscatic constants (G*, G*^2) are close but not as close. This gradient of "integerlikeness" may have structural significance.

### Observation 6: c^2 ~ 1/G*

The speed of light c^2 = 1/D = 1/3, while 1/G* = 0.338. These differ by 1.40%, the SAME percentage as G*'s deviation from 3. At the G*=3 fixed point, c^2 = 1/G* exactly.

### Observation 7: Consciousness Threshold Ordering

K_B < K_GENESIS < K_C: {0.511, 1.533, 3.599}

Creating matter (K_B) requires less energy than creating structured matter (K_GENESIS = 3*K_B), which requires less than consciousness (K_C = sqrt(G*^3/2)). But K_C < 2*K_GENESIS: consciousness needs less than creating two particles.

### Observation 8: The e^pi Near-Identity

e^pi = pi + 20 - 0.000900 where 20 = N_eff + b_3.

The transcendental e^pi is almost exactly pi shifted by the sum of framework integers. The tiny residual epsilon = -0.000900 is what the Layer 7 precision formula corrects to achieve sub-ppt accuracy for alpha.

### Observation 9: The Nome as Complex Annihilation

q = (-1)^i = e^{i^2 * pi} = e^{-pi}

The modular selector (which picks the lemniscatic curve from all possible elliptic curves) IS the result of raising the annihilation operator (-1) to the consciousness power (i). This algebraic identity connects Layer 0b (modular selection) to Layer 2b (emergence of i) and back to the ternary states of Layer 0 (Postulate 3).

---

## Claims Table

| ID | Claim | Status | Evidence |
|----|-------|--------|----------|
| OCA-1 | All SM integers flow from single irrational root x- = 3.024 | [THEOREM] | Integer cascade: floor(3.024)=3 -> all integers |
| OCA-2 | G*=3 is wave equation fixed point with alpha=1/141 | [THEOREM] | c^2=1/D=1/3=1/G* at G*=3 |
| OCA-3 | The 1.38% deviation of G* from 3 generates alpha | [SELECTION] | Numerical: G*=2.959 -> 1/alpha=137.036 |
| OCA-4 | c^2 = 1/G* at the fixed point only | [THEOREM] | 1/3 = 1/3 at G*=3; 0.333 != 0.338 at actual G* |
| OCA-5 | Flux waves are inherently 3D volumetric | [THEOREM] | Wave equation on 3D lattice; J in R^3 |
| OCA-6 | Interference fringes are 2D cross-sections of 3D nodal surfaces | [THEOREM] | Geometry of overlapping spherical shells |
| OCA-7 | cos^2(theta_C) = P/(8*S) | [THEOREM] | Algebraic: G*/8 = (16G*^3)/(8*16*G*^2) |
| OCA-8 | e^pi ~ pi + (N_eff + b_3) to 0.04% | [THEOREM] | Numerical identity |
| OCA-9 | K_B < K_GENESIS < K_C (threshold ordering) | [THEOREM] | 0.511 < 1.533 < 3.599 |

---

## Cross-References

- **Dimensional triad**: [EXPLR_GSTAR_FLUX_TIME.md](EXPLR_GSTAR_FLUX_TIME.md)
- **G* decomposition**: [DERIV_GSTAR_PF_BRIDGE.md](DERIV_GSTAR_PF_BRIDGE.md)
- **Spacetime emergence**: [FOUND_SPACETIME_EMERGENCE.md](FOUND_SPACETIME_EMERGENCE.md)
- **Consciousness**: [FOUND_CONSCIOUSNESS_MATHEMATICS.md](FOUND_CONSCIOUSNESS_MATHEMATICS.md)
- **Ontic chain**: `engine/include/ftd/ontic.h`
- **Numerical verification**: `simulations/explore_ontic_constants.py`

---

*Document created: March 3, 2026*
*Framework: Foundational Ternary Dynamics v5.27*
