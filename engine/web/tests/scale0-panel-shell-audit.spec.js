// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors, switchMode } from './_helpers.js';

test.describe('Scale 0 panel shell audit gate', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(90_000);
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page);
    });

    test('floating-window teardown releases every global drag listener', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const { floatingWindowManager } = await import('/js/ui/components/floating-window/component.js');
            const trackedTypes = new Set(['pointermove', 'pointerup', 'pointercancel', 'blur']);
            const adds = new Map();
            const removes = new Map();
            const originalAdd = window.addEventListener;
            const originalRemove = window.removeEventListener;

            window.addEventListener = function (type, listener, options) {
                if (trackedTypes.has(type)) {
                    const key = `${type}:${String(listener)}`;
                    adds.set(key, (adds.get(key) || 0) + 1);
                }
                return originalAdd.call(this, type, listener, options);
            };
            window.removeEventListener = function (type, listener, options) {
                if (trackedTypes.has(type)) {
                    const key = `${type}:${String(listener)}`;
                    removes.set(key, (removes.get(key) || 0) + 1);
                }
                return originalRemove.call(this, type, listener, options);
            };

            const panel = document.createElement('section');
            panel.textContent = 'audit fixture';
            const before = floatingWindowManager.windows.size;
            let win;
            try {
                win = floatingWindowManager.floatPanel(
                    'scale0-shell-audit-fixture',
                    'Audit fixture',
                    'A',
                    panel,
                    { x: 80, y: 80 },
                    () => {},
                );
                win.startDrag(100, 100);
                win.destroy();
            } finally {
                window.addEventListener = originalAdd;
                window.removeEventListener = originalRemove;
                panel.remove();
            }

            const imbalances = [];
            for (const [key, count] of adds) {
                if ((removes.get(key) || 0) !== count) {
                    imbalances.push({ key, added: count, removed: removes.get(key) || 0 });
                }
            }
            return {
                before,
                after: floatingWindowManager.windows.size,
                stillRegistered: floatingWindowManager.has('scale0-shell-audit-fixture'),
                dragActive: win?._drag.active,
                connected: win?.el.isConnected,
                trackedAdds: [...adds.values()].reduce((sum, count) => sum + count, 0),
                imbalances,
            };
        });

        expect(result.trackedAdds).toBe(4);
        expect(result.imbalances).toEqual([]);
        expect(result.after).toBe(result.before);
        expect(result.stillRegistered).toBe(false);
        expect(result.dragActive).toBe(false);
        expect(result.connected).toBe(false);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('ten float, drag, collapse, and dock cycles return to baseline', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const report = await page.evaluate(async () => {
            const probe = await import('/tests/scale0-ui-audit-probe.js');
            const { floatingWindowManager } = await import('/js/ui/components/floating-window/component.js');
            const dock = window.__ftdCtx?.appShell?.panelDock;
            if (!dock) throw new Error('Panel dock controller unavailable');

            floatingWindowManager.getWindow('inspector')?.dock();
            // Match the certified browser protocol: bridge-ready occurs before
            // every lazy panel/canvas has necessarily finished its first work.
            await new Promise((resolve) => setTimeout(resolve, 3000));
            probe.startScale0UiAuditProbe({
                label: 'panel-shell-ten-cycle',
                rootSelector: '#panel-area',
            });
            probe.trackScale0UiMethods('panelDock', dock, ['floatPanel', 'activate', 'setCollapsed']);
            const baselineWindows = floatingWindowManager.windows.size;
            const baselineParent = document.getElementById('panel-inspector')?.parentElement;
            const latencies = [];

            for (let i = 0; i < 10; i += 1) {
                const latency = await probe.measureScale0UiActionToPaint(`panel-shell-cycle-${i}`, () => {
                    const win = dock.floatPanel('inspector', 180 + i, 100 + i);
                    if (!win) throw new Error(`Float failed on cycle ${i}`);
                    win.toggleCollapse();
                    win.toggleCollapse();
                    win.startDrag(200 + i, 120 + i);
                    win._onPointerMove({ clientX: 220 + i, clientY: 140 + i });
                    win._onPointerUp();
                    win.dock();
                });
                latencies.push(latency);
            }

            await new Promise((resolve) => setTimeout(resolve, 350));
            const stopped = await probe.stopScale0UiAuditProbe();
            return {
                report: stopped,
                latencies,
                baselineWindows,
                finalWindows: floatingWindowManager.windows.size,
                panelReturned: document.getElementById('panel-inspector')?.parentElement === baselineParent,
                floatedTabs: document.querySelectorAll('.tab.is-floated').length,
            };
        });

        expect(report.finalWindows).toBe(report.baselineWindows);
        expect(report.panelReturned).toBe(true);
        expect(report.floatedTabs).toBe(0);
        expect(report.report.resourceDelta.rafSubscribers).toBe(0);
        expect(report.report.errors).toEqual([]);
        expect(report.report.longTasks).toEqual([]);
        expect(Math.max(...report.latencies)).toBeLessThan(50);
        expect(report.report.methods['panelDock.floatPanel']).toBe(10);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('collapse resize notification is debounced to the final state', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const dock = window.__ftdCtx?.appShell?.panelDock;
            if (!dock) throw new Error('Panel dock controller unavailable');
            const original = dock.onViewportResize;
            const originalCollapsed = document.getElementById('app')?.classList.contains('panels-collapsed');
            let calls = 0;
            dock.onViewportResize = () => { calls += 1; };
            try {
                for (let i = 0; i < 10; i += 1) {
                    dock.setCollapsed(i % 2 === 0);
                }
                await new Promise((resolve) => setTimeout(resolve, 300));
                return { calls, finalCollapsed: document.getElementById('app')?.classList.contains('panels-collapsed') };
            } finally {
                dock.onViewportResize = original;
                dock.setCollapsed(originalCollapsed);
            }
        });

        expect(result.calls).toBe(1);
        expect(result.finalCollapsed).toBe(false);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('mobile sheet swipe, cancel, and scroll-lock lifecycle is race-safe', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.setViewportSize({ width: 390, height: 844 });
        const result = await page.evaluate(async () => {
            const shell = window.__ftdCtx?.appShell;
            const mobile = shell?.mobilePanel;
            const dock = shell?.panelDock;
            const panelArea = document.getElementById('panel-area');
            if (!mobile || !dock || !panelArea) throw new Error('Mobile panel shell unavailable');

            const dispatchTouch = (type, clientY) => {
                const event = new Event(type, { bubbles: true, cancelable: true });
                Object.defineProperty(event, 'touches', {
                    value: clientY === null ? [] : [{ clientY }],
                });
                panelArea.dispatchEvent(event);
                return event.defaultPrevented;
            };

            dock.setCollapsed(false);
            await new Promise((resolve) => setTimeout(resolve, 0));
            const openLock = document.body.classList.contains('body-panel-open');
            dispatchTouch('touchstart', 100);
            const movePrevented = dispatchTouch('touchmove', 170);
            dispatchTouch('touchend', null);
            await new Promise((resolve) => setTimeout(resolve, 0));
            const collapsed = document.getElementById('app')?.classList.contains('panels-collapsed');
            const collapsedLock = document.body.classList.contains('body-panel-open');

            dispatchTouch('touchstart', 200);
            dispatchTouch('touchmove', 150);
            dispatchTouch('touchend', null);
            await new Promise((resolve) => setTimeout(resolve, 0));
            const reopened = !document.getElementById('app')?.classList.contains('panels-collapsed');
            const reopenedLock = document.body.classList.contains('body-panel-open');

            dispatchTouch('touchstart', 100);
            dispatchTouch('touchmove', 130);
            const transformDuringSwipe = panelArea.style.transform;
            dispatchTouch('touchcancel', null);
            return {
                openLock,
                movePrevented,
                collapsed,
                collapsedLock,
                reopened,
                reopenedLock,
                transformDuringSwipe,
                transformAfterCancel: panelArea.style.transform,
                touchActiveAfterCancel: mobile._touch.active,
                initialized: mobile._initialized,
                touchTargetCount: mobile._getTouchTargets().length,
            };
        });

        expect(result.openLock).toBe(true);
        expect(result.movePrevented).toBe(true);
        expect(result.collapsed).toBe(true);
        expect(result.collapsedLock).toBe(false);
        expect(result.reopened).toBe(true);
        expect(result.reopenedLock).toBe(true);
        expect(result.transformDuringSwipe).toContain('translateY');
        expect(result.transformAfterCancel).toBe('');
        expect(result.touchActiveAfterCancel).toBe(false);
        expect(result.initialized).toBe(true);
        expect(result.touchTargetCount).toBe(1);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('Scale 0 exit and re-entry preserve one shell owner and one mount state', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const beforeMount = await page.evaluate(() => {
            window.__gate1DockBefore = window.__ftdCtx?.appShell?.panelDock;
            return document.documentElement.dataset.panelMount;
        });

        await switchMode(page, 'particles');
        await expect.poll(() => page.locator('#app').getAttribute('data-active-scale')).toBe('1');
        await switchMode(page, 'lattice');
        await expect.poll(() => page.locator('#app').getAttribute('data-active-scale'), { timeout: 30_000 }).toBe('0');
        await page.waitForTimeout(500);

        const after = await page.evaluate((initialMount) => {
            const shell = window.__ftdCtx?.appShell;
            const activePanel = document.querySelector('#panel-area .panel.active');
            return {
                sameDock: shell?.panelDock === window.__gate1DockBefore,
                mount: document.documentElement.dataset.panelMount,
                initialMount,
                mountToggleCount: document.querySelectorAll('#panel-mount-toggle').length,
                activePanelCount: document.querySelectorAll('#panel-area .panel.active').length,
                mountedPanelClassCount: document.querySelectorAll(
                    '#panel-area .panel.panel-mount-left,' +
                    '#panel-area .panel.panel-mount-bottom,' +
                    '#panel-area .panel.panel-mount-right',
                ).length,
                activePanelHasMountClass: [...(activePanel?.classList || [])]
                    .some((name) => name.startsWith('panel-mount-')),
                floatingWindowCount: document.querySelectorAll('.floating-window').length,
                activeScale: document.getElementById('app')?.dataset.activeScale,
            };
        }, beforeMount);

        expect(after.sameDock).toBe(true);
        expect(after.mount).toBe(after.initialMount);
        expect(after.mountToggleCount).toBe(1);
        expect(after.activePanelCount).toBe(1);
        expect(after.mountedPanelClassCount).toBe(1);
        expect(after.activePanelHasMountClass).toBe(true);
        expect(after.floatingWindowCount).toBe(0);
        expect(after.activeScale).toBe('0');
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
