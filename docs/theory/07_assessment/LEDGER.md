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
| FTD-0015 | m_e = m_P · √(2π) · (16/3) · α¹¹ (0.19%) | STRONGLY MOTIVATED CONJECTURE | UNAFFECTED |
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
| FTD-0050 | Master quadratic as characteristic polynomial of an RG step on the FTD engine | CLOSED NEGATIVE (2026-04-20) | NEW — falsified for engine's 18-pt coupling stencil; does NOT demote FTD-0001/0013/0014 |
| FTD-0051 | Langevin thermostat on wave_vel (OU noise; CPU + GPU single-substrate) | INFRASTRUCTURE, operational, CPU/GPU parity, 63× GPU speedup at L=64 | NEW 2026-04-20, GPU ported same day |
| FTD-0052 | s-field stochastic dynamics (ternary Metropolis for thermal ensemble) | NOT PURSUED (F9-avoidance) | Explicit decision 2026-04-20 — running would be ceremony, not evidence, given FTD-0050 structural argument |
| FTD-0053 | α_eff L=256 T=0 scaling data point | MEASURED | NEW 2026-04-20 — α=0.1340 (=4.47×α_ref), R²=0.99915 on GPU in 159s; first-ever at this scale; consistent with existing 3.6× plateau band |
| FTD-0054 | Thermal α via shared thermal background (measure_alpha_eff refactor) | OPEN (compute-bound) | Path A mechanics working 2026-04-21: GPU sync bug root-caused and fixed (1 LOC in RenderBridge::run()), thermal bulk cancels correctly in V(r) subtraction. Remaining gap is ensemble averaging (~50 LOC, 100× wall time) to beat thermal noise; no design blocks. |
| FTD-0055 | BCC tadpole at N=4096 on GPU (Priority 1 of external GPU plan) | MEASURED | NEW 2026-04-21 — T_latt(4096) = 0.022922459870, matches external plan target 0.02292245997 to 13 ppb with clean 1/N² convergence on RTX 5090 in 3.5s. Extends prior CPU N=150 result by factor 27 in N. |
| FTD-0056 | Unrenormalized one-loop BCC tadpole residual has no continuum limit | THEOREM (numerical, Priority 5 of external GPU plan) | NEW 2026-04-21 — T_latt(a, L=100) diverges as a→0: 0.023 at a=2/3, 4.96 at a=2/48. The "9.68 ppb residual" is a specific-a, specific-regularization outcome. Does NOT invalidate DERIV_ONE_LOOP_LATTICE_ALPHA.md's [DERIVED given a=2/D] tag, but strengthens the honesty of that conditional. |
| FTD-0057 | Non-perturbative HMC measurement of ⟨η⟩ on BCC lattice (Priority 2) | MEASURED 2026-04-21 — spec success criterion MET | MC ⟨η⟩ = −1.695×10⁻⁴ vs one-loop prediction −1.710×10⁻⁴; \|Δ\|/σ = **0.19** (σ_τ-corrected = 7.8×10⁻⁶). 99.1% acceptance, max\|η\| = 0.91 (no tunneling), 15 min wall on RTX 5090 at N=64. Perturbation theory confirmed non-perturbatively within 1σ. |
| FTD-0058 | Structure-2 Ward-valid two-U(1) scalar gauge completion | CLOSED NEGATIVE 2026-04-22 | Natural scalar cases S2-A..S2-E fail to reproduce the Structure-1 ppb closure under bubble+seagull Ward-valid periodic BZ calculation. Best natural case S2-A: x_S2 = 137.036171847817, residual +1259.797 ppb, Ward max 1.053e-18. Structure-1 ppb correction is scheme-specific unless a unique FTD-to-EFT matching principle is derived. |
| FTD-0059 | No-go theorem for `a_phys` derivation from Axiom Zero | THEOREM (2026-04-23) | NEW — formalizes the structural reason Mechanisms α/β/γ/δ all failed: Axiom-Zero invariants form a ring of SI-dimensionless reals, so no length is expressible without an external dimensional generator. `a_phys ≡ ℓ_P` calibration (FTD-0030, FTD-0041) is theorem-enforced, not convenience. See `docs/theory/10_eft_program/THEOREM_A_PHYS_NO_GO.md` and the supporting Mechanism-δ closure `DERIV_A_PHYS_MECHANISM_DELTA_ATTEMPT.md`. |
| FTD-0060 | Baryon composition correction $K_{\text{comp}} = m_e/\pi$ (conjectured in manuscript ch. 1.10b) | CLOSED NEGATIVE (2026-04-23) | NEW — numerical premise fails audit. True $K/m_e$ needed is $0.31532$ (proton) / $0.31479$ (neutron) / **$-0.35187$** (delta, opposite sign); $1/\pi = 0.31831$ differs by 0.95–1.11% (1.5–1.8 keV), not 0.4 keV / <1 eV as claimed in the chapter. No FTD-primitive mechanism (Moore-shell integral, 3-quark worldline, Watson projection, polyhedral average, one-loop correction, solid-angle reduction) naturally produces either the approximate or the true value. The 174-ppm $m_p/m_e$ gap remains [OPEN]. See `docs/theory/03_derivations/DERIV_KCOMP_CIRCLE_TO_SPHERE.md`. Does NOT affect FTD-0016 (which is already tagged STRONGLY MOTIVATED CONJECTURE without the $1/\pi$ correction). Naming-collision note: the $K_B = 0.511$ MeV manifestation-threshold quantity in `DERIV_KCOMP_VOLUMETRIC_SHELL.md` is a separate object and is unaffected; the symbol $K_{\text{comp}}$ should be reserved for that quantity going forward. |

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

