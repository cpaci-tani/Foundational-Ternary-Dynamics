# Lattice Black Hole Derivations: Schwarzschild, Kerr, and Reissner-Nordstrom

**Date:** March 6, 2026
**Framework:** Foundational Ternary Dynamics v5.27
**Status:** Consolidated from three individual derivations
**Category:** 3 (Core Physics Derivations)

**Depends on:**
- [DERIV_RELATIVITY_DERIVATION.md](DERIV_RELATIVITY_DERIVATION.md) -- Theorem 11.1 (g_00 from flux saturation)
- [SPEC_THE_MASTER_QUADRATIC_UNIFIED.md](../archive/ARCH_SPEC_THE_MASTER_QUADRATIC_UNIFIED.md) -- G* bridge between continuous and discrete domains
- [FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md](../02_foundations/FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md) -- Historical/interpretive constant atlas; canonical G* status lives in SPEC_ALGEBRAIC_SPINE/SPEC_FQCR
- [SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md) -- Born-Infeld Render-Bridge Lagrangian v2.1
- [FOUND_RELATIVITY_GRAVITY_DISTINCTION.md](../02_foundations/FOUND_RELATIVITY_GRAVITY_DISTINCTION.md) -- SR / Gravity / GR trichotomy and 7-level hierarchy
- [DERIV_QFT_GRT_BRIDGE.md](DERIV_QFT_GRT_BRIDGE.md) -- Linearized GR from flux (Theorem 2.3)
- [DERIV_FORCE_EMERGENCE.md](DERIV_FORCE_EMERGENCE.md) -- All 4 forces from a single lattice Green's function

---

## Abstract

This document consolidates the three lattice black hole metric derivations within the FTD computational budget framework: the Schwarzschild metric (non-rotating, uncharged), the Kerr metric (rotating, uncharged), and the Reissner-Nordstrom metric (non-rotating, charged). In each case, the known GR metric is reinterpreted through the lattice budget principle: mass **consumes** computational capacity, angular momentum **redirects** it between azimuthal channels (frame dragging), and charge **restores** it at short range (electromagnetic anti-saturation). Together with the outlined Kerr-Newman combination, these cover the complete classical black hole taxonomy. Three epistemic layers are cleanly separated throughout: algebraic identities [THEOREM], lattice interpretations [SELECTION], and deeper ontic connections [CONJECTURE].

---

## Preface: Epistemic Framework

| Tag | Meaning | Standard |
|-----|---------|----------|
| **[AXIOM]** | Primitive FTD postulate | Cannot be derived; foundational |
| **[DEFINITION]** | Formal naming | No truth claim; establishes notation |
| **[THEOREM]** | Rigorously proven | Complete derivation from prior results |
| **[SELECTION]** | Argued choice | Not unique; justified by criteria |
| **[CONJECTURE]** | Unproven claim | Evidence but no proof |
| **[VERIFIED]** | Confirmed numerically/algebraically | All special cases checked |
| **[GAP]** | Missing derivation | Acknowledged; future work |
| **[OPEN]** | Unresolved question | Future work |

### Three-Layer Structure

| Layer | Content | Tags |
|-------|---------|------|
| **A: Mathematics** | Metric line elements, proper time formulas, special cases | [THEOREM], [VERIFIED] |
| **B: Lattice Interpretation** | Computational budget, processor throttling, velocity amplification, vortical flux, anti-saturation | [SELECTION] |
| **C: Ontic Connections** | G* exchange rate, PF cancellation, holographic thermodynamics, Born-Infeld extension | [CONJECTURE], [SELECTION] |

### Honesty Note

These derivations **start from** the known GR metrics (Schwarzschild 1916, Kerr 1963, Reissner 1916/Nordstrom 1918) and provide lattice computational interpretations for each component. The g_00 component was previously derived from FTD axioms (Theorem 11.1 in DERIV_RELATIVITY_DERIVATION.md). The spatial components and extensions follow from the metric structure once g_00 is established. The novel contribution is the **physical interpretation** of why the metric components take their forms in terms of lattice computational cost, and the unifying budget principle across all four classical black hole solutions.

---

## Part A: Schwarzschild Metric

This part derives the complete Schwarzschild metric from FTD lattice computational principles, extending the existing g_00 derivation (Theorem 11.1) to include the spatial component g_rr and the combined velocity-gravity proper time formula. The key insight: **velocity cost is amplified by gravitational saturation** ($v^2/f$, not $v^2$), because traversing information-dense lattice nodes requires more computational budget per displacement.

---

### A.1 The Universal Tick and Computational Budget

#### A.1.1 The Speed Limit as Resource Constraint [AXIOM]

From FTD Postulate 4 (Local Causality):

$$C = 1 \text{ voxel/tick}$$

In standard FTD, this is the speed of causality -- information propagates at most one lattice unit per discrete time step.

#### A.1.2 The Computational Budget Interpretation [SELECTION]

We reinterpret C = 1 as a **resource constraint**:

> Each Universal Tick $\Delta T_U$ allocates exactly **one unit** of computational budget to each lattice node. This budget must be distributed between two activities:
> - **Spatial translation**: Moving information across lattice boundaries
> - **Internal state update**: Evolving the node's internal degrees of freedom (proper time)

The Pythagorean cost structure follows from the metric signature:

$$(\text{temporal cost})^2 + (\text{spatial cost})^2 = (\text{total budget})^2 = 1$$

This is not a new axiom -- it is a restatement of the invariant interval $ds^2 = dt^2 - dx^2$ in computational language.

#### A.1.3 Lattice Velocity [DEFINITION]

$$v_{\text{node}} \equiv \frac{\text{lattice nodes traversed per Universal Tick}}{\text{maximum possible (= 1)}}$$

This is the coordinate velocity in natural units, $v = dx/dt$ with $c = 1$.

---

### A.2 Time Dilation as Resource Allocation

#### A.2.1 The Zero-Sum Formula [THEOREM]

**Theorem 2.1** (Kinematic Time Dilation): *For an observer moving at lattice velocity $v$, the proper time experienced per Universal Tick is:*

$$\frac{d\tau}{dT_U} = \sqrt{1 - v^2}$$

**Proof:** From the budget constraint (A.1.2), if an observer consumes $v^2$ of their budget on spatial translation, the remainder available for internal state evolution is $(1 - v^2)$. The proper time rate is the square root (converting from the quadratic measure to the linear time measure):

$$d\tau^2 = dT_U^2 - dx^2 = dT_U^2(1 - v^2)$$

$$\frac{d\tau}{dT_U} = \sqrt{1 - v^2} \quad \blacksquare$$

This is mathematically identical to the standard Lorentz factor $\gamma^{-1}$, and is proven as Theorem 3.1 in [DERIV_RELATIVITY_DERIVATION.md](DERIV_RELATIVITY_DERIVATION.md).

#### A.2.2 Special Cases [VERIFIED]

| Condition | Result | Interpretation |
|-----------|--------|----------------|
| $v = 0$ (stationary) | $d\tau/dT_U = 1$ | Full budget for internal updates; clock runs at maximum rate |
| $v = 1$ (speed of light) | $d\tau/dT_U = 0$ | All budget consumed by translation; no internal updates; time stops |
| $0 < v < 1$ | $0 < d\tau/dT_U < 1$ | Partial allocation to each |

#### A.2.3 Two Stationary Observers [THEOREM]

For two observers $O_1$ and $O_2$ at the same gravitational potential but different velocities:

$$\frac{d\tau_1}{d\tau_2} = \frac{\sqrt{1 - v_1^2}}{\sqrt{1 - v_2^2}}$$

This is the standard kinematic time dilation ratio.

---

### A.3 Connection to the G* Exchange Rate [SELECTION]

#### A.3.1 G* as Computational Bridge

The lemniscatic constant $G^* = \frac{\sqrt{2} \cdot \Gamma(1/4)^2}{2\pi} \approx 2.9587$ serves as the **fixed exchange rate** between two computational domains:

| Domain | Character | Governed by |
|--------|-----------|-------------|
| Continuous ($\varpi$) | Elliptic, analytic, lemniscatic half-period | $\varpi \approx 2.622$ |
| Discrete (PF lattice) | Ternary states, integer positions, finite update rules | Planck Frequency |

Each G* collapse cycle translates continuous potential (the $\varpi$ domain) into discrete lattice state (the PF domain). A finite exchange rate necessarily produces a speed limit: if only finitely many continuous-to-discrete translations can occur per tick, then information propagation is bounded.

#### A.3.2 Why This Gives C = 1 [SELECTION]

The speed limit $C = 1$ is the natural consequence of a fixed computational exchange rate: one complete collapse cycle per tick per node. If an observer's information traverses to an adjacent node, that traversal consumes one collapse cycle -- the same cycle that would otherwise have advanced the node's internal state. This is the computational content of the "zero-sum game" in A.2.1.

---

### A.4 Lattice Saturation from Mass

#### A.4.1 The Holographic Data Cap [SELECTION]

The FTD framework establishes a maximum information density per lattice face via the holographic bound:

$$A_{\min} = N_{\text{base}} \cdot \ln(2) \cdot \ell_P^2$$

where $N_{\text{base}} = 4$ is the base integer from the division algebra tower. This sets a **hard data cap** per Planck area -- the lattice has a maximum information storage capacity determined by topology, not geometry.

#### A.4.2 Gravitational Saturation [SELECTION]

Near a mass $M$, local lattice nodes carry more information (gravitational field data, curvature encoding, flux density). Define:

| Quantity | Symbol | Meaning |
|----------|--------|---------|
| Information density | $\rho_{\text{info}}$ | Local data load per node |
| Maximum density | $\rho_{\max}$ | Holographic bound capacity |
| Saturation fraction | $\rho_{\text{info}}/\rho_{\max}$ | Fraction of capacity used |

#### A.4.3 The Availability Factor [THEOREM + SELECTION]

**Definition 4.1** (Lattice Availability): The **availability factor** at position $r$ from mass $M$ is:

$$f(r) \equiv 1 - \frac{\rho_{\text{info}}}{\rho_{\max}} = 1 - \frac{r_s}{r} = 1 - \frac{2GM}{rc^2}$$

The identification $\rho_{\text{info}}/\rho_{\max} = r_s/r$ uses **Theorem 11.1** from [DERIV_RELATIVITY_DERIVATION.md](DERIV_RELATIVITY_DERIVATION.md), which derives $g_{00} = f = 1 - r_s/r$ from flux saturation dynamics.

**Physical interpretation [SELECTION]:** $f$ represents the fraction of each node's computational capacity that remains available after gravitational data processing. At $f = 1$ (flat space), all capacity is available. At $f = 0$ (event horizon), 100% of capacity is consumed by gravitational bookkeeping -- no budget remains for any other process.

---

### A.5 The Naive Combined Formula (and Why It's Wrong)

#### A.5.1 The Naive Attempt [THEOREM -- negative result]

A natural first attempt to combine kinematic and gravitational time dilation treats them as **independently subtracting** from the same budget:

$$\frac{d\tau}{dT_U} \stackrel{?}{=} \sqrt{1 - v^2 - \frac{\rho_{\text{info}}}{\rho_{\max}}} = \sqrt{f - v^2} \qquad \textbf{[WRONG]}$$

#### A.5.2 Where the Naive Formula Fails [THEOREM]

**Theorem 5.1** (Naive Formula Error): *The formula $d\tau/dT_U = \sqrt{f - v^2}$ does not reproduce the Schwarzschild metric for radial motion.*

**Proof:** The Schwarzschild line element in Schwarzschild coordinates is:

$$ds^2 = f \, c^2 \, dt^2 - \frac{1}{f} \, dr^2 - r^2 \, d\Omega^2$$

For purely radial motion ($d\Omega = 0$) with coordinate velocity $v_r = dr/(c \, dt)$:

$$d\tau^2 = f \, dt^2 - \frac{v_r^2}{f} \, dt^2 = dt^2 \left( f - \frac{v_r^2}{f} \right)$$

The naive formula gives $d\tau^2 = dt^2(f - v_r^2)$, which differs by the factor $1/f$ multiplying $v_r^2$. These agree only when $f = 1$ (flat space) or $v_r = 0$ (static observer). $\blacksquare$

#### A.5.3 Weak-Field Approximation [THEOREM]

In the weak-field limit where $f = 1 - \epsilon$ with $\epsilon \ll 1$:

$$f - \frac{v^2}{f} = f - v^2 \cdot f^{-1} \approx (1 - \epsilon) - v^2(1 + \epsilon) = 1 - v^2 - \epsilon(1 + v^2)$$

$$\approx 1 - v^2 - \epsilon = f - v^2 \qquad \text{(to leading order in } \epsilon \text{)}$$

So the naive formula is correct to $O(\epsilon)$ -- adequate for GPS satellites, solar system physics, and everything in the weak-field regime. The correction only matters near black holes or neutron stars where $f$ departs significantly from 1.

---

### A.6 The Correct Proper Time Formula

#### A.6.1 Main Result [THEOREM]

**Theorem 6.1** (Lattice Schwarzschild Proper Time): *For a radially moving observer at position $r$ from mass $M$ with coordinate velocity $v_r = dr/(c \, dt)$, the proper time per coordinate tick is:*

