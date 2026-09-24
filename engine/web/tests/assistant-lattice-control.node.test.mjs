import test from 'node:test';
import assert from 'node:assert/strict';
import { setImmediate } from 'node:timers';
import { createLatticeControl } from '../js/assistant/lattice-control.js';
import { getScale0State, setLatticeNeedsUpload } from '../js/scales/scale0/state/store.js';

function fixture(extra = {}) {
    let tick = 0, writes = 0, active = true;
    const state = { mutationEpoch: 0, currentScenarioId: 'empty' };
    const owner = { isWorker: true, isWasm: true, ready: true, configurationToken: 1,
        latticeSize: 9, currentTick: () => tick, getDiagnostics: () => ({ tick, manifested: 0 }),
        getScale0TelemetryGroupMeta: () => ({ sourceEpoch: 1, sampleTick: tick, status: 'available' }),
        async executeScale0Control(command) { if (command.type === 'step') { tick++; writes++; } return { ok: true, tick }; } };
    const ctx = { bridge: owner, engineMode: 'lattice', _loadGeneration: 1, running: true,
        pauseSimulation() { this.running = false; }, togglePlay() { this.running = !this.running; } };
    const adapter = createLatticeControl({ getCtx: () => ctx, isActive: () => active,
        getState: () => state, getOwner: () => ctx.bridge, getQualification: () => ({}),
        getScenarios: () => [{ id: 'empty', name: 'Empty' }], ...extra });
    return { adapter, owner, ctx, state, get writes() { return writes; }, advance: () => tick++, hide: () => { active = false; } };
}

test('ordinary ticks do not stale plans; exact step receipts wait for each completion', async () => {
    const f = fixture(), before = f.adapter.observe(); f.advance();
    const receipt = await f.adapter.execute({ type: 'lattice.step', args: { count: 3 } }, { expected: before });
    assert.equal(f.writes, 3); assert.equal(receipt.completedTicks, 3);
    assert.equal(receipt.status, 'applied'); assert.equal(receipt.after.tick, '4');
    assert.equal(receipt.before.preparationVersion, receipt.after.preparationVersion);
    assert.equal(f.ctx.running, false); f.adapter.dispose();
});

test('native receipt clock advances without relabelling the older completed diagnostic sample', async () => {
    const f = fixture();
    Object.assign(f.owner, { isNativeGPU: true, scale0ControlVersion: 1, _nativeCompletedTick: '3',
        getTelemetrySnapshot: () => ({ sourceEpoch: '1', groups: { diagnostics: { tick: '0', manifested: 0 } },
            groupMeta: { diagnostics: { sourceEpoch: '1', sampleTick: '0', status: 'available' } } }),
        async executeScale0Control(command) {
            if (command.type === 'step') this._nativeCompletedTick = String(BigInt(this._nativeCompletedTick) + 1n);
            return { ok: true, tick: this._nativeCompletedTick };
        } });
    const before = f.adapter.observe();
    const receipt = await f.adapter.execute({ type: 'lattice.step', args: { count: 2 } }, { expected: before });
    assert.equal(receipt.after.tick, '5');
    assert.equal(receipt.after.facts.completedTick, '5');
    assert.equal(receipt.after.facts.sampleTick, '0');
    assert.equal(receipt.after.facts.tick, '0');
    assert.equal(receipt.after.preparationVersion, before.preparationVersion);
    f.adapter.dispose();
});

test('owner, source, preparation changes and hidden workspace reject without dispatch', async () => {
    for (const change of [f => { f.ctx.bridge = { ...f.owner }; }, f => { f.owner.configurationToken++; },
        f => { f.state.mutationEpoch++; }, f => { f.ctx._loadGeneration++; }, f => f.hide(),
        f => { f.ctx.presentationSuspended = true; }]) {
        const f = fixture(), expected = f.adapter.observe(); change(f);
        await assert.rejects(f.adapter.execute({ type: 'lattice.step', args: { count: 1 } }, { expected }), /changed/);
        assert.equal(f.writes, 0); f.adapter.dispose();
    }
});

