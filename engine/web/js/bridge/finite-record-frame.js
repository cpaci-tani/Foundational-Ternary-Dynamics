/** Passive transposition from x-major finite records to the web lattice ABI.
 * The normal volume is z-major. Manifestation remains the lagged s readout,
 * never a reconstruction from Q or a claim that tokens are particles/energy.
 */
export function recordFrame(view, quantity = 'tokens') {
    const L = Number(view.lattice_size), N = L ** 3;
    if (Number(view.width) !== 1 || !Number.isInteger(L) || L < 3
        || ['incidence', 'manifestation_counts', 'field_tokens', 'relation_tokens'].some(k => view[k]?.length !== N))
        throw new Error('Complete site observation required by the lattice renderer');
    if (!['tokens', 'field_tokens', 'relation_tokens', 'incidence'].includes(quantity)) throw new Error('Unknown record quantity');
    const volume = new Float32Array(N), positions = [], colors = [], states = [], allPositions = new Float32Array(N * 3);
    const stateValues = new Float32Array(N);
    for (let x = 0; x < L; x++) for (let y = 0; y < L; y++) for (let z = 0; z < L; z++) {
        const i = (x * L + y) * L + z, j = (z * L + y) * L + x;
        const s = Number(view.manifestation_counts[i][2]) - Number(view.manifestation_counts[i][0]);
        volume[j] = quantity === 'tokens' ? Number(view.field_tokens[i]) + Number(view.relation_tokens[i]) : Math.abs(Number(view[quantity][i]));
        allPositions.set([x + .5, y + .5, z + .5], i * 3); stateValues[i] = s;
        if (!s) continue;
        positions.push(x + .5, y + .5, z + .5); states.push(s);
        colors.push(...(s > 0 ? [.2, .95, .45] : [.95, .25, .25]));
    }
    return {volume, particles: {positions: new Float32Array(positions), colors: new Float32Array(colors),
        states: new Int8Array(states), sizes: new Float32Array(states.length).fill(3), count: states.length},
        state: {positions: allPositions, values: stateValues, count: N}};
}

export function recordAt(view, x, y, z) {
    const L = Number(view?.lattice_size);
    if (![x, y, z].every(n => Number.isInteger(n) && n >= 0 && n < L)) return null;
    const i = (x * L + y) * L + z;
    return {state: Number(view.manifestation_counts[i][2]) - Number(view.manifestation_counts[i][0]),
        incidence: Number(view.incidence[i]), fieldTokens: Number(view.field_tokens[i]),
        relationTokens: Number(view.relation_tokens[i]), tick: view.microtick};
}
