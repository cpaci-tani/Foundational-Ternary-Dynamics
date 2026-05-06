# FTD Native EFT Checklist

**Date:** 2026-05-05 (R1 refresh; previous: 2026-04-26 — FTD-0098/0099/0100 mixing-matrix measurements)
**Status:** [PARTIAL] Native Gaussian bridge closed at theorem + GPU-measured level (§4); GPU full-tick ledgers in place; microscopic native action selected as a constrained history measure (`DERIV_FTD_NATIVE_COMPLETE_HISTORY_ACTION.md`); first nonlinear operator-mixing matrix $M_{ab}(b=2)$ measured 2026-04-26 (FTD-0098/0099/0100, asymmetric flux↔state mixing structure recovered); blocking-diagonal identities $M_{JJ}=16$, $M_{J^4}=256$ established as [THEOREM] (`THEOREM_BLOCKING_DIAGONAL_IDENTITIES.md`, 2026-04-30); structural-decoupling diagnosis L-independent at L∈{24,32,48,64} (`ANALYSIS_GATE_C_VS_L.md`, 2026-04-30); engine-side CPU↔GPU parity bugs F1/F4/F6 + diagnostics + F3 + F12 + F2 + F15 all closed (2026-05-04/05). **Full nonlinear effective EFT $S_\text{eff}$ remains open** — central R3 deliverable of the FTD-EFT roadmap.

This checklist tracks what is required for FTD to count as a real, native EFT
rather than a collection of projected continuum comparisons. It follows the
current pivot documented in `INDEX_FTD_NATIVE_EFT.md` (R1 deliverable, 2026-05-05;
supersedes `00_INDEX.md`): the projected `x_+` / QED-alpha RG interpretation is
closed negative; the active program is the native finite-volume/blocking EFT of
FTD histories.

**Companion R1 deliverables:**
- `INDEX_FTD_NATIVE_EFT.md` — categorised inventory of all 105 docs in this directory.
- `docs/theory/01_reference/MAP_LAGRANGIAN_TO_ENGINE.md` — line-anchored cross-reference from `SPEC_FTD_LAGRANGIAN.md` §3 to `engine/src/lagrangian.cpp` and per-phase implementation files.

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
- [x] Decide whether the canonical EFT field basis is collocated `(s,J)` or face-centered/dual-cell `J*`. **R2 closed 2026-05-05: `DECISION_FIELD_BASIS.md` picks collocated $(s, J)$ at lattice vertices. Field tuple $(s, J, v_\text{wave}, \mathcal{L})$.**
- [x] Define the final minimal field tuple for nonlinear campaigns. **R2 closed 2026-05-05 (same DECISION).**

## 2. Native Kinematics And Constraints

- [x] Lattice geometry and Moore-neighborhood support documented.
- [x] Local finite-volume continuity convention fixed:
  `Delta rho + div I = S_reaction`.
