# AUDIT — Look-Elsewhere Scan Results (FTD-0097)

**Tag:** [MEASURED] — pre-registered scan executed; verdict per PROTOCOL §7
**Date:** 2026-04-27
**LEDGER row:** FTD-0097
**Pre-registration:** [`PROTOCOL_LOOK_ELSEWHERE_SCAN.md`](PROTOCOL_LOOK_ELSEWHERE_SCAN.md)
**Pre-reg tag:** `preregister-look-elsewhere-scan-v1` (commit `f11dcaa`, 2026-04-27)
**Runner:** [`tools/scan_look_elsewhere.py`](../../../tools/scan_look_elsewhere.py)
**Runner SHA256:** `6d9f0f5aebe924023b09003cd13448eb87fc7d036e7bac48cb8e442bb82d628f`
**Scan output:** `engine/results/look_elsewhere_2026-04-27/`

---

## 0 · Pre-registration verification

Per PROTOCOL §2 the scan executes ONLY after the runner is hash-locked and tagged. Verified:

- ✅ Runner committed at git rev `ebc5178` (FTD-0097 tooling commit)
- ✅ SHA256 = `6d9f0f5aebe924023b09003cd13448eb87fc7d036e7bac48cb8e442bb82d628f` inscribed in PROTOCOL §2 line 128 (commit `f11dcaa`)
- ✅ Tag `preregister-look-elsewhere-scan-v1` applied at the SHA256-lock commit
- ✅ Tag pushed to origin
- ✅ Scan executed AFTER the tag (no post-tag runner edits; verifiable via `git log tools/scan_look_elsewhere.py`)
- ✅ All hits at ε ≤ 10⁻³ enumerated in `hits_eps_1e-3.csv` (421 entries; cherry-picking closure per PROTOCOL §6(b) achieved)

Author isolation handled via §6(b) (deterministic runner + complete enumeration).

---

## 1 · Summary verdict

**Per PROTOCOL §7 verdict matrix: NULL REJECTED upward (catalog over-rich).**

The FTD constant catalog produces dramatically more 10⁻⁴-level matches against the 20 fixed physics targets than chance would predict, and the matches cluster non-uniformly on FTD-derived targets (m_p/m_e, m_e in MeV) rather than uniformly across the target set.

**Both pre-registered rejection criteria fire**:

| Criterion | Threshold | Observed | Verdict |
|---|---|---|---|
| Total hits at ε=10⁻⁴ (raw) | ≥11 → over-rich | **62** | NULL REJECTED upward (>5× over) |
| Total hits at ε=10⁻⁴ (value-deduped) | ≥11 → over-rich | **11** | NULL REJECTED upward (at threshold) |
| Per-target uniformity χ² (df=19, raw) | ≥43.82 → 99.9% reject | **470.26** | rejected at >>99.9% |
| Per-target uniformity χ² (df=19, dedup) | ≥36.19 → 99% reject | **38.07** | rejected at 99% |

Both raw and value-dedup accountings give the same verdict. The catalog is over-rich and clustered.

**FTD-0094 (L2 candidate identity 2·m_e/α = 16G\*²) status post-scan**: terminally [PARAMETRIC] (already there per FTD-0093 closure 2026-04-27; this scan **confirms** the demotion from the methodological side). Per PROTOCOL §7: "NULL REJECTED upward → L2 demoted to [PARAMETRIC]; closes the chain definitively from the look-elsewhere side."

---

## 2 · Total hit counts at each tolerance

| Tolerance ε | Total hits (20 targets) | Null Poisson E[total] | Per-target avg | Verdict |
|---|---|---|---|---|
| 10⁻³ | 421 | 40 | 21.05 | over-rich |
| **10⁻⁴ (headline)** | **62** | **4** | 3.10 | **over-rich** |
| 10⁻⁵ | 0 | 0.4 | 0.0 | catalog DOES NOT match at 10ppm precision |
| 10⁻⁶ | 0 | 0.04 | 0.0 | catalog DOES NOT match at 1ppm precision |

**Critical structural observation**: while the catalog is over-rich at ε ≤ 10⁻⁴, it produces **zero hits** at ε = 10⁻⁵ and 10⁻⁶ across all 20 targets. The polynomial space of degree ≤ 4 monomials with coefficients ±{1,2,3} cannot reach ppm precision against any of the 20 targets at tolerance 10⁻⁵ or below.

