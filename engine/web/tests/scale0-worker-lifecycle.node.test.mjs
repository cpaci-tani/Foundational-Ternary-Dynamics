import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const testsDir = path.dirname(fileURLToPath(import.meta.url));
const webRoot = path.resolve(testsDir, '..');

class FakeWorker {
    static instances = [];

    constructor() {
        this.messages = [];
        this.onmessage = null;
        this.onerror = null;
        this.terminateCount = 0;
        FakeWorker.instances.push(this);
    }

    postMessage(message) { this.messages.push(message); }
    terminate() { this.terminateCount++; }
    emit(data) { this.onmessage?.({ data }); }
}

function identity() {
    return {
        bundleSha256: 'a'.repeat(64),
        source: { commit: 'b'.repeat(40), dirty: false },
        variant: { id: 'wasm32-threads' },
    };
}

function readyPacket(token, generation, N = 33) {
    const ctrl = new SharedArrayBuffer(8 * 4);
    const heap = new SharedArrayBuffer(N * N * N * 8);
    return {
        type: 'ready',
        configurationToken: token,
        N,
        ctrl,
        heap,
        fluxPtr: 0,
        fluxLen: N * N * N,
        setupOk: true,
        artifactIdentity: identity(),
        workerRuntimeId: 'runtime-one',
        moduleInitCount: 1,
        renderBridgeGeneration: generation,
    };
}

function framePacket(token, tick, energy) {
    return {
        type: 'frame',
        configurationToken: token,
        dataVersion: tick,
        diag: { tick, dynamicEnergy: energy },
        diagMeta: {
            sourceEpoch: token,
            stateVersion: tick,
            status: 'available',
            stale: false,
            tick,
        },
        parts: { count: 0 },
        samplers: {
            'gravity@2': { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 },
        },
        engineToggles: { wave_propagation: true },
    };
}

test('proxy reuses one worker/module while fencing every RenderBridge generation', async () => {
    FakeWorker.instances.length = 0;
    globalThis.Worker = FakeWorker;

    const proxyUrl = pathToFileURL(path.join(webRoot, 'js', 'bridge', 'wasm-bridge-proxy.js'));
    proxyUrl.searchParams.set('node-lifecycle-test', String(Date.now()));
    const { WasmBridgeProxy } = await import(proxyUrl.href);

    const callbacks = [];
    const proxy = new WasmBridgeProxy(33);
    const worker = FakeWorker.instances[0];
    assert.equal(FakeWorker.instances.length, 1);

    proxy.replaceSamplerWants('gravity-panel', ['gravity@2']);
    const standingWantMessages = worker.messages.filter((m) => m.type === 'replaceSamplerWants').length;
    const token1 = proxy.beginConfiguration({
        latticeSize: 33,
        scenarioId: 'empty',
        onSetupFailure: (message) => callbacks.push(['setup-1', message]),
        onConfigurationApplied: (ack) => callbacks.push(['applied-1', ack.configurationToken]),
    });
    proxy.setToggle('wave_propagation', true);
    assert.equal(proxy.setupScenario('empty'), true);
    await Promise.resolve();
    assert.equal(worker.messages.filter((message) => message.type === 'create').length, 1);
    worker.emit(readyPacket(token1, 1));
    worker.emit(framePacket(token1, 0, 11));

    assert.equal(proxy.capabilities.scale0.getScale0Diagnostics(), null,
        'pre-barrier diagnostics must remain unavailable');
    assert.equal(proxy.getFluxVolume().length, 0,
        'pre-barrier flux must remain unavailable');

    worker.emit({
        type: 'configurationApplied',
        configurationToken: token1,
        ok: true,
        errors: [],
        engineToggles: { wave_propagation: true },
    });
    assert.equal(proxy.capabilities.scale0.getScale0Diagnostics()?.dynamicEnergy, 11);
    assert.equal(proxy.lifecycleDebug.workerRuntimeId, 'runtime-one');
    assert.equal(proxy.lifecycleDebug.moduleInitCount, 1);
    assert.equal(proxy.lifecycleDebug.renderBridgeGeneration, 1);

    const supersededToken = proxy.beginConfiguration({
        latticeSize: 35,
        scenarioId: 'flux-dipole',
    });
    proxy.setToggle('wave_propagation', true);
    proxy.setupScenario('flux-dipole');
    const token2 = proxy.beginConfiguration({
        latticeSize: 35,
        scenarioId: 'flux-pulse',
        onSetupFailure: (message) => callbacks.push(['setup-2', message]),
        onConfigurationApplied: (ack) => callbacks.push(['applied-2', ack.configurationToken]),
    });
    assert.equal(FakeWorker.instances.length, 1, 'configuration must not allocate a Worker');
    assert.equal(proxy.capabilities.scale0.getScale0Diagnostics(), null,
        'new generation clears old scientific cache');
    assert.equal(worker.messages.filter((m) => m.type === 'replaceSamplerWants').length,
        standingWantMessages, 'standing sampler demand survives reconfiguration');

    proxy.setToggle('wave_propagation', true);
    proxy.setupScenario('flux-pulse');
    await Promise.resolve();
    const createMessages = worker.messages.filter((message) => message.type === 'create');
    assert.equal(createMessages.length, 2,
        'two same-turn configurations publish only their final create');
    assert.equal(createMessages.at(-1).configurationToken, token2);
    assert.notEqual(createMessages.at(-1).configurationToken, supersededToken);
    worker.emit(framePacket(token1, 99, 999));
    worker.emit({
        type: 'error', where: 'setupScenario', msg: 'stale failure',
        configurationToken: token1,
    });
    worker.emit({
        type: 'configurationApplied', configurationToken: token1,
        ok: true, errors: [], engineToggles: { wave_propagation: false },
    });
    assert.equal(proxy.capabilities.scale0.getScale0Diagnostics(), null,
        'stale frame/config acknowledgement cannot repopulate caches');
    assert.equal(callbacks.some(([kind]) => kind === 'setup-2'), false,
        'stale error cannot reach current setup callback');

    worker.emit(readyPacket(token2, 2, 35));
    worker.emit(framePacket(token2, 0, 22));
    worker.emit({
        type: 'configurationApplied',
        configurationToken: token2,
        ok: true,
        errors: [],
        engineToggles: { wave_propagation: true },
    });
    assert.equal(proxy.capabilities.scale0.getScale0Diagnostics()?.dynamicEnergy, 22);
    assert.equal(proxy.lifecycleDebug.workerRuntimeId, 'runtime-one');
    assert.equal(proxy.lifecycleDebug.moduleInitCount, 1);
    assert.equal(proxy.lifecycleDebug.renderBridgeGeneration, 2);
    assert.deepEqual(callbacks.filter(([kind]) => kind.startsWith('applied')),
        [['applied-1', token1], ['applied-2', token2]]);

    proxy.dispose();
    const disposeMessage = worker.messages.findLast((m) => m.type === 'dispose');
    assert.equal(disposeMessage.configurationToken, token2);
    assert.equal(worker.terminateCount, 0, 'proxy waits for disposal acknowledgement');
    worker.emit({ type: 'disposed', configurationToken: token2, workerRuntimeId: 'runtime-one' });
    assert.equal(worker.terminateCount, 1, 'acknowledgement finalizes the Worker exactly once');
    assert.equal(proxy.canReconfigure(), false);
});

