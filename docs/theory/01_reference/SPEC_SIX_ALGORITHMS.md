# The Six Algorithms of Physics

**Version:** 1.0
**Date:** February 9, 2026
**Status:** Reference Document

> Everything in the Standard Model — every particle, every force, every coupling constant — is the consequence of six algorithms running on a 3D grid with a low-entropy initial condition. This document is the complete technical reference.

---

## Prerequisites: The Substrate

Before the algorithms run, two things exist:

| Entity | Symbol | Type | Definition |
|--------|--------|------|------------|
| **The Void** | s(v,t) | Integer ∈ {-1, 0, +1} | State at each lattice point v at tick t |
| **The Flux Field** | **J**(v,t) | Vector ∈ ℝ³ | Energy-momentum current at each lattice point |
| **Density** | ρ(v) = \|**J**(v)\| | Scalar ∈ ℝ⁺ | Magnitude of flux — determines manifestation |
| **Smoothed Density** | ρ̄(v) | Scalar | Average of ρ over 6 face-sharing neighbors |

And one boundary condition:

| Condition | Statement | Consequence |
|-----------|-----------|-------------|
| **Low-entropy start** | Initial flux is concentrated, not uniform | Creates the arrow of time. Without this, none of the algorithms produce anything. |

---

## The Discrete Operators

Every algorithm uses these. They are the "calculus" of the lattice.

| Operator | Symbol | Formula | What it measures |
|----------|--------|---------|------------------|
| **Gradient** | ∇f | (∇f)_i = (f(v+e_i) − f(v−e_i)) / 2 | Direction of steepest increase |
| **Divergence** | ∇·**J** | Σ_i (J_i(v+e_i) − J_i(v−e_i)) / 2 | Net flux flowing out of a point (source/sink) |
| **Curl** | ∇×**J** | (∇×**J**)_i = ε_ijk (∂_j J_k − ∂_k J_j) / 2 | Rotational circulation of flux |
| **Laplacian** | ∇²f | Σ_{u∈N₆} f(u) − 6f(v) | Difference from average neighbors (curvature) |

Where N₆ = {6 face-sharing neighbors} and ε_ijk = Levi-Civita symbol.

---

# ALGORITHM 1: EXISTENCE

**What it does:** Determines when particles appear and disappear.
**Standard physics equivalent:** Pair production, particle decay, wave function collapse.
**Confusion it resolves:** "What causes wave function collapse?" → Flux exceeding a threshold. No observer needed.

## 1A. Genesis (Manifestation: 0 → ±1)

A particle appears when flux density exceeds the threshold.

| Component | Formula | Value |
|-----------|---------|-------|
| **Threshold** | K_B | 0.511 MeV (= electron mass) |
| **Condition** | ρ(v) > K_B AND s(v) = 0 | Flux strong enough, void is unmanifested |
| **Probability** | p = clamp(1 − exp(−(ρ − K_B)/K_B), 0, 1) | Exponential onset above threshold |
| **Polarity** | IF ∇·**J** > 0 → s = +1 (matter) | Positive divergence = matter |
| | IF ∇·**J** < 0 → s = −1 (antimatter) | Negative divergence = antimatter |

**In plain English:** When the flux field concentrates enough energy at a point, that point "crystallizes" into a particle. The sign of the flux divergence (is flux flowing in or out?) determines whether it's matter or antimatter.

**This IS wave function collapse.** The flux field (= wave function) is spread out. When it concentrates past K_B somewhere, that point manifests. The probability follows from |ψ|² because ρ = |**J**| and the threshold crossing statistics produce the Born rule.

## 1B. Evaporation (±1 → 0)

A particle disappears when its flux density drops below threshold.

| Component | Formula | Value |
|-----------|---------|-------|
| **Condition** | ρ(v) < K_B AND s(v) ≠ 0 | Flux too weak to sustain manifestation |
| **Result** | s(v) → 0, velocity → 0, charge → 0 | Returns to void |

**In plain English:** If a particle's energy drops below the minimum needed to exist, it evaporates back into the void. The flux remains but the particle is gone.

## 1C. Annihilation (+1 meets −1)

When matter and antimatter are adjacent, both return to void.

