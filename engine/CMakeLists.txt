cmake_minimum_required(VERSION 3.18)
# VERSION mirrors ftd::ENGINE_VERSION in include/ftd/constants.h (revision 6.1
# single-sourcing — keep in sync on release bumps).
project(ftd_engine VERSION 2.18.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell MSVC to interpret source files as UTF-8 so that hex-escaped multi-byte
# sequences in string literals (e.g. ontic_audit.cpp) are accepted correctly.
if(MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/utf-8>)
endif()

# ── Floating-point determinism policy (revision 0.2) ────────────────────────
# The golden-tick gates (ADR-0012) pin bit-exact hashes across MSVC (Windows)
# and gcc (WSL2 campaign builds). Measured 2026-07-02: both toolchains
# reproduce GOLDEN_HASH 0xb604d81a3d79366e at their CURRENT defaults. These
# flags make that agreement explicit and immune to toolchain-default drift:
#   MSVC  /fp:precise      — the current default, pinned against future
#                            /fp:fast-by-default changes.
#   GCC   -ffp-contract=off — forbids FMA contraction (gcc at -O2/-O3 defaults
#                            to -ffp-contract=fast; x86-64 without -march
#                            AVX2/FMA doesn't emit FMA today, which is WHY the
#                            hashes agree — this pin keeps it true if -march or
#                            the default ISA ever changes).
# See engine/docs/DESIGN_RNG_PORTABILITY.md (per-platform hash policy).
if(NOT EMSCRIPTEN)
    if(MSVC)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/fp:precise>)
    else()
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-ffp-contract=off>)
    endif()
endif()

# gcc lowers every std::complex<double> multiply to the checked __muldc3
# libcall (C99 Annex G NaN/Inf recovery); MSVC inlines the naive arithmetic.
# In the SU(2)/SU(3) staple sweeps (relax_su2/su3_links_cpu) that libcall is
# the dominant cost: test_gauge_links measured 1.3 s (MSVC) vs ~4.5 min
# (WSL2 gcc 11.4 -O3), with 100 __muldc3 relocation sites in this one TU.
# -fcx-limited-range emits the naive ac−bd / ad+bc inline. For finite inputs
# this is BIT-IDENTICAL to __muldc3 — the libcall computes those exact
# products first and only branches into its recovery path when the result is
# NaN — and the TU contains no complex division (zero __divdc3 relocs), so
# the flag's non-value-identical division rewrite (naive instead of Smith's
# algorithm) never fires. FMA contraction is already pinned off above.
# GAUGE_GOLDEN_HASH 0xa4dec20d1dd94ec8 re-verified on WSL2-gcc after this
# flag landed; MSVC codegen is untouched (ADR-0012 record).
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_source_files_properties(src/transmutation_phases.cpp PROPERTIES
        COMPILE_OPTIONS "-fcx-limited-range")
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

# Experimental-module quarantine (revision 3.7, ADR-0016): DagEngine, the
# cognition sidecar, and the discrete-universe CUDA prototype are kept —
# never deleted (provenance convention) — but their build membership is an
# explicit boundary. Declared here (moved 2026-08-16 from its prior location
# ~250 lines down, immediately before the ftd_core target that also gates on
# it) because CMake processes top-to-bottom: this option() must exist before
# any if(FTD_BUILD_EXPERIMENTAL) check, including the ftd_cognition library
# just below. It didn't before, so on a FRESH configure (no cached value from
# a prior run — exactly what CI always does) that first if() saw an
# undefined, falsy variable and silently skipped ftd_cognition, while
# test_cognitive_lattice/benchmark_cognitive_lattice (gated further down,
# after the old declaration point) correctly saw it ON and tried to link
# against a library that was never built — LNK1181 "cannot open input file
# ftd_cognition.lib". Invisible on any machine with a previously-configured
# build directory, since CMake reuses a variable already in the cache from
# the very start of the next run, which is exactly why this went unnoticed
# locally but failed on every CI run's from-scratch checkout.
option(FTD_BUILD_EXPERIMENTAL "Build experimental/quarantined modules (DagEngine, cognition sidecar, discrete-universe prototype)" ON)