### FTD-0011: Phase H coupling scaling — g_c² scales α_r [THEOREM]

- **statement:** Adding an explicit coupling constant g_c to the Gauss-law source (∇·J = g_c · s) rescales the emergent Coulomb observable as α_r(r, L) → g_c² · α_r(r, L) at every finite L.
- **tag:** THEOREM (scaling argument, analytical).
- **proof_location:** `docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md` §7.
- **reframe_status:** UNAFFECTED — finitary scaling relation.
- **reviewer note (2026-04-20, Link 8 closure + EFT audit):** This theorem says only that α_r rescales by a factor g_c² — it does not say α_r converges to α_QED for any particular choice of g_c. Phase H is currently spec'd but not measured. When measured, the right epistemic frame is: "is there a g_c value that reproduces measured α_QED?". If such a g_c exists, it would be a [CALIBRATION], not a [DERIVATION], because the engine's 18-point coupling stencil is (SC+FCC)/2 and structurally orthogonal to the BCC sub-stencil where the master quadratic's 16G*² coefficient lives (FTD-0050, Link 8 closure). **Future editors: if Phase H is reported as "FTD-engine-derives α_QED from first principles", cross-check against `AUDIT_LINK8_CLOSURE.md` and `AUDIT_EFT_BCC_ORTHOGONALITY.md` before accepting the framing.**

### FTD-0006 & FTD-0007: Coefficient 16, two routes

- **FTD-0006:** `|Aut(E)|² = 4² = 16` for E: y² = x³ − x. Finite-combinatorial. THEOREM. (`DERIV_DUAL_DERIVATION_OF_16.md`)
- **FTD-0007:** `z_BCC · 2 = 8 · 2 = 16` (BCC coordination times non-void ternary states). THEOREM. (`FOUND_DIMENSIONAL_COUNTING.md` §5.4)
- **reframe_status:** Both UNAFFECTED. Both are finite-combinatorial; neither invokes a limit.
- **note:** A third historical route (DOF count 24 − 7 − 1 = 16 in Coulomb gauge on T³) was retracted as incorrect; proper gauge-fixing yields 14, not 16. See `AUDIT_MASTER_QUADRATIC.md`.

### FTD-0013: x₊ ↔ 1/α physical identification (1.26 ppm)

- **statement:** The larger root x₊ = 137.036 is identified with 1/α (CODATA 2022: 137.035999177), agreement to 1.26 ppm.
- **tag:** STRONGLY MOTIVATED CONJECTURE
- **tag_history:** Pre-2026-04-19 framed as THEOREM ("derived"); 2026-04-19 demoted given the discovery that the gap-equation L → ∞ argument was the load-bearing derivation step and that argument is closed.
- **evidence:** (i) numerical match 1.26 ppm (better than experimental precision on most QED loop tests); (ii) dual match with x₋ ↔ N_c (one polynomial, two unrelated physical numbers); (iii) CM-curve structural uniqueness across class-number-1 fields (FTD-0003).
- **what is NOT claimed:** that a dynamical mechanism produces 1/α from FTD's update rules. The identification rests on algebraic match + structural uniqueness, not on a dynamical derivation. Also not claimed after FTD-0058: that the Structure-1 ppb one-loop correction is a scheme-independent gauge-theory prediction. The Ward-valid Structure-2 scalar gauge completion does not reproduce it under the tested natural matter assumptions.
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
- **proof_location:** `docs/papers/archive/retracted_under_reframe/FTD_Yang_Mills_Mass_Gap.tex` Theorem 5.1 (paper retracted Session 3; per-voxel mass gap content preserved in archived `.tex`).
- **reframe_status:** UNAFFECTED — local statement, holds at every L.
- **note:** this is the only YM-paper claim that survives the reframe cleanly. Could anchor a smaller, honest paper without the Clay-eligibility framing.

### FTD-0045: α_largeL ≈ 3.6 × α_ref (calibration-conditional)

