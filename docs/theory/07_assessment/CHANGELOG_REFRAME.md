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
| `06_consciousness/DERIV_VON_NEUMANN_CONSTRUCTION.md` | Type III₁ demoted SELECTION → HYPOTHESIS. Sections 5–6 restated as "Araki–Woods inductive-limit scaffold yields Type III₁ *if* applied"; Section 7 numerical-verification rows tagged [HYPOTHESIS] (Scaffold) for III₁ items; Section 8 epistemic accounting updated. | FTD-0033 |
| `docs/SPEC_FTD.md` | Postulate 1 restated to "undefined-boundary cubic lattice"; DEF.1 updated; finite-size-effects note updated. | FTD-0036 |

#### Mechanical sweep (5 parallel batches, ~127 edits across 38 files)

| Batch | Files | Edits | FLAGs |
|---|---|---|---|
| 1: 03_derivations gauge files | DERIV_LATTICE_QED_COMPLETE, DERIV_LATTICE_CHIRAL_ANOMALY, DERIV_COULOMB_SCATTERING_AMPLITUDE, DERIV_LATTICE_SU2_WEAK, DERIV_LATTICE_SU3_GAUGE, DERIV_KCOMP_VOLUMETRIC_SHELL | 26 | 0 |
| 2: 03_derivations remainder | DERIV_QM_FROM_LATTICE, DERIV_SINGLET_FROM_VOID_EVENT, DERIV_NC_FROM_TOPOLOGY, DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE, DERIV_EINSTEIN_NONLINEAR_FROM_LATTICE, DERIV_MASTER_QUADRATIC_FROM_Z, DERIV_THREE_RESOLUTIONS, DERIV_FORCE_EMERGENCE, DERIV_VARIATIONAL_PROOF, DERIV_QFT_GRT_BRIDGE, DERIV_SPIN_STATISTICS_BRIDGE | 48 | 0 |
| 3: foundations + bridges | FOUND_DIMENSIONAL_COUNTING, FOUND_RELATIVITY_GRAVITY_DISTINCTION, DERIV_GSTAR_PF_BRIDGE, DERIV_BCC_MULTIPLICATIVE_STRUCTURE, CONJ_ALPHA_FROM_CM, BRIDGE_QUADRATIC_PHYSICS | 11 | 1 (CONJ_ALPHA_FROM_CM Path A) |
| 4: 01_reference + papers | SPEC_FTD_COMPLETE_CHAIN, SPEC_FTD_REFERENCE, SPEC_FTD_LAGRANGIAN, SPEC_QFT_GRT_BRIDGE_ROADMAP, SPEC_FTD_COMPARATIVE_PHYSICS, PAPER_CONSCIOUSNESS_QFT_GR_BRIDGE, AUDIT_BELL_ANALYSIS | 23 | 0 |
| 5: consciousness + EFT + audits | DERIV_COLLAPSE_MECHANISM, AUDIT_ALPHA_EXTRACTION, SPEC_EFT_RECOVERY_PROGRAM, DERIV_DAY2_CAMPAIGN, DERIV_EMERGENT_COULOMB_GEOMETRIC, DERIV_DYNAMICAL_SM_EMERGENCE, DERIV_GAP_CLOSURE, DERIV_BETA_FUNCTION_MEASURED | 18 | 4 (EFT-program "engine→QED in L→∞" claims) |

**Total mechanical edits: 126.** **Total inline FLAGs: 5** (collected in `TRACKER_REFRAME_FLAGS.md`).

#### New documents created

| File | Purpose | LEDGER row |
|---|---|---|
| `10_eft_program/OPEN_A_PHYS_DERIVATION.md` | Frames the calibration question created by the reframe; analyses 3 derivation candidates. | FTD-0030 |
| `10_eft_program/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` | Explicit dimensional-chain attempt at γ; concludes Mechanism γ does not deliver derivation; recommends `a_phys ≡ ℓ_P` declaration. | FTD-0035 |
| `07_assessment/TRACKER_REFRAME_FLAGS.md` | Catalog of the 5 inline FLAGs with restatement guidance (Restatement A: scaling; Restatement B: calibration). | FTD-0034 |
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
| Orphan Type III₁ premise: "the flux field IS Type III$_1$" still asserted in 3 places | `06_consciousness/DERIV_COLLAPSE_MECHANISM.md` (lines 28, 320, 511) | Restated all three to "[HYPOTHESIS] under Araki–Woods scaffold"; cross-cited `DERIV_VON_NEUMANN_CONSTRUCTION.md`. |

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
| `docs/theory/07_assessment/TRACKER_PDF_ONLY_PAPERS.md` | 13 PDF-only papers triage + recovery options |
| `docs/theory/07_assessment/REDERIVE_REPORT_YM_NS.md` | YM + NS paper RE-DERIVE assessments |

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

## Maintenance footer

Append-only. Next session header: `## YYYY-MM-DD — <session description>`.

Every change must add a row in the relevant Phase section, with: file affected, change type (RESTATE / RE-DERIVE / RETRACT / NEW / DEMOTE / PROMOTE / RETAG), LEDGER row impacted, and one-line rationale.
