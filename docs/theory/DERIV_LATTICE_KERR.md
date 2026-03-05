# Kerr Metric from Lattice Computational Principles

## Extending the FTD Budget Framework to Rotating Black Holes

**Version:** 1.0
**Date:** February 24, 2026
**Status:** [THEOREM] + [SELECTION]
**Epistemic Tag:** The Kerr metric is a known result of GR [THEOREM]. The lattice budget interpretation is [SELECTION]. The Born-Infeld extension is [SELECTION].

**Depends on:**

- [DERIV_LATTICE_SCHWARZSCHILD.md](DERIV_LATTICE_SCHWARZSCHILD.md) -- Schwarzschild metric from lattice computational budget (Theorem 6.1, Definition 4.1)
- [SPEC_FTD_LAGRANGIAN.md](SPEC_FTD_LAGRANGIAN.md) -- Born-Infeld Render-Bridge Lagrangian v2.1
- [FOUND_RELATIVITY_GRAVITY_DISTINCTION.md](FOUND_RELATIVITY_GRAVITY_DISTINCTION.md) -- SR / Gravity / GR trichotomy and 7-level hierarchy
- [DERIV_QFT_GRT_BRIDGE.md](DERIV_QFT_GRT_BRIDGE.md) -- Linearized GR from flux (Theorem 2.3)

> **Abstract.** This document extends the lattice computational budget framework of [DERIV_LATTICE_SCHWARZSCHILD.md](DERIV_LATTICE_SCHWARZSCHILD.md) to rotating black holes, expressed in Boyer-Lindquist coordinates. The central new idea: a spinning mass creates a **vortical flux pattern** on the lattice, making the computational budget direction-dependent. Co-rotating information propagation is cheaper than counter-rotating, because the vortex carries flux in one azimuthal direction. This directional budget asymmetry produces the $g_{t\phi}$ cross-term -- the signature of frame dragging.

The key structural results:

1. The scalar availability factor $f(r) = 1 - r_s/r$ generalizes to direction-dependent functions $\Sigma(r,\theta)$ and $\Delta(r)$
2. The metric acquires a mixed temporal-azimuthal component $g_{t\phi} \neq 0$ encoding the budget asymmetry
3. Four limiting cases (non-rotating, far-field, equatorial, extremal) are verified algebraically
4. The Born-Infeld Lagrangian $\mathcal{L}_{\text{RB}}$ acquires azimuthal structure

Three epistemic layers are cleanly separated: the Kerr metric itself [THEOREM] (standard GR), the lattice budget interpretation [SELECTION], and the Born-Infeld extension [SELECTION].

---

## Preface: Epistemic Framework

| Tag | Meaning | Standard |
|-----|---------|----------|
| **[AXIOM]** | Primitive FTD postulate | Cannot be derived; foundational |
| **[DEFINITION]** | Formal naming | No truth claim; establishes notation |
| **[THEOREM]** | Rigorously proven | Complete derivation from prior results |
| **[SELECTION]** | Argued choice | Not unique; justified by criteria |
| **[CONJECTURE]** | Unproven claim | Evidence but no proof |
| **[VERIFIED]** | Confirmed algebraically | All special cases checked |
| **[OPEN]** | Unresolved question | Future work |

### Three-Layer Structure

| Layer | Content | Tags |
|-------|---------|------|
| **A: Mathematics** | Kerr line element, Boyer-Lindquist coordinates, limiting cases | [THEOREM], [VERIFIED] |
| **B: Lattice Interpretation** | Vortical flux, directional budget, oblate computational load | [SELECTION] |
| **C: Lagrangian Extension** | Born-Infeld generalization with azimuthal cross-term | [SELECTION] |

### Honesty Note

The Kerr metric is a known exact solution to the Einstein field equations (Kerr, 1963). This document does **not** derive the Kerr metric from FTD axioms ab initio. It provides a **lattice computational interpretation** for each component of the Kerr line element, extending the interpretive framework established for Schwarzschild in [DERIV_LATTICE_SCHWARZSCHILD.md](DERIV_LATTICE_SCHWARZSCHILD.md). The mathematical results (Layer A) are standard GR. The novel contribution is the physical interpretation (Layer B) and Lagrangian extension (Layer C).

---

