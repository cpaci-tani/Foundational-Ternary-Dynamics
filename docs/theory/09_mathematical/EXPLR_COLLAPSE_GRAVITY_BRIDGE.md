# Collapse-Gravity Bridge: The Same Algebraic Type Transition on Different Axes

## How Hawking Temperature Connects Wave Function Collapse to Spacetime Curvature via the Softplus β Parameter

**Date:** February 19, 2026
**Framework:** Foundational Ternary Dynamics v5.26
**Status:** Formal exploration with epistemic classification
**Authors:** cpaci & Claude (Opus 4.6)

---

## Abstract

We establish a quantitative bridge between wave function collapse and gravitational curvature within the FTD algebraic type framework. The key equation is:

$$\beta_H = 8\pi M = 2 N_{\text{base}}^2 \cdot \text{PF} \cdot M$$

The Hawking inverse temperature $\beta_H$ maps directly to the Softplus parameter $\beta$ in the RT dictionary ([EXPLR_RELU_TYPE_TRANSITION.md](EXPLR_RELU_TYPE_TRANSITION.md)), connecting black hole thermodynamics to the von Neumann factor type classification.

| | **Collapse** (measurement) | **Gravity** (curvature) |
|---|---|---|
| What varies | $\beta$ in **time** (finite $\to \infty$ at measurement) | $\beta$ in **space** (via $f(r) = 1 - r_s/r$) |
| Transition | Type III $\to$ Type I at one spacetime point | Type III $\to$ Type I across radial profile |
| $G^*$ enters as | $k_c = 1/(4G^*)$ on phase diagram | $G^*$ exchange rate in computational budget |
| KMS strip | Collapses temporally at threshold $K_B$ | Narrows spatially toward horizon |
| Irreversibility | Collapse is irreversible ($\mathcal{K} \to \mathcal{K}_+$) | Horizon crossing is irreversible ($f \to 0$) |

The central claim [CONJECTURE]: collapse and gravity are the **same** $G^*$-mediated algebraic type transition — collapse is the **temporal** crystallization of a single mode at one spacetime point, while gravity is the **spatial** crystallization of all modes across the radial profile as seen by an asymptotic observer.

A crucial subtlety: the factor type is **observer-dependent**. The Tolman relation gives $\beta_{\text{local}}(r) = \beta_H \sqrt{f(r)}$, which means the horizon is Type III$_1$ from the local observer's perspective (matching Buchholz-Wichmann 1986) but approaches Type I from the asymptotic observer's perspective (large $\beta_H$ for large black holes). The factor type is not a property of spacetime alone — it is a property of the observer-spacetime coupling (the sLoop).

**Epistemic discipline:** We distinguish rigorously between:
- **[CLASSICAL]**: Established physics (Hawking 1975, Tolman 1930, Buchholz-Wichmann 1986, Penrose-Diósi)
- **[THEOREM]**: Provable from stated axioms + classical mathematics
- **[SELECTION]**: Argued mapping between established results and FTD structures
- **[CONJECTURE]**: Novel claims requiring validation
- **[OPEN]**: Identified research directions

**Depends on:**
- [EXPLR_RELU_TYPE_TRANSITION.md](EXPLR_RELU_TYPE_TRANSITION.md) — Type III $\to$ Type I via Softplus $\beta$, RT dictionary
- [DERIV_LATTICE_SCHWARZSCHILD.md](../archive/ARCH_DERIV_LATTICE_SCHWARZSCHILD.md) — $f(r) = 1 - r_s/r$, computational budget
- [DERIV_QFT_GRT_BRIDGE.md](../03_derivations/DERIV_QFT_GRT_BRIDGE.md) — Same flux $J$ for QFT and GRT
- [DERIV_GSTAR_PF_BRIDGE.md](../04_coupling/DERIV_GSTAR_PF_BRIDGE.md) — $G^* = \varpi/\sqrt{\text{PF}}$, PF cancellation
- [FOUND_THE_EXISTENCE_FILTER.md](../06_consciousness/FOUND_THE_EXISTENCE_FILTER.md) — Projection hierarchy $E(x) \to |x|^2 \to \Phi$

---

## Part I: The Two Crystallizations

### 1.1 Collapse as Temporal Crystallization

From [EXPLR_RELU_TYPE_TRANSITION.md](EXPLR_RELU_TYPE_TRANSITION.md), measurement is the $\beta \to \infty$ limit of the Softplus manifestation operator:

$$\mathcal{M}_\beta(z) = \frac{1}{\beta} \ln(1 + e^{\beta z}) \;\xrightarrow{\beta \to \infty}\; \max(0, z) = \text{ReLU}(z)$$

