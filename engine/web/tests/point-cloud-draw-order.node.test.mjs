import test from 'node:test';
import assert from 'node:assert/strict';

// Translucent THREE.Points clouds with depthWrite:false and NormalBlending are
// composited in draw order. Draw order is buffer order unless an index buffer
// re-orders it, so a view from one side draws near points last (a solid shell)
// and the opposite view draws far points last (hollow, inside-out). These
// helpers produce a back-to-front index order for the current camera.
import {
    dominantAxisKey,
    buildAxisOrder,
    sortBackToFront,
    attachBackToFrontOrdering,
} from '../js/viewport/point-cloud-draw-order.js';

function lattice(n) {
    // x-fast cubic lattice of voxel centres, matching the flux cloud's layout.
    const positions = new Float32Array(n * n * n * 3);
    let i = 0;
    for (let z = 0; z < n; z++) for (let y = 0; y < n; y++) for (let x = 0; x < n; x++) {
        positions[i++] = x + 0.5; positions[i++] = y + 0.5; positions[i++] = z + 0.5;
    }
    return positions;
}

function depthAlong(positions, index, dir) {
    return positions[index * 3] * dir[0] + positions[index * 3 + 1] * dir[1] + positions[index * 3 + 2] * dir[2];
}

test('dominantAxisKey encodes the largest view-direction component and its sign', () => {
    assert.equal(dominantAxisKey([1, 0.2, -0.3]), 1);   // +x
    assert.equal(dominantAxisKey([-1, 0.2, 0.3]), 0);   // -x
    assert.equal(dominantAxisKey([0.1, 0.9, -0.2]), 3); // +y
    assert.equal(dominantAxisKey([0.1, -0.9, 0.2]), 2); // -y
    assert.equal(dominantAxisKey([0.1, 0.2, 0.9]), 5);  // +z
    assert.equal(dominantAxisKey([0.1, 0.2, -0.9]), 4); // -z
});

test('buildAxisOrder walks the lattice far-to-near along the dominant view axis, every index exactly once', () => {
    const n = 5, count = n * n * n, positions = lattice(n);
    for (const dir of [[1, 0, 0], [-1, 0, 0], [0, 1, 0], [0, -1, 0], [0, 0, 1], [0, 0, -1]]) {
        const order = buildAxisOrder(positions, count, dominantAxisKey(dir));
        assert.equal(order.length, count);
        assert.equal(new Set(order).size, count, 'permutation');
        // A point drawn later must never be farther along the view direction
        // than a point drawn earlier: depth is non-increasing in draw order.
        for (let k = 1; k < count; k++) {
            assert.ok(depthAlong(positions, order[k], dir) <= depthAlong(positions, order[k - 1], dir) + 1e-6,
                `dir ${dir}: index ${k} drawn out of order`);
        }
        // Last drawn = nearest to the camera, first drawn = farthest.
        assert.ok(depthAlong(positions, order[count - 1], dir) <= depthAlong(positions, order[0], dir));
    }
});

test('buildAxisOrder respects the draw count and ignores stale slots beyond it', () => {
    const n = 3, positions = lattice(n), live = 10;
    const order = buildAxisOrder(positions, live, dominantAxisKey([0, 0, 1]));
    assert.equal(order.length, live);
    assert.ok(Math.max(...order) < live);
});

test('sortBackToFront orders arbitrary off-lattice points by decreasing view depth', () => {
    const count = 1000, positions = new Float32Array(count * 3);
    let seed = 12345;
    const rnd = () => (seed = (seed * 1664525 + 1013904223) >>> 0) / 4294967296;
    for (let i = 0; i < count * 3; i++) positions[i] = rnd() * 200 - 100;
    const dir = [0.36, -0.48, 0.8];
    const order = sortBackToFront(positions, count, dir);
    assert.equal(order.length, count);
    assert.equal(new Set(order).size, count, 'permutation');
    for (let k = 1; k < count; k++) {
        // Quantised sort: allow equality within one bucket, never a reversal
        // larger than the bucket width.
        const d0 = depthAlong(positions, order[k - 1], dir), d1 = depthAlong(positions, order[k], dir);
        assert.ok(d1 <= d0 + 0.05, `reversal at ${k}: ${d0} -> ${d1}`);
    }
});

test('attachBackToFrontOrdering installs an onBeforeRender hook that re-indexes only when the view octant changes', () => {
    const n = 4, count = n * n * n, positions = lattice(n);
    let indexSets = 0;
    const geometry = {
        index: null,
        drawRange: { start: 0, count },
        attributes: { position: { array: positions, count } },
        getAttribute(name) { return this.attributes[name]; },
        // Count only real re-indexing; detach() legitimately sets null.
        setIndex(arr) { this.index = arr; if (arr) indexSets++; },
        setDrawRange(start, c) { this.drawRange = { start, count: c }; },
    };
    const points = { geometry, onBeforeRender: null };
    const camera = { getWorldDirection(v) { v.set(...dirNow); return v; } };
    let dirNow = [0, 0, 1];
    const detach = attachBackToFrontOrdering(points, { mode: 'axis' });
    assert.equal(typeof points.onBeforeRender, 'function');

    points.onBeforeRender(null, null, camera);
    assert.equal(indexSets, 1);
    assert.equal(geometry.drawRange.count, count, 'draw range preserved after indexing');
    // Looking toward +z the camera sits at low z: the z=0.5 row is nearest and
    // must be drawn last (a painter's shell), the z=3.5 row first.
    assert.equal(positions[geometry.index[count - 1] * 3 + 2], 0.5, 'facing +z, nearest row (z=0.5) drawn last');
    assert.equal(positions[geometry.index[0] * 3 + 2], 3.5, 'facing +z, farthest row (z=3.5) drawn first');
    points.onBeforeRender(null, null, camera);
    assert.equal(indexSets, 1, 'same octant: no re-index');

    dirNow = [0, 0, -1];
    points.onBeforeRender(null, null, camera);
    assert.equal(indexSets, 2, 'opposite octant: re-indexed');
    // Looking toward -z the camera sits at high z: now z=3.5 is nearest.
    assert.equal(positions[geometry.index[count - 1] * 3 + 2], 3.5, 'facing -z, nearest row (z=3.5) drawn last');
    assert.equal(positions[geometry.index[0] * 3 + 2], 0.5, 'facing -z, farthest row (z=0.5) drawn first');

    detach();
    // three.js invokes object.onBeforeRender unconditionally each render, so
    // detach must leave a callable no-op behind, never null.
    assert.equal(typeof points.onBeforeRender, 'function', 'detach leaves a callable hook');
    assert.equal(geometry.index, null, 'detach removes the index buffer');
    assert.doesNotThrow(() => points.onBeforeRender(null, null, camera));
    assert.equal(geometry.index, null, 'the no-op never re-indexes');
    assert.equal(indexSets, 2, 'the no-op never re-indexes');
});