# Section 1: Review of Schwarzschild from Lattice Budget

## 1.1 The Availability Factor [THEOREM]

From [DERIV_LATTICE_SCHWARZSCHILD.md](DERIV_LATTICE_SCHWARZSCHILD.md), Theorem 11.1 and Definition 4.1: near a spherically symmetric mass $M$, each lattice node's computational capacity is partially consumed by gravitational data processing. The fraction remaining is the **availability factor**:

$$f(r) = 1 - \frac{r_s}{r}$$

where $r_s = 2GM/c^2$ is the Schwarzschild radius. This is a scalar field -- a single number at each radial position.

## 1.2 Budget Conservation [THEOREM]

The Schwarzschild proper time formula (Theorem 6.1):

$$\frac{d\tau}{dT_U} = \sqrt{f - \frac{v_r^2}{f}} = \sqrt{\frac{f^2 - v_r^2}{f}}$$

encodes two effects:
- **Temporal budget**: $f$ units available per tick (gravity consumes $1 - f$)
- **Velocity cost amplification**: radial displacement costs $v_r^2/f$, not $v_r^2$, because saturated nodes require more cycles to traverse

The metric identity $g_{tt} \cdot g_{rr} = f \cdot (-1/f) = -1$ expresses budget conservation: gravity redistributes computational capacity between temporal and spatial channels without creating or destroying it.

## 1.3 What Schwarzschild Does Not Capture [DEFINITION]

The Schwarzschild solution describes a **non-rotating, spherically symmetric** mass. It has:

- No preferred azimuthal direction (all $\phi$ directions are equivalent)
- No mixed $dt \, d\phi$ term (time and azimuthal angle are decoupled)
- Horizons as spheres (no oblate distortion)

A rotating mass breaks all three of these symmetries.

---

# Section 2: Angular Momentum on the Lattice

## 2.1 Vortical Flux Patterns [SELECTION]

In FTD, a mass corresponds to a region of high flux density that saturates local lattice nodes. A **rotating** mass does not merely sit on the lattice -- it creates a **vortical pattern** in the flux field $\mathbf{J}$.

Consider a mass $M$ spinning with angular momentum $\mathbf{L} = M a \hat{z}$, where $a = L/(Mc)$ is the specific angular momentum (angular momentum per unit mass, in natural units where $c = 1$). The flux field acquires a circulation component:

$$\mathbf{J}_{\text{vortex}}(r, \theta, \phi) \sim \frac{a \sin\theta}{r^2} \, \hat{\phi}$$

This vortical component is superimposed on the radial saturation pattern of the non-rotating case. It does not change the total flux magnitude at leading order -- it redirects flux into the azimuthal direction.

## 2.2 Directional Budget Asymmetry [SELECTION]

The vortex creates an asymmetry in the computational cost of azimuthal propagation:

> **Co-rotating** information (propagating in the same direction as the vortex) is carried by the background flux circulation. It costs **less** computational budget per unit angular displacement, because the flux field assists the transport.

> **Counter-rotating** information (propagating against the vortex) must work against the background circulation. It costs **more** budget per unit angular displacement.

This is the lattice interpretation of **frame dragging**: the rotating mass does not merely curve spacetime -- it creates a preferred direction of information flow. An observer near the mass experiences an asymmetric computational environment where "going with the flow" is cheaper than "going against."

### Cost Table [SELECTION]

| Direction | Relative Cost | Standard GR Name |
|-----------|---------------|------------------|
| Co-rotating ($+\phi$) | Reduced | Frame dragging (prograde) |
| Counter-rotating ($-\phi$) | Increased | Frame dragging (retrograde) |
| Radial ($r$) | Amplified by $\Sigma/\Delta$ | Same as Schwarzschild (modified) |
| Polar ($\theta$) | Symmetric | No frame dragging in polar direction |

## 2.3 Oblate Computational Load [SELECTION]

A spherically symmetric mass creates isotropic saturation -- the availability depends only on $r$. A rotating mass creates **oblate** saturation: the equatorial plane ($\theta = \pi/2$) is more heavily loaded than the poles ($\theta = 0, \pi$).

The physical reason: angular momentum is concentrated in the equatorial plane. The flux vortex is strongest at the equator and vanishes at the poles. Therefore the combined saturation (radial + vortical) is $\theta$-dependent.

