# Engine File Manifest (auto-generated)

> Regenerate: `python engine/tools/build_file_manifest.py`  
> Machine-readable source of truth: [`ENGINE_FILE_MANIFEST.json`](ENGINE_FILE_MANIFEST.json)  
> Narrative map: [`ENGINE_CODE_MAP.md`](ENGINE_CODE_MAP.md)

**969 code files, 235,999 LOC** (tracked `.cpp/.cc/.h/.hpp/.cu/.cuh/.js/.mjs/.py` under `engine/`).

## Subsystem rollup

| Subsystem | Files | LOC |
|---|--:|--:|
| `tests` | 346 | 96,524 |
| `include` | 99 | 17,424 |
| `web/js-toplevel` | 40 | 14,673 |
| `web/scale0` | 57 | 13,928 |
| `web/tests` | 69 | 11,069 |
| `web/ui` | 83 | 10,839 |
| `src/core` | 40 | 10,818 |
| `web/bridge` | 31 | 9,960 |
| `web/viewport` | 13 | 8,045 |
| `cuda` | 17 | 6,951 |
| `tools` | 29 | 6,578 |
| `archive` | 12 | 5,597 |
| `src/scenarios` | 7 | 2,813 |
| `wasm` | 5 | 2,755 |
| `web/scale2` | 11 | 1,998 |
| `sim` | 16 | 1,951 |
| `web/scale1` | 10 | 1,844 |
| `web/atlas` | 9 | 1,386 |
| `src/phases` | 4 | 1,366 |
| `vendor` | 9 | 1,274 |
| `web/inspector` | 9 | 1,172 |
| `web/config` | 4 | 1,131 |
| `src/cli_demos` | 1 | 981 |
| `src/atom` | 3 | 847 |
| `src/cognition` | 1 | 835 |
| `web/backgrounds` | 7 | 781 |
| `web/scale4` | 5 | 454 |
| `web/scale5` | 5 | 403 |
| `web/core` | 6 | 348 |
| `web/scale3` | 6 | 314 |
| `web/other` | 6 | 268 |
| `other` | 1 | 261 |
| `web/scales-shared` | 1 | 197 |
| `web/telemetry` | 4 | 144 |
| `web/scale23` | 3 | 70 |

## Files by subsystem

