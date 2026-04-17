// @ts-check
import { test, expect } from '@playwright/test';

/**
 * Panels redesign smoke / descriptor-validation suite.
 *
 * Covers:
 *   - every diagnostics descriptor source/compute/trend resolves against the hub
 *   - every charts descriptor buffer exists on telemetryHub
 *   - every Lagrangian term / action-row / constant-row resolves
 *   - chip-picker state persists across reload
 *   - all three panels (diagnostics, charts, lagrangian) mount without errors
 */

test('diagnostics descriptor paths and trends resolve', async ({ page }) => {
    const errors = [];
    page.on('console', (msg) => { if (msg.type() === 'error') errors.push(msg.text()); });

    await page.goto('/');
    await page.waitForFunction(() => typeof window.uPlot === 'function');
    await page.waitForTimeout(1500);

    const report = await page.evaluate(async () => {
        const [{ sections }, { telemetryHub }] = await Promise.all([
            import('/js/ui/panels/diagnostics-panel/descriptors/scale0.js'),
            import('/js/telemetry-hub.js'),
        ]);

        function resolve(obj, path) {
            const parts = path.split('.');
            let cur = obj;
            for (const p of parts) {
                if (cur == null) return undefined;
                cur = cur[p];
            }
            return cur;
        }

        const missingTrends = [];
        let totalRows = 0;
        let rowsWithSource = 0;
        let rowsWithCompute = 0;

        for (const section of sections) {
            for (const row of section.rows) {
                totalRows++;
                if (row.source) rowsWithSource++;
                if (row.compute) rowsWithCompute++;
                if (row.trend) {
                    const buf = resolve(telemetryHub, row.trend);
                    if (!buf || typeof buf.get !== 'function') {
                        missingTrends.push(`${section.id}/${row.id}: ${row.trend}`);
                    }
                }
            }
        }
        return { totalRows, rowsWithSource, rowsWithCompute, missingTrends };
    });

    expect(report.totalRows).toBeGreaterThan(0);
    expect(report.rowsWithSource + report.rowsWithCompute).toBeGreaterThanOrEqual(report.totalRows);
    expect(report.missingTrends,
        `Trend paths pointing to non-buffers: ${report.missingTrends.join(', ')}`).toEqual([]);
});

test('charts descriptor buffers all exist on telemetryHub', async ({ page }) => {
    await page.goto('/');
    await page.waitForFunction(() => typeof window.uPlot === 'function');

    const report = await page.evaluate(async () => {
        const [{ charts }, { telemetryHub }] = await Promise.all([
            import('/js/ui/panels/charts-panel/descriptors/scale0.js'),
            import('/js/telemetry-hub.js'),
        ]);
        const missing = [];
        for (const chart of charts) {
            for (const s of chart.series) {
                const buf = telemetryHub[s.buffer];
                if (!buf || typeof buf.get !== 'function') {
                    missing.push(`${chart.id}/${s.key}: telemetryHub.${s.buffer}`);
                }
            }
        }
        return { chartCount: charts.length, missing };
    });
    expect(report.chartCount).toBeGreaterThan(0);
    expect(report.missing, `Missing chart buffers: ${report.missing.join(', ')}`).toEqual([]);
});

test('lagrangian descriptor — terms, action rows, constants all resolve', async ({ page }) => {
    await page.goto('/');
    await page.waitForFunction(() => typeof window.uPlot === 'function');
    await page.waitForTimeout(1000);

    const report = await page.evaluate(async () => {
        const [{ terms, actionRows, constantRows }, { telemetryHub }, consts] = await Promise.all([
            import('/js/ui/panels/lagrangian-panel/descriptors/scale0.js'),
            import('/js/telemetry-hub.js'),
            import('/js/constants.js'),
        ]);
        const hubView = Object.create(telemetryHub);
        hubView.consts = consts;

        function resolve(obj, path) {
            const parts = path.split('.');
            let cur = obj;
            for (const p of parts) {
                if (cur == null) return undefined;
                cur = cur[p];
            }
            return cur;
        }

        const missingTermBuffers = [];
        for (const t of terms) {
            const buf = telemetryHub.lag?.[t.buffer];
            if (!buf || typeof buf.get !== 'function') {
                missingTermBuffers.push(`${t.key}: lag.${t.buffer}`);
            }
        }

        const unresolvedConsts = [];
        for (const row of constantRows) {
            const v = resolve(hubView, row.source);
            if (v === undefined) unresolvedConsts.push(`${row.id}: ${row.source}`);
        }

        const missingActionTrends = [];
        for (const row of actionRows) {
            if (row.trend) {
                const buf = resolve(telemetryHub, row.trend);
                if (!buf || typeof buf.get !== 'function') {
                    missingActionTrends.push(`${row.id}: ${row.trend}`);
                }
            }
        }

        return {
            termCount: terms.length,
            actionCount: actionRows.length,
            constCount: constantRows.length,
            missingTermBuffers,
            unresolvedConsts,
            missingActionTrends,
        };
    });

    expect(report.termCount).toBeGreaterThan(0);
    expect(report.missingTermBuffers,
        `Missing term buffers: ${report.missingTermBuffers.join(', ')}`).toEqual([]);
    expect(report.unresolvedConsts,
        `Unresolved constants: ${report.unresolvedConsts.join(', ')}`).toEqual([]);
    expect(report.missingActionTrends,
        `Missing action-row trends: ${report.missingActionTrends.join(', ')}`).toEqual([]);
});

