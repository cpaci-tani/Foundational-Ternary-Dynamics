import test from 'node:test';
import assert from 'node:assert/strict';
import { LINK_DISPLACEMENT, siteIndex, writeLinkHalves, selectLinks, writeSelectedLinks } from '../js/link-geometry.js';

const halves = (L, site, k) => {
    const out = new Float32Array(12);
    writeLinkHalves(L, site, k, out, 0);
    return Array.from(out);
};

test('link displacements are the nine owned Moore links: faces and face diagonals, never a corner', () => {
    assert.deepEqual(LINK_DISPLACEMENT.map((d) => [...d]),
        [[1, 0, 0], [0, 1, 0], [0, 0, 1], [1, 1, 0], [1, -1, 0], [1, 0, 1], [1, 0, -1], [0, 1, 1], [0, 1, -1]]);
    for (const d of LINK_DISPLACEMENT) {
        const nonzero = d.filter((v) => v !== 0).length;
        assert.ok(nonzero === 1 || nonzero === 2, `displacement ${d}`);
    }
});

test('owned links cover every one of the 18 neighbour pairs exactly once', () => {
    const L = 4;
    const seen = new Set();
    const key = (a, b) => (a < b ? `${a}:${b}` : `${b}:${a}`);
    for (let x = 0; x < L; x++) for (let y = 0; y < L; y++) for (let z = 0; z < L; z++) {
        for (const d of LINK_DISPLACEMENT) {
            const j = siteIndex(L, (x + d[0] + L) % L, (y + d[1] + L) % L, (z + d[2] + L) % L);
            const pair = key(siteIndex(L, x, y, z), j);
            assert.ok(!seen.has(pair), `duplicate pair ${pair}`);
            seen.add(pair);
        }
    }
    assert.equal(seen.size, (18 / 2) * L ** 3);
});

test('site index is x-major', () => {
    assert.equal(siteIndex(5, 1, 2, 3), (1 * 5 + 2) * 5 + 3);
});

test('an interior link is two halves meeting at the midpoint between site centres', () => {
    const L = 5;
    const site = siteIndex(L, 2, 2, 2);
    LINK_DISPLACEMENT.forEach((d, k) => {
        const [ax, ay, az, mx, my, mz, bx, by, bz, nx, ny, nz] = halves(L, site, k);
        assert.deepEqual([ax, ay, az], [2.5, 2.5, 2.5]);
        assert.deepEqual([mx, my, mz], [2.5 + d[0] / 2, 2.5 + d[1] / 2, 2.5 + d[2] / 2]);
        assert.deepEqual([bx, by, bz], [mx, my, mz]);
        assert.deepEqual([nx, ny, nz], [2.5 + d[0], 2.5 + d[1], 2.5 + d[2]]);
    });
});

test('a wrapped link is drawn as two halves that end on the box faces they cross', () => {
    const L = 5;
    // +x through the x = L face
    assert.deepEqual(halves(L, siteIndex(L, 4, 1, 1), 0),
        [4.5, 1.5, 1.5, 5, 1.5, 1.5, 0, 1.5, 1.5, 0.5, 1.5, 1.5]);
    // +y through the y = L face
    assert.deepEqual(halves(L, siteIndex(L, 1, 4, 1), 1),
        [1.5, 4.5, 1.5, 1.5, 5, 1.5, 1.5, 0, 1.5, 1.5, 0.5, 1.5]);
    // +z through the z = L face
    assert.deepEqual(halves(L, siteIndex(L, 1, 1, 4), 2),
        [1.5, 1.5, 4.5, 1.5, 1.5, 5, 1.5, 1.5, 0, 1.5, 1.5, 0.5]);
    // Owned links never step in -x, so the x = 0 face is only ever reached from the other side.
    // (+1,-1,0) through the y = 0 face
    assert.deepEqual(halves(L, siteIndex(L, 1, 0, 2), 4),
        [1.5, 0.5, 2.5, 2, 0, 2.5, 2, 5, 2.5, 2.5, 4.5, 2.5]);
    // (+1,+1,0) through the x = L, y = L edge
    assert.deepEqual(halves(L, siteIndex(L, 4, 4, 0), 3),
        [4.5, 4.5, 0.5, 5, 5, 0.5, 0, 0, 0.5, 0.5, 0.5, 0.5]);
    // (0,+1,-1) through the z = 0 face only
    assert.deepEqual(halves(L, siteIndex(L, 0, 2, 0), 8),
        [0.5, 2.5, 0.5, 0.5, 3, 0, 0.5, 3, 5, 0.5, 3.5, 4.5]);
});

test('selectLinks keeps exactly the links above the threshold, strongest first, capped', () => {
    const values = new Float32Array([0.5, -1, 0.04, 0, -0.2, 0.06, 0.049]);
    const all = selectLinks(values, 0.05, 100);
    assert.equal(all.max, 1);
    assert.deepEqual(Array.from(all.ids), [1, 0, 4, 5]);
    assert.deepEqual(Array.from(selectLinks(values, 0.05, 2).ids), [1, 0]);
    assert.equal(selectLinks(new Float32Array(9), 0.05, 10).ids.length, 0);
});

test('writeSelectedLinks writes four vertices per selected link and marks the receiving half', () => {
    const L = 3;
    const values = new Float32Array(9 * 27);
    values[9 * siteIndex(L, 1, 1, 1) + 0] = 2;   // toward the +x neighbour
    values[9 * siteIndex(L, 0, 0, 0) + 3] = -1;  // toward the owner
    const { ids } = selectLinks(values, 0.05, 10);
    const positions = new Float32Array(12 * ids.length);
    const head = new Uint8Array(ids.length);
    assert.equal(writeSelectedLinks(L, values, ids, positions, head), 8);
    assert.deepEqual(Array.from(head), [1, 0]);
    assert.deepEqual(Array.from(positions.slice(0, 3)), [1.5, 1.5, 1.5]);
    assert.deepEqual(Array.from(positions.slice(9, 12)), [2.5, 1.5, 1.5]);
});
