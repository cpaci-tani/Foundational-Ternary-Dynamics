// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

/**
 * De Broglie internal-clock scenario (FTD-0271).
 *
 * Verifies the new Scale-0 `s0-seed-de-broglie-clock` scenario runs on the REAL
 * WASM engine, mounts its interactive panel, and that pressing "Run clock"
 * drives the engine so the centre flux J_x(t) OSCILLATES — the Klein-Gordon
 * rest-mass term -omega0^2*J turning FTD's natively-massless flux into a de
 * Broglie internal clock. The panel must clean up the de_broglie_clock toggle
 * and dispose on scenario switch.
 *
 * [CONDITIONAL]: the clock omega0~M_REST is IMPOSED; this test asserts the
 * lattice CORRECTLY carries the oscillation, not that FTD predicts it.
 */

test.describe('De Broglie internal clock (FTD-0271)', () => {
    /** @type {import('@playwright/test').Page} */
    let page;

    test.beforeAll(async ({ browser, baseURL }) => {
        const context = await browser.newContext({ baseURL });
        page = await context.newPage();
        page.setDefaultTimeout(60_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!(window.__ftdCtx?.bridge)), { timeout: 20_000 }).toBe(true);
    });

    test.afterAll(async () => {
        await page.close();
    });

    test('scenario runs on real WASM, panel mounts, and the clock oscillates', async () => {
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            sel.value = 's0-seed-de-broglie-clock';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        // panel mounts on load
        await expect.poll(() => page.evaluate(() => !!window.__ftdDeBroglieClockPanel), { timeout: 10_000 }).toBe(true);

        // real engine, not the JS flux-mock
        const useMock = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            return !!getScale0State().useFluxMock;
        });
        expect(useMock, 'de Broglie clock must run on the real WASM engine').toBe(false);

        // drive the panel's run() at omega0=0.3 and read the centre J_x(t) trace
        const trace = await page.evaluate(async () => {
            await window.__ftdDeBroglieClockPanel.run(0.3);
            return window.__ftdDeBroglieClockPanel.getTrace();
        });
        expect(trace.length, 'a full trace was recorded').toBeGreaterThan(100);
        const mn = Math.min(...trace);
        const mx = Math.max(...trace);
        // J_x(t) = J0*cos(omega0*t) swings negative: a genuine oscillation, not a drift.
        expect(mn, 'centre flux swings negative (genuine clock oscillation)').toBeLessThan(0);
        expect(mx, 'centre flux stays positive at the peak').toBeGreaterThan(0);
    });

    test('the de_broglie_clock toggle is cleaned up after a run (no leak)', async () => {
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            sel.value = 's0-seed-de-broglie-clock';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        await expect.poll(() => page.evaluate(() => !!window.__ftdDeBroglieClockPanel), { timeout: 10_000 }).toBe(true);
        const after = await page.evaluate(async () => {
            const b = window.__ftdCtx?.bridge;
            if (b && typeof b.setToggle === 'function') {
                b.setToggle('de_broglie_clock', false);
            }
            await window.__ftdDeBroglieClockPanel.run(0.3);
            return b?.getToggle ? b.getToggle('de_broglie_clock') : null;
        });
        expect(after, 'clock toggle is restored to its pre-run state (false)').toBe(false);
    });

    test('panel is disposed when switching away from the scenario', async () => {
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            sel.value = 's0-seed-de-broglie-clock';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        await expect.poll(() => page.evaluate(() => !!window.__ftdDeBroglieClockPanel), { timeout: 10_000 }).toBe(true);
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            sel.value = 's0-seed-emergent-ic1';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        await expect.poll(() => page.evaluate(() => !!window.__ftdDeBroglieClockPanel), { timeout: 5_000 }).toBe(false);
        expect(await page.evaluate(() => !!document.getElementById('de-broglie-clock-panel'))).toBe(false);
    });
});
