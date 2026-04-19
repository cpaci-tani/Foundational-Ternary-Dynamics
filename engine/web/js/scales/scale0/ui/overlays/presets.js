/**
 * Scale 0 Overlay Presets
 *
 * A preset is a named bundle of overlay-toggle states. Applying a preset
 * flips every relevant overlay on or off in one step, then leaves
 * everything else alone.
 *
 * The DOM button IDs are the source of truth — the preset dictionaries
 * just list which buttons should be ACTIVE under that preset. Anything
 * not listed is turned OFF. This makes adding a new overlay straight-
 * forward: if it doesn't show up in any preset, it's off-by-default in
 * every preset except 'custom'.
 *
 * 'custom' is a sentinel: selecting it never touches any toggle and just
 * marks the dropdown so the user can collect their own selection without
 * auto-triggering a preset.
 */

// Every button that the preset system manages. The preset application
// loop flips each of these off unless listed in `ON` for the active
// preset. Kept sorted by the panel column so it reads like the UI.
export const MANAGED_TOGGLES = [
    // Volume
    'toggle-flux-volume', 'toggle-flux-slice', 'toggle-flux-lines', 'toggle-div-field',
    // Fields
    'toggle-e-field', 'toggle-b-field', 'toggle-poynting', 'toggle-light',
    // Forces
    'toggle-force-em', 'toggle-force-gravity', 'toggle-force-strong', 'toggle-force-weak',
    // Quantum
    'toggle-psi-squared', 'toggle-phase', 'toggle-lagrangian-density', 'toggle-entropy-density',
    // Topology
    'toggle-grav-potential', 'toggle-em-energy', 'toggle-charge-density',
    'toggle-vorticity', 'toggle-helicity', 'toggle-kretschmann',
    // Stress-energy
    'toggle-e-pressure', 'toggle-b-pressure', 'toggle-kinetic-energy', 'toggle-fisher',
    // Phenomena
    'toggle-dual-substrate', 'toggle-chirality', 'toggle-dark-halo',
    'toggle-genesis-iso', 'toggle-damping-zones', 'toggle-confinement',
    'toggle-horizon', 'toggle-coherence',
];

// Preset definitions — map of preset id → list of toggle ids that should
// be ON under that preset. Any toggle in MANAGED_TOGGLES not listed is
// turned OFF when the preset is applied.
export const OVERLAY_PRESETS = {
    // Just the flux substrate — the cleanest view for introducing a user
    // to Scale 0. Shows what's there without interpretation.
    'clean': [
        'toggle-flux-volume',
    ],
    // Classical electromagnetism focus: E, B, and Poynting vector.
    'em': [
        'toggle-flux-volume',
        'toggle-e-field',
        'toggle-b-field',
        'toggle-poynting',
    ],
    // Quantum-mechanics readouts: |psi|^2, phase (requires dual substrate),
    // Lagrangian. Turns on dual substrate as a dependency for phase.
    'quantum': [
        'toggle-flux-volume',
        'toggle-psi-squared',
        'toggle-lagrangian-density',
        'toggle-dual-substrate',
        'toggle-phase',
    ],
    // Rubber-sheet physics: potential landscapes + conservation laws that
    // read as height fields.
    'topology': [
        'toggle-flux-volume',
        'toggle-grav-potential',
        'toggle-em-energy',
        'toggle-charge-density',
        'toggle-vorticity',
    ],
    // Stress-energy tensor components — the classical energy / pressure
    // decomposition, plus Fisher information as an information-theoretic
    // sister field.
    'stress-energy': [
        'toggle-flux-volume',
        'toggle-e-pressure',
        'toggle-b-pressure',
        'toggle-kinetic-energy',
        'toggle-fisher',
    ],
    // Everything — use with care. Expect frame-rate drops at N > 32 when
    // many scenarios are active. Useful for one-off "what's in here"
    // screenshots.
    'full': MANAGED_TOGGLES.slice(),
    // All off — bare viewport. Sometimes useful for performance profiling
    // or to reset before building a custom selection.
    'off': [],
};

export const COL_TO_TOGGLES = {
    'volume':        ['toggle-flux-volume', 'toggle-flux-slice', 'toggle-flux-lines', 'toggle-div-field'],
    'fields':        ['toggle-e-field', 'toggle-b-field', 'toggle-poynting', 'toggle-light'],
    'forces':        ['toggle-force-em', 'toggle-force-gravity', 'toggle-force-strong', 'toggle-force-weak'],
    'quantum':       ['toggle-psi-squared', 'toggle-phase', 'toggle-lagrangian-density', 'toggle-entropy-density'],
    'topology':      ['toggle-grav-potential', 'toggle-em-energy', 'toggle-charge-density',
                      'toggle-vorticity', 'toggle-helicity', 'toggle-kretschmann'],
    'stress-energy': ['toggle-e-pressure', 'toggle-b-pressure', 'toggle-kinetic-energy', 'toggle-fisher'],
    'phenomena':     ['toggle-dual-substrate', 'toggle-chirality', 'toggle-dark-halo',
                      'toggle-genesis-iso', 'toggle-damping-zones', 'toggle-confinement',
                      'toggle-horizon', 'toggle-coherence'],
};
