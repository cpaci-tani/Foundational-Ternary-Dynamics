# Reissner-Nordstrom Metric from Lattice Computational Principles

## Extending the FTD Budget Framework to Charged Black Holes

**Version:** 1.0
**Date:** February 25, 2026
**Status:** [THEOREM] + [SELECTION]
**Epistemic Tag:** The Reissner-Nordstrom metric is a known result of GR [THEOREM]. The lattice budget interpretation of electromagnetic energy as computational anti-saturation is [SELECTION]. The Born-Infeld extension is [SELECTION].

**Depends on:**

- [DERIV_LATTICE_SCHWARZSCHILD.md](DERIV_LATTICE_SCHWARZSCHILD.md) -- Schwarzschild metric from lattice computational budget (Theorem 6.1, Definition 4.1, Theorem 8.1)
- [DERIV_LATTICE_KERR.md](DERIV_LATTICE_KERR.md) -- Kerr metric from vortical flux (Sections 3--7)
- [SPEC_FTD_LAGRANGIAN.md](SPEC_FTD_LAGRANGIAN.md) -- Born-Infeld Render-Bridge Lagrangian v2.1 (Schwarzschild-exact)
- [DERIV_QFT_GRT_BRIDGE.md](DERIV_QFT_GRT_BRIDGE.md) -- Flux field as both QFT propagator and gravitational source
- [DERIV_FORCE_EMERGENCE.md](DERIV_FORCE_EMERGENCE.md) -- All 4 forces from a single lattice Green's function

> **Abstract.** This document extends the lattice computational budget framework of [DERIV_LATTICE_SCHWARZSCHILD.md](DERIV_LATTICE_SCHWARZSCHILD.md) to charged, non-rotating black holes -- the Reissner-Nordstrom (RN) solution (Reissner 1916, Nordstrom 1918). The central new idea: a charged mass creates an electromagnetic flux field whose energy density **adds** computational capacity back to the lattice, opposing the gravitational drain. Mass consumes budget (attractive); charge restores it (repulsive at short range). This dual-source budget produces the availability factor $f(r) = 1 - r_s/r + r_Q^2/r^2$, with its two horizons, extremal limit, and inner Cauchy horizon.
>
> The Reissner-Nordstrom extension closes the **[OPEN]** item noted in [SPEC_FTD_LAGRANGIAN.md](SPEC_FTD_LAGRANGIAN.md) Section 5.6 and [DERIV_LATTICE_KERR.md](DERIV_LATTICE_KERR.md) Section 8.3. Together with the Schwarzschild and Kerr derivations, the four-member Kerr-Newman family of black hole solutions is now three-quarters addressed within the lattice budget framework.
>
> A structurally significant connection emerges: the FTD Lagrangian is a Born-Infeld action, and Born-Infeld electrodynamics was originally proposed by Born and Infeld (1934) to regularize the electromagnetic self-energy of the electron. The lattice provides exactly the UV regularization that Born and Infeld sought phenomenologically -- the discrete lattice spacing $\ell_P$ cuts off the $1/r^2$ Coulomb divergence at the Planck scale.
>
> Three epistemic layers are cleanly separated: the RN metric itself [THEOREM] (standard GR), the lattice budget interpretation [SELECTION], and the Born-Infeld extension [SELECTION].

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
| **A: Mathematics** | RN line element, horizons, limiting cases, comparison with Kerr | [THEOREM], [VERIFIED] |
| **B: Lattice Interpretation** | Electromagnetic anti-saturation, dual-source budget, charge as repulsive capacity | [SELECTION] |
| **C: Lagrangian Extension** | Born-Infeld generalization with charge, Born-Infeld historical connection | [SELECTION] |

### Honesty Note

The Reissner-Nordstrom metric is a known exact solution to the Einstein-Maxwell field equations (Reissner 1916, Nordstrom 1918). This document does **not** derive the RN metric from FTD axioms ab initio. It provides a **lattice computational interpretation** for each component of the RN line element, extending the interpretive framework established for Schwarzschild in [DERIV_LATTICE_SCHWARZSCHILD.md](DERIV_LATTICE_SCHWARZSCHILD.md). The mathematical results (Layer A) are standard GR. The novel contribution is the physical interpretation (Layer B) and Lagrangian extension (Layer C).