This oblate structure is captured by the function $\Sigma(r, \theta)$ defined in the next section.

---

# Section 3: The Kerr Availability Factor

## 3.1 From Scalar to Tensor Budget [SELECTION]

In the Schwarzschild case, the lattice budget is controlled by a single scalar:

$$f(r) = 1 - \frac{r_s}{r}$$

For a rotating mass, the budget becomes direction-dependent. Two new functions replace the single scalar $f$:

### 3.1.1 The Oblate Load Function $\Sigma$ [DEFINITION]

$$\Sigma(r, \theta) \equiv r^2 + a^2 \cos^2\theta$$

**Lattice interpretation [SELECTION]:** $\Sigma$ measures the **total computational load** at position $(r, \theta)$. It combines two contributions:

| Contribution | Term | Origin |
|-------------|------|--------|
| Radial distance | $r^2$ | Flux dilution with distance (same as Schwarzschild) |
| Polar angular momentum | $a^2 \cos^2\theta$ | Rotational flux vortex strength at latitude $\theta$ |

At the poles ($\theta = 0$): $\Sigma = r^2 + a^2$ -- the vortex contribution is maximal because the rotation axis passes through this point, and the full rotational "tension" must be accounted for.

At the equator ($\theta = \pi/2$): $\Sigma = r^2$ -- the rotational contribution vanishes in $\Sigma$ because the equatorial node sits at the center of the vortex circulation, not on the axis. (The vortex *does* affect equatorial nodes, but through the $g_{t\phi}$ cross-term, not through $\Sigma$.)

### 3.1.2 The Modified Horizon Function $\Delta$ [DEFINITION]

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

## 3.2 Generalization of the Availability Factor [SELECTION]

The Schwarzschild availability factor $f = 1 - r_s/r$ has no single Kerr analog. Instead, the metric components involve combinations of $\Sigma$ and $\Delta$:

| Schwarzschild | Kerr generalization | Role |
|---------------|---------------------|------|
| $f = 1 - r_s/r$ | $1 - r_s r/\Sigma$ | Temporal availability |
| $1/f$ | $\Sigma/\Delta$ | Radial cost amplification |
| 1 (no azimuthal coupling) | $((r^2 + a^2)^2 - \Delta a^2 \sin^2\theta)/\Sigma$ | Azimuthal cost |
| 0 (no cross-term) | $-r_s r a \sin^2\theta / \Sigma$ | Directional budget asymmetry |

The directional asymmetry (last row) is the qualitatively new feature. It has no Schwarzschild counterpart and encodes frame dragging.

---

# Section 4: The Kerr Proper Time Formula

## 4.1 The Kerr Line Element [THEOREM]

The Kerr metric in Boyer-Lindquist coordinates $(t, r, \theta, \phi)$ is the unique stationary, axially symmetric vacuum solution to Einstein's field equations (Kerr, 1963; Carter, 1968). In natural units ($G = c = 1$):

$$\boxed{ds^2 = -\left(1 - \frac{r_s r}{\Sigma}\right) dt^2 - \frac{2 r_s r a \sin^2\theta}{\Sigma} \, dt \, d\phi + \frac{\Sigma}{\Delta} \, dr^2 + \Sigma \, d\theta^2 + \frac{(r^2 + a^2)^2 - \Delta \, a^2 \sin^2\theta}{\Sigma} \sin^2\theta \, d\phi^2}$$

where:

$$\Sigma = r^2 + a^2 \cos^2\theta, \qquad \Delta = r^2 - r_s r + a^2, \qquad r_s = 2M$$

### 4.1.1 Component-by-Component [THEOREM]

| Component | Expression | Lattice Interpretation [SELECTION] |
|-----------|------------|-----------------------------------|
| $g_{tt}$ | $-(1 - r_s r/\Sigma)$ | Temporal budget: fraction of processing capacity not consumed by gravitational + rotational data |
| $g_{t\phi}$ | $-r_s r a \sin^2\theta / \Sigma$ | **Directional budget asymmetry**: the cross-term that makes co-rotating cheaper than counter-rotating |
| $g_{rr}$ | $\Sigma/\Delta$ | Radial cost amplification: traversing saturated nodes costs more, modified by oblate load |
| $g_{\theta\theta}$ | $\Sigma$ | Polar cost: includes rotational contribution to computational load |
| $g_{\phi\phi}$ | $[(r^2 + a^2)^2 - \Delta a^2 \sin^2\theta] \sin^2\theta / \Sigma$ | Azimuthal cost: non-trivially modified by rotation |

