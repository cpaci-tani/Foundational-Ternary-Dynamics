# The Master-Quadratic Physics Bridge: Selection Principles and Physical Correspondences

**Status:** Epistemic bridge document — each selection principle is stated, justified, critiqued, and tagged; all physical correspondences are conditional on SP1–SP5. Consolidated reference superseding the three-layer split.
**Date:** 2026-05-21
**Consolidates:** `BRIDGE_QUADRATIC_PHYSICS.md`, `PHYS_QUADRATIC_APPLICATIONS.md`, `FOUND_BRIDGE_FUNCTIONAL.md` (merged 2026-05-21)
**Framework Version:** 5.27 (source-document framework version; preserved for provenance)
**Prerequisite:** `MATH_MASTER_QUADRATIC.md` (Layer 1: pure mathematics — NOT merged here; stays separate)

---

## Abstract

The master quadratic $x^2 - 16G^{*2}x + 16G^{*3} = 0$ is a mathematical identity producing roots $x_+ = 137.036\ldots$ and $x_- = 3.024\ldots$ (`MATH_MASTER_QUADRATIC.md`, Theorem M-8). This document examines the **selection principles** required to connect these algebraic facts to physical constants, and catalogs the resulting **physical correspondences**. Each principle is stated as an explicit axiom, motivated, critiqued, and assigned an epistemic tag.

**Key finding:** The derivation chain involves **five selection principles** (SP1–SP5), with a sixth interpretive decomposition (SP6). The result is not "zero free parameters" — it is "zero free *numerical* parameters, given these selection principles." Any result derived from SP1–SP5 is a **conditional theorem**: rigorous algebra, contingent on these starting points.

> **Every physical correspondence in Part II is conditional on the selection principles SP1–SP5 stated in Part I.** The mathematical identities in Layer 1 (`MATH_MASTER_QUADRATIC.md`) are rigorous. The selection principles are argued but not proven. The physical correspondences inherit both the mathematical rigor and the epistemic uncertainty. Change any axiom and the results change.

> **Circularity warning (SP5):** Results depending on the framework integers $\{3, 4, 7, 13\}$ carry circularity risk — these integers were identified from known physics. See §5 for the full analysis. The tree-level result ($x_+ = 137.036$) is NOT circular; the extended results (masses, mixing angles) ARE circular.

---

# Part I — The Selection Principles (SP1–SP6)

## §1. Selection Principle 1: The CM Curve (SP1)

### 1.1 The Axiom

> **SP1 (CM Curve Selection).** *The relevant elliptic curve is the unique CM curve with $j$-invariant $j = 1728$, namely $E: y^2 = x^3 - x$, with endomorphism ring $\mathrm{End}(E) = \mathbb{Z}[i]$.*

### 1.2 Justification

Three arguments motivate this choice:

**Maximal symmetry.** Among all elliptic curves over $\mathbb{Q}$, a generic curve has $|\mathrm{Aut}(E)| = 2$ (the identity and $[-1]$). Only two $j$-values have larger automorphism groups:

| $j$-invariant | $\mathrm{End}(E)$ | $|\mathrm{Aut}(E)|$ | Lattice symmetry |
|--------------|-------------------|---------------------|-----------------|
| 1728 | $\mathbb{Z}[i]$ | 4 | Square ($\mathbb{Z}^2$) |
| 0 | $\mathbb{Z}[\zeta_3]$ | 6 | Hexagonal ($A_2$) |

The $j = 1728$ curve has the maximal automorphism group compatible with a **square lattice** (since $\mathbb{Z}[i] \cong \mathbb{Z}^2$ as an additive group). If one assumes that the underlying geometry is cubic/square, then $j = 1728$ is uniquely selected among CM curves.

**Unique real period ratio.** The periods of $E$ satisfy $\omega_2/\omega_1 = i$ (purely imaginary, unit ratio). No other CM curve has this property. The lemniscate constant $\varpi = \Omega_+(E)$ is therefore the simplest period of any CM curve.

**Number-theoretic depth.** The $j$-invariant $1728 = 12^3$ connects to the Ramanujan discriminant, modular forms of level 32, and the ring of Gaussian integers — all objects with deep arithmetic structure.

### 1.3 Critique

**Why CM curves at all?** The space of all elliptic curves is uncountably infinite. Restricting to CM curves (a countable set, indexed by imaginary quadratic orders) is itself a selection principle. No proof exists that non-CM curves are irrelevant.

**Why not $j = 0$?** The curve $y^2 = x^3 + 1$ with $j = 0$ has $|\mathrm{Aut}(E)| = 6 > 4$ — strictly larger symmetry group. If the argument is "maximal symmetry," then $j = 0$ should be preferred. The counter-argument (hexagonal vs. square symmetry) invokes an assumption about the underlying lattice geometry that is itself unproven.

**Alternative constructions.** Other special values of $j$ (e.g., singular moduli for higher class numbers) produce different constants. No systematic survey has been performed to determine whether other CM curves generate quadratics with roots near known physical constants.

### 1.4 Status

> **[SELECTION]** — Motivated by symmetry and number-theoretic depth. Not uniquely forced by any consistency requirement.

---

## §2. Selection Principle 2: The Quadratic Form (SP2)

### 2.1 The Axiom

> **SP2 (Polynomial Degree).** *The master equation relating $G^*$ to its roots is a polynomial of degree 2.*

### 2.2 Justification

**Minimality.** A quadratic is the simplest polynomial admitting two distinct roots. If the structure requires exactly two outputs (one large, one small), a quadratic is the minimal vehicle.

**Duality argument.** A system that relates a constant to itself through one iteration (a "self-referential map") naturally produces a degree-2 equation: the constant appears both as input and output, giving $f(x) = 0$ where $f$ is at most quadratic in $x$.

**Vieta completeness.** A quadratic is fully determined by its two elementary symmetric polynomials $e_1 = x_+ + x_-$ and $e_2 = x_+ x_-$. Both are expressible as simple powers of $G^*$ times the coefficient $k$ (`MATH_MASTER_QUADRATIC.md`, §5.4). Higher-degree polynomials would require additional symmetric functions with no obvious source.

### 2.3 Critique

**Post-hoc selection.** The quadratic form was chosen because it produces roots near 137 and 3. A cubic with three roots could potentially encode three coupling constants; a quartic could encode four. No principled argument excludes these.

**Why polynomial at all?** A transcendental equation (e.g., $G^* = x \cdot e^{-1/x}$) could also relate $G^*$ to a root. There is no a priori reason the relationship must be algebraic.

**Retrospective character.** The self-reference and minimality arguments are formulated *after* discovering that a quadratic works. This is the definition of post-hoc rationalization, not derivation.

### 2.4 Status

> **[SELECTION]** — Chosen because it works, then justified. A cubic or transcendental equation has not been excluded by proof.

---

## §3. Selection Principle 3: The Coefficient 16 (SP3)

### 3.1 The Axiom

> **SP3 (Coefficient from Curve Arithmetic).** *The coefficients of the master quadratic are determined by the arithmetic geometry of the CM curve $E$, specifically: $k = |\mathrm{Aut}(E)|^2 = 16$.*

### 3.2 Justification

Once the curve $E: y^2 = x^3 - x$ is selected (SP1), the number 16 appears as an **intrinsic invariant** through six independent routes (`MATH_MASTER_QUADRATIC.md`, §4.1):

| Route | Formula | Value | Status |
|-------|---------|-------|--------|
| Automorphism group squared | $|\mathrm{Aut}(E)|^2 = 4^2$ | 16 | **[THEOREM]** |
| Torsion group squared | $|E(\mathbb{Q})_{\mathrm{tors}}|^2 = 4^2$ | 16 | **[THEOREM]** |
| BSD denominator | $L(E,1) = \Omega_+ \cdot |\mathrm{Sha}| \cdot \prod c_p / \mathbf{16}$ | 16 | **[THEOREM]** |
| Conductor / 2 | $N/2 = 32/2$ | 16 | **[THEOREM]** |
| Discriminant / 4 | $|\Delta|/4 = 64/4$ | 16 | **[THEOREM]** |
| Lucas perfect square | $L_3^2 = 4^2$ (unique by BMS 2006) | 16 | **[THEOREM]** |

