# AUDIT — Rational-Integer Fit Claims (Option 4 sub-audit)

**Tag:** [AUDIT]
**Trigger:** Option 4 from Phase I follow-through menu — apply the same
rigidity method used on the master quadratic to the other
[THEOREM]/[DERIVED] claims in `CATALOG_PARAMETRIC_INSERTIONS.md`.
**Date:** 2026-04-19
**Scope:** the electron-mass formula, proton/electron ratio, sin²θ_W,
α_s(M_Z), PMNS mixing angles, and the Δm² ratio — all currently tagged
[THEOREM] or [DERIVED].
**Verdict:** **several of these claims fail the rigidity test at their
own experimental precision**. Specifically, sin²θ_W = 3/13 and
sin²θ_13 = 1/52 are overstatements that should be downgraded; the
PMNS angles and α_s are loose fits tagged stronger than warranted.
Only the master quadratic / m_p/m_e / m_e survive as genuinely
structurally-motivated claims, and even those are [STRONGLY
MOTIVATED CONJECTURE] rather than [THEOREM].

---

## 1 · Method

For each rational-integer claim `p/q`, compute:
1. The FTD-claim numerical value
2. The experimental value (PDG 2024 / CODATA)
3. FTD's relative error vs experiment (in ppm)
4. The experimental precision in ppm
5. The count of **competitor** rationals in `{p/q : p ≤ 200, q ≤ 60, gcd(p,q)=1}`
   that fit experiment within the same tolerance

If the FTD claim is (a) within experimental precision AND (b) the unique
small-rational that does so, the [THEOREM] / [DERIVED] tag is justified.
If either fails, the tag is overstated.

Script: `scripts/proofs/audit_ratio_formulas.py`.

## 2 · Results

| Claim | FTD formula | FTD value | Exp | FTD err (ppm) | Exp precision | Competitors within FTD precision |
|---|---|--:|--:|--:|--:|--:|
| m_p / m_e | N_eff/α + N_base·N_eff + N_c | 1836.47 | 1836.15 | **173** | ~30 ppm | (complex formula, no rigidity scan) |
| α_s(M_Z) = 7/59 | b_3 / (something) | 0.1186 | 0.1179 | **6311** | ~8500 | **1** (2/17 = 0.1176, fits BETTER at 0.3%) |
| sin²θ_W = 3/13 | N_c / N_eff | 0.2308 | 0.2229 | **35 304** | ~20 ppm | **2** within 1%; 2/9 fits at 0.31% vs FTD's 3.5% |
| sin²θ_12 = 3/10 | N_c / (N_c + b_3) | 0.3000 | 0.3070 | **22 801** | ~6500 | **4** within 2.3% |
| sin²θ_23 = 16/29 | (N_eff + N_c) / (2N_eff + N_c) | 0.5517 | 0.5460 | **10 484** | ~6400 | **3** within 1%, including 11/20 (closer) |
| sin²θ_13 = 1/52 | 1/(N_base · N_eff) | 0.01923 | 0.0220 | **125 874** | ~3400 | FTD err is **12.6%**, 37× exp precision |
| Δm²₃₁/Δm²₂₁ = 100/3 | (b_3+N_c)²/N_c | 33.33 | 32.80 | **16 260** | ~9000 | 1 (33/1 also within 1%) |

## 3 · Findings by claim

### 3.1 · sin²θ_W = 3/13 — [THEOREM] tag INCORRECT

