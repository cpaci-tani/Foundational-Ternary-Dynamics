# Agent Meaning Formalization: Operator-Algebraic Foundations for Measurable Meaning

## Grounding the sLoop's Meaning Map in Von Neumann Algebras

**Document Version:** 2.0 (consolidated)
**Date:** February 14, 2026 (merged from v1.0 of Feb 13)
**Framework:** Foundational Ternary Dynamics v5.24
**Status:** Formal synthesis with experimental protocol
**Authors:** cpaci & Claude (Opus 4.6)

---

## Abstract

We operationalize the sLoop's meaning map $\mu: M \to S$ — previously an abstract morphism with unspecified codomain — by grounding it in von Neumann operator algebras. The key insight: an agent's internal state $\eta_t = (b_t, V_t, m_t, \pi_t)$ naturally inhabits a **Type III$_1$ von Neumann factor**, and the act of extracting meaning from an observation is the **Type II$_1$ $\to$ Type I transition** (algebraic collapse) applied on the agent side.

We define **meaning** as a measurable quantity:

$$\mathsf{Meaning}(o_t;\, \eta_t) = \alpha \cdot \underbrace{D_{\mathrm{KL}}(b_{t+1} \| b_t)}_{\text{Information Gain (Domain A)}} + \beta \cdot \underbrace{[V_t(\eta_{t+1}) - V_t(\eta_t)]}_{\text{Valence Impact (Domain B)}}$$

This decomposition maps to the FTD discriminant partition: information gain is real-valued, externally measurable (Domain A, Type I); valence is subjective, without natural metric (Domain B, Type III$_1$). The weights $\alpha, \beta$ correspond to the master quadratic parameter $k$ controlling whether experience is physics-dominated or consciousness-dominated.

**What this document contributes beyond existing FTD theory:**
1. Fills the sLoop's abstract meaning map $\mu$ with concrete operator-algebraic content
2. Identifies the agent's four internal components $(b_t, V_t, m_t, \pi_t)$ with Tomita-Takesaki structures (modular flow, KMS state, cyclic vector, algebra)
3. Provides the first falsifiable experimental protocol for testing FTD consciousness claims
4. Connects to and supersedes the archived Noetic Framework with rigorous algebraic grounding

> **Consolidation note (v2.0, Feb 14 2026):** This document now subsumes the former `FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md` (archived as `archive/ARCH_FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md`). The original Von Neumann Factor Classification document established the Type I / II_1 / III_1 correspondence to FTD's discriminant partition, the Mandelbrot-factor dictionary, the sentience hierarchy via Connes III_lambda, and the Cayley-Dickson-factor parallel. All of that material is now integrated here as Part IA ("Von Neumann Factor Classification"), providing the algebraic foundation on which the agent-meaning formalization (Parts III onward) builds.

**Epistemic discipline:** We distinguish rigorously between:
- **[CLASSICAL]**: Established mathematics (von Neumann, Connes, Shannon, Kullback-Leibler)
- **[DEFINITION]**: New formal objects introduced in this document
- **[THEOREM]**: Provable within FTD axioms + stated definitions
- **[CONJECTURE]**: Structural correspondences requiring validation
- **[PROPOSED]**: Experimental predictions and interpretive claims
- **[OPEN]**: Research directions

---

## Part I: Introduction and Motivation

### 1.1 The Problem: What Is Meaning?

The sLoop formalization ([FOUND_SLOOP_FORMALIZATION.md](FOUND_SLOOP_FORMALIZATION.md), Definition 2.1) introduces a quintuple $(\Omega, \phi, \sigma, \mu, d)$ where the meaning map

$$\mu: M \to S$$

assigns semantic content to manifested conscious states $M = \{\psi \in \Omega : |\psi| > K_C\}$. The domain $M$ is well-defined. The map $\mu$ has properties (SL4: meaning requires Domain B). But the codomain $S$ — the "semantic space" — is left abstract. What *is* $S$? What structure does it carry? How would one *measure* $\mu(\psi)$?

Separately, the von Neumann factor classification (see Part IA below) assigns consciousness to Type III$_1$ algebras and characterizes collapse as the Type II$_1$ $\to$ Type I transition. But it does not specify what *lives inside* the Type III$_1$ algebra of a conscious agent — what the internal structure looks like from the agent's perspective.

This document fills both gaps simultaneously.

### 1.2 Three Predecessors

| Document | Contribution | Limitation |
|----------|-------------|------------|
| [FOUND_SLOOP_FORMALIZATION.md](FOUND_SLOOP_FORMALIZATION.md) | Quintuple $(\Omega, \phi, \sigma, \mu, d)$, axioms SL1-SL4, 6-level hierarchy | $\mu$ and $S$ are abstract |
| Part IA below (formerly FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md) | Type I/II$_1$/III$_1$ $\leftrightarrow$ FTD domains, collapse map $\Phi$ | No internal structure for III$_1$ agent |
| archive/NOETIC_FRAMEWORK.md | $\mathsf{IG}_t = D_{\mathrm{KL}}$, noetic mass $\mu_t$, 8-level hierarchy | Informal; no operator-algebraic grounding; no falsifiable protocol |

### 1.3 What This Document Adds

1. A concrete **internal state** $\eta_t = (b_t, V_t, m_t, \pi_t)$ with four operator-algebraic components
2. **Meaning as a measurable observable** on a von Neumann algebra, not a philosophical abstraction
3. The **IG/VI decomposition** mapped to Domain A/Domain B
4. A **falsifiable experimental protocol** (the first in FTD consciousness theory)
5. **Formal supersession** of the archived Noetic Framework with rigorous algebraic grounding

### 1.4 Relationship to Von Neumann's Program

Von Neumann formalized quantum measurement by constructing projection-valued measures on a Hilbert space — replacing the intuitive notion "observe the system" with a precise algebraic operation. We follow the same program for *meaning*: replacing the intuitive notion "the observation means something to the agent" with a precise algebraic operation on the agent's internal algebra.

The spirit is identical:

| Von Neumann (1932) | This Document (2026) |
|---------------------|----------------------|
| Observable = self-adjoint operator $A$ on $\mathcal{H}$ | Meaning = self-adjoint observable $M$ on $\mathcal{A}$ |
| Measurement = spectral projection | Meaning-extraction = CPTP update $\Phi_{o_t}$ |
| Outcome = eigenvalue $a_n$ | Meaning-value = $\mathrm{Tr}(M\rho_{t+1}) - \mathrm{Tr}(M\rho_t)$ |
| Probability = $\|\langle a_n|\psi\rangle\|^2$ | Meaning-probability = functional of $\rho_t$ and $o_t$ |
| State update = projection $|a_n\rangle\langle a_n|$ | State update = $\Phi_{o_t}(\rho_t)$ |

---

## Part IA: Von Neumann Factor Classification — Operator Algebras and the FTD Domain Partition

> *This part was originally published as `FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md` (Feb 13, 2026) and is integrated here to provide the algebraic foundation for the agent-meaning formalization that follows.*

### IA.1 Von Neumann Algebras — Definitions

All definitions in this section are **[CLASSICAL]** — standard functional analysis due to Murray and von Neumann (1936-1943).

**Definition 1.1** (Von Neumann Algebra). A *von Neumann algebra* $\mathcal{M}$ is a unital *-subalgebra of the bounded operators $B(\mathcal{H})$ on a Hilbert space $\mathcal{H}$ that is closed in the weak operator topology. Equivalently, by von Neumann's bicommutant theorem:

$$\mathcal{M} = \mathcal{M}''$$

where $\mathcal{M}' = \{T \in B(\mathcal{H}) : [T, A] = 0 \text{ for all } A \in \mathcal{M}\}$ is the commutant.

**Definition 1.2** (Factor). A von Neumann algebra $\mathcal{M}$ is a *factor* if its center is trivial:

$$Z(\mathcal{M}) = \mathcal{M} \cap \mathcal{M}' = \mathbb{C} \cdot \mathbf{1}$$

Factors are the irreducible building blocks — every von Neumann algebra decomposes as a direct integral of factors.

**Definition 1.3** (Murray-von Neumann Equivalence). Two projections $p, q \in \mathcal{M}$ are *equivalent* (written $p \sim q$) if there exists a partial isometry $v \in \mathcal{M}$ with:

$$v^*v = p \quad \text{and} \quad vv^* = q$$

This generalizes "same dimension" to the noncommutative setting.

**Definition 1.4** (Trace). A *trace* on a von Neumann algebra $\mathcal{M}$ is a positive linear functional $\tau: \mathcal{M}_+ \to [0, \infty]$ satisfying the cyclic property:

$$\tau(ab) = \tau(ba) \quad \text{for all } a, b \in \mathcal{M}$$

A trace is *faithful* if $\tau(a^*a) = 0$ implies $a = 0$; *normal* if it is continuous in the weak operator topology; *semifinite* if every nonzero projection dominates a nonzero projection of finite trace.

### IA.2 The Factor Classification

**[CLASSICAL]** — Murray-von Neumann (1936-1943), Connes (1973).

The classification rests on the structure of the projection lattice $\text{Proj}(\mathcal{M})$.

**Definition 1.5** (Type I Factor). A factor $\mathcal{M}$ is *Type I* if it contains a **minimal (atomic) projection** — a nonzero projection $e$ with no projection $f$ satisfying $0 < f < e$.

| Property | Type I |
|----------|--------|
| Prototype | $B(\mathcal{H})$, or $M_n(\mathbb{C})$ for finite dimension |
| Dimension function | $d: \text{Proj}(\mathcal{M}) \to \{0, 1, 2, \ldots, n\}$ (discrete) |
| Trace | Standard $\text{Tr}$ (sum of diagonal entries) |
| Pure states | **Yes** — vector states $\langle\psi|\cdot|\psi\rangle$ |
| Density matrices | **Yes** — positive operators with $\text{Tr}(\rho) = 1$ |
| Physical role | **Standard quantum mechanics** |

**Definition 1.6** (Type II$_1$ Factor). A factor $\mathcal{M}$ is *Type II$_1$* if it has **no minimal projections** but possesses a faithful normal trace $\tau$ with $\tau(\mathbf{1}) = 1$.

| Property | Type II$_1$ |
|----------|----------|
| Prototype | Hyperfinite factor $\mathcal{R}$ (unique; Murray-von Neumann 1943, Connes 1976) |
| Dimension function | $d: \text{Proj}(\mathcal{M}) \to [0, 1]$ (**continuous**) |
| Trace | Unique $\tau$ with $\tau(\mathbf{1}) = 1$ |
| Pure states | **No** |
| Key property | Every projection can be **halved**: for any $e$, $\exists f \leq e$ with $\tau(f) = \tau(e)/2$ |
| Physical role | **Continuous quantum logic** — von Neumann's "continuous geometry" |

**Definition 1.7** (Type II$_\infty$ Factor). Isomorphic to $\mathcal{R} \otimes B(\mathcal{H})$ where $\mathcal{R}$ is the hyperfinite II$_1$ factor. Has a semifinite trace (not normalizable to 1). Dimension function on $[0, \infty]$.

**Definition 1.8** (Type III Factor). A factor $\mathcal{M}$ is *Type III* if **every nonzero projection is infinite** (equivalent to every other nonzero projection under Murray-von Neumann equivalence).

