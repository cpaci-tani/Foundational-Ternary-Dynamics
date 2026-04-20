# Physicist Spot-Check — Reframe Rewrites (Session 4, 2026-04-19)

**Reviewer role:** Adversarial physicist; read-only audit.
**Scope:** the 12 files enumerated in the Session-4 brief plus the `CANONICAL_REFRAME.md`/`LEDGER.md`/`CHANGELOG_REFRAME.md` triad.
**Method:** verified the algebraic identities with `mpmath` (30-digit precision); reproduced the dimensional-chain arithmetic by hand; compared each rewrite against (i) the canonical reframe, (ii) the LEDGER tags, (iii) standard physics.

---

## Overall verdict

**PASS-WITH-NOTES.** Sessions 1–3 substantially improved epistemic hygiene. The major structural moves — algebraicising the master quadratic, demoting Type III₁ to a scaffold hypothesis, calibration-anchoring the EFT-program FLAGs, declaring `a_phys ≡ ℓ_P` — are physics-correct and well-executed. Three concrete physics issues (one numerical error, one scaling inconsistency, one unstated mass-scale conflation) need correction; none invalidate the reframe direction.

| Severity | Count | Files |
|---|---|---|
| **Numerical error (must fix)** | 1 | `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` |
| **Internal inconsistency** | 2 | `SPEC_FTD.md` §14.2, `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` (mass-scale conflation) |
| **Pass-with-notes (clarifying edits)** | 5 | `FOUND_AXIOM_ZERO.md`, `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`, `DERIV_PATH_INTEGRAL_CONSTRUCTION.md`, `DERIV_VON_NEUMANN_CONSTRUCTION.md`, `DERIV_COLLAPSE_MECHANISM.md` |
| **Clean pass** | 4 | `OPEN_A_PHYS_DERIVATION.md`, `DERIV_BETA_FUNCTION_MEASURED.md`, `DERIV_DAY2_CAMPAIGN.md`, `DERIV_DYNAMICAL_SM_EMERGENCE.md`, `CONJ_ALPHA_FROM_CM.md` |

---

## Per-file findings

### File 1: `02_foundations/FOUND_AXIOM_ZERO.md`

- **Verdict:** PASS-WITH-NOTES.
- **Algebra reproduced:** `G* = √2·Γ(1/4)²/(2π) = 2.95867511919…`; `G*²/(2π) = Γ(1/4)⁴/(4π³) = 1.39320392969…`; roots `x₊ = 137.03617146…` (1.258 ppm vs CODATA 137.035999084), `x₋ = 3.02396391634…` (0.799 % vs N_c = 3); harmonic mean `2x₊x₋/(x₊+x₋) = 5.91735… = 2G*` exactly. All numerics in §3.2 are correct to displayed precision.
- **Vieta + discriminant trichotomy:** correctly stated. The critical k for Δ = 0 is `k_crit = 4/G* = 1.35196`, so the regime ordering is `bosonic (k > k_crit) | critical (k = k_crit) | fermionic (k < k_crit)`. The doc's table presents this correctly.
- **Physics issues found:**
  1. §2.3(c) repeats the previously-flagged claim `g_c · s · (∇·J)` is "the simplest scalar quantity that can be formed from J at a point, respecting O_h symmetry" — this is true at first-derivative order but the doc itself elsewhere admits other O_h-invariant couplings exist. The [SELECTION] tag is correct; the prose in §2.3(c) could be tighter about why first-order is preferred.
  2. §3.1 ("Time = energy processing, energy/tick/DOF = G*²") uses the Vieta sum `x₊ + x₋ = 16G*²` to claim `G*²` per DOF per tick. Sixteen `G*²` is the *sum*; per DOF would require dividing by some integer count. The "G*² per DOF per tick" identification is not derived; the doc tags it [SELECTION] which is honest but the arithmetic linkage is loose.
- **Specific concerns:** None blocking. The reframe-driven rewrite is internally consistent.
- **Recommended fix:** in §3.1, either drop the "= G*²" specificity (replace with "the Vieta sum normalises to a coefficient in G*²") or supply the DOF-counting argument explicitly.

### File 2: `03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`

