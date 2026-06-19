// engine/web/tests/field-line-knots-color.spec.js
// Per-knot color (knotHue) + selection API on the field-line knot tracker.
import { test, expect } from '@playwright/test';
import { FieldLineKnotTracker, knotHue } from '../js/scales/scale0/runtime/field-line-knots.js';

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

test('knotHue is deterministic and in [0,1)', () => {
    for (const id of [0, 1, 2, 7, 42, 1000]) {
        const h = knotHue(id);
        expect(h).toBeGreaterThanOrEqual(0);
        expect(h).toBeLessThan(1);
        expect(knotHue(id)).toBe(h); // stable
    }
    // distinct ids → distinct hues (over a small spread)
    const hues = new Set([0, 1, 2, 3, 4, 5].map((i) => knotHue(i).toFixed(6)));
    expect(hues.size).toBe(6);
});

test('getKnotZones carries ids (length == count) and selectedId default -1', () => {
    const tr = new FieldLineKnotTracker(opt);
    tr.record(makeStreamlines([...clump([5, 5, 5]), ...clump([25, 25, 25])]), null, 0, 33);
    const z = tr.getKnotZones();
    expect(z.count).toBe(2);
    expect(z.ids.length).toBe(2);
    expect(z.selectedId).toBe(-1);
});

test('setSelected reflects in getKnotZones().selectedId; clearing resets to -1', () => {
    const tr = new FieldLineKnotTracker(opt);
    tr.record(makeStreamlines(clump([5, 5, 5])), null, 0, 33);
    const id = tr.getTelemetry().ids[0];
    tr.setSelected(id);
    expect(tr.getSelected()).toBe(id);
    expect(tr.getKnotZones().selectedId).toBe(id);
    tr.setSelected(null);
    expect(tr.getKnotZones().selectedId).toBe(-1);
});

test('reset() clears the selection', () => {
    const tr = new FieldLineKnotTracker(opt);
    tr.record(makeStreamlines(clump([5, 5, 5])), null, 0, 33);
    tr.setSelected(tr.getTelemetry().ids[0]);
    tr.reset();
    expect(tr.getSelected()).toBe(-1);
});