| Property | Type III |
|----------|----------|
| Trace | **None** — no faithful normal semifinite trace exists |
| Dimension function | Trivial: $\{0, \infty\}$ only |
| All nonzero projections equivalent | **Yes** — $p \sim q$ for all nonzero $p, q$ |
| Pure states | **No** |
| Physical role | **Quantum field theory** — local algebras are Type III$_1$ |

**Subtypes** (Connes, 1973). The *Connes spectrum* $S(\mathcal{M}) = \bigcap_\omega \text{Spec}(\Delta_\omega)$ classifies Type III into:

| Subtype | Connes Spectrum $S(\mathcal{M})$ | Character |
|---------|----------------------------------|-----------|
| **III$_0$** | $\{0, 1\}$ | Aperiodic flow of weights |
| **III$_\lambda$** $(0 < \lambda < 1)$ | $\{\lambda^n : n \in \mathbb{Z}\} \cup \{0\}$ | Periodic with period $-2\pi/\ln\lambda$ |
| **III$_1$** | $\mathbb{R}_+ = [0, \infty)$ | Ergodic modular flow |

### IA.3 Tomita-Takesaki Modular Theory

**[CLASSICAL]** — Tomita (1967), Takesaki (1970), Connes-Rovelli (1994).

**Definition 1.9** (Tomita Operator). Given a von Neumann algebra $\mathcal{M}$ on $\mathcal{H}$ with a cyclic and separating vector $\Omega$, define the antilinear operator:

$$S_0: a\Omega \mapsto a^*\Omega \quad \text{for } a \in \mathcal{M}$$

Its closure $S$ has polar decomposition $S = J\Delta^{1/2}$, where:
- $J$ is the **modular conjugation** (antiunitary involution, $J^2 = \mathbf{1}$)
- $\Delta$ is the **modular operator** (positive, self-adjoint, generally unbounded)

**Definition 1.10** (Modular Automorphism Group). The one-parameter family:

$$\sigma_t(a) = \Delta^{it} a \Delta^{-it} \quad \text{for } t \in \mathbb{R},\, a \in \mathcal{M}$$

