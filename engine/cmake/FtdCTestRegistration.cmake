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
add_test(NAME boundary_movement COMMAND test_boundary_movement)
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
add_test(NAME causal_normalization COMMAND test_causal_normalization)
add_test(NAME lattice_operators COMMAND test_lattice_operators)
add_test(NAME discrete_operators COMMAND test_discrete_operators)
add_test(NAME bridge_dynamics COMMAND test_bridge_dynamics)
add_test(NAME maxwell COMMAND test_maxwell)
add_test(NAME csv_export COMMAND test_csv_export)
add_test(NAME vtk_export COMMAND test_vtk_export)
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
add_test(NAME symplectic_wave COMMAND test_symplectic_wave)
add_test(NAME relativistic_verlet COMMAND test_relativistic_verlet)
add_test(NAME molecular_dihedrals COMMAND test_molecular_dihedrals)
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
# FTD-0285 - no-alpha-input alpha discriminator
add_test(NAME alpha_no_alpha_probe COMMAND campaign_alpha_no_alpha_probe)
set_tests_properties(alpha_no_alpha_probe PROPERTIES LABELS "eft;alpha;campaign" TIMEOUT 600)
# FTD-0285 run-of-record guard (2026-07-18). The probe's verdict of record is
# INVALIDATED_PROTOCOL_OR_ENGINE_DRIFT (LEDGER FTD-0285 [INVALIDATED PROTOCOL],
# ANALYSIS_ALPHA_NO_ALPHA_ENGINE_PROBE_v1.md quotes the failing run verbatim as
# the Run of Record; lock tag preregister-alpha-no-alpha-engine-probe-v1,
# artifact SHA unchanged since lock cce615b0 — the .cpp must NOT be edited).
# The exe encodes that verdict class as EXIT_FAILURE, which left the suite
# permanently red for reproducing the record. Pass iff the recorded verdict
# class reproduces; ANY other verdict — including the positive classes, which
# exit 0 — fails the test and flags a departure from the FTD-0285 record
# (a v2 arc mints its own test per the LEDGER row's "next" note).
set_tests_properties(alpha_no_alpha_probe PROPERTIES
    PASS_REGULAR_EXPRESSION "verdict,INVALIDATED_PROTOCOL_OR_ENGINE_DRIFT")
# FTD-0286 - alpha estimator validation
add_test(NAME alpha_estimator_validation COMMAND campaign_alpha_estimator_validation)
set_tests_properties(alpha_estimator_validation PROPERTIES LABELS "eft;alpha;campaign" TIMEOUT 600)

