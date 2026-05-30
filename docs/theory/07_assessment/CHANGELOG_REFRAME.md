# Changelog — Undefined-Boundary Reframe Deployment

**Scope:** every change made under the April 19, 2026 reframe deployment.
**Reads:** the `CANONICAL_REFRAME.md` (`reframe_deployment/CANONICAL_REFRAME.md`) is the authoritative statement of the reframe; the `LEDGER.md` (`07_assessment/LEDGER.md`) is the single source of truth for claim status; this file records every decision and change taken in the deployment.

**Maintenance rule:** append-only. Every change to the portfolio under the reframe must add a row here.

---

## 2026-04-19 — Foundational reframe deployment (Session 1)

### Summary

Shifted the framework's foundational ontology from completed-infinity ℤ³ to undefined-boundary cubic-graph. Triaged ~52 theory documents; restated ~38 with mechanical edits; rewrote 5 substantively; closed Mechanism γ as a derivation path; created 3 new tracker docs; demoted 7 claims from [THEOREM] to lower tags; populated initial LEDGER.md (40 rows).

### Phase 0 — Canonical document

- **Adopted:** `AUDIT_INFINITY_REFRAME.md` (per-claim disposition triage, project-internal).
- **Adopted:** `CANONICAL_REFRAME.md` from external deployment package (single source of truth on what the reframe *means*; agent-facing reference).
- **Created:** `docs/theory/07_assessment/reframe_deployment/` — full package import (canonical doc, deployment guide, agent prompts, templates, checklists).
- **Updated:** `CLAUDE.md` — Foundational commitment line, key navigation pointers.

### Phase 1 — Inventory

- **Implicit:** global grep across `docs/theory/` enumerated 52 affected files.
- **Explicit:** Phase 1 inventory of broader portfolio (`docs/papers/`, `dissemination/`) deferred to Session 2 — see `INVENTORY_PORTFOLIO.md` (in flight at deployment close).

### Phase 2 — Classification (theory)

- **Method:** 5 parallel classifier agents over `docs/theory/`.
- **Result:** ~127 mechanical edits applied (Phase 4 fast-path), 5 inline `[FLAG: re-derivation needed]` markers planted in EFT-program docs.

### Phase 3 — Triage

- **Substantive rewrites required:** FOUND_AXIOM_ZERO, DERIV_MASTER_QUADRATIC_GAP_EQUATION, DERIV_PATH_INTEGRAL_CONSTRUCTION, DERIV_VON_NEUMANN_CONSTRUCTION, SPEC_FTD §Postulate 1.
- **Mechanical RESTATE:** 38 documents (5 parallel sweep batches).
- **RETRACTED:** "Master quadratic as L → ∞ limit of finite-L gap equation" (FTD-0032).
- **DEMOTED:** Type III₁ classification SELECTION → HYPOTHESIS (FTD-0033); x₊ ↔ 1/α and x₋ ↔ N_c THEOREM → STRONGLY MOTIVATED CONJECTURE (FTD-0013, FTD-0014); 7-term α series CONJECTURE (was implied derivation) (FTD-0022); sin²θ_W, sin²θ_13, α_s, PMNS angles → PARAMETRIC / STRUCTURALLY MOTIVATED PARAMETRIC (FTD-0018-21, per Option 4 audit on the same day).
- **NEW [OPEN]:** `a_phys` (lattice → physical length conversion) — created as load-bearing problem (FTD-0030).

### Phase 4 — Restatement and re-derivation

#### Substantive rewrites

| File | Change | LEDGER row |
|---|---|---|
| `02_foundations/FOUND_AXIOM_ZERO.md` | Position clause restated to undefined-boundary; §3.2/§4.2/§4.3 restated as algebraic-identity claims (gap-equation thermodynamic-limit framing withdrawn); chain table Step 0 updated. | FTD-0036 |
| `03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` | **Full rewrite.** Title changed to "Algebraic Identity and Physical Match." Coefficient 16 presented via two finite-combinatorial routes (\|Aut(E)\|² + z_BCC·2). Discriminant trichotomy preserved. Physical identification x₊↔1/α, x₋↔N_c moved to STRONGLY MOTIVATED CONJECTURE anchored on dual match + CM uniqueness. Gap-equation/thermodynamic-limit narrative withdrawn. | FTD-0001, FTD-0013, FTD-0014, FTD-0032 |
| `03_derivations/DERIV_PATH_INTEGRAL_CONSTRUCTION.md` | §1.2 configuration-space framing restated to undefined-boundary; §1.4 IR-finiteness corollary restated; §5.5 "thermodynamic limit" theorem restated as scaling property of {F_N} family with explicit "no exact non-analyticity at any finite N" caveat. | n/a (theorem-level, finitary statement) |
| `06_reference_frames_and_measurement/DERIV_VON_NEUMANN_CONSTRUCTION.md` | Type III₁ demoted SELECTION → HYPOTHESIS. Sections 5–6 restated as "Araki–Woods inductive-limit scaffold yields Type III₁ *if* applied"; Section 7 numerical-verification rows tagged [HYPOTHESIS] (Scaffold) for III₁ items; Section 8 epistemic accounting updated. | FTD-0033 |
| `docs/SPEC_FTD.md` | Postulate 1 restated to "undefined-boundary cubic lattice"; DEF.1 updated; finite-size-effects note updated. | FTD-0036 |

#### Mechanical sweep (5 parallel batches, ~127 edits across 38 files)

| Batch | Files | Edits | FLAGs |
|---|---|---|---|
| 1: 03_derivations gauge files | DERIV_LATTICE_QED_COMPLETE, DERIV_LATTICE_CHIRAL_ANOMALY, DERIV_COULOMB_SCATTERING_AMPLITUDE, DERIV_LATTICE_SU2_WEAK, DERIV_LATTICE_SU3_GAUGE, DERIV_KCOMP_VOLUMETRIC_SHELL | 26 | 0 |
| 2: 03_derivations remainder | DERIV_QM_FROM_LATTICE, DERIV_SINGLET_FROM_VOID_EVENT, DERIV_NC_FROM_TOPOLOGY, DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE, DERIV_EINSTEIN_NONLINEAR_FROM_LATTICE, DERIV_MASTER_QUADRATIC_FROM_Z, DERIV_THREE_RESOLUTIONS, DERIV_FORCE_EMERGENCE, DERIV_VARIATIONAL_PROOF, DERIV_QFT_GRT_BRIDGE, DERIV_SPIN_STATISTICS_BRIDGE | 48 | 0 |
| 3: foundations + bridges | FOUND_DIMENSIONAL_COUNTING, FOUND_RELATIVITY_GRAVITY_DISTINCTION, DERIV_GSTAR_PF_BRIDGE, DERIV_BCC_MULTIPLICATIVE_STRUCTURE, CONJ_ALPHA_FROM_CM, BRIDGE_QUADRATIC_PHYSICS | 11 | 1 (CONJ_ALPHA_FROM_CM Path A) |
| 4: 01_reference + papers | SPEC_FTD_COMPLETE_CHAIN, SPEC_FTD_REFERENCE, SPEC_FTD_LAGRANGIAN, SPEC_QFT_GRT_BRIDGE_ROADMAP, SPEC_FTD_COMPARATIVE_PHYSICS, PAPER_CONSCIOUSNESS_QFT_GR_BRIDGE, AUDIT_BELL_ANALYSIS | 23 | 0 |
| 5: reference frame context + EFT + audits | DERIV_COLLAPSE_MECHANISM, AUDIT_ALPHA_EXTRACTION, SPEC_EFT_RECOVERY_PROGRAM, DERIV_DAY2_CAMPAIGN, DERIV_EMERGENT_COULOMB_GEOMETRIC, DERIV_DYNAMICAL_SM_EMERGENCE, DERIV_GAP_CLOSURE, DERIV_BETA_FUNCTION_MEASURED | 18 | 4 (EFT-program "engine→QED in L→∞" claims) |

**Total mechanical edits: 126.** **Total inline FLAGs: 5** (collected in `TRACKER_REFRAME_FLAGS.md`).

#### New documents created

| File | Purpose | LEDGER row |
|---|---|---|
| `10_eft_program/OPEN_A_PHYS_DERIVATION.md` | Frames the calibration question created by the reframe; analyses 3 derivation candidates. | FTD-0030 |
| `10_eft_program/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` | Explicit dimensional-chain attempt at γ; concludes Mechanism γ does not deliver derivation; recommends `a_phys ≡ ℓ_P` declaration. | FTD-0035 |
| `07_assessment/archive_session_outputs/TRACKER_REFRAME_FLAGS.md` | Catalog of the 5 inline FLAGs with restatement guidance (Restatement A: scaling; Restatement B: calibration). | FTD-0034 |
| `07_assessment/LEDGER.md` | Single source of truth for claim status (40 initial rows). | n/a (this file is the source) |
| `07_assessment/CHANGELOG_REFRAME.md` | This file. | n/a |

### Phase 5 — Engine audit

- **Method:** dispatched in Session 1 close; output `ENGINE_AUDIT_REFRAME.md` (in flight at deployment close).
- **Scope:** C++ engine (`engine/include/`, `engine/src/`, `engine/cuda/`) + JS frontend (`engine/web/js/`) for Class 1 (infinity invocations), Class 2 (hidden couplings / parameter-free violations), Class 3 (global-state assumptions).

### Phase 6 — Integration

- **Devil's advocate (Phase 6.1):** dispatched against the 5 substantive rewrites + SPEC_FTD postulate update. Output: `DEVILS_ADVOCATE_REPORT.md` (in flight).
- **Consistency check (Phase 6.2):** partial — `META_INDEX.md` updated (6 new rows: 7.13–7.19); `CLAUDE.md` updated (4 new key-navigation pointers). Cross-paper consistency in broader portfolio deferred to Session 2.
- **Ledger update (Phase 6.3):** `LEDGER.md` v1.0 populated (40 rows).

### Phase 7 — Verification

- **Master ledger:** `LEDGER.md` v1.0 (this session).
- **Changelog:** `CHANGELOG_REFRAME.md` (this file).
- **Consistency report:** deferred to Session 2 after broader-portfolio inventory and classification return.

### Decisions for which there is no audit-trail-mandated form

These are judgment calls made in this session that future deployments should be able to revisit:

