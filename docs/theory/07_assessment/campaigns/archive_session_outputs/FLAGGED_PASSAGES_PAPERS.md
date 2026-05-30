# Flagged Passages — Papers (Phase 2)

**Date:** 2026-04-19
**Reframe canonical:** `docs/theory/07_assessment/reframe_deployment/CANONICAL_REFRAME.md` v1.0
**Scanner:** automated grep over `docs/papers/*.{tex,md}`, `docs/papers/src/*.{tex,md}`, `docs/papers/speculative/*.{tex,md}`. PDFs and `archive/` excluded.

---

## Summary

- **Papers scanned (sources):** 34 (.tex/.md)
- **Files with at least one trigger hit:** 24
- **Files with NO trigger hits at all:** 10
- **Total proscribed (load-bearing) passages:** ~37
- **Total ambiguous (need user judgment):** ~12
- **Total permitted (algebraic / classical / Z-symbol notation / β-parameter limits):** majority of remaining hits

### Papers with NO proscribed passages
Confirmed clean (zero trigger hits or only Z-symbol notation):
- `PAPER_RATIO_AND_THE_ARROW.tex`
- `PAPER_RATIO_AND_PRODUCT.tex` (only `\mathbb{Z}` notation)
- `ratio_and_the_arrow.tex` (duplicate root copy)
- `PAPER_PATH.md`
- `README.md`
- `LETTER_HERMITIAN_COPE.tex`
- `DERIV_GEOMETRIC_BIOPHYSICS.tex`
- `PAPER_3A_PHYSICAL_IDENTIFICATION.tex` (only "asymptotic divergence of QED" descriptor — meta-comment about QED, not an FTD claim)
- `PAPER_0A_PERIOD_DESCENT.tex`
- `PAPER_0B_THREE_CONSTANTS.tex`

### Papers with HEAVY proscribed reliance (>5 load-bearing passages)
- `speculative/FTD_Yang_Mills_Mass_Gap.tex` — entire premise rests on infinite-volume limit, path integral over all configurations, Wightman axioms on $\R^4$
- `speculative/FTD_Navier_Stokes.tex` — continuum limit + lattice-to-continuum bridge are load-bearing
- `speculative/FTD_Finitude_Theorem.tex` — paradoxically, this paper *is* the reframe in another voice; many uses of `\infty` are *quoting the proscribed object to deny it*. Self-frame-relative — needs careful re-classification rather than simple flag-by-grep.

---

## Per-paper audit

### `PAPER_GSTAR_BRIDGE_CONSTANT.tex`

**Trigger hits:** 5
**Proscribed (load-bearing):** 0
**Permitted (Wallis / algebraic):** 5
**Ambiguous:** 0

**Top hits:**
1. Line 55: `$\Gstar = \lim_{N\to\infty} N^{-1/2}\prod (4k+3)/(4k+1)$` → **SURVIVES** (Wallis-style; canonical worked example).
2. Line 163, 166, 184, 266: identical pattern, all defining $\sqrt\pi$ and $G^*$ via Wallis-type products.

**Triage recommendation:** SURVIVES — pure algebraic identities with constructive convergence. May benefit from optional rewording to "for any precision $\varepsilon$, there exists $N$ such that $S_N$ approximates $G^*$ within $\varepsilon$."

---

### `PAPER_GSTAR_IDENTITIES.tex`

**Trigger hits:** 6
**Proscribed:** 0
**Permitted:** 6
**Ambiguous:** 0

**Top hits:** Lines 85, 93, 121, 148, 180, 280 — all Wallis-type `\lim_{N\to\infty}` defining $\sqrt\pi$, $G^*$, $\varpi$.

**Triage:** SURVIVES.

---

### `PAPER_MISSING_RATIO.tex`

**Trigger hits:** 2
**Proscribed:** 0
**Permitted:** 2
**Ambiguous:** 0

**Top hits:**
1. Line 190: `${\Gstar} = \lim_{N\to\infty}(N+1)^{-1/2}…$` → SURVIVES (Wallis).
2. `\mathbb{Z}` notation in setup.

**Triage:** SURVIVES.

---

### `PAPER_TWO_RACES.tex`

**Trigger hits:** 8
**Proscribed:** 0
**Permitted:** 7 (Wallis)
**Ambiguous:** 1

