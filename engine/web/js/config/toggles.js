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
// ordinary scenario loads. Isolation helpers (configure_*_terms) MAY clear
// them when an IC promises an isolated map. Per-scenario pins for the JS
// loader / restore path live in SCALE0_SCENARIO_RESEARCH_TERMS. Putting them
// in SCALE0_TOGGLES would cause the whitelist-reset to clobber them on every
// load. See scenario-loader.js and engine/include/ftd/scenarios.h.

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
const SCALE0_TOGGLE_KEYS = new Set(SCALE0_TOGGLES.map(([key]) => key));

const isolatedScale0Profile = (...enabledTerms) => {
    const enabled = new Set();
    for (const term of enabledTerms) {
        if (!SCALE0_TOGGLE_KEYS.has(term)) {
            console.warn(
                '[toggles] isolatedScale0Profile: \'' + term + '\' is not in SCALE0_TOGGLES — use SCALE0_SCENARIO_RESEARCH_TERMS',
            );
            continue;
        }
        enabled.add(term);
    }
    return SCALE0_TOGGLES.map(([key, _defaultValue, elementId]) =>
        [key, enabled.has(key), elementId]);
};

export const SCALE0_SCENARIO_OVERRIDES = {
    'flux-pulse': isolatedScale0Profile('wave_propagation'),
    // Visible mirror of the isolated native pair-rule profile. Hidden
    // pair_production is pinned via SCALE0_SCENARIO_RESEARCH_TERMS.
    'flux-pair-production': isolatedScale0Profile(),
    // Visible mirror of the isolated movement-only collision profile.
    'flux-annihilation': isolatedScale0Profile('movement'),
    'flux-dual-substrate': isolatedScale0Profile('wave_propagation'),
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
    'flux-dipole': isolatedScale0Profile('wave_propagation'),
    'flux-standing': isolatedScale0Profile('wave_propagation'),
    'flux-nested-standing': isolatedScale0Profile('wave_propagation'),
    'flux-interference': isolatedScale0Profile('wave_propagation'),
    // Exact imposed helical-ring initial data; the qualified scenario is inert.
    'flux-vortex': isolatedScale0Profile(),
    'flux-soliton': isolatedScale0Profile('wave_propagation', 'gauss_projection'),
    'flux-cascade': isolatedScale0Profile('genesis'),
    'flux-random-genesis': isolatedScale0Profile('genesis'),
    // Finite periodic random-wave invariant probe: only the bare wave map.
    'flux-zero-point': isolatedScale0Profile('wave_propagation'),
    // FTD-0388 one-tick gate discriminator (bands at 1.5160 / 1.5250 / 1.5340).
    'flux-genesis-between-gates': isolatedScale0Profile('genesis'),
    'quantum-born-rule': isolatedScale0Profile('genesis'),
    'quantum-double-slit': isolatedScale0Profile(
        'wave_propagation', 'gauss_projection'),
    'quantum-eraser': isolatedScale0Profile(
        'wave_propagation', 'gauss_projection', 'coupling'),
    'quantum-tunnel': isolatedScale0Profile(
        'wave_propagation', 'gauss_projection', 'coupling'),
    'quantum-well': isolatedScale0Profile('wave_propagation'),
    'quantum-entangle': isolatedScale0Profile(),
    'quantum-aharonov-bohm': isolatedScale0Profile(
        'wave_propagation', 'gauss_projection'),
    'quantum-casimir': isolatedScale0Profile('wave_propagation'),
    'quantum-zeno': isolatedScale0Profile('genesis'),

    // Gauge geometry. Color binding is color_forces; confinement selects
    // the linear SIGMA_STRING shell (requires color_forces). strong_force
    // is a CPU no-op (GPU Yukawa).
    's0-seed-dynamical-flux-dressing': isolatedScale0Profile('wave_propagation', 'coupling'),
    's0-seed-moving-source-reciprocity': isolatedScale0Profile(
        'wave_propagation', 'coupling', 'forces', 'movement'),
    's0-seed-wilson-loop':         isolatedScale0Profile(),
    's0-seed-flux-tube':           isolatedScale0Profile(),
    's0-seed-monopole':            isolatedScale0Profile(),
    's0-seed-instanton':           isolatedScale0Profile(),
    // Gravity-shaped research setups.
    's0-seed-schwarzschild':       isolatedScale0Profile(),
    // These legacy gravity/time names are exact aliases of one plain native
    // transverse harmonic. Keep only its actual wave operator enabled.
    's0-seed-gravitational-wave':  isolatedScale0Profile('wave_propagation'),
    's0-seed-gravitational-lensing': isolatedScale0Profile('wave_propagation'),
    's0-seed-massive-body':        isolatedScale0Profile('gravity'),
    // Time-dilation labels are CLOSED NEGATIVE aliases of the plain wave /
    // schwarzschild seeds — NOT members of SCALE0_MASS_GRAVITY_SCENARIOS.
    // Do not enable latency_field here; that would contradict their qualifications.
    's0-seed-time-gravity-well':   isolatedScale0Profile('wave_propagation'),
    's0-seed-time-twin-clocks':    isolatedScale0Profile('wave_propagation'),
    's0-seed-time-horizon':        isolatedScale0Profile(),
    // Self-reference / observation pedagogy seeds (Scale 0)
    's0-seed-sloop':               isolatedScale0Profile(),
    's0-seed-observer-cell':       isolatedScale0Profile(),
    'empty':                       isolatedScale0Profile(),

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
    's0-seed-ew-phase-transition': isolatedScale0Profile('wave_propagation', 'gauss_projection', 'genesis'),
    's0-seed-quark-gluon-plasma': isolatedScale0Profile('wave_propagation', 'gauss_projection', 'movement'),

    // Exact harmonic eigenmodes: isolate the production kick-drift wave map.
    's0-field-plane-wave': isolatedScale0Profile('wave_propagation'),
    's0-field-standing-wave': isolatedScale0Profile('wave_propagation'),
    // Static uniform fields — every production phase off so the pattern stays clean.
    's0-field-uniform-e': isolatedScale0Profile(),
    's0-field-uniform-b': isolatedScale0Profile(),
    's0-field-photon-pulse': isolatedScale0Profile('wave_propagation', 'gauss_projection'),
    's0-field-rf-lattice-wave': isolatedScale0Profile('wave_propagation'),
    's0-field-light-lattice-wave': isolatedScale0Profile('wave_propagation'),
    's0-field-sound-lattice-wave': isolatedScale0Profile('wave_propagation'),
    's0-field-sound-collision': isolatedScale0Profile('wave_propagation'),
    's0-field-thomson-scattering': isolatedScale0Profile('wave_propagation', 'coupling'),
    's0-field-thomson-unlocked-recoil': isolatedScale0Profile(
        'wave_propagation', 'coupling', 'forces', 'movement'),
    // Pure wave-equation half of FTD-0253: keep the center pulse below genesis.
    's0-field-spacetime-forcing-boundary': isolatedScale0Profile('wave_propagation'),
    's0-field-electric-dipole':  isolatedScale0Profile(),
    's0-field-magnetic-dipole':  isolatedScale0Profile(),
    's0-field-vortex-line':      isolatedScale0Profile(),

    // Beta-decay — leptonic output of weak transmutation needs both
    // dual_substrate (chiral L/R substrates) and weak_transmutation on.
    // Prepared cohort; only the selected weak polarity-flip rule is active.
    // damping mirrors C++ B1 (bounds dual-substrate stress growth).
    's0-seed-beta-decay': isolatedScale0Profile('dual_substrate', 'weak_transmutation', 'damping'),

    // Moore Seeds — inert geometry inspection; every production phase off
    's0-seed-octahedron':          isolatedScale0Profile(),
    's0-seed-cuboctahedron':       isolatedScale0Profile(),
    's0-seed-stella-octangula':    isolatedScale0Profile(),
    's0-seed-moore-cell':          isolatedScale0Profile(),
    's0-seed-moore-decomposition': isolatedScale0Profile(),
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
    's0-seed-cluster-law-subknee': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-cluster-law-knee': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-cluster-law-superknee': isolatedScale0Profile(
        'wave_propagation', 'genesis', 'gauss_projection'),
    's0-seed-de-broglie-clock': isolatedScale0Profile(
        'wave_propagation', 'de_broglie_clock'),

    // Native neutral wave candidates. These are propagation experiments, so
    // every non-wave production phase is disabled explicitly in both the
    // WASM and JS fallback paths.
    's0-vacuum-photon': isolatedScale0Profile('wave_propagation', 'gauss_projection'),
    's0-vacuum-electron-neutrino': isolatedScale0Profile('wave_propagation', 'gauss_projection'),
    's0-vacuum-muon-neutrino': isolatedScale0Profile('wave_propagation', 'gauss_projection'),
    's0-vacuum-tau-neutrino': isolatedScale0Profile('wave_propagation', 'gauss_projection'),
    's0-vacuum-electron-antineutrino': isolatedScale0Profile('wave_propagation', 'gauss_projection'),
    's0-vacuum-muon-antineutrino': isolatedScale0Profile('wave_propagation', 'gauss_projection'),
    's0-vacuum-tau-antineutrino': isolatedScale0Profile('wave_propagation', 'gauss_projection'),

    // Light family — same isolated wave+Gauss profile (also used by getScale0ScenarioToggleProfile).
    'light-rainbow': isolatedScale0Profile('wave_propagation', 'gauss_projection'),
    'light-dipole': isolatedScale0Profile('wave_propagation', 'gauss_projection'),
    'light-two-slit': isolatedScale0Profile('wave_propagation', 'gauss_projection'),
    'light-photon-race': isolatedScale0Profile('wave_propagation', 'gauss_projection'),
};