- **Verdict:** PASS. The most important rewrite of the session.
- **Algebraic content preserved:** Vieta relations (§1.3), harmonic-mean interpretation (G* as half the harmonic mean), discriminant trichotomy with correct condition `k > 4/G*` for bosonic regime (§3). The two finite-combinatorial routes to the coefficient 16 (`|Aut(E)|² = 16` and `z_BCC · 2 = 16`) are clearly separated and correctly tagged [THEOREM].
- **Numerics confirmed:** as in File 1. The 30-digit reproduction matches the doc's 5-digit displayed values.
- **Epistemic discipline:** the document is now a model for how to present an algebraic fact whose physical interpretation is conjectural. The "What this document does **not** do" abstract paragraph (lines 21) is exactly the right posture.
- **Minor note:** §1.3 says "the harmonic mean of the two roots is `2G*`, so `G* is half the harmonic mean`." Verified: `2x₊x₋/(x₊+x₋) = 2·16G*³/(16G*²) = 2G*`. Correct.
- **Recommended fix:** none needed. This file should be the template for other reframe rewrites.

### File 3: `03_derivations/DERIV_PATH_INTEGRAL_CONSTRUCTION.md`

- **Verdict:** PASS-WITH-NOTES.
- **§5.3 small-β / large-β behaviour (the explicitly-flagged section):**
  - Small-β: `Z(β) = 3^N · ∫DJ exp(-½J^TM₀J) · (1 + O(β))` — correct rate. Standard high-T expansion of `e^{-βH} = 1 - βH + O(β²)`; the leading correction in `Z` is indeed `O(β)`.
  - Large-β: `Z(β) = g₀ e^{-βE₀} (1 + O(e^{-β·Δ}))` — correct rate. Standard low-T expansion; gap suppression of excited contributions.
  - **Both rates are physics-correct** and properly framed as "behaviour at arbitrarily extreme finite β" rather than completed limits.
- **§5.5 finite-N free-energy stability:** `f_N = F_N/N` converges with `O(1/N^{1/D})` boundary correction — correct van Hove rate for short-range interactions in D dimensions. The "no exact non-analyticity at any finite N" statement is true (finite sum of analytics is analytic).
- **§4.1 long-wavelength expansion:** `2(3 - cos k_x - cos k_y - cos k_z) = k² - k⁴/12 + O(k⁶)` — verified. Standard lattice Laplacian expansion. The `O(k²)` error in the propagator approach to continuum is correct.
- **Physics issues found:**
  1. §1.4 (Theorem 1.3 IR finiteness): the "as L_μ is taken arbitrarily large, the FTD manifestation threshold K_B > 0 provides a natural mass gap" sentence is slightly hand-wavy. K_B is a manifestation threshold for the state field s, not a mass gap for the flux propagator. The mass gap for J is conventionally zero (massless gauge field). The IR-finiteness argument should probably say "for any finite L_μ, IR is bounded by the discrete momentum spacing 2π/L_μ; the K_B-induced mass gap applies separately to the s-sector."
  2. §6.5 still treats the Unruh/Hawking/KMS connection as [CONJECTURE], but the prose "approximately at scales >> lattice spacing" in step 3 of the Unruh argument has the same continuum-limit flavour as the ones edited elsewhere. It is tagged [CONJECTURE] so it is not load-bearing, but consistency would suggest restating.
- **Specific concerns:** none blocking.
- **Recommended fix:** clean up the K_B-as-mass-gap phrasing in §1.4; restate §6.5 step 3 to match the §14.2 SPEC_FTD treatment.

### File 4: `06_consciousness/DERIV_VON_NEUMANN_CONSTRUCTION.md`

- **Verdict:** PASS. The Type III₁ → HYPOTHESIS demotion is epistemically correct.
- **The key distinction is properly drawn:**
  - Sections 1–4: finite-region Type I results — these are [THEOREM] and survive untouched.
  - Section 5: the Araki–Woods inductive-limit construction — properly framed as "a separate scaffold that the framework neither builds nor disposes of."
  - The position-property axiom (undefined-boundary) does *not* commit to an inductive-limit construction; treating Type III₁ as a property of FTD-as-defined would have been over-reach. The demotion is justified.
- **Operator-algebra content correct:**
  - `Aut(C^{3^N}) = M_{3^N}(C)` is Type I_{3^N}: correct.
  - Powers/Araki–Woods: Type III_λ classification of ITP factors via eigenvalue ratios: correct.
  - For `ρ_β = diag(e^β, 1, e^{-β})/Z(β)` at β = π, the ratio λ₁/λ₃ = e^{2π} ≈ 535 with closure of `{e^{-2nβ}}` = R₊: correct (this is the classical Powers Type III_1 case for any β > 0 with non-trivial spectrum).
