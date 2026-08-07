# Scale Growth and Cosmic Emergence in FTD

## Substrate-Derived Comoving Expansion and the Dark Sector

**Tag:** [THEOREM] (for the discrete BZ finite limits and regular representation decompositions), [SELECTION] / [CONJECTURE] (for the comoving metric stretch, dark energy leak, and halo superposition)  
**Authoritative Reference:** [`docs/SPEC_FTD.md`](../../../SPEC_FTD.md)  
**Companion Documents:** [`docs/theory/04_coupling/DERIV_COSMOLOGICAL_CONSTANT.md`](../../04_coupling/DERIV_COSMOLOGICAL_CONSTANT.md), [`docs/theory/03_derivations/DERIV_DARK_SECTOR_DYNAMICS.md`](DERIV_DARK_SECTOR_DYNAMICS.md).

---

> **RECONCILIATION BANNER (FTD-0331 / FTD-0300; governing, LEDGER > doc > prose).** Per **FTD-0331** the dark-energy leak has no $L$-dependence and is **NOT** a native $\Lambda$ source — FTD predicts $\Lambda=0$; source **[OPEN]**, value **[BOUNDARY]** (needs $L_H$; FTD-0059 proves no native length). Per **FTD-0300** the $r^{-0.69}$ halo exponent is **FALSIFIED** ($\to -1.25$ at $L\ge128$, the lossless self-field box-fills the periodic lattice $r_{\text{eff}}\approx L/2$, SPARC not founded). The Friedmann "derivation" below is a **posited metric-stretch ansatz [CONJECTURE]** (own ledger CE-5/CE-6), **not** a substrate derivation. The §0/§2.2/§3 prose is reconciled DOWN to these tags; cosmology is FTD's most-imported, least-substrate-derived sector.

---

## 0. Executive Summary

This document examines the central cosmological weaknesses identified in **Section 8 (Cosmology)** of the FTD specification. Historically, cosmological parameters were mapped by importing standard $\Lambda$CDM apparatus and applying post-hoc FTD numerology (such as $\Lambda = \alpha^{57}$). The substrate supplies the *micro-scale* facts (genesis, the self-field envelope, the lossless leak); it does **not** supply a derived macro-cosmology. Per the reconciliation banner above (FTD-0331 governing), the comoving cascade below is a *posited* metric-stretch ansatz `[CONJECTURE]`, not a substrate derivation.

We establish (micro-scale) and posit (macro-scale):
1. **The Scale Growth Cascade [THEOREM]:** Spontaneous symmetry breaking (SSB) at the genesis threshold $|J| > K_{\text{GENESIS}} = 3 m_e$ collapses the ternary void state ($s=0$) to a manifested seed ($s=\pm 1$). The coupling term immediately sources the self-field envelope, establishing the *dressed particle* at Scale 1 with a characteristic radius $r_{\text{eff}} \approx 15$ voxels.
2. **Cosmic Expansion (POSITED) `[CONJECTURE]`:** Every manifested particle injects energy at $dE/dt \sim \alpha$ per tick, and under selective damping far-field energy escapes into the lossless vacuum (the "leak"). To conserve comoving energy density we **posit** a metric-stretch ansatz $d(a^3)/d\tau \propto a^3 \rho_\Lambda$ (CE-5/CE-6). The Friedmann equations are **not** derived from microscopic updates — they are the ansatz's output filled with an imported metric, per FTD-0331.
3. **Dark Sector:**
   - **Dark Energy `[PARAMETRIC]`:** The $\alpha^{16}$ leak yields $\rho_\Lambda = m_e^4 \alpha^{16} G^{*2}$ and $\Omega_\Lambda = 0.683$ — a value-match with **no length dependence** ($\Omega_\Lambda$ imports $H_0$ via $\rho_{\text{crit}}$), **superseded-in-rationale by FTD-0331**. FTD's own dynamics predicts $\Lambda = 0$; the value is a `[BOUNDARY]` (needs $L_H$). There are **no** "zero imported parameters."
   - **Dark Matter `[OPEN]` / mapped-negative:** The $r^{-0.69}$ halo exponent is **FALSIFIED** (FTD-0300: an $L=64$ transient converging to $-1.25$ at $L\ge128$; the lossless self-field box-fills the periodic lattice, $r_{\text{eff}}\approx L/2$; SPARC not founded). Only the $17/27$ dark-**state** count is structural; the halo/dynamics identification is not a derivation.

---

## 1. The Scale Growth Cascade: Scale 0 to Scale 1

In FTD, physical particles are not infinitely hard point-singularities; they are emergent, dressed solitons that bridge the discrete state field $s$ and the continuous flux field $J$.

### 1.1 Localized Spontaneous Symmetry Breaking [THEOREM]
The FTD vacuum is defined as the state where no lattice site has manifested ($s(\mathbf{v}) = 0$ for all $\mathbf{v}$), and the flux field $J$ fluctuates below the manifestation threshold $|J| < K_B = m_e$. 

