# PREREG — Causal normalization targeted dependency closure

**Prospective claim ID:** FTD-0403 (registry and preregistration census rechecked before lock; FTD-0402 is the current maximum).  
**Tag:** `[PRE-REGISTRATION — SCOPED REGRESSION CLOSURE]` · LOCK-STD v1 · git tag `preregister-causal-normalization-targeted-closure-v1`.  
**Scope:** a proportional replacement for FTD-0402 gate G9's repository-wide CTest requirement. It changes no production code, framework type, causal formula, mass role, or FTD-0402 frozen verdict. It asks whether a fresh, exact dependency-closure regression run is sufficient to close `§12-cnorm` without running unrelated campaigns.

## 1. Frozen rationale

FTD-0402 returned `PARTIAL` solely because a full CTest aggregate did not finish. Its exact anchors, causal enforcement, mass-role propagation, CPU/GPU parity, deterministic repeated targets, goldens, WASM build, browser contracts, and documentation gates passed. A repository-wide suite includes campaigns with no dependency on the changed causal/mass surface and multi-hour runtimes; their inclusion is not a proportional regression gate for this contract.

This supplemental lock preserves FTD-0402 as executed. It independently tests the transitive changed surface and may close the remaining normalization gate only within that scope.

## 2. Frozen source boundary

The implementation boundary is commit `6526fefa637cd0c0d1feb56e421bdc60c19a290e`, with no production engine changes permitted after it for this campaign. Documentation-only commits `e2583b25` and `f916c49f` do not alter the executable surface.

The changed surface comprises:

- causal kinematics, `Voxel`, proper time, Born–Infeld, momentum integration, movement, evaporation, and phase;
- inertial/rest/gravitational mass-role consumers and energy-ledger diagnostics;
- CUDA force/Poisson buffers and CPU/GPU synchronization;
- WASM energy-audit indices and directly consuming Scale-0 telemetry/UI code;
- golden states whose velocity, remainder, flux, acceleration, and audit fields changed downstream.

## 3. Frozen test closure

No full CTest run is permitted or required. Execute exactly these gates:

### T1 — exact contract verifier

```text
python scripts/proofs/verify_causal_normalization_mass_roles.py
```

All A1–A7 and S1–S9 checks must pass.

### T2 — native changed-surface CTests

One fresh WSL2 run of:

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

The prior FTD-0402 duplicate runs supply the determinism repetition; this fresh run is a post-lock confirmation, not a third repetition requirement.

### T3 — CUDA changed-surface parity CTests

One fresh WSL2 run of:

```text
gpu_evaporation_parity
gauge_gpu_parity
force_diag_parity
causal_normalization
gpu_parity
gpu_parity_complete
```

GPU tests remain serialized by their CTest resource lock. `FTD_FORCE_GPU` is unset.

### T4 — golden regression boundary

Run `ctest -L golden -j 24 --output-on-failure` once. All seven registered golden tests must pass with the FTD-0402 hashes.

### T5 — WASM and direct web contracts

Build `ftd_wasm` in Release, run `engine/web/tests/time-analysis.node.test.mjs`, the physical-energy WASM contract in `scale0-conservation-panel.spec.js`, and `scale0-scenario-telemetry-contract.spec.js`.

### T6 — repository contract checks

Run `git diff --check`, the added-link checker for this campaign's documentation, `rg` confirming `M_REST` has no production consumer beyond compatibility aliases, and `tools/preregister_census.py` with GREEN result.

## 4. Correctness and vacuity gates

1. The causal test must exercise all four force paths (base, color, Yukawa, exchange), external projection, and ordinary zero-projection controls.
2. CPU/GPU comparison must cover one and sixteen ticks, including `tau`, phase, evaporation, causal budget, energy, and momentum.
3. Fixed WASM indices 0–18 must remain unchanged and appended fields must retain their FTD-0402 positions.
4. The exact verifier must still reject the legacy raw-`c=1` anchor at `u=C_SPEED,L=0`.
5. No production engine or UI source may change between this lock and execution; a source change invalidates reuse of the prior duplicate evidence.
6. No unrelated disabled, campaign, long-running, or full-suite CTest may be invoked as evidence.

## 5. Frozen outcomes and precedence

Apply correctness gates first, then:

1. **TARGETED-CLOSURE:** T1–T6 all pass. This independently licenses closure of `§12-cnorm` for the selected current-engine causal/mass-role contract. FTD-0402 remains historically `PARTIAL`; FTD-0403 supplies the missing proportional regression result.
2. **REMAINS-PARTIAL:** an environmental/tooling interruption prevents a required T2–T5 target from producing a verdict, with no test failure and all completed correctness gates passing.
3. **INVALID:** any exact, native, parity, golden, WASM-contract, compatibility, source-boundary, or vacuity gate fails.

Precedence is `INVALID` over `REMAINS-PARTIAL` over `TARGETED-CLOSURE`. A failing relevant test cannot be reframed as an environmental interruption.

## 6. Licensed interpretation

`TARGETED-CLOSURE` licenses only `[THEOREM — current engine implementation conforms to the selected raw-lattice causal and mass-role contract over the frozen changed surface]`. It does not derive the clock budget, `K_B`, an electron mass, covariance, inertial–gravitational equivalence, confinement energy, a strong Hamiltonian, or a common stress–energy source.

On `TARGETED-CLOSURE`, `§12-cnorm` may close and NCEMC becomes admissible as a separately locked successor. FTD-0015, FC-2, FC-W, FTD-0208, FTD-0252/0268, FTD-0400, FTD-0401, and the original FTD-0402 verdict retain their recorded tags.

## 7. Execution window and executor

Executor: the current Codex repository session on branch `codex/ftd-0402-causal-normalization`.  
Window: from tag creation through `2026-07-24T21:05:24Z` (72 hours).  
Platform: WSL2 Ubuntu 22.04, canonical `engine/build_wsl`, RTX 5090 for CUDA gates.
