# PREREG — Volumetric measure reconciliation

**Prospective claim ID:** FTD-0404 (registry maximum FTD-0403; census GREEN before lock).  
**Tag:** `[PRE-REGISTRATION — DIMENSIONAL IMPLEMENTATION RECONCILIATION]` · LOCK-STD v1 · git tag `preregister-volumetric-measure-reconciliation-v1`.  
**Question:** can the current three-dimensional unit lattice make its hidden cell measure explicit without changing any production result, confusing density with integrated energy, or claiming that spatial dimension fixes the algebraic degree of local norms?

## 1. Frozen scope

The production representation remains a cubic lattice with unit edge

```text
a_lat = 1
A_face = a_lat^2
V_cell = a_lat^3
```

For every existing local energy density `rho_i`, the corresponding volume-integrated channel is

```text
E = sum_i rho_i * V_cell.
```

The local field density remains quadratic, for example

```text
rho_field = 1/2 |J|^2
rho_wave  = 1/2 |wave_vel|^2.
```

Three-dimensionality enters through `V_cell=a_lat^3`, not by replacing Euclidean squares with component cubes. Point-particle rest/kinetic energy, particle momentum, charge counts, pair-potential energy, causal norms, and Gauss residual definitions are not volume densities and must not acquire a cell-volume multiplier.

The latency Poisson source consumes local density `T00_i`, not the already integrated per-cell energy `T00_i*V_cell`. Its current unit-spacing discrete operator remains unchanged. Supporting arbitrary non-unit `a_lat`, including Laplacian and coupling rescaling, is outside scope and requires a new lock.

## 2. Frozen implementation boundary

Permitted production changes are limited to:

1. one CUDA-safe header defining the unit edge, square face area, cubic cell volume, density integration, and the current ordinary field/wave density;
2. explicit `V_cell` factors on existing volume-summed EnergyAudit channels in CPU and GPU diagnostics;
3. explicit `V_cell` factors on existing spatially summed Lagrangian/Hamiltonian diagnostic densities;
4. explicit use of the local-density helper in CPU field-energy gravity with no volume multiplier;
5. append-only EnergyAudit/Lagrangian metadata through WASM and its direct JS consumer;
6. exact/native/parity tests and documentation.

No force law, tick phase, state transition, Poisson value, causal budget, particle energy, calibration, toggle default, framework type, strong Hamiltonian, interaction-energy term, or gravity coupling may change. `strong_energy` remains a diagnostic sub-channel and remains excluded from `total_energy` pending NCEMC.

## 3. Exact anchors

The recomputing verifier and native unit test must establish:

1. `D_SPATIAL=3` and `V_cell(a)=a*a*a`.
2. `a_lat=1`, `A_face=1`, and `V_cell=1` exactly.
3. At test edge `a=2`, `A_face=4` and `V_cell=8`.
4. Density `rho=7/2` integrates to cell energy `28` at test edge `a=2`.
5. `|J|^2=J_x^2+J_y^2+J_z^2` remains rotationally invariant and quadratic.
6. A one-site `J=(1,2,2)` field has `rho_field=9/2`; at the production edge, both the density sum and integrated field energy are `9/2` bit-exactly.
7. Point-particle `E_REST` and momentum retain the FTD-0402 values with no `V_cell` factor.
8. The field-energy gravity source reads local `rho_field+rho_wave`; it must not call the density-integration helper.
9. CPU and GPU expose identical cell-volume and integrated-energy semantics.
10. Existing WASM EnergyAudit indices 0–24 and Lagrangian indices 0–15 retain their meanings; new metadata is append-only.

## 4. Correctness and vacuity gates

- **N1 numerical neutrality:** because `V_cell=1`, every pre-existing EnergyAudit, Lagrangian, latency, golden, and browser value must remain unchanged within its pre-existing exact/tolerance contract. Any accepted golden-hash change is forbidden.
- **N2 cube non-vacuity:** the pure measure helper must be tested at `a=2`, where the volume is 8 rather than 1 or 4.
- **N3 density/integral separation:** the energy audit must expose both the density sum and the integrated value; the test must show equality only because the production cell volume is one.
- **N4 point/volume separation:** particle rest energy, kinetic energy, momentum, charge, and Coulomb pair energy must not be multiplied by `V_cell`.
- **N5 source separation:** the gravity source uses local density, not integrated cell energy.
- **N6 epistemic ceiling:** no result may be described as deriving three dimensions, a mass scale, confinement energy, gravity, or a stress tensor. This is dimensional bookkeeping made explicit.

## 5. Frozen verification surface

No full CTest run is permitted or required.

### T1 — exact verifier

Run the new `scripts/proofs/verify_volumetric_measure_reconciliation.py`; all exact/source-contract checks must pass.

### T2 — native changed surface

Run exactly:

```text
volumetric_measure
audit_regression
lagrangian
action_stationarity
causal_normalization
```

### T3 — CUDA changed surface

With `FTD_FORCE_GPU` unset, run exactly:

```text
gpu_parity
gpu_parity_complete
causal_normalization
```

### T4 — numerical-neutrality goldens

Run `ctest -L golden -j 24 --output-on-failure`; all seven registered goldens must retain their accepted hashes.

### T5 — WASM and direct web contracts

Build only `ftd_wasm` in Release. Run the physical-energy test in `scale0-conservation-panel.spec.js` and both tests in `scale0-scenario-telemetry-contract.spec.js`.

### T6 — repository contracts

Run `git diff --check`, added-link checks, a static scan excluding component-cube causal/energy formulas, and `tools/preregister_census.py` with GREEN result.

## 6. Frozen outcomes and precedence

After correctness gates:

1. **VOLUMETRIC-NEUTRAL:** the cubic cell measure is explicit, every gate passes, and all pre-existing production values remain unchanged.
2. **PARTIAL:** the exact/native measure closes, but a required parity, WASM, or documentation surface is incomplete without a failing correctness gate.
3. **INVALID:** any exact anchor, numerical-neutrality gate, point/volume separation, source-density separation, parity, compatibility, or golden gate fails.

Precedence is `INVALID` over `PARTIAL` over `VOLUMETRIC-NEUTRAL`.

## 7. Licensed interpretation

`VOLUMETRIC-NEUTRAL` licenses only `[THEOREM — current engine volume-integrated diagnostic channels explicitly use the cubic unit-cell measure]`. It does not establish a physical stress–energy tensor. It prepares, but does not execute, NCEMC. FTD-0400–0403 and every mass/confinement/gravity tag remain unchanged.

## 8. Executor and window

Executor: current Codex repository session on `codex/ftd-0402-causal-normalization`.  
Window: tag creation through `2026-07-24T22:15:00Z` (72 hours).  
Platform: Windows 11 host; WSL2 Ubuntu 22.04 canonical `engine/build_wsl`; RTX 5090 for CUDA gates.
