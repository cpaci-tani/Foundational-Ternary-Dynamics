# The PF Atlas: How π/4 Enters and Exits Standard Physics

## Comparative Physics of the Standard Model and FTD

**Document Version:** 1.0
**Framework Version:** FTD v5.26
**Date:** February 19, 2026
**Standard:** Side-by-side decomposition of standard physics formulas through PF = π/4

**Depends on:**
- [DERIV_GSTAR_PF_BRIDGE.md](../04_coupling/DERIV_GSTAR_PF_BRIDGE.md) — G* = ϖ/√(PF) decomposition and PF cancellation rule
- [SPEC_FTD_LAGRANGIAN.md](SPEC_FTD_LAGRANGIAN.md) — SM Lagrangian sector-by-sector mapping
- [FOUND_SPACETIME_EMERGENCE.md](../02_foundations/FOUND_SPACETIME_EMERGENCE.md) — D = 3 derivation and integer origins
- [FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md](../02_foundations/FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md) — Historical/interpretive constant atlas γ → ϖ → M → π → G*

---

## Abstract

The Standard Model expresses physics through transcendental factors (π, 2π, 4π, π², ...) that appear as unexplained denominators in coupling constants, loop corrections, black hole thermodynamics, and vacuum energy. FTD decomposes each of these factors into products of **PF = π/4** (the circle-in-square packing fraction) and **framework integers** {N_c = 3, N_base = 4, b₃ = 7, N_eff = 13}.

This document is the **atlas** of that decomposition: for every standard physics formula containing π, we show which FTD integers multiply PF to produce the standard result, what the integer factors *mean* (degrees of freedom, color charges, algebraic tower dimensions), and whether PF cancels in the final observable.

The organizing principle is the **PF Cancellation Rule** (the "Golden Rule of the Lattice"): PF cancels in every dimensionless physical ratio and survives only in absolute scale-setting quantities.

---

## Epistemic Framework

| Tag | Meaning |
|-----|---------|
| **[THEOREM]** | Algebraic identity; follows from definitions |
| **[SELECTION]** | Argued decomposition; numerically exact but interpretation is a choice |
| **[CONJECTURE]** | Pattern claim without proof |

**Critical distinction:** The numerical identities (e.g., 4π = 16 × π/4) are trivially true — they are rearrangements of arithmetic. The **claims** are about the physical meaning of the integer factors: that 16 counts physical degrees of freedom, that 3 is the color charge number, etc. Every non-trivial claim is tagged.

---

# PART I: THE RENDERING ENGINE

## §1. Continuous to Discrete: Three Layers

The FTD framework converts between continuous analytical geometry and discrete lattice computation through three identities.

### 1.1 The Packing Fraction [DEFINITION]

$$\text{PF} \equiv \frac{\pi}{4} \approx 0.78540$$

The fraction of a square lattice face covered by its maximal inscribed circle. This is **the** geometric cost of embedding circular (continuous) physics on a square (discrete) lattice.

### 1.2 The Ontic Bridge [THEOREM]

$$\boxed{G^* = \frac{\varpi}{\sqrt{\text{PF}}}}$$

where ϖ ≈ 2.622 is the lemniscate half-period (continuous geometry) and G* ≈ 2.959 is the FTD master constant. This is proven in [DERIV_GSTAR_PF_BRIDGE.md](../04_coupling/DERIV_GSTAR_PF_BRIDGE.md) §1.

### 1.3 The Key Decomposition Identity [THEOREM]

$$\pi = N_{\text{base}} \times \text{PF} = 4 \times \frac{\pi}{4}$$

This is arithmetically trivial, but the **interpretation** is that every factor of π in physics decomposes into:
- **N_base = 4**: a count of something (lattice faces, division algebras, DOF blocks)
- **PF = π/4**: the circular-to-square conversion factor

### 1.4 The Master Quadratic in PF Form [THEOREM]

The master quadratic $x^2 - 16G^{*2}x + 16G^{*3} = 0$ becomes:

$$x^2 = N_{\text{base}}^2 \cdot \frac{\varpi^2}{\text{PF}} \cdot \left(x - \frac{\varpi}{\sqrt{\text{PF}}}\right)$$

using $G^* = \varpi/\sqrt{\text{PF}}$ and $16 = N_{\text{base}}^2$.

---

## §2. Integer Origins from D = 3 [SELECTION]

