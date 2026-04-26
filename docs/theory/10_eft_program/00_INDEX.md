# 10_eft_program — Index

**Date:** 2026-04-23 (revised; `a_phys` no-go theorem added)
**Maintainer note:** This index is agent-readable. Read it first to orient in
this directory. Status tags reflect the LEDGER (`../07_assessment/LEDGER.md`)
where applicable; if this index disagrees with the LEDGER, the LEDGER wins.

---

## Executive summary

As of 2026-04-23 the EFT program in this directory has two distinct threads,
one replaced by the other:

1. **Replaced target (historical).** The attempt to derive QED `1/alpha` from
   the master quadratic via a projected lattice EFT. Four closure routes were
   pre-registered and run: R1 transverse stiffness, R2 source-current
   normalization, R3 two-sector response eigenvalue, and the background
   Structure-2 Ward-valid scalar gauge completion. All closed NEGATIVE under
   the current projected action. The EFT Recovery Program (Phases 0-F,
   `SPEC_EFT_RECOVERY_PROGRAM.md`) ran to completion; its `alpha_infinity`
   plateau is a falsifiable FTD-vs-CODATA gap, not an agreement.
2. **Active target.** FTD-native electrodynamics: derive the native response
   tuple `(C_L^FTD, K_T^FTD, Z_j^FTD, c_FTD, W_18, g_sJ^FTD)` and its Wilsonian
   flow under a declared blocking map, with QED matching demoted to diagnostic.
   Current status: Gaussian b=2 flow closed for all four couplings at the
   canonical value 1; the native microscopic action has been selected as a
   constrained source-coupled history measure; non-linear/reaction-sector flow
   and the explicit blocked effective action remain OPEN.

The master quadratic itself is unaffected. FTD-0001 stays [THEOREM] at the
algebraic / number-theoretic layer (Gamma(1/4)^4 + CM-curve uniqueness).
FTD-0013 / FTD-0014 (physical identifications x_+ <-> 1/alpha and x_- <-> N_c)
remain [STRONGLY MOTIVATED CONJECTURE]; they never depended on an RG-step
reading of the polynomial. The Link 8 closure (FTD-0050) rules out the
additional RG-flow interpretation on the engine's (SC+FCC)/2 stencil.

---

## Reading order for a new agent

Read in this sequence to reach the current front:

1. `OPEN_FTD_TO_EFT_BRIDGE_STATUS.md` — authoritative bridge-inventory table.
2. `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` — why the target pivoted.
3. `DERIV_FTD_NATIVE_RESPONSE_TUPLE.md` — first fixed probe of the native tuple.
4. `SPEC_FTD_EFT_BRIDGE_CONTRACT.md` — frozen gates for what counts as bridge progress.
5. `SPEC_FTD_NATIVE_BLOCKING_MAP.md` — native Wilsonian blocking contract.
6. `DERIV_FTD_NATIVE_RESPONSE_FLOW.md` — b=2 Gaussian flow closure.
7. `AUDIT_LINK8_CLOSURE.md` — why the RG reading of the master quadratic is closed.
8. `OPEN_FTD_NATIVE_ACTION_OR_MEASURE.md` — the remaining load-bearing open problem.

Historical context (optional, for tracing why the current target is what it is):
`OPEN_FTD_TO_EFT_MATCHING.md`, then the four R1-R4 `..._XPLUS_ATTEMPT.md`
docs, then `SPEC_EFT_RECOVERY_PROGRAM.md` and `DERIV_DAY2_CAMPAIGN.md`.

---

## Cluster 1 — Native EFT program (active target)

