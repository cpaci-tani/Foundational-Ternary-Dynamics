import { test, expect } from '@playwright/test';
import { bootDashboard, openDockPanel, selectScale0Scenario, attachConsoleWatcher, realErrors } from './_helpers.js';

async function openLagrangian(page) {
    await bootDashboard(page);
    if (await page.locator('#gpu-card-close').isVisible()) await page.locator('#gpu-card-close').click();
    await selectScale0Scenario(page, 'flux-pulse');
    await openDockPanel(page, 'lagrangian');
    await page.waitForFunction(async () => {
        const { telemetryHub } = await import('/js/telemetry-hub.js');
        return telemetryHub.getScale0TelemetryMeta('lagrangian')?.stale === false;
    });
}

test('Lagrangian selector governs action statistics and trends as well as term charts', async ({ page }) => {
    await openLagrangian(page);
    await page.evaluate(() => window.__ftdCtx.pauseSimulation());
    await page.waitForFunction(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        return getScale0State().fluxMock.runningStateSettled;
    });
    const result = await page.evaluate(async () => {
        const { telemetryHub: h } = await import('/js/telemetry-hub.js');
        const panel = document.getElementById('panel-lagrangian')._ftdLagrangianPanel;
        const table = panel.tables[0];
        // Deterministic samples with irregular spacing catch sample-count
        // windows incorrectly masquerading as the selected tick window.
        h._s0_lag.clear();
        for (const [tick, value] of [[0,1000],[100,100],[190,10],[200,20]]) {
            h._s0_lag.push({ action: value, fieldKinetic: value }, tick);
        }
        const spark = table.sparkEntries.find(e => e.buffer === h.lag.action);
        table.mountSpark(spark);
        panel.historyControl.setTicks(10);
        panel.historyControl.setMode('window');
        table.invalidateHistoryWindow(); panel.update();
        const narrow = {
            min: table.cells.get('action:min').title, max: table.cells.get('action:max').title,
            avg: table.cells.get('action:avg').title, xs: [...spark.spark.uplot.data[0]],
        };
        panel.historyControl.setMode('all');
        const all = { avg: table.cells.get('action:avg').title, xs: [...spark.spark.uplot.data[0]] };
        return { narrow, all, sameControl: table.historyControl === panel.historyControl,
            first: panel.el.firstElementChild === panel.historyControl.el,
            retained: h.lag.action.count, tableCount: panel.tables.length };
    });
    expect(result).toEqual({ narrow: { min: '10', max: '20', avg: '15', xs: [190,200] },
        all: { avg: '282.5', xs: [0,100,190,200] }, sameControl: true, first: true,
        retained: 4, tableCount: 1 });
    await expect(page.locator('#panel-lagrangian')).not.toContainText('Ontic Constants');
});

test('Lagrangian lazily mounts offscreen charts, restores term order and supports fullscreen', async ({ page }) => {
    const errors = attachConsoleWatcher(page);
    await page.setViewportSize({ width: 1100, height: 620 });
    await openLagrangian(page);
    const panel = page.locator('#panel-lagrangian');
    await panel.locator('.lag-term-card').first().scrollIntoViewIfNeeded();
    await expect.poll(() => page.evaluate(() => {
        const cards = [...document.getElementById('panel-lagrangian')._ftdLagrangianPanel.cards.values()];
        return cards.some(c => c.onScreen && !!c.chart) && cards.some(c => !c.onScreen && !c.chart);
    })).toBe(true);
    await panel.locator('.lag-term-card[data-term="dissipation"]').scrollIntoViewIfNeeded();
    await expect(panel.locator('.lag-term-card[data-term="dissipation"] .uplot')).toBeVisible();
    await panel.locator('.lag-term-toggle[data-term="fieldKinetic"] input').uncheck();
    await expect(panel.locator('.lag-term-card[data-term="fieldKinetic"]')).toHaveCount(0);
    await panel.locator('.lag-term-toggle[data-term="fieldKinetic"] input').check();
    await expect(panel.locator('.lag-term-card').first()).toHaveAttribute('data-term','fieldKinetic');
    const card = panel.locator('.lag-term-card').first();
    await card.scrollIntoViewIfNeeded();
    await expect(card.locator('.uplot')).toBeVisible();
    await card.locator('.chart-card-expand').click();
    await expect(page.locator('#chart-fullscreen-overlay')).toHaveClass(/is-open/);
    await expect(page.locator('#chart-fullscreen-overlay .uplot')).toBeVisible();
    await page.keyboard.press('Escape');
    await expect(panel.locator('.lag-term-card').first()).toHaveAttribute('data-term','fieldKinetic');
    expect(realErrors(errors)).toEqual([]);
});

test('direct WASM samples Lagrangian at the display-rate budget with source-consistent ticks', async ({ page }) => {
    await page.addInitScript(() => { window.__ftdWasmWorker = false; });
    const errors = attachConsoleWatcher(page);
    await openLagrangian(page);
    if (await page.locator('#btn-play').getAttribute('data-paused') === 'true') await page.locator('#btn-play').click();
    const result = await page.evaluate(async () => {
        const { telemetryHub } = await import('/js/telemetry-hub.js');
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const state = getScale0State(), owner = state.useFluxMock ? state.fluxMock : window.__ftdCtx.bridge;
        let reads = 0;
        const original = owner.getLagrangian;
        owner.getLagrangian = function (...args) { reads++; return original.apply(this, args); };
        const startTick = owner.currentTick();
        const observed = [];
        try {
            for (let i = 0; i < 60; i++) {
                await new Promise(resolve => requestAnimationFrame(resolve));
                const meta = telemetryHub.getScale0TelemetryMeta('lagrangian');
                observed.push({ version: meta?.stateVersion, tick: meta?.sampleTick });
            }
        } finally { owner.getLagrangian = original; }
        return { reads, worker: !!owner.isWorker, ticks: owner.currentTick() - startTick,
            observed, retained: telemetryHub.lag.total.count };
    });
    expect(result.worker).toBe(false);
    // App collection is display-budgeted at 30 Hz. The direct bridge still
    // preserves a real sampled tick rather than fabricating the skipped frames.
    expect(result.reads).toBeGreaterThanOrEqual(20);
    expect(result.reads).toBeLessThanOrEqual(35);
    expect(result.ticks).toBeGreaterThan(result.reads);
    expect(new Set(result.observed.map(row => row.version)).size).toBeLessThan(result.observed.length);
    const fresh = result.observed.filter((row, index, rows) => index === 0
        || row.version !== rows[index - 1].version);
    expect(fresh.length).toBeGreaterThanOrEqual(20);
    for (let i = 1; i < fresh.length; i++) {
        expect(fresh[i].version).toBeGreaterThan(fresh[i - 1].version);
        expect(fresh[i].tick).toBeGreaterThan(fresh[i - 1].tick);
    }
    for (let i = 1; i < result.observed.length; i++) {
        if (result.observed[i].version === result.observed[i - 1].version) {
            expect(result.observed[i].tick).toBe(result.observed[i - 1].tick);
        }
    }
    expect(realErrors(errors)).toEqual([]);
});