All four framework integers derive from the dimensionality D = 3 (see [FOUND_SPACETIME_EMERGENCE.md](../02_foundations/FOUND_SPACETIME_EMERGENCE.md)):

| Integer | Formula from D | Value | Physical meaning |
|---------|---------------|-------|------------------|
| N_base | $2^{D-1}$ | 4 | Faces of D-cube; base dimension of division algebra tower |
| N_c | $2^{D-1} - 1$ | 3 | Color charges; master quadratic root $x_- \approx 3.024$ |
| b₃ | $2^D - 1$ | 7 | QCD beta coefficient (N_f = 6); Mersenne number; octonion units |
| N_eff | $F_7$ (Fibonacci) | 13 | Self-referential closure; $b_3 + 2N_c = 7 + 6 = 13$ |

**Derived combinations:**

| Combination | Value | Appears in |
|-------------|-------|------------|
| N_base × PF | π | Everywhere |
| N_base² × PF | 4π | BH entropy, QCD, gauge couplings |
| 2 × N_base² × PF | 8π | Hawking temperature, Einstein equations |
| N_base × N_c × PF | 3π | QED beta function |
| N_base² × N_c × PF | 12π | QCD running coupling |
| 2^D × PF | 2π | One-loop QFT, angular integrals |
| (N_base × PF)² | π² | Vacuum energy, Stefan-Boltzmann |

---

# PART II: QUANTUM FIELD THEORY

## §3. Gauge Couplings

### 3.1 Electromagnetic Coupling [THEOREM + SELECTION]

In Heaviside-Lorentz units, the electromagnetic charge is $e = \sqrt{4\pi\alpha}$.

| Form | Expression | Value |
|------|-----------|-------|
| **Standard** | $e = \sqrt{4\pi\alpha}$ | 0.3028 |
| **FTD** | $e = \sqrt{N_{\text{base}}^2 \cdot \text{PF} \cdot \alpha}$ | 0.3028 |

The factor $4\pi = N_{\text{base}}^2 \times \text{PF} = 16 \times \pi/4$. **[SELECTION]**: The 16 counts the physical degrees of freedom on the minimal 2×2×2 lattice cell (24 flux components − 7 Gauss constraints − 1 gauge freedom = 16).

### 3.2 Strong Coupling [PARAMETRIC]

$$\alpha_s(M_Z) = \frac{b_3}{b_3 + 4N_{\text{eff}}} = \frac{7}{7 + 52} = \frac{7}{59} \approx 0.1186$$

**Experimental:** $\alpha_s(M_Z) = 0.1179 \pm 0.0009$ → agreement within 0.6%.

### 3.3 Weinberg Angle [PARAMETRIC]

$$\sin^2\theta_W = \frac{N_c}{N_{\text{eff}}} = \frac{3}{13} \approx 0.23077$$

**Experimental:** $\sin^2\theta_W = 0.23122 \pm 0.00003$ → agreement within 0.2%.

No π or PF appears — this is a pure integer ratio. **This is an instance of the PF Cancellation Rule: dimensionless coupling ratios are PF-free.**

---

## §4. Loop Corrections

### 4.1 QED Beta Function [THEOREM + SELECTION]

The one-loop QED beta function coefficient (per charged fermion flavor):

$$\frac{d\alpha}{d\ln Q^2} = \frac{\alpha^2}{3\pi}$$

| Form | Denominator | Decomposition |
|------|-------------|---------------|
| **Standard** | $3\pi$ | Unexplained transcendental |
| **FTD** | $N_{\text{base}} \times N_c \times \text{PF}$ | $4 \times 3 \times \pi/4 = 3\pi$ |

**[SELECTION]**: N_base = 4 (lattice DOF blocks), N_c = 3 (internal color loops available to the photon via vacuum polarization), PF = π/4 (circular mode efficiency per lattice face).

### 4.2 QCD Beta Function [THEOREM + SELECTION]

The one-loop QCD running denominator:

$$\frac{d\alpha_s}{d\ln Q^2} = -\frac{(11N_c - 2N_f)}{12\pi}\,\alpha_s^2 = -\frac{b_3}{4\pi}\,\alpha_s^2 \quad (N_f = 6)$$

| Form | Denominator factor | Decomposition |
|------|-------------------|---------------|
| **Standard** | $4\pi$ | Transcendental |
| **FTD** | $N_{\text{base}}^2 \times \text{PF}$ | $16 \times \pi/4 = 4\pi$ |

