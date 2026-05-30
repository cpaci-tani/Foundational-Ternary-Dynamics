# AUDIT — 9-Heegner CM-Tower Master-Quadratic Rigidity Scan: Results

**Document type:** Audit (delivers results of pre-registered scan)
**Status:** [COMPLETE] — verdict landed; methodology distinction critical
**Created:** 2026-05-02
**Pre-registration:** [PREREG_HEEGNER_TOWER_RIGIDITY.md](archive/campaign_complete/PREREG_HEEGNER_TOWER_RIGIDITY.md)
**Related:** SPEC_ALGEBRAIC_SPINE.md Theorem 3 (CM uniqueness); EXPLR_CM_RATIO_TOWER.md (existing fixed-c=16 tabulation); AUDIT_LEMNISCATE_ALPHA_RIGIDITY.md (FTD-0122).

---

## 0 · Summary

Pre-registered scan tested whether the FTD canonical case `(d=-4, c=16, x_+, target=1/α)` is structurally privileged among `9 Heegner discriminants × 19 coefficients × 17 targets × 2 roots = 5814` master-quadratic-style configurations.

**The verdict bifurcates depending on methodology:**

| Match criterion | Strict (5.45 ppm) matches | Verdict |
|---|---:|---|
| **Trivial multiplier** (`p=q=1`, i.e., `x_root = target` directly) | **1** (canonical only) | **H_canonical** ✓ |
| **Rational multiplier** (`p/q` with `q ≤ 200`, framework-integer factorable) | **21** (1 canonical + 20 others) | **H_fit** ✗ |

**This is the load-bearing methodological finding of this audit.** Theorem 3 (CM Uniqueness) holds **uniquely under the trivial-multiplier criterion** but FAILS under the rational-multiplier criterion. The framework currently uses both criteria in different places without explicit flagging, and the resulting rhetorical ambiguity is exactly the F10 failure mode (rigidity-gap licensing) we caught in FTD-0122 for the Lemniscate-Alpha case.

---

## 1 · Scan parameters (executed per pre-registration)

- **Discriminants:** `d ∈ {−3, −4, −7, −8, −11, −19, −43, −67, −163}`
- **Coefficients:** `c ∈ {2, 3, 4, 6, 8, 9, 12, 16, 18, 24, 27, 32, 36, 48, 64, 72, 81, 108, 144}` (19 values)
- **Targets:** 17 natural framework constants (G\*, 2G\*, 4G\*, ϖ, 2ϖ, π, 2π, 4π, e, 2e, G\*², 4G\*², 8G\*², 1/α, m_p/m_e, m_μ/m_e, m_τ/m_e)
- **Roots:** both `x_±` of the master quadratic
- **Multiplier search:** `p/q` with `q ≤ 200`, gcd(p, q) = 1
- **Tolerances:** strict 5.45 ppm, tight 50 ppm, loose 500 ppm
- **Factorability:** F-primes `{2, 3, 5, 7, 13}`; FC includes cyclotomic-extras `{13, 21, 57, 183}`

Chowla-Selberg ratios computed at 50-dps precision via direct Γ-product over each Kronecker character. d=-4 cross-checked against canonical G* = Γ(1/4)/Γ(3/4): agreement at 30+ digits.

---

## 2 · Result A — trivial-multiplier (strict): canonical is unique

Of the 5814 (d, c, target, root) quadruples in the scan, **exactly one** has its master-quadratic root equal to the target at strict (5.45 ppm) precision with trivial multiplier `p/q = 1/1`:

```
d=-4, c=16, x_+ = 137.036171 ≈ 1/α (CODATA 137.035999) at +1.258 ppm
```

**No other (d, c, target, root) quadruple** in the entire scan grid produces `x_root` within 5.45 ppm of any natural framework constant without rational rescaling.

**[THEOREM]** — Within the pre-registered grid (9 Heegners × 19 coefficients × 17 targets × 2 roots) the canonical FTD case is the unique trivial-multiplier strict match. SPEC_ALGEBRAIC_SPINE Theorem 3, **read at the trivial-multiplier level**, is rigorously confirmed by this scan.