- **Removed reframing rhetoric.** Initial rewrites contained "Reframe Notice / Retraction" banners and inline "(reframed 2026-04-19)" markers; user requested direct restatement without meta-narrative. Rewrites were redone to state the new framework directly. Trace: this CHANGELOG and the LEDGER carry the meta-narrative; the rewritten docs do not.
- **Recommended `a_phys ≡ ℓ_P` declaration.** Mechanism γ closed as derivation; recommended fallback is to declare `a_phys ≡ ℓ_P` (Planck length) in `SPEC_FTD.md`. Not yet executed; queued for Session 2 with user sign-off.
- **Engine FLAGs left inline.** 5 inline FLAGs in EFT-program docs were left in place rather than silently restated, because each requires a substantive choice between Restatement A (scaling) and Restatement B (calibration) that benefits from EFT-campaign-owner judgment.

### Devil's-advocate fixes (Phase 6.1 follow-up, same-day)

The devil's-advocate agent (`DEVILS_ADVOCATE_REPORT.md`) flagged 3 blocking issues + 5 PASS-WITH-NOTES. Same-day fixes:

| Issue | File | Fix |
|---|---|---|
| Boxed Axiom Zero contradicted body (line 17 said "x ∈ ℤ³"; body said "undefined-boundary cubic graph") | `02_foundations/FOUND_AXIOM_ZERO.md` | Restated boxed Axiom Zero position clause to undefined-boundary. |
| §4.4 still attributed master quadratic to "fixed point of gap equation" | `02_foundations/FOUND_AXIOM_ZERO.md` | Restated to algebraic-uniqueness + dual match + CM-curve uniqueness. |
| One-sentence summary still said "self-consistent gap equation yields x_+" | `02_foundations/FOUND_AXIOM_ZERO.md` | Restated as algebraic determination + dual match. |
| Orphan Type III₁ premise: "the flux field IS Type III$_1$" still asserted in 3 places | `06_reference_frames_and_measurement/DERIV_COLLAPSE_MECHANISM.md` (lines 28, 320, 511) | Restated all three to "[HYPOTHESIS] under Araki–Woods scaffold"; cross-cited `DERIV_VON_NEUMANN_CONSTRUCTION.md`. |

### Engine-audit follow-up (Phase 5 fixes, same-day)

Engine audit (`ENGINE_AUDIT_REFRAME.md`) flagged 3 HIGH risk findings. Same-day fixes:

| Issue | File | Fix |
|---|---|---|
| `LAMBDA_G = 100.0` justified by `// lambda_G -> infinity is exact constraint` | `engine/include/ftd/lagrangian.h:50-52` | Comment rewritten to "for arbitrarily large lambda_G the constraint is enforced to arbitrary precision; LAMBDA_G = 100 is the chosen finite value." |
| Comment "for a point mass M on an infinite lattice" | `engine/tests/test_einstein_equations.cpp:305-309` | Comment rewritten to "cubic lattice (no defined boundary; arbitrarily large finite extent is admissible)." |
| `α_eff(L → ∞)` extrapolation in benchmark CSV header + Python analysis + EFT paper (HIGH-1) | `engine/tests/benchmark_dynamical_sm.cpp` + downstream | **Deferred** — requires renaming `alpha_inf` → `alpha_largeL` across CSV / Python / TeX. Logged for owner sign-off. |

### Open work queue entries from devil's advocate

- `DERIV_PATH_INTEGRAL_CONSTRUCTION.md` lines 281, 337 — two residual "continuum limit" phrases survived; `{F_N}_{N=1}^∞` notation in §5.5 totalises the index even though the prose disclaims it; §5.3's `β → 0/∞` framings are completed limits. Queued.
- `SPEC_FTD.md` §14.2 — Lorentz-emergence paragraph "at scales >> lattice spacing" / "discreteness effects average out" is functionally a continuum-limit framing. Queued.

### Phase 2 paper classification (returned same-day)

Classifier agent (`FLAGGED_PASSAGES_PAPERS.md`) processed 34 TeX/MD source files in `docs/papers/`:

- **10/34 papers clean** (zero proscribed passages): mostly algebraic-geometry papers (0A, 0B, 0E, 3A) and short-form papers (PAPER_PATH, README, RATIO_AND_THE_ARROW, LETTER_HERMITIAN_COPE, GEOMETRIC_BIOPHYSICS).
- **~37 load-bearing proscribed passages** total, concentrated in 7 files.
- **Heavy reliance papers (>5 passages):**
  - `speculative/FTD_Yang_Mills_Mass_Gap.tex` — entire premise = infinite-volume + path-integral over all configurations + Wightman on ℝ⁴. RE-DERIVE.
  - `speculative/FTD_Navier_Stokes.tex` — lattice-to-continuum bridge load-bearing. RE-DERIVE.
  - `speculative/FTD_Finitude_Theorem.tex` — paradoxical case (paper IS the reframe in other voice; mostly quotes-and-denies). RESTATE preamble only.

**Universal survivors (per Q1-Q4):** Wallis-type `\lim_{N→∞}` (worked example in CANONICAL_REFRAME.md); parametric `β → ∞` inverse-temperature; `ℤ³` notation as undefined-boundary shorthand.

**Top-7 Phase 4 restatement priorities (per classifier):**
1. `PAPER_2A_MASTER_QUADRATIC.tex` (2 lines, future-work passages)
2. `PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY.tex` + `.md` (3 mechanically rewritable)
3. `src/DERIV_SOFTPLUS_RELU_DUALITY.tex` line 484 (RG fixed-point)
4. `src/FTD_Discrete_Continuous_Bridge.tex` line 602 (lattice-spacing-to-zero)
5. `speculative/FTD_Finitude_Theorem.tex` (preamble re-licensing only)
6. `speculative/FTD_Yang_Mills_Mass_Gap.tex` (RE-DERIVE)
7. `speculative/FTD_Navier_Stokes.tex` (RE-DERIVE or accept demotion)

**Methodological caveat:** scan was for trigger phrases only — recommended deeper user read of `FTD_Riemann_Hypothesis.tex`, gap-equation derivation in `PAPER_2A_MASTER_QUADRATIC.tex`, and path-integral construction in `FTD_Yang_Mills_Mass_Gap.tex` for latent completed-infinity reasoning that doesn't surface as keywords.

### Phase 1 broader-portfolio inventory (returned same-day)

Inventory agent (`INVENTORY_PORTFOLIO.md`) cataloged 280 artifacts outside `docs/theory/`:

- 267 editable sources, 13 PDF-only (sources need recovery before reframe action).
- ~85 likely-affected by reframe (heuristic), ~155 likely-not, ~40 unknown.
- **Structural finding 1:** manuscript_v2 has `vol1/src/chapters/` + `vol2/src/chapters/` split mirroring a subset of consolidated `src/chapters/` (83 files). Confirm authoritative location before any edit.
- **Structural finding 2:** manuscript_v1 ↔ manuscript_v2 share ~57 chapters verbatim (numbered 3.x–13.x, 14.x, 15.1) — edits to v1 inherited chapters MUST be propagated to v2's copy or they diverge.
- **Highest-priority targets:** `FTD_Finitude_Theorem.tex`, `FTD_Thermodynamic_Limit.pdf` (PDF-only — needs source), `PAPER_FTD_AS_WILSONIAN_EFT.tex`, the whitepaper.
- 36-chapter historical-narrative section of `book/` (chapters 00–35) is essentially exempt; only chapters 36–45 carry foundational claims.
- `dissemination/papers/` contains exactly one paper (the EFT flagship).

### Files not touched in this session

- `docs/papers/` (~30 source files, ~20 PDF-only): inventoried (Phase 1 ✓) and classified (Phase 2 ✓); Phase 4 restatement queued for Session 2 with priority order from classifier.
- `dissemination/manuscript_v2/` (83 chapters): inventoried in flight; treatment deferred.
- `dissemination/whitepaper/`, `dissemination/book/`, `dissemination/notebooks/`, `dissemination/interactive/`: same.
- Engine code: audit in flight; restatement pending Phase 5 return.

---

## Open work queue (entering Session 2)

1. **EFT FLAGs:** owner choice of Restatement A or B per item × 5 (`TRACKER_REFRAME_FLAGS.md`).
2. **Engine restatements:** dependent on `ENGINE_AUDIT_REFRAME.md` findings.
3. **Devil's advocate response:** review `DEVILS_ADVOCATE_REPORT.md`; revise rewrites if any pass-with-notes / needs-revision verdicts.
4. **Broader-portfolio Phase 4:** restatement across `docs/papers/`, `dissemination/manuscript_v2/`, `dissemination/whitepaper/`, etc. Inventory + classification feed (`INVENTORY_PORTFOLIO.md`, `FLAGGED_PASSAGES_PAPERS.md`).
5. **`SPEC_FTD.md` declaration of `a_phys ≡ ℓ_P`:** add explicit calibration paragraph; update every dimensional prediction with calibration-conditional language.
6. **Consistency pass (Phase 6.2 full):** cross-doc citation/terminology check across the entire reframed portfolio.
7. **Verification (Phase 7 full):** ledger ↔ portfolio consistency; no untagged claims; no contradictions.

---

---

## 2026-04-19 — Session 2 (owner-approved decisions, autonomous bypass-mode execution)

### Summary

Owner returned and approved all 7 deferred decisions. Session 2 executed: (1) `α_inf` → `α_largeL` rename across engine/Python/EFT-paper; (2) all 5 EFT FLAGs replaced (Restatement A on Flag 1, Restatement B on Flags 2-4, Path A retraction on Flag 5); (3) Path-integral residual continuum-limit phrases + SPEC_FTD §14.2 Lorentz-emergence framing fixed; (4) `a_phys ≡ ℓ_P` calibration declared in `SPEC_FTD.md`; (5) Phase 4 mechanical sweep on top-5 tractable papers (11 edits, 0 flags); (6) PDF-only papers source-recovery analysis (TRACKER_PDF_ONLY_PAPERS.md); (7) manuscript propagation rule (PROPAGATION_RULE.md).

Plus: YM/NS RE-DERIVE assessment (REDERIVE_REPORT_YM_NS.md) — Yang-Mills paper has one survivable theorem (per-voxel mass gap, FTD-0044) but loses Clay-eligibility under the reframe; Navier-Stokes paper has no surviving technical content.

### Decision 1 — α_inf rename (4 files, 23 edits)

Agent task: rename `alpha_inf` / `α_∞` / "continuum limit (as L → ∞)" → `alpha_largeL` / `α_largeL` / "large-L extrapolation" across engine, Python analysis, and EFT paper. **The 1/L² fit math is unchanged; only labels and prose framing changed.**