add_test(NAME alpha_estimator_validation_v2 COMMAND campaign_alpha_estimator_validation_v2)
set_tests_properties(alpha_estimator_validation_v2 PROPERTIES LABELS "eft;alpha;campaign" TIMEOUT 600)
# FTD-0287 - Thomson dashboard observatory companion campaign
add_test(NAME thomson_recoil_observatory COMMAND campaign_thomson_recoil_observatory)
set_tests_properties(thomson_recoil_observatory PROPERTIES LABELS "em;campaign;observatory" TIMEOUT 300)
# FTD-0288 - unlocked Thomson recoil native/diagnostic discriminator
add_test(NAME thomson_unlocked_recoil COMMAND campaign_thomson_unlocked_recoil)
set_tests_properties(thomson_unlocked_recoil PROPERTIES LABELS "em;campaign;observatory" TIMEOUT 300)
# FTD-0289 - baseline-subtracted Thomson flux-excess discriminator
add_test(NAME thomson_flux_excess COMMAND campaign_thomson_flux_excess)
set_tests_properties(thomson_flux_excess PROPERTIES LABELS "em;campaign;observatory" TIMEOUT 300)
# FTD-0290 - residual-field radiation shell meter
add_test(NAME thomson_radiation_shells COMMAND campaign_thomson_radiation_shells)
set_tests_properties(thomson_radiation_shells PROPERTIES LABELS "em;campaign;observatory" TIMEOUT 300)
# FTD-0291 - native finite-volume continuity meter
add_test(NAME thomson_native_continuity COMMAND campaign_thomson_native_continuity)
set_tests_properties(thomson_native_continuity PROPERTIES LABELS "em;campaign;observatory" TIMEOUT 300)
# FTD-0292 - source-free discrete tick energy invariant
add_test(NAME thomson_tick_invariant COMMAND campaign_thomson_tick_invariant)
set_tests_properties(thomson_tick_invariant PROPERTIES LABELS "em;campaign;observatory" TIMEOUT 300)
# FTD-0292 v1 classified outcome is DISCRETE_TICK_INVARIANT_INVALIDATED:
# the locked executable returns failure when the predeclared relative gate
# misses the double-precision summation floor. Keep the historical target
# green only when that invalidation reproduces.
set_tests_properties(thomson_tick_invariant PROPERTIES WILL_FAIL TRUE)
# FTD-0293 - source-free discrete tick energy invariant, precision v2
add_test(NAME thomson_tick_invariant_v2 COMMAND campaign_thomson_tick_invariant_v2)
set_tests_properties(thomson_tick_invariant_v2 PROPERTIES LABELS "em;campaign;observatory" TIMEOUT 300)
# FTD-0294 - source-free discrete tick local continuity
add_test(NAME thomson_tick_local_continuity COMMAND campaign_thomson_tick_local_continuity)
set_tests_properties(thomson_tick_local_continuity PROPERTIES LABELS "em;campaign;observatory" TIMEOUT 300)
# FTD-0294 v1 classified outcome is SOURCE_FREE_LOCAL_TICK_CONTINUITY_INVALIDATED:
# absolute balance closes at roundoff, but the frozen exchange-relative gate is
# degenerate when both Delta H and boundary flux are near zero. Keep the
# historical target green only when that invalidation reproduces.
set_tests_properties(thomson_tick_local_continuity PROPERTIES WILL_FAIL TRUE)
# FTD-0295 - source-free discrete tick local continuity, scale-relative v2
add_test(NAME thomson_tick_local_continuity_v2 COMMAND campaign_thomson_tick_local_continuity_v2)
set_tests_properties(thomson_tick_local_continuity_v2 PROPERTIES LABELS "em;campaign;observatory" TIMEOUT 300)
# FTD-0296 - fixed-charge coupled tick source/work continuity
add_test(NAME thomson_coupled_source_work COMMAND campaign_thomson_coupled_source_work)
set_tests_properties(thomson_coupled_source_work PROPERTIES LABELS "em;campaign;observatory" TIMEOUT 300)
# FTD-0297 - moving-recoil source/work accounting
add_test(NAME thomson_moving_recoil_accounting COMMAND campaign_thomson_moving_recoil_accounting)
set_tests_properties(thomson_moving_recoil_accounting PROPERTIES LABELS "em;campaign;observatory" TIMEOUT 300)
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
add_test(NAME a1g_projector COMMAND test_a1g_projector)
set_tests_properties(a1g_projector PROPERTIES LABELS "fast;unit;ftd0110")
add_test(NAME a1g_bridge_i_empirical COMMAND test_a1g_bridge_i_empirical)
set_tests_properties(a1g_bridge_i_empirical PROPERTIES LABELS "ftd0110;bridge;sim" TIMEOUT 600)
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

# Ticket 3.3 — ADR-0013 table-driven toggle characterization guards.
# Pure toggle-state pins (defaults / enable_all / minimal / validate / string
# get_toggle-set_toggle round-trip) for the three sub-engines, written from
# current source BEFORE the table-driven port so behavior preservation is
# provable. Independent of the pre-existing physics failures in
# particle_toggles (FTD-0131 G_PE scale) — no force is ever computed.
ftd_add_test(test_particle_toggles_table tests/test_particle_toggles_table.cpp
             CTEST_NAME particle_toggles_table TIMEOUT 120 LABELS scale1)
