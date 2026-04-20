# Master Claim Ledger — FTD

**Status:** v1.0 (initial population, 2026-04-19).
**Maintenance rule:** this is the **single source of truth** for claim status. Tags in papers and theory docs are derived from here; if they disagree, this ledger wins. Never delete a row — retracted claims stay with `tag: RETRACTED`. Every tag change requires a `tag_history` entry. Dependencies/dependents must be symmetric.

**Format:** Markdown table for browsability + per-row YAML-style detail blocks below for the load-bearing rows. Format conforms to `reframe_deployment/templates/LEDGER_ENTRY_TEMPLATE.md`.

---

## Tag legend

| Tag | Meaning | Reviewer expectation |
|---|---|---|
| **THEOREM** | Formally proven from stated axioms; no completed-infinity steps in the proof; no free parameters | Check the proof |
| **SELECTION** | Argued from consistency / structural uniqueness; not uniquely proven | Critique the argument |
| **STRONGLY MOTIVATED CONJECTURE** | Empirical match + structural uniqueness; not derived | Demand validating evidence |
| **HYPOTHESIS** | Fits empirical data with known dofs; quantitative predictions stated | Test predictions |
| **CONJECTURE** | Well-motivated but not derivable from current axioms | Demand derivation or evidence |
| **PARAMETRIC** | FTD value inserted into a standard physics formula; not a derivation | Note as input, not output |
| **STRUCTURALLY MOTIVATED PARAMETRIC** | Parametric, but the structure (rational form, integer count) is constrained | Same |
| **AXIOM** | Foundational postulate; not derivable | Accept as model definition |
| **OPEN** | Unresolved load-bearing question | Research opportunity |
| **RETRACTED** | Withdrawn; kept in ledger for history | None |

---

## Quick index

