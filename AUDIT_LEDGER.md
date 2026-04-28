# Physics Stack Audit Ledger — 2026-04-27

Working ledger for the comprehensive sweep. Each row tracks one finding from
the 6-agent parallel audit. Updated as fixes land.

Status legend: `[ ]` open · `[x]` fixed · `[~]` partial · `[d]` deferred (with reason) · `[n]` not-a-bug after re-check

## Already-landed before this sweep (commits aa83cd8 and earlier)

- [x] **C-1** field_energy / wave_energy / E_L_total / E_R_total / wv_L_total / wv_R_total missing ½ in C++ Audit (`engine/src/diagnostics_compute.cpp`)
- [x] **C-2** coulomb_pe missing ½ in C++ Audit (same file)
- [x] **C-3** MockBridge `weak_transmutation=true` default tripped validator
- [x] **C-4** Neutrino masses 1000× off (×1e-3 stray factors in `particle-catalog.js`)
- [x] **C-5** `meta-pedagogy.js` orphan `N_C/N_BASE/B_3/N_EFF` redeclaration
- [x] **C-6** `resources/data/constants.json` EPSILON sign + `x_plus_tree` 47 ppm drift
- [x] **F-1** MockBridge `_computePairwiseForces` outer-loop locked-skip bug (commit dc329d6)
- [x] **F-2** MockBridge absorbing boundary — graded sponge layer (commit 98115e7)
- [x] **F-3** MockBridge particle drop-out at edge when reflective=OFF (commit 1a862d8)
- [x] **F-4** wire.js fluxMock mirroring for live UI controls (commit fcb9320)
- [x] **F-5** MockBridge pairwise Coulomb PE (commit d78129c)

## TRACK A — Cross-scale convention drift (physics-orchestrator agent)

- [ ] **A-2 (HIGH)** Coulomb prefactor convention split: 4 different prefactors across mock-particle-engine.js, mock-diagnostics.js, mock-lattice-samplers.js, fields.js, pe-telemetry.js, cross-sections.js, spectroscopy.js. Add canonical `COULOMB_K_LATTICE` and `COULOMB_K_PE` exports to constants.js with explicit per-scale comments.
- [ ] **A-3 (HIGH)** Strong-force tuning constants hardcoded in `wasm-bridge-dag.js:462-471` and `mock-lattice-samplers.js:725-756`: ALPHA_S=1.0, run-coeff 0.1, regime cutoffs 3.0/8.0, linear coeff 64.0, color factors ±0.5/±1.0. Add 7 named constants to constants.js.
- [ ] **A-4 (MEDIUM)** Scale-4 G value misleading comment in `mock-scale4.js:8-13` (claims G=G_N=0.01 reproduces Kepler at AU/yr units, would require G≈4π²). Either rename or fix.
- [ ] **A-5 (MEDIUM)** `units.js:42` mislabels PLANCK_TIME_S as the FTD tick unit. Add `FTD_TICK_S = √3·PLANCK_TIME_S` to constants.js.
- [ ] **A-6 (MEDIUM)** Cosmic mass-unit calibration undeclared (`bridge/cosmic-physics.js`, `mock-scale5.js`). Add `LATTICE_TO_SOLAR_MASS = 50.0` to constants.js.
- [ ] **A-7 (MEDIUM)** `H0_LATTICE` declared but never integrated into `a(t)`. Either implement Friedmann or downgrade to placeholder.
- [ ] **A-8 (MEDIUM)** `units.js:45-64` orphan SI literals (PLANCK_MASS_KG, PLANCK_FORCE_N, J_PER_EV, C_MS) — promote to constants.js.
- [ ] **A-9 (MEDIUM)** `mock-scale4.js:93-98` hardcoded G=1.0 for figure-8 scenario bypasses bridge G semantics — flag with explicit `_threebody_G` field.
- [ ] **A-10 (MEDIUM)** `cross-sections.js`, `spectroscopy.js`, `decay-rates.js` use `K_B` (FTD anchor 0.511) for electron mass instead of `M_E_PHYS = 0.51099895`. Switch when comparing to PDG.
- [ ] **A-11 (MEDIUM)** `wasm-bridge-dag.js:445` gravity uses `K_B²` instead of per-particle masses (mock-particle-engine.js correctly uses pj.mass). Align.
- [ ] **A-13 (LOW)** `AE_K_COULOMB` is MD-tuning, not α/(4π) — already labeled `[IMPOSED]` in constants.js but needs a discoverability comment.
- [ ] **A-14 (LOW)** `particle-catalog.js` mixes derived (lepton) and PDG (quark/baryon) masses without `units_source` tag.
- [ ] **A-15 (LOW)** AE_* MD constants in constants.js have no unit comments.
- [ ] **A-16 (LOW)** `R_BOHR` (FTD natural) and `BOHR_RADIUS_M` (PDG SI) lack a documented conversion factor.
- [ ] **A-17 (LOW)** `scale11/scenario-loader.js:44` uses `K_B*0.3` as "consciousness amplitude" — should be a named constant.
- [ ] **A-18 (LOW)** `scale0/ui/controls/substrate-controls.js:54` K_B slider hardcodes value="0.511" instead of templating from K_B.
- [ ] **A-19 (LOW)** `decay-rates.js:94` uses `M_ELECTRON = K_B = 0.511` for Gamow phase space (should be `M_E_PHYS` for PDG comparison).
- [ ] **A-21 (LOW)** `pe-telemetry.js:390-391` Coulomb force-constant inspector readout — verify sign + 4π match `mock-particle-engine.js:144`.
- [ ] **A-22 (LOW)** `wasm-bridge-dag.js:445` and `mock-lattice-samplers.js:555-557` gravity uses K_B² for both masses.
- [ ] **A-23 (LOW)** `decay-rates.js:43` ad-hoc `G_F_MEV = G_FERMI * 1e-6` — promote to constants.js.
- [ ] **A-24 (LOW)** `scales/scale1/pe-cloud-expander.js:51,82` uses `mass_MeV * 1000` heuristic — derive from K_B.
- [ ] **A-25 (LOW)** `bridge/scenarios/s0-seed-scenarios.js:607` direct gravity formula bypasses G_N and gravity toggle — clarify.

