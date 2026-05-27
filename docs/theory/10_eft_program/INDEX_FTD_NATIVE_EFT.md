# INDEX · FTD-Native EFT Program

**Tag:** [REFERENCE]
**Date:** 2026-05-22 (rewritten after the cluster consolidation)
**Status:** [REFERENCE] — categorised inventory of every live document in `docs/theory/10_eft_program/`, with current epistemic tags read from each doc's own header.
**Purpose:** Navigational entry point for the FTD-native EFT program. The 2026-05-22 consolidation took the cluster from 89 top-level docs to 34: 48 completed-campaign and closed-route scaffolding docs were archived, 9 `DERIV_FTD_NATIVE_*` husks were merged into 2 docs, and a narrative `RETROSPECTIVE` was added. This INDEX reflects that current state; the archived material is summarised under **§ Archive** and indexed in full by the retrospective.

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
2. **`OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`** — the 2026-04-22 methodological pivot from QED-α projection to FTD-native blocking EFT. The program's keystone honesty document.
3. **`SPEC_FTD_EFT_BRIDGE_CONTRACT.md`** — the 7-gate epistemic guardrail for any FTD→EFT route.
4. **`RETROSPECTIVE_EFT_RECOVERY.md`** — `[SYNTHESIS]` narrative roll-up of the whole April–May 2026 program; ties the archived scaffolding back to the live survivors.
5. **`STATUS_EFT_CHECKLIST.md`** — living tracker of what is required for FTD to count as a native EFT.

---

## §1 — SPEC (canonical specifications)

These define the formal structure. They get read; they don't get measured.

| File | Tag | Purpose |
|---|---|---|
| [`SPEC_EFT_RECOVERY_PROGRAM.md`](SPEC_EFT_RECOVERY_PROGRAM.md) | [REFERENCE] | Pre-registered Phases 0–F; complete, outcome NULL relative to the QED-α target. |
| [`SPEC_FTD_EFT_BRIDGE_CONTRACT.md`](SPEC_FTD_EFT_BRIDGE_CONTRACT.md) | [SELECTION] | The minimal 7-gate contract that lets FTD become a Wilsonian EFT without using physical α or SM targets as inputs. |
| [`SPEC_FTD_NATIVE_BLOCKING_MAP.md`](SPEC_FTD_NATIVE_BLOCKING_MAP.md) | [SELECTION] | The fixed finite-volume coarse-graining map; canonical b=2 blocking transformation used throughout the program. |
| [`SPEC_FTD_NATIVE_ELECTRODYNAMICS.md`](SPEC_FTD_NATIVE_ELECTRODYNAMICS.md) | [SELECTION] | Post-pivot native EM spec: native source/flux response theory replacing the QED-α derivation attempt. |
| [`SPEC_OPERATOR_BASIS_COMPLETE.md`](SPEC_OPERATOR_BASIS_COMPLETE.md) | [THEOREM]/[SELECTION] | Gate-3 closure: all $O_h$- and $C$-invariant local operators in $(\rho,J,j,A)$ through dimension $D\le 6$. |
| [`SPEC_WILSON_DIRAC_FTD.md`](SPEC_WILSON_DIRAC_FTD.md) | [SELECTION] | Branch-B Wilson-Dirac matter sector (native FTD fermion emergence is closed-negative per FTD-0073/0076). |

---

## §2 — THEOREM (completed theorems and no-go theorems)

| File | Tag | Purpose |
|---|---|---|
| [`DERIV_EMERGENT_COULOMB_GEOMETRIC.md`](DERIV_EMERGENT_COULOMB_GEOMETRIC.md) | [THEOREM] | $V(r)$ is geometric Coulomb with zero fine-structure content; closed-form, zero free parameters. The Phase-G resolution of the Phase-F α-plateau. |
| [`DERIV_PARTITION_FUNCTION_L2.md`](DERIV_PARTITION_FUNCTION_L2.md) | [THEOREM at L=2] | First explicit FTD partition-function computation; ultralocality structural finding. The Phase-J result. |
| [`THEOREM_A_PHYS_NO_GO.md`](THEOREM_A_PHYS_NO_GO.md) | [THEOREM] | No length $a_\text{phys}$ is derivable from Axiom Zero; calibration only (FTD-0059). |
| [`THEOREM_MU_NO_GO_FTD0096.md`](THEOREM_MU_NO_GO_FTD0096.md) | [THEOREM] / [CLOSED NEGATIVE] | Mass-unit $\mu$ not derivable from Axiom Zero; closes FTD-0096, confirms terminal [PARAMETRIC] for the L₂ identity. |
| [`THEOREM_BLOCKING_DIAGONAL_IDENTITIES.md`](THEOREM_BLOCKING_DIAGONAL_IDENTITIES.md) | [THEOREM]/[MEASURED] | Diagonal blocking identities $M_{JJ}=16$, $M_{J^4}=256$ exact; engine smoothness measured at all L. |