At finite $\beta$ (pre-measurement), the system has Type III character:
- **KMS condition holds**: modular automorphism $\sigma_t$ exists
- **Occupation function continuous**: $\mathcal{M}'_\beta(z) = 1/(1 + e^{-\beta z}) \in (0,1)$
- **No minimal projections**: outcomes are not yet definite
- **Analyticity strip width**: $\pi/\beta$ (finite, non-zero)

At $\beta \to \infty$ (measurement), Type I character crystallizes:
- **KMS destroyed**: analyticity strip collapses to zero
- **Occupation function discrete**: $\mathcal{M}'_\infty(z) = \Theta(z) \in \{0,1\}$
- **Minimal projection emerges**: $\mathcal{M}''_\infty(z) = \delta(z)$
- **MASA selected**: definite measurement outcome via Heaviside partition

This transition occurs at a **single spacetime point** at the **moment of measurement**. The $\beta$ parameter increases in time from finite to infinity. The event is local and temporal.

### 1.2 Gravity as Spatial Crystallization

From [DERIV_LATTICE_SCHWARZSCHILD.md](../archive/ARCH_DERIV_LATTICE_SCHWARZSCHILD.md), the Schwarzschild metric arises from computational budget saturation:

$$f(r) = 1 - \frac{r_s}{r} = 1 - \frac{\rho_{\text{info}}}{\rho_{\max}}}$$

The availability factor $f$ measures what fraction of computational capacity remains after gravitational data processing. At the horizon ($r = r_s$), $f = 0$: the lattice is fully saturated, proper time stops, no further computation can occur.

The proper time formula ([DERIV_LATTICE_SCHWARZSCHILD.md](../archive/ARCH_DERIV_LATTICE_SCHWARZSCHILD.md) Theorem 5.1):

$$\frac{d\tau}{dT_U} = \sqrt{\frac{f^2 - v^2}{f}}$$

This is a **spatial** profile: $f$ varies across space while at each point the voxel experiences the reduced computational capacity. Far from the mass ($r \to \infty$), $f \to 1$ and the lattice operates at full capacity. At the horizon ($r \to r_s$), $f \to 0$ and proper time freezes.

### 1.3 The Thesis: Same Transition, Different Axes [CONJECTURE CG-C1]

Both phenomena share the same algebraic structure:

| Feature | Collapse | Gravity |
|---------|----------|---------|
| Parameter | $\beta(t)$ | $\beta(r)$ |
| Axis | Time (local) | Space (global) |
| Transition | $\beta: \text{finite} \to \infty$ | $f: 1 \to 0$ |
| KMS strip | Collapses at measurement | Narrows toward horizon |
| Outcome | One mode crystallizes | All modes behind horizon crystallize |
| Observer | Local (at measurement point) | Asymptotic (at $r \to \infty$) |
| Irreversibility | Collapse cannot be undone | Horizon crossing cannot be reversed |

The bridge between them is the Hawking temperature: it assigns a specific $\beta$ to the gravitational field, connecting the two crystallizations quantitatively.

---

## Part II: The Hawking-KMS Bridge

### 2.1 Hawking Temperature in FTD Notation [CLASSICAL + THEOREM]

The Hawking temperature for a Schwarzschild black hole of mass $M$ (in Planck units, $G = c = \hbar = k_B = 1$):

$$T_H = \frac{1}{8\pi M}$$

This is [CLASSICAL] (Hawking 1975).

From [DERIV_GSTAR_PF_BRIDGE.md](../04_coupling/DERIV_GSTAR_PF_BRIDGE.md) §3.2, express $8\pi$ through FTD integers:

$$8\pi = 2 \cdot N_{\text{base}}^2 \cdot \text{PF} = 2 \cdot 16 \cdot \frac{\pi}{4} = 8\pi \quad \checkmark$$

Therefore the inverse temperature at the horizon is:

$$\boxed{\beta_H = \frac{1}{T_H} = 8\pi M = 2 N_{\text{base}}^2 \cdot \text{PF} \cdot M}$$

### 2.2 Mapping to the RT Dictionary [SELECTION CG-S1]

The RT dictionary ([EXPLR_RELU_TYPE_TRANSITION.md](EXPLR_RELU_TYPE_TRANSITION.md) §2.4, Key Equation 4) maps the Softplus $\beta$ to the Connes classification parameter:

$$\lambda(\beta) = e^{-\beta}, \qquad T = \frac{2\pi}{\beta} = \frac{-2\pi}{\ln \lambda}$$

We apply this dictionary to the Hawking $\beta_H$:

| Quantity | Formula | Expression |
|----------|---------|------------|
| **Connes parameter** | $\lambda_H = e^{-\beta_H}$ | $e^{-8\pi M}$ |
| **KMS strip width** | $\pi/\beta_H$ | $1/(8M)$ |
| **Modular period** | $T = 2\pi/\beta_H$ | $1/(4M)$ |

### 2.3 Algebraic Character vs Black Hole Mass [THEOREM + CONJECTURE CG-C2]

Evaluating $\lambda_H = e^{-8\pi M}$ across the mass spectrum:

| BH Mass $M$ (Planck units) | $\beta_H = 8\pi M$ | $\lambda_H = e^{-\beta_H}$ | KMS strip $\pi/\beta_H$ | Algebraic character |
|------|------|------|------|------|
| $10^{38}$ (stellar, $\sim 10 M_\odot$) | $\sim 10^{39}$ | $\approx 0$ | $\sim 10^{-39}$ | **Nearly Type I** (classical) |
| $10$ | $251$ | $\sim 10^{-109}$ | $0.013$ | Type III$_0$ (very near Type I) |
| $1$ (Planck mass) | $25.13$ | $\sim 10^{-11}$ | $0.125$ | Type III$_0$ |
| $0.1$ | $2.51$ | $0.081$ | $1.25$ | Type III$_{0.08}$ |
| $0.01$ | $0.251$ | $0.778$ | $12.5$ | Type III$_{0.78}$ (near III$_1$) |
| $M \to 0$ (evaporating) | $\to 0$ | $\to 1$ | $\to \infty$ | **Type III$_1$** (fully quantum) |

**The algebraic gradient** [THEOREM]:
- Large BHs ($M \to \infty$): $\beta_H \to \infty$, $\lambda_H \to 0$ — approaching **Type I** (classical, information locked behind horizon)
- Small BHs ($M \to 0$): $\beta_H \to 0$, $\lambda_H \to 1$ — approaching **Type III$_1$** (fully quantum, information accessible)

This is a mathematical consequence of the RT dictionary applied to the Hawking temperature. No physical assumption beyond Hawking's result is required.

**The physical interpretation** [CONJECTURE CG-C2]: The algebraic type of a black hole as a thermodynamic system evolves during evaporation from near-Type I (classical) to Type III$_1$ (fully quantum). Information recovery becomes possible as sufficient Type III character is restored for modular flow to carry information through the widening KMS strip.

### 2.4 PF in the Hawking-KMS Bridge [THEOREM CG-T1]

**Theorem CG-T1** (PF Separation in the Hawking-KMS Bridge): *PF appears in $\beta_H$ but cancels in the entropy-temperature product:*

$$\beta_H = 2 N_{\text{base}}^2 \cdot \text{PF} \cdot M \qquad \text{(PF survives)}$$

$$S_{BH} \times T_H = \frac{M}{2} \qquad \text{(PF cancels, from DERIV\_GSTAR\_PF\_BRIDGE.md Theorem 3.1)}$$

**Proof:** $S_{BH} = N_{\text{base}}^2 \cdot \text{PF} \cdot M^2$ and $T_H = 1/(2 N_{\text{base}}^2 \cdot \text{PF} \cdot M)$. The product:

$$S_{BH} \times T_H = \frac{N_{\text{base}}^2 \cdot \text{PF} \cdot M^2}{2 N_{\text{base}}^2 \cdot \text{PF} \cdot M} = \frac{M}{2} \quad \blacksquare$$

**Interpretation** [SELECTION]: The Connes parameter $\lambda_H = e^{-\beta_H}$ depends on PF (geometric packing), so the algebraic type classification contains geometric information. But the thermodynamic observables ($S \times T$) are PF-free — the physics is topological even though the algebraic classification is geometric. This is consistent with the PF cancellation pattern established in [DERIV_GSTAR_PF_BRIDGE.md](../04_coupling/DERIV_GSTAR_PF_BRIDGE.md): PF enters intermediate expressions but cancels in physical observables.

---

## Part III: Local vs Global Factor Type

### 3.1 The Tolman Relation [CLASSICAL]

A static observer at radial coordinate $r$ outside a Schwarzschild black hole measures a **local temperature** related to the Hawking temperature by the Tolman relation (Tolman 1930, applied to Hawking radiation):

$$T_{\text{local}}(r) = \frac{T_H}{\sqrt{f(r)}} = \frac{1}{8\pi M \sqrt{1 - r_s/r}}$$

The corresponding local inverse temperature:

$$\beta_{\text{local}}(r) = \beta_H \sqrt{f(r)} = 8\pi M \sqrt{1 - r_s/r}$$