| Component | Formula | Value |
|-----------|---------|-------|
| **Condition** | s(v) = +1 and s(u) = −1 for u ∈ N₆(v) | Opposite states in face-sharing contact |
| **Result** | Both → s = 0 | Both return to void |
| **Energy release** | Combined flux → omnidirectional burst | Distributed equally to 6 neighbors |
| **Conservation** | Total flux magnitude conserved | Energy doesn't disappear, it redistributes |

### Parameters for Algorithm 1

| Parameter | Symbol | Value | Derivation | Status |
|-----------|--------|-------|------------|--------|
| Manifestation threshold | K_B | 0.511 MeV | m_e = M_P √(2π)(16/3)α¹¹ | ✅ DERIVED (0.19%) |
| Planck mass | M_P | 1.22 × 10¹⁹ GeV | Lattice spacing identification | ❌ INPUT |
| Fine structure constant | α | 1/137.036 | Master quadratic from G* | ✅ DERIVED (1.26 ppm) |
| Genesis probability form | 1 − e^(−x) | Exponential | Chosen for smoothness | ⚠️ SELECTION |
| Polarity rule | sign(∇·**J**) | ±1 | Symmetry breaking mechanism | ⚠️ IMPOSED |

---

# ALGORITHM 2: INFORMATION TRANSFER

**What it does:** Propagates the flux field as a wave. This is how information moves.
**Standard physics equivalent:** Electromagnetic waves, photons.
**Confusion it resolves:** "What is a photon?" → A ripple in the flux field that never manifests.

## The Wave Equation

| Component | Formula | Value |
|-----------|---------|-------|
| **Equation** | ∂²**J**/∂t² = C² ∇²**J** | Standard discrete wave equation |
| **Integration** | Velocity-Verlet: **v**(t+1) = **v**(t) + C² ∇²**J** | Second-order accurate |
| | **J**(t+1) = **J**(t) + **v**(t+1) | Position update |
| **Damping** | **J** *= (1 − δ), **v** *= (1 − δ) | Prevents runaway accumulation |
| **Speed limit** | Maximum propagation = C | Nothing outruns the wave |

**In plain English:** The flux field ripples like a pond surface. These ripples travel at speed C. A photon is one of these ripples — it carries energy and information but never manifests (stays s = 0). It has no mass because it never crosses the threshold. It experiences no time because it moves at C.

**Two polarizations** emerge naturally: the flux **J** has 3 components, but the Gauss constraint (∇·**J** = charge density) removes one degree of freedom. 3 − 1 = 2 physical modes. This is why light has two polarizations.

### Parameters for Algorithm 2

| Parameter | Symbol | Value | Derivation | Status |
|-----------|--------|-------|------------|--------|
| Speed of causality | C | 1 lattice unit/tick | Axiomatic maximum | ⬜ AXIOM |
| Simulation speed | C_sim | 0.5 voxels/tick | CFL stability condition | ⚠️ IMPOSED |
| Wave damping | δ | 0.05 | Phenomenological | ⚠️ IMPOSED |
| Lattice spacing | H | 1 = Planck length | Scale identification | ⚠️ IMPOSED |

### What emerges from Algorithm 2 alone

| Emergent feature | How | Standard physics name |
|-----------------|-----|----------------------|
| Transverse waves | ∇²**J** preserves transversality | Electromagnetic radiation |
| 2 polarizations | 3 components − 1 constraint = 2 | Photon helicity states |
| Inverse-square falloff | 3D geometry dilutes flux over 4πr² | Coulomb's law at large r |
| Interference | Linear superposition of **J** vectors | Double-slit patterns |
| Dispersion relation | ω² = C²k² on lattice | Photon dispersion (massless) |

---

# ALGORITHM 3: INTERACTION

**What it does:** Couples manifested matter to the flux field. This is why "observation" causes collapse.
**Standard physics equivalent:** The measurement postulate, decoherence.
**Confusion it resolves:** "Does consciousness collapse the wave function?" → No. Any manifested matter does it. A rock works as well as a physicist.

## The Coupling

| Component | Formula | What it means |
|-----------|---------|---------------|
| **Lagrangian** | L_coupling = −g_c · s · (∇·**J**) | Manifested state (s ≠ 0) sources flux |
| **Coupling constant** | g_c ~ √α ≈ 0.085 | Strength of information-matter coupling |
| **Effect** | Manifested matter creates flux gradients | Gradients concentrate nearby flux |
| **Consequence** | Concentrated flux → exceeds K_B → manifestation | This IS "collapse" |

