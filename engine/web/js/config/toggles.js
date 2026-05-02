/**
 * Toggle Configuration — Single source of truth for all scale toggles.
 *
 * Each entry: [toggleKey, defaultValue, domElementId]
 *   - toggleKey: String key used by bridge.setToggle() / bridge.getToggle()
 *   - defaultValue: Boolean initial state when scenario loads
 *   - domElementId: HTML checkbox/button ID for UI sync
 *
 * Extracted from app_dag.js to eliminate scattered toggle definitions.
 * Human coders: add new toggles here, not in app_dag.js or wasm-bridge-dag.js.
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
    ['color_forces',     false, 't-color-forces'],
    ['strong_force',     false, 't-strong-force'],
    ['exchange_force',   false, 't-exchange'],
    ['weak_transmutation', false, 't-weak'],
];

// ── Engine toggles deliberately NOT in SCALE0_TOGGLES ───────────────
//
// The C++ `TermToggles` struct (engine/include/ftd/term_toggles.h) has
// additional boolean fields that are intentionally OMITTED from the
// dashboard whitelist above:
//
//   - `triad_binding`         — research toggle, no UI checkbox.
//   - `pair_production`       — research toggle, no UI checkbox.
//   - `latency_field`         — Poisson-based gravity potential mode;
//                                tied to specific campaigns.
//   - `exact_dual_gauss`      — Phase 1 electrodynamics variant; campaign-only.
//   - `emergent_forces`       — EFT-mode toggle (alpha = G_C²); mutually
//                                exclusive with `poisson_coulomb` per
//                                TermToggles::validate.
//   - `langevin`              — stochastic thermostat; paired with the
//                                non-bool params `langevin_T`,
//                                `langevin_gamma`, `langevin_seed`.
//   - `langevin_site_filter`  — enum, not bool.
//   - `bcc_stencil`           — enum (FULL / SC / FCC / BCC), not bool.
//   - `strict_validation`     — process-level guard, not physics.
//
// These are LONG-TERM RESEARCH CONTROLS owned by the user across
// scenario loads. Putting them in SCALE0_TOGGLES would cause the
// scenario-loader's whitelist-reset to clobber them on every scenario
// load, breaking research workflow. See the toggle-reset contract
// documented in `engine/web/js/scales/scale0/runtime/scenario-loader.js`
// and mirrored in `engine/include/ftd/scenarios.h`.

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
    'flux-triad': [
        ['gravity', true, 't-gravity'],
    ],
    'flux-baryon': [
        ['gravity', true, 't-gravity'],
        ['confinement', true, 't-confinement'],
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

    // Gauge — confinement on for flux tube / Wilson loop binding
    's0-seed-wilson-loop':         [['genesis', false, 't-genesis'], ['confinement', true, 't-confinement']],
    's0-seed-flux-tube':           [['genesis', false, 't-genesis'], ['confinement', true, 't-confinement'], ['color_forces', true, 't-color-forces'], ['strong_force', true, 't-strong-force']],
    's0-seed-monopole':            [['genesis', false, 't-genesis']],
    's0-seed-instanton':           [['genesis', false, 't-genesis']],
    // Gravity — latency field on for Schwarzschild
    's0-seed-schwarzschild':       [['genesis', false, 't-genesis'], ['gravity', true, 't-gravity']],
    's0-seed-frw-patch':           [['genesis', false, 't-genesis'], ['gravity', true, 't-gravity']],
    's0-seed-gravitational-wave':  [['genesis', false, 't-genesis'], ['gravity', true, 't-gravity']],
    // Self-reference / observation pedagogy seeds (live on Scale 0; the
    // Scale 11 reflexivity-mode UI was deleted 2026-05-01)
    's0-seed-sloop':               [['genesis', false, 't-genesis'], ['confinement', true, 't-confinement']],
    's0-seed-observer-cell':       [['genesis', false, 't-genesis']],

    // Particles + composites — toggle profiles for s0-seed-{electron-l3, muon,
    // tau, photon, w-boson, z-boson, higgs-boson, positron, pion, proton-l4,
    // neutron, neutrino, quark, antiquark} removed in audit-3+audit-4 2026-04-28.
    // Use the s0-vacuum-* toggle profiles below for canonical entries.
    // Atoms — strong force for nucleus binding + gravity for electron orbit.
    's0-seed-hydrogen':       [['genesis', false, 't-genesis'], ['confinement', true, 't-confinement'], ['color_forces', true, 't-color-forces'], ['strong_force', true, 't-strong-force'], ['gravity', true, 't-gravity']],
    's0-seed-helium':         [['genesis', false, 't-genesis'], ['confinement', true, 't-confinement'], ['color_forces', true, 't-color-forces'], ['strong_force', true, 't-strong-force'], ['gravity', true, 't-gravity']],
    's0-seed-2-hydrogen-atoms': [['genesis', false, 't-genesis'], ['confinement', true, 't-confinement'], ['color_forces', true, 't-color-forces'], ['strong_force', true, 't-strong-force'], ['gravity', true, 't-gravity']],

    // Field Configurations — genesis off so the field pattern stays clean
    's0-field-plane-wave':       [['genesis', false, 't-genesis']],
    's0-field-standing-wave':    [['genesis', false, 't-genesis']],
    's0-field-uniform-e':        [['genesis', false, 't-genesis']],
    's0-field-uniform-b':        [['genesis', false, 't-genesis']],
    's0-field-photon-pulse':     [['genesis', false, 't-genesis']],
    's0-field-electric-dipole':  [['genesis', false, 't-genesis']],
    's0-field-magnetic-dipole':  [['genesis', false, 't-genesis']],
    's0-field-vortex-line':      [['genesis', false, 't-genesis']],

    // Beta-decay — leptonic output of weak transmutation needs both
    // dual_substrate (chiral L/R substrates) and weak_transmutation on.
    // Order is important here: dual_substrate must be set before
    // weak_transmutation per the validator (see scenario-loader.js
    // applyToggleDefaults sort). Genesis is forced off so the pre-seeded
    // electron and neutrino aren't immediately recreated by genesis.
    's0-seed-beta-decay': [
        ['dual_substrate',     true,  't-dual'],
        ['weak_transmutation', true,  't-weak'],
        ['genesis',            false, 't-genesis'],
    ],

    // Moore Seeds — genesis off so the geometric pattern stays clean
    's0-seed-octahedron':          [['genesis', false, 't-genesis']],
    's0-seed-cuboctahedron':       [['genesis', false, 't-genesis']],
    's0-seed-stella-octangula':    [['genesis', false, 't-genesis']],
    's0-seed-moore-cell':          [['genesis', false, 't-genesis']],
    's0-seed-moore-decomposition': [['genesis', false, 't-genesis']],
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