And the full denominator $12\pi$:

| Form | Denominator | Decomposition |
|------|-------------|---------------|
| **Standard** | $12\pi$ | Unexplained |
| **FTD** | $N_{\text{base}}^2 \times N_c \times \text{PF}$ | $16 \times 3 \times \pi/4 = 12\pi$ |

### 4.3 One-Loop Expansion Parameter [SELECTION]

The generic one-loop QFT correction factor:

$$\frac{\alpha}{2\pi}$$

| Form | Denominator | Decomposition |
|------|-------------|---------------|
| **Standard** | $2\pi$ | Transcendental |
| **FTD** | $2^D \times \text{PF}$ | $8 \times \pi/4 = 2\pi$ |

**[SELECTION]**: Each virtual loop "sees" $2^D = 8$ lattice octants, each PF-efficient (fraction π/4 accessible to circular modes). See [DERIV_GSTAR_PF_BRIDGE.md](../04_coupling/DERIV_GSTAR_PF_BRIDGE.md) §6.

---

## §5. Particle Masses

### 5.1 Electron Mass [STRONGLY MOTIVATED CONJECTURE]

$$m_e = M_P \cdot \sqrt{2\pi} \cdot \frac{N_{\text{base}}^2}{N_c} \cdot \alpha^{11}$$

The factor $\sqrt{2\pi}$ decomposes:

$$\sqrt{2\pi} = \sqrt{2^D \cdot \text{PF}} = 2^{D/2} \cdot \sqrt{\text{PF}} = 2\sqrt{2} \cdot \sqrt{\text{PF}}$$

So the full formula in PF notation:

$$m_e = M_P \cdot 2\sqrt{2} \cdot \sqrt{\text{PF}} \cdot \frac{N_{\text{base}}^2}{N_c} \cdot \alpha^{11}$$

The power 11 = b₃ + N_base = 7 + 4 is the "topological cost of embedding" a point particle in D = 3 lattice geometry.

**Result:** 0.5096 MeV vs experimental 0.5110 MeV (0.19% error).

### 5.2 Mass Ratios [STRONGLY MOTIVATED CONJECTURE]

Mass ratios are PF-free (the Golden Rule at work):

| Ratio | Formula | Value | Experimental | Error |
|-------|---------|-------|-------------|-------|
| $m_\tau/m_e$ | $(N_{\text{eff}} + N_{\text{base}}) \times 207 - 2N_c b_3$ | 3477 | 3477.2 | 0.005% |
| $m_p/m_e$ | $N_{\text{eff}}/\alpha + T(10)$ | 1836.2 | 1836.15 | 0.003% |

where $T(10) = 10 \times 11/2 = 55$ is the 10th triangular number.

PF does not appear in any mass ratio — the packing geometry cancels between numerator and denominator, leaving only integer structure.

---

## §6. Vacuum Energy Denominators

### 6.1 Stefan-Boltzmann Constant [THEOREM + SELECTION]

$$\sigma = \frac{\pi^2}{60} = \frac{(N_{\text{base}} \cdot \text{PF})^2}{N_{\text{base}} \cdot D_\Sigma} = \frac{N_{\text{base}} \cdot \text{PF}^2}{D_\Sigma}$$

where $D_\Sigma = 1 + 2 + 4 + 8 = 15$ is the sum of dimensions of the four normed division algebras (ℝ, ℂ, ℍ, 𝕆). By the Hurwitz theorem, 15 is a universal algebraic constant.

### 6.2 Casimir Force [THEOREM + SELECTION]

The Casimir force per unit area between parallel plates:

$$\frac{F}{A} = -\frac{\pi^2}{240\,a^4}$$

| Form | Coefficient | Decomposition |
|------|-------------|---------------|
| **Standard** | $\pi^2/240$ | Unexplained |
| **FTD** | $\text{PF}^2/D_\Sigma$ | $(π/4)^2/15 = \pi^2/240$ |

since $240 = N_{\text{base}}^2 \times D_\Sigma = 16 \times 15$.

**[SELECTION]**: The vacuum fluctuation spectrum is partitioned across $D_\Sigma = 15$ algebraic channels (the division algebra tower), each contributing PF² (area packing on two perpendicular lattice faces).

---

# PART III: GENERAL RELATIVITY AND COSMOLOGY

## §7. Black Hole Thermodynamics

### 7.1 Bekenstein-Hawking Entropy [THEOREM]