| File | One-line | Status |
|---|---|---|
| `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` | Program spec after QED-alpha closure | [SELECTION] |
| `SPEC_FTD_EFT_BRIDGE_CONTRACT.md` | Frozen bridge gates; native first, QED second | [SELECTION] |
| `SPEC_FTD_NATIVE_BLOCKING_MAP.md` | Finite-volume dual-cell b=2 blocking contract | [SELECTION] |
| `DERIV_FTD_NATIVE_RESPONSE_TUPLE.md` | First bare probe; C_L=K_T=Z_j=g_sJ=1, c=1/sqrt(3), W_18~1.2679 | [PARTIAL] / mixed tags per-row |
| `DERIV_FTD_NATIVE_LINEAR_GENERATOR.md` | Minimal linear source-coupled generator reproducing the bare tuple | [PARTIAL] |
| `DERIV_FTD_NATIVE_COMPLETE_HISTORY_ACTION.md` | Complete microscopic native action as constrained source-coupled history measure | [SELECTION]/[PARTIAL] |
| `DERIV_FTD_NATIVE_SOURCE_FLUX_COUPLING_CLOSURE.md` | g_sJ^FTD = 1 under current action; non-unit not derived | [CLOSED NEGATIVE]/[DEFINITION] |
| `DERIV_FTD_NATIVE_BARE_FLOW.md` | Bare Gaussian dual-cell flow at b=2 | [PARTIAL] |
| `DERIV_FTD_NATIVE_CURRENT_FLOW.md` | Transport-current b=2 flow; Z_j'=1 | [PARTIAL] |
| `DERIV_FTD_NATIVE_RESPONSE_FLOW.md` | Static/vertex b=2 flow; C_L'=g_sJ'=1 | [PARTIAL] |
| `DERIV_FTD_NATIVE_ENGINE_HISTORY_FLOW.md` | Real-engine reaction-history flow adapter | [PARTIAL] |
| `DERIV_FTD_NATIVE_ENGINE_TRANSPORT_FLOW.md` | Real-engine face-transport flow adapter | [PARTIAL] |
| `DERIV_FTD_NATIVE_SCALE_FLOW.md` | Kadanoff block-spin on bare Green; C_L' running under canonical scaling | [THEOREM] (bare Gaussian FP) |
| `OPEN_FTD_NATIVE_ACTION_OR_MEASURE.md` | Native action/measure gate; now resolved at microscopic history-measure level, nonlinear S_eff open | [PARTIAL]/[OPEN] |

## Cluster 2 — Projected-EFT xplus attempts (historical, all negative)

These are the four R1-R4 pre-registered routes from attempting to make x_+ a
physical observable in the projected EFT. All closed under the current action.

| File | One-line | Status |
|---|---|---|
| `DERIV_PROJECTED_STIFFNESS_XPLUS_ATTEMPT.md` | R1: K_T,0 = x_+ test | [CLOSED NEGATIVE] |
| `DERIV_SOURCE_CURRENT_NORMALIZATION_XPLUS_ATTEMPT.md` | R2: e0^2 = 1/x_+ from source transport | [CLOSED NEGATIVE] |
| `DERIV_PROJECTED_RESPONSE_EIGENVALUE_XPLUS_ATTEMPT.md` | R3: x_+ as kinetic-response eigenvalue | [CLOSED NEGATIVE] |
| `DERIV_PROJECTED_DIRAC_OPERATOR_AND_CHARGE_NORMALIZATION.md` | Symbolic projected Dirac; e0 not fixed | [CLOSED NEGATIVE] |
| `OPEN_PROJECTED_EFT_RENORMALIZATION_AND_ALPHA_OBSERVABLE.md` | Renorm/observable gate; marked CLOSED | [CLOSED NEGATIVE] |
| `OPEN_FTD_TO_EFT_MATCHING.md` | Pivot memo: QED-alpha matching no longer primary | [CLOSED NEGATIVE] |
| `DERIV_PROJECTED_EFT_MATTER_COUPLING.md` | Matter step of the projected branch; Dirac remains selected | [PARTIAL] |
| `DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md` | U(1) as projection redundancy, not primitive gauge | [PARTIAL] |
| `DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md` | Source-coupled vector dictionary from (s, J) | [PARTIAL] |
| `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` | V(r) mode is lattice-geometric Coulomb with zero free params | [THEOREM] |

## Cluster 3 — Link 8 + master-quadratic-as-RG-step (closed negative)

| File | One-line | Status |
|---|---|---|
| `AUDIT_LINK8_CLOSURE.md` | All four Link 8 tests closed negative; stencil is (SC+FCC)/2, orthogonal to BCC | [CLOSED NEGATIVE] (FTD-0050) |
| `AUDIT_EFT_BCC_ORTHOGONALITY.md` | Audit of prior EFT claims against Link 8 finding; no existing claims invalidated | [AUDIT] |