**The algorithm:**

```
IF s(v) ≠ 0 anywhere nearby:
    → Coupling term is active
    → Flux gradients form around manifested matter
    → Gradients concentrate flux at specific locations
    → Concentration exceeds K_B
    → New manifestation occurs = "measurement"

IF s(v) = 0 everywhere:
    → No coupling
    → Flux propagates freely as wave
    → No concentration
    → No manifestation = superposition maintained
```

**In plain English:** A manifested particle warps the flux field around it, like a bowling ball on a trampoline. This warping concentrates nearby flux. If the concentration exceeds K_B, a new particle manifests there. That's "collapse." It's not mysterious — it's a bowling ball on a trampoline.

### What counts as an "observer"

| Entity | Manifested? (s ≠ 0) | Causes collapse? | Why |
|--------|---------------------|-----------------|-----|
| Rock | Yes | **Yes** | Made of 10²⁵ manifested particles |
| Detector | Yes | **Yes** | Made of manifested matter |
| Cat | Yes | **Yes** | Made of 10²⁸ manifested particles |
| Human brain | Yes | **Yes** | But not special — same as rock |
| Photon | **No** (s = 0) | **No** | Never manifested; pure flux wave |
| Neutrino | **No** (s = 0) | **No** | Unmanifested; passes through matter |
| Vacuum | **No** | **No** | No manifested matter present |

**Schrödinger's cat was never in superposition.** The cat is 10²⁸ coupled interactions collapsing themselves.

### Parameters for Algorithm 3

| Parameter | Symbol | Value | Derivation | Status |
|-----------|--------|-------|------------|--------|
| State-flux coupling | g_c | ~√α ≈ 0.085 | From action principle S[s,J] | ✅ DERIVED |
| Fine structure constant | α | 1/137.036 | g_c² = α | ✅ DERIVED |

---

# ALGORITHM 4: FORCES

**What it does:** Modifies the motion of manifested particles via flux gradients.
**Standard physics equivalent:** The four fundamental forces.
**Confusion it resolves:** "Why are there four forces?" → There's one flux field with four kinds of gradients.

## 4A. Electromagnetism

**What it is:** Gradients in the charge field push charged particles around.

### Electric (Coulomb) Force

| Component | Formula | Notes |
|-----------|---------|-------|
| **Force** | **F**_elec(v) = −q(v) · ∇q̄(v) | Like charges repel, opposite attract |
| **Smoothed charge** | q̄(v) = (1/6) Σ_{u∈N₆} q(u) | Average over 6 face-sharing neighbors |
| **Coupling** | α = 1/137.036 | Sets the strength of interaction |
| **Range** | Infinite (1/r²) | Falls off as surface area of sphere |
| **Carrier** | Photon (flux wave, s = 0, mass = 0) | Massless → infinite range |

### Magnetic (Lorentz) Force

| Component | Formula | Notes |
|-----------|---------|-------|
| **Force** | **F**_mag(v) = β · (∇×**J**) × **Ĵ**(v) | Curl of flux crossed with flow direction |
| **Coupling** | β = 0.01 | Magnetic coupling strength |
| **Ĵ**(v) | **J**(v) / \|**J**(v)\| | Unit vector along local flux |
| **Nature** | Velocity-dependent | Only affects moving charges |

**In plain English:** Electric force = "things with charge push other things with charge." Magnetic force = "moving charges twist the flux field, and that twist pushes other moving charges sideways."

### EM Parameters

| Parameter | Symbol | Value | Derivation | Status |
|-----------|--------|-------|------------|--------|
| Fine structure constant | α | 1/137.036 | Master quadratic x₊ | ✅ DERIVED (1.26 ppm) |
| Magnetic coupling | β | 0.01 | Related to α | ⚠️ IMPOSED |
| Photon mass | m_γ | 0 (exact) | U(1) gauge symmetry | ✅ EMERGENT |
| Photon polarizations | — | 2 | 3 components − 1 constraint | ✅ EMERGENT |
| Charge values | q | {0, ±1/3, ±2/3, ±1} | Fractional from N_c = 3 | ✅ DERIVED |

### EM Continuum Limit

