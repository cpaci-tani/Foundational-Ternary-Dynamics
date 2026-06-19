// engine/web/tests/field-line-knots-contributions.spec.js
// Per-knot scientific contributions: energy / flux / charge integrated over each
// knot's region, expressed as a share of the scenario total, + history.
import { test, expect } from '@playwright/test';
import { FieldLineKnotTracker } from '../js/scales/scale0/runtime/field-line-knots.js';

function makeStreamlines(lines) {
    let total = 0;
    for (const l of lines) total += l.length * 3;
    const buffer = new Float32Array(total);
    const offsets = new Int32Array(lines.length);
    const lengths = new Int32Array(lines.length);
    let o = 0;
    for (let i = 0; i < lines.length; i++) {
        offsets[i] = o; lengths[i] = lines[i].length * 3;
        for (const v of lines[i]) { buffer[o++] = v[0]; buffer[o++] = v[1]; buffer[o++] = v[2]; }
    }
    return { count: lines.length, buffer, offsets, lengths };
}
function clump(c) {
    const [x, y, z] = c;
    return [[[x - 2, y, z], [x + 2, y, z]], [[x, y - 2, z], [x, y + 2, z]], [[x, y, z - 2], [x, y, z + 2]]];
}
const opt = { cellSize: 2, densityThreshold: 2, minCellsPerKnot: 1, crossingDist: 2.0 };

// Two single-cell knots at (5,5,5) and (25,25,25), each box = centroid ± 1.
function twoKnots() {
    const tr = new FieldLineKnotTracker(opt);
    tr.record(makeStreamlines([...clump([5, 5, 5]), ...clump([25, 25, 25])]), null, 0, 33);
    const z = tr.getKnotZones();
    expect(z.count).toBe(2);
    return { tr, z };
}

test('energy: per-knot ½|E|² over the box + scenario fractions', () => {
    const { tr, z } = twoKnots();
    const c0 = [z.centroids[0], z.centroids[1], z.centroids[2]];
    const c1 = [z.centroids[3], z.centroids[4], z.centroids[5]];
    // E samples: at knot0 centroid (|v|²/2 = 2), at knot1 centroid (2), one far outside (50, background)
    const eField = {
        count: 3,
        positions: new Float32Array([...c0, ...c1, 15, 15, 15]),
        vectors: new Float32Array([2, 0, 0, 0, 2, 0, 10, 0, 0]),
    };
    const c = tr.measureContributions({ eField, bField: null, fluxVolume: null, divJ: null, latticeSize: 33 });
    expect(c.count).toBe(2);
    expect(c.totals.energy).toBeCloseTo(54, 5);     // 2 + 2 + 50
    expect(c.energy[0]).toBeCloseTo(2, 5);
    expect(c.energy[1]).toBeCloseTo(2, 5);
    expect(c.energyFrac[0]).toBeCloseTo(2 / 54, 6);
    expect(c.captured.energyFrac).toBeCloseTo(4 / 54, 6);   // background not captured
    expect(c.energyFrac[0]).toBeGreaterThanOrEqual(0);
    expect(c.energyFrac[0]).toBeLessThanOrEqual(1);
});

test('flux: exact integral over the dense N³ volume; each voxel counted once', () => {
    const { tr } = twoKnots();
    const N = 33;
    const fluxVolume = new Float64Array(N * N * N).fill(1);   // |J|=1 everywhere
    const c = tr.measureContributions({ eField: null, bField: null, fluxVolume, divJ: null, latticeSize: N });
    expect(c.totals.flux).toBeCloseTo(N * N * N, 5);          // sum of all voxels
    // each box is centroid±1 → integer voxels {c-1,c,c+1}³ = 27, disjoint boxes
    expect(c.flux[0]).toBe(27);
    expect(c.flux[1]).toBe(27);
    expect(c.fluxFrac[0]).toBeCloseTo(27 / (N * N * N), 8);
    expect(c.captured.fluxFrac).toBeCloseTo(54 / (N * N * N), 8);
});

test('charge: per-knot |∇·J| over the box + fractions', () => {
    const { tr, z } = twoKnots();
    const c0 = [z.centroids[0], z.centroids[1], z.centroids[2]];
    const divJ = {
        count: 2,
        positions: new Float32Array([...c0, 15, 15, 15]),
        values: new Float32Array([3, 7]),                    // 3 inside knot0, 7 background
    };
    const c = tr.measureContributions({ eField: null, bField: null, fluxVolume: null, divJ, latticeSize: 33 });
    expect(c.totals.charge).toBeCloseTo(10, 5);
    expect(c.charge[0]).toBeCloseTo(3, 5);
    expect(c.chargeFrac[0]).toBeCloseTo(0.3, 6);
});

test('E and B both contribute to energy', () => {
    const { tr, z } = twoKnots();
    const c0 = [z.centroids[0], z.centroids[1], z.centroids[2]];
    const eField = { count: 1, positions: new Float32Array(c0), vectors: new Float32Array([2, 0, 0]) }; // ½·4=2
    const bField = { count: 1, positions: new Float32Array(c0), vectors: new Float32Array([0, 0, 2]) }; // ½·4=2
    const c = tr.measureContributions({ eField, bField, fluxVolume: null, divJ: null, latticeSize: 33 });
    expect(c.energy[0]).toBeCloseTo(4, 5);          // E + B
    expect(c.totals.energy).toBeCloseTo(4, 5);
});

test('history accumulates per knot and prunes the dead', () => {
    const tr = new FieldLineKnotTracker(opt);
    tr.record(makeStreamlines(clump([5, 5, 5])), null, 0, 33);
    const id = tr.getTelemetry().ids[0];
    const c0 = [tr.getKnotZones().centroids[0], tr.getKnotZones().centroids[1], tr.getKnotZones().centroids[2]];
    const eField = { count: 1, positions: new Float32Array(c0), vectors: new Float32Array([1, 0, 0]) };
    tr.measureContributions({ eField, latticeSize: 33 });
    tr.measureContributions({ eField, latticeSize: 33 });
    const h = tr.getKnotHistory(id);
    expect(h.n).toBe(2);
    expect(h.energyFrac.length).toBe(2);
    expect(h.energyFrac[0]).toBeCloseTo(1, 6);      // the only sample → 100% of energy
    // knot dies (empty record) → its history is pruned
    tr.record({ count: 0, buffer: new Float32Array(0), offsets: new Int32Array(0), lengths: new Int32Array(0) }, null, 1, 33);
    tr.measureContributions({ eField, latticeSize: 33 });
    expect(tr.getKnotHistory(id).n).toBe(0);
});

test('reset clears contributions + history', () => {
    const { tr } = twoKnots();
    const c0 = [tr.getKnotZones().centroids[0], tr.getKnotZones().centroids[1], tr.getKnotZones().centroids[2]];
    tr.measureContributions({ eField: { count: 1, positions: new Float32Array(c0), vectors: new Float32Array([1, 0, 0]) }, latticeSize: 33 });
    tr.reset();
    expect(tr.getContributions().count).toBe(0);
    expect(tr.getContributions().totals.energy).toBe(0);
});
