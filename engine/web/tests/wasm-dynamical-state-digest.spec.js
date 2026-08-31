// @ts-check
/**
 * Focused publication contract for the canonical Scale-0 dynamical digest.
 *
 * The checked-in WASM binaries are intentionally not rewritten by this test.
 * Static assertions pin the Embind/worker plumbing, while browser-side fakes
 * exercise the direct wrapper and worker proxy without duplicating or
 * reimplementing the production C++ hash.
 */
import { test, expect } from '@playwright/test';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const webRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const engineRoot = path.resolve(webRoot, '..');

const RAW_EMPTY_DIGEST = Object.freeze({
    schema_version: 1,
    lattice_size: 9,
    site_count: 729,
    tick: 0,
    state_version: 0,
    hash_lo: '00000000000000af',
    hash_hi: 'fedcba9876543210',
    nonfinite_value_count: 0,
    nondefault_value_count: 0,
    device_to_host_bytes: 0,
    full_mirror_calls: 0,
    exact_default_record: true,
});

const EMPTY_DIGEST = Object.freeze({
    schemaVersion: 1,
    latticeSize: 9,
    siteCount: 729,
    tick: 0,
    stateVersion: 0,
    sourceEpoch: null,
    telemetrySourceEpoch: null,
    hashLo: '00000000000000af',
    hashHi: 'fedcba9876543210',
    nonfiniteValueCount: 0,
    nondefaultValueCount: 0,
    deviceToHostBytes: 0,
    fullMirrorCalls: 0,
    exactDefaultRecord: true,
    compute: 'CPU',
    runtime: 'wasm',
    transport: 'direct',
});

