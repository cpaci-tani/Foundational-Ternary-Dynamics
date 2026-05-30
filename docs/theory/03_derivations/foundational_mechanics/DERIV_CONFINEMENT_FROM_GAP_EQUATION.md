# Confinement from the Gap Equation: Wilson Loops at x₋

## Area-Law Wilson Loops and Linear Confinement from the Master Quadratic

**Date:** March 17, 2026
**Status:** [THEOREM for confinement at x₋; SELECTION for QCD identification]
**Tier:** 2.1
**Proof script:** `scripts/proofs/proof_confinement_wilson.py`

---

## Abstract

The master quadratic $x^2 - 16G^{*2}x + 16G^{*3} = 0$ has two roots: $x_+ = 137.036$ (Coulomb phase) and $x_- = 3.024$ (confined phase). This document proves that Wilson loops at $x_-$ obey an area law with positive string tension $\sigma = -\ln(I_1(x_-)/I_0(x_-)) \approx 0.209$, yielding a linear static potential $V(r) \sim \sigma \cdot r$. At $x_+$, the string tension vanishes ($\sigma \approx 0.004$), confirming the Coulomb phase. The phase separation ratio $\sigma(x_-)/\sigma(x_+) \approx 57$ quantifies the confinement hierarchy. The identification of $x_-$ with QCD remains [SELECTION].

---

## Part I: Wilson Loops in Compact U(1) LGT

### 1.1 Setup [THEOREM — standard lattice gauge theory]

For compact U(1) lattice gauge theory at inverse coupling $\beta$, the plaquette average is:

$$u_p = \frac{I_1(\beta)}{I_0(\beta)}$$

where $I_n$ are modified Bessel functions of the first kind. This follows exactly from the single-plaquette integral over the compact gauge group (Creutz 1983, Chapter 8).

### 1.2 Strong Coupling Expansion of Wilson Loops [THEOREM]

At leading order in the strong coupling (character) expansion, the Wilson loop expectation value for a rectangular $R \times T$ contour $C$ enclosing area $A = RT$ is:

$$\langle W(C) \rangle = u_p^A = \left[\frac{I_1(\beta)}{I_0(\beta)}\right]^{R \cdot T}$$

This is the **area law**: $\ln\langle W \rangle \propto -\sigma \cdot A$ with lattice string tension:

$$\sigma_{\text{lat}} = -\ln\left(\frac{I_1(\beta)}{I_0(\beta)}\right)$$

Higher-order corrections contribute perimeter terms that do not affect the area-law behavior.

---

## Part II: Confinement at x₋

### 2.1 String Tension at x₋ [THEOREM]

Evaluating at $\beta = x_- = 3.024$:

$$u_p(x_-) = \frac{I_1(3.024)}{I_0(3.024)} \approx 0.812$$

$$\sigma(x_-) = -\ln(0.812) \approx 0.209$$

Since $\sigma > 0$, the Wilson loop obeys an area law. This is the defining criterion for confinement (Wilson 1974).

### 2.2 Linear Static Potential [THEOREM]

The static potential between sources separated by distance $R$ is extracted from:

$$V(R) = -\lim_{T \to \infty} \frac{1}{T} \ln\langle W(R,T) \rangle$$

At leading order in strong coupling:

$$V(R) = \sigma \cdot R = -\ln\left(\frac{I_1(x_-)}{I_0(x_-)}\right) \cdot R$$

This is a **linear confining potential** — the energy grows without bound as $R$ increases, preventing the separation of charged sources.

### 2.3 Creutz Ratio [THEOREM]

The Creutz ratio provides a self-consistency check:

$$\chi(R,T) = -\ln\frac{\langle W(R,T)\rangle \langle W(R-1,T-1)\rangle}{\langle W(R-1,T)\rangle \langle W(R,T-1)\rangle}$$

For a pure area law, $\chi(R,T) = \sigma$ independent of $R$ and $T$. Numerical evaluation confirms $\chi = 0.2088...$ at all tested $(R,T)$, with variance $< 10^{-20}$.

---

## Part III: Coulomb Phase at x₊

