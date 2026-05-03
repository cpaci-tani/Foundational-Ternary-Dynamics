# PRE-REGISTRATION — 9-Heegner CM-Tower Master-Quadratic Rigidity Scan

**Document type:** Pre-registration (locks protocol BEFORE measurement)
**Status:** [PRE-REGISTERED] — locked at this commit prior to scan execution
**Created:** 2026-05-02
**Provenance:** Direct extension of FTD-0097 (monomial-level look-elsewhere), FTD-0121 (polynomial-level look-elsewhere within Cayley-Dickson Fourcier family), and FTD-0122 (Lemniscate-Alpha rigidity scan). This scan applies the same methodology to the 9-element Heegner CM-tower with master-quadratic-style construction.
**Related:** `EXPLR_CM_RATIO_TOWER.md` (existing tabulation at fixed c=16); `SPEC_ALGEBRAIC_SPINE.md` Theorem 3 (CM uniqueness); `AUDIT_LEMNISCATE_ALPHA_RIGIDITY.md` (FTD-0122 verdict H_mid trending H_fit).

---

## 0 · Question

The framework's CM Uniqueness Theorem (SPEC_ALGEBRAIC_SPINE Theorem 3) asserts that among class-number-1 imaginary quadratic fields, **only d = −4 produces a master-quadratic polynomial whose roots simultaneously match dimensionless physical constants**. The existing audit (`EXPLR_CM_RATIO_TOWER.md` §3) checks the **fixed-coefficient (c = 16)** master quadratic for each of the 9 discriminants and confirms that only d = −4 lands on (1/α, N_c) at sub-percent precision.

The present scan extends this in three orthogonal ways, mirroring the FTD-0097 / FTD-0121 / FTD-0122 methodology:

1. **Variable coefficient `c`**: instead of fixing c = 16, sweep `c` over small framework-integer-factorable values. The c = 16 choice is privileged for d = −4 by `|Aut(E_{-4})|² = 16`; other d values have |Aut|² ∈ {4, 36}, so c = 16 is *not* their natural coefficient. A complete rigidity test must allow each d's natural |Aut|² and also test variation around it.

2. **Broader target set**: instead of checking only against (1/α, N_c), test against the same 14-target set used in FTD-0122 plus three SM mass ratios (m_p/m_e, m_μ/m_e, m_τ/m_e) that the framework claims are integer-arithmetic-derived.

3. **Framework-integer factorability filter**: rational-multiplier search (q ≤ 200) with F-factorability and FC-factorability criteria, exactly as in FTD-0122.

The pre-registered question: **of the 9 Heegner discriminants × 18 coefficient values × 17 targets = 2754 (d, c, target) triples, how many yield strict (5.45 ppm) matches with framework-integer-factorable rational multipliers? Is d = −4 with c = 16 uniquely positioned, or are there other (d, c, target) triples with comparable matches?**

---

## 1 · Search space (LOCKED)

### 1.1 · Heegner discriminants

```
d ∈ {-3, -4, -7, -8, -11, -19, -43, -67, -163}
```

with corresponding Chowla-Selberg bridge constants ρ_d (verified to 30 dps via direct Γ-product computation; values match `EXPLR_CM_RATIO_TOWER.md §2`):

```
ρ_{-3}    =  1.97836425965...
ρ_{-4}    =  2.95867511919...   (= G*)
ρ_{-7}    = 11.01719287594...
ρ_{-8}    = 11.42500228878...
ρ_{-11}   = 12.17410354680...
ρ_{-19}   = 12.18257208845...
ρ_{-43}   =  8.71992297060...
ρ_{-67}   =  5.79345595325...
ρ_{-163}  =  1.27987907660...
```

### 1.2 · Coefficient set

```
C ∈ {2, 3, 4, 6, 8, 9, 12, 16, 18, 24, 27, 32, 36, 48, 64, 72, 81, 108, 144}
```

19 values, all framework-integer-factorable (composed of primes from F = {2, 3, 5, 7, 13}; small enough to be "natural" in the FTD sense). The set covers |Aut|² for the natural CM curves (4, 16, 36) and a range of nearby values for sensitivity testing.

### 1.3 · Master-quadratic construction