---

## §3 — Native flow & bridge (DERIV)

The post-pivot FTD-native blocking-EFT program. The first two docs are the 2026-05-22 consolidation of 9 former `DERIV_FTD_NATIVE_*` husks.

| File | Tag | Purpose |
|---|---|---|
| [`DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md`](DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md) | [PARTIAL]/[THEOREM] | **Consolidated** (6 husks: response-tuple, linear-generator, bare/response/current/scale flow). Bare linear source/flux response tuple + its invariance under native b=2 blocking; bare Gaussian fixed point scale-invariant. |
| [`DERIV_FTD_NATIVE_NONLINEAR_FLOW.md`](DERIV_FTD_NATIVE_NONLINEAR_FLOW.md) | [THEOREM]/[MEASURED]/[PARTIAL] | **Consolidated** (3 husks: multiscale-flow, Langevin-ensemble, engine-transport-flow). Native RG flow into the nonlinear regime: Langevin stationary ensemble, Gaussian fixed point at $b\le 8$, engine-transport plumbing. |
| [`DERIV_BCC_ALGEBRAIC_READOUT.md`](DERIV_BCC_ALGEBRAIC_READOUT.md) | [DERIVED]/[PARTIAL] | ARC-B2: BCC algebraic readout and complex $V_{\text{complex}}$ observable; operationalizes the $\mathbb{Z}[i]$-module structure of $V_{\text{complex}}$. |
| [`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`](FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md) | [THEOREM]/[SELECTION] | FTD-0215: BCC complex readout resolution; proves finite-block closed-negative and infinite-aperture ARC-2 level found verdicts. |
| [`PREREG_ALPHA_READOUT_QUANTIZATION_v1.md`](PREREG_ALPHA_READOUT_QUANTIZATION_v1.md) | [PRE-REGISTRATION] | FTD-0216: Candidate C Quantization/Readout Rule pre-registration. |
| [`FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md`](FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md) | [THEOREM]/[SELECTION] | FTD-0216: Candidate C Quantization/Readout Rule resolution, establishing a FOUND verdict at the ARC-2 level. |
| [`PREREG_COLOR_CONFINEMENT_v1.md`](PREREG_COLOR_CONFINEMENT_v1.md) | [PRE-REGISTRATION] | FTD-0217: Color Confinement Substrate Derivation pre-registration. |
| [`FOUND_COLOR_CONFINEMENT_RESOLUTION.md`](FOUND_COLOR_CONFINEMENT_RESOLUTION.md) | [THEOREM]/[SELECTION] | FTD-0217: Color Confinement Substrate Derivation resolution, establishing a FOUND verdict. |
| [`PREREG_STOCHASTIC_EFFECTIVE_ACTION_v1.md`](PREREG_STOCHASTIC_EFFECTIVE_ACTION_v1.md) | [PRE-REGISTRATION] | FTD-0218: Stochastic Effective Action pre-registration. |
| [`FOUND_STOCHASTIC_EFFECTIVE_ACTION_RESOLUTION.md`](FOUND_STOCHASTIC_EFFECTIVE_ACTION_RESOLUTION.md) | [THEOREM]/[SELECTION] | FTD-0218: Langevin noise integration and Stochastic Effective Action derivation resolution, establishing a FOUND verdict. |
| [`DERIV_FTD_NATIVE_COMPLETE_HISTORY_ACTION.md`](DERIV_FTD_NATIVE_COMPLETE_HISTORY_ACTION.md) | [SELECTION]/[THEOREM]/[OPEN] | Canonical microscopic history action $Z_u[\eta,h,a,\lambda_R]$; [THEOREM] reduction to the linear G18 generator. Kept as-is in the consolidation. |
| [`DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md`](DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md) | [PARTIAL] | FTD needs no microscopic U(1); U(1) is an effective description of projected flux. Post-pivot ontology foundation. |
| [`DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md`](DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md) | [PARTIAL] | Native state↔flux mapping to an EFT dictionary; scaling dimensions frozen under FTD-0059 (Gate-1 closure). |
| [`RETROSPECTIVE_EFT_RECOVERY.md`](RETROSPECTIVE_EFT_RECOVERY.md) | [SYNTHESIS] | Narrative roll-up of the whole program; integrates existing claims at their canonical tags, promotes nothing. Pointer index back to live survivors and archived scaffolding. |
| [`SCOPE_ALPHA_READOUT_NEXT_STEPS.md`](SCOPE_ALPHA_READOUT_NEXT_STEPS.md) | [SCOPING MEMO] | ARC-A1 track: outline of Candidate A, C, and B2 unattempted readout routes. |
| [`SCOPE_GC_QUANTUM_PATH_INTEGRAL.md`](SCOPE_GC_QUANTUM_PATH_INTEGRAL.md) | [SCOPING MEMO] | Mechanism B track (FTD-0216): formulation of Euclidean quantum partition function over the body-diagonal sub-stencil $\sigma_{\text{BCC}}$, discrete-to-continuum vacuum polarization, and non-circular Wilsonian matching. |
| [`ANALYSIS_NONLINEAR_BRIDGE_SWEEPS.md`](ANALYSIS_NONLINEAR_BRIDGE_SWEEPS.md) | [MEASUREMENT ANALYSIS] | F-D3 track: analysis of parameter sweeps D3a-D3d, leading to the final mechanism discrimination verdict among Mechanism $\alpha$, $\beta$, and $\gamma$. |
| [`EXPLR_MASS_SCALE_GENERATION.md`](EXPLR_MASS_SCALE_GENERATION.md) | [CONJECTURE]/[SELECTION] | FTD-0219: Formulates Candidate A (holographic area-to-volume scaling) and Candidate B (non-perturbative sLoop self-energy feedback) loopholes to bypass the FTD-0096 no-go mass barrier. |

