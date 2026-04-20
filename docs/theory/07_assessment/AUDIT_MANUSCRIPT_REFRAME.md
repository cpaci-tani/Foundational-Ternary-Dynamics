# Manuscript Reframe Audit (2026-04-19, Session 4)

**Audit scope:** 175 chapter files across four manuscript locations:
- `dissemination/manuscript_v2/src/chapters/` (83 files — single source of truth, per `PROPAGATION_RULE.md`)
- `dissemination/manuscript/src/chapters/` (92 files — v1)
- `dissemination/manuscript_v2/vol1/src/chapters/` (35 files — publication snapshot)
- `dissemination/manuscript_v2/vol2/src/chapters/` (45 files — publication snapshot)

**Method:** trigger-phrase grep over all locations; targeted reads on chapters that grep flagged. No silent rewrites; this is a reading-only audit per task scope.

**Canonical sources consulted:**
- `docs/theory/07_assessment/reframe_deployment/CANONICAL_REFRAME.md` v1.0
- `docs/theory/07_assessment/LEDGER.md` v1.0 (49 rows, including 6 RETRACTED + 1 CALIBRATION)
- `dissemination/manuscript_v2/PROPAGATION_RULE.md`

---

## Summary

| Metric | Count |
|---|---|
| Chapters audited (total across four locations) | 255 |
| v2 src/chapters audited | 83 |
| v1 src/chapters audited | 92 |
| v2 vol1 / vol2 snapshots audited | 80 |
| Chapters with reframe-language issues | 9 (8 in v2 src/chapters, 1 in v1 only — but several v2 issues replicate in v1) |
| Chapters citing retracted papers | 0 (no chapter cites `FTD_Yang_Mills_Mass_Gap`, `FTD_Navier_Stokes`, `FTD_Thermodynamic_Limit`, `DERIV_THERMODYNAMIC_REFLEXION`, or any of the 11 PDF-only papers archived to `docs/papers/archive/pdf_only_no_source/`) |
| Chapters with critical LEDGER tag mismatches | 7 (1 in v1+v2, 1 v2 only, 5 in v1 only) |
| Chapters where the "Type III₁" classification is overclaimed (LEDGER says HYPOTHESIS) | 1 v2 + likely v1 counterparts |
| Chapters whose physical identifications of x₊ ↔ 1/α and x₋ ↔ N_c match LEDGER | Ch 9 (correctly tagged [SELECTION], close to LEDGER's [STRONGLY MOTIVATED CONJECTURE]); Ch 8 master quadratic correctly [THEOREM] |
| v2 src ↔ vol1 divergences | All 35 v2-vol1 files diverge — but **divergences are systematic editorial macros** (`\Gs`, `\Gssq`) and stripped section numbers, not content drift |
| v2 src ↔ vol2 divergences | 45 v2-vol2 files exist; same systematic-editorial pattern (not deeply audited; representative spot-check on ch 5.1, 9-roots, 11-precision) |

**Headline:** The reframe damage to manuscripts is small and concentrated. The most serious issues are:
1. **Ch 13 (complete-standard-model)** marks parametric insertions (sin²θ_W, α_s, 1/α) as "D" (Derived) — direct contradiction of LEDGER FTD-0013, FTD-0018, FTD-0020.
2. **Ch 20 (measurement)** asserts Type III₁ "in the thermodynamic limit N → ∞" as load-bearing — proscribed under reframe (CANONICAL §"Proscribed Moves" #3) and contradicts LEDGER FTD-0033 demotion to HYPOTHESIS.
3. **Ch 11 (precision-formula)** repeats the "0.001 ppt" 7-term claim that LEDGER FTD-0022 demoted to CONJECTURE (post-hoc fit beyond CODATA precision).
4. **Ch 14.5 (assumption-ledger)** lists α and N_c as `[T] THEOREM` with `0.21 ppt` precision — contradicts LEDGER FTD-0013/0014 (STRONGLY MOTIVATED CONJECTURE).
5. **v1 chapters 1.10b, 1.8a, 14.3, 14.5** carry pre-reframe overclaims that v2 partially fixed (e.g., 1.10b says x₋ → N_c is "PROVEN via RG flow + topological quantization [T]"; 1.8a tags α as `[DERIVED] from master quadratic`).

---

## Per-chapter findings (only chapters with issues)

### v2 src/chapters/

#### Chapter 06: bcc-eigenvalue-watson.qmd
- Path: `dissemination/manuscript_v2/src/chapters/06-bcc-eigenvalue-watson.qmd`
- Issues:
  - Line 18: "The BCC lattice Green's function at the origin **on the infinite lattice $\mathbb{Z}^3$**" — this invokes a completed-totality construction. Under CANONICAL_REFRAME §"Proscribed" #7 ("Claims that the lattice is infinite"), this is proscribed framing for what the authors intend to express (a closed-form algebraic identity defining $W_3$).
  - The actual content is sound (FTD-0002, THEOREM, UNAFFECTED). Only the wording invokes completed infinity.
- Recommended action: **RESTATE** — replace "on the infinite lattice $\mathbb{Z}^3$" with "the BCC Green's function at origin (the closed-form algebraic identity $G(0) = G^{*2}/(2\pi)$, computable to arbitrary finite precision)" or similar.

#### Chapter 09: roots-alpha-and-nc.qmd
- Path: `dissemination/manuscript_v2/src/chapters/09-roots-alpha-and-nc.qmd`
- Issues:
  - Line 25: "What would upgrade [x₊ = 1/α] to [THEOREM]: a proof that the lattice partition function's effective coupling **in the continuum limit** equals the electromagnetic coupling constant" — references "continuum limit" as a permitted upgrade target. Under reframe, the continuum limit as a completed object is proscribed (CANONICAL §"Proscribed" #2). The phrase as stated implies that such a proof is in principle achievable in those terms, which contradicts the reframe.
  - Tag-level: Ch 9 uses `[SELECTION]` for both x₊ ↔ 1/α and x₋ ↔ N_c. LEDGER FTD-0013 and FTD-0014 use the more specific `STRONGLY MOTIVATED CONJECTURE`. The two tags are not identical in the v1.0 ledger taxonomy (LEDGER table at lines 13–23 distinguishes them: SELECTION = "argued from consistency / structural uniqueness, not uniquely proven"; STRONGLY MOTIVATED CONJECTURE = "empirical match + structural uniqueness, not derived"). The chapter's `[SELECTION]` is in the right neighbourhood but is not the same tag the LEDGER uses.
- Recommended action: **OWNER-JUDGMENT** — either restate "continuum limit" → "arbitrarily fine finite spacings", or remove the upgrade pathway entirely (since under reframe it isn't a realisable upgrade). Tag alignment SELECTION↔STRONGLY MOTIVATED CONJECTURE: owner decides whether the manuscript adopts the LEDGER's finer-grained taxonomy.

#### Chapter 11: precision-formula.qmd
- Path: `dissemination/manuscript_v2/src/chapters/11-precision-formula.qmd`
- Issues:
  - Line 69: "FTD 7-term | 137.035999177... (predicted) | < 0.001 ppt | 0" — the LEDGER FTD-0022 demotes the 7-term series to **CONJECTURE (post-hoc fit)** because the claimed precision (sub-ppt) **exceeds CODATA's experimental precision (~11 digits)**. Calling this a "prediction" with `< 0.001 ppt` accuracy is the exact overclaim the LEDGER row was created to retire.
  - Line 71: "if the loop coefficients $c_4$–$c_7$ are confirmed, FTD will match the experimental value to sub-parts-per-trillion" — same overclaim. The c_4–c_7 are tagged `[CONJECTURE]` in the same chapter (line 32) but the section title is `## 11.2 The 7-term precision formula [THEOREM for structure]` which is more confident than LEDGER FTD-0022 supports.
- Recommended action: **RESTATE** — re-tag the 7-term-series claim per LEDGER FTD-0022 (CONJECTURE: post-hoc fit), drop the sub-ppt precision quote, and note that the precision claim cannot be experimentally verified to that level.

#### Chapter 12: mass-spectrum.qmd
- Path: `dissemination/manuscript_v2/src/chapters/12-mass-spectrum.qmd`
- Issues:
  - Line 72: "$\sin^2\theta_W = 3/13 = N_c/N_{\text{eff}}$ is **[SELECTION]**" — LEDGER FTD-0018 explicitly downgrades sin²θ_W to **PARAMETRIC** (2026-04-19). The chapter's `[SELECTION]` is an overclaim relative to the current LEDGER tag.
- Recommended action: **RESTATE** — adjust tag to PARAMETRIC per FTD-0018; either accept FTD as parametric insertion into the standard EWSB formula, or distinguish "structural choice of integers (selection)" from "physical match (parametric)".

#### Chapter 13: complete-standard-model.qmd
- Path: `dissemination/manuscript_v2/src/chapters/13-complete-standard-model.qmd`
- Issues (most serious in the v2 portfolio):
  - Line 17: `1/α | x_+ from master quadratic | 137.036 | 137.036 | 1.26 ppm | **D**` — tagged D (Derived). LEDGER FTD-0013: STRONGLY MOTIVATED CONJECTURE. Direct overclaim.
  - Line 18: `α_s(M_Z) | b_3/(b_3 + 4 N_eff) = 7/59 | 0.1186 | 0.1179 | 0.6% | **D**` — tagged D. LEDGER FTD-0020: PARAMETRIC. Overclaim.
  - Line 20: `sin²θ_W | N_c/N_eff = 3/13 | 0.2308 | 0.2312 | 0.2% | **D**` — tagged D. LEDGER FTD-0018: PARAMETRIC. Overclaim.
  - Line 21: `α_GUT | 1/(2·3·b_3) = 1/42 | 0.0238 | ~0.024 | ~1% | D` — not tracked in LEDGER but should be PARAMETRIC by analogy.
  - Lines 51–53: PMNS angles correctly tagged PI. (Match LEDGER FTD-0021.)
  - Line 59: `ρ_Λ ... | D` — overclaim if treated as derivation; not in LEDGER but worth a separate tag.
- Recommended action: **RESTATE** (urgent). Re-tag the table per LEDGER. The chapter's own framing ("genuine derivation: both formula and numbers come from FTD") is correct in spirit but its table mis-applies it. sin²θ_W and α_s are precisely cases where the formula comes from standard EWSB / RG running and FTD provides the numerical inputs — these are PARAMETRIC, not D.

#### Chapter 14.5: assumption-ledger.qmd
- Path: `dissemination/manuscript_v2/src/chapters/14.5-assumption-ledger.qmd`
- Issues:
  - Line 100: `Fine structure α | 1/137.036 | [T] THEOREM | **0.21 ppt**` — LEDGER FTD-0013 says STRONGLY MOTIVATED CONJECTURE; the "0.21 ppt" precision is the same overclaim as Ch 11.
  - Line 101: `Number of colors N_c | 3.024 → 3 | [T] THEOREM | Exact` — LEDGER FTD-0014 says STRONGLY MOTIVATED CONJECTURE; "Exact" is misleading (LEDGER notes 0.80% agreement).
  - Line 105: `Consciousness y | 2.19 ± 2.86i | [T] THEOREM | Derived` — not in LEDGER; consciousness claims are not formally in scope of the v1.0 LEDGER but tagging this `[T] THEOREM` in an "assumption ledger" chapter is the kind of overclaim the LEDGER framework is designed to suppress.
  - Line 117: `Fibonacci constraint: n_eff = F_7 = 13 = b_3 + 2N_c only satisfied for D = 3` — speculative/numerological, not in LEDGER.
- Recommended action: **RESTATE** the THEOREM tags per LEDGER. This chapter is ostensibly the ledger of assumptions; it must not contradict the canonical LEDGER.

#### Chapter 16: qm-from-flux.qmd
- Path: `dissemination/manuscript_v2/src/chapters/16-qm-from-flux.qmd`
- Issues:
  - Line 3: "Quantum mechanics is not postulated in FTD. It emerges from the lattice dynamics through three steps: complexification of the flux field via the Gauss constraint, construction of a Hilbert space, and **derivation of the Schrodinger equation as the continuum limit of the lattice wave equation**." — proscribed under CANONICAL §"Proscribed" #2.
  - Line 41: "## 16.3 The Schrodinger equation [THEOREM]" plus line 43: "The lattice wave equation (Eq. 14.6) **in the continuum limit** becomes:" — same proscription. Under reframe this needs reframing as "for arbitrarily fine spacings, the lattice wave equation is approximated by..." rather than treating the continuum limit as a completed object.
- Recommended action: **RESTATE** — re-tag "Schrödinger equation" as `[SELECTION]` (because it is recovered in a permitted-but-restated form), and rephrase "continuum limit" as "arbitrarily fine lattice spacing" or use ε–N language per the worked example in CANONICAL_REFRAME.md.

#### Chapter 20: measurement.qmd
- Path: `dissemination/manuscript_v2/src/chapters/20-measurement.qmd`
- Issues (most serious reframe violation):
  - Title: "Measurement: Type III$_1$ to Type I" — the entire framing depends on a Type III₁ classification that LEDGER FTD-0033 demoted from SELECTION to **HYPOTHESIS** (under Araki–Woods scaffold; the Type III₁ is a property of the scaffold, not of FTD-as-defined; every region the framework actually exhibits is Type I).
  - Line 3: "the algebraic type of the observable algebra changes from Type III$_1$ ... to Type I" — same.
  - Line 15: "## 20.2 The type transition [SELECTION]" — should be `[HYPOTHESIS]` per LEDGER FTD-0033.
  - Line 17: "**Pre-measurement (Type III$_1$).** The algebra of observables generated by the continuous flux field $\mathbf{J}$ on the **infinite lattice** is (conjectured to be) a **Type III$_1$ von Neumann algebra**" — invokes "infinite lattice" (proscribed) and the Type III₁ classification (HYPOTHESIS-only).
  - Line 19: "The Type III$_1$ character emerges only in the **thermodynamic limit $N \to \infty$**" — direct invocation of proscribed thermodynamic limit (CANONICAL §"Proscribed" #3) as load-bearing argument.
  - Line 21: "consistent with the spectrum filling $\mathbb{R}_+$ in the **thermodynamic limit**, as required for Type III$_1$" — same proscription.
  - Line 51: "**Self-referential:** The gap equation is self-determining" tagged [SELECTION] — note: the LEDGER FTD-0032 RETRACTED the framing of master quadratic as L → ∞ limit of finite-L gap equation. The "gap equation is self-determining" framing should be qualified to avoid implicitly resurrecting that retracted route.
- Recommended action: **RE-DERIVE** or **RESTATE** with major revisions. Either:
  - (A) restate Type III₁ as a property of an Araki–Woods scaffold attached to FTD's local algebra and label HYPOTHESIS throughout (per LEDGER FTD-0033), or
  - (B) re-derive the measurement story without the Type III₁ → Type I narrative — anchor it on the per-voxel mass gap (LEDGER FTD-0044, THEOREM) and the local manifestation threshold instead.
  - In either case, drop "thermodynamic limit", "infinite lattice" framing as load-bearing.

#### Chapter 24: predictions-status.qmd
- Path: `dissemination/manuscript_v2/src/chapters/24-predictions-status.qmd`
- Issues:
  - Line 11–12: `sin²θ_W | 3/13 = 0.23077 | 0.23122(4) | 0.2% agreement` and `α_s(M_Z) | 7/59 = 0.11864 | 0.1179(9) | 0.6% agreement` — the table doesn't carry an explicit tag column here, but the chapter title is "predictions-status", and listing these as front-line predictions without tagging PARAMETRIC (per LEDGER FTD-0018, FTD-0020) understates the calibration content. Compatible with LEDGER if the chapter explicitly references PARAMETRIC status; otherwise readers will read the row as a derivation.
  - Line 32: "**$\alpha$ incompatible at > 1 ppt.** If the 7-term precision formula's prediction for $\alpha$ is wrong at the parts-per-trillion level (after all loop coefficients are computed), the master quadratic is incorrect." — this falsification claim is unfalsifiable in practice (CODATA precision ~11 digits, sub-ppt is ≥12 digits), and the underlying 7-term claim is LEDGER FTD-0022 CONJECTURE (post-hoc fit). Stating it as a falsification trigger is over-precise.
  - Line 42: "**Lorentz invariance violation at accessible scales.** The lattice breaks continuous Lorentz symmetry. If Lorentz violation is detected at energy scales far below the Planck scale, **the continuum limit fails**." — uses the proscribed "continuum limit" framing, although in this case it could plausibly be restated as "the recovery of approximate Lorentz invariance at large finite scales would fail."
- Recommended action: **RESTATE** — annotate table with tag column matching LEDGER; rewrite the ppt-falsification entry to use ≥10 ppm or a level CODATA can actually probe; rephrase "continuum limit" as "arbitrarily fine spacings."

#### Chapter 5.1: states-of-matter.qmd
- Path: `dissemination/manuscript_v2/src/chapters/5.1-states-of-matter.qmd`
- Issues:
  - Line 24: "Standard thermodynamic laws (PV = nRT, entropy increase) emerge in the **continuum limit**." — proscribed framing. Permitted restatement: "...emerge at arbitrarily large but finite particle numbers."
  - Existing reference to "THEORETICAL_FOUNDATIONS Part IV: Statistical Mechanics" — verify this document still exists and has not been retracted.
- Recommended action: **RESTATE** — single-sentence fix.

#### Chapter 14: action-principle-forces.qmd
- Path: `dissemination/manuscript_v2/src/chapters/14-action-principle-forces.qmd`
- Issues:
  - Line 121: "In the **continuum limit**, full Poincare symmetry is recovered." — proscribed framing.
- Recommended action: **RESTATE** — "for arbitrarily fine lattice spacings, Poincaré symmetry is recovered to arbitrary precision" or similar.

#### Chapter 15: gravity-from-lattice.qmd
- Path: `dissemination/manuscript_v2/src/chapters/15-gravity-from-lattice.qmd`
- Issues:
  - Line 79: "the discrete lattice $\mathbb{Z}^3$ fundamentally violates **continuous diffeomorphism invariance**" — `\mathbb{Z}^3` here is invoked as the "lattice as a totalised object", which is the proscribed reading. Permitted restatement uses the undefined-boundary lattice per FTD-0036 (AXIOM, restated 2026-04-19).
- Recommended action: **RESTATE** — replace `\mathbb{Z}^3` with "the undefined-boundary cubic lattice" or similar.

#### Postulate-1 / Two-Layer-Ontology references (multiple chapters)
- Affected: `01-five-postulates.qmd`, `02-two-layer-ontology.qmd`, `04-moore-decomposition.qmd`, `07-bridge-constant-gstar.qmd`, `16-qm-from-flux.qmd`, `19-observer-formalism.qmd`, `P1-epistemic-framework.qmd`, `P2-mathematical-prerequisites.qmd`
- Issue: All write "$\mathbb{Z}^3$" (the integer lattice as a completed totality) for the lattice. LEDGER FTD-0036 was explicitly restated 2026-04-19 to "undefined-boundary cubic lattice" (no defined boundary; at every specified position, neighbours exist; not a completed-totality $\mathbb{Z}^3$).
- Severity: **lower than the Type III / continuum-limit issues** because every use here is as an indexing set for fields (`s : \mathbb{Z}^3 \to \{-1, 0, +1\}`) rather than as a load-bearing argument step. Per CANONICAL "Permitted" #3 ("Properties that hold at every specified scale"), if the chapter's argument is "for any specified voxel, neighbours exist", this is permitted. The notation $\mathbb{Z}^3$ is shorthand and could be retained with a reframing footnote.
- Recommended action: **OWNER-JUDGMENT** — either (a) blanket-replace `\mathbb{Z}^3` with `\mathbb{L}` (the undefined-boundary lattice) or (b) add a footnote in P1 / P2 / Ch 1 stating that `\mathbb{Z}^3` is shorthand for the undefined-boundary cubic lattice, no completed-totality commitment intended. The latter is much less invasive.

### v1 src/chapters/

#### Chapter 1.10b: master-quadratic-derivation.qmd
- Path: `dissemination/manuscript/src/chapters/1.10b-master-quadratic-derivation.qmd`
- Issues:
  - Line 17: "**C2 ($x_- \to N_c = 3$)**: [T] **PROVEN** via RG flow + topological quantization" — LEDGER FTD-0014 says STRONGLY MOTIVATED CONJECTURE. Major overclaim.
  - Line 472: "QCD beta function coefficient β₀ = 11 - 2n_f/3 = 7 = b_3" — invokes RG flow as derivation route; LEDGER FTD-0032 RETRACTED the "master quadratic as L → ∞ limit of finite-L gap equation" framing on which this argument depended.
  - Line 506: "$x_- \to N_c = 3$ | **PROVEN** (RG flow + topology) | **[T]**" — overclaim.
- Recommended action: **RETRACT** the "PROVEN" framing on row C2 / row 8; cross-reference LEDGER FTD-0014 and FTD-0032; restate as STRONGLY MOTIVATED CONJECTURE.

#### Chapter 1.10a: fermat-encoding.qmd
- Path: `dissemination/manuscript/src/chapters/1.10a-fermat-encoding.qmd`
- Issues:
  - Line 302: "$x_- \to N_c = 3$ via RG flow + topological quantization" — same overclaim as 1.10b. LEDGER FTD-0014.
- Recommended action: **RESTATE** to STRONGLY MOTIVATED CONJECTURE.

#### Chapter 1.8a: forces-from-action.qmd
- Path: `dissemination/manuscript/src/chapters/1.8a-forces-from-action.qmd`
- Issues:
  - Line 197: `α (EM) | 1/137.036 | [DERIVED] from master quadratic` — LEDGER FTD-0013 STRONGLY MOTIVATED CONJECTURE.
  - Line 198: `m_π (Yukawa range) | ~140 MeV | [DERIVED] from confinement scale` — not in LEDGER but [DERIVED] is overclaim relative to FTD's typical confinement-σ tag (LEDGER FTD-0025 SELECTION).
  - Line 196: `G_N (gravity) | ~10⁻³⁸ | [DERIVED] from hierarchy (see Chapter 1.12)` — under reframe, dimensional gravitational claims are conditional on calibration FTD-0030 / FTD-0041 (a_phys ≡ ℓ_P calibration).
- Recommended action: **RESTATE** all three [DERIVED] tags per LEDGER (STRONGLY MOTIVATED CONJECTURE for α; SELECTION/PARAMETRIC for m_π; CALIBRATION-CONDITIONAL for G_N).

#### Chapter 1.8: the-four-forces.qmd
- Path: `dissemination/manuscript/src/chapters/1.8-the-four-forces.qmd`
- Issues:
  - Line 360: "This is **derived**, not input. The slight deviation from exactly 3 represents RG flow corrections at energies above the confinement scale." — same RG-flow overclaim as 1.10b. LEDGER FTD-0014: STRONGLY MOTIVATED CONJECTURE.
  - Line 398: `sin²θ_W = N_c/N_eff = 3/13` — should carry PARAMETRIC tag per FTD-0018.
- Recommended action: **RESTATE** per LEDGER.

#### Chapter 14.3 (v1): glossary.qmd
- Path: `dissemination/manuscript/src/chapters/14.3-glossary.qmd`
- Issues:
  - Line 662: "Electroweak mixing angle. **DERIVED** as sin²θ_W = N_c/N_eff = 3/13 = 0.2308 (0.19% accuracy)." — LEDGER FTD-0018 PARAMETRIC.
- Recommended action: **RESTATE** to PARAMETRIC.

#### Chapter 14.3 (v2): glossary.qmd
- Path: `dissemination/manuscript_v2/src/chapters/14.3-glossary.qmd`
- Issues: same overclaim as v1 line 662 (verified by grep — `manuscript_v2/src/chapters/14.3-glossary.qmd:662` carries the same "DERIVED" string).
- Recommended action: **RESTATE** to PARAMETRIC; sync with v1.

#### Chapter 14.5 (v1): assumption-ledger.qmd
- Path: `dissemination/manuscript/src/chapters/14.5-assumption-ledger.qmd`
- Issues: same as v2 14.5 above (lines 95–105: α and N_c marked [T] THEOREM with sub-ppt precision claim). Verified by grep — same 0.21 ppt entry exists in both v1 and v2.
- Recommended action: **RESTATE** per LEDGER FTD-0013, FTD-0014.

#### Chapter 1.0: before-the-void.qmd
- Path: `dissemination/manuscript/src/chapters/1.0-before-the-void.qmd`
- Issues: contains reframe trigger phrases (flagged by grep, content not deeply audited). Likely uses "thermodynamic limit" or "continuum limit" — not a load-bearing physics chapter (introductory/philosophical), but worth a quick read.
- Recommended action: **OWNER-JUDGMENT** — quick read, restate any load-bearing usage; permissive read of philosophical/narrative usage.

#### Chapter 2.4: quantum-phenomena.qmd, 1.0, 14.10
- These three carry the [DERIVED]/[THEOREM] tag pattern (per grep) — sample should be read for load-bearing claims that conflict with LEDGER. Not deeply audited in this pass.
- Recommended action: **OWNER-JUDGMENT** — sample for LEDGER conflicts.

---

## Vol1/Vol2 divergence audit

**Result:** Per `PROPAGATION_RULE.md`, vol1 and vol2 are publication snapshots and must mirror src/chapters. Diff sweep:

- All 35 vol1 files differ from their src/chapters counterparts.
- All 45 vol2 files differ from their src/chapters counterparts.

**Nature of divergence (sampled on `09-roots-alpha-and-nc.qmd`, `14-action-principle-forces.qmd`, `06-bcc-eigenvalue-watson.qmd`, `5.1-states-of-matter.qmd`):**
- Section headers stripped of section numbers ("## 9.1 The larger root..." → "## The larger root...")
- Consistent use of LaTeX macros for vol builds: `\Gs`, `\Gssq`, etc. in place of `G^*`, `G^{*2}`
- Otherwise identical content

**Verdict:** divergences are systematic editorial transformations applied for volume builds, not content drift. **No divergence introduces a new reframe violation or a new LEDGER mismatch beyond what is already in src/chapters.**

**Caveat:** the spot-check did not cover every chapter; if reframe edits are applied to src/chapters going forward, owners must re-propagate to vol1 / vol2 per the PROPAGATION_RULE, and any vol-specific edits made directly (against the rule) would be lost. Recommended: run the batch sync command from PROPAGATION_RULE.md at the end of every reframe edit cycle.

---

## Cross-references to retracted/archived papers

Searched both manuscript trees for citations to:
- `FTD_Yang_Mills_Mass_Gap` (LEDGER FTD-0042 RETRACTED)
- `FTD_Navier_Stokes` (FTD-0043 RETRACTED)
- `FTD_Thermodynamic_Limit` (FTD-0046 RETRACTED)
- `DERIV_THERMODYNAMIC_REFLEXION` (FTD-0047 RETRACTED)
- The 11 PDF-only papers archived to `docs/papers/archive/pdf_only_no_source/` (FTD-0048):
  - `DERIV_ALPHA_INVERSE_LATTICE_GAUGE`, `DERIV_EMERGENT_GRAVITY`, `DERIV_FUNDAMENTAL_CONSTANTS`, `DERIV_GAUGE_COUPLINGS_DISCRETE_SPACETIME`, `DERIV_QUANTUM_INFERENCE`, `DERIV_SELF_REFERENCE_FOUR_INTEGERS`, `FTD_KMS_Thermal_Time`, `FTD_Modular_Structure`, `FTD_Spatial_Correlations`, `SPEC_MASTER_QUADRATIC_DISCRETE_SPACETIME`, `SPEC_MASTER_QUADRATIC_PAPER`

**Result:** **Zero citations found in either manuscript tree.** The reframe-related paper retractions and the PDF-only archival did not strand any manuscript citations. Clean.

The single tangential mention is `P1-epistemic-framework.qmd` line 61: "Prove confinement rigorously (the lattice strong-coupling expansion shows area-law Wilson loops, but a rigorous proof is a Millennium Prize problem)" — this references Yang-Mills mass-gap as a known open problem in the field, not as an FTD claim, so it is correct under reframe and consistent with LEDGER FTD-0042 (which retracted FTD's claim to have proven YM mass gap).

---

## LEDGER tag mismatches

| Chapter (path) | Claim | Manuscript tag | LEDGER tag | LEDGER row | Severity |
|---|---|---|---|---|---|
| `manuscript_v2/src/chapters/13-complete-standard-model.qmd:17` | 1/α from x₊ | D (Derived) | STRONGLY MOTIVATED CONJECTURE | FTD-0013 | **CRITICAL** |
| `manuscript_v2/src/chapters/13-complete-standard-model.qmd:18` | α_s = 7/59 | D (Derived) | PARAMETRIC | FTD-0020 | **CRITICAL** |
| `manuscript_v2/src/chapters/13-complete-standard-model.qmd:20` | sin²θ_W = 3/13 | D (Derived) | PARAMETRIC | FTD-0018 | **CRITICAL** |
| `manuscript_v2/src/chapters/12-mass-spectrum.qmd:72` | sin²θ_W = 3/13 | [SELECTION] | PARAMETRIC | FTD-0018 | high |
| `manuscript_v2/src/chapters/14.5-assumption-ledger.qmd:100` | Fine structure α | [T] THEOREM, 0.21 ppt | STRONGLY MOTIVATED CONJECTURE; LEDGER notes 1.26 ppm | FTD-0013, FTD-0022 | **CRITICAL** (assumption ledger should not contradict canonical LEDGER) |
| `manuscript_v2/src/chapters/14.5-assumption-ledger.qmd:101` | N_c | [T] THEOREM, "Exact" | STRONGLY MOTIVATED CONJECTURE; 0.80% | FTD-0014 | **CRITICAL** |
| `manuscript_v2/src/chapters/14.3-glossary.qmd:662` | sin²θ_W | DERIVED | PARAMETRIC | FTD-0018 | high |
| `manuscript_v2/src/chapters/20-measurement.qmd:15` | Type III₁ → Type I transition | [SELECTION] | HYPOTHESIS | FTD-0033 | **CRITICAL** |
| `manuscript_v2/src/chapters/20-measurement.qmd:17,19,21` | Type III₁ + thermodynamic-limit framing | inline assertions | HYPOTHESIS only under Araki-Woods scaffold; thermodynamic limit proscribed | FTD-0033 + CANONICAL §"Proscribed" #3 | **CRITICAL** |
| `manuscript_v2/src/chapters/11-precision-formula.qmd:69,71` | 7-term series: < 0.001 ppt | "predicted", "[THEOREM for structure]" | CONJECTURE (post-hoc fit) | FTD-0022 | **CRITICAL** |
| `manuscript_v2/src/chapters/09-roots-alpha-and-nc.qmd` headers | x₊ ↔ 1/α and x₋ ↔ N_c | [SELECTION] | STRONGLY MOTIVATED CONJECTURE | FTD-0013, FTD-0014 | minor (taxonomy alignment) |
| `manuscript/src/chapters/1.10b-master-quadratic-derivation.qmd:17,506` | x₋ → N_c "PROVEN" | [T] PROVEN | STRONGLY MOTIVATED CONJECTURE | FTD-0014, FTD-0032 | **CRITICAL** |
| `manuscript/src/chapters/1.10b-master-quadratic-derivation.qmd:472` | RG-flow derivation route for x₋ | "PROVEN" | RETRACTED framing per FTD-0032 | FTD-0032 | **CRITICAL** |
| `manuscript/src/chapters/1.10a-fermat-encoding.qmd:302` | x₋ → N_c via RG flow + topology | implied derivation | STRONGLY MOTIVATED CONJECTURE | FTD-0014 | high |
| `manuscript/src/chapters/1.8a-forces-from-action.qmd:197` | α | [DERIVED] | STRONGLY MOTIVATED CONJECTURE | FTD-0013 | **CRITICAL** |
| `manuscript/src/chapters/1.8a-forces-from-action.qmd:198` | m_π Yukawa range | [DERIVED] | (confinement σ is FTD-0025 SELECTION; m_π not separately rowed) | FTD-0025 | high |
| `manuscript/src/chapters/1.8-the-four-forces.qmd:360` | x₋ → N_c "derived, not input" | derived | STRONGLY MOTIVATED CONJECTURE | FTD-0014 | high |
| `manuscript/src/chapters/14.3-glossary.qmd:662` | sin²θ_W | DERIVED | PARAMETRIC | FTD-0018 | high |
| `manuscript/src/chapters/14.5-assumption-ledger.qmd` | α, N_c (same as v2) | [T] THEOREM | STRONGLY MOTIVATED CONJECTURE | FTD-0013, FTD-0014 | **CRITICAL** |

Notes:
- The PMNS angles in v2 Ch 13 (lines 51–53) correctly use `PI`. Match LEDGER FTD-0021.
- v2 Ch 8 ("master quadratic [THEOREM]") matches LEDGER FTD-0001 (THEOREM, pure algebra).
- v2 Ch 9 uses `[SELECTION]` for x₊ ↔ 1/α and x₋ ↔ N_c — neighbour-tag of LEDGER's STRONGLY MOTIVATED CONJECTURE. Owner can decide whether to adopt the more granular LEDGER taxonomy or accept the slight discrepancy.

---

## Recommended Phase 4 priority order for owner

Ranked by reader-impact (i.e. how badly a reviewer would react) × ease-of-fix:

1. **`manuscript_v2/src/chapters/13-complete-standard-model.qmd`** — single-table edit; lines 17, 18, 20, 21 to be re-tagged D → SMC / PARAMETRIC per LEDGER. This is the "front-of-house" SM table; most likely to be screenshot, quoted, screenshotted-out-of-context, or used as a citation anchor. **Highest impact, smallest edit.**
2. **`manuscript_v2/src/chapters/14.5-assumption-ledger.qmd`** + **`manuscript/src/chapters/14.5-assumption-ledger.qmd`** — the chapter named "assumption-ledger" must align with the project's canonical LEDGER. Fix lines 95–105; sync between v1 and v2.
3. **`manuscript_v2/src/chapters/20-measurement.qmd`** — Type III₁ framing needs major restatement per LEDGER FTD-0033 + removal of "thermodynamic limit" load-bearing language. This is a chapter rewrite, not a tag tweak.
4. **`manuscript_v2/src/chapters/11-precision-formula.qmd`** — drop the sub-ppt precision claim per LEDGER FTD-0022. Replace with honest precision bracket (CODATA ~11 digits cap) and re-tag the 7-term claim CONJECTURE.
5. **`manuscript/src/chapters/1.10b-master-quadratic-derivation.qmd`** + **`1.10a-fermat-encoding.qmd`** + **`1.8-the-four-forces.qmd`** + **`1.8a-forces-from-action.qmd`** — v1's pre-reframe overclaim cluster around the master-quadratic / RG-flow / topological-quantization derivation route. Per LEDGER FTD-0032, this route is RETRACTED. Either restate to STRONGLY MOTIVATED CONJECTURE or note explicitly that the RG-flow argument was retracted. (Decision: do v1 chapters get reframed, or are they marked "manuscript_v1, pre-reframe" with a banner? Owner judgment.)
6. **`manuscript_v2/src/chapters/12-mass-spectrum.qmd`** — line 72 sin²θ_W tag SELECTION → PARAMETRIC.
7. **`manuscript_v2/src/chapters/14.3-glossary.qmd`** + **`manuscript/src/chapters/14.3-glossary.qmd`** — line 662 sin²θ_W DERIVED → PARAMETRIC; sync v1 + v2.
8. **`manuscript_v2/src/chapters/16-qm-from-flux.qmd`** — three "continuum limit" phrasings (lines 3, 41–43) need ε–N restatement; Schrödinger-equation derivation should be re-tagged.
9. **`manuscript_v2/src/chapters/24-predictions-status.qmd`** — line 11–12 add tag column / context; line 32 ppt-falsification triggers should be replaced with falsifiable thresholds; line 42 "continuum limit" → "arbitrarily fine spacings".
10. **`manuscript_v2/src/chapters/06-bcc-eigenvalue-watson.qmd`** + **`14-action-principle-forces.qmd`** + **`5.1-states-of-matter.qmd`** + **`15-gravity-from-lattice.qmd`** — single-line / single-phrase reframe restatements per the per-chapter sections above. Bundle into one editing pass.
11. **Postulate-1 / `\mathbb{Z}^3` cluster (multiple chapters)** — owner-judgment edit: either blanket-replace `\mathbb{Z}^3` with `\mathbb{L}` or add a one-paragraph footnote in P1 / P2 / 01-five-postulates.qmd noting that the integer-lattice notation is shorthand for the undefined-boundary cubic lattice (FTD-0036). The footnote is much less invasive.
12. **Vol1 / vol2 propagation** — after each above edit, run the batch sync from `PROPAGATION_RULE.md`. Track in `CHANGELOG_REFRAME.md`.

**Fixes 1–4 land 80% of the reframe-credibility benefit at low cost** (re-tagging tables + a single chapter-level rewrite). Fixes 5–12 are completeness work.

---

## End of audit

Audit produced by Manuscript Reframe Audit agent, 2026-04-19, Session 4. Reading-only; no rewrites performed. Owner authority required for restatement / re-derivation / retraction decisions.
