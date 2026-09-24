// @ts-check
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    bootDashboard,
    openDockPanel,
    realErrors,
    selectScale0Scenario,
} from './_helpers.js';

async function waitForQualifiedGravity(page, latticeSize, scenarioId) {
    await expect.poll(() => page.evaluate(async ({ latticeSize, scenarioId }) => {
        const store = await import('/js/scales/scale0/state/store.js');
        const state = store.getScale0State();
        const owner = store.getActiveScale0Bridge(window.__ftdCtx, state);
        return state.currentScenarioId === scenarioId
            && owner?.isWorker === true
            && owner?.ready === true
            && Number(owner.latticeSize) === latticeSize
            && store.isScale0AuthoritativeGenerationReady(state);
    }, { latticeSize, scenarioId }), {
        timeout: 90_000,
        message: `qualified Gravity owner did not become ready at L=${latticeSize}`,
    }).toBe(true);
}

async function waitForCoherentObservation(page) {
    await expect.poll(() => page.evaluate(() => {
        const api = window.__ftdGravityPanel;
        const panel = document.getElementById('gravity-panel');
        const telemetry = document.getElementById('gravity-panel-telemetry');
        const delta = document.getElementById('gravity-panel-delta');
        const tiles = [...document.querySelectorAll('.grav-tile canvas')];
        const ticks = [api?.sampleTick, panel?.dataset.sampleTick,
            telemetry?.dataset.sampleTick, delta?.dataset.sampleTick,
            ...tiles.map(canvas => canvas.dataset.sampleTick)].map(Number);
        return api?.telemetryState === 'ready'
            && ticks.length === 7
            && Number.isSafeInteger(ticks[0])
            && ticks.every(tick => tick === ticks[0]);
    }), { timeout: 30_000, message: 'Gravity did not publish one exact tick across every surface' })
        .toBe(true);
}

async function readCoherentSurface(page) {
    await page.evaluate(() => new Promise(resolve => requestAnimationFrame(
        () => requestAnimationFrame(resolve),
    )));
    return page.evaluate(() => {
        const api = window.__ftdGravityPanel;
        const panel = document.getElementById('gravity-panel');
        const telemetry = document.getElementById('gravity-panel-telemetry');
        const delta = document.getElementById('gravity-panel-delta');
        const canvases = [...document.querySelectorAll('.grav-tile canvas')];
        const toTick = value => value === undefined ? null : Number(value);
        return {
            api: api?.sampleTick ?? null,
            panel: toTick(panel?.dataset.sampleTick),
            telemetry: toTick(telemetry?.dataset.sampleTick),
            delta: toTick(delta?.dataset.sampleTick),
            tiles: canvases.map(canvas => toTick(canvas.dataset.sampleTick)),
            tileLabels: [...document.querySelectorAll('.grav-tile-tick')]
                .map(node => node.textContent?.trim()),
            observationStatus: panel?.querySelector('.grav-observation-status')?.textContent?.trim(),
            provenance: telemetry?.querySelector('[data-grav-sampler-provenance]')?.textContent?.trim(),
            stamp: api?.observationStamp ?? null,
        };
    });
}

async function prepareGravity(page, latticeSize) {
    await page.setViewportSize({ width: 1600, height: 1100 });
    await bootDashboard(page, { engine: 'wasm', timeout: 90_000 });
    if (await page.locator('#gpu-card-close').isVisible()) await page.locator('#gpu-card-close').click();
    await page.selectOption('#lattice-size', String(latticeSize));
    await selectScale0Scenario(page, 's0-seed-massive-body', { settleMs: 0 });
    await waitForQualifiedGravity(page, latticeSize, 's0-seed-massive-body');
    await openDockPanel(page, 'gravity');
    if (await page.locator('#btn-play').getAttribute('data-paused') === 'true') {
        await page.locator('#btn-play').click();
    }
    await waitForCoherentObservation(page);
}

