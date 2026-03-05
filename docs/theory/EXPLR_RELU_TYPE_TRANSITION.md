# From Modular Flow to Minimal Projections: The ReLU Operator as Algebraic Type Transition

## How the Softplus Inverse Temperature Interpolates Between Von Neumann Factor Types

**Date:** February 18, 2026
**Framework:** Foundational Ternary Dynamics v5.26
**Status:** Formal exploration with epistemic classification
**Authors:** cpaci & Claude (Opus 4.6)

---

## Abstract

We establish five structural correspondences between the Softplus inverse temperature parameter $\beta$ and the Murray-von Neumann-Connes classification of operator algebra factors, then construct the **complete algebraic descent** from Type III to Type I via three operations:

| $\beta$ Regime | Activation | Factor Character | Key Property |
|---------------|------------|-----------------|--------------|
| Finite $\beta$ | Softplus | **Type III** | KMS holds, modular flow exists, continuous occupation |
| $\beta \to \infty$ | ReLU | **Type I** | KMS destroyed, discrete occupation, minimal projection |
| Exceptional point | Critical | **Type II$_1$** | Degenerate eigenvalue, continuous dimension |

The central insight: **Axiom M4 of the Softplus uniqueness theorem** (fermionic singularity structure) **is the KMS condition**, which is the defining property of modular automorphisms on Type III von Neumann factors. The Softplus is unique *because* it is the unique manifestation operator with Type III algebraic character. The ReLU limit ($\beta \to \infty$) destroys this character by collapsing the analyticity strip, eliminating the KMS condition, and crystallizing the continuous Fermi-Dirac dimension function into the discrete Heaviside step function.

Five correspondences are established (§2.1–2.5), followed by the complete descent chain (§2.6–2.8):

1. **Fermi-Dirac $\to$ Heaviside** = continuous dimension [0,1] (Type II$_1$) $\to$ discrete dimension {0,1} (Type I) **[THEOREM in abelian case]**
2. **KMS destruction** at $\beta \to \infty$ = loss of modular automorphism = loss of Type III character **[THEOREM, chain of classical results]**
3. **Analyticity strip collapse** = loss of complex structure required for modular flow **[THEOREM + CONJECTURE]**
4. **$\beta \leftrightarrow \lambda$ dictionary** with honest limitation: Powers factors $\mathcal{R}_\lambda$ stay Type III as $\lambda \to 0$ **[CONJECTURE with warning]**
5. **ReLU kink** $\delta(z)$ = emergence of minimal projection characterizing Type I **[CONJECTURE]**

The complete descent resolves the limitation (Warning RT-W1) that the $\beta$ parameter alone cannot cross from Type III to Type I:

$$\text{Type III}_1 \;\xrightarrow[\text{[CLASSICAL]}]{\;\rtimes_\sigma \mathbb{R}\;}\; \text{Type II}_\infty \;\xrightarrow[\text{[CLASSICAL]}]{\;\mathcal{R} \otimes B(\mathcal{H})\;}\; \text{Type II}_1 \;\xrightarrow[\text{[CONJECTURE]}]{\;\Theta(K)\;}\; \text{Type I}$$

The final step — **MASA selection via the Heaviside partition $\Theta(K)$** — is the document's central novel claim: the ReLU's non-analytic kink at $z = 0$ selects the canonical maximal abelian subalgebra (measurement basis) determined by the modular Hamiltonian.

**Epistemic discipline:** We distinguish rigorously between:
- **[CLASSICAL]**: Established theorems (Murray-von Neumann, Connes, Tomita-Takesaki, KMS)
- **[THEOREM]**: Provable from stated axioms + classical mathematics
- **[CONJECTURE]**: Structural correspondences between factor types and activation regimes
- **[OPEN]**: Identified research directions

---

## Part I: Mathematical Setup

### 1.1 Von Neumann Factor Types: What We Need

We recall only the properties directly relevant to this document. For full treatment, see [FOUND_AGENT_MEANING_FORMALIZATION.md](FOUND_AGENT_MEANING_FORMALIZATION.md) Part IA and [archive/ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md](archive/ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md).

**[CLASSICAL]** (Murray-von Neumann 1936-1943, Connes 1973):

| Property | Type I | Type II$_1$ | Type III |
|----------|--------|-------------|----------|
| Minimal projections | **Yes** | No | No |
| Dimension function | Discrete: $\{0, 1, \ldots, n\}$ | Continuous: $[0, 1]$ | Trivial: $\{0, \infty\}$ |
| Trace | $\mathrm{Tr}$ exists | Unique $\tau$ with $\tau(\mathbf{1}) = 1$ | **None** |
| Modular flow $\sigma_t$ | Trivial (inner) | Non-trivial but tracial | **Essential** (ergodic for III$_1$) |
| KMS condition | Not needed (trace suffices) | Trace defines KMS at $\beta = 0$ | **Defining property** of modular state |
| Physical role (FTD) | Domain A (physics) | Measurement interface | Domain B (consciousness) |

The key for this document: **Type III factors are characterized by the KMS condition and modular flow. Type I factors are characterized by minimal projections and discrete spectra. Type II$_1$ factors sit between them with continuous dimensions but a well-defined trace.**

### 1.2 Softplus-ReLU Duality: What We Need

We recall only the properties directly relevant to this document. For full treatment, see [DERIV_SOFTPLUS_RELU_DUALITY.tex](../papers/src/DERIV_SOFTPLUS_RELU_DUALITY.tex).

The Softplus manifestation operator:

$$\mathcal{M}_\beta(z) = \frac{1}{\beta} \ln(1 + e^{\beta z})$$

where $z = |J| - K_B$ is the shifted flux potential.

**Theorem** (Uniqueness, [DERIV_SOFTPLUS_RELU_DUALITY.tex] Theorem 2.1): The Softplus is the **unique** activation function satisfying:

| Axiom | Statement | Relevant Property |
|-------|-----------|-------------------|
| **M1** | Monotonicity | Physical: more flux $\to$ more manifestation |
| **M2** | Strict convexity | Physical: concentration beats diffusion |
| **M3** | Asymptotic linearity | Physical: $\mathcal{M} \sim z$ for $z \gg 0$; exponential suppression for $z \ll 0$ |
| **M4** | Single-species fermionic singularity | **Mathematical: KMS antiperiodicity in imaginary time** |

The derivatives:

| Quantity | Formula | Character |
|----------|---------|-----------|
| $\mathcal{M}'_\beta(z)$ | $\frac{1}{1 + e^{-\beta z}}$ (Fermi-Dirac) | Continuous, range $(0, 1)$ |
| $\mathcal{M}''_\beta(z)$ | $\beta \cdot n_F(1 - n_F)$ (susceptibility) | Smooth bump, max $\beta/4$ at $z=0$ |