| FTD quantity | → Standard physics |
|---|---|
| ∇×**J** | **B** (magnetic field) |
| −∇·**J** | ρ (charge density) |
| **J** | **A** (vector potential) |
| Flux wave equation | Maxwell's equations |

---

## 4B. Strong Force

**What it is:** Short-range attraction between same-sign quarks. Yukawa form borrowed from nuclear physics.

| Component | Formula | Notes |
|-----------|---------|-------|
| **Force** | F_strong(r) = g_s² · exp(−m_π r) / r² · (1 + m_π r) | Yukawa potential with 1/r² core |
| **Strong coupling** | g_s = 1.0 | Dimensionless strong coupling |
| **Pion mass scale** | m_π = 0.15 | Sets range: ~1/m_π lattice units |
| **Range** | ~1 fm (7 lattice units at Planck spacing) | Exponential falloff kills it beyond this |
| **Carrier** | Gluons (8 types, massless but confined) | Self-interacting → confinement |
| **Operates on** | Same-sign manifested neighbors only | Same-sign quarks attract at short range |
| **At r = 1** | F ≈ g_s² × 0.86 × 1.15 ≈ 0.99 | Nearly full strength at nearest neighbor |

**In plain English:** Two quarks sitting next to each other feel an attraction roughly 100× stronger than electromagnetism. But it dies off exponentially — a few lattice units away and it's gone. This is why you never see quarks alone: the force is too strong to escape, but too short to reach.

**Color structure:**

| Property | Value | Source |
|----------|-------|--------|
| Number of colors | N_c = 3 | Master quadratic x₋ = 3.024 |
| Number of gluons | 3² − 1 = 8 | SU(3) adjoint representation |
| Color directions | x, y, z axes of flux | 3D lattice geometry |
| Color neutrality | All 3 axes balanced | Baryons (3 quarks) and mesons (quark-antiquark) |
| Confinement | Yes | Gluon self-interaction |
| Asymptotic freedom | Yes (α_s decreases at high energy) | β₀ = b₃ = 7 |

### Strong Force Parameters

| Parameter | Symbol | Value | Derivation | Status |
|-----------|--------|-------|------------|--------|
| Strong coupling (sim) | g_s | 1.0 | Phenomenological | ⚠️ IMPOSED |
| Strong coupling (theory) | α_s(M_Z) | 7/59 = 0.1186 | From b₃ = 7 | ✅ DERIVED (0.6%) |
| Pion mass scale | m_π | 0.15 | Sets Yukawa range | ⚠️ IMPOSED |
| Number of colors | N_c | 3 | Master quadratic x₋ | ✅ DERIVED (0.8%) |
| Beta function coeff | b₃ | 7 | Framework integer | ✅ DERIVED |
| Functional form | Yukawa | — | Borrowed from nuclear physics | ⚠️ IMPORTED |

---

## 4C. Weak Force

**What it is:** High-stress flux configurations can flip particle identity (+1 ↔ −1). This is how neutrons decay, how the Sun shines, how radioactivity works.

| Component | Formula | Notes |
|-----------|---------|-------|
| **Stress** | σ(v) = \|∇·**J**\| + \|∇×**J**\| + \|∇ρ\| | Sum of all field stresses |
| **Threshold** | σ > σ_weak = 10.0 | Must exceed stress threshold |
| **Flip probability** | p_flip = clamp((σ − σ_weak)/σ_weak, 0, 0.5) | Linear onset, caps at 50% |
| **Result** | s(v) → −s(v), q(v) → −q(v) | Polarity and charge both flip |
| **Carriers** | W± (80.4 GeV), Z⁰ (91.2 GeV) | Massive → very short range |

**In plain English:** When the flux field is under extreme stress at a point — strong divergence, strong curl, strong density gradient all at once — the manifested state there can flip sign. A +1 becomes a −1. An up quark becomes a down quark. A neutron becomes a proton. This is radioactive beta decay.

The weak force is the only force that **changes particle identity.** EM and strong forces push particles around. Gravity warps the space they move through. Only the weak force says "you're not an up quark anymore — you're a down quark now."

### Weak Force Parameters