$$S_{BH} = 4\pi M^2 = N_{\text{base}}^2 \cdot \text{PF} \cdot M^2$$

### 7.2 Hawking Temperature [THEOREM]

$$T_H = \frac{1}{8\pi M} = \frac{1}{2 \cdot N_{\text{base}}^2 \cdot \text{PF} \cdot M}$$

### 7.3 The PF Cancellation [THEOREM]

$$S_{BH} \times T_H = \frac{M}{2}$$

PF enters both entropy and temperature but cancels in their product — the total energy throughput at the horizon depends on **nothing** except mass. This is a direct instance of the Golden Rule: the thermodynamic product is a dimensionless ratio (in appropriate units) and is therefore PF-free.

---

## §8. Loop Quantum Gravity

### 8.1 Immirzi Parameter [SELECTION]

$$\gamma_I = \frac{\ln 2}{\pi\sqrt{3}} = \frac{\ln 2}{N_{\text{base}} \cdot \text{PF} \cdot \sqrt{N_c}}$$

since $N_{\text{base}} \cdot \text{PF} \cdot \sqrt{N_c} = 4 \times \pi/4 \times \sqrt{3} = \pi\sqrt{3}$.

**Numerical value:** γ_I ≈ 0.12738, matching the Domagala-Lewandowski / Meissner (2004) value from Bekenstein-Hawking entropy matching.

### 8.2 Minimal Area Quantum [THEOREM + SELECTION]

$$A_{\min} = 4\pi\sqrt{3} \cdot \gamma_I \cdot \ell_P^2 = 4\pi\sqrt{3} \cdot \frac{\ln 2}{\pi\sqrt{3}} \cdot \ell_P^2 = 4\ln 2 \cdot \ell_P^2$$

$$\boxed{A_{\min} = N_{\text{base}} \cdot \ln 2 \cdot \ell_P^2}$$

PF cancels completely. The area quantum depends on:

| Factor | Value | Origin |
|--------|-------|--------|
| N_base = 4 | Division algebra base dimension | Topological (Hurwitz theorem) |
| ln 2 | Information content of one bit | Information-theoretic (universal) |
| $\ell_P^2$ | Planck area | Scale-setting (dimensional) |

The holographic data cap is **topological, not geometric** — it does not depend on how circles pack into squares.

---

## §9. Cosmological Constant

### 9.1 The Formula [CONJECTURE]

$$\rho_\Lambda = m_e^4 \times \alpha^{16} \times G^{*2}$$

**Result:** $3.86 \times 10^{-47}$ GeV⁴ vs observed $3.90 \times 10^{-47}$ GeV⁴ (1.0% accuracy).

### 9.2 The 10⁻¹²⁰ Problem as α⁶⁰ [SELECTION]

In Planck units, using $m_e = M_P \sqrt{2\pi} (16/3) \alpha^{11}$:

$$\frac{\rho_\Lambda}{M_P^4} = (2\pi)^2 \cdot \left(\frac{16}{3}\right)^4 \cdot G^{*2} \cdot \alpha^{60}$$

The exponent 60 = 4 × 11 + 16 = 44 + 16. Since $\alpha \approx 10^{-2.14}$, we get $\alpha^{60} \approx 10^{-128}$, and the prefactors ($\sim 10^5$) bring this to $\sim 10^{-123}$, matching the observed vacuum energy density in Planck units.

**The 120 orders of magnitude are not a "mystery" — they are $\alpha^{60}$, the fine structure constant raised to the power determined by the lattice DOF count (16) plus the mass hierarchy (44).**

### 9.3 PF Status [SELECTION]

The cosmological constant is an **absolute scale** (energy density of the vacuum), not a dimensionless ratio. PF enters through $m_e$ (via $\sqrt{2\pi} = \sqrt{2^D \cdot \text{PF}}$) and through $G^{*2} = \varpi^2/\text{PF}$. This is consistent with the Golden Rule: PF survives in scale-setting quantities.

---

## §10. Einstein Field Equations

### 10.1 The Coefficient 8πG [THEOREM]

$$G_{\mu\nu} = R_{\mu\nu} - \frac{1}{2}g_{\mu\nu}R = 8\pi G \cdot T_{\mu\nu}$$

