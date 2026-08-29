// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

test.describe('Telemetry sidepanel audit gate', () => {
    test.beforeEach(async ({ page }) => {
        test.setTimeout(90_000);
        await page.setViewportSize({ width: 1440, height: 640 });
        await page.addInitScript(() => {
            localStorage.removeItem('ftd.panel.side-width');
            localStorage.removeItem('ftd.panel.rail-width');
            localStorage.removeItem('ftd-panels-collapsed');
        });
        await gotoAndReady(page);
        await page.evaluate(async () => {
            const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
            writePanelMount('left');
            const dock = window.__ftdCtx?.appShell?.panelDock;
            dock?.setCollapsed(false);
            dock?.activate('telemetry-grid');
            await new Promise((resolve) => requestAnimationFrame(resolve));
        });
    });

    test('rail scroll, label expansion, and panel edge resizing are independent and persistent', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const initial = await page.evaluate(() => ({
            rail: document.getElementById('tab-bar')?.getBoundingClientRect().width || 0,
            panel: document.getElementById('panel-area')?.getBoundingClientRect().width || 0,
            railHandleCount: document.querySelectorAll('#panel-rail-resizer').length,
            sideHandleCount: document.querySelectorAll('#panel-side-resizer').length,
        }));

        expect(initial.railHandleCount).toBe(1);
        expect(initial.sideHandleCount).toBe(1);
        expect(initial.rail).toBeGreaterThanOrEqual(43);

        await page.locator('#panel-rail-resizer').focus();
        await page.keyboard.press('End');
        await page.waitForTimeout(250); // allow the intentional rail-width transition to settle
        const expanded = await page.evaluate(() => {
            const rail = document.getElementById('tab-bar');
            const label = rail?.querySelector('.tab[data-panel="telemetry-grid"] .tab-label');
            return {
                width: rail?.getBoundingClientRect().width || 0,
                expanded: rail?.dataset.railExpanded,
                labelDisplay: label ? getComputedStyle(label).display : 'missing',
                stored: localStorage.getItem('ftd.panel.rail-width'),
                safeLeft: parseFloat(getComputedStyle(document.getElementById('viewport')).getPropertyValue('--viewport-safe-left')),
            };
        });
        expect(expanded.width).toBeGreaterThanOrEqual(170);
        expect(expanded.expanded).toBe('true');
        expect(expanded.labelDisplay).toBe('block');
        expect(Number(expanded.stored)).toBeGreaterThanOrEqual(170);
        expect(expanded.safeLeft).toBeGreaterThan(initial.panel + initial.rail);

        const railGesture = await page.evaluate(async () => {
            const rail = document.getElementById('tab-bar');
            const tab = rail?.querySelector('.tab[data-panel="telemetry-grid"]');
            if (!rail || !tab) throw new Error('Rail fixture missing');
            rail.scrollTop = 0;
            const beforeWindows = document.querySelectorAll('.floating-window').length;
            tab.dispatchEvent(new PointerEvent('pointerdown', {
                bubbles: true, cancelable: true, button: 0, buttons: 1,
                pointerId: 41, pointerType: 'mouse', clientX: 30, clientY: 560,
            }));
            window.dispatchEvent(new PointerEvent('pointermove', {
                bubbles: true, cancelable: true, buttons: 1,
                pointerId: 41, pointerType: 'mouse', clientX: 32, clientY: 220,
            }));
            window.dispatchEvent(new PointerEvent('pointerup', {
                bubbles: true, pointerId: 41, pointerType: 'mouse', clientX: 32, clientY: 220,
            }));
            await new Promise((resolve) => requestAnimationFrame(resolve));
            return {
                scrollTop: rail.scrollTop,
                scrollable: rail.scrollHeight > rail.clientHeight,
                beforeWindows,
                afterWindows: document.querySelectorAll('.floating-window').length,
                scrollingClass: rail.classList.contains('is-rail-scrolling'),
            };
        });
        expect(railGesture.scrollable).toBe(true);
        expect(railGesture.scrollTop).toBeGreaterThan(0);
        expect(railGesture.afterWindows).toBe(railGesture.beforeWindows);
        expect(railGesture.scrollingClass).toBe(false);

        const touchGesture = await page.evaluate(() => {
            const rail = document.getElementById('tab-bar');
            const tab = rail?.querySelector('.tab[data-panel="telemetry-grid"]');
            if (!rail || !tab) throw new Error('Touch rail fixture missing');
            const beforeWindows = document.querySelectorAll('.floating-window').length;
            tab.dispatchEvent(new PointerEvent('pointerdown', {
                bubbles: true, cancelable: true, button: 0, buttons: 1,
                pointerId: 42, pointerType: 'touch', clientX: 30, clientY: 520,
            }));
            const move = new PointerEvent('pointermove', {
                bubbles: true, cancelable: true, buttons: 1,
                pointerId: 42, pointerType: 'touch', clientX: 32, clientY: 260,
            });
            window.dispatchEvent(move);
            const scrollingDuringMove = rail.classList.contains('is-rail-scrolling');
            window.dispatchEvent(new PointerEvent('pointercancel', {
                bubbles: true, pointerId: 42, pointerType: 'touch', clientX: 32, clientY: 260,
            }));
            return {
                defaultPrevented: move.defaultPrevented,
                scrollingDuringMove,
                scrollingAfterCancel: rail.classList.contains('is-rail-scrolling'),
                beforeWindows,
                afterWindows: document.querySelectorAll('.floating-window').length,
            };
        });
        expect(touchGesture.defaultPrevented).toBe(false);
        expect(touchGesture.scrollingDuringMove).toBe(true);
        expect(touchGesture.scrollingAfterCancel).toBe(false);
        expect(touchGesture.afterWindows).toBe(touchGesture.beforeWindows);

        const resized = await page.evaluate(async () => {
            const handle = document.getElementById('panel-side-resizer');
            const panel = document.getElementById('panel-area');
            if (!handle || !panel) throw new Error('Side resize fixture missing');
            const box = handle.getBoundingClientRect();
            const before = panel.getBoundingClientRect().width;
            handle.dispatchEvent(new PointerEvent('pointerdown', {
                bubbles: true, cancelable: true, button: 0, buttons: 1,
                pointerId: 52, pointerType: 'mouse', clientX: box.left + 4, clientY: box.top + 120,
            }));
            handle.dispatchEvent(new PointerEvent('pointermove', {
                bubbles: true, cancelable: true, buttons: 1,
                pointerId: 52, pointerType: 'mouse', clientX: box.left + 124, clientY: box.top + 120,
            }));
            await new Promise((resolve) => requestAnimationFrame(resolve));
            handle.dispatchEvent(new PointerEvent('pointerup', {
                bubbles: true, pointerId: 52, pointerType: 'mouse', clientX: box.left + 124, clientY: box.top + 120,
            }));
            await new Promise((resolve) => requestAnimationFrame(resolve));
            return {
                before,
                after: panel.getBoundingClientRect().width,
                stored: Number(localStorage.getItem('ftd.panel.side-width')),
                activeDrag: window.__ftdCtx?.appShell?.panelDock?._drag.active,
                resizeRaf: window.__ftdCtx?.appShell?.panelDock?._resizeRaf,
                bodyCursor: document.body.style.cursor,
            };
        });
        expect(resized.after).toBeGreaterThan(resized.before + 100);
        expect(resized.stored).toBeCloseTo(resized.after, 0);
        expect(resized.activeDrag).toBe(false);
        expect(resized.resizeRaf).toBeNull();
        expect(resized.bodyCursor).toBe('');

        const cycles = await page.evaluate(async () => {
            const dock = window.__ftdCtx?.appShell?.panelDock;
            const railHandle = document.getElementById('panel-rail-resizer');
            const sideHandle = document.getElementById('panel-side-resizer');
            if (!dock || !railHandle || !sideHandle) throw new Error('Resize controller missing');
            for (let i = 0; i < 10; i += 1) {
                dock._toggleRailWidth();
                dock._toggleRailWidth();
                dock.setCollapsed(true);
                dock.setCollapsed(false);
            }
            await new Promise((resolve) => requestAnimationFrame(resolve));
            return {
                railHandles: document.querySelectorAll('#panel-rail-resizer').length,
                sideHandles: document.querySelectorAll('#panel-side-resizer').length,
                activeDrag: dock._drag.active,
                resizeRaf: dock._resizeRaf,
                panelsCollapsed: document.getElementById('app')?.classList.contains('panels-collapsed'),
            };
        });
        expect(cycles).toEqual({
            railHandles: 1,
            sideHandles: 1,
            activeDrag: false,
            resizeRaf: null,
            panelsCollapsed: false,
        });

        const rightMount = await page.evaluate(async () => {
            const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
            const dock = window.__ftdCtx?.appShell?.panelDock;
            const handle = document.getElementById('panel-side-resizer');
            const panel = document.getElementById('panel-area');
            if (!dock || !handle || !panel) throw new Error('Right resize fixture missing');
            writePanelMount('right');
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const box = handle.getBoundingClientRect();
            const before = panel.getBoundingClientRect().width;
            handle.dispatchEvent(new PointerEvent('pointerdown', {
                bubbles: true, cancelable: true, button: 0, buttons: 1,
                pointerId: 63, pointerType: 'mouse', clientX: box.left + 6, clientY: box.top + 100,
            }));
            handle.dispatchEvent(new PointerEvent('pointermove', {
                bubbles: true, cancelable: true, buttons: 1,
                pointerId: 63, pointerType: 'mouse', clientX: box.left - 86, clientY: box.top + 100,
            }));
            await new Promise((resolve) => requestAnimationFrame(resolve));
            handle.dispatchEvent(new PointerEvent('pointerup', {
                bubbles: true, pointerId: 63, pointerType: 'mouse', clientX: box.left - 86, clientY: box.top + 100,
            }));
            await new Promise((resolve) => requestAnimationFrame(resolve));
            return {
                before,
                after: panel.getBoundingClientRect().width,
                mount: document.documentElement.dataset.panelMount,
                activeDrag: dock._drag.active,
            };
        });
        expect(rightMount.mount).toBe('right');
        expect(rightMount.after).toBeGreaterThan(rightMount.before + 70);
        expect(rightMount.activeDrag).toBe(false);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('Telemetry Grid culls off-screen work, coalesces reflow, and goes quiescent when collapsed', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.evaluate(() => {
            const play = document.getElementById('btn-play');
            if (play?.getAttribute('data-paused') === 'true') play.click();
        });
        await page.waitForTimeout(1200);

        const report = await page.evaluate(async () => {
            const grid = window.__ftdCtx?.telemetryGridPanel;
            const dock = window.__ftdCtx?.appShell?.panelDock;
            if (!grid || !dock) throw new Error('Telemetry Grid unavailable');
            grid.update();
            await new Promise((resolve) => requestAnimationFrame(resolve));

            const entries = [...grid.charts.values()];
            const visible = entries.filter((entry) => entry.onScreen && entry.u);
            const offscreen = entries.filter((entry) => !entry.onScreen);

            for (const entry of offscreen) entry.lastDisplayValue = Number.NaN;
            grid._lastDraw = 0;
            grid.update();
            const offscreenTouched = offscreen.filter((entry) => !Number.isNaN(entry.lastDisplayValue)).length;

            let reflows = 0;
            const originalReflow = grid.reflowCharts.bind(grid);
            grid.reflowCharts = (...args) => {
                reflows += 1;
                return originalReflow(...args);
            };
            for (let i = 0; i < 50; i += 1) grid._scheduleReflow();
            await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));
            grid.reflowCharts = originalReflow;

            let draws = 0;
            let valueRefreshes = 0;
            const originalDraw = grid._drawEntry.bind(grid);
            const originalRefresh = grid._refreshValue.bind(grid);
            grid._drawEntry = (...args) => { draws += 1; return originalDraw(...args); };
            grid._refreshValue = (...args) => { valueRefreshes += 1; return originalRefresh(...args); };

            dock.setCollapsed(true);
            for (let i = 0; i < 20; i += 1) {
                grid._lastDraw = 0;
                grid.update();
            }
            const collapsedCounts = { draws, valueRefreshes };
            const collapsedDemand = await import('/js/telemetry/demand.js').then(({ getScale0TelemetryDemand }) =>
                getScale0TelemetryDemand(window.__ftdCtx));

            for (let i = 0; i < 10; i += 1) {
                dock.setCollapsed(false);
                grid._lastDraw = 0;
                grid.update();
                dock.setCollapsed(true);
                grid._lastDraw = 0;
                grid.update();
            }
            dock.setCollapsed(false);
            await new Promise((resolve) => requestAnimationFrame(resolve));

            grid._drawEntry = originalDraw;
            grid._refreshValue = originalRefresh;
            return {
                totalEntries: entries.length,
                builtEntries: entries.filter((entry) => entry.u).length,
                visibleEntries: visible.length,
                offscreenEntries: offscreen.length,
                offscreenTouched,
                reflows,
                collapsedCounts,
                collapsedDemand: {
                    wantAudit: collapsedDemand.wantAudit,
                    wantLag: collapsedDemand.wantLag,
                },
                cards: grid.el.querySelectorAll('.telemetry-card').length,
                mapSize: grid.charts.size,
                resizeObserver: !!grid._ro,
                intersectionObserver: !!grid._io,
                reflowRaf: grid._reflowRaf,
            };
        });

        expect(report.totalEntries).toBeGreaterThan(10);
        expect(report.visibleEntries).toBeGreaterThan(0);
        expect(report.offscreenEntries).toBeGreaterThan(0);
        expect(report.builtEntries).toBeLessThan(report.totalEntries);
        expect(report.offscreenTouched).toBe(0);
        expect(report.reflows).toBe(1);
        expect(report.collapsedCounts).toEqual({ draws: 0, valueRefreshes: 0 });
        expect(report.collapsedDemand).toEqual({ wantAudit: false, wantLag: false });
        expect(report.cards).toBe(report.mapSize);
        expect(report.mapSize).toBe(report.totalEntries);
        expect(report.resizeObserver).toBe(true);
        expect(report.intersectionObserver).toBe(true);
        expect(report.reflowRaf).toBeNull();
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('Telemetry Grid consumes each visible Scale-0 sample without degrading frame pacing', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.evaluate(() => {
            const play = document.getElementById('btn-play');
            if (play?.getAttribute('data-paused') === 'true') play.click();
        });
        await page.waitForTimeout(800);

        const report = await page.evaluate(async () => {
            const { telemetryHub } = await import('/js/telemetry-hub.js');
            const grid = window.__ftdCtx?.telemetryGridPanel;
            const entry = grid?.charts.get('flux');
            if (!grid || !entry?.u || !entry.onScreen) {
                throw new Error('Visible Total Flux sparkline unavailable');
            }

            const commits = [];
            const sourceTimes = [];
            const updateDurations = [];
            const frameDeltas = [];
            const originalSetData = entry.u.setData.bind(entry.u);
            const originalUpdate = grid.update.bind(grid);

            entry.u.setData = (...args) => {
                commits.push(performance.now());
                return originalSetData(...args);
            };
            grid.update = (...args) => {
                const started = performance.now();
                const result = originalUpdate(...args);
                updateDurations.push(performance.now() - started);
                return result;
            };

            const sourceStart = telemetryHub.flux.total ?? telemetryHub.flux.count;
            let observedSourceTotal = sourceStart;
            const measureStart = performance.now();
            await new Promise((resolve) => {
                let previous = performance.now();
                const end = previous + 1800;
                const frame = (now) => {
                    frameDeltas.push(now - previous);
                    previous = now;
                    const currentSourceTotal = telemetryHub.flux.total ?? telemetryHub.flux.count;
                    if (currentSourceTotal !== observedSourceTotal) {
                        sourceTimes.push(now);
                        observedSourceTotal = currentSourceTotal;
                    }
                    if (now < end) requestAnimationFrame(frame);
                    else resolve();
                };
                requestAnimationFrame(frame);
            });
            const measureEnd = performance.now();
            const sourceEnd = telemetryHub.flux.total ?? telemetryHub.flux.count;

            entry.u.setData = originalSetData;
            grid.update = originalUpdate;

            const percentile = (values, p) => {
                if (!values.length) return 0;
                const ordered = [...values].sort((a, b) => a - b);
                return ordered[Math.min(ordered.length - 1, Math.floor(ordered.length * p))];
            };
            const commitGaps = commits.slice(1).map((time, index) => time - commits[index]);
            const sourceGaps = sourceTimes.slice(1).map((time, index) => time - sourceTimes[index]);
            const sourceAdvances = sourceEnd - sourceStart;

            return {
                durationMs: measureEnd - measureStart,
                sourceAdvances,
                commits: commits.length,
                coverage: sourceAdvances > 0 ? commits.length / sourceAdvances : 0,
                commitGapMedianMs: percentile(commitGaps, 0.5),
                commitGapP95Ms: percentile(commitGaps, 0.95),
                sourceGapMedianMs: percentile(sourceGaps, 0.5),
                sourceGapP95Ms: percentile(sourceGaps, 0.95),
                updateP95Ms: percentile(updateDurations, 0.95),
                updateMaxMs: Math.max(0, ...updateDurations),
                frameMedianMs: percentile(frameDeltas, 0.5),
                frameP95Ms: percentile(frameDeltas, 0.95),
            };
        });

        expect(report.sourceAdvances, 'Scale-0 source produced enough samples for a cadence audit').toBeGreaterThanOrEqual(20);
        expect(report.commits, 'visible sparkline redraws').toBeGreaterThanOrEqual(18);
        expect(report.coverage, 'rendered/source sample coverage').toBeGreaterThanOrEqual(0.8);
        expect(report.commitGapMedianMs, 'median chart cadence tracks the source cadence')
            .toBeLessThanOrEqual(report.sourceGapMedianMs * 1.25 + 10);
        expect(report.commitGapP95Ms, 'p95 chart cadence tracks the source cadence')
            .toBeLessThanOrEqual(report.sourceGapP95Ms * 1.35 + 15);
        expect(report.updateP95Ms, 'p95 Telemetry Grid update cost').toBeLessThanOrEqual(4);
        expect(report.updateMaxMs, 'maximum Telemetry Grid update cost').toBeLessThanOrEqual(10);
        expect(report.frameP95Ms, 'p95 frame time stays within a 60 FPS frame budget')
            .toBeLessThanOrEqual(16.9);
        expect(report.frameP95Ms, 'headless frame-time tail stays bounded')
            .toBeLessThanOrEqual(report.frameMedianMs * 1.75 + 2);
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