# ftd_cognition — sidecar ternary memory/retrieval substrate for LLM/agent
# workflows. Deliberately independent of ftd_core / RenderBridge / Voxel.
if(FTD_BUILD_EXPERIMENTAL)  # ADR-0016 quarantine
add_library(ftd_cognition STATIC
    src/cognition/cognitive_lattice.cpp
)
target_include_directories(ftd_cognition PUBLIC ${CMAKE_SOURCE_DIR}/include)
endif()  # FTD_BUILD_EXPERIMENTAL (ftd_cognition)

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
    src/eft/history_event_journal.cpp
    src/eft/conserved_charge_basis.cpp
    src/eft/matched_gauss_transport.cpp
    src/eft/subcell_polarity_shape.cpp
    src/eft/face_current_segment.cpp
    src/eft/spacetime_worldline_coupling.cpp
    src/eft/two_slab_variational_force.cpp
    src/eft/open_worldline_hop_selector.cpp
    src/eft/discrete_legendre_worldline.cpp
    src/eft/knot_legendre_branch.cpp
    src/eft/centered_knot_trace.cpp
    src/eft/centered_trace_work.cpp
    src/eft/cusp_dressing_integrability.cpp
    src/eft/dressing_fiber_ledger.cpp
    src/eft/centered_fiber_knot_transaction.cpp
    src/eft/axial_face_hop_reciprocity.cpp
    src/eft/subcell_representation_quotient.cpp
    src/eft/finite_memory_reversible_lift.cpp
    src/eft/canonical_subcell_section.cpp
    src/eft/multibody_shape_observability.cpp
    src/eft/worldline_current_kernel.cpp
    src/eft/momentum_selected_worldline_matching.cpp
    src/eft/ternary_collision_vertex.cpp
    src/eft/boundary_collision_resolution.cpp
    src/eft/production_same_sign_bounce.cpp
    src/eft/boundary_chart_capacity.cpp
    src/eft/constituent_relative_collision.cpp
    src/eft/constituent_stress_moment.cpp
    src/eft/momentum_face_balance.cpp
    src/eft/hard_contact_corner_action.cpp
    src/eft/native_contact_active_set.cpp
    src/eft/contact_quotient_horizon.cpp
    src/eft/overshoot_preserving_contact_rebase.cpp
    src/eft/contact_quotient_coupling_scope.cpp
    src/eft/matched_contact_energy_obstruction.cpp
    src/eft/axial_contact_longitudinal_work.cpp
    src/eft/symmetric_diagonal_coupled_endpoint.cpp
    src/eft/diagonal_endpoint_action_domain.cpp
    src/eft/multicell_worldline_variation.cpp
    src/eft/single_slab_connection_compatibility.cpp
    src/eft/staggered_current_split_compatibility.cpp
    src/eft/implicit_atomic_face_action.cpp
    src/eft/implicit_atomic_endpoint_solve.cpp
    src/eft/edge_plane_one_sided_variation.cpp
    src/eft/local_polarity_regularity.cpp
    src/eft/quadratic_coat_face_current.cpp
    src/eft/quadratic_coat_spacetime_action.cpp
    src/eft/fixed_step_energy_scope.cpp
    src/eft/coupled_quartic_clock_field.cpp
    src/eft/contextual_actualization.cpp
    src/eft/native_event_characteristics.cpp
    src/eft/relative_action_transducer.cpp
    src/eft/phase_referenced_action_rail.cpp
    src/eft/catalytic_phase_reference.cpp
    src/eft/clock_gated_hamiltonian_exchange.cpp
    src/eft/ternary_eligibility_clutch.cpp
    src/eft/signal_acknowledged_two_stroke_reset.cpp
    src/eft/reversible_ternary_signal_uncomputation.cpp
    src/eft/oriented_ternary_quarter_turn.cpp
    src/eft/hamiltonian_ternary_quarter_turn_actuator.cpp
    src/eft/alternating_oriented_ternary_parity_rail.cpp
    src/eft/local_canonical_hamiltonian_parity_rail.cpp
    src/eft/flux_wave_velocity_markov_carrier.cpp
    src/eft/gauss_record_canonical_reduction.cpp
    src/eft/reversible_checkerboard_gauss_preparation.cpp
    src/eft/finite_port_gauss_battery.cpp
    src/eft/canonical_source_centered_gauss_gate.cpp
    src/eft/autonomous_phase_parity_source_reaction.cpp
    src/eft/cubic_reaction_vector_source_transport.cpp
    src/eft/collective_reaction_triplet_inertia.cpp
    src/eft/dressed_boost_momentum_map.cpp
    src/eft/bloch_quasimomentum_lift.cpp
    src/eft/reciprocal_carry_reservoir.cpp
    src/eft/quartic_relative_carry_gearbox.cpp
    src/eft/common_relative_connection_gearbox.cpp
    src/eft/self_pair_connection_critical_gearbox.cpp
    src/eft/oriented_even_self_pair_rectifier.cpp
    src/eft/native_ternary_dipole_phase_wedge_memory.cpp
    src/eft/native_ternary_plaquette_quarter_turn.cpp
    src/eft/reciprocal_record_port.cpp
    src/eft/matched_midpoint_poynting.cpp
    src/eft/quadratic_coat_matter_work.cpp
    src/eft/quadratic_coat_neutral_pair_work.cpp
    src/eft/accelerated_worldline_energy.cpp
    src/eft/accelerated_coat_spacetime_current.cpp
    src/eft/endpoint_schedule_underdetermination.cpp
    src/eft/quadratic_coat_orbit_gather.cpp
    src/eft/quadratic_coat_discrete_gradient_transaction.cpp
    src/eft/quadratic_coat_composite_peierls.cpp
    src/eft/continuous_translation_locality.cpp
    src/eft/extended_source_peierls_scaling.cpp
    src/eft/integer_bloch_transport.cpp
    src/eft/free_flux_localization.cpp
    src/eft/native_moving_source_pole.cpp
    src/eft/external_drive_radiation.cpp
    src/eft/native_hop_dressing_obstruction.cpp
    src/eft/hop_source_multipole_hierarchy.cpp
    src/eft/full_surface_source_obstruction.cpp
    src/eft/native_gauss_monopole_dichotomy.cpp
    src/eft/orientation_gauss_independence.cpp
    src/eft/genesis_action_obstruction.cpp
    src/eft/genesis_reservoir_dilation.cpp
    src/eft/genesis_natural_extension.cpp
    src/eft/genesis_environment_feedback.cpp
    src/eft/genesis_minimal_bath.cpp
    src/eft/genesis_cubic_canonical_form.cpp
    src/eft/native_field_discrete_action.cpp
    src/eft/native_hodge_reciprocity.cpp
    src/eft/native_hodge_energy_continuity.cpp
    src/eft/minimal_moore_compatibility_coat.cpp
    src/eft/common_moore_worldline_action.cpp
    src/eft/site_ontic_atomic_reciprocal_hop.cpp
    src/eft/constituent_complete_charged_trimer.cpp
    src/eft/closed_neutral_trimer_pair.cpp
    src/eft/finite_rigid_moore_carrier_obstruction.cpp
    src/eft/symmetric_chord_moore_action.cpp
    src/eft/passive_dressing_depinning_obstruction.cpp
    src/eft/native_active_mode_backreaction.cpp
    src/eft/noncompact_face_cohomology.cpp
    src/eft/configuration_space_carrier.cpp
    src/eft/native_motion_reaction_front.cpp
    src/eft/endogenous_reaction_carrier_bound.cpp
    src/eft/ignition_cut_support_ablation.cpp
    src/eft/collective_source_history_bound.cpp
    src/eft/removal_time_pulse_bound.cpp
    src/eft/removal_time_orbit_coherence.cpp
    src/eft/derived_interaction_graph.cpp
    src/eft/spline_poynting_momentum.cpp
    src/eft/connected_moore_block_action.cpp
    src/eft/state_only_matter_field_observer.cpp
    src/eft/transported_chart_morphology.cpp
    src/eft/paired_field_response.cpp
    # Arc 2 momentum stress ledger: unit-bond T^(i)/S^(i) construction and the
    # cumulative F/W/Q accumulators.  Observer-only; no production consumer.
    src/eft/momentum_transport_current.cpp
    src/eft/support_invariant_matter_predicate.cpp
    src/eft/matched_regional_energy_transport.cpp
    src/eft/batched_regional_energy_profile.cpp
    src/eft/component_aware_radial_field_profile.cpp
    src/eft/eight_source_orbit_coherence.cpp
    src/eft/nine_source_orbit_coherence.cpp
    src/eft/ten_source_orbit_coherence.cpp
    src/eft/ten_source_shared_m_coherence.cpp
    src/eft/ten_source_pair_distance_capacity.cpp
    src/eft/ten_source_distance_distribution_lp.cpp
    src/eft/ten_source_temporal_product_capacity.cpp
    src/eft/coupled_matched_face_transaction.cpp
    src/eft/connected_reservoir_decomposition.cpp
    src/eft/localized_basin_observer.cpp
    src/eft/matched_face_current_spectrum.cpp
    src/eft/matched_symmetry_ray_spectrum.cpp
    src/eft/ternary_block_bipole_peierls.cpp
    src/eft/pole_matching.cpp
)
target_include_directories(ftd_eft PUBLIC ${CMAKE_SOURCE_DIR}/include)
if(OpenMP_CXX_FOUND)
    # Observer-only state reconstruction scans O(L^3) centered samples.  The
    # pragma lives in ftd_eft, so linking OpenMP only through ftd_core leaves
    # CUDA-only research executables silently serial.  No dynamics uses this
    # pragma; this enables the declared deterministic x-slab reduction only.
    target_link_libraries(ftd_eft PUBLIC OpenMP::OpenMP_CXX)