- FTD prediction 0.2308 vs experimental 0.2229 → **3.53% error**
- Experimental precision is **20 ppm** (1700× tighter than the FTD claim)
- Within 1% of experiment, the rational **2/9 = 0.2222** fits **better**
  (0.31% error vs FTD's 3.5%); neither has any obvious Moore-neighborhood
  interpretation
- **This is not a theorem.** It is a loose structural coincidence whose
  claim to precision is wildly overstated.

**Recommended status:** [CONJECTURE] or [PARAMETRIC] rather than
[THEOREM]. If N_c / N_eff = 3/13 is truly the predicted value, then FTD
predicts sin²θ_W ≠ experimental value at 1700σ — a falsifying result
that FTD would need to explain, not a derivation.

### 3.2 · sin²θ_13 = 1/52 — [DERIVED] tag INCORRECT

- FTD prediction 0.01923 vs experimental 0.0220 → **12.6% error**
- This is 37× the experimental precision
- **No small-rational competitors fit within 12% either**; the issue is
  that experimental 0.022 is simply not 1/52. The formula is off.

**Recommended status:** [PARAMETRIC] or explicit mis-prediction. The
0.0220 experimental value is closer to 1/45 (0.0222, 1.0%) or 1/46
(0.0217, 1.2%), neither of which has Moore-neighborhood meaning.

### 3.3 · α_s(M_Z) = 7/59 — [DERIVED] tag is soft

- FTD prediction 0.11864 vs experimental 0.1179 → **0.63% error**
- Experimental precision is ~8500 ppm (0.85%); FTD sits at 0.63%, just
  inside experimental precision
- **Competitor 2/17 = 0.1176 fits better** (0.29% error); 2/17 has no
  natural Moore-neighborhood interpretation
- The formula "α_s = b_3 / 59" is built with a specific denominator (59)
  that is NOT a structural Moore number — the denominator was chosen to
  make the ratio hit 0.1186

**Recommended status:** [PARAMETRIC]. The 7 in the numerator IS b_3
(Moore-neighborhood integer), but the 59 denominator is not structurally
motivated; it was chosen to match α_s.

### 3.4 · PMNS sin² angles — [DERIVED] tags overstate precision

All three PMNS sin² angles sit at 1-2.3% error vs. experimental
precision of 0.6-0.9%. Each has 2-4 small-rational competitors within
the same precision band. The formulas are rational combinations of
{N_c, N_base, N_eff, b_3} so they have some structural content, but
multiple such combinations hit each experimental value.

**Recommended status:** [STRUCTURALLY MOTIVATED PARAMETRIC]. These are
algebraically "nice" rationals built from the Moore integers, but they
are not unique within their class, and the agreement is at 1-2% where
experiment gives ~0.1%.

### 3.5 · Δm²₃₁/Δm²₂₁ = 100/3 — [DERIVED] tag is soft

- FTD prediction 33.33 vs experimental 32.8 → **1.63% error**
- Experimental precision is ~9000 ppm (0.9%); FTD is just outside
- Competitor 33/1 fits within 1% and is simpler

**Recommended status:** [STRUCTURALLY MOTIVATED PARAMETRIC].

### 3.6 · m_p/m_e formula — surviving as [DERIVED] at ~173 ppm

- FTD prediction 1836.47 vs experimental 1836.15 → **173 ppm**
- Experimental precision is ~30 ppm (0.003%); FTD is 5.8× above
  experimental precision but within an order of magnitude
- The formula uses THREE independent Moore integers and α as inputs
  (N_eff, N_base, N_c, α) — harder to dismiss as a rational fit since
  it involves α, not just p/q
- This formula is closer to the master quadratic's epistemic tier:
  [STRONGLY MOTIVATED CONJECTURE] with structural content

**Recommended status:** [DERIVED] survives but with caveat that 173 ppm
is 5.8× above experimental precision; a 1-loop refinement (or a more
precise derivation) is warranted.

### 3.7 · Electron mass m_e = m_P·√(2π)·(16/3)·α¹¹ — [DERIVED] with caveats

(Separate audit: `scripts/proofs/audit_electron_mass_formula.py`)

- FTD prediction 0.51002 MeV vs experimental 0.51100 MeV → **0.19% error**
- Among 6489 rational-prefactor + integer-exponent combinations
  (p, q ≤ 50, n ∈ [8, 14]): only **2 combinations** fit within 1%, and
  the FTD one (16/3, n=11) is the tighter of the two
- At the exponent n = 11, the FTD prefactor 16/3 is the unique small
  rational that fits within 0.2%; the competitor 43/8 is at 0.6%
- The exponent n = 11 is itself structurally motivated: n=10 is too
  large (1.4% off), n=12 requires a prefactor far from any small
  rational (8055/11 to hit ppm)

**Recommended status:** [STRONGLY MOTIVATED CONJECTURE] rather than
[DERIVED]. The formula is structurally tight at 0.2% precision, but
the claim of "derivation" should be qualified — neither the prefactor
16/3 nor the exponent 11 has been derived from first principles
dynamically.

## 4 · Summary table — recommended catalog revisions

| Claim | Current tag | Recommended tag | Justification |
|---|---|---|---|
| G* | [THEOREM] | [THEOREM] | Pure math identity, no change |
| α (master quadratic) | [STRONGLY MOTIVATED CONJECTURE] (already downgraded) | unchanged | Already fixed in Phase I core audit |
| N_c | [THEOREM] + [CONJECTURE] (already mixed) | unchanged | Already accurate |
| {N_base, N_eff, b_3} | [THEOREM] | [THEOREM] | Moore integers, structurally proven |
| G_C = √α | [THEOREM] | [DEFINITION] | Conditional on α; not an independent theorem |
| **sin²θ_W = 3/13** | **[THEOREM]** | **[PARAMETRIC]** | **3.5% error, 1700× exp precision; 2/9 fits better** |
| m_e formula | [DERIVED] | [STRONGLY MOTIVATED CONJECTURE] | 0.19% error, structurally tight among peers |
| m_μ/m_e | [DERIVED] | (not audited; likely [PARAMETRIC]) | Similar epistemic tier probable |
| m_p/m_e | [DERIVED] | [STRONGLY MOTIVATED CONJECTURE] | 173 ppm, 5.8× exp precision |
| **α_s(M_Z) = 7/59** | **[DERIVED]** | **[PARAMETRIC]** | **59 not structural; 2/17 fits better** |
| **sin²θ_12 = 3/10** | **[DERIVED]** | **[STRUCTURALLY MOTIVATED PARAMETRIC]** | **2.3% error, 4 competitors** |
| **sin²θ_23 = 16/29** | **[DERIVED]** | **[STRUCTURALLY MOTIVATED PARAMETRIC]** | **1.0% error, 3 competitors** |
| **sin²θ_13 = 1/52** | **[DERIVED]** | **[PARAMETRIC]** or retract | **12.6% error, 37× exp precision** |
| **Δm²₃₁/Δm²₂₁ = 100/3** | **[DERIVED]** | **[STRUCTURALLY MOTIVATED PARAMETRIC]** | **1.6% error, simpler 33/1 fits** |

## 5 · Meta-observation

Before today's audit cycle, the catalog listed ~23 [DERIVED]/[THEOREM]
claims. After Phase I core (master quadratic), one was downgraded.
After Option 4 (this audit), **7 more should be downgraded**:
sin²θ_W, sin²θ_12, sin²θ_23, sin²θ_13, Δm²₃₁/Δm²₂₁, α_s, and m_e (to
STRONGLY MOTIVATED CONJECTURE rather than [DERIVED]).

**Honest count after this audit:** ~5 claims remain at firm [THEOREM]
(G*, N_c topology, Moore integers, Emergent Coulomb Green's function,
structural null predictions). ~8 claims sit at [STRONGLY MOTIVATED
CONJECTURE]. ~10 claims previously tagged [DERIVED] are structurally
motivated but not unique within small-rational families and should be
tagged [STRUCTURALLY MOTIVATED PARAMETRIC].

This is a substantial but healthy narrowing. The project's genuinely
novel content (the master quadratic dual-match, the CM curve
uniqueness, the Moore-neighborhood integer structure, the emergent-
Coulomb lattice identity) is real. The surrounding "1/137 to 0.001 ppt
from 23 independent derivations" framing is overstated and should be
replaced with "a core structural insight + several structurally-
motivated fits at 1-2% precision."

## 6 · Reproducibility

```
scripts/proofs/audit_electron_mass_formula.py  # electron mass rigidity
scripts/proofs/audit_ratio_formulas.py         # all rational-integer claims
docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md  # catalog to update
```

Running both scripts reproduces the tables above.
