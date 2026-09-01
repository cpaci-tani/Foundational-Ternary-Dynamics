// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

test.describe('Scale 0 Charts and Lagrangian lifecycle gates', () => {
    test('public initializers are idempotent and direct components cleanly remount', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await gotoAndReady(page, { path: '/?engine=wasm' });

        const result = await page.evaluate(async () => {
            const chartsModule = await import('/js/ui/panels/charts-panel/component.js');
            const lagModule = await import('/js/ui/panels/lagrangian-panel/component.js');
            const liveCharts = chartsModule.initChartsPanel();
            const liveLag = lagModule.initLagrangianPanel();
            const chartIdentities = Array.from({ length: 10 }, () => chartsModule.initChartsPanel());
            const lagIdentities = Array.from({ length: 10 }, () => lagModule.initLagrangianPanel());

            const fixture = document.createElement('div');
            fixture.style.width = '720px';
            document.body.appendChild(fixture);

            const chartFixture = new chartsModule.ChartsPanelComponent(fixture).init();
            const firstChartCount = fixture.querySelectorAll('.chart-card').length;
            chartFixture.cleanup();
            const chartClean = {
                children: fixture.children.length,
                mounted: fixture.dataset.panelRedesignMounted ?? null,
            };
            const chartRemount = new chartsModule.ChartsPanelComponent(fixture).init();
            const secondChartCount = fixture.querySelectorAll('.chart-card').length;
            chartRemount.cleanup();

            const lagFixture = new lagModule.LagrangianPanelComponent(fixture).init();
            const firstLagCount = fixture.querySelectorAll('.lag-term-card').length;
            lagFixture.cleanup();
            const lagClean = {
                children: fixture.children.length,
                mounted: fixture.dataset.panelRedesignMounted ?? null,
            };
            const lagRemount = new lagModule.LagrangianPanelComponent(fixture).init();
            const secondLagCount = fixture.querySelectorAll('.lag-term-card').length;
            lagRemount.cleanup();
            fixture.remove();

            return {
                chartsSingleton: chartIdentities.every((value) => value === liveCharts),
                lagSingleton: lagIdentities.every((value) => value === liveLag),
                liveChartCards: document.querySelectorAll('#panel-charts .chart-card').length,
                liveLagCards: document.querySelectorAll('#panel-lagrangian .lag-term-card').length,
                firstChartCount,
                secondChartCount,
                chartClean,
                firstLagCount,
                secondLagCount,
                lagClean,
            };
        });

        expect(result.chartsSingleton).toBe(true);
        expect(result.lagSingleton).toBe(true);
        expect(result.liveChartCards).toBeGreaterThan(0);
        expect(result.liveLagCards).toBeGreaterThan(0);
        expect(result.firstChartCount).toBeGreaterThan(0);
        expect(result.secondChartCount).toBe(result.firstChartCount);
        expect(result.chartClean).toEqual({ children: 0, mounted: null });
        expect(result.firstLagCount).toBeGreaterThan(0);
        expect(result.secondLagCount).toBe(result.firstLagCount);
        expect(result.lagClean).toEqual({ children: 0, mounted: null });
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
