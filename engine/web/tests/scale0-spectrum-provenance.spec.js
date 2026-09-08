import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario, attachConsoleWatcher, realErrors } from './_helpers.js';

// End-to-end numerical bookkeeping and demand lifecycle, not a microscopic
// continuum certificate: Parseval concerns only the panel's resampled grid.
for (const size of [33, 97]) {
    test(`Spectrum qualified observations and asynchronous Deep Measure L=${size}`, async ({ page }) => {
        test.setTimeout(180000);
        if (size === 33) await page.addInitScript(() => { window.__ftdWasmWorker = false; });
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90000 });
        await page.selectOption('#lattice-size', String(size));
        await selectScale0Scenario(page, 'flux-pulse');
        await expect.poll(() => page.evaluate(async size => {
            const { getScale0State, isScale0AuthoritativeGenerationReady } = await import('/js/scales/scale0/state/store.js');
            const s = getScale0State();
            const b = s.useFluxMock ? s.fluxMock : window.__ftdCtx.bridge;
            return b?.ready && b.isWasm && b.latticeSize === size
                && (size === 97 ? b.isWorker === true : b.isWorker !== true)
                && isScale0AuthoritativeGenerationReady(s)
                && (!b.lifecycleDebug || b.lifecycleDebug.configurationToken === b.lifecycleDebug.appliedConfigurationToken);
        }, size), { timeout: 60000 }).toBe(true);
        await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const s = getScale0State();
            window.__spectrumTestOwner = s.useFluxMock ? s.fluxMock : window.__ftdCtx.bridge;
            const c = window.__ftdCtx;
            c.pauseSimulation();
            c.appShell.panelDock.setCollapsed(false);
            c.appShell.panelDock.activate('spectrum');
        });
        await expect.poll(() => page.evaluate(() => window.__spectrumTestOwner.runningStateSettled !== false)).toBe(true);
        await expect.poll(() => page.evaluate(() => {
            const s = window.__ftdSpectrumPanel?.lastSpec;
            return s && s.spec.E.some(x => x > 0) ? s.M : null;
        }), { timeout: 30000 }).toBe(size === 33 ? 32 : 8);
        await expect(page.locator('#spectrum-panel-energy .spec-observation-scope')).toContainText(/Audit tick \d+; diagnostics tick \d+/);
        const live = await page.evaluate(() => ({
            parseval: window.__ftdSpectrumPanel.lastSpec.parseval,
            tick: window.__spectrumTestOwner.currentTick(),
        }));
        expect(Math.abs(live.parseval - 1)).toBeLessThan(0.05);
        await page.evaluate(() => window.__ftdSpectrumPanel.deepMeasure());
        await expect.poll(() => page.evaluate(() => window.__ftdSpectrumPanel?.lastSpec?.M), { timeout: 12000 }).toBe(64);
        const deep = await page.evaluate(() => ({
            parseval: window.__ftdSpectrumPanel.lastSpec.parseval,
            mode: window.__ftdSpectrumPanel.mode,
            tick: window.__spectrumTestOwner.currentTick(),
            available: window.__spectrumTestOwner.capabilities.scale0.hasScale0SamplerSnapshot('fluxVector', 1),
        }));
        expect(deep.mode).toBe('deep');
        expect(deep.available).toBe(true);
        expect(deep.tick).toBe(live.tick);
        expect(Math.abs(deep.parseval - 1)).toBeLessThan(0.05);

        // Cancel through the real scenario path before the deferred computation.
        await page.click('#spectrum-panel-live');
        await page.evaluate(() => {
            window.__ftdSpectrumPanel.deepMeasure();
            const select = document.getElementById('scenario-select');
            select.value = 'flux-pulse';
            select.dispatchEvent(new Event('change', { bubbles: true }));
        });
        await expect.poll(() => page.evaluate(() => window.__ftdSpectrumPanel?.mode)).toBe('live');
        await expect.poll(() => page.evaluate(() => window.__ftdSpectrumPanel?.lastSpec?.M), { timeout: 30000 }).toBe(size === 33 ? 32 : 8);
        expect(await page.locator('#spectrum-panel-deep').isEnabled()).toBe(true);
        expect(realErrors(errors)).toEqual([]);
    });
}
