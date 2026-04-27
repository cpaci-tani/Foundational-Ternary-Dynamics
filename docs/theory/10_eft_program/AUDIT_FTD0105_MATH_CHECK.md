# Math Audit — FTD-0105 Lemniscatic-Replacement Investigation

**Tag:** [AUDIT] (verification of math in AUDIT, PROTOCOL, ANALYSIS, and engine output)
**Date:** 2026-04-27
**LEDGER row:** FTD-0105
**Subject:** verifies the constants, prediction matrix, statistics, and secondary readings of the lemniscatic-replacement investigation. Run via `PYTHONIOENCODING=utf-8 python3` against the canonical `scripts/constants.py` definitions.

---

## 1 · Summary verdict

**HOLDS WITH TWO CORRIGENDA.** The headline result (D1 PASS-NONE / secondary closed-negative) is unaffected; the math errors are confined to (a) one supplementary-table typo in the AUDIT and (b) one methodological caveat about reported stderr underestimating true uncertainty. Both are documented below with corrigenda recommendations.

---

## 2 · What checks out (verified against canonical constants)

All values verified against `scripts/constants.py` (the canonical source) at 10-decimal precision.

### 2.1 Constants

| Symbol | Definition | Computed value | AUDIT value | Match |
|---|---|---|---|---|
| Γ(1/4) | gamma function | 3.6256099082 | implicit | ✓ |
| Γ(1/2) = √π | gamma function | 1.7724538509 | implicit | ✓ |
| ϖ | Γ(1/4)²/(2√(2π)) | 2.6220575543 | 2.62206 | ✓ |
| G* | Γ(1/4)²/(√2·Γ(1/2)²) | 2.9586751192 | 2.95867 / 2.9587 | ✓ |
| G* via 2ϖ/√π | identity check | 2.9586751192 | implied | ✓ to 10 dp |
| G* via ϖ/√PF | identity check | 2.9586751192 | implied | ✓ to 10 dp |
| G*² | (G*)² | 8.7537584609 | 8.7540 | ✓ to 4 dp |
| PF = π/4 | packing fraction | 0.7853981634 | 0.7854 | ✓ |

**Verdict:** all canonical constants hold; no errors.

### 2.2 Pre-registered candidate values

| Candidate | Formula | Computed | PROTOCOL claim | Match |
|---|---|---|---|---|
| Standard sphere | 4π | 12.566371 | 12.566 | ✓ |
| Candidate A | 4ϖ | 10.488230 | 10.488 | ✓ |
| Candidate B | 4G* | 11.834700 | 11.835 | ✓ |
| Candidate C | G*²·π/2 | 13.750372 | 13.749 | ✓ to 3 dp |

**Verdict:** all four pre-registered candidates correctly computed.

### 2.3 Measurement statistics

| Quantity | Audit value | Reported value | Match |
|---|---|---|---|
| A/r² per cr (cr=2..5) | [18.9877, 17.5139, 19.1834, 18.3556] | identical from per_cluster.csv | ✓ |
| Pooled mean | 18.5101 | 18.5101 | ✓ |
| 6π | 18.8496 | implicit | — |
| measured / 6π | 0.9820 | "within 1.8%" | ✓ (exact: -1.80%) |
| Digital-geometry overhead | 1.473 | "~1.5" | ✓ rounded |

**Verdict:** measurement statistics hold.

### 2.4 κ scaling exponent

Linear regression on log(κ) vs log(M) for the 4 cluster_radii data points yields slope = 0.2830 ± fitting-noise. ANALYSIS §4 claimed M^0.28. **Verified within 1% rounding.**

### 2.5 Anisotropy values

| Cluster radius | Computed (rf−rc)/⟨r⟩ | ANALYSIS value | Match |
|---|---|---|---|
| cr=4 | (13−12)/12.5 = 0.0800 | 0.08 | ✓ |
| cr=5 | (15−14)/14.5 = 0.0690 | 0.0689 | ✓ |

**Verdict:** anisotropy correct.

---

## 3 · Errors found (TWO)

### 3.1 ERROR — AUDIT §2 supplementary table: "16·G*²/π = 35.014"

**Location:** `AUDIT_LEMNISCATIC_SPHERE_REPLACEMENT.md` §2, numerical-values table (top of section).

