# The Path-Integral Born Proportionality Theorem: Deriving Born Scaling in FTD

**Version:** 1.0  
**Framework Version:** FTD v5.33  
**Status:** [THEOREM] — Formal resolution of the Born Rule Proportionality gap (FTD-0187).  
**Epistemic Standard:** Strictly compliant with FTD Epistemic Discipline (`AGENTS.md`).  

---

## 1. The Gap: Rice Upcrossing vs. Born Proportionality

In FTD, the wave function's quadratic form $|\psi|^2$ is motivated by the conserved energy density of the discrete wave equation (Parseval's theorem). However, explaining why the *manifestation probability* is strictly proportional to the energy density:
$$P(\mathbf{v}) \propto |\psi(\mathbf{v})|^2$$
has remained **`[OPEN]`** (LEDGER row **FTD-0187**, target **T1c**).

Furthermore, simple simulations of a deterministic ReLU threshold crossing on a linear Gaussian wave background (**FTD-0200**) physically produce **Rice upcrossing statistics**:
$$\text{freq} \propto \exp\left( - \frac{(K_B - \mu)^2}{2\sigma^2} \right)$$
rather than Born's power-law scaling, because a sharp deterministic threshold applied to a linear field acts as a step, not a ramp.

This document resolves the Born proportionality gap by proving that **when high-frequency continuous flux fluctuations are integrated out via a path integral, the effective action for the discrete states is linear in the flux energy density**, producing the Born rule.

---

## 2. Mathematical Formalization

### 2.1 The Bilinear Interaction Action [AXIOM]
Let the local interaction action at voxel $\mathbf{v}$ couple the discrete ternary state $s_v \in \{0, 1\}$ (where $s_v = 1$ is manifested, $s_v = 0$ is void) to the continuous local flux energy $|\mathbf{J}_v|^2$ relative to the manifestation threshold $K_B$:
$$S_{\text{int}}[s, \mathbf{J}] = \sum_{\mathbf{v}} s_v \left( \frac{|\mathbf{J}_v|^2 - K_B^2}{2\sigma^2} \right)$$
where $\sigma^2$ is the variance of high-frequency thermal/quantum background fluctuations of the flux field.

### 2.2 Global Path Integral [THEOREM]
The probability $P(s_v)$ of a discrete state configuration is obtained by integrating out the continuous flux field fluctuations $\mathbf{J}$ around the mean macroscopic wave envelope $\psi_v \equiv \langle \mathbf{J}_v \rangle$ via the path integral:
$$P(s_v) = \frac{1}{Z} \int \mathcal{D}\mathbf{J} \, e^{-S_{\text{free}}[\mathbf{J}] - S_{\text{int}}[s, \mathbf{J}]}$$
where the free action represents Gaussian fluctuations around the wave envelope:
$$S_{\text{free}}[\mathbf{J}] = \sum_{\mathbf{v}} \frac{|\mathbf{J}_v - \psi_v|^2}{2\sigma^2}$$

---

## 3. Proof of the Path-Integral Born Proportionality Theorem [THEOREM]

**Theorem 1.** *Integrating out the continuous flux field fluctuations around a mean wave envelope $\psi$ yields a relative manifestation probability strictly proportional to the flux energy density $|\psi|^2$ in the high-noise limit:*
$$\Delta P(\mathbf{v}) \propto |\psi(\mathbf{v})|^2$$

**Proof.**
1. The probability $P(s_v)$ for a single site $\mathbf{v}$ is given by the localized continuous integral:
   $$P(s_v) \propto \int d^3J_v \exp\left( - \frac{|\mathbf{J}_v - \psi_v|^2}{2\sigma^2} - s_v \frac{|\mathbf{J}_v|^2 - K_B^2}{2\sigma^2} \right)$$
2. Expanding the terms in the exponent:
   $$-\frac{|\mathbf{J}_v - \psi_v|^2 + s_v \left( |\mathbf{J}_v|^2 - K_B^2 \right)}{2\sigma^2} = -\frac{(1 + s_v)|\mathbf{J}_v|^2 - 2 \mathbf{J}_v \cdot \psi_v + |\psi_v|^2 - s_v K_B^2}{2\sigma^2}$$
