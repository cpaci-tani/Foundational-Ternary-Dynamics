# FTD Native EFT Checklist

**Date:** 2026-04-26 (last: 2026-04-24)
**Status:** [PARTIAL] Native Gaussian bridge closed at theorem + GPU-measured level (§4 fully checked 2026-04-26); GPU full-tick ledgers are in place; microscopic native action selected as a constrained history measure; full nonlinear effective EFT remains open.

This checklist tracks what is required for FTD to count as a real, native EFT
rather than a collection of projected continuum comparisons. It follows the
current pivot documented in `00_INDEX.md`: the projected `x_+` / QED-alpha
RG interpretation is closed negative; the active program is the native
finite-volume/blocking EFT of FTD histories.

---

## 0. Epistemic Ground Rules

- [x] No numerical near-miss searches used as derivations.
- [x] Projected QED-alpha matching separated from native EFT construction.
- [x] Parametric insertions kept distinct from derivations.
- [x] `a_phys` no-go theorem accepted: physical length scale is a calibration interface, not derived from Axiom Zero.
- [x] Master quadratic remains algebraic/number-theoretic; no longer treated as a proven physical RG step.
- [ ] Keep all new EFT claims tagged: [THEOREM], [MEASUREMENT], [SELECTION], [CONJECTURE], [OPEN], or [CLOSED NEGATIVE].

## 1. Native Degrees Of Freedom

- [x] State field `s in {-1,0,+1}` identified as charge/manifestation density.
- [x] Flux field `J in R^3` identified as dispositional field variable.
- [x] Dual-substrate fields `J_L`, `J_R` represented in engine.
- [x] Native dual-cell / finite-volume containers implemented.
- [x] Engine flat indexing aligned across native continuity/blocking code.
- [ ] Decide whether the formal master-quadratic paper's BCC/corner channel is part of the canonical field basis or a delayed/nonlinear sector.
- [ ] Decide whether the canonical EFT field basis is collocated `(s,J)` or face-centered/dual-cell `J*`.
- [ ] Define the final minimal field tuple for nonlinear campaigns.

## 2. Native Kinematics And Constraints

- [x] Lattice geometry and Moore-neighborhood support documented.
- [x] Local finite-volume continuity convention fixed:
  `Delta rho + div I = S_reaction`.
- [x] Native b=2 blocking map implemented for density, reaction, and oriented currents.
- [x] Dual-cell Gauss prototype implemented and tested.
- [x] Moore-shell and source-core alternatives compared.
- [ ] Resolve source-core compromise: skipped particle sites vs exact full-site `div J = s`.
- [ ] Select production Gauss representation: collocated, source-core fork, or dual-cell face flux.

## 3. Engine History Ledger

- [x] Reaction-only histories block correctly.
- [x] Face transport histories block correctly.
- [x] Moore diagonal transport is routed through oriented face currents.
- [x] Same-sign bounce is classified as continuity no-op.
- [x] Opposite-sign annihilation is classified as local reaction.
- [x] Mixed transport plus local reaction closes.
- [x] Multi-tick interval histories telescope correctly.
- [x] Operator moments measured from continuity histories.
- [x] GPU movement ledgers match host snapshot inference.
- [x] GPU full-tick ledgers cover genesis, evaporation, pair production, movement, annihilation, weak transmutation.
- [x] `RenderBridge::continuity_step()` exposes the latest native GPU ledger.
- [ ] Add device-side reductions for long-run ledger moment streams without host downloads.
- [ ] Add production campaign output format for large history ensembles.

## 4. Linear / Gaussian EFT