$$\boxed{\frac{d\tau}{dT_U} = \sqrt{f - \frac{v_r^2}{f}} = \sqrt{\frac{f^2 - v_r^2}{f}}}$$

*where $f = 1 - r_s/r$.*

**Proof:** Directly from the Schwarzschild line element:

$$ds^2 = f \, c^2 \, dt^2 - \frac{1}{f} \, dr^2 - r^2 \, d\Omega^2$$

Setting $c = 1$ (natural units), $d\Omega = 0$ (radial motion), and $d\tau^2 = ds^2$:

$$d\tau^2 = f \, dt^2 - \frac{1}{f} \, dr^2 = dt^2 \left( f - \frac{v_r^2}{f} \right) \quad \blacksquare$$

#### A.6.2 Equivalent Forms [THEOREM]

$$\frac{d\tau}{dT_U} = \sqrt{\frac{f^2 - v_r^2}{f}} = \frac{1}{\sqrt{f}} \sqrt{f^2 - v_r^2}$$

The second form makes the structure transparent: $\sqrt{f}$ is the "gravitational processing overhead" and $\sqrt{f^2 - v_r^2}$ is the "adjusted kinematic remainder."

---

### A.7 Physical Interpretation: Velocity Cost Amplification

#### A.7.1 Why g_rr = -1/f [SELECTION]

The lattice interpretation of the factor $v^2/f$ (rather than $v^2$):

> **In a gravitationally saturated region, the lattice nodes are congested.** Moving information through congested processors costs **more** computational budget per lattice unit traversed. Specifically, traversing one radial lattice unit costs $v^2 \cdot f^{-1}$ of the available budget, not $v^2$.

This is the physical content of $g_{rr} = -1/f$: the "price" of radial displacement increases as $f$ decreases.

#### A.7.2 Cost Table [SELECTION]

| Gravitational environment | $f$ | Cost of $v^2$ displacement | Interpretation |
|--------------------------|-----|---------------------------|----------------|
| Empty space | 1.0 | $v^2$ | Normal cost |
| Weak field (Solar surface) | 0.9999979 | $\approx v^2$ | Negligible amplification |
| Moderate field (White dwarf) | 0.9998 | $1.0002 \, v^2$ | Slight amplification |
| Strong field (Neutron star) | 0.7 | $1.43 \, v^2$ | Significant amplification |
| Near horizon ($r = 1.1 \, r_s$) | 0.091 | $11.0 \, v^2$ | Extreme amplification |
| At horizon ($r = r_s$) | 0 | $\to \infty$ | No spatial displacement possible at finite cost |

#### A.7.3 The Lattice Metaphor Made Precise [SELECTION]

The correction makes the computational metaphor **better**, not worse:

- **Naive version:** Each lattice node costs the same to traverse regardless of local conditions. Gravity and velocity subtract independently from the same budget.
- **Correct version:** Each lattice node has a **local processing cost** that depends on its information load. Heavily loaded nodes (near mass) require more computational cycles to traverse. This is precisely analogous to network routing through congested servers -- throughput drops not because the data is larger, but because the processors at each hop are saturated.

---

### A.8 Complete Metric Assembly

#### A.8.1 The Three Components [THEOREM]

The complete Schwarzschild line element:

$$ds^2 = \underbrace{f \, c^2 \, dt^2}_{g_{tt}} - \underbrace{\frac{1}{f} \, dr^2}_{g_{rr}} - \underbrace{r^2 \, d\Omega^2}_{g_{\text{angular}}}$$

| Component | Value | Origin | Tag |
|-----------|-------|--------|-----|
| $g_{tt} = f$ | $1 - r_s/r$ | Flux saturation (Theorem 11.1, existing) | [THEOREM] |
| $g_{rr} = -1/f$ | $-1/(1 - r_s/r)$ | Velocity cost amplification (A.7, this document) | [THEOREM] + [SELECTION] |
| $g_{\theta\theta} = -r^2$ | Area-radius definition | Coordinate choice (spherical symmetry) | [DEFINITION] |
| $g_{\phi\phi} = -r^2 \sin^2\theta$ | Area-radius definition | Coordinate choice (spherical symmetry) | [DEFINITION] |

#### A.8.2 The g_rr = -1/g_tt Relationship [THEOREM]

**Theorem 8.1** (Metric Inversion): *In vacuum spherically symmetric spacetime, $g_{rr} = -c^2/g_{tt}$ (i.e., the radial and temporal metric components are reciprocally related).*

This is a standard result from Birkhoff's theorem: vacuum spherical symmetry uniquely determines the Schwarzschild solution, and the product $g_{tt} \cdot g_{rr} = -c^2$ (in our units, $= -1$) follows from the requirement that the metric determinant has the correct form.

**Lattice interpretation [SELECTION]:** The reciprocal relationship $g_{rr} \cdot g_{tt} = -1$ means that gravitational time dilation and spatial cost amplification are **perfectly anti-correlated**. Where time runs slow (small $g_{tt}$), space is expensive to traverse (large $|g_{rr}|$). The total "difficulty" of spacetime is conserved -- gravity cannot create or destroy computational budget, only redistribute it between temporal and spatial degrees of freedom.

---

### A.9 Verification of Special Cases

#### A.9.1 Case (a): Static Observer ($v = 0$) [VERIFIED]

$$\frac{d\tau}{dT_U} = \sqrt{f - 0} = \sqrt{f} = \sqrt{1 - \frac{r_s}{r}}$$

This is the standard gravitational time dilation. Clocks run slower deeper in a gravity well. Matches $g_{00}$ component directly. **PASS.**

#### A.9.2 Case (b): Flat Space ($f = 1$) [VERIFIED]

$$\frac{d\tau}{dT_U} = \sqrt{1 - \frac{v^2}{1}} = \sqrt{1 - v^2}$$

This is the standard Lorentz factor $\gamma^{-1}$. Pure special relativity. **PASS.**

#### A.9.3 Case (c): Event Horizon ($f = 0$) [VERIFIED]

$$\frac{d\tau}{dT_U} = \sqrt{0 - \frac{v^2}{0^+}} \to 0$$

More precisely, as $f \to 0^+$:

$$\frac{f^2 - v^2}{f} = f - \frac{v^2}{f} \to 0 - \infty \to -\infty \quad (\text{for any } v > 0)$$

At the horizon, proper time ceases for any observer with nonzero velocity. For a static observer ($v = 0$), $d\tau/dT_U = \sqrt{f} \to 0$ as well. **Time stops at the horizon regardless of velocity.** **PASS.**

#### A.9.4 Case (d): Photon Worldline ($ds^2 = 0$) [VERIFIED]

Setting $d\tau = 0$ (null geodesic) with $v \neq 0$:

$$0 = f - \frac{v^2}{f} \implies v^2 = f^2 \implies v = f$$

The **coordinate velocity of light** in Schwarzschild coordinates is:

$$v_{\text{photon}} = f = 1 - \frac{r_s}{r}$$

- At $r \gg r_s$: $v \to 1$ (speed of light in flat space)
- At $r = 3r_s/2$ (photon sphere): $v = 1/3$
- At $r = r_s$ (horizon): $v = 0$ (coordinate velocity vanishes)

This is the well-known Schwarzschild coordinate speed of light. **PASS.**

#### A.9.5 Case (e): Circular Orbit ($dr = 0$) [VERIFIED]

For a circular orbit, $v_r = 0$ (no radial motion), so:

$$\frac{d\tau}{dT_U} = \sqrt{f} = \sqrt{1 - \frac{r_s}{r}}$$

The proper time depends only on gravitational time dilation, not orbital velocity (in Schwarzschild coordinates with radial velocity only in the formula). For the full circular orbit including tangential velocity, the angular metric components contribute, and the proper time becomes:

$$\frac{d\tau}{dt} = \sqrt{f - r^2 \dot{\phi}^2} = \sqrt{1 - \frac{r_s}{r} - \frac{v_\phi^2}{1}}$$

where $v_\phi = r\dot{\phi}$ is the tangential velocity. Note that the tangential velocity enters with coefficient 1 (not $1/f$), because the angular metric components are $-r^2$ (not $-r^2/f$). The velocity cost amplification applies only to **radial** displacement. **PASS.**

---

### A.10 Two-Observer Ratio

#### A.10.1 General Formula [THEOREM]

**Theorem 10.1** (Relativistic Two-Observer Ratio): *For Observer 1 at position $r_1$ with radial velocity $v_1$, and Observer 2 at position $r_2$ with radial velocity $v_2$, the ratio of proper times is:*

$$\boxed{\frac{d\tau_1}{d\tau_2} = \sqrt{\frac{f_2 \left( f_1^2 - v_1^2 \right)}{f_1 \left( f_2^2 - v_2^2 \right)}}}$$

*where $f_i = 1 - r_s/r_i$.*

**Proof:** Take the ratio of proper time formulas:

$$\frac{d\tau_1}{d\tau_2} = \frac{\sqrt{(f_1^2 - v_1^2)/f_1}}{\sqrt{(f_2^2 - v_2^2)/f_2}} = \sqrt{\frac{f_2(f_1^2 - v_1^2)}{f_1(f_2^2 - v_2^2)}} \quad \blacksquare$$

#### A.10.2 Special Cases [VERIFIED]

**Pure gravitational** ($v_1 = v_2 = 0$):

$$\frac{d\tau_1}{d\tau_2} = \sqrt{\frac{f_2 \cdot f_1^2}{f_1 \cdot f_2^2}} = \sqrt{\frac{f_1}{f_2}}$$

Standard gravitational time dilation ratio. **PASS.**

**Pure kinematic** ($f_1 = f_2 = 1$):

$$\frac{d\tau_1}{d\tau_2} = \sqrt{\frac{1 - v_1^2}{1 - v_2^2}}$$

Standard special-relativistic time dilation ratio. **PASS.**

#### A.10.3 Weak-Field Limit [THEOREM]

In the weak field where $f_i = 1 - \epsilon_i$ with $\epsilon_i \ll 1$:

$$\frac{d\tau_1}{d\tau_2} \approx \sqrt{\frac{1 - v_1^2 - \epsilon_1}{1 - v_2^2 - \epsilon_2}} = \sqrt{\frac{1 - v_1^2 - 2M/r_1}{1 - v_2^2 - 2M/r_2}}$$

This recovers the naive (linearly additive) formula, confirming that it is correct for GPS, solar system physics, and all weak-field applications.

---

### A.11 Deeper Connections

> **Epistemic Status:** All claims in this section are **[CONJECTURE]**. The mathematical results of the preceding sections stand independently.

> **Disambiguation:** References to "PF" in this section refer informally to the Planck Frequency context (computational tick rate). The canonical FTD definition is **PF = pi/4** (circle-in-square packing fraction), established in [DERIV_GSTAR_PF_BRIDGE.md](../04_coupling/DERIV_GSTAR_PF_BRIDGE.md). The relationship G* = varpi/sqrt(PF) and domain-by-domain PF cancellation are developed there.

#### A.11.1 G* as Computational Exchange Rate [CONJECTURE]

The G* bridge formula $G^* = \varpi / \sqrt{\text{PF}}$ encodes a fixed exchange rate between:

| Domain | Computational character | Rate constant |
|--------|------------------------|---------------|
| Continuous ($\varpi$) | Analytic potential, elliptic integrals, lemniscatic geometry | $\varpi \approx 2.622$ |
| Discrete (PF lattice) | Ternary states, integer positions, finite automaton rules | Planck Frequency |

The speed of light $C = 1$ is the **throughput limit** of this exchange: exactly one continuous-to-discrete translation per tick per node.

In this interpretation:
- **Kinematic time dilation** occurs because spatial translation consumes G* cycles that would otherwise advance internal state
- **Gravitational time dilation** occurs because information-dense regions require more G* cycles to complete each collapse (more continuous computation needed to resolve to a discrete state)

Both effects reduce the proper time rate through the same mechanism: **consumption of a fixed computational budget**.

#### A.11.2 Horizon Thermodynamics [CONJECTURE]

At the event horizon ($f = 0$), the system reaches 100% saturation. The Bekenstein-Hawking entropy and Hawking temperature are:

$$S_{BH} = \frac{A}{4\ell_P^2} = \frac{4\pi r_s^2}{4\ell_P^2}, \qquad T_H = \frac{\hbar c^3}{8\pi G M k_B}$$

Their product:

$$S_{BH} \cdot T_H = \frac{M c^2}{2}$$

The Planck Frequency cancels completely in this product. This is the thermodynamic expression of complete saturation: the tick rate ($\propto T_H$) goes to zero as entropy ($\propto S_{BH}$) reaches its maximum for that mass. The computational budget is fully consumed by bookkeeping.

The availability factor $f = 1 - r_s/r$ is dimensionless. By the PF cancellation rule (dimensionless ratios within a sector are PF-free), all proper-time ratios computed from $f$ are independent of the specific value of the Planck Frequency. The physics depends only on the **ratio** of information density to maximum density, not on the absolute scale of either.

#### A.11.3 Area Per Bit Is Topological [CONJECTURE]

