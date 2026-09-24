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
        let invalidNextBatch = false;
        const getBatch = proxy.getFluxSlabsWithMaxRho.bind(proxy);
        proxy.getFluxSlabsWithMaxRho = requests => {
            let result = getBatch(requests);
            if (invalidNextBatch && result) {
                invalidNextBatch = false;
                result = { ...result, slabs: result.slabs.slice(0, 2) };
            }
            batches.push({ selectors: requests.map(r => [r.axis, r.index]), result });
            return result;
        };
        const scale0Capabilities = createScale0Capabilities(proxy);
        // This fixture audits the legacy slab fallback independently of the
        // worker's atomic gravity-observation bundle.
        scale0Capabilities.getScale0GravityObservation = undefined;
        proxy.capabilities = { scale0: scale0Capabilities };
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
                const beforeQuantity = batches.length;
                button.click(); // actual delegated quantity-change path
                const prepared = [];
                for (let axis = 0; axis < 3; axis++) {
                    prepared.push(transposeAndFlipNN(gravitySlice(data, N, axis, mid, kind), N));
                }
                let upper = 1;
                if (kind === 'kretschmann' || kind === 'force') {
                    upper = 0;
                    for (const plane of prepared) {
                        for (const value of plane) if (value > upper) upper = value;
                    }
                }
                const pixelMatches = [];
                for (let axis = 0; axis < 3; axis++) {
                    const referenceCanvas = document.createElement('canvas');
                    referenceCanvas.width = 116; referenceCanvas.height = 116;
                    paintSliceToCanvas(referenceCanvas, prepared[axis], N,
                        { ramp: rampFor[kind], norm: upper > 1e-30 ? 1 / upper : 1 });
                    const actualCanvas = host.querySelector(`#gravity-panel-tile-${axis}`);
                    const actualPixels = actualCanvas.getContext('2d').getImageData(0, 0, 116, 116).data;
                    const originalPixels = referenceCanvas.getContext('2d').getImageData(0, 0, 116, 116).data;
                    pixelMatches.push(samePixels(actualPixels, originalPixels));
                }
                quantities.push({ kind, reads: batches.length - beforeQuantity, pixelMatches,
                    pressedStates: [...host.querySelectorAll('.grav-qbtn')]
                        .map(el => [el.dataset.kind, el.getAttribute('aria-pressed')]),
                    activeKind: api.activeKind,
                    ticks: [...host.querySelectorAll('.grav-tile-tick')].map(el => el.textContent),
                    sampleTick: api.sampleTick });
            }

            const canvases = [...host.querySelectorAll('.grav-tile canvas')];
            const pixelsBeforeFailure = canvases.map(canvas =>
                [...canvas.getContext('2d').getImageData(0, 0, 116, 116).data]);
            const readoutsBeforeFailure = [...host.querySelectorAll('.grav-tile-readout')]
                .map(el => el.textContent);
            invalidNextBatch = true;
            const beforeInvalid = batches.length;
            store.getScale0State().fieldDataVersion++;
            api.update();
            const invalid = {
                count: batches.length - beforeInvalid,
                incomplete: batches.at(-1).result?.slabs?.length === 2,
                pixelsRetained: canvases.map((canvas, axis) => samePixels(
                    [...canvas.getContext('2d').getImageData(0, 0, 116, 116).data],
                    pixelsBeforeFailure[axis])),
                readoutsRetained: [...host.querySelectorAll('.grav-tile-readout')]
                    .every((el, axis) => el.textContent === readoutsBeforeFailure[axis]),
            };
            const beforeFailure = batches.length;
            const pin = pub.acquire(buffer, N ** 3);
            store.getScale0State().fieldDataVersion++;
            try { api.update(); } finally { pin.release(); }
            const unavailable = {
                count: batches.length - beforeFailure,
                nullRead: batches.at(-1).result === null,
                readoutsReady: [...host.querySelectorAll('.grav-tile-readout')]
                    .every(el => /^max /.test(el.textContent)),
                pixelsRetained: canvases.map((canvas, axis) => samePixels(
                    [...canvas.getContext('2d').getImageData(0, 0, 116, 116).data],
                    pixelsBeforeFailure[axis])),
                readoutsRetained: [...host.querySelectorAll('.grav-tile-readout')]
                    .every((el, axis) => el.textContent === readoutsBeforeFailure[axis]),
                status: host.querySelector('.grav-observation-status').textContent,
            };
            // The next field version must reacquire all three planes atomically.
            const beforeRecovery = batches.length;
            store.getScale0State().fieldDataVersion++;
            api.update();
            const recovery = { count: batches.length - beforeRecovery,
                selectors: batches.at(-1).selectors,
                readoutsReady: [...host.querySelectorAll('.grav-tile-readout')].every(el => /^max /.test(el.textContent)),
                pixelsRestored: canvases.map((canvas, axis) => samePixels(
                    [...canvas.getContext('2d').getImageData(0, 0, 116, 116).data],
                    pixelsBeforeFailure[axis])),
                tickLabels: [...host.querySelectorAll('.grav-tile-tick')].map(el => el.textContent),
                sampleTick: api.sampleTick,
                noDenseCache: proxy._fluxSnapshot === null };
            host.classList.remove('active');
            const beforeHidden = batches.length;
            api.update();
            const hiddenNoRead = batches.length === beforeHidden;
            return { initialBatches, quantities, invalid, unavailable, recovery, hiddenNoRead };
        } finally { api.dispose(); host.remove(); select.remove(); }
    });
    expect(result.initialBatches).toEqual([[[0, 3], [1, 3], [2, 3]]]);
    expect(result.quantities).toEqual(['latency', 'dilation', 'kretschmann', 'force'].map(kind => ({
        kind, reads: 0, pixelMatches: [true, true, true],
        pressedStates: ['latency', 'kretschmann', 'force', 'dilation']
            .map(candidate => [candidate, String(candidate === kind)]),
        activeKind: kind,
        ticks: ['tick unavailable', 'tick unavailable', 'tick unavailable'], sampleTick: null,
    })));
    expect(result.invalid).toEqual({ count: 1, incomplete: true,
        pixelsRetained: [true, true, true], readoutsRetained: true });
    expect(result.unavailable).toEqual({ count: 1, nullRead: true,
        readoutsReady: true,
        pixelsRetained: [true, true, true], readoutsRetained: true,
        status: 'Waiting for a complete observation · previous tick retained' });
    expect(result.recovery).toEqual({ count: 1, selectors: [[0, 3], [1, 3], [2, 3]],
        readoutsReady: true, pixelsRestored: [true, true, true],
        tickLabels: ['tick unavailable', 'tick unavailable', 'tick unavailable'],
        sampleTick: null, noDenseCache: true });
    expect(result.hiddenNoRead).toBe(true);
});