endif()

# Core library
add_library(ftd_core
    src/render_bridge.cpp
    src/native_telemetry_scheduler.cpp
    # src/dag_engine.cpp is quarantined — added below under FTD_BUILD_EXPERIMENTAL (ADR-0016)
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
    src/vtk_export.cpp     # ParaView/VTK XML research export bundle
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
    src/visual_field_sample.cpp      # compact CPU/native field-overlay sampler
    src/strong_stress_energy.cpp     # FTD-0406 selected strong Hamiltonian / local T00-stress contract
    src/injection.cpp                # R5: inject_flux/particle/wavepacket + aggregate profile
    src/scale_context.cpp            # Scale-context readout admissibility gate (read-only; α-blind)
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

# Precompiled headers (revision 3.6): every ftd_core TU re-parses
# render_bridge.h (which pulls the ontic chain + stdlib); 300+ test targets
# re-parse it again. PCH is compile-time only — zero codegen/behavior change
# (golden gates verified). FTD_ENABLE_PCH=OFF is the instant opt-out; the
# Emscripten guard keeps the three WASM build trees untouched.
option(FTD_ENABLE_PCH "Use precompiled headers for ftd_core (compile-speed only)" ON)

# FTD_BUILD_EXPERIMENTAL is declared earlier now (~line 140, before the
# ftd_cognition target that needs it defined first on a fresh configure —
# see the comment there, 2026-08-16). Default ON = zero change to any
# existing build; OFF produces a lean core with the experimental targets
# absent. A source-lint (cmake/FtdSourceLint.cmake) guards that no production
# TU includes the quarantined headers.
if(FTD_ENABLE_PCH AND NOT EMSCRIPTEN)
    target_precompile_headers(ftd_core PRIVATE
        include/ftd/render_bridge.h
        <vector> <cmath> <cstdint> <algorithm> <memory> <string>)
endif()

# ADR-0016: DagEngine is compiled into ftd_core only when the experimental
# boundary is open (its WASM binding was removed in the 2026 consolidation
# sweep; the only consumer is test_dag_engine, gated below).
if(FTD_BUILD_EXPERIMENTAL)
    target_sources(ftd_core PRIVATE src/dag_engine.cpp)
endif()

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
    src/ws_server_commands.cpp
    src/ws_server_binary.cpp
    src/ws_server_telemetry.cpp
    src/ws_server_runtime.cpp
    src/ws_protocol.cpp   # WS2: framing + recv/send + JSON helpers split from ws_server.cpp
)
target_link_libraries(ws_server ftd_core)
if(WIN32)
    target_link_libraries(ws_server ws2_32)
