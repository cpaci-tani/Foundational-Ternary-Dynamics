# INDEX · FTD-Native EFT Program

**Tag:** [REFERENCE]
**Status:** [REFERENCE] — categorised inventory of `docs/theory/10_eft_program/`, with epistemic tags read from each doc's own header. Not all run-of-record analyses are listed below — notably `ANALYSIS_THERMAL_PHASE_MAP_v1` (FTD-0275), `ANALYSIS_GENESIS_CRITICALITY_v1`, `ANALYSIS_CLUSTER_ENERGY_SPECTROSCOPY_v1`, `ANALYSIS_HALO_FORCEDNESS_v1` (FTD-0300), `ANALYSIS_PROTON_STABILITY_v1` (FTD-0301), `ANALYSIS_ALPHA_D3_FORCED_ESCAPE_v1` (FTD-0284). The authoritative complete catalog is [`../META_INDEX.md`](../META_INDEX.md). Settled routes are archived — see **§ Archive**.
**Purpose:** Navigational entry point for the FTD-native EFT program. Completed-campaign and closed-route scaffolding docs are archived, and several `DERIV_FTD_NATIVE_*` husks are merged into consolidated docs; the archived material is summarised under **§ Archive** and indexed in full by the `RETROSPECTIVE`.

> **How to use this document**:
> 1. To orient from scratch → read the **Read first** list, then `RETROSPECTIVE_EFT_RECOVERY.md` for the whole-program narrative.
> 2. To find the canonical specification for a topic → look in **§1 SPEC**.
> 3. To find what is currently OPEN → look in **§4 OPEN** + the trackers in **§5 STATUS**.
> 4. To find current measurement-campaign status → start at **`STATUS_EFT_CHECKLIST.md`** (§5).
> 5. To find what was tried and rejected, or which completed campaign produced a number → see **§ Archive** and the retrospective's pointer index.
>
> Tags below are quoted from each doc's header. This is a navigation file: it describes docs, it does not change them.

---

## Read first

The methodological spine of the program, in reading order:

1. **`SPEC_EFT_RECOVERY_PROGRAM.md`** — the original Phase 0–F pre-registration. Outcome: NULL on the QED-α target.
2. **`OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`** — the methodological pivot from QED-α projection to FTD-native blocking EFT. The program's keystone honesty document.
3. **`SPEC_FTD_EFT_BRIDGE_CONTRACT.md`** — the 7-gate epistemic guardrail for any FTD→EFT route.
4. **`RETROSPECTIVE_EFT_RECOVERY.md`** — `[SYNTHESIS]` narrative roll-up of the whole program; ties the archived scaffolding back to the live survivors.
5. **`STATUS_EFT_CHECKLIST.md`** — living tracker of what is required for FTD to count as a native EFT.

---

## §1 — SPEC (canonical specifications)

These define the formal structure. They get read; they don't get measured.

| File | Tag | Purpose |
|---|---|---|
| [`SPEC_EFT_RECOVERY_PROGRAM.md`](scopes_and_specs/SPEC_EFT_RECOVERY_PROGRAM.md) | [REFERENCE] | Pre-registered Phases 0–F; complete, outcome NULL relative to the QED-α target. |
| [`SPEC_FTD_DYNAMICAL_SU3_HADRODYNAMICS.md`](scopes_and_specs/SPEC_FTD_DYNAMICAL_SU3_HADRODYNAMICS.md) | [THEOREM]/[SELECTION] | Post-pivot dynamical QCD: compact link connection variables, Langevin updates, and local voxel-gauge coupling stencils. |
| [`SPEC_FTD_EFT_BRIDGE_CONTRACT.md`](scopes_and_specs/SPEC_FTD_EFT_BRIDGE_CONTRACT.md) | [SELECTION] | The minimal 7-gate contract that lets FTD become a Wilsonian EFT without using physical α or SM targets as inputs. |
| [`SPEC_FTD_NATIVE_BLOCKING_MAP.md`](scopes_and_specs/SPEC_FTD_NATIVE_BLOCKING_MAP.md) | [SELECTION] | The fixed finite-volume coarse-graining map; canonical b=2 blocking transformation used throughout the program. |
| [`SPEC_FTD_NATIVE_ELECTRODYNAMICS.md`](scopes_and_specs/SPEC_FTD_NATIVE_ELECTRODYNAMICS.md) | [SELECTION] | Post-pivot native EM spec: native source/flux response theory replacing the QED-α derivation attempt. |
| [`SPEC_OPERATOR_BASIS_COMPLETE.md`](scopes_and_specs/SPEC_OPERATOR_BASIS_COMPLETE.md) | [THEOREM]/[SELECTION] | Gate-3 closure: all $O_h$- and $C$-invariant local operators in $(\rho,J,j,A)$ through dimension $D\le 6$. |
| [`SPEC_WILSON_DIRAC_FTD.md`](scopes_and_specs/SPEC_WILSON_DIRAC_FTD.md) | [SELECTION] | Branch-B Wilson-Dirac matter sector (native FTD fermion emergence is closed-negative per FTD-0073/0076). |

---

## §2 — THEOREM (completed theorems and no-go theorems)

