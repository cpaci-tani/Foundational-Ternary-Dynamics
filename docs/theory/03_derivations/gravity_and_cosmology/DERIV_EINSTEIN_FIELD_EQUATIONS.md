# Einstein's Field Equations from FTD First Principles

## From Lattice Axiom to R_μν − ½g_μνR = 8πGT_μν

**Status:** **[CHAIN CONDITIONAL on Conjecture 10.1 per FTD-0189]**. The chain's linearized-Einstein input (Step 3, citing DERIV_RELATIVITY Theorem 14.1) was retagged `[SELECTION/CONDITIONAL]` by FTD-0189. The Lovelock-completion (Step 5) inherits this conditionality. The Schwarzschild scalar sector (g_00 via Step 4 of FTD-0131; SPEC §4.3 Born-Infeld → Schwarzschild proper time) survives unchanged because it does not invoke `h_μν` correspondence. See `AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` §3 + LEDGER FTD-0189.
**Closes (qualified):** GAP-2 (nonlinear Einstein equations) — **conditional on Conjecture 10.1** (h_μν posited per FTD-0189; substrate spin-2 emergence `[CLOSED NEGATIVE per FTD-0193]` in probed regime). GAP-14.1 (DERIV_RELATIVITY_DERIVATION.md) closure also inherits this conditionality.

**Depends on:**

- [DERIV_RELATIVITY_DERIVATION.md](DERIV_RELATIVITY_DERIVATION.md) — SR, linearized GR (§§1-15)
- [DERIV_QFT_GRT_BRIDGE.md](../foundational_mechanics/DERIV_QFT_GRT_BRIDGE.md) — T_μν via Noether's theorem
- [SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md) — Born-Infeld render-bridge action
- [SPEC_SM_REPLACEMENT_COMPLETE.md](../../01_reference/SPEC_SM_REPLACEMENT_COMPLETE.md) — the α_G integer-dressing formula's canonical [STRUCTURALLY MOTIVATED PARAMETRIC] filing (citation corrected 2026-07-01 — the previously-cited lemniscate-hierarchy whitepaper does not contain this formula)

---

## Abstract

We derive the full nonlinear Einstein field equations from FTD's lattice postulate in five steps:

1. **Metric emergence** from the latency field $\mathcal{L}$
2. **Stress-energy tensor** from the Born-Infeld action via Noether's theorem
3. **Linearized Einstein equations** from the flux wave equation
4. **Newton's constant** from the FTD coupling hierarchy
5. **Nonlinear completion** forced uniquely by Lovelock's theorem

The final result:

$$\boxed{R_{\mu\nu} - \frac{1}{2}g_{\mu\nu}R = \frac{8\pi G}{c^4} T_{\mu\nu}}$$

is the **unique** nonlinear field equation consistent with FTD's established linearized structure, conservation law $\nabla_\mu T^{\mu\nu} = 0$, and D=4 spacetime dimensionality.

---

# Step 1: Metric Emergence from the Latency Field [SELECTION]

## 1.1 The Effective Metric

From the FTD lattice postulate ($D=3$ cubic lattice, ternary states, $C=1$), the Minkowski metric $\eta_{\mu\nu} = \text{diag}(+1,-1,-1,-1)$ emerges as the null cone structure of the flux wave equation (Theorem 7.2, DERIV_RELATIVITY_DERIVATION.md):

$$\partial_t^2 J = C^2 \nabla^2_L J \quad \Rightarrow \quad ds^2 = c^2 dt^2 - dx^2 - dy^2 - dz^2$$

Near a mass source, the latency field $\mathcal{L}(\mathbf{r})$ modifies the effective metric. From the Born-Infeld Lagrangian (SPEC_FTD_LAGRANGIAN.md), the proper time formula:

$$d\tau^2 = \frac{f^2 - v^2}{f} \, dt^2 \quad \text{where} \quad f = 1 - \mathcal{L}^2$$

identifies the effective metric components (DERIV_LATTICE_BLACK_HOLES.md, §A.8; the g_rr identification is imported from standard GR and is a substrate `[GAP]` per FTD-0361 — see §1.3):