## Cluster 4 — EFT Recovery Program (Phases 0-F, historical)

Pre-registered 7-phase campaign. Ran to completion. Headline: `alpha_infinity`
plateau at ~3.6x alpha_ref, not CODATA. The Phase-F audits are the line-by-line
forensics of that plateau. `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` (Cluster 2)
closed the interpretation: the 1.8x residual was a convention artifact, not
physics.

| File | One-line | Status |
|---|---|---|
| `SPEC_EFT_RECOVERY_PROGRAM.md` | Pre-registration for Phases 0-F | [REFERENCE] / complete |
| `SPEC_OPERATOR_BASIS.md` | Phase-3 operator basis, pre-registered | [REFERENCE] |
| `DERIV_SYMMETRY_RECOVERY.md` | Phase 1: rotational/Lorentz/Ward measurements | [MEASUREMENT] |
| `DERIV_BETA_FUNCTION_MEASURED.md` | Phase 2: measured screened alpha_eff(L); qualitative negative-beta, quantitative gap | [MEASUREMENT] |
| `DERIV_OPERATOR_SPECTRUM.md` | Phase 3: 5/6 operators fit; Delta values outside pre-reg brackets | [MEASUREMENT] |
| `DERIV_DYNAMICAL_SM_EMERGENCE.md` | Phase 4: EWSB / generations / condensate cold-start tests | [MEASUREMENT] |
| `DERIV_GAP_CLOSURE.md` | Post-campaign follow-up tickets from manuscript Sec. 7 | [MEASUREMENT] |
| `DERIV_DAY2_CAMPAIGN.md` | Day-2 threads: matched-stencil Poisson, EWSB, spectroscopy, Rutherford | [MEASUREMENT] |
| `AUDIT_ALPHA_EXTRACTION.md` | Line-by-line audit of the "3.6x alpha_ref" pipeline | [AUDIT] / superseded by Phase G resolution |
| `AUDIT_ALPHA_SCALING_L256.md` | GPU Langevin first-use: T=0 scan to L=256; thermal-alpha identified untractable | [AUDIT] |
| `AUDIT_GPU_PLAN_PRIORITIES_1_3_5_6.md` | BCC tadpole + HMC + scheme + continuum + sunset priorities | [AUDIT] |
| `AUDIT_STRUCTURE2_WARD_VALIDATION.md` | Ward-valid S2 gauge completion; does not reproduce Structure-1 ppb | [CLOSED NEGATIVE] (FTD-0058) |
| `DERIV_PARTITION_FUNCTION_L2.md` | First explicit FTD Z on L=2 | [THEOREM] / [OPEN FINDING] |

## Cluster 5 — a_phys calibration [CLOSED by theorem 2026-04-23]

| File | One-line | Status |
|---|---|---|
| `THEOREM_A_PHYS_NO_GO.md` | No length derivable from Axiom Zero; `a_phys` must be calibrated | [THEOREM] (FTD-0059, 2026-04-23) |
| `OPEN_A_PHYS_DERIVATION.md` | Open-problem scoping doc; four mechanisms (α, β, γ, δ) closed | [CLOSED — RESOLVED BY THEOREM] (2026-04-23) |
| `DERIV_A_PHYS_MECHANISM_DELTA_ATTEMPT.md` | Information/CFL/ontic-chain/two-anchor routes all fail unit-trace check | [CLOSED NEGATIVE] (2026-04-23) |
| `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` | Dimensional chain; shows the chain converts one calibration into another rather than deriving `a_phys` | [CLOSED NEGATIVE] |
| `DERIV_A_PHYS_MECHANISM_GAMMA_SUCCESS.md` | Originally claimed [THEOREM] `a_phys ≈ 4.39 ℓ_P`; chain silently replaces `K_B = m_e` with `ℏ_lat = 1` | [RETRACTED 2026-04-23] |