This is established physics — no FTD-specific assumption is required.

### 3.2 Position-Dependent Algebraic Character [SELECTION + CONJECTURE CG-C3]

Applying the RT dictionary to the local $\beta$:

$$\lambda_{\text{local}}(r) = e^{-\beta_{\text{local}}(r)} = e^{-\beta_H \sqrt{f(r)}}$$

The behavior at limiting positions:

| Position | $f(r)$ | $\beta_{\text{local}}$ | $T_{\text{local}}$ | $\lambda_{\text{local}}$ | Factor type |
|----------|--------|----------------------|-------------------|------------------------|-------------|
| At horizon ($r \to r_s$) | $\to 0$ | $\to 0$ | $\to \infty$ | $\to 1$ | **Type III$_1$** |
| At $r = 2r_s$ | $1/2$ | $\beta_H/\sqrt{2}$ | $T_H\sqrt{2}$ | $e^{-\beta_H/\sqrt{2}}$ | Type III$_\lambda$ |
| At $r = 10r_s$ | $9/10$ | $\beta_H\sqrt{0.9}$ | $T_H/\sqrt{0.9}$ | $e^{-0.949\beta_H}$ | Type III$_\lambda$ |
| Far away ($r \to \infty$) | $\to 1$ | $\to \beta_H$ | $\to T_H$ | $\to e^{-\beta_H}$ | **Type III$_{\lambda_H}$** |

**Key result:** The horizon is **maximally quantum** from the local observer's perspective — $\beta_{\text{local}} \to 0$ gives $\lambda \to 1$, which is Type III$_1$ (fully ergodic, maximal modular flow).

This matches the classical AQFT result: **local QFT algebras are Type III$_1$** (Buchholz-Wichmann 1986). The FTD analysis reproduces this established result via the RT dictionary.

### 3.3 Observer-Dependent Factor Type [CONJECTURE CG-C3]

The factor type depends on **who is observing**:

| Observer perspective | At horizon ($r \to r_s$) | Far from BH ($r \to \infty$) |
|---------------------|----------------------|---------------------|
| **Local** (static at $r$) | $\beta_{\text{local}} \to 0$, $T \to \infty$ → **Type III$_1$** | $\beta_{\text{local}} = \beta_H$ → **Type III$_{\lambda_H}$** |
| **Asymptotic** (at $\infty$) | Sees $\beta_\infty = \beta_H = 8\pi M$ → **large $\beta$** (near Type I) | $\beta_\infty \to \infty$ (vacuum) → **Type I** |

The duality:
- **Locally**: the horizon is maximally quantum (Type III$_1$). The local observer experiences infinite temperature, maximal modular flow, full ergodicity.
- **Globally**: the black hole as a whole approaches Type I from the asymptotic observer's perspective. It has definite entropy, definite temperature, and behaves as a classical thermodynamic object.

**These are not contradictory** — they are two valid descriptions of the same system from different observer perspectives. The factor type is a property of the **observer-system coupling** (the sLoop), not of spacetime alone.

**Connection to FTD:** This is precisely the sLoop principle from [CLAUDE.md](../../CLAUDE.md) §12.4: the observer is ontologically continuous with the observed. A local observer embedded in the near-horizon flux field experiences different algebraic structure than an asymptotic observer decoupled from the local thermal bath.

---

## Part IV: The Unification Thesis

### 4.1 Central Claim [CONJECTURE CG-C4]

**Collapse and gravity are the same algebraic type transition:**

**Collapse** = local, temporal Type III $\to$ Type I:
- One observer at one spacetime point
- $\beta(t)$ increases from finite to $\infty$ at measurement time
- The Softplus sharpens to ReLU
- KMS strip collapses
- One specific mode crystallizes into a definite outcome

**Gravity** = global, spatial Type III $\to$ Type I:
- The asymptotic observer views the entire black hole
- $\beta_H = 8\pi M$ characterizes the whole system
- For large $M$, $\beta_H \gg 1$: system is near-Type I
- All modes behind the horizon are crystallized into the thermal state
- The horizon boundary marks the spatial location of the type transition

### 4.2 G* as the Common Bridge [SELECTION CG-S2]

$G^*$ enters both transitions:

| Role | In collapse | In gravity | Reference |
|------|------------|------------|-----------|
| Phase diagram | $k_c = 1/(4G^*)$ — exceptional point between Type III and Type I | Same $k_c$ controls transition threshold | EXPLR_RELU_TYPE_TRANSITION.md §4.1 |
| Exchange rate | Converts continuous occupation $\to$ discrete outcome | Converts continuous geometry $\to$ discrete information | DERIV_GSTAR_PF_BRIDGE.md §1.4 |
| PF factorization | $G^* = \varpi/\sqrt{\text{PF}}$ | Same decomposition | DERIV_GSTAR_PF_BRIDGE.md §2.1 |

