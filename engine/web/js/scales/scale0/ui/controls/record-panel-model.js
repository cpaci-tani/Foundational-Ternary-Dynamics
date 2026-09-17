/** Passive observations only. No effective-field units or closure assumptions. */
export const RECORD_PANEL_CONTRACTS = Object.freeze({
    controls: 'existing', inspector: 'existing', scene: 'existing', seeding: 'existing',
    diagnostics: 'counts', 'telemetry-grid': 'counts', charts: 'history',
    time: 'clock', thermo: 'occupancy', 'flux-slice': 'slice', 'scale-context': 'geometry',
    transactions: 'No transaction journal is exported by this runtime. Observed counts cannot reconstruct intermediate events or lifetimes.',
    fluid: 'Token counts do not define fluid density, velocity, pressure or transport coefficients. Fluid recovery remains open.',
    lagrangian: 'No action or Lagrangian observable is defined for this finite-record law.',
    'wave-lab': 'No calibrated wave amplitude, frequency or physical propagation speed is exported by this runtime.',
    'p1-observables': 'Particle, Coulomb, Bell and spectroscopy instruments require recovery contracts not supplied by these preparations.',
    spectrum: 'No spectral observable is exported. Token counts are not effective field amplitudes or mode energies.',
    dispersion: 'No dispersion relation is established by these preparations. The analytical thermal gate remains separate evidence.',
    knots: 'No knot identity or trajectory is exported; stored manifestation does not establish a persistent particle identity.',
    gravity: 'No gravitational field, potential or force identification is defined for these records.',
});
export const RECORD_PHASES = ['Admission', 'Collision / relations', 'Streaming', 'Manifestation'];
const sum = values => values.reduce((total, value) => total + BigInt(value), 0n).toString();
export function summarizeRecordObservation(view) {
    const states = [0, 1, 2].map(index => sum(view.manifestation_counts.map(row => row[index])));
    return Object.freeze({tick: String(view.microtick), phase: RECORD_PHASES[Number(view.phase)],
        field: sum(view.field_tokens), relation: sum(view.relation_tokens), incidence: sum(view.incidence),
        negative: states[0], zero: states[1], positive: states[2], size: Number(view.lattice_size)});
}
export function recordSlice(view, quantity, z = Math.floor(Number(view.lattice_size) / 2)) {
    const n = Number(view.lattice_size);
    if (!Number.isInteger(z) || z < 0 || z >= n) throw new RangeError('Invalid slice');
    return Array.from({length: n}, (_, y) => Array.from({length: n}, (_, x) => {
        const i = (x * n + y) * n + z;
        if (quantity === 'tokens') return (BigInt(view.field_tokens[i]) + BigInt(view.relation_tokens[i])).toString();
        if (quantity === 'incidence') return String(view.incidence[i]);
        if (quantity === 'field_tokens' || quantity === 'relation_tokens') return String(view[quantity][i]);
        throw new RangeError('Unsupported record quantity');
    }));
}
export function appendRecordSample(history, sample, limit = 128) {
    if (history.at(-1)?.tick === sample.tick) return false;
    history.push(sample);
    if (history.length > limit) history.splice(0, history.length - limit);
    return true;
}
