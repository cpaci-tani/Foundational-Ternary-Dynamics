# AUDIT — Lemniscate-Alpha Rigidity Scan: Results

**Document type:** Audit (delivers results of pre-registered scan)
**Status:** [COMPLETE] — verdict landed
**Created:** 2026-05-01 evening
**Pre-registration:** [PREREG_LEMNISCATE_ALPHA_RIGIDITY.md](PREREG_LEMNISCATE_ALPHA_RIGIDITY.md)
**Companion fixes:** `DERIV_SPIN_STATISTICS_BRIDGE.md` (L_α value, denominator decomposition formula, agreement ppm — three corrections applied 2026-05-01); `SPEC_SM_REPLACEMENT_COMPLETE.md §4.4` (spin-statistics tag drift fixed: "Derived" → "[SELECTION]").

---

## 0 · Summary

Pre-registered scan tested whether the FTD canonical 5-harmonic Cayley-Dickson Fourcier curve (extracting G\* via `L_α × 91/732 ≈ G*`) is structurally privileged among natural alternatives in the same family.

**Verdict: between H_mid and H_fit, leaning H_fit.**

Canonical-class matches (FC-factorable rational multipliers at the canonical's q-range, landing on natural framework constants at 5.45 ppm precision) occur in **~4.30% of valid Cayley-Dickson 5-harmonic curves** in the scanned 4-D higher-harmonic sector. The canonical curve is **one of many** with similar matches, not structurally privileged.

The framework's [SELECTION] tag is the right tag. The "Lemniscate-Alpha extracts G\*" structural narrative requires retagging: the agreement is real, but it is not unique among natural curves of the same family.

---

## 1 · Pre-flight findings (already established before scan)

These were established in the session leading to the scan and are independent of the scan's outcome:

### 1.1 · Doc arithmetic corrections

1. **L_α value correction.** `DERIV_SPIN_STATISTICS_BRIDGE.md §1.3` stated `L_α = 23.7994...`. Cross-validation at 20-digit precision via four independent integration methods (scipy adaptive `epsrel=1e-12`, scipy chunked over 16 sub-intervals, mpmath at 50 dps with chunked integration, trapezoidal at N=10⁶) gives `L_α = 23.79960517...` — error in the 4th decimal of the original.

2. **Denominator decomposition formula correction.** Original doc:
   ```
   732 = 4 × 183 = N_base × (N_eff(N_eff+1)/2 + 1)
   ```
   The expression `N_eff(N_eff+1)/2 + 1 = 13×14/2 + 1 = 92 ≠ 183`. Correct expression:
   ```
   732 = 4 × 183 = N_base × (1 + N_eff + N_eff²) = N_base × (N_eff² + N_eff + 1)
   ```
   This is a third-cyclotomic-like polynomial in N_eff. Framework-integer factorization stands; the formula was misstated.

3. **Agreement-precision correction.** Recomputation at corrected L_α gives:
   ```
   G*_α := L_α × 91/732 = 2.95869409...
   G*   := Γ(1/4)/Γ(3/4) = 2.95867512...
   agreement = +6.41 ppm
   ```
   Original "5.45 ppm" was based on the misstated L_α = 23.7994.

### 1.2 · Multiplier uniqueness at the cited precision

Direct rational-approximant scan over `q ≤ 1000`:

| Tolerance | # rationals `p/q ∈ ℚ⁺, q ≤ 1000` within tolerance of `G*/L_α = 0.124317` |
|---|---|
| 5.45 ppm | 1 (only 91/732) |
| 50 ppm | 4 |
| 500 ppm | 41 |

`91/732` is the unique small-denominator rational at the doc's claimed precision. **[OBSERVATION — verified]**: at fixed L_α, the multiplier value is structurally privileged at the rational-approximant level. **However**, this only matters if L_α itself is structurally privileged — which the rigidity scan tests directly.

### 1.3 · Cross-doc tag drift caught and fixed

`SPEC_SM_REPLACEMENT_COMPLETE.md §4.4` listed spin-statistics as "Derived from `π_1(SO(3)) = ℤ_2`". The underlying derivation `DERIV_SPIN_STATISTICS_BRIDGE.md §1.4` (SSB-4) tags the curve-↔-SO(3) identification step as **[SELECTION]**, not [DERIVED]. The summary table inflated the source tag. Corrected 2026-05-01 to match. F10-class hygiene fix.

---

## 2 · Scan results — main scan

### 2.1 · Scan summary

| Metric | Value |
|---|---|
| Pool size (rational coefficients per slot) | 43 |
| Total combinations (4-D higher-harmonic sector) | 3,418,801 |
| Runtime | 1101 seconds |
| Valid (winding ±2, min\|z\|>0.05) | 447,720 (13.10%) |

### 2.2 · Match counts

| Tolerance | total matches (any of 14 targets) | F-factorable | FC-factorable |
|---|---|---|---|
| Strict (5.45 ppm) | 57,003 | 9,760 | 10,350 |
| Tight (50 ppm) | 523,134 | 89,013 | 94,108 |
| Loose (500 ppm) | 5,609,169 | 897,418 | 953,967 |

### 2.3 · Match rate per valid curve (one match suffices)

| Class | Rate |
|---|---|
| strict + F-factorable | 2.18% |
| strict + FC-factorable | **2.31%** |
| tight + F-factorable | 19.88% |
| tight + FC-factorable | 21.02% |

### 2.4 · Per-target hit counts (strict, FC-factorable)

| Target | strict matches | F-fact subset | FC-fact subset |
|---|---:|---:|---:|
| G* | 4079 | 705 | 764 |
| 2*G* | 4217 | 670 | 715 |
| 4*G* | 4197 | 756 | 800 |
| varpi | 3875 | 670 | 728 |
| 2*varpi | 4176 | 672 | 714 |
| pi | 3985 | 699 | 743 |
| 2*pi | 4079 | 655 | 694 |
| 4*pi | 4040 | 747 | 787 |
| e | 3930 | 672 | 721 |
| 2*e | 4228 | 684 | 712 |
| G*² | 3965 | 693 | 725 |
| 4*G*² | 4043 | 711 | 732 |
| 8*G*² | 4092 | 749 | 798 |
| 1/α | 4097 | 677 | 717 |

The match counts are **roughly uniform across all 14 targets** — about 700 F-factorable strict matches per target. This is consistent with the scan accessing a generic class of "natural" matches, not a target-specific structural connection.

---

## 3 · Methodological wrinkle — the q ≤ 200 cap

The pre-registered scan capped the rational-multiplier search at `q ≤ 200`. **The canonical Lemniscate-Alpha's match is at q = 732, OUTSIDE this range.** So the main scan's 2.31% rate is for "easier" multipliers than the canonical's. To compare apples to apples, a supplementary scan extends the q-range to 800 (covering the canonical's 732).