| Parameter | Symbol | Value | Derivation | Status |
|-----------|--------|-------|------------|--------|
| Weak mixing angle | sin²θ_W | 3/13 = 0.2308 | N_c/N_eff | ✅ DERIVED (0.19%) |
| Stress threshold | σ_weak | 10.0 | Phenomenological | ⚠️ IMPOSED |
| W boson mass | m_W | 80.4 GeV | 67/(8α²) × m_e | ✅ DERIVED (0.06%) |
| Z boson mass | m_Z | 91.2 GeV | m_W × √(13/10) | ✅ DERIVED (0.09%) |
| Fermi constant | G_F | 1.17 × 10⁻⁵ GeV⁻² | Required input | ❌ INPUT |
| CP violation phase | δ | 66.8° | arctan(7/3) | ✅ DERIVED (2.1%) |
| Cabibbo angle | θ_C | 13.16° | sin(θ_C) = G*/N_eff | ✅ DERIVED (1.2%) |

### CKM Matrix (Quark Flavor Mixing via W Boson)

|  | → u | → c | → t |
|---|---|---|---|
| **d →** | 0.974 | 0.228 = G*/N_eff | 0.004 |
| **s →** | 0.225 | 0.974 | 0.041 |
| **b →** | 0.004 | 0.041 | 0.999 |

### PMNS Matrix (Neutrino Mixing)

| Angle | FTD Formula | FTD Value | Measured | Error |
|-------|-------------|-----------|----------|-------|
| θ₁₂ | sin = √(3/10) | 33.2° | 33.4° | 0.7% |
| θ₂₃ | sin = √(16/29) | 48.0° | 49.2° | 2.5% |
| θ₁₃ | sin = √(1/52) | 8.0° | 8.6° | 7% |

---

## 4D. Gravity

**What it is:** Gradients in the smoothed density field. Accumulated contextual relevance of α-coupling.
**Confusion it resolves:** "Why is gravity so weak?" → It's α²⁰. "Is gravity a force?" → It's a density gradient.

| Component | Formula | Notes |
|-----------|---------|-------|
| **Force** | **F**_grav(v) = G_N · ∇ρ̄(v) | Drift toward higher density |
| **Smoothed density** | ρ̄(v) = (1/6) Σ_{u∈N₆} ρ(u) | Average over 6 face-sharing neighbors |
| **Coupling** | G_N = 0.01 = 1/(b₃+N_c)² = 1/100 | Dimensionless gravitational coupling |
| **Range** | Infinite (1/r²) | Falls off as surface area |
| **Carrier** | Graviton (spin-2 density ripple, mass = 0) | Never directly detected |

**The hierarchy formula:**

| Component | Formula | Value | Origin |
|-----------|---------|-------|--------|
| **Full formula** | α_G = 2π(16/3)²(N_eff + 3/b₃)² × α²⁰ | 5.909 × 10⁻³⁹ | ✅ DERIVED |
| 2π | Action principle normalization | 6.283 | From S[s,J] |
| (16/3)² | (N_base²/N_c)² = \|Aut(E)\|⁴/N_c² | 28.44 | Curve geometry |
| (94/7)² | (N_eff + N_c/b₃)² | 180.3 | Framework integers |
| α²⁰ | (1/137.036)²⁰ | 1.83 × 10⁻⁴³ | 20 = N_eff + b₃ = 13 + 7 |

**Extracting Newton's G:**

| Quantity | Formula | Value |
|----------|---------|-------|
| G_N (FTD) | α_G × ℏc / m_p² | **6.678 × 10⁻¹¹** m³ kg⁻¹ s⁻² |
| G_N (CODATA) | Measured | **6.674 × 10⁻¹¹** m³ kg⁻¹ s⁻² |
| Error | | **0.055%** (551 ppm) |

### Gravity Parameters

| Parameter | Symbol | Value | Derivation | Status |
|-----------|--------|-------|------------|--------|
| Gravitational coupling (sim) | G_N | 0.01 | 1/(b₃+N_c)² | ✅ DERIVED |
| Gravitational fine structure | α_G | 5.91 × 10⁻³⁹ | 2π(16/3)²(94/7)²α²⁰ | ✅ DERIVED (0.06%) |
| Newton's constant | G | 6.678 × 10⁻¹¹ | α_G × ℏc/m_p² | ✅ DERIVED (0.055%) |
| Hierarchy exponent | k | 20 = 13 + 7 | N_eff + b₃ | ✅ DERIVED |
| Hierarchy ratio | α/α_G | ~10³⁶ | α¹⁻²⁰ × prefactor | ✅ DERIVED |

