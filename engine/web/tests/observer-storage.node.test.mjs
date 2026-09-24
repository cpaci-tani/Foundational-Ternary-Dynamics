/* global structuredClone, TextEncoder, DOMException */
import test from 'node:test';
import assert from 'node:assert/strict';
import { ObserverStorage } from '../js/observer/storage.js';
import { ObserverSession } from '../js/observer/session.js';
import { DEFAULT_SETTINGS } from '../js/observer/catalog.js';
import { MAX_GRAVITY_STRENGTH } from '../js/observer/types.js';

function fixture(options = {}) {
    let now = 0;
    const records = new Map();
    let writes = 0;
    const backend = {
        async getAll() { return [...records.values()].map(record => structuredClone(record)); },
        async get(id) { return structuredClone(records.get(id)); },
        async put(record) { writes++; records.set(record.id, structuredClone(record)); },
        async delete(id) { writes++; records.delete(id); },
        async clear() { writes++; records.clear(); },
    };
    const storage = new ObserverStorage({ backend, clock: () => now, ...options });
    const session = new ObserverSession();
    const settings = structuredClone(DEFAULT_SETTINGS);
    return { storage, session, settings, backend, records, advance(ms) { now += ms; }, get writes() { return writes; } };
}

test('disabled autosave performs no writes and portable export has no persistence side effect', async () => {
    const f = fixture();
    f.advance(90000);
    assert.equal(await f.storage.maybeAutosave(f.session.snapshot(), f.settings), null);
    const text = f.storage.exportWorld(f.session.snapshot(), f.settings);
    assert.equal(f.writes, 0);
    assert.deepEqual(f.storage.importWorld(text), { snapshot: f.session.snapshot(), settings: f.settings });
    assert.equal(f.writes, 0);
});
test('immersion settings and fractal presets round-trip with bounded integer recursion', async () => {
    const f = fixture();
    for (const preset of ['fractal-julia', 'fractal-menger', 'fractal-kaleidoscope',
        'fractal-mandelbulb', 'fractal-mandelbox', 'fractal-sierpinski',
        'fractal-apollonian', 'fractal-julia-quaternion', 'fractal-kleinian']) {
        await f.session.command({ type: 'environment', patch: { preset, animationRate: 0.35 } });
        const text = f.storage.exportWorld(f.session.snapshot(), { ...f.settings, feedbackDepth: 5, feedbackLayers: 3, fractalDetail: 0.8 });
        const restored = f.storage.importWorld(text);
        assert.equal(restored.snapshot.environment.preset, preset);
        assert.equal(restored.settings.grounded, false);
        assert.equal(restored.settings.mirrorWorld, true);
        assert.equal(restored.settings.feedbackDepth, 5);
        assert.equal(restored.settings.fractalDetail, 0.8);
    }
    for (const patch of [{ feedbackDepth: 6 }, { feedbackDepth: 3.5 }, { feedbackLayers: 4 }, { feedbackLayers: 1.5 }, { feedbackScale: 0 }, { feedbackStrength: 2 }, { mirrorWorld: 'yes' }, { feedbackEnabled: 1 },
        { fractalDetail: -0.01 }, { fractalDetail: 1.01 }, { fractalDetail: Infinity }, { fractalDetail: 'high' }]) {
        assert.throws(() => f.storage.exportWorld(f.session.snapshot(), { ...f.settings, ...patch }), /Invalid Observer world/);
    }
    const legacy = { ...f.settings, grounded: true };
    for (const fractalDetail of [0, 1]) {
        assert.equal(f.storage.importWorld(f.storage.exportWorld(f.session.snapshot(), { ...f.settings, fractalDetail })).settings.fractalDetail, fractalDetail);
    }
    for (const key of ['mirrorWorld','feedbackEnabled','feedbackLayers','feedbackDepth','feedbackScale','feedbackStrength','fractalDetail']) delete legacy[key];
    assert.equal(f.storage.importWorld(f.storage.exportWorld(f.session.snapshot(), legacy)).settings.grounded, true);
    assert.equal(f.storage.importWorld(f.storage.exportWorld(f.session.snapshot(), legacy)).settings.fractalDetail, undefined);
});