// Research / non-UI terms pinned per scenario. Keys here are NOT in
// SCALE0_TOGGLES (no checkbox). The loader applies them after the whitelist
// reset; C++ configure_* helpers remain authoritative for the engine body.
//
// @type {Partial<Record<ScenarioId, Record<string, boolean>>>}
export const SCALE0_SCENARIO_RESEARCH_TERMS = {
    'flux-pair-production': { pair_production: true },
    's0-seed-ew-phase-transition': { ew_background_sweep: true },
    's0-seed-quark-gluon-plasma': { langevin: true },
    's0-field-thomson-unlocked-recoil': { emergent_forces: true },
};

// ── Per-scenario BOUNDARY preference ────────────────────────────────
// Mirrors SCALE0_SCENARIO_OVERRIDES (toggle defaults), but for the lattice
// boundary. A scenario declares an entry here when its physics needs a
// specific boundary. The loader applies these at load AND on resize.
// Missing entry → dispersal (mode 2), NOT the live DOM boundary controls.
// Any configure_* / body that sets Periodic MUST register mode: 0 here or
// applyAuxiliaryDefaults will sponge the seed after load.
//
//   mode:       number  → 0 periodic, 1 reflective, 2 dispersal.
//   reflective: boolean → legacy alias for mode 1/2.
//   shape:      string  → optional 'boundary-select' value (e.g. 'cube').
//
// @type {Partial<Record<ScenarioId, { mode?: number, reflective?: boolean, shape?: string }>>}
export const SCALE0_SCENARIO_BOUNDARY = {
    // configure_free_wave_terms leaves the freshly constructed engine's
    // canonical Periodic boundary intact. The dashboard fallback used to
    // overwrite flux-pulse with Dispersal immediately after setup.
    'flux-pulse': { mode: 0 },
    'flux-zero-point': { mode: 0 },
    's0-seed-dynamical-flux-dressing': { mode: 0 },
    's0-seed-moving-source-reciprocity': { mode: 0 },

    // These initial conditions are uniform across at least one transverse
    // face or are defined as periodic harmonics. A dispersal sponge edits the
    // seed at tick one and injects a false longitudinal/transverse distortion.
    'flux-standing': { mode: 0 },
    'flux-dipole': { mode: 0 },
    'flux-soliton': { mode: 0 },
    'flux-dual-substrate': { mode: 0 },
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
    // configure_emergent_recoil_terms pins Periodic; without this entry
    // applyAuxiliaryDefaults would clobber it back to dispersal (mode 2).
    's0-field-thomson-unlocked-recoil': { mode: 0 },
    's0-field-thomson-scattering': { mode: 0 },
    's0-field-spacetime-forcing-boundary': { mode: 0 },
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
    's0-seed-cluster-law-subknee': { mode: 0 },
    's0-seed-cluster-law-knee': { mode: 0 },
    's0-seed-cluster-law-superknee': { mode: 0 },
    's0-seed-de-broglie-clock': { mode: 0 },
    // Weak probe leaves flux_boundary at TermToggles default (Periodic);
    // pin so applyAuxiliaryDefaults cannot sponge the stress packet.
    's0-seed-beta-decay': { mode: 0 },
    'empty': { mode: 0 },
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
 * Shared by load-time UI sync and the "profile modified?" check. Prefer full
 * isolatedScale0Profile rows in SCALE0_SCENARIO_OVERRIDES. When no override is
 * registered, return an all-off isolation profile (never silent dashboard
 * defaults that can re-arm disabled terms).
 *
 * @param {ScenarioId|string} scenarioId
 * @returns {Array<[string, boolean, string]>}
 */
export function getScale0ScenarioToggleProfile(scenarioId) {
    const ov = SCALE0_SCENARIO_OVERRIDES[scenarioId];
    if (ov) return ov.map(([key, value, elementId]) => [key, !!value, elementId]);
    if (String(scenarioId).startsWith('light-')) {
        return isolatedScale0Profile('wave_propagation', 'gauss_projection');
    }
    console.warn(
        '[toggles] no SCALE0_SCENARIO_OVERRIDES for ' + scenarioId + '; using isolated-off profile',
    );
    return isolatedScale0Profile();
}