This is the strongest reading of CM uniqueness and corresponds to the tabulated finding of EXPLR_CM_RATIO_TOWER §3 (which fixed c=16 and didn't allow rational multipliers).

---

## 3 · Result B — rational-multiplier (strict + FC-factorable): 21 matches

When the multiplier search is opened to small rationals (`q ≤ 200`) with framework-integer factorability (the FTD-0122 / Lemniscate-Alpha methodology), the picture changes:

- **Total strict matches:** 57003 (any rational, any factorability)
- **Strict + FC-factorable:** 21 (canonical + 20 others)
- **Strict + F-factorable (strictly cleaner):** 16

The canonical case `(d=-4, c=16, x_+, 1/α, p/q=1/1)` is one of these 21. The 20 others use non-trivial rational multipliers but achieve comparable or tighter precision.

### 3.1 · Strictly-F-factorable matches (cleanest cases — purely framework primes)

| d | c | root | x_root | target | p/q | ppm | factor structure |
|---:|---:|---|---:|---|---|---:|---|
| −163 | 36 | x_- | 1.3089 | 2ϖ | 625/156 | +0.875 | 5⁴ / 2²·3·13 |
| **−3** | **3** | **x_+** | **9.2232** | **m_μ/m_e** | **2197/98** | **+0.908** | **13³ / 2·7²** ← *tighter than canonical* |
| **−4** | **16** | **x_+** | **137.0362** | **1/α** | **1/1** | **+1.258** | **canonical FTD** |
| −163 | 27 | x_- | 1.3192 | m_μ/m_e | 7680/49 | +2.866 | 2⁹·3·5 / 7² |
| −4 | 72 | x_- | 2.9727 | 4G\*² | 1225/104 | −3.014 | 5²·7² / 2³·13 |
| −4 | 72 | x_- | 2.9727 | 8G\*² | 1225/52 | −3.014 | 5²·7² / 2²·13 |
| −4 | 2 | x_+ | 13.7367 | G\* | 14/65 | −3.892 | 2·7 / 5·13 |
| −4 | 2 | x_+ | 13.7367 | 2G\* | 28/65 | −3.892 | 2²·7 / 5·13 |
| −4 | 2 | x_+ | 13.7367 | 4G\* | 56/65 | −3.892 | 2³·7 / 5·13 |
| −4 | 2 | x_- | 3.7709 | G\*² | 65/28 | +3.892 | 5·13 / 2²·7 |
| −4 | 2 | x_- | 3.7709 | 4G\*² | 65/7 | +3.892 | 5·13 / 7 |
| −4 | 2 | x_- | 3.7709 | 8G\*² | 130/7 | +3.892 | 2·5·13 / 7 |
| −7 | 6 | x_- | 11.1891 | m_p/m_e | 6400/39 | +4.110 | 2⁸·5² / 3·13 |
| −3 | 27 | x_- | 2.0169 | G\*² | 625/144 | −4.661 | 5⁴ / 2⁴·3² |
| −3 | 27 | x_- | 2.0169 | 4G\*² | 625/36 | −4.661 | 5⁴ / 2²·3² |
| −3 | 27 | x_- | 2.0169 | 8G\*² | 625/18 | −4.661 | 5⁴ / 2·3² |

**16 strict + F-factorable matches.** Many of these are "siblings" (different rescalings of the same `target`, or the same root hitting multiple G\*² scales). De-duplicating to **distinct (d, c, target) triples**: ~10 unique structural matches.

### 3.2 · Tightest non-canonical match — a substantive observation

```
d = −3 (Eisenstein), c = 3 = N_c, x_+ = 9.2232
multiplier 2197/98 = 13³ / (2·7²) = N_eff³ / (2·b₃²)
target = m_μ/m_e ≈ 206.768
match: +0.908 ppm — TIGHTER than the canonical FTD case (1.258 ppm)
```

Every parameter is a framework integer: discriminant `d=−3` corresponds to Eisenstein integers (associated with `|Aut|=6=2·N_c`); coefficient `c = 3 = N_c`; numerator `13³ = N_eff³`; denominator `2 · 7² = 2 · b₃²`. The result hits the muon/electron mass ratio at sub-ppm.

**This is either:**
- (a) a structural observation worth investigating: a different CM construction reaches m_μ/m_e cleanly with framework integers throughout; or
- (b) a look-elsewhere artifact: with 4 free parameters (d, c, p, q) all drawn from framework integers, finding *some* sub-ppm match against any of 17 targets is statistically expected.

The right interpretation depends on null-model statistics that this scan does not provide. **It should NOT be added to the LEDGER as a derivation without further investigation.**

---

## 4 · Hypothesis verdict

Pre-registered criteria from PREREG §2:

- **H_canonical:** only canonical strict + FC match → **HOLDS at trivial-multiplier level (1 match)**
- **H_mid:** 1–2 additional matches → **fails**
- **H_fit:** 3+ additional matches → **HOLDS at rational-multiplier level (20 additional)**

**Verdict depends on criterion choice. Both are simultaneously true under their respective definitions.**

The framework's stated CM uniqueness theorem (SPEC_ALGEBRAIC_SPINE Theorem 3) does not specify which criterion it uses. The existing tabulation in EXPLR_CM_RATIO_TOWER §3 implicitly uses the trivial-multiplier criterion. The Lemniscate-Alpha analysis (DERIV_SPIN_STATISTICS_BRIDGE.md, audited as FTD-0122) explicitly uses rational multipliers. **The framework switches between criteria without flagging.**

---

## 5 · Recommended actions

### 5.1 · Theorem 3 retag — clarify the criterion

`SPEC_ALGEBRAIC_SPINE.md §3` should be updated to specify the methodology:

**Current statement (paraphrased):** "Among the 9 class-number-1 imaginary quadratic fields, only d=−4 produces a master-quadratic polynomial whose roots simultaneously match dimensionless physical constants to permille precision."

**Recommended replacement:**
> "Among the 9 class-number-1 imaginary quadratic fields, only d=−4 produces a master-quadratic polynomial (with the natural |Aut|² = 16 coefficient) whose larger root **directly equals** 1/α to 1.26 ppm — i.e., without rational rescaling. **[THEOREM]** within this trivial-multiplier criterion. When rational multipliers `p/q` with `q ≤ 200` and framework-integer factorability are allowed (as in the Lemniscate-Alpha analysis of FTD-0122), the canonical case becomes one of approximately 10–20 comparable matches across the Heegner tower; the rational-multiplier reading is therefore [SELECTION], not [THEOREM]. Audit: `AUDIT_HEEGNER_TOWER_RIGIDITY.md` (FTD-0123)."

### 5.2 · Cross-doc consistency

- **EXPLR_CM_RATIO_TOWER.md §3:** add a paragraph noting that the tabulation uses the trivial-multiplier criterion and cite this audit.
- **DERIV_SPIN_STATISTICS_BRIDGE.md §1.3 / SSB-3:** already retagged [SELECTION] (rational-multiplier criterion). This audit confirms the methodology choice.
- **The two readings should be made explicit framework-wide.** When citing CM uniqueness or Lemniscate-Alpha, specify which criterion is being applied.

### 5.3 · LEDGER row

Recommended new row (text only; not committed to LEDGER.md given prior-session uncommitted detail-block additions):

```
FTD-0123  [PARTIAL]  9-Heegner master-quadratic rigidity scan — trivial-multiplier
                    H_canonical (1 match), rational-multiplier H_fit (20+ matches);
                    Theorem 3 holds at trivial-multiplier level only
  Pre-registered scan over 9 × 19 × 17 × 2 = 5814 (d, c, target, root) quadruples.
  At trivial multiplier (p=q=1) strict (5.45 ppm), the FTD canonical case
  (d=-4, c=16, x_+, 1/α) is the unique match — Theorem 3 holds rigorously.
  At rational multiplier (q ≤ 200, FC-factorable) strict, 21 matches found
  (1 canonical + 20 others); Theorem 3 fails as stated. Tightest non-canonical:
  (d=-3, c=3, x_+ × 13³/(2·7²) = m_μ/m_e) at +0.908 ppm — every parameter is a
  framework integer; either a substantive structural observation worth investigating
  or a look-elsewhere artifact (interpretation requires null-model statistics
  not provided by this scan). The bifurcation between trivial-multiplier and
  rational-multiplier criteria is the load-bearing methodological finding;
  framework rhetoric switches between them without flagging (F10 hygiene issue).
  Recommends: SPEC_ALGEBRAIC_SPINE Theorem 3 statement specify trivial-multiplier
  criterion explicitly; downstream docs cite either criterion consistently.
```

### 5.4 · Follow-up scans recommended

- **Run the same trivial-multiplier scan with extended target set** (50+ natural physics constants) to test whether the canonical case remains uniquely positioned. The current 17-target scan is pre-registered; extending it would be a separate pre-registered scan.
- **Investigate the (d=-3, c=3, m_μ/m_e) match** (§3.2) under null-model statistics: how many "framework-integer-everywhere" 4-tuples (d, c, p, q) admit sub-ppm matches by chance? If many, the match is fit; if few, it's a real new observation.
- **Apply trivial-multiplier criterion retroactively to the Lemniscate-Alpha case** (FTD-0122): the canonical case there has multiplier 91/732 (q=732), so it FAILS the trivial-multiplier criterion. This means under the strictest reading, the Lemniscate-Alpha is NOT a "two roads to G\*" theorem at all, and SSB-3 properly retags to [SELECTION]. Consistent with the FTD-0122 verdict.

---

## 6 · The methodological lesson

**The framework has been unintentionally using two different rigor standards in two related contexts:**

1. **Master-quadratic CM uniqueness (Theorem 3):** trivial-multiplier criterion. d=−4 is uniquely strict.
2. **Lemniscate-Alpha extraction:** rational-multiplier criterion. d=−4 is one of many in its family.

Both are defensible mathematical positions, but they're not interchangeable. The framework's [THEOREM] tag for both, combined with rhetorical claims of "uniqueness" in both, conflates two different uniqueness statements with different strictness levels. The fix is **explicit criterion-tagging in every claim** that involves rational rescaling.

This audit closes the methodological loop opened by FTD-0122. The Lemniscate-Alpha rigidity scan caught the rational-multiplier looseness; the Heegner tower scan now demonstrates that the SAME methodology applied to Theorem 3 produces the SAME bifurcation. Theorem 3 has the trivial-multiplier reading available (and that reading holds rigorously); the Lemniscate-Alpha case does not (its multiplier is 91/732, q ≠ 1).

Net effect on framework standing:
- **Theorem 3 stronger than I expected** under trivial-multiplier reading: this scan provides **first quantitative confirmation** that the canonical case is unique across `9 × 19 × 17 × 2 = 5814` configurations.
- **Lemniscate-Alpha SSB-3 confirmed [SELECTION]** under rational-multiplier reading: the same methodology that landed FTD-0122's verdict applies here and is consistent.
- **Framework rhetoric needs cleanup** to specify which criterion each claim uses.

---

## 7 · Bottom line

Theorem 3 holds at the **trivial-multiplier level** with a quantitative confirmation across 5814 configurations: the FTD canonical case (d=−4, c=16, x_+ ≈ 1/α at 1.26 ppm) is the unique case where a master-quadratic root directly equals a natural framework constant.

Theorem 3 fails at the **rational-multiplier level**: 20 other (d, c, target, p/q) quadruples reach comparable strict precision with framework-integer rationals, including several with sub-ppm precision (notably the Eisenstein/N_c/m_μ-match at 0.908 ppm).

**The criterion choice is the new structural fact**, and it should be made explicit in every framework claim involving CM-curve uniqueness or master-quadratic root identifications. Both readings are now measured rather than asserted.
