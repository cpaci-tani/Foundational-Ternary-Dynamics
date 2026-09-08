import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario, attachConsoleWatcher, realErrors } from './_helpers.js';

test('paused worker publishes Lagrangian-only demand boundaries without advancing physics', async ({ page }) => {
    test.setTimeout(150000);
    const errors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90000 });
    await page.selectOption('#lattice-size', '97');
    await selectScale0Scenario(page, 'flux-pulse');
    await expect.poll(() => page.evaluate(async () => {
        const { getScale0State, isScale0AuthoritativeGenerationReady } = await import('/js/scales/scale0/state/store.js');
        const s = getScale0State(), owner = s.useFluxMock ? s.fluxMock : window.__ftdCtx.bridge;
        return owner?.isWorker === true && owner.ready && owner.latticeSize === 97
            && isScale0AuthoritativeGenerationReady(s)
            && owner.lifecycleDebug.configurationToken === owner.lifecycleDebug.appliedConfigurationToken;
    }), { timeout: 60000 }).toBe(true);
    await page.evaluate(() => {
        window.__ftdCtx.pauseSimulation();
        window.__ftdCtx.appShell.panelDock.setCollapsed(false);
        window.__ftdCtx.appShell.panelDock.activate('diagnostics');
    });
    await expect.poll(() => page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        return getScale0State().fluxMock.runningStateSettled;
    })).toBe(true);

    try {
        await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const owner = getScale0State().fluxMock;
            // Isolate a controlled producer demand from the application's UI
            // mask publisher; the actual proxy method/worker handler still run.
            const original = owner.setTelemetryMask;
            window.__lagDemandProbe = { owner, original,
                mask: value => original.call(owner, true, value, false) };
            owner.setTelemetryMask = () => {};
            window.__lagDemandProbe.mask(false);
        });
        await expect.poll(() => page.evaluate(() => {
            const b = window.__lagDemandProbe.owner;
            return b.getScale0TelemetryGroupMeta('audit')?.status === 'available'
                && b.getScale0TelemetryGroupMeta('lagrangian')?.status === 'inactive';
        })).toBe(true);
        const baseline = await page.evaluate(() => {
            const b = window.__lagDemandProbe.owner;
            return { tick: b.currentTick(), dataVersion: b.dataVersion,
                generation: b.lifecycleDebug.appliedConfigurationToken,
                version: b.getScale0TelemetryGroupMeta('lagrangian').stateVersion };
        });
        await page.evaluate(() => window.__lagDemandProbe.mask(true));
        await expect.poll(() => page.evaluate(() => {
            const b = window.__lagDemandProbe.owner;
            return b.getScale0TelemetryGroupMeta('lagrangian')?.status === 'available' && b.getLagrangian() !== null;
        })).toBe(true);
        const active = await page.evaluate(() => {
            const b = window.__lagDemandProbe.owner;
            return { tick: b.currentTick(), dataVersion: b.dataVersion,
                generation: b.lifecycleDebug.appliedConfigurationToken,
                meta: b.getScale0TelemetryGroupMeta('lagrangian') };
        });
        expect(active.tick).toBe(baseline.tick);
        expect(active.dataVersion).toBe(baseline.dataVersion);
        expect(active.generation).toBe(baseline.generation);
        expect(active.meta.sourceEpoch).toBe(baseline.generation);
        expect(active.meta.sampleTick).toBe(baseline.tick);
        expect(active.meta.stateVersion).toBeGreaterThan(baseline.version);
        await page.evaluate(async () => {
            for (let i = 0; i < 4; i++) window.__lagDemandProbe.mask(true);
            await new Promise(resolve => setTimeout(resolve, 250));
        });
        expect(await page.evaluate(() => window.__lagDemandProbe.owner.getScale0TelemetryGroupMeta('lagrangian').stateVersion)).toBe(active.meta.stateVersion);

        await page.evaluate(() => window.__lagDemandProbe.mask(false));
        await expect.poll(() => page.evaluate(() => {
            const b = window.__lagDemandProbe.owner;
            return b.getScale0TelemetryGroupMeta('lagrangian')?.status === 'inactive' && b.getLagrangian() === null;
        })).toBe(true);
        const inactive = await page.evaluate(() => {
            const b = window.__lagDemandProbe.owner;
            return { tick: b.currentTick(), dataVersion: b.dataVersion,
                meta: b.getScale0TelemetryGroupMeta('lagrangian') };
        });
        expect(inactive.meta.stale).toBe(true);
        expect(inactive.meta.stateVersion).toBeGreaterThan(active.meta.stateVersion);
        expect(inactive.tick).toBe(baseline.tick); expect(inactive.dataVersion).toBe(baseline.dataVersion);
        expect(realErrors(errors)).toEqual([]);
    } finally {
        await page.evaluate(() => {
            const p = window.__lagDemandProbe;
            if (p) { p.owner.setTelemetryMask = p.original; p.original.call(p.owner, true, false, false); }
            delete window.__lagDemandProbe;
        });
    }
});