| ID | Short name | Tag | Reframe status |
|---|---|---|---|
| FTD-0001 | Master Quadratic Polynomial + Roots | THEOREM | UNAFFECTED |
| FTD-0002 | G* algebraic identity (Watson–Chowla–Selberg) | THEOREM | UNAFFECTED |
| FTD-0003 | CM-curve uniqueness across class-number-1 fields | THEOREM | UNAFFECTED |
| FTD-0004 | Phase G emergent Coulomb at every finite L | THEOREM | UNAFFECTED |
| FTD-0005 | Phase J partition-function ultralocality at L=2 | THEOREM | UNAFFECTED |
| FTD-0006 | Coefficient 16 from \|Aut(E)\|² (Route A) | THEOREM | UNAFFECTED |
| FTD-0007 | Coefficient 16 from z_BCC × 2 (Route B) | THEOREM | UNAFFECTED |
| FTD-0008 | Moore neighbourhood integers {N_base=4, N_eff=13, b_3=7} | THEOREM | UNAFFECTED |
| FTD-0009 | Charge conservation per tick | THEOREM | RESOLVED (restated finitarily) |
| FTD-0010 | D = 3 from \|Aut(E)\|² = 2^D · (D−1)! | THEOREM | UNAFFECTED |
| FTD-0011 | Phase H coupling scaling (g_c² scales α_r) | THEOREM | UNAFFECTED |
| FTD-0012 | Discriminant trichotomy (bosons/critical/fermions) | THEOREM (algebra) / SELECTION (physical readings) | UNAFFECTED |
| FTD-0013 | x₊ ↔ 1/α (1.26 ppm) | STRONGLY MOTIVATED CONJECTURE | RESOLVED (downgraded from THEOREM) |
| FTD-0014 | x₋ ↔ N_c = 3 (0.80%) | STRONGLY MOTIVATED CONJECTURE | RESOLVED (downgraded from THEOREM) |
| FTD-0015 | m_e = m_P · √(2π) · (16/3) · α¹¹ (0.27%) | STRONGLY MOTIVATED CONJECTURE | UNAFFECTED |
| FTD-0016 | m_p/m_e = N_eff/α + N_base·N_eff + N_c (174 ppm) | STRONGLY MOTIVATED CONJECTURE | UNAFFECTED |
| FTD-0017 | Higgs mass m_H = (N_eff/α²)·m_e (0.24%) | STRUCTURALLY MOTIVATED PARAMETRIC | UNAFFECTED |
| FTD-0018 | sin²θ_W = 3/13 | PARAMETRIC | RESOLVED (downgraded 2026-04-19) |
| FTD-0019 | sin²θ_13 = 1/52 | PARAMETRIC | RESOLVED (downgraded 2026-04-19) |
| FTD-0020 | α_s = 7/59 | PARAMETRIC | RESOLVED (downgraded 2026-04-19) |
| FTD-0021 | PMNS angles (sin²θ_12, θ_23, Δm²) | STRUCTURALLY MOTIVATED PARAMETRIC | RESOLVED (downgraded 2026-04-19) |
| FTD-0022 | 7-term α series matching CODATA to 24 digits | CONJECTURE (post-hoc fit) | RESOLVED (downgraded; precision exceeds CODATA's ~11 digits) |
| FTD-0023 | Bell violation S = 2√2 | SELECTION | UNAFFECTED |
| FTD-0024 | Loop coefficients c1=9/47, c2=5/64, c3=4/141 | SELECTION (lattice Feynman) | UNAFFECTED |
| FTD-0025 | Confinement σ = 0.209 from area-law Wilson loops at x₋ | SELECTION | UNAFFECTED |
| FTD-0026 | Einstein equations from Deser bootstrap | SELECTION | UNAFFECTED |
| FTD-0027 | Cyclotomic Hamiltonian parameters (Φ_4, Φ_1·Φ_2, Φ_6) | SELECTION | UNAFFECTED |
| FTD-0028 | Moore Layer Theorem (gauge groups + 3 generations) | SELECTION (theorem-shaped argument; physical reading is selection) | UNAFFECTED |
| FTD-0029 | BCC multiplicative structure (W₃ + SU(3) from same eigenvalue) | SELECTION | UNAFFECTED |
| FTD-0030 | a_phys (lattice → physical length conversion) | RESOLVED-BY-CALIBRATION (a_phys ≡ ℓ_P declared in SPEC_FTD.md, 2026-04-19) | RESOLVED |
| FTD-0031 | g_c first-principles derivation | OPEN | UNAFFECTED |
| FTD-0032 | Master quadratic as L → ∞ limit of finite-L gap equation | RETRACTED (2026-04-19) | RETRACTED |
| FTD-0033 | Type III₁ classification of FTD flux algebra | HYPOTHESIS (Araki–Woods scaffold) | RESOLVED (demoted from SELECTION 2026-04-19) |
| FTD-0034 | Engine convergence to QED in L → ∞ limit (EFT campaign) | RESOLVED — 5 FLAGs replaced by Restatement A or B (2026-04-19, Session 2) | RESOLVED |
| FTD-0035 | Mechanism γ — gravitational a_phys derivation | OPEN-CLOSED-AS-DERIVATION (2026-04-19) | RESOLVED (closed; calibration recommended) |
| FTD-0036 | Postulate 1 (Discrete Space) — undefined-boundary cubic lattice | AXIOM | RESOLVED (restated 2026-04-19) |
| FTD-0037 | Postulate 2 (Discrete Time) — emergent from Lagrangian | SELECTION | UNAFFECTED |
| FTD-0038 | Postulate 3 (Ternary States {−1, 0, +1}) | AXIOM | UNAFFECTED |
| FTD-0039 | Postulate 4 (26-Moore locality) — derived from P1 + symmetry | THEOREM (per Axiom Zero §2.3a) | UNAFFECTED |
| FTD-0040 | Postulate 5 (Determinism) — derived from Lagrangian well-posedness | THEOREM (per Axiom Zero §3.3) | UNAFFECTED |
| FTD-0041 | a_phys ≡ ℓ_P calibration declaration (with K_B = m_e mass anchor) | CALIBRATION (declared in SPEC_FTD.md, 2026-04-19) | NEW |
| FTD-0042 | Yang-Mills mass gap "proof" (FTD_Yang_Mills_Mass_Gap.tex) | RETRACTED 2026-04-19 (Session 3) | RESOLVED — moved to docs/papers/archive/retracted_under_reframe/ with retraction note |
| FTD-0043 | Navier-Stokes regularity "proof" (FTD_Navier_Stokes.tex) | RETRACTED 2026-04-19 (Session 3) | RESOLVED — moved to docs/papers/archive/retracted_under_reframe/ with retraction note |
| FTD-0044 | Per-voxel mass gap from manifestation threshold (Theorem 5.1 of retracted YM paper) | THEOREM (survives reframe; preserved in retracted-archive .tex; could anchor a smaller honest paper if owner wishes) | UNAFFECTED |
| FTD-0045 | α_largeL ≈ 3.6 × α_ref (engine measurement at largest tested L, under a_phys ≡ ℓ_P) | HYPOTHESIS (calibration-conditional; falsifiable as stated) | RESOLVED (was FTD-0034 sub-claim; now standalone under calibration) |
| FTD-0046 | FTD_Thermodynamic_Limit (PDF-only, no TeX source) | RETRACTED 2026-04-19 (Session 3) | RESOLVED — moved to docs/papers/archive/retracted_under_reframe/ with pdftotext extraction |
| FTD-0047 | DERIV_THERMODYNAMIC_REFLEXION (PDF-only, no TeX source) | RETRACTED 2026-04-19 (Session 3) | RESOLVED — moved to docs/papers/archive/retracted_under_reframe/ with pdftotext extraction |
| FTD-0048 | 11 PDF-only papers without recoverable TeX source | ARCHIVED (not retracted; status pending owner re-authoring or accept-as-historical) | RESOLVED — moved to docs/papers/archive/pdf_only_no_source/ with pdftotext extractions + per-paper README |
| FTD-0049 | Project commit-attribution policy: no AI co-author trailers | POLICY (declared in CLAUDE.md, 2026-04-19) | NEW — historical commits cleaned via git filter-repo on main (222 → 0); force-push to remote pending owner approval |

---

## Detail blocks (load-bearing claims only)

### FTD-0001: Master Quadratic Polynomial + Roots

- **statement:** The polynomial `x² − 16 G*² x + 16 G*³ = 0` has two real roots, x₊ ≈ 137.036 and x₋ ≈ 3.024, computable from G* by the quadratic formula to arbitrary finite precision.
- **tag:** THEOREM
- **tag_history:** 2026-04-15 SELECTION (numerical verification only); 2026-04-19 THEOREM (pure algebra, no completed-infinity steps; Phase I audit verified).
- **proof_status:** COMPLETE
- **proof_location:** `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` (rewritten 2026-04-19), `scripts/proofs/proof_motivic_master_quadratic.py`
- **dependencies:** FTD-0002 (G*), FTD-0006 + FTD-0007 (coefficient 16 routes)
- **dependents:** FTD-0013, FTD-0014, FTD-0012, FTD-0024, FTD-0025
- **last_reviewed:** 2026-04-19
- **reframe_status:** UNAFFECTED — pure algebra.
- **citations:** `FOUND_AXIOM_ZERO.md` §3.2, `META_INDEX.md` row 7.17, `CLAUDE.md` "Firm theorems."

### FTD-0002: G* algebraic identity

- **statement:** `G* = √2 · Γ(1/4)² / (2π)`; equivalently `G*² / (2π) = Watson I_1 = Γ(1/4)⁴ / (4π³)` via Chowla–Selberg.
- **tag:** THEOREM
- **proof_status:** COMPLETE
- **proof_location:** `DERIV_WATSON_GSTAR_IDENTITY.md`, classical Chowla–Selberg.
- **dependencies:** none (algebraic primitive)
- **dependents:** FTD-0001, FTD-0007, FTD-0029
- **reframe_status:** UNAFFECTED — closed-form algebraic identity. Computable to arbitrary precision; does not require a completed lattice sum.

### FTD-0003: CM-curve uniqueness across class-number-1 fields

- **statement:** Among the 9 imaginary quadratic fields with class number 1, the discriminant `d = −4` (CM curve E: y² = x³ − x) is the **unique** field whose associated polynomial reproduces the dual match (1/α, N_c).
- **tag:** THEOREM (numerical scan exhaustive over 9 fields).
- **proof_status:** COMPLETE
- **proof_location:** `scripts/exploration/scan_cm_curves.py`, `CONJ_ALPHA_FROM_CM.md` (with one path flagged for re-derivation), `AUDIT_MASTER_QUADRATIC.md` Item (b).
- **reframe_status:** UNAFFECTED — finite-combinatorial scan.

### FTD-0004: Phase G emergent Coulomb at every finite L

- **statement:** The engine's emergent Gauss-law measurement satisfies `α_r(r, L) = 2 · r · G_L(r)` at every finite L (R² = 1.0000 at L = 384). Holds pointwise in L; no limit invoked.
- **tag:** THEOREM (finitary)
- **proof_location:** `DERIV_EMERGENT_COULOMB_GEOMETRIC.md`, Phase G EFT-program results.
- **reframe_status:** UNAFFECTED — explicitly finite-L.

### FTD-0005: Phase J partition-function ultralocality at L=2

- **statement:** Explicit calculation on the L=2 torus (8 voxels, 1107 charge-neutral configs) shows the action `S_E = (c²/2)|∇J|² + g_c · s · (∇·J)` is ultralocal in s (Parseval: ∫|∇J|² = ∫s²). Two dipoles at different separations give identical S_E.
- **tag:** THEOREM (finite-combinatorial)
- **proof_location:** `DERIV_PARTITION_FUNCTION_L2.md`, `scripts/proofs/partition_function_L2.py`
- **reframe_status:** UNAFFECTED — explicit finite-L computation.
- **consequence:** The FTD analytical action contains no Coulomb interaction between static charges; classical extremisation cannot fix g_c. This closes Mechanism C (fixed-point self-consistency) as a route for first-principles g_c.

### FTD-0006 & FTD-0007: Coefficient 16, two routes

- **FTD-0006:** `|Aut(E)|² = 4² = 16` for E: y² = x³ − x. Finite-combinatorial. THEOREM. (`DERIV_DUAL_DERIVATION_OF_16.md`)
- **FTD-0007:** `z_BCC · 2 = 8 · 2 = 16` (BCC coordination times non-void ternary states). THEOREM. (`FOUND_DIMENSIONAL_COUNTING.md` §5.4)
- **reframe_status:** Both UNAFFECTED. Both are finite-combinatorial; neither invokes a limit.
- **note:** A third historical route (DOF count 24 − 7 − 1 = 16 in Coulomb gauge on T³) was retracted as incorrect; proper gauge-fixing yields 14, not 16. See `AUDIT_MASTER_QUADRATIC.md`.

### FTD-0013: x₊ ↔ 1/α physical identification (1.26 ppm)

- **statement:** The larger root x₊ = 137.036 is identified with 1/α (CODATA 137.035999084), agreement to 1.26 ppm.
- **tag:** STRONGLY MOTIVATED CONJECTURE
- **tag_history:** Pre-2026-04-19 framed as THEOREM ("derived"); 2026-04-19 demoted given the discovery that the gap-equation L → ∞ argument was the load-bearing derivation step and that argument is closed.
- **evidence:** (i) numerical match 1.26 ppm (better than experimental precision on most QED loop tests); (ii) dual match with x₋ ↔ N_c (one polynomial, two unrelated physical numbers); (iii) CM-curve structural uniqueness across class-number-1 fields (FTD-0003).
- **what is NOT claimed:** that a dynamical mechanism produces 1/α from FTD's update rules. The identification rests on algebraic match + structural uniqueness, not on a dynamical derivation.
- **reframe_status:** RESOLVED — the demotion was triggered by the reframe's clarification that the gap-equation derivation route was always conjectural.

### FTD-0014: x₋ ↔ N_c = 3 physical identification (0.80%)

- **statement:** The smaller root x₋ = 3.024 is identified with N_c = 3 (number of QCD colour charges), agreement 0.80%.
- **tag:** STRONGLY MOTIVATED CONJECTURE
- **tag_history:** Same as FTD-0013.
- **evidence:** Same as FTD-0013. The dual nature of the match (one polynomial → two physical constants from disjoint sectors) is the key structural evidence.

### FTD-0030: a_phys derivation status [OPEN — NEW]

- **statement:** The lattice-to-physical-length conversion `a_phys` (one voxel, in metres) must be derived from Axiom-Zero invariants or declared an empirical calibration. Under undefined-boundary ontology, every dimensional finite-L prediction depends on this conversion.
- **tag:** OPEN
- **proof_status:** OPEN
- **history:** Created 2026-04-19 by `AUDIT_INFINITY_REFRAME.md` Section 3 (Interpretation D). Prior to the reframe, the question was implicit because L → ∞ allowed deferral.
- **status:** Mechanism α (algebraic) cannot deliver — no dimensional invariant in lattice algebra. Mechanism β (EFT-matching) delivers calibration not derivation. Mechanism γ (gravitational fixed point) **closed as derivation** by `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` (FTD-0035).
- **recommended disposition:** declare `a_phys ≡ ℓ_P` in `SPEC_FTD.md`; quote all dimensional predictions as conditional on this calibration; flag dimensionless predictions (α, mass ratios, mixing angles) as the calibration-independent falsifiable spine.
- **dependents:** FTD-0034 (3.6× α plateau interpretation), every dimensional engine benchmark.
- **citations:** `OPEN_A_PHYS_DERIVATION.md`, `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md`.

### FTD-0032: Master quadratic as thermodynamic limit [RETRACTED 2026-04-19]

- **statement (original, retracted):** "The master quadratic is the L → ∞ limit of FTD's finite-L lattice gap equation."
- **tag:** RETRACTED
- **tag_history:** Pre-2026-04-19 SELECTION; 2026-04-19 RETRACTED.
- **why retracted:**
  1. Phase I (`AUDIT_MASTER_QUADRATIC.md` Item 1): the claimed numerical convergence does **not** hold — the corrected scan shows no convergence to (137.036, 3.024).
  2. Phase J (`DERIV_PARTITION_FUNCTION_L2.md`): the L=2 partition function carries no master-quadratic signature; the action is ultralocal at finite L.
  3. `AUDIT_INFINITY_REFRAME.md`: under undefined-boundary ontology, "L → ∞" is not a well-posed load-bearing step.
- **replacement:** the polynomial is treated as a **pure algebraic object** (FTD-0001); physical identification rests on dual match + CM-curve uniqueness (FTD-0013, FTD-0014, FTD-0003), not on a thermodynamic-limit derivation.
- **citations:** `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` (rewritten 2026-04-19), `FOUND_AXIOM_ZERO.md` §4.2 (rewritten 2026-04-19).

### FTD-0033: Type III₁ classification — Araki–Woods scaffold

- **statement (current):** Under the Araki–Woods inductive-limit scaffold applied to a system of FTD's local algebra type, the would-be limit factor is Type III₁. **This is a property of the scaffold, not of FTD-as-defined.** Every region the framework actually exhibits is Type I.
- **tag:** HYPOTHESIS
- **tag_history:** Pre-2026-04-19 SELECTION ("Type III₁ in the thermodynamic limit"); 2026-04-19 demoted to HYPOTHESIS — the position-property axiom does not commit to an inductive-limit construction.
- **what survives:** the finite-region results (Sections 1–4 of `DERIV_VON_NEUMANN_CONSTRUCTION.md`) remain THEOREM. The Type III₁ classification + the measurement-as-type-transition reading are HYPOTHESIS.
- **citations:** `DERIV_VON_NEUMANN_CONSTRUCTION.md` (rewritten 2026-04-19), `META_INDEX.md` row 7.18.

### FTD-0034: Engine → QED convergence claim (EFT program) [PENDING-RE-DERIVATION]

- **statement (original, flagged):** Various forms of "engine measurement X(L) → QED-target Y as L → ∞" across `DERIV_BETA_FUNCTION_MEASURED.md`, `DERIV_DYNAMICAL_SM_EMERGENCE.md`, `DERIV_DAY2_CAMPAIGN.md` (×2), `CONJ_ALPHA_FROM_CM.md` (Path A).
- **tag:** RETRACTED-PENDING (5 inline FLAGs)
- **status:** Each flagged claim has two acceptable restatements per `TRACKER_REFRAME_FLAGS.md`:
  - **A — Finitary scaling claim:** "X(L) − Y ∝ L⁻ᵖ with exponent p ≈ N at canonical regime, fitted across L ∈ {…}, tolerance ε."
  - **B — Calibration-conditional claim:** "X(L_canonical) = Y at fixed `a_phys = …`."
- **owner action required:** EFT-campaign owner must choose A or B per item.
- **citations:** `TRACKER_REFRAME_FLAGS.md`, `META_INDEX.md` row 7.19.

### FTD-0035: Mechanism γ — gravitational a_phys [CLOSED AS DERIVATION 2026-04-19]

- **statement:** Attempt to fix `a_phys` from `G_N(physical) = G_N(lattice) · a_phys³ / (M_unit · t_phys²)` using framework calibrations `c_lat = 1/√3` and `K_B = m_e`.
- **tag:** OPEN-CLOSED-AS-DERIVATION
- **result:** With `G_N(lat) = 0.01` (engine toy-regime), `a_phys ≈ 4 × 10⁻⁵⁵ m`. With `G_N(lat) = α_G ≈ 5.91 × 10⁻³⁹`, `a_phys ≈ 7 × 10⁻⁷ m`. Both implausible. Both `G_N(lat)` and `M_unit` are themselves calibrations rather than Axiom-Zero invariants — the chain converts one calibration into another rather than deriving `a_phys`.
- **disposition:** Mechanism γ is **not** a path to first-principles `a_phys`. Reverts to the fallback in FTD-0030: declare `a_phys` as calibrated parameter.
- **citations:** `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md`, `META_INDEX.md` row 7.15.

### FTD-0036: Postulate 1 — Discrete Space (undefined-boundary)

- **statement (current):** Space is a 3D cubic lattice with **no defined boundary**: at every specified position, the six axis-adjacent (and 26-Moore-adjacent) sites exist. The lattice is finitely specified; no completed-totality commitment.
- **tag:** AXIOM
- **tag_history:** Pre-2026-04-19 read as `L ⊂ ℤ³` with implicit completed-infinity ontology; 2026-04-19 restated to undefined-boundary directly.
- **citations:** `SPEC_FTD.md` Postulate 1 (lines 211–214), `FOUND_AXIOM_ZERO.md` §1.1 (rewritten 2026-04-19), `CANONICAL_REFRAME.md`.

---

### FTD-0041: a_phys ≡ ℓ_P calibration declaration

- **statement:** One voxel ≡ one Planck length. Tick: t_phys = √3 · ℓ_P / c. Mass unit: M_unit = m_e / K_B = 1 MeV/c².
- **tag:** CALIBRATION (declared, not derived)
- **declared in:** `SPEC_FTD.md` (between Postulate 2 and Postulate 3, "LATTICE ↔ PHYSICAL CALIBRATION" section).
- **rationale:** `a_phys` cannot be derived from Axiom-Zero invariants alone. Mechanisms α / β / γ all closed (see FTD-0030, FTD-0035). Planck-length declaration is the natural calibration if the framework operates at the smallest length physics has reason to invoke.
- **discipline:** every dimensional FTD prediction is **conditional on this calibration**. Dimensionless predictions are calibration-independent and constitute the falsifiable spine.
- **dependents:** every dimensional engine benchmark; FTD-0045.

### FTD-0044: Per-voxel mass gap (survives in YM paper)

- **statement:** A per-voxel mass gap exists from the manifestation threshold K_B; this holds at every site, in every region the framework actually exhibits.
- **tag:** THEOREM
- **proof_location:** `docs/papers/speculative/FTD_Yang_Mills_Mass_Gap.tex` Theorem 5.1.
- **reframe_status:** UNAFFECTED — local statement, holds at every L.
- **note:** this is the only YM-paper claim that survives the reframe cleanly. Could anchor a smaller, honest paper without the Clay-eligibility framing.

### FTD-0045: α_largeL ≈ 3.6 × α_ref (calibration-conditional)

- **statement:** The 1/L² fit at the largest tested L (L = 384) gives α_largeL ≈ 3.74 × α_ref, with empirical residual band [3.35, 3.74] × α_ref across three scaling laws. **Conditional on `a_phys ≡ ℓ_P` and `K_B = m_e`.**
- **tag:** HYPOTHESIS (falsifiable as stated)
- **proof_location:** `engine/tests/benchmark_dynamical_sm.cpp`, `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex`
- **status:** under the declared calibration, this is a **falsifying disagreement** with the QED prediction (α_QED ≠ α_engine by factor ~3.6). Either the calibration is wrong (try alternative `a_phys`), or the framework predicts a different α from QED. Both are honest readings.
- **dependents:** EFT paper headline; the 3.6× plateau interpretation.

---

## Maintenance log

| Date | Change | By |
|---|---|---|
| 2026-04-19 | Initial population. Captured 40 load-bearing claims with tag history through reframe cycle. | Session 1. |
| 2026-04-19 | Session 2: FTD-0030 RESOLVED via SPEC_FTD calibration declaration; FTD-0034 RESOLVED via 5 FLAG restatements; new rows FTD-0041, 0042, 0043, 0044, 0045 added. | Session 2. |
| 2026-04-19 | Session 3: FTD-0042 (YM) and FTD-0043 (NS) RETRACTED to archive; FTD-0044 (per-voxel mass gap survives) preserved in retracted-archive .tex; FTD-0046 (Thermodynamic_Limit) and FTD-0047 (Thermodynamic_Reflexion) RETRACTED with pdftotext extractions; FTD-0048 (11 other PDF-only papers) ARCHIVED; FTD-0049 (commit-attribution policy + 222-commit Co-Authored-By cleanup on main) NEW. | Session 3. |

---

## Notes on coverage

This v1.0 ledger captures the **load-bearing** claims tracked through the April 19 audit cycle. It is not exhaustive: there are ~50 additional parametric insertions catalogued in `CATALOG_PARAMETRIC_INSERTIONS.md` and ~50 external-physics adoptions catalogued in `AUDIT_EPISTEMIC_AUDIT.md`, neither of which is duplicated here. Future maintenance should cross-reference the catalog rather than re-list every parametric.

Outstanding portfolio claims (papers in `docs/papers/`, `dissemination/papers/`, `dissemination/manuscript_v2/`) are not yet ledger-rowed pending the broader-portfolio inventory + classifier output. Add rows as those audits return.
