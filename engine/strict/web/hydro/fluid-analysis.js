/** Approximate display arithmetic on exact integers. No time evolution or closure. */
const pairs = [[0, 0], [1, 1], [2, 2], [0, 1], [0, 2], [1, 2]];
const integer = value => {
    if (typeof value !== 'string' || !/^(0|-?[1-9][0-9]*)$/.test(value)) throw new Error('Expected an exact decimal integer');
    const n = Number(value);
    if (!Number.isSafeInteger(n)) throw new Error('Observation exceeds display precision');
    return n;
};
export function analyzeFluid(snapshot, polarity = '0') {
    if (!['0', '1', 'both'].includes(polarity)) throw new Error('Invalid polarity');
    if (snapshot.schema !== 'ftd-hydro-fluid-snapshot-v1' || snapshot.observation_phase !== 'snapshot'
        || snapshot.second_moment_order.join(',') !== 'xx,yy,zz,xy,xz,yz') throw new Error('Unsupported fluid snapshot');
    const L = integer(snapshot.L), side = integer(snapshot.side), width = integer(snapshot.width), volume = integer(snapshot.block_sites);
    if (L < 3 || L > 32 || side < 1 || side > 32 || width < 1 || side * width !== L
        || volume !== width ** 3 || snapshot.blocks.length !== side ** 3) throw new Error('Invalid block support');
    const pols = polarity === 'both' ? [0, 1] : [Number(polarity)];
    const blocks = snapshot.blocks.map(b => {
        const N = pols.reduce((s, p) => s + integer(b.field_tokens[p]), 0);
        const P = [0, 1, 2].map(j => pols.reduce((s, p) => s + integer(b.momentum[p][j]), 0));
        const M2 = pairs.map((_, j) => pols.reduce((s, p) => s + integer(b.second_moment[p][j]), 0));
        if (N < 0 || N > 48 * volume) throw new Error('Invalid token density');
        if (P.some(v => Math.abs(v) > N) || M2.some(v => Math.abs(v) > N)
            || M2.slice(0, 3).some((v, i) => v < 0 || N * v < P[i] ** 2)
            || (!N && [...P, ...M2].some(v => v !== 0))) throw new Error('Inconsistent raw moments');
        const velocity = N ? P.map(v => v / N) : null;
        const stress = pairs.map(([i, j], k) => N ? (N * M2[k] - P[i] * P[j]) / (volume * N) : 0);
        const pressure = (stress[0] + stress[1] + stress[2]) / 3;
        const dev = stress.map((v, i) => i < 3 ? v - pressure : v);
        const devNorm = Math.sqrt(dev.slice(0, 3).reduce((s, v) => s + v * v, 0) + 2 * dev.slice(3).reduce((s, v) => s + v * v, 0));
        return { N, P, M2, density: N / volume, velocity, stress, pressure, devNorm, gradient: null, curl: null, divergence: null, strainNorm: null, advection: null };
    });
    const index = (x, y, z) => (((x + side) % side) * side + (y + side) % side) * side + (z + side) % side;
    for (let x = 0; x < side; x++) for (let y = 0; y < side; y++) for (let z = 0; z < side; z++) {
        const b = blocks[index(x, y, z)];
        if (!b.velocity || side < 3) continue; // Opposite neighbors coincide for side <=2.
        const gradient = Array.from({ length: 3 }, () => [0, 0, 0]);
        let defined = true;
        for (let j = 0; j < 3; j++) {
            const lo = [x, y, z], hi = [x, y, z]; lo[j]--; hi[j]++;
            const ul = blocks[index(...lo)].velocity, uh = blocks[index(...hi)].velocity;
            if (!ul || !uh) { defined = false; break; }
            for (let i = 0; i < 3; i++) gradient[i][j] = (uh[i] - ul[i]) / (2 * width);
        }
        if (!defined) continue;
        b.gradient = gradient;
        b.divergence = gradient[0][0] + gradient[1][1] + gradient[2][2];
        b.strainNorm = Math.sqrt(gradient.reduce((sum, row, i) => sum + row.reduce((s, v, j) =>
            s + ((v + gradient[j][i]) / 2) ** 2, 0), 0));
        b.curl = [gradient[2][1] - gradient[1][2], gradient[0][2] - gradient[2][0], gradient[1][0] - gradient[0][1]];
        b.advection = gradient.map(row => row.reduce((s, v, j) => s + v * b.velocity[j], 0));
    }
    const total = blocks.reduce((s, b) => s + b.N, 0);
    return { blocks, side, width, polarity, total,
        momentum: [0, 1, 2].map(j => blocks.reduce((s, b) => s + b.P[j], 0)),
        rmsSpeed: total ? Math.sqrt(blocks.reduce((s, b) => s + b.N * (b.velocity?.reduce((t, v) => t + v * v, 0) || 0), 0) / total) : null,
        pressure: blocks.reduce((s, b) => s + b.pressure, 0) / blocks.length,
        stressNorm: blocks.reduce((s, b) => s + b.devNorm, 0) / blocks.length,
        derivativeBlocks: blocks.filter(b => b.gradient).length };
}

/** Finite-wave empirical slope; correlated samples supply no certified CI. */
export function fitMode(history, kSquared, eligible) {
    if (!eligible) return { available: false, reason: 'Only a single transverse shear preparation has this diagnostic.' };
    const samples = history.slice(-32);
    if (samples.length < 8) return { available: false, reason: 'At least 8 distinct cycle observations are needed.' };
    if (!(kSquared > 0) || !Number.isFinite(kSquared)
        || samples.some((s, i) => !Number.isFinite(s.cycle) || !(s.amplitude > 0) || !Number.isFinite(s.amplitude)
            || (i && s.cycle <= samples[i - 1].cycle))) return { available: false, reason: 'Zero, missing or unordered amplitude samples.' };
    const n = samples.length, times = samples.map(v => v.cycle - samples[0].cycle);
    const mx = times.reduce((s, v) => s + v, 0) / n;
    const logs = samples.map(s => Math.log(s.amplitude)), my = logs.reduce((s, v) => s + v, 0) / n;
    const xx = times.reduce((s, v) => s + (v - mx) ** 2, 0);
    if (!(xx > 0) || !Number.isFinite(xx)) return { available: false, reason: 'Regression time precision is unresolved.' };
    const slope = times.reduce((s, v, i) => s + (v - mx) * (logs[i] - my), 0) / xx;
    const residual = times.reduce((s, v, i) => s + (logs[i] - my - slope * (v - mx)) ** 2, 0);
    const yy = logs.reduce((s, v) => s + (v - my) ** 2, 0);
    if (![slope, -slope / kSquared, residual, yy, yy > 0 ? 1 - residual / yy : 0].every(Number.isFinite)) {
        return { available: false, reason: 'Regression exceeds finite display arithmetic.' };
    }
    return { available: true, gamma: -slope, diffusivity: -slope / kSquared,
        r2: yy > 0 ? 1 - residual / yy : null, logRms: Math.sqrt(residual / n), samples: n,
        from: samples[0].cycle, to: samples.at(-1).cycle, uncertainty: 'not certified' };
}
