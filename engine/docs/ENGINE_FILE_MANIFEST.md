# Engine File Manifest (auto-generated)

> Regenerate: `python engine/tools/build_file_manifest.py`  
> Machine-readable source of truth: [`ENGINE_FILE_MANIFEST.json`](ENGINE_FILE_MANIFEST.json)  
> Narrative map: [`ENGINE_CODE_MAP.md`](ENGINE_CODE_MAP.md)

**2616 code files, 692,518 LOC** (tracked `.cpp/.cc/.h/.hpp/.cu/.cuh/.js/.mjs/.py` under `engine/`).

## Subsystem rollup

| Subsystem | Files | LOC |
|---|--:|--:|
| `tests` | 810 | 245,128 |
| `other` | 806 | 163,084 |
| `web/js-toplevel` | 51 | 74,989 |
| `src/core` | 196 | 66,956 |
| `include` | 296 | 37,353 |
| `cuda` | 29 | 17,907 |
| `web/scale0` | 65 | 17,876 |
| `web/tests` | 93 | 17,215 |
| `web/ui` | 84 | 10,986 |
| `web/viewport` | 19 | 8,653 |
| `web/bridge` | 24 | 7,646 |
| `src/scenarios` | 7 | 3,387 |
| `wasm` | 5 | 2,706 |
| `web/scale1` | 14 | 2,077 |
| `web/scale2` | 12 | 2,074 |
| `sim` | 16 | 1,951 |
| `src/phases` | 4 | 1,606 |
| `vendor` | 12 | 1,425 |
| `web/atlas` | 9 | 1,386 |
| `web/config` | 4 | 1,373 |
| `web/inspector` | 9 | 1,291 |
| `tools` | 4 | 1,199 |
| `src/atom` | 3 | 844 |
| `web/backgrounds` | 7 | 781 |
| `web/scale4` | 5 | 427 |
| `web/other` | 6 | 408 |
| `web/scale5` | 5 | 379 |
| `web/core` | 6 | 348 |
| `web/scale3` | 6 | 314 |
| `web/scales-shared` | 1 | 253 |
| `web/telemetry` | 4 | 239 |
| `web/scale6` | 1 | 187 |
| `web/scale23` | 3 | 70 |

## Files by subsystem

### `tests`  (810 files, 245,128 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`test_l17_complete_tangent_candidate.cpp`](../../engine/tests/test_l17_complete_tangent_candidate.cpp) | 4161 | Shared implementation for the SHA-locked FTD-0774 candidate and the target-blind FTD-0829 certificate-repair successor. |
| [`test_scenario_behavior.cpp`](../../engine/tests/test_scenario_behavior.cpp) | 3308 | Behavioral regression tests for the Scale-0 scenarios whose initial data are intended to evolve, rather than merely render a recognizable picture. |
| [`test_gpu_physics.cpp`](../../engine/tests/test_gpu_physics.cpp) | 2697 | GPU Physics Test Suite — Testing Ontic Predictions at Scale Leverages the CUDA GpuEngine ( speedup) to run physics campaigns at lattice sizes and tick counts impractical on CPU. |
| [`campaign_total_momentum_stress_ledger.cpp`](../../engine/tests/campaign_total_momentum_stress_ledger.cpp) | 2071 | Total momentum stress ledger campaign (Arc 2) on CUDA. |
| [`campaign_dark_sector.cpp`](../../engine/tests/campaign_dark_sector.cpp) | 1762 | Campaign: Dark Sector (consolidated) Wave 4c.11 consolidation, 7->1 dark sector merge. |
| [`test_gpu_experiments.cpp`](../../engine/tests/test_gpu_experiments.cpp) | 1734 | GPU Particle Physics Experiment Suite Simulations of real scientific experiments on the FTD GPU engine, using ALL available telemetry (EnergyAudit, sync_to_host, phi_coulomb) with quantitative pred... |
| [`connected_moore_tangent_codec.h`](../../engine/tests/support/connected_moore_tangent_codec.h) | 1672 | @file connected_moore_tangent_codec.h @brief Test-only constrained tangent chart used by the locked FTD-0774 run. |
| [`test_constructors.cpp`](../../engine/tests/test_constructors.cpp) | 1354 | test_constructors — unit tests for ftd::ctor::* Spec: docs/superpowers/specs/2026-04-15-ftd-constructors-design.md |
| [`campaign_hydrogen_spectrum.cpp`](../../engine/tests/campaign_hydrogen_spectrum.cpp) | 1294 | Campaign: Hydrogen Spectrum (consolidated suite) Merges 5 legacy hydrogen test files into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_hydrogen_scale1 -> sect... |
| [`campaign_graviton_tt_correlator.cpp`](../../engine/tests/campaign_graviton_tt_correlator.cpp) | 1220 | @file campaign_graviton_tt_correlator.cpp @brief Frontier 4, Step 4a-ii — emergent transverse-traceless (spin-2) pole. |
| [`benchmark_engine_theory.cpp`](../../engine/tests/benchmark_engine_theory.cpp) | 1181 | ENGINE-THEORY BRIDGE BENCHMARK — COMPREHENSIVE Quantitative comparison of C++ engine output to FTD theory. |
| [`test_pe_forces.cpp`](../../engine/tests/test_pe_forces.cpp) | 1139 | Test: ParticleEngine force variants (consolidated suite) Merges 7 legacy test_pe_*.cpp files into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_pe_exchange ->... |
| [`benchmark_black_hole_thermo.cpp`](../../engine/tests/benchmark_black_hole_thermo.cpp) | 1132 | BLACK HOLE THERMODYNAMICS BENCHMARKS Tests FTD lattice predictions for black hole thermodynamics using the Scale 0 RenderBridge engine. |
| [`campaign_long_transport_dynamic_response_cuda.cpp`](../../engine/tests/campaign_long_transport_dynamic_response_cuda.cpp) | 1043 | FTD-0768: long-transport paired dynamic-response campaign on CUDA. |
| [`test_atom_engine_forces.cpp`](../../engine/tests/test_atom_engine_forces.cpp) | 1014 | Test: AtomEngine force variants (consolidated suite) Merges 5 legacy test_ae_*.cpp files into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_ae_angle_strain ->... |
| [`campaign_held_out_pair_specific_phase_wedge_centrality.cpp`](../../engine/tests/campaign_held_out_pair_specific_phase_wedge_centrality.cpp) | 1008 | @file campaign_held_out_pair_specific_phase_wedge_centrality.cpp @brief FTD-0911 locked held-out pair-specificity/centrality census. |
| [`campaign_m3_support_invariant_validation_cuda.cpp`](../../engine/tests/campaign_m3_support_invariant_validation_cuda.cpp) | 968 | FTD-0755: support-invariant finite-time matter-family validation runner. |
| [`campaign_production_ternary_plaquette_recurrence_census.cpp`](../../engine/tests/campaign_production_ternary_plaquette_recurrence_census.cpp) | 945 | @file campaign_production_ternary_plaquette_recurrence_census.cpp @brief FTD-0915 locked observation-only production plaquette census. |
| [`test_cell_measure_long_horizon_transport.cpp`](../../engine/tests/test_cell_measure_long_horizon_transport.cpp) | 944 | FTD-0650: long-horizon transport of one fixed-mass cell-measure object. |
| [`test_causal_excitation_separation_v1.cpp`](../../engine/tests/test_causal_excitation_separation_v1.cpp) | 927 | FTD-0684/0685: fresh causal excitation-separation discriminator. |
| [`test_full_mirrored_internal_shape_core.cpp`](../../engine/tests/test_full_mirrored_internal_shape_core.cpp) | 906 | FTD-0605: full mirrored internal-shape matter-core discriminator. |
| [`campaign_m4_boosted_relational_transport_discovery_cuda.cpp`](../../engine/tests/campaign_m4_boosted_relational_transport_discovery_cuda.cpp) | 873 | FTD-0761: boosted transport discovery for the certified M3 family. |
| [`campaign_genesis_amplitude_ceiling.cpp`](../../engine/tests/campaign_genesis_amplitude_ceiling.cpp) | 844 | @file campaign_genesis_amplitude_ceiling.cpp @brief Is there a maximum amplitude for coherent light in FTD? |
| [`test_captured_state_perturbation_survival.cpp`](../../engine/tests/test_captured_state_perturbation_survival.cpp) | 840 | FTD-0732: locked captured-state perturbation survival campaign. |
| [`campaign_coulomb_force_law.cpp`](../../engine/tests/campaign_coulomb_force_law.cpp) | 837 | Campaign: Coulomb force law (consolidated suite) Merges 5 legacy test/campaign_*.cpp files into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_poisson_coulomb -... |
| [`campaign_thomson_moving_recoil_accounting.cpp`](../../engine/tests/campaign_thomson_moving_recoil_accounting.cpp) | 823 | FTD-0297: Thomson moving-recoil source/work accounting. |
| [`test_global_orientation_strain_core.cpp`](../../engine/tests/test_global_orientation_strain_core.cpp) | 819 | FTD-0606: global SO(3) x local-strain compact matter-core discriminator. |
| [`test_gpu_parity_complete.cpp`](../../engine/tests/test_gpu_parity_complete.cpp) | 803 | GPU Parity Complete: Every physics domain tested on GPU vs CPU. |
| [`test_logic_engine.cpp`](../../engine/tests/test_logic_engine.cpp) | 790 | Test: Logic-First Engine — Comprehensive Verification 40 checks verifying that the 6-rule logic-first engine behaves correctly. |
| [`test_lorentz.cpp`](../../engine/tests/test_lorentz.cpp) | 785 | Test: Lorentz + Magnetic family (consolidated suite) Merges 5 legacy tests into test_lorentz.cpp (self-ref target) using the Phase 2a ftd::test NDJSON telemetry API: test_lorentz -> section "lorent... |
| [`test_native_excited_matter_clock.cpp`](../../engine/tests/test_native_excited_matter_clock.cpp) | 774 | FTD-0659: basis-independent action--angle phase of the first internal doublet. |
| [`test_causal_regional_field_flow.cpp`](../../engine/tests/test_causal_regional_field_flow.cpp) | 766 | FTD-0672: causal nested-radius field-flow discriminator. |
| [`campaign_production_orientation_memory_census.cpp`](../../engine/tests/campaign_production_orientation_memory_census.cpp) | 762 | @file campaign_production_orientation_memory_census.cpp @brief FTD-0908 locked observation-only production formation census. |
| [`test_prescribed_trajectory_comoving_field_shooting.cpp`](../../engine/tests/test_prescribed_trajectory_comoving_field_shooting.cpp) | 761 | FTD-0710: solve the complete matched field in the co-moving frame of the prescribed two-tick rigid translation, then replay the unchanged reciprocal matter action without retuning. |
| [`test_maxwell.cpp`](../../engine/tests/test_maxwell.cpp) | 753 | Test: Maxwell Equation Recovery Verifies that the FTD wave equation + Gauss constraint recovers Maxwell's equations by reconstructing E and B fields and checking their relationships. |
| [`test_persistence_covariance_convergence.cpp`](../../engine/tests/test_persistence_covariance_convergence.cpp) | 736 | FTD-0728: locked persistence covariance convergence campaign. |
| [`test_multipass_formation_persistence.cpp`](../../engine/tests/test_multipass_formation_persistence.cpp) | 721 | FTD-0731: locked multi-pass formation persistence campaign. |
| [`campaign_free_dynamics.cpp`](../../engine/tests/campaign_free_dynamics.cpp) | 717 | Campaign: Free Particle Dynamics — Hierarchical Exploration Probes the engine's behavior with FREE (unlocked) particles, building from simplest to most complex: FD1: Single free particle — inertia... |
| [`test_internal_mode_action_transfer.cpp`](../../engine/tests/test_internal_mode_action_transfer.cpp) | 714 | FTD-0660: direct constituent/dressing/dynamic-field action-transfer ledger. |
| [`test_finite_support_environmental_closure.cpp`](../../engine/tests/test_finite_support_environmental_closure.cpp) | 709 | FTD-0745: held-out environmental closure after FTD-0739 formation. |
| [`test_site_admissible_compact_matter_motion.cpp`](../../engine/tests/test_site_admissible_compact_matter_motion.cpp) | 698 | FTD-0607: site-admissible compact matter autonomous-motion discriminator. |
| [`test_emergent_ic1_topology.cpp`](../../engine/tests/test_emergent_ic1_topology.cpp) | 683 | @file test_emergent_ic1_topology.cpp @brief Regression + topology verification for the canonical ic1 cluster. |
| [`test_bound_dressing_persistence.cpp`](../../engine/tests/test_bound_dressing_persistence.cpp) | 678 | FTD-0727: locked finite-volume bound-dressing persistence campaign. |
| [`test_connected_block_translation_stability.cpp`](../../engine/tests/test_connected_block_translation_stability.cpp) | 676 | FTD-0624: dynamical classification of connected-block translation extrema. |
| [`test_finite_support_outgoing_tail_formation.cpp`](../../engine/tests/test_finite_support_outgoing_tail_formation.cpp) | 668 | FTD-0739: finite-support outgoing-tail matter formation. |
| [`test_persistence_reentry_volume_discriminator.cpp`](../../engine/tests/test_persistence_reentry_volume_discriminator.cpp) | 668 | FTD-0730: locked persistence/re-entry volume discriminator. |
| [`campaign_m4_moving_dressing_observer_forensics_cuda.cpp`](../../engine/tests/campaign_m4_moving_dressing_observer_forensics_cuda.cpp) | 664 | FTD-0762: outcome-aware CUDA forensics for the FTD-0761 observer failure. |
| [`test_lower_energy_covariance_conditioning.cpp`](../../engine/tests/test_lower_energy_covariance_conditioning.cpp) | 656 | FTD-0725: locked translation-covariance conditioning diagnostic. |
| [`test_einstein_equations.cpp`](../../engine/tests/test_einstein_equations.cpp) | 655 | Test: Einstein Equations on the FTD Lattice Validates the gravitational sector: Poisson potential, 1/r profile, G_N extraction, and proper time dilation. |
| [`test_refined_core_peierls_landscape.cpp`](../../engine/tests/test_refined_core_peierls_landscape.cpp) | 654 | FTD-0614: selected compact-core Peierls landscape and proper covariance. |
| [`test_moving_dressed_matter_transverse_field_growth.cpp`](../../engine/tests/test_moving_dressed_matter_transverse_field_growth.cpp) | 652 | FTD-0705: finite-horizon transverse-field growth from coherently moving selected dressed matter. |
| [`campaign_flux_self_interference_response.cpp`](../../engine/tests/campaign_flux_self_interference_response.cpp) | 647 | @file campaign_flux_self_interference_response.cpp @brief FTD-0435 polarity and polarization audit of the selected flux/self-field force response. |
| [`campaign_dispersion.cpp`](../../engine/tests/campaign_dispersion.cpp) | 646 | Campaign: Dispersion Relation (consolidated suite) Merges 3 legacy dispersion test/campaign files into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_dispersion... |
| [`campaign_thomson_native_continuity.cpp`](../../engine/tests/campaign_thomson_native_continuity.cpp) | 646 | FTD-0291: Thomson native finite-volume continuity meter. |
| [`test_energy_adapted_mixed_capture_corners.cpp`](../../engine/tests/test_energy_adapted_mixed_capture_corners.cpp) | 641 | FTD-0734: energy-adapted mixed capture corners. |
| [`test_spline_poynting_noether_defect.cpp`](../../engine/tests/test_spline_poynting_noether_defect.cpp) | 639 | FTD-0619: spline-Poynting versus fixed-lattice Noether defect. |
| [`campaign_transported_chart_matter_morphology_cuda.cpp`](../../engine/tests/campaign_transported_chart_matter_morphology_cuda.cpp) | 638 | FTD-0764: transported-chart morphology and momentum replay on CUDA. |
| [`campaign_m3_relational_chart_held_out_validation_cuda.cpp`](../../engine/tests/campaign_m3_relational_chart_held_out_validation_cuda.cpp) | 637 | FTD-0760: relational-chart fresh held-out M3 validation. |
| [`test_causal_buffer_relational_formation.cpp`](../../engine/tests/test_causal_buffer_relational_formation.cpp) | 637 | FTD-0736: causal-buffer relational-formation discriminator. |
| [`campaign_reciprocal_moving_source.cpp`](../../engine/tests/campaign_reciprocal_moving_source.cpp) | 627 | @file campaign_reciprocal_moving_source.cpp @brief FTD-0477 selected-force moving-source reciprocity discriminator. |
| [`test_connected_block_dynamic_stabilization.cpp`](../../engine/tests/test_connected_block_dynamic_stabilization.cpp) | 621 | FTD-0625: existing-variable circulation at the connected-block collision surface. |
| [`test_dk_evolution.cpp`](../../engine/tests/test_dk_evolution.cpp) | 617 | @file test_dk_evolution.cpp @brief M1 (FTD-0379) — Dirac-Kähler evolution test. |
| [`test_volume_scaled_internal_mode_transfer.cpp`](../../engine/tests/test_volume_scaled_internal_mode_transfer.cpp) | 616 | FTD-0664: volume-scaled pre-return transfer of the first internal doublet. |
| [`test_connected_block_shared_anchor_fibre.cpp`](../../engine/tests/test_connected_block_shared_anchor_fibre.cpp) | 612 | FTD-0626: connected Moore block under the already priced chart fibre. |
| [`test_closed_neutral_trimer_pair.cpp`](../../engine/tests/test_closed_neutral_trimer_pair.cpp) | 610 | FTD-0601: closed neutral pair of constituent-complete charged trimers. |
| [`test_causal_horizon_environmental_persistence.cpp`](../../engine/tests/test_causal_horizon_environmental_persistence.cpp) | 607 | FTD-0746: causal-horizon radius-48 environmental persistence. |
| [`test_covariant_lower_energy_formation.cpp`](../../engine/tests/test_covariant_lower_energy_formation.cpp) | 607 | FTD-0726: locked covariant lower-energy formation campaign. |
| [`test_lower_energy_formation_crossover.cpp`](../../engine/tests/test_lower_energy_formation_crossover.cpp) | 602 | FTD-0724: locked lower-energy formation-crossover campaign. |
| [`test_closed_symmetry_balanced_gait.cpp`](../../engine/tests/test_closed_symmetry_balanced_gait.cpp) | 600 | FTD-0618: one closed, symmetry-paired six-constituent internal gait. |
| [`test_connected_block_static_dressing_refinement.cpp`](../../engine/tests/test_connected_block_static_dressing_refinement.cpp) | 599 | FTD-0628: symmetry-reduced static dressing refinement. |
| [`test_cuda_paired_field_response.cpp`](../../engine/tests/test_cuda_paired_field_response.cpp) | 599 | FTD-0768: paired field-response and regional-ledger CPU/CUDA qualification. |
| [`test_resonant_internal_gait_cancellation.cpp`](../../engine/tests/test_resonant_internal_gait_cancellation.cpp) | 598 | FTD-0712: vary only the internal midpoint gait of the qualified composite and test exact cancellation of the eight body-diagonal co-moving null modes. |
| [`test_field_assisted_capture_window.cpp`](../../engine/tests/test_field_assisted_capture_window.cpp) | 594 | FTD-0723: locked incident-momentum capture-window campaign. |
| [`test_period_three_internal_momentum_lift.cpp`](../../engine/tests/test_period_three_internal_momentum_lift.cpp) | 594 | FTD-0715: lift the FTD-0713 causal internal deformation to the first momentum-return period not excluded by FTD-0714. |
| [`test_late_reentry_covariance_convergence.cpp`](../../engine/tests/test_late_reentry_covariance_convergence.cpp) | 588 | FTD-0729: locked late-reentry covariance convergence diagnostic. |
| [`test_localized_basin_relaxation.cpp`](../../engine/tests/test_localized_basin_relaxation.cpp) | 585 | FTD-0678: fresh localized-basin relaxation discriminator. |
| [`test_single_core_neutralizer_control.cpp`](../../engine/tests/test_single_core_neutralizer_control.cpp) | 583 | FTD-0610: single compact core versus uniform and frozen neutralizers. |
| [`test_uniform_neutralized_single_core_static.cpp`](../../engine/tests/test_uniform_neutralized_single_core_static.cpp) | 580 | FTD-0611: stationary compact core under a uniform periodic neutralizer. |
| [`test_capture_root_regularity_neighborhood.cpp`](../../engine/tests/test_capture_root_regularity_neighborhood.cpp) | 579 | FTD-0735: implicit-root regularity on captured matter histories. |
| [`test_symmetric_breathing_matter_core.cpp`](../../engine/tests/test_symmetric_breathing_matter_core.cpp) | 578 | FTD-0604: symmetric breathing matter-core discriminator. |
| [`campaign_thomson_radiation_shells.cpp`](../../engine/tests/campaign_thomson_radiation_shells.cpp) | 571 | FTD-0290: Thomson radiation shell meter. |
| [`test_minimum_energy_neutral_pair_force_sign.cpp`](../../engine/tests/test_minimum_energy_neutral_pair_force_sign.cpp) | 568 | FTD-0602: minimum-energy neutral-pair force-sign discriminator. |
| [`test_zero_momentum_internal_mode_mobility.cpp`](../../engine/tests/test_zero_momentum_internal_mode_mobility.cpp) | 568 | FTD-0615: zero-centre-momentum internal-mode mobility discriminator. |
| [`test_dk_evolution_v11.cpp`](../../engine/tests/test_dk_evolution_v11.cpp) | 566 | @file test_dk_evolution_v11.cpp @brief M1 v1.1 (FTD-0379 scope extension) — corrected-operator, free-scale Dirac-Kähler evolution re-test. |
| [`test_internal_walker_direction_persistence.cpp`](../../engine/tests/test_internal_walker_direction_persistence.cpp) | 563 | FTD-0616: signed-vector and long-time persistence discriminator for the constructive FTD-0615 zero-centre-momentum internal walker. |
| [`test_causally_isolated_envelope_turning.cpp`](../../engine/tests/test_causally_isolated_envelope_turning.cpp) | 561 | FTD-0670: held-out half-amplitude action-envelope turning before periodic self-contact. |
| [`campaign_neutral_pair_wave_response.cpp`](../../engine/tests/campaign_neutral_pair_wave_response.cpp) | 560 | @file campaign_neutral_pair_wave_response.cpp @brief FTD-0436 neutral-pair common-mode versus polarization campaign. |
| [`test_gauss_law_fidelity.cpp`](../../engine/tests/test_gauss_law_fidelity.cpp) | 558 | ============================================================================ test_gauss_law_fidelity.cpp (engine-fidelity investigation, 2026-07-16) ------------------------------------------------... |
| [`test_internal_gait_angular_response.cpp`](../../engine/tests/test_internal_gait_angular_response.cpp) | 555 | FTD-0617: complete angular response map of the constructive two-mode internal rotational gait. |
| [`campaign_gnc_qij.cpp`](../../engine/tests/campaign_gnc_qij.cpp) | 553 | @file campaign_gnc_qij.cpp @brief FTD-0349 §9 / FTD-0354 — the GNC-w discriminator: the member-site gradient quadratic form Q_ij measured on constructed, locked, Gauss-dressed engine clusters. |
| [`test_constituent_complete_charged_trimer.cpp`](../../engine/tests/test_constituent_complete_charged_trimer.cpp) | 553 | FTD-0600: constituent-complete charged-trimer common-action gate. |
| [`test_mobile_dressing_structure_factor.cpp`](../../engine/tests/test_mobile_dressing_structure_factor.cpp) | 551 | FTD-0655: observer-only co-motion of matter and field-energy structure factors. |
| [`test_energy_conservation.cpp`](../../engine/tests/test_energy_conservation.cpp) | 545 | Test: Energy Conservation (consolidated suite) Merges 3 legacy tests into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_energy -> section "energy_basic" (4 sub... |
| [`campaign_thomson_unlocked_recoil.cpp`](../../engine/tests/campaign_thomson_unlocked_recoil.cpp) | 543 | FTD-0288 provenance: native mobile-source recoil campaign. |
| [`test_out_of_sample_mobility_convergence.cpp`](../../engine/tests/test_out_of_sample_mobility_convergence.cpp) | 538 | FTD-0654: out-of-sample mobility convergence at new speeds and horizon. |
| [`campaign_bound_pilot_wake_discriminator.cpp`](../../engine/tests/campaign_bound_pilot_wake_discriminator.cpp) | 534 | @file campaign_bound_pilot_wake_discriminator.cpp @brief FTD-0475 bound-field / leading-response / wake discriminator v2. |
| [`test_balanced_gait_phase_return.cpp`](../../engine/tests/test_balanced_gait_phase_return.cpp) | 532 | FTD-0620: internal phase-return discriminator for the balanced neutral gait. |
| [`test_bell_aggregate.cpp`](../../engine/tests/test_bell_aggregate.cpp) | 525 | Test: Bell Aggregate — Ensemble S = 2sqrt(2) Verifies the three-level observer Bell hierarchy from FTD: Level 1 (Substrate): S <= 2 (local deterministic, triangular correlation) Level 2 (Complex):... |
| [`campaign_aged_wake_entrainment_cuda.cpp`](../../engine/tests/campaign_aged_wake_entrainment_cuda.cpp) | 523 | FTD-0766: aged, signed-pair wake/entrainment discriminator on CUDA. |
| [`test_l33_symmetry_rest_refinement.cpp`](../../engine/tests/test_l33_symmetry_rest_refinement.cpp) | 522 | FTD-0707: repair the L=33 rest preparation in the existing four-coordinate symmetry sector, then qualify the complete reciprocal fixed point. |
| [`test_gpu_compact_diagnostics.cpp`](../../engine/tests/test_gpu_compact_diagnostics.cpp) | 521 |  |
| [`campaign_thomson_flux_excess.cpp`](../../engine/tests/campaign_thomson_flux_excess.cpp) | 516 | FTD-0289: Thomson flux-excess discriminator. |
| [`campaign_target_blind_particlehood.cpp`](../../engine/tests/campaign_target_blind_particlehood.cpp) | 514 | @file campaign_target_blind_particlehood.cpp @brief FTD-0399 target-blind particlehood campaign. |
| [`benchmark_alpha_relaxation_lean_gpu.cu`](../../engine/tests/benchmark_alpha_relaxation_lean_gpu.cu) | 513 | @file benchmark_alpha_relaxation_lean_gpu.cu @brief Lean dynamical Coulomb benchmark using GPU Poisson relaxation. |
| [`test_connected_block_dynamical_rest_recurrence.cpp`](../../engine/tests/test_connected_block_dynamical_rest_recurrence.cpp) | 512 | FTD-0627: long-horizon classification of fibre-enabled centre rest. |
| [`test_neutral_pair_translation_phase_balance.cpp`](../../engine/tests/test_neutral_pair_translation_phase_balance.cpp) | 511 | FTD-0603: neutral-pair translation-phase/Umklapp discriminator. |
| [`campaign_wave_dynamics.cpp`](../../engine/tests/campaign_wave_dynamics.cpp) | 510 | Campaign: Wave dynamics (consolidated suite) Merges 4 legacy tests into a single ftd::test-instrumented suite using the Phase 2a NDJSON telemetry API: test_wave_speed -> section "wave_speed" test_i... |
| [`test_light.cpp`](../../engine/tests/test_light.cpp) | 510 | Test: Light & Photon Properties — 8 Checks Verifies that the engine's wave equation naturally produces massless, frequency-bearing, linearly-propagating EM waves — i.e., LIGHT — without any explici... |
| [`campaign_mass_gap_v2.cpp`](../../engine/tests/campaign_mass_gap_v2.cpp) | 509 | @file campaign_mass_gap_v2.cpp @brief FTD-0270 closure swing (P2), v2 instrument — the nonlinear-loop native mass gap, rebuilt per the FTD-0333 postmortem and the FTD-0337 mechanism correction. |
| [`benchmark_emergent_alpha.cpp`](../../engine/tests/benchmark_emergent_alpha.cpp) | 504 | EMERGENT PHYSICS BENCHMARK — Reverse-Engineering Alpha Can the fine structure constant be MEASURED from lattice field dynamics rather than read from a hardcoded constant? |
| [`campaign_m3_fixed_chart_held_out_validation_cuda.cpp`](../../engine/tests/campaign_m3_fixed_chart_held_out_validation_cuda.cpp) | 504 | FTD-0758: fixed-chart fresh held-out M3 validation. |
| [`campaign_thomson_coupled_source_work.cpp`](../../engine/tests/campaign_thomson_coupled_source_work.cpp) | 503 | FTD-0296: Fixed-charge coupled tick source/work continuity. |
| [`test_connected_dressed_matter_high_speed_preflight.cpp`](../../engine/tests/test_connected_dressed_matter_high_speed_preflight.cpp) | 503 | FTD-0704: short reversible high-speed preflight for the selected dressed connected matter candidate. |
| [`test_period_three_field_bound_common_action_selector.cpp`](../../engine/tests/test_period_three_field_bound_common_action_selector.cpp) | 503 | FTD-0718: export the locked force seed and independently replay the algorithmically selected source-free co-moving field correction. |
| [`test_triad_confinement.cpp`](../../engine/tests/test_triad_confinement.cpp) | 495 | Test: Triad Binding from Confinement (Checklist #38) Verifies that three same-sign particles with different color orientations form a bound state via the color confinement force, rather than relyin... |
| [`test_shared_anchor_constituent_fibre_transport.cpp`](../../engine/tests/test_shared_anchor_constituent_fibre_transport.cpp) | 491 | FTD-0609: two-record shared-anchor constituent-fibre transport. |
| [`test_causally_isolated_internal_recurrence.cpp`](../../engine/tests/test_causally_isolated_internal_recurrence.cpp) | 490 | FTD-0668: test the first internal-mode recurrence before periodic self-contact in a large causal buffer. |
| [`test_field_assisted_derived_pair_capture.cpp`](../../engine/tests/test_field_assisted_derived_pair_capture.cpp) | 489 | FTD-0722: locked field-assisted derived-pair capture campaign. |
| [`test_native_moore_shell_gauss.cpp`](../../engine/tests/test_native_moore_shell_gauss.cpp) | 489 | Native Moore-shell Gauss audit. |
| [`test_recovery_reservoir_donor.cpp`](../../engine/tests/test_recovery_reservoir_donor.cpp) | 487 | FTD-0674: identify the exact donor of the causal-envelope recovery. |
| [`test_canonical_subcell_section.cpp`](../../engine/tests/test_canonical_subcell_section.cpp) | 485 | FTD-0500: canonical-section repair and exact half-cell obstruction. |
| [`campaign_emergent_boundary_mechanism.cpp`](../../engine/tests/campaign_emergent_boundary_mechanism.cpp) | 484 | @file campaign_emergent_boundary_mechanism.cpp @brief FTD-0474 membrane/environment/periodic-boundary discriminator. |
| [`campaign_localized_packet_recoil.cpp`](../../engine/tests/campaign_localized_packet_recoil.cpp) | 483 | @file campaign_localized_packet_recoil.cpp @brief FTD-0457 finite localized-packet R=1 recoil gate. |
| [`test_period_three_common_action_preflight.cpp`](../../engine/tests/test_period_three_common_action_preflight.cpp) | 480 | FTD-0717: replay the independently solved FTD-0715 matter momenta and FTD-0716 minimum-norm field, then measure common-action defects. |
| [`test_toggle_matrix.cpp`](../../engine/tests/test_toggle_matrix.cpp) | 480 | @file test_toggle_matrix.cpp @brief Pairwise toggle-combination smoke test. |
| [`campaign_quantum_correlations.cpp`](../../engine/tests/campaign_quantum_correlations.cpp) | 477 | Campaign: Quantum correlations (consolidated suite) Merges 3 legacy tests into a single ftd::test-instrumented suite: test_entanglement -> section "entanglement" (16 checks) campaign_epr_correlatio... |
| [`campaign_native_dressed_hazard_ir_scaling_v2.cpp`](../../engine/tests/campaign_native_dressed_hazard_ir_scaling_v2.cpp) | 475 | @file campaign_native_dressed_hazard_ir_scaling_v2.cpp @brief FTD-0436 phase-corrected dressed evaporation-hazard scaling campaign. |
| [`test_integer_bloch_transport.cpp`](../../engine/tests/test_integer_bloch_transport.cpp) | 469 | FTD-0556: integer translation and native Bloch transport. |
| [`test_uniform_single_core_stationary_refinement.cpp`](../../engine/tests/test_uniform_single_core_stationary_refinement.cpp) | 469 | FTD-0612: deterministic refinement of the FTD-0611 positive basin. |
| [`campaign_native_retarded_polarity_transport.cpp`](../../engine/tests/campaign_native_retarded_polarity_transport.cpp) | 466 | @file campaign_native_retarded_polarity_transport.cpp @brief FTD-0430 production-hop retarded polarity-response campaign. |
| [`campaign_stagewise_e1_cpu_cuda_parity.cpp`](../../engine/tests/campaign_stagewise_e1_cpu_cuda_parity.cpp) | 466 | FTD-0751: bounded stagewise CPU/CUDA classifier for selected E1 matter. |
| [`campaign_common_action_kick_reciprocity.cpp`](../../engine/tests/campaign_common_action_kick_reciprocity.cpp) | 463 | @file campaign_common_action_kick_reciprocity.cpp @brief FTD-0468 exact source-kick/common-action momentum reciprocity. |
| [`test_atomic_energy.cpp`](../../engine/tests/test_atomic_energy.cpp) | 460 | Physics Checklist #69: Atomic Energy Levels from Scale 0 Lattice Full lattice-scale hydrogen is computationally prohibitive: a_0 ~ 613 lattice units (with gravity) or ~3374 (pure EM) -> need L > 20... |
| [`test_native_contact_active_set.cpp`](../../engine/tests/test_native_contact_active_set.cpp) | 459 | FTD-0525: frozen production vs selected hard-contact active set. |
| [`campaign_thomson_tick_local_continuity_v2.cpp`](../../engine/tests/campaign_thomson_tick_local_continuity_v2.cpp) | 453 | FTD-0295: Source-free discrete tick local continuity v2. |
| [`campaign_atomic_spectroscopy.cpp`](../../engine/tests/campaign_atomic_spectroscopy.cpp) | 451 | Campaign: Atomic Spectroscopy — hydrogen-1s on the engine (FTD-0281 rung 1) FIRST RUNG of engine-native atomic spectroscopy: does the engine's OWN Coulomb-clocked flux field ring at the bound 1s fr... |
| [`test_cluster_inertia.cpp`](../../engine/tests/test_cluster_inertia.cpp) | 448 | ============================================================================ test_cluster_inertia.cpp (unified-mass Phase 2, 2026-06-06) ------------------------------------------------------------... |
| [`test_baryogenesis.cpp`](../../engine/tests/test_baryogenesis.cpp) | 446 | Test: Baryogenesis (#51 Physics Checklist) FTD derives the baryon-to-photon ratio eta ~ 10^-10 from CP violation + Sakharov conditions. |
| [`campaign_nested_source_history_translation.cpp`](../../engine/tests/campaign_nested_source_history_translation.cpp) | 445 | @file campaign_nested_source_history_translation.cpp @brief FTD-0464 fixed local source-history translation, dressing on/off. |
| [`campaign_isolated_pair_action_reaction.cpp`](../../engine/tests/campaign_isolated_pair_action_reaction.cpp) | 442 | @file campaign_isolated_pair_action_reaction.cpp @brief FTD-0437 polarity/injection-order mirror for isolated pair mechanics. |
| [`test_canonical_precontact_mode_decay.cpp`](../../engine/tests/test_canonical_precontact_mode_decay.cpp) | 441 | FTD-0676: held-out canonical pre-contact mode-decay discriminator. |
| [`test_gpu_movement_transaction.cpp`](../../engine/tests/test_gpu_movement_transaction.cpp) | 439 | Exact greedy CUDA movement transaction regression. |
| [`test_boundary_chart_capacity.cpp`](../../engine/tests/test_boundary_chart_capacity.cpp) | 436 | FTD-0507: boundary collision chart-capacity correction. |
| [`campaign_thomson_tick_local_continuity.cpp`](../../engine/tests/campaign_thomson_tick_local_continuity.cpp) | 435 | FTD-0294: Source-free discrete tick local continuity. |
| [`campaign_vacuum_photon_scenario_diagnostic.cpp`](../../engine/tests/campaign_vacuum_photon_scenario_diagnostic.cpp) | 435 | @file campaign_vacuum_photon_scenario_diagnostic.cpp @brief FTD-0434 exact s0-vacuum-photon production-state diagnostic. |
| [`test_chart_contained_atomic_endpoint_solve.cpp`](../../engine/tests/test_chart_contained_atomic_endpoint_solve.cpp) | 435 | FTD-0538: chart-contained six-coordinate solve of the FTD-0536 action. |
| [`campaign_state_only_observer_discovery_cuda.cpp`](../../engine/tests/campaign_state_only_observer_discovery_cuda.cpp) | 433 | FTD-0754: discovery-only replay for the state-only field observer. |
| [`test_latency_field.cpp`](../../engine/tests/test_latency_field.cpp) | 433 | Test: Latency Field (Gravitational Potential) Validates the Poisson-based latency field implementation: ∇²φ_L = 4πG·ρ_mass → L = √(clamp(φ_L, 0, 0.998)) Sections: LAT-1: Single particle → latency >... |
| [`campaign_mass_gap.cpp`](../../engine/tests/campaign_mass_gap.cpp) | 432 | @file campaign_mass_gap.cpp @brief FTD-0270 closure swing (P2): the nonlinear-loop native mass gap. |
| [`test_lagrangian.cpp`](../../engine/tests/test_lagrangian.cpp) | 432 | Test: Lagrangian 2.0 — Zero Free Parameters via Master Quadratic Verifies that the engine dynamics are fully determined by G* and the master quadratic. |
| [`test_cell_measure_common_action_closure.cpp`](../../engine/tests/test_cell_measure_common_action_closure.cpp) | 431 | FTD-0649: cell-measure factors inside one reciprocal common action. |
| [`test_precontact_energetic_capture_delay.cpp`](../../engine/tests/test_precontact_energetic_capture_delay.cpp) | 430 | FTD-0737: precontact energetic-capture delay discriminator. |
| [`benchmark_alpha_window_lean_gpu.cu`](../../engine/tests/benchmark_alpha_window_lean_gpu.cu) | 428 | @file benchmark_alpha_window_lean_gpu.cu @brief Lean fixed-window Coulomb geometry benchmark. |
| [`test_higgs_mechanism.cpp`](../../engine/tests/test_higgs_mechanism.cpp) | 427 | Test: Higgs Mechanism from Manifestation Physics Checklist Item #37: The Higgs mechanism in FTD is understood as spontaneous symmetry breaking (SSB) via manifestation dynamics. |
| [`test_tick_phase_order.cpp`](../../engine/tests/test_tick_phase_order.cpp) | 427 | ============================================================================ test_tick_phase_order.cpp (ticket W4-2) ---------------------------------------------------------------------------- Reg... |
| [`campaign_sequential_no_reset_transactions.cpp`](../../engine/tests/campaign_sequential_no_reset_transactions.cpp) | 426 | @file campaign_sequential_no_reset_transactions.cpp @brief FTD-0459 repeated local transactions on the actual evolved field. |
| [`test_l33_full_impulse_rest_solve.cpp`](../../engine/tests/test_l33_full_impulse_rest_solve.cpp) | 426 | FTD-0708: solve the complete 48-coordinate rest residual at L=33 using the actual one-tick common-action impulses as the force vector. |
| [`campaign_central_field_momentum_recoil.cpp`](../../engine/tests/campaign_central_field_momentum_recoil.cpp) | 423 | @file campaign_central_field_momentum_recoil.cpp @brief FTD-0438 central-generator field-recoil audit. |
| [`test_cuda_quadratic_coat_orbit_gather.cpp`](../../engine/tests/test_cuda_quadratic_coat_orbit_gather.cpp) | 421 | FTD-0759: resident CUDA quadratic-coat orbit-gather parity. |
| [`test_connected_moore_block_common_action.cpp`](../../engine/tests/test_connected_moore_block_common_action.cpp) | 417 | FTD-0622: runtime-size connected Moore-block common action. |
| [`test_dyadic_recurrence_probe.cpp`](../../engine/tests/test_dyadic_recurrence_probe.cpp) | 417 | @file test_dyadic_recurrence_probe.cpp @brief [EXPLORATORY] Target-blind C3 geometry -> engine recurrence probe. |
| [`test_asymptotic_freedom.cpp`](../../engine/tests/test_asymptotic_freedom.cpp) | 415 | Test: Asymptotic Freedom (Checklist #34) Verifies that the QCD running coupling alpha_s(Q) exhibits asymptotic freedom both analytically (via the alpha_s_running / alpha_s_lattice formulas) and on... |
| [`campaign_sm_observables.cpp`](../../engine/tests/campaign_sm_observables.cpp) | 414 | Campaign: Standard Model Observables from Lattice Dynamics Computes 5 key SM observables directly from the FTD engine dynamics (not from plugged-in formulas). |
| [`test_multiscale_bridge.cpp`](../../engine/tests/test_multiscale_bridge.cpp) | 414 | Test: Multi-Scale Bridge (13 unit checks) Covers quantum number preservation, position round-trips, OnticEntity consistency, multi-nuclei clustering, edge cases, and energy budget across Scale 0 ↔... |
| [`benchmark_alpha_convergence.cpp`](../../engine/tests/benchmark_alpha_convergence.cpp) | 413 | @file benchmark_alpha_convergence.cpp @brief α_eff continuum-limit convergence study at high SOR precision. |
| [`campaign_minimum_norm_transaction_selector.cpp`](../../engine/tests/campaign_minimum_norm_transaction_selector.cpp) | 413 | @file campaign_minimum_norm_transaction_selector.cpp @brief FTD-0458 unique cubic-covariant minimum-norm local selector. |
| [`test_conservation_profile.cpp`](../../engine/tests/test_conservation_profile.cpp) | 412 | ============================================================================ test_conservation_profile.cpp (engine-flawless audit, 2026-06-01) ------------------------------------------------------... |
| [`campaign_force_branch_reciprocity.cpp`](../../engine/tests/campaign_force_branch_reciprocity.cpp) | 411 | @file campaign_force_branch_reciprocity.cpp @brief FTD-0439 identical-pair reciprocity matrix for existing force modes. |
| [`test_native_dual_half_shell.cpp`](../../engine/tests/test_native_dual_half_shell.cpp) | 407 | Native dual half-shell audit. |
| [`campaign_link_action_work_compatibility.cpp`](../../engine/tests/campaign_link_action_work_compatibility.cpp) | 406 | @file campaign_link_action_work_compatibility.cpp @brief FTD-0470 exact finite-link work versus centered site-gradient force. |
| [`internal_excitation_symmetry_ray_spectrum_hook.h`](../../engine/tests/internal_excitation_symmetry_ray_spectrum_hook.h) | 406 | FTD-0698 hook for test_causal_excitation_separation_v1.cpp. |
| [`campaign_single_action_reciprocity.cpp`](../../engine/tests/campaign_single_action_reciprocity.cpp) | 405 | @file campaign_single_action_reciprocity.cpp @brief FTD-0467 common-action source/force reciprocity audit. |
| [`test_coupled_quartic_clock_field.cpp`](../../engine/tests/test_coupled_quartic_clock_field.cpp) | 404 | FTD-0770: Coupled Quartic Clock Field v1 selected-extension verifier. |
| [`campaign_shell_predictions.cpp`](../../engine/tests/campaign_shell_predictions.cpp) | 403 | Campaign: Self-Field Shell Predictions (High-Precision) Tests two structural predictions about the electron's self-field: Prediction 1: E_field / K_B^2 = 16 * alpha Prediction 2: r_eff / r_shell =... |
| [`test_bivector_closure.cpp`](../../engine/tests/test_bivector_closure.cpp) | 403 | @file test_bivector_closure.cpp @brief Program F-double-prime — closure tests for the plaquette bivector algebra detected in Program F-prime (FTD-0086). |
| [`test_cell_measure_fixed_mass_refinement.cpp`](../../engine/tests/test_cell_measure_fixed_mass_refinement.cpp) | 403 | FTD-0648: observer-only cell-measure fixed-mass refinement scaling. |
| [`campaign_bcc_band_spectrum.cpp`](../../engine/tests/campaign_bcc_band_spectrum.cpp) | 402 | @file campaign_bcc_band_spectrum.cpp @brief BCC sub-stencil two-state spectrum campaign — smoke test. |
| [`campaign_dynamical_flux_dressing.cpp`](../../engine/tests/campaign_dynamical_flux_dressing.cpp) | 402 | @file campaign_dynamical_flux_dressing.cpp @brief FTD-0476 source-built dressing / movement / release campaign v2. |
| [`test_atom_engine.cpp`](../../engine/tests/test_atom_engine.cpp) | 400 | Test: AtomEngine (Scale 2) unit tests Checks covering injection, properties, forces, bonding, conservation laws, and integration. |
| [`test_connected_moore_block_repeated_dynamics.cpp`](../../engine/tests/test_connected_moore_block_repeated_dynamics.cpp) | 399 | FTD-0623: repeated rest/boost dynamics of the connected w=2 integer object. |
| [`test_cuda_transported_chart_morphology.cpp`](../../engine/tests/test_cuda_transported_chart_morphology.cpp) | 398 | FTD-0764: transported-chart morphology CPU/CUDA qualification. |
| [`test_gpu_identity_lifecycle_parity.cpp`](../../engine/tests/test_gpu_identity_lifecycle_parity.cpp) | 396 | Focused CPU/CUDA parity for sparse lifecycle transactions and identity allocation. |
| [`test_cell_measure_long_horizon_transport_v2.cpp`](../../engine/tests/test_cell_measure_long_horizon_transport_v2.cpp) | 395 | FTD-0652: checkpointed cached-solver execution of the FTD-0650 physics gate. |
| [`test_edge_plane_one_sided_variation.cpp`](../../engine/tests/test_edge_plane_one_sided_variation.cpp) | 395 | FTD-0539: in-plane roots and one-sided normal variation. |
| [`test_localized_basin_relaxation_v2.cpp`](../../engine/tests/test_localized_basin_relaxation_v2.cpp) | 394 | FTD-0679: corrected localized-basin relaxation discriminator. |
| [`campaign_travelling_wave_recoil_threshold.cpp`](../../engine/tests/campaign_travelling_wave_recoil_threshold.cpp) | 392 | @file campaign_travelling_wave_recoil_threshold.cpp @brief FTD-0455 exact travelling-wave recoil-capacity threshold. |
| [`test_bivector_closure_v2.cpp`](../../engine/tests/test_bivector_closure_v2.cpp) | 392 | @file test_bivector_closure_v2.cpp @brief M2 (FTD-0380) — noise-controlled bivector closure re-test. |
| [`test_cuda_fractional_center_state_only_observer.cpp`](../../engine/tests/test_cuda_fractional_center_state_only_observer.cpp) | 392 | FTD-0763: fractional-center state-only observer CPU/CUDA qualification. |
| [`campaign_symmetric_half_tick_energy.cpp`](../../engine/tests/campaign_symmetric_half_tick_energy.cpp) | 387 | @file campaign_symmetric_half_tick_energy.cpp @brief FTD-0469 symmetric half-tick transaction energy/momentum/reversal gate for the common-action kick pair (parent FTD-0468). |
| [`campaign_emergent_static_charge.cpp`](../../engine/tests/campaign_emergent_static_charge.cpp) | 384 | @file campaign_emergent_static_charge.cpp @brief FTD-0426 polarity-sourced static-charge discriminator. |
| [`test_dual_substrate.cpp`](../../engine/tests/test_dual_substrate.cpp) | 384 | Test: Dual-Substrate Engine Verifies the dual-substrate implementation from "The Algebraic Identity of Two Substrates" (Montanez & Claude, 2026). |
| [`test_wz_mass.cpp`](../../engine/tests/test_wz_mass.cpp) | 383 | Test: W/Z Mass Generation from Chirality Gap in Dual Substrate Physics Checklist Item #36 In dual-substrate mode, flux splits into left-handed (J_L) and right-handed (J_R) components. |
| [`test_flavor_physics.cpp`](../../engine/tests/test_flavor_physics.cpp) | 382 | Test: Flavor Physics — CKM/PMNS from Lattice (Checklist #40) FTD derives PMNS neutrino mixing angles and CKM parameters from framework integers {3, 4, 7, 13}. |
| [`test_gpu_native_extension_parity.cpp`](../../engine/tests/test_gpu_native_extension_parity.cpp) | 382 | Focused CPU/CUDA parity gates for native Scale-0 extension phases that sit outside the historical six-phase CUDA core: EP-1 EW background drive executes before phase_read and enters L/R. |
| [`test_particle_lifetime.cpp`](../../engine/tests/test_particle_lifetime.cpp) | 380 | Diagnostic: Particle Lifetime & Energy Loss Probes three issues: PL1: Slow particle survival (v=0.01, 0.02 — were evaporating) PL2: Energy loss rate vs velocity (quantify radiation) PL3: Orbital en... |
| [`campaign_beta_measurement.cpp`](../../engine/tests/campaign_beta_measurement.cpp) | 379 | @file campaign_beta_measurement.cpp @brief β-function measurement at non-zero temperature — smoke test. |
| [`campaign_local_coat_injectivity_momentum.cpp`](../../engine/tests/campaign_local_coat_injectivity_momentum.cpp) | 379 | @file campaign_local_coat_injectivity_momentum.cpp @brief FTD-0465 injectivity and momentum audit of FTD-0464's R1 event. |
| [`campaign_fractional_center_dressing_observer_cuda.cpp`](../../engine/tests/campaign_fractional_center_dressing_observer_cuda.cpp) | 378 | FTD-0763: untouched CUDA replay through the fractional-center observer. |
| [`test_repeated_exact_root_acceleration.cpp`](../../engine/tests/test_repeated_exact_root_acceleration.cpp) | 378 | FTD-0651: qualify repeated exact-root acceleration without changing physics. |
| [`campaign_face_flux_observer_qualification.cpp`](../../engine/tests/campaign_face_flux_observer_qualification.cpp) | 377 | @file campaign_face_flux_observer_qualification.cpp @brief FTD-0480 observer-only qualification of the FTD-0478/0479 records. |
| [`test_multibody_shape_observability.cpp`](../../engine/tests/test_multibody_shape_observability.cpp) | 377 | FTD-0501: multibody kernel of additive trilinear shape/current. |
| [`test_vortex.cpp`](../../engine/tests/test_vortex.cpp) | 377 | Test: Vortex Formation — Biot-Savart Feedback Loop Explores whether the Biot-Savart coupling (curl of charge current) creates stable vortex structures when an electron has initial tangential veloci... |
| [`test_worldline_current_kernel.cpp`](../../engine/tests/test_worldline_current_kernel.cpp) | 377 | FTD-0502: endpoint multiset versus divergence-free worldline current. |
| [`test_implicit_atomic_endpoint_solve.cpp`](../../engine/tests/test_implicit_atomic_endpoint_solve.cpp) | 376 | FTD-0537: six-coordinate stationary solve of the FTD-0536 action. |
| [`test_clifford_multigrade.cpp`](../../engine/tests/test_clifford_multigrade.cpp) | 375 | @file test_clifford_multigrade.cpp @brief Path 1 — Wilson-loop-style multi-grade decomposition. |
| [`test_gpu_continuity_ledger.cpp`](../../engine/tests/test_gpu_continuity_ledger.cpp) | 375 | GPU-native continuity ledger parity. |
| [`test_larmor.cpp`](../../engine/tests/test_larmor.cpp) | 374 | Test: Larmor Radiation (Acceleration-Dependent Damping) When the larmor_radiation toggle is ON, damping at manifested sites is modulated by the particle's acceleration: larmor_mod = min(1, LARMOR_F... |
| [`test_cuda_matched_field_pipeline.cpp`](../../engine/tests/test_cuda_matched_field_pipeline.cpp) | 373 | CUDA qualification for the matched face/edge field and regional observer. |
| [`test_multicell_worldline_variation.cpp`](../../engine/tests/test_multicell_worldline_variation.cpp) | 369 | FTD-0533: complete deposited-action variation through internal knots. |
| [`test_qualified_interior_compact_matter_transport.cpp`](../../engine/tests/test_qualified_interior_compact_matter_transport.cpp) | 369 | FTD-0608: autonomous transport from a preregistered qualified interior core. |
| [`campaign_light_deflection.cpp`](../../engine/tests/campaign_light_deflection.cpp) | 368 | @file campaign_light_deflection.cpp @brief Gate 2 — the gravitational-optical channel: does the substrate bend light? |
| [`test_refined_single_core_directional_boost.cpp`](../../engine/tests/test_refined_single_core_directional_boost.cpp) | 367 | FTD-0613: directional boosts from the refined compact rest state. |
| [`campaign_rigid_source_history_translation.cpp`](../../engine/tests/campaign_rigid_source_history_translation.cpp) | 366 | @file campaign_rigid_source_history_translation.cpp @brief FTD-0462 rigid source-history translation versus production carry. |
| [`campaign_born_regime_map.cpp`](../../engine/tests/campaign_born_regime_map.cpp) | 365 | @file campaign_born_regime_map.cpp @brief Engine-side Born regime map (temporal-interior front T3, FTD-0200 path): does the mechanism-level regime law — Born-fraction rising with Omega*tau — transf... |
| [`campaign_m3_fixed_chart_parent_qualification_cuda.cpp`](../../engine/tests/campaign_m3_fixed_chart_parent_qualification_cuda.cpp) | 364 | FTD-0757: fixed integer-chart qualification of the M3 parent replay. |
| [`test_link_bilinear_clifford.cpp`](../../engine/tests/test_link_bilinear_clifford.cpp) | 363 | @file test_link_bilinear_clifford.cpp @brief Program F — link-bilinear fermion probe. |
| [`campaign_m3_device_resident_pipeline_parity_cuda.cpp`](../../engine/tests/campaign_m3_device_resident_pipeline_parity_cuda.cpp) | 361 | FTD-0759: frozen small-volume device-resident parity matrix. |
| [`campaign_native_dynamic_polarity_response.cpp`](../../engine/tests/campaign_native_dynamic_polarity_response.cpp) | 360 | @file campaign_native_dynamic_polarity_response.cpp @brief FTD-0429 native long-wavelength polarity-response campaign. |
| [`test_plaquette_bivector_clifford.cpp`](../../engine/tests/test_plaquette_bivector_clifford.cpp) | 358 | @file test_plaquette_bivector_clifford.cpp @brief Program F-prime — plaquette bivector probe. |
| [`test_subcell_representation_quotient.cpp`](../../engine/tests/test_subcell_representation_quotient.cpp) | 357 | FTD-0498: quotient factorization versus production anchor dependence. |
| [`test_action_stationarity.cpp`](../../engine/tests/test_action_stationarity.cpp) | 355 | Test: Field Action Stationarity and Production-Force Replay Verifies the field-sector action identities and independently replays the selected production-force formulas. |
| [`test_internal_mode_action_transfer_v2.cpp`](../../engine/tests/test_internal_mode_action_transfer_v2.cpp) | 355 | FTD-0661: tight-frame covariance and observer-floor correction of FTD-0660. |
| [`test_boundary_scenario_physics.cpp`](../../engine/tests/test_boundary_scenario_physics.cpp) | 354 | Quantitative certification for the public Scale-0 finite-box boundary probe. |
| [`campaign_canonical_current_horizon_cuda.cpp`](../../engine/tests/campaign_canonical_current_horizon_cuda.cpp) | 353 | FTD-0748 candidate: canonical net-current CUDA horizon successor. |
| [`test_native_engine_transport_flow.cpp`](../../engine/tests/test_native_engine_transport_flow.cpp) | 353 | Native engine transport-history flow audit. |
| [`test_overshoot_preserving_contact_rebase.cpp`](../../engine/tests/test_overshoot_preserving_contact_rebase.cpp) | 352 | FTD-0527: quotient-correct overshoot rebase and raw inverse audit. |
| [`campaign_local_support_recoil_threshold.cpp`](../../engine/tests/campaign_local_support_recoil_threshold.cpp) | 351 | @file campaign_local_support_recoil_threshold.cpp @brief FTD-0456 fixed-radius travelling-wave recoil thresholds. |
| [`test_complete_moving_dressing_relative_orbit.cpp`](../../engine/tests/test_complete_moving_dressing_relative_orbit.cpp) | 350 | FTD-0706: test whether the selected static dressing plus a uniform v=1/2 constituent boost is already a complete relative-periodic moving state. |
| [`test_gauss.cpp`](../../engine/tests/test_gauss.cpp) | 346 | Test: Gauss constraint (consolidated suite) Merges 2 legacy tests into test_gauss.cpp (self-ref target): test_gauss -> section "gauss_structure" (16 checks) test_gauss_convergence -> section "gauss... |
| [`campaign_injective_local_permutation_event.cpp`](../../engine/tests/campaign_injective_local_permutation_event.cpp) | 345 | @file campaign_injective_local_permutation_event.cpp @brief FTD-0466 injective 36-site cyclic-permutation event control. |
| [`test_polarity_snapshot_current_nonuniqueness.cpp`](../../engine/tests/test_polarity_snapshot_current_nonuniqueness.cpp) | 344 | FTD-0719: an unordered polarity snapshot fixes div(J), not the cycle current. |
| [`campaign_matched_gauss_transport.cpp`](../../engine/tests/campaign_matched_gauss_transport.cpp) | 343 | @file campaign_matched_gauss_transport.cpp @brief FTD-0427 projection-free matched-current campaign. |
| [`test_quadratic_coat_face_current.cpp`](../../engine/tests/test_quadratic_coat_face_current.cpp) | 343 | FTD-0541: smooth positive coat, exact face current, and C1 plane gate. |
| [`test_em_energy_conservation.cpp`](../../engine/tests/test_em_energy_conservation.cpp) | 340 | Test: EM Energy Conservation in Undamped Vacuum Verifies that total electromagnetic energy is conserved when there are no particles, no coupling, and no damping — pure wave equation dynamics. |
| [`test_wilson_dirac_gauge.cpp`](../../engine/tests/test_wilson_dirac_gauge.cpp) | 340 | Wilson-Dirac gauge-link verification (Phase II.2-C milestone). |
| [`campaign_neutrino_sector.cpp`](../../engine/tests/campaign_neutrino_sector.cpp) | 339 | Campaign: Neutrino Sector Verification (Phase 8 — Particle Zoo) Verifies the complete neutrino sector derived from framework integers {3, 4, 7, 13}. |
| [`campaign_thermal_ignition.cpp`](../../engine/tests/campaign_thermal_ignition.cpp) | 339 | @file campaign_thermal_ignition.cpp @brief FTD-0274 scout — min/max temperature + ignition/detonation map of the lattice. |
| [`test_interacting_common_action_root_multiseed.cpp`](../../engine/tests/test_interacting_common_action_root_multiseed.cpp) | 339 | FTD-0720: deterministic multiseed probe of the interacting common-action root. |
| [`test_quadratic_coat_spacetime_action.cpp`](../../engine/tests/test_quadratic_coat_spacetime_action.cpp) | 339 | FTD-0542: spacetime current and gauge action of the quadratic coupling coat. |
| [`campaign_native_reaction_polarity_slow_mode.cpp`](../../engine/tests/campaign_native_reaction_polarity_slow_mode.cpp) | 336 | @file campaign_native_reaction_polarity_slow_mode.cpp @brief FTD-0431 reaction-aware polarity decay campaign. |
| [`campaign_paired_jw_recoil_capacity.cpp`](../../engine/tests/campaign_paired_jw_recoil_capacity.cpp) | 336 | @file campaign_paired_jw_recoil_capacity.cpp @brief FTD-0454 simultaneous J/W recoil-capacity gate. |
| [`test_spacetime_worldline_coupling.cpp`](../../engine/tests/test_spacetime_worldline_coupling.cpp) | 336 | FTD-0484: exact spacetime worldline current and gauge-endpoint identity. |
| [`test_measurement.cpp`](../../engine/tests/test_measurement.cpp) | 335 | Test: Measurement = Manifestation Validation Validates that observer coupling (manifested structure s != 0) triggers wave function localization (collapse = manifestation): MEAS-1: Without observer... |
| [`campaign_thomson_recoil_observatory.cpp`](../../engine/tests/campaign_thomson_recoil_observatory.cpp) | 334 | FTD-0287: Thomson recoil observatory. |
| [`campaign_thomson_tick_invariant_v2.cpp`](../../engine/tests/campaign_thomson_tick_invariant_v2.cpp) | 333 | FTD-0293: Source-free discrete tick energy invariant, precision v2. |
| [`campaign_matched_face_momentum_transaction.cpp`](../../engine/tests/campaign_matched_face_momentum_transaction.cpp) | 332 | @file campaign_matched_face_momentum_transaction.cpp @brief FTD-0473 matched local pseudomomentum and hop-recoil gate. |
| [`campaign_halo_forcedness.cpp`](../../engine/tests/campaign_halo_forcedness.cpp) | 331 | @file campaign_halo_forcedness.cpp @brief FTD-0300: is the single-particle self-field HALO EXPONENT forced by the dynamics, or tuned by engine calibration constants? |
| [`test_stress_energy.cpp`](../../engine/tests/test_stress_energy.cpp) | 331 | Test: Stress-Energy Tensor T_mu_nu From Noether's theorem applied to the FTD wave equation, the stress-energy tensor components are: T^00 = (1/2)\|wave_vel\|^2 + (1/2)*C^2*sum_neighbors\|J_n - J_c\|^2/... |
| [`benchmark_dirac_electron_in_B.cpp`](../../engine/tests/benchmark_dirac_electron_in_B.cpp) | 330 | Single-electron stable orbit in uniform B (Phase II.3 milestone). |
| [`campaign_native_dressed_hazard_ir_scaling.cpp`](../../engine/tests/campaign_native_dressed_hazard_ir_scaling.cpp) | 329 | @file campaign_native_dressed_hazard_ir_scaling.cpp @brief FTD-0433 pole-phased dressed evaporation-hazard scaling campaign. |
| [`campaign_native_dressed_evaporation_hazard.cpp`](../../engine/tests/campaign_native_dressed_evaporation_hazard.cpp) | 328 | @file campaign_native_dressed_evaporation_hazard.cpp @brief FTD-0432 exact conditional evaporation-hazard campaign. |
| [`campaign_poisson_cold_start_memory.cpp`](../../engine/tests/campaign_poisson_cold_start_memory.cpp) | 327 | @file campaign_poisson_cold_start_memory.cpp @brief FTD-0441 matched cold/pre-relaxed Poisson trajectory replay. |
| [`test_branch_holonomy_gap.cpp`](../../engine/tests/test_branch_holonomy_gap.cpp) | 327 | test_branch_holonomy_gap.cpp — verifies the Z_2 torus branch-twist gap λ_min = 4 sin²( π / (2N) ) (eq. |
| [`test_hard_contact_corner_action.cpp`](../../engine/tests/test_hard_contact_corner_action.cpp) | 326 | FTD-0516: selected relativistic hard-contact corner action. |
| [`test_born_rule_ensemble.cpp`](../../engine/tests/test_born_rule_ensemble.cpp) | 325 | Test: Born Rule Ensemble — Multi-site \|psi\|^2 Distribution Validation Enhances Born rule testing beyond single-site genesis to validate the full multi-site probability distribution: BORN-1: Gaussia... |
| [`test_momentum_selected_worldline_matching.cpp`](../../engine/tests/test_momentum_selected_worldline_matching.cpp) | 325 | FTD-0503: phase-space selection of the free multibody current 1-chain. |
| [`campaign_kinetic_drain_curl_isolation.cpp`](../../engine/tests/campaign_kinetic_drain_curl_isolation.cpp) | 323 | @file campaign_kinetic_drain_curl_isolation.cpp @brief Isolating the transverse-contamination mechanism: is it the genesis kinetic-drain operation? |
| [`test_free_flux_localization.cpp`](../../engine/tests/test_free_flux_localization.cpp) | 323 | FTD-0557: free-flux localization obstruction. |
| [`campaign_hedgehog_charge_robustness.cpp`](../../engine/tests/campaign_hedgehog_charge_robustness.cpp) | 320 | @file campaign_hedgehog_charge_robustness.cpp @brief Is the hedgehog topological charge of the flux field robust across birth circumstances that produced a 9.2x energy spread? |
| [`campaign_matched_maxwell_integration.cpp`](../../engine/tests/campaign_matched_maxwell_integration.cpp) | 320 | FTD-0428 run-of-record campaign: integrated matched Maxwell/Gauss branch. |
| [`benchmark_dynamical_sm.cpp`](../../engine/tests/benchmark_dynamical_sm.cpp) | 319 | @file benchmark_dynamical_sm.cpp @brief EFT Phase 4 — dynamical SM emergence tests. |
| [`campaign_multiscale_pipeline.cpp`](../../engine/tests/campaign_multiscale_pipeline.cpp) | 317 | Campaign: Multi-Scale Pipeline (12 checks across 4 phases) Phase 1: Full pipeline round-trip (Scale 0 → 1 → 2 → 1 → 0) Phase 2: Energy conservation across transitions Phase 3: Multi-atom pipeline (... |
| [`test_slow_envelope_live_newton.cpp`](../../engine/tests/test_slow_envelope_live_newton.cpp) | 317 | ============================================================================ test_slow_envelope_live_newton.cpp ---------------------------------------------------------------------------- FTD-1022... |
| [`campaign_m3_parent_replay_forensics_cuda.cpp`](../../engine/tests/campaign_m3_parent_replay_forensics_cuda.cpp) | 315 | FTD-0756: read-only forensics for the FTD-0755 parent replay failure. |
| [`test_centered_fiber_knot_transaction.cpp`](../../engine/tests/test_centered_fiber_knot_transaction.cpp) | 315 | FTD-0496: unique centered knot-to-subcell fiber transaction. |
| [`test_continuity.cpp`](../../engine/tests/test_continuity.cpp) | 315 | Test: Charge Continuity Equation Verifies that total electric charge Q = sum(state) is exactly conserved through all dynamics: static, dynamic, annihilation, genesis. |
| [`test_causal_normalization.cpp`](../../engine/tests/test_causal_normalization.cpp) | 314 | FTD-0402 exact causal-normalization and mass-role contract. |
| [`test_gpu_parity.cpp`](../../engine/tests/test_gpu_parity.cpp) | 314 | GPU vs CPU parity tests for the FTD CUDA engine. |
| [`campaign_gravity_profile.cpp`](../../engine/tests/campaign_gravity_profile.cpp) | 313 | Campaign: Gravitational Density Profile (Phase 7 — Gravitational Sector) Tests the radial density profile around a static massive object. |
| [`test_connected_block_fixed_mass_refinement_obstruction.cpp`](../../engine/tests/test_connected_block_fixed_mass_refinement_obstruction.cpp) | 313 | FTD-0647: frozen-coefficient fixed-mass refinement obstruction. |
| [`test_cuda_state_only_support_ladder.cpp`](../../engine/tests/test_cuda_state_only_support_ladder.cpp) | 313 | FTD-0759: CUDA state-only support-ladder reduction parity. |
| [`test_telemetry.cpp`](../../engine/tests/support/test_telemetry.cpp) | 312 | ============================================================================ tests/support/test_telemetry.cpp ---------------------------------------------------------------------------- Implementa... |
| [`test_matched_symmetry_ray_spectrum.cpp`](../../engine/tests/test_matched_symmetry_ray_spectrum.cpp) | 311 | FTD-0696: carrier-aware matched-field symmetry-ray spectrum observer. |
| [`test_strong_stress_energy_contract.cpp`](../../engine/tests/test_strong_stress_energy_contract.cpp) | 310 | FTD-0406 frozen owner-authorized strong stress-energy CPU contract. |
| [`test_live_sourced_newton.cpp`](../../engine/tests/test_live_sourced_newton.cpp) | 309 | ============================================================================ test_live_sourced_newton.cpp ---------------------------------------------------------------------------- FTD-1021 / PRE... |
| [`test_gpu_graph_capture.cpp`](../../engine/tests/test_gpu_graph_capture.cpp) | 308 | ============================================================================ test_gpu_graph_capture.cpp — graph replay must be BIT-IDENTICAL to direct launch, not merely close. |
| [`test_frozen_well_characteristic_deflection.cpp`](../../engine/tests/test_frozen_well_characteristic_deflection.cpp) | 307 | ============================================================================ test_frozen_well_characteristic_deflection.cpp -------------------------------------------------------------------------... |
| [`test_native_dual_cell_gauss.cpp`](../../engine/tests/test_native_dual_cell_gauss.cpp) | 306 | Native dual-cell Gauss audit. |
| [`test_ternary_block_bipole_peierls_scaling.cpp`](../../engine/tests/test_ternary_block_bipole_peierls_scaling.cpp) | 306 | FTD-0621: exact ternary block-bipole Peierls scaling. |
| [`test_quadratic_coat_orbit_gather.cpp`](../../engine/tests/test_quadratic_coat_orbit_gather.cpp) | 305 | FTD-0550: quadratic-coat adjoint orbit gather and commuting curl. |
| [`test_soliton_sweeps.cpp`](../../engine/tests/test_soliton_sweeps.cpp) | 305 | Test: Soliton Emergence and Sweeps Campaign (Class B Track 2) Performs automated sweeps over amplitudes, seeds, and toggle configs using the triplet metric (n_total, centroid_drift, rms_radius) to... |
| [`test_gpu_shell_battery.cpp`](../../engine/tests/test_gpu_shell_battery.cpp) | 302 | GPU Shell Battery — Understanding Self-Field Dynamics Runs multiple configurations at 128^3 to understand how the electron's self-field depends on: 1. |
| [`test_audit_regression.cpp`](../../engine/tests/test_audit_regression.cpp) | 301 | ============================================================================ test_audit_regression.cpp ---------------------------------------------------------------------------- Focused regressio... |
| [`campaign_einstein.cpp`](../../engine/tests/campaign_einstein.cpp) | 300 | Campaign: Einstein — Relativistic and Gravitational Tests Three tests probing energy conservation, Lorentz contraction, and gravitational redshift in the FTD engine: E1: Energy Conservation — 3-par... |
| [`campaign_wave_sectors.cpp`](../../engine/tests/campaign_wave_sectors.cpp) | 300 | Campaign: Wave Sectors (FTD-0299) [hardened v2 after adversarial pre-reg review] Arm 1 (--arm=light): light-sector dispersion atlas. |
| [`test_closed_negatives.cpp`](../../engine/tests/test_closed_negatives.cpp) | 299 | @file test_closed_negatives.cpp @brief Regression guards for closed-negative ledger claims. |
| [`test_constituent_relative_collision.cpp`](../../engine/tests/test_constituent_relative_collision.cpp) | 299 | FTD-0512: constituent-relative selected collision and face-kernel audit. |
| [`test_poynting.cpp`](../../engine/tests/test_poynting.cpp) | 298 | Test: Poynting Vector S = E x B Verifies the Poynting vector diagnostic API, which gives the direction and magnitude of electromagnetic energy flow. |
| [`test_thermodynamics.cpp`](../../engine/tests/test_thermodynamics.cpp) | 296 | Test: Thermodynamics Verifies thermodynamic properties of the FTD lattice: 1. |
| [`test_flux_mediated.cpp`](../../engine/tests/test_flux_mediated.cpp) | 295 | Test: Flux-Mediated Force — 1/r² from Field Dynamics Verifies that the coupling term in the Lagrangian: L_coupling = -g_c * s * (div J) produces a self-consistent flux field around charged particle... |
| [`campaign_thomson_tick_invariant.cpp`](../../engine/tests/campaign_thomson_tick_invariant.cpp) | 294 | FTD-0292: Source-free discrete tick energy invariant. |
| [`test_constituent_stress_moment.cpp`](../../engine/tests/test_constituent_stress_moment.cpp) | 294 | FTD-0513: minimal constituent kinetic-stress lift. |
| [`test_localized_basin_relaxation_v3.cpp`](../../engine/tests/test_localized_basin_relaxation_v3.cpp) | 293 | FTD-0681: corrected-output replication of localized-basin relaxation. |
| [`campaign_discrete_interaction_work_contract.cpp`](../../engine/tests/campaign_discrete_interaction_work_contract.cpp) | 292 | @file campaign_discrete_interaction_work_contract.cpp @brief FTD-0443 exact hop-work and production-force contract audit. |
| [`test_spin_statistics.cpp`](../../engine/tests/test_spin_statistics.cpp) | 292 | Test: Spin-Statistics (720 degree periodicity) Verifies that framed flux exhibits spinor behavior: SPIN-1: 360 degree rotation inverts framed flux sign SPIN-2: 720 degree rotation returns to origin... |
| [`test_native_charge_gate.cpp`](../../engine/tests/test_native_charge_gate.cpp) | 291 | FTD native conserved-charge gate. |
| [`campaign_matched_face_energy_transaction.cpp`](../../engine/tests/campaign_matched_face_energy_transaction.cpp) | 290 | @file campaign_matched_face_energy_transaction.cpp @brief FTD-0472 exact face-current energy and Moore-route ambiguity. |
| [`test_falsifiability.cpp`](../../engine/tests/test_falsifiability.cpp) | 288 | Falsifiability Tests: Negative-Result Validation PURPOSE: Demonstrate that FTD is CONSTRAINED, not arbitrary. |
| [`campaign_cluster_energy_spectroscopy.cpp`](../../engine/tests/campaign_cluster_energy_spectroscopy.cpp) | 287 | @file campaign_cluster_energy_spectroscopy.cpp @brief FTD-0273 Phase 1 — mass as flux-energy in flip-quanta. |
| [`campaign_flux_slice_propagation.cpp`](../../engine/tests/campaign_flux_slice_propagation.cpp) | 286 | @file campaign_flux_slice_propagation.cpp @brief 2D flux-slice diagnostic for wave-propagation isotropy. |
| [`test_ladder_walk_from_oh.cpp`](../../engine/tests/test_ladder_walk_from_oh.cpp) | 285 | @file test_ladder_walk_from_oh.cpp @brief Program A (partial closure) — derive ladder-walk step-size multiset from O_h structure. |
| [`test_two_slab_variational_force.cpp`](../../engine/tests/test_two_slab_variational_force.cpp) | 284 | FTD-0485: two-slab common-action force and threshold differentiability. |
| [`benchmark_budget_equation.cpp`](../../engine/tests/benchmark_budget_equation.cpp) | 283 | BUDGET EQUATION EXPERIMENT Tests: x/K + G_star/x = 1 The budget equation says the coupling x partitions between two phases: Coulomb (deconfined): fraction = x/K ~ 0.978 Confined: fraction = G_star/... |
| [`campaign_parity_violation.cpp`](../../engine/tests/campaign_parity_violation.cpp) | 283 | Campaign: Parity Violation (Phase 6 — Weak Sector & SU(2)) Tests that weak transmutation in dual-substrate mode is state-asymmetric because it reads only the left register. |
| [`test_quadratic_coat_self_force.cpp`](../../engine/tests/test_quadratic_coat_self_force.cpp) | 283 | FTD-0552: isolated quadratic-coat self-force discriminator. |
| [`test_confinement.cpp`](../../engine/tests/test_confinement.cpp) | 282 | Test: Strong Force Confinement Dynamics Verifies flux-tube based confinement model: CONF-1: Two color charges separated by r feel constant force at large r CONF-2: Force weakens at short range (asy... |
| [`test_implicit_atomic_face_action.cpp`](../../engine/tests/test_implicit_atomic_face_action.cpp) | 281 | FTD-0536: minimal implicit face action versus FTD-0531 scalar roots. |
| [`test_inflation.cpp`](../../engine/tests/test_inflation.cpp) | 281 | Test: Inflation (Sub-Threshold Flux Dynamics) Verifies that high-density uniform flux undergoes dynamics consistent with inflationary cosmology: exponential energy growth, approximately scale-invar... |
| [`campaign_genesis_energy_ledger.cpp`](../../engine/tests/campaign_genesis_energy_ledger.cpp) | 280 | @file campaign_genesis_energy_ledger.cpp @brief Does a REAL (genesis-created) manifested charge lock the SAME constraint self-energy W_SC(L) that a SYNTHETIC unit charge does? |
| [`test_one_well_redshift_falling.cpp`](../../engine/tests/test_one_well_redshift_falling.cpp) | 280 | ============================================================================ test_one_well_redshift_falling.cpp ---------------------------------------------------------------------------- FTD-1019... |
| [`campaign_two_clock_consistency.cpp`](../../engine/tests/campaign_two_clock_consistency.cpp) | 279 | @file campaign_two_clock_consistency.cpp @brief Does the substrate's DECAY clock dilate like its PROPER-TIME clock? |
| [`test_finite_memory_reversible_lift.cpp`](../../engine/tests/test_finite_memory_reversible_lift.cpp) | 279 | FTD-0499: finite-fiber reversible-lift obstruction. |
| [`test_generation_graph.cpp`](../../engine/tests/test_generation_graph.cpp) | 278 | test_generation_graph.cpp — Γ_F(d) [CANDIDATE RECONSTRUCTION] diagnostic. |
| [`test_quadratic_coat_discrete_gradient_transaction.cpp`](../../engine/tests/test_quadratic_coat_discrete_gradient_transaction.cpp) | 278 | FTD-0551: quadratic-coat reciprocal discrete-gradient transaction. |
| [`test_extended_source_peierls_scaling.cpp`](../../engine/tests/test_extended_source_peierls_scaling.cpp) | 277 | FTD-0555: extended local-source Peierls scaling. |
| [`test_period_three_comoving_field_source.cpp`](../../engine/tests/test_period_three_comoving_field_source.cpp) | 277 | FTD-0716 source side: deposit the locked FTD-0715 three-phase trajectory and write the exact affine RHS for the translated three-tick field equation. |
| [`test_render_bridge_golden.cpp`](../../engine/tests/test_render_bridge_golden.cpp) | 275 | ============================================================================ test_render_bridge_golden.cpp ---------------------------------------------------------------------------- Phase 4 PRE-F... |
| [`campaign_annihilation_angular.cpp`](../../engine/tests/campaign_annihilation_angular.cpp) | 274 | Campaign: e+e- Annihilation Angular Distribution (QED Scattering) — GPU Validates that the FTD lattice produces the expected angular distribution for e+e- -> gamma gamma annihilation radiation. |
| [`campaign_blocked_hop_work_decomposition.cpp`](../../engine/tests/campaign_blocked_hop_work_decomposition.cpp) | 274 | @file campaign_blocked_hop_work_decomposition.cpp @brief FTD-0460 exact additive decomposition of the FTD-0459 blocked work. |
| [`test_face_current_segment.cpp`](../../engine/tests/test_face_current_segment.cpp) | 274 | Focused continuity, locality, and cubic-covariance tests for exact straight-segment face-current deposition. |
| [`test_removal_time_pulse_bound.cpp`](../../engine/tests/test_removal_time_pulse_bound.cpp) | 274 | FTD-0589: exact rectangular-pulse and arbitrary-removal bound. |
| [`test_wavepacket.cpp`](../../engine/tests/test_wavepacket.cpp) | 274 | Test: Wavepacket Injection (Phase 6, Stage 2) Verifies that Gaussian wavepacket initialization: - Produces correct total energy - Conserves energy under evolution - Reaches the same steady state as... |
| [`campaign_genesis_timing_dependence.cpp`](../../engine/tests/campaign_genesis_timing_dependence.cpp) | 273 | @file campaign_genesis_timing_dependence.cpp @brief Is the mass excess a stable property of manifestation, or does it depend on exactly when (within the stochastic hazard's eligible window) genesis... |
| [`test_ftd0110_cluster_geometry.cpp`](../../engine/tests/test_ftd0110_cluster_geometry.cpp) | 273 | Test: FTD-0110 cluster GEOMETRY diagnostic. |
| [`test_internal_mode_action_transfer_v3.cpp`](../../engine/tests/test_internal_mode_action_transfer_v3.cpp) | 273 | FTD-0662: generalized-amplitude-normalized tight-frame covariance. |
| [`campaign_coupled_matched_face_transaction.cpp`](../../engine/tests/campaign_coupled_matched_face_transaction.cpp) | 271 | @file campaign_coupled_matched_face_transaction.cpp @brief FTD-0479 observer-only coupled matched-face matter/field gate. |
| [`campaign_moore_hop_route_ambiguity.cpp`](../../engine/tests/campaign_moore_hop_route_ambiguity.cpp) | 271 | @file campaign_moore_hop_route_ambiguity.cpp @brief FTD-0445 Moore-hop to oriented-face routing audit. |
| [`test_nonlinear_flow_multiscale.cpp`](../../engine/tests/test_nonlinear_flow_multiscale.cpp) | 270 | @file test_nonlinear_flow_multiscale.cpp @brief P2.1 + P2.2 + P2.3: native response tuple at b ∈ {1, 2, 4, 8} under mixed-toggle nonlinear dynamics, with ensemble uncertainties. |
| [`test_particle_engine.cpp`](../../engine/tests/test_particle_engine.cpp) | 270 | Phase 7 — Stage 2: ParticleEngine unit tests (12 checks) PE1: Particle injection (id assigned, charge correct) PE2: Free particle (constant velocity when alone, no damping) PE3: Opposite attract (f... |
| [`test_gpu_delta_upload.cpp`](../../engine/tests/test_gpu_delta_upload.cpp) | 269 | ============================================================================ test_gpu_delta_upload.cpp — C5 (CUDA ticket): host→device delta upload. |
| [`campaign_causal_horizon_environmental_persistence_cuda.cpp`](../../engine/tests/campaign_causal_horizon_environmental_persistence_cuda.cpp) | 268 | FTD-0747 candidate: CUDA successor to the aborted FTD-0746 CPU run. |
| [`test_native_ternary_dipole_phase_wedge_memory.cpp`](../../engine/tests/test_native_ternary_dipole_phase_wedge_memory.cpp) | 268 |  |
| [`test_color_binding_and_structure.cpp`](../../engine/tests/test_color_binding_and_structure.cpp) | 267 | @file test_color_binding_and_structure.cpp @brief Phase-4i: Combined tests for (1) RGB triad binding and (2) FTD "color" transformation structure vs SU(3). |
| [`test_cusp_dressing_integrability.cpp`](../../engine/tests/test_cusp_dressing_integrability.cpp) | 267 | FTD-0494: cellwise cusp primitive and global gluing obstruction. |
| [`test_engine_lifecycle.cpp`](../../engine/tests/test_engine_lifecycle.cpp) | 267 | ============================================================================ test_engine_lifecycle.cpp — ScaleEngine RAII / lifecycle contract (ticket W5) ------------------------------------------... |
| [`campaign_integer_sweep.cpp`](../../engine/tests/campaign_integer_sweep.cpp) | 266 | Campaign: Integer Uniqueness Sweep THE critical test for scientific credibility. |
| [`test_derived_interaction_graph_transaction.cpp`](../../engine/tests/test_derived_interaction_graph_transaction.cpp) | 266 | FTD-0721: derived interaction graph and closed-pair capture discriminator. |
| [`test_master_quadratic_uniqueness.cpp`](../../engine/tests/test_master_quadratic_uniqueness.cpp) | 266 | @file test_master_quadratic_uniqueness.cpp @brief Program E — Uniqueness of the master quadratic as minimal polynomial. |
| [`test_state_only_matter_field_observer_covariance.cpp`](../../engine/tests/test_state_only_matter_field_observer_covariance.cpp) | 266 | FTD-0754 supplemental: complete-observer covariance on nontrivial fields. |
| [`campaign_mechanical_history_hop_work.cpp`](../../engine/tests/campaign_mechanical_history_hop_work.cpp) | 265 | @file campaign_mechanical_history_hop_work.cpp @brief FTD-0449 mechanical journal sufficiency and production-hop work. |
| [`campaign_plato.cpp`](../../engine/tests/campaign_plato.cpp) | 264 | Campaign: Plato — Dispositional Field Tests Three tests probing the dispositional (flux) layer of FTD: P1: Dispositional Ratio — Verify 1/r^2 Coulomb falloff of \|J(r)\|^2 P2: Genesis Phase Transitio... |
| [`campaign_wigner.cpp`](../../engine/tests/campaign_wigner.cpp) | 263 | Campaign: Wigner Tests — Octahedral Symmetry, Parity, CPT Invariance W1: Octahedral Symmetry L=48, single +1 at center, 200 ticks. |
| [`test_axial_face_hop_reciprocity.cpp`](../../engine/tests/test_axial_face_hop_reciprocity.cpp) | 263 | FTD-0497: exact axial face hop and raw threshold-map reciprocity gate. |
| [`benchmark_g_n_mass_spectrum.cpp`](../../engine/tests/benchmark_g_n_mass_spectrum.cpp) | 262 | Benchmark: G_N(M, L) mass-spectrum scan (Arc D gap (ii) scaffold) Purpose: Verify that the engine's solve_latency_poisson_cpu (poisson_solvers.cpp:190-228) correctly reproduces a constant engine-in... |
| [`campaign_cluster_fission_fusion.cpp`](../../engine/tests/campaign_cluster_fission_fusion.cpp) | 262 | campaign_cluster_fission_fusion — Exp-A of the cluster-thermodynamics EXPLORATORY pass (P2 fission/fusion asymmetry + P3 fusion-is-lossy). |
| [`test_cluster_persistence_quiescent.cpp`](../../engine/tests/test_cluster_persistence_quiescent.cpp) | 262 | Test: Cluster Persistence Under Quiescent Dynamics (Class B Phase B.2) Per SPEC_CLASS_B_CLUSTER_PERSISTENCE.md §6.2: "Verify deterministic engine produces tau -> infty for all single clusters under... |
| [`test_collective_source_history_bound.cpp`](../../engine/tests/test_collective_source_history_bound.cpp) | 262 | FTD-0588: collective common-history and asynchronous source bounds. |
| [`benchmark_field_soa_cpu.cpp`](../../engine/tests/benchmark_field_soa_cpu.cpp) | 261 | @file benchmark_field_soa_cpu.cpp @brief Non-physics timing probe for CPU FieldSoA read paths. |
| [`campaign_proton_stability.cpp`](../../engine/tests/campaign_proton_stability.cpp) | 261 | @file campaign_proton_stability.cpp @brief FTD-0301: is the proton (uud triad) dynamically stable, or does it DECAY (evaporate / transmute) under FTD's native dynamics? |
| [`campaign_production_local_flux_carry_work.cpp`](../../engine/tests/campaign_production_local_flux_carry_work.cpp) | 260 | @file campaign_production_local_flux_carry_work.cpp @brief FTD-0461 energy accounting for production's integer-hop flux carry. |
| [`test_native_source_core_fork.cpp`](../../engine/tests/test_native_source_core_fork.cpp) | 260 | Native source-core fork audit. |
| [`campaign_topological_charge_transport.cpp`](../../engine/tests/campaign_topological_charge_transport.cpp) | 259 | @file campaign_topological_charge_transport.cpp @brief FTD-0398 terminal transport test for the existing octahedral Berg--Luescher charge convention. |
| [`test_ws_protocol.cpp`](../../engine/tests/test_ws_protocol.cpp) | 259 | ============================================================================ test_ws_protocol.cpp — WebSocket remote-control protocol unit tests (revision 1.4). |
| [`test_connected_block_analytic_envelope_hessian.cpp`](../../engine/tests/test_connected_block_analytic_envelope_hessian.cpp) | 258 | FTD-0637: analytic envelope gradient/Hessian of the frozen dressed block. |
| [`test_quadratic_coat_matter_work.cpp`](../../engine/tests/test_quadratic_coat_matter_work.cpp) | 257 | FTD-0545: the smooth-coat fixed-step action does not automatically satisfy the exact matter-work identity required by the matched field transaction. |
| [`test_wh_clifford_alt_routes.cpp`](../../engine/tests/test_wh_clifford_alt_routes.cpp) | 257 | @file test_wh_clifford_alt_routes.cpp @brief Phase-4 fermion-emergence alt-route measurements (FTD-0061 extension). |
| [`campaign_latency_slow_gate.cpp`](../../engine/tests/campaign_latency_slow_gate.cpp) | 256 | @file campaign_latency_slow_gate.cpp @brief T3 slow-gate candidacy, Stage A: is the latency sector SLOW relative to the flux band, under native matter activity? |
| [`test_helium_scale1.cpp`](../../engine/tests/test_helium_scale1.cpp) | 256 | Helium at Scale 1: Multi-Electron Atoms He⁺ (Z=2, 1 electron): Bohr model predicts a₀/2 radius, 4× binding He (Z=2, 2 electrons): exchange force creates e⁻-e⁻ repulsion HE-1: He⁺ electron survives... |
| [`test_quadratic_coat_neutral_pair_work.cpp`](../../engine/tests/test_quadratic_coat_neutral_pair_work.cpp) | 254 | FTD-0546: neutral self-consistent longitudinal transaction of the smooth quadratic-coat action. |
| [`test_discrete_legendre_worldline.cpp`](../../engine/tests/test_discrete_legendre_worldline.cpp) | 253 | FTD-0490: gauge-covariant interior discrete Legendre map. |
| [`test_centered_knot_trace.cpp`](../../engine/tests/test_centered_knot_trace.cpp) | 252 | FTD-0492: centered weak trace and branch-action discriminator. |
| [`campaign_poisson_reciprocity_convergence.cpp`](../../engine/tests/campaign_poisson_reciprocity_convergence.cpp) | 251 | @file campaign_poisson_reciprocity_convergence.cpp @brief FTD-0440 cold/pre-relaxed Poisson reciprocity convergence audit. |
| [`test_contact_quotient_coupling_scope.cpp`](../../engine/tests/test_contact_quotient_coupling_scope.cpp) | 251 | FTD-0528: native snapshot source versus matched history quotient. |
| [`test_contact_quotient_horizon.cpp`](../../engine/tests/test_contact_quotient_horizon.cpp) | 251 | FTD-0526: actual-production horizon of the identical-contact quotient. |
| [`test_thomson_scattering.cpp`](../../engine/tests/test_thomson_scattering.cpp) | 251 | Test: Thomson Scattering — 6 Checks Verifies that an EM wave encountering a charged particle causes the charge to oscillate and re-radiate (scatter). |
| [`test_gauge_links.cpp`](../../engine/tests/test_gauge_links.cpp) | 250 | ============================================================================ test_gauge_links.cpp — SU(2)/SU(3) gauge-link sector: golden profile + invariants (revision 0.9, option a — WIRED). |
| [`test_native_continuity.cpp`](../../engine/tests/test_native_continuity.cpp) | 250 | Native continuity audit for signed ternary state transport. |
| [`test_gpu_visual_snapshot.cpp`](../../engine/tests/test_gpu_visual_snapshot.cpp) | 249 | CUDA native visual-capture contract. |
| [`test_state_only_matter_field_observer.cpp`](../../engine/tests/test_state_only_matter_field_observer.cpp) | 249 | FTD-0754: state-only bound/characteristic observer algebra. |
| [`campaign_hop_mechanics_underdetermination.cpp`](../../engine/tests/campaign_hop_mechanics_underdetermination.cpp) | 248 | @file campaign_hop_mechanics_underdetermination.cpp @brief FTD-0444 scalar-work sufficiency and selected reversible-map audit. |
| [`campaign_novel_predictions.cpp`](../../engine/tests/campaign_novel_predictions.cpp) | 248 | Campaign: Novel Predictions & Falsifiability (Phase 10) Tests the sharpest predictions of FTD that can be falsified by experiment. |
| [`campaign_thermostat_off_sweep.cpp`](../../engine/tests/campaign_thermostat_off_sweep.cpp) | 248 | @file campaign_thermostat_off_sweep.cpp @brief FTD-0260 discriminator: is the FTD-0110 k(A) drift thermostat physics? |
| [`test_entanglement_basis.cpp`](../../engine/tests/test_entanglement_basis.cpp) | 248 | Test: Entanglement Basis Dependence — Measurement-Basis Correlations Tests the local hidden-variable model of entangled pairs, verifying that correlations depend on the measurement basis angle and... |
| [`test_finite_support_pair_preparation.cpp`](../../engine/tests/test_finite_support_pair_preparation.cpp) | 247 | FTD-0739: exact finite-support neutral-pair Gauss preparation. |
| [`test_lorentz_common_cone_improved.cpp`](../../engine/tests/test_lorentz_common_cone_improved.cpp) | 247 | FTD-0413 Moore-local q^4 common-cone fermion gate. |
| [`campaign_time_dilation.cpp`](../../engine/tests/campaign_time_dilation.cpp) | 246 | @file campaign_time_dilation.cpp @brief CAMPAIGN 2 — Dynamical time dilation: does a moving lattice clock dilate as √(1−v²) [L²/γ] or 1−v [L¹/FTD-0208]? |
| [`test_matched_regional_energy_transport.cpp`](../../engine/tests/test_matched_regional_energy_transport.cpp) | 246 | FTD-0671: exact regional matched-field energy transport identity. |
| [`test_oriented_even_self_pair_rectifier.cpp`](../../engine/tests/test_oriented_even_self_pair_rectifier.cpp) | 246 |  |
| [`test_volume_scaled_internal_mode_transfer_v2.cpp`](../../engine/tests/test_volume_scaled_internal_mode_transfer_v2.cpp) | 245 | FTD-0665: corrected mass-weighted normalization and accumulated recovery gate. |
| [`test_emergent_measurements.cpp`](../../engine/tests/test_emergent_measurements.cpp) | 243 | Emergent Measurements — High-Fidelity Lattice Physics Verification Five quantitative tests using high SOR resolution to measure emergent quantities that arise from the six FTD update rules: EM1: Co... |
| [`test_matched_contact_energy_obstruction.cpp`](../../engine/tests/test_matched_contact_energy_obstruction.cpp) | 243 | FTD-0529: reciprocal matched-field energy obstruction at contact. |
| [`test_eft_blocking.cpp`](../../engine/tests/test_eft_blocking.cpp) | 242 | @file test_eft_blocking.cpp @brief EFT Phase 2A — block-spin transformation validation gate. |
| [`test_gauss_law_fidelity_gpu.cpp`](../../engine/tests/test_gauss_law_fidelity_gpu.cpp) | 242 | ============================================================================ test_gauss_law_fidelity_gpu.cpp (engine-fidelity investigation, 2026-07-16) --------------------------------------------... |
| [`test_selffield_profile.cpp`](../../engine/tests/test_selffield_profile.cpp) | 241 | Test: Self-Field Profile Investigation (Phase 6, Stage 1) Characterizes the steady-state flux envelope around a single locked point-particle. |
| [`test_spacetime_forcing_demo.cpp`](../../engine/tests/test_spacetime_forcing_demo.cpp) | 241 | @file test_spacetime_forcing_demo.cpp @brief DEMONSTRATION for FTD-0253 (FOUND_SPACETIME_FORCING_BOUNDARY): the causal cone is forced by locality; the Lorentzian *metric* is not — it rides on the d... |
| [`test_production_same_sign_bounce.cpp`](../../engine/tests/test_production_same_sign_bounce.cpp) | 240 | FTD-0506: production same-sign bounce reciprocity audit. |
| [`golden_hash.h`](../../engine/tests/support/golden_hash.h) | 239 | ============================================================================ golden_hash.h — shared golden-gate state-hash harness (revision 0.5). |
| [`test_staggered_current_split_compatibility.cpp`](../../engine/tests/test_staggered_current_split_compatibility.cpp) | 239 | FTD-0535: exact endpoint current split versus staggered phase order. |
| [`test_geometric_freefall_integrator.cpp`](../../engine/tests/test_geometric_freefall_integrator.cpp) | 238 | ============================================================================ test_geometric_freefall_integrator.cpp ---------------------------------------------------------------------------- FTD-... |
| [`campaign_central_gauss_hop_realizability.cpp`](../../engine/tests/campaign_central_gauss_hop_realizability.cpp) | 237 | @file campaign_central_gauss_hop_realizability.cpp @brief FTD-0471 one-site Gauss transport under central versus face fields. |
| [`test_gpu_stream_binding.cpp`](../../engine/tests/test_gpu_stream_binding.cpp) | 237 | ============================================================================ test_gpu_stream_binding.cpp — the engine owns one non-legacy CUDA stream. |
| [`test_localized_basin_observer.cpp`](../../engine/tests/test_localized_basin_observer.cpp) | 237 |  |
| [`benchmark_rutherford_alpha.cpp`](../../engine/tests/benchmark_rutherford_alpha.cpp) | 236 | @file benchmark_rutherford_alpha.cpp @brief Thread 4 of the EFT Day-2 program — Rutherford scattering α extraction. |
| [`test_axial_contact_longitudinal_work.cpp`](../../engine/tests/test_axial_contact_longitudinal_work.cpp) | 236 | FTD-0530: Gauss-fixed axial contact longitudinal work. |
| [`test_benchmark.cpp`](../../engine/tests/test_benchmark.cpp) | 236 | Performance benchmark for the FTD engine. |
| [`test_dipole_radiation.cpp`](../../engine/tests/test_dipole_radiation.cpp) | 236 | Test: Dipole Radiation Pattern — 6 Checks Verifies that a z-polarized current burst produces the classical sin²θ angular radiation pattern. |
| [`campaign_genesis_trajectory.cpp`](../../engine/tests/campaign_genesis_trajectory.cpp) | 235 | @file campaign_genesis_trajectory.cpp @brief FTD-0267: genesis-vs-survival per-tick trajectory in the canonical engine. |
| [`campaign_pe_fine_structure.cpp`](../../engine/tests/campaign_pe_fine_structure.cpp) | 235 | Campaign: PE Fine Structure Tests multiple Phase 2 forces working together: spin-orbit + relativistic corrections produce fine structure splitting. |
| [`test_diagonal_endpoint_action_domain.cpp`](../../engine/tests/test_diagonal_endpoint_action_domain.cpp) | 235 | FTD-0532: composition audit at simultaneous diagonal hop planes. |
| [`test_universal_freefall_engine_align.cpp`](../../engine/tests/test_universal_freefall_engine_align.cpp) | 235 | ============================================================================ test_universal_freefall_engine_align.cpp ---------------------------------------------------------------------------- FT... |
| [`campaign_cross_scale.cpp`](../../engine/tests/campaign_cross_scale.cpp) | 234 | Phase 7 — Stage 4: Cross-Scale Validation (6 checks) Run the SAME two-body scenario at both Scale 0 (voxels) and Scale 1 (ParticleEngine). |
| [`campaign_lorentz_measure.cpp`](../../engine/tests/campaign_lorentz_measure.cpp) | 234 | Campaign: Lorentz Invariance Quantitative Measurement Runs wave packets along all 13 distinct lattice directions on the cubic lattice and measures the effective wave speed in each direction. |
| [`test_substrate_angle_probe.cpp`](../../engine/tests/test_substrate_angle_probe.cpp) | 233 | @file test_substrate_angle_probe.cpp @brief Stage-1 exploratory probe: which substrate phase (if any) carries a *native dynamical angle* under the bare wave + Gauss dynamics? |
| [`test_symmetric_diagonal_coupled_endpoint.cpp`](../../engine/tests/test_symmetric_diagonal_coupled_endpoint.cpp) | 233 | FTD-0531: symmetry-reduced diagonal field/matter endpoint solve. |
| [`benchmark_lorentz_recovery.cpp`](../../engine/tests/benchmark_lorentz_recovery.cpp) | 232 | @file benchmark_lorentz_recovery.cpp @brief EFT Phase 1B — free-flux correlator-collapse benchmark. |
| [`campaign_fixed_j_recoil_capacity.cpp`](../../engine/tests/campaign_fixed_j_recoil_capacity.cpp) | 232 | @file campaign_fixed_j_recoil_capacity.cpp @brief FTD-0453 fixed-J central-recoil energy-capacity gate. |
| [`test_causal_bound_internal_gait_continuation.cpp`](../../engine/tests/test_causal_bound_internal_gait_continuation.cpp) | 232 | FTD-0713: continue the FTD-0712 internal gait under causal-speed and graph bounds after removing only the auxiliary 0.05 coordinate cap. |
| [`test_eft_ward_identity.cpp`](../../engine/tests/test_eft_ward_identity.cpp) | 232 | @file test_eft_ward_identity.cpp @brief EFT Phase 1C — Ward-identity test suite. |
| [`test_scale_context.cpp`](../../engine/tests/test_scale_context.cpp) | 232 | Test: ScaleContext readout admissibility gate (C_scale) Verifies the read-only scale-context diagnostics layer that decides whether an engine cloud is eligible for public physical readout. |
| [`campaign_quark_quantization.cpp`](../../engine/tests/campaign_quark_quantization.cpp) | 231 | @file campaign_quark_quantization.cpp @brief FTD-0273 Phase 2 — quantize a colored "quark" with voxels; observe its phenomena. |
| [`test_eft_operator_spectrum.cpp`](../../engine/tests/test_eft_operator_spectrum.cpp) | 231 | @file test_eft_operator_spectrum.cpp @brief EFT Phase 3 — operator-basis scaling-dimension extraction. |
| [`test_ignition_cut_support_ablation.cpp`](../../engine/tests/test_ignition_cut_support_ablation.cpp) | 231 | FTD-0587: ignition-cut support-mechanism ablation. |
| [`test_internal_mode_return_time.cpp`](../../engine/tests/test_internal_mode_return_time.cpp) | 231 | FTD-0666: out-of-sample L=17 extension of the FTD-0665 return threshold. |
| [`test_momentum_face_balance.cpp`](../../engine/tests/test_momentum_face_balance.cpp) | 231 | FTD-0514: exact local momentum balance from oriented face current. |
| [`campaign_alpha_no_alpha_probe.cpp`](../../engine/tests/campaign_alpha_no_alpha_probe.cpp) | 230 | FTD-0285: fixed no-alpha-input alpha probe. |
| [`test_gpu_term_contract.cpp`](../../engine/tests/test_gpu_term_contract.cpp) | 230 | GPU term-completeness oracle. |
| [`test_csv_export.cpp`](../../engine/tests/test_csv_export.cpp) | 229 | Test: CSV Export Utility Verifies that the csv_export.h functions produce valid CSV files with correct headers, dimensions, and data content. |
| [`test_wh_clifford_anticommutator.cpp`](../../engine/tests/test_wh_clifford_anticommutator.cpp) | 229 | @file test_wh_clifford_anticommutator.cpp @brief Measure the anticommutator of the engine-induced product on the three weight-1 Walsh–Hadamard modes of a 2^3 block. |
| [`test_rest_qualified_moving_dressing_relative_orbit.cpp`](../../engine/tests/test_rest_qualified_moving_dressing_relative_orbit.cpp) | 227 | FTD-0709: rerun the complete v=1/2 relative-orbit test from the qualified L=33 full-coordinate rest state. |
| [`test_dark_matter.cpp`](../../engine/tests/test_dark_matter.cpp) | 226 | Test: Dark Matter (Sub-Threshold Flux) Verifies that flux with 0 < \|J\| < K_B behaves as dark matter: present but not manifested, gravitates but does not interact electromagnetically. |
| [`test_mixed_history_flow.cpp`](../../engine/tests/test_mixed_history_flow.cpp) | 226 | @file test_mixed_history_flow.cpp @brief P1.3 + P1.4 closure: multi-tick mixed-toggle reaction-transport Ward identity. |
| [`test_constants.cpp`](../../engine/tests/test_constants.cpp) | 225 | Test: Derivation chain D=3 -> alpha Verifies that all constants are self-consistently derived from D=3 + varpi (lemniscate constant). |
| [`test_reaction_scenario_physics.cpp`](../../engine/tests/test_reaction_scenario_physics.cpp) | 225 | Certification for the two native opposite-polarity event scenarios. |
| [`campaign_perturbation_magnitude_curl_sweep.cpp`](../../engine/tests/campaign_perturbation_magnitude_curl_sweep.cpp) | 224 | @file campaign_perturbation_magnitude_curl_sweep.cpp @brief Does injected curl scale with perturbation size, or is it a symmetry-breaking floor set by acting on a single site at all? |
| [`test_connected_block_linear_modes.cpp`](../../engine/tests/test_connected_block_linear_modes.cpp) | 224 | FTD-0629: generalized linear modes about the FTD-0628 dressed fixed point. |
| [`test_common_relative_connection_gearbox.cpp`](../../engine/tests/test_common_relative_connection_gearbox.cpp) | 223 |  |
| [`test_determinism.cpp`](../../engine/tests/test_determinism.cpp) | 223 | @file test_determinism.cpp @brief Bit-identical reproducibility under fixed seed. |
| [`test_smallest_particle_emergence.cpp`](../../engine/tests/test_smallest_particle_emergence.cpp) | 223 | @file test_smallest_particle_emergence.cpp @brief Phase-4h: Material emergence from the lattice. |
| [`test_identity_lifecycle.cpp`](../../engine/tests/test_identity_lifecycle.cpp) | 222 | CPU identity/provenance regression gates. |
| [`test_native_reaction_ledger.cpp`](../../engine/tests/test_native_reaction_ledger.cpp) | 222 | Native reaction ledger for signed ternary state changes. |
| [`test_single_slab_connection_compatibility.cpp`](../../engine/tests/test_single_slab_connection_compatibility.cpp) | 222 | FTD-0534: exact one-slab Faraday/work-field compatibility. |
| [`campaign_determinism_gate.cpp`](../../engine/tests/campaign_determinism_gate.cpp) | 221 | @file campaign_determinism_gate.cpp @brief GATE: is the langevin-OFF genesis spectroscopy harness deterministic? |
| [`test_cluster_tracker.cpp`](../../engine/tests/test_cluster_tracker.cpp) | 221 | Test: ClusterTracker (Class B Phase B.1) Smoke + invariant tests for the ClusterTracker introduced as the first concrete deliverable of the Discrete-Native Derivation Program (FTD-0136). |
| [`test_gpu_eft_parity.cpp`](../../engine/tests/test_gpu_eft_parity.cpp) | 221 | @file test_gpu_eft_parity.cpp @brief GPU vs CPU parity tests for the EFT operators and blocking map. |
| [`test_momentum.cpp`](../../engine/tests/test_momentum.cpp) | 221 | Test: Momentum Conservation — Noether Current from Translation Symmetry Verifies that total flux momentum is conserved in closed systems (no external forces, no boundary effects). |
| [`campaign_alpha_estimator_validation.cpp`](../../engine/tests/campaign_alpha_estimator_validation.cpp) | 220 | FTD-0286: alpha estimator validation after FTD-0285 invalidated. |
| [`test_annihilation.cpp`](../../engine/tests/test_annihilation.cpp) | 220 | Test: Annihilation — Matter-Antimatter Energy Conservation Verifies that when a +1 and -1 particle annihilate: 1. |
| [`test_gauge.cpp`](../../engine/tests/test_gauge.cpp) | 220 | Test: Gauge Invariance — J -> J + grad(lambda) Symmetry Verifies that physical observables are invariant under gauge transformations J -> J + grad(lambda) for arbitrary scalar lambda. |
| [`campaign_native_energy_contract_reconciliation.cpp`](../../engine/tests/campaign_native_energy_contract_reconciliation.cpp) | 219 | @file campaign_native_energy_contract_reconciliation.cpp @brief FTD-0452 native energy-contract and diagnostic reconciliation. |
| [`test_batched_matched_symmetry_ray_spectrum.cpp`](../../engine/tests/test_batched_matched_symmetry_ray_spectrum.cpp) | 219 | FTD-0697: batched/direct matched symmetry-ray spectrum equivalence. |
| [`test_gauss_record_canonical_reduction.cpp`](../../engine/tests/test_gauss_record_canonical_reduction.cpp) | 219 | FTD-0877/0880 matched Gauss-record canonical-reduction verifier. |
| [`test_wave_collapse.cpp`](../../engine/tests/test_wave_collapse.cpp) | 219 | Test: Wave Collapse — Flux Concentration & Manifestation Verifies that manifestation acts as wave function collapse: 1. |
| [`campaign_guide_cross_energy_decomposition.cpp`](../../engine/tests/campaign_guide_cross_energy_decomposition.cpp) | 218 | @file campaign_guide_cross_energy_decomposition.cpp @brief FTD-0463 packet/source versus dressing/source wave cross energy. |
| [`campaign_triad_binding.cpp`](../../engine/tests/campaign_triad_binding.cpp) | 217 | Campaign: Triad Binding Energy (Phase 8 — Particle Zoo) Tests whether three same-sign particles in an equilateral triangle configuration form a bound state with measurable binding energy. |
| [`test_discrete_operators.cpp`](../../engine/tests/test_discrete_operators.cpp) | 217 | Test: Discrete differential operators Verifies laplacian_flux(), divergence_flux(), curl_flux(), gradient_density() on small lattices with known configurations. |
| [`campaign_gravitational_wave.cpp`](../../engine/tests/campaign_gravitational_wave.cpp) | 216 | Campaign: Gravitational Wave Detection (Phase 7 — Gravitational Sector) Tests whether oscillating mass distributions produce propagating density perturbations — the FTD analog of gravitational waves. |
| [`campaign_manifestation_readout_collision.cpp`](../../engine/tests/campaign_manifestation_readout_collision.cpp) | 216 | @file campaign_manifestation_readout_collision.cpp @brief Do genuinely different flux configurations manifest to the identical discrete (state, color, spin) readout? |
| [`campaign_de_broglie_guidance.cpp`](../../engine/tests/campaign_de_broglie_guidance.cpp) | 215 | ============================================================================ campaign_de_broglie_guidance.cpp (FTD-0271 Phase E, 2026-06-11) --------------------------------------------------------... |
| [`test_a1g_bridge_i_empirical.cpp`](../../engine/tests/test_a1g_bridge_i_empirical.cpp) | 214 | Test: A_{1g}-fraction characterization of the FTD pipeline (FTD-0110). |
| [`test_genesis.cpp`](../../engine/tests/test_genesis.cpp) | 214 | Test: Genesis — Pair Production from Flux Collision Verifies that when two high-energy flux waves collide, particle pairs are created via the manifestation mechanism: - Density > K_GENESIS triggers... |
| [`test_local_canonical_hamiltonian_parity_rail.cpp`](../../engine/tests/test_local_canonical_hamiltonian_parity_rail.cpp) | 214 | FTD-0875 isolated local canonical Hamiltonian parity-rail verifier. |
| [`test_flux_propagator.cpp`](../../engine/tests/test_flux_propagator.cpp) | 213 | @file test_flux_propagator.cpp @brief Phase-4g: measure the 2-point flux correlator on the Langevin ensemble and classify as bosonic-vector vs fermionic/anomalous. |
| [`campaign_weak_decay.cpp`](../../engine/tests/campaign_weak_decay.cpp) | 212 | Campaign: Weak Decay Rate (Phase 6 — Weak Sector & SU(2)) Measures how transmutation rate depends on stress level, verifying the exponential probability formula. |
| [`test_accelerated_coat_spacetime_current.cpp`](../../engine/tests/test_accelerated_coat_spacetime_current.cpp) | 212 | FTD-0548: exact quadratic-coat current on an accelerated worldline. |
| [`test_gpu_shell_256.cpp`](../../engine/tests/test_gpu_shell_256.cpp) | 212 | GPU Shell Predictions at 256^3 — High-Precision Measurement Tests three structural predictions about the electron's self-field at 256^3 lattice resolution on GPU (GPU). |
| [`test_reversible_checkerboard_gauss_preparation.cpp`](../../engine/tests/test_reversible_checkerboard_gauss_preparation.cpp) | 212 | FTD-0881/0882 reversible checkerboard Gauss-preparation EFT verifier. |
| [`test_selective_damping.cpp`](../../engine/tests/test_selective_damping.cpp) | 212 | Test: Selective Damping (Phase D — FDTD Bridge) Verifies that selective_damping = true preserves vacuum EM waves while still damping flux near manifested particles. |
| [`dump_koopman_trajectory.cpp`](../../engine/tests/dump_koopman_trajectory.cpp) | 211 | FTD Koopman Observable Dumper Injects the A=14 canonical cloud and runs the Langevin bath. |
| [`test_phase_referenced_action_rail.cpp`](../../engine/tests/test_phase_referenced_action_rail.cpp) | 211 | FTD-0862 isolated phase-referenced action export rail verifier. |
| [`test_clock_gated_hamiltonian_exchange.cpp`](../../engine/tests/test_clock_gated_hamiltonian_exchange.cpp) | 210 | FTD-0865 isolated clock-gated Hamiltonian exchange verifier. |
| [`test_em_fields.cpp`](../../engine/tests/test_em_fields.cpp) | 210 | Test: E/B Field Diagnostics (Phase A — FDTD Bridge) Verifies the electromagnetic field decomposition: E = -wave_vel (electric field from leapfrog momentum) B = curl(J) (magnetic field from flux cur... |
| [`test_canonical_source_centered_gauss_gate.cpp`](../../engine/tests/test_canonical_source_centered_gauss_gate.cpp) | 209 | FTD-0885/0886 canonical source-centered Gauss-gate EFT verifier. |
| [`test_cluster_mask_persistence.cpp`](../../engine/tests/test_cluster_mask_persistence.cpp) | 209 | Phase B.3 protocol candidate: position-fixed mask persistence. |
| [`campaign_gravity_hierarchy.cpp`](../../engine/tests/campaign_gravity_hierarchy.cpp) | 208 | Campaign: Gravitational Hierarchy (Phase 7 — Gravitational Sector) Verifies the gravitational coupling hierarchy derived from the ontic chain: why gravity is 10^39 times weaker than EM in the physi... |
| [`test_polarization.cpp`](../../engine/tests/test_polarization.cpp) | 207 | Test: Polarization Counting — 2 Transverse Modes Verifies that the flux field has exactly 2 independent propagating polarization modes, as expected from: 3 components - 1 Gauss constraint = 2 physi... |
| [`test_gravity_dynamics.cpp`](../../engine/tests/test_gravity_dynamics.cpp) | 206 | Test: Gravity Dynamics — Gravitational Attraction from Density Gradient Verifies that the gravity term in phase_forces: F_grav = G_N · ∇ρ where G_N = 1/(b₃+N_c)² = 1/100 = 0.01 is an ENGINE-INTERNA... |
| [`test_moore_laplacian_isotropy.cpp`](../../engine/tests/test_moore_laplacian_isotropy.cpp) | 206 | test_moore_laplacian_isotropy.cpp — characterises TRACKER §1.8. |
| [`test_connected_bipole_deposited_current_form_factor.cpp`](../../engine/tests/test_connected_bipole_deposited_current_form_factor.cpp) | 205 | _symbols:_ Point, Row |
| [`test_dressing_fiber_ledger.cpp`](../../engine/tests/test_dressing_fiber_ledger.cpp) | 205 | FTD-0495: reversible scalar dressing ledger and action obstruction. |
| [`test_scale_bridge.cpp`](../../engine/tests/test_scale_bridge.cpp) | 205 | Phase 7 — Stage 3: Scale Bridge unit tests (8 checks) SB1: Coarsen charge matches voxel state SB2: Coarsen position matches coord + remainder SB3: Coarsen velocity preserved exactly SB4: Refine pla... |
| [`test_eft_matched_poisson.cpp`](../../engine/tests/test_eft_matched_poisson.cpp) | 204 | @file test_eft_matched_poisson.cpp @brief Day-2 Ticket A — matched-stencil CG Poisson solver validation. |
| [`test_full_state_irreversibility.cpp`](../../engine/tests/test_full_state_irreversibility.cpp) | 204 | FTD-0395 lock instrument: exact full-state collision under evaporation. |
| [`campaign_half_tick_link_exchange.cpp`](../../engine/tests/campaign_half_tick_link_exchange.cpp) | 203 | @file campaign_half_tick_link_exchange.cpp @brief FTD-0451 reversible half-tick Moore-link exchange ledger. |
| [`test_particle_toggles.cpp`](../../engine/tests/test_particle_toggles.cpp) | 203 | Test: ParticleToggles — Per-force toggle control for ParticleEngine Verifies that each toggle in ParticleToggles enables/disables its force, and that force_diag_ correctly decomposes forces by type. |
| [`test_sourced_geometric_freefall.cpp`](../../engine/tests/test_sourced_geometric_freefall.cpp) | 203 | ============================================================================ test_sourced_geometric_freefall.cpp ---------------------------------------------------------------------------- FTD-101... |
| [`campaign_alpha_estimator_validation_v2.cpp`](../../engine/tests/campaign_alpha_estimator_validation_v2.cpp) | 202 | FTD-0286 v2: alpha estimator validation — half-energy gate pairing. |
| [`campaign_amplitude_time_series.cpp`](../../engine/tests/campaign_amplitude_time_series.cpp) | 202 | @file campaign_amplitude_time_series.cpp @brief Per-tick cluster-size logging at custom injection amplitude. |
| [`campaign_cosmological_predictions.cpp`](../../engine/tests/campaign_cosmological_predictions.cpp) | 201 | Campaign: Cosmological Predictions (Phase 9 — Cosmological Validation) Verifies cosmological observables derived from framework integers {3, 4, 7, 13} and the master quadratic. |
| [`campaign_spontaneous_structure.cpp`](../../engine/tests/campaign_spontaneous_structure.cpp) | 201 | Campaign: Spontaneous Structure Formation 6 free particles (3+, 3-) with small random velocities on 48^3 lattice. |
| [`test_vtk_export.cpp`](../../engine/tests/test_vtk_export.cpp) | 201 | Test: Native ParaView/VTK XML export. |
| [`test_catalytic_phase_reference.cpp`](../../engine/tests/test_catalytic_phase_reference.cpp) | 200 | FTD-0863 isolated catalytic phase-reference transducer verifier. |
| [`campaign_weak_transmutation.cpp`](../../engine/tests/campaign_weak_transmutation.cpp) | 199 | Campaign: Weak Transmutation (Phase 6 — Weak Sector & SU(2)) Tests stress-threshold polarity flipping (+1 <-> -1) as the FTD analog of weak interactions (beta decay). |
| [`benchmark_phase_i_native_coupling.cpp`](../../engine/tests/benchmark_phase_i_native_coupling.cpp) | 198 | Phase I — FTD-Native Coupling Cross-Check (FTD-0125) Pre-registration: docs/theory/10_eft_program/PREREG_PHASE_I_NATIVE_COUPLING.md git tag: preregister-phase-i-native-coupling-v1 (commit e1f8157)... |
| [`dump_causal_golden_manifest.cpp`](../../engine/tests/dump_causal_golden_manifest.cpp) | 198 | FTD-0402 golden reconciliation instrument. |
| [`test_boundary_collision_resolution.cpp`](../../engine/tests/test_boundary_collision_resolution.cpp) | 198 | FTD-0505: exact tick-boundary collision resolution trilemma. |
| [`test_de_broglie_clock.cpp`](../../engine/tests/test_de_broglie_clock.cpp) | 198 | ============================================================================ test_de_broglie_clock.cpp (FTD-0271 Phase A, 2026-06-11) ---------------------------------------------------------------... |
| [`test_ncemc_feasibility.cpp`](../../engine/tests/test_ncemc_feasibility.cpp) | 198 | FTD-0405: Native Confinement Energy-Momentum Contract feasibility. |
| [`test_eft_anisotropy.cpp`](../../engine/tests/test_eft_anisotropy.cpp) | 197 | @file test_eft_anisotropy.cpp @brief EFT Phase 1A — rotational-anisotropy diagnostics. |
| [`test_fractional_heat_flow.cpp`](../../engine/tests/test_fractional_heat_flow.cpp) | 197 | Test: Fractional Heat Flow (Continuum Limit) Verifies that the FTD lattice wave equation transitions from purely ballistic wave propagation (r_rms ~ t^1) to fractional diffusion/heat flow (r_rms ~... |
| [`campaign_cubic_hop_work_response.cpp`](../../engine/tests/campaign_cubic_hop_work_response.cpp) | 196 | @file campaign_cubic_hop_work_response.cpp @brief FTD-0447 exact cubic-stabilizer closure of isolated-hop work. |
| [`test_coupling_readout_sweeps.cpp`](../../engine/tests/test_coupling_readout_sweeps.cpp) | 196 | Test: Coupling Readout Sweeps (Class C Phase C.3) Implements the automated sweeps and coupling constant extraction of the Class C Infrastructure Specification (FTD-0222). |
| [`test_helpers.h`](../../engine/tests/test_helpers.h) | 196 | @file test_helpers.h @brief Shared test utilities — checks, inspectors, toggle presets. |
| [`test_a1g_projector.cpp`](../../engine/tests/test_a1g_projector.cpp) | 195 | Unit test for the A_{1g} projector on a 27-voxel Moore block. |
| [`test_atom_toggles.cpp`](../../engine/tests/test_atom_toggles.cpp) | 195 | Test: AtomToggles — Per-force toggle control for AtomEngine Verifies that each toggle in AtomToggles enables/disables its force, and that force_diag_ correctly decomposes forces by type. |
| [`test_cubic_reaction_vector_source_transport.cpp`](../../engine/tests/test_cubic_reaction_vector_source_transport.cpp) | 195 |  |
| [`test_native_source_response.cpp`](../../engine/tests/test_native_source_response.cpp) | 195 | Native full-tick source-response audit. |
| [`benchmark_ewsb_threshold_map.cpp`](../../engine/tests/benchmark_ewsb_threshold_map.cpp) | 193 | @file benchmark_ewsb_threshold_map.cpp @brief Gap-closure Ticket 4 / Day 2 Thread 1b — EWSB amplitude threshold map. |
| [`test_signal_acknowledged_two_stroke_reset.cpp`](../../engine/tests/test_signal_acknowledged_two_stroke_reset.cpp) | 193 | FTD-0869 isolated signal-acknowledged two-stroke reset verifier. |
| [`test_ternary_collision_vertex.cpp`](../../engine/tests/test_ternary_collision_vertex.cpp) | 193 | FTD-0504: ternary collision capacity and identical-crossing quotient. |
| [`test_tracker.cpp`](../../engine/tests/test_tracker.cpp) | 193 | Test: Particle Tracker (Phase 1 — Measurement Infrastructure) Verifies that the Tracker correctly records particle trajectories using the engine's existing particle_id infrastructure. |
| [`test_born_infeld.cpp`](../../engine/tests/test_born_infeld.cpp) | 192 | Test: Born-Infeld Lagrangian Predictions Verifies: 1. |
| [`test_gpu_force_stack_parity.cpp`](../../engine/tests/test_gpu_force_stack_parity.cpp) | 192 | CPU/CUDA parity for the pairwise force stack and cluster inertia. |
| [`test_portable_field.cpp`](../../engine/tests/test_portable_field.cpp) | 192 | Test: Portable Self-Field Verifies that particles carry their flux when they move. |
| [`campaign_triad_energy.cpp`](../../engine/tests/campaign_triad_energy.cpp) | 191 | Campaign: Triad Energy Measurement (Phase 4 — Emergent Mass Spectrum) Measures the total energy of locked triads (3 same-sign particles) and compares with single-particle energy to extract binding... |
| [`test_atom_scale_bridge.cpp`](../../engine/tests/test_atom_scale_bridge.cpp) | 191 | Test: Atom Scale Bridge (Scale 1 ↔ Scale 2) 6 checks covering coarsen_to_atoms and refine_to_particles. |
| [`test_centered_trace_work.cpp`](../../engine/tests/test_centered_trace_work.cpp) | 191 | FTD-0493: exact field work omitted by the centered knot trace. |
| [`test_collective_reaction_triplet_inertia.cpp`](../../engine/tests/test_collective_reaction_triplet_inertia.cpp) | 191 |  |
| [`test_flux_link_clifford.cpp`](../../engine/tests/test_flux_link_clifford.cpp) | 190 | @file test_flux_link_clifford.cpp @brief Phase-4f: fermion-emergence test on the FLUX 1-form (link-like degrees of freedom), not the state 0-form. |
| [`campaign_explicit_rounding_e1_cuda_parity.cpp`](../../engine/tests/campaign_explicit_rounding_e1_cuda_parity.cpp) | 189 | FTD-0752: explicit-rounding qualification of the FTD-0751 stage map. |
| [`campaign_manifestation_seed_diversity.cpp`](../../engine/tests/campaign_manifestation_seed_diversity.cpp) | 189 | @file campaign_manifestation_seed_diversity.cpp @brief Is genesis's locked energy stable across genuinely different birth circumstances, or does it depend on how the particle was made? |
| [`internal_excitation_symmetry_ray_spectrum_hook_v2.h`](../../engine/tests/internal_excitation_symmetry_ray_spectrum_hook_v2.h) | 189 | FTD-0699 correction layer over the immutable FTD-0698 observation core. |
| [`test_cluster_interaction_dynamic.cpp`](../../engine/tests/test_cluster_interaction_dynamic.cpp) | 189 | Test: Dynamic Cluster-Cluster Interaction (Class C Phase C.2) Implements the Dynamical Scattering Protocol of the Class C Infrastructure Specification (FTD-0222). |
| [`test_local_polarity_regularity.cpp`](../../engine/tests/test_local_polarity_regularity.cpp) | 189 | FTD-0540: exact local-polarity regularity trilemma and witnesses. |
| [`test_correlations.cpp`](../../engine/tests/test_correlations.cpp) | 188 | Test: Correlation Functions (Phase 1 — Measurement Infrastructure) Verifies that spatial and temporal correlation functions work correctly on known field configurations. |
| [`test_gpu_benchmark.cpp`](../../engine/tests/test_gpu_benchmark.cpp) | 188 | GPU performance benchmark for the FTD CUDA engine. |
| [`test_native_conserved_parent.cpp`](../../engine/tests/test_native_conserved_parent.cpp) | 188 | Native conserved-parent audit for weak transmutation. |
| [`test_bloch_quasimomentum_lift.cpp`](../../engine/tests/test_bloch_quasimomentum_lift.cpp) | 187 |  |
| [`test_consciousness.cpp`](../../engine/tests/test_consciousness.cpp) | 187 | Test: Reference frame context Quadratic Verifies the reference frame context sector of the ontic derivation chain: the master quadratic with k = 1/2 produces complex roots whose real and imaginary... |
| [`test_finite_port_gauss_battery.cpp`](../../engine/tests/test_finite_port_gauss_battery.cpp) | 187 | FTD-0883/0884 finite port Gauss battery EFT verifier. |
| [`test_reaction_operators.cpp`](../../engine/tests/test_reaction_operators.cpp) | 187 | @file test_reaction_operators.cpp @brief Unit tests for the FTD-0112 reaction-sector operators (O7-O10). |
| [`test_self_pair_connection_critical_gearbox.cpp`](../../engine/tests/test_self_pair_connection_critical_gearbox.cpp) | 187 |  |
| [`campaign_genesis_geometry.cpp`](../../engine/tests/campaign_genesis_geometry.cpp) | 186 | @file campaign_genesis_geometry.cpp @brief FTD-0110 nonlinear bridge: per-fired-voxel FIRING GEOMETRY in the engine. |
| [`test_boundary_modes_golden.cpp`](../../engine/tests/test_boundary_modes_golden.cpp) | 186 | ============================================================================ test_boundary_modes_golden.cpp — boundary-mode characterization goldens (revision 0.6; ADR-0012 amendment / multi-profil... |
| [`campaign_born_ensemble.cpp`](../../engine/tests/campaign_born_ensemble.cpp) | 185 | Phase 7 — Stage 6: Born Rule Ensemble (4 checks) Demonstrate that the Born rule P(x) = \|psi(x)\|^2 emerges as the ensemble average over sub-scale initial conditions. |
| [`test_visual_field_sample.cpp`](../../engine/tests/test_visual_field_sample.cpp) | 185 | Compact Scale-0 visual-field readback contract. |
| [`test_z3_color_center.cpp`](../../engine/tests/test_z3_color_center.cpp) | 185 | test_z3_color_center.cpp — Z_3 center-closure [THEOREM] verification. |
| [`campaign_born_rule.cpp`](../../engine/tests/campaign_born_rule.cpp) | 184 | Campaign: Born Rule from Genesis Statistics (Phase 3 — Quantum Mechanics) Validates that particle manifestation (genesis) follows Born rule: P(x) ∝ \|J(x)\|² Theory: FTD genesis probability is p = cl... |
| [`campaign_production_hop_kinematics_correction.cpp`](../../engine/tests/campaign_production_hop_kinematics_correction.cpp) | 184 | @file campaign_production_hop_kinematics_correction.cpp @brief FTD-0450 correction of FTD-0444's selected energy convention. |
| [`test_mechanism_b.cpp`](../../engine/tests/test_mechanism_b.cpp) | 184 | Test: Mechanism B (Lattice-to-Continuum Matching via Vacuum Polarization) Implements the explicit stochastic quantization of the FTD engine. |
| [`test_wave_spin_invariant.cpp`](../../engine/tests/test_wave_spin_invariant.cpp) | 184 | test_wave_spin_invariant.cpp — field circulation ledger gate. |
| [`campaign_structure_stability.cpp`](../../engine/tests/campaign_structure_stability.cpp) | 183 | Campaign: Structure Stability Survey (Phase 4 — Emergent Mass Spectrum) Tests which particle configurations survive long evolution (5000 ticks) under EM + gravity dynamics. |
| [`test_autonomous_phase_parity_source_reaction.cpp`](../../engine/tests/test_autonomous_phase_parity_source_reaction.cpp) | 183 | FTD-0887/0888 autonomous phase-parity/source-reaction EFT verifier. |
| [`test_langevin_equipartition.cpp`](../../engine/tests/test_langevin_equipartition.cpp) | 183 | @file test_langevin_equipartition.cpp @brief Verify the Langevin thermostat produces the expected equilibrium. |
| [`test_quadratic_coat_composite_peierls.cpp`](../../engine/tests/test_quadratic_coat_composite_peierls.cpp) | 183 | FTD-0553: exact Peierls obstruction for rigid integer-offset neutral composites built from the compact quadratic polarity coat. |
| [`test_bridge_dynamics.cpp`](../../engine/tests/test_bridge_dynamics.cpp) | 182 | Test: RenderBridge tick dynamics Integration tests for vacuum stability, flux injection, propagation, manifestation, and diagnostics. |
| [`test_cluster_persistence_toggle_sweep.cpp`](../../engine/tests/test_cluster_persistence_toggle_sweep.cpp) | 182 | Test: Cluster Persistence — toggle configuration sweep (B.2 diagnosis (b)) The alpha-sweep diagnostic (test_cluster_persistence_alpha_sweep.cpp) showed cluster lifetimes saturate at ~45 ticks even... |
| [`test_native_projection_convergence.cpp`](../../engine/tests/test_native_projection_convergence.cpp) | 182 | Native Gauss-projection convergence audit. |
| [`test_wilson_dirac_smoke.cpp`](../../engine/tests/test_wilson_dirac_smoke.cpp) | 182 | Wilson-Dirac smoke test (Phase II.2-A milestone). |
| [`campaign_inertial_mass.cpp`](../../engine/tests/campaign_inertial_mass.cpp) | 181 | Campaign: Inertial Mass Measurement (Phase 4 — Emergent Mass Spectrum) Measures effective inertial mass of manifested particles via F = ma. |
| [`test_sublattice_laplacian.cpp`](../../engine/tests/test_sublattice_laplacian.cpp) | 181 | @file test_sublattice_laplacian.cpp @brief Validate sublattice-projected Laplacians (laplacian_sc, _fcc, _bcc). |
| [`test_reciprocal_carry_reservoir.cpp`](../../engine/tests/test_reciprocal_carry_reservoir.cpp) | 180 |  |
| [`campaign_cluster_relaxation.cpp`](../../engine/tests/campaign_cluster_relaxation.cpp) | 179 | campaign_cluster_relaxation — Exp-B of the cluster-thermodynamics EXPLORATORY pass (P4 N_internal + P1 cost<->N). |
| [`campaign_statistical_convergence.cpp`](../../engine/tests/campaign_statistical_convergence.cpp) | 179 | Campaign: Statistical Convergence (Phase 1 — Measurement Infrastructure) Validates that ensemble moments converge as N_runs increases. |
| [`test_native_current_flow.cpp`](../../engine/tests/test_native_current_flow.cpp) | 179 | Native current-flow audit for finite-volume b=2 blocking. |
| [`test_radiative_decay_scale1.cpp`](../../engine/tests/test_radiative_decay_scale1.cpp) | 179 | Radiative Decay at Scale 1: Orbit Shrinkage from Larmor Radiation FTD note: radiation reaction is [IMPOSED] physics — the Larmor formula P = (2α/3) q²a²/(mc³) is adopted from SM, with the coefficie... |
| [`graviton_fft_cuda.h`](../../engine/tests/graviton_fft_cuda.h) | 178 | graviton_fft_cuda.h — GPU (cuFFT) backend for the per-tick 3D FFTs of campaign_graviton_tt_correlator.cpp. |
| [`test_erdos_unit_distance.cpp`](../../engine/tests/test_erdos_unit_distance.cpp) | 178 |  |
| [`test_lorentz_common_cone.cpp`](../../engine/tests/test_lorentz_common_cone.cpp) | 177 | FTD-0412 common-cone matter gate. |
| [`test_callstack_audit_fixes.cpp`](../../engine/tests/test_callstack_audit_fixes.cpp) | 176 | test_callstack_audit_fixes.cpp — verifies the 2026-04-17 callstack audit fixes (findings F1–F8). |
| [`test_dressed_boost_momentum_map.cpp`](../../engine/tests/test_dressed_boost_momentum_map.cpp) | 176 |  |
| [`test_cosmological_constant.cpp`](../../engine/tests/test_cosmological_constant.cpp) | 175 | Test: Cosmological Constant Verifies that the vacuum energy density from the dual-substrate framework gives Omega_Lambda = 2/3, consistent with the FTD cosmological constant conjecture. |
| [`test_matched_face_current_spectrum.cpp`](../../engine/tests/test_matched_face_current_spectrum.cpp) | 175 |  |
| [`campaign_endpoint_recoil_support.cpp`](../../engine/tests/campaign_endpoint_recoil_support.cpp) | 174 | @file campaign_endpoint_recoil_support.cpp @brief FTD-0448 cubic covariance versus endpoint recoil support. |
| [`test_alternating_oriented_ternary_parity_rail.cpp`](../../engine/tests/test_alternating_oriented_ternary_parity_rail.cpp) | 174 | FTD-0874 isolated alternating oriented ternary parity-rail verifier. |
| [`test_native_pair_energy_recursion.cpp`](../../engine/tests/test_native_pair_energy_recursion.cpp) | 174 | FTD-0840 isolated native-pair energy recursion regression. |
| [`test_native_event_characteristics.cpp`](../../engine/tests/test_native_event_characteristics.cpp) | 173 | FTD-0858 isolated native event-acceptance/characteristic verifier. |
| [`test_component_aware_radial_field_profile.cpp`](../../engine/tests/test_component_aware_radial_field_profile.cpp) | 172 | FTD-0683: fixed-origin component-aware radial field profile. |
| [`test_gpu_geometric_gravity_parity.cpp`](../../engine/tests/test_gpu_geometric_gravity_parity.cpp) | 172 | ============================================================================ test_gpu_geometric_gravity_parity.cpp ---------------------------------------------------------------------------- FTD-1... |
| [`test_open_worldline_hop_selector.cpp`](../../engine/tests/test_open_worldline_hop_selector.cpp) | 172 | FTD-0489: an open charged-worldline action is not an endpoint cost. |
| [`test_ramsey_multicolor.cpp`](../../engine/tests/test_ramsey_multicolor.cpp) | 171 |  |
| [`test_ternary_eligibility_clutch.cpp`](../../engine/tests/test_ternary_eligibility_clutch.cpp) | 171 | FTD-0867 isolated ternary eligibility clutch/handshake verifier. |
| [`test_gpu_energy_ledger_parity.cpp`](../../engine/tests/test_gpu_energy_ledger_parity.cpp) | 170 | GPU energy-ledger gap regression (FTD engine, 2026-08-20). |
| [`test_spectral.cpp`](../../engine/tests/test_spectral.cpp) | 170 | Test: Spectral Analysis (Phase 1 — Measurement Infrastructure) Verifies FFT implementation and dispersion relation measurement. |
| [`test_cluster_interaction_static.cpp`](../../engine/tests/test_cluster_interaction_static.cpp) | 169 | Test: Static Cluster-Cluster Interaction (Class C Phase C.1) Implements the Static Template of the Class C Infrastructure Specification (FTD-0222). |
| [`test_accelerated_worldline_energy.cpp`](../../engine/tests/test_accelerated_worldline_energy.cpp) | 168 | FTD-0547: exact uniform-force accelerated-worldline energy escape. |
| [`test_connected_reservoir_decomposition.cpp`](../../engine/tests/test_connected_reservoir_decomposition.cpp) | 168 | FTD-0673: exact complete perturbation reservoir decomposition. |
| [`test_gauss_threshold_force_obstruction.cpp`](../../engine/tests/test_gauss_threshold_force_obstruction.cpp) | 168 | FTD-0487: Gauss-source lower bound on threshold force jumps. |
| [`test_matched_maxwell_integration.cpp`](../../engine/tests/test_matched_maxwell_integration.cpp) | 167 | Production-tick integration gates for the FTD-0428 selected branch. |
| [`test_native_manifestation_ledger.cpp`](../../engine/tests/test_native_manifestation_ledger.cpp) | 167 | Native manifestation ledger for genesis and evaporation. |
| [`test_native_engine_history_flow.cpp`](../../engine/tests/test_native_engine_history_flow.cpp) | 166 | Native engine-history flow audit. |
| [`benchmark_invariant_matrix_constant_memory.cu`](../../engine/tests/benchmark_invariant_matrix_constant_memory.cu) | 165 | benchmark_invariant_matrix_constant_memory.cu Standalone benchmark for the CUDA constant-memory invariant pattern established by ADR-0014. |
| [`test_connected_moore_block_local_residual_solve.cpp`](../../engine/tests/test_connected_moore_block_local_residual_solve.cpp) | 165 | FTD-0692: exact-equivalence gate for local nonlinear residual storage. |
| [`test_matched_midpoint_poynting.cpp`](../../engine/tests/test_matched_midpoint_poynting.cpp) | 165 | FTD-0544: exact matched midpoint field-energy identity. |
| [`test_moore26_clifford_test.cpp`](../../engine/tests/test_moore26_clifford_test.cpp) | 164 | @file test_moore26_clifford_test.cpp @brief Phase-4c fermion-emergence route: Moore-26 / 3³ block with axial sawtooth modes as "weight-1" generators. |
| [`test_self_field_decomposition.cpp`](../../engine/tests/test_self_field_decomposition.cpp) | 164 | FTD-0488: locality and provenance limits of self-field subtraction. |
| [`test_spin_field_clifford.cpp`](../../engine/tests/test_spin_field_clifford.cpp) | 164 | @file test_spin_field_clifford.cpp @brief Phase-4e: fermion-emergence test on the SPIN field, not the state field. |
| [`campaign_scale_context_confine.cpp`](../../engine/tests/campaign_scale_context_confine.cpp) | 163 | campaign_scale_context_confine.cpp Confinement scan for the scale-context readout admissibility gate. |
| [`test_cpu_gpu_divergence.cpp`](../../engine/tests/test_cpu_gpu_divergence.cpp) | 162 | _symbols:_ DifferenceSummary |
| [`test_molecular_dihedrals.cpp`](../../engine/tests/test_molecular_dihedrals.cpp) | 162 | @file test_molecular_dihedrals.cpp @brief Torsional dihedrals and improper planarity potentials unit test. |
| [`campaign_genesis_criticality.cpp`](../../engine/tests/campaign_genesis_criticality.cpp) | 161 | @file campaign_genesis_criticality.cpp @brief Order of the FTD genesis/manifestation transition (RG-spectrum probe). |
| [`test_gamma_ftd_momentum.cpp`](../../engine/tests/test_gamma_ftd_momentum.cpp) | 161 | test_gamma_ftd_momentum.cpp — verifies γ_FTD momentum integration in phase_forces (closes TRACKER_OPEN_ITEMS §1.2). |
| [`test_knot_legendre_branch.cpp`](../../engine/tests/test_knot_legendre_branch.cpp) | 161 | FTD-0491: branch multiplicity of the Legendre equation at a knot. |
| [`test_batched_regional_energy_profile.cpp`](../../engine/tests/test_batched_regional_energy_profile.cpp) | 160 | FTD-0686: batched/scalar exact regional-energy equivalence. |
| [`probe_cuda_current_support.cpp`](../../engine/tests/probe_cuda_current_support.cpp) | 159 | FTD-0748 pre-lock probe: characterize sparse-current support semantics. |
| [`test_cluster_persistence_alpha_sweep.cpp`](../../engine/tests/test_cluster_persistence_alpha_sweep.cpp) | 159 | Test: Cluster Persistence — alpha sensitivity sweep (Phase B.2 diagnostic) FINDING B.2-B from test_cluster_persistence_quiescent: clusters nucleate under FTD-0110-canonical injection but dissolve w... |
| [`test_watson_integrals.cpp`](../../engine/tests/test_watson_integrals.cpp) | 159 | @file test_watson_integrals.cpp @brief Numerical Watson integrals for SC, BCC, FCC, and Moore-18 stencils. |
| [`test_gpu_verlet_parity.cpp`](../../engine/tests/test_gpu_verlet_parity.cpp) | 158 | CPU/CUDA parity for the E1 / FTD-0337 velocity-Verlet (KDK) wave integrator. |
| [`campaign_hydrogen_binding.cpp`](../../engine/tests/campaign_hydrogen_binding.cpp) | 157 | Campaign: Hydrogen-Like Bound State (Phase 4 — Emergent Mass Spectrum) Tests whether opposite-charge particles form stable bound states with measurable binding energy and orbital structure. |
| [`test_lattice_operators.cpp`](../../engine/tests/test_lattice_operators.cpp) | 157 | Test: Extended lattice topology and wrapping Complements test_lattice.cpp with additional checks: neighbor symmetry, self-reference exclusion, boundary wrapping edge cases, and multi-size sanity. |
| [`test_wilson_dirac_bz_spectrum.cpp`](../../engine/tests/test_wilson_dirac_bz_spectrum.cpp) | 156 | Wilson-Dirac full-BZ spectrum sweep (Phase II.2-B milestone). |
| [`test_langevin_gpu_cpu_parity.cpp`](../../engine/tests/test_langevin_gpu_cpu_parity.cpp) | 154 | @file test_langevin_gpu_cpu_parity.cpp @brief Langevin thermostat: equipartition + GPU/CPU statistical agreement. |
| [`test_leapfrog_integrator_audit.cpp`](../../engine/tests/test_leapfrog_integrator_audit.cpp) | 154 | test_leapfrog_integrator_audit.cpp — closes TRACKER_OPEN_ITEMS §1.4. |
| [`campaign_hydrogen_lscan.cpp`](../../engine/tests/campaign_hydrogen_lscan.cpp) | 153 | Campaign: Hydrogen L-scan — does s0-seed-hydrogen stay a stable atom, or does it flood/condense the periodic box, and is the onset L-dependent? |
| [`campaign_vacuum_energy.cpp`](../../engine/tests/campaign_vacuum_energy.cpp) | 153 | ============================================================================= SUPERSEDED (2026-07-03, FTD-0364) — DO NOT USE FOR THE Λ SOURCE-GAP CHECK. |
| [`test_cuda_ordered_current_observer.cpp`](../../engine/tests/test_cuda_ordered_current_observer.cpp) | 153 | Ordered raw CUDA deposition and deterministic selected-radius observation. |
| [`test_energy_conservation_tight.cpp`](../../engine/tests/test_energy_conservation_tight.cpp) | 153 | @file test_energy_conservation_tight.cpp @brief Symplectic-leapfrog energy conservation: bounded oscillation, no drift. |
| [`test_hamiltonian_ternary_quarter_turn_actuator.cpp`](../../engine/tests/test_hamiltonian_ternary_quarter_turn_actuator.cpp) | 153 | FTD-0873 isolated Hamiltonian ternary quarter-turn actuator verifier. |
| [`test_quartic_relative_carry_gearbox.cpp`](../../engine/tests/test_quartic_relative_carry_gearbox.cpp) | 153 |  |
| [`test_subcell_polarity_shape.cpp`](../../engine/tests/test_subcell_polarity_shape.cpp) | 153 | Focused algebra tests for the isolated sub-cell polarity shape. |
| [`test_voxel_properties.cpp`](../../engine/tests/test_voxel_properties.cpp) | 153 | Test: Voxel derived quantities Verifies density(), speed(), bandwidth_used(), gamma_ftd(), and born_infeld_core() for known inputs. |
| [`test_wilson_dirac_limit.cpp`](../../engine/tests/test_wilson_dirac_limit.cpp) | 153 | Wilson-Dirac limit consistency (Phase II.2-D milestone). |
| [`test_wilson_dirac_cuda_parity.cpp`](../../engine/tests/test_wilson_dirac_cuda_parity.cpp) | 152 | Wilson-Dirac CPU/GPU parity (Phase II.2-E milestone). |
| [`benchmark_beta_function.cpp`](../../engine/tests/benchmark_beta_function.cpp) | 151 | @file benchmark_beta_function.cpp @brief EFT Phase 2C — lattice-measured β(g) via multi-scale α_eff extraction. |
| [`test_ensemble.cpp`](../../engine/tests/test_ensemble.cpp) | 151 | Test: Ensemble Runner (Phase 1 — Measurement Infrastructure) Verifies that ensemble statistics work correctly: EN1: 5-run ensemble on identical setup produces non-zero variance (stochastic genesis... |
| [`test_matched_gauss_transport.cpp`](../../engine/tests/test_matched_gauss_transport.cpp) | 151 | Exact operator and transport tests for FTD-0427. |
| [`test_reversible_ternary_signal_uncomputation.cpp`](../../engine/tests/test_reversible_ternary_signal_uncomputation.cpp) | 151 | FTD-0871 isolated reversible ternary signal-uncomputation verifier. |
| [`test_ui_observer_neutrality_cpu.cpp`](../../engine/tests/test_ui_observer_neutrality_cpu.cpp) | 151 |  |
| [`test_genesis_scenario_physics.cpp`](../../engine/tests/test_genesis_scenario_physics.cpp) | 150 | One-tick certification for the selected native genesis hazard. |
| [`test_master_quadratic_identities.cpp`](../../engine/tests/test_master_quadratic_identities.cpp) | 150 | @file test_master_quadratic_identities.cpp @brief Numerical verification of the bare algebraic content of the master quadratic. |
| [`benchmark_sm_masses_gpu.cpp`](../../engine/tests/benchmark_sm_masses_gpu.cpp) | 149 | @file benchmark_sm_masses_gpu.cpp @brief GPU exploratory Standard Model hierarchy benchmark Computes the equilibrium field energy for fundamental particles on the ternary lattice and compares ratio... |
| [`test_gpu_particle_capacity.cpp`](../../engine/tests/test_gpu_particle_capacity.cpp) | 149 | ============================================================================ test_gpu_particle_capacity.cpp — fixed-capacity pairwise-force launches. |
| [`test_lorentz_ir_envelope.cpp`](../../engine/tests/test_lorentz_ir_envelope.cpp) | 149 | @file test_lorentz_ir_envelope.cpp @brief FTD-0414 exact and finite-q gates for the selected IR envelope. |
| [`test_flux_wave_velocity_markov_carrier.cpp`](../../engine/tests/test_flux_wave_velocity_markov_carrier.cpp) | 148 | FTD-0876 native flux/wave-velocity canonical-carrier verifier. |
| [`test_gpu_dissipation_source.cpp`](../../engine/tests/test_gpu_dissipation_source.cpp) | 147 | Dissipation Source Analysis Question: Where does energy go in each phase of the tick cycle? |
| [`test_gpu_floquet_parity.cpp`](../../engine/tests/test_gpu_floquet_parity.cpp) | 147 | CPU/CUDA parity for the FTD-0408 / FTD-0411 period-two Floquet wave kicks. |
| [`test_variational_coulomb.cpp`](../../engine/tests/test_variational_coulomb.cpp) | 147 | Test: Coupling helper conventions -- Field-Mediated Electrostatics Legacy regression coverage for the standalone coupling_term and coupling_force helpers. |
| [`campaign_moore_channel_projection_kernel.cpp`](../../engine/tests/campaign_moore_channel_projection_kernel.cpp) | 146 | @file campaign_moore_channel_projection_kernel.cpp @brief FTD-0446 exact kernel of the 13-channel to Vec3 projection. |
| [`test_fine_structure_scale1.cpp`](../../engine/tests/test_fine_structure_scale1.cpp) | 146 | Fine Structure at Scale 1: Spin-Orbit Splitting FTD Multi-Scale: Scale 1 has spin-orbit and relativistic toggles already implemented. |
| [`test_observable_commutativity.cpp`](../../engine/tests/test_observable_commutativity.cpp) | 146 | ============================================================================ test_observable_commutativity.cpp ---------------------------------------------------------------------------- Part C of... |
| [`test_connected_block_full_half_static_refinement.cpp`](../../engine/tests/test_connected_block_full_half_static_refinement.cpp) | 145 | FTD-0631: refine and qualify the fully-half connected matter candidate. |
| [`dump_full_physics.cpp`](../../engine/tests/dump_full_physics.cpp) | 144 | Complete-physics-lattice test: all FTD physics toggles ON. |
| [`bridge_fixtures.cpp`](../../engine/tests/support/bridge_fixtures.cpp) | 143 | ============================================================================ tests/support/bridge_fixtures.cpp ---------------------------------------------------------------------------- Implement... |
| [`test_scenario_velocity_wiring.cpp`](../../engine/tests/test_scenario_velocity_wiring.cpp) | 143 | test_scenario_velocity_wiring.cpp Audit item (physics-orchestrator, 2026-04-18): after porting the JS flux-meson / flux-string-breaking / flux-baryon scenarios to engine/src/scenarios.cpp, verify t... |
| [`campaign_genesis_moore_signature.cpp`](../../engine/tests/campaign_genesis_moore_signature.cpp) | 142 | @file campaign_genesis_moore_signature.cpp @brief Do genesis CLUSTERS carry an O_h / Moore quantum-number signature? |
| [`test_gpu_visual_field_sample.cpp`](../../engine/tests/test_gpu_visual_field_sample.cpp) | 142 | CUDA compact visual-field fidelity contract. |
| [`test_oriented_ternary_quarter_turn.cpp`](../../engine/tests/test_oriented_ternary_quarter_turn.cpp) | 142 | FTD-0872 isolated oriented ternary quarter-turn verifier. |
| [`test_reciprocal_record_port.cpp`](../../engine/tests/test_reciprocal_record_port.cpp) | 142 | FTD-0856 isolated reciprocal record-port reference verifier. |
| [`test_relative_action_transducer.cpp`](../../engine/tests/test_relative_action_transducer.cpp) | 142 | FTD-0860 isolated relative action/orientation transducer verifier. |
| [`campaign_h2_molecule.cpp`](../../engine/tests/campaign_h2_molecule.cpp) | 141 | Campaign: H2 Molecule Formation Two hydrogen atoms approach, form a covalent bond, settle into vibrational equilibrium, and conserve energy. |
| [`test_native_moore_layer_coupling.cpp`](../../engine/tests/test_native_moore_layer_coupling.cpp) | 141 | Native Moore-layer coupling audit. |
| [`test_symmetric_movement.cpp`](../../engine/tests/test_symmetric_movement.cpp) | 141 | Test: Symmetric Movement and Coordinate-Independent Chirality Density Verifies that: 1. |
| [`benchmark_ewsb_pipe.cpp`](../../engine/tests/benchmark_ewsb_pipe.cpp) | 140 | @file benchmark_ewsb_pipe.cpp @brief EWSB amplitude-threshold map — Phase E port of benchmark_ewsb_threshold_map. |
| [`dump_toggle_bisection.cpp`](../../engine/tests/dump_toggle_bisection.cpp) | 140 | Toggle-bisection: which physics toggle drives which feature? |
| [`test_atom_toggles_table.cpp`](../../engine/tests/test_atom_toggles_table.cpp) | 139 | Test: AtomToggles characterization (ticket 3.3) Pins the EXACT post-construction toggle state, enable_all()/minimal() profiles, validate() verdicts, and the string get_toggle/set_toggle round-trip... |
| [`test_gauge_gpu_parity.cpp`](../../engine/tests/test_gauge_gpu_parity.cpp) | 139 | ============================================================================ test_gauge_gpu_parity.cpp — SU(2)/SU(3) gauge-link relaxation: CPU/GPU parity + GPU determinism (revision 0.9 option a;... |
| [`test_force_diag_parity.cpp`](../../engine/tests/test_force_diag_parity.cpp) | 138 | test_force_diag_parity.cpp CPU-vs-GPU parity test for the force_diag mirror added 2026-04-25. |
| [`test_gpu_matched_gauss_parity.cpp`](../../engine/tests/test_gpu_matched_gauss_parity.cpp) | 138 | CPU/CUDA parity for FTD-0428 matched_gauss_dynamics. |
| [`test_support_invariant_matter_predicate.cpp`](../../engine/tests/test_support_invariant_matter_predicate.cpp) | 138 | FTD-0755: state-only support-independent relational-core predicate. |
| [`test_wilson_topology.cpp`](../../engine/tests/test_wilson_topology.cpp) | 138 | test_wilson_topology.cpp — Phase I Item 3 Mechanism A diagnostic. |
| [`campaign_drain_scan.cpp`](../../engine/tests/campaign_drain_scan.cpp) | 137 | @file campaign_drain_scan.cpp @brief FTD-0276 Leg A: does the cluster-efficiency k_eff scale as drain²? |
| [`graviton_fft_cuda.cu`](../../engine/tests/graviton_fft_cuda.cu) | 137 | graviton_fft_cuda.cu — cuFFT (GPU) implementation of the batched double-precision 3D FFT service declared in graviton_fft_cuda.h. |
| [`test_correlations_diagonal.cpp`](../../engine/tests/test_correlations_diagonal.cpp) | 137 | @file test_correlations_diagonal.cpp @brief Validate sublattice-filtered + diagonal-displacement correlators. |
| [`test_native_evaporation_hazard_observer.cpp`](../../engine/tests/test_native_evaporation_hazard_observer.cpp) | 137 | @file test_native_evaporation_hazard_observer.cpp @brief Unit and neutrality checks for the FTD-0432 hazard observer. |
| [`test_cosmic_toggles_table.cpp`](../../engine/tests/test_cosmic_toggles_table.cpp) | 136 | Test: CosmicToggles characterization (ticket 3.3) Pins the EXACT post-construction toggle state, enable_all()/minimal() profiles, and the string get_toggle/set_toggle round-trip for CosmicEngine —... |
| [`test_langevin_sublattice_equipartition.cpp`](../../engine/tests/test_langevin_sublattice_equipartition.cpp) | 136 | @file test_langevin_sublattice_equipartition.cpp @brief Verify the Langevin thermostat with site-class filter only thermalizes the selected parity class. |
| [`test_contextual_actualization.cpp`](../../engine/tests/test_contextual_actualization.cpp) | 135 | FTD-0825 isolated contextual-actualization reference verifier. |
| [`test_gpu_strong_stress_parity.cpp`](../../engine/tests/test_gpu_strong_stress_parity.cpp) | 135 | CPU/CUDA parity for FTD-0406 strong_stress_energy. |
| [`test_db_clock_coulomb.cpp`](../../engine/tests/test_db_clock_coulomb.cpp) | 134 | ============================================================================ test_db_clock_coulomb.cpp (FTD-0281 hook smoke, 2026-06-13) ------------------------------------------------------------... |
| [`test_gpu_golden.cpp`](../../engine/tests/test_gpu_golden.cpp) | 134 | ============================================================================ test_gpu_golden.cpp — GPU-backend golden characterization (revision 0.7 GPU half / CUDA audit ticket C4; ADR-0012 amendm... |
| [`test_two_state_extraction.cpp`](../../engine/tests/test_two_state_extraction.cpp) | 134 | @file test_two_state_extraction.cpp @brief Validate Prony + GEVP two-state extractors on synthetic data. |
| [`benchmark_beta_function_pipe.cpp`](../../engine/tests/benchmark_beta_function_pipe.cpp) | 133 | @file benchmark_beta_function_pipe.cpp @brief Pipeline-based β-function benchmark — Phase E port of benchmark_beta_function. |
| [`campaign_bound_lifetime.cpp`](../../engine/tests/campaign_bound_lifetime.cpp) | 133 | Campaign: Bound State Lifetime Place free +1 and -1 at various separations on 32^3 lattice. |
| [`test_connected_block_analytic_static_refinement.cpp`](../../engine/tests/test_connected_block_analytic_static_refinement.cpp) | 133 | FTD-0638: full-coordinate Newton refinement using the FTD-0637 analytic jet. |
| [`test_particle_toggles_table.cpp`](../../engine/tests/test_particle_toggles_table.cpp) | 132 | Test: ParticleToggles characterization (ticket 3.3) Pins the EXACT post-construction toggle state, enable_all()/minimal() profiles, validate() verdicts, and the string get_toggle/set_toggle round-t... |
| [`test_continuous_translation_locality.cpp`](../../engine/tests/test_continuous_translation_locality.cpp) | 131 | FTD-0554: exact continuous translation versus strict locality. |
| [`test_annihilation_conservation.cpp`](../../engine/tests/test_annihilation_conservation.cpp) | 130 | Test: Annihilation Flux Conservation Verifies that annihilation conserves total flux energy. |
| [`test_erdos_capset.cpp`](../../engine/tests/test_erdos_capset.cpp) | 130 |  |
| [`test_knot_tracking_golden.cpp`](../../engine/tests/test_knot_tracking_golden.cpp) | 130 | ============================================================================ test_knot_tracking_golden.cpp ---------------------------------------------------------------------------- Proves the kn... |
| [`test_strict_validation.cpp`](../../engine/tests/test_strict_validation.cpp) | 130 | @file test_strict_validation.cpp @brief Verify ARCH-3 toggle-validator strictness contract. |
| [`campaign_genesis_hysteresis.cpp`](../../engine/tests/campaign_genesis_hysteresis.cpp) | 129 | @file campaign_genesis_hysteresis.cpp @brief First-order confirmation for the FTD genesis transition: HYSTERESIS. |
| [`test_gpu_evaporation_parity.cpp`](../../engine/tests/test_gpu_evaporation_parity.cpp) | 129 | ============================================================================ test_gpu_evaporation_parity.cpp — CPU↔GPU stochastic-evaporation parity (BH-F5 completion, 2026-07-16). |
| [`campaign_explicit_rounding_causal_horizon_m2.cpp`](../../engine/tests/campaign_explicit_rounding_causal_horizon_m2.cpp) | 128 | FTD-0753: fresh explicit-rounding causal-horizon M2 witness. |
| [`test_native_modal_phase_action.cpp`](../../engine/tests/test_native_modal_phase_action.cpp) | 127 | Target-blind native modal phase/action carrier regression. |
| [`benchmark_nucleon_mass.cpp`](../../engine/tests/benchmark_nucleon_mass.cpp) | 126 | @file benchmark_nucleon_mass.cpp @brief Dynamical Nucleon Mass Benchmark Tests the triad (nucleon analog) binding energy under physical fine-structure coupling limits rather than geometric limits. |
| [`test_gpu_confinement_parity.cpp`](../../engine/tests/test_gpu_confinement_parity.cpp) | 126 | CPU/CUDA parity for TermToggles::confinement. |
| [`test_gpu_symmetric_movement_parity.cpp`](../../engine/tests/test_gpu_symmetric_movement_parity.cpp) | 126 | CPU/CUDA parity for symmetric_movement_order. |
| [`test_native_moore_temporal_layers.cpp`](../../engine/tests/test_native_moore_temporal_layers.cpp) | 125 | Native Moore temporal-layer audit. |
| [`campaign_alpha_readout_scattering.cpp`](../../engine/tests/campaign_alpha_readout_scattering.cpp) | 123 | campaign_alpha_readout_scattering.cpp ARC-D1 Empirical Readout Campaign Injects a stable cluster (A=14) and applies a minimal flux perturbation (delta=0.5). |
| [`test_minimal_moore_compatibility_coat.cpp`](../../engine/tests/test_minimal_moore_compatibility_coat.cpp) | 123 | FTD-0577: minimal Moore compatibility-coat audit. |
| [`test_mobile_dressing_structure_factor_v2.cpp`](../../engine/tests/test_mobile_dressing_structure_factor_v2.cpp) | 123 | FTD-0656: corrected full rerun of the mobile dressing structure factor. |
| [`test_endpoint_schedule_underdetermination.cpp`](../../engine/tests/test_endpoint_schedule_underdetermination.cpp) | 122 | FTD-0549: endpoints do not determine the spacetime current split. |
| [`test_native_blocking_map.cpp`](../../engine/tests/test_native_blocking_map.cpp) | 121 | Native finite-volume blocking map audit. |
| [`test_phase_h_regression.cpp`](../../engine/tests/test_phase_h_regression.cpp) | 121 | @file test_phase_h_regression.cpp @brief Phase-H `coulomb_charge_coupling` knob regression. |
| [`benchmark_alpha_scaling.cpp`](../../engine/tests/benchmark_alpha_scaling.cpp) | 120 | @file benchmark_alpha_scaling.cpp @brief First productive use of the FTD-0051 GPU Langevin port: scan measure_alpha_eff across L ∈ {32, 64, 128, 256} on GPU, plus a Langevin-equilibrated variant at... |
| [`test_lorentz_bcc_time_floquet.cpp`](../../engine/tests/test_lorentz_bcc_time_floquet.cpp) | 120 | @file test_lorentz_bcc_time_floquet.cpp @brief FTD-0411 exact and live-wiring gates for the BCC-time IR surrogate. |
| [`test_scale_ratio.cpp`](../../engine/tests/test_scale_ratio.cpp) | 120 | engine/tests/test_scale_ratio.cpp Unit tests for engine/include/ftd/scale_ratio.h FC-3 (SPEC_SCALE_RATIO_ONTOLOGY.md §6) — minimal reference implementation. |
| [`bridge_fixtures.h`](../../engine/tests/support/bridge_fixtures.h) | 118 | ============================================================================ tests/support/bridge_fixtures.h ---------------------------------------------------------------------------- Phase 7 (20... |
| [`test_native_hodge_reciprocity.cpp`](../../engine/tests/test_native_hodge_reciprocity.cpp) | 117 | FTD-0575: native Hodge reciprocity and static-pole audit. |
| [`benchmark_manifestation_flow_cpu.cpp`](../../engine/tests/benchmark_manifestation_flow_cpu.cpp) | 116 | @file benchmark_manifestation_flow_cpu.cpp @brief Single-seed CPU measurement of the FTD-native b=2 flow on a manifestation-dressed background. |
| [`campaign_deterministic_canonical_current_cuda.cpp`](../../engine/tests/campaign_deterministic_canonical_current_cuda.cpp) | 116 | FTD-0749 candidate: deterministic unique-face CUDA replay. |
| [`test_cluster_genealogy.cpp`](../../engine/tests/test_cluster_genealogy.cpp) | 116 | test_cluster_genealogy — correctness gate for the genealogy detector. |
| [`campaign_ordered_current_observer_cuda.cpp`](../../engine/tests/campaign_ordered_current_observer_cuda.cpp) | 115 | FTD-0750 candidate: ordered-current and deterministic-observer CUDA replay. |
| [`benchmark_langevin_gpu.cpp`](../../engine/tests/benchmark_langevin_gpu.cpp) | 114 | @file benchmark_langevin_gpu.cpp @brief Timing benchmark for the Langevin thermostat on CPU vs GPU paths. |
| [`campaign_flux_equation_of_state.cpp`](../../engine/tests/campaign_flux_equation_of_state.cpp) | 114 | FTD-0312 Leg B — flux equation-of-state engine measurement. |
| [`test_cuda_canonical_current_deposition.cpp`](../../engine/tests/test_cuda_canonical_current_deposition.cpp) | 114 | Collision-free deterministic CUDA deposition for canonical oriented faces. |
| [`test_de_broglie_redshift.cpp`](../../engine/tests/test_de_broglie_redshift.cpp) | 113 | ============================================================================ test_de_broglie_redshift.cpp (FTD-0271 Phase A5, 2026-06-11) -----------------------------------------------------------... |
| [`test_reciprocal_moving_source_scenario.cpp`](../../engine/tests/test_reciprocal_moving_source_scenario.cpp) | 113 | @file test_reciprocal_moving_source_scenario.cpp @brief Mechanical admission gate for the FTD-0477 dashboard scenario. |
| [`test_native_response_flow.cpp`](../../engine/tests/test_native_response_flow.cpp) | 112 | Native C_L and g_sJ b=2 flow audit. |
| [`test_render_bridge_golden_default.cpp`](../../engine/tests/test_render_bridge_golden_default.cpp) | 112 | ============================================================================ test_render_bridge_golden_default.cpp — DEFAULT-PROFILE golden gate (revision 0.5b; ADR-0012 amendment). |
| [`test_cpu_warnings.cpp`](../../engine/tests/test_cpu_warnings.cpp) | 111 | @file test_cpu_warnings.cpp @brief CPU Yukawa / exchange are live pairwise channels, not GPU-only no-ops. |
| [`test_ui_observer_neutrality_gpu.cpp`](../../engine/tests/test_ui_observer_neutrality_gpu.cpp) | 111 |  |
| [`benchmark_nucleon_mass_gpu.cpp`](../../engine/tests/benchmark_nucleon_mass_gpu.cpp) | 110 | @file benchmark_nucleon_mass_gpu.cpp @brief GPU Dynamical Nucleon Mass Benchmark Tests the triad (nucleon analog) binding energy under physical fine-structure coupling limits using the CUDA engine. |
| [`campaign_native_scale_flow.cpp`](../../engine/tests/campaign_native_scale_flow.cpp) | 109 |  |
| [`test_lattice.cpp`](../../engine/tests/test_lattice.cpp) | 109 | Test: Lattice operations Verifies periodic boundary conditions, neighbor access, and coordinate mapping. |
| [`test_native_field_discrete_action.cpp`](../../engine/tests/test_native_field_discrete_action.cpp) | 109 | FTD-0574: native field discrete action and source-operator audit. |
| [`test_noncompact_face_cohomology.cpp`](../../engine/tests/test_noncompact_face_cohomology.cpp) | 109 | FTD-0583: noncompact matched-face cohomology/local-carrier gate. |
| [`test_ontic_chain.cpp`](../../engine/tests/test_ontic_chain.cpp) | 109 | Test: Ontic Derivation Chain — γ → Γ(1/4) → ϖ → G* → α → all physics Pure mathematics. |
| [`test_relativistic_verlet.cpp`](../../engine/tests/test_relativistic_verlet.cpp) | 109 | @file test_relativistic_verlet.cpp @brief Relativistic Verlet integrator speed cap and momentum verification test. |
| [`test_sublattice_helpers.cpp`](../../engine/tests/test_sublattice_helpers.cpp) | 108 | @file test_sublattice_helpers.cpp @brief Unit tests for sublattice classification and neighbors_8_corner. |
| [`test_lorentz_period2_floquet.cpp`](../../engine/tests/test_lorentz_period2_floquet.cpp) | 107 | @file test_lorentz_period2_floquet.cpp @brief FTD-0408 exact and engine-wiring gates for the P4 period-two wave map. |
| [`test_manifestation_background.cpp`](../../engine/tests/test_manifestation_background.cpp) | 106 | @file test_manifestation_background.cpp @brief Unit tests for prepare_manifestation_background. |
| [`test_dissipation.cpp`](../../engine/tests/test_dissipation.cpp) | 105 | Test: Rayleigh Dissipation Function Verifies R = (DAMPING/2) * \|wave_vel\|^2 where DAMPING = alpha [IMPOSED — see ontic.h ASSUMP.6]. |
| [`test_dynamic_flux_dressing_scenario.cpp`](../../engine/tests/test_dynamic_flux_dressing_scenario.cpp) | 105 | @file test_dynamic_flux_dressing_scenario.cpp @brief Behavioral admission gate for s0-seed-dynamical-flux-dressing. |
| [`test_phase_h_coupling.cpp`](../../engine/tests/test_phase_h_coupling.cpp) | 105 | test_phase_h_coupling.cpp — Phase H: explicit coupling constant in Gauss law. |
| [`test_sloop.cpp`](../../engine/tests/test_sloop.cpp) | 105 | Test: Reference frame context Constants and Quadratic Structure Verifies that the reference frame context-sector constants from ontic.h Layer 8 are correctly derived and internally consistent. |
| [`test_dual_cell_adapter.cpp`](../../engine/tests/test_dual_cell_adapter.cpp) | 104 | @file test_dual_cell_adapter.cpp @brief Unit tests for render_bridge_to_dual_cell_fields. |
| [`probe_cpu_current_support.cpp`](../../engine/tests/probe_cpu_current_support.cpp) | 101 | FTD-0748 pre-lock probe: measure the original FTD-0745 CPU support. |
| [`test_gpu_profile_compare.cpp`](../../engine/tests/test_gpu_profile_compare.cpp) | 100 | Compare inner radial profiles across damping modes. |
| [`test_ten_source_shared_m_coherence.cpp`](../../engine/tests/test_ten_source_shared_m_coherence.cpp) | 99 |  |
| [`test_native_flow.cpp`](../../engine/tests/test_native_flow.cpp) | 98 | Native bare-flow audit for the dual-cell blocking map. |
| [`test_native_retarded_polarity_response.cpp`](../../engine/tests/test_native_retarded_polarity_response.cpp) | 97 | Unit checks for the read-only FTD-0430 moving-source observer. |
| [`test_external_drive_radiation.cpp`](../../engine/tests/test_external_drive_radiation.cpp) | 96 | FTD-0559: exact external-drive field-energy functional. |
| [`test_native_gauss_monopole_dichotomy.cpp`](../../engine/tests/test_native_gauss_monopole_dichotomy.cpp) | 96 | FTD-0563: Gauss monopole / mobile-dressing dichotomy. |
| [`test_passive_dressing_depinning_obstruction.cpp`](../../engine/tests/test_passive_dressing_depinning_obstruction.cpp) | 96 | FTD-0581: passive-dressing/depinning observer. |
| [`campaign_ew_phase_transition.cpp`](../../engine/tests/campaign_ew_phase_transition.cpp) | 95 |  |
| [`test_native_hodge_energy_continuity.cpp`](../../engine/tests/test_native_hodge_energy_continuity.cpp) | 95 | FTD-0576: native Hodge energy and central-continuity audit. |
| [`test_knot_telemetry.cpp`](../../engine/tests/test_knot_telemetry.cpp) | 94 | engine/tests/test_knot_telemetry.cpp Unit test for KnotTracker: per-knot lifecycle + observable assembly. |
| [`test_open5_legacy_flux_l.cpp`](../../engine/tests/test_open5_legacy_flux_l.cpp) | 94 | @file test_open5_legacy_flux_l.cpp @brief OPEN-5 micro-regression: legacy single-substrate inject_flux must leave flux_L untouched when toggles.dual_substrate=false. |
| [`test_field_soa.cpp`](../../engine/tests/test_field_soa.cpp) | 93 |  |
| [`test_voxel_layout.cpp`](../../engine/tests/test_voxel_layout.cpp) | 93 | @file test_voxel_layout.cpp @brief Voxel memory-layout characterization guard (revision 0.8). |
| [`test_fixed_step_energy_scope.cpp`](../../engine/tests/test_fixed_step_energy_scope.cpp) | 92 | FTD-0543: fixed-step action energy is not an automatic consequence. |
| [`test_causal_excitation_separation_preflight.cpp`](../../engine/tests/test_causal_excitation_separation_preflight.cpp) | 91 | Diagnostic-only initialization probe following execution-invalid FTD-0684. |
| [`test_connected_block_analytic_dynamical_rest.cpp`](../../engine/tests/test_connected_block_analytic_dynamical_rest.cpp) | 91 | FTD-0639: common-action rest and state-only inversion of FTD-0638. |
| [`test_ten_source_temporal_product_capacity.cpp`](../../engine/tests/test_ten_source_temporal_product_capacity.cpp) | 90 |  |
| [`test_native_moving_source_pole.cpp`](../../engine/tests/test_native_moving_source_pole.cpp) | 88 | FTD-0558: native moving-source pole correction. |
| [`test_endogenous_reaction_carrier_bound.cpp`](../../engine/tests/test_endogenous_reaction_carrier_bound.cpp) | 87 | FTD-0586: endogenous reaction-carrier/autocatalysis bound. |
| [`test_common_moore_worldline_action.cpp`](../../engine/tests/test_common_moore_worldline_action.cpp) | 86 | FTD-0578: common Moore spacetime/action and self-force audit. |
| [`test_connected_block_independent_field_modes.cpp`](../../engine/tests/test_connected_block_independent_field_modes.cpp) | 86 | FTD-0641: independent source-free face/edge modes on the dressed background. |
| [`test_genesis_reservoir_dilation.cpp`](../../engine/tests/test_genesis_reservoir_dilation.cpp) | 85 | FTD-0569: exact one-event genesis reservoir dilation and cycle obstruction. |
| [`test_render_bridge_golden_l9.cpp`](../../engine/tests/test_render_bridge_golden_l9.cpp) | 85 | ============================================================================ test_render_bridge_golden_l9.cpp — secondary lattice-size golden (L=9) (revision 0.7 CPU half; ADR-0012 amendment / mult... |
| [`test_boundary_movement.cpp`](../../engine/tests/test_boundary_movement.cpp) | 84 | test_boundary_movement.cpp Verifies phase_movement face handling with an in-budget raw velocity and accumulated movement remainder: reflective_boundary OFF → particle exhausts into the void (no tor... |
| [`test_connected_block_full_constituent_hessian.cpp`](../../engine/tests/test_connected_block_full_constituent_hessian.cpp) | 84 | FTD-0634: complete 48-coordinate adiabatic Hessian. |
| [`test_native_reaction_polarity_slow_mode.cpp`](../../engine/tests/test_native_reaction_polarity_slow_mode.cpp) | 84 | Unit checks for the FTD-0431 reaction-mode observer. |
| [`test_site_ontic_atomic_reciprocal_hop.cpp`](../../engine/tests/test_site_ontic_atomic_reciprocal_hop.cpp) | 84 |  |
| [`test_ten_source_pair_distance_capacity.cpp`](../../engine/tests/test_ten_source_pair_distance_capacity.cpp) | 82 |  |
| [`test_native_active_mode_backreaction.cpp`](../../engine/tests/test_native_active_mode_backreaction.cpp) | 81 | FTD-0582: frozen native active-mode backreaction discriminator. |
| [`test_native_ternary_plaquette_quarter_turn.cpp`](../../engine/tests/test_native_ternary_plaquette_quarter_turn.cpp) | 81 |  |
| [`test_genesis_cubic_canonical_form.cpp`](../../engine/tests/test_genesis_cubic_canonical_form.cpp) | 80 | FTD-0573: O_h canonical-form uniqueness and bath-rank price. |
| [`test_genesis_natural_extension.cpp`](../../engine/tests/test_genesis_natural_extension.cpp) | 80 | FTD-0570: exact-real natural extension and symplectic genesis boundary. |
| [`campaign_higgs_bi_pair_production.cpp`](../../engine/tests/campaign_higgs_bi_pair_production.cpp) | 79 |  |
| [`test_configuration_space_carrier.cpp`](../../engine/tests/test_configuration_space_carrier.cpp) | 79 | FTD-0584: fixed-source configuration-space carrier necessity gate. |
| [`gauge_test_utils.h`](../../engine/tests/support/gauge_test_utils.h) | 78 | ============================================================================ gauge_test_utils.h — shared helpers for the SU(2)/SU(3) gauge-sector tests (test_gauge_links.cpp, test_gauge_gpu_parity.... |
| [`test_native_motion_reaction_front.cpp`](../../engine/tests/test_native_motion_reaction_front.cpp) | 78 | FTD-0585: native transport/reaction/source-memory discriminator. |
| [`test_symplectic_wave.cpp`](../../engine/tests/test_symplectic_wave.cpp) | 78 | @file test_symplectic_wave.cpp @brief Symplectic Leapfrog wave propagation energy conservation test. |
| [`test_connected_moore_block_solve_cache.cpp`](../../engine/tests/test_connected_moore_block_solve_cache.cpp) | 77 | Engineering equivalence gate for the observer-only repeated-root cache. |
| [`test_ten_source_distance_distribution_lp.cpp`](../../engine/tests/test_ten_source_distance_distribution_lp.cpp) | 77 |  |
| [`test_native_dynamic_polarity_response.cpp`](../../engine/tests/test_native_dynamic_polarity_response.cpp) | 76 | Unit checks for the read-only FTD-0429 Fourier observer. |
| [`test_native_hop_dressing_obstruction.cpp`](../../engine/tests/test_native_hop_dressing_obstruction.cpp) | 75 | FTD-0560: periodic point-hop co-moving dressing obstruction. |
| [`test_volumetric_measure.cpp`](../../engine/tests/test_volumetric_measure.cpp) | 75 | FTD-0404: cubic cell measure and density/integral separation. |
| [`test_ten_source_orbit_coherence.cpp`](../../engine/tests/test_ten_source_orbit_coherence.cpp) | 74 |  |
| [`test_eight_source_orbit_coherence.cpp`](../../engine/tests/test_eight_source_orbit_coherence.cpp) | 73 |  |
| [`test_gf3_codes.cpp`](../../engine/tests/test_gf3_codes.cpp) | 73 |  |
| [`test_ternary_field.cpp`](../../engine/tests/test_ternary_field.cpp) | 73 |  |
| [`test_blume_capel_glass.cpp`](../../engine/tests/test_blume_capel_glass.cpp) | 72 |  |
| [`test_native_injectivity_gate.cpp`](../../engine/tests/test_native_injectivity_gate.cpp) | 72 |  |
| [`test_nine_source_orbit_coherence.cpp`](../../engine/tests/test_nine_source_orbit_coherence.cpp) | 72 |  |
| [`test_connected_block_analytic_matter_modes.cpp`](../../engine/tests/test_connected_block_analytic_matter_modes.cpp) | 71 | FTD-0640: complete analytic matter-mode response about the FTD-0638 center. |
| [`test_matched_action_normalization.cpp`](../../engine/tests/test_matched_action_normalization.cpp) | 71 | FTD-0486: exact coefficient no-go for the selected matched action. |
| [`dump_full_physics_l256.cpp`](../../engine/tests/dump_full_physics_l256.cpp) | 69 | L=256 full-physics spot check — does the FTD-framework-integer pattern continue? |
| [`test_full_surface_source_obstruction.cpp`](../../engine/tests/test_full_surface_source_obstruction.cpp) | 69 | FTD-0562: finite rigid source full-resonance obstruction. |
| [`test_genesis_minimal_bath.cpp`](../../engine/tests/test_genesis_minimal_bath.cpp) | 69 | FTD-0572: minimum bath-rank and prepared-dilation theorem. |
| [`test_potts_3d.cpp`](../../engine/tests/test_potts_3d.cpp) | 69 |  |
| [`dump_full_physics_amp_scan.cpp`](../../engine/tests/dump_full_physics_amp_scan.cpp) | 68 | Multi-amplitude scan under FULL PHYSICS at L=64. |
| [`test_aperiodic_monotile.cpp`](../../engine/tests/test_aperiodic_monotile.cpp) | 68 | _symbols:_ Tile |
| [`test_scenario_meta.cpp`](../../engine/tests/test_scenario_meta.cpp) | 68 |  |
| [`test_finite_rigid_moore_carrier_obstruction.cpp`](../../engine/tests/test_finite_rigid_moore_carrier_obstruction.cpp) | 67 | FTD-0579: finite rigid Moore-carrier obstruction audit. |
| [`test_hop_source_multipole_hierarchy.cpp`](../../engine/tests/test_hop_source_multipole_hierarchy.cpp) | 67 | FTD-0561: periodic-hop finite-source multipole hierarchy. |
| [`test_symmetric_chord_moore_action.cpp`](../../engine/tests/test_symmetric_chord_moore_action.cpp) | 67 | FTD-0580: symmetric chord Moore-action observer. |
| [`test_telemetry_selftest.cpp`](../../engine/tests/test_telemetry_selftest.cpp) | 67 | Test: ftd::test telemetry library self-test Exercises every public method of ftd/test_telemetry.h in both modes: - FTD_TEST_TELEMETRY unset → human-readable output (matches legacy check()/check_clo... |
| [`test_3d_ca.cpp`](../../engine/tests/test_3d_ca.cpp) | 66 |  |
| [`test_genesis_environment_feedback.cpp`](../../engine/tests/test_genesis_environment_feedback.cpp) | 65 | FTD-0571: environment-feedback necessity for noncanonical genesis. |
| [`test_orientation_gauss_independence.cpp`](../../engine/tests/test_orientation_gauss_independence.cpp) | 65 | FTD-0564: orientation degree and electric Gauss flux are independent. |
| [`test_removal_time_orbit_coherence.cpp`](../../engine/tests/test_removal_time_orbit_coherence.cpp) | 62 |  |
| [`test_connected_moore_block_matrix_free_solve.cpp`](../../engine/tests/test_connected_moore_block_matrix_free_solve.cpp) | 61 | Engineering equivalence gate for the Jacobian-free common-action solver. |
| [`test_fox_coloring.cpp`](../../engine/tests/test_fox_coloring.cpp) | 61 |  |
| [`test_genesis_action_obstruction.cpp`](../../engine/tests/test_genesis_action_obstruction.cpp) | 59 | FTD-0567: production genesis does not lock amplitude or share the written action. |
| [`test_mass_metric_modal_energy.cpp`](../../engine/tests/test_mass_metric_modal_energy.cpp) | 59 | FTD-0675: canonical mass metric for connected tangent-mode energy. |
| [`test_analytic_center_collective_boost_ladder.cpp`](../../engine/tests/test_analytic_center_collective_boost_ladder.cpp) | 56 | FTD-0643: finite collective boost ladder from the analytic center. |
| [`test_native_observable_registry.cpp`](../../engine/tests/test_native_observable_registry.cpp) | 56 | @file test_native_observable_registry.cpp @brief Seed observable-registry contract test. |
| [`test_connected_block_coupled_transverse_response.cpp`](../../engine/tests/test_connected_block_coupled_transverse_response.cpp) | 54 | FTD-0642: coupled response of exact-center matter to transverse field modes. |
| [`test_connected_block_translation_curvature.cpp`](../../engine/tests/test_connected_block_translation_curvature.cpp) | 52 | FTD-0630: three-axis translation curvature and fully-half control. |
| [`test_pole_matching_contract.cpp`](../../engine/tests/test_pole_matching_contract.cpp) | 52 |  |
| [`test_connected_block_cubic_eight_fibre.cpp`](../../engine/tests/test_connected_block_cubic_eight_fibre.cpp) | 39 | FTD-0632: finite cubic chart-fibre derivation and qualification. |
| [`test_analytic_center_long_horizon_transport.cpp`](../../engine/tests/test_analytic_center_long_horizon_transport.cpp) | 37 | FTD-0646: long-horizon low-momentum transport discriminator. |
| [`test_engine_select_cpu.cpp`](../../engine/tests/test_engine_select_cpu.cpp) | 34 | CPU-only SimEngine selection regression. |
| [`test_ui_thread_guard.cpp`](../../engine/tests/test_ui_thread_guard.cpp) | 34 |  |
| [`test_constructor_contract.cpp`](../../engine/tests/test_constructor_contract.cpp) | 33 | @file test_constructor_contract.cpp @brief Constructor-domain metadata helper smoke test. |
| [`test_face_flux_normalization.cpp`](../../engine/tests/test_face_flux_normalization.cpp) | 33 | Focused compatibility test for the selected matched-face/native-J scale. |
| [`test_interop_particle_record_layout.cpp`](../../engine/tests/test_interop_particle_record_layout.cpp) | 32 | engine/tests/test_interop_particle_record_layout.cpp |
| [`test_analytic_center_collective_boost_ladder_v2.cpp`](../../engine/tests/test_analytic_center_collective_boost_ladder_v2.cpp) | 31 | FTD-0644: corrected finite collective boost ladder. |
| [`test_causal_horizon_csv_loader.cpp`](../../engine/tests/test_causal_horizon_csv_loader.cpp) | 25 | Regression for the cross-platform FTD-0745 baseline loader. |
| [`phase_i_green_fixtures.h`](../../engine/tests/phase_i_green_fixtures.h) | 24 | Auto-generated by scripts/proofs/generate_phase_i_lattice_green_fixtures.py G18 Poisson Green's function values at selected (L, r) pairs along x-axis. |
| [`test_internal_excitation_symmetry_ray_spectrum_v1.cpp`](../../engine/tests/test_internal_excitation_symmetry_ray_spectrum_v1.cpp) | 23 | FTD-0698: fresh held-out amplitude symmetry-ray matter spectrum. |
| [`test_internal_excitation_symmetry_ray_spectrum_v2.cpp`](../../engine/tests/test_internal_excitation_symmetry_ray_spectrum_v2.cpp) | 23 | FTD-0699: corrected discrete-phase classifier at a fresh amplitude. |
| [`test_connected_block_full_constituent_hessian_v2.cpp`](../../engine/tests/test_connected_block_full_constituent_hessian_v2.cpp) | 22 | FTD-0635: separately conditioned first derivative for the 48D Hessian rig. |
| [`test_connected_block_knot_local_hessian.cpp`](../../engine/tests/test_connected_block_knot_local_hessian.cpp) | 22 | FTD-0636: full Hessian restricted to one C1-kernel polynomial sector. |
| [`test_causal_excitation_separation_indexed_local_v1.cpp`](../../engine/tests/test_causal_excitation_separation_indexed_local_v1.cpp) | 18 | FTD-0694: indexed local-root execution of the L=113 discriminator. |
| [`test_causal_excitation_separation_local_v1.cpp`](../../engine/tests/test_causal_excitation_separation_local_v1.cpp) | 18 | FTD-0693: qualified local-root execution of the L=113 discriminator. |
| [`test_connected_block_eight_fibre_static_basin.cpp`](../../engine/tests/test_connected_block_eight_fibre_static_basin.cpp) | 17 | FTD-0633: cap-eight instantiation of the locked fully-half refinement rig. |
| [`test_causal_excitation_separation_block_v1.cpp`](../../engine/tests/test_causal_excitation_separation_block_v1.cpp) | 15 | FTD-0691: four-tick exact block-transport sampling. |
| [`test_analytic_center_collective_boost_ladder_v3.cpp`](../../engine/tests/test_analytic_center_collective_boost_ladder_v3.cpp) | 14 | FTD-0645: covariant soft-basis correction for the collective boost ladder. |
| [`test_causal_excitation_separation_l113_v1.cpp`](../../engine/tests/test_causal_excitation_separation_l113_v1.cpp) | 14 | FTD-0690: executable causal extension beyond tick 80. |
| [`campaign_blocked_hop_work_decomposition_v2.cpp`](../../engine/tests/campaign_blocked_hop_work_decomposition_v2.cpp) | 11 | FTD-0460 v2 wrapper: preserve the locked v1 campaign body while replacing only its pathological observer tick with the algebraically identical snapshot implementation. |
| [`test_causal_excitation_separation_v3.cpp`](../../engine/tests/test_causal_excitation_separation_v3.cpp) | 9 | FTD-0687: exact batched-regional execution of the frozen campaign. |
| [`test_causal_excitation_separation_v4.cpp`](../../engine/tests/test_causal_excitation_separation_v4.cpp) | 9 | FTD-0689: prefix-sum regional execution of the frozen campaign. |
| [`test_causal_excitation_separation_v2.cpp`](../../engine/tests/test_causal_excitation_separation_v2.cpp) | 8 | FTD-0685: sole correction is a numeric center-preflight tolerance. |
| [`test_l17_complete_tangent_nonsingular_product_chart_v5.cpp`](../../engine/tests/test_l17_complete_tangent_nonsingular_product_chart_v5.cpp) | 8 | FTD-0832: explicit electric harmonic direct sum and nonsingular complete product-chart norm, inheriting every physical gate from FTD-0774. |
| [`test_l17_complete_tangent_representability_floor_v4.cpp`](../../engine/tests/test_l17_complete_tangent_representability_floor_v4.cpp) | 8 | FTD-0831: binary64 backward-error floor for the retained face-harmonic coordinate, inheriting the FTD-0829 and FTD-0830 certificate repairs. |
| [`test_l17_complete_tangent_certificate_repair_v2.cpp`](../../engine/tests/test_l17_complete_tangent_certificate_repair_v2.cpp) | 7 | FTD-0829: target-blind certificate repair for the locked FTD-0774 physics campaign. |
| [`test_l17_complete_tangent_harmonic_reinsertion_repair_v3.cpp`](../../engine/tests/test_l17_complete_tangent_harmonic_reinsertion_repair_v3.cpp) | 7 | FTD-0830: target-blind stable reinsertion of the explicitly retained uniform face coordinates, inheriting both FTD-0829 certificate repairs. |

### `other`  (806 files, 163,084 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`crc32.h`](../../engine/thirdparty/freetype/src/gzip/crc32.h) | 9446 | crc32.h -- tables for rapid CRC calculation Generated automatically by crc32.c |
| [`freetype.h`](../../engine/thirdparty/freetype/include/freetype/freetype.h) | 5289 | freetype.h FreeType high-level API and common types (specification only). |
| [`pstables.h`](../../engine/thirdparty/freetype/src/psnames/pstables.h) | 4238 |  |
| [`Element.cpp`](../../engine/thirdparty/rmlui/Source/Core/Element.cpp) | 3017 | dimensions |
| [`run_app.cpp`](../../engine/native/src/app/run_app.cpp) | 2614 | native_app — the live windowed FTD native application (M-UI-1..M-UI-3 fused). |
| [`robin_hood.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Containers/robin_hood.h) | 2401 | ______ _____ ______ _________ ______________ ___ /_ ___(_)_______ ___ /_ ______ ______ ______ / __ ___/_ __ \__ __ \__ / __ __ \ __ __ \_ __ \_ __ \_ __ / _ / / /_/ /_ /_/ /_ / _ / / / _ / / // /_/... |
| [`scale0_adapter.cpp`](../../engine/native/src/host/adapters/scale0_adapter.cpp) | 2062 | host/adapters/scale0_adapter.cpp — Scale 0 (RenderBridge) behind the seam. |
| [`zlib.h`](../../engine/thirdparty/freetype/src/gzip/zlib.h) | 1972 | zlib.h -- interface of the 'zlib' general purpose compression library version 1.3, August 18th, 2023 Copyright (C) 1995-2023 Jean-loup Gailly and Mark Adler This software is provided 'as-is', witho... |
| [`tttypes.h`](../../engine/thirdparty/freetype/include/freetype/internal/tttypes.h) | 1747 | tttypes.h Basic SFNT/TrueType type definitions and interface (specification only). |
| [`d3d12_presenter.cpp`](../../engine/native/src/d3d12_presenter.cpp) | 1745 | _symbols:_ GpuVertex, CameraConstants, VSIn, VSOut |
| [`ftcolor.h`](../../engine/thirdparty/freetype/include/freetype/ftcolor.h) | 1667 | ftcolor.h FreeType's glyph color management (specification). |
| [`Context.cpp`](../../engine/thirdparty/rmlui/Source/Core/Context.cpp) | 1627 | _symbols:_ ElementObserverListBackInserter |
| [`WidgetTextInput.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/WidgetTextInput.cpp) | 1580 | value |
| [`psaux.h`](../../engine/thirdparty/freetype/include/freetype/internal/psaux.h) | 1447 | psaux.h Auxiliary functions and data structures related to PostScript fonts (specification). |
| [`ftdriver.h`](../../engine/thirdparty/freetype/include/freetype/ftdriver.h) | 1320 | ftdriver.h FreeType API for controlling driver modules (specification only). |
| [`ftimage.h`](../../engine/thirdparty/freetype/include/freetype/ftimage.h) | 1289 | ftimage.h FreeType glyph image formats and default raster interface (specification). |
| [`ttnameid.h`](../../engine/thirdparty/freetype/include/freetype/ttnameid.h) | 1235 | ttnameid.h TrueType name ID definitions (specification only). |
| [`ftobjs.h`](../../engine/thirdparty/freetype/include/freetype/internal/ftobjs.h) | 1232 | ftobjs.h The FreeType private base classes (specification). |
| [`DataExpression.cpp`](../../engine/thirdparty/rmlui/Source/Core/DataExpression.cpp) | 1218 | The abstract machine for RmlUi data expressions. |
| [`StyleSheetParser.cpp`](../../engine/thirdparty/rmlui/Source/Core/StyleSheetParser.cpp) | 1129 | PropertySpecificationParser just passes the parsing to a property specification. |
| [`sfnt.h`](../../engine/thirdparty/freetype/include/freetype/internal/sfnt.h) | 1099 | sfnt.h High-level 'sfnt' driver interface (specification). |
| [`ftcache.h`](../../engine/thirdparty/freetype/include/freetype/ftcache.h) | 1087 | ftcache.h FreeType Cache subsystem (specification). |
| [`ftoption.h`](../../engine/thirdparty/freetype/include/freetype/config/ftoption.h) | 1030 | ftoption.h User-selectable configuration macros (specification only). |
| [`FlexFormattingContext.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/FlexFormattingContext.cpp) | 972 | _symbols:_ FlexItem, Size, FlexLine, FlexLineContainer |
| [`ElementStyle.cpp`](../../engine/thirdparty/rmlui/Source/Core/ElementStyle.cpp) | 919 |  |
| [`tttables.h`](../../engine/thirdparty/freetype/include/freetype/tttables.h) | 856 | tttables.h Basic SFNT/TrueType tables definitions and interface (specification only). |
| [`ElementDocument.cpp`](../../engine/thirdparty/rmlui/Source/Core/ElementDocument.cpp) | 840 | content |
| [`ftheader.h`](../../engine/thirdparty/freetype/include/freetype/config/ftheader.h) | 836 |  |
| [`ftmm.h`](../../engine/thirdparty/freetype/include/freetype/ftmm.h) | 834 | ftmm.h FreeType Multiple Master font interface (specification). |
| [`ftmodapi.h`](../../engine/thirdparty/freetype/include/freetype/ftmodapi.h) | 807 | ftmodapi.h FreeType modules public interface (specification). |
| [`Element.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Element.h) | 797 | A generic element in the DOM tree. |
| [`ElementAnimation.cpp`](../../engine/thirdparty/rmlui/Source/Core/ElementAnimation.cpp) | 785 | An abstraction for decorator and filter declarations. |
| [`ftstroke.h`](../../engine/thirdparty/freetype/include/freetype/ftstroke.h) | 773 |  |
| [`TransformUtilities.cpp`](../../engine/thirdparty/rmlui/Source/Core/TransformUtilities.cpp) | 773 | p |
| [`ftglyph.h`](../../engine/thirdparty/freetype/include/freetype/ftglyph.h) | 750 | ftglyph.h FreeType convenience functions to handle glyphs (specification). |
| [`t1tables.h`](../../engine/thirdparty/freetype/include/freetype/t1tables.h) | 735 | t1tables.h Basic Type 1/Type 2 tables definitions and interface (specification only). |
| [`ElementText.cpp`](../../engine/thirdparty/rmlui/Source/Core/ElementText.cpp) | 722 | _symbols:_ TextOverflowResolved |
| [`pshints.h`](../../engine/thirdparty/freetype/include/freetype/internal/pshints.h) | 699 | pshints.h Interface to Postscript-specific (Type 1 and Type 2) hints recorders (specification only). |
| [`DecoratorGradient.cpp`](../../engine/thirdparty/rmlui/Source/Core/DecoratorGradient.cpp) | 697 | interface_ |
| [`streamlines.cpp`](../../engine/native/src/host/adapters/streamlines.cpp) | 647 | host/adapters/streamlines.cpp — CPU RK4 field-line integrator for the Scale-0 STREAMLINE overlays. |
| [`WidgetDropDown.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/WidgetDropDown.cpp) | 647 |  |
| [`TableFormattingDetails.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/TableFormattingDetails.cpp) | 643 | _symbols:_ TrackAvailableSize |
| [`StringUtilities.cpp`](../../engine/thirdparty/rmlui/Source/Core/StringUtilities.cpp) | 626 |  |
| [`URL.cpp`](../../engine/thirdparty/rmlui/Source/Core/URL.cpp) | 619 |  |
| [`fttypes.h`](../../engine/thirdparty/freetype/include/freetype/fttypes.h) | 617 | fttypes.h FreeType simple types definitions (specification only). |
| [`WidgetSlider.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/WidgetSlider.cpp) | 616 |  |
| [`Factory.cpp`](../../engine/thirdparty/rmlui/Source/Core/Factory.cpp) | 608 | _symbols:_ DefaultInstancers, FactoryData |
| [`DataViewDefault.cpp`](../../engine/thirdparty/rmlui/Source/Core/DataViewDefault.cpp) | 602 | expression |
| [`PropertySpecification.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertySpecification.cpp) | 602 |  |
| [`command_applier.cpp`](../../engine/native/src/command_applier.cpp) | 601 |  |
| [`rml_d3d12_renderer.cpp`](../../engine/native/src/ui/rml_d3d12_renderer.cpp) | 591 | w |
| [`ftoutln.h`](../../engine/thirdparty/freetype/include/freetype/ftoutln.h) | 588 | ftoutln.h Support for the FT_Outline type used to store glyph shapes of most scalable font formats (specification). |
| [`ftcalc.h`](../../engine/thirdparty/freetype/include/freetype/internal/ftcalc.h) | 584 |  |
| [`gxvcommn.h`](../../engine/thirdparty/freetype/src/gxvalid/gxvcommn.h) | 581 | gxvcommn.h TrueTypeGX/AAT common tables validation (specification). |
| [`ui_model.cpp`](../../engine/native/src/app/ui_model.cpp) | 576 | app/ui_model.cpp — builders for the RmlUi data model (see app/ui_model.h). |
| [`ftstream.h`](../../engine/thirdparty/freetype/include/freetype/internal/ftstream.h) | 570 |  |
| [`LayoutDetails.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/LayoutDetails.cpp) | 558 |  |
| [`FreeTypeInterface.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FreeTypeInterface.cpp) | 555 |  |
| [`ftzconf.h`](../../engine/thirdparty/freetype/src/gzip/ftzconf.h) | 551 | zconf.h -- configuration of the zlib compression library Copyright (C) 1995-2016 Jean-loup Gailly, Mark Adler For conditions of distribution and use, see copyright notice in zlib.h |
| [`PropertyParserColour.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserColour.cpp) | 548 | parameters |
| [`BlockContainer.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/BlockContainer.cpp) | 535 |  |
| [`Variant.cpp`](../../engine/thirdparty/rmlui/Source/Core/Variant.cpp) | 533 |  |
| [`BaseXMLParser.cpp`](../../engine/thirdparty/rmlui/Source/Core/BaseXMLParser.cpp) | 514 | name |
| [`aftypes.h`](../../engine/thirdparty/freetype/src/autofit/aftypes.h) | 511 |  |
| [`WidgetScroll.cpp`](../../engine/thirdparty/rmlui/Source/Core/WidgetScroll.cpp) | 504 |  |
| [`ftserv.h`](../../engine/thirdparty/freetype/include/freetype/internal/ftserv.h) | 495 | ftserv.h The FreeType services (specification only). |
| [`ElementUtilities.cpp`](../../engine/thirdparty/rmlui/Source/Core/ElementUtilities.cpp) | 490 | _symbols:_ ViewControllerInitializer |
| [`afstyles.h`](../../engine/thirdparty/freetype/src/autofit/afstyles.h) | 487 | afstyles.h Auto-fitter styles (specification only). |
| [`test_ui_rml_smoke.cpp`](../../engine/native/tests/test_ui_rml_smoke.cpp) | 469 | M-UI-1 headless smoke test: render the FTD native shell (shell.rml + ftd.rcss) through RmlD3D12Renderer into an offscreen 1280x800 RGBA8 render target, read it back, assert a meaningful fraction of... |
| [`otvcommn.h`](../../engine/thirdparty/freetype/src/otvalid/otvcommn.h) | 468 | otvcommn.h OpenType common tables validation (specification). |
| [`afhints.h`](../../engine/thirdparty/freetype/src/autofit/afhints.h) | 467 | afhints.h Auto-fitter hinting routines (specification). |
| [`Matrix4.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Matrix4.h) | 467 | Templated class that acts as base strategy for vectors access patterns of matrices. |
| [`FontFaceHandleDefault.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FontFaceHandleDefault.cpp) | 466 |  |
| [`TableFormattingContext.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/TableFormattingContext.cpp) | 466 | The table height algorithm works similar to the table width algorithm. |
| [`ttinterp.h`](../../engine/thirdparty/freetype/src/truetype/ttinterp.h) | 465 | ttinterp.h TrueType bytecode interpreter (specification). |
| [`Core.cpp`](../../engine/thirdparty/rmlui/Source/Core/Core.cpp) | 460 | _symbols:_ CoreData |
| [`ttgxvar.h`](../../engine/thirdparty/freetype/src/truetype/ttgxvar.h) | 453 | ttgxvar.h TrueType GX Font Variation loader (specification) Copyright (C) 2004-2024 by David Turner, Robert Wilhelm, Werner Lemberg and George Williams. |
| [`Context.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Context.h) | 453 | A context for storing, rendering, and processing RML documents. |
| [`ui_model.h`](../../engine/native/src/app/ui_model.h) | 452 | app/ui_model.h — the RmlUi data-model layer of native_app: the C++ mirror of the shell (ShellData + its row types) plus the config-knob spec table, split out of app/main.cpp for readability (behavi... |
| [`ftdebug.h`](../../engine/thirdparty/freetype/include/freetype/internal/ftdebug.h) | 442 | ftdebug.h Debugging and logging component (specification). |
| [`StyleSheetSpecification.cpp`](../../engine/thirdparty/rmlui/Source/Core/StyleSheetSpecification.cpp) | 440 | Style property specifications (ala RCSS). |
| [`ComputedValues.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/ComputedValues.h) | 438 | A computed value is a value resolved as far as possible :before: introducing layouting. |
| [`afblue.h`](../../engine/thirdparty/freetype/src/autofit/afblue.h) | 429 | This file has been generated by the Perl script `afblue.pl', |
| [`ttobjs.h`](../../engine/thirdparty/freetype/src/truetype/ttobjs.h) | 426 |  |
| [`cfftypes.h`](../../engine/thirdparty/freetype/include/freetype/internal/cfftypes.h) | 416 | cfftypes.h Basic OpenType/CFF type definitions and interface (specification only). |
| [`LineBox.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/LineBox.cpp) | 415 | fragment |
| [`afscript.h`](../../engine/thirdparty/freetype/src/autofit/afscript.h) | 408 | afscript.h Auto-fitter scripts (specification only). |
| [`scale0_overlays.h`](../../engine/native/include/native/scale0_overlays.h) | 403 | scale0_overlays.h — the data-driven Scale-0 overlay registry. |
| [`ftmemory.h`](../../engine/thirdparty/freetype/include/freetype/internal/ftmemory.h) | 401 | ftmemory.h The FreeType memory management macros (specification). |
| [`StyleSheetNode.cpp`](../../engine/thirdparty/rmlui/Source/Core/StyleSheetNode.cpp) | 392 |  |
| [`flat_map.hpp`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Containers/itlib/flat_map.hpp) | 387 | itlib-flat-map v1.07 std::map-like class with an underlying vector SPDX-License-Identifier: MIT MIT License: Copyright(c) 2016-2019 Chobolabs Inc. |
| [`DataModel.cpp`](../../engine/thirdparty/rmlui/Source/Core/DataModel.cpp) | 378 |  |
| [`DecoratorTiled.cpp`](../../engine/thirdparty/rmlui/Source/Core/DecoratorTiled.cpp) | 373 |  |
| [`RenderManager.cpp`](../../engine/thirdparty/rmlui/Source/Core/RenderManager.cpp) | 370 | _symbols:_ ResourceCount |
| [`StyleSheetSelector.cpp`](../../engine/thirdparty/rmlui/Source/Core/StyleSheetSelector.cpp) | 367 |  |
| [`ElementProgress.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/ElementProgress.cpp) | 362 | ratio |
| [`ftccache.h`](../../engine/thirdparty/freetype/src/cache/ftccache.h) | 359 | ftccache.h FreeType internal cache interface (specification). |
| [`ftgxval.h`](../../engine/thirdparty/freetype/include/freetype/ftgxval.h) | 354 | ftgxval.h FreeType API for validating TrueTypeGX/AAT tables (specification). |
| [`PropertyParserAnimation.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserAnimation.cpp) | 352 | parameters |
| [`ftsystem.h`](../../engine/thirdparty/freetype/include/freetype/ftsystem.h) | 350 | ftsystem.h FreeType low-level system interface definition (specification). |
| [`fttrigon.h`](../../engine/thirdparty/freetype/include/freetype/fttrigon.h) | 350 | fttrigon.h FreeType trigonometric functions (specification). |
| [`ftincrem.h`](../../engine/thirdparty/freetype/include/freetype/ftincrem.h) | 348 | ftincrem.h FreeType incremental loading (specification). |
| [`ElementHandle.cpp`](../../engine/thirdparty/rmlui/Source/Core/ElementHandle.cpp) | 346 | _symbols:_ ElementHandleTargetData, HandleEdgeMarginParser |
| [`compiler-macros.h`](../../engine/thirdparty/freetype/include/freetype/internal/compiler-macros.h) | 343 | internal/compiler-macros.h Compiler-specific macro definitions used internally by FreeType. |
| [`otsvg.h`](../../engine/thirdparty/freetype/include/freetype/otsvg.h) | 336 | otsvg.h Interface for OT-SVG support related things (specification). |
| [`ScrollController.cpp`](../../engine/thirdparty/rmlui/Source/Core/ScrollController.cpp) | 332 |  |
| [`engine_session.cpp`](../../engine/native/src/engine_session.cpp) | 331 |  |
| [`pfrtypes.h`](../../engine/thirdparty/freetype/src/pfr/pfrtypes.h) | 331 | pfrtypes.h FreeType PFR data structures (specification only). |
| [`GeometryBackgroundBorder.cpp`](../../engine/thirdparty/rmlui/Source/Core/GeometryBackgroundBorder.cpp) | 330 |  |
| [`ftbitmap.h`](../../engine/thirdparty/freetype/include/freetype/ftbitmap.h) | 329 | ftbitmap.h FreeType utility functions for bitmaps (specification). |
| [`test_interop_reload_reset.cpp`](../../engine/native/tests/test_interop_reload_reset.cpp) | 328 | engine/native/tests/test_interop_reload_reset.cpp Regression coverage for two related Task 9/Task 12 findings: 1. |
| [`ElementEffects.cpp`](../../engine/thirdparty/rmlui/Source/Core/ElementEffects.cpp) | 327 |  |
| [`flat_set.hpp`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Containers/itlib/flat_set.hpp) | 325 | itlib-flat-set v1.06 std::set-like class with an underlying vector SPDX-License-Identifier: MIT MIT License: Copyright(c) 2021-2023 Borislav Stanimirov Permission is hereby granted, free of charge,... |
| [`ftlcdfil.h`](../../engine/thirdparty/freetype/include/freetype/ftlcdfil.h) | 323 | ftlcdfil.h FreeType API for color filtering of subpixel bitmap glyphs (specification). |
| [`ftcglyph.h`](../../engine/thirdparty/freetype/src/cache/ftcglyph.h) | 314 | ftcglyph.h FreeType abstract glyph cache (specification). |
| [`wofftypes.h`](../../engine/thirdparty/freetype/include/freetype/internal/wofftypes.h) | 312 | wofftypes.h Basic WOFF/WOFF2 type definitions and interface (specification only). |
| [`psobjs.h`](../../engine/thirdparty/freetype/src/psaux/psobjs.h) | 312 | psobjs.h Auxiliary functions for PostScript fonts (specification). |
| [`t1types.h`](../../engine/thirdparty/freetype/include/freetype/internal/t1types.h) | 307 | t1types.h Basic Type1/Type2 type definitions and interface (specification only). |
| [`ContainerBox.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/ContainerBox.cpp) | 298 |  |
| [`ElementImage.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/ElementImage.cpp) | 297 |  |
| [`fterrors.h`](../../engine/thirdparty/freetype/include/freetype/fterrors.h) | 296 | fterrors.h FreeType error code handling (specification). |
| [`ftlist.h`](../../engine/thirdparty/freetype/include/freetype/ftlist.h) | 296 | ftlist.h Generic list support for FreeType (specification). |
| [`ftmac.h`](../../engine/thirdparty/freetype/include/freetype/ftmac.h) | 289 |  |
| [`ftdrv.h`](../../engine/thirdparty/freetype/include/freetype/internal/ftdrv.h) | 289 | ftdrv.h FreeType internal font driver interface (specification). |
| [`pshints.h`](../../engine/thirdparty/freetype/src/psaux/pshints.h) | 288 | pshints.h Adobe's code for handling CFF hints (body). |
| [`DataVariable.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/DataVariable.h) | 288 | A 'DataVariable' wraps a user handle (pointer) and a VariableDefinition. |
| [`InlineContainer.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/InlineContainer.cpp) | 288 |  |
| [`WidgetTextInput.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/WidgetTextInput.h) | 287 | An abstract widget for editing and navigating around a text field. |
| [`fterrdef.h`](../../engine/thirdparty/freetype/include/freetype/fterrdef.h) | 283 |  |
| [`zutil.h`](../../engine/thirdparty/freetype/src/gzip/zutil.h) | 281 | zutil.h -- internal interface and configuration of the compression library Copyright (C) 1995-2022 Jean-loup Gailly, Mark Adler For conditions of distribution and use, see copyright notice in zlib.h |
| [`TypeConverter.cpp`](../../engine/thirdparty/rmlui/Source/Core/TypeConverter.cpp) | 280 | declaration |
| [`ftwinfnt.h`](../../engine/thirdparty/freetype/include/freetype/ftwinfnt.h) | 276 | ftwinfnt.h FreeType API for accessing Windows fnt-specific data. |
| [`ftsnames.h`](../../engine/thirdparty/freetype/include/freetype/ftsnames.h) | 272 | ftsnames.h Simple interface to access SFNT 'name' tables (which are used to hold font names, copyright info, notices, etc.) (specification). |
| [`DecoratorTiledBox.cpp`](../../engine/thirdparty/rmlui/Source/Core/DecoratorTiledBox.cpp) | 269 | name |
| [`Math.cpp`](../../engine/thirdparty/rmlui/Source/Core/Math.cpp) | 269 |  |
| [`ComputeProperty.cpp`](../../engine/thirdparty/rmlui/Source/Core/ComputeProperty.cpp) | 265 | _symbols:_ ComputedPropertyData |
| [`BlockFormattingContext.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/BlockFormattingContext.cpp) | 263 | element |
| [`PropertyParserTransform.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserTransform.cpp) | 263 | parameters |
| [`run_tests_json.py`](../../engine/run_tests_json.py) | 261 | Run CTest suite and produce structured JSON for the test dashboard. |
| [`EventDispatcher.cpp`](../../engine/thirdparty/rmlui/Source/Core/EventDispatcher.cpp) | 257 | CollectedListener When dispatching an event we collect all possible event listeners to execute. |
| [`bdf.h`](../../engine/thirdparty/freetype/src/bdf/bdf.h) | 253 |  |
| [`pcf.h`](../../engine/thirdparty/freetype/src/pcf/pcf.h) | 251 |  |
| [`integer-types.h`](../../engine/thirdparty/freetype/include/freetype/config/integer-types.h) | 250 | config/integer-types.h FreeType integer types definitions. |
| [`FontFaceLayer.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FontFaceLayer.cpp) | 247 |  |
| [`ftrfork.h`](../../engine/thirdparty/freetype/include/freetype/internal/ftrfork.h) | 245 | ftrfork.h Embedded resource forks accessor (specification). |
| [`ftrender.h`](../../engine/thirdparty/freetype/include/freetype/ftrender.h) | 244 | ftrender.h FreeType renderer modules public interface (specification). |
| [`Input.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Input.h) | 244 | Enumerants for sending input events into RmlUi. |
| [`GeometryBoxShadow.cpp`](../../engine/thirdparty/rmlui/Source/Core/GeometryBoxShadow.cpp) | 244 |  |
| [`backgrounds.cpp`](../../engine/native/src/host/backgrounds.cpp) | 243 | Environment backgrounds — see native/backgrounds.h. |
| [`scale_host.cpp`](../../engine/native/src/host/scale_host.cpp) | 243 | host/scale_host.cpp — the scale-generic session host. |
| [`ftcmru.h`](../../engine/thirdparty/freetype/src/cache/ftcmru.h) | 242 |  |
| [`ElementScroll.cpp`](../../engine/thirdparty/rmlui/Source/Core/ElementScroll.cpp) | 241 |  |
| [`app_context.cpp`](../../engine/native/src/app/app_context.cpp) | 239 | app/app_context.cpp — command-emission helpers (see app/app_context.h). |
| [`StyleSheet.cpp`](../../engine/thirdparty/rmlui/Source/Core/StyleSheet.cpp) | 239 |  |
| [`FontProvider.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FontProvider.cpp) | 238 |  |
| [`ID.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/ID.h) | 236 | The following values define the shorthand ids for the main stylesheet specification. |
| [`FloatedBoxSpace.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/FloatedBoxSpace.cpp) | 235 |  |
| [`autohint.h`](../../engine/thirdparty/freetype/include/freetype/internal/autohint.h) | 234 | autohint.h High-level 'autohint' module-specific interface (specification). |
| [`pshalgo.h`](../../engine/thirdparty/freetype/src/pshinter/pshalgo.h) | 233 | pshalgo.h PostScript hinting algorithm (specification). |
| [`TransformPrimitive.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/TransformPrimitive.h) | 233 | The TransformPrimitive struct is the base struct of geometric transforms such as rotations, scalings and translations. |
| [`DecoratorNinePatch.cpp`](../../engine/thirdparty/rmlui/Source/Core/DecoratorNinePatch.cpp) | 227 | In the following, we operate on the four diagonal vertices in the grid, as they define the whole grid. |
| [`ftparams.h`](../../engine/thirdparty/freetype/include/freetype/ftparams.h) | 218 | ftparams.h FreeType API for possible FT_Parameter tags (specification only). |
| [`gzguts.h`](../../engine/thirdparty/freetype/src/gzip/gzguts.h) | 218 | gzguts.h -- zlib internal header definitions for gz* operations Copyright (C) 2004-2019 Mark Adler For conditions of distribution and use, see copyright notice in zlib.h |
| [`engine_session.h`](../../engine/native/include/native/engine_session.h) | 216 | _symbols:_ RenderBridge, GpuEngine, NativeEngineOptions, NativeEngineSession |
| [`ui_command.h`](../../engine/native/include/native/ui_command.h) | 216 | _symbols:_ SetToggle, SetToggleProfile, SetDouble, SetEnum |
| [`Math.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Math.h) | 216 | _symbols:_ Colour, Vector2, Rectangle |
| [`StyleSheetContainer.cpp`](../../engine/thirdparty/rmlui/Source/Core/StyleSheetContainer.cpp) | 215 |  |
| [`svmm.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svmm.h) | 214 | svmm.h The FreeType Multiple Masters and GX var services (specification). |
| [`ftbdf.h`](../../engine/thirdparty/freetype/include/freetype/ftbdf.h) | 212 | ftbdf.h FreeType API for accessing BDF-specific strings (specification). |
| [`Tween.cpp`](../../engine/thirdparty/rmlui/Source/Core/Tween.cpp) | 212 | Tweening functions below. |
| [`ftotval.h`](../../engine/thirdparty/freetype/include/freetype/ftotval.h) | 206 | ftotval.h FreeType API for validating OpenType tables (specification). |
| [`ftmoderr.h`](../../engine/thirdparty/freetype/include/freetype/ftmoderr.h) | 204 | ftmoderr.h FreeType module error offsets (specification). |
| [`StringUtilities.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/StringUtilities.h) | 204 | Helper functions for string manipulation. |
| [`InlineLevelBox.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/InlineLevelBox.cpp) | 204 | first_box |
| [`DecoratorGradient.h`](../../engine/thirdparty/rmlui/Source/Core/DecoratorGradient.h) | 202 | Straight gradient. |
| [`StyleSheetFactory.cpp`](../../engine/thirdparty/rmlui/Source/Core/StyleSheetFactory.cpp) | 200 |  |
| [`DataModelHandle.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/DataModelHandle.h) | 197 | _symbols:_ DataModel, RMLUICORE_API, DataModelConstructorAccessor |
| [`pshglob.h`](../../engine/thirdparty/freetype/src/pshinter/pshglob.h) | 196 | pshglob.h PostScript hinter global hinting management. |
| [`StreamMemory.cpp`](../../engine/thirdparty/rmlui/Source/Core/StreamMemory.cpp) | 196 |  |
| [`XMLParser.cpp`](../../engine/thirdparty/rmlui/Source/Core/XMLParser.cpp) | 196 | _symbols:_ XmlParserData |
| [`boundary_shapes.cpp`](../../engine/native/src/host/boundary_shapes.cpp) | 195 | Boundary-shape wireframes — see native/boundary_shapes.h. |
| [`aflatin.h`](../../engine/thirdparty/freetype/src/autofit/aflatin.h) | 194 | aflatin.h Auto-fitter hinting routines for latin writing system (specification). |
| [`scale1_adapter.cpp`](../../engine/native/src/host/adapters/scale1_adapter.cpp) | 193 | host/adapters/scale1_adapter.cpp — Scale 1 (ParticleEngine) behind the seam. |
| [`test_d3d12_render_frame_fencing.cpp`](../../engine/native/tests/test_d3d12_render_frame_fencing.cpp) | 192 | engine/native/tests/test_d3d12_render_frame_fencing.cpp D3D12Presenter::render() is called from exactly one place in the whole repo -- main.cpp's message loop -- so before this test existed, no CTe... |
| [`Factory.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Factory.h) | 191 | The Factory contains a registry of instancers for different types. |
| [`d3d12_presenter.h`](../../engine/native/include/native/d3d12_presenter.h) | 189 | _symbols:_ Camera, NativeViewOptions, D3D12PresenterOptions, CaptureToken |
| [`scale0_adapter.h`](../../engine/native/include/native/host/adapters/scale0_adapter.h) | 189 | host/adapters/scale0_adapter.h — Scale 0 (voxel field, RenderBridge) behind the ScaleAdapter seam. |
| [`RenderInterfaceCompatibility.cpp`](../../engine/thirdparty/rmlui/Source/Core/RenderInterfaceCompatibility.cpp) | 189 | vertices |
| [`parameter_journal.cpp`](../../engine/native/src/parameter_journal.cpp) | 188 |  |
| [`spectrum.cpp`](../../engine/native/src/spectrum.cpp) | 188 | native/spectrum.cpp — E(k) of the flux field (see native/spectrum.h). |
| [`ftadvanc.h`](../../engine/thirdparty/freetype/include/freetype/ftadvanc.h) | 188 | ftadvanc.h Quick computation of advance widths (specification only). |
| [`Config.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Config/Config.h) | 187 | This file provides the means to configure various types used across RmlUi. |
| [`ftstdlib.h`](../../engine/thirdparty/freetype/include/freetype/config/ftstdlib.h) | 185 | ftstdlib.h ANSI-specific library and header configuration file (specification only). |
| [`psblues.h`](../../engine/thirdparty/freetype/src/psaux/psblues.h) | 185 | psblues.h Adobe's code for handling Blue Zones (specification). |
| [`ElementTabSet.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/ElementTabSet.cpp) | 185 |  |
| [`LineBox.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/LineBox.h) | 185 | Horizontally places fragments generated from inline-level boxes. |
| [`ftlogging.h`](../../engine/thirdparty/freetype/include/freetype/ftlogging.h) | 184 |  |
| [`scenario_catalog.h`](../../engine/native/include/native/scenario_catalog.h) | 183 | native/scenario_catalog.h — engine/native's OWN Scale-0 scenario catalog. |
| [`DataStructHandle.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/DataStructHandle.h) | 183 | ... |
| [`TextureDatabase.cpp`](../../engine/thirdparty/rmlui/Source/Core/TextureDatabase.cpp) | 180 |  |
| [`ftpfr.h`](../../engine/thirdparty/freetype/include/freetype/ftpfr.h) | 179 | ftpfr.h FreeType API for accessing PFR-specific data (specification only). |
| [`rml_d3d12_renderer.h`](../../engine/native/src/ui/rml_d3d12_renderer.h) | 177 | RmlD3D12Renderer — RmlUi 6.2 Rml::RenderInterface implemented on the engine's own Direct3D 12 device (M-UI-1, see native/docs/SPEC_NATIVE_UI_RMLUI.md §2). |
| [`parse_modules_cfg.py`](../../engine/thirdparty/freetype/builds/meson/parse_modules_cfg.py) | 177 | Copyright (C) 2020-2024 by David Turner, Robert Wilhelm, and Werner Lemberg. |
| [`ftcmanag.h`](../../engine/thirdparty/freetype/src/cache/ftcmanag.h) | 175 |  |
| [`ftzopen.h`](../../engine/thirdparty/freetype/src/lzw/ftzopen.h) | 174 | ftzopen.h FreeType support for .Z compressed files. |
| [`ElementFormControlTextArea.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/ElementFormControlTextArea.cpp) | 174 | ratio |
| [`ftd_chart_element.cpp`](../../engine/native/src/ui/ftd_chart_element.cpp) | 173 | parent |
| [`fttrace.h`](../../engine/thirdparty/freetype/include/freetype/internal/fttrace.h) | 173 |  |
| [`afglobal.h`](../../engine/thirdparty/freetype/src/autofit/afglobal.h) | 173 | afglobal.h Auto-fitter routines to compute global hinting values (specification). |
| [`gxvfeat.h`](../../engine/thirdparty/freetype/src/gxvalid/gxvfeat.h) | 173 | gxvfeat.h TrueTypeGX/AAT feat table validation (specification). |
| [`pshrec.h`](../../engine/thirdparty/freetype/src/pshinter/pshrec.h) | 171 | pshrec.h Postscript (Type1/Type2) hints recorder (specification). |
| [`ElementDocument.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/ElementDocument.h) | 170 | ModalFlag controls the modal state of the document. |
| [`ElementFormControlInput.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/ElementFormControlInput.cpp) | 170 |  |
| [`ftchapters.h`](../../engine/thirdparty/freetype/include/freetype/ftchapters.h) | 168 | This file defines the structure of the FreeType reference. |
| [`PropertyDefinition.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyDefinition.cpp) | 168 |  |
| [`app_context.h`](../../engine/native/src/app/app_context.h) | 167 | app/app_context.h — AppContext (the shared GUI-thread state every wnd_proc / RmlUi callback reaches) plus the command-emission helpers that translate UI actions into ScaleCommands on the bus. |
| [`ftcid.h`](../../engine/thirdparty/freetype/include/freetype/ftcid.h) | 167 | ftcid.h FreeType API for accessing CID font information (specification). |
| [`psft.h`](../../engine/thirdparty/freetype/src/psaux/psft.h) | 167 | psft.h FreeType Glue Component to Adobe's Interpreter (specification). |
| [`ObserverPtr.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/ObserverPtr.h) | 167 | Observer pointer. |
| [`Types.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Types.h) | 167 | _symbols:_ Element, ElementInstancer, ElementAnimation, RenderManager |
| [`test_interop_visual_parity.cpp`](../../engine/native/tests/test_interop_visual_parity.cpp) | 166 | engine/native/tests/test_interop_visual_parity.cpp Confirms the interop gather kernel (Task 6) produces exactly the same world positions and colors the pre-interop CPU path (NativeEngineSession::ca... |
| [`DataVariable.cpp`](../../engine/thirdparty/rmlui/Source/Core/DataVariable.cpp) | 166 | ptr |
| [`EventSpecification.cpp`](../../engine/thirdparty/rmlui/Source/Core/EventSpecification.cpp) | 166 | _symbols:_ EventSpecificationData |
| [`ElementFormControlSelect.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/ElementFormControlSelect.cpp) | 165 | ratio |
| [`BlockContainer.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/BlockContainer.h) | 165 | A container for block-level boxes. |
| [`XMLParseTools.cpp`](../../engine/thirdparty/rmlui/Source/Core/XMLParseTools.cpp) | 165 |  |
| [`winfnt.h`](../../engine/thirdparty/freetype/src/winfonts/winfnt.h) | 164 | winfnt.h FreeType font driver for Windows FNT/FON files Copyright (C) 1996-2024 by David Turner, Robert Wilhelm, and Werner Lemberg. |
| [`DataViewDefault.h`](../../engine/thirdparty/rmlui/Source/Core/DataViewDefault.h) | 162 | _symbols:_ Element, DataExpression, DataViewCommon, DataViewAttribute |
| [`ftd_chart_element.h`](../../engine/native/src/ui/ftd_chart_element.h) | 161 | FtdChartElement — the FTD instrument widget: a custom RmlUi element (<ftd-chart>) that plots one or more scalar time-series through the engine's own D3D12 UI pipeline (native/docs/SPEC_NATIVE_UI_RM... |
| [`ftvalid.h`](../../engine/thirdparty/freetype/include/freetype/internal/ftvalid.h) | 160 | ftvalid.h FreeType validation support (specification). |
| [`t1objs.h`](../../engine/thirdparty/freetype/src/type1/t1objs.h) | 160 |  |
| [`Traits.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Traits.h) | 160 | var |
| [`ftsizes.h`](../../engine/thirdparty/freetype/include/freetype/ftsizes.h) | 159 | ftsizes.h FreeType size objects management (specification). |
| [`Core.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Core.h) | 159 | RmlUi library core API. |
| [`ftmodule.h`](../../engine/thirdparty/freetype/builds/amiga/include/config/ftmodule.h) | 158 | / /* |
| [`Event.cpp`](../../engine/thirdparty/rmlui/Source/Core/Event.cpp) | 157 |  |
| [`cidobjs.h`](../../engine/thirdparty/freetype/src/cid/cidobjs.h) | 154 |  |
| [`FontEffectGlow.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEffectGlow.cpp) | 154 | glyph |
| [`TypeConverter.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/TypeConverter.h) | 152 | Templatised TypeConverters with Template Specialisation. |
| [`ftgzip.h`](../../engine/thirdparty/freetype/include/freetype/ftgzip.h) | 151 |  |
| [`test_interop_reload_orchestration.cpp`](../../engine/native/tests/test_interop_reload_orchestration.cpp) | 150 | engine/native/tests/test_interop_reload_orchestration.cpp Fast, hardware-independent regression coverage for the InteropReloadOutcome transition/logging contract of reimport_interop_after_reload()... |
| [`cfftoken.h`](../../engine/thirdparty/freetype/src/cff/cfftoken.h) | 150 | cfftoken.h CFF token definitions (specification only). |
| [`TableFormattingDetails.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/TableFormattingDetails.h) | 149 | TableGrid builds the structure of the table, that is a list of rows, columns, and cells, taking spanning attributes into account to position cells. |
| [`Variant.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Variant.h) | 148 | Variant is a container that can store a selection of basic types. |
| [`TransformPrimitive.cpp`](../../engine/thirdparty/rmlui/Source/Core/TransformPrimitive.cpp) | 148 |  |
| [`ftgloadr.h`](../../engine/thirdparty/freetype/include/freetype/internal/ftgloadr.h) | 147 | ftgloadr.h The FreeType glyph loader (specification). |
| [`DecoratorTiledVertical.cpp`](../../engine/thirdparty/rmlui/Source/Core/DecoratorTiledVertical.cpp) | 147 | name |
| [`PropertyParserDecorator.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserDecorator.cpp) | 146 | parameters |
| [`svpscmap.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svpscmap.h) | 145 | svpscmap.h The FreeType PostScript charmap service (specification). |
| [`StyleTypes.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/StyleTypes.h) | 145 | _symbols:_ LengthPercentageAuto, LengthPercentage, NumberAuto, LineHeight |
| [`DecoratorTiledHorizontal.cpp`](../../engine/thirdparty/rmlui/Source/Core/DecoratorTiledHorizontal.cpp) | 145 | name |
| [`cffparse.h`](../../engine/thirdparty/freetype/src/cff/cffparse.h) | 144 |  |
| [`psglue.h`](../../engine/thirdparty/freetype/src/psaux/psglue.h) | 144 | psglue.h Adobe's code for shared stuff (specification only). |
| [`ftgasp.h`](../../engine/thirdparty/freetype/include/freetype/ftgasp.h) | 143 | ftgasp.h Access of TrueType's 'gasp' table (specification). |
| [`t1tokens.h`](../../engine/thirdparty/freetype/src/type1/t1tokens.h) | 143 |  |
| [`RenderInterface.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/RenderInterface.h) | 143 | The abstract base class for application-specific rendering implementation. |
| [`BoxShadowHash.h`](../../engine/thirdparty/rmlui/Source/Core/BoxShadowHash.h) | 143 |  |
| [`ftd_slice_element.cpp`](../../engine/native/src/ui/ftd_slice_element.cpp) | 142 | parent |
| [`ElementStyle.h`](../../engine/thirdparty/rmlui/Source/Core/ElementStyle.h) | 142 | Manages an element's style and property information. |
| [`afcjk.h`](../../engine/thirdparty/freetype/src/autofit/afcjk.h) | 141 | afcjk.h Auto-fitter hinting routines for CJK writing system (specification). |
| [`DecoratorText.cpp`](../../engine/thirdparty/rmlui/Source/Core/DecoratorText.cpp) | 141 | name |
| [`XMLNodeHandlerHead.cpp`](../../engine/thirdparty/rmlui/Source/Core/XMLNodeHandlerHead.cpp) | 141 | type |
| [`ftsdfcommon.h`](../../engine/thirdparty/freetype/src/sdf/ftsdfcommon.h) | 140 | ftsdfcommon.h Auxiliary data for Signed Distance Field support (specification). |
| [`ElementBackgroundBorder.cpp`](../../engine/thirdparty/rmlui/Source/Core/ElementBackgroundBorder.cpp) | 140 |  |
| [`MeshUtilities.cpp`](../../engine/thirdparty/rmlui/Source/Core/MeshUtilities.cpp) | 140 |  |
| [`test_cuda_import_shared_buffer.cpp`](../../engine/native/tests/test_cuda_import_shared_buffer.cpp) | 139 | Off-screen: a message-only window is enough to build a swapchain-free D3D12 device via initialize()'s HWND-taking path -- same rationale as test_d3d12_shared_buffer.cpp. |
| [`ContainerBox.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/ContainerBox.h) | 139 | Abstraction for layout boxes that can act as a containing block. |
| [`public-macros.h`](../../engine/thirdparty/freetype/include/freetype/config/public-macros.h) | 138 | config/public-macros.h Define a set of compiler macros used in public FreeType headers. |
| [`DataView.cpp`](../../engine/thirdparty/rmlui/Source/Core/DataView.cpp) | 138 |  |
| [`app_input.cpp`](../../engine/native/src/app/app_input.cpp) | 137 | app/app_input.cpp — Win32 window procedure + input helpers (see app/app_input.h). |
| [`t1parse.h`](../../engine/thirdparty/freetype/src/type1/t1parse.h) | 137 |  |
| [`Vector2.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Vector2.h) | 136 | Templated class for a generic two-component vector. |
| [`fthash.h`](../../engine/thirdparty/freetype/include/freetype/internal/fthash.h) | 135 | fthash.h Hashing functions (specification). |
| [`psfont.h`](../../engine/thirdparty/freetype/src/psaux/psfont.h) | 134 | psfont.h Adobe's code for font instances (specification). |
| [`PropertyIdSet.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/PropertyIdSet.h) | 133 | PropertyIdSet is a 'set'-like container for PropertyIds. |
| [`app_pick.cpp`](../../engine/native/src/app/app_pick.cpp) | 131 | app/app_pick.cpp — camera framing + ray picking (see app/app_pick.h). |
| [`svmetric.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svmetric.h) | 131 | svmetric.h The FreeType services for metrics variations (specification). |
| [`inflate.h`](../../engine/thirdparty/freetype/src/gzip/inflate.h) | 131 |  |
| [`test_interop_fence_roundtrip.cpp`](../../engine/native/tests/test_interop_fence_roundtrip.cpp) | 130 | engine/native/tests/test_interop_fence_roundtrip.cpp |
| [`cidparse.h`](../../engine/thirdparty/freetype/src/cid/cidparse.h) | 130 |  |
| [`InlineBox.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/InlineBox.cpp) | 130 | mode |
| [`StyleSheetSelector.h`](../../engine/thirdparty/rmlui/Source/Core/StyleSheetSelector.h) | 129 | Constants used to determine the specificity of a selector. |
| [`RenderManager.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/RenderManager.h) | 128 | A wrapper over the render interface, which tracks its state and resources. |
| [`Vector4.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Vector4.h) | 127 | Templated class for a generic four-component vector. |
| [`FontFaceHandleDefault.h`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FontFaceHandleDefault.h) | 127 | _symbols:_ FontFaceLayer, FontFaceHandleDefault, EffectLayerPair |
| [`ttcmap.h`](../../engine/thirdparty/freetype/src/sfnt/ttcmap.h) | 126 | ttcmap.h TrueType character mapping table (cmap) support (specification). |
| [`t1load.h`](../../engine/thirdparty/freetype/src/type1/t1load.h) | 126 |  |
| [`DecoratorTiled.h`](../../engine/thirdparty/rmlui/Source/Core/DecoratorTiled.h) | 126 | Base class for tiled decorators. |
| [`WidgetSlider.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/WidgetSlider.h) | 126 | A generic widget for incorporating sliding functionality into an element. |
| [`LayoutDetails.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/LayoutDetails.h) | 126 | ComputedAxisSize is an abstraction of an element's computed size properties along a single axis, either horizontally or vertically, allowing eg. |
| [`tttags.h`](../../engine/thirdparty/freetype/include/freetype/tttags.h) | 124 | tttags.h Tags for TrueType and OpenType tables (specification only). |
| [`cffload.h`](../../engine/thirdparty/freetype/src/cff/cffload.h) | 124 | cffload.h OpenType & CFF data/program tables loader (specification). |
| [`pfrload.h`](../../engine/thirdparty/freetype/src/pfr/pfrload.h) | 123 |  |
| [`t42objs.h`](../../engine/thirdparty/freetype/src/type42/t42objs.h) | 123 |  |
| [`Colour.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Colour.h) | 123 | Templated class for a four-component RGBA colour. |
| [`TextureLayoutTexture.cpp`](../../engine/thirdparty/rmlui/Source/Core/TextureLayoutTexture.cpp) | 123 |  |
| [`psstack.h`](../../engine/thirdparty/freetype/src/psaux/psstack.h) | 122 | psstack.h Adobe's code for emulating a CFF stack (specification). |
| [`Vector3.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Vector3.h) | 122 | Templated class for a generic three-component vector. |
| [`draw_list.h`](../../engine/native/include/native/model/draw_list.h) | 121 | model/draw_list.h — the scale-generic render vocabulary. |
| [`Event.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Event.h) | 121 | An event that propagates through the element hierarchy. |
| [`pserror.h`](../../engine/thirdparty/freetype/src/psaux/pserror.h) | 120 | pserror.h Adobe's code for error handling (specification). |
| [`native_frame.h`](../../engine/native/include/native/native_frame.h) | 119 | _symbols:_ NativeParticle, NativeLine, NativeSheetVertex, NativeGlyph |
| [`DataTypeRegister.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/DataTypeRegister.h) | 119 | _symbols:_ RMLUICORE_API |
| [`PropertyParserFontEffect.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserFontEffect.cpp) | 119 | parameters |
| [`Stream.cpp`](../../engine/thirdparty/rmlui/Source/Core/Stream.cpp) | 119 | buffer |
| [`ftmisc.h`](../../engine/thirdparty/freetype/src/raster/ftmisc.h) | 118 | ftmisc.h Miscellaneous macros for stand-alone rasterizer (specification only). |
| [`ftsdfrend.h`](../../engine/thirdparty/freetype/src/sdf/ftsdfrend.h) | 118 | ftsdfrend.h Signed Distance Field renderer interface (specification). |
| [`DataControllerDefault.cpp`](../../engine/thirdparty/rmlui/Source/Core/DataControllerDefault.cpp) | 118 | modifier |
| [`extract_freetype_version.py`](../../engine/thirdparty/freetype/builds/meson/extract_freetype_version.py) | 117 | Copyright (C) 2020-2024 by David Turner, Robert Wilhelm, and Werner Lemberg. |
| [`Template.cpp`](../../engine/thirdparty/rmlui/Source/Core/Template.cpp) | 117 |  |
| [`ui_snapshot.h`](../../engine/native/include/native/ui_snapshot.h) | 116 | _symbols:_ EnvInfo, UiForceDiag, NeighborCell, ContinuitySnapshot |
| [`app_options.cpp`](../../engine/native/src/app/app_options.cpp) | 116 | app/app_options.cpp — native_app CLI parsing (see app/app_options.h). |
| [`ftd_slice_element.h`](../../engine/native/src/ui/ftd_slice_element.h) | 116 | FtdSliceElement — a custom RmlUi element (<ftd-slice>) that draws a 2D field slice as a colour-mapped heatmap, through the engine's own D3D12 UI pipeline. |
| [`extract_libtool_version.py`](../../engine/thirdparty/freetype/builds/meson/extract_libtool_version.py) | 115 | Copyright (C) 2020-2024 by David Turner, Robert Wilhelm, and Werner Lemberg. |
| [`process_ftoption_h.py`](../../engine/thirdparty/freetype/builds/meson/process_ftoption_h.py) | 115 | Copyright (C) 2020-2024 by David Turner, Robert Wilhelm, and Werner Lemberg. |
| [`cidtoken.h`](../../engine/thirdparty/freetype/src/cid/cidtoken.h) | 115 | cidtoken.h CID token definitions (specification only). |
| [`PropertySpecification.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/PropertySpecification.h) | 115 | A property specification stores a group of property definitions. |
| [`InlineLevelBox.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/InlineLevelBox.h) | 115 | A box that takes part in inline layout. |
| [`PluginRegistry.cpp`](../../engine/thirdparty/rmlui/Source/Core/PluginRegistry.cpp) | 115 | _symbols:_ PluginVectors |
| [`scale_adapter.h`](../../engine/native/include/native/host/scale_adapter.h) | 114 | host/scale_adapter.h — the uniform contract every scale implements used by every retained native scale. |
| [`test_interop_gather.cpp`](../../engine/native/tests/test_interop_gather.cpp) | 113 |  |
| [`FontEffectBlur.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEffectBlur.cpp) | 113 | glyph |
| [`Pool.h`](../../engine/thirdparty/rmlui/Source/Core/Pool.h) | 113 | Iterator objects are used for safe traversal of the allocated members of a pool. |
| [`ElementUtilities.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/ElementUtilities.h) | 112 | Utility functions for dealing with elements. |
| [`URL.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/URL.h) | 112 | _symbols:_ RMLUICORE_API |
| [`InputTypeText.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputTypeText.cpp) | 112 | event |
| [`ttload.h`](../../engine/thirdparty/freetype/src/sfnt/ttload.h) | 111 | ttload.h Load the basic TrueType tables, i.e., tables that can be either in TTF or OTF fonts (specification). |
| [`ElementText.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/ElementText.h) | 110 | _symbols:_ RMLUICORE_API, Line, TexturedGeometry |
| [`ElementFormControlTextArea.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Elements/ElementFormControlTextArea.h) | 110 | Default RmlUi implementation of a text area. |
| [`GeometryBackgroundBorder.h`](../../engine/thirdparty/rmlui/Source/Core/GeometryBackgroundBorder.h) | 109 | _symbols:_ Box, BorderMetrics, GeometryBackgroundBorder |
| [`BaseXMLParser.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/BaseXMLParser.h) | 108 | _symbols:_ Stream, URL, RMLUICORE_API |
| [`cffotypes.h`](../../engine/thirdparty/freetype/include/freetype/internal/cffotypes.h) | 107 | cffotypes.h Basic OpenType/CFF object type definitions (specification). |
| [`gxvalid.h`](../../engine/thirdparty/freetype/src/gxvalid/gxvalid.h) | 107 | gxvalid.h TrueTypeGX/AAT table validation (specification only). |
| [`Box.cpp`](../../engine/thirdparty/rmlui/Source/Core/Box.cpp) | 107 |  |
| [`Memory.h`](../../engine/thirdparty/rmlui/Source/Core/Memory.h) | 107 | Basic stack allocator. |
| [`app_options.h`](../../engine/native/src/app/app_options.h) | 106 | app/app_options.h — native_app command-line options (parsed once at startup). |
| [`ftcimage.h`](../../engine/thirdparty/freetype/src/cache/ftcimage.h) | 106 | ftcimage.h FreeType Generic Image cache (specification) Copyright (C) 2000-2024 by David Turner, Robert Wilhelm, and Werner Lemberg. |
| [`WidgetDropDown.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/WidgetDropDown.h) | 106 | Widget for drop-down functionality. |
| [`afcover.h`](../../engine/thirdparty/freetype/src/autofit/afcover.h) | 105 | afcover.h Auto-fitter coverages (specification only). |
| [`InlineContainer.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/InlineContainer.h) | 105 | A container for inline-level boxes. |
| [`ftsynth.h`](../../engine/thirdparty/freetype/include/freetype/ftsynth.h) | 104 | ftsynth.h FreeType synthesizing code for emboldening and slanting (specification). |
| [`t1cmap.h`](../../engine/thirdparty/freetype/src/psaux/t1cmap.h) | 104 | t1cmap.h Type 1 character map support (specification). |
| [`Decorator.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Decorator.h) | 104 | The abstract base class for any visual object that can be attached to any element. |
| [`FontEngineInterface.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/FontEngineInterface.h) | 104 | The abstract base class for an application-specific font engine implementation. |
| [`scale_host.h`](../../engine/native/include/native/host/scale_host.h) | 103 | host/scale_host.h — the scale-generic session host. |
| [`ElementFormControlSelect.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Elements/ElementFormControlSelect.h) | 103 | A drop-down select form control. |
| [`StyleSheetSpecification.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/StyleSheetSpecification.h) | 103 | _symbols:_ PropertyParser, DefaultStyleSheetParsers, RMLUICORE_API |
| [`DataTypeRegister.cpp`](../../engine/thirdparty/rmlui/Source/Core/DataTypeRegister.cpp) | 103 |  |
| [`InputTypeRadio.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputTypeRadio.cpp) | 103 |  |
| [`XMLNodeHandlerTabSet.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/XMLNodeHandlerTabSet.cpp) | 103 | parser |
| [`ftbzip2.h`](../../engine/thirdparty/freetype/include/freetype/ftbzip2.h) | 102 |  |
| [`StyleSheetParser.h`](../../engine/thirdparty/rmlui/Source/Core/StyleSheetParser.h) | 102 | Helper class for parsing a style sheet into its memory representation. |
| [`ftbbox.h`](../../engine/thirdparty/freetype/include/freetype/ftbbox.h) | 101 | ftbbox.h FreeType exact bbox computation (specification). |
| [`ElementImage.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/ElementImage.h) | 101 | The 'img' element can render images and sprites. |
| [`ftlzw.h`](../../engine/thirdparty/freetype/include/freetype/ftlzw.h) | 100 |  |
| [`Stream.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Stream.h) | 100 | Abstract class for a media-independent byte stream. |
| [`DataView.h`](../../engine/thirdparty/rmlui/Source/Core/DataView.h) | 100 | Data view. |
| [`StreamFile.cpp`](../../engine/thirdparty/rmlui/Source/Core/StreamFile.cpp) | 100 | buffer |
| [`app_win32.cpp`](../../engine/native/src/app/app_win32.cpp) | 99 | app/app_win32.cpp — Win32/WIC helpers (see app/app_win32.h). |
| [`gxvmort.h`](../../engine/thirdparty/freetype/src/gxvalid/gxvmort.h) | 99 | gxvmort.h TrueTypeGX/AAT common definition for mort table (specification). |
| [`psarrst.h`](../../engine/thirdparty/freetype/src/psaux/psarrst.h) | 99 | psarrst.h Adobe's code for Array Stacks (specification). |
| [`PropertyParserNumber.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserNumber.cpp) | 99 | parameters |
| [`WidgetScroll.h`](../../engine/thirdparty/rmlui/Source/Core/WidgetScroll.h) | 99 | A widget for incorporating scrolling functionality into an element. |
| [`Debug.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Debug.h) | 98 | Define for breakpointing. |
| [`ftsdf.h`](../../engine/thirdparty/freetype/src/sdf/ftsdf.h) | 97 | ftsdf.h Signed Distance Field support (specification). |
| [`StyleSheet.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/StyleSheet.h) | 97 | StyleSheet maintains a single stylesheet definition. |
| [`BoxShadowCache.cpp`](../../engine/thirdparty/rmlui/Source/Core/BoxShadowCache.cpp) | 97 | _symbols:_ BoxShadowCacheData |
| [`FontFaceLayer.h`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FontFaceLayer.h) | 97 | A textured layer stored as part of a font face handle. |
| [`FloatedBoxSpace.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/FloatedBoxSpace.h) | 97 | Each block box has a space object for managing the space occupied by its floating elements, and those of its ancestors as relevant. |
| [`pfrobjs.h`](../../engine/thirdparty/freetype/src/pfr/pfrobjs.h) | 96 | pfrobjs.h FreeType PFR object methods (specification). |
| [`CallbackTexture.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/CallbackTexture.h) | 96 | Callback function for generating textures on demand. |
| [`StyleSheetTypes.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/StyleSheetTypes.h) | 96 | StyleSheetIndex contains a cached index of all styled nodes for quick lookup when finding applicable style nodes for the current state of a given element. |
| [`CallbackTexture.cpp`](../../engine/thirdparty/rmlui/Source/Core/CallbackTexture.cpp) | 96 |  |
| [`ElementLabel.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/ElementLabel.cpp) | 96 |  |
| [`FontEffectOutline.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEffectOutline.cpp) | 96 | glyph |
| [`TemplateCache.cpp`](../../engine/thirdparty/rmlui/Source/Core/TemplateCache.cpp) | 96 |  |
| [`StableVector.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/StableVector.h) | 95 | A vector-like container that returns stable indices to refer to entries. |
| [`inffixed.h`](../../engine/thirdparty/freetype/src/gzip/inffixed.h) | 94 | inffixed.h -- table for decoding fixed codes Generated automatically by makefixed(). |
| [`psfixed.h`](../../engine/thirdparty/freetype/src/psaux/psfixed.h) | 94 | psfixed.h Adobe's code for Fixed-Point Mathematics (specification only). |
| [`ElementInstancer.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/ElementInstancer.h) | 94 | An element instancer provides a method for allocating and deallocating elements. |
| [`ftfntfmt.h`](../../engine/thirdparty/freetype/include/freetype/ftfntfmt.h) | 93 |  |
| [`ElementFormControlInput.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Elements/ElementFormControlInput.h) | 92 | A form control for the generic input element. |
| [`ElementProgress.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Elements/ElementProgress.h) | 92 | The 'progress' element. |
| [`ftmmtypes.h`](../../engine/thirdparty/freetype/include/freetype/internal/ftmmtypes.h) | 91 | ftmmtypes.h OpenType Variations type definitions for internal use with the multi-masters service (specification). |
| [`afloader.h`](../../engine/thirdparty/freetype/src/autofit/afloader.h) | 91 | afloader.h Auto-fitter glyph loading routines (specification). |
| [`ftcsbits.h`](../../engine/thirdparty/freetype/src/cache/ftcsbits.h) | 91 |  |
| [`t42parse.h`](../../engine/thirdparty/freetype/src/type42/t42parse.h) | 91 |  |
| [`RenderInterfaceCompatibility.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/RenderInterfaceCompatibility.h) | 91 | Provides a backward-compatible adapter for render interfaces written for RmlUi 5 and lower. |
| [`DecoratorShader.cpp`](../../engine/thirdparty/rmlui/Source/Core/DecoratorShader.cpp) | 91 | name |
| [`PropertyParserFilter.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserFilter.cpp) | 91 | parameters |
| [`svcfftl.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svcfftl.h) | 90 | svcfftl.h The FreeType CFF tables loader service (specification). |
| [`svttcmap.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svttcmap.h) | 90 | svttcmap.h The FreeType TrueType/sfnt cmap extra information service. |
| [`ConvolutionFilter.cpp`](../../engine/thirdparty/rmlui/Source/Core/ConvolutionFilter.cpp) | 90 |  |
| [`PropertyParserBoxShadow.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserBoxShadow.cpp) | 90 | parameters |
| [`SystemInterface.cpp`](../../engine/thirdparty/rmlui/Source/Core/SystemInterface.cpp) | 90 | cursor_name |
| [`generate_reference_docs.py`](../../engine/thirdparty/freetype/builds/meson/generate_reference_docs.py) | 89 | Copyright (C) 2020-2024 by David Turner, Robert Wilhelm, and Werner Lemberg. |
| [`XMLParser.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/XMLParser.h) | 89 | RmlUi's XML parsing engine. |
| [`streamlines.h`](../../engine/native/include/native/host/adapters/streamlines.h) | 88 | host/adapters/streamlines.h — CPU RK4 field-line integrator for the Scale-0 STREAMLINE overlays (Flux Lines · Radiative E · B Field), ported from the web dashboard's engine/web/js/fieldlines.js `co... |
| [`svsfnt.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svsfnt.h) | 88 | svsfnt.h The FreeType SFNT table loading service (specification). |
| [`afmparse.h`](../../engine/thirdparty/freetype/src/psaux/afmparse.h) | 88 |  |
| [`test_d3d12_overlay_once.cpp`](../../engine/native/tests/test_d3d12_overlay_once.cpp) | 87 | Counts OverlayRecorder::record() invocations: the composition contract requires exactly one overlay record per D3D12Presenter::render(), with a NULL DSV rebind already performed by the presenter (t... |
| [`svpsinfo.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svpsinfo.h) | 86 | svpsinfo.h The FreeType PostScript info service (specification). |
| [`ftccback.h`](../../engine/thirdparty/freetype/src/cache/ftccback.h) | 85 | ftccback.h Callback functions of the caching sub-system (specification only). |
| [`Rectangle.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Rectangle.h) | 85 | Templated class for a generic axis-aligned rectangle. |
| [`Unit.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Unit.h) | 85 |  |
| [`IdNameMap.h`](../../engine/thirdparty/rmlui/Source/Core/IdNameMap.h) | 85 | _symbols:_ IdNameMap, PropertyIdNameMap, ShorthandIdNameMap |
| [`cffobjs.h`](../../engine/thirdparty/freetype/src/cff/cffobjs.h) | 84 | cffobjs.h OpenType objects manager (specification). |
| [`scale1_adapter.h`](../../engine/native/include/native/host/adapters/scale1_adapter.h) | 83 | host/adapters/scale1_adapter.h — Scale 1 (ParticleEngine) behind the ScaleAdapter seam (validation that the seam is scale-generic — a SECOND adapter with a structurally different engine). |
| [`commands.h`](../../engine/native/include/native/model/commands.h) | 83 | model/commands.h — the scale-generic command vocabulary. |
| [`test_engine_session.cpp`](../../engine/native/tests/test_engine_session.cpp) | 83 |  |
| [`psintrp.h`](../../engine/thirdparty/freetype/src/psaux/psintrp.h) | 83 |  |
| [`ttcolr.h`](../../engine/thirdparty/freetype/src/sfnt/ttcolr.h) | 83 | ttcolr.h TrueType and OpenType colored glyph layer support (specification). |
| [`Box.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Box.h) | 83 | Stores a box with four sized areas; content, padding, a border and margin. |
| [`StreamMemory.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/StreamMemory.h) | 83 | Memory Byte Stream Class |
| [`DataController.h`](../../engine/thirdparty/rmlui/Source/Core/DataController.h) | 83 | Data controller. |
| [`ftbase.h`](../../engine/thirdparty/freetype/src/base/ftbase.h) | 82 | ftbase.h Private functions used in the `base' module (specification). |
| [`Core.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core.h) | 82 |  |
| [`DataTypes.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/DataTypes.h) | 82 | _symbols:_ VariableDefinition, DataTypeRegister, TransformFuncRegister, DataModelHandle |
| [`ScrollController.h`](../../engine/thirdparty/rmlui/Source/Core/ScrollController.h) | 81 | Implements scrolling behavior that occurs over time. |
| [`snapshot.h`](../../engine/native/include/native/model/snapshot.h) | 80 | model/snapshot.h — the scale-generic published snapshot. |
| [`test_d3d12_capture_lifecycle.cpp`](../../engine/native/tests/test_d3d12_capture_lifecycle.cpp) | 79 | Capture seam: request_capture returns a token; the next render records the readback copy; poll_capture stays Pending until the submission fence retires, then Ready with pitched bytes. |
| [`test_ui_harness_commands.cpp`](../../engine/native/tests/test_ui_harness_commands.cpp) | 79 |  |
| [`EventDispatcher.h`](../../engine/thirdparty/rmlui/Source/Core/EventDispatcher.h) | 79 | The Event Dispatcher manages a list of event listeners and triggers the events via EventHandlers whenever requested. |
| [`sfwoff2.h`](../../engine/thirdparty/freetype/src/sfnt/sfwoff2.h) | 78 | sfwoff2.h WOFFF2 format management (specification). |
| [`PropertiesIterator.h`](../../engine/thirdparty/rmlui/Source/Core/PropertiesIterator.h) | 78 | _symbols:_ PropertiesIterator |
| [`Spritesheet.cpp`](../../engine/thirdparty/rmlui/Source/Core/Spritesheet.cpp) | 78 |  |
| [`test_ui_snapshot_publisher.cpp`](../../engine/native/tests/test_ui_snapshot_publisher.cpp) | 77 |  |
| [`otvalid.h`](../../engine/thirdparty/freetype/src/otvalid/otvalid.h) | 77 | otvalid.h OpenType table validation (specification only). |
| [`pstypes.h`](../../engine/thirdparty/freetype/src/psaux/pstypes.h) | 77 | pstypes.h Adobe's code for defining data types (specification only). |
| [`Decorator.cpp`](../../engine/thirdparty/rmlui/Source/Core/Decorator.cpp) | 77 |  |
| [`ElementScroll.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/ElementScroll.h) | 76 | Manages an element's scrollbars and scrolling state. |
| [`DecoratorTiledImage.cpp`](../../engine/thirdparty/rmlui/Source/Core/DecoratorTiledImage.cpp) | 76 | name |
| [`LayoutPools.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/LayoutPools.cpp) | 76 | _symbols:_ LayoutChunk, LayoutPoolsData |
| [`StyleSheetNode.h`](../../engine/thirdparty/rmlui/Source/Core/StyleSheetNode.h) | 76 | A style sheet is composed of a tree of nodes. |
| [`DataModel.h`](../../engine/thirdparty/rmlui/Source/Core/DataModel.h) | 75 | _symbols:_ DataViews, DataControllers, DataVariable, Element |
| [`PropertyDictionary.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyDictionary.cpp) | 75 |  |
| [`ttpload.h`](../../engine/thirdparty/freetype/src/truetype/ttpload.h) | 74 | ttpload.h TrueType-specific tables loader (specification). |
| [`FontEngineInterfaceDefault.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FontEngineInterfaceDefault.cpp) | 74 |  |
| [`TextureDatabase.h`](../../engine/thirdparty/rmlui/Source/Core/TextureDatabase.h) | 74 | _symbols:_ RenderInterface, CallbackTextureDatabase, CallbackTextureEntry, FileTextureDatabase |
| [`TextureLayoutRow.cpp`](../../engine/thirdparty/rmlui/Source/Core/TextureLayoutRow.cpp) | 74 |  |
| [`test_scenario_catalog.cpp`](../../engine/native/tests/test_scenario_catalog.cpp) | 73 | test_scenario_catalog.cpp — proves engine/native's OWN Scale-0 scenario catalog (native/scenario_catalog.h, ftd::native) stays set-equal with the canonical engine registry ftd::scale0_scenario_ids(). |
| [`gxvmorx.h`](../../engine/thirdparty/freetype/src/gxvalid/gxvmorx.h) | 73 | gxvmorx.h TrueTypeGX/AAT common definition for morx table (specification). |
| [`t1decode.h`](../../engine/thirdparty/freetype/src/psaux/t1decode.h) | 73 | t1decode.h PostScript Type 1 decoding routines (specification). |
| [`InputType.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputType.h) | 73 | An interface for a input type handler used by ElementFormControlInput. |
| [`svgldict.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svgldict.h) | 72 | svgldict.h The FreeType glyph dictionary services (specification). |
| [`svgxval.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svgxval.h) | 72 | svgxval.h FreeType API for validating TrueTypeGX/AAT tables (specification). |
| [`bdfdrivr.h`](../../engine/thirdparty/freetype/src/bdf/bdfdrivr.h) | 72 |  |
| [`InputTypeRange.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputTypeRange.cpp) | 72 | event |
| [`FilterDropShadow.cpp`](../../engine/thirdparty/rmlui/Source/Core/FilterDropShadow.cpp) | 72 | name |
| [`PropertyParserColorStopList.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserColorStopList.cpp) | 72 |  |
| [`afshaper.h`](../../engine/thirdparty/freetype/src/autofit/afshaper.h) | 71 | afshaper.h HarfBuzz interface for accessing OpenType features (specification). |
| [`ComputedValues.cpp`](../../engine/thirdparty/rmlui/Source/Core/ComputedValues.cpp) | 71 |  |
| [`scene_rect.h`](../../engine/native/include/native/scene_rect.h) | 70 | _symbols:_ SceneRect |
| [`psconv.h`](../../engine/thirdparty/freetype/src/psaux/psconv.h) | 70 | psconv.h Some convenience conversions (specification). |
| [`TextureLayout.cpp`](../../engine/thirdparty/rmlui/Source/Core/TextureLayout.cpp) | 70 | _symbols:_ RectangleSort |
| [`svcid.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svcid.h) | 69 | svcid.h The FreeType CID font services (specification). |
| [`FontProvider.h`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FontProvider.h) | 69 | The font provider contains all font families currently in use by RmlUi. |
| [`InlineBox.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/InlineBox.h) | 69 | Inline boxes are inline-level boxes whose contents (child boxes) participate in the same inline formatting context as the box itself. |
| [`Memory.cpp`](../../engine/thirdparty/rmlui/Source/Core/Memory.cpp) | 69 |  |
| [`psread.h`](../../engine/thirdparty/freetype/src/psaux/psread.h) | 68 | psread.h Adobe's code for stream handling (specification). |
| [`PropertyDefinition.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/PropertyDefinition.h) | 68 | _symbols:_ RMLUICORE_API, ParserState |
| [`ElementInstancer.cpp`](../../engine/thirdparty/rmlui/Source/Core/ElementInstancer.cpp) | 68 | parent |
| [`Log.cpp`](../../engine/thirdparty/rmlui/Source/Core/Log.cpp) | 68 |  |
| [`TextureLayoutRectangle.cpp`](../../engine/thirdparty/rmlui/Source/Core/TextureLayoutRectangle.cpp) | 68 |  |
| [`cffcmap.h`](../../engine/thirdparty/freetype/src/cff/cffcmap.h) | 67 | cffcmap.h CFF character mapping table (cmap) support (specification). |
| [`inftrees.h`](../../engine/thirdparty/freetype/src/gzip/inftrees.h) | 67 |  |
| [`FontEffect.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEffect.cpp) | 67 | origin |
| [`FontFamily.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FontFamily.cpp) | 67 |  |
| [`svbdf.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svbdf.h) | 66 |  |
| [`svprop.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svprop.h) | 66 | svprop.h The FreeType property service (specification). |
| [`ElementTabSet.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Elements/ElementTabSet.h) | 66 | A tabulated set of panels. |
| [`TransformState.cpp`](../../engine/thirdparty/rmlui/Source/Core/TransformState.cpp) | 66 |  |
| [`svpfr.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svpfr.h) | 65 | svpfr.h Internal PFR service functions (specification). |
| [`svpostnm.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svpostnm.h) | 65 | svpostnm.h The FreeType PostScript name services (specification). |
| [`Animation.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Animation.h) | 65 | Data parsed from the 'animation' property. |
| [`SystemInterface.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/SystemInterface.h) | 65 | RmlUi's system interface provides an interface for time, translation, logging, and other system utilities. |
| [`ElementEffects.h`](../../engine/thirdparty/rmlui/Source/Core/ElementEffects.h) | 65 | Manages and renders an element's effects: decorators, filters, backdrop filters, and mask images. |
| [`FontFace.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FontFace.cpp) | 65 |  |
| [`TableFormattingContext.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/TableFormattingContext.h) | 65 | Formats a table element and its parts according to table layout rules. |
| [`RenderInterface.cpp`](../../engine/thirdparty/rmlui/Source/Core/RenderInterface.cpp) | 65 | enable |
| [`InputTypeText.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputTypeText.h) | 64 | A single-line input type handler. |
| [`cffdecode.h`](../../engine/thirdparty/freetype/src/psaux/cffdecode.h) | 63 | cffdecode.h PostScript CFF (Type 2) decoding routines (specification). |
| [`FontEffect.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/FontEffect.h) | 63 | _symbols:_ RMLUICORE_API |
| [`cffgload.h`](../../engine/thirdparty/freetype/src/cff/cffgload.h) | 62 |  |
| [`ttsbit.h`](../../engine/thirdparty/freetype/src/sfnt/ttsbit.h) | 62 | ttsbit.h TrueType and OpenType embedded bitmap support (specification). |
| [`TextInputContext.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/TextInputContext.h) | 62 | Interface for an editable text area. |
| [`ControlledLifetimeResource.h`](../../engine/thirdparty/rmlui/Source/Core/ControlledLifetimeResource.h) | 62 | leaked |
| [`ElementAnimation.h`](../../engine/thirdparty/rmlui/Source/Core/ElementAnimation.h) | 62 | _symbols:_ AnimationKey, ElementAnimation |
| [`ElementFormControl.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/ElementFormControl.cpp) | 62 |  |
| [`FontEffectShadow.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEffectShadow.cpp) | 62 | dimensions |
| [`FontEngineInterface.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEngineInterface.cpp) | 62 | file_path |
| [`TextureLayoutRectangle.h`](../../engine/thirdparty/rmlui/Source/Core/TextureLayoutRectangle.h) | 62 | A texture layout rectangle is an area positioned with a texture layout. |
| [`test_d3d12_shared_buffer.cpp`](../../engine/native/tests/test_d3d12_shared_buffer.cpp) | 61 | Off-screen: a message-only window is enough to build a swapchain-free D3D12 device via initialize()'s HWND-taking path. |
| [`ttgload.h`](../../engine/thirdparty/freetype/src/truetype/ttgload.h) | 61 |  |
| [`ConvolutionFilter.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/ConvolutionFilter.h) | 61 | A programmable convolution filter, designed to aid in the generation of texture data by custom FontEffect types. |
| [`FileInterface.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/FileInterface.h) | 61 | The abstract base class for application-specific file I/O. |
| [`XMLNodeHandlerDefault.cpp`](../../engine/thirdparty/rmlui/Source/Core/XMLNodeHandlerDefault.cpp) | 61 | parser |
| [`psauxmod.h`](../../engine/thirdparty/freetype/src/psaux/psauxmod.h) | 60 | psauxmod.h FreeType auxiliary PostScript module implementation (specification). |
| [`MeshUtilities.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/MeshUtilities.h) | 60 | A class containing helper functions for generating meshes. |
| [`Property.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Property.h) | 60 | _symbols:_ PropertyDefinition, RMLUICORE_API |
| [`InputTypeSubmit.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputTypeSubmit.cpp) | 60 | dimensions |
| [`RenderBox.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/RenderBox.h) | 59 | Provides the data needed to generate a mesh for a given element's box. |
| [`command_applier.h`](../../engine/native/include/native/command_applier.h) | 58 | _symbols:_ NativeTelemetryScheduler, RenderBridge, NativeEngineSession, UiBoundaryState |
| [`ftconfig.h`](../../engine/thirdparty/freetype/builds/vms/ftconfig.h) | 58 | ftconfig.h VMS-specific configuration file (specification only). |
| [`cidgload.h`](../../engine/thirdparty/freetype/src/cid/cidgload.h) | 58 |  |
| [`sfobjs.h`](../../engine/thirdparty/freetype/src/sfnt/sfobjs.h) | 58 |  |
| [`FontEffectInstancer.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/FontEffectInstancer.h) | 58 | A font effect instancer provides a method for allocating and deallocating font effects. |
| [`InputTypeCheckbox.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputTypeCheckbox.cpp) | 58 |  |
| [`XMLNodeHandlerBody.cpp`](../../engine/thirdparty/rmlui/Source/Core/XMLNodeHandlerBody.cpp) | 58 | name |
| [`ftgrays.h`](../../engine/thirdparty/freetype/src/smooth/ftgrays.h) | 57 |  |
| [`Spritesheet.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Spritesheet.h) | 57 | Spritesheet holds a list of sprite names given in the @spritesheet at-rule in RCSS. |
| [`Texture.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Texture.h) | 57 | Texture is a simple view of either a file texture or a callback texture. |
| [`ElementForm.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/ElementForm.cpp) | 57 |  |
| [`XMLNodeHandlerSelect.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/XMLNodeHandlerSelect.cpp) | 57 |  |
| [`GeometryBoxShadow.h`](../../engine/thirdparty/rmlui/Source/Core/GeometryBoxShadow.h) | 57 | _symbols:_ BoxShadowGeometryInfo, Geometry, CallbackTexture, RenderManager |
| [`svttglyf.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svttglyf.h) | 56 |  |
| [`ttcmapc.h`](../../engine/thirdparty/freetype/src/sfnt/ttcmapc.h) | 56 | ttcmapc.h TT CMAP classes definitions (specification only). |
| [`t42types.h`](../../engine/thirdparty/freetype/src/type42/t42types.h) | 56 | t42types.h Type 42 font data types (specification only). |
| [`Plugin.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Plugin.h) | 56 | Generic Interface for plugins to RmlUi. |
| [`PropertyDictionary.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/PropertyDictionary.h) | 56 | A dictionary to property names to values. |
| [`UniqueRenderResource.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/UniqueRenderResource.h) | 56 | Abstraction for a uniquely owned render resource. |
| [`DataModelHandle.cpp`](../../engine/thirdparty/rmlui/Source/Core/DataModelHandle.cpp) | 56 |  |
| [`DecoratorText.h`](../../engine/thirdparty/rmlui/Source/Core/DecoratorText.h) | 56 | _symbols:_ DecoratorText, TexturedGeometry, ElementData, DecoratorTextInstancer |
| [`ObserverPtr.cpp`](../../engine/thirdparty/rmlui/Source/Core/ObserverPtr.cpp) | 56 | _symbols:_ ObserverPtrData |
| [`test_ui_command_queue.cpp`](../../engine/native/tests/test_ui_command_queue.cpp) | 55 |  |
| [`ftconfig.h`](../../engine/thirdparty/freetype/builds/amiga/include/config/ftconfig.h) | 55 | / /* |
| [`svfntfmt.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svfntfmt.h) | 55 | svfntfmt.h The FreeType font format service (specification only). |
| [`svotval.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svotval.h) | 55 | svotval.h The FreeType OpenType validation service (specification). |
| [`afmodule.h`](../../engine/thirdparty/freetype/src/autofit/afmodule.h) | 55 | afmodule.h Auto-fitter module implementation (specification). |
| [`pcfutil.h`](../../engine/thirdparty/freetype/src/pcf/pcfutil.h) | 55 |  |
| [`Header.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Header.h) | 55 | Note: Changing a RMLUICORE_API_INLINE method breaks ABI compatibility!! |
| [`ttmtx.h`](../../engine/thirdparty/freetype/src/sfnt/ttmtx.h) | 54 | ttmtx.h Load the metrics tables common to TTF and OTF fonts (specification). |
| [`PropertyParserColour.h`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserColour.h) | 54 | A property parser that parses a colour value. |
| [`test_ui_load_scenario.cpp`](../../engine/native/tests/test_ui_load_scenario.cpp) | 53 |  |
| [`svtteng.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svtteng.h) | 53 | svtteng.h The FreeType TrueType engine query service (specification). |
| [`ttgpos.h`](../../engine/thirdparty/freetype/src/sfnt/ttgpos.h) | 53 | ttgpos.c Load the TrueType GPOS table. |
| [`t1afm.h`](../../engine/thirdparty/freetype/src/type1/t1afm.h) | 53 | t1afm.h AFM support for Type 1 fonts (specification). |
| [`ElementFormControl.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Elements/ElementFormControl.h) | 53 | A generic specialisation of the generic Element for all input controls. |
| [`Transform.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Transform.h) | 53 | The Transform class holds the information parsed from an element's `transform' property. |
| [`DecoratorTiledBox.h`](../../engine/thirdparty/rmlui/Source/Core/DecoratorTiledBox.h) | 53 | _symbols:_ DecoratorTiledBox, DecoratorTiledBoxInstancer |
| [`TextureLayout.h`](../../engine/thirdparty/rmlui/Source/Core/TextureLayout.h) | 53 | A texture layout generates and stores a layout of rectangles within a series of textures. |
| [`test_d3d12_debug_observability.cpp`](../../engine/native/tests/test_d3d12_debug_observability.cpp) | 52 |  |
| [`cidload.h`](../../engine/thirdparty/freetype/src/cid/cidload.h) | 52 | cidload.h CID-keyed Type1 font loader (specification). |
| [`t1gload.h`](../../engine/thirdparty/freetype/src/type1/t1gload.h) | 52 |  |
| [`DataControllerDefault.h`](../../engine/thirdparty/rmlui/Source/Core/DataControllerDefault.h) | 52 | _symbols:_ Element, DataModel, DataExpression, DataControllerValue |
| [`FilterBasic.cpp`](../../engine/thirdparty/rmlui/Source/Core/FilterBasic.cpp) | 52 |  |
| [`FormattingContext.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/FormattingContext.cpp) | 52 |  |
| [`StyleSheetFactory.h`](../../engine/thirdparty/rmlui/Source/Core/StyleSheetFactory.h) | 52 | Creates stylesheets on the fly as needed. |
| [`ui_result.h`](../../engine/native/include/native/ui_result.h) | 51 | _symbols:_ ApplyResult, ObservationResult, ReloadResult, TickResult |
| [`app_input.h`](../../engine/native/src/app/app_input.h) | 51 | app/app_input.h — the Win32 window procedure and the RmlOverlay recorder that draws the RmlUi shell into the presenter's command list. |
| [`test_ui_scene_rect.cpp`](../../engine/native/tests/test_ui_scene_rect.cpp) | 51 |  |
| [`ftconfig.h`](../../engine/thirdparty/freetype/include/freetype/config/ftconfig.h) | 51 | ftconfig.h ANSI-specific configuration file (specification only). |
| [`svkern.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svkern.h) | 51 | svkern.h The FreeType Kerning service (specification). |
| [`gxverror.h`](../../engine/thirdparty/freetype/src/gxvalid/gxverror.h) | 51 | gxverror.h TrueTypeGX/AAT validation module error codes (specification only). |
| [`ttkern.h`](../../engine/thirdparty/freetype/src/sfnt/ttkern.h) | 51 | ttkern.h Load the basic TrueType kerning table. |
| [`DataExpression.h`](../../engine/thirdparty/rmlui/Source/Core/DataExpression.h) | 51 | _symbols:_ Element, DataModel, InstructionData, DataExpressionInterface |
| [`ElementHandle.h`](../../engine/thirdparty/rmlui/Source/Core/ElementHandle.h) | 51 | A derivation of an element for use as a mouse drag handle. |
| [`XMLNodeHandlerTextArea.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/XMLNodeHandlerTextArea.cpp) | 51 | parser |
| [`ReplacedFormattingContext.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/ReplacedFormattingContext.cpp) | 51 |  |
| [`svwinfnt.h`](../../engine/thirdparty/freetype/include/freetype/internal/services/svwinfnt.h) | 50 | svwinfnt.h The FreeType Windows FNT/FONT service (specification). |
| [`pngshim.h`](../../engine/thirdparty/freetype/src/sfnt/pngshim.h) | 50 |  |
| [`Platform.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Platform.h) | 50 | declaration of 'identifier' hides class member |
| [`FontEffectGlow.h`](../../engine/thirdparty/rmlui/Source/Core/FontEffectGlow.h) | 50 | A font effect for rendering glow around text. |
| [`StreamFile.h`](../../engine/thirdparty/rmlui/Source/Core/StreamFile.h) | 50 | _symbols:_ StreamFile |
| [`mac-support.h`](../../engine/thirdparty/freetype/include/freetype/config/mac-support.h) | 49 | config/mac-support.h Mac/OS X support configuration header. |
| [`pfrgload.h`](../../engine/thirdparty/freetype/src/pfr/pfrgload.h) | 49 | pfrgload.h FreeType PFR glyph loader (specification). |
| [`ttbdf.h`](../../engine/thirdparty/freetype/src/sfnt/ttbdf.h) | 49 | ttbdf.h TrueType and OpenType embedded BDF properties (specification). |
| [`Filter.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Filter.h) | 49 | The abstract base class for visual filters that are applied when rendering the element. |
| [`Profiling.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Profiling.h) | 49 |  |
| [`DecoratorShader.h`](../../engine/thirdparty/rmlui/Source/Core/DecoratorShader.h) | 49 | _symbols:_ DecoratorShader, DecoratorShaderInstancer, PropertyIds, ShaderElementData |
| [`Texture.cpp`](../../engine/thirdparty/rmlui/Source/Core/Texture.cpp) | 49 |  |
| [`test_dpi_awareness.cpp`](../../engine/native/tests/test_dpi_awareness.cpp) | 48 |  |
| [`ft-hb.h`](../../engine/thirdparty/freetype/src/autofit/ft-hb.h) | 48 |  |
| [`ttcpal.h`](../../engine/thirdparty/freetype/src/sfnt/ttcpal.h) | 48 | ttcpal.h TrueType and OpenType color palette support (specification). |
| [`DocumentHeader.h`](../../engine/thirdparty/rmlui/Source/Core/DocumentHeader.h) | 48 | The document header struct contains the header details gathered from an XML document parse. |
| [`FilterBlur.cpp`](../../engine/thirdparty/rmlui/Source/Core/FilterBlur.cpp) | 48 | name |
| [`FontEngineInterfaceDefault.h`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FontEngineInterfaceDefault.h) | 48 | _symbols:_ RMLUICORE_API |
| [`LayoutBox.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/LayoutBox.h) | 48 | A box used to represent the formatting structure of the document, taking part in the box tree. |
| [`RenderManagerAccess.cpp`](../../engine/thirdparty/rmlui/Source/Core/RenderManagerAccess.cpp) | 48 |  |
| [`RenderManagerAccess.h`](../../engine/thirdparty/rmlui/Source/Core/RenderManagerAccess.h) | 48 | _symbols:_ CompiledFilter, CompiledShader, CallbackTexture, Geometry |
| [`XMLNodeHandlerTemplate.cpp`](../../engine/thirdparty/rmlui/Source/Core/XMLNodeHandlerTemplate.cpp) | 48 | parser |
| [`command_bus.h`](../../engine/native/include/native/host/command_bus.h) | 47 | host/command_bus.h — the scale-generic command FIFO. |
| [`app_util.cpp`](../../engine/native/src/app/app_util.cpp) | 47 | app/app_util.cpp — portable string/format helpers (see app/app_util.h). |
| [`test_cuda_d3d12_adapter_match.cpp`](../../engine/native/tests/test_cuda_d3d12_adapter_match.cpp) | 47 |  |
| [`ftpsprop.h`](../../engine/thirdparty/freetype/include/freetype/internal/ftpsprop.h) | 47 | ftpsprop.h Get and set properties of PostScript drivers (specification). |
| [`afranges.h`](../../engine/thirdparty/freetype/src/autofit/afranges.h) | 47 | afranges.h Auto-fitter Unicode script ranges (specification). |
| [`ftraster.h`](../../engine/thirdparty/freetype/src/raster/ftraster.h) | 47 | ftraster.h The FreeType glyph rasterizer (specification). |
| [`StyleSheetContainer.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/StyleSheetContainer.h) | 47 | StyleSheetContainer contains a list of media blocks and creates a combined style sheet when getting properties of the current context regarding the available media features. |
| [`InputType.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputType.cpp) | 47 | changed_attributes |
| [`command_queue.cpp`](../../engine/native/src/command_queue.cpp) | 46 |  |
| [`svginterface.h`](../../engine/thirdparty/freetype/include/freetype/internal/svginterface.h) | 46 | svginterface.h Interface of ot-svg module (specification only). |
| [`gxvmod.h`](../../engine/thirdparty/freetype/src/gxvalid/gxvmod.h) | 46 | gxvmod.h FreeType's TrueTypeGX/AAT validation module implementation (specification). |
| [`ttpost.h`](../../engine/thirdparty/freetype/src/sfnt/ttpost.h) | 46 | ttpost.h PostScript name table processing for TrueType and OpenType fonts (specification). |
| [`ComputeProperty.h`](../../engine/thirdparty/rmlui/Source/Core/ComputeProperty.h) | 46 | _symbols:_ Property |
| [`FontEffectBlur.h`](../../engine/thirdparty/rmlui/Source/Core/FontEffectBlur.h) | 46 | A concrete font effect for rendering Gaussian blurred text. |
| [`FontEffectOutline.h`](../../engine/thirdparty/rmlui/Source/Core/FontEffectOutline.h) | 46 | A concrete font effect for rendering outlines around text. |
| [`FontFamily.h`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FontFamily.h) | 46 | _symbols:_ FontFace, FontFaceHandleDefault, FontFamily, FontFaceEntry |
| [`md5.h`](../../engine/thirdparty/freetype/src/base/md5.h) | 45 | This is an OpenSSL-compatible implementation of the RSA Data Security, Inc. |
| [`bdferror.h`](../../engine/thirdparty/freetype/src/bdf/bdferror.h) | 45 |  |
| [`pfrcmap.h`](../../engine/thirdparty/freetype/src/pfr/pfrcmap.h) | 45 | pfrcmap.h FreeType PFR cmap handling (specification). |
| [`PropertiesIteratorView.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/PropertiesIteratorView.h) | 45 | Provides an iterator for properties defined in the element's style or definition. |
| [`InputTypeRadio.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputTypeRadio.h) | 45 | A radio button input type handler. |
| [`InputTypeRange.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputTypeRange.h) | 45 | A range input type handler. |
| [`FileInterfaceDefault.h`](../../engine/thirdparty/rmlui/Source/Core/FileInterfaceDefault.h) | 45 | Implementation of the RmlUi file interface using the Standard C file functions. |
| [`PropertiesIteratorView.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertiesIteratorView.cpp) | 45 |  |
| [`PropertyShorthandDefinition.h`](../../engine/thirdparty/rmlui/Source/Core/PropertyShorthandDefinition.h) | 45 | _symbols:_ PropertyDefinition, ShorthandDefinition, ShorthandItem |
| [`pcfdrivr.h`](../../engine/thirdparty/freetype/src/pcf/pcfdrivr.h) | 44 |  |
| [`pcfread.h`](../../engine/thirdparty/freetype/src/pcf/pcfread.h) | 44 |  |
| [`CompiledFilterShader.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/CompiledFilterShader.h) | 44 | A compiled filter to be applied when compositing layers in the render manager. |
| [`TextureLayoutRow.h`](../../engine/thirdparty/rmlui/Source/Core/TextureLayoutRow.h) | 44 | A texture layout row is a single row of rectangles positioned vertically within a texture. |
| [`sfwoff.h`](../../engine/thirdparty/freetype/src/sfnt/sfwoff.h) | 43 |  |
| [`ttsvg.h`](../../engine/thirdparty/freetype/src/sfnt/ttsvg.h) | 43 |  |
| [`EffectSpecification.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/EffectSpecification.h) | 43 | Specifies properties and shorthands for effects (decorators and filters). |
| [`LogDefault.cpp`](../../engine/thirdparty/rmlui/Source/Core/LogDefault.cpp) | 43 | type |
| [`PluginRegistry.h`](../../engine/thirdparty/rmlui/Source/Core/PluginRegistry.h) | 43 | _symbols:_ Context, Element, ElementDocument, Plugin |
| [`TextureLayoutTexture.h`](../../engine/thirdparty/rmlui/Source/Core/TextureLayoutTexture.h) | 43 | A texture layout texture is a single rectangular area which sub-rectangles are placed on within a complete texture layout. |
| [`overlay_recorder.h`](../../engine/native/include/native/overlay_recorder.h) | 42 | _symbols:_ RenderTargetInfo, OverlayRecorder, PresenterUiContext |
| [`ft2build.h`](../../engine/thirdparty/freetype/include/ft2build.h) | 42 |  |
| [`aferrors.h`](../../engine/thirdparty/freetype/src/autofit/aferrors.h) | 42 | aferrors.h Autofitter error codes (specification only). |
| [`ftcerror.h`](../../engine/thirdparty/freetype/src/cache/ftcerror.h) | 42 | ftcerror.h Caching sub-system error codes (specification only). |
| [`cfferrs.h`](../../engine/thirdparty/freetype/src/cff/cfferrs.h) | 42 |  |
| [`otverror.h`](../../engine/thirdparty/freetype/src/otvalid/otverror.h) | 42 | otverror.h OpenType validation module error codes (specification only). |
| [`psauxerr.h`](../../engine/thirdparty/freetype/src/psaux/psauxerr.h) | 42 | psauxerr.h PS auxiliary module error codes (specification only). |
| [`psnamerr.h`](../../engine/thirdparty/freetype/src/psnames/psnamerr.h) | 42 | psnamerr.h PS names module error codes (specification only). |
| [`rasterrs.h`](../../engine/thirdparty/freetype/src/raster/rasterrs.h) | 42 | rasterrs.h monochrome renderer error codes (specification only). |
| [`ftsmerrs.h`](../../engine/thirdparty/freetype/src/smooth/ftsmerrs.h) | 42 | ftsmerrs.h smooth renderer error codes (specification only). |
| [`svgtypes.h`](../../engine/thirdparty/freetype/src/svg/svgtypes.h) | 42 | svgtypes.h The FreeType SVG renderer internal types (specification). |
| [`tterrors.h`](../../engine/thirdparty/freetype/src/truetype/tterrors.h) | 42 | tterrors.h TrueType error codes (specification only). |
| [`fnterrs.h`](../../engine/thirdparty/freetype/src/winfonts/fnterrs.h) | 42 | fnterrs.h Win FNT/FON error codes (specification only). |
| [`FontGlyph.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/FontGlyph.h) | 42 | Metrics and bitmap data for a single glyph within a font face. |
| [`DecoratorTiledHorizontal.h`](../../engine/thirdparty/rmlui/Source/Core/DecoratorTiledHorizontal.h) | 42 | _symbols:_ DecoratorTiledHorizontal, DecoratorTiledHorizontalInstancer |
| [`DecoratorTiledVertical.h`](../../engine/thirdparty/rmlui/Source/Core/DecoratorTiledVertical.h) | 42 | _symbols:_ DecoratorTiledVertical, DecoratorTiledVerticalInstancer |
| [`InputTypeButton.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputTypeButton.cpp) | 42 | event |
| [`FontEffectShadow.h`](../../engine/thirdparty/rmlui/Source/Core/FontEffectShadow.h) | 42 | A concrete font effect for rendering text shadows. |
| [`FlexFormattingContext.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/FlexFormattingContext.h) | 42 | Formats a flex container element and its flex items according to flexible box (flexbox) layout rules. |
| [`test_spectrum.cpp`](../../engine/native/tests/test_spectrum.cpp) | 41 | test_spectrum.cpp — unit check for native/spectrum.h: Parseval + a known peak. |
| [`afindic.h`](../../engine/thirdparty/freetype/src/autofit/afindic.h) | 41 | afindic.h Auto-fitter hinting routines for Indic writing system (specification). |
| [`ciderrs.h`](../../engine/thirdparty/freetype/src/cid/ciderrs.h) | 41 |  |
| [`pcferror.h`](../../engine/thirdparty/freetype/src/pcf/pcferror.h) | 41 |  |
| [`pfrerror.h`](../../engine/thirdparty/freetype/src/pfr/pfrerror.h) | 41 |  |
| [`pshnterr.h`](../../engine/thirdparty/freetype/src/pshinter/pshnterr.h) | 41 | pshnterr.h PS Hinter error codes (specification only). |
| [`sferrors.h`](../../engine/thirdparty/freetype/src/sfnt/sferrors.h) | 41 |  |
| [`woff2tags.h`](../../engine/thirdparty/freetype/src/sfnt/woff2tags.h) | 41 |  |
| [`t1errors.h`](../../engine/thirdparty/freetype/src/type1/t1errors.h) | 41 | t1errors.h Type 1 error codes (specification only). |
| [`t42error.h`](../../engine/thirdparty/freetype/src/type42/t42error.h) | 41 | t42error.h Type 42 error codes (specification only). |
| [`Span.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Span.h) | 41 | Basic implementation of a span, which refers to a contiguous sequence of objects. |
| [`DataController.cpp`](../../engine/thirdparty/rmlui/Source/Core/DataController.cpp) | 41 |  |
| [`DecoratorNinePatch.h`](../../engine/thirdparty/rmlui/Source/Core/DecoratorNinePatch.h) | 41 | _symbols:_ DecoratorNinePatch, DecoratorNinePatchInstancer |
| [`DocumentHeader.cpp`](../../engine/thirdparty/rmlui/Source/Core/DocumentHeader.cpp) | 41 |  |
| [`ElementBackgroundBorder.h`](../../engine/thirdparty/rmlui/Source/Core/ElementBackgroundBorder.h) | 41 | _symbols:_ BoxShadowRenderable, ElementBackgroundBorder, Background |
| [`InputTypeButton.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputTypeButton.h) | 41 | A button input type handler. |
| [`FileInterface.cpp`](../../engine/thirdparty/rmlui/Source/Core/FileInterface.cpp) | 41 |  |
| [`FreeTypeInterface.h`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FreeTypeInterface.h) | 41 |  |
| [`BlockFormattingContext.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/BlockFormattingContext.h) | 41 | Places boxes according to normal flow, while handling floated boxes. |
| [`backgrounds.h`](../../engine/native/include/native/backgrounds.h) | 40 | Environment backgrounds for the Scale-0 viewport (native parity with the web dashboard's background selector). |
| [`afdummy.h`](../../engine/thirdparty/freetype/src/autofit/afdummy.h) | 40 | afdummy.h Auto-fitter dummy routines to be used if no hinting should be performed (specification). |
| [`Tween.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Tween.h) | 40 | _symbols:_ RMLUICORE_API |
| [`DecoratorTiledImage.h`](../../engine/thirdparty/rmlui/Source/Core/DecoratorTiledImage.h) | 40 | _symbols:_ DecoratorTiledImage, DecoratorTiledImageInstancer |
| [`Template.h`](../../engine/thirdparty/rmlui/Source/Core/Template.h) | 40 | Contains a RML template. |
| [`TransformUtilities.h`](../../engine/thirdparty/rmlui/Source/Core/TransformUtilities.h) | 40 | _symbols:_ TransformPrimitive, DecomposedMatrix4 |
| [`XMLParseTools.h`](../../engine/thirdparty/rmlui/Source/Core/XMLParseTools.h) | 40 | Tools for aiding in parsing XML documents. |
| [`ui_snapshot_builder.cpp`](../../engine/native/src/ui_snapshot_builder.cpp) | 39 |  |
| [`XMLNodeHandler.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/XMLNodeHandler.h) | 39 | A handler gets ElementStart, ElementEnd and ElementData called by the XMLParser. |
| [`EventSpecification.h`](../../engine/thirdparty/rmlui/Source/Core/EventSpecification.h) | 39 | _symbols:_ EventSpecification |
| [`Geometry.cpp`](../../engine/thirdparty/rmlui/Source/Core/Geometry.cpp) | 39 |  |
| [`InlineTypes.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/InlineTypes.h) | 39 | _symbols:_ FragmentConstructor, PlacedFragment |
| [`PropertyParserRatio.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserRatio.cpp) | 39 | parameters |
| [`Transform.cpp`](../../engine/thirdparty/rmlui/Source/Core/Transform.cpp) | 39 |  |
| [`main.cpp`](../../engine/native/src/app/main.cpp) | 38 | Thin Win32 entry point for the native desktop application. |
| [`otvmod.h`](../../engine/thirdparty/freetype/src/otvalid/otvmod.h) | 38 | otvmod.h FreeType's OpenType validation module implementation (specification). |
| [`pshmod.h`](../../engine/thirdparty/freetype/src/pshinter/pshmod.h) | 38 | pshmod.h PostScript hinter module interface (specification). |
| [`DecorationTypes.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/DecorationTypes.h) | 38 | _symbols:_ ColorStop, BoxShadow |
| [`InputTypeCheckbox.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputTypeCheckbox.h) | 38 | A checkbox input type handler. |
| [`InputTypeSubmit.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/InputTypeSubmit.h) | 38 | A submit input type handler. |
| [`FontFace.h`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FontFace.h) | 38 | _symbols:_ FontFaceHandleDefault, FontFace |
| [`TransformState.h`](../../engine/thirdparty/rmlui/Source/Core/TransformState.h) | 38 | _symbols:_ TransformState |
| [`snapshot_bus.h`](../../engine/native/include/native/host/snapshot_bus.h) | 37 | host/snapshot_bus.h — immutable publication of the scale-generic HostSnapshot. |
| [`pfrsbit.h`](../../engine/thirdparty/freetype/src/pfr/pfrsbit.h) | 37 | pfrsbit.h FreeType PFR bitmap loader (specification). |
| [`psmodule.h`](../../engine/thirdparty/freetype/src/psnames/psmodule.h) | 37 | psmodule.h High-level psnames module interface (specification). |
| [`ftrend1.h`](../../engine/thirdparty/freetype/src/raster/ftrend1.h) | 37 | ftrend1.h The FreeType glyph rasterizer interface (specification). |
| [`ftsdferrs.h`](../../engine/thirdparty/freetype/src/sdf/ftsdferrs.h) | 37 | ftsdferrs.h Signed Distance Field error codes (specification only). |
| [`ftsmooth.h`](../../engine/thirdparty/freetype/src/smooth/ftsmooth.h) | 37 | ftsmooth.h Anti-aliasing renderer interface (specification). |
| [`ScrollTypes.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/ScrollTypes.h) | 37 | Defines behavior of Element::ScrollIntoView. |
| [`ElementMeta.h`](../../engine/thirdparty/rmlui/Source/Core/ElementMeta.h) | 37 | _symbols:_ ElementMeta, ElementMetaPool |
| [`FormattingContext.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/FormattingContext.h) | 37 | An environment in which related boxes are layed out. |
| [`PropertyParserNumber.h`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserNumber.h) | 37 | A property parser that parses a floating-point number with an optional unit. |
| [`PropertyParserTransform.h`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserTransform.h) | 37 | A property parser that parses a RCSS transform property specification. |
| [`cidriver.h`](../../engine/thirdparty/freetype/src/cid/cidriver.h) | 36 | cidriver.h High-level CID driver interface (specification). |
| [`otvgpos.h`](../../engine/thirdparty/freetype/src/otvalid/otvgpos.h) | 36 | otvgpos.h OpenType GPOS table validator (specification). |
| [`pfrdrivr.h`](../../engine/thirdparty/freetype/src/pfr/pfrdrivr.h) | 36 | pfrdrivr.h High-level Type PFR driver interface (specification). |
| [`t42drivr.h`](../../engine/thirdparty/freetype/src/type42/t42drivr.h) | 36 | t42drivr.h High-level Type 42 driver interface (specification). |
| [`Log.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Log.h) | 36 | RmlUi logging API. |
| [`PropertyParserAnimation.h`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserAnimation.h) | 36 | Parses the RCSS 'animation' and 'transition' property specifications. |
| [`TemplateCache.h`](../../engine/thirdparty/rmlui/Source/Core/TemplateCache.h) | 36 | Manages requests for loading templates, caching as it goes. |
| [`knot_snapshot.h`](../../engine/native/include/native/knot_snapshot.h) | 35 | _symbols:_ KnotRowUi, KnotSnapshot |
| [`test_ui_boot_snapshot.cpp`](../../engine/native/tests/test_ui_boot_snapshot.cpp) | 35 |  |
| [`cffdrivr.h`](../../engine/thirdparty/freetype/src/cff/cffdrivr.h) | 35 | cffdrivr.h High-level OpenType driver interface (specification). |
| [`sfdriver.h`](../../engine/thirdparty/freetype/src/sfnt/sfdriver.h) | 35 | sfdriver.h High-level SFNT driver interface (specification). |
| [`ftsvg.h`](../../engine/thirdparty/freetype/src/svg/ftsvg.h) | 35 | ftsvg.h The FreeType SVG renderer interface (specification). |
| [`ttdriver.h`](../../engine/thirdparty/freetype/src/truetype/ttdriver.h) | 35 | ttdriver.h High-level TrueType driver interface (specification). |
| [`t1driver.h`](../../engine/thirdparty/freetype/src/type1/t1driver.h) | 35 | t1driver.h High-level Type 1 driver interface (specification). |
| [`Geometry.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Geometry.h) | 35 | A representation of geometry to be rendered through its underlying render interface. |
| [`TextInputHandler.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/TextInputHandler.h) | 35 | Handler of changes to text editable areas. |
| [`BoxShadowCache.h`](../../engine/thirdparty/rmlui/Source/Core/BoxShadowCache.h) | 35 | _symbols:_ ComputedValues, BoxShadowGeometryInfo, BoxShadowRenderable, BoxShadowCache |
| [`ElementDefinition.h`](../../engine/thirdparty/rmlui/Source/Core/ElementDefinition.h) | 35 | ElementDefinition provides an element's applicable properties from its stylesheet. |
| [`FileInterfaceDefault.cpp`](../../engine/thirdparty/rmlui/Source/Core/FileInterfaceDefault.cpp) | 35 | RMLUI_NO_FILE_INTERFACE_DEFAULT |
| [`FilterDropShadow.h`](../../engine/thirdparty/rmlui/Source/Core/FilterDropShadow.h) | 35 | _symbols:_ FilterDropShadow, FilterDropShadowInstancer, PropertyIds |
| [`ReplacedFormattingContext.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/ReplacedFormattingContext.h) | 35 | A formatting context that handles replaced elements. |
| [`parameter_journal.h`](../../engine/native/include/native/parameter_journal.h) | 34 | _symbols:_ RenderBridge, NativeEngineSession, ParameterJournal |
| [`test_native_cli.cpp`](../../engine/native/tests/test_native_cli.cpp) | 34 |  |
| [`ContextInstancer.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/ContextInstancer.h) | 34 | Abstract instancer interface for instancing contexts. |
| [`FilterBasic.h`](../../engine/thirdparty/rmlui/Source/Core/FilterBasic.h) | 34 | _symbols:_ FilterBasic, FilterBasicInstancer, PropertyIds |
| [`FilterBlur.h`](../../engine/thirdparty/rmlui/Source/Core/FilterBlur.h) | 34 | _symbols:_ FilterBlur, FilterBlurInstancer, PropertyIds |
| [`command_queue.h`](../../engine/native/include/native/command_queue.h) | 33 | _symbols:_ CommandSink, QueuedCommand, CommandQueue |
| [`spectrum.h`](../../engine/native/include/native/spectrum.h) | 33 | native/spectrum.h — the spatial energy spectrum E(k) of the Scale-0 flux field. |
| [`ftmodule.h`](../../engine/thirdparty/freetype/include/freetype/config/ftmodule.h) | 33 | This file registers the FreeType modules compiled into the library. |
| [`afws-decl.h`](../../engine/thirdparty/freetype/src/autofit/afws-decl.h) | 33 | afws-decl.h Auto-fitter writing system declarations (specification only). |
| [`EventInstancer.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/EventInstancer.h) | 33 | Abstract instancer interface for instancing events. |
| [`ElementTextSelection.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/ElementTextSelection.h) | 33 | A stub element used by the WidgetTextInput to query the RCSS-specified text colour and background colour for selected text. |
| [`EventInstancerDefault.h`](../../engine/thirdparty/rmlui/Source/Core/EventInstancerDefault.h) | 33 | Default instancer for instancing events. |
| [`LayoutEngine.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/LayoutEngine.cpp) | 33 |  |
| [`Property.cpp`](../../engine/thirdparty/rmlui/Source/Core/Property.cpp) | 33 |  |
| [`boundary_shapes.h`](../../engine/native/include/native/boundary_shapes.h) | 32 | Boundary-shape wireframes for the Scale-0 viewport (native parity with the web dashboard's boundary-select). |
| [`ui_demand.h`](../../engine/native/include/native/ui_demand.h) | 32 | _symbols:_ DataNeeds |
| [`Dictionary.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Dictionary.h) | 32 |  |
| [`CompiledFilterShader.cpp`](../../engine/thirdparty/rmlui/Source/Core/CompiledFilterShader.cpp) | 32 |  |
| [`ElementLabel.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/ElementLabel.h) | 32 | A specialisation of the generic Core::Element representing a label element. |
| [`ElementTextSelection.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/ElementTextSelection.cpp) | 32 |  |
| [`afws-iter.h`](../../engine/thirdparty/freetype/src/autofit/afws-iter.h) | 31 | afws-iter.h Auto-fitter writing systems iterator (specification only). |
| [`EventListenerInstancer.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/EventListenerInstancer.h) | 31 | Abstract instancer interface for instancing event listeners. |
| [`DecoratorUtilities.cpp`](../../engine/thirdparty/rmlui/Source/Core/DecoratorUtilities.cpp) | 31 |  |
| [`LayoutBox.cpp`](../../engine/thirdparty/rmlui/Source/Core/Layout/LayoutBox.cpp) | 31 | out_baseline |
| [`EventListener.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/EventListener.h) | 30 | Abstract interface class for handling events. |
| [`ElementMeta.cpp`](../../engine/thirdparty/rmlui/Source/Core/ElementMeta.cpp) | 30 |  |
| [`Plugin.cpp`](../../engine/thirdparty/rmlui/Source/Core/Plugin.cpp) | 30 | context |
| [`app_pick.h`](../../engine/native/src/app/app_pick.h) | 29 | app/app_pick.h — camera framing + click-to-inspect ray picking (Scale-0 voxel / Scale-1 particle). |
| [`FontEffectInstancer.cpp`](../../engine/thirdparty/rmlui/Source/Core/FontEffectInstancer.cpp) | 29 |  |
| [`PropertyParserDecorator.h`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserDecorator.h) | 29 | A property parser for the decorator property. |
| [`cli_options.cpp`](../../engine/native/src/cli_options.cpp) | 28 |  |
| [`ui_journal.h`](../../engine/native/include/native/ui_journal.h) | 27 | _symbols:_ JValue, JournalEntry |
| [`app_win32.h`](../../engine/native/src/app/app_win32.h) | 27 | app/app_win32.h — Win32/WIC helpers: UTF-8 command-line parsing, a WIC PNG readback writer, and parent-console attach for the WIN32-subsystem exe. |
| [`ElementForm.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Elements/ElementForm.h) | 27 | A specialisation of the generic Element representing a form element. |
| [`Vertex.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Vertex.h) | 27 | The element that makes up all geometry sent to the renderer. |
| [`ElementDefinition.cpp`](../../engine/thirdparty/rmlui/Source/Core/ElementDefinition.cpp) | 27 |  |
| [`PropertyParserBoxShadow.h`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserBoxShadow.h) | 27 | Parses the RCSS 'box-shadow' property. |
| [`PropertyParser.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/PropertyParser.h) | 26 | A property parser takes a property declaration in string form, validates it, and converts it to a Property. |
| [`ContextInstancerDefault.h`](../../engine/thirdparty/rmlui/Source/Core/ContextInstancerDefault.h) | 26 | Default instancer for instancing contexts. |
| [`WidgetTextInputSingleLinePassword.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/WidgetTextInputSingleLinePassword.cpp) | 26 |  |
| [`Mesh.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Mesh.h) | 25 | _symbols:_ RMLUICORE_API |
| [`NumericValue.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/NumericValue.h) | 25 | A numeric value is a number combined with a unit. |
| [`ContextInstancerDefault.cpp`](../../engine/thirdparty/rmlui/Source/Core/ContextInstancerDefault.cpp) | 25 |  |
| [`EffectSpecification.cpp`](../../engine/thirdparty/rmlui/Source/Core/EffectSpecification.cpp) | 25 |  |
| [`WidgetTextInputSingleLine.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/WidgetTextInputSingleLine.cpp) | 25 |  |
| [`EventInstancerDefault.cpp`](../../engine/thirdparty/rmlui/Source/Core/EventInstancerDefault.cpp) | 25 |  |
| [`PropertyParserColorStopList.h`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserColorStopList.h) | 25 | A property parser that parses color stop lists, particularly for gradients. |
| [`XMLNodeHandlerDefault.h`](../../engine/thirdparty/rmlui/Source/Core/XMLNodeHandlerDefault.h) | 25 | Element Node handler that creates elements |
| [`dpi_support.cpp`](../../engine/native/src/dpi_support.cpp) | 24 |  |
| [`ascii2mpw.py`](../../engine/thirdparty/freetype/builds/mac/ascii2mpw.py) | 24 |  |
| [`XMLNodeHandlerTabSet.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/XMLNodeHandlerTabSet.h) | 24 | XML node handler for processing the tabset tags. |
| [`XMLNodeHandlerTextArea.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/XMLNodeHandlerTextArea.h) | 24 | Node handler that processes the contents of the textarea tag. |
| [`FontTypes.h`](../../engine/thirdparty/rmlui/Source/Core/FontEngineDefault/FontTypes.h) | 24 | _symbols:_ FaceVariation |
| [`PropertyParserKeyword.h`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserKeyword.h) | 24 | A property parser that validates a value is part of a specified list of keywords. |
| [`PropertyParserRatio.h`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserRatio.h) | 24 | A property parser that parses an ratio in the format of x/y, like 16/9. |
| [`PropertyParserString.h`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserString.h) | 24 | A passthrough property parser that parses a string. |
| [`XMLNodeHandlerBody.h`](../../engine/thirdparty/rmlui/Source/Core/XMLNodeHandlerBody.h) | 24 | Element Node handler that processes the HEAD tag |
| [`XMLNodeHandlerHead.h`](../../engine/thirdparty/rmlui/Source/Core/XMLNodeHandlerHead.h) | 24 | Element Node handler that processes the HEAD tag |
| [`XMLNodeHandlerTemplate.h`](../../engine/thirdparty/rmlui/Source/Core/XMLNodeHandlerTemplate.h) | 24 | Element Node handler that processes the custom template tags |
| [`test_d3d12_adapter_selection.cpp`](../../engine/native/tests/test_d3d12_adapter_selection.cpp) | 23 |  |
| [`WidgetTextInputMultiLine.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/WidgetTextInputMultiLine.h) | 23 | A specialisation of the text input widget for multi-line text fields. |
| [`field_slice.h`](../../engine/native/include/native/field_slice.h) | 22 | _symbols:_ FieldSliceResult |
| [`app_util.h`](../../engine/native/src/app/app_util.h) | 22 | app/app_util.h — small portable string/format helpers shared across the split native_app translation units. |
| [`FontMetrics.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/FontMetrics.h) | 22 | _symbols:_ FontMetrics |
| [`ScriptInterface.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/ScriptInterface.h) | 22 | Base class for all objects that hold a scriptable object. |
| [`WidgetTextInputSingleLine.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/WidgetTextInputSingleLine.h) | 22 | A specialisation of the text input widget for single-line input fields. |
| [`Profiling.cpp`](../../engine/thirdparty/rmlui/Source/Core/Profiling.cpp) | 21 |  |
| [`PropertyParserKeyword.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserKeyword.cpp) | 21 |  |
| [`run_config.h`](../../engine/native/include/native/host/run_config.h) | 20 | host/run_config.h — scale-common run knobs. |
| [`snapshot_publisher.h`](../../engine/native/include/native/snapshot_publisher.h) | 20 | _symbols:_ SnapshotPublisher |
| [`ATARI.H`](../../engine/thirdparty/freetype/builds/atari/ATARI.H) | 20 | too many unevaluated variables in gxvalid |
| [`XMLNodeHandlerSelect.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/XMLNodeHandlerSelect.h) | 20 | XML node handler for processing the select and option tags. |
| [`PropertyParserFilter.h`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserFilter.h) | 20 | A property parser for the filter property. |
| [`PropertyParserFontEffect.h`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserFontEffect.h) | 20 | A property parser for the font-effect property. |
| [`WidgetTextInputMultiLine.cpp`](../../engine/thirdparty/rmlui/Source/Core/Elements/WidgetTextInputMultiLine.cpp) | 19 |  |
| [`WidgetTextInputSingleLinePassword.h`](../../engine/thirdparty/rmlui/Source/Core/Elements/WidgetTextInputSingleLinePassword.h) | 19 | _symbols:_ WidgetTextInputSingleLinePassword |
| [`LayoutEngine.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/LayoutEngine.h) | 19 | See the CSS glossary for terms used in the layout engine: https://www.w3.org/TR/css-display-3/#glossary |
| [`cli_options.h`](../../engine/native/include/native/cli_options.h) | 18 | _symbols:_ NativeDesktopCli |
| [`ui_snapshot_builder.h`](../../engine/native/include/native/ui_snapshot_builder.h) | 18 | _symbols:_ RenderBridge |
| [`snapshot_publisher.cpp`](../../engine/native/src/snapshot_publisher.cpp) | 18 |  |
| [`TextShapingContext.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/TextShapingContext.h) | 18 | Data extracted from the properties of an element to help provide context for text shaping and spacing. |
| [`Utilities.h`](../../engine/thirdparty/rmlui/Include/RmlUi/Core/Utilities.h) | 17 |  |
| [`Clock.h`](../../engine/thirdparty/rmlui/Source/Core/Clock.h) | 17 | RmlUi's Interface to Time. |
| [`LayoutPools.h`](../../engine/thirdparty/rmlui/Source/Core/Layout/LayoutPools.h) | 17 |  |
| [`LogDefault.h`](../../engine/thirdparty/rmlui/Source/Core/LogDefault.h) | 17 | Provides a platform-dependent default implementation for message logging. |
| [`PropertyParserString.cpp`](../../engine/thirdparty/rmlui/Source/Core/PropertyParserString.cpp) | 17 | parameters |
| [`Clock.cpp`](../../engine/thirdparty/rmlui/Source/Core/Clock.cpp) | 16 |  |
| [`Filter.cpp`](../../engine/thirdparty/rmlui/Source/Core/Filter.cpp) | 16 | element |
| [`dpi_support.h`](../../engine/native/include/native/dpi_support.h) | 13 |  |
| [`DecoratorUtilities.h`](../../engine/thirdparty/rmlui/Source/Core/DecoratorUtilities.h) | 13 |  |
| [`inffast.h`](../../engine/thirdparty/freetype/src/gzip/inffast.h) | 11 |  |
| [`Traits.cpp`](../../engine/thirdparty/rmlui/Source/Core/Traits.cpp) | 11 |  |
| [`run_app.h`](../../engine/native/src/app/run_app.h) | 10 |  |
| [`ContextInstancer.cpp`](../../engine/thirdparty/rmlui/Source/Core/ContextInstancer.cpp) | 7 |  |
| [`EventInstancer.cpp`](../../engine/thirdparty/rmlui/Source/Core/EventInstancer.cpp) | 7 |  |
| [`EventListenerInstancer.cpp`](../../engine/thirdparty/rmlui/Source/Core/EventListenerInstancer.cpp) | 7 |  |
| [`XMLNodeHandler.cpp`](../../engine/thirdparty/rmlui/Source/Core/XMLNodeHandler.cpp) | 7 |  |
| [`precompiled.h`](../../engine/thirdparty/rmlui/Source/Core/precompiled.h) | 3 |  |

### `web/js-toplevel`  (51 files, 74,989 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`three.module.js`](../../engine/web/js/vendor/three/build/three.module.js) | 54155 | _symbols:_ EventDispatcher, Vector2, Matrix3, ImageUtils |
| [`ws-bridge.js`](../../engine/web/js/ws-bridge.js) | 2659 | WebSocket Bridge — connects web dashboard to native GPU engine. |
| [`app.js`](../../engine/web/js/app.js) | 1744 | @file app.js @brief FTD Web Dashboard — Main Application Controller [EXTENDED] Initializes all subsystems, manages the frame loop, and wires up UI controls to the simulation bridge. |
| [`OrbitControls.js`](../../engine/web/js/vendor/three/examples/jsm/controls/OrbitControls.js) | 1523 | _symbols:_ OrbitControls |
| [`telemetry-hub.js`](../../engine/web/js/telemetry-hub.js) | 1294 | TelemetryHub — single source of truth for all FTD simulation telemetry. |
| [`ConvexHull.js`](../../engine/web/js/vendor/three/examples/jsm/math/ConvexHull.js) | 1271 | Ported from: https://github.com/maurizzzio/quickhull3d/ by Mauricio Poppe (https://github.com/maurizzzio) |
| [`fieldlines.js`](../../engine/web/js/fieldlines.js) | 1024 | ── fieldlines.js ── Streamline computation for 3D field visualization ── RK4 integration through sampled vector fields. |
| [`particle-catalog.js`](../../engine/web/js/particle-catalog.js) | 691 | Particle Catalog: Standard Model particles with FTD context. |
| [`cosmic-renderer.js`](../../engine/web/js/cosmic-renderer.js) | 681 | CosmicRenderer: Scale 5 — cinematic deep-space rendering Design goals: - Stars: soft glow sprites with diffraction cross, blackbody color, size ~ luminosity - Gas: large volumetric nebula sprites,... |
| [`meta-unit.js`](../../engine/web/js/meta-unit.js) | 678 | ── meta-unit.js ── 3x3x3 existential unit lattice visualization ── Renders the 27-site Moore neighborhood as togglable geometric layers: shells (center, octahedron, cuboctahedron, cube), wireframe... |
| [`molecules.js`](../../engine/web/js/molecules.js) | 605 | Molecular Library — 25 molecules for Scale 2 (AtomEngine). |
| [`constants.js`](../../engine/web/js/constants.js) | 595 | @file constants.js @brief FTD Constants — single source of truth for the web dashboard. |
| [`physics-harness.js`](../../engine/web/js/physics/physics-harness.js) | 535 | PhysicsHarness — single canonical surface for reading and writing Scale-0 lattice physics state. |
| [`RGBELoader.js`](../../engine/web/js/vendor/three/examples/jsm/loaders/RGBELoader.js) | 450 | default error routine. |
| [`planetary-renderer.js`](../../engine/web/js/planetary-renderer.js) | 432 | F-7: build one of the two shared shader-program templates. |
| [`fields.js`](../../engine/web/js/fields.js) | 428 | Force Field Sampling & Visualization Samples Coulomb/ionic potential and force vectors on a 2D grid (XZ plane) for rendering as a heatmap + arrow overlay. |
| [`units.js`](../../engine/web/js/units.js) | 421 | FTD Unit Conversion Layer Central module for converting raw simulation values to human-readable strings with proper physical unit labels. |
| [`UnrealBloomPass.js`](../../engine/web/js/vendor/three/examples/jsm/postprocessing/UnrealBloomPass.js) | 415 | UnrealBloomPass is inspired by the bloom pass of Unreal Engine. |
| [`inspector.js`](../../engine/web/js/inspector.js) | 412 | Inspector Panel — click-to-inspect particle properties. |
| [`aggregation-bridge.js`](../../engine/web/js/aggregation-bridge.js) | 371 | Aggregation Bridge Module — Appendix A of the FTD project (2026). |
| [`meta-pedagogy.js`](../../engine/web/js/meta-pedagogy.js) | 356 | Meta Pedagogy — Interactive exploration of the 3³ Existential Unit. |
| [`lattice-synth.js`](../../engine/web/js/audio/lattice-synth.js) | 353 | @file engine/web/js/audio/lattice-synth.js @purpose Connects FTD wave telemetry into the Web Audio API to hear the lattice. |
| [`ontic-observatory.js`](../../engine/web/js/ontic-observatory.js) | 335 | Ontic Observatory — Makes the Ontic Incompleteness narrative visible. |
| [`orbitals.js`](../../engine/web/js/orbitals.js) | 325 | Electron Orbital Cloud Generator + Nuclear Structure VISUALIZATION ONLY (not a physics derivation — FTD-0270): generates electron probability clouds by rejection-sampling HYDROGENIC wavefunctions (... |
| [`quantum-chemistry.js`](../../engine/web/js/orbitals/quantum-chemistry.js) | 247 | quantum-chemistry.js — pure QM helpers extracted from orbitals.js Aufbau filling, configuration exceptions, Slater's shielding rules, real spherical harmonic angular probabilities, and rejection-sa... |
| [`EffectComposer.js`](../../engine/web/js/vendor/three/examples/jsm/postprocessing/EffectComposer.js) | 231 | _symbols:_ EffectComposer |
| [`atomic-props.js`](../../engine/web/js/atomic-props.js) | 228 | K |
| [`lifecycle.js`](../../engine/web/js/lifecycle.js) | 207 | Unified Lifecycle Controller for FTD Web Frontend ──────────────────────────────────────────────────────────────────── Base class providing robust, automated resource reclamation. |
| [`atomic-energy.js`](../../engine/web/js/atomic-energy.js) | 204 | Atomic Energy Calculator Computes physical atomic energies for all 118 elements: - Nuclear binding energy (Bethe-Weizsäcker semi-empirical mass formula) - Total rest mass energy (protons + neutrons... |
| [`charts.js`](../../engine/web/js/charts.js) | 189 | Canvas 2D Time-Series Charts — ring-buffered, auto-scaling. |
| [`backgrounds.js`](../../engine/web/js/backgrounds.js) | 178 | FTD Environment Backgrounds — registry + BackgroundManager. |
| [`elements.js`](../../engine/web/js/elements.js) | 175 | Periodic Table Data — all 118 elements. |
| [`zoo.js`](../../engine/web/js/zoo.js) | 168 | Particle Zoo — interactive table of all SM particles with FTD data. |
| [`spectroscopy.js`](../../engine/web/js/spectroscopy.js) | 156 | Spectroscopy Module — Hydrogen energy levels and spectral series. |
| [`meta-unit-geometry.js`](../../engine/web/js/meta-unit-geometry.js) | 134 | ── meta-unit-geometry.js ── Pure geometry helpers for MetaUnit ── Extracted from meta-unit.js: sphere/wireframe/axis/mirror factories and edge-finding utilities. |
| [`shaders.js`](../../engine/web/js/cosmic/shaders.js) | 114 | cosmic/shaders.js — GLSL shader sources + blackbody color helper. |
| [`MaskPass.js`](../../engine/web/js/vendor/three/examples/jsm/postprocessing/MaskPass.js) | 104 | , deltaTime, maskActive |
| [`RenderPass.js`](../../engine/web/js/vendor/three/examples/jsm/postprocessing/RenderPass.js) | 99 | , deltaTime, maskActive |
| [`Pass.js`](../../engine/web/js/vendor/three/examples/jsm/postprocessing/Pass.js) | 95 | width, height |
| [`nuclear-cloud.js`](../../engine/web/js/orbitals/nuclear-cloud.js) | 87 | nuclear-cloud.js — nuclear structure point cloud generation. |
| [`bridge-boot.js`](../../engine/web/js/app-wire/bridge-boot.js) | 83 | app-wire/bridge-boot.js — boot-time bridge probe (native → WASM). |
| [`sprites.js`](../../engine/web/js/cosmic/sprites.js) | 77 | cosmic/sprites.js — procedural canvas-texture factories for CosmicRenderer. |
| [`ShaderPass.js`](../../engine/web/js/vendor/three/examples/jsm/postprocessing/ShaderPass.js) | 77 | , deltaTime, maskActive |
| [`keyboard.js`](../../engine/web/js/app-wire/keyboard.js) | 66 | app-wire/keyboard.js — keyboard-shortcut handler for the FTD dashboard. |
| [`LuminosityHighPassShader.js`](../../engine/web/js/vendor/three/examples/jsm/shaders/LuminosityHighPassShader.js) | 64 | Luminosity http://en.wikipedia.org/wiki/Luminosity |
| [`dom-utils.js`](../../engine/web/js/dom-utils.js) | 57 | DOM Utilities — shared helpers for DOM access patterns that appear in multiple unrelated modules (diagnostics, pe-telemetry, charts, ...). |
| [`ConvexGeometry.js`](../../engine/web/js/vendor/three/examples/jsm/geometries/ConvexGeometry.js) | 53 | _symbols:_ ConvexGeometry |
| [`CopyShader.js`](../../engine/web/js/vendor/three/examples/jsm/shaders/CopyShader.js) | 45 | Full-screen textured quad shader |
| [`index.js`](../../engine/web/js/physics/index.js) | 39 | Physics module entry point. |
| [`bridge-init.js`](../../engine/web/js/bridge-init.js) | 32 | @file engine/web/js/bridge-init.js @purpose Bridge barrel + capability-getter installer. |
| [`status.js`](../../engine/web/js/app-wire/status.js) | 27 | app-wire/status.js — toast + loading-bar helpers for the dashboard. |

### `src/core`  (196 files, 66,956 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`connected_moore_block_action.cpp`](../../engine/src/eft/connected_moore_block_action.cpp) | 2572 | _symbols:_ PreparedForwardFields, PreparedReverseFields, Candidate, RootResult |
| [`render_bridge.cpp`](../../engine/src/render_bridge.cpp) | 1294 | Logic-First FTD Engine (v2.0) Built from axioms: {3D lattice, ternary states, flux field, local causality} Six rules, nothing else: 1. |
| [`ws_server_commands.cpp`](../../engine/src/ws_server_commands.cpp) | 899 | @file ws_server_commands.cpp @brief Resource policy, transactional bridge changes, and command dispatch. |
| [`closed_neutral_trimer_pair.cpp`](../../engine/src/eft/closed_neutral_trimer_pair.cpp) | 841 | _symbols:_ PreparedForwardFields, Candidate, RootResult |
| [`momentum_transport_current.cpp`](../../engine/src/eft/momentum_transport_current.cpp) | 840 | Sec 2.2 R+ selection (frozen with this implementation; Banned move B5): pair (r,a,b) with (-r,b,a) and keep the member whose key (r_x,r_y,r_z,a,b) is lexicographically greater. |
| [`implicit_atomic_face_action.cpp`](../../engine/src/eft/implicit_atomic_face_action.cpp) | 826 | _symbols:_ AggregateCurrent, EndpointDerivatives |
| [`coupled_matched_face_transaction.cpp`](../../engine/src/eft/coupled_matched_face_transaction.cpp) | 810 | _symbols:_ Candidate, RootResult, PreparedForward |
| [`constituent_complete_charged_trimer.cpp`](../../engine/src/eft/constituent_complete_charged_trimer.cpp) | 796 | _symbols:_ PreparedForwardFields, Candidate, RootResult |
| [`ten_source_distance_distribution_lp.cpp`](../../engine/src/eft/ten_source_distance_distribution_lp.cpp) | 793 | _symbols:_ CompensatedSum, Index3, Orbit, Scheme |
| [`minimal_moore_compatibility_coat.cpp`](../../engine/src/eft/minimal_moore_compatibility_coat.cpp) | 759 | _symbols:_ Path |
| [`common_relative_connection_gearbox.cpp`](../../engine/src/eft/common_relative_connection_gearbox.cpp) | 749 | _symbols:_ ChartCoordinate |
| [`removal_time_pulse_bound.cpp`](../../engine/src/eft/removal_time_pulse_bound.cpp) | 744 |  |
| [`ten_source_shared_m_coherence.cpp`](../../engine/src/eft/ten_source_shared_m_coherence.cpp) | 743 | _symbols:_ BigInt, Index3, ModeOrbit, CompensatedSum |
| [`site_ontic_atomic_reciprocal_hop.cpp`](../../engine/src/eft/site_ontic_atomic_reciprocal_hop.cpp) | 709 | _symbols:_ Candidate, NewtonResult |
| [`ten_source_pair_distance_capacity.cpp`](../../engine/src/eft/ten_source_pair_distance_capacity.cpp) | 703 | _symbols:_ Index3, ModeOrbit, CompensatedSum |
| [`particle_engine.cpp`](../../engine/src/particle_engine.cpp) | 693 | ParticleEngine: Scale 1 simulation Phase 7: Lattice-free engine with continuous positions and analytical forces. |
| [`native_field_discrete_action.cpp`](../../engine/src/eft/native_field_discrete_action.cpp) | 681 | _symbols:_ PeriodicFields |
| [`ws_server_telemetry.cpp`](../../engine/src/ws_server_telemetry.cpp) | 669 | @file ws_server_telemetry.cpp @brief Telemetry, inspection, and scalar JSON serialization. |
| [`state_only_matter_field_observer.cpp`](../../engine/src/eft/state_only_matter_field_observer.cpp) | 667 |  |
| [`noncompact_face_cohomology.cpp`](../../engine/src/eft/noncompact_face_cohomology.cpp) | 636 | _symbols:_ PlaneFlux |
| [`vtk_export.cpp`](../../engine/src/vtk_export.cpp) | 636 |  |
| [`backend.cpp`](../../engine/src/backend.cpp) | 631 | @file backend.cpp @brief Backend implementations — CpuBackend + GpuBackend. |
| [`matched_gauss_transport.cpp`](../../engine/src/eft/matched_gauss_transport.cpp) | 612 |  |
| [`native_hodge_reciprocity.cpp`](../../engine/src/eft/native_hodge_reciprocity.cpp) | 590 | _symbols:_ PolynomialField |
| [`quadratic_coat_orbit_gather.cpp`](../../engine/src/eft/quadratic_coat_orbit_gather.cpp) | 587 |  |
| [`common_moore_worldline_action.cpp`](../../engine/src/eft/common_moore_worldline_action.cpp) | 568 |  |
| [`genesis_natural_extension.cpp`](../../engine/src/eft/genesis_natural_extension.cpp) | 564 | _symbols:_ LocalGeometry |
| [`transmutation_phases.cpp`](../../engine/src/transmutation_phases.cpp) | 554 | Transmutation phases — implementation. |
| [`wilson_dirac.cpp`](../../engine/src/wilson_dirac.cpp) | 544 | Wilson-Dirac CPU implementation -- Phase II.2-A. |
| [`collective_source_history_bound.cpp`](../../engine/src/eft/collective_source_history_bound.cpp) | 533 |  |
| [`quadratic_coat_face_current.cpp`](../../engine/src/eft/quadratic_coat_face_current.cpp) | 523 |  |
| [`native_hodge_energy_continuity.cpp`](../../engine/src/eft/native_hodge_energy_continuity.cpp) | 517 | _symbols:_ Fixture |
| [`spacetime_worldline_coupling.cpp`](../../engine/src/eft/spacetime_worldline_coupling.cpp) | 516 |  |
| [`ignition_cut_support_ablation.cpp`](../../engine/src/eft/ignition_cut_support_ablation.cpp) | 515 |  |
| [`axial_face_hop_reciprocity.cpp`](../../engine/src/eft/axial_face_hop_reciprocity.cpp) | 513 | _symbols:_ Endpoint, Trial |
| [`quadratic_coat_composite_peierls.cpp`](../../engine/src/eft/quadratic_coat_composite_peierls.cpp) | 507 | _symbols:_ Deposit, LongitudinalField, Spectrum, CachedPoint |
| [`ws_server_runtime.cpp`](../../engine/src/ws_server_runtime.cpp) | 507 | @file ws_server_runtime.cpp @brief Socket readiness, client session ordering, and server lifecycle. |
| [`cosmic_engine.cpp`](../../engine/src/cosmic_engine.cpp) | 505 | CosmicEngine: Scale 5 simulation — core TU. |
| [`gauss_record_canonical_reduction.cpp`](../../engine/src/eft/gauss_record_canonical_reduction.cpp) | 502 | _symbols:_ MeanZeroSolve |
| [`native_gauss_monopole_dichotomy.cpp`](../../engine/src/eft/native_gauss_monopole_dichotomy.cpp) | 479 | _symbols:_ Site, Profile |
| [`momentum_face_balance.cpp`](../../engine/src/eft/momentum_face_balance.cpp) | 475 |  |
| [`symmetric_chord_moore_action.cpp`](../../engine/src/eft/symmetric_chord_moore_action.cpp) | 471 | _symbols:_ ChordArm, PeierlsSpectrum |
| [`ontic_audit.cpp`](../../engine/src/ontic_audit.cpp) | 471 |  |
| [`quadratic_coat_discrete_gradient_transaction.cpp`](../../engine/src/eft/quadratic_coat_discrete_gradient_transaction.cpp) | 460 | _symbols:_ PreparedFields, Candidate, RootResult |
| [`native_telemetry_scheduler.cpp`](../../engine/src/native_telemetry_scheduler.cpp) | 446 |  |
| [`native_motion_reaction_front.cpp`](../../engine/src/eft/native_motion_reaction_front.cpp) | 434 | _symbols:_ RestArm, StaleArm |
| [`strong_stress_energy.cpp`](../../engine/src/strong_stress_energy.cpp) | 434 | FTD-0406 owner-authorized strong stress-energy contract. |
| [`paired_field_response.cpp`](../../engine/src/eft/paired_field_response.cpp) | 423 |  |
| [`genesis_cubic_canonical_form.cpp`](../../engine/src/eft/genesis_cubic_canonical_form.cpp) | 421 |  |
| [`full_surface_source_obstruction.cpp`](../../engine/src/eft/full_surface_source_obstruction.cpp) | 419 | _symbols:_ Site, Profile |
| [`ternary_collision_vertex.cpp`](../../engine/src/eft/ternary_collision_vertex.cpp) | 415 | _symbols:_ PhasePoint |
| [`contact_quotient_horizon.cpp`](../../engine/src/eft/contact_quotient_horizon.cpp) | 410 | _symbols:_ RawCarrier |
| [`quadratic_coat_matter_work.cpp`](../../engine/src/eft/quadratic_coat_matter_work.cpp) | 406 | _symbols:_ FaceSample, ScalarSample, DirectEndpointAction |
| [`quadratic_coat_spacetime_action.cpp`](../../engine/src/eft/quadratic_coat_spacetime_action.cpp) | 404 |  |
| [`discrete_legendre_worldline.cpp`](../../engine/src/eft/discrete_legendre_worldline.cpp) | 402 | _symbols:_ Dual6 |
| [`dual_cell_continuity.cpp`](../../engine/src/eft/dual_cell_continuity.cpp) | 396 |  |
| [`native_active_mode_backreaction.cpp`](../../engine/src/eft/native_active_mode_backreaction.cpp) | 393 | _symbols:_ ActiveArm, BallisticArm |
| [`accelerated_coat_spacetime_current.cpp`](../../engine/src/eft/accelerated_coat_spacetime_current.cpp) | 390 | _symbols:_ PathSample |
| [`genesis_reservoir_dilation.cpp`](../../engine/src/eft/genesis_reservoir_dilation.cpp) | 376 |  |
| [`two_slab_variational_force.cpp`](../../engine/src/eft/two_slab_variational_force.cpp) | 375 | _symbols:_ Dual3 |
| [`constructors_bulk_matter.cpp`](../../engine/src/constructors/constructors_bulk_matter.cpp) | 370 | constructors_bulk_matter.cpp Covers source lines 736-1078 of the pre-split constructors.cpp: Level 4 composites — pion, proton, neutron Level 5 atoms/mol — hydrogen, helium, h2_molecule Level 6 gau... |
| [`endogenous_reaction_carrier_bound.cpp`](../../engine/src/eft/endogenous_reaction_carrier_bound.cpp) | 370 | _symbols:_ LiveArm |
| [`removal_time_orbit_coherence.cpp`](../../engine/src/eft/removal_time_orbit_coherence.cpp) | 370 | _symbols:_ Index3, ModeOrbit, CompensatedSum |
| [`symmetric_diagonal_coupled_endpoint.cpp`](../../engine/src/eft/symmetric_diagonal_coupled_endpoint.cpp) | 367 | _symbols:_ Candidate, Problem |
| [`atom_engine.cpp`](../../engine/src/atom_engine.cpp) | 361 | AtomEngine: Scale 2 simulation — class lifecycle and tick orchestration. |
| [`reversible_checkerboard_gauss_preparation.cpp`](../../engine/src/eft/reversible_checkerboard_gauss_preparation.cpp) | 361 |  |
| [`injection.cpp`](../../engine/src/injection.cpp) | 358 | Injection — implementation. |
| [`poisson_solvers.cpp`](../../engine/src/poisson_solvers.cpp) | 354 | Poisson solvers — implementation. |
| [`centered_fiber_knot_transaction.cpp`](../../engine/src/eft/centered_fiber_knot_transaction.cpp) | 349 |  |
| [`ws_protocol.cpp`](../../engine/src/ws_protocol.cpp) | 348 | WebSocket framing protocol implementation. |
| [`quadratic_coat_neutral_pair_work.cpp`](../../engine/src/eft/quadratic_coat_neutral_pair_work.cpp) | 344 | _symbols:_ PoissonResult |
| [`constructors_atoms.cpp`](../../engine/src/constructors/constructors_atoms.cpp) | 342 | constructors_atoms.cpp Covers source lines 156-496 of the pre-split constructors.cpp: Level 2 field configurations (plane_wave, standing_wave, uniform_e/b, photon_pulse, electric_dipole, magnetic_d... |
| [`local_canonical_hamiltonian_parity_rail.cpp`](../../engine/src/eft/local_canonical_hamiltonian_parity_rail.cpp) | 333 |  |
| [`cosmic_scenarios.cpp`](../../engine/src/cosmic/cosmic_scenarios.cpp) | 332 | CosmicEngine scenario builders. |
| [`native_hop_dressing_obstruction.cpp`](../../engine/src/eft/native_hop_dressing_obstruction.cpp) | 327 |  |
| [`ten_source_temporal_product_capacity.cpp`](../../engine/src/eft/ten_source_temporal_product_capacity.cpp) | 324 | Recompile the frozen FTD-0596 observer under a private symbol so this verifier can reuse its exact cyclotomic association-scheme reconstruction and LP certificate checker without changing the paren... |
| [`matched_symmetry_ray_spectrum.cpp`](../../engine/src/eft/matched_symmetry_ray_spectrum.cpp) | 322 | _symbols:_ RayBins |
| [`overshoot_preserving_contact_rebase.cpp`](../../engine/src/eft/overshoot_preserving_contact_rebase.cpp) | 322 |  |
| [`passive_dressing_depinning_obstruction.cpp`](../../engine/src/eft/passive_dressing_depinning_obstruction.cpp) | 322 | _symbols:_ Threshold, PassiveFixture |
| [`transported_chart_morphology.cpp`](../../engine/src/eft/transported_chart_morphology.cpp) | 322 |  |
| [`scale_context.cpp`](../../engine/src/scale_context.cpp) | 320 | @file engine/src/scale_context.cpp @purpose Implementation of the read-only scale-context readout admissibility gate (C_scale). |
| [`constituent_stress_moment.cpp`](../../engine/src/eft/constituent_stress_moment.cpp) | 319 | _symbols:_ FourthMoment |
| [`native_ternary_dipole_phase_wedge_memory.cpp`](../../engine/src/eft/native_ternary_dipole_phase_wedge_memory.cpp) | 318 |  |
| [`contextual_actualization.cpp`](../../engine/src/eft/contextual_actualization.cpp) | 317 |  |
| [`face_current_segment.cpp`](../../engine/src/eft/face_current_segment.cpp) | 317 |  |
| [`visual_field_sample.cpp`](../../engine/src/visual_field_sample.cpp) | 307 | _symbols:_ Row |
| [`connected_reservoir_decomposition.cpp`](../../engine/src/eft/connected_reservoir_decomposition.cpp) | 306 |  |
| [`matched_regional_energy_transport.cpp`](../../engine/src/eft/matched_regional_energy_transport.cpp) | 306 |  |
| [`canonical_source_centered_gauss_gate.cpp`](../../engine/src/eft/canonical_source_centered_gauss_gate.cpp) | 305 |  |
| [`oriented_even_self_pair_rectifier.cpp`](../../engine/src/eft/oriented_even_self_pair_rectifier.cpp) | 305 |  |
| [`native_ternary_plaquette_quarter_turn.cpp`](../../engine/src/eft/native_ternary_plaquette_quarter_turn.cpp) | 302 |  |
| [`native_moving_source_pole.cpp`](../../engine/src/eft/native_moving_source_pole.cpp) | 300 |  |
| [`multicell_worldline_variation.cpp`](../../engine/src/eft/multicell_worldline_variation.cpp) | 299 | _symbols:_ OneSidedPair, BreakData, Entry |
| [`genesis_environment_feedback.cpp`](../../engine/src/eft/genesis_environment_feedback.cpp) | 297 |  |
| [`csv_export.cpp`](../../engine/src/csv_export.cpp) | 295 |  |
| [`finite_rigid_moore_carrier_obstruction.cpp`](../../engine/src/eft/finite_rigid_moore_carrier_obstruction.cpp) | 293 | _symbols:_ Constituent, Spectrum, BinomialSample |
| [`axial_contact_longitudinal_work.cpp`](../../engine/src/eft/axial_contact_longitudinal_work.cpp) | 287 | _symbols:_ DepositResult |
| [`hard_contact_corner_action.cpp`](../../engine/src/eft/hard_contact_corner_action.cpp) | 287 |  |
| [`boundary_chart_capacity.cpp`](../../engine/src/eft/boundary_chart_capacity.cpp) | 284 |  |
| [`continuous_translation_locality.cpp`](../../engine/src/eft/continuous_translation_locality.cpp) | 284 | _symbols:_ InverseResult |
| [`implicit_atomic_endpoint_solve.cpp`](../../engine/src/eft/implicit_atomic_endpoint_solve.cpp) | 283 |  |
| [`scale_bridge.cpp`](../../engine/src/scale_bridge.cpp) | 283 | Scale Bridge: coarsen/refine between Scale 0 (voxels) and Scale 1 (particles) Phase 7 Stage 3. |
| [`external_drive_radiation.cpp`](../../engine/src/eft/external_drive_radiation.cpp) | 281 |  |
| [`hop_source_multipole_hierarchy.cpp`](../../engine/src/eft/hop_source_multipole_hierarchy.cpp) | 281 | _symbols:_ Profile |
| [`orientation_gauss_independence.cpp`](../../engine/src/eft/orientation_gauss_independence.cpp) | 281 |  |
| [`finite_port_gauss_battery.cpp`](../../engine/src/eft/finite_port_gauss_battery.cpp) | 275 |  |
| [`matched_contact_energy_obstruction.cpp`](../../engine/src/eft/matched_contact_energy_obstruction.cpp) | 275 | _symbols:_ EmbeddedDeposit |
| [`genesis_minimal_bath.cpp`](../../engine/src/eft/genesis_minimal_bath.cpp) | 274 |  |
| [`quartic_relative_carry_gearbox.cpp`](../../engine/src/eft/quartic_relative_carry_gearbox.cpp) | 274 | _symbols:_ ChartCoordinate |
| [`constituent_relative_collision.cpp`](../../engine/src/eft/constituent_relative_collision.cpp) | 273 |  |
| [`ws_server_binary.cpp`](../../engine/src/ws_server_binary.cpp) | 272 | @file ws_server_binary.cpp @brief Bounded binary visualization payload encoders. |
| [`blocking.cpp`](../../engine/src/eft/blocking.cpp) | 270 | blocking.cpp — Phase 2A of the EFT Recovery Program. |
| [`configuration_space_carrier.cpp`](../../engine/src/eft/configuration_space_carrier.cpp) | 268 |  |
| [`genesis_action_obstruction.cpp`](../../engine/src/eft/genesis_action_obstruction.cpp) | 264 |  |
| [`signal_acknowledged_two_stroke_reset.cpp`](../../engine/src/eft/signal_acknowledged_two_stroke_reset.cpp) | 264 |  |
| [`diagnostics_compute.cpp`](../../engine/src/diagnostics_compute.cpp) | 257 | Diagnostics — implementation. |
| [`autonomous_phase_parity_source_reaction.cpp`](../../engine/src/eft/autonomous_phase_parity_source_reaction.cpp) | 257 |  |
| [`edge_plane_one_sided_variation.cpp`](../../engine/src/eft/edge_plane_one_sided_variation.cpp) | 255 |  |
| [`scenarios.cpp`](../../engine/src/scenarios.cpp) | 251 | ========================================================================== engine/src/scenarios.cpp Thin router + shared RNG for the Scale-0 scenario library. |
| [`reciprocal_carry_reservoir.cpp`](../../engine/src/eft/reciprocal_carry_reservoir.cpp) | 245 | _symbols:_ WrappedValue |
| [`spline_poynting_momentum.cpp`](../../engine/src/eft/spline_poynting_momentum.cpp) | 245 | _symbols:_ ShiftWeight |
| [`boundary_collision_resolution.cpp`](../../engine/src/eft/boundary_collision_resolution.cpp) | 244 |  |
| [`constructors_molecules.cpp`](../../engine/src/constructors/constructors_molecules.cpp) | 241 | constructors_molecules.cpp Covers source lines 499-733 of the pre-split constructors.cpp: Level 3 elementary particles (electron, positron, neutrino, quark, antiquark). |
| [`batched_regional_energy_profile.cpp`](../../engine/src/eft/batched_regional_energy_profile.cpp) | 239 | _symbols:_ EnergyProfile |
| [`coupled_quartic_clock_field.cpp`](../../engine/src/eft/coupled_quartic_clock_field.cpp) | 239 |  |
| [`localized_basin_observer.cpp`](../../engine/src/eft/localized_basin_observer.cpp) | 239 |  |
| [`cubic_reaction_vector_source_transport.cpp`](../../engine/src/eft/cubic_reaction_vector_source_transport.cpp) | 238 |  |
| [`extended_source_peierls_scaling.cpp`](../../engine/src/eft/extended_source_peierls_scaling.cpp) | 238 | _symbols:_ Accumulator |
| [`self_pair_connection_critical_gearbox.cpp`](../../engine/src/eft/self_pair_connection_critical_gearbox.cpp) | 238 |  |
| [`collective_reaction_triplet_inertia.cpp`](../../engine/src/eft/collective_reaction_triplet_inertia.cpp) | 235 |  |
| [`contact_quotient_coupling_scope.cpp`](../../engine/src/eft/contact_quotient_coupling_scope.cpp) | 233 | _symbols:_ NativeCouplingProbe |
| [`derived_interaction_graph.cpp`](../../engine/src/eft/derived_interaction_graph.cpp) | 231 |  |
| [`staggered_current_split_compatibility.cpp`](../../engine/src/eft/staggered_current_split_compatibility.cpp) | 231 | _symbols:_ SplitFields |
| [`hamiltonian_ternary_quarter_turn_actuator.cpp`](../../engine/src/eft/hamiltonian_ternary_quarter_turn_actuator.cpp) | 229 |  |
| [`phase_referenced_action_rail.cpp`](../../engine/src/eft/phase_referenced_action_rail.cpp) | 225 |  |
| [`diagonal_endpoint_action_domain.cpp`](../../engine/src/eft/diagonal_endpoint_action_domain.cpp) | 224 | A nonzero point immediately before start, guaranteed to stay in the closed unit cell containing start for the registered non-boundary starts. |
| [`matched_midpoint_poynting.cpp`](../../engine/src/eft/matched_midpoint_poynting.cpp) | 224 |  |
| [`clock_gated_hamiltonian_exchange.cpp`](../../engine/src/eft/clock_gated_hamiltonian_exchange.cpp) | 222 |  |
| [`reversible_ternary_signal_uncomputation.cpp`](../../engine/src/eft/reversible_ternary_signal_uncomputation.cpp) | 222 |  |
| [`ternary_block_bipole_peierls.cpp`](../../engine/src/eft/ternary_block_bipole_peierls.cpp) | 217 | _symbols:_ CompensatedSum, CompensatedComplexSum |
| [`production_same_sign_bounce.cpp`](../../engine/src/eft/production_same_sign_bounce.cpp) | 210 |  |
| [`catalytic_phase_reference.cpp`](../../engine/src/eft/catalytic_phase_reference.cpp) | 207 | _symbols:_ PhaseFrame |
| [`flux_wave_velocity_markov_carrier.cpp`](../../engine/src/eft/flux_wave_velocity_markov_carrier.cpp) | 197 |  |
| [`native_event_characteristics.cpp`](../../engine/src/eft/native_event_characteristics.cpp) | 195 |  |
| [`constructors_exotic.cpp`](../../engine/src/constructors/constructors_exotic.cpp) | 194 | constructors_exotic.cpp Covers source lines 1081-1245 of the pre-split constructors.cpp: Level 7 gravity/cosmology — schwarzschild, frw_patch, gravitational_wave Level 8 reference frame context — s... |
| [`knot_legendre_branch.cpp`](../../engine/src/eft/knot_legendre_branch.cpp) | 194 |  |
| [`bloch_quasimomentum_lift.cpp`](../../engine/src/eft/bloch_quasimomentum_lift.cpp) | 181 | _symbols:_ WrappedAngle |
| [`integer_bloch_transport.cpp`](../../engine/src/eft/integer_bloch_transport.cpp) | 181 |  |
| [`component_aware_radial_field_profile.cpp`](../../engine/src/eft/component_aware_radial_field_profile.cpp) | 180 |  |
| [`single_slab_connection_compatibility.cpp`](../../engine/src/eft/single_slab_connection_compatibility.cpp) | 180 |  |
| [`lagrangian.cpp`](../../engine/src/lagrangian.cpp) | 178 |  |
| [`ternary_eligibility_clutch.cpp`](../../engine/src/eft/ternary_eligibility_clutch.cpp) | 174 |  |
| [`cosmic_sph.cpp`](../../engine/src/cosmic/cosmic_sph.cpp) | 171 | CosmicEngine SPH hydrodynamics. |
| [`accelerated_worldline_energy.cpp`](../../engine/src/eft/accelerated_worldline_energy.cpp) | 170 |  |
| [`oriented_ternary_quarter_turn.cpp`](../../engine/src/eft/oriented_ternary_quarter_turn.cpp) | 168 |  |
| [`local_polarity_regularity.cpp`](../../engine/src/eft/local_polarity_regularity.cpp) | 167 |  |
| [`multibody_shape_observability.cpp`](../../engine/src/eft/multibody_shape_observability.cpp) | 162 |  |
| [`cusp_dressing_integrability.cpp`](../../engine/src/eft/cusp_dressing_integrability.cpp) | 157 |  |
| [`alternating_oriented_ternary_parity_rail.cpp`](../../engine/src/eft/alternating_oriented_ternary_parity_rail.cpp) | 156 |  |
| [`free_flux_localization.cpp`](../../engine/src/eft/free_flux_localization.cpp) | 156 |  |
| [`native_contact_active_set.cpp`](../../engine/src/eft/native_contact_active_set.cpp) | 155 |  |
| [`endpoint_schedule_underdetermination.cpp`](../../engine/src/eft/endpoint_schedule_underdetermination.cpp) | 152 | _symbols:_ IntegratedMoments |
| [`centered_trace_work.cpp`](../../engine/src/eft/centered_trace_work.cpp) | 150 |  |
| [`open_worldline_hop_selector.cpp`](../../engine/src/eft/open_worldline_hop_selector.cpp) | 149 |  |
| [`momentum_selected_worldline_matching.cpp`](../../engine/src/eft/momentum_selected_worldline_matching.cpp) | 141 |  |
| [`dual_cell_blocking.cpp`](../../engine/src/eft/dual_cell_blocking.cpp) | 138 |  |
| [`subcell_representation_quotient.cpp`](../../engine/src/eft/subcell_representation_quotient.cpp) | 134 | _symbols:_ AxisChart, AxisCharts |
| [`relative_action_transducer.cpp`](../../engine/src/eft/relative_action_transducer.cpp) | 130 |  |
| [`dressed_boost_momentum_map.cpp`](../../engine/src/eft/dressed_boost_momentum_map.cpp) | 127 |  |
| [`matched_face_current_spectrum.cpp`](../../engine/src/eft/matched_face_current_spectrum.cpp) | 124 |  |
| [`conserved_charge_basis.cpp`](../../engine/src/eft/conserved_charge_basis.cpp) | 122 | _symbols:_ Rational |
| [`worldline_current_kernel.cpp`](../../engine/src/eft/worldline_current_kernel.cpp) | 121 |  |
| [`constructors_core.cpp`](../../engine/src/constructors/constructors_core.cpp) | 119 | constructors_core.cpp Level 0 (flux/particle/wavepacket/entangled_pair) and Level 1A (octahedron/cuboctahedron/stella_octangula/moore_cell) constructors. |
| [`reciprocal_record_port.cpp`](../../engine/src/eft/reciprocal_record_port.cpp) | 117 |  |
| [`energy_ledger_compute.cpp`](../../engine/src/energy_ledger_compute.cpp) | 116 | Energy ledger computation — implementation. |
| [`subcell_polarity_shape.cpp`](../../engine/src/eft/subcell_polarity_shape.cpp) | 111 | _symbols:_ AxisWeights |
| [`dressing_fiber_ledger.cpp`](../../engine/src/eft/dressing_fiber_ledger.cpp) | 110 |  |
| [`cosmic_barnes_hut.cpp`](../../engine/src/cosmic/cosmic_barnes_hut.cpp) | 103 | CosmicEngine Barnes-Hut octree + gravity. |
| [`cosmic_gravitational_waves.cpp`](../../engine/src/cosmic/cosmic_gravitational_waves.cpp) | 99 | CosmicEngine gravitational wave emission + propagation. |
| [`ws_server_internal.h`](../../engine/src/ws_server_internal.h) | 96 | @file ws_server_internal.h @brief Private module boundary for the native WebSocket server. |
| [`canonical_subcell_section.cpp`](../../engine/src/eft/canonical_subcell_section.cpp) | 95 | _symbols:_ CanonicalAxis |
| [`_common.h`](../../engine/src/constructors/_common.h) | 88 | Internal shared helpers for the split constructors.cpp translation units. |
| [`eight_source_orbit_coherence.cpp`](../../engine/src/eft/eight_source_orbit_coherence.cpp) | 83 |  |
| [`ten_source_orbit_coherence.cpp`](../../engine/src/eft/ten_source_orbit_coherence.cpp) | 83 |  |
| [`cosmic_cosmology.cpp`](../../engine/src/cosmic/cosmic_cosmology.cpp) | 82 | CosmicEngine cosmology: Friedmann / Hubble / dark energy. |
| [`nine_source_orbit_coherence.cpp`](../../engine/src/eft/nine_source_orbit_coherence.cpp) | 81 |  |
| [`dual_cell_flow.cpp`](../../engine/src/eft/dual_cell_flow.cpp) | 77 |  |
| [`bridge_rng.cpp`](../../engine/src/bridge_rng.cpp) | 76 | @file bridge_rng.cpp @brief PIMPL'd RNG state implementation. |
| [`fixed_step_energy_scope.cpp`](../../engine/src/eft/fixed_step_energy_scope.cpp) | 76 |  |
| [`centered_knot_trace.cpp`](../../engine/src/eft/centered_knot_trace.cpp) | 74 |  |
| [`finite_memory_reversible_lift.cpp`](../../engine/src/eft/finite_memory_reversible_lift.cpp) | 74 |  |
| [`support_invariant_matter_predicate.cpp`](../../engine/src/eft/support_invariant_matter_predicate.cpp) | 74 |  |
| [`history_event_journal.cpp`](../../engine/src/eft/history_event_journal.cpp) | 72 | _symbols:_ HistoryEventJournal |
| [`qcd_one_loop_perturbative.cpp`](../../engine/src/eft/qcd_one_loop_perturbative.cpp) | 52 | qcd_one_loop_perturbative.cpp [IMPOSED] — Imported one-loop QCD running coupling from perturbative QFT. |
| [`pole_matching.cpp`](../../engine/src/eft/pole_matching.cpp) | 34 |  |
| [`ws_server.cpp`](../../engine/src/ws_server.cpp) | 12 | FTD WebSocket Server Standalone executable bridging the native engine to the web dashboard. |

### `include`  (296 files, 37,353 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`render_bridge.h`](../../engine/include/ftd/render_bridge.h) | 731 | Render-Bridge Tick Engine Implements the Scale-0 lattice tick dynamics with staged read/write loops and toggle-gated extension phases. |
| [`cosmic_engine.h`](../../engine/include/ftd/cosmic_engine.h) | 556 | CosmicEngine: Scale 5 simulation N-body + SPH cosmic simulation with Barnes-Hut octree gravity. |
| [`gpu_engine.h`](../../engine/include/ftd/gpu_engine.h) | 550 | GPU-Accelerated FTD Engine Drop-in alternative to RenderBridge that executes the tick cycle on NVIDIA GPU via CUDA. |
| [`constants.h`](../../engine/include/ftd/constants.h) | 545 | FTD Render-Bridge Constants Engine-facing interface to the ontic constants registry. |
| [`atom_engine.h`](../../engine/include/ftd/atom_engine.h) | 523 | AtomEngine: Scale 2 simulation Atoms as composite objects with inter-atomic forces: Ionic: F = -ALPHA * Q_i * Q_j / (4*pi * r²_soft) * r_hat Van der Waals: Lennard-Jones 12-6 with eps/sigma from on... |
| [`gpu_buffers.h`](../../engine/include/ftd/gpu_buffers.h) | 514 | SoA (Structure-of-Arrays) device buffers for GPU-accelerated FTD engine. |
| [`term_toggles.h`](../../engine/include/ftd/term_toggles.h) | 473 | Runtime toggles for the logic-first engine. |
| [`coupling_measurement.h`](../../engine/include/ftd/eft/coupling_measurement.h) | 440 | @file ftd/eft/coupling_measurement.h @brief Lattice-coupling measurement for the EFT Recovery Program (Phase 2B). |
| [`momentum_transport_current.h`](../../engine/include/ftd/eft/momentum_transport_current.h) | 415 | @file momentum_transport_current.h @brief Observer-only discrete momentum stress ledger T^(i) / S^(i). |
| [`matched_poisson.h`](../../engine/include/ftd/eft/matched_poisson.h) | 413 | @file ftd/eft/matched_poisson.h @brief Matched-stencil conjugate-gradient Poisson solver for EFT measurements. |
| [`particle_engine.h`](../../engine/include/ftd/particle_engine.h) | 409 | ParticleEngine: Scale 1 simulation Phase 7: Lattice-free engine with continuous positions and analytical forces. |
| [`engine_state.h`](../../engine/include/ftd/engine_state.h) | 408 | @file engine_state.h @brief Cache-friendly simulation storage anchored by authoritative ternary state. |
| [`connected_moore_block_action.h`](../../engine/include/ftd/eft/connected_moore_block_action.h) | 379 | @file connected_moore_block_action.h @brief Runtime-size Moore-local integer-carrier common action (FTD-0622). |
| [`minimum_norm_transaction_selector.h`](../../engine/include/ftd/eft/minimum_norm_transaction_selector.h) | 339 | @file minimum_norm_transaction_selector.h @brief Independent certificate for the minimum-norm zero-energy selector. |
| [`constructors.h`](../../engine/include/ftd/constructors.h) | 338 | ftd/constructors.h — lattice constructor library Named factory functions that stamp FTD theoretical entities onto a RenderBridge's voxel grid. |
| [`branch_holonomy.h`](../../engine/include/ftd/branch_holonomy.h) | 326 | branch_holonomy.h — signed difference operator and Z_2 torus branch twists. |
| [`cluster_tracker.h`](../../engine/include/ftd/cluster_tracker.h) | 315 | Cluster Tracker — Connected-component cluster identification + persistence. |
| [`native_pair_energy_recursion.h`](../../engine/include/ftd/eft/native_pair_energy_recursion.h) | 315 | @file native_pair_energy_recursion.h @brief FTD-0840 isolated signed-pair energy recursion reference. |
| [`correlations.h`](../../engine/include/ftd/correlations.h) | 313 | Correlation Function Infrastructure Physics justification: Correlation functions are the fundamental observables of field theories. |
| [`atomic_closure_context.h`](../../engine/include/ftd/atomic_closure_context.h) | 307 | Atomic closure-context diagnostics. |
| [`gauge_couplings.h`](../../engine/include/ftd/ontic/gauge_couplings.h) | 290 | ontic/gauge_couplings.h — Layers 5, 5b, 7 and simulation parameters. |
| [`cluster_genealogy.h`](../../engine/include/ftd/cluster_genealogy.h) | 282 | Cluster genealogy tracker — merge/split (fusion/fission) event detection. |
| [`render_bridge_diagnostics.h`](../../engine/include/ftd/render_bridge_diagnostics.h) | 282 | @file engine/include/ftd/render_bridge_diagnostics.h @purpose POD diagnostic structs returned by RenderBridge inspection methods. |
| [`field_operators.h`](../../engine/include/ftd/field_operators.h) | 278 | Field operators — discrete differential operators on the lattice. |
| [`lagrangian.h`](../../engine/include/ftd/lagrangian.h) | 278 | Partial discrete field/kinematic action diagnostic (6 active terms + Rayleigh dissipation; not the complete production tick) L_FTD = L_KINETIC + L_GRADIENT + L_BI + L_COUPLING + L_VELOCITY + L_GAUS... |
| [`master_quadratic.h`](../../engine/include/ftd/ontic/master_quadratic.h) | 268 | ontic/master_quadratic.h — Layers 3, 3b, 3c, 4, 4b of the ontic chain. |
| [`parallel.h`](../../engine/include/ftd/parallel.h) | 268 | ============================================================================ ftd/parallel.h — unified parallel primitives with THREE compile-time backends ==========================================... |
| [`symmetric_half_tick_transaction.h`](../../engine/include/ftd/eft/symmetric_half_tick_transaction.h) | 267 | @file symmetric_half_tick_transaction.h @brief Observer-only source-centered half-kick / drift / half-kick transaction for the written interaction L_int = +G_C sum_x s divJ. |
| [`backend.h`](../../engine/include/ftd/backend.h) | 264 | @file backend.h @brief Backend abstraction — collapses #ifdef FTD_ENABLE_CUDA proliferation. |
| [`anisotropy.h`](../../engine/include/ftd/eft/anisotropy.h) | 263 | @file ftd/eft/anisotropy.h @brief Rotational-anisotropy measurement for the EFT Recovery Program (Phase 1A). |
| [`matched_face_energy_transaction.h`](../../engine/include/ftd/eft/matched_face_energy_transaction.h) | 255 | @file matched_face_energy_transaction.h @brief Observer-only finite-current work ledger for the matched face complex. |
| [`generation_graph.h`](../../engine/include/ftd/generation_graph.h) | 253 | generation_graph.h — Γ_F(d) triangle graph + 3x3 Hermitian eigensolver. |
| [`manifestation_background.h`](../../engine/include/ftd/eft/manifestation_background.h) | 248 | @file ftd/eft/manifestation_background.h @brief Forced Poisson manifestation-injection background (Plan B, P2 protocol). |
| [`wilson_dirac.h`](../../engine/include/ftd/wilson_dirac.h) | 245 | Wilson-Dirac Matter Sector for FTD (Phase II.2 of the campaign) Pre-registration: docs/theory/10_eft_program/PREREG_PHASE_II_WILSON_DIRAC_G2.md (tag: preregister-phase-ii-wilson-dirac-g2-v1) Specif... |
| [`native_telemetry_scheduler.h`](../../engine/include/ftd/native_telemetry_scheduler.h) | 244 | @file native_telemetry_scheduler.h @brief Tick-boundary publisher for native interactive telemetry. |
| [`emergent_boundary_observer.h`](../../engine/include/ftd/eft/emergent_boundary_observer.h) | 225 | @file ftd/eft/emergent_boundary_observer.h @brief Read-only manifested-boundary and free-wave-stress observer (FTD-0474). |
| [`supported_paired_recoil_capacity.h`](../../engine/include/ftd/eft/supported_paired_recoil_capacity.h) | 222 | @file supported_paired_recoil_capacity.h @brief Paired J/W recoil minimization inside a fixed site-support mask. |
| [`color_center.h`](../../engine/include/ftd/color_center.h) | 218 | color_center.h — Z_3 color-center charges and the center projector. |
| [`paired_jw_recoil_capacity.h`](../../engine/include/ftd/eft/paired_jw_recoil_capacity.h) | 218 | @file paired_jw_recoil_capacity.h @brief Constrained energy capacity of an additive symplectic J/W impulse. |
| [`lorentz_recovery.h`](../../engine/include/ftd/eft/lorentz_recovery.h) | 216 | @file ftd/eft/lorentz_recovery.h @brief Free-flux correlator-collapse diagnostic (EFT Recovery Program, Phase 1B). |
| [`reaction_operators.h`](../../engine/include/ftd/eft/reaction_operators.h) | 215 | @file ftd/eft/reaction_operators.h @brief Reaction-sector operators (O7-O10) for the S_eff campaign (FTD-0112). |
| [`native_dynamic_polarity_response.h`](../../engine/include/ftd/eft/native_dynamic_polarity_response.h) | 213 | @file native_dynamic_polarity_response.h @brief Read-only Fourier observer for FTD-0429. |
| [`hilbert.h`](../../engine/include/ftd/hilbert.h) | 212 | Hilbert Space Construction from Complexified Flux H_FTD = L^2(Lattice, C) where psi(v) = J_x(v) + i*J_y(v) The complexified transverse flux components form a wave function. |
| [`voxel.h`](../../engine/include/ftd/voxel.h) | 210 | Per-node state for the FTD render-bridge simulation. |
| [`ward_identities.h`](../../engine/include/ftd/eft/ward_identities.h) | 209 | @file ftd/eft/ward_identities.h @brief Ward-identity diagnostics for the EFT Recovery Program (Phase 1C). |
| [`native_retarded_polarity_response.h`](../../engine/include/ftd/eft/native_retarded_polarity_response.h) | 208 | @file native_retarded_polarity_response.h @brief Batched read-only moving-source observer for FTD-0430. |
| [`operator_spectrum.h`](../../engine/include/ftd/eft/operator_spectrum.h) | 208 | @file ftd/eft/operator_spectrum.h @brief Operator-basis and scaling-dimension extraction (EFT Phase 3). |
| [`lattice.h`](../../engine/include/ftd/lattice.h) | 206 | 3D Cubic Lattice with periodic boundary conditions. |
| [`spectrum_extraction.h`](../../engine/include/ftd/spectrum_extraction.h) | 202 | Two-state spectrum extraction from a one-dimensional correlator C(τ). |
| [`ensemble.h`](../../engine/include/ftd/ensemble.h) | 200 | Ensemble Runner — Statistical mechanics over independent RenderBridge runs. |
| [`state_only_matter_field_observer.h`](../../engine/include/ftd/eft/state_only_matter_field_observer.h) | 198 | @file state_only_matter_field_observer.h @brief Instantaneous bound/characteristic field observer (FTD-0754). |
| [`knot_telemetry.h`](../../engine/include/ftd/knot_telemetry.h) | 197 | engine/include/ftd/knot_telemetry.h |
| [`tracker.h`](../../engine/include/ftd/tracker.h) | 197 | Particle Tracker — Trajectory recording using persistent particle_id. |
| [`spectral.h`](../../engine/include/ftd/spectral.h) | 195 | Spectral Analysis — FFT-based dispersion relation measurement. |
| [`barnes_hut.h`](../../engine/include/ftd/barnes_hut.h) | 193 | Universal Barnes-Hut Octree Implements an O(N log N) spatial partitioner that accurately preserves long-range interactions (1/r^2) via monopole summation (gravity, Coulomb). |
| [`reciprocal_moving_source_observer.h`](../../engine/include/ftd/eft/reciprocal_moving_source_observer.h) | 193 | @file reciprocal_moving_source_observer.h @brief Read-only matter-conditioned field morphology for FTD-0477. |
| [`wave_morphology_observer.h`](../../engine/include/ftd/eft/wave_morphology_observer.h) | 193 | @file ftd/eft/wave_morphology_observer.h @brief Read-only bound/bow/wake morphology observer (FTD-0475). |
| [`sublattice.h`](../../engine/include/ftd/sublattice.h) | 193 | Sublattice projection — SC / FCC / BCC sub-stencils of the Moore-26 neighborhood. |
| [`dynamical_flux_dressing_observer.h`](../../engine/include/ftd/eft/dynamical_flux_dressing_observer.h) | 192 | @file dynamical_flux_dressing_observer.h @brief Read-only source-centred flux morphology for FTD-0476. |
| [`lemniscate.h`](../../engine/include/ftd/ontic/lemniscate.h) | 189 | ontic/lemniscate.h — Layers -1 through 2b of the ontic chain. |
| [`canonical_source_centered_gauss_gate.h`](../../engine/include/ftd/eft/canonical_source_centered_gauss_gate.h) | 186 | @file canonical_source_centered_gauss_gate.h @brief FTD-0885/0886 canonical Gauss-layer and battery-phase boundary. |
| [`contextual_actualization.h`](../../engine/include/ftd/eft/contextual_actualization.h) | 178 | @file contextual_actualization.h @brief FTD-0825 isolated contextual-actualization reference interfaces. |
| [`matched_gauss_transport.h`](../../engine/include/ftd/eft/matched_gauss_transport.h) | 171 | @file ftd/eft/matched_gauss_transport.h @brief Projection-free oriented-face Gauss transport sidecar (FTD-0427). |
| [`scenario_meta.h`](../../engine/include/ftd/scenario_meta.h) | 169 | Scale-0 scenario descriptors for the native shell. |
| [`a1g_projector.h`](../../engine/include/ftd/a1g_projector.h) | 168 | A_{1g} projector for the 27-voxel Moore block. |
| [`fixed_j_recoil_capacity.h`](../../engine/include/ftd/eft/fixed_j_recoil_capacity.h) | 168 | @file fixed_j_recoil_capacity.h @brief Global minimum tick-energy cost of a fixed-J wave-velocity recoil. |
| [`render_bridge_phases.h`](../../engine/include/ftd/render_bridge_phases.h) | 167 | @file engine/include/ftd/render_bridge_phases.h @purpose Free-function declarations for the decomposed phase methods. |
| [`causal_kinematics.h`](../../engine/include/ftd/causal_kinematics.h) | 162 | Raw-lattice causal kinematics — FTD-0402 source of truth. |
| [`native_reaction_polarity_slow_mode.h`](../../engine/include/ftd/eft/native_reaction_polarity_slow_mode.h) | 160 | @file native_reaction_polarity_slow_mode.h @brief Read-only reaction/source mode observer for FTD-0431. |
| [`reversible_checkerboard_gauss_preparation.h`](../../engine/include/ftd/eft/reversible_checkerboard_gauss_preparation.h) | 158 | @file reversible_checkerboard_gauss_preparation.h @brief FTD-0881/0882 reversible local matched-Gauss preparation witness. |
| [`native_evaporation_hazard_observer.h`](../../engine/include/ftd/eft/native_evaporation_hazard_observer.h) | 155 | @file native_evaporation_hazard_observer.h @brief Exact pre-RNG conditional evaporation observer for FTD-0432. |
| [`common_relative_connection_gearbox.h`](../../engine/include/ftd/eft/common_relative_connection_gearbox.h) | 154 | @file common_relative_connection_gearbox.h @brief FTD-0899/0901 isolated common/relative connection witness. |
| [`blocking.h`](../../engine/include/ftd/eft/blocking.h) | 153 | @file ftd/eft/blocking.h @brief Real-space block-spin transformation (EFT Recovery Program, Phase 2A). |
| [`implicit_atomic_face_action.h`](../../engine/include/ftd/eft/implicit_atomic_face_action.h) | 153 | @file implicit_atomic_face_action.h @brief Observer-only minimal implicit face-action endpoint audit (FTD-0536). |
| [`engine_select.h`](../../engine/include/ftd/engine_select.h) | 153 | Engine selection helper. |
| [`coupled_matched_face_transaction.h`](../../engine/include/ftd/eft/coupled_matched_face_transaction.h) | 152 | @file coupled_matched_face_transaction.h @brief Observer-only coupled matter/matched-field transaction (FTD-0479). |
| [`test_telemetry.h`](../../engine/include/ftd/test_telemetry.h) | 152 | ============================================================================ ftd/test_telemetry.h — NDJSON telemetry for the FTD Test Bench runner ==================================================... |
| [`matched_face_momentum_transaction.h`](../../engine/include/ftd/eft/matched_face_momentum_transaction.h) | 151 | @file matched_face_momentum_transaction.h @brief Observer-only local translation pseudomomentum for matched fields. |
| [`cuda_matched_field_pipeline.h`](../../engine/include/ftd/eft/cuda_matched_field_pipeline.h) | 150 | @file cuda_matched_field_pipeline.h @brief CUDA accelerator for the selected matched face/edge field step. |
| [`ws_protocol.h`](../../engine/include/ftd/ws_protocol.h) | 149 | WebSocket framing protocol (RFC 6455) + minimal string-search JSON helpers. |
| [`localized_transverse_packet.h`](../../engine/include/ftd/eft/localized_transverse_packet.h) | 146 | @file localized_transverse_packet.h @brief Observer-side finite divergence-free packet and reversible wave tick. |
| [`particle_masses.h`](../../engine/include/ftd/ontic/particle_masses.h) | 144 | ontic/particle_masses.h — Layers 6, 6b, 6c of the ontic chain. |
| [`gauss_projection_ext.h`](../../engine/include/ftd/eft/gauss_projection_ext.h) | 142 | @file ftd/eft/gauss_projection_ext.h @brief High-tolerance Gauss projection for EFT measurements (post-campaign). |
| [`scale_context.h`](../../engine/include/ftd/scale_context.h) | 141 | @file engine/include/ftd/scale_context.h @purpose Read-only "scale-context readout admissibility gate" (C_scale). |
| [`native_event_characteristics.h`](../../engine/include/ftd/eft/native_event_characteristics.h) | 140 | @file native_event_characteristics.h @brief FTD-0858 isolated event-acceptance and characteristic-chart witness. |
| [`local_canonical_hamiltonian_parity_rail.h`](../../engine/include/ftd/eft/local_canonical_hamiltonian_parity_rail.h) | 138 | @file local_canonical_hamiltonian_parity_rail.h @brief FTD-0875 local canonical Hamiltonian lift of the parity rail. |
| [`ws_sha1.h`](../../engine/include/ftd/ws_sha1.h) | 138 | Minimal SHA-1 (RFC 3174) Header-only, stdlib-only. |
| [`oriented_even_self_pair_rectifier.h`](../../engine/include/ftd/eft/oriented_even_self_pair_rectifier.h) | 136 | @file oriented_even_self_pair_rectifier.h @brief FTD-0904 isolated oriented even-self-pair rectifier witness. |
| [`gauss_record_canonical_reduction.h`](../../engine/include/ftd/eft/gauss_record_canonical_reduction.h) | 135 | @file gauss_record_canonical_reduction.h @brief Matched Gauss-record canonical reduction witness (FTD-0877/0880). |
| [`paired_field_response.h`](../../engine/include/ftd/eft/paired_field_response.h) | 135 | @file paired_field_response.h @brief Observer-only moving/rest field-response algebra (FTD-0768). |
| [`constituent_complete_charged_trimer.h`](../../engine/include/ftd/eft/constituent_complete_charged_trimer.h) | 134 | @file constituent_complete_charged_trimer.h @brief Observer-only lossless charged-trimer common-action transaction (FTD-0600). |
| [`central_gauss_hop_transport.h`](../../engine/include/ftd/eft/central_gauss_hop_transport.h) | 133 | @file central_gauss_hop_transport.h @brief Observer-only realizability of one-site source transport under the production cell-centered central divergence. |
| [`autonomous_phase_parity_source_reaction.h`](../../engine/include/ftd/eft/autonomous_phase_parity_source_reaction.h) | 131 | @file autonomous_phase_parity_source_reaction.h @brief FTD-0887/0888 autonomous parity and source-reaction reference. |
| [`finite_port_gauss_battery.h`](../../engine/include/ftd/eft/finite_port_gauss_battery.h) | 130 | @file finite_port_gauss_battery.h @brief FTD-0883/0884 finite ready-port bank and positive battery witness. |
| [`native_ternary_dipole_phase_wedge_memory.h`](../../engine/include/ftd/eft/native_ternary_dipole_phase_wedge_memory.h) | 129 | @file native_ternary_dipole_phase_wedge_memory.h @brief FTD-0905/0907 isolated native-type orientation-memory analyzer. |
| [`quadratic_coat_face_current.h`](../../engine/include/ftd/eft/quadratic_coat_face_current.h) | 124 | @file quadratic_coat_face_current.h @brief Smooth positive coupling coat and exact straight face current (FTD-0541). |
| [`self_pair_connection_critical_gearbox.h`](../../engine/include/ftd/eft/self_pair_connection_critical_gearbox.h) | 123 | @file self_pair_connection_critical_gearbox.h @brief FTD-0902/0903 isolated signed-self-pair connection witness. |
| [`gpu_particle_engine.h`](../../engine/include/ftd/gpu_particle_engine.h) | 123 | GPU-accelerated ParticleEngine backend (Wave 5.4 Phase 1). |
| [`ternary_collision_vertex.h`](../../engine/include/ftd/eft/ternary_collision_vertex.h) | 120 | @file ternary_collision_vertex.h @brief Observer-only ternary collision-capacity and identical-worldline quotient analysis (FTD-0504). |
| [`gpu_atom_engine.h`](../../engine/include/ftd/gpu_atom_engine.h) | 120 | GPU-accelerated AtomEngine backend (Wave 5.3 Phase 1). |
| [`self_field_decomposition.h`](../../engine/include/ftd/eft/self_field_decomposition.h) | 119 | @file self_field_decomposition.h @brief Global matched Hodge observer for self-field locality (FTD-0488). |
| [`boundary_collision_resolution.h`](../../engine/include/ftd/eft/boundary_collision_resolution.h) | 118 | @file boundary_collision_resolution.h @brief Observer-only boundary-collision capacity/range/phase trilemma (FTD-0505). |
| [`gauss_threshold_force_obstruction.h`](../../engine/include/ftd/eft/gauss_threshold_force_obstruction.h) | 118 | @file gauss_threshold_force_obstruction.h @brief Local Gauss lower bound on compact point-force jumps (FTD-0487). |
| [`transported_chart_morphology.h`](../../engine/include/ftd/eft/transported_chart_morphology.h) | 118 | @file transported_chart_morphology.h @brief Observer-only transported field morphology (FTD-0764/0766). |
| [`closed_neutral_trimer_pair.h`](../../engine/include/ftd/eft/closed_neutral_trimer_pair.h) | 116 | @file closed_neutral_trimer_pair.h @brief Observer-only closed neutral pair of charged constituent trimers (FTD-0601). |
| [`spacetime_worldline_coupling.h`](../../engine/include/ftd/eft/spacetime_worldline_coupling.h) | 116 | @file spacetime_worldline_coupling.h @brief Exact spacetime completion of the subcell face current (FTD-0484). |
| [`csv_export.h`](../../engine/include/ftd/csv_export.h) | 115 | CSV Data Export Utility for FTD Simulations Utility for exporting simulation data to CSV files. |
| [`constituent_stress_moment.h`](../../engine/include/ftd/eft/constituent_stress_moment.h) | 115 | @file constituent_stress_moment.h @brief Observer-only minimal kinetic-stress lift for the axial current kernel (FTD-0513). |
| [`phase_referenced_action_rail.h`](../../engine/include/ftd/eft/phase_referenced_action_rail.h) | 115 | @file phase_referenced_action_rail.h @brief FTD-0862 isolated phase-referenced action export rail witness. |
| [`voxel_rng.h`](../../engine/include/ftd/voxel_rng.h) | 115 | FTD voxel-level RNG (BH-F5 / BH-F8 / BH-F9 closure, 2026-05-05). |
| [`cubic_reaction_vector_source_transport.h`](../../engine/include/ftd/eft/cubic_reaction_vector_source_transport.h) | 114 | @file cubic_reaction_vector_source_transport.h @brief FTD-0889/0890 cubic reaction-vector/source-transport reference. |
| [`signal_acknowledged_two_stroke_reset.h`](../../engine/include/ftd/eft/signal_acknowledged_two_stroke_reset.h) | 114 | @file signal_acknowledged_two_stroke_reset.h @brief FTD-0869 isolated signal-acknowledged recursive reset witness. |
| [`catalytic_phase_reference.h`](../../engine/include/ftd/eft/catalytic_phase_reference.h) | 113 | @file catalytic_phase_reference.h @brief FTD-0863 isolated catalytic phase-reference transducer witness. |
| [`cuda_momentum_transport_current.h`](../../engine/include/ftd/eft/cuda_momentum_transport_current.h) | 113 | @file cuda_momentum_transport_current.h @brief Fused per-tick masked momentum-ledger reduction on CUDA. |
| [`genesis_natural_extension.h`](../../engine/include/ftd/eft/genesis_natural_extension.h) | 111 | @file genesis_natural_extension.h @brief Observer-only exact-real natural extension and branchwise symplectic lift of the canonical single-genesis trial (FTD-0570). |
| [`ignition_cut_support_ablation.h`](../../engine/include/ftd/eft/ignition_cut_support_ablation.h) | 111 | @file ignition_cut_support_ablation.h @brief Observer-only ignition-cut mechanism ablation (FTD-0587). |
| [`momentum_face_balance.h`](../../engine/include/ftd/eft/momentum_face_balance.h) | 111 | @file momentum_face_balance.h @brief Exact componentwise momentum-continuity lift of oriented face current (FTD-0514). |
| [`lorentz_ir_envelope.h`](../../engine/include/ftd/lorentz_ir_envelope.h) | 111 | @file ftd/lorentz_ir_envelope.h @brief Leading infrared Lorentz-violation envelope for the FTD-0413 cone. |
| [`collective_reaction_triplet_inertia.h`](../../engine/include/ftd/eft/collective_reaction_triplet_inertia.h) | 110 | @file collective_reaction_triplet_inertia.h @brief FTD-0891/0892 collective symplectic and inertia witness. |
| [`scale_engine.h`](../../engine/include/ftd/scale_engine.h) | 110 | ScaleEngine: Abstract base class for all FTD per-scale simulation engines. |
| [`native_modal_phase_action.h`](../../engine/include/ftd/eft/native_modal_phase_action.h) | 109 | Target-blind action-angle chart for one nonzero source-free field mode. |
| [`injector.h`](../../engine/include/ftd/injector.h) | 109 | @file injector.h @brief Injector — owns particle-ID and pair-ID counters. |
| [`genesis_reservoir_dilation.h`](../../engine/include/ftd/eft/genesis_reservoir_dilation.h) | 107 | @file genesis_reservoir_dilation.h @brief Observer-only reversible-dilation analysis for the frozen genesis/evaporation event kernel (FTD-0569). |
| [`hamiltonian_ternary_quarter_turn_actuator.h`](../../engine/include/ftd/eft/hamiltonian_ternary_quarter_turn_actuator.h) | 107 | @file hamiltonian_ternary_quarter_turn_actuator.h @brief FTD-0873 isolated Hamiltonian lift of the ternary quarter-turn. |
| [`visual_snapshot.h`](../../engine/include/ftd/visual_snapshot.h) | 107 | @file visual_snapshot.h @brief Versioned asynchronous capture contract for native visual frames. |
| [`axial_face_hop_reciprocity.h`](../../engine/include/ftd/eft/axial_face_hop_reciprocity.h) | 106 | @file axial_face_hop_reciprocity.h @brief Observer-only axial face-current transaction through one native remainder threshold (FTD-0497). |
| [`emergent_charge_surface.h`](../../engine/include/ftd/eft/emergent_charge_surface.h) | 106 | @file emergent_charge_surface.h @brief Read-only closed-surface charge observer for the native flux field. |
| [`half_tick_link_exchange.h`](../../engine/include/ftd/eft/half_tick_link_exchange.h) | 105 | @file half_tick_link_exchange.h @brief Selected reversible exchange ledger on an oriented Moore link. |
| [`site_ontic_atomic_reciprocal_hop.h`](../../engine/include/ftd/eft/site_ontic_atomic_reciprocal_hop.h) | 105 | @file site_ontic_atomic_reciprocal_hop.h @brief Locked observer-only Gate R0 atomic hop candidate (FTD-0599). |
| [`telemetry_snapshot.h`](../../engine/include/ftd/telemetry_snapshot.h) | 105 | @file telemetry_snapshot.h @brief Versioned, coherent observation snapshots for interactive engines. |
| [`flux_wave_velocity_markov_carrier.h`](../../engine/include/ftd/eft/flux_wave_velocity_markov_carrier.h) | 104 | @file flux_wave_velocity_markov_carrier.h @brief FTD-0876 read-only canonical chart for native flux/wave velocity. |
| [`minimal_moore_compatibility_coat.h`](../../engine/include/ftd/eft/minimal_moore_compatibility_coat.h) | 104 | @file minimal_moore_compatibility_coat.h @brief Observer-only local bridge from face to central continuity (FTD-0577). |
| [`gpu_term_contract.h`](../../engine/include/ftd/gpu_term_contract.h) | 104 | Live CUDA implementation class for every TOGGLE_SPECS row. |
| [`relative_action_transducer.h`](../../engine/include/ftd/eft/relative_action_transducer.h) | 102 | @file relative_action_transducer.h @brief FTD-0860 isolated nonzero-carrier action-pump witness. |
| [`link_action_work.h`](../../engine/include/ftd/eft/link_action_work.h) | 101 | @file link_action_work.h @brief Observer-only comparison between site-gradient impulse and exact finite-link interaction work. |
| [`moore_link_routes.h`](../../engine/include/ftd/eft/moore_link_routes.h) | 100 | @file moore_link_routes.h @brief Analysis-only routing of one Moore hop through oriented SC faces. |
| [`quadratic_coat_orbit_gather.h`](../../engine/include/ftd/eft/quadratic_coat_orbit_gather.h) | 100 | @file quadratic_coat_orbit_gather.h @brief Quadratic-coat face/edge orbit gathers and commuting curl (FTD-0550). |
| [`removal_time_pulse_bound.h`](../../engine/include/ftd/eft/removal_time_pulse_bound.h) | 100 | @file removal_time_pulse_bound.h @brief Observer-only exact removal-history pulse bounds (FTD-0589). |
| [`consciousness.h`](../../engine/include/ftd/ontic/consciousness.h) | 100 | ontic/reference frame context.h — Layers 8 and 8b of the ontic chain. |
| [`observable_registry.h`](../../engine/include/ftd/observable_registry.h) | 99 | @file observable_registry.h @brief Seed registry for constructor-domain observable maps. |
| [`collective_source_history_bound.h`](../../engine/include/ftd/eft/collective_source_history_bound.h) | 97 | @file collective_source_history_bound.h @brief Observer-only collective causal-source bounds (FTD-0588). |
| [`clock_gated_hamiltonian_exchange.h`](../../engine/include/ftd/eft/clock_gated_hamiltonian_exchange.h) | 96 | @file clock_gated_hamiltonian_exchange.h @brief FTD-0865 isolated autonomous Hamiltonian exchange witness. |
| [`external_drive_radiation.h`](../../engine/include/ftd/eft/external_drive_radiation.h) | 94 | @file external_drive_radiation.h @brief Exact modal work and external-drive radiation observer (FTD-0559). |
| [`discrete_interaction_work.h`](../../engine/include/ftd/eft/discrete_interaction_work.h) | 93 | @file discrete_interaction_work.h @brief Exact finite-site virtual work for L_int = G_C sum_x s_x div(J)_x. |
| [`coupled_quartic_clock_field.h`](../../engine/include/ftd/eft/coupled_quartic_clock_field.h) | 91 | @file coupled_quartic_clock_field.h @brief FTD-0770 selected coupled-clock EFT probe. |
| [`derived_interaction_graph.h`](../../engine/include/ftd/eft/derived_interaction_graph.h) | 91 | @file derived_interaction_graph.h @brief Observer-only reversible derived-topology pair transaction (FTD-0721). |
| [`bloch_quasimomentum_lift.h`](../../engine/include/ftd/eft/bloch_quasimomentum_lift.h) | 90 | @file bloch_quasimomentum_lift.h @brief FTD-0894/0896 isolated Bloch wrap/lift/carry reference witness. |
| [`centered_fiber_knot_transaction.h`](../../engine/include/ftd/eft/centered_fiber_knot_transaction.h) | 90 | @file centered_fiber_knot_transaction.h @brief Unique centered knot-to-subcell transaction with an explicit dressing-history fiber (FTD-0496). |
| [`common_moore_worldline_action.h`](../../engine/include/ftd/eft/common_moore_worldline_action.h) | 89 | @file common_moore_worldline_action.h @brief Observer-only spacetime/action completion of the Moore coat (FTD-0578). |
| [`dressed_boost_momentum_map.h`](../../engine/include/ftd/eft/dressed_boost_momentum_map.h) | 89 | @file dressed_boost_momentum_map.h @brief FTD-0893 conditional dressed-inertia reference witness. |
| [`quartic_relative_carry_gearbox.h`](../../engine/include/ftd/eft/quartic_relative_carry_gearbox.h) | 89 | @file quartic_relative_carry_gearbox.h @brief FTD-0898 isolated relative-quartic impulse/carry composition. |
| [`dual_cell_continuity.h`](../../engine/include/ftd/eft/dual_cell_continuity.h) | 88 | @file ftd/eft/dual_cell_continuity.h @brief Finite-volume reaction/transport continuity and b=2 blocking. |
| [`reciprocal_carry_reservoir.h`](../../engine/include/ftd/eft/reciprocal_carry_reservoir.h) | 88 | @file reciprocal_carry_reservoir.h @brief FTD-0897 isolated reciprocal-carry transaction witness. |
| [`quadratic_coat_composite_peierls.h`](../../engine/include/ftd/eft/quadratic_coat_composite_peierls.h) | 87 | @file quadratic_coat_composite_peierls.h @brief Observer-only rigid-composite Peierls analysis (FTD-0553). |
| [`visual_field_sample.h`](../../engine/include/ftd/visual_field_sample.h) | 87 | _symbols:_ VisualFieldSample |
| [`native_moving_source_pole.h`](../../engine/include/ftd/eft/native_moving_source_pole.h) | 85 | @file native_moving_source_pole.h @brief Observer-only correction of the native moving-source pole (FTD-0558). |
| [`alternating_oriented_ternary_parity_rail.h`](../../engine/include/ftd/eft/alternating_oriented_ternary_parity_rail.h) | 83 | @file alternating_oriented_ternary_parity_rail.h @brief FTD-0874 isolated alternating nearest-neighbour ternary rail. |
| [`production_hop_kinematics.h`](../../engine/include/ftd/eft/production_hop_kinematics.h) | 83 | @file production_hop_kinematics.h @brief Analysis-only momentum form of the production flat kinematics. |
| [`scale.h`](../../engine/include/ftd/scale.h) | 83 | Multi-Scale Physics: OnticEntity and Scale definitions Phase 7: The universal ternary triple {state, energy, boundary} recurs at every scale of reality. |
| [`coupled_wave_tick_snapshot.h`](../../engine/include/ftd/eft/coupled_wave_tick_snapshot.h) | 82 | @file coupled_wave_tick_snapshot.h @brief Linear-time snapshot observer for the coupled wave kick-drift. |
| [`constituent_relative_collision.h`](../../engine/include/ftd/eft/constituent_relative_collision.h) | 81 | @file constituent_relative_collision.h @brief Observer-only constituent-relative boundary collision audit (FTD-0512). |
| [`boundary_chart_capacity.h`](../../engine/include/ftd/eft/boundary_chart_capacity.h) | 80 | @file boundary_chart_capacity.h @brief Stable-chart storage audit for coincident manifested carriers (FTD-0507). |
| [`scenarios.h`](../../engine/include/ftd/scenarios.h) | 80 | ========================================================================== engine/include/ftd/scenarios.h C++ port of the Scale-0 scenario library that was previously JS-only on the MockBridge (eng... |
| [`vtk_export.h`](../../engine/include/ftd/vtk_export.h) | 80 | Native ParaView/VTK research export for RenderBridge snapshots. |
| [`cuda_state_only_support_ladder.h`](../../engine/include/ftd/eft/cuda_state_only_support_ladder.h) | 79 | @file cuda_state_only_support_ladder.h @brief Device reduction for the FTD-0754 state-only support ladder. |
| [`discrete_hop_mechanics.h`](../../engine/include/ftd/eft/discrete_hop_mechanics.h) | 79 | @file discrete_hop_mechanics.h @brief Selected reversible longitudinal map for finite-site hop work. |
| [`reversible_ternary_signal_uncomputation.h`](../../engine/include/ftd/eft/reversible_ternary_signal_uncomputation.h) | 79 | @file reversible_ternary_signal_uncomputation.h @brief FTD-0871 reversible actual-layer latch uncomputation witness. |
| [`ten_source_shared_m_coherence.h`](../../engine/include/ftd/eft/ten_source_shared_m_coherence.h) | 79 | @file ten_source_shared_m_coherence.h @brief Exact shared-stencil-eigenvalue refinement at N=10 (FTD-0594). |
| [`ternary_eligibility_clutch.h`](../../engine/include/ftd/eft/ternary_eligibility_clutch.h) | 79 | @file ternary_eligibility_clutch.h @brief FTD-0867 isolated ternary clutch and one-shot handshake witness. |
| [`face_current_segment.h`](../../engine/include/ftd/eft/face_current_segment.h) | 78 | @file face_current_segment.h @brief Exact straight-segment current for the sub-cell polarity shape. |
| [`gpu_dual_cell_fields.cuh`](../../engine/include/ftd/eft/gpu_dual_cell_fields.cuh) | 78 | @file ftd/eft/gpu_dual_cell_fields.cuh @brief Device-side data structures for GPU-native EFT calculations. |
| [`open_worldline_hop_selector.h`](../../engine/include/ftd/eft/open_worldline_hop_selector.h) | 78 | @file open_worldline_hop_selector.h @brief Observer for the gauge status of finite open-hop action comparisons. |
| [`dual_cell_blocking.h`](../../engine/include/ftd/eft/dual_cell_blocking.h) | 77 | @file ftd/eft/dual_cell_blocking.h @brief Native finite-volume source/flux fields and b=2 blocking. |
| [`extended_source_peierls_scaling.h`](../../engine/include/ftd/eft/extended_source_peierls_scaling.h) | 77 | @file extended_source_peierls_scaling.h @brief Observer-only spectral pinning analysis for extended sources (FTD-0555). |
| [`hard_contact_corner_action.h`](../../engine/include/ftd/eft/hard_contact_corner_action.h) | 77 | @file hard_contact_corner_action.h @brief Observer-only relativistic hard-contact corner action (FTD-0516). |
| [`history_event_journal.h`](../../engine/include/ftd/eft/history_event_journal.h) | 77 | @file ftd/eft/history_event_journal.h @brief Read-only event instrumentation for the native charge gate. |
| [`matched_symmetry_ray_spectrum.h`](../../engine/include/ftd/eft/matched_symmetry_ray_spectrum.h) | 77 | @file matched_symmetry_ray_spectrum.h @brief Carrier-aware Fourier observer for the matched face/edge field. |
| [`ten_source_distance_distribution_lp.h`](../../engine/include/ftd/eft/ten_source_distance_distribution_lp.h) | 77 | @file ten_source_distance_distribution_lp.h @brief Sparse dual-certificate verifier for FTD-0596. |
| [`matched_regional_energy_transport.h`](../../engine/include/ftd/eft/matched_regional_energy_transport.h) | 76 | @file matched_regional_energy_transport.h @brief Exact regional split of matched modified field energy (FTD-0671). |
| [`poisson_solvers.h`](../../engine/include/ftd/poisson_solvers.h) | 76 | Poisson solvers — SOR sweep + top-level solvers. |
| [`cluster_observables.h`](../../engine/include/ftd/cluster_observables.h) | 74 | Per-cluster observables for the cluster-thermodynamics EXPLORATORY campaign. |
| [`oriented_ternary_quarter_turn.h`](../../engine/include/ftd/eft/oriented_ternary_quarter_turn.h) | 74 | @file oriented_ternary_quarter_turn.h @brief FTD-0872 isolated reversible ternary source/port quarter-turn. |
| [`quadratic_coat_spacetime_action.h`](../../engine/include/ftd/eft/quadratic_coat_spacetime_action.h) | 74 | @file quadratic_coat_spacetime_action.h @brief Exact spacetime completion of the quadratic coupling coat (FTD-0542). |
| [`visual_sample_grid.h`](../../engine/include/ftd/visual_sample_grid.h) | 74 | ───────────────────────────────────────────────────────────────────────────── Shared sampling grid for the visual field-overlay samplers. |
| [`native_energy_contract.h`](../../engine/include/ftd/eft/native_energy_contract.h) | 73 | @file native_energy_contract.h @brief Observer-only energy decomposition for the exact production wave tick. |
| [`ten_source_pair_distance_capacity.h`](../../engine/include/ftd/eft/ten_source_pair_distance_capacity.h) | 73 | @file ten_source_pair_distance_capacity.h @brief Two-class pair-distance capacity bound at N=10 (FTD-0595). |
| [`pole_matching.h`](../../engine/include/ftd/eft/pole_matching.h) | 72 | @file ftd/eft/pole_matching.h @brief Scheme-carrying records for physical pole and universal-cone matching. |
| [`interop_particle_record.h`](../../engine/include/ftd/interop_particle_record.h) | 72 | @file interop_particle_record.h @brief GPU-resident particle record shared between the CUDA gather kernel and the D3D12 vertex shader that reads it via a StructuredBuffer. |
| [`connected_reservoir_decomposition.h`](../../engine/include/ftd/eft/connected_reservoir_decomposition.h) | 71 | @file connected_reservoir_decomposition.h @brief Exact complete-state perturbation reservoir ledger (FTD-0673). |
| [`integer_bloch_transport.h`](../../engine/include/ftd/eft/integer_bloch_transport.h) | 71 | @file integer_bloch_transport.h @brief Observer-only Bloch analysis of the isolated production wave map (FTD-0556). |
| [`localized_basin_observer.h`](../../engine/include/ftd/eft/localized_basin_observer.h) | 71 | @file localized_basin_observer.h @brief Observer-only localized rest-basin metric (FTD-0677). |
| [`matched_action_normalization.h`](../../engine/include/ftd/eft/matched_action_normalization.h) | 71 | @file matched_action_normalization.h @brief Coefficient consequences of the selected matched gauge action. |
| [`quadratic_coat_neutral_pair_work.h`](../../engine/include/ftd/eft/quadratic_coat_neutral_pair_work.h) | 70 | @file quadratic_coat_neutral_pair_work.h @brief Neutral self-consistent longitudinal coat transaction (FTD-0546). |
| [`neutrino.h`](../../engine/include/ftd/ontic/neutrino.h) | 70 | ontic/neutrino.h — Layer 7b: Absolute Neutrino Masses (Seesaw). |
| [`native_gauss_monopole_dichotomy.h`](../../engine/include/ftd/eft/native_gauss_monopole_dichotomy.h) | 69 | @file native_gauss_monopole_dichotomy.h @brief Observer-only Gauss-monopole/mobile-dressing dichotomy (FTD-0563). |
| [`reciprocal_record_port.h`](../../engine/include/ftd/eft/reciprocal_record_port.h) | 68 | @file reciprocal_record_port.h @brief FTD-0856 isolated reciprocal record/field boundary witness. |
| [`movement_order.h`](../../engine/include/ftd/movement_order.h) | 68 | Coordinate-independent movement helpers shared by CPU phase_movement and the CUDA serial commit kernel. |
| [`dual_cell_flow.h`](../../engine/include/ftd/eft/dual_cell_flow.h) | 67 | @file ftd/eft/dual_cell_flow.h @brief Bare native-flow measurements for finite-volume dual-cell fields. |
| [`face_flux_normalization.h`](../../engine/include/ftd/eft/face_flux_normalization.h) | 67 | @file face_flux_normalization.h @brief Selected normalization map from the matched face complex to native J. |
| [`lattice_coulomb_gate.h`](../../engine/include/ftd/eft/lattice_coulomb_gate.h) | 67 | @file ftd/eft/lattice_coulomb_gate.h @brief Phase-G lattice Coulomb gate paired with energy_audit conventions. |
| [`bridge_rng.h`](../../engine/include/ftd/bridge_rng.h) | 66 | @file bridge_rng.h @brief PIMPL'd RNG state for RenderBridge. |
| [`multibody_shape_observability.h`](../../engine/include/ftd/eft/multibody_shape_observability.h) | 66 | @file multibody_shape_observability.h @brief Additive trilinear-shape and exact face-current observability for multiple worldline segments (FTD-0501). |
| [`native_hop_dressing_obstruction.h`](../../engine/include/ftd/eft/native_hop_dressing_obstruction.h) | 66 | @file native_hop_dressing_obstruction.h @brief Observer-only periodic point-hop dressing obstruction (FTD-0560). |
| [`native_ternary_plaquette_quarter_turn.h`](../../engine/include/ftd/eft/native_ternary_plaquette_quarter_turn.h) | 65 | @file native_ternary_plaquette_quarter_turn.h @brief FTD-0914 isolated ternary-plaquette recursion analyzer. |
| [`overshoot_preserving_contact_rebase.h`](../../engine/include/ftd/eft/overshoot_preserving_contact_rebase.h) | 65 | @file overshoot_preserving_contact_rebase.h @brief Quotient-correct paired contact rebase and raw inverse audit (FTD-0527). |
| [`ten_source_temporal_product_capacity.h`](../../engine/include/ftd/eft/ten_source_temporal_product_capacity.h) | 65 | @file ten_source_temporal_product_capacity.h @brief Observer-only verifier for the FTD-0597 temporal product capacity. |
| [`discrete_legendre_worldline.h`](../../engine/include/ftd/eft/discrete_legendre_worldline.h) | 63 | @file discrete_legendre_worldline.h @brief Interior discrete Legendre transform of the FTD-0484 action. |
| [`full_surface_source_obstruction.h`](../../engine/include/ftd/eft/full_surface_source_obstruction.h) | 63 | @file full_surface_source_obstruction.h @brief Observer-only finite-source full-resonance obstruction (FTD-0562). |
| [`implicit_atomic_endpoint_solve.h`](../../engine/include/ftd/eft/implicit_atomic_endpoint_solve.h) | 63 | @file implicit_atomic_endpoint_solve.h @brief Six-coordinate initial-value solve of the FTD-0536 action (FTD-0537). |
| [`volumetric_measure.h`](../../engine/include/ftd/volumetric_measure.h) | 63 | @file volumetric_measure.h @brief Explicit spatial measure for the three-dimensional unit lattice. |
| [`dressing_fiber_ledger.h`](../../engine/include/ftd/eft/dressing_fiber_ledger.h) | 62 | @file dressing_fiber_ledger.h @brief Minimal history-fiber bookkeeping for cusp work (FTD-0495). |
| [`finite_memory_reversible_lift.h`](../../engine/include/ftd/eft/finite_memory_reversible_lift.h) | 62 | @file finite_memory_reversible_lift.h @brief Finite-fiber obstruction and unbounded-history control for lifting a many-to-one raw matter map (FTD-0499). |
| [`local_polarity_regularity.h`](../../engine/include/ftd/eft/local_polarity_regularity.h) | 62 | @file local_polarity_regularity.h @brief Exact regularity audit for local subcell polarity kernels (FTD-0540). |
| [`cusp_dressing_integrability.h`](../../engine/include/ftd/eft/cusp_dressing_integrability.h) | 61 | @file cusp_dressing_integrability.h @brief Cellwise cusp energy and global gluing obstruction (FTD-0494). |
| [`continuous_translation_locality.h`](../../engine/include/ftd/eft/continuous_translation_locality.h) | 60 | @file continuous_translation_locality.h @brief Observer for the exact-translation/locality trilemma (FTD-0554). |
| [`noncompact_face_cohomology.h`](../../engine/include/ftd/eft/noncompact_face_cohomology.h) | 60 | @file noncompact_face_cohomology.h @brief Observer-only cohomology/local-defect gate for the matched complex (FTD-0583). |
| [`production_same_sign_bounce.h`](../../engine/include/ftd/eft/production_same_sign_bounce.h) | 60 | @file production_same_sign_bounce.h @brief Read-only production same-sign collision reciprocity audit (FTD-0506). |
| [`accelerated_coat_spacetime_current.h`](../../engine/include/ftd/eft/accelerated_coat_spacetime_current.h) | 58 | @file accelerated_coat_spacetime_current.h @brief Nonuniform-time quadratic-coat current deposits (FTD-0548). |
| [`native_hodge_reciprocity.h`](../../engine/include/ftd/eft/native_hodge_reciprocity.h) | 58 | @file native_hodge_reciprocity.h @brief Observer-only reciprocal-force/static-pole audit (FTD-0575). |
| [`quadratic_coat_discrete_gradient_transaction.h`](../../engine/include/ftd/eft/quadratic_coat_discrete_gradient_transaction.h) | 58 | @file quadratic_coat_discrete_gradient_transaction.h @brief Selected reciprocal quadratic-coat matter/field step (FTD-0551). |
| [`contact_quotient_horizon.h`](../../engine/include/ftd/eft/contact_quotient_horizon.h) | 57 | @file contact_quotient_horizon.h @brief Actual-production quotient horizon for identical contact (FTD-0526). |
| [`endogenous_reaction_carrier_bound.h`](../../engine/include/ftd/eft/endogenous_reaction_carrier_bound.h) | 57 | @file endogenous_reaction_carrier_bound.h @brief Observer-only endogenous genesis/autocatalysis bound (FTD-0586). |
| [`hop_source_multipole_hierarchy.h`](../../engine/include/ftd/eft/hop_source_multipole_hierarchy.h) | 57 | @file hop_source_multipole_hierarchy.h @brief Observer-only slow-hop source multipole hierarchy (FTD-0561). |
| [`removal_time_orbit_coherence.h`](../../engine/include/ftd/eft/removal_time_orbit_coherence.h) | 57 | @file removal_time_orbit_coherence.h @brief Cubic-orbit coherence bound for arbitrary removal histories (FTD-0590). |
| [`strong_stress_energy.h`](../../engine/include/ftd/strong_stress_energy.h) | 57 | _symbols:_ RenderBridge, StrongStressCell, StrongEnergyStepDiagnostics |
| [`coupled_wave_tick.h`](../../engine/include/ftd/eft/coupled_wave_tick.h) | 56 | @file coupled_wave_tick.h @brief Reversible observer form of the production wave/coupling kick-drift. |
| [`cubic_hop_response.h`](../../engine/include/ftd/eft/cubic_hop_response.h) | 56 | @file cubic_hop_response.h @brief Cubic-covariant isolated-hop work response (analysis only). |
| [`native_contact_active_set.h`](../../engine/include/ftd/eft/native_contact_active_set.h) | 56 | @file native_contact_active_set.h @brief Geometry observer for frozen production vs selected hard contact (FTD-0525). |
| [`symmetric_diagonal_coupled_endpoint.h`](../../engine/include/ftd/eft/symmetric_diagonal_coupled_endpoint.h) | 56 | @file symmetric_diagonal_coupled_endpoint.h @brief Energy-coupled symmetric edge/corner endpoint observer (FTD-0531). |
| [`free_flux_localization.h`](../../engine/include/ftd/eft/free_flux_localization.h) | 55 | @file free_flux_localization.h @brief Analytic observer for the isolated free-flux localization boundary (FTD-0557). |
| [`knot_legendre_branch.h`](../../engine/include/ftd/eft/knot_legendre_branch.h) | 55 | @file knot_legendre_branch.h @brief Incident-cell branch census at a manifested lattice knot (FTD-0491). |
| [`native_hodge_energy_continuity.h`](../../engine/include/ftd/eft/native_hodge_energy_continuity.h) | 55 | @file native_hodge_energy_continuity.h @brief Observer-only native Hodge energy/continuity audit (FTD-0576). |
| [`quadratic_coat_matter_work.h`](../../engine/include/ftd/eft/quadratic_coat_matter_work.h) | 55 | @file quadratic_coat_matter_work.h @brief Endpoint Legendre and matter-work audit for the smooth coat (FTD-0545). |
| [`genesis_action_obstruction.h`](../../engine/include/ftd/eft/genesis_action_obstruction.h) | 54 | @file genesis_action_obstruction.h @brief Exact observer for the genesis amplitude/common-action gate (FTD-0567). |
| [`moore_channel_projection.h`](../../engine/include/ftd/eft/moore_channel_projection.h) | 54 | @file moore_channel_projection.h @brief Exact 13-channel Moore-shell to three-vector projection. |
| [`multicell_worldline_variation.h`](../../engine/include/ftd/eft/multicell_worldline_variation.h) | 54 | @file multicell_worldline_variation.h @brief Complete deposited-action variation through internal cell knots (FTD-0533). |
| [`subcell_polarity_shape.h`](../../engine/include/ftd/eft/subcell_polarity_shape.h) | 54 | @file subcell_polarity_shape.h @brief Compact signed trilinear charge shape for a manifested polarity. |
| [`gauge_field.h`](../../engine/include/ftd/gauge_field.h) | 54 | @file engine/include/ftd/gauge_field.h @purpose Declarations of edge-based SU(2) and SU(3) link variable structures for non-Abelian gauge field simulations (Scale 0 upgrades). |
| [`component_aware_radial_field_profile.h`](../../engine/include/ftd/eft/component_aware_radial_field_profile.h) | 53 | @file component_aware_radial_field_profile.h @brief Fixed-origin component-aware radial field morphology (FTD-0683). |
| [`endpoint_recoil_support.h`](../../engine/include/ftd/eft/endpoint_recoil_support.h) | 53 | @file endpoint_recoil_support.h @brief Exact endpoint splits of a longitudinal hop recoil (analysis only). |
| [`momentum_selected_worldline_matching.h`](../../engine/include/ftd/eft/momentum_selected_worldline_matching.h) | 53 | @file momentum_selected_worldline_matching.h @brief Exact free discrete-Legendre endpoint permutation matcher (FTD-0503). |
| [`native_field_discrete_action.h`](../../engine/include/ftd/eft/native_field_discrete_action.h) | 53 | @file native_field_discrete_action.h @brief Observer-only native wave-action and source-operator audit (FTD-0574). |
| [`orientation_gauss_independence.h`](../../engine/include/ftd/eft/orientation_gauss_independence.h) | 53 | @file orientation_gauss_independence.h @brief Observer-only orientation-degree/Gauss-flux independence (FTD-0564). |
| [`lorentz_bcc_time.h`](../../engine/include/ftd/lorentz_bcc_time.h) | 53 | @file ftd/lorentz_bcc_time.h @brief Stable local IR surrogate for the selected BCC-time cone hypothesis. |
| [`accelerated_worldline_energy.h`](../../engine/include/ftd/eft/accelerated_worldline_energy.h) | 51 | @file accelerated_worldline_energy.h @brief Exact uniform-force relativistic worldline observer (FTD-0547). |
| [`diagonal_endpoint_action_domain.h`](../../engine/include/ftd/eft/diagonal_endpoint_action_domain.h) | 51 | @file diagonal_endpoint_action_domain.h @brief Composition audit for the coupled diagonal endpoint and compact one-cell worldline action (FTD-0532). |
| [`matched_contact_energy_obstruction.h`](../../engine/include/ftd/eft/matched_contact_energy_obstruction.h) | 51 | @file matched_contact_energy_obstruction.h @brief Field-independent elastic-contact energy obstruction (FTD-0529). |
| [`constants_shared.h`](../../engine/include/ftd/constants_shared.h) | 49 | Shared physics constants — included by both constants.h and CUDA kernels. |
| [`two_slab_variational_force.h`](../../engine/include/ftd/eft/two_slab_variational_force.h) | 49 | @file two_slab_variational_force.h @brief Two-slab path variation of the selected Whitney action (FTD-0485). |
| [`axial_contact_longitudinal_work.h`](../../engine/include/ftd/eft/axial_contact_longitudinal_work.h) | 48 | @file axial_contact_longitudinal_work.h @brief Gauss-fixed axial contact work audit (FTD-0530). |
| [`matched_face_current_spectrum.h`](../../engine/include/ftd/eft/matched_face_current_spectrum.h) | 48 | @file matched_face_current_spectrum.h @brief Carrier-aware Fourier observer for oriented face current (FTD-0702). |
| [`passive_dressing_depinning_obstruction.h`](../../engine/include/ftd/eft/passive_dressing_depinning_obstruction.h) | 48 | @file passive_dressing_depinning_obstruction.h @brief Observer-only passive-dressing/depinning discriminator (FTD-0581). |
| [`ontic.h`](../../engine/include/ftd/ontic.h) | 48 | Ontic constant registry — umbrella header. |
| [`configuration_space_carrier.h`](../../engine/include/ftd/eft/configuration_space_carrier.h) | 47 | @file configuration_space_carrier.h @brief Observer-only fixed-source configuration-space carrier gate (FTD-0584). |
| [`matched_midpoint_poynting.h`](../../engine/include/ftd/eft/matched_midpoint_poynting.h) | 46 | @file matched_midpoint_poynting.h @brief Exact matched-Maxwell midpoint work identity (FTD-0544). |
| [`subcell_representation_quotient.h`](../../engine/include/ftd/eft/subcell_representation_quotient.h) | 46 | @file subcell_representation_quotient.h @brief Exact overlapping-chart quotient of site + subcell remainder (FTD-0498). |
| [`ten_source_orbit_coherence.h`](../../engine/include/ftd/eft/ten_source_orbit_coherence.h) | 46 | @file ten_source_orbit_coherence.h @brief Locked N=10 evaluation of the FTD-0590 orbit bound (FTD-0593). |
| [`worldline_current_kernel.h`](../../engine/include/ftd/eft/worldline_current_kernel.h) | 46 | @file worldline_current_kernel.h @brief Exact divergence-kernel dimension and constructive spanning-tree routing for periodic oriented face currents (FTD-0502). |
| [`cuda_quadratic_coat_orbit_gather.h`](../../engine/include/ftd/eft/cuda_quadratic_coat_orbit_gather.h) | 45 | @file cuda_quadratic_coat_orbit_gather.h @brief Device-resident quadratic-coat orbit gather (FTD-0759). |
| [`eight_source_orbit_coherence.h`](../../engine/include/ftd/eft/eight_source_orbit_coherence.h) | 45 | @file eight_source_orbit_coherence.h @brief Locked N=8 evaluation of the FTD-0590 orbit bound (FTD-0591). |
| [`finite_rigid_moore_carrier_obstruction.h`](../../engine/include/ftd/eft/finite_rigid_moore_carrier_obstruction.h) | 45 | @file finite_rigid_moore_carrier_obstruction.h @brief Observer-only finite rigid-carrier obstruction (FTD-0579). |
| [`gpu_discrete_universe.h`](../../engine/include/ftd/eft/gpu_discrete_universe.h) | 45 | @file ftd/eft/gpu_discrete_universe.h @brief Standalone, hyper-optimized GPU Discrete Universe simulation engine prototype. |
| [`edge_plane_one_sided_variation.h`](../../engine/include/ftd/eft/edge_plane_one_sided_variation.h) | 44 | @file edge_plane_one_sided_variation.h @brief In-plane solve and one-sided normal audit for shell-2 action (FTD-0539). |
| [`nine_source_orbit_coherence.h`](../../engine/include/ftd/eft/nine_source_orbit_coherence.h) | 44 | @file nine_source_orbit_coherence.h @brief Locked N=9 evaluation of the FTD-0590 orbit bound (FTD-0592). |
| [`spline_poynting_momentum.h`](../../engine/include/ftd/eft/spline_poynting_momentum.h) | 44 | @file spline_poynting_momentum.h @brief Observer-only B-spline Poynting momentum candidate (FTD-0619). |
| [`scale_ratio.h`](../../engine/include/ftd/scale_ratio.h) | 44 | engine/include/ftd/scale_ratio.h This module implements FC-3 (SPEC_SCALE_RATIO_ONTOLOGY.md §6). |
| [`exact_travelling_mode.h`](../../engine/include/ftd/eft/exact_travelling_mode.h) | 43 | @file exact_travelling_mode.h @brief Exact one-axis travelling eigenmode of the production wave tick. |
| [`staggered_current_split_compatibility.h`](../../engine/include/ftd/eft/staggered_current_split_compatibility.h) | 43 | @file staggered_current_split_compatibility.h @brief Exact FTD-0484 endpoint-current split versus frozen staggered field ordering (FTD-0535). |
| [`scenario_profiles.h`](../../engine/include/ftd/scenario_profiles.h) | 43 |  |
| [`conserved_charge_basis.h`](../../engine/include/ftd/eft/conserved_charge_basis.h) | 42 | @file ftd/eft/conserved_charge_basis.h @brief Exact additive-charge nullspace for the frozen native event catalog. |
| [`native_motion_reaction_front.h`](../../engine/include/ftd/eft/native_motion_reaction_front.h) | 42 | @file native_motion_reaction_front.h @brief Observer-only transport/reaction/source-memory discriminator (FTD-0585). |
| [`ternary_block_bipole_peierls.h`](../../engine/include/ftd/eft/ternary_block_bipole_peierls.h) | 42 | @file ternary_block_bipole_peierls.h @brief Exact integer-site extended-carrier spectral observer (FTD-0621). |
| [`centered_trace_work.h`](../../engine/include/ftd/eft/centered_trace_work.h) | 41 | @file centered_trace_work.h @brief Exact work omitted by the centered knot trace (FTD-0493). |
| [`contact_quotient_coupling_scope.h`](../../engine/include/ftd/eft/contact_quotient_coupling_scope.h) | 40 | @file contact_quotient_coupling_scope.h @brief Native snapshot source versus exact history-current quotient audit (FTD-0528). |
| [`single_slab_connection_compatibility.h`](../../engine/include/ftd/eft/single_slab_connection_compatibility.h) | 40 | @file single_slab_connection_compatibility.h @brief Faraday compatibility of the FTD-0531 work field and staggered magnetic history (FTD-0534). |
| [`canonical_subcell_section.h`](../../engine/include/ftd/eft/canonical_subcell_section.h) | 39 | @file canonical_subcell_section.h @brief Observer-only centered canonical section and exact half-cell symmetry obstruction (FTD-0500). |
| [`cuda_paired_field_response.h`](../../engine/include/ftd/eft/cuda_paired_field_response.h) | 39 | CUDA reductions for the FTD-0768 paired response and regional ledger. |
| [`native_active_mode_backreaction.h`](../../engine/include/ftd/eft/native_active_mode_backreaction.h) | 39 | @file native_active_mode_backreaction.h @brief Observer-only frozen field-to-matter backreaction audit (FTD-0582). |
| [`symmetric_chord_moore_action.h`](../../engine/include/ftd/eft/symmetric_chord_moore_action.h) | 39 | @file symmetric_chord_moore_action.h @brief Observer-only symmetric chord Moore action (FTD-0580). |
| [`lorentz_period2.h`](../../engine/include/ftd/lorentz_period2.h) | 39 | @file ftd/lorentz_period2.h @brief Exact coefficients for the P4-preserving period-two wave prototype. |
| [`transmutation_phases.h`](../../engine/include/ftd/transmutation_phases.h) | 39 | Transmutation phases — optional, toggle-gated physics. |
| [`endpoint_schedule_underdetermination.h`](../../engine/include/ftd/eft/endpoint_schedule_underdetermination.h) | 38 | @file endpoint_schedule_underdetermination.h @brief Endpoint insufficiency witness for spacetime current (FTD-0549). |
| [`genesis_cubic_canonical_form.h`](../../engine/include/ftd/eft/genesis_cubic_canonical_form.h) | 38 | @file genesis_cubic_canonical_form.h @brief Observer-only O_h canonical-form classification and genesis bath-rank comparison (FTD-0573). |
| [`support_invariant_matter_predicate.h`](../../engine/include/ftd/eft/support_invariant_matter_predicate.h) | 38 | @file support_invariant_matter_predicate.h @brief State-only relational-core predicate for FTD-0755. |
| [`fixed_step_energy_scope.h`](../../engine/include/ftd/eft/fixed_step_energy_scope.h) | 37 | @file fixed_step_energy_scope.h @brief Exact fixed-step variational energy witness (FTD-0543). |
| [`genesis_minimal_bath.h`](../../engine/include/ftd/eft/genesis_minimal_bath.h) | 36 | @file genesis_minimal_bath.h @brief Observer-only minimum symplectic-bath construction for the accepted production genesis derivative (FTD-0572). |
| [`batched_regional_energy_profile.h`](../../engine/include/ftd/eft/batched_regional_energy_profile.h) | 35 | @file batched_regional_energy_profile.h @brief Algebraically equivalent multi-radius FTD-0671 observer (FTD-0686). |
| [`genesis_environment_feedback.h`](../../engine/include/ftd/eft/genesis_environment_feedback.h) | 34 | @file genesis_environment_feedback.h @brief Observer-only block-symplectic and existing-spectator audit for the accepted production genesis event (FTD-0571). |
| [`energy_ledger_compute.h`](../../engine/include/ftd/energy_ledger_compute.h) | 34 | Energy ledger computation — moved out of render_bridge.cpp in the 2026-04-18 R3 refactor. |
| [`centered_knot_trace.h`](../../engine/include/ftd/eft/centered_knot_trace.h) | 32 | @file centered_knot_trace.h @brief Unique local linear cubic-average trace at a lattice knot (FTD-0492). |
| [`wilson_dirac_gpu.h`](../../engine/include/ftd/wilson_dirac_gpu.h) | 31 | Wilson-Dirac GPU host-side API (Phase II.2-E). |
| [`cuda_transported_chart_morphology.h`](../../engine/include/ftd/eft/cuda_transported_chart_morphology.h) | 29 | CUDA reduction counterpart of the FTD-0764 morphology observer. |
| [`injection.h`](../../engine/include/ftd/injection.h) | 28 | Injection — state-mutating primitives for seeding the lattice. |
| [`diagnostics_compute.h`](../../engine/include/ftd/diagnostics_compute.h) | 26 | Diagnostics — read-only reductions over voxel state. |
| [`proper_time_rate.h`](../../engine/include/ftd/proper_time_rate.h) | 6 | Compatibility include. |
| [`native_wave_energy.h`](../../engine/include/ftd/eft/native_wave_energy.h) | 3 | Compatibility name for the exact wave-energy observer used by FTD-0452. |

### `cuda`  (29 files, 17,907 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`gpu_engine.cu`](../../engine/cuda/gpu_engine.cu) | 2222 | @file gpu_engine.cu @brief GPU-accelerated FTD tick engine. |
| [`kernels_forces.cu`](../../engine/cuda/kernels_forces.cu) | 2057 | @file kernels_forces.cu @brief GPU kernels for Phase 4 (Forces) and Phase 5 (Movement). |
| [`gpu_buffers.cu`](../../engine/cuda/gpu_buffers.cu) | 1484 | SoA device buffer management for FTD GPU engine. |
| [`cuda_state_only_support_ladder.cu`](../../engine/cuda/cuda_state_only_support_ladder.cu) | 1360 | _symbols:_ DeviceTriplet, MappedCompactPreparation, DeviceVec3, DeviceCharacteristicSample |
| [`cuda_matched_field_pipeline.cu`](../../engine/cuda/cuda_matched_field_pipeline.cu) | 1323 | _symbols:_ DeviceVectorField, DeviceCurrentEntry, DeviceCurrentGroup, DeviceSelectedRadii |
| [`diagnostic_reductions.cu`](../../engine/cuda/diagnostic_reductions.cu) | 1250 | @file diagnostic_reductions.cu @brief Fixed-size CUDA reductions for native interactive diagnostics. |
| [`kernels_aux.cu`](../../engine/cuda/kernels_aux.cu) | 1015 | @file kernels_aux.cu @brief Auxiliary physics kernels (drives, boundaries, reactions). |
| [`kernels_stencil_single.cu`](../../engine/cuda/kernels_stencil_single.cu) | 750 | @file kernels_stencil_single.cu @brief Single-substrate Phase Read / Phase Write kernels (FTD tick cycle). |
| [`kernels_stencil_dual.cu`](../../engine/cuda/kernels_stencil_dual.cu) | 643 | @file kernels_stencil_dual.cu @brief Dual-substrate Phase Read / Phase Write kernels (FTD tick cycle). |
| [`cuda_momentum_transport_current.cu`](../../engine/cuda/cuda_momentum_transport_current.cu) | 603 | _symbols:_ DeviceTriplet, DeviceBond, DeviceBondRange, DeviceRadii |
| [`cuda_paired_field_response.cu`](../../engine/cuda/cuda_paired_field_response.cu) | 564 | _symbols:_ DeviceTriplet, OwnedTriplet |
| [`kernels_poisson.cu`](../../engine/cuda/kernels_poisson.cu) | 537 | FFT-based Poisson solver for FTD GPU engine. |
| [`kernels_gauge.cu`](../../engine/cuda/kernels_gauge.cu) | 480 | @file kernels_gauge.cu @brief GPU kernels for Scale 0 Gauge Field non-Abelian plaquette relaxation. |
| [`cuda_quadratic_coat_orbit_gather.cu`](../../engine/cuda/cuda_quadratic_coat_orbit_gather.cu) | 468 | _symbols:_ DeviceVec3, DeviceCurrentEntry, DeviceOrbitSegment, DeviceOrbitOutput |
| [`cuda_transported_chart_morphology.cu`](../../engine/cuda/cuda_transported_chart_morphology.cu) | 451 | _symbols:_ DeviceTriplet |
| [`visual_field_sample.cu`](../../engine/cuda/visual_field_sample.cu) | 418 | _symbols:_ VisualDeviceView |
| [`kernels_eft.cu`](../../engine/cuda/kernels_eft.cu) | 381 | @file kernels_eft.cu @brief GPU-native EFT calculations: face-flux conversion, blocking, operator evaluation, and parallel reductions. |
| [`atom_engine_gpu.cu`](../../engine/cuda/atom_engine_gpu.cu) | 359 | GPU AtomEngine backend (Wave 5.3 Phase 1). |
| [`particle_engine_gpu.cu`](../../engine/cuda/particle_engine_gpu.cu) | 320 | GPU ParticleEngine backend (Wave 5.4 Phase 1). |
| [`kernels_injection.cu`](../../engine/cuda/kernels_injection.cu) | 318 | @file kernels_injection.cu @brief Device-resident interactive and scenario injection primitives. |
| [`kernels_matched_gauss.cu`](../../engine/cuda/kernels_matched_gauss.cu) | 221 | Native CUDA advance for FTD-0428 matched_gauss_dynamics. |
| [`wilson_dirac_gpu.cu`](../../engine/cuda/wilson_dirac_gpu.cu) | 170 | Wilson-Dirac GPU kernel (Phase II.2-E). |
| [`kernels_stencil_common.cuh`](../../engine/cuda/kernels_stencil_common.cuh) | 100 | Shared device-side helpers for the stencil kernel TUs. |
| [`cuda_index.cuh`](../../engine/cuda/cuda_index.cuh) | 78 | Shared device-side index helpers for CUDA kernels. |
| [`cuda_invariants.cu`](../../engine/cuda/cuda_invariants.cu) | 78 | cuda_invariants.cu — Implementation of the CUDA constant-memory invariant pattern declared in cuda_invariants.cuh. |
| [`cuda_device_buffer.cuh`](../../engine/cuda/cuda_device_buffer.cuh) | 71 | _symbols:_ CudaDeviceBuffer |
| [`cuda_invariants.cuh`](../../engine/cuda/cuda_invariants.cuh) | 66 | cuda_invariants.cuh — CUDA constant-memory pattern for small read-only invariants (matrices and companion scalars). |
| [`kernels_proper_time.cu`](../../engine/cuda/kernels_proper_time.cu) | 61 | @file kernels_proper_time.cu @brief Device-resident proper-time accumulation for latency scenarios. |
| [`cuda_error.cuh`](../../engine/cuda/cuda_error.cuh) | 59 | @file cuda_error.cuh — shared CUDA / cuFFT error-check macros (revision C1). |

### `web/scale0`  (65 files, 17,876 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`flux-slice-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/flux-slice-panel.js) | 1107 | Scale 0 — Live Multi-Field Flux Slice Panel The flagship \|J\| row is visible by default; every other field mirrors its 3D visualization toggle until the user explicitly enables that row. |
| [`scenario-registry.js`](../../engine/web/js/scales/scale0/scenario-registry.js) | 1099 | Scenario: empty (Empty Lattice) Physical purpose: Serves as the baseline state of the lattice with no initial particles or fields. |
| [`field-overlays.js`](../../engine/web/js/scales/scale0/runtime/field-overlays.js) | 1061 | Native mass-gravity scenarios expose the actual latency-Poisson solution as FTS2 kind 17. |
| [`scenario-loader.js`](../../engine/web/js/scales/scale0/runtime/scenario-loader.js) | 985 | Frame a compact or bounded center-seeded structure once when its scenario is loaded. |
| [`scenario-validation.js`](../../engine/web/js/scales/scale0/scenario-validation.js) | 822 | Hard admission gate for the normal Scale-0 menu. |
| [`field-line-knots.js`](../../engine/web/js/scales/scale0/runtime/field-line-knots.js) | 763 | engine/web/js/scales/scale0/runtime/field-line-knots.js Field-line KNOT detection + quantification + identity tracking — JS-native. |
| [`flux-slice-helpers.js`](../../engine/web/js/scales/scale0/ui/overlays/flux-slice-helpers.js) | 713 | Scale 0 — Flux slice panel pure helpers/constants. |
| [`overlay-frames.js`](../../engine/web/js/scales/scale0/runtime/overlay-frames.js) | 567 | ══════════════════════════════════════════════════════════════════════ Overlay frame builders — one pure function per topology overlay. |
| [`wave-info.js`](../../engine/web/js/scales/scale0/ui/overlays/wave-lab/wave-info.js) | 555 | @file engine/web/js/scales/scale0/ui/overlays/wave-lab/wave-info.js @purpose Live telemetry and controls for standalone RF/light/sound lattice waves. |
| [`knots-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/knots-panel.js) | 547 | _symbols:_ mountKnotsPanel(), initKnotsPanel() |
| [`wire.js`](../../engine/web/js/scales/scale0/ui/controls/wire.js) | 546 | Scale 0 Controls Panel Wiring Binds event listeners for every control card mounted by Scale0ControlsComponent: - Physics toggles card (all 18 toggles from SCALE0_TOGGLES) - Substrate controls card... |
| [`time-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/time-panel.js) | 532 | Time Observatory — Scale-0 time-dilation instrument. |
| [`wave-spectrum.js`](../../engine/web/js/scales/scale0/analysis/wave-spectrum.js) | 462 | _symbols:_ isSingleWaveScenario(), getWaveScenarioDefaults(), sanitizeWaveScenarioSettings(), getWaveScenarioSettings() |
| [`controller.js`](../../engine/web/js/scales/scale0/controller.js) | 457 | Scale 0 (Lattice) Controller Refactored into a package-style module with explicit runtime phases, a viewport adapter, scenario registry, and UI bindings owned by Scale 0. |
| [`gravity-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/gravity-panel.js) | 390 | Gravity Observatory — Scale-0 gravity-field instrument. |
| [`spectrum-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/spectrum-panel.js) | 355 | Lattice Spectroscopy — Scale-0 field-structure instrument. |
| [`template.js`](../../engine/web/js/scales/scale0/ui/overlays/template.js) | 318 | Scale 0 Viewport Overlay — Field visualization controls A dense 2-up chip grid: a filter box + an active-overlays strip on top, then the toggles grouped into semantic categories (collapsible per-ca... |
| [`conservation-micropanel.js`](../../engine/web/js/scales/scale0/ui/overlays/conservation-micropanel.js) | 316 | Conservation-law audit micropanel. |
| [`helium-spectrum-protocol.js`](../../engine/web/js/scales/scale0/analysis/helium-spectrum-protocol.js) | 302 | Helium lattice-spectrum protocol helpers. |
| [`store.js`](../../engine/web/js/scales/scale0/state/store.js) | 285 | localStorage may be blocked (privacy mode) — fall through to default |
| [`bindings.js`](../../engine/web/js/scales/scale0/ui/bindings.js) | 283 | v=2: Tier 1 quantum overlay bindings added — see SPEC_S0_QUANTUM_OVERLAYS.md |
| [`viewport-adapter.js`](../../engine/web/js/scales/scale0/viewport-adapter.js) | 256 | _symbols:_ createScale0ViewportAdapter() |
| [`g2.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/g2.js) | 255 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/g2.js @purpose Lepton g-2 (Schwinger) and live precession component. |
| [`p1-observables-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables-panel.js) | 250 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables-panel.js @purpose Orchestrator for the Scale 0 P1 Observables panel, composing sub-components. |
| [`coulomb.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/coulomb.js) | 250 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/coulomb.js @purpose Coulomb V(r) and E-field probe component. |
| [`thermo-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/thermo-panel.js) | 239 | Thermodynamics — docked Scale-0 side panel (FTD-0274). |
| [`scale-context-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/scale-context-panel.js) | 234 | Scale Context — docked Scale-0 side panel (FTD-0306). |
| [`applicability.js`](../../engine/web/js/scales/scale0/ui/overlays/applicability.js) | 227 | Scenario-aware availability for Scale-0 visualization overlays. |
| [`panel-shell.js`](../../engine/web/js/scales/scale0/ui/overlays/panel-shell.js) | 209 | Scale-0 Visualization panel shell — accordion + active strip + filter. |
| [`lattice-spectrum.js`](../../engine/web/js/scales/scale0/analysis/lattice-spectrum.js) | 208 | Lattice spectrum analysis — the spatial energy spectrum E(k) of the flux field. |
| [`gravity-analysis.js`](../../engine/web/js/scales/scale0/analysis/gravity-analysis.js) | 201 | Gravity analysis — pure, DOM-free scalar telemetry for the Gravity Observatory. |
| [`genesis-burst-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/genesis-burst-panel.js) | 199 | Selected genesis response N(A) — interactive fire panel (FTD-0269 provenance). |
| [`_card-helpers.js`](../../engine/web/js/scales/scale0/ui/overlays/_card-helpers.js) | 195 | Shared card-rendering helpers for Scale 0 dock-mode panels. |
| [`dispersion-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/dispersion-panel.js) | 195 | Dispersion — docked Scale-0 side panel (FTD-0298 / FTD-0299). |
| [`anisotropy.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/anisotropy.js) | 188 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/anisotropy.js @purpose Lattice Anisotropy & SO(2) Recovery component. |
| [`flux-volume.js`](../../engine/web/js/scales/scale0/ui/controls/flux-volume.js) | 174 | Scale 0 Flux Volume Card |
| [`thomson.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/thomson.js) | 162 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/thomson.js @purpose Live readout for the locked-source null and native recoil probes. |
| [`field-sample-cache.js`](../../engine/web/js/scales/scale0/runtime/field-sample-cache.js) | 157 | ══════════════════════════════════════════════════════════════════════ Lazy field-sample cache — visual overlay layer only. |
| [`fine-structure.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/fine-structure.js) | 143 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/fine-structure.js @purpose Fine-structure constant instrument panel for flux-recoil scenarios. |
| [`dom.js`](../../engine/web/js/scales/scale0/ui/dom.js) | 132 | _symbols:_ getEl(), setButtonActive(), readButtonActive(), setCheckboxValue() |
| [`bell.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/bell.js) | 127 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/bell.js @purpose Bell CHSH component. |
| [`slice-render.js`](../../engine/web/js/scales/scale0/ui/overlays/slice-render.js) | 119 | Shared 2D-slice rendering helpers for Scale-0 heatmap panels. |
| [`gravity.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/gravity.js) | 114 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/gravity.js @purpose Gravitational time dilation component. |
| [`lattice-topology.js`](../../engine/web/js/scales/scale0/analysis/lattice-topology.js) | 111 | Lattice topology + metric-distribution analysis. |
| [`diagnostics.js`](../../engine/web/js/scales/scale0/runtime/diagnostics.js) | 98 | Live decomposition tooltip for the status-bar energy readout (whole-box audit channels, sim units). |
| [`manifestation-flash.js`](../../engine/web/js/scales/scale0/runtime/manifestation-flash.js) | 97 | Scale-0 manifestation spawn-flash — a REAL "just manifested this tick" visual, distinct from the pre-existing genesis isosurface (which shows the \|J\|~K_GENESIS precondition band, not the discrete e... |
| [`wave-lab-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/wave-lab-panel.js) | 84 | @file engine/web/js/scales/scale0/ui/overlays/wave-lab-panel.js @purpose Side-panel host for standalone RF/light/sound wave instruments. |
| [`substrate-controls.js`](../../engine/web/js/scales/scale0/ui/controls/substrate-controls.js) | 80 | Scale 0 Substrate Controls Card |
| [`viewport-scalar-adapter.js`](../../engine/web/js/scales/scale0/viewport-scalar-adapter.js) | 72 | _symbols:_ createScalarOverlayAdapter() |
| [`tick.js`](../../engine/web/js/scales/scale0/runtime/tick.js) | 65 | Advance Scale-0 physics by `tickCount` ticks on the active owner only. |
| [`physics-toggles.js`](../../engine/web/js/scales/scale0/ui/controls/physics-toggles.js) | 62 | Scale 0 Physics Toggles Card |
| [`frame-sync.js`](../../engine/web/js/scales/scale0/runtime/frame-sync.js) | 58 | _symbols:_ syncRenderableData() |
| [`hydrogen.js`](../../engine/web/js/scales/scale0/ui/overlays/p1-observables/hydrogen.js) | 56 | @file engine/web/js/scales/scale0/ui/overlays/p1-observables/hydrogen.js @purpose Hydrogen Spectrum component. |
| [`time-analysis.js`](../../engine/web/js/scales/scale0/analysis/time-analysis.js) | 54 | Pure FTD causal-clock math. |
| [`streamline-integrator.js`](../../engine/web/js/scales/scale0/runtime/streamline-integrator.js) | 53 | ══════════════════════════════════════════════════════════════════════ Streamline parameter derivation + particle-buffer seeding helpers shared between the EM and force overlay builders. |
| [`component.js`](../../engine/web/js/scales/scale0/ui/controls/component.js) | 39 | Scale 0 Controls Component Mounts all Scale 0 control cards into the controls panel |
| [`template.js`](../../engine/web/js/scales/scale0/ui/toolbar/template.js) | 38 | _symbols:_ getScale0ScenarioToolbarTemplate(), getScale0LatticeSizeToolbarTemplate() |
| [`knot-line-attribution.js`](../../engine/web/js/scales/scale0/runtime/knot-line-attribution.js) | 37 | engine/web/js/scales/scale0/runtime/knot-line-attribution.js Attribute field-line streamline segments to the nearest knot centroid. |
| [`genesis-cluster-profile.js`](../../engine/web/js/scales/scale0/runtime/genesis-cluster-profile.js) | 33 | Live toggle profile for the interactive genesis-cluster measurement panel. |
| [`ftd0252-reference.js`](../../engine/web/js/scales/scale0/data/ftd0252-reference.js) | 32 | Measured FTD-0252 kinematic time-dilation data — OFFLINE campaign, NOT live. |
| [`knot-streamline-plan.js`](../../engine/web/js/scales/scale0/runtime/knot-streamline-plan.js) | 26 | engine/web/js/scales/scale0/runtime/knot-streamline-plan.js When to BUILD vs DRAW streamline jobs for field-line knot tracking. |
| [`symmetry-panel.js`](../../engine/web/js/scales/scale0/ui/overlays/symmetry-panel.js) | 24 | _symbols:_ SymmetryPanelComponent, mountSymmetryPanel() |
| [`presets.js`](../../engine/web/js/scales/scale0/ui/overlays/presets.js) | 21 | Scale 0 Overlay column groupings. |
| [`register-scale0-ui.js`](../../engine/web/js/scales/scale0/ui/register-scale0-ui.js) | 19 | _symbols:_ registerScale0ToolbarUI() |
| [`component.js`](../../engine/web/js/scales/scale0/ui/toolbar/component.js) | 18 | _symbols:_ createScale0ScenarioToolbarGroup(), createScale0LatticeSizeToolbarGroup() |

### `web/tests`  (93 files, 17,215 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`ws-bridge-visual-cache.spec.js`](../../engine/web/tests/ws-bridge-visual-cache.spec.js) | 2411 | @ts-check |
| [`scales.spec.js`](../../engine/web/tests/scales.spec.js) | 711 | @ts-check |
| [`scenario-parity.spec.js`](../../engine/web/tests/scenario-parity.spec.js) | 675 | @ts-check |
| [`overlay-scheduler.spec.js`](../../engine/web/tests/overlay-scheduler.spec.js) | 613 | @ts-check |
| [`native-ws-smoke.mjs`](../../engine/web/tests/native-ws-smoke.mjs) | 605 |  |
| [`scale0-substrate-protocol-v2.spec.js`](../../engine/web/tests/scale0-substrate-protocol-v2.spec.js) | 575 | @ts-check |
| [`panel-mount.spec.js`](../../engine/web/tests/panel-mount.spec.js) | 472 | @ts-check |
| [`perf-baseline.spec.js`](../../engine/web/tests/perf-baseline.spec.js) | 390 | @ts-check |
| [`responsive-overflow.spec.js`](../../engine/web/tests/responsive-overflow.spec.js) | 377 | @ts-check |
| [`lifecycle-harness.spec.js`](../../engine/web/tests/lifecycle-harness.spec.js) | 340 | @ts-check |
| [`reconcile-claims.spec.js`](../../engine/web/tests/reconcile-claims.spec.js) | 321 | @ts-check |
| [`scale2-physics.spec.js`](../../engine/web/tests/scale2-physics.spec.js) | 313 | @ts-check |
| [`toggle-coverage.spec.js`](../../engine/web/tests/toggle-coverage.spec.js) | 309 | @ts-check |
| [`audit-regression.spec.js`](../../engine/web/tests/audit-regression.spec.js) | 305 | @ts-check |
| [`_helpers.js`](../../engine/web/tests/_helpers.js) | 256 | @ts-check |
| [`panels-redesign.spec.js`](../../engine/web/tests/panels-redesign.spec.js) | 256 | @ts-check |
| [`scale0-overlay-applicability.spec.js`](../../engine/web/tests/scale0-overlay-applicability.spec.js) | 252 | @ts-check |
| [`audit-fix-contracts.spec.js`](../../engine/web/tests/audit-fix-contracts.spec.js) | 244 | @ts-check |
| [`scale0-scenario-health.spec.js`](../../engine/web/tests/scale0-scenario-health.spec.js) | 237 | @ts-check |
| [`scale2-atom-overlays.spec.js`](../../engine/web/tests/scale2-atom-overlays.spec.js) | 234 | @ts-check |
| [`knots-telemetry.spec.js`](../../engine/web/tests/knots-telemetry.spec.js) | 207 | @ts-check |
| [`scale1-promotion.spec.js`](../../engine/web/tests/scale1-promotion.spec.js) | 200 | @ts-check "⤴ Scale up" promotion pipeline: live Scale-0 lattice clusters → Scale-1 continuous particles (mass = N·K_B, charge = sign·N). |
| [`scale1-side-panels.spec.js`](../../engine/web/tests/scale1-side-panels.spec.js) | 197 | @ts-check |
| [`scale1-particle-overlays.spec.js`](../../engine/web/tests/scale1-particle-overlays.spec.js) | 191 | @ts-check |
| [`scale0-scenario-telemetry-contract.spec.js`](../../engine/web/tests/scale0-scenario-telemetry-contract.spec.js) | 188 | @ts-check |
| [`flux-slice-axes.spec.js`](../../engine/web/tests/flux-slice-axes.spec.js) | 183 | @ts-check |
| [`scale2-side-panels.spec.js`](../../engine/web/tests/scale2-side-panels.spec.js) | 180 | @ts-check |
| [`scale0-worker.spec.js`](../../engine/web/tests/scale0-worker.spec.js) | 177 | @ts-check |
| [`scale0-panel-request-budget.spec.js`](../../engine/web/tests/scale0-panel-request-budget.spec.js) | 170 | @ts-check |
| [`color-ramps.spec.js`](../../engine/web/tests/color-ramps.spec.js) | 166 | @ts-check |
| [`wasm-scenario-coverage.spec.js`](../../engine/web/tests/wasm-scenario-coverage.spec.js) | 163 | @ts-check |
| [`helium-lattice-spectrum.manual.js`](../../engine/web/tests/manual/helium-lattice-spectrum.manual.js) | 157 | @ts-check |
| [`take_gallery_screenshots.spec.js`](../../engine/web/tests/take_gallery_screenshots.spec.js) | 152 | @ts-check |
| [`animation-clock-freeze.spec.js`](../../engine/web/tests/animation-clock-freeze.spec.js) | 150 | @ts-check |
| [`panel-mount-integration.spec.js`](../../engine/web/tests/panel-mount-integration.spec.js) | 142 | @ts-check |
| [`field-line-knots-contributions.spec.js`](../../engine/web/tests/field-line-knots-contributions.spec.js) | 139 | engine/web/tests/field-line-knots-contributions.spec.js Per-knot scientific contributions: energy / flux / charge integrated over each knot's region, expressed as a share of the scenario total, + h... |
| [`per-scenario-position-audit.manual.js`](../../engine/web/tests/manual/per-scenario-position-audit.manual.js) | 136 | @ts-check |
| [`scene-panel.spec.js`](../../engine/web/tests/scene-panel.spec.js) | 136 | @ts-check |
| [`playback-smoke.spec.js`](../../engine/web/tests/playback-smoke.spec.js) | 135 | @ts-check |
| [`field-line-knots-color.spec.js`](../../engine/web/tests/field-line-knots-color.spec.js) | 134 | engine/web/tests/field-line-knots-color.spec.js Per-knot color (knotHue) + selection API on the field-line knot tracker. |
| [`field-line-knots-detection.spec.js`](../../engine/web/tests/field-line-knots-detection.spec.js) | 129 | engine/web/tests/field-line-knots-detection.spec.js Detection logic for the field-line knot tracker (density + crossings gate). |
| [`scale0-dynamical-flux-dressing.spec.js`](../../engine/web/tests/scale0-dynamical-flux-dressing.spec.js) | 129 | @ts-check |
| [`scale0-toggle-engine-parity.spec.js`](../../engine/web/tests/scale0-toggle-engine-parity.spec.js) | 127 | @ts-check |
| [`scale0-telemetry-gating.spec.js`](../../engine/web/tests/scale0-telemetry-gating.spec.js) | 126 | @ts-check |
| [`genesis-burst.spec.js`](../../engine/web/tests/genesis-burst.spec.js) | 125 | @ts-check |
| [`scale0-panel-wiring.spec.js`](../../engine/web/tests/scale0-panel-wiring.spec.js) | 124 | @ts-check |
| [`scale0-worker-teardown.spec.js`](../../engine/web/tests/scale0-worker-teardown.spec.js) | 123 | @ts-check |
| [`verify_web_consistency.js`](../../engine/web/tests/verify_web_consistency.js) | 118 |  |
| [`scale0-toggle-leak.spec.js`](../../engine/web/tests/scale0-toggle-leak.spec.js) | 115 | @ts-check |
| [`flux-upload-microbench.spec.js`](../../engine/web/tests/flux-upload-microbench.spec.js) | 113 | @ts-check |
| [`s0-overlay-accordion.spec.js`](../../engine/web/tests/s0-overlay-accordion.spec.js) | 112 | @ts-check |
| [`scale0-panel-render.spec.js`](../../engine/web/tests/scale0-panel-render.spec.js) | 108 | @ts-check |
| [`scale0-reciprocal-moving-source.spec.js`](../../engine/web/tests/scale0-reciprocal-moving-source.spec.js) | 106 | @ts-check |
| [`scale0-resize-guard.spec.js`](../../engine/web/tests/scale0-resize-guard.spec.js) | 105 | @ts-check |
| [`scale0-conservation-panel.spec.js`](../../engine/web/tests/scale0-conservation-panel.spec.js) | 103 | @ts-check |
| [`scale0-persisted-scenario-boot.spec.js`](../../engine/web/tests/scale0-persisted-scenario-boot.spec.js) | 103 | @ts-check |
| [`scenario-closure-parity.spec.js`](../../engine/web/tests/scenario-closure-parity.spec.js) | 102 | @ts-check |
| [`scale0-gravity.spec.js`](../../engine/web/tests/scale0-gravity.spec.js) | 101 | @ts-check |
| [`native-ws-ctest.mjs`](../../engine/web/tests/native-ws-ctest.mjs) | 99 |  |
| [`scale0-time.spec.js`](../../engine/web/tests/scale0-time.spec.js) | 99 | @ts-check |
| [`field-line-knots-identity.spec.js`](../../engine/web/tests/field-line-knots-identity.spec.js) | 98 | engine/web/tests/field-line-knots-identity.spec.js Identity persistence + birth/death/fission/fusion for the field-line knot tracker. |
| [`helium-spectrum-protocol.spec.js`](../../engine/web/tests/helium-spectrum-protocol.spec.js) | 94 | @ts-check |
| [`scale0-toggle-trap.spec.js`](../../engine/web/tests/scale0-toggle-trap.spec.js) | 94 | @ts-check |
| [`scale0-zero-point.spec.js`](../../engine/web/tests/scale0-zero-point.spec.js) | 91 | @ts-check |
| [`scale0-sampler-lifetime.spec.js`](../../engine/web/tests/scale0-sampler-lifetime.spec.js) | 84 | @ts-check |
| [`scale0-p1-fine-structure.spec.js`](../../engine/web/tests/scale0-p1-fine-structure.spec.js) | 80 | @ts-check |
| [`faq.spec.js`](../../engine/web/tests/faq.spec.js) | 79 | @ts-check |
| [`force-field-samplers.spec.js`](../../engine/web/tests/force-field-samplers.spec.js) | 74 | @ts-check |
| [`scale0-massbody.spec.js`](../../engine/web/tests/scale0-massbody.spec.js) | 70 | @ts-check |
| [`scale0-spectrum.spec.js`](../../engine/web/tests/scale0-spectrum.spec.js) | 70 | @ts-check |
| [`debug-conservation.manual.js`](../../engine/web/tests/manual/debug-conservation.manual.js) | 69 |  |
| [`web-physics-constants-contract.spec.js`](../../engine/web/tests/web-physics-constants-contract.spec.js) | 69 |  |
| [`scale0-scalecontext.spec.js`](../../engine/web/tests/scale0-scalecontext.spec.js) | 63 | @ts-check |
| [`scale0-inject-paused.spec.js`](../../engine/web/tests/scale0-inject-paused.spec.js) | 59 | @ts-check |
| [`math-formatting.spec.js`](../../engine/web/tests/math-formatting.spec.js) | 55 | @ts-check |
| [`playwright.config.js`](../../engine/web/tests/playwright.config.js) | 51 | @ts-check |
| [`field-line-knots-seeds.spec.js`](../../engine/web/tests/field-line-knots-seeds.spec.js) | 48 | engine/web/tests/field-line-knots-seeds.spec.js Coverage seeds: particle-anchored + importance-sampled field peaks, voxel-deduped. |
| [`particle-units-contract.spec.js`](../../engine/web/tests/particle-units-contract.spec.js) | 45 |  |
| [`field-line-knots-attribution-integration.spec.js`](../../engine/web/tests/field-line-knots-attribution-integration.spec.js) | 44 | engine/web/tests/field-line-knots-attribution-integration.spec.js The tracker's per-knot segments/length/legs must equal attributeSegmentsToKnots run against the tracker's OWN detected centroids (g... |
| [`epistemic-copy-contract.spec.js`](../../engine/web/tests/epistemic-copy-contract.spec.js) | 40 |  |
| [`latency-clamp-contract.spec.js`](../../engine/web/tests/latency-clamp-contract.spec.js) | 39 |  |
| [`time-analysis.node.test.mjs`](../../engine/web/tests/time-analysis.node.test.mjs) | 36 | Node unit test for the pure time-dilation math. |
| [`scale1-orbit-period.node.test.mjs`](../../engine/web/tests/scale1-orbit-period.node.test.mjs) | 34 | Node unit test for the pure orbit-period estimator. |
| [`atlas-content.node.test.mjs`](../../engine/web/tests/atlas-content.node.test.mjs) | 33 | Node unit test for the Ontology Atlas content + chain integrity + tag honesty. |
| [`atlas-data.node.test.mjs`](../../engine/web/tests/atlas-data.node.test.mjs) | 33 | Node unit test for the Ontology Atlas static analytic field math. |
| [`scale2-scenario-registry.spec.js`](../../engine/web/tests/scale2-scenario-registry.spec.js) | 33 | @ts-check |
| [`wasm-numeric-semantics.spec.js`](../../engine/web/tests/wasm-numeric-semantics.spec.js) | 32 |  |
| [`atlas.spec.js`](../../engine/web/tests/atlas.spec.js) | 31 | Smoke + acceptance test for the standalone FTD Ontology Atlas page. |
| [`scenario-profile-contract.spec.js`](../../engine/web/tests/scenario-profile-contract.spec.js) | 30 |  |
| [`scale1-mass-comparison.node.test.mjs`](../../engine/web/tests/scale1-mass-comparison.node.test.mjs) | 26 | engine/web/tests/scale1-mass-comparison.node.test.mjs Run: node engine/web/tests/scale1-mass-comparison.node.test.mjs |
| [`verify_web_consistency.test.mjs`](../../engine/web/tests/verify_web_consistency.test.mjs) | 22 |  |
| [`knot-line-attribution.spec.js`](../../engine/web/tests/knot-line-attribution.spec.js) | 15 | engine/web/tests/knot-line-attribution.spec.js |
| [`playwright.manual.config.js`](../../engine/web/tests/playwright.manual.config.js) | 8 | @ts-check |

### `web/ui`  (84 files, 10,986 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`data.js`](../../engine/web/js/ui/components/knowledge-base/data.js) | 949 | _symbols:_ getKnowledgeBaseSections(), getKnowledgeBaseEntry(), searchKnowledgeBase(), KNOWLEDGE_BASE |
| [`template.js`](../../engine/web/js/ui/components/panel-resources/template.js) | 593 | _symbols:_ getScaleControlsBlocksTemplate(), getZooPanelTemplate(), getInspectorPanelTemplate(), getPhysicsPanelTemplate() |
| [`component.js`](../../engine/web/js/ui/panels/telemetry-grid/component.js) | 474 | _symbols:_ TelemetryGridPanelComponent, initTelemetryGridPanel() |
| [`data.js`](../../engine/web/js/ui/components/faq/data.js) | 443 | FAQ sidebar data — 16 canonical hard problems framed through the FTD lens. |
| [`definitions.js`](../../engine/web/js/ui/components/tooltips/definitions.js) | 434 | _symbols:_ applyUiTooltipDefinitions() |
| [`table.js`](../../engine/web/js/ui/panels/diagnostics-panel/table.js) | 352 | DiagnosticsTable — renders one section's table + owns per-row cells, reset-scoped running stats, and per-row Sparkline instances. |
| [`panel-dock-controller.js`](../../engine/web/js/ui/shell/panel-dock-controller.js) | 308 | _symbols:_ PanelDockController |
| [`component.js`](../../engine/web/js/ui/components/sidebar-library/component.js) | 239 | SidebarLibraryComponent — reusable library-style sidebar. |
| [`component.js`](../../engine/web/js/ui/panels/scene-panel/component.js) | 234 | ScenePanelComponent — wires the Scene panel DOM to the SceneAdapter and manages localStorage-backed persistence. |
| [`app-ontic.js`](../../engine/web/js/ui/app-ontic.js) | 233 | Ontic Observatory / Physics / Hierarchy panel glue. |
| [`component.js`](../../engine/web/js/ui/components/play-bar/component.js) | 229 | PlayBarComponent — the floating transport + speed bar at the bottom of the viewport. |
| [`component.js`](../../engine/web/js/ui/components/floating-window/component.js) | 227 | FloatingWindow — high-performance draggable/resizable glassmorphic panel wrapper. |
| [`scale1.js`](../../engine/web/js/ui/panels/diagnostics-panel/descriptors/scale1.js) | 224 | Scale 1 diagnostics table descriptor. |
| [`app-shell.js`](../../engine/web/js/ui/shell/app-shell.js) | 223 | Shell facade around the current dashboard DOM. |
| [`adapter.js`](../../engine/web/js/ui/panels/scene-panel/adapter.js) | 212 | SceneAdapter — one place to translate "the user moved a slider" into "change this Three.js object on the Viewport". |
| [`uplot-chart.js`](../../engine/web/js/ui/charts/uplot-chart.js) | 207 | UPlotChart — line/area chart primitive. |
| [`physics-terms.js`](../../engine/web/js/ui/components/knowledge-base/data/physics-terms.js) | 196 | Knowledge-base section `physics-terms` |
| [`component.js`](../../engine/web/js/ui/components/viewport-overlays/component.js) | 195 | Viewport Overlays Component — mounts scale-specific and universal overlay controls Orchestrates: - Scale-specific field/visualization toggles - Universal axes/grid controls - Bottom status-bar scen... |
| [`component.js`](../../engine/web/js/ui/components/tooltips/component.js) | 194 | _symbols:_ TooltipComponent |
| [`mount-toggle.js`](../../engine/web/js/ui/components/panel-dock/mount-toggle.js) | 181 | Updates --viewport-safe-left / --viewport-safe-right on <html> so that any overlay consumers can inset themselves past the sidebar without hardcoding the sidebar width. |
| [`dimensions-units.js`](../../engine/web/js/ui/components/knowledge-base/data/dimensions-units.js) | 178 | Knowledge-base section `dimensions-units` |
| [`component.js`](../../engine/web/js/ui/components/topbar/component.js) | 163 | _symbols:_ TopbarComponent |
| [`constants.js`](../../engine/web/js/ui/components/knowledge-base/data/constants.js) | 161 | Knowledge-base section `constants` |
| [`component.js`](../../engine/web/js/ui/panels/lagrangian-panel/component.js) | 155 | Build a small-multiple card for one Lagrangian term. |
| [`gpu-server-card.js`](../../engine/web/js/ui/components/gpu-server-card.js) | 154 | GPU Acceleration splash card (local dev only). |
| [`component.js`](../../engine/web/js/ui/panels/charts-panel/component.js) | 154 | _symbols:_ ChartsPanelComponent, initChartsPanel() |
| [`scale0.js`](../../engine/web/js/ui/panels/diagnostics-panel/descriptors/scale0.js) | 147 | Scale 0 diagnostics table descriptor. |
| [`stacked-area.js`](../../engine/web/js/ui/charts/stacked-area.js) | 146 | Stacked-area renderer for uPlot. |
| [`symbols.js`](../../engine/web/js/ui/components/knowledge-base/data/symbols.js) | 144 | Knowledge-base section `symbols` |
| [`template.js`](../../engine/web/js/ui/panels/scene-panel/template.js) | 139 | Scene panel template — 4 sections of curated render controls. |
| [`component.js`](../../engine/web/js/ui/components/keyboard-help/component.js) | 138 | Keyboard Help Overlay — press `?` to toggle a modal listing every keyboard shortcut the dashboard supports. |
| [`component.js`](../../engine/web/js/ui/components/loading-overlay/component.js) | 138 | _symbols:_ LoadingOverlayComponent |
| [`mobile-panel.js`](../../engine/web/js/ui/shell/mobile-panel.js) | 134 | MobilePanelController — touch swipe-to-dismiss and body scroll lock for the bottom-sheet panel on mobile (≤767px). |
| [`scales.js`](../../engine/web/js/ui/components/knowledge-base/data/scales.js) | 115 | Knowledge-base section `scales` |
| [`foundations.js`](../../engine/web/js/ui/components/knowledge-base/data/foundations.js) | 110 | Knowledge-base section `foundations` |
| [`template.js`](../../engine/web/js/ui/components/settings-modal/template.js) | 109 | _symbols:_ getSettingsModalTemplate() |
| [`template.js`](../../engine/web/js/ui/components/play-bar/template.js) | 103 | Play bar DOM template — a floating control strip at the bottom of the viewport that hosts the primary playback controls. |
| [`scale2.js`](../../engine/web/js/ui/panels/diagnostics-panel/descriptors/scale2.js) | 98 | Scale 2/3 diagnostics table descriptor (Atom / Molecule Engine). |
| [`chart-fullscreen.js`](../../engine/web/js/ui/charts/chart-fullscreen.js) | 97 | Shared fullscreen portal for chart cards. |
| [`component.js`](../../engine/web/js/ui/panels/diagnostics-panel/component.js) | 96 | DiagnosticsPanelComponent — composes scale-specific diagnostics tables from descriptors. |
| [`runtime.js`](../../engine/web/js/ui/components/knowledge-base/data/runtime.js) | 93 | Knowledge-base section `runtime` |
| [`chart-hover-tooltip.js`](../../engine/web/js/ui/charts/chart-hover-tooltip.js) | 90 | _symbols:_ ChartHoverTooltip, formatChartValue(), formatChartSample() |
| [`reader.js`](../../engine/web/js/ui/components/knowledge-base/reader.js) | 83 | KB entry reader — extracted from component.js so the Knowledge Base can share the generic SidebarLibraryComponent shell and only plug in its own reader render function. |
| [`panel-mount-state.js`](../../engine/web/js/ui/shell/panel-mount-state.js) | 81 | Single source of truth for panel-mount state. |
| [`reader.js`](../../engine/web/js/ui/components/faq/reader.js) | 79 | FAQ entry reader — 4 fixed sections with epistemic tag chips on ftdAngle bullets. |
| [`scale1.js`](../../engine/web/js/ui/panels/charts-panel/descriptors/scale1.js) | 78 | Scale 1 charts panel descriptor. |
| [`panel-registry.js`](../../engine/web/js/ui/scale-registry/panel-registry.js) | 78 | Shared shell panel registry. |
| [`sparkline.js`](../../engine/web/js/ui/charts/sparkline.js) | 71 | Sparkline — micro uPlot for table Trend cells and chart-chip previews. |
| [`scale0.js`](../../engine/web/js/ui/panels/charts-panel/descriptors/scale0.js) | 71 | Scale 0 charts panel descriptor. |
| [`panel-shell.js`](../../engine/web/js/ui/components/viewport-overlays/panel-shell.js) | 70 | Shared viewport overlay panel shell — scales 1–5 (Scale 0 uses s0-overlay-panel). |
| [`shell-template.js`](../../engine/web/js/ui/shell/shell-template.js) | 69 | Phase 0 template pass: annotate the current DOM with shell regions and create future mount roots without reparenting the existing markup yet. |
| [`scale2.js`](../../engine/web/js/ui/panels/charts-panel/descriptors/scale2.js) | 66 | Scale 2/3 (Atom / Molecule Engine) charts panel descriptor. |
| [`theme.js`](../../engine/web/js/ui/charts/theme.js) | 64 | Chart theme reader — converts CSS custom properties into a uPlot-shaped theme object. |
| [`template.js`](../../engine/web/js/ui/components/topbar/template.js) | 62 | _symbols:_ getTopbarInlineTemplate(), getTopbarActionButtons(), getAssistantSidebarTemplate() |
| [`template.js`](../../engine/web/js/ui/components/sidebar-library/template.js) | 60 | Shared template for library-style sidebars (KB, FAQ, any future ones). |
| [`breakpoint-service.js`](../../engine/web/js/ui/shell/breakpoint-service.js) | 60 | Observes viewport size and emits shell layout snapshots. |
| [`chart-card.js`](../../engine/web/js/ui/panels/charts-panel/chart-card.js) | 55 | ChartCard — wraps a chart descriptor entry in a .chart-card DOM node and owns the UPlotChart lifecycle. |
| [`component.js`](../../engine/web/js/ui/components/panel-resources/component.js) | 54 | _symbols:_ ensurePanelResources() |
| [`term-row.js`](../../engine/web/js/ui/panels/lagrangian-panel/term-row.js) | 52 | TermRow — renders the Lagrangian term-toggle row. |
| [`layout-state.js`](../../engine/web/js/ui/shell/layout-state.js) | 52 | UI shell layout state helpers. |
| [`render.js`](../../engine/web/js/ui/math-format/render.js) | 51 | renderMathInHtml — scan an already-escaped HTML string for LaTeX delimiters (\\( ... |
| [`component.js`](../../engine/web/js/ui/components/panel-dock/component.js) | 45 | _symbols:_ PanelDockComponent |
| [`diagnostics-template.js`](../../engine/web/js/ui/components/panel-resources/diagnostics-template.js) | 43 | _symbols:_ getDiagnosticsPanelTemplate() |
| [`formatters.js`](../../engine/web/js/ui/panels/diagnostics-panel/formatters.js) | 43 | Value formatters for the diagnostics table. |
| [`toolbar-registry.js`](../../engine/web/js/ui/scale-registry/toolbar-registry.js) | 41 | Registry for toolbar contributions. |
| [`scale0.js`](../../engine/web/js/ui/panels/lagrangian-panel/descriptors/scale0.js) | 38 | Scale 0 Lagrangian panel descriptor. |
| [`component.js`](../../engine/web/js/ui/components/knowledge-base/component.js) | 32 | Knowledge Base — thin factory around SidebarLibraryComponent. |
| [`template.js`](../../engine/web/js/ui/components/workspace-tabs/template.js) | 31 | _symbols:_ getWorkspaceTabsTemplate() |
| [`mount-registry.js`](../../engine/web/js/ui/shell/mount-registry.js) | 30 | Simple registry for shell mount points and named regions. |
| [`component.js`](../../engine/web/js/ui/components/faq/component.js) | 28 | FAQ — thin factory around SidebarLibraryComponent. |
| [`component.js`](../../engine/web/js/ui/components/settings-modal/component.js) | 24 | Settings Modal Component Mounts the settings modal into a container. |
| [`template.js`](../../engine/web/js/ui/components/loading-overlay/template.js) | 22 | _symbols:_ getLoadingOverlayTemplate() |
| [`component.js`](../../engine/web/js/ui/components/viewport-frame/component.js) | 22 | _symbols:_ ViewportFrameComponent |
| [`panel-visibility.js`](../../engine/web/js/ui/panels/panel-visibility.js) | 20 | Shared panel-visibility predicate — SPEC_SCALE0_PERF_TELEMETRY_PANELS §6.4. |
| [`template.js`](../../engine/web/js/ui/components/panel-dock/template.js) | 15 | _symbols:_ getPanelDockShellTemplate() |
| [`component.js`](../../engine/web/js/ui/components/workspace-tabs/component.js) | 15 | _symbols:_ WorkspaceTabsComponent |
| [`register-scale-ui.js`](../../engine/web/js/ui/scale-registry/register-scale-ui.js) | 14 | Shared shell UI registry bundle. |
| [`template.js`](../../engine/web/js/ui/panels/lagrangian-panel/template.js) | 13 | _symbols:_ getLagrangianPanelTemplate() |
| [`register-legacy-toolbar-ui.js`](../../engine/web/js/ui/shell/register-legacy-toolbar-ui.js) | 13 | _symbols:_ registerLegacyToolbarUi() |
| [`overlay-registry.js`](../../engine/web/js/ui/scale-registry/overlay-registry.js) | 12 | Placeholder registry seam for viewport overlays. |
| [`template.js`](../../engine/web/js/ui/panels/charts-panel/template.js) | 10 | Charts panel shell — chip picker + grid. |
| [`index.js`](../../engine/web/js/ui/panels/index.js) | 5 |  |
| [`template.js`](../../engine/web/js/ui/components/viewport-frame/template.js) | 3 | _symbols:_ getViewportFrameTemplate() |
| [`uPlot.iife.min.js`](../../engine/web/js/ui/charts/vendor/uPlot.iife.min.js) | 2 | ! https://github.com/leeoniya/uPlot (v1.6.30) |

### `web/viewport`  (19 files, 8,653 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`viewport.js`](../../engine/web/js/viewport.js) | 1302 | @file viewport.js @brief Three.js 3D Viewport — renders particles and fields from the simulation bridge. |
| [`particle-renderer.js`](../../engine/web/js/viewport/particle-renderer.js) | 1000 | @file engine/web/js/viewport/particle-renderer.js @purpose Owns particle positions, trails, velocity vectors, per-particle force vectors for the Scale-0 lattice dashboard. |
| [`field-em-renderer.js`](../../engine/web/js/viewport/field-em-renderer.js) | 858 | fieldEmMethods — ViewportFieldRenderer mixin (heatmap / EM / phase / state). |
| [`molecular-renderer.js`](../../engine/web/js/viewport/molecular-renderer.js) | 816 | Molecular renderer — Scale 2 (atoms) and Scale 3 (molecules). |
| [`topology-sheet-renderer.js`](../../engine/web/js/viewport/topology-sheet-renderer.js) | 592 | viewport/topology-sheet-renderer.js — deformable rubber-sheet visualization Extracted from viewport.js as refactoring-analyst ticket RF-1 of the post-modularization cleanup (see engine/web/docs/IND... |
| [`flux-renderer.js`](../../engine/web/js/viewport/flux-renderer.js) | 576 | @file engine/web/js/viewport/flux-renderer.js @purpose Owns flux volume, flux streamlines for the Scale-0 lattice dashboard. |
| [`field-force-renderer.js`](../../engine/web/js/viewport/field-force-renderer.js) | 558 | fieldForceMethods — ViewportFieldRenderer mixin (EM/gravity/strong/weak force viz). |
| [`scene-core.js`](../../engine/web/js/viewport/scene-core.js) | 531 | @file engine/web/js/viewport/scene-core.js @purpose Owns scene-level rendering infrastructure for the Scale-0 dashboard: boundary wireframe, axis indicators, post-processing pipeline (bloom), camer... |
| [`field-quantum-renderer.js`](../../engine/web/js/viewport/field-quantum-renderer.js) | 525 | fieldQuantumMethods — ViewportFieldRenderer mixin (dual / chirality / quantum / entropy). |
| [`field-topology-renderer.js`](../../engine/web/js/viewport/field-topology-renderer.js) | 487 | fieldTopologyMethods — ViewportFieldRenderer mixin (halo / damping / genesis / strings). |
| [`field-renderer.js`](../../engine/web/js/viewport/field-renderer.js) | 276 | ViewportFieldRenderer — Scale-0 field overlay façade. |
| [`boundary-geometry.js`](../../engine/web/js/viewport/boundary-geometry.js) | 274 | viewport/boundary-geometry.js — Three.js boundary wireframe builders Extracted from viewport.js as refactoring-analyst ticket RF-4 of the post-modularization cleanup (see engine/web/docs/INDEX.md). |
| [`color-ramps.js`](../../engine/web/js/viewport/color-ramps.js) | 265 | Color ramps for Scale 0 viewport overlays. |
| [`spin-arrow-manager.js`](../../engine/web/js/viewport/spin-arrow-manager.js) | 233 | Spin-Arrow Manager — Three.js primitive that follows tracked particles and visualizes their spin orientation + precession rate. |
| [`shaders.js`](../../engine/web/js/viewport/shaders.js) | 149 | Centralized Shaders for FTD Web Frontend ──────────────────────────────────────────────────────────────────── Houses shared GLSL shader strings to ensure DRY compliance and enable global shader opt... |
| [`field-renderer-shared.js`](../../engine/web/js/viewport/field-renderer-shared.js) | 63 | Shared helpers/constants for FieldRenderer mixins. |
| [`mesh-factory.js`](../../engine/web/js/viewport/mesh-factory.js) | 61 | @file engine/web/js/viewport/mesh-factory.js @purpose Utility factory functions to build Three.js buffer geometries and line meshes. |
| [`field-renderer-core.js`](../../engine/web/js/viewport/field-renderer-core.js) | 46 | fieldCoreMethods — ViewportFieldRenderer mixin (clip / center sync). |
| [`constants.js`](../../engine/web/js/viewport/constants.js) | 41 | @file engine/web/js/viewport/constants.js @purpose Shared pre-allocated-buffer sizes for the viewport renderer cluster. |

### `web/bridge`  (24 files, 7,646 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`mock-atom-engine.js`](../../engine/web/js/bridge/mock-atom-engine.js) | 1261 | Scale-2 Atom Engine (AE) — MockBridge side only. |
| [`wasm-bridge.js`](../../engine/web/js/bridge/wasm-bridge.js) | 1057 | @file engine/web/js/bridge/wasm-bridge.js @purpose Thin wrapper around the compiled C++/WASM physics engine (engine/wasm/ftd_wasm.cpp). |
| [`wasm-bridge-proxy.js`](../../engine/web/js/bridge/wasm-bridge-proxy.js) | 738 | Main-thread proxy for the Scale-0 WASM physics Web Worker. |
| [`cosmic-physics.js`](../../engine/web/js/bridge/cosmic-physics.js) | 563 | Cosmic scale-5 force kernel. |
| [`wasm-bridge.worker.js`](../../engine/web/js/bridge/wasm-bridge.worker.js) | 546 | Scale-0 WASM physics Web Worker. |
| [`galaxies.js`](../../engine/web/js/bridge/cosmic-scenarios/galaxies.js) | 539 | Cosmic scale-5 scenarios — galaxy-family. |
| [`native-particle-engine.js`](../../engine/web/js/bridge/native-particle-engine.js) | 492 | Scale-1 Particle Engine — native C++/WASM adapter. |
| [`mock-scale5.js`](../../engine/web/js/bridge/mock-scale5.js) | 450 | CosmicMockBridge — JS-only N-body simulation for cosmic scale (Scale 5). |
| [`cosmic-postupdates.js`](../../engine/web/js/bridge/cosmic-postupdates.js) | 351 | Cosmic scale-5 post-integration updates. |
| [`mock-scale4.js`](../../engine/web/js/bridge/mock-scale4.js) | 288 | PlanetaryMockBridge — JS-only N-body simulation for Planetary scale (Scale 4). |
| [`exotic.js`](../../engine/web/js/bridge/cosmic-scenarios/exotic.js) | 243 | Cosmic scale-5 scenarios — exotic / lifecycle family. |
| [`bridge-contract.js`](../../engine/web/js/bridge/bridge-contract.js) | 240 | Bridge contract — the surface every Scale-0 bridge must implement. |
| [`boundary.js`](../../engine/web/js/bridge/boundary.js) | 198 | Boundary shape geometry for the FTD particle engine. |
| [`ws-binary-codec.js`](../../engine/web/js/bridge/ws-binary-codec.js) | 121 | _symbols:_ decodeNativeBinaryFrame(), FLUX_VOLUME_AXIS_SAMPLES, FIELD_SAMPLE_KINDS, FIELD_SAMPLE_KIND_CODES |
| [`ws-scale-fallback-facade.js`](../../engine/web/js/bridge/ws-scale-fallback-facade.js) | 115 | _symbols:_ WebSocketScaleFallbackFacade |
| [`pe-catalog-map.js`](../../engine/web/js/bridge/pe-catalog-map.js) | 70 | Catalog → engine-field mapping helpers for Scale-1 particle injection. |
| [`mock-atom-valence.js`](../../engine/web/js/bridge/mock-atom-valence.js) | 64 | Valence / bond-order helpers for Scale-2 mock atom engine. |
| [`sampler-want-set.js`](../../engine/web/js/bridge/sampler-want-set.js) | 64 | Multi-owner sampler-want union for the WASM worker proxy. |
| [`scale0.js`](../../engine/web/js/bridge/capabilities/scale0.js) | 60 | @file engine/web/js/bridge/capabilities/scale0.js @purpose Scale-0 (lattice/substrate) capability factory. |
| [`index.js`](../../engine/web/js/bridge/cosmic-scenarios/index.js) | 55 | Cosmic scale-5 scenario dispatcher. |
| [`scale1.js`](../../engine/web/js/bridge/capabilities/scale1.js) | 37 | @file engine/web/js/bridge/capabilities/scale1.js @purpose Scale-1 (particle engine) capability factory. |
| [`install.js`](../../engine/web/js/bridge/capabilities/install.js) | 36 | @file engine/web/js/bridge/capabilities/install.js @purpose Installs the lazy `bridge.capabilities` getter on WasmBridge and WebSocketBridge prototypes so consumers see one symmetric surface (CONTR... |
| [`sampler-registry.classic.js`](../../engine/web/js/bridge/sampler-registry.classic.js) | 33 | Classic-worker sampler + toggle-requires registry. |
| [`scale2.js`](../../engine/web/js/bridge/capabilities/scale2.js) | 25 | @file engine/web/js/bridge/capabilities/scale2.js @purpose Scale-2 (atom engine) capability factory. |

### `src/scenarios`  (7 files, 3,387 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`s0_seed.cpp`](../../engine/src/scenarios/s0_seed.cpp) | 998 | ========================================================================== engine/src/scenarios/s0_seed.cpp Group: s0-seed-* (50 scenarios) Canonical seed implementation; the former JS mirror is ar... |
| [`_helpers.h`](../../engine/src/scenarios/_helpers.h) | 756 | ========================================================================== engine/src/scenarios/_helpers.h Private (non-installed) helper header shared by the split scenario group files (flux.cpp,... |
| [`flux.cpp`](../../engine/src/scenarios/flux.cpp) | 525 | ========================================================================== engine/src/scenarios/flux.cpp Group: flux-* (22 scenarios) Canonical seed implementation; the former JS mirror is archived. |
| [`vacuum.cpp`](../../engine/src/scenarios/vacuum.cpp) | 383 | ========================================================================== engine/src/scenarios/vacuum.cpp Group: s0-vacuum-* (15 scenarios) Canonical seed implementation; the former JS mirror is a... |
| [`s0_field.cpp`](../../engine/src/scenarios/s0_field.cpp) | 380 | ========================================================================== engine/src/scenarios/s0_field.cpp Group: s0-field-* (9 scenarios) Canonical seed implementation; the former JS mirror is a... |
| [`quantum.cpp`](../../engine/src/scenarios/quantum.cpp) | 229 | ========================================================================== engine/src/scenarios/quantum.cpp Group: quantum-* (8 scenarios) Canonical seed implementation; the former JS mirror is arc... |
| [`light.cpp`](../../engine/src/scenarios/light.cpp) | 116 | ========================================================================== engine/src/scenarios/light.cpp Group: light-* (4 scenarios) Canonical seed implementation; the former JS mirror is archived. |

### `wasm`  (5 files, 2,706 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`ftd_wasm.cpp`](../../engine/wasm/ftd_wasm.cpp) | 1374 | @file ftd_wasm.cpp @brief Emscripten Embind bindings for the FTD engine — shared helpers. |
| [`bindings_particle.cpp`](../../engine/wasm/bindings_particle.cpp) | 461 | @file bindings_particle.cpp @brief Embind bindings for ParticleEngine (Scale 1). |
| [`bindings_render_bridge.cpp`](../../engine/wasm/bindings_render_bridge.cpp) | 434 | @file bindings_render_bridge.cpp @brief Embind bindings for RenderBridge (Scale 0 — voxel lattice engine). |
| [`bindings_atom.cpp`](../../engine/wasm/bindings_atom.cpp) | 345 | @file bindings_atom.cpp @brief Embind bindings for AtomEngine (Scale 2). |
| [`bindings_internal.h`](../../engine/wasm/bindings_internal.h) | 92 | @file bindings_internal.h @brief Shared helpers exposed across the split Embind binding TUs. |

### `web/scale1`  (14 files, 2,077 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`controller.js`](../../engine/web/js/scales/scale1/controller.js) | 596 | Scale 1 (Particles) Controller — native-engine edition. |
| [`pe-cloud-expander.js`](../../engine/web/js/scales/scale1/pe-cloud-expander.js) | 402 | Scale 1 — PE Cloud Expander ──────────────────────────────────────────────────────────────────── Fixed-boundary point cloud per particle. |
| [`promotion.js`](../../engine/web/js/scales/scale1/promotion.js) | 346 | Scale-0 → Scale-1 promotion pipeline ("⤴ Scale up"). |
| [`scenario-registry.js`](../../engine/web/js/scales/scale1/scenario-registry.js) | 249 | Scale-1 scenario registry. |
| [`pe-controls.js`](../../engine/web/js/scales/scale1/ui/controls/pe-controls.js) | 124 | Scale 1 — Particle Engine Controls Card (native-engine edition). |
| [`template.js`](../../engine/web/js/scales/scale1/ui/overlays/template.js) | 93 | Scale 1 Viewport Overlay — particle engine dynamics (grouped by physical role) |
| [`store.js`](../../engine/web/js/scales/scale1/state/store.js) | 75 | Scale-1 state store. |
| [`overlay-billboards.js`](../../engine/web/js/scales/scale1/overlay-billboards.js) | 58 | engine/web/js/scales/scale1/overlay-billboards.js |
| [`mass-comparison.js`](../../engine/web/js/scales/scale1/telemetry/mass-comparison.js) | 29 | Pairs a promoted cluster seed's mass (N·K_B, the physics-bearing convention `phase_forces_integrate_clusters` uses) against the sum of its constituent voxels' scale-bridge masses (max(density, K_B)... |
| [`component.js`](../../engine/web/js/scales/scale1/ui/controls/component.js) | 29 | Scale 1 Controls Component Mounts the Scale 1 (Particle Engine) control card into the controls panel. |
| [`orbit-period.js`](../../engine/web/js/scales/scale1/telemetry/orbit-period.js) | 27 | Bound-orbit period proxy from 2-body separation history: record the starting separation, then report the tick delta the first time separation returns within `tolerancePct` of that starting value. |
| [`template.js`](../../engine/web/js/scales/scale1/ui/toolbar/template.js) | 21 | Scale-1 toolbar template. |
| [`component.js`](../../engine/web/js/scales/scale1/ui/toolbar/component.js) | 17 | _symbols:_ createScale1ScenarioToolbarGroup() |
| [`register-scale1-ui.js`](../../engine/web/js/scales/scale1/ui/register-scale1-ui.js) | 11 | _symbols:_ registerScale1ToolbarUI() |

### `web/scale2`  (12 files, 2,074 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`controller.js`](../../engine/web/js/scales/scale2/controller.js) | 772 | Scale 2 (Atoms) Controller ──────────────────────────────────────────────────────────────────── Owns the Atom Engine (AE) frame loop, force decomposition rendering, element legend building, orbital... |
| [`scenarios.js`](../../engine/web/js/scales/scale2/scenarios.js) | 522 | Scale 2 — AE Scenario Loader ──────────────────────────────────────────────────────────────────── Extracted verbatim from scales/scale2/controller.js (ticket S2-1). |
| [`scenario-registry.js`](../../engine/web/js/scales/scale2/scenario-registry.js) | 280 | Scale 2 — AE scenario registry (canonical metadata + select population). |
| [`template.js`](../../engine/web/js/scales/scale2/ui/overlays/template.js) | 135 | Scale 2/3 Viewport Overlay — atom/molecule MD + QM structure visualization. |
| [`ae-controls.js`](../../engine/web/js/scales/scale2/ui/controls/ae-controls.js) | 113 | Scale 2 — Atom Engine control cards (split by concern). |
| [`ui-bindings.js`](../../engine/web/js/scales/scale2/ui-bindings.js) | 77 | Scale 2 — AE UI Bindings ──────────────────────────────────────────────────────────────────── Houses the DOM-coupled helpers that sync AE physics parameters and toggles between the Scale 2 control... |
| [`binding-energy-chart.js`](../../engine/web/js/scales/scale2/ui/binding-energy-chart.js) | 76 | Nuclear binding-energy-per-nucleon curve — the classic B/A vs mass-number plot, peaking at Fe-56. |
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

### `src/phases`  (4 files, 1,606 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`phase_write.cpp`](../../engine/src/render_bridge_phases/phase_write.cpp) | 555 | @file engine/src/render_bridge_phases/phase_write.cpp @purpose Implementation of phase_write decomposition (Phase 4a, 2026-04-27). |
| [`phase_forces.cpp`](../../engine/src/render_bridge_phases/phase_forces.cpp) | 411 | @file engine/src/render_bridge_phases/phase_forces.cpp @purpose Implementation of phase_forces decomposition (Phase 4b, 2026-04-27). |
| [`phase_movement.cpp`](../../engine/src/render_bridge_phases/phase_movement.cpp) | 410 | @file engine/src/render_bridge_phases/phase_movement.cpp @purpose Implementation of phase_movement decomposition (Phase 4c, 2026-04-27). |
| [`phase_read.cpp`](../../engine/src/render_bridge_phases/phase_read.cpp) | 230 | @file engine/src/render_bridge_phases/phase_read.cpp @purpose Implementation of phase_read decomposition (Phase 4c, 2026-04-27). |

### `vendor`  (12 files, 1,425 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`quarto.js`](../../engine/VISUAL_GUIDE_files/libs/quarto-html/quarto.js) | 847 |  |
| [`raf-coordinator.js`](../../engine/web/js/lib/raf-coordinator.js) | 156 | Single rAF coordinator for all dashboard panels. |
| [`axe-check.js`](../../engine/VISUAL_GUIDE_files/libs/quarto-html/axe/axe-check.js) | 145 | _symbols:_ QuartoAxeReporter, QuartoAxeJsonReporter, QuartoAxeConsoleReporter, QuartoAxeDocumentReporter |
| [`tabsets.js`](../../engine/VISUAL_GUIDE_files/libs/quarto-html/tabsets/tabsets.js) | 95 | grouped tabsets |
| [`origin-policy.js`](../../engine/web/js/lib/origin-policy.js) | 70 | Loopback-only policy for the dashboard's native WebSocket probe and for the C++ handshake's Origin allowlist (kept in sync with ws_origin_allowed). |
| [`ftv2.js`](../../engine/web/js/lib/ftv2.js) | 56 | Compact native flux-volume frames (FTV2). |
| [`visual-sample-grid.js`](../../engine/web/js/lib/visual-sample-grid.js) | 25 | JS twin of engine/include/ftd/visual_sample_grid.h. |
| [`anchor.min.js`](../../engine/VISUAL_GUIDE_files/libs/quarto-html/anchor.min.js) | 9 |  |
| [`bootstrap.min.js`](../../engine/VISUAL_GUIDE_files/libs/bootstrap/bootstrap.min.js) | 7 |  |
| [`clipboard.min.js`](../../engine/VISUAL_GUIDE_files/libs/clipboard/clipboard.min.js) | 7 |  |
| [`popper.min.js`](../../engine/VISUAL_GUIDE_files/libs/quarto-html/popper.min.js) | 6 |  |
| [`tippy.umd.min.js`](../../engine/VISUAL_GUIDE_files/libs/quarto-html/tippy.umd.min.js) | 2 |  |

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

### `web/config`  (4 files, 1,373 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`toggles.js`](../../engine/web/js/config/toggles.js) | 675 | Toggle Configuration — Single source of truth for all scale toggles. |
| [`scenarios.js`](../../engine/web/js/config/scenarios.js) | 377 | Scenario Descriptions — Metadata for scenario dropdowns and info panels. |
| [`exoplanet-seeds.js`](../../engine/web/js/config/exoplanet-seeds.js) | 287 | _symbols:_ EXOPLANET_SEEDS |
| [`perf-flags.js`](../../engine/web/js/config/perf-flags.js) | 34 | Perf rollout flags — SPEC_SCALE0_PERF_TELEMETRY_PANELS.md §10. |

### `web/inspector`  (9 files, 1,291 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`lattice.js`](../../engine/web/js/inspector/scales/lattice.js) | 353 | _symbols:_ handleLatticeClick(), showLatticeInspector(), hideLatticeInspector(), updateLatticeFields() |
| [`atoms.js`](../../engine/web/js/inspector/scales/atoms.js) | 295 | _symbols:_ handleAEClick(), showAEInspector(), hideAEInspector(), updateAEFields() |
| [`dom-bindings.js`](../../engine/web/js/inspector/dom-bindings.js) | 166 | _symbols:_ collectInspectorDom() |
| [`planetary.js`](../../engine/web/js/inspector/scales/planetary.js) | 109 | _symbols:_ classifyBiome(), handlePlanetaryClick(), showPlanetaryInspector(), hidePlanetaryInspector() |
| [`particles.js`](../../engine/web/js/inspector/scales/particles.js) | 92 | _symbols:_ handlePEClick(), showPEInspector(), hidePEInspector(), updatePEFields() |
| [`cosmic.js`](../../engine/web/js/inspector/scales/cosmic.js) | 86 | _symbols:_ handleCosmicClick(), showCosmicInspector(), hideCosmicInspector(), updateCosmicFields() |
| [`chrome.js`](../../engine/web/js/inspector/chrome.js) | 76 | _symbols:_ resetInspectorSelection(), hasInspectorSelection(), getInspectorModeCopy(), getInspectorSelectionSummary() |
| [`pointer-controller.js`](../../engine/web/js/inspector/pointer-controller.js) | 63 | _symbols:_ bindInspectorPointerControls() |
| [`app-runtime.js`](../../engine/web/js/inspector/app-runtime.js) | 51 | _symbols:_ createInspectorAppRuntime() |

### `tools`  (4 files, 1,199 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`audit_ontic_phase0.py`](../../engine/tools/audit_ontic_phase0.py) | 630 | Phase 0 Ontic Derivation Chain Audit ===================================== Independent verification of every constant in engine/include/ftd/ontic.h using mpmath high-precision arithmetic. |
| [`build_file_manifest.py`](../../engine/tools/build_file_manifest.py) | 329 | Build a machine-readable manifest of every tracked code file in engine/. |
| [`print_ontic.py`](../../engine/tools/print_ontic.py) | 189 | Print the complete ontic derivation chain to 12 decimal places. |
| [`audit_reexports.py`](../../engine/tools/audit_reexports.py) | 51 | Item 0.21: Verify constants.h re-exports match ontic.h (no stale overrides) |

### `src/atom`  (3 files, 844 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`atom_forces.cpp`](../../engine/src/atom/atom_forces.cpp) | 629 | AtomEngine force computation. |
| [`atom_thermostat.cpp`](../../engine/src/atom/atom_thermostat.cpp) | 142 | AtomEngine velocity post-processing: speed limit, damping, Berendsen thermostat, and per-atom dipole moment computation. |
| [`atom_bonding.cpp`](../../engine/src/atom/atom_bonding.cpp) | 73 | AtomEngine dynamic bond formation / breaking. |

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

### `web/scale4`  (5 files, 427 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`controller.js`](../../engine/web/js/scales/scale4/controller.js) | 346 | Scale 4 (Planetary) Controller Owns the Scale 4 N-Body physics loop, scenario loading, and UI list mapping. |
| [`template.js`](../../engine/web/js/scales/scale4/ui/overlays/template.js) | 31 | Scale 4 Viewport Overlay — planetary visualization controls |
| [`template.js`](../../engine/web/js/scales/scale4/ui/toolbar/template.js) | 28 | _symbols:_ getScale4ScenarioToolbarTemplate() |
| [`register-scale4-ui.js`](../../engine/web/js/scales/scale4/ui/register-scale4-ui.js) | 11 | _symbols:_ registerScale4ToolbarUI() |
| [`component.js`](../../engine/web/js/scales/scale4/ui/toolbar/component.js) | 11 | _symbols:_ createScale4ScenarioToolbarGroup() |

### `web/other`  (6 files, 408 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`serve.py`](../../engine/web/serve.py) | 228 | Dev http server for engine/web/ that disables browser caching. |
| [`coi-serviceworker.js`](../../engine/web/coi-serviceworker.js) | 137 | ! coi-serviceworker v0.1.7 - Guido Zuidhof and contributors, licensed under MIT |
| [`wasm-threads-proof.worker.js`](../../engine/web/wasm-threads-proof.worker.js) | 37 | Phase-1 off-thread proof: host ftd_core_mt in a worker at pool=1 (pure serial, no thread spawns). |
| [`ftd_core.js`](../../engine/web/wasm/ftd_core.js) | 2 |  |
| [`ftd_core64.js`](../../engine/web/wasm/ftd_core64.js) | 2 |  |
| [`ftd_core_mt.js`](../../engine/web/wasm/ftd_core_mt.js) | 2 |  |

### `web/scale5`  (5 files, 379 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`controller.js`](../../engine/web/js/scales/scale5/controller.js) | 281 | Scale 5 — Cosmic Controller Manages the cosmic scale: N-body gravitational simulation with Hubble expansion, dark matter, and cosmological diagnostics. |
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

### `web/scales-shared`  (1 files, 253 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`scale-utils.js`](../../engine/web/js/scales/scale-utils.js) | 253 | Scale Utilities -- Shared helpers for scale controllers ──────────────────────────────────────────────────────────────────── Common formatting, throttling, and DOM-update utilities extracted from a... |

### `web/telemetry`  (4 files, 239 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`demand.js`](../../engine/web/js/telemetry/demand.js) | 153 | Scale-0 telemetry demand gating — decides which expensive hub collectors run. |
| [`scale0-read.js`](../../engine/web/js/telemetry/scale0-read.js) | 37 | Scale-0 telemetry read helpers — prefer hub snapshots, fall back to bridge. |
| [`scale0-grid-channels.js`](../../engine/web/js/telemetry/registry/scale0-grid-channels.js) | 31 | Scale-0 telemetry grid channel registry. |
| [`index.js`](../../engine/web/js/telemetry/index.js) | 18 | Telemetry module barrel — hub (single write path) + demand gating + registries. |

### `web/scale6`  (1 files, 187 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`controller.js`](../../engine/web/js/scales/scale6/controller.js) | 187 | Scale 6 (Meta / Existential Unit) Controller ──────────────────────────────────────────────────────────────────── Reconnects the previously-orphaned MetaUnit module (the 27-site Moore neighborhood... |

### `web/scale23`  (3 files, 70 LOC)

| File | LOC | Purpose |
|---|--:|---|
| [`template.js`](../../engine/web/js/scales/scale23/ui/toolbar/template.js) | 36 | _symbols:_ getScale23VisualToolbarTemplate(), getScale23ForceToolbarTemplate() |
| [`register-scale23-ui.js`](../../engine/web/js/scales/scale23/ui/register-scale23-ui.js) | 19 | _symbols:_ registerScale23ToolbarUI() |
| [`component.js`](../../engine/web/js/scales/scale23/ui/toolbar/component.js) | 15 | _symbols:_ createScale23VisualToolbarGroup(), createScale23ForceToolbarGroup() |

