// engine/web/tests/field-line-knots-attribution-integration.spec.js
// The tracker's per-knot segments/length/legs must equal attributeSegmentsToKnots
// run against the tracker's OWN detected centroids (guards the wiring).
import { test, expect } from '@playwright/test';
import { FieldLineKnotTracker } from '../js/scales/scale0/runtime/field-line-knots.js';
import { attributeSegmentsToKnots } from '../js/scales/scale0/runtime/knot-line-attribution.js';

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

test('tracker per-knot segments/length/legs equal attribution on its own centroids', () => {
    const tr = new FieldLineKnotTracker({ cellSize: 2, densityThreshold: 2, minCellsPerKnot: 1, crossingDist: 2.0 });
    const sl = makeStreamlines([...clump([5, 5, 5]), ...clump([25, 25, 25]), ...clump([5, 25, 25])]);
    const tel = tr.record(sl, null, 0, 33);
    expect(tel.count).toBe(3);
    const centroids = new Float32Array(tel.count * 3);
    for (let k = 0; k < tel.count; k++) {
        centroids[k * 3] = tel.fields[k * 8]; centroids[k * 3 + 1] = tel.fields[k * 8 + 1]; centroids[k * 3 + 2] = tel.fields[k * 8 + 2];
    }
    const attr = attributeSegmentsToKnots(sl, centroids, tel.count);
    let sumSegs = 0;
    for (let k = 0; k < tel.count; k++) {
        expect(tel.fields[k * 8 + 3]).toBe(attr.get(k).segments);
        expect(tel.fields[k * 8 + 5]).toBe(attr.get(k).legSet.size);
        expect(tel.fields[k * 8 + 6]).toBeCloseTo(attr.get(k).length, 4);
        sumSegs += attr.get(k).segments;
    }
    expect(tr.getAggregate().sumSegs).toBe(sumSegs);
});
