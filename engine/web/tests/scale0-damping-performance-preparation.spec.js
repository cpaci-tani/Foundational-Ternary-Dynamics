import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario, attachConsoleWatcher, realErrors } from './_helpers.js';
import { prepareDampingPerformanceProfile, readDampingPerformanceSnapshot } from './scale0-damping-performance-preparation.js';

for (const size of [33, 97]) {
    test(`Damping modified-profile preparation retains its owner and 12-particle support at L=${size}`, async ({ page }) => {
        test.setTimeout(180000);
        if (size === 33) await page.addInitScript(() => { window.__ftdWasmWorker = false; });
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90000 });
        await page.selectOption('#lattice-size', String(size));
        await selectScale0Scenario(page, 's0-seed-sloop', { settleMs: 0 });
        const preparation = await prepareDampingPerformanceProfile(page);
        const read = () => page.evaluate(readDampingPerformanceSnapshot);
        const owner = await page.evaluateHandle(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const s = getScale0State(); return s.useFluxMock ? s.fluxMock : window.__ftdCtx.bridge;
        });
        const stableProfile = snapshot => {
            expect(snapshot.size).toBe(size); expect(snapshot.isWorker).toBe(size === 97);
            expect(snapshot.terms).toEqual(preparation.modified.terms);
            expect(snapshot.qualification).toEqual(preparation.modified.qualification);
            expect(snapshot.ready).toBe(true); expect(snapshot.particleCount).toBe(12);
            expect(snapshot.lifecycle?.configurationToken ?? null).toBe(preparation.modified.lifecycle?.configurationToken ?? null);
            expect(snapshot.lifecycle?.appliedConfigurationToken ?? null).toBe(preparation.modified.lifecycle?.appliedConfigurationToken ?? null);
        };
        try {
            await page.evaluate(() => {
                const button = document.getElementById('toggle-damping-zones');
                if (!button || button.classList.contains('is-inapplicable')) throw new Error('Damping is unavailable');
                if (!button.classList.contains('active')) button.click();
            });
            await expect.poll(async () => { const s = await read(); return s.visible && s.drawCount === 288; }, { timeout: 30000 }).toBe(true);
            const paused = await read(); stableProfile(paused); expect(paused.paused && paused.settled).toBe(true);
            await page.waitForTimeout(500);
            const pausedLater = await read(); stableProfile(pausedLater); expect(pausedLater.tick).toBe(paused.tick);
            await page.click('#btn-play');
            await expect.poll(async () => { const s = await read(); return !s.paused && s.settled && s.tick > paused.tick; }, { timeout: 30000 }).toBe(true);
            const playing = await read(); stableProfile(playing); expect(playing.drawCount).toBe(288);
            await page.evaluate(async original => {
                const { getScale0State } = await import('/js/scales/scale0/state/store.js');
                const s = getScale0State();
                if ((s.useFluxMock ? s.fluxMock : window.__ftdCtx.bridge) !== original) throw new Error('Damping run replaced its owner');
            }, owner);
            expect(realErrors(errors)).toEqual([]);
        } finally {
            await page.evaluate(() => {
                window.__ftdCtx?.pauseSimulation?.();
                const button = document.getElementById('toggle-damping-zones');
                if (button?.classList.contains('active') && !button.classList.contains('is-inapplicable')) button.click();
            });
            await owner.dispose();
        }
    });
}