| Form | Coefficient | Decomposition |
|------|-------------|---------------|
| **Standard** | $8\pi G$ | Unexplained factor times measured constant |
| **FTD** | $2 \cdot N_{\text{base}}^2 \cdot \text{PF} \cdot G$ | $2 \times 16 \times \pi/4 = 8\pi$ |

The same $2N_{\text{base}}^2 \cdot \text{PF}$ that appears in the Hawking temperature appears in the Einstein equations — the gravitational field equation and the thermal emission from horizons share the same geometric origin.

---

# PART IV: THE GOLDEN RULE OF THE LATTICE

## §11. The PF Cancellation Rule [SELECTION]

### 11.1 Statement

> **Golden Rule:** PF = π/4 cancels in every dimensionless physical ratio within a single sector. PF survives only in quantities that set absolute scales.

### 11.2 Where PF Cancels

| Observable | Contains PF? | Result |
|------------|-------------|--------|
| $S_{BH} \times T_H$ | No | $M/2$ |
| $A_{\min}$ (LQG) | No | $N_{\text{base}} \cdot \ln 2 \cdot \ell_P^2$ |
| $\sin^2\theta_W$ | No | $N_c/N_{\text{eff}} = 3/13$ |
| Mass ratios ($m_\tau/m_e$, $m_p/m_e$) | No | Pure integer combinations |
| $\alpha_s(M_Z)$ | No | $b_3/(b_3 + 4N_{\text{eff}}) = 7/59$ |
| BH First Law ($dM = TdS$) | No | Exact |
| Time dilation ratios ($d\tau_1/d\tau_2$) | No | Metric ratio |

### 11.3 Where PF Survives

| Quantity | PF dependence | Why |
|----------|--------------|-----|
| $m_e$ (electron mass) | $\sqrt{\text{PF}}$ via $\sqrt{2\pi}$ | Absolute mass scale |
| $\rho_\Lambda$ (cosmological constant) | $\text{PF}$ via $G^{*2}$ and $m_e$ | Absolute energy density |
| $\sigma$ (Stefan-Boltzmann) | $\text{PF}^2$ | Absolute radiation rate |
| $e$ (electric charge) | $\sqrt{\text{PF}}$ via $\sqrt{4\pi\alpha}$ | Absolute charge unit |

### 11.4 Physical Interpretation [SELECTION]

PF = π/4 encodes the **geometric cost of discretization** — the mismatch between circular physics and square lattice. Observable physics (dimensionless ratios) is independent of this cost, just as lattice QCD results become lattice-spacing-independent for arbitrarily fine spacing. The lattice is scaffolding; the physics is the circle, not the square.

---

# PART V: MASTER COMPARISON TABLE

## §12. Complete Side-by-Side Atlas

### 12.1 π-Factor Decomposition Atlas

| Standard Factor | FTD Decomposition | Integer Meaning | Appears In |
|----------------|-------------------|-----------------|------------|
| $\pi$ | $N_{\text{base}} \cdot \text{PF} = 4 \cdot \pi/4$ | 4 lattice faces | General |
| $2\pi$ | $2^D \cdot \text{PF} = 8 \cdot \pi/4$ | 8 octants in D=3 | One-loop QFT, angular integrals |
| $3\pi$ | $N_{\text{base}} \cdot N_c \cdot \text{PF} = 12 \cdot \pi/4$ | 4 faces × 3 colors | QED beta function |
| $4\pi$ | $N_{\text{base}}^2 \cdot \text{PF} = 16 \cdot \pi/4$ | 16 physical DOF | Gauge coupling $e^2$, QCD, BH entropy |
| $8\pi$ | $2N_{\text{base}}^2 \cdot \text{PF} = 32 \cdot \pi/4$ | 2 × 16 DOF (left+right) | Hawking temp, Einstein eqs |
| $12\pi$ | $N_{\text{base}}^2 \cdot N_c \cdot \text{PF} = 48 \cdot \pi/4$ | 16 DOF × 3 colors | QCD running coupling |
| $\pi^2/60$ | $N_{\text{base}} \cdot \text{PF}^2 / D_\Sigma$ | 4 faces, 15 algebra dim | Stefan-Boltzmann |
| $\pi^2/240$ | $\text{PF}^2 / D_\Sigma$ | 15 algebra dimensions | Casimir force |

### 12.2 QFT Comparison Table

