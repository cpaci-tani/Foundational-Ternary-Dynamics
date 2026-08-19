// @ts-check
/**
 * Scale-0 physics Web Worker regression (Phase 1 WASM off-thread). Pins that,
 * when the page is cross-origin isolated (SharedArrayBuffer available),
 * flux-* scenarios run on WasmBridgeProxy / wasm-bridge.worker.js, the worker
 * self-ticks the C++ engine, the shared SAB field populates, and switching to a
 * non-worker WASM scenario tears the worker down.
 *
 * Requires a COOP/COEP server. The default test server is plain http.server
 * (not isolated) so these tests SKIP there; run against the caching+COOP server
 * to exercise them:  python serve.py 8081 --cache
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

const coiReady = (page) => page.evaluate(() =>
    globalThis.crossOriginIsolated === true && typeof SharedArrayBuffer !== 'undefined');

const fluxMockInfo = (page) => page.evaluate(async () => {
    const st = (await import('/js/scales/scale0/state/store.js')).getScale0State?.();
    const fm = st?.fluxMock;
    return {
        type: fm?.constructor?.name ?? null, isWorker: !!fm?.isWorker, ready: !!fm?.ready,
        fc: fm?.frameCounter ?? null, scenario: st?.currentScenarioId, useFluxMock: !!st?.useFluxMock,
    };
});

test.beforeEach(async ({ page }) => { page.setDefaultTimeout(30_000); });

test.describe('Scale-0 physics Web Worker', () => {

    test('flux-pulse runs on WasmBridgeProxy, self-ticks, and populates the shared field', async ({ page }) => {
        await gotoAndReady(page);
        test.skip(!(await coiReady(page)), 'requires cross-origin isolation (serve.py --cache COOP/COEP)');

        await expect.poll(async () => (await fluxMockInfo(page)).ready,
            { timeout: 20_000, message: 'WASM worker proxy never became ready' }).toBe(true);

        const info = await fluxMockInfo(page);
        expect(info.type, 'flux-pulse uses WasmBridgeProxy').toBe('WasmBridgeProxy');
        expect(info.isWorker).toBe(true);
        expect(info.useFluxMock).toBe(true);

        // The worker ran its setup + at least one tick. NOTE: headless/background
        // tabs throttle worker setTimeout, so CONTINUOUS ticking can't be observed
        // here — it's verified manually in a foreground tab (~60 fps).
        // Here we assert the mechanism: the worker advanced ≥1 frame and wrote the shared field.
        const fc1 = (await fluxMockInfo(page)).fc;
        expect(fc1, 'worker completed at least one tick').toBeGreaterThanOrEqual(1);

        // The shared field is populated, read via the proxy/shadow over the SAB.
        const nonzero = await page.evaluate(async () => {
            const fm = (await import('/js/scales/scale0/state/store.js')).getScale0State().fluxMock;
            const fv = fm.capabilities.scale0.getScale0FluxVolume();
            let n = 0; for (let i = 0; i < fv.length; i += 13) if (fv[i] !== 0) n++; return n;
        });
        expect(nonzero, 'shared flux field is populated').toBeGreaterThan(0);
    });

    test('switching to a WASM-owned scenario (empty) tears the worker down', async ({ page }) => {
        await gotoAndReady(page);
        test.skip(!(await coiReady(page)), 'requires cross-origin isolation');

        await expect.poll(async () => (await fluxMockInfo(page)).ready, { timeout: 20_000 }).toBe(true);
        expect((await fluxMockInfo(page)).isWorker).toBe(true);

        await page.evaluate(() => {
            const s = document.getElementById('scenario-select');
            s.value = 'empty'; s.dispatchEvent(new Event('change', { bubbles: true }));
        });
        await expect.poll(async () => (await fluxMockInfo(page)).scenario, { timeout: 15_000 }).toBe('empty');

        const info = await fluxMockInfo(page);
        expect(info.useFluxMock, 'empty is WASM-owned (direct bridge, no worker proxy)').toBe(false);
    });

    // ── Bridge-wiring regression (audit 2026-06-03) ──────────────────────────
    // Before the fix, the proxy forwarded only 5 direct reads, so panels
    // that call sampler/list methods directly on the bridge (flux-slice |E|/|B|/
    // |S|/∇·J, spectrum, p1-observables) silently blanked under the worker. The
    // proxy now delegates the whole canonical read-surface to its SAB-backed
    // shadow + ships a worker-sourced particle list.

    test('worker-proxy direct field-sampler reads match the capability path (flux-slice wiring)', async ({ page }) => {
        await gotoAndReady(page);
        test.skip(!(await coiReady(page)), 'requires cross-origin isolation (serve.py --cache COOP/COEP)');

        await expect.poll(async () => (await fluxMockInfo(page)).ready,
            { timeout: 20_000, message: 'WASM worker proxy never became ready' }).toBe(true);

        const r = await page.evaluate(async () => {
            const fm = (await import('/js/scales/scale0/state/store.js')).getScale0State().fluxMock;
            const caps = fm.capabilities.scale0;
            const count = (o) => (o && typeof o.count === 'number' ? o.count : null);
            // Direct-on-bridge reads — exactly what flux-slice-panel.js:91/101/111/121
            // and p1-observables-panel.js:780 call.
            const direct = {
                e: fm.getEFieldSampled(1), b: fm.getBFieldSampled(1),
                poynting: fm.getPoyntingSampled(1), divJ: fm.getDivJSampled(1),
                latency: fm.getLatencySampled(2),
            };
            // Capability path (field-overlays.js) — known-good; both hit the shadow.
            const cap = {
                e: caps.getScale0FieldSamples({ kind: 'e', stride: 1 }),
                b: caps.getScale0FieldSamples({ kind: 'b', stride: 1 }),
                poynting: caps.getScale0FieldSamples({ kind: 'poynting', stride: 1 }),
                divJ: caps.getScale0FieldSamples({ kind: 'divJ', stride: 1 }),
                latency: caps.getScale0FieldSamples({ kind: 'latency', stride: 2 }),
            };
            const out = {};
            for (const k of Object.keys(direct)) {
                out[k] = { defined: typeof direct[k] !== 'undefined', direct: count(direct[k]), cap: count(cap[k]) };
            }
            return out;
        });

        for (const k of ['e', 'b', 'poynting', 'divJ', 'latency']) {
            expect(r[k].defined, `${k}: direct read is wired on the proxy`).toBe(true);
            expect(r[k].direct, `${k}: direct count is a number`).not.toBeNull();
            expect(r[k].direct, `${k}: direct read matches capability path`).toBe(r[k].cap);
        }
        const anyPopulated = ['e', 'b', 'poynting', 'divJ'].some((k) => (r[k].direct ?? 0) > 0);
        expect(anyPopulated, 'at least one field sampler is non-empty (charts have data)').toBe(true);
    });

    test('WasmBridgeProxy answers every canonical direct-read (anti-drift contract)', async ({ page }) => {
        await gotoAndReady(page);
        test.skip(!(await coiReady(page)), 'requires cross-origin isolation (serve.py --cache COOP/COEP)');

        await expect.poll(async () => (await fluxMockInfo(page)).ready,
            { timeout: 20_000, message: 'WASM worker proxy never became ready' }).toBe(true);

        const r = await page.evaluate(async () => {
            const fm = (await import('/js/scales/scale0/state/store.js')).getScale0State().fluxMock;
            const { SCALE0_DIRECT_READS } = await import('/js/bridge/bridge-contract.js');
            const missing = [], threw = [], undef = [];
            for (const { name } of SCALE0_DIRECT_READS) {
                if (typeof fm[name] !== 'function') { missing.push(name); continue; }
                try {
                    const v = name === 'getForceAt' ? fm[name](1, 1, 1) : fm[name](2);
                    if (typeof v === 'undefined') undef.push(name);
                } catch (e) { threw.push(`${name}: ${e && e.message}`); }
            }
            return {
                missing, threw, undef,
                particleListIsArray: Array.isArray(fm.getScale0ParticleList()),
                workerShippedList: fm._lastParticleList !== null,   // worker posted a list (B2)
            };
        });

        expect(r.missing, 'every canonical direct-read is present on WasmBridgeProxy').toEqual([]);
        expect(r.threw, 'no canonical direct-read throws on WasmBridgeProxy').toEqual([]);
        expect(r.undef, 'no canonical direct-read returns undefined once ready').toEqual([]);
        expect(r.particleListIsArray, 'getScale0ParticleList returns an array').toBe(true);
        expect(r.workerShippedList, 'WASM worker shipped a particle list to the proxy (B2)').toBe(true);
    });
});
