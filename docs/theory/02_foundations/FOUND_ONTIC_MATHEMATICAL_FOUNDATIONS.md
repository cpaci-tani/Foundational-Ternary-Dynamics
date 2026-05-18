# The Ontic Mathematical Foundations: From Counting to Coupling

## The Constant Chain γ → ϖ → M → π → G*

**Date:** February 10, 2026
**Framework:** Foundational Ternary Dynamics v5.22
**Status:** Historical/interpretive structural atlas of the γ → ϖ → M → π → G* constant chain; **not** the canonical derivation chain for α.
**Epistemic Tags:** [THEOREM] for structural results, [DEFINITION] for defining relations, [STANDARD] for classical analysis, [SELECTION] for ordering, [CONJECTURE] for physical interpretation

> **Reevaluation note (2026-05-18).** This document predates the current FQCR / Algebraic Spine / Ontic Truth tracker stack. Its useful content is the classical-analysis atlas showing how Euler's constant γ appears in Γ(1/4), and hence in ϖ and G*. It should not be cited as deriving the physical fine-structure constant. Current canonical status lives in `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`, `docs/theory/01_reference/SPEC_FQCR.md`, and `docs/theory/07_assessment/TRACKER_ONTIC_TRUTH.md`: the G* identities and master quadratic are theorem-level; the physical identification `x_+ = 1/α` remains [STRONGLY MOTIVATED CONJECTURE].

---

## Abstract

We formalize an interpretive ontic ordering of mathematical constants in FTD as a directed chain:

$$\gamma \;\longrightarrow\; \varpi \;\longrightarrow\; M \;\longrightarrow\; \pi \;\longrightarrow\; G^*$$

where γ is the Euler-Mascheroni constant, ϖ is the lemniscate constant, M = AGM(1, √2) is the Gauss arithmetic-geometric mean, π is Archimedes' constant, and G* is the lemniscatic constant of FTD. The chain is grounded by the interpretive claim that γ is the inversion term where the discrete integer world first makes contact with the continuous analytical world, and this contact is logarithmic in character. Without γ, there is no Weierstrass-product route from counting to the elliptic/lemniscatic structure from which G* is defined. The later physical reading `x_+ = 1/α` is not supplied by this chain; it is tracked elsewhere as [STRONGLY MOTIVATED CONJECTURE].

The ordering is not by numerical value but by **ontological depth**: γ is most fundamental because it requires only the integers and the concept of a limit; the subsequent constants each require additional geometric or algebraic structure.

---

## Part I: The Five Constants

### 1.1 The Euler-Mascheroni Constant γ

$$\gamma = \lim_{n \to \infty} \left( \sum_{k=1}^{n} \frac{1}{k} - \ln n \right) = 0.5772156649\ldots$$

**What it requires:** Only the positive integers and the concept of a limit.

**What it encodes:** The exact offset between *discrete counting* (the harmonic series H_n = 1 + 1/2 + 1/3 + ... + 1/n) and *continuous scaling* (the natural logarithm ln n). This makes γ the **universal conversion constant** between the arithmetic world of integers and the analytic world of continuous functions.

**The inversion property:** Rearranging H_n ≈ ln(n) + γ:

$$\exp(H_n - \gamma) \;\approx\; n$$

Without γ, you cannot invert from continuous (logarithmic) back to discrete (counting). It is literally the constant that makes the integers recoverable from their logarithmic shadow.

**Why γ is ontologically first:** Every subsequent constant in the chain depends on the Gamma function Γ(z), and γ appears in the Weierstrass product representation:

$$\frac{1}{\Gamma(z)} = z \cdot e^{\gamma z} \cdot \prod_{n=1}^{\infty}\left[\left(1 + \frac{z}{n}\right) e^{-z/n}\right]$$

Here γ is the **exponential rate** that controls how the Gamma function scales. It is embedded multiplicatively — as exp(γz) — into every evaluation of Γ, including the critical value Γ(1/4) from which ϖ and G* are built.

### 1.2 The Lemniscate Constant ϖ (varpi)

$$\varpi = \frac{\Gamma(1/4)^2}{2\sqrt{2\pi}} = 2.6220575543\ldots$$

**What it requires:** γ (through Γ(1/4)), the integer 4, and π.

**What it encodes:** The half-period of the Bernoulli lemniscate r² = cos(2θ) — the "π of the lemniscate." Just as π measures the circle, ϖ measures the figure-eight: the first self-crossing algebraic curve, encoding self-reference geometrically.

