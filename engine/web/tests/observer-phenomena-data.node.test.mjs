import test from 'node:test';
import assert from 'node:assert/strict';
import { phenomenaData } from '../js/observer/phenomena-data.js';

const close = (actual, expected, tolerance = 1e-12) => assert.ok(Math.abs(actual - expected) <= tolerance, `${actual} != ${expected}`);
const scene = (patch = {}) => ({
    time: 10, profile: 'sr', historyStart: -50,
    observer: { position: [0, 0, 0], velocity: [0, 0, 0], yaw: 0, pitch: 0, roll: 0 },
    entities: [], segments: [], ...patch,
});
const hit = (patch = {}) => ({
    entityId: 'cube', id: 'cube', revision: 3, distance: 5, emissionTime: 5,
    properTime: 4, position: [0, 0, -5], sourcePosition: [0, 0, -5], restPosition: [0, 0, 0],
    normal: [0, 0, 1], historical: false, mirrored: false, segmentIndex: 0, doppler: 1, ...patch,
});
const segment = (patch = {}) => ({
    entityId: 'cube', revision: 1, start: -50, end: null, originTime: 0,
    position: [0, 0, -5], velocity: [.6, 0, 0], ...patch,
});

test('received surface readout independently satisfies the null interval', () => {
    const data = phenomenaData(scene(), { layers: { lightPaths: true } }, hit());
    assert.equal(data.available, true);
    assert.deepEqual(data.axis, [0, 0, -1]);
    assert.equal(data.axisLabel, 'Received surface direction');
    assert.equal(data.received.entityId, 'cube'); assert.equal(data.received.revision, 3);
    assert.equal(data.received.emissionTime, 5); assert.equal(data.received.arrivalTime, 10);
    assert.equal(data.received.delay, 5); assert.equal(data.received.distance, 5);
    assert.equal(data.received.properTime, 4); assert.equal(data.received.doppler, 1);
    assert.equal(data.received.nullResidual, 0); assert.equal(data.received.xi, 5);
    assert.equal(phenomenaData(scene()).received, null);
});

test('stale, future, unavailable-history and simultaneous hits are not received events', () => {
    assert.equal(phenomenaData(scene({ time: 11 }), {}, hit()).received, null);
    assert.equal(phenomenaData(scene(), {}, hit({ emissionTime: 10.5, distance: -.5 })).received, null);
    assert.equal(phenomenaData(scene({ historyStart: 6 }), {}, hit()).received, null);
    assert.equal(phenomenaData(scene(), { optical: false }, hit()).received, null);
    assert.equal(phenomenaData(scene(), {}, hit({ position: [NaN, 0, -5] })).received, null);
    assert.equal(phenomenaData(scene(), {}, hit({ properTime: Infinity })).received, null);
});

test('Playground produces no SR events, compass, or source trails', () => {
    const data = phenomenaData(scene({ profile: 'playground', segments: [segment()] }), { selectedId: 'cube' }, hit());
    assert.equal(data.available, false); assert.match(data.reason, /Playground/);
    assert.equal(data.received, null); assert.deepEqual(data.compass, []); assert.deepEqual(data.trails, []);
});

test('mirrored received surface retains its actual reflected position and history identity', () => {
    const data = phenomenaData(scene(), { layers: { lightPaths: true } }, hit({ position: [0, -3, -4], sourcePosition: [0, 3, -4], historical: true, mirrored: true }));
    assert.deepEqual(data.received.position, [0, -3, -4]);
    assert.equal(data.received.mirrored, true); assert.equal(data.received.historical, true);
    close(data.axis[1], -.6); close(data.axis[2], -.8); close(data.received.nullResidual, 0);
});

test('camera relocation rejects a cached non-null hit while retaining valid frame readouts', () => {
    const data = phenomenaData(scene(), { cameraOverride: { position: [3, 0, 0], velocity: [.6, 0, 0] } }, hit());
    assert.equal(data.received, null);
    close(data.beta, .6); close(data.clockRate, .8); close(data.parallelBeta, .6);
    assert.deepEqual(data.axis, [1, 0, 0]);
});

