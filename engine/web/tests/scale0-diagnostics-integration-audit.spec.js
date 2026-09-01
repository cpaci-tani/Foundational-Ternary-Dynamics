// @ts-check
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    gotoAndReady,
    realErrors,
    selectScale0Scenario,
} from './_helpers.js';

test.describe('Scale 0 Diagnostics sidepanel integration gate', () => {
    test('descriptor rows, singleton mount, floated visibility, collapse gating, and cleanup are exact', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        const result = await page.evaluate(async () => {
            const [componentModule, descriptorModule, { floatingWindowManager }] = await Promise.all([
                import('/js/ui/panels/diagnostics-panel/component.js'),
                import('/js/ui/panels/diagnostics-panel/descriptors/scale0.js'),
                import('/js/ui/components/floating-window/component.js'),
            ]);
            const panel = window.__ftdCtx.diagnosticsPanel;
            const tableIdentities = panel.tables.slice();
            const initResults = [];
            for (let i = 0; i < 10; i++) initResults.push(componentModule.initDiagnosticsPanel());

            const expectedRows = descriptorModule.sections.flatMap((section) => (
                section.rows.map((row) => `${section.id}:${row.id}`)
            ));
            const actualRows = panel.tablesByScale['0'].flatMap((table) => (
                [...table.el.querySelectorAll('tbody > tr.diag-data-row')]
                    .map((row) => `${table.section.id}:${row.dataset.row}`)
            ));

            const calls = new Map();
            const originals = new Map();
            for (const table of panel.tablesByScale['0']) {
                originals.set(table, table.update);
                calls.set(table, 0);
                table.update = function (...args) {
                    calls.set(table, calls.get(table) + 1);
                    return originals.get(table).apply(this, args);
                };
            }
            const totalCalls = () => [...calls.values()].reduce((sum, value) => sum + value, 0);
            const dock = window.__ftdCtx.appShell.panelDock;
            document.querySelector('.tab[data-panel="diagnostics"]')?.click();
            let beforeCalls = totalCalls();
            panel.update();
            const activeCalls = totalCalls() - beforeCalls;
            floatingWindowManager.getWindow('diagnostics')?.dock();
            const win = dock.floatPanel('diagnostics', 100, 80);
            beforeCalls = totalCalls();
            panel.update();
            const floatedCalls = totalCalls() - beforeCalls;
            win.toggleCollapse();
            beforeCalls = totalCalls();
            panel.update();
            const collapsedCalls = totalCalls() - beforeCalls;
            win.toggleCollapse();
            beforeCalls = totalCalls();
            panel.update();
            const restoredCalls = totalCalls() - beforeCalls;
            win.dock();
            document.querySelector('.tab[data-panel="controls"]')?.click();
            beforeCalls = totalCalls();
            panel.update();
            const hiddenCalls = totalCalls() - beforeCalls;
            for (const [table, original] of originals) table.update = original;

            const fixture = document.createElement('div');
            document.body.appendChild(fixture);
            const fixtureFirst = new componentModule.DiagnosticsPanelComponent(fixture).init();
            const fixtureTables = fixtureFirst.tables.length;
            fixtureFirst.cleanup();
            const afterCleanup = {
                tables: fixtureFirst.tables.length,
                roots: fixture.querySelectorAll('.diag-scale0-root, .diag-scale1-root, .diag-ae-root').length,
                marker: fixture.dataset.panelRedesignMounted || null,
            };
            const fixtureSecond = new componentModule.DiagnosticsPanelComponent(fixture).init();
            const afterRemount = {
                tables: fixtureSecond.tables.length,
                roots: fixture.querySelectorAll('.diag-scale0-root, .diag-scale1-root, .diag-ae-root').length,
            };
            fixtureSecond.cleanup();
            fixture.remove();

            return {
                singleton: initResults.every((candidate) => candidate === panel),
                retainedIdentity: panel.tables.every((table, index) => table === tableIdentities[index]),
                roots: {
                    scale0: document.querySelectorAll('#panel-diagnostics > .diag-scale0-root').length,
                    scale1: document.querySelectorAll('#panel-diagnostics > .diag-scale1-root').length,
                    ae: document.querySelectorAll('#panel-diagnostics > .diag-ae-root').length,
                },
                expectedRows,
                actualRows,
                tableCount: panel.tablesByScale['0'].length,
                calls: { activeCalls, floatedCalls, collapsedCalls, restoredCalls, hiddenCalls },
                fixture: { fixtureTables, afterCleanup, afterRemount },
            };
        });

        expect(result.singleton).toBe(true);
        expect(result.retainedIdentity).toBe(true);
        expect(result.roots).toEqual({ scale0: 1, scale1: 1, ae: 1 });
        expect(result.actualRows).toEqual(result.expectedRows);
        expect(result.tableCount).toBe(5);
        expect(result.calls).toEqual({
            activeCalls: 5,
            floatedCalls: 5,
            collapsedCalls: 0,
            restoredCalls: 5,
            hiddenCalls: 0,
        });
        expect(result.fixture.fixtureTables).toBeGreaterThan(5);
        expect(result.fixture.afterCleanup).toEqual({ tables: 0, roots: 0, marker: null });
        expect(result.fixture.afterRemount).toEqual({
            tables: result.fixture.fixtureTables,
            roots: 3,
        });
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('L=97 visible diagnostics sustain chart cadence and the formal hardware frame budget', async ({ page }, testInfo) => {
        testInfo.setTimeout(180_000);
        const consoleErrors = attachConsoleWatcher(page);
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
        await selectScale0Scenario(page, 'flux-pulse', { settleMs: 0 });
        const supported = await page.evaluate(() => globalThis.crossOriginIsolated === true
            && !![...document.querySelectorAll('#lattice-size option')]
                .find((option) => option.value === '97' && !option.disabled));
        test.skip(!supported, 'L=97 worker path unavailable');
        await page.selectOption('#lattice-size', '97');
        await expect.poll(async () => page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const state = getScale0State();
            return state.fluxMock?.isWorker === true
                && state.fluxMock?.ready === true
                && state.fluxMock?.latticeSize === 97;
        }), { timeout: 90_000 }).toBe(true);
        await page.evaluate(() => {
            document.querySelector('.tab[data-panel="diagnostics"]')?.click();
            const play = document.getElementById('btn-play');
            if (play?.getAttribute('data-paused') === 'true') play.click();
        });
        await page.waitForTimeout(3_000);

        const report = await page.evaluate(async () => {
            const probe = await import('/tests/scale0-ui-audit-probe.js');
            const panel = window.__ftdCtx.diagnosticsPanel;
            const gl = window.__ftdCtx?.viewport?.renderer?.getContext?.() || null;
            const rendererInfo = gl?.getExtension?.('WEBGL_debug_renderer_info') || null;
            const webglRenderer = rendererInfo
                ? String(gl.getParameter(rendererInfo.UNMASKED_RENDERER_WEBGL) || '')
                : '';
            const samples = [];
            const originalUpdate = panel.update;
            panel.update = function (...args) {
                const started = performance.now();
                try { return originalUpdate.apply(this, args); }
                finally { samples.push(performance.now() - started); }
            };
            const entry = panel.tablesByScale['0']
                .flatMap((table) => table.sparkEntries || [])
                .find((candidate) => candidate.spark?.uplot && candidate.buffer);
            const commits = [];
            const originalSetData = entry.spark.uplot.setData.bind(entry.spark.uplot);
            entry.spark.uplot.setData = (...args) => {
                commits.push(performance.now());
                return originalSetData(...args);
            };
            const sourceStart = entry.buffer.total ?? entry.buffer.count;
            probe.startScale0UiAuditProbe({ rootSelector: '#panel-diagnostics' });
            await new Promise((resolve) => setTimeout(resolve, 12_000));
            const audit = await probe.stopScale0UiAuditProbe();
            const sourceEnd = entry.buffer.total ?? entry.buffer.count;
            entry.spark.uplot.setData = originalSetData;
            panel.update = originalUpdate;
            const sorted = samples.slice().sort((a, b) => a - b);
            const percentile = (q) => sorted.length
                ? sorted[Math.min(sorted.length - 1, Math.ceil(sorted.length * q) - 1)] : 0;
            return {
                ...audit,
                webglRenderer,
                panelUpdates: {
                    count: sorted.length,
                    p95Ms: percentile(0.95),
                    maxMs: sorted.at(-1) || 0,
                },
                chart: {
                    commits: commits.length,
                    sourceAdvances: sourceEnd - sourceStart,
                },
            };
        });
        await testInfo.attach('scale0-diagnostics-performance-report.json', {
            body: Buffer.from(JSON.stringify(report, null, 2)),
            contentType: 'application/json',
        });
        console.log('scale0 diagnostics performance', JSON.stringify(report));

        if (process.env.FTD_HARDWARE_WEBGL === '1') {
            expect(report.webglRenderer, 'release gate exposes a WebGL renderer').not.toBe('');
            expect(report.webglRenderer, 'release gate does not certify SwiftShader/software WebGL')
                .not.toMatch(/swiftshader|software/i);
        }
        expect(report.frames.count).toBeGreaterThanOrEqual(600);
        expect(report.frames.effectiveFps).toBeGreaterThanOrEqual(59.5);
        expect(report.frames.p95Ms).toBeLessThanOrEqual(17);
        expect(report.frames.p99Ms).toBeLessThanOrEqual(20);
        expect(report.frames.intervalsOver33_4ms).toBe(0);
        expect(report.longTasks).toEqual([]);
        expect(report.panelUpdates.count).toBeGreaterThanOrEqual(200);
        expect(report.panelUpdates.p95Ms).toBeLessThanOrEqual(2);
        expect(report.panelUpdates.maxMs).toBeLessThanOrEqual(8);
        expect(report.chart.sourceAdvances).toBeGreaterThanOrEqual(40);
        expect(report.chart.commits / report.chart.sourceAdvances).toBeGreaterThanOrEqual(0.7);
        expect(report.resourceDelta.rafSubscribers).toBe(0);
        expect(report.resourceDelta.domNodes).toBe(0);
        expect(report.resourceDelta.canvases).toBe(0);
        expect(report.errors).toEqual([]);
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
