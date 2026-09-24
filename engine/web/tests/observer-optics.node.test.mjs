import test from 'node:test';
import assert from 'node:assert/strict';
import { boost, observerRay, traceObserverRay, projectPoint, apparentCenter, visibleSegments } from '../js/observer/optics.js';
import { SHAPE_NAMES, restMesh, subdivideRestMesh, rotationMatrix, rotate, inverseRotate, intersectShape, buildBVH, hitBounds, cross } from '../js/observer/geometry.js';
import { feedbackConfiguration } from '../js/observer/feedback.js';

const close = (a, b, tolerance = 1e-9) => assert.ok(Math.abs(a - b) < tolerance, `${a} differs from ${b}`);
function body(overrides = {}) { return { id: 'a', entityId: 'a', revision: 1, start: -60, end: null, originTime: 0, position: [0, 1.6, 0], velocity: [0, 0, 0], size: [2, 2, 2], rotation: [0, 0, 0], shape: 'sphere', color: [1, 1, 1], clockOffset: 0, alive: true, ...overrides }; }
function scene(segments = [body()], overrides = {}) { return { time: 0, historyStart: -60, profile: 'sr', observer: { position: [0, 1.6, 8], velocity: [0, 0, 0], yaw: 0, pitch: 0, roll: 0 }, entities: segments, segments, ...overrides }; }

test('mirror images retain source identity and historical event while the reference grid is passable', () => {
    const original = body({ position: [0, 3, 0] }), snapshot = scene([original]);
    const camera = { position: [0, -3, 8], pitch: 0 };
    const hit = traceObserverRay(snapshot, { mirrorWorld: true, cameraOverride: camera });
    assert.equal(hit.entityId, original.id); assert.equal(hit.mirrored, true);
    close(hit.distance, 7); close(hit.position[1], -3); close(hit.sourcePosition[1], 3); close(hit.emissionTime, -7);
    assert.equal(traceObserverRay(snapshot, { mirrorWorld: false, cameraOverride: camera }), null);
    const above = { position: [0, 1, 8], pitch: -Math.atan2(4, 8) };
    assert.equal(traceObserverRay(snapshot, { cameraOverride: above }), null);
    assert.equal(traceObserverRay(snapshot, { mirrorWorld: true, cameraOverride: above }).mirrored, true);
});

test('mirror reflection preserves asymmetric mesh handedness and moving historical rest geometry', () => {
    const shape = body({ shape: 'wedge', position: [0, 4, 0], rotation: [.31, .23, .61], velocity: [.12, .08, -.11], size: [3, 2, 2], originTime: -8 });
    const snapshot = scene([shape]), original = structuredClone(snapshot);
    const upperCamera = { position: [0, 4, 8], yaw: .02, pitch: -.01 };
    const lowerCamera = { position: [0, -4, 8], yaw: .02, pitch: .01 };
    let hits = 0;
    for (const x of [-.15, 0, .15]) for (const y of [-.1, 0, .1]) {
        const upper = traceObserverRay(snapshot, { cameraOverride: upperCamera }, x, y);
        const lower = traceObserverRay(snapshot, { mirrorWorld: true, cameraOverride: lowerCamera }, x, -y);
        assert.equal(!!upper, !!lower);
        if (!upper) continue;
        hits++; assert.equal(lower.mirrored, true); assert.equal(lower.entityId, upper.entityId);
        close(lower.distance, upper.distance); close(lower.properTime, upper.properTime); close(lower.emissionTime, upper.emissionTime);
        lower.position.forEach((value, j) => close(value, j === 1 ? -upper.position[j] : upper.position[j]));
        lower.normal.forEach((value, j) => close(value, j === 1 ? -upper.normal[j] : upper.normal[j]));
        lower.restPosition.forEach((value, j) => close(value, upper.restPosition[j]));
    }
    assert.ok(hits >= 2); assert.deepEqual(snapshot, original);
});