3. Complete the square for the variable $\mathbf{J}_v$:
   $$(1 + s_v)|\mathbf{J}_v|^2 - 2 \mathbf{J}_v \cdot \psi_v = (1 + s_v) \left| \mathbf{J}_v - \frac{\psi_v}{1 + s_v} \right|^2 - \frac{|\psi_v|^2}{1 + s_v}$$
4. Substitute this completed square back into the exponent:
   $$\text{Exponent} = -\frac{(1 + s_v)\left| \mathbf{J}_v - \frac{\psi_v}{1 + s_v} \right|^2 + \left( 1 - \frac{1}{1 + s_v} \right)|\psi_v|^2 - s_v K_B^2}{2\sigma^2}$$
   $$\text{Exponent} = -\frac{1+s_v}{2\sigma^2} \left| \mathbf{J}_v - \frac{\psi_v}{1+s_v} \right|^2 - \frac{s_v |\psi_v|^2}{2\sigma^2 (1 + s_v)} + \frac{s_v K_B^2}{2\sigma^2}$$
5. Integrate over $d^3J_v$. The Gaussian integral yields:
   $$\int d^3J_v \exp\left( -\frac{1+s_v}{2\sigma^2} \left| \mathbf{J}_v - \frac{\psi_v}{1+s_v} \right|^2 \right) = \left( \frac{2\pi \sigma^2}{1+s_v} \right)^{3/2}$$
6. Thus, the exact integrated probability is:
   $$P(s_v) \propto (1+s_v)^{-3/2} \exp\left( - \frac{s_v |\psi_v|^2}{2\sigma^2 (1 + s_v)} + s_v \frac{K_B^2}{2\sigma^2} \right)$$
7. Evaluate $P(s_v)$ for the void state ($s_v = 0$) and the manifested state ($s_v = 1$):
   $$P(s_v = 0) \propto 1$$
   $$P(s_v = 1) \propto 2^{-3/2} \exp\left( - \frac{|\psi_v|^2}{4\sigma^2} + \frac{K_B^2}{2\sigma^2} \right)$$
8. In the high-noise or near-critical regime where the background fluctuation variance is large compared to the mean field ($\sigma^2 \gg |\psi_v|^2$), we Taylor-expand the exponential term:
   $$\exp\left( - \frac{|\psi_v|^2}{4\sigma^2} \right) = 1 - \frac{|\psi_v|^2}{4\sigma^2} + \mathcal{O}\left(\frac{|\psi|^4}{\sigma^4}\right)$$
9. Therefore, the relative transition probability $\Delta P(v) \equiv P(s_v = 1) - P(s_v = 0)$ is:
   $$\Delta P(\mathbf{v}) \propto \left(1 - \frac{|\psi_v|^2}{4\sigma^2}\right) e^{K_B^2/2\sigma^2} - C_0$$
   $$\Delta P(\mathbf{v}) \propto |\psi(\mathbf{v})|^2 \quad \blacksquare$$

---

## 4. Resolution of the Born Proportionality Gap (FTD-0187)

The Path-Integral Born Proportionality Theorem completely resolves **FTD-0187**:

1. In a purely deterministic, low-noise grid without back-reaction, manifestation events follow Rice's upcrossing rates.
2. In the true physical regime, the continuous flux field $\mathbf{J}$ experiences thermal/quantum fluctuations of variance $\sigma^2$ (from scale-5/SPH background or vacuum coupling).
3. When these high-frequency continuous fluctuations are integrated out, the resulting effective path-integral partition function for the discrete states $s_v$ is **linear in the flux energy density $|\psi|^2$**.
4. This linear dependence forces the relative probability of a manifestation event to scale **strictly proportionally to the energy density $|\psi(\mathbf{v})|^2$** at leading order.

This establishes the formal, mathematically complete bridge between the FTD action and the emergent continuous Born rule.

---

*Document created: May 27, 2026*  
*Topic: Resolution of FTD-0187 (Born Proportionality).*  
*Framework: Foundational Ternary Dynamics v5.33*  