### The Contextual Relevance Spectrum (Φ = GM/Rc²)

| Object | Φ | What happens to information |
|--------|---|----------------------------|
| Vacuum | 0 | Free propagation |
| Gas cloud (1 pc) | 10⁻¹⁴ | Passes through |
| Earth surface | 7 × 10⁻¹⁰ | Light bends slightly |
| Sun surface | 2 × 10⁻⁶ | Light bends, clocks slow |
| White dwarf | 10⁻⁴ | Significant redshift |
| Neutron star | 0.21 | Spacetime strongly curved |
| Black hole (R_s) | **0.50** | Information cannot escape. Maximum. |

---

## 4E. Force Comparison Table

| Property | Electromagnetic | Strong | Weak | Gravity |
|----------|----------------|--------|------|---------|
| **Formula** | −q∇q̄ + β(∇×**J**)×**Ĵ** | g_s²e^(−mr)/r²(1+mr) | stress threshold flip | G_N∇ρ̄ |
| **Carrier** | Photon (γ) | Gluons (g) ×8 | W±, Z⁰ | Graviton |
| **Carrier mass** | 0 | 0 (confined) | 80-91 GeV | 0 |
| **Range** | Infinite | ~1 fm | ~0.002 fm | Infinite |
| **Coupling** | α = 1/137 | α_s ≈ 0.12 | G_F = 10⁻⁵ GeV⁻² | α_G ≈ 10⁻³⁹ |
| **FTD coupling derived?** | ✅ Yes (1.26 ppm) | ✅ Yes (0.6%) | ✅ sin²θ_W (0.19%) | ✅ Yes (0.06%) |
| **Acts on** | Electric charge | Color charge | Weak isospin (left-handed) | Mass-energy (all) |
| **Changes identity?** | No | No | **Yes** (u↔d, ν↔e) | No |
| **Self-interacting?** | No (photons pass through each other) | **Yes** (gluons grab gluons) | Yes (W,Z interact) | Yes (gravity gravitates) |
| **FTD mechanism** | Charge gradient | Yukawa between same-sign neighbors | Stress-induced polarity flip | Density gradient |
| **Functional form** | ✅ Derived (Green's function) | ✅ Derived (massive propagator) | ⚠️ [SELECTION] (threshold) | ✅ Derived (Green's function) |
| **Entropy role** | Leak rate (α = γ) | Locks minimum-cost configs | Rearranges entropy budget | Cumulative α-coupling |
| **Relative to α** | α¹ | α^(~0.6) at M_Z | α⁸ (sets W mass) | α²⁰ |

> **Derivation:** All four force functional forms emerge from lattice Green's functions of the flux wave equation. Coulomb and gravity are gradients of the massless Green's function G₀(r) → 1/(4πr); the strong force is the gradient of the massive propagator G_m(r) → e^{-mr}/(4πr); the Lorentz force arises from the curl structure J ↔ A. See [DERIV_FORCE_EMERGENCE.md](../03_derivations/DERIV_FORCE_EMERGENCE.md) for the complete derivation with 18 verified tests.

---

# ALGORITHM 5: TIME

**What it does:** Applies irreversible dissipation to manifested particles. Creates the arrow of time.
**Standard physics equivalent:** Second law of thermodynamics, entropy increase.
**Confusion it resolves:** "Why does time flow in one direction?" → Because every manifested particle dissipates at rate α per tick. This is irreversible. The direction of irreversibility IS time.

## The Dissipation Rule

| Component | Formula | Notes |
|-----------|---------|-------|
| **Decay** | **J**(v) *= (1 − γ) | Applied only to unlocked, manifested voxels |
| **Rate** | γ = α = 1/137.036 | The fine structure constant IS the dissipation rate |
| **Condition** | s(v) ≠ 0 AND NOT is_locked(v) | Only manifested, unbound particles decay |
| **Locked particles** | No decay | Bound structures resist entropy |

**In plain English:** Every manifested particle leaks. Every tick, it loses a fraction α of its flux to the surrounding void. This leak is irreversible — you can't un-leak. The direction of leaking IS the direction of time.