| File | Tag | Purpose |
|---|---|---|
| [`DERIV_EMERGENT_COULOMB_GEOMETRIC.md`](derivations/DERIV_EMERGENT_COULOMB_GEOMETRIC.md) | [THEOREM] | $V(r)$ is geometric Coulomb with zero fine-structure content; closed-form, zero free parameters. The Phase-G resolution of the Phase-F α-plateau. |
| [`DERIV_PARTITION_FUNCTION_L2.md`](derivations/DERIV_PARTITION_FUNCTION_L2.md) | [THEOREM at L=2] | First explicit FTD partition-function computation; ultralocality structural finding. The Phase-J result. |
| [`THEOREM_A_PHYS_NO_GO.md`](derivations/THEOREM_A_PHYS_NO_GO.md) | [THEOREM] | No length $a_\text{phys}$ is derivable from Axiom Zero; calibration only (FTD-0059). |
| [`THEOREM_MU_NO_GO_FTD0096.md`](derivations/THEOREM_MU_NO_GO_FTD0096.md) | [THEOREM] / [CLOSED NEGATIVE] | Mass-unit $\mu$ not derivable from Axiom Zero; closes FTD-0096, confirms terminal [PARAMETRIC] for the L₂ identity. |
| [`THEOREM_BLOCKING_DIAGONAL_IDENTITIES.md`](derivations/THEOREM_BLOCKING_DIAGONAL_IDENTITIES.md) | [THEOREM]/[MEASURED] | Diagonal blocking identities $M_{JJ}=16$, $M_{J^4}=256$ exact; engine smoothness measured at all L. |
| [`FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md`](derivations/FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md) | [THEOREM] | Substrate-native operator calculus: trace/det of native operators lie in $\mathbb{Q}(G^*)$; Galois degree-2 obstruction forces W selection; K-BIND closed theorem-negative. |
| [`FOUND_MCT43_NATIVE_Z2_PERMANENCE.md`](derivations/FOUND_MCT43_NATIVE_Z2_PERMANENCE.md) | [DERIVED]+[SYNTHESIS] | FTD-0326: no FTD-native ℤ/2 (i-conjugation, ±ω, matter/antimatter, parity, time-reversal) can supply $\delta$ — all are $\mathbb{Q}$-entry, Galois-blind; the $\delta$-selection is a Galois orbit no operator performs. Verdict PERMANENT-EXTENDED; strengthens FTD-0242. Pre-reg: [`PREREG_MCT43_NATIVE_Z2_PERMANENCE_TEST_v1.md`](derivations/PREREG_MCT43_NATIVE_Z2_PERMANENCE_TEST_v1.md). |
| [`DERIV_DISCRETE_TICK_ENERGY_INVARIANT.md`](derivations/DERIV_DISCRETE_TICK_ENERGY_INVARIANT.md) | [THEOREM -- LINEAR TICK ENERGY, LOCAL CURRENT, AND ADDITIVE SOURCE WORK] / [MEASURED -- SUBVOXEL RECOIL ACCOUNTING] / [OPEN -- INTEGER TRANSPORT WORK] | Exact modified-energy invariant, finite-volume current, and additive source/work term for the `phase_read`/`phase_write` tick. FTD-0293 confirms global energy, FTD-0295 confirms local continuity, FTD-0296 confirms fixed-source work, and FTD-0297 measures subvoxel unlocked recoil as accounted by the same source/work law. Integer transport work remains open. |

---

## §3 — Native flow & bridge (DERIV)

The post-pivot FTD-native blocking-EFT program. The first two docs are the consolidation of 9 former `DERIV_FTD_NATIVE_*` husks.

