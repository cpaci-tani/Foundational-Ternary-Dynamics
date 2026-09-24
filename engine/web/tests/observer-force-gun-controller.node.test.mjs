/* global structuredClone */
import test from 'node:test';
import assert from 'node:assert/strict';
import { ObserverForceGun } from '../js/observer/force-gun.js';

const deferred = () => { let resolve; const promise = new Promise(r => { resolve = r; }); return { promise, resolve }; };
function fixture() {
    const snapshot = { epoch: 1, profile: 'playground', playing: false,
        observer: { position: [0, -2, 8], velocity: [0, 0, 0], yaw: 0, pitch: 0 },
        entities: [{ id: 'body', name: 'Body', alive: true, revision: 4, bodyType: 'dynamic', mass: 2, position: [0, 2, -4], rotation: [0, 0, 0] }] };
    const settings = { forceGunEnabled: true, forceGunSensitivity: 'normal', forceGunMultiplier: 1, cameraOverride: { yaw: 0, pitch: 0 } };
    const hit = { entityId: 'body', revision: 4, distance: 11.5, historical: false, mirrored: true, restPosition: [0, 0, .5] };
    let telemetry = null;
    const requests = [], statuses = [], replies = [];
    const gun = new ObserverForceGun({ getSnapshot: () => snapshot, getSettings: () => settings, getTelemetry: () => telemetry,
        command: command => { requests.push(command); const reply = deferred(); replies.push(reply); return reply.promise; },
        onStatus: message => statuses.push(message) });
    return { gun, snapshot, settings, hit, requests, statuses, replies, setTelemetry: value => { telemetry = value; } };
}

test('mirrored grabbing preserves the source rest anchor and reflects target/direction once', async () => {
    const f = fixture(), beginning = f.gun.begin(f.hit, 'pull');
    const begin = f.requests[0];
    assert.deepEqual(begin.localAnchor, f.hit.restPosition);
    assert.deepEqual(begin.target, [0, 2, -3.5]);
    assert.equal(begin.expectedTargetRevision, 4);
    f.snapshot.playing = true; f.setTelemetry({ token: begin.token, mass: 2, forceMagnitude: 7, maxForce: 20 });
    f.replies[0].resolve({ ok: true }); await beginning;
    assert.deepEqual(f.gun.anchor, [0, -2, -3.5]);
    assert.equal(f.gun.state.force, 7); assert.equal(f.gun.state.cap, 20);
    f.settings.cameraOverride.pitch = .3;
    const update = f.gun.sample();
    assert.ok(update.direction[1] < 0, 'looking up below the plane pulls the source down');
    assert.ok(update.target[1] < 2);
    assert.ok(update.sequence > begin.sequence);
    f.gun.end(); f.replies[1].resolve({ ok: true });
});

test('wheel changes only retained depth; live sensitivity is included in monotonic inputs', async () => {
    const f = fixture(), beginning = f.gun.begin(f.hit, 'push');
    f.snapshot.playing = true; f.setTelemetry({ token: f.requests[0].token });
    f.replies[0].resolve({ ok: true }); await beginning;
    const before = structuredClone(f.snapshot.observer), depth = f.gun.state.depth;
    assert.equal(f.gun.wheel(-400), true); assert.ok(f.gun.state.depth < depth);
    f.settings.forceGunSensitivity = 'strong'; f.settings.forceGunMultiplier = 2;
    const update = f.gun.sample();
    assert.equal(update.sensitivity, 'strong'); assert.equal(update.multiplier, 2);
    assert.deepEqual(f.snapshot.observer, before);
    for (let i = 0; i < 10; i++) f.gun.wheel(1e9);
    assert.equal(f.gun.state.depth, 1e6);
    f.gun.end(); f.replies[1].resolve({ ok: true });
    assert.equal(f.gun.wheel(100), false);
});

test('release before begin acknowledgement cannot revive a force or cancel a newer token', async () => {
    const f = fixture(), first = f.gun.begin(f.hit, 'pull'), oldToken = f.requests[0].token;
    f.gun.end();
    assert.equal(f.gun.state, null);
    const second = f.gun.begin(f.hit, 'push'), newToken = f.requests[2].token;
    assert.notEqual(newToken, oldToken);
    f.replies[0].resolve({ ok: true });
    await Promise.resolve();
    assert.equal(f.requests[3].type, 'gun-end'); assert.equal(f.requests[3].token, oldToken);
    f.replies[3].resolve({ ok: true }); await first;
    assert.equal(f.gun.held.token, newToken);
    f.replies[2].resolve({ ok: true }); await second;
    f.replies[1].resolve({ ok: true });
    f.gun.dispose(); f.replies[4].resolve({ ok: true });
});

test('pause, deletion, epoch changes and authoritative cancellation release the transient gesture', async () => {
    for (const mutate of [f => { f.snapshot.playing = false; }, f => { f.snapshot.entities[0].alive = false; },
        f => { f.snapshot.epoch++; }, f => { f.setTelemetry(null); }]) {
        const f = fixture(), beginning = f.gun.begin(f.hit, 'pull');
        f.snapshot.playing = true; f.setTelemetry({ token: f.requests[0].token });
        f.replies[0].resolve({ ok: true }); await beginning;
        mutate(f); assert.equal(f.gun.sample(), undefined); assert.equal(f.gun.held, null);
        assert.equal(f.requests[1].type, 'gun-end'); f.replies[1].resolve({ ok: true });
    }
});

test('unsupported profiles, fixed bodies, historical images and disabled gun do not issue commands', async () => {
    for (const mutate of [f => { f.snapshot.profile = 'sr'; }, f => { f.snapshot.entities[0].bodyType = 'fixed'; },
        f => { f.hit.historical = true; }, f => { f.settings.forceGunEnabled = false; }]) {
        const f = fixture(); mutate(f); await f.gun.begin(f.hit, 'pull');
        assert.equal(f.requests.length, 0); assert.equal(f.gun.held, null); assert.ok(f.statuses[0]);
    }
});