**Connection to γ:** The digamma function satisfies

$$\psi(1/4) = -\gamma - \frac{\pi}{2} - 3\ln 2$$

This identity shows that γ is **analytically present** in the logarithmic derivative of Γ at the critical point s = 1/4. Since Γ(1/4) determines ϖ, γ flows through to ϖ via this derivative chain:

$$\gamma \;\xrightarrow{\text{Weierstrass}}\; \Gamma(1/4) \;\xrightarrow{\text{definition}}\; \varpi$$

### 1.3 The Gauss Constant M

$$M = \text{AGM}(1, \sqrt{2}) = 1.1981402347\ldots$$

**What it requires:** The arithmetic-geometric mean iteration applied to 1 and √2.

**What it encodes:** The rate of convergence of the AGM process — itself a bridge between arithmetic means (discrete averaging) and geometric means (multiplicative scaling). The AGM is the fastest-converging classical algorithm, doubling precision with each step.

**Exact relationship [THEOREM]:**

$$\varpi = \frac{\pi}{M} \qquad \Longleftrightarrow \qquad M = \frac{\pi}{\varpi}$$

This identity reveals M as the **conversion factor between circular geometry (π) and lemniscatic geometry (ϖ)**. The reciprocal 1/M ≈ 0.8346 is the classical Gauss constant.

**Note on ordering:** M sits between ϖ and π in the chain because it is the bridge that **converts** between them. ϖ is more fundamental (it requires only Γ(1/4) and hence only γ and 4); M requires the AGM concept; π requires circular geometry or, equivalently, the combination M · ϖ.

### 1.4 Archimedes' Constant π

$$\pi = M \cdot \varpi = \frac{4\varpi^2}{G^{*2}} = 3.1415926536\ldots$$

**What it requires:** The circle, or equivalently, the product of ϖ and M.

**What it encodes:** The ratio of circumference to diameter — the constant of **circular closure**. In FTD, π is not ontologically primitive. It is the product of two more fundamental constants:

$$\pi = \varpi \cdot M = \varpi \cdot \text{AGM}(1, \sqrt{2})$$

This decomposition reveals π as a **composite**: the lemniscatic half-period (ϖ) scaled by the arithmetic-geometric bridge (M). Equivalently:

$$\pi = \frac{4\varpi^2}{G^{*2}}$$

**Epistemic status [SELECTION]:** The claim that π is ontologically derivative of ϖ is a selection principle. Mathematically, any of {γ, ϖ, M, π, G*} can be expressed in terms of the others. The *ordering* we impose reflects the principle that constants requiring less structure are more fundamental.

### 1.5 The Lemniscatic Constant G*

$$G^* = \frac{\sqrt{2}\;\Gamma(1/4)^2}{2\pi} = \frac{2\varpi}{\sqrt{\pi}} = 2.9586751192\ldots$$

**What it requires:** All four preceding constants, or equivalently, ϖ and π together.

**What it encodes:** The master coefficient of the FTD quadratic:

$$x^2 - 16\,G^{*2}\,x + 16\,G^{*3} = 0$$

whose roots are x₊ = 137.036... ≈ 1/α and x₋ = 3.024... ≈ N_c.

**Why G* is ontologically last:** It is the most "constructed" of the five. It requires √2 (from the Gauss constraint geometry), Γ(1/4)² (which carries γ through Weierstrass), and π (which carries M and ϖ). G* is where all the preceding structure **converges** into a single number that, through the master quadratic, produces physics.

---

## Part II: The Chain as Ontological Ordering

### 2.1 The Principle

The ordering γ → ϖ → M → π → G* is not by numerical magnitude but by **ontological depth**: the amount of mathematical structure required for each constant's definition.

| Level | Constant | Value | Requires | Encodes |
|-------|----------|-------|----------|---------|
| **0** | γ | 0.5772... | Integers + limit | Discrete ↔ continuous bridge |
| **1** | ϖ | 2.6221... | γ (via Γ), integer 4 | Self-crossing geometry (lemniscate) |
| **2** | M | 1.1981... | AGM iteration | Arithmetic ↔ geometric bridge |
| **3** | π | 3.1416... | ϖ × M | Circular closure |
| **4** | G* | 2.9587... | √2, ϖ, π | Master quadratic coefficient → α |

### 2.2 Each Level Adds Structure

**Level 0 → 1 (γ → ϖ):** From discrete/continuous bridging to self-crossing geometry. The Weierstrass product converts γ (a limit of sums) into Γ(1/4) (a special function value), and the definition ϖ = Γ(1/4)²/(2√(2π)) crystallizes this into the period of a self-referential curve. This step adds **geometric self-reference**.