test('mirrored deleted revisions disappear only after their original light history expires', () => {
    const deleted = body({ position: [0, 3, 0], end: -7.5 }), snapshot = scene([deleted], { entities: [body({ alive: false, revision: 2 })] });
    const settings = { mirrorWorld: true, cameraOverride: { position: [0, -3, 8] } };
    const hit = traceObserverRay(snapshot, settings);
    assert.equal(hit.mirrored, true); assert.equal(hit.historical, true); close(hit.distance, 9);
    assert.equal(traceObserverRay({ ...snapshot, historyStart: -8 }, settings), null);
});

test('mirrored observation preserves aberration and Doppler for reflected observer velocity', () => {
    const shape = body({ position: [0, 4, 0], velocity: [.05, .02, 0], size: [3, 3, 3] });
    const snapshot = scene([shape]);
    const upper = { position: [0, 4, 8], velocity: [.05, .08, .1] };
    const lower = { position: [0, -4, 8], velocity: [.05, -.08, .1] };
    for (const [x, y] of [[0, 0], [.04, .08], [-.06, -.05]]) {
        const source = traceObserverRay(snapshot, { cameraOverride: upper }, x, y);
        const image = traceObserverRay(snapshot, { mirrorWorld: true, cameraOverride: lower }, x, -y);
        assert.ok(source && image); assert.equal(image.mirrored, true);
        close(image.distance, source.distance); close(image.doppler, source.doppler); close(image.properTime, source.properTime);
    }
});

test('camera recursion configuration always terminates within five passes and three layers', () => {
    assert.deepEqual(feedbackConfiguration({ feedbackEnabled: true, feedbackLayers: Infinity, feedbackDepth: NaN }), { enabled: true, layers: 2, depth: 3, strength: .45, scale: .6 });
    const bounded = feedbackConfiguration({ feedbackEnabled: true, feedbackLayers: 20, feedbackDepth: 1e12, feedbackStrength: 100, feedbackScale: 10 });
    assert.equal(bounded.layers, 3); assert.equal(bounded.depth, 5); assert.equal(bounded.strength, 1); assert.equal(bounded.scale, 1);
    assert.equal(feedbackConfiguration({ feedbackDepth: -1 }).depth, 3);
});

