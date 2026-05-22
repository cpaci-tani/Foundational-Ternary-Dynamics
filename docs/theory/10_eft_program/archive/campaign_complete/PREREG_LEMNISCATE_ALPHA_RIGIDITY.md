# PRE-REGISTRATION — Lemniscate-Alpha Rigidity Scan

**Document type:** Pre-registration (locks protocol BEFORE measurement)
**Status:** [PRE-REGISTERED] — committed before scan execution
**Created:** 2026-05-01 evening (post-physics-bridge session)
**Provenance:** Direct extension of FTD-0097 monomial-level look-elsewhere methodology and the polynomial-level look-elsewhere scan (`EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md`), applied to the Cayley-Dickson 5-harmonic Fourcier family hosting the FTD canonical Lemniscate-Alpha curve.
**Related:** `DERIV_SPIN_STATISTICS_BRIDGE.md §1.3-1.4` (canonical curve definition); `AUDIT_LOOK_ELSEWHERE_RESULTS.md` (FTD-0097 monomial scan); `EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md` (polynomial-level scan).

---

## 0 · Why this scan

The FTD canonical Lemniscate-Alpha curve (DERIV_SPIN_STATISTICS_BRIDGE.md §1.3) extracts G\* via the relation `G*_α = L_α × 91/732 ≈ 2.9587` to ~2 ppm. The doc tags the multiplier `91/732` as [SELECTION], with framework-integer factorization (`91 = 7 × 13 = b₃ × N_eff`; `732 = 4 × (1 + N_eff + N_eff²) = 4 × 183`) as supporting rationale.

Today's session (2026-05-01 evening, post-Paper-A) verified two facts:
1. `91/732` is the unique rational `p/q` with `q ≤ 1000` landing within 5.45 ppm of `G*/L_α`. **No simpler approximant works at this precision.**
2. The doc's denominator-decomposition formula contained a typo: `N_eff(N_eff+1)/2 + 1 = 92` (not 183); correct expression is `N_eff² + N_eff + 1 = 183`. **Framework-integer factorization stands; expression was misstated.**

What is NOT yet established:
- Whether the 5-harmonic curve coefficients `(1, ½, ½, ⅖, 1/16)` for x and `(1, −½, ½, −7/20, 1/16)` for y are structurally privileged among "natural" alternatives.
- Whether the framework-integer factorization of the multiplier numerator/denominator is unusually clean, or whether many small rationals near `G*/L` for natural curve coefficients similarly factor.

The present scan addresses both gaps in the FTD-0097 methodology lineage.

---

## 1 · Search space (LOCKED)

**Curve form.** 5-harmonic Cayley-Dickson Fourcier curves
```
x(t) = Σ_{k=0..4} a_k cos(2^k t)
y(t) = Σ_{k=0..4} b_k sin(2^k t)
```
with frequencies forced by Cayley-Dickson `{1, 2, 4, 8, 16}`.

**Coefficient pool (rational, bounded complexity).**
- Leading coefficients FIXED: `a_0 = 1`, `b_0 = 1` (canonical normalization).
- Lower-harmonic coefficients FIXED at canonical: `a_1 = ½, a_2 = ½, b_1 = −½, b_2 = ½` (canonical Lemniscate-Alpha values for harmonics 2 and 4; chosen to test sensitivity of the higher-harmonic sector specifically).
- Higher-harmonic coefficients SWEPT: `a_3, a_4, b_3, b_4` over the rational pool

  ```
  P = { p/q : q ∈ {2, 3, 4, 5, 6, 8, 10, 16, 20}, p ∈ {-q, ..., q}, |p/q| ≤ 0.6 } ∪ {0}
  ```

  After deduplication: pool size ~43 values per slot.
  Total combinations: |P|⁴ ≈ 3.4 × 10⁶ before filtering.

**Filters applied (locked before run):**
1. Winding number ≈ ±2 (tolerance 0.1)
2. Minimum |curve| > 0.05 (well-defined winding, no near-singularity)

**Why these constraints:** the Cayley-Dickson scheme forces frequencies; the framework's spin-statistics argument requires winding-2 topology; the minimum-distance constraint excludes degenerate (origin-passing) curves where winding is ill-defined. Pool bounded at |coefficient| ≤ 0.6 is comparable to canonical values (largest is 0.5).

---

## 2 · Targets (LOCKED)

For each valid curve, we test whether `L × (p/q)` lands close to any of the following natural framework constants:

```
T = { G*, 2 G*, 4 G*, varpi, 2 varpi, pi, 2 pi, 4 pi, e, 2e, G*², 4 G*², 8 G*², 1/alpha }
```

For each (curve, target) pair, we search for the smallest-denominator rational `p/q` (with `q ≤ 200`) such that `L × (p/q)` is within tolerance of the target.

**Tolerances (tiered):**
- Strict: 5.45 ppm (matches canonical claim precision)
- Tight: 50 ppm
- Loose: 500 ppm

---

## 3 · "Framework-integer factorability" criterion (LOCKED)

A rational `p/q` is "framework-integer factorable" iff both `p` and `q`, after factoring into primes, use ONLY primes from the framework integer multiset:

```
F = {2, 3, 5, 7, 13}    (primes appearing in {N_base=4, N_c=3, b_3=7, N_eff=13} 
                         and in 5 = N_base + 1; we EXCLUDE 11 and ≥17)
```

A factorization like `91 = 7 × 13` uses only `F`-primes ✓.
A factorization like `732 = 2² × 3 × 61` uses 61 ∉ `F` ✗ — but the framework cleanly justifies 732 via `4 × 183 = 4 × (1 + 13 + 13²)`, where 183's prime factor 61 emerges from `1 + 13 + 13² = 1 + N_eff + N_eff²`, a cyclotomic-like structural expression in N_eff. We therefore include the secondary criterion:

A rational `p/q` is "framework-cyclotomic factorable" iff each of `p` and `q` is expressible as either:
- a product of `F`-primes, OR
- a product of `F`-primes and one factor of the form `(1 + n + n²)` for some `n ∈ {N_base, N_c, b_3, N_eff}`.

This second criterion captures the canonical 91/732 case and acts as a slightly broader rationalization basket. Counts will be reported BOTH ways.

---

## 4 · Hypotheses to test (LOCKED before measurement)

**H_canonical (the "structurally rigid" reading).** The canonical Lemniscate-Alpha curve `(a_3, a_4, b_3, b_4) = (2/5, 1/16, −7/20, 1/16)` is among the small minority of natural Cayley-Dickson curves whose `L × p/q` admits a uniquely-small framework-integer-factorable rational landing on a natural framework constant at strict precision. Other natural curves predominantly fail this multi-constraint test.

  Concretely: of valid curves in the scan, **fewer than 1%** admit a rational `p/q` with `q ≤ 200` AND `L × p/q` within 5.45 ppm of any target in `T` AND `p, q` both framework-integer factorable.

**H_fit (the "look-elsewhere" reading).** The natural family is rich enough that "Lemniscate-Alpha-grade" multi-constraint matches are common — many random Cayley-Dickson curves admit similar matches.

  Concretely: more than 5% of valid curves admit such matches.

**H_mid (intermediate, ambiguous).** Match rate is in [1%, 5%] — neither rigid nor common. Tag retains [SELECTION] but with measured rather than asserted rigidity.

---

## 5 · Pre-registered conclusions

If H_canonical confirms:
- LEDGER row updates: FTD-0111 [STRONGLY MOTIVATED CONJECTURE] strengthened; the multiplier in FTD-0111-related claims becomes positively constrained
- `DERIV_SPIN_STATISTICS_BRIDGE.md` §1.3 [SELECTION] tag justified by quantitative scan
- New ledger row for the rigidity result

If H_fit confirms:
- LEDGER row updates: FTD-0111 unchanged but doc note added — multiplier is parametric
- `DERIV_SPIN_STATISTICS_BRIDGE.md` §1.3 [SELECTION] tag retained but accompanied by explicit fit acknowledgement
- The "Lemniscate-Alpha extracts G\*" claim is honestly retagged [PARAMETRIC] in light of look-elsewhere result

If H_mid:
- Mixed verdict; tag retains [SELECTION] with measured rigidity number cited in the doc

---

## 6 · Discipline notes

- The scan code, search space, target set, tolerances, and hypothesis criteria are LOCKED at this commit.
- Result will be reported in `AUDIT_LEMNISCATE_ALPHA_RIGIDITY.md` with full match enumeration.
- No tolerance-tightening or scope-narrowing post-hoc.
- If the scan reveals an interesting structural feature NOT captured by the above hypotheses (e.g., a different uniqueness pattern), it is reported separately and pre-registered for follow-up scan rather than retroactively folded into this one.

**Author commit before scan run.** Pre-registration is locked at the commit recording this file. Tag `preregister-lemniscate-alpha-rigidity-v1` to be applied at lock-in commit (per FTD pre-registration discipline established 2026-04-27).
