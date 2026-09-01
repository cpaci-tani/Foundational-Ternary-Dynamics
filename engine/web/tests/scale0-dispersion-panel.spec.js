// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors, selectScale0Scenario } from './_helpers.js';

test('Dispersion is an explicit static reference atlas with one-shot repaint ownership', async ({ page }) => {
    const consoleErrors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: '/?engine=wasm' });
    await selectScale0Scenario(page, 'empty');

    const before = await page.evaluate(async () => {
        const { initDispersionPanel } = await import('/js/scales/scale0/ui/overlays/dispersion-panel.js');
        const api = window.__ftdDispersionPanel;
        return {
            singleton: Array.from({ length: 10 }, () => initDispersionPanel())
                .every((value) => value === api),
            applicability: api?.applicability,
            armActive: api?.armCoordinatorActive,
            rows: api?.element?.querySelectorAll('.dp-row').length,
            atlasPoints: api?.element?.querySelectorAll('.dp-plot circle').length,
        };
    });

    expect(before.singleton).toBe(true);
    expect(before.applicability).toBe('reference-atlas');
    expect(before.armActive).toBe(true);
    expect(before.rows).toBe(5);
    expect(before.atlasPoints).toBe(24);

    await page.locator('#tab-bar .tab[data-panel="dispersion"]').click();
    await page.waitForTimeout(1_200);
    const active = await page.evaluate(() => ({
        armActive: window.__ftdDispersionPanel?.armCoordinatorActive,
        applicability: window.__ftdDispersionPanel?.applicability,
    }));
    expect(active).toEqual({ armActive: false, applicability: 'reference-atlas' });

    await page.locator('#dispersion-panel-measure').click();
    await expect(page.locator('#dispersion-panel-status')).toContainText(
        'Live remeasurement is not implemented',
    );
    await expect(page.locator('#dispersion-panel-livelegend')).toBeHidden();
    expect(realErrors(consoleErrors)).toEqual([]);
});