## TRACK B — GPU/CPU parity (engine-expert HIGH items)

- [d] **B-1 (HIGH)** CPU evaporation probabilistic, GPU deterministic threshold. *Deferred — needs WSL2 verification.*
- [d] **B-2 (HIGH)** CPU genesis applies latent-heat drain, GPU does not. *Deferred — needs WSL2 verification.*
- [d] **B-4 (HIGH)** CUDA genesis_kernel doesn't use SplitMix64 voxel hash. *Deferred — needs WSL2 verification.*
- [d] **B-5 (HIGH)** CUDA zero-curl spin defaults to +1, CPU randomizes. *Deferred — needs WSL2 verification.*
- [d] **B-6 (HIGH)** CUDA particle_id collision risk. *Deferred — needs WSL2 verification.*
- [d] **B-7 (HIGH)** CUDA phase_forces uses pre-2026-04-17 Newtonian velocity update. *Deferred — needs WSL2 verification.*

## TRACK C — MockBridge architectural gaps

- [ ] **C-arch-1 (HIGH)** MockBridge has no Gauss projection — toggle reads ON but constraint unenforced. Add minimal SOR or hard-wire OFF with banner.
- [ ] **C-arch-2 (HIGH)** MockBridge `_tickFlux` ignores `_dt` (kernel hardcodes dt=1). Either thread dt or remove `setDt`.
- [ ] **C-arch-3 (MEDIUM)** Dual-substrate fields hardcoded to 0 in MockBridge. Force flux-dual-substrate to WASM path OR implement L/R split.
- [ ] **C-arch-4 (MEDIUM)** `setDt` clamps min 1.0 silently. Log warning or remove clamp.
- [ ] **C-arch-5 (MEDIUM)** `state.useFluxMock` mode hides genesisIsosurface/dampingZones overlays for WASM scenarios in `field-overlays.js:311-321`.
- [ ] **C-arch-6 (MEDIUM)** Toggle order `setToggle('weak_transmutation', true)` BEFORE `setToggle('dual_substrate', true)` leaves intermediate invalid state. Add atomic batch.
- [ ] **C-arch-7 (MEDIUM)** `coulomb_charge_coupling` mismatch between C++ (1.0, Phase G) and JS pair force (α/4π classical) — factor of ~22 difference.
- [ ] **C-arch-8 (MEDIUM)** `s0-seed-beta-decay` direct toggle mutation in scenario file should be in OVERRIDES.
- [ ] **C-arch-9 (LOW-MED)** Sponge attenuation table allocated per tick — cache.
- [ ] **C-arch-10 (MEDIUM)** `getFluxSlice` allocates Float64Array per call — fine for 4Hz panel.
- [ ] **C-arch-11 (MEDIUM)** `_stateGrid.fill(0)` runs N³ per tick — optimize to dirty list.
- [ ] **C-arch-12 (LOW)** Damping factor `(1 - _params.damping)` not clamped to [0,1].
- [ ] **C-arch-13 (LOW)** `K_GENESIS_SQ` recomputed every tick — hoist.
- [ ] **C-arch-14 (LOW)** Inspector `inspectVoxel` returns hardcoded 0 for waveVel/divJ/curl on mock.
- [ ] **C-arch-15 (LOW)** `_dt ?? _params.dt ?? 1.0` chain — second branch dead code.

