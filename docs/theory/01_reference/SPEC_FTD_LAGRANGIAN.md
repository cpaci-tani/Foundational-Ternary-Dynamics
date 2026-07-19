# The FTD Action

## A Lattice Field Theory Unifying Quantum Propagators and Gravitational Potentials

**Status:** [THEOREM] (action, EOM, limits) + [SELECTION] (physical identifications)

---

## §1. Axioms

> **Axiom 1 (The Lattice).** The kinematic arena is the cubic graph $\Lambda$ with no defined boundary, equipped with Moore neighborhood adjacency ($N(\mathbf{v}) = \{\mathbf{u} : \|\mathbf{u}-\mathbf{v}\|_\infty = 1\}$, $|N| = 26$); at every specified position the 26 Moore-adjacent sites exist. At each vertex $\mathbf{v} \in \Lambda$ and discrete time $t \in \mathbb{N}$, there exist:
> - A **flux field** $\mathbf{J}(\mathbf{v}, t) \in \mathbb{R}^3$ (vector-valued),
> - A **state field** $s(\mathbf{v}, t) \in \{-1, 0, +1\}$ (ternary-valued),
> - A **latency field** $\mathcal{L}(\mathbf{v}) \in [0, 1)$ (scalar, quasi-static).
>
> The dynamics of $\mathbf{J}$ and $s$ are determined by the action principle (§3). The maximum propagation speed is $C = 1$ vertex per tick.

> **Calibration declaration (FTD-0041, gauge per FTD-0137).** One lattice spacing is *declared* to equal one Planck length: $a \equiv \ell_P = \sqrt{\hbar G/c^3} \approx 1.616 \times 10^{-35}$ m. This is **not an axiom** — the FTD postulates (P1-P5 in `SPEC_FTD.md`) do not specify a physical scale for one voxel; per FTD-0137, the lattice spacing is a **gauge degree of freedom** undetermined by the framework. The Planck-primary calibration is one of four defensible gauge choices (see `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` §4) and is retained as the default for backward compatibility. Under this gauge, the Planck mass $M_P = \sqrt{\hbar c/G} \approx 1.221 \times 10^{19}$ GeV becomes the natural mass scale.

**Structural-axiom count:** One (the lattice + fields + locality content). Dimensional content enters through the calibration declaration above (gauge), not through additional axioms. No dimensionless free parameters.

| Item | Content | What it provides |
|-------|---------|-----------------|
| Axiom 1 | Lattice + fields + locality | UV finiteness, Lorentz invariance (arbitrarily fine spacing), gauge structure |
| Calibration (gauge) | $a \equiv \ell_P$ (declared, not derived; FTD-0137) | Dimensional translation to SI units; dimensionless predictions are calibration-independent |

---

## §2. Mathematical Constants

### 2.1 From the Lattice Geometry

The **packing fraction** of the inscribed circle on each lattice face:

$$\text{PF} = \frac{\pi}{4} = 0.78540\ldots$$

### 2.2 From Pure Mathematics

The **lemniscate constant** — the half-period of the lemniscate of Bernoulli ($r^2 = \cos 2\theta$):

$$\varpi = \frac{\Gamma(1/4)^2}{2\sqrt{2\pi}} = 2.62206\ldots$$

### 2.3 The Bridge Constant

$$G^* \;=\; \frac{\varpi}{\sqrt{\smash[b]{\text{PF}}}} \;=\; \frac{2\varpi}{\sqrt{\pi}} \;=\; \frac{\sqrt{2}\;\Gamma(1/4)^2}{2\pi} \;=\; 2.95868\ldots$$

### 2.4 The Master Quadratic

$$x^2 - 16\,G^{*2}\,x + 16\,G^{*3} = 0$$

**Coefficient 16.** On the minimal lattice cell (cube with 8 vertices), the flux field has $8 \times 3 = 24$ components. The Gauss constraint $\nabla_L \cdot \mathbf{J} = \rho$ removes 7 degrees of freedom (one per interior dual vertex), and one overall gauge mode is unphysical. The count of physical degrees of freedom is:

$$k_\text{phys} = 24 - 7 - 1 = 16 = 2^{D+1}\big|_{D=3}$$

**Roots.** By the quadratic formula:

$$x_\pm = 8G^{*2} \pm \sqrt{64G^{*4} - 16G^{*3}}$$

$$x_+ = 137.0362\ldots, \qquad x_- = 3.0240\ldots$$

### 2.5 The Fine-Structure Constant [SELECTION: SP4]

The electromagnetic coupling constant is identified as:

$$\alpha = \frac{1}{x_+} = 0.0072974\ldots$$

**CODATA 2022:** $\alpha^{-1} = 137.035999177(21)$. **Agreement: 1.26 ppm.**

This identification is labeled [SELECTION] because no physical mechanism connecting elliptic-curve geometry to gauge couplings has been established. The numerical coincidence (1.26 ppm with zero adjustable parameters) motivates the identification but does not prove it.

---

## §3. The Action

### 3.1 Lattice Derivatives

On the lattice $\Lambda$ with spacing $a = 1$ (lattice units), define the forward difference operators:

$$(\Delta_\mu \mathbf{J})(\mathbf{v}, t) = \mathbf{J}(\mathbf{v} + \hat{\mu}, t) - \mathbf{J}(\mathbf{v}, t), \qquad \mu \in \{1,2,3\}$$

$$(\Delta_t \mathbf{J})(\mathbf{v}, t) = \mathbf{J}(\mathbf{v}, t+1) - \mathbf{J}(\mathbf{v}, t)$$

The lattice divergence and squared gradient:

$$(\nabla_L \cdot \mathbf{J})(\mathbf{v}) = \sum_{\mu=1}^{3} (\Delta_\mu J_\mu)(\mathbf{v})$$

$$|\Delta_t \mathbf{J}|^2 = \sum_{a=1}^{3} (\Delta_t J_a)^2, \qquad |\nabla_L \mathbf{J}|^2 = \sum_{a=1}^{3}\sum_{\mu=1}^{3} (\Delta_\mu J_a)^2$$

### 3.2 Derived Kinematic Quantities

Define the **normalized velocity** and **availability factor** at each vertex:

$$v(\mathbf{v}, t) \;=\; \frac{|\Delta_t \mathbf{J}(\mathbf{v}, t)|}{K_B}, \qquad f(\mathbf{v}) \;=\; 1 - \mathcal{L}(\mathbf{v})^2$$

where $K_B$ is the manifestation threshold (see §3.3), and the velocity $v$ is measured in units of $K_B$ (the rest energy scale). The constraint $v < f$ is the lattice speed limit.

### 3.3 The Action Functional

The total action is a sum over all lattice sites and time steps:

$$S[\mathbf{J}, s, \mathcal{L}] = \sum_{\mathbf{v} \in \Lambda}\;\sum_{t=0}^{T-1} \mathcal{L}_{\text{matter}}(\mathbf{v}, t) \;+\; \sum_{\mathbf{v} \in \Lambda} \mathcal{L}_{\text{grav}}(\mathbf{v})$$

with the **matter Lagrangian density**:

$$\boxed{\mathcal{L}_{\text{matter}} = -K_B \frac{\sqrt{f^2 - v^2}}{\sqrt{f}} \;+\; g_c \cdot s \cdot (\nabla_L \cdot \mathbf{J}) \;-\; \lambda_G\,(\nabla_L \cdot \mathbf{J} - \rho)^2}$$