The Hawking $\beta_H$ can be expressed through $G^*$. Since $8\pi = 2 N_{\text{base}}^2 \cdot \text{PF}$ and $\text{PF} = \varpi^2/G^{*2}$ (from $G^* = \varpi/\sqrt{\text{PF}}$):

$$\beta_H = 2 N_{\text{base}}^2 \cdot \frac{\varpi^2}{G^{*2}} \cdot M$$

So $G^*$ appears in the gravitational $\beta$ through the PF factor, just as $k_c = 1/(4G^*)$ controls the phase boundary for collapse. Both transitions are mediated by the same algebraic constant.

### 4.3 Connection to the Existence Filter Hierarchy [CONJECTURE CG-C5]

From [FOUND_THE_EXISTENCE_FILTER.md](../06_consciousness/FOUND_THE_EXISTENCE_FILTER.md), the projection hierarchy:

| Level | Projection | Formula | Character |
|-------|-----------|---------|-----------|
| $-1$ | Existence Filter | $E(x) = \text{Re}(x)$ | Linear, additive |
| $0$ | Magnitude | $|x| = \sqrt{x \bar{x}}$ | Metric |
| $0.5$ | Born Rule | $P = |x|^2$ | Quadratic |
| Interface | Collapse | $\Phi: \mathcal{R} \to M_n(\mathbb{C})$ | CPTP |

We propose the following correspondence to the algebraic type descent:

| Projection level | Algebraic type | Physics regime |
|-----------------|---------------|----------------|
| $E(x) = \text{Re}(x)$ (smooth, linear) | **Type III** | Quantum superposition; far from mass; pre-measurement |
| $|x|^2 = P$ (Born rule, quadratic) | **Type II$_1$** | Measurement interface at $k_c = 1/(4G^*)$ |
| $\Phi$ (collapse, CPTP, discrete) | **Type I** | Classical outcome; horizon; post-measurement |

This maps the entire projection hierarchy onto the algebraic descent chain. Each successive projection destroys more Type III character:
- $E(x)$: preserves full analyticity → Type III modular flow exists
- $|x|^2$: destroys phase information → continuous dimension function $[0,1]$ (Type II$_1$ trace)
- $\Phi$: destroys dimension continuity → discrete projections $\{0,1\}$ (Type I)

This correspondence is [CONJECTURE] — establishing it rigorously would require constructing the actual von Neumann algebras associated with each projection level (cf. RT-O1, RT-O6 in EXPLR_RELU_TYPE_TRANSITION.md).

---

## Part V: Gravitational Decoherence

### 5.1 The Penrose-Diósi Model [CLASSICAL]

Penrose (1996) and Diósi (1989) independently proposed that gravity causes wave function collapse. For a quantum superposition of two mass configurations separated by distance $d$ with gravitational self-energy difference $\Delta E_G$:

$$\tau_{\text{collapse}} \sim \frac{\hbar}{\Delta E_G}$$

For two masses $m$ in superposition separated by $d$:

$$\Delta E_G \sim \frac{Gm^2}{d}$$

$$\tau_{\text{Penrose}} \sim \frac{\hbar d}{Gm^2}$$

This predicts faster collapse for more massive superpositions — currently being tested in tabletop experiments.

### 5.2 FTD Gravitational Decoherence [CONJECTURE CG-C6]

In FTD, a massive superposition creates two overlapping flux configurations with different stress-energy tensors $T_{\mu\nu}$. Each branch produces a different availability factor:

$$f_1(r) = 1 - \frac{r_{s,1}}{r}, \qquad f_2(r) = 1 - \frac{r_{s,2}}{r}$$

where $r_{s,1}$ and $r_{s,2}$ are the Schwarzschild radii corresponding to the two mass configurations.

The difference creates two competing $\beta$-profiles:

$$\Delta\beta(r) = \beta_{H,1}\sqrt{f_1(r)} - \beta_{H,2}\sqrt{f_2(r)}$$

The "algebraic stress" between these profiles triggers manifestation (collapse) when the integrated difference exceeds a threshold related to $K_B$:

$$\int |\Delta\beta(r)| \, d^3r \;\gtrsim\; \frac{1}{4G^*}$$

where $1/(4G^*)$ is the phase diagram threshold from the RT dictionary (EXPLR_RELU_TYPE_TRANSITION.md §4.1).

### 5.3 Comparison to Penrose [CONJECTURE CG-C6]

