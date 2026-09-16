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
test('a sparse cell-centred sample keeps a dense view with zeros in omitted cells', async () => {
    const mod = await import('../js/fieldlines.js');
    // Sampler contract: anchors 0,2,...,32; positions are anchor + 0.5; zero-field cells omitted.
    const Cc = 16.5;                                   // a cell centre, so an axis passes through centres
    const pos = [], vec = [];
    for (let z = 0; z < N; z += STRIDE) for (let y = 0; y < N; y += STRIDE) for (let x = 0; x < N; x += STRIDE) {
        const cx = x + 0.5, cy = y + 0.5, cz = z + 0.5;
        const dx = cx - Cc, dy = cy - Cc, dz = cz - Cc, r = Math.hypot(dx, dy, dz);
        if (r > 8) continue;                           // omitted like a zero-field block
        const a = 1 / (r * r * r);
        pos.push(cx, cy, cz); vec.push(a * dx, a * dy, a * dz);
    }
    const s = {positions: new Float32Array(pos), vectors: new Float32Array(vec), count: pos.length / 3};
    assert.ok(s.count < 17 * 17 * 17, 'test premise: the sample is sparse');
    const d = mod.buildDenseGrid(s.positions, s.vectors, s.count, N, STRIDE);
    assert.ok(d, 'a sparse sample must still yield a dense view');
    assert.deepEqual([d.dimX, d.dimY, d.dimZ], [17, 17, 17]);
    assert.deepEqual([d.ox, d.oy, d.oz], [0.5, 0.5, 0.5]);
    mod.lookupFieldTrilinearInto(d, 0.5, 0.5, 0.5);                 // an omitted corner cell
    assert.deepEqual(mod.__scratchField(), [0, 0, 0]);
    mod.lookupFieldTrilinearInto(d, 20.5, 16.5, 16.5);              // kept cell at offset (4,0,0): field (1/16, 0, 0)
    const f = mod.__scratchField();
    assert.ok(Math.abs(f[0] - 1 / 16) < 1e-6 && Math.abs(f[1]) < 1e-9 && Math.abs(f[2]) < 1e-9, 'kept cell returns its sample: ' + f);
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
function radialDeviationsDeg(result, centre) {
    const devs = [];
    for (let k = result.offsets[0] + 3; k < result.offsets[0] + result.lengths[0]; k += 3) {
        const b = result.buffer;
        const d = [b[k] - b[k - 3], b[k + 1] - b[k - 2], b[k + 2] - b[k - 1]];
        const m = [(b[k] + b[k - 3]) / 2 - centre, (b[k + 1] + b[k - 2]) / 2 - centre, (b[k + 2] + b[k - 1]) / 2 - centre];
        const dn = Math.hypot(...d), mn = Math.hypot(...m); if (dn < 1e-9 || mn < 4) continue;
        devs.push(Math.acos(Math.min(1, Math.abs((d[0]*m[0] + d[1]*m[1] + d[2]*m[2]) / (dn * mn)))) * 180 / Math.PI);
    }
    devs.sort((a, b) => a - b);
    return { devs, median: devs[Math.floor(devs.length / 2)], p90: devs[Math.floor(devs.length * 0.9)] };
}

test('streamlines along a symmetry axis stay exactly radial', () => {
    const s = radialSample();
    const r = computeStreamlines(s, [[20, 16, 16]], {N, stride: STRIDE, stepSize: 0.5, maxSteps: 40, bidirectional: false, maxLines: 1});
    assert.equal(r.count, 1);
    const {devs, median, p90} = radialDeviationsDeg(r, C);
    assert.ok(devs.length >= 10, 'need segments');
    assert.ok(median < 1.5, 'median deviation < 1.5 deg, got ' + median);
    assert.ok(p90 < 4.0, 'p90 deviation < 4 deg');
});

test('streamlines from an off-axis seed stay radial (interpolation exercised)', () => {
    const s = radialSample();
    const r = computeStreamlines(s, [[21, 18, 17]], {N, stride: STRIDE, stepSize: 0.5, maxSteps: 40, bidirectional: false, maxLines: 1});
    assert.equal(r.count, 1);
    const {devs, median, p90} = radialDeviationsDeg(r, C);
    assert.ok(devs.length >= 10, 'need segments');
    assert.ok(median < 1.5, `median deviation ${median} deg`);
    assert.ok(p90 < 4.0, `p90 deviation ${p90} deg`);
});

test('trilinear lookup reproduces a trilinear field when all three weights are fractional', async () => {
    const mod = await import('../js/fieldlines.js');
    // In node-index coordinates (u,v,w) trilinear interpolation is exact on the
    // span {1,u,v,w,uv,vw,uw,uvw}; each component uses the mixed terms so a
    // weight taken from the wrong axis cannot cancel out.
    const field = (u, v, w) => [1 + 0.5 * u + 0.25 * v + 0.125 * w + 0.01 * u * v * w,
                                0.3 * u * v - 0.2 * w, 0.05 * v * w + 0.4 * u];
    const pos = [], vec = [];
    for (let z = 0; z < N; z += STRIDE) for (let y = 0; y < N; y += STRIDE) for (let x = 0; x < N; x += STRIDE) {
        pos.push(x, y, z); vec.push(...field(x / STRIDE, y / STRIDE, z / STRIDE));
    }
    const d = buildDenseGrid(new Float32Array(pos), new Float32Array(vec), pos.length / 3, N, STRIDE);
    assert.ok(d);
    const [px, py, pz] = [21.3, 17.7, 15.1];                         // t = 0.65, 0.85, 0.55
    mod.lookupFieldTrilinearInto(d, px, py, pz);
    const got = mod.__scratchField(), want = field(px / STRIDE, py / STRIDE, pz / STRIDE);
    for (let c = 0; c < 3; c++) assert.ok(Math.abs(got[c] - want[c]) < 1e-4, `component ${c}: ${got[c]} vs ${want[c]}`);
});

test('trilinear lookup at a non-finite coordinate returns a non-finite field, not an edge node', async () => {
    const mod = await import('../js/fieldlines.js');
    const s = radialSample();
    const d = buildDenseGrid(s.positions, s.vectors, s.count, N, STRIDE);
    for (const bad of [NaN, Infinity, -Infinity]) {
        mod.lookupFieldTrilinearInto(d, bad, 16, 16);
        assert.ok(mod.__scratchField().every(Number.isNaN), `x=${bad} gave ${mod.__scratchField()}`);
    }
});

test('a streamline entering omitted cells stops on the field it integrates', async () => {
    const mod = await import('../js/fieldlines.js');
    // Sampler contract: an omitted cell is a zero-field cell. Uniform +x field on
    // cell centres, omitted for every anchor x >= 16 (centres 16.5, 18.5, ...).
    const pos = [], vec = [];
    for (let z = 0; z < N; z += STRIDE) for (let y = 0; y < N; y += STRIDE) for (let x = 0; x < 16; x += STRIDE) {
        pos.push(x + 0.5, y + 0.5, z + 0.5); vec.push(1, 0, 0);
    }
    const s = {positions: new Float32Array(pos), vectors: new Float32Array(vec), count: pos.length / 3};
    const d = buildDenseGrid(s.positions, s.vectors, s.count, N, STRIDE);
    assert.ok(d);
    const h = 0.5;
    const r = computeStreamlines(s, [[8.5, 16.5, 16.5]], {N, stride: STRIDE, stepSize: h, maxSteps: 60, bidirectional: false, maxLines: 1});
    assert.equal(r.count, 1);
    const b = r.buffer, o = r.offsets[0], len = r.lengths[0];
    for (let k = 0; k + 3 < len; k += 3) {                          // every vertex a step was taken from
        mod.lookupFieldTrilinearInto(d, b[o + k], b[o + k + 1], b[o + k + 2]);
        assert.ok(Math.hypot(...mod.__scratchField()) > 0, `step taken from zero interpolated field at x=${b[o + k]}`);
    }
    const lastX = b[o + len - 3];
    assert.ok(lastX >= 14.5, `line stopped before leaving the measured region: x=${lastX}`);
    assert.ok(lastX <= 16.5 + h, `line ran into the zero-field region to x=${lastX}`);
});