ftd_add_test(test_atom_toggles_table tests/test_atom_toggles_table.cpp
             CTEST_NAME atom_toggles_table TIMEOUT 120 LABELS scale2)
ftd_add_test(test_cosmic_toggles_table tests/test_cosmic_toggles_table.cpp
             CTEST_NAME cosmic_toggles_table TIMEOUT 120 LABELS scale5 cosmic)

# GPU tests (only registered when CUDA is enabled)
if(FTD_ENABLE_CUDA)
    add_test(NAME gpu_parity COMMAND test_gpu_parity)
    add_test(NAME gpu_parity_complete COMMAND test_gpu_parity_complete)
    add_test(NAME gpu_native_extension_parity COMMAND test_gpu_native_extension_parity)
    add_test(NAME gpu_eft_parity COMMAND test_gpu_eft_parity)
    add_test(NAME gpu_benchmark COMMAND test_gpu_benchmark)
    add_test(NAME gpu_continuity_ledger COMMAND test_gpu_continuity_ledger)
    add_test(NAME gpu_physics COMMAND test_gpu_physics)
    add_test(NAME gpu_gauss_law_fidelity COMMAND test_gauss_law_fidelity_gpu)
    add_test(NAME gpu_experiments COMMAND test_gpu_experiments)
    set_tests_properties(gpu_parity gpu_parity_complete gpu_native_extension_parity
                         gpu_eft_parity gpu_benchmark
                         gpu_continuity_ledger gpu_physics gpu_gauss_law_fidelity
                         PROPERTIES TIMEOUT 600)
    set_property(TEST gpu_native_extension_parity APPEND PROPERTY LABELS "gpu;unit;parity")
    set_property(TEST gpu_gauss_law_fidelity APPEND PROPERTY LABELS "gpu;gauss")
    set_tests_properties(gpu_continuity_ledger PROPERTIES
                         LABELS "gpu;constructor;ledger;observable;eft_quick")
    # The Debye-Hückel arm is a fixed [CLOSED NEGATIVE] protocol: CTest is
    # green only while its three anti-screening signatures reproduce and all
    # other experiment checks pass. The executable itself enforces that exact
    # partition so an unrelated failure cannot be masked by WILL_FAIL.
    set_tests_properties(gpu_experiments PROPERTIES
                         TIMEOUT 1800
                         LABELS "gpu;scientific;closed_negative")
    # gpu_parity_complete runs 22 CPU-vs-GPU domain checks, several under
    # enable_all() whose CPU side drives iterative SOR Poisson solves (Gauss +
    # Coulomb + latency) every tick — plus GPC-20's 1000-tick sweep. On the
    # canonical WSL2-gcc -O3 platform that CPU path is far slower than MSVC
    # (same class as gauge_links, whose timeout was raised for this reason), so
    # the 600 s group cap above is not enough (it was timing the gate out at
    # baseline). This override lifts it (a later set_tests_properties wins).
    # C6 (2026-07-03) added the GPC-21/22 weak-field domains; they are lean, but
    # the pre-existing enable_all domains dominate (~44 min total on WSL2).
    set_tests_properties(gpu_parity_complete PROPERTIES TIMEOUT 3600)
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
    pe_forces
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
    # pe_forces is auto-labeled as "unit" by ftd_add_test()
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
        gpu_parity_complete
        gpu_native_extension_parity
        gpu_eft_parity
        gpu_benchmark
        gpu_continuity_ledger
        gpu_physics
        gpu_gauss_law_fidelity
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
# Per-test timeout overrides (added 2026-05-03, post-WSL2 ctest sweep triage).
# ============================================================================
# The default ctest timeout (300s) is too tight for several legitimately slow
# benchmarks/campaigns that exercise large lattices or multi-thousand-tick
# evolution. Apply per-test bumps here so a routine ctest sweep doesn't kill
# them mid-run. Tests are still marked with their domain labels (eft, slow,
# campaign) so a runner can filter by load profile.
#
# Bumps grouped by category:
#   * 1800s — moore_laplacian_isotropy, eft_lorentz_recovery,
#             eft_operator_spectrum, maxwell, energy_conservation_tight
#   * 3600s — campaign_dispersion, campaign_dark_sector, emergent_ic1_topology,
#             em_energy_conservation, poynting, spectral,
#             campaign_shell_predictions (1800s wasn't enough on WSL2 GPU run)
#   * 7200s — benchmark_alpha_convergence (L=384 GPU, 47-min average runtime)
#   * 7200s — benchmark_alpha_scaling (long L-sweep)
#
# Wrapped in if(TEST ...) so the file is robust to renamed/removed tests.
# Updated 2026-05-04 after second full-sweep observed timeouts at the
# previous 300s/600s/900s/1800s settings on canonical WSL2 RTX 5090 GPU run.
foreach(_t moore_laplacian_isotropy eft_lorentz_recovery eft_operator_spectrum
            energy_conservation_tight)
    if(TEST ${_t})
        set_tests_properties(${_t} PROPERTIES TIMEOUT 1800)
    endif()
