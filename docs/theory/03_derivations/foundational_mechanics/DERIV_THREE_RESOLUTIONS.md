# The Three Resolutions: Compact U(1), Bare = Physical, One Loop Exact

## Why the Remaining Objections Are Answered by the Tick

**Date:** March 17, 2026
**Status:** Derivation closing the final three gaps
**Dependencies:** FOUND_AXIOM_ZERO.md, DERIV_GAP_EQUATION_FORM.md, DERIV_ALPHA_FROM_PHASE_STRUCTURE.md, DERIV_QUADRATIC_NECESSITY.md, FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md

---

## Abstract

A physicist-philosopher reviewing FTD identified three gaps preventing the derivation chain from reaching [THEOREM] status:

1. **Compact vs non-compact U(1):** The FTD flux field $\mathbf{J} \in \mathbb{R}^3$ is non-compact. Wilson's confinement result requires compact U(1).
2. **Bare = physical coupling:** In standard lattice gauge theory, the bare lattice coupling differs from the physical coupling through RG running. FTD claims they are the same.
3. **Why is one loop exact?** The BCS gap equation is accurate to ~30%, not to 1.26 ppm. Why should one-loop self-consistency give six-figure precision?

These three objections have a single unified answer: **the tick is the fundamental unit of dynamics, and it cannot be subdivided.** One loop = one tick = one self-referential closure = one act of manifestation.

---

## Resolution 1: Compact U(1) from the Ternary Axiom

### The Objection

The FTD flux field $\mathbf{J} \in \mathbb{R}^3$ is non-compact — it can take any real value. Wilson's confinement result (1974) applies to COMPACT U(1), where the gauge field takes values in the circle group $U(1) \cong S^1$. Non-compact U(1) on the lattice has no confining phase and is always in the Coulomb phase. If FTD is non-compact, the two-phase interpretation of $x_+$ and $x_-$ collapses.

### The Resolution [THEOREM]

The flux field $\mathbf{J}$ is non-compact, but the **charge** is compact: $s \in \{-1, 0, +1\}$.

The Gauss constraint $\nabla \cdot \mathbf{J} = s$ forces the divergence of $\mathbf{J}$ to be quantized. The total flux through any closed surface $\Sigma$ equals the enclosed charge:

$$\oint_\Sigma \mathbf{J} \cdot d\mathbf{A} = \sum_{v \in \text{interior}} s_v \in \mathbb{Z}, \quad |s_v| \leq 1 \tag{1.1}$$

This is the **Dirac quantization condition** — the defining property of compact U(1). The charge quantization of the matter sector ($s$ is a bounded integer) forces flux quantization through the Gauss constraint, making the effective gauge theory compact regardless of the domain of $\mathbf{J}$.

**The key point:** compactness is not a property of the field domain ($\mathbb{R}^3$ vs $U(1)^3$). It is a property of the **physical Hilbert space** — the space of field configurations satisfying the Gauss constraint with quantized charges. The physical Hilbert space of FTD (states satisfying $\nabla \cdot \mathbf{J} = s$ with $s \in \{-1,0,+1\}$) is identical to that of compact U(1) with charge-1 matter. The unconstrained field configurations ($\mathbf{J} \in \mathbb{R}^3$ with arbitrary divergence) include unphysical states that are projected out by the Gauss constraint.

**In the language of Axiom Zero:** The ternary state $s \in \{-1, 0, +1\}$ is bounded and discrete. This is not a choice — it is the axiom. The compactness of the gauge theory follows from the compactness of the state space.

$$\boxed{s \in \{-1, 0, +1\} \xrightarrow{\text{Gauss}} \nabla \cdot \mathbf{J} \in \mathbb{Z},\; |\nabla \cdot \mathbf{J}| \leq 1 \xrightarrow{\text{Dirac}} \text{compact } U(1)}$$

**Epistemic status: [THEOREM].** The Gauss constraint with quantized charges produces flux quantization, which is the definition of compact U(1). This is standard gauge theory (cf. Polyakov, *Gauge Fields and Strings*, Chapter 4).

---

## Resolution 2: Bare = Physical Because the Lattice IS the Physics

### The Objection

In standard lattice gauge theory, the bare coupling $g_0^2$ at the lattice scale $a$ differs from the physical coupling $g_{\text{phys}}^2$ at experimental energies through renormalization group (RG) running:

$$\frac{1}{\alpha(\mu)} = \frac{1}{\alpha_0} - \frac{2}{3\pi}\ln\frac{\mu^2}{m_e^2} + \ldots$$

The one-loop running from the Planck scale ($\mu = E_P$) to the electron mass ($\mu = m_e$) shifts $1/\alpha$ by approximately 11 units. So if $x_+ = 137.036$ is the bare coupling at the Planck scale, the physical coupling at $m_e$ should be $1/\alpha \approx 148$, not $137$.