**Level 1 → 2 (ϖ → M):** From lemniscatic period to the AGM convergence rate. M = π/ϖ is defined by the arithmetic-geometric mean, which iterates between two types of averaging. This step adds **iterative convergence**.

**Level 2 → 3 (M → π):** From AGM rate to circular constant. π = ϖ · M combines the self-crossing period with the convergence bridge. This step adds **closure** (the circle is the simplest closed curve).

**Level 3 → 4 (π → G*):** From circular to lemniscatic with √2 scaling. G* = Γ(1/4)/Γ(3/4) = 2ϖ/√π = √2 · Γ(1/4)²/(2π) introduces the √2 from the FCC/Gauss constraint geometry. This step adds **constraint physics** — the discrete lattice's Gauss law.

### 2.3 γ as the Logarithmic Inversion Term

The deepest claim: **γ is where numbers first scale logarithmically into the decimal space.**

The harmonic series H_n = 1 + 1/2 + ... + 1/n grows as ln(n) + γ. This means the "natural" way to accumulate the reciprocals of integers (a purely arithmetic operation on 1, 2, 3, ...) produces a logarithmic function plus a correction term γ. The integers, through their reciprocal sum, *generate* logarithmic scaling — and γ is the remainder.

More precisely:
- **Discrete world:** H_n = Σ 1/k (sum over integers)
- **Continuous world:** ln(n) (integral ∫ dx/x)
- **Bridge:** γ = H_n - ln(n) in the limit

The exponential form exp(−γ) ≈ 0.5615 acts as a **decimal discount factor**: it governs how the discrete harmonic world undershoots the continuous logarithmic world. By Mertens' theorem, exp(−γ) even controls the distribution of primes:

$$\prod_{p \leq N,\; p\text{ prime}} \left(1 - \frac{1}{p}\right) \;\sim\; \frac{e^{-\gamma}}{\ln N}$$

So γ is not merely a correction term — it is the constant that governs how **prime factorization** (the most fundamental discrete structure) maps to **logarithmic density** (the most fundamental continuous measure).

---

## Part III: The Complete Derivation Chain

### 3.1 From γ to G* to the α candidate

The historical chain, with each mathematical step separated from the final physical identification:

```
γ = 0.5772...
│
│  [Weierstrass product: 1/Γ(z) = z·exp(γz)·∏...]
│  [Evaluate at z = 1/4]
│  [psi(1/4) = -γ - π/2 - 3ln2]
▼
Γ(1/4) = 3.6256...
│
│  [Definition: ϖ = Γ(1/4)²/(2√(2π))]
▼
ϖ = 2.6221...
│
│  [AGM identity: M = π/ϖ = AGM(1,√2)]
│  [Equivalently: ϖ = π/M]
▼
M = 1.1981...
│
│  [Product: π = ϖ · M]
▼
π = 3.1416...
│
│  [Scaling: G* = 2ϖ/√π = √2·Γ(1/4)²/(2π)]
│  [The √2 comes from Gauss constraint geometry]
▼
G* = 2.9587...
│
│  [Watson-G* Identity: W₃ = G*²/(2π) = Γ(1/4)⁴/(4π³)]
│  [The 3D cubic lattice self-energy IS a G*-derived quantity]
│
│  [Master quadratic: x² - 16G*²x + 16G*³ = 0]
│  [Degree 2: self-referential closure + CM field degree]
│  [Coefficient 16 = |Aut(E)|² where E: y²=x³-x]
│  [Vieta sum: x₊+x₋ = 16G*² = 32πW₃]
▼
x₊ = 137.036...  ≈  1/α
x₋ = 3.024...    ≈  N_c
│
│  [1/α + N_c = 32π × (Watson integral of Z³)]
│
│  [7-term precision formula with ε = e^π - π - 20]
▼
x₊ ≈ 1/α                (physical identification [STRONGLY MOTIVATED CONJECTURE];
                         precision-series refinements are conjectural/post-hoc unless
                         separately justified)
```

### 3.2 The Defining Relations [DEFINITION]

The entire chain rests on exactly **two independent definitions** plus one **FTD-specific claim**:

**Definition 1:** The lemniscate constant

$$\varpi = \frac{\Gamma(1/4)^2}{2\sqrt{2\pi}}$$

**Definition 2:** The lemniscatic constant