test('force gun preferences round-trip, reject unsafe ranges, and accept legacy worlds', () => {
    const f = fixture();
    for (const forceGunSensitivity of ['delicate', 'normal', 'strong']) {
        const settings = { ...f.settings, forceGunSensitivity, forceGunMultiplier: 2.5, forceGunEnabled: false };
        const restored = f.storage.importWorld(f.storage.exportWorld(f.session.snapshot(), settings));
        assert.equal(restored.settings.forceGunSensitivity, forceGunSensitivity);
        assert.equal(restored.settings.forceGunMultiplier, 2.5);
        assert.equal(restored.settings.forceGunEnabled, false);
    }
    for (const patch of [{ forceGunMultiplier: 0 }, { forceGunMultiplier: 10.01 }, { forceGunMultiplier: Infinity }, { forceGunSensitivity: 'instant' }, { forceGunEnabled: 'yes' }]) {
        assert.throws(() => f.storage.exportWorld(f.session.snapshot(), { ...f.settings, ...patch }), /Invalid Observer world/);
    }
    const legacy = { ...f.settings };
    for (const key of ['forceGunSensitivity', 'forceGunMultiplier', 'forceGunEnabled']) delete legacy[key];
    assert.equal(f.storage.importWorld(f.storage.exportWorld(f.session.snapshot(), legacy)).settings.forceGunEnabled, undefined);
    f.storage.dispose(); f.session.dispose();
});

test('named worlds round-trip full history and local operations account storage', async () => {
    const f = fixture();
    const snapshot = f.session.snapshot();
    const saved = await f.storage.save('An authored frame', snapshot, f.settings);
    assert.equal((await f.storage.list())[0].name, 'An authored frame');
    assert.deepEqual(await f.storage.load(saved.id), { snapshot, settings: f.settings });
    assert.equal((await f.storage.usage()).bytes, saved.bytes);
    assert.ok(saved.bytes > 0);
    await f.storage.remove(saved.id);
    assert.equal((await f.storage.list()).length, 0);
    await assert.rejects(f.storage.load(saved.id), /no longer available/);
});

test('world gravity and collision flags round-trip and reject malformed portable settings', () => {
    const f = fixture();
    const snapshot = f.session.snapshot();
    Object.assign(snapshot, { gravityMode: 'plane', gravityStrength: 4.5, objectCollisions: false, planeCollision: false });
    snapshot.entities[0].collisions = false;
    const restored = f.storage.importWorld(f.storage.exportWorld(snapshot, f.settings));
    assert.equal(restored.snapshot.gravityStrength, 4.5);
    assert.equal(restored.snapshot.gravityMode, 'plane');
    assert.equal(restored.snapshot.objectCollisions, false);
    assert.equal(restored.snapshot.planeCollision, false);
    assert.equal(restored.snapshot.entities[0].collisions, false);
    for (const patch of [{ gravityMode: 'planet' }, { gravityStrength: -1 }, { gravityStrength: MAX_GRAVITY_STRENGTH + 1 }, { objectCollisions: 'false' }, { planeCollision: 1 }, { gravity: [0, 1] }]) {
        assert.throws(() => f.storage.exportWorld({ ...snapshot, ...patch }, f.settings), /Invalid Observer world/);
    }
    const invalid = structuredClone(snapshot); invalid.entities[0].collisions = 'off';
    assert.throws(() => f.storage.exportWorld(invalid, f.settings), /Invalid Observer world/);
    const legacy = structuredClone(snapshot);
    for (const key of ['gravityMode', 'gravityStrength', 'objectCollisions', 'planeCollision']) delete legacy[key];
    for (const entity of legacy.entities) delete entity.collisions;
    assert.equal(f.storage.importWorld(f.storage.exportWorld(legacy, f.settings)).snapshot.gravityMode, undefined);
    f.storage.dispose(); f.session.dispose();
});