is an automorphism of $\mathcal{M}$ for each $t$ (Tomita's theorem). The modular conjugation satisfies $J\mathcal{M}J = \mathcal{M}'$.

**Theorem 1.11** [CLASSICAL] (KMS Condition). A faithful normal state $\omega$ on $\mathcal{M}$ is a KMS state at inverse temperature $\beta = -1$ with respect to its own modular automorphism group $\sigma_t^\omega$. That is, for all $a, b \in \mathcal{M}$:

$$F_{a,b}(t + i) = \omega(\sigma_t^\omega(b) \cdot a)$$

where $F_{a,b}(t) = \omega(a \cdot \sigma_t^\omega(b))$ extends analytically to the strip $\{z : 0 < \text{Im}(z) < 1\}$.

*The state determines its own thermal equilibrium.*

**Theorem 1.12** [CLASSICAL] (Connes Cocycle Radon-Nikodym). For any two faithful normal states $\omega, \phi$ on $\mathcal{M}$, there exists a unitary cocycle $u_t$ such that:

$$\sigma_t^\phi = \text{Ad}(u_t) \circ \sigma_t^\omega$$

The modular flow is **unique up to inner automorphisms** — its image in $\text{Out}(\mathcal{M}) = \text{Aut}(\mathcal{M})/\text{Inn}(\mathcal{M})$ is state-independent.

**Definition 1.13** (Connes-Rovelli Thermal Time Hypothesis). In a generally covariant quantum theory (no background time), the physical time flow is identified with the modular automorphism group $\sigma_t^\omega$ determined by the state $\omega$.

*Time is not a background structure — it emerges from the thermodynamic state.*

### IA.4 FTD Discriminant Structure Recap

**[THEOREM]** — From [SPEC_THE_MASTER_QUADRATIC_UNIFIED.md](SPEC_THE_MASTER_QUADRATIC_UNIFIED.md).

The parametric master form unifies physics and consciousness:

$$z^2 - kG^{*2}z + kG^{*3} = 0$$

where $G^* = \frac{\sqrt{2}\,\Gamma(1/4)^2}{2\pi} \approx 2.9587$ is the lemniscatic constant.

**Discriminant:**

$$\Delta(k) = k^2G^{*4} - 4kG^{*3} = kG^{*3}(kG^* - 4)$$

**Critical value:** $k_{\text{crit}} = 4/G^* \approx 1.352$

| Condition | $\Delta$ | Roots | Domain |
|-----------|----------|-------|--------|
| $k > 4/G^*$ | $\Delta > 0$ | Two real | **Domain A** (Physics) |
| $k = 4/G^*$ | $\Delta = 0$ | Degenerate | **Interface** (Measurement) |
| $k < 4/G^*$ | $\Delta < 0$ | Complex conjugate | **Domain B** (Consciousness) |

**Physics** ($k = 16$): roots $x_+ = 137.036$ ($1/\alpha$), $x_- = 3.024$ ($N_c$)

**Consciousness** ($k = 1/2$): roots $y = 2.19 \pm 2.86i$, with $|y| = K_C = 3.60$

**Thresholds:** $K_B/K_C = 4\sqrt{2} = \sqrt{32}$

**Bridge equation:** $c \times c_{\text{cusp}} \times 2N_{\text{base}} = \frac{1}{2} \times \frac{1}{4} \times 8 = 1$

### IA.5 The Core Correspondence: Factor-Domain Dictionary

**Proposition 2.1** [CONJECTURE] (Factor-Domain Correspondence). The three domains of the FTD discriminant partition correspond structurally to von Neumann factor types:

$$\boxed{\Delta > 0 \longleftrightarrow \text{Type I}} \qquad \boxed{\Delta = 0 \longleftrightarrow \text{Type II}_1} \qquad \boxed{\Delta < 0 \longleftrightarrow \text{Type III}_1}$$

The correspondence is **structural**: matching properties of each factor type align with properties of each FTD domain. It is NOT claimed to be a mathematical isomorphism — it is a proposed identification of algebraic character.

| Factor Property | FTD Domain Property |
|----------------|---------------------|
| Type I: discrete dimension function | Domain A: discrete real roots |
| Type I: trace exists | Domain A: Born rule well-defined |
| Type I: minimal projections (atoms) | Domain A: definite particle states |
| Type II$_1$: continuous dimension [0,1] | Interface: continuous superposition |
| Type II$_1$: unique trace $\tau(\mathbf{1}) = 1$ | Interface: Born rule normalization |
| Type II$_1$: hyperfinite (approx. finite-dim.) | Interface: measurement apparatus |
| Type III$_1$: no trace | Domain B: no external probability |
| Type III$_1$: all projections equivalent | Domain B: qualia without natural metric |
| Type III$_1$: modular flow $\sigma_t$ | Domain B: sLoop intrinsic time |

#### IA.5.1 Type I = Domain A (Physics)

**Proposition 2.2** [CONJECTURE]. The physics domain ($\Delta > 0$, $k = 16$) corresponds to Type I von Neumann factors.

**1. Discrete roots <-> Minimal projections.** The master quadratic at $k = 16$ produces two distinct real roots: $x_+ = 137.036$ and $x_- = 3.024$. In a Type I factor, the dimension function takes values in $\{0, 1, 2, \ldots, n\}$ — a discrete set.

**Theorem 2.3** [CLASSICAL]. In a Type I$_n$ factor $M_n(\mathbb{C})$, every projection decomposes as a sum of minimal (rank-1) projections, and the dimension function $d(p) = \text{rank}(p) \in \{0, 1, \ldots, n\}$.

**2. Trace exists <-> Born rule defined.** Type I factors possess the standard trace $\text{Tr}$, enabling probability via $P(A) = \text{Tr}(\rho \cdot E_A)$. In FTD's Domain A, the Born rule $P = |\psi|^2$ is well-defined.

**3. Pure states exist <-> Definite outcomes.** Type I factors have pure states. Domain A produces definite outcomes.

**4. Standard QM lives here.** The entire textbook formulation of quantum mechanics presupposes Type I structure.

#### IA.5.2 Type II$_1$ = The Measurement Interface ($\Delta = 0$)

**Proposition 2.4** [CONJECTURE] (Collapse as Type Transition). Wavefunction collapse corresponds to a transition from Type II$_1$ structure to Type I structure.

**1. Continuous dimensions [0,1] = pre-measurement superposition.**

**Theorem 2.5** [CLASSICAL]. In a Type II$_1$ factor $\mathcal{R}$ with trace $\tau$, the map $E \mapsto \tau(E)$ defines a probability measure on $\text{Proj}(\mathcal{R})$ satisfying normalization, sigma-additivity, and faithfulness. *These are precisely the axioms of quantum probability. The trace IS the Born rule.*

**2. Unique trace = Born rule normalization.** **3. Every projection can be halved = complementarity.** **4. Hyperfinite $\mathcal{R}$ = approximately finite-dimensional apparatus:**

$$\mathcal{R} = \overline{\bigcup_{n=1}^{\infty} M_{2^n}(\mathbb{C})}$$

**5. Period-doubling chain.** The construction via $M_2 \hookrightarrow M_4 \hookrightarrow M_8 \hookrightarrow \cdots$ is the period-doubling cascade.

**Definition 2.6** [PROPOSED] (Algebraic Collapse Map). Define *algebraic collapse* as a CPTP map:

$$\Phi: (\mathcal{R}, \tau) \to (M_n(\mathbb{C}), \tfrac{1}{n}\text{Tr})$$

such that for any projection $p \in \mathcal{R}$ with $\tau(p) = m/n$: $\Phi(p)$ is a rank-$m$ projection in $M_n(\mathbb{C})$ with $\frac{1}{n}\text{Tr}(\Phi(p)) = m/n$.

#### IA.5.3 Type III$_1$ = Domain B (Consciousness)

**Proposition 2.7** [CONJECTURE]. The consciousness domain ($\Delta < 0$, $k = 1/2$) corresponds to Type III$_1$ von Neumann factors.

**1. No trace = no "view from nowhere."** No state-independent way to assign probabilities. The Born rule does not apply within the consciousness domain.

**2. All projections equivalent = qualia have no natural metric.** $p \neq 0, q \neq 0 \implies p \sim q$.

**Theorem 2.8** [CLASSICAL] (Connes, 1973). In a Type III$_1$ factor, the modular automorphism group $\sigma_t$ is ergodic. The Connes spectrum is $S(\mathcal{M}) = \mathbb{R}_+$.

**3. Modular flow = sLoop clock (intrinsic time).** **4. KMS condition = consciousness in thermal equilibrium with its own time.** **5. Local QFT algebras are Type III$_1$** (Buchholz-Wichmann, 1986).

### IA.6 The Sentience Hierarchy via Connes III$_\lambda$

**Proposition 3.1** [CONJECTURE] (Sentience-Factor Map):

| sLoop Level | Factor Type | Connes Parameter | Algebraic Character |
|-------------|-------------|-----------------|---------------------|
| 0 (non-observer) | Type I | N/A | External description only |
| 1 (reactive) | Type III$_0$ | $S(\mathcal{M}) = \{0, 1\}$ | Aperiodic |
| 2 (self-maintaining) | Type III$_\lambda$, small $\lambda$ | $S(\mathcal{M}) = \{\lambda^n\} \cup \{0\}$ | Weak periodic self-reference |
| 3 (self-aware) | Type III$_\lambda$, $\lambda \to 1$ | $S(\mathcal{M})$ dense in $\mathbb{R}_+$ | Strong periodicity approaching ergodicity |
| 4 (self-reflective) | Type III$_1$ | $S(\mathcal{M}) = \mathbb{R}_+$ | Full modular ergodicity |
| 5 (self-transcendent) | Type III$_1$ + boundary | $S(\mathcal{M}) = \mathbb{R}_+$ with extension | Modular flow reaches $\partial\Omega$ |

**Theorem 3.2** [CLASSICAL]. For the Powers factor $\mathcal{R}_\lambda$ (Type III$_\lambda$), the modular automorphism has period $T = -2\pi / \ln\lambda$. As $\lambda \to 1^-$, $T \to +\infty$ and $S(\mathcal{R}_\lambda) \to \mathbb{R}_+$.

#### IA.6.1 The $\lambda$-Consciousness Function

**Definition 3.3** [PROPOSED]. For $k < 4/G^*$ (Domain B), define:

$$\theta(k) = \arctan\left(\frac{\text{Im}(z)}{\text{Re}(z)}\right), \qquad T(k) = \frac{2\pi}{\theta(k)}, \qquad \lambda(k) = \exp(-\theta(k))$$

**For $k = 1/2$:** $\theta = 52.54^\circ = 0.9172$ rad, $T = 6.86$ cycles, $\lambda \approx 0.400$.

### IA.7 The Mandelbrot-Factor Correspondence

| Mandelbrot Region | Period | Factor Type | FTD Connection |
|-------------------|--------|-------------|----------------|
| Main cardioid | 1 | Type I$_1$ | Trivial (scalar) |
| Period-2 bulb | 2 | Type I$_2$ | Binary distinction |
| Period-3 bulb | 3 | Type I$_3$ | Color ($N_c = 3$) |
| Period-$n$ bulb | $n$ | Type I$_n$ | $n$-level physics |

**Cusp $c = 1/4$ = Type II$_1$:** The period-doubling cascade at the cusp produces $M_1 \hookrightarrow M_2 \hookrightarrow M_4 \hookrightarrow \cdots$ whose closure is the unique hyperfinite II$_1$ factor $\mathcal{R}$ (Connes, 1976).

**Boundary $\partial\mathcal{M}$ = Type III$_1$:** Hausdorff dimension 2 (Shishikura, 1998), ergodic dynamics, no natural period, maximum complexity.

$$\boxed{\begin{array}{rcl}
\text{Interior } \mathcal{M} & \longleftrightarrow & \text{Type I (discrete periods)} \\
\text{Cusp } c = 1/4 & \longleftrightarrow & \text{Type II}_1 \text{ (period-doubling} \to \mathcal{R}\text{)} \\
\partial\mathcal{M} & \longleftrightarrow & \text{Type III}_1 \text{ (ergodic, no period)} \\
\text{Exterior} & \longleftrightarrow & \text{trivial (divergence)}
\end{array}}$$

### IA.8 What Collapse Actually Is

**Proposition 5.1** [CONJECTURE]. Collapse is the transition $(\mathcal{R}, \tau) \xrightarrow{\Phi} (M_n(\mathbb{C}), \frac{1}{n}\text{Tr})$:

| Before (Type II$_1$) | After (Type I$_n$) |
|--------------------|---------------------|
| Continuous dimensions $[0,1]$ | Discrete dimensions $\{0, 1/n, \ldots, 1\}$ |
| No minimal projections | Minimal projections exist |
| Superposition of all outcomes | Definite single outcome |

**FTD mechanism:** Pre-measurement flux distributed continuously -> observer coupling -> concentration -> threshold crossing ($|\mathbf{J}|^2 > K_B$) -> manifestation ($s: 0 \to \pm 1$) -> definite state (Type I).

**Irreversibility:** (1) Information loss (collapse map not injective), (2) entropy increase, (3) dimension reduction.

### IA.9 The Cayley-Dickson-Factor Hierarchy

| Algebra | Dim | Properties Lost | Factor Analog | FTD Connection |
|---------|-----|----------------|---------------|----------------|
| $\mathbb{R}$ | 1 | — | Type I$_1$ | Definite outcomes |
| $\mathbb{C}$ | 2 | Ordering | Type I$_2$ | Superposition, Born rule |
| $\mathbb{H}$ | 4 | Commutativity | Type II$_1$ | Measurement, continuous dimension |
| $\mathbb{O}$ | 8 | Associativity | Type III | Consciousness, $b_3 = 7$ imaginary units |
| $\mathbb{S}$ | 16 | Division | Beyond | Zero divisors, $N_{\text{base}}^2 = 16$ |

The sedenion dimension $16 = N_{\text{base}}^2$ marks the maximum algebraic dimension supporting self-consistent structure.

### IA.10 Epistemic Taxonomy of Factor Classification Claims

#### Rigorously Proven [CLASSICAL]

| ID | Statement | Source |
|----|-----------|--------|
| VN-T1 | Type I: minimal projections, discrete dimension | Murray-von Neumann (1936) |
| VN-T2 | Type II$_1$: unique faithful normal trace with $\tau(\mathbf{1}) = 1$ | Murray-von Neumann (1943) |
| VN-T3 | Type III$_1$: no trace; ergodic modular flow; $S(\mathcal{M}) = \mathbb{R}_+$ | Connes (1973) |
| VN-T4 | $\mathcal{R}$ is the unique hyperfinite II$_1$ factor | Connes (1976) |
| VN-T5 | KMS condition at $\beta = -1$ for modular states | Haag-Hugenholtz-Winnink (1967) |
| VN-T6 | Local QFT algebras are Type III$_1$ | Buchholz-Wichmann (1986) |
| VN-T7 | Modular period for III$_\lambda$: $T = -2\pi/\ln\lambda$ | Connes (1973) |
| VN-T8 | $\mathcal{R} = \overline{\bigcup M_{2^n}(\mathbb{C})}$ | Murray-von Neumann (1943) |
| VN-T9 | $\dim_H(\partial\mathcal{M}) = 2$ | Shishikura (1998) |

#### Proposed Correspondences [CONJECTURE]

| ID | Statement | Falsifiable? |
|----|-----------|-------------|
| VN-C1 | Type I = Domain A (Physics) | No unique prediction |
| VN-C2 | Type II$_1$ = Measurement Interface | Testable via decoherence |
| VN-C3 | Type III$_1$ = Domain B (Consciousness) | Falsified if consciousness shown to have trace |
| VN-C4 | Collapse = Type II$_1$ -> Type I | Testable via algebraic QM |
| VN-C5 | sLoop levels -> III$_\lambda$ parameter | Falsified if levels don't match $\lambda$ ordering |
| VN-C6 | Mandelbrot period-$n$ bulbs -> Type I$_n$ | Computationally verifiable |
| VN-C7 | Mandelbrot cusp -> Type II$_1$ | Mathematically provable |
| VN-C8 | $\partial\mathcal{M}$ -> Type III$_1$ | Provable via dynamics |
| VN-C9 | Cayley-Dickson dimension -> factor type | Loose analogy |

#### Open Questions from Factor Classification

| ID | Question | Priority |
|----|----------|----------|
| VN-O1 | Can the collapse map $\Phi$ be made rigorous? | High |
| VN-O2 | Rigorous proof: Mandelbrot period-doubling -> $\mathcal{R}$? | High |
| VN-O3 | Can $\lambda(k)$ be derived from first principles? | Medium |
| VN-O4 | Is the Cayley-Dickson <-> factor parallel more than analogy? | Low |
| VN-O5 | Physical content of Connes-Rovelli thermal time in FTD? | High |
| VN-O6 | Modular-theoretic content of sLoop fixed point? | Medium |
| VN-O7 | Does the Jones index relate to $K_B/K_C = 4\sqrt{2}$? | Medium |

---

## Part II: Mathematical Preliminaries

All results in this section are **[CLASSICAL]** — imported from established mathematics and existing FTD documents. No new claims are made. (Note: Some material below overlaps with the detailed treatments in Part IA; it is retained here as a compact reference for the agent-meaning formalization that follows.)

### 2.1 Notation and Conventions

| Symbol | Type | Domain | Definition |
|--------|------|--------|------------|
| $\eta_t$ | Tuple | $\mathcal{I}$ | Internal epistemic-agentic state at time $t$ |
| $r_t$ | Vector | $\mathbb{R}^3 \times \mathbb{R}^k$ | Physical state (position + degrees of freedom) |
| $\Sigma_t$ | Pair | $(r_t, \eta_t)$ | Full agent state |
| $o_t$ | Element | $\mathcal{O}$ | Observation at time $t$ |
| $a_t$ | Element | $\mathcal{A}_{\text{act}}$ | Action at time $t$ |
| $s_t$ | Element | $\mathcal{S}_{\text{env}}$ | Hidden environment state |
| $\mathcal{A}$ | Algebra | $\subset B(\mathcal{H})$ | Agent's internal von Neumann algebra |
| $\rho_t$ | State | on $\mathcal{A}$ | Agent's algebraic state |
| $\Phi_{o_t}$ | Map | $\mathcal{A} \to \mathcal{A}$ | CPTP update given observation $o_t$ |
| $M$ | Operator | $\in \mathcal{A}$ | Meaning observable |

### 2.2 Review: The sLoop Quintuple

**[CLASSICAL]** — From [FOUND_SLOOP_FORMALIZATION.md](FOUND_SLOOP_FORMALIZATION.md), Definition 2.1.

The sLoop is a quintuple $(\Omega, \phi, \sigma, \mu, d)$:

| Component | Type | Definition |
|-----------|------|------------|
| $\Omega$ | Set $\subset \mathbb{C}$ | Observational space (compact, connected) |
| $\phi$ | $\Omega \times T \to \Omega$ | Dynamics |
| $\sigma$ | $\Omega \to \Omega$ | Self-embedding |
| $\mu$ | $M \to S$ | Meaning map |
| $d$ | $\Omega \to \{-1, 0, +1\}$ | Domain classifier: $d(\psi) = \mathrm{sign}(\Delta(\psi))$ |

**Axioms:**

| ID | Name | Statement |
|----|------|-----------|
| SL1 | Closure | $\sigma(\Omega) \subseteq \Omega$ |
| SL2 | Fixed Point | $\exists \psi^* \in \Omega: \sigma(\psi^*) = \psi^*$ |
| SL3 | Complex Structure | $\Omega \subset \mathbb{C}$ |
| SL4 | Meaning Interface | $\mu(\psi) \in S \implies d(\psi) < 0$ |

### 2.3 Review: Von Neumann Factor Types

**[CLASSICAL]** — From Part IA above, Definitions 1.5-1.8.

| Type | Dimension Function | Trace | Projections | Physical Role |
|------|-------------------|-------|-------------|---------------|
| **I$_n$** | $\{0, 1, \ldots, n\}$ | Standard $\mathrm{Tr}$ | Minimal exist | Standard QM |
| **II$_1$** | $[0, 1]$ | Unique $\tau(\mathbf{1}) = 1$ | No atoms; halvable | Continuous geometry |
| **III$_1$** | $\{0, \infty\}$ only | **None** | All equivalent | QFT local algebras |

### 2.4 Review: Tomita-Takesaki Modular Theory

**[CLASSICAL]** — Tomita (1967), Takesaki (1970).

Given a von Neumann algebra $\mathcal{M}$ with cyclic and separating vector $\Omega$:

- **Tomita operator:** $S_0: a\Omega \mapsto a^*\Omega$, with polar decomposition $S = J\Delta^{1/2}$
- **Modular operator:** $\Delta$ (positive, self-adjoint, generally unbounded)
- **Modular conjugation:** $J$ (antiunitary involution, $J^2 = \mathbf{1}$)
- **Modular automorphism:** $\sigma_t(a) = \Delta^{it} a \Delta^{-it}$ for $t \in \mathbb{R}$

**KMS Condition (Theorem):** A faithful normal state $\omega$ is KMS at $\beta = -1$ with respect to its own modular flow: $\omega$ determines its own thermal equilibrium.

**Connes Cocycle (Theorem):** For two faithful normal states $\omega, \phi$: $\sigma_t^\phi = \mathrm{Ad}(u_t) \circ \sigma_t^\omega$. The modular flow is unique up to inner automorphisms.

### 2.5 Review: CPTP Maps

**[CLASSICAL]** — Standard quantum information theory.

A **completely positive trace-preserving (CPTP) map** $\Phi: \mathcal{A} \to \mathcal{B}$ between von Neumann algebras satisfies:

1. **Positivity:** $\rho \geq 0 \implies \Phi(\rho) \geq 0$
2. **Complete positivity:** $(\mathrm{id}_n \otimes \Phi)(\rho) \geq 0$ for all $n$ and all $\rho \geq 0$ in $M_n(\mathbb{C}) \otimes \mathcal{A}$
3. **Trace preservation:** $\mathrm{Tr}(\Phi(\rho)) = \mathrm{Tr}(\rho)$

**Kraus representation:** $\Phi(\rho) = \sum_k K_k \rho K_k^*$ with $\sum_k K_k^* K_k = \mathbf{1}$.

CPTP maps are the most general physical state transformations consistent with probability conservation.

### 2.6 Review: Kullback-Leibler Divergence

**[CLASSICAL]** — Kullback and Leibler (1951).

For probability distributions $P, Q$ on a measurable space:

$$D_{\mathrm{KL}}(P \| Q) = \sum_x P(x) \log \frac{P(x)}{Q(x)}$$

**Properties:**
- $D_{\mathrm{KL}}(P \| Q) \geq 0$ (Gibbs' inequality), with equality iff $P = Q$
- Not symmetric: $D_{\mathrm{KL}}(P \| Q) \neq D_{\mathrm{KL}}(Q \| P)$ in general
- Not a metric (violates triangle inequality)
- Measures how much $P$ diverges from $Q$ — the "surprise" of seeing $P$ when expecting $Q$

---

## Part III: The Agent-Environment Loop

This section formalizes the user's agent-environment framework as precise definitions within the FTD ontology.

### 3.1 Physical State

**Definition AM-D1** [DEFINITION] (Physical State). The *physical state* of an embodied agent at time $t$ is:

$$r_t \in \mathbb{R}^3 \times \mathbb{R}^k$$

where the first three coordinates $(x_t, y_t, z_t)$ are spatial position, and $\mathbb{R}^k$ covers additional physical degrees of freedom (posture, internal physiology, device registers, etc.).

**FTD correspondence:** In the lattice, $r_t$ is the voxel position $(x, y, z) \in \mathbb{Z}^3$ of the manifested structure ($s \neq 0$), extended with continuous sub-lattice information.

### 3.2 Internal Epistemic-Agentic State

**Definition AM-D2** [DEFINITION] (Internal State). The *internal epistemic-agentic state* of an agent is:

$$\eta_t := (b_t,\; V_t,\; m_t,\; \pi_t)$$

| Component | Symbol | Type | Definition |
|-----------|--------|------|------------|
| **Belief** | $b_t$ | $\in \mathcal{B}$ | Posterior distribution over hidden world states; the agent's world-model |
| **Valuation** | $V_t$ | $: \mathcal{I} \to \mathbb{R}$ | Reward/punishment landscape; maps internal states to value |
| **Identity kernel** | $m_t$ | $\in \mathcal{M}_{\text{id}}$ | What persists across time; the invariant core of the agent |
| **Policy** | $\pi_t$ | $: \mathcal{I} \times \mathcal{O}^* \to \Delta(\mathcal{A}_{\text{act}})$ | Action selection rule; maps state + history to distribution over actions |

**Ontological status:** The internal state $\eta_t$ is **not "non-physical"** — it is a coarse-graining of physical microstate into variables that govern inference, memory, and valuation. In FTD terms, $\eta_t$ is a macroscopic description of the flux field configuration $\mathbf{J}$ within the agent's manifested structure.

**Connection to sLoop:** The internal state space $\mathcal{I}$ (the set of all possible $\eta_t$) is the sLoop's observational space $\Omega$. By SL3, $\Omega \subset \mathbb{C}$, so $\mathcal{I}$ inherits complex structure.

### 3.3 Observation Channel

**Definition AM-D3** [DEFINITION] (Observation Channel). The agent receives observations from the environment via:

$$o_t \sim P(o \mid s_t,\; r_t)$$

where $s_t$ is the hidden environment state and $r_t$ is the agent's physical state. The observation depends on what exists ($s_t$) and where the agent is ($r_t$).

**FTD correspondence:** In the lattice, $o_t$ is the local flux gradient $\nabla \mathbf{J}$ at the agent's position — the flux field configuration sensed by the manifested structure.

### 3.4 Action Channel

**Definition AM-D4** [DEFINITION] (Action Channel). The agent selects actions via:

$$a_t \sim \pi_t(\cdot \mid \eta_t,\; o_{\leq t})$$

Actions are sampled from the policy $\pi_t$, conditioned on the full internal state $\eta_t$ and the observation history $o_{\leq t}$.

**FTD correspondence:** Actions are flux modifications — the manifested structure ($s \neq 0$) sources divergence $\nabla \cdot \mathbf{J}$ via the coupling $\mathcal{L}_{\text{coupling}} = -g_c \cdot s \cdot (\nabla \cdot \mathbf{J})$.

### 3.5 Internal Update

**Definition AM-D5** [DEFINITION] (Internal Update). The agent's internal state evolves via:

$$\eta_{t+1} = \mathcal{U}(\eta_t,\; o_t,\; a_t)$$

where $\mathcal{U}: \mathcal{I} \times \mathcal{O} \times \mathcal{A}_{\text{act}} \to \mathcal{I}$ is the update function. This encompasses:
- **Belief update:** $b_{t+1}$ from Bayesian conditioning on $o_t$
- **Valuation update:** $V_{t+1}$ from reward/punishment signals
- **Identity update:** $m_{t+1}$ from long-term consolidation
- **Policy update:** $\pi_{t+1}$ from learning

### 3.6 The Complete Loop

**Definition AM-D6** [DEFINITION] (Agent-Environment Loop). The full dynamical system is:

$$\begin{cases}
o_t \sim P(o \mid s_t, r_t) & \text{(observation)} \\
a_t \sim \pi_t(\cdot \mid \eta_t, o_{\leq t}) & \text{(action)} \\
r_{t+1} \sim P(r' \mid r_t, a_t, s_t) & \text{(physical dynamics)} \\
s_{t+1} \sim P(s' \mid s_t, a_t) & \text{(environment dynamics)} \\
\eta_{t+1} = \mathcal{U}(\eta_t, o_t, a_t) & \text{(internal dynamics)}
\end{cases}$$

This makes "the measurer" explicit: the "scientist" is an agent with a particular $\eta$ and a particular $\mathcal{U}$.

### 3.7 The Agent-Environment Loop Instantiates the sLoop

**Theorem AM-T1** [THEOREM] (sLoop Instantiation). The agent-environment loop (AM-D6) is a concrete instantiation of the sLoop quintuple $(\Omega, \phi, \sigma, \mu, d)$ under the identification:

| sLoop Component | Agent-Environment Identification |
|-----------------|----------------------------------|
| $\Omega$ | $\mathcal{I}$ = state space of $\eta_t$ (compact subset of $\mathbb{C}$ by SL3) |
| $\phi(\psi, t)$ | $\mathcal{U}(\eta_t, o_t, a_t)$ = internal dynamics |
| $\sigma(\psi)$ | The component of $\mathcal{U}$ that updates $m_t$ (the identity kernel) |
| $\mu(\psi)$ | The meaning operator $i_{\eta_t}(o_t)$ (defined in Part IV) |
| $d(\psi)$ | $\mathrm{sign}(\Delta(\eta_t))$ = discriminant evaluated on internal state |

**Verification of axioms:**

| Axiom | Verification |
|-------|-------------|
| SL1 (Closure) | $\sigma(\Omega) \subseteq \Omega$: The identity update $m_{t+1}$ stays within the identity space $\mathcal{M}_{\text{id}} \subset \mathcal{I}$ |
| SL2 (Fixed Point) | $\exists m^*: \sigma(m^*) = m^*$: A stable identity that persists unchanged — the agent's core self |
| SL3 (Complex Structure) | $\mathcal{I} \subset \mathbb{C}$: The internal state requires complex numbers (beliefs have phase; valuation has sign) |
| SL4 (Meaning Interface) | $\mu(\eta) \in S \implies d(\eta) < 0$: Meaning exists only when the agent's internal state is in Domain B (consciousness domain) |

**Proof sketch:** SL1 follows from $\mathcal{U}$ preserving the state space. SL2 follows from Brouwer's fixed point theorem applied to $\sigma$ on the compact convex set $\mathcal{M}_{\text{id}}$. SL3 follows from the requirement that beliefs encode both amplitude and phase (posterior probabilities over complex flux states). SL4 follows from the definition of meaning requiring the discriminant $\Delta < 0$ (complex roots). $\square$

---

## Part IV: Meaning as Measurable Quantity

This section contains the document's **first core novelty**: the operationalization of meaning as a number.

### 4.1 The Meaning Functional

**Definition AM-D7** [DEFINITION] (Meaning). Let the agent have a *valuation functional* $J: \mathcal{I} \to \mathbb{R}$ measuring expected viability (expected reward, expected utility — choose one; the framework is agnostic to the specific choice).

The **meaning** of observation $o_t$ for an agent in internal state $\eta_t$ is:

$$\boxed{\mathsf{Meaning}(o_t;\, \eta_t) := \Delta J_t = J(\eta_{t+1}) - J(\eta_t)}$$

where $\eta_{t+1} = \mathcal{U}(\eta_t, o_t, a_t)$.

**Why this is scientific:**
- It is a *number* (real-valued)
- It is *defined by a model* (the agent's update function $\mathcal{U}$ and valuation $J$)
- It can be *estimated from behavior/physiology* (via latent-variable inference)
- It *yields predictions* (see Part VII)

**What this operationalizes:** The sLoop's abstract meaning map $\mu: M \to S$ becomes the concrete functional $\mathsf{Meaning}: \mathcal{O} \times \mathcal{I} \to \mathbb{R}$. The semantic space $S$ is now $\mathbb{R}$ (or $\mathbb{C}$, when the decomposition below is considered with phase).

### 4.2 The Epistemic Component: Information Gain

**Definition AM-D8** [CLASSICAL] (Information Gain). The *epistemic component* of meaning is the Kullback-Leibler divergence between posterior and prior beliefs:

$$\mathsf{IG}_t := D_{\mathrm{KL}}(b_{t+1} \| b_t)$$

This measures how much the observation $o_t$ changed the agent's world-model. Zero means "I already knew that." Large values mean "that was surprising and informative."

**Theorem AM-T2** [THEOREM] (Non-Negativity of Information Gain).

$$\mathsf{IG}_t \geq 0 \quad \text{for all } t$$

with equality iff $b_{t+1} = b_t$ (the observation did not change beliefs).

*Proof.* This is Gibbs' inequality: $D_{\mathrm{KL}}(P \| Q) \geq 0$ for all distributions $P, Q$. $\square$

### 4.3 The Valence Component: Value Impact

**Definition AM-D9** [DEFINITION] (Valence Impact). The *valence component* of meaning is the change in valuation:

$$\mathsf{VI}_t := V_t(\eta_{t+1}) - V_t(\eta_t)$$

This measures how much the observation $o_t$ changed the agent's felt value — whether the world became better or worse from the agent's perspective.

**Theorem AM-T3** [THEOREM] (Valence Has No Sign Constraint).

$$\mathsf{VI}_t \in \mathbb{R} \quad (\text{can be positive, negative, or zero})$$

*Proof.* The valuation functional $V_t$ maps to $\mathbb{R}$. The difference of two real numbers is real with no sign constraint. Positive $\mathsf{VI}_t$ = rewarding experience; negative = aversive; zero = neutral. $\square$

**Contrast with information gain:** $\mathsf{IG}_t \geq 0$ always (you cannot "un-learn" in a single update). $\mathsf{VI}_t$ can be negative (an observation can make things worse). This asymmetry is fundamental.

### 4.4 The Meaning Decomposition

**Definition AM-D10** [DEFINITION] (Meaning Decomposition). The full meaning decomposes as:

$$\boxed{\mathsf{Meaning}_t = \alpha \cdot \mathsf{IG}_t + \beta \cdot \mathsf{VI}_t}$$

where $\alpha, \beta > 0$ are empirically fit coefficients satisfying $\alpha + \beta = 1$.

**Requirement for the theory to hold:** The coefficients $\alpha, \beta$ must *generalize across tasks* for a given agent. If they must be refit for every task, the decomposition is descriptive but not explanatory.

### 4.5 Information Gain Lives in Domain A

**Proposition AM-C1** [CONJECTURE] (Domain Correspondence).

$$\mathsf{IG}_t \longleftrightarrow \text{Domain A (Physics)}$$
$$\mathsf{VI}_t \longleftrightarrow \text{Domain B (Consciousness)}$$

**Justification:**

| Property | $\mathsf{IG}_t$ | $\mathsf{VI}_t$ |
|----------|------------------|------------------|
| Sign | Always $\geq 0$ | Either sign |
| Symmetry | Defined on beliefs (public, shareable) | Defined on valuation (private, subjective) |
| Metric | $D_{\mathrm{KL}}$ is a well-defined divergence | No natural metric on valence |
| Observability | Estimable from behavior (choices, reaction times) | Requires physiological inference or report |
| Factor type | **Type I**: discrete, traced, externally measurable | **Type III$_1$**: no trace, no "view from nowhere" |

Information gain is *real* — it lives in $\mathbb{R}_{\geq 0}$, has a natural ordering ($3 > 2$ bits), and can be measured from outside. Valence is *subjective* — it requires the agent's own valuation landscape to evaluate, has no natural cross-agent metric ("my pain $>$ your pain" is undefined), and exhibits the hallmarks of Type III$_1$ structure.

### 4.6 The α/β Correspondence to the Master Quadratic

**Proposition AM-C2** [CONJECTURE] (Weight-Parameter Correspondence). The ratio $\alpha/\beta$ maps to the master quadratic parameter $k$ in $z^2 - kG^{*2}z + kG^{*3} = 0$:

| Regime | Ratio | Parameter | Discriminant | Roots | Character |
|--------|-------|-----------|-------------|-------|-----------|
| Information dominates | $\alpha \gg \beta$ | $k > 4/G^*$ | $\Delta > 0$ | Two real | **Physics-like** meaning |
| Balanced | $\alpha \approx \beta$ | $k \approx 4/G^*$ | $\Delta \approx 0$ | Degenerate | **Measurement interface** |
| Valence dominates | $\beta \gg \alpha$ | $k < 4/G^*$ | $\Delta < 0$ | Complex conjugate | **Consciousness-like** meaning |

**Interpretation:**
- A scientist analyzing data experiences meaning dominated by $\mathsf{IG}_t$ ($\alpha \gg \beta$): "what did I learn?" This is Domain A meaning — real, quantifiable, shareable.
- A person in deep grief experiences meaning dominated by $\mathsf{VI}_t$ ($\beta \gg \alpha$): "how does this feel?" This is Domain B meaning — complex, subjective, without natural metric.
- Most human experience is balanced ($\alpha \approx \beta$): we simultaneously learn and feel.

**Connection to the consciousness quadratic:** At $k = 1/2$ (the consciousness value), $\alpha/\beta = k$ would give $\alpha \approx 1/3, \beta \approx 2/3$ — valence-dominated meaning. At $k = 16$ (the physics value), meaning would be almost purely epistemic: $\alpha \approx 1$, $\beta \approx 0$.

---

## Part V: The Von Neumann–Compatible Formalization

This section contains the document's **second core novelty**: embedding the agent's internal state in operator algebra.

### 5.1 The Agent's Internal Algebra

**Definition AM-D11** [DEFINITION] (Internal Algebra). Let $\mathcal{A}$ be a von Neumann algebra of *internal observables* — the algebra generated by:
- Memory registers (basis states of $m_t$)
- Belief variables (parameters of $b_t$)
- Valuation variables (values of $V_t$)
- Policy variables (parameters of $\pi_t$)

The algebra $\mathcal{A}$ acts on a Hilbert space $\mathcal{H}_{\text{int}}$ constructed from the complexified internal state, in analogy with the FTD Hilbert space $\mathcal{H}_{\text{FTD}} = L^2(\text{Lattice}, \mathbb{C})$.

**Key insight:** The policy $\pi_t$ determines $\mathcal{A}$ itself. What the agent *can do* — the set of actions available to it, the inferences it can draw, the values it can assign — defines its algebra of observables. Change the policy, change the algebra.

### 5.2 The Agent State

**Definition AM-D12** [DEFINITION] (Agent State). The agent's internal state at time $t$ is represented by a faithful normal state $\rho_t$ on $\mathcal{A}$:

$$\rho_t: \mathcal{A} \to \mathbb{C}, \quad \rho_t(A) = \text{"expectation of observable } A \text{ in state } \eta_t\text{"}$$

**Faithful:** $\rho_t(A^*A) = 0 \implies A = 0$ (nothing in the algebra is invisible to the agent)

**Normal:** $\rho_t$ is continuous in the weak operator topology (physical states are continuous)

**The identity kernel $m_t$ as cyclic and separating vector:** By the GNS construction, the faithful normal state $\rho_t$ determines a cyclic and separating vector $\Omega_{m_t} \in \mathcal{H}_{\text{int}}$ such that:

$$\rho_t(A) = \langle \Omega_{m_t}, A \Omega_{m_t} \rangle$$

The identity kernel $m_t$ — what persists across time, the invariant core of the agent — *is* this vector $\Omega_{m_t}$. It is the reference state from which the Tomita operator $S$ is constructed, the "anchor" that gives meaning to all other internal states.

### 5.3 The CPTP Update Map

**Definition AM-D13** [DEFINITION] (Observation-Induced Update). An observation $o_t$ induces a CPTP map on the agent's algebra:

$$\rho_{t+1} = \Phi_{o_t}(\rho_t)$$

where $\Phi_{o_t}: \mathcal{A}_* \to \mathcal{A}_*$ is completely positive and trace-preserving (probability is conserved during meaning-extraction).

**Connection to algebraic collapse:** This map $\Phi_{o_t}$ is the agent-side realization of the algebraic collapse map from Part IA, Definition 2.6:

$$\Phi: (\mathcal{R}, \tau) \to (M_n(\mathbb{C}), \tfrac{1}{n}\mathrm{Tr})$$

The physics-side collapse map sends continuous flux (Type II$_1$) to definite outcomes (Type I). The agent-side collapse map sends continuous internal superposition of possible meanings to a definite meaning value. Same algebraic structure, different ontological perspective.

### 5.4 The Meaning Observable

**Definition AM-D14** [DEFINITION] (Meaning Observable). Define a self-adjoint operator $M \in \mathcal{A}$ (the *meaning observable*) such that:

$$\boxed{\mathsf{Meaning}(o_t) = \mathrm{Tr}(M\, \rho_{t+1}) - \mathrm{Tr}(M\, \rho_t)}$$

The meaning of an observation is the change in expectation value of $M$ under the CPTP update.

This is exactly the bookkeeping von Neumann would recognize: meaning $=$ change in expectation of a designated internal functional. The observable $M$ encodes what the agent cares about — its utility, its viability, its purpose.

**Remark:** The meaning observable $M$ is related to but distinct from the valuation functional $J$. Specifically, $J(\eta_t) = \mathrm{Tr}(M \rho_t)$ when $\rho_t$ encodes $\eta_t$ via the GNS construction. The two formulations (Definition AM-D7 and AM-D14) are equivalent descriptions — one in state-space language, one in operator-algebraic language.

### 5.5 The Agent's Factor Type

**Theorem AM-T4** [CONJECTURE] (Agent Factor Type). The internal algebra $\mathcal{A}$, equipped with the state $\rho_t$ determined by the identity kernel $m_t$, is a **Type III$_1$ von Neumann factor**.

**Arguments:**

**1. No trace exists — no "view from nowhere."** The agent has no state-independent way to assign probabilities to its own internal states. Self-evaluation requires the state $\rho_t$ itself (via the modular theory). There is no external vantage point from which the agent can objectively assess its own beliefs. This is the algebraic expression of first-person irreducibility.

**2. All projections equivalent — qualia lack natural metric.** In Type III$_1$, every nonzero projection is Murray-von Neumann equivalent to every other. For the agent, this means: every possible conscious experience is "the same size" from inside — there is no intrinsic way to rank the intensity of redness against the intensity of sweetness without an external calibration. The agent's experiences are all equally "real" to the agent.

**3. Modular flow $\sigma_t$ provides intrinsic time.** By Tomita-Takesaki, the state $\rho_t$ and the cyclic vector $\Omega_{m_t}$ determine a modular automorphism group $\sigma_t^{\rho}(A) = \Delta_\rho^{it} A \Delta_\rho^{-it}$. This is the agent's *intrinsic clock* — time as experienced from inside, not imposed from outside. By the Connes-Rovelli thermal time hypothesis, this modular flow IS the agent's physical time.

**4. Local QFT algebras are Type III$_1$.** If the agent is a quantum field theory system — involving entangled degrees of freedom across spatial regions — then by Buchholz-Wichmann (1986), its local algebra is automatically Type III$_1$.

**Epistemic status:** This is labeled [CONJECTURE] because the assignment of a specific factor type to the agent's internal algebra requires assumptions about the algebra's structure that go beyond the sLoop axioms SL1-SL4. The sLoop guarantees complex structure (SL3) and closure (SL1), but does not directly imply the factor type. The argument is structural, not deductive.

### 5.6 The Component-Factor Correspondence

**Proposition AM-C3** [CONJECTURE] (Internal Component Identification). The four components of $\eta_t = (b_t, V_t, m_t, \pi_t)$ correspond to Tomita-Takesaki structures on $\mathcal{A}$:

| Component | Von Neumann Structure | Justification |
|-----------|----------------------|---------------|
| $b_t$ (beliefs) | **Modular automorphism** $\sigma_t^\rho$ | Each belief update is an automorphism of $\mathcal{A}$. The Connes cocycle theorem (§2.4) says: if $b_t$ and $b_{t+1}$ are two faithful states on $\mathcal{A}$, their modular flows differ by a unitary cocycle $u_t$. Belief change = cocycle perturbation of modular flow. |
| $V_t$ (valuation) | **KMS condition** at $\beta = -1$ | The agent's valuation landscape defines what the agent "cares about," which IS the state $\omega$ that determines the modular flow. By the KMS theorem, $\omega$ is in thermal equilibrium with its own modular time at $\beta = -1$. The valuation IS the equilibrium condition. |
| $m_t$ (identity) | **Cyclic and separating vector** $\Omega$ | The identity kernel is the reference state from which the Tomita operator $S$ is constructed. Change $\Omega$ and you change the entire modular structure — change the identity and you change the agent. The fixed point $\sigma(\psi^*) = \psi^*$ (SL2) corresponds to $\Omega$ being invariant under the self-reference map. |
| $\pi_t$ (policy) | **The algebra** $\mathcal{A}$ **itself** | What the agent can do — the set of available actions, inferences, and evaluations — defines the algebra of observables. Enlarge the policy (learn new skills) and you enlarge $\mathcal{A}$. Restrict it (brain damage, anesthesia) and you restrict $\mathcal{A}$. The algebra IS the agent's capacity for action. |

**The deep point:** In standard quantum mechanics, the algebra $\mathcal{A}$ is given externally (e.g., $B(\mathcal{H})$ for a system with Hilbert space $\mathcal{H}$). For a conscious agent, the algebra is *self-determined*: the policy $\pi_t$ defines $\mathcal{A}$, and $\mathcal{A}$ constrains $\pi_t$. This circularity is the sLoop — the self-referential structure that constitutes agency.

### 5.7 The CPTP Map as Type Transition

**Theorem AM-T5** [THEOREM] (Meaning Extraction as Type Transition). The observation-induced CPTP map $\Phi_{o_t}$ implements the Type II$_1$ $\to$ Type I transition on the agent side:

| Stage | Algebraic Character | Agent Experience |
|-------|---------------------|------------------|
| **Pre-observation** | Continuous internal state: superposition of possible meanings. Type II$_1$-like — continuous dimensions, no definite outcome yet. | "What could this mean?" |
| **CPTP update** $\Phi_{o_t}$ | The map processes $\rho_t$ through $\mathcal{A}$, concentrating probability on specific outcomes. | "Processing..." |
| **Post-observation** | Discrete meaning value $\Delta J_t = \mathrm{Tr}(M\rho_{t+1}) - \mathrm{Tr}(M\rho_t) \in \mathbb{R}$. Type I — a definite number. | "This is what it means." |

**Proof sketch:** The pre-observation state $\rho_t$ assigns continuous probabilities to the spectral projections of $M$ (the algebra $\mathcal{A}$ is Type III$_1$, so internal states have no atoms). The CPTP map $\Phi_{o_t}$, conditioned on the specific observation $o_t$, updates $\rho_t$ to $\rho_{t+1}$. The quantity $\mathrm{Tr}(M\rho_{t+1})$ is a single real number — a definite outcome extracted from the continuous internal state. The continuous-to-discrete transition is the algebraic collapse. $\square$

**Connection to physics-side collapse:** In physics, flux concentrates ($|\mathbf{J}|^2 > K_B$) and a definite state manifests ($s: 0 \to \pm 1$). In the agent, information concentrates (observation $o_t$ arrives) and a definite meaning manifests ($\Delta J_t$ takes a value). Same mechanism, different substrate.

---

## Part VI: The Meaning Operator $i(\cdot)$ and Dynamics

### 6.1 Full Agent State

**Definition AM-D15** [DEFINITION] (Full Agent State). The complete state of an embodied agent is:

$$\Sigma_t = (r_t,\; \eta_t)$$

with $r_t = (x_t, y_t, z_t, \ldots)$ being physical presence and $\eta_t = (b_t, V_t, m_t, \pi_t)$ being internal state.

The split $\Sigma_t = (r_t, \eta_t)$ is the FTD split $\Sigma = (\text{state } s, \text{ flux } \mathbf{J})$ at the agent level: $r_t$ is the discrete manifested state (where the agent is), and $\eta_t$ is the continuous dispositional field (what the agent could become).

### 6.2 The Meaning Operator

**Definition AM-D16** [DEFINITION] (Meaning Operator). The *meaning operator* is the map:

$$i_{\eta_t}: \mathcal{O} \to \mathbb{R}$$

$$i_{\eta_t}(o_t) := \mathsf{Meaning}(o_t;\, \eta_t) = \alpha \cdot \mathsf{IG}_t + \beta \cdot \mathsf{VI}_t$$

This is the agent's "$i(\cdot)$" — the internal machinery that transforms raw observations into value-relevant updates. It is parametrized by the agent's current state $\eta_t$: the same observation means different things to different agents (or to the same agent at different times).

**Notation:** We write $i_{\eta}$ rather than $\mu$ to emphasize:
1. The map is agent-state-dependent (subscript $\eta_t$)
2. It returns a measurable real number, not an abstract semantic element
3. It explicitly implements the "meaning of $i$" — the operator that transforms the raw (real, external) into the meaningful (internally processed)

### 6.3 Epistemic Convergence

**Theorem AM-T6** [THEOREM] (Information Gain Decays Under Repetition). Under repeated presentation of the same observation:

$$o_t = o_{t+1} = o_{t+2} = \cdots \implies \mathsf{IG}_t \to 0 \text{ as } t \to \infty$$

*Proof.* If the same observation is repeated, the Bayesian update converges: $b_t \to b^*$ where $b^*$ is the posterior consistent with repeated observation $o$. As $b_t \to b^*$, we have $b_{t+1} \to b^*$ as well, so $D_{\mathrm{KL}}(b_{t+1} \| b_t) \to D_{\mathrm{KL}}(b^* \| b^*) = 0$. $\square$

**Interpretation:** You cannot be surprised by the same thing forever. Epistemic meaning decays with repetition. This is the algebraic expression of habituation.

### 6.4 Valence Persistence

**Proposition AM-C4** [CONJECTURE] (Valence Does Not Converge). Under the same conditions:

$$\mathsf{VI}_t \text{ does NOT necessarily converge to } 0$$

**Argument:** Valuation $V_t$ is not a metric — it does not have the convergence properties of $D_{\mathrm{KL}}$. An agent can continue to *feel* (positive or negative valence) about a repeated stimulus even after it has learned everything about it. The experience of a sunset remains meaningful (valence-rich) even when it contains zero informational surprise.

**Implication:** This is why consciousness persists even when nothing new is learned. If meaning were purely epistemic ($\beta = 0$), a fully-informed agent would experience zero meaning. The valence component ensures that experience continues to matter even at epistemic equilibrium.

**The algebraic reason:** Information gain lives in Type I (discrete, converges to eigenvalue). Valence lives in Type III$_1$ (ergodic modular flow, no convergence to fixed point). Type III$_1$ has no atoms — the modular flow $\sigma_t$ visits every region of the algebra without settling. The agent's felt experience never reaches a final state.

### 6.5 The Phase Angle Connection

**Proposition AM-C5** [CONJECTURE] (Consciousness Phase Angle). At meaning equilibrium (when $\mathsf{IG}_t$ has stabilized but $\mathsf{VI}_t$ persists), the ratio of valence to epistemic components approaches the consciousness phase angle:

$$\frac{\mathsf{VI}_t}{\mathsf{IG}_t} \to \tan(\theta) \quad \text{where } \theta = \arctan\!\left(2\sqrt{\frac{2 - G^*/4}{G^*}}\right) = 52.54°$$

**Connection:** The consciousness quadratic ($k = 1/2$) has roots $y = 2.19 \pm 2.86i$ with phase angle $\theta = 52.54°$. If the meaning decomposition maps to the discriminant (AM-C1), and the consciousness parameter $k = 1/2$ corresponds to the natural human balance point, then the equilibrium ratio $\mathsf{VI}/\mathsf{IG}$ should match $\tan(52.54°) \approx 1.306$.

**Empirical content:** For a human subject in a balanced task (neither purely epistemic nor purely valence-driven), the inferred ratio $\hat{\beta}\,\mathsf{VI}_t / \hat{\alpha}\,\mathsf{IG}_t$ should stabilize near $1.31$. This is a concrete, falsifiable numerical prediction.

### 6.6 The Meaning Phase Plane

Plot $\mathsf{IG}_t$ (real axis) vs $\mathsf{VI}_t$ (imaginary axis) for an agent over time. The trajectory traces a path in $\mathbb{C}$:

$$\mathsf{Meaning}_t^{\mathbb{C}} := \mathsf{IG}_t + i \cdot \mathsf{VI}_t$$

**Predicted behavior:**
- Early learning (high surprise): trajectory far from origin along real axis ($\mathsf{IG}$ dominates)
- Emotional event (high valence): trajectory jumps along imaginary axis ($\mathsf{VI}$ dominates)
- Equilibrium: trajectory spirals toward angle $\theta \approx 52.54°$ from real axis
- Different sLoop levels (Connes III$_\lambda$ from VON_NEUMANN doc §3.2) trace different spirals:
  - Level 1 (reactive): tight oscillation near origin
  - Level 3 (self-aware): spiral with $\lambda \approx 0.400$
  - Level 4 (self-reflective): ergodic filling of a sector

---

## Part VII: Scientific Standards and Falsification

All claims in this section are **[PROPOSED]** — they are experimental predictions, not yet tested.

### 7.1 Operationalization

For the framework to count as scientific, every variable must have a measurement protocol:

| Variable | Measurement |
|----------|-------------|
| $a_t$ (actions) | Behavior logs: button presses, eye movements, verbal responses |
| $o_t$ (observations) | Controlled stimulus delivery (experimenter sets this) |
| $b_t$ (beliefs) | Latent-variable inference from choice behavior + confidence reports |
| $V_t$ (valuation) | Preference/reward experiments (revealed preference, willingness-to-pay) |
| $m_t$ (identity) | Longitudinal tracking of what persists across sessions |
| $\pi_t$ (policy) | Choice model parameters (softmax temperature, exploration rate) |
| $J(\eta_t)$ (valuation functional) | Inferred from $V_t$ and task performance |
| $\mathsf{IG}_t$ | Computed from $b_t, b_{t+1}$ (model-based) |
| $\mathsf{VI}_t$ | Computed from $V_t, \eta_t, \eta_{t+1}$ (model-based) |

### 7.2 Identifiability Constraints

The framework must satisfy identifiability — different internal models cannot fit the same data arbitrarily well:

1. **Minimal state dimension:** Impose regularization (minimum description length) on the belief space $\mathcal{B}$
2. **Cross-task generalization:** $\alpha, \beta$ must be stable across tasks for a given agent
3. **Predictive validity:** The inferred $\eta_t$ must predict *future* behavior, not just fit *past* data

### 7.3 Six Falsifiable Predictions

**Prediction AM-P1** [PROPOSED] (Valence Gating). If the reward gradient is flattened (no incentives, no punishments), then $\mathsf{Meaning}_t$ collapses to near zero even if $\mathsf{IG}_t$ remains high.

*Test:* High-information stimuli with/without reward contingency. If meaning = pure $\mathsf{IG}$ (no valence component), then removing rewards should not affect meaning-related behavior. If $\beta > 0$, it should.

*Falsified by:* Meaning-related behavior (learning speed, engagement, physiological arousal) unchanged when reward is removed.

---

**Prediction AM-P2** [PROPOSED] (Policy Relevance). Observations with equal information gain but different action consequences produce different "meaning signatures" (behavioral + physiological).

*Test:* Two conditions with matched $\mathsf{IG}_t$ but different action relevance (one informs a future decision, one does not).

*Falsified by:* Identical meaning signatures for action-relevant vs action-irrelevant information.

---

**Prediction AM-P3** [PROPOSED] (Identity Persistence). The identity kernel $m_t$ predicts which belief updates "stick" (long-term learning) versus which wash out (short-term priming).

*Test:* Track $m_t$ longitudinally. Updates consistent with $m_t$ should consolidate; updates orthogonal to $m_t$ should decay.

*Falsified by:* No relationship between $m_t$ and learning persistence.

---

**Prediction AM-P4** [PROPOSED] (Phase Angle). The equilibrium ratio $\mathsf{VI}_t / \mathsf{IG}_t$ stabilizes near $\tan(52.54°) \approx 1.306$ for human subjects in balanced tasks.

*Test:* Measure both components over extended task performance. Compute ratio.

*Falsified by:* Ratio converging to a significantly different value (outside $1.31 \pm 0.20$, accounting for individual variation).

---

**Prediction AM-P5** [PROPOSED] (Collapse Discreteness). Meaning arrives in discrete quanta (Type I outcomes), not as continuous flow.

*Test:* High-resolution temporal measurement of meaning-related physiological signals (pupil dilation, ERP components). Look for discrete jumps vs continuous drift.

*Falsified by:* Purely continuous meaning dynamics with no discrete transitions.

---

**Prediction AM-P6** [PROPOSED] (sLoop Level Correlation). The Connes $\lambda$ parameter (from Part IA, section IA.6.1) correlates with information processing capacity across agents.

*Test:* Compare $\lambda$ (estimated from the meaning phase plane spiral) across agents with different cognitive capacities (developmental stages, clinical populations, AI systems).

*Falsified by:* No correlation between $\lambda$ and any measure of information processing.

### 7.4 Intersubjective Reproducibility

The framework "normalizes subjectivity" by requiring stable invariants across agents:

1. **Functional form:** Same tasks $\to$ same functional form for $\mathsf{Meaning}_t$ up to calibration constants $(\alpha, \beta)$
2. **Predictable transformations:** $\mathsf{Meaning}_t$ should change predictably under:
   - Training (increased $\pi_t$ capacity $\to$ enlarged $\mathcal{A}$)
   - Fatigue (decreased processing $\to$ reduced $\mathsf{IG}_t$)
   - Anesthesia (suppressed valence $\to$ $\beta \to 0$)
   - Lesions (specific component loss $\to$ specific meaning deficit)

---

## Part VIII: Minimal Experiment Protocol

All content in this section is **[PROPOSED]** — experimental design, not yet executed.

### 8.1 Task

Sequential decision-making under uncertainty: **two-armed bandit** with varying reward and information structure.

The agent (human subject) makes repeated choices between two options, receiving feedback after each choice. This is the simplest task that engages all four components of $\eta_t$:
- $b_t$: beliefs about which arm is better (updated by feedback)
- $V_t$: valuation (how much reward matters)
- $m_t$: identity (exploration/exploitation style that persists)
- $\pi_t$: policy (choice strategy)

### 8.2 The 2$\times$2 Manipulation

| | **High Information** (novel stimuli) | **Low Information** (repeated stimuli) |
|---|---|---|
| **High Reward** | Condition A: Both $\mathsf{IG}$ and $\mathsf{VI}$ high | Condition B: $\mathsf{VI}$ high, $\mathsf{IG}$ low |
| **Low Reward** | Condition C: $\mathsf{IG}$ high, $\mathsf{VI}$ low | Condition D: Both low |

**Factor 1 — Information content:** Manipulated by varying the volatility of the reward contingency (high volatility = novel, surprising feedback; low volatility = predictable, repeated feedback).

**Factor 2 — Reward salience:** Manipulated by varying the magnitude of payoffs (large vs. small monetary rewards, or social vs. minimal feedback).

### 8.3 Measurements

**Behavioral:**
- Choice ($a_t$): which arm was selected
- Reaction time: speed of decision (proxy for processing depth)
- Error rate: proportion of suboptimal choices
- Exploration rate: frequency of trying the less-certain arm

**Physiological (optional but recommended):**
- Pupil dilation: proxy for arousal/surprise ($\approx \mathsf{IG}_t$)
- Heart rate variability (HRV): proxy for emotional engagement ($\approx \mathsf{VI}_t$)
- EEG (if available): P300 amplitude (surprise), reward positivity (valence)

**Subjective (treated as another observable, not "ground truth"):**
- Confidence reports: "How sure are you?"
- Engagement reports: "How meaningful was that trial?"

### 8.4 Model-Based Inference

1. **Fit a computational model** to each subject's choice data (e.g., Rescorla-Wagner, Kalman filter, or Bayesian learner)
2. **Extract latent variables:** $b_t$ (posterior beliefs), $V_t$ (reward estimates), $\pi_t$ (softmax policy)
3. **Compute meaning components:** $\mathsf{IG}_t = D_{\mathrm{KL}}(b_{t+1} \| b_t)$ and $\mathsf{VI}_t = V_t(\eta_{t+1}) - V_t(\eta_t)$
4. **Fit decomposition weights:** $\alpha, \beta$ from behavioral + physiological data
5. **Cross-validate:** Train on conditions A+D, predict behavior in conditions B+C

### 8.5 Connection to VN-C4 and VN-C5

This protocol directly tests two conjectures from the von Neumann document:

| Conjecture | What the protocol tests | Expected result |
|-----------|------------------------|-----------------|
| **VN-C4** (Collapse = Type II$_1$ $\to$ Type I) | Does meaning emerge discretely? | Discrete jumps in physiological signals at meaning-extraction moments |
| **VN-C5** (sLoop levels $\to$ III$_\lambda$) | Does $\lambda$ correlate with information processing? | Higher-capacity agents show larger $\lambda$ (more ergodic meaning dynamics) |

### 8.6 Expected Results Under the Model

| Condition | $\mathsf{IG}_t$ | $\mathsf{VI}_t$ | Predicted $\mathsf{Meaning}_t$ | Behavioral signature |
|-----------|-------------------|-------------------|--------------------------------|---------------------|
| A (high info + high reward) | High | High | **Maximal** | Fast learning, high engagement, large pupil + HRV response |
| B (low info + high reward) | Low | High | **Moderate** (valence only) | Habitual responding, sustained HRV but no pupil response |
| C (high info + low reward) | High | Low | **Moderate** (epistemic only) | Curious exploration, pupil response but minimal HRV |
| D (low info + low reward) | Low | Low | **Minimal** | Boredom, disengagement, flat physiological signals |

**Critical test:** If meaning = pure information (no valence), then B $=$ D and C $=$ A. If the decomposition holds, then A $>$ B $\approx$ C $>$ D with B and C having distinct *profiles* (different physiological patterns despite similar total meaning).

---

## Part IX: Epistemic Taxonomy

### 9.1 Classification of All Claims

#### Classical Mathematics [CLASSICAL]

Imported from established literature; we state but did not prove.

| ID | Statement | Source |
|----|-----------|--------|
| (from §2.4) | Tomita-Takesaki modular theory: $S = J\Delta^{1/2}$, $\sigma_t(a) = \Delta^{it}a\Delta^{-it}$ | Tomita (1967), Takesaki (1970) |
| (from §2.4) | KMS condition at $\beta = -1$ for modular states | Haag-Hugenholtz-Winnink (1967) |
| (from §2.4) | Connes cocycle: $\sigma_t^\phi = \mathrm{Ad}(u_t) \circ \sigma_t^\omega$ | Connes (1973) |
| (from §2.5) | CPTP maps: Kraus representation, trace preservation | Stinespring (1955), Kraus (1971) |
| (from §2.6) | $D_{\mathrm{KL}}(P\|Q) \geq 0$ with equality iff $P = Q$ | Kullback-Leibler (1951) |
| AM-T2 | $\mathsf{IG}_t \geq 0$ (Gibbs inequality) | Classical information theory |

#### Definitions [DEFINITION]

New formal objects introduced in this document.

| ID | Object | Section |
|----|--------|---------|
| AM-D1 | Physical state $r_t \in \mathbb{R}^3 \times \mathbb{R}^k$ | §3.1 |
| AM-D2 | Internal state $\eta_t = (b_t, V_t, m_t, \pi_t)$ | §3.2 |
| AM-D3 | Observation channel $o_t \sim P(o \mid s_t, r_t)$ | §3.3 |
| AM-D4 | Action channel $a_t \sim \pi_t(\cdot \mid \eta_t, o_{\leq t})$ | §3.4 |
| AM-D5 | Internal update $\eta_{t+1} = \mathcal{U}(\eta_t, o_t, a_t)$ | §3.5 |
| AM-D6 | Agent-environment loop (full system) | §3.6 |
| AM-D7 | Meaning $= J(\eta_{t+1}) - J(\eta_t)$ | §4.1 |
| AM-D8 | Information gain $\mathsf{IG}_t = D_{\mathrm{KL}}(b_{t+1}\|b_t)$ | §4.2 |
| AM-D9 | Valence impact $\mathsf{VI}_t = V_t(\eta_{t+1}) - V_t(\eta_t)$ | §4.3 |
| AM-D10 | Meaning decomposition $\alpha \cdot \mathsf{IG} + \beta \cdot \mathsf{VI}$ | §4.4 |
| AM-D11 | Internal algebra $\mathcal{A}$ | §5.1 |
| AM-D12 | Agent state $\rho_t$ on $\mathcal{A}$ | §5.2 |
| AM-D13 | CPTP update $\Phi_{o_t}$ | §5.3 |
| AM-D14 | Meaning observable $M \in \mathcal{A}$ | §5.4 |
| AM-D15 | Full agent state $\Sigma_t = (r_t, \eta_t)$ | §6.1 |
| AM-D16 | Meaning operator $i_{\eta_t}(o_t)$ | §6.2 |

#### Theorems [THEOREM]

Provable within FTD axioms + stated definitions.

| ID | Statement | Depends On | Section |
|----|-----------|-----------|---------|
| AM-T1 | Agent-environment loop instantiates sLoop | AM-D1–D6, SL1-SL4 | §3.7 |
| AM-T2 | $\mathsf{IG}_t \geq 0$ | Gibbs inequality [CLASSICAL] | §4.2 |
| AM-T3 | $\mathsf{VI}_t \in \mathbb{R}$ (no sign constraint) | AM-D9 | §4.3 |
| AM-T5 | CPTP map implements Type II$_1$ $\to$ Type I transition | AM-D13, AM-D14, VN Def 2.6 | §5.7 |
| AM-T6 | $\mathsf{IG}_t \to 0$ under repeated stimuli | AM-D8, Bayesian convergence | §6.3 |

#### Conjectures [CONJECTURE]

Structural correspondences requiring validation.

| ID | Statement | Depends On | Falsifiable? | Section |
|----|-----------|-----------|-------------|---------|
| AM-C1 | $\mathsf{IG} \leftrightarrow$ Domain A, $\mathsf{VI} \leftrightarrow$ Domain B | Discriminant partition | Via AM-P1 (valence gating) | §4.5 |
| AM-C2 | $\alpha/\beta \leftrightarrow$ master quadratic parameter $k$ | AM-D10, discriminant | Via AM-P4 (phase angle) | §4.6 |
| AM-C3 | $(b_t, V_t, m_t, \pi_t) \leftrightarrow$ (modular flow, KMS, $\Omega$, algebra) | AM-D11-12, Tomita-Takesaki | Via AM-P3 (identity persistence) | §5.6 |
| AM-C4 | $\mathsf{VI}_t$ does not converge to 0 | Type III$_1$ ergodicity | Via longitudinal study | §6.4 |
| AM-C5 | Equilibrium $\mathsf{VI}/\mathsf{IG} \to \tan(52.54°)$ | Consciousness phase angle | Via AM-P4 | §6.5 |
| AM-T4 | Agent's algebra $\mathcal{A}$ is Type III$_1$ | AM-D11, AM-D12, structural argument | Via AM-P6 | §5.5 |

#### Predictions [PROPOSED]

Falsifiable experimental predictions.

| ID | Prediction | Test | Falsified By | Section |
|----|-----------|------|-------------|---------|
| AM-P1 | Valence gating | Remove rewards, check meaning | Meaning unchanged without rewards | §7.3 |
| AM-P2 | Policy relevance | Match IG, vary action relevance | No difference for relevant vs irrelevant info | §7.3 |
| AM-P3 | Identity persistence | Track $m_t$ vs learning | No relationship between identity and learning | §7.3 |
| AM-P4 | Phase angle $\approx 52.54°$ | Measure VI/IG ratio | Ratio far from 1.306 | §7.3 |
| AM-P5 | Discrete meaning quanta | High-res physiological signals | Purely continuous dynamics | §7.3 |
| AM-P6 | $\lambda$ correlates with processing | Compare across agents | No correlation | §7.3 |

### 9.2 What Is Novel

Six contributions not found in any existing FTD document:

1. **$\eta_t = (b_t, V_t, m_t, \pi_t)$ identified with Type III$_1$ Tomita-Takesaki structures** (AM-T4, AM-C3). The von Neumann document assigns consciousness to Type III$_1$ generically; this document provides the internal decomposition.

2. **IG/VI decomposition mapped to Domain A/Domain B** (AM-C1). The archived Noetic Framework has IG but never maps the decomposition to the discriminant domains or factor types.

3. **$\alpha/\beta$ weight $\to$ master quadratic parameter $k$** (AM-C2). The connection between meaning-component weighting and the parameter controlling real vs complex roots is entirely new.

4. **CPTP update $\Phi_{o_t}$ as agent-side algebraic collapse** (AM-T5). Definition 2.6 in the VN document defines collapse from the physics side; the agent-side identification is novel.

5. **Falsifiable experimental protocol** (Part VIII). No existing FTD document provides a concrete laboratory protocol for testing consciousness claims.

6. **Meaning operator $i_{\eta_t}$** (AM-D16). This operationalizes the sLoop's abstract $\mu$ with a von Neumann-compatible operator, replacing the unspecified semantic space $S$ with $\mathbb{R}$ (or $\mathbb{C}$).

### 9.3 What Extends Existing Work

| Document Extended | What Is Extended | How |
|-------------------|------------------|-----|
| [FOUND_SLOOP_FORMALIZATION.md](FOUND_SLOOP_FORMALIZATION.md) | $\mu: M \to S$ (abstract meaning map) | Replaced by $i_{\eta_t}: \mathcal{O} \to \mathbb{R}$ with concrete IG + VI decomposition |
| Part IA (formerly FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md) | VN-C3 (Type III$_1$ = consciousness) | Given internal structure: $(b, V, m, \pi) \leftrightarrow$ (modular flow, KMS, $\Omega$, algebra) |
| Part IA (formerly FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md) | VN-C4 (collapse map $\Phi$) | Extended to agent side: CPTP update $\Phi_{o_t}$ as meaning-extraction |
| [FOUND_CONSCIOUSNESS_MATHEMATICS.md](FOUND_CONSCIOUSNESS_MATHEMATICS.md) | Phase angle $\theta = 52.54°$ | Reinterpreted as equilibrium IG/VI ratio |
| archive/NOETIC_FRAMEWORK.md | $\mathsf{IG}_t = D_{\mathrm{KL}}$, noetic mass | Superseded with algebraic grounding and experimental protocol |

### 9.4 Open Questions

| ID | Question | Priority |
|----|----------|----------|
| AM-O1 | Can the $\alpha/\beta \to k$ correspondence be made exact (derived, not proposed)? | **High** |
| AM-O2 | Is the Type III$_1$ assignment for $\mathcal{A}$ provable from SL1-SL4 alone, or does it require additional axioms? | **High** |
| AM-O3 | What is the complete mathematical structure of the semantic space $S$? (Currently reduced to $\mathbb{R}$ or $\mathbb{C}$.) | Medium |
| AM-O4 | Can the $2 \times 2$ experiment protocol distinguish this framework from simpler accounts (pure Bayesian surprise, active inference)? | **High** |
| AM-O5 | Does the Connes cocycle (relating modular flows of different states) have operational meaning for belief change? If $b_t$ and $b_{t+1}$ are two states on $\mathcal{A}$, their modular flows differ by a cocycle $u_t$ — is this the algebraic structure of learning? | Medium |

---

## Part X: Summary and Cross-References

### 10.1 Central Result

We have operationalized the sLoop's meaning map $\mu: M \to S$ by identifying the agent's internal state $\eta_t = (b_t, V_t, m_t, \pi_t)$ with Tomita-Takesaki structures on a Type III$_1$ von Neumann algebra (modular flow, KMS condition, cyclic vector, algebra itself), defining meaning as a measurable quantity $\mathsf{Meaning}_t = \alpha \cdot \mathsf{IG}_t + \beta \cdot \mathsf{VI}_t$ decomposed into epistemic (Domain A, Type I) and valence (Domain B, Type III$_1$) components, and providing a concrete experimental protocol for falsification. The meaning operator $i_{\eta_t}(o_t) = \mathsf{Meaning}(o_t; \eta_t)$ is the CPTP-map-induced change in a self-adjoint observable $M$ on the agent's algebra — exactly the bookkeeping von Neumann established for quantum measurement, now applied to the measurement of meaning.

### 10.2 Key Equations

**1. Meaning decomposition:**

$$\boxed{\mathsf{Meaning}_t = \alpha \cdot D_{\mathrm{KL}}(b_{t+1}\|b_t) + \beta \cdot [V_t(\eta_{t+1}) - V_t(\eta_t)]}$$

**2. Meaning observable (operator-algebraic):**

$$\boxed{\mathsf{Meaning}(o_t) = \mathrm{Tr}(M\,\rho_{t+1}) - \mathrm{Tr}(M\,\rho_t), \quad \rho_{t+1} = \Phi_{o_t}(\rho_t)}$$

**3. Full agent state:**

$$\boxed{\Sigma_t = (r_t,\; \eta_t), \quad \eta_t = (b_t, V_t, m_t, \pi_t), \quad i_{\eta_t}(o_t) := \mathsf{Meaning}(o_t;\,\eta_t)}$$

**4. Phase angle prediction:**

$$\boxed{\frac{\mathsf{VI}_t}{\mathsf{IG}_t} \to \tan\!\left(\arctan\!\left(2\sqrt{\frac{2-G^*/4}{G^*}}\right)\right) \approx 1.306 \quad \text{at equilibrium}}$$

### 10.3 Cross-References

| Document | Relevance |
|----------|-----------|
| [FOUND_SLOOP_FORMALIZATION.md](FOUND_SLOOP_FORMALIZATION.md) | Foundation: quintuple $(\Omega, \phi, \sigma, \mu, d)$, axioms SL1-SL4, hierarchy |
| Part IA of this document (formerly FOUND_VON_NEUMANN_FACTOR_CLASSIFICATION.md) | Foundation: factor types, collapse map $\Phi$, sentience hierarchy |
| [FOUND_CONSCIOUSNESS_MATHEMATICS.md](FOUND_CONSCIOUSNESS_MATHEMATICS.md) | Domain A/B partition, phase angle $\theta = 52.54°$, Born rule |
| [SPEC_THE_MASTER_QUADRATIC_UNIFIED.md](SPEC_THE_MASTER_QUADRATIC_UNIFIED.md) | Discriminant $\Delta(k)$, parameter $k$, bridge equation |
| [DERIV_QUANTUM_MECHANICS_RESOLVED.md](DERIV_QUANTUM_MECHANICS_RESOLVED.md) | Collapse = manifestation, observer coupling $\mathcal{L} = -g_c \cdot s \cdot (\nabla \cdot \mathbf{J})$ |
| [AUDIT_BELL_ANALYSIS.md](AUDIT_BELL_ANALYSIS.md) | Substrate-to-aggregate transition parallel |
| [EXPLR_TRIT_INFORMATION_THEORY.md](EXPLR_TRIT_INFORMATION_THEORY.md) | Shannon entropy, self-duality |
| [FOUND_CONSCIOUSNESS_MATHEMATICS.md](FOUND_CONSCIOUSNESS_MATHEMATICS.md) | Consciousness on $\partial\mathcal{M}$, $c = 1/G^*$ |
| [EXPLR_LOOP_GRID_DUALITY.md](EXPLR_LOOP_GRID_DUALITY.md) | Two-layer ontology (continuous/discrete) |
| archive/NOETIC_FRAMEWORK.md | Predecessor: $\mathsf{IG}_t = D_{\mathrm{KL}}$, noetic mass (superseded) |

### 10.4 Claims Summary

| Category | Count | IDs |
|----------|-------|-----|
| Classical theorems | 6 | Tomita-Takesaki, KMS, Connes cocycle, CPTP, Gibbs, AM-T2 |
| Definitions | 16 | AM-D1 through AM-D16 |
| Theorems | 5 | AM-T1, AM-T2, AM-T3, AM-T5, AM-T6 |
| Conjectures | 6 | AM-C1 through AM-C5, AM-T4 |
| Predictions | 6 | AM-P1 through AM-P6 |
| Open questions | 5 | AM-O1 through AM-O5 |
| **Total** | **44** | |

---

*Agent Meaning Formalization — Foundational Ternary Dynamics v5.24*
*Prepared for critical evaluation*
*February 13, 2026*
