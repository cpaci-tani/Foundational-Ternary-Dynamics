# FTD-0404 — Volumetric measure reconciliation result

**Frozen outcome:** **VOLUMETRIC-NEUTRAL**

**Lock:** `preregister-volumetric-measure-reconciliation-v1` at commit `adb35cdb601f8b49a607176d861fa31693353a6d`; preregistration SHA256 `270ad712890e90d932e07aa62cf2572353c54867ac60cf4a80902dee25f06b36`.

## Verdict

The current three-dimensional unit lattice now represents its spatial measure explicitly:

\[
A_{\rm face}=a_{\rm lat}^2,\qquad
V_{\rm cell}=a_{\rm lat}^3,\qquad
E=\sum_i \rho_i V_{\rm cell}.
\]

Production remains at `a_lat=1`, so `V_cell=1` and every pre-existing numerical energy, action, latency, golden, and browser value is unchanged. The change separates a local density from its volume-integrated value without altering the local algebra. In particular,

\[
\rho_{\rm field}=\tfrac12|J|^2,\qquad
\rho_{\rm wave}=\tfrac12|\dot J|^2
\]

remain quadratic and rotationally invariant. Three-dimensionality supplies the integration measure; it does not replace squared norms by component cubes.

## Implemented boundary

- A CUDA-safe measure interface defines unit edge, square face area, cubic cell volume, quadratic field/wave densities, and density integration.
- CPU and GPU energy audits explicitly integrate volume-density channels and expose `cell_volume`, `field_energy_density_sum`, and `wave_energy_density_sum` append-only.
- Lagrangian and Hamiltonian diagnostic densities are explicitly volume-integrated and expose the same cell volume.
- The latency Poisson solver explicitly consumes local field/wave density, not already integrated cell energy.
- Point-particle rest/kinetic energy, momentum, charge, Coulomb pair energy, causal norms, and Gauss residuals receive no cell-volume multiplier.
- WASM retains EnergyAudit indices 0–24 and Lagrangian indices 0–15; the new metadata is appended and verified through the direct browser consumer.

No force law, tick phase, state transition, coupling, calibration, gravity strength, interaction energy, or toggle default changed. Strong energy remains diagnostic and excluded from accounted total energy pending NCEMC.

## Frozen gates

| Gate | Result | Evidence |
|---|---|---|
| T1 exact verifier | PASS | Five arithmetic anchors and twelve source-contract checks; `VOLUMETRIC-CONTRACT-PASS` |
| T2/T3 changed native and CUDA surface | PASS 7/7 | `volumetric_measure`, audit, Lagrangian/action, causal normalization, and both GPU parity targets; `FTD_FORCE_GPU` unset |
| T4 numerical-neutrality goldens | PASS 7/7 | Every accepted golden remains unchanged |
| T5 WASM/web | PASS | Release WASM build; physical-energy contract 1/1; scenario telemetry 2/2; runtime measure metadata verified |
| T6 repository contracts | PASS | `git diff --check`, static component-cube exclusion, link checks, and preregistration census GREEN |

The first C++ test invocation used a zero tolerance with a helper whose comparison requires a positive tolerance. Every reported numerical difference was exactly zero. The instrument tolerance was corrected to `1e-15`, and the three affected targets then passed 3/3. This was a test-harness correction, not an anchor or production-value failure.

No full CTest aggregate, unrelated campaign, numerical near-miss search, or substitution search was run.

## Licensed consequence

This result licenses only:

> **[THEOREM — current engine volume-integrated diagnostic channels explicitly use the cubic unit-cell measure].**

It does not derive three spatial dimensions, a physical stress–energy tensor, a mass scale, confinement energy, gravity, or a strong Hamiltonian. It prepares the density-versus-integral boundary required by NCEMC but does not execute NCEMC. FTD-0400–0403 and every mass, confinement, gravity, FC-2, and clock tag remain unchanged.

## Reproduction record

- Implementation commit: `92535fe76d43541486bc67927849d36a7cc9d066`
- Platform: WSL2 Ubuntu 22.04, Linux `6.6.87.2-microsoft-standard-WSL2`, NVIDIA GeForce RTX 5090, driver `610.47`
- Volumetric test source SHA256: `f51899ee71622b9c1c5ee31a0143e900e940d6d138d2aaf4f5a6bf33c1254e1c`
- Volumetric test binary SHA256: `9d5138c239a2968d7a1abbdf87ff32b7efa72a5c588dcc8b6c1893df100876b7`
- Exact verifier SHA256: `bc9a674f2bbec63b150f44b47130744f6e9b5f3c6e546a06179c0cd892912de9`
- WASM SHA256: `4d4e46524616d9da95be43cc66f4a2e9cf17c6c0b6863da2633c548a28615508`
- Effective execution: CPU-forced only where target fixtures require it; `FTD_FORCE_GPU` unset for CUDA gates; production/default toggles otherwise unchanged
- Raw command and count record: `engine/results/volumetric_measure_reconciliation_2026-07-21/verification.txt`