test('source contract pins token fences, lifecycle observability, and cache generations', () => {
    const proxy = fs.readFileSync(path.join(webRoot, 'js', 'bridge', 'wasm-bridge-proxy.js'), 'utf8');
    const worker = fs.readFileSync(path.join(webRoot, 'js', 'bridge', 'wasm-bridge.worker.js'), 'utf8');
    const loader = fs.readFileSync(path.join(webRoot, 'js', 'scales', 'scale0', 'runtime', 'scenario-loader.js'), 'utf8');
    const controller = fs.readFileSync(path.join(webRoot, 'js', 'scales', 'scale0', 'controller.js'), 'utf8');
    const app = fs.readFileSync(path.join(webRoot, 'js', 'app.js'), 'utf8');
    const bridgeBoot = fs.readFileSync(path.join(webRoot, 'js', 'app-wire', 'bridge-boot.js'), 'utf8');
    const bridgeInit = fs.readFileSync(path.join(webRoot, 'js', 'bridge-init.js'), 'utf8');
    const gravityPanel = fs.readFileSync(path.join(webRoot, 'js', 'scales', 'scale0', 'ui', 'overlays', 'gravity-panel.js'), 'utf8');
    const viewport = fs.readFileSync(path.join(webRoot, 'js', 'viewport.js'), 'utf8');
    const index = fs.readFileSync(path.join(webRoot, 'index.html'), 'utf8');

    assert.match(proxy, /beginConfiguration\(opts = \{\}\)/);
    assert.match(proxy, /disposeAcknowledged/);
    assert.match(proxy, /hardTerminated/);
    assert.match(proxy, /CONFIGURATION_APPLIED_TIMEOUT_MS/);
    assert.match(proxy, /queueMicrotask\(\(\) =>/);
    assert.match(proxy, /configurationToken: this\._pendingConfigurationToken/);
    assert.match(worker, /moduleInitCount\+\+/);
    assert.match(worker, /renderBridgeGeneration\+\+/);
    assert.match(worker, /type: 'disposed'/);
    assert.match(worker, /Number\(msg\.configurationToken\) !== activeConfigurationToken/);
    assert.match(loader, /priorWorker\?\.isWorker && priorWorker\?\.canReconfigure\?\.\(\)/);
    assert.match(loader, /wasm-bridge-proxy\.js\?v=5/);
    assert.match(controller, /scenario-loader\.js\?v=17/);
    assert.match(controller, /bindings\.js\?v=11/);
    assert.match(controller, /limits\.js\?v=2/);
    assert.match(controller, /gravity-panel\.js\?v=6/);
    assert.match(app, /viewport\.js\?v=9/);
    assert.match(app, /scale0\/controller\.js\?v=35/);
    assert.match(app, /gravity-panel\.js\?v=6/);
    assert.match(app, /app-shell\.js\?v=4/);
    assert.match(app, /bridge-boot\.js\?v=3/);
    assert.match(bridgeBoot, /bridge-init\.js\?v=3/);
    assert.match(bridgeBoot, /ws-bridge\.js\?v=4/);
    assert.match(bridgeInit, /ws-bridge\.js\?v=4/);
    assert.match(gravityPanel, /gravity-analysis\.js\?v=4/);
    assert.match(viewport, /flux-renderer\.js\?v=5/);
    assert.match(index, /app\.js\?v=61/);
    assert.match(index, /capabilities\/scale0\.js\?v=3/);
    assert.match(index, /sampler-want-set\.js\?v=2/);
    assert.match(index, /wasm-bridge\.js\?v=4/);
    assert.match(index, /ws-bridge\.js\?v=4/);
    assert.match(index, /state\/store\.js\?v=5/);
});