endif()
endif()

# In-process Win32/D3D12 desktop app (engine/native): RmlUi UI composited over
# the live D3D12 lattice/particle scene. Isolated from engine/web. Windows-only.
# The prior ImGui prototype (native_desktop) was retired 2026-08-21; engine/native
# is now the sole native app. See native/docs/PLAN_NATIVE_REBUILD.md.
if(NOT EMSCRIPTEN)
    option(FTD_BUILD_NATIVE
           "Build the native desktop app engine/native (does not affect web/WASM)" ON)
    if(FTD_BUILD_NATIVE)
        add_subdirectory(native)
    endif()
endif()

# Native test/campaign targets, CTest registration, source ownership, and
# scheduling form one build-system unit. Emscripten builds do not compile test
# executables, so none of these modules may inspect or register the native test
# tree there.
if(NOT EMSCRIPTEN)
    include(${CMAKE_SOURCE_DIR}/cmake/FtdTestTargets.cmake)
    include(${CMAKE_SOURCE_DIR}/cmake/FtdCTestRegistration.cmake)
    include(${CMAKE_SOURCE_DIR}/cmake/FtdTestSourceOwnership.cmake)
    include(${CMAKE_SOURCE_DIR}/cmake/FtdCTestScheduling.cmake)
endif()

# ============================================================================
# WASM build (Emscripten)
# ============================================================================

if(EMSCRIPTEN)
    add_subdirectory(wasm)
endif()