## TRACK D — WASM binding gaps (engine-expert)

- [ ] **D-1 (MEDIUM)** `EnergyLedger` populated in C++ but no `getEnergyLedger` WASM binding.
- [ ] **D-2 (MEDIUM)** `strong_energy`, `weak_energy` exported but never written in `compute_energy_audit` — silent zeros.
- [ ] **D-3 (MEDIUM)** `confinement` toggle not in `rb_toggle_map` — silently ignored by WASM (every quark/baryon scenario broken).
- [ ] **D-4 (LOW)** `getStrongForceField` uses hardcoded `ALPHA_S=1.0`, `TUBE_W=1.5`, regime cutoffs.
- [ ] **D-5 (LOW)** `bindings_render_bridge.cpp` missing pair_production, triad_binding, latency_field, emergent_forces, exact_dual_gauss in toggle map.
- [ ] **D-6 (LOW)** `f_exchange` field exported but never written by either CPU or GPU `exchange_force` path.

## TRACK E — Toggle wiring + validation

- [ ] **E-1 (HIGH)** `confinement` JS toggle silently ignored by WASM (D-3 above).
- [ ] **E-2 (MEDIUM)** `validate()` in `term_toggles.h` covers 8/23 toggles. Missing: `pair_production`→`genesis`, `selective_damping`→`damping`, `langevin`/`larmor_radiation` mutex, `coulomb_charge_coupling != 1` ⇔ `!emergent_forces`, `bcc_stencil != FULL` ⇒ wave on.

## TRACK F — Refactoring (mechanical, low-risk)

- [ ] **F-7-1 (HIGH)** Bare physics constants `0.5`, `1e-9`, `0.1` in `render_bridge.cpp:543, 545, 592` — name them.
- [ ] **F-13 (MEDIUM)** Bare salt ints (`/*salt=*/1`, 2, 3) in `render_bridge.cpp` — convert to enum.
- [ ] **F-19 (MEDIUM)** Bare `0.25` in `kernels_forces.cu:253-255` — use `GRAD_TIER2_SCALE`.
- [ ] **F-8 (HIGH)** CUDA index helpers (`idx3d`, `wrap`, `decode_xyz`, `periodic_delta`) defined twice in stencil.cu and forces.cu — extract to `cuda_index.cuh`.
- [ ] **F-laplace-weights (HIGH)** 18-pt Laplacian weights named differently across 5 implementations — add `LAPLACIAN_FACE_WEIGHT`, `LAPLACIAN_EDGE_WEIGHT` to constants headers.

## TRACK G — Test coverage gaps (test-orchestrator)

- [ ] **G-1** No C++ test for locked-particle pair-force semantics (commit dc329d6 fix).
- [ ] **G-2** No C++ test for E_L vs wv_L split (commit d0329f6 fix).
- [ ] **G-3** No JS Playwright spec for any of: locked-particle fix, sponge BC, fluxMock mirroring, Coulomb PE wiring.
- [ ] **G-4** Add stencil sum-rule unit test (W_FACE·6 + W_EDGE·12 - 4 = 0).
- [ ] **G-5** Add CPU/GPU index parity test (inject at (1,2,3) on both, assert match).
- [ ] **G-6** Add ½-convention regression test for energy audit.

## TRACK H — Constants completeness

- [ ] **H-1** Add `COULOMB_K_LATTICE`, `COULOMB_K_PE` to constants.js with documentation
- [ ] **H-2** Add 7 strong-force tuning constants to constants.js
- [ ] **H-3** Add `LATTICE_TO_SOLAR_MASS`, `G_HELIOCENTRIC`, `FTD_TICK_S` to constants.js
- [ ] **H-4** Promote `J_PER_EV`, `PLANCK_MASS_KG`, `PLANCK_FORCE_N`, `C_MS` from units.js to constants.js
- [ ] **H-5** Add `BOHR_LATTICE_TO_M` conversion factor
- [ ] **H-6** Add `G_FERMI_MEV` (currently inline-converted in decay-rates.js)
- [ ] **H-7** Add `CS_SUB_AMPLITUDE` for scale11 consciousness
- [ ] **H-8** Add `LAPLACIAN_FACE_WEIGHT`, `LAPLACIAN_EDGE_WEIGHT` to constants.h, constants_gpu.cuh, constants.js
- [ ] **H-9** Add `K_GENESIS_KINETIC_DRAIN`, `K_EVAP_RATE`, `K_GENESIS_FLUX_EPSILON` to constants.h