For each (d, c) pair:
```
M_{d,c}(x) = x² − c · ρ_d² · x + c · ρ_d³ = 0
```

Both roots `x_+, x_-` reported. (Discriminant = c²ρ⁴ − 4cρ³ = cρ³(cρ − 4); positive for cρ > 4. All 9 ρ_d > 1/4, so c ≥ 16 always positive; smaller c may give complex roots for small ρ_d.)

### 1.4 · Target set (LOCKED)

```
T = { G*, 2G*, 4G*, ϖ, 2ϖ, π, 2π, 4π, e, 2e, G*², 4G*², 8G*², 1/α,
      m_p/m_e, m_μ/m_e, m_τ/m_e }
```

17 targets total. The first 14 are the FTD-0122 set (general framework-relevant constants). The last 3 are SM mass ratios that the framework claims integer-arithmetic-derived (per `dimensional_map.json` notes).

### 1.5 · Rational multiplier search

For each (d, c, target) triple AND each root x_± of M_{d,c}, search for rational `p/q` with `q ≤ 200` such that `x_± · p/q` is within tolerance of the target. **The trivial multiplier p = q = 1 is allowed** (corresponds to "x_± directly hits target without rescaling"); p/q = 1/1 is the natural FTD-style match.

### 1.6 · Tolerances (LOCKED)

```
Strict:  5.45 ppm  (matches FTD-0122 tolerance for direct comparison)
Tight:   50 ppm
Loose:   500 ppm
```

### 1.7 · Factorability criterion (LOCKED — same as FTD-0122)

- **F-factorable**: both p and q have all prime factors in F = {2, 3, 5, 7, 13}
- **FC-factorable**: F-factorable, OR includes one factor from cyclotomic-extras `{1+n+n² : n ∈ {3, 4, 7, 13}} = {13, 21, 57, 183}`

---

## 2 · Hypothesis criteria (LOCKED)

**H_canonical (d=-4 / c=16 uniquely privileged):**
- Of all (d, c, target, root) quadruples in the scan that yield strict-tier FC-factorable matches, the (d=-4, c=16, target=1/α, root=x_+) match is **unique** in the strict tier.
- Equivalently: NO other (d, c, target, root) quadruple in the scan reaches 5.45 ppm precision with F- or FC-factorable rational multiplier.

**H_fit (d=-4 / c=16 is one of many):**
- Three or more (d, c, target, root) quadruples reach strict-tier FC-factorable precision, of which (d=-4, c=16, 1/α, x_+) is just one. The framework's claim of unique CM-curve privilege fails.

**H_mid (intermediate):**
- One or two strict matches in addition to (d=-4, c=16, 1/α, x_+). The privilege is real but narrower than the framework rhetorically claims.

The existing `EXPLR_CM_RATIO_TOWER.md` analysis at fixed c=16 implicitly tests a sub-case; this scan extends it to the full coefficient × target × discriminant grid, which has not been done before.

---

## 3 · Pre-registered actions per outcome

If **H_canonical**: SPEC_ALGEBRAIC_SPINE Theorem 3 retains its current [THEOREM] status; the scan provides quantitative confirmation of CM-uniqueness within the larger search grid; LEDGER row added confirming Heegner tower rigidity; manuscript chapters can lean on this scan-level result.

If **H_mid**: Theorem 3 retagged with "[THEOREM] within fixed c=16; supplementary scan reveals N additional matches at varying c values, see audit doc". Honest qualifier.

If **H_fit**: Theorem 3 retagged [SELECTION] (significant downgrade); the framework's CM-uniqueness claim is meaningfully weaker than presented; structural narrative requires substantial revision.

---

## 4 · Methodological discipline

- Scan code SHA-256 will be locked at this commit.
- Verdict reported in `AUDIT_HEEGNER_TOWER_RIGIDITY.md` regardless of outcome (no cherry-picking).
- All match enumerations included for transparency (no "we'll just report the headline" cherry-pick).
- If an unexpected interesting finding emerges (e.g., a different d with cleaner factorability than d=-4), it is reported separately and a follow-up scan is pre-registered for it rather than retroactively folded in.

**Author commit before scan run.** Pre-registration locks at this commit; tag `preregister-heegner-tower-rigidity-v1` to be applied per FTD discipline (FTD-0097 pattern).