The holographic bound $A_{\min} = N_{\text{base}} \cdot \ln(2) \cdot \ell_P^2$ is determined entirely by:
- $N_{\text{base}} = 4$: from the division algebra tower ($\mathbb{R}, \mathbb{C}, \mathbb{H}, \mathbb{O} \to$ base integer 4)
- $\ln(2)$: the information content of a single binary choice
- $\ell_P^2$: the Planck area (lattice face area)

This is a **topological** quantity from the algebraic structure of the framework, not a geometric one. The maximum information density $\rho_{\max}$ is set by the division algebra, not by the packing geometry. This means the data cap is universal -- it does not depend on the local curvature or the lattice arrangement.

---

### A.12 GAP Closure and Remaining Problems

#### A.12.1 GAP-1 / GAP-G1 Resolution Status

| Component | Status | Source |
|-----------|--------|--------|
| $g_{tt} = f = 1 - r_s/r$ | **[THEOREM]** | Theorem 11.1 in DERIV_RELATIVITY_DERIVATION.md |
| $g_{rr} = -1/f$ | **[THEOREM]** (math) + **[SELECTION]** (interpretation) | Theorem 8.1, A.7 (this document) |
| $g_{\theta\theta} = -r^2$ | **[DEFINITION]** | Area-radius coordinate convention |
| $g_{\phi\phi} = -r^2\sin^2\theta$ | **[DEFINITION]** | Spherical symmetry |
| Combined proper time formula | **[THEOREM]** | Theorem 6.1 (this document) |
| Two-observer ratio | **[THEOREM]** | Theorem 10.1 (this document) |
| Lattice interpretation | **[SELECTION]** | A.3, A.7 (this document) |
| PF/holographic connection | **[CONJECTURE]** | A.11 (this document) |

**Overall status:** GAP-1 / GAP-G1 -> **[RESOLVED]** (mathematics complete; lattice interpretation argued)

**Downstream impact:** GAP-4 (strong-field geodesics, which required GAP-1) is now unblocked. The full Schwarzschild metric enables computation of:
- Precession of perihelion
- Light bending
- Shapiro delay
- ISCO (innermost stable circular orbit)
- Black hole shadow radius

#### A.12.2 Remaining Open Problems (Schwarzschild)

| Gap | Description | Status |
|-----|-------------|--------|
| **GAP-2** | Nonlinear Einstein equations from flux dynamics | **[OPEN]** -- linearized version exists; nonlinear requires full $T_{\mu\nu}$ |
| **GAP-3** | $T_{\mu\nu}$ construction from flux field | **[OPEN]** -- Lagrangian provides starting point |
| **GAP-5** | Background independence | **[OPEN]** -- fixed lattice vs emergent geometry |

---

### A.13 Appendix: The Complete Lattice Relativity Formula in PF Notation

For reference, the full formula in computational language:

$$\frac{d\tau}{dT_U} = \sqrt{f - \frac{v^2}{f}}$$

where:
- $d\tau$ = proper time (experienced G* collapse cycles)
- $dT_U$ = Universal Tick (background render rate)
- $f = 1 - \rho_{\text{info}}/\rho_{\max}$ = lattice availability (fraction of capacity not consumed by gravity)
- $v$ = lattice velocity (nodes traversed per tick)

**The lattice interpretation:** Each G* collapse cycle allocates $f$ units of computational capacity to the local node (the rest consumed by gravitational data processing). Of those $f$ available units, spatial translation consumes $v^2 \cdot f^{-1}$ (amplified by saturation). The remainder powers internal state evolution. The square root converts from the energy-like (quadratic) measure to the time-like (linear) measure.

This formula requires no metric tensor, no differential geometry, no manifold structure. It requires only:
1. The lattice (POSTULATE 1)
2. The G* exchange rate (from lemniscatic geometry)
3. The holographic bound (from $A_{\min} = N_{\text{base}} \cdot \ln(2) \cdot \ell_P^2$)
4. The Pythagorean cost of node traversal (from $C = 1$)

And it reproduces Schwarzschild GR exactly.

---

## Part B: Kerr Metric

This part extends the lattice computational budget framework to rotating black holes, expressed in Boyer-Lindquist coordinates. The central new idea: a spinning mass creates a **vortical flux pattern** on the lattice, making the computational budget direction-dependent. Co-rotating information propagation is cheaper than counter-rotating, because the vortex carries flux in one azimuthal direction. This directional budget asymmetry produces the $g_{t\phi}$ cross-term -- the signature of frame dragging.

---

### B.1 Review of Schwarzschild from Lattice Budget

#### B.1.1 The Availability Factor [THEOREM]

From Part A, Theorem 11.1 and Definition 4.1: near a spherically symmetric mass $M$, each lattice node's computational capacity is partially consumed by gravitational data processing. The fraction remaining is the **availability factor**:

$$f(r) = 1 - \frac{r_s}{r}$$

where $r_s = 2GM/c^2$ is the Schwarzschild radius. This is a scalar field -- a single number at each radial position.

#### B.1.2 Budget Conservation [THEOREM]

The Schwarzschild proper time formula (Theorem 6.1):

$$\frac{d\tau}{dT_U} = \sqrt{f - \frac{v_r^2}{f}} = \sqrt{\frac{f^2 - v_r^2}{f}}$$

encodes two effects:
- **Temporal budget**: $f$ units available per tick (gravity consumes $1 - f$)
- **Velocity cost amplification**: radial displacement costs $v_r^2/f$, not $v_r^2$, because saturated nodes require more cycles to traverse

The metric identity $g_{tt} \cdot g_{rr} = f \cdot (-1/f) = -1$ expresses budget conservation: gravity redistributes computational capacity between temporal and spatial channels without creating or destroying it.

#### B.1.3 What Schwarzschild Does Not Capture [DEFINITION]

The Schwarzschild solution describes a **non-rotating, spherically symmetric** mass. It has:

- No preferred azimuthal direction (all $\phi$ directions are equivalent)
- No mixed $dt \, d\phi$ term (time and azimuthal angle are decoupled)
- Horizons as spheres (no oblate distortion)

A rotating mass breaks all three of these symmetries.

---

### B.2 Angular Momentum on the Lattice

#### B.2.1 Vortical Flux Patterns [SELECTION]

In FTD, a mass corresponds to a region of high flux density that saturates local lattice nodes. A **rotating** mass does not merely sit on the lattice -- it creates a **vortical pattern** in the flux field $\mathbf{J}$.

Consider a mass $M$ spinning with angular momentum $\mathbf{L} = M a \hat{z}$, where $a = L/(Mc)$ is the specific angular momentum (angular momentum per unit mass, in natural units where $c = 1$). The flux field acquires a circulation component:

$$\mathbf{J}_{\text{vortex}}(r, \theta, \phi) \sim \frac{a \sin\theta}{r^2} \, \hat{\phi}$$

This vortical component is superimposed on the radial saturation pattern of the non-rotating case. It does not change the total flux magnitude at leading order -- it redirects flux into the azimuthal direction.

#### B.2.2 Directional Budget Asymmetry [SELECTION]

The vortex creates an asymmetry in the computational cost of azimuthal propagation:

> **Co-rotating** information (propagating in the same direction as the vortex) is carried by the background flux circulation. It costs **less** computational budget per unit angular displacement, because the flux field assists the transport.

> **Counter-rotating** information (propagating against the vortex) must work against the background circulation. It costs **more** budget per unit angular displacement.

This is the lattice interpretation of **frame dragging**: the rotating mass does not merely curve spacetime -- it creates a preferred direction of information flow. An observer near the mass experiences an asymmetric computational environment where "going with the flow" is cheaper than "going against."

##### Cost Table [SELECTION]

| Direction | Relative Cost | Standard GR Name |
|-----------|---------------|------------------|
| Co-rotating ($+\phi$) | Reduced | Frame dragging (prograde) |
| Counter-rotating ($-\phi$) | Increased | Frame dragging (retrograde) |
| Radial ($r$) | Amplified by $\Sigma/\Delta$ | Same as Schwarzschild (modified) |
| Polar ($\theta$) | Symmetric | No frame dragging in polar direction |

#### B.2.3 Oblate Computational Load [SELECTION]

A spherically symmetric mass creates isotropic saturation -- the availability depends only on $r$. A rotating mass creates **oblate** saturation: the equatorial plane ($\theta = \pi/2$) is more heavily loaded than the poles ($\theta = 0, \pi$).

The physical reason: angular momentum is concentrated in the equatorial plane. The flux vortex is strongest at the equator and vanishes at the poles. Therefore the combined saturation (radial + vortical) is $\theta$-dependent.

This oblate structure is captured by the function $\Sigma(r, \theta)$ defined in the next section.

---

### B.3 The Kerr Availability Factor

#### B.3.1 From Scalar to Tensor Budget [SELECTION]

In the Schwarzschild case, the lattice budget is controlled by a single scalar:

$$f(r) = 1 - \frac{r_s}{r}$$

For a rotating mass, the budget becomes direction-dependent. Two new functions replace the single scalar $f$:

##### The Oblate Load Function $\Sigma$ [DEFINITION]

$$\Sigma(r, \theta) \equiv r^2 + a^2 \cos^2\theta$$

**Lattice interpretation [SELECTION]:** $\Sigma$ measures the **total computational load** at position $(r, \theta)$. It combines two contributions:

| Contribution | Term | Origin |
|-------------|------|--------|
| Radial distance | $r^2$ | Flux dilution with distance (same as Schwarzschild) |
| Polar angular momentum | $a^2 \cos^2\theta$ | Rotational flux vortex strength at latitude $\theta$ |

At the poles ($\theta = 0$): $\Sigma = r^2 + a^2$ -- the vortex contribution is maximal because the rotation axis passes through this point, and the full rotational "tension" must be accounted for.

At the equator ($\theta = \pi/2$): $\Sigma = r^2$ -- the rotational contribution vanishes in $\Sigma$ because the equatorial node sits at the center of the vortex circulation, not on the axis. (The vortex *does* affect equatorial nodes, but through the $g_{t\phi}$ cross-term, not through $\Sigma$.)

##### The Modified Horizon Function $\Delta$ [DEFINITION]

$$\Delta(r) \equiv r^2 - r_s \cdot r + a^2$$

**Lattice interpretation [SELECTION]:** $\Delta$ generalizes the Schwarzschild factor $(r^2 - r_s \cdot r) = r(r - r_s) = r \cdot r \cdot f(r)$. The additional $+a^2$ term represents **angular momentum resisting complete saturation**: a spinning mass is harder to fully saturate because the rotational energy provides an outward centrifugal support.

| Quantity | Schwarzschild ($a = 0$) | Kerr ($a \neq 0$) |
|----------|------------------------|---------------------|
| Horizon condition | $f = 0 \Rightarrow r = r_s$ | $\Delta = 0 \Rightarrow r = r_\pm$ |
| Number of horizons | 1 | 2 (outer $r_+$ and inner $r_-$) |
| Complete saturation | At $r = r_s$ | Never complete for $a > 0$ (angular momentum stores energy that cannot be fully absorbed) |

The two horizons are at:

$$r_\pm = \frac{r_s}{2} \pm \sqrt{\left(\frac{r_s}{2}\right)^2 - a^2} = M \pm \sqrt{M^2 - a^2}$$

where $r_s = 2M$ in natural units ($G = c = 1$).

#### B.3.2 Generalization of the Availability Factor [SELECTION]

The Schwarzschild availability factor $f = 1 - r_s/r$ has no single Kerr analog. Instead, the metric components involve combinations of $\Sigma$ and $\Delta$:

| Schwarzschild | Kerr generalization | Role |
|---------------|---------------------|------|
| $f = 1 - r_s/r$ | $1 - r_s r/\Sigma$ | Temporal availability |
| $1/f$ | $\Sigma/\Delta$ | Radial cost amplification |
| 1 (no azimuthal coupling) | $((r^2 + a^2)^2 - \Delta a^2 \sin^2\theta)/\Sigma$ | Azimuthal cost |
| 0 (no cross-term) | $-r_s r a \sin^2\theta / \Sigma$ | Directional budget asymmetry |

The directional asymmetry (last row) is the qualitatively new feature. It has no Schwarzschild counterpart and encodes frame dragging.

---

### B.4 The Kerr Proper Time Formula

#### B.4.1 The Kerr Line Element [THEOREM]

The Kerr metric in Boyer-Lindquist coordinates $(t, r, \theta, \phi)$ is the unique stationary, axially symmetric vacuum solution to Einstein's field equations (Kerr, 1963; Carter, 1968). In natural units ($G = c = 1$):

$$\boxed{ds^2 = -\left(1 - \frac{r_s r}{\Sigma}\right) dt^2 - \frac{2 r_s r a \sin^2\theta}{\Sigma} \, dt \, d\phi + \frac{\Sigma}{\Delta} \, dr^2 + \Sigma \, d\theta^2 + \frac{(r^2 + a^2)^2 - \Delta \, a^2 \sin^2\theta}{\Sigma} \sin^2\theta \, d\phi^2}$$

where:

$$\Sigma = r^2 + a^2 \cos^2\theta, \qquad \Delta = r^2 - r_s r + a^2, \qquad r_s = 2M$$

##### Component-by-Component [THEOREM]

| Component | Expression | Lattice Interpretation [SELECTION] |
|-----------|------------|-----------------------------------|
| $g_{tt}$ | $-(1 - r_s r/\Sigma)$ | Temporal budget: fraction of processing capacity not consumed by gravitational + rotational data |
| $g_{t\phi}$ | $-r_s r a \sin^2\theta / \Sigma$ | **Directional budget asymmetry**: the cross-term that makes co-rotating cheaper than counter-rotating |
| $g_{rr}$ | $\Sigma/\Delta$ | Radial cost amplification: traversing saturated nodes costs more, modified by oblate load |
| $g_{\theta\theta}$ | $\Sigma$ | Polar cost: includes rotational contribution to computational load |
| $g_{\phi\phi}$ | $[(r^2 + a^2)^2 - \Delta a^2 \sin^2\theta] \sin^2\theta / \Sigma$ | Azimuthal cost: non-trivially modified by rotation |

#### B.4.2 The Cross-Term: Signature of Frame Dragging [SELECTION]

The $g_{t\phi}$ component is the qualitatively new term relative to Schwarzschild. In the proper time formula, it creates a coupling between temporal evolution and azimuthal motion:

$$d\tau^2 \supset -\frac{2 r_s r a \sin^2\theta}{\Sigma} \, dt \, d\phi$$

For an observer with azimuthal angular velocity $\omega = d\phi/dt$:

- If $\omega > 0$ (co-rotating with the mass), this term is **negative**, which **increases** $d\tau^2$ (the observer's clock runs faster relative to a static observer at the same $r$)
- If $\omega < 0$ (counter-rotating), this term is **positive**, which **decreases** $d\tau^2$ (the clock runs slower)

**Lattice interpretation [SELECTION]:** The vortical flux pattern carries information in the $+\phi$ direction. A co-rotating observer "surfs" the flux, gaining a budget bonus. A counter-rotating observer fights the flux, paying a budget penalty. This is why frame dragging is sometimes called "gravitomagnetism" -- it is the gravitational analog of a magnetic force acting on a moving charge.

#### B.4.3 General Proper Time [THEOREM]

For an observer with coordinate velocities $v_r = dr/dt$, $v_\theta = d\theta/dt$, and $\omega = d\phi/dt$:

$$\left(\frac{d\tau}{dt}\right)^2 = \left(1 - \frac{r_s r}{\Sigma}\right) + \frac{2 r_s r a \sin^2\theta}{\Sigma} \, \omega - \frac{\Sigma}{\Delta} v_r^2 - \Sigma \, v_\theta^2 - \frac{(r^2 + a^2)^2 - \Delta a^2 \sin^2\theta}{\Sigma} \sin^2\theta \, \omega^2$$

This generalizes the Schwarzschild formula $d\tau/dt = \sqrt{f - v_r^2/f}$ to include:
- Oblate gravitational saturation ($\Sigma$ replacing $r^2$ in several places)
- Angular momentum resistance ($a^2$ in $\Delta$)
- Directional budget asymmetry (the $\omega$ cross-term)

---

### B.5 FTD Interpretation

#### B.5.1 Frame Dragging as Asymmetric Flux Propagation [SELECTION]

Standard GR describes frame dragging as the "dragging of inertial frames" by a rotating mass. In FTD, the mechanism is more concrete:

1. A rotating mass establishes a **vortical flux pattern** $\mathbf{J}_{\text{vortex}}$ on the lattice
2. The vortex creates a preferred direction for flux propagation (the direction of circulation)
3. Information propagating with the circulation encounters pre-aligned flux -- less computational work is needed to advance through each node
4. Information propagating against the circulation must overwrite the pre-existing flux orientation -- more work per node

This is analogous to swimming with or against a current. The "current" is the background flux vortex, and the "swimming effort" is the computational budget consumed per unit displacement.

##### Frame-Dragging Angular Velocity [THEOREM]

The angular velocity at which a zero-angular-momentum observer (ZAMO) is dragged is:

$$\omega_{\text{drag}} = -\frac{g_{t\phi}}{g_{\phi\phi}} = \frac{r_s r a}{(r^2 + a^2)^2 - \Delta a^2 \sin^2\theta}$$

In the FTD picture, this is the angular velocity at which an observer must rotate to experience zero net azimuthal flux -- the velocity that exactly matches the background vortex speed.

#### B.5.2 The Ergosphere [SELECTION]

The **ergosphere** is the region where $g_{tt} > 0$, i.e., where:

$$1 - \frac{r_s r}{\Sigma} < 0 \qquad \Longrightarrow \qquad r < r_{\text{ergo}}(\theta) = M + \sqrt{M^2 - a^2 \cos^2\theta}$$

##### Lattice Interpretation [SELECTION]

Inside the ergosphere, the temporal computational budget goes negative. An observer trying to remain static ($d\phi/dt = 0$) would require more budget than is available -- the gravitational + rotational saturation exceeds 100% of the temporal capacity.

The only escape is to co-rotate. The $g_{t\phi}$ cross-term provides a budget subsidy for co-rotating observers that can offset the temporal deficit. Inside the ergosphere, **every physical observer must co-rotate with the mass** -- not because a force pushes them, but because the computational budget is only balanced when azimuthal motion supplements the temporal deficit.

This is forced co-rotation as a **budget constraint**, not as a dynamical force.

##### Comparison with Schwarzschild [SELECTION]

| Feature | Schwarzschild | Kerr |
|---------|---------------|------|
| Temporal budget zero | $f = 0$ at $r = r_s$ (horizon) | $g_{tt} = 0$ at $r = r_{\text{ergo}}(\theta)$ (ergosphere boundary) |
| Forced behavior at zero | Time stops; nothing can remain stationary | Must co-rotate; can still escape if moving outward |
| Region between | N/A | **Ergosphere**: temporal budget negative, but rotational subsidy available |

#### B.5.3 Two Horizons [THEOREM + SELECTION]

The Kerr horizons occur where $\Delta = 0$:

$$r_\pm = M \pm \sqrt{M^2 - a^2}$$

| Horizon | Location | Lattice Interpretation [SELECTION] |
|---------|----------|-----------------------------------|
| Outer ($r_+$) | $M + \sqrt{M^2 - a^2}$ | Complete radial saturation: no radial displacement possible (even with rotational subsidy). This is the event horizon. |
| Inner ($r_-$) | $M - \sqrt{M^2 - a^2}$ | Second saturation surface inside the outer horizon, associated with a Cauchy horizon and potential breakdown of predictability |

**Angular momentum shrinks the outer horizon**: $r_+ < r_s = 2M$ for $a > 0$. The rotational energy partially offsets gravitational collapse, pushing the horizon inward.

**Angular momentum creates the inner horizon**: $r_-$ has no Schwarzschild counterpart. It arises because the centrifugal barrier from angular momentum creates a second surface where $\Delta$ passes through zero from below.

#### B.5.4 Budget Conservation [THEOREM]

The determinant of the Kerr metric in Boyer-Lindquist coordinates:

$$\det(g_{\mu\nu}) = -\Sigma^2 \sin^2\theta$$

This depends on position ($r$ and $\theta$) but is **independent of the mass $M$ and spin $a$**. The identity holds for $a = 0$ (Schwarzschild, where $\Sigma = r^2$ and $\det(g) = -r^4 \sin^2\theta$) and for arbitrary $a$.

**Lattice interpretation [SELECTION]:** The total computational volume element -- the product of all budget channels -- is determined by the coordinate geometry alone, not by the mass or spin of the source. Gravity and rotation redistribute the budget across temporal, radial, polar, and azimuthal channels, but they do not change the total budget available at each spacetime point.

This generalizes the Schwarzschild result $g_{tt} \cdot g_{rr} = -1$ to the full Kerr geometry. Budget is conserved; it is only redistributed.

---

### B.6 Limits and Verification

#### B.6.1 Limit (a): Non-Rotating ($a \to 0$) [VERIFIED]

Setting $a = 0$:

$$\Sigma = r^2, \qquad \Delta = r^2 - r_s r = r(r - r_s)$$

The metric components become:

$$g_{tt} = -\left(1 - \frac{r_s}{r}\right) = -f, \qquad g_{t\phi} = 0$$

$$g_{rr} = \frac{r^2}{r(r - r_s)} = \frac{r}{r - r_s} = \frac{1}{f}$$

$$g_{\theta\theta} = r^2, \qquad g_{\phi\phi} = r^2 \sin^2\theta$$

This is **exactly** the Schwarzschild metric. The cross-term vanishes, the oblate load becomes spherical, and the availability factor reduces to $f(r)$. **PASS.**

#### B.6.2 Limit (b): Far Field ($r \to \infty$) [VERIFIED]

As $r \to \infty$ with $a$ fixed:

$$\Sigma \approx r^2, \qquad \Delta \approx r^2, \qquad \frac{r_s r}{\Sigma} \approx \frac{r_s}{r} \to 0$$

The metric components approach:

$$g_{tt} \to -1, \qquad g_{t\phi} \to 0, \qquad g_{rr} \to 1, \qquad g_{\theta\theta} \to r^2, \qquad g_{\phi\phi} \to r^2 \sin^2\theta$$

This is the **flat Minkowski metric** in spherical coordinates. Far from any mass, the lattice is unsaturated and rotationally isotropic. **PASS.**

#### B.6.3 Limit (c): Equatorial Plane ($\theta = \pi/2$) [VERIFIED]

At $\theta = \pi/2$ (the equator), $\cos\theta = 0$ and $\sin\theta = 1$:

$$\Sigma = r^2, \qquad \Delta = r^2 - r_s r + a^2$$

The metric simplifies to:

$$ds^2 = -\left(1 - \frac{r_s}{r}\right) dt^2 - \frac{2 r_s a}{r} \, dt \, d\phi + \frac{r^2}{\Delta} \, dr^2 + \frac{(r^2 + a^2)^2 - \Delta a^2}{r^2} \, d\phi^2$$

(with $d\theta = 0$ for motion confined to the equatorial plane).

The frame-dragging angular velocity simplifies to:

$$\omega_{\text{drag}} = \frac{r_s a}{(r^2 + a^2)^2 - \Delta a^2} = \frac{2Ma}{(r^2 + a^2)^2 - (r^2 - 2Mr + a^2)a^2}$$

This is the standard Kerr equatorial metric. **PASS.**

#### B.6.4 Limit (d): Extreme Kerr ($a = M$) [VERIFIED]

The maximum spin for a Kerr black hole is $a = M$ (the extremal limit). Setting $a = M$:

$$\Delta = r^2 - 2Mr + M^2 = (r - M)^2$$

The two horizons merge:

$$r_+ = r_- = M$$

A single degenerate horizon at $r = M$ (half the Schwarzschild radius $r_s = 2M$).

The ergosphere boundary at the equator ($\theta = \pi/2$):

$$r_{\text{ergo}} = M + \sqrt{M^2 - 0} = 2M = r_s$$

So the ergosphere extends from $r = M$ (degenerate horizon) to $r = 2M$ (Schwarzschild radius) at the equator -- the entire region between the Schwarzschild radius and the shrunk horizon is ergosphere.

**Lattice interpretation [SELECTION]:** Maximum spin creates the most extreme budget asymmetry. The rotational subsidy is so large that the horizon shrinks to half the non-rotating value. The ergosphere -- the region of forced co-rotation -- is maximized. **PASS.**

---

### B.7 The Born-Infeld Extension

#### B.7.1 Review of Schwarzschild-Exact Form [THEOREM]

From [SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md), the v2.1 Born-Infeld core:

$$\mathcal{L}_{\text{RB}} = -K_B \frac{\sqrt{f^2 - v^2}}{\sqrt{f}}$$

with $f = 1 - \mathcal{L}^2$ (lattice availability). This reproduces the Schwarzschild proper time exactly:

$$\frac{d\tau}{dt} = \frac{\sqrt{f^2 - v^2}}{\sqrt{f}} = \sqrt{f - \frac{v^2}{f}}$$

#### B.7.2 Generalization to Kerr [SELECTION]

The Schwarzschild Born-Infeld core assumes:
1. Isotropic availability ($f$ depends only on $r$)
2. No directional asymmetry (no cross-term between time and azimuthal angle)

For a rotating mass, we must generalize both. The proper time formula from B.4.3 gives:

$$\left(\frac{d\tau}{dt}\right)^2 = \left(1 - \frac{r_s r}{\Sigma}\right) + \frac{2 r_s r a \sin^2\theta}{\Sigma} \omega - \frac{\Sigma}{\Delta} v_r^2 - \Sigma \, v_\theta^2 - \frac{A}{\Sigma} \sin^2\theta \, \omega^2$$

where $A = (r^2 + a^2)^2 - \Delta a^2 \sin^2\theta$ and $\omega = d\phi/dt$.

##### Proposed Kerr-Extended Lagrangian [SELECTION]

$$\boxed{\mathcal{L}_{\text{Kerr}} = -K_B \sqrt{\left(1 - \frac{r_s r}{\Sigma}\right) + \frac{2 r_s r a \sin^2\theta}{\Sigma} \omega - \frac{\Sigma}{\Delta} v_r^2 - \Sigma \, v_\theta^2 - \frac{A}{\Sigma} \sin^2\theta \, \omega^2}}$$

This is structurally a Born-Infeld action: a square root of a quadratic form in the velocities $(v_r, v_\theta, \omega)$ with position-dependent coefficients. The square root enforces the constraint that the expression under the radical must remain non-negative -- the gravitationally modified speed limit.

##### Equivalence to Geodesic Action [THEOREM]

The Lagrangian $\mathcal{L}_{\text{Kerr}} = -K_B \sqrt{-g_{\mu\nu} \dot{x}^\mu \dot{x}^\nu / \dot{t}^2}$ is (up to sign and constant) the standard **reparametrization-invariant geodesic Lagrangian**. The Euler-Lagrange equations derived from this action reproduce the Kerr geodesic equations exactly. This is a mathematical identity, independent of the lattice interpretation.

##### New Features Relative to Schwarzschild [SELECTION]

| Feature | Schwarzschild BI | Kerr BI |
|---------|------------------|---------|
| Availability | $f(r)$ (scalar) | $1 - r_s r / \Sigma(r,\theta)$ (position-dependent) |
| Radial cost | $v_r^2 / f$ | $(\Sigma/\Delta) v_r^2$ |
| Polar cost | $v_\theta^2$ (trivial) | $\Sigma \, v_\theta^2$ (non-trivial) |
| Azimuthal cost | $r^2 \sin^2\theta \, \omega^2$ | $(A/\Sigma) \sin^2\theta \, \omega^2$ |
| Cross-term | 0 | $(2 r_s r a \sin^2\theta / \Sigma) \omega$ |
| Speed limit | $v_r < f$ | Direction-dependent; co-rotating can exceed counter-rotating |

#### B.7.3 Reduction to Schwarzschild [VERIFIED]

Setting $a = 0$ in the Kerr Lagrangian:

$$\Sigma = r^2, \quad \Delta = r(r - r_s) = r^2 f, \quad A = r^4, \quad \text{cross-term} = 0$$

$$\mathcal{L}_{\text{Kerr}} \to -K_B \sqrt{f - \frac{r^2}{r^2 f} v_r^2 - r^2 v_\theta^2 - r^2 \sin^2\theta \, \omega^2}$$

$$= -K_B \sqrt{f - \frac{v_r^2}{f} - r^2 v_\theta^2 - r^2 \sin^2\theta \, \omega^2}$$

For purely radial motion ($v_\theta = \omega = 0$), this is $-K_B \sqrt{f - v_r^2/f}$ -- exactly the Schwarzschild Born-Infeld core from [SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md). **PASS.**

#### B.7.4 Remaining Open Questions [OPEN]

1. **Latency generalization**: In the Schwarzschild case, the availability factor was identified as $f = 1 - \mathcal{L}^2$ where $\mathcal{L}$ is the topological latency. The Kerr generalization $1 - r_s r / \Sigma$ cannot be written as $1 - \mathcal{L}^2$ for any simple scalar $\mathcal{L}$ because the latency is now direction-dependent. A tensorial latency $\mathcal{L}^{ij}$ may be required.

2. **Poisson equation generalization**: The Schwarzschild latency satisfies $\nabla^2 \mathcal{L} = 4\pi G \rho$. The Kerr case requires the full Einstein field equations (or at minimum, the Ernst equation for stationary axisymmetric vacuum). Whether the lattice framework can produce the Ernst equation from computational budget arguments is [OPEN].

3. **Superradiance**: The Kerr metric admits superradiant scattering -- waves can extract rotational energy from the black hole. Whether this has a lattice budget interpretation (extracting computational capacity from the vortex) is [OPEN].

---

### B.8 Claims Table

#### B.8.1 Claims Summary

| ID | Claim | Tag | Evidence | Falsification |
|----|-------|-----|----------|---------------|
| KR-1 | Kerr metric in Boyer-Lindquist coordinates | [THEOREM] | Standard GR (Kerr 1963, Carter 1968) | Algebraic identity -- unfalsifiable |
| KR-2 | Rotating mass creates vortical flux pattern on the lattice | [SELECTION] | Consistent with angular momentum conservation; analogous to electromagnetic vortex from rotating charge | Alternative flux pattern that produces same metric |
| KR-3 | Directional budget asymmetry produces $g_{t\phi}$ cross-term | [SELECTION] | Correct structure (co-rotating cheaper); reproduces frame dragging | Show budget asymmetry produces wrong sign or magnitude |
| KR-4 | $\Sigma = r^2 + a^2 \cos^2\theta$ is oblate computational load | [SELECTION] | Correct $\theta$-dependence (equator vs poles); reduces to $r^2$ for $a = 0$ | Alternative interpretation of oblate structure |
| KR-5 | $\Delta = r^2 - r_s r + a^2$: angular momentum resists saturation | [SELECTION] | Correct horizon structure; $a^2$ term shrinks outer horizon | Derivation of $\Delta$ from FTD axioms contradicting this form |
| KR-6 | Ergosphere = forced co-rotation from temporal budget deficit | [SELECTION] | Matches GR prediction that static observers cannot exist inside ergosphere | Physical measurement contradicting forced co-rotation |
| KR-7 | Budget conservation: $\det(g) = -\Sigma^2 \sin^2\theta$ independent of $M, a$ | [THEOREM] | Direct computation from metric components | Algebraic identity -- unfalsifiable |
| KR-8 | $a \to 0$ recovers Schwarzschild exactly | [VERIFIED] | Explicit computation (B.6.1) | Algebraic -- unfalsifiable |
| KR-9 | $r \to \infty$ recovers Minkowski exactly | [VERIFIED] | Explicit computation (B.6.2) | Algebraic -- unfalsifiable |
| KR-10 | Kerr Born-Infeld Lagrangian reproduces Kerr geodesics | [THEOREM] | Equivalent to reparametrization-invariant geodesic action | Algebraic identity |
| KR-11 | Latency generalization to tensorial form | [OPEN] | No derivation yet; scalar $\mathcal{L}$ insufficient for Kerr | Future work |
| KR-12 | Superradiance from lattice budget extraction | [OPEN] | Qualitative analogy only | Future work |

#### B.8.2 Epistemic Breakdown

| Category | Count | Examples |
|----------|-------|---------|
| [THEOREM] (standard GR results) | 4 | KR-1, KR-7, KR-8/9, KR-10 |
| [SELECTION] (lattice interpretation) | 5 | KR-2, KR-3, KR-4, KR-5, KR-6 |
| [CONJECTURE] | 0 | -- |
| [OPEN] | 2 | KR-11, KR-12 |

#### B.8.3 What This Part Does NOT Claim

1. The Kerr metric is **derived** from FTD axioms -- it is interpreted within the lattice budget framework, not derived from it
2. The vortical flux pattern is the **unique** lattice representation of angular momentum -- it is argued, not proven
3. The Born-Infeld extension to Kerr **predicts** anything beyond standard GR -- it reproduces known geodesic equations
4. The lattice budget framework handles Kerr-Newman (charged + rotating) or cosmological Kerr-de Sitter metrics -- these remain future work
5. Frame dragging is **derived** from the flux vortex -- the vortex is the interpretive lens through which the known GR result is understood

---

### B.9 Appendix: The Kerr Budget in Computational Language

For reference, the full Kerr proper time formula in lattice computational language:

$$\left(\frac{d\tau}{dT_U}\right)^2 = \underbrace{\left(1 - \frac{r_s r}{\Sigma}\right)}_{\text{temporal budget}} + \underbrace{\frac{2 r_s r a \sin^2\theta}{\Sigma} \, \omega}_{\text{rotational subsidy/penalty}} - \underbrace{\frac{\Sigma}{\Delta} \, v_r^2}_{\text{radial cost}} - \underbrace{\Sigma \, v_\theta^2}_{\text{polar cost}} - \underbrace{\frac{A}{\Sigma} \sin^2\theta \, \omega^2}_{\text{azimuthal cost}}$$

where:
- $d\tau$ = proper time (experienced $G^*$ collapse cycles)
- $dT_U$ = Universal Tick (background render rate)
- $\Sigma = r^2 + a^2 \cos^2\theta$ = oblate computational load
- $\Delta = r^2 - r_s r + a^2$ = modified horizon function
- $A = (r^2 + a^2)^2 - \Delta a^2 \sin^2\theta$ = azimuthal metric factor
- $a = J/(Mc)$ = specific angular momentum of the source
- $\omega = d\phi/dt$ = azimuthal angular velocity of the observer
- $v_r, v_\theta$ = radial and polar coordinate velocities

The **five budget channels** (temporal, rotational, radial, polar, azimuthal) replace the **two channels** (temporal, radial) of the Schwarzschild case. The total budget is still conserved: $\det(g) = -\Sigma^2 \sin^2\theta$ is independent of $M$ and $a$.

The formula requires no differential geometry or Riemannian curvature. It requires only:
1. The lattice (POSTULATE 1)
2. The $G^*$ exchange rate (from lemniscatic geometry)
3. The oblate computational load $\Sigma$ (from vortical flux)
4. The modified horizon function $\Delta$ (angular momentum resisting saturation)
5. The directional budget asymmetry (co-rotation is cheaper)

And it reproduces Kerr GR exactly.

---

## Part C: Reissner-Nordstrom Metric

This part extends the lattice computational budget framework to charged, non-rotating black holes -- the Reissner-Nordstrom (RN) solution (Reissner 1916, Nordstrom 1918). The central new idea: a charged mass creates an electromagnetic flux field whose energy density **adds** computational capacity back to the lattice, opposing the gravitational drain. Mass consumes budget (attractive); charge restores it (repulsive at short range). A structurally significant connection emerges: the FTD Lagrangian is a Born-Infeld action, and Born-Infeld electrodynamics was originally proposed by Born and Infeld (1934) to regularize the electromagnetic self-energy of the electron -- the lattice provides exactly the UV regularization that Born and Infeld sought phenomenologically.

---

### C.1 Review of Schwarzschild and Kerr Budget Frameworks

#### C.1.1 The Schwarzschild Framework [THEOREM]

From Part A: a spherically symmetric mass $M$ saturates the computational capacity of nearby lattice nodes. The fraction of capacity remaining is the **availability factor**:

$$f(r) = 1 - \frac{r_s}{r}$$

where $r_s = 2GM/c^2$ is the Schwarzschild radius. The complete proper time formula is:

$$\frac{d\tau}{dT_U} = \sqrt{f - \frac{v_r^2}{f}} = \sqrt{\frac{f^2 - v_r^2}{f}}$$

Key properties:
- **Budget conservation**: $g_{tt} \cdot g_{rr} = f \cdot (-1/f) = -1$. Gravity redistributes budget between temporal and spatial channels without creating or destroying it.
- **Velocity cost amplification**: radial displacement costs $v_r^2/f$, not $v_r^2$, because traversing saturated nodes requires more computational cycles.
- **Single horizon**: $f = 0$ at $r = r_s$. Complete saturation; time stops.

#### C.1.2 The Kerr Framework [SELECTION]

From Part B: a rotating mass with angular momentum $J = Ma$ creates a **vortical flux pattern** on the lattice, making the computational budget direction-dependent. The scalar availability factor $f(r)$ is replaced by:

- $\Sigma(r,\theta) = r^2 + a^2 \cos^2\theta$ -- oblate computational load
- $\Delta(r) = r^2 - r_s r + a^2$ -- modified horizon function

The key new feature is the $g_{t\phi}$ cross-term encoding frame dragging: co-rotating information propagation is cheaper than counter-rotating, because the vortex carries flux in the preferred azimuthal direction.

Two horizons arise from $\Delta = 0$:

$$r_\pm = M \pm \sqrt{M^2 - a^2}$$

Angular momentum **resists** gravitational collapse -- the $+a^2$ term in $\Delta$ acts as centrifugal support, shrinking the outer horizon relative to Schwarzschild.

#### C.1.3 The Common Principle [SELECTION]

Both Schwarzschild and Kerr share a unifying principle within the lattice budget framework:

> **Gravitational data processing consumes lattice computational budget.** Each lattice node near a mass must process gravitational field data (encoding curvature, flux density, tidal information). This processing load reduces the node's available capacity for other operations -- spatial translation and internal state evolution (proper time).

The budget is **always conserved**. The determinant of the metric -- which measures the total computational volume element -- is independent of the source parameters:

| Solution | $\det(g_{\mu\nu})$ | Depends on $M$? | Depends on second parameter? |
|----------|---------------------|------------------|------------------------------|
| Schwarzschild | $-r^4 \sin^2\theta$ | No | N/A |
| Kerr | $-\Sigma^2 \sin^2\theta = -(r^2 + a^2 \cos^2\theta)^2 \sin^2\theta$ | No | No ($a$ enters through $\Sigma$, but $\Sigma$ is a coordinate function) |

The question this part addresses: **what happens when the source carries charge in addition to mass?**

---

### C.2 Electromagnetic Field on the Lattice

#### C.2.1 Dual Nature of Flux [SELECTION]

In FTD, the flux field $\mathbf{J}(v,t) \in \mathbb{R}^3$ serves dual roles (see [DERIV_QFT_GRT_BRIDGE.md](DERIV_QFT_GRT_BRIDGE.md)):

| Aspect | Quantity | Role |
|--------|----------|------|
| Gravitational | $\rho = |\mathbf{J}|$ (flux magnitude) | Sources curvature; determines saturation |
| Electromagnetic | $\nabla \cdot \mathbf{J}$ (divergence), $\nabla \times \mathbf{J}$ (curl) | Sources electric/magnetic fields |

A charged mass has **both** a gravitational flux envelope (high $|\mathbf{J}|$) and an electromagnetic flux structure (nonzero $\nabla \cdot \mathbf{J}$). The gravitational aspect saturates lattice nodes (consuming budget). The electromagnetic aspect carries additional energy that also interacts with the lattice budget -- but with a crucial sign difference.

#### C.2.2 Electromagnetic Energy Density [THEOREM]

For a point charge $Q$ at rest, the electric field in natural units ($G = c = 4\pi\varepsilon_0 = 1$) is:

$$\mathbf{E} = \frac{Q}{r^2} \hat{r}$$

The electromagnetic energy-momentum tensor is:

$$T^{\text{EM}}_{\mu\nu} = \frac{1}{4\pi}\left(F_{\mu\alpha} F^{\alpha}_{\ \nu} - \frac{1}{4} g_{\mu\nu} F_{\alpha\beta} F^{\alpha\beta}\right)$$

For a purely electric, spherically symmetric field:

- Energy density: $T^{\text{EM}}_{00} = \frac{E^2}{8\pi} = \frac{Q^2}{8\pi r^4}$ -- **positive** (energy is stored in the field)
- Radial pressure: $T^{\text{EM}}_{rr} = -\frac{E^2}{8\pi} = -\frac{Q^2}{8\pi r^4}$ -- **negative** (radial tension)
- Tangential pressure: $T^{\text{EM}}_{\theta\theta} = T^{\text{EM}}_{\phi\phi} = +\frac{E^2}{8\pi}$ -- **positive** (tangential compression)

The crucial feature is the **sign structure**: $T^{\text{EM}}_{00} > 0$ but $T^{\text{EM}}_{rr} < 0$. The electromagnetic field has positive energy but negative radial pressure (tension). This combination, when sourcing Einstein's equations, produces a **repulsive** gravitational effect at short range.

#### C.2.3 Electromagnetic Budget Contribution [SELECTION]

In the lattice budget framework, the electromagnetic field energy acts as an **anti-saturation** mechanism:

> Each lattice node near a charged mass must process both gravitational data (from the mass $M$) and electromagnetic data (from the charge $Q$). The gravitational processing **consumes** computational budget, reducing the availability factor. The electromagnetic processing, due to the sign structure of $T^{\text{EM}}_{\mu\nu}$, **adds capacity back** to the availability factor at short range.

The physical intuition: the electromagnetic field's radial tension ($T^{\text{EM}}_{rr} < 0$) counteracts gravitational compression. In lattice terms, the EM field provides a mechanism for the lattice to resist complete saturation -- the charge "pushes back" against the gravitational drain on computational resources.

This is **not** a claim that electromagnetic energy is negative -- it is positive ($T^{\text{EM}}_{00} > 0$). The repulsive effect arises from the **anisotropic stress** structure of the electromagnetic energy-momentum tensor, which enters Einstein's equations through $R_{\mu\nu} - \frac{1}{2}g_{\mu\nu}R = 8\pi G \, T_{\mu\nu}$ and produces a metric contribution with the opposite sign to the mass term in $f(r)$.

#### C.2.4 Lattice Regularization and Born-Infeld [SELECTION]

The electromagnetic energy density $u_{\text{EM}} = Q^2/(8\pi r^4)$ diverges as $r \to 0$ in the continuum. The total electromagnetic self-energy:

$$E_{\text{self}} = \int_0^\infty \frac{Q^2}{8\pi r^4} \cdot 4\pi r^2 \, dr = \int_0^\infty \frac{Q^2}{2r^2} \, dr \to \infty$$

This divergence was the original motivation for Born-Infeld electrodynamics (1934): Born and Infeld proposed a nonlinear modification of Maxwell's equations with a maximum field strength $E_{\max}$, which regularizes the self-energy to a finite value.

In FTD, the lattice provides this regularization naturally. The discrete lattice spacing $\ell_P$ (one voxel = one Planck length) sets a minimum distance:

$$r_{\min} = \ell_P$$

The maximum electric field on the lattice is:

$$E_{\max} = \frac{Q}{r_{\min}^2} = \frac{Q}{\ell_P^2}$$

and the self-energy is finite:

$$E_{\text{self}} = \int_{\ell_P}^\infty \frac{Q^2}{2r^2} \, dr = \frac{Q^2}{2\ell_P}$$

This connection is structurally significant: FTD's Born-Infeld Lagrangian $\mathcal{L}_{\text{RB}} = -K_B\sqrt{(f^2 - v^2)/f}$ naturally accommodates electromagnetic contributions to the metric, and the lattice provides exactly the UV cutoff that Born and Infeld introduced phenomenologically. The FTD framework realizes the Born-Infeld program from first principles -- the lattice IS the fundamental regularization.

---

### C.3 The Reissner-Nordstrom Availability Factor

#### C.3.1 The RN Availability Factor [THEOREM]

The Reissner-Nordstrom solution to the coupled Einstein-Maxwell equations gives the availability factor for a mass $M$ with charge $Q$:

$$\boxed{f(r) = 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2}}$$