---

# Section 1: Review of Schwarzschild and Kerr Budget Frameworks

## 1.1 The Schwarzschild Framework [THEOREM]

From [DERIV_LATTICE_SCHWARZSCHILD.md](DERIV_LATTICE_SCHWARZSCHILD.md): a spherically symmetric mass $M$ saturates the computational capacity of nearby lattice nodes. The fraction of capacity remaining is the **availability factor**:

$$f(r) = 1 - \frac{r_s}{r}$$

where $r_s = 2GM/c^2$ is the Schwarzschild radius. The complete proper time formula is:

$$\frac{d\tau}{dT_U} = \sqrt{f - \frac{v_r^2}{f}} = \sqrt{\frac{f^2 - v_r^2}{f}}$$

Key properties:
- **Budget conservation**: $g_{tt} \cdot g_{rr} = f \cdot (-1/f) = -1$. Gravity redistributes budget between temporal and spatial channels without creating or destroying it.
- **Velocity cost amplification**: radial displacement costs $v_r^2/f$, not $v_r^2$, because traversing saturated nodes requires more computational cycles.
- **Single horizon**: $f = 0$ at $r = r_s$. Complete saturation; time stops.

## 1.2 The Kerr Framework [SELECTION]

From [DERIV_LATTICE_KERR.md](DERIV_LATTICE_KERR.md): a rotating mass with angular momentum $J = Ma$ creates a **vortical flux pattern** on the lattice, making the computational budget direction-dependent. The scalar availability factor $f(r)$ is replaced by:

- $\Sigma(r,\theta) = r^2 + a^2 \cos^2\theta$ -- oblate computational load
- $\Delta(r) = r^2 - r_s r + a^2$ -- modified horizon function

The key new feature is the $g_{t\phi}$ cross-term encoding frame dragging: co-rotating information propagation is cheaper than counter-rotating, because the vortex carries flux in the preferred azimuthal direction.

Two horizons arise from $\Delta = 0$:

$$r_\pm = M \pm \sqrt{M^2 - a^2}$$

Angular momentum **resists** gravitational collapse -- the $+a^2$ term in $\Delta$ acts as centrifugal support, shrinking the outer horizon relative to Schwarzschild.

## 1.3 The Common Principle [SELECTION]

Both Schwarzschild and Kerr share a unifying principle within the lattice budget framework:

> **Gravitational data processing consumes lattice computational budget.** Each lattice node near a mass must process gravitational field data (encoding curvature, flux density, tidal information). This processing load reduces the node's available capacity for other operations -- spatial translation and internal state evolution (proper time).

The budget is **always conserved**. The determinant of the metric -- which measures the total computational volume element -- is independent of the source parameters:

| Solution | $\det(g_{\mu\nu})$ | Depends on $M$? | Depends on second parameter? |
|----------|---------------------|------------------|------------------------------|
| Schwarzschild | $-r^4 \sin^2\theta$ | No | N/A |
| Kerr | $-\Sigma^2 \sin^2\theta = -(r^2 + a^2 \cos^2\theta)^2 \sin^2\theta$ | No | No ($a$ enters through $\Sigma$, but $\Sigma$ is a coordinate function) |

The question this document addresses: **what happens when the source carries charge in addition to mass?**

---

# Section 2: Electromagnetic Field on the Lattice

## 2.1 Dual Nature of Flux [SELECTION]

In FTD, the flux field $\mathbf{J}(v,t) \in \mathbb{R}^3$ serves dual roles (see [DERIV_QFT_GRT_BRIDGE.md](DERIV_QFT_GRT_BRIDGE.md)):

| Aspect | Quantity | Role |
|--------|----------|------|
| Gravitational | $\rho = |\mathbf{J}|$ (flux magnitude) | Sources curvature; determines saturation |
| Electromagnetic | $\nabla \cdot \mathbf{J}$ (divergence), $\nabla \times \mathbf{J}$ (curl) | Sources electric/magnetic fields |

A charged mass has **both** a gravitational flux envelope (high $|\mathbf{J}|$) and an electromagnetic flux structure (nonzero $\nabla \cdot \mathbf{J}$). The gravitational aspect saturates lattice nodes (consuming budget). The electromagnetic aspect carries additional energy that also interacts with the lattice budget -- but with a crucial sign difference.