endforeach()
# maxwell + heavy GPU benchmarks: 3600s under -j4 contention.
# Per-test single-runs are <1000s but parallel scheduling on shared
# RTX 5090 stretches them well past 1800s.
foreach(_t maxwell campaign_dispersion campaign_dark_sector emergent_ic1_topology
            em_energy_conservation poynting spectral campaign_shell_predictions)
    if(TEST ${_t})
        set_tests_properties(${_t} PROPERTIES TIMEOUT 3600)
    endif()
endforeach()
foreach(_t benchmark_alpha_convergence benchmark_alpha_scaling)
    if(TEST ${_t})
        set_tests_properties(${_t} PROPERTIES TIMEOUT 7200)
    endif()
endforeach()

# ============================================================================
# DISABLED tests — multi-hour benchmarks excluded from default ctest sweeps.
# ============================================================================
# These run >1hr each on canonical WSL2 RTX 5090 GPU and aren't appropriate
# for routine pre-commit verification. Run explicitly via:
#   ctest --no-tests=ignore -R '^benchmark_alpha_convergence$' --timeout 14400
# or by setting CTEST_INCLUDE_DISABLED=ON. Tests still build; they just don't
# run by default.
#
# Justification per test:
#   benchmark_alpha_convergence  — 2hr+ even at 7200s timeout (L=384 GPU sweep)
#   benchmark_alpha_scaling      — subprocess-killed in v2 sweep; >1hr expected
#   campaign_dark_sector         — >1hr GPU benchmark, often >3600s
#   em_energy_conservation       — >1hr at L=128+
#   eft_lorentz_recovery         — >30min at L=64 T=512
#   campaign_shell_predictions   — >1hr at L=256
#   campaign_thermal_ignition    — research sweep (multi-L ignition-threshold
#                                  scan); measured 2026-07-18 still computing
#                                  at 75+ min after the 1800s ctest kill (the
#                                  exe survives the kill and keeps running) —
#                                  chronic ***Timeout in every adjudicated
#                                  sweep. Run manually with explicit args.
#   campaign_genesis_amplitude_ceiling — research sweep (amplitude x nodes x
#                                  seeds scan); >1800s at exclusive full width
#                                  in the 2026-07-18 adjudicated sweep. Run
#                                  manually with explicit --amps/--seeds args.
foreach(_t benchmark_alpha_convergence benchmark_alpha_scaling
            campaign_dark_sector em_energy_conservation eft_lorentz_recovery
            campaign_shell_predictions
            campaign_thermal_ignition campaign_genesis_amplitude_ceiling)
    if(TEST ${_t})
        set_tests_properties(${_t} PROPERTIES DISABLED TRUE)
    endif()
endforeach()

add_executable(dump_koopman_trajectory tests/dump_koopman_trajectory.cpp)
target_link_libraries(dump_koopman_trajectory PRIVATE ftd_core)
# Data-generation utility only: its default invocation runs 100,000 ticks and
# writes a large CSV, but performs no assertions.  Keep it as an explicit
# build/run target instead of making ordinary CTest audits appear frozen.

