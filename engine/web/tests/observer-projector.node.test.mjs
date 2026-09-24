import test from 'node:test';
import assert from 'node:assert/strict';
import { createPointProjector, projectPoint, boost, cameraBasis } from '../js/observer/optics.js';
import { dot } from '../js/observer/geometry.js';

const close = (actual, expected) => assert.ok(Math.abs(actual - expected) <= 1e-11 * Math.max(1, Math.abs(expected)), `${actual} != ${expected}`);
function reference(snapshot, settings, point) {
    const observer = { ...snapshot.observer, ...settings.cameraOverride };
    const displacement = point.map((value, i) => value - observer.position[i]), distance = Math.hypot(...displacement);
    const optical = snapshot.profile === 'sr' && settings.optical !== false;
    const inverse = snapshot.profile === 'sr' ? boost(optical ? -distance : dot(observer.velocity, displacement), displacement, observer.velocity.map(value => -value)).space : displacement;
    const basis = cameraBasis(observer), depth = dot(inverse, basis.forward), tangent = Math.tan((settings.fov || 60) * Math.PI / 360);
    const homogeneousX = .5 * depth + dot(inverse, basis.right) / (2 * tangent * (settings.aspect || 1));
    const homogeneousY = .5 * depth - dot(inverse, basis.up) / (2 * tangent);
    return { distance, depth, homogeneousX, homogeneousY, x: homogeneousX / depth, y: homogeneousY / depth };
}
function scene(profile = 'sr', velocity = [.22, -.31, .4]) {
    return { time: 12, historyStart: -48, profile, entities: [], segments: [], observer: { position: [2, 1.6, -5], velocity, yaw: .3, pitch: -.2, roll: .14 } };
}

test('cached camera projector agrees with independent Lorentz projection across frame modes', () => {
    const points = [[0, 0, -9], [-10, 4, 7], [.3, -6, 20], [100, 200, -300], [1e80, -2e80, 3e80]];
    for (const profile of ['sr', 'playground']) for (const optical of [true, false]) for (const velocity of [[0, 0, 0], [.22, -.31, .4], [.9, 0, 0]]) {
        const snapshot = scene(profile, velocity), settings = { optical, fov: 83, aspect: 16 / 9, cameraOverride: { roll: -.31 } };
        const projector = createPointProjector(snapshot, settings);
        for (const point of points) {
            const actual = projector(point), expected = reference(snapshot, settings, point);
            for (const key of Object.keys(expected)) close(actual[key], expected[key]);
            assert.deepEqual(actual, projectPoint(snapshot, settings, point));
        }
    }
});

test('cached projector captures one immutable observation and does not change its inputs', () => {
    const snapshot = scene(), settings = { fov: 60, aspect: 1, cameraOverride: { yaw: .3 } }, point = [2, 1, -20];
    const before = structuredClone({ snapshot, settings, point }), projector = createPointProjector(snapshot, settings);
    const result = projector(point);
    assert.deepEqual({ snapshot, settings, point }, before);
    snapshot.observer.position[0] = 500; snapshot.observer.velocity[0] = .1; settings.cameraOverride.yaw = 2; settings.fov = 90;
    assert.deepEqual(projector(point), result);
    assert.notDeepEqual(createPointProjector(snapshot, settings)(point), result);
});

test('Playground projection does not impose an SR velocity restriction', () => {
    const snapshot = scene('playground', [10, 20, -30]), settings = { optical: true };
    const projector = createPointProjector(snapshot, settings), point = [3, 2, -10];
    const actual = projector(point), expected = reference(snapshot, settings, point);
    for (const key of Object.keys(expected)) close(actual[key], expected[key]);
});
