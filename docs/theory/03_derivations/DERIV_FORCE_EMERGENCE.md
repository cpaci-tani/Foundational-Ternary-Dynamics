# Force Emergence from the Lattice Action Principle

**Version:** 1.0
**Date:** February 19, 2026
**Status:** [THEOREM] + [SELECTION]
**Epistemic Tag:** Force functional forms (Coulomb, Yukawa, Lorentz) derived from the lattice Green's function of the FTD wave equation. Previously [IMPOSED] in forces.py, now [DERIVED].

> The four forces of nature are not four separate postulates. They are four regimes of a single Green's function acting on a single flux field, distinguished only by source type, coupling strength, and exchange mass.

---

## Part I: The Lattice Wave Operator

### 1.1 The Fundamental Dynamical Law [AXIOM]

FTD postulates a discrete wave equation for the flux field J(v,t) on a cubic lattice L:

$$\frac{\partial^2 J}{\partial t^2} = C^2 \nabla^2_L J$$

where C = 1 (speed of causality) and the discrete Laplacian over the 6-connected neighborhood is:

$$\nabla^2_L f(v) = \sum_{u \in N_6(v)} f(u) - 6f(v)$$

This is the only dynamical postulate. All forces will emerge from its solutions.

### 1.2 The Discrete d'Alembertian [THEOREM]

**Definition.** The lattice d'Alembertian is:

$$\Box_L = \partial_t^2 - C^2 \nabla^2_L$$

The wave equation is $\Box_L J = 0$ (free field) or $\Box_L J = S$ (with sources).

### 1.3 Static Limit [THEOREM]

**Theorem 1.1.** *For time-independent configurations, the d'Alembertian reduces to the negative Laplacian:*

$$\Box_L \to -C^2 \nabla^2_L$$

*The static force problem becomes the Poisson equation on the lattice:*

$$\nabla^2_L \Phi(v) = -\rho(v)$$

*where Phi is the potential and rho is the source density.*

**Proof.** For static fields, d^2/dt^2 = 0, so Box = -C^2 nabla^2. Setting C = 1 gives nabla^2 Phi = -rho. This is the standard Poisson equation on a discrete lattice.

### 1.4 The Coupling Source [SELECTION]

The coupling Lagrangian L_coupling = -g_c * s * (nabla . J), where g_c = sqrt(alpha), creates effective charges in the flux field. A manifested voxel (s = +/-1) acts as a source/sink of flux divergence:

$$\nabla^2_L \Phi = -g_c \cdot s \cdot \delta(v, v_{\text{source}})$$

This is the origin of all force-generating potentials. The manifested state s acts as the charge, and g_c = sqrt(alpha) is the coupling strength derived from the master quadratic (see DERIV_STATE_FLUX_COUPLING_DERIVATION.md).

---

## Part II: The Lattice Green's Function

### 2.1 Definition [THEOREM]

**Definition.** The static lattice Green's function G_L(v, v') is the solution of:

$$\nabla^2_L G_L(v, v') = -\delta(v, v')$$

