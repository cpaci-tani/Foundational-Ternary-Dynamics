import {test} from 'node:test';
import assert from 'node:assert/strict';
import {buildDenseGrid, buildPersistentIndex, computeStreamlines} from '../js/fieldlines.js';

const N = 33, STRIDE = 2, C = 16;
function radialSample() {
    const pos = [], vec = [];
    for (let z = 0; z < N; z += STRIDE) for (let y = 0; y < N; y += STRIDE) for (let x = 0; x < N; x += STRIDE) {
        const dx = x - C, dy = y - C, dz = z - C, r = Math.hypot(dx, dy, dz);
        pos.push(x, y, z);
        if (r < 1e-9) vec.push(0, 0, 0); else { const a = 1 / (r * r * r); vec.push(a * dx, a * dy, a * dz); }
    }
    return {positions: new Float32Array(pos), vectors: new Float32Array(vec), count: pos.length / 3};
}
test('a regular stride grid yields a dense view', () => {
    const s = radialSample();
    const d = buildDenseGrid(s.positions, s.vectors, s.count, N, STRIDE);
    assert.ok(d, 'dense view expected');
    assert.equal(d.dimX, 17); assert.equal(d.dimY, 17); assert.equal(d.dimZ, 17);
    assert.equal(d.ox, 0); assert.equal(d.stride, STRIDE);
});
test('an off-grid sample yields no dense view (nearest fallback stays)', () => {
    const s = radialSample(); s.positions[3] += 0.25;
    assert.equal(buildDenseGrid(s.positions, s.vectors, s.count, N, STRIDE), null);
});
test('trilinear lookup is exact at grid points and averages at midpoints', async () => {
    const mod = await import('../js/fieldlines.js');
    const s = radialSample();
    const index = buildPersistentIndex(s.positions, s.vectors, s.count, N, STRIDE);
    assert.ok(index.dense);
    const at = (x, y, z) => { mod.lookupFieldTrilinearInto(index.dense, x, y, z); return mod.__scratchField(); };
    const g = at(20, 16, 16), i = 3 * ((20 / STRIDE) + 17 * ((16 / STRIDE) + 17 * (16 / STRIDE)));
    assert.ok(Math.abs(g[0] - s.vectors[i]) < 1e-7 && Math.abs(g[1] - s.vectors[i + 1]) < 1e-7);
    const m = at(21, 16, 16), i2 = i + 3;
    assert.ok(Math.abs(m[0] - 0.5 * (s.vectors[i] + s.vectors[i2])) < 1e-7);
});
test('streamlines on an exactly radial field stay radial', () => {
    const s = radialSample();
    const r = computeStreamlines(s, [[20, 16, 16]], {N, stride: STRIDE, stepSize: 0.5, maxSteps: 40, bidirectional: false, maxLines: 1});
    assert.equal(r.count, 1);
    const devs = [];
    for (let k = r.offsets[0] + 3; k < r.offsets[0] + r.lengths[0]; k += 3) {
        const d = [r.buffer[k] - r.buffer[k - 3], r.buffer[k + 1] - r.buffer[k - 2], r.buffer[k + 2] - r.buffer[k - 1]];
        const m = [(r.buffer[k] + r.buffer[k - 3]) / 2 - C, (r.buffer[k + 1] + r.buffer[k - 2]) / 2 - C, (r.buffer[k + 2] + r.buffer[k - 1]) / 2 - C];
        const dn = Math.hypot(...d), mn = Math.hypot(...m); if (dn < 1e-9 || mn < 4) continue;
        devs.push(Math.acos(Math.min(1, Math.abs((d[0]*m[0]+d[1]*m[1]+d[2]*m[2]) / (dn*mn)))) * 180 / Math.PI);
    }
    devs.sort((a, b) => a - b);
    assert.ok(devs.length >= 10, 'need segments');
    assert.ok(devs[Math.floor(devs.length / 2)] < 1.5, 'median deviation < 1.5 deg, got ' + devs[Math.floor(devs.length / 2)]);
    assert.ok(devs[Math.floor(devs.length * 0.9)] < 4.0, 'p90 deviation < 4 deg');
});
