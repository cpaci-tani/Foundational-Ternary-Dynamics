/**
 * Toggle Configuration — Single source of truth for all scale toggles.
 *
 * Each entry: [toggleKey, defaultValue, domElementId]
 *   - toggleKey: String key used by bridge.setToggle() / bridge.getToggle()
 *   - defaultValue: Boolean initial state when scenario loads
 *   - domElementId: HTML checkbox/button ID for UI sync
 *
 * Extracted from app.js to eliminate scattered toggle definitions.
 * Human coders: add new toggles here, not in app.js or bridge-init.js.
 */

/**
 * @typedef {('empty' |
 * 'flux-pulse' |
 * 'flux-dipole' |
 * 'flux-standing' |
 * 'flux-nested-standing' |
 * 'flux-soliton' |
 * 'flux-interference' |
 * 'flux-vortex' |
 * 'flux-dual-substrate' |
 * 'flux-cascade' |
 * 'flux-random-genesis' |
 * 'flux-genesis-between-gates' |
 * 'flux-pair-production' |
 * 'flux-annihilation' |
 * 'flux-vacuum-foam' |
 * 'flux-meson' |
 * 'flux-string-breaking' |
 * 'flux-baryon' |
 * 'flux-cyclotron' |
 * 'flux-screening' |
 * 'flux-thermalization' |
 * 'flux-triad' |
 * 'flux-zero-point' |
 * 'light-rainbow' |
 * 'light-dipole' |
 * 'light-two-slit' |
 * 'light-photon-race' |
 * 'quantum-born-rule' |
 * 'quantum-double-slit' |
 * 'quantum-eraser' |
 * 'quantum-tunnel' |
 * 'quantum-well' |
 * 'quantum-entangle' |
 * 'quantum-aharonov-bohm' |
 * 'quantum-casimir' |
 * 'quantum-zeno' |
 * 's0-seed-up-quark' |
 * 's0-seed-down-quark' |
 * 's0-seed-strange-quark' |
 * 's0-seed-charm-quark' |
 * 's0-seed-bottom-quark' |
 * 's0-seed-top-quark' |
 * 's0-seed-anti-up-quark' |
 * 's0-seed-anti-down-quark' |
 * 's0-seed-anti-strange-quark' |
 * 's0-seed-anti-charm-quark' |
 * 's0-seed-anti-bottom-quark' |
 * 's0-seed-anti-top-quark' |
 * 's0-seed-higgs-field' |
 * 's0-seed-gluon' |
 * 's0-seed-beta-decay' |
 * 's0-seed-ee-annihilation' |
 * 's0-seed-hydrogen' |
 * 's0-seed-helium' |
 * 's0-seed-h2-bond-formation' |
 * 's0-seed-spark-of-life' |
 * 's0-seed-wilson-loop' |
 * 's0-seed-flux-tube' |
 * 's0-seed-monopole' |
 * 's0-seed-instanton' |
 * 's0-seed-schwarzschild' |
 * 's0-seed-gravitational-lensing' |
 * 's0-seed-gravitational-wave' |
 * 's0-seed-massive-body' |
 * 's0-seed-dynamical-flux-dressing' |
 * 's0-seed-moving-source-reciprocity' |
 * 's0-seed-time-gravity-well' |
 * 's0-seed-time-twin-clocks' |
 * 's0-seed-time-horizon' |
 * 's0-seed-sloop' |
 * 's0-seed-observer-cell' |
 * 's0-field-plane-wave' |
 * 's0-field-standing-wave' |
 * 's0-field-uniform-e' |
 * 's0-field-uniform-b' |
 * 's0-field-photon-pulse' |
 * 's0-field-rf-lattice-wave' |
 * 's0-field-light-lattice-wave' |
 * 's0-field-sound-lattice-wave' |
 * 's0-field-sound-collision' |
 * 's0-field-thomson-scattering' |
 * 's0-field-thomson-unlocked-recoil' |
 * 's0-field-spacetime-forcing-boundary' |
 * 's0-field-electric-dipole' |
 * 's0-field-magnetic-dipole' |
 * 's0-field-vortex-line' |
 * 's0-seed-octahedron' |
 * 's0-seed-cuboctahedron' |
 * 's0-seed-stella-octangula' |
 * 's0-seed-moore-cell' |
 * 's0-seed-moore-decomposition' |
 * 's0-vacuum-electron' |
 * 's0-vacuum-muon' |
 * 's0-vacuum-tau' |
 * 's0-vacuum-positron' |
 * 's0-vacuum-antimuon' |
 * 's0-vacuum-antitau' |
 * 's0-vacuum-electron-neutrino' |
 * 's0-vacuum-muon-neutrino' |
 * 's0-vacuum-tau-neutrino' |
 * 's0-vacuum-electron-antineutrino' |
 * 's0-vacuum-muon-antineutrino' |
 * 's0-vacuum-tau-antineutrino' |
 * 's0-vacuum-photon' |
 * 's0-vacuum-w-boson' |
 * 's0-vacuum-w-minus-boson' |
 * 's0-vacuum-z-boson' |
 * 's0-vacuum-higgs' |
 * 's0-vacuum-proton' |
 * 's0-vacuum-neutron' |
 * 's0-vacuum-pion-charged' |
 * 's0-vacuum-pion-neutral' |
 * 's0-vacuum-kaon-charged' |
 * 's0-seed-ew-phase-transition' |
 * 's0-seed-quark-gluon-plasma' |
 * 's0-seed-emergent-ic1' |
 * 's0-seed-emergent-ic3-collision' |
 * 's0-seed-emergent-ic4-subthreshold' |
 * 's0-seed-emergent-ic2-thermal-runaway' |
 * 's0-seed-emergent-ic1-diagonal' |
 * 's0-seed-emergent-ic1-isotropic' |
 * 's0-seed-emergent-ic1-viz' |
 * 's0-seed-emergent-ic1-diagonal-viz' |
 * 's0-seed-emergent-ic1-isotropic-viz' |
 * 's0-seed-cluster-law' |
 * 's0-seed-cluster-law-subknee' |
 * 's0-seed-cluster-law-knee' |
 * 's0-seed-cluster-law-superknee' |
 * 's0-seed-thermal-ignition' |
 * 's0-seed-de-broglie-clock')} ScenarioId
 */