$$g_{00} = f = 1 - \mathcal{L}^2, \qquad g_{rr} = -\frac{1}{f} = -\frac{1}{1 - \mathcal{L}^2}$$

## 1.2 The General Identification

More generally, the metric perturbation $h_{\mu\nu}$ is sourced by the latency field:

$$g_{\mu\nu} = \eta_{\mu\nu} + h_{\mu\nu}(\mathcal{L})$$

In the weak-field regime, $h_{00} = -2\Phi/c^2$ where $\Phi = -GM/r$ is the Newtonian potential, giving $g_{00} = 1 - 2GM/(rc^2)$ (Theorem 10.1, DERIV_RELATIVITY_DERIVATION.md).

## 1.3 What This Step Establishes

| Result | Tag | Source |
|--------|-----|--------|
| $\eta_{\mu\nu}$ from wave equation | [THEOREM] | Thm 7.2, DERIV_RELATIVITY |
| $g_{00} = 1 - r_s/r$ from flux saturation | [THEOREM] | Thm 11.1, DERIV_RELATIVITY |
| $g_{rr} = -1/f$ from velocity amplification | **[GAP]** substrate g_rr undetermined (FTD-0361) + [SELECTION] *(retagged 2026-07-14; was [THEOREM] — imported standard GR, not substrate-derived)* | §A.7/§A.8.2, DERIV_LATTICE_BLACK_HOLES |
| General $g_{\mu\nu} = \eta_{\mu\nu} + h_{\mu\nu}(\mathcal{L})$ | [SELECTION] | Effective metric proposal |

---

# Step 2: Stress-Energy Tensor from the Flux Lagrangian [THEOREM]

## 2.1 The Canonical T_μν

From the free-field flux Lagrangian $\mathcal{L}_\text{free} = \frac{1}{2}\dot{J}_a\dot{J}_a - \frac{1}{2}C^2(\partial_i J_a)(\partial_i J_a)$, Noether's theorem gives (Theorem 2.1, DERIV_QFT_GRT_BRIDGE.md):

$$T^{\mu\nu} = (\partial^\mu J_a)(\partial^\nu J_a) - \eta^{\mu\nu}\mathcal{L}$$

**Explicit components:**

$$T^{00} = \frac{1}{2}|\dot{J}|^2 + \frac{1}{2}C^2|\nabla J|^2 \quad \text{(energy density, positive-definite)}$$

$$T^{0i} = \dot{J}_a \partial_i J_a \quad \text{(energy flux / Poynting vector)}$$

$$T^{ij} = (\partial_i J_a)(\partial_j J_a) - \delta^{ij}\mathcal{L} \quad \text{(stress tensor)}$$

## 2.2 Conservation Law

From the wave equation $\Box J_a = 0$ (Theorem 2.2, DERIV_QFT_GRT_BRIDGE.md):

$$\partial_\mu T^{\mu\nu} = 0 \quad \text{[THEOREM]}$$

## 2.3 Properties

| Property | Status | Significance for Einstein eqs |
|----------|--------|-------------------------------|
| Symmetric: $T^{\mu\nu} = T^{\nu\mu}$ | [THEOREM] | Required by Einstein equations |
| Conserved: $\partial_\mu T^{\mu\nu} = 0$ | [THEOREM] | Forces Bianchi identity on LHS |
| Positive energy: $T^{00} \geq 0$ | [THEOREM] | Energy conditions satisfied |
| Traceless for radiation | [THEOREM] | Correct conformal limit |

All four properties required for a consistent source of Einstein's equations are derived, not assumed.

## 2.4 Upgrade to Born-Infeld T_μν [SELECTION]

The full Born-Infeld Lagrangian $\mathcal{L}_\text{RB} = -K_B\sqrt{(f^2-v^2)/f}$ yields a more general stress-energy tensor. In the weak-field limit ($v \ll 1$, $\mathcal{L} \ll 1$), it reduces to the canonical form above (Theorem 3.1, SPEC_FTD_LAGRANGIAN.md). The Born-Infeld form naturally incorporates:

- **Rest-mass energy:** $K_B = m_e c^2$ appears as the constant term
- **Self-gravitating systems:** The $\mathcal{L}$ field's energy itself gravitates
- **Speed limit:** The $v < f$ constraint is manifest

For this derivation, the canonical (weak-field) form is sufficient. The Born-Infeld corrections become important only in the strong-field regime.

---

# Step 3: Linearized Einstein Equations [SELECTION — conditional on Conjecture 10.1, per FTD-0189]

## 3.1 The Established Result

**FTD-0189 ripple correction:** Theorem 14.1 of `DERIV_RELATIVITY_DERIVATION.md` is retagged from `[THEOREM]` to `[SELECTION/CONDITIONAL]` per the FTD-0189 audit — the proof rests on the h  J correspondence (Conjecture 10.1), which is `[CONJECTURE]` rather than derived. Step 3 below inherits this conditionality. Substrate spin-2 emergence (the h_μν construction Conjecture 10.1 would require) was tested via FTD-0193 and `[CLOSED NEGATIVE]` in the probed regime L∈{32,64}.

From the flux wave equation and the metric perturbation, the linearized Einstein equations are (Theorem 14.1, DERIV_RELATIVITY_DERIVATION.md):

$$\Box \bar{h}_{\mu\nu} = -\frac{16\pi G}{c^4} T_{\mu\nu}$$

where $\bar{h}_{\mu\nu} = h_{\mu\nu} - \frac{1}{2}\eta_{\mu\nu}h$ is the trace-reversed perturbation, in the Lorenz gauge $\partial^\mu \bar{h}_{\mu\nu} = 0$.

## 3.2 What the Linearized Form Tells Us

The linearized equation encodes:

1. **The source:** $T_{\mu\nu}$ (derived in Step 2)
2. **The coupling:** $16\pi G/c^4$ (derived in Step 4)
3. **The dynamics:** Wave propagation at speed $c$
4. **The gauge:** Lorenz condition (from Gauss constraint)

## 3.3 Content of the Linearized Equations

The linearized equations are equivalent to linearized GR. They correctly predict:

| Phenomenon | Test | Status |
|-----------|------|--------|
| Gravitational time dilation | Pound-Rebka | [THEOREM] |
| Precession of perihelion | Mercury | [THEOREM] (from Schwarzschild) |
| Light bending | Eddington 1919 | [THEOREM] (from geodesics) |
| Gravitational waves | LIGO/Virgo | [THEOREM] — 2 polarizations, speed $c$ |
| Shapiro delay | Viking/Cassini | [THEOREM] |
| GPS corrections | Operating system | [THEOREM] |

---

# Step 4: Newton's Constant from the Lattice [STRUCTURALLY MOTIVATED PARAMETRIC + SELECTION]

**(Corrected 2026-07-01, FTD-0348, per the Fable specialist review — the previous version of this Step defined $\alpha_G$ with $m_e^2$ but applied the proton-coupling formula (a $(m_p/m_e)^2 \approx 3.4\times10^6$ definitional mismatch), mis-stated $\alpha^{20}$ by a factor of 1.99, cited a whitepaper that does not contain the formula, and tagged the result [THEOREM]. All fixed below; nothing promoted.)**

## 4.1 The Gravitational Coupling Hierarchy

Newton's constant $G$ is related to the **proton** gravitational fine structure constant by the standard (imported) dimensional relation:

$$\alpha_G(p,p) = \frac{G m_p^2}{\hbar c} = \frac{m_p^2}{M_P^2} \approx 5.906 \times 10^{-39} \quad \text{(CODATA 2022)}$$