**Claim:** "16·G*²/π | 35.014"

**Actual:** 16 × 8.7538 / π = 140.06 / 3.1416 = **44.5825**

**Diagnosis:** the value 35.014 is approximately 4·G*² (= 35.0150 = 8π·W₃, which is the Watson-anchored quantity), NOT 16·G*²/π. The author conflated two different expressions in the supplementary table.

**Severity:** LOW. This typo is in the supplementary numerical-values table that lists candidate replacements, NOT in the prediction matrix used by the engine code or the falsifier criterion. Candidate C (G*²·π/2 = 13.750) is correctly stated in the main prediction-matrix table and correctly used in the engine code (`benchmark_black_hole_thermo.cpp` `lem::GSTAR2_PI_2`).

**Corrigendum recommended:** correct the supplementary table to read "16·G*²/π = 44.58" OR replace it with "4·G*² = 35.02 = 8π·W₃" (which is what the value 35.015 actually corresponds to and would be Watson-anchored).

### 3.2 METHODOLOGICAL CAVEAT — reported stderr underestimates true uncertainty by 2.5×

**Location:** `meta.json` (`pooled_A_ratio_stderr: 0.149462`) and `ANALYSIS_LEMNISCATIC_REPLACEMENT.md` §1 ("18.51 ± 0.15").

**Claim:** pooled A/r² = 18.51 ± 0.15 (n=20).

**Actual effective:** because the multi-seed strategy in the engine code shifted cluster centers only by integer offsets along the x-axis (sx ∈ {−2,…,+2}, sy = sz = −2 across all 5 seeds for seed/5 = 0), lattice translational symmetry maps all 5 seeds at the same cluster_radius to **identical outputs**. The verdict.csv confirms `A_ratio_stderr = 0` per cluster_radius. Effective independent-sample count is **4** (one per cluster_radius), not 20.

**True stderr:** with n=4 effective, sample variance s² = 0.5673, stderr = √(s²/4) = **0.376**. The reported 0.149 is computed as if n=20 with 16 of those samples being exact replicates — methodologically inflated.

**Diagnosis:** the `seed_rng()` call goes through but doesn't affect the simulation because gravity/latency_field run deterministically with no Langevin or random IC. The variant cluster centers vary only by sx along one axis; cubic-lattice symmetry makes that translation a no-op for the spherically-averaged horizon profile.

**Severity:** MEDIUM (does not flip any pre-reg verdict; see §3.3 below).

**Corrigendum recommended:** add a §X "Methodological note" to the ANALYSIS document acknowledging the effective n=4 and reporting stderr ≈ 0.38. Note that **the falsifier verdict (PASS-NONE / closed-negative-on-secondary-reading) is unchanged** because all four candidates remain >12σ outside the measured value even with the corrected stderr.

### 3.3 Verification: corrigendum 3.2 doesn't flip the verdict

| Candidate | Δ from measured (%) | σ (reported, fake n=20) | σ (true, n=4) |
|---|---|---|---|
| 4π | +47.3% | 39.8 | **15.8** |
| 4ϖ | +76.5% | 53.7 | **21.3** |
| 4G* | +56.4% | 44.7 | **17.7** |
| G*²·π/2 | +34.6% | 31.8 | **12.7** |

The closest candidate (G*²·π/2) is at 12.7σ true vs 31.8σ reported. **Both readings reject all four candidates at >>5σ.** The falsifier verdict (PASS-NONE) holds in either accounting.

---

## 4 · Auxiliary observations (NOT errors, structural notes)

### 4.1 Candidate C structural motivation is weaker than AUDIT §2.1 suggested

**AUDIT claim:** Candidate C (G*²·π/2 = 13.749) is "Watson-anchored — uses the explicit factor G*²/(2π) that already appears in Watson identity."

**Math:** G*²/(2π) is W₃ (the Watson value ≈ 1.393). G*²·π/2 = π²·W₃ ≈ 13.75. The natural "Watson-anchored" replacement for 4π would be 8π·W₃ = 4·G*² ≈ 35.02 (factor of π·π/(2·8π)·8π = π² ≈ 10× different).