// Scale 0 (Lattice) — physics term toggles matching TermToggles in term_toggles.h
export const SCALE0_TOGGLES = [
    ['wave_propagation', true,  't-wave'],
    ['coupling',         true,  't-coupling'],
    ['damping',          true,  't-damping'],
    ['genesis',          true,  't-genesis'],
    ['evaporation',      false, 't-evaporation'],
    ['gauss_projection', true,  't-gauss'],
    ['forces',           true,  't-forces'],
    ['gravity',          false, 't-gravity'],
    ['movement',         true,  't-movement'],
    ['poisson_coulomb',  true,  't-poisson'],
    ['lorentz_force',    false, 't-lorentz'],
    ['selective_damping', true,  't-selective'],
    ['larmor_radiation', false, 't-larmor'],
    ['dual_substrate',   false, 't-dual'],
    ['confinement',      false, 't-confinement'],
    ['color_forces',     false, 't-color-forces'],
    ['strong_force',     false, 't-strong-force'],
    ['exchange_force',   false, 't-exchange'],
    ['weak_transmutation', false, 't-weak'],
    // FTD-0271 de Broglie internal clock. Listed here so it RESETS to false on
    // every scenario load (the −ω₀²·J mass term must not leak to other
    // scenarios' manifested voxels). It has no dashboard checkbox ('t-de-broglie'
    // is absent — the setCheckbox/readCheckbox helpers no-op on a missing id);
    // the s0-seed-de-broglie-clock scenario re-enables it in its registry load().
    ['de_broglie_clock', false, 't-de-broglie'],
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
// These are normally LONG-TERM RESEARCH CONTROLS owned by the user across
// scenario loads. Isolated certification scenarios may explicitly override
// them inside setupScenario(). Putting them in SCALE0_TOGGLES would cause the
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
//
// @type {Partial<Record<ScenarioId, Array<[string, boolean, string]>>>}
const isolatedScale0Profile = (...enabledTerms) => {
    const enabled = new Set(enabledTerms);
    return SCALE0_TOGGLES.map(([key, _defaultValue, elementId]) =>
        [key, enabled.has(key), elementId]);
};

export const SCALE0_SCENARIO_OVERRIDES = {
    'flux-pulse': [
        ['wave_propagation', true, 't-wave'],
        ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'],
        ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'],
        ['evaporation', false, 't-evaporation'],
        ['gauss_projection', false, 't-gauss'],
        ['forces', false, 't-forces'],
        ['gravity', false, 't-gravity'],
        ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'],
        ['lorentz_force', false, 't-lorentz'],
        ['dual_substrate', false, 't-dual'],
        ['color_forces', false, 't-color-forces'],
        ['strong_force', false, 't-strong-force'],
        ['exchange_force', false, 't-exchange'],
        ['weak_transmutation', false, 't-weak'],
    ],
    'flux-pair-production': [
        // Visible mirror of the isolated native pair-rule profile. The hidden
        // pair_production toggle is enabled inside setupScenario itself so it
        // cannot leak onto an inactive main bridge when a worker owns Scale 0.
        ['wave_propagation', false, 't-wave'],
        ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'],
        ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'],
        ['evaporation', false, 't-evaporation'],
        ['gauss_projection', false, 't-gauss'],
        ['forces', false, 't-forces'],
        ['gravity', false, 't-gravity'],
        ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'],
        ['lorentz_force', false, 't-lorentz'],
        ['larmor_radiation', false, 't-larmor'],
        ['dual_substrate', false, 't-dual'],
        ['confinement', false, 't-confinement'],
        ['color_forces', false, 't-color-forces'],
        ['strong_force', false, 't-strong-force'],
        ['exchange_force', false, 't-exchange'],
        ['weak_transmutation', false, 't-weak'],
        ['de_broglie_clock', false, 't-de-broglie'],
    ],
    'flux-annihilation': [
        // Visible mirror of the isolated movement-only collision profile.
        ['wave_propagation', false, 't-wave'],
        ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'],
        ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'],
        ['evaporation', false, 't-evaporation'],
        ['gauss_projection', false, 't-gauss'],
        ['forces', false, 't-forces'],
        ['gravity', false, 't-gravity'],
        ['movement', true, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'],
        ['lorentz_force', false, 't-lorentz'],
        ['larmor_radiation', false, 't-larmor'],
        ['dual_substrate', false, 't-dual'],
        ['confinement', false, 't-confinement'],
        ['color_forces', false, 't-color-forces'],
        ['strong_force', false, 't-strong-force'],
        ['exchange_force', false, 't-exchange'],
        ['weak_transmutation', false, 't-weak'],
        ['de_broglie_clock', false, 't-de-broglie'],
    ],
    'flux-dual-substrate': [
        ['dual_substrate', true, 't-dual'],
    ],
    // Evidence-gated profiles mirror the exact isolated native term sets used
    // by their C++ and JS scenario bodies.  The loader applies these after its
    // broad defaults, so partial overrides would silently re-enable unrelated
    // forces, damping, genesis, or movement in the user-facing run.
    'flux-triad': isolatedScale0Profile(),
    'flux-screening': isolatedScale0Profile(),
    'flux-baryon': isolatedScale0Profile('movement'),
    'flux-meson': isolatedScale0Profile('movement'),
    'flux-string-breaking': isolatedScale0Profile('movement'),
    'flux-cyclotron': isolatedScale0Profile(
        'forces', 'movement', 'poisson_coulomb', 'lorentz_force'),
    'flux-thermalization': isolatedScale0Profile('wave_propagation'),
    'flux-vacuum-foam': isolatedScale0Profile('wave_propagation'),
    'flux-cascade': isolatedScale0Profile('genesis'),
    'flux-random-genesis': isolatedScale0Profile('genesis'),
    'flux-zero-point': [
        // Finite periodic random-wave invariant probe: only the bare wave map.
        ['genesis', false, 't-genesis'],
        ['damping', false, 't-damping'],
        ['gauss_projection', false, 't-gauss'],
    ],
    'flux-genesis-between-gates': [
        // FTD-0388 one-tick gate discriminator. The three band amplitudes
        // (1.5160 / 1.5250 / 1.5340) are exact at the initial decision. The
        // middle band manifesting at all is the
        // FTD-0388 signature (the retired 3·K_B = 1.533 gate kept it silent).
        // The C++/JS scenario bodies additionally clear every research toggle.
        ['genesis', true, 't-genesis'],
        ['wave_propagation', false, 't-wave'],
        ['coupling', false, 't-coupling'],
        ['gauss_projection', false, 't-gauss'],
        // selective_damping BEFORE damping: dependents off first, so the
        // per-setToggle C++ validate never sees the transient invalid combo
        // "selective_damping requires damping".
        ['selective_damping', false, 't-selective'],
        ['damping', false, 't-damping'],
        ['movement', false, 't-movement'],
        ['weak_transmutation', false, 't-weak'],
        // Explicit (baseline already false): the C++ branch pins dual_substrate
        // off BEFORE injecting so the bands live on the mono substrate — keep
        // the checkbox in sync if a prior scenario turned dual on.
        ['dual_substrate', false, 't-dual'],
    ],
    'quantum-born-rule': isolatedScale0Profile('genesis'),
    'quantum-double-slit': isolatedScale0Profile(
        'wave_propagation', 'gauss_projection'),
    'quantum-eraser': isolatedScale0Profile(
        'wave_propagation', 'gauss_projection', 'coupling'),
    'quantum-tunnel': isolatedScale0Profile(
        'wave_propagation', 'gauss_projection', 'coupling'),
    'quantum-well': isolatedScale0Profile('wave_propagation'),
    'quantum-entangle': [
        ['genesis', false, 't-genesis'],
        ['evaporation', false, 't-evaporation'],
        ['coupling', false, 't-coupling'],
        ['movement', false, 't-movement'],
    ],
    'quantum-aharonov-bohm': isolatedScale0Profile(
        'wave_propagation', 'gauss_projection'),
    'quantum-casimir': isolatedScale0Profile('wave_propagation'),
    'quantum-zeno': isolatedScale0Profile('genesis'),

    // Gauge geometry. The implemented CPU binding path is color_forces;
    // confinement is an intent-only flag and strong_force is a CPU no-op.
    's0-seed-dynamical-flux-dressing': isolatedScale0Profile('wave_propagation', 'coupling'),
    's0-seed-moving-source-reciprocity': isolatedScale0Profile(
        'wave_propagation', 'coupling', 'forces', 'movement'),
    's0-seed-wilson-loop':         [['genesis', false, 't-genesis']],
    's0-seed-flux-tube':           [['genesis', false, 't-genesis'], ['color_forces', true, 't-color-forces']],
    's0-seed-monopole':            [['genesis', false, 't-genesis']],
    's0-seed-instanton':           [['genesis', false, 't-genesis']],
    // Gravity-shaped research setups.
    's0-seed-schwarzschild':       isolatedScale0Profile(),
    // These legacy gravity/time names are exact aliases of one plain native
    // transverse harmonic. Keep only its actual wave operator enabled.
    's0-seed-gravitational-wave':  isolatedScale0Profile('wave_propagation'),
    's0-seed-gravitational-lensing': isolatedScale0Profile('wave_propagation'),
    's0-seed-massive-body':        [['genesis', false, 't-genesis'], ['gravity', true, 't-gravity']],
    // Time-dilation scenarios. gravity-well + twin-clocks reuse the real-mass
    // body (latency_field is enabled by SCALE0_MASS_GRAVITY_SCENARIOS below);
    // horizon reuses the seed-bias Schwarzschild well (gravity toggle proxy).
    's0-seed-time-gravity-well':   isolatedScale0Profile('wave_propagation'),
    's0-seed-time-twin-clocks':    isolatedScale0Profile('wave_propagation'),
    's0-seed-time-horizon':        isolatedScale0Profile(),
    // Self-reference / observation pedagogy seeds (Scale 0)
    's0-seed-sloop':               [['genesis', false, 't-genesis']],
    's0-seed-observer-cell':       [['genesis', false, 't-genesis']],

    // Particles + composites — toggle profiles for s0-seed-{electron-l3, muon,
    // tau, photon, w-boson, z-boson, higgs-boson, positron, pion, proton-l4,
    // neutron, neutrino, quark, antiquark} removed in audit-3+audit-4 2026-04-28.
    // Use the s0-vacuum-* toggle profiles below for canonical entries.
    // Selected amplitude/shape cohorts. Their physical particle names are
    // rejected by qualification; each runs only the unprojected native wave map.
    's0-seed-up-quark':       isolatedScale0Profile('wave_propagation'),
    's0-seed-down-quark':     isolatedScale0Profile('wave_propagation'),
    's0-seed-strange-quark':  isolatedScale0Profile('wave_propagation'),
    's0-seed-charm-quark':    isolatedScale0Profile('wave_propagation'),
    's0-seed-bottom-quark':   isolatedScale0Profile('wave_propagation'),
    's0-seed-top-quark':      isolatedScale0Profile('wave_propagation'),
    's0-seed-anti-up-quark':       isolatedScale0Profile('wave_propagation'),
    's0-seed-anti-down-quark':     isolatedScale0Profile('wave_propagation'),
    's0-seed-anti-strange-quark':  isolatedScale0Profile('wave_propagation'),
    's0-seed-anti-charm-quark':    isolatedScale0Profile('wave_propagation'),
    's0-seed-anti-bottom-quark':   isolatedScale0Profile('wave_propagation'),
    's0-seed-anti-top-quark':      isolatedScale0Profile('wave_propagation'),
    's0-seed-higgs-field':    isolatedScale0Profile('wave_propagation'),
    's0-seed-gluon':          isolatedScale0Profile('wave_propagation'),
    's0-vacuum-electron':     isolatedScale0Profile('wave_propagation'),
    's0-vacuum-muon':         isolatedScale0Profile('wave_propagation'),
    's0-vacuum-tau':          isolatedScale0Profile('wave_propagation'),
    's0-vacuum-positron':     isolatedScale0Profile('wave_propagation'),
    's0-vacuum-antimuon':     isolatedScale0Profile('wave_propagation'),
    's0-vacuum-antitau':      isolatedScale0Profile('wave_propagation'),
    's0-vacuum-w-boson':      isolatedScale0Profile('wave_propagation'),
    's0-vacuum-w-minus-boson': isolatedScale0Profile('wave_propagation'),
    's0-vacuum-z-boson':      isolatedScale0Profile('wave_propagation'),
    's0-vacuum-higgs':        isolatedScale0Profile('wave_propagation'),
    // Prepared locked-nucleus Coulomb candidates. No gravity, wave, color,
    // genesis, or inherited interaction terms participate.
    's0-seed-hydrogen':       isolatedScale0Profile('forces', 'poisson_coulomb', 'movement'),
    's0-seed-helium':         isolatedScale0Profile('forces', 'poisson_coulomb', 'movement'),
    's0-seed-h2-bond-formation': isolatedScale0Profile('forces', 'poisson_coulomb', 'movement'),
    's0-seed-spark-of-life': isolatedScale0Profile('wave_propagation', 'coupling', 'damping', 'genesis', 'gauss_projection', 'forces', 'movement'),

    // Unlocked selected-color candidates: static dressing, force, color force,
    // and movement only. No inherited wave, gravity, Poisson, weak, or reaction terms.
    's0-vacuum-proton':       isolatedScale0Profile('forces', 'movement', 'color_forces'),
    's0-vacuum-neutron':      isolatedScale0Profile('forces', 'movement', 'color_forces'),
    's0-vacuum-pion-charged': isolatedScale0Profile('forces', 'movement', 'color_forces'),
    's0-vacuum-pion-neutral': isolatedScale0Profile('forces', 'movement', 'color_forces'),
    's0-vacuum-kaon-charged': isolatedScale0Profile('forces', 'movement', 'color_forces'),
    's0-seed-ee-annihilation': isolatedScale0Profile('movement'),
    's0-seed-ew-phase-transition': isolatedScale0Profile('wave_propagation', 'gauss_projection', 'genesis', 'ew_background_sweep'),
    's0-seed-quark-gluon-plasma': isolatedScale0Profile('wave_propagation', 'gauss_projection', 'movement', 'langevin'),

    // Exact harmonic eigenmodes: isolate the production kick-drift wave map.
    's0-field-plane-wave': [
        ['wave_propagation', true, 't-wave'],
        ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'],
        ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'],
        ['evaporation', false, 't-evaporation'],
        ['gauss_projection', false, 't-gauss'],
        ['forces', false, 't-forces'],
        ['gravity', false, 't-gravity'],
        ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'],
        ['lorentz_force', false, 't-lorentz'],
        ['dual_substrate', false, 't-dual'],
        ['color_forces', false, 't-color-forces'],
        ['strong_force', false, 't-strong-force'],
        ['exchange_force', false, 't-exchange'],
        ['weak_transmutation', false, 't-weak'],
    ],
    's0-field-standing-wave': [
        ['wave_propagation', true, 't-wave'],
        ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'],
        ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'],
        ['evaporation', false, 't-evaporation'],
        ['gauss_projection', false, 't-gauss'],
        ['forces', false, 't-forces'],
        ['gravity', false, 't-gravity'],
        ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'],
        ['lorentz_force', false, 't-lorentz'],
        ['dual_substrate', false, 't-dual'],
        ['color_forces', false, 't-color-forces'],
        ['strong_force', false, 't-strong-force'],
        ['exchange_force', false, 't-exchange'],
        ['weak_transmutation', false, 't-weak'],
    ],
    // Other field configurations — genesis off so the field pattern stays clean.
    's0-field-uniform-e':        [['genesis', false, 't-genesis']],
    's0-field-uniform-b':        [['genesis', false, 't-genesis']],
    's0-field-photon-pulse':     isolatedScale0Profile('wave_propagation', 'gauss_projection'),
    's0-field-rf-lattice-wave': [
        ['wave_propagation', true, 't-wave'],
        ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'],
        ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'],
        ['gauss_projection', false, 't-gauss'],
        ['forces', false, 't-forces'],
        ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'],
        ['lorentz_force', false, 't-lorentz'],
    ],
    's0-field-light-lattice-wave': [
        ['wave_propagation', true, 't-wave'],
        ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'],
        ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'],
        ['gauss_projection', false, 't-gauss'],
        ['forces', false, 't-forces'],
        ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'],
        ['lorentz_force', false, 't-lorentz'],
    ],
    's0-field-sound-lattice-wave': [
        ['wave_propagation', true, 't-wave'],
        ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'],
        ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'],
        ['gauss_projection', false, 't-gauss'],
        ['forces', false, 't-forces'],
        ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'],
        ['lorentz_force', false, 't-lorentz'],
    ],
    's0-field-sound-collision': isolatedScale0Profile('wave_propagation'),
    's0-field-thomson-scattering': [
        ['coupling', true, 't-coupling'],
        ['damping', false, 't-damping'],
        ['genesis', false, 't-genesis'],
        ['gauss_projection', false, 't-gauss'],
        ['forces', false, 't-forces'],
        ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'],
    ],
    's0-field-thomson-unlocked-recoil': [
        ['coupling', true, 't-coupling'],
        ['damping', false, 't-damping'],
        ['genesis', false, 't-genesis'],
        ['gauss_projection', false, 't-gauss'],
        ['forces', true, 't-forces'],
        ['movement', true, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'],
    ],
    's0-field-spacetime-forcing-boundary': [
        // Pure wave-equation half of FTD-0253: keep the center pulse below
        // genesis and remove non-wave phases so the dashboard seed matches
        // the controlled WAVE branch. The DIFF branch is not an engine phase;
        // it lives only in the linked counterfactual demo page.
        ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'],
        ['genesis', false, 't-genesis'],
        ['gauss_projection', false, 't-gauss'],
        ['forces', false, 't-forces'],
        ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'],
    ],
    's0-field-electric-dipole':  [['genesis', false, 't-genesis']],
    's0-field-magnetic-dipole':  [['genesis', false, 't-genesis']],
    's0-field-vortex-line':      [['genesis', false, 't-genesis']],

    // Beta-decay — leptonic output of weak transmutation needs both
    // dual_substrate (chiral L/R substrates) and weak_transmutation on.
    // Prepared cohort; only the selected weak polarity-flip rule is active.
    's0-seed-beta-decay': isolatedScale0Profile('dual_substrate', 'weak_transmutation'),

    // Moore Seeds — genesis off so the geometric pattern stays clean
    's0-seed-octahedron':          [['genesis', false, 't-genesis']],
    's0-seed-cuboctahedron':       [['genesis', false, 't-genesis']],
    's0-seed-stella-octangula':    [['genesis', false, 't-genesis']],
    's0-seed-moore-cell':          [['genesis', false, 't-genesis']],
    's0-seed-moore-decomposition': [['genesis', false, 't-genesis']],
    's0-seed-thermal-ignition': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-emergent-ic1': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-emergent-ic3-collision': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-emergent-ic4-subthreshold': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-emergent-ic2-thermal-runaway': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-emergent-ic1-diagonal': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-emergent-ic1-isotropic': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-emergent-ic1-viz': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-emergent-ic1-diagonal-viz': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-emergent-ic1-isotropic-viz': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-cluster-law': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-de-broglie-clock': isolatedScale0Profile(
        'wave_propagation', 'de_broglie_clock'),

    // Native neutral wave candidates. These are propagation experiments, so
    // every non-wave production phase is disabled explicitly in both the
    // WASM and JS fallback paths.
    's0-vacuum-photon': [
        ['wave_propagation', true, 't-wave'],
        ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'],
        ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'],
        ['gauss_projection', true, 't-gauss'],
        ['forces', false, 't-forces'],
        ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'],
        ['lorentz_force', false, 't-lorentz'],
    ],
    's0-vacuum-electron-neutrino': [
        ['wave_propagation', true, 't-wave'], ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'], ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'], ['gauss_projection', true, 't-gauss'],
        ['forces', false, 't-forces'], ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'], ['lorentz_force', false, 't-lorentz'],
    ],
    's0-vacuum-muon-neutrino': [
        ['wave_propagation', true, 't-wave'], ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'], ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'], ['gauss_projection', true, 't-gauss'],
        ['forces', false, 't-forces'], ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'], ['lorentz_force', false, 't-lorentz'],
    ],
    's0-vacuum-tau-neutrino': [
        ['wave_propagation', true, 't-wave'], ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'], ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'], ['gauss_projection', true, 't-gauss'],
        ['forces', false, 't-forces'], ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'], ['lorentz_force', false, 't-lorentz'],
    ],
    's0-vacuum-electron-antineutrino': [
        ['wave_propagation', true, 't-wave'], ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'], ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'], ['gauss_projection', true, 't-gauss'],
        ['forces', false, 't-forces'], ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'], ['lorentz_force', false, 't-lorentz'],
    ],
    's0-vacuum-muon-antineutrino': [
        ['wave_propagation', true, 't-wave'], ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'], ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'], ['gauss_projection', true, 't-gauss'],
        ['forces', false, 't-forces'], ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'], ['lorentz_force', false, 't-lorentz'],
    ],
    's0-vacuum-tau-antineutrino': [
        ['wave_propagation', true, 't-wave'], ['coupling', false, 't-coupling'],
        ['damping', false, 't-damping'], ['selective_damping', false, 't-selective'],
        ['genesis', false, 't-genesis'], ['gauss_projection', true, 't-gauss'],
        ['forces', false, 't-forces'], ['movement', false, 't-movement'],
        ['poisson_coulomb', false, 't-poisson'], ['lorentz_force', false, 't-lorentz'],
    ],
};