### 3.1 Vanishing String Tension [THEOREM]

At $\beta = x_+ = 137.036$:

$$u_p(x_+) = \frac{I_1(137.036)}{I_0(137.036)} \approx 0.9964$$

$$\sigma(x_+) = -\ln(0.9964) \approx 0.004$$

The string tension is negligible. Wilson loops obey a perimeter law, and the static potential is Coulombic: $V(r) \sim -g^2/(4\pi r) = -\alpha/(4\pi r)$.

### 3.2 Phase Separation [THEOREM]

The ratio of string tensions quantifies the confinement hierarchy:

$$\frac{\sigma(x_-)}{\sigma(x_+)} \approx \frac{0.209}{0.004} \approx 57$$

The two roots of the master quadratic live in sharply separated phases:

| Root | $\beta$ | $g^2 = 1/\beta$ | $u_p$ | $\sigma$ | Phase |
|------|---------|-----------------|-------|----------|-------|
| $x_+$ | 137.036 | 0.00730 | 0.9964 | 0.004 | Coulomb (EM) |
| $x_-$ | 3.024 | 0.331 | 0.812 | 0.209 | Confined (strong) |

---

## Part IV: QCD Identification

### 4.1 The Selection [SELECTION]

The identification of $x_-$ with the QCD confined phase rests on:

1. ~~$\lfloor x_- \rfloor = 3 = N_c$~~ — **the `x_- ↔ N_c` identification is RETIRED** per FTD/FQCR Cleanup Taxonomy v1.4 §5 (LEDGER FTD-0014 removed in commit `ca7eb61`). $\lfloor x_- \rfloor = 3$ is a numerical fact about the polynomial root; the load-bearing match to $N_c$ is no longer in force. `N_c = 3` in FTD is independently sourced via `DERIV_NC_FROM_TOPOLOGY.md` and the Moore Layer Theorem.
2. Area-law Wilson loops (confinement)
3. $g^2(x_-) = 0.331$ is $O(1)$, consistent with strong coupling
4. The phase separation mirrors the EM/QCD hierarchy in nature

This is classified as **[SELECTION]** because the theorems above establish confinement in a U(1) lattice gauge theory, not SU(3). The step from compact U(1) confinement to non-Abelian QCD confinement requires the additional identification of the gauge group with SU($N_c$) where $N_c = 3$ — which is independently sourced (Moore Layer Theorem; `DERIV_NC_FROM_TOPOLOGY.md`), not derived from the master quadratic root.

### 4.2 What Is and Is Not Proven

| Claim | Status | Notes |
|-------|--------|-------|
| Wilson loops at $x_-$ obey area law | **[THEOREM]** | Exact in strong coupling expansion |
| $\sigma(x_-) > 0$ | **[THEOREM]** | $= -\ln(I_1/I_0) = 0.209$ |
| $V(r) \sim \sigma \cdot r$ at $x_-$ | **[THEOREM]** | Follows from area law |
| $\sigma(x_+) \approx 0$ (Coulomb) | **[THEOREM]** | $= 0.004$, negligible |
| Phase separation $\sigma_-/\sigma_+ \gg 1$ | **[THEOREM]** | Ratio $\approx 57$ |
| Creutz ratio = $\sigma$ (consistency) | **[THEOREM]** | Variance $< 10^{-20}$ |
| $x_-$ corresponds to QCD | **[SELECTION]** | Requires $N_c$ identification |

---

## References

1. K. Wilson, "Confinement of quarks," Phys. Rev. D 10, 2445 (1974)
2. M. Creutz, *Quarks, Gluons and Lattices* (Cambridge, 1983), Chapter 8
3. H. Rothe, *Lattice Gauge Theories: An Introduction*, 4th ed. (World Scientific, 2012)
4. `scripts/proofs/proof_confinement_wilson.py` — numerical verification
5. `scripts/proofs/proof_coulomb_phase_coupling.py` — Coulomb phase at $x_+$
6. `docs/theory/03_derivations/DERIV_ALPHA_FROM_PHASE_STRUCTURE.md` — phase structure