| File | Tag | Purpose |
|---|---|---|
| [`DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md`](derivations/DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md) | [PARTIAL]/[THEOREM] | **Consolidated** (6 husks: response-tuple, linear-generator, bare/response/current/scale flow). Bare linear source/flux response tuple + its invariance under native b=2 blocking; bare Gaussian fixed point scale-invariant. |
| [`DERIV_FTD_NATIVE_NONLINEAR_FLOW.md`](derivations/DERIV_FTD_NATIVE_NONLINEAR_FLOW.md) | [THEOREM]/[MEASURED]/[PARTIAL] | **Consolidated** (3 husks: multiscale-flow, Langevin-ensemble, engine-transport-flow). Native RG flow into the nonlinear regime: Langevin stationary ensemble, Gaussian fixed point at $b\le 8$, engine-transport plumbing. |
| [`DERIV_BCC_ALGEBRAIC_READOUT.md`](derivations/DERIV_BCC_ALGEBRAIC_READOUT.md) | [DERIVED]/[PARTIAL] | ARC-B2: BCC algebraic readout and complex $V_{\text{complex}}$ observable; operationalizes the $\mathbb{Z}[i]$-module structure of $V_{\text{complex}}$. |
| [`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`](derivations/FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md) | [THEOREM]/[SELECTION]/[UNDERDETERMINED] | FTD-0230: BCC complex readout. Finite-block closed-negative **stands**; the infinite-aperture "FOUND-at-ARC-2" was **corrected → UNDERDETERMINED** (FTD-0232/0234/0235) — the `(Tr,Det)` operator assembly is unforced (W-CRIT-2), route-invariant per FTD-0242. |
| [`PREREG_ALPHA_READOUT_QUANTIZATION_v1.md`](preregistrations/PREREG_ALPHA_READOUT_QUANTIZATION_v1.md) | [PRE-REGISTRATION] | FTD-0231: Candidate C Quantization/Readout Rule pre-registration. |
| [`FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md`](derivations/FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md) | [THEOREM]/[SELECTION]/[UNDERDETERMINED] | FTD-0231: Candidate C Quantization/Readout Rule. The ARC-2 "FOUND" was **corrected → UNDERDETERMINED** (FTD-0232/0234/0235): genuine `[THEOREM]` charge-quantization stands, but the master-quadratic `(Tr,Det)` assembly is unforced (W-CRIT-2), route-invariant per FTD-0242. MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`. |
| [`PREREG_COLOR_CONFINEMENT_v1.md`](preregistrations/PREREG_COLOR_CONFINEMENT_v1.md) | [PRE-REGISTRATION — **VOID**] | Declared hash-lock tag was never created; no content SHA recorded; lock void (census audit §2, 2026-07-12). |
| [`FOUND_COLOR_CONFINEMENT_RESOLUTION.md`](derivations/FOUND_COLOR_CONFINEMENT_RESOLUTION.md) | **[RETRACTED 2026-07-12]** | ~~FTD-0217 FOUND verdict~~ **WITHDRAWN** — lock tag never existed, id never minted as a LEDGER row, and the claim contradicted the confinement obstruction of record (FTD-0025). Confinement stays [OPEN STRUCTURAL OBSTRUCTION]; retraction is prior art for the Front-D P5 priced-no-go (FTD-0384). |
| [`PREREG_STOCHASTIC_EFFECTIVE_ACTION_v1.md`](preregistrations/PREREG_STOCHASTIC_EFFECTIVE_ACTION_v1.md) | [PRE-REGISTRATION — **VOID**] | Declared hash-lock tag was never created; no content SHA recorded; lock void (census audit §2, 2026-07-12). |
| [`FOUND_STOCHASTIC_EFFECTIVE_ACTION_RESOLUTION.md`](derivations/FOUND_STOCHASTIC_EFFECTIVE_ACTION_RESOLUTION.md) | **[RETRACTED 2026-07-12]** | ~~FTD-0218 FOUND verdict~~ **WITHDRAWN** — lock tag never existed, id never minted (FTD-0384). Fresh LOCK-STD pre-registration required for any future claim. |
| [`DERIV_FTD_NATIVE_COMPLETE_HISTORY_ACTION.md`](derivations/DERIV_FTD_NATIVE_COMPLETE_HISTORY_ACTION.md) | [SELECTION]/[THEOREM]/[OPEN] | Canonical microscopic history action $Z_u[\eta,h,a,\lambda_R]$; [THEOREM] reduction to the linear G18 generator. Kept as-is in the consolidation. |
| [`DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md`](derivations/DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md) | [PARTIAL] | FTD needs no microscopic U(1); U(1) is an effective description of projected flux. Post-pivot ontology foundation. |
| [`DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md`](derivations/DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md) | [PARTIAL] | Native stateflux mapping to an EFT dictionary; scaling dimensions frozen under FTD-0059 (Gate-1 closure). |
| [`DERIV_FTD_NATIVE_EFFECTIVE_ACTION.md`](derivations/DERIV_FTD_NATIVE_EFFECTIVE_ACTION.md) | [THEOREM]/[MEASURED]/[SELECTION] | FTD-0264: Explicit polynomial effective action after spatial $b=2$ coarse-graining, sector decoupling, and measured Wilson coefficients. |
| [`RETROSPECTIVE_EFT_RECOVERY.md`](reports_and_audits/RETROSPECTIVE_EFT_RECOVERY.md) | [SYNTHESIS] | Narrative roll-up of the whole program; integrates existing claims at their canonical tags, promotes nothing. Pointer index back to live survivors and archived scaffolding. |
| [`SCOPE_ALPHA_READOUT_NEXT_STEPS.md`](scopes_and_specs/SCOPE_ALPHA_READOUT_NEXT_STEPS.md) | [SCOPING MEMO] | ARC-A1 track: outline of Candidate A, C, and B2 unattempted readout routes. |
| [`SCOPE_DET_IDENTITY_ATTACK_v1.md`](scopes_and_specs/SCOPE_DET_IDENTITY_ATTACK_v1.md) | [SCOPING / OPEN] | Scopes the single MC-T4.3 hinge (FTD-0235): is the readout determinant $16G^{*3}$ a *forced* J-twisted det_ζ identity or merely the asserted Vieta $\det=\operatorname{Tr}\cdot G^*$? States proof obligations A/B/C, the even-power wall, and the boundary-theorem payoff of the prior-favoured UNDERDETERMINED. No FOUND, no closure. Proposed LEDGER row FTD-0240. |
| [`SCOPE_GC_QUANTUM_PATH_INTEGRAL.md`](scopes_and_specs/SCOPE_GC_QUANTUM_PATH_INTEGRAL.md) | [SCOPING MEMO] | Mechanism B track (FTD-0231): formulation of Euclidean quantum partition function over the body-diagonal sub-stencil $\sigma_{\text{BCC}}$, discrete-to-continuum vacuum polarization, and non-circular Wilsonian matching. |
| [`ANALYSIS_NONLINEAR_BRIDGE_SWEEPS.md`](reports_and_audits/ANALYSIS_NONLINEAR_BRIDGE_SWEEPS.md) | [MEASUREMENT ANALYSIS] | F-D3 track: analysis of parameter sweeps D3a-D3d, leading to the final mechanism discrimination verdict among Mechanism $\alpha$, $\beta$, and $\gamma$. |
| [`FOUND_NO_4TH_GENERATION_NO_GO.md`](derivations/FOUND_NO_4TH_GENERATION_NO_GO.md) | [THEOREM] | FTD-0220: No 4th generation fermions no-go formalization result, establishing a FOUND verdict. |

---

## §4 — OPEN (open-problem statements)

| File | Tag | Purpose |
|---|---|---|
| [`EXPLR_FQCR_V1_5_NEXT_FRONTIERS.md`](EXPLR_FQCR_V1_5_NEXT_FRONTIERS.md) | [OPEN] / [EXPLORATION] | Next Frontiers for FQCR v1.5: Strong-field compliance audit, finite Schwinger coefficient, flavor/CKM. |
| [`OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`](scopes_and_specs/OPEN_FTD_TO_EFT_BRIDGE_STATUS.md) | [CLOSED NEGATIVE for QED α] / pivot | Where the QED-α bridge failed; defines the replacement target as native FTD source/flux physics. |
| [`OPEN_FTD_NATIVE_ACTION_OR_MEASURE.md`](scopes_and_specs/OPEN_FTD_NATIVE_ACTION_OR_MEASURE.md) | [PARTIAL] | Bridge Gate 2: linear generator derived, microscopic history measure selected; explicit nonlinear blocked effective action remains open. |
| [`OPEN_GC_FROM_FIRST_PRINCIPLES.md`](scopes_and_specs/OPEN_GC_FROM_FIRST_PRINCIPLES.md) | [OPEN] | $g_c$ from first principles after Mechanisms A–C closures; dimensionless origin unknown. |

---

## §5 — STATUS (checklists, trackers, manifests)

| File | Tag | Purpose |
|---|---|---|
| [`STATUS_EFT_CHECKLIST.md`](reports_and_audits/STATUS_EFT_CHECKLIST.md) | [PARTIAL] | Living checklist of what is required for FTD to count as a native EFT. Full nonlinear $S_\text{eff}$ remains the central open deliverable. |
| [`STATUS_NONLINEAR_REGIME_2026-04-30.md`](reports_and_audits/STATUS_NONLINEAR_REGIME_2026-04-30.md) | [REFERENCE] | Consolidated handoff for the nonlinear regime: phases, gates, next targets. |
| [`REF_PREREGISTER_MANIFEST.md`](REF_PREREGISTER_MANIFEST.md) | [REFERENCE] | Single authoritative table mapping every pre-registered measurement to its git tag, commit SHA, runner, output dir, and analysis doc. |
| [`ANALYSIS_ATOMIC_PARADIGM_CAMPAIGN_v1.md`](ANALYSIS_ATOMIC_PARADIGM_CAMPAIGN_v1.md) | [MIXED RESULT] | FTD-0280 through FTD-0283 atomic campaign: H/He replay confirmed, live engine hook confirmed, exchange/correlation wall confirmed, no-new-knob ladder fails Z²-scaling gate. |
| [`ANALYSIS_ALPHA_NO_ALPHA_ENGINE_PROBE_v1.md`](ANALYSIS_ALPHA_NO_ALPHA_ENGINE_PROBE_v1.md) | [INVALIDATED PROTOCOL] | FTD-0285 run of record: explicit coupling scales as expected, but the frozen absolute Phase-G gate fails for the finite live-engine protocol; no alpha claim promoted. |
| [`ANALYSIS_ALPHA_ESTIMATOR_VALIDATION_v1.md`](ANALYSIS_ALPHA_ESTIMATOR_VALIDATION_v1.md) | [MEASUREMENT ANALYSIS -- ENERGY FUNCTIONAL MISMATCH] | FTD-0286 v1 run of record: v1 gate `2rG_L(r)` mispaired with `½Σ|J|²`; matched ratio ≈0.501. Superseded for pairing by v2. |
| [`ANALYSIS_ALPHA_ESTIMATOR_VALIDATION_v2.md`](ANALYSIS_ALPHA_ESTIMATOR_VALIDATION_v2.md) | [MEASUREMENT ANALYSIS -- HALF ENERGY GATE CONFIRMED MATCHED] | FTD-0286 v2 run of record: paired gate `rG_L(r)` closes matched arm (max rel err 0.26%); production live-tick still fails (~12% stencil drift). No alpha claim promoted. |
| [`ANALYSIS_THOMSON_RECOIL_OBSERVATORY_v1.md`](ANALYSIS_THOMSON_RECOIL_OBSERVATORY_v1.md) | [OBSERVATION -- LINEAR SUPERPOSITION, NO MECHANICAL RECOIL] | FTD-0287 dashboard/campaign companion: locked-charge plane-wave observatory shows field-level linear superposition to machine precision; no nonlinear Thomson scattering, recoil, or alpha claim promoted. |
| [`ANALYSIS_THOMSON_UNLOCKED_RECOIL_v1.md`](ANALYSIS_THOMSON_UNLOCKED_RECOIL_v1.md) | [MEASUREMENT -- NATIVE EMERGENT FLUX-GRADIENT RECOIL] | FTD-0288 run of record: legacy force gives no recoil above gate; emergent flux-gradient force gives deterministic recoil; diagnostic qE responds transversely but remains imposed. |
| [`ANALYSIS_THOMSON_FLUX_EXCESS_v1.md`](ANALYSIS_THOMSON_FLUX_EXCESS_v1.md) | [MEASUREMENT -- NATIVE EMERGENT EXCESS FLUX DEFLECTION] | FTD-0289 run of record: locked and legacy baseline-subtracted residuals stay at machine noise; emergent flux-gradient path produces above-gate excess flux/wave residual; no alpha or cross-section claim promoted. |
| [`ANALYSIS_THOMSON_RADIATION_SHELLS_v1.md`](ANALYSIS_THOMSON_RADIATION_SHELLS_v1.md) | [MEASUREMENT -- NO BASELINE-SUBTRACTED OUTWARD POWER] | FTD-0290 run of record: locked and legacy residual shell power stay machine-zero; emergent shell trace remains below the frozen `1e-8` outward-power gate; no radiation/cross-section claim promoted. |
| [`ANALYSIS_THOMSON_NATIVE_CONTINUITY_v1.md`](ANALYSIS_THOMSON_NATIVE_CONTINUITY_v1.md) | [MEASUREMENT -- NATIVE GRAPH CONTINUITY CANDIDATE INVALIDATED] | FTD-0291 run of record: repeat and locked-linear residual controls pass, but the 18-neighbor graph-current candidate fails the free-wave balance gate (`max_abs_balance=0.0857`); no radiation/source/alpha claim promoted. |
| [`ANALYSIS_THOMSON_TICK_INVARIANT_v1.md`](ANALYSIS_THOMSON_TICK_INVARIANT_v1.md) | [MEASUREMENT -- NUMERIC GATE INVALIDATED] | FTD-0292 run of record: ordinary double accumulation showed small modified-energy drift (`2.11e-11`) but missed the frozen relative gate; no invariant claim promoted by v1. |
| [`ANALYSIS_THOMSON_TICK_INVARIANT_v2.md`](ANALYSIS_THOMSON_TICK_INVARIANT_v2.md) | [MEASUREMENT -- DISCRETE TICK MODIFIED ENERGY CONFIRMED] | FTD-0293 run of record: long-double Kahan accumulation confirms the source-free modified tick energy (`max_abs_modified_drift=7.11e-15`) while naive energy drifts by `0.7987`; no radiation/source/alpha claim promoted. |
| [`ANALYSIS_THOMSON_TICK_LOCAL_CONTINUITY_v1.md`](ANALYSIS_THOMSON_TICK_LOCAL_CONTINUITY_v1.md) | [MEASUREMENT -- NUMERIC RELATIVE-GATE INVALIDATED] | FTD-0294 run of record: source-free local tick balance closes absolutely (`max_abs_balance=4.66e-16`) but the exchange-relative denominator is degenerate (`max_rel_balance=1`); no local theorem promoted by v1. |
| [`ANALYSIS_THOMSON_TICK_LOCAL_CONTINUITY_v2.md`](ANALYSIS_THOMSON_TICK_LOCAL_CONTINUITY_v2.md) | [MEASUREMENT -- SOURCE-FREE LOCAL TICK CONTINUITY CONFIRMED] | FTD-0295 run of record: same density/current as v1, scale-relative gate passes (`max_scale_rel_balance=2.98e-16`); source-free finite-volume continuity confirmed, coupled source/work still open. |
| [`ANALYSIS_THOMSON_COUPLED_SOURCE_WORK_v1.md`](ANALYSIS_THOMSON_COUPLED_SOURCE_WORK_v1.md) | [MEASUREMENT -- FIXED-CHARGE SOURCE WORK CONTINUITY CONFIRMED] | FTD-0296 run of record: fixed charge, coupling on, movement off; additive source/work closes the finite-volume balance (`max_abs_balance=4.43e-16`, `max_scale_rel_balance=2.20e-16`). Moving recoil remains open. |
| [`ANALYSIS_THOMSON_MOVING_RECOIL_ACCOUNTING_v1.md`](ANALYSIS_THOMSON_MOVING_RECOIL_ACCOUNTING_v1.md) | [MEASUREMENT -- SUBVOXEL RECOIL ACCOUNTED BY ADDITIVE SOURCE WORK] | FTD-0297 run of record: native emergent unlocked recoil is deterministic (`extra_disp_mag=0.2102203195`) but has zero integer transport events; additive source/work still closes (`max_abs_balance=4.64e-16`). Integer transport work remains open. |

---

## §6 — PREREG (hash-locked pre-registrations)

Methodology committed before measurement. See `REF_PREREGISTER_MANIFEST.md` for tag/SHA provenance.

| File | Tag | Purpose |
|---|---|---|
| [`PREREG_ADVERSARIAL_LOOK_ELSEWHERE_v1.md`](preregistrations/PREREG_ADVERSARIAL_LOOK_ELSEWHERE_v1.md) | [PRE-REGISTRATION] | FTD-0189 adversarial look-elsewhere scan over an 18-constant basket FTD did not design. |
| [`PREREG_ALPHA_ARITHMETIC_GENERATIVITY_v1.md`](preregistrations/PREREG_ALPHA_ARITHMETIC_GENERATIVITY_v1.md) | [PRE-REGISTRATION] | FTD-0185 alpha arithmetic-generativity test (Test 4) — the Balmer-to-Bohr gate. |
| [`PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md`](preregistrations/PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md) | [PRE-REGISTRATION] | FTD-0230: BCC complex readout pre-registration locking design and verification parameters. |
| [`PREREG_ALPHA_READOUT_DETERMINANT_GRADING_v1.md`](preregistrations/PREREG_ALPHA_READOUT_DETERMINANT_GRADING_v1.md) | [PRE-REGISTRATION] | FTD-0233: Determinant Grading Pre-Registration locking design and verification. |
| [`PREREG_ALPHA_READOUT_ODD_PERIOD_v1.md`](preregistrations/PREREG_ALPHA_READOUT_ODD_PERIOD_v1.md) | [PRE-REGISTRATION] | FTD-0234: J-twisted det_ζ ratio odd-period Pre-Registration locking design and verification. |
| [`PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md`](preregistrations/PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md) | [PRE-REGISTRATION] | FTD-0235: detdet_ζ identity Pre-Registration locking design and verification. |
| [`PREREG_ALPHA_DYNAMICAL_READOUT_v1.md`](preregistrations/PREREG_ALPHA_DYNAMICAL_READOUT_v1.md) | [PRE-REGISTRATION -- LOCKED] | FTD-0284: Alpha dynamical readout discriminator after FTD-0242/0244; freezes native-vs-Postulate-W outcomes before any no-alpha-input engine coupling measurement. |
| [`PREREG_ALPHA_NO_ALPHA_ENGINE_PROBE_v1.md`](preregistrations/PREREG_ALPHA_NO_ALPHA_ENGINE_PROBE_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0285: fixed live-engine no-alpha probe for FTD-0284; run invalidated the finite-protocol absolute gate. See `ANALYSIS_ALPHA_NO_ALPHA_ENGINE_PROBE_v1.md`. |
| [`PREREG_ALPHA_ESTIMATOR_VALIDATION_v1.md`](archive/superseded/PREREG_ALPHA_ESTIMATOR_VALIDATION_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0286 v1: estimator-validation with legacy `2rG_L(r)` gate. Result: `ENERGY_FUNCTIONAL_MISMATCH` (pairing error; see v2). |
| [`PREREG_ALPHA_ESTIMATOR_VALIDATION_v2.md`](preregistrations/PREREG_ALPHA_ESTIMATOR_VALIDATION_v2.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0286 v2: half-energy gate pairing (`rG_L(r)` vs `½Σ|J|²`). Result: `HALF_ENERGY_GATE_CONFIRMED_MATCHED`. |
| [`PREREG_THOMSON_UNLOCKED_RECOIL_v1.md`](preregistrations/PREREG_THOMSON_UNLOCKED_RECOIL_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0288: unlocked-charge recoil discriminator after FTD-0287. Run verdict: `NATIVE_EMERGENT_FLUX_GRADIENT_RECOIL_DETECTED`; no alpha or cross-section claim. |
| [`PREREG_THOMSON_FLUX_EXCESS_v1.md`](preregistrations/PREREG_THOMSON_FLUX_EXCESS_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0289: baseline-subtracted flux-excess discriminator after FTD-0288. Run verdict: `NATIVE_EMERGENT_EXCESS_FLUX_DEFLECTION_DETECTED`; no alpha or cross-section claim. |
| [`PREREG_THOMSON_RADIATION_SHELLS_v1.md`](preregistrations/PREREG_THOMSON_RADIATION_SHELLS_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0290: residual-field radiation shell meter after FTD-0289. Run verdict: `NO_BASELINE_SUBTRACTED_OUTWARD_POWER`; no alpha or cross-section claim. |
| [`PREREG_THOMSON_NATIVE_CONTINUITY_v1.md`](preregistrations/PREREG_THOMSON_NATIVE_CONTINUITY_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0291: native graph finite-volume continuity meter after FTD-0290. Run verdict: `NATIVE_GRAPH_CONTINUITY_CANDIDATE_INVALIDATED`; no alpha or cross-section claim. |
| [`PREREG_THOMSON_TICK_INVARIANT_v1.md`](preregistrations/PREREG_THOMSON_TICK_INVARIANT_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0292: source-free discrete tick invariant v1 after FTD-0291. Run verdict: `DISCRETE_TICK_INVARIANT_INVALIDATED` due relative numeric gate; no alpha or cross-section claim. |
| [`PREREG_THOMSON_TICK_INVARIANT_v2.md`](preregistrations/PREREG_THOMSON_TICK_INVARIANT_v2.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0293: precision-controlled source-free discrete tick invariant v2. Run verdict: `DISCRETE_TICK_MODIFIED_ENERGY_CONFIRMED`; no alpha or cross-section claim. |
| [`PREREG_THOMSON_TICK_LOCAL_CONTINUITY_v1.md`](preregistrations/PREREG_THOMSON_TICK_LOCAL_CONTINUITY_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0294: source-free local tick continuity v1. Run verdict: `SOURCE_FREE_LOCAL_TICK_CONTINUITY_INVALIDATED` due exchange-relative numeric gate; no alpha or cross-section claim. |
| [`PREREG_THOMSON_TICK_LOCAL_CONTINUITY_v2.md`](preregistrations/PREREG_THOMSON_TICK_LOCAL_CONTINUITY_v2.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0295: scale-relative source-free local tick continuity v2. Run verdict: `SOURCE_FREE_LOCAL_TICK_CONTINUITY_CONFIRMED`; no alpha or cross-section claim. |
| [`PREREG_THOMSON_COUPLED_SOURCE_WORK_v1.md`](preregistrations/PREREG_THOMSON_COUPLED_SOURCE_WORK_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0296: fixed-charge coupled source/work continuity. Run verdict: `FIXED_CHARGE_SOURCE_WORK_CONTINUITY_CONFIRMED`; no alpha or cross-section claim. |
| [`PREREG_THOMSON_MOVING_RECOIL_ACCOUNTING_v1.md`](preregistrations/PREREG_THOMSON_MOVING_RECOIL_ACCOUNTING_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0297: unlocked moving-recoil source/work accounting. Run verdict: `SUBVOXEL_RECOIL_ACCOUNTED_BY_ADDITIVE_SOURCE_WORK`; no alpha or cross-section claim. |
| [`PREREG_ATOMIC_SECTOR_HARDENING_v1.md`](preregistrations/PREREG_ATOMIC_SECTOR_HARDENING_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0280: atomic-sector replay/manifest hardening for FTD-0278/0279; run confirmed replay. See `ANALYSIS_ATOMIC_PARADIGM_CAMPAIGN_v1.md`. |
| [`PREREG_DB_CLOCK_COULOMB_SPECTROSCOPY_v1.md`](preregistrations/PREREG_DB_CLOCK_COULOMB_SPECTROSCOPY_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0281: engine-native live Coulomb clock hook; hook smoke confirmed, FFT spectroscopy verdict still downstream. |
| [`PREREG_ATOMIC_EXCHANGE_CORRELATION_WALL_v1.md`](preregistrations/PREREG_ATOMIC_EXCHANGE_CORRELATION_WALL_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0282: fixed-import negative boundary confirmed for exchange, ortho/para splitting, and correlation under I1+I2+I3. |
| [`PREREG_ATOMIC_NO_NEW_KNOB_LADDER_v1.md`](preregistrations/PREREG_ATOMIC_NO_NEW_KNOB_LADDER_v1.md) | [PRE-REGISTRATION -- LOCKED/RUN] | FTD-0283: fixed-cell no-new-knob ion ladder; run failed the frozen Z²-scaling gate. |
| [`PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md`](preregistrations/PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md) | [PRE-REGISTRATION] | FTD-0143 uniqueness scan of the FQCR Model IV $(4,6;3,2)$ exponent quadruple. |
| [`PREREG_FTD_0110_NONLINEAR_BRIDGE_v1.md`](preregistrations/PREREG_FTD_0110_NONLINEAR_BRIDGE_v1.md) | [PRE-REGISTRATION] | FTD-0215 nonlinear bridge coordinated parameters sweeps and active partitioning (F-D3). |
| [`PREREG_GENESIS_COUNTING_v1.md`](preregistrations/PREREG_GENESIS_COUNTING_v1.md) | [PRE-REGISTRATION -- LOCAL HASH-LOCK/RUN] | FTD-0277: collective-coordinate genesis-counting model v1 for the current-stack N(A) law. Run verdict: `COUNTING_MODEL_V1_CLOSED_NEGATIVE`; see [`ANALYSIS_GENESIS_COUNTING_v1.md`](../03_derivations/archive/closed_negative/ANALYSIS_GENESIS_COUNTING_v1.md). |
| [`PREREG_NO_4TH_GENERATION_NO_GO_v1.md`](preregistrations/PREREG_NO_4TH_GENERATION_NO_GO_v1.md) | [PRE-REGISTRATION] | FTD-0220: No 4th generation fermions no-go formalization pre-registration locking design and verification parameters. |
| [`PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md`](archive/superseded/PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md) | [PRE-REGISTRATION] | FTD-0186 boundary-theorem Stage 1: the structural/dynamical discriminator. |

---

## §7 — Frontier 4: emergent graviton substrate mode

The graviton-census trio — does the FTD substrate carry an emergent massless spin-2 mode?

| File | Tag | Purpose |
|---|---|---|
| [`PREREG_GRAVITON_SUBSTRATE_MODE_v1.md`](archive/superseded/PREREG_GRAVITON_SUBSTRATE_MODE_v1.md) | [PRE-REGISTRATION] | Frontier 4 Step 4a v1; locks the hypothesis and decision criteria. Retained as provenance. |
| [`PREREG_GRAVITON_SUBSTRATE_MODE_v2.md`](preregistrations/PREREG_GRAVITON_SUBSTRATE_MODE_v2.md) | [PRE-REGISTRATION] | Step 4a-ii v2 — supersedes v1 §5/§8; locks the decisive engine campaign. |
| [`REPORT_GRAVITON_SUBSTRATE_MODE.md`](reports_and_audits/REPORT_GRAVITON_SUBSTRATE_MODE.md) | [MEASUREMENT REPORT] | Canonical measurement registered by v2; Outcome verdict applied against PREREG v2 §6/§7. **FTD-0193** (renumbered from FTD-0190 to resolve a collision with the Q10 finite-neutral-lock FTD-0190). |

---

## §8 — AUDIT (load-bearing audits)

| File | Tag | Purpose |
|---|---|---|
| [`AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md`](../07_assessment/audits/AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md) | [AUDIT] | FTD-0231: Charge quantization audit and exact QED-vs-native normalization boundary integrity. |
| [`AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md`](../07_assessment/audits/AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md) | [AUDIT + CORRECTION] | FTD-0232: Independent review of the ARC-C1/B2 overclaim, downgrading to UNDERDETERMINED. |
| [`AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md`](archive/closed_negative/AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md) | [CLOSED NEGATIVE] | FTD-0233: Parity no-go audit proving odd lemniscatic period unreachable from frozen even-degree ring. |
| [`AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md`](../07_assessment/audits/AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md) | [UNDERDETERMINED] | FTD-0234: Audit of the J-twisted det_ζ ratio = G* as a clean odd-degree period source. |
| [`AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md`](../07_assessment/audits/AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md) | [UNDERDETERMINED] | FTD-0235: detdet_ζ operator identity audit showing master-quadratic Vieta dependencies are unforced. |
| [`AUDIT_ALPHA_EXTRACTION.md`](../07_assessment/audits/AUDIT_ALPHA_EXTRACTION.md) | [AUDIT] | Line-by-line audit of the Phase-F α extraction pipeline; the "3.6× α_ref" claim, resolved by Phase G as a category error. |
| [`AUDIT_HEEGNER_TOWER_RIGIDITY.md`](../07_assessment/audits/AUDIT_HEEGNER_TOWER_RIGIDITY.md) | [COMPLETE] | 9-Heegner CM-tower master-quadratic rigidity scan; CM-uniqueness bifurcates by methodology. Pre-reg now in `archive/campaign_complete/`. |
| [`AUDIT_LEMNISCATE_ALPHA_RIGIDITY.md`](../07_assessment/audits/AUDIT_LEMNISCATE_ALPHA_RIGIDITY.md) | [COMPLETE] | Lemniscate-alpha rigidity scan; canonical 5-harmonic curve found in ~4.3% of natural alternatives, retagged [SELECTION]. Pre-reg now in `archive/campaign_complete/`. |

---

## § Archive

Archived material is preserved for provenance — cite it to explain "why this route was rejected" or "which completed campaign produced this number"; don't act on it as live work. The full pointer index lives in **`RETROSPECTIVE_EFT_RECOVERY.md`**, which ties the archived scaffolding into the program narrative. The archive also holds settled MC-T4.3 route docs ([EMPIRICAL/BOUNDARY/K2_REGULATOR]) under `closed_negative/`, a [W5_CONFIRMATION] under `resolved/`, and a `superseded/` subdir ([THERMAL_IGNITION, GRAVITON_SUBSTRATE_MODE_v1, STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1, ALPHA_ESTIMATOR_VALIDATION_v1]):

| Subdir | Count | Description |
|---|---|---|
| [`archive/closed_negative/`](archive/closed_negative/) | 21 | Closed-negative routes — pre-pivot projected-α derivations, the three $g_c$ mechanism attempts, $a_\text{phys}$ mechanism attempts, the mass-unit $\mu$ missing arrow, and superseded protocols/pre-regs. |
| [`archive/campaign_complete/`](archive/campaign_complete/) | 42 | Completed-campaign scaffolding — protocol/pre-reg/analysis/audit triplets for campaigns that have run and recorded their result (emergent spectrum, topological observables, operator-mixing, s_eff nonlinear, Lorentz/Ward, rigidity scans, decisions). |
| [`archive/resolved/`](archive/resolved/) | 1 | `OPEN_A_PHYS_DERIVATION.md` — the $a_\text{phys}$ open problem, closed by `THEOREM_A_PHYS_NO_GO.md`. |
| [`archive/retracted/`](archive/retracted/) | 1 | `DERIV_A_PHYS_MECHANISM_GAMMA_SUCCESS.md` — a calibration-substitution false positive, retracted. |
| [`archive/phase_0_f_campaign/`](archive/phase_0_f_campaign/) | 6 | The original Phase 0–F measurement campaign docs (β-function, day-2, gap-closure, operator-spectrum, symmetry-recovery, dynamical-SM). |

`archive/ARCH_00_INDEX_2026-04-27.md` is the superseded predecessor of this INDEX.

---

**Live document count:** 62 top-level docs (this INDEX excluded) — 6 SPEC, 6 THEOREM, 12 native-flow/bridge DERIV incl. the retrospective, 3 OPEN, 6 STATUS/ANALYSIS, 18 PREREG, 3 Frontier-4 graviton, 8 AUDIT.