This matters because **the master quadratic dual match (x_+ ≈ 1/α at 1.26 ppm = 1.26×10⁻⁶)** lives below the scan's resolution. The scan does NOT directly invalidate the master quadratic root identification — it shows that the **monomial space** can't reach that precision, but x_+ comes from a quadratic ROOT which is structurally different from any monomial in the catalog. The verdict here is on the catalog's monomial richness, not on root-of-polynomial richness.

What the scan DOES invalidate at the 10⁻⁴ level: any [CONJECTURE] claim of the form "FTD-derived formula F = monomial in the catalog ≈ measured ratio at ppm" — those formulas are exactly the kind of thing the catalog produces by chance.

---

## 3 · Per-target distribution + chi-squared test

### Raw counts at ε = 10⁻⁴

| Target | Raw count | Status | Diagnostic? |
|---|---|---|---|
| m_p_over_m_e | 38 | [DERIVED in FTD per LEDGER] | — |
| m_e_in_MeV | 13 | [DERIVED via m_e formula] | — |
| sin2_theta_W | 4 | [PARAMETRIC] (3/13 ≈ 0.231; 3.5% off) | DIAGNOSTIC |
| Vud_squared | 4 | not in FTD claim base | — |
| m_n_over_m_e | 2 | not in FTD claim base | — |
| h_Hubble | 1 | not in FTD claim base | — |
| **alpha_inv** | **0** | [STRONGLY MOTIVATED CONJECTURE] x_+ = 137.036 at 1.26 ppm | **DIAGNOSTIC** |
| m_mu_over_m_e | 0 | composite α·m_μ/m_e (DIAGNOSTIC composite) | — |
| **m_tau_over_m_e** | **0** | [DERIVED in FTD per LEDGER] | **DIAGNOSTIC** |
| m_p_over_m_n | 0 | not in claim base | — |
| g_e_minus_2 | 0 | derived from α | — |
| a_mu | 0 | not in claim base | — |
| alpha_s_MZ | 0 | [PARAMETRIC] | — |
| m_W_over_m_Z | 0 | not in claim base | — |
| m_b_over_m_c | 0 | not in claim base | — |
| m_t_over_v_higgs | 0 | not in claim base | — |
| Omega_b | 0 | cosmological | — |
| Omega_dm | 0 | cosmological | — |
| Theta_13 | 0 | [PARAMETRIC] | — |
| delta_CP | 0 | not in claim base | — |
| **Total** | **62** | (vs null E=4) | |

### Chi-squared on per-target uniformity

Under the null (FTD atom set has no structural privilege), hits should distribute uniformly across targets at rate λ/N = 0.2 per target.

**Raw counts:** χ²(df=19) = **470.26**, vastly exceeding the 99.9% rejection threshold of 43.82. Null uniformity rejected at >99.99%.

**Value-deduplicated counts** (treating multiple integer-factorizations of the same value as a single hit, e.g. 4·17·27 = 9·12·17 = 2·2·17·27 = 1836 → counted once):

| Target | Dedup count | Distinct value(s) at ε ≤ 10⁻⁴ |
|---|---|---|
| m_p_over_m_e | 4 | 1836.000, 1836.068, 1836.118, 1836.229 |
| m_e_in_MeV | 2 | 0.51102, 0.51103 |
| sin2_theta_W | 2 | 0.22289, 0.22292 |
| Vud_squared | 1 | 0.94894 |
| m_n_over_m_e | 1 | 1838.661 |
| h_Hubble | 1 | 0.67398 |
| **Total** | **11** | (across 6 targets) |

χ²(df=19, dedup) = **38.07**, exceeding 99% rejection threshold of 36.19. Null uniformity rejected at 99%.

The verdict is robust under either accounting. Even after deduplicating algebraically-identical polynomial expressions (e.g., 38 ways to write 4·17·27 = 1836 collapsed to 1 distinct value), the over-richness threshold is met at the boundary (11 ≥ 11).

---

## 4 · Diagnostic targets (PROTOCOL §3) — all 0 at headline ε

The three pre-declared diagnostic controls each anchor a different question:

| Diagnostic | Hits at 10⁻⁴ | Interpretation |
|---|---|---|
| **alpha_inv** (1/α = 137.036) | **0** | Catalog cannot reach 1/α at 10⁻⁴ precision. The FTD claim x_+ ≈ 1/α at 1.26 ppm comes from the **master quadratic root** (not a monomial). The scan does NOT directly support OR refute the structural claim — it shows the monomial space alone is insufficient. *(Historical: "The dual prediction (x_+ AND x_− simultaneously) lives outside the scan's polynomial space by construction." The dual-prediction framing depended on the now-retired `x_- ↔ N_c` identification per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`. The polynomial-template uniqueness fact about the master quadratic survives independently and is now carried by FTD-0189's adversarial scan.)* |
| **sin2_theta_W** (0.22290) | 4 raw / 2 dedup | The catalog DOES match sin²θ_W at 10⁻⁴ via various polynomial expressions. FTD's existing PARAMETRIC tag (3/13 ≈ 0.231 at 3.5% off) is not improved here — the new 4 hits all match at the proper experimental value (0.22289–0.22292), not the 3/13 approximation. Says: catalog absorbs sin²θ_W via different polynomial routes. |
| **m_tau_over_m_e** (3477.23) | **0** | FTD has a DERIVED formula for m_tau/m_e in the LEDGER. The catalog at degree ≤ 4 with the chosen atoms cannot reach 3477.23 at 10⁻⁴ — meaning **the FTD derivation of m_tau/m_e uses higher-order combinations than the scan's degree-4 cap, OR uses atoms not in the catalog**. This is informative methodologically: it suggests FTD's claim base is more selective than the scanned space at this specific target. |

**Reading**: 1 diagnostic clean (m_tau/m_e: scan does not absorb), 1 diagnostic intermediate (sin²θ_W: scan absorbs, but only via different paths than the existing 3/13 PARAMETRIC), 1 diagnostic informative (alpha_inv: scan can't reach; the master quadratic ROOT identification is structurally outside the scan's resolution).

The diagnostic-control story is thus mixed: the scan verifies that the catalog **is over-rich enough to absorb arbitrary integer-bound ratios** like m_p/m_e = 1836 (via 4 distinct polynomial values), and that FTD-derived formulas like m_e = 8·G\*²·α (the L2 identity FTD-0094) **are exactly the kind of monomials the catalog produces by chance**. But the alpha_inv = 137.036 case is structurally distinct (root, not monomial) and the scan can't reach it.

---

## 5 · Full enumeration of hits at ε ≤ 10⁻³ (PROTOCOL §6(b) cherry-picking closure)

421 total hits at ε ≤ 10⁻³ (raw, before dedup). Distributed across 13 of the 20 targets.

The full enumeration is in `engine/results/look_elsewhere_2026-04-27/hits_eps_1e-3.csv` (CSV, columns: target_name, target_value, polynomial_value, polynomial_string, residual, degree, coefficient).

**Top patterns** (sample of representative hits — full file is the load-bearing artifact):

- **m_p/m_e ≈ 1836** at residual 8.31×10⁻⁵: 4·17·27, 9·12·17, 2·2·17·27, 3·3·12·17, etc. (these are integer factorizations of 1836 via the catalog's integer atoms; many redundant routes per protocol's §1.2 dedup framework)
- **m_p/m_e ≈ 1834.27** at residual 1.88×10⁻⁵: 6π⁵ (via 3 · π² · π² · 2π); the only transcendental path that reaches m_p/m_e at this precision
- **m_e_in_MeV ≈ 0.51102** at residual 6.88×10⁻⁵: 8·G\*²·α (and equivalent factorizations 2·4·G\*²·α, 1·8·G\*²·α, 2·2·2·G\*²·α). **This is exactly the FTD-0094 L2 candidate identity** 2·m_e/α = 16G\*² rearranged. Confirms that FTD-0094 IS the kind of polynomial expression the catalog produces.
- **sin²θ_W ≈ 0.22289** at residual ~9×10⁻⁵: various combinations including 1/G\* · 1/G\*² · 1/√3 with integer coefficients
- **Vud² ≈ 0.94894** at residual ~5×10⁻⁵: combinations involving 1/G\*² · π/e

The runner enumerates EVERY hit at ε ≤ 10⁻³, including those at ε > 10⁻⁴. No filtering by polynomial "interestingness" or epistemic privilege.

---

## 6 · Verdict on FTD-0094 (L2 candidate identity)

Per PROTOCOL §7 verdict matrix, NULL REJECTED upward implies **L2 demoted to [PARAMETRIC]**.

FTD-0094 was already terminally [PARAMETRIC] as of 2026-04-27 (per FTD-0093 closure: all three first-principles routes for g_c — Mechanisms A, B, C — closed negative; FTD-0096 μ-arrow remains OPEN). This scan **confirms** the demotion from the look-elsewhere side: the L2 identity 2·m_e/α = 16G\*² is **directly visible in the scan as one of the 13 raw hits on m_e_in_MeV** at residual 6.88×10⁻⁵. That residual is exactly the L2 identity's reported precision (68.77 ppm = 6.88×10⁻⁵). The scan reproduces FTD-0094 as one of the catalog's chance-level monomial fits.

**Consequence**: any future attempt to promote a [CONJECTURE]-tagged identity past [SELECTION] purely on ppm-strength of monomial fit is now ruled out methodologically. Promotion requires either:
- An **independent structural derivation** (the route attempted by FTD-0093 Mechanism C, closed negative)
- A **dual-match property** that holds at much tighter precision than 10⁻⁴ (the master quadratic's 1.26 ppm dual match for x_+ AND x_− simultaneously — which the scan cannot evaluate because its polynomial space doesn't include polynomial roots, only monomials)

The master quadratic dual match (x_+ + x_− identifications) remains [STRONGLY MOTIVATED CONJECTURE] — the scan does NOT directly weaken or strengthen it. It only tells us that monomial-level FTD claims are not load-bearing.

---

## 7 · Cross-references and what this scan does NOT do

Per PROTOCOL §9:

- **Does NOT** scan continuously for new candidate identities. One-time validation only.
- **Does NOT** close the methodological question. NULL REJECTED upward is strong methodological evidence that the catalog is over-rich, but per CLAUDE.md Constraint 11 even this is one piece of evidence; structural/derivation routes are independent epistemic moves.
- **Does NOT** replace the structural tests (D1, D4 of FTD-0093). Those stand independently.
- **Does NOT** evaluate polynomial-root identifications (master quadratic, characteristic-polynomial-style identities). The scan covers only monomial polynomial expressions.

Cross-references:

- `docs/theory/07_assessment/PROTOCOL_LOOK_ELSEWHERE_SCAN.md` — the locked spec
- `docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md` — the ~129 PARAMETRIC entries this scan implicitly tested against
- `docs/theory/07_assessment/LEDGER.md` FTD-0093 (Mechanism C closed negative), FTD-0094 (L2 [PARAMETRIC]), FTD-0096 (μ-arrow [OPEN])
- `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` — the seven-theorem algebraic spine remains UNCHANGED by this scan
- `docs/theory/09_mathematical/EXPLR_CM_RATIO_TOWER.md` — 9-Heegner Chowla-Selberg tower, foundational-math reference
- `CLAUDE.md` Constraint 11 — methodological commitment this scan operationalizes

---

## 8 · Single-line summary

**Pre-registered FTD-0097 look-elsewhere scan executed 2026-04-27 with hash-locked deterministic runner at SHA256 `6d9f0f5a…`, tag `preregister-look-elsewhere-scan-v1`, output in `engine/results/look_elsewhere_2026-04-27/`. Scanned 671,574 monomials of degree d ∈ {1,2,3,4} with coefficients c ∈ {−3,−2,−1,1,2,3} from the 38-atom FTD catalog against 20 dimensionless physics targets at tolerances ε ∈ {10⁻³, 10⁻⁴, 10⁻⁵, 10⁻⁶}. **HEADLINE RESULT: 62 raw hits / 11 value-deduplicated hits at ε = 10⁻⁴ vs Poisson null λ = 4 → NULL REJECTED upward (catalog over-rich); χ²(df=19) = 470.26 raw / 38.07 dedup → uniformity rejected at 99.9%+ / 99%; verdict robust under both accountings.** Hits cluster strongly on FTD-derived targets (m_p/m_e: 38 raw / 4 dedup; m_e in MeV: 13 raw / 2 dedup including the L2 identity 8·G\*²·α at 6.88×10⁻⁵ residual). Diagnostic targets: alpha_inv = 0 hits (master quadratic root is structurally outside monomial-only scan), m_tau/m_e = 0 (FTD derivation uses higher-order than scanned), sin²θ_W = 4 raw / 2 dedup. Zero hits at ε ≤ 10⁻⁵ across all 20 targets → polynomial space cannot reach ppm precision; the master quadratic's 1.26 ppm dual match (x_+ AND x_− simultaneously) lives below scan resolution and is not directly tested. **FTD-0094 (L2 candidate identity 2·m_e/α = 16G\*²) terminally [PARAMETRIC] confirmed from the methodological side; FTD-0094's exact residual (68.77 ppm) reproduced as one of the catalog's chance-level monomial fits.** Algebraic spine [THEOREM]s (master quadratic, G\* identity, CM uniqueness, Phase G geometric Coulomb, Phase J ultralocality, Watson identity, coefficient 16) UNCHANGED — they live at the polynomial-root / number-theoretic layer, not in the monomial scan space.**