test('rest sphere surface is observed on the past light cone with its emission clock', () => {
    const hit = traceObserverRay(scene()); assert.equal(hit.entityId, 'a'); close(hit.distance, 7); close(hit.emissionTime, -7); close(hit.properTime, -7); close(hit.position[2], 1); close(hit.doppler, 1);
});
test('Lorentz boost and inverse preserve timelike, spacelike and null intervals', () => {
    for (const time of [-2, 0, 3]) for (const space of [[1, 2, 0], [0, 0, 2]]) {
        const velocity = [.3, -.4, .2], shifted = boost(time, space, velocity), restored = boost(shifted.time, shifted.space, velocity.map(v => -v));
        close(restored.time, time); restored.space.forEach((v, i) => close(v, space[i]));
        close(shifted.time ** 2 - shifted.space.reduce((s, v) => s + v * v, 0), time ** 2 - space.reduce((s, v) => s + v * v, 0));
    }
});
test('camera tetrad produces a null ray and transverse relativistic aberration', () => {
    const snapshot = scene(); snapshot.observer.velocity = [.6, 0, 0];
    const ray = observerRay(snapshot, {}); close(ray.direction[0], -.6); close(ray.direction[2], -.8); close(Math.hypot(...ray.direction), 1); close(ray.timeSlope, -1);
});
test('approaching and receding inertial spheres use rest-frame shape and longitudinal Doppler', () => {
    const approaching = traceObserverRay(scene([body({ velocity: [0, 0, .6] })]));
    close(approaching.distance, 18); close(approaching.doppler, 2); close(approaching.emissionTime, -18);
    const receding = traceObserverRay(scene([body({ velocity: [0, 0, -.6] })]));
    close(receding.distance, 4.5); close(receding.doppler, .5);
});
test('current-time sphere can miss the sight line while retained history is visible', () => {
    const segment = body({ position: [0, 1.6, 0], originTime: -8, velocity: [.6, 0, 0], size: [1, 1, 1] });
    const snapshot = scene([segment], { entities: [body({ position: [4.8, 1.6, 0], velocity: [.6, 0, 0], size: [1, 1, 1] })] });
    assert.ok(traceObserverRay(snapshot)); assert.equal(traceObserverRay(snapshot, { optical: false }), null);
});
test('deletion preserves the old far surface after the old near surface ceases to exist', () => {
    const segment = body({ end: -7.5 });
    const snapshot = scene([segment], { entities: [body({ alive: false, revision: 2 })] });
    const hit = traceObserverRay(snapshot); close(hit.distance, 9); assert.equal(hit.historical, true);
});
test('same-event revision boundaries are half-open and never show a removed revision at its end', () => {
    const old = body({ end: -7, revision: 1 }), current = body({ start: -7, revision: 2 });
    const hit = traceObserverRay(scene([old, current], { entities: [current] })); assert.equal(hit.revision, 2); close(hit.emissionTime, -7);
});
test('history gaps never substitute the current pose', () => {
    assert.equal(traceObserverRay(scene([body()], { historyStart: -6 })), null);
    assert.equal(traceObserverRay(scene([body({ start: -3 })])), null);
});
test('expired segment whose light cannot remain in history is removed from tracing', () => {
    const snapshot = scene([body({ end: -70 })]); assert.equal(visibleSegments(snapshot).length, 0);
});
test('simultaneous geometry and Playground are explicitly instantaneous', () => {
    close(traceObserverRay(scene(), { optical: false }).emissionTime, 0);
    close(traceObserverRay(scene([body()], { profile: 'playground' })).emissionTime, 0);
});
test('FoV changes only ray projection and partial cameraOverride preserves physical state', () => {
    const snapshot = scene(), original = structuredClone(snapshot);
    close(traceObserverRay(snapshot, { fov: 20 }).distance, traceObserverRay(snapshot, { fov: 110 }).distance);
    assert.ok(traceObserverRay(snapshot, { cameraOverride: { yaw: 0, pitch: 0 } })); assert.deepEqual(snapshot, original);
});
test('null-ray projection is inverse to generating the same observer ray', () => {
    const snapshot = scene(); snapshot.observer.velocity = [.3, .1, -.2]; snapshot.observer.yaw = .3; snapshot.observer.pitch = .15;
    for (const [x, y] of [[0, 0], [.4, -.3], [-.7, .1]]) {
        const ray = observerRay(snapshot, { aspect: 1.7, fov: 75 }, x, y), point = ray.origin.map((v, j) => v + ray.direction[j] * 5);
        const p = projectPoint(snapshot, { aspect: 1.7, fov: 75 }, point); close(p.x, (x + 1) / 2); close(p.y, (1 - y) / 2);
    }
});
test('retarded center solver obeys the emission null interval', () => {
    const segment = body({ velocity: [.6, .1, 0] }), snapshot = scene([segment]), event = apparentCenter(snapshot, {}, segment);
    close(Math.hypot(...event.position.map((v, i) => v - snapshot.observer.position[i])), -event.emissionTime);
});
test('SR comparison labels use the same observer simultaneity hyperplane as the ray tracer', () => {
    const segment = body({ position: [3, 1.6, 0], velocity: [.6, 0, 0] });
    const snapshot = scene([segment]); snapshot.observer.velocity = [.6, 0, 0];
    const settings = { optical: false }, event = apparentCenter(snapshot, settings, segment);
    close(event.position[0], 4.6875); close(event.emissionTime, 2.8125);
    const delta = event.position.map((v, i) => v - snapshot.observer.position[i]);
    close(event.emissionTime - snapshot.time, delta.reduce((sum, v, i) => sum + v * snapshot.observer.velocity[i], 0));
    const projected = projectPoint(snapshot, settings, event.position);
    close(projected.x, .5 + 3.75 / (16 * Math.tan(Math.PI / 6)));
    const hit = traceObserverRay(snapshot, settings, projected.x * 2 - 1, 1 - projected.y * 2);
    assert.equal(hit.entityId, segment.id);
    close(hit.doppler, 1);
    const projectedSurface = projectPoint(snapshot, settings, hit.position);
    close(projectedSurface.x, projected.x); close(projectedSurface.y, projected.y);
    const playground = apparentCenter({ ...snapshot, profile: 'playground' }, settings, segment);
    close(playground.position[0], 3); close(playground.emissionTime, 0);
});
test('rest-clock surface synchronization includes spatial Einstein offset', () => {
    const segment = body({ velocity: [0, 0, .6], clockOffset: 5 }), hit = traceObserverRay(scene([segment]));
    const expected = boost(hit.emissionTime, hit.position.map((v, i) => v - segment.position[i]), [0, 0, -.6]); close(hit.properTime, 5 + expected.time);
});
test('world floor occludes below-plane geometry for targeting too', () => {
    const snapshot = scene([body({ position: [0, -3, 0] })]); snapshot.observer.pitch = -.6;
    assert.equal(traceObserverRay(snapshot), null);
});
test('all sixteen primitives and instrument meshes have deterministic intersections', () => {
    for (const shape of SHAPE_NAMES) {
        if (shape === 'torus') { assert.ok(intersectShape(shape, [.35, 0, 2], [0, 0, -1])); continue; }
        const origin = ['plane', 'disk'].includes(shape) ? [0, 2, 0] : [0, 0, 2], direction = ['plane', 'disk'].includes(shape) ? [0, -1, 0] : [0, 0, -1];
        assert.ok(intersectShape(shape, origin, direction), shape);
    }
    assert.equal(restMesh('icosahedron').triangles.length, 20); assert.equal(restMesh('dodecahedron').triangles.length, 36);
});
test('rotation XYZ round trips preserve lengths and frame orientation', () => {
    const matrix = rotationMatrix([.3, .7, -.4]), vector = [1, 2, 3], roundtrip = inverseRotate(matrix, rotate(matrix, vector));
    roundtrip.forEach((v, i) => close(v, vector[i])); close(Math.hypot(...rotate(matrix, vector)), Math.hypot(...vector));
});
test('threaded BVH has complete leaves, valid escapes and zero-direction slab handling', () => {
    const bounds = Array.from({ length: 97 }, (_, index) => ({ min: [index, 0, 0], max: [index + .5, 1, 1], index })), bvh = buildBVH(bounds);
    assert.deepEqual([...bvh.order].sort((a, b) => a - b), bounds.map(b => b.index));
    bvh.nodes.forEach((node, i) => assert.ok(node.escape > i && node.escape <= bvh.nodes.length));
    assert.equal(hitBounds([0, .5, .5], [1, 0, 0], [0, 0, 0], [1, 1, 1]), true);
    assert.equal(hitBounds([0, 2, .5], [1, 0, 0], [0, 0, 0], [1, 1, 1]), false);
});
test('benchmark subdivision uses distinct surface-preserving triangles and complete BVH leaves', () => {
    const source = restMesh('tetrahedron'), refined = subdivideRestMesh(source, 1000);
    assert.equal(source.triangles.length, 4); assert.equal(refined.triangles.length, 1000);
    const area = mesh => mesh.triangles.reduce((sum, indices) => {
        const [a, b, c] = indices.map(i => mesh.vertices[i]);
        return sum + .5 * Math.hypot(...cross(b.map((value, j) => value - a[j]), c.map((value, j) => value - a[j])));
    }, 0);
    close(area(refined), area(source));
    assert.equal(new Set(refined.triangles.map(indices => [...indices].sort((a, b) => a - b).join(','))).size, 1000);
    assert.equal(new Set(refined.bvh.order).size, 1000);
    assert.deepEqual(refined.bvh.nodes[0].min, source.bvh.nodes[0].min);
    assert.deepEqual(refined.bvh.nodes[0].max, source.bvh.nodes[0].max);
    assert.equal(subdivideRestMesh(source, 1002).triangles.length, 1000);
    assert.throws(() => subdivideRestMesh(source, 100001), RangeError);
});