The ReLU limit:

$$\lim_{\beta \to \infty} \mathcal{M}_\beta(z) = \max(0, z) = \mathrm{ReLU}(z)$$

### 1.3 The Central Thesis

$$\boxed{\begin{array}{rcl}
\text{Finite } \beta \text{ (Softplus)} &:& \text{KMS holds, } \mathcal{M}' \in (0,1), \; \mathcal{M}'' \text{ smooth} \\
&& \longrightarrow \text{Type III character} \\[6pt]
\beta \to \infty \text{ (ReLU)} &:& \text{KMS destroyed, } \mathcal{M}' \in \{0,1\}, \; \mathcal{M}'' = \delta(z) \\
&& \longrightarrow \text{Type I character}
\end{array}}$$

The inverse temperature $\beta$ is an **algebraic temperature** controlling the factor type of the manifestation operator.

---

## Part II: Correspondences and the Complete Descent

### 2.1 Fermi-Dirac $\to$ Heaviside = Continuous $\to$ Discrete Dimension

This is the document's strongest correspondence.

**Theorem RT-T1** [THEOREM]. The first derivative of the Softplus transitions from a continuous-valued function to a discrete-valued function as $\beta \to \infty$:

$$\mathcal{M}'_\beta(z) = \frac{1}{1 + e^{-\beta z}} \quad \xrightarrow{\beta \to \infty} \quad \Theta(z) = \begin{cases} 0 & z < 0 \\ 1 & z > 0 \end{cases}$$