test('attachBackToFrontOrdering depth mode writes into the buffer that is actually uploaded, even when wrapIndex copies the array', () => {
    // Regression for a bug where three.js's Uint32BufferAttribute does not
    // keep the input array as its live backing store (it may copy), so
    // sorting into the ORIGINAL array never reached the GPU-uploaded one:
    // the uploaded index stayed all-zero forever and only vertex 0 ever drew.
    const count = 50, positions = new Float32Array(count * 3);
    let seed = 42;
    const rnd = () => (seed = (seed * 1664525 + 1013904223) >>> 0) / 4294967296;
    for (let i = 0; i < count * 3; i++) positions[i] = rnd() * 100 - 50;
    const geometry = {
        index: null,
        drawRange: { start: 0, count },
        attributes: { position: { array: positions, count } },
        getAttribute(name) { return this.attributes[name]; },
        setIndex(arr) { this.index = arr; },
        setDrawRange(start, c) { this.drawRange = { start, count: c }; },
    };
    const points = { geometry, material: { blending: 1 }, onBeforeRender: null };
    const camera = { getWorldDirection(v) { return v.set(0.3, -0.2, 0.9).normalize().negate(); } };
    // Mimics THREE.Uint32BufferAttribute: does NOT keep the input array as its
    // live backing store — copies into its own array, exactly like the
    // three.js build this regressed against.
    const wrapIndex = (order) => ({ array: new Uint32Array(order), needsUpdate: false });
    attachBackToFrontOrdering(points, { mode: 'depth', wrapIndex });

    points.onBeforeRender(null, null, camera);
    assert.ok(geometry.index, 'indexed');
    // The backing buffer may be over-allocated for growth headroom (like the
    // production code's Math.max(count, ...*2) sizing) — only the first
    // `count` entries are ever read, per geometry.setDrawRange(0, count).
    const uploaded = geometry.index.array.slice(0, count);
    assert.equal(new Set(uploaded).size, count,
        'uploaded index must be a permutation of 0..count-1, not all zeros');
    assert.deepEqual([...uploaded].sort((a, b) => a - b), [...Array(count).keys()]);

    // A second frame (no growth) must keep updating the SAME uploaded buffer.
    const firstUploadRef = geometry.index.array;
    points.onBeforeRender(null, null, camera);
    assert.equal(geometry.index.array, firstUploadRef, 'stable buffer reused across frames');
    assert.equal(new Set(geometry.index.array.slice(0, count)).size, count);

    // Growth to a much larger count must still end up correctly indexed.
    const count2 = 3000, positions2 = new Float32Array(count2 * 3);
    for (let i = 0; i < count2 * 3; i++) positions2[i] = rnd() * 100 - 50;
    geometry.attributes.position = { array: positions2, count: count2 };
    geometry.drawRange = { start: 0, count: count2 };
    points.onBeforeRender(null, null, camera);
    const grown = geometry.index.array.slice(0, count2);
    assert.equal(new Set(grown).size, count2, 'after growth, still a real permutation, not all zeros');
});

test('the hook survives three.js Camera.getWorldDirection, which chains set().normalize().negate()', () => {
    // three.js Object3D.getWorldDirection returns target.set(e8,e9,e10).normalize();
    // Camera overrides it to .negate() that result. A target vector missing
    // any of the three methods throws inside three, which is exactly what a
    // fake camera that only calls set() cannot detect.
    const n = 3, count = n * n * n, positions = lattice(n);
    const geometry = {
        index: null, drawRange: { start: 0, count },
        attributes: { position: { array: positions, count } },
        getAttribute(name) { return this.attributes[name]; },
        setIndex(arr) { this.index = arr; },
        setDrawRange(start, c) { this.drawRange = { start, count: c }; },
    };
    const points = { geometry, onBeforeRender: null };
    // Matrix column +Z of a camera looking toward world -z is (0,0,+1);
    // three negates it to yield the look direction (0,0,-1).
    const camera = { getWorldDirection(v) { return v.set(0, 0, 3).normalize().negate(); } };
    attachBackToFrontOrdering(points, { mode: 'axis' });
    assert.doesNotThrow(() => points.onBeforeRender(null, null, camera));
    assert.ok(geometry.index, 'indexed');
    // Look direction is -z: nearest row is z = 2.5, drawn last.
    assert.equal(positions[geometry.index[count - 1] * 3 + 2], 2.5);
});