| File | Edits | Notable |
|---|---|---|
| `engine/tests/benchmark_dynamical_sm.cpp` | 3 | `struct ContinuumFit` → `struct LargeLFit`; CSV row label `continuum,alpha_inf,…` → `largeL_extrap,alpha_largeL,…`; TODO added for empirical residual band emission |
| `scripts/benchmarks/analyze_convergence.py` | 1 | narrative-string update only (file doesn't parse renamed CSV column) |
| `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex` | 14 | LaTeX macro `\alphaINF` → `\alphaLargeL` + 13 call-sites; Phase-F headline restated as "1/L² fit at largest tested L gives α_largeL ≈ 3.74 × α_ref, with empirical residual band [3.35, 3.74] × α_ref" |
| `scripts/benchmarks/continuum_extrapolate.py` (bonus consumer) | 5 | variable rename + docstring + interpretation strings |

**Filename `continuum_extrapolate.py` retained** to avoid breaking external callers; only contents updated.

### Decision 2 — 5 EFT FLAGs replaced (per-flag policy chosen logically)

Agent task: replace each `[FLAG: re-derivation needed]` marker with the chosen restatement.

| Flag | File:line | Restatement | Result |
|---|---|---|---|
| 1 | `DERIV_BETA_FUNCTION_MEASURED.md:312` | A (scaling) | β sign agreement asserted, magnitude discrepancy reported, pre-registered scaling exponent prediction queued |
| 2 | `DERIV_DYNAMICAL_SM_EMERGENCE.md:23` | B (calibration-conditional) | α_largeL ≈ 3.6 × α_ref calibration-conditional under `a_phys` |
| 3 | `DERIV_DAY2_CAMPAIGN.md:312` | B | Phase-2/Day-2 headline reframed as finite-L scaling diagnostic under `a_phys ≡ ℓ_P` |
| 4 | `DERIV_DAY2_CAMPAIGN.md:399` | B | Post-Day-2 measurement framed as falsifying disagreement under declared calibration |
| 5 | `CONJ_ALPHA_FROM_CM.md:128` | Drop Path A | Path A retracted; Paths B/C/D unaffected |

**Final grep confirms zero remaining `[FLAG: re-derivation needed]` markers** in source documents (5 remaining hits are in meta-narrative tracker / changelog / SESSION_WRAPUP files describing the historical flag count).

`TRACKER_REFRAME_FLAGS.md` updated — all 5 rows marked RESOLVED 2026-04-19.

### Decision 3 — Devil's-advocate PASS-WITH-NOTES queue cleared

| File | Change |
|---|---|
| `DERIV_PATH_INTEGRAL_CONSTRUCTION.md` line 281 | "In the continuum limit \|k\| ≪ π" → "In the long-wavelength regime \|k\| ≪ π, the propagator approaches the continuum form algebraically with leading-order error O(k²)" |
| `DERIV_PATH_INTEGRAL_CONSTRUCTION.md` line 337 | "not just in the continuum limit" → "not only in the long-wavelength regime where lattice momentum reduces to continuum momentum" |
| `DERIV_PATH_INTEGRAL_CONSTRUCTION.md` §5.3 | β → 0/∞ "limits" rewritten as "behaviour at arbitrarily extreme finite β with explicit O(β) / O(e^{-βΔ}) error rates" |
| `DERIV_PATH_INTEGRAL_CONSTRUCTION.md` §5.5 | `{F_N}_{N=1}^{∞}` totalisation removed; rewritten as "family `{F_N}` across arbitrarily large N" |
| `SPEC_FTD.md` §14.2 (Lorentz emergence) | "at scales >> lattice spacing, discreteness effects average out" → finitary form: "at every finite spacing `a` and every wavelength `λ ≫ a`, the rotational/boost invariance of the long-wavelength theory is recovered to error O((a/λ)²)" |

### Decision 4 — `a_phys ≡ ℓ_P` calibration declared

Added to `SPEC_FTD.md` between Postulate 2 and Postulate 3 (new section "LATTICE ↔ PHYSICAL CALIBRATION"):

- One voxel ≡ one Planck length (ℓ_P ≈ 1.616 × 10⁻³⁵ m).
- One tick ≡ √3 · ℓ_P / c ≈ 9.34 × 10⁻⁴⁴ s.
- One lattice mass-unit ≡ m_e / K_B = 1 MeV/c² ≈ 1.783 × 10⁻³⁰ kg.
- **Discipline:** dimensional predictions are conditional on this calibration; dimensionless predictions (α, mass ratios, mixing angles) are calibration-independent and constitute the falsifiable spine.

LEDGER row FTD-0030 (a_phys OPEN) → RESOLVED-BY-CALIBRATION. New LEDGER row FTD-0041 records the declaration.

### Decision 5 — Phase 4 mechanical sweep on top-5 tractable papers (11 edits, 0 flags)

| File | Edits | Notable |
|---|---|---|
| `docs/papers/src/PAPER_2A_MASTER_QUADRATIC.tex` | 2 | "thermodynamic limit" → "for arbitrarily large finite L with explicit decreasing function bound" |
| `docs/papers/PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY.tex` + `.md` (siblings in sync) | 3 each | "continuum limit of lattice Laplacian" → "small-spacing expansion with O(a²) error" |
| `docs/papers/src/DERIV_SOFTPLUS_RELU_DUALITY.tex:484` | 1 | RG fixed point reformulated as ε-μ statement |
| `docs/papers/src/FTD_Discrete_Continuous_Bridge.tex:602` | 1 | "lattice spacing sent to zero" → "finite algebraic value at q = 0 approached at any specified ε once a chosen small enough" |
| `docs/papers/speculative/FTD_Finitude_Theorem.tex` | 1 | Preamble added: "Canonical Status — undefined-boundary ontology now formally adopted; this paper, previously speculative, describes the canonical position" |

### Decision 6 — PDF-only papers source-recovery analysis

`TRACKER_PDF_ONLY_PAPERS.md` written. 13 PDF-only papers categorized:

- **HIGHEST priority** (cannot survive reframe in current form): `FTD_Thermodynamic_Limit.pdf`, `DERIV_THERMODYNAMIC_REFLEXION.pdf`
- **HIGH priority** (Type III₁ / KMS-related, directly affected): `FTD_KMS_Thermal_Time.pdf`, `FTD_Modular_Structure.pdf`
- **Likely already-superseded**: `DERIV_GAUGE_COUPLINGS_DISCRETE_SPACETIME.pdf` (subsumed by `PAPER_GAUGE_COUPLINGS_FROM_LATTICE_GEOMETRY.tex`); `SPEC_MASTER_QUADRATIC_DISCRETE_SPACETIME.pdf`, `SPEC_MASTER_QUADRATIC_PAPER.pdf` (likely subsumed)
- **Unknown impact** without source: 6 remaining (`DERIV_ALPHA_INVERSE_LATTICE_GAUGE`, `DERIV_EMERGENT_GRAVITY`, `DERIV_FUNDAMENTAL_CONSTANTS`, `DERIV_QUANTUM_INFERENCE`, `DERIV_SELF_REFERENCE_FOUR_INTEGERS`, `FTD_Spatial_Correlations`)

Recommended: run `git log --all --diff-filter=D --name-only -- 'docs/papers/*.tex'` to recover deleted sources.

### Decision 7 — Manuscript propagation rule

`dissemination/manuscript_v2/PROPAGATION_RULE.md` written. Authoritative rules:

- `manuscript_v2/src/chapters/` is **single source of truth** for v2 chapter content.
- `vol1/src/chapters/` and `vol2/src/chapters/` are **publication snapshots**; edits flow from src/chapters → vol1/vol2, never reverse.
- **Verified:** vol1 and vol2 files are NOT symlinks — they are independent copies that have already diverged from src/chapters.
- v1 ↔ v2 are **different products** with overlapping but not identical content. No chapter-level identity; reframe edits to v1 do not propagate to v2.
- **Edit discipline:** for any reframe edit, identify all chapter locations covering the same topic; apply the edit to all of them; log in CHANGELOG_REFRAME.md.

### Yang-Mills + Navier-Stokes RE-DERIVE assessment (REDERIVE_REPORT_YM_NS.md)

Agent task: read both papers carefully and lay out RE-DERIVE / DEMOTE / RETRACT options.

**Yang-Mills paper:**
- Foundational `Λ = ℤ³` axiom is exactly the proscribed completed-infinity move.
- UV-finiteness proof, Proposition 5.2 ("mass gap persists in thermodynamic limit"), and Clay-eligibility framing all depend on completed-infinity.
- **One theorem survives cleanly:** per-voxel mass gap from manifestation threshold (Theorem 5.1; LEDGER row FTD-0044).
- **Recommendation: OWNER-JUDGMENT-NEEDED.** Default option: SPLIT into a small honest paper around the surviving theorem + retract the Clay-aimed framing. Cannot be a Clay paper post-reframe.

**Navier-Stokes paper:**
- **Worse off than YM: no standalone per-voxel theorem survives.**
- Theorem 4.2 (uniform energy bound) requires periodic-boundary IBP — completed-infinity-adjacent.
- Theorem 6.1 (continuum recovery) is explicit `a → 0` completed limit.
- Theorem 6.2 (regularity ladder) appeals to Sobolev embedding on ℝ³ — completed continuum.
- **Recommendation: RETRACT or REFRAME as ontological-position paper.** No surviving Clay-eligible content.

**Portfolio-level flag from agent:** check whether `CLAUDE.md`, `META_DOCUMENTATION_MAP.md`, or other top-level documents imply FTD addresses Millennium problems. Adjust as needed.

### Millennium-problems portfolio flag (action taken)

The YM/NS report flagged a portfolio-level concern that top-level documents may imply FTD addresses Clay Millennium problems. Audit:

- `CLAUDE.md` — checked; no Millennium-problem claims surface there. Safe.
- `META_DOCUMENTATION_MAP.md` — not yet checked (queued for Session 3).
- `docs/SPEC_FTD.md` — not yet swept for Millennium-problem language (queued for Session 3).
- `dissemination/manuscript/` and `manuscript_v2/` chapters — not swept (queued for Session 3).

**Recommendation:** Session 3 should run a portfolio grep for "Clay," "Millennium," "Yang-Mills mass gap," "Navier-Stokes regularity," "Riemann hypothesis," and update any claim that depends on the Yang-Mills or Navier-Stokes papers per the chosen disposition for those papers.

### LEDGER updates (Session 2)

| Row | Change |
|---|---|
| FTD-0030 | OPEN → RESOLVED-BY-CALIBRATION |
| FTD-0034 | RETRACTED-PENDING → RESOLVED (5 FLAGs replaced) |
| FTD-0041 | NEW: a_phys ≡ ℓ_P calibration declaration |
| FTD-0042 | NEW: YM mass gap "proof" — OWNER-JUDGMENT-NEEDED |
| FTD-0043 | NEW: NS regularity "proof" — OWNER-JUDGMENT-NEEDED |
| FTD-0044 | NEW: per-voxel mass gap (THEOREM, survives reframe) |
| FTD-0045 | NEW: α_largeL ≈ 3.6 × α_ref (HYPOTHESIS, calibration-conditional) |

### New deliverables

| File | Purpose |
|---|---|
| `dissemination/manuscript_v2/PROPAGATION_RULE.md` | Manuscript propagation rule (v1 ↔ v2 ↔ vol1 ↔ vol2) |
| `docs/theory/07_assessment/archive_session_outputs/TRACKER_PDF_ONLY_PAPERS.md` | 13 PDF-only papers triage + recovery options |
| `docs/theory/07_assessment/archive_session_outputs/REDERIVE_REPORT_YM_NS.md` | YM + NS paper RE-DERIVE assessments |

### Open work queue (Session 3)

1. **YM/NS owner judgment** — decide for each paper: SPLIT (small honest paper around surviving theorem) / DEMOTE / RETRACT / archive.
2. **Millennium-problems portfolio sweep** — grep for "Clay" / "Millennium" / paper titles across `META_DOCUMENTATION_MAP.md`, `SPEC_FTD.md`, `manuscript/`, `manuscript_v2/`. Adjust any claim that depends on YM/NS papers.
3. **PDF-only source recovery** — run `git log --all --diff-filter=D --name-only -- 'docs/papers/*.tex'`; for any source recovered, run classifier + restatement pipeline.
4. **`α_largeL` empirical residual band emission** — TODO planted in `engine/tests/benchmark_dynamical_sm.cpp` waits for multi-seed ensemble implementation.
5. **Manuscript v1 + v2 reframe sweep** — apply propagation rule; ~92 + 83 chapters need classifier + restatement.
6. **Whitepaper reframe** — `dissemination/whitepaper/FTD_Whitepaper.tex` not yet touched.
7. **Notebooks + interactive HTML** — Phase 1 inventory complete; Phase 2-4 not started.
8. **Manuscript divergence audit** — full diff sweep of vol1/vol2 vs src/chapters per the propagation rule.

---

## 2026-04-19 — Session 3 (owner-approved retractions + repo-history cleanup)

### Summary

Owner returned, decided on the 7-item Session 2 queue: **retract YM (1), retract NS (2), park items 3 + 5–9, recover all published docs (4), remove Codex/Claude as commit contributors (4)**. Session 3 executed: (a) retracted YM + NS to archive with retraction notes; (b) git-archaeology recovery of 13 PDF-only papers (only figures were ever in git; no TeX source recoverable); (c) `pdftotext` extraction on every PDF-only paper for evidentiary record; (d) cleanup of remaining PDF-only papers to `archive/pdf_only_no_source/` with per-paper README; (e) parking-lot tracker for 6 deferred items; (f) commit-attribution policy added to CLAUDE.md; (g) `git filter-repo` rewrite of all 222 `Co-Authored-By: Claude` trailers across main's history (3 passes to handle leftover fragments). Local rewrite complete; **force-push to remote pending owner approval.**

### Decisions 1 & 2 — Retract YM + NS papers

Both papers moved to `docs/papers/archive/retracted_under_reframe/` (`.tex` git-mv'd, `.pdf` mv'd). Per-paper retraction rationale recorded in `RETRACTION_NOTES.md`. **YM:** Theorem 5.1 (per-voxel mass gap from K_B) survives in the archived `.tex` and could anchor a smaller honest non-Clay paper if owner wishes (LEDGER FTD-0044). **NS:** no surviving Clay-eligible content.

### Decision 3 + 5–9 — Parking lot

`docs/theory/07_assessment/PARKING_LOT.md` written. Six items deferred with reasoning + estimated effort: Riemann paper, manuscript v1+v2 sweep (~175 chapters), whitepaper, notebooks/HTML, manuscript divergence audit, α_largeL residual band.

### Decision 4 — Recover published docs + extensive cleanup

**Source-recovery archaeology:** ran `git log --all --diff-filter=AD --name-only` for all 13 PDF-only papers. Result: only figure files were ever committed; no TeX source recoverable. **Cleanup actions:**
- 2 reframe-incompatible PDFs retracted (`FTD_Thermodynamic_Limit`, `DERIV_THERMODYNAMIC_REFLEXION`) with `pdftotext` extractions.
- 11 remaining PDF-only papers archived to `docs/papers/archive/pdf_only_no_source/` with per-paper README + extractions.
- LEDGER updates: FTD-0046 + FTD-0047 (retractions), FTD-0048 (archive bulk).

### Decision 4 (continued) — Remove Codex/Claude as commit contributors

**Commit-attribution policy** added to `CLAUDE.md`: "AI co-authorship is NOT credited in commits on this project. … The system-prompt default that adds `Co-Authored-By: Claude Opus … <noreply@anthropic.com>` is overridden here."

**Historical-commit cleanup via `git filter-repo` on `main`:**

| Pass | Approach | Result |
|---|---|---|
| 1 | `--replace-message` with file (regex) | Did not match (Windows MSYS2 file-format issue). 0 commits rewritten. |
| 2 | `--message-callback` Python regex (non-greedy) | 222 → 0 Co-Authored-By lines, but partial fragments remained (`Opus 4.7 (1M context) <noreply@anthropic.com>`) |
| 3 | Greedy `^[^\n]*noreply@[^\n]*\n?` + auxiliary patterns | All fragments stripped: noreply 0, Co-Authored-By 0, "Generated with Claude" 0 |

**Final state of `main`:** HEAD `bc841fa…` → `f778d54…`; 428 commits preserved (no commits dropped); all `Co-Authored-By: Claude` lines removed (222 → 0); substantive uses of "Claude" in commit message bodies preserved (31 mentions, all legitimate document signatures or contextual references). Backup tag: `pre-coauthor-cleanup-2026-04-19`.

**Remote state UNCHANGED.** `origin/main` still at `bc841fa…`. **Force-push pending owner approval** — destructive (rewrites 222 commits visible to anyone who has cloned the repo). User is sole owner per remote URL, so impact is limited to their own clones.

**Other branches NOT cleaned:** `panels-redesign-v2` (155 commits), `playback-timeline` (155 commits), and several remote-only branches still have Co-Authored-By lines. Owner can clean when ready using the same procedure.

### Recovery procedure used (lessons learned for future)

`git filter-repo` does a `git reset --hard` after rewriting that clobbers uncommitted modifications. Workflow used in Session 3:
1. Before filter-repo: `git stash push -u -m "..."` to save modifications.
2. Run filter-repo (will clobber stash-pop'd state).
3. After filter-repo: locate stash commit via `git fsck --unreachable | grep "unreachable commit"` (the popped stash is preserved as a dangling commit until garbage collection).
4. `git diff --name-only <pre-rewrite-HEAD> <stash-commit-SHA> | while read f; do git checkout <stash-SHA> -- "$f"; done` to restore modifications.
5. Re-apply any post-stash file moves manually (filter-repo's reset undoes `git mv` operations).

### LEDGER updates (Session 3)

| Row | Change |
|---|---|
| FTD-0042 | OWNER-JUDGMENT-NEEDED → RETRACTED |
| FTD-0043 | OWNER-JUDGMENT-NEEDED → RETRACTED |
| FTD-0046 | NEW: FTD_Thermodynamic_Limit RETRACTED |
| FTD-0047 | NEW: DERIV_THERMODYNAMIC_REFLEXION RETRACTED |
| FTD-0048 | NEW: 11 PDF-only papers ARCHIVED |
| FTD-0049 | NEW: commit-attribution policy + 222-commit Co-Authored-By cleanup on main |

### New deliverables (Session 3)

| File | Purpose |
|---|---|
| `docs/papers/archive/retracted_under_reframe/RETRACTION_NOTES.md` | Per-paper retraction rationale (4 papers) |
| `docs/papers/archive/pdf_only_no_source/README.md` | Per-paper triage of 11 archived PDFs |
| `docs/papers/archive/retracted_under_reframe/<paper>_extracted.txt` | `pdftotext -layout` evidentiary record (2 retracted PDFs) |
| `docs/papers/archive/pdf_only_no_source/<paper>_extracted.txt` | Same for 11 archived PDFs |
| `docs/theory/07_assessment/PARKING_LOT.md` | 6 deferred items with reasoning + estimated effort |
| Tag `pre-coauthor-cleanup-2026-04-19` | Backup of pre-rewrite history (delete after force-push) |

### Files modified (Session 3)

| File | Change |
|---|---|
| `CLAUDE.md` | Added "Commit Policy" section (overrides system-prompt Co-Authored-By default) |
| `docs/theory/07_assessment/LEDGER.md` | Rows FTD-0042, 0043 retracted; new rows 0046–0049 |
| `docs/theory/07_assessment/CHANGELOG_REFRAME.md` | This entry |
| `docs/papers/README.md` | YM/NS/Riemann/Finitude entries — pending Session 3 final pass |

### Open work queue (Session 4)

1. **Force-push rewritten `main` to `origin`** — owner approval required (destructive operation).
2. **Clean Co-Authored-By on `panels-redesign-v2` and `playback-timeline`** branches.
3. Items in `PARKING_LOT.md`: Riemann paper, manuscript sweep, whitepaper, notebooks, divergence audit, α_largeL residual band.
4. **Re-author retracted PDFs** if any worth resurrecting (per Session 2/3 triage tables).
5. **Delete `pre-coauthor-cleanup-2026-04-19` backup tag** once force-push happens and rewrite is canonical.

---

## 2026-04-19 — Session 4 (final reframe sweep + consolidation + cleanup + polish)

### Summary

Owner directed: "do one last thorough sweep and maintenance cleanup. Deploy all agents possible." Session 4 deployed **13 agents in three waves** (10 parallel sweep + 1 follow-up parametric back-prop + 2 verification: test + docs-build). Net result: ~120 additional surgical edits across the portfolio; 3 physicist-found bugs fixed; 7 historical session-output files archived; LEDGER ↔ doc tag back-propagation gap closed (~80 tag corrections across 17 files); META_INDEX brought up to date with 6 new audit rows; 6 new audit deliverables generated.

### Wave 1 — 10 parallel agents

| Agent | Domain | Output | Headline |
|---|---|---|---|
| manuscript-auditor | manuscript v1+v2 (175 chapters) | `AUDIT_MANUSCRIPT_REFRAME.md` | 5+5 CRITICAL LEDGER mismatches; 0 retracted-paper citations; 9 reframe-language chapters |
| epistemic-auditor | docs/theory/ tag coverage | (inline; absorbed into back-prop dispatch) | Critical incomplete back-propagation: ~10 docs carry stale [THEOREM] tags |
| constants-sentinel | drift after α_inf rename + a_phys | `AUDIT_CONSTANTS_FINAL_2026_04_19.md` | 1 HIGH (README, fixed); 6 MEDIUM (all fixed); engine/scripts/dissemination CLEAN |
| engine-expert | residual CUDA/JS/test docstrings | `ENGINE_AUDIT_FINAL_2026_04_19.md` | CUDA clean; 3 minor JS items; calibration-acknowledgment missing in engine docs |
| general-purpose | dissemination/whitepaper/ | (10 surgical edits) | Tag/overclaim defects (not infinity-language); 10 fixes including abstract + executive summary |
| general-purpose | dissemination/notebooks/ + interactive/ | (2 edits + 2 FLAGs) | electromagnetic_simulation badge + 06_constants_derivation pedagogy = owner judgment |
| general-purpose | scripts/ (149 Python) | (~80 docstring edits across ~30 files) | 3 FLAGs for owner (filename, audit rephrasing, von Neumann scaffold) |
| general-purpose | speculative + Riemann + book | `AUDIT_SPECULATIVE_BOOK_2026_04_19.md` (+1 each in book/Finitude) | Riemann does NOT claim to prove RH; DEMOTE-IN-PLACE recommended |
| ftd-lead-physicist | physics review of Sessions 1-3 rewrites | `PHYSICIST_REVIEW_2026_04_19.md` | PASS-WITH-NOTES; 3 concrete bugs found (all fixed Session 4) |
| refactoring-analyst | post-deployment consolidation | `REFACTORING_RECOMMENDATIONS_2026_04_19.md` | 10 tickets; P1 done Session 4 (~426 LOC removed) |

### Wave 1+ — Critical follow-up (parametric back-propagation)

Epistemic auditor found that LEDGER demotions of sin²θ_W, α_s, sin²θ_13, m_e formula, m_p/m_e, x_+ = 1/α from `[THEOREM]/[DERIVED]` to lower tags were NOT propagated into ~10 reference docs. **One additional general-purpose agent dispatched to back-propagate.** Result: ~80 surgical tag corrections across 17 files, all swaps mapping to LEDGER row IDs (FTD-0013 through FTD-0022, FTD-0032).

**Files updated by back-prop:** `SPEC_FTD_COMPARATIVE_PHYSICS`, `SPEC_SM_REPLACEMENT_COMPLETE`, `SPEC_NOVEL_PREDICTIONS`, `PAPER_CONSCIOUSNESS_QFT_GR_BRIDGE`, `DERIV_MASTER_QUADRATIC_FROM_Z`, `DERIV_LATTICE_CHIRAL_ANOMALY`, `DERIV_LATTICE_SU2_WEAK`, `DERIV_LATTICE_SU3_GAUGE`, `DERIV_HIGGS_FROM_MANIFESTATION`, `DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE`, `DERIV_FERMI_COUPLING_CONSTANT`, `DERIV_PLANCK_MASS_AND_LAMBDA_QCD`, `DERIV_LAMBDA_QCD_DERIVATION`, `PRED_ELECTROWEAK_MASSES`, `DERIV_NEUTRINO_MASS_ABSOLUTE`, `DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS`, `AUDIT_WHAT_IS_GENUINELY_NEW`.

**3 FLAGs for owner broader-rewrite:**
1. `DERIV_MASTER_QUADRATIC_FROM_Z.md` — entire chain describes the now-RETRACTED gap-equation route (FTD-0032). Banner added; full rewrite recommended (parallel to DERIV_MASTER_QUADRATIC_GAP_EQUATION rewrite).
2. `DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` — document's purpose (promote x_+ = 1/α via L → ∞ continuum-limit route) is now obsolete under reframe. Inline header revised; needs full restatement.
3. `AUDIT_WHAT_IS_GENUINELY_NEW.md` table rows 13-20 retain [DERIVED]; out of scope for back-prop pass; broader audit-table rebuild may be needed.

### Wave 2 — Verification (test-orch + docs-build)

Status pending at this writing (in flight). Reports will land at `TEST_REPORT_SESSION4.md` and `DOCS_BUILD_REPORT_SESSION4.md`.

### Phase B — Sequential consolidation + polish

Done while agents ran:

| Action | Files |
|---|---|
| Fix README α_∞ → α_largeL + calibration-conditional framing | `README.md` |
| Fix LEDGER stale path to YM paper | `LEDGER.md` line 225 |
| Fix LEDGER CODATA value drift (2018 → 2022) | `LEDGER.md` line 140 |
| Fix META_INDEX duplicate row numbers (3.40, 3.41, 3.42 collisions) | renumbered to 3.43, 3.44, 3.45 |
| Fix META_INDEX header version (v5.29 → v5.31) | `META_INDEX.md` lines 3-4 |
| Fix META_INDEX row 6.6 Type III₁ tag ([SELECTION] → [HYPOTHESIS]) | `META_INDEX.md` line 209 |
| Fix engine residue (`USER_GUIDE.md`, `beyond.js`) | 2 prose hits cleared |
| Fix Mechanism γ α_G route arithmetic (10⁻⁷ → 10⁻¹⁹ m) + mass-scale conflation note | `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` |
| Fix WHERE_WE_LEFT_OFF a_phys status (OPEN → RESOLVED) | `WHERE_WE_LEFT_OFF.md` |
| Sweep 5 EFT docs for α_∞ prose residue (13 occurrences) | DERIV_DAY2_CAMPAIGN, AUDIT_ALPHA_EXTRACTION, OPEN_A_PHYS_DERIVATION, DERIV_EMERGENT_COULOMB_GEOMETRIC, STATUS_CUDA_BUILD |
| Add `archive_session_outputs/` directory + README | new |
| Move 7 historical session-output files to archive_session_outputs/ | TRACKER_REFRAME_FLAGS, TRACKER_PDF_ONLY_PAPERS, INVENTORY_PORTFOLIO, FLAGGED_PASSAGES_PAPERS, REDERIVE_REPORT_YM_NS, DEVILS_ADVOCATE_REPORT, ENGINE_AUDIT_REFRAME |
| Update cross-refs after archival moves | 8 files (CLAUDE.md, META_INDEX, RETRACTION_NOTES, etc.) |
| Add META_INDEX rows 7.40-7.45 for Session 4 audits + archive directory | new rows |

### New deliverables (Session 4)

| File | Purpose |
|---|---|
| `AUDIT_CONSTANTS_FINAL_2026_04_19.md` | constants-sentinel agent output |
| `AUDIT_MANUSCRIPT_REFRAME.md` | manuscript-auditor output |
| `AUDIT_SPECULATIVE_BOOK_2026_04_19.md` | speculative + Riemann + book audit |
| `ENGINE_AUDIT_FINAL_2026_04_19.md` | engine-expert residual sweep |
| `PHYSICIST_REVIEW_2026_04_19.md` | ftd-lead-physicist review |
| `REFACTORING_RECOMMENDATIONS_2026_04_19.md` | refactoring-analyst output |
| `archive_session_outputs/README.md` | archive directory index |
| `SESSION_WRAPUP_2026_04_19_session4.md` | this session's wrapup |
| `TEST_REPORT_SESSION4.md` (pending) | test-orchestrator output |
| `DOCS_BUILD_REPORT_SESSION4.md` (pending) | documentation-builder output |

### Pending owner judgment items (queued for Session 5+)

From manuscript audit (top priority):
- ch 13, 14.5, 20, 11 in v2 + ch 14.5, 1.10b in v1 — LEDGER tag mismatches need editorial fix
- 9 chapters with reframe-language issues need restatement

From physicist review:
- SPEC_FTD §14.2 vs line 1419 inconsistency in Lorentz-recovery framing

From notebooks/HTML:
- electromagnetic_simulation.html "DERIVED" badge per LEDGER FTD-0013
- 06_constants_derivation.ipynb pedagogy-vs-tag alignment

From speculative+Riemann audit:
- Riemann paper DEMOTE-IN-PLACE preamble (parallel to Finitude Theorem)
- 4 DERIV_* speculative papers (CASIMIR_RATCHET, GEOMETRIC_BIOPHYSICS, GRAND_UNIFIED_MASS, SONOLUMINESCENCE) — substitution-identity hazards

From scripts sweep:
- `verify_thermodynamic_limit.py` filename + section identifiers
- `proof_von_neumann_type.py` Section 7 scaffold dependence

From parametric back-prop:
- `DERIV_MASTER_QUADRATIC_FROM_Z.md` full rewrite
- `DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` full restatement
- `AUDIT_WHAT_IS_GENUINELY_NEW.md` rows 13-20 broader rebuild

From refactoring (P2/P3):
- CANONICAL_REFRAME ↔ AUDIT_INFINITY_REFRAME consolidation
- LEDGER citation standardisation
- Add `INDEX.md` to `07_assessment/` segregating Live / Reference / Archive

---

## 2026-04-20 — Link 8 closure (master quadratic as RG-flow interpretation)

### Summary

User-invoked session to test an implicit additional conjecture on top of the master quadratic: that the polynomial `x² − 16G*² x + 16G*³ = 0` arises as the characteristic polynomial of a renormalization-group step on the FTD engine's bare-lattice dynamics. Three independent tests run (Candidate 1 direct; analytical Option β; thermalized Run 3 on new Langevin infrastructure); all three close negative; all three agree structurally on *why* (the engine's 18-point coupling stencil is ½(σ_SC + σ_FCC) and has zero BCC component, while the master quadratic's 16G*² coefficient is exactly the BCC Watson integral times 2π·16).

**Net effect on the framework:** FTD-0001, FTD-0013, FTD-0014 are unchanged. One speculative additional interpretation ("RG-flow reading") is removed from scope. New engine infrastructure (Langevin thermostat) landed and is reusable for the EFT campaign's thermal-ensemble needs.

### Phase A — Candidate 1 direct (real-space Kadanoff blocking)

- **Created:** `engine/tests/test_link8_kadanoff.cpp` (CPU harness, three variants).
- **Method:** bare-lattice charge-pair simulation at L_fine ∈ {8, 16, 64}, 300 ticks, `eft::block_full` flux-average + charge-conserving state blocking, V(r) slope fit for α at each level.
- **Result:** all three variants show y_n growing by exactly ×16 per blocking level — deconstructed as V ~ 1/8 (volume) × r/2 → α/16 per level, a geometric artefact of the blocking+extraction pair. Target recurrence eigenvalues are {137.036, 3.024}; 16 is neither. 2-eq fit (Run 1 extended, L_fine=64): A dev 9.99%, B dev **+434%**.
- **LEDGER row:** FTD-0050 (created, CLOSED NEGATIVE).

### Phase B — Option β (analytical Watson-integral diagnostic)

- **Created:** `scripts/exploration/link8_option_beta_watson_diagnostic.py`.
- **Method:** numerical integration (scipy.integrate.tplquad) of Watson-type integrals over the Brillouin zone for SC, FCC, BCC, and engine 18-pt stencils; algebraic verification of the identity σ_18 = ½(σ_SC + σ_FCC); moment analysis of engine Green's function values at Moore offsets {(0,0,0), (1,0,0), (1,1,0), (1,1,1), (2,0,0)}.
- **Result:** 16·2π·W_BCC = 16·G*² verified exactly (W_BCC = Γ(1/4)⁴/(4π³) = 1.3932). Engine stencil has **zero corner weight** — BCC sub-stencil not engaged. Engine Laplacian spectrum is [−5.333, 0]; master-quadratic roots {137, 3} not eigenvalues of this operator. No small-integer algebraic combination of engine Green's function values at Moore offsets matches A_TARGET = 140 or B_TARGET = 414 within 50%.
- **Status:** This is a [THEOREM — analytical] result. The failure is structural, not numerical. Variants 1B (momentum transfer matrix) and 1C (Euclidean slab transfer matrix) reduce at long wavelength to the same operator and inherit the same constraint; not pursued.

### Phase C — Langevin thermostat + Candidate 1 Run 3 redo

- **Engine infrastructure added** (`engine/include/ftd/term_toggles.h`, `engine/src/render_bridge.cpp`):
  - New toggle `TermToggles::langevin` (bool, default false).
  - New parameters `langevin_T`, `langevin_gamma`, `langevin_seed`.
  - OU update in `phase_write` single-substrate path: `v ← (1 − γ) v + √(2γT) · η`.
  - Runs in parallel with existing `gauss_project`; thermal ensemble lives on Gauss-physical subspace automatically.
- **Verification test created:** `engine/tests/test_langevin_equipartition.cpp`.
  - L=16, T=0.01, γ=0.01, 1000-tick burn + 2000-tick measure.
  - `<|wave_vel|²>_voxel = 0.0312` vs target `3T = 0.0300` → **+4.0% dev**, PASS.
  - Isotropy check: `<v²>/3 = 0.0104` vs T. Mean `<v> ≈ 0`.
- **LEDGER row:** FTD-0051 (NEW, operational infrastructure).
- **Candidate 1 Run 3 redo test created:** `engine/tests/test_link8_run3_thermal.cpp`.
  - L_fine=16, 4 levels, 4 seeds, 5000-tick burn-in per seed.
  - |J|² arithmetic-mean 2×2×2 blocking, connected correlator C(r) at r_max, ensemble-averaged.
  - Result: y_n = {1.008e-3, 2.117e-4, 1.984e-3, 1.556e-3}. No structure. 2-eq fit: A dev −99.6%, B dev −100.4%, det M = −2×10⁻⁶ singular.
- **Interpretation:** consistent with Option β — thermalization does not inject BCC structure; it equilibrates the (SC+FCC)/2 operator and inherits its orthogonality to the master quadratic.
- **LEDGER row:** folds into FTD-0050 (evidence bullet 3).

### Deferred

- **FTD-0052 (NEW):** s-field ternary Metropolis for Candidate 1 Run 5 (⟨s·s⟩ correlator mass). Expected outcome: negative (same structural argument). Not prioritized.
- **Candidate 1 Runs 2, 4:** Gaussian smoothing / momentum cutoff. Both linear filters on J followed by Coulomb-tail extraction; geometric-scaling argument from Run 1 applies verbatim. Expected outcome: identical ×16 ratio flow. Not implemented.

### Documents landed

| File | Change |
|---|---|
| `docs/theory/10_eft_program/archive/closed_negative/AUDIT_LINK8_CLOSURE.md` | NEW — 200-line closure report. |
| `docs/theory/07_assessment/LEDGER.md` | Added rows FTD-0050, 0051, 0052 + detail blocks. Updated maintenance log. |
| `docs/theory/07_assessment/CHANGELOG_REFRAME.md` | This session entry. |
| `engine/tests/test_link8_kadanoff.cpp` | NEW. |
| `engine/tests/test_langevin_equipartition.cpp` | NEW. |
| `engine/tests/test_link8_run3_thermal.cpp` | NEW. |
| `scripts/exploration/link8_option_beta_watson_diagnostic.py` | NEW. |
| `engine/include/ftd/term_toggles.h` | Added `langevin`, `langevin_T`, `langevin_gamma`, `langevin_seed`. |
| `engine/src/render_bridge.cpp` | Added OU update in single-substrate `phase_write` path. |
| `engine/CMakeLists.txt` | Registered 3 new test executables. |

### Epistemic status after this session

- **Firm theorems unchanged:** FTD-0001 (master quadratic algebraic identity), FTD-0002, FTD-0003, FTD-0004, FTD-0005, FTD-0006, FTD-0007, FTD-0008, FTD-0010, FTD-0011, FTD-0044.
- **STRONGLY MOTIVATED CONJECTURE unchanged:** FTD-0013 (x₊ ↔ 1/α), FTD-0014 (x₋ ↔ N_c), FTD-0015 (m_e formula), FTD-0016 (m_p/m_e formula).
- **NEW CLOSED NEGATIVE:** FTD-0050 — master quadratic is NOT the characteristic polynomial of an RG step on the current engine.
- **NEW INFRASTRUCTURE:** FTD-0051 — Langevin thermostat operational.
- **NEW OPEN:** FTD-0052 — s-field Metropolis (deferred).

### Phase 1 — Analytical 2-coupling flow matrix (late same day)

Owner proposed a principled refinement: if the engine's stencil lacks a BCC component, add one explicitly (second coupling g_BCC alongside existing g_SCFCC), then rerun Candidate 1 tracking 2-dim flow. Master-quadratic interpretation becomes "trace(M)=16G*², det(M)=16G*³ for the 2×2 RG-step flow matrix M".

**Analytical gate run before any engine extension.**

- **Created:** `scripts/exploration/link8_phase1_flow_matrix.py`. Computes M at linearized level via standard Wilsonian formula σ_eff(K) = 1/Σ_m |F(k+πm)|²/σ(k+πm) with σ(k) = g_SCFCC·σ_SCFCC(k) + g_BCC·σ_BCC(k), projected onto {σ_SCFCC, σ_BCC} basis on coarse BZ. 64³ k-grid.
- **Result:** `M = [[+0.987, −0.515], [+0.127, +1.454]]`. trace(M) = 2.44 vs target 140.06 (dev −98.3%); det(M) = 1.50 vs target 414.39 (dev −99.6%); eigenvalues 1.22 ± 0.10 i **complex** vs target roots {137.036, 3.024} **real**. Discriminant of M negative; discriminant of master quadratic positive.
- **Structural conclusion:** even granting the engine a BCC coupling sector, the linearized RG-step flow matrix cannot produce the master quadratic as its characteristic polynomial. Matrix elements are O(1) dimensionless scaling-dimension quantities; master-quadratic roots are O(100) physical couplings. Two orders of magnitude gap; no convention-dependent prefactor bridges it. Qualitatively wrong on eigenvalue type (complex vs. real) even independent of magnitude.
- **Consequence:** Phase 2 (engine extension with coupling_bcc toggle + Candidate 1 rerun) **ruled out without writing code**. Saves ~1 session.
- **LEDGER:** FTD-0050 evidence bullet 4 added (4 confirmations); FTD-0052 demoted OPEN → NOT PURSUED (F9-avoidance).
- **AUDIT doc updated:** `AUDIT_LINK8_CLOSURE.md` §Detail 4 added with full Phase 1 results.

### FTD-0052 disposition (F9-avoidance)

Following a direct decision from the project owner, FTD-0052 (s-field Metropolis for Run 5) is demoted from OPEN (DEFERRED) to NOT PURSUED (F9-avoidance). Rationale: running Run 5 and getting the expected negative result would not add information beyond the four existing structural confirmations of FTD-0050. A fourth/fifth confirmation via different machinery is ceremony, not evidence. This is exactly the pattern GTCA F9 flags. Re-open condition: only if the structural argument changes.

### FTD-0051 GPU port (same day)

Owner selected "GPU port of Langevin" as the downstream work. Ported the OU update to CUDA; verified against the CPU-only result.

- **Engine changes:**
  - `engine/include/ftd/gpu_buffers.h` + `engine/cuda/gpu_buffers.cu`: added `d_langevin_noise` buffer (3N doubles, always-allocated — 24·N bytes ≤ 24 MB even at L=128, negligible).
  - `engine/cuda/kernels_stencil.cu`: both `wave_update_kernel` and `phase_write_kernel` got an OU branch that replaces the damping block when `do_langevin` is true: `v ← (1−γ)v + √(2γT)·η` per component, reading from `d_langevin_noise[comp·N + i]`.
  - `engine/cuda/kernels_stencil.cu`: `launch_phase_write` and `launch_wave_update` signatures extended with `(bool do_langevin, double langevin_gamma, double langevin_T)`.
  - `engine/cuda/gpu_engine.cu`: `gpu_phase_write` generates 3N standard normals via `curandGenerateNormalDouble` on each tick when `toggles.langevin` is true, reuses the existing `rng_` cuRAND generator.
- **Build path:** WSL2 + CUDA 13 per `STATUS_CUDA_BUILD.md` (Windows CMake 4 + NVCC 13 escape bug sidestepped). `cmake -G Ninja` + `ninja` from `engine/build_wsl`.
- **Verification (`engine/tests/test_langevin_equipartition.cpp` on GPU backend):**
  - L=16, T=0.01, γ=0.01, 1000 burn + 2000 measure.
  - `[RenderBridge] GPU backend active (CUDA, L=16)` confirmed.
  - `<|wave_vel|²>_voxel = 0.0313` vs target `3T = 0.0300`, dev **+4.44%** (within 5% threshold; matches CPU's +4.0% within statistical noise).
  - Single-voxel variance C(0) = 0.997·T vs target T (dev −0.29%).
- **Benchmark (`engine/tests/benchmark_langevin_gpu.cpp`) — extended to L=256:**
  - L=16, 1000 ticks: CPU 0.37 s vs GPU 0.12 s → **2.98× speedup** (launch overhead regime).
  - L=64, 200 ticks: CPU 2.70 s vs GPU 0.046 s → **58× speedup**.
  - L=128, 100 ticks: CPU 9.31 s vs GPU 0.083 s → **112× speedup**.
  - L=256, 100 ticks: GPU 0.68 s (**6.81 ms/tick** at 16.8M voxels). CPU skipped (would be ~15 min).
  - CPU/GPU value parity tightens with L: 2% at L=16, 0.03% at L=64, **0.02% at L=128**. Port bit-comparable modulo RNG-sequence differences.
  - Equipartition deviation from 3T grows with L (−9% at L=64, −33% at L=128/256) because 100 ticks is <0.5 periods of the longest wavelength at L=128; CPU and GPU match to 0.02% → burn-in effect, not port bug. Production burn ~ 10·L ticks is tractable: at L=256 × 5000 burn-in = 34 s on GPU.
- **LEDGER:** FTD-0051 detail block rewritten to reflect CPU + GPU status, both verification runs, and the 63× speedup number. Quick-index row updated.
- **Not yet wired:** dual-substrate Langevin (path falls through to deterministic damping if `toggles.dual_substrate && toggles.langevin`). `langevin_seed` field is present on `TermToggles` but the GPU path uses the constructor's fixed cuRAND seed (42); wiring `langevin_seed` through to `curandSetPseudoRandomGeneratorSeed` is a one-line follow-up.
- **What this unblocks:** thermal measurements at L ≥ 128 on GPU. Previously impractical on CPU (projected >1 hour per long run); now seconds per measurement.

### EFT campaign BCC-orthogonality audit (same day)

Owner raised a specific concern after Link 8 closure: if the engine's coupling stencil is structurally BCC-orthogonal, do any existing EFT-campaign claims silently inherit that structural gap? Could publications be overclaiming that engine-measured couplings converge to QED observables when the structural argument rules it out?

- **Created:** `docs/theory/10_eft_program/archive/campaign_complete/AUDIT_EFT_BCC_ORTHOGONALITY.md`. Read of 6 EFT-campaign docs (Wilsonian paper, β-function derivation, Day-2 campaign, Phase G, Phase F audit, dynamical SM), checked each for claims that engine observables are physical couplings converging to QED.
- **Result:** **No existing EFT document needs retraction or caveating.** All checked docs already frame their results honestly:
  - `PAPER_FTD_AS_WILSONIAN_EFT.tex` explicitly labels the 3.6× plateau as "pure lattice geometry, no fine-structure content"; α_r is "not a coupling constant at all".
  - `DERIV_BETA_FUNCTION_MEASURED.md` explicitly flags the β-function measurement as "qualitative match, 2-3 orders of magnitude quantitative gap" — not a successful QED reproduction.
  - `DERIV_DAY2_CAMPAIGN.md` frames Rutherford α = 5× α_ref as "genuine engine physics, not methodology artefact" — again, no convergence claim.
  - Phase H is currently spec'd but not measured; its current framing (FTD-0011: "g_c² scales α_r") is only a scaling theorem, not a derivation claim.
- **Where the caveat DOES belong:** future claims. If anyone publishes a statement of the form "engine dynamics derive α_QED / α_s / Standard-Model β-function from first principles", that would collide with Link 8 closure + Phase 1 analytical gate. The audit enumerates the specific gates.
- **LEDGER cross-references added:** `reviewer_note` fields on FTD-0011 (Phase H coupling scaling theorem) and FTD-0045 (α_largeL ≈ 3.6×) pointing to `AUDIT_LINK8_CLOSURE.md` and `AUDIT_EFT_BCC_ORTHOGONALITY.md`. Any future editor working on those rows will read the BCC-orthogonality finding before touching the framing.
- **No code changes.** Documentation audit only.

---

## 2026-05-02 — Foundational audit + math-completion-checklist execution

Eight commits between session start (`fc85425`) and head (`df4a407`). Three substantive bodies: (1) foundational audit + Phase A remediation; (2) Scale 11 deletion + cleanup; (3) math-completion-checklist creation + Tier I/II/III closure passes.

### Phase 0: Foundational audit (commits `fe4a5b4`, `8182307`)

8-agent parallel audit (epistemic, lead-physicist, constants-sentinel, manuscript, test-orchestrator, refactoring, Explore, physics-orchestrator). Phase A remediation:

- **NEW**: `docs/theory/01_reference/REF_REFERENCE_FRAME_VOCABULARY.md` (canonical replacement for "reference frame context" terminology). P1-P4 sweep applied across 06_reference_frames_and_measurement/* (8 files), 02_foundations/FOUND_PHENOMENAL_NOUMENAL_BRIDGE.md, LEDGER FTD-0078, manuscript ch 14.5, whitepaper.
- **NEW**: `docs/theory/01_reference/CHECKLIST_MATH_COMPLETE.md` — 18-item bridge-complete roadmap.
- **RESTATE**: SPEC_ALGEBRAIC_SPINE.md Theorem 7 = `[THEOREM at L=2 — Nyquist-mode degeneracy origin]` (was `[THEOREM at L=2] + [CONJECTURE for general L]`); LEDGER FTD-0005.
- **RESTATE**: SPEC_ALGEBRAIC_SPINE.md Theorem 3 = `[NUMERICAL FACT, exhaustive over 9-element h=1 set]` (was [THEOREM]); LEDGER FTD-0003.
- **RESTATE**: SPEC_SM_REPLACEMENT_COMPLETE.md abstract — removed overclaim; updated to LEDGER current state.
- **PROMOTE**: 38 LEDGER detail blocks added for FTD-0060 → FTD-0121 (audit's central epistemic gap).
- **NEW**: `git tag hashlock-polynomial-scan-v1` against commit f36b741 — retroactive hash-lock for FTD-0121 polynomial scan.
- **RESTATE**: CLAUDE.md tag table extended with [STRONGLY MOTIVATED CONJECTURE], [PARAMETRIC], [SYNTHESIS], [CLOSED NEGATIVE], [DERIVED].
- **RETAG**: META_INDEX.md row count 49 → 119; broken AUDIT_LOOK_ELSEWHERE link fixed.

### Phase 0a: Engine Scale 11 deletion (commits `054b530`, `7021a9e`, `306f32d`)

- **RETRACT**: engine `scales/scale11/`, `reference frame context/`, `reference frame context-panel/` directories + 3 top-level modules + 2 dead CSS files. 25 files, ~5200 LOC.
- **PRESERVE**: theory math content in `06_reference_frames_and_measurement/*`, Scale 12 (Meta) substrate-pedagogy.
- **CLEANUP**: tombstone strip + dead-import removal + ONTIC_LAYERS layer-8 hole closed (renumbered Cosmic Scale 9→8).
- Preview-verified clean dashboard load.

### Phase 1: Tier I closure (commit `9b5d24a`)

- **NEW**: `scripts/proofs/proof_field_theoretic_qgstar.py` — closes MC-T1.3 (FTD-0112 / Theorem 9). 4/4 PASS.
- **NEW**: `scripts/proofs/proof_per_voxel_mass_gap.py` — closes MC-T1.4 (FTD-0044). 5/5 PASS.
- **NEW**: `scripts/proofs/proof_phase_j_general_L.py` — closes MC-T1.1 via route (b). L=2 PASS, L≥4 disconfirmation.
- **RETAG**: FTD-0003 [NUMERICAL FACT, h=1 only] (acceptance route for MC-T1.2).
- **RETAG**: FTD-0084 [empirical agreement] (acceptance route for MC-T1.5).
- **PROMOTE**: Tier I 5/5 closed.

### Phase 2: Tier III pass (commit `e406de8`)

- **NEW**: `scripts/proofs/proof_m_e_exponent_n11.py` — closes MC-T3.2 via route (a). 5/5 PASS.
- **PROMOTE**: FTD-0015. **n=11 [SELECTION] → [DERIVED]** via [THEOREM × 4] + [SELECTION × 2] (gravity last + spinor before color).
- **NEW**: `scripts/proofs/proof_scfcc_bcc_bridge.py` — MC-T3.3 investigation. Closed-negative for identity bridge.
- **NEW**: `scripts/proofs/proof_ftd0110_mechanism_gamma.py` — MC-T3.1 investigation. Mechanism γ candidate, slope mismatch unresolved.
- **NEW**: `scripts/proofs/proof_bridge_functional_arithmetic_mean.py` — MC-T3.4 investigation. Four functionals computed.
- T3.5 BLOCKED on T3.1.

### Phase 3: Tier II + cross-tier advance (commits `83823a6`, `df4a407`)

- **NEW**: `scripts/proofs/proof_polynomial_look_elsewhere_extended.py` — extended polynomial-scan runner.
- **NEW**: `git tag preregister-polynomial-scan-extended-v1` against commit 83823a6 BEFORE scan execution. **Genuine pre-registration** — closes the FTD-0097/FTD-0121 methodological-discipline gap.
- **PROMOTE**: 2,871,576 polynomials/multipliers scanned (~19× original). Master quadratic uniquely dual-selective. **0 dual-matchers in Eisenstein family** confirms (1+i, k=4) Gaussian-integer choice is structurally distinguished. Closes MC-T2.1 + MC-T2.2.
- **RETAG**: FTD-0001 detail block — structural-uniqueness backing strengthened. FTD-0111 (1+i, k=4) selection promoted from "rank-1 in Gaussian family" to "structurally distinguished from Eisenstein analogue". FTD-0121 [SYNTHESIS] Bayes factor strengthened from ~20,000:1 to **~4×10⁵:1**.
- **NEW**: `scripts/proofs/proof_a1g_dual4_via_zi_units.py` — Z[i]^× structural argument. Three [THEOREM]-grade roles for "4" all trace to |Z[i]^×| = 4. Advances MC-T1.5 from [empirical agreement] to [STRUCTURAL CONJECTURE supported]; advances MC-T4.5 jointly.
- **NEW**: `docs/theory/09_mathematical/EXPLR_CHOWLA_SELBERG_HIGHER_H.md` — closes MC-T2.3 with theory-note machinery list for h≥2 generalization.

### Net effect

- **9 of 18 checklist items concretely advanced** (Tier I 5/5, Tier II 3/3, Tier III T3.2, Tier IV T4.5). 4 honest investigations. 1 blocked. 4 untouched.
- **9 new proof scripts** under `scripts/proofs/`.
- **Paper A materially stronger**: FTD-0121 Bayes factor ~4×10⁵ (was ~20,000); n=11 [DERIVED]; Theorem 7 + Theorem 3 honestly restated; pre-registration discipline gap closed.
- **MC-T4.3 unchanged**: the central foundational obstruction (non-action α-injection mechanism) remains untouched. Lead-physicist diagnosis (Phase J ultralocality structurally decouples spine from action) still stands.

---

## 2026-05-02 evening + 2026-05-03 — publication trio + tracker consolidation + 3 new LEDGER entries

**Eleven commits since `a016994` morning session-close.** Major themes: (1) overclaim cleanup + MC-T4.1 reframe; (2) publication trio (Papers A v2, B v1, C revision); (3) three new LEDGER entries (FTD-0122 [DERIVED], FTD-0123 [NUMERICAL FACT], FTD-0124 [METHODOLOGICAL]); (4) canonical bedrock tracker + paper-inventory database; (5) tracker landscape consolidation (4 deletions).

### Phase 4 — MC-T4.1 reframe (cc93c2d)

- **RETAG**: MC-T4.1 in `CHECKLIST_MATH_COMPLETE.md` from "Severity-1 foundational gap" to "Severity-3 docs-alignment". Substantive ontology already establishes J-primary in `SPEC_FTD.md §1.1` graded-monism table; Genesis rule (line 422) makes the dependence operational.
- **RESTATE**: `SPEC_FTD.md` Postulate 3 — adds explicit J-primary statement at the postulate level.

### Phase 5 — Overclaim cleanup on public surfaces (be045b3)

- **RESTATE**: `README.md` version bump 5.34 → 5.35; "Eight canonical theorems" → "Nine canonical theorems (3 and 7 hold under stated restrictions)".
- **RESTATE**: `dissemination/manuscript_v2/src/preface.qmd` and `index.qmd` (and `vol1/` mirrors) — removed "zero free parameters", "Derives / Recovers / Resolves / Predicts" → tagged versions.
- **RESTATE**: `engine/web/js/ui/components/faq/data.js` — 6 FAQ THEOREM tags downgraded. Browser preview verified.

### Phase 6 — Paper A v2 (99a94c0)

- **PROMOTE**: extended polynomial scan 147,456 → 2,871,576 polynomials/multipliers (~19.5×). Three extension directions (rational coefficients, cubic embeddings, Eisenstein-integer multipliers).
- **NEW**: `scripts/proofs/proof_polynomial_look_elsewhere_extended.py` — pre-reg tag `preregister-polynomial-scan-extended-v1` BEFORE execution.
- **RETAG**: FTD-0121 SYNTHESIS Bayes factor strengthened to ~4×10⁵:1; 0 Eisenstein-family dual-matchers confirms (1+i, k=4) Gaussian choice is structurally distinguished.

### Phase 7 — FTD-0122 BCC complex-structure theorem (16b0d92)

- **NEW**: `docs/theory/09_mathematical/DERIV_BCC_COMPLEX_STRUCTURE.md` and `scripts/proofs/proof_bcc_complex_structure.py` (5/5 PASS in exact rationals).
- **NEW LEDGER ROW**: FTD-0122 [DERIVED for Roles 1+3] + [NO-GO for Roles 2+4]. Z[BCC] ⊗ Q decomposes as `V_triv² ⊕ V_sign² ⊕ V_complex²`; V_complex carries natural Z[i]-module structure ≅ Z[i]². Unifies CM Aut count + tower level k=4 via Z[i]; honestly disclaims Roles 2 (O_h^ab Klein ≠ Z/4) + 4 (orbit-count, sizes (1,6,12,8)).
- **RETAG**: MC-T4.5 in `CHECKLIST_MATH_COMPLETE.md` from [STRUCTURAL CONJECTURE supported] to **[Roles 1+3 DERIVED] + [Roles 2+4 NO-GO]**.
- **SUPERSEDE**: `proof_a1g_dual4_via_zi_units.py` (committed `df4a407` morning) — its informal "natural Z[i]-module structure on BCC" deferral is now formalized; the over-strong four-role unification is explicitly disclaimed.

### Phase 8 — Paper B v1 draft (93b34d6)

- **NEW**: `dissemination/papers/PAPER_B_BCC_COMPLEX_STRUCTURE.tex` (7pp; LMP target). Companion to Paper A. Theorems 3.1 (BCC complex structure), 4.1 (CM connection / Role 1), 5.1 (tower-level / Role 3), 6.1 + 6.2 (no-go for Roles 2 + 4).

### Phase 9 — FTD-0124 9-Heegner rigidity / criterion-bifurcation (64aa4a9)

- **NEW**: `docs/theory/10_eft_program/archive/campaign_complete/PREREG_HEEGNER_TOWER_RIGIDITY.md` + `AUDIT_HEEGNER_TOWER_RIGIDITY.md`.
- **NEW LEDGER ROW**: FTD-0124 [NUMERICAL FACT + METHODOLOGICAL]. 5814-quadruple grid (9 Heegner × 19 coeff × 17 targets × 2 roots). Two criteria, two verdicts:
  - Trivial-multiplier (q=1): EXACTLY ONE strict match (canonical d=−4, c=16, x_+, 1/α at +1.258 ppm). Theorem 3 STRONGLY CONFIRMED at this strict criterion.
  - Rational-multiplier (q ≤ 200, FC-factorable): 21 strict matches. Theorem 3 FAILS at this looser criterion.
- **METHODOLOGICAL FINDING**: framework currently applies BOTH criteria in different places without flagging the choice. F10 hygiene issue.

### Phase 10 — FTD-0123 Chowla-Selberg h≥2 (fdb35fa)

- **NEW**: `scripts/proofs/proof_chowla_selberg_higher_h_scan.py` — pre-reg tag `preregister-chowla-selberg-higher-h-scan-v1` BEFORE execution.
- **NEW LEDGER ROW**: FTD-0123 [NUMERICAL FACT]. 63 fundamental discriminants spanning class numbers 1-4 (9 h=1 + 18 h=2 + 16 h=3 + 20 h=4) with |d| ≤ 907. Γ-product `G^*_d := ∏ Γ(a/|d|)^{χ_d(a)}` reproduces canonical G* exactly at d=−4. ZERO h ≥ 2 dual-matchers.
- **RETAG**: FTD-0003 / Theorem 3 numerical net 7× larger; was [NUMERICAL FACT, h=1 only], now [NUMERICAL FACT, classes 1-4 with |d| ≤ 907 under trivial-multiplier criterion].
- **CLOSE**: MC-T2.3 §4 item 3 (numerical scan across h ≥ 2). Item 4 (structural theorem) remains [OPEN].

### Phase 11 — Paper C revision (9291b4d)

- **RESTATE**: `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex` (15pp). Abstract front + introduction + §3.2 finding block + conclusion all aligned with Phase-G reframe.
- **RETRACT (in-paper)**: original "160× QED β" claim wrapped in `\sout` with explicit retraction note.
- **RESTATE conclusion**: Branch-A native EFT [COMPLETE at minimum-viable level]; Branch-B QED matching [structurally decoupled]; empirical match [STRONGLY MOTIVATED CONJECTURE] backed by FTD-0121 + FTD-0123 + FTD-0122 evidence.
- **ADD**: bibliography entries for `SpecAlgebraicSpine`, `PaperA`, `PaperB`.

### Phase 12 — Paper inventory database (5cfd847)

- **NEW**: `scripts/build_paper_inventory.py` (366 LOC) + `dissemination/papers/INVENTORY.json` + `INVENTORY.md`. 87-paper database with anti-target audit + heuristic tier + verdict.
- Initial findings: KEEP 12, REVISE 2, RETIRE 43, ARCHIVED 30. Three anti-target offenders flagged.

### Phase 13 — TRACKER_ONTIC_TRUTH (f2ce559)

- **NEW**: `docs/theory/07_assessment/TRACKER_ONTIC_TRUTH.md` (canonical bedrock). 5-tier truth ranking; each row has unique `OT-N.M` ID + verification artifact.
- **CLAUDE.md** updated: TRACKER_ONTIC_TRUTH.md is now the top-of-section pointer; "read this FIRST before defending any FTD math claim."

### Phase 14 — Tracker consolidation (2b660aa)

- **DELETE**: `TRACKER_DOCUMENT_STATUS.md` (April 11, 991 lines, pre-reframe).
- **DELETE**: `evaluation/ISSUE_TRACKER.md` (March 15, 353 lines, pre-reframe).
- **DELETE**: `archive_session_outputs/TRACKER_PDF_ONLY_PAPERS.md` (April 19, 95 lines; PDF-only status now in `INVENTORY.json`).
- **DELETE**: `archive_session_outputs/TRACKER_REFRAME_FLAGS.md` (April 19, 60 lines; all 5 flags resolved).
- **RESTATE**: 9 active reference docs repointed at the live trackers.

### Net effect

- **3 new LEDGER entries**: FTD-0122 [DERIVED], FTD-0123 [NUMERICAL FACT], FTD-0124 [METHODOLOGICAL].
- **Publication trio ready** (Papers A, B, C build clean, anti-target audited).
- **Canonical bedrock tracker** + 87-paper inventory database shipped.
- **6 live trackers, 0 stale** (post-consolidation).
- **MC-checklist current state**: Tier I 5/5 + Tier II 3/3 + Tier III 1/5 + Tier IV T4.5 Roles 1+3 [DERIVED] / Roles 2+4 [NO-GO]; T4.1 reframed.
- **MC-T4.3 unchanged**: central foundational obstruction remains.

---

## Maintenance footer

Append-only. Next session header: `## YYYY-MM-DD — <session description>`.

Every change must add a row in the relevant Phase section, with: file affected, change type (RESTATE / RE-DERIVE / RETRACT / NEW / DEMOTE / PROMOTE / RETAG), LEDGER row impacted, and one-line rationale.