- [x] Bare native blocking map specified.
- [x] Native response tuple specified.
- [x] Linear generator derived for Gaussian response flow.
- [x] b=2 Gaussian response flow closes for the canonical couplings.
- [x] Response flow tests pass in engine.
- [x] Re-run and archive a fresh full campaign table after the GPU-ledger changes. **2026-04-26 — 21/21 Gaussian-flow ctests PASS on GPU (RTX 5090, commit 347a38f). Headline `nonlinear_flow_multiscale` reproduces FTD-0070 verdict: β consistent with 0 within 1σ at b ∈ {1,2,4,8}, K(b+1)/K(b) ratios stable. Note: 70-75 % uniform level shift in E_b vs. FTD-0070 baseline (commit 347a38f stencil-weights derivation moved an internal normalization); fixed-point classification unchanged. Per-test logs + meta.json archived in `engine/results/gaussian_expansion_2026-04-26/`. Inventory: `docs/theory/10_eft_program/GAUSSIAN_EXPANSION_DATA_INVENTORY.md`.**
- [x] Decide whether Gaussian fixed-point data is a theorem-level result or a measured engine result. **2026-04-26 — both. The bare-tuple statement `(C_L, K_T, Z_j, g_sJ)(b) = (1,1,1,1)` for the bare linear generator under the dual-cell b=2 blocking map is a [THEOREM] of the finite-volume Wilsonian map (`SPEC_FTD_NATIVE_BLOCKING_MAP.md` lemmas; `native_response_flow` + `native_blocking_map` tests verify floating-point closure). The Gaussian-fixed-point *behavior on a full nonlinear Langevin ensemble* is [MEASURED] (FTD-0070 + 2026-04-26 GPU rerun). See `GAUSSIAN_EXPANSION_DATA_INVENTORY.md` §6 for the per-component classification.**

## 5. Operator Basis

- [x] Initial operator basis pre-registered.
- [x] Current, reaction, source-response, and transport moments represented.
- [x] Native operator spectrum test exists.
- [x] Ward-identity and matched-Poisson tests exist.
- [ ] Extend basis with all nonlinear/reaction operators observed in full-tick GPU histories.
- [ ] Define operator mixing matrix from blocked full-history ensembles.
- [~] Classify relevant, marginal, and irrelevant directions from measured native flow. **[PARTIAL] 2026-04-25** — `AUDIT_OPERATOR_SPECTRUM.md` (FTD-0091): all 5 measurable operators classify as "relevant" (Δ < D = 4) at L=32 in both the pulse and flux-baryon scenarios; the marginal/irrelevant bands of the pre-reg bracket are not recovered. Operator stratification IS present (divJ² Δ jumps ×3.4 between scenarios), so the basis is non-degenerate; the pulse-regime "all Δ ≈ 0.5" collapse is a scenario envelope artefact, not strong-coupling. Full classification requires L ≥ 64 + multi-scenario ensemble.
- [ ] Separate engine-rule operators from emergent coarse operators.

## 6. Nonlinear Flow

- [x] Real engine histories can now be converted into native continuity ledgers.
- [x] Full-tick GPU histories expose all currently known state-changing channels.
- [ ] Build systematic nonlinear b=2 flow campaigns from engine histories.
- [~] Add BCC/corner-channel observables motivated by `PAPER_MASTER_QUADRATIC_FORMAL.pdf`. **2026-04-26 — Cluster A engine build complete (`engine/include/ftd/sublattice.h`, `correlations.h`, `spectrum_extraction.h`, `term_toggles.h::bcc_stencil`, `campaign_bcc_band_spectrum.cpp`, all tests PASS); D2 protocol drafted (`PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`); D1 derivation drafted (`DERIV_MECHANISM_C_GC_BCC_BRIDGE.md`, FTD-0093). Awaiting publication-grade run + D6 audit.**
- [ ] Measure operator mixing under blocking.
- [ ] Measure reaction-sector scaling.
- [ ] Measure transport-sector scaling.
- [ ] Measure mixed transport/reaction couplings.
- [ ] Pre-register any trace/determinant comparison to the master-quadratic Vieta data before measuring.
- [ ] Determine whether a nontrivial native fixed point exists.
- [ ] Determine whether continuum symmetries improve, degrade, or stay lattice-native under flow.

## 7. Gauge / Electrodynamics Status

- [x] Geometric Coulomb limit documented.
- [x] Emergent U(1) projection story documented as partial.
- [x] Native electrodynamics spec exists.
- [x] Projection convergence and source-response tests exist.
- [ ] Resolve exact Gauss representation for production.
- [ ] Establish a native Ward identity for full histories, not only projected probes.
- [ ] Decide whether U(1) is emergent redundancy, constraint symmetry, or only a projected interpretation.
- [ ] Quantify lattice artifacts in the native electromagnetic sector under blocking.

## 8. Couplings And Calibrations