$$G^* = \frac{\sqrt{2}\;\Gamma(1/4)^2}{2\pi} = \frac{2\varpi}{\sqrt{\pi}}$$

**FTD-specific claim** [CONJECTURE]: The master quadratic with coefficient 16 (from lattice degrees of freedom)

$$x^2 - 16\,G^{*2}\,x + 16\,G^{*3} = 0$$

has roots x₊ = 137.036... ≈ 1/α and x₋ = 3.024... ≈ N_c.

**Honesty note:** All other relations previously listed (ζ(2n) in lemniscatic form, Vieta relations "in ϖ form," fractional power decompositions, discriminant factorizations) are **algebraic consequences of substituting these definitions**. Writing ζ(2) = 16ϖ⁴/(6G*⁴) is just replacing π² with 16ϖ⁴/G*⁴ in the standard formula π²/6. It looks impressive but adds no information. These substitutions are catalogued in `scripts/verification/explore_chain_roots_powers.py` for reference, but they are not independent results.

### 3.3 The γ–ϖ Connection: How Counting Becomes Geometry [STANDARD + SELECTION]

This is the genuinely meaningful part of the chain — the mechanism by which the discrete-to-continuous bridge constant γ flows into the lemniscatic structure.

**The Weierstrass product** provides the link:

$$\frac{1}{\Gamma(z)} = z \cdot e^{\gamma z} \cdot \prod_{n=1}^{\infty}\left[\left(1 + \frac{z}{n}\right) e^{-z/n}\right]$$

Evaluated at z = 1/4 (the FTD-selected evaluation point), γ enters as the factor exp(γ/4) inside Γ(1/4). Since ϖ and G* depend on Γ(1/4)², γ propagates through as exp(γ/2). This is not a numerical coincidence — it is a **structural consequence** of how the Gamma function is built from the integers.

The digamma function (logarithmic derivative of Γ) makes this connection explicit:

$$\psi(1/4) = -\gamma - \frac{\pi}{2} - 3\ln 2 \qquad \text{[STANDARD — Gauss digamma theorem]}$$

$$\psi(1/3) = -\gamma - \frac{3}{2}\ln 3 - \frac{\pi}{2\sqrt{3}} \qquad \text{[STANDARD — Gauss digamma theorem]}$$

These are not FTD results — they are classical identities due to Gauss (1813). What is meaningful for FTD is that the evaluation points z = 1/3 and z = 1/4 correspond to the two primary framework integers N_c = 3 and N_base = 4. Setting both expressions equal (since both define γ) yields:

$$\psi(1/4) - \psi(1/3) = \frac{3}{2}\ln 3 + \frac{\pi}{2\sqrt{3}} - \frac{\pi}{2} - 3\ln 2$$

This is a non-trivial **cross-constraint** linking the digamma at the two FTD integers through a specific combination of π, ln 2, ln 3, and √3. The constraint is standard mathematics, but the fact that FTD's integers sit exactly at evaluation points with closed-form digamma values is what makes it structurally relevant.

### 3.4 Standard Results Used [STANDARD]

The chain uses several classical results from analysis. These are **tools**, not FTD discoveries:

| Result | Source | Role in Chain |
|--------|--------|---------------|
| Γ(z)·Γ(1−z) = π/sin(πz) | Euler reflection formula | Relates Γ(1/4) to Γ(3/4) |
| Gauss multiplication formula | Gauss (1812) | Connects Γ at multiples of 1/n |
| ψ(1/n) closed forms | Gauss digamma theorem (1813) | Links γ to Γ(1/4) analytically |
| ϖ·M = π | AGM–elliptic identity | Defines M as bridge between ϖ and π |
| ζ(2n) = (−1)^{n+1}B_{2n}(2π)^{2n}/(2·(2n)!) | Euler (1735) | Even zeta from Bernoulli numbers |
| Mertens' theorem: ∏(1−1/p) ~ e^{−γ}/ln N | Mertens (1874) | γ governs prime distribution |

Listing these honestly — as deep mathematics that the framework *connects to* rather than *derives* — is both more accurate and more compelling than claiming them as FTD identities.

---

## Part III-B: Structural Theorems

### 3B.1 The Minimal Generating Set [THEOREM]

**Definitional/derived observation:** The constant atlas can be generated from exactly **two constants**: γ (Euler-Mascheroni) and π (Archimedes), plus standard analytic machinery.