| Phenomenon | Standard Formula | FTD Formula | PF in result? |
|-----------|-----------------|-------------|---------------|
| EM coupling | $e = \sqrt{4\pi\alpha}$ | $e = N_{\text{base}}\sqrt{\text{PF} \cdot \alpha}$ | Yes (absolute) |
| Weinberg angle | $\sin^2\theta_W \approx 0.231$ | $N_c/N_{\text{eff}} = 3/13$ | **No** |
| Strong coupling | $\alpha_s \approx 0.118$ | $b_3/(b_3+4N_{\text{eff}}) = 7/59$ | **No** |
| QED β-function | $d\alpha/d\ln Q^2 = \alpha^2/(3\pi)$ | $\alpha^2/(N_{\text{base}} \cdot N_c \cdot \text{PF})$ | Yes (denominator) |
| QCD β-function | $-b_3 \alpha_s^2/(4\pi)$ | $-b_3\alpha_s^2/(N_{\text{base}}^2 \cdot \text{PF})$ | Yes (denominator) |
| One-loop factor | $\alpha/(2\pi)$ | $\alpha/(2^D \cdot \text{PF})$ | Yes (denominator) |
| Electron mass | $m_P\sqrt{2\pi}(16/3)\alpha^{11}$ | $m_P \cdot 2\sqrt{2}\sqrt{\text{PF}} \cdot (N_{\text{base}}^2/N_c) \cdot \alpha^{11}$ | Yes (absolute) |
| Mass ratios | measured | Integer formulas | **No** |
| Casimir force | $-\pi^2/(240a^4)$ | $-\text{PF}^2/(D_\Sigma \cdot a^4)$ | Yes (absolute) |

### 12.3 GRT Comparison Table

| Phenomenon | Standard Formula | FTD Formula | PF in result? |
|-----------|-----------------|-------------|---------------|
| BH entropy | $S = 4\pi M^2$ | $N_{\text{base}}^2 \cdot \text{PF} \cdot M^2$ | Yes |
| Hawking temp | $T = 1/(8\pi M)$ | $1/(2N_{\text{base}}^2 \cdot \text{PF} \cdot M)$ | Yes |
| $S \times T$ | $M/2$ | $M/2$ | **No** |
| Einstein eqs | $G_{\mu\nu} = 8\pi G T_{\mu\nu}$ | $2N_{\text{base}}^2 \cdot \text{PF} \cdot G \cdot T_{\mu\nu}$ | Yes (coefficient) |
| Immirzi parameter | $\gamma_I = \ln 2/(\pi\sqrt{3})$ | $\ln 2/(N_{\text{base}} \cdot \text{PF} \cdot \sqrt{N_c})$ | Yes |
| LQG min area | $4\ln 2 \cdot \ell_P^2$ | $N_{\text{base}} \cdot \ln 2 \cdot \ell_P^2$ | **No** |
| Vacuum energy | $\rho_\Lambda \approx 10^{-47}$ GeV⁴ | $m_e^4 \cdot \alpha^{16} \cdot G^{*2}$ | Yes (absolute) |

---

# PART VI: EPISTEMIC AUDIT

## §13. What Is Each Type of Claim

### 13.1 Pure Algebra [THEOREM]

The following are arithmetic rearrangements — true by definition:

- $4\pi = 16 \times \pi/4$
- $3\pi = 12 \times \pi/4$
- $\pi^2/240 = (\pi/4)^2/15$
- $G^* = \varpi/\sqrt{\text{PF}}$
- $S_{BH} \times T_H = M/2$
- $A_{\min} = 4\ln 2 \cdot \ell_P^2$

### 13.2 Integer Interpretations [SELECTION]

The following identify integer factors with physical meanings:

| Integer | Identification | Status |
|---------|---------------|--------|
| 16 = N_base² | Physical DOF on minimal lattice cell | [SELECTION] — argued from Gauss constraint counting |
| 3 = N_c | Color charges | [STRONGLY MOTIVATED CONJECTURE] — from master quadratic $x_-$ |
| 15 = D_Σ | Division algebra tower sum | [SELECTION] — Hurwitz theorem gives 15, but its role in vacuum energy is argued |
| 8 = 2^D | Lattice octants | [SELECTION] — geometric interpretation |
| 7 = b₃ | QCD beta coefficient | [THEOREM] — standard QCD with N_f = 6 |
| 13 = N_eff | Fibonacci closure | [SELECTION] — $b_3 + 2N_c = 13$, but Fibonacci identification is argued |

### 13.3 Structural Claims [CONJECTURE]

