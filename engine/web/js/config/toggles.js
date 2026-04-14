/**
 * Toggle Configuration — Single source of truth for all scale toggles.
 *
 * Each entry: [toggleKey, defaultValue, domElementId]
 *   - toggleKey: String key used by bridge.setToggle() / bridge.getToggle()
 *   - defaultValue: Boolean initial state when scenario loads
 *   - domElementId: HTML checkbox/button ID for UI sync
 *
 * Extracted from app.js to eliminate scattered toggle definitions.
 * Human coders: add new toggles here, not in app.js or wasm-bridge.js.
 */

// Scale 0 (Lattice) — physics term toggles matching TermToggles in term_toggles.h
export const SCALE0_TOGGLES = [
    ['wave_propagation', true,  't-wave'],
    ['coupling',         true,  't-coupling'],
    ['damping',          true,  't-damping'],
    ['genesis',          true,  't-genesis'],
    ['gauss_projection', true,  't-gauss'],
    ['forces',           true,  't-forces'],
    ['gravity',          false, 't-gravity'],
    ['movement',         true,  't-movement'],
    ['poisson_coulomb',  true,  't-poisson'],
    ['lorentz_force',    false, 't-lorentz'],
    ['selective_damping',false, 't-selective'],
    ['larmor_radiation', false, 't-larmor'],
    ['dual_substrate',   false, 't-dual'],
    ['confinement',      false, 't-confinement'],
];

// Scale 2/3 (Atoms/Molecules) — matching AtomToggles in atom_engine.h
export const SCALE2_TOGGLES = [
    ['ae-ionic', true, 'aeSetIonic'],
    ['ae-vdw', true, 'aeSetVdw'],
    ['ae-bonds-force', true, 'aeSetBondsForce'],
    ['ae-bonding', true, 'aeSetBonding'],
    ['ae-damping', false, 'aeSetDamping'],
    ['ae-speed-limit', true, 'aeSetSpeedLimit'],
    // Phase 3 extensions (off by default — scenarios enable as needed)
    ['ae-hbonds', false, 'aeSetHBonds'],
    ['ae-angle', false, 'aeSetAngleStrain'],
    ['ae-dipole', false, 'aeSetDipoleDipole'],
    ['ae-thermostat', false, 'aeSetThermostat'],
    ['ae-electronegativity', false, 'aeSetElectronegativity'],
];

// Scale 0 scenario-specific toggle overrides.
// Maps scenario name to array of [toggleKey, value, domId].
// When a scenario loads, these overrides are applied AFTER defaults reset.
export const SCALE0_SCENARIO_OVERRIDES = {
    'flux-dual-substrate': [
        ['dual_substrate', true, 't-dual'],
    ],
    'flux-cosmic-web': [
        ['gravity', true, 't-gravity'],
    ],
    'flux-gravitational-wave': [
        ['gravity', true, 't-gravity'],
    ],
    'flux-triad': [
        ['gravity', true, 't-gravity'],
    ],
    'flux-baryon': [
        ['gravity', true, 't-gravity'],
        ['confinement', true, 't-confinement'],
        ['genesis', false, 't-genesis'],
    ],
    'flux-black-hole': [
        ['gravity', true, 't-gravity'],
        ['genesis', false, 't-genesis'],
    ],
    'flux-stable-vortex': [
        ['damping', false, 't-damping'],
        ['genesis', false, 't-genesis'],
    ],
    'flux-cyclotron': [
        ['lorentz_force', true, 't-lorentz'],
    ],
    'flux-meson': [
        ['confinement', true, 't-confinement'],
        ['genesis', false, 't-genesis'],
    ],
    'flux-string-breaking': [
        ['confinement', true, 't-confinement'],
        ['genesis', true, 't-genesis'],
    ],
    'flux-dark-matter': [
        ['gravity', true, 't-gravity'],
        ['genesis', false, 't-genesis'],
    ],
    'flux-baryogenesis': [
        ['genesis', true, 't-genesis'],
        ['gravity', true, 't-gravity'],
    ],
    'flux-vacuum-foam': [
        ['genesis', true, 't-genesis'],
        ['damping', true, 't-damping'],
    ],
    'quantum-born-rule': [
        ['genesis', true, 't-genesis'],
        ['damping', true, 't-damping'],
    ],
    'quantum-double-slit': [
        ['selective_damping', true, 't-selective'],
        ['coupling', false, 't-coupling'],
        ['genesis', true, 't-genesis'],
        ['gauss_projection', false, 't-gauss'],
        ['forces', false, 't-forces'],
        ['movement', false, 't-movement'],
    ],
    'quantum-tunnel': [
        ['coupling', true, 't-coupling'],
        ['genesis', false, 't-genesis'],
    ],
    'quantum-well': [
        ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'],
        ['genesis', false, 't-genesis'],
    ],
    'quantum-entangle': [
        ['genesis', true, 't-genesis'],
        ['coupling', true, 't-coupling'],
    ],
    'quantum-aharonov-bohm': [
        ['coupling', true, 't-coupling'],
        ['genesis', false, 't-genesis'],
    ],
    'quantum-casimir': [
        ['genesis', false, 't-genesis'],
        ['damping', true, 't-damping'],
    ],
    'quantum-zeno': [
        ['genesis', true, 't-genesis'],
        ['damping', true, 't-damping'],
    ],
    // ── Standard Model scenarios ──
    'sm-particle-zoo': [
        ['genesis', false, 't-genesis'],
    ],
    'sm-higgs-field': [
        ['genesis', false, 't-genesis'],
        ['damping', false, 't-damping'],
    ],
    'sm-higgs-mechanism': [
        ['genesis', false, 't-genesis'],
        ['damping', false, 't-damping'],
    ],
    'sm-electroweak': [
        ['genesis', false, 't-genesis'],
    ],
    'sm-three-generations': [
        ['genesis', false, 't-genesis'],
    ],
    'sm-qcd-vacuum': [
        ['confinement', true, 't-confinement'],
        ['genesis', false, 't-genesis'],
        ['gravity', true, 't-gravity'],
    ],
};

// Light scenarios: pure EM wave propagation (no matter coupling)
export const LIGHT_SCENARIO_OVERRIDES = [
    ['selective_damping', true, 't-selective'],
    ['coupling',   false, 't-coupling'],
    ['damping',    false, 't-damping'],
    ['genesis',    false, 't-genesis'],
    ['gauss_projection', false, 't-gauss'],
    ['forces',     false, 't-forces'],
    ['movement',   false, 't-movement'],
    ['poisson_coulomb', false, 't-poisson'],
];