test('Embind publishes the shared production digest with lossless hash lanes', () => {
    const src = fs.readFileSync(
        path.join(engineRoot, 'wasm', 'bindings_render_bridge.cpp'),
        'utf8',
    );

    expect(src).toContain('rb.capture_dynamical_state_digest(digest)');
    expect(src).toContain('function("captureDynamicalStateDigest"');
    expect(src).toMatch(/result\.set\("hash_lo",\s*uint64_hex\(digest\.hash_lo\)\)/);
    expect(src).toMatch(/result\.set\("hash_hi",\s*uint64_hex\(digest\.hash_hi\)\)/);
    expect(src).toContain('std::setw(16)');
    expect(src).toContain('std::nouppercase');
    expect(src).not.toMatch(/result\.set\("hash_(?:lo|hi)",\s*static_cast<double>/);

    for (const key of [
        'schema_version', 'lattice_size', 'site_count', 'tick',
        'state_version', 'nonfinite_value_count', 'nondefault_value_count',
        'device_to_host_bytes', 'full_mirror_calls',
    ]) {
        expect(src, `missing publication field ${key}`).toContain(`result.set("${key}"`);
    }
});

test('direct WasmBridge returns the Embind digest synchronously without lane coercion', async ({ page }) => {
    await page.goto('/');
    const result = await page.evaluate(async (expected) => {
        const { WasmBridge } = await import('/js/bridge/wasm-bridge.js');
        const bridge = Object.create(WasmBridge.prototype);
        bridge._bridge = {};
        bridge._module = {
            captureDynamicalStateDigest(owner) {
                if (owner !== bridge._bridge) throw new Error('wrong RenderBridge owner');
                return { ...expected };
            },
        };
        const direct = bridge.captureDynamicalStateDigest();
        return {
            direct,
            getter: bridge.getDynamicalStateDigest(),
            capabilityAlias: bridge.getScale0DynamicalStateDigest(),
            laneTypes: [typeof direct.hashLo, typeof direct.hashHi],
        };
    }, RAW_EMPTY_DIGEST);

    expect(result.direct).toEqual(EMPTY_DIGEST);
    expect(result.getter).toEqual(EMPTY_DIGEST);
    expect(result.capabilityAlias).toEqual(EMPTY_DIGEST);
    expect(result.laneTypes).toEqual(['string', 'string']);
});

test('worker proxy caches initial publication and captures fresh digest on demand', async ({ page }) => {
    await page.goto('/');
    const result = await page.evaluate(async (expected) => {
        const workers = [];
        class FakeWorker {
            constructor() { workers.push(this); this.onmessage = null; this.onerror = null; }
            postMessage(message) {
                if (message.type !== 'captureDigest') return;
                const fresh = { ...expected, tick: 17, stateVersion: 17 };
                queueMicrotask(() => this.onmessage?.({
                    data: {
                        type: 'digestResult', reqId: message.reqId, digest: fresh,
                        configurationToken: message.configurationToken,
                    },
                }));
            }
            terminate() {}
        }
        globalThis.Worker = FakeWorker;

        const { WasmBridgeProxy } = await import('/js/bridge/wasm-bridge-proxy.js');
        const proxy = new WasmBridgeProxy(9);
        try {
            proxy._ready = true;
            proxy._onMessage({
                type: 'frame',
                configurationToken: 0,
                diag: { tick: 0 },
                dynamicalStateDigest: { ...expected },
            });
            const initial = proxy.getCachedDynamicalStateDigest();
            const fresh = await proxy.captureDynamicalStateDigest(1000);
            return {
                workerCount: workers.length,
                initial,
                fresh,
                cached: proxy.getCachedDynamicalStateDigest(),
                capabilityFresh: await proxy.getScale0DynamicalStateDigest(),
                laneTypes: [typeof fresh.hashLo, typeof fresh.hashHi],
            };
        } finally {
            proxy.terminate();
        }
    }, { ...EMPTY_DIGEST, transport: 'worker' });

    expect(result.workerCount).toBe(1);
    const workerDigest = { ...EMPTY_DIGEST, transport: 'worker' };
    expect(result.initial).toEqual(workerDigest);
    expect(result.fresh).toEqual({ ...workerDigest, tick: 17, stateVersion: 17 });
    expect(result.cached).toEqual(result.fresh);
    expect(result.capabilityFresh).toEqual(result.fresh);
    expect(result.laneTypes).toEqual(['string', 'string']);
});

test('worker qualification callback waits for a successful configuration barrier', async ({ page }) => {
    await page.goto('/');
    const result = await page.evaluate(async () => {
        const OriginalWorker = globalThis.Worker;
        const posted = [];
        class FakeWorker {
            constructor() { this.onmessage = null; this.onerror = null; }
            postMessage(message) { posted.push(message); }
            terminate() {}
        }
        globalThis.Worker = FakeWorker;

        const engineReadbacks = [];
        const acknowledgements = [];
        const { WasmBridgeProxy } = await import('/js/bridge/wasm-bridge-proxy.js');
        const proxy = new WasmBridgeProxy(9, {
            onEngineToggles: (toggles) => engineReadbacks.push({ ...toggles }),
            onConfigurationApplied: (ack) => acknowledgements.push({
                ok: ack.ok,
                errors: [...ack.errors],
            }),
        });
        try {
            proxy.setupScenario('empty');
            proxy.setFluxBoundaryMode(0);
            proxy.setFluxPeriodicAxis(2);
            await Promise.resolve();
            const create = posted.find((message) => message.type === 'create');
            const ctrl = new SharedArrayBuffer(8 * 4);
            const heap = new ArrayBuffer(9 ** 3 * 8);
            proxy._onMessage({
                type: 'ready',
                N: 9,
                ctrl,
                heap,
                fluxPtr: 0,
                fluxLen: 9 ** 3,
                setupOk: true,
                configurationToken: create.configurationToken,
                artifactIdentity: { variant: { id: 'wasm32-threads' } },
            });
            const batch = posted.find((message) => message.type === 'batchCommand');

            // This is the real worker ordering: initial/final readback frames
            // are queued before the explicit configurationApplied message.
            proxy._onMessage({
                type: 'frame',
                configurationToken: batch.configurationToken,
                diag: { tick: 0 },
                engineToggles: { wave_propagation: false },
            });
            const readbacksBeforeBarrier = engineReadbacks.length;
            proxy._onMessage({
                type: 'configurationApplied',
                configurationToken: batch.configurationToken,
                ok: true,
                errors: [],
                engineToggles: { wave_propagation: false },
                fluxBoundaryMode: 0,
                fluxPeriodicAxis: 2,
            });
            proxy._onMessage({
                type: 'frame',
                configurationToken: batch.configurationToken,
                diag: { tick: 1 },
                engineToggles: { wave_propagation: false },
            });
            const readbacksAfterAcceptedBarrier = engineReadbacks.length;

            proxy.setupScenario('empty');
            await Promise.resolve();
            const rejectedCreate = posted.filter((message) => message.type === 'create').at(-1);
            proxy._onMessage({
                type: 'frame',
                configurationToken: batch.configurationToken,
                diag: { tick: 99 },
                engineToggles: { wave_propagation: true },
            });
            const staleFrameAccepted = proxy._lastDiag !== null;
            proxy._onMessage({
                type: 'ready',
                N: 9,
                ctrl,
                heap,
                fluxPtr: 0,
                fluxLen: 9 ** 3,
                setupOk: true,
                configurationToken: rejectedCreate.configurationToken,
                artifactIdentity: { variant: { id: 'wasm32-threads' } },
            });
            const rejectedBatch = posted.filter((message) => message.type === 'batchCommand').at(-1);
            proxy._onMessage({
                type: 'configurationApplied',
                configurationToken: rejectedBatch.configurationToken,
                ok: false,
                errors: ['setFluxBoundary failed'],
                engineToggles: { wave_propagation: false },
                fluxBoundaryMode: 0,
            });
            proxy._onMessage({
                type: 'frame',
                configurationToken: rejectedBatch.configurationToken,
                diag: { tick: 2 },
                engineToggles: { wave_propagation: false },
            });

            return {
                batchMethods: batch.commands.map((command) => command.method),
                readbacksBeforeBarrier,
                readbacksAfterAcceptedBarrier,
                readbacksAfterRejectedBarrier: engineReadbacks.length,
                staleFrameAccepted,
                acknowledgements,
            };
        } finally {
            proxy.terminate();
            globalThis.Worker = OriginalWorker;
        }
    });

    expect(result.batchMethods).toContain('setFluxBoundary');
    expect(result.batchMethods).toContain('setFluxPeriodicAxis');
    expect(result.readbacksBeforeBarrier).toBe(0);
    expect(result.staleFrameAccepted).toBe(false);
    expect(result.readbacksAfterRejectedBarrier).toBe(result.readbacksAfterAcceptedBarrier);
    expect(result.acknowledgements).toEqual([
        { ok: true, errors: [] },
        { ok: false, errors: ['setFluxBoundary failed'] },
    ]);
});

test('worker hashes once on build and only on explicit capture thereafter', () => {
    const src = fs.readFileSync(
        path.join(webRoot, 'js', 'bridge', 'wasm-bridge.worker.js'),
        'utf8',
    );
    const buildBody = src.slice(src.indexOf('function buildBridge('), src.indexOf('function postFrame('));
    const frameBody = src.slice(src.indexOf('function postFrame('), src.indexOf('function loop('));
    const commandBody = src.slice(src.indexOf("case 'captureDigest':"), src.indexOf("case 'wantSampler':"));

    expect(buildBody).toContain('lastDynamicalStateDigest = captureDynamicalStateDigest()');
    expect(frameBody).not.toContain('captureDynamicalStateDigest()');
    expect(commandBody).toContain('lastDynamicalStateDigest = captureDynamicalStateDigest()');
    expect(commandBody).toContain("type: 'digestResult'");
});
