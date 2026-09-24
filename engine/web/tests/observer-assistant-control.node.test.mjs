import test from 'node:test';
import assert from 'node:assert/strict';
import { Worker as NodeWorker } from 'node:worker_threads';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import { createObserverControl, OBSERVER_ACTION_DESCRIPTORS } from '../js/assistant/observer-control.js';
import { ObserverSession } from '../js/observer/session.js';
import { ObserverWorkerClient } from '../js/observer/worker-client.js';
import { DEFAULT_SETTINGS } from '../js/observer/catalog.js';
import { reflectAuthorPatch, reflectPolar } from '../js/observer/mirror-frame.js';

async function fixture(options = {}) {
    const session = new ObserverSession(options); await session.initialize();
    let active = true, calls = 0;
    const workspace = {
        snapshot: session.snapshot(), settings: structuredClone(DEFAULT_SETTINGS), selectedId: session.state.entities[0].id,
        selectedHit: null, hit: null, assistantViewVersion: 0, authorMirrored: false, disposed: false,
        forceGun: { state: null, end() {} }, input: { releaseForce() {}, setPose(pose) { this.pose = pose; } }, tether: { hide() {} },
        selectionKey: '',
        getAssistantPreparationVersion() {
            const next = JSON.stringify([this.selectedId, this.authorMirrored]);
            if (next !== this.selectionKey) { this.selectionKey = next; this.assistantViewVersion++; }
            return `${this.snapshot.epoch}:${this.snapshot.preparationVersion}:${this.assistantViewVersion}`;
        },
        setSetting(key, value) { this.settings[key] = value; this.assistantViewVersion++; }, updateUI() {},
        async authorCommand(command, options = {}) {
            options.assertActive?.(); calls++;
            const translated = structuredClone(command);
            if (this.authorMirrored && this.selectedId === command.id) {
                if (command.patch) translated.patch = reflectAuthorPatch(command.patch);
                if (command.impulse) translated.impulse = reflectPolar(command.impulse);
            }
            const result = await session.command(translated, { isCancelled: () => options.signal?.aborted });
            if (result.snapshot.epoch !== this.snapshot.epoch) { this.selectedHit = null; this.hit = null; this.authorMirrored = false; }
            if (!result.snapshot.entities.some(entity => entity.id === this.selectedId)) this.selectedId = null;
            this.snapshot = result.snapshot; return result;
        },
    };
    const control = createObserverControl({ getWorkspace: () => workspace, isActive: () => active });
    return { session, workspace, control, calls: () => calls, active: value => { active = value; },
        execute: (type, args = {}, expected = control.observe(), extra = {}) => control.execute({ type: `observer.${type}`, args }, { expected, ...extra }),
        dispose() { control.dispose(); session.dispose(); } };
}

test('assistant observations preserve current versus historical facts and publish closed schemas', async () => {
    const f = await fixture();
    f.workspace.hit = { entityId: f.workspace.selectedId, historical: true, mirrored: true, revision: 1, emissionTime: -3, position: [1, 2, 3] };
    f.workspace.selectedHit = f.workspace.hit;
    const seen = f.control.observe();
    assert.equal(seen.workspace, 'observer'); assert.equal(seen.facts.running, true);
    assert.equal(seen.selected.observed.historical, true); assert.equal(seen.selected.current.alive, true);
    assert.equal(seen.facts.crosshair.emissionTime, -3); assert.notDeepEqual(seen.selected.current.position, seen.selected.observed.observedPosition);
    assert.ok(!seen.capabilities.includes('observer.impulse'));
    assert.ok(OBSERVER_ACTION_DESCRIPTORS.every(action => action.args.additionalProperties === false));
    assert.deepEqual(JSON.parse(JSON.stringify(seen)), seen); f.dispose();
});

test('relative edits use a stable current target, with normal motion allowed and manual edits fenced', async () => {
    const f = await fixture(), expected = f.control.observe(), id = expected.selected.id;
    const originalMass = expected.selected.current.mass;
    for (let i = 0; i < 12; i++) f.session.advance(1 / 120);
    f.workspace.snapshot = f.session.snapshot();
    assert.equal(f.control.observe().preparationVersion, expected.preparationVersion);
    const result = await f.execute('update', { massFactor: 2, color: [1, 0, 0] }, expected);
    assert.equal(result.status, 'applied', result.error); assert.equal(result.after.selected.id, id);
    assert.equal(f.session.state.entities[0].mass, originalMass * 2);
    assert.deepEqual(f.session.state.entities[0].color, [1, 0, 0]);
    assert.equal((await f.execute('delete', {}, expected)).status, 'superseded');
    const latest = f.control.observe();
    await f.workspace.authorCommand({ type: 'update', id, patch: { name: 'Manual edit' } });
    assert.equal((await f.execute('update', { massFactor: 2 }, latest)).status, 'superseded');
    assert.equal(f.session.state.entities[0].mass, originalMass * 2); f.dispose();
});