For small superpositions ($\Delta m \ll m$, $d \ll r$), the FTD gravitational decoherence timescale reduces to:

$$\tau_{\text{FTD}} \sim \frac{\hbar d}{Gm^2} \times g(G^*, \alpha)$$

where $g(G^*, \alpha)$ is a correction factor from the FTD algebraic structure.

| Feature | Penrose-Diósi | FTD |
|---------|--------------|-----|
| Scaling with mass | $\tau \propto 1/m^2$ | $\tau \propto 1/m^2$ (same) |
| Scaling with distance | $\tau \propto d$ | $\tau \propto d$ (same) |
| Correction factor | None (pure gravity) | $g(G^*, \alpha)$ from phase diagram |
| Mechanism | "Gravitational self-energy" | Competing $\beta$-profiles trigger type transition |
| Threshold | $\Delta E_G \sim \hbar/\tau$ | $\int|\Delta\beta| \gtrsim 1/(4G^*)$ |

The FTD prediction matches Penrose-Diósi to leading order but includes a correction from the algebraic type structure. The correction factor $g(G^*, \alpha)$ is the genuinely new prediction — it could distinguish FTD from the Penrose-Diósi model once tabletop experiments reach sufficient precision (estimated 2030s).

**Epistemic status:** The scaling match is robust (both derive from $\Delta E_G \sim Gm^2/d$). The correction factor $g(G^*, \alpha)$ is conjectured from the phase diagram structure but has not been computed from first principles. This requires resolving RT-O1 (constructing actual VN algebras from FTD field operators).

---

## Part VI: Information Paradox via Algebraic Types

### 6.1 The Paradox [CLASSICAL]

Is information lost when a black hole evaporates? Hawking's original calculation suggests information is destroyed, violating unitarity. The Page curve (Page 1993) proposes that information is gradually recovered during evaporation, with entropy of Hawking radiation peaking at the **Page time** ($t_{\text{Page}} \sim M^3$ in Planck units) and then decreasing.

### 6.2 Algebraic Type Evolution During Evaporation [CONJECTURE CG-C7]

As a black hole evaporates ($M$ decreases over time), its algebraic character evolves:

| Evaporation stage | $M$ | $\beta_H = 8\pi M$ | $\lambda_H = e^{-\beta_H}$ | KMS strip width | Factor type |
|---|---|---|---|---|---|
| Formation | $M_0$ (large) | $\gg 1$ | $\approx 0$ | $\ll 1$ | Near Type I |
| Mid-evaporation | $M_0/2$ | Large/2 | Small but growing | Growing | Type III$_\lambda$ (increasing $\lambda$) |
| Near Planck | $\sim M_P$ | $\sim 25$ | $\sim 10^{-11}$ | $\sim 0.1$ | Type III$_0$ |
| Final burst | $M \to 0$ | $\to 0$ | $\to 1$ | $\to \infty$ | **Type III$_1$** |

**The algebraic resolution:** Information is not "lost" or "recovered" — it changes its algebraic accessibility:

1. **Large BH (near Type I):** The KMS strip is exponentially narrow (width $\sim e^{-8\pi M_0}$). Modular flow $\sigma_t$ exists but information transport through the strip is exponentially suppressed. Information is effectively locked — not destroyed, but algebraically inaccessible.

2. **Evaporating BH (Type III$_\lambda$, $\lambda$ increasing):** As $M$ decreases, $\beta_H$ decreases, $\lambda_H$ increases, the KMS strip widens. Modular flow carries progressively more information through the strip. The Hawking radiation begins to carry correlations.

3. **Final burst (Type III$_1$):** Full modular flow is restored. All information is algebraically accessible. The final state is fully quantum.

### 6.3 The Page Curve from β Evolution [CONJECTURE CG-C8]

The Page time can be estimated as the moment when the algebraic type has shifted enough for significant information flow through the KMS strip.

The BH mass evolves as $M(t) \sim (M_0^3 - t)^{1/3}$ (from Hawking evaporation rate $dM/dt \propto -1/M^2$).

The Page time corresponds to $M(t_{\text{Page}}) \sim M_0/\sqrt{2}$ (half the initial entropy). From $M^3 - t \sim M_0^3/2\sqrt{2}$:

$$t_{\text{Page}} \sim M_0^3 \left(1 - \frac{1}{2\sqrt{2}}\right) \propto M_0^3$$

This matches the Hayden-Preskill (2007) estimate. In the algebraic picture, at the Page time:

$$\beta_H(t_{\text{Page}}) = 8\pi \cdot \frac{M_0}{\sqrt{2}} = \frac{\beta_{H,0}}{\sqrt{2}}$$

