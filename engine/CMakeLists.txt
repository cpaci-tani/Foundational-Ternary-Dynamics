cmake_minimum_required(VERSION 3.18)
project(ftd_engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenMP for multi-threaded tick phases (not available under Emscripten)
if(NOT EMSCRIPTEN)
    find_package(OpenMP)
endif()

# CUDA GPU acceleration — DEFAULT ON via AUTO probe (Phase 0: FTD Test Bench).
#
# Tests are 10–100x faster on GPU. CUDA is now opt-OUT rather than opt-in.
# If CUDAToolkit is not found on the system (e.g. CI runners without a GPU),
# we gracefully downgrade to a CPU-only build with a status message.
#
# Override behaviors:
#   -DFTD_ENABLE_CUDA=OFF     → force CPU build even if CUDA is available
#   -DFTD_ENABLE_CUDA=ON      → require CUDA (error if not found)
#   (unset / AUTO)            → use CUDA if found, else CPU (default)
#
# Probe strategy: find_package(CUDAToolkit) succeeds as long as nvcc exists
# in PATH, and does NOT require a working host compiler (unlike
# check_language(CUDA) which tries to compile a test program). This makes
# the probe robust across both Developer Command Prompt and plain shells.
#
if(NOT DEFINED FTD_ENABLE_CUDA)
    set(FTD_ENABLE_CUDA "AUTO" CACHE STRING "Enable CUDA (ON/OFF/AUTO)")
endif()

set(_ftd_cuda_active OFF)
if(NOT EMSCRIPTEN AND NOT FTD_ENABLE_CUDA STREQUAL "OFF")
    # Non-REQUIRED probe so OFF-path stays clean.
    find_package(CUDAToolkit QUIET)
    if(CUDAToolkit_FOUND)
        set(_ftd_cuda_active ON)
    elseif(FTD_ENABLE_CUDA STREQUAL "ON")
        message(FATAL_ERROR "FTD_ENABLE_CUDA=ON but CUDAToolkit not found. "
                            "Install CUDA Toolkit or set -DFTD_ENABLE_CUDA=OFF.")
    else()
        message(STATUS "CUDAToolkit not found — building CPU-only. "
                       "Install CUDA and re-run cmake for 10-100x test speedup.")
    endif()
endif()

if(_ftd_cuda_active)
    # Allow VS 2025+ (v18) — CUDA 13.0 officially supports up to VS 2022,
    # but VS 2025/2026 work with this flag. Must be set BEFORE enable_language.
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --allow-unsupported-compiler")

    # Default architectures: Ada (89: RTX 4060-4090) + Blackwell (120: RTX 5090+)
    # Must be set BEFORE enable_language(CUDA) — otherwise CMake 3.24+ picks
    # its own default (often 75 / Turing).
    # Override: cmake -DCMAKE_CUDA_ARCHITECTURES="86;89;120" for Ampere support
    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES OR CMAKE_CUDA_ARCHITECTURES STREQUAL "")
        set(CMAKE_CUDA_ARCHITECTURES "89;120")
    endif()

    enable_language(CUDA)
    set(CMAKE_CUDA_STANDARD 17)
    message(STATUS "CUDA enabled — architectures: ${CMAKE_CUDA_ARCHITECTURES}")
    add_compile_definitions(FTD_ENABLE_CUDA)
    # Make FTD_ENABLE_CUDA boolean-true for downstream if() checks.
    set(FTD_ENABLE_CUDA ON CACHE STRING "CUDA is enabled" FORCE)
    add_subdirectory(cuda)
else()
    set(FTD_ENABLE_CUDA OFF CACHE STRING "CUDA is disabled" FORCE)
endif()

