import test from 'node:test';
import assert from 'node:assert/strict';

import {
    clusterInspectionFocus,
    focusedFieldSources,
    focusedSystemObservables,
    particleInspectionFocus,
    reconcileInspectionFocus,
} from '../js/scales/scale1/inspection-focus.js';

test('particle and cluster focus normalize and reconcile against live hierarchy', () => {
    const particle = particleInspectionFocus(7);
    assert.deepEqual(particle.particleIds, [7]);

    const cluster = clusterInspectionFocus({
        key: '3.7', id: 'C2', anchorId: 7, energy: 12,
        center: { x: 1, y: 2, z: 3 },
        particles: [{ id: 7 }, { id: 3 }],
    });
    assert.deepEqual(cluster.particleIds, [3, 7]);
    assert.equal(cluster.anchorId, 7);

    const reconfigured = reconcileInspectionFocus(cluster, {
        energyBasis: 'dynamic_activity',
        particles: [{ id: 3 }, { id: 7 }, { id: 9 }],
        clusters: [{
            key: '3.7.9', id: 'C1', anchorId: 9, energy: 20,
            center: { x: 4, y: 5, z: 6 },
            particles: [{ id: 3 }, { id: 7 }, { id: 9 }],
        }],
    });
    assert.equal(reconfigured.key, '3.7.9');
    assert.deepEqual(reconfigured.particleIds, [3, 7, 9]);
    assert.equal(reconcileInspectionFocus(particle, { particles: [], clusters: [] }), null);
});

test('field and system observables include only focused native particle IDs', () => {
    const focus = clusterInspectionFocus({
        id: 'C1', anchorId: 4,
        particles: [{ id: 2 }, { id: 4 }],
    });
    const ids = new Int32Array([2, 3, 4]);
    const sources = {
        positions: new Float32Array([1, 0, 0, 99, 0, 0, 5, 0, 0]),
        charges: new Float32Array([1, 50, -1]),
        masses: new Float32Array([2, 50, 2]),
        count: 3,
    };
    const filtered = focusedFieldSources(sources, ids, focus, {});
    assert.equal(filtered.count, 2);
    assert.deepEqual(Array.from(filtered.positions.slice(0, 6)), [1, 0, 0, 5, 0, 0]);
    assert.deepEqual(Array.from(filtered.charges.slice(0, 2)), [1, -1]);

    const observables = focusedSystemObservables({
        ids,
        positions: sources.positions,
        masses: sources.masses,
        velocities: new Float32Array([1, 0, 0, 10, 0, 0, 0, 1, 0]),
        count: 3,
    }, focus);
    assert.deepEqual(observables.center, [3, 0, 0]);
    assert.deepEqual(observables.momentum, [2, 2, 0]);
    assert.deepEqual(observables.angularMomentum, [0, 0, 4]);
    assert.equal(observables.count, 2);
});