## 2.2 Electromagnetic Energy Density [THEOREM]

For a point charge $Q$ at rest, the electric field in natural units ($G = c = 4\pi\varepsilon_0 = 1$) is:

$$\mathbf{E} = \frac{Q}{r^2} \hat{r}$$

The electromagnetic energy-momentum tensor is:

$$T^{\text{EM}}_{\mu\nu} = \frac{1}{4\pi}\left(F_{\mu\alpha} F^{\alpha}_{\ \nu} - \frac{1}{4} g_{\mu\nu} F_{\alpha\beta} F^{\alpha\beta}\right)$$

For a purely electric, spherically symmetric field:

- Energy density: $T^{\text{EM}}_{00} = \frac{E^2}{8\pi} = \frac{Q^2}{8\pi r^4}$ -- **positive** (energy is stored in the field)
- Radial pressure: $T^{\text{EM}}_{rr} = -\frac{E^2}{8\pi} = -\frac{Q^2}{8\pi r^4}$ -- **negative** (radial tension)
- Tangential pressure: $T^{\text{EM}}_{\theta\theta} = T^{\text{EM}}_{\phi\phi} = +\frac{E^2}{8\pi}$ -- **positive** (tangential compression)

The crucial feature is the **sign structure**: $T^{\text{EM}}_{00} > 0$ but $T^{\text{EM}}_{rr} < 0$. The electromagnetic field has positive energy but negative radial pressure (tension). This combination, when sourcing Einstein's equations, produces a **repulsive** gravitational effect at short range.

## 2.3 Electromagnetic Budget Contribution [SELECTION]

In the lattice budget framework, the electromagnetic field energy acts as an **anti-saturation** mechanism:

> Each lattice node near a charged mass must process both gravitational data (from the mass $M$) and electromagnetic data (from the charge $Q$). The gravitational processing **consumes** computational budget, reducing the availability factor. The electromagnetic processing, due to the sign structure of $T^{\text{EM}}_{\mu\nu}$, **adds capacity back** to the availability factor at short range.

The physical intuition: the electromagnetic field's radial tension ($T^{\text{EM}}_{rr} < 0$) counteracts gravitational compression. In lattice terms, the EM field provides a mechanism for the lattice to resist complete saturation -- the charge "pushes back" against the gravitational drain on computational resources.

This is **not** a claim that electromagnetic energy is negative -- it is positive ($T^{\text{EM}}_{00} > 0$). The repulsive effect arises from the **anisotropic stress** structure of the electromagnetic energy-momentum tensor, which enters Einstein's equations through $R_{\mu\nu} - \frac{1}{2}g_{\mu\nu}R = 8\pi G \, T_{\mu\nu}$ and produces a metric contribution with the opposite sign to the mass term in $f(r)$.

## 2.4 Lattice Regularization and Born-Infeld [SELECTION]

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

# Section 3: The Reissner-Nordstrom Availability Factor

## 3.1 The RN Availability Factor [THEOREM]

The Reissner-Nordstrom solution to the coupled Einstein-Maxwell equations gives the availability factor for a mass $M$ with charge $Q$:

$$\boxed{f(r) = 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2}}$$

where:

| Parameter | Definition | Natural units ($G = c = 4\pi\varepsilon_0 = 1$) |
|-----------|------------|--------------------------------------------------|
| $r_s$ | Schwarzschild radius | $2M$ |
| $r_Q$ | Charge radius | $Q$ (i.e., $r_Q^2 = Q^2$) |

In SI units: $r_s = 2GM/c^2$ and $r_Q^2 = GQ^2/(4\pi\varepsilon_0 c^4)$.

## 3.2 Decomposition of Budget Consumption [SELECTION]

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

## 3.3 Horizon Structure [THEOREM]

Setting $f(r) = 0$:

$$r^2 - r_s r + r_Q^2 = 0$$

$$r_\pm = \frac{r_s}{2} \pm \sqrt{\left(\frac{r_s}{2}\right)^2 - r_Q^2} = M \pm \sqrt{M^2 - Q^2}$$

Three regimes:

| Condition | Horizons | Physical Regime |
|-----------|----------|-----------------|
| $Q < M$ (sub-extremal) | Two: $r_+ > r_-$ | Standard charged black hole |
| $Q = M$ (extremal) | One (degenerate): $r_+ = r_- = M$ | Extremal RN black hole |
| $Q > M$ (super-extremal) | None (complex roots) | Naked singularity (likely unphysical; cosmic censorship) |

## 3.4 Comparison with Kerr Horizons [THEOREM]

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

# Section 4: The Full RN Line Element

## 4.1 The Metric [THEOREM]

The Reissner-Nordstrom line element in Schwarzschild-like coordinates $(t, r, \theta, \phi)$, in natural units ($G = c = 1$):

$$\boxed{ds^2 = -f(r) \, dt^2 + \frac{dr^2}{f(r)} + r^2 \, d\Omega^2}$$

where:

$$f(r) = 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2}, \qquad d\Omega^2 = d\theta^2 + \sin^2\theta \, d\phi^2$$

This is **spherically symmetric** -- the charge does not break spherical symmetry because the Coulomb field of a point charge is isotropic. Unlike Kerr (where angular momentum selects a preferred axis), charge produces no directional preference.

## 4.2 Component-by-Component Lattice Interpretation [SELECTION]

| Component | Expression | Lattice Interpretation |
|-----------|------------|------------------------|
| $g_{tt} = -f(r)$ | $-(1 - r_s/r + r_Q^2/r^2)$ | **Temporal budget**: gravitational drain ($-r_s/r$) partially compensated by electromagnetic restoration ($+r_Q^2/r^2$). Clock rate reflects the net computational capacity. |
| $g_{rr} = 1/f(r)$ | $r^2/(r^2 - r_s r + r_Q^2)$ | **Radial cost amplification**: traversing nodes costs $v_r^2/f$, amplified when net saturation is high. Same mechanism as Schwarzschild, with modified $f$. |
| $g_{\theta\theta} = r^2$ | $r^2$ | **Polar cost**: unchanged from flat space. Spherical symmetry is preserved. |
| $g_{\phi\phi} = r^2\sin^2\theta$ | $r^2\sin^2\theta$ | **Azimuthal cost**: unchanged from flat space. No frame dragging (no rotation). |
| $g_{t\phi}$ | $0$ | **No cross-term**: charge is a scalar quantity with no preferred direction. No directional budget asymmetry. |

### Structural Comparison

| Feature | Schwarzschild | Kerr | Reissner-Nordstrom |
|---------|---------------|------|--------------------|
| $g_{tt}$ | $-(1 - r_s/r)$ | $-(1 - r_s r/\Sigma)$ | $-(1 - r_s/r + r_Q^2/r^2)$ |
| $g_{rr}$ | $1/(1 - r_s/r)$ | $\Sigma/\Delta$ | $1/(1 - r_s/r + r_Q^2/r^2)$ |
| Angular sector | $r^2 d\Omega^2$ | $\Sigma \, d\theta^2 + [(r^2+a^2)^2 - \Delta a^2\sin^2\theta]\sin^2\theta \, d\phi^2 / \Sigma$ | $r^2 d\Omega^2$ |
| Cross-term $g_{t\phi}$ | 0 | $-r_s r a \sin^2\theta / \Sigma$ | 0 |
| Spherical symmetry | Yes | No (axial) | Yes |

## 4.3 Budget Conservation [THEOREM]

**Theorem 4.1** (RN Budget Conservation): *The product $g_{tt} \cdot g_{rr} = -1$ for the Reissner-Nordstrom metric.*

**Proof:**

$$g_{tt} \cdot g_{rr} = \left(-f(r)\right) \cdot \frac{1}{f(r)} = -1 \qquad \blacksquare$$

This is identical to the Schwarzschild result (Theorem 8.1 of [DERIV_LATTICE_SCHWARZSCHILD.md](DERIV_LATTICE_SCHWARZSCHILD.md)). The charge parameter $r_Q$ modifies $f(r)$ but does not affect the reciprocal relationship between temporal and radial metric components.