where:

| Parameter | Definition | Natural units ($G = c = 4\pi\varepsilon_0 = 1$) |
|-----------|------------|--------------------------------------------------|
| $r_s$ | Schwarzschild radius | $2M$ |
| $r_Q$ | Charge radius | $Q$ (i.e., $r_Q^2 = Q^2$) |

In SI units: $r_s = 2GM/c^2$ and $r_Q^2 = GQ^2/(4\pi\varepsilon_0 c^4)$.

#### C.3.2 Decomposition of Budget Consumption [SELECTION]

The three terms in $f(r)$ have distinct lattice interpretations:

| Term | Source | Sign in $f(r)$ | Effect on Budget | Radial Dependence |
|------|--------|----------------|------------------|-------------------|
| $+1$ | Flat-space baseline | Positive | Full capacity | Constant |
| $-r_s/r$ | Mass (gravitational) | Negative | Consumes budget | $\sim 1/r$ (slow decay) |
| $+r_Q^2/r^2$ | Charge (electromagnetic) | Positive | Restores budget | $\sim 1/r^2$ (fast decay) |

**Lattice interpretation [SELECTION]:** The gravitational term $-r_s/r$ represents the fraction of each node's computational capacity consumed by processing gravitational field data -- the same mechanism as in the Schwarzschild case. The electromagnetic term $+r_Q^2/r^2$ represents the fraction of capacity **restored** by the electromagnetic field's anisotropic stress structure. At large $r$, the gravitational drain dominates ($1/r$ vs $1/r^2$). At small $r$, the electromagnetic restoration dominates, preventing complete saturation unless $Q < M$.

This produces a qualitatively different picture from Schwarzschild:

$$f(r) = \frac{r^2 - r_s r + r_Q^2}{r^2}$$

The numerator is a quadratic in $r$ with **two** roots (when they exist), rather than the single root of the Schwarzschild case.

#### C.3.3 Horizon Structure [THEOREM]

Setting $f(r) = 0$:

$$r^2 - r_s r + r_Q^2 = 0$$

$$r_\pm = \frac{r_s}{2} \pm \sqrt{\left(\frac{r_s}{2}\right)^2 - r_Q^2} = M \pm \sqrt{M^2 - Q^2}$$

Three regimes:

| Condition | Horizons | Physical Regime |
|-----------|----------|-----------------|
| $Q < M$ (sub-extremal) | Two: $r_+ > r_-$ | Standard charged black hole |
| $Q = M$ (extremal) | One (degenerate): $r_+ = r_- = M$ | Extremal RN black hole |
| $Q > M$ (super-extremal) | None (complex roots) | Naked singularity (likely unphysical; cosmic censorship) |

#### C.3.4 Comparison with Kerr Horizons [THEOREM]

The horizon equations for Kerr and Reissner-Nordstrom have identical algebraic structure:

| Property | Kerr | Reissner-Nordstrom |
|----------|------|--------------------|
| Second parameter | $a$ (specific angular momentum) | $Q$ (charge) |
| $\Delta$ or numerator | $r^2 - r_s r + a^2$ | $r^2 - r_s r + r_Q^2$ |
| Horizons | $r_\pm = M \pm \sqrt{M^2 - a^2}$ | $r_\pm = M \pm \sqrt{M^2 - Q^2}$ |
| Extremal condition | $a = M$ | $Q = M$ |
| Physical mechanism resisting collapse | Centrifugal support from angular momentum | Electromagnetic repulsion from charge |
| Symmetry | Axial (oblate) | Spherical (isotropic) |
| Frame dragging ($g_{t\phi} \neq 0$) | Yes | No |

**Lattice interpretation [SELECTION]:** Both angular momentum and charge provide mechanisms that **resist complete gravitational saturation**. In the Kerr case, the resistance is directional (centrifugal, along the equatorial plane). In the RN case, the resistance is isotropic (electromagnetic pressure, spherically symmetric). Both enter the horizon equation as a positive $+\text{(parameter)}^2$ term that opposes the gravitational $-r_s r$ drain.

This structural parallel is not accidental -- it reflects a deep property of the Einstein equations: any energy content beyond pure mass resists gravitational collapse, because the additional energy has stress-energy components that partially counteract the attraction.

---

### C.4 The Full RN Line Element

#### C.4.1 The Metric [THEOREM]

The Reissner-Nordstrom line element in Schwarzschild-like coordinates $(t, r, \theta, \phi)$, in natural units ($G = c = 1$):

$$\boxed{ds^2 = -f(r) \, dt^2 + \frac{dr^2}{f(r)} + r^2 \, d\Omega^2}$$

where:

$$f(r) = 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2}, \qquad d\Omega^2 = d\theta^2 + \sin^2\theta \, d\phi^2$$

This is **spherically symmetric** -- the charge does not break spherical symmetry because the Coulomb field of a point charge is isotropic. Unlike Kerr (where angular momentum selects a preferred axis), charge produces no directional preference.

#### C.4.2 Component-by-Component Lattice Interpretation [SELECTION]

| Component | Expression | Lattice Interpretation |
|-----------|------------|------------------------|
| $g_{tt} = -f(r)$ | $-(1 - r_s/r + r_Q^2/r^2)$ | **Temporal budget**: gravitational drain ($-r_s/r$) partially compensated by electromagnetic restoration ($+r_Q^2/r^2$). Clock rate reflects the net computational capacity. |
| $g_{rr} = 1/f(r)$ | $r^2/(r^2 - r_s r + r_Q^2)$ | **Radial cost amplification**: traversing nodes costs $v_r^2/f$, amplified when net saturation is high. Same mechanism as Schwarzschild, with modified $f$. |
| $g_{\theta\theta} = r^2$ | $r^2$ | **Polar cost**: unchanged from flat space. Spherical symmetry is preserved. |
| $g_{\phi\phi} = r^2\sin^2\theta$ | $r^2\sin^2\theta$ | **Azimuthal cost**: unchanged from flat space. No frame dragging (no rotation). |
| $g_{t\phi}$ | $0$ | **No cross-term**: charge is a scalar quantity with no preferred direction. No directional budget asymmetry. |

##### Structural Comparison

| Feature | Schwarzschild | Kerr | Reissner-Nordstrom |
|---------|---------------|------|--------------------|
| $g_{tt}$ | $-(1 - r_s/r)$ | $-(1 - r_s r/\Sigma)$ | $-(1 - r_s/r + r_Q^2/r^2)$ |
| $g_{rr}$ | $1/(1 - r_s/r)$ | $\Sigma/\Delta$ | $1/(1 - r_s/r + r_Q^2/r^2)$ |
| Angular sector | $r^2 d\Omega^2$ | $\Sigma \, d\theta^2 + [(r^2+a^2)^2 - \Delta a^2\sin^2\theta]\sin^2\theta \, d\phi^2 / \Sigma$ | $r^2 d\Omega^2$ |
| Cross-term $g_{t\phi}$ | 0 | $-r_s r a \sin^2\theta / \Sigma$ | 0 |
| Spherical symmetry | Yes | No (axial) | Yes |

#### C.4.3 Budget Conservation [THEOREM]

**Theorem 4.1** (RN Budget Conservation): *The product $g_{tt} \cdot g_{rr} = -1$ for the Reissner-Nordstrom metric.*

**Proof:**

$$g_{tt} \cdot g_{rr} = \left(-f(r)\right) \cdot \frac{1}{f(r)} = -1 \qquad \blacksquare$$

This is identical to the Schwarzschild result (Theorem 8.1 of Part A). The charge parameter $r_Q$ modifies $f(r)$ but does not affect the reciprocal relationship between temporal and radial metric components.

