import test from 'node:test';
import assert from 'node:assert/strict';
import { applySeed } from '../js/seeding/service.js';

const recipe = { version: 2, scenarioId: 'empty', size: 9, blank: false, randomSeed: 0, components: [], overrides: {} };
const scenarios = [{ id: 'empty' }];
function fixture() {
    let disposed = 0, loaded = 0;
    const owner = { ready: true, dispose() { disposed++; } };
    const ctx = { engineMode: 'lattice', _loadGeneration: 1, bridge: {}, pauseSimulation() {} };
    const options = { recipe, scenarios, owner, loadScenario() { loaded++; ctx._loadGeneration++; ctx.bridge = owner; },
        waitForLoad: async () => {} };
    return { owner, ctx, options, get disposed() { return disposed; }, get loaded() { return loaded; } };
}

test('headless seed service installs the staged owner and preserves the applied recipe', async () => {
    const f = fixture(), result = await applySeed(f.ctx, f.options);
    assert.equal(result.owner, f.owner); assert.equal(result.installedGeneration, 2);
    assert.equal(f.loaded, 1); assert.equal(f.disposed, 0);
    assert.deepEqual(f.ctx._appliedSeedRecipe, recipe);
    assert.notEqual(f.ctx._appliedSeedRecipe, recipe);
});

test('cancelled preparation cannot pause, replace or resume the existing owner', async () => {
    const f = fixture(), signal = new AbortController(); let paused = 0;
    f.ctx.pauseSimulation = () => paused++;
    await assert.rejects(applySeed(f.ctx, { ...f.options, owner: null, signal: signal.signal,
        async prepare() { signal.abort(); return f.owner; } }), /cancelled/);
    assert.equal(paused, 0); assert.equal(f.loaded, 0); assert.equal(f.disposed, 1);
});

test('a changed source after asynchronous preparation is rejected before installation', async () => {
    const f = fixture();
    await assert.rejects(applySeed(f.ctx, { ...f.options, owner: null,
        async prepare() { f.ctx._loadGeneration++; return f.owner; } }), /superseded/);
    assert.equal(f.loaded, 0); assert.equal(f.disposed, 1);
});

test('native commit consumes its source fence once; its own new epoch does not block local adoption', async () => {
    const f = fixture(); let committed = false, sourceChecks = 0;
    const preview = { isNativeSeedPreview: true, dispose() { throw new Error('committed owner must stay resident'); },
        async commit() { committed = true; f.ctx.bridge = f.owner; return f.owner; } };
    const result = await applySeed(f.ctx, { ...f.options, owner: preview,
        assertCurrent() { sourceChecks++; assert.equal(committed, false); } });
    assert.equal(result.owner, f.owner); assert.ok(sourceChecks >= 2); assert.equal(f.loaded, 1);
});

test('stop after native commit retains the live owner and suppresses later installation', async () => {
    const f = fixture(), signal = new AbortController();
    const preview = { isNativeSeedPreview: true, async commit() { f.ctx.bridge = f.owner; signal.abort(); return f.owner; } };
    await assert.rejects(applySeed(f.ctx, { ...f.options, owner: preview, signal: signal.signal }), /cancelled/);
    assert.equal(f.ctx.bridge, f.owner); assert.equal(f.disposed, 0); assert.equal(f.loaded, 0);
});

test('hidden lattice cannot be replaced by headless seeding', async () => {
    const f = fixture(); f.ctx.presentationSuspended = true;
    await assert.rejects(applySeed(f.ctx, f.options), /inactive/);
    assert.equal(f.loaded, 0); assert.equal(f.disposed, 1);
});