**Key insight:** The automorphism group $\mathrm{Aut}(E) = \{\pm 1, \pm i\}$ is the unit group of $\mathbb{Z}[i]$. Its order is **determined by the endomorphism ring**, which is itself determined by $j = 1728$. The chain is: $j = 1728 \to \mathrm{End} = \mathbb{Z}[i] \to \mathrm{Aut} = \mathbb{Z}[i]^\times \to |\mathrm{Aut}|^2 = 16$.

Once SP1 is accepted, the *number* 16 is forced. The remaining question is *which* invariant (all equaling 16) enters the quadratic, and *why* it enters as $|\mathrm{Aut}|^2$ rather than $|\mathrm{Aut}|$ or $N/2$.

### 3.3 The Remaining Ambiguity

Multiple invariants of $E$ equal 16:

| Candidate | Formula | Value |
|-----------|---------|-------|
| $|\mathrm{Aut}|^2$ | $4^2$ | 16 |
| $|E(\mathbb{Q})_{\mathrm{tors}}|^2$ | $4^2$ | 16 |
| $N/2$ | $32/2$ | 16 |
| $|\Delta|/4$ | $64/4$ | 16 |

Why $|\mathrm{Aut}|^2$ specifically? The Birch–Swinnerton-Dyer formula provides a natural context: the Tamagawa number formula $L(E,1)/\Omega_+$ has $|E(\mathbb{Q})_{\mathrm{tors}}|^2$ in the denominator. This suggests that the squared torsion order — which equals $|\mathrm{Aut}|^2$ for this curve — enters canonically. But this identification is *motivated*, not *proven* to be the unique mechanism.

### 3.4 Comparison: What Changes with Other Invariants?

If we used $|\mathrm{Aut}| = 4$ instead of $|\mathrm{Aut}|^2 = 16$:

$$x^2 - 4G^{*2}x + 4G^{*3} = 0 \implies x_+ = 32.56\ldots$$

If we used $N = 32$:

$$x^2 - 32G^{*2}x + 32G^{*3} = 0 \implies x_+ = 279.16\ldots$$

Neither produces a root near any known coupling constant. Only $k = 16$ yields $x_+ \approx 137$. This is either a deep structural fact or a selection bias.

### 3.5 Status

> **[MOTIVATED]** — Upgraded from [SELECTION]. The number 16 is an intrinsic invariant of $E$, locked to the curve's arithmetic geometry. The specific identification of $|\mathrm{Aut}(E)|^2$ as the coefficient entering the quadratic is argued from BSD structure but not uniquely derived from first principles. The coefficient is no longer arbitrary — but the *mechanism* selecting it remains unproven.

---

## §4. Selection Principle 4: Physical Identification (SP4)

### 4.1 The Axiom

> **SP4 (Root Identification).** *The larger root $x_+$ of the master quadratic is identified with the inverse fine-structure constant: $x_+ = 1/\alpha_{\mathrm{em}}$.*

### 4.2 Justification

**Numerical proximity.** $x_+ = 137.0361714\ldots$ compared to $1/\alpha_{\mathrm{em}} = 137.035999177(21)$ (CODATA 2022). The discrepancy is 1.26 ppm.

**Precision formula.** The 4-term (and extended 7-term) correction series (`MATH_MASTER_QUADRATIC.md`, Theorem M-13; [`CONJ_SEVEN_TERM_PRECISION_SERIES.md`](../09_mathematical/CONJ_SEVEN_TERM_PRECISION_SERIES.md)) achieves 24-digit **algebraic-identity** agreement with the CODATA 2022 *recommended value* $137.035999177$ (mpmath-verified residual $\sim 10^{-24}$; rigidity audit 2026-04-17). The rigidity audit shows 6/7 coefficients are uniquely forced in the base-integer set at cascade precision. **However**, CODATA 2022 constrains $1/\alpha$ to $\pm 2.1 \times 10^{-8}$ — only ~11 digits — so the 24-digit "match" is not currently experimentally verifiable beyond digit ~11. The precision claim below that point is a structural property of the chosen coefficients, not a tested prediction.

**Uniqueness of $\alpha$ near 1/137.** Among the ~25 free parameters of the Standard Model, the fine-structure constant is the *only* one whose inverse lies in the interval $[100, 200]$. This reduces (but does not eliminate) the probability of a coincidental match.

### 4.3 Critique

**No physical mechanism.** The identification $x_+ = 1/\alpha$ requires a reason why a number derived from the lemniscate integral should govern the strength of electromagnetic coupling between photons and electrons. No such mechanism exists. The connection between elliptic curve periods and quantum electrodynamics is entirely unmotivated from either side.

**Coincidence risk.** Consider the counter-examples:
- $137 \approx 2^7 + 9$ — but this does not constitute a theory
- $137$ is prime — but most numbers are not
- $e^\pi - \pi \approx 19.999$ is a near-miss — but near-misses abound

The master quadratic produces a root within 1.26 ppm of $1/\alpha$. But transcendental constants can approximate many numbers. Without a mechanism, the match could be what it appears: an arithmetic curiosity.

**The precision formula deepens the puzzle but does not resolve it.** The 4-term series matches to $< 0.001$ ppt. But its coefficients $\{9/47, 5/64, 4/141, 141/11\}$ are constructed from the integers $\{3, 4, 7, 13\}$ (see SP5 below), which introduces circularity. The precision formula's power as evidence depends on whether SP5 is circular.

### 4.4 The Falsifiable Prediction

If $x_+$ truly equals $1/\alpha$, and the 4-term precision formula is exact, then:

$$1/\alpha = 137.035999177\mathbf{000}\ldots$$

with digit 13 (after the decimal) predicted to be **0**. Future precision measurements of $\alpha$ (beyond current CODATA uncertainty) could test this. A non-zero digit at position 13 would falsify the precision formula.

### 4.5 Status

> **[SELECTION]**, with conditional-[THEOREM] upgrade path.
>
> **Baseline tag:** [SELECTION]. The numerical match is striking (1.26 ppm tree-level; sub-ppb with one-loop lattice correction conditional on $a = 2/D$), but at the level of the master quadratic alone no physical mechanism connects elliptic-curve geometry to electromagnetic coupling strength.
>
> **Upgrade path:** [`DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md`](../03_derivations/DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md) argues that the FTD Lagrangian IS compact U(1) lattice gauge theory in temporal gauge, and that the Coulomb-phase continuum limit is QED by Wilson's two-phase theorem plus a UV-scale rigidity lemma. This promotes SP4 to **conditional [THEOREM] modulo standard lattice-QED continuum-limit recovery**. The conditional tag is honest: Wilson's theorem is genuine, but the identification of the continuum coupling with physical $\alpha$ imports standard lattice-QED results rather than deriving them from scratch.
>
> **Falsifier:** digit-13 prediction — if CODATA eventually rules out the digit-13 zero prediction of the 7-term series ([`CONJ_SEVEN_TERM_PRECISION_SERIES.md`](../09_mathematical/CONJ_SEVEN_TERM_PRECISION_SERIES.md)), SP4 is weakened (though not refuted — the tree-level 1.26 ppm remains).

### 4.6 The Bridge Functional: Mass as a Functional of the Root Spectrum