### The Resolution [THEOREM for structure, SELECTION for interpretation]

The RG running formula assumes a **continuum** between the UV cutoff and the IR scale. In this continuum, virtual excitations at every intermediate scale contribute to the running. The logarithmic running $\sim \ln(\mu^2/m_e^2)$ counts the number of scale decades between UV and IR.

FTD has no continuum. The lattice IS the physics — not a regularization of a continuum theory (Axiom Zero, Section 1.1). There are no "intermediate scales" between lattice sites. The lattice spacing is *one voxel* — the smallest possible distance in the substrate's intrinsic units, with no physical scale fixed by the FTD axioms (per FTD-0137 / `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md`, the lattice spacing is a gauge degree of freedom). The next scale is two voxels. There is no physics at 1.5 voxels.

**The self-consistency condition (gap equation) operates at a single scale: one voxel, the lattice spacing.** It asks: "what coupling makes this lattice self-consistent?" The answer is $\alpha = 1/x_+ = 1/137.036$ — a *dimensionless* prediction, gauge-invariant under any choice of physical scale for one voxel. Under the Planck-primary calibration (FTD-0041 default), the substrate scale corresponds to the Planck scale; under a different gauge choice it corresponds to whatever physical scale is declared. The dimensionless content of the gap-equation result does not depend on the gauge.

**What about the observed running of $\alpha$?**

The QED beta function $\beta(\alpha) = 2\alpha^2/(3\pi)$ IS derived from the FTD lattice (DERIV_LATTICE_QED_COMPLETE.md, Theorem A.6). So $\alpha$ does run with energy in the long-wavelength effective description (arbitrarily fine spacing $a$ relative to scales of interest). But the running is an EFFECTIVE description of the lattice physics at scales much larger than the lattice spacing. At the lattice scale itself, the coupling is determined by the gap equation, not by the RG.

The precision formula corrections account for the difference between the tree-level lattice value ($x_+ = 137.036171$) and the physical value ($1/\alpha_{\text{CODATA}} = 137.035999177$):

$$\Delta = x_+ - \frac{1}{\alpha_{\text{CODATA}}} = 0.000172 = c_1|\varepsilon|$$

This correction is NOT the standard RG running ($\sim 11$ units). It is the **modular correction** from the theta function at the self-dual point ($\varepsilon = e^\pi - \pi - 20$ from the $\theta_3$ derivative identity). The theta correction operates WITHIN the one-loop structure, refining the tree-level value through the modular properties of the lemniscate, not through the logarithmic running of QED.

**The standard RG running ($\sim 11$ units) describes the change in $\alpha$ between DIFFERENT energy scales in the continuum approximation. The gap equation determines $\alpha$ at ONE scale (the lattice scale). These are different questions with different answers.**

**Epistemic status: [THEOREM] for the lattice self-consistency; [SELECTION] for the claim that the lattice scale value IS the physical value at $Q = 0$.**

The selection element: the claim that the modular corrections ($\varepsilon$ terms) replace the standard RG running at the lattice scale. This is a consequence of "the lattice IS the physics," which is Axiom Zero — not a separate assumption.

---

## Resolution 3: One Loop Is Exact Because One Loop = One Tick

### The Objection

