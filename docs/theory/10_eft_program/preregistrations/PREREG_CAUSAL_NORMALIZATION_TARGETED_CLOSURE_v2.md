# PREREG — Causal normalization targeted dependency closure v2

**Prospective claim ID:** FTD-0403 (v2 adjudication of the same supplemental closure claim; registry maximum remains FTD-0402).  
**Tag:** `[PRE-REGISTRATION — SCOPED REGRESSION CLOSURE]` · LOCK-STD v1 · git tag `preregister-causal-normalization-targeted-closure-v2`.  
**Scope:** a proportional replacement for FTD-0402 gate G9's repository-wide CTest requirement. No full CTest run is permitted or required. This lock changes no production engine behavior, framework type, causal formula, mass role, or FTD-0402 frozen verdict.

## 1. Prior-attempt disposition and frozen repair

FTD-0403 v1 returned `INVALID` after 13 of 14 native changed-surface tests passed. Its historical `boundary_movement` fixture directly assigned raw `velocity.x=-1`, outside the FTD-0402 causal boundary `C_SPEED=1/sqrt(3)`, then required a one-tick face crossing. Correct movement-entry projection reduced that speed below one cell per tick, invalidating the fixture premise.

The only executable change after v1 is test commit `4325b36a`: both boundary arms now use raw `velocity.x=-0.5*C_SPEED` and `remainder.x=-0.75`. The accumulated displacement crosses the face while the velocity remains inside the causal domain. Both arms also require zero causal-projection events. The repaired test source SHA256 is `aa3538c47d048e78f8ddcd6ae753a0924a7cc8679cca6a6ccd802f86ae4d0d06`.

No production engine or UI source changed after implementation commit `6526fefa637cd0c0d1feb56e421bdc60c19a290e`. V1's `INVALID` verdict remains immutable provenance.

## 2. Frozen causal and mass-role contract

The selected current-engine contract remains:

```text
C = C_SPEED
beta^2 = |u|^2 / C^2
f = 1 - L^2
B = beta^2 + L^2 < 1
d_tau/dt = sqrt(max(1-B, 0))
gamma_FTD = 1 / sqrt(1-B)
u_max = C sqrt(max(f, 0))
M_INERTIAL = K_B
E_REST = M_INERTIAL C^2
M_GRAVITATIONAL = K_B  [IMPOSED numerical equality]
```

This is an implementation-consistency contract for the existing clock/bandwidth axiom. It derives neither the clock law nor a mass scale.

## 3. Frozen execution surface

### T1 — exact contract verifier

Run `python scripts/proofs/verify_causal_normalization_mass_roles.py`. All A1–A7 and S1–S9 checks must pass.

### T2 — native changed-surface CTests

Run exactly one fresh WSL2 CTest selection containing:

```text
causal_normalization
tick_phase_order
voxel_properties
born_infeld
lorentz
lagrangian
gamma_ftd_momentum
de_broglie_redshift
cluster_inertia
energy_conservation
energy_conservation_tight
full_state_irreversibility
boundary_movement
symmetric_movement
```

The repaired `boundary_movement` target must demonstrate a face crossing in both modes with zero causal projection. The prior FTD-0402 duplicate runs remain the determinism evidence for unchanged targets.

### T3 — CUDA changed-surface parity CTests

With `FTD_FORCE_GPU` unset, run exactly:

```text
gpu_evaporation_parity
gauge_gpu_parity
force_diag_parity
causal_normalization
gpu_parity
gpu_parity_complete
```

GPU tests remain serialized by their CTest resource lock.

### T4 — golden regression boundary

Run `ctest -L golden -j 24 --output-on-failure` once. All seven registered golden tests must pass with the accepted FTD-0402 hashes.

### T5 — WASM and direct web contracts

Build only the `ftd_wasm` target in Release. Run:

- `engine/web/tests/time-analysis.node.test.mjs`;
- the physical-energy WASM contract in `scale0-conservation-panel.spec.js`;
- `scale0-scenario-telemetry-contract.spec.js`.

### T6 — repository contract checks

Run `git diff --check`, an added-link checker for v2 documentation, a static `rg` check confirming that no production consumer uses `M_REST` beyond compatibility aliases, and `tools/preregister_census.py` with a GREEN result.

No disabled, campaign, long-running, unrelated, or full-suite CTest may be invoked as evidence.

## 4. Correctness and vacuity gates

1. The exact verifier must pass every exact anchor and reject the legacy raw-`c=1` normalization.
2. The causal test must exercise base, color, Yukawa, and exchange force paths, external projection, and ordinary zero-projection controls.
3. CPU/GPU parity must cover one and sixteen ticks including `tau`, phase, evaporation, causal budget, energy, and momentum.
4. WASM indices 0–18 remain fixed and appended FTD-0402 fields remain in their locked positions.
5. `boundary_movement` must cross through accumulated remainder without invoking the causal projector; a test that merely avoids movement is vacuous.
6. No production engine or UI source may change between this lock and execution.
7. The original FTD-0402 `PARTIAL` and FTD-0403 v1 `INVALID` verdicts must not be rewritten.

## 5. Frozen outcomes and precedence

Apply correctness gates first, then:

1. **TARGETED-CLOSURE:** T1–T6 all pass. This independently licenses closure of `§12-cnorm` for the selected current-engine causal/mass-role contract. FTD-0402 remains historically `PARTIAL`; FTD-0403 supplies the missing proportional regression result.
2. **REMAINS-PARTIAL:** an environmental or tooling interruption prevents a required T2–T5 target from producing a verdict, with no test failure and all completed correctness gates passing.
3. **INVALID:** any exact, native, parity, golden, WASM-contract, compatibility, source-boundary, or vacuity gate fails.

Precedence is `INVALID` over `REMAINS-PARTIAL` over `TARGETED-CLOSURE`. A relevant failure cannot be reframed as environmental interruption.

## 6. Licensed interpretation

`TARGETED-CLOSURE` licenses only `[THEOREM — current engine implementation conforms to the selected raw-lattice causal and mass-role contract over the frozen changed surface]`. It does not derive `K_B`, an electron mass, covariance, inertial–gravitational equivalence, confinement energy, a strong Hamiltonian, or a common stress–energy source.

On `TARGETED-CLOSURE`, `§12-cnorm` may close and NCEMC becomes admissible as a separately locked successor. FTD-0015, FC-2, FC-W, FTD-0208, FTD-0252/0268, FTD-0400, FTD-0401, and the original FTD-0402 verdict retain their recorded tags.

## 7. Execution window and executor

Executor: the current Codex repository session on branch `codex/ftd-0402-causal-normalization`.  
Window: from tag creation through `2026-07-24T21:15:00Z` (72 hours).  
Platform: WSL2 Ubuntu 22.04, canonical `engine/build_wsl`, RTX 5090 for CUDA gates.