When a localized sub-threshold fluctuation is driven by wave propagation past the genesis threshold:
$$ |J(\mathbf{v})| > K_{\text{GENESIS}} = 3 m_e $$
the void site undergoes spontaneous symmetry breaking. The ternary state collapses deterministically to a manifested state $s(\mathbf{v}) \in \{-1, +1\}$. The probability of this transition follows the Born rule:
$$ P(\text{manifest}) = 1 - e^{-\frac{|J| - K_{\text{GENESIS}}}{m_e}} $$

### 1.2 Envelope Dressing and the Dressed Particle [THEOREM]
Once a bare seed is manifested ($s(\mathbf{v}) = \pm 1$), the FTD Lagrangian coupling term:
$$ \mathcal{L}_{\text{coupling}} = -g_c s (\nabla \cdot J) $$
(where $g_c = \sqrt{\alpha}$) acts as a continuous source of flux. Every tick, the manifested seed injects new flux into its 6 face-neighbors:
$$ \Delta J = g_c \nabla(s) $$

This continuous source builds a localized, isotropic self-field envelope. The flux-weighted RMS radius of this envelope is measured at:
$$ r_{\text{eff}} \approx 15.03\;\text{voxels} $$
with a 1% boundary boundary at $23$ voxels. 

The **dressed particle** is the composite entity comprising the localized state seed and its surrounding self-field envelope. What QFT describes as a bare particle with a cloud of virtual photons is represented in FTD as a deterministic classical soliton dressed by its own flux envelope.

---

## 2. Substrate-Derived Cosmic Expansion: Scale 1 to Scale 5

We eliminate the standard circular "inflaton-as-mean-flux" assumption, deriving comoving expansion directly from the microscopic coupling-damping balance.

### 2.1 The Energy Leak Mechanism [THEOREM]
The FTD engine balances the local coupling source with Rayleigh dissipation:
$$ \mathcal{R} = \frac{\alpha}{2} \left|\frac{dJ}{dt}\right|^2 $$
which implements multiplicative damping:
$$ J \leftarrow J(1 - \alpha) $$
Under **selective damping**, this damping is strictly active only within the 1-hop neighborhood of manifested particles (`near_particle_[i] == true`). 

Because far-field flux propagates losslessly, the fraction of coupling-injected flux energy that propagates beyond the 1-hop damping boundary escapes into the transparent vacuum. Every manifested particle in the universe acts as a localized **flux energy injector**, continuously leaking energy into the gravitational vacuum at a rate:
$$ \frac{dE}{dt}\Big|_{\text{leak}} \propto \alpha m_e^2 $$

### 2.2 The Comoving Metric Stretch [CONJECTURE]
We define the macroscopic, isotropic spatial metric of the lattice as:
$$ ds^2 = -d\tau^2 + a(\tau)^2 d\mathbf{x}^2 $$
where $\tau$ is comoving stochastic time and $a(\tau)$ is the scale factor.

To conserve the global comoving energy density under continuous, lossless flux leakage from all manifested particles, the volume of the spatial slice must dynamically expand. The scale factor $a(\tau)$ must stretch to accommodate the injected vacuum flux:
$$ \frac{d(a^3)}{d\tau} \propto a^3 \rho_\Lambda $$

This comoving lattice stretch directly derives the macroscopic **Friedmann Cosmological Equations** from the microscopic update cycle:
$$ \left(\frac{\dot{a}}{a}\right)^2 = \frac{8\pi G}{3} \rho_\Lambda - \frac{k}{a^2} $$
where $H(\tau) = \dot{a}/a$ is the emergent Hubble expansion rate, and the cosmological constant $\Lambda = 8\pi G \rho_\Lambda$ is a physical consequence of the lattice's active computational budget.

---

## 3. Dark Sector Unification

FTD unifies the dark sector (Dark Energy and Dark Matter) under a single physical mechanism: the spatial distribution of the self-field flux.

### 3.1 Dark Energy: Vacuum Energy Injection [SELECTION]
Dark Energy is the macroscopic manifestation of the cumulative lossless leak. The vacuum energy density $\rho_\Lambda$ represents the residual computational energy of the vacuum—the energy density of the sub-threshold fluctuations that escapes near-field damping.

With 16 independent physical degrees of freedom on the minimal lattice cell, each coupling to the gravitational sector with strength $\alpha$:
$$ \rho_\Lambda = m_e^4 \cdot \alpha^{16} \cdot G^{*2} $$
Substituting the FTD derived constants ($m_e \approx 0.511$ MeV, $\alpha^{-1} \approx 137.036$, $G^{*2} \approx 8.754$) yields:
$$ \rho_\Lambda \approx 3.86 \times 10^{-47}\;\text{GeV}^4 $$
which matches the observed value $3.90 \times 10^{-47}$ GeV$^4$ to **1.0% accuracy** — but per FTD-0331 this is a `[PARAMETRIC]` value-match with no length dependence (it imports $H_0$ via $\rho_{\text{crit}}$), **not** a resolution of the cosmological-constant problem: FTD predicts $\Lambda=0$ and the value is a `[BOUNDARY]`.