The FTD integer-dressing formula for this quantity — **[STRUCTURALLY MOTIVATED PARAMETRIC]**, as filed in `SPEC_SM_REPLACEMENT_COMPLETE.md` (the functional form $G = \alpha_G \hbar c/m^2$ is standard dimensional analysis; only the $\alpha^{20}$ integer dressing is FTD's) — is:

$$\alpha_G = 2\pi \left(\frac{16}{3}\right)^2 \left(N_\text{eff} + \frac{3}{b_3}\right)^2 \alpha^{20}$$

with $\{N_\text{eff} = 13, b_3 = 7\}$.

## 4.2 Numerical Evaluation

$$\alpha_G = 2\pi \times \frac{256}{9} \times \left(13 + \frac{3}{7}\right)^2 \times \alpha^{20}$$

$$= 178.72 \times 180.33 \times 1.834 \times 10^{-43} = 5.909 \times 10^{-39}$$

against the measured $(m_p/m_P)^2 = 5.906 \times 10^{-39}$: a $+0.05\%$ match. **Two honesty caveats:** (i) the formula's implicit proton-mass spelling, $(N_\text{eff}+3/b_3)/\alpha = 1840.35\,m_e$, is inconsistent with FTD's canonical $m_p/m_e = N_\text{eff}/\alpha + N_\text{base}N_\text{eff} + N_c = 1836.47$ ($+0.21\%$ apart); computing $\alpha_G(p,p)$ from the canonical mass formulas instead gives $-0.33\%$. The sub-0.1% precision is spelling-dependent, not robust. (ii) The substrate-*derived* gravity result is a different, weaker quantity: $\alpha_G(e,e) = (m_e/m_P)^2$ per FTD-0131 (`DERIV_NEWTON_FROM_SUBSTRATE.md`), `[DERIVED]` modulo the clock-hypothesis axiom, inheriting FTD-0015's `[SMC]` floor (0.38% match).

The coefficient $8\pi G/c^4$ in the Einstein equations then emerges from:

$$\frac{8\pi G}{c^4} = \frac{8\pi}{M_P^2} \quad \text{(in natural units)}$$

## 4.3 The 8πG Factor

| Component | Origin | Tag |
|-----------|--------|-----|
| $\pi$ | Solid angle integration (Gauss law on lattice) | [THEOREM] |
| Factor 8 | $2 \times N_\text{base} = 2 \times 4$ from trace-reversal and spinor dimension | [SELECTION] |
| $G = \alpha_G \hbar c / m_p^2$ | $\alpha^{20}$ integer dressing of the imported dimensional relation | [STRUCTURALLY MOTIVATED PARAMETRIC] (corrected 2026-07-01; was [THEOREM]) |

---

# Step 5: Nonlinear Completion via Lovelock's Theorem [THEOREM]

## 5.1 The Key Theorem

**Lovelock's Theorem (1971-1972):** *In $D = 4$ spacetime dimensions, the unique symmetric, divergence-free, second-rank tensor constructed from the metric $g_{\mu\nu}$ and its first and second derivatives is:*

$$G_{\mu\nu} + \Lambda g_{\mu\nu} = R_{\mu\nu} - \frac{1}{2}g_{\mu\nu}R + \Lambda g_{\mu\nu}$$

*where $R_{\mu\nu}$ is the Ricci tensor (a contraction of the Riemann curvature) and $\Lambda$ is a constant.*

This theorem is a mathematical fact about differential geometry, requiring no physical input.

## 5.2 The Derivation

We now assemble the full nonlinear Einstein equations:

**Premise 1 (Linearized form):** FTD derives the linearized Einstein equations $\Box \bar{h}_{\mu\nu} = -(16\pi G/c^4) T_{\mu\nu}$ [THEOREM, Step 3].

**Premise 2 (Conservation):** The source satisfies $\nabla_\mu T^{\mu\nu} = 0$ [THEOREM, Step 2].

**Premise 3 (Consistency):** The LHS of the field equations must also be divergence-free, i.e., $\nabla_\mu(\text{geometric LHS})^{\mu\nu} = 0$, for the equations to be self-consistent.

**Premise 4 (Dimensionality):** FTD operates in $D = 3$ spatial dimensions, giving $D = 4$ spacetime dimensions [AXIOM].

**Premise 5 (Metric structure):** The effective metric $g_{\mu\nu}$ is determined by the latency field [SELECTION, Step 1]. The field equations must be second-order in derivatives of $g_{\mu\nu}$ (higher-order theories introduce ghosts/instabilities).

**Application of Lovelock's theorem:**

Given Premises 3-5, Lovelock's theorem states that the geometric LHS must be:

$$(\text{LHS})_{\mu\nu} = G_{\mu\nu} + \Lambda g_{\mu\nu} = R_{\mu\nu} - \frac{1}{2}g_{\mu\nu}R + \Lambda g_{\mu\nu}$$

**Matching to the linearized form:**

In the weak-field limit ($g_{\mu\nu} \approx \eta_{\mu\nu} + h_{\mu\nu}$), the Einstein tensor $G_{\mu\nu}$ linearizes to:

$$G_{\mu\nu}^{(1)} = -\frac{1}{2}\Box \bar{h}_{\mu\nu} \quad \text{(in Lorenz gauge)}$$

Comparing with Step 3:

$$-\frac{1}{2}\Box \bar{h}_{\mu\nu} = \frac{8\pi G}{c^4} T_{\mu\nu}$$

$$\Rightarrow \quad G_{\mu\nu} = \frac{8\pi G}{c^4} T_{\mu\nu}$$

**The cosmological constant $\Lambda$:**

Lovelock's theorem allows a cosmological constant. In the linearized regime ($h \ll 1$), the $\Lambda g_{\mu\nu}$ term contributes $\Lambda \eta_{\mu\nu}$ — a constant background that does not affect the linearized dynamics (it can be absorbed into the background). Therefore, $\Lambda$ is not determined by the linearized Einstein equations alone.

FTD does **not** supply $\Lambda$ from a lattice vacuum energy. Its classical substrate declines $\hbar$ (FC-1), so the empty void is identically zero-energy and FTD's own dynamics predicts $\Lambda = 0$ [DERIVED]; the old $M_\text{Planck}^4$ catastrophe is dissolved by construction, not solved. For any *nonzero* $\Lambda$, FC-3 forces the form $\Lambda\,\ell_P^2 = f(\ell_P/L)$ [DERIVED-from-FC-3] and the holographic principle fixes the ceiling $\Lambda \lesssim (\ell_P/L_H)^2$ [SELECTION] — a bound, not a source. The source is [OPEN] and the numerical value is a [BOUNDARY] (it needs the horizon $L_H$, which FTD cannot supply natively, FTD-0059). The standard relation $\Lambda = 3H_0^2\Omega_\Lambda/c^2$ with $\Omega_\Lambda \approx 0.69$ is the imported $\Lambda$CDM apparatus, **not** an FTD prediction. See [`DERIV_LAMBDA_SCALE_COVARIANT.md`](DERIV_LAMBDA_SCALE_COVARIANT.md) (FTD-0331).

## 5.3 The Complete Result

$$\boxed{R_{\mu\nu} - \frac{1}{2}g_{\mu\nu}R + \Lambda g_{\mu\nu} = \frac{8\pi G}{c^4} T_{\mu\nu}}$$

where:

| Symbol | FTD Origin | Tag |
|--------|-----------|-----|
| $g_{\mu\nu}$ | Effective metric from latency field $\mathcal{L}$ | [SELECTION] |
| $R_{\mu\nu}$ | Ricci curvature (from $g_{\mu\nu}$ and its derivatives) | [THEOREM] (mathematics) |
| $T_{\mu\nu}$ | Noether current of flux Lagrangian | [THEOREM] |
| $G$ | From $\alpha_G = 2\pi(16/3)^2(N_\text{eff}+3/b_3)^2\alpha^{20}$ | [STRUCTURALLY MOTIVATED PARAMETRIC] (corrected 2026-07-01) |
| $\Lambda$ | No native source; FTD predicts $\Lambda = 0$, form fixed by FC-3, value a [BOUNDARY] (FTD-0331) | [OPEN] (source) |

---

# Step 5.5: Why Lovelock's Theorem Is Not Circular [CONTEXT]

A potential objection: "You're assuming Einstein's equations and calling it a derivation."

This objection fails for three reasons:

**1. Lovelock's theorem is mathematics, not physics.**

Lovelock's theorem is a theorem in differential geometry, analogous to Noether's theorem. It states that the Einstein tensor $G_{\mu\nu}$ is the **unique** divergence-free symmetric 2-tensor built from the metric and at most second derivatives. Using it is no more "assuming GR" than using Noether's theorem is "assuming conservation laws."

**2. The physical content comes from FTD.**

Lovelock's theorem tells you the *form* of the LHS. FTD provides:
- The RHS: $T_{\mu\nu}$ [derived from flux Lagrangian]
- The coupling: $8\pi G/c^4$ [derived from $\alpha_G$ hierarchy]
- The conservation law: $\nabla_\mu T^{\mu\nu} = 0$ [derived from wave equation]
- The linearized limit: [derived from flux dynamics]

Without FTD, Lovelock gives you an empty mathematical structure. FTD fills it with physical content.

**3. The alternative would be remarkable.**

If FTD's linearized equations did NOT extend to Einstein's equations, it would mean FTD produces gravity that is **not** described by GR. This would be an even stronger claim — either FTD recovers GR (which it does, via Lovelock), or it contradicts the most precisely tested theory in physics.

---

# Verification: Consistency Checks

## Check 1: Schwarzschild as a Vacuum Solution

Setting $T_{\mu\nu} = 0$ (vacuum) and $\Lambda = 0$, the field equations become $R_{\mu\nu} = 0$.

By Birkhoff's theorem, the unique spherically symmetric vacuum solution is the Schwarzschild metric:

$$ds^2 = f \, c^2 dt^2 - \frac{dr^2}{f} - r^2 d\Omega^2, \quad f = 1 - \frac{r_s}{r}$$

This is **exactly** the metric derived from the Born-Infeld Lagrangian in DERIV_LATTICE_SCHWARZSCHILD.md. 

## Check 2: Weak-Field → Poisson Equation

In the weak-field, static limit:
- $R_{00} \approx -\frac{1}{2}\nabla^2 h_{00}$
- With $h_{00} = -2\Phi/c^2$: $R_{00} = \nabla^2\Phi/c^2$
- The 00-component of Einstein's equations gives:

$$\nabla^2 \Phi = 4\pi G \rho$$

This is Poisson's equation — the Newtonian limit. FTD derives this as $\nabla^2\mathcal{L} = 4\pi G\rho$ in SPEC_FTD_LAGRANGIAN.md. 

## Check 3: Gravitational Waves

Setting $T_{\mu\nu} = 0$ in the linearized equations:

$$\Box \bar{h}_{\mu\nu} = 0$$

This gives transverse-traceless waves with 2 polarizations propagating at speed $c$. Matches LIGO/Virgo observations ($|c_{GW} - c|/c < 10^{-15}$). Derived as Theorem 15.1-15.3 in DERIV_RELATIVITY_DERIVATION.md. 

## Check 4: Conservation Consistency

The Bianchi identity $\nabla_\mu G^{\mu\nu} = 0$ is a mathematical identity. Combined with the field equations, it implies $\nabla_\mu T^{\mu\nu} = 0$. This is **independently derived** from the wave equation (Theorem 2.2, DERIV_QFT_GRT_BRIDGE.md). 

The conservation law is derived from **both** the geometric side (Bianchi) and the matter side (Noether). This mutual consistency is a non-trivial structural check.

## Check 5: Kerr Metric

The rotating black hole solution to $R_{\mu\nu} = 0$ is the Kerr metric, which is independently derived from FTD's vortical flux patterns in DERIV_LATTICE_KERR.md. 

---

# Claims Table

| ID | Claim | Status | Key dependency |
|----|-------|--------|---------------|
| EFE-1 | Minkowski metric from wave equation | **[THEOREM]** | DERIV_RELATIVITY Thm 7.2 |
| EFE-2 | $g_{00} = 1 - r_s/r$ from flux saturation | **[THEOREM]** | DERIV_RELATIVITY Thm 11.1 |
| EFE-3 | $g_{rr} = -1/f$ from velocity amplification | **[GAP]** substrate g_rr undetermined (FTD-0361) + **[SELECTION]** *(retagged 2026-07-14; was [THEOREM] — imported standard GR, not substrate-derived)* | DERIV_LATTICE_BLACK_HOLES §A.7/§A.8.2 |
| EFE-4 | $T_{\mu\nu}$ via Noether from flux $\mathcal{L}$ | **[THEOREM]** | DERIV_QFT_GRT_BRIDGE Thm 2.1 |
| EFE-5 | $\nabla_\mu T^{\mu\nu} = 0$ from wave equation | **[THEOREM]** | DERIV_QFT_GRT_BRIDGE Thm 2.2 |
| EFE-6 | Linearized Einstein: $\Box\bar{h}_{\mu\nu} = -16\pi G T_{\mu\nu}/c^4$ | **[SELECTION — conditional on Conjecture 10.1]** *(retagged per FTD-0189 ripple; was [THEOREM])* | DERIV_RELATIVITY Thm 14.1 ([SELECTION/CONDITIONAL]) |
| EFE-7 | $G$ from $\alpha_G$ hierarchy ($\alpha^{20}$) | **[STRUCTURALLY MOTIVATED PARAMETRIC]** (corrected 2026-07-01; was [THEOREM]) | SPEC_SM_REPLACEMENT_COMPLETE §gravity (Step 4 above) |
| EFE-8 | Nonlinear completion via Lovelock | **[SELECTION — conditional on EFE-6]** *(inherits from EFE-6)* | Lovelock (1971) + Premises 1-5 |
| EFE-9 | Full Einstein equations recovered | **[SELECTION — conditional on Conjecture 10.1]** *(inherits from EFE-6 + EFE-8)* | EFE-1 through EFE-8 |
| EFE-10 | $\Lambda$: no native source (FTD predicts $\Lambda=0$); form [DERIVED-from-FC-3], value a [BOUNDARY] | **[OPEN]** (source) | FTD-0331 (CC dissolved, not solved) |
| EFE-11 | Schwarzschild as vacuum solution (imported standard GR) | **[VERIFIED]** | Birkhoff + DERIV_LATTICE_BLACK_HOLES |
| EFE-12 | Poisson equation in weak-field limit | **[VERIFIED]** | Standard GR + SPEC_FTD_LAGRANGIAN |
| EFE-13 | GW propagation (2 polarizations, speed $c$) | **[VERIFIED]** | DERIV_RELATIVITY §15 |

**Epistemic breakdown** *(per FTD-0189 ripple; EFE-10 reconciled per FTD-0331; EFE-7 corrected per FTD-0348; EFE-3 retagged 2026-07-14 per FTD-0361)*: 4 [THEOREM] (EFE-1, EFE-2, EFE-4, EFE-5) + 1 [GAP] (EFE-3, the substrate g_rr — imported standard GR, not substrate-derived; FTD-0361) + 1 [STRUCTURALLY MOTIVATED PARAMETRIC] (EFE-7, corrected 2026-07-01 — the dimensional relation is imported, the α²⁰ dressing is a parametric insertion, and the previous Step-4 arithmetic was wrong by 2× under a mismatched m_e² definition) + 3 [SELECTION — conditional on Conjecture 10.1] (EFE-6, EFE-8, EFE-9) + 1 [OPEN] (EFE-10, the $\Lambda$ source) + 3 [VERIFIED] (EFE-11, EFE-12, EFE-13). Three [THEOREM] tags were downgraded to [SELECTION] when FTD-0189 retagged the load-bearing Theorem 14.1 input; EFE-3's g_rr was downgraded to [GAP] when FTD-0361 declined to adjudicate the velocity-amplification route and retracted its sibling proof.

---

# What This Derivation Does and Does NOT Claim

## What it does:

1. **Closes GAP-2:** The full nonlinear Einstein field equations are now derived from FTD premises plus Lovelock's mathematical theorem.
2. **Uniqueness:** The derivation shows that GR is the **unique** classical gravity theory consistent with FTD's lattice structure in D=4.
3. **Self-consistency:** All five verification checks pass — Schwarzschild, Poisson, gravitational waves, conservation, and Kerr.

## What it does NOT claim:

1. **Independent derivation of the Riemann tensor from the lattice.** The Riemann curvature is defined via standard differential geometry applied to the effective metric $g_{\mu\nu}$. FTD provides $g_{\mu\nu}$; Riemannian geometry provides the technology to compute its curvature.

2. **Derivation of Lovelock's theorem.** This is a mathematical result (proven by Lovelock in 1971), not a physics claim. Using it is analogous to using calculus — it's mathematical technology, not a physical assumption.

3. **Resolution of the cosmological constant problem.** FTD does not solve the cosmological-constant problem; it *dissolves* the old $M_\text{Planck}^4$ catastrophe by construction (FC-1 declines $\hbar$ ⇒ classical vacuum is zero-energy ⇒ $\Lambda = 0$ [DERIVED]). The form of any nonzero $\Lambda$ is [DERIVED-from-FC-3] and its ceiling is a holographic [SELECTION], but the *source* of the small observed dark energy is [OPEN] and its *value* is a [BOUNDARY] (FTD-0331). The earlier $\rho_\Lambda = m_e^4\,\alpha^{16}\,G^{*2}$ / $\alpha^{57}$ value-match is [PARAMETRIC] numerology, not a resolution.

4. **Full quantum gravity.** This derivation yields the *classical* Einstein equations. Quantum gravitational effects (Planck-scale physics, black hole information problem) require the full lattice dynamics, not the effective metric description.

---

# Cross-References

| Document | Relevant Content |
|----------|-----------------|
| [DERIV_RELATIVITY_DERIVATION.md](DERIV_RELATIVITY_DERIVATION.md) | SR, linearized GR, GAP-2 now resolved |
| [DERIV_QFT_GRT_BRIDGE.md](../foundational_mechanics/DERIV_QFT_GRT_BRIDGE.md) | T_μν derivation, conservation law |
| [SPEC_FTD_LAGRANGIAN.md](../01_reference/SPEC_FTD_LAGRANGIAN.md) | Born-Infeld action, Poisson equation |
| [SPEC_SM_REPLACEMENT_COMPLETE.md](../../01_reference/SPEC_SM_REPLACEMENT_COMPLETE.md) | α_G integer-dressing formula ([STRUCTURALLY MOTIVATED PARAMETRIC]; citation corrected 2026-07-01) |

---

## Appendix: The Complete Derivation Chain

```
AXIOM: D=3 cubic lattice, ternary states, C=1
    ↓ [wave equation structure]
Theorem 7.2: η_μν = diag(+1,-1,-1,-1)        (Minkowski metric)
    ↓ [flux saturation near mass]
Theorem 11.1: g₀₀ = 1 - r_s/r                 (gravitational time dilation)
    ↓ [velocity amplification in congested lattice]
§7: g_rr = -1/f                                 (spatial metric component)
    ↓ [combine → effective metric]
g_μν = η_μν + h_μν(𝓛)                          (latency → curvature)
    ↓ [Noether's theorem on flux Lagrangian]
Theorem 2.1: T_μν = (∂μJa)(∂νJa) - η_μν 𝓛     (stress-energy tensor)
    ↓ [wave equation □J = 0]
Theorem 2.2: ∂_μ T^μν = 0                       (conservation)
    ↓ [flux wave equation + metric identification]
Theorem 14.1: □h̄_μν = -(16πG/c⁴) T_μν          (linearized Einstein)
    ↓ [α_G = 2π(16/3)²(N_eff+3/b₃)²α²⁰]
G = α_G ℏc/m_p² = M_P⁻²                        (Newton's constant; m_p² — corrected 2026-07-01)
    ↓ [Lovelock's theorem: unique divergence-free 2-tensor in D=4]
    ↓ [only possible nonlinear completion of linearized form]
╔══════════════════════════════════════════════════════════════╗
║  R_μν - ½ g_μν R + Λg_μν = (8πG/c⁴) T_μν                 ║
║                                                              ║
║  Einstein's Field Equations                                  ║
╚══════════════════════════════════════════════════════════════╝
```

---

*Framework: Foundational Ternary Dynamics v5.27*
*Closes: GAP-2, GAP-14.1*