- [x] Native b=2 blocking map implemented for density, reaction, and oriented currents.
- [x] Dual-cell Gauss prototype implemented and tested.
- [x] Moore-shell and source-core alternatives compared.
- [x] Resolve source-core compromise: skipped particle sites vs exact full-site `div J = s`. **R2 closed 2026-05-05 (`DECISION_GAUSS_REPRESENTATION.md`): source-core fork remains as prototype path, not adopted; canonical enforcement is everywhere via cuFFT.**
- [x] Select production Gauss representation: collocated, source-core fork, or dual-cell face flux. **R2 closed 2026-05-05: `DECISION_GAUSS_REPRESENTATION.md` picks collocated cuFFT (GPU production path) with residual ≤ 1e-8.**

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
- [~] Extend basis with all nonlinear/reaction operators observed in full-tick GPU histories. **2026-04-30 — `THEOREM_BLOCKING_DIAGONAL_IDENTITIES.md` establishes $M_{JJ}=16$ and $M_{J^4}=256$ exact under blocking; `ANALYSIS_OFFDIAGONAL_ASYMMETRY.md` and `MEASUREMENT_GATE_D_T_PERTURBATION.md` extend the operator catalogue. Pre-registered 12-operator basis (per `SPEC_EFT_RECOVERY_PROGRAM.md` §6) remains pending full measurement under R3b.**
- [~] Define operator mixing matrix from blocked full-history ensembles. **[PARTIAL] 2026-04-26 (FTD-0098 + FTD-0099 + FTD-0100)** — first measured M_ab(b=2) on a Langevin+genesis ensemble at L=16 (FTD-0098) and at L=32 (FTD-0099 follow-up); first **full 6×6** measurement at L=16 with `inj-mult=1.0` (FTD-0100 follow-up — F2 closure). Per-config artifacts in `engine/results/operator_mixing_2026-04-26/{L16_b2, L16_b4, L32_b4, L16_b4_inj1.00}/`. FTD-0098/-0099 5×5 reduced subspace (s² dropped by ensemble state saturation); FTD-0100 with boundary injection (`inj-mult=1.0`) restores 6×6 active subspace, recovers M_stateSq,stateSq = +8.0 = b³ (trivial scaling) AND non-trivial flux→s² off-diagonal mixing (M_J⁴,stateSq = +6.47); s² → flux row at machine precision (asymmetric mixing — state is a sink under blocking, not a source). cond(S) 5.80×10⁷ (L=16, inj=3.0) → 8.74×10⁶ (L=32, inj=3.0) → 3.51×10⁷ (L=16, inj=1.0). Bootstrap-converged entries 6/25 → 7/25 → 10/36. Wilson positive eigenvalues 3/5 → 4/5 → 4/6. See `PROTOCOL_OPERATOR_MIXING_MATRIX.md` §7b and `engine/results/operator_mixing_2026-04-26/ANALYSIS.md` Appendices B + C.
- [~] Classify relevant, marginal, and irrelevant directions from measured native flow. **[PARTIAL] 2026-04-25** — `AUDIT_OPERATOR_SPECTRUM.md` (FTD-0091): all 5 measurable operators classify as "relevant" (Δ < D = 4) at L=32 in both the pulse and flux-baryon scenarios; the marginal/irrelevant bands of the pre-reg bracket are not recovered. Operator stratification IS present (divJ² Δ jumps ×3.4 between scenarios), so the basis is non-degenerate; the pulse-regime "all Δ ≈ 0.5" collapse is a scenario envelope artefact, not strong-coupling. Full classification requires L ≥ 64 + multi-scenario ensemble. **2026-04-26 (FTD-0098):** mixing-matrix eigenvalue diagnostic on the L=16 Langevin+genesis ensemble independently confirms the all-relevant compression — same finding via complementary mechanism.
- [ ] Separate engine-rule operators from emergent coarse operators.

## 6. Nonlinear Flow