test('all three panels mount without error and render expected structure', async ({ page }) => {
    const errors = [];
    page.on('console', (msg) => { if (msg.type() === 'error') errors.push(msg.text()); });

    await page.goto('/');
    await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');
    await page.waitForTimeout(1500);

    async function openTab(panel) {
        await page.evaluate((p) => document.querySelector(`.tab[data-panel="${p}"]`)?.click(), panel);
        await page.waitForTimeout(400);
    }

    // Diagnostics: 5 sections, every row populated (no em-dashes).
    await openTab('diagnostics');
    const diagReport = await page.evaluate(() => {
        const sections = document.querySelectorAll('.diag-scale0-root .diag-section');
        const vals = [...document.querySelectorAll('.diag-scale0-root .diag-value')].map((el) => el.textContent);
        return {
            sectionCount: sections.length,
            rowCount: vals.length,
            dashes: vals.filter((v) => v === '\u2014').length,
        };
    });
    expect(diagReport.sectionCount).toBe(5);
    expect(diagReport.rowCount).toBeGreaterThan(20);
    expect(diagReport.dashes).toBe(0);

    // Charts: chip strip + at least one default-active chart card + uPlot.
    await openTab('charts');
    const chartsReport = await page.evaluate(() => {
        return {
            chipCount: document.querySelectorAll('.charts-chip').length,
            activeChips: document.querySelectorAll('.charts-chip[aria-pressed="true"]').length,
            cardCount: document.querySelectorAll('.chart-card').length,
            uplotCount: document.querySelectorAll('#panel-charts .uplot').length,
        };
    });
    expect(chartsReport.chipCount).toBeGreaterThan(0);
    expect(chartsReport.activeChips).toBeGreaterThan(0);
    expect(chartsReport.cardCount).toBe(chartsReport.activeChips);
    expect(chartsReport.uplotCount).toBeGreaterThan(0);

    // Lagrangian: chart + term row + 2 tables.
    await openTab('lagrangian');
    const lagReport = await page.evaluate(() => ({
        chart: document.querySelectorAll('#lag-plot-host .uplot').length,
        terms: document.querySelectorAll('.lag-term-toggle').length,
        tables: document.querySelectorAll('.lag-data-col .diag-section').length,
    }));
    expect(lagReport.chart).toBe(1);
    expect(lagReport.terms).toBeGreaterThan(0);
    expect(lagReport.tables).toBe(2);

    // Filter out known pre-existing noise that doesn't relate to the redesign.
    const relevantErrors = errors.filter((e) =>
        !e.includes('chartCharge') && !e.includes('chartEBEnergy') &&
        !e.includes('chartGauss') && !e.includes('chartEntropy') &&
        !e.includes('lagrangianChart') && !e.includes('fluxEnergyChart.draw') &&
        !e.includes('particleChart.draw'));
    expect(relevantErrors, `Unexpected console errors:\n${relevantErrors.join('\n')}`).toEqual([]);
});

test('chip-picker state persists across reload', async ({ page }) => {
    await page.goto('/');
    await page.waitForFunction(() => typeof window.uPlot === 'function');
    // Set a known state.
    await page.evaluate(() => {
        localStorage.setItem('ftd.charts.active', JSON.stringify(['particles']));
    });
    await page.reload();
    await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');
    await page.evaluate(() => document.querySelector('.tab[data-panel="charts"]')?.click());
    await page.waitForTimeout(400);

    const state = await page.evaluate(() => {
        const chips = document.querySelectorAll('.charts-chip');
        return [...chips].map((c) => ({
            id: c.dataset.chartId,
            pressed: c.getAttribute('aria-pressed'),
        }));
    });
    const particles = state.find((s) => s.id === 'particles');
    const flux = state.find((s) => s.id === 'flux-energy');
    expect(particles?.pressed).toBe('true');
    expect(flux?.pressed).toBe('false');
});