### `tests`  (346 files, 96,524 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`test_gpu_physics.cpp`](../../engine/tests/test_gpu_physics.cpp) | 2618 | GPU Physics Test Suite — Testing Ontic Predictions at Scale Leverages the CUDA GpuEngine ( speedup) to run physics campaigns at lattice sizes and tick counts impractical on CPU. |
| [`campaign_dark_sector.cpp`](../../engine/tests/campaign_dark_sector.cpp) | 1762 | Campaign: Dark Sector (consolidated) Wave 4c.11 consolidation, 7->1 dark sector merge. |
| [`test_gpu_experiments.cpp`](../../engine/tests/test_gpu_experiments.cpp) | 1679 | GPU Particle Physics Experiment Suite Simulations of real scientific experiments on the FTD GPU engine, using ALL available telemetry (EnergyAudit, sync_to_host, phi_coulomb) with quantitative pred... |
| [`test_constructors.cpp`](../../engine/tests/test_constructors.cpp) | 1354 | test_constructors — unit tests for ftd::ctor::* Spec: docs/superpowers/specs/2026-04-15-ftd-constructors-design.md |
| [`campaign_hydrogen_spectrum.cpp`](../../engine/tests/campaign_hydrogen_spectrum.cpp) | 1281 | Campaign: Hydrogen Spectrum (consolidated suite) Merges 5 legacy hydrogen test files into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_hydrogen_scale1 -> sect... |
| [`campaign_graviton_tt_correlator.cpp`](../../engine/tests/campaign_graviton_tt_correlator.cpp) | 1220 | @file campaign_graviton_tt_correlator.cpp @brief Frontier 4, Step 4a-ii — emergent transverse-traceless (spin-2) pole. |
| [`benchmark_engine_theory.cpp`](../../engine/tests/benchmark_engine_theory.cpp) | 1145 | ENGINE-THEORY BRIDGE BENCHMARK — COMPREHENSIVE Quantitative comparison of C++ engine output to FTD theory. |
| [`benchmark_black_hole_thermo.cpp`](../../engine/tests/benchmark_black_hole_thermo.cpp) | 1132 | BLACK HOLE THERMODYNAMICS BENCHMARKS Tests FTD lattice predictions for black hole thermodynamics using the Scale 0 RenderBridge engine. |
| [`test_pe_forces.cpp`](../../engine/tests/test_pe_forces.cpp) | 1080 | Test: ParticleEngine force variants (consolidated suite) Merges 7 legacy test_pe_*.cpp files into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_pe_exchange ->... |
| [`test_atom_engine_forces.cpp`](../../engine/tests/test_atom_engine_forces.cpp) | 1005 | Test: AtomEngine force variants (consolidated suite) Merges 5 legacy test_ae_*.cpp files into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_ae_angle_strain ->... |
| [`test_tritium_algebra.cpp`](../../engine/tests/test_tritium_algebra.cpp) | 847 | Test: Tritium algebra (consolidated suite) Merges 7 legacy header-only test files into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_trit_packing -> section "p... |
| [`campaign_coulomb_force_law.cpp`](../../engine/tests/campaign_coulomb_force_law.cpp) | 837 | Campaign: Coulomb force law (consolidated suite) Merges 5 legacy test/campaign_*.cpp files into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_poisson_coulomb -... |
| [`campaign_thomson_moving_recoil_accounting.cpp`](../../engine/tests/campaign_thomson_moving_recoil_accounting.cpp) | 821 | FTD-0297: Thomson moving-recoil source/work accounting. |
| [`test_lorentz.cpp`](../../engine/tests/test_lorentz.cpp) | 789 | Test: Lorentz + Magnetic family (consolidated suite) Merges 5 legacy tests into test_lorentz.cpp (self-ref target) using the Phase 2a ftd::test NDJSON telemetry API: test_lorentz -> section "lorent... |
| [`test_logic_engine.cpp`](../../engine/tests/test_logic_engine.cpp) | 785 | Test: Logic-First Engine — Comprehensive Verification 40 checks verifying that the 6-rule logic-first engine behaves correctly. |
| [`test_maxwell.cpp`](../../engine/tests/test_maxwell.cpp) | 753 | Test: Maxwell Equation Recovery Verifies that the FTD wave equation + Gauss constraint recovers Maxwell's equations by reconstructing E and B fields and checking their relationships. |
| [`campaign_free_dynamics.cpp`](../../engine/tests/campaign_free_dynamics.cpp) | 680 | Campaign: Free Particle Dynamics — Hierarchical Exploration Probes the engine's behavior with FREE (unlocked) particles, building from simplest to most complex: FD1: Single free particle — inertia... |
| [`test_einstein_equations.cpp`](../../engine/tests/test_einstein_equations.cpp) | 655 | Test: Einstein Equations on the FTD Lattice Validates the gravitational sector: Poisson potential, 1/r profile, G_N extraction, and proper time dilation. |
| [`test_gpu_parity_complete.cpp`](../../engine/tests/test_gpu_parity_complete.cpp) | 655 | GPU Parity Complete: Every physics domain tested on GPU vs CPU. |
| [`campaign_thomson_native_continuity.cpp`](../../engine/tests/campaign_thomson_native_continuity.cpp) | 646 | FTD-0291: Thomson native finite-volume continuity meter. |
| [`campaign_dispersion.cpp`](../../engine/tests/campaign_dispersion.cpp) | 643 | Campaign: Dispersion Relation (consolidated suite) Merges 3 legacy dispersion test/campaign files into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_dispersion... |
| [`test_emergent_ic1_topology.cpp`](../../engine/tests/test_emergent_ic1_topology.cpp) | 631 | @file test_emergent_ic1_topology.cpp @brief Regression + topology verification for the canonical ic1 cluster. |
| [`campaign_thomson_radiation_shells.cpp`](../../engine/tests/campaign_thomson_radiation_shells.cpp) | 571 | FTD-0290: Thomson radiation shell meter. |
| [`test_energy_conservation.cpp`](../../engine/tests/test_energy_conservation.cpp) | 545 | Test: Energy Conservation (consolidated suite) Merges 3 legacy tests into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_energy -> section "energy_basic" (4 sub... |
| [`test_bell_aggregate.cpp`](../../engine/tests/test_bell_aggregate.cpp) | 525 | Test: Bell Aggregate — Ensemble S = 2sqrt(2) Verifies the three-level observer Bell hierarchy from FTD: Level 1 (Substrate): S <= 2 (local deterministic, triangular correlation) Level 2 (Complex):... |
| [`campaign_thomson_flux_excess.cpp`](../../engine/tests/campaign_thomson_flux_excess.cpp) | 516 | FTD-0289: Thomson flux-excess discriminator. |
| [`benchmark_alpha_relaxation_lean_gpu.cu`](../../engine/tests/benchmark_alpha_relaxation_lean_gpu.cu) | 513 | @file benchmark_alpha_relaxation_lean_gpu.cu @brief Lean dynamical Coulomb benchmark using GPU Poisson relaxation. |
| [`campaign_thomson_unlocked_recoil.cpp`](../../engine/tests/campaign_thomson_unlocked_recoil.cpp) | 512 | FTD-0288: Thomson unlocked recoil campaign. |
| [`test_light.cpp`](../../engine/tests/test_light.cpp) | 510 | Test: Light & Photon Properties — 8 Checks Verifies that the engine's wave equation naturally produces massless, frequency-bearing, linearly-propagating EM waves — i.e., LIGHT — without any explici... |
| [`benchmark_emergent_alpha.cpp`](../../engine/tests/benchmark_emergent_alpha.cpp) | 504 | EMERGENT PHYSICS BENCHMARK — Reverse-Engineering Alpha Can the fine structure constant be MEASURED from lattice field dynamics rather than read from a hardcoded constant? |
| [`campaign_wave_dynamics.cpp`](../../engine/tests/campaign_wave_dynamics.cpp) | 504 | Campaign: Wave dynamics (consolidated suite) Merges 4 legacy tests into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_wave_speed -> section "wave_speed" test_i... |
| [`campaign_thomson_coupled_source_work.cpp`](../../engine/tests/campaign_thomson_coupled_source_work.cpp) | 501 | FTD-0296: Fixed-charge coupled tick source/work continuity. |
| [`test_native_moore_shell_gauss.cpp`](../../engine/tests/test_native_moore_shell_gauss.cpp) | 489 | Native Moore-shell Gauss audit. |
| [`test_triad_confinement.cpp`](../../engine/tests/test_triad_confinement.cpp) | 482 | Test: Triad Binding from Confinement (Checklist #38) Verifies that three same-sign particles with different color orientations form a bound state via the color confinement force, rather than relyin... |
| [`campaign_quantum_correlations.cpp`](../../engine/tests/campaign_quantum_correlations.cpp) | 477 | Campaign: Quantum correlations (consolidated suite) Merges 3 legacy tests into a single ftd::test-instrumented suite: test_entanglement -> section "entanglement" (16 checks) campaign_epr_correlatio... |
| [`test_atomic_energy.cpp`](../../engine/tests/test_atomic_energy.cpp) | 454 | Physics Checklist #69: Atomic Energy Levels from Scale 0 Lattice Full lattice-scale hydrogen is computationally prohibitive: a_0 ~ 613 lattice units (with gravity) or ~3374 (pure EM) -> need L > 20... |
| [`campaign_thomson_tick_local_continuity_v2.cpp`](../../engine/tests/campaign_thomson_tick_local_continuity_v2.cpp) | 453 | FTD-0295: Source-free discrete tick local continuity v2. |
| [`test_baryogenesis.cpp`](../../engine/tests/test_baryogenesis.cpp) | 446 | Test: Baryogenesis (#51 Physics Checklist) FTD derives the baryon-to-photon ratio eta ~ 10^-10 from CP violation + Sakharov conditions. |
| [`test_cluster_inertia.cpp`](../../engine/tests/test_cluster_inertia.cpp) | 446 | ============================================================================ test_cluster_inertia.cpp (unified-mass Phase 2, 2026-06-06) ------------------------------------------------------------... |
| [`campaign_thomson_tick_local_continuity.cpp`](../../engine/tests/campaign_thomson_tick_local_continuity.cpp) | 435 | FTD-0294: Source-free discrete tick local continuity. |
| [`test_latency_field.cpp`](../../engine/tests/test_latency_field.cpp) | 433 | Test: Latency Field (Gravitational Potential) Validates the Poisson-based latency field implementation: ∇²φ_L = 4πG·ρ_mass → L = √(clamp(φ_L, 0, 0.998)) Sections: LAT-1: Single particle → latency >... |
| [`test_lagrangian.cpp`](../../engine/tests/test_lagrangian.cpp) | 429 | Test: Lagrangian 2.0 — Zero Free Parameters via Master Quadratic Verifies that the engine dynamics are fully determined by G* and the master quadratic. |
| [`benchmark_alpha_window_lean_gpu.cu`](../../engine/tests/benchmark_alpha_window_lean_gpu.cu) | 428 | @file benchmark_alpha_window_lean_gpu.cu @brief Lean fixed-window Coulomb geometry benchmark. |
| [`test_tick_phase_order.cpp`](../../engine/tests/test_tick_phase_order.cpp) | 426 | ============================================================================ test_tick_phase_order.cpp (ticket W4-2) ---------------------------------------------------------------------------- Reg... |
| [`test_higgs_mechanism.cpp`](../../engine/tests/test_higgs_mechanism.cpp) | 424 | Test: Higgs Mechanism from Manifestation Physics Checklist Item #37: The Higgs mechanism in FTD is understood as spontaneous symmetry breaking (SSB) via manifestation dynamics. |
| [`test_asymptotic_freedom.cpp`](../../engine/tests/test_asymptotic_freedom.cpp) | 415 | Test: Asymptotic Freedom (Checklist #34) Verifies that the QCD running coupling alpha_s(Q) exhibits asymptotic freedom both analytically (via the alpha_s_running / alpha_s_lattice formulas) and on... |
| [`campaign_sm_observables.cpp`](../../engine/tests/campaign_sm_observables.cpp) | 414 | Campaign: Standard Model Observables from Lattice Dynamics Computes 5 key SM observables directly from the FTD engine dynamics (not from plugged-in formulas). |
| [`test_multiscale_bridge.cpp`](../../engine/tests/test_multiscale_bridge.cpp) | 414 | Test: Multi-Scale Bridge (13 unit checks) Covers quantum number preservation, position round-trips, OnticEntity consistency, multi-nuclei clustering, edge cases, and energy budget across Scale 0 ↔... |
| [`benchmark_alpha_convergence.cpp`](../../engine/tests/benchmark_alpha_convergence.cpp) | 413 | @file benchmark_alpha_convergence.cpp @brief α_eff continuum-limit convergence study at high SOR precision. |
| [`test_conservation_profile.cpp`](../../engine/tests/test_conservation_profile.cpp) | 412 | ============================================================================ test_conservation_profile.cpp (engine-flawless audit, 2026-06-01) ------------------------------------------------------... |
| [`test_native_dual_half_shell.cpp`](../../engine/tests/test_native_dual_half_shell.cpp) | 407 | Native dual half-shell audit. |
| [`campaign_shell_predictions.cpp`](../../engine/tests/campaign_shell_predictions.cpp) | 403 | Campaign: Self-Field Shell Predictions (High-Precision) Tests two structural predictions about the electron's self-field: Prediction 1: E_field / K_B^2 = 16 * alpha Prediction 2: r_eff / r_shell =... |
| [`test_bivector_closure.cpp`](../../engine/tests/test_bivector_closure.cpp) | 403 | @file test_bivector_closure.cpp @brief Program F-double-prime — closure tests for the plaquette bivector algebra detected in Program F-prime (FTD-0086). |
| [`campaign_bcc_band_spectrum.cpp`](../../engine/tests/campaign_bcc_band_spectrum.cpp) | 402 | @file campaign_bcc_band_spectrum.cpp @brief BCC sub-stencil two-state spectrum campaign — smoke test. |
| [`test_engine_lifecycle.cpp`](../../engine/tests/test_engine_lifecycle.cpp) | 390 | ============================================================================ test_engine_lifecycle.cpp — ScaleEngine RAII / lifecycle contract (ticket W5) ------------------------------------------... |
| [`test_flavor_physics.cpp`](../../engine/tests/test_flavor_physics.cpp) | 382 | Test: Flavor Physics — CKM/PMNS from Lattice (Checklist #40) FTD derives PMNS neutrino mixing angles and CKM parameters from framework integers {3, 4, 7, 13}. |
| [`campaign_beta_measurement.cpp`](../../engine/tests/campaign_beta_measurement.cpp) | 379 | @file campaign_beta_measurement.cpp @brief β-function measurement at non-zero temperature — smoke test. |
| [`test_vortex.cpp`](../../engine/tests/test_vortex.cpp) | 377 | Test: Vortex Formation — Biot-Savart Feedback Loop Explores whether the Biot-Savart coupling (curl of charge current) creates stable vortex structures when an electron has initial tangential veloci... |
| [`test_clifford_multigrade.cpp`](../../engine/tests/test_clifford_multigrade.cpp) | 375 | @file test_clifford_multigrade.cpp @brief Path 1 — Wilson-loop-style multi-grade decomposition. |
| [`test_larmor.cpp`](../../engine/tests/test_larmor.cpp) | 374 | Test: Larmor Radiation (Acceleration-Dependent Damping) When the larmor_radiation toggle is ON, damping at manifested sites is modulated by the particle's acceleration: larmor_mod = min(1, LARMOR_F... |
| [`test_wz_mass.cpp`](../../engine/tests/test_wz_mass.cpp) | 369 | Test: W/Z Mass Generation from Chirality Gap in Dual Substrate Physics Checklist Item #36 In dual-substrate mode, flux splits into left-handed (J_L) and right-handed (J_R) components. |
| [`test_link_bilinear_clifford.cpp`](../../engine/tests/test_link_bilinear_clifford.cpp) | 363 | @file test_link_bilinear_clifford.cpp @brief Program F — link-bilinear fermion probe. |
| [`test_plaquette_bivector_clifford.cpp`](../../engine/tests/test_plaquette_bivector_clifford.cpp) | 358 | @file test_plaquette_bivector_clifford.cpp @brief Program F-prime — plaquette bivector probe. |
| [`test_atom_engine.cpp`](../../engine/tests/test_atom_engine.cpp) | 354 | Test: AtomEngine (Scale 2) unit tests Checks covering injection, properties, forces, bonding, conservation laws, and integration. |
| [`test_gauss.cpp`](../../engine/tests/test_gauss.cpp) | 346 | Test: Gauss constraint (consolidated suite) Merges 2 legacy tests into test_gauss.cpp (self-ref target): test_gauss -> section "gauss_structure" (16 checks) test_gauss_convergence -> section "gauss... |
| [`campaign_parity_violation.cpp`](../../engine/tests/campaign_parity_violation.cpp) | 340 | Campaign: Parity Violation (Phase 6 — Weak Sector & SU(2)) Tests that weak transmutation in dual-substrate mode preferentially affects +1 (left-chiral) particles over -1 (right-chiral) particles. |
| [`test_em_energy_conservation.cpp`](../../engine/tests/test_em_energy_conservation.cpp) | 340 | Test: EM Energy Conservation in Undamped Vacuum Verifies that total electromagnetic energy is conserved when there are no particles, no coupling, and no damping — pure wave equation dynamics. |
| [`campaign_neutrino_sector.cpp`](../../engine/tests/campaign_neutrino_sector.cpp) | 339 | Campaign: Neutrino Sector Verification (Phase 8 — Particle Zoo) Verifies the complete neutrino sector derived from framework integers {3, 4, 7, 13}. |
| [`campaign_thermal_ignition.cpp`](../../engine/tests/campaign_thermal_ignition.cpp) | 339 | @file campaign_thermal_ignition.cpp @brief FTD-0274 scout — min/max temperature + ignition/detonation map of the lattice. |
| [`test_measurement.cpp`](../../engine/tests/test_measurement.cpp) | 335 | Test: Measurement = Manifestation Validation Validates that observer coupling (manifested structure s != 0) triggers wave function localization (collapse = manifestation): MEAS-1: Without observer... |
| [`test_particle_lifetime.cpp`](../../engine/tests/test_particle_lifetime.cpp) | 334 | Diagnostic: Particle Lifetime & Energy Loss Probes three issues: PL1: Slow particle survival (v=0.01, 0.02 — were evaporating) PL2: Energy loss rate vs velocity (quantify radiation) PL3: Orbital en... |
| [`campaign_thomson_tick_invariant_v2.cpp`](../../engine/tests/campaign_thomson_tick_invariant_v2.cpp) | 333 | FTD-0293: Source-free discrete tick energy invariant, precision v2. |
| [`campaign_thomson_recoil_observatory.cpp`](../../engine/tests/campaign_thomson_recoil_observatory.cpp) | 332 | FTD-0287: Thomson recoil observatory. |
| [`campaign_halo_forcedness.cpp`](../../engine/tests/campaign_halo_forcedness.cpp) | 331 | @file campaign_halo_forcedness.cpp @brief FTD-0300: is the single-particle self-field HALO EXPONENT forced by the dynamics, or tuned by engine calibration constants? |
| [`test_stress_energy.cpp`](../../engine/tests/test_stress_energy.cpp) | 331 | Test: Stress-Energy Tensor T_mu_nu From Noether's theorem applied to the FTD wave equation, the stress-energy tensor components are: T^00 = (1/2)\|wave_vel\|^2 + (1/2)*C^2*sum_neighbors\|J_n - J_c\|^2/... |
| [`test_native_engine_transport_flow.cpp`](../../engine/tests/test_native_engine_transport_flow.cpp) | 330 | Native engine transport-history flow audit. |
| [`test_branch_holonomy_gap.cpp`](../../engine/tests/test_branch_holonomy_gap.cpp) | 327 | test_branch_holonomy_gap.cpp — verifies the Z_2 torus branch-twist gap λ_min = 4 sin²( π / (2N) ) (eq. |
| [`test_born_rule_ensemble.cpp`](../../engine/tests/test_born_rule_ensemble.cpp) | 325 | Test: Born Rule Ensemble — Multi-site \|psi\|^2 Distribution Validation Enhances Born rule testing beyond single-site genesis to validate the full multi-site probability distribution: BORN-1: Gaussia... |
| [`benchmark_dirac_electron_in_B.cpp`](../../engine/tests/benchmark_dirac_electron_in_B.cpp) | 322 | Single-electron stable orbit in uniform B (Phase II.3 milestone). |
| [`benchmark_dynamical_sm.cpp`](../../engine/tests/benchmark_dynamical_sm.cpp) | 319 | @file benchmark_dynamical_sm.cpp @brief EFT Phase 4 — dynamical SM emergence tests. |
| [`campaign_multiscale_pipeline.cpp`](../../engine/tests/campaign_multiscale_pipeline.cpp) | 317 | Campaign: Multi-Scale Pipeline (12 checks across 4 phases) Phase 1: Full pipeline round-trip (Scale 0 → 1 → 2 → 1 → 0) Phase 2: Energy conservation across transitions Phase 3: Multi-atom pipeline (... |
| [`test_continuity.cpp`](../../engine/tests/test_continuity.cpp) | 315 | Test: Charge Continuity Equation Verifies that total electric charge Q = sum(state) is exactly conserved through all dynamics: static, dynamic, annihilation, genesis. |
| [`test_telemetry.cpp`](../../engine/tests/support/test_telemetry.cpp) | 312 | ============================================================================ tests/support/test_telemetry.cpp ---------------------------------------------------------------------------- Implementa... |
| [`test_native_dual_cell_gauss.cpp`](../../engine/tests/test_native_dual_cell_gauss.cpp) | 306 | Native dual-cell Gauss audit. |
| [`test_soliton_sweeps.cpp`](../../engine/tests/test_soliton_sweeps.cpp) | 305 | Test: Soliton Emergence and Sweeps Campaign (Class B Track 2) Performs automated sweeps over amplitudes, seeds, and toggle configs using the triplet metric (n_total, centroid_drift, rms_radius) to... |
| [`test_gpu_shell_battery.cpp`](../../engine/tests/test_gpu_shell_battery.cpp) | 302 | GPU Shell Battery — Understanding Self-Field Dynamics Runs multiple configurations at 128^3 to understand how the electron's self-field depends on: 1. |
| [`campaign_einstein.cpp`](../../engine/tests/campaign_einstein.cpp) | 300 | Campaign: Einstein — Relativistic and Gravitational Tests Three tests probing energy conservation, Lorentz contraction, and gravitational redshift in the FTD engine: E1: Energy Conservation — 3-par... |
| [`campaign_wave_sectors.cpp`](../../engine/tests/campaign_wave_sectors.cpp) | 300 | Campaign: Wave Sectors (FTD-0299) [hardened v2 after adversarial pre-reg review] Arm 1 (--arm=light): light-sector dispersion atlas. |
| [`test_closed_negatives.cpp`](../../engine/tests/test_closed_negatives.cpp) | 299 | @file test_closed_negatives.cpp @brief Regression guards for closed-negative ledger claims. |
| [`test_gpu_parity.cpp`](../../engine/tests/test_gpu_parity.cpp) | 299 | GPU vs CPU parity tests for the FTD CUDA engine. |
| [`test_poynting.cpp`](../../engine/tests/test_poynting.cpp) | 298 | Test: Poynting Vector S = E x B Verifies the Poynting vector diagnostic API, which gives the direction and magnitude of electromagnetic energy flow. |
| [`test_thermodynamics.cpp`](../../engine/tests/test_thermodynamics.cpp) | 296 | Test: Thermodynamics Verifies thermodynamic properties of the FTD lattice: 1. |
| [`test_flux_mediated.cpp`](../../engine/tests/test_flux_mediated.cpp) | 295 | Test: Flux-Mediated Force — 1/r² from Field Dynamics Verifies that the coupling term in the Lagrangian: L_coupling = -g_c * s * (div J) produces a self-consistent flux field around charged particle... |
| [`campaign_thomson_tick_invariant.cpp`](../../engine/tests/campaign_thomson_tick_invariant.cpp) | 294 | FTD-0292: Source-free discrete tick energy invariant. |
| [`test_render_bridge_golden.cpp`](../../engine/tests/test_render_bridge_golden.cpp) | 293 | ============================================================================ test_render_bridge_golden.cpp ---------------------------------------------------------------------------- Phase 4 PRE-F... |
| [`test_audit_regression.cpp`](../../engine/tests/test_audit_regression.cpp) | 292 | ============================================================================ test_audit_regression.cpp ---------------------------------------------------------------------------- Focused regressio... |
| [`test_spin_statistics.cpp`](../../engine/tests/test_spin_statistics.cpp) | 292 | Test: Spin-Statistics (720 degree periodicity) Verifies that framed flux exhibits spinor behavior: SPIN-1: 360 degree rotation inverts framed flux sign SPIN-2: 720 degree rotation returns to origin... |
| [`campaign_gravity_profile.cpp`](../../engine/tests/campaign_gravity_profile.cpp) | 290 | Campaign: Gravitational Density Profile (Phase 7 — Gravitational Sector) Tests the radial density profile around a static massive object. |
| [`test_dual_substrate.cpp`](../../engine/tests/test_dual_substrate.cpp) | 290 | Test: Dual-Substrate Engine Verifies the dual-substrate implementation from "The Algebraic Identity of Two Substrates" (Montanez & Claude, 2026). |
| [`test_falsifiability.cpp`](../../engine/tests/test_falsifiability.cpp) | 288 | Falsifiability Tests: Negative-Result Validation PURPOSE: Demonstrate that FTD is CONSTRAINED, not arbitrary. |
| [`campaign_cluster_energy_spectroscopy.cpp`](../../engine/tests/campaign_cluster_energy_spectroscopy.cpp) | 287 | @file campaign_cluster_energy_spectroscopy.cpp @brief FTD-0273 Phase 1 — mass as flux-energy in flip-quanta. |
| [`campaign_flux_slice_propagation.cpp`](../../engine/tests/campaign_flux_slice_propagation.cpp) | 286 | @file campaign_flux_slice_propagation.cpp @brief 2D flux-slice diagnostic for wave-propagation isotropy. |
| [`test_gpu_continuity_ledger.cpp`](../../engine/tests/test_gpu_continuity_ledger.cpp) | 286 | GPU-native continuity ledger parity. |
| [`test_ladder_walk_from_oh.cpp`](../../engine/tests/test_ladder_walk_from_oh.cpp) | 285 | @file test_ladder_walk_from_oh.cpp @brief Program A (partial closure) — derive ladder-walk step-size multiset from O_h structure. |
| [`benchmark_budget_equation.cpp`](../../engine/tests/benchmark_budget_equation.cpp) | 283 | BUDGET EQUATION EXPERIMENT Tests: x/K + G_star/x = 1 The budget equation says the coupling x partitions between two phases: Coulomb (deconfined): fraction = x/K ~ 0.978 Confined: fraction = G_star/... |
| [`test_wilson_dirac_gauge.cpp`](../../engine/tests/test_wilson_dirac_gauge.cpp) | 282 | Wilson-Dirac gauge-link verification (Phase II.2-C milestone). |
| [`test_inflation.cpp`](../../engine/tests/test_inflation.cpp) | 281 | Test: Inflation (Sub-Threshold Flux Dynamics) Verifies that high-density uniform flux undergoes dynamics consistent with inflationary cosmology: exponential energy growth, approximately scale-invar... |
| [`test_generation_graph.cpp`](../../engine/tests/test_generation_graph.cpp) | 278 | test_generation_graph.cpp — Γ_F(d) [CANDIDATE RECONSTRUCTION] diagnostic. |
| [`campaign_annihilation_angular.cpp`](../../engine/tests/campaign_annihilation_angular.cpp) | 274 | Campaign: e+e- Annihilation Angular Distribution (QED Scattering) — GPU Validates that the FTD lattice produces the expected angular distribution for e+e- -> gamma gamma annihilation radiation. |
| [`test_wavepacket.cpp`](../../engine/tests/test_wavepacket.cpp) | 274 | Test: Wavepacket Injection (Phase 6, Stage 2) Verifies that Gaussian wavepacket initialization: - Produces correct total energy - Conserves energy under evolution - Reaches the same steady state as... |
| [`test_ftd0110_cluster_geometry.cpp`](../../engine/tests/test_ftd0110_cluster_geometry.cpp) | 273 | Test: FTD-0110 cluster GEOMETRY diagnostic. |
| [`test_nonlinear_flow_multiscale.cpp`](../../engine/tests/test_nonlinear_flow_multiscale.cpp) | 270 | @file test_nonlinear_flow_multiscale.cpp @brief P2.1 + P2.2 + P2.3: native response tuple at b ∈ {1, 2, 4, 8} under mixed-toggle nonlinear dynamics, with ensemble uncertainties. |
| [`test_particle_engine.cpp`](../../engine/tests/test_particle_engine.cpp) | 269 | Phase 7 — Stage 2: ParticleEngine unit tests (12 checks) PE1: Particle injection (id assigned, charge correct) PE2: Free particle (constant velocity when alone, no damping) PE3: Opposite attract (f... |
| [`test_color_binding_and_structure.cpp`](../../engine/tests/test_color_binding_and_structure.cpp) | 267 | @file test_color_binding_and_structure.cpp @brief Phase-4i: Combined tests for (1) RGB triad binding and (2) FTD "color" transformation structure vs SU(3). |
| [`campaign_integer_sweep.cpp`](../../engine/tests/campaign_integer_sweep.cpp) | 266 | Campaign: Integer Uniqueness Sweep THE critical test for scientific credibility. |
| [`test_master_quadratic_uniqueness.cpp`](../../engine/tests/test_master_quadratic_uniqueness.cpp) | 266 | @file test_master_quadratic_uniqueness.cpp @brief Program E — Uniqueness of the master quadratic as minimal polynomial. |
| [`campaign_plato.cpp`](../../engine/tests/campaign_plato.cpp) | 264 | Campaign: Plato — Dispositional Field Tests Three tests probing the dispositional (flux) layer of FTD: P1: Dispositional Ratio — Verify 1/r^2 Coulomb falloff of \|J(r)\|^2 P2: Genesis Phase Transitio... |
| [`campaign_wigner.cpp`](../../engine/tests/campaign_wigner.cpp) | 263 | Campaign: Wigner Tests — Octahedral Symmetry, Parity, CPT Invariance W1: Octahedral Symmetry L=48, single +1 at center, 200 ticks. |
| [`test_action_stationarity.cpp`](../../engine/tests/test_action_stationarity.cpp) | 263 | Test: Discrete Action Stationarity Verifies that the FTD tick cycle IS the Euler-Lagrange equation of the complete discrete Lagrangian S = Sigma_v L(v). |
| [`benchmark_g_n_mass_spectrum.cpp`](../../engine/tests/benchmark_g_n_mass_spectrum.cpp) | 262 | Benchmark: G_N(M, L) mass-spectrum scan (Arc D gap (ii) scaffold) Purpose: Verify that the engine's solve_latency_poisson_cpu (poisson_solvers.cpp:190-228) correctly reproduces a constant engine-in... |
| [`campaign_cluster_fission_fusion.cpp`](../../engine/tests/campaign_cluster_fission_fusion.cpp) | 262 | campaign_cluster_fission_fusion — Exp-A of the cluster-thermodynamics EXPLORATORY pass (P2 fission/fusion asymmetry + P3 fusion-is-lossy). |
| [`benchmark_field_soa_cpu.cpp`](../../engine/tests/benchmark_field_soa_cpu.cpp) | 261 | @file benchmark_field_soa_cpu.cpp @brief Non-physics timing probe for CPU FieldSoA read paths. |
| [`campaign_proton_stability.cpp`](../../engine/tests/campaign_proton_stability.cpp) | 261 | @file campaign_proton_stability.cpp @brief FTD-0301: is the proton (uud triad) dynamically stable, or does it DECAY (evaporate / transmute) under FTD's native dynamics? |
| [`benchmark_cognitive_lattice.cpp`](../../engine/tests/benchmark_cognitive_lattice.cpp) | 260 | @file benchmark_cognitive_lattice.cpp @brief Non-physics timing probe for the ternary cognitive sidecar. |
| [`test_native_source_core_fork.cpp`](../../engine/tests/test_native_source_core_fork.cpp) | 260 | Native source-core fork audit. |
| [`test_wh_clifford_alt_routes.cpp`](../../engine/tests/test_wh_clifford_alt_routes.cpp) | 257 | @file test_wh_clifford_alt_routes.cpp @brief Phase-4 fermion-emergence alt-route measurements (FTD-0061 extension). |
| [`test_thomson_scattering.cpp`](../../engine/tests/test_thomson_scattering.cpp) | 251 | Test: Thomson Scattering — 6 Checks Verifies that an EM wave encountering a charged particle causes the charge to oscillate and re-radiate (scatter). |
| [`test_confinement.cpp`](../../engine/tests/test_confinement.cpp) | 249 | Test: Strong Force Confinement Dynamics Verifies flux-tube based confinement model: CONF-1: Two color charges separated by r feel constant force at large r CONF-2: Force weakens at short range (asy... |
| [`campaign_novel_predictions.cpp`](../../engine/tests/campaign_novel_predictions.cpp) | 248 | Campaign: Novel Predictions & Falsifiability (Phase 10) Tests the sharpest predictions of FTD that can be falsified by experiment. |
| [`campaign_thermostat_off_sweep.cpp`](../../engine/tests/campaign_thermostat_off_sweep.cpp) | 248 | @file campaign_thermostat_off_sweep.cpp @brief FTD-0260 discriminator: is the FTD-0110 k(A) drift thermostat physics? |
| [`test_entanglement_basis.cpp`](../../engine/tests/test_entanglement_basis.cpp) | 248 | Test: Entanglement Basis Dependence — Measurement-Basis Correlations Tests the local hidden-variable model of entangled pairs, verifying that correlations depend on the measurement basis angle and... |
| [`campaign_time_dilation.cpp`](../../engine/tests/campaign_time_dilation.cpp) | 246 | @file campaign_time_dilation.cpp @brief CAMPAIGN 2 — Dynamical time dilation: does a moving lattice clock dilate as √(1−v²) [L²/γ] or 1−v [L¹/FTD-0208]? |
| [`test_emergent_measurements.cpp`](../../engine/tests/test_emergent_measurements.cpp) | 243 | Emergent Measurements — High-Fidelity Lattice Physics Verification Five quantitative tests using high SOR resolution to measure emergent quantities that arise from the six FTD update rules: EM1: Co... |
| [`test_eft_blocking.cpp`](../../engine/tests/test_eft_blocking.cpp) | 242 | @file test_eft_blocking.cpp @brief EFT Phase 2A — block-spin transformation validation gate. |
| [`test_selffield_profile.cpp`](../../engine/tests/test_selffield_profile.cpp) | 241 | Test: Self-Field Profile Investigation (Phase 6, Stage 1) Characterizes the steady-state flux envelope around a single locked point-particle. |
| [`test_spacetime_forcing_demo.cpp`](../../engine/tests/test_spacetime_forcing_demo.cpp) | 241 | @file test_spacetime_forcing_demo.cpp @brief DEMONSTRATION for FTD-0253 (FOUND_SPACETIME_FORCING_BOUNDARY): the causal cone is forced by locality; the Lorentzian *metric* is not — it rides on the d... |
| [`benchmark_rutherford_alpha.cpp`](../../engine/tests/benchmark_rutherford_alpha.cpp) | 236 | @file benchmark_rutherford_alpha.cpp @brief Thread 4 of the EFT Day-2 program — Rutherford scattering α extraction. |
| [`test_benchmark.cpp`](../../engine/tests/test_benchmark.cpp) | 236 | Performance benchmark for the FTD engine. |
| [`test_dipole_radiation.cpp`](../../engine/tests/test_dipole_radiation.cpp) | 236 | Test: Dipole Radiation Pattern — 6 Checks Verifies that a z-polarized current burst produces the classical sin²θ angular radiation pattern. |
| [`campaign_genesis_trajectory.cpp`](../../engine/tests/campaign_genesis_trajectory.cpp) | 235 | @file campaign_genesis_trajectory.cpp @brief FTD-0267: genesis-vs-survival per-tick trajectory in the canonical engine. |
| [`campaign_pe_fine_structure.cpp`](../../engine/tests/campaign_pe_fine_structure.cpp) | 235 | Campaign: PE Fine Structure Tests multiple Phase 2 forces working together: spin-orbit + relativistic corrections produce fine structure splitting. |
| [`campaign_lorentz_measure.cpp`](../../engine/tests/campaign_lorentz_measure.cpp) | 234 | Campaign: Lorentz Invariance Quantitative Measurement Runs wave packets along all 13 distinct lattice directions on the cubic lattice and measures the effective wave speed in each direction. |
| [`test_helium_scale1.cpp`](../../engine/tests/test_helium_scale1.cpp) | 233 | Helium at Scale 1: Multi-Electron Atoms He⁺ (Z=2, 1 electron): Bohr model predicts a₀/2 radius, 4× binding He (Z=2, 2 electrons): exchange force creates e⁻-e⁻ repulsion HE-1: He⁺ electron survives... |
| [`test_substrate_angle_probe.cpp`](../../engine/tests/test_substrate_angle_probe.cpp) | 233 | @file test_substrate_angle_probe.cpp @brief Stage-1 exploratory probe: which substrate phase (if any) carries a *native dynamical angle* under the bare wave + Gauss dynamics? |
| [`campaign_cross_scale.cpp`](../../engine/tests/campaign_cross_scale.cpp) | 232 | Phase 7 — Stage 4: Cross-Scale Validation (6 checks) Run the SAME two-body scenario at both Scale 0 (voxels) and Scale 1 (ParticleEngine). |
| [`test_eft_ward_identity.cpp`](../../engine/tests/test_eft_ward_identity.cpp) | 232 | @file test_eft_ward_identity.cpp @brief EFT Phase 1C — Ward-identity test suite. |
| [`test_scale_context.cpp`](../../engine/tests/test_scale_context.cpp) | 232 | Test: ScaleContext readout admissibility gate (C_scale) Verifies the read-only scale-context diagnostics layer that decides whether an engine cloud is eligible for public physical readout. |
| [`campaign_quark_quantization.cpp`](../../engine/tests/campaign_quark_quantization.cpp) | 231 | @file campaign_quark_quantization.cpp @brief FTD-0273 Phase 2 — quantize a colored "quark" with voxels; observe its phenomena. |
| [`test_eft_operator_spectrum.cpp`](../../engine/tests/test_eft_operator_spectrum.cpp) | 231 | @file test_eft_operator_spectrum.cpp @brief EFT Phase 3 — operator-basis scaling-dimension extraction. |
| [`test_native_continuity.cpp`](../../engine/tests/test_native_continuity.cpp) | 231 | Native continuity audit for signed ternary state transport. |
| [`campaign_alpha_no_alpha_probe.cpp`](../../engine/tests/campaign_alpha_no_alpha_probe.cpp) | 230 | FTD-0285: fixed no-alpha-input alpha probe. |
| [`test_csv_export.cpp`](../../engine/tests/test_csv_export.cpp) | 229 | Test: CSV Export Utility Verifies that the csv_export.h functions produce valid CSV files with correct headers, dimensions, and data content. |
| [`test_wh_clifford_anticommutator.cpp`](../../engine/tests/test_wh_clifford_anticommutator.cpp) | 229 | @file test_wh_clifford_anticommutator.cpp @brief Measure the anticommutator of the engine-induced product on the three weight-1 Walsh–Hadamard modes of a 2^3 block. |
| [`test_dark_matter.cpp`](../../engine/tests/test_dark_matter.cpp) | 226 | Test: Dark Matter (Sub-Threshold Flux) Verifies that flux with 0 < \|J\| < K_B behaves as dark matter: present but not manifested, gravitates but does not interact electromagnetically. |
| [`test_mixed_history_flow.cpp`](../../engine/tests/test_mixed_history_flow.cpp) | 226 | @file test_mixed_history_flow.cpp @brief P1.3 + P1.4 closure: multi-tick mixed-toggle reaction-transport Ward identity. |
| [`test_constants.cpp`](../../engine/tests/test_constants.cpp) | 225 | Test: Derivation chain D=3 -> alpha Verifies that all constants are self-consistently derived from D=3 + varpi (lemniscate constant). |
| [`test_determinism.cpp`](../../engine/tests/test_determinism.cpp) | 223 | @file test_determinism.cpp @brief Bit-identical reproducibility under fixed seed. |
| [`test_smallest_particle_emergence.cpp`](../../engine/tests/test_smallest_particle_emergence.cpp) | 223 | @file test_smallest_particle_emergence.cpp @brief Phase-4h: Material emergence from the lattice. |
| [`benchmark_lorentz_recovery.cpp`](../../engine/tests/benchmark_lorentz_recovery.cpp) | 222 | @file benchmark_lorentz_recovery.cpp @brief EFT Phase 1B — Lorentz-covariance recovery benchmark. |
| [`test_native_reaction_ledger.cpp`](../../engine/tests/test_native_reaction_ledger.cpp) | 222 | Native reaction ledger for signed ternary state changes. |
| [`campaign_determinism_gate.cpp`](../../engine/tests/campaign_determinism_gate.cpp) | 221 | @file campaign_determinism_gate.cpp @brief GATE: is the langevin-OFF genesis spectroscopy harness deterministic? |
| [`test_cluster_tracker.cpp`](../../engine/tests/test_cluster_tracker.cpp) | 221 | Test: ClusterTracker (Class B Phase B.1) Smoke + invariant tests for the ClusterTracker introduced as the first concrete deliverable of the Discrete-Native Derivation Program (FTD-0136). |
| [`test_gpu_eft_parity.cpp`](../../engine/tests/test_gpu_eft_parity.cpp) | 221 | @file test_gpu_eft_parity.cpp @brief GPU vs CPU parity tests for the EFT operators and blocking map. |
| [`test_momentum.cpp`](../../engine/tests/test_momentum.cpp) | 221 | Test: Momentum Conservation — Noether Current from Translation Symmetry Verifies that total flux momentum is conserved in closed systems (no external forces, no boundary effects). |
| [`campaign_alpha_estimator_validation.cpp`](../../engine/tests/campaign_alpha_estimator_validation.cpp) | 220 | FTD-0286: alpha estimator validation after FTD-0285 invalidated. |
| [`test_annihilation.cpp`](../../engine/tests/test_annihilation.cpp) | 220 | Test: Annihilation — Matter-Antimatter Energy Conservation Verifies that when a +1 and -1 particle annihilate: 1. |
| [`test_gauge.cpp`](../../engine/tests/test_gauge.cpp) | 220 | Test: Gauge Invariance — J -> J + grad(lambda) Symmetry Verifies that physical observables are invariant under gauge transformations J -> J + grad(lambda) for arbitrary scalar lambda. |
| [`test_wave_collapse.cpp`](../../engine/tests/test_wave_collapse.cpp) | 219 | Test: Wave Collapse — Flux Concentration & Manifestation Verifies that manifestation acts as wave function collapse: 1. |
| [`campaign_triad_binding.cpp`](../../engine/tests/campaign_triad_binding.cpp) | 217 | Campaign: Triad Binding Energy (Phase 8 — Particle Zoo) Tests whether three same-sign particles in an equilateral triangle configuration form a bound state with measurable binding energy. |
| [`test_discrete_operators.cpp`](../../engine/tests/test_discrete_operators.cpp) | 217 | Test: Discrete differential operators Verifies laplacian_flux(), divergence_flux(), curl_flux(), gradient_density() on small lattices with known configurations. |
| [`campaign_gravitational_wave.cpp`](../../engine/tests/campaign_gravitational_wave.cpp) | 216 | Campaign: Gravitational Wave Detection (Phase 7 — Gravitational Sector) Tests whether oscillating mass distributions produce propagating density perturbations — the FTD analog of gravitational waves. |
| [`campaign_de_broglie_guidance.cpp`](../../engine/tests/campaign_de_broglie_guidance.cpp) | 215 | ============================================================================ campaign_de_broglie_guidance.cpp (FTD-0271 Phase E, 2026-06-11) --------------------------------------------------------... |
| [`test_a1g_bridge_i_empirical.cpp`](../../engine/tests/test_a1g_bridge_i_empirical.cpp) | 214 | Test: A_{1g}-fraction characterization of the FTD pipeline (FTD-0110). |
| [`test_genesis.cpp`](../../engine/tests/test_genesis.cpp) | 214 | Test: Genesis — Pair Production from Flux Collision Verifies that when two high-energy flux waves collide, particle pairs are created via the manifestation mechanism: - Density > K_GENESIS triggers... |
| [`test_flux_propagator.cpp`](../../engine/tests/test_flux_propagator.cpp) | 213 | @file test_flux_propagator.cpp @brief Phase-4g: measure the 2-point flux correlator on the Langevin ensemble and classify as bosonic-vector vs fermionic/anomalous. |
| [`campaign_weak_decay.cpp`](../../engine/tests/campaign_weak_decay.cpp) | 212 | Campaign: Weak Decay Rate (Phase 6 — Weak Sector & SU(2)) Measures how transmutation rate depends on stress level, verifying the exponential probability formula. |
| [`test_gpu_shell_256.cpp`](../../engine/tests/test_gpu_shell_256.cpp) | 212 | GPU Shell Predictions at 256^3 — High-Precision Measurement Tests three structural predictions about the electron's self-field at 256^3 lattice resolution on GPU (GPU). |
| [`test_selective_damping.cpp`](../../engine/tests/test_selective_damping.cpp) | 212 | Test: Selective Damping (Phase D — FDTD Bridge) Verifies that selective_damping = true preserves vacuum EM waves while still damping flux near manifested particles. |
| [`dump_koopman_trajectory.cpp`](../../engine/tests/dump_koopman_trajectory.cpp) | 211 | FTD Koopman Observable Dumper Injects the A=14 canonical cloud and runs the Langevin bath. |
| [`test_em_fields.cpp`](../../engine/tests/test_em_fields.cpp) | 210 | Test: E/B Field Diagnostics (Phase A — FDTD Bridge) Verifies the electromagnetic field decomposition: E = -wave_vel (electric field from leapfrog momentum) B = curl(J) (magnetic field from flux cur... |
| [`test_cluster_mask_persistence.cpp`](../../engine/tests/test_cluster_mask_persistence.cpp) | 209 | Phase B.3 protocol candidate: position-fixed mask persistence. |
| [`campaign_gravity_hierarchy.cpp`](../../engine/tests/campaign_gravity_hierarchy.cpp) | 208 | Campaign: Gravitational Hierarchy (Phase 7 — Gravitational Sector) Verifies the gravitational coupling hierarchy derived from the ontic chain: why gravity is 10^39 times weaker than EM in the physi... |
| [`test_polarization.cpp`](../../engine/tests/test_polarization.cpp) | 207 | Test: Polarization Counting — 2 Transverse Modes Verifies that the flux field has exactly 2 independent propagating polarization modes, as expected from: 3 components - 1 Gauss constraint = 2 physi... |
| [`test_moore_laplacian_isotropy.cpp`](../../engine/tests/test_moore_laplacian_isotropy.cpp) | 206 | test_moore_laplacian_isotropy.cpp — characterises TRACKER §1.8. |
| [`test_scale_bridge.cpp`](../../engine/tests/test_scale_bridge.cpp) | 205 | Phase 7 — Stage 3: Scale Bridge unit tests (8 checks) SB1: Coarsen charge matches voxel state SB2: Coarsen position matches coord + remainder SB3: Coarsen velocity preserved exactly SB4: Refine pla... |
| [`test_eft_matched_poisson.cpp`](../../engine/tests/test_eft_matched_poisson.cpp) | 204 | @file test_eft_matched_poisson.cpp @brief Day-2 Ticket A — matched-stencil CG Poisson solver validation. |
| [`campaign_alpha_estimator_validation_v2.cpp`](../../engine/tests/campaign_alpha_estimator_validation_v2.cpp) | 202 | FTD-0286 v2: alpha estimator validation — half-energy gate pairing. |
| [`campaign_amplitude_time_series.cpp`](../../engine/tests/campaign_amplitude_time_series.cpp) | 202 | @file campaign_amplitude_time_series.cpp @brief Per-tick cluster-size logging at custom injection amplitude. |
| [`test_cluster_persistence_quiescent.cpp`](../../engine/tests/test_cluster_persistence_quiescent.cpp) | 202 | Test: Cluster Persistence Under Quiescent Dynamics (Class B Phase B.2) Per SPEC_CLASS_B_CLUSTER_PERSISTENCE.md §6.2: "Verify deterministic engine produces tau -> infty for all single clusters under... |
| [`campaign_cosmological_predictions.cpp`](../../engine/tests/campaign_cosmological_predictions.cpp) | 201 | Campaign: Cosmological Predictions (Phase 9 — Cosmological Validation) Verifies cosmological observables derived from framework integers {3, 4, 7, 13} and the master quadratic. |
| [`campaign_spontaneous_structure.cpp`](../../engine/tests/campaign_spontaneous_structure.cpp) | 201 | Campaign: Spontaneous Structure Formation 6 free particles (3+, 3-) with small random velocities on 48^3 lattice. |
| [`test_vtk_export.cpp`](../../engine/tests/test_vtk_export.cpp) | 201 | Test: Native ParaView/VTK XML export. |
| [`campaign_weak_transmutation.cpp`](../../engine/tests/campaign_weak_transmutation.cpp) | 199 | Campaign: Weak Transmutation (Phase 6 — Weak Sector & SU(2)) Tests stress-threshold polarity flipping (+1 <-> -1) as the FTD analog of weak interactions (beta decay). |
| [`benchmark_phase_i_native_coupling.cpp`](../../engine/tests/benchmark_phase_i_native_coupling.cpp) | 198 | Phase I — FTD-Native Coupling Cross-Check (FTD-0125) Pre-registration: docs/theory/10_eft_program/PREREG_PHASE_I_NATIVE_COUPLING.md git tag: preregister-phase-i-native-coupling-v1 (commit e1f8157)... |
| [`test_de_broglie_clock.cpp`](../../engine/tests/test_de_broglie_clock.cpp) | 198 | ============================================================================ test_de_broglie_clock.cpp (FTD-0271 Phase A, 2026-06-11) ---------------------------------------------------------------... |
| [`test_eft_anisotropy.cpp`](../../engine/tests/test_eft_anisotropy.cpp) | 197 | @file test_eft_anisotropy.cpp @brief EFT Phase 1A — rotational-anisotropy diagnostics. |
| [`test_fractional_heat_flow.cpp`](../../engine/tests/test_fractional_heat_flow.cpp) | 197 | Test: Fractional Heat Flow (Continuum Limit) Verifies that the FTD lattice wave equation transitions from purely ballistic wave propagation (r_rms ~ t^1) to fractional diffusion/heat flow (r_rms ~... |
| [`test_particle_toggles.cpp`](../../engine/tests/test_particle_toggles.cpp) | 197 | Test: ParticleToggles — Per-force toggle control for ParticleEngine Verifies that each toggle in ParticleToggles enables/disables its force, and that force_diag_ correctly decomposes forces by type. |
| [`test_coupling_readout_sweeps.cpp`](../../engine/tests/test_coupling_readout_sweeps.cpp) | 196 | Test: Coupling Readout Sweeps (Class C Phase C.3) Implements the automated sweeps and coupling constant extraction of the Class C Infrastructure Specification (FTD-0222). |
| [`test_helpers.h`](../../engine/tests/test_helpers.h) | 196 | @file test_helpers.h @brief Shared test utilities — checks, inspectors, toggle presets. |
| [`test_a1g_projector.cpp`](../../engine/tests/test_a1g_projector.cpp) | 195 | Unit test for the A_{1g} projector on a 27-voxel Moore block. |
| [`test_atom_toggles.cpp`](../../engine/tests/test_atom_toggles.cpp) | 195 | Test: AtomToggles — Per-force toggle control for AtomEngine Verifies that each toggle in AtomToggles enables/disables its force, and that force_diag_ correctly decomposes forces by type. |
| [`test_native_source_response.cpp`](../../engine/tests/test_native_source_response.cpp) | 195 | Native full-tick source-response audit. |
| [`benchmark_ewsb_threshold_map.cpp`](../../engine/tests/benchmark_ewsb_threshold_map.cpp) | 193 | @file benchmark_ewsb_threshold_map.cpp @brief Gap-closure Ticket 4 / Day 2 Thread 1b — EWSB amplitude threshold map. |
| [`test_tracker.cpp`](../../engine/tests/test_tracker.cpp) | 193 | Test: Particle Tracker (Phase 1 — Measurement Infrastructure) Verifies that the Tracker correctly records particle trajectories using the engine's existing particle_id infrastructure. |
| [`test_gravity_dynamics.cpp`](../../engine/tests/test_gravity_dynamics.cpp) | 192 | Test: Gravity Dynamics — Gravitational Attraction from Density Gradient Verifies that the gravity term in phase_forces: F_grav = G_N · ∇ρ where G_N = 1/(b₃+N_c)² = 1/100 = 0.01: 1. |
| [`campaign_triad_energy.cpp`](../../engine/tests/campaign_triad_energy.cpp) | 191 | Campaign: Triad Energy Measurement (Phase 4 — Emergent Mass Spectrum) Measures the total energy of locked triads (3 same-sign particles) and compares with single-particle energy to extract binding... |
| [`test_atom_scale_bridge.cpp`](../../engine/tests/test_atom_scale_bridge.cpp) | 191 | Test: Atom Scale Bridge (Scale 1 ↔ Scale 2) 6 checks covering coarsen_to_atoms and refine_to_particles. |
| [`test_born_infeld.cpp`](../../engine/tests/test_born_infeld.cpp) | 190 | Test: Born-Infeld Lagrangian Predictions Verifies: 1. |
| [`test_flux_link_clifford.cpp`](../../engine/tests/test_flux_link_clifford.cpp) | 190 | @file test_flux_link_clifford.cpp @brief Phase-4f: fermion-emergence test on the FLUX 1-form (link-like degrees of freedom), not the state 0-form. |
| [`test_correlations.cpp`](../../engine/tests/test_correlations.cpp) | 188 | Test: Correlation Functions (Phase 1 — Measurement Infrastructure) Verifies that spatial and temporal correlation functions work correctly on known field configurations. |
| [`test_gpu_benchmark.cpp`](../../engine/tests/test_gpu_benchmark.cpp) | 188 | GPU performance benchmark for the FTD CUDA engine. |
| [`test_native_conserved_parent.cpp`](../../engine/tests/test_native_conserved_parent.cpp) | 188 | Native conserved-parent audit for weak transmutation. |
| [`test_consciousness.cpp`](../../engine/tests/test_consciousness.cpp) | 187 | Test: Reference frame context Quadratic Verifies the reference frame context sector of the ontic derivation chain: the master quadratic with k = 1/2 produces complex roots whose real and imaginary... |
| [`test_reaction_operators.cpp`](../../engine/tests/test_reaction_operators.cpp) | 187 | @file test_reaction_operators.cpp @brief Unit tests for the FTD-0112 reaction-sector operators (O7-O10). |
| [`campaign_genesis_geometry.cpp`](../../engine/tests/campaign_genesis_geometry.cpp) | 186 | @file campaign_genesis_geometry.cpp @brief FTD-0110 nonlinear bridge: per-fired-voxel FIRING GEOMETRY in the engine. |
| [`test_wilson_dirac_smoke.cpp`](../../engine/tests/test_wilson_dirac_smoke.cpp) | 186 | Wilson-Dirac smoke test (Phase II.2-A milestone). |
| [`campaign_born_ensemble.cpp`](../../engine/tests/campaign_born_ensemble.cpp) | 185 | Phase 7 — Stage 6: Born Rule Ensemble (4 checks) Demonstrate that the Born rule P(x) = \|psi(x)\|^2 emerges as the ensemble average over sub-scale initial conditions. |
| [`test_z3_color_center.cpp`](../../engine/tests/test_z3_color_center.cpp) | 185 | test_z3_color_center.cpp — Z_3 center-closure [THEOREM] verification. |
| [`campaign_born_rule.cpp`](../../engine/tests/campaign_born_rule.cpp) | 184 | Campaign: Born Rule from Genesis Statistics (Phase 3 — Quantum Mechanics) Validates that particle manifestation (genesis) follows Born rule: P(x) ∝ \|J(x)\|² Theory: FTD genesis probability is p = cl... |
| [`test_mechanism_b.cpp`](../../engine/tests/test_mechanism_b.cpp) | 184 | Test: Mechanism B (Lattice-to-Continuum Matching via Vacuum Polarization) Implements the explicit stochastic quantization of the FTD engine. |
| [`test_langevin_equipartition.cpp`](../../engine/tests/test_langevin_equipartition.cpp) | 183 | @file test_langevin_equipartition.cpp @brief Verify the Langevin thermostat produces the expected equilibrium. |
| [`test_bridge_dynamics.cpp`](../../engine/tests/test_bridge_dynamics.cpp) | 182 | Test: RenderBridge tick dynamics Integration tests for vacuum stability, flux injection, propagation, manifestation, and diagnostics. |
| [`test_cluster_persistence_toggle_sweep.cpp`](../../engine/tests/test_cluster_persistence_toggle_sweep.cpp) | 182 | Test: Cluster Persistence — toggle configuration sweep (B.2 diagnosis (b)) The alpha-sweep diagnostic (test_cluster_persistence_alpha_sweep.cpp) showed cluster lifetimes saturate at ~45 ticks even... |
| [`test_native_projection_convergence.cpp`](../../engine/tests/test_native_projection_convergence.cpp) | 182 | Native Gauss-projection convergence audit. |
| [`test_sublattice_laplacian.cpp`](../../engine/tests/test_sublattice_laplacian.cpp) | 181 | @file test_sublattice_laplacian.cpp @brief Validate sublattice-projected Laplacians (laplacian_sc, _fcc, _bcc). |
| [`test_cognitive_lattice.cpp`](../../engine/tests/test_cognitive_lattice.cpp) | 180 |  |
| [`campaign_cluster_relaxation.cpp`](../../engine/tests/campaign_cluster_relaxation.cpp) | 179 | campaign_cluster_relaxation — Exp-B of the cluster-thermodynamics EXPLORATORY pass (P4 N_internal + P1 cost<->N). |
| [`test_native_current_flow.cpp`](../../engine/tests/test_native_current_flow.cpp) | 179 | Native current-flow audit for finite-volume b=2 blocking. |
| [`campaign_structure_stability.cpp`](../../engine/tests/campaign_structure_stability.cpp) | 178 | Campaign: Structure Stability Survey (Phase 4 — Emergent Mass Spectrum) Tests which particle configurations survive long evolution (5000 ticks) under EM + gravity dynamics. |
| [`graviton_fft_cuda.h`](../../engine/tests/graviton_fft_cuda.h) | 178 | graviton_fft_cuda.h — GPU (cuFFT) backend for the per-tick 3D FFTs of campaign_graviton_tt_correlator.cpp. |
| [`test_erdos_unit_distance.cpp`](../../engine/tests/test_erdos_unit_distance.cpp) | 178 |  |
| [`campaign_inertial_mass.cpp`](../../engine/tests/campaign_inertial_mass.cpp) | 177 | Campaign: Inertial Mass Measurement (Phase 4 — Emergent Mass Spectrum) Measures effective inertial mass of manifested particles via F = ma. |
| [`test_callstack_audit_fixes.cpp`](../../engine/tests/test_callstack_audit_fixes.cpp) | 176 | test_callstack_audit_fixes.cpp — verifies the 2026-04-17 callstack audit fixes (findings F1–F8). |
| [`test_cluster_interaction_dynamic.cpp`](../../engine/tests/test_cluster_interaction_dynamic.cpp) | 176 | Test: Dynamic Cluster-Cluster Interaction (Class C Phase C.2) Implements the Dynamical Scattering Protocol of the Class C Infrastructure Specification (FTD-0222). |
| [`test_radiative_decay_scale1.cpp`](../../engine/tests/test_radiative_decay_scale1.cpp) | 176 | Radiative Decay at Scale 1: Orbit Shrinkage from Larmor Radiation FTD note: radiation reaction is [IMPOSED] physics — the Larmor formula P = (2α/3) q²a²/(mc³) is adopted from SM, with the coefficie... |
| [`test_cosmological_constant.cpp`](../../engine/tests/test_cosmological_constant.cpp) | 175 | Test: Cosmological Constant Verifies that the vacuum energy density from the dual-substrate framework gives Omega_Lambda = 2/3, consistent with the FTD cosmological constant conjecture. |
| [`test_ramsey_multicolor.cpp`](../../engine/tests/test_ramsey_multicolor.cpp) | 171 |  |
| [`test_spectral.cpp`](../../engine/tests/test_spectral.cpp) | 170 | Test: Spectral Analysis (Phase 1 — Measurement Infrastructure) Verifies FFT implementation and dispersion relation measurement. |
| [`test_cluster_interaction_static.cpp`](../../engine/tests/test_cluster_interaction_static.cpp) | 169 | Test: Static Cluster-Cluster Interaction (Class C Phase C.1) Implements the Static Template of the Class C Infrastructure Specification (FTD-0222). |
| [`test_native_manifestation_ledger.cpp`](../../engine/tests/test_native_manifestation_ledger.cpp) | 167 | Native manifestation ledger for genesis and evaporation. |
| [`test_portable_field.cpp`](../../engine/tests/test_portable_field.cpp) | 167 | Test: Portable Self-Field Verifies that particles carry their flux when they move. |
| [`test_native_engine_history_flow.cpp`](../../engine/tests/test_native_engine_history_flow.cpp) | 166 | Native engine-history flow audit. |
| [`benchmark_invariant_matrix_constant_memory.cu`](../../engine/tests/benchmark_invariant_matrix_constant_memory.cu) | 165 | benchmark_invariant_matrix_constant_memory.cu Standalone benchmark for the CUDA constant-memory invariant pattern established by ADR-0014. |
| [`test_moore26_clifford_test.cpp`](../../engine/tests/test_moore26_clifford_test.cpp) | 164 | @file test_moore26_clifford_test.cpp @brief Phase-4c fermion-emergence route: Moore-26 / 3³ block with axial sawtooth modes as "weight-1" generators. |
| [`test_spin_field_clifford.cpp`](../../engine/tests/test_spin_field_clifford.cpp) | 164 | @file test_spin_field_clifford.cpp @brief Phase-4e: fermion-emergence test on the SPIN field, not the state field. |
| [`campaign_scale_context_confine.cpp`](../../engine/tests/campaign_scale_context_confine.cpp) | 163 | campaign_scale_context_confine.cpp Confinement scan for the scale-context readout admissibility gate. |
| [`campaign_statistical_convergence.cpp`](../../engine/tests/campaign_statistical_convergence.cpp) | 162 | Campaign: Statistical Convergence (Phase 1 — Measurement Infrastructure) Validates that ensemble moments converge as N_runs increases. |
| [`test_molecular_dihedrals.cpp`](../../engine/tests/test_molecular_dihedrals.cpp) | 162 | @file test_molecular_dihedrals.cpp @brief Torsional dihedrals and improper planarity potentials unit test. |
| [`test_gamma_ftd_momentum.cpp`](../../engine/tests/test_gamma_ftd_momentum.cpp) | 161 | test_gamma_ftd_momentum.cpp — verifies γ_FTD momentum integration in phase_forces (closes TRACKER_OPEN_ITEMS §1.2). |
| [`test_cluster_persistence_alpha_sweep.cpp`](../../engine/tests/test_cluster_persistence_alpha_sweep.cpp) | 159 | Test: Cluster Persistence — alpha sensitivity sweep (Phase B.2 diagnostic) FINDING B.2-B from test_cluster_persistence_quiescent: clusters nucleate under FTD-0110-canonical injection but dissolve w... |
| [`test_watson_integrals.cpp`](../../engine/tests/test_watson_integrals.cpp) | 159 | @file test_watson_integrals.cpp @brief Numerical Watson integrals for SC, BCC, FCC, and Moore-18 stencils. |
| [`test_cpu_warnings.cpp`](../../engine/tests/test_cpu_warnings.cpp) | 158 | @file test_cpu_warnings.cpp @brief Verify CPU-build runtime warnings fire for GPU-only toggles. |
| [`campaign_hydrogen_binding.cpp`](../../engine/tests/campaign_hydrogen_binding.cpp) | 157 | Campaign: Hydrogen-Like Bound State (Phase 4 — Emergent Mass Spectrum) Tests whether opposite-charge particles form stable bound states with measurable binding energy and orbital structure. |
| [`test_lattice_operators.cpp`](../../engine/tests/test_lattice_operators.cpp) | 157 | Test: Extended lattice topology and wrapping Complements test_lattice.cpp with additional checks: neighbor symmetry, self-reference exclusion, boundary wrapping edge cases, and multi-size sanity. |
| [`test_voxel_properties.cpp`](../../engine/tests/test_voxel_properties.cpp) | 156 | Test: Voxel derived quantities Verifies density(), speed(), bandwidth_used(), gamma_ftd(), and born_infeld_core() for known inputs. |
| [`test_langevin_gpu_cpu_parity.cpp`](../../engine/tests/test_langevin_gpu_cpu_parity.cpp) | 154 | @file test_langevin_gpu_cpu_parity.cpp @brief Langevin thermostat: equipartition + GPU/CPU statistical agreement. |
| [`test_leapfrog_integrator_audit.cpp`](../../engine/tests/test_leapfrog_integrator_audit.cpp) | 154 | test_leapfrog_integrator_audit.cpp — closes TRACKER_OPEN_ITEMS §1.4. |
| [`campaign_hydrogen_lscan.cpp`](../../engine/tests/campaign_hydrogen_lscan.cpp) | 153 | Campaign: Hydrogen L-scan — does s0-seed-hydrogen stay a stable atom, or does it flood/condense the periodic box, and is the onset L-dependent? |
| [`test_energy_conservation_tight.cpp`](../../engine/tests/test_energy_conservation_tight.cpp) | 153 | @file test_energy_conservation_tight.cpp @brief Symplectic-leapfrog energy conservation: bounded oscillation, no drift. |
| [`test_wilson_dirac_limit.cpp`](../../engine/tests/test_wilson_dirac_limit.cpp) | 153 | Wilson-Dirac limit consistency (Phase II.2-D milestone). |
| [`test_wilson_dirac_bz_spectrum.cpp`](../../engine/tests/test_wilson_dirac_bz_spectrum.cpp) | 152 | Wilson-Dirac full-BZ spectrum sweep (Phase II.2-B milestone). |
| [`test_wilson_dirac_cuda_parity.cpp`](../../engine/tests/test_wilson_dirac_cuda_parity.cpp) | 152 | Wilson-Dirac CPU/GPU parity (Phase II.2-E milestone). |
| [`benchmark_beta_function.cpp`](../../engine/tests/benchmark_beta_function.cpp) | 151 | @file benchmark_beta_function.cpp @brief EFT Phase 2C — lattice-measured β(g) via multi-scale α_eff extraction. |
| [`test_ensemble.cpp`](../../engine/tests/test_ensemble.cpp) | 151 | Test: Ensemble Runner (Phase 1 — Measurement Infrastructure) Verifies that ensemble statistics work correctly: EN1: 5-run ensemble on identical setup produces non-zero variance (stochastic genesis... |
| [`test_master_quadratic_identities.cpp`](../../engine/tests/test_master_quadratic_identities.cpp) | 150 | @file test_master_quadratic_identities.cpp @brief Numerical verification of the bare algebraic content of the master quadratic. |
| [`benchmark_sm_masses_gpu.cpp`](../../engine/tests/benchmark_sm_masses_gpu.cpp) | 149 | @file benchmark_sm_masses_gpu.cpp @brief GPU exploratory Standard Model hierarchy benchmark Computes the equilibrium field energy for fundamental particles on the ternary lattice and compares ratio... |
| [`campaign_genesis_criticality.cpp`](../../engine/tests/campaign_genesis_criticality.cpp) | 147 | @file campaign_genesis_criticality.cpp @brief Order of the FTD genesis/manifestation transition (RG-spectrum probe). |
| [`test_gpu_dissipation_source.cpp`](../../engine/tests/test_gpu_dissipation_source.cpp) | 147 | Dissipation Source Analysis Question: Where does energy go in each phase of the tick cycle? |
| [`test_fine_structure_scale1.cpp`](../../engine/tests/test_fine_structure_scale1.cpp) | 146 | Fine Structure at Scale 1: Spin-Orbit Splitting FTD Multi-Scale: Scale 1 has spin-orbit and relativistic toggles already implemented. |
| [`test_observable_commutativity.cpp`](../../engine/tests/test_observable_commutativity.cpp) | 146 | ============================================================================ test_observable_commutativity.cpp ---------------------------------------------------------------------------- Part C of... |
| [`dump_full_physics.cpp`](../../engine/tests/dump_full_physics.cpp) | 144 | Complete-physics-lattice test: all FTD physics toggles ON. |
| [`bridge_fixtures.cpp`](../../engine/tests/support/bridge_fixtures.cpp) | 143 | ============================================================================ tests/support/bridge_fixtures.cpp ---------------------------------------------------------------------------- Implement... |
| [`test_toggle_matrix.cpp`](../../engine/tests/test_toggle_matrix.cpp) | 143 | @file test_toggle_matrix.cpp @brief Pairwise toggle-combination smoke test. |
| [`test_variational_coulomb.cpp`](../../engine/tests/test_variational_coulomb.cpp) | 143 | Test: Variational Coulomb -- Field-Mediated Electrostatics Verifies that the coupling term L_COUPLING = -g_c * s * div(J) produces the correct Coulomb force via its Euler-Lagrange equation: F = -al... |
| [`campaign_genesis_moore_signature.cpp`](../../engine/tests/campaign_genesis_moore_signature.cpp) | 142 | @file campaign_genesis_moore_signature.cpp @brief Do genesis CLUSTERS carry an O_h / Moore quantum-number signature? |
| [`campaign_h2_molecule.cpp`](../../engine/tests/campaign_h2_molecule.cpp) | 141 | Campaign: H2 Molecule Formation Two hydrogen atoms approach, form a covalent bond, settle into vibrational equilibrium, and conserve energy. |
| [`test_native_moore_layer_coupling.cpp`](../../engine/tests/test_native_moore_layer_coupling.cpp) | 141 | Native Moore-layer coupling audit. |
| [`benchmark_ewsb_pipe.cpp`](../../engine/tests/benchmark_ewsb_pipe.cpp) | 140 | @file benchmark_ewsb_pipe.cpp @brief EWSB amplitude-threshold map — Phase E port of benchmark_ewsb_threshold_map. |
| [`dump_toggle_bisection.cpp`](../../engine/tests/dump_toggle_bisection.cpp) | 140 | Toggle-bisection: which physics toggle drives which feature? |
| [`test_wilson_topology.cpp`](../../engine/tests/test_wilson_topology.cpp) | 138 | test_wilson_topology.cpp — Phase I Item 3 Mechanism A diagnostic. |
| [`graviton_fft_cuda.cu`](../../engine/tests/graviton_fft_cuda.cu) | 137 | graviton_fft_cuda.cu — cuFFT (GPU) implementation of the batched double-precision 3D FFT service declared in graviton_fft_cuda.h. |
| [`test_correlations_diagonal.cpp`](../../engine/tests/test_correlations_diagonal.cpp) | 137 | @file test_correlations_diagonal.cpp @brief Validate sublattice-filtered + diagonal-displacement correlators. |
| [`campaign_drain_scan.cpp`](../../engine/tests/campaign_drain_scan.cpp) | 136 | @file campaign_drain_scan.cpp @brief FTD-0276 Leg A: does the cluster-efficiency k_eff scale as drain²? |
| [`campaign_vacuum_energy.cpp`](../../engine/tests/campaign_vacuum_energy.cpp) | 136 |  |
| [`test_langevin_sublattice_equipartition.cpp`](../../engine/tests/test_langevin_sublattice_equipartition.cpp) | 136 | @file test_langevin_sublattice_equipartition.cpp @brief Verify the Langevin thermostat with site-class filter only thermalizes the selected parity class. |
| [`test_symmetric_movement.cpp`](../../engine/tests/test_symmetric_movement.cpp) | 135 | Test: Symmetric Movement and Coordinate-Independent Chirality Density Verifies that: 1. |
| [`test_db_clock_coulomb.cpp`](../../engine/tests/test_db_clock_coulomb.cpp) | 134 | ============================================================================ test_db_clock_coulomb.cpp (FTD-0281 hook smoke, 2026-06-13) ------------------------------------------------------------... |
| [`test_force_diag_parity.cpp`](../../engine/tests/test_force_diag_parity.cpp) | 134 | test_force_diag_parity.cpp CPU-vs-GPU parity test for the force_diag mirror added 2026-04-25. |
| [`test_two_state_extraction.cpp`](../../engine/tests/test_two_state_extraction.cpp) | 134 | @file test_two_state_extraction.cpp @brief Validate Prony + GEVP two-state extractors on synthetic data. |
| [`benchmark_beta_function_pipe.cpp`](../../engine/tests/benchmark_beta_function_pipe.cpp) | 133 | @file benchmark_beta_function_pipe.cpp @brief Pipeline-based β-function benchmark — Phase E port of benchmark_beta_function. |
| [`campaign_bound_lifetime.cpp`](../../engine/tests/campaign_bound_lifetime.cpp) | 133 | Campaign: Bound State Lifetime Place free +1 and -1 at various separations on 32^3 lattice. |
| [`test_scenario_velocity_wiring.cpp`](../../engine/tests/test_scenario_velocity_wiring.cpp) | 133 | test_scenario_velocity_wiring.cpp Audit item (physics-orchestrator, 2026-04-18): after porting the JS flux-meson / flux-string-breaking / flux-baryon scenarios to engine/src/scenarios.cpp, verify t... |
| [`test_erdos_capset.cpp`](../../engine/tests/test_erdos_capset.cpp) | 130 |  |
| [`test_knot_tracking_golden.cpp`](../../engine/tests/test_knot_tracking_golden.cpp) | 130 | ============================================================================ test_knot_tracking_golden.cpp ---------------------------------------------------------------------------- Proves the kn... |
| [`test_strict_validation.cpp`](../../engine/tests/test_strict_validation.cpp) | 130 | @file test_strict_validation.cpp @brief Verify ARCH-3 toggle-validator strictness contract. |
| [`campaign_genesis_hysteresis.cpp`](../../engine/tests/campaign_genesis_hysteresis.cpp) | 129 | @file campaign_genesis_hysteresis.cpp @brief First-order confirmation for the FTD genesis transition: HYSTERESIS. |
| [`benchmark_nucleon_mass.cpp`](../../engine/tests/benchmark_nucleon_mass.cpp) | 126 | @file benchmark_nucleon_mass.cpp @brief Dynamical Nucleon Mass Benchmark Tests the triad (nucleon analog) binding energy under physical fine-structure coupling limits rather than geometric limits. |
| [`test_native_moore_temporal_layers.cpp`](../../engine/tests/test_native_moore_temporal_layers.cpp) | 125 | Native Moore temporal-layer audit. |
| [`test_annihilation_conservation.cpp`](../../engine/tests/test_annihilation_conservation.cpp) | 124 | Test: Annihilation Flux Conservation Verifies that annihilation conserves total flux energy. |
| [`campaign_alpha_readout_scattering.cpp`](../../engine/tests/campaign_alpha_readout_scattering.cpp) | 123 | campaign_alpha_readout_scattering.cpp ARC-D1 Empirical Readout Campaign Injects a stable cluster (A=14) and applies a minimal flux perturbation (delta=0.5). |
| [`test_cpu_gpu_divergence.cpp`](../../engine/tests/test_cpu_gpu_divergence.cpp) | 123 |  |
| [`test_native_blocking_map.cpp`](../../engine/tests/test_native_blocking_map.cpp) | 121 | Native finite-volume blocking map audit. |
| [`test_phase_h_regression.cpp`](../../engine/tests/test_phase_h_regression.cpp) | 121 | @file test_phase_h_regression.cpp @brief Phase-H `coulomb_charge_coupling` knob regression. |
| [`benchmark_alpha_scaling.cpp`](../../engine/tests/benchmark_alpha_scaling.cpp) | 120 | @file benchmark_alpha_scaling.cpp @brief First productive use of the FTD-0051 GPU Langevin port: scan measure_alpha_eff across L ∈ {32, 64, 128, 256} on GPU, plus a Langevin-equilibrated variant at... |
| [`test_scale_ratio.cpp`](../../engine/tests/test_scale_ratio.cpp) | 120 | engine/tests/test_scale_ratio.cpp Unit tests for engine/include/ftd/scale_ratio.h FC-3 (SPEC_SCALE_RATIO_ONTOLOGY.md §6) — minimal reference implementation. |
| [`bridge_fixtures.h`](../../engine/tests/support/bridge_fixtures.h) | 118 | ============================================================================ tests/support/bridge_fixtures.h ---------------------------------------------------------------------------- Phase 7 (20... |
| [`test_de_broglie_redshift.cpp`](../../engine/tests/test_de_broglie_redshift.cpp) | 118 | ============================================================================ test_de_broglie_redshift.cpp (FTD-0271 Phase A5, 2026-06-11) -----------------------------------------------------------... |
| [`benchmark_manifestation_flow_cpu.cpp`](../../engine/tests/benchmark_manifestation_flow_cpu.cpp) | 116 | @file benchmark_manifestation_flow_cpu.cpp @brief Single-seed CPU measurement of the FTD-native b=2 flow on a manifestation-dressed background. |
| [`test_cluster_genealogy.cpp`](../../engine/tests/test_cluster_genealogy.cpp) | 116 | test_cluster_genealogy — correctness gate for the genealogy detector. |
| [`benchmark_langevin_gpu.cpp`](../../engine/tests/benchmark_langevin_gpu.cpp) | 114 | @file benchmark_langevin_gpu.cpp @brief Timing benchmark for the Langevin thermostat on CPU vs GPU paths. |
| [`test_native_response_flow.cpp`](../../engine/tests/test_native_response_flow.cpp) | 112 | Native C_L and g_sJ b=2 flow audit. |
| [`benchmark_nucleon_mass_gpu.cpp`](../../engine/tests/benchmark_nucleon_mass_gpu.cpp) | 110 | @file benchmark_nucleon_mass_gpu.cpp @brief GPU Dynamical Nucleon Mass Benchmark Tests the triad (nucleon analog) binding energy under physical fine-structure coupling limits using the CUDA engine. |
| [`campaign_native_scale_flow.cpp`](../../engine/tests/campaign_native_scale_flow.cpp) | 109 |  |
| [`test_relativistic_verlet.cpp`](../../engine/tests/test_relativistic_verlet.cpp) | 109 | @file test_relativistic_verlet.cpp @brief Relativistic Verlet integrator speed cap and momentum verification test. |
| [`test_sublattice_helpers.cpp`](../../engine/tests/test_sublattice_helpers.cpp) | 108 | @file test_sublattice_helpers.cpp @brief Unit tests for sublattice classification and neighbors_8_corner. |
| [`test_dag_engine.cpp`](../../engine/tests/test_dag_engine.cpp) | 106 | test_dag_engine.cpp — SparseVoxelDAG structural-parity tests. |
| [`test_manifestation_background.cpp`](../../engine/tests/test_manifestation_background.cpp) | 106 | @file test_manifestation_background.cpp @brief Unit tests for prepare_manifestation_background. |
| [`test_dissipation.cpp`](../../engine/tests/test_dissipation.cpp) | 105 | Test: Rayleigh Dissipation Function Verifies R = (DAMPING/2) * \|wave_vel\|^2 where DAMPING = alpha [IMPOSED — see ontic.h ASSUMP.6]. |
| [`test_phase_h_coupling.cpp`](../../engine/tests/test_phase_h_coupling.cpp) | 105 | test_phase_h_coupling.cpp — Phase H: explicit coupling constant in Gauss law. |
| [`test_sloop.cpp`](../../engine/tests/test_sloop.cpp) | 105 | Test: Reference frame context Constants and Quadratic Structure Verifies that the reference frame context-sector constants from ontic.h Layer 8 are correctly derived and internally consistent. |
| [`test_dual_cell_adapter.cpp`](../../engine/tests/test_dual_cell_adapter.cpp) | 104 | @file test_dual_cell_adapter.cpp @brief Unit tests for render_bridge_to_dual_cell_fields. |
| [`test_ontic_chain.cpp`](../../engine/tests/test_ontic_chain.cpp) | 103 | Test: Ontic Derivation Chain — γ → Γ(1/4) → ϖ → G* → α → all physics Pure mathematics. |
| [`test_gpu_profile_compare.cpp`](../../engine/tests/test_gpu_profile_compare.cpp) | 100 | Compare inner radial profiles across damping modes. |
| [`test_native_flow.cpp`](../../engine/tests/test_native_flow.cpp) | 98 | Native bare-flow audit for the dual-cell blocking map. |
| [`campaign_ew_phase_transition.cpp`](../../engine/tests/campaign_ew_phase_transition.cpp) | 95 |  |
| [`test_knot_telemetry.cpp`](../../engine/tests/test_knot_telemetry.cpp) | 94 | engine/tests/test_knot_telemetry.cpp Unit test for KnotTracker: per-knot lifecycle + observable assembly. |
| [`test_open5_legacy_flux_l.cpp`](../../engine/tests/test_open5_legacy_flux_l.cpp) | 94 | @file test_open5_legacy_flux_l.cpp @brief OPEN-5 micro-regression: legacy single-substrate inject_flux must leave flux_L untouched when toggles.dual_substrate=false. |
| [`test_field_soa.cpp`](../../engine/tests/test_field_soa.cpp) | 93 |  |
| [`test_lattice.cpp`](../../engine/tests/test_lattice.cpp) | 86 | Test: Lattice operations Verifies periodic boundary conditions, neighbor access, and coordinate mapping. |
| [`campaign_higgs_bi_pair_production.cpp`](../../engine/tests/campaign_higgs_bi_pair_production.cpp) | 79 |  |
| [`test_symplectic_wave.cpp`](../../engine/tests/test_symplectic_wave.cpp) | 78 | @file test_symplectic_wave.cpp @brief Symplectic Leapfrog wave propagation energy conservation test. |
| [`test_boundary_movement.cpp`](../../engine/tests/test_boundary_movement.cpp) | 77 | test_boundary_movement.cpp Verifies phase_movement face handling: reflective_boundary OFF → particle exhausts into the void (no toroidal wrap) reflective_boundary ON → mirror bounce at the face |
| [`test_gf3_codes.cpp`](../../engine/tests/test_gf3_codes.cpp) | 73 |  |
| [`test_ternary_field.cpp`](../../engine/tests/test_ternary_field.cpp) | 73 |  |
| [`test_blume_capel_glass.cpp`](../../engine/tests/test_blume_capel_glass.cpp) | 72 |  |
| [`dump_full_physics_l256.cpp`](../../engine/tests/dump_full_physics_l256.cpp) | 69 | L=256 full-physics spot check — does the FTD-framework-integer pattern continue? |
| [`test_potts_3d.cpp`](../../engine/tests/test_potts_3d.cpp) | 69 |  |
| [`dump_full_physics_amp_scan.cpp`](../../engine/tests/dump_full_physics_amp_scan.cpp) | 68 | Multi-amplitude scan under FULL PHYSICS at L=64. |
| [`test_aperiodic_monotile.cpp`](../../engine/tests/test_aperiodic_monotile.cpp) | 68 | _symbols:_ Tile |
| [`test_telemetry_selftest.cpp`](../../engine/tests/test_telemetry_selftest.cpp) | 67 | Test: ftd::test telemetry library self-test Exercises every public method of ftd/test_telemetry.h in both modes: - FTD_TEST_TELEMETRY unset → human-readable output (matches legacy check()/check_clo... |
| [`test_3d_ca.cpp`](../../engine/tests/test_3d_ca.cpp) | 66 |  |
| [`test_fox_coloring.cpp`](../../engine/tests/test_fox_coloring.cpp) | 61 |  |
| [`test_native_observable_registry.cpp`](../../engine/tests/test_native_observable_registry.cpp) | 56 | @file test_native_observable_registry.cpp @brief Seed observable-registry contract test. |
| [`test_constructor_contract.cpp`](../../engine/tests/test_constructor_contract.cpp) | 33 | @file test_constructor_contract.cpp @brief Constructor-domain metadata helper smoke test. |
| [`phase_i_green_fixtures.h`](../../engine/tests/phase_i_green_fixtures.h) | 24 | Auto-generated by scripts/proofs/generate_phase_i_lattice_green_fixtures.py G18 Poisson Green's function values at selected (L, r) pairs along x-axis. |

### `include`  (99 files, 17,424 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`render_bridge.h`](../../engine/include/ftd/render_bridge.h) | 556 | Render-Bridge Tick Engine Implements the Scale-0 lattice tick dynamics with staged read/write loops and toggle-gated extension phases. |
| [`cosmic_engine.h`](../../engine/include/ftd/cosmic_engine.h) | 482 | CosmicEngine: Scale 5 simulation N-body + SPH cosmic simulation with Barnes-Hut octree gravity. |
| [`coupling_measurement.h`](../../engine/include/ftd/eft/coupling_measurement.h) | 440 | @file ftd/eft/coupling_measurement.h @brief Lattice-coupling measurement for the EFT Recovery Program (Phase 2B). |
| [`constants.h`](../../engine/include/ftd/constants.h) | 423 | FTD Render-Bridge Constants Engine-facing interface to the ontic derivation chain. |
| [`atom_engine.h`](../../engine/include/ftd/atom_engine.h) | 419 | AtomEngine: Scale 2 simulation Atoms as composite objects with inter-atomic forces: Ionic: F = -ALPHA * Q_i * Q_j / (4*pi * r²_soft) * r_hat Van der Waals: Lennard-Jones 12-6 with eps/sigma from on... |
| [`matched_poisson.h`](../../engine/include/ftd/eft/matched_poisson.h) | 413 | @file ftd/eft/matched_poisson.h @brief Matched-stencil conjugate-gradient Poisson solver for EFT measurements. |
| [`engine_state.h`](../../engine/include/ftd/engine_state.h) | 408 | @file engine_state.h @brief Cache-friendly simulation storage anchored by authoritative ternary state. |
| [`term_toggles.h`](../../engine/include/ftd/term_toggles.h) | 342 | Runtime toggles for the logic-first engine. |
| [`particle_engine.h`](../../engine/include/ftd/particle_engine.h) | 340 | ParticleEngine: Scale 1 simulation Phase 7: Lattice-free engine with continuous positions and analytical forces. |
| [`cognitive_lattice.h`](../../engine/include/ftd/cognition/cognitive_lattice.h) | 338 | @file cognitive_lattice.h @brief Ternary cognitive sidecar for LLM memory and retrieval routing. |
| [`constructors.h`](../../engine/include/ftd/constructors.h) | 336 | ftd/constructors.h — lattice constructor library Named factory functions that stamp FTD theoretical entities onto a RenderBridge's voxel grid. |
| [`branch_holonomy.h`](../../engine/include/ftd/branch_holonomy.h) | 326 | branch_holonomy.h — signed difference operator and Z_2 torus branch twists. |
| [`cluster_tracker.h`](../../engine/include/ftd/cluster_tracker.h) | 315 | Cluster Tracker — Connected-component cluster identification + persistence. |
| [`correlations.h`](../../engine/include/ftd/correlations.h) | 313 | Correlation Function Infrastructure Physics justification: Correlation functions are the fundamental observables of field theories. |
| [`atomic_closure_context.h`](../../engine/include/ftd/atomic_closure_context.h) | 307 | Atomic closure-context diagnostics. |
| [`cluster_genealogy.h`](../../engine/include/ftd/cluster_genealogy.h) | 282 | Cluster genealogy tracker — merge/split (fusion/fission) event detection. |
| [`field_operators.h`](../../engine/include/ftd/field_operators.h) | 271 | Field operators — discrete differential operators on the lattice. |
| [`parallel.h`](../../engine/include/ftd/parallel.h) | 267 | ============================================================================ ftd/parallel.h — unified parallel primitives with THREE compile-time backends ==========================================... |
| [`gauge_couplings.h`](../../engine/include/ftd/ontic/gauge_couplings.h) | 265 | ontic/gauge_couplings.h — Layers 5, 5b, 7 and simulation parameters. |
| [`anisotropy.h`](../../engine/include/ftd/eft/anisotropy.h) | 263 | @file ftd/eft/anisotropy.h @brief Rotational-anisotropy measurement for the EFT Recovery Program (Phase 1A). |
| [`generation_graph.h`](../../engine/include/ftd/generation_graph.h) | 253 | generation_graph.h — Γ_F(d) triangle graph + 3x3 Hermitian eigensolver. |
| [`master_quadratic.h`](../../engine/include/ftd/ontic/master_quadratic.h) | 249 | ontic/master_quadratic.h — Layers 3, 3b, 3c, 4, 4b of the ontic chain. |
| [`manifestation_background.h`](../../engine/include/ftd/eft/manifestation_background.h) | 248 | @file ftd/eft/manifestation_background.h @brief Forced Poisson manifestation-injection background (Plan B, P2 protocol). |
| [`render_bridge_diagnostics.h`](../../engine/include/ftd/render_bridge_diagnostics.h) | 233 | @file engine/include/ftd/render_bridge_diagnostics.h @purpose POD diagnostic structs returned by RenderBridge inspection methods. |
| [`voxel.h`](../../engine/include/ftd/voxel.h) | 225 | Per-node state for the FTD render-bridge simulation. |
| [`lagrangian.h`](../../engine/include/ftd/lagrangian.h) | 222 | Complete Discrete FTD Lagrangian (6 active terms + Rayleigh dissipation) L_FTD = L_KINETIC + L_GRADIENT + L_BI + L_COUPLING + L_VELOCITY + L_GAUSS Field sector: L_field = ½\|Δ_t J\|² - ½c²Σ_μ w_μ\|ΔJ_... |
| [`lemniscate.h`](../../engine/include/ftd/ontic/lemniscate.h) | 221 | ontic/lemniscate.h — Layers -1 through 2b of the ontic chain. |
| [`color_center.h`](../../engine/include/ftd/color_center.h) | 218 | color_center.h — Z_3 color-center charges and the center projector. |
| [`reaction_operators.h`](../../engine/include/ftd/eft/reaction_operators.h) | 215 | @file ftd/eft/reaction_operators.h @brief Reaction-sector operators (O7-O10) for the S_eff campaign (FTD-0112). |
| [`lorentz_recovery.h`](../../engine/include/ftd/eft/lorentz_recovery.h) | 210 | @file ftd/eft/lorentz_recovery.h @brief Lorentz-covariance recovery diagnostic (EFT Recovery Program, Phase 1B). |
| [`ward_identities.h`](../../engine/include/ftd/eft/ward_identities.h) | 209 | @file ftd/eft/ward_identities.h @brief Ward-identity diagnostics for the EFT Recovery Program (Phase 1C). |
| [`hilbert.h`](../../engine/include/ftd/hilbert.h) | 209 | Hilbert Space Construction from Complexified Flux H_FTD = L^2(Lattice, C) where psi(v) = J_x(v) + i*J_y(v) The complexified transverse flux components form a wave function. |
| [`operator_spectrum.h`](../../engine/include/ftd/eft/operator_spectrum.h) | 208 | @file ftd/eft/operator_spectrum.h @brief Operator-basis and scaling-dimension extraction (EFT Phase 3). |
| [`wilson_dirac.h`](../../engine/include/ftd/wilson_dirac.h) | 206 | Wilson-Dirac Matter Sector for FTD (Phase II.2 of the campaign) Pre-registration: docs/theory/10_eft_program/PREREG_PHASE_II_WILSON_DIRAC_G2.md (tag: preregister-phase-ii-wilson-dirac-g2-v1) Specif... |
| [`gpu_buffers.h`](../../engine/include/ftd/gpu_buffers.h) | 205 | SoA (Structure-of-Arrays) device buffers for GPU-accelerated FTD engine. |
| [`spectrum_extraction.h`](../../engine/include/ftd/spectrum_extraction.h) | 202 | Two-state spectrum extraction from a one-dimensional correlator C(τ). |
| [`ensemble.h`](../../engine/include/ftd/ensemble.h) | 200 | Ensemble Runner — Statistical mechanics over independent RenderBridge runs. |
| [`knot_telemetry.h`](../../engine/include/ftd/knot_telemetry.h) | 197 | engine/include/ftd/knot_telemetry.h |
| [`tracker.h`](../../engine/include/ftd/tracker.h) | 197 | Particle Tracker — Trajectory recording using persistent particle_id. |
| [`spectral.h`](../../engine/include/ftd/spectral.h) | 195 | Spectral Analysis — FFT-based dispersion relation measurement. |
| [`sublattice.h`](../../engine/include/ftd/sublattice.h) | 193 | Sublattice projection — SC / FCC / BCC sub-stencils of the Moore-26 neighborhood. |
| [`lattice.h`](../../engine/include/ftd/lattice.h) | 189 | 3D Cubic Lattice with periodic boundary conditions. |
| [`barnes_hut.h`](../../engine/include/ftd/barnes_hut.h) | 184 | Universal Barnes-Hut Octree Implements an O(N log N) spatial partitioner that accurately preserves long-range interactions (1/r^2) via monopole summation (gravity, Coulomb). |
| [`trit.h`](../../engine/include/tritium/trit.h) | 175 | tritium/trit.h — Core ternary types for balanced ternary {-1, 0, +1} Two representations: TritWord64 — compute format: 32 trits in uint64_t (2 bits each) TritPack — storage format: 5 trits in uint8... |
| [`arithmetic.h`](../../engine/include/tritium/arithmetic.h) | 172 | tritium/arithmetic.h — Balanced ternary arithmetic on packed TritWord64 All operations process 32 trits in parallel via bitwise ops on uint64_t. |
| [`simd.h`](../../engine/include/tritium/simd.h) | 171 | tritium/simd.h — SIMD-accelerated operations for packed trit words Provides optimized implementations using SSE4.2/AVX2 intrinsics with scalar fallbacks for portability. |
| [`a1g_projector.h`](../../engine/include/ftd/a1g_projector.h) | 168 | A_{1g} projector for the 27-voxel Moore block. |
| [`dag_lattice.h`](../../engine/include/ftd/dag_lattice.h) | 168 | SparseVoxelDAG A Directed Acyclic Graph that compresses identical octree nodes. |
| [`render_bridge_phases.h`](../../engine/include/ftd/render_bridge_phases.h) | 165 | @file engine/include/ftd/render_bridge_phases.h @purpose Free-function declarations for the decomposed phase methods. |
| [`trit_vector.h`](../../engine/include/tritium/trit_vector.h) | 158 | tritium/trit_vector.h — Dynamic-length packed trit arrays with fast operations Backed by TritWord64 (compute format). |
| [`dag_engine.h`](../../engine/include/ftd/dag_engine.h) | 154 | @brief DagEngine — EXPERIMENTAL / DEPRECATED sparse-voxel-DAG prototype. |
| [`test_telemetry.h`](../../engine/include/ftd/test_telemetry.h) | 154 | ============================================================================ ftd/test_telemetry.h — NDJSON telemetry for the FTD Test Bench runner ==================================================... |
| [`blocking.h`](../../engine/include/ftd/eft/blocking.h) | 153 | @file ftd/eft/blocking.h @brief Real-space block-spin transformation (EFT Recovery Program, Phase 2A). |
| [`engine_select.h`](../../engine/include/ftd/engine_select.h) | 148 | Engine selection helper. |
| [`gpu_engine.h`](../../engine/include/ftd/gpu_engine.h) | 145 | GPU-Accelerated FTD Engine Drop-in alternative to RenderBridge that executes the tick cycle on NVIDIA GPU via CUDA. |
| [`gauss_projection_ext.h`](../../engine/include/ftd/eft/gauss_projection_ext.h) | 142 | @file ftd/eft/gauss_projection_ext.h @brief High-tolerance Gauss projection for EFT measurements (post-campaign). |
| [`scale_context.h`](../../engine/include/ftd/scale_context.h) | 141 | @file engine/include/ftd/scale_context.h @purpose Read-only "scale-context readout admissibility gate" (C_scale). |
| [`ws_sha1.h`](../../engine/include/ftd/ws_sha1.h) | 138 | Minimal SHA-1 (RFC 3174) Header-only, stdlib-only. |
| [`packing.h`](../../engine/include/tritium/packing.h) | 137 | tritium/packing.h — Conversion between compute (TritWord64) and storage (TritPack) formats Uses precomputed lookup tables for zero-branch encode/decode. |
| [`gpu_particle_engine.h`](../../engine/include/ftd/gpu_particle_engine.h) | 123 | GPU-accelerated ParticleEngine backend (Wave 5.4 Phase 1). |
| [`backend.h`](../../engine/include/ftd/backend.h) | 120 | @file backend.h @brief Backend abstraction — collapses #ifdef FTD_ENABLE_CUDA proliferation. |
| [`gpu_atom_engine.h`](../../engine/include/ftd/gpu_atom_engine.h) | 120 | GPU-accelerated AtomEngine backend (Wave 5.3 Phase 1). |
| [`csv_export.h`](../../engine/include/ftd/csv_export.h) | 115 | CSV Data Export Utility for FTD Simulations Utility for exporting simulation data to CSV files. |
| [`voxel_rng.h`](../../engine/include/ftd/voxel_rng.h) | 114 | FTD voxel-level RNG (BH-F5 / BH-F8 / BH-F9 closure, 2026-05-05). |
| [`convolution.h`](../../engine/include/tritium/convolution.h) | 114 | tritium/convolution.h — 1D/2D/3D ternary convolution with trit kernels Convolution with trit-valued kernels produces integer outputs. |
| [`logic.h`](../../engine/include/tritium/logic.h) | 112 | tritium/logic.h — Kleene strong three-valued logic on packed TritWord64 Maps balanced ternary to truth values: Neg = False, Zero = Unknown, Pos = True Logic ops: NOT, AND (min), OR (max), CONSENSUS... |
| [`particle_masses.h`](../../engine/include/ftd/ontic/particle_masses.h) | 110 | ontic/particle_masses.h — Layers 6, 6b, 6c of the ontic chain. |
| [`scale_engine.h`](../../engine/include/ftd/scale_engine.h) | 110 | ScaleEngine: Abstract base class for all FTD per-scale simulation engines. |
| [`ws_protocol.h`](../../engine/include/ftd/ws_protocol.h) | 100 | WebSocket framing protocol (RFC 6455) + minimal string-search JSON helpers. |
| [`observable_registry.h`](../../engine/include/ftd/observable_registry.h) | 99 | @file observable_registry.h @brief Seed registry for constructor-domain observable maps. |
| [`consciousness.h`](../../engine/include/ftd/ontic/consciousness.h) | 98 | ontic/reference frame context.h — Layers 8 and 8b of the ontic chain. |
| [`threshold.h`](../../engine/include/tritium/threshold.h) | 90 | tritium/threshold.h — Convert continuous values to/from balanced ternary Hard quantization (deterministic) and stochastic quantization (FTD Born rule). |
| [`dual_cell_continuity.h`](../../engine/include/ftd/eft/dual_cell_continuity.h) | 88 | @file ftd/eft/dual_cell_continuity.h @brief Finite-volume reaction/transport continuity and b=2 blocking. |
| [`trit_matrix.h`](../../engine/include/tritium/trit_matrix.h) | 87 | tritium/trit_matrix.h — Row-major trit matrix with mat-vec and mat-mat operations Each row is a TritVector. |
| [`scale.h`](../../engine/include/ftd/scale.h) | 83 | Multi-Scale Physics: OnticEntity and Scale definitions Phase 7: The universal ternary triple {state, energy, boundary} recurs at every scale of reality. |
| [`vtk_export.h`](../../engine/include/ftd/vtk_export.h) | 80 | Native ParaView/VTK research export for RenderBridge snapshots. |
| [`gpu_dual_cell_fields.cuh`](../../engine/include/ftd/eft/gpu_dual_cell_fields.cuh) | 78 | @file ftd/eft/gpu_dual_cell_fields.cuh @brief Device-side data structures for GPU-native EFT calculations. |
| [`dual_cell_blocking.h`](../../engine/include/ftd/eft/dual_cell_blocking.h) | 77 | @file ftd/eft/dual_cell_blocking.h @brief Native finite-volume source/flux fields and b=2 blocking. |
| [`cluster_observables.h`](../../engine/include/ftd/cluster_observables.h) | 74 | Per-cluster observables for the cluster-thermodynamics EXPLORATORY campaign. |
| [`neutrino.h`](../../engine/include/ftd/ontic/neutrino.h) | 70 | ontic/neutrino.h — Layer 7b: Absolute Neutrino Masses (Seesaw). |
| [`scenarios.h`](../../engine/include/ftd/scenarios.h) | 70 | ========================================================================== engine/include/ftd/scenarios.h C++ port of the Scale-0 scenario library that was previously JS-only on the MockBridge (eng... |
| [`poisson_solvers.h`](../../engine/include/ftd/poisson_solvers.h) | 69 | Poisson solvers — SOR sweep + top-level solvers. |
| [`dual_cell_flow.h`](../../engine/include/ftd/eft/dual_cell_flow.h) | 67 | @file ftd/eft/dual_cell_flow.h @brief Bare native-flow measurements for finite-volume dual-cell fields. |
| [`lattice_coulomb_gate.h`](../../engine/include/ftd/eft/lattice_coulomb_gate.h) | 67 | @file ftd/eft/lattice_coulomb_gate.h @brief Phase-G lattice Coulomb gate paired with energy_audit conventions. |
| [`bridge_rng.h`](../../engine/include/ftd/bridge_rng.h) | 62 | @file bridge_rng.h @brief PIMPL'd RNG state for RenderBridge. |
| [`injector.h`](../../engine/include/ftd/injector.h) | 61 | @file injector.h @brief Injector — owns particle-ID and pair-ID counters. |
| [`gauge_field.h`](../../engine/include/ftd/gauge_field.h) | 54 | @file engine/include/ftd/gauge_field.h @purpose Declarations of edge-based SU(2) and SU(3) link variable structures for non-Abelian gauge field simulations (Scale 0 upgrades). |
| [`ontic.h`](../../engine/include/ftd/ontic.h) | 48 | The Ontic Derivation Chain — umbrella header. |
| [`gpu_discrete_universe.h`](../../engine/include/ftd/eft/gpu_discrete_universe.h) | 45 | @file ftd/eft/gpu_discrete_universe.h @brief Standalone, hyper-optimized GPU Discrete Universe simulation engine prototype. |
| [`constants_gpu.cuh`](../../engine/include/ftd/constants_gpu.cuh) | 44 | Shared physics constants — included by both constants.h and CUDA kernels. |
| [`scale_ratio.h`](../../engine/include/ftd/scale_ratio.h) | 44 | engine/include/ftd/scale_ratio.h This module implements FC-3 (SPEC_SCALE_RATIO_ONTOLOGY.md §6). |
| [`test_telemetry_snapshot.h`](../../engine/include/ftd/test_telemetry_snapshot.h) | 44 | @file engine/include/ftd/test_telemetry_snapshot.h @purpose RenderBridge-aware lattice snapshot encoder for FTD Test Bench NDJSON. |
| [`cli_demos.h`](../../engine/include/ftd/cli_demos.h) | 39 | CLI demo scenarios extracted from main.cpp. |
| [`transmutation_phases.h`](../../engine/include/ftd/transmutation_phases.h) | 39 | Transmutation phases — optional, toggle-gated physics. |
| [`wilson_dirac_gpu.h`](../../engine/include/ftd/wilson_dirac_gpu.h) | 31 | Wilson-Dirac GPU host-side API (Phase II.2-E). |
| [`injection.h`](../../engine/include/ftd/injection.h) | 28 | Injection — state-mutating primitives for seeding the lattice. |
| [`diagnostics_compute.h`](../../engine/include/ftd/diagnostics_compute.h) | 26 | Diagnostics — read-only reductions over voxel state. |
| [`tritium.h`](../../engine/include/tritium/tritium.h) | 24 | tritium.h — Single-include header for the Tritium ternary compute library Tritium provides efficient balanced ternary {-1, 0, +1} computation: - Hybrid storage: 2-bit compute format (32 trits/uint6... |
| [`energy_ledger_compute.h`](../../engine/include/ftd/energy_ledger_compute.h) | 17 | Energy ledger computation — moved out of render_bridge.cpp in the 2026-04-18 R3 refactor. |

### `web/js-toplevel`  (40 files, 14,673 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`app.js`](../../engine/web/js/app.js) | 1732 | @file app.js @brief FTD Web Dashboard — Main Application Controller [EXTENDED] Initializes all subsystems, manages the frame loop, and wires up UI controls to the simulation bridge. |
| [`fieldlines.js`](../../engine/web/js/fieldlines.js) | 979 | ── fieldlines.js ── Streamline computation for 3D field visualization ── RK4 integration through sampled vector fields. |
| [`telemetry-hub.js`](../../engine/web/js/telemetry-hub.js) | 927 | TelemetryHub — single source of truth for all FTD simulation telemetry. |
| [`particle-catalog.js`](../../engine/web/js/particle-catalog.js) | 696 | Particle Catalog: Standard Model particles with FTD context. |
| [`cosmic-renderer.js`](../../engine/web/js/cosmic-renderer.js) | 684 | CosmicRenderer: Scale 5 — cinematic deep-space rendering Design goals: - Stars: soft glow sprites with diffraction cross, blackbody color, size ~ luminosity - Gas: large volumetric nebula sprites,... |
| [`ws-bridge.js`](../../engine/web/js/ws-bridge.js) | 642 | WebSocket Bridge — connects web dashboard to native GPU engine. |
| [`meta-unit.js`](../../engine/web/js/meta-unit.js) | 635 | ── meta-unit.js ── 3x3x3 existential unit lattice visualization ── Renders the 27-site Moore neighborhood as togglable geometric layers: shells (center, octahedron, cuboctahedron, cube), wireframe... |
| [`molecules.js`](../../engine/web/js/molecules.js) | 605 | Molecular Library — 25 molecules for Scale 2 (AtomEngine). |
| [`constants.js`](../../engine/web/js/constants.js) | 569 | @file constants.js @brief FTD Constants — single source of truth for the web dashboard. |
| [`physics-harness.js`](../../engine/web/js/physics/physics-harness.js) | 500 | PhysicsHarness — single canonical surface for reading and writing Scale-0 lattice physics state. |
| [`pe-telemetry.js`](../../engine/web/js/pe-telemetry.js) | 499 | PE Telemetry Panel — non-duplicated Scale 1 drill-down surfaces. |
| [`planetary-renderer.js`](../../engine/web/js/planetary-renderer.js) | 432 | F-7: build one of the two shared shader-program templates. |
| [`fields.js`](../../engine/web/js/fields.js) | 428 | Force Field Sampling & Visualization Samples Coulomb/ionic potential and force vectors on a 2D grid (XZ plane) for rendering as a heatmap + arrow overlay. |
| [`inspector.js`](../../engine/web/js/inspector.js) | 410 | Inspector Panel — click-to-inspect particle properties. |
| [`units.js`](../../engine/web/js/units.js) | 386 | FTD Unit Conversion Layer Central module for converting raw simulation values to human-readable strings with proper physical unit labels. |
| [`aggregation-bridge.js`](../../engine/web/js/aggregation-bridge.js) | 371 | Aggregation Bridge Module — Appendix A of Steinmetz (2026). |
| [`meta-pedagogy.js`](../../engine/web/js/meta-pedagogy.js) | 356 | Meta Pedagogy — Interactive exploration of the 3³ Existential Unit. |
| [`lattice-synth.js`](../../engine/web/js/audio/lattice-synth.js) | 353 | @file engine/web/js/audio/lattice-synth.js @purpose Connects FTD wave telemetry into the Web Audio API to hear the lattice. |
| [`ontic-observatory.js`](../../engine/web/js/ontic-observatory.js) | 334 | Ontic Observatory — Makes the Ontic Incompleteness narrative visible. |
| [`orbitals.js`](../../engine/web/js/orbitals.js) | 325 | Electron Orbital Cloud Generator + Nuclear Structure VISUALIZATION ONLY (not a physics derivation — FTD-0270): generates electron probability clouds by rejection-sampling HYDROGENIC wavefunctions (... |
| [`decay-rates.js`](../../engine/web/js/decay-rates.js) | 267 | Decay Rates Module — particle lifetimes computed from FTD constants. |
| [`quantum-chemistry.js`](../../engine/web/js/orbitals/quantum-chemistry.js) | 247 | quantum-chemistry.js — pure QM helpers extracted from orbitals.js Aufbau filling, configuration exceptions, Slater's shielding rules, real spherical harmonic angular probabilities, and rejection-sa... |
| [`cross-sections.js`](../../engine/web/js/cross-sections.js) | 229 | Cross-Sections Module — scattering cross-sections (standard QED forms). |
| [`atomic-props.js`](../../engine/web/js/atomic-props.js) | 228 | K |
| [`lifecycle.js`](../../engine/web/js/lifecycle.js) | 207 | Unified Lifecycle Controller for FTD Web Frontend ──────────────────────────────────────────────────────────────────── Base class providing robust, automated resource reclamation. |
| [`atomic-energy.js`](../../engine/web/js/atomic-energy.js) | 201 | Atomic Energy Calculator Computes physical atomic energies for all 118 elements: - Nuclear binding energy (Bethe-Weizsäcker semi-empirical mass formula) - Total rest mass energy (protons + neutrons... |
| [`charts.js`](../../engine/web/js/charts.js) | 189 | Canvas 2D Time-Series Charts — ring-buffered, auto-scaling. |
| [`backgrounds.js`](../../engine/web/js/backgrounds.js) | 178 | FTD Environment Backgrounds — registry + BackgroundManager. |
| [`elements.js`](../../engine/web/js/elements.js) | 175 | Periodic Table Data — all 118 elements. |
| [`spectroscopy.js`](../../engine/web/js/spectroscopy.js) | 155 | Spectroscopy Module — Hydrogen energy levels and spectral series. |
| [`meta-unit-geometry.js`](../../engine/web/js/meta-unit-geometry.js) | 134 | ── meta-unit-geometry.js ── Pure geometry helpers for MetaUnit ── Extracted from meta-unit.js: sphere/wireframe/axis/mirror factories and edge-finding utilities. |
| [`zoo.js`](../../engine/web/js/zoo.js) | 129 | Particle Zoo — interactive table of all SM particles with FTD data. |
| [`shaders.js`](../../engine/web/js/cosmic/shaders.js) | 114 | cosmic/shaders.js — GLSL shader sources + blackbody color helper. |
| [`nuclear-cloud.js`](../../engine/web/js/orbitals/nuclear-cloud.js) | 87 | nuclear-cloud.js — nuclear structure point cloud generation. |
| [`sprites.js`](../../engine/web/js/cosmic/sprites.js) | 77 | cosmic/sprites.js — procedural canvas-texture factories for CosmicRenderer. |
| [`keyboard.js`](../../engine/web/js/app-wire/keyboard.js) | 68 | app-wire/keyboard.js — keyboard-shortcut handler for the FTD dashboard. |
| [`dom-utils.js`](../../engine/web/js/dom-utils.js) | 57 | DOM Utilities — shared helpers for DOM access patterns that appear in multiple unrelated modules (diagnostics, pe-telemetry, charts, ...). |
| [`index.js`](../../engine/web/js/physics/index.js) | 39 | Physics module entry point. |
| [`bridge-init.js`](../../engine/web/js/bridge-init.js) | 23 | @file engine/web/js/bridge-init.js @purpose Bridge barrel + capability-getter installer. |
| [`diagnostics.js`](../../engine/web/js/diagnostics.js) | 6 | @deprecated Legacy diagnostics panel removed (2026-06-13 telemetry cleanup). |

### `web/scale0`  (57 files, 13,928 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`field-overlays.js`](../../engine/web/js/scales/scale0/runtime/field-overlays.js) | 1102 | @deprecated Prefer createFieldSampleCache + per-job ensureSample. |
| [`flux-slice-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/flux-slice-panel.js) | 1078 | Scale 0 — Live Multi-Field Flux Slice Panel Mirrors the visualization panel's enabled fields (FIELDS column + \|J\|), rendering each enabled field as a row of three 2D heatmaps at the lattice mid-pla... |
| [`scenario-registry.js`](../../engine/web/js/scales/scale0/scenario-registry.js) | 1006 | Scenario: empty (Empty Lattice) Physical purpose: Serves as the baseline state of the lattice with no initial particles or fields. |
| [`field-line-knots.js`](../../engine/web/js/scales/scale0/runtime/field-line-knots.js) | 655 | engine/web/js/scales/scale0/runtime/field-line-knots.js Field-line KNOT detection + quantification + identity tracking — JS-native. |
| [`scenario-loader.js`](../../engine/web/js/scales/scale0/runtime/scenario-loader.js) | 610 | .js` and // `engine/src/scenarios/*.cpp` MUST only mutate toggle keys that // appear in `SCALE0_TOGGLES`. |
| [`wave-info.js`](../../engine/web/js/scales/scale0/ui/overlays/wave-lab/wave-info.js) | 546 | @file engine/web/js/scales/scale0/ui/overlays/wave-lab/wave-info.js @purpose Live telemetry and controls for standalone RF/light/sound lattice waves. |
| [`knots-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/knots-panel.js) | 526 | _symbols:_ mountKnotsPanel(), initKnotsPanel() |
| [`time-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/time-panel.js) | 524 | Time Observatory — Scale-0 time-dilation instrument. |
| [`wire.js`](../../engine/web/js/scales/scale0/ui/controls/wire.js) | 482 | Scale 0 Controls Panel Wiring Binds event listeners for every control card mounted by Scale0ControlsComponent: - Physics toggles card (all 18 toggles from SCALE0_TOGGLES) - Substrate controls card... |
| [`overlay-frames.js`](../../engine/web/js/scales/scale0/runtime/overlay-frames.js) | 409 | ══════════════════════════════════════════════════════════════════════ Overlay frame builders — one pure function per topology overlay. |
| [`controller.js`](../../engine/web/js/scales/scale0/controller.js) | 362 | Scale 0 (Lattice) Controller Refactored into a package-style module with explicit runtime phases, a viewport adapter, scenario registry, and UI bindings owned by Scale 0. |
| [`spectrum-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/spectrum-panel.js) | 346 | Lattice Spectroscopy — Scale-0 field-structure instrument. |
| [`gravity-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/gravity-panel.js) | 344 | Gravity Observatory — Scale-0 gravity-field instrument. |
| [`helium-spectrum-protocol.js`](../../engine/web/js/scales/scale0/analysis/helium-spectrum-protocol.js) | 294 | Helium lattice-spectrum protocol helpers. |
| [`conservation-micropanel.js`](../../engine/web/js/scales/scale0/ui/overlays/conservation-micropanel.js) | 272 | Conservation-law audit micropanel. |
| [`viewport-adapter.js`](../../engine/web/js/scales/scale0/viewport-adapter.js) | 259 | _symbols:_ createScale0ViewportAdapter() |
| [`store.js`](../../engine/web/js/scales/scale0/state/store.js) | 249 | localStorage may be blocked (privacy mode) — fall through to default |
| [`template.js`](../../engine/web/js/scales/scale0/ui/overlays/template.js) | 248 | Scale 0 Viewport Overlay — Field visualization controls A compact accordion: a filter box + an active-overlays strip on top, then the toggles grouped into collapsible semantic categories (the colla... |
| [`p1-observables-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables-panel.js) | 245 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables-panel.js @purpose Orchestrator for the Scale 0 P1 Observables panel, composing sub-components. |
| [`thermo-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/thermo-panel.js) | 238 | Thermodynamics — docked Scale-0 side panel (FTD-0274). |
| [`scale-context-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/scale-context-panel.js) | 233 | Scale Context — docked Scale-0 side panel (FTD-0306). |
| [`coulomb.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/coulomb.js) | 224 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/coulomb.js @purpose Coulomb V(r) and E-field probe component. |
| [`g2.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/g2.js) | 223 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/g2.js @purpose Lepton g-2 (Schwinger) and live precession component. |
| [`bindings.js`](../../engine/web/js/scales/scale0/ui/bindings.js) | 217 | v=2: Tier 1 quantum overlay bindings added — see SPEC_S0_QUANTUM_OVERLAYS.md |
| [`lattice-spectrum.js`](../../engine/web/js/scales/scale0/analysis/lattice-spectrum.js) | 208 | Lattice spectrum analysis — the spatial energy spectrum E(k) of the flux field. |
| [`_card-helpers.js`](../../engine/web/js/scales/scale0/ui/overlays/_card-helpers.js) | 195 | Shared card-rendering helpers for Scale 0 dock-mode panels. |
| [`dispersion-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/dispersion-panel.js) | 194 | Dispersion — docked Scale-0 side panel (FTD-0298 / FTD-0299). |
| [`genesis-burst-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/genesis-burst-panel.js) | 190 | Genesis-Burst N(A) Law — interactive fire panel + live N(A) plot (FTD-0269). |
| [`gravity-analysis.js`](../../engine/web/js/scales/scale0/analysis/gravity-analysis.js) | 186 | Gravity analysis — pure, DOM-free scalar telemetry for the Gravity Observatory. |
| [`flux-volume.js`](../../engine/web/js/scales/scale0/ui/controls/flux-volume.js) | 174 | Scale 0 Flux Volume Card |
| [`thomson.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/thomson.js) | 158 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/thomson.js @purpose Live readout for the Thomson scattering observatory scenario. |
| [`panel-shell.js`](../../engine/web/js/scales/scale0/ui/overlays/panel-shell.js) | 154 | Scale-0 Visualization panel shell — accordion + active strip + filter. |
| [`field-sample-cache.js`](../../engine/web/js/scales/scale0/runtime/field-sample-cache.js) | 151 | ══════════════════════════════════════════════════════════════════════ Lazy field-sample cache — visual overlay layer only. |
| [`anisotropy.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/anisotropy.js) | 150 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/anisotropy.js @purpose Lattice Anisotropy & SO(2) Recovery component. |
| [`fine-structure.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/fine-structure.js) | 140 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/fine-structure.js @purpose Fine-structure constant instrument panel for flux-recoil scenarios. |
| [`bell.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/bell.js) | 127 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/bell.js @purpose Bell CHSH component. |
| [`dom.js`](../../engine/web/js/scales/scale0/ui/dom.js) | 119 | _symbols:_ getEl(), setButtonActive(), readButtonActive(), setCheckboxValue() |
| [`slice-render.js`](../../engine/web/js/scales/scale0/ui/overlays/slice-render.js) | 119 | Shared 2D-slice rendering helpers for Scale-0 heatmap panels. |
| [`gravity.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/gravity.js) | 114 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/gravity.js @purpose Gravitational time dilation component. |
| [`lattice-topology.js`](../../engine/web/js/scales/scale0/analysis/lattice-topology.js) | 111 | Lattice topology + metric-distribution analysis. |
| [`wave-lab-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/wave-lab-panel.js) | 84 | @file engine/web/js/scales/scale0/ui/overlays/wave-lab-panel.js @purpose Side-panel host for standalone RF/light/sound wave instruments. |
| [`substrate-controls.js`](../../engine/web/js/scales/scale0/ui/controls/substrate-controls.js) | 78 | Scale 0 Substrate Controls Card |
| [`tick.js`](../../engine/web/js/scales/scale0/runtime/tick.js) | 58 | Advance Scale-0 physics by `tickCount` ticks on the active owner only. |
| [`hydrogen.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/hydrogen.js) | 56 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/hydrogen.js @purpose Hydrogen Spectrum component. |
| [`diagnostics.js`](../../engine/web/js/scales/scale0/runtime/diagnostics.js) | 54 | _symbols:_ updateDiagnosticsAndPanels() |
| [`streamline-integrator.js`](../../engine/web/js/scales/scale0/runtime/streamline-integrator.js) | 53 | ══════════════════════════════════════════════════════════════════════ Streamline parameter derivation + particle-buffer seeding helpers shared between the EM and force overlay builders. |
| [`time-analysis.js`](../../engine/web/js/scales/scale0/analysis/time-analysis.js) | 48 | Pure FTD time-dilation math. |
| [`physics-toggles.js`](../../engine/web/js/scales/scale0/ui/controls/physics-toggles.js) | 47 | Scale 0 Physics Toggles Card |
| [`frame-sync.js`](../../engine/web/js/scales/scale0/runtime/frame-sync.js) | 40 | _symbols:_ syncRenderableData() |
| [`component.js`](../../engine/web/js/scales/scale0/ui/controls/component.js) | 39 | Scale 0 Controls Component Mounts all Scale 0 control cards into the controls panel |
| [`template.js`](../../engine/web/js/scales/scale0/ui/toolbar/template.js) | 38 | _symbols:_ getScale0ScenarioToolbarTemplate(), getScale0LatticeSizeToolbarTemplate() |
| [`knot-line-attribution.js`](../../engine/web/js/scales/scale0/runtime/knot-line-attribution.js) | 37 | engine/web/js/scales/scale0/runtime/knot-line-attribution.js Attribute field-line streamline segments to the nearest knot centroid. |
| [`ftd0252-reference.js`](../../engine/web/js/scales/scale0/data/ftd0252-reference.js) | 32 | Measured FTD-0252 kinematic time-dilation data — OFFLINE campaign, NOT live. |
| [`symmetry-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/symmetry-panel.js) | 24 | _symbols:_ SymmetryPanelComponent, mountSymmetryPanel() |
| [`presets.js`](../../engine/web/js/scales/scale0/ui/overlays/presets.js) | 21 | Scale 0 Overlay column groupings. |
| [`register-scale0-ui.js`](../../engine/web/js/scales/scale0/ui/register-scale0-ui.js) | 19 | _symbols:_ registerScale0ToolbarUI() |
| [`component.js`](../../engine/web/js/scales/scale0/ui/toolbar/component.js) | 18 | _symbols:_ createScale0ScenarioToolbarGroup(), createScale0LatticeSizeToolbarGroup() |

### `web/tests`  (69 files, 11,069 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`scales.spec.js`](../../engine/web/tests/scales.spec.js) | 709 | @ts-check |
| [`scale0-substrate-protocol-v2.spec.js`](../../engine/web/tests/scale0-substrate-protocol-v2.spec.js) | 661 | @ts-check |
| [`overlay-scheduler.spec.js`](../../engine/web/tests/overlay-scheduler.spec.js) | 567 | @ts-check |
| [`panel-mount.spec.js`](../../engine/web/tests/panel-mount.spec.js) | 439 | @ts-check |
| [`responsive-overflow.spec.js`](../../engine/web/tests/responsive-overflow.spec.js) | 377 | @ts-check |
| [`perf-baseline.spec.js`](../../engine/web/tests/perf-baseline.spec.js) | 365 | @ts-check |
| [`lifecycle-harness.spec.js`](../../engine/web/tests/lifecycle-harness.spec.js) | 339 | @ts-check |
| [`reconcile-claims.spec.js`](../../engine/web/tests/reconcile-claims.spec.js) | 321 | @ts-check |
| [`audit-regression.spec.js`](../../engine/web/tests/audit-regression.spec.js) | 279 | @ts-check |
| [`scenario-parity.spec.js`](../../engine/web/tests/scenario-parity.spec.js) | 270 | @ts-check |
| [`panels-redesign.spec.js`](../../engine/web/tests/panels-redesign.spec.js) | 256 | @ts-check |
| [`toggle-coverage.spec.js`](../../engine/web/tests/toggle-coverage.spec.js) | 253 | @ts-check |
| [`scale2-atom-overlays.spec.js`](../../engine/web/tests/scale2-atom-overlays.spec.js) | 234 | @ts-check |
| [`scale2-physics.spec.js`](../../engine/web/tests/scale2-physics.spec.js) | 218 | @ts-check |
| [`knots-telemetry.spec.js`](../../engine/web/tests/knots-telemetry.spec.js) | 205 | @ts-check |
| [`_helpers.js`](../../engine/web/tests/_helpers.js) | 194 | @ts-check |
| [`scale0-scenario-telemetry-contract.spec.js`](../../engine/web/tests/scale0-scenario-telemetry-contract.spec.js) | 191 | @ts-check |
| [`scale1-side-panels.spec.js`](../../engine/web/tests/scale1-side-panels.spec.js) | 184 | @ts-check |
| [`flux-slice-axes.spec.js`](../../engine/web/tests/flux-slice-axes.spec.js) | 183 | @ts-check |
| [`scale2-side-panels.spec.js`](../../engine/web/tests/scale2-side-panels.spec.js) | 170 | @ts-check |
| [`scale1-particle-overlays.spec.js`](../../engine/web/tests/scale1-particle-overlays.spec.js) | 169 | @ts-check |
| [`color-ramps.spec.js`](../../engine/web/tests/color-ramps.spec.js) | 166 | @ts-check |
| [`scale0-scenario-health.spec.js`](../../engine/web/tests/scale0-scenario-health.spec.js) | 163 | @ts-check |
| [`take_gallery_screenshots.spec.js`](../../engine/web/tests/take_gallery_screenshots.spec.js) | 159 | @ts-check |
| [`helium-lattice-spectrum.manual.js`](../../engine/web/tests/manual/helium-lattice-spectrum.manual.js) | 157 | @ts-check |
| [`scale0-worker.spec.js`](../../engine/web/tests/scale0-worker.spec.js) | 157 | @ts-check |
| [`animation-clock-freeze.spec.js`](../../engine/web/tests/animation-clock-freeze.spec.js) | 150 | @ts-check |
| [`panel-mount-integration.spec.js`](../../engine/web/tests/panel-mount-integration.spec.js) | 142 | @ts-check |
| [`per-scenario-position-audit.manual.js`](../../engine/web/tests/manual/per-scenario-position-audit.manual.js) | 136 | @ts-check |
| [`scene-panel.spec.js`](../../engine/web/tests/scene-panel.spec.js) | 136 | @ts-check |
| [`playback-smoke.spec.js`](../../engine/web/tests/playback-smoke.spec.js) | 135 | @ts-check |
| [`genesis-burst.spec.js`](../../engine/web/tests/genesis-burst.spec.js) | 131 | @ts-check |
| [`field-line-knots-color.spec.js`](../../engine/web/tests/field-line-knots-color.spec.js) | 129 | engine/web/tests/field-line-knots-color.spec.js Per-knot color (knotHue) + selection API on the field-line knot tracker. |
| [`scale0-panel-wiring.spec.js`](../../engine/web/tests/scale0-panel-wiring.spec.js) | 124 | @ts-check |
| [`scale0-worker-teardown.spec.js`](../../engine/web/tests/scale0-worker-teardown.spec.js) | 123 | @ts-check |
| [`wasm-scenario-coverage.spec.js`](../../engine/web/tests/wasm-scenario-coverage.spec.js) | 120 | @ts-check |
| [`verify_web_consistency.js`](../../engine/web/tests/verify_web_consistency.js) | 119 |  |
| [`field-line-knots-contributions.spec.js`](../../engine/web/tests/field-line-knots-contributions.spec.js) | 118 | engine/web/tests/field-line-knots-contributions.spec.js Per-knot scientific contributions: energy / flux / charge integrated over each knot's region, expressed as a share of the scenario total, + h... |
| [`flux-upload-microbench.spec.js`](../../engine/web/tests/flux-upload-microbench.spec.js) | 113 | @ts-check |
| [`s0-overlay-accordion.spec.js`](../../engine/web/tests/s0-overlay-accordion.spec.js) | 109 | @ts-check |
| [`scale0-toggle-leak.spec.js`](../../engine/web/tests/scale0-toggle-leak.spec.js) | 105 | @ts-check |
| [`scale0-telemetry-gating.spec.js`](../../engine/web/tests/scale0-telemetry-gating.spec.js) | 100 | @ts-check |
| [`scale0-resize-guard.spec.js`](../../engine/web/tests/scale0-resize-guard.spec.js) | 99 | @ts-check |
| [`scale0-time.spec.js`](../../engine/web/tests/scale0-time.spec.js) | 99 | @ts-check |
| [`field-line-knots-identity.spec.js`](../../engine/web/tests/field-line-knots-identity.spec.js) | 98 | engine/web/tests/field-line-knots-identity.spec.js Identity persistence + birth/death/fission/fusion for the field-line knot tracker. |
| [`scale0-zero-point.spec.js`](../../engine/web/tests/scale0-zero-point.spec.js) | 95 | @ts-check |
| [`field-line-knots-detection.spec.js`](../../engine/web/tests/field-line-knots-detection.spec.js) | 88 | engine/web/tests/field-line-knots-detection.spec.js Detection logic for the field-line knot tracker (density + crossings gate). |
| [`helium-spectrum-protocol.spec.js`](../../engine/web/tests/helium-spectrum-protocol.spec.js) | 88 | @ts-check |
| [`scale0-gravity.spec.js`](../../engine/web/tests/scale0-gravity.spec.js) | 88 | @ts-check |
| [`scale0-panel-render.spec.js`](../../engine/web/tests/scale0-panel-render.spec.js) | 83 | @ts-check |
| [`scale0-conservation-panel.spec.js`](../../engine/web/tests/scale0-conservation-panel.spec.js) | 82 | @ts-check |
| [`faq.spec.js`](../../engine/web/tests/faq.spec.js) | 79 | @ts-check |
| [`force-field-samplers.spec.js`](../../engine/web/tests/force-field-samplers.spec.js) | 74 | @ts-check |
| [`scale0-spectrum.spec.js`](../../engine/web/tests/scale0-spectrum.spec.js) | 70 | @ts-check |
| [`debug-conservation.manual.js`](../../engine/web/tests/manual/debug-conservation.manual.js) | 69 |  |
| [`pe-dynamics.node.test.mjs`](../../engine/web/tests/pe-dynamics.node.test.mjs) | 68 | Node unit test: multi-body equilibrium orbit batch seeding. |
| [`scale0-massbody.spec.js`](../../engine/web/tests/scale0-massbody.spec.js) | 63 | @ts-check |
| [`scale0-inject-paused.spec.js`](../../engine/web/tests/scale0-inject-paused.spec.js) | 59 | @ts-check |
| [`scale0-scalecontext.spec.js`](../../engine/web/tests/scale0-scalecontext.spec.js) | 58 | @ts-check |
| [`math-formatting.spec.js`](../../engine/web/tests/math-formatting.spec.js) | 55 | @ts-check |
| [`playwright.config.js`](../../engine/web/tests/playwright.config.js) | 50 | @ts-check |
| [`field-line-knots-attribution-integration.spec.js`](../../engine/web/tests/field-line-knots-attribution-integration.spec.js) | 44 | engine/web/tests/field-line-knots-attribution-integration.spec.js The tracker's per-knot segments/length/legs must equal attributeSegmentsToKnots run against the tracker's OWN detected centroids (g... |
| [`atlas-content.node.test.mjs`](../../engine/web/tests/atlas-content.node.test.mjs) | 33 | Node unit test for the Ontology Atlas content + chain integrity + tag honesty. |
| [`atlas-data.node.test.mjs`](../../engine/web/tests/atlas-data.node.test.mjs) | 33 | Node unit test for the Ontology Atlas static analytic field math. |
| [`scale2-scenario-registry.spec.js`](../../engine/web/tests/scale2-scenario-registry.spec.js) | 33 | @ts-check |
| [`atlas.spec.js`](../../engine/web/tests/atlas.spec.js) | 31 | Smoke + acceptance test for the standalone FTD Ontology Atlas page. |
| [`time-analysis.node.test.mjs`](../../engine/web/tests/time-analysis.node.test.mjs) | 31 | Node unit test for the pure time-dilation math. |
| [`knot-line-attribution.spec.js`](../../engine/web/tests/knot-line-attribution.spec.js) | 15 | engine/web/tests/knot-line-attribution.spec.js |
| [`playwright.manual.config.js`](../../engine/web/tests/playwright.manual.config.js) | 8 | @ts-check |

### `web/ui`  (83 files, 10,839 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`data.js`](../../engine/web/js/ui/components/knowledge-base/data.js) | 1807 | _symbols:_ getKnowledgeBaseSections(), getKnowledgeBaseEntry(), searchKnowledgeBase(), KNOWLEDGE_BASE |
| [`template.js`](../../engine/web/js/ui/components/panel-resources/template.js) | 599 | _symbols:_ getScaleControlsBlocksTemplate(), getZooPanelTemplate(), getInspectorPanelTemplate(), getPhysicsPanelTemplate() |
| [`definitions.js`](../../engine/web/js/ui/components/tooltips/definitions.js) | 513 | _symbols:_ applyUiTooltipDefinitions() |
| [`data.js`](../../engine/web/js/ui/components/faq/data.js) | 441 | FAQ sidebar data — 16 canonical hard problems framed through the FTD lens. |
| [`component.js`](../../engine/web/js/ui/panels/telemetry-grid/component.js) | 354 | _symbols:_ TelemetryGridPanelComponent, initTelemetryGridPanel() |
| [`panel-dock-controller.js`](../../engine/web/js/ui/shell/panel-dock-controller.js) | 308 | _symbols:_ PanelDockController |
| [`table.js`](../../engine/web/js/ui/panels/diagnostics-panel/table.js) | 296 | DiagnosticsTable — renders one section's table + owns per-row cells, reset-scoped running stats, and per-row Sparkline instances. |
| [`app-ontic.js`](../../engine/web/js/ui/app-ontic.js) | 248 | Ontic Observatory / Physics / Hierarchy panel glue. |
| [`component.js`](../../engine/web/js/ui/components/sidebar-library/component.js) | 239 | SidebarLibraryComponent — reusable library-style sidebar. |
| [`component.js`](../../engine/web/js/ui/panels/scene-panel/component.js) | 234 | ScenePanelComponent — wires the Scene panel DOM to the SceneAdapter and manages localStorage-backed persistence. |
| [`component.js`](../../engine/web/js/ui/components/play-bar/component.js) | 229 | PlayBarComponent — the floating transport + speed bar at the bottom of the viewport. |
| [`component.js`](../../engine/web/js/ui/components/floating-window/component.js) | 227 | FloatingWindow — high-performance draggable/resizable glassmorphic panel wrapper. |
| [`component.js`](../../engine/web/js/ui/components/topbar/component.js) | 225 | _symbols:_ TopbarComponent |
| [`app-shell.js`](../../engine/web/js/ui/shell/app-shell.js) | 223 | Shell facade around the current dashboard DOM. |
| [`adapter.js`](../../engine/web/js/ui/panels/scene-panel/adapter.js) | 212 | SceneAdapter — one place to translate "the user moved a slider" into "change this Three.js object on the Viewport". |
| [`uplot-chart.js`](../../engine/web/js/ui/charts/uplot-chart.js) | 207 | UPlotChart — line/area chart primitive. |
| [`component.js`](../../engine/web/js/ui/components/viewport-overlays/component.js) | 195 | Viewport Overlays Component — mounts scale-specific and universal overlay controls Orchestrates: - Scale-specific field/visualization toggles - Universal axes/grid controls - Bottom status-bar scen... |
| [`component.js`](../../engine/web/js/ui/components/tooltips/component.js) | 194 | _symbols:_ TooltipComponent |
| [`mount-toggle.js`](../../engine/web/js/ui/components/panel-dock/mount-toggle.js) | 181 | Updates --viewport-safe-left / --viewport-safe-right on <html> so that any overlay consumers can inset themselves past the sidebar without hardcoding the sidebar width. |
| [`component.js`](../../engine/web/js/ui/panels/lagrangian-panel/component.js) | 155 | Build a small-multiple card for one Lagrangian term. |
| [`component.js`](../../engine/web/js/ui/panels/charts-panel/component.js) | 154 | _symbols:_ ChartsPanelComponent, initChartsPanel() |
| [`stacked-area.js`](../../engine/web/js/ui/charts/stacked-area.js) | 146 | Stacked-area renderer for uPlot. |
| [`component.js`](../../engine/web/js/ui/components/loading-overlay/component.js) | 139 | _symbols:_ LoadingOverlayComponent |
| [`template.js`](../../engine/web/js/ui/panels/scene-panel/template.js) | 139 | Scene panel template — 4 sections of curated render controls. |
| [`component.js`](../../engine/web/js/ui/components/keyboard-help/component.js) | 138 | Keyboard Help Overlay — press `?` to toggle a modal listing every keyboard shortcut the dashboard supports. |
| [`scale0.js`](../../engine/web/js/ui/panels/diagnostics-panel/descriptors/scale0.js) | 136 | Scale 0 diagnostics table descriptor. |
| [`mobile-panel.js`](../../engine/web/js/ui/shell/mobile-panel.js) | 134 | MobilePanelController — touch swipe-to-dismiss and body scroll lock for the bottom-sheet panel on mobile (≤767px). |
| [`diagnostics-template.js`](../../engine/web/js/ui/components/panel-resources/diagnostics-template.js) | 127 | _symbols:_ getDiagnosticsPanelTemplate() |
| [`template.js`](../../engine/web/js/ui/components/settings-modal/template.js) | 109 | _symbols:_ getSettingsModalTemplate() |
| [`template.js`](../../engine/web/js/ui/components/play-bar/template.js) | 103 | Play bar DOM template — a floating control strip at the bottom of the viewport that hosts the primary playback controls. |
| [`scale1.js`](../../engine/web/js/ui/panels/diagnostics-panel/descriptors/scale1.js) | 100 | Scale 1 diagnostics table descriptor. |
| [`chart-fullscreen.js`](../../engine/web/js/ui/charts/chart-fullscreen.js) | 97 | Shared fullscreen portal for chart cards. |
| [`scale2.js`](../../engine/web/js/ui/panels/diagnostics-panel/descriptors/scale2.js) | 95 | Scale 2/3 diagnostics table descriptor (Atom / Molecule Engine). |
| [`canvas-sparkline.js`](../../engine/web/js/ui/charts/canvas-sparkline.js) | 92 | CanvasSparkline — lightweight canvas mini-chart for legacy PE telemetry rows. |
| [`chart-hover-tooltip.js`](../../engine/web/js/ui/charts/chart-hover-tooltip.js) | 90 | _symbols:_ ChartHoverTooltip, formatChartValue(), formatChartSample() |
| [`reader.js`](../../engine/web/js/ui/components/knowledge-base/reader.js) | 83 | KB entry reader — extracted from component.js so the Knowledge Base can share the generic SidebarLibraryComponent shell and only plug in its own reader render function. |
| [`panel-mount-state.js`](../../engine/web/js/ui/shell/panel-mount-state.js) | 81 | Single source of truth for panel-mount state. |
| [`reader.js`](../../engine/web/js/ui/components/faq/reader.js) | 79 | FAQ entry reader — 4 fixed sections with epistemic tag chips on ftdAngle bullets. |
| [`panel-registry.js`](../../engine/web/js/ui/scale-registry/panel-registry.js) | 79 | Shared shell panel registry. |
| [`component.js`](../../engine/web/js/ui/panels/diagnostics-panel/component.js) | 78 | DiagnosticsPanelComponent — composes scale-specific diagnostics tables from descriptors. |
| [`scale1.js`](../../engine/web/js/ui/panels/charts-panel/descriptors/scale1.js) | 76 | Scale 1 charts panel descriptor. |
| [`sparkline.js`](../../engine/web/js/ui/charts/sparkline.js) | 71 | Sparkline — micro uPlot for table Trend cells and chart-chip previews. |
| [`scale0.js`](../../engine/web/js/ui/panels/charts-panel/descriptors/scale0.js) | 71 | Scale 0 charts panel descriptor. |
| [`panel-shell.js`](../../engine/web/js/ui/components/viewport-overlays/panel-shell.js) | 70 | Shared viewport overlay panel shell — scales 1–5 (Scale 0 uses s0-overlay-panel). |
| [`shell-template.js`](../../engine/web/js/ui/shell/shell-template.js) | 69 | Phase 0 template pass: annotate the current DOM with shell regions and create future mount roots without reparenting the existing markup yet. |
| [`scale2.js`](../../engine/web/js/ui/panels/charts-panel/descriptors/scale2.js) | 66 | Scale 2/3 (Atom / Molecule Engine) charts panel descriptor. |
| [`template.js`](../../engine/web/js/ui/components/topbar/template.js) | 65 | _symbols:_ getTopbarInlineTemplate(), getTopbarActionButtons(), getAssistantSidebarTemplate() |
| [`theme.js`](../../engine/web/js/ui/charts/theme.js) | 64 | Chart theme reader — converts CSS custom properties into a uPlot-shaped theme object. |
| [`template.js`](../../engine/web/js/ui/components/sidebar-library/template.js) | 60 | Shared template for library-style sidebars (KB, FAQ, any future ones). |
| [`breakpoint-service.js`](../../engine/web/js/ui/shell/breakpoint-service.js) | 60 | Observes viewport size and emits shell layout snapshots. |
| [`chart-card.js`](../../engine/web/js/ui/panels/charts-panel/chart-card.js) | 55 | ChartCard — wraps a chart descriptor entry in a .chart-card DOM node and owns the UPlotChart lifecycle. |
| [`component.js`](../../engine/web/js/ui/components/panel-resources/component.js) | 54 | _symbols:_ ensurePanelResources() |
| [`term-row.js`](../../engine/web/js/ui/panels/lagrangian-panel/term-row.js) | 52 | TermRow — renders the Lagrangian term-toggle row. |
| [`layout-state.js`](../../engine/web/js/ui/shell/layout-state.js) | 52 | UI shell layout state helpers. |
| [`render.js`](../../engine/web/js/ui/math-format/render.js) | 51 | renderMathInHtml — scan an already-escaped HTML string for LaTeX delimiters (\\( ... |
| [`component.js`](../../engine/web/js/ui/components/panel-dock/component.js) | 45 | _symbols:_ PanelDockComponent |
| [`formatters.js`](../../engine/web/js/ui/panels/diagnostics-panel/formatters.js) | 43 | Value formatters for the diagnostics table. |
| [`toolbar-registry.js`](../../engine/web/js/ui/scale-registry/toolbar-registry.js) | 41 | Registry for toolbar contributions. |
| [`scale0.js`](../../engine/web/js/ui/panels/lagrangian-panel/descriptors/scale0.js) | 38 | Scale 0 Lagrangian panel descriptor. |
| [`component.js`](../../engine/web/js/ui/components/knowledge-base/component.js) | 32 | Knowledge Base — thin factory around SidebarLibraryComponent. |
| [`template.js`](../../engine/web/js/ui/components/workspace-tabs/template.js) | 31 | _symbols:_ getWorkspaceTabsTemplate() |
| [`mount-registry.js`](../../engine/web/js/ui/shell/mount-registry.js) | 30 | Simple registry for shell mount points and named regions. |
| [`component.js`](../../engine/web/js/ui/components/faq/component.js) | 28 | FAQ — thin factory around SidebarLibraryComponent. |
| [`component.js`](../../engine/web/js/ui/components/settings-modal/component.js) | 24 | Settings Modal Component Mounts the settings modal into a container. |
| [`template.js`](../../engine/web/js/ui/components/loading-overlay/template.js) | 23 | _symbols:_ getLoadingOverlayTemplate() |
| [`component.js`](../../engine/web/js/ui/components/viewport-frame/component.js) | 22 | _symbols:_ ViewportFrameComponent |
| [`component.js`](../../engine/web/js/ui/panels/cosmic-info-panel/component.js) | 22 | Cosmic Info Panel Component Wraps #panel-cosmic-info and owns lifecycle for future migration. |
| [`component.js`](../../engine/web/js/ui/panels/inspector-panel/component.js) | 22 | Inspector Panel Component Wraps #panel-inspector and owns lifecycle for future migration. |
| [`component.js`](../../engine/web/js/ui/panels/ontic-panel/component.js) | 22 | Ontic Panel Component Wraps #panel-ontic and owns lifecycle for future migration. |
| [`component.js`](../../engine/web/js/ui/panels/physics-panel/component.js) | 22 | Physics Panel Component Wraps #panel-physics and owns lifecycle for future migration. |
| [`component.js`](../../engine/web/js/ui/panels/planetary-panel/component.js) | 22 | Planetary Panel Component Wraps #panel-planetary and owns lifecycle for future migration. |
| [`component.js`](../../engine/web/js/ui/panels/zoo-panel/component.js) | 22 | Zoo Panel Component Wraps #panel-zoo and owns lifecycle for future migration. |
| [`panel-visibility.js`](../../engine/web/js/ui/panels/panel-visibility.js) | 20 | Shared panel-visibility predicate — SPEC_SCALE0_PERF_TELEMETRY_PANELS §6.4. |
| [`template.js`](../../engine/web/js/ui/components/panel-dock/template.js) | 15 | _symbols:_ getPanelDockShellTemplate() |
| [`component.js`](../../engine/web/js/ui/components/workspace-tabs/component.js) | 15 | _symbols:_ WorkspaceTabsComponent |
| [`register-scale-ui.js`](../../engine/web/js/ui/scale-registry/register-scale-ui.js) | 14 | Shared shell UI registry bundle. |
| [`template.js`](../../engine/web/js/ui/panels/lagrangian-panel/template.js) | 13 | _symbols:_ getLagrangianPanelTemplate() |
| [`register-legacy-toolbar-ui.js`](../../engine/web/js/ui/shell/register-legacy-toolbar-ui.js) | 13 | _symbols:_ registerLegacyToolbarUi() |
| [`overlay-registry.js`](../../engine/web/js/ui/scale-registry/overlay-registry.js) | 12 | Placeholder registry seam for viewport overlays. |
| [`index.js`](../../engine/web/js/ui/panels/index.js) | 11 |  |
| [`template.js`](../../engine/web/js/ui/panels/charts-panel/template.js) | 10 | Charts panel shell — chip picker + grid. |
| [`template.js`](../../engine/web/js/ui/components/viewport-frame/template.js) | 3 | _symbols:_ getViewportFrameTemplate() |
| [`uPlot.iife.min.js`](../../engine/web/js/ui/charts/vendor/uPlot.iife.min.js) | 2 | ! https://github.com/leeoniya/uPlot (v1.6.30) |

### `src/core`  (40 files, 10,818 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`render_bridge.cpp`](../../engine/src/render_bridge.cpp) | 795 | Logic-First FTD Engine (v2.0) Built from axioms: {3D lattice, ternary states, flux field, local causality} Six rules, nothing else: 1. |
| [`particle_engine.cpp`](../../engine/src/particle_engine.cpp) | 669 | ParticleEngine: Scale 1 simulation Phase 7: Lattice-free engine with continuous positions and analytical forces. |
| [`vtk_export.cpp`](../../engine/src/vtk_export.cpp) | 636 |  |
| [`ws_server.cpp`](../../engine/src/ws_server.cpp) | 571 | FTD WebSocket Server Standalone executable that bridges the FTD engine to the web dashboard via WebSocket on port 9100. |
| [`cosmic_engine.cpp`](../../engine/src/cosmic_engine.cpp) | 500 | CosmicEngine: Scale 5 simulation — core TU. |
| [`ontic_audit.cpp`](../../engine/src/ontic_audit.cpp) | 476 |  |
| [`transmutation_phases.cpp`](../../engine/src/transmutation_phases.cpp) | 459 | Transmutation phases — implementation. |
| [`dual_cell_continuity.cpp`](../../engine/src/eft/dual_cell_continuity.cpp) | 396 |  |
| [`wilson_dirac.cpp`](../../engine/src/wilson_dirac.cpp) | 389 | Wilson-Dirac CPU implementation -- Phase II.2-A. |
| [`constructors_bulk_matter.cpp`](../../engine/src/constructors/constructors_bulk_matter.cpp) | 370 | constructors_bulk_matter.cpp Covers source lines 736-1078 of the pre-split constructors.cpp: Level 4 composites — pion, proton, neutron Level 5 atoms/mol — hydrogen, helium, h2_molecule Level 6 gau... |
| [`constructors_atoms.cpp`](../../engine/src/constructors/constructors_atoms.cpp) | 342 | constructors_atoms.cpp Covers source lines 156-496 of the pre-split constructors.cpp: Level 2 field configurations (plane_wave, standing_wave, uniform_e/b, photon_pulse, electric_dipole, magnetic_d... |
| [`atom_engine.cpp`](../../engine/src/atom_engine.cpp) | 339 | AtomEngine: Scale 2 simulation — class lifecycle and tick orchestration. |
| [`cosmic_scenarios.cpp`](../../engine/src/cosmic/cosmic_scenarios.cpp) | 329 | CosmicEngine scenario builders. |
| [`scale_context.cpp`](../../engine/src/scale_context.cpp) | 320 | @file engine/src/scale_context.cpp @purpose Implementation of the read-only scale-context readout admissibility gate (C_scale). |
| [`poisson_solvers.cpp`](../../engine/src/poisson_solvers.cpp) | 319 | Poisson solvers — implementation. |
| [`dag_engine.cpp`](../../engine/src/dag_engine.cpp) | 316 | ══════════════════════════════════════════════════════════════════════ STATUS BANNER — DAG Engine is a DEPRECATED SKELETON (ticket W6), NOT the production physics path. |
| [`injection.cpp`](../../engine/src/injection.cpp) | 314 | Injection — implementation. |
| [`csv_export.cpp`](../../engine/src/csv_export.cpp) | 295 |  |
| [`scale_bridge.cpp`](../../engine/src/scale_bridge.cpp) | 283 | Scale Bridge: coarsen/refine between Scale 0 (voxels) and Scale 1 (particles) Phase 7 Stage 3. |
| [`blocking.cpp`](../../engine/src/eft/blocking.cpp) | 270 | blocking.cpp — Phase 2A of the EFT Recovery Program. |
| [`constructors_molecules.cpp`](../../engine/src/constructors/constructors_molecules.cpp) | 241 | constructors_molecules.cpp Covers source lines 499-733 of the pre-split constructors.cpp: Level 3 elementary particles (electron, positron, neutrino, quark, antiquark). |
| [`ws_protocol.cpp`](../../engine/src/ws_protocol.cpp) | 212 | WebSocket framing protocol implementation. |
| [`constructors_exotic.cpp`](../../engine/src/constructors/constructors_exotic.cpp) | 194 | constructors_exotic.cpp Covers source lines 1081-1245 of the pre-split constructors.cpp: Level 7 gravity/cosmology — schwarzschild, frw_patch, gravitational_wave Level 8 reference frame context — s... |
| [`diagnostics_compute.cpp`](../../engine/src/diagnostics_compute.cpp) | 189 | Diagnostics — implementation. |
| [`backend.cpp`](../../engine/src/backend.cpp) | 175 | @file backend.cpp @brief Backend implementations — CpuBackend + GpuBackend. |
| [`cosmic_sph.cpp`](../../engine/src/cosmic/cosmic_sph.cpp) | 171 | CosmicEngine SPH hydrodynamics. |
| [`lagrangian.cpp`](../../engine/src/lagrangian.cpp) | 166 |  |
| [`dual_cell_blocking.cpp`](../../engine/src/eft/dual_cell_blocking.cpp) | 138 |  |
| [`constructors_core.cpp`](../../engine/src/constructors/constructors_core.cpp) | 119 | constructors_core.cpp Level 0 (flux/particle/wavepacket/entangled_pair) and Level 1A (octahedron/cuboctahedron/stella_octangula/moore_cell) constructors. |
| [`cosmic_barnes_hut.cpp`](../../engine/src/cosmic/cosmic_barnes_hut.cpp) | 103 | CosmicEngine Barnes-Hut octree + gravity. |
| [`cosmic_gravitational_waves.cpp`](../../engine/src/cosmic/cosmic_gravitational_waves.cpp) | 99 | CosmicEngine gravitational wave emission + propagation. |
| [`scenarios.cpp`](../../engine/src/scenarios.cpp) | 89 | ========================================================================== engine/src/scenarios.cpp Thin router + shared RNG for the Scale-0 scenario library. |
| [`_common.h`](../../engine/src/constructors/_common.h) | 88 | Internal shared helpers for the split constructors.cpp translation units. |
| [`cosmic_cosmology.cpp`](../../engine/src/cosmic/cosmic_cosmology.cpp) | 82 | CosmicEngine cosmology: Friedmann / Hubble / dark energy. |
| [`main.cpp`](../../engine/src/main.cpp) | 81 | FTD Render-Bridge Simulation Engine — CLI entry point. |
| [`dual_cell_flow.cpp`](../../engine/src/eft/dual_cell_flow.cpp) | 77 |  |
| [`energy_ledger_compute.cpp`](../../engine/src/energy_ledger_compute.cpp) | 62 | Energy ledger computation — implementation. |
| [`bridge_rng.cpp`](../../engine/src/bridge_rng.cpp) | 61 | @file bridge_rng.cpp @brief PIMPL'd RNG state implementation. |
| [`qcd_one_loop_perturbative.cpp`](../../engine/src/eft/qcd_one_loop_perturbative.cpp) | 52 | qcd_one_loop_perturbative.cpp [IMPOSED] — Imported one-loop QCD running coupling from perturbative QFT. |
| [`add_headers.py`](../../engine/src/add_headers.py) | 31 |  |

### `web/bridge`  (31 files, 9,960 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`mock-atom-engine.js`](../../engine/web/js/bridge/mock-atom-engine.js) | 1284 | Scale-2 Atom Engine (AE) — MockBridge side only. |
| [`wasm-bridge.js`](../../engine/web/js/bridge/wasm-bridge.js) | 920 | @file engine/web/js/bridge/wasm-bridge.js @purpose Thin wrapper around the compiled C++/WASM physics engine (engine/wasm/ftd_wasm.cpp). |
| [`s0-seed-scenarios.js`](../../engine/web/js/bridge/scenarios/s0-seed-scenarios.js) | 829 | S0Seed scenarios — s0-seed-* group. |
| [`mock-particle-engine.js`](../../engine/web/js/bridge/mock-particle-engine.js) | 788 | Scale-1 Particle Engine (PE) — MockBridge side only. |
| [`cosmic-physics.js`](../../engine/web/js/bridge/cosmic-physics.js) | 563 | Cosmic scale-5 force kernel. |
| [`galaxies.js`](../../engine/web/js/bridge/cosmic-scenarios/galaxies.js) | 539 | Cosmic scale-5 scenarios — galaxy-family. |
| [`flux-scenarios.js`](../../engine/web/js/bridge/scenarios/flux-scenarios.js) | 498 | Flux scenarios — flux-* group. |
| [`mock-scale5.js`](../../engine/web/js/bridge/mock-scale5.js) | 448 | CosmicMockBridge — JS-only N-body simulation for cosmic scale (Scale 5). |
| [`spectrum-comparator.js`](../../engine/web/js/bridge/scenarios/spectrum-comparator.js) | 423 | _symbols:_ isSingleWaveScenario(), getWaveScenarioDefaults(), sanitizeWaveScenarioSettings(), getWaveScenarioSettings() |
| [`wasm-bridge-proxy.js`](../../engine/web/js/bridge/wasm-bridge-proxy.js) | 391 | Main-thread proxy for the Scale-0 WASM physics Web Worker. |
| [`cosmic-postupdates.js`](../../engine/web/js/bridge/cosmic-postupdates.js) | 351 | Cosmic scale-5 post-integration updates. |
| [`s0-field-scenarios.js`](../../engine/web/js/bridge/scenarios/s0-field-scenarios.js) | 291 | S0Field scenarios — s0-field-* group. |
| [`mock-scale4.js`](../../engine/web/js/bridge/mock-scale4.js) | 288 | PlanetaryMockBridge — JS-only N-body simulation for Planetary scale (Scale 4). |
| [`pe-force-kernel.js`](../../engine/web/js/bridge/pe-force-kernel.js) | 278 | Scale-1 particle force kernel — mirrors C++ ParticleEngine::compute_pairwise_force and per-particle post-processing (radiation, relativistic correction). |
| [`wasm-bridge.worker.js`](../../engine/web/js/bridge/wasm-bridge.worker.js) | 251 | Scale-0 WASM physics Web Worker. |
| [`exotic.js`](../../engine/web/js/bridge/cosmic-scenarios/exotic.js) | 243 | Cosmic scale-5 scenarios — exotic / lifecycle family. |
| [`quantum-scenarios.js`](../../engine/web/js/bridge/scenarios/quantum-scenarios.js) | 237 | Quantum scenarios — quantum-* group. |
| [`_helpers.js`](../../engine/web/js/bridge/scenarios/_helpers.js) | 218 | Shared scenario primitives — JS mirror of `engine/src/scenarios/_helpers.h`. |
| [`vacuum-scenarios.js`](../../engine/web/js/bridge/scenarios/vacuum-scenarios.js) | 214 | Vacuum Particle Scenarios — s0-vacuum-* group. |
| [`boundary.js`](../../engine/web/js/bridge/boundary.js) | 198 | Boundary shape geometry for the FTD particle engine. |
| [`bridge-contract.js`](../../engine/web/js/bridge/bridge-contract.js) | 118 | Bridge contract — the surface every Scale-0 bridge must implement. |
| [`light-scenarios.js`](../../engine/web/js/bridge/scenarios/light-scenarios.js) | 111 | Light scenarios — light-* group. |
| [`pe-spin-dynamics.js`](../../engine/web/js/bridge/pe-spin-dynamics.js) | 100 | Classical spin precession for Scale-1 PE — mirrors C++ ParticleEngine::evolve_spin_axes. |
| [`index.js`](../../engine/web/js/bridge/scenarios/index.js) | 83 | Scale-0 / Scale-1 / Scale-2 scenario dispatcher — MockBridge side. |
| [`scale0.js`](../../engine/web/js/bridge/capabilities/scale0.js) | 70 | @file engine/web/js/bridge/capabilities/scale0.js @purpose Scale-0 (lattice/substrate) capability factory. |
| [`physics-lattice.js`](../../engine/web/js/bridge/scenarios/physics-lattice.js) | 58 | Fixed-voxel physics extents for Scale-0 scenarios. |
| [`index.js`](../../engine/web/js/bridge/cosmic-scenarios/index.js) | 55 | Cosmic scale-5 scenario dispatcher. |
| [`install.js`](../../engine/web/js/bridge/capabilities/install.js) | 36 | @file engine/web/js/bridge/capabilities/install.js @purpose Installs the lazy `bridge.capabilities` getter on WasmBridge and WebSocketBridge prototypes so consumers see one symmetric surface (CONTR... |
| [`scale1.js`](../../engine/web/js/bridge/capabilities/scale1.js) | 34 | @file engine/web/js/bridge/capabilities/scale1.js @purpose Scale-1 (particle engine) capability factory. |
| [`scale2.js`](../../engine/web/js/bridge/capabilities/scale2.js) | 25 | @file engine/web/js/bridge/capabilities/scale2.js @purpose Scale-2 (atom engine) capability factory. |
| [`bridge-factory.js`](../../engine/web/js/bridge/bridge-factory.js) | 18 | Bridge Factory — creates the WASM simulation bridge. |

### `web/viewport`  (13 files, 8,045 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`field-renderer.js`](../../engine/web/js/viewport/field-renderer.js) | 2807 | @file engine/web/js/viewport/field-renderer.js @purpose Owns ALL field overlays for the Scale-0 lattice dashboard: E/B fields, Poynting, divergence, force volumes (EM/gravity/ strong/weak in 4 styl... |
| [`viewport.js`](../../engine/web/js/viewport.js) | 1257 | @file viewport.js @brief Three.js 3D Viewport — renders particles and fields from the simulation bridge. |
| [`molecular-renderer.js`](../../engine/web/js/viewport/molecular-renderer.js) | 802 | Molecular renderer — Scale 2 (atoms) and Scale 3 (molecules). |
| [`particle-renderer.js`](../../engine/web/js/viewport/particle-renderer.js) | 713 | @file engine/web/js/viewport/particle-renderer.js @purpose Owns particle positions, trails, velocity vectors, per-particle force vectors for the Scale-0 lattice dashboard. |
| [`scene-core.js`](../../engine/web/js/viewport/scene-core.js) | 530 | @file engine/web/js/viewport/scene-core.js @purpose Owns scene-level rendering infrastructure for the Scale-0 dashboard: boundary wireframe, axis indicators, post-processing pipeline (bloom), camer... |
| [`flux-renderer.js`](../../engine/web/js/viewport/flux-renderer.js) | 488 | @file engine/web/js/viewport/flux-renderer.js @purpose Owns flux volume, flux streamlines for the Scale-0 lattice dashboard. |
| [`topology-sheet-renderer.js`](../../engine/web/js/viewport/topology-sheet-renderer.js) | 483 | viewport/topology-sheet-renderer.js — deformable rubber-sheet visualization Extracted from viewport.js as refactoring-analyst ticket RF-1 of the post-modularization cleanup (see engine/web/docs/IND... |
| [`boundary-geometry.js`](../../engine/web/js/viewport/boundary-geometry.js) | 255 | viewport/boundary-geometry.js — Three.js boundary wireframe builders Extracted from viewport.js as refactoring-analyst ticket RF-4 of the post-modularization cleanup (see engine/web/docs/INDEX.md). |
| [`color-ramps.js`](../../engine/web/js/viewport/color-ramps.js) | 237 | Color ramps for Scale 0 viewport overlays. |
| [`spin-arrow-manager.js`](../../engine/web/js/viewport/spin-arrow-manager.js) | 233 | Spin-Arrow Manager — Three.js primitive that follows tracked particles and visualizes their spin orientation + precession rate. |
| [`shaders.js`](../../engine/web/js/viewport/shaders.js) | 149 | Centralized Shaders for FTD Web Frontend ──────────────────────────────────────────────────────────────────── Houses shared GLSL shader strings to ensure DRY compliance and enable global shader opt... |
| [`mesh-factory.js`](../../engine/web/js/viewport/mesh-factory.js) | 61 | @file engine/web/js/viewport/mesh-factory.js @purpose Utility factory functions to build Three.js buffer geometries and line meshes. |
| [`constants.js`](../../engine/web/js/viewport/constants.js) | 30 | @file engine/web/js/viewport/constants.js @purpose Shared pre-allocated-buffer sizes for the viewport renderer cluster. |

### `cuda`  (17 files, 6,951 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`kernels_forces.cu`](../../engine/cuda/kernels_forces.cu) | 1027 | @file kernels_forces.cu @brief GPU kernels for Phase 4 (Forces) and Phase 5 (Movement). |
| [`gpu_buffers.cu`](../../engine/cuda/gpu_buffers.cu) | 770 | SoA device buffer management for FTD GPU engine. |
| [`gpu_engine.cu`](../../engine/cuda/gpu_engine.cu) | 666 | @file gpu_engine.cu @brief GPU-accelerated FTD tick engine. |
| [`kernels_stencil_single.cu`](../../engine/cuda/kernels_stencil_single.cu) | 621 | @file kernels_stencil_single.cu @brief Single-substrate Phase Read / Phase Write kernels (FTD tick cycle). |
| [`kernels_stencil_dual.cu`](../../engine/cuda/kernels_stencil_dual.cu) | 599 | @file kernels_stencil_dual.cu @brief Dual-substrate Phase Read / Phase Write kernels (FTD tick cycle). |
| [`kernels_poisson.cu`](../../engine/cuda/kernels_poisson.cu) | 525 | FFT-based Poisson solver for FTD GPU engine. |
| [`experimental_discrete_universe.cu`](../../engine/cuda/experimental_discrete_universe.cu) | 459 | @file experimental_discrete_universe.cu @brief Standalone, hyper-optimized GPU Discrete Universe simulation engine prototype. |
| [`kernels_gauge.cu`](../../engine/cuda/kernels_gauge.cu) | 459 | @file kernels_gauge.cu @brief GPU kernels for Scale 0 Gauge Field non-Abelian plaquette relaxation. |
| [`kernels_eft.cu`](../../engine/cuda/kernels_eft.cu) | 391 | @file kernels_eft.cu @brief GPU-native EFT calculations: face-flux conversion, blocking, operator evaluation, and parallel reductions. |
| [`atom_engine_gpu.cu`](../../engine/cuda/atom_engine_gpu.cu) | 361 | GPU AtomEngine backend (Wave 5.3 Phase 1). |
| [`particle_engine_gpu.cu`](../../engine/cuda/particle_engine_gpu.cu) | 322 | GPU ParticleEngine backend (Wave 5.4 Phase 1). |
| [`kernels_aux.cu`](../../engine/cuda/kernels_aux.cu) | 293 | @file kernels_aux.cu @brief Auxiliary physics kernels (weak transmutation, pair production). |
| [`wilson_dirac_gpu.cu`](../../engine/cuda/wilson_dirac_gpu.cu) | 183 | Wilson-Dirac GPU kernel (Phase II.2-E). |
| [`kernels_stencil_common.cuh`](../../engine/cuda/kernels_stencil_common.cuh) | 82 | Shared device-side helpers for the stencil kernel TUs. |
| [`cuda_invariants.cu`](../../engine/cuda/cuda_invariants.cu) | 78 | cuda_invariants.cu — Implementation of the CUDA constant-memory invariant pattern declared in cuda_invariants.cuh. |
| [`cuda_invariants.cuh`](../../engine/cuda/cuda_invariants.cuh) | 66 | cuda_invariants.cuh — CUDA constant-memory pattern for small read-only invariants (matrices and companion scalars). |
| [`cuda_index.cuh`](../../engine/cuda/cuda_index.cuh) | 49 | Shared device-side index helpers for CUDA kernels. |

### `tools`  (29 files, 6,578 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`audit_ontic_phase0.py`](../../engine/tools/audit_ontic_phase0.py) | 627 | Phase 0 Ontic Derivation Chain Audit ===================================== Independent verification of every constant in engine/include/ftd/ontic.h using mpmath high-precision arithmetic. |
| [`visualize_sims.py`](../../engine/tools/visualize_sims.py) | 565 | FTD Simulation Visualizer ========================= Reads CSV data exported by ftd_sim scenarios H/I/J and generates publication-quality figures. |
| [`TestModel.cpp`](../../engine/tools/test_runner/src/TestModel.cpp) | 556 | ============================================================================ TestModel.cpp — CTest-driven Category→Test tree model ==================================================================... |
| [`LatticeViewer.cpp`](../../engine/tools/test_runner/src/LatticeViewer.cpp) | 523 | ============================================================================ LatticeViewer.cpp — implementation ============================================================================ |
| [`MainWindow.cpp`](../../engine/tools/test_runner/src/MainWindow.cpp) | 510 | ============================================================================ MainWindow.cpp — top-level window wiring ============================================================================ |
| [`HistoryDb.cpp`](../../engine/tools/test_runner/src/HistoryDb.cpp) | 483 | ============================================================================ HistoryDb.cpp — sqlite-backed run history ============================================================================ |
| [`HistoryTab.cpp`](../../engine/tools/test_runner/src/HistoryTab.cpp) | 450 | ============================================================================ HistoryTab.cpp — "History" tab widget for the FTD Test Bench ===========================================================... |
| [`TelemetryCharts.cpp`](../../engine/tools/test_runner/src/TelemetryCharts.cpp) | 450 | ============================================================================ TelemetryCharts.cpp — live multi-trace scalar telemetry (Qt6 QtCharts) =================================================... |
| [`visualize_engine.py`](../../engine/tools/visualize_engine.py) | 257 | FTD Engine Visualizer — generates figures from CSV scenario outputs. |
| [`TestRunner.cpp`](../../engine/tools/test_runner/src/TestRunner.cpp) | 230 | ============================================================================ TestRunner.cpp — QProcess-based subprocess launcher ====================================================================... |
| [`print_ontic.py`](../../engine/tools/print_ontic.py) | 187 | Print the complete ontic derivation chain to 12 decimal places. |
| [`HistoryDb.h`](../../engine/tools/test_runner/include/HistoryDb.h) | 161 | ============================================================================ HistoryDb.h — SQLite-backed run history for the FTD Test Bench =========================================================... |
| [`OutputPanel.cpp`](../../engine/tools/test_runner/src/OutputPanel.cpp) | 158 | ============================================================================ OutputPanel.cpp — interleaved output view ============================================================================ |
| [`TelemetryCharts.h`](../../engine/tools/test_runner/include/TelemetryCharts.h) | 144 | ============================================================================ TelemetryCharts.h — live multi-trace scalar telemetry (Qt6 QtCharts) ===================================================... |
| [`LatticeViewer.h`](../../engine/tools/test_runner/include/LatticeViewer.h) | 142 | ============================================================================ LatticeViewer.h — live 3D voxel viewer (QOpenGLWidget, OpenGL 3.3 core) ================================================... |
| [`MainWindow.h`](../../engine/tools/test_runner/include/MainWindow.h) | 135 | ============================================================================ MainWindow.h — top-level window for the FTD Test Bench =================================================================... |
| [`TestModel.h`](../../engine/tools/test_runner/include/TestModel.h) | 118 | ============================================================================ TestModel.h — two-level (Category → Test) tree model for FTD Test Bench ================================================... |
| [`SmartDispatcher.cpp`](../../engine/tools/test_runner/src/SmartDispatcher.cpp) | 111 | ============================================================================ SmartDispatcher.cpp — parallel CPU + serial GPU scheduler ==============================================================... |
| [`TestRunner.h`](../../engine/tools/test_runner/include/TestRunner.h) | 100 | ============================================================================ TestRunner.h — QProcess-per-test subprocess launcher for FTD Test Bench ================================================... |
| [`NdjsonParser.cpp`](../../engine/tools/test_runner/src/NdjsonParser.cpp) | 97 | ============================================================================ NdjsonParser.cpp — implementation ============================================================================ |
| [`SmartDispatcher.h`](../../engine/tools/test_runner/include/SmartDispatcher.h) | 90 | ============================================================================ SmartDispatcher.h — parallel CPU + serial GPU test scheduler ===========================================================... |
| [`LatticeViewer_shaders.h`](../../engine/tools/test_runner/src/LatticeViewer_shaders.h) | 87 | ============================================================================ LatticeViewer_shaders.h — inline GLSL 330 core shader sources ==========================================================... |
| [`HistoryTab.h`](../../engine/tools/test_runner/include/HistoryTab.h) | 78 | ============================================================================ HistoryTab.h — "History" tab widget for the FTD Test Bench =============================================================... |
| [`main.cpp`](../../engine/tools/test_runner/src/main.cpp) | 66 | ============================================================================ main.cpp — entry point for the FTD Test Bench runner (ftd_test_runner) =================================================... |
| [`NdjsonParser.h`](../../engine/tools/test_runner/include/NdjsonParser.h) | 65 | ============================================================================ NdjsonParser.h — incremental line-based NDJSON parser for test subprocesses ============================================... |
| [`OutputPanel.h`](../../engine/tools/test_runner/include/OutputPanel.h) | 61 | ============================================================================ OutputPanel.h — interleaved per-test output view for Phase 3 scaffold ==================================================... |
| [`FieldLines.h`](../../engine/tools/test_runner/include/FieldLines.h) | 52 | ============================================================================ FieldLines.h — RK4 vector-field line integrator (stub) =================================================================... |
| [`audit_reexports.py`](../../engine/tools/audit_reexports.py) | 51 | Item 0.21: Verify constants.h re-exports match ontic.h (no stale overrides) |
| [`FieldLines.cpp`](../../engine/tools/test_runner/src/FieldLines.cpp) | 24 | ============================================================================ FieldLines.cpp — RK4 vector-field line integrator (stub) ===============================================================... |

### `archive`  (12 files, 5,597 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`campaign_operator_mixing_2026-04-26.cpp`](../../engine/archive/phase_b_2026-04/campaign_operator_mixing_2026-04-26.cpp) | 1160 | @file campaign_operator_mixing_2026-04-26.cpp @brief FTD-0098: First measured native operator-mixing matrix M_ab(b=2). |
| [`campaign_s_eff_nonlinear_wilson_2026-06-04.cpp`](../../engine/archive/phase_b_2026-04/campaign_s_eff_nonlinear_wilson_2026-06-04.cpp) | 1110 | @file engine/archive/phase_b_2026-04/campaign_s_eff_nonlinear_wilson_2026-06-04.cpp @purpose Archived unregistered nonlinear S_eff campaign variant with Wilson-coefficient CLI hooks. |
| [`campaign_s_eff_nonlinear_2026-04-29.cpp`](../../engine/archive/phase_b_2026-04/campaign_s_eff_nonlinear_2026-04-29.cpp) | 1075 | @file campaign_s_eff_nonlinear_2026-04-29.cpp @brief FTD-0112: Nonlinear S_eff measurement campaign. |
| [`campaign_emergent_spectrum_2026-04-27.cpp`](../../engine/archive/phase_b_2026-04/campaign_emergent_spectrum_2026-04-27.cpp) | 435 | @file campaign_emergent_spectrum_2026-04-27.cpp @brief FTD-0102: Emergent particle spectrum from generic initial conditions. |
| [`test_link8_kadanoff.cpp`](../../engine/archive/link8_closed/tests/test_link8_kadanoff.cpp) | 392 | @file test_link8_kadanoff.cpp @brief Link 8 Candidate 1 — Kadanoff blocking vs master-quadratic recurrence. |
| [`campaign_topological_observables_2026-04-27.cpp`](../../engine/archive/phase_b_2026-04/campaign_topological_observables_2026-04-27.cpp) | 386 | @file campaign_topological_observables_2026-04-27.cpp @brief FTD-0104: Topological observable mapping (engine-native exploration) Pre-registration: docs/theory/10_eft_program/PROTOCOL_TOPOLOGICAL_O... |
| [`test_link8_run3_thermal.cpp`](../../engine/archive/link8_closed/tests/test_link8_run3_thermal.cpp) | 267 | @file test_link8_run3_thermal.cpp @brief Link 8 Candidate 1 — Run 3 redo on Langevin-thermalized ensemble. |
| [`campaign_3d_einstein_gpu_2026-06-04.cu`](../../engine/archive/cuda_exploratory/campaign_3d_einstein_gpu_2026-06-04.cu) | 208 |  |
| [`campaign_wang_extraction_2026-06-04.cpp`](../../engine/archive/exploratory/campaign_wang_extraction_2026-06-04.cpp) | 174 | Archived 2026-06-04 from engine/tests/campaign_wang_extraction.cpp. |
| [`confinement_kernel_2026-06-04.cu`](../../engine/archive/cuda_exploratory/confinement_kernel_2026-06-04.cu) | 172 | FTD Flux field J (Ux, Uy) has curl F = S1 + S2, where S1, S2 are independent ternary topological noise. |
| [`dump_a1g_decay.cpp`](../../engine/archive/dumps_non_load_bearing/dump_a1g_decay.cpp) | 134 | Diagnostic: dump A_{1g} fraction per tick for sub-genesis δ_center IC. |
| [`test_compile_confinement_2026-06-04.cu`](../../engine/archive/cuda_exploratory/test_compile_confinement_2026-06-04.cu) | 84 |  |

### `src/scenarios`  (7 files, 2,813 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`s0_seed.cpp`](../../engine/src/scenarios/s0_seed.cpp) | 1109 | ========================================================================== engine/src/scenarios/s0_seed.cpp Group: s0-seed-* (49 scenarios) JS source: engine/web/js/bridge/scenarios/s0-seed-scenari... |
| [`flux.cpp`](../../engine/src/scenarios/flux.cpp) | 468 | ========================================================================== engine/src/scenarios/flux.cpp Group: flux-* (21 scenarios) JS source: engine/web/js/bridge/scenarios/flux-scenarios.js Spl... |
| [`s0_field.cpp`](../../engine/src/scenarios/s0_field.cpp) | 462 | ========================================================================== engine/src/scenarios/s0_field.cpp Group: s0-field-* (9 scenarios) JS source: engine/web/js/bridge/scenarios/s0-field-scena... |
| [`vacuum.cpp`](../../engine/src/scenarios/vacuum.cpp) | 297 | ========================================================================== engine/src/scenarios/vacuum.cpp Group: s0-vacuum-* (15 scenarios) JS source: engine/web/js/bridge/scenarios/vacuum-scenari... |
| [`quantum.cpp`](../../engine/src/scenarios/quantum.cpp) | 252 | ========================================================================== engine/src/scenarios/quantum.cpp Group: quantum-* (8 scenarios) JS source: engine/web/js/bridge/scenarios/quantum-scenario... |
| [`light.cpp`](../../engine/src/scenarios/light.cpp) | 115 | ========================================================================== engine/src/scenarios/light.cpp Group: light-* (4 scenarios) JS source: engine/web/js/bridge/scenarios/light-scenarios.js S... |
| [`_helpers.h`](../../engine/src/scenarios/_helpers.h) | 110 | ========================================================================== engine/src/scenarios/_helpers.h Private (non-installed) helper header shared by the split scenario group files (flux.cpp,... |

### `wasm`  (5 files, 2,755 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`ftd_wasm.cpp`](../../engine/wasm/ftd_wasm.cpp) | 1500 | @file ftd_wasm.cpp @brief Emscripten Embind bindings for the FTD engine — shared helpers. |
| [`bindings_particle.cpp`](../../engine/wasm/bindings_particle.cpp) | 446 | @file bindings_particle.cpp @brief Embind bindings for ParticleEngine (Scale 1). |
| [`bindings_render_bridge.cpp`](../../engine/wasm/bindings_render_bridge.cpp) | 366 | @file bindings_render_bridge.cpp @brief Embind bindings for RenderBridge (Scale 0 — voxel lattice engine). |
| [`bindings_atom.cpp`](../../engine/wasm/bindings_atom.cpp) | 351 | @file bindings_atom.cpp @brief Embind bindings for AtomEngine (Scale 2). |
| [`bindings_internal.h`](../../engine/wasm/bindings_internal.h) | 92 | @file bindings_internal.h @brief Shared helpers exposed across the split Embind binding TUs. |

### `web/scale2`  (11 files, 1,998 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`controller.js`](../../engine/web/js/scales/scale2/controller.js) | 772 | Scale 2 (Atoms) Controller ──────────────────────────────────────────────────────────────────── Owns the Atom Engine (AE) frame loop, force decomposition rendering, element legend building, orbital... |
| [`scenarios.js`](../../engine/web/js/scales/scale2/scenarios.js) | 522 | Scale 2 — AE Scenario Loader ──────────────────────────────────────────────────────────────────── Extracted verbatim from scales/scale2/controller.js (ticket S2-1). |
| [`scenario-registry.js`](../../engine/web/js/scales/scale2/scenario-registry.js) | 280 | Scale 2 — AE scenario registry (canonical metadata + select population). |
| [`template.js`](../../engine/web/js/scales/scale2/ui/overlays/template.js) | 135 | Scale 2/3 Viewport Overlay — atom/molecule MD + QM structure visualization. |
| [`ae-controls.js`](../../engine/web/js/scales/scale2/ui/controls/ae-controls.js) | 113 | Scale 2 — Atom Engine control cards (split by concern). |
| [`ui-bindings.js`](../../engine/web/js/scales/scale2/ui-bindings.js) | 77 | Scale 2 — AE UI Bindings ──────────────────────────────────────────────────────────────────── Houses the DOM-coupled helpers that sync AE physics parameters and toggles between the Scale 2 control... |
| [`component.js`](../../engine/web/js/scales/scale2/ui/controls/component.js) | 33 | Scale 2 Controls Component Mounts Atom Engine control cards into the controls panel. |
| [`dom.js`](../../engine/web/js/scales/scale2/ui/dom.js) | 28 | Scale 2 — DOM helpers (scenario description strip). |
| [`component.js`](../../engine/web/js/scales/scale2/ui/toolbar/component.js) | 14 | _symbols:_ createScale2ScenarioToolbarGroup() |
| [`template.js`](../../engine/web/js/scales/scale2/ui/toolbar/template.js) | 13 | _symbols:_ getScale2ScenarioToolbarTemplate() |
| [`register-scale2-ui.js`](../../engine/web/js/scales/scale2/ui/register-scale2-ui.js) | 11 | _symbols:_ registerScale2ToolbarUI() |

### `sim`  (16 files, 1,951 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`test_sim_observables.cpp`](../../engine/sim/tests/test_sim_observables.cpp) | 281 | @file test_sim_observables.cpp @brief Phase D — observable-library unit tests (analytical correctness). |
| [`test_sim_parity.cpp`](../../engine/sim/tests/test_sim_parity.cpp) | 254 | @file test_sim_parity.cpp @brief Phase C exit gate — GPU vs CPU parity on 3 reference observables. |
| [`test_sim_pipeline_cpu.cpp`](../../engine/sim/tests/test_sim_pipeline_cpu.cpp) | 250 | @file test_sim_pipeline_cpu.cpp @brief Phase B exit gate — Pipeline<BackendCpu> + 3 reference observables. |
| [`measure_v_of_r.h`](../../engine/sim/include/ftd/sim/measure_v_of_r.h) | 197 | @file ftd/sim/measure_v_of_r.h @brief Two-charge interaction potential V(r) — composite measurement. |
| [`fit_scaling_dimension.h`](../../engine/sim/include/ftd/sim/fit_scaling_dimension.h) | 154 | @file ftd/sim/fit_scaling_dimension.h @brief Power-law fit C(r) ∝ r^(−2Δ) → extract scaling dimension Δ. |
| [`pipeline.h`](../../engine/sim/include/ftd/sim/pipeline.h) | 127 | @file ftd/sim/pipeline.h @brief Pipeline<Backend> — the orchestrator. |
| [`backend_gpu.h`](../../engine/sim/include/ftd/sim/backend_gpu.h) | 120 | @file ftd/sim/backend_gpu.h @brief GPU backend specialisation for Pipeline<Backend>. |
| [`flux_correlator.h`](../../engine/sim/include/ftd/sim/observables/flux_correlator.h) | 89 | @file ftd/sim/observables/flux_correlator.h @brief FluxCorrelator — direction-averaged ⟨J(x)·J(x+r)⟩ over separations r. |
| [`backend_cpu.h`](../../engine/sim/include/ftd/sim/backend_cpu.h) | 82 | @file ftd/sim/backend_cpu.h @brief CPU backend specialisation for Pipeline<Backend>. |
| [`ewsb_condensate_count.h`](../../engine/sim/include/ftd/sim/observables/ewsb_condensate_count.h) | 75 | @file ftd/sim/observables/ewsb_condensate_count.h @brief EwsbCondensateCount — EWSB-threshold diagnostic. |
| [`observable.h`](../../engine/sim/include/ftd/sim/observable.h) | 74 | @file ftd/sim/observable.h @brief Observable base class — the unit of measurement in a pipeline. |
| [`state_histogram.h`](../../engine/sim/include/ftd/sim/observables/state_histogram.h) | 62 | @file ftd/sim/observables/state_histogram.h @brief StateHistogram — counts of state ∈ {−1, 0, +1}. |
| [`total_field_energy.h`](../../engine/sim/include/ftd/sim/observables/total_field_energy.h) | 54 | @file ftd/sim/observables/total_field_energy.h @brief TotalFieldEnergy — sum of ½\|J\|² over all voxels. |
| [`mean_abs_flux.h`](../../engine/sim/include/ftd/sim/observables/mean_abs_flux.h) | 47 | @file ftd/sim/observables/mean_abs_flux.h @brief MeanAbsFlux — average \|J\| over all voxels. |
| [`field_energy_audit.h`](../../engine/sim/include/ftd/sim/observables/field_energy_audit.h) | 44 | @file ftd/sim/observables/field_energy_audit.h @brief FieldEnergyAudit — forwards to the engine's built-in energy audit. |
| [`device_state.h`](../../engine/sim/include/ftd/sim/device_state.h) | 41 | @file ftd/sim/device_state.h @brief Backend-agnostic handle to a lattice simulation in flight. |

### `web/scale1`  (10 files, 1,844 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`controller.js`](../../engine/web/js/scales/scale1/controller.js) | 593 | Scale 1 (Particles) Controller Extracted from app.js to isolate the Particle Engine (PE) frame loop, scenario loader, cloud rendering, trail history, and field overlay logic. |
| [`scenarios.js`](../../engine/web/js/scales/scale1/scenarios.js) | 440 | Scale 1 — PE Scenario Loader ──────────────────────────────────────────────────────────────────── Extracted verbatim from scales/scale1/controller.js (ticket S1-1). |
| [`pe-cloud-expander.js`](../../engine/web/js/scales/scale1/pe-cloud-expander.js) | 351 | Scale 1 — PE Cloud Expander ──────────────────────────────────────────────────────────────────── Fixed-boundary point cloud per particle. |
| [`pe-dynamics.js`](../../engine/web/js/scales/scale1/pe-dynamics.js) | 167 | Scale-1 initial-condition helpers — velocities derived from the live force kernel at t=0, not closed-form orbital formulas. |
| [`pe-controls.js`](../../engine/web/js/scales/scale1/ui/controls/pe-controls.js) | 104 | Scale 1 — Particle Engine Controls Card Factory function that returns the "Particle Engine Controls" card DOM element. |
| [`template.js`](../../engine/web/js/scales/scale1/ui/overlays/template.js) | 87 | Scale 1 Viewport Overlay — particle engine dynamics (grouped by physical role) |
| [`template.js`](../../engine/web/js/scales/scale1/ui/toolbar/template.js) | 51 | _symbols:_ getScale1ScenarioToolbarTemplate() |
| [`component.js`](../../engine/web/js/scales/scale1/ui/controls/component.js) | 29 | Scale 1 Controls Component Mounts the Scale 1 (Particle Engine) control card into the controls panel. |
| [`register-scale1-ui.js`](../../engine/web/js/scales/scale1/ui/register-scale1-ui.js) | 11 | _symbols:_ registerScale1ToolbarUI() |
| [`component.js`](../../engine/web/js/scales/scale1/ui/toolbar/component.js) | 11 | _symbols:_ createScale1ScenarioToolbarGroup() |

### `web/atlas`  (9 files, 1,386 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`atlas-fields.js`](../../engine/web/js/atlas/atlas-fields.js) | 453 | FTD Ontology Atlas — field-layer renderers. |
| [`atlas-ui.js`](../../engine/web/js/atlas/atlas-ui.js) | 318 | FTD Ontology Atlas — UI: layer panel, detail panel, chain stepper, mode switch. |
| [`atlas-main.js`](../../engine/web/js/atlas/atlas-main.js) | 206 | FTD Ontology Atlas — bootstrap. |
| [`atlas-overlay.js`](../../engine/web/js/atlas/atlas-overlay.js) | 129 | FTD Ontology Atlas — animated SVG chain overlay. |
| [`atlas-content.js`](../../engine/web/js/atlas/atlas-content.js) | 73 | FTD Ontology Atlas — pedagogical content + epistemic tags. |
| [`atlas-scene.js`](../../engine/web/js/atlas/atlas-scene.js) | 64 | Three.js scene for the Ontology Atlas: camera, OrbitControls, bloom, and a world→screen projector the SVG overlay uses to anchor 2D chain arrows. |
| [`atlas-lattice.js`](../../engine/web/js/atlas/atlas-lattice.js) | 57 | The Moore-neighbourhood lattice — the substrate "stage". |
| [`atlas-data.js`](../../engine/web/js/atlas/atlas-data.js) | 53 | Static analytic illustrative fields for the Ontology Atlas. |
| [`atlas-chain.js`](../../engine/web/js/atlas/atlas-chain.js) | 33 | FTD Ontology Atlas — the causal chain the stepper walks (pure data). |

### `src/phases`  (4 files, 1,366 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`phase_write.cpp`](../../engine/src/render_bridge_phases/phase_write.cpp) | 453 | @file engine/src/render_bridge_phases/phase_write.cpp @purpose Implementation of phase_write decomposition (Phase 4a, 2026-04-27). |
| [`phase_forces.cpp`](../../engine/src/render_bridge_phases/phase_forces.cpp) | 355 | @file engine/src/render_bridge_phases/phase_forces.cpp @purpose Implementation of phase_forces decomposition (Phase 4b, 2026-04-27). |
| [`phase_movement.cpp`](../../engine/src/render_bridge_phases/phase_movement.cpp) | 350 | @file engine/src/render_bridge_phases/phase_movement.cpp @purpose Implementation of phase_movement decomposition (Phase 4c, 2026-04-27). |
| [`phase_read.cpp`](../../engine/src/render_bridge_phases/phase_read.cpp) | 208 | @file engine/src/render_bridge_phases/phase_read.cpp @purpose Implementation of phase_read decomposition (Phase 4c, 2026-04-27). |

### `vendor`  (9 files, 1,274 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`quarto.js`](../../engine/VISUAL_GUIDE_files/libs/quarto-html/quarto.js) | 847 |  |
| [`raf-coordinator.js`](../../engine/web/js/lib/raf-coordinator.js) | 156 | Single rAF coordinator for all dashboard panels. |
| [`axe-check.js`](../../engine/VISUAL_GUIDE_files/libs/quarto-html/axe/axe-check.js) | 145 | _symbols:_ QuartoAxeReporter, QuartoAxeJsonReporter, QuartoAxeConsoleReporter, QuartoAxeDocumentReporter |
| [`tabsets.js`](../../engine/VISUAL_GUIDE_files/libs/quarto-html/tabsets/tabsets.js) | 95 | grouped tabsets |
| [`anchor.min.js`](../../engine/VISUAL_GUIDE_files/libs/quarto-html/anchor.min.js) | 9 |  |
| [`bootstrap.min.js`](../../engine/VISUAL_GUIDE_files/libs/bootstrap/bootstrap.min.js) | 7 |  |
| [`clipboard.min.js`](../../engine/VISUAL_GUIDE_files/libs/clipboard/clipboard.min.js) | 7 |  |
| [`popper.min.js`](../../engine/VISUAL_GUIDE_files/libs/quarto-html/popper.min.js) | 6 |  |
| [`tippy.umd.min.js`](../../engine/VISUAL_GUIDE_files/libs/quarto-html/tippy.umd.min.js) | 2 |  |

### `web/inspector`  (9 files, 1,172 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`atoms.js`](../../engine/web/js/inspector/scales/atoms.js) | 295 | _symbols:_ handleAEClick(), showAEInspector(), hideAEInspector(), updateAEFields() |
| [`lattice.js`](../../engine/web/js/inspector/scales/lattice.js) | 235 | _symbols:_ handleLatticeClick(), showLatticeInspector(), hideLatticeInspector(), updateLatticeFields() |
| [`dom-bindings.js`](../../engine/web/js/inspector/dom-bindings.js) | 166 | _symbols:_ collectInspectorDom() |
| [`planetary.js`](../../engine/web/js/inspector/scales/planetary.js) | 110 | _symbols:_ classifyBiome(), handlePlanetaryClick(), showPlanetaryInspector(), hidePlanetaryInspector() |
| [`particles.js`](../../engine/web/js/inspector/scales/particles.js) | 92 | _symbols:_ handlePEClick(), showPEInspector(), hidePEInspector(), updatePEFields() |
| [`cosmic.js`](../../engine/web/js/inspector/scales/cosmic.js) | 84 | _symbols:_ handleCosmicClick(), showCosmicInspector(), hideCosmicInspector(), updateCosmicFields() |
| [`chrome.js`](../../engine/web/js/inspector/chrome.js) | 76 | _symbols:_ resetInspectorSelection(), hasInspectorSelection(), getInspectorModeCopy(), getInspectorSelectionSummary() |
| [`pointer-controller.js`](../../engine/web/js/inspector/pointer-controller.js) | 63 | _symbols:_ bindInspectorPointerControls() |
| [`app-runtime.js`](../../engine/web/js/inspector/app-runtime.js) | 51 | _symbols:_ createInspectorAppRuntime() |

### `web/config`  (4 files, 1,131 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`toggles.js`](../../engine/web/js/config/toggles.js) | 467 | Toggle Configuration — Single source of truth for all scale toggles. |
| [`scenarios.js`](../../engine/web/js/config/scenarios.js) | 343 | Scenario Descriptions — Metadata for scenario dropdowns and info panels. |
| [`exoplanet-seeds.js`](../../engine/web/js/config/exoplanet-seeds.js) | 287 | _symbols:_ EXOPLANET_SEEDS |
| [`perf-flags.js`](../../engine/web/js/config/perf-flags.js) | 34 | Perf rollout flags — SPEC_SCALE0_PERF_TELEMETRY_PANELS.md §10. |

### `src/cli_demos`  (1 files, 981 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`cli_demo_scenarios.cpp`](../../engine/src/cli_demos/cli_demo_scenarios.cpp) | 981 | Implementations of the ftd_sim CLI demo scenarios. |

### `src/atom`  (3 files, 847 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`atom_forces.cpp`](../../engine/src/atom/atom_forces.cpp) | 632 | AtomEngine force computation. |
| [`atom_thermostat.cpp`](../../engine/src/atom/atom_thermostat.cpp) | 142 | AtomEngine velocity post-processing: speed limit, damping, Berendsen thermostat, and per-atom dipole moment computation. |
| [`atom_bonding.cpp`](../../engine/src/atom/atom_bonding.cpp) | 73 | AtomEngine dynamic bond formation / breaking. |

### `src/cognition`  (1 files, 835 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`cognitive_lattice.cpp`](../../engine/src/cognition/cognitive_lattice.cpp) | 835 |  |

### `web/backgrounds`  (7 files, 781 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`nebula.js`](../../engine/web/js/backgrounds/nebula.js) | 202 | Nebula theme — colorful multi-layer volumetric gas clouds with filaments and a composed starfield backdrop. |
| [`beyond.js`](../../engine/web/js/backgrounds/beyond.js) | 145 | "The Beyond" theme — fading grid extending outward, suggesting a lattice with no defined boundary, with sparse flickering void points between lines. |
| [`flux-storm.js`](../../engine/web/js/backgrounds/flux-storm.js) | 139 | Flux Storm theme — swirling tilted bands of colored particles suggesting active flux dynamics, with a composed starfield backdrop. |
| [`starfield.js`](../../engine/web/js/backgrounds/starfield.js) | 99 | Starfield theme — deep-space star field with twinkling. |
| [`foam.js`](../../engine/web/js/backgrounds/foam.js) | 94 | Quantum Foam theme — dense flickering micro-points suggesting vacuum fluctuations. |
| [`hdri-loader.js`](../../engine/web/js/backgrounds/hdri-loader.js) | 52 | HDRI environment loader for 360 degree scene backgrounds. |
| [`_shared.js`](../../engine/web/js/backgrounds/_shared.js) | 50 | Shared utilities for background theme modules. |

### `web/scale4`  (5 files, 454 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`controller.js`](../../engine/web/js/scales/scale4/controller.js) | 375 | Scale 4 (Planetary) Controller Owns the Scale 4 N-Body physics loop, scenario loading, and UI list mapping. |
| [`template.js`](../../engine/web/js/scales/scale4/ui/overlays/template.js) | 31 | Scale 4 Viewport Overlay — planetary visualization controls |
| [`template.js`](../../engine/web/js/scales/scale4/ui/toolbar/template.js) | 26 | _symbols:_ getScale4ScenarioToolbarTemplate() |
| [`register-scale4-ui.js`](../../engine/web/js/scales/scale4/ui/register-scale4-ui.js) | 11 | _symbols:_ registerScale4ToolbarUI() |
| [`component.js`](../../engine/web/js/scales/scale4/ui/toolbar/component.js) | 11 | _symbols:_ createScale4ScenarioToolbarGroup() |

### `web/scale5`  (5 files, 403 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`controller.js`](../../engine/web/js/scales/scale5/controller.js) | 305 | Scale 5 — Cosmic Controller Manages the cosmic scale: N-body gravitational simulation with Hubble expansion, dark matter, and cosmological diagnostics. |
| [`template.js`](../../engine/web/js/scales/scale5/ui/toolbar/template.js) | 40 | _symbols:_ getScale5ScenarioToolbarTemplate(), getScale5TelemetryToolbarTemplate() |
| [`template.js`](../../engine/web/js/scales/scale5/ui/overlays/template.js) | 24 | Scale 5 Viewport Overlay — cosmic simulation context |
| [`register-scale5-ui.js`](../../engine/web/js/scales/scale5/ui/register-scale5-ui.js) | 19 | _symbols:_ registerScale5ToolbarUI() |
| [`component.js`](../../engine/web/js/scales/scale5/ui/toolbar/component.js) | 15 | _symbols:_ createScale5ScenarioToolbarGroup(), createScale5TelemetryToolbarGroup() |

### `web/core`  (6 files, 348 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`ui-binder.js`](../../engine/web/js/core/ui-binder.js) | 98 | @file ui-binder.js @brief DOM Event Binder and Mediator for FTD web dashboard. |
| [`registry.js`](../../engine/web/js/core/registry.js) | 73 | @file registry.js @brief Service Registry for the FTD web dashboard. |
| [`app-store.js`](../../engine/web/js/core/app-store.js) | 59 | @file app-store.js @brief Reactive App State Store for FTD web dashboard. |
| [`BaseRenderer.js`](../../engine/web/js/core/BaseRenderer.js) | 46 | _symbols:_ BaseRenderer |
| [`log.js`](../../engine/web/js/core/log.js) | 39 | Lightweight debug logger for the web dashboard. |
| [`component.js`](../../engine/web/js/core/component.js) | 33 | @file engine/web/js/core/component.js @purpose Base component class utilizing browser-native template elements and reference binding. |

### `web/scale3`  (6 files, 314 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`controller.js`](../../engine/web/js/scales/scale3/controller.js) | 249 | Scale 3 (Molecules) Controller ──────────────────────────────────────────────────────────────────── Owns molecule scenario loading for Scale 3. |
| [`component.js`](../../engine/web/js/scales/scale3/ui/controls/component.js) | 18 | Scale 3 Controls Component Scale 3 (Molecules) shares the Atom Engine controls card with Scale 2. |
| [`template.js`](../../engine/web/js/scales/scale3/ui/toolbar/template.js) | 17 | _symbols:_ getScale3ScenarioToolbarTemplate() |
| [`register-scale3-ui.js`](../../engine/web/js/scales/scale3/ui/register-scale3-ui.js) | 11 | _symbols:_ registerScale3ToolbarUI() |
| [`component.js`](../../engine/web/js/scales/scale3/ui/toolbar/component.js) | 11 | _symbols:_ createScale3ScenarioToolbarGroup() |
| [`template.js`](../../engine/web/js/scales/scale3/ui/overlays/template.js) | 8 | Scale 3 overlay — re-exported from Scale 2 (shared scale-ae panel). |

### `web/other`  (6 files, 268 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`coi-serviceworker.js`](../../engine/web/coi-serviceworker.js) | 137 | ! coi-serviceworker v0.1.7 - Guido Zuidhof and contributors, licensed under MIT |
| [`serve.py`](../../engine/web/serve.py) | 88 | Dev http server for engine/web/ that disables browser caching. |
| [`wasm-threads-proof.worker.js`](../../engine/web/wasm-threads-proof.worker.js) | 37 | Phase-1 off-thread proof: host ftd_core_mt in a worker at pool=1 (pure serial, no thread spawns). |
| [`ftd_core.js`](../../engine/web/wasm/ftd_core.js) | 2 |  |
| [`ftd_core64.js`](../../engine/web/wasm/ftd_core64.js) | 2 |  |
| [`ftd_core_mt.js`](../../engine/web/wasm/ftd_core_mt.js) | 2 |  |

### `other`  (1 files, 261 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`run_tests_json.py`](../../engine/run_tests_json.py) | 261 | Run CTest suite and produce structured JSON for the test dashboard. |

### `web/scales-shared`  (1 files, 197 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`scale-utils.js`](../../engine/web/js/scales/scale-utils.js) | 197 | Scale Utilities -- Shared helpers for scale controllers ──────────────────────────────────────────────────────────────────── Common formatting, throttling, and DOM-update utilities extracted from a... |

### `web/telemetry`  (4 files, 144 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`demand.js`](../../engine/web/js/telemetry/demand.js) | 58 | Scale-0 telemetry demand gating — decides which expensive hub collectors run. |
| [`scale0-read.js`](../../engine/web/js/telemetry/scale0-read.js) | 37 | Scale-0 telemetry read helpers — prefer hub snapshots, fall back to bridge. |
| [`scale0-grid-channels.js`](../../engine/web/js/telemetry/registry/scale0-grid-channels.js) | 31 | Scale-0 telemetry grid channel registry. |
| [`index.js`](../../engine/web/js/telemetry/index.js) | 18 | Telemetry module barrel — hub (single write path) + demand gating + registries. |

### `web/scale23`  (3 files, 70 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`template.js`](../../engine/web/js/scales/scale23/ui/toolbar/template.js) | 36 | _symbols:_ getScale23VisualToolbarTemplate(), getScale23ForceToolbarTemplate() |
| [`register-scale23-ui.js`](../../engine/web/js/scales/scale23/ui/register-scale23-ui.js) | 19 | _symbols:_ registerScale23ToolbarUI() |
| [`component.js`](../../engine/web/js/scales/scale23/ui/toolbar/component.js) | 15 | _symbols:_ createScale23VisualToolbarGroup(), createScale23ForceToolbarGroup() |