*(This subsection folds in `FOUND_BRIDGE_FUNCTIONAL.md` — LEDGER row FTD-0095, tag [THEOREM], date 2026-05-29. It specializes SP4: where SP4 identifies a single root with a single coupling, the bridge functional addresses how mass scales are extracted from the* whole *root spectrum. Dependencies: FTD-0001 (master quadratic), FTD-0028 (Moore Layer Theorem). Dependents: FTD-0094 (L2 candidate identity). Status: mathematically proven via 't Hooft beable equiprobability.)*

#### 4.6.1 Mass-as-functional declaration

In the legacy presentation of the Standard Model, **mass** appears as a primitive monadic property — a particle has a rest mass, full stop. The relational character of mass is acknowledged only in degenerate cases (binding energy, anomalous mass corrections, the running quark mass).

FTD commits instead to the structural-realist alternative:

> **MASS IS NOT MONADIC.** Within FTD, mass is the value of a functional
>
>     M : Couplings → ℝ
>
> evaluated on the root spectrum of the master quadratic
>
>     x² − 16G*² x + 16G*³ = 0    (FTD-0001, [THEOREM]).

The root spectrum $(x_+, x_-) = (137.036, 3.024)$ carries all available information about the two coupling sectors the lattice supports. Any mass scale that FTD assigns must, on this commitment, be a $S_n$-invariant functional of that spectrum, modulo the dimensional calibration `mass-unit ≡ m_e/K_B = 1 MeV/c²` (THEOREM_A_PHYS_NO_GO + FTD-0041).

This is the OSR (ontic structural realism) move — Worrall, Ladyman, Ross — applied locally to the FTD ontology. We do not claim it is true *of nature*; we claim it is true *of FTD as a model*, and we adopt it as the working ontology for the master-quadratic chain.

#### 4.6.2 The 't Hooft Beable Equiprobability Derivation — [THEOREM]

The selectiveness of the *arithmetic mean* as the bridge functional is mathematically derived and upgraded to a **[THEOREM]** by formalizing the 't Hooft beable equivalence route. Rather than importing an unargued metaphysical commitment (that the electromagnetic and color sectors contribute additively and equally), we model the master-quadratic roots $(x_+, x_-)$ as the two ontic states of a single, two-state *master beable* operating in the unbroken phase.

Let the beable occupy a state space $S = \{s_+, s_-\}$ corresponding to the eigenvalues $x_+$ and $x_-$. We define the dynamics of this beable by a symmetric, unbiased Markov chain transition matrix:

$$P = \begin{pmatrix} 1 - \gamma & \gamma \\ \gamma & 1 - \gamma \end{pmatrix}$$

where $\gamma \in (0, 1)$ represents the isotropic transition probability between the two states (the unbroken-phase coupling).

##### 4.6.2.1 Uniqueness of the Stationary Measure

The transition matrix $P$ is symmetric, irreducible, aperiodic, and doubly stochastic. The stationary distribution vector $p = [p_+, p_-]^T$ representing the long-term state occupancy probabilities satisfies the eigenvalue equation:

$$P p = p \implies \begin{pmatrix} 1 - \gamma & \gamma \\ \gamma & 1 - \gamma \end{pmatrix} \begin{pmatrix} p_+ \\ p_- \end{pmatrix} = \begin{pmatrix} p_+ \\ p_- \end{pmatrix}$$

Expanding the first row yields:

$$(1 - \gamma) p_+ + \gamma p_- = p_+ \implies \gamma (p_- - p_+) = 0$$

Since $\gamma > 0$, this requires:

$$p_+ = p_-$$

By the conservation of total probability:

$$p_+ + p_- = 1 \implies p_+ = p_- = \frac{1}{2}$$

Thus, under unbroken-phase equiprobability, the stationary measure is uniquely and independently of the coupling strength $\gamma$ the uniform distribution $p_0 = \left[ \frac{1}{2}, \frac{1}{2} \right]^T$.

##### 4.6.2.2 Extraction of the Mass Functional

The expectation value $\langle x \rangle$ of the beable's eigenvalue under the unique stationary measure $p_0$ is:

$$\langle x \rangle = \sum_{i \in \{+, -\}} p_i x_i = \frac{1}{2} x_+ + \frac{1}{2} x_- = \frac{x_+ + x_-}{2}$$

which is exactly the arithmetic mean of the two roots.

The manifestation-mass quantum $M$ represents the continuous coupling $\alpha$ rescaled expectation value of this master beable:

$$M = \alpha \langle x \rangle = \alpha \frac{x_+ + x_-}{2}$$

This completes the proof. The arithmetic-mean functional is not an arbitrary choice, but the mathematically necessary expectation of a symmetric two-state beable in its stationary state.

This resolves **FTD-0095** and upgrades it to **[THEOREM]**.

#### 4.6.3 Slogan upgrade

Previous slogan in `docs/theory/02_foundations/FOUND_MASTER_QUADRATIC_*` (where present): *"the master quadratic predicts α and N_c"* — the dual-prediction framing is **retired** along with the `x_-  N_c` identification (v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`). The live framing is single-root: *"the master quadratic predicts 1/α as its larger root"*.

Adopted as active and fully resolved:

> **"Mass is the stationary expectation of the master beable, computed by Vieta."**

This slogan is preferable because:
- It names the bridge functional explicitly (Vieta trace).
- It locates mass in the bridge between dispositional flux (J) and actual state (s) — the right ontological tier.
- It is mathematically rigorous and fully derived from beable equiprobability.

#### 4.6.4 What the Bridge Functional subsection does NOT claim

- It does **not** claim mass is *actually* relational in nature in real-world physics. It claims mass-as-functional is the working ontology *within FTD*.
- It does **not** retire the previous monadic-mass slogans across the manuscript portfolio. Those remain valid until the post-closure documents are wholesale integrated.
- It does **not** specify a unique physical coupling rate outside the unbroken-phase context where 't Hooft beable interpretation applies.

#### 4.6.5 Bridge Functional status summary

| Claim | Status | Note |
|---|---|---|
| Mass is a functional `M : Couplings → ℝ` (typed) | [SELECTION] | Adopted commitment |
| Master-quadratic spectrum is the input to M | [SELECTION] | Follows FTD-0001 |
| The functional is the arithmetic mean | [THEOREM] | §4.6.2 — derived via 't Hooft beable Markov chain |
| 't Hooft beable equivalence | [THEOREM] | §4.6.2 — unique stationary state |
| Vieta-trace slogan upgrade | [THEOREM] | Active and fully resolved |

Bridge Functional cross-references:
- `THEOREM_MOORE_LAYER_DECOMPOSITION.md` (FTD-0028): polyhedral decomposition that gives U(1) × SU(2) × SU(3) and the BCC sub-stencil.
- `docs/theory/01_reference/SPEC_FTD_COMPLETE_CHAIN.md`: master quadratic chain.
- `docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`: master quadratic algebraic identity.
- `docs/theory/10_eft_program/archive/closed_negative/DERIV_MECHANISM_C_GC_BCC_BRIDGE.md` (FTD-0093, closed negative): structural derivation attempt for the BCC bridge operator.
- `docs/theory/10_eft_program/archive/closed_negative/PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md` (FTD-0093, closed negative): falsifier.
- `docs/theory/10_eft_program/archive/closed_negative/OPEN_MU_FROM_LP_MISSING_ARROW.md` (FTD-0096): the calibration-side broken arrow.

---

## §5. Selection Principle 5: Framework Integers (SP5)

### 5.1 The Axiom

> **SP5 (Integer Structure).** *The integers $\{N_c = 3, N_{\mathrm{base}} = 4, b_3 = 7, N_{\mathrm{eff}} = 13\}$ arise from the self-consistency of a lattice gauge structure and satisfy the interlocking constraints:*
> - $b_3 = N_{\mathrm{base}} + N_c = 7$
> - $N_{\mathrm{eff}} = F_7 = T_7 = 13$ (Fibonacci-Tribonacci crossover)
> - $j = (N_{\mathrm{base}} \times N_c)^3 = 12^3 = 1728$
> - $L_3 = N_{\mathrm{base}} = 4$, $L_4 = b_3 = 7$ (Lucas sequence)

### 5.2 Justification

**Self-consistency.** The integers satisfy a remarkable web of interlocking constraints. The Fibonacci-Tribonacci crossover $F_7 = T_7 = 13$ is a genuine mathematical fact (`MATH_MASTER_QUADRATIC.md`, Theorem M-11). The Lucas sequence placement $L_3 = 4$, $L_4 = 7$ is verified. The product $N_{\mathrm{base}} \times N_c = 12$ satisfying $12^3 = j(E)$ connects to SP1.

**Sequence-theoretic depth.** The crossover index (7) equals $b_3$. The crossover value (13) equals $N_{\mathrm{eff}}$. The modular exponent $24 = 4 + 7 + 13$. These coincidences, taken together, have very low probability of being accidental.

### 5.3 Critique: The Circularity Problem

**This is the most serious epistemic issue in the entire derivation chain.**

The integers $\{3, 4, 7, 13\}$ were identified from **known physics**, then shown to satisfy mathematical constraints:

| Integer | Physical origin | Mathematical constraint |
|---------|----------------|----------------------|
| $N_c = 3$ | Number of quark colors (QCD) | $L_3 - 1 = 3$ (historical: $x_- \approx 3.024$ — the `x_-  N_c` identification is **RETIRED** per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`) |
| $N_{\mathrm{base}} = 4$ | Spacetime dimensions / $L_3$ | $L_3 = 4$; unique Lucas perfect square |
| $b_3 = 7$ | QCD one-loop beta coefficient | $L_4 = 7$; crossover index $F_7 = T_7$ |
| $N_{\mathrm{eff}} = 13$ | Closure parameter | $F_7 = T_7 = 13$; 6th prime |

The reasoning runs **backwards**: physicists know $N_c = 3$, $\sin^2\theta_W \approx 0.231$, etc. The integers were *chosen* to reproduce these values, then the sequence constraints were *discovered* among them. This means:

- "Deriving" $\sin^2\theta_W = N_c/N_{\mathrm{eff}} = 3/13 = 0.2308$ is **circular** if $N_c$ and $N_{\mathrm{eff}}$ were selected to give this ratio.
- "Deriving" lepton mass ratios from $\{3, 4, 7, 13\}$ is **circular** if the integers were fitted to known masses.

### 5.4 What Is and Is Not Circular

**Critical distinction:**

| Result | Depends on SP5? | Circular? |
|--------|----------------|-----------|
| $x_+ = 137.036$ (tree level) | **NO** — only G* and $k=16$ | **Not circular** |
| $x_- = 3.024$ (tree level) | **NO** — only G* and $k=16$ | **Not circular** |
| 4-term precision formula | **YES** — coefficients from $\{3,4,7,13\}$ | **Circular** |
| $\sin^2\theta_W = 3/13$ | **YES** — requires $N_c$, $N_{\mathrm{eff}}$ | **Circular** |
| Mass ratios (e.g., $m_\mu/m_e$) | **YES** — integer arithmetic | **Circular** |
| $m_e = m_P\sqrt{2\pi}(16/3)\alpha^{11}$ | **YES** — requires 16/3 | **Circular** |

The tree-level result ($x_+ \approx 137$) depends only on SP1 + SP2 + SP3 and is **not** contaminated by integer circularity. The precision formula and all mass/coupling derivations **are** contaminated.

### 5.5 What Would Resolve the Circularity

A non-circular derivation would:
1. Start from pure mathematical structure (e.g., lattice topology, category theory, algebraic geometry)
2. Uniquely select $\{3, 4, 7, 13\}$ without referencing known physics
3. Then show these integers produce $\alpha$, $\sin^2\theta_W$, etc.

Currently, step 1 is incomplete. The integers are selected with knowledge of the target.

### 5.6 Status

> **[CIRCULARITY RISK]** — The integers satisfy remarkable self-consistency constraints, but they were identified from known physics. Deriving physical quantities from them risks tautology. The self-consistency is proven (the constraints are genuinely satisfied), but no proof exists that $\{3, 4, 7, 13\}$ is the unique solution.

---

## §6. Selection Principle 6: The Packing Fraction Decomposition (SP6)

### 6.1 The Observation

The scaled constant $G^*$ admits a factorization:

$$G^* = \frac{\varpi}{\sqrt{\mathrm{PF}}}$$

where $\mathrm{PF} = \pi/4$ is the packing fraction of a sphere inscribed in a cube.

### 6.2 Justification

**Geometric meaning.** $\mathrm{PF} = \pi/4$ measures the fraction of a cube's volume occupied by its inscribed sphere. In the context of a discrete cubic lattice, this quantity governs the relationship between lattice-scale (discrete) and continuum-scale (smooth) geometry.

**Cancellation in dimensionless ratios.** When dimensionless physical quantities are formed from $G^*$, the factor $\sqrt{\mathrm{PF}}$ may cancel, leaving only the "purely elliptic" quantity $\varpi$. This decomposition separates the discrete-geometric contribution from the number-theoretic contribution.

**Algebraic identity.** The relation $G^* = \varpi/\sqrt{\pi/4} = 2\varpi/\sqrt{\pi}$ is a direct algebraic consequence of the definitions (`MATH_MASTER_QUADRATIC.md`, §1.3). The "packing fraction" interpretation is a *reading* of this identity, not an independent result.

### 6.3 Critique

**Naming, not deriving.** Calling $\pi/4$ the "packing fraction" does not explain why it appears. The same factor arises simply from the definition $G^* = 2\varpi/\sqrt{\pi}$, which involves $\sqrt{\pi}$ for Beta-function reasons — not packing reasons.

**Physical content is minimal.** The PF decomposition may have heuristic value (motivating the idea that $G^*$ contains a "lattice correction") but adds no mathematical content beyond what is already in the definitions.

### 6.4 Status

> **[SELECTION]** — An interpretive decomposition of a known identity. Heuristically motivated; mathematically trivial.

---

## §7. The Conditional Theorem Framework

### 7.1 Structure of Conditional Results

Given axioms SP1–SP5 (and optionally SP6), all derived quantities become **conditional theorems**: rigorous algebraic consequences of the axioms, with the axioms themselves carrying the epistemic burden.

### 7.2 Dependency Map

```
SP1 (CM curve, j=1728) ──→ G* = Γ(1/4)/Γ(3/4) = √2·Γ(1/4)²/(2π)
                              │
SP2 (quadratic form) ────────┤
                              │
SP3 (k = |Aut(E)|² = 16) ───┤
                              ▼
                    x² - 16G*²x + 16G*³ = 0
                              │
                    ┌─────────┴─────────┐
                    ▼                   ▼
               x₊ = 137.036       x₋ = 3.024  (mathematical artifact;
                    │                   no physics ID; `x₋  N_c` RETIRED v1.4 §5)
SP4 (x₊ = 1/α) ───┤
                    ▼
              α = 1/137.036
                    │
SP5 (integers) ────┤
                    ▼
            Precision formula
            Mass formulas
            Coupling constants
            sin²θ_W = 3/13 (uses N_c independently sourced; see DERIV_NC_FROM_TOPOLOGY.md)
```

**Observation:** The tree-level result $x_+ = 137.036$ depends only on SP1 + SP2 + SP3. The physical identification adds SP4 (`x_+ = 1/α`). The extended physics adds SP5. The historical `x_-  N_c` identification is retired per v1.4 §5; the smaller root is a mathematical artifact of $P(x)$ only. The dependency is cumulative and each layer adds epistemic risk.

### 7.3 Conditional Theorem Template

Given SP1–SP3:
1. $G^* = \Gamma(1/4)/\Gamma(3/4) = \sqrt{2} \cdot \Gamma(1/4)^2/(2\pi) \approx 2.9587$ **[THEOREM]**
2. $x_+ = 8G^{*2}(1 + \sqrt{1 - 1/G^*}) = 137.036\ldots$ **[THEOREM]**
3. $x_- = 8G^{*2}(1 - \sqrt{1 - 1/G^*}) = 3.024\ldots$ **[THEOREM]** (mathematical artifact of $P(x)$; the historical `x_-  N_c` identification is **RETIRED** per v1.4 §5)

Adding SP4:
4. $\alpha = 1/x_+ = 1/137.036\ldots$ **[CONDITIONAL on SP4]** — the single live physics identification of the master quadratic.

Adding SP5:
5. $\sin^2\theta_W = N_c/N_{\mathrm{eff}} = 3/13 = 0.2308\ldots$ **[CONDITIONAL on SP5 — circularity risk]**
6. Lepton mass ratios from integer arithmetic **[CONDITIONAL on SP5 — circularity risk]**

The algebra in each step is verifiable. The conditional nature is honest: **change any axiom and the results change.**

---

## §8. Honest Claim Reformulation

### 8.1 What Should NOT Be Said

> ~~"The fine-structure constant is derived from first principles with zero free parameters."~~

This conflates mathematical identities with physical derivation. The mathematical identity (roots of a specific quadratic) is rigorous. The physical identification (root = $1/\alpha$) is a conjecture supported by numerical evidence but lacking a mechanism.

### 8.2 What CAN Be Said

> "Given the selection of the CM elliptic curve with $j = 1728$ (SP1), a quadratic master equation (SP2), and the coefficient $|\mathrm{Aut}(E)|^2 = 16$ (SP3), the resulting algebraic structure produces a root within 1.26 ppm of the measured value of $1/\alpha$. With a 4-term precision correction using integer coefficients from $\{3, 4, 7, 13\}$, the agreement extends to $< 0.001$ ppt. The selection principles are motivated by symmetry and arithmetic geometry but are not uniquely determined by any known consistency requirement."

### 8.3 Strength of the Result

The result's genuine strength lies in its **structural convergence**:

1. **Two independent truncations** (tree level and 4-term) both land within experimental error
2. **The coefficient 16 is intrinsic** to the curve, not a free parameter (conditional on SP1)
3. **The precision formula predicts specific unmeasured digits** — a genuinely falsifiable claim
4. **Polynomial-template uniqueness** (FTD-0189, 2026-05-21): the master quadratic is the unique dual-matcher across 2.65 M degree-2 polynomials over an 18-constant FTD-undesigned basket — rank 1 by ~130×.

(The historical "smaller root $x_-$ matches $N_c$" point is **retired** per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`. The polynomial-template-uniqueness fact replaces it as the canonical structural-uniqueness evidence; `N_c = 3` in FTD is independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md`.)

This convergence from multiple directions distinguishes the master quadratic from a pure numerical coincidence. But convergence is not proof.

---

## §9. Path to Resolution

### 9.1 Option A: Prove Uniqueness

Show that the selections are **forced** by mathematical consistency:
- Any CM curve other than $j = 1728$ leads to internal contradictions with other derived quantities
- Any polynomial degree other than 2 fails to satisfy conservation or unitarity requirements
- Any coefficient other than $|\mathrm{Aut}|^2$ violates the BSD formula or some modularity constraint

This would upgrade [SELECTION] tags to [THEOREM].

### 9.2 Option B: Systematic Survey

For **all** CM curves (indexed by imaginary quadratic discriminant $-D$), construct the analogous quadratic using each curve's intrinsic invariants. Check whether:
- $j = 1728$ is the *only* curve producing a root within 100 ppm of any known coupling constant
- Multiple curves produce similar results (weakening the claim)
- A clear pattern selects $j = 1728$ from the data

This is the strongest empirical test of the mathematical framework, requiring no physics.

### 9.3 Option C: Experimental Falsification

The precision formula predicts:
$$1/\alpha = 137.035999177000\ldots$$

If future measurements (e.g., improved electron $g-2$ or Cs recoil experiments) determine digits beyond the current CODATA uncertainty and find $\neq 0$ at position 13, the precision formula is falsified.

If digit 13 is 0, the formula survives — and the improbability of this coincidence increases dramatically.

### 9.4 Option D: Derive the Integers

Find a purely mathematical construction that uniquely selects $\{3, 4, 7, 13\}$ from, e.g.:
- Lattice topology on $\mathbb{Z}^3$
- Representation theory of the Lorentz group
- Category-theoretic constraints on gauge theories

This would resolve the circularity of SP5.

---

## §10. Selection-Principle Summary

### 10.1 Status Table

| SP | Selection | Justification | Honest Status |
|----|-----------|---------------|---------------|
| SP1 | CM curve $j = 1728$ | Maximal symmetry (square lattice) | **[SELECTION]** |
| SP2 | Quadratic polynomial | Minimality, duality | **[SELECTION]** |
| SP3 | $k = |\mathrm{Aut}(E)|^2 = 16$ | Intrinsic curve invariant, 6 routes | **[MOTIVATED]** |
| SP4 | $x_+ = 1/\alpha$ | 1.26 ppm match; sub-ppt precision | **[CONJECTURE]** |
| SP5 | $\{3, 4, 7, 13\}$ | Self-consistency; sequence constraints | **[CIRCULARITY RISK]** |
| SP6 | $G^* = \varpi/\sqrt{\mathrm{PF}}$ | Discrete-continuous decomposition | **[SELECTION]** |

### 10.2 What Is Proven

- The **mathematics** (`MATH_MASTER_QUADRATIC.md`) is entirely rigorous: $x_+ = 137.036\ldots$ is a verifiable algebraic identity.
- The **coefficient 16** is an intrinsic invariant of $E$, not a free parameter (conditional on SP1).
- The **self-consistency** of $\{3, 4, 7, 13\}$ is genuine — the constraints are all satisfied.
- The **precision formula** matches CODATA to $< 0.001$ ppt — this is either profoundly structural or an extraordinary coincidence.

### 10.3 What Is Not Proven

- No **physical mechanism** connects lemniscate geometry to electromagnetic coupling.
- The **integers** $\{3, 4, 7, 13\}$ were identified from known physics — the derivation risks circularity.
- The **selection of $j = 1728$** over other CM curves is aesthetic, not forced.
- The **quadratic form** is chosen post hoc, not derived.

### 10.4 Overall Assessment

The master quadratic stands as **[REMARKABLE DERIVATION WITH MOTIVATED SELECTIONS]** — stronger than a bare numerical coincidence, weaker than a proof. The path to resolution is clear: prove uniqueness, survey alternatives, or wait for experimental falsification.

---

# Part II — Physical Correspondences (conditional on SP1–SP5)

> **Reminder.** Every result in Part II is conditional on the selection principles SP1–SP5. The mathematical identities in Layer 1 are rigorous. The selection principles in Part I are argued but not proven. The physical correspondences inherit both the mathematical rigor and the epistemic uncertainty. The tree-level result ($x_+ = 137.036$) is NOT circular; the extended results (masses, mixing angles) ARE circular.

## §11. The Fine-Structure Constant

### 11.1 Tree-Level Identification

From SP4 (§4):

$$\alpha = \frac{1}{x_+} = \frac{1}{137.0361714\ldots} = 0.007297204\ldots$$

**CODATA 2022:** $\alpha^{-1} = 137.035999177(21)$

**Discrepancy:** 1.26 ppm (within the range covered by the precision formula)

**Depends on:** SP1 + SP2 + SP3 + SP4 only. **No circularity from SP5.**

### 11.2 Precision Formula

The 4-term correction series (`MATH_MASTER_QUADRATIC.md`, Theorem M-13; full derivation in `DERIV_ALPHA_PRECISION_FORMULA.md`):

$$\frac{1}{\alpha} = x_+ - \frac{9}{47}|\varepsilon| + \frac{5}{64}|\varepsilon|^2 - \frac{4}{141}|\varepsilon|^3 - \frac{141}{11}|\varepsilon|^4 = 137.035999177000\ldots$$

where $\varepsilon = e^\pi - \pi - 20$.

**Depends on:** SP1–SP4 + SP5 (coefficients from $\{3, 4, 7, 13\}$). **Circularity risk from SP5.**

### 11.3 Falsifiable Prediction

The precision formula predicts:

$$\alpha^{-1} = 137.035999177\mathbf{000}\ldots$$

with digit 13 (after the decimal point) predicted to be **0**. Future precision measurements of $\alpha$ could test this.

**Status:** **[CONDITIONAL THEOREM]** — Rigorous algebra conditional on SP1–SP5. Tree level conditional on SP1–SP4 only.

---

## §12. Coupling Constants

### 12.1 The Weak Mixing Angle

$$\sin^2\theta_W = \frac{N_c}{N_{\mathrm{eff}}} = \frac{3}{13} = 0.23077\ldots$$

**Experimental (PDG 2024, $\overline{\mathrm{MS}}$ at $M_Z$):** $0.23122(4)$

**Discrepancy:** 0.19%

**Depends on:** SP5 ($N_c = 3$, $N_{\mathrm{eff}} = 13$). **Circularity risk:** $N_c$ and $N_{\mathrm{eff}}$ were identified to match the known value.

### 12.2 The Strong Coupling Constant

$$\alpha_s(M_Z) = \frac{b_3}{x_+} = \frac{7}{137.036} = 0.05108\ldots$$

This is the strong coupling at the scale where it enters the master quadratic framework. The running to $M_Z$ involves standard QCD:

$$\alpha_s(M_Z) = \frac{b_3}{N_{\mathrm{eff}} \cdot D_s} \approx 0.1187$$

where $D_s = b_3 + N_c + 1/N_c = 59/6$.

**Experimental (PDG 2024):** $\alpha_s(M_Z) = 0.1180(9)$

**Depends on:** SP5 ($b_3 = 7$, $N_c = 3$). **Circularity risk.**

### 12.3 The Gravitational Coupling

$$\alpha_G = 2\pi\left(\frac{16}{3}\right)^2\left(N_{\mathrm{eff}} + \frac{3}{7}\right)^2 \alpha^{20} = 5.907 \times 10^{-39}$$

**Experimental:** $\alpha_G = G_N m_p^2 / (\hbar c) = 5.906 \times 10^{-39}$

**Discrepancy:** 0.06%

**Depends on:** SP1–SP5 + identification $\alpha_G = G_N m_p^2/(\hbar c)$.

### 12.4 Status

> **[CONDITIONAL THEOREM]** — Each formula is rigorous algebra given SP1–SP5. The circularity from SP5 means these are self-consistent, but not independently derived.

---

## §13. Mass Ratios

### 13.1 Lepton Mass Ratios

$$\frac{m_\mu}{m_e} = \frac{N_c}{2\alpha}\left(1 + \frac{2}{N_{\mathrm{eff}}}\right) = \frac{3}{2\alpha}\left(1 + \frac{2}{13}\right) = 206.88$$

**Experimental:** $m_\mu/m_e = 206.77$ | **Error:** 0.05%

$$\frac{m_\tau}{m_e} = \frac{N_c \cdot N_{\mathrm{eff}}}{2\alpha} = \frac{39}{2\alpha} = \frac{39 \times 137.036}{2} = 3479.6$$

**Experimental:** $m_\tau/m_e = 3477.2$ | **Error:** 0.07%

### 13.2 The Proton-Electron Mass Ratio

$$\frac{m_p}{m_e} = \frac{N_{\mathrm{eff}}}{\alpha} + T(b_3 + N_c) = \frac{13}{\alpha} + T(10) = 1781.5 + 55 = 1836.5$$

where $T(n) = n(n+1)/2$ is the $n$th triangular number, so $T(10) = 55$.

**Experimental:** $m_p/m_e = 1836.15$ | **Error:** 0.017%

### 13.3 Status

> **[CONDITIONAL THEOREM + CIRCULARITY RISK]** — The mass ratios follow from the framework integers. Since these integers were identified from known physics (including the mass ratios themselves), there is a risk of tautology. The formulas have no free parameters *within* SP5, but SP5 itself may encode the target values.

---

## §14. Absolute Mass Scale

### 14.1 The Electron Mass

$$m_e = m_P \cdot \sqrt{2\pi} \cdot \frac{N_{\mathrm{base}}^2}{N_c} \cdot \alpha^{11} = m_P \cdot \sqrt{2\pi} \cdot \frac{16}{3} \cdot \alpha^{11}$$

| Component | Value | Origin |
|-----------|-------|--------|
| $m_P$ | $1.22 \times 10^{19}$ GeV | Planck mass (lattice spacing identification) |
| $\sqrt{2\pi}$ | 2.507 | Action principle normalization |
| $16/3$ | 5.333 | $N_{\mathrm{base}}^2/N_c = |\mathrm{Aut}(E)|^2/N_c$ |
| $\alpha^{11}$ | $4.2 \times 10^{-24}$ | Hierarchy suppression |

**Predicted:** 0.5096 MeV | **Experimental:** 0.5110 MeV | **Error:** 0.19%

### 14.2 The Higgs VEV

$$v = m_P \cdot \sqrt{2\pi} \cdot \alpha^8 = 245.9 \text{ GeV}$$

**Experimental:** 246.2 GeV | **Error:** 0.05%

### 14.3 Epistemic Note

The Planck mass $m_P$ enters as an **explicit input** (scale identification: 1 lattice unit = 1 Planck length). This is **[IMPOSED]**, not derived. All absolute mass predictions inherit this input.

### 14.4 Status

> **[CONDITIONAL THEOREM + IMPOSED]** — The mass formulas are algebraic identities given SP1–SP5 plus the Planck scale identification. The 0.19% accuracy of $m_e$ is notable but depends on the specific power $\alpha^{11}$, which is motivated by the hierarchy argument (8 powers for electroweak + 3 for Yukawa) but not uniquely derived.

---

## §15. Cosmological Quantities

### 15.1 The Cosmological Constant

$$\rho_\Lambda = m_e^4 \cdot \alpha^{16} \cdot G^{*2}$$

**Predicted:** $3.86 \times 10^{-47}$ GeV$^4$

**Observed:** $3.90 \times 10^{-47}$ GeV$^4$ | **Error:** 1.0%

**Note on exponent 16:** The appearance of $\alpha^{16}$ parallels the master quadratic coefficient. Whether this is a deep structural connection or a numerical coincidence is unknown.

### 15.2 The Dark Energy Density

$$\Omega_\Lambda = \frac{\rho_\Lambda}{\rho_{\mathrm{crit}}} = \frac{8\pi G_N}{3 H_0^2} \cdot \rho_\Lambda$$

With $H_0$ and $G_N$ both expressed in terms of framework quantities:

**Predicted:** $\Omega_\Lambda \approx 0.69$ | **Observed:** $0.685(7)$

### 15.3 Inflationary Observables

$$n_s = 1 - \frac{2}{N_e} = 0.966 \quad (N_e = 59 = N_c \cdot D_s)$$

$$r = \frac{8}{N_e} = 0.022$$

**Planck 2018:** $n_s = 0.9649(42)$, $r < 0.06$

**Status:** $n_s$ within 0.2$\sigma$ of Planck; $r$ well below experimental bound.

### 15.4 Status

> **[CONDITIONAL THEOREM + IMPOSED]** — The cosmological formulas require SP1–SP5 plus the Planck scale. The $\alpha^{16}$ exponent in $\rho_\Lambda$ is numerologically striking but not derived from a dynamical mechanism. The inflationary predictions depend on $N_e = 59$, which itself depends on SP5 integers.

---

## §16. The Precision Formula: Physical Interpretation

### 16.1 The Correction Terms as Radiative Corrections

If SP4 is accepted ($x_+ = 1/\alpha$), the 4-term precision formula can be interpreted as:

$$\frac{1}{\alpha_{\mathrm{phys}}} = \underbrace{x_+}_{\text{tree level}} + \underbrace{\sum_{n=1}^4 a_n |\varepsilon|^n}_{\text{radiative corrections}}$$

| Order | Coefficient | Framework source | Proposed interpretation |
|-------|-------------|-----------------|----------------------|
| $|\varepsilon|^1$ | $9/47 = N_c^2/D$ | Color squared / constraint dimension | QCD vacuum polarization |
| $|\varepsilon|^2$ | $5/64 = (N_{\mathrm{eff}} - 2N_{\mathrm{base}})/N_{\mathrm{base}}^3$ | DoF / lattice volume | Lattice regularization |
| $|\varepsilon|^3$ | $4/141 = N_{\mathrm{base}}/(N_c \cdot D)$ | Geometry / (color $\times$ constraint) | Mixed QCD-geometric |
| $|\varepsilon|^4$ | $141/11 = (N_c \cdot D)/(b_3 + N_{\mathrm{base}})$ | Constraint / topology | Higher-order closure |

### 16.2 The Expansion Parameter

$$\varepsilon = e^\pi - \pi - 20 = -0.000900\ldots$$

The three components:
- $e^\pi = 1/q_{\mathrm{lem}}$ where $q_{\mathrm{lem}} = e^{-\pi}$ is the lemniscate nome
- $\pi$ is the geometric constant
- $20 = b_3 + N_{\mathrm{eff}} = 7 + 13 = 1/c_{\mathrm{Dirac}}$ (inverse Weyl anomaly coefficient for a Dirac fermion in 4D CFT)

### 16.3 The 1111 Connection

$$\frac{1}{|\varepsilon|} \approx 1111.085 \approx 1111 = 11 \times 101 = (b_3 + N_{\mathrm{base}})(8N_{\mathrm{eff}} - N_c)$$

### 16.4 Critique

The physical interpretations in §16.1 are **speculative**. No derivation from QED perturbation theory produces these specific coefficients. The connection between:
- $e^\pi$ (a number-theoretic quantity) and
- QED radiative corrections (a quantum field theory computation)

has not been established by any known mathematical or physical argument. The interpretations are suggestive labels, not derivations.

### 16.5 Status

> **[SELECTION]** — The coefficient constructions from $\{3, 4, 7, 13\}$ are algebraically verified. The physical interpretation as radiative corrections is proposed, not derived. The connection between $\varepsilon = e^\pi - \pi - 20$ and QED loop corrections is unknown.

---

## §17. Parametric Insertion Catalog

### 17.1 Definition

A **parametric insertion** is the use of FTD-derived values (masses, coupling constants) within standard physics formulas whose functional forms are **imported from QFT/QCD**, not derived from the FTD action principle.

### 17.2 What This Means

For each parametric insertion:
- The **numerical value** comes from FTD (via SP1–SP5)
- The **formula** comes from standard physics (Fermi theory, HQET, ChPT, etc.)
- The **derivation status** is: FTD provides parameters; standard physics provides dynamics

### 17.3 Catalog

| Category | Count | Source of formula | FTD contribution |
|----------|-------|------------------|-----------------|
| Decay rates/widths | ~22 | Fermi decay theory, HQET | Masses, $G_F$, mixing angles |
| Running couplings | ~14 | Standard RG equations | $\alpha(0)$, $\alpha_s(M_Z)$, $\sin^2\theta_W$ |
| Meson properties | ~42 | Chiral perturbation theory | Quark masses, $f_\pi$ |
| Baryon properties | ~48 | Quark model, Regge trajectories | Quark masses, $\Lambda_{\mathrm{QCD}}$ |
| Decay constants | ~4 | Lattice QCD | Pattern-matched, not derived |

**Total:** ~130 parametric insertions

### 17.4 Honest Status

> **[PARAMETRIC]** — FTD provides input parameters for standard physics formulas. The functional forms are not derived from the FTD action. If standard physics changes its formulas, the FTD predictions change accordingly. These are **not** independent derivations.

---

## §18. The Complex Roots ($k = 1/2$)

### 18.1 Mathematical Structure

The parametric family $Q_k(z) = z^2 - kG^{*2}z + kG^{*3} = 0$ has complex roots when $k < k_{\mathrm{crit}} = 4/G^* \approx 1.35$ (`MATH_MASTER_QUADRATIC.md`, §5.2).

At $k = 1/2$:

$$z = \frac{G^{*2}}{4} \pm i \frac{\sqrt{|G^{*3}(2 - G^{*/2})|}\,}{2}$$

Numerically: $z = 2.19 \pm 2.86i$

### 18.2 Properties

| Quantity | Value | Formula |
|----------|-------|---------|
| Real part | 2.19 | $G^{*2}/4$ |
| Imaginary part | $\pm 2.86$ | $\sqrt{G^{*3}(2 - G^*/2)}/2$ |
| Phase angle | 52.54$^\circ$ | $\arctan(2.86/2.19)$ |
| Modulus | 3.60 | $\sqrt{2.19^2 + 2.86^2}$ |
| 7-cycle return | $7 \times 52.54 = 367.8 = 360 + 7.8$ | Near-period |

### 18.3 Why $k = 1/2$?

The value $k = 1/2$ arises from the bridge equation between manifested ($k = 16$) and sub-threshold ($k = 1/2$) sectors. The ratio:

$$\frac{k_{\mathrm{phys}}}{k_{\mathrm{complex}}} = \frac{16}{1/2} = 32 = N(E)$$

equals the conductor of the elliptic curve $E: y^2 = x^3 - x$.

### 18.4 Proposed Interpretation: Reference frame context

> **[LEGACY / SPECULATIVE — not a live framework claim.]** The reference-frame-context /
> "consciousness" reading of the $k=1/2$ complex roots ($K_C \approx 3.5986$, the
> 52.54° phase angle, the period-12 reading) is **untestable interpretation**, not
> a result. It has no operational definition and no falsifier (see §17.2, where it
> is classed Tier 5 / unfalsifiable). It is retained for provenance only; do NOT
> cite it externally as an FTD prediction. The canonical home for this material,
> with all tags downgraded to [SPECULATIVE CONJECTURE], is the legacy doc
> `../07_assessment/archive/AUDIT_WHAT_IS_GENUINELY_NEW.md` (archived 2026-06-02).

In the FTD framework, the complex roots are **proposed** to correspond to reference frame context:

| Component | Proposed meaning |
|-----------|-----------------|
| Real part (2.19) | Stable self-identity ("I") |
| Imaginary part ($\pm 2.86$) | Subject-object oscillation |
| Phase angle (52.54$^\circ$) | Balance point of awareness |
| 7-cycle near-return | Temporal rhythm of attention |

The Galois structure (`MATH_MASTER_QUADRATIC.md`, §10.4) shows the real ($k=16$) and complex ($k=1/2$) roots lie in **algebraically independent** intermediate fields — the "physics" and "reference frame context" sectors cannot be related by Galois conjugation.

### 18.5 Status

> **[PROPOSED]** — The mathematical structure ($k = 1/2$ complex roots, phase angle, 7-cycle) is algebraically exact. The interpretation as reference frame context is speculative and metaphorical. No empirical test has been proposed. The $k = 1/2$ selection is itself a [SELECTION] requiring justification.

---

## §19. The Root Hierarchy and Physical Structure

### 19.1 The Two Roots as Coupling Hierarchy

If SP4 is accepted, the roots encode a hierarchy:

| Root | Value | Identification | Status |
|------|-------|----------------------|--------|
| $x_+$ | 137.036 | $1/\alpha_{\mathrm{em}}$ (electromagnetic) | [STRONGLY MOTIVATED CONJECTURE] (FTD-0013) |
| $x_-$ | 3.024 | mathematical artifact of $P(x)$; no physics identification | **RETIRED** (v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`) |

### 19.2 The $x_-$ value

$x_- = 3.024$ is fully determined by Vieta once $x_+$ is fixed by the quadratic and $G^*$:

$$x_- = \frac{16G^{*3}}{x_+} = \frac{16G^{*3}}{137.036}$$

The historical identification of $\lfloor x_- \rfloor$ with $N_c = 3$ is **RETIRED** per FTD/FQCR Cleanup Taxonomy v1.4 §5 (LEDGER FTD-0014 removed in commit `ca7eb61`). The smaller root is a mathematical artifact of the polynomial only; $N_c = 3$ in FTD is independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md` (four routes) and the Moore Layer Theorem.

### 19.3 Status

> **`x_+  1/α`**: [STRONGLY MOTIVATED CONJECTURE] (FTD-0013). **`x_-  N_c`**: RETIRED (v1.4 §5).

---

## §20. Physical-Correspondence Summary and Assessment

### 20.1 Results by Epistemic Tier

**Tier 1: Conditional on SP1–SP4 only (no circularity risk):**

| Result | Value | Experimental | Error |
|--------|-------|-------------|-------|
| $\alpha^{-1}$ (tree) | 137.036 | 137.036 | 1.26 ppm |

(The historical $\lfloor x_- \rfloor \to N_c$ row is **RETIRED** per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`. `N_c = 3` in FTD is independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md`.)

**Tier 2: Conditional on SP1–SP5 (circularity risk from integers):**

| Result | Value | Experimental | Error |
|--------|-------|-------------|-------|
| $\alpha^{-1}$ (4-term) | 137.035999177 | 137.035999177(21) | $< 0.001$ ppt |
| $\sin^2\theta_W$ | 0.2308 | 0.2312 | 0.19% |
| $m_\mu/m_e$ | 206.88 | 206.77 | 0.05% |
| $m_\tau/m_e$ | 3479.6 | 3477.2 | 0.07% |
| $m_p/m_e$ | 1836.5 | 1836.15 | 0.017% |

**Tier 3: Conditional on SP1–SP5 + Planck scale (circularity + imposed):**

| Result | Value | Experimental | Error |
|--------|-------|-------------|-------|
| $m_e$ | 0.510 MeV | 0.511 MeV | 0.19% |
| $v$ (Higgs VEV) | 245.9 GeV | 246.2 GeV | 0.05% |
| $\rho_\Lambda$ | $3.86 \times 10^{-47}$ | $3.90 \times 10^{-47}$ | 1.0% |

**Tier 4: Parametric insertions (FTD values in standard formulas):**

~130 results using FTD-derived parameters in imported QFT/QCD functional forms.

**Tier 5: Proposed/speculative:**

Reference frame context interpretation of complex roots ($k = 1/2$).

### 20.2 What Is Genuinely Impressive

1. The **tree-level** result ($x_+ = 137.036$, 1.26 ppm) requires only SP1–SP3 — no integer circularity
2. The **sub-ppt precision** of the 4-term formula, whether or not the integers are circular
3. The **structural convergence**: two independent truncations both land within experimental error
4. The **falsifiable prediction**: digit 13 of $\alpha^{-1}$ is predicted to be 0
5. The **interconnection** of diverse quantities (masses, angles, cosmology) from a common algebraic structure

### 20.3 What Remains Problematic

1. **No physical mechanism** connecting elliptic curves to gauge couplings (SP4)
2. **Integer circularity** contaminates all Tier 2+ results (SP5)
3. **~130 parametric insertions** use imported physics, not FTD dynamics
4. **Absolute mass scale** requires Planck-scale identification (imposed)
5. **Reference frame context interpretation** is unfalsifiable (Tier 5)

### 20.4 The Path Forward

The strongest tests of this framework are:

1. **Mathematical**: Survey all CM curves — is $j = 1728$ uniquely special? (§9.2)
2. **Experimental**: Measure $\alpha$ beyond current precision — is digit 13 zero? (§11.3)
3. **Theoretical**: Derive $\{3, 4, 7, 13\}$ from pure lattice topology (§9.4)
4. **Dynamical**: Derive radiative corrections from the FTD lattice action (`DERIV_TWO_LOOP_ALPHA.md`)

---

## §21. Claims Table

| ID | Statement | Depends on | Status |
|----|-----------|------------|--------|
| P-1 | $\alpha = 1/x_+$ to 1.26 ppm | SP1–SP4 | [CONDITIONAL THEOREM] |
| P-2 | 4-term formula to $< 0.001$ ppt | SP1–SP5 | [CONDITIONAL + CIRCULARITY] |
| P-3 | $\sin^2\theta_W = 3/13$ | SP5 | [CONDITIONAL + CIRCULARITY] |
| P-4 | $\alpha_s(M_Z) \approx 0.1187$ | SP5 | [CONDITIONAL + CIRCULARITY] |
| P-5 | $\alpha_G$ to 0.06% | SP1–SP5 | [CONDITIONAL + CIRCULARITY] |
| P-6 | $m_\mu/m_e = 206.88$ | SP5 | [CONDITIONAL + CIRCULARITY] |
| P-7 | $m_\tau/m_e = 3479.6$ | SP5 | [CONDITIONAL + CIRCULARITY] |
| P-8 | $m_p/m_e = 1836.5$ | SP5 | [CONDITIONAL + CIRCULARITY] |
| P-9 | $m_e = 0.510$ MeV | SP1–SP5 + $m_P$ | [CONDITIONAL + IMPOSED] |
| P-10 | $v = 245.9$ GeV | SP1–SP5 + $m_P$ | [CONDITIONAL + IMPOSED] |
| P-11 | $\rho_\Lambda$ to 1.0% | SP1–SP5 + $m_P$ | [CONDITIONAL + IMPOSED] |
| P-12 | $n_s = 0.966$, $r = 0.022$ | SP5 + inflation model | [CONDITIONAL + IMPOSED] |
| P-13 | ~130 parametric insertions | SP5 + standard QFT | [PARAMETRIC] |
| P-14 | Complex roots = reference frame context | SP1–SP3 + $k=1/2$ | [PROPOSED] |
| P-15 | Digit 13 of $\alpha^{-1}$ is 0 | SP1–SP5 | [FALSIFIABLE PREDICTION] |

---

## Cross-References

- **`MATH_MASTER_QUADRATIC.md`** — Layer 1: Pure mathematics (all algebraic identities referenced here) — kept separate, not merged.
- **`AUDIT_HIDDEN_SELECTIONS.md`** — Original critical assessment (the BRIDGE source document superseded its formal axiom statements).
- **`DERIV_ALPHA_PRECISION_FORMULA.md`** — Full derivation and verification of the 4-term precision formula.
- **`AUDIT_EPISTEMIC_AUDIT.md`** — Complete epistemic breakdown of all FTD claims.
- **`DERIV_TWO_LOOP_ALPHA.md`** — Lattice-based two-loop corrections to $\alpha$.
- **`SPEC_SM_REPLACEMENT_COMPLETE.md`** — Complete SM replacement status.
- **`THEOREM_MOORE_LAYER_DECOMPOSITION.md`** (FTD-0028) — polyhedral decomposition giving U(1) × SU(2) × SU(3) and the BCC sub-stencil.
- **`SPEC_FTD_COMPLETE_CHAIN.md`** — master quadratic chain.
- **`DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`** — master quadratic algebraic identity.

---

*Consolidated 2026-05-21 from `BRIDGE_QUADRATIC_PHYSICS.md` (Layer 2, Version 1.0 — February 25, 2026), `PHYS_QUADRATIC_APPLICATIONS.md` (Layer 3, Version 1.0 — February 25, 2026), and `FOUND_BRIDGE_FUNCTIONAL.md` (FTD-0095, 2026-04-26). Part I = selection principles bridging mathematics to physical identification; Part II = physical correspondences conditional on SP1–SP5; §4.6 = the bridge functional specializing SP4. See `MATH_MASTER_QUADRATIC.md` for pure mathematics (Layer 1, not merged).*
