// engine/web/tests/field-line-knots-identity.spec.js
// Identity persistence + birth/death/fission/fusion for the field-line knot tracker.
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
// Mutually-perpendicular crossing at center c (a tangle confined to one cell).
function clump(c, d = 0) {
    const [x, y, z] = c;
    return [
        [[x - 2, y + d, z], [x + 2, y + d, z]],
        [[x, y - 2 + d, z], [x, y + 2 + d, z]],
        [[x, y + d, z - 2], [x, y + d, z + 2]],
    ];
}
// A knot spanning several cells: a crossing clump at each x-center.
function wide(centersX) {
    let lines = [];
    for (const x of centersX) lines = lines.concat(clump([x, 5, 5]));
    return makeStreamlines(lines);
}
const opt = { cellSize: 2, densityThreshold: 2, minCellsPerKnot: 1, crossingDist: 2.0 };
const EMPTY = { count: 0, buffer: new Float32Array(0), offsets: new Int32Array(0), lengths: new Int32Array(0) };

test('persistence: same clump shifted < cellSize keeps id, ages by one tick', () => {
    const tr = new FieldLineKnotTracker(opt);
    const a = tr.record(makeStreamlines(clump([5, 5, 5])), null, 0, 33);
    expect(a.count).toBe(1);
    const id0 = a.ids[0];
    expect(a.age[0]).toBe(0);
    const b = tr.record(makeStreamlines(clump([5, 5, 5], 0.5)), null, 1, 33);
    expect(b.count).toBe(1);
    expect(b.ids[0]).toBe(id0);          // same identity
    expect(b.age[0]).toBe(1);            // aged one tick
    const agg = tr.getAggregate();
    expect(agg.births).toBe(1);          // no second birth
    expect(agg.deaths).toBe(0);
});

test('birth then death: clump appears (births=1), vanishes (deaths=1)', () => {
    const tr = new FieldLineKnotTracker(opt);
    tr.record(makeStreamlines(clump([5, 5, 5])), null, 0, 33);
    expect(tr.getAggregate().births).toBe(1);
    expect(tr.getAggregate().alive).toBe(1);
    tr.record(EMPTY, null, 1, 33);
    expect(tr.getAggregate().deaths).toBe(1);
    expect(tr.getAggregate().alive).toBe(0);
    const evs = tr.getEvents();
    expect([...evs.type]).toContain(1); // Death
});

test('fission: one knot → two, type 3, 1→2', () => {
    const tr = new FieldLineKnotTracker(opt);
    tr.record(wide([5, 7, 9]), null, 0, 33);           // one 3-cell knot
    expect(tr.getTelemetry().count).toBe(1);
    const b = tr.record(wide([5, 9]), null, 1, 33);     // two non-adjacent knots
    expect(b.count).toBe(2);
    expect(tr.getAggregate().fissions).toBe(1);
    const evs = tr.getEvents();
    let found = false;
    for (let i = 0; i < evs.count; i++) if (evs.type[i] === 3 && evs.nparents[i] === 1 && evs.nchildren[i] === 2) found = true;
    expect(found).toBe(true);
});

test('fusion: two knots → one, type 4, 2→1', () => {
    const tr = new FieldLineKnotTracker(opt);
    tr.record(wide([5, 9]), null, 0, 33);               // two non-adjacent knots
    expect(tr.getTelemetry().count).toBe(2);
    const b = tr.record(wide([5, 7, 9]), null, 1, 33);  // one 3-cell knot
    expect(b.count).toBe(1);
    expect(tr.getAggregate().fusions).toBe(1);
    const evs = tr.getEvents();
    let found = false;
    for (let i = 0; i < evs.count; i++) if (evs.type[i] === 4 && evs.nparents[i] === 2 && evs.nchildren[i] === 1) found = true;
    expect(found).toBe(true);
});

test('reset() restarts ids at 0 and zeroes the aggregate', () => {
    const tr = new FieldLineKnotTracker(opt);
    tr.record(wide([5, 9]), null, 0, 33);
    expect(tr.getTelemetry().ids[0]).toBeGreaterThanOrEqual(0);
    tr.reset();
    expect(tr.getAggregate()).toEqual({ alive: 0, births: 0, deaths: 0, fissions: 0, fusions: 0, sumSegs: 0 });
    const a = tr.record(makeStreamlines(clump([5, 5, 5])), null, 0, 33);
    expect(a.ids[0]).toBe(0);
});
