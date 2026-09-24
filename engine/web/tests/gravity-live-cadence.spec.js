// @ts-check
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    bootDashboard,
    openDockPanel,
    realErrors,
    selectScale0Scenario,
} from './_helpers.js';
import { measureGravityCadence } from './_gravity-cadence.js';

async function waitForGravitySurface(page) {
    await page.waitForFunction(() => {
        const api = window.__ftdGravityPanel;
        const root = document.getElementById('gravity-panel');
        const ticks = [api?.sampleTick, root?.dataset.sampleTick,
            document.getElementById('gravity-panel-telemetry')?.dataset.sampleTick,
            document.getElementById('gravity-panel-delta')?.dataset.sampleTick,
            ...[...root?.querySelectorAll?.('.grav-tile canvas') || []]
                .map(canvas => canvas.dataset.sampleTick)].map(Number);
        return api?.telemetryState === 'ready'
            && ticks.length === 7
            && Number.isSafeInteger(ticks[0])
            && ticks.every(tick => tick === ticks[0])
            && [...root.querySelectorAll('#gravity-panel-delta .grav-spark path')]
                .every(path => (path.getAttribute('d') || '').length > 0);
    }, undefined, { timeout: 30_000 });
}

function totalRows(groups) {
    return Object.values(groups).reduce((total, rows) => total + rows.length, 0);
}

const cases = [
    { backend: 'worker', layout: 'docked', latticeSize: 33, minRate: 18, minConsumption: 0.8 },
    { backend: 'worker', layout: 'floating', latticeSize: 33, minRate: 18, minConsumption: 0.8 },
    // Direct WASM budgets synchronous display reads against a 16.7 ms target
    // with a 25% duty ceiling. Ten fresh observations/s preserves responsive
    // motion without requiring it to consume every physics frame.
    { backend: 'direct', layout: 'docked', latticeSize: 17, minRate: 10, minConsumption: 0.5 },
];

for (const row of cases) {
    const { backend, layout, latticeSize, minRate, minConsumption } = row;
    test(`${backend} WASM ${layout} Gravity keeps three heatmaps and four traces on the shared source`, async ({ page }) => {
        test.setTimeout(150_000);
        const errors = attachConsoleWatcher(page);
        await page.setViewportSize({ width: 1440, height: 1400 });
        if (backend === 'direct') {
            await page.addInitScript(() => { window.__ftdWasmWorker = false; });
        }
        await bootDashboard(page, { engine: 'wasm', timeout: 90_000 });
        if (await page.locator('#gpu-card-close').isVisible()) await page.locator('#gpu-card-close').click();
        await page.selectOption('#lattice-size', String(latticeSize));
        await selectScale0Scenario(page, 'flux-pulse', { settleMs: 0 });
        await openDockPanel(page, 'gravity');
        if (await page.locator('#btn-play').getAttribute('data-paused') === 'true') {
            await page.locator('#btn-play').click();
        }
        if (layout === 'floating') {
            await page.evaluate(() => {
                const dock = window.__ftdCtx.appShell.panelDock;
                const floating = dock.floatPanel('gravity', 200, 50);
                floating.el.style.width = '600px';
                floating.el.style.height = '1200px';
                dock.activate('controls');
                floating.triggerChartResize();
            });
        }
        await waitForGravitySurface(page);

        const live = await page.evaluate(measureGravityCadence, 2500);
        expect(Object.keys(live.heatmaps)).toEqual(['0', '1', '2']);
        expect(Object.keys(live.sparks)).toEqual(['Lmax', 'Kmax', 'Fmean', 'dil']);
        const heatCounts = Object.values(live.heatmaps).map(rows => rows.length);
        const sparkCounts = Object.values(live.sparks).map(rows => rows.length);
        for (const [axis, rows] of Object.entries(live.heatmaps)) {
            expect(rows.length / live.elapsed * 1000, `axis ${axis} actual heatmap paint rate`)
                .toBeGreaterThan(minRate);
            expect(rows.every(row => Number.isSafeInteger(row.tick)
                && row.tick === row.sourceTick), `axis ${axis} matches shared observation tick`).toBe(true);
        }
        for (const [key, rows] of Object.entries(live.sparks)) {
            expect(rows.length / live.elapsed * 1000, `${key} actual path commit rate`)
                .toBeGreaterThan(minRate);
            expect(rows.every(row => Number.isSafeInteger(row.tick)
                && row.tick === row.sourceTick), `${key} matches shared observation tick`).toBe(true);
        }
        expect(Math.max(...heatCounts) - Math.min(...heatCounts),
            'three heatmaps paint as one observation').toBeLessThanOrEqual(1);
        expect(Math.max(...sparkCounts) - Math.min(...sparkCounts),
            'four spark paths commit as one observation').toBeLessThanOrEqual(1);
        expect(live.source.length).toBeGreaterThan(0);
        expect(Math.min(...heatCounts) / live.source.length,
            'heatmaps consume the available shared observations').toBeGreaterThan(minConsumption);
        expect(Math.min(...sparkCounts) / live.source.length,
            'sparks consume the available shared observations').toBeGreaterThan(minConsumption);
        expect(live.surfaces.length).toBeGreaterThan(0);
        for (const surface of live.surfaces) {
            const shared = [surface.panelTick, surface.telemetryTick, surface.deltaTick,
                ...surface.tileTicks, ...surface.sparkTicks];
            expect(shared, `visible surface at tick ${surface.apiTick}`)
                .toEqual(Array(shared.length).fill(surface.apiTick));
        }

        if (layout === 'floating') {
            await page.evaluate(async () => {
                const { floatingWindowManager } = await import('/js/ui/components/floating-window/component.js');
                floatingWindowManager.getWindow('gravity')?.toggleCollapse();
            });
        } else {
            await page.evaluate(() => window.__ftdCtx.appShell.panelDock.activate('controls'));
        }
        await page.evaluate(() => new Promise(resolve => requestAnimationFrame(
            () => requestAnimationFrame(resolve),
        )));
        const hidden = await page.evaluate(measureGravityCadence, 650);
        expect(totalRows(hidden.heatmaps), 'hidden Gravity heatmap work').toBe(0);
        expect(totalRows(hidden.sparks), 'hidden Gravity spark work').toBe(0);
        expect(await page.evaluate(() => window.__ftdGravityPanel.samplerWantsActive)).toBe(false);

        if (layout === 'floating') {
            await page.evaluate(async () => {
                const { floatingWindowManager } = await import('/js/ui/components/floating-window/component.js');
                floatingWindowManager.getWindow('gravity')?.toggleCollapse();
            });
        } else {
            await openDockPanel(page, 'gravity');
        }
        await waitForGravitySurface(page);
        if (await page.locator('#btn-play').getAttribute('data-paused') !== 'true') {
            await page.locator('#btn-play').click();
        }
        await expect.poll(() => page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            const state = store.getScale0State();
            const owner = store.getActiveScale0Bridge(window.__ftdCtx, state);
            return window.__ftdCtx.running === false
                && (owner?.isWorker !== true || owner?.runningStateSettled === true);
        }), { timeout: 15_000 }).toBe(true);
        // Consume the final already-posted worker observation before probing.
        await page.waitForTimeout(250);
        const paused = await page.evaluate(measureGravityCadence, 650);
        expect(paused.tickEnd).toBe(paused.tickStart);
        expect(totalRows(paused.heatmaps), 'paused stable state does not repaint heatmaps').toBe(0);
        expect(totalRows(paused.sparks), 'paused stable state does not recommit spark paths').toBe(0);
        expect(realErrors(errors)).toEqual([]);
    });
}