// ── Per-scenario BOUNDARY preference ────────────────────────────────
// Mirrors SCALE0_SCENARIO_OVERRIDES (toggle defaults), but for the lattice
// boundary — which the loader otherwise reads from the live DOM controls
// (#boundary-select / #toggle-reflective). A scenario declares an entry here
// when its physics needs a specific boundary regardless of the user's current
// DOM selection; the loader applies these at load AND on resize, and falls
// back to the DOM controls for any scenario without an entry. (This also
// decouples a scenario's boundary need from raw DOM reads — the UI↔bridge
// coupling noted in SPEC_SCALE0_SCENARIO_ARCHITECTURE.md §6.6.)
//
//   mode:       number  → 0 periodic, 1 reflective, 2 dispersal.
//   reflective: boolean → legacy alias for mode 1/2.
//   shape:      string  → optional 'boundary-select' value (e.g. 'cube').
//
// @type {Partial<Record<ScenarioId, { mode?: number, reflective?: boolean, shape?: string }>>}
export const SCALE0_SCENARIO_BOUNDARY = {
    'flux-zero-point': { mode: 0 },
    's0-seed-dynamical-flux-dressing': { mode: 0 },
    's0-seed-moving-source-reciprocity': { mode: 0 },

    // These initial conditions are uniform across at least one transverse
    // face or are defined as periodic harmonics. A dispersal sponge edits the
    // seed at tick one and injects a false longitudinal/transverse distortion.
    'flux-standing': { mode: 0 },
    'flux-cyclotron': { mode: 0 },
    'flux-thermalization': { mode: 0 },
    'flux-vacuum-foam': { mode: 0 },
    'flux-nested-standing': { mode: 0 },
    'flux-interference': { mode: 0 },
    'light-rainbow': { mode: 0 },
    'light-dipole': { mode: 0 },
    'light-two-slit': { mode: 0 },
    'light-photon-race': { mode: 0 },
    'quantum-double-slit': { mode: 0 },
    'quantum-eraser': { mode: 0 },
    'quantum-tunnel': { mode: 0 },
    'quantum-well': { mode: 0 },
    'quantum-aharonov-bohm': { mode: 0 },
    'quantum-casimir': { mode: 0 },
    's0-field-plane-wave': { mode: 0 },
    's0-field-standing-wave': { mode: 0 },
    's0-field-photon-pulse': { mode: 0 },
    's0-field-rf-lattice-wave': { mode: 0 },
    's0-field-light-lattice-wave': { mode: 0 },
    's0-field-sound-lattice-wave': { mode: 0 },
    's0-field-sound-collision': { mode: 0 },
    's0-seed-thermal-ignition': { mode: 0 },
    // The genesis-response qualification matrix uses the engine's periodic
    // default.  Pin the dashboard to the same boundary so its auxiliary
    // dispersal default cannot change the measured cohort.
    's0-seed-emergent-ic1': { mode: 0 },
    's0-seed-emergent-ic3-collision': { mode: 0 },
    's0-seed-emergent-ic4-subthreshold': { mode: 0 },
    's0-seed-emergent-ic2-thermal-runaway': { mode: 0 },
    's0-seed-emergent-ic1-diagonal': { mode: 0 },
    's0-seed-emergent-ic1-isotropic': { mode: 0 },
    's0-seed-emergent-ic1-viz': { mode: 0 },
    's0-seed-emergent-ic1-diagonal-viz': { mode: 0 },
    's0-seed-emergent-ic1-isotropic-viz': { mode: 0 },
    's0-seed-cluster-law': { mode: 0 },
    's0-seed-de-broglie-clock': { mode: 0 },
    's0-seed-gravitational-wave': { mode: 0 },
    's0-seed-time-gravity-well': { mode: 0 },
    's0-seed-time-twin-clocks': { mode: 0 },
    's0-seed-gravitational-lensing': { mode: 0 },
    's0-vacuum-photon': { mode: 0 },
    's0-vacuum-electron-neutrino': { mode: 0 },
    's0-vacuum-muon-neutrino': { mode: 0 },
    's0-vacuum-tau-neutrino': { mode: 0 },
    's0-vacuum-electron-antineutrino': { mode: 0 },
    's0-vacuum-muon-antineutrino': { mode: 0 },
    's0-vacuum-tau-antineutrino': { mode: 0 },
    's0-seed-up-quark': { mode: 0 },
    's0-seed-down-quark': { mode: 0 },
    's0-seed-strange-quark': { mode: 0 },
    's0-seed-charm-quark': { mode: 0 },
    's0-seed-bottom-quark': { mode: 0 },
    's0-seed-top-quark': { mode: 0 },
    's0-seed-anti-up-quark': { mode: 0 },
    's0-seed-anti-down-quark': { mode: 0 },
    's0-seed-anti-strange-quark': { mode: 0 },
    's0-seed-anti-charm-quark': { mode: 0 },
    's0-seed-anti-bottom-quark': { mode: 0 },
    's0-seed-anti-top-quark': { mode: 0 },
    's0-seed-higgs-field': { mode: 0 },
    's0-seed-gluon': { mode: 0 },
    's0-vacuum-electron': { mode: 0 },
    's0-vacuum-muon': { mode: 0 },
    's0-vacuum-tau': { mode: 0 },
    's0-vacuum-positron': { mode: 0 },
    's0-vacuum-antimuon': { mode: 0 },
    's0-vacuum-antitau': { mode: 0 },
    's0-vacuum-w-boson': { mode: 0 },
    's0-vacuum-w-minus-boson': { mode: 0 },
    's0-vacuum-z-boson': { mode: 0 },
    's0-vacuum-higgs': { mode: 0 },
    's0-vacuum-proton': { mode: 0 },
    's0-vacuum-neutron': { mode: 0 },
    's0-vacuum-pion-charged': { mode: 0 },
    's0-vacuum-pion-neutral': { mode: 0 },
    's0-vacuum-kaon-charged': { mode: 0 },
    's0-seed-ee-annihilation': { mode: 0 },
    's0-seed-hydrogen': { mode: 0 },
    's0-seed-helium': { mode: 0 },
    's0-seed-h2-bond-formation': { mode: 0 },
    's0-seed-ew-phase-transition': { mode: 0 },
    's0-seed-quark-gluon-plasma': { mode: 0 },
    's0-seed-spark-of-life': { mode: 0 },
};

