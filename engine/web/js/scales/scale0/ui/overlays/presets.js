/**
 * Scale 0 Overlay column groupings.
 *
 * Maps each column name in the Scale 0 overlay panel to its list of
 * toggle button IDs. Used by the per-column clear (×) buttons and the
 * live overlay-count badges in the column headers.
 */

export const COL_TO_TOGGLES = {
    'volume':        ['toggle-flux-volume', 'toggle-flux-slice', 'toggle-flux-lines', 'toggle-div-field', 'toggle-state-field', 'toggle-moore-decomp'],
    'fields':        ['toggle-e-field', 'toggle-b-field', 'toggle-poynting', 'toggle-light'],
    'forces':        ['toggle-force-em', 'toggle-force-gravity', 'toggle-force-strong', 'toggle-force-weak'],
    'quantum':       ['toggle-psi-squared', 'toggle-phase', 'toggle-lagrangian-density', 'toggle-entropy-density'],
    'topology':      ['toggle-grav-potential', 'toggle-em-energy', 'toggle-charge-density',
                      'toggle-vorticity', 'toggle-helicity', 'toggle-kretschmann',
                      'toggle-latency', 'toggle-gauss-residual'],
    'stress-energy': ['toggle-e-pressure', 'toggle-b-pressure', 'toggle-kinetic-energy', 'toggle-fisher'],
    'phenomena':     ['toggle-dual-substrate', 'toggle-chirality', 'toggle-dark-halo',
                      'toggle-genesis-iso', 'toggle-damping-zones', 'toggle-confinement',
                      'toggle-horizon', 'toggle-coherence'],
};