---

## §4 — OPEN (open-problem statements)

| File | Tag | Purpose |
|---|---|---|
| [`OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`](OPEN_FTD_TO_EFT_BRIDGE_STATUS.md) | [CLOSED NEGATIVE for QED α] / pivot | Where the QED-α bridge failed; defines the replacement target as native FTD source/flux physics. |
| [`OPEN_FTD_NATIVE_ACTION_OR_MEASURE.md`](OPEN_FTD_NATIVE_ACTION_OR_MEASURE.md) | [PARTIAL] | Bridge Gate 2: linear generator derived, microscopic history measure selected; explicit nonlinear blocked effective action remains open. |
| [`OPEN_GC_FROM_FIRST_PRINCIPLES.md`](OPEN_GC_FROM_FIRST_PRINCIPLES.md) | [OPEN] | $g_c$ from first principles after Mechanisms A–C closures; dimensionless origin unknown. |

---

## §5 — STATUS (checklists, trackers, manifests)

| File | Tag | Purpose |
|---|---|---|
| [`STATUS_EFT_CHECKLIST.md`](STATUS_EFT_CHECKLIST.md) | [PARTIAL] | Living checklist of what is required for FTD to count as a native EFT. Full nonlinear $S_\text{eff}$ remains the central open deliverable. |
| [`STATUS_NONLINEAR_REGIME_2026-04-30.md`](STATUS_NONLINEAR_REGIME_2026-04-30.md) | [REFERENCE] | Consolidated handoff for the nonlinear regime: phases, gates, next targets. |
| [`REF_PREREGISTER_MANIFEST.md`](REF_PREREGISTER_MANIFEST.md) | [REFERENCE] | Single authoritative table mapping every pre-registered measurement to its git tag, commit SHA, runner, output dir, and analysis doc. |

---

## §6 — PREREG (hash-locked pre-registrations)

Methodology committed before measurement. See `REF_PREREGISTER_MANIFEST.md` for tag/SHA provenance.

| File | Tag | Purpose |
|---|---|---|
| [`PREREG_ADVERSARIAL_LOOK_ELSEWHERE_v1.md`](PREREG_ADVERSARIAL_LOOK_ELSEWHERE_v1.md) | [PRE-REGISTRATION] | FTD-0189 adversarial look-elsewhere scan over an 18-constant basket FTD did not design. |
| [`PREREG_ALPHA_ARITHMETIC_GENERATIVITY_v1.md`](PREREG_ALPHA_ARITHMETIC_GENERATIVITY_v1.md) | [PRE-REGISTRATION] | FTD-0185 alpha arithmetic-generativity test (Test 4) — the Balmer-to-Bohr gate. |
| [`PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md`](PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md) | [PRE-REGISTRATION] | FTD-0215: BCC complex readout pre-registration locking design and verification parameters. |
| [`PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md`](PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md) | [PRE-REGISTRATION] | FTD-0143 uniqueness scan of the FQCR Model IV $(4,6;3,2)$ exponent quadruple. |
| [`PREREG_FTD_0110_NONLINEAR_BRIDGE_v1.md`](PREREG_FTD_0110_NONLINEAR_BRIDGE_v1.md) | [PRE-REGISTRATION] | FTD-0215 nonlinear bridge coordinated parameters sweeps and active partitioning (F-D3). |
| [`PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md`](PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md) | [PRE-REGISTRATION] | FTD-0186 boundary-theorem Stage 1: the structural/dynamical discriminator. |