**Lattice interpretation [SELECTION]:** Budget conservation persists in the presence of charge. The electromagnetic field redistributes computational capacity -- adding to the temporal budget and reducing the radial cost -- but the total capacity product remains fixed. Gravity, angular momentum, and charge can all redistribute the budget, but none can create or destroy it. This is the lattice expression of the diffeomorphism invariance of GR: the coordinate volume element is determined by geometry, not by source content.

The determinant of the RN metric:

$$\det(g_{\mu\nu}) = -f(r) \cdot \frac{1}{f(r)} \cdot r^2 \cdot r^2 \sin^2\theta = -r^4 \sin^2\theta$$

This is **independent of $M$ and $Q$** -- identical to the Schwarzschild and flat-space results. Budget conservation is exact.

## 4.4 The RN Proper Time Formula [THEOREM]

For a radially moving observer with coordinate velocity $v_r = dr/dt$:

$$\boxed{\frac{d\tau}{dt} = \sqrt{f - \frac{v_r^2}{f}} = \sqrt{\frac{f^2 - v_r^2}{f}}}$$

where $f = 1 - r_s/r + r_Q^2/r^2$.

This has the **same functional form** as the Schwarzschild proper time formula (Theorem 6.1 of [DERIV_LATTICE_SCHWARZSCHILD.md](DERIV_LATTICE_SCHWARZSCHILD.md)), with $f$ now containing the electromagnetic contribution. The velocity cost amplification factor $v_r^2/f$ persists -- radial traversal through charged-mass-saturated nodes costs more computational budget per unit displacement.

---

# Section 5: Born-Infeld Extension

## 5.1 The Schwarzschild-Exact Born-Infeld Core [THEOREM]

From [SPEC_FTD_LAGRANGIAN.md](SPEC_FTD_LAGRANGIAN.md), the v2.1 Born-Infeld render-bridge Lagrangian:

$$\mathcal{L}_{\text{RB}} = -K_B \frac{\sqrt{f^2 - v^2}}{\sqrt{f}}$$

reproduces the Schwarzschild proper time exactly for $f = 1 - r_s/r$.

## 5.2 Generalization to Reissner-Nordstrom [SELECTION]

The RN extension is immediate: replace $f$ with the charged availability factor:

$$\boxed{\mathcal{L}_{\text{RN}} = -K_B \frac{\sqrt{f^2 - v^2}}{\sqrt{f}}, \qquad f = 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2}}$$

The Born-Infeld structure is **unchanged** -- only the definition of $f$ is modified. This is because the RN metric has the same functional relationship between $g_{tt}$ and $g_{rr}$ as Schwarzschild: $g_{rr} = -1/g_{tt}$. The Born-Infeld core depends on $f$ through the proper time formula, and the proper time formula depends only on the relationship $g_{tt} = -f$, $g_{rr} = 1/f$.

### Equivalence to Geodesic Action [THEOREM]

As in the Schwarzschild case, $\mathcal{L}_{\text{RN}} = -K_B \sqrt{-g_{\mu\nu}\dot{x}^\mu\dot{x}^\nu / \dot{t}^2}$ for the RN metric. The Euler-Lagrange equations reproduce the RN geodesic equations exactly. This is a mathematical identity.

## 5.3 The Born-Infeld Connection [SELECTION]

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

## 5.4 Relativistic Momentum [THEOREM]

The conjugate momentum derived from $\mathcal{L}_{\text{RN}}$:

$$p_r = \frac{\partial \mathcal{L}_{\text{RN}}}{\partial v_r} = \frac{K_B \, v_r}{\sqrt{f}\,\sqrt{f^2 - v_r^2}} = K_B \, \gamma_{\text{RN}} \, v_r$$

where the RN Lorentz factor is:

$$\gamma_{\text{RN}} = \frac{\sqrt{f}}{\sqrt{f^2 - v_r^2}}, \qquad f = 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2}$$

This has the same form as the Schwarzschild Lorentz factor (Section 5.1 of [SPEC_FTD_LAGRANGIAN.md](SPEC_FTD_LAGRANGIAN.md)), with the charged availability factor. The gravitationally modified speed limit is:

$$v_r < f(r) = 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2}$$

