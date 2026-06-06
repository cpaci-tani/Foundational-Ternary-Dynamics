// @ts-check
/**
 * Regression — substrate inject controls must take effect IMMEDIATELY, even
 * while paused, on the worker path.
 *
 * Bug: the Scale-0 physics worker only shipped a frame (`postFrame`) when it
 * ticked or received `tickScale0`. Inject / clear / seed commands mutated the
 * worker's state but never shipped a frame, so while PAUSED (the default load
 * state) the injection was invisible to the main thread until the next tick or
 * single-step — the "Wave / Flux / Pair don't work" report. Fix: the worker
 * posts a frame after every command. (engine/web/js/bridge/mock-bridge.worker.js)
 *
 * This regression covers all four inject buttons via the shared root cause; the
 * entangled pair is the cleanest signal (deterministic +2 particles, and the
 * tick must NOT advance, proving it's the injection and not genesis).
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.describe('Scale-0 substrate inject while paused (worker path)', () => {
    test('entangled-pair injection is visible immediately without a tick/step', async ({ page }) => {
        test.setTimeout(60_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!(window.__ftdCtx?.bridge)), { timeout: 20_000 }).toBe(true);
        await page.waitForTimeout(600);

        const isWorker = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const st = getScale0State();
            return !!(st.useFluxMock && st.fluxMock && st.fluxMock.isWorker);
        });
        test.skip(!isWorker, 'worker path inactive (no cross-origin isolation) — bug is worker-specific');

        // Force paused (it is by default) + center the inject position.
        await page.evaluate(() => { window.__ftdCtx.running = false; document.getElementById('btn-center')?.click(); });
        await page.waitForTimeout(500);
        expect(await page.evaluate(() => !!window.__ftdCtx.running), 'sim must be paused').toBe(false);

        const read = async () => page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const d = getScale0State().fluxMock.capabilities.scale0.getScale0Diagnostics?.() || {};
            return { tick: d.tick ?? -1, parts: Number(d.manifested ?? 0) };
        });

        const before = await read();
        await page.evaluate(() => document.getElementById('btn-inject-pair')?.click());

        // No step, no resume — the proxy must reflect the +2 particles purely
        // from the worker shipping a frame after the inject command.
        await expect.poll(async () => (await read()).parts, {
            timeout: 5_000,
            message: 'paused entangled-pair injection should appear without a tick/step',
        }).toBe(before.parts + 2);

        // The pair appeared with NO tick advance — proving it's the injection,
        // not genesis firing on a running sim.
        expect((await read()).tick, 'no tick should have advanced while paused').toBe(before.tick);
    });
});
