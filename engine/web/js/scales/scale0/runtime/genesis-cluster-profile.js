/**
 * Live toggle profile for the interactive genesis-cluster measurement panel.
 *
 * This is intentionally separate from scenario seeding. The panel resets the
 * native engine to `empty`, applies this isolated term profile, then injects a
 * user-selected amplitude. C++ remains the sole Scale-0 scenario owner.
 */

const DISABLED_TERMS = Object.freeze([
    'coupling', 'damping', 'genesis', 'evaporation', 'forces', 'gravity',
    'poisson_coulomb', 'movement', 'lorentz_force', 'selective_damping',
    'larmor_radiation', 'dual_substrate', 'color_forces',
    'strong_stress_energy', 'weak_transmutation', 'strong_force',
    'triad_binding', 'pair_production', 'exchange_force', 'latency_field',
    'exact_dual_gauss', 'matched_gauss_dynamics', 'emergent_forces',
    'langevin', 'symplectic_leapfrog', 'verlet_wave_integrator',
    'lorentz_period2_floquet', 'lorentz_bcc_time_floquet', 'su2_gauge',
    'su3_gauge', 'symmetric_movement_order', 'absorbing_boundary',
    'reflective_boundary', 'field_energy_gravity', 'cluster_inertia',
    'geometric_gravity', 'de_broglie_clock', 'db_clock_coulomb',
    'knot_tracking', 'confinement', 'strict_validation',
    'ew_background_sweep', 'flux_pump', 'flux_cell_port',
]);

export function configureGenesisClusterTerms(harness, temperature, gamma = 0.02) {
    for (const key of DISABLED_TERMS) harness.setToggle(key, false);
    harness.setToggle('wave_propagation', true);
    harness.setToggle('gauss_projection', true);
    harness.setToggle('genesis', true);
    harness.setToggle('langevin', true);
    harness.setToggle('dual_substrate', false);
    harness.setLangevinParams?.(temperature, gamma);
}