**Top hits:**
1. Lines 85, 93, 121, 148, 180, 280: Wallis-style limits → SURVIVES.
2. Line 195: `$x\to\infty$ (with $a=3/4, b=1/4$)` — asymptotic descriptor for Gamma-ratio → SURVIVES (characterizes behavior, not value-defining).
3. Line 416: `$R(N)/N \to \pi$ as $N\to\infty$` → SURVIVES (constructive ratio, classical pi computation).

**Triage:** SURVIVES.

---

### `DERIV_CLOSURE_RENORMALIZATION.tex`

**Trigger hits:** 4
**Proscribed:** 0
**Permitted:** 4
**Ambiguous:** 0

**Top hits:** Lines 290, 298, 317, 351 — all of form "$\mathcal R_p(k)\to 1$ as $k\to\infty$" / "$\mathcal S(n)\to 1$ as $n\to\infty$". These characterize asymptotic *behavior of a ratio* (ratio approaches unity), not value-definition.

**Triage:** SURVIVES — Question 2 of decision procedure: "characterize behavior" not "define a value".

---

### `PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY.tex` and `.md`

**Trigger hits:** 8 (.tex) + 8 (.md), same content
**Proscribed (load-bearing):** 3
**Permitted:** 0
**Ambiguous:** 0

**Top proscribed passages:**
1. Line 271 (.tex) / 229 (.md): "Adding the spatial gradient term … from the **continuum limit** of the lattice Laplacian" → **RESTATE**. Use of "continuum limit" as the operation that produces the gradient term. Per Q3, replace with "from the small-spacing expansion of the lattice Laplacian, which agrees with the continuum gradient at any specified spacing."
2. Line 342 / 291: "symmetry breaking to $SU(3)\times SU(2)\times U(1)$ **in the continuum limit**" → **RESTATE**. Parametric description of EFT matching at arbitrarily fine spacing.
3. Line 377 / 329: "Restoration of $O(3,1)$ invariance **in the continuum limit** is expected on universality grounds" → **RESTATE**. Standard physics framing imported wholesale; needs finitary version: "at arbitrarily small lattice spacing, $O(3,1)$-violating residuals are bounded by [explicit bound]."

**Triage:** RESTATE-MOSTLY. Three uses of "continuum limit" are framing shorthand; underlying content is sound (small-spacing expansions, EFT matching). Rewrite is mechanical.

---

### `PAPER_LIFECYCLE_SOFTPLUS.tex`

**Trigger hits:** 10
**Proscribed:** 0
**Permitted:** 9 (β-parameter limit, not L-spatial limit)
**Ambiguous:** 1

**Top hits:**
1. Lines 133, 164, 228, 428, 449, 512, 529, 728, 769: `$\beta \to \infty$` is the inverse-temperature limit characterizing the ReLU (sharp manifestation) regime — biological "death". Per Q1, this is a *parameter limit on a single voxel/system*, not a structural limit on lattice extent. It is a description of the operator's pointwise behavior.
2. Line 769: "Death = β → ∞ [Theorem]; Mathematical limit of the Softplus" → AMBIGUOUS. The framing "limit of the Softplus" suggests defining ReLU as completed limit. Probably SURVIVES under Q3 (ReLU exists as a closed-form $\max(0,z)$ object independently of any limit). Recommend rewording: "ReLU is the $\beta\to\infty$ asymptote of Softplus" or "ReLU is the closed-form $\max(0,z)$ which Softplus approaches at large $\beta$".

**Triage:** SURVIVES with cosmetic restate of one line.

---

### `src/DERIV_SOFTPLUS_RELU_DUALITY.tex`

**Trigger hits:** 16
**Proscribed (load-bearing):** 1
**Permitted (β-parameter):** 14
**Ambiguous:** 1

