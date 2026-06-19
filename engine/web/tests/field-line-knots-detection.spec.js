// engine/web/tests/field-line-knots-detection.spec.js
// Detection logic for the field-line knot tracker (density + crossings gate).
import { test, expect } from '@playwright/test';
import { FieldLineKnotTracker } from '../js/scales/scale0/runtime/field-line-knots.js';
import { attributeSegmentsToKnots } from '../js/scales/scale0/runtime/knot-line-attribution.js';

// Build a pooled-shape StreamlineResult from an array of polylines
// (each polyline = array of [x,y,z]).
function makeStreamlines(lines) {
    let total = 0;
    for (const l of lines) total += l.length * 3;
    const buffer = new Float32Array(total);
    const offsets = new Int32Array(lines.length);
    const lengths = new Int32Array(lines.length);
    let o = 0;
    for (let i = 0; i < lines.length; i++) {
        offsets[i] = o;
        lengths[i] = lines[i].length * 3;
        for (const v of lines[i]) { buffer[o++] = v[0]; buffer[o++] = v[1]; buffer[o++] = v[2]; }
    }
    return { count: lines.length, buffer, offsets, lengths };
}

// Three mutually-perpendicular lines crossing at center c → a dense tangle.
function crossingClump(c) {
    const [x, y, z] = c;
    return [
        [[x - 2, y, z], [x + 2, y, z]],   // x-dir
        [[x, y - 2, z], [x, y + 2, z]],   // y-dir
        [[x, y, z - 2], [x, y, z + 2]],   // z-dir
    ];
}

test('two crossing clumps at x≈5 and x≈25, Particles=0 → two knots', () => {
    const tr = new FieldLineKnotTracker({ cellSize: 2, densityThreshold: 2, minCellsPerKnot: 1, crossingDist: 2.0 });
    const sl = makeStreamlines([...crossingClump([5, 5, 5]), ...crossingClump([25, 25, 25])]);
    const tel = tr.record(sl, null, 0, 33);
    expect(tel.count).toBe(2);
    // centroids near the two clump centers (order-independent)
    const cs = [[tel.fields[0], tel.fields[1], tel.fields[2]], [tel.fields[8], tel.fields[9], tel.fields[10]]];
    const near = (p, t) => Math.abs(p[0] - t) <= 2;
    expect(cs.some((p) => near(p, 5)) && cs.some((p) => near(p, 25))).toBe(true);
    // each knot has at least one crossing recorded (fields[k*8+4])
    expect(tel.fields[4]).toBeGreaterThanOrEqual(1);
    expect(tel.fields[12]).toBeGreaterThanOrEqual(1);
});

test('dense PARALLEL bundle (no crossings) → zero knots (AND gate)', () => {
    const tr = new FieldLineKnotTracker({ cellSize: 2, densityThreshold: 2, minCellsPerKnot: 1, crossingDist: 2.0 });
    // five parallel x-direction lines packed into one cell — dense, never crossing
    const lines = [
        [[3, 5.0, 5.0], [7, 5.0, 5.0]],
        [[3, 5.2, 5.0], [7, 5.2, 5.0]],
        [[3, 4.8, 5.0], [7, 4.8, 5.0]],
        [[3, 5.0, 5.2], [7, 5.0, 5.2]],
        [[3, 5.0, 4.8], [7, 5.0, 4.8]],
    ];
    const tel = tr.record(makeStreamlines(lines), null, 0, 33);
    expect(tel.count).toBe(0);
});

test('sparse crossing below density threshold → zero knots (density half of AND gate)', () => {
    const tr = new FieldLineKnotTracker({ cellSize: 2, densityThreshold: 4, minCellsPerKnot: 1, crossingDist: 2.0 });
    // two crossing lines (density 2) but threshold 4 → not dense enough
    const sl = makeStreamlines([[[3, 5, 5], [7, 5, 5]], [[5, 3, 5], [5, 7, 5]]]);
    const tel = tr.record(sl, null, 0, 33);
    expect(tel.count).toBe(0);
});

test('segments / length / legs match a direct attributeSegmentsToKnots call', () => {
    const tr = new FieldLineKnotTracker({ cellSize: 2, densityThreshold: 2, minCellsPerKnot: 1, crossingDist: 2.0 });
    const sl = makeStreamlines([...crossingClump([5, 5, 5]), ...crossingClump([25, 25, 25])]);
    const tel = tr.record(sl, null, 0, 33);
    expect(tel.count).toBe(2);
    const centroids = new Float32Array([
        tel.fields[0], tel.fields[1], tel.fields[2],
        tel.fields[8], tel.fields[9], tel.fields[10],
    ]);
    const attr = attributeSegmentsToKnots(sl, centroids, 2);
    for (let k = 0; k < 2; k++) {
        expect(tel.fields[k * 8 + 3]).toBe(attr.get(k).segments);
        expect(tel.fields[k * 8 + 5]).toBe(attr.get(k).legSet.size);
        expect(tel.fields[k * 8 + 6]).toBeCloseTo(attr.get(k).length, 4);
    }
});