- [x] Real engine histories can now be converted into native continuity ledgers.
- [x] Full-tick GPU histories expose all currently known state-changing channels.
- [~] Build systematic nonlinear b=2 flow campaigns from engine histories. **[PARTIAL] 2026-04-26 (FTD-0098)** — first nonlinear-flow campaign of this kind landed: `engine/tests/campaign_operator_mixing_2026-04-26.cpp` consumes the same Langevin+genesis ensemble as `test_nonlinear_flow_multiscale.cpp` and assembles M_ab(b=2). Pre-reg + analysis cited above.
- [~] Add BCC/corner-channel observables motivated by `PAPER_MASTER_QUADRATIC_FORMAL.pdf`. **2026-04-26 — Cluster A engine build complete (`engine/include/ftd/sublattice.h`, `correlations.h`, `spectrum_extraction.h`, `term_toggles.h::bcc_stencil`, `campaign_bcc_band_spectrum.cpp`, all tests PASS); D2 protocol drafted (`PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`); D1 derivation drafted (`DERIV_MECHANISM_C_GC_BCC_BRIDGE.md`, FTD-0093). Awaiting publication-grade run + D6 audit.**
- [~] Measure operator mixing under blocking. **[PARTIAL] 2026-04-26 (FTD-0098)** — first measurement landed (5×5 reduced subspace after pre-registered s² degradation ladder; bootstrap-stderr-limited at this ensemble size; tag = [PARTIAL]). See FTD-0098.
- [ ] Measure reaction-sector scaling.
- [ ] Measure transport-sector scaling.
- [ ] Measure mixed transport/reaction couplings.
- [ ] Pre-register any trace/determinant comparison to the master-quadratic Vieta data before measuring.
- [ ] Determine whether a nontrivial native fixed point exists. **R4 deliverable of the FTD-EFT roadmap. `SPEC_FTD_NATIVE_FIXED_POINTS.md` queued.**
- [ ] Determine whether continuum symmetries improve, degrade, or stay lattice-native under flow. **`AUDIT_LORENTZ_ANISOTROPY.md` (2026-04-25) provides one data point: anisotropy exponent p=4.0008±0.0006, rotation-breaking operator dimension 7 (Wilsonian-irrelevant). Need extension to other symmetries.**

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
- [ ] Derive the native effective action after b=2 blocking. **Definition fixed as `exp(-S_eff[H']) = sum_{H:B_b H=H'} exp(-S_H[H])`; explicit measured/fitted `S_eff` remains open. **CENTRAL R3 DELIVERABLE OF THE FTD-EFT ROADMAP** — `DERIV_FTD_NATIVE_EFFECTIVE_ACTION.md` queued. Plan: (R3a) extend M_ab campaign to L ∈ {64, 96, 128} with b=2 and b=4; (R3b) measure 12 pre-registered dim-6 operators; (R3c) classify relevant/marginal/irrelevant via observable drift across explicit L (per `SPEC_DISCRETE_NATIVE_DERIVATION.md` 2026-05-04 reframe — no continuous-RG framing); (R3d) write up explicit polynomial in $\{J, s\}$ with measured Wilson coefficients reproducing known phenomenology.**
- [~] Connect the action/measure to the observed operator-flow matrix. **[PARTIAL] 2026-04-26 (FTD-0098)** — operator-flow matrix M_ab(b=2) measured; explicit $S_\text{eff}$ connection is the R3d sub-deliverable above.

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

FTD now has a serious native EFT skeleton with a first measured operator-mixing matrix:

```text
fields -> constraints -> per-tick histories -> finite-volume ledger -> b=2 blocking
        -> operator basis (6 ops) -> M_ab(b=2) measured (FTD-0098, [PARTIAL])
```

The Gaussian/native bridge is closed enough to build on, the engine emits
full-tick GPU continuity histories for the state-changing sectors, and the
first non-trivial operator-mixing matrix has been measured (5×5 reduced
subspace, factor-96 eigenvalue stratification — basis non-degenerate).

FTD does **not** yet have a complete EFT. The missing core is:

```text
blocked nonlinear effective action from the selected native history measure
multilatitude (L≥64) operator-mixing matrix to recover marginal/irrelevant tiers
Wilson-coefficient extraction for clean fixed-point eigendirection identification
fixed-point / scaling classification from those measured flows
final Gauss/source-core representation
first-principles status of g_c
constructor-domain metadata and observable registry
```

**FTD-0099 update (2026-04-26):** F1 (multilatitude at L=32) closed positive — at L=32 the Wilson eigendecomposition recovers a fourth positive eigenvalue (3⁺/2⁻ at L=16 → 4⁺/1⁻ at L=32), and cond(S) improves 7×, confirming finite-sample noise as the leading-order constraint at L=16. F3 (Wilson eigendecomp) emitted for all configs. F5 (RG semigroup test M(b=4) ≈ M(b=2)·M(b=2)) closed negative — fails at the 50% threshold for both L (relerr 1.61–1.80×); bootstrap noise on the 4³–8³ b=4 grid is the leading-order explanation.