test('aberration compass uses source directions with the correct propagation sign', () => {
    const data = phenomenaData(scene(), { cameraOverride: { velocity: [.6, 0, 0] } });
    assert.equal(data.compass.length, 6);
    const transverse = data.compass.find(entry => entry.label === '+Y');
    assert.deepEqual(transverse.original, [0, 1, 0]);
    close(transverse.received[0], .6); close(transverse.received[1], .8);
    close(transverse.angle, Math.acos(.8));
    for (const entry of data.compass) close(Math.hypot(...entry.received), 1);
    close(data.compass.find(entry => entry.label === '+X').angle, 0);
    close(data.compass.find(entry => entry.label === '−X').angle, 0);
    const resting = phenomenaData(scene());
    for (const entry of resting.compass) close(entry.angle, 0);
});

test('clock rates and motion-axis projection agree with SR at beta .6 and .8', () => {
    for (const [beta, rate] of [[.6, .8], [.8, .6]]) {
        const data = phenomenaData(scene(), { cameraOverride: { velocity: [0, beta, 0] } });
        close(data.beta, beta); close(data.parallelBeta, beta); close(data.clockRate, rate); close(data.gamma, 1 / rate);
    }
    const radial = phenomenaData(scene(), { cameraOverride: { velocity: [.6, 0, 0] }, layers: { lightPaths: true } }, hit());
    close(radial.parallelBeta, 0);
    const turned = phenomenaData(scene(), { cameraOverride: { yaw: Math.PI / 2 } });
    close(turned.axis[0], -1); close(turned.axis[2], 0);
});

test('selected source trails respect retained history, time window and revision gaps', () => {
    const snapshot = scene({ time: 30, historyStart: 15, segments: [
        segment({ entityId: 'other' }),
        segment({ revision: 1, start: 0, end: 17 }),
        segment({ revision: 2, start: 21, end: 25 }),
        segment({ revision: 3, start: 25, end: 25 }),
        segment({ revision: 4, start: 31, end: null }),
    ] });
    const data = phenomenaData(snapshot, { selectedId: 'cube', cameraOverride: { velocity: [.6, 0, 0] } });
    assert.equal(data.trails.length, 2);
    assert.deepEqual(data.trails.map(trail => trail.revision), [1, 2]);
    for (const trail of data.trails) { assert.equal(trail.id, 'cube'); assert.equal(trail.points.length, 8); }
    close(data.trails[0].points[0][0], 9); close(data.trails[0].points[0][1], -15);
    close(data.trails[0].points.at(-1)[1], -13); close(data.trails[1].points[0][1], -9);
    close(data.trails[1].points.at(-1)[1], -5);
    const limited = phenomenaData(scene({ time: 30, segments: [segment()] }), { selectedId: 'cube' });
    close(limited.trails[0].points[0][1], -20); close(limited.trails[0].points.at(-1)[1], 0);
    assert.deepEqual(phenomenaData(snapshot).trails, []);
});

test('source trails retain at most the last sixteen segments and 128 sample pairs', () => {
    const segments = Array.from({ length: 40 }, (_, index) => segment({ revision: index, start: index / 2, end: (index + 1) / 2 }));
    const data = phenomenaData(scene({ time: 20, segments }), { selectedId: 'cube' });
    assert.equal(data.trails.length, 16); assert.equal(data.trails[0].revision, 24);
    assert.equal(data.trails.reduce((sum, trail) => sum + trail.points.length, 0), 128);
});

test('data collection and returned-vector edits cannot mutate snapshot, settings or hit', () => {
    const snapshot = scene({ segments: [segment()] });
    const settings = { selectedId: 'cube', layers: { lightPaths: true }, cameraOverride: { velocity: [.6, 0, 0] } };
    const received = hit(); const before = structuredClone({ snapshot, settings, received });
    const data = phenomenaData(snapshot, settings, received);
    data.axis[0] = 500; data.received.position[0] = 500; data.compass[0].original[0] = 500;
    data.compass[0].received[0] = 500; data.trails[0].points[0][0] = 500;
    assert.deepEqual({ snapshot, settings, received }, before);
    assert.deepEqual(phenomenaData(snapshot).compass[0].original, [1, 0, 0]);
});

test('non-timelike or nonfinite observer events fail visibly instead of producing fake SR values', () => {
    for (const velocity of [[1, 0, 0], [NaN, 0, 0]]) {
        const data = phenomenaData(scene(), { cameraOverride: { velocity } });
        assert.equal(data.available, false); assert.match(data.reason, /timelike/);
    }
});