Note that $f(r)$ can be **larger** than $1 - r_s/r$ (the uncharged value) in the region where $r_Q^2/r^2$ is significant. The electromagnetic contribution effectively **increases** the local speed limit relative to Schwarzschild at the same mass -- the charge partially de-saturates the lattice, allowing faster information propagation.

---

# Section 6: Limiting Cases

## 6.1 Case (a): $Q \to 0$ Recovers Schwarzschild [VERIFIED]

Setting $r_Q = 0$:

$$f(r) = 1 - \frac{r_s}{r} + 0 = 1 - \frac{r_s}{r}$$

This is the Schwarzschild availability factor. The single horizon at $r = r_s$ is recovered. All electromagnetic contributions vanish. The proper time formula, Born-Infeld Lagrangian, and budget conservation reduce to their Schwarzschild forms.

**PASS.**

## 6.2 Case (b): $r \to \infty$ Recovers Flat Space [VERIFIED]

As $r \to \infty$:

$$f(r) = 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2} \to 1 - 0 + 0 = 1$$

The metric becomes:

$$ds^2 \to -dt^2 + dr^2 + r^2 \, d\Omega^2$$

This is the Minkowski metric in spherical coordinates. Far from the charged mass, the lattice is unsaturated and all budget channels operate at full capacity.

**PASS.**

## 6.3 Case (c): Extremal RN ($Q = M$, i.e., $r_Q = r_s/2$) [VERIFIED]

When $Q = M$ (equivalently $r_Q = M = r_s/2$ in natural units), the two horizons merge:

$$r_+ = r_- = M = \frac{r_s}{2}$$

The availability factor becomes a perfect square:

$$f(r) = 1 - \frac{2M}{r} + \frac{M^2}{r^2} = \left(1 - \frac{M}{r}\right)^2$$

**Lattice interpretation [SELECTION]:** At the extremal limit, the electromagnetic restoration exactly balances the gravitational drain at the degenerate horizon. The availability factor touches zero ($f = 0$ at $r = M$) but does so **quadratically**, not linearly as in the Schwarzschild case. This means the approach to zero is gentler -- the gradient of $f$ vanishes at the horizon:

$$\frac{df}{dr}\bigg|_{r=M} = 0$$

In lattice terms, the computational saturation at the extremal horizon is a **saddle point**, not a cliff. The gravitational budget drain and electromagnetic budget restoration are in perfect equilibrium, creating a marginally stable horizon.

The surface gravity of an extremal RN black hole is zero ($\kappa = 0$), which implies zero Hawking temperature ($T_H = \kappa/(2\pi) = 0$). In the lattice picture: the equilibrium is so perfect that no thermal fluctuations can overcome it -- no Hawking radiation is emitted.

**PASS.**

## 6.4 Case (d): Weak Field ($r \gg r_s, r_Q$) [VERIFIED]

In the weak-field regime:

$$f(r) \approx 1 - \frac{r_s}{r} + \frac{r_Q^2}{r^2} = 1 - \frac{2GM}{rc^2} + \frac{GQ^2}{4\pi\varepsilon_0 c^4 r^2}$$

The Newtonian potential is $\Phi = -GM/r$, so $f \approx 1 + 2\Phi/c^2 + O(\Phi^2)$ with an electromagnetic correction at $O(1/r^2)$.

The proper time for a static observer:

$$\frac{d\tau}{dt} = \sqrt{f} \approx 1 - \frac{GM}{rc^2} + \frac{GQ^2}{8\pi\varepsilon_0 c^4 r^2}$$

The gravitational redshift acquires a charge-dependent correction. This correction is negligible for astrophysical objects (where $Q/M \ll 1$ in Planck units) but is significant for elementary particles (where $Q/M$ can be large).

**PASS.**

## 6.5 Case (e): Comparison with Kerr [VERIFIED]

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

# Section 7: Toward Kerr-Newman

## 7.1 The Kerr-Newman Metric [CONJECTURE]

The Kerr-Newman metric describes a black hole with mass $M$, angular momentum $J = Ma$, and charge $Q$. It combines both the Kerr and Reissner-Nordstrom effects:

$$\Sigma = r^2 + a^2\cos^2\theta \qquad \text{(oblate load from rotation -- same as Kerr)}$$