**FTD-0100 update (2026-04-26):** F2 (s² zero-variance break via parameter sweep) closed positive — `inj-mult=1.0` (injecting AT the genesis threshold rather than 3× above) restores Var(s²) > 0 and unlocks the first **full 6×6** mixing-matrix measurement. M_stateSq,stateSq = exactly +8.0 = b³ (trivial volume-weight scaling for an integer per-cell scalar); non-trivial flux→s² off-diagonal entries (M_J⁴,stateSq = +6.47); s²→flux row at machine precision — asymmetric mixing structure recovered (state crystallization is a sink for flux information under blocking, not a source). 4 of 6 FTD-0098 follow-ups closed. The over-saturation in the original FTD-0098 baseline is a parameter-regime artifact, not a fundamental property; future canonical mixing-matrix campaigns should use boundary injection (`inj-mult=1.0`).

The next milestone is **L=32/L=64 multilatitude continuation at `inj-mult=1.0`** to land a clean 6×6 at multiple scales (combines FTD-0099's L-trend + FTD-0100's 6×6 unlock). If cond(S) and bootstrap stderr both improve as predicted, this would be the upgrade path from [PARTIAL] toward [MEASUREMENT]. F4 (multi-scenario) and F6 (Vieta trace/det) remain open.

---

## Engine-as-Instrument Portfolio Verdict (2026-04-27)

Per the user's 2026-04-26 reorientation toward "use the engine as a primary instrument; let it produce phenomena and measure what emerges." Four-campaign portfolio executed in a single session on RTX 5090 / WSL2.

| # | Campaign | LEDGER | Outcome | Tag |
|---|---|---|---|---|
| A | BCC sub-stencil spectrum (Mechanism C closure) | FTD-0093 | Falsifier FAIL at L ∈ {24, 32, 48}; non-monotonic in L; FCC overlaps BCC at L=48 — basis-specificity removed | [CLOSED NEGATIVE] |
| B | Emergent particle spectrum from generic IC | FTD-0102 | Three-regime phase structure (vacuum / deterministic bound states / runaway crystallization); pre-registered Outcome A (discrete IC-invariant spectrum) NOT recovered | [PARTIAL] |
| C | Continuum-limit at L ∈ {16, 32, 64} for M_ab(b=2) | FTD-0103 | cond(S) monotonically improving (factor 18 over L=16→L=64); Wilson eigenvalue positivity non-monotonic; RG semigroup fails at all L | [PARTIAL] |
| D | Topological observable mapping (Wilson, flux tube, monopole, vacuum instanton) | FTD-0104 | See ANALYSIS_TOPOLOGICAL_OBSERVABLES.md | [PARTIAL] |

**Cross-campaign findings:**

- **All three first-principles routes for `g_c` (Mechanisms A, B, C) are now closed-negative.** The coupling remains [PARAMETRIC]. Master quadratic / G* / Watson identity unaffected (independent number-theoretic [THEOREMs]).
- **Engine has structural phase content not anticipated by SM-targeted measurements.** Campaign B's three-regime finding (vacuum / bound states / runaway crystallization) and the deterministic cluster counts (1 for point injection, 2 for collision; 5/5 seeds) are engine-native observations that SM-comparison framings would have masked or tuned away.
- **Continuum-limit measurement shows monotone cond(S) improvement but non-monotone eigenvalue positivity** at fixed N_SAMPLES — finite-sample noise is the dominant constraint, not basis closure or genuine L-dependence (consistent across FTD-0099/0103).
- **Negative results outnumber positive results in this portfolio**, by design. The user's reorientation explicitly framed negative results as informative; this portfolio confirms that framing produces structurally informative content even when prior conjectures fail.

**FTD-0094 (L2 candidate identity 2·m_e/α = 16G*²)** terminally demoted to [PARAMETRIC] per pre-registered FTD-0093 closure criterion. FTD-0096 (μ-from-ℓ_P missing arrow) remains [OPEN]; the demotion is conditional on FTD-0096 staying open.