test('stop during a step receipt prevents every later step and playback continuation', async () => {
    const f = fixture(), signal = new AbortController(); let steps = 0;
    setLatticeNeedsUpload(false); getScale0State().fieldNeedsUpdate = false;
    f.owner.executeScale0Control = async command => {
        if (command.type === 'step') { steps++; signal.abort(); }
        return { ok: true, tick: steps };
    };
    const receipt = await f.adapter.execute({ type: 'lattice.step', args: { count: 5 } }, { signal: signal.signal });
    assert.equal(receipt.status, 'unknown'); assert.equal(receipt.completedTicks, 1);
    assert.match(receipt.error, /1 confirmed ticks of 5/);
    assert.equal(getScale0State().latticeNeedsUpload, true, 'a stopped partial commit remains visible');
    assert.equal(getScale0State().fieldNeedsUpdate, true);
    assert.equal(steps, 1); assert.equal(f.ctx.running, false); f.adapter.dispose();
});

test('finite transport generation advances without invalidating preparation identity', async () => {
    const f = fixture();
    Object.assign(f.owner, { isFiniteRecord: true, ownerId: 'records-1', generation: '0', checkpointSHA256: 'seed-a',
        observation: { microtick: '0' },
        async advance(n) { this.observation = { microtick: String(BigInt(this.observation.microtick) + BigInt(n)) }; this.generation = String(BigInt(this.generation) + 1n); },
        setRecordQuantity(quantity) { this.quantity = quantity; } });
    const before = f.adapter.observe();
    const receipt = await f.adapter.execute({ type: 'lattice.step', args: { count: 2 } }, { expected: before });
    assert.equal(f.owner.generation, '2');
    assert.equal(receipt.after.preparationVersion, before.preparationVersion);
    assert.ok(!receipt.after.capabilities.includes('lattice.setToggle')); f.adapter.dispose();
});

test('closed arguments and backend support reject before any mutation', async () => {
    const f = fixture();
    await assert.rejects(f.adapter.execute({ type: 'lattice.step', args: { count: 1, script: 'evil' } }), /args/);
    await assert.rejects(f.adapter.execute({ type: 'lattice.step', args: { count: 0 } }), /args/);
    f.owner.isNativeGPU = true; f.owner.scale0ControlVersion = 0;
    assert.ok(!f.adapter.observe().capabilities.includes('lattice.step'));
    await assert.rejects(f.adapter.execute({ type: 'lattice.step', args: { count: 1 } }), /unavailable/);
    assert.equal(f.writes, 0); f.adapter.dispose();
});

test('a prepared seed completed after cancellation is disposed and never installed', async () => {
    const signal = new AbortController(); let disposed = 0, installed = 0;
    const f = fixture({ seedServices: async () => ({
        async prepareSeed() { signal.abort(); return { dispose() { disposed++; } }; },
        async applySeed() { installed++; },
    }) });
    f.ctx.bridge.seedRecipeVersion = 2;
    await assert.rejects(f.adapter.execute({ type: 'lattice.seed.preview', args: { recipeJson: '{}' } }, { signal: signal.signal }), /cancelled/);
    assert.equal(disposed, 1); assert.equal(installed, 0); f.adapter.dispose();
});

test('queued implicit tokens are captured at submission, never rebased after another edit', async () => {
    const f = fixture(); let release;
    f.owner.executeScale0Control = () => new Promise(resolve => { release = resolve; });
    const first = f.adapter.execute({ type: 'lattice.pause', args: {} });
    await new Promise(resolve => setImmediate(resolve));
    const second = f.adapter.execute({ type: 'lattice.resume', args: {} });
    f.state.mutationEpoch++; release({ ok: true });
    await assert.rejects(first, /changed/); await assert.rejects(second, /changed/);
    assert.equal(f.ctx.running, false); f.adapter.dispose();
});