**Amendment of record (2026-07-18) — electric coupling sign.** The coupling term's original sign, $-g_c\,s\,(\nabla_L\cdot\mathbf{J})$, was in internal conflict with the Gauss constraint term: its Hamiltonian contribution ($+g_c\,s\,\nabla_L\cdot\mathbf{J}$) is minimized by $s$-*anti*-correlated divergence, while the constraint term demands $\nabla_L\cdot\mathbf{J} = \rho \propto s$. The two terms of the same action preferred opposite signs of $s\,(\nabla_L\cdot\mathbf{J})$ at every charge site; the live engine settled the compromise at $-0.095$ of the Gauss target — wrong-signed flux (inward at a $+1$ charge; measured in `engine/tests/test_gauss_law_fidelity.cpp`, 2026-07-16). With the amended sign the Euler-Lagrange source in `phase_read` becomes $-g_c\nabla_L s$ (outward at a positive charge), both interaction terms prefer the same constraint manifold, and the live equilibrium is constraint-aligned ($f = +0.114$ at shipping defaults, same test, 2026-07-18). The Gauss projector's fixed point is untouched by the amendment (its unit-charge self-energy remains $W_{SC}(L)$ exactly — the frozen §9.1 prediction of `EXPLR_VOXEL_NEIGHBORHOOD_DYNAMICS.md` is upheld). Residual enforcement magnitude (0.114 vs 1.0) is limited by the leapfrog's `wave_vel` longitudinal reservoir, which the flux-only projector does not clean — completing enforcement would require projecting the velocity field's longitudinal sector as well, a separate scope decision recorded as [OPEN]. Downstream formulas in this document carry the amended sign.

and the **gravitational sector**:

$$\boxed{\mathcal{L}_{\text{grav}} = -\frac{1}{8\pi G}\,|\nabla_L \mathcal{L}|^2}$$

### 3.4 The Constants

| Symbol | Name | Definition | Value | Tag |
|--------|------|-----------|-------|-----|
| $K_B$ | Manifestation threshold | $M_P\sqrt{2\pi}\,(16/3)\,\alpha^{11}$ | 0.511 MeV ($= m_e$) | [THEOREM] |
| $g_c$ | Coupling constant | $\sqrt{\alpha}$ | 0.0854 | [THEOREM] |
| $G$ | Newton's constant | $\alpha_G\,\hbar c\,/\,m_e^2$ | $6.674 \times 10^{-11}$ m³kg⁻¹s⁻² | [THEOREM] |
| $\lambda_G$ | Constraint strength | $\to \infty$ (primary constraint) | — | [AXIOM] |

**Note on $\lambda_G$:** In the limit $\lambda_G \to \infty$, the third term enforces the primary constraint $\nabla_L \cdot \mathbf{J} = \rho$ exactly. We work in the constrained theory throughout.

### 3.5 The Three Terms

| Term | Expression | Role |
|------|-----------|------|
| Born-Infeld core | $-K_B\sqrt{(f^2-v^2)/f}$ | Kinetic + gravitational + rest energy; enforces $v < f$ |
| State-flux coupling | $+g_c\,s\,(\nabla_L \cdot \mathbf{J})$ | Source/sink for gauge interactions (sign amended 2026-07-18, see §3.3) |
| Gauss constraint | $-\lambda_G(\nabla_L \cdot \mathbf{J} - \rho)^2$ | Enforces charge conservation → U(1) gauge symmetry |

### 3.6 Engine Decomposition (6 Active Terms + Dissipation)

The analytical action (§3.3) is implemented in the simulation engine as six independently tracked terms plus a non-conservative dissipation function. This decomposition separates the **field sector** (wave propagation energy), the **particle sector** (Born-Infeld rest mass), and the **interaction sector** (coupling + constraint):

1. **Born-Infeld core** (Particle sector): $-K_B\sqrt{(f^2-v^2)/f}$ — `born_infeld_term()`
2. **State-flux coupling — electric** (Interaction): $+g_c\,s\,(\nabla_L \cdot \mathbf{J})$ — `coupling_term()` (sign amended 2026-07-18, see §3.3)
3. **Velocity coupling — magnetic** (Interaction): $-g_c\,s\,(\mathbf{v} \cdot \mathbf{J})$ — `velocity_coupling_term()`
4. **Gauss constraint** (Constraint): $-\lambda_G\,(\nabla_L \cdot \mathbf{J} - \rho)^2$ — `gauss_term()`
5. **Field kinetic energy** (Field sector): $\tfrac{1}{2}\lVert\Delta_t\mathbf{J}\rVert^2$ — `field_kinetic_term()`
6. **Field gradient energy** (Field sector): $-\tfrac{1}{2}c^2\sum_\mu w_\mu\lVert\Delta_\mu\mathbf{J}\rVert^2$ — `field_gradient_term()`