- **§7 Numerical verification table:** correctly tags the finite-region results [THEOREM] and the scaffold-projection rows [HYPOTHESIS]. The qualitative observation that "the number of distinct modular eigenvalue ratios grows with system size (5 → 13 → 43)" is presented as evidence-for-the-hypothesis, not as a derivation. This is correct epistemic discipline.
- **Minor concern:** the document still uses `Tr(I) = 3^N < ∞` to argue Type I_{3^N}. This is correct, but adjacent sentences refer to "semifinite trace," which technically allows infinite trace. For the finite-N case, `Tr` is just finite; "semifinite" is unnecessary jargon that confuses the reader. Cosmetic.
- **Recommended fix:** none blocking. This file is now a model for how to handle scaffold-vs-derivation distinctions.

### File 5: `06_consciousness/DERIV_COLLAPSE_MECHANISM.md`

- **Verdict:** PASS-WITH-NOTES. The orphan Type III₁ premises identified by the devil's-advocate audit have been restated.
- **Verification of the restatements:** at lines 26, 28, 31, 33, 322, 484-485, 506 — all relevant Type III₁ assertions now carry "[HYPOTHESIS — under Araki–Woods scaffold]" tags or are surrounded by "if the scaffold is appropriate" framing. Cross-citation to `DERIV_VON_NEUMANN_CONSTRUCTION.md` is consistent.
- **The structure of the argument remains intact:** Softplus → ReLU as a continuous decoherence-then-collapse mechanism. The mathematical content (Lindblad master equation, Born rule from Gaussian action, ReLU idempotence) is unchanged and correct.
- **Concern:** the abstract's "decoherence timescale `N_meas ≈ 18` lattice ticks" remains tagged [CONJECTURE]. With the Type III₁ demotion to HYPOTHESIS, this number is even more downstream than before — it is "conjecture under hypothesis under scaffold." The doc could acknowledge this layered conditionality more explicitly.
- **Specific concerns:** none blocking. Restatements were mechanically correct.
- **Recommended fix:** add one sentence to the abstract noting that all collapse-mechanism quantitative claims are conditional on the scaffold hypothesis.

### File 6: `10_eft_program/OPEN_A_PHYS_DERIVATION.md`

- **Verdict:** PASS. Clean exposition of the calibration question.
- The three mechanism analyses (α/β/γ) are honestly framed: α cannot deliver because no dimensional invariant exists in dimensionless lattice algebra; β reduces "derive" to "calibrate"; γ is the only candidate that could in principle force a unique value.
- **Physics-correctness note on Mechanism α:** the claim "no dimensional length emerges from the dimensionless lattice algebra" is physics-true. To get a length, one needs at minimum a dimensional anchor (Planck length, electron Compton wavelength, etc.). Pure combinatorics on N integer lattice sites cannot produce metres.
- The three falsifiability tests (table in §6) are well-posed: under β-matching the test is intra-observable consistency; under γ the test is whether the resulting `a_phys` makes other dimensional predictions agree.
- **Recommended fix:** none. This is a clean scoping document.

### File 7: `10_eft_program/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md`