test('autosave uses two rotating slots, waits 30 seconds, skips unchanged content, and preserves named saves', async () => {
    const f = fixture();
    const first = f.session.snapshot();
    const named = await f.storage.save('Keep forever', first, f.settings);
    await f.storage.setAutosave(true);
    f.advance(29999);
    assert.equal(await f.storage.maybeAutosave(first, f.settings), null);
    f.advance(1);
    assert.equal((await f.storage.maybeAutosave(first, f.settings)).id, 'autosave-0');
    f.advance(30000);
    assert.equal(await f.storage.maybeAutosave(first, f.settings), null);
    const second = structuredClone(first); second.revision++;
    f.advance(30000);
    assert.equal((await f.storage.maybeAutosave(second, f.settings)).id, 'autosave-1');
    const third = structuredClone(second); third.revision++;
    f.advance(30000);
    assert.equal((await f.storage.maybeAutosave(third, f.settings)).id, 'autosave-0');
    assert.equal((await f.storage.list()).length, 3);
    assert.deepEqual((await f.storage.load(named.id)).snapshot, first);
    assert.equal((await f.storage.load('autosave-0')).snapshot.revision, third.revision);
    await f.storage.setAutosave(false);
    const before = f.writes;
    f.advance(30000);
    await f.storage.maybeAutosave(first, f.settings);
    assert.equal(f.writes, before);
});

test('cap rejection keeps named saves intact and browser quota errors explain recovery', async () => {
    const f = fixture();
    const snapshot = f.session.snapshot();
    const bytes = new TextEncoder().encode(f.storage.exportWorld(snapshot, f.settings)).byteLength;
    f.storage.storageCapBytes = bytes + 10;
    const saved = await f.storage.save('Only save', snapshot, f.settings);
    await assert.rejects(f.storage.save('Over limit', snapshot, f.settings), /limit reached/);
    assert.deepEqual((await f.storage.list()).map(item => item.id), [saved.id]);
    f.storage.storageCapBytes = bytes * 3;
    f.backend.put = async () => { throw new DOMException('No space', 'QuotaExceededError'); };
    await assert.rejects(f.storage.save('Browser quota', snapshot, f.settings), /quota exceeded/);
    assert.equal((await f.storage.list()).length, 1);
});

test('concurrent saves cannot exceed a storage cap through a stale usage read', async () => {
    const f = fixture();
    const snapshot = f.session.snapshot();
    f.storage.storageCapBytes = new TextEncoder().encode(f.storage.exportWorld(snapshot, f.settings)).byteLength + 10;
    const results = await Promise.allSettled([f.storage.save('One', snapshot, f.settings), f.storage.save('Two', snapshot, f.settings)]);
    assert.equal(results.filter(result => result.status === 'fulfilled').length, 1);
    assert.equal((await f.storage.list()).length, 1);
});

test('changing the cap preserves named records and a failed autosave switches itself off', async () => {
    const f = fixture();
    const snapshot = f.session.snapshot();
    const named = await f.storage.save('Retain this preparation', snapshot, f.settings);
    f.storage.setStorageCap(1048576);
    assert.deepEqual((await f.storage.load(named.id)).snapshot, snapshot);
    assert.throws(() => f.storage.setStorageCap(0), /between 1 and 1024/);
    assert.throws(() => f.storage.setStorageCap(Infinity), /between 1 and 1024/);
    await f.storage.setAutosave(true);
    f.backend.put = async () => { throw new DOMException('No space', 'QuotaExceededError'); };
    f.advance(30000);
    await assert.rejects(f.storage.maybeAutosave(snapshot, f.settings), /quota exceeded/);
    assert.equal(f.storage.autosave, false);
    f.advance(30000);
    assert.equal(await f.storage.maybeAutosave(snapshot, f.settings), null);
    assert.deepEqual((await f.storage.list()).map(item => item.id), [named.id]);
});