**Lattice interpretation [SELECTION]:** Budget conservation persists in the presence of charge. The electromagnetic field redistributes computational capacity -- adding to the temporal budget and reducing the radial cost -- but the total capacity product remains fixed. Gravity, angular momentum, and charge can all redistribute the budget, but none can create or destroy it. This is the lattice expression of the diffeomorphism invariance of GR: the coordinate volume element is determined by geometry, not by source content.

The determinant of the RN metric:

$$\det(g_{\mu\nu}) = -f(r) \cdot \frac{1}{f(r)} \cdot r^2 \cdot r^2 \sin^2\theta = -r^4 \sin^2\theta$$

This is **independent of $M$ and $Q$** -- identical to the Schwarzschild and flat-space results. Budget conservation is exact.

#### C.4.4 The RN Proper Time Formula [THEOREM]

For a radially moving observer with coordinate velocity $v_r = dr/dt$:

$$\boxed{\frac{d\tau}{dt} = \sqrt{f - \frac{v_r^2}{f}} = \sqrt{\frac{f^2 - v_r^2}{f}}}$$

where $f = 1 - r_s/r + r_Q^2/r^2$.

This has the **same functional form** as the Schwarzschild proper time formula (Theorem 6.1 of Part A), with $f$ now containing the electromagnetic contribution. The velocity cost amplification factor $v_r^2/f$ persists -- radial traversal through charged-mass-saturated nodes costs more computational budget per unit displacement.

---

### C.5 Born-Infeld Extension

#### C.5.1 The Schwarzschild-Exact Born-Infeld Core [THEOREM]

From [SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md), the v2.1 Born-Infeld render-bridge Lagrangian:

$$\mathcal{L}_{\text{RB}} = -K_B \frac{\sqrt{f^2 - v^2}}{\sqrt{f}}$$

reproduces the Schwarzschild proper time exactly for $f = 1 - r_s/r$.

#### C.5.2 Generalization to Reissner-Nordstrom [SELECTION]

The RN extension is immediate: replace $f$ with the charged availability factor:

$$\boxed{\mathcal{L}_{\text{RN}} = -K_B \frac{\sqrt{f^2 - v^2}}{\sqrt{f}}, \qquad f = 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2}}$$

The Born-Infeld structure is **unchanged** -- only the definition of $f$ is modified. This is because the RN metric has the same functional relationship between $g_{tt}$ and $g_{rr}$ as Schwarzschild: $g_{rr} = -1/g_{tt}$. The Born-Infeld core depends on $f$ through the proper time formula, and the proper time formula depends only on the relationship $g_{tt} = -f$, $g_{rr} = 1/f$.

##### Equivalence to Geodesic Action [THEOREM]

As in the Schwarzschild case, $\mathcal{L}_{\text{RN}} = -K_B \sqrt{-g_{\mu\nu}\dot{x}^\mu\dot{x}^\nu / \dot{t}^2}$ for the RN metric. The Euler-Lagrange equations reproduce the RN geodesic equations exactly. This is a mathematical identity.

#### C.5.3 The Born-Infeld Connection [SELECTION]

The relationship between FTD's Born-Infeld Lagrangian and the original Born-Infeld electrodynamics is structurally illuminating:

| Aspect | Born-Infeld (1934) | FTD Render-Bridge |
|--------|---------------------|-------------------|
| **Motivation** | Regularize electron self-energy | Enforce speed limit + unify gravity and kinematics |
| **What is limited** | Maximum electric field $E_{\max}$ | Total bandwidth $(v^2 + \mathcal{L}^2 < 1)$ |
| **UV regularization** | Ad hoc maximum field strength | Lattice spacing $\ell_P$ (structural) |
| **Gravity** | Not included | Native ($f = 1 - \mathcal{L}^2$) |
| **Charge in metric** | Not addressed | Included via $f = 1 - r_s/r + r_Q^2/r^2$ |
| **Self-energy** | Finite (by construction) | Finite (by lattice discreteness) |
| **Speed limit** | Not built in | Built in ($v < f$) |

The FTD framework achieves what Born and Infeld originally sought: a unified nonlinear electrodynamics with finite self-energy and a natural maximum field strength. The lattice is the physical realization of their mathematical regularization.

Specifically:
1. Born-Infeld electrodynamics regularizes the $1/r^2$ Coulomb divergence by imposing a maximum field strength. FTD regularizes it by imposing a minimum distance (one lattice unit).
2. Born-Infeld electrodynamics uses a square root Lagrangian $\sqrt{1 - F_{\mu\nu}F^{\mu\nu}/E_{\max}^2}$ that naturally limits field amplitudes. FTD uses a square root Lagrangian $\sqrt{(f^2 - v^2)/f}$ that naturally limits velocities.
3. Both approaches arise from the same mathematical structure: a constraint that prevents a physical quantity from exceeding a maximum value, implemented through a square root that diverges (in the conjugate momentum) as the limit is approached.

#### C.5.4 Relativistic Momentum [THEOREM]

The conjugate momentum derived from $\mathcal{L}_{\text{RN}}$:

$$p_r = \frac{\partial \mathcal{L}_{\text{RN}}}{\partial v_r} = \frac{K_B \, v_r}{\sqrt{f}\,\sqrt{f^2 - v_r^2}} = K_B \, \gamma_{\text{RN}} \, v_r$$

where the RN Lorentz factor is:

$$\gamma_{\text{RN}} = \frac{\sqrt{f}}{\sqrt{f^2 - v_r^2}}, \qquad f = 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2}$$

This has the same form as the Schwarzschild Lorentz factor (Section 5.1 of [SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md)), with the charged availability factor. The gravitationally modified speed limit is:

$$v_r < f(r) = 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2}$$

Note that $f(r)$ can be **larger** than $1 - r_s/r$ (the uncharged value) in the region where $r_Q^2/r^2$ is significant. The electromagnetic contribution effectively **increases** the local speed limit relative to Schwarzschild at the same mass -- the charge partially de-saturates the lattice, allowing faster information propagation.

---

### C.6 Limiting Cases

#### C.6.1 Case (a): $Q \to 0$ Recovers Schwarzschild [VERIFIED]

Setting $r_Q = 0$:

$$f(r) = 1 - \frac{r_s}{r} + 0 = 1 - \frac{r_s}{r}$$

This is the Schwarzschild availability factor. The single horizon at $r = r_s$ is recovered. All electromagnetic contributions vanish. The proper time formula, Born-Infeld Lagrangian, and budget conservation reduce to their Schwarzschild forms.

**PASS.**

#### C.6.2 Case (b): $r \to \infty$ Recovers Flat Space [VERIFIED]

As $r \to \infty$:

$$f(r) = 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2} \to 1 - 0 + 0 = 1$$

The metric becomes:

$$ds^2 \to -dt^2 + dr^2 + r^2 \, d\Omega^2$$

This is the Minkowski metric in spherical coordinates. Far from the charged mass, the lattice is unsaturated and all budget channels operate at full capacity.

**PASS.**

#### C.6.3 Case (c): Extremal RN ($Q = M$, i.e., $r_Q = r_s/2$) [VERIFIED]

When $Q = M$ (equivalently $r_Q = M = r_s/2$ in natural units), the two horizons merge:

$$r_+ = r_- = M = \frac{r_s}{2}$$

The availability factor becomes a perfect square:

$$f(r) = 1 - \frac{2M}{r} + \frac{M^2}{r^2} = \left(1 - \frac{M}{r}\right)^2$$

**Lattice interpretation [SELECTION]:** At the extremal limit, the electromagnetic restoration exactly balances the gravitational drain at the degenerate horizon. The availability factor touches zero ($f = 0$ at $r = M$) but does so **quadratically**, not linearly as in the Schwarzschild case. This means the approach to zero is gentler -- the gradient of $f$ vanishes at the horizon:

$$\frac{df}{dr}\bigg|_{r=M} = 0$$

In lattice terms, the computational saturation at the extremal horizon is a **saddle point**, not a cliff. The gravitational budget drain and electromagnetic budget restoration are in perfect equilibrium, creating a marginally stable horizon.

The surface gravity of an extremal RN black hole is zero ($\kappa = 0$), which implies zero Hawking temperature ($T_H = \kappa/(2\pi) = 0$). In the lattice picture: the equilibrium is so perfect that no thermal fluctuations can overcome it -- no Hawking radiation is emitted.

**PASS.**

#### C.6.4 Case (d): Weak Field ($r \gg r_s, r_Q$) [VERIFIED]

In the weak-field regime:

$$f(r) \approx 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2} = 1 - \frac{2GM}{rc^2} + \frac{GQ^2}{4\pi\varepsilon_0 c^4 r^2}$$

The Newtonian potential is $\Phi = -GM/r$, so $f \approx 1 + 2\Phi/c^2 + O(\Phi^2)$ with an electromagnetic correction at $O(1/r^2)$.

The proper time for a static observer:

$$\frac{d\tau}{dt} = \sqrt{f} \approx 1 - \frac{GM}{rc^2} + \frac{GQ^2}{8\pi\varepsilon_0 c^4 r^2}$$

The gravitational redshift acquires a charge-dependent correction. This correction is negligible for astrophysical objects (where $Q/M \ll 1$ in Planck units) but is significant for elementary particles (where $Q/M$ can be large).

**PASS.**

#### C.6.5 Case (e): Comparison with Kerr [VERIFIED]

| Property | Kerr | Reissner-Nordstrom |
|----------|------|--------------------|
| Second parameter | $a$ (angular momentum per unit mass) | $Q$ (charge) |
| Symmetry | Axial (oblate geometry, $\Sigma$ depends on $\theta$) | Spherical ($f$ depends only on $r$) |
| Frame dragging | Yes ($g_{t\phi} \neq 0$) | No ($g_{t\phi} = 0$) |
| Budget asymmetry | Directional (co-rotating cheaper) | Isotropic (no preferred direction) |
| Horizon equation | $r^2 - r_s r + a^2 = 0$ | $r^2 - r_s r + r_Q^2 = 0$ |
| Extremal condition | $a = M$ | $Q = M$ |
| Physical resistance mechanism | Centrifugal support | Electromagnetic repulsion |
| Ergosphere | Yes (region of forced co-rotation) | No (no rotation means no ergosphere) |
| $g_{tt} \cdot g_{rr}$ | $\neq -1$ (off-diagonal terms) | $= -1$ (diagonal metric) |
| $\det(g)$ independent of parameters | Yes ($-\Sigma^2\sin^2\theta$) | Yes ($-r^4\sin^2\theta$) |

Both solutions share the key feature: **the second parameter resists gravitational collapse**, entering the horizon equation as a positive term that reduces the horizon radius relative to Schwarzschild. The mechanisms are entirely different (centrifugal vs electromagnetic), but the algebraic structure is identical.

**PASS.**

---

### C.7 Toward Kerr-Newman

#### C.7.1 The Kerr-Newman Metric [CONJECTURE]

The Kerr-Newman metric describes a black hole with mass $M$, angular momentum $J = Ma$, and charge $Q$. It combines both the Kerr and Reissner-Nordstrom effects:

$$\Sigma = r^2 + a^2\cos^2\theta \qquad \text{(oblate load from rotation -- same as Kerr)}$$

$$\Delta_{\text{KN}} = r^2 - r_s r + a^2 + r_Q^2 \qquad \text{(both spin and charge resist collapse)}$$

The full line element:

$$ds^2 = -\frac{\Delta_{\text{KN}} - a^2\sin^2\theta}{\Sigma}dt^2 - \frac{2r_s r a \sin^2\theta}{\Sigma}dt\,d\phi + \frac{\Sigma}{\Delta_{\text{KN}}}dr^2 + \Sigma\,d\theta^2 + \frac{(r^2 + a^2)^2 - \Delta_{\text{KN}}a^2\sin^2\theta}{\Sigma}\sin^2\theta\,d\phi^2$$

Two horizons at:

$$r_\pm = M \pm \sqrt{M^2 - a^2 - Q^2}$$

Extremal condition: $a^2 + Q^2 = M^2$.

#### C.7.2 Lattice Interpretation Outline [CONJECTURE]

The Kerr-Newman black hole, in the lattice budget picture, is a mass that simultaneously:

1. **Saturates** nearby nodes with gravitational data (the $-r_s r$ term in $\Delta_{\text{KN}}$)
2. **Creates a vortical flux pattern** from rotation, making azimuthal budget direction-dependent (the $+a^2$ term and $g_{t\phi}$ cross-term)
3. **Generates electromagnetic anti-saturation** from charge, partially restoring budget at short range (the $+r_Q^2$ term)

The budget equation inherits all three mechanisms. The horizon condition $\Delta_{\text{KN}} = 0$ gives a quadratic with both $a^2$ and $r_Q^2$ resisting collapse:

$$r^2 - r_s r + (a^2 + r_Q^2) = 0$$

The lattice interpretation: the effective "collapse resistance" is the sum of centrifugal ($a^2$) and electromagnetic ($r_Q^2$) contributions. These are additive because they operate through independent channels -- angular momentum is a vector quantity while charge is a scalar, so their budget contributions are orthogonal.