Resolution: the four mechanism closures share a single structural cause — the ring of Axiom-Zero invariants is entirely SI-dimensionless. `THEOREM_A_PHYS_NO_GO.md` (FTD-0059) formalizes this, promoting the `a_phys ≡ ℓ_P` calibration from pragmatic fallback to theorem-enforced calibration interface. The earlier "[STATUS UNKNOWN]" tension between the SUCCESS doc and the `a_phys ≡ ℓ_P` declaration in `SPEC_FTD.md` / CLAUDE.md is subsumed: the authoritative disposition is **`a_phys ≡ ℓ_P` as a CALIBRATION, theorem-enforced** (LEDGER rows FTD-0030, FTD-0041, FTD-0059).

## Cluster 6 — g_c first principles / Mechanism B

| File | One-line | Status |
|---|---|---|
| `OPEN_GC_FROM_FIRST_PRINCIPLES.md` | Scoping doc for deriving g_c; three candidate mechanisms | [OPEN] |

## Cluster 7 — Other

| File | One-line | Status |
|---|---|---|
| `EXPLR_SELF_DUAL_HALF_SHELL.md` | k=1/2, m=1/2, r^2=1/2 exploration | [EXPLORATORY] / [MEASURED] / [CONJECTURE] (G* bridge) |
| `STATUS_CUDA_BUILD.md` | CUDA build status; resolved via WSL2 | [SOLVED] |

---

## Consolidation candidates (recommendations, not executed)

1. **Keep separate: `DERIV_FTD_NATIVE_RESPONSE_TUPLE.md` vs `DERIV_FTD_NATIVE_RESPONSE_FLOW.md`.** The Tuple doc is the bare probe (fixed-L measurements, engine audits, half-shell, Moore-layer closure). The Flow doc is the b=2 blocking result for static/vertex. Different questions, both referenced from the bridge-status table. Leave separate but add a cross-link header to each.
2. **Merge candidates: the four `DERIV_FTD_NATIVE_*_FLOW.md` docs (BARE / CURRENT / RESPONSE / SCALE) plus the two `..._ENGINE_*_FLOW.md` adapters.** Six docs at b=2 each proving one small gate. Candidate: a single `DERIV_FTD_NATIVE_B2_FLOW.md` with one section per gate. Against merger: the current split matches git-history of discrete same-day closures. Weak recommendation: merge after the nonlinear flow is closed (so the consolidated doc has a stable scope).
3. **Demote to `archive/closed_negative/`: the four R1-R4 projected-EFT xplus attempts** (`DERIV_PROJECTED_STIFFNESS_XPLUS_ATTEMPT.md`, `DERIV_SOURCE_CURRENT_NORMALIZATION_XPLUS_ATTEMPT.md`, `DERIV_PROJECTED_RESPONSE_EIGENVALUE_XPLUS_ATTEMPT.md`, `DERIV_PROJECTED_DIRAC_OPERATOR_AND_CHARGE_NORMALIZATION.md`) plus `OPEN_PROJECTED_EFT_RENORMALIZATION_AND_ALPHA_OBSERVABLE.md` and `OPEN_FTD_TO_EFT_MATCHING.md`. All explicitly marked CLOSED NEGATIVE / superseded. The bridge-status doc links to them by path, so the move must be executed with link updates. Keep the files (do not delete): they are the historical record of what was tried and why the pivot was justified.
4. **Keep both: `DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` + `_SUCCESS.md`.** Both documents are retained for epistemic transparency. The ATTEMPT doc is the substantive closure (calibration shuffle identified). The SUCCESS doc was retracted 2026-04-23 (same flaw at one remove) and now carries a retraction preamble; it is preserved in place as the canonical cautionary example. Consolidation would erase the epistemic record. Leave as-is. (Open escalation: user may wish to rename `_SUCCESS.md` → `_ATTEMPT_2.md` or move to an explicit `closed_negative/` archive; not executed pending review.)
5. **Keep separate: `OPEN_FTD_TO_EFT_MATCHING.md` vs `OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`.** Matching doc is the pivot memo (why the old matching program stopped). Bridge-status doc is the live inventory. Different purposes; the Matching doc should move to archive once its pivot role is no longer referenced.
6. **Weak merge candidate: `AUDIT_ALPHA_EXTRACTION.md` + `AUDIT_ALPHA_SCALING_L256.md` + `AUDIT_GPU_PLAN_PRIORITIES_1_3_5_6.md`** as three parallel forensic audits of the same Phase-F pipeline at different sizes/accelerators. They could share a header but the content is independent. Recommend leaving separate and adding a `## Related audits` cross-link block to each.