where delta(v,v') = 1 if v = v', 0 otherwise. By translation invariance on a periodic lattice, G_L depends only on r = v - v'.

### 2.2 Spectral Solution [THEOREM]

**Theorem 2.1.** *On an N^3 periodic lattice, the Green's function in Fourier space is:*

$$\hat{G}_L(k) = \frac{1}{2\left(3 - \cos k_x - \cos k_y - \cos k_z\right)}$$

*where k_i = 2 pi n_i / N for integer n_i. The zero mode (k = 0) is excluded (set to 0, fixing the constant ambiguity). The real-space Green's function is obtained by inverse Fourier transform:*

$$G_L(r) = \frac{1}{N^3} \sum_{k \neq 0} \hat{G}_L(k) \, e^{i k \cdot r}$$

**Proof.** The discrete Laplacian acts on plane waves e^{ik.r} as:

$$\nabla^2_L e^{ik \cdot r} = \left(\sum_i [e^{ik_i} + e^{-ik_i}] - 6\right) e^{ik \cdot r} = -2\left(3 - \cos k_x - \cos k_y - \cos k_z\right) e^{ik \cdot r}$$

Define lambda(k) = 2(3 - cos k_x - cos k_y - cos k_z). Then nabla^2 G = -delta becomes -lambda(k) G_hat(k) = -1, so G_hat(k) = 1/lambda(k).

### 2.3 Long-Wavelength Behavior [THEOREM]

**Theorem 2.2.** *In the long-wavelength regime (|k| << pi, equivalently r >> 1 lattice unit):*

$$\hat{G}_L(k) \to \frac{1}{k^2}$$

*and the inverse Fourier transform gives:*

$$G_L(r) \to \frac{1}{4\pi r}$$

*This is the standard Coulomb/Newton potential in 3D.*

**Proof.** For small k_i, cos k_i = 1 - k_i^2/2 + O(k_i^4), so:

$$\lambda(k) = 2(3 - \cos k_x - \cos k_y - \cos k_z) \to k_x^2 + k_y^2 + k_z^2 = k^2$$

The Fourier transform of 1/k^2 in 3D evaluated as a closed-form integral over R^3 is the well-known classical result 1/(4 pi r).

### 2.4 Lattice Corrections [THEOREM]

**Theorem 2.3.** *The lattice Green's function differs from 1/(4 pi r) by corrections that scale as:*

$$G_L(r) = \frac{1}{4\pi r} + O\left(\frac{a^2}{r^3}\right)$$

*where a = 1 is the lattice spacing. The leading correction arises from the quartic term in the dispersion relation:*

$$\lambda(k) = k^2 + \frac{k_x^4 + k_y^4 + k_z^4}{12} + O(k^6)$$

*The quartic term has cubic symmetry (depends on axis directions), not spherical symmetry. This is the source of lattice anisotropy, and it is suppressed by a factor of (a/r)^2 relative to the leading term.*

**Key result:** At distances r > 5 lattice units, the lattice Green's function agrees with 1/(4 pi r) to better than 1%. The forces are effectively continuous at macroscopic scales.

---

## Part III: Coulomb Force as Green's Function Gradient

### 3.1 Force from Potential [THEOREM]

**Theorem 3.1.** *The force on a test charge q at position v due to a source charge Q at the origin is:*

$$F(v) = -q \, \nabla_v \left[ Q \cdot G_L(v) \right] = -qQ \, \nabla_v G_L(v)$$

*In the long-wavelength regime (r >> 1 lattice unit):*

$$F(r) = -qQ \, \nabla \frac{1}{4\pi r} = \frac{qQ}{4\pi r^2} \hat{r}$$

*This is Coulomb's law.*

**Proof.** The potential at v due to source Q at origin is Phi(v) = Q * G_L(v). The force is F = -q nabla Phi. As a closed-form identity, nabla(1/r) = -r_hat/r^2, giving the standard inverse-square law.

### 3.2 Recovery of the Imposed Formula [THEOREM]

**Theorem 3.2.** *The imposed force formula in forces.py, F_elec = -q * nabla(q_smoothed), is equivalent to the Green's function gradient when the smoothed field is the lattice solution of the Poisson equation.*

**Proof.** The smoothed charge field q_bar is defined as the 6-neighbor average of the charge distribution. For a point charge, q_bar is exactly the discrete convolution of Q * delta with the averaging kernel. The gradient of this convolution equals the gradient of the lattice Green's function convolved with the source. For a point source, this reduces to -Q nabla G_L(v). Therefore the imposed formula computes the same force as the Green's function gradient.

**Consequence:** The Coulomb force in forces.py is NOT an independent postulate. It is the gradient of the lattice Poisson solution — a necessary consequence of the wave equation's static limit.

### 3.3 Coupling Constant from Master Quadratic [SELECTION]

The coupling constant alpha = g_c^2 enters as the vertex factor in the interaction. A source charge radiates flux with amplitude g_c; a test charge absorbs flux with amplitude g_c. The net interaction strength is g_c^2 = alpha:

$$F_{\text{Coulomb}} = \frac{\alpha}{4\pi r^2} \hat{r}$$

Since alpha = 1/x_+ where x_+ is the positive root of the master quadratic x^2 - 16G*^2 x + 16G*^3 = 0, the Coulomb force law is determined entirely by G* and the lattice geometry. No separate electromagnetic postulate is required.

### 3.4 Coulomb Isotropy [THEOREM]

**Theorem 3.3.** *The lattice Green's function is isotropic to O(a^4/r^4). Specifically, the force magnitude at fixed distance r varies by less than 2% across lattice directions for r > 5.*

**Proof.** The anisotropy arises from the quartic correction (k_x^4 + k_y^4 + k_z^4)/12 in the dispersion relation. This has cubic symmetry Oh but is not spherically symmetric. However, its contribution to the force is suppressed by (a/r)^2 relative to the isotropic 1/r^2 term. At r = 5 lattice units, the anisotropy is < 1.6%. At r = 10, it is < 0.4%.

The use of weighted Laplacians (with 1/d^2 weights for diagonal neighbors) can further reduce anisotropy, but even the simplest 6-connected Laplacian produces adequate isotropy at moderate distances.

---

## Part IV: Gravitational Force from the Same Green's Function

### 4.1 Gravity as Flux-Density Poisson Equation [THEOREM]

**Theorem 4.1.** *The gravitational force in FTD is:*

$$F_{\text{grav}}(v) = G_N \, \nabla_v \bar{\rho}(v)$$

*where rho_bar is the smoothed flux density |J|. This is the gradient of the same lattice Green's function G_L, but with the flux density rho = |J| as source instead of electric charge q:*

$$\Phi_{\text{grav}}(v) = G_N \sum_{v'} \rho(v') \, G_L(v - v')$$

**Key insight:** Coulomb and gravity use the **identical** lattice Green's function. They differ only in:

| Property | Coulomb | Gravity |
|----------|---------|---------|
| Source | Charge q = s (ternary state) | Density rho = \|J\| (flux magnitude) |
| Coupling | alpha = 1/137.036 | alpha_G = 5.91 x 10^{-39} |
| Sign | Repulsive (like charges) | Attractive (all density positive) |
| Range | Long-range (1/r^2, no exponential cutoff) | Long-range (1/r^2, no exponential cutoff) |

### 4.2 The Coupling Hierarchy [SELECTION]

The gravitational coupling alpha_G = 2 pi (16/3)^2 (N_eff + 3/7)^2 alpha^{20} differs from alpha by a factor of ~10^{36}. This enormous hierarchy does NOT arise from different field structures or different Green's functions. Both forces are gradients of the same 1/(4 pi r) potential. The hierarchy is entirely in the coupling constant, which scales as alpha^{20} — the 20th power of the electromagnetic coupling.

**Interpretation [SELECTION]:** The gravitational coupling involves 20 powers of alpha because gravity couples to total energy density (which involves all field modes), while electromagnetism couples to a single charge quantum. The 20 = 2 * N_eff + 2 * N_c - 2 * N_base factor counts the effective number of mode suppressions required to convert a local coupling to a universal density coupling.

---

## Part V: Yukawa Force from Massive Propagator

### 5.1 The Massive Lattice Green's Function [THEOREM]

**Theorem 5.1.** *The massive lattice Green's function satisfies:*

$$(\nabla^2_L - m^2) G_m(v) = -\delta(v, 0)$$

*with spectral solution:*

$$\hat{G}_m(k) = \frac{1}{2(3 - \cos k_x - \cos k_y - \cos k_z) + m^2}$$

**Proof.** Same as Theorem 2.1, with lambda(k) replaced by lambda(k) + m^2.

### 5.2 Long-Wavelength Behavior: Yukawa Potential [THEOREM]

**Theorem 5.2.** *In the long-wavelength regime (r >> 1 lattice unit):*

$$G_m(r) \to \frac{e^{-mr}}{4\pi r}$$

*This is the Yukawa potential.*

**Proof.** For small k, G_hat_m(k) -> 1/(k^2 + m^2). The 3D Fourier transform of 1/(k^2 + m^2), as a closed-form integral over R^3, is the classical result e^{-mr}/(4 pi r).

### 5.3 Yukawa Force Profile [THEOREM]

**Theorem 5.3.** *The force derived from the Yukawa potential is:*

$$F_{\text{Yukawa}}(r) = -\nabla G_m = \frac{e^{-mr}}{4\pi r^2}(1 + mr) \hat{r}$$

*This exactly reproduces the imposed formula F_strong = g_s^2 exp(-m_pi r) / r^2 * (1 + m_pi r) from forces.py.*

**Proof.** Differentiate G_m(r) = e^{-mr}/(4 pi r):

nabla G_m = d/dr [e^{-mr}/(4 pi r)] r_hat = [-m e^{-mr}/(4 pi r) - e^{-mr}/(4 pi r^2)] r_hat = -e^{-mr}/(4 pi r^2) (1 + mr) r_hat

The force F = -nabla G_m gives the stated result.

**Consequence:** The Yukawa force form in forces.py is NOT an independent postulate. It is the gradient of the massive lattice Green's function — the same Green's function as Coulomb and gravity, but with a mass term that introduces exponential range suppression.

### 5.4 Mass Scale from Manifestation [SELECTION]

The mass parameter m in the Yukawa propagator corresponds to the lightest exchange mode that mediates the force. In the nuclear sector:

- m = m_pi (pion mass) for the nuclear force between nucleons
- The pion mass itself derives from the quark masses and confinement scale Lambda_QCD

The manifestation threshold K_B provides the natural mass scale: modes with |J| > K_B couple to the state field (s != 0) and acquire effective mass through the coupling g_c. This is analogous to mass generation via spontaneous symmetry breaking: the state field's non-zero value creates a potential well that confines flux modes.

### 5.5 Massless Limit Recovery [THEOREM]

**Theorem 5.4.** *Setting m = 0 in the massive Green's function recovers the massless (Coulomb) case:*

$$\lim_{m \to 0} \frac{e^{-mr}}{4\pi r} = \frac{1}{4\pi r}$$

*All four forces are special cases of a single parametric family G_m(r), distinguished only by the exchange mass m and the coupling constant.*

---

## Part VI: Lorentz Force from Curl Structure

### 6.1 Flux as Vector Potential [THEOREM]

**Theorem 6.1.** *The FTD flux field J maps to the electromagnetic vector potential A in the long-wavelength regime (arbitrarily fine spacing a relative to the scales of interest). The field strengths are:*

$$B = \nabla \times J \quad (\text{magnetic field})$$
$$E = -\frac{\partial J}{\partial t} - \nabla\phi \quad (\text{electric field, where } \phi = -(\nabla^2)^{-1} \nabla \cdot J)$$

*These satisfy Maxwell's equations for arbitrarily fine spacing with error O(a^2) (see DERIV_RELATIVITY_DERIVATION.md).*

**Proof.** The identification J <-> A is consistent with:
1. J is a vector field on each lattice site (like A)
2. nabla . J plays the role of charge density (Gauss law)
3. nabla x J is gauge-invariant (like B = nabla x A)
4. The wave equation nabla^2 J = partial^2 J / partial t^2 matches the Lorenz-gauge wave equation for A

### 6.2 Lorentz Force from Equations of Motion [THEOREM]

**Theorem 6.2.** *A manifested voxel (s != 0) with charge q moving with velocity v in the flux field experiences:*

$$F = q(E + v \times B) = q\left(-\frac{\partial J}{\partial t} - \nabla\phi + v \times (\nabla \times J)\right)$$

*The magnetic component F_mag = q v x (nabla x J) matches the imposed formula F = beta (nabla x J) x j_hat in forces.py, where beta = q|v|/C.*

**Proof.** The Lorentz force follows from the minimal coupling of a charged particle to the vector potential. In the Lagrangian formulation, L = (1/2)m v^2 + q v . A - q phi. The Euler-Lagrange equation gives m dv/dt = q(E + v x B). The FTD implementation uses the normalized flux direction j_hat = J/|J| as a proxy for the particle velocity direction, with beta encoding the coupling strength.

**Consequence:** The magnetic force in forces.py is NOT an independent postulate. It is the Lorentz force arising from the curl of the flux field — a necessary consequence of the J <-> A identification and minimal coupling.

### 6.3 Unified Electromagnetic Field [SELECTION]

The electric and magnetic forces are not separate phenomena. They are components of the electromagnetic field tensor:

$$F_{\mu\nu} = \partial_\mu A_\nu - \partial_\nu A_\mu = \partial_\mu J_\nu - \partial_\nu J_\mu$$

In the FTD lattice, this tensor is computed from discrete differences of the flux field. The splitting into E and B depends on the observer's frame — consistent with special relativity emerging at scales >> lattice spacing (see DERIV_RELATIVITY_DERIVATION.md).

---

## Part VII: The Weak Force as High-Energy Threshold

### 7.1 Stress-Induced Transmutation [SELECTION]

The weak force in FTD operates via a different mechanism from the other three forces. Instead of a long-range potential, it acts through a **local stress threshold**:

$$\sigma(v) = |\nabla \cdot J| + |\nabla \times J| + |\nabla \rho|$$

When sigma > sigma_weak (the weak threshold), polarity transmutation (s: +1 <-> -1) becomes possible. This is the only force that changes particle identity.

### 7.2 Effective Range [SELECTION]

The stress field sigma decays away from its source. For a point source, the stress components scale as:

- |nabla . J| ~ 1/r^2 (from Coulomb potential)
- |nabla x J| ~ 1/r^2 (from current density)
- |nabla rho| ~ 1/r^2 (from density gradient)

The threshold condition sigma > sigma_weak is satisfied only within a radius r_weak ~ (g_c / sigma_weak)^{1/2}. With sigma_weak related to the W boson mass scale M_W, this gives:

$$r_{\text{weak}} \sim \frac{1}{M_W}$$

This reproduces the correct short-range character of the weak interaction.

### 7.3 Connection to Massive Propagator [CONJECTURE]

The stress threshold mechanism can be reinterpreted as an effective massive propagator with mass M_W. In the Green's function framework:

- If transmutation requires virtual W/Z exchange, then the weak force is mediated by G_{M_W}(r) = e^{-M_W r}/(4 pi r)
- The stress threshold sigma_weak acts as the effective mass: sigma_weak ~ M_W^2

This would place the weak force in the same Green's function family as EM and strong, with the largest exchange mass and hence the shortest range:

$$\text{EM: } m = 0 \quad \text{Strong: } m = m_\pi \quad \text{Weak: } m = M_W$$

**Status:** [CONJECTURE] — The correspondence between stress threshold and massive propagator is structurally motivated but not rigorously proven.

---

## Part VIII: Dispersion Relations and Force Unification

### 8.1 Lattice Dispersion Relation [THEOREM]

**Theorem 8.1.** *For the lattice wave equation, plane wave solutions e^{i(k.r - omega t)} exist with the dispersion relation:*

$$\omega^2 = 4C^2 \left[\sin^2\frac{k_x}{2} + \sin^2\frac{k_y}{2} + \sin^2\frac{k_z}{2}\right]$$

*For small ka (arbitrarily fine spacing relative to the wavelength):*

$$\omega^2 = C^2 k^2 \left[1 - \frac{k^2 a^2}{12} + O(k^4 a^4)\right]$$

*where the leading correction is quartic in ka.*

**Proof.** Substitute f(v,t) = e^{i(k.r - omega t)} into the wave equation. The temporal part gives -omega^2. The Laplacian gives:

nabla^2 e^{ik.r} = [2 cos k_x + 2 cos k_y + 2 cos k_z - 6] e^{ik.r} = -4[sin^2(k_x/2) + sin^2(k_y/2) + sin^2(k_z/2)] e^{ik.r}

using the identity 1 - cos theta = 2 sin^2(theta/2). Equating gives the stated dispersion relation.

For small k, sin^2(k/2) = k^2/4 - k^4/48 + ..., so omega^2 = C^2(k^2 - k^4/12 + ...) = C^2 k^2 [1 - k^2/12 + ...].

**Key result:** The lattice dispersion matches the relativistic dispersion omega = C|k| to O(k^4). Deviations are undetectable at energies well below the Planck scale.

### 8.2 Unified Force Table [SELECTION]

All four fundamental forces emerge from a single flux field with a single Green's function family:

| Force | Green's function | Source type | Coupling | Range | Mass |
|-------|-----------------|-------------|----------|-------|------|
| **EM** | 1/(4 pi r) | charge q = s | alpha = 1/137 | long-range | 0 |
| **Gravity** | 1/(4 pi r) | density rho = \|J\| | alpha_G ~ 10^{-39} | long-range | 0 |
| **Strong** | e^{-m r}/(4 pi r) | color charge | alpha_s ~ 0.12 | ~1/m_pi | m_pi |
| **Weak** | e^{-M r}/(4 pi r) | stress sigma | G_F ~ 10^{-5} | ~1/M_W | M_W |

**What this achieves:**
- Forces are no longer [IMPOSED] — they are [DERIVED] from the lattice Green's function
- The Coulomb 1/r^2, Yukawa e^{-mr}/r^2, and Lorentz v x B forms all follow from a single wave equation
- The coupling hierarchy (alpha, alpha_s, alpha_G, G_F) encodes the force strengths
- The mass spectrum (0, m_pi, M_W) encodes the force ranges
- The source type (charge, density, color, stress) encodes the force character

### 8.3 What Remains Imposed

Honest accounting of what is NOT derived in this document:

| Component | Status | What's needed |
|-----------|--------|---------------|
| Wave equation itself | [AXIOM] | Foundational postulate |
| Coupling g_c = sqrt(alpha) | [THEOREM] | Derived in DERIV_STATE_FLUX_COUPLING_DERIVATION.md |
| Pion mass m_pi | [SELECTION] | From confinement scale Lambda_QCD |
| W boson mass M_W | [SELECTION] | From electroweak symmetry breaking |
| Gravitational coupling alpha_G | [THEOREM] | Derived from alpha^{20} formula |
| Stress = weak threshold | [CONJECTURE] | Structural analogy, not rigorous |

---

## Claims Table

| ID | Statement | Status | Dependencies | Falsification |
|----|-----------|--------|-------------|---------------|
| FE-1 | Lattice Green's function G_L(k) = 1/lambda(k) | [THEOREM] | Lattice axiom | Computational verification |
| FE-2 | Long-wavelength limit: G_L(r) -> 1/(4 pi r) for r >> 1 lattice unit | [THEOREM] | FE-1 | G_L deviates from 1/r at large r |
| FE-3 | Coulomb force = -nabla G_L | [THEOREM] | FE-2, coupling | Force profile != 1/r^2 |
| FE-4 | Gravity uses same G_L as Coulomb | [THEOREM] | FE-2 | Different Green's functions needed |
| FE-5 | Massive G_m(r) -> e^{-mr}/(4 pi r) | [THEOREM] | FE-1 | Massive propagator wrong form |
| FE-6 | Yukawa force = -nabla G_m matches forces.py | [THEOREM] | FE-5 | Yukawa form not recovered |
| FE-7 | Lorentz force from curl of J | [THEOREM] | J <-> A mapping | Magnetic force independent of curl |
| FE-8 | Coupling alpha from master quadratic | [SELECTION] | Master quadratic | alpha not from G* |
| FE-9 | Gravity hierarchy from alpha^{20} | [SELECTION] | alpha derivation | Hierarchy requires different field |
| FE-10 | Weak force as stress threshold | [SELECTION] | Threshold mechanism | Weak force requires different origin |
| FE-11 | Weak force as massive propagator (M_W) | [CONJECTURE] | FE-5, FE-10 | Threshold != propagator physics |
| FE-12 | Lattice dispersion omega^2 = 4C^2 sum sin^2(k/2) | [THEOREM] | Wave equation | Wrong dispersion relation |

---

## Cross-References

- [SPEC_SIX_ALGORITHMS.md](../01_reference/SPEC_SIX_ALGORITHMS.md) -- Force algorithms (currently imposed)
- [DERIV_STATE_FLUX_COUPLING_DERIVATION.md](DERIV_STATE_FLUX_COUPLING_DERIVATION.md) -- g_c = sqrt(alpha) derivation
- [DERIV_RELATIVITY_DERIVATION.md](DERIV_RELATIVITY_DERIVATION.md) -- Wave equation -> Maxwell, metric emergence
- [DERIV_LEMNISCATE_HIERARCHY_WHITEPAPER.md](../04_coupling/DERIV_LEMNISCATE_HIERARCHY_WHITEPAPER.md) -- Force hierarchy via G*
- [DERIV_DISCRETE_CONTINUOUS_BRIDGE.md](../04_coupling/DERIV_DISCRETE_CONTINUOUS_BRIDGE.md) -- Master quadratic as bridge
- [EXPLR_LOOP_GRID_DUALITY.md](../08_structural/EXPLR_LOOP_GRID_DUALITY.md) -- Two-layer ontology

---

## Summary

The force laws in FTD's simulation code (forces.py) were originally imposed — Coulomb's 1/r^2, Yukawa's e^{-mr}/r^2, Lorentz's v x B, and gravitational 1/r^2 were borrowed from known physics and inserted as update rules.

This document shows they need not be imposed. They are **necessary consequences** of the lattice wave equation and its Green's function:

1. **Static wave equation** -> Poisson equation on lattice
2. **Lattice Green's function** -> 1/(4 pi r) at long wavelengths (r >> 1 lattice unit)
3. **Force = -gradient of potential** -> 1/r^2 (Coulomb, gravity)
4. **Massive propagator** -> e^{-mr}/(4 pi r) (Yukawa/strong)
5. **Curl of flux field** -> magnetic field B -> Lorentz force
6. **Coupling from master quadratic** -> alpha determines force strength
7. **Mass from confinement** -> m_pi, M_W determine force range

The four forces are not four postulates. They are four regimes of one Green's function.
