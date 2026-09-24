import { test, expect } from '@playwright/test';
import { bootDashboard, openDockPanel, selectScale0Scenario, attachConsoleWatcher, realErrors } from './_helpers.js';
import { measureLagrangianCadence } from './_lagrangian-cadence.js';

for (const layout of ['docked', 'floating']) {
    test(`all seven ${layout} Lagrangian charts paint fresh shared samples without a timer cap`, async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await page.setViewportSize({ width: 1440, height: 2200 });
        await bootDashboard(page, { engine: 'wasm', timeout: 90000 });
        if (await page.locator('#gpu-card-close').isVisible()) await page.locator('#gpu-card-close').click();
        await page.selectOption('#lattice-size', '33');
        await selectScale0Scenario(page, 'flux-pulse');
        await openDockPanel(page, 'lagrangian');
        if (await page.locator('#btn-play').getAttribute('data-paused') === 'true') await page.locator('#btn-play').click();
        if (layout === 'floating') {
            await page.evaluate(() => {
                const dock = window.__ftdCtx.appShell.panelDock;
                const floating = dock.floatPanel('lagrangian', 200, 50);
                floating.el.style.width = '500px';
                floating.el.style.height = '2100px';
                dock.activate('controls');
                floating.triggerChartResize();
            });
        }
        await page.waitForFunction(() => {
            const cards = [...window.__ftdCtx.lagrangianPanel.cards.values()];
            return cards.length === 7 && cards.every(entry => entry.onScreen && entry.chart?.uplot?.data[0].length > 3);
        });
        const report = await page.evaluate(measureLagrangianCadence, 2500);
        const counts = Object.values(report.commits).map(rows => rows.length);
        expect(Object.keys(report.commits)).toHaveLength(7);
        // These real draw hooks catch a fast viewport with slow/unchanged charts.
        // 18 Hz leaves CI headroom while failing the old 4/8 Hz timer caps.
        for (const [key, rows] of Object.entries(report.commits)) {
            expect(rows.length / report.elapsed * 1000, `${key} commit rate`).toBeGreaterThan(18);
            expect(report.paints[key].length, `${key} actual canvas paints`).toBeGreaterThanOrEqual(rows.length - 1);
            expect(rows.every(row => row.tick === row.sourceTick), `${key} matches shared source tick`).toBe(true);
            expect(rows.length / report.source.length, `${key} consumes available observations`).toBeGreaterThan(0.9);
        }
        expect(Math.max(...counts) - Math.min(...counts), 'seven plots update together').toBeLessThanOrEqual(1);
        expect(realErrors(errors)).toEqual([]);
    });
}