**Proof sketch:** Given γ and π:
1. Γ(1/4) is determined by the Weierstrass product (which requires only γ and the integer 4)
2. ϖ = Γ(1/4)²/(2√(2π))
3. M = π/ϖ = AGM(1,√2)
4. G* = 2ϖ/√π = √2·Γ(1/4)²/(2π)
5. x₊, x₋ from the master quadratic
6. α = 1/x₊ (with precision corrections using framework integers)

The circularity (π appears in step 2, which seems to require π before generating it) resolves because **both γ and π are independently definable from the integers alone**:
- γ = lim[H_n − ln(n)]
- π = 4·arctan(1) = 4·Σ(−1)^n/(2n+1)

**Implication:** G* and the master-quadratic root candidate are determined by this analytic constant package plus framework integers. The fine-structure reading requires the separate physical identification `x_+ = 1/α`.

### 3B.2 The exp(γ/2) Universal Scaling [THEOREM]

**Theorem:** The factor exp(γ/2) is the **universal rescaling** that removes γ from all lemniscatic constants simultaneously.

Define the "γ-free" constants:
- Γ₀(1/4) = Γ(1/4) · exp(γ/4)
- ϖ₀ = ϖ · exp(γ/2) = Γ₀(1/4)²/(2√(2π))
- G*₀ = G* · exp(γ/2)

**Why exp(γ/2)?** Because γ enters the chain **only** through Γ(1/4) via the Weierstrass product, where it contributes a factor of exp(γ/4). Since ϖ and G* depend on Γ(1/4)², the γ-dependence always enters as exp(γ/4)² = exp(γ/2).

**Physical reading:** The factor exp(−γ/2) ≈ 0.749 is the "discount" that the discrete-to-continuous bridge (γ) applies to the idealized lemniscatic constants. Without this discount, ϖ₀ ≈ 3.499 and G*₀ ≈ 3.949 — both larger than their actual values. The integers, through their logarithmic shadow, **compress** the lemniscatic constants.

---

## Part IV: Relation to Existing FTD Hierarchy

### 4.1 Where This Fits

The existing FTD ontological hierarchy (FOUND_ONTOLOGICAL_GENESIS.md) begins at Level 0 with the Pure Integral I₄. The ontic constant chain provides the **sub-structure beneath I₄**:

| Prior Hierarchy | This Document | Constant |
|-----------------|---------------|----------|
| Level -3: Absolute Void | — | (no mathematics) |
| Level -2: Pregnant Void | — | (potentiality) |
| Level -1: First Distinction | Level 0 | γ (integers emerge, counting begins) |
| Level 0: Self-Reference (n=4) | Level 0→1 | γ → Γ(1/4) (evaluation at n=4) |
| Level 1: Pure Integral I₄ | Level 1 | I₄ = ϖ/2 (half the lemniscate constant) |
| Level 5: Lemniscatic | Levels 1-4 | ϖ → M → π → G* |

**Key identification:** I₄ = ϖ/2. The "Pure Integral" of the prior hierarchy is exactly half the lemniscate constant. The ontic chain γ → ϖ provides the foundation *beneath* I₄ by showing where the Gamma function (and hence ϖ) gets its scaling from.

### 4.2 The Role of the Integer 4

In the prior hierarchy, n = 4 is selected by self-reference (FOUND_THE_FIRST_DISTINCTION.md). In the ontic chain, 4 appears at the **evaluation point** z = 1/4 where the Weierstrass product (carrying γ) is evaluated to produce Γ(1/4). The two hierarchies converge:

- **Prior:** Self-reference selects n = 4 → I₄ = ∫₀¹ dx/√(1−x⁴)
- **Ontic:** γ flows through Γ(z) at z = 1/4 → Γ(1/4) → ϖ = 2I₄

Both paths arrive at the same destination: the lemniscate constant ϖ. The integer 4 is the **meeting point** of the self-referential argument (why n = 4) and the analytic argument (why z = 1/4 in the Weierstrass product).

---

## Part V: Numerical Observations

Systematic exploration of power, root, and logarithmic relations among the chain constants was performed at 200–300 digit precision (see `scripts/verification/`). Several approximate numerical relationships were found, none of which are exact. They were discovered by testing thousands of combinations and reporting the closest hits — a process that will always produce near-misses regardless of whether any structural relationship exists.

These are recorded in the verification scripts for completeness but **are not claimed as structural results**. The most notable (ln(M)/ln(G*) ≈ 1/6 at 109 ppm, and exp(ψ(1/3)−ψ(1/4)) ≈ 3 at 0.3%) were investigated at 300-digit precision in `scripts/verification/investigate_near_misses.py` and confirmed to be approximate, not exact.