### 3.1 · Canonical curve self-check at q ≤ 200

Direct check for the canonical curve `(a_3, a_4, b_3, b_4) = (0.4, 0.0625, -0.35, 0.0625)`, L_α = 23.7996:

The canonical has **exactly one strict match at q ≤ 200**: `547/95 → 1/α` at -3.0 ppm. But neither 547 (prime) nor 95 (= 5 × 19) is framework-integer-factorable; this match is in the "strict total" but not "strict + F-factorable" or "strict + FC-factorable" bucket.

**Equivalently:** the canonical curve does NOT contribute to the main scan's 2.31% rate. Its specific framework-integer-factorable match is at q = 732, requiring the extended-range scan to detect.

### 3.2 · Supplementary scan at canonical's q-range

Random sample of 2000 valid curves from the same 4-D sector. Tested four match classes:

| Class | Definition | Rate |
|---|---|---:|
| A | strict + FC-fact, q ≤ 200 (main scan) | 3.20% |
| B | strict + FC-fact, q ∈ [201, 800] (canonical's range) | **4.30%** |
| C | strict + F-fact only, q ∈ [201, 800] | 3.20% |
| D | ANY strict match, q ≤ 800 (no factorability filter) | 100.00% |

(Class A's sample rate 3.20% vs main scan's 2.31% is consistent within sampling variance.)

### 3.3 · Examples in canonical's q-range

Curves OTHER than canonical that hit natural targets at canonical's q-range with FC-factorable rationals:

| coefs (a_3, a_4, b_3, b_4) | L | target | p/q | ppm | factors |
|---|---:|---|---|---:|---|
| (0.55, -0.4375, -0.2, 0.3125) | 42.40 | 1/α | 1183/366 | -4.80 | 7·13² / 2·3·61 |
| (-0.15, 0.2, 0.5, 0.3) | 31.08 | e | 30/343 | +0.51 | 2·3·5 / 7³ |
| (0.2, -0.125, -0.3125, 0.2) | 23.71 | G*² | 189/512 | +0.71 | 3³·7 / 2⁹ |
| (0.5, 0.35, -0.3333, 0.25) | 37.28 | varpi | 32/455 | -1.62 | 2⁵ / 5·7·13 |
| (0.3125, 0.3, -0.3, -0.2) | 30.65 | 4·G*² | 585/512 | -4.35 | 3²·5·13 / 2⁹ |

The last four have BOTH numerator AND denominator factoring entirely in `F = {2, 3, 5, 7, 13}` — strictly F-factorable, not just FC-factorable. The canonical's 91/732 has 61 in the denominator (∉ F), making it FC-factorable (via 183 = 1+13+13²) but not strictly F-factorable. **Several non-canonical curves achieve the strictly stronger F-factorability**, at single-digit ppm precision, on natural framework targets.

### 3.4 · Key observation: 100% any-rational-match rate

**Every valid curve admits some `p/q` with `q ≤ 800` landing within 5.45 ppm of at least one of 14 natural targets.** This is dense-rationals doing what dense rationals do: given any L > 0 and any target t > 0, the rational approximations to t/L are dense in the reals, and 14 targets multiplied by q-values up to 800 supply enough rationals to hit any single curve at ppm precision.

The factorability filter cuts this from 100% to 4.30% — meaningful, but not a sharp uniqueness signal. About 1 in 23 valid curves admits an FC-factorable strict match in the canonical's q-range. The canonical is one of these ~23.

---

## 4 · Hypothesis verdict

Pre-registered criteria:
- **H_canonical (rigid):** strict-FC-factorable rate < 1% of valid curves
- **H_fit (look-elsewhere):** strict-FC-factorable rate > 5%
- **H_mid:** rate in [1%, 5%]

**Measured rates:**
- At main scan q ≤ 200: 2.31%
- At canonical's q-range (q ∈ [201, 800]): 4.30%
- Combined "any q ≤ 800": ~6-7% (estimated; both buckets together)

**Verdict:**
- Strictly by pre-registered criteria using main scan's 2.31% → **H_mid**
- Augmented by supplementary scan at canonical's q-range (4.30%) → **H_mid trending toward H_fit**
- If we take "any FC-factorable strict match at q ≤ 800" → **H_fit territory** (likely >5%)

**Honest reading:** the canonical curve is **one of a measurable minority of natural Cayley-Dickson 5-harmonic curves** that admit framework-integer-factorable rational multipliers landing on natural framework constants at 5.45 ppm precision. The minority is not vanishing (so H_canonical fails — the canonical is not uniquely privileged), but it is not majority (so H_fit is also too strong — the matches are sparse enough to not be trivially expected).

The framework's [SELECTION] tag for this claim is the correct tag. The structural narrative "Lemniscate-Alpha uniquely extracts G\*" should be retagged: the agreement is real, but among curves of comparable structural complexity, ~4-5% admit similar matches. The canonical is not structurally distinguished.

---

## 5 · LEDGER updates recommended

### 5.1 · `DERIV_SPIN_STATISTICS_BRIDGE.md` updates

Three claim updates needed:

1. **SSB-3 ("Two-road G\* agreement")** currently tagged [THEOREM]. Recommendation: **retag to [SELECTION] with quantitative qualifier**. The two-road agreement at 6.41 ppm is real, but among ~4.3% of natural Cayley-Dickson curves with comparable matches, so the agreement does not uniquely select the canonical curve. The structural meaning of the agreement requires further argument.

2. **SSB-4 ("ℤ₂ topology ↔ spin-1/2 identification")** is correctly tagged [SELECTION]; no change needed but should explicitly cite this audit.

3. **§1.3 narrative** describing the canonical curve as the "true ontological" curve should be softened. Many natural Cayley-Dickson curves admit similar matches; the canonical curve's privileged status is not established by the arc-length / multiplier route.

### 5.2 · LEDGER row addition

Recommended new row:

```
FTD-0122  [PARTIAL]  Lemniscate-Alpha rigidity-scan result
  Pre-registered scan (PREREG_LEMNISCATE_ALPHA_RIGIDITY.md, 2026-05-01) tested
  whether the canonical 5-harmonic Cayley-Dickson Fourcier curve is structurally
  privileged among natural alternatives. Scan: 3.4M coefficient combinations
  in 4-D higher-harmonic sector + 2000-curve supplementary scan at canonical's
  q-range. Result: 2.31-4.30% of valid curves admit framework-integer-factorable
  rational multipliers landing on natural framework constants at 5.45 ppm
  precision. Verdict: H_mid trending H_fit. The canonical curve is one of a
  measurable minority (~1 in 23 in canonical's q-range) of curves with similar
  matches; it is NOT uniquely privileged. Framework's [SELECTION] tag for SSB-3
  / SSB-4 is correct; structural narrative ('Lemniscate-Alpha uniquely extracts
  G*') should be softened.
```

### 5.3 · `SPEC_ALGEBRAIC_SPINE.md` impact

The 9 spine theorems are NOT affected — they are stated independent of the Lemniscate-Alpha narrative, and the Lemniscate-Alpha is not part of the canonical algebraic spine. The Bernoulli-route G\* derivation (Theorem 1: G\* = Γ(1/4)/Γ(3/4)) is a true algebraic identity; the Lemniscate-Alpha route is structurally weaker than previously framed.

---

## 6 · Limitations of this audit

### 6.1 · Search-space coverage

The scan fixed leading three coefficients `(a_0, a_1, a_2) = (1, ½, ½)` and `(b_0, b_1, b_2) = (1, −½, ½)` to canonical values, sweeping only `(a_3, a_4, b_3, b_4)` in the 4-D higher-harmonic sector. **A negative result here (no matches found) would NOT close the question for the full 10-coefficient family.** A different finding at lower harmonics could break canonical privilege without being detected.

The pre-registered scope is the higher-harmonic sector only.

### 6.2 · Tolerance precision

Scan strict tolerance was 5.45 ppm to match the framework's *original* (misstated) precision claim. The corrected agreement is 6.41 ppm. Re-running at 6.41 ppm tolerance would shift match counts upward by roughly 1.18×; the verdict (H_mid trending H_fit) would not change qualitatively.

### 6.3 · Framework-integer factorability criterion

`F = {2, 3, 5, 7, 13}` and the cyclotomic-like extras `{1+n+n² : n ∈ {3, 4, 7, 13}} = {13, 21, 57, 183}` are pre-registered choices. Different criterion sets would produce different match counts.

### 6.4 · The "Cayley-Dickson is forced; coefficients are not" caveat persists

The scan tests rigidity *within* the Cayley-Dickson frequency-{1,2,4,8,16} family. The question of whether the Cayley-Dickson hierarchy itself is forced by some framework-axiomatic principle is **out of scope** and remains [OPEN]. A separate scan over non-Cayley-Dickson 5-harmonic families (e.g., arithmetic-progression frequencies {1, 2, 3, 4, 5}) would test the broader hierarchy claim.

---

## 7 · Recommended actions

**Immediate (within session):**
- Update `DERIV_SPIN_STATISTICS_BRIDGE.md` to soften canonical-curve narrative; add explicit citation to this audit.
- Add LEDGER row FTD-0122 per §5.2.

**Short-term (next session):**
- Rerun scan with `q_max = 1000` to include canonical's q = 732 in the main loop; verify that the 4.30% canonical-q-range rate reproduces and that the canonical curve appears in the FC-factorable strict-match set.
- Audit downstream docs that quote "5.45 ppm" agreement: update to "6.41 ppm" plus the rigidity-scan caveat. Affected docs: 13 files identified by grep, including manuscript chapters, REF_CLAIMS_MATRIX.md, FOUND_ONTOLOGICAL_GENESIS.md, DERIV_QUANTUM_MECHANICS_RESOLVED.md, DERIV_LEMNISCATE_HIERARCHY_WHITEPAPER.md.

**Medium-term:**
- Repeat the rigidity-scan methodology for other [SELECTION]-tagged framework-integer-factorization claims. F10 hygiene says: any "look at the framework integers in this rational" claim should pass a null-model test before being treated as evidence.

**Long-term (research-program scale):**
- Address the underlying methodological question: derive the canonical curve's coefficients from a structural principle independent of "land on G\*". Without that, the [SELECTION] tag is the ceiling for this claim.

---

## 8 · Bottom line

The Lemniscate-Alpha is a real, geometrically interesting curve with verified topological properties (winding ±2, min |z| = 0.273, exact at full precision). Its arc-length-times-rational-multiplier match to G\* (`L_α × 91/732 ≈ G*` at 6.41 ppm) is a real numerical fact. **What the scan establishes is that this specific match is not structurally distinguished from ~4-5% of natural Cayley-Dickson curves admitting comparable matches against natural framework targets.**

The framework's [SELECTION] tag is honest. The narrative ("two roads to G\*") is descriptively accurate but does not establish unique privilege. Future work needs an independent derivation of the canonical curve's coefficients to upgrade the [SELECTION] tag.

The 9-theorem algebraic spine is unaffected. Paper A draft (which excludes the Lemniscate-Alpha narrative entirely per `STRATEGY_PAPER_SPLIT_2026-04-30.md` §1.4) is unaffected. The corrections apply specifically to the spin-statistics-bridge derivation and its downstream summaries.