- [x] `a_phys` status resolved by no-go theorem: external calibration.
- [x] `K_B` status remains calibration/manifestation scale.
- [x] Old physical RG reading of `x_+` closed negative.
- [x] Alpha extraction tooling builds again.
- [~] Derive or demote `g_c`: `OPEN_GC_FROM_FIRST_PRINCIPLES.md` remains open. **2026-04-26 — Mechanism A ruled out; Mechanism B closed negative 2026-04-25 (`DERIV_MECHANISM_B_GC_DERIVATION.md`, circularity); Mechanism C drafted as [CONJECTURE] (FTD-0093, `DERIV_MECHANISM_C_GC_BCC_BRIDGE.md`). Awaiting D6 audit on `PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`. Companion: μ-from-ℓ_P missing arrow (FTD-0096) tracked in `OPEN_MU_FROM_LP_MISSING_ARROW.md`.**
- [~] Evaluate "G18 direct sector vs BCC/CM capacity matching" as a possible `g_c` mechanism, tagged [CONJECTURE] until derived. **2026-04-26 — predecessor reading of Mechanism C bridge-operator hypothesis (FTD-0093 §3 cites the BCC sub-stencil structural argument).**
- [ ] Decide whether `coulomb_charge_coupling` is a measurement knob, calibration, or derived engine convention.
- [ ] Separate dimensionless native couplings from physical-unit calibrations in all docs and APIs.

## 9. Statistical Measure / Action

- [x] L=2 partition function exists as a first explicit finite example.
- [x] Define the full native path measure over state/flux histories. **2026-04-26 — selected in `DERIV_FTD_NATIVE_COMPLETE_HISTORY_ACTION.md` as a constrained source-coupled history measure over `(q_t, ledger_t)` with deterministic tick constraints plus stochastic log-likelihood terms.**
- [x] Decide whether the EFT object is an action, transfer matrix, Markov/deterministic pushforward measure, or constrained history measure. **2026-04-26 — microscopic object is a transfer-kernel/history action; smooth continuum Lagrangian is deferred to blocked `S_eff`.**
- [ ] Decide whether the BCC Watson denominator `1 - cos(kx) cos(ky) cos(kz)` enters the native transfer/action kernel.
- [~] Include deterministic update Jacobian or prove it cancels/does not enter. **2026-04-26 — exact microscopic object uses a delta/indicator transfer kernel, so no smooth change-of-variables Jacobian is needed at that level; a Jacobian question reappears only if rewritten as a continuum field integral.**
- [~] Include stochastic genesis/Langevin sectors in the measure when toggled on. **2026-04-26 — Langevin OU cost specified; discrete channel log-likelihood form specified; channel-by-channel probability catalogue remains open.**
- [ ] Derive the native effective action after b=2 blocking. **Definition now fixed as `exp(-S_eff[H']) = sum_{H:B_b H=H'} exp(-S_H[H])`; explicit measured/fitted `S_eff` remains open.**
- [ ] Connect the action/measure to the observed operator-flow matrix.

## 10. GPU-First Engine Infrastructure

- [x] CUDA default enabled for native builds.
- [x] GPU path no longer silently falls back to CPU.
- [x] `dt` and Langevin seed propagate to GPU.
- [x] GPU `run(N)` syncs ledger/host bookkeeping correctly.
- [x] Full engine Release build passes.
- [x] Native/GPU ledger suite passes.
- [ ] Full CTest pass after unrelated long/slow campaign policy is clarified.
- [ ] Move long scans/benchmarks out of default quick validation if needed.
- [ ] Add a standard "EFT quick suite" CTest label.

## 11. Documentation Hygiene

- [x] Current live index exists.
- [x] Closed-negative projected attempts are documented.
- [x] Transport-flow doc updated for GPU full-tick ledgers.
- [x] Master-quadratic formal paper correlated with EFT open items in `ANALYSIS_MASTER_QUADRATIC_EFT_OPEN_ITEMS.md`.
- [x] Engine constructor-domain contract drafted in `engine/SPEC_ENGINE_CONSTRUCTOR_CONTRACT.md`.
- [ ] Update `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` to reflect GPU full-tick ledger closure for genesis/evaporation.
- [ ] Move closed-negative projected-EFT attempts into an archive folder after link updates.
- [ ] Add a native EFT capstone summary replacing the old alpha-recovery narrative.
- [ ] Keep this checklist updated after every bridge milestone.

---

## 12. Constructor-Domain Engine Audit

This section maps the minimal-universe constructor domains onto the engine. The
goal is not to claim that every domain is proven; the goal is to make every
domain explicit enough that proof, measurement, or demotion is possible.