---

## Part VI: Summary

### The Ontic Chain

$$\boxed{\gamma \;\xrightarrow{\text{Weierstrass}}\; \Gamma(1/4) \;\xrightarrow{\text{def.}}\; \varpi \;\xrightarrow{\text{AGM}}\; M \;\xrightarrow{\times\,\varpi}\; \pi \;\xrightarrow{\sqrt{2}\,\text{scaling}}\; G^* \;\xrightarrow{\text{quadratic}}\; x_+ \approx 1/\alpha}$$

### Physical Interpretation [CONJECTURE]

γ is the **first constant that exists** once the integers exist and the concept of a limit is available. It encodes the irreducible gap between counting and measuring — between the discrete and the continuous. Every subsequent constant in the chain inherits this gap through the Gamma function's Weierstrass product, which carries γ as an exponential rate factor.

The physical implication (if the master quadratic identification x₊ = 1/α is accepted at its current canonical tag): **the fine structure constant α would inherit its specific numerical value from γ through a chain of exact algebraic relationships plus one conjectured physical identification.** The integers provide the skeleton (3, 4, 7, 13); γ marks the continuous scaling in the Gamma-function route; and the lemniscatic structure provides the self-referential closure. The chain is: γ → Γ(1/4) [STANDARD] → ϖ, G* [DEFINITION/THEOREM in the canonical spine] → master quadratic [THEOREM] → x₊ ≈ 1/α [STRONGLY MOTIVATED CONJECTURE].

### Epistemic Status

| Claim | Tag | Notes |
|-------|-----|-------|
| γ as logarithmic inversion term | **[STANDARD]** | Follows from H_n = ln(n) + γ + O(1/n) (Euler, 1735) |
| Weierstrass product: γ flows into Γ(1/4) via exp(γz) | **[STANDARD]** | Classical analysis; structural mechanism (not an FTD result) |
| exp(γ/2) universal scaling of lemniscatic constants | **[THEOREM]** | Consequence of Γ(1/4)² dependence |
| Minimal generating set {γ, π} | **[DERIVED / DEFINITIONAL]** | Constructive: Weierstrass + definitions; not a physical theorem |
| Digamma cross-constraint at z=1/3, 1/4 | **[STANDARD]** | Gauss (1813); meaningful that FTD integers have closed forms |
| Ordering by ontological depth | **[SELECTION]** | Principled but not uniquely determined |
| Master quadratic yields x₊ ≈ 1/α | **[STRONGLY MOTIVATED CONJECTURE]** | Numerical match 1.26 ppm tree-level; current canonical tag from TRACKER_ONTIC_TRUTH OT-5.1 |
| Coefficient 16 from lattice/CM structure | **[T4 identification / structural conjecture]** | Value-level equality with \|Aut(E)\|² is true; necessity is not proved |
| Substitution identities (ζ in ϖ form, etc.) | **[DEFINITION]** | Algebraic consequences; no independent content |

---

## References

- **Verification scripts:** `scripts/verification/verify_ontic_constant_chain.py`, `explore_chain_deep.py`, `explore_chain_roots_powers.py`, `investigate_near_misses.py`
- **Prior hierarchy:** [FOUND_ONTOLOGICAL_GENESIS.md](FOUND_ONTOLOGICAL_GENESIS.md), [FOUND_THE_FIRST_DISTINCTION.md](FOUND_THE_FIRST_DISTINCTION.md)
- **G* and master quadratic:** [DERIV_LEMNISCATE_HIERARCHY_WHITEPAPER.md](../04_coupling/DERIV_LEMNISCATE_HIERARCHY_WHITEPAPER.md)
- **Precision formula:** [DERIV_ALPHA_PRECISION_FORMULA.md](../04_coupling/DERIV_ALPHA_PRECISION_FORMULA.md)
- **Framework integers:** [EXPLR_NUMBER_THEORY.md](../09_mathematical/EXPLR_NUMBER_THEORY.md)
- **Classical analysis:** Gauss, *Disquisitiones Generales* (1813); Euler, *De summis serierum reciprocarum* (1735); Mertens (1874)

---

*Document created: February 10, 2026*
*Revised: February 11, 2026 (v3 — honest reclassification: structural results distinguished from definitional tautologies and numerical searches)*
*Epistemic corrections (v5.29): February 2026 — standard analysis results re-tagged [STANDARD] (not FTD [THEOREM]s); gamma chain summary clarified*
*Framework: Foundational Ternary Dynamics v5.22*