// Explicit opt-in for the imposed absorbing sponge. No currently qualified
// Scale-0 scenario depends on it; research setups may be added only with a
// boundary-specific behavioral test.
//
// @type {Set<ScenarioId>}
export const SCALE0_ABSORBING_SCENARIOS = new Set([
]);

// Scenarios whose gravity uses imposed manifested charge (rho = M_GRAVITATIONAL·|state|)
// via the latency-Poisson solver — the faithful gravity source (SPEC_FTD_LAGRANGIAN
// §4.2), distinct from the |J|² field-energy proxy above. The scenario-loader enables
// latency_field for these WITHOUT field_energy_gravity (the mass is the source).
//
// @type {Set<ScenarioId>}
export const SCALE0_MASS_GRAVITY_SCENARIOS = new Set([
    's0-seed-massive-body',
]);

// Light scenarios: pure EM wave propagation (no matter coupling)
export const LIGHT_SCENARIO_OVERRIDES = [
    ['selective_damping', false, 't-selective'],
    ['coupling',   false, 't-coupling'],
    ['damping',    false, 't-damping'],
    ['genesis',    false, 't-genesis'],
    ['gauss_projection', true, 't-gauss'],
    ['forces',     false, 't-forces'],
    ['movement',   false, 't-movement'],
    ['poisson_coulomb', false, 't-poisson'],
];

/**
 * Resolve the complete visible Scale-0 physics profile for a scenario.
 *
 * This is intentionally shared by scenario loading and the controls card's
 * "restore profile" action.  A partial override is not a scenario profile:
 * start from the canonical dashboard defaults, then apply every registered
 * scenario and light-family override.
 *
 * @param {ScenarioId|string} scenarioId
 * @returns {Array<[string, boolean, string]>}
 */
export function getScale0ScenarioToggleProfile(scenarioId) {
    const values = new Map(SCALE0_TOGGLES.map(([key, value]) => [key, value]));
    for (const [key, value] of SCALE0_SCENARIO_OVERRIDES[scenarioId] ?? []) {
        values.set(key, value);
    }
    if (String(scenarioId).startsWith('light-')) {
        for (const [key, value] of LIGHT_SCENARIO_OVERRIDES) values.set(key, value);
    }
    return SCALE0_TOGGLES.map(([key, defaultValue, elementId]) =>
        [key, values.has(key) ? !!values.get(key) : !!defaultValue, elementId]);
}