$$\Delta_{\text{KN}} = r^2 - r_s r + a^2 + r_Q^2 \qquad \text{(both spin and charge resist collapse)}$$

The full line element:

$$ds^2 = -\frac{\Delta_{\text{KN}} - a^2\sin^2\theta}{\Sigma}dt^2 - \frac{2r_s r a \sin^2\theta}{\Sigma}dt\,d\phi + \frac{\Sigma}{\Delta_{\text{KN}}}dr^2 + \Sigma\,d\theta^2 + \frac{(r^2 + a^2)^2 - \Delta_{\text{KN}}a^2\sin^2\theta}{\Sigma}\sin^2\theta\,d\phi^2$$

Two horizons at:

$$r_\pm = M \pm \sqrt{M^2 - a^2 - Q^2}$$

Extremal condition: $a^2 + Q^2 = M^2$.

## 7.2 Lattice Interpretation Outline [CONJECTURE]

The Kerr-Newman black hole, in the lattice budget picture, is a mass that simultaneously:

1. **Saturates** nearby nodes with gravitational data (the $-r_s r$ term in $\Delta_{\text{KN}}$)
2. **Creates a vortical flux pattern** from rotation, making azimuthal budget direction-dependent (the $+a^2$ term and $g_{t\phi}$ cross-term)
3. **Generates electromagnetic anti-saturation** from charge, partially restoring budget at short range (the $+r_Q^2$ term)

The budget equation inherits all three mechanisms. The horizon condition $\Delta_{\text{KN}} = 0$ gives a quadratic with both $a^2$ and $r_Q^2$ resisting collapse:

$$r^2 - r_s r + (a^2 + r_Q^2) = 0$$

The lattice interpretation: the effective "collapse resistance" is the sum of centrifugal ($a^2$) and electromagnetic ($r_Q^2$) contributions. These are additive because they operate through independent channels -- angular momentum is a vector quantity while charge is a scalar, so their budget contributions are orthogonal.

## 7.3 The Born-Infeld Extension (Outline) [CONJECTURE]

A Kerr-Newman Born-Infeld Lagrangian would take the form:

$$\mathcal{L}_{\text{KN}} = -K_B \sqrt{\text{(Kerr-Newman proper time)}^2}$$

with the full Kerr-Newman proper time formula replacing the simple $\sqrt{(f^2 - v^2)/f}$. The details require careful assembly of all five budget channels (temporal, radial, polar, azimuthal, cross-term) with both charge and spin contributions, following the pattern of [DERIV_LATTICE_KERR.md](DERIV_LATTICE_KERR.md) Section 7.2.

This is **not** derived here. It is outlined as the natural synthesis of the Kerr (rotation) and RN (charge) extensions, indicating that the lattice budget framework can in principle accommodate the full Kerr-Newman family.

---

# Section 8: The Black Hole Taxonomy

## 8.1 The Four Classical Solutions [THEOREM + SELECTION]

The no-hair theorem states that an uncharged black hole in GR is completely characterized by mass $M$, angular momentum $J$, and charge $Q$. The four combinations define the classical black hole taxonomy:

| Black Hole | Parameters | Horizons | Symmetry | FTD Document | Budget Physics |
|------------|-----------|----------|----------|--------------|----------------|
| **Schwarzschild** | $M$ | 1 ($r_s = 2M$) | Spherical | [DERIV_LATTICE_SCHWARZSCHILD.md](DERIV_LATTICE_SCHWARZSCHILD.md) | Mass consumes budget |
| **Kerr** | $M, J$ | 2 ($r_\pm$) | Axial | [DERIV_LATTICE_KERR.md](DERIV_LATTICE_KERR.md) | Spin redirects budget (frame dragging) |
| **Reissner-Nordstrom** | $M, Q$ | 2 ($r_\pm$) | Spherical | This document | Charge restores budget (EM repulsion) |
| **Kerr-Newman** | $M, J, Q$ | 2 ($r_\pm$) | Axial | Outlined (Section 7) | All three mechanisms combined |

## 8.2 The Unifying Budget Principle [SELECTION]

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

# Section 9: Claims Table

## 9.1 Claims Summary

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

## 9.2 Epistemic Breakdown