test('native scenario replacement uses source-fenced seed orchestration instead of the raw loader', async () => {
    let applied = 0, rawLoads = 0;
    const f = fixture({ loadScenario() { rawLoads++; }, seedServices: async () => ({
        async applySeed(ctx, options) {
            applied++; assert.equal(options.recipe.scenarioId, 'empty'); assert.equal(options.recipe.size, 9);
            options.assertCurrent(); options.assertActive();
            return { owner: ctx.bridge, installedGeneration: ctx._loadGeneration };
        },
    }) });
    Object.assign(f.owner, { isNativeGPU: true, scale0ControlVersion: 1, seedRecipeVersion: 2 });
    const receipt = await f.adapter.execute({ type: 'lattice.scenario', args: { scenarioId: 'empty' } });
    assert.equal(receipt.status, 'applied'); assert.equal(applied, 1); assert.equal(rawLoads, 0); f.adapter.dispose();
});

test('seed description uses constructor descriptors without loading, ticking, or pausing the owner', async () => {
    const f = fixture({ seedDescriptions: async () => ({
        async describeNativeRecipe(ctx, recipe) {
            assert.equal(ctx, f.ctx); assert.equal(recipe.size, 9);
            return { properties: [
                { key: 'recipe.blank', path: ['overrides', 'recipe.blank'] },
                { key: 'ingredient.0.scenario', path: ['components', 0, 'overrides', 'scenario'] },
                { key: 'amplitude', path: ['overrides', 'amplitude'], type: 'real', min: 0, max: 10,
                    units: 'flux units', value: 1, default: 1, source: undefined },
                { key: 'ingredient.0.enabled', path: ['components', 0, 'enabled'], type: 'integer', min: 0, max: 1, value: 0, default: 1 },
            ] };
        },
    }) });
    f.owner.seedRecipeVersion = 2;
    f.owner.getDiagnostics = () => ({ tick: 0, manifested: 0, positive: 0, negative: 0, totalFlux: 0 });
    const before = f.adapter.observe();
    assert.ok(before.capabilities.includes('lattice.seed.describe'));
    const receipt = await f.adapter.execute({ type: 'lattice.seed.describe', args: { scenarioId: 'empty' } });
    const template = receipt.result.template;
    assert.equal(template.size, 9); assert.equal(template.backend, 'effective-lattice');
    assert.deepEqual(template.allowedMeasurements, ['manifested', 'positive', 'negative', 'totalFlux']);
    assert.equal(template.properties.length, 2); assert.equal(template.properties[0].source, undefined);
    assert.ok(!Object.hasOwn(template.properties[0], 'source'));
    assert.equal(template.properties[1].type, 'choice'); assert.equal(template.properties[1].value, false);
    assert.deepEqual(template.properties[1].options.map(row => row[0]), [false, true]);
    assert.match(template.measurementDefinitions.totalFlux.meaning, /Sum of/);
    assert.equal(receipt.after.ownerId, before.ownerId); assert.equal(receipt.after.preparationVersion, before.preparationVersion);
    assert.equal(f.ctx.running, true); assert.equal(f.writes, 0); f.adapter.dispose();
});

test('finite descriptions retain constructor recipes, reject silent size fallback, and omit immutable size settings', async () => {
    let reads = 0;
    const recipe = { version: 2, scenarioId: 'record-empty', size: 7, blank: false, randomSeed: 0, components: [] };
    const f = fixture({ getScenarios: () => [{ id: 'record-empty', title: 'Record empty', backend: 'finite-records', sizes: [3, 7] }],
        finiteCatalog: async () => ({ loadFiniteCatalog: async () => { reads++; return {}; },
            findFiniteSchema: () => ({ recipe, properties: [{ key: 'size', path: ['size'] },
                { key: 'randomSeed', path: ['randomSeed'], type: 'integer', min: 0, max: 10, value: 0 }] }) }) });
    f.owner.seedRecipeVersion = 2;
    await assert.rejects(f.adapter.execute({ type: 'lattice.seed.describe', args: { scenarioId: 'record-empty' } }), /does not support size 9/);
    assert.equal(reads, 0);
    const receipt = await f.adapter.execute({ type: 'lattice.seed.describe', args: { scenarioId: 'record-empty', size: 7 } });
    const template = receipt.result.template;
    assert.deepEqual(template.recipe, recipe); assert.equal(template.size, 7);
    assert.deepEqual(template.properties.map(row => row.key), ['randomSeed']);
    assert.ok(template.allowedMeasurements.includes('fieldTokens')); assert.ok(!template.allowedMeasurements.includes('dynamicEnergy'));
    assert.equal(f.ctx.running, true); assert.equal(f.writes, 0); f.adapter.dispose();
});

