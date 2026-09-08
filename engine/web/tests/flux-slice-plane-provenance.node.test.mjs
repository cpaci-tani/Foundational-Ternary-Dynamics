import test from 'node:test';
import assert from 'node:assert/strict';
import { resolveSamplePlane, sliceVectorMag } from '../js/scales/scale0/ui/overlays/flux-slice-helpers.js';
import { FIELD_SAMPLE_KINDS } from '../js/bridge/ws-binary-codec.js';
import { visualSampleGrid } from '../js/lib/visual-sample-grid.js';

const INTERIOR = new Set(['vorticity', 'helicity', 'kretschmann', 'fisher', 'coherence', 'curlJ']);

test('all declared native kinds use their canonical interior rule on an even lattice', () => {
    for (const kind of FIELD_SAMPLE_KINDS) {
        const interior = INTERIOR.has(kind);
        const grid = visualSampleGrid(8, 2, interior);
        const sample = { kind, origin: grid.origin, effectiveStride: grid.stride };
        assert.equal(grid.origin, 1);
        assert.equal(resolveSamplePlane(sample, 0, 7, 8), interior ? 5 : 7, kind);
        assert.equal(resolveSamplePlane(sample, 0, 2147483647, 8), interior ? 5 : 7, kind);
        assert.equal(resolveSamplePlane(sample, 0, -2147483648, 8), 1, kind);
    }
});

test('nonzero full-grid boundary feature survives; interior slices select the preceding plane', () => {
    const sample = { kind: 'fluxVector', origin: 1, effectiveStride: 2, count: 2,
        positions: new Float32Array([5.5, 3.5, 3.5, 7.5, 3.5, 3.5]),
        vectors: new Float32Array([2, 0, 0, 9, 0, 0]) };
    const index = (8 - 1 - 3) * 8 + 3;
    assert.equal(sliceVectorMag(sample, 0, 7, 8)[index], 9);
    assert.equal(sliceVectorMag(sample, 0, 2147483647, 8)[index], 9);
    assert.equal(sliceVectorMag({ ...sample, kind: 'curlJ' }, 0, 7, 8)[index], 2);
});

test('explicit grid extent remains usable when the legacy sample has no registered kind', () => {
    const sample = { origin: 1, effectiveStride: 2 };
    assert.equal(resolveSamplePlane({ ...sample, interior: false }, 0, 7, 8), 7);
    assert.equal(resolveSamplePlane({ ...sample, interior: true }, 0, 7, 8), 5);
    assert.equal(resolveSamplePlane({ ...sample, axisCount: 4 }, 0, 7, 8), 7);
    assert.equal(resolveSamplePlane({ ...sample, axisCount: 3 }, 0, 7, 8), 5);
    // A declared native kind takes precedence over a contradictory display hint.
    assert.equal(resolveSamplePlane({ ...sample, kind: 'e', interior: true }, 0, 7, 8), 7);
});

test('legacy sparse samples retain their declared zero midpoint rather than jumping to a feature', () => {
    const sample = { effectiveStride: 3, origin: 1, count: 1,
        positions: new Float32Array([10.5, 10.5, 20.5]) };
    assert.equal(resolveSamplePlane(sample, 0, 90, 181), 91);
    assert.equal(sample.kind, undefined); // Compatibility does not invent a kind.
});
