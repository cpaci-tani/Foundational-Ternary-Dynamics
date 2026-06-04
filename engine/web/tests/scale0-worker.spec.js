// @ts-check
/**
 * Scale-0 physics Web Worker regression (Phase 2). Pins that, when the page is
 * cross-origin isolated (SharedArrayBuffer available), flux-* scenarios run on
 * the MockBridgeProxy / worker, the worker self-ticks, the shared field
 * populates, and switching to a WASM-owned scenario tears the worker down.
 *
 * Requires a COOP/COEP server. The default test server is plain http.server
 * (not isolated) so these tests SKIP there; run against the caching+COOP server
 * to exercise them:  python serve.py 8081 --cache   (see PLAN_SCALE0_PHYSICS_WORKER.md).
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

    test('flux-pulse runs on a worker proxy, self-ticks, and populates the shared field', async ({ page }) => {
        await gotoAndReady(page);
        test.skip(!(await coiReady(page)), 'requires cross-origin isolation (serve.py --cache COOP/COEP)');

        await expect.poll(async () => (await fluxMockInfo(page)).ready,
            { timeout: 20_000, message: 'worker proxy never became ready' }).toBe(true);

        const info = await fluxMockInfo(page);
        expect(info.type, 'flux-pulse uses MockBridgeProxy').toBe('MockBridgeProxy');
        expect(info.isWorker).toBe(true);
        expect(info.useFluxMock).toBe(true);

        // The worker ran its setup + at least one tick. NOTE: headless/background
        // tabs throttle worker setTimeout, so CONTINUOUS ticking can't be observed
        // here — it's verified manually in a foreground tab (~60 fps; see
        // PLAN_SCALE0_PHYSICS_WORKER.md). Here we assert the mechanism: the worker
        // advanced ≥1 frame and wrote the shared field.
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
        expect(info.useFluxMock, 'empty is WASM-owned — no fluxMock/worker').toBe(false);
    });
});