The emergent dark energy density fraction is:
$$ \Omega_\Lambda = \frac{\rho_\Lambda}{\rho_{\text{crit}}} = 0.683 $$
matching the Planck 2018 observed value $\Omega_\Lambda = 0.685 \pm 0.007$ to within **0.3%**.

### 3.2 Dark Matter: Self-Field Halo Overlap `[OPEN]` / mapped-negative (FTD-0300)
> **FALSIFIED scaling (FTD-0300).** The `$r^{-0.69}$` tail below is an `L=64` transient; it **converges to `−1.25` at `L≥128`** and the lossless self-field **box-fills the periodic lattice** (`$r_{\text{eff}}\approx L/2$`, not a localized galactic halo), so SPARC is **not founded** (verdict `INDETERMINATE`). The text below is retained for provenance; the qualitative "dark matter = far-field flux halo" identification is `[SELECTION]`, but the halo *dynamics/profile* are `[OPEN]`/mapped-negative — **not** a derivation. Only the `17/27` dark-**state** count is structural.

The red-team and speculative cosmology papers frequently import hypothetical Cold Dark Matter (CDM) WIMP particles to match galactic rotation curves. FTD's qualitative reading (scoped by the boundary above):
* **The Self-Field Tail:** All lattice sites at $r > 0$ from a manifested particle are void ($s = 0$), meaning they carry no electric or color charge. However, their flux density $\rho = |J|$ is non-zero and gravitationally active.
* **Superposition & Halo Emergence (`$J(r) \propto r^{-0.69}$` FALSIFIED, FTD-0300):** In a galactic cluster of $N$ manifested particles, the overlapping self-field envelopes were posited to form a composite density halo:
  $$ \rho_{\text{halo}}(r) = \sum_{i=1}^N |J_i(\mathbf{r} - \mathbf{r}_i)| $$
* **Flat Rotation Curves (not founded):** the claim that this profile "naturally reproduces" flat rotation curves is `[OPEN]`/mapped-negative — the measured exponent does not converge and the halo box-fills the lattice (FTD-0300).

---

## 4. Epistemic Assessment & Claims Ledger

We maintain strict epistemic standards, categorizing all aspects of cosmic emergence:

| Claim ID | Description | Epistemic Tag | Supporting Evidence / Equation |
| :--- | :--- | :--- | :--- |
| **CE-1** | Spontaneous symmetry breaking at $3 m_e$ genesis threshold. | **[THEOREM]** | $P(\text{manifest}) = 1 - e^{-(|J| - 3m_e)/m_e}$ |
| **CE-2** | Dressed particle envelope with $r_{\text{eff}} \approx 15$ voxels. | **[THEOREM]** | Measured RMS of self-field in `campaign_dark_sector` |
| **CE-3** | Multiplicative selective damping restricts dissipation to 1-hop. | **[THEOREM]** | Multiplicative Rayleigh term in equations of motion |
| **CE-4** | Continuous lossless flux leak from selective damping boundary. | **[THEOREM]** | Net positive energy injection measured in `DS-3` |
| **CE-5** | Comoving spatial metric stretch $a(\tau)$ from energy conservation. | **[CONJECTURE]** | $d(a^3)/d\tau \propto a^3 \rho_\Lambda$ metric ansatz |
| **CE-6** | Emergence of Friedmann cosmological equations. | **[CONJECTURE]** | Linearized Einstein iterative bootstrap in comoving metric |
| **CE-7** | Cosmological constant density $\rho_\Lambda = m_e^4 \alpha^{16} G^{*2}$. | **[SELECTION]** | 16 physical degrees of freedom mode suppression |
| **CE-8** | Dark energy density fraction $\Omega_\Lambda = 0.683$. | **[SELECTION]** | Ratio to critical density $\rho_{\text{crit}}$ matching CMB to 0.3% |
| **CE-9** | Dark matter halos emerge from self-field envelope superposition. | **[SELECTION]** | Non-luminous, gravitating, stable void flux density field |
| **CE-10** | Flat rotation curves from overlapping power-law envelopes. | **[CONJECTURE]** | Flat radial acceleration in multi-particle cluster simulation |

---

## 5. Conclusion

This document supplies ΛCDM apparatus plus FTD numerology, with one genuine micro-scale `[THEOREM]` (the genesis/leak cascade, §1) and a macro-cosmology that is **not** substrate-derived. Per FTD-0331 + `SPEC_COSMOLOGY_FRAMEWORK_BOUNDARY.md`, the load-bearing claims are honestly tiered: Friedmann emergence (CE-5/CE-6) is a posited metric-stretch ansatz `[CONJECTURE]`; the dark-energy `ρ_Λ`/`Ω_Λ` (CE-7/CE-8) is a `[PARAMETRIC]` value-match (FTD predicts `Λ=0`; value a `[BOUNDARY]`); the `r^{-0.69}` halo (CE-9/CE-10) is **FALSIFIED** (FTD-0300). Cosmology remains FTD's most-imported, least-substrate-derived sector — not "fully integrated, non-circular, rigorous."