# Test runner macros (Phase 0 of FTD Test Bench)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(FtdAddTest)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library
add_library(ftd_core
    src/lattice.cpp
    src/render_bridge.cpp
    src/dag_engine.cpp
    src/lagrangian.cpp
    src/particle_engine.cpp
    src/atom_engine.cpp
    src/cosmic_engine.cpp
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

# ---- Test Telemetry self-test (uses ftd_add_test macro for first time) ----
# This test exercises the ftd::test NDJSON API. It doesn't need ftd_core.
ftd_add_test(test_telemetry_selftest tests/test_telemetry_selftest.cpp NO_CORE)

# Foundational
add_executable(test_constants tests/test_constants.cpp)
target_link_libraries(test_constants ftd_core)

# Wave 4b.6: test_lorentz_force + test_lorentz_invariance + test_magnetic +
# test_magnetic_lagrangian merged into test_lorentz (self-ref target)
ftd_add_test(test_lorentz tests/test_lorentz.cpp CTEST_NAME lorentz TIMEOUT 600)

add_executable(test_lattice tests/test_lattice.cpp)
target_link_libraries(test_lattice ftd_core)

# Lagrangian verification
add_executable(test_born_infeld tests/test_born_infeld.cpp)
target_link_libraries(test_born_infeld ftd_core)

# (test_energy merged into test_energy_conservation via Wave 4a.3)

# Wave 4b.5 (2026-04-14): test_gauss_convergence merged into test_gauss
ftd_add_test(test_gauss tests/test_gauss.cpp CTEST_NAME gauss TIMEOUT 600)

add_executable(test_intervoxel_coupling tests/test_intervoxel_coupling.cpp)
target_link_libraries(test_intervoxel_coupling ftd_core)

add_executable(test_stress_energy tests/test_stress_energy.cpp)
target_link_libraries(test_stress_energy ftd_core)

add_executable(test_thermodynamics tests/test_thermodynamics.cpp)
target_link_libraries(test_thermodynamics ftd_core)

add_executable(test_lagrangian tests/test_lagrangian.cpp)
target_link_libraries(test_lagrangian ftd_core)

# DAG Engine Phase 3
add_executable(test_dag_engine tests/test_dag_engine.cpp)
target_link_libraries(test_dag_engine ftd_core)

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
# Wave 4a.2 (2026-04-14): wave dynamics consolidated into
# campaign_wave_dynamics (4 files → 1). Registered via ftd_add_test
# near the other campaign tests.

# (test_interference merged into campaign_wave_dynamics)

add_executable(test_gauge tests/test_gauge.cpp)
target_link_libraries(test_gauge ftd_core)

add_executable(test_polarization tests/test_polarization.cpp)
target_link_libraries(test_polarization ftd_core)

add_executable(test_momentum tests/test_momentum.cpp)
target_link_libraries(test_momentum ftd_core)

# (test_magnetic merged into test_lorentz via Wave 4b.6)

add_executable(test_flux_mediated tests/test_flux_mediated.cpp)
target_link_libraries(test_flux_mediated ftd_core)

# Phase 2: SM sectors (active subset)
# (test_entanglement merged into campaign_quantum_correlations via Wave 4a.4)

# Phase 4: Variational forces
add_executable(test_variational_coulomb tests/test_variational_coulomb.cpp)
target_link_libraries(test_variational_coulomb ftd_core)

# (test_magnetic_lagrangian merged into test_lorentz via Wave 4b.6)

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
# Wave 4c.8: test_poisson_coulomb + campaign_coulomb_convergence +
# campaign_force_law + campaign_poisson_force_law + campaign_poisson_binding
# merged into campaign_coulomb_force_law (NEW target)
ftd_add_test(ftd_coulomb_force_law tests/campaign_coulomb_force_law.cpp
             CTEST_NAME campaign_coulomb_force_law TIMEOUT 1800)

# Wave 4a.3 (2026-04-14): test_energy + test_energy_tracking merged into
# test_energy_conservation (self-ref target). Registered via ftd_add_test.
ftd_add_test(test_energy_conservation tests/test_energy_conservation.cpp
             CTEST_NAME energy_conservation TIMEOUT 600)

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

# Wave 4c.9: test_hydrogen_scale1 + test_hydrogen_em_only +
# test_hydrogen_spectrum_scale1 + campaign_poisson_hydrogen merged
# into campaign_hydrogen_spectrum (self-ref target)
ftd_add_test(ftd_hydrogen_spectrum tests/campaign_hydrogen_spectrum.cpp
             CTEST_NAME campaign_hydrogen_spectrum TIMEOUT 1800)

# Multi-scale physics recovery tests (April 2026)
# (test_hydrogen_spectrum_scale1 merged into campaign_hydrogen_spectrum)

add_executable(test_fine_structure_scale1 tests/test_fine_structure_scale1.cpp)
target_link_libraries(test_fine_structure_scale1 ftd_core)

add_executable(test_scale_proof_chain tests/test_scale_proof_chain.cpp)
target_link_libraries(test_scale_proof_chain ftd_core)

add_executable(test_helium_scale1 tests/test_helium_scale1.cpp)
target_link_libraries(test_helium_scale1 ftd_core)

add_executable(test_radiative_decay_scale1 tests/test_radiative_decay_scale1.cpp)
target_link_libraries(test_radiative_decay_scale1 ftd_core)

# Phase 8: Atom engine (Scale 2)
add_executable(test_atom_engine tests/test_atom_engine.cpp)
target_link_libraries(test_atom_engine ftd_core)

add_executable(test_atom_scale_bridge tests/test_atom_scale_bridge.cpp)
target_link_libraries(test_atom_scale_bridge ftd_core)

add_executable(test_multiscale_bridge tests/test_multiscale_bridge.cpp)
target_link_libraries(test_multiscale_bridge ftd_core)

add_executable(test_em_fields tests/test_em_fields.cpp)
target_link_libraries(test_em_fields ftd_core)

# (test_gauss_convergence merged into test_gauss via Wave 4b.5)

# (test_lorentz_force merged into test_lorentz via Wave 4b.6)

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

# (test_dispersion_relation merged into campaign_dispersion via Wave 4b.7)

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

    add_executable(test_gpu_parity_complete tests/test_gpu_parity_complete.cpp)
    target_link_libraries(test_gpu_parity_complete ftd_core ftd_cuda)

    add_executable(test_gpu_profile_compare tests/test_gpu_profile_compare.cpp)
    target_link_libraries(test_gpu_profile_compare ftd_core ftd_cuda)

    add_executable(test_gpu_dissipation tests/test_gpu_dissipation_source.cpp)
    target_link_libraries(test_gpu_dissipation ftd_core ftd_cuda)
endif()

# ============================================================================
# Simulation Campaigns
# ============================================================================

# Wave 4b.7: test_dispersion_relation + campaign_dispersion_convergence
# merged into campaign_dispersion (self-ref target)
ftd_add_test(ftd_dispersion tests/campaign_dispersion.cpp
             CTEST_NAME campaign_dispersion GPU_HEAVY TIMEOUT 1800)

add_executable(ftd_gauge_dynamics tests/campaign_gauge_dynamics.cpp)
target_link_libraries(ftd_gauge_dynamics ftd_core)

add_executable(ftd_gauge_constraint tests/campaign_gauge_constraint.cpp)
target_link_libraries(ftd_gauge_constraint ftd_core)

add_executable(ftd_free_dynamics tests/campaign_free_dynamics.cpp)
target_link_libraries(ftd_free_dynamics ftd_core)

add_executable(ftd_aggregate_interaction tests/campaign_aggregate_interaction.cpp)
target_link_libraries(ftd_aggregate_interaction ftd_core)

# (campaign_poisson_force_law + campaign_poisson_binding merged into campaign_coulomb_force_law)

# (campaign_poisson_hydrogen merged into campaign_hydrogen_spectrum)

# (campaign_force_law merged into campaign_coulomb_force_law)

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
# (campaign_dispersion_convergence merged into campaign_dispersion via Wave 4b.7)

# (campaign_coulomb_convergence merged into campaign_coulomb_force_law)

# Benchmark suites (April 2026). Registered via ftd_add_test so they appear
# in ctest and get the "benchmark" label auto-applied by the macro. All use
# a 1800s TIMEOUT because benchmarks run longer than regular unit tests.
# Wave 3.1 (2026-04-14): was previously just add_executable without add_test,
# so ctest never ran them and their internal assertion failures never showed
# up in the pass/fail tally. Registering them exposes ~28 internal asserts
# that were invisible before.
ftd_add_test(ftd_benchmark_engine_theory tests/benchmark_engine_theory.cpp
             CTEST_NAME benchmark_engine_theory TIMEOUT 1800)
ftd_add_test(ftd_emergent_alpha tests/benchmark_emergent_alpha.cpp
             CTEST_NAME benchmark_emergent_alpha TIMEOUT 1800)
ftd_add_test(ftd_budget_equation tests/benchmark_budget_equation.cpp
             CTEST_NAME benchmark_budget_equation TIMEOUT 1800)
ftd_add_test(ftd_bh_thermo tests/benchmark_black_hole_thermo.cpp
             CTEST_NAME benchmark_bh_thermo TIMEOUT 1800)
ftd_add_test(ftd_wilson_loops tests/benchmark_wilson_loops.cpp GPU_HEAVY
             CTEST_NAME benchmark_wilson_loops TIMEOUT 1800)

add_executable(ftd_sm_observables tests/campaign_sm_observables.cpp)
target_link_libraries(ftd_sm_observables ftd_core)

# (campaign_wave_isotropy merged into campaign_wave_dynamics)

# Five Minds campaign tests (April 2026)
add_executable(ftd_plato tests/campaign_plato.cpp)
target_link_libraries(ftd_plato ftd_core)
add_executable(ftd_einstein tests/campaign_einstein.cpp)
target_link_libraries(ftd_einstein ftd_core)
add_executable(ftd_vonneumann tests/campaign_vonneumann.cpp)
target_link_libraries(ftd_vonneumann ftd_core)
add_executable(ftd_wigner tests/campaign_wigner.cpp)
target_link_libraries(ftd_wigner ftd_core)
add_executable(ftd_grothendieck tests/campaign_grothendieck.cpp)
target_link_libraries(ftd_grothendieck ftd_core)

# Phase 3: Bell Test & Quantum Mechanics
add_executable(ftd_born_rule tests/campaign_born_rule.cpp)
target_link_libraries(ftd_born_rule ftd_core)

# Wave 4a.4 (2026-04-14): test_entanglement + campaign_epr_correlation +
# campaign_bell_substrate merged into campaign_quantum_correlations.
ftd_add_test(ftd_quantum_correlations tests/campaign_quantum_correlations.cpp
             CTEST_NAME campaign_quantum_correlations TIMEOUT 600)

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
# Wave 4c.10: 5 QCD campaigns merged into campaign_qcd_forces
ftd_add_test(ftd_qcd_forces tests/campaign_qcd_forces.cpp
             CTEST_NAME campaign_qcd_forces TIMEOUT 1800)

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
# Wave 4c.11: 7 dark-sector tests merged into campaign_dark_sector (self-ref)
ftd_add_test(ftd_dark_sector tests/campaign_dark_sector.cpp
             CTEST_NAME campaign_dark_sector GPU_HEAVY TIMEOUT 1800)

# Phase 10: Novel Predictions & Falsifiability campaign
add_executable(ftd_novel_predictions tests/campaign_novel_predictions.cpp)
target_link_libraries(ftd_novel_predictions ftd_core)

# Phase 12: Gravity Sector (latency field, proper time, bandwidth constraint)
add_executable(test_latency_field tests/test_latency_field.cpp)
target_link_libraries(test_latency_field ftd_core)
add_executable(test_einstein_equations tests/test_einstein_equations.cpp)
target_link_libraries(test_einstein_equations ftd_core)

# Phase 11: Scientific Validation (negative results, uniqueness, quantitative benchmarks)
add_executable(test_falsifiability tests/test_falsifiability.cpp)
target_link_libraries(test_falsifiability ftd_core)
add_executable(ftd_integer_sweep tests/campaign_integer_sweep.cpp)
target_link_libraries(ftd_integer_sweep ftd_core)
# (ftd_hydrogen_spectrum already registered via ftd_add_test above)
# Wave dynamics consolidation target
ftd_add_test(ftd_wave_dynamics tests/campaign_wave_dynamics.cpp
             CTEST_NAME campaign_wave_dynamics TIMEOUT 600)
# (campaign_two_slit merged into campaign_wave_dynamics)

# (Wave 4c.11: 6 campaign_ds_* tests merged into campaign_dark_sector above)

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
# (test_lorentz_invariance merged into test_lorentz via Wave 4b.6)
add_executable(test_electroweak tests/test_electroweak.cpp)
target_link_libraries(test_electroweak ftd_core)
# (test_hydrogen_em_only merged into campaign_hydrogen_spectrum)
add_executable(ftd_lorentz_measure tests/campaign_lorentz_measure.cpp)
target_link_libraries(ftd_lorentz_measure ftd_core)

# Scale 1 Phase 2: ParticleEngine force expansion (7 new forces)
# Phase 1+2b POC: consolidated into test_pe_forces (see below)
ftd_add_test(test_pe_forces tests/test_pe_forces.cpp)
add_executable(ftd_pe_fine_structure tests/campaign_pe_fine_structure.cpp)
target_link_libraries(ftd_pe_fine_structure ftd_core)

# Scale 2 Phase 3: AtomEngine force expansion (5 new forces)
# Wave 4a.1 (2026-04-14): ae_* forces consolidated into test_atom_engine_forces
# (5 files → 1). Registered via ftd_add_test; see section_* functions in the
# source for angle_strain, dipole, electronegativity, hbonds, thermostat.
ftd_add_test(test_atom_engine_forces tests/test_atom_engine_forces.cpp
             CTEST_NAME atom_engine_forces TIMEOUT 600)
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
# Phase 1+2b POC: consolidated into test_tritium_algebra (7 legacy files merged)
ftd_add_test(test_tritium_algebra tests/test_tritium_algebra.cpp NO_CORE)

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
# (lorentz registered via ftd_add_test above)
add_test(NAME lattice COMMAND test_lattice)
add_test(NAME born_infeld COMMAND test_born_infeld)
# (test_energy merged into energy_conservation)
# (gauss registered via ftd_add_test above)
add_test(NAME intervoxel_coupling COMMAND test_intervoxel_coupling)
add_test(NAME stress_energy COMMAND test_stress_energy)
add_test(NAME thermodynamics COMMAND test_thermodynamics)
add_test(NAME lagrangian COMMAND test_lagrangian)
add_test(NAME dag_engine COMMAND test_dag_engine)
add_test(NAME ontic_chain COMMAND test_ontic_chain)
add_test(NAME dual_substrate COMMAND test_dual_substrate)
add_test(NAME genesis COMMAND test_genesis)
add_test(NAME gravity_dynamics COMMAND test_gravity_dynamics)
add_test(NAME annihilation COMMAND test_annihilation)
add_test(NAME annihilation_conservation COMMAND test_annihilation_conservation)
add_test(NAME wave_collapse COMMAND test_wave_collapse)
# (wave_speed + interference merged into campaign_wave_dynamics)
add_test(NAME gauge COMMAND test_gauge)
add_test(NAME polarization COMMAND test_polarization)
add_test(NAME momentum COMMAND test_momentum)
# (magnetic merged into lorentz)
add_test(NAME flux_mediated COMMAND test_flux_mediated)
# (entanglement merged into campaign_quantum_correlations)
add_test(NAME variational_coulomb COMMAND test_variational_coulomb)
# (magnetic_lagrangian merged into lorentz)
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
# (poisson_coulomb merged into campaign_coulomb_force_law)
# (energy_tracking + energy_conservation merged via ftd_add_test above)
add_test(NAME selffield_profile COMMAND test_selffield_profile)
add_test(NAME wavepacket COMMAND test_wavepacket)
add_test(NAME particle_engine COMMAND test_particle_engine)
add_test(NAME scale_bridge COMMAND test_scale_bridge)
# (hydrogen_scale1 + hydrogen_spectrum_scale1 merged into campaign_hydrogen_spectrum)
add_test(NAME fine_structure_scale1 COMMAND test_fine_structure_scale1)
add_test(NAME scale_proof_chain COMMAND test_scale_proof_chain)
add_test(NAME helium_scale1 COMMAND test_helium_scale1)
add_test(NAME radiative_decay_scale1 COMMAND test_radiative_decay_scale1)
add_test(NAME atom_engine COMMAND test_atom_engine)
add_test(NAME atom_scale_bridge COMMAND test_atom_scale_bridge)
add_test(NAME multiscale_bridge COMMAND test_multiscale_bridge)
add_test(NAME em_fields COMMAND test_em_fields)
# (gauss_convergence merged into gauss)
# (lorentz_force merged into lorentz)
add_test(NAME selective_damping COMMAND test_selective_damping)
add_test(NAME em_energy_conservation COMMAND test_em_energy_conservation)
add_test(NAME continuity COMMAND test_continuity)
add_test(NAME poynting COMMAND test_poynting)
add_test(NAME larmor COMMAND test_larmor)
add_test(NAME dipole_radiation COMMAND test_dipole_radiation)
# (dispersion_relation merged into campaign_dispersion)
add_test(NAME thomson_scattering COMMAND test_thomson_scattering)
add_test(NAME light COMMAND test_light)
add_test(NAME ensemble COMMAND test_ensemble)
add_test(NAME correlations COMMAND test_correlations)
add_test(NAME spectral COMMAND test_spectral)
add_test(NAME tracker COMMAND test_tracker)
add_test(NAME benchmark COMMAND test_benchmark)
add_test(NAME latency_field COMMAND test_latency_field)
add_test(NAME einstein_equations COMMAND test_einstein_equations)

# Scale 1 Phase 2: PE force expansion
# Phase 1+2b POC: 7 legacy pe_* CTest entries consolidated under pe_forces
# (registered by ftd_add_test() above).
add_test(NAME campaign_pe_fine_structure COMMAND ftd_pe_fine_structure)

# Scale 2 Phase 3: AE force expansion
# (ae_* consolidated into atom_engine_forces via ftd_add_test above)
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
# Phase 1+2b POC: 7 legacy trit_* CTest entries consolidated under tritium_algebra
# (registered by ftd_add_test() above).

# GPU tests (only registered when CUDA is enabled)
if(FTD_ENABLE_CUDA)
    add_test(NAME gpu_parity COMMAND test_gpu_parity)
    add_test(NAME gpu_parity_complete COMMAND test_gpu_parity_complete)
    add_test(NAME gpu_benchmark COMMAND test_gpu_benchmark)
    add_test(NAME gpu_physics COMMAND test_gpu_physics)
    add_test(NAME gpu_experiments COMMAND test_gpu_experiments)
    set_tests_properties(gpu_parity gpu_parity_complete gpu_benchmark gpu_physics PROPERTIES TIMEOUT 600)
    set_tests_properties(gpu_experiments PROPERTIES TIMEOUT 1800)
endif()

# Campaign tests
# (campaign_dispersion registered via ftd_add_test above)
add_test(NAME campaign_gauge_dynamics COMMAND ftd_gauge_dynamics)
add_test(NAME campaign_gauge_constraint COMMAND ftd_gauge_constraint)
add_test(NAME campaign_free_dynamics COMMAND ftd_free_dynamics)
add_test(NAME campaign_aggregate_interaction COMMAND ftd_aggregate_interaction)
# (campaign_poisson_force_law + campaign_poisson_binding merged)
# (campaign_poisson_hydrogen merged)
# (campaign_force_law merged)
add_test(NAME campaign_shell_predictions COMMAND ftd_shell_predictions)
add_test(NAME campaign_energy_audit COMMAND ftd_energy_audit)
add_test(NAME campaign_bound_lifetime COMMAND ftd_bound_lifetime)
add_test(NAME campaign_spontaneous COMMAND ftd_spontaneous)
add_test(NAME campaign_plato COMMAND ftd_plato)
add_test(NAME campaign_einstein COMMAND ftd_einstein)
add_test(NAME campaign_vonneumann COMMAND ftd_vonneumann)
add_test(NAME campaign_wigner COMMAND ftd_wigner)
add_test(NAME campaign_grothendieck COMMAND ftd_grothendieck)
add_test(NAME campaign_cross_scale COMMAND ftd_cross_scale)
add_test(NAME campaign_born_ensemble COMMAND ftd_born_ensemble)
add_test(NAME campaign_h2_molecule COMMAND ftd_h2_molecule)
add_test(NAME campaign_multiscale_pipeline COMMAND ftd_multiscale_pipeline)
add_test(NAME campaign_statistical_convergence COMMAND ftd_statistical_convergence)
# (campaign_dispersion_convergence merged into campaign_dispersion)
# (campaign_coulomb_convergence merged)
add_test(NAME campaign_sm_observables COMMAND ftd_sm_observables)
# (campaign_wave_isotropy merged into campaign_wave_dynamics)
add_test(NAME campaign_born_rule COMMAND ftd_born_rule)
# (campaign_bell_substrate + campaign_epr_correlation merged above)
add_test(NAME campaign_hydrogen_binding COMMAND ftd_hydrogen_binding)
add_test(NAME campaign_triad_energy COMMAND ftd_triad_energy)
add_test(NAME campaign_inertial_mass COMMAND ftd_inertial_mass)
add_test(NAME campaign_structure_stability COMMAND ftd_structure_stability)

# Phase 5: Color Dynamics & SU(3) campaigns
# (5 QCD campaigns merged into campaign_qcd_forces)

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
# (campaign_dark_sector registered via ftd_add_test above)

# Phase 10: Novel Predictions & Falsifiability campaign
add_test(NAME campaign_novel_predictions COMMAND ftd_novel_predictions)

# Phase 11: Scientific Validation
add_test(NAME falsifiability COMMAND test_falsifiability)
add_test(NAME campaign_integer_sweep COMMAND ftd_integer_sweep)
# (campaign_hydrogen_spectrum registered via ftd_add_test)
# (campaign_two_slit merged into campaign_wave_dynamics)
# (6 campaign_ds_* tests merged into campaign_dark_sector)

# Phase 12: Cosmology + Consciousness
add_test(NAME inflation COMMAND test_inflation)
add_test(NAME dark_matter COMMAND test_dark_matter)
add_test(NAME cosmological_constant COMMAND test_cosmological_constant)
add_test(NAME consciousness COMMAND test_consciousness)
add_test(NAME sloop COMMAND test_sloop)
# Phase 12b: Precision + Electroweak Sector
# (lorentz_invariance merged into lorentz)
add_test(NAME electroweak COMMAND test_electroweak)
# (hydrogen_em_only merged)
add_test(NAME campaign_lorentz_measure COMMAND ftd_lorentz_measure)

# Timeouts for long-running tests (10 min)
set_tests_properties(
    benchmark logic_engine energy_conservation
    selffield_profile wavepacket
    particle_lifetime
    campaign_dispersion campaign_gauge_dynamics campaign_gauge_constraint
    campaign_energy_audit campaign_bound_lifetime
    campaign_spontaneous campaign_cross_scale campaign_born_ensemble
    particle_engine scale_bridge
    atom_engine atom_scale_bridge multiscale_bridge campaign_h2_molecule
    campaign_multiscale_pipeline
    selective_damping stress_energy
    em_energy_conservation continuity poynting larmor
    ensemble correlations spectral tracker
    campaign_statistical_convergence
    campaign_born_rule campaign_quantum_correlations
    campaign_hydrogen_binding campaign_triad_energy
    campaign_inertial_mass campaign_structure_stability
    campaign_qcd_forces
    campaign_weak_transmutation campaign_parity_violation
    campaign_weak_decay
    campaign_gravitational_wave campaign_gravity_profile
    campaign_gravity_hierarchy
    campaign_triad_binding campaign_neutrino_sector
    campaign_cosmological_predictions campaign_dark_sector
    campaign_novel_predictions
    falsifiability campaign_integer_sweep
    campaign_hydrogen_spectrum campaign_wave_dynamics
    latency_field
    pe_forces tritium_algebra
    campaign_pe_fine_structure
    atom_engine_forces campaign_ae_water
    inflation dark_matter cosmological_constant consciousness sloop
    electroweak campaign_lorentz_measure
    helium_scale1 fine_structure_scale1
    radiative_decay_scale1 scale_proof_chain
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
# Using set_property(... APPEND ...) so later scale1/scale2/lagrangian
# blocks can stack their category labels on top of "unit" rather than
# overwriting it. Tests that are BOTH unit AND scale1 (e.g. pe_forces,
# particle_engine) will end up with labels=[unit, scale1].
set_property(TEST
    constants lattice born_infeld gauss stress_energy
    thermodynamics lagrangian ontic_chain dual_substrate genesis
    gravity_dynamics annihilation annihilation_conservation wave_collapse
    # wave_speed/interference merged into campaign_wave_dynamics
    gauge polarization momentum
    flux_mediated variational_coulomb
    dissipation portable_field particle_lifetime vortex voxel_properties
    lattice_operators discrete_operators bridge_dynamics maxwell csv_export
    logic_engine energy_conservation
    selffield_profile wavepacket particle_engine scale_bridge
    atom_engine atom_scale_bridge multiscale_bridge em_fields
    selective_damping em_energy_conservation continuity poynting
    larmor dipole_radiation thomson_scattering light
    ensemble correlations spectral tracker benchmark latency_field
    # pe_forces and tritium_algebra auto-labeled as "unit" by ftd_add_test()
    # atom_engine_forces auto-labeled as "unit" by ftd_add_test()
    action_stationarity asymptotic_freedom atom_toggles atomic_energy
    baryogenesis bell_aggregate born_rule_ensemble confinement_test
    entanglement_basis flavor_physics higgs_mechanism hilbert measurement
    particle_toggles spin_statistics triad_confinement wz_mass
    falsifiability inflation dark_matter cosmological_constant consciousness
    sloop electroweak
    APPEND PROPERTY LABELS "unit")

# Campaign tests (47 campaign_*.cpp files)
# APPEND is critical so campaign-level labels (scale1, scale2, gpu, etc.)
# stack on top of these category labels rather than overwriting them.
set_property(TEST
    campaign_dispersion campaign_gauge_dynamics campaign_gauge_constraint
    campaign_free_dynamics campaign_aggregate_interaction
    campaign_coulomb_force_law
    campaign_energy_audit campaign_bound_lifetime
    campaign_spontaneous campaign_cross_scale campaign_born_ensemble
    campaign_h2_molecule campaign_multiscale_pipeline
    campaign_statistical_convergence campaign_wave_dynamics
    campaign_born_rule
    campaign_hydrogen_binding campaign_triad_energy campaign_inertial_mass
    campaign_structure_stability campaign_qcd_forces
    campaign_weak_transmutation campaign_parity_violation campaign_weak_decay
    campaign_gravitational_wave campaign_gravity_profile campaign_gravity_hierarchy
    campaign_triad_binding campaign_neutrino_sector
    campaign_cosmological_predictions campaign_dark_sector campaign_novel_predictions
    campaign_integer_sweep campaign_hydrogen_spectrum
    campaign_pe_fine_structure campaign_ae_water campaign_lorentz_measure
    APPEND PROPERTY LABELS "campaign")

# Foundation tests (fast math/constants checks)
set_property(TEST
    constants lattice ontic_chain born_infeld voxel_properties
    lattice_operators discrete_operators dissipation
    APPEND PROPERTY LABELS "foundation")

# Lagrangian & variational tests
set_property(TEST
    lagrangian variational_coulomb dissipation
    action_stationarity
    APPEND PROPERTY LABELS "lagrangian")

# Scale 1: ParticleEngine tests
# APPEND so pe_forces keeps both "unit" (from ftd_add_test) and "scale1".
set_property(TEST
    particle_engine pe_forces particle_toggles
    campaign_pe_fine_structure
    APPEND PROPERTY LABELS "scale1")

# Scale 2: AtomEngine tests
set_property(TEST
    atom_engine atom_scale_bridge atom_engine_forces
    atom_toggles campaign_ae_water
    APPEND PROPERTY LABELS "scale2")

# ============================================================================
# GPU labels (Phase 0, FTD Test Bench)
# ============================================================================
#
# Tests marked with the "gpu" label need a dedicated CUDA device and should
# be dispatched serially by the test runner's SmartDispatcher. All other
# tests can run in parallel on CPU workers.
#
# Uses set_property(... APPEND ...) so labels stack cleanly with the earlier
# "unit" / "campaign" / "foundation" labels.
#
if(FTD_ENABLE_CUDA)
    set(_ftd_gpu_tests
        # Explicit GPU parity/benchmark/physics tests
        gpu_parity
        gpu_benchmark
        gpu_physics
        gpu_experiments
        # Wave 2 (Apr 14, 2026): GPU-heavy tests previously false-timing out
        # under ctest -j 24 due to single-RTX-5090 contention. Serializing
        # them via the "gpu" label eliminates the false TIMEOUT failures.
        # Confirmed contention list from AUDIT_LATENCY_2026_04_14.md.
        lorentz
        maxwell
        em_energy_conservation
        poynting
        spectral
        # (campaign_dispersion now has "gpu" via GPU_HEAVY in ftd_add_test;
        #  campaign_dispersion_convergence merged into campaign_dispersion)
        campaign_grothendieck
        # (campaign_ds_vortex_lines + campaign_ds_correlation_function
        #  merged into campaign_dark_sector; it has GPU_HEAVY + gpu label)
    )
    foreach(_t IN LISTS _ftd_gpu_tests)
        if(TEST ${_t})
            set_property(TEST ${_t} APPEND PROPERTY LABELS "gpu")
        endif()
    endforeach()
    unset(_ftd_gpu_tests)
endif()

# ============================================================================
# FTD Test Bench (Phase 3+: Qt6 native test runner)
# ============================================================================
#
# Builds engine/tools/test_runner/ — a Qt6 Widgets native desktop application
# that drives the CTest inventory, streams NDJSON telemetry from the test
# subprocesses (see engine/include/ftd/test_telemetry.h), and displays
# results in a main window with a category tree, output panel, lattice
# viewer (Phase 4), telemetry charts (Phase 5), and history DB (Phase 6).
#
# Controlled by FTD_BUILD_TEST_RUNNER:
#   AUTO (default) — build if Qt6 is present, silently skip otherwise
#   ON             — require Qt6; fail configure if not found
#   OFF            — never build
#
# Pass -DCMAKE_PREFIX_PATH=C:/Qt/6.10.2/msvc2022_64 (or equivalent) on the
# configure command line so find_package(Qt6) resolves.
#
set(FTD_BUILD_TEST_RUNNER "AUTO" CACHE STRING
    "Build the Qt6 test runner GUI (ON/OFF/AUTO)")
set_property(CACHE FTD_BUILD_TEST_RUNNER PROPERTY STRINGS ON OFF AUTO)

if(NOT FTD_BUILD_TEST_RUNNER STREQUAL "OFF" AND NOT EMSCRIPTEN)
    find_package(Qt6 QUIET COMPONENTS Core Widgets OpenGL OpenGLWidgets Sql Charts)
    if(Qt6_FOUND)
        message(STATUS "FTD Test Bench: enabled (Qt6 ${Qt6_VERSION})")
        add_subdirectory(tools/test_runner)
    elseif(FTD_BUILD_TEST_RUNNER STREQUAL "ON")
        message(FATAL_ERROR "FTD_BUILD_TEST_RUNNER=ON but Qt6 not found. "
                            "Install Qt6 or set -DFTD_BUILD_TEST_RUNNER=OFF.")
    else()
        message(STATUS "FTD Test Bench: disabled (Qt6 not found)")
    endif()
endif()