**Status of the algebraic spine (refreshed 2026-05-05; spine count of 9 theorems unchanged from `SPEC_ALGEBRAIC_SPINE.md`):**
- FTD-0001 G* algebraic identity — [THEOREM]
- FTD-0013 master quadratic dual prediction (x₊ ≈ 1/α, x₋ ≈ N_c) — [STRONGLY MOTIVATED CONJECTURE]
- FTD-0014 master quadratic algebraic identity x² − 16G*²x + 16G*³ = 0 — [THEOREM]
- FTD-0015 CM-curve uniqueness h=1 — [NUMERICAL FACT, h=1 only] (downgraded 2026-05-02 — see MC-T1.2)
- FTD-0016 coefficient 16 = |Aut(E)|² — [THEOREM]
- Phase G geometric Coulomb α_r(r,L) = 2r·G_L(r) — [THEOREM]
- Watson identity W₃ = G*²/(2π) — [THEOREM]
- Phase J ultralocality at L=2 — [THEOREM at L=2 — Nyquist-mode degeneracy origin] (downgraded 2026-05-02 — see MC-T1.1)
- FTD-0111 (1+i)-tower harmonic invariant (Theorem 8) — [THEOREM]
- FTD-0112 field-theoretic Q(G*) characterization (Theorem 9) — [THEOREM]

These are independent of the engine-as-instrument campaigns and stand on number-theoretic + lattice-Green's-function grounds.

---

## R1 closure milestones (2026-04-27 → 2026-05-05)

The following items closed or substantively advanced since the previous checklist refresh (2026-04-26):

**2026-04-27 evening**: FTD-0107 cluster-tracker findings — deterministic L-invariant cluster counts at L ∈ {32, 64} from canonical genesis injection. 5/5 seed reproducibility. Cluster-↔-mass identification candidate bridge.

**2026-04-28**: FTD-0110 linear-level closure. $k = 1/N_\text{base} = 1/4$ derived from O_h representation theory (`DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`). Cluster-size formula $N(A) \approx \tfrac{1}{4}(A/K_\text{GENESIS})^2$ at the linear level — closes the long-standing "why 25 voxels at A=10" question structurally.

**2026-04-29**: FTD-0111 / Theorem 8 — (1+i)-tower harmonic invariant. $1/y_+ + 1/y_- = 1$ with anomaly transcendence $A_k \notin \mathbb{Q}$ for $k \geq 4$.

**2026-04-30**: Maxwell-exploit thread completion (`THEOREM_BLOCKING_DIAGONAL_IDENTITIES.md` $M_{JJ}=16$, $M_{J^4}=256$); `ANALYSIS_GATE_C_VS_L.md` (sector decoupling L-independent at L∈{24,32,48,64}); `AUDIT_GAUSSIANITY_v1_LARGE.md`; `ANALYSIS_OFFDIAGONAL_ASYMMETRY.md`; `MEASUREMENT_GATE_D_T_PERTURBATION.md`; `AUDIT_S_EFF_SMOKE_VALIDATION.md`. FTD-0112 / Theorem 9 — field-theoretic Q(G*) maximal π-free subfield characterization (conditional on Chudnovsky 1976).

**2026-05-01**: `AUDIT_LEMNISCATE_ALPHA_RIGIDITY.md` complete — Cayley-Dickson 5-harmonic curve found in ~4.3% of natural alternatives; retagged [SELECTION].

**2026-05-02**: Math-completion checklist Tier I 5/5 closed + Tier II 3/3 closed; `AUDIT_HEEGNER_TOWER_RIGIDITY.md` complete (CM Uniqueness bifurcates by methodology); Theorem 7 retagged [L=2 only]; Theorem 3 retagged [h=1 only].

**2026-05-03**: `PREREG_PHASE_I_NATIVE_COUPLING.md` + `PREREG_PHASE_II_WILSON_DIRAC_G2.md` + `SPEC_WILSON_DIRAC_FTD.md`. FTD-0127 (G* parity-twist between $\zeta$ and $L(s, \chi_{-4})$) [DERIVED]. Reflexivity-vocabulary sweep across theory + manuscript + whitepaper.