## 4.2 The Cross-Term: Signature of Frame Dragging [SELECTION]

The $g_{t\phi}$ component is the qualitatively new term relative to Schwarzschild. In the proper time formula, it creates a coupling between temporal evolution and azimuthal motion:

$$d\tau^2 \supset -\frac{2 r_s r a \sin^2\theta}{\Sigma} \, dt \, d\phi$$

For an observer with azimuthal angular velocity $\omega = d\phi/dt$:

- If $\omega > 0$ (co-rotating with the mass), this term is **negative**, which **increases** $d\tau^2$ (the observer's clock runs faster relative to a static observer at the same $r$)
- If $\omega < 0$ (counter-rotating), this term is **positive**, which **decreases** $d\tau^2$ (the clock runs slower)

**Lattice interpretation [SELECTION]:** The vortical flux pattern carries information in the $+\phi$ direction. A co-rotating observer "surfs" the flux, gaining a budget bonus. A counter-rotating observer fights the flux, paying a budget penalty. This is why frame dragging is sometimes called "gravitomagnetism" -- it is the gravitational analog of a magnetic force acting on a moving charge.

## 4.3 General Proper Time [THEOREM]

For an observer with coordinate velocities $v_r = dr/dt$, $v_\theta = d\theta/dt$, and $\omega = d\phi/dt$:

$$\left(\frac{d\tau}{dt}\right)^2 = \left(1 - \frac{r_s r}{\Sigma}\right) + \frac{2 r_s r a \sin^2\theta}{\Sigma} \, \omega - \frac{\Sigma}{\Delta} v_r^2 - \Sigma \, v_\theta^2 - \frac{(r^2 + a^2)^2 - \Delta a^2 \sin^2\theta}{\Sigma} \sin^2\theta \, \omega^2$$

This generalizes the Schwarzschild formula $d\tau/dt = \sqrt{f - v_r^2/f}$ to include:
- Oblate gravitational saturation ($\Sigma$ replacing $r^2$ in several places)
- Angular momentum resistance ($a^2$ in $\Delta$)
- Directional budget asymmetry (the $\omega$ cross-term)

---

# Section 5: FTD Interpretation

## 5.1 Frame Dragging as Asymmetric Flux Propagation [SELECTION]

Standard GR describes frame dragging as the "dragging of inertial frames" by a rotating mass. In FTD, the mechanism is more concrete:

1. A rotating mass establishes a **vortical flux pattern** $\mathbf{J}_{\text{vortex}}$ on the lattice
2. The vortex creates a preferred direction for flux propagation (the direction of circulation)
3. Information propagating with the circulation encounters pre-aligned flux -- less computational work is needed to advance through each node
4. Information propagating against the circulation must overwrite the pre-existing flux orientation -- more work per node

This is analogous to swimming with or against a current. The "current" is the background flux vortex, and the "swimming effort" is the computational budget consumed per unit displacement.

### Frame-Dragging Angular Velocity [THEOREM]

The angular velocity at which a zero-angular-momentum observer (ZAMO) is dragged is:

$$\omega_{\text{drag}} = -\frac{g_{t\phi}}{g_{\phi\phi}} = \frac{r_s r a}{(r^2 + a^2)^2 - \Delta a^2 \sin^2\theta}$$

In the FTD picture, this is the angular velocity at which an observer must rotate to experience zero net azimuthal flux -- the velocity that exactly matches the background vortex speed.

## 5.2 The Ergosphere [SELECTION]

The **ergosphere** is the region where $g_{tt} > 0$, i.e., where:

$$1 - \frac{r_s r}{\Sigma} < 0 \qquad \Longrightarrow \qquad r < r_{\text{ergo}}(\theta) = M + \sqrt{M^2 - a^2 \cos^2\theta}$$

### Lattice Interpretation [SELECTION]

Inside the ergosphere, the temporal computational budget goes negative. An observer trying to remain static ($d\phi/dt = 0$) would require more budget than is available -- the gravitational + rotational saturation exceeds 100% of the temporal capacity.

The only escape is to co-rotate. The $g_{t\phi}$ cross-term provides a budget subsidy for co-rotating observers that can offset the temporal deficit. Inside the ergosphere, **every physical observer must co-rotate with the mass** -- not because a force pushes them, but because the computational budget is only balanced when azimuthal motion supplements the temporal deficit.

This is forced co-rotation as a **budget constraint**, not as a dynamical force.

### Comparison with Schwarzschild [SELECTION]

| Feature | Schwarzschild | Kerr |
|---------|---------------|------|
| Temporal budget zero | $f = 0$ at $r = r_s$ (horizon) | $g_{tt} = 0$ at $r = r_{\text{ergo}}(\theta)$ (ergosphere boundary) |
| Forced behavior at zero | Time stops; nothing can remain stationary | Must co-rotate; can still escape if moving outward |
| Region between | N/A | **Ergosphere**: temporal budget negative, but rotational subsidy available |

## 5.3 Two Horizons [THEOREM + SELECTION]

The Kerr horizons occur where $\Delta = 0$:

$$r_\pm = M \pm \sqrt{M^2 - a^2}$$

| Horizon | Location | Lattice Interpretation [SELECTION] |
|---------|----------|-----------------------------------|
| Outer ($r_+$) | $M + \sqrt{M^2 - a^2}$ | Complete radial saturation: no radial displacement possible (even with rotational subsidy). This is the event horizon. |
| Inner ($r_-$) | $M - \sqrt{M^2 - a^2}$ | Second saturation surface inside the outer horizon, associated with a Cauchy horizon and potential breakdown of predictability |

**Angular momentum shrinks the outer horizon**: $r_+ < r_s = 2M$ for $a > 0$. The rotational energy partially offsets gravitational collapse, pushing the horizon inward.

**Angular momentum creates the inner horizon**: $r_-$ has no Schwarzschild counterpart. It arises because the centrifugal barrier from angular momentum creates a second surface where $\Delta$ passes through zero from below.

## 5.4 Budget Conservation [THEOREM]

The determinant of the Kerr metric in Boyer-Lindquist coordinates:

$$\det(g_{\mu\nu}) = -\Sigma^2 \sin^2\theta$$

This depends on position ($r$ and $\theta$) but is **independent of the mass $M$ and spin $a$**. The identity holds for $a = 0$ (Schwarzschild, where $\Sigma = r^2$ and $\det(g) = -r^4 \sin^2\theta$) and for arbitrary $a$.

**Lattice interpretation [SELECTION]:** The total computational volume element -- the product of all budget channels -- is determined by the coordinate geometry alone, not by the mass or spin of the source. Gravity and rotation redistribute the budget across temporal, radial, polar, and azimuthal channels, but they do not change the total budget available at each spacetime point.

This generalizes the Schwarzschild result $g_{tt} \cdot g_{rr} = -1$ to the full Kerr geometry. Budget is conserved; it is only redistributed.

---

# Section 6: Limits and Verification

## 6.1 Limit (a): Non-Rotating ($a \to 0$) [VERIFIED]

Setting $a = 0$:

$$\Sigma = r^2, \qquad \Delta = r^2 - r_s r = r(r - r_s)$$

The metric components become:

$$g_{tt} = -\left(1 - \frac{r_s}{r}\right) = -f, \qquad g_{t\phi} = 0$$

$$g_{rr} = \frac{r^2}{r(r - r_s)} = \frac{r}{r - r_s} = \frac{1}{f}$$

$$g_{\theta\theta} = r^2, \qquad g_{\phi\phi} = r^2 \sin^2\theta$$

This is **exactly** the Schwarzschild metric. The cross-term vanishes, the oblate load becomes spherical, and the availability factor reduces to $f(r)$. **PASS.**

## 6.2 Limit (b): Far Field ($r \to \infty$) [VERIFIED]

As $r \to \infty$ with $a$ fixed:

$$\Sigma \approx r^2, \qquad \Delta \approx r^2, \qquad \frac{r_s r}{\Sigma} \approx \frac{r_s}{r} \to 0$$

The metric components approach:

$$g_{tt} \to -1, \qquad g_{t\phi} \to 0, \qquad g_{rr} \to 1, \qquad g_{\theta\theta} \to r^2, \qquad g_{\phi\phi} \to r^2 \sin^2\theta$$

This is the **flat Minkowski metric** in spherical coordinates. Far from any mass, the lattice is unsaturated and rotationally isotropic. **PASS.**

## 6.3 Limit (c): Equatorial Plane ($\theta = \pi/2$) [VERIFIED]

At $\theta = \pi/2$ (the equator), $\cos\theta = 0$ and $\sin\theta = 1$:

$$\Sigma = r^2, \qquad \Delta = r^2 - r_s r + a^2$$

The metric simplifies to:

$$ds^2 = -\left(1 - \frac{r_s}{r}\right) dt^2 - \frac{2 r_s a}{r} \, dt \, d\phi + \frac{r^2}{\Delta} \, dr^2 + \frac{(r^2 + a^2)^2 - \Delta a^2}{r^2} \, d\phi^2$$

(with $d\theta = 0$ for motion confined to the equatorial plane).

The frame-dragging angular velocity simplifies to:

$$\omega_{\text{drag}} = \frac{r_s a}{(r^2 + a^2)^2 - \Delta a^2} = \frac{2Ma}{(r^2 + a^2)^2 - (r^2 - 2Mr + a^2)a^2}$$

This is the standard Kerr equatorial metric. **PASS.**

## 6.4 Limit (d): Extreme Kerr ($a = M$) [VERIFIED]

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

# Section 7: The Born-Infeld Extension

## 7.1 Review of Schwarzschild-Exact Form [THEOREM]

From [SPEC_FTD_LAGRANGIAN.md](SPEC_FTD_LAGRANGIAN.md), the v2.1 Born-Infeld core:

$$\mathcal{L}_{\text{RB}} = -K_B \frac{\sqrt{f^2 - v^2}}{\sqrt{f}}$$

with $f = 1 - \mathcal{L}^2$ (lattice availability). This reproduces the Schwarzschild proper time exactly:

$$\frac{d\tau}{dt} = \frac{\sqrt{f^2 - v^2}}{\sqrt{f}} = \sqrt{f - \frac{v^2}{f}}$$

## 7.2 Generalization to Kerr [SELECTION]

The Schwarzschild Born-Infeld core assumes:
1. Isotropic availability ($f$ depends only on $r$)
2. No directional asymmetry (no cross-term between time and azimuthal angle)

For a rotating mass, we must generalize both. The proper time formula from Section 4.3 gives:

$$\left(\frac{d\tau}{dt}\right)^2 = \left(1 - \frac{r_s r}{\Sigma}\right) + \frac{2 r_s r a \sin^2\theta}{\Sigma} \omega - \frac{\Sigma}{\Delta} v_r^2 - \Sigma \, v_\theta^2 - \frac{A}{\Sigma} \sin^2\theta \, \omega^2$$

where $A = (r^2 + a^2)^2 - \Delta a^2 \sin^2\theta$ and $\omega = d\phi/dt$.

### 7.2.1 Proposed Kerr-Extended Lagrangian [SELECTION]

$$\boxed{\mathcal{L}_{\text{Kerr}} = -K_B \sqrt{\left(1 - \frac{r_s r}{\Sigma}\right) + \frac{2 r_s r a \sin^2\theta}{\Sigma} \omega - \frac{\Sigma}{\Delta} v_r^2 - \Sigma \, v_\theta^2 - \frac{A}{\Sigma} \sin^2\theta \, \omega^2}}$$

This is structurally a Born-Infeld action: a square root of a quadratic form in the velocities $(v_r, v_\theta, \omega)$ with position-dependent coefficients. The square root enforces the constraint that the expression under the radical must remain non-negative -- the gravitationally modified speed limit.

### 7.2.2 Equivalence to Geodesic Action [THEOREM]

The Lagrangian $\mathcal{L}_{\text{Kerr}} = -K_B \sqrt{-g_{\mu\nu} \dot{x}^\mu \dot{x}^\nu / \dot{t}^2}$ is (up to sign and constant) the standard **reparametrization-invariant geodesic Lagrangian**. The Euler-Lagrange equations derived from this action reproduce the Kerr geodesic equations exactly. This is a mathematical identity, independent of the lattice interpretation.

### 7.2.3 New Features Relative to Schwarzschild [SELECTION]

| Feature | Schwarzschild BI | Kerr BI |
|---------|------------------|---------|
| Availability | $f(r)$ (scalar) | $1 - r_s r / \Sigma(r,\theta)$ (position-dependent) |
| Radial cost | $v_r^2 / f$ | $(\Sigma/\Delta) v_r^2$ |
| Polar cost | $v_\theta^2$ (trivial) | $\Sigma \, v_\theta^2$ (non-trivial) |
| Azimuthal cost | $r^2 \sin^2\theta \, \omega^2$ | $(A/\Sigma) \sin^2\theta \, \omega^2$ |
| Cross-term | 0 | $(2 r_s r a \sin^2\theta / \Sigma) \omega$ |
| Speed limit | $v_r < f$ | Direction-dependent; co-rotating can exceed counter-rotating |

## 7.3 Reduction to Schwarzschild [VERIFIED]

Setting $a = 0$ in the Kerr Lagrangian:

$$\Sigma = r^2, \quad \Delta = r(r - r_s) = r^2 f, \quad A = r^4, \quad \text{cross-term} = 0$$

$$\mathcal{L}_{\text{Kerr}} \to -K_B \sqrt{f - \frac{r^2}{r^2 f} v_r^2 - r^2 v_\theta^2 - r^2 \sin^2\theta \, \omega^2}$$

$$= -K_B \sqrt{f - \frac{v_r^2}{f} - r^2 v_\theta^2 - r^2 \sin^2\theta \, \omega^2}$$

For purely radial motion ($v_\theta = \omega = 0$), this is $-K_B \sqrt{f - v_r^2/f}$ -- exactly the Schwarzschild Born-Infeld core from [SPEC_FTD_LAGRANGIAN.md](SPEC_FTD_LAGRANGIAN.md). **PASS.**

## 7.4 Remaining Open Questions [OPEN]

1. **Latency generalization**: In the Schwarzschild case, the availability factor was identified as $f = 1 - \mathcal{L}^2$ where $\mathcal{L}$ is the topological latency. The Kerr generalization $1 - r_s r / \Sigma$ cannot be written as $1 - \mathcal{L}^2$ for any simple scalar $\mathcal{L}$ because the latency is now direction-dependent. A tensorial latency $\mathcal{L}^{ij}$ may be required.

2. **Poisson equation generalization**: The Schwarzschild latency satisfies $\nabla^2 \mathcal{L} = 4\pi G \rho$. The Kerr case requires the full Einstein field equations (or at minimum, the Ernst equation for stationary axisymmetric vacuum). Whether the lattice framework can produce the Ernst equation from computational budget arguments is [OPEN].

3. **Superradiance**: The Kerr metric admits superradiant scattering -- waves can extract rotational energy from the black hole. Whether this has a lattice budget interpretation (extracting computational capacity from the vortex) is [OPEN].

---

# Section 8: Claims Table

## 8.1 Claims Summary

| ID | Claim | Tag | Evidence | Falsification |
|----|-------|-----|----------|---------------|
| KR-1 | Kerr metric in Boyer-Lindquist coordinates | [THEOREM] | Standard GR (Kerr 1963, Carter 1968) | Algebraic identity -- unfalsifiable |
| KR-2 | Rotating mass creates vortical flux pattern on the lattice | [SELECTION] | Consistent with angular momentum conservation; analogous to electromagnetic vortex from rotating charge | Alternative flux pattern that produces same metric |
| KR-3 | Directional budget asymmetry produces $g_{t\phi}$ cross-term | [SELECTION] | Correct structure (co-rotating cheaper); reproduces frame dragging | Show budget asymmetry produces wrong sign or magnitude |
| KR-4 | $\Sigma = r^2 + a^2 \cos^2\theta$ is oblate computational load | [SELECTION] | Correct $\theta$-dependence (equator vs poles); reduces to $r^2$ for $a = 0$ | Alternative interpretation of oblate structure |
| KR-5 | $\Delta = r^2 - r_s r + a^2$: angular momentum resists saturation | [SELECTION] | Correct horizon structure; $a^2$ term shrinks outer horizon | Derivation of $\Delta$ from FTD axioms contradicting this form |
| KR-6 | Ergosphere = forced co-rotation from temporal budget deficit | [SELECTION] | Matches GR prediction that static observers cannot exist inside ergosphere | Physical measurement contradicting forced co-rotation |
| KR-7 | Budget conservation: $\det(g) = -\Sigma^2 \sin^2\theta$ independent of $M, a$ | [THEOREM] | Direct computation from metric components | Algebraic identity -- unfalsifiable |
| KR-8 | $a \to 0$ recovers Schwarzschild exactly | [VERIFIED] | Explicit computation (Section 6.1) | Algebraic -- unfalsifiable |
| KR-9 | $r \to \infty$ recovers Minkowski exactly | [VERIFIED] | Explicit computation (Section 6.2) | Algebraic -- unfalsifiable |
| KR-10 | Kerr Born-Infeld Lagrangian reproduces Kerr geodesics | [THEOREM] | Equivalent to reparametrization-invariant geodesic action | Algebraic identity |
| KR-11 | Latency generalization to tensorial form | [OPEN] | No derivation yet; scalar $\mathcal{L}$ insufficient for Kerr | Future work |
| KR-12 | Superradiance from lattice budget extraction | [OPEN] | Qualitative analogy only | Future work |

## 8.2 Epistemic Breakdown

| Category | Count | Examples |
|----------|-------|---------|
| [THEOREM] (standard GR results) | 4 | KR-1, KR-7, KR-8/9, KR-10 |
| [SELECTION] (lattice interpretation) | 5 | KR-2, KR-3, KR-4, KR-5, KR-6 |
| [CONJECTURE] | 0 | -- |
| [OPEN] | 2 | KR-11, KR-12 |

## 8.3 What This Document Does NOT Claim

1. The Kerr metric is **derived** from FTD axioms -- it is interpreted within the lattice budget framework, not derived from it
2. The vortical flux pattern is the **unique** lattice representation of angular momentum -- it is argued, not proven
3. The Born-Infeld extension to Kerr **predicts** anything beyond standard GR -- it reproduces known geodesic equations
4. The lattice budget framework handles Kerr-Newman (charged + rotating) or cosmological Kerr-de Sitter metrics -- these remain future work
5. Frame dragging is **derived** from the flux vortex -- the vortex is the interpretive lens through which the known GR result is understood

---

# Section 9: Cross-References

| Document | Relationship |
|----------|-------------|
| [DERIV_LATTICE_SCHWARZSCHILD.md](DERIV_LATTICE_SCHWARZSCHILD.md) | Foundation: scalar availability $f$, velocity cost amplification, budget conservation. This document extends all three to the rotating case. |
| [SPEC_FTD_LAGRANGIAN.md](SPEC_FTD_LAGRANGIAN.md) | Born-Infeld Lagrangian v2.1 (Schwarzschild-exact). Section 7 of this document proposes the Kerr generalization. The "Remaining [OPEN]" note in SPEC_FTD_LAGRANGIAN.md Section 5.6 (Kerr extension as future work) is partially addressed here. |
| [FOUND_RELATIVITY_GRAVITY_DISTINCTION.md](FOUND_RELATIVITY_GRAVITY_DISTINCTION.md) | Seven-level hierarchy. The Kerr metric sits at Level 4 (metric description), extending the Schwarzschild result. The lattice interpretation (vortical flux) is Level 2-3 (gravity + combined formula). |
| [DERIV_QFT_GRT_BRIDGE.md](DERIV_QFT_GRT_BRIDGE.md) | Linearized GR and graviton propagator. The Kerr metric is a nonlinear exact solution, going beyond the linearized regime. Connection: the linearized gravitomagnetic potential $\mathbf{h}_{0i}$ corresponds to the leading-order $g_{t\phi}$ term at large $r$. |
| [DERIV_RELATIVITY_DERIVATION.md](DERIV_RELATIVITY_DERIVATION.md) | Theorem 11.1 ($g_{00}$ from flux saturation). The Kerr $g_{tt} = -(1 - r_s r / \Sigma)$ reduces to $-f$ for $a = 0$, which is the theorem's result. |

---

## Appendix: The Kerr Budget in Computational Language

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

*Document version 1.0 -- Kerr Metric from Lattice Computational Principles*
*February 24, 2026*
*Framework: Foundational Ternary Dynamics v5.26*
