// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

test('Scale 0 visualization panel aligns with the left sidebar above the status bar', async ({ page }) => {
    test.setTimeout(120_000);
    await page.setViewportSize({ width: 2048, height: 1022 });
    const consoleErrors = attachConsoleWatcher(page);
    await gotoAndReady(page);
    await page.waitForFunction(() => window.__ftdCtx?.fluxMock?.ready === true);

    await page.evaluate(async () => {
        const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
        writePanelMount('left');
        const app = document.getElementById('app');
        if (app?.classList.contains('panels-collapsed')) {
            document.getElementById('btn-panel-toggle')?.click();
        }
        await new Promise((resolve) => requestAnimationFrame(() => resolve()));
    });

    const layout = await page.evaluate(() => {
        const overlay = document.getElementById('viewport-overlay');
        const panelArea = document.getElementById('panel-area');
        const statusBar = document.getElementById('status-bar');
        const body = overlay?.querySelector('.s0-overlay-body');
        if (!overlay || !panelArea || !statusBar || !body) return null;
        const overlayRect = overlay.getBoundingClientRect();
        const panelRect = panelArea.getBoundingClientRect();
        const statusRect = statusBar.getBoundingClientRect();
        return {
            overlayBottom: overlayRect.bottom,
            panelBottom: panelRect.bottom,
            statusTop: statusRect.top,
            gapAboveStatus: statusRect.top - overlayRect.bottom,
            maxHeight: getComputedStyle(overlay).maxHeight,
            bodyOverflowY: getComputedStyle(body).overflowY,
        };
    });

    expect(layout).not.toBeNull();
    expect(Math.abs(layout.overlayBottom - layout.panelBottom)).toBeLessThanOrEqual(1);
    expect(layout.gapAboveStatus).toBeGreaterThanOrEqual(11);
    expect(layout.maxHeight).not.toBe('none');
    expect(layout.bodyOverflowY).toBe('auto');
    expect(realErrors(consoleErrors)).toEqual([]);
});