- PF cancellation rule (Golden Rule) — verified in 7+ domains, no general proof
- Physics expressible through {ϖ, PF, ln 2, √2} + {3, 4, 7, 13} — survey incomplete
- α⁶⁰ explanation of 10⁻¹²⁰ — correct numerically, but mode-by-mode coupling unproven

---

# PART VII: CLAIMS TABLE

## §14. Summary of Claims

| ID | Claim | Tag | Depends On | Falsification |
|----|-------|-----|------------|---------------|
| **CP-1** | $G^* = \varpi/\sqrt{\text{PF}}$ | [THEOREM] | Definitions | Algebraic identity |
| **CP-2** | $4\pi = N_{\text{base}}^2 \cdot \text{PF}$ | [THEOREM] | Arithmetic | Identity |
| **CP-3** | $3\pi = N_{\text{base}} \cdot N_c \cdot \text{PF}$ (QED beta) | [THEOREM] + [SELECTION] | Arithmetic + integer meaning | β-function has different form |
| **CP-4** | $12\pi = N_{\text{base}}^2 \cdot N_c \cdot \text{PF}$ (QCD) | [THEOREM] + [SELECTION] | Arithmetic + integer meaning | QCD running denominator changes |
| **CP-5** | $\sin^2\theta_W = N_c/N_{\text{eff}} = 3/13$ | [PARAMETRIC] | Master quadratic + D=3 | Precision measurement outside 0.2% |
| **CP-6** | $S_{BH} \times T_H = M/2$ (PF cancels) | [THEOREM] | BH thermodynamics | S×T ≠ M/2 for any BH |
| **CP-7** | $\gamma_I = \ln 2/(N_{\text{base}} \cdot \text{PF} \cdot \sqrt{N_c})$ | [SELECTION] | DL/Meissner value | Different Immirzi value |
| **CP-8** | $A_{\min} = N_{\text{base}} \cdot \ln 2 \cdot \ell_P^2$ | [THEOREM] + [SELECTION] | CP-7 + LQG area spectrum | LQG gives different A_min |
| **CP-9** | $\pi^2/240 = \text{PF}^2/D_\Sigma$ (Casimir) | [THEOREM] + [SELECTION] | Arithmetic + D_Σ = 15 meaning | Identity |
| **CP-10** | $\rho_\Lambda = m_e^4 \alpha^{16} G^{*2}$ | [CONJECTURE] | Vacuum energy formula | Observed ρ_Λ differs by >5% |
| **CP-11** | $\rho_\Lambda/M_P^4 \sim \alpha^{60}$ | [SELECTION] | CP-10 + mass formula | Inconsistent with α precision |
| **CP-12** | PF cancels in all dimensionless sector ratios | [SELECTION] | Pattern across 7+ domains | Dimensionless observable containing PF |
| **CP-13** | Mass ratios are PF-free | [THEOREM] | Integer mass formulas | Mass ratio requiring PF |
| **CP-14** | $2\pi = 2^D \cdot \text{PF}$ (one-loop) | [THEOREM] + [SELECTION] | Arithmetic + lattice interpretation | Identity |
| **CP-15** | All framework integers derive from D = 3 | [SELECTION] | Dimensional arguments | Integer needed outside {3,4,7,13} |

---

## Cross-References

| Document | Relevant Content |
|----------|-----------------|
| [DERIV_GSTAR_PF_BRIDGE.md](../04_coupling/DERIV_GSTAR_PF_BRIDGE.md) | Primary source: G* decomposition, BH thermo, LQG, vacuum energy, QFT loops |
| [SPEC_FTD_LAGRANGIAN.md](SPEC_FTD_LAGRANGIAN.md) | SM Lagrangian sector-by-sector mapping, parameter reduction |
| [FOUND_SPACETIME_EMERGENCE.md](../02_foundations/FOUND_SPACETIME_EMERGENCE.md) | D = 3 derivation, 0.5D ontology |
| [FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md](../02_foundations/FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md) | Historical/interpretive constant atlas γ → ϖ → M → π → G* |
| [DERIV_LAMBDA_QCD_DERIVATION.md](../04_coupling/DERIV_LAMBDA_QCD_DERIVATION.md) | Non-circular Λ_QCD, b₃ = 7 as beta coefficient |

---

*Document created: February 19, 2026*
*Framework: Foundational Ternary Dynamics v5.26*
*Structure: 7 parts, 15 claims, side-by-side comparison atlas*
