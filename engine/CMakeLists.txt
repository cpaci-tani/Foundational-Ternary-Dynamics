cmake_minimum_required(VERSION 3.18)
project(ftd_engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenMP for multi-threaded tick phases (not available under Emscripten)
if(NOT EMSCRIPTEN)
    find_package(OpenMP)
endif()

# CUDA GPU acceleration (optional — requires CUDA Toolkit 12.0+)
option(FTD_ENABLE_CUDA "Enable CUDA GPU acceleration (requires NVIDIA GPU)" OFF)
if(FTD_ENABLE_CUDA AND NOT EMSCRIPTEN)
    # Allow VS 2025 (v18) — CUDA 13.0 officially supports up to VS 2022,
    # but VS 2025 works with this flag. Must be set BEFORE enable_language.
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --allow-unsupported-compiler")
    enable_language(CUDA)
    find_package(CUDAToolkit REQUIRED)
    set(CMAKE_CUDA_STANDARD 17)
    # Default: Ada (89: RTX 4060-4090, 5070-5080) + Blackwell (120: RTX 5090)
    # Override: cmake -DCMAKE_CUDA_ARCHITECTURES="86;89;120" for Ampere support
    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES OR CMAKE_CUDA_ARCHITECTURES STREQUAL "")
        set(CMAKE_CUDA_ARCHITECTURES "89;120")
    endif()
    message(STATUS "CUDA enabled — architectures: ${CMAKE_CUDA_ARCHITECTURES}")
    add_compile_definitions(FTD_ENABLE_CUDA)
    add_subdirectory(cuda)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library
