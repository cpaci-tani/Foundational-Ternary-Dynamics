import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

test('paused worker resolves a complete selected-voxel neighbourhood after stepping and reloading', async ({ page }) => {
    test.setTimeout(180000);
    const errors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90000 });
    await page.selectOption('#lattice-size', '97');

    const prepare = async () => {
        await selectScale0Scenario(page, 'flux-pulse');
        await expect.poll(() => page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const s = getScale0State();
            const b = s.useFluxMock ? s.fluxMock : window.__ftdCtx.bridge;
            return b?.isWorker === true && b.ready && b.lifecycleDebug.configurationToken === b.lifecycleDebug.appliedConfigurationToken;
        }), { timeout: 60000 }).toBe(true);
        await page.evaluate(() => {
            const c = window.__ftdCtx;
            c.pauseSimulation();
            c.appShell.panelDock.setCollapsed(false);
            c.appShell.panelDock.activate('inspector');
            if (!c.inspector.selectLatticePosition({ x: 48, y: 48, z: 48 })) throw new Error('No voxel selection');
        });
        await expect.poll(() => page.evaluate(() => window.__ftdCtx.inspector.bridge.runningStateSettled)).toBe(true);
    };
    const complete = async () => {
        await expect.poll(() => page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const s = getScale0State();
            const c = window.__ftdCtx;
            const b = s.useFluxMock ? s.fluxMock : c.bridge;
            if (c.inspector.bridge !== b) return false;
            const cache = c.inspector._latticeInspectionCache;
            if (!cache?.voxel || cache.neighbours.size !== 26) return false;
            const points = [{ x: 48, y: 48, z: 48 }, ...cache.neighbourOrder];
            return points.every(p => {
                const m = b.getInspectionSampleMeta(p.x, p.y, p.z);
                return m && m.sampleTick === b.currentTick() && m.stale === false
                    && m.configurationToken === b.lifecycleDebug.appliedConfigurationToken;
            });
        }), { timeout: 30000, intervals: [100, 250, 500] }).toBe(true);
        await expect(page.locator('#insp-moore-grid .inspector-observation-scope')).toContainText('27');
        return page.evaluate(() => ({
            tick: window.__ftdCtx.inspector.bridge.currentTick(),
            configuration: window.__ftdCtx.inspector.bridge.lifecycleDebug.appliedConfigurationToken,
        }));
    };

    await prepare();
    const first = await complete();
    await page.click('#btn-step');
    await expect.poll(() => page.evaluate(() => window.__ftdCtx.inspector.bridge.currentTick())).toBeGreaterThan(first.tick);
    const stepped = await complete();
    expect(stepped.configuration).toBe(first.configuration);
    await prepare();
    const reloaded = await complete();
    expect(reloaded.configuration).not.toBe(first.configuration);
    await switchMode(page, 'particles');
    await expect.poll(() => page.evaluate(() => {
        const c = window.__ftdCtx;
        return c.inspector.bridge === c.bridge && c.inspector.bridge.isWorker !== true;
    })).toBe(true);
    await switchMode(page, 'lattice');
    await prepare();
    await complete();
    expect(realErrors(errors)).toEqual([]);
});