#### C.7.3 The Born-Infeld Extension (Outline) [CONJECTURE]

A Kerr-Newman Born-Infeld Lagrangian would take the form:

$$\mathcal{L}_{\text{KN}} = -K_B \sqrt{\text{(Kerr-Newman proper time)}^2}$$

with the full Kerr-Newman proper time formula replacing the simple $\sqrt{(f^2 - v^2)/f}$. The details require careful assembly of all five budget channels (temporal, radial, polar, azimuthal, cross-term) with both charge and spin contributions, following the pattern of Part B, Section B.7.2.

This is **not** derived here. It is outlined as the natural synthesis of the Kerr (rotation) and RN (charge) extensions, indicating that the lattice budget framework can in principle accommodate the full Kerr-Newman family.

---

### C.8 The Black Hole Taxonomy

#### C.8.1 The Four Classical Solutions [THEOREM + SELECTION]

The no-hair theorem states that an uncharged black hole in GR is completely characterized by mass $M$, angular momentum $J$, and charge $Q$. The four combinations define the classical black hole taxonomy:

| Black Hole | Parameters | Horizons | Symmetry | Part | Budget Physics |
|------------|-----------|----------|----------|------|----------------|
| **Schwarzschild** | $M$ | 1 ($r_s = 2M$) | Spherical | Part A | Mass consumes budget |
| **Kerr** | $M, J$ | 2 ($r_\pm$) | Axial | Part B | Spin redirects budget (frame dragging) |
| **Reissner-Nordstrom** | $M, Q$ | 2 ($r_\pm$) | Spherical | This part | Charge restores budget (EM repulsion) |
| **Kerr-Newman** | $M, J, Q$ | 2 ($r_\pm$) | Axial | Outlined (C.7) | All three mechanisms combined |

#### C.8.2 The Unifying Budget Principle [SELECTION]

All four solutions are understood within a single interpretive framework:

> **Mass** creates gravitational data that **consumes** lattice computational budget (attractive, always present).

> **Angular momentum** creates vortical flux that **redirects** budget between azimuthal channels (frame dragging, direction-dependent).

> **Charge** creates electromagnetic field energy that **restores** budget at short range (repulsive, isotropic).

Budget is **always conserved**. The determinant of the metric is independent of the source parameters in every case. Gravity, spin, and charge can only redistribute the fixed computational capacity of the lattice -- they cannot create or destroy it.

The horizon conditions reflect the balance between these mechanisms:

| Solution | Horizon equation | Budget balance |
|----------|------------------|----------------|
| Schwarzschild | $r - r_s = 0$ | Drain = capacity (single balance point) |
| Kerr | $r^2 - r_s r + a^2 = 0$ | Drain $-$ centrifugal support = 0 (two balance points) |
| RN | $r^2 - r_s r + r_Q^2 = 0$ | Drain $-$ EM restoration = 0 (two balance points) |
| Kerr-Newman | $r^2 - r_s r + a^2 + r_Q^2 = 0$ | Drain $-$ centrifugal $-$ EM = 0 (two balance points) |

---

### C.9 Claims Table

#### C.9.1 Claims Summary

| ID | Claim | Tag | Evidence | Falsification |
|----|-------|-----|----------|---------------|
| RN-1 | RN metric $ds^2 = -f\,dt^2 + dr^2/f + r^2 d\Omega^2$ with $f = 1 - r_s/r + r_Q^2/r^2$ | [THEOREM] | Standard GR (Reissner 1916, Nordstrom 1918) | Algebraic identity -- unfalsifiable |
| RN-2 | EM field energy contributes $+r_Q^2/r^2$ to availability factor (anti-saturation) | [SELECTION] | Consistent with $T^{\text{EM}}_{\mu\nu}$ sign structure; positive energy with negative radial pressure produces repulsive metric contribution | Alternative lattice interpretation of charge |
| RN-3 | Dual-source budget: mass consumes, charge restores | [SELECTION] | Correct signs, correct limiting behavior, consistent with Schwarzschild and Kerr frameworks | Derivation from FTD axioms contradicting sign structure |
| RN-4 | Budget conservation: $g_{tt} \cdot g_{rr} = -1$ for RN | [THEOREM] | Direct computation: $(-f) \cdot (1/f) = -1$ | Algebraic identity -- unfalsifiable |
| RN-5 | $Q \to 0$ recovers Schwarzschild | [VERIFIED] | Algebraic: $f \to 1 - r_s/r$ | Algebraic -- unfalsifiable |
| RN-6 | $r \to \infty$ recovers Minkowski | [VERIFIED] | Algebraic: $f \to 1$ | Algebraic -- unfalsifiable |
| RN-7 | Extremal RN at $Q = M$: $f = (1 - M/r)^2$, degenerate horizon, zero surface gravity | [VERIFIED] | Algebraic: perfect square, $df/dr|_{r=M} = 0$, $\kappa = 0$ | Algebraic -- unfalsifiable |
| RN-8 | Born-Infeld extension: $\mathcal{L}_{\text{RN}} = -K_B\sqrt{(f^2 - v^2)/f}$ with charged $f$ | [SELECTION] | Natural generalization; same functional form as Schwarzschild BI; equivalent to geodesic action | Alternative Lagrangian formulation |
| RN-9 | EM anti-saturation: charge partially de-saturates lattice, increasing local speed limit | [SELECTION] | $f_{\text{RN}} > f_{\text{Schw}}$ when $r_Q^2/r^2$ dominates; correct physics of EM repulsion | Physical measurement contradicting |
| RN-10 | Kerr-Newman outline: $\Delta_{\text{KN}} = r^2 - r_s r + a^2 + r_Q^2$ combines both effects | [CONJECTURE] | Pattern extension from Kerr and RN; known GR result | Full lattice derivation may reveal additional structure |

#### C.9.2 Epistemic Breakdown

| Category | Count | IDs |
|----------|-------|-----|
| [THEOREM] (standard GR / algebraic identities) | 2 | RN-1, RN-4 |
| [SELECTION] (lattice interpretation) | 4 | RN-2, RN-3, RN-8, RN-9 |
| [VERIFIED] (limiting cases) | 3 | RN-5, RN-6, RN-7 |
| [CONJECTURE] (unproven extension) | 1 | RN-10 |
| [OPEN] | 0 | (Kerr-Newman full derivation is future work) |

#### C.9.3 What This Part Does NOT Claim

1. The RN metric is **derived** from FTD axioms ab initio -- it is interpreted within the lattice budget framework, not derived from it
2. Electromagnetic anti-saturation is the **unique** lattice interpretation of charge in the metric -- it is argued from $T^{\text{EM}}_{\mu\nu}$ sign structure, not proven
3. The Born-Infeld extension **predicts** anything beyond standard GR -- it reproduces known geodesic equations
4. The Kerr-Newman outline in C.7 is **complete** -- it identifies the structure but does not provide the full lattice interpretation
5. Astrophysical black holes carry significant charge -- they do not (discharge rapidly by accreting opposite-sign particles). The RN solution is primarily of theoretical importance for the completeness of the framework and for understanding the role of charge in the lattice budget picture

---

### C.10 Appendix A: The RN Budget Formula in Computational Language

For reference, the full RN proper time formula in lattice computational language:

$$\frac{d\tau}{dT_U} = \sqrt{f - \frac{v_r^2}{f}}$$

where:
- $d\tau$ = proper time (experienced $G^*$ collapse cycles)
- $dT_U$ = Universal Tick (background render rate)
- $f = 1 - r_s/r + r_Q^2/r^2$ = lattice availability (fraction of capacity remaining after gravitational drain and electromagnetic restoration)
- $v_r$ = radial lattice velocity (nodes traversed per tick)

The **three budget contributions**:

| Source | Contribution to $f$ | Lattice Mechanism |
|--------|---------------------|-------------------|
| Baseline | $+1$ | Full capacity of each node (flat space) |
| Mass | $-r_s/r = -2M/r$ | Gravitational data processing drains budget |
| Charge | $+r_Q^2/r^2 = +Q^2/r^2$ | Electromagnetic stress restores budget |

At $r = r_+$ (outer horizon), $f = 0$: the gravitational drain and electromagnetic restoration exactly cancel the baseline, leaving zero available capacity. Time stops. No information can propagate outward.

At $r < r_+$ but $r > r_-$ (between the horizons), $f < 0$: the budget is **over-consumed**. The coordinate system breaks down (as in Schwarzschild, Kruskal-like extensions are needed for the interior).

At $r = r_-$ (inner horizon), $f = 0$ again: a second balance point where the electromagnetic term, now dominant at small $r$, pulls the budget back to zero from below.

For $r < r_-$: $f > 0$ again, suggesting a region with positive budget inside the inner horizon. (The physical significance of this region is debated in the GR literature -- it may be unstable due to mass inflation at the Cauchy horizon.)

### C.11 Appendix B: Numerical Example

Consider a hypothetical charged black hole with $M = 10 M_\odot$ and $Q = 0.5 M$ (in Planck units, well below the extremal limit):

| Quantity | Value |
|----------|-------|
| $r_s$ | $2M = 20 M_\odot$ |
| $r_Q^2$ | $Q^2 = 0.25 M^2$ |
| $r_+$ | $M + \sqrt{M^2 - Q^2} = M + \sqrt{0.75}M \approx 1.866 M$ |
| $r_-$ | $M - \sqrt{0.75}M \approx 0.134 M$ |
| $r_+/r_s$ | $0.933$ (outer horizon at 93.3% of Schwarzschild radius) |

At $r = 3M$ (outside both horizons):

$$f = 1 - \frac{2M}{3M} + \frac{0.25M^2}{9M^2} = 1 - 0.667 + 0.028 = 0.361$$

Compare with Schwarzschild at the same radius: $f_{\text{Schw}} = 1 - 2/3 = 0.333$. The charge adds $0.028$ to the availability, representing an 8.3% increase in computational capacity at this distance. The lattice is slightly less saturated thanks to electromagnetic anti-saturation.

---

## Cross-References

| Document | Relationship |
|----------|-------------|
| [DERIV_BLACK_HOLE_PHYSICS.md](DERIV_BLACK_HOLE_PHYSICS.md) | Black hole thermodynamics (Hawking temperature, Bekenstein-Hawking entropy, information paradox). Complements the metric derivations here with thermodynamic content. |
| [DERIV_EINSTEIN_FIELD_EQUATIONS.md](DERIV_EINSTEIN_FIELD_EQUATIONS.md) | Full Einstein field equations from FTD. The metrics derived here are exact vacuum (Schwarzschild, Kerr) or electrovacuum (RN) solutions to those equations. |
| [DERIV_RELATIVITY_DERIVATION.md](DERIV_RELATIVITY_DERIVATION.md) | Theorem 11.1 ($g_{00}$ from flux saturation); foundational result that all three metric derivations build upon. |
| [SPEC_THE_MASTER_QUADRATIC_UNIFIED.md](../archive/ARCH_SPEC_THE_MASTER_QUADRATIC_UNIFIED.md) | G* definition, 16 forms. The G* exchange rate underpins the computational budget interpretation. |
| [SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md) | Born-Infeld Lagrangian v2.1 (Schwarzschild-exact). Extended to Kerr (Part B) and RN (Part C) here. |
| [FOUND_RELATIVITY_GRAVITY_DISTINCTION.md](../02_foundations/FOUND_RELATIVITY_GRAVITY_DISTINCTION.md) | SR / Gravity / GR trichotomy and 7-level hierarchy. The lattice metrics sit at Level 4 (metric description); the budget interpretation is Level 2-3. |
| [FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md](../02_foundations/FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md) | Historical/interpretive constant atlas. Not the canonical source for G* status or framework-integer derivation. |
| [FOUND_SPACETIME_EMERGENCE.md](../02_foundations/FOUND_SPACETIME_EMERGENCE.md) | Dimensional hierarchy, space-time ontological separation. |
| [DERIV_QFT_GRT_BRIDGE.md](DERIV_QFT_GRT_BRIDGE.md) | Linearized GR from flux; graviton propagator. The Kerr metric goes beyond the linearized regime. The RN metric demonstrates electromagnetic back-reaction on gravity. |
| [DERIV_FORCE_EMERGENCE.md](DERIV_FORCE_EMERGENCE.md) | All 4 forces from a single lattice Green's function. The RN metric provides the gravitational context in which the electromagnetic force operates. |
| [SPEC_QFT_GRT_BRIDGE_ROADMAP.md](../01_reference/SPEC_QFT_GRT_BRIDGE_ROADMAP.md) | GAP-G1 definition and critical path. GAP-1 / GAP-G1 resolved in Part A. |

---

*Consolidated document version 1.0*
*March 6, 2026*
*Framework: Foundational Ternary Dynamics v5.27*
*Consolidated from: DERIV_LATTICE_SCHWARZSCHILD.md (v1.0, Feb 19, 2026), DERIV_LATTICE_KERR.md (v1.0, Feb 24, 2026), DERIV_LATTICE_REISSNER_NORDSTROM.md (v1.0, Feb 25, 2026)*