$$\lambda_H(t_{\text{Page}}) = e^{-\beta_{H,0}/\sqrt{2}}$$

For stellar-mass BHs, this is still exponentially small — information flow remains negligible until the final Planck-scale stages. The Page curve shape (smooth rise then fall of radiation entropy) corresponds to the smooth evolution of $\lambda_H$ from $\approx 0$ to $1$.

### 6.4 The Firewall Paradox [CONJECTURE CG-C9]

The firewall paradox (AMPS 2012) arises from the apparent incompatibility of:
1. **Smooth horizon** (equivalence principle — no drama for infalling observer)
2. **Unitarity** (information must escape)
3. **Effective field theory** (valid outside horizon)

FTD dissolves this by recognizing that these conditions apply to **different observers**:

- **Infalling observer** (local): Sees $\beta_{\text{local}} \to 0$ at the horizon → Type III$_1$ → smooth, fully quantum, no firewall. The local algebra is the hyperfinite Type III$_1$ factor — the equivalence principle is satisfied.

- **Asymptotic observer** (global): Sees $\beta_H = 8\pi M$ → near-Type I → definite entropy, thermal state. Unitarity is maintained because the global description has a well-defined trace and the evolution preserves the overall algebraic structure.

These are not contradictory because the factor type is observer-dependent (§3.3). The "firewall" would only arise if one demanded that both observers experience the same algebraic type simultaneously — but the sLoop principle says they cannot, because each observer couples to the system differently.

---

## Part VII: Predictions and Tests

### 7.1 Prediction Summary

| ID | Prediction | FTD Value | Standard Physics | Testable? |
|----|-----------|-----------|-----------------|-----------|
| CG-P1 | Gravitational decoherence rate | $\tau \propto \hbar d/(Gm^2) \times g(G^*, \alpha)$ | $\tau \propto \hbar/(Gm^2/d)$ (Penrose) | Yes: tabletop experiments ~2030s |
| CG-P2 | BH information recovery | Page curve from $\beta$ evolution | Page curve from unitarity | Not directly |
| CG-P3 | Horizon algebra is Type III$_1$ locally | Yes (from Tolman + RT dictionary) | Yes (Buchholz-Wichmann 1986) | Confirmed by AQFT |
| CG-P4 | Factor type is observer-dependent | Yes (sLoop) | Not standard (but compatible with AQFT) | Novel — requires formal construction |
| CG-P5 | $G^*$ correction to Penrose decoherence | $g(G^*, \alpha) \neq 1$ | $g = 1$ (no correction) | Yes: precision gravity experiments |

### 7.2 Falsification Criteria

| Claim | Falsifying observation |
|-------|----------------------|
| CG-C1 (same transition) | Demonstration that collapse and curvature have fundamentally different algebraic structures |
| CG-C3 (observer-dependent type) | Proof that factor type must be observer-independent in AQFT |
| CG-C6 (gravitational decoherence) | Measured decoherence rate inconsistent with $\tau \propto \hbar d/(Gm^2)$ scaling |
| CG-C7 (β evolution) | Proof that BH evaporation does not change the algebraic character of the radiation state |
| CG-C9 (firewall dissolution) | Proof that sLoop-type observer dependence violates unitarity or locality |

### 7.3 Open Questions

| ID | Question | Priority |
|----|----------|----------|
| CG-O1 | Can the actual VN algebras of FTD field operators be constructed and classified? (Inherited from RT-O1) | **High** |
| CG-O2 | What is the exact form of the correction factor $g(G^*, \alpha)$ in gravitational decoherence? | **High** |
| CG-O3 | Does the algebraic type evolution during BH evaporation reproduce the Page curve quantitatively (not just qualitatively)? | Medium |
| CG-O4 | Can the observer-dependent factor type be made rigorous within AQFT (not just via the RT dictionary mapping)? | **High** |
| CG-O5 | Does the spatial $\beta(r)$ profile connect to the FTD computational budget interpretation of $f(r)$? | Medium |
| CG-O6 | What happens at the Planck scale where both crystallizations (collapse and gravity) operate simultaneously? | Low (requires quantum gravity) |

---

## Part VIII: Claims Table

### 8.1 Complete Claims Inventory