**Dissipation (non-conservative):**

$$R = \frac{\alpha}{2}\,|\mathbf{v}_\text{wave}|^2 \qquad [\text{IMPOSED}]$$

where $\mathbf{v}_\text{wave} = \Delta_t\mathbf{J}$ is the wave velocity (canonical momentum of the flux field), and the dissipation rate $\gamma = \alpha$ is identified with the fine-structure constant. This is a **parameter choice** (ASSUMP.6), not a derivation — see `ontic.h` Layer 6b. The motivation is that manifested particles negotiating discrete lattice geometry each tick lose energy at a rate governed by the state-flux coupling strength $g_c = \sqrt{\alpha}$.

**Relationship to the analytical action:**

- Terms 5 and 6 (field sector) emerge from the weak-field ($v \ll 1$) expansion of the Born-Infeld core (§5.4, Klein-Gordon limit). In the engine, these are tracked as independent diagnostic quantities but are not double-counted with Term 1.
- Term 3 (velocity coupling) is the magnetic counterpart of Term 2. Its Euler-Lagrange equation produces the lattice Lorentz force $\mathbf{F} = g_c\,q\,(\mathbf{v} \times \nabla_L \times \mathbf{J})$, which maps to $\mathbf{F} = q(\mathbf{v} \times \mathbf{B})$ for arbitrarily fine lattice spacing. Term 3 vanishes for stationary particles ($\mathbf{v} = 0$).
- The Rayleigh dissipation $R$ is not part of the action $S$ but enters through the dissipative Euler-Lagrange equations: $\frac{d}{dt}\frac{\partial L}{\partial \dot{q}} - \frac{\partial L}{\partial q} = -\frac{\partial R}{\partial \dot{q}}$.

**Euler-Lagrange verification:** The engine provides `compute_el_residual()` which independently recomputes the field equation of motion from the Lagrangian and compares against the stored `delta_j_` buffer after `phase_read()`. The residual is typically $\sim 10^{-15}$ (machine epsilon), confirming exact correspondence between the action and the tick cycle.

### 3.7 The Bandwidth Constraint

$$v < f = 1 - \mathcal{L}^2$$

- **Flat space** ($\mathcal{L} = 0$): $v < 1$ — the standard speed limit.
- **Near a mass** ($\mathcal{L} > 0$): $v < f < 1$ — gravitationally reduced speed limit.
- **At the horizon** ($\mathcal{L} \to 1$): $f \to 0$, coordinate velocity must vanish.

This is the **equivalence principle** made manifest: velocity ($v$) and gravity ($\mathcal{L}$) draw from the same bandwidth budget.

---

## §4. Equations of Motion

### 4.1 Variation with Respect to $\mathbf{J}$ [THEOREM]

The Born-Infeld core, viewed as a function of $v = |\Delta_t\mathbf{J}|/K_B$, has:

$$\frac{\partial \mathcal{L}_\text{matter}}{\partial (\Delta_t J_a)} = \frac{\partial}{\partial (\Delta_t J_a)}\left[-K_B\sqrt{\frac{f^2-v^2}{f}}\right]$$

Computing the chain rule with $v^2 = |\Delta_t\mathbf{J}|^2/K_B^2$:

$$\frac{\partial v^2}{\partial (\Delta_t J_a)} = \frac{2\,\Delta_t J_a}{K_B^2}$$

$$\frac{\partial}{\partial v^2}\left[-K_B\sqrt{\frac{f^2-v^2}{f}}\right] = \frac{K_B}{2\sqrt{f}\,\sqrt{f^2-v^2}}$$

Therefore the **canonical momentum** conjugate to $J_a$ is:

$$p_a = \frac{\partial \mathcal{L}}{\partial (\Delta_t J_a)} = \frac{\Delta_t J_a}{\sqrt{f}\,\sqrt{f^2-v^2}}$$

In vector form:

$$\mathbf{p} = \frac{\Delta_t\mathbf{J}/K_B}{\sqrt{f}\,\sqrt{f^2-v^2}} = \gamma_{\text{FTD}}\,\mathbf{v}$$

where the **FTD Lorentz factor** is:

$$\gamma_{\text{FTD}} = \frac{\sqrt{f}}{\sqrt{f^2-v^2}}$$

The Euler-Lagrange equation $\Delta_t p_a = -\partial\mathcal{L}/\partial J_a$ yields:

$$\Delta_t\!\left[\gamma_\text{FTD}\,\mathbf{v}\right] = -\frac{1}{K_B}\left[-g_c\,s\,\nabla_L(\cdot) + 2\lambda_G(\nabla_L\cdot\mathbf{J}-\rho)\nabla_L(\cdot)\right]$$

In the weak-field limit ($v \ll 1$, $\mathcal{L} \ll 1$, so $f \approx 1$ and $\gamma \approx 1$):

$$K_B\,\Delta_t\mathbf{v} \approx \mathbf{F}$$

This is **Newton's second law** with mass $m = K_B$. The manifestation threshold is the inertial mass. $\square$

### 4.2 Variation with Respect to $\mathcal{L}$ [THEOREM]

The latency field $\mathcal{L}$ appears in both the matter and gravitational sectors. The total Lagrangian density at each vertex is:

$$\mathcal{L}_\text{total} = -K_B\sqrt{\frac{f^2-v^2}{f}} - \frac{1}{8\pi G}\,|\nabla_L\mathcal{L}|^2 + \ldots$$

The variation $\delta S / \delta \mathcal{L} = 0$ gives two contributions.

**From the matter term:** Using $f = 1 - \mathcal{L}^2$ and $\partial f/\partial\mathcal{L} = -2\mathcal{L}$:

$$\frac{\partial \mathcal{L}_\text{matter}}{\partial \mathcal{L}} = K_B\,\frac{\mathcal{L}(f^2+v^2)}{f^{3/2}\sqrt{f^2-v^2}}$$

In the static, weak-field limit ($v = 0$, $\mathcal{L} \ll 1$): this reduces to $K_B\,\mathcal{L} \approx \rho_\text{mass}\,\mathcal{L}$, where $\rho_\text{mass} = K_B\,n$ is the mass density ($n$ = number density of manifested sites).

**From the gravitational term:** Standard lattice Laplacian variation:

$$\frac{\delta}{\delta\mathcal{L}(\mathbf{v})}\left[-\frac{1}{8\pi G}\sum_\mathbf{u}|\nabla_L\mathcal{L}|^2\right] = \frac{1}{4\pi G}\,\nabla_L^2\mathcal{L}(\mathbf{v})$$

**The field equation:** Setting the total variation to zero and taking the weak-field limit:

$$\nabla_L^2\,\mathcal{L} = 4\pi G\,\rho_\text{mass}$$

**This is Poisson's equation**, derived from the action — not postulated. For arbitrarily fine lattice spacing, it is the time-time component of the linearized Einstein equations. $\square$

### 4.3 Proper Time [THEOREM]

The proper time per coordinate tick follows from the Born-Infeld core. The action of a free particle ($s = 0$, no constraint term) is $S = -K_B \sum_t \sqrt{(f^2-v^2)/f}$. By the Clock Hypothesis (which v3 audit [AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md](../03_derivations/archive/AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md) proved is an independent, non-derivable coordinate-level **[AXIOM]** incompatible with Scale 0 substrate primitives), $d\tau \propto \sqrt{(f^2-v^2)/f}\,dt$, giving:

$$\frac{d\tau}{dt} = \frac{1}{\gamma_\text{FTD}} = \frac{\sqrt{f^2-v^2}}{\sqrt{f}} = \sqrt{f - \frac{v^2}{f}}$$

With $f = 1 - r_s/r$ (Schwarzschild identification), this is **exactly** the proper time of the Schwarzschild metric:

$$ds^2 = f\,c^2\,dt^2 - \frac{dr^2}{f} - r^2\,d\Omega^2 \implies \frac{d\tau}{dt}\bigg|_\text{radial} = \sqrt{f - \frac{\dot{r}^2}{f\,c^2}}$$

The agreement is exact for all $f \in (0, 1]$ and all $v \in [0, f)$. $\square$

---

## §5. Limiting Cases [THEOREM]