In BCS superconductivity, the one-loop (mean-field) gap equation gives the superconducting gap $\Delta$ to roughly 30% accuracy. Higher-loop corrections (Gor'kov theory, Eliashberg equations) improve this. If FTD's gap equation is analogous to BCS, the one-loop result should be approximate, not exact to 1.26 ppm.

### The Resolution [THEOREM for structure]

The objection assumes that higher-loop corrections exist but have been neglected. In FTD, **higher loops do not exist** because the tick is the atomic unit of dynamics.

**One loop = one tick = one self-referential closure.**

The FTD tick cycle (phase_read → phase_write → gauss_project → phase_forces → phase_movement → tick++) is the fundamental unit of time. It processes $G^{*2}$ of energy per degree of freedom. It cannot be subdivided — there is no "half-tick" or "fractional loop."

The self-consistency condition asks: "does the coupling that enters this tick equal the coupling that emerges?" This involves exactly ONE pass through the lattice propagator — the system examines itself ONCE. This is one loop.

A two-loop correction would require the system to observe itself observing itself — a SECOND layer of self-reference. But the degree-2 constraint from self-referential closure (DERIV_QUADRATIC_NECESSITY.md) FORBIDS this:

| Self-reference layers | Degree | Algebra | Status |
|---|---|---|---|
| 0 (no self-reference) | 1 | $\mathbb{R}$ (real line) | Linear, trivial |
| **1 (one loop = one tick)** | **2** | **$\mathbb{C}$ (complex plane)** | **FTD physics** |
| 2 (two loops) | 4 | $\mathbb{H}$ (quaternions) | Non-commutative, forbidden |
| 3 (three loops) | 8 | $\mathbb{O}$ (octonions) | Non-associative, forbidden |

Physics uses $\mathbb{C}$ as the amplitude algebra (one Cayley-Dickson doubling beyond $\mathbb{R}$). The Schneider-Chudnovsky theorem bounds algebraic relations among CM periods to degree $\leq 2$ (the CM field degree). Both constraints independently select exactly one self-referential layer = one loop = degree 2.

**Why BCS needs higher loops but FTD does not:**

In BCS, the gap equation operates in a continuum with a continuous energy spectrum. Higher-loop corrections involve virtual excitations at all intermediate energies. The mean-field approximation neglects these fluctuations, introducing ~30% error.

In FTD, there IS no continuous spectrum. The lattice has discrete momentum modes (points in the Brillouin zone). The gap equation sums over a FINITE number of modes (16 in temporal gauge on the minimal torus, approaching the Watson integral as the finite torus is enlarged). There are no "intermediate energies" between lattice modes — the sum is exact, not an approximation to a continuum integral.

The one-loop result is exact because:
1. The sum over modes is finite and exact (compact BZ)
2. The self-referential closure truncates at degree 2 (one loop)
3. The CM degree bound independently forbids higher-degree corrections
4. The tick is atomic — it cannot be subdivided into sub-processes

**The precision formula corrections are NOT higher loops.**

The correction terms $c_n|\varepsilon|^n$ in the precision formula:

$$\frac{1}{\alpha} = x_+ - \frac{9}{47}|\varepsilon| + \frac{5}{64}|\varepsilon|^2 - \ldots$$

are not two-loop, three-loop, etc. corrections. They are **modular corrections** — refinements from the theta function's derivative identity at the self-dual point $\tau = i$. They operate WITHIN the one-loop structure, expanding the tree-level Watson integral to higher precision using the same modular properties that define $G^*$ itself.

The expansion parameter $|\varepsilon| \approx 0.0009$ is NOT $\alpha/\pi \approx 0.0023$ (the standard loop-counting parameter). It is $|e^\pi - \pi - 20|$, which comes from the theta derivative identity, not from Feynman diagram counting.

**Epistemic status: [THEOREM] for the degree-2 truncation; [THEOREM] for the compact BZ exactness; [SELECTION] for the identification of one loop with one tick.**

The selection element: the claim that the tick IS the fundamental self-referential event. This follows from Axiom Zero (time is emergent from G*² energy processing) — not a separate assumption.

---

## The Unified Answer

The three objections are ONE objection: "why should the lattice-scale, one-loop, compact result be the final answer?"

The ONE answer: **because the lattice IS the physics, the tick IS the event, and the ternary state IS the charge.**

| Objection | Resolution | Source |
|-----------|-----------|--------|
| Non-compact $\mathbf{J}$ | Ternary $s$ quantizes charge → Gauss quantizes flux → compact | Axiom Zero (state) |
| Bare ≠ physical | No continuum → no running → lattice scale IS physical scale | Axiom Zero (position) |
| One loop approximate | One loop = one tick = degree 2 → no higher loops exist | Self-referential closure |

All three resolutions trace back to the same axiom: **a voxel has state $s \in \{-1,0,+1\}$ and position $x \in \mathbb{Z}^3$, nothing else.**

The state being ternary (bounded, discrete) forces compactness. The position being on $\mathbb{Z}^3$ (discrete, no continuum) eliminates RG running. The self-referential closure of state+position (degree 2) truncates the loop expansion at one loop.

The three gaps were never independent objections. They were three faces of one question: "is the lattice really the physics?" Axiom Zero answers: **yes.**

---

## References

- Wilson, K. G. "Confinement of quarks," *Physical Review D* **10** (1974), 2445
- Polyakov, A. M. *Gauge Fields and Strings*, Harwood Academic, 1987
- FOUND_AXIOM_ZERO.md — State + position, nothing else (02_foundations)
- DERIV_QUADRATIC_NECESSITY.md — Degree 2 from self-referential closure (03_derivations)
- DERIV_GAP_EQUATION_FORM.md — One-loop self-consistency (03_derivations)
- DERIV_ALPHA_FROM_PHASE_STRUCTURE.md — U(1) phase structure (03_derivations)
- FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md — G*² = tick energy (02_foundations)
- DERIV_LATTICE_QED_COMPLETE.md — Beta function from lattice (03_derivations)
- DERIV_ALPHA_PRECISION_FORMULA.md — Modular corrections (04_coupling)