test('SR motion revisions do not invalidate a captured author revision', async () => {
    const f = await fixture();
    const id = f.workspace.selectedId;
    await f.workspace.authorCommand({ type: 'update', id, patch: { shape: 'clock', properAcceleration: [0.1, 0, 0] } });
    const seen = f.control.observe(), oldRevision = f.session.state.entities[0].revision;
    f.session.advance(0.1); f.workspace.snapshot = f.session.snapshot();
    assert.ok(f.session.state.entities[0].revision > oldRevision);
    const result = await f.execute('update', { color: [0, 1, 0] }, seen);
    assert.equal(result.status, 'applied', result.error); f.dispose();
});

test('closed schemas, physical profile restrictions and captured lifecycle reject before dispatch', async () => {
    const f = await fixture(), baseline = f.session.snapshot();
    for (const [type, args] of [
        ['update', { patch: { mass: 2 } }], ['update', { mass: Infinity }], ['update', { velocity: [1, 0, 0] }],
        ['update', { angularVelocity: [0, 1, 0] }], ['update', { properAcceleration: [1, 0, 0] }],
        ['update', { mass: 2, massFactor: 2 }], ['world', { gravityStrength: 1 }],
        ['environment', { preset: 'https://untrusted.example/hdr' }], ['camera', { mode: 'pose', position: [0, 0, 0], fov: 70 }],
        ['overlay', { layer: 'axes', filter: 'all', enabled: true }],
    ]) assert.equal((await f.execute(type, args)).status, 'rejected', type);
    assert.equal(f.calls(), 0); assert.deepEqual(f.session.snapshot(), baseline);
    await f.workspace.authorCommand({ type: 'delete', id: f.workspace.selectedId });
    assert.equal((await f.execute('update', { color: [1, 0, 0] })).status, 'rejected');
    assert.equal((await f.execute('restore')).status, 'applied'); f.dispose();
});

test('mirrored authoring and impulse use the existing source-frame translation once', async () => {
    const f = await fixture({ profile: 'playground', playing: false });
    f.workspace.authorMirrored = true;
    const observed = f.control.observe();
    assert.deepEqual(observed.selected.author.position, reflectPolar(observed.selected.current.position));
    assert.equal((await f.execute('update', { position: [2, -3, 4], mass: 2 })).status, 'applied');
    assert.deepEqual(f.session.state.entities[0].position, [2, 3, 4]);
    const result = await f.execute('impulse', { impulse: [0, 2, 0] });
    assert.equal(result.status, 'applied', result.error);
    assert.ok(Math.abs(f.session.state.entities[0].velocity[1] + 1) < 1e-6); f.dispose();
});

test('bounded stepping advances exact local ticks, remains paused, and retains undo history', async () => {
    const f = await fixture({ playing: false });
    await f.execute('update', { color: [1, 0, 0] }); const undoDepth = f.session.undoStack.length;
    const result = await f.execute('step', { count: 3 });
    assert.equal(result.status, 'applied', result.error); assert.equal(result.after.tick, 3);
    assert.equal(result.after.facts.running, false); assert.equal(f.session.state.time, 3 / 120);
    assert.equal(f.session.undoStack.length, undoDepth);
    assert.equal((await f.execute('step', { count: 121 })).status, 'rejected');
    await f.execute('resume'); assert.equal((await f.execute('step')).status, 'rejected'); f.dispose();
});

test('receipt after does not adopt an intervening manual change or new owner', async () => {
    const f = await fixture(); const original = f.workspace.authorCommand.bind(f.workspace);
    f.workspace.authorCommand = async (...args) => {
        const result = await original(...args);
        await original({ type: 'update', id: f.workspace.selectedId, patch: { name: 'Intervening manual edit' } });
        return result;
    };
    const applied = await f.execute('update', { color: [1, 0, 0] });
    assert.equal(applied.status, 'applied'); assert.notEqual(applied.after.selected.current.name, 'Intervening manual edit');
    assert.equal((await f.execute('delete', {}, applied.after)).status, 'superseded');
    const seen = f.control.observe(); f.workspace.snapshot.sessionId = 'replacement-owner';
    assert.equal((await f.execute('pause', {}, seen)).status, 'superseded'); f.dispose();
});

test('abort, inactive workspace, and presentation changes prevent stale dispatch', async () => {
    const f = await fixture(), seen = f.control.observe(), abort = new AbortController(); abort.abort();
    assert.equal((await f.execute('delete', {}, seen, { signal: abort.signal })).status, 'superseded');
    f.workspace.setSetting('fov', 70);
    assert.equal((await f.execute('delete', {}, seen)).status, 'superseded');
    f.active(false); assert.equal(f.control.observe(), null);
    assert.equal((await f.execute('delete', {}, seen)).status, 'superseded');
    assert.equal(f.calls(), 0); f.dispose();
});

