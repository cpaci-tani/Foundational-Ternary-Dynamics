# FTD Native EFT Checklist

**Date:** 2026-04-24
**Status:** [PARTIAL] Native Gaussian bridge and GPU full-tick ledgers are in place; full nonlinear EFT remains open.

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
- [ ] Re-run and archive a fresh full campaign table after the GPU-ledger changes.
- [ ] Decide whether Gaussian fixed-point data is a theorem-level result or a measured engine result.

## 5. Operator Basis

- [x] Initial operator basis pre-registered.
- [x] Current, reaction, source-response, and transport moments represented.
- [x] Native operator spectrum test exists.
- [x] Ward-identity and matched-Poisson tests exist.
- [ ] Extend basis with all nonlinear/reaction operators observed in full-tick GPU histories.
- [ ] Define operator mixing matrix from blocked full-history ensembles.
- [ ] Classify relevant, marginal, and irrelevant directions from measured native flow.
- [ ] Separate engine-rule operators from emergent coarse operators.

## 6. Nonlinear Flow

- [x] Real engine histories can now be converted into native continuity ledgers.
- [x] Full-tick GPU histories expose all currently known state-changing channels.
- [ ] Build systematic nonlinear b=2 flow campaigns from engine histories.
- [ ] Add BCC/corner-channel observables motivated by `PAPER_MASTER_QUADRATIC_FORMAL.pdf`.
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
- [ ] Derive or demote `g_c`: `OPEN_GC_FROM_FIRST_PRINCIPLES.md` remains open.
- [ ] Evaluate "G18 direct sector vs BCC/CM capacity matching" as a possible `g_c` mechanism, tagged [CONJECTURE] until derived.
- [ ] Decide whether `coulomb_charge_coupling` is a measurement knob, calibration, or derived engine convention.
- [ ] Separate dimensionless native couplings from physical-unit calibrations in all docs and APIs.

## 9. Statistical Measure / Action

- [x] L=2 partition function exists as a first explicit finite example.
- [ ] Define the full native path measure over state/flux histories.
- [ ] Decide whether the EFT object is an action, transfer matrix, Markov/deterministic pushforward measure, or constrained history measure.
- [ ] Decide whether the BCC Watson denominator `1 - cos(kx) cos(ky) cos(kz)` enters the native transfer/action kernel.
- [ ] Include deterministic update Jacobian or prove it cancels/does not enter.
- [ ] Include stochastic genesis/Langevin sectors in the measure when toggled on.
- [ ] Derive the native effective action after b=2 blocking.
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
- [ ] Update `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` to reflect GPU full-tick ledger closure for genesis/evaporation.
- [ ] Move closed-negative projected-EFT attempts into an archive folder after link updates.
- [ ] Add a native EFT capstone summary replacing the old alpha-recovery narrative.
- [ ] Keep this checklist updated after every bridge milestone.

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
native history measure/action
operator mixing matrix from nonlinear full histories
fixed-point / scaling classification from those measured flows
final Gauss/source-core representation
first-principles status of g_c
```

The next milestone should be the nonlinear operator-flow campaign: consume
`RenderBridge::continuity_step()` histories, block them, compute the operator
moment vector before/after blocking, and assemble the first measured native
mixing matrix.