## Proposed directory moves (review, not executed)

1. Create `10_eft_program/archive/closed_negative/` and move:
   - `DERIV_PROJECTED_STIFFNESS_XPLUS_ATTEMPT.md`
   - `DERIV_SOURCE_CURRENT_NORMALIZATION_XPLUS_ATTEMPT.md`
   - `DERIV_PROJECTED_RESPONSE_EIGENVALUE_XPLUS_ATTEMPT.md`
   - `DERIV_PROJECTED_DIRAC_OPERATOR_AND_CHARGE_NORMALIZATION.md`
   - `OPEN_PROJECTED_EFT_RENORMALIZATION_AND_ALPHA_OBSERVABLE.md`
   - `OPEN_FTD_TO_EFT_MATCHING.md`
   Requires updating links in `OPEN_FTD_TO_EFT_BRIDGE_STATUS.md` and in papers under `dissemination/` or `docs/papers/` that cite these filenames.
2. Create `10_eft_program/archive/alpha_recovery_program/` and move the full EFT Recovery Program docs once a capstone doc is written (`SPEC_EFT_RECOVERY_PROGRAM.md`, `SPEC_OPERATOR_BASIS.md`, `DERIV_SYMMETRY_RECOVERY.md`, `DERIV_BETA_FUNCTION_MEASURED.md`, `DERIV_OPERATOR_SPECTRUM.md`, `DERIV_DYNAMICAL_SM_EMERGENCE.md`, `DERIV_GAP_CLOSURE.md`, `DERIV_DAY2_CAMPAIGN.md`, the three `AUDIT_ALPHA_*` docs, `AUDIT_STRUCTURE2_WARD_VALIDATION.md`, `AUDIT_GPU_PLAN_PRIORITIES_*`, `DERIV_PARTITION_FUNCTION_L2.md`). DO NOT move until a replacement summary doc exists; these are referenced by `CHANGELOG_REFRAME.md` and the manuscript.
3. Leave `AUDIT_LINK8_CLOSURE.md` and `AUDIT_EFT_BCC_ORTHOGONALITY.md` in place. They remain load-bearing negative constraints for any future RG-flow proposal.
4. Move `STATUS_CUDA_BUILD.md` out of `10_eft_program/` entirely. It is infrastructure, not an EFT artifact. Suggested destination: `docs/internal/` or an engine-side location.

## Stale / orphaned flags

1. **`EXPLR_SELF_DUAL_HALF_SHELL.md`** — [STATUS UNKNOWN] whether its G* bridge hypothesis is still live. Bridge-status doc lists it as [MEASURED]/[CONJECTURE]. No clear referrer outside the tuple doc. Flag for user review: retire or graduate to a DERIV_ doc.
2. **`DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md`** and **`DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md`** — both are [PARTIAL] bridge results produced same-day 2026-04-22 during the pivot. Content is substantive and referenced from the bridge-status doc; no action needed but flag for possible merger into a single `DERIV_FTD_NATIVE_DICTIONARY.md` once the projected branch is fully archived.
3. **`DERIV_FTD_NATIVE_SOURCE_FLUX_COUPLING_CLOSURE.md`** — closes a single line item (`g_sJ = 1`). Short. Candidate for merger into the Tuple doc if the tuple doc is updated; otherwise keep separate.
4. **`OPEN_A_PHYS_DERIVATION.md`** — [CLOSED — RESOLVED BY THEOREM] (2026-04-23). The four mechanism attempts (α, β, γ, δ) are subsumed by `THEOREM_A_PHYS_NO_GO.md` (FTD-0059). `a_phys ≡ ℓ_P` is theorem-enforced as the calibration interface. File is now a closed-open-problem scoping doc; flag for archive migration in a future housekeeping pass.
5. **`STATUS_CUDA_BUILD.md`** — not an EFT doc at all; belongs elsewhere (see move proposal 4).
6. **Orphan check:** no file in this directory appears to lack incoming references from at least one of {`OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`, the LEDGER, the reframe changelog, or a sibling doc in-cluster}. No true orphans found.