async function monitorGravityPaints(page, durationMs = 1_100) {
    return page.evaluate(async durationMs => {
        const prototype = CanvasRenderingContext2D.prototype;
        const original = prototype.drawImage;
        const rows = [];
        prototype.drawImage = function monitoredGravityDraw(...args) {
            const match = /^gravity-panel-tile-(\d)$/.exec(this.canvas?.id || '');
            if (match) rows.push({
                axis: Number(match[1]),
                tick: Number(this.canvas.dataset.sampleTick),
                at: performance.now(),
            });
            return original.apply(this, args);
        };
        try {
            await new Promise(resolve => setTimeout(resolve, durationMs));
        } finally {
            prototype.drawImage = original;
        }
        return rows;
    }, durationMs);
}

for (const latticeSize of [33, 97]) {
    test(`Gravity L${latticeSize} paints every square from one exact live observation`, async ({ page }) => {
        test.setTimeout(150_000);
        const errors = attachConsoleWatcher(page);
        await prepareGravity(page, latticeSize);

        const paints = await monitorGravityPaints(page);
        expect(paints.length, 'at least one complete three-plane repaint').toBeGreaterThanOrEqual(3);
        expect(paints.length % 3, 'paint calls are complete atomic trios').toBe(0);
        const counts = [0, 1, 2].map(axis => paints.filter(row => row.axis === axis).length);
        expect(counts[0], 'each plane painted during the live interval').toBeGreaterThan(0);
        expect(new Set(counts).size, 'all three planes have identical paint counts').toBe(1);
        for (let index = 0; index < paints.length; index += 3) {
            const trio = paints.slice(index, index + 3);
            expect(trio.map(row => row.axis), `paint trio ${index / 3} axis order`).toEqual([0, 1, 2]);
            expect(Number.isSafeInteger(trio[0].tick), `paint trio ${index / 3} has an exact tick`).toBe(true);
            expect(new Set(trio.map(row => row.tick)).size,
                `paint trio ${index / 3} uses one observation tick`).toBe(1);
        }

        const surface = await readCoherentSurface(page);
        expect(Number.isSafeInteger(surface.api)).toBe(true);
        expect([surface.panel, surface.telemetry, surface.delta, ...surface.tiles])
            .toEqual(Array(6).fill(surface.api));
        expect(surface.tileLabels).toEqual(Array(3).fill(`tick ${surface.api}`));
        expect(surface.observationStatus).toContain(`Sample tick ${surface.api}`);
        expect(surface.provenance).toContain(`Sample tick ${surface.api}`);
        expect(surface.stamp).toMatch(new RegExp(`:${surface.api}$`));
        expect(realErrors(errors)).toEqual([]);

        if (latticeSize !== 33) return;

        // Pause through the public transport control, then prove presentation
        // changes repaint only the retained coherent observation.
        if (await page.locator('#btn-play').getAttribute('data-paused') !== 'true') {
            await page.locator('#btn-play').click();
        }
        await expect.poll(() => page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            const state = store.getScale0State();
            const owner = store.getActiveScale0Bridge(window.__ftdCtx, state);
            return window.__ftdCtx.running === false && owner?.runningStateSettled === true;
        }), { timeout: 15_000 }).toBe(true);
        await page.waitForTimeout(350);
        const heldTick = (await readCoherentSurface(page)).api;

        const quantitySnapshots = [];
        for (const kind of ['latency', 'kretschmann', 'force', 'dilation']) {
            await page.locator(`.grav-qbtn[data-kind="${kind}"]`).click();
            quantitySnapshots.push(await page.evaluate(kind => {
                const api = window.__ftdGravityPanel;
                const buttons = [...document.querySelectorAll('.grav-qbtn')];
                const legend = document.querySelector('.grav-slice-legend');
                return {
                    kind,
                    activeKind: api?.activeKind,
                    apiTick: api?.sampleTick,
                    panelTick: Number(document.getElementById('gravity-panel')?.dataset.sampleTick),
                    tileTicks: [...document.querySelectorAll('.grav-tile canvas')]
                        .map(canvas => Number(canvas.dataset.sampleTick)),
                    readouts: [...document.querySelectorAll('.grav-tile-readout')]
                        .map(node => node.textContent?.trim()),
                    pressed: buttons.filter(button => button.getAttribute('aria-pressed') === 'true')
                        .map(button => button.dataset.kind),
                    legendHidden: legend?.hidden,
                    legend: legend?.textContent?.trim(),
                    ramp: getComputedStyle(legend.querySelector('.grav-color-ramp')).backgroundImage,
                };
            }, kind));
        }
        // Exercise the exported controller path independently of delegated
        // button clicks, including its visual/ARIA synchronization contract.
        await page.evaluate(() => window.__ftdGravityPanel.setKind('latency'));
        const apiSelection = await page.evaluate(() => ({
            activeKind: window.__ftdGravityPanel.activeKind,
            pressed: [...document.querySelectorAll('.grav-qbtn')]
                .filter(button => button.getAttribute('aria-pressed') === 'true')
                .map(button => button.dataset.kind),
            ticks: [...document.querySelectorAll('.grav-tile canvas')]
                .map(canvas => Number(canvas.dataset.sampleTick)),
        }));
        for (const snapshot of quantitySnapshots) {
            expect(snapshot.activeKind).toBe(snapshot.kind);
            expect(snapshot.apiTick).toBe(heldTick);
            expect(snapshot.panelTick).toBe(heldTick);
            expect(snapshot.tileTicks).toEqual(Array(3).fill(heldTick));
            expect(snapshot.readouts).toHaveLength(3);
            expect(snapshot.readouts.every(value => /^max\s+\S+/.test(value))).toBe(true);
            expect(snapshot.pressed).toEqual([snapshot.kind]);
            expect(snapshot.legendHidden).toBe(false);
            expect(snapshot.legend).toContain('shared scale');
            expect(snapshot.ramp).not.toBe('none');
        }
        expect(apiSelection).toEqual({
            activeKind: 'latency', pressed: ['latency'], ticks: Array(3).fill(heldTick),
        });

        await page.evaluate(() => window.__ftdGravityPanel.setKind('invalid'));
        expect(await page.evaluate(() => ({
            activeKind: window.__ftdGravityPanel.activeKind,
            pressed: [...document.querySelectorAll('.grav-qbtn')]
                .filter(button => button.getAttribute('aria-pressed') === 'true')
                .map(button => button.dataset.kind),
            ticks: [...document.querySelectorAll('.grav-tile canvas')]
                .map(canvas => Number(canvas.dataset.sampleTick)),
        }))).toEqual(apiSelection);

        const historyControl = page.locator(
            '.gravity-applicable-content > .tick-history-control[data-history-control="gravity-panel"]',
        );
        await expect(historyControl).toHaveCount(1);
        expect(await page.evaluate(() => {
            const content = document.querySelector('.gravity-applicable-content');
            return content?.firstElementChild?.matches(
                '.tick-history-control[data-history-control="gravity-panel"]',
            ) === true;
        }), 'Gravity history selector remains the first applicable-content child').toBe(true);

        await historyControl.locator('[data-history-mode="all"]').click();
        await page.evaluate(() => new Promise(resolve => requestAnimationFrame(resolve)));
        const allHistory = await page.evaluate(() => ({
            paths: [...document.querySelectorAll('#gravity-panel-delta .grav-spark path')]
                .map(path => path.getAttribute('d')),
            ranges: [...document.querySelectorAll('#gravity-panel-delta .grav-spark-range')]
                .map(node => node.textContent?.trim()),
            commands: [...document.querySelectorAll('#gravity-panel-delta .grav-spark path')]
                .map(path => (path.getAttribute('d')?.match(/[ML]/g) || []).length),
            allPressed: document.querySelector('#gravity-panel [data-history-mode="all"]')
                ?.getAttribute('aria-pressed'),
        }));
        expect(allHistory.paths).toHaveLength(4);
        expect(allHistory.ranges).toHaveLength(4);
        expect(allHistory.commands.every(count => count >= 2),
            'All history draws multiple retained observations').toBe(true);
        expect(allHistory.ranges.every(range => range?.startsWith('ticks ')),
            'All history exposes the retained tick range').toBe(true);
        expect(allHistory.allPressed).toBe('true');

        await historyControl.locator('[data-history-mode="window"]').click();
        // The shared control enforces a minimum 10-tick viewport. Entering 1
        // through the real input/change path clamps to 10; live Gravity
        // observations are farther apart here, so only the held tick remains.
        const historyTicks = historyControl.locator('input[aria-label="Ticks shown in rolling chart window"]');
        await historyTicks.fill('1');
        await historyTicks.dispatchEvent('change');
        await expect(historyTicks).toHaveValue('10');
        await page.evaluate(() => new Promise(resolve => requestAnimationFrame(resolve)));
        const lastHistory = await page.evaluate(() => ({
            ranges: [...document.querySelectorAll('#gravity-panel-delta .grav-spark-range')]
                .map(node => node.textContent?.trim()),
            tickBounds: [...document.querySelectorAll('#gravity-panel-delta .grav-spark-range')]
                .map(node => {
                    const match = /^ticks?\s+(\d+)(?:–(\d+))?\s+·/.exec(node.textContent?.trim() || '');
                    return match ? { min: Number(match[1]), max: Number(match[2] || match[1]) } : null;
                }),
            commands: [...document.querySelectorAll('#gravity-panel-delta .grav-spark path')]
                .map(path => (path.getAttribute('d')?.match(/[ML]/g) || []).length),
            windowPressed: document.querySelector('#gravity-panel [data-history-mode="window"]')
                ?.getAttribute('aria-pressed'),
        }));
        expect(lastHistory.tickBounds).toHaveLength(4);
        expect(lastHistory.tickBounds.every(bounds => bounds
            && bounds.max === heldTick && bounds.min >= heldTick - 10)).toBe(true);
        expect(lastHistory.commands.every((count, index) => count <= allHistory.commands[index]),
            'Last 10 ticks never draw more observations than All').toBe(true);
        expect(lastHistory.windowPressed).toBe('true');

        await historyControl.locator('[data-history-mode="all"]').click();
        await page.evaluate(() => new Promise(resolve => requestAnimationFrame(resolve)));
        const restoredAll = await page.evaluate(() => ({
            paths: [...document.querySelectorAll('#gravity-panel-delta .grav-spark path')]
                .map(path => path.getAttribute('d')),
            ranges: [...document.querySelectorAll('#gravity-panel-delta .grav-spark-range')]
                .map(node => node.textContent?.trim()),
        }));
        expect(restoredAll).toEqual({ paths: allHistory.paths, ranges: allHistory.ranges });

        const chartSemantics = await page.evaluate(() => ({
            sparks: [...document.querySelectorAll('#gravity-panel-delta .grav-spark')]
                .map(svg => svg.getAttribute('aria-label')),
            sparkRanges: [...document.querySelectorAll('#gravity-panel-delta .grav-spark-range')]
                .map(node => node.textContent?.trim()),
            histograms: [...document.querySelectorAll('#gravity-panel-telemetry .grav-mini-hist')]
                .map(svg => svg.getAttribute('aria-label')),
            histogramRanges: [...document.querySelectorAll('#gravity-panel-telemetry [data-grav-hist-range]')]
                .map(node => node.textContent?.trim()),
        }));
        expect(chartSemantics.sparks).toHaveLength(4);
        expect(chartSemantics.sparks.every(label => /tick|ticks/.test(label || ''))).toBe(true);
        expect(chartSemantics.sparkRanges.every(label => /tick|ticks/.test(label || ''))).toBe(true);
        expect(chartSemantics.histograms).toHaveLength(3);
        expect(chartSemantics.histograms.every(label => /sampled values; range/.test(label || ''))).toBe(true);
        expect(chartSemantics.histogramRanges.every(label => /\S+–\S+/.test(label || ''))).toBe(true);

        const responsive = await page.evaluate(async () => {
            const dock = window.__ftdCtx.appShell.panelDock;
            const floating = dock.floatPanel('gravity', 200, 50);
            const reports = [];
            for (const [width, expectedColumns] of [[340, 1], [600, 2], [900, 3]]) {
                floating.el.style.width = `${width}px`;
                floating.triggerChartResize();
                await new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve)));
                const panel = document.getElementById('gravity-panel');
                const grid = panel.querySelector('.grav-slice-tiles');
                const tileWidths = [...grid.querySelectorAll('.grav-tile')]
                    .map(tile => tile.getBoundingClientRect().width);
                reports.push({
                    width,
                    expectedColumns,
                    columns: getComputedStyle(grid).gridTemplateColumns.split(/\s+/).filter(Boolean).length,
                    tileWidths,
                    noHorizontalOverflow: panel.scrollWidth <= panel.clientWidth + 1,
                });
            }
            return reports;
        });
        for (const report of responsive) {
            expect(report.columns, `${report.width}px floating grid columns`).toBe(report.expectedColumns);
            expect(report.noHorizontalOverflow, `${report.width}px floating panel overflow`).toBe(true);
            expect(report.tileWidths.every(width => width >= 100 && width <= 301)).toBe(true);
        }

        await selectScale0Scenario(page, 'empty', { settleMs: 0 });
        await expect.poll(() => page.evaluate(() => ({
            applicability: window.__ftdGravityPanel?.applicability,
            tick: window.__ftdGravityPanel?.sampleTick,
            history: window.__ftdGravityPanel?.historyLength,
            wants: window.__ftdGravityPanel?.samplerWantsActive,
            coordinator: window.__ftdGravityPanel?.coordinatorActive,
            contentHidden: document.querySelector('.gravity-applicable-content')?.hidden,
            messageVisible: !document.querySelector('.gravity-inapplicable')?.hidden,
            paintedTicks: [...document.querySelectorAll('.grav-tile canvas')]
                .map(canvas => canvas.dataset.sampleTick ?? null),
        })), { timeout: 15_000 }).toEqual({
            applicability: 'inapplicable-empty', tick: null, history: 0,
            wants: false, coordinator: false, contentHidden: true,
            messageVisible: true, paintedTicks: [null, null, null],
        });

        await selectScale0Scenario(page, 'flux-pulse', { settleMs: 0 });
        await waitForQualifiedGravity(page, 33, 'flux-pulse');
        if (await page.locator('#btn-play').getAttribute('data-paused') === 'true') {
            await page.locator('#btn-play').click();
        }
        await waitForCoherentObservation(page);
        const replacement = await readCoherentSurface(page);
        expect(Number.isSafeInteger(replacement.api)).toBe(true);
        expect([replacement.panel, replacement.telemetry, replacement.delta, ...replacement.tiles])
            .toEqual(Array(6).fill(replacement.api));

        const hiddenBoundary = await page.evaluate(async () => {
            const [{ floatingWindowManager }, store, { getScale0TelemetryDemand }] = await Promise.all([
                import('/js/ui/components/floating-window/component.js'),
                import('/js/scales/scale0/state/store.js'),
                import('/js/telemetry/demand.js'),
            ]);
            floatingWindowManager.getWindow('gravity')?.toggleCollapse();
            const api = window.__ftdGravityPanel;
            return {
                live: api.liveCoordinatorActive,
                wants: api.samplerWantsActive,
                wantGravity: getScale0TelemetryDemand(window.__ftdCtx, store.getScale0State()).wantGravity,
            };
        });
        expect(hiddenBoundary).toEqual({ live: false, wants: false, wantGravity: false });
        expect(realErrors(errors)).toEqual([]);
    });
}