| Category | Count | IDs |
|----------|-------|-----|
| [THEOREM] (standard GR / algebraic identities) | 2 | RN-1, RN-4 |
| [SELECTION] (lattice interpretation) | 4 | RN-2, RN-3, RN-8, RN-9 |
| [VERIFIED] (limiting cases) | 3 | RN-5, RN-6, RN-7 |
| [CONJECTURE] (unproven extension) | 1 | RN-10 |
| [OPEN] | 0 | (Kerr-Newman full derivation is future work) |

## 9.3 What This Document Does NOT Claim

1. The RN metric is **derived** from FTD axioms ab initio -- it is interpreted within the lattice budget framework, not derived from it
2. Electromagnetic anti-saturation is the **unique** lattice interpretation of charge in the metric -- it is argued from $T^{\text{EM}}_{\mu\nu}$ sign structure, not proven
3. The Born-Infeld extension **predicts** anything beyond standard GR -- it reproduces known geodesic equations
4. The Kerr-Newman outline in Section 7 is **complete** -- it identifies the structure but does not provide the full lattice interpretation
5. Astrophysical black holes carry significant charge -- they do not (discharge rapidly by accreting opposite-sign particles). The RN solution is primarily of theoretical importance for the completeness of the framework and for understanding the role of charge in the lattice budget picture

---

# Section 10: Cross-References

| Document | Relationship |
|----------|-------------|
| [DERIV_LATTICE_SCHWARZSCHILD.md](DERIV_LATTICE_SCHWARZSCHILD.md) | Foundation: scalar availability $f = 1 - r_s/r$, velocity cost amplification $v^2/f$, budget conservation $g_{tt} \cdot g_{rr} = -1$. This document extends $f$ to include charge. |
| [DERIV_LATTICE_KERR.md](DERIV_LATTICE_KERR.md) | Parallel extension: Kerr adds angular momentum to Schwarzschild. This document adds charge to Schwarzschild. Section 7 outlines the combination (Kerr-Newman). The Kerr document's "Remaining [OPEN]" (Section 8.3 item 4) regarding Kerr-Newman is partially addressed here. |
| [SPEC_FTD_LAGRANGIAN.md](SPEC_FTD_LAGRANGIAN.md) | Born-Infeld Lagrangian v2.1 (Schwarzschild-exact). Section 5.6 note: "Remaining [OPEN]: Reissner-Nordstrom (charged) metric extension" is **resolved** by this document. The Born-Infeld core $\sqrt{(f^2 - v^2)/f}$ naturally accommodates the charged $f$. |
| [DERIV_QFT_GRT_BRIDGE.md](DERIV_QFT_GRT_BRIDGE.md) | Flux field as both QFT propagator and gravitational source. The RN solution demonstrates how the electromagnetic sector of the flux field ($\nabla \cdot \mathbf{J}$, $\nabla \times \mathbf{J}$) back-reacts on the gravitational sector ($|\mathbf{J}|$) through the Einstein-Maxwell coupling. |
| [DERIV_FORCE_EMERGENCE.md](DERIV_FORCE_EMERGENCE.md) | All four forces from a single lattice Green's function. The RN metric provides the gravitational context in which the electromagnetic force operates -- the electromagnetic field both sources the metric (through $T^{\text{EM}}_{\mu\nu}$) and propagates on it. |
| [DERIV_RELATIVITY_DERIVATION.md](DERIV_RELATIVITY_DERIVATION.md) | Theorem 11.1 ($g_{00}$ from flux saturation). The RN $g_{tt} = -(1 - r_s/r + r_Q^2/r^2)$ reduces to $-f$ for $Q = 0$, consistent with the original derivation. |
| [FOUND_RELATIVITY_GRAVITY_DISTINCTION.md](FOUND_RELATIVITY_GRAVITY_DISTINCTION.md) | Seven-level hierarchy. The RN metric sits at Level 4 (metric description), extending Schwarzschild to include electromagnetic energy. The lattice interpretation (anti-saturation) is Level 2-3. |

---

## Appendix A: The RN Budget Formula in Computational Language

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

## Appendix B: Numerical Example

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

*Document version 1.0 -- Reissner-Nordstrom Metric from Lattice Computational Principles*
*February 25, 2026*
*Framework: Foundational Ternary Dynamics v5.26*