**Top hits:**
1. Lines 257, 295, 475, 484, 497, 524, 580: `$\beta \to \infty$` — same parametric pattern as Lifecycle paper → SURVIVES.
2. Line 484: "At $\beta = 0$ (infinite temperature), all modes are thermally active … At $\beta \to \infty$ … corresponding to the deep **IR continuum limit** where the coupling has flowed to its fixed point." → **PROSCRIBED** (RG flow to fixed point — proscribed move #6 in canonical doc). RESTATE.
3. Line 580: "the Softplus temperature $\beta$ and the Wilsonian RG scale $\mu$ provide structurally analogous descriptions of UV mode suppression, with $\beta\to\infty$ … corresponding to the IR ground state" → AMBIGUOUS / RESTATE. The analogy itself is fine; the words "IR ground state" and "fixed point" import completed-limit RG language.

**Triage:** RESTATE — one passage uses "IR continuum limit" + "fixed point" as load-bearing identification. Replace with finitary "at arbitrarily small lattice spacing the coupling is bounded near $g_*$ with $|g(\mu)-g_*|<\varepsilon$ for $\mu < \mu_\varepsilon$" or similar.

---

### `src/PAPER_2A_MASTER_QUADRATIC.tex`

**Trigger hits:** 14
**Proscribed (load-bearing):** 1
**Permitted:** 11 (asymptotic descriptors of $F(x)$, behavior of rational function as $x\to\infty$)
**Ambiguous:** 2

**Top passages:**
1. Lines 243, 251: "$F(x) \to K$ as $x \to \infty$ (screening vanishes at weak coupling)"; "$g(x)/x \to 1$ as $x \to \infty$" → SURVIVES. Asymptotic *behavior of a function*, characterizing its functional form for the rational-function uniqueness argument.
2. Line 473: "If the quadratic sector dominates **in the thermodynamic limit**, the [SELECTION] becomes a theorem." → **PROSCRIBED**. Direct invocation of thermodynamic limit as the regime in which a [SELECTION] would upgrade to [THEOREM]. **RESTATE** as "at arbitrarily large but finite $L$ the quadratic sector dominates in the sense that [explicit bound]".
3. Line 475: "Preliminary calculations on the $L = 2$ torus show convergence to the master quadratic **as $L \to \infty$**, but a closed-form proof is lacking." → AMBIGUOUS. As written this asserts a completed limit. RESTATE: "for $L \in \{2, 4, 8, …\}$ tested, the deviation from the master quadratic decreases as a function of $L$; a finitary closed-form bound is open."

**Triage:** RESTATE — two passages need finitary rephrasing; both are open-conjecture promissory notes (not proofs), so the restatement only changes the standard for what would constitute proof.

---

### `src/DERIV_ALPHA_PRECISION.tex`

**Trigger hits:** 9 (`\mathbb Z` notation) + 1 (asymptotic)
**Proscribed:** 0
**Permitted:** 1
**Ambiguous:** 0

**Top hits:**
1. Line 391: "the **asymptotic divergence** of QED is resolved—UV regularization and series truncation are distinct issues." → SURVIVES. Meta-comment *about* the standard QED asymptotic series; FTD's claim is finitary (precision-bracketed).

**Triage:** SURVIVES.

---

### `src/FTD_One_Unit_Final.tex`

**Trigger hits:** 1
**Proscribed:** 0
**Permitted:** 1

**Top hit:**
1. Line 600: "$(\coth\to 1, \csch\to 0)$, corresponding to $\ell\to\infty$" → SURVIVES (asymptotic descriptor of hyperbolic functions for large argument).

**Triage:** SURVIVES.

---

### `src/FTD_Discrete_Continuous_Bridge.tex`

**Trigger hits:** 9
**Proscribed (load-bearing):** 1–2
**Permitted:** 5 (descriptive use of "continuous", "discrete-to-continuous bridge")
**Ambiguous:** 2

**Top passages:**
1. Line 602: "the value the sum would approach if **the lattice spacing were sent to zero**" → **PROSCRIBED**. This is a value-defining limit (Q2). RESTATE: "the value parameterizing the discrete contribution at zero spacing-parameter $q=0$, which is a finite algebraic quantity attainable by the explicit formula …"
2. Line 656, 708: "an **infinite product** over its roots" / "factorisations to **infinite products** — discovers $\pi$" → AMBIGUOUS. References to Euler's classical infinite-product formulas. SURVIVES under Q3+Q4 (these are constructive Wallis/Euler products with specified $N$-term truncations).
3. Line 735: "Among the **infinite family** of lattice [families]" → AMBIGUOUS. "Infinite family" used as casual cardinality. SURVIVES with reword to "among the unbounded family" if literal-reading is desired.

**Triage:** RESTATE — one load-bearing limit; rest survives with optional cosmetic rewords.

---

### `src/FOUND_ONTIC_INCOMPLETENESS.tex`

**Trigger hits:** 4 (3 `\to\infty` + 1 `infinite`)
**Proscribed:** 0
**Permitted:** 1
**Ambiguous:** 0

**Top hits:**
1. Line 225: "$\lambda = \lim_{t \to \infty} (1/t) \ln |df^t/dC|$" → SURVIVES under Q4 (defines Lyapunov exponent as a per-trajectory quantity; constructive in finite-time bound: for any $\varepsilon$ and any specified time $T$, the finite-$t$ approximation suffices).

**Triage:** SURVIVES — single use is a standard-physics quotation embedded in a meta-discussion of epistemic vs ontic.

---

### `src/ontic_derivation_chain.tex`

**Trigger hits:** 3 (`\mathbb Z`)
**Proscribed:** 0
**Permitted:** all (Z-symbol notation)

**Triage:** SURVIVES.

---

### `src/PAPER_0E_ARITHMETIC_GEOMETRIC_EQUIVALENCE.tex`

**Trigger hits:** 32 (all `\mathbb{Z}` symbol uses for Z[i], Z_4, Z^3 notation)
**Proscribed:** 0
**Permitted:** all

**Triage:** SURVIVES — every hit is `\mathbb{Z}`-symbol use (Gaussian integers, cyclic groups, $\Z^3$ as carrier set named in algebraic context, not as completed totality). Note: any *load-bearing* claim about $\Z^3$ as the substrate would be elsewhere (PAPER_2A, Yang-Mills, Finitude); this paper is pure algebraic geometry.

---

### `src/PAPER_1A_WATSON_LATTICE_BRIDGE.tex`

**Trigger hits:** 22 (Z-symbol) + 1 (D→∞ asymptotic)
**Proscribed:** 0
**Permitted:** 23
**Ambiguous:** 0

**Top hit:**
1. Line 215: "as $D \to \infty$, $W_D \to 1$ … $x_-(\infty) \to 2.5$" → SURVIVES. Asymptotic *behavior* used to argue the floor of $x_-$ stabilizes at integer 2 for all sufficiently large finite $D$ — a property that holds at every specified large $D$.

**Triage:** SURVIVES.

---

### `speculative/FTD_Yang_Mills_Mass_Gap.tex`

**Trigger hits:** ~25 across all categories
**Proscribed (load-bearing):** ~10
**Permitted:** ~5
**Ambiguous:** ~3

**Top proscribed passages:**
1. Line 207: "the sum runs over **all state configurations** $s:\Lambda\to\{-1,0,+1\}$ and the integral over **all flux configurations**" → **PROSCRIBED** (canonical proscribed move #4: path/functional integral over all configurations). For finite $\Lambda$ this is permitted; for $|\Lambda|\to\infty$ it is not.
2. Line 430, 432: section "Mass gap survives **infinite volume**" / Proposition "**Infinite-volume mass gap**" → **PROSCRIBED** (#7).
3. Line 434: "The mass gap $\Delta=K_B$ persists in the **thermodynamic limit** $|\Lambda|\to\infty$." → **PROSCRIBED** (#3).
4. Line 444: "Wilson's framework, the mass gap is observed numerically at finite coupling but not proved to survive the **continuum limit** $a\to 0$." → context-bound (describes Wilson, not FTD), borderline SURVIVES.
5. Line 506, 513, 537, 590: claims about Wightman axioms on $\R^4$, "continuum reconstruction theorem" needed → AMBIGUOUS. Paper itself flags these as gaps; framing is honest. RESTATE the framing to make explicit that Clay-as-stated requires completed-infinity reconstruction which FTD does not attempt.
6. Line 121: paper *quotes* the Clay statement of YM existence on $\R^4$, then makes the FTD claim on $\Z^3\times\N$ — internal acknowledgment of the gap.

**Triage:** **RE-DERIVE** for the central "infinite-volume mass gap" theorem. The mass gap as a *local* threshold property at every voxel is permitted (per the paper's own argument lines 587, 438). The completed `|\Lambda|\to\infty` framing is not; the substantive content (a per-voxel Boolean threshold $K_B>0$) is finitary and survives. Rewrite proposition + proof in finitary form: "for any specified region $\Lambda$, the mass gap holds; it does not depend on $|\Lambda|$." That is what the paper *actually* proves; the framing as "thermodynamic-limit theorem" is a wrapper.

---

### `speculative/FTD_Finitude_Theorem.tex`

**Trigger hits:** 26 (`infinite` keyword) + 8 (`\to\infty`)
**Proscribed:** 0 (positively asserting completed infinity)
**Permitted:** ~30 — *the paper denies completed infinity; many `\infty` occurrences quote-and-deny.*
**Ambiguous:** 4

**Top passages (note: this is a self-frame-relative case — paper aligns with the reframe):**
1. Line 89, 94: "no physical observable can be infinite within a framework where space is a discrete lattice $\Z^3$ … Every 'infinity' that appears in physics … is traced to the use of $\R^3$ where $\Z^3$ is the actual substrate." → SURVIVES — exactly the reframe message, predates the canonical document.
2. Lines 218–251: propositions "No infinite address / state / velocity / energy density / information" → SURVIVES, finitary.
3. Line 277, 278: tabulates QED/QFT divergences and shows they vanish under the lattice substrate; the `\to\infty` symbols here are *naming the proscribed object to dispatch it*. SURVIVES.
4. Line 116: paper still uses "$\Z^3$" as shorthand for the substrate. Per the new commitment that $\Z^3$ is *not* completed-infinity but undefined-boundary, this is now technically a notational legacy. AMBIGUOUS. Recommend a one-sentence preamble clarifying that "$\Z^3$" in this paper means "the undefined-boundary cubic substrate (every specified voxel has 26 neighbors)" rather than the totalized integer lattice.

**Triage:** SURVIVES with one-paragraph preamble note. The paper is *the closest* of the portfolio to the canonical reframe; needs a notational alignment, not content surgery. The substantive claim (finitude of all observables) is exactly what the reframe protects.

---

### `speculative/FTD_Navier_Stokes.tex`

**Trigger hits:** 18
**Proscribed (load-bearing):** 6
**Permitted:** 6 (R^3 used as quoted-and-denied object — same pattern as Finitude paper)
**Ambiguous:** 6

**Top proscribed passages:**
1. Line 314: "In the continuum, blow-up means $\|\mathbf u(\cdot,t)\|_{L^\infty}\to\infty$ as $t\to T^*$" → SURVIVES (quotes Clay statement to deny FTD applicability).
2. Line 328: section title "**The lattice-to-continuum limit**" → **PROSCRIBED** (use of "continuum limit" as section-defining concept).
3. Line 332: "**In the limit** where the lattice spacing $a\to 0$ and the tick duration $\Delta t\to 0$ with $C=a/\Delta t=1/\sqrt{3}$ fixed, the FTD update rule reduces to the continuum equation" → **PROSCRIBED**. This is a value-defining completed limit (the continuum equation is *defined* by it).
4. Line 364, 388: "the corresponding continuum-limit field $\mathbf u(\mathbf x,t)$ belongs to $H^m(\R^3)$" / "$\mathbf u\in C^\infty(\R^3)$ for all $t\ge 0$" → **PROSCRIBED**. The C^∞-on-R^3 statement is a completed-totality regularity claim.
5. Line 498: "Global existence, uniqueness, uniform energy bounds, bounded vorticity, and **smooth continuum limits** on the lattice $\Z^3\times\N$." → **PROSCRIBED**. Same issue.
6. Line 501: "the continuum Navier–Stokes equations on $\R^3$ (as literally formulated by Clay) have smooth solutions." → AMBIGUOUS — borderline; the paper's own caveat at 503 partially recoups.

**Triage:** **RE-DERIVE** for the lattice-to-continuum bridge. The finitary content (per-voxel bounds, discrete energy budget, no per-voxel blow-up) is sound. The continuum-limit framing requires either a finitary EFT-matching restatement ("at arbitrarily small spacing, the discrete solutions agree with the continuum solutions on a specified compact region to specified precision") or removal. Per the canonical doc's "What this document does not decide" (item 3), it is open whether Clay-on-$\R^3$ can be re-derived from FTD; the current claim is at minimum overstated under the reframe.

---

### `speculative/FTD_Yang_Mills_Mass_Gap.tex` — see above (heavy reliance section).

---

### `speculative/FTD_Riemann_Hypothesis.tex`

**Trigger hits:** 2
**Proscribed:** 0
**Permitted:** 2
**Ambiguous:** 0

**Top hits:**
1. Line 137: "Euler–Mascheroni constant $\gamma = \lim_{n\to\infty}[\sum_{k=1}^n 1/k - \ln n]$" → SURVIVES (constructive partial-sum Wallis-type characterization).
2. Line 370: "Point counts $\#E(\mathbb F_p)$ are computed directly for small primes" — finite enumeration. SURVIVES.

**Triage:** SURVIVES — but **caution:** RH-related work that operationally uses "all primes" or "all zeros" should be re-examined; this scan was for trigger phrases, not for completeness over zeta-function content. Recommend a deeper read by user.

---

### `speculative/DERIV_CASIMIR_RATCHET.tex`

**Trigger hits:** 4 (3 `\to\infty` + 1 other)
**Proscribed:** 0
**Permitted:** 3 (β-parameter limit, same pattern as Lifecycle/Softplus)
**Ambiguous:** 1

**Top hits:**
1. Line 72, 74: "$T \to 0$ … $\beta_{th} \to \infty$ … $\lim_{\beta_{th} \to \infty} (1/\beta_{th})\ln(1+e^{\beta_{th}(z-K_B)}) = \max(0,z-K_B)$" → SURVIVES (parametric ReLU limit).

**Triage:** SURVIVES.

---

### `speculative/DERIV_SONOLUMINESCENCE.tex`

**Trigger hits:** 1
**Proscribed:** 0
**Permitted:** 1

**Top hit:**
1. Line 134: "the effective computational temperature $T \to \infty$, driving the thermodynamic parameter $\beta_{th} \to 0$. **Taking the mathematical limit** of the Softplus function as $\beta_{th} \to 0$ …" → SURVIVES (parametric β-limit, same as Casimir).

**Triage:** SURVIVES.

---

### `speculative/DERIV_GRAND_UNIFIED_MASS.tex`

**Trigger hits:** 1
**Proscribed:** 0
**Permitted:** 1 — only `\mathbb{Z}` notation.

**Triage:** SURVIVES.

---

## Cross-paper patterns

1. **Wallis-type `\lim_{N\to\infty}` is the most common trigger and is universally permitted** under the canonical reframe (worked-example pattern). Five papers (`PAPER_GSTAR_BRIDGE_CONSTANT`, `PAPER_GSTAR_IDENTITIES`, `PAPER_TWO_RACES`, `PAPER_MISSING_RATIO`, `FTD_Discrete_Continuous_Bridge`) repeat the exact same Wallis-type formulation. Optional cosmetic restatement to "for any precision $\varepsilon$, there exists $N$ such that $|S_N - G^*| < \varepsilon$" would be the cleanest portfolio-wide alignment.

2. **`\beta\to\infty` is a parametric (per-system inverse-temperature) limit, not a structural-extent limit.** Six papers use this (`PAPER_LIFECYCLE_SOFTPLUS`, `DERIV_SOFTPLUS_RELU_DUALITY`, `DERIV_CASIMIR_RATCHET`, `DERIV_SONOLUMINESCENCE`, plus minor uses elsewhere). All survive Q1–Q4 of the decision procedure. The single exception is `DERIV_SOFTPLUS_RELU_DUALITY` line 484 where β→∞ is *equated to* the "deep IR continuum limit … fixed point" — that import of completed-RG language is the load-bearing problem.

3. **"Continuum limit" as a defining operation appears in three papers** as load-bearing: `FTD_Navier_Stokes.tex` (sections 4 and 5), `PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY.tex` (3 uses), `DERIV_SOFTPLUS_RELU_DUALITY.tex` (1 use). All require RESTATE to small-spacing-expansion / finitary EFT-matching language.

4. **"Thermodynamic limit" as load-bearing appears in two papers**: `FTD_Yang_Mills_Mass_Gap.tex` (central proposition) and `PAPER_2A_MASTER_QUADRATIC.tex` (one open promissory note). The Yang-Mills case is the more serious — the entire "infinite-volume mass gap" subsection is its premise. The PAPER_2A case is just a future-work remark.

5. **`\Z^3` notation is heavily used across the portfolio (~30 instances per paper in algebraic-geometry papers) and is permitted as a *notational shorthand for the undefined-boundary cubic substrate*** as long as no operation requires the totalized set. The Finitude paper would benefit from a one-paragraph preamble re-licensing this notation under the reframe; other papers can rely on a portfolio-wide convention.

6. **Two "speculative" papers do most of the load-bearing infinity-work**: `FTD_Yang_Mills_Mass_Gap` and `FTD_Navier_Stokes`. Both attack Clay-Millennium problems whose statements *literally require completed-infinity objects* ($\R^4$, smooth solutions on $\R^3$). Under the reframe, the FTD claim is necessarily restricted: "FTD provides finitary analogues that imply the Clay statement *would* hold under continuum reconstruction, which we do not perform." This is honest but it is also a downgrade in the portfolio's claim-strength.

7. **No paper in the portfolio invokes "Hilbert space of the universe", "all paths", or "all configurations" as the *positive* central object** outside the Yang-Mills paper line 207. Most other invocations of `\R^3` / completed totalities are in *quotation-of-standard-physics* mode (Finitude, Navier-Stokes Clay-statement passages) — they survive the reframe because their function is to dispatch the proscribed object, not assert it.

8. **Clean papers** (10/34) cluster around: (a) the algebraic-geometry papers (0A_PERIOD_DESCENT, 0B_THREE_CONSTANTS, 0E_ARITHMETIC_GEOMETRIC, 3A_PHYSICAL_IDENTIFICATION) which are pure algebra; (b) the philosophy/short-form papers (PAPER_PATH, README, PAPER_RATIO_AND_THE_ARROW, ratio_and_the_arrow, LETTER_HERMITIAN_COPE, DERIV_GEOMETRIC_BIOPHYSICS).

---

## Recommended priority order for Phase 4 restatement

Ranked by impact-of-restatement × tractability-of-restatement:

1. **`PAPER_2A_MASTER_QUADRATIC.tex`** — central paper of the framework, only 2 lines need finitary rephrasing (lines 473, 475), both in open-conjecture / future-work passages. Highest-impact lowest-cost.
2. **`PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY.tex` (+ `.md` mirror)** — 3 uses of "continuum limit" in matched body text, all mechanically rewritable as small-spacing expansions / EFT-matching. Medium impact, very tractable.
3. **`src/DERIV_SOFTPLUS_RELU_DUALITY.tex`** — line 484 alone needs care (RG fixed-point identification). One-sentence rewrite.
4. **`src/FTD_Discrete_Continuous_Bridge.tex`** — line 602 needs finitary rephrasing of "lattice spacing sent to zero". Single load-bearing line.
5. **`speculative/FTD_Finitude_Theorem.tex`** — preamble paragraph re-licensing $\Z^3$ notation. The paper is already the reframe-in-action; this is alignment, not surgery.
6. **`speculative/FTD_Yang_Mills_Mass_Gap.tex`** — RE-DERIVE the "infinite-volume mass gap" proposition as a per-region statement. Substantive content survives; framing is the issue. Higher cost, higher payoff (this is the Clay-attack paper).
7. **`speculative/FTD_Navier_Stokes.tex`** — RE-DERIVE the lattice-to-continuum bridge, or restate Clay-attack as "finitary analogue + open continuum-reconstruction conjecture". Higher cost; the central regularity claim ($C^\infty$ on $\R^3$) likely cannot survive without auxiliary work and may need demotion.
8. `PAPER_LIFECYCLE_SOFTPLUS.tex` — one cosmetic restate (line 769 framing).
9. `PAPER_GSTAR_BRIDGE_CONSTANT.tex`, `PAPER_GSTAR_IDENTITIES.tex`, `PAPER_TWO_RACES.tex`, `PAPER_MISSING_RATIO.tex` — optional portfolio-wide Wallis rewording for stylistic uniformity. Zero claim-content change.
10. `speculative/FTD_Riemann_Hypothesis.tex` — recommended deeper user read (this scan checked trigger phrases only; "all primes" / "all zeros" content not scanned in detail).

---

## Methodological notes

- Trigger phrases applied: `thermodynamic limit`, `continuum limit`, `infinite lattice`, `infinite-volume`, `UV/IR limit`, `UV/IR fixed point`, `all configurations`, `path integral over`, `in the limit`, `\to \infty`, `\rightarrow \infty`, `\mathbb{Z}`-notation occurrences (case-insensitive).
- Each hit was classified using CANONICAL_REFRAME.md's Q1–Q4 decision procedure.
- **Limitation:** scan is over trigger *phrases*. Latent completed-infinity reasoning that does not surface as a trigger phrase (e.g., implicit appeal to all primes in zeta-function manipulations, implicit infinite sums, completed-totality framings without the keyword "infinite") is not caught. Recommend deeper read of `FTD_Riemann_Hypothesis.tex`, `PAPER_2A_MASTER_QUADRATIC.tex` (gap equation derivation), and `FTD_Yang_Mills_Mass_Gap.tex` (path-integral construction) for latent issues.
- This audit flagged but did not fix. Phase 4 is the restatement pass.