| Domain | Engine status | Next formalization step |
|---|---|---|
| Instantiation | [PARTIAL] `RenderBridge` scenarios instantiate lattice, fields, and carriers; context metadata is not centralized. | Add scenario/test metadata for context, closure, observable map, and epistemic tag. |
| Identity / persistence | [PARTIAL] particle IDs, transport histories, and ledgers exist. | Define when identity is particle ID, transport path, blocked history, or no-op. |
| Relation / primitive space | [PRESENT] lattice/Moore relation and flat indexing are tested. | Add one canonical relation-space declaration used by EFT docs and telemetry. |
| Frame / symmetry redundancy | [PARTIAL] parity, anisotropy, and backend parity tests exist. | Add a frame/gauge-relabeling contract: what changes are descriptive vs intrinsic. |
| Dynamics / time | [PRESENT] tick/run/dt are implemented, GPU `run(N)` now uses per-tick ledger sync. | Add test metadata declaring deterministic vs stochastic tick semantics. |
| Transport / continuity | [PRESENT] full-tick GPU continuity ledgers cover known state-changing channels. | Add device-side reductions and long-run moment streams. |
| Constraint / Gauss | [PARTIAL] collocated, matched-Poisson, and dual-cell prototypes exist. | Select the production Gauss representation. |
| Locality / causality | [PARTIAL] Moore support and CFL limits exist. | Add formal propagation-bound tests for each state-changing toggle. |
| Conservation / closure | [PARTIAL] continuity, reaction, and energy ledgers exist. | Define closure domains for periodic lattice, blocked cells, and open-boundary campaigns. |
| Observables | [PARTIAL] diagnostics/operator moments exist. | Add a native observable registry keyed by domain and epistemic tag. |
| Probability / statistics | [PARTIAL] ensembles and Langevin seed plumbing exist; GPU Langevin has open failures. | Fix GPU Langevin and define the native stochastic measure when toggles are enabled. |
| Thermodynamics / arrow | [PARTIAL] entropy and thermodynamic tests exist. | Tie entropy to blocking/history records and define arrow observables. |
| Metric / geometry | [PARTIAL] lattice metric, anisotropy, and Lorentz recovery tests exist. | Add continuum-limit scaling protocol before smooth-geometry claims. |
| Topology / defects | [PARTIAL] Wilson/topology tests and BCC/stella sector work exist. | Port topology diagnostics to GPU and preregister sector observables. |
| Many-body / correlation | [PARTIAL] correlation tests and many-body campaigns exist. | Define native correlation observables from GPU full-history ledgers. |
| Mixing / sectors | [OPEN] genuine nonlinear operator mixing matrix is not measured yet. | Build blocked full-history operator mixing matrix. |
| Blocking / EFT | [PARTIAL] native b=2 Gaussian bridge works. | Extend from Gaussian response flow to nonlinear flow campaigns. |
| Continuum limit | [OPEN] supporting tests/docs exist, but no full engine-native proof. | Establish scaling tests across L and b with predeclared observables. |
| Action / generator | [PARTIAL] Gaussian linear generator exists; microscopic constrained-history action selected. | Derive/measure the blocked nonlinear effective action and connect it to the operator-flow matrix. |
| Phenomenology | [PARTIAL] campaigns exist but physical matching remains bridge-dependent. | Require observable-map + calibration declaration before any phenomenology claim. |

Constructor-critical engine gaps:

```text
production Gauss representation
native observable registry seed implemented; needs expansion as new claims land
constructor-domain metadata helper implemented for first retrofitted tests
GPU stochastic/Langevin correctness
long-run device-side ledger reductions
nonlinear operator mixing matrix
blocked nonlinear effective action from the selected native history measure
continuum-limit scaling protocol
EFT quick-suite CTest label seeded; needs expansion as FORM tickets close
```

---

## Current Verdict

FTD now has a serious native EFT skeleton:

```text
fields -> constraints -> per-tick histories -> finite-volume ledger -> b=2 blocking
```

The Gaussian/native bridge is closed enough to build on, and the engine can now
emit full-tick GPU continuity histories for the state-changing sectors.

FTD does **not** yet have a complete EFT. The missing core is:

```text
blocked nonlinear effective action from the selected native history measure
operator mixing matrix from nonlinear full histories
fixed-point / scaling classification from those measured flows
final Gauss/source-core representation
first-principles status of g_c
constructor-domain metadata and observable registry
```

The next milestone should be the nonlinear operator-flow campaign: consume
`RenderBridge::continuity_step()` histories, block them, compute the operator
moment vector before/after blocking, and assemble the first measured native
mixing matrix.