### 5.1 Special Relativity ($\mathcal{L} = 0 \implies f = 1$)

$$\mathcal{L}_\text{matter} \to -K_B\sqrt{1-v^2}, \qquad \gamma_\text{FTD} = \frac{1}{\sqrt{1-v^2}} = \gamma_\text{SR}$$

All of special relativity follows: time dilation, length contraction, relativistic momentum $\mathbf{p} = m\gamma\mathbf{v}$, energy $E = m\gamma c^2$.

### 5.2 Gravitational Time Dilation ($v = 0$)

$$\frac{d\tau}{dt} = \sqrt{f} = \sqrt{1-\mathcal{L}^2}$$

With $\mathcal{L}^2 = r_s/r$: $d\tau/dt = \sqrt{1 - r_s/r}$. This is standard Schwarzschild time dilation.

### 5.3 Full Schwarzschild (arbitrary $v$, $\mathcal{L}$)

$$\frac{d\tau}{dt} = \sqrt{f - \frac{v^2}{f}}, \qquad f = 1 - \frac{r_s}{r}$$

Exact Schwarzschild for all $f \in (0,1]$.

### 5.4 Klein-Gordon Limit (Weak Field)

**Theorem.** *For $v^2 \ll 1$ and $\mathcal{L}^2 \ll 1$:*

$$\mathcal{L}_\text{matter} \approx -K_B + \frac{1}{2}|\Delta_t\mathbf{J}|^2 + \frac{1}{2}|\nabla_L\mathbf{J}|^2 + g_c\,s\,(\nabla_L\cdot\mathbf{J}) - \lambda_G(\ldots)^2$$

*This is the Klein-Gordon Lagrangian on the lattice.*

**Proof.** Write $f = 1 - \mathcal{L}^2$ and expand:

$$\frac{f^2 - v^2}{f} = (1-\mathcal{L}^2) - \frac{v^2}{1-\mathcal{L}^2} \approx 1 - v^2 - \mathcal{L}^2 + O(v^2\mathcal{L}^2)$$

Therefore $-K_B\sqrt{(f^2-v^2)/f} \approx -K_B\sqrt{1-v^2-\mathcal{L}^2} \approx -K_B + \tfrac{K_B}{2}v^2 + \tfrac{K_B}{2}\mathcal{L}^2$.

Substituting $v^2 = |\Delta_t\mathbf{J}|^2/K_B^2$ gives the kinetic term $|\Delta_t\mathbf{J}|^2/(2K_B)$. After canonical normalization ($\mathbf{J} \to \mathbf{J}/\sqrt{K_B}$), this is the standard Klein-Gordon form. The constant $-K_B$ is the rest energy and drops from the equations of motion. $\square$

### 5.5 Special Cases

| Scenario | $v$ | $\mathcal{L}$ | $f$ | $d\tau/dt$ | Regime |
|----------|-----|------|-----|-----------|--------|
| Rest, flat space | 0 | 0 | 1 | 1 | Minkowski |
| Moving, flat | $v$ | 0 | 1 | $\sqrt{1-v^2}$ | Special relativity |
| Rest, gravitational field | 0 | $\sqrt{r_s/r}$ | $1-r_s/r$ | $\sqrt{f}$ | Gravitational time dilation |
| Moving in field | $v$ | $\sqrt{r_s/r}$ | $1-r_s/r$ | $\sqrt{f-v^2/f}$ | **Exact Schwarzschild** |
| Photon | $f$ | any | $f$ | 0 | Null geodesic |
| Horizon | any | $\to 1$ | $\to 0$ | 0 | $f = 0$ surface |

---

## §6. The QM-GR Bridge

The action $S[\mathbf{J}, s, \mathcal{L}]$ produces both quantum mechanics and general relativity from the **same field** $\mathbf{J}$ on the **same lattice** $\Lambda$.

### 6.1 Quantum Mechanics

In the weak-field limit (§5.4), the flux field $\mathbf{J}$ obeys the lattice wave equation $\Delta_t^2 J_a = \nabla_L^2 J_a$. The lattice Green's function:

$$G_L(\mathbf{k}) = \frac{1}{\hat{k}^2}, \qquad \hat{k}_\mu = 2\sin(k_\mu/2), \qquad \mathbf{k} \in [-\pi,\pi]^3$$

is the **Euclidean QFT propagator** on the compact Brillouin zone BZ³ = $[-\pi,\pi]^3$.

| QFT result | Source | Reference |
|-----------|--------|-----------|
| Wick rotation yields Feynman propagator | Lattice → continuum | DERIV_QFT_GRT_BRIDGE §1 |
| Vertex factor $g_c = \sqrt{\alpha}$ | State-flux coupling term | DERIV_STATE_FLUX_COUPLING |
| Ward identity (exact on lattice) | Gauss constraint | DERIV_QFT_GRT_BRIDGE §1.5 |
| UV finiteness (loop integrals on a finite region of arbitrarily large extent) | Compact BZ: $\int_\text{BZ} < \infty$ | Mathematical fact |
| One-loop QED beta function | Vacuum polarization on BZ | DERIV_LATTICE_LOOP_CORRECTIONS |

### 6.2 General Relativity

The stress-energy tensor $T_{\mu\nu}$, derived from $\mathcal{L}_\text{matter}$ via Noether's theorem, sources spacetime curvature:

$$T^{\mu\nu} = (\partial^\mu J_a)(\partial^\nu J_a) - \eta^{\mu\nu}\mathcal{L}_\text{matter}$$

| GR result | Source | Reference |
|----------|--------|-----------|
| $\partial_\mu T^{\mu\nu} = 0$ | Wave equation $\Box J_a = 0$ | DERIV_QFT_GRT_BRIDGE §2.2 |
| Linearized Einstein eqs *(FTD-0189: conditional on Conjecture 10.1)* | Flux wave eq + metric identification | DERIV_EINSTEIN_FIELD_EQUATIONS §3 (now [SELECTION/CONDITIONAL]) |

| Poisson eq $\nabla^2\mathcal{L} = 4\pi G\rho$ | Variation of $S$ w.r.t. $\mathcal{L}$ (§4.2) | This document |
| Nonlinear completion: $G_{\mu\nu} = 8\pi G\,T_{\mu\nu}/c^4$ *(FTD-0189: inherits conditionality from linearized input)* | Lovelock's theorem [1] | DERIV_EINSTEIN_FIELD_EQUATIONS §5 (now [SELECTION — conditional on Conjecture 10.1]) |
| Exact Schwarzschild | Born-Infeld core (§4.3) | DERIV_LATTICE_SCHWARZSCHILD |

**Lovelock's theorem** [1]: *In four spacetime dimensions, the Einstein tensor $G_{\mu\nu} + \Lambda g_{\mu\nu}$ is the unique symmetric, divergence-free, rank-2 tensor constructed from the metric and at most its second derivatives.* Given that FTD independently derives $\partial_\mu T^{\mu\nu} = 0$ and the linearized Einstein equations, Lovelock's theorem forces the unique nonlinear completion to be the full Einstein equations.

### 6.3 The Unity

| Aspect | QM description | GR description | Common origin |
|--------|---------------|----------------|--------------|
| Field | Propagator $G_L(\mathbf{k})$ | Potential $\Phi(\mathbf{r})$ | Flux $\mathbf{J}$ on $\Lambda$ |
| Source | Vertex: $g_c\,s\,(\nabla\cdot J)$ | Stress-energy: $T_{\mu\nu}$ | $\mathcal{L}_\text{matter}$ |
| Coupling | $\alpha \approx 1/137$ | $\alpha_G \approx 10^{-39}$ | Master quadratic |
| Regularization | Compact BZ (UV finite) | Lattice spacing (no singularities) | Cubic graph $\Lambda$ |

**They are not two theories. They are two regimes of the same lattice action at different coupling scales.**

---

## §7. Derived Constants

All physical constants trace to Axiom 1 (cubic graph $\Lambda$ with no defined boundary), the calibration declaration $a \equiv \ell_P$ (gauge per FTD-0137; required for SI translation but not derived), and the mathematical constant $\varpi$:

| Constant | Formula | Value | vs. Experiment | Tag |
|----------|---------|-------|----------------|-----|
| $\alpha$ | $1/x_+$ (master quadratic) | $1/137.036$ | 1.26 ppm | [THEOREM]+[SELECTION] |
| $g_c$ | $\sqrt{\alpha}$ | 0.0854 | — | [THEOREM] |
| $K_B = m_e$ | $M_P\sqrt{2\pi}\,(16/3)\,\alpha^{11}$ | 0.510 MeV | 0.19% | [THEOREM] |
| $G$ | $\alpha_G\,\hbar c\,/\,m_e^2$ | $6.674 \times 10^{-11}$ | 0.06% | [THEOREM] |
| $\rho_\Lambda$ | $m_e^4\,\alpha^{16}\,G^{*2}$ | $3.86 \times 10^{-47}$ GeV⁴ | 1.0% (value-match only) | [PARAMETRIC] |

---

## §8. Claims Table

| ID | Statement | Tag |
|----|-----------|-----|
| L-1 | Born-Infeld core exactly reproduces Schwarzschild proper time for all $f$ | **[THEOREM conditional on clock-hypothesis AXIOM]** *(audit [`AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md`](../03_derivations/archive/AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md) established that the clock hypothesis is structurally incompatible with Scale 0 discrete primitives and must be posited as an independent macroscopic **[AXIOM]**; this theorem holds exactly conditional on that axiom; survives FTD-0189 audit since no h_μν correspondence is invoked)* |
| L-2 | Reduces to Klein-Gordon on the lattice in the weak-field limit | **[THEOREM]** |
| L-3 | $\gamma_\text{FTD}$ unifies SR and GR Lorentz factors | **[THEOREM]** |

| L-4 | Gauss constraint ($\lambda_G \to \infty$) generates U(1) gauge symmetry | **[THEOREM]** |
| L-5 | Lattice Green's function $G_L = 1/\hat{k}^2$ is the Euclidean QFT propagator | **[THEOREM]** |
| L-6 | Same $T_{\mu\nu}$ sources both QFT amplitudes and Einstein equations | **[THEOREM]** |
| L-7 | Poisson equation $\nabla^2\mathcal{L} = 4\pi G\rho$ follows from the action | **[THEOREM]** |
| L-8 | $\alpha = 1/x_+$ from master quadratic | **[SELECTION]** |
| L-9 | $K_B = m_e$ (manifestation threshold = electron mass) | **[SELECTION]** |
| L-10 | Velocity coupling $-g_c\,s\,(\mathbf{v}\cdot\mathbf{J})$ produces lattice Lorentz force | **[THEOREM]** |
| L-11 | Rayleigh dissipation $R = (\alpha/2)\lVert\mathbf{v}_\text{wave}\rVert^2$ with $\gamma = \alpha$ | **[IMPOSED]** |

**Epistemic breakdown: 8 [THEOREM], 2 [SELECTION], 0 [CONJECTURE], 1 [IMPOSED].**

---

## References

[1] D. Lovelock, "The Einstein tensor and its generalizations," *J. Math. Phys.* **12**, 498–501 (1971).

[2] [DERIV_QFT_GRT_BRIDGE.md](../03_derivations/foundational_mechanics/DERIV_QFT_GRT_BRIDGE.md) — QFT-GR duality of the flux field.

[3] [DERIV_EINSTEIN_FIELD_EQUATIONS.md](../03_derivations/gravity_and_cosmology/DERIV_EINSTEIN_FIELD_EQUATIONS.md) — Full Einstein equations via Lovelock.

[4] [DERIV_STATE_FLUX_COUPLING_DERIVATION.md](../03_derivations/electromagnetism/DERIV_STATE_FLUX_COUPLING_DERIVATION.md) — Vertex factor $g_c = \sqrt{\alpha}$.

[5] [DERIV_COSMOLOGICAL_CONSTANT.md](../04_coupling/DERIV_COSMOLOGICAL_CONSTANT.md) — Cosmological constant from lattice vacuum energy.

---

*Version 3.2 — March 16, 2026*
*Framework: Foundational Ternary Dynamics*
