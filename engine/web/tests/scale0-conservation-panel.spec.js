// @ts-check
/**
 * Conservation panel ownership regression.
 *
 * flux-* scenarios are owned by the JS MockBridge/worker in the dashboard,
 * not by the main WASM bridge. The always-on conservation panel must sample
 * the active owner or it sits at t=0 and its energy deltas look frozen while
 * the visible flux pulse evolves.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.describe('Conservation panel and WASM diagnostics', () => {
    /** @type {import('@playwright/test').Page} */
    let page;

    test.beforeAll(async ({ browser, baseURL }) => {
        const context = await browser.newContext({ baseURL });
        page = await context.newPage();
        page.setDefaultTimeout(60_000);
        await gotoAndReady(page);
    });

    test.afterAll(async () => {
        await page.close();
    });

    test('conservation panel follows worker-owned flux-pulse ticks', async () => {
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            if (sel) {
                sel.value = 'flux-pulse';
                sel.dispatchEvent(new Event('change', { bubbles: true }));
            }
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
        });

        await expect.poll(
            () => page.locator('#conservation-micropanel-status').textContent(),
            { timeout: 10_000, message: 'conservation panel never advanced on flux-pulse' },
        ).toMatch(/^t=([1-9]\d*)$/);
    });

    test('WASM vacuum diagnostics expose moving physical energy', async () => {
        const snap = await page.evaluate(async () => {
            const sel = document.getElementById('scenario-select');
            if (sel) {
                sel.value = 's0-vacuum-electron';
                sel.dispatchEvent(new Event('change', { bubbles: true }));
            }

            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const st = getScale0State();
            const bridge = (st.useFluxMock && st.fluxMock) ? st.fluxMock : window.__ftdCtx.bridge;
            const caps = bridge.capabilities.scale0;

            const d0 = caps.getScale0Diagnostics();
            const a0 = caps.getScale0EnergyAudit();
            for (let i = 0; i < 20; i++) caps.tickScale0();
            const d20 = caps.getScale0Diagnostics();
            const a20 = caps.getScale0EnergyAudit();

            return {
                owner: st.useFluxMock ? 'mock' : 'wasm',
                e0: d0.totalEnergy,
                e20: d20.totalEnergy,
                audit0: a0.totalEnergy,
                audit20: a20.totalEnergy,
                baseline0: d0.vacuumBaselineEnergy ?? null,
                baseline20: d20.vacuumBaselineEnergy ?? null,
            };
        });

        expect(snap.owner).toBe('wasm');
        expect(Math.abs(snap.e0 - snap.audit0)).toBeLessThan(1e-9);
        expect(Math.abs(snap.e20 - snap.audit20)).toBeLessThan(1e-9);
        expect(snap.e0).not.toBe(snap.e20);
        expect(snap.baseline0).toBeGreaterThan(1000);
        expect(snap.baseline20).toBe(snap.baseline0);
    });
});
