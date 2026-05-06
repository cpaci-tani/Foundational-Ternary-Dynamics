# INDEX · FTD-Native EFT Program

**Tag:** [REFERENCE]
**Date:** 2026-05-05 (Phase R1 of the FTD-EFT roadmap)
**Status:** [REFERENCE] — categorised inventory of every document in `docs/theory/10_eft_program/` with current epistemic tags + dependencies.
**Purpose:** Replace `00_INDEX.md` as the navigational entry point for the FTD-native EFT program. After the 2026-04-22 methodological pivot (QED-α projection closed-negative; FTD-native blocking EFT became the active program — see `OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`), the program accumulated ~105 documents across kinematic / dynamic / measurement / audit / decision / protocol / pre-registration / status / open / theorem / spec / analysis families. This INDEX makes that body legible without needing a depth-first scan.

> **How to use this document**:
> 1. To find the canonical specification for a topic → look in **§2 SPEC** first.
> 2. To find what's currently OPEN → look in **§9 OPEN** + STATUS rows in **§8**.
> 3. To find what was tried and rejected → look in **archive/** rows at the bottom of each category.
> 4. To find the current measurement campaign status → start at **STATUS_EFT_CHECKLIST.md** (§8).
>
> Dependency arrows in the "Key deps" column read X → Y as "this doc selects on top of, or audits, doc Y".

---

## §1 — Top of the stack: read these first

These are the 5 docs every reader of the FTD-native EFT program should read first, in this order:

1. **`SPEC_FTD_LAGRANGIAN.md`** (in `docs/theory/01_reference/`, not this directory) — canonical 3-term action + 6-engine decomposition. v3.2, [THEOREM] for action/EOM/limits + [SELECTION] for physical IDs.
2. **`MAP_LAGRANGIAN_TO_ENGINE.md`** (in `docs/theory/01_reference/`, sibling of (1)) — line-anchored cross-reference from spec terms to engine implementation. R1 deliverable.
3. **`SPEC_EFT_RECOVERY_PROGRAM.md`** — the original Phase 0–F pre-registration. Outcome: NULL on QED-α target; reframed in (4) and (5).
4. **`OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`** (2026-04-22) — methodological pivot: from QED-α projection to FTD-native blocking EFT.
5. **`STATUS_EFT_CHECKLIST.md`** — living tracker of checked / unchecked items.

After these, the directory's other 100 documents fill in details by category.

---

## §2 — SPEC (canonical specifications, [REFERENCE]/[SPEC])

These define the formal structure. They get read; they don't get measured.

| File | Date | Purpose | Key deps |
|---|---|---|---|
| `SPEC_EFT_RECOVERY_PROGRAM.md` | 2026-04-19 | Pre-registered Phases 0–F with falsifiable α_∞ plateau. Status: complete; outcome NULL relative to QED-α target. | Bridge gate 0 |
| `SPEC_FTD_EFT_BRIDGE_CONTRACT.md` | 2026-04-23 | Bridge contract gates 1–7. Phase deliverables and dependencies for any FTD-→-EFT route. | architectural foundation |
| `SPEC_FTD_NATIVE_BLOCKING_MAP.md` | 2026-04-23 | Formal blocking map specification; dual-cell representation; the canonical b=2 blocking transformation used throughout the program. | KINEMATIC foundation |
| `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` | 2026-04-22 | Native response tuple $(C_L^\text{FTD}, K_T^\text{FTD}, Z_j^\text{FTD}, c^\text{FTD})$ + flow under declared blocking. The post-pivot canonical EM specification. | FTD-native architecture |
| `SPEC_OPERATOR_BASIS.md` | 2026-04-19 | Operator basis reference; dimensionless and dimensional operators in the early projection program. | KINEMATIC, partially superseded |
| `SPEC_OPERATOR_BASIS_COMPLETE.md` | 2026-04-24 | Complete 10-operator basis (6 spatial + 4 reaction) for measurement; supersedes the §2 row above for active campaigns. | Gate 7 (native observables) |
| `SPEC_WILSON_DIRAC_FTD.md` | 2026-05-03 | Phase II.1 Wilson-Dirac specification; internal pre-registration milestone. | PREREG_PHASE_II_WILSON_DIRAC_G2 |

---

## §3 — KINEMATIC (fields, lattice geometry, indexing)

The static structure: what FTD's degrees of freedom are and how they map to the EFT.

| File | Date | Status | Purpose | Key deps |
|---|---|---|---|---|
| `DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md` | 2026-04-22 | [PARTIAL] | FTD does not need microscopic U(1); U(1) is effective description of projected flux. Foundational for the post-pivot ontology. | FTD native ontology |
| `DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md` | 2026-04-24 | [PARTIAL] | Native state↔flux mapping to EFT dictionary; scaling closure P1.1 (Bridge gate 2A). | EFT bridge layer 1 |

---

## §4 — DYNAMIC (flow, blocking, RG-step, history-kernel, response, transport)

The active structure. This is the single largest category — the FTD-native flow program is where the post-pivot work concentrates.

| File | Date | Status | Purpose | Key deps |
|---|---|---|---|---|
| `DERIV_FTD_NATIVE_BARE_FLOW.md` | 2026-04-23 | [PARTIAL] | Finite-volume blocking map; native bare $b=2$ flow for Gaussian flux energy. | FTD-native RG seed |
| `DERIV_FTD_NATIVE_COMPLETE_HISTORY_ACTION.md` | 2026-04-26 | [SELECTION/THEOREM] | Canonical microscopic history action $Z_u[\eta, h, a, \lambda_R]$; [THEOREM] reduction to linear G18 generator. **Central foundation for R3.** | FTD bridge gate 2B |
| `DERIV_FTD_NATIVE_CURRENT_FLOW.md` | 2026-04-23 | [PARTIAL] | Signed transport-current normalization canonical under native $b=2$ blocking; continuity equation closes. | Native gate 3 |
| `DERIV_FTD_NATIVE_ENGINE_HISTORY_FLOW.md` | 2026-04-23 | [PARTIAL] | Actual `RenderBridge::tick()` reaction histories connected to dual-cell continuity ledger. | engine integration |
| `DERIV_FTD_NATIVE_ENGINE_TRANSPORT_FLOW.md` | 2026-04-24 | [PARTIAL] | Real-engine Moore transport + multi-tick intervals connected to native dual-cell ledger; GPU-native movement. | bridge gate 3 |
| `DERIV_FTD_NATIVE_LANGEVIN_ENSEMBLE.md` | 2026-04-24 | [THEOREM/PARTIAL] | Stationary ensemble exists + equipartitioned [THEOREM]; full source-coupled generator $Z[J]$ under nonlinear dynamics [PARTIAL]. | FTD-0051 (Langevin GPU) |
| `DERIV_FTD_NATIVE_LINEAR_GENERATOR.md` | 2026-04-23 | [PARTIAL] | Minimal source-coupled generator for linear FTD sector; constrained flux-energy functional. | Bridge gate 2A |
| `DERIV_FTD_NATIVE_MULTISCALE_FLOW.md` | 2026-04-24 | [MEASURED] | Gaussian fixed point confirmed at $b\in\{1,2,4,8\}$; native flux-energy density under mixed nonlinear dynamics. | FTD-0070 |
| `DERIV_FTD_NATIVE_RESPONSE_FLOW.md` | 2026-04-23 | [PARTIAL] | Gaussian native response-flow tuple fixed for $b=2$; closes $C_L^\text{FTD}, K_T^\text{FTD}, Z_j^\text{FTD}, c^\text{FTD}$. | Native gates 5–6 |
| `DERIV_FTD_NATIVE_RESPONSE_TUPLE.md` | 2026-04-22 | [PARTIAL] | Native electrodynamics bare engine operator; first fixed probe of FTD-native response tuple. | FTD-native measurement |
| `DERIV_FTD_NATIVE_SCALE_FLOW.md` | 2026-04-22 | [THEOREM] | Scale-invariance theorem for native multiscale flow; blocking transformation self-similar. | Native RG closure |
| `DERIV_FTD_NATIVE_SOURCE_FLUX_COUPLING_CLOSURE.md` | 2026-04-22 | [CLOSED NEGATIVE]/[DEFINITION] | Unit-derivation attempt closed; source-flux coupling is DEFINITION, not measured quantity. | architecture |
| `DERIV_PROJECTED_EFT_MATTER_COUPLING.md` | 2026-04-22 | [PARTIAL] | Matter coupling under projection; EFT matter sector bridge. | Projected EFT route |
| `DERIV_BETA_FUNCTION_MEASURED.md` | 2026-04-19 | [MEASUREMENT] | Screened two-charge potential at all L; qualitative RG match (sign correct), quantitative gap 2–3× (later updated to ~80–160× in projection framing). | Phase 2 EFT campaign |

---

## §5 — MEASUREMENT (engine campaigns, mixing matrices, operator measurements)

Empirical results. Every entry corresponds to a CTest or campaign output.

| File | Date | Status | Purpose | Key deps |
|---|---|---|---|---|
| `ANALYSIS_EMERGENT_SPECTRUM.md` | 2026-04-27 | [PARTIAL] | Three-regime phase structure from generic ICs at L=32. | PROTOCOL_EMERGENT_PARTICLE_SPECTRUM |
| `ANALYSIS_EMERGENT_SPECTRUM_G1.md` | 2026-04-27 | [PARTIAL] | L=64 confirmation of deterministic cluster counts; 64× volume. | PROTOCOL_EMERGENT_SPECTRUM_G1 |
| `ANALYSIS_EMERGENT_SPECTRUM_G2.md` | 2026-04-28 | [PARTIAL] | L=128 confirmation of L-invariance across 64× volume range. | PROTOCOL_EMERGENT_SPECTRUM_G2 |
| `ANALYSIS_GATE_C_VS_L.md` | 2026-04-30 | [MEASURED] | Gate C non-monotone in L; sector decoupling L-independent at L∈{24,32,48,64}. **Important structural finding for R3c.** | THEOREM_BLOCKING_DIAGONAL_IDENTITIES |
| `ANALYSIS_LEMNISCATIC_REPLACEMENT.md` | 2026-04-27 | [PARTIAL] | Pre-reg outcome PASS-NONE; lemniscatic candidates fail horizon-area observable. | PROTOCOL_LEMNISCATIC_REPLACEMENT |
| `ANALYSIS_OFFDIAGONAL_ASYMMETRY.md` | 2026-04-30 | [STRUCTURAL] | Off-diagonal $J^4$ column at machine precision; operator-class decomposition. | MEASUREMENT_S_EFF_NONLINEAR_v1_partial |
| `ANALYSIS_TOPOLOGICAL_OBSERVABLES.md` | 2026-04-27 | [PARTIAL] | Four sub-experiments (Wilson loop, flux tube, monopole, instanton) match pre-reg grid. | PROTOCOL_TOPOLOGICAL_OBSERVABLES |
| `MEASUREMENT_GATE_D_T_PERTURBATION.md` | 2026-04-30 | [MEASURED] | Gate D PASS for theorem-grade diagonals; T-perturbation response validated. | PROTOCOL_S_EFF_NONLINEAR_v2_DESIGN |
| `MEASUREMENT_S_EFF_NONLINEAR_v1_partial.md` | 2026-04-29 | [PARTIAL] | Gates B & C PASS; Gate A subthreshold pending larger ensemble. | s_eff_nonlinear v1 campaign |
| `DERIV_DAY2_CAMPAIGN.md` | 2026-04-19 | [MEASUREMENT] | Four threads converge: matched-stencil Poisson 1e-8, EWSB sharp, spectroscopy clean. | Day 2 EFT measurements |
| `DERIV_DYNAMICAL_SM_EMERGENCE.md` | 2026-04-19 | [MEASUREMENT] | Phase 4 closures: 4A EWSB null, 4B three-gen null, 4C decay-chain null. | Phase 4 |
| `DERIV_GAP_CLOSURE.md` | 2026-04-19 | [MEASUREMENT] | EFT Recovery Phase 1–3 gaps closed (continuum limit, Lorentz recovery, Ward). | EFT framework |
| `DERIV_OPERATOR_SPECTRUM.md` | 2026-04-19 | [MEASUREMENT] | Operator spectrum classification from native RG flow. **R3b extends this.** | EFT Pillar 5 |
| `DERIV_SYMMETRY_RECOVERY.md` | 2026-04-19 | [MEASUREMENT] | Symmetry recovery under blocking; continuum symmetry structure. | EFT Pillar 2 |

---

## §6 — AUDIT (methodological audits, no-go closures, scheme checks)

| File | Date | Status | Purpose | Key deps |
|---|---|---|---|---|
| `AUDIT_ALPHA_EXTRACTION.md` | 2026-04-19 | [AUDIT] | Line-by-line audit of Phase F α extraction; resolves 1.8× residual as lattice-Coulomb geometry. | DERIV_EMERGENT_COULOMB_GEOMETRIC |
| `AUDIT_ALPHA_SCALING_L256.md` | 2026-04-20 | [MEASURED] | T=0 α_eff scaling clean to L=256; thermal α extraction untractable. | Langevin-GPU validation |
| `AUDIT_BCC_SUBLATTICE_SPECTRUM.md` | 2026-04-27 | [CLOSED NEGATIVE] | Mechanism C BCC ratio misses prediction 45.31 by ≥10σ at L∈{24,32,48}. | DERIV_MECHANISM_C_GC_BCC_BRIDGE |
| `AUDIT_CONTINUUM_LIMIT.md` | 2026-04-27 | [PARTIAL] | Operator-mixing matrix convergence at L∈{16,32,64}; cond(S), eigenvalue positivity, RG semigroup tracked. | FTD-0098/0099 |
| `AUDIT_EFT_BCC_ORTHOGONALITY.md` | 2026-04-20 | [AUDIT] | Link 8 BCC-orthogonality does not invalidate existing claims. | AUDIT_LINK8_CLOSURE |
| `AUDIT_FTD0105_MATH_CHECK.md` | 2026-04-27 | [AUDIT] | Math audit of lemniscatic-replacement; holds with two corrigenda. | ANALYSIS_LEMNISCATIC_REPLACEMENT |
| `AUDIT_GAUSSIANITY_v1_LARGE.md` | 2026-04-30 | [MEASURED] | Ensemble far from Gaussian (skew 1.1–26); Gate D requires full perturbation campaign. | PROTOCOL_S_EFF_NONLINEAR_v2_DESIGN |
| `AUDIT_GPU_PLAN_PRIORITIES_1_3_5_6.md` | 2026-04-21 | [MEASURED] | Priorities 1,2,3,5,6 executed; P1 BCC target reached, P2 one-loop match at 3σ. | GPU computation framework |
| `AUDIT_GSTAR_ASYMMETRY_SCAN.md` | 2026-04-27 | [HYPOTHESIS] | G* vs π asymmetry catalog; per-domain verdicts; look-elsewhere control mandatory. | PAPER_RATIO_AND_THE_ARROW |
| `AUDIT_HEEGNER_TOWER_RIGIDITY.md` | 2026-05-02 | [COMPLETE] | Canonical FTD case NOT unique under rational-multiplier criterion (1 vs 21 matches); CM Uniqueness bifurcates by methodology. | SPEC_ALGEBRAIC_SPINE Theorem 3 |
| `AUDIT_LEMNISCATE_ALPHA_RIGIDITY.md` | 2026-05-01 | [COMPLETE] | Canonical Cayley-Dickson 5-harmonic curve found in ~4.3% of natural alternatives; retagged [SELECTION]. | PREREG_LEMNISCATE_ALPHA_RIGIDITY |
| `AUDIT_LEMNISCATIC_SPHERE_REPLACEMENT.md` | 2026-04-27 | [HYPOTHESIS] | Structured catalog of Einstein + thermodynamics formulas; ϖ-native candidate matrix; engine arbitrates. | (awaiting PROTOCOL/ANALYSIS) |
| `AUDIT_LINK8_CLOSURE.md` | 2026-04-20 | [CLOSED NEGATIVE] | RG-flow interpretation of master quadratic closed; coupling flow does NOT satisfy characteristic equation (FTD-0050). | test_link8_kadanoff |
| `AUDIT_LORENTZ_ANISOTROPY.md` | 2026-04-25 | [MEASURED] | Anisotropy exponent $p=4.0008\pm0.0006$ ($R^2=1.0$); rotation-breaking operator dimension 7, Wilsonian-irrelevant. | EFT-Recovery Pillars 1, 3 |
| `AUDIT_OPERATOR_SPECTRUM.md` | 2026-04-25 | [PARTIAL] | Relevant/marginal/irrelevant classification NOT recovered in propagating-pulse; partial in confinement-era. | STATUS_EFT_CHECKLIST §5 |
| `AUDIT_STRUCTURE2_WARD_VALIDATION.md` | 2026-04-22 | [MEASURED] | Ward-valid Structure-2 two-U(1) BCC scalar loop does NOT reproduce Structure-1 closure. | gpu_plan_priority4 |
| `AUDIT_S_EFF_SMOKE_VALIDATION.md` | 2026-04-29 | [PARTIAL] | End-to-end smoke validation of s_eff campaign; all 10 components PASS; parameter-regime tuning pending. | campaign_s_eff_nonlinear |
| `AUDIT_WARD_IDENTITY.md` | 2026-04-25 | [MEASURED] | Ward residual: matched-stencil CG drives to ≤1e-8; SOR saturates at stencil-mismatch floor (~1%). | test_eft_ward_identity |
| `ANALYSIS_MASTER_QUADRATIC_EFT_OPEN_ITEMS.md` | 2026-04-24 | [ANALYSIS] | Correlates formal master-quadratic paper with native EFT checklist items. | PAPER_MASTER_QUADRATIC_FORMAL |

---

## §7 — PROTOCOL & PREREG (measurement-pipeline specs and pre-registrations)

PROTOCOL_*.md = measurement specifications. PREREG_*.md = hash-locked pre-registrations.

| File | Date | Status | Purpose | Key deps |
|---|---|---|---|---|
| `PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md` | 2026-04-26 | [PROTOCOL] | Mechanism C BCC spectrum falsifier (closed-negative). | AUDIT_BCC_SUBLATTICE_SPECTRUM |
| `PROTOCOL_BETA_MEASUREMENT.md` | 2026-04-25 | [PROTOCOL] | β-function extraction campaign. | DERIV_BETA_FUNCTION_MEASURED |
| `PROTOCOL_EMERGENT_PARTICLE_SPECTRUM.md` | 2026-04-27 | [PROTOCOL] | Engine phenomenology via generic ICs. | ANALYSIS_EMERGENT_SPECTRUM |
| `PROTOCOL_EMERGENT_SPECTRUM_G1.md` | 2026-04-27 | [PROTOCOL] | L=64 emergent-spectrum follow-up. | ANALYSIS_EMERGENT_SPECTRUM_G1 |
| `PROTOCOL_EMERGENT_SPECTRUM_G2.md` | 2026-04-28 | [PROTOCOL] | L=128 emergent-spectrum follow-up. | ANALYSIS_EMERGENT_SPECTRUM_G2 |
| `PROTOCOL_GSTAR_ASYMMETRY_SCAN.md` | 2026-04-27 | [PROTOCOL] | G*-native candidate predictions per domain. | AUDIT_GSTAR_ASYMMETRY_SCAN |
| `PROTOCOL_LEMNISCATIC_REPLACEMENT.md` | 2026-04-27 | [PROTOCOL] | Lemniscatic-replacement measurement. | ANALYSIS_LEMNISCATIC_REPLACEMENT |
| `PROTOCOL_OPERATOR_MIXING_MATRIX.md` | 2026-04-26 | [PROTOCOL] | 9×9 operator-mixing matrix measurement. | AUDIT_CONTINUUM_LIMIT |
| `PROTOCOL_S_EFF_NONLINEAR_CAMPAIGN.md` | 2026-04-29 | [DRAFT] | s_eff nonlinear campaign design; Gates A–D structure. Not hash-locked. | (R3.2 roadmap) |
| `PROTOCOL_S_EFF_NONLINEAR_v2_DESIGN.md` | 2026-04-30 | [DESIGN DRAFT] | s_eff v2 with Gaussianity shortcut framing. Not hash-locked. | AUDIT_GAUSSIANITY_v1_LARGE |
| `PROTOCOL_TOPOLOGICAL_OBSERVABLES.md` | 2026-04-27 | [PROTOCOL] | Topological observable campaign. | ANALYSIS_TOPOLOGICAL_OBSERVABLES |
| `PREREG_HEEGNER_TOWER_RIGIDITY.md` | 2026-05-02 | [PRE-REGISTRATION] | Heegner-tower 9-discriminant rigidity. | AUDIT_HEEGNER_TOWER_RIGIDITY |
| `PREREG_LEMNISCATE_ALPHA_RIGIDITY.md` | 2026-05-01 | [PRE-REGISTRATION] | Cayley-Dickson 5-harmonic curve rigidity. | AUDIT_LEMNISCATE_ALPHA_RIGIDITY |
| `PREREG_PHASE_I_NATIVE_COUPLING.md` | 2026-05-03 | [PRE-REGISTRATION] | Phase I native-coupling protocol (`preregister-phase-i-native-coupling-v1`). | FTD-native flow gates |
| `PREREG_PHASE_II_WILSON_DIRAC_G2.md` | 2026-05-03 | [PRE-REGISTRATION] | Phase II Wilson-Dirac G2 protocol (`preregister-phase-ii-wilson-dirac-g2-v1`). | SPEC_WILSON_DIRAC_FTD |

---

## §8 — STATUS (checklists, trackers)

| File | Date | Purpose |
|---|---|---|
| `STATUS_EFT_CHECKLIST.md` | 2026-04-26 | Living checklist of FTD-native EFT items. **Refreshed in Phase R1 against current state — see edit history.** |
| `STATUS_NONLINEAR_REGIME_2026-04-30.md` | 2026-04-30 | Consolidated handoff for nonlinear regime; phases, gates, next targets. |
| `REF_PREREGISTER_MANIFEST.md` | (date in file) | Single authoritative table of every pre-registered measurement. |
| `GAUSSIAN_EXPANSION_DATA_INVENTORY.md` | 2026-04-26 | Catalogue of Gaussian expansion data across campaigns. |

---

## §9 — OPEN (open-problem statements)

| File | Date | Status | Purpose |
|---|---|---|---|
| `OPEN_A_PHYS_DERIVATION.md` | 2026-04-23 | [CLOSED — RESOLVED] | Closed by no-go theorem; calibration only (see THEOREM_A_PHYS_NO_GO). |
| `OPEN_FTD_NATIVE_ACTION_OR_MEASURE.md` | 2026-04-23 | [OPEN] | Nonlinear effective action after blocking [OPEN]; linear form [SELECTION]. **Central R3 deliverable.** |
| `OPEN_FTD_TO_EFT_BRIDGE_STATUS.md` | 2026-04-22 | [OPEN] | Bridge status: gates 1–7 summary; 1–4 [PARTIAL], 5–6 [PARTIAL], 7 [OPEN]. |
| `OPEN_GC_FROM_FIRST_PRINCIPLES.md` | 2026-04-19 | [OPEN] | $g_c$ remains open after Mechanisms A–C closures; dimensionless origin unknown. |
| `OPEN_MU_FROM_LP_MISSING_ARROW.md` | 2026-04-26 | [OPEN] | $\mu$ from $\Lambda_P$: Riemann-Roch → ? → $g_N$; missing arrow. |

---

## §10 — THEOREM (completed theorems)

| File | Date | Purpose |
|---|---|---|
| `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` | 2026-04-19 | [THEOREM] $V(r)$ is geometric Coulomb with zero fine-structure; $\alpha_r$ match $R^2=1.0000$ at L=384. **Phase G resolution.** |
| `DERIV_FTD_NATIVE_SCALE_FLOW.md` | 2026-04-22 | [THEOREM] Scale-invariance for native multiscale flow. (Cross-listed in DYNAMIC.) |
| `DERIV_PARTITION_FUNCTION_L2.md` | 2026-04-19 | [THEOREM at L=2] + [OPEN finding] Partition function at L=2 + ultralocality structural finding. **Phase J.** |
| `THEOREM_A_PHYS_NO_GO.md` | 2026-04-23 | [THEOREM] No first-principles $a_\text{phys}$ from Axiom-Zero invariants. |
| `THEOREM_BLOCKING_DIAGONAL_IDENTITIES.md` | 2026-04-30 | [THEOREM/MEASURED] Blocking identities ($M_{JJ}=16$, $M_{J4}=256$ exact); engine smoothness measured at all L. |
| `THEOREM_MU_NO_GO_FTD0096.md` | 2026-04-28 | [THEOREM] FTD-0096 ($\mu$ via L-function) closed-negative; dimension obstruction. |

---

## §11 — DECISION (architectural choices and their resolutions)

| File | Date | Status | Purpose |
|---|---|---|---|
| `DERIV_A_PHYS_MECHANISM_DELTA_ATTEMPT.md` | 2026-04-23 | [CLOSED NEGATIVE] | $a_\text{phys}$ not derivable from information-density/CFL. |
| `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` | 2026-04-19 | [CLOSED NEGATIVE] | Gravitational $a_\text{phys}$ mechanism closed. |
| `DERIV_A_PHYS_MECHANISM_GAMMA_SUCCESS.md` | 2026-04-23 | **RETRACTED** | Calibration substitution, not derivation. |
| `DERIV_MECHANISM_B_GC_DERIVATION.md` | 2026-04-25 | [CLOSED NEGATIVE] | $g_c$ matching procedure circular. |
| `DERIV_MECHANISM_C_GC_BCC_BRIDGE.md` | 2026-04-26 | [CONJECTURE → CLOSED NEGATIVE] | BCC bridge to $g_c$; closed by AUDIT_BCC_SUBLATTICE_SPECTRUM. |

---

## §12 — Cross-cutting and exploratory

| File | Date | Status | Purpose |
|---|---|---|---|
| `EXPLR_SELF_DUAL_HALF_SHELL.md` | 2026-04-22 | [EXPLORATORY] | Self-dual half-shell structure investigation. |

---

## §13 — Archive (closed-negative routes)

These docs are preserved to prevent zombie re-emergence of closed approaches. Don't act on them; cite them only to explain "why this approach was rejected."

| File | Status | What it tried |
|---|---|---|
| `archive/.../DERIV_PROJECTED_DIRAC_OPERATOR_AND_CHARGE_NORMALIZATION.md` | [CLOSED NEGATIVE] | Projected Dirac normalization route (superseded by native EFT). |
| `archive/.../DERIV_PROJECTED_RESPONSE_EIGENVALUE_XPLUS_ATTEMPT.md` | [CLOSED NEGATIVE] | $x_+$ response eigenvalue (pre-pivot). |
| `archive/.../DERIV_PROJECTED_STIFFNESS_XPLUS_ATTEMPT.md` | [CLOSED NEGATIVE] | $x_+$ stiffness derivation. |
| `archive/.../DERIV_SOURCE_CURRENT_NORMALIZATION_XPLUS_ATTEMPT.md` | [CLOSED NEGATIVE] | $x_+$ source-current normalization. |
| `archive/.../OPEN_FTD_TO_EFT_MATCHING.md` | [CLOSED NEGATIVE] | FTD-→-EFT matching via projected lattice. |
| `archive/.../OPEN_PROJECTED_EFT_RENORMALIZATION_AND_ALPHA_OBSERVABLE.md` | [CLOSED NEGATIVE] | Projected EFT renormalization + α_observable. |

---

## §14 — Most recently updated (active in flux)

As of 2026-05-05:

- `SPEC_WILSON_DIRAC_FTD.md` (2026-05-03) — Phase II.1 spec
- `PREREG_PHASE_II_WILSON_DIRAC_G2.md` (2026-05-03) — Phase II pre-reg
- `PREREG_PHASE_I_NATIVE_COUPLING.md` (2026-05-03) — Phase I pre-reg
- `AUDIT_HEEGNER_TOWER_RIGIDITY.md` (2026-05-02) — rigidity scan complete
- `AUDIT_LEMNISCATE_ALPHA_RIGIDITY.md` (2026-05-01) — rigidity scan complete

---

## §15 — Statistics

- **Total documents**: 105 (99 main + 6 archived)
- **By category** (approximate; some docs span categories):
  - SPEC: 7
  - KINEMATIC: 2 (cross-listed in DYNAMIC)
  - DYNAMIC: 14
  - MEASUREMENT/ANALYSIS: 14
  - AUDIT: 19
  - PROTOCOL: 11
  - PREREG: 4
  - STATUS: 4
  - OPEN: 5
  - THEOREM: 6
  - DECISION: 5
  - Cross-cutting: 1
  - Archive: 6
- **Methodological pivot date**: 2026-04-22 (`OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`). Pre-pivot docs are largely [CLOSED NEGATIVE] (archive); post-pivot docs are largely [PARTIAL] (active program).

---

## §16 — Roadmap pointer

This INDEX is the R1 deliverable of the FTD-EFT roadmap. The roadmap's six phases are:

- **R1** (this doc + MAP_LAGRANGIAN_TO_ENGINE.md + STATUS_EFT_CHECKLIST refresh): inventory done.
- **R2**: production decisions (field basis, Gauss representation), stencil-variational derivation, dissipation closure, EM-regime unification.
- **R3** (central): explicit nonlinear blocked $S_\text{eff}[J,s]$ — closes `OPEN_FTD_NATIVE_ACTION_OR_MEASURE.md`.
- **R4**: FTD-internal flow analysis — extends `DERIV_BETA_FUNCTION_MEASURED.md` to $\beta(g, L)$ as a unified function.
- **R5**: Inter-scale formalisation (Scales 0→1, 1→2, 2→3, 3→4).
- **R6**: Synthesis manuscript `PAPER_FTD_NATIVE_EFT.tex`.

When a future doc supersedes one cited here, update this INDEX (search-and-replace by filename) and the corresponding row's epistemic tag.
