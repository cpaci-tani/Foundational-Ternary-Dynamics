cmake_minimum_required(VERSION 3.18)
project(ftd_engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell MSVC to interpret source files as UTF-8 so that hex-escaped multi-byte
# sequences in string literals (e.g. ontic_audit.cpp) are accepted correctly.
if(MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/utf-8>)
endif()

# OpenMP for multi-threaded tick phases (not available under Emscripten)
if(NOT EMSCRIPTEN)
    find_package(OpenMP)
endif()

# CUDA GPU acceleration — DEFAULT REQUIRED for native builds.
#
# Tests are 10–100x faster on GPU. CUDA is now the normal engine backend, and
# missing CUDA should fail native configuration instead of silently producing a
# CPU run.
#
# Override behaviors:
#   -DFTD_ENABLE_CUDA=OFF     → force CPU build even if CUDA is available
#   -DFTD_ENABLE_CUDA=ON      → require CUDA (error if not found)
#   -DFTD_ENABLE_CUDA=AUTO    → use CUDA if found, else CPU (explicit fallback)
#   unset native default      → ON
#   unset Emscripten default  → OFF
#
# Probe strategy: find_package(CUDAToolkit) succeeds as long as nvcc exists
# in PATH, and does NOT require a working host compiler (unlike
# check_language(CUDA) which tries to compile a test program). This makes
# the probe robust across both Developer Command Prompt and plain shells.
#
if(NOT DEFINED FTD_ENABLE_CUDA)
    if(EMSCRIPTEN)
        set(FTD_ENABLE_CUDA "OFF" CACHE STRING "Enable CUDA (ON/OFF/AUTO)")
    else()
        set(FTD_ENABLE_CUDA "ON" CACHE STRING "Enable CUDA (ON/OFF/AUTO)")
    endif()
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
    # Workaround for CMake 4.x + NVCC 13 + MSVC 18: the CMake-generated
    # `-Xcompiler=-Fd<path>,-FS` triggers `nvcc fatal: A single input
    # file is required` because nvcc interprets the comma as an argument
    # separator. Forcing MSVC debug info into the .obj (/Z7 instead of
    # /Zi+/Fd) sidesteps the problematic -Fd entirely.
    if(MSVC)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")
    endif()
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

# ftd_eft — leaf library for EFT objects shared between ftd_core and ftd_cuda.
# Solves a static-archive link-order issue (2026-05-03): gpu_engine.cu
# (in libftd_cuda.a) calls DualCellContinuity::DualCellContinuity(int);
# extracting the providing TU into a leaf library that BOTH ftd_core
# and ftd_cuda PUBLIC-depend on guarantees libftd_eft.a appears AFTER
# both consumers in the link line, satisfying gpu_engine's reference.
# Replaces the manual `target_link_libraries(... ftd_core ftd_cuda ftd_core)`
# workaround used for test_wilson_dirac_cuda_parity (commit 514cdb3).
add_library(ftd_eft STATIC
    src/eft/dual_cell_continuity.cpp
)
target_include_directories(ftd_eft PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Core library
add_library(ftd_core
    src/render_bridge.cpp
    src/dag_engine.cpp
    src/lagrangian.cpp
    src/particle_engine.cpp
    src/atom_engine.cpp
    src/cosmic_engine.cpp
    src/scale_bridge.cpp
    src/ontic_audit.cpp    # RF-03: ontic_audit() body moved from ontic.h
    src/eft/qcd_one_loop_perturbative.cpp  # [IMPOSED] renamed from ontic_running_coupling.cpp
                                           # EFT Phase 0: reserves running_coupling.cpp
                                           # for Phase-2 lattice-measured β(g)
    src/eft/blocking.cpp                    # EFT Phase 2A: real-space block-spin transformation
    src/eft/dual_cell_blocking.cpp          # Native EFT: finite-volume source/flux blocking
    # src/eft/dual_cell_continuity.cpp moved to ftd_eft (see above)
    src/eft/dual_cell_flow.cpp              # Native EFT: bare b=2 flow measurements
    src/csv_export.cpp     # RF-05: csv export bodies moved from csv_export.h
    src/constructors/constructors_core.cpp
    src/constructors/constructors_atoms.cpp
    src/constructors/constructors_bulk_matter.cpp
    src/constructors/constructors_exotic.cpp
    src/constructors/constructors_molecules.cpp
    # Scale-0 scenario library (split from monolithic scenarios.cpp, 2026-04-19)
    src/scenarios.cpp                # thin router + shared RNG
    src/scenarios/flux.cpp           # 20 flux-* scenarios
    src/scenarios/light.cpp          # 4 light-* scenarios
    src/scenarios/quantum.cpp        # 8 quantum-* scenarios
    src/scenarios/s0_seed.cpp        # 49 s0-seed-* scenarios (+ seed_lepton/dp/tri statics)
    src/scenarios/s0_field.cpp       # 8 s0-field-* scenarios
    src/scenarios/vacuum.cpp         # 15 s0-vacuum-* scenarios
    # R1-R5 modular extraction from render_bridge.cpp (2026-04-18)
    src/poisson_solvers.cpp          # R1: sor_sweep_18pt + gauss/coulomb/latency solvers
    src/wilson_dirac.cpp             # Phase II.2: Wilson-Dirac matter sector (Branch-B)
    src/transmutation_phases.cpp     # R2: weak/pair-production/triad/proper-time
    src/energy_ledger_compute.cpp    # R3: per-tick conservation ledger
    src/diagnostics_compute.cpp      # R4: diagnostics / energy audit / EM fields / Poynting / entropy
    src/injection.cpp                # R5: inject_flux/particle/wavepacket + aggregate profile
    # Phase 4a — phase_write decomposition (2026-04-27, ADR-0008)
    src/render_bridge_phases/phase_write.cpp  # leapfrog + damping + genesis + evaporation
    # Phase 4b — phase_forces decomposition (2026-04-27, ADR-0008)
    src/render_bridge_phases/phase_forces.cpp # EM/gravity/Lorentz/color + γ_FTD integration
    # Phase 4c — phase_read + phase_movement decomposition (2026-04-27, ADR-0008)
    src/render_bridge_phases/phase_read.cpp     # 18-pt isotropic Laplacian + state-flux coupling
    src/render_bridge_phases/phase_movement.cpp # drift + collision + annihilation
    src/backend.cpp                  # ARCH-2: CpuBackend / GpuBackend abstraction over use_gpu_ ifdef
    src/bridge_rng.cpp               # RF-9: PIMPL'd <random> state to keep render_bridge.h slim
    # Cosmic engine split (CE1-CE5, 2026-04-19)
    src/cosmic/cosmic_scenarios.cpp           # setup_spiral_galaxy, galaxy_cluster, cosmic_web, etc.
    src/cosmic/cosmic_barnes_hut.cpp          # octree + tree_force + compute_gravity
    src/cosmic/cosmic_sph.cpp                 # SPH kernel, neighbors, density, forces
    src/cosmic/cosmic_cosmology.cpp           # Friedmann/Hubble/dark energy
    src/cosmic/cosmic_gravitational_waves.cpp # GW detection + propagation
    # Atom engine split (AE1-AE3, 2026-04-19)
    src/atom/atom_forces.cpp         # compute_all_forces + pairwise/tree force helpers
    src/atom/atom_bonding.cpp        # check_bonding
    src/atom/atom_thermostat.cpp     # speed limit, damping, thermostat, dipole moments
)
target_include_directories(ftd_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/sim/include   # Phase B: Pipeline<Backend> + observables
)
if(OpenMP_CXX_FOUND)
    target_link_libraries(ftd_core PUBLIC OpenMP::OpenMP_CXX)
endif()
# Both ftd_core and ftd_cuda PUBLIC-depend on ftd_eft so libftd_eft.a appears
# AFTER both consumers in the generated link line (CMake topological sort
# places leaves last). This satisfies gpu_engine.cu's DualCellContinuity
# reference at the linker's ftd_cuda → ftd_eft scan transition. See ftd_eft
# definition above.
target_link_libraries(ftd_core PUBLIC ftd_eft)
if(FTD_ENABLE_CUDA)
    target_link_libraries(ftd_core PUBLIC ftd_cuda)
endif()

# Main simulation executable (native builds only)
if(NOT EMSCRIPTEN)
add_executable(ftd_sim
    src/main.cpp
    src/cli_demos/cli_demo_scenarios.cpp  # M1: 10 scenario_*() demos extracted from main.cpp
)
target_link_libraries(ftd_sim ftd_core)
endif()

# WebSocket server executable (native builds only — bridges engine to web dashboard)
if(NOT EMSCRIPTEN)
add_executable(ws_server
    src/ws_server.cpp
    src/ws_protocol.cpp   # WS2: framing + recv/send + JSON helpers split from ws_server.cpp
)
target_link_libraries(ws_server ftd_core)
if(WIN32)
    target_link_libraries(ws_server ws2_32)
endif()
endif()

# ============================================================================
# Unit Tests (native builds only)
# ============================================================================
if(NOT EMSCRIPTEN)

# ============================================================================
# Phase 7 (2026-04-27): Test support static library
# ----------------------------------------------------------------------------
# Holds:
#   - tests/support/test_telemetry.cpp    — impl extracted from ftd/test_telemetry.h
#                                           (was 412 LOC inline; now declarations in
#                                           the header + bodies in this lib so test
#                                           TUs only see the small declaration set).
#   - tests/support/bridge_fixtures.cpp   — shared RenderBridge fixtures (make_bridge,
#                                           run_for, inject_particle_at_center,
#                                           assert_energy_conserved).
#
# Every test target links this library to pull in the test_telemetry impl. Tests
# that don't need bridge_fixtures still link it without penalty (the linker drops
# unreferenced object code). NO_CORE tests link only this lib (no ftd_core).
# ============================================================================
add_library(ftd_test_support STATIC
    tests/support/test_telemetry.cpp
    tests/support/bridge_fixtures.cpp
)
target_include_directories(ftd_test_support PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests/support
)
# bridge_fixtures.cpp uses RenderBridge; ftd_core supplies it.
target_link_libraries(ftd_test_support PUBLIC ftd_core)

# Auto-link ftd_test_support into every test executable defined below in this
# directory. Each test_*.cpp includes <ftd/test_telemetry.h>; the impl now
# lives in ftd_test_support. This avoids touching every existing
# `target_link_libraries(test_xxx ftd_core)` site individually. Targets
# defined BEFORE this point (the engine library, ftd_sim, ws_server) are
# unaffected because link_libraries() applies forward only.
link_libraries(ftd_test_support)

# ---- Test Telemetry self-test (uses ftd_add_test macro for first time) ----
# This test exercises the ftd::test NDJSON API. It doesn't need ftd_core.
ftd_add_test(test_telemetry_selftest tests/test_telemetry_selftest.cpp NO_CORE)
ftd_add_test(test_constructor_contract tests/test_constructor_contract.cpp
             NO_CORE LABELS constructor observable eft_quick)
ftd_add_test(test_native_observable_registry tests/test_native_observable_registry.cpp
             NO_CORE LABELS constructor observable eft_quick)

# Foundational
add_executable(test_constants tests/test_constants.cpp)
target_link_libraries(test_constants ftd_core)

# Audit regression tests (Wave-3 Agent G, 2026-04-27)
# Guards recent fixes that previously had no C++ coverage:
#   - ½ factor on field_energy / wave_energy / E_L / E_R / coulomb_pe
#   - locked-particle pair force does NOT skip unlocked partner
#   - E_L vs wv_L split (post-d0329f6 conflation fix)
#   - 18-pt Laplacian sum-rule
#   - coulomb_pe = ½·Σ α·q·φ pair-PE convention
add_executable(test_audit_regression tests/test_audit_regression.cpp)
target_link_libraries(test_audit_regression ftd_core)

# Phase 4 PRE-FLIGHT regression gate (2026-04-27)
# Before any phase extraction is allowed out of render_bridge.cpp, every
# extraction commit MUST reproduce the byte-hash captured in this test.
# 100-tick deterministic scenario at L=16, CPU-only, RNG seeded with 42.
# See `.claude/plans/i-want-to-try-crispy-charm.md` Phase 4 + ADR-0008.
ftd_add_test(test_render_bridge_golden tests/test_render_bridge_golden.cpp
             CTEST_NAME render_bridge_golden)

# Wave 4b.6: test_lorentz_force + test_lorentz_invariance + test_magnetic +
# test_magnetic_lagrangian merged into test_lorentz (self-ref target)
ftd_add_test(test_lorentz tests/test_lorentz.cpp CTEST_NAME lorentz TIMEOUT 600)

add_executable(test_lattice tests/test_lattice.cpp)
target_link_libraries(test_lattice ftd_core)

# Constructor library tests — see spec 2026-04-15
ftd_add_test(test_constructors tests/test_constructors.cpp)

# Class B Phase B.1 (FTD-0136 Discrete-Native Derivation Program):
# ClusterTracker connected-component identification + persistence-tracking.
# See docs/theory/01_reference/SPEC_CLASS_B_CLUSTER_PERSISTENCE.md.
ftd_add_test(test_cluster_tracker tests/test_cluster_tracker.cpp)

# Lagrangian verification
add_executable(test_born_infeld tests/test_born_infeld.cpp)
target_link_libraries(test_born_infeld ftd_core)

# (test_energy merged into test_energy_conservation via Wave 4a.3)

# Wave 4b.5 (2026-04-14): test_gauss_convergence merged into test_gauss
ftd_add_test(test_gauss tests/test_gauss.cpp CTEST_NAME gauss TIMEOUT 600)


add_executable(test_scenario_velocity_wiring tests/test_scenario_velocity_wiring.cpp)
target_link_libraries(test_scenario_velocity_wiring ftd_core)

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

# 2026-04-17 (consolidation sweep): empirical audit that the phase_write
# advance pair (wave_vel += Δ; flux += wave_vel) is symplectic leapfrog.
# Closes TRACKER_OPEN_ITEMS §1.4 on pass.
ftd_add_test(test_leapfrog_integrator_audit
             tests/test_leapfrog_integrator_audit.cpp
             CTEST_NAME leapfrog_integrator_audit TIMEOUT 120)

# 2026-04-17 (step 2): radial-symmetry benchmark on the 18-point
# Moore Laplacian. Closes TRACKER §1.8 on pass.
ftd_add_test(test_moore_laplacian_isotropy
             tests/test_moore_laplacian_isotropy.cpp
             CTEST_NAME moore_laplacian_isotropy TIMEOUT 120)

# 2026-04-17 (step 4): γ_FTD momentum integration replaces the old
# non-relativistic velocity clamp. Closes TRACKER §1.2.
ftd_add_test(test_gamma_ftd_momentum
             tests/test_gamma_ftd_momentum.cpp
             CTEST_NAME gamma_ftd_momentum TIMEOUT 120)

# 2026-04-17 (callstack audit): verifies the F1–F8 fixes — pair_production
# and triad_binding now run on CPU, accumulate_proper_time updates tau,
# ALPHA/ALPHA_EFT identity holds.

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

# EFT Recovery Program — Phase 1A (rotational-anisotropy diagnostics)
# See docs/theory/10_eft_program/SPEC_EFT_RECOVERY_PROGRAM.md §4.1
add_executable(test_eft_anisotropy tests/test_eft_anisotropy.cpp)
target_link_libraries(test_eft_anisotropy ftd_core)

# EFT Recovery Program — Phase 1B (Lorentz-recovery benchmark)
# See docs/theory/10_eft_program/SPEC_EFT_RECOVERY_PROGRAM.md §4.2
add_executable(benchmark_lorentz_recovery tests/benchmark_lorentz_recovery.cpp)
target_link_libraries(benchmark_lorentz_recovery ftd_core)

# EFT Recovery Program — Phase 1C (Ward-identity suite)
# See docs/theory/10_eft_program/SPEC_EFT_RECOVERY_PROGRAM.md §4.3
add_executable(test_eft_ward_identity tests/test_eft_ward_identity.cpp)
target_link_libraries(test_eft_ward_identity ftd_core)

# EFT Recovery Program — Phase 2A (block-spin validation gate)
# See docs/theory/10_eft_program/SPEC_EFT_RECOVERY_PROGRAM.md §5.1
add_executable(test_eft_blocking tests/test_eft_blocking.cpp)
target_link_libraries(test_eft_blocking ftd_core)

# EFT Recovery Program — Phase 2C (multi-scale α_eff → β extraction)
# Heavy: runs 9+ engine simulations at L ∈ {16, 32, 64}. Gated behind
# the `--quick` flag (≈ 15 s) for CTest; full run (≈ 90 s) manual.
# See docs/theory/10_eft_program/SPEC_EFT_RECOVERY_PROGRAM.md §5
add_executable(benchmark_beta_function tests/benchmark_beta_function.cpp)
target_link_libraries(benchmark_beta_function ftd_core)

# EFT Recovery Program — Phase 3 (operator-basis scaling dimensions)
# See docs/theory/10_eft_program/SPEC_OPERATOR_BASIS.md
add_executable(test_eft_operator_spectrum tests/test_eft_operator_spectrum.cpp)
target_link_libraries(test_eft_operator_spectrum ftd_core)

# EFT Recovery Program — Phase 4 (dynamical SM emergence; report-only)
# See docs/theory/10_eft_program/SPEC_EFT_RECOVERY_PROGRAM.md §7
add_executable(benchmark_dynamical_sm tests/benchmark_dynamical_sm.cpp)
target_link_libraries(benchmark_dynamical_sm ftd_core)

# EFT Day-2 Ticket 4 — EWSB amplitude threshold map + spectroscopy dump
add_executable(benchmark_ewsb_threshold_map tests/benchmark_ewsb_threshold_map.cpp)
target_link_libraries(benchmark_ewsb_threshold_map ftd_core)

# EFT Day-2 Ticket A — matched-stencil CG Poisson solver
add_executable(test_eft_matched_poisson tests/test_eft_matched_poisson.cpp)
target_link_libraries(test_eft_matched_poisson ftd_core)

# Phase H — explicit coupling g_c in Gauss law; tests alpha_r scaling.
# See docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md §7.
add_executable(test_phase_h_coupling tests/test_phase_h_coupling.cpp)
target_link_libraries(test_phase_h_coupling ftd_core)

# Phase I Item 3 Mechanism A — Wilson-loop topology diagnostic.
# Tests whether the flux field's circulation is quantised (i.e., can
# topology force g_c?). Expected: no (J is R^3-valued, not U(1)).
add_executable(test_wilson_topology tests/test_wilson_topology.cpp)
target_link_libraries(test_wilson_topology ftd_core)

# Sim Pipeline Phase B — CPU-backend pipeline + 3 reference observables
# Plan: ~/.claude/plans/vivid-marinating-pudding.md Phase B
add_executable(test_sim_pipeline_cpu sim/tests/test_sim_pipeline_cpu.cpp)
target_link_libraries(test_sim_pipeline_cpu ftd_core)

# Sim Pipeline Phase C — GPU/CPU parity test (SKIP gracefully on non-CUDA)
add_executable(test_sim_parity sim/tests/test_sim_parity.cpp)
target_link_libraries(test_sim_parity ftd_core)

# Sim Pipeline Phase D — observable library unit tests
add_executable(test_sim_observables sim/tests/test_sim_observables.cpp)
target_link_libraries(test_sim_observables ftd_core)

# Sim Pipeline Phase E — pipeline-based benchmarks
# benchmark_beta_function_pipe: Phase-E port of benchmark_beta_function,
# runs on either CPU or GPU via --gpu/--cpu flags
add_executable(benchmark_beta_function_pipe tests/benchmark_beta_function_pipe.cpp)
target_link_libraries(benchmark_beta_function_pipe ftd_core)

# benchmark_ewsb_pipe: Phase-E port of benchmark_ewsb_threshold_map
add_executable(benchmark_ewsb_pipe tests/benchmark_ewsb_pipe.cpp)
target_link_libraries(benchmark_ewsb_pipe ftd_core)

# EFT Day-2 Thread 4 — Rutherford scattering α extraction
add_executable(benchmark_rutherford_alpha tests/benchmark_rutherford_alpha.cpp)
target_link_libraries(benchmark_rutherford_alpha ftd_core)


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

    add_executable(test_gpu_continuity_ledger tests/test_gpu_continuity_ledger.cpp)
    target_link_libraries(test_gpu_continuity_ledger ftd_core ftd_cuda)

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

    # Phase II.2-E: Wilson-Dirac CPU/GPU parity
    add_executable(test_wilson_dirac_cuda_parity tests/test_wilson_dirac_cuda_parity.cpp)
    target_link_libraries(test_wilson_dirac_cuda_parity ftd_core ftd_cuda)
    add_test(NAME test_wilson_dirac_cuda_parity COMMAND test_wilson_dirac_cuda_parity)
    set_tests_properties(test_wilson_dirac_cuda_parity PROPERTIES TIMEOUT 120)
endif()

# ============================================================================
# Simulation Campaigns
# ============================================================================

# Wave 4b.7: test_dispersion_relation + campaign_dispersion_convergence
# merged into campaign_dispersion (self-ref target)
ftd_add_test(ftd_dispersion tests/campaign_dispersion.cpp
             CTEST_NAME campaign_dispersion GPU_HEAVY TIMEOUT 1800)



add_executable(ftd_free_dynamics tests/campaign_free_dynamics.cpp)
target_link_libraries(ftd_free_dynamics ftd_core)


# (campaign_poisson_force_law + campaign_poisson_binding merged into campaign_coulomb_force_law)

# (campaign_poisson_hydrogen merged into campaign_hydrogen_spectrum)

# (campaign_force_law merged into campaign_coulomb_force_law)

add_executable(ftd_shell_predictions tests/campaign_shell_predictions.cpp)
target_link_libraries(ftd_shell_predictions ftd_core)


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
ftd_add_test(ftd_phase_i_native_coupling tests/benchmark_phase_i_native_coupling.cpp
             CTEST_NAME benchmark_phase_i_native_coupling TIMEOUT 1800)
# 2026-05-03: FTD-0125 outcome-C pre-registered falsification (commit 7d421ef).
# The benchmark's exit-code FAIL is the correct epistemic state: the
# pre-registered hypothesis (g_engine² = 1/x_+ to 1e-3 tolerance) is
# empirically falsified. Mark WILL_FAIL so CTest reports green when the
# falsification reproduces.
set_tests_properties(benchmark_phase_i_native_coupling PROPERTIES WILL_FAIL TRUE)
ftd_add_test(test_wilson_dirac_smoke tests/test_wilson_dirac_smoke.cpp
             CTEST_NAME test_wilson_dirac_smoke TIMEOUT 60)
ftd_add_test(test_wilson_dirac_bz_spectrum tests/test_wilson_dirac_bz_spectrum.cpp
             CTEST_NAME test_wilson_dirac_bz_spectrum TIMEOUT 120)
ftd_add_test(test_wilson_dirac_gauge tests/test_wilson_dirac_gauge.cpp
             CTEST_NAME test_wilson_dirac_gauge TIMEOUT 120)
ftd_add_test(test_wilson_dirac_limit tests/test_wilson_dirac_limit.cpp
             CTEST_NAME test_wilson_dirac_limit TIMEOUT 60)
ftd_add_test(benchmark_dirac_electron_in_B tests/benchmark_dirac_electron_in_B.cpp
             CTEST_NAME benchmark_dirac_electron_in_B TIMEOUT 600)
ftd_add_test(ftd_budget_equation tests/benchmark_budget_equation.cpp
             CTEST_NAME benchmark_budget_equation TIMEOUT 1800)
ftd_add_test(ftd_bh_thermo tests/benchmark_black_hole_thermo.cpp
             CTEST_NAME benchmark_bh_thermo TIMEOUT 1800)

# Native EFT/response tests (Apr 2026). These are fixed audits, not target
# searches, and form the baseline for the FTD-native electrodynamics program.
ftd_add_test(test_native_continuity tests/test_native_continuity.cpp
             CTEST_NAME native_continuity TIMEOUT 600 LABELS native eft)
ftd_add_test(test_native_reaction_ledger tests/test_native_reaction_ledger.cpp
             CTEST_NAME native_reaction_ledger TIMEOUT 600 LABELS native eft)
ftd_add_test(test_native_conserved_parent tests/test_native_conserved_parent.cpp
             CTEST_NAME native_conserved_parent TIMEOUT 600 LABELS native eft)
ftd_add_test(test_native_manifestation_ledger tests/test_native_manifestation_ledger.cpp
             CTEST_NAME native_manifestation_ledger TIMEOUT 600 LABELS native eft)
ftd_add_test(test_native_source_response tests/test_native_source_response.cpp
             CTEST_NAME native_source_response TIMEOUT 600 LABELS native eft)
ftd_add_test(test_native_projection_convergence tests/test_native_projection_convergence.cpp
             CTEST_NAME native_projection_convergence TIMEOUT 600 LABELS native eft)
ftd_add_test(test_native_source_core_fork tests/test_native_source_core_fork.cpp
             CTEST_NAME native_source_core_fork TIMEOUT 600 LABELS native eft)
ftd_add_test(test_native_dual_cell_gauss tests/test_native_dual_cell_gauss.cpp
             CTEST_NAME native_dual_cell_gauss TIMEOUT 600 LABELS native eft)
# Adapter from RenderBridge to DualCellFields (Plan B reroute, 2026-04-23).
ftd_add_test(test_dual_cell_adapter tests/test_dual_cell_adapter.cpp
             CTEST_NAME dual_cell_adapter TIMEOUT 120 LABELS native eft)
ftd_add_test(test_native_dual_half_shell tests/test_native_dual_half_shell.cpp
             CTEST_NAME native_dual_half_shell TIMEOUT 600 LABELS native eft)
ftd_add_test(test_native_moore_shell_gauss tests/test_native_moore_shell_gauss.cpp
             CTEST_NAME native_moore_shell_gauss TIMEOUT 600 LABELS native eft)
ftd_add_test(test_native_moore_layer_coupling tests/test_native_moore_layer_coupling.cpp
             CTEST_NAME native_moore_layer_coupling TIMEOUT 600 LABELS native eft)
ftd_add_test(test_native_moore_temporal_layers tests/test_native_moore_temporal_layers.cpp
             CTEST_NAME native_moore_temporal_layers TIMEOUT 600 LABELS native eft)
ftd_add_test(test_native_blocking_map tests/test_native_blocking_map.cpp
             CTEST_NAME native_blocking_map TIMEOUT 120
             LABELS native eft constructor blocking observable eft_quick)
ftd_add_test(test_native_flow tests/test_native_flow.cpp
             CTEST_NAME native_flow TIMEOUT 120 LABELS native eft)
ftd_add_test(test_native_current_flow tests/test_native_current_flow.cpp
             CTEST_NAME native_current_flow TIMEOUT 120 LABELS native eft)
ftd_add_test(test_native_response_flow tests/test_native_response_flow.cpp
             CTEST_NAME native_response_flow TIMEOUT 120 LABELS native eft)
ftd_add_test(test_native_engine_history_flow tests/test_native_engine_history_flow.cpp
             CTEST_NAME native_engine_history_flow TIMEOUT 120 LABELS native eft)
ftd_add_test(test_native_engine_transport_flow tests/test_native_engine_transport_flow.cpp
             CTEST_NAME native_engine_transport_flow TIMEOUT 120
             LABELS native eft constructor ledger blocking eft_quick)

# Fractional heat flow test (Euler ratio continuum limit verification)
ftd_add_test(test_fractional_heat_flow tests/test_fractional_heat_flow.cpp
             CTEST_NAME fractional_heat_flow TIMEOUT 600 LABELS native eft)

# Manifestation background unit tests (Plan B scale-flow campaign).
ftd_add_test(test_manifestation_background tests/test_manifestation_background.cpp
             CTEST_NAME manifestation_background TIMEOUT 120 LABELS native eft)

# Local repro (do not commit) for GPU empty-bridge crash triage.
ftd_add_test(_repro_gpu_empty_bridge tests/_repro_gpu_empty_bridge.cpp
             CTEST_NAME _repro_gpu_empty_bridge TIMEOUT 120 LABELS native eft)

# Langevin thermostat equipartition (FTD-0051 acceptance test).
# Verifies <|wave_vel|^2> = 3T on bare lattice single-substrate path.
ftd_add_test(test_langevin_equipartition tests/test_langevin_equipartition.cpp
             CTEST_NAME langevin_equipartition TIMEOUT 600 LABELS native eft)

# Cluster A — sublattice helpers (FTD-0093). Parity classification +
# neighbors_8_corner; load-bearing for Mechanism C / BCC sub-stencil
# experiment. See engine/include/ftd/sublattice.h.
ftd_add_test(test_sublattice_helpers tests/test_sublattice_helpers.cpp
             CTEST_NAME sublattice_helpers TIMEOUT 60 LABELS native eft)

# Cluster A — sublattice-projected Laplacians (FTD-0093). Verifies
# Taylor consistency, plane-wave eigenvalues, and dispatch agreement
# for SC / FCC / BCC / FULL stencil modes.
ftd_add_test(test_sublattice_laplacian tests/test_sublattice_laplacian.cpp
             CTEST_NAME sublattice_laplacian TIMEOUT 60 LABELS native eft)

# Cluster A — Langevin sublattice filter (FTD-0093). Verifies that with
# langevin_site_filter = BCC_SITES, only BCC voxels equilibrate to <|v|^2>=3T
# while SC/FCC voxels remain unfilled. Confound-check infrastructure for
# PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md.
ftd_add_test(test_langevin_sublattice_equipartition tests/test_langevin_sublattice_equipartition.cpp
             CTEST_NAME langevin_sublattice_equipartition TIMEOUT 600 LABELS native eft)

# Cluster A — sublattice + diagonal correlators (FTD-0093). Validates
# spatial_flux_correlation_sublattice and sum_flux_energy_sublattice for
# axis vs body-diagonal displacements and parity-class filters. Required
# infrastructure for the BCC band-spectrum campaign.
ftd_add_test(test_correlations_diagonal tests/test_correlations_diagonal.cpp
             CTEST_NAME correlations_diagonal TIMEOUT 60 LABELS native eft)

# Cluster A — two-state spectrum extractors (FTD-0093). Verifies Prony
# and GEVP extractors recover input eigenvalues to 1% on synthetic data,
# and that a wrong-ratio synthesis is rejected. Required for the
# campaign's master-quadratic falsifier (target ratio 45.31).
ftd_add_test(test_two_state_extraction tests/test_two_state_extraction.cpp
             CTEST_NAME two_state_extraction TIMEOUT 30 LABELS native eft)

# Cluster A — BCC band-spectrum campaign (FTD-0093). Smoke-test harness
# implementing PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md. Thermalizes σ_BCC
# (and SC/FCC/FULL controls), extracts two-state Prony spectrum, emits
# CSV. Smoke parameters L=16, n_seeds=2 fit within 1800 s; publication
# run needs L>=24, n_seeds>=8 and a separately-tagged invocation.
ftd_add_test(campaign_bcc_band_spectrum tests/campaign_bcc_band_spectrum.cpp
             CTEST_NAME bcc_band_spectrum TIMEOUT 1800 LABELS native eft)

# Flux-slice propagation diagnostic (FTD-0092 companion). Real-space
# 2D slice (xy / xz / yz) capture of a centred Gaussian flux pulse
# under wave-only dynamics. Complements the spectral Lorentz-isotropy
# measurement with a visual wavefront-symmetry check. Runs on GPU.
ftd_add_test(campaign_flux_slice_propagation tests/campaign_flux_slice_propagation.cpp
             CTEST_NAME flux_slice_propagation TIMEOUT 600 LABELS native)

# β-function measurement protocol smoke test (Phase 2 of EFT Recovery
# Program). Implements PROTOCOL_BETA_MEASUREMENT.md: Langevin thermal
# ensemble → measure_alpha_eff_on_bg at two lattice sizes → extract β
# = (g(L/b) − g(L)) / log(b). Smoke-test parameters (L=32, 4 seeds,
# 5000-tick burn, 200-tick probe) keep CPU wall time within 1800 s;
# publishable measurement requires WSL2 + larger ensemble.
ftd_add_test(campaign_beta_measurement tests/campaign_beta_measurement.cpp
             CTEST_NAME beta_measurement TIMEOUT 1800 LABELS native eft)

# Walsh–Hadamard / Clifford anticommutator measurement (FTD-0061 falsifier).
# Excites the three weight-1 WH modes on a 2^3 block, applies in two orders,
# measures whether the engine-induced product satisfies {e_i,e_j} = 2δ_{ij}·1.
# See docs/theory/09_mathematical/DERIV_WH_ALGEBRA_VS_CLIFFORD_NOGO.md §7.
ftd_add_test(test_wh_clifford_anticommutator tests/test_wh_clifford_anticommutator.cpp
             CTEST_NAME wh_clifford_anticommutator TIMEOUT 120 LABELS gpu native eft)

# Mixed-toggle multi-tick Ward identity (P1.3 + P1.4 closure, Gates 4 + 5).
# Runs 10 ticks with genesis + pair_production + movement + forces
# simultaneously, verifies Δ_t ρ + div j = S_R to 1e-12 per tick and over
# the accumulated interval, including after b=2 blocking. First non-Gaussian
# flow data on the Moore-26 stencil under mixed non-linearities.
ftd_add_test(test_mixed_history_flow tests/test_mixed_history_flow.cpp
             CTEST_NAME mixed_history_flow TIMEOUT 120 LABELS gpu native eft)

# Phase-2 multi-scale nonlinear flow measurement (Gates 4 + 7 closure).
# Langevin ensemble + genesis at L=16, measures native tuple at b ∈ {1,2,4,8}
# with ensemble uncertainties. First multi-scale RG flow measurement on the
# FTD engine under genuine non-linearities.
ftd_add_test(test_nonlinear_flow_multiscale tests/test_nonlinear_flow_multiscale.cpp
             CTEST_NAME nonlinear_flow_multiscale TIMEOUT 300 LABELS gpu native eft)

# FTD-0102 (2026-04-27): engine-as-instrument flagship — emergent particle
# spectrum from generic initial conditions. 5 IC classes × 5 seeds at L=32.
# Pre-registered in PROTOCOL_EMERGENT_PARTICLE_SPECTRUM.md. Output: per-IC
# clustering history + stable-cluster mass histogram + volumetric snapshots.
ftd_add_test(campaign_emergent_spectrum tests/campaign_emergent_spectrum_2026-04-27.cpp
             CTEST_NAME campaign_emergent_spectrum TIMEOUT 7200
             LABELS native eft observable engine_as_instrument)

# FTD-0107 regression + L¹-ball-radius-2 topology verification.
# Confirms the deterministic 25-voxel cluster (FTD-0102/0107) and tests the
# EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md topology hypothesis directly via
# per-voxel positional dump.
ftd_add_test(test_emergent_ic1_topology tests/test_emergent_ic1_topology.cpp
             CTEST_NAME emergent_ic1_topology TIMEOUT 600
             LABELS native eft observable engine_as_instrument regression)

# FTD-0110 Bridge-II regime-4 (temporal/frequency) variance test:
# per-tick cluster_history logging at custom amplitude. Used to test
# whether the cluster has nonzero temporal entropy contribution at
# active-boundary amplitudes (A ≥ 30 where boundary becomes free).
# See DERIV_FTD0110_VARIANCE_ENTROPY.md.
ftd_add_test(campaign_amplitude_time_series tests/campaign_amplitude_time_series.cpp
             CTEST_NAME campaign_amplitude_time_series TIMEOUT 1800
             LABELS native eft observable engine_as_instrument)

# FTD-0104 (2026-04-27): Topological observable mapping (Campaign D of the
# engine-as-instrument portfolio). Four sub-experiments — Wilson loop,
# flux tube, monopole, vacuum instanton — with shared per-snapshot CSV.
# Pre-registered in PROTOCOL_TOPOLOGICAL_OBSERVABLES.md.
ftd_add_test(campaign_topological_observables tests/campaign_topological_observables_2026-04-27.cpp
             CTEST_NAME campaign_topological_observables TIMEOUT 7200
             LABELS native eft observable engine_as_instrument)

# FTD-0098 (2026-04-26): first measured native operator-mixing matrix M_ab(b=2).
# Pre-registered in docs/theory/10_eft_program/PROTOCOL_OPERATOR_MIXING_MATRIX.md.
# Reuses the test_nonlinear_flow_multiscale ensemble (Langevin + gauss + genesis,
# L=16, 40 samples × 5-tick stride, 5 seeds) and extracts the 6-operator basis
# moments before/after b=2 blocking, solving for M_ab via the Wilsonian normal
# equations with bootstrap stderr.
ftd_add_test(campaign_operator_mixing tests/campaign_operator_mixing_2026-04-26.cpp
             CTEST_NAME campaign_operator_mixing TIMEOUT 1800
             LABELS native eft blocking observable eft_quick)

# FTD-0112: reaction-sector operators (O7-O10) unit tests.
# Pre-registration: docs/theory/10_eft_program/PROTOCOL_S_EFF_NONLINEAR_CAMPAIGN.md
# Header: engine/include/ftd/eft/reaction_operators.h
# Validates SnapshotPair-based per-tick state-delta evaluators on
# deterministic synthetic inputs before they enter the L=32/L=64 campaign.
ftd_add_test(test_reaction_operators tests/test_reaction_operators.cpp
             CTEST_NAME reaction_operators TIMEOUT 30
             LABELS native eft eft_quick)

# FTD-0112: nonlinear S_eff campaign binary.
# Pre-registration: docs/theory/10_eft_program/PROTOCOL_S_EFF_NONLINEAR_CAMPAIGN.md
# Tag: preregister-s-eff-nonlinear-v1
# Extends FTD-0098's 6-op M_ab(b=2) with the 4 reaction-sector ops via
# snapshot-pair sampling; supports --scenario={langevin-pure, genesis-rich,
# pair-rich, mixed-balanced}, --L, --N-seeds, --N-samples, --b4, --smoke.
# Long-running production: 5-8 hours per L size on RTX 5090; smoke variant
# completes in <30s.
ftd_add_test(campaign_s_eff_nonlinear tests/campaign_s_eff_nonlinear_2026-04-29.cpp
             CTEST_NAME campaign_s_eff_nonlinear TIMEOUT 28800
             LABELS native eft blocking observable s_eff_campaign)

# Phase-4 fermion-emergence alternative routes (FTD-0061 extension).
# Runs WH/Clifford anticommutator measurement under three alternative
# non-linearities: pair_production (4a), weak_transmutation (4b),
# velocity-driven movement + forces (4d). Tests whether ANY non-genesis
# route produces Clifford structure on the 2³ block.
ftd_add_test(test_wh_clifford_alt_routes tests/test_wh_clifford_alt_routes.cpp
             CTEST_NAME wh_clifford_alt_routes TIMEOUT 120 LABELS gpu native eft)

# Phase-4c: Moore-26 / 3³ block Clifford falsifier.
# Uses axial dipole modes (sawtooth) on a 3³ = 27-site block containing the
# full Moore-26 neighborhood + center. Tests whether the richer 27-dim
# grade structure supports Clifford where the 8-dim 2³ block doesn't.
ftd_add_test(test_moore26_clifford_test tests/test_moore26_clifford_test.cpp
             CTEST_NAME moore26_clifford_test TIMEOUT 120 LABELS gpu native eft)

# Phase-4e: spin-field WH/Clifford anticommutator test.
# Reads the spin field (curl-assigned) rather than state field (divergence-
# assigned). Off-axis injections induce non-trivial curl and hence non-trivial
# spin. Tests whether spin-field readout preserves mode information that
# state-field readout loses.
ftd_add_test(test_spin_field_clifford tests/test_spin_field_clifford.cpp
             CTEST_NAME spin_field_clifford TIMEOUT 120 LABELS gpu native eft)

# Phase-4f: flux 1-form (link) WH/Clifford anticommutator test.
# Reads the flux field's three components separately as 1-form degrees of
# freedom, WH-decomposes each, tests for Clifford structure on axial
# link operators L_i = WH[flux_i, χ_{axis_i}].
ftd_add_test(test_flux_link_clifford tests/test_flux_link_clifford.cpp
             CTEST_NAME flux_link_clifford TIMEOUT 120 LABELS gpu native eft)

# Phase-4g: flux propagator classification on Langevin ensemble.
# Measures the 2-point flux correlator G_ij(r) along the x-axis on
# thermalized Langevin ensemble; fits power-law decay; classifies as
# bosonic-vector (α≈1), fermionic (α≈2), or anomalous.
ftd_add_test(test_flux_propagator tests/test_flux_propagator.cpp
             CTEST_NAME flux_propagator TIMEOUT 600 LABELS gpu native eft)

# Watson integrals for phenomenal/noumenal bridge test (FTD-0078).
# Numerical quadrature of SC, BCC, FCC, Moore-18 lattice Green's functions
# at origin; tests whether W_M18/W_BCC ratio matches 27/8 = 3.375 block-
# volume hypothesis or is close to unity (Watson-integral bulk insensitivity).
ftd_add_test(test_watson_integrals tests/test_watson_integrals.cpp
             CTEST_NAME watson_integrals TIMEOUT 120 LABELS unit eft)

# Master quadratic bare algebraic identities (Vieta, means, normalized form).
# Verifies α + 1/N_c = 1/G* to machine precision and the three-means
# geometric progression.
ftd_add_test(test_master_quadratic_identities
             tests/test_master_quadratic_identities.cpp
             CTEST_NAME master_quadratic_identities TIMEOUT 60 LABELS unit eft)

# Program E: uniqueness-of-minimal-polynomial proof. Enumerates all monic
# quadratics with coefficients in the bounded G*-integer class
# {i * G*^k : |i| <= 16, k in [0,4]} and verifies only (16 G*^2, 16 G*^3)
# satisfies the dual root match (x_+ = 1/alpha, x_- = N_c). Closes SP2
# as [THEOREM] modulo the L-value bound |Aut(E_i)|^2 = 16.
ftd_add_test(test_master_quadratic_uniqueness
             tests/test_master_quadratic_uniqueness.cpp
             CTEST_NAME master_quadratic_uniqueness TIMEOUT 120 LABELS unit eft)

# Program A (partial closure): ladder-walk step sizes from O_h structure.
# Verifies the O_h irrep decomposition of the Moore-26 shells and enumerates
# 4-part partitions of 16 from {3, 4, 6} under the "all features present"
# constraint — showing {3, 3, 4, 6} is unique. Step-size multiset becomes
# [THEOREM]; step ORDER remains [SELECTION].
ftd_add_test(test_ladder_walk_from_oh
             tests/test_ladder_walk_from_oh.cpp
             CTEST_NAME ladder_walk_from_oh TIMEOUT 60 LABELS unit eft)

# Program F: link-bilinear fermion probe. Runs full non-local dynamics
# (forces + triad + strong + exchange + pair_production + weak_transmutation)
# with both linear WH readout and bilinear link readout (J_i(x)*J_i(x+hat_i)).
# Tests whether non-local dynamics + bilinear readout produces Clifford
# anticommutation, or whether the no-go (FTD-0073) extends here too.
ftd_add_test(test_link_bilinear_clifford
             tests/test_link_bilinear_clifford.cpp
             CTEST_NAME link_bilinear_clifford TIMEOUT 120
             LABELS gpu native eft)

# Program F-prime: plaquette bivector probe. Tests the natural Cl(3,0)
# bivector basis: P_{ij}(x) = J_i(x)*J_j(x+hat_i) - J_i(x+hat_j)*J_j(x).
# If commutator [E_i, E_j] concentrates on P_{ij} and SU(2) signature
# emerges, fermions shift from Branch-B selection to Branch-A derivation.
ftd_add_test(test_plaquette_bivector_clifford
             tests/test_plaquette_bivector_clifford.cpp
             CTEST_NAME plaquette_bivector_clifford TIMEOUT 120
             LABELS gpu native eft)

# Program F-double-prime: bivector algebra closure tests.
# Three tests on the F-prime plaquette-bivector algebra:
#   A) multi-seed robustness (8 seeds x 3 pairs)
#   B) Casimir uniformity (e_i^2 scalar grade axis-isotropy)
#   D) bivector commutator closure ([B_a, B_b] on B_c via 8-sequence
#      4-injection linear combinations).
# If 3/3 closure tests pass, su(2) bivector subalgebra is confirmed.
ftd_add_test(test_bivector_closure
             tests/test_bivector_closure.cpp
             CTEST_NAME bivector_closure TIMEOUT 300
             LABELS gpu native eft)

# Path 1: Cl(3,0) multi-grade decomposition. Tests whether F-prime's
# bivector signature is part of a coherent grade structure: scalar (S),
# vector (V_i), bivector (P_ij), pseudoscalar (T). For 2-axis injection
# (f, g), Cl(3,0) predicts pseudoscalar near zero (third axis absent),
# vector on active axes, bivector concentrated on P_fg.
ftd_add_test(test_clifford_multigrade
             tests/test_clifford_multigrade.cpp
             CTEST_NAME clifford_multigrade TIMEOUT 120
             LABELS gpu native eft)

# Determinism regression: same seed + same protocol -> bit-identical voxel
# arrays. Closes a structural test-coverage hole flagged by the 2026-04-25
# audit (no existing test asserts byte-identical reproducibility, despite
# multi-seed campaigns assuming it).
ftd_add_test(test_determinism
             tests/test_determinism.cpp
             CTEST_NAME determinism TIMEOUT 60 LABELS unit fast)

# Closed-negative regression guards. Pins FTD-0061/0073 (mode-erasure) and
# FTD-0074 (flux 1-form separable). If the engine drifts to break either
# no-go signature, the test fails — preventing silent re-opening of closed
# claims.
ftd_add_test(test_closed_negatives
             tests/test_closed_negatives.cpp
             CTEST_NAME closed_negatives TIMEOUT 60 LABELS unit fast)

# CPU-only warning contract. Verifies that GPU-only toggles (strong_force,
# exchange_force) emit stderr warnings on CPU builds AND are genuine no-ops
# (voxel state byte-identical to baseline).
ftd_add_test(test_cpu_warnings
             tests/test_cpu_warnings.cpp
             CTEST_NAME cpu_warnings TIMEOUT 60 LABELS unit fast)

# ARCH-3: strict-validation contract for TermToggles. Verifies that:
#   (a) permissive mode (default) dedups warnings to one per unique string;
#   (b) strict_validation=true causes tick() to throw std::logic_error;
#   (c) the dedup memo resets when toggles transition from invalid to valid.
ftd_add_test(test_strict_validation
             tests/test_strict_validation.cpp
             CTEST_NAME strict_validation TIMEOUT 30 LABELS unit fast)

# OPEN-5 micro-regression: legacy single-substrate inject_flux must NOT
# populate flux_L. Pinned separately from test_dual_substrate (which has
# unrelated DS-CHIRALITY failures still under triage).
ftd_add_test(test_open5_legacy_flux_l
             tests/test_open5_legacy_flux_l.cpp
             CTEST_NAME open5_legacy_flux_l TIMEOUT 30 LABELS unit fast)

# TEST-007: tight floating-point energy conservation. Pure-wave + symplectic
# leapfrog should preserve energy at machine precision; asserts |dE/E| per
# tick < 1e-12 over 5000 ticks. Catches per-tick leaks invisible at the 1e-10
# tolerance most other audits use.
ftd_add_test(test_energy_conservation_tight
             tests/test_energy_conservation_tight.cpp
             CTEST_NAME energy_conservation_tight TIMEOUT 60
             LABELS unit fast)

# TEST-005: pairwise toggle-combination smoke test. 78 (i, j) pairs of OFF-
# default toggles, each runs 5 ticks; asserts no crash, no NaN, validator
# rejects cleanly. Catches "toggle X + toggle Y silently misbehave when
# combined" — the most common failure mode for new engine extensions.
ftd_add_test(test_toggle_matrix
             tests/test_toggle_matrix.cpp
             CTEST_NAME toggle_matrix TIMEOUT 120
             LABELS unit fast)

# TEST-008: Langevin equipartition + GPU/CPU statistical parity. Scans
# (gamma, T) pairs on both backends; asserts <|wave_vel|^2> = 3T within 10%
# AND CPU/GPU mean energies agree within 10% (stochastic — strict bit-parity
# not possible).
ftd_add_test(test_langevin_gpu_cpu_parity
             tests/test_langevin_gpu_cpu_parity.cpp
             CTEST_NAME langevin_gpu_cpu_parity TIMEOUT 300
             LABELS unit native gpu eft)

# TEST-009: Phase-H coulomb_charge_coupling regression. Pins the three
# documented values (1.0, 0.2141, 0.3028) by asserting they produce
# distinct field energies and never NaN out.
ftd_add_test(test_phase_h_regression
             tests/test_phase_h_regression.cpp
             CTEST_NAME phase_h_regression TIMEOUT 60
             LABELS unit fast)

# Phase-4h: Material emergence — catalogs the quantum numbers of the
# smallest particle that manifests from a single genesis event, compares
# to SM quark / electron.
ftd_add_test(test_smallest_particle_emergence
             tests/test_smallest_particle_emergence.cpp
             CTEST_NAME smallest_particle_emergence TIMEOUT 60
             LABELS gpu native eft)

# Phase-4i: 3-quark binding + color transformation structure test.
# Stamps RGB vs RRR vs RGG quark triads, measures binding. Also tests
# O_h permutation symmetry of color by comparing (R,G,B) to (G,B,R).
ftd_add_test(test_color_binding_and_structure
             tests/test_color_binding_and_structure.cpp
             CTEST_NAME color_binding_and_structure TIMEOUT 60
             LABELS gpu native eft)

# Emergent measurement probes (alpha, Larmor, wave group velocity, Gauss).
# EM2 reframed 2026-04-23 as Coulomb-solver consistency check, not alpha derivation.
ftd_add_test(test_emergent_measurements tests/test_emergent_measurements.cpp
             CTEST_NAME emergent_measurements TIMEOUT 600 LABELS native eft)

# Mechanism B: Vacuum Polarization
ftd_add_test(test_mechanism_b tests/test_mechanism_b.cpp
             CTEST_NAME test_mechanism_b TIMEOUT 600 LABELS native eft)

# Baseline EFT/GPU benchmarks. Registered with CTest so the benchmark inventory
# is visible, but still treated as diagnostics rather than acceptance tests for
# physical constants.
ftd_add_test(benchmark_nucleon_mass tests/benchmark_nucleon_mass.cpp
             CTEST_NAME benchmark_nucleon_mass TIMEOUT 1800 LABELS benchmark eft)
# Manifestation scale-flow Tier-1 CPU harness (single-seed measurement).
# Called by scripts/exploration/measure_manifestation_flow_quickcheck.py.
ftd_add_test(benchmark_manifestation_flow_cpu
             tests/benchmark_manifestation_flow_cpu.cpp
             CTEST_NAME benchmark_manifestation_flow_cpu TIMEOUT 600
             LABELS benchmark eft)
ftd_add_test(benchmark_alpha_convergence tests/benchmark_alpha_convergence.cpp
             CTEST_NAME benchmark_alpha_convergence TIMEOUT 7200 LABELS benchmark eft)
if(FTD_ENABLE_CUDA)
    add_executable(benchmark_alpha_window_lean_gpu
                   tests/benchmark_alpha_window_lean_gpu.cu)
    target_link_libraries(benchmark_alpha_window_lean_gpu PRIVATE CUDA::cudart)
    target_compile_options(benchmark_alpha_window_lean_gpu PRIVATE
        $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>
        $<$<COMPILE_LANGUAGE:CUDA>:--allow-unsupported-compiler>
    )
    set_target_properties(benchmark_alpha_window_lean_gpu PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_RESOLVE_DEVICE_SYMBOLS ON
        COMPILE_PDB_NAME ""
        COMPILE_PDB_OUTPUT_DIRECTORY ""
    )
    add_test(NAME benchmark_alpha_window_lean_gpu
             COMMAND benchmark_alpha_window_lean_gpu)
    set_tests_properties(benchmark_alpha_window_lean_gpu PROPERTIES
        TIMEOUT 1800
        LABELS "benchmark;eft;gpu")

    add_executable(benchmark_alpha_relaxation_lean_gpu
                   tests/benchmark_alpha_relaxation_lean_gpu.cu)
    target_link_libraries(benchmark_alpha_relaxation_lean_gpu PRIVATE CUDA::cudart)
    target_compile_options(benchmark_alpha_relaxation_lean_gpu PRIVATE
        $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>
        $<$<COMPILE_LANGUAGE:CUDA>:--allow-unsupported-compiler>
    )
    set_target_properties(benchmark_alpha_relaxation_lean_gpu PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_RESOLVE_DEVICE_SYMBOLS ON
        COMPILE_PDB_NAME ""
        COMPILE_PDB_OUTPUT_DIRECTORY ""
    )
    add_test(NAME benchmark_alpha_relaxation_lean_gpu
             COMMAND benchmark_alpha_relaxation_lean_gpu --iters 800 64)
    set_tests_properties(benchmark_alpha_relaxation_lean_gpu PROPERTIES
        TIMEOUT 1800
        LABELS "benchmark;eft;gpu")

    ftd_add_test(benchmark_nucleon_mass_gpu tests/benchmark_nucleon_mass_gpu.cpp
                 CTEST_NAME benchmark_nucleon_mass_gpu GPU_HEAVY TIMEOUT 1800
                 LABELS benchmark eft)
    ftd_add_test(benchmark_sm_masses_gpu tests/benchmark_sm_masses_gpu.cpp
                 CTEST_NAME benchmark_sm_masses_gpu GPU_HEAVY TIMEOUT 1800
                 LABELS benchmark eft)

    # GPU Langevin port (FTD-0051): thermal-ensemble alpha scan + wall-time bench.
    ftd_add_test(benchmark_alpha_scaling tests/benchmark_alpha_scaling.cpp
                 CTEST_NAME benchmark_alpha_scaling GPU_HEAVY TIMEOUT 7200
                 LABELS benchmark eft)
    ftd_add_test(benchmark_langevin_gpu tests/benchmark_langevin_gpu.cpp
                 CTEST_NAME benchmark_langevin_gpu GPU_HEAVY TIMEOUT 1800
                 LABELS benchmark eft)
endif()

add_executable(ftd_sm_observables tests/campaign_sm_observables.cpp)
target_link_libraries(ftd_sm_observables ftd_core)

# (campaign_wave_isotropy merged into campaign_wave_dynamics)

# Five Minds campaign tests (April 2026)
add_executable(ftd_plato tests/campaign_plato.cpp)
target_link_libraries(ftd_plato ftd_core)
add_executable(ftd_einstein tests/campaign_einstein.cpp)
target_link_libraries(ftd_einstein ftd_core)
add_executable(ftd_wigner tests/campaign_wigner.cpp)
target_link_libraries(ftd_wigner ftd_core)

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

# Orphaned tests registered by I-04 audit (Mar 2026)
add_executable(test_action_stationarity tests/test_action_stationarity.cpp)
target_link_libraries(test_action_stationarity PRIVATE ftd_core)
add_executable(test_asymptotic_freedom tests/test_asymptotic_freedom.cpp)
target_link_libraries(test_asymptotic_freedom PRIVATE ftd_core)
add_executable(test_force_diag_parity tests/test_force_diag_parity.cpp)
target_link_libraries(test_force_diag_parity PRIVATE ftd_core)
add_test(NAME force_diag_parity COMMAND test_force_diag_parity)
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
add_test(NAME audit_regression COMMAND test_audit_regression)
# (lorentz registered via ftd_add_test above)
add_test(NAME lattice COMMAND test_lattice)
add_test(NAME born_infeld COMMAND test_born_infeld)
# (test_energy merged into energy_conservation)
# (gauss registered via ftd_add_test above)
add_test(NAME scenario_velocity_wiring COMMAND test_scenario_velocity_wiring)
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
# EFT Recovery Program — Phase 1A
add_test(NAME eft_anisotropy COMMAND test_eft_anisotropy)
set_tests_properties(eft_anisotropy PROPERTIES LABELS "eft")
# EFT Recovery Program — Phase 1B (heavier: runs engine dynamics)
add_test(NAME eft_lorentz_recovery COMMAND benchmark_lorentz_recovery)
set_tests_properties(eft_lorentz_recovery PROPERTIES LABELS "eft" TIMEOUT 120)
# EFT Recovery Program — Phase 1C
add_test(NAME eft_ward_identity COMMAND test_eft_ward_identity)
set_tests_properties(eft_ward_identity PROPERTIES
    LABELS "eft;constructor;observable;eft_quick" TIMEOUT 60)
# EFT Recovery Program — Phase 2A
add_test(NAME eft_blocking COMMAND test_eft_blocking)
set_tests_properties(eft_blocking PROPERTIES
    LABELS "eft;constructor;blocking;observable;eft_quick" TIMEOUT 60)
# EFT Recovery Program — Phase 3
add_test(NAME eft_operator_spectrum COMMAND test_eft_operator_spectrum)
set_tests_properties(eft_operator_spectrum PROPERTIES LABELS "eft" TIMEOUT 120)
# EFT Recovery Program — Phase 4 (quick mode for CTest)
add_test(NAME eft_dynamical_sm COMMAND benchmark_dynamical_sm --quick)
set_tests_properties(eft_dynamical_sm PROPERTIES LABELS "eft" TIMEOUT 180)
# EFT Day-2 Ticket A
add_test(NAME eft_matched_poisson COMMAND test_eft_matched_poisson)
set_tests_properties(eft_matched_poisson PROPERTIES
    LABELS "eft;constructor;observable;eft_quick" TIMEOUT 60)
# Phase H — coupling scaling test
add_test(NAME eft_phase_h_coupling COMMAND test_phase_h_coupling)
set_tests_properties(eft_phase_h_coupling PROPERTIES LABELS "eft" TIMEOUT 180)
# Sim Pipeline Phase B
add_test(NAME sim_pipeline_cpu COMMAND test_sim_pipeline_cpu)
set_tests_properties(sim_pipeline_cpu PROPERTIES LABELS "sim" TIMEOUT 30)
# Sim Pipeline Phase C (parity — SKIPs on non-CUDA)
add_test(NAME sim_parity COMMAND test_sim_parity)
set_tests_properties(sim_parity PROPERTIES LABELS "sim" TIMEOUT 120)
# Sim Pipeline Phase D (observable library unit tests)
add_test(NAME sim_observables COMMAND test_sim_observables)
set_tests_properties(sim_observables PROPERTIES
    LABELS "sim;constructor;observable;eft_quick" TIMEOUT 60)
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
add_test(NAME measurement COMMAND test_measurement)
add_test(NAME particle_toggles COMMAND test_particle_toggles)
add_test(NAME spin_statistics COMMAND test_spin_statistics)
add_test(NAME triad_confinement COMMAND test_triad_confinement)
add_test(NAME wz_mass COMMAND test_wz_mass)
set_tests_properties(
    action_stationarity asymptotic_freedom atom_toggles atomic_energy
    baryogenesis bell_aggregate born_rule_ensemble confinement_test
    entanglement_basis flavor_physics higgs_mechanism
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
    add_test(NAME gpu_continuity_ledger COMMAND test_gpu_continuity_ledger)
    add_test(NAME gpu_physics COMMAND test_gpu_physics)
    add_test(NAME gpu_experiments COMMAND test_gpu_experiments)
    set_tests_properties(gpu_parity gpu_parity_complete gpu_benchmark
                         gpu_continuity_ledger gpu_physics PROPERTIES TIMEOUT 600)
    set_tests_properties(gpu_continuity_ledger PROPERTIES
                         LABELS "gpu;constructor;ledger;observable;eft_quick")
    set_tests_properties(gpu_experiments PROPERTIES TIMEOUT 1800)
endif()

# Campaign tests
# (campaign_dispersion registered via ftd_add_test above)
add_test(NAME campaign_free_dynamics COMMAND ftd_free_dynamics)
# (campaign_poisson_force_law + campaign_poisson_binding merged)
# (campaign_poisson_hydrogen merged)
# (campaign_force_law merged)
add_test(NAME campaign_shell_predictions COMMAND ftd_shell_predictions)
add_test(NAME campaign_bound_lifetime COMMAND ftd_bound_lifetime)
add_test(NAME campaign_spontaneous COMMAND ftd_spontaneous)
add_test(NAME campaign_plato COMMAND ftd_plato)
add_test(NAME campaign_einstein COMMAND ftd_einstein)
add_test(NAME campaign_wigner COMMAND ftd_wigner)
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
# (hydrogen_em_only merged)
add_test(NAME campaign_lorentz_measure COMMAND ftd_lorentz_measure)

# Timeouts for long-running tests (10 min)
# Skip under EMSCRIPTEN — these native tests aren't built for WASM.
if(NOT EMSCRIPTEN)
set_tests_properties(
    benchmark logic_engine energy_conservation
    selffield_profile wavepacket
    particle_lifetime
    campaign_dispersion
    campaign_bound_lifetime
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
    atom_engine_forces
    inflation dark_matter cosmological_constant consciousness sloop
    campaign_lorentz_measure
    helium_scale1 fine_structure_scale1
    radiative_decay_scale1
    PROPERTIES TIMEOUT 600)

# Heavy campaigns with 64^3 grids or multi-phase dynamics need more time (30 min)
# flux_mediated moved here from 600s group per I-06 audit
set_tests_properties(
    campaign_free_dynamics
    campaign_gravity_profile flux_mediated
    PROPERTIES TIMEOUT 1800)
endif() # NOT EMSCRIPTEN — end of timeout blocks

# ============================================================================
# CTest Labels — run subsets with: ctest -L <label>
# ============================================================================
# Skip the entire label section under EMSCRIPTEN — native tests aren't
# built for WASM, so set_property(TEST ...) would fail on nonexistent names.
if(NOT EMSCRIPTEN)

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
    entanglement_basis flavor_physics higgs_mechanism measurement
    particle_toggles spin_statistics triad_confinement wz_mass
    falsifiability inflation dark_matter cosmological_constant consciousness
    sloop
    APPEND PROPERTY LABELS "unit")

# Campaign tests (47 campaign_*.cpp files)
# APPEND is critical so campaign-level labels (scale1, scale2, gpu, etc.)
# stack on top of these category labels rather than overwriting them.
set_property(TEST
    campaign_dispersion
    campaign_free_dynamics
    campaign_coulomb_force_law
    campaign_bound_lifetime
    campaign_spontaneous campaign_cross_scale campaign_born_ensemble
    campaign_h2_molecule campaign_multiscale_pipeline
    campaign_statistical_convergence campaign_wave_dynamics
    campaign_born_rule
    campaign_hydrogen_binding campaign_triad_energy campaign_inertial_mass
    campaign_structure_stability
    campaign_weak_transmutation campaign_parity_violation campaign_weak_decay
    campaign_gravitational_wave campaign_gravity_profile campaign_gravity_hierarchy
    campaign_triad_binding campaign_neutrino_sector
    campaign_cosmological_predictions campaign_dark_sector campaign_novel_predictions
    campaign_integer_sweep campaign_hydrogen_spectrum
    campaign_pe_fine_structure campaign_lorentz_measure
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
    atom_toggles
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
        gpu_continuity_ledger
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
# RF-17: Speed labels (fast / slow / benchmark)
# ============================================================================
# fast  — typically <1 s (constants, lattice math, small unit tests)
# slow  — typically >10 s (large-grid or multi-phase simulations)
# benchmark — dedicated throughput/performance measurement suites
# campaign  — already labeled above; also mark as slow
# ============================================================================

# Fast tests: pure math / constants / small lattice
set_property(TEST
    constants ontic_chain born_infeld voxel_properties
    lattice_operators discrete_operators
    dual_substrate genesis
    action_stationarity asymptotic_freedom atom_toggles
    flavor_physics higgs_mechanism measurement
    spin_statistics wz_mass
    APPEND PROPERTY LABELS "fast")

# Slow tests: large grids, many ticks, multi-phase dynamics
set_property(TEST
    flux_mediated selffield_profile wavepacket
    particle_lifetime
    campaign_free_dynamics
    campaign_bound_lifetime
    campaign_spontaneous campaign_cross_scale campaign_born_ensemble
    campaign_h2_molecule campaign_multiscale_pipeline
    campaign_statistical_convergence campaign_wave_dynamics
    campaign_born_rule
    campaign_hydrogen_binding campaign_triad_energy campaign_inertial_mass
    campaign_structure_stability
    campaign_weak_transmutation campaign_parity_violation campaign_weak_decay
    campaign_gravitational_wave campaign_gravity_profile campaign_gravity_hierarchy
    campaign_triad_binding campaign_neutrino_sector
    campaign_cosmological_predictions campaign_dark_sector campaign_novel_predictions
    campaign_integer_sweep campaign_hydrogen_spectrum
    campaign_pe_fine_structure campaign_lorentz_measure
    particle_engine scale_bridge atom_engine atom_scale_bridge multiscale_bridge
    selective_damping stress_energy em_energy_conservation continuity
    poynting larmor ensemble correlations spectral tracker
    inflation dark_matter cosmological_constant consciousness sloop
    helium_scale1 fine_structure_scale1
    radiative_decay_scale1
    APPEND PROPERTY LABELS "slow")

# Benchmark tests: dedicated engine-vs-theory measurement suites
set_property(TEST
    benchmark_engine_theory benchmark_emergent_alpha
    benchmark_budget_equation benchmark_bh_thermo
    benchmark
    APPEND PROPERTY LABELS "benchmark;slow")

# ============================================================================
# Phase 7 (2026-04-27): "golden" + "physics" labels
# ----------------------------------------------------------------------------
# golden  — bit-exact regression vs frozen output; only the render-bridge
#           hash test belongs here today. Run alone with `ctest -L golden`
#           before any commit that touches engine physics.
# physics — energy-conservation, Coulomb law, locked-particle, absorbing-BC
#           tests that pin physical correctness independent of bit-hash.
# ============================================================================
if(TEST render_bridge_golden)
    set_property(TEST render_bridge_golden APPEND PROPERTY LABELS "golden;regression")
endif()

# Conservative physics-correctness subset: only tests that pin a
# physical-law assertion (not generic stencil/boilerplate tests). Each
# listed name MUST already exist as a registered test (the if-guard makes
# this resilient to future renames / disablings).
foreach(_phys_test
        audit_regression
        energy_conservation
        energy_conservation_tight
        gauss
        annihilation_conservation
        em_energy_conservation
        continuity
        poynting
        leapfrog_integrator_audit
        moore_laplacian_isotropy
        gamma_ftd_momentum
        langevin_equipartition
        determinism
        closed_negatives
        master_quadratic_identities)
    if(TEST ${_phys_test})
        set_property(TEST ${_phys_test} APPEND PROPERTY LABELS "physics")
    endif()
endforeach()

# ============================================================================
# RF-17: Domain labels (physics area)
# ============================================================================

# Electromagnetic domain
set_property(TEST
    maxwell em_fields em_energy_conservation
    continuity poynting larmor
    dipole_radiation thomson_scattering light
    telemetry_selftest
    lorentz gauge polarization
    flux_mediated variational_coulomb
    campaign_lorentz_measure
    APPEND PROPERTY LABELS "em")

# QCD / color domain
set_property(TEST
    asymptotic_freedom confinement_test triad_confinement
    campaign_triad_binding campaign_triad_energy
    APPEND PROPERTY LABELS "qcd")

# Gravity domain
set_property(TEST
    gravity_dynamics latency_field einstein_equations
    campaign_gravitational_wave campaign_gravity_profile campaign_gravity_hierarchy
    campaign_gravity_hierarchy inflation cosmological_constant
    dark_matter campaign_cosmological_predictions
    APPEND PROPERTY LABELS "gravity")

# Lattice / geometry domain
set_property(TEST
    lattice lattice_operators discrete_operators voxel_properties
    born_infeld bridge_dynamics
    campaign_dispersion
    APPEND PROPERTY LABELS "lattice")

# Particle / quantum domain
set_property(TEST
    genesis annihilation annihilation_conservation wave_collapse
    particle_lifetime particle_engine portable_field
    baryogenesis bell_aggregate born_rule_ensemble
    entanglement_basis
    campaign_born_rule campaign_quantum_correlations
    campaign_dark_sector campaign_novel_predictions
    APPEND PROPERTY LABELS "particle")

# Atomic / hydrogen domain
set_property(TEST
    atom_engine atom_scale_bridge atom_engine_forces
    helium_scale1 fine_structure_scale1
    radiative_decay_scale1
    campaign_hydrogen_spectrum campaign_hydrogen_binding
    campaign_h2_molecule
    APPEND PROPERTY LABELS "atom")

# Consciousness / measurement domain
set_property(TEST
    consciousness sloop measurement
    ensemble correlations spectral tracker
    campaign_wigner
    APPEND PROPERTY LABELS "consciousness")

# ============================================================================
# RF-17: Phase labels (engine tick-phase relevance)
# ============================================================================

set_property(TEST
    maxwell gauge continuity
    APPEND PROPERTY LABELS "phase_read")

set_property(TEST
    annihilation annihilation_conservation genesis
    campaign_spontaneous
    APPEND PROPERTY LABELS "phase_write")

set_property(TEST
    gravity_dynamics latency_field einstein_equations
    campaign_gravitational_wave
    APPEND PROPERTY LABELS "phase_forces")

set_property(TEST
    particle_lifetime portable_field wavepacket
    campaign_wave_dynamics
    APPEND PROPERTY LABELS "phase_movement")

endif() # NOT EMSCRIPTEN — end of CTest labels block

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

# ============================================================================
# Per-test timeout overrides (added 2026-05-03, post-WSL2 ctest sweep triage).
# ============================================================================
# The default ctest timeout (300s) is too tight for several legitimately slow
# benchmarks/campaigns that exercise large lattices or multi-thousand-tick
# evolution. Apply per-test bumps here so a routine ctest sweep doesn't kill
# them mid-run. Tests are still marked with their domain labels (eft, slow,
# campaign) so a runner can filter by load profile.
#
# Bumps grouped by category:
#   *  300s — moore_laplacian_isotropy (L=64 anisotropy sweep)
#   *  600s — eft_lorentz_recovery, eft_operator_spectrum (was 120s)
#   *  900s — maxwell (large-L curl/divergence battery)
#   * 1800s — campaign_dispersion, campaign_dark_sector, emergent_ic1_topology,
#             em_energy_conservation, poynting, spectral, campaign_shell_predictions
#
# Wrapped in if(TEST ...) so the file is robust to renamed/removed tests.
foreach(_t moore_laplacian_isotropy)
    if(TEST ${_t})
        set_tests_properties(${_t} PROPERTIES TIMEOUT 300)
    endif()
endforeach()
foreach(_t eft_lorentz_recovery eft_operator_spectrum)
    if(TEST ${_t})
        set_tests_properties(${_t} PROPERTIES TIMEOUT 600)
    endif()
endforeach()
foreach(_t maxwell)
    if(TEST ${_t})
        set_tests_properties(${_t} PROPERTIES TIMEOUT 900)
    endif()
endforeach()
foreach(_t campaign_dispersion campaign_dark_sector emergent_ic1_topology
            em_energy_conservation poynting spectral campaign_shell_predictions)
    if(TEST ${_t})
        set_tests_properties(${_t} PROPERTIES TIMEOUT 1800)
    endif()
endforeach()