test('imports reject malformed versions, unsafe prototype keys, non-finite values, oversized data and entity limits', () => {
    const f = fixture();
    const text = f.storage.exportWorld(f.session.snapshot(), f.settings);
    const change = mutate => { const document = JSON.parse(text); mutate(document); return JSON.stringify(document); };
    assert.throws(() => f.storage.importWorld(change(d => { d.schemaVersion = 2; })), /version/);
    assert.throws(() => f.storage.importWorld(text.replace('"format":', '"__proto__":{},"format":')), /unsafe property/);
    assert.throws(() => f.storage.importWorld(change(d => { d.snapshot.observer.position[0] = { $observerNumber: 'Infinity' }; })), /non-finite/);
    assert.throws(() => f.storage.importWorld(change(d => { d.snapshot.entities.push(structuredClone(d.snapshot.entities[0])); })), /duplicate/);
    assert.throws(() => f.storage.importWorld(change(d => { d.snapshot.entities = Array(257).fill(d.snapshot.entities[0]); })), /entity limit/);
    assert.throws(() => f.storage.importWorld(change(d => { d.snapshot.environment.preset = 'javascript:bad'; })), /environment/);
    assert.throws(() => f.storage.importWorld(' '.repeat(32 * 1024 * 1024 + 1)), /32 MiB/);
    const bad = f.session.snapshot(); bad.observer.position[0] = NaN;
    assert.throws(() => f.storage.exportWorld(bad, f.settings), /non-finite/);
});

test('open-ended history survives portable infinity markers without admitting infinity in ordinary fields', () => {
    const f = fixture();
    const snapshot = f.session.snapshot();
    snapshot.segments[0].end = Infinity;
    const text = f.storage.exportWorld(snapshot, f.settings);
    assert.match(text, /\$observerNumber/);
    assert.equal(f.storage.importWorld(text).snapshot.segments[0].end, null);
    snapshot.segments[0].start = Infinity;
    assert.throws(() => f.storage.exportWorld(snapshot, f.settings), /non-finite/);
});

test('clear is explicit and disposal stops future persistence', async () => {
    const f = fixture();
    await f.storage.save('Remove me', f.session.snapshot(), f.settings);
    await f.storage.clear();
    assert.equal((await f.storage.usage()).bytes, 0);
    f.storage.dispose();
    assert.equal(await f.storage.maybeAutosave(f.session.snapshot(), f.settings), null);
    await assert.rejects(f.storage.save('Too late', f.session.snapshot(), f.settings), /disposed/);
});

test('wave presentation settings round trip with bounded numeric values and known polarization', () => {
    const f = fixture();
    const settings = { ...f.settings, waveWavelength: 0.5, waveSeparation: 12, waveAmplitude: 2, wavePhase: Math.PI, polarizationMode: 'elliptical' };
    const document = JSON.parse(f.storage.exportWorld(f.session.snapshot(), settings));
    assert.equal(f.storage.importWorld(JSON.stringify(document)).settings.polarizationMode, 'elliptical');
    assert.equal(f.storage.importWorld(JSON.stringify(document)).settings.wavePhase, Math.PI);
    for (const [key, value] of [['waveWavelength', 0], ['waveSeparation', -1], ['waveAmplitude', 2.1], ['wavePhase', 4], ['polarizationMode', 'script']]) {
        const bad = structuredClone(document); bad.settings[key] = value;
        assert.throws(() => f.storage.importWorld(JSON.stringify(bad)), /wave|polarization/);
    }
    const legacy = structuredClone(document);
    for (const key of ['waveWavelength', 'waveSeparation', 'waveAmplitude', 'wavePhase', 'polarizationMode']) delete legacy.settings[key];
    assert.doesNotThrow(() => f.storage.importWorld(JSON.stringify(legacy)));
});
