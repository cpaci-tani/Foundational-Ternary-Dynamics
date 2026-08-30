// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, attachConsoleWatcher, realErrors } from './_helpers.js';

const PANEL_CASES = [
    { id: 'charts', component: 'chartsPanel' },
    { id: 'diagnostics', component: 'diagnosticsPanel' },
    { id: 'telemetry-grid', component: 'telemetryGridPanel' },
    { id: 'lagrangian', component: 'lagrangianPanel' },
];

test('visible side-panel charts consume live samples at a smooth presentation cadence', async ({ page }) => {
    test.setTimeout(90_000);
    const consoleErrors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 60_000 });
    await page.evaluate(() => {
        const play = document.getElementById('btn-play');
        if (play?.getAttribute('data-paused') === 'true') play.click();
    });

    const reports = {};
    for (const panelCase of PANEL_CASES) {
        await page.locator(`.tab[data-panel="${panelCase.id}"]`).click();
        await page.waitForTimeout(500);
        reports[panelCase.id] = await page.evaluate(async ({ component }) => {
            const { telemetryHub } = await import('/js/telemetry-hub.js');
            const panel = window.__ftdCtx?.[component];
            let plot = null;
            let buffer = null;
            if (component === 'chartsPanel') {
                const entry = panel?.cards?.get('flux-energy');
                plot = entry?.chart?.uplot;
                buffer = telemetryHub.flux;
            } else if (component === 'diagnosticsPanel') {
                const entry = panel?.tables
                    ?.flatMap(table => table.sparkEntries || [])
                    .find(candidate => candidate.spark?.uplot && candidate.buffer);
                plot = entry?.spark?.uplot;
                buffer = entry?.buffer;
            } else if (component === 'telemetryGridPanel') {
                const entry = panel?.charts?.get('flux');
                plot = entry?.u;
                buffer = telemetryHub.flux;
            } else if (component === 'lagrangianPanel') {
                const entry = [...(panel?.cards?.values?.() || [])][0];
                plot = entry?.chart?.uplot;
                buffer = telemetryHub.lag[entry?.term?.buffer];
            }
            if (!plot || !buffer) throw new Error(`${component} visible chart is unavailable`);

            const commits = [];
            const sourceTimes = [];
            const originalSetData = plot.setData.bind(plot);
            plot.setData = (...args) => {
                commits.push(performance.now());
                return originalSetData(...args);
            };
            const sourceStart = buffer.total ?? buffer.count;
            let observedTotal = sourceStart;
            const start = performance.now();
            await new Promise(resolve => {
                const end = start + 1500;
                const frame = (now) => {
                    const total = buffer.total ?? buffer.count;
                    if (total !== observedTotal) {
                        sourceTimes.push(now);
                        observedTotal = total;
                    }
                    if (now < end) requestAnimationFrame(frame);
                    else resolve();
                };
                requestAnimationFrame(frame);
            });
            plot.setData = originalSetData;

            const sourceEnd = buffer.total ?? buffer.count;
            const sourceAdvances = sourceEnd - sourceStart;
            const gaps = commits.slice(1).map((time, index) => time - commits[index]);
            gaps.sort((a, b) => a - b);
            return {
                sourceAdvances,
                commits: commits.length,
                coverage: sourceAdvances > 0 ? commits.length / sourceAdvances : 0,
                medianGapMs: gaps.length ? gaps[Math.floor(gaps.length / 2)] : null,
                sourceEvents: sourceTimes.length,
            };
        }, panelCase);
    }

    console.log('side-panel chart cadence', JSON.stringify(reports));
    for (const [panel, report] of Object.entries(reports)) {
        expect(report.sourceAdvances, `${panel} source advances`).toBeGreaterThanOrEqual(6);
        expect(report.commits, `${panel} visible chart commits`).toBeGreaterThanOrEqual(5);
        expect(report.coverage, `${panel} rendered/source coverage`).toBeGreaterThanOrEqual(0.7);
    }
    expect(realErrors(consoleErrors)).toEqual([]);
});