**2026-05-04**: Phase B cluster-persistence arc (FTD-0136) — full physics returns FTD framework integers across 18 runs at L ∈ {32, 64, 256}; FTD-0110 baseline drift discovered (cluster sizes ~5× smaller than 2026-04-28 LEDGER reference); engine bug-hunt commit `f2a721a` fixes F1 (color sign), F4 (genesis energy), F6 (evaporation toggle) + diagnostics; `SPEC_DISCRETE_NATIVE_DERIVATION.md` methodological pivot.

**2026-05-05** (this R1 refresh): engine cleanup commits `c714f71`, `56985a4`, `2881238`, `255c1dd`, `37c3fcd`, `10f00f9`, `c887948`, `2504c9b`, `8b1a750` close out CALLSTACK F1–F10 + bug-hunt F2/F3/F12/F15 + add `toggles.evaporation` flag + `gpu_continuity_ledger` test fix + RNG portability design note + STATUS doc.

**2026-05-05 evening** (BH-F5/F8/F9 RNG portability — Option A landed): commits `c1a4f88` (shared SplitMix64 header + CPU refactor) + `c8e03a5` (GPU plumbing in genesis_kernel, genesis_dual_kernel, phase_write_kernel, wave_update_kernel, launchers, gpu_engine dispatch). Both backends now share `engine/include/ftd/voxel_rng.h`'s SplitMix64 stream for: (a) genesis Boltzmann probability; (b) genesis zero-curl spin fallback (BH-F8 fix — pre-fix GPU assigned deterministic +1 vs CPU's stochastic ±1); (c) Langevin OU Gaussian noise (Box-Muller of two SplitMix64 uniforms per axis). Verified: golden hash bit-exact, gpu_parity_complete PASS at 41s (3-4× slower than pre-fix due to in-kernel Box-Muller transcendentals; parity holds per-voxel), genesis/baryogenesis/toggle_matrix PASS. Out of scope: weak_transmutation + pair_production kernels still use cuRAND (separate BH-F5 follow-up); GPU evaporation criterion uses deterministic threshold while CPU is stochastic Boltzmann (a CRITERION divergence, not RNG portability — separately deferred).

**Engine-side CPU↔GPU parity work is now substantially complete.** All bug-hunt audit items from the 2026-05-04 commit `f2a721a` body — F1, F2, F3, F4, F5 (genesis/Langevin RNG portion), F6, F8, F9, F12, F15 — are closed at HEAD `c8e03a5`. Residual items are: (i) deterministic-vs-stochastic GPU evaporation criterion, (ii) weak_transmutation + pair_production cuRAND-to-SplitMix64 migration, (iii) FTD-0110 baseline drift bisect (research, separate session). None of these block the R3a measurement campaign.

---

## R2–R6 deliverables (FTD-EFT roadmap, planned)

Per `INDEX_FTD_NATIVE_EFT.md` §16:

- **R2** (2–3 weeks): `DECISION_FIELD_BASIS.md`, `DECISION_GAUSS_REPRESENTATION.md`, `DERIV_18PT_LAPLACIAN_VARIATIONAL.md`, `DERIV_DAMPING_RAYLEIGH.md`, `DERIV_EM_REGIMES_UNIFIED.md`. Closes §1, §2, §7, §8 production decisions.
- **R3** (6–8 weeks, central): explicit nonlinear blocked $S_\text{eff}[J, s]$. Closes §6, §9 nonlinear-flow + action items.
- **R4** (4–6 weeks): `ANALYSIS_BETA_LATTICE_NATIVE.md`, `SPEC_FTD_NATIVE_FIXED_POINTS.md`. Closes the §6 fixed-point determination.
- **R5** (8–12 weeks): inter-scale formalisation Scales 0→1, 1→2, 2→3, 3→4 — moves the inter-scale story from operational-only to formalised.
- **R6** (3–4 weeks): synthesis manuscript `PAPER_FTD_NATIVE_EFT.tex`. Updates Paper C; new LEDGER entries (FTD-0140+); refresh `TRACKER_ONTIC_TRUTH.md`.

Total wall-clock: ~3–6 months for a small focused team. See `C:\Users\cpaci\.claude\plans\we-fixed-a-lot-composed-reef.md` for the approved roadmap.