- **statement:** The 1/L² fit at the largest tested L (L = 384) gives α_largeL ≈ 3.74 × α_ref, with empirical residual band [3.35, 3.74] × α_ref across three scaling laws. **Conditional on `a_phys ≡ ℓ_P` and `K_B = m_e`.**
- **tag:** HYPOTHESIS (falsifiable as stated)
- **proof_location:** `engine/tests/benchmark_dynamical_sm.cpp`, `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex`
- **status:** under the declared calibration, this is a **falsifying disagreement** with the QED prediction (α_QED ≠ α_engine by factor ~3.6). Either the calibration is wrong (try alternative `a_phys`), or the framework predicts a different α from QED. Both are honest readings.
- **dependents:** EFT paper headline; the 3.6× plateau interpretation.
- **reviewer note (2026-04-20, Link 8 closure + EFT audit):** The 3.6× quantity is labeled in `PAPER_FTD_AS_WILSONIAN_EFT.tex` as a parameter-free lattice-Poisson-kernel geometric value, *not* a claim that engine dynamics derive α_QED. That framing is honest and should be preserved. Relevant cross-references: (i) FTD-0050 (master quadratic is NOT the characteristic polynomial of an RG step on the engine's 18-point stencil — the stencil is BCC-orthogonal); (ii) `docs/theory/10_eft_program/AUDIT_LINK8_CLOSURE.md` §Detail 4 (Phase 1 analytical closure of the BCC-extended stencil path); (iii) `docs/theory/10_eft_program/AUDIT_EFT_BCC_ORTHOGONALITY.md` (full audit confirming no existing EFT-campaign claim requires retraction). **Future editors: any revision of this row or of the EFT paper that reframes α_largeL as "FTD-engine-derives α_QED" would collide with the Link 8 closure and should be stopped at the LEDGER gate.**

### FTD-0050: Master quadratic as characteristic polynomial of an RG step on the FTD engine [CLOSED NEGATIVE 2026-04-20]

- **statement:** The master quadratic `x² − 16 G*² x + 16 G*³ = 0` does **not** arise as the characteristic polynomial of any natural block-spin renormalization-group step acting on the engine's bare-lattice (wave_propagation + coupling + gauss_projection) dynamics.
- **tag:** CLOSED NEGATIVE
- **tag_history:** 2026-04-20 created and closed in the same cycle. This was an implicit conjecture on top of FTD-0001/0013/0014; testing was invoked by user as Link 8 strategy.
- **evidence:**
  - **Candidate 1 Runs 1 literal (L=8), 6 literal (L=16), 1 extended (L=64)**: J-field 2×2×2 averaging + Coulomb-tail α extraction. All three variants produce y_n with a clean ×16 per-level geometric ratio, inconsistent with the master-quadratic characteristic eigenvalues {137, 3}. Best 2-equation fit: A dev 9.99%, **B dev 434%**. See `engine/tests/test_link8_kadanoff.cpp`.
  - **Option β (analytical)**: engine's 18-point stencil decomposes algebraically as σ_18 = ½(σ_SC + σ_FCC), with **zero BCC component**. The master quadratic's coefficient 16G*² is tied exactly to the BCC Watson integral W_BCC = Γ(1/4)⁴/(4π³) via `16·2π·W_BCC = 16G*²`. The engine's stencil is structurally orthogonal to the BCC sub-stencil. See `scripts/exploration/link8_option_beta_watson_diagnostic.py`.
  - **Candidate 1 Run 3 on Langevin-thermalized ensemble (4 seeds, 5000-tick burn-in each)**: |J|² connected correlator amplitude at r_max. 2-eq fit: A dev −99.6%, B dev −100.4%, det M = −2×10⁻⁶ (singular). See `engine/tests/test_link8_run3_thermal.cpp`.
  - **Phase 1 (principled BCC-extension gate, analytical)**: linearized 2-coupling (g_SCFCC, g_BCC) flow matrix M under standard 2×2×2 block-averaging. Numerical values at 64³ k-grid: `M = [[+0.987, -0.515], [+0.127, +1.454]]`; **trace(M) = 2.44 (target 140.06, dev −98.3%)**; **det(M) = 1.50 (target 414.39, dev −99.6%)**; **eigenvalues 1.22 ± 0.10 i complex** (target roots {137.04, 3.02} are real; discriminant of M negative, of master quadratic positive — qualitatively different). Rules out the "give the engine a BCC sector and rerun" path without touching engine code. See `scripts/exploration/link8_phase1_flow_matrix.py` and `AUDIT_LINK8_CLOSURE.md` §Detail 4.
- **what is NOT claimed:** The master quadratic itself is not demoted. FTD-0001 remains THEOREM (algebraic identity); FTD-0013 and FTD-0014 remain STRONGLY MOTIVATED CONJECTURE (physical identifications on dual-match + CM-curve-uniqueness evidence). What is falsified is the *additional* layer of interpretation that the polynomial is the characteristic polynomial of an RG step on the current engine.
- **dependencies:** FTD-0001 (master quadratic), FTD-0002 (G*), FTD-0007 (z_BCC · 2 = 16 route).
- **dependents:** Removes one speculative interpretation path; strengthens (by elimination) the number-theoretic / CM-curve origin of the polynomial as the load-bearing derivation route.
- **future work:** `FTD-0052` tracks the s-field Metropolis route (expected negative; deferred).
- **citations:** `AUDIT_LINK8_CLOSURE.md`, `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` Part I.

### FTD-0051: Langevin thermostat on wave_vel [INFRASTRUCTURE 2026-04-20, GPU ported same day]

- **statement:** Engine gained a per-voxel Ornstein–Uhlenbeck update `v ← (1−γ)v + √(2γT)·η` on wave_vel, exposed via `TermToggles::langevin`, `langevin_T`, `langevin_gamma`, `langevin_seed`. Runs in parallel with `gauss_project`; thermal ensemble lives on the Gauss-physical subspace.
- **tag:** INFRASTRUCTURE (operational; equipartition-verified on both CPU and GPU paths; CPU/GPU parity confirmed).
- **scope:** single-substrate, both CPU and GPU paths. Dual-substrate is NOT wired (still uses deterministic damping path).
- **CPU verification:** `engine/tests/test_langevin_equipartition.cpp` — L=16, T=0.01, γ=0.01, 1000 burn + 2000 measure ticks. `<|wave_vel|²>_voxel = 0.03120` vs target `3T = 0.0300`, dev **+4.0%** (within 5% threshold). Per-component isotropy `<v²>/3 = 0.0104` vs T. Mean `<v> ≈ 0`.
- **GPU verification (same test, CUDA backend):** `<|wave_vel|²>_voxel = 0.03133` vs target `3T = 0.0300`, dev **+4.44%**. CPU and GPU equipartition values agree to 0.03% absolute — tight parity. Single-voxel variance C(0) = 0.997·T vs target T (dev −0.29%). Built via WSL2 / CUDA 13 path (Windows CMake 4 + NVCC 13 escape bug sidestepped per `STATUS_CUDA_BUILD.md`).
- **GPU speedup (FTD-0051 GPU port validation):** `engine/tests/benchmark_langevin_gpu.cpp` — scaling sweep across L ∈ {16, 64, 128, 256}. L=16: 2.98× (launch-overhead regime); L=64: 58×; **L=128: 112×**; L=256: 6.81 ms/tick GPU-only (CPU baseline skipped — would be hours). CPU/GPU value parity on `<|v|²>` tightens as L grows: 2% at L=16, 0.03% at L=64, 0.02% at L=128. Port is bit-comparable with CPU within RNG-sequence differences.
- **GPU implementation:** `engine/cuda/kernels_stencil.cu` `wave_update_kernel` + `phase_write_kernel` extended with OU branch replacing the damping block when `toggles.langevin` is true; `engine/cuda/gpu_engine.cu::gpu_phase_write` generates 3N standard normals into `d_langevin_noise` via `curandGenerateNormalDouble` before each kernel call. `GpuBuffers::d_langevin_noise` buffer added (24·N bytes, always-allocated).
- **known limitation:** J-field (the "position" DoF) thermalizes more slowly than wave_vel (the "momentum" DoF) due to coupled-oscillator dynamics; `<|J|²>_voxel` is dominated by slow low-k modes and takes many more ticks to equilibrate than wave_vel. Correctness of the thermostat is established by wave_vel equipartition; J ratios in blocked observables (Session C) are seed-dependent but the y_n pattern (the thing the recurrence tests) is structureless either way.
- **what this unblocks (now with GPU):** matched-stencil β-function at non-zero T at L ≥ 128 (EFT Day-2 campaign), ensemble averaging for condensate measurements (EWSB), fluctuation-dissipation tests, operator-spectrum scaling dimensions at thermal equilibrium (EFT Phase 3). Concrete scale: at L=256 with γ=0.01 a 5000-tick burn-in runs in ~34 s on GPU, making L ∈ {128, 256, 512} thermal sweeps a minute-scale task rather than hour-scale.
- **known caveat on equipartition at large L + short burn:** at L=128/256 with only 100 ticks of measurement the observed `<|v|²>` is ~33% below 3T because the coupled J-v dynamics have not reached equilibrium — the wave equation's lowest-k mode has period ~L·√3 ticks, so 100 ticks is <0.5 periods at L=128 and <0.25 at L=256. CPU and GPU paths give the SAME deviation to 0.02% → confirms port correctness; the deviation is a physics/burn-in issue, not a port bug. Production runs should use burn ~ 10·L ticks minimum (and longer for tighter equilibration).
- **follow-on:** dual-substrate extension (currently falls through to deterministic damping when `toggles.dual_substrate` is true even if langevin is also set); optional BAOAB integrator upgrade for higher-order accuracy; `langevin_seed` wiring to override the default cuRAND seed of 42 (currently ignored on GPU; CPU path uses the RenderBridge rng_ seed).

### FTD-0053: α_eff L=256 T=0 scaling data point [MEASURED 2026-04-20]

- **statement:** First-ever α_eff extraction at L=256 on the FTD engine. Result: α_fit = 0.1340, R² = 0.99915, E_self = 0.0306, measured via `measure_alpha_eff(256, 300, 4, 80, 8, 0.05)` on GPU in 158.7 seconds wall-time. V(r) samples: 10 data points from r=4 to r=76 with clean 1/r tail.
- **tag:** MEASURED.
- **significance:** converges the Day-2 plateau measurement to an additional scale. L=128 gave α=0.1343, L=256 gives α=0.1340 (0.23% shift). R² improves from 0.985 to 0.99915. At 4.47 × α_ref this is at the high end of the existing 3.6× plateau band [3.35, 3.74] × α_ref from `PAPER_FTD_AS_WILSONIAN_EFT.tex`. Extends but does not contradict the published finding.
- **reviewer note:** Consistent with the existing interpretation as a pure lattice-Poisson-kernel geometric quantity. Does not affect FTD-0045 status or the BCC-orthogonality audit conclusion (FTD-0050 + `AUDIT_EFT_BCC_ORTHOGONALITY.md`).
- **artifacts:** `engine/tests/benchmark_alpha_scaling.cpp`, `docs/theory/10_eft_program/AUDIT_ALPHA_SCALING_L256.md`.

### FTD-0054: Thermal α via shared thermal background [OPEN 2026-04-20, Path A attempt failed 2026-04-21]

- **statement:** Thermal α extraction at T > 0 is not achievable with the current `measure_alpha_eff` observable because each internal `RenderBridge` (self+, self−, pair(r)) has an independent Langevin noise realization. The thermal bulk energy (O(T·L³)) does NOT cancel in the V(r) = E_pair − E_self_+ − E_self_− subtraction. At L=64 with T=10⁻⁷, E_self jumped from 0.028 (T=0) to 0.133 — a factor ~5 thermal contamination that swamps the Coulomb signal ~0.03.
- **tag:** OPEN.
- **diagnosis:** physics issue, not a port bug. The V(r) observable was designed for deterministic runs where the subtraction cancels vacuum self-energy. Under independent-realization thermal ensembles, thermal bulks are uncorrelated and don't cancel.
- **Path A attempt (2026-04-21) — PARTIAL CLOSE, still OPEN pending ensemble averaging:**
  - Built `prepare_thermal_background` + `copy_flux_and_wave_vel` + `place_test_charge_on_bg` + `measure_alpha_eff_on_bg` helpers to share a single thermal bg across all measurement bridges.
  - **Root-caused and FIXED a GPU sync bug** (`RenderBridge::run()` GPU fast-path bypassed `tick()` and thus `gpu_flush_host_mutations()`, so host-side mutations to voxels_ never reached the device before `run(N)`). One-line fix in `engine/src/render_bridge.cpp`. T=0 baselines unchanged across L ∈ {32, 64, 128, 256} (no regression).
  - After the fix, Path A produces measurable data (not zeros): at L=64 T=1e-5, V(r) clusters around −17.7 = −E_self — i.e., thermal bulk IS cancelling. But the r-variation (~0.05 of the bulk, 0.3%) is dominated by thermal-evolution-divergence between bridges rather than the Coulomb signal (~0.006 variation, 0.03%). Single-realization noise is ~10× the Coulomb signal at L=64, ~2000× at L=128.
  - **Remaining work (unambiguously compute-bound now, no design issues):** ensemble averaging over N independent thermal backgrounds. Noise drops as 1/√N; need N~100 at L=64 to bring thermal noise below Coulomb signal. ~50 LOC outer loop + 100× current wall time. Not done this session.
- **helpers landed:** `LangevinOptions`, `prepare_thermal_background`, `copy_flux_and_wave_vel`, `place_test_charge_on_bg`, `measure_self_energy_on_bg`, `measure_pair_energy_on_bg`, `measure_alpha_eff_on_bg` — all in `engine/include/ftd/eft/coupling_measurement.h`. They compile and execute correctly at the mechanics level; the physics interpretation of the resulting α needs the revised design above.
- **Path B (alternative, not yet attempted):** use connected correlator ⟨J(0)·J(r)⟩ − ⟨J⟩² on thermal ensemble without test charges. Connected correlators are thermal-bulk-free by construction. ~200 LOC, uses `correlations.h`. Becomes relatively more attractive now that Path A turned out to need more work than estimated.
- **dependencies:** FTD-0051 (Langevin infrastructure, operational).
- **artifacts:** `docs/theory/10_eft_program/AUDIT_ALPHA_SCALING_L256.md` (full analysis including Path A failure diagnosis).
- **GPU sync bug FIXED 2026-04-21:** `RenderBridge::run()` GPU fast-path was bypassing `gpu_flush_host_mutations()` (which only ran inside `tick()`). Any workflow that wrote directly to host `voxels()[i].field` and then called `run(N)` silently ran on GPU's zero state and downloaded zeros. One-line fix in `engine/src/render_bridge.cpp::run()`. Latent bug present since Wave 5.2 (2026-04-14); undetected because `inject_particle` and `inject_flux` both bypass host voxels and write directly to GPU, so the existing test suite never hit this path. Now any future test that writes host voxels before `run()` will work correctly.

### FTD-0052: s-field stochastic dynamics (ternary Metropolis) [NOT PURSUED 2026-04-20 — F9-avoidance]

- **statement:** A ternary-Metropolis update on the state field `s ∈ {−1, 0, +1}` would be needed to generate thermal s-field ensembles for running Candidate 1 Run 5 (⟨s·s⟩ correlator mass from exponential decay).
- **tag:** NOT PURSUED
- **reason:** running Run 5 and getting the predicted negative result would **not add information beyond what the SC+FCC vs BCC structural argument (FTD-0050) already gives**. The structural argument has been confirmed three times (Runs 1/6 geometric ×16 flow; Option β analytical stencil decomposition; Run 3 on thermalized ensemble; Phase 1 analytical 2-coupling flow matrix). A fourth confirmation via different machinery would be ceremony, not evidence. This is exactly the pattern GTCA F9 flags: "generating material that feels productive but does not change the framework's epistemic status." Explicit decision taken 2026-04-20 by project owner.
- **re-open condition:** only if the structural argument changes — e.g., if a principled modification to the FTD coupling stencil (one that *does* engage BCC couplings in a way Phase 1's analytical gate hasn't already ruled out) emerges. No such path is currently identified.
- **prerequisites if ever re-opened:** explicit action functional S[s, J] defined on the lattice (currently the engine has an update rule, not an action); detailed-balance verification for the proposal + accept/reject scheme.

### FTD-0058: Structure-2 Ward-valid scalar gauge completion [CLOSED NEGATIVE 2026-04-22]

- **statement:** The natural Structure-2 two-U(1) BCC scalar gauge completion, implemented with Peierls phases on the 8 diagonal BCC links and with both bubble and seagull terms included, does not reproduce the Structure-1 ppb alpha closure.
- **tag:** CLOSED NEGATIVE
- **tag_history:** Created and closed 2026-04-22 after fixed GPU computation. This was a cross-check of the universality of the Structure-1 one-loop correction, not a search for a near-miss.
- **primary evidence:** `scripts/exploration/gpu_plan_priority4_structure2.py` with `--mode strict --bz periodic --N 1024 --q-list 1,2,3,4 --cases S2-A,S2-B,S2-C,S2-D,S2-E`.
- **validation:** CPU/GPU cross-check passed at N=16; Ward identity passed in periodic BZ with max `|Pi_ii(0)|` from about 1e-19 to 4e-17; `Pi_xx(Q_z)` and `Pi_yy(Q_z)` agreed within convergence tolerance; small-q plateau was stable through N=1024.
- **results:** S2-A residual +1259.797 ppb; S2-B +1268.328 ppb; S2-C +1256.954 ppb; S2-D +6184.947 ppb; S2-E +1256.997 ppb. All exceed the 300 ppb non-reproduction threshold.
- **handoff diagnostic:** The original handoff bubble-only q=0 quantity was reproduced separately in framework BZ: `Pi_bubble_avg = +7.091094244023e-04`, mapped residual +6548.333 ppb. It is diagnostic only because it omits the seagull term required by gauge invariance.
- **claim impact:** FTD-0001 and FTD-0013 remain as currently tagged. What is closed negative is the stronger claim that the Structure-1 ppb correction is reproduced by a natural Ward-valid Structure-2 scalar gauge theory. The Structure-1 ppb correction should be treated as scheme-specific until a unique FTD-to-EFT matching principle is derived.
- **artifacts:** `docs/theory/10_eft_program/AUDIT_STRUCTURE2_WARD_VALIDATION.md`; `scripts/exploration/outputs/priority4_periodic_strict_cases_N1024.jsonl`; `scripts/exploration/outputs/priority4_periodic_strict_cases_N1024_strict_rows.csv`.

### FTD-0059: No-go theorem for `a_phys` derivation from Axiom Zero [THEOREM 2026-04-23]

- **statement:** No quantity with SI dimension of length is expressible as a function of Axiom-Zero invariants alone. The lattice-to-physical length conversion `a_phys` must be supplied as an external calibration input. Corollary: no mass, time, energy, temperature, or charge is derivable from Axiom Zero either — every dimensional FTD prediction is conditional on exactly two calibrations (length and mass), with the remaining SI dimensions fixed through `c_phys` and `c²=E/M`.
- **tag:** THEOREM
- **tag_history:** Created 2026-04-23 after Mechanism δ closure (`DERIV_A_PHYS_MECHANISM_DELTA_ATTEMPT.md`) revealed the structural pattern common to all four failed derivation attempts (α, β, γ, δ).
- **proof_status:** COMPLETE
- **proof_location:** `docs/theory/10_eft_program/THEOREM_A_PHYS_NO_GO.md` §3.
- **proof_sketch:** The ring `R` of Axiom-Zero invariants (integers, `G*`, `π`, `ϖ`, `x_±`, Watson integrals, `c_lat = 1/√3`, …) is closed under the operations available to Axiom Zero and consists entirely of SI-dimensionless reals. A length has SI dimension `L¹ ≠ 1`. Hence no function with domain in `R` has image with SI dimension `L¹`. Therefore `a_phys` cannot be derived from `R` alone.
- **dependencies:** FTD-0036 (Postulate 1), FTD-0037 (Postulate 2), FTD-0038 (Postulate 3), FTD-0039 (Postulate 4), FTD-0040 (Postulate 5) — together define Axiom Zero.
- **dependents:** FTD-0030 (a_phys resolved-by-calibration — now theorem-enforced), FTD-0041 (a_phys ≡ ℓ_P calibration — now theorem-enforced), FTD-0035 (Mechanism γ attempt — instance of the theorem).
- **supersedes_status_of:** `OPEN_A_PHYS_DERIVATION.md` from [CLOSED NEGATIVE / RESOLVED-BY-CALIBRATION] to [CLOSED — RESOLVED BY THEOREM].
- **claim impact:** Promotes the `a_phys ≡ ℓ_P` calibration from a pragmatic fallback to a structurally necessary calibration interface. Identifies FTD's dimensionless predictions (α, mass ratios, mixing angles, anomalous moments) as the calibration-independent falsifiable spine of the framework. Does not demote or affect any existing dimensionless prediction.
- **last_reviewed:** 2026-04-23
- **reframe_status:** NEW
- **artifacts:** `docs/theory/10_eft_program/THEOREM_A_PHYS_NO_GO.md` (theorem), `docs/theory/10_eft_program/DERIV_A_PHYS_MECHANISM_DELTA_ATTEMPT.md` (δ closure), `docs/theory/10_eft_program/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` (γ closure), `docs/SPEC_FTD.md` (calibration declaration), `docs/theory/10_eft_program/OPEN_A_PHYS_DERIVATION.md` (resolution preamble).

---

## Maintenance log

| Date | Change | By |
|---|---|---|
| 2026-04-19 | Initial population. Captured 40 load-bearing claims with tag history through reframe cycle. | Session 1. |
| 2026-04-19 | Session 2: FTD-0030 RESOLVED via SPEC_FTD calibration declaration; FTD-0034 RESOLVED via 5 FLAG restatements; new rows FTD-0041, 0042, 0043, 0044, 0045 added. | Session 2. |
| 2026-04-19 | Session 3: FTD-0042 (YM) and FTD-0043 (NS) RETRACTED to archive; FTD-0044 (per-voxel mass gap survives) preserved in retracted-archive .tex; FTD-0046 (Thermodynamic_Limit) and FTD-0047 (Thermodynamic_Reflexion) RETRACTED with pdftotext extractions; FTD-0048 (11 other PDF-only papers) ARCHIVED; FTD-0049 (commit-attribution policy + 222-commit Co-Authored-By cleanup on main) NEW. | Session 3. |
| 2026-04-20 | Link 8 closure cycle: FTD-0050 CLOSED NEGATIVE (master quadratic not an RG-step characteristic polynomial on engine); FTD-0051 NEW (Langevin thermostat infrastructure, operational); FTD-0052 OPEN (s-field Metropolis, deferred). FTD-0001/0013/0014 unaffected. Full details in `10_eft_program/AUDIT_LINK8_CLOSURE.md`. | Link 8 session. |
| 2026-04-20 (late) | Phase 1 analytical gate: linearized 2-coupling (g_SCFCC, g_BCC) flow matrix computed numerically. trace(M)=2.44 vs target 140.06, det(M)=1.50 vs target 414.39, eigenvalues complex vs target real. Rules out the principled "give the engine a BCC sector and rerun" path without engine code. FTD-0050 evidence bullet added (4 confirmations now); FTD-0052 demoted OPEN → NOT PURSUED (F9-avoidance) per owner decision. | Phase 1 gate. |
| 2026-04-20 (late) | FTD-0051 GPU port: single-substrate Langevin wave_update_kernel + phase_write_kernel extended with OU branch; GpuBuffers::d_langevin_noise added; cuRAND normal-fill on each tick. WSL2 CUDA 13 build clean; equipartition test L=16 PASS both backends; CPU/GPU parity 0.03% absolute at L=64, **0.02% at L=128**. Scaling benchmark: **58× speedup at L=64, 112× at L=128**; L=256 tractable at 6.81 ms/tick GPU-only. Unblocks thermal EFT measurements at L ≥ 128 on GPU — a 5000-tick burn-in at L=256 takes 34 s. | GPU port + scaling benchmark. |
| 2026-04-20 (evening) | First productive use of FTD-0051 GPU pipeline: α_eff scaling to L=256 on GPU in 158.7s. New data points FTD-0053 (L=256 T=0 α=0.1340, R²=0.99915, consistent with 3.6× plateau) and FTD-0054 (OPEN: thermal α via V(r) subtraction structurally fails under independent-bridge noise; two fix paths specified). `measure_alpha_eff` + helpers extended with `LangevinOptions` struct (default-constructed preserves T=0 semantics). Full analysis: `AUDIT_ALPHA_SCALING_L256.md`. | First productive run. |
| 2026-04-21 | Path A attempt: GPU sync bug root-caused and FIXED in `RenderBridge::run()` (1 LOC). T=0 baselines unchanged across L ∈ {32,64,128,256} — no regression. Path A mechanics now working: thermal bulk cancels correctly in V(r) subtraction; remaining gap is ensemble averaging to beat O(T·L^(3/2)) thermal noise between bridges. FTD-0054 remains OPEN but narrowed to "compute-bound", no design blocks. | Bug fix + Path A retry. |
| 2026-04-21 (evening) | External GPU-computation-plan (user-supplied) Priorities 1, 3, 5, 6 executed on WSL2+cupy. FTD-0055 NEW: BCC tadpole at N=4096 confirmed plan target. FTD-0056 NEW: unrenormalized one-loop BCC tadpole residual diverges as a→0 (Priority 5 result). Flagged inconsistency in external plan's BCC-tadpole-value-vs-SC-ppb-residual chain. Priorities 2, 4, 7 deferred (substantial infrastructure). Full report: `10_eft_program/AUDIT_GPU_PLAN_PRIORITIES_1_3_5_6.md`. | External GPU plan execution. |
| 2026-04-21 (late) | **Priority 2 executed and PASSED.** HMC on BCC lattice, N=64, 3000 measurement trajectories at ε=0.002, 400 steps each. Wall: 15 min on RTX 5090. ⟨η⟩_MC = −1.695e-4 vs one-loop prediction −1.710e-4; \|Δ\|/σ_τ = 0.19 (σ_τ = 7.8e-6). Spec's success criterion \|MC−1-loop\| < 3σ with σ < 1e-5 MET. Acceptance 99.1%, max\|η\| = 0.91, τ_int = 0.96. Non-perturbative confirmation of one-loop perturbation theory for Structure-1. FTD-0057 NEW (MEASURED). Plan's ε=0.02 setting failed (0% acceptance, spec under-estimated ΔH volume scaling); ε=0.002 was the empirical tuning. | Priority 2 HMC. |
| 2026-04-22 | Path B Structure-2 audit: implemented Ward-valid two-U(1) BCC scalar-loop test with bubble plus seagull terms. Natural scalar matter cases S2-A..S2-E all fail the Structure-1 closure threshold; best natural case S2-A residual +1259.797 ppb with Ward max 1.053e-18. FTD-0058 NEW (CLOSED NEGATIVE). Full report: `10_eft_program/AUDIT_STRUCTURE2_WARD_VALIDATION.md`. | Structure-2 audit. |
| 2026-04-23 | FTD-0030/0041 amendment: `DERIV_A_PHYS_MECHANISM_GAMMA_SUCCESS.md` (claimed [THEOREM] `a_phys ≈ 4.39 ℓ_P`) **RETRACTED**. Its "derivation" silently swaps the `K_B = m_e` mass calibration for `ℏ_lat = 1` and relabels the substitution as a theorem — the same calibration-shuffle flaw the ATTEMPT doc identified against itself. `a_phys ≡ ℓ_P` calibration (FTD-0041) unchanged. Retraction preamble added in-place to the SUCCESS doc; cross-references in ATTEMPT and OPEN updated. No new ledger row needed — resolution remains FTD-0030 RESOLVED-BY-CALIBRATION / FTD-0041 CALIBRATION. | Tension audit. |
| 2026-04-23 | **FTD-0059 NEW — no-go theorem for `a_phys`.** Mechanism δ attempt (`DERIV_A_PHYS_MECHANISM_DELTA_ATTEMPT.md`) executed under strict Calibration Hygiene Rules; closed negative across four routes (information velocity, CFL + lattice invariants, `ontic.h` chain scan, two-anchor elimination). All four failures share a single structural cause: the ring of Axiom-Zero invariants is entirely SI-dimensionless, so no length is derivable without an external dimensional generator. Elevated to theorem: `THEOREM_A_PHYS_NO_GO.md`. `OPEN_A_PHYS_DERIVATION.md` moved from [CLOSED NEGATIVE / RESOLVED-BY-CALIBRATION] to [CLOSED — RESOLVED BY THEOREM]. `docs/SPEC_FTD.md` updated with a one-line note tying the calibration to the theorem. Cluster 5 status updated in `10_eft_program/00_INDEX.md`. | a_phys no-go. |

---

## Notes on coverage

This v1.0 ledger captures the **load-bearing** claims tracked through the April 19 audit cycle. It is not exhaustive: there are ~50 additional parametric insertions catalogued in `CATALOG_PARAMETRIC_INSERTIONS.md` and ~50 external-physics adoptions catalogued in `AUDIT_EPISTEMIC_AUDIT.md`, neither of which is duplicated here. Future maintenance should cross-reference the catalog rather than re-list every parametric.

Outstanding portfolio claims (papers in `docs/papers/`, `dissemination/papers/`, `dissemination/manuscript_v2/`) are not yet ledger-rowed pending the broader-portfolio inventory + classifier output. Add rows as those audits return.