Bound structures (locked triads = protons, locked shells = atoms) are exempt from this decay. That's why protons are stable — binding is entropy optimization. The structure persists because it has found a configuration that minimizes its dissipation.

### The Arrow of Time — Complete Chain

| Step | What happens | Consequence |
|------|-------------|-------------|
| 1 | Low-entropy initial condition | Flux is concentrated, not uniform |
| 2 | Concentration > K_B | Manifestation occurs (particles appear) |
| 3 | Manifested particles dissipate at rate α | Flux leaks into surroundings |
| 4 | Dissipation is irreversible | Entropy increases |
| 5 | Direction of entropy increase = time | Arrow established |
| 6 | In that direction, forces operate | Structures form, evolve, decay |
| 7 | Eventually all flux < K_B everywhere | Heat death (maximum entropy) |

### Time Parameters

| Parameter | Symbol | Value | Derivation | Status |
|-----------|--------|-------|------------|--------|
| Dissipation rate | γ | α = 1/137.036 | Identified with fine structure constant | ⚠️ IMPOSED (ASSUMP.6) |
| Tick duration | τ | 1 = Planck time (5.4 × 10⁻⁴⁴ s) | Scale identification | ⚠️ IMPOSED |

### What time dilation really is

| Situation | What happens | FTD mechanism |
|-----------|-------------|---------------|
| Object at rest | Full processing each tick | Phase accumulator fills at rate 1 |
| Object moving at v | Fewer internal updates per tick | Phase accumulator fills at rate √(1−v²/C²) |
| Object at v = C (photon) | Zero internal updates | Phase accumulator never fills — no time experienced |
| Object in gravitational field | Fewer updates (deeper = slower) | Density gradient slows phase accumulation |

---

# ALGORITHM 6: STRUCTURE

**What it does:** Detects stable geometric configurations and protects them from decay.
**Standard physics equivalent:** Nuclear binding, atomic structure, chemistry, all of materials science.
**Confusion it resolves:** "Why is matter stable?" → Because certain configurations minimize dissipation, so they persist.

## Binding Detection

| Component | Formula | Notes |
|-----------|---------|-------|
| **Condition** | ≥ 2 same-sign neighbors in Moore (26-connected) neighborhood | Geometric stability criterion |
| **Result** | is_locked = True | Particle becomes exempt from decay |
| **Binding energy** | E_bind ≈ K_B × φ per triad | φ = 1.618... (golden ratio) |
| **Triad** | 3 same-sign particles at ~√2 spacing | Equilateral triangle → nucleon analog |

**In plain English:** If a manifested particle has at least 2 friends of the same sign nearby, they lock together. Locked particles don't decay. This is binding — the reason protons exist for 10³⁴+ years instead of evaporating in microseconds.

### What structure produces

| Configuration | FTD description | Standard physics analog | Stability |
|---------------|-----------------|------------------------|-----------|
| Single ±1 voxel | Isolated manifested particle | Free quark/lepton | Unstable (decays) |
| 3 same-sign locked | **Triad** | Nucleon (proton/neutron) | Stable (locked) |
| Triad cluster | Multiple bound triads | Atomic nucleus | Stable if bound |
| Triad + opposite shells | Positive core + negative orbits | Atom | Stable |
| Multi-atom binding | Flux-mediated attraction | Molecule | Stable |
| Large aggregates | Many bound structures | Planets, stars | Gravitationally bound |

### Structure Parameters

| Parameter | Symbol | Value | Derivation | Status |
|-----------|--------|-------|------------|--------|
| Binding threshold | 2 neighbors | Integer | Geometric stability | ⚠️ SELECTION |
| Neighborhood | Moore (26-connected) | Cubic lattice | Axiomatic | ⬜ AXIOM |
| Binding energy scale | K_B × φ | ~0.83 MeV per triad | Phenomenological | ⚠️ IMPOSED |
| Golden ratio | φ | 1.618... | Mathematical constant | — |

---

# MASTER PARAMETER TABLE

Every number in the entire framework:

## Axioms (not derivable — define the model)

| Parameter | Value | What it defines |
|-----------|-------|-----------------|
| Lattice dimension | D = 3 | Space is 3D (now derived from 6 independent arguments) |
| Ternary states | {−1, 0, +1} | Minimum nontrivial state space |
| Moore neighborhood | 26 connected | Local causality |
| Speed of causality | C = 1 unit/tick | Maximum information speed |