add_library(ftd_core
    src/lattice.cpp
    src/render_bridge.cpp
    src/lagrangian.cpp
    src/particle_engine.cpp
    src/atom_engine.cpp
    src/scale_bridge.cpp
)
target_include_directories(ftd_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
if(OpenMP_CXX_FOUND)
    target_link_libraries(ftd_core PUBLIC OpenMP::OpenMP_CXX)
endif()
if(FTD_ENABLE_CUDA)
    target_link_libraries(ftd_core PUBLIC ftd_cuda)
endif()

# Main simulation executable (native builds only)
if(NOT EMSCRIPTEN)
add_executable(ftd_sim src/main.cpp)
target_link_libraries(ftd_sim ftd_core)
endif()

# WebSocket server executable (native builds only — bridges engine to web dashboard)
if(NOT EMSCRIPTEN)
add_executable(ws_server src/ws_server.cpp)
target_link_libraries(ws_server ftd_core)
if(WIN32)
    target_link_libraries(ws_server ws2_32)
endif()
endif()

# ============================================================================
# Unit Tests (native builds only)
# ============================================================================
if(NOT EMSCRIPTEN)

# Foundational
add_executable(test_constants tests/test_constants.cpp)
target_link_libraries(test_constants ftd_core)

add_executable(test_lorentz tests/test_lorentz.cpp)
target_link_libraries(test_lorentz ftd_core)

add_executable(test_lattice tests/test_lattice.cpp)
target_link_libraries(test_lattice ftd_core)

# Lagrangian verification
add_executable(test_born_infeld tests/test_born_infeld.cpp)
target_link_libraries(test_born_infeld ftd_core)

add_executable(test_energy tests/test_energy.cpp)
target_link_libraries(test_energy ftd_core)

add_executable(test_gauss tests/test_gauss.cpp)
target_link_libraries(test_gauss ftd_core)

add_executable(test_stress_energy tests/test_stress_energy.cpp)
target_link_libraries(test_stress_energy ftd_core)

add_executable(test_thermodynamics tests/test_thermodynamics.cpp)
target_link_libraries(test_thermodynamics ftd_core)

add_executable(test_lagrangian tests/test_lagrangian.cpp)
target_link_libraries(test_lagrangian ftd_core)

# Ontic physics
add_executable(test_ontic_chain tests/test_ontic_chain.cpp)
target_link_libraries(test_ontic_chain ftd_core)

# Dual-substrate (Paper: "The Algebraic Identity of Two Substrates")
add_executable(test_dual_substrate tests/test_dual_substrate.cpp)
target_link_libraries(test_dual_substrate ftd_core)

add_executable(test_genesis tests/test_genesis.cpp)
target_link_libraries(test_genesis ftd_core)

add_executable(test_gravity_dynamics tests/test_gravity_dynamics.cpp)
target_link_libraries(test_gravity_dynamics ftd_core)

add_executable(test_annihilation tests/test_annihilation.cpp)
target_link_libraries(test_annihilation ftd_core)

add_executable(test_annihilation_conservation tests/test_annihilation_conservation.cpp)
target_link_libraries(test_annihilation_conservation ftd_core)

add_executable(test_wave_collapse tests/test_wave_collapse.cpp)
target_link_libraries(test_wave_collapse ftd_core)

# Phase 1: Lagrangian implications
add_executable(test_wave_speed tests/test_wave_speed.cpp)
target_link_libraries(test_wave_speed ftd_core)

add_executable(test_interference tests/test_interference.cpp)
target_link_libraries(test_interference ftd_core)

add_executable(test_gauge tests/test_gauge.cpp)
target_link_libraries(test_gauge ftd_core)

add_executable(test_polarization tests/test_polarization.cpp)
target_link_libraries(test_polarization ftd_core)

add_executable(test_momentum tests/test_momentum.cpp)
target_link_libraries(test_momentum ftd_core)

add_executable(test_magnetic tests/test_magnetic.cpp)
target_link_libraries(test_magnetic ftd_core)

add_executable(test_flux_mediated tests/test_flux_mediated.cpp)
target_link_libraries(test_flux_mediated ftd_core)

# Phase 2: SM sectors (active subset)
add_executable(test_entanglement tests/test_entanglement.cpp)
target_link_libraries(test_entanglement ftd_core)

# Phase 4: Variational forces
add_executable(test_variational_coulomb tests/test_variational_coulomb.cpp)
target_link_libraries(test_variational_coulomb ftd_core)

add_executable(test_magnetic_lagrangian tests/test_magnetic_lagrangian.cpp)
target_link_libraries(test_magnetic_lagrangian ftd_core)

add_executable(test_dissipation tests/test_dissipation.cpp)
target_link_libraries(test_dissipation ftd_core)

# Phase 5: Free dynamics
add_executable(test_portable_field tests/test_portable_field.cpp)
target_link_libraries(test_portable_field ftd_core)

add_executable(test_particle_lifetime tests/test_particle_lifetime.cpp)
target_link_libraries(test_particle_lifetime ftd_core)

# Phase 7: Vortex formation
add_executable(test_vortex tests/test_vortex.cpp)
target_link_libraries(test_vortex ftd_core)

# Operators and dynamics
add_executable(test_voxel_properties tests/test_voxel_properties.cpp)
target_link_libraries(test_voxel_properties ftd_core)

add_executable(test_lattice_operators tests/test_lattice_operators.cpp)
target_link_libraries(test_lattice_operators ftd_core)

add_executable(test_discrete_operators tests/test_discrete_operators.cpp)
target_link_libraries(test_discrete_operators ftd_core)

add_executable(test_bridge_dynamics tests/test_bridge_dynamics.cpp)
target_link_libraries(test_bridge_dynamics ftd_core)

# Maxwell equation recovery
add_executable(test_maxwell tests/test_maxwell.cpp)
target_link_libraries(test_maxwell ftd_core)

# CSV export
add_executable(test_csv_export tests/test_csv_export.cpp)
target_link_libraries(test_csv_export ftd_core)

# Logic-first engine
add_executable(test_logic_engine tests/test_logic_engine.cpp)
target_link_libraries(test_logic_engine ftd_core)

# Phase 3: Poisson-based Coulomb
add_executable(test_poisson_coulomb tests/test_poisson_coulomb.cpp)
target_link_libraries(test_poisson_coulomb ftd_core)

add_executable(test_energy_tracking tests/test_energy_tracking.cpp)
target_link_libraries(test_energy_tracking ftd_core)

# Phase 4: Energy conservation
add_executable(test_energy_conservation tests/test_energy_conservation.cpp)
target_link_libraries(test_energy_conservation ftd_core)

# Phase 6: Flux-aggregate particles
add_executable(test_selffield_profile tests/test_selffield_profile.cpp)
target_link_libraries(test_selffield_profile ftd_core)

add_executable(test_wavepacket tests/test_wavepacket.cpp)
target_link_libraries(test_wavepacket ftd_core)

# Phase 7: Multi-scale physics engine
add_executable(test_particle_engine tests/test_particle_engine.cpp)
target_link_libraries(test_particle_engine ftd_core)

add_executable(test_scale_bridge tests/test_scale_bridge.cpp)
target_link_libraries(test_scale_bridge ftd_core)

add_executable(test_hydrogen_scale1 tests/test_hydrogen_scale1.cpp)
target_link_libraries(test_hydrogen_scale1 ftd_core)

# Phase 8: Atom engine (Scale 2)
add_executable(test_atom_engine tests/test_atom_engine.cpp)
target_link_libraries(test_atom_engine ftd_core)

add_executable(test_atom_scale_bridge tests/test_atom_scale_bridge.cpp)
target_link_libraries(test_atom_scale_bridge ftd_core)

add_executable(test_multiscale_bridge tests/test_multiscale_bridge.cpp)
target_link_libraries(test_multiscale_bridge ftd_core)

add_executable(test_em_fields tests/test_em_fields.cpp)
target_link_libraries(test_em_fields ftd_core)

add_executable(test_gauss_convergence tests/test_gauss_convergence.cpp)
target_link_libraries(test_gauss_convergence ftd_core)

add_executable(test_lorentz_force tests/test_lorentz_force.cpp)
target_link_libraries(test_lorentz_force ftd_core)

add_executable(test_selective_damping tests/test_selective_damping.cpp)
target_link_libraries(test_selective_damping ftd_core)

add_executable(test_em_energy_conservation tests/test_em_energy_conservation.cpp)
target_link_libraries(test_em_energy_conservation ftd_core)

add_executable(test_continuity tests/test_continuity.cpp)
target_link_libraries(test_continuity ftd_core)

add_executable(test_poynting tests/test_poynting.cpp)
target_link_libraries(test_poynting ftd_core)

add_executable(test_larmor tests/test_larmor.cpp)
target_link_libraries(test_larmor ftd_core)

# Emergent EM physics tests
add_executable(test_dipole_radiation tests/test_dipole_radiation.cpp)
target_link_libraries(test_dipole_radiation ftd_core)

add_executable(test_dispersion_relation tests/test_dispersion_relation.cpp)
target_link_libraries(test_dispersion_relation ftd_core)

add_executable(test_thomson_scattering tests/test_thomson_scattering.cpp)
target_link_libraries(test_thomson_scattering ftd_core)

add_executable(test_light tests/test_light.cpp)
target_link_libraries(test_light ftd_core)

# Phase 1: Measurement Infrastructure
add_executable(test_ensemble tests/test_ensemble.cpp)
target_link_libraries(test_ensemble ftd_core)

add_executable(test_correlations tests/test_correlations.cpp)
target_link_libraries(test_correlations ftd_core)

add_executable(test_spectral tests/test_spectral.cpp)
target_link_libraries(test_spectral ftd_core)

add_executable(test_tracker tests/test_tracker.cpp)
target_link_libraries(test_tracker ftd_core)

# Benchmark
add_executable(test_benchmark tests/test_benchmark.cpp)
target_link_libraries(test_benchmark ftd_core)

# GPU parity and benchmark tests (CUDA builds only)
if(FTD_ENABLE_CUDA)
    add_executable(test_gpu_parity tests/test_gpu_parity.cpp)
    target_link_libraries(test_gpu_parity ftd_core ftd_cuda)

    add_executable(test_gpu_benchmark tests/test_gpu_benchmark.cpp)
    target_link_libraries(test_gpu_benchmark ftd_core ftd_cuda)

    add_executable(test_gpu_physics tests/test_gpu_physics.cpp)
    target_link_libraries(test_gpu_physics ftd_core ftd_cuda)

    add_executable(test_gpu_experiments tests/test_gpu_experiments.cpp)
    target_link_libraries(test_gpu_experiments ftd_core ftd_cuda)

    # e+e- annihilation angular distribution (Einstein's scattering experiment)
    add_executable(ftd_annihilation_angular_gpu tests/campaign_annihilation_angular.cpp)
    target_link_libraries(ftd_annihilation_angular_gpu ftd_core ftd_cuda)

    add_executable(test_gpu_shell_256 tests/test_gpu_shell_256.cpp)
    target_link_libraries(test_gpu_shell_256 ftd_core ftd_cuda)

    add_executable(test_gpu_shell_battery tests/test_gpu_shell_battery.cpp)
    target_link_libraries(test_gpu_shell_battery ftd_core ftd_cuda)

    add_executable(test_gpu_profile_compare tests/test_gpu_profile_compare.cpp)
    target_link_libraries(test_gpu_profile_compare ftd_core ftd_cuda)

    add_executable(test_gpu_dissipation tests/test_gpu_dissipation_source.cpp)
    target_link_libraries(test_gpu_dissipation ftd_core ftd_cuda)
endif()

# ============================================================================
# Simulation Campaigns
# ============================================================================

add_executable(ftd_dispersion tests/campaign_dispersion.cpp)
target_link_libraries(ftd_dispersion ftd_core)

add_executable(ftd_gauge_dynamics tests/campaign_gauge_dynamics.cpp)
target_link_libraries(ftd_gauge_dynamics ftd_core)

add_executable(ftd_gauge_constraint tests/campaign_gauge_constraint.cpp)
target_link_libraries(ftd_gauge_constraint ftd_core)

add_executable(ftd_free_dynamics tests/campaign_free_dynamics.cpp)
target_link_libraries(ftd_free_dynamics ftd_core)

add_executable(ftd_aggregate_interaction tests/campaign_aggregate_interaction.cpp)
target_link_libraries(ftd_aggregate_interaction ftd_core)

add_executable(ftd_poisson_force_law tests/campaign_poisson_force_law.cpp)
target_link_libraries(ftd_poisson_force_law ftd_core)

add_executable(ftd_poisson_binding tests/campaign_poisson_binding.cpp)
target_link_libraries(ftd_poisson_binding ftd_core)

add_executable(ftd_poisson_hydrogen tests/campaign_poisson_hydrogen.cpp)
target_link_libraries(ftd_poisson_hydrogen ftd_core)

add_executable(ftd_force_law tests/campaign_force_law.cpp)
target_link_libraries(ftd_force_law ftd_core)

add_executable(ftd_shell_predictions tests/campaign_shell_predictions.cpp)
target_link_libraries(ftd_shell_predictions ftd_core)

add_executable(ftd_energy_audit tests/campaign_energy_audit.cpp)
target_link_libraries(ftd_energy_audit ftd_core)

add_executable(ftd_bound_lifetime tests/campaign_bound_lifetime.cpp)
target_link_libraries(ftd_bound_lifetime ftd_core)

add_executable(ftd_spontaneous tests/campaign_spontaneous_structure.cpp)
target_link_libraries(ftd_spontaneous ftd_core)

add_executable(ftd_cross_scale tests/campaign_cross_scale.cpp)
target_link_libraries(ftd_cross_scale ftd_core)

add_executable(ftd_born_ensemble tests/campaign_born_ensemble.cpp)
target_link_libraries(ftd_born_ensemble ftd_core)

add_executable(ftd_h2_molecule tests/campaign_h2_molecule.cpp)
target_link_libraries(ftd_h2_molecule ftd_core)

add_executable(ftd_multiscale_pipeline tests/campaign_multiscale_pipeline.cpp)
target_link_libraries(ftd_multiscale_pipeline ftd_core)

# Phase 1: Measurement Infrastructure campaigns
add_executable(ftd_statistical_convergence tests/campaign_statistical_convergence.cpp)
target_link_libraries(ftd_statistical_convergence ftd_core)

# Phase 2: Continuum Limit campaigns
add_executable(ftd_dispersion_convergence tests/campaign_dispersion_convergence.cpp)
target_link_libraries(ftd_dispersion_convergence ftd_core)

add_executable(ftd_coulomb_convergence tests/campaign_coulomb_convergence.cpp)
target_link_libraries(ftd_coulomb_convergence ftd_core)

add_executable(ftd_wave_isotropy tests/campaign_wave_isotropy.cpp)
target_link_libraries(ftd_wave_isotropy ftd_core)

# Phase 3: Bell Test & Quantum Mechanics
add_executable(ftd_born_rule tests/campaign_born_rule.cpp)
target_link_libraries(ftd_born_rule ftd_core)

add_executable(ftd_bell_substrate tests/campaign_bell_substrate.cpp)
target_link_libraries(ftd_bell_substrate ftd_core)

add_executable(ftd_epr_correlation tests/campaign_epr_correlation.cpp)
target_link_libraries(ftd_epr_correlation ftd_core)

# Phase 4: Emergent Mass Spectrum
add_executable(ftd_hydrogen_binding tests/campaign_hydrogen_binding.cpp)
target_link_libraries(ftd_hydrogen_binding ftd_core)
add_executable(ftd_triad_energy tests/campaign_triad_energy.cpp)
target_link_libraries(ftd_triad_energy ftd_core)
add_executable(ftd_inertial_mass tests/campaign_inertial_mass.cpp)
target_link_libraries(ftd_inertial_mass ftd_core)
add_executable(ftd_structure_stability tests/campaign_structure_stability.cpp)
target_link_libraries(ftd_structure_stability ftd_core)

# Phase 5: Color Dynamics & SU(3)
add_executable(ftd_color_force tests/campaign_color_force.cpp)
target_link_libraries(ftd_color_force ftd_core)
add_executable(ftd_color_neutral tests/campaign_color_neutral.cpp)
target_link_libraries(ftd_color_neutral ftd_core)
add_executable(ftd_confinement tests/campaign_confinement.cpp)
target_link_libraries(ftd_confinement ftd_core)
add_executable(ftd_baryon_formation tests/campaign_baryon_formation.cpp)
target_link_libraries(ftd_baryon_formation ftd_core)

# Phase 6: Weak Sector & SU(2)
add_executable(ftd_weak_transmutation tests/campaign_weak_transmutation.cpp)
target_link_libraries(ftd_weak_transmutation ftd_core)
add_executable(ftd_parity_violation tests/campaign_parity_violation.cpp)
target_link_libraries(ftd_parity_violation ftd_core)
add_executable(ftd_weak_decay tests/campaign_weak_decay.cpp)
target_link_libraries(ftd_weak_decay ftd_core)

# Phase 7: Gravitational Sector
add_executable(ftd_gravitational_wave tests/campaign_gravitational_wave.cpp)
target_link_libraries(ftd_gravitational_wave ftd_core)
add_executable(ftd_gravity_profile tests/campaign_gravity_profile.cpp)
target_link_libraries(ftd_gravity_profile ftd_core)
add_executable(ftd_gravity_hierarchy tests/campaign_gravity_hierarchy.cpp)
target_link_libraries(ftd_gravity_hierarchy ftd_core)

# Phase 8: Particle Zoo campaigns
add_executable(ftd_triad_binding tests/campaign_triad_binding.cpp)
target_link_libraries(ftd_triad_binding ftd_core)
add_executable(ftd_neutrino_sector tests/campaign_neutrino_sector.cpp)
target_link_libraries(ftd_neutrino_sector ftd_core)

# Phase 9: Cosmological Predictions campaign
add_executable(ftd_cosmological_predictions tests/campaign_cosmological_predictions.cpp)
target_link_libraries(ftd_cosmological_predictions ftd_core)

# Phase 9b: Dark Sector Dynamics campaign
add_executable(ftd_dark_sector tests/campaign_dark_sector.cpp)
target_link_libraries(ftd_dark_sector ftd_core)

# Phase 10: Novel Predictions & Falsifiability campaign
add_executable(ftd_novel_predictions tests/campaign_novel_predictions.cpp)
target_link_libraries(ftd_novel_predictions ftd_core)

# Phase 12: Gravity Sector (latency field, proper time, bandwidth constraint)
add_executable(test_latency_field tests/test_latency_field.cpp)
target_link_libraries(test_latency_field ftd_core)

# Phase 11: Scientific Validation (negative results, uniqueness, quantitative benchmarks)
add_executable(test_falsifiability tests/test_falsifiability.cpp)
target_link_libraries(test_falsifiability ftd_core)
add_executable(ftd_integer_sweep tests/campaign_integer_sweep.cpp)
target_link_libraries(ftd_integer_sweep ftd_core)
add_executable(ftd_hydrogen_spectrum tests/campaign_hydrogen_spectrum.cpp)
target_link_libraries(ftd_hydrogen_spectrum ftd_core)
add_executable(ftd_two_slit tests/campaign_two_slit.cpp)
target_link_libraries(ftd_two_slit ftd_core)

# Double-slit information loss campaigns (3D lattice experiments)
# All use SimEngine (engine_select.h) for GPU acceleration when CUDA is enabled
set(DS_CAMPAIGNS
    ftd_ds_phase_recovery
    ftd_ds_ternary_detector
    ftd_ds_vortex_lines
    ftd_ds_correlation_function
    ftd_ds_void_classification
    ftd_ds_information_cascade
)
set(DS_SOURCES
    tests/campaign_ds_phase_recovery.cpp
    tests/campaign_ds_ternary_detector.cpp
    tests/campaign_ds_vortex_lines.cpp
    tests/campaign_ds_correlation_function.cpp
    tests/campaign_ds_void_classification.cpp
    tests/campaign_ds_information_cascade.cpp
)
list(LENGTH DS_CAMPAIGNS DS_COUNT)
math(EXPR DS_LAST "${DS_COUNT} - 1")
foreach(idx RANGE ${DS_LAST})
    list(GET DS_CAMPAIGNS ${idx} tgt)
    list(GET DS_SOURCES ${idx} src)
    add_executable(${tgt} ${src})
    if(FTD_ENABLE_CUDA)
        target_link_libraries(${tgt} ftd_core ftd_cuda)
    else()
        target_link_libraries(${tgt} ftd_core)
    endif()
endforeach()

# Phase 12: Cosmology + Consciousness Sector
add_executable(test_inflation tests/test_inflation.cpp)
target_link_libraries(test_inflation ftd_core)

add_executable(test_dark_matter tests/test_dark_matter.cpp)
target_link_libraries(test_dark_matter ftd_core)

add_executable(test_cosmological_constant tests/test_cosmological_constant.cpp)
target_link_libraries(test_cosmological_constant ftd_core)

add_executable(test_consciousness tests/test_consciousness.cpp)
target_link_libraries(test_consciousness ftd_core)

add_executable(test_sloop tests/test_sloop.cpp)
target_link_libraries(test_sloop ftd_core)

# Phase 12b: Precision + Electroweak Sector
add_executable(test_lorentz_invariance tests/test_lorentz_invariance.cpp)
target_link_libraries(test_lorentz_invariance ftd_core)
add_executable(test_electroweak tests/test_electroweak.cpp)
target_link_libraries(test_electroweak ftd_core)
add_executable(test_hydrogen_em_only tests/test_hydrogen_em_only.cpp)
target_link_libraries(test_hydrogen_em_only ftd_core)
add_executable(ftd_lorentz_measure tests/campaign_lorentz_measure.cpp)
target_link_libraries(ftd_lorentz_measure ftd_core)

# Scale 1 Phase 2: ParticleEngine force expansion (7 new forces)
add_executable(test_pe_exchange tests/test_pe_exchange.cpp)
target_link_libraries(test_pe_exchange ftd_core)
add_executable(test_pe_strong tests/test_pe_strong.cpp)
target_link_libraries(test_pe_strong ftd_core)
add_executable(test_pe_lorentz tests/test_pe_lorentz.cpp)
target_link_libraries(test_pe_lorentz ftd_core)
add_executable(test_pe_magnetic_dipole tests/test_pe_magnetic_dipole.cpp)
target_link_libraries(test_pe_magnetic_dipole ftd_core)
add_executable(test_pe_spin_orbit tests/test_pe_spin_orbit.cpp)
target_link_libraries(test_pe_spin_orbit ftd_core)
add_executable(test_pe_radiation tests/test_pe_radiation.cpp)
target_link_libraries(test_pe_radiation ftd_core)
add_executable(test_pe_relativistic tests/test_pe_relativistic.cpp)
target_link_libraries(test_pe_relativistic ftd_core)
add_executable(ftd_pe_fine_structure tests/campaign_pe_fine_structure.cpp)
target_link_libraries(ftd_pe_fine_structure ftd_core)

# Scale 2 Phase 3: AtomEngine force expansion (5 new forces)
add_executable(test_ae_hbonds tests/test_ae_hbonds.cpp)
target_link_libraries(test_ae_hbonds ftd_core)
add_executable(test_ae_angle_strain tests/test_ae_angle_strain.cpp)
target_link_libraries(test_ae_angle_strain ftd_core)
add_executable(test_ae_dipole tests/test_ae_dipole.cpp)
target_link_libraries(test_ae_dipole ftd_core)
add_executable(test_ae_thermostat tests/test_ae_thermostat.cpp)
target_link_libraries(test_ae_thermostat ftd_core)
add_executable(test_ae_electronegativity tests/test_ae_electronegativity.cpp)
target_link_libraries(test_ae_electronegativity ftd_core)
add_executable(ftd_ae_water tests/campaign_ae_water.cpp)
target_link_libraries(ftd_ae_water ftd_core)

# Orphaned tests registered by I-04 audit (Mar 2026)
add_executable(test_action_stationarity tests/test_action_stationarity.cpp)
target_link_libraries(test_action_stationarity PRIVATE ftd_core)
add_executable(test_asymptotic_freedom tests/test_asymptotic_freedom.cpp)
target_link_libraries(test_asymptotic_freedom PRIVATE ftd_core)
add_executable(test_atom_toggles tests/test_atom_toggles.cpp)
target_link_libraries(test_atom_toggles PRIVATE ftd_core)
add_executable(test_atomic_energy tests/test_atomic_energy.cpp)
target_link_libraries(test_atomic_energy PRIVATE ftd_core)
add_executable(test_baryogenesis tests/test_baryogenesis.cpp)
target_link_libraries(test_baryogenesis PRIVATE ftd_core)
add_executable(test_bell_aggregate tests/test_bell_aggregate.cpp)
target_link_libraries(test_bell_aggregate PRIVATE ftd_core)
add_executable(test_born_rule_ensemble tests/test_born_rule_ensemble.cpp)
target_link_libraries(test_born_rule_ensemble PRIVATE ftd_core)
add_executable(test_confinement tests/test_confinement.cpp)
target_link_libraries(test_confinement PRIVATE ftd_core)
add_executable(test_entanglement_basis tests/test_entanglement_basis.cpp)
target_link_libraries(test_entanglement_basis PRIVATE ftd_core)
add_executable(test_flavor_physics tests/test_flavor_physics.cpp)
target_link_libraries(test_flavor_physics PRIVATE ftd_core)
add_executable(test_higgs_mechanism tests/test_higgs_mechanism.cpp)
target_link_libraries(test_higgs_mechanism PRIVATE ftd_core)
add_executable(test_hilbert tests/test_hilbert.cpp)
target_link_libraries(test_hilbert PRIVATE ftd_core)
add_executable(test_measurement tests/test_measurement.cpp)
target_link_libraries(test_measurement PRIVATE ftd_core)
add_executable(test_particle_toggles tests/test_particle_toggles.cpp)
target_link_libraries(test_particle_toggles PRIVATE ftd_core)
add_executable(test_spin_statistics tests/test_spin_statistics.cpp)
target_link_libraries(test_spin_statistics PRIVATE ftd_core)
add_executable(test_triad_confinement tests/test_triad_confinement.cpp)
target_link_libraries(test_triad_confinement PRIVATE ftd_core)
add_executable(test_wz_mass tests/test_wz_mass.cpp)
target_link_libraries(test_wz_mass PRIVATE ftd_core)

# Tritium ternary compute library tests (header-only, no ftd_core dependency)
add_executable(test_trit_packing tests/test_trit_packing.cpp)
target_include_directories(test_trit_packing PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(test_trit_arithmetic tests/test_trit_arithmetic.cpp)
target_include_directories(test_trit_arithmetic PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(test_trit_logic tests/test_trit_logic.cpp)
target_include_directories(test_trit_logic PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(test_trit_threshold tests/test_trit_threshold.cpp)
target_include_directories(test_trit_threshold PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(test_trit_vector tests/test_trit_vector.cpp)
target_include_directories(test_trit_vector PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(test_trit_matrix tests/test_trit_matrix.cpp)
target_include_directories(test_trit_matrix PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(test_trit_convolution tests/test_trit_convolution.cpp)
target_include_directories(test_trit_convolution PRIVATE ${CMAKE_SOURCE_DIR}/include)

endif() # NOT EMSCRIPTEN — end of tests/campaigns/CLI

# ============================================================================
# WASM build (Emscripten)
# ============================================================================

if(EMSCRIPTEN)
    add_subdirectory(wasm)
endif()

# ============================================================================
# CTest Registration
# ============================================================================

enable_testing()

# Unit tests
add_test(NAME constants COMMAND test_constants)
add_test(NAME lorentz COMMAND test_lorentz)
add_test(NAME lattice COMMAND test_lattice)
add_test(NAME born_infeld COMMAND test_born_infeld)
add_test(NAME energy COMMAND test_energy)
add_test(NAME gauss COMMAND test_gauss)
add_test(NAME stress_energy COMMAND test_stress_energy)
add_test(NAME thermodynamics COMMAND test_thermodynamics)
add_test(NAME lagrangian COMMAND test_lagrangian)
add_test(NAME ontic_chain COMMAND test_ontic_chain)
add_test(NAME dual_substrate COMMAND test_dual_substrate)
add_test(NAME genesis COMMAND test_genesis)
add_test(NAME gravity_dynamics COMMAND test_gravity_dynamics)
add_test(NAME annihilation COMMAND test_annihilation)
add_test(NAME annihilation_conservation COMMAND test_annihilation_conservation)
add_test(NAME wave_collapse COMMAND test_wave_collapse)
add_test(NAME wave_speed COMMAND test_wave_speed)
add_test(NAME interference COMMAND test_interference)
add_test(NAME gauge COMMAND test_gauge)
add_test(NAME polarization COMMAND test_polarization)
add_test(NAME momentum COMMAND test_momentum)
add_test(NAME magnetic COMMAND test_magnetic)
add_test(NAME flux_mediated COMMAND test_flux_mediated)
add_test(NAME entanglement COMMAND test_entanglement)
add_test(NAME variational_coulomb COMMAND test_variational_coulomb)
add_test(NAME magnetic_lagrangian COMMAND test_magnetic_lagrangian)
add_test(NAME dissipation COMMAND test_dissipation)
add_test(NAME portable_field COMMAND test_portable_field)
add_test(NAME particle_lifetime COMMAND test_particle_lifetime)
add_test(NAME vortex COMMAND test_vortex)
add_test(NAME voxel_properties COMMAND test_voxel_properties)
add_test(NAME lattice_operators COMMAND test_lattice_operators)
add_test(NAME discrete_operators COMMAND test_discrete_operators)
add_test(NAME bridge_dynamics COMMAND test_bridge_dynamics)
add_test(NAME maxwell COMMAND test_maxwell)
add_test(NAME csv_export COMMAND test_csv_export)
add_test(NAME logic_engine COMMAND test_logic_engine)
add_test(NAME poisson_coulomb COMMAND test_poisson_coulomb)
add_test(NAME energy_tracking COMMAND test_energy_tracking)
add_test(NAME energy_conservation COMMAND test_energy_conservation)
add_test(NAME selffield_profile COMMAND test_selffield_profile)
add_test(NAME wavepacket COMMAND test_wavepacket)
add_test(NAME particle_engine COMMAND test_particle_engine)
add_test(NAME scale_bridge COMMAND test_scale_bridge)
add_test(NAME hydrogen_scale1 COMMAND test_hydrogen_scale1)
add_test(NAME atom_engine COMMAND test_atom_engine)
add_test(NAME atom_scale_bridge COMMAND test_atom_scale_bridge)
add_test(NAME multiscale_bridge COMMAND test_multiscale_bridge)
add_test(NAME em_fields COMMAND test_em_fields)
add_test(NAME gauss_convergence COMMAND test_gauss_convergence)
add_test(NAME lorentz_force COMMAND test_lorentz_force)
add_test(NAME selective_damping COMMAND test_selective_damping)
add_test(NAME em_energy_conservation COMMAND test_em_energy_conservation)
add_test(NAME continuity COMMAND test_continuity)
add_test(NAME poynting COMMAND test_poynting)
add_test(NAME larmor COMMAND test_larmor)
add_test(NAME dipole_radiation COMMAND test_dipole_radiation)
add_test(NAME dispersion_relation COMMAND test_dispersion_relation)
add_test(NAME thomson_scattering COMMAND test_thomson_scattering)
add_test(NAME light COMMAND test_light)
add_test(NAME ensemble COMMAND test_ensemble)
add_test(NAME correlations COMMAND test_correlations)
add_test(NAME spectral COMMAND test_spectral)
add_test(NAME tracker COMMAND test_tracker)
add_test(NAME benchmark COMMAND test_benchmark)
add_test(NAME latency_field COMMAND test_latency_field)

# Scale 1 Phase 2: PE force expansion
add_test(NAME pe_exchange COMMAND test_pe_exchange)
add_test(NAME pe_strong COMMAND test_pe_strong)
add_test(NAME pe_lorentz COMMAND test_pe_lorentz)
add_test(NAME pe_magnetic_dipole COMMAND test_pe_magnetic_dipole)
add_test(NAME pe_spin_orbit COMMAND test_pe_spin_orbit)
add_test(NAME pe_radiation COMMAND test_pe_radiation)
add_test(NAME pe_relativistic COMMAND test_pe_relativistic)
add_test(NAME campaign_pe_fine_structure COMMAND ftd_pe_fine_structure)

# Scale 2 Phase 3: AE force expansion
add_test(NAME ae_hbonds COMMAND test_ae_hbonds)
add_test(NAME ae_angle_strain COMMAND test_ae_angle_strain)
add_test(NAME ae_dipole COMMAND test_ae_dipole)
add_test(NAME ae_thermostat COMMAND test_ae_thermostat)
add_test(NAME ae_electronegativity COMMAND test_ae_electronegativity)
add_test(NAME campaign_ae_water COMMAND ftd_ae_water)

# Orphaned tests registered by I-04 audit (Mar 2026)
add_test(NAME action_stationarity COMMAND test_action_stationarity)
add_test(NAME asymptotic_freedom COMMAND test_asymptotic_freedom)
add_test(NAME atom_toggles COMMAND test_atom_toggles)
add_test(NAME atomic_energy COMMAND test_atomic_energy)
add_test(NAME baryogenesis COMMAND test_baryogenesis)
add_test(NAME bell_aggregate COMMAND test_bell_aggregate)
add_test(NAME born_rule_ensemble COMMAND test_born_rule_ensemble)
add_test(NAME confinement_test COMMAND test_confinement)
add_test(NAME entanglement_basis COMMAND test_entanglement_basis)
add_test(NAME flavor_physics COMMAND test_flavor_physics)
add_test(NAME higgs_mechanism COMMAND test_higgs_mechanism)
add_test(NAME hilbert COMMAND test_hilbert)
add_test(NAME measurement COMMAND test_measurement)
add_test(NAME particle_toggles COMMAND test_particle_toggles)
add_test(NAME spin_statistics COMMAND test_spin_statistics)
add_test(NAME triad_confinement COMMAND test_triad_confinement)
add_test(NAME wz_mass COMMAND test_wz_mass)
set_tests_properties(
    action_stationarity asymptotic_freedom atom_toggles atomic_energy
    baryogenesis bell_aggregate born_rule_ensemble confinement_test
    entanglement_basis flavor_physics higgs_mechanism hilbert
    measurement particle_toggles spin_statistics triad_confinement
    wz_mass
    PROPERTIES TIMEOUT 600)

# Tritium ternary compute library tests
add_test(NAME trit_packing COMMAND test_trit_packing)
add_test(NAME trit_arithmetic COMMAND test_trit_arithmetic)
add_test(NAME trit_logic COMMAND test_trit_logic)
add_test(NAME trit_threshold COMMAND test_trit_threshold)
add_test(NAME trit_vector COMMAND test_trit_vector)
add_test(NAME trit_matrix COMMAND test_trit_matrix)
add_test(NAME trit_convolution COMMAND test_trit_convolution)

# GPU tests (only registered when CUDA is enabled)
if(FTD_ENABLE_CUDA)
    add_test(NAME gpu_parity COMMAND test_gpu_parity)
    add_test(NAME gpu_benchmark COMMAND test_gpu_benchmark)
    add_test(NAME gpu_physics COMMAND test_gpu_physics)
    add_test(NAME gpu_experiments COMMAND test_gpu_experiments)
    set_tests_properties(gpu_parity gpu_benchmark gpu_physics PROPERTIES TIMEOUT 600)
    set_tests_properties(gpu_experiments PROPERTIES TIMEOUT 1800)
endif()

# Campaign tests
add_test(NAME campaign_dispersion COMMAND ftd_dispersion)
add_test(NAME campaign_gauge_dynamics COMMAND ftd_gauge_dynamics)
add_test(NAME campaign_gauge_constraint COMMAND ftd_gauge_constraint)
add_test(NAME campaign_free_dynamics COMMAND ftd_free_dynamics)
add_test(NAME campaign_aggregate_interaction COMMAND ftd_aggregate_interaction)
add_test(NAME campaign_poisson_force_law COMMAND ftd_poisson_force_law)
add_test(NAME campaign_poisson_binding COMMAND ftd_poisson_binding)
add_test(NAME campaign_poisson_hydrogen COMMAND ftd_poisson_hydrogen)
add_test(NAME campaign_force_law COMMAND ftd_force_law)
add_test(NAME campaign_shell_predictions COMMAND ftd_shell_predictions)
add_test(NAME campaign_energy_audit COMMAND ftd_energy_audit)
add_test(NAME campaign_bound_lifetime COMMAND ftd_bound_lifetime)
add_test(NAME campaign_spontaneous COMMAND ftd_spontaneous)
add_test(NAME campaign_cross_scale COMMAND ftd_cross_scale)
add_test(NAME campaign_born_ensemble COMMAND ftd_born_ensemble)
add_test(NAME campaign_h2_molecule COMMAND ftd_h2_molecule)
add_test(NAME campaign_multiscale_pipeline COMMAND ftd_multiscale_pipeline)
add_test(NAME campaign_statistical_convergence COMMAND ftd_statistical_convergence)
add_test(NAME campaign_dispersion_convergence COMMAND ftd_dispersion_convergence)
add_test(NAME campaign_coulomb_convergence COMMAND ftd_coulomb_convergence)
add_test(NAME campaign_wave_isotropy COMMAND ftd_wave_isotropy)
add_test(NAME campaign_born_rule COMMAND ftd_born_rule)
add_test(NAME campaign_bell_substrate COMMAND ftd_bell_substrate)
add_test(NAME campaign_epr_correlation COMMAND ftd_epr_correlation)
add_test(NAME campaign_hydrogen_binding COMMAND ftd_hydrogen_binding)
add_test(NAME campaign_triad_energy COMMAND ftd_triad_energy)
add_test(NAME campaign_inertial_mass COMMAND ftd_inertial_mass)
add_test(NAME campaign_structure_stability COMMAND ftd_structure_stability)

# Phase 5: Color Dynamics & SU(3) campaigns
add_test(NAME campaign_color_force COMMAND ftd_color_force)
add_test(NAME campaign_color_neutral COMMAND ftd_color_neutral)
add_test(NAME campaign_confinement COMMAND ftd_confinement)
add_test(NAME campaign_baryon_formation COMMAND ftd_baryon_formation)

# Phase 6: Weak Sector & SU(2) campaigns
add_test(NAME campaign_weak_transmutation COMMAND ftd_weak_transmutation)
add_test(NAME campaign_parity_violation COMMAND ftd_parity_violation)
add_test(NAME campaign_weak_decay COMMAND ftd_weak_decay)

# Phase 7: Gravitational Sector campaigns
add_test(NAME campaign_gravitational_wave COMMAND ftd_gravitational_wave)
add_test(NAME campaign_gravity_profile COMMAND ftd_gravity_profile)
add_test(NAME campaign_gravity_hierarchy COMMAND ftd_gravity_hierarchy)

# Phase 8: Particle Zoo campaigns
add_test(NAME campaign_triad_binding COMMAND ftd_triad_binding)
add_test(NAME campaign_neutrino_sector COMMAND ftd_neutrino_sector)

# Phase 9: Cosmological Predictions campaign
add_test(NAME campaign_cosmological_predictions COMMAND ftd_cosmological_predictions)
add_test(NAME campaign_dark_sector COMMAND ftd_dark_sector)

# Phase 10: Novel Predictions & Falsifiability campaign
add_test(NAME campaign_novel_predictions COMMAND ftd_novel_predictions)

# Phase 11: Scientific Validation
add_test(NAME falsifiability COMMAND test_falsifiability)
add_test(NAME campaign_integer_sweep COMMAND ftd_integer_sweep)
add_test(NAME campaign_hydrogen_spectrum COMMAND ftd_hydrogen_spectrum)
add_test(NAME campaign_two_slit COMMAND ftd_two_slit)
add_test(NAME campaign_ds_phase_recovery COMMAND ftd_ds_phase_recovery)
add_test(NAME campaign_ds_ternary_detector COMMAND ftd_ds_ternary_detector)
add_test(NAME campaign_ds_vortex_lines COMMAND ftd_ds_vortex_lines)
add_test(NAME campaign_ds_correlation_function COMMAND ftd_ds_correlation_function)
add_test(NAME campaign_ds_void_classification COMMAND ftd_ds_void_classification)
add_test(NAME campaign_ds_information_cascade COMMAND ftd_ds_information_cascade)

# Phase 12: Cosmology + Consciousness
add_test(NAME inflation COMMAND test_inflation)
add_test(NAME dark_matter COMMAND test_dark_matter)
add_test(NAME cosmological_constant COMMAND test_cosmological_constant)
add_test(NAME consciousness COMMAND test_consciousness)
add_test(NAME sloop COMMAND test_sloop)
# Phase 12b: Precision + Electroweak Sector
add_test(NAME lorentz_invariance COMMAND test_lorentz_invariance)
add_test(NAME electroweak COMMAND test_electroweak)
add_test(NAME hydrogen_em_only COMMAND test_hydrogen_em_only)
add_test(NAME campaign_lorentz_measure COMMAND ftd_lorentz_measure)

# Timeouts for long-running tests (10 min)
set_tests_properties(
    benchmark logic_engine energy_tracking energy_conservation
    poisson_coulomb selffield_profile wavepacket
    particle_lifetime
    campaign_dispersion campaign_gauge_dynamics campaign_gauge_constraint
    campaign_poisson_force_law campaign_poisson_binding campaign_poisson_hydrogen
    campaign_force_law campaign_energy_audit campaign_bound_lifetime
    campaign_spontaneous campaign_cross_scale campaign_born_ensemble
    particle_engine scale_bridge hydrogen_scale1
    atom_engine atom_scale_bridge multiscale_bridge campaign_h2_molecule
    campaign_multiscale_pipeline
    selective_damping stress_energy
    em_energy_conservation continuity poynting larmor
    ensemble correlations spectral tracker
    campaign_statistical_convergence
    campaign_dispersion_convergence campaign_coulomb_convergence
    campaign_wave_isotropy
    campaign_born_rule campaign_bell_substrate campaign_epr_correlation
    campaign_hydrogen_binding campaign_triad_energy
    campaign_inertial_mass campaign_structure_stability
    campaign_color_force campaign_color_neutral
    campaign_confinement campaign_baryon_formation
    campaign_weak_transmutation campaign_parity_violation
    campaign_weak_decay
    campaign_gravitational_wave campaign_gravity_profile
    campaign_gravity_hierarchy
    campaign_triad_binding campaign_neutrino_sector
    campaign_cosmological_predictions campaign_dark_sector
    campaign_novel_predictions
    falsifiability campaign_integer_sweep
    campaign_hydrogen_spectrum campaign_two_slit
    latency_field
    pe_exchange pe_strong pe_lorentz pe_magnetic_dipole
    pe_spin_orbit pe_radiation pe_relativistic
    campaign_pe_fine_structure
    ae_hbonds ae_angle_strain ae_dipole ae_thermostat
    ae_electronegativity campaign_ae_water
    inflation dark_matter cosmological_constant consciousness sloop
    lorentz_invariance electroweak hydrogen_em_only campaign_lorentz_measure
    PROPERTIES TIMEOUT 600)

# Heavy campaigns with 64^3 grids or multi-phase dynamics need more time (30 min)
# flux_mediated moved here from 600s group per I-06 audit
set_tests_properties(
    campaign_free_dynamics campaign_aggregate_interaction
    campaign_gravity_profile flux_mediated
    PROPERTIES TIMEOUT 1800)

# ============================================================================
# CTest Labels — run subsets with: ctest -L <label>
# ============================================================================

# Unit tests (108 test_*.cpp files)
set_tests_properties(
    constants lorentz lattice born_infeld energy gauss stress_energy
    thermodynamics lagrangian ontic_chain dual_substrate genesis
    gravity_dynamics annihilation annihilation_conservation wave_collapse
    wave_speed interference gauge polarization momentum magnetic
    flux_mediated entanglement variational_coulomb magnetic_lagrangian
    dissipation portable_field particle_lifetime vortex voxel_properties
    lattice_operators discrete_operators bridge_dynamics maxwell csv_export
    logic_engine poisson_coulomb energy_tracking energy_conservation
    selffield_profile wavepacket particle_engine scale_bridge hydrogen_scale1
    atom_engine atom_scale_bridge multiscale_bridge em_fields gauss_convergence
    lorentz_force selective_damping em_energy_conservation continuity poynting
    larmor dipole_radiation dispersion_relation thomson_scattering light
    ensemble correlations spectral tracker benchmark latency_field
    pe_exchange pe_strong pe_lorentz pe_magnetic_dipole pe_spin_orbit
    pe_radiation pe_relativistic
    ae_hbonds ae_angle_strain ae_dipole ae_thermostat ae_electronegativity
    action_stationarity asymptotic_freedom atom_toggles atomic_energy
    baryogenesis bell_aggregate born_rule_ensemble confinement_test
    entanglement_basis flavor_physics higgs_mechanism hilbert measurement
    particle_toggles spin_statistics triad_confinement wz_mass
    falsifiability inflation dark_matter cosmological_constant consciousness
    sloop lorentz_invariance electroweak hydrogen_em_only
    PROPERTIES LABELS "unit")

# Campaign tests (47 campaign_*.cpp files)
set_tests_properties(
    campaign_dispersion campaign_gauge_dynamics campaign_gauge_constraint
    campaign_free_dynamics campaign_aggregate_interaction
    campaign_poisson_force_law campaign_poisson_binding campaign_poisson_hydrogen
    campaign_force_law campaign_energy_audit campaign_bound_lifetime
    campaign_spontaneous campaign_cross_scale campaign_born_ensemble
    campaign_h2_molecule campaign_multiscale_pipeline
    campaign_statistical_convergence campaign_dispersion_convergence
    campaign_coulomb_convergence campaign_wave_isotropy
    campaign_born_rule campaign_bell_substrate campaign_epr_correlation
    campaign_hydrogen_binding campaign_triad_energy campaign_inertial_mass
    campaign_structure_stability campaign_color_force campaign_color_neutral
    campaign_confinement campaign_baryon_formation
    campaign_weak_transmutation campaign_parity_violation campaign_weak_decay
    campaign_gravitational_wave campaign_gravity_profile campaign_gravity_hierarchy
    campaign_triad_binding campaign_neutrino_sector
    campaign_cosmological_predictions campaign_dark_sector campaign_novel_predictions
    campaign_integer_sweep campaign_hydrogen_spectrum campaign_two_slit
    campaign_pe_fine_structure campaign_ae_water campaign_lorentz_measure
    PROPERTIES LABELS "campaign")

# Foundation tests (fast math/constants checks)
set_tests_properties(
    constants lorentz lattice ontic_chain born_infeld voxel_properties
    lattice_operators discrete_operators dissipation magnetic_lagrangian
    PROPERTIES LABELS "foundation")

# Lagrangian & variational tests
set_tests_properties(
    lagrangian variational_coulomb magnetic_lagrangian dissipation
    action_stationarity
    PROPERTIES LABELS "lagrangian")

# Scale 1: ParticleEngine tests
set_tests_properties(
    particle_engine pe_exchange pe_strong pe_lorentz pe_magnetic_dipole
    pe_spin_orbit pe_radiation pe_relativistic particle_toggles
    campaign_pe_fine_structure
    PROPERTIES LABELS "scale1")

# Scale 2: AtomEngine tests
set_tests_properties(
    atom_engine atom_scale_bridge ae_hbonds ae_angle_strain ae_dipole
    ae_thermostat ae_electronegativity atom_toggles campaign_ae_water
    PROPERTIES LABELS "scale2")
