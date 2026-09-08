import { test, expect } from '@playwright/test';
import { readFileSync } from 'node:fs';
import { createHash } from 'node:crypto';

const oracleBytes = readFileSync(new URL('./fixtures/gravity-dense-v3-oracle.mjs', import.meta.url));
const oracleSha256 = '6b4d7b232ad3a58a5c9fc3c96e011683d1d53b4f83a07d1c356d6e698643d3d8';

test('mounted Gravity uses one batch for all planes, exact pixels and unavailable recovery without dense reads', async ({ page }) => {
    // Windows MIME defaults may serve .mjs as text/plain. Route only the
    // immutable test oracle with its real module type; keep its URL so the
    // unchanged relative constants import still resolves to /js/constants.js.
    expect(createHash('sha256').update(oracleBytes).digest('hex')).toBe(oracleSha256);
    await page.route('**/tests/fixtures/gravity-dense-v3-oracle.mjs', route => route.fulfill({
        contentType: 'application/javascript; charset=utf-8', body: oracleBytes,
    }));
    await page.goto('/js/ui/panels/retained-readout.js');
    const result = await page.evaluate(async () => {
        const [{ mountGravityPanel }, { WasmBridgeProxy }, { createScale0Capabilities }, store,
            { gravitySlice }, { transposeAndFlipNN, paintSliceToCanvas }, ramps] = await Promise.all([
            import('/js/scales/scale0/ui/overlays/gravity-panel.js?slab-fixture=1'),
            import('/js/bridge/wasm-bridge-proxy.js'),
            import('/js/bridge/capabilities/scale0.js'),
            import('/js/scales/scale0/state/store.js'),
            import('/tests/fixtures/gravity-dense-v3-oracle.mjs'),
            import('/js/scales/scale0/ui/overlays/slice-render.js'),
            import('/js/viewport/color-ramps.js'),
        ]);
        const pub = globalThis.FTD_FLUX_PUBLICATION;
        const scenarioId = 's0-seed-massive-body', generation = 930001, N = 7, mid = 3;
        const select = document.createElement('select');
        select.id = 'scenario-select';
        select.add(new Option(scenarioId, scenarioId));
        document.body.appendChild(select);
        store.setCurrentScenarioId(scenarioId);
        store.beginScale0AuthoritativeLoad({ scenarioId, loadGeneration: generation });
        if (!store.completeScale0AuthoritativeLoad({ scenarioId, loadGeneration: generation,
            tick: 0, source: 'mounted-gravity-slab-test' })) throw Error('fixture load did not qualify');
        const data = Float64Array.from({ length: N ** 3 }, (_, i) => 1 + (i % 13) / 17);
        data[0] = 97; // outside all three central slabs
        const buffer = pub.create(data.length);
        pub.publish(buffer, data);
        const proxy = Object.create(WasmBridgeProxy.prototype);
        Object.assign(proxy, { latticeSize: N, isWasm: true, isWorker: true,
            _ready: true, _terminated: false, _disposing: false,
            _pendingConfigurationToken: 2, _appliedConfigurationToken: 2, _fluxBufferGeneration: 0,
            _digestPending: new Map(), _inspectionCache: new Map(), _inspectionPending: new Map(),
            _engineToggles: { forces: true, gravity: true },
            _samplerCache: {
                'latency@2': { values: new Float32Array([0.25]), count: 1 },
                'kretschmann@2': { values: new Float32Array([0.5]), count: 1 },
                'gravity@2': { vectors: new Float32Array([0.25, 0, 0]), count: 1 },
            },
            _samplerCacheVersion: { 'latency@2': 1, 'kretschmann@2': 1, 'gravity@2': 1 },
        });
        proxy._bindFlux({ fluxSab: buffer, fluxLen: data.length, doubleBuffered: true, fluxProtocol: pub.PROTOCOL });
        proxy.replaceSamplerWants = () => {};
        proxy.getFluxVolume = () => { throw Error('Gravity attempted a forbidden full-volume fallback'); };
        proxy.getDiagnostics = () => { throw Error('Gravity attempted a diagnostics-normalizer substitute'); };
        const batches = [];
        const getBatch = proxy.getFluxSlabsWithMaxRho.bind(proxy);
        proxy.getFluxSlabsWithMaxRho = requests => {
            const result = getBatch(requests);
            batches.push({ selectors: requests.map(r => [r.axis, r.index]), result });
            return result;
        };
        proxy.capabilities = { scale0: createScale0Capabilities(proxy) };
        const host = document.createElement('section');
        host.className = 'active';
        document.body.appendChild(host);
        const api = mountGravityPanel(host, () => proxy);
        const rampFor = { latency: ramps.rampViridis, dilation: ramps.rampViridis,
            kretschmann: ramps.rampEmEnergy, force: ramps.rampVorticity };
        const samePixels = (a, b) => a.length === b.length && a.every((value, i) => value === b[i]);
        try {
            const initialBatches = batches.map(b => b.selectors);
            batches.length = 0;
            const quantities = [];
            for (const kind of ['latency', 'dilation', 'kretschmann', 'force']) {
                const button = host.querySelector(`.grav-qbtn[data-kind="${kind}"]`);
                button.click(); // actual delegated quantity-change path
                const observed = batches.at(-1).result;
                const pixelMatches = [];
                for (let axis = 0; axis < 3; axis++) {
                    const original = gravitySlice(data, N, axis, mid, kind);
                    const transformed = transposeAndFlipNN(original, N);
                    let max = 0;
                    for (const value of transformed) if (value > max) max = value;
                    const referenceCanvas = document.createElement('canvas');
                    referenceCanvas.width = 116; referenceCanvas.height = 116;
                    paintSliceToCanvas(referenceCanvas, transformed, N,
                        { ramp: rampFor[kind], norm: max > 1e-30 ? 1 / max : 1 });
                    const actualCanvas = host.querySelector(`#gravity-panel-tile-${axis}`);
                    const actualPixels = actualCanvas.getContext('2d').getImageData(0, 0, 116, 116).data;
                    const originalPixels = referenceCanvas.getContext('2d').getImageData(0, 0, 116, 116).data;
                    pixelMatches.push(samePixels(actualPixels, originalPixels));
                }
                quantities.push({ kind, selectors: batches.at(-1).selectors, pixelMatches,
                    commonMetadata: observed.slabs.every(s => s.metadata === observed.metadata),
                    maxRho: observed.maxRho, tick: observed.metadata.sampleTick });
            }
            const beforeFailure = batches.length;
            const pin = pub.acquire(buffer, N ** 3);
            try { api.setKind('latency'); } finally { pin.release(); }
            const unavailable = {
                count: batches.length - beforeFailure,
                nullRead: batches.at(-1).result === null,
                readouts: [...host.querySelectorAll('.grav-tile-readout')].map(el => el.textContent),
                waiting: host.querySelector('.grav-mode').textContent,
            };
            // Versions remain fixed. Recover every cleared plane in one batch,
            // rather than letting the paused-read latch retain blank tiles.
            const beforeRecovery = batches.length;
            api.update();
            const recovery = { count: batches.length - beforeRecovery,
                selectors: batches.at(-1).selectors,
                readoutsReady: [...host.querySelectorAll('.grav-tile-readout')].every(el => /^max /.test(el.textContent)),
                noDenseCache: proxy._fluxSnapshot === null };
            host.classList.remove('active');
            const beforeHidden = batches.length;
            api.update();
            const hiddenNoRead = batches.length === beforeHidden;
            return { initialBatches, quantities, unavailable, recovery, hiddenNoRead };
        } finally { api.dispose(); host.remove(); select.remove(); }
    });
    expect(result.initialBatches).toEqual([[[0, 3]]]);
    expect(result.quantities).toEqual(['latency', 'dilation', 'kretschmann', 'force'].map(kind => ({
        kind, selectors: [[0, 3], [1, 3], [2, 3]], pixelMatches: [true, true, true],
        commonMetadata: true, maxRho: 97 * 97, tick: null,
    })));
    expect(result.unavailable).toEqual({ count: 1, nullRead: true, readouts: ['—', '—', '—'], waiting: 'proxy · waiting' });
    expect(result.recovery).toEqual({ count: 1, selectors: [[0, 3], [1, 3], [2, 3]], readoutsReady: true, noDenseCache: true });
    expect(result.hiddenNoRead).toBe(true);
});