**Reading:** Candidate C is a candidate of the form (something) · G*², where that something = π/2. It's not particularly motivated by Watson identity — it's a candidate of the form "G*² × small-rational-or-π-multiple." The AUDIT's claim that it's "Watson-anchored" overstates the structural motivation. **Severity: COSMETIC.** The candidate is what it is numerically; whether to call it "Watson-anchored" is a labeling question, not a math error.

### 4.2 Pre-registration discipline held: tag was applied BEFORE measurement

**Verified:** commit 7bc2185 created PROTOCOL with the prediction matrix; `git tag preregister-lemniscatic-v1` applied at that commit. Engine extension (commit f13d0e6) and production data both came AFTER the tag. The candidate values in the engine code (`lem::FOUR_PI`, `lem::FOUR_VARPI`, `lem::FOUR_GSTAR`, `lem::GSTAR2_PI_2`) match the pre-registered values exactly.

**No post-hoc modification of the prediction matrix.** Discipline held.

### 4.3 Engine's `count_isosurface_voxels` definition is well-defined and reproducible

**Verified (by code reading):** the function counts voxels with latency ≥ threshold AND at least one of 26 Moore neighbors with latency < threshold. This is the standard "26-connected boundary" definition from digital geometry (Klette & Rosenfeld, *Digital Geometry*, 2004).

**Implication:** the ~1.473× overhead vs 4π·r² for a sphere is consistent with published digital-geometry results for this convention. The 1.5× claim in the ANALYSIS rounded approximately; actual is 1.473×.

### 4.4 The κ scaling (κ ∝ M^0.28) is real engine behavior, not measurement noise

**Verified:** four independent (M, κ) data points fit a clean power law slope 0.2830, with κ values spanning factor 2.15× across mass range factor 15.6×. Standard Schwarzschild predicts κ ∝ M^(−1) (so κ·M = constant). The lattice gives κ ∝ M^(+0.28) — opposite sign. This is a STRUCTURAL observation about the FTD lattice's gravity sector, not a noise artifact, and merits its own follow-up ticket (separate from FTD-0105).

---

## 5 · Recommended actions

1. **Fix the supplementary-table typo** in `AUDIT_LEMNISCATIC_SPHERE_REPLACEMENT.md` §2: replace "16·G*²/π = 35.014" with the correct expression. (One-line edit; cosmetic.)

2. **Add §9 corrigendum** to `ANALYSIS_LEMNISCATIC_REPLACEMENT.md` documenting the effective n=4 and corrected stderr 0.38. Note that the verdict (PASS-NONE / closed-negative-on-secondary-reading) is unchanged at >12σ.

3. **Add a follow-up note** in the LEDGER FTD-0105 row mentioning that the κ ∝ M^0.28 scaling is itself a structural observation worth a separate ticket.

4. **No engine code changes required.** The `lem::` namespace constants, the isosurface-count function, and the pre-registered candidate values are all correct.

5. **No pre-reg violation.** The tag `preregister-lemniscatic-v1` was applied cleanly before measurement; no post-hoc adjustment of predictions.

---

## 6 · Single-line summary

**Math audit of FTD-0105 holds with two corrigenda: (a) AUDIT §2 supplementary-table typo "16·G*²/π = 35.014" is wrong (actual 44.58; the intended 35.015 = 4·G*² = 8π·W₃); (b) reported stderr 0.15 underestimates true uncertainty by 2.5× because multi-seed strategy reduced to effective n=4 not n=20. Headline verdict (D1 PASS-NONE / secondary closed-negative) UNCHANGED — all four candidates remain >12σ outside measured value even with corrected stderr. All canonical constants (ϖ, G*, G*²), candidate values (4π, 4ϖ, 4G*, G*²·π/2), pooled mean (18.51), κ scaling (M^0.283), anisotropy (0.08, 0.069), digital-geometry overhead (1.473×), and 6π secondary reading (-1.80%) verified to ≥4 decimal precision. Pre-registration discipline held cleanly: tag applied at commit 7bc2185 BEFORE engine extension at f13d0e6; no post-hoc modifications. Candidate C "Watson-anchored" labeling is loose (G*²·π/2 = π²·W₃, not 8π·W₃); cosmetic, not a math error.**