---

## §7 — Frontier 4: emergent graviton substrate mode

The graviton-census trio — does the FTD substrate carry an emergent massless spin-2 mode?

| File | Tag | Purpose |
|---|---|---|
| [`PREREG_GRAVITON_SUBSTRATE_MODE_v1.md`](PREREG_GRAVITON_SUBSTRATE_MODE_v1.md) | [PRE-REGISTRATION] | Frontier 4 Step 4a v1; locks the hypothesis and decision criteria. Retained as provenance. |
| [`PREREG_GRAVITON_SUBSTRATE_MODE_v2.md`](PREREG_GRAVITON_SUBSTRATE_MODE_v2.md) | [PRE-REGISTRATION] | Step 4a-ii v2 — supersedes v1 §5/§8; locks the decisive engine campaign. |
| [`REPORT_GRAVITON_SUBSTRATE_MODE.md`](REPORT_GRAVITON_SUBSTRATE_MODE.md) | [MEASUREMENT REPORT] | Canonical measurement registered by v2; Outcome verdict applied against PREREG v2 §6/§7. **FTD-0193** (renumbered 2026-05-22 from FTD-0190 to resolve a collision with the Q10 finite-neutral-lock FTD-0190). |

---

## §8 — AUDIT (load-bearing audits)

| File | Tag | Purpose |
|---|---|---|
| [`AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md`](AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md) | [AUDIT] | ARC-C1: Charge quantization audit and exact QED-vs-native normalization boundary integrity. |
| [`AUDIT_ALPHA_EXTRACTION.md`](AUDIT_ALPHA_EXTRACTION.md) | [AUDIT] | Line-by-line audit of the Phase-F α extraction pipeline; the "3.6× α_ref" claim, resolved by Phase G as a category error. |
| [`AUDIT_HEEGNER_TOWER_RIGIDITY.md`](AUDIT_HEEGNER_TOWER_RIGIDITY.md) | [COMPLETE] | 9-Heegner CM-tower master-quadratic rigidity scan; CM-uniqueness bifurcates by methodology. Pre-reg now in `archive/campaign_complete/`. |
| [`AUDIT_LEMNISCATE_ALPHA_RIGIDITY.md`](AUDIT_LEMNISCATE_ALPHA_RIGIDITY.md) | [COMPLETE] | Lemniscate-alpha rigidity scan; canonical 5-harmonic curve found in ~4.3% of natural alternatives, retagged [SELECTION]. Pre-reg now in `archive/campaign_complete/`. |

---

## § Archive

Archived material is preserved for provenance — cite it to explain "why this route was rejected" or "which completed campaign produced this number"; don't act on it as live work. The full pointer index lives in **`RETROSPECTIVE_EFT_RECOVERY.md`**, which ties the archived scaffolding into the program narrative. Counts as of 2026-05-22:

| Subdir | Count | Description |
|---|---|---|
| [`archive/closed_negative/`](archive/closed_negative/) | 21 | Closed-negative routes — pre-pivot projected-α derivations, the three $g_c$ mechanism attempts, $a_\text{phys}$ mechanism attempts, the mass-unit $\mu$ missing arrow, and superseded protocols/pre-regs. |
| [`archive/campaign_complete/`](archive/campaign_complete/) | 42 | Completed-campaign scaffolding — protocol/pre-reg/analysis/audit triplets for campaigns that have run and recorded their result (emergent spectrum, topological observables, operator-mixing, s_eff nonlinear, Lorentz/Ward, rigidity scans, decisions). |
| [`archive/resolved/`](archive/resolved/) | 1 | `OPEN_A_PHYS_DERIVATION.md` — the $a_\text{phys}$ open problem, closed by `THEOREM_A_PHYS_NO_GO.md`. |
| [`archive/retracted/`](archive/retracted/) | 1 | `DERIV_A_PHYS_MECHANISM_GAMMA_SUCCESS.md` — a calibration-substitution false positive, retracted. |
| [`archive/phase_0_f_campaign/`](archive/phase_0_f_campaign/) | 6 | The original Phase 0–F measurement campaign docs (β-function, day-2, gap-closure, operator-spectrum, symmetry-recovery, dynamical-SM). |

`archive/ARCH_00_INDEX_2026-04-27.md` is the superseded predecessor of this INDEX.

---

**Live document count:** 45 top-level docs (this INDEX excluded) — 6 SPEC, 6 THEOREM, 12 native-flow/bridge DERIV incl. the retrospective, 3 OPEN, 3 STATUS, 9 PREREG, 3 Frontier-4 graviton, 4 AUDIT.