test('seed descriptions preserve backend lattice-size contracts for native even sizes and browser odd sizes', async () => {
    const described = [];
    const f = fixture({ seedDescriptions: async () => ({ async describeNativeRecipe(ctx, recipe) {
        described.push(recipe.size); return { properties: [] };
    } }) });
    Object.assign(f.owner, { isNativeGPU: true, scale0ControlVersion: 1, seedRecipeVersion: 2 });
    const native = await f.adapter.execute({ type: 'lattice.seed.describe', args: { scenarioId: 'empty', size: 32 } });
    assert.equal(native.result.template.size, 32);
    await assert.rejects(f.adapter.execute({ type: 'lattice.seed.describe', args: { scenarioId: 'empty', size: 3 } }), /native backend requires/);
    f.owner.isNativeGPU = false;
    await assert.rejects(f.adapter.execute({ type: 'lattice.seed.describe', args: { scenarioId: 'empty', size: 32 } }), /browser backend requires an odd/);
    const browser = await f.adapter.execute({ type: 'lattice.seed.describe', args: { scenarioId: 'empty', size: 3 } });
    assert.equal(browser.result.template.size, 3); assert.deepEqual(described, [32, 3]);
    assert.equal(f.writes, 0); f.adapter.dispose();
});

test('late seed descriptions fail cancellation and owner/preparation fences', async () => {
    for (const change of [f => f.state.mutationEpoch++, f => { f.ctx.bridge = { ...f.owner }; }, f => f.hide()]) {
        const f = fixture({ seedDescriptions: async () => ({ async describeNativeRecipe() { change(f); return { properties: [] }; } }) });
        f.owner.seedRecipeVersion = 2;
        await assert.rejects(f.adapter.execute({ type: 'lattice.seed.describe', args: { scenarioId: 'empty' } }), /changed/);
        assert.equal(f.writes, 0); assert.equal(f.ctx.running, true); f.adapter.dispose();
    }
    const signal = new AbortController();
    const f = fixture({ seedDescriptions: async () => ({ async describeNativeRecipe() { signal.abort(); return { properties: [] }; } }) });
    f.owner.seedRecipeVersion = 2;
    await assert.rejects(f.adapter.execute({ type: 'lattice.seed.describe', args: { scenarioId: 'empty' } }, { signal: signal.signal }), /cancelled/);
    assert.equal(f.writes, 0); f.adapter.dispose();
});

test('template metadata includes every registry row without paging and disappears for inactive owners', () => {
    const rows = Array.from({ length: 629 }, (_, i) => ({ id: `case-${i}`, title: `Case ${i}`, category: 'Controls' }));
    const f = fixture({ getScenarios: () => rows });
    const list = f.adapter.listScenarioTemplates();
    assert.equal(list.length, rows.length); assert.equal(list[628].label, 'Case 628');
    assert.deepEqual(list[0].sizes, [9]); list[0].sizes.push(99);
    assert.deepEqual(f.adapter.listScenarioTemplates()[0].sizes, [9]);
    f.hide(); assert.deepEqual(f.adapter.listScenarioTemplates(), []); f.adapter.dispose();
});

test('catalog coverage status remains supplied by the existing registry owner', () => {
    let status = Object.freeze({ status: 'loading', available: false, registeredCount: 0, reason: null });
    const f = fixture({ getCatalogStatus: () => status });
    assert.deepEqual(f.adapter.observe().facts.scenarioCatalogStatus, status);
    status = Object.freeze({ status: 'ready', available: true, registeredCount: 629, reason: null });
    assert.deepEqual(f.adapter.observe().facts.scenarioCatalogStatus, status);
    assert.equal(f.writes, 0); assert.equal(f.ctx.running, true); f.adapter.dispose();
});
