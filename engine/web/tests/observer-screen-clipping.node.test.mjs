import test from 'node:test';
import assert from 'node:assert/strict';
import { clipProjectedSegment } from '../js/observer/screen-clipping.js';
import { projectPoint } from '../js/observer/optics.js';

const point = (x, y, depth = 1) => ({ depth, homogeneousX: x * depth, homogeneousY: y * depth });
const close = (actual, expected) => assert.ok(Math.abs(actual - expected) < 1e-10, `${actual} != ${expected}`);

test('on-screen chords are unchanged and both-offscreen crossings survive', () => {
    assert.deepEqual(clipProjectedSegment(point(.2, .3), point(.8, .7)), { a: { x: .2, y: .3 }, b: { x: .8, y: .7 } });
    assert.deepEqual(clipProjectedSegment(point(-2, .5), point(2, .5)), { a: { x: 0, y: .5 }, b: { x: 1, y: .5 } });
    const diagonal = clipProjectedSegment(point(-1, -1), point(2, 2));
    close(diagonal.a.x, 0); close(diagonal.a.y, 0); close(diagonal.b.x, 1); close(diagonal.b.y, 1);
});

test('one endpoint on or behind the camera plane clips to finite screen coordinates', () => {
    const onPlane = clipProjectedSegment({ depth: 0, homogeneousX: -.2, homogeneousY: 0 }, point(.75, .5, 3));
    assert.ok(onPlane); close(onPlane.a.x, 0); close(onPlane.b.x, .75);
    const behind = clipProjectedSegment(point(.5, .5, -1), point(.5, .5, 1));
    assert.ok(behind); close(behind.a.x, .5); close(behind.a.y, .5);
    for (const result of [onPlane, behind]) for (const endpoint of [result.a, result.b]) {
        assert.ok(Number.isFinite(endpoint.x) && Number.isFinite(endpoint.y));
        assert.ok(endpoint.x >= 0 && endpoint.x <= 1 && endpoint.y >= 0 && endpoint.y <= 1);
    }
});

test('outside, behind-camera and nonfinite chords do not produce spurious strokes', () => {
    assert.equal(clipProjectedSegment(point(-2, .2), point(-1, .9)), null);
    assert.equal(clipProjectedSegment(point(.5, .5, -1), point(.5, .5, -2)), null);
    assert.equal(clipProjectedSegment(point(Infinity, .5), point(.5, .5)), null);
    assert.equal(clipProjectedSegment(point(.1, .1), point(.9, .9), 0), null);
});

test('actual perspective projection supplies clipping coordinates at zero and negative depth', () => {
    const snapshot = { time: 0, profile: 'sr', historyStart: -60, entities: [], segments: [], observer: { position: [0, 0, 0], velocity: [0, 0, 0], yaw: 0, pitch: 0 } };
    for (const optical of [true, false]) {
        const a = projectPoint(snapshot, { optical, aspect: 1 }, [-3, 0, 0]);
        const b = projectPoint(snapshot, { optical, aspect: 1 }, [0, 0, -5]);
        assert.equal(a.visible, false); assert.equal(b.visible, true);
        assert.ok(Number.isFinite(a.homogeneousX)); assert.equal(a.depth, 0);
        const clipped = clipProjectedSegment(a, b);
        assert.ok(clipped); close(clipped.a.x, 0); close(clipped.b.x, .5); close(clipped.a.y, .5);
    }
});