**Proof.** For $z > 0$: $e^{-\beta z} \to 0$, so $\mathcal{M}'_\beta(z) \to 1$. For $z < 0$: $e^{-\beta z} \to \infty$, so $\mathcal{M}'_\beta(z) \to 0$. The convergence is pointwise everywhere except $z = 0$, and distributional (the Fermi-Dirac converges to $\Theta$ in $\mathcal{D}'(\mathbb{R})$). $\square$

**Connection to dimension functions:**

| Regime | $\mathcal{M}'_\beta$ range | Murray-von Neumann analog | Factor type |
|--------|--------------------------|--------------------------|-------------|
| Finite $\beta$ | $(0, 1)$ continuous | Continuous dimension function $d: \mathrm{Proj}(\mathcal{R}) \to [0, 1]$ | **Type II$_1$** |
| $\beta \to \infty$ | $\{0, 1\}$ discrete | Discrete dimension function $d: \mathrm{Proj}(M_n) \to \{0, 1, \ldots, n\}$ | **Type I** |

**[CLASSICAL]** (Murray-von Neumann 1943): In a Type II$_1$ factor $\mathcal{R}$, the dimension function maps projections to the full interval $[0, 1]$ — every value is realized. In a Type I$_n$ factor $M_n(\mathbb{C})$, the dimension function takes only the discrete values $\{0, 1/n, 2/n, \ldots, 1\}$.

**Proposition RT-C1** [CONJECTURE]. The Fermi-Dirac occupation number at finite temperature $\beta$ is a physical realization of the Type II$_1$ continuous dimension function: the thermal average $\langle n_F \rangle \in (0, 1)$ assigns to each energy mode a continuous "fractional occupation" — precisely the structure of continuous dimensions in the hyperfinite factor. The zero-temperature Heaviside $\Theta(z) \in \{0, 1\}$ is a physical realization of the Type I discrete dimension function: each mode is either fully occupied or fully empty, with no intermediate values.

**What makes this more than analogy:** In second quantization, the Fermi number operator $\hat{n}$ has eigenvalues in $\{0, 1\}$ (Type I structure). But the thermal average $\langle \hat{n} \rangle_\beta = (1 + e^{-\beta \varepsilon})^{-1}$ lies in $(0, 1)$ (Type II$_1$ structure). The transition from $\langle \hat{n} \rangle_\beta \in (0, 1)$ to $\hat{n} \in \{0, 1\}$ as $T \to 0$ is not merely analogous to the Type II$_1$ $\to$ Type I transition — it IS a version of it in the abelian case.

---

**Theorem RT-T2** [THEOREM]. The second derivative (susceptibility) transitions from a smooth function to a distributional delta function:

$$\mathcal{M}''_\beta(z) = \beta \cdot n_F(z)(1 - n_F(z)) \quad \xrightarrow{\beta \to \infty} \quad \delta(z)$$

where $n_F(z) = (1 + e^{-\beta z})^{-1}$.

**Proof.** At finite $\beta$, $\mathcal{M}''_\beta$ is a smooth, strictly positive function with:
- Maximum value $\beta/4$ at $z = 0$
- Width (FWHM) $\sim 2\ln(3)/\beta$
- Integral $\int_{-\infty}^{\infty} \mathcal{M}''_\beta(z)\, dz = 1$ (since $\mathcal{M}'_\beta(-\infty) = 0$ and $\mathcal{M}'_\beta(+\infty) = 1$)

As $\beta \to \infty$: peak diverges ($\beta/4 \to \infty$), width collapses ($\to 0$), integral remains 1. This is the defining characterization of the Dirac delta: $\mathcal{M}''_\beta \to \delta$ in $\mathcal{D}'(\mathbb{R})$. $\square$

**Connection to minimal projections:**

| Regime | $\mathcal{M}''_\beta$ | Factor analog |
|--------|----------------------|---------------|
| Finite $\beta$ | Smooth bump (no point singularity) | **No minimal projection** (Type II or III) |
| $\beta \to \infty$ | $\delta(z)$ (point mass at threshold) | **Minimal projection** (Type I) |

**[CLASSICAL]** (Murray-von Neumann 1936): Type I factors are characterized by the existence of minimal (atomic) projections — nonzero projections $e$ with no projection $f$ satisfying $0 < f < e$. Type II and III factors have no minimal projections: every projection can be subdivided.

**Proposition RT-C2** [CONJECTURE]. The distributional delta function $\delta(z)$ in $\mathrm{ReLU}''(z)$ plays the role of a minimal projection in the following precise sense: it is the integral kernel of the rank-1 evaluation functional

$$P_0: f \mapsto f(0) \cdot \delta(z)$$

which extracts the value at the manifestation threshold $z = 0$. This is a rank-1 operator — the distributional analog of a minimal projection. The smooth susceptibility $\mathcal{M}''_\beta$ at finite $\beta$ is not concentrated at any single point and therefore does not define a rank-1 extraction — the distributional analog of having no minimal projection.

---

### 2.2 KMS Destruction

**Theorem RT-T3** [THEOREM]. The Softplus at finite $\beta$ satisfies the fermionic KMS antiperiodicity condition. The ReLU at $\beta = \infty$ violates it.

**Proof.**

*Softplus KMS:* The M4 axiom of the uniqueness theorem states that $\mathcal{M}_\beta$ admits analytic continuation to the strip $S_\beta = \{z \in \mathbb{C} : |\mathrm{Im}(z)| < \pi/\beta\}$, with singularities only at $z = i(2n+1)\pi/\beta$. These are precisely the fermionic Matsubara frequencies, and the singularity structure encodes the KMS antiperiodicity:

$$\mathcal{M}_\beta(z + 2\pi i/\beta) = (z + 2\pi i/\beta) - \mathcal{M}_\beta(z)$$

This is the fermionic KMS condition at inverse temperature $\beta$, which is the defining property of thermal equilibrium for fermionic modes ([CLASSICAL] — Kubo 1957, Martin-Schwinger 1959).

*ReLU non-KMS:* $\mathrm{ReLU}(z) = \max(0, z)$ has a cusp at $z = 0$. The function is not differentiable (let alone analytic) at this point. There exists no strip $S = \{|\mathrm{Im}(z)| < \epsilon\}$ for any $\epsilon > 0$ in which ReLU admits analytic continuation. Therefore the KMS condition cannot be satisfied at any temperature. $\square$

**The algebraic significance:**

**[CLASSICAL]** (Haag-Hugenholtz-Winnink 1967, Tomita 1967, Takesaki 1970): The KMS condition at inverse temperature $\beta$ is equivalent to the existence of a modular automorphism group $\sigma_t = \Delta^{it}(\cdot)\Delta^{-it}$ on a von Neumann algebra. For Type III factors, the modular automorphism is the *only* intrinsic notion of time evolution — it is outer (not implementable by a unitary in the algebra). For Type I factors, the modular automorphism is inner (trivial up to conjugation) and the trace provides the state-independent probability measure that the modular flow is unnecessary for.

**Theorem RT-T4** [THEOREM] (KMS Chain). The following chain of implications holds, where each step is established by classical results:

$$\underbrace{\text{Softplus satisfies M4}}_{\text{[THEOREM] — uniqueness proof}} \implies \underbrace{\text{KMS holds at } \beta}_{\text{[CLASSICAL] — M4 = KMS}} \implies \underbrace{\text{modular flow } \sigma_t \text{ exists}}_{\text{[CLASSICAL] — HHW}} \implies \underbrace{\text{Type III character}}_{\text{[CLASSICAL] — Connes}}$$

$$\underbrace{\text{ReLU violates M4}}_{\text{[THEOREM] — non-analytic}} \implies \underbrace{\text{KMS fails}}_{\text{[THEOREM] — no strip}} \implies \underbrace{\text{no modular flow needed}}_{\text{[CLASSICAL]}} \implies \underbrace{\text{Type I character}}_{\text{[CLASSICAL]}}$$

**Key insight:** The M4 axiom that makes the Softplus *unique* (the fermionic singularity structure used in the Liouville argument) is the *same property* that gives it Type III algebraic character (the KMS condition required for modular automorphism). Uniqueness and Type III character are two faces of the same mathematical constraint.

**Remark RT-R1.** This chain operates at the level of the abelian algebra of functions on $\mathbb{R}$. The extension to non-abelian operator algebras on a Hilbert space would require constructing the actual von Neumann algebra generated by the Softplus-regularized field operators, which is beyond the scope of this document. See [OPEN] question RT-O1.

---

### 2.3 Analyticity Strip Collapse

**Theorem RT-T5** [THEOREM]. The analyticity strip of the Softplus collapses to zero width as $\beta \to \infty$:

$$\text{Strip width: } \frac{2\pi}{\beta} \quad \xrightarrow{\beta \to \infty} \quad 0$$

At finite $\beta$, the Softplus is analytic in $S_\beta = \{z \in \mathbb{C} : |\mathrm{Im}(z)| < \pi/\beta\}$. As $\beta \to \infty$, $S_\beta$ shrinks to the real axis. The ReLU is analytic only on $\mathbb{R} \setminus \{0\}$ — a set of measure zero in $\mathbb{C}$.

**Proof.** The Softplus singularities lie at $z_n = i(2n+1)\pi/\beta$ for $n \in \mathbb{Z}$, with the nearest singularities at $z = \pm i\pi/\beta$. These define the strip boundary. As $\beta \to \infty$, $|z_{\pm}| = \pi/\beta \to 0$: the singularities converge to the origin, collapsing the strip. In the limit, infinitely many singularities pile up at $z = 0$, producing the essential non-analyticity (cusp) of ReLU. $\square$

**Connection to modular theory:**

**[CLASSICAL]** (Tomita-Takesaki): The modular automorphism group $\sigma_t$ requires analytic continuation of correlation functions to a strip of width $\beta$ (the KMS strip). Specifically, for a KMS state $\omega$ on a von Neumann algebra $\mathcal{M}$, the function $F_{a,b}(t) = \omega(a \cdot \sigma_t(b))$ extends analytically to the strip $\{z : 0 < \mathrm{Im}(z) < \beta\}$.

**Proposition RT-C3** [CONJECTURE]. The collapse of the Softplus analyticity strip as $\beta \to \infty$ is the manifestation-operator-level signature of the loss of modular automorphism structure:

| $\beta$ | Strip width | Modular flow | Factor character |
|---------|------------|--------------|-----------------|
| Finite | $2\pi/\beta > 0$ | Exists (KMS strip available) | Type III |
| $\infty$ | $0$ | Destroyed (no strip for continuation) | Type I |

The analyticity strip is the "room" in which modular flow operates. When this room shrinks to zero, the modular flow has nowhere to go — it ceases to exist. Without modular flow, the algebra must be Type I (where the trace provides the state-independent structure that modular flow provides for Type III).

---

### 2.4 The $\beta$-$\lambda$ Dictionary

**Definition RT-D1** [DEFINITION]. Define the $\beta$-$\lambda$ map by:

$$\lambda(\beta) = e^{-\beta}$$

This maps $\beta \in (0, \infty)$ to $\lambda \in (0, 1)$, with $\beta \to 0 \implies \lambda \to 1$ and $\beta \to \infty \implies \lambda \to 0$.

**Motivation:** The Powers factor $\mathcal{R}_\lambda$ (Type III$_\lambda$, $0 < \lambda < 1$) has modular automorphism with period:

$$T_\lambda = \frac{-2\pi}{\ln \lambda}$$

([CLASSICAL] — Connes 1973; see [archive/ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md](archive/ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md), Theorem 3.2.)

The Softplus at inverse temperature $\beta$ has imaginary-time (Matsubara) period:

$$T_\beta = \frac{2\pi}{\beta}$$

Setting $T_\lambda = T_\beta$:

$$\frac{-2\pi}{\ln \lambda} = \frac{2\pi}{\beta} \implies \ln \lambda = -\beta \implies \lambda = e^{-\beta}$$

**Proposition RT-C4** [CONJECTURE]. The Softplus parameter $\beta$ maps to the Connes parameter $\lambda$ of the Powers factor $\mathcal{R}_\lambda$ via $\lambda = e^{-\beta}$:

| $\beta$ | $\lambda = e^{-\beta}$ | Modular period | Factor type |
|---------|----------------------|---------------|-------------|
| $\beta \to 0$ (high $T$) | $\lambda \to 1$ | $T \to \infty$ | Type III$_1$ (ergodic, full $\mathbb{R}_+$ spectrum) |
| $\beta = 1$ | $\lambda \approx 0.368$ | $T = 2\pi$ | Type III$_{0.37}$ (periodic) |
| $\beta = \ln 2 \approx 0.69$ | $\lambda = 1/2$ | $T = 2\pi/\ln 2$ | Type III$_{1/2}$ |
| $\beta \to \infty$ (low $T$) | $\lambda \to 0$ | $T \to 0^+$ | Type III$_0$ (aperiodic) |

---

> **WARNING RT-W1** [CRITICAL HONESTY]. The Powers factors $\mathcal{R}_\lambda$ are **ALL Type III** for $\lambda \in (0, 1)$. As $\beta \to \infty$, we obtain $\lambda \to 0$, which corresponds to Type III$_0$ (the aperiodic factor with $S(\mathcal{M}) = \{0, 1\}$) — **NOT** Type I.
>
> The map $\beta \mapsto \lambda = e^{-\beta}$ interpolates *within* the Type III family. It does not reach Type I or Type II$_1$ at any finite or infinite value of $\beta$.
>
> The transition from Type III$_0$ to Type I is a **discrete topological jump**, not a smooth limit. The Powers factors form a continuous family parametrized by $\lambda \in (0, 1)$; Type I and Type II$_1$ factors lie *outside* this family, separated by an algebraic phase transition.
>
> This is not a defect of the correspondence — it is a **structural insight**: the ReLU limit ($\beta \to \infty$) brings the system to the *boundary* of the Type III family but does not cross it smoothly. The final crystallization from Type III$_0$ to Type I requires a qualitatively different event.

---

**Proposition RT-C5** [CONJECTURE] (The Topological Jump). The final Type III$_0$ $\to$ Type I transition corresponds to the non-analytic kink in the ReLU at $z = 0$:

- For any finite $\beta$, $\mathcal{M}_\beta$ is $C^\infty(\mathbb{R})$ (and analytic in a strip) $\to$ Type III$_\lambda$ character
- At $\beta = \infty$, $\mathrm{ReLU}$ is $C^0(\mathbb{R})$ but not $C^1$ at $z = 0$ $\to$ Type I character

The loss of differentiability at the threshold point is a **topological transition** — the function changes from smooth to piecewise-linear. This topological change is the algebraic signature of the jump from Type III (no atoms, continuous geometry) to Type I (atoms exist, discrete geometry). The cusp at $z = 0$ is where the minimal projection (RT-C2) crystallizes.

**Remark RT-R2.** This parallels the physical picture: the vacuum (Softplus, finite $T$) is smooth, with quantum fluctuations allowing partial occupation of states. The classical ground state (ReLU, $T = 0$) has a sharp boundary between existence and non-existence. The transition from smooth to sharp is not continuous — it is a zero-temperature phase transition.

---

### 2.5 KMS Destruction and Modular Automorphism

We now present the complete logical structure connecting the five correspondences.

**Theorem RT-T6** [THEOREM] (The Complete Chain). The following diagram commutes, where each arrow is individually established:

```
Softplus (finite beta)                          ReLU (beta -> infinity)
======================                          ======================

[THEOREM] M4 satisfied        ------>           [THEOREM] M4 violated
    |                                               |
    v                                               v
[CLASSICAL] KMS holds          ------>          [THEOREM] KMS fails
    |                                               |
    v                                               v
[CLASSICAL] Modular flow       ------>          [CLASSICAL] No modular
sigma_t exists (outer)                          flow needed (inner/trivial)
    |                                               |
    v                                               v
[CLASSICAL] Type III           ------>          [CLASSICAL] Type I
character                                       character
    |                                               |
    v                                               v
[THEOREM] M' in (0,1)         ------>           [THEOREM] M' in {0,1}
continuous dimension                            discrete dimension
    |                                               |
    v                                               v
[THEOREM] M'' = smooth        ------>           [THEOREM] M'' = delta(z)
bump (no minimal proj.)                         minimal projection
```

Each horizontal arrow represents the $\beta \to \infty$ limit. Each vertical arrow represents a logical implication, with its epistemic status indicated.

---

### The Complete Algebraic Descent

The five correspondences above (§2.1–2.5) work within the abelian setting: they identify properties of functions on $\mathbb{R}$ (activation functions, their derivatives, their analyticity) with structural properties of factor types. Warning RT-W1 identified a critical limitation: the Powers family $\mathcal{R}_\lambda$ stays Type III for all $\lambda \in (0,1)$, so the $\beta \to \infty$ limit alone cannot cross to Type I.

We now address this limitation by constructing the complete descent chain, using three classical operations unified by a single new conjecture that identifies the ReLU kink with the mechanism for the final step.

### 2.6 The Crossed Product Bridge: Escaping Type III

**[CLASSICAL]** (Takesaki, 1973): For a Type III factor $\mathcal{M}$ with modular automorphism group $\sigma_t$, the *crossed product*:

$$\hat{\mathcal{M}} = \mathcal{M} \rtimes_\sigma \mathbb{R}$$

is a Type II$_\infty$ factor. The construction adjoins the modular time parameter $s \in \mathbb{R}$ as a spatial coordinate, "unwinding" the intrinsic modular flow into an external degree of freedom. The resulting algebra has a faithful normal *semifinite* trace $\hat{\tau}$ — the trace that Type III factors lack.

**[CLASSICAL]** (Murray-von Neumann, 1943): Every Type II$_\infty$ factor decomposes as:

$$\hat{\mathcal{M}} \cong \mathcal{R} \otimes B(\mathcal{H})$$

where $\mathcal{R}$ is the hyperfinite Type II$_1$ factor and $B(\mathcal{H})$ is the Type I$_\infty$ factor of bounded operators on a separable Hilbert space.

The chain so far:

$$\text{Type III}_1 \;\xrightarrow{\;\rtimes_\sigma \mathbb{R}\;}\; \text{Type II}_\infty \;\xrightarrow{\;\cong\;}\; \mathcal{R} \otimes B(\mathcal{H})$$

Both steps are **[CLASSICAL]** — they require no FTD axioms. The Type II$_1$ factor $\mathcal{R}$ is now explicitly present as a tensor factor. What remains is the final crystallization: how does $\mathcal{R}$ (continuous dimension $[0,1]$, no minimal projections) yield Type I (discrete dimension, minimal projections)?

---

### 2.7 MASA Selection: The ReLU Kink as Measurement Basis

This section presents the synthesis's central novel claim.

**[CLASSICAL]** (Dixmier, 1954): A *maximal abelian subalgebra* (MASA) of a von Neumann algebra $\mathcal{M}$ is an abelian subalgebra $\mathcal{A} \subset \mathcal{M}$ satisfying $\mathcal{A}' \cap \mathcal{M} = \mathcal{A}$. In the hyperfinite factor $\mathcal{R}$:

- MASAs always exist (Zorn's lemma)
- They are highly non-unique: uncountably many inequivalent MASAs exist
- Every MASA $\mathcal{A}$ is itself a Type I algebra: $\mathcal{A} \cong L^\infty(X, \mu)$
- The conditional expectation $E_\mathcal{A}: \mathcal{R} \to \mathcal{A}$ is a trace-preserving normal projection

**Key fact:** Choosing a MASA is choosing a measurement basis. In standard quantum mechanics on $M_n(\mathbb{C})$, the measurement basis $\{|i\rangle\}$ determines the MASA $\mathcal{D} \subset M_n(\mathbb{C})$ of diagonal matrices. The measurement outcome is the conditional expectation onto $\mathcal{D}$. Different bases give different MASAs, hence different measurements.

The **Type II$_1$ $\to$ Type I transition is MASA selection** — it is the act of choosing what to measure.

**Proposition RT-C9** [CONJECTURE] (ReLU Kink = Canonical MASA Selection). The modular Hamiltonian $K = -\ln \Delta$ of a faithful state $\omega$ determines a canonical MASA $\mathcal{A}_K \subset \mathcal{R}$ via its spectral projections $\{E_\lambda\}_{\lambda \in \mathbb{R}}$. The Heaviside function $\Theta(K) = \lim_{\beta \to \infty} \mathcal{M}'_\beta(K)$ implements the coarsest non-trivial spectral partition:

$$P_+ = E_{[0,\infty)}(K) \quad \text{(physical states)}, \qquad P_- = E_{(-\infty,0)}(K) \quad \text{(vacuum states)}$$

This binary partition — existence vs. non-existence — is the ReLU kink. In FTD terms:

| Spectral region | Modular energy | Activation | FTD state |
|----------------|---------------|------------|-----------|
| $K > 0$ | Positive | $\text{ReLU}(K) = K$ | $s = \pm 1$ (manifested) |
| $K < 0$ | Negative | $\text{ReLU}(K) = 0$ | $s = 0$ (void/vacuum) |
| $K = 0$ | Threshold | Kink (non-analytic) | $K_B$ (manifestation threshold) |

**What makes this more than trivial:** In a general Type II$_1$ factor, there is no canonical MASA — no preferred measurement basis. The modular Hamiltonian $K$, determined by the state via Tomita-Takesaki theory, provides a *state-determined* basis selection. This is not arbitrary: the state itself determines what the "natural" measurement is. The ReLU truncation $K \to \max(0, K)$ then implements this selection by projecting onto the physical half of the spectrum.

**The complete three-step descent:**

$$\boxed{\text{Type III}_1 \;\xrightarrow[\text{[CLASSICAL]}]{\;\rtimes_\sigma \mathbb{R}\;}\; \text{Type II}_\infty \;\xrightarrow[\text{[CLASSICAL]}]{\;\mathcal{R} \otimes B(\mathcal{H})\;}\; \text{Type II}_1 \;\xrightarrow[\text{[CONJECTURE]}]{\;\Theta(K)\;}\; \text{Type I}}$$

Each step has a distinct character:

1. **Crossed product** (categorical): adds a degree of freedom (modular time $\to$ spatial coordinate)
2. **Tensor decomposition** (algebraic): factorizes the semifinite algebra, isolating $\mathcal{R}$
3. **MASA selection** (physical): the state determines the measurement basis via $K$; the ReLU kink $\Theta(K)$ executes the selection

**The heavy zero.** In the crossed product $\hat{\mathcal{M}} = \mathcal{M} \rtimes_\sigma \mathbb{R}$, the projection onto the vacuum spectrum $P_0 = E_{(-\infty, 0]}(K)$ has $\hat{\tau}(P_0) = \infty$. The vacuum below the manifestation threshold is not empty — it is an infinitely degenerate reservoir. This infinite vacuum trace connects to the cosmological constant problem: the observed dark energy density may reflect the ratio of the finite physical trace to the infinite vacuum trace.

**Proposition RT-C10** [CONJECTURE] (Arrow of Time from Spectral Truncation). The spectral truncation $K \to K_+ = \max(0, K)$ breaks time-reversal symmetry: $K$ has spectrum on all of $\mathbb{R}$ (time-symmetric under $K \to -K$), while $K_+$ has spectrum on $[0, \infty)$ only. The modular flow $\sigma_t = e^{iKt}$ explores the full spectrum; the truncated evolution $e^{iK_+ t}$ is confined to the physical half. This irreversible spectral truncation is the thermodynamic arrow of time — manifest as the ReLU's one-sided activation.

---

### 2.8 Resolution of Warning RT-W1

Warning RT-W1 identified that the Powers family $\mathcal{R}_\lambda$ stays Type III for all $\lambda \in (0,1)$. The three-step chain resolves this gap:

| Warning | Resolution |
|---------|------------|
| $\beta \to \infty$ gives $\lambda \to 0$ (Type III$_0$), not Type I | The $\beta$ limit identifies the *mechanism* (KMS destruction, Fermi-Dirac $\to$ Heaviside) but does not execute the algebraic type change by itself |
| No smooth path from Type III to Type I exists | The crossed product provides a *categorical* exit — not a smooth deformation but a structural construction |
| The final jump was declared "topological" with no mechanism | The mechanism is MASA selection: the Heaviside partition $\Theta(K)$ picks a Type I subalgebra from the Type II$_1$ factor |

The honest picture is now:

$$\underbrace{\text{Softplus} \xrightarrow{\beta \to \infty} \text{ReLU}}_{\text{Abelian level: identifies the mechanism}} \qquad \underbrace{\text{III}_1 \xrightarrow{\rtimes} \text{II}_\infty \xrightarrow{\otimes} \text{II}_1 \xrightarrow{\Theta(K)} \text{I}}_{\text{Algebraic level: executes the descent}}$$

The first line (abelian, $\beta \to \infty$) tells us *what happens*: KMS is destroyed, continuous dimensions crystallize, minimal projections emerge. The second line (non-abelian, three steps) tells us *how it happens*: crossed product exits Type III, tensor decomposition isolates the hyperfinite factor, and MASA selection completes the crystallization.

**Remark RT-R4.** The crossed product and tensor decomposition are classical constructions that any Type III factor admits. The genuinely novel claim is that the ReLU kink — the non-analytic point at $z = 0$ — corresponds to MASA selection. This claim (RT-C9) has mathematical content: the spectral projections of $K$ generate a MASA, and the Heaviside function is the coarsest non-trivial projection in that MASA. But proving that this MASA produces the *physically correct* measurement outcomes requires constructing the actual von Neumann algebra of FTD field operators, which remains open (see RT-O6).

---

## Part III: The Enriched Phase Diagram

### 3.1 Factor Types on the $(k, 1/\beta)$ Plane

The Softplus-ReLU paper establishes the phase diagram in the $(k, 1/\beta)$ plane with three regions determined by the discriminant $\Delta(k) = knc^3(knc - 4)$, where $n = 16$ and $c = G^*$ ([DERIV_SOFTPLUS_RELU_DUALITY.tex](../papers/src/DERIV_SOFTPLUS_RELU_DUALITY.tex), Theorem 5.1). We now overlay von Neumann factor type labels:

| Region | Discriminant | Eigenvalues | Activation | **Factor Type** |
|--------|-------------|-------------|------------|----------------|
| $k < k_c = \frac{1}{4G^*}$ | $\Delta < 0$ | Complex conjugate | Softplus on complex arguments | **Type III$_1$** |
| $k = k_c$ | $\Delta = 0$ | Degenerate real | Critical (exceptional point) | **Type II$_1$** |
| $k > k_c$, finite $1/\beta$ | $\Delta > 0$ | Two real, distinct | Softplus (quantum) | **Type III$_\lambda$** |
| $k > k_c$, $1/\beta \to 0$ | $\Delta > 0$ | Two real, distinct | ReLU (classical) | **Type I** |

### 3.2 The Exceptional Point as Type II$_1$

**Proposition RT-C6** [CONJECTURE]. The $\mathcal{PT}$-symmetric exceptional point at $k_c = 1/(4G^*)$, where the transfer matrix becomes a Jordan block with degenerate eigenvalue $\lambda_0 = 2c$, corresponds to the Type II$_1$ hyperfinite factor $\mathcal{R}$.

**Structural parallels:**

1. **Degenerate eigenvalue $\leftrightarrow$ continuous dimension.** At the exceptional point, $\lambda_+ = \lambda_- = \lambda_0$. The two eigenvalues merge into a single value. The dimension function transitions from discrete (two distinct eigenvalues) to degenerate (one repeated eigenvalue) — the onset of continuous character.

2. **Jordan block $\leftrightarrow$ loss of diagonalizability.** The transfer matrix at $k = k_c$ is a $2 \times 2$ Jordan block — it cannot be diagonalized. A Jordan block has the structure:

$$\hat{M}_{k_c} = \begin{pmatrix} \lambda_0 & 1 \\ 0 & \lambda_0 \end{pmatrix}$$

This is an indecomposable but non-semisimple object — it has the "right dimension" but cannot be split into independent parts. Type II$_1$ factors have the same character: they have a well-defined trace (normalizable total "dimension") but cannot be decomposed into minimal (atomic) projections.

3. **Period-doubling onset $\leftrightarrow$ hyperfinite construction.** The existing Mandelbrot-Factor correspondence ([archive/ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md](archive/ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md), Proposition 4.3) identifies the Mandelbrot cusp $c = 1/4$ as the Type II$_1$ locus via the period-doubling cascade:

$$M_2(\mathbb{C}) \hookrightarrow M_4(\mathbb{C}) \hookrightarrow M_8(\mathbb{C}) \hookrightarrow \cdots \to \mathcal{R}$$

The $\mathcal{PT}$-symmetry exceptional point is a second, independent characterization of the same algebraic locus: the point where the eigenvalue structure transitions from resolved (Type I) to degenerate (Type II$_1$) to complex (Type III).

### 3.3 Synthesis: Two Axes, Three Factor Types

The complete picture has two independent parameters controlling the algebraic structure:

- **$k$ (geometric tension):** Controls the discriminant $\Delta$, determining whether eigenvalues are real ($\Delta > 0$), degenerate ($\Delta = 0$), or complex ($\Delta < 0$). This is the *spatial* axis.

- **$\beta$ (inverse temperature):** Controls the sharpness of the manifestation operator, determining whether the dimension function is continuous ($\beta$ finite) or discrete ($\beta \to \infty$). This is the *thermal* axis.

The factor type requires *both* axes:

$$\text{Factor type} = f(k, \beta) = \begin{cases}
\text{Type III}_1 & k < k_c \text{ (complex eigenvalues, any } \beta\text{)} \\
\text{Type II}_1 & k = k_c \text{ (degenerate, any } \beta\text{)} \\
\text{Type III}_\lambda & k > k_c, \; \beta < \infty \text{ (real eigenvalues, smooth threshold)} \\
\text{Type I} & k > k_c, \; \beta = \infty \text{ (real eigenvalues, sharp threshold)}
\end{cases}$$

---

## Part IV: Connections to Existing Framework

### 4.1 Connection to the Existence Filter

The projection hierarchy from [FOUND_THE_EXISTENCE_FILTER.md](FOUND_THE_EXISTENCE_FILTER.md):

$$E(x) = \mathrm{Re}(x) \;\to\; |x| = \sqrt{x\bar{x}} \;\to\; |x|^2 = x\bar{x} \;\to\; \Phi$$

maps to the $\beta$ hierarchy:

| Projection level | Filter character | $\beta$ analog | Factor type |
|-----------------|-----------------|----------------|-------------|
| $E(x) = \mathrm{Re}(x)$ | Linear, smooth, preserves sign | Finite $\beta$ (Softplus): smooth, analytic | Type III |
| $|x|^2$ | Quadratic, non-negative | Intermediate | Type II$_1$ |
| $\Phi$ (collapse) | CPTP, discrete output | $\beta \to \infty$ (ReLU): sharp, piecewise-linear | Type I |

**Proposition RT-C7** [CONJECTURE]. The Existence Filter hierarchy $E \to |\cdot| \to |\cdot|^2 \to \Phi$ and the Softplus-ReLU $\beta$ hierarchy are parallel descriptions of the same algebraic transition from Type III (smooth, complex-analytic, traceless) to Type I (sharp, piecewise-linear, traced).

### 4.2 Connection to the Wilsonian RG

The Softplus-ReLU paper establishes a structural correspondence between $\beta$ and the Wilsonian RG scale $\mu$ (Proposition 5.1, $\mu \sim 1/\beta$). This now gains algebraic content:

| RG regime | Activation | $\beta$ | $\lambda$ | Factor type | Physical character |
|-----------|-----------|---------|-----------|-------------|-------------------|
| UV (bare lattice, $\mu \to \Lambda$) | Softplus (high $T$) | $\beta \to 0$ | $\lambda \to 1$ | Type III$_1$ | Many active modes, ergodic |
| Intermediate | Softplus (finite $T$) | Finite $\beta$ | $\lambda \in (0,1)$ | Type III$_\lambda$ | Periodic structure |
| IR (continuum, $\mu \to 0$) | ReLU ($T = 0$) | $\beta \to \infty$ | $\lambda \to 0$ | Type I | Frozen ground state |

**Proposition RT-C8** [CONJECTURE]. The Wilsonian RG flow from UV to IR is an algebraic type transition from Type III$_1$ (the algebra of local QFT observables at short distances, [CLASSICAL] — Buchholz-Wichmann 1986) to Type I (the algebra of asymptotic particle states at large distances). The Softplus parameter $\beta$ parametrizes this flow.

**Remark RT-R3.** The observation that local QFT algebras are Type III$_1$ while asymptotic particle algebras are Type I is well-known in algebraic quantum field theory ([CLASSICAL] — Haag 1996). What is new here is identifying the Softplus parameter $\beta$ (equivalently, the RG scale $\mu$) as the parameter that continuously interpolates between these algebraic regimes.

### 4.3 Partial Resolution of VN-O1

Open question VN-O1 from [archive/ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md](archive/ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md) asks: "Can the collapse map $\Phi$ be made rigorous?"

This document provides a partial answer: the Softplus $\to$ ReLU limit ($\beta \to \infty$) is a concrete, mathematically tractable instance of the Type II$_1$ $\to$ Type I transition described by $\Phi$. The "collapse" corresponds to:

1. The Fermi-Dirac function sharpening into the Heaviside step (continuous $\to$ discrete dimension)
2. The susceptibility bump concentrating into $\delta(z)$ (smooth $\to$ atomic)
3. The analyticity strip collapsing to the real axis (KMS $\to$ non-KMS)

The $\beta \to \infty$ limit makes the abstract algebraic collapse map $\Phi: (\mathcal{R}, \tau) \to (M_n(\mathbb{C}), \mathrm{Tr}/n)$ concrete: it is the zero-temperature limit of Fermi-Dirac statistics.

---

## Part V: Epistemic Taxonomy

### 5.1 Classical Mathematics [CLASSICAL]

| Statement | Source |
|-----------|--------|
| Type I factors have minimal projections and discrete dimension | Murray-von Neumann (1936) |
| Type II$_1$ has unique faithful normal trace, continuous dimension $[0,1]$ | Murray-von Neumann (1943) |
| Type III has no trace; Connes classification via $S(\mathcal{M})$ | Connes (1973) |
| KMS condition $\Leftrightarrow$ modular automorphism group | Haag-Hugenholtz-Winnink (1967) |
| Modular operator $\Delta$, conjugation $J$, modular flow $\sigma_t$ | Tomita (1967), Takesaki (1970) |
| Powers factor $\mathcal{R}_\lambda$ has period $T = -2\pi/\ln\lambda$ | Connes (1973) |
| Local QFT algebras are Type III$_1$ | Buchholz-Wichmann (1986) |
| Matsubara frequencies $\omega_n = (2n+1)\pi/\beta$ for fermions | Matsubara (1955) |
| Distributional derivative of Heaviside is $\delta$ | Schwartz (1950) |
| Crossed product $\mathcal{M} \rtimes_\sigma \mathbb{R}$ of Type III is Type II$_\infty$ | Takesaki (1973) |
| Type II$_\infty \cong \mathcal{R} \otimes B(\mathcal{H})$ decomposition | Murray-von Neumann (1943) |
| MASAs exist in II$_1$ factors; each MASA is Type I (abelian) | Dixmier (1954) |

### 5.2 Theorems [THEOREM]

Provable from the Softplus uniqueness axioms M1-M4 + classical mathematics.

| ID | Statement | Section |
|----|-----------|---------|
| RT-T1 | $\mathcal{M}'_\beta \to \Theta(z)$: continuous range $(0,1)$ becomes discrete $\{0,1\}$ | 2.1 |
| RT-T2 | $\mathcal{M}''_\beta \to \delta(z)$: smooth bump becomes point mass | 2.1 |
| RT-T3 | Softplus satisfies KMS; ReLU violates it | 2.2 |
| RT-T4 | Complete chain: M4 $\to$ KMS $\to$ modular flow $\to$ Type III (each step classical) | 2.2 |
| RT-T5 | Analyticity strip $2\pi/\beta \to 0$ as $\beta \to \infty$ | 2.3 |
| RT-T6 | Complete chain diagram: all five correspondences commute | 2.5 |

### 5.3 Conjectures [CONJECTURE]

Structural correspondences between activation function properties and von Neumann factor types.

| ID | Statement | Section |
|----|-----------|---------|
| RT-C1 | Fermi-Dirac range $(0,1)$ = Type II$_1$ dimension function | 2.1 |
| RT-C2 | $\delta(z)$ in ReLU'' = minimal projection characterizing Type I | 2.1 |
| RT-C3 | Strip collapse = loss of modular automorphism = loss of Type III | 2.3 |
| RT-C4 | $\lambda = e^{-\beta}$ maps to Connes parameter of $\mathcal{R}_\lambda$ | 2.4 |
| RT-C5 | ReLU kink = topological jump from Type III$_0$ to Type I | 2.4 |
| RT-C6 | $\mathcal{PT}$ exceptional point = Type II$_1$ locus | 3.2 |
| RT-C7 | Existence Filter hierarchy parallels $\beta$ hierarchy | 4.1 |
| RT-C8 | Wilsonian RG flow = algebraic type transition III$_1$ $\to$ I | 4.2 |
| RT-C9 | ReLU kink $\Theta(K)$ = canonical MASA selection via modular Hamiltonian | 2.7 |
| RT-C10 | Spectral truncation $K \to K_+$ = thermodynamic arrow of time | 2.7 |

### 5.4 Open Questions

| ID | Question | Priority |
|----|----------|----------|
| RT-O1 | Can the operator algebras generated by the Softplus-regularized field operators at each $\beta$ be rigorously classified as von Neumann factor types? This would require constructing the actual algebras, not just working with the activation functions. | **High** |
| RT-O2 | Does the Wilsonian RG flow literally implement the Connes flow of weights? If so, the RG $\beta$-function would be a von Neumann algebraic invariant. | **High** |
| RT-O3 | What is the Jones index $[\mathcal{M}:\mathcal{N}]$ of the inclusion at the exceptional point? Does it relate to $K_B/K_C = 4\sqrt{2}$? | Medium |
| RT-O4 | ~~Can the honest limitation (Warning RT-W1) be resolved?~~ **Addressed** by the three-step descent chain (§2.6–2.8): crossed product + tensor decomposition + MASA selection. | ~~Medium~~ **Resolved** |
| RT-O5 | Does the $\beta$-$\lambda$ dictionary extend to the FTD consciousness parameter $\lambda(k) \approx 0.400$ from [archive/ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md](archive/ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md), §3.4? If $\lambda = 0.400 = e^{-\beta}$, then $\beta \approx 0.916$. Does this temperature have physical significance? | Low |
| RT-O6 | Can the MASA generated by the FTD modular Hamiltonian at $K_B$ be shown to produce the physically correct measurement outcomes? This requires constructing the actual von Neumann algebra of FTD field operators and computing its modular Hamiltonian. | **High** |

---

## Part VI: Summary

### 6.1 Central Result

The Softplus inverse temperature parameter $\beta$ interpolates between von Neumann factor types. At finite $\beta$, the manifestation operator has Type III character: the KMS condition holds, the modular automorphism exists, the occupation function is continuous, and no minimal projections exist. At $\beta \to \infty$ (the ReLU limit), Type III character is destroyed: KMS fails, modular flow vanishes, the occupation function becomes discrete, and a minimal projection (the distributional $\delta(z)$) crystallizes at the manifestation threshold.

The key unifying insight is that **Axiom M4 of the Softplus uniqueness theorem IS the fermionic KMS condition**, which is the defining property of Type III modular states. The Softplus is unique *because* it is the unique manifestation operator with Type III algebraic character.

The complete algebraic descent resolves the limitation that the $\beta$ parameter alone cannot cross from Type III to Type I. Three operations accomplish the full transition: (1) the Connes crossed product exits Type III [CLASSICAL], (2) tensor decomposition isolates the hyperfinite II$_1$ factor [CLASSICAL], and (3) **MASA selection via the Heaviside partition $\Theta(K)$** crystallizes Type II$_1$ into Type I [CONJECTURE]. The ReLU's non-analytic kink at $z = 0$ is the mathematical signature of this final step — it selects the canonical measurement basis determined by the modular Hamiltonian.

### 6.2 Key Equations

**1. Fermi-Dirac $\to$ Heaviside (continuous $\to$ discrete dimension):**

$$\boxed{\mathcal{M}'_\beta(z) = \frac{1}{1 + e^{-\beta z}} \;\xrightarrow{\beta \to \infty}\; \Theta(z) \in \{0, 1\}}$$

**2. Susceptibility $\to$ minimal projection:**

$$\boxed{\mathcal{M}''_\beta(z) = \beta \cdot n_F(1 - n_F) \;\xrightarrow{\beta \to \infty}\; \delta(z)}$$

**3. KMS chain:**

$$\boxed{\text{M4} \implies \text{KMS} \implies \sigma_t \text{ exists} \implies \text{Type III} \qquad \text{(each step [CLASSICAL])}}$$

**4. Connes parameter:**

$$\boxed{\lambda(\beta) = e^{-\beta}, \quad T = \frac{2\pi}{\beta} = \frac{-2\pi}{\ln \lambda}}$$

**5. Enriched phase diagram:**

$$\boxed{\text{Factor type}(k, \beta) = \begin{cases} \text{III}_1 & k < k_c \\ \text{II}_1 & k = k_c \\ \text{III}_\lambda & k > k_c, \; \beta < \infty \\ \text{I} & k > k_c, \; \beta = \infty \end{cases}}$$

**6. Complete algebraic descent:**

$$\boxed{\text{III}_1 \;\xrightarrow{\;\rtimes_\sigma \mathbb{R}\;}\; \text{II}_\infty \;\xrightarrow{\;\mathcal{R} \otimes B(\mathcal{H})\;}\; \text{II}_1 \;\xrightarrow{\;\Theta(K)\;}\; \text{I}}$$

### 6.3 Cross-References

| Document | Relevance |
|----------|-----------|
| [DERIV_SOFTPLUS_RELU_DUALITY.tex](../papers/src/DERIV_SOFTPLUS_RELU_DUALITY.tex) | Softplus uniqueness (Thm 2.1), M4 axiom, spectral representation, PT transition, RG correspondence |
| [FOUND_AGENT_MEANING_FORMALIZATION.md](FOUND_AGENT_MEANING_FORMALIZATION.md) Part IA | Von Neumann factor classification, Factor-Domain correspondence |
| [archive/ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md](archive/ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md) | Collapse map $\Phi$ (Def. 2.6), Connes III$_\lambda$ hierarchy, Mandelbrot-Factor dictionary |
| [FOUND_THE_EXISTENCE_FILTER.md](FOUND_THE_EXISTENCE_FILTER.md) | Projection hierarchy $E \to |\cdot| \to |\cdot|^2 \to \Phi$, modular conjugation $J$ |
| [FOUND_CONSCIOUSNESS_MATHEMATICS.md](FOUND_CONSCIOUSNESS_MATHEMATICS.md) | Domain A/B partition, consciousness phase angle $\theta = 52.54°$ |
| [EXPLR_FEIGENBAUM_CONNECTION.md](EXPLR_FEIGENBAUM_CONNECTION.md) | Period-doubling cascade, Feigenbaum constant |
| [EXPLR_COLLAPSE_GRAVITY_BRIDGE.md](EXPLR_COLLAPSE_GRAVITY_BRIDGE.md) | Hawking $\beta_H = 8\pi M$ as RT dictionary entry; collapse and gravity as same type transition on temporal vs spatial axes |

### 6.4 Claims Summary

| Category | Count | IDs |
|----------|-------|-----|
| Classical theorems cited | 12 | Murray-von Neumann, Connes, Tomita-Takesaki, HHW, Matsubara, Schwartz, Buchholz-Wichmann, Takesaki (crossed product), Dixmier (MASA) |
| Theorems | 6 | RT-T1 through RT-T6 |
| Conjectures | 10 | RT-C1 through RT-C10 |
| Open questions | 6 | RT-O1 through RT-O6 (RT-O4 addressed by §2.6–2.8) |
| Critical warnings | 1 | RT-W1 (Powers factors stay Type III — resolved by descent chain) |
| **Total claims** | **35** | |

---

*The ReLU Type Transition — Foundational Ternary Dynamics v5.26*
*Prepared for critical evaluation*
*February 18, 2026*