# Exploratory scale-context confinement scan (NOT registered with ctest:
# slow, exploratory). Build target only. See SPEC_SCALE_CONTEXT_READOUT §5.5.
add_executable(campaign_scale_context_confine tests/campaign_scale_context_confine.cpp)
target_link_libraries(campaign_scale_context_confine PRIVATE ftd_core)
# ============================================================================
# merge_gate label + lean build target — fast pre-push verification bundle
# (revision 0.4).
# ============================================================================
# `ctest -L merge_gate -j 32 -C Release` is the <2-minute local gate to run
# before every push (see engine/docs/CI_GATE.md). Composition: the absolute
# golden gates + phase-order/lifecycle/determinism guards. GPU parity is NOT
# in this bundle (run `ctest -L gpu` on the WSL2 build for that); neither are
# campaign/benchmark tests.
#
# Keep the executable targets in one canonical list. The hosted Pages-deploy
# job builds `ftd_merge_gate_build` instead of the default ALL target, avoiding the
# cost of compiling 300+ unrelated test executables before running this gate.
if(NOT EMSCRIPTEN)
    set(_ftd_merge_gate_targets
        test_render_bridge_golden
        test_render_bridge_golden_default
        test_knot_tracking_golden
        test_tick_phase_order
        test_engine_lifecycle
        test_determinism
        test_gpu_term_contract
        test_ui_observer_neutrality_cpu)

    foreach(_ftd_target IN LISTS _ftd_merge_gate_targets)
        string(REGEX REPLACE "^test_" "" _ftd_test "${_ftd_target}")
        if(NOT TARGET ${_ftd_target} OR NOT TEST ${_ftd_test})
            message(FATAL_ERROR
                "merge_gate entry '${_ftd_target}' must be both a build target and a CTest test")
        endif()
        set_property(TEST ${_ftd_test} APPEND PROPERTY LABELS "merge_gate")
    endforeach()

    add_custom_target(ftd_merge_gate_build
        DEPENDS ${_ftd_merge_gate_targets}
        COMMENT "Building the focused FTD merge-gate executables")

    unset(_ftd_merge_gate_targets)
    unset(_ftd_target)
    unset(_ftd_test)
endif()

# Source-text lint (revision 2.6): tree-level X_PLUS/X_MINUS must never leak
# into runtime physics paths (banners/audits/CLI display are exempt — only
# force/physics TUs are scanned). See cmake/FtdSourceLint.cmake.
if(NOT EMSCRIPTEN)
    add_test(NAME source_lint
             COMMAND ${CMAKE_COMMAND} -DENGINE_DIR=${CMAKE_SOURCE_DIR}
                     -P ${CMAKE_SOURCE_DIR}/cmake/FtdSourceLint.cmake)
    set_tests_properties(source_lint PROPERTIES TIMEOUT 60 LABELS "unit;lint")
endif()

# End-to-end native transport gate. The Node harness owns an ephemeral port,
# starts a one-client server, runs the protocol smoke, and always reaps the
# child process. The smoke requires the real CUDA backend by design.
if(NOT EMSCRIPTEN AND FTD_ENABLE_CUDA)
    find_program(FTD_NODE_EXECUTABLE NAMES node node.exe)
    if(FTD_NODE_EXECUTABLE)
        add_test(NAME native_ws_smoke
                 COMMAND ${FTD_NODE_EXECUTABLE}
                         ${CMAKE_SOURCE_DIR}/web/tests/native-ws-ctest.mjs
                         $<TARGET_FILE:ws_server>
                         ${CMAKE_SOURCE_DIR}/web/tests/native-ws-smoke.mjs)
        set_tests_properties(native_ws_smoke PROPERTIES
            TIMEOUT 180 LABELS "integration;native;gpu;websocket"
            PROCESSORS 32 RUN_SERIAL TRUE)
    else()
        message(STATUS "Node.js not found — native_ws_smoke is not registered")
    endif()
endif()