- **Verdict:** **NEEDS-REVISION.** Numerical error in the alpha_G route, plus an unstated mass-scale conflation that strengthens the negative conclusion (so the document's bottom-line stands, but the number quoted is wrong).
- **Physics issues found:**
  1. **NUMERICAL ERROR (must fix).** The doc claims:
     > "Re-running with `G_N(lattice) = α_G ≈ 5.91 × 10⁻³⁹` gives `a_phys ≈ 6.7 × 10⁻⁷ m`, sub-micron length."

     Reproducing the formula `a_phys = 3·M_unit·G_phys / (G_lat·c²)` exactly:
     ```
     numerator   = 3 × 1.783e-30 × 6.674e-11 = 3.5699e-40
     denominator = 5.91e-39       × 8.988e16 = 5.3119e-22
     a_phys      = 3.5699e-40    / 5.3119e-22 = 6.72e-19 m
     ```
     The result is **6.7 × 10⁻¹⁹ m**, not 6.7 × 10⁻⁷ m. A factor of 10¹² has been dropped. The G_lat = 0.01 result of `~4 × 10⁻⁵⁵ m` is correct (I get 3.97e-55). Only the alpha_G result is wrong.
     - Note: 6.7 × 10⁻¹⁹ m ≈ 4 × 10⁻⁴ × ℓ_P, still implausible as a fundamental lattice spacing — so the **document's negative conclusion is unaffected**, but the quoted length scale is off by 12 orders of magnitude.
  2. **Mass-scale conflation (clarification).** The value `α_G = 5.91 × 10⁻³⁹` plugged into the formula is the gravitational fine-structure constant for **proton-proton interaction** (`G·m_p²/(ℏc) = 5.906e-39`; verified). However, the formula uses `M_unit = 1 MeV/c²`, which is set by the **electron-mass** anchor (K_B = m_e). One cannot self-consistently mix proton-scale α_G with electron-scale M_unit. The correct "α_G at electron scale" is `G·m_e²/(ℏc) = 1.75e-45`. Either the formula should use `M_unit = m_p ≈ 938 MeV/c²` (which would re-introduce a factor ~1000 in the numerator), or `α_G(electron) = 1.75e-45` (which would shift the denominator by 6 orders of magnitude). The doc currently mixes scales.
- **Specific concerns:** the document's headline conclusion ("Mechanism γ does not deliver a derived a_phys") is **strengthened** by both findings, not weakened. The arithmetic should be fixed but the reframe disposition (declare `a_phys ≡ ℓ_P`) is the right call.
- **Recommended fix:** correct the alpha_G arithmetic. Add a sentence noting that even with the corrected number, the result remains implausible. Consider a single-sentence note about the mass-scale inconsistency in the alpha_G substitution.

### File 8: `docs/SPEC_FTD.md` Postulate 1 + LATTICE↔PHYSICAL CALIBRATION

- **Verdict:** PASS-WITH-NOTES.
- **Postulate 1 rewrite (line 211–214):** "no defined boundary; at every specified position, the six axis-adjacent (and 26-Moore-adjacent) sites exist" is the correct undefined-boundary formulation.
- **Calibration section (line 221–238):** the declaration `a_phys ≡ ℓ_P`, `t_phys = √3·ℓ_P/c`, `M_unit = m_e/K_B = 1 MeV/c²` is internally consistent.
  - Verified: `c_lat = 1/√3` lattice/tick combined with `c_phys = 2.998e8 m/s` and `a_phys = ℓ_P = 1.616e-35 m` gives `t_phys = √3 · 1.616e-35 / 2.998e8 = 9.34e-44 s` — correct.
  - Verified: `m_e = 0.511 MeV/c² = 9.11e-31 kg` and K_B = 0.511 lattice gives `M_unit = 9.11e-31 / 0.511 = 1.78e-30 kg = 1 MeV/c²` — correct.
- **§14.2 Lorentz emergence rewrite:**
  - The new framing "at every finite spacing `a` and every wavelength `λ ≫ a`, rotational/boost invariance recovered to error `O((a/λ)²)`" is **finitarily honest** and avoids a totalised limit.
  - **Physics concern:** the *isotropic* leading correction in lattice dispersion is `O(k² · a²)` (true for the wave equation), but the *rotational anisotropy* (the actually-Lorentz-violating piece) on a cubic lattice with octahedral O_h symmetry starts at `O(k⁴ a⁴)` because the cubic invariants (`p_x⁴ + p_y⁴ + p_z⁴` vs `(p²)²`) only differ at fourth order. The earlier SPEC text quoted "Lorentz violation at ε ~ (E/E_Planck)⁴ ~ 10⁻⁸⁰" (line 1419), which is the rotational-anisotropy rate. The two rates differ by a factor of `(a/λ)²`. The §14.2 rewrite collapses both into one O((a/λ)²) statement, which is **internally inconsistent** with line 1419's `(E/E_Planck)⁴` claim.
  - The fluid/Galilean analogy is apt for the rotational-symmetry recovery, but Lorentz-boost recovery is harder than rotational recovery (an absolute-time lattice has *no* boost symmetry at any finite a, even per-mode; the analogy stretches).
- **Recommended fix:** distinguish the two error orders. Suggest: "rotational anisotropy in dispersion: `O((a/λ)⁴)` from O_h symmetry; isotropic dispersion correction: `O((a/λ)²)`; boost invariance: a stronger statement requiring the long-wavelength effective theory's emergent c to be Lorentz-covariant under the lattice's preferred frame, which is a separate selection argument." Reconcile with the existing line 1419 numerical rate.

### File 9: `10_eft_program/DERIV_BETA_FUNCTION_MEASURED.md`

- **Verdict:** PASS. Restatement A applied cleanly to Flag 1.
- The new wording at line 309–319 commits only to (i) sign agreement with QED at tested L, (ii) magnitude discrepancy 2–3 OOM. It explicitly does **not** claim asymptotic convergence; the scaling-exponent prediction is queued as future work.
- This is exactly what Restatement A (finitary scaling claim) should look like: a finite-L observation with a stated falsifiable extension.

### File 10: `10_eft_program/DERIV_DAY2_CAMPAIGN.md`

- **Verdict:** PASS. Restatement B applied at lines 312 and 399.
- Both passages now frame the 3.6× plateau as "predicted consequence of the framework's current `a_phys` choice" rather than "convergence to QED." The 4-point GPU data table (lines 333–340) is reported with all four scaling-law fits and explicit residual band [3.35, 3.74] × α_ref — honest empirical reporting.
- The retraction of the Day-2 "1.23×" claim (lines 319–329) is appropriately blunt.

### File 11: `10_eft_program/DERIV_DYNAMICAL_SM_EMERGENCE.md`

- **Verdict:** PASS. Restatement B applied to the headline (lines 17–27).
- The 4C entry now reads "the 1/L² fit gives α_largeL ≈ 3.6 · α_ref. **Whether this large-L value matches α_ref is a calibration question conditional on `a_phys`, not a convergence theorem.**" This is the correct framing under Interpretation D of the infinity-reframe audit.
- Branch B for EWSB (no condensate forms cold-start) and the null three-generation result are reported as such, not adjusted — good pre-registration discipline.

### File 12: `09_mathematical/CONJ_ALPHA_FROM_CM.md`

- **Verdict:** PASS. Path A retraction is clean.
- Lines 127–128 explicitly mark Path A as "RETRACTED — load-bearing premise was completed-infinity self-consistency" and note that Paths B/C/D survive unaffected. Cross-citation back to the rewritten DERIV_MASTER_QUADRATIC document is appropriate.
- The "Attack Vectors" table preserves the live mathematical content (Paths B–D) without dragging Path A's framing along.

---

## Cross-file inconsistencies

1. **Lorentz-violation rate (`SPEC_FTD.md` §14.2 vs line 1419).** §14.2 now says `O((a/λ)²)` recovery; line 1419 says `(E/E_Planck)⁴ ~ 10⁻⁸⁰` violation. These are not the same scaling. The first is the leading isotropic dispersion correction; the second is the leading rotational-anisotropy term. Both numbers can coexist if labelled correctly, but as written one of them is wrong. Recommend the SPEC harmonise these two rate statements with explicit labels for which symmetry-breaking effect each refers to.

2. **`a_phys ≡ ℓ_P` consequences flow vs DERIV_A_PHYS arithmetic.** The SPEC_FTD calibration section (line 221) declares `a_phys ≡ ℓ_P`. The DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT document (file 7) reports γ-mechanism trial values that miss ℓ_P by ~20 orders of magnitude (G_lat=0.01 case) or ~16 orders (corrected α_G case, after my fix). The two documents are mutually consistent in their conclusion (γ doesn't derive ℓ_P, hence declaration), but the arithmetic-error in file 7 means the documents currently disagree on the magnitude of the γ-route failure. After file 7 is corrected, the failures become 10²⁰ and 10¹⁶ — both implausible — and the conclusion strengthens.

3. **`K_B` semantics across documents.** In the path-integral document (§1.4) K_B is invoked as a "natural mass gap" that controls IR contributions. In the consciousness documents and the SPEC, K_B is a "manifestation threshold" on the state field s. These are different roles; the path-integral usage is loose (a manifestation threshold is not a propagator mass gap for J). Cosmetic but worth aligning.

---

## Honest assessment of the reframe's epistemic effect

**Sessions 1–3 strengthened the framework's intellectual credibility, full stop.** The previous portfolio carried a mix of legitimate algebraic results, parametric insertions presented as derivations, and load-bearing completed-infinity arguments that — once examined — could not survive scrutiny. The reframe does what every honest research program eventually has to do: separate the algebra from the physical interpretation, name the calibrations, and demote the claims that were doing more work than their proofs supported. The portfolio now has a smaller but genuinely defensible core. The master quadratic, presented as an algebraic identity with empirically-motivated identification, is much more honest than the previous "fixed point of an L → ∞ gap equation" framing — and importantly, the dual-match plus CM-curve uniqueness evidence still places the conjecture in the "interesting and worth investigating" range rather than the "bare numerical coincidence" range. The Type III₁ demotion to HYPOTHESIS is exactly the kind of move a careful operator algebraist would have flagged, and it concedes the right thing without throwing out the structural insight.

**Where the reframe weakens the framework, it does so by making explicit what was already true.** The 3.6× α plateau in the EFT program was always either a falsifying disagreement with QED or a calibration miscount; the reframe just makes the user pick. The α_largeL retraction (from the previous "α_∞" framing) was forced by the equilibration audit, not by the reframe per se, but the reframe provided the language to state the new claim cleanly. The Yang-Mills and Navier-Stokes retractions are losses to the portfolio's ambition but gains to its credibility — neither paper had a reframe-survivable load-bearing argument. The per-voxel mass gap that survives in the YM .tex (FTD-0044) is real and could anchor a smaller honest paper.

**The remaining vulnerabilities are clearly localised.** The biggest open structural question is `a_phys` derivation, now explicitly named. The `g_c` first-principles derivation remains [OPEN]. The connection between the master-quadratic algebra and FTD's actual local update rules — the dynamical-derivation question — remains the deepest gap. None of these is hidden anymore. A reader can now find them all by reading the LEDGER. The framework has moved from "claims a lot, supports some" to "claims what it can support, names what it cannot." That is the correct trajectory.

---

## Recommendations to owner

**Priority 1 (must fix this week):**

1. **Correct the alpha_G arithmetic in `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` §2 reading 1.** The quoted `~6.7 × 10⁻⁷ m` should be `~6.7 × 10⁻¹⁹ m`. Twelve orders of magnitude. Both the formula and the negative conclusion are unaffected; only the displayed length is wrong.

2. **Reconcile the two Lorentz-violation rates in `SPEC_FTD.md`** (§14.2 `O((a/λ)²)` vs line 1419 `(E/E_Planck)⁴ ~ 10⁻⁸⁰`). Either label them as different symmetry-breaking orders (rotational anisotropy vs isotropic dispersion correction), or pick one and remove the other.

**Priority 2 (clarifying edits, this month):**

3. In `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md`, add a one-sentence note that the alpha_G value `5.91e-39` is the gravitational fine-structure constant at proton scale, while M_unit is set by the electron mass — the two ingredients are at different scales, which is a separate inconsistency in the substitution. The conclusion ("Mechanism γ doesn't deliver derivation") strengthens further when the scales are properly aligned.

4. In `DERIV_PATH_INTEGRAL_CONSTRUCTION.md` §1.4, distinguish the K_B threshold for the s-field from the (zero) mass gap of the J-propagator. The IR finiteness argument should rest on the discrete momentum spacing 2π/L_μ, not on K_B as a generic mass gap.

5. In `DERIV_COLLAPSE_MECHANISM.md` abstract, add a sentence noting that quantitative claims (e.g., the `N_meas ≈ 18` decoherence-cluster size) are conditional on the Araki–Woods scaffold hypothesis, since that hypothesis was demoted in `DERIV_VON_NEUMANN_CONSTRUCTION.md`.

**Priority 3 (structural, this quarter):**

6. **Revisit Mechanism γ with cleaner dimensional bookkeeping.** The current attempt mixes engine-toy `G_N(lat) = 0.01` with Newton's G and electron-scale M_unit. A cleaner attempt would either (i) declare which scale is being matched, (ii) work at a single specified scale throughout, and (iii) report the calibration ratio rather than calling the result a derived a_phys. The conclusion is unchanged but the analysis would be more credible.

7. **The g_c open problem (`OPEN_GC_FROM_FIRST_PRINCIPLES.md`) is now the senior open structural item.** With Mechanism γ closed and a_phys declared, the next-most-load-bearing first-principles question is: where does the Coulomb coupling come from in the partition function? The Phase J L=2 result (FTD-0005) — that the action is ultralocal at L=2 and provides no extremum for g_c — is a serious negative finding that should be elevated in any further EFT-program planning.

8. **Consider a "post-reframe headline result" document** that states, in one place, what the reframed framework now claims and what it now does not. The LEDGER captures this as a tag list; a prose statement would help anyone (reviewer, collaborator, future you) get oriented quickly.

---

## Summary by severity

- **Numerical errors found that must be fixed:** 1 (file 7, alpha_G arithmetic).
- **Inconsistencies that should be reconciled:** 2 (file 8 Lorentz rate; file 7 mass-scale conflation).
- **Pass-with-notes (clarifying edits desirable, not blocking):** 5 (files 1, 3, 4, 5, 8).
- **Clean pass:** 4 (files 2, 6, 9, 10, 11, 12 — five files).

Net assessment: the reframe accomplished what it set out to accomplish. The framework is in better epistemic shape than it was before Session 1. The remaining issues are correctable and do not threaten the structural claims that survived the triage.