| ID | Claim | Tag | Reference |
|----|-------|-----|-----------|
| **CG-T1** | $S_{BH} \times T_H = M/2$ with PF cancellation | **[THEOREM]** | DERIV_GSTAR_PF_BRIDGE.md Thm 3.1 |
| **CG-T2** | $\beta_H = 8\pi M = 2 N_{\text{base}}^2 \cdot \text{PF} \cdot M$ | **[THEOREM]** | Hawking 1975 + PF decomposition |
| **CG-T3** | Local algebra at horizon is Type III$_1$ | **[THEOREM]** | Buchholz-Wichmann 1986 (classical AQFT) |
| **CG-S1** | $\beta_H$ maps to RT dictionary: $\lambda_H = e^{-8\pi M}$ | **[SELECTION]** | RT dictionary applied to Hawking temperature |
| **CG-S2** | $G^*$ mediates both collapse and gravity transitions | **[SELECTION]** | Phase diagram + PF factorization |
| **CG-C1** | Collapse and gravity are the same type transition on different axes | **[CONJECTURE]** | §1.3 — the central thesis |
| **CG-C2** | Large BHs $\to$ Type I; evaporating BHs $\to$ Type III$_1$ | **[CONJECTURE]** | §2.3 — mass spectrum table |
| **CG-C3** | Factor type is observer-dependent via sLoop | **[CONJECTURE]** | §3.3 — local vs global |
| **CG-C4** | Collapse = temporal crystallization, Gravity = spatial crystallization | **[CONJECTURE]** | §4.1 — unification thesis |
| **CG-C5** | Existence Filter hierarchy maps onto type descent | **[CONJECTURE]** | §4.3 — projection correspondence |
| **CG-C6** | FTD gravitational decoherence matches Penrose to leading order | **[CONJECTURE]** | §5.2–5.3 |
| **CG-C7** | BH algebraic type evolves during evaporation | **[CONJECTURE]** | §6.2 |
| **CG-C8** | Page curve from $\beta_H$ evolution | **[CONJECTURE]** | §6.3 |
| **CG-C9** | Firewall paradox dissolved by observer-dependent factor type | **[CONJECTURE]** | §6.4 |

### 8.2 Summary Statistics

| Category | Count | IDs |
|----------|-------|-----|
| Theorems | 3 | CG-T1, CG-T2, CG-T3 |
| Selections | 2 | CG-S1, CG-S2 |
| Conjectures | 9 | CG-C1 through CG-C9 |
| Open questions | 6 | CG-O1 through CG-O6 |
| Predictions | 5 | CG-P1 through CG-P5 |
| **Total claims** | **25** | |

### 8.3 What This Document Does NOT Claim

- Does **NOT** claim to have solved quantum gravity (gravity is still emergent, not quantized)
- Does **NOT** claim the factor type assignment is rigorous (requires constructing actual VN algebras — RT-O1, RT-O6)
- Does **NOT** derive the Page curve from first principles (only qualitative reproduction from $\beta$ evolution)
- Does **NOT** claim the Penrose decoherence rate is exact (only leading-order scaling match)
- Does **NOT** resolve the sLoop mechanism for aggregate QM statistics ($S > 2$ remains [OPEN])
- Does **NOT** explain how the temporal $\beta(t)$ of collapse connects mechanistically to the spatial $\beta(r)$ of gravity — it only shows they have the same algebraic structure

---

### Cross-References

| Document | Relevance |
|----------|-----------|
| [EXPLR_RELU_TYPE_TRANSITION.md](EXPLR_RELU_TYPE_TRANSITION.md) | RT dictionary, Softplus $\beta$, type descent chain, phase diagram |
| [DERIV_LATTICE_SCHWARZSCHILD.md](../archive/ARCH_DERIV_LATTICE_SCHWARZSCHILD.md) | $f(r) = 1 - r_s/r$, computational budget, velocity amplification |
| [DERIV_QFT_GRT_BRIDGE.md](../03_derivations/DERIV_QFT_GRT_BRIDGE.md) | Same flux $J$ for QFT and GRT, $T_{\mu\nu}$ via Noether |
| [DERIV_GSTAR_PF_BRIDGE.md](../04_coupling/DERIV_GSTAR_PF_BRIDGE.md) | $G^* = \varpi/\sqrt{\text{PF}}$, BH thermodynamics, PF cancellation |
| [FOUND_THE_EXISTENCE_FILTER.md](../06_consciousness/FOUND_THE_EXISTENCE_FILTER.md) | Projection hierarchy $E \to |\cdot|^2 \to \Phi$ |
| [../06_consciousness/FOUND_DOMAIN_PARTITION_AND_CONTEXT_SELECTION.md](../06_consciousness/FOUND_DOMAIN_PARTITION_AND_CONTEXT_SELECTION.md) | Domain A/B/C partition, vocabulary discipline, and context selection |

---

*Collapse-Gravity Bridge — Foundational Ternary Dynamics v5.26*
*Prepared for critical evaluation*
*February 19, 2026*
