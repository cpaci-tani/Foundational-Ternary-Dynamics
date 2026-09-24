import test from 'node:test';
import assert from 'node:assert/strict';
import { simultaneousBoundsCorners, simultaneousBoundsEdges } from '../js/observer/frame-geometry.js';
import { boostEvent } from '../js/observer/math.js';
import { inverseRotate, rotationMatrix } from '../js/observer/geometry.js';

const close = (actual, expected, tolerance = 1e-10) => assert.ok(Math.abs(actual - expected) < tolerance, `${actual} != ${expected}`);
const observer = (velocity = [0, 0, 0]) => ({ position: [0, 0, 0], velocity });
const body = (changes = {}) => ({ id: 'cube', entityId: 'cube', revision: 1, shape: 'box', position: [3, 2, -8], size: [2, 2, 2], rotation: [0, 0, 0], velocity: [0, 0, 0], originTime: 0, start: -60, end: null, color: [1, 1, 1], ...changes });

test('a stationary observer sees equal-coordinate-time length contraction, not a rest box', () => {
    const corners = simultaneousBoundsCorners(observer(), 0, body({ velocity: [0.6, 0, 0] }));
    close(Math.max(...corners.map(p => p.position[0])) - Math.min(...corners.map(p => p.position[0])), 1.6);
    close(Math.max(...corners.map(p => p.position[1])) - Math.min(...corners.map(p => p.position[1])), 2);
    for (const corner of corners) close(corner.time, 0);
});

test('every moving and rotated corner lies at t prime zero in the observer frame', () => {
    const camera = { position: [-4, 1, 6], velocity: [0.21, -0.34, 0.17] }, time = 9;
    const segment = body({ rotation: [0.32, -0.4, 0.17], velocity: [-0.43, 0.22, 0.1] });
    for (const corner of simultaneousBoundsCorners(camera, time, segment)) {
        const event = [corner.time - time, ...corner.position.map((x, j) => x - camera.position[j])];
        close(boostEvent(event, camera.velocity)[0], 0);
        const bodyEvent = boostEvent([corner.time - segment.originTime, ...corner.position.map((x, j) => x - segment.position[j])], segment.velocity);
        const local = inverseRotate(rotationMatrix(segment.rotation), bodyEvent.slice(1));
        local.forEach((value, j) => close(Math.abs(value), segment.size[j] * 0.5));
    }
});

test('co-moving observers recover full rest length after the frame transform', () => {
    const velocity = [0.8, 0, 0], camera = observer(velocity), segment = body({ velocity });
    const positions = simultaneousBoundsCorners(camera, 0, segment).map(p => boostEvent([p.time, ...p.position], velocity));
    close(Math.max(...positions.map(p => p[1])) - Math.min(...positions.map(p => p[1])), 2);
});

test('revision lifetimes and exhausted history clip rather than invent simultaneous edges', () => {
    const camera = observer([0.6, 0, 0]), segment = body({ position: [0, 0, -8], start: 0, end: 0.3 });
    const edges = simultaneousBoundsEdges(camera, 0, segment, -60);
    assert.ok(edges.length > 0 && edges.length < 12);
    for (const edge of edges) {
        assert.ok(edge.startTime >= -1e-12 && edge.endTime >= -1e-12);
        assert.ok(edge.startTime <= 0.3 + 1e-12 && edge.endTime <= 0.3 + 1e-12);
    }
    assert.equal(simultaneousBoundsEdges(observer(), 0, body({ end: 0 }), -60).length, 0);
    assert.equal(simultaneousBoundsEdges(observer(), 0, body(), 1).length, 0);
    assert.equal(simultaneousBoundsEdges(camera, 0, body({ end: 0.5 }), 1).length, 0);
});

test('mirroring reflects geometry and motion without mutating source records', () => {
    const segment = body({ velocity: [0.1, 0.2, 0.05], rotation: [0.32, 0.4, -0.2] }), original = structuredClone(segment);
    const direct = simultaneousBoundsCorners(observer([0.1, 0.3, 0.2]), 4, segment);
    const mirror = simultaneousBoundsCorners(observer([0.1, -0.3, 0.2]), 4, segment, true);
    mirror.forEach((corner, i) => {
        close(corner.time, direct[i].time);
        corner.position.forEach((value, j) => close(value, direct[i].position[j] * (j === 1 ? -1 : 1)));
    });
    assert.deepEqual(segment, original);
});