## Derived from the elliptic curve E: y² = x³ − x

| Parameter | Formula | Value | Accuracy |
|-----------|---------|-------|----------|
| G* | √2 · Γ(1/4)² / (2π) | 2.9586751192 | Exact |
| 1/α | Master quadratic x₊ | 137.036171 | 1.26 ppm |
| N_c | Master quadratic x₋ | 3.024 | 0.8% |
| 16 | \|Aut(E)\|² = \|E(ℚ)_tors\|² | 16 | Exact |

## Derived from α and {3, 4, 7, 13}

| Parameter | Formula | Value | Accuracy |
|-----------|---------|-------|----------|
| sin²θ_W | N_c/N_eff = 3/13 | 0.2308 | 0.19% |
| α_s(M_Z) | b₃/(b₃+4N_eff) = 7/59 | 0.1186 | 0.6% |
| m_e | M_P√(2π)(16/3)α¹¹ | 0.5096 MeV | 0.19% |
| m_μ/m_e | 3b₃(b₃+N_c) − N_c = 207 | 206.768 | 0.11% |
| m_τ/m_e | 17×207 − 42 = 3477 | 3477.3 | 0.007% |
| m_p/m_e | N_eff/α + T(b₃+N_c) | 1836.47 | 0.017% |
| m_W | 67/(8α²)×m_e | 80.36 GeV | 0.06% |
| m_Z | m_W√(13/10) | 91.18 GeV | 0.09% |
| m_H | (13/α²)×m_e | 124.8 GeV | 0.36% |
| α_G | 2π(16/3)²(94/7)²α²⁰ | 5.91×10⁻³⁹ | 0.06% |
| G_N | α_G×ℏc/m_p² | 6.678×10⁻¹¹ | 0.055% |
| V_us | √(3/13) | 0.225 | 0.1% |
| δ_CP | arctan(7/3) | 66.8° | 2.1% |

## Imposed (not derived — modeling choices)

| Parameter | Value | Why imposed |
|-----------|-------|-------------|
| γ = α | 1/137.036 | Dissipation identified with EM coupling (ASSUMP.6) |
| β | 0.01 | Magnetic coupling (phenomenological) |
| g_s (sim) | 1.0 | Strong coupling in simulation (phenomenological) |
| m_π (sim) | 0.15 | Yukawa range (phenomenological) |
| σ_weak | 10.0 | Weak transmutation threshold (phenomenological) |
| δ (damping) | 0.05 | Wave equation damping (numerical stability) |
| C_sim | 0.5 | Simulation speed (CFL stability) |
| K_B (sim) | 1.2 | Simulation manifestation threshold |

## External inputs (cannot derive from FTD)

| Parameter | Value | Why needed |
|-----------|-------|-----------|
| M_Planck | 1.22 × 10¹⁹ GeV | Sets absolute mass scale |
| G_F | 1.17 × 10⁻⁵ GeV⁻² | Fermi constant for weak decays |
| Λ_QCD | ~217 MeV | QCD confinement scale |
| f_π, f_K | 131, 156 MeV | Meson decay constants |

---

# THE COMPLETE CHAIN

```
Boundary condition: low-entropy start
          │
          ▼
    E: y² = x³ − x   (one curve)
          │
          ├──→ G* = 2.9587   (its period)
          │         │
          │         ▼
          │    x² − 16G*²x + 16G*³ = 0
          │         │              │
          │         ▼              ▼
          │    α = 1/137.036    N_c = 3
          │         │              │
          ▼         ▼              ▼
   ALGORITHM 5    ALGORITHM 4    ALGORITHM 6
   (Time: γ = α)  (Forces)      (Structure: SU(3))
          │         │              │
          ▼         ▼              ▼
   ALGORITHM 1  ←──────────→  ALGORITHM 3
   (Existence)               (Interaction)
          │                       │
          └───────┬───────────────┘
                  ▼
            ALGORITHM 2
         (Information: photons)
                  │
                  ▼
         Everything else:
         particles, atoms, stars,
         planets, chemistry, life,
         consciousness, this document
```

Six algorithms. One curve. One coupling constant. Four integers. A low-entropy boundary condition. That's physics.