test('presentation operations use local settings; pose and world changes use authoritative owner', async () => {
    const f = await fixture({ profile: 'playground', playing: false });
    for (const [type, args] of [ ['forceGun', { sensitivity: 'delicate', multiplier: 0.5 }], ['overlay', { layer: 'clocks', enabled: true }], ['camera', { mode: 'view', fov: 90 }] ]) {
        const r = await f.execute(type, args); assert.equal(r.status, 'applied', r.error);
        assert.equal(r.after.preparationVersion, f.control.observe().preparationVersion);
    }
    assert.equal(f.calls(), 0);
    assert.equal((await f.execute('world', { gravityStrength: 2 })).status, 'applied');
    assert.equal((await f.execute('camera', { mode: 'pose', position: [0, 2, 5], yaw: 0.4 })).status, 'applied');
    assert.equal(f.workspace.input.pose.yaw, 0.4); assert.equal(f.session.state.observer.properTime, 0);
    assert.equal((await f.execute('create', { name: 'New body', shape: 'box' })).status, 'applied');
    assert.equal((await f.execute('environment', { preset: 'stars' })).status, 'applied');
    assert.equal((await f.execute('undo')).status, 'applied');
    const preset = await f.execute('preset', { preset: 'clocks' }); assert.equal(preset.status, 'applied');
    assert.equal(preset.after.preparationVersion, f.control.observe().preparationVersion);
    assert.equal((await f.execute('environment', { preset: 'void' }, preset.after)).status, 'applied'); f.dispose();
});

function workerFactory(url) {
    const source = `import {parentPort} from 'node:worker_threads';
        globalThis.postMessage=message=>parentPort.postMessage(message);
        globalThis.addEventListener=(name,fn)=>{if(name==='message')parentPort.on('message',data=>fn({data}));};
        await import(${JSON.stringify(url.href)});`;
    const worker = new NodeWorker(new URL(`data:text/javascript,${encodeURIComponent(source)}`));
    return { postMessage: data => worker.postMessage(data), terminate: () => worker.terminate(), addEventListener(type, listener) {
        if (type === 'message') worker.on('message', data => listener({ data }));
        if (type === 'error') worker.on('error', error => listener({ message: error.message }));
        if (type === 'messageerror') worker.on('messageerror', listener);
    } };
}
test('real worker acknowledges cancellation before mutation and keeps command receipts ordered', async () => {
    const client = new ObserverWorkerClient({ workerFactory }); await client.ready;
    try {
        const before = client.snapshot, abort = new AbortController();
        const request = client.request({ type: 'update', id: before.entities[0].id, patch: { name: 'Must not apply' }, expectedPreparationVersion: before.preparationVersion }, { signal: abort.signal });
        await Promise.resolve(); abort.abort();
        const result = await request;
        assert.equal(result.ok, false); assert.match(result.error, /cancelled/);
        assert.equal(client.snapshot.entities[0].name, before.entities[0].name);
        assert.equal(client.snapshot.preparationVersion, before.preparationVersion);
        const applied = await client.request({ type: 'pause', expectedPreparationVersion: before.preparationVersion });
        assert.equal(applied.ok, true); assert.equal(client.snapshot.playing, false);
    } finally { client.dispose(); }
});

test('async cancellation restores the author transaction and undo stack', async () => {
    const f = await fixture({ profile: 'playground', playing: false }), before = f.session.snapshot();
    const original = f.session.initializePlayground.bind(f.session); let cancel = false;
    f.session.initializePlayground = async () => { await original(); cancel = true; };
    const result = await f.session.command({ type: 'update', id: before.entities[0].id, patch: { mass: 3 } }, { isCancelled: () => cancel });
    assert.equal(result.ok, false); assert.deepEqual(f.session.snapshot(), before); assert.equal(f.session.undoStack.length, 0); f.dispose();
});

test('manual workspace settings cancel an already queued assistant command', async () => {
    // Exercise the real workspace methods without instantiating its WebGL UI.
    const source = readFileSync(new URL('../js/observer/workspace.js', import.meta.url), 'utf8');
    const Workspace = vm.runInNewContext(source.replace(/^import .*;\r?$/gm, '').replace('export class ObserverWorkspace', 'class ObserverWorkspace') + '\nObserverWorkspace;', { AbortController });
    const workspace = Object.create(Workspace.prototype), session = new ObserverSession();
    const before = session.snapshot(); let unblock;
    const barrier = new Promise(resolve => { unblock = resolve; });
    Object.assign(workspace, { disposed: false, snapshot: before, settings: structuredClone(DEFAULT_SETTINGS),
        assistantCommands: new Set(), assistantViewVersion: 0, input: { releaseForce() {} },
        updateUI() {}, setStatus() {}, receive(snapshot) { this.snapshot = snapshot; },
        client: { async request(command, options) { await barrier; return session.command(command, { isCancelled: () => options.signal.aborted }); } },
    });
    const pending = workspace.command({ type: 'update', id: before.entities[0].id, patch: { mass: 7 }, expectedPreparationVersion: before.preparationVersion }, { assertActive() {} });
    workspace.setSetting('fov', 75); unblock();
    const result = await pending;
    assert.equal(result.ok, false); assert.match(result.error, /cancelled/);
    assert.deepEqual(session.snapshot(), before); assert.equal(workspace.assistantCommands.size, 0);
    session.dispose();
});
